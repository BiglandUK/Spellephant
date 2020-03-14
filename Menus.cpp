// Menus.cpp
#include "Menus.h"
#include "BackBuffer.h"
#include "Button.h"
#include <algorithm>
#include <functional>
#include <string>
#include "DBController.h"
#include "ScreenPrinter.h"
#include "ScrollBox.h"
#include "Utility.h"

using namespace Gdiplus;
using namespace std;

//template <typename T>
//static bool deleteAll( T* theElement ) { delete theElement; return true; }

TopMenu::TopMenu(unsigned int& nextMode, unsigned int previousMode, unsigned int id, BackBuffer* bb,
                 const unsigned int numSpellers):
    Mode(nextMode, previousMode, id),
    numSpellers_(numSpellers) 
{
    pBackground_ = new Image(L"Images/MainMenu.jpg");
    
    // Get button images
    pButtonImages_.push_back(new Bitmap(L"Images/btnSelectSpeller.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnNewSpeller.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnQuit.jpg"));
    
    // Create buttons
    buttons_.push_back(Button(pButtonImages_[SELECT], PointF(113.0, 313.0)));
    buttons_.push_back(Button(pButtonImages_[NEW], PointF(113.0, 187.0),2));
    buttons_.push_back(Button(pButtonImages_[QUIT], PointF(551.0, 313.0),2));
    
    if( numSpellers_ == 0 ) buttons_[0].Disable();
}



TopMenu::~TopMenu(){

delete pBackground_;
pBackground_ = 0;

remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>);
}

void TopMenu::Update( double dt, const Gdiplus::PointF* cursorPos){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun_ref( &Button::Update ), cursorPos));
}


void TopMenu::Display(BackBuffer* bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX ),
                                          static_cast<float>( bb->mOffY ),
                                          static_cast<float>( bb->width() ),
                                          static_cast<float>( bb->height() )));
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun_ref( &Button::Display), bb));
}

void TopMenu::LMBUp(const Gdiplus::PointF* cursorPos){
    //for_each(buttons_.begin(), buttons_.end(), mem_fun_ref( &Button::Release ) );
    if( buttons_[QUIT].Release(cursorPos) ){
        nextMode_ = 3; // Quit game
        return;
    }
    if( buttons_[SELECT].Release(cursorPos) ){
        nextMode_ = 5; // SELECTSPELLER
        return;
    }
    if( buttons_[NEW].Release(cursorPos) ) {
        nextMode_ = 4;
        return;
    }
}
void TopMenu::LMBDown(const Gdiplus::PointF* cursorPos, const double time){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun_ref( &Button::Click ), cursorPos));
}
void TopMenu::KeyDown(unsigned int key){
    //if( key == 78 ) buttons_[NEW].ForceClick();
    if( key == 81 ){
        nextMode_=3;
        return;
    }

}

void TopMenu::KeyUp(){}

void TopMenu::RMBUp(const Gdiplus::PointF* cursorPos){}
void TopMenu::RMBDown(const Gdiplus::PointF* cursorPos){}
void TopMenu::Wheel(short zDelta, Gdiplus::PointF* mousePos){}


/* NewSpeller Page
    Allows the creation of a new speller account
*/

NewSpeller::NewSpeller(unsigned int& nextMode, unsigned int previousMode, unsigned int id, Font* font,
                        DBController* db, ScreenPrinter* sp, unsigned int& spellerID,
                        TagList& tagList):
    Mode(nextMode, previousMode, id), mpFont_(font), keyboard_(0), pDB_(db), pScreenPrinter_(sp), pAvatar_(0),
    spellerName_(L""), avatarZone_(212.0, 186.0,128.0f, 128.0f), maxCharacters_(20),
    showNameExistsWarning_(false), spellerID_(spellerID), tagList_(tagList)
{
    pBackground_ = new Image(L"Images/NewSpeller.jpg");
    pNameExists_ = new Image(L"Images/NameExists.png");
    
    // Get button images
    pButtonImages_.push_back(new Bitmap(L"Images/btnOK.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnCancel.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnKeyboards.jpg"));
    
    // Create buttons
    buttons_.push_back(new Button(pButtonImages_[OK], PointF(250.0, 340.0)));
    buttons_.push_back(new Button(pButtonImages_[CANCEL], PointF(630.0, 340.0)));
    buttons_.push_back(new ToggleButton(pButtonImages_[KEYBOARDS], PointF(440.0, 340.0), 2,
                        0.0f, 0.0f,Button::Normal, Button::Normal, Color(255, 255, 0),
                        5.0f, 4, 1, 1));
    
    
    //Keyboards
    KeyboardSetup();
    
    //Set up Avatar images
    idList_.clear();
    pDB_->GetAvatarIDList(idList_);  // populate list with avatar IDs
    displayedAvatar_ = idList_.begin(); // point to first avatar
    GetNextAvatar(); // ensures a valid avatar is displayed
    
    // Get existing speller names
    existingNames_.clear();
    pDB_->GetSpellerNames(existingNames_);
}

NewSpeller::~NewSpeller(){

delete pBackground_;
pBackground_ = 0;

delete pNameExists_;
pNameExists_ = 0;

remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>);
remove_if(buttons_.begin(), buttons_.end(), deleteAll<Button>);

delete pAvatar_;
}

void NewSpeller::Update( double dt, const Gdiplus::PointF* cursorPos){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Update ), cursorPos));
    if( keyboard_ )
        keyboard_->Update(cursorPos);
}


void NewSpeller::Display(BackBuffer* bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF( static_cast<float>( bb->mOffX ),
                                           static_cast<float>( bb->mOffY ),
                                           static_cast<float>( bb->width() ),
                                           static_cast<float>( bb->height() )));
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun( &Button::Display), bb));
    if( keyboard_ )
        keyboard_->Display(bb);
    
    // Avatar
    RectF avatarZoneDisplay = avatarZone_;
    avatarZoneDisplay.Offset(PointF(static_cast<float>( bb->mOffX ), static_cast<float>( bb->mOffY )));
    graphics.DrawImage(pAvatar_, avatarZoneDisplay);
    
    // Name
    PointF namePos(360.0, 250.0);
    wstring caret = L"_";
    if( spellerName_.length() == maxCharacters_ ) caret = L"";
    RectF targetRec(namePos.X, namePos.Y, 460.0f, 50.0f);
    pScreenPrinter_->PrintTruncString(spellerName_ + caret, namePos, *mpFont_, Color(0,0,0),
                                      targetRec);
                                      
    // Name Exists warning
    if( showNameExistsWarning_ )
        graphics.DrawImage(pNameExists_, PointF(300.0f + static_cast<float>( bb->mOffX ),
                                                500.0f + static_cast<float>( bb->mOffY )));
        
}

void NewSpeller::LMBUp(const Gdiplus::PointF* cursorPos){
    
    if( buttons_[CANCEL]->Release(cursorPos) ){
        nextMode_ = 2;
        return;
    }
    
    if( buttons_[KEYBOARDS]->Release(cursorPos) ) {
        SwitchKeyboard(dynamic_cast<ToggleButton*>(buttons_[KEYBOARDS])->GetCurrentButton() );
        return;
    }
    
    if( buttons_[OK]->Release(cursorPos) ){
        CreateSpeller();
        return;
    }
    
    //for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Release ), cursorPos) );
    
    // Check keyboard
    if( keyboard_ )
        keyboard_->KeyRelease();
    
    // Check if avatar was clicked
    if( avatarZone_.Contains(*cursorPos) ){
        GetNextAvatar(); // Change avatar
    }
    
}

void NewSpeller::GetNextAvatar(){
    IDList::iterator iteratorCopy = displayedAvatar_; // Make a copy
    do{
        iteratorCopy++;
        if( iteratorCopy == idList_.end() )  // wraparound
            iteratorCopy = idList_.begin();
            
        delete pAvatar_; // delete previous image
        // Create new image
        pAvatar_ = new Image((L"Images/avatars/"+pDB_->GetAvatarFilenameFromID(*iteratorCopy)).c_str());
        
        if( iteratorCopy == displayedAvatar_ ) // Prevents infinite loop
            break;
            
    } while( pAvatar_->GetLastStatus() != Gdiplus::Ok );
    
    displayedAvatar_ = iteratorCopy; // copy back
}

void NewSpeller::LMBDown(const Gdiplus::PointF* cursorPos, const double time){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Click ), cursorPos));
    
    if( keyboard_ ){
        wchar_t ch = keyboard_->KeyClick(cursorPos); // Get character from key
        EditSpellerName(ch);
    }
}
void NewSpeller::KeyDown(unsigned int key){
    if( keyboard_ )
        keyboard_->Keystroke( key );
    
    EditSpellerName( static_cast<wchar_t>( key ) );
    
}

void NewSpeller::EditSpellerName( wchar_t ch ){

    // Has the speller pressed Enter?
    if( ch == ENTER ){
        CreateSpeller();
        return;
    }

    // Do nothing more if characters are at maximum and the key pressed isn't the delete key.
    if( spellerName_.length() == maxCharacters_ && ch != DEL )
        return;
    
    
    // Delete key
    if( ch == DEL && spellerName_.length() > 0 ) {
        spellerName_ = spellerName_.substr(0, spellerName_.length() - 1);
        showNameExistsWarning_ = false;
        return;
    }
    
    // Non-character key pressed - do nothing.
    if( ch < SPACE )
        return;
    
    spellerName_ += ch;
    showNameExistsWarning_ = false;
}

void NewSpeller::KeyUp(){
        if( keyboard_ ) keyboard_->KeyRelease();
}

void NewSpeller::RMBUp(const Gdiplus::PointF* cursorPos){}
void NewSpeller::RMBDown(const Gdiplus::PointF* cursorPos){}
void NewSpeller::Wheel(short zDelta, Gdiplus::PointF* mousePos){}

void NewSpeller::KeyboardSetup(){
    qwerty_.LoadKeyboard("Data/Keyboards/qwerty.txt");
    qwerty_.SetFont(&FontFamily(L"Impact"),26.0);//mpFont_);
    qwerty_.SetKeySize(50.0f, 50.0f);
    qwerty_.SetPosition(PointF(150.0, 480.0));
    qwerty_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));
                        
    abc_.LoadKeyboard("Data/Keyboards/abc.txt");
    abc_.SetFont(&FontFamily(L"Impact"),26.0);//mpFont_);
    abc_.SetKeySize(50.0f, 50.0f);
    abc_.SetPosition(PointF(200.0, 450.0));
    abc_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));

    special_.LoadKeyboard("Data/Keyboards/special.txt");
    special_.SetFont(&FontFamily(L"Impact"),26.0);//mpFont_);
    special_.SetKeySize(50.0f, 45.0f);
    special_.SetPosition(PointF(400.0, 430.0));
    special_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));                        
    keyboard_ = 0;
}

void NewSpeller::SwitchKeyboard(int num){
    switch( num ){
        case 1:{
            keyboard_ = 0;
            break;
        }
        case 2:{
            keyboard_ = &qwerty_;
            break;
        }
        case 3:{
            keyboard_ = &abc_;
            break;
        }
        case 4:{
            keyboard_ = &special_;
            break;
        }
        default:{
            keyboard_ = 0;
            break;
        }
    }
}


//extern const std::wstring reduce(const std::wstring& pString,
//                         const std::wstring& pFill = L" ",
//                         const std::wstring& pWhitespace = L" \t");
void NewSpeller::CreateSpeller(){
    // Cut out unnecessary spaces from the name
    wstring temp = reduce(spellerName_);
    if( temp.length() == 0 ) return;
    // Check if name already exists
    StringList::iterator iter = find(existingNames_.begin(), existingNames_.end(), temp);
    if( iter != existingNames_.end() ){
        // name already exists
        showNameExistsWarning_ = true;
        return;
    }
    // Create Speller
    spellerID_ = pDB_->AddSpeller(temp, *displayedAvatar_);
    CreateSpellerTags();
    SetDefaultDifficulty();
    nextMode_ = 7;
}

void NewSpeller::CreateSpellerTags(){
    if( tagList_.empty() )
        return;
    
    IDList tags;
    for(TagList::iterator iter = tagList_.begin(); iter != tagList_.end(); ++iter ){
        tags.insert(iter->GetID());
    }
    pDB_->AddSpellerTags(spellerID_, tags);
}

void NewSpeller::SetDefaultDifficulty(){
    pDB_->UpdateDifficulty(spellerID_, 1, 3);
}

/* SelectSpeller page
    Allows the selection of an existing account
*/

SelectSpeller::SelectSpeller(unsigned int &nextMode, unsigned int previousMode, unsigned int id, Gdiplus::Font *font,
                             DBController *db, unsigned int& spellerID)
: Mode(nextMode, previousMode, id), mpFont_(font), pDB_(db), spellerID_(spellerID)
{
    pBackground_ = new Image(L"Images/MainMenu.jpg");
    
    // Get button images
    pButtonImages_.push_back(new Bitmap(L"Images/btnOK.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnCancel.jpg"));
    
    // Create buttons
    buttons_.push_back(new Button(pButtonImages_[OK], PointF(250.0, 670.0)));
    buttons_.push_back(new Button(pButtonImages_[CANCEL], PointF(630.0, 670.0)));

    // SetUp ScrollBox
    SetUpScrollBox();
}

SelectSpeller::~SelectSpeller(){
    delete pBackground_;
    pBackground_ = 0;

    remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>);
    remove_if(buttons_.begin(), buttons_.end(), deleteAll<Button>);

    delete sbSpellerList_;
    sbSpellerList_ = 0;
    
    remove_if(spellerData_.begin(), spellerData_.end(), deleteAll<RowData>);
}

void SelectSpeller::Update( double dt, const Gdiplus::PointF* cursorPos){
   for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Update ), cursorPos));
   sbSpellerList_->Update(dt, *cursorPos);

}

void SelectSpeller::Display(BackBuffer* bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX ),
                                          static_cast<float>( bb->mOffY ),
                                          static_cast<float>( bb->width()),
                                          static_cast<float>( bb->height() )));
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun( &Button::Display), bb));
    
    sbSpellerList_->Display(*bb);

}

void SelectSpeller::LMBUp(const Gdiplus::PointF* cursorPos){
    
    if( buttons_[CANCEL]->Release(cursorPos) ){
        nextMode_ = 2;
        return;
    }
    
    if( buttons_[OK]->Release(cursorPos) ){
        int row = sbSpellerList_->SelectedRow();
        if( row < 0 || row >= static_cast<int>( spellerData_.size() ) )
            return;
        LoadSpeller( row );
    }
    
    sbSpellerList_->Release(*cursorPos);
}

void SelectSpeller::LMBDown(const Gdiplus::PointF* cursorPos, const double time){
    if( buttons_[CANCEL]->Click(cursorPos) ){
        return;
    }
    
    if( buttons_[OK]->Click(cursorPos) ){
        return;
    }
    
    bool dblClicked = false;
    std::pair<int,int> cell = sbSpellerList_->Click(*cursorPos, dblClicked, time);
    if( dblClicked )
        LoadSpeller(cell.first);
}

void SelectSpeller::KeyDown(unsigned int key){}
void SelectSpeller::KeyUp(){}
void SelectSpeller::RMBUp(const Gdiplus::PointF* cursorPos){}
void SelectSpeller::RMBDown(const Gdiplus::PointF* cursorPos){}
void SelectSpeller::Wheel(short zDelta, Gdiplus::PointF* mousePos){
    sbSpellerList_->Wheel(zDelta, *mousePos);
}


void SelectSpeller::SetUpScrollBox(){
    GetData();
    
    sbSpellerList_ = new ScrollBox(&spellerData_, PointF(160.0f, 160.0f), 75, 6);
    sbSpellerList_->AddColumn(Column::Centre, 75, 70, 70);
    sbSpellerList_->AddColumn(Column::Left, 500, mpFont_);
    
    sbSpellerList_->SetSelectedColour(Color(100,30,30));
}

void SelectSpeller::GetData(){
    spellerData_.clear();
    pDB_->GetSpellersAndAvatars( spellerData_ );
}

void SelectSpeller::LoadSpeller(int rowID){
    spellerID_ = spellerData_[rowID]->dataID_; // Get Speller ID from data
    nextMode_ = 7;
}

/* Speller Menu page
    The launch page for Game Modes and options
*/

SpellerMenu::SpellerMenu(unsigned int& nextMode, unsigned int previousMode, unsigned int id, Gdiplus::Font* font,
               DBController* db)
: Mode(nextMode, previousMode, id), mpFont_(font), pDB_(db)
{
    pBackground_ = new Image(L"Images/MainMenu.jpg");
    
    // Get button images
    pButtonImages_.push_back(new Bitmap(L"Images/btnMainMenu.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnWordOptions.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnQuickSpell.jpg"));
    
    // Create buttons
    buttons_.push_back(new Button(pButtonImages_[MAIN], PointF(50.0, 600.0)));
    buttons_.push_back(new Button(pButtonImages_[WORDOPTIONS], PointF(600.0, 600.0)));
    buttons_.push_back(new Button(pButtonImages_[QUICKSPELL], PointF(50.0, 100.0)));

}

SpellerMenu::~SpellerMenu(){
    delete pBackground_;
    pBackground_ = 0;

    remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>);
    remove_if(buttons_.begin(), buttons_.end(), deleteAll<Button>);
    
}

void SpellerMenu::Update(double dt, const Gdiplus::PointF *cursorPos){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Update ), cursorPos));
}

void SpellerMenu::Display(BackBuffer *bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX ),
                                          static_cast<float>( bb->mOffY ),
                                          static_cast<float>( bb->width()),
                                          static_cast<float>( bb->height() )));
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun( &Button::Display), bb));
}

void SpellerMenu::LMBDown(const Gdiplus::PointF *cursorPos, const double time){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Click ), cursorPos));
}

void SpellerMenu::LMBUp(const Gdiplus::PointF *cursorPos){
    if( buttons_[MAIN]->Release(cursorPos) ){
        nextMode_ = MENU;
        return;
    }
    if( buttons_[WORDOPTIONS]->Release(cursorPos) ){
        nextMode_ = WORDLISTOPTIONS;
        return;
    }
    
   if( buttons_[QUICKSPELL]->Release(cursorPos) ){
        nextMode_ = ::QUICKSPELL;
        return;
   }
}

void SpellerMenu::RMBDown(const Gdiplus::PointF *cursorPos) {}

void SpellerMenu::RMBUp(const Gdiplus::PointF *cursorPos) {}

void SpellerMenu::Wheel(short zDelta, Gdiplus::PointF *mousePos) {}

void SpellerMenu::KeyUp() {}

void SpellerMenu::KeyDown(unsigned int key) {}
// Mode.cpp

#include "Mode.h"
#include "Utility.h"
#include <algorithm>
#include <functional>
#include <vector>
#include "Button.h"
#include "Definitions.h"
#include "Speller.h"
#include "Random.h"
#include "ScreenPrinter.h"
#include "DBController.h"
#include "AnimatedFeedback.h"
#include "SpellingSpotter.h"
using namespace Gdiplus;
using namespace std;

MiniSpell::MiniSpell(unsigned int& nextMode, unsigned int previousMode, unsigned int id,
                     WordBank &wordbank, Speller &speller, BackBuffer* bb, ScreenPrinter* sp, Font* font,
                     DBController* db,
                     Game game )
: Mode(nextMode, previousMode, id), wordBank_(wordbank), speller_(speller), bb_(bb), pScreenPrinter_(sp),
    mpFont_(font), pDB_(db), FADE_SPEED(1.00), lengthLimit_(WORD_LENGTH_LIMIT),
    game_(SPELLINGSPOTTING), usedWords_(0), state_(WAIT), pWord_(0), keyboard_(0), pAF_(0), pSSRegion_(0)
{
    pBackground_ = new Image(L"Images/background.jpg");
    CreateButtons();
    
    SetUp(); // Adjusts for different Game settings.
    SetUpWorkingList(); // Creates (possibly weighted) list of words to select from based on settings.
    KeyboardSetup();
}

MiniSpell::~MiniSpell(){
    if( pBackground_ )
        delete pBackground_;
    if( pAF_ )
        delete pAF_;
    if( pSSRegion_)
        delete pSSRegion_;
        
    pButtonImages_.erase(remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>),
                         pButtonImages_.end() );
    buttons_.erase(remove_if(buttons_.begin(), buttons_.end(), deleteAll<Button>),
                   buttons_.end() );
}

void MiniSpell::Update(double dt, const Gdiplus::PointF *cursorPos){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Update ), cursorPos));
    if( keyboard_ )
        keyboard_->Update(cursorPos);
    
    
    if( game_ == WORDWORKOUT ){ // Update instant word analysis variables   
        UpdateLetterTimings(dt);
        UpdateLetterAnalysis();
        UpdateLetterColours();
    }
    
    if( game_ == QUICKSPELL && state_ == CHECK && pAF_){ // Update Animated Feedback
        pAF_->Update( dt, *cursorPos );
    }
    
    if( game_ == SPELLINGSPOTTING && pSSRegion_ ){// Update highlight or colour animation
        pSSRegion_->Update( dt, cursorPos );
    }
    
}

void MiniSpell::Display(BackBuffer *bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX ),
                                          static_cast<float>( bb->mOffY ),
                                          static_cast<float>( bb->width()),
                                          static_cast<float>( bb->height() )));
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun( &Button::Display), bb));
    
    // Print rectangle in Speller's PAPER colour
    if( game_ != SPELLINGSPOTTING ){
        graphics.FillRectangle( &SolidBrush( speller_.GetColour(Speller::PAPER)),
                                 RectF(static_cast<float>( bb->mOffX ), 250.0f, static_cast<float>(bb->width()), 100.0f) );
    }
    
    //rest of display determined by state
    if( state_ == READ ){
        if( game_ == SPELLINGSPOTTING ){
            if( pSSRegion_ ){
                pSSRegion_->Display();
            }
        } else {
            ReadDisplay(bb);
        }
    }
    if( state_ == WRITE ){
        WriteDisplay(bb);
    }
    if( state_ == CHECK ){
        if( game_ == SPELLINGSPOTTING ){
            pSSRegion_->Display();
        } else {
            CheckDisplay(bb);
        }
    }
    if( keyboard_ )
        keyboard_->Display(bb);
}

void MiniSpell::LMBUp(const Gdiplus::PointF *cursorPos){
    if( buttons_[SPELLEROPTION]->Release(cursorPos) )
        return;
    
    if( buttons_[EXIT]->Release(cursorPos) ){
        nextMode_ = ::SPELLERMENU;
        return;
    }
    if( buttons_[WORDOPTION]->Release(cursorPos) ) {
        nextMode_ = ::WORDLISTOPTIONS;
        return;
    }
    if( buttons_[NEWWORD]->Release(cursorPos) ) {
        SetUpRead();
        return;
    }
    if( buttons_[WORDLIST]->Release(cursorPos) ) {
        ChangeSelectedList();
        return;
    }
    if( buttons_[COVEROPTION]->Release(cursorPos) ) {
        ChangeCoverOption();
        return;
    }
    if( buttons_[WRITEOPTION]->Release(cursorPos) ) {
        ChangeWriteOption();
        return;
    }
    if( buttons_[KEYBOARD]->Release(cursorPos) ){
        SwitchKeyboard(dynamic_cast<ToggleButton*>(buttons_[KEYBOARD])->GetCurrentButton() );
        return;
    }
    if( buttons_[DIACRITICS]->Release(cursorPos) ){
        ToggleDiacriticKeyboard();
        return;
    }
    
    // Check keyboard
    // TODO: Prevents click on the background, as this always returns if keyboard is on.
    if( keyboard_ ){
        keyboard_->KeyRelease();
    }
     
    if( pAF_ )
        pAF_->LMBUp( cursorPos );
    
    if( pSSRegion_ && state_ ==  READ){
        if( pSSRegion_->Click( cursorPos ) == true ) {
            state_ = CHECK;
            return;
        }
    }
    
    //IF CLICKED ELSEWHERE:
    if( state_ == READ && game_ != SPELLINGSPOTTING){
        SetUpWrite();
        return;
    }
    if( state_ == CHECK ){
        SetUpRead();
    }
}

void MiniSpell::LMBDown(const Gdiplus::PointF *cursorPos, const double time){
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Click ), cursorPos));
    
    if( state_ == WRITE && keyboard_ ){
        unsigned int key = static_cast<unsigned int>(keyboard_->KeyClick(cursorPos) ); // Get character from key
        if( key ) 
            EditAttempt( key );
    }
    
    if( pAF_ )
        pAF_->LMBDown( cursorPos, time );
}

void MiniSpell::KeyDown(unsigned int key){
    if( game_ != SPELLINGSPOTTING ){
        if( keyboard_ ) // Show stroke on keyboard
            keyboard_->Keystroke( key );

        if( state_ == READ ){
            SetUpWrite();
            return;
        }
        if( state_ == WRITE ){
            EditAttempt( key );
            return;
        }
    }
    if( state_ == CHECK ){ // Pressing a key during check gets a new word
        SetUpRead();
        return;
    }
}

void MiniSpell::KeyUp(){

    if( keyboard_ ) keyboard_->KeyRelease();
}

void MiniSpell::RMBUp(const Gdiplus::PointF *cursorPos){

}

void MiniSpell::RMBDown(const Gdiplus::PointF *cursorPos){

}

void MiniSpell::Wheel(short zDelta, Gdiplus::PointF *mousePos){

}

void MiniSpell::SetUp(){
    selectedList_ = NORMALLIST;
    readOption_   = READORSOUND;
    coverOption_  = TOTAL;
    writeOption_  = NOHELP;
    weightingOption_ = NOWEIGHTING;

    switch (game_){
        case QUICKSPELL:{
            buttons_[CHECKOPTION]->Disable();            
            break;        
        }
        case SPELLINGSPOTTING:{
            buttons_[COVEROPTION]->Disable();
            buttons_[WRITEOPTION]->Disable();
            buttons_[CHECKOPTION]->Disable();
            buttons_[KEYBOARD]->Hide();
            buttons_[DIACRITICS]->Hide();
            break;
        }
        case WORDWORKOUT:{
            buttons_[COVEROPTION]->Disable();
            buttons_[WRITEOPTION]->Disable();
            buttons_[CHECKOPTION]->Disable();
            break;
        }
        default:
            break;    
    }

}

void MiniSpell::SetUpWorkingList(){
    workingList_.clear(); // Clear any previous list
    //TODO: if list is changed with usedwords in use, might there be a problem reactivating a word that is no longer in the current list?
    totalWeighting_ = 0;  // Stores the total weighting of all available words.
    IDList tempList;      // This stores valid IDs ready for copying to Working List
    
    // Determine which base list to copy.
    if( selectedList_ == NORMALLIST ){
        tempList = speller_.GetWordList();
    } else {
        tempList = speller_.GetStarList();
    }
    
    // If speller wants sound only, remove any IDs of words that cannot be used for sound only challenges.
    if( readOption_ == SOUNDONLY ){
        IDList::iterator iter;
        for(iter = tempList.begin(); iter != tempList.end(); ){
            if( !( wordBank_.find(*iter)->second.SoundOnlyOK() ) ){
                tempList.erase( iter++ );
            } else {
                ++iter;
            }
        }
    }
    
    // If SPELLINGSPOTTING, there must only be words with at least one wrong spelling
    if( game_ == SPELLINGSPOTTING ){
        IDList::iterator iter;
        for(iter = tempList.begin(); iter != tempList.end(); ){
            if( speller_.GetNumWrongWords( *iter ) == 0 ){
                tempList.erase( iter++ );
            } else {
                ++iter;
            }
        }
    }

    // Weighting
    // As IDs copied into Working List, set the Weighting if required by speller's weighting Option.
    if( weightingOption_ == WEIGHTING ){
        WeightingCalculator(tempList, speller_, workingList_, totalWeighting_);
    } else {
        IDList::iterator iter;
        for(iter = tempList.begin(); iter != tempList.end(); ++iter ){
            SelectionID sid;
            sid.id_ = *iter;
            sid.enabled_ = true;
            sid.weighting_ = 1;  // No separate weight required.
            workingList_.push_back(sid);
        }
        totalWeighting_ = workingList_.size();
    }
    
    //Update fixed queue size
    if( workingList_.size() < 2 ){
        usedWords_.ChangeSize(0);
    } else {
        usedWords_.ChangeSize(workingList_.size()/2);
    }
    
    if( workingList_.empty() )
        buttons_[NEWWORD]->Disable(); // No words available, so disable New Word button.
    else
        buttons_[NEWWORD]->Enable();
}

void MiniSpell::SetUpRead(){
    if( workingList_.empty() ) { return; }
    
    unsigned int selectedID = 0;
    do{
        selectedID = GetNewWord();
    } while( selectedID == 0 ); // TODO: Infinite loop?
    unsigned int id = usedWords_.Add(selectedID);
    pWord_ = &(*(wordBank_.find(selectedID))).second; // TODO: Ouch, this looks clumsy.
    if( id > 0 )
        ReenableID(id);

    if( game_ == SPELLINGSPOTTING ){
        SetUpSSRegion(selectedID);
    }
    
    //buttons_[COVEROPTION]->Enable();  // Not needed.
    
    state_ = READ;
}

unsigned int MiniSpell::GetNewWord(){
    int select = Random(1, totalWeighting_);
    WorkingList::iterator iter = workingList_.begin();
    for( ; iter != workingList_.end(); ++iter){
        select -= iter->weighting_;
        if( select <= 0 ) break;
    }
    
    if( iter == workingList_.end() || !(iter->enabled_) )
        return 0;
    
    if (workingList_.size() > 1) // Only disable if using fixed queue (when more than one word available)
        iter->enabled_ = false;
    return iter->id_;
}

void MiniSpell::SetUpSSRegion(unsigned int id){
    if( pSSRegion_ ) delete pSSRegion_;
    
    StringVec tempWrong = speller_.GetWrongWords( id );
    // todo randomise wrong spellings (perhaps do in SSRegion)
    pSSRegion_ = new SSRegion(pWord_->GetMainSpellingString(), tempWrong, PointF(12.0f, 130.0f), bb_, mpFont_, speller_);
}

void MiniSpell::ReenableID(unsigned int id){
    WorkingList::iterator iter = find(workingList_.begin(), workingList_.end(), id);
    if( iter != workingList_.end() )
        iter->enabled_ = true;    
}

void MiniSpell::ChangeSelectedList(){
    int btnState = dynamic_cast<ToggleButton*>(buttons_[WORDLIST])->GetCurrentButton();
    switch( btnState ){
        case 1:{ // WordList
            selectedList_ = NORMALLIST;
            break;
        }
        default:{ // StarList
            selectedList_ = STAR;
            break;
        }
    }
    
    SetUpWorkingList();
}

void MiniSpell::SetUpWrite(){
    state_ = WRITE;
    ChangeCoverOption(); //Needed to update limitLength
    attempt_ = L""; // Clear attempt string.
    
    if( keyboard_ )
        WriteOptionAllKeys(); // update keyboard if already showing
        
    // Word Workout - clear analysis
    if( game_ == WORDWORKOUT){
        timings_.clear();
        analysis_.clear();
        letterColours_.clear();
    }
}

void MiniSpell::ChangeCoverOption(){
    switch( dynamic_cast<ToggleButton*>(buttons_[COVEROPTION])->GetCurrentButton() ){
        case 1:{ // Hide
            coverOption_ = TOTAL;
            lengthLimit_ = WORD_LENGTH_LIMIT;
            break;
        }
        default:{ // Spaces
            coverOption_ = SPACES;
            if( pWord_ ){
                lengthLimit_ = pWord_->GetMainSpelling().GetSpelling().size();
            }            
            break;
        }
    }
}

void MiniSpell::ChangeWriteOption(){
    switch( dynamic_cast<ToggleButton*>(buttons_[WRITEOPTION])->GetCurrentButton() ){
        case 1:{ // Hide
            writeOption_ = NOHELP;
            if( keyboard_ )
                keyboard_->ResetKeys();
            break;
        }
        case 2:{ // Hide
            writeOption_ = LETTERS;
            WriteOptionAllKeys();
            break;
        }
        default:{ // Spaces
            writeOption_ = LETTERSONCE;
            WriteOptionAllKeys();
            break;
        }
    }
}

void MiniSpell::ReadDisplay(BackBuffer* bb){
    if( readOption_ == READALWAYS )
        ShowWord(bb);
        
    if( readOption_ == READORSOUND ){
        if( pWord_->SoundOnlyOK() ){ // If sound exists, don't bother displaying.
            state_ = WRITE;
        } else { // Otherwise, show word
            ShowWord(bb);
        }
    }
    
    if( readOption_ == SOUNDONLY ) {
        // Don't display - go straight to WRITE
        SetUpWrite();
    }
        
}

void MiniSpell::WriteDisplay(BackBuffer* bb){
    // TEST - TODO: SpellSpotter does something very different.
    wstring suffix = L""; 
    if( coverOption_ == TOTAL )
        suffix = L"_";
    else {
        int spacesLeft = lengthLimit_ - attempt_.size();
        if( spacesLeft > 0 ){
            suffix.assign(spacesLeft, L'-');
        }
    }
    DisplayAttempt( bb, attempt_ + suffix);
}

void MiniSpell::CheckDisplay(BackBuffer* bb){
    // TODO: for QuickSpell, display animated feedback, or direct comparison
    if( game_ == QUICKSPELL && pAF_ ){
        pAF_->Display( 0.0 );
    }
    // TODO: for SpellingSpotting, display fading coloured words.
    // WordWorkout
    if( game_ == WORDWORKOUT)
        DisplayAttempt( bb, attempt_ );
}

void MiniSpell::DisplayAttempt(BackBuffer* bb, wstring printString ){
    if( game_ == QUICKSPELL ){
        WordPrinter wp;
        wp.PrintAttempt( printString, &speller_, pScreenPrinter_, bb, mpFont_, PointF(250.0f, 250.0f));
    }
    if( game_ == WORDWORKOUT ){
        PointF pos(250.0f, 250.0f);
        for( int i = 0; i < printString.size(); ++i ){
            pos = pScreenPrinter_->PrintLetter(printString[i], pos, *mpFont_, GetFadeColour(i));
        }
    } 
}



void MiniSpell::ShowWord(BackBuffer* bb){
    WordPrinter wp;
    wp.PrintWord(pWord_, &speller_, pScreenPrinter_, bb, mpFont_, PointF(250.0f, 250.0f));
}

void MiniSpell::CreateButtons(){
    pButtonImages_.push_back( new Bitmap(L"Images/btnSpellerOptionsS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnWordOptionsS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnListTypeS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnReadOptionS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnCoverOptionS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnWriteOptionS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnCheckOptionS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnExitS.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnNewWord.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnKeyboardsNoSymbols.jpg") );
    pButtonImages_.push_back( new Bitmap(L"Images/btnSymbols.jpg") );
    buttons_.push_back( new Button(pButtonImages_[SPELLEROPTION], PointF(0.0f, 640.0f),
                                    128.0f, 128.0f) );
    buttons_.push_back( new Button(pButtonImages_[WORDOPTION], PointF(128.0f, 640.0f),
                                    128.0f, 128.0f) );
    buttons_.push_back( new ToggleButton(pButtonImages_[WORDLIST], PointF(256.0f, 640.0f),
                                    128.0f, 128.0f,
                                    Button::Normal, Button::Normal,
                                    Color(255,255,0), 5.0,
                                    2, 1, 1  ) );
    buttons_.push_back( new ToggleButton(pButtonImages_[READOPTION], PointF(384.0f, 640.0f),
                                    128.0f, 128.0f,
                                    Button::Normal, Button::Normal,
                                    Color(255,255,0), 5.0,
                                    3, 2, 1  ) );
    buttons_.push_back( new ToggleButton(pButtonImages_[COVEROPTION], PointF(512.0f, 640.0f),
                                    128.0f, 128.0f,
                                    Button::Normal, Button::Normal,
                                    Color(255,255,0), 5.0,
                                    2, 1, 1  ) );
    buttons_.push_back( new ToggleButton(pButtonImages_[WRITEOPTION], PointF(640.0f, 640.0f),
                                    128.0f, 128.0f,
                                    Button::Normal, Button::Normal,
                                    Color(255,255,0), 5.0,
                                    3, 1, 1  ) );
    buttons_.push_back( new Button(pButtonImages_[CHECKOPTION], PointF(768.0f, 640.0f),
                                    128.0f, 128.0f ) );
    buttons_.push_back( new Button(pButtonImages_[EXIT], PointF(896.0f, 640.0f),
                                    128.0f, 128.0f ) );
    buttons_.push_back( new Button(pButtonImages_[NEWWORD], PointF(200.0f, 0.0f),
                                    128.0f, 128.0f ) );
    buttons_.push_back( new ToggleButton(pButtonImages_[KEYBOARD], PointF(600.0f, 0.0f),
                                    2,
                                    140.0f,77.0f,
                                    Button::Normal, Button::Normal,
                                    Color(255,255,0), 5.0f,
                                    3, 3, 3 ) );
    buttons_.push_back( new Button(pButtonImages_[DIACRITICS], PointF(750.0f, 0.0f),
                                   2) );
                                   
                                    

}


bool MiniSpell::CharacterAccepted(const wchar_t character){
    if( writeOption_ == NOHELP ) // Nothing to check, as no help allowed.
        return true;
    // Convert Character 
    wchar_t convertedCharacter = ConvertCharacter( character );
    
    wstring tempSpelling = ConvertSpelling( pWord_->GetMainSpellingString() );
    wstring tempAttempt  = ConvertSpelling( attempt_ );
    int charCountS = count(tempSpelling.begin(), tempSpelling.end(), convertedCharacter );
    if( writeOption_ == LETTERS )
        if( charCountS )
            return true;
        else
            return false;
    if( writeOption_ == LETTERSONCE ){
        int charCountA = count( tempAttempt.begin(), tempAttempt.end(), convertedCharacter );
            return charCountS > charCountA;
    }
    
    // Catch all - return false if in doubt.  TODO: Should this be true instead?
    return false;
}

wstring MiniSpell::ConvertSpelling( const wstring& original ){
    if( speller_.UseAutoDiacritics() ){
        return ToLower( original, true ); // Replace diacritics and capitals
    } else {
        return ToLower( original ); // Replace capitals
    }
}

wchar_t MiniSpell::ConvertCharacter(const wchar_t &original){
    wstring temp = L"";
    temp.assign(1, original);
    temp = ConvertSpelling( temp );
    return *(temp.begin());
}

void MiniSpell::EditAttempt( unsigned int key ){
    if( attempt_.size() >= lengthLimit_ ){ // Accept only delete or return
        if( key != 8 && key != 13 )
            return;
    }
    // TODO: If NOHELP writeoption, potentially any non-printable character is appended.
    // TODO: For Word Workout, need to allow alternative spellings.
    if( key == 13 ){ // ENTER
        // If using spaces, can only enter when all spaces filled
        if( coverOption_ == SPACES && attempt_.size() != lengthLimit_ )
            return;
        if( attempt_.empty() )// otherwise, can only enter when entering at least (1) letter
            return;
        // If WordWorkout, can only enter if all letters correct and no wrong ones
        if( game_ == WORDWORKOUT ){
            wstring spelling = pWord_->GetMainSpellingString();
            if( analysis_.size() != spelling.size() ) // Wrong no. of letters.
                return;
            if( count(analysis_.begin(), analysis_.end(), CORRECT ) != spelling.size() ) // Not all correct.
                return;
        }
        SetUpCheck();
        return;
    }
    
    if( key == 8 ) { // DELETE
        // Cannot delete if nothing to delete!
        if( attempt_.empty() )
            return;
        //Store the deleted character
        unsigned int deletedChar = attempt_[attempt_.size()-1];
        attempt_ = attempt_.substr(0, attempt_.size()-1);
        // Need to update for WordWorkout - otherwise it's possible to delete and replace a correct letter
        // with a wrong one, and still get it deemed correct.
        if( game_ == WORDWORKOUT ){
            DeleteLetterData();
        }
        WriteOptionSingleKey(deletedChar); // Update available letters
        return;
    }
    
    // Attempt to avoid adding keys like SHIFT to the attempt.
    if( key < 32 )
        return;
    
    // All other characters
    if( CharacterAccepted( static_cast<wchar_t>(key) ) ){
        attempt_ += static_cast<wchar_t>( key );
        WriteOptionSingleKey(key);
        if( game_ == WORDWORKOUT )
            CreateLetterData();
    }
}

void MiniSpell::KeyboardSetup(){
    qwerty_.LoadKeyboard("Data/Keyboards/qwerty.txt");
    qwerty_.SetFont(&FontFamily(L"Impact"),26.0);
    qwerty_.SetKeySize(50.0f, 50.0f);
    qwerty_.SetPosition(PointF(150.0, 350.0));
    qwerty_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));
                        
    abc_.LoadKeyboard("Data/Keyboards/abc.txt");
    abc_.SetFont(&FontFamily(L"Impact"),26.0);
    abc_.SetKeySize(50.0f, 50.0f);
    abc_.SetPosition(PointF(150.0, 350.0));
    abc_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));

    special_.LoadKeyboard("Data/Keyboards/special.txt");
    special_.SetFont(&FontFamily(L"Impact"),26.0);
    special_.SetKeySize(50.0f, 45.0f);
    special_.SetPosition(PointF(150.0, 350.0));
    special_.SetGraphics(L"Images/keys.bmp",0.0f,20.0f,40.0f,0.0f, 19.0f, 19.0f,
                        Color(0,0,0), Color(200,200,200), Color(200,200,0),
                        Color(200,200,0));                        
    keyboard_ = 0;
}

void MiniSpell::SwitchKeyboard(int num){
    switch( num ){
        case 1:{
            keyboard_ = &qwerty_;
            WriteOptionAllKeys();
            break;
        }
        case 2:{
            keyboard_ = &abc_;
            WriteOptionAllKeys();
            break;
        }
        default:{
            keyboard_ = 0;
            WriteOptionAllKeys();
            break;
        }
    }
}

void MiniSpell::ToggleDiacriticKeyboard(){
    keyboard_ = &special_;
    WriteOptionAllKeys();
    buttons_[KEYBOARD]->ResetState();
}

void MiniSpell::WriteOptionAllKeys(){
    // If no keyboard, wrong writeOption or wrong state, do nothing.
    if( !keyboard_ || writeOption_ == NOHELP || state_ != WRITE ) return;

    keyboard_->DisableAllKeys();
    

    keyboard_->EnableKey(static_cast<wchar_t>(8)); // enable delete
        
    keyboard_->EnableKey(static_cast<wchar_t>(13)); // enable enter

    // Cycle through each letter/character in spelling
    wstring spelling = pWord_->GetMainSpellingString();
    for( wstring::iterator iter = spelling.begin();
         iter != spelling.end();
         ++iter ){
        wchar_t character = ConvertCharacter(*iter);
        if( writeOption_ == LETTERS ){ // This option doesn't care if the letters are already in the attempt
            keyboard_->EnableKey( character ); // enable the key based on the converted character
            keyboard_->EnableKey( *iter );     // but also based on the original character
        } else { // Check the required letters are not already in the attempt
            if( CharacterAccepted( character ) )
                keyboard_->EnableKey( character );
            if( CharacterAccepted( *iter ) )
                keyboard_->EnableKey( *iter );
        }
    }
}

void MiniSpell::WriteOptionSingleKey( unsigned int key ){
    // If no keyboard, wrong writeOption or wrong state, do nothing.
    if( !keyboard_ || writeOption_ != LETTERSONCE || state_ != WRITE ) return;
    WriteOptionAllKeys();   
}

void MiniSpell::UpdateLetterTimings( double dt ){
    // Update existing timings.
    for( TimingsList::iterator iter = timings_.begin();
         iter != timings_.end();
         ++iter ){
         if( *iter < FADE_SPEED ){
            *iter += dt;
         }
    }
}

void MiniSpell::UpdateLetterAnalysis(){
    while( analysis_.size() < attempt_.size() ){
        wstring spelling = pWord_->GetMainSpellingString();
        for( int i = analysis_.size();
             i < attempt_.size();
             ++i) {
            if( i >= spelling.size() ){
                analysis_.push_back( WRONG );
            } else {
                if( LettersEqual( attempt_[i], spelling[i] ) ){
                    attempt_[i] = spelling[i]; // make the attempt character equal to the spelling character
                    analysis_.push_back(CORRECT);
                } else {
                    analysis_.push_back( WRONG );
                    if( !CheckForSwap( i ) ) {
                        // Letter is wrong and not a swap, but there still might be some autodiacritic stuff to do:
                        if( speller_.UseAutoDiacritics() && !( speller_.UseAutoCapitals() ) ){
                            SetAutoDiacritic( attempt_[i], spelling[i] );
                        }
                    }
                }
            }
        }// End for
    } // End while
}

void MiniSpell::UpdateLetterColours(){    
    // Update existing colours
    for( int i = 0;
         i < letterColours_.size();
         ++i ){
        
        BYTE rPen = letterColours_[i].GetRed();
        BYTE gPen = letterColours_[i].GetGreen();
        BYTE bPen = letterColours_[i].GetBlue();
        // Get target colour
        Color target = GetTargetColour( analysis_[i] );
               
        BYTE rTarget = target.GetRed();
        BYTE gTarget = target.GetGreen();
        BYTE bTarget = target.GetBlue();
    
        double ratio = timings_[i] / FADE_SPEED;
        if( ratio > 1.0 )
            ratio = 1.0;
    
        letterColours_[i] = Color(rPen + (rTarget - rPen) * ratio,
                                  gPen + (gTarget - gPen) * ratio,
                                  bPen + (bTarget - bPen) * ratio );
    } // End for


}

bool MiniSpell::LettersEqual(wchar_t c1, wchar_t c2) const{
    if( speller_.UseAutoDiacritics() ){
        c1 = RemoveDiacritic(c1);
        c2 = RemoveDiacritic(c2);
    }
    if( speller_.UseAutoCapitals() ){
        c1 = ToLower(c1);
        c2 = ToLower(c2);
    }
    return c1 == c2;
}

void MiniSpell::SetAutoDiacritic(wchar_t &attempt, wchar_t spelling){
    //Only continue if letters share the same base letter
    if( ToLower(attempt, true) == ToLower(spelling, true) ){
        //Determine cases first of all
        bool attemptIsUpper = false;
        if( attempt < 91 ||
            ( attempt > 191 && attempt < 222 )
           ) {
            attemptIsUpper = true;
        }
        bool spellingIsUpper = false;
        if( spelling < 91 ||
           ( spelling > 191 && spelling < 222 )
           ) {
            spellingIsUpper = true;
        }
        attempt = spelling;
        if( attemptIsUpper ){
            attempt = ToUpper( attempt );
        } else {
            attempt = ToLower( attempt );
        }
    }
}

Gdiplus::Color MiniSpell::GetFadeColour(int i){
    if( i >= letterColours_.size() ){
        return speller_.GetColour( Speller::PEN );
    }
    return letterColours_[i];
}

Gdiplus::Color MiniSpell::GetTargetColour(WWAnalysis analysis){
    switch( analysis ){
        case CORRECT: {
            return speller_.GetColour( Speller::CORRECT );
        }
        case WRONG: {
            return speller_.GetColour( Speller::WRONG );
        }
        case SWAPL:
        case SWAPR:{
            return speller_.GetColour( Speller::SWAPPED );
        }
        default: {
            return Color(0,0,0);
        }
    }
}

// TODO: Check for a deleted swap!
bool MiniSpell::CheckForSwap( int pos ){
    if( attempt_.size() < 2 ) return false; // can't compare with only one letter
    if( analysis_[pos-1] != WRONG ) return false; // previous letter has to be wrong
    wstring spelling = pWord_->GetMainSpellingString();
    if( attempt_.size() > spelling.size() ) return false; // speller has gone beyond the end of the word.
    
    // All basic checks okay, now on to the swap check
    if( LettersEqual( attempt_[pos], spelling[pos-1] ) &&
        LettersEqual( attempt_[pos-1], spelling[pos] ) ){
        analysis_[pos] = SWAPR;
        analysis_[pos-1] = SWAPL; // change analysis for previous letter
        timings_[pos-1] = 0.0; // Reset timing for previous letter
        return true; 
    }
    return false;

}

void MiniSpell::DeleteLetterData(){
    timings_.pop_back();
    if( analysis_.back() == SWAPR ){ // Deleted a swapped letter
        analysis_.pop_back();
        analysis_.back() = WRONG; // reset previous to wrong
        timings_.back() = 0.0; // and reset timings for it too.
    } else {
        analysis_.pop_back();
    }
    
    letterColours_.pop_back();
}

void MiniSpell::CreateLetterData(){
    UpdateLetterAnalysis();
    timings_.push_back(0.0);
    letterColours_.push_back( speller_.GetColour( Speller::PEN ) );
}

void MiniSpell::SetUpCheck(){
    // TODO: trim extra spaces?? Or restrict multiple spaces and spaces from ends?
    
    // See if there is an existing record for this word, and create one if there isn't.
    if( speller_.RecordExists( pWord_->GetID() ) ){
        // If so, increase the number of attempts.
        speller_.IncreaseAttempts( pWord_->GetID() );        
    } else {
        // If not, create a record and set attempts to 1.
        speller_.CreateRecord( pWord_->GetID() );
        // Add to database
        pDB_->AddSpellerRecord( speller_.GetID(), pWord_->GetID(), 1, 0);
    }
    
    // Sort out levels and wrong spellings, where appropriate.
    if( game_ == WORDWORKOUT ){ // Getting it correct is the only option in Word Workout
        IncreaseLevel(); // CONSIDER: ignoring changes to level in either direction in practice modes.
    }
    // Word analysis for QUICKSPELL - also adds a wrong spelling, if appropriate, and ups the level if word correct
    if( game_ == QUICKSPELL ){
        AnalysedWord aw( pWord_->GetMainSpellingString().length() );
        SpellingAnalyser sp( attempt_, pWord_, speller_, aw );
        if( !( aw.IsCorrect() || aw.IsBeyondWrong() ) ){
            WrongSpelling ws(aw);
            speller_.AddWrongSpelling( pWord_->GetID(), ws, pDB_ );
        }
        if( aw.IsCorrect() )
            IncreaseLevel();
        SetUpAnimatedFeedback( aw );
    }
    
    // Update attempts and level in DB
    pDB_->UpdateSpellerRecord( speller_, pWord_->GetID() ); 

    state_ = CHECK;
}

void MiniSpell::IncreaseLevel(){
    int level = speller_.GetWordLevel( pWord_->GetID() );
    if( level == 0 ) return; // no change if not got a level yet.
    
    if( level < TOP_LEVEL_RANK_1 ){
        level = TOP_LEVEL_RANK_1;
    } else if( level < TOP_LEVEL_RANK_2 ){
        level = TOP_LEVEL_RANK_2;
    } else if ( level < TOP_LEVEL_RANK_3 ){
        level = TOP_LEVEL_RANK_3;
    }
    if( level != speller_.GetWordLevel( pWord_->GetID() ) )
        speller_.SetLevel( pWord_->GetID(), level );
}

void MiniSpell::SetUpAnimatedFeedback( AnalysedWord& aw ){
    // Delete any previous one
    if( pAF_ ){
        delete pAF_;
        pAF_ = 0;
    }

    // No AF if speller option off or attempt is Beyond Wrong
    if( !(speller_.UseAnimatedFeedback() ) || aw.IsBeyondWrong() ) 
        return;
    // Also no AF for 2 or 3 letter words, unless:
    // At LEAST ONE letter is correct, AND There is NO MORE than ONE error.
    if( pWord_->GetMainSpellingString().length() < 4 &&
        ( aw.NumCorrect() < 1 || aw.NumErrors() > 1 ) )
        return;
        
    pAF_ = new AnimatedFeedback(speller_, mpFont_, aw.GetAnalysis(), bb_ );
    pAF_->WordPosition( PointF(250.0f, 250.0f) );
    
}
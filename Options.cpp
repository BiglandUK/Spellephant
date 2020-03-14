//Options.cpp
#include "Options.h"
#include "BackBuffer.h"
#include "Button.h"
#include <algorithm>
#include <functional>
#include "ScrollBox.h"
#include "Convert.h"
#include "Word.h"
#include "Dumbell.h"
#include "DBController.h"
#include "Speller.h"

using namespace Gdiplus;
using namespace std;

//// Forward declarations and externs.
//template <typename T>
//static bool deleteAll( T* theElement ) { delete theElement; return true; }
//
//std::wstring ToLower( std::wstring s, const std::locale loc = std::locale() );


//extern std::wstring stringify(const unsigned int& x);


WordListOptions::WordListOptions(unsigned int &nextMode, unsigned int previousMode, unsigned int id,
                                 BackBuffer* bb,
                                 Gdiplus::Font* font, WordBank& wordBank, TagList& tagList,
                                 Speller* speller,
                                 DBController* db)
    : Mode(nextMode, previousMode, id),
    mpFont_(font), wordBank_(wordBank), tagList_(tagList),
    speller_(speller),
    refDifficulty_(speller->GetDifficulty()),
    refSpellerStars_(speller->GetStarList()),
    refSpellerTags_(speller->GetTagList()),
    difficulty_(speller->GetDifficulty()),
    pBackground_(0),
    selectedRow_(-1), 
    fState_(SHOWFILTERED),
    pDB_(db),
    spellerID_(speller->GetID())
{
    pTagFont_ = new Gdiplus::Font(L"Arial", 15.0);
    pWordFont_ = new Gdiplus::Font(L"Arial", 20.0);
    
    pBackground_ = new Image(L"Images/WordListOptions.jpg");

    pButtonImages_.push_back(new Bitmap(L"Images/btnOK.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnCancel.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/tagsAll.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/tagsSwap.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/wordsDiff.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/wordsAZ.jpg"));
    pButtonImages_.push_back(new Bitmap(L"Images/btnToggleFilter.jpg"));

    buttons_.push_back(new Button(pButtonImages_[SAVE], PointF(600.0f, 650.0f)));
    buttons_.push_back(new Button(pButtonImages_[CANCEL], PointF(750.0f, 650.0f)));
    buttons_.push_back(new Button(pButtonImages_[TAGSALL], PointF(600.0f, 170.0f),0.0, 0.0,
                                   Button::Normal, Button::Normal, Gdiplus::Color(255, 255, 0), 2.0));
    buttons_.push_back(new Button(pButtonImages_[TAGSSWAP], PointF(700.0f, 170.0f),0.0, 0.0,
                                   Button::Normal, Button::Normal, Gdiplus::Color(255, 255, 0), 2.0));
    buttons_.push_back(new Button(pButtonImages_[SORTDIFF], PointF(150.0f, 60.0f),0.0, 0.0,
                                   Button::Normal, Button::Normal, Gdiplus::Color(255, 255, 0), 3.0));
    buttons_.push_back(new Button(pButtonImages_[SORTAZ], PointF(250.0f, 60.0f),0.0, 0.0,
                                   Button::Normal, Button::Normal, Gdiplus::Color(255, 255, 0), 3.0));
    buttons_.push_back(new ToggleButton(pButtonImages_[WORDFILTER], PointF(350.0f, 60.0f), 0.0f, 0.0f,
                                   Button::Normal, Button::Normal, Gdiplus::Color(255,255, 0), 3.0,
                                   2,2,2));
   
    dbDifficulty_ = new Dumbell(10, 1, 580.0, 100.0, 350.0, Dumbell::SCALEWIDTH, 50.0, Color(55,0,255));
    dbDifficulty_->SetBallPositions(difficulty_.mLow, difficulty_.mHigh);
    
    SetUpTagScrollBox();
    SetUpWordScrollBox();
}

WordListOptions::~WordListOptions(){
    if( pBackground_)
        delete pBackground_;
    pBackground_ = 0;

    remove_if(pButtonImages_.begin(), pButtonImages_.end(), deleteAll<Bitmap>);
    remove_if(buttons_.begin(), buttons_.end(), deleteAll<Button>);
    
    if( sbTagList_ )
        delete sbTagList_;
    sbTagList_ = 0;
    
    if( sbWordList_ )
        delete sbWordList_;
    sbWordList_ = 0;
    
    remove_if(wordData_.begin(), wordData_.end(), deleteAll<RowData>);
    remove_if(tagData_.begin(), tagData_.end(), deleteAll<RowData>);
    
    delete pTagFont_;
    delete pWordFont_;
    delete dbDifficulty_;
}

void WordListOptions::Update( double dt, const Gdiplus::PointF* cursorPos){
    // Has the speller selected a different word since last time?  Or no word at all?
    if( selectedRow_ != sbWordList_->SelectedRow() ){
        selectedRow_ = sbWordList_->SelectedRow(); // Make them equal
        if( selectedRow_ == -1 ){ // No word selected
            for_each(tagData_.begin(), tagData_.end(), mem_fun( &RowData::Show ));
            stable_sort(tagData_.begin(), tagData_.end(), ColumnAlphaSort<0>);
            sbTagList_->Refresh();
        }
        else{ // Particular word selected
            SetUpWordTagData(wordData_[selectedRow_]->dataID_);
        }
    }
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd( mem_fun( &Button::Update ), cursorPos));
    
    sbTagList_->Update(dt, *cursorPos);
    sbWordList_->Update(dt, *cursorPos);
    dbDifficulty_->Update(*cursorPos);
}

void WordListOptions::Display(BackBuffer* bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX ),
                                          static_cast<float>( bb->mOffY ),
                                          static_cast<float>( bb->width() ),
                                          static_cast<float>( bb->height() )));
    
    for_each(buttons_.begin(), buttons_.end(), bind2nd(mem_fun( &Button::Display), bb));
    
    sbTagList_->Display(*bb);
    sbWordList_->Display(*bb);
    dbDifficulty_->Display(*bb);
    
    // Difficulty numbers
    float gap = static_cast<float>( dbDifficulty_->GetGapSize() );
    for(int i = 1; i < 11; ++i){
        wstring n = stringify(i);
        graphics.DrawString(n.c_str(), -1, pWordFont_, PointF(720.0f+(i-1)*gap, 70.0f), &SolidBrush(Color(0,0,0)));
    }
}

void WordListOptions::LMBDown(const Gdiplus::PointF *cursorPos, const double time ){
    bool offClick = true; // Determines if nothing of importance has been clicked (to clear the selection in Word list
    
    if( sbTagList_->Click(*cursorPos).first != -2 )
        offClick = false;
    if( sbTagList_->SelectedRow() > -1 ){
        tagData_[sbTagList_->SelectedRow()]->VisToggleActive();
        int tagID = tagData_[sbTagList_->SelectedRow()]->dataID_;
        UpdateWords(tagID);
        sbTagList_->ClearSelectedRow();
    }
    
    pair<int, int> cell = sbWordList_->Click(*cursorPos);
    if( cell.first != -2 ){
        offClick = false;
        if( cell.second == 3 ){ // TODO: Will need to be 4 if thumbs going in col 3.
            StarChange( wordData_[sbWordList_->SelectedRow()]->dataID_);
            if( selectedRow_ == -1 )
                sbWordList_->ClearSelectedRow();
        }
    }
    
    if( buttons_[SAVE]->Click(cursorPos) )
        offClick = false;
    if( buttons_[CANCEL]->Click(cursorPos) )
        offClick = false;
    if( buttons_[TAGSALL]->Click(cursorPos) )
        offClick = false;
    if( buttons_[TAGSSWAP]->Click(cursorPos) )
        offClick = false;
    if( buttons_[SORTDIFF]->Click(cursorPos) )
        offClick = false;
    if( buttons_[SORTAZ]->Click(cursorPos) )
        offClick = false;
    if( buttons_[WORDFILTER]->Click(cursorPos) )
        offClick = false;
        
    if( dbDifficulty_->Grab(*cursorPos) )
        offClick = false;
        
    if( offClick ) { // If nothing of importance has been clicked, clear the selection in the word list.
        sbWordList_->ClearSelectedRow();
    }

}

void WordListOptions::LMBUp(const Gdiplus::PointF *cursorPos){
    sbTagList_->Release( *cursorPos );
    sbWordList_->Release( *cursorPos );
    
    if( buttons_[TAGSALL]->Release(cursorPos) ){
        for_each(tagData_.begin(), tagData_.end(), mem_fun( &RowData::VisActivate ) );
        UpdateWords();
    }
    if( buttons_[TAGSSWAP]->Release(cursorPos) ){
        for_each(tagData_.begin(), tagData_.end(), mem_fun( &RowData::VisToggleActive ) );
        UpdateWords();
    }
    
    // Diff Sort button pressed & released
    if( buttons_[SORTDIFF]->Release(cursorPos) ){
        SortWords(SORTDIFFICULTY);
        ToggleWordFilter( fState_ );
        sbWordList_->ClearSelectedRow();
    }
    
    // AZ Sort button pressed & released
    if( buttons_[SORTAZ]->Release(cursorPos) ){
        SortWords( SORTALPHA );
        ToggleWordFilter( fState_ );
        sbWordList_->ClearSelectedRow();
    }
    
    // Word filter toggle button pressed & released
    if( buttons_[WORDFILTER]->Release(cursorPos) ){
        fState_ = static_cast<FilterState>(dynamic_cast<ToggleButton*>(buttons_[WORDFILTER])->GetCurrentButton());
        ToggleWordFilter( fState_ );
        SortWords( sState_ );
        sbWordList_->Refresh();
    }
    
    if( dbDifficulty_->Release() ){
        Range currDiff(0,0);
        dbDifficulty_->GetValues(currDiff.mLow, currDiff.mHigh);
        if( difficulty_.mLow != (currDiff.mLow) || difficulty_.mHigh != (currDiff.mHigh) ){
            ChangeDifficulty(currDiff);
        }
    }
    
    if( buttons_[SAVE]->Release(cursorPos) ){
        SaveChanges();
    }
    
    if( buttons_[CANCEL]->Release(cursorPos) ){
        Cancel();
    }
}

void WordListOptions::RMBDown(const Gdiplus::PointF *cursorPos){}

void WordListOptions::RMBUp(const Gdiplus::PointF *cursorPos){}

void WordListOptions::Wheel(short zDelta, Gdiplus::PointF *mousePos){
    sbTagList_->Wheel(zDelta, *mousePos);
    sbWordList_->Wheel(zDelta, *mousePos);
}

void WordListOptions::KeyDown(unsigned int key){}

void WordListOptions::KeyUp(){}

void WordListOptions::SetUpTagScrollBox(){
    tagData_.clear();
    // Add actual tags
    for( TagList::iterator iter = tagList_.begin(); iter != tagList_.end(); ++iter ){
        RowData rd;
        rd.dataID_     = iter->GetID();
        
        if( refSpellerTags_.find( rd.dataID_ ) == refSpellerTags_.end() ){
            rd.Deactivate();
        }
        else {
            rd.Activate();
        }
        
        rd.data_.push_back( iter->GetName() );
        rd.data_.push_back( stringify(iter->WordCount()) );
        tagData_.push_back(new RowData(rd.dataID_, rd.data_, rd.active_)); //problem - local object
    }

    sbTagList_ = new ScrollBox(&tagData_, PointF(600.0f, 200.0f), 40, 6);
    sbTagList_->AddColumn(Column::Left, 150, pTagFont_);
    sbTagList_->AddColumn(Column::Right, 50, pTagFont_);
    
    sbTagList_->SetRowColours(Color(100,10,100,100), Color(80,10,100,100) );

}

void WordListOptions::SetUpWordScrollBox(){
    wordData_.clear();
    
    for( WordBank::iterator iter = wordBank_.begin(); iter != wordBank_.end(); ++iter ){
        RowData rd;
        if( !WordInDifficultyRange(iter->first) )
            rd.active_ = false;
        else if( !WordHasActiveTags(iter->first) )
            rd.active_ = false;
        // Row ID (word ID)
        rd.dataID_ = iter->first;
        // Difficulty
        rd.data_.push_back( stringify(iter->second.GetDifficulty()) );
        // Current main spelling
        rd.data_.push_back( iter->second.GetMainSpellingString() );
        // TODO: rating.
        // Star status
        if( refSpellerStars_.find( iter->first) == refSpellerStars_.end() ){
            rd.data_.push_back(L"2Off");
        }
        else {
            rd.data_.push_back(L"1On");
        }
        wordData_.push_back(new RowData(rd.dataID_, rd.data_, rd.active_));
        TableData::iterator iter2 = wordData_.begin();

    }
    
    sbWordList_ = new ScrollBox(&wordData_, PointF(50.0f, 100.0f), 50);
    sbWordList_->AddColumn(Column::Centre, 50, pWordFont_); // Difficulty
    sbWordList_->AddColumn(Column::Left, 400, pWordFont_);  // (main) Spelling
    //// Word ranking
    //IconColumn::ValueList rank;
    //rank.push_back(L"3");
    //rank.push_back(L"2");
    //rank.push_back(L"1");
    //rank.push_back(L"0");
    //rank.push_back(L"-1");
    //rank.push_back(L"-2");
    //rank.push_back(L"-3");
    //sbWordList_->AddColumn(Column::Centre, 100, THUMBIMAGE, rank);
    // Starred words
    IconColumn::ValueList star;
    star.push_back(L"2Off");
    star.push_back(L"1On");
    starIcons_ = new Image(L"Images/stars.png");
    sbWordList_->AddColumn(Column::Centre, 50, starIcons_, star);
    
    sbWordList_->SetRowColours(Color(100,50,50,100), Color(80,50,50,100) );
    sbWordList_->SetSelectedColour( Color(50, 150, 100, 0) );
    
    // Sort into difficulty groups and A-Z within difficulties.
    SortWords( SORTALPHA );
    SortWords( SORTDIFFICULTY );
    sState_ = SORTDIFFICULTY;
}

void WordListOptions::SetUpWordTagData(unsigned int wordID) {
    WordBank::iterator iter = wordBank_.find(wordID);
    IDList& tags = iter->second.GetTags();
    // Cycle through the full tag data
    for( unsigned int i = 0; i < tagData_.size(); ++i ){
        // Is this tag in the selected Word's tag list?
        if( tags.count(tagData_[i]->dataID_) == 0 ){ 
            tagData_[i]->Hide();
        }
        else{
            tagData_[i]->Show();
        }
    }
    stable_sort(tagData_.begin(), tagData_.end(), ColumnAlphaSort<0>);
        
    sbTagList_->Refresh();
}

bool WordListOptions::WordInDifficultyRange(unsigned int wordID){
    unsigned int diff = wordBank_.find(wordID)->second.GetDifficulty();
    if( diff < static_cast<unsigned int>(difficulty_.mLow) || diff > static_cast<unsigned int>(difficulty_.mHigh) )
        return false;
    return true;
}

bool WordListOptions::WordHasActiveTags( unsigned int wordID ){
    IDList& tags = wordBank_.find(wordID)->second.GetTags();
    for( IDList::iterator iter = tags.begin(); iter != tags.end(); ++iter){
        TableData::iterator tIter = find_if(tagData_.begin(), tagData_.end(),
                                         bind2nd(mem_fun(&RowData::EqualToID), *iter));
        if( tIter == tagData_.end() ) continue;
        if( ((*tIter)->active_) )
            return true;
    }
    return false;
}

void WordListOptions::UpdateWords( int tagID ){
    TagList::iterator tIter = find_if(tagList_.begin(), tagList_.end(),
                                        bind2nd(mem_fun_ref(&Tag::EqualToID), static_cast<unsigned int>(tagID) ));
    if( tIter == tagList_.end() )
        return;
    IDList& words = tIter->GetWords();
    for( IDList::iterator iter = words.begin(); iter != words.end(); ++iter ){
        if( WordInDifficultyRange(*iter) ){ // Word must be within difficulty range to be active, regardless of tags
            if( WordHasActiveTags(*iter) ) {
                // Word is active (at least one tag active, and in difficulty range)
                (*find_if( wordData_.begin(), wordData_.end(),
                      bind2nd(mem_fun(&RowData::EqualToID), *iter)))->Activate();
            }
            else{
                // Word is inactive (all tags inactive)
                (*find_if( wordData_.begin(), wordData_.end(),
                      bind2nd(mem_fun(&RowData::EqualToID), *iter)))->Deactivate(); 
            }
        }
        else{
            // Word is inactive (difficulty range)
            (*find_if( wordData_.begin(), wordData_.end(),
                bind2nd(mem_fun(&RowData::EqualToID), *iter)))->Deactivate(); 
        }
    }
    ToggleWordFilter( fState_ );
    SortWords( sState_ );
}

void WordListOptions::UpdateWords(){
    for( TableData::iterator iter = wordData_.begin(); iter != wordData_.end(); ++iter ){   
        if( !WordInDifficultyRange((*iter)->dataID_) )
            (*iter)->Deactivate();
        else if( !WordHasActiveTags((*iter)->dataID_) )
            (*iter)->Deactivate();
        else
            (*iter)->Activate();
    }
    ToggleWordFilter( fState_ );
    SortWords( sState_ );
}

void WordListOptions::ToggleWordFilter(int state){
    if( state == SHOWFILTERED ){ // Show all words
      for_each(wordData_.begin(), wordData_.end(), mem_fun( &RowData::Show ));  
    }
    else{
        //Hide all filtered words
        for( TableData::iterator iter = wordData_.begin(); iter != wordData_.end(); ++iter){
            if( (*iter)->active_ )
                (*iter)->Show();
            else
                (*iter)->Hide();
        }
    }
}

void WordListOptions::SortWords( int state ){
    switch( state ){
        case SORTDIFFICULTY:{
            stable_sort(wordData_.begin(), wordData_.end(), ColumnNumericSort<0>);
            sState_ = SORTDIFFICULTY;
            break;
        }
        case SORTALPHA:{
            stable_sort(wordData_.begin(), wordData_.end(), ColumnAlphaSort<1>);
            sState_ = SORTALPHA;
            break;
        }
        case SORTSTARS:{
            stable_sort(wordData_.begin(), wordData_.end(), ColumnAlphaSort<2>);
            sState_ = SORTSTARS;
            break;
        }
        default:{
            break;
        }

    }
    sbWordList_->Refresh();
}

void WordListOptions::ChangeDifficulty( Range newDiff ){
    difficulty_.mLow = newDiff.mLow;
    difficulty_.mHigh = newDiff.mHigh;
    
    UpdateWords();
}

void WordListOptions::SaveChanges(){

    // Save difficulty level into current Speller object
    refDifficulty_.mLow = difficulty_.mLow;
    refDifficulty_.mHigh = difficulty_.mHigh;
    // Save difficulty level into database
    pDB_->UpdateDifficulty(spellerID_, refDifficulty_.mLow, refDifficulty_.mHigh);
    
    // Stars update
    IDList spellerStars; // Will hold new stars list
    for(TableData::iterator iter = wordData_.begin(); iter != wordData_.end(); ++iter ){
        if( (*iter)->data_[2] == L"1On" ){
            spellerStars.insert((*iter)->dataID_);
        }
    }
    
    if( !refSpellerStars_.empty() ) { // Only do this if there is an old stars list.
        // Remove stars from old list that are no longer in the new list
        IDList deleteStars; // Will hold word IDs to be deleted from star list in DB
        for( IDList::iterator iter = refSpellerStars_.begin(); iter != refSpellerStars_.end(); ++iter ){
            if( spellerStars.find( *iter ) == spellerStars.end() ){ // id not in new list
                // Add it to the delete list.
                deleteStars.insert( *iter );
            }
        }
        if( !deleteStars.empty() ){
            pDB_->DeleteStars(spellerID_, deleteStars );
        }
    }
        
    // Add stars in new list that are not currently in old list to the database
    if( !spellerStars.empty() ) { // Only do this if there are new stars
        IDList newStars; // For word IDs to be added to DB
        for( IDList::iterator iter = spellerStars.begin(); iter != spellerStars.end(); ++iter ){
            if( refSpellerStars_.find( *iter ) == refSpellerStars_.end() ) { // id not in old list
                // Add to new list
                newStars.insert( *iter );
            }
        }
        if( !newStars.empty() ){
            pDB_->AddStars( spellerID_, newStars );
        }
    }
    
    // Tags update
    IDList spellerTags; // Will hold new tags list
    for(TableData::iterator iter = tagData_.begin(); iter != tagData_.end(); ++iter ){
        if( (*iter)->active_ ){
            spellerTags.insert((*iter)->dataID_);
        }
    }
    
    if( !refSpellerTags_.empty() ) { // Only do this if there is an old tags list.
        // Remove tags from old list that are no longer in the new list
        IDList deleteTags; // Will hold tag IDs to be deleted from speller-tag list in DB
        for( IDList::iterator iter = refSpellerTags_.begin(); iter != refSpellerTags_.end(); ++iter ){
            if( spellerTags.find( *iter ) == spellerTags.end() ){ // id not in new list
                // Add it to the delete list.
                deleteTags.insert( *iter );
            }
        }
        if( !deleteTags.empty() ){
            pDB_->DeleteSpellerTags(spellerID_, deleteTags );
        }
    }
        
    // Add tags in new list that are not currently in old list to the database
    if( !spellerTags.empty() ) { // Only do this if there are new activated tags
        IDList newTags; // For tag IDs to be added to DB
        for( IDList::iterator iter = spellerTags.begin(); iter != spellerTags.end(); ++iter ){
            if( refSpellerTags_.find( *iter ) == refSpellerTags_.end() ) { // id not in old list
                // Add to new list
                newTags.insert( *iter );
            }
        }
        if( !newTags.empty() ){
            pDB_->AddSpellerTags( spellerID_, newTags );
        }
    }
    
    //Update speller wordlist
    speller_->UpdateWordList(wordBank_);
    
    // Return to Speller Menu
    nextMode_ = previousMode_;
}

void WordListOptions::Cancel(){
    // ignore all changes and return to calling Mode.
    nextMode_ = previousMode_;
}

void WordListOptions::StarChange(unsigned int wordID){
    TableData::iterator iter = find_if(wordData_.begin(), wordData_.end(),
                                        bind2nd(mem_fun( &RowData::EqualToID ), wordID ));
    if( (*iter)->data_[2] == L"1On" )
        (*iter)->data_[2] = L"2Off";
    else
        (*iter)->data_[2] = L"1On";
}
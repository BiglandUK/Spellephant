// Speller.cpp
#include "Speller.h"
#include "DBController.h"
#include <set>
#include <algorithm>

using namespace std;

WrongSpelling::WrongSpelling()
    : spelling_(L""), score_(0.0), aveLinkLength_(0.0), longestLink_(0), lengthDifference_(0)
{}

WrongSpelling::WrongSpelling(std::wstring spelling, int score, double aveLinkLength, unsigned int longestLink, unsigned int lengthDifference)
    : spelling_(spelling), score_(score), aveLinkLength_(aveLinkLength), longestLink_(longestLink), lengthDifference_(lengthDifference)
{}

WrongSpelling::WrongSpelling(const AnalysedWord &aw )
    : spelling_( aw.GetString() ),
      score_( aw.Score() ),
      aveLinkLength_( aw.AverageLinkSize() ),
      longestLink_( aw.LargestLink() ),
      lengthDifference_( aw.LengthDifference() )
{}

// TODO: This breaks the requirement of strict weak ordering...
// TODO:Check this with the version in AnalysedWord (SortOrder).  AnalysedWord seems to work - this needs rewriting.
bool WrongSpelling::operator <(const WrongSpelling &rhs){
    if( score_ != rhs.score_ )
        return score_ > rhs.score_;
    if( aveLinkLength_ != rhs.aveLinkLength_ )
        return aveLinkLength_ > rhs.aveLinkLength_;
    if( lengthDifference_ != rhs.lengthDifference_ )
        return lengthDifference_ < rhs.lengthDifference_;
    if( longestLink_ != rhs.longestLink_ )
        return longestLink_ > rhs.longestLink_;
    return false;  // Must return false for equal values, to comply with Strict Weak Ordering.
    // (see Josuttis P176 and Item 21 of Meyers' Effective STL)
        
    //1.	The highest Score is a better spelling.  If equal: 
    //2.	The largest average link length is better.  If equal:
    //3.	The least difference between length of target and attempt is better.  If equal:
    //4.	The largest "biggest link" is better.  If equal:
    //5.	The algorithm results / wrong spellings are deemed equal.  The newer result replaces the oldest.
    // PART 5 not dealt with by this function - It's dealt with (hopefully!) by inserting the newer word
    // at the start of the container so that, when sorting, it ends up ahead of other, equal words.
}

unsigned int Record::MAXWRONGSPELLINGS = 10;

Record::Record()
    : wordID_(0), numAttempts_(0), level_(0)
{
    wrongSpellings_.clear();
}

Record::Record(unsigned int wordID, unsigned int numAttempts, int level)
    : wordID_(wordID), numAttempts_(numAttempts), level_(level)
{
    wrongSpellings_.clear();
}

bool Record::operator <( const Record& rhs){
    return wordID_ <  rhs.wordID_;
}

unsigned int Record::GetAttempts() const{
    return numAttempts_;
}

int Record::GetLevel() const{
    return level_;
}

void Record::SetLevel( const int level ){
    level_ = level;
}

void Record::AddAttempt(){
    ++numAttempts_;
}

void Record::AddWrongSpelling( WrongSpelling& ws ){
    wrongSpellings_.push_back( ws );
}

void Record::AddWrongSpelling( WrongSpelling &ws, unsigned int spellerID, DBController* db ){
    // Does this wrongspelling already exist?
    for( WrongSpellingList::iterator iter = wrongSpellings_.begin();
         iter != wrongSpellings_.end();
         ++iter ){
        if( iter->spelling_ == ws.spelling_ )
            return;    
    }
    
    wrongSpellings_.insert(wrongSpellings_.begin(), ws );  // Placed at start on purpose - older equal words should be removed first.
    // Add it to the database
    db->AddWrongSpelling( spellerID, wordID_, ws );
    RemoveExcessiveWrongSpellings( spellerID, db );

}

unsigned int Record::GetNumWrongWords() const{
    return static_cast<unsigned int>( wrongSpellings_.size() );
}

StringVec Record::GetWrongWords() const{
    StringVec wrongSpellings;
    for( int i = 0; i < wrongSpellings_.size(); ++i ){
        wrongSpellings.push_back( wrongSpellings_[i].spelling_ );    
    }
    return wrongSpellings;
}

void Record::RemoveExcessiveWrongSpellings( unsigned int spellerID, DBController* db ){
    // Are there too many wrong spellings?
    if( wrongSpellings_.size() > MAXWRONGSPELLINGS ){
        sort( wrongSpellings_.begin(), wrongSpellings_.end() );
        while( wrongSpellings_.size() > MAXWRONGSPELLINGS ){ 
            // Remove from database
            db->DeleteWrongSpelling( spellerID, wordID_, wrongSpellings_[wrongSpellings_.size() - 1].spelling_ );
            wrongSpellings_.pop_back(); // Remove from list
        }
    }
}

Speller::Speller()
    :difficulty_(1,9)
    {
        spellingRecord_.clear();
    }
    
Speller::Speller(unsigned int id, std::wstring name, Range difficulty, std::wstring avatarFilename,
                 IDList stars, IDList tags)
    :id_(id), name_(name), difficulty_(difficulty), avatarFilename_(avatarFilename),
     starList_(stars), tagList_(tags)
{
    spellingRecord_.clear();
    SetUpColours(); // Creates default colours.
    useAutoCapitals_ = true;
    useAutoDiacritics_ = true;
    useAnimatedFeedback_ = true;
}


wstring Speller::GetName() const{
    return name_;
}

Range& Speller::GetDifficulty(){
    return difficulty_;
}

IDList& Speller::GetWordList() {
    return wordList_;
}

IDList& Speller::GetStarList() {
    return starList_;
}

IDList& Speller::GetTagList() {
    return tagList_;
}

unsigned int Speller::GetID() const{
    return id_;
}

bool Speller::UseNaturalSpacing() const{
    return useNaturalLetterSpacing_;
}

bool Speller::UseWordBreakdown() const{
    return useWordBreakdown_;
}

bool Speller::UseAutoDiacritics() const{
    return useAutoDiacritics_;
}

bool Speller::UseAutoCapitals() const{
    return useAutoCapitals_;
}

bool Speller::UseAnimatedFeedback() const{
    return useAnimatedFeedback_;
}

Gdiplus::Color Speller::GetColour( int colourCode ) const{
    if( colourCode < PAPER || colourCode > SWAPPED )
        return Gdiplus::Color(0,0,0);
    return colours_[colourCode];
}

unsigned int Speller::GetWordAttempts( const int wordID ) const{
    if( spellingRecord_.empty() )
        return 0;
    SpellingRecord::const_iterator iter = spellingRecord_.find(wordID);
    if( iter == spellingRecord_.end() )
        return 0;
    return iter->second.GetAttempts();
}

unsigned int Speller::GetNumWrongWords( const int wordID ) const{
    if( spellingRecord_.empty() )
        return 0;
    SpellingRecord::const_iterator iter = spellingRecord_.find(wordID);
    if( iter == spellingRecord_.end() )
        return 0;
    return iter->second.GetNumWrongWords();
}

StringVec Speller::GetWrongWords( const int wordID ) const{
    SpellingRecord::const_iterator iter = spellingRecord_.find(wordID);
    return iter->second.GetWrongWords();
}

int Speller::GetWordLevel(const int wordID) const {
    if( spellingRecord_.empty() )
        return 4; // 4 is starting level for unattempted words.
    SpellingRecord::const_iterator iter = spellingRecord_.find( wordID );
    if( iter == spellingRecord_.end() )
        return 4;
    return iter->second.GetLevel();  // TODO: proper function
}

bool Speller::RecordExists( const int wordID ) const {
    return( spellingRecord_.count( wordID ) );
}

void Speller::CreateRecord( const int wordID ) {
    spellingRecord_[wordID] = Record(wordID, 1, 0);
}

void Speller::AddRecord( const int wordID, unsigned int attempts, int level ){
    spellingRecord_[wordID] = Record( wordID, attempts, level );
}

void Speller::IncreaseAttempts( const int wordID ) {
    if( RecordExists( wordID ) )
        spellingRecord_[wordID].AddAttempt();
}

void Speller::SetLevel( const int wordID, const int level ){
    if( RecordExists( wordID ) )
        spellingRecord_[wordID].SetLevel( level );
}

void Speller::AddWrongSpelling( int wordID, WrongSpelling& ws ){
    if( RecordExists( wordID ) )
        spellingRecord_[wordID].AddWrongSpelling( ws );  
}

void Speller::AddWrongSpelling( int wordID, WrongSpelling& ws, DBController* db ){
    if( RecordExists( wordID ) )
        spellingRecord_[wordID].AddWrongSpelling( ws, id_, db );
}

void Speller::SetColour(int colourCode, Gdiplus::Color& newColour ){
    if( colourCode < PAPER || colourCode > SWAPPED )
        return;
    colours_[colourCode] = newColour;
}

void Speller::UpdateWordList(WordBank &wordBank){
    wordList_.clear();
    for( WordBank::iterator iter = wordBank.begin(); iter != wordBank.end(); ++iter ){
        unsigned int diff = iter->second.GetDifficulty();
        if( diff >= difficulty_.mLow && diff <= difficulty_.mHigh ){ // Is the word in the difficulty range?
            // Check tags
            IDList tags = iter->second.GetTags();
            for( IDList::iterator tIter = tags.begin(); tIter != tags.end(); ++tIter ){
                if( find(tagList_.begin(), tagList_.end(), *tIter ) != tagList_.end() ){ // tag exists in speller taglist
                    wordList_.insert(iter->first);
                }
            }
        }
    }
}

void Speller::SetUpColours(){
    colours_.push_back( Gdiplus::Color(150,100,100,150) ); //Transparent PAPER
    colours_.push_back( Gdiplus::Color(255,255,0) ); // Breakdown 1
    colours_.push_back( Gdiplus::Color(255,0,255) ); // Breakdown 2
    colours_.push_back( Gdiplus::Color(0,255,255) ); // Breakdown 3
    colours_.push_back( Gdiplus::Color(150,150,150) ); // Breakdown 4
    colours_.push_back( Gdiplus::Color(32,64,128) ); // Breakdown 5
    colours_.push_back( Gdiplus::Color(0,0,0) ); // PEN
    colours_.push_back( Gdiplus::Color(0,150,0) ); // CORRECT
    colours_.push_back( Gdiplus::Color(200,0,0) ); // WRONG
    colours_.push_back( Gdiplus::Color(200,200,0) ); // MISSING
    colours_.push_back( Gdiplus::Color(200,0,200) ); // SWAPPED

}
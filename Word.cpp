// Word.cpp
#include "Word.h"
#include <map>
#include <algorithm>
#include <functional> //needed for equal_to

#include "Speller.h"
#include "ScreenPrinter.h"
#include "Utility.h"

using namespace std;

// Tag
Tag::Tag(unsigned int id, std::wstring name, bool active)
: id_(id), name_(name), active_(active)
{}

unsigned int Tag::GetID() const{
    return id_;
}

std::wstring Tag::GetName() const{
    return name_;
}

bool Tag::IsActive() const{
    return active_;
}

IDList& Tag::GetWords() {
    return wordIDList_;
}

void Tag::Toggle(){
    active_ = !active_;
}

void Tag::SetActive(bool active){
    active_ = active;
}

void Tag::SetName(std::wstring &name){
    name_ = name;
}

bool Tag::EqualToID( const unsigned int id ) const {
    return id_ == id;
}

void Tag::AddWordID( unsigned int wordID ){
    wordIDList_.insert( wordID );
}

unsigned int Tag::WordCount() const{
    return wordIDList_.size();
}

// BREAKDOWN
Breakdown::Breakdown()
    : position_(0), length_(0), colourNum_(0)
    {}
    
Breakdown::Breakdown(unsigned int position, unsigned int length, unsigned int colourNum)
    : position_(position), length_(length), colourNum_(colourNum)
    {}
    
Breakdown& Breakdown::operator =(const Breakdown &rhs){
    if( this == &rhs )
        return *this;
        
    this->position_ = rhs.position_;
    this->length_   = rhs.length_;
    this->colourNum_= rhs.colourNum_;
    
    return *this;
}
    
bool Breakdown::operator<( const Breakdown& rhs ) const{
    return position_ < rhs.position_;
}

// SPELLING
Spelling::Spelling(unsigned int id, std::wstring spelling)
    : id_(id), spelling_(spelling)
    {
        breakdownList_.clear();
    }

bool Spelling::operator==( const Spelling& rhs ){
    return spelling_ == rhs.spelling_;
}


bool Spelling::EqualToID( const unsigned int id ) const {
    return id_ == id;
}

unsigned int Spelling::GetID() const{
    return id_;
}

wstring Spelling::GetSpelling() const {
    return spelling_;
}

size_t Spelling::GetBreakdownListSize() const{
    return breakdownList_.size();
}

void Spelling::GetBreakdownAtPosition( unsigned int position, Breakdown& breakdown )const {
    std::map<unsigned int, Breakdown>::const_iterator iter = breakdownList_.find(position);
    if( iter != breakdownList_.end() )
        breakdown = iter->second;
}

bool Spelling::AddBreakdown(unsigned int position, unsigned int length, unsigned int colourNum){
    // Insert the breakdown (returns false if position already exists).
    return breakdownList_.insert(
            make_pair(position, Breakdown(position, length, colourNum))).second;
            
}

// WORD
Word::Word(unsigned int id, unsigned int difficulty, bool confusable, unsigned int mainSpellingID)
    : id_(id), difficulty_(difficulty), confusable_(confusable), mainSpellingID_(mainSpellingID)
{}

unsigned int Word::GetID() const {
    return id_;
}

bool Word::IsConfusable() const {
    return confusable_;
}

unsigned int Word::GetDifficulty() const {
    return difficulty_;
}

wstring Word::GetMainSpellingString() const {
    SpellingList::const_iterator iter = find_if(spellingList_.begin(), spellingList_.end(), bind2nd(mem_fun_ref(&Spelling::EqualToID), mainSpellingID_));
    if (iter == spellingList_.end() )
        return L"";
    return iter->GetSpelling();
}

const Spelling& Word::GetMainSpelling() const{
    SpellingList::const_iterator iter = find_if(spellingList_.begin(), spellingList_.end(),
                                                bind2nd(mem_fun_ref(&Spelling::EqualToID), mainSpellingID_));
    return *iter; // TODO: what if iter = spellingList_.end() ?
}

const SpellingList& Word::GetSpellings() const{
    return spellingList_;
}

bool Word::AddSpelling(Spelling spelling){
    SpellingList::iterator iter = find(spellingList_.begin(), spellingList_.end(), spelling);
    if( iter != spellingList_.end() ) // This spelling is already present
        return false;                 // Do not add!
    
    spellingList_.push_back(spelling); // Add unique spelling.
    return true;
}

bool Word::AddBreakdown(unsigned int spellingID, unsigned int position, unsigned int length, unsigned int colourNum){
    // Does a spelling with a matching ID exist in this Word's spelling List?
    SpellingList::iterator iter = find_if(spellingList_.begin(), spellingList_.end(), bind2nd(mem_fun_ref(&Spelling::EqualToID),spellingID));
    // If not, return false
    if (iter == spellingList_.end() )
        return false;
    // Otherwise, add the Breakdown details to that spelling.    
    iter->AddBreakdown(position, length, colourNum);
    return true;
}

void Word::AddTagID(unsigned int tagID){
    tagIDList_.insert( tagID );
}

bool Word::EqualToID( const unsigned int id ) const {
    return id_ == id;
}

IDList& Word::GetTags(){
    return tagIDList_;
}

bool Word::HasAudio() const {
    return speechList_.size() > 0;
}

bool Word::HasContext() const {
    return contextList_.size() > 0;
}

bool Word::SoundOnlyOK() const {
    return ( HasAudio() && HasContext() ||
             HasAudio() && !confusable_ );
}

// ANALYSEDLETTER
AnalysedLetter::AnalysedLetter(wchar_t letter, LetterStatus status)
: letter_(letter), status_(status){}

//ANALYSEDWORD
AnalysedWord::AnalysedWord(unsigned int numLetters)
: originalLength_(numLetters), score_(-1), analysisState_(NA)
{}

bool AnalysedWord::SortOrder(const AnalysedWord &rhs){
    if( score_ != rhs.score_ )
        return score_ > rhs.score_;
    if( averageLinkSize_ != rhs.averageLinkSize_ )
        return averageLinkSize_ > rhs.averageLinkSize_;
        return largestLink_ > rhs.largestLink_;
        
    //1.	The highest Score is a better spelling.  If equal: 
    //2.	The largest average link length is better.  If equal:
    //3.	The least difference between length of target and attempt is better.  If equal:
    //4.	The largest "biggest link" is better.  If equal:
    //5.	The algorithm results / wrong spellings are deemed equal.  The newer result replaces the oldest.
    // PART 3 & 5 not dealt with by this function. Part 3 is irrelevant here, as it is only used to compare
    // AnalysedWords from the same attempt.
}

void AnalysedWord::Add(const AnalysedLetter &al){
    word_.push_back(al);
}

void AnalysedWord::Add(wchar_t letter, LetterStatus status){
    word_.push_back(AnalysedLetter(letter, status));
}

void AnalysedWord::Clear(){
    word_.clear();
}

// Variables accessors
unsigned int AnalysedWord::LengthDifference() const{
    return lengthDifference_;
}

double AnalysedWord::AverageLinkSize() const{
    return averageLinkSize_;
}

unsigned int AnalysedWord::LargestLink() const{
    return largestLink_;
}

unsigned int AnalysedWord::NumLinks() const{
    return numLinks_;
}

int AnalysedWord::Score() const{
    return score_;
}

AnalysedLetter& AnalysedWord::operator[]( unsigned int i ){
    if( i >= word_.size() || i < 0 )
        return AnalysedLetter(L'', Null);
    return word_[i];
}

void AnalysedWord::RemoveNull(){
    word_.erase( remove_if(word_.begin(), word_.end(), IsNull() ), word_.end() );
}

void AnalysedWord::Reverse(){
    reverse( word_.begin(), word_.end() );
}

bool AnalysedWord::empty() const{
    return word_.empty();
}

size_t AnalysedWord::size() const{
    return word_.size();
}



void AnalysedWord::CalculateStats(){
    CalculateScore();
    CalculateNumLetters();
    CalculateLinks();
}

int AnalysedWord::NumCorrect() const{
    return CountStatus( Correct );
}

//TODO: Not sure if this definitely works, but it's only used with 2 and 3 letter words currently.
// Should check it's valid.  It uses similar algorithm to the score calculator.
int AnalysedWord::NumErrors() const{
    // Each swapped pair is 1 error.
    // Each three away is 1 error.
    // Each matching pair of missing/wrong (adjacent groups) is 1 error
    // Each extra wrong or missing is 1 error.
    int errors = 0;
    int countA = 0;
    int countB = 0;
    LetterStatus currentType = Null;
    LetterStatus lastType = Null;
    
    for(AWConstIter i = word_.begin(); i != word_.end(); ++i){

        switch( i->status_ ){
            case Correct:
            case ThreeAwayMissingF:
            case ThreeAwayMissingB: 
            case Missing:
            case Wrong:{
                // Do nothing.
                // Missing and Wrong dealt with elsewhere
                // Only count ThreeAwayWrong, so the error isn't counted twice.
                break;
            }
            case Swapped:{
                ++errors;
                ++i; // advance iterator to skip next swapped
                break;
            }
            
            case ThreeAwayWrongF:
            case ThreeAwayWrongB: {
                ++errors;
                break;
            }
            default:{
                break;
            }
        }//switch
        if( currentType != i->status_ ){ // If the type is different to the type parsed previously...
            lastType = currentType;      // ...make a copy of the previous type...
            currentType = i->status_;    // ...and store the new type.
            
            if( countA > 0 && countB > 0 ){ // Has there been a string of Missing and Wrong?
                errors += (countA > countB ? countA : countB ); // Add the number of the largest group
            }
            if( currentType != Wrong && currentType != Missing ){
                countA = 0;
                countB = 0;
            }
        }
        if( currentType != lastType ){ // This type is different from the type we've parsed previously.
            if( currentType == Wrong ){ // If it's of type Wrong...
                if( lastType == Missing ){ // ...and previous was Missing...
                    ++countB;              // ...increase the second counter.
                } else {                   
                    ++countA;              // Otherwise increase the first counter.
                }
            } else if( currentType == Missing ){
                if( lastType == Wrong ) {
                    ++countB;
                } else {
                    ++countA;
                }
            }
        }  
    } // for loop
    // Last check for any final missing/wrong letters
    if( countA > 0 && countB > 0 ){ // Has there been a string of Missing and Wrong?
        errors += countA > countB ? countA : countB;
        countA = 0;
        countB = 0;
    }
    return errors;
}

void AnalysedWord::CalculateScore(){
    int total = originalLength_ * 10;
    
    int countA = 0;
    int countB = 0;
    LetterStatus currentType = Null;
    LetterStatus lastType = Null;
    
    for(AWiter i = word_.begin(); i != word_.end(); ++i){

        switch( i->status_ ){
            case Correct:
            case ThreeAwayMissingF:
            case ThreeAwayMissingB:{
                // Do nothing.
                break;
            }
            case Swapped:{
                total -= 3;
                break;
            }
            
            case ThreeAwayWrongF:
            case ThreeAwayWrongB: {
                total -= 6;
                break;
            }
            case Missing:
            case Wrong:
            {
                total -= 10;
            }
            default:{
                break;
            }
        }//switch
        if( currentType != i->status_ ){ // If the type is different to the type parsed previously...
            lastType = currentType;      // ...make a copy of the previous type...
            currentType = i->status_;    // ...and store the new type.
            
            if( countA > 0 && countB > 0 ){ // Has there been a string of Missing and Wrong?
                if( countA < countB ){
                    total += (11 * countA);
                } else {
                    total += (11*countB);
                }
            }
            // TODO: Is it possible to have Missing - Wrong - Missing or similar?
            // That is, to have a string of missing, followed by wrong, followed by missing, or vice versa?
            if( currentType != Wrong && currentType != Missing ){
                countA = 0;
                countB = 0;
            }
        }
        if( currentType != lastType ){ // This type is different from the type we've parsed previously.
            if( currentType == Wrong ){ // If it's of type Wrong...
                if( lastType == Missing ){ // ...and previous was Missing...
                    ++countB;              // ...increase the second counter.
                } else {                   
                    ++countA;              // Otherwise increase the first counter.
                }
            } else if( currentType == Missing ){
                if( lastType == Wrong ) {
                    ++countB;
                } else {
                    ++countA;
                }
            }
        }  
    } // for loop
    // Last check for any final missing/wrong letters
    if( countA > 0 && countB > 0 ){ // Has there been a string of Missing and Wrong?
        if( countA < countB ){
            total += (11*countA);
        } else {
            total += (11*countB);
        }
        countA = 0;
        countB = 0;
    }
    score_ = total;
}

std::wstring AnalysedWord::GetString() const{
    wstring s;
    for(AWConstIter iter = word_.begin(); iter != word_.end(); ++iter ){
        switch( iter->status_ ){
            case Correct:
            case Wrong:
            case Swapped:
            case ThreeAwayWrongF:
            case ThreeAwayWrongB: {
                s += iter->letter_;
                break;
            }
            default: {
                break;
            }
        }// end switch
    }// End for
    return s;
}

std::vector<AnalysedLetter> AnalysedWord::GetAnalysis() const{
    return word_;
}

void AnalysedWord::CalculateNumLetters(){
    // Total up number of correct, wrong, swapped and threeawaywrong
    // Don't count missing (not in the attempt) and threeawaymissing (already counted as threeawaywrong)
    attemptLength_ = 0;
    for(AWiter iter = word_.begin(); iter != word_.end(); ++iter ){
        switch( iter->status_ ){
            case Correct:
            case Wrong:
            case Swapped:
            case ThreeAwayWrongF:
            case ThreeAwayWrongB: {
                ++attemptLength_;
                break;
            }
            default: {
                break;
            }
        }// end switch
    }// End for
    
    // Calculate lengthDifference_
    lengthDifference_ = abs(static_cast<int>(attemptLength_) - static_cast<int>(originalLength_));
}

void AnalysedWord::CalculateLinks(){
    averageLinkSize_ = 0;
    largestLink_ = 0;
    numLinks_ = 0;
    int currentLinkSize = 0;
    for(AWiter iter = word_.begin(); iter != word_.end(); ++iter ){
        
        switch( iter->status_ ){
            case Correct:{
                if( currentLinkSize == 0 ){ // No link being counted
                    ++numLinks_;                    
                }
                ++currentLinkSize;
                break;
            }
            default:{
                if( currentLinkSize > 0 ){
                    if( currentLinkSize > largestLink_ ){
                        largestLink_ = currentLinkSize;
                    }
                    currentLinkSize = 0;
                }
                break;
            }
        } // End switch
    }// End for
    
    // Avoid divide by zero problems.  Set average to zero in these circumstances.
    if( numLinks_ == 0 )
        averageLinkSize_ = 0;
    else
        averageLinkSize_ = static_cast<double>( CountStatus( Correct ) ) / static_cast<double>( numLinks_ ) ;
}

int AnalysedWord::CountStatus(LetterStatus status) const{
     unsigned int countS = 0;
     for(AWConstIter iter = word_.begin(); iter != word_.end(); ++iter ){
        if( iter->status_ == status ){
            ++countS;
        }
     }
     return countS;
}

void AnalysedWord::SetAnalysisState(const AnalysisState &as){
    analysisState_ = as;
}

bool AnalysedWord::IsExact() const{
    return analysisState_ == EXACT;
}

bool AnalysedWord::IsCorrect() const{
    return analysisState_ == EXACT || analysisState_ == ALTSPELLING;
}

bool AnalysedWord::IsAlternateSpelling() const{
    return analysisState_ == ALTSPELLING;
}

bool AnalysedWord::IsBeyondWrong() const{
    return  analysisState_ == BEYONDWRONG1 ||
            analysisState_ == BEYONDWRONG2 ||
            analysisState_ == BEYONDWRONG3;
}

// SPELLINGANALYSER
SpellingAnalyser::SpellingAnalyser(const std::wstring& attempt, const Word* word, Speller& speller,
                     AnalysedWord& analysedWord)
: attempt_(attempt), pWord_(word), speller_(speller),
  analysedWord_(analysedWord)
{
    // Compare the attempt with the original spelling
    attCopy_ = ApplyOptionsToString( attempt_ );
    spelling_ = pWord_->GetMainSpellingString();
    speCopy_ = ApplyOptionsToString( spelling_ );
    if( !ExactMatch() ){
        // Start comparison algorithms
        vector<AnalysedWord> analyses; // Store results of each analysis
        // For reverse analysis
        wstring revAttempt = attempt_;
        reverse( revAttempt.begin(), revAttempt.end() );
        wstring revAttemptProcessed = ApplyOptionsToString( revAttempt );
        wstring revSpelling = spelling_;
        reverse( revSpelling.begin(), revSpelling.end() );
        wstring revSpellingProcessed = ApplyOptionsToString( revSpelling );        
        unsigned int distance;
        // For each distance ( no. letters in attempt - 2 )
        for(distance = attempt_.length(); distance >= 2; --distance){
            analysedWord_.Clear();
            //Pattern Matching
            PatternMatching( distance, attCopy_, attempt_, speCopy_, spelling_, analysedWord_ );            
            // FindSwaps (length of original spelling > 2? letters)
            if( spelling_.length() > 2 )
                SwapSearch( analysedWord_ );
            // ThreeAway (length of original spelling > 3? letters)
            if( spelling_.length() > 3 )
                ThreeAwaySearch( analysedWord_ );
            //Store AnalysedWord
            analyses.push_back( analysedWord_ );
                
            // Repeat in REVERSE
            analysedWord_.Clear();
            PatternMatching( distance, revAttemptProcessed, revAttempt, revSpellingProcessed, revSpelling, analysedWord_ );
            if( spelling_.length() > 2 )
                SwapSearch( analysedWord_ );
            // Re-reverse analysedWord results
            analysedWord_.Reverse();
            if( spelling_.length() > 3 )
                ThreeAwaySearch( analysedWord_ );

            //Store AnalysedWord
            analyses.push_back( analysedWord_ );    
            
            // Pattern Matching with Swaps (length of original spelling > 2? letters)
            analysedWord_.Clear();
            PatternMatching( distance, attCopy_, attempt_, speCopy_, spelling_, analysedWord_, true );
            // FindSwaps (length of original spelling > 2? letters)
            if( spelling_.length() > 2 )
                SwapSearch( analysedWord_ );
            // ThreeAway (length of original spelling > 3? letters)
            if( spelling_.length() > 3 )
                ThreeAwaySearch( analysedWord_ );
            // Store AnalysedWord
            analyses.push_back( analysedWord_ ); 
            // Repeat in REVERSE
            analysedWord_.Clear();
            PatternMatching( distance, revAttemptProcessed, revAttempt, revSpellingProcessed, revSpelling, analysedWord_, true );
            if( spelling_.length() > 2 )
                SwapSearch( analysedWord_ );
            // Re-reverse analysedWord results
            analysedWord_.Reverse();
            if( spelling_.length() > 3 )
                ThreeAwaySearch( analysedWord_ );

            //Store AnalysedWord
            analyses.push_back( analysedWord_ );    
        }
        // Select Best AnalysedWord
        for_each(analyses.begin(), analyses.end(), mem_fun_ref(&AnalysedWord::CalculateStats));
        sort(analyses.begin(), analyses.end(), mem_fun_ref( &AnalysedWord::SortOrder ) );
        // Select the first item as the analysed Word choice.
        analysedWord_.Clear();
        analysedWord_ = analyses[0];
        // Now check for Special Cases: Beyond Wrong.  If a special case, set a flag to this effect.
        CheckBeyondWrong();        
    }
}

std::wstring SpellingAnalyser::ApplyOptionsToString( const std::wstring s ){
    if( speller_.UseAutoDiacritics() ){
        if( speller_.UseAutoCapitals() ){
            return ToLower( s, true ); // lose caps and diacritics
        }
        return RemoveDiacritics(s); // keep caps but lose diacritics
    }
    
    if( speller_.UseAutoCapitals() ){ // keep diacritics but lose caps
        return ToLower( s );
    }
    
    return s; // keep diacritics and caps
}

bool SpellingAnalyser::ExactMatch(){
    // TODO: Make a note if an alternative spelling is used
    if( attCopy_ == speCopy_ ){
        // Construct AnalysedWord out of spelling_
        ConstructAnalysedWordFromSpelling( spelling_ );
        analysedWord_.SetAnalysisState( EXACT );
        return true;
    }
    // No match, so try each alternative spelling.
    SpellingList sList = pWord_->GetSpellings();
    for( SpellingList::iterator iter = sList.begin();
         iter != sList.end();
         ++iter ){
         
        if( iter->GetSpelling() == pWord_->GetMainSpellingString() ) // Don't bother with main spelling again.
            continue;
        wstring copy = ApplyOptionsToString( iter->GetSpelling() );
        if( attCopy_ == copy ){
            // Construct AnalysedWord out of *iter.
            ConstructAnalysedWordFromSpelling( iter->GetSpelling() );
            // Set Special Case flag
            analysedWord_.SetAnalysisState( ALTSPELLING );
            return true;
        }
    }
    return false;
}

void SpellingAnalyser::PatternMatching( const unsigned int distance,
                          const std::wstring& attemptProcessed, const std::wstring& attempt,
                          const std::wstring& spellingProcessed, const std::wstring& spelling,
                          AnalysedWord& aw, bool swapOnTheFly ){
    // Set the beginning of the pattern range for the target copy to the first letter.
    wstring::size_type patternStart = 0;
    
    wstring::size_type length = spellingProcessed.length() - patternStart;// Set the length of the pattern to be searched for.
    wstring pattern = L"";
    // Set the beginning of the search to the first letter of the attempt copy.
    wstring::size_type searchPosition = 0;
    // Find the 1st occurrence of the current pattern in the search range.
    wstring::size_type location;
    while(true){
        // If no pattern (no letters) left:
        if( length == 0 ){
            // Any remaining letters in the attempt are WRONG
            wstring wrongLetters = attempt.substr( searchPosition );
            FillAnalysedWord( wrongLetters, Wrong );
            return;
        }
        pattern = spellingProcessed.substr(patternStart, length);
        location = attemptProcessed.find(pattern, searchPosition );
        // If found:
        if( location != wstring::npos && location <= distance ){
            // Check for preceding letters in the attempt copy.
            wstring wrongLetters = attempt.substr(searchPosition, location - searchPosition);
            // If any:
            if( !wrongLetters.empty() ){
                // Record these letters as WRONG.
                FillAnalysedWord( wrongLetters, Wrong );
            }
            // Mark the range (using the target original) as CORRECT.
            wstring correctLetters = spelling.substr(patternStart, length);
            FillAnalysedWord( correctLetters, Correct );
            // Set the beginning of the pattern range to the next letter after the end of the located pattern.
            patternStart += length;
            // If none:
            if( patternStart >= speCopy_.length() ){
                // Check for any remaining letters in the attempt copy.
                wrongLetters = attempt.substr( location + length );
                // If any:
                if( !wrongLetters.empty() ){
                    // Record these letters (using the attempt original) as WRONG.
                    FillAnalysedWord( wrongLetters, Wrong );
                }
                return;
            }
             // Set the beginning of the search to the first letter after the located pattern.
            searchPosition = location+length;

            // If none:
            if( searchPosition >= attemptProcessed.length() ){
                // Check for any remaining letters in the target copy.
                wstring missingLetters = spelling.substr(patternStart);
                // If any:
                if( !missingLetters.empty() ){
                    // Record these letters (using the target original) as MISSING.
                    FillAnalysedWord( missingLetters, Missing );
                }
                return;
            }
            length = spellingProcessed.length() - patternStart; // Calculate new length
        } else { // NOT FOUND: 
            if ( length > 2 ){ // If pattern length is greater than two letters:
                // Reduce pattern length by one and continue.
                --length;
                continue;
            }
            // If pattern length is one letter only:
            if( length == 1 ){
            // Record this letter (using the target original) as MISSING.
                wstring missingLetter = spelling.substr(patternStart, 1);
                FillAnalysedWord( missingLetter, Missing );
                // Advance pattern
                ++patternStart;
                length = spellingProcessed.length() - patternStart;
                continue;
            }
            // Pattern must therefore be exactly two letters:
            // SWAP ON THE FLY SHOULD HAPPEN HERE
            if( swapOnTheFly ){
                // Swap the pair of letters around
                reverse( pattern.begin(), pattern.end() );
                location = attemptProcessed.find(pattern, searchPosition );

                if( location != wstring::npos && location <= distance ){
                    // Found reversed pattern, so follow almost the same process as finding correct letters
                    // Check for preceding letters in the attempt copy.
                    wstring wrongLetters = attempt.substr(searchPosition, location - searchPosition);
                    // If any:
                    if( !wrongLetters.empty() ){
                        // Record these letters as WRONG.
                        FillAnalysedWord( wrongLetters, Wrong );
                    }
                    // Record Swapped letters
                    // (Need to get the letters from the original spelling and reverse them.)
                    wstring lettersFromOriginal = spelling_.substr(patternStart,2);
                    reverse( lettersFromOriginal.begin(), lettersFromOriginal.end() );
                    FillAnalysedWord( lettersFromOriginal, Swapped );
                    // Advance pattern to after the swapped letters
                    patternStart += 2;
                    // Set search position and length
                    searchPosition = location + length;
                    length = spellingProcessed.length() - patternStart;
                    continue;
                }
                // Not found, so put pattern back as it was before continuing rest of algorithm
                reverse( pattern.begin(), pattern.end() );                
            }
            // Look at first letter only.
            wchar_t firstLetterOfPattern = pattern[0];
            // If this is the initial letter of the target original
            if( firstLetterOfPattern == spellingProcessed[0] ||
                (patternStart > 1 && spelling[patternStart-1] == L' ') ){// OR if the letter immediately before this in the target original is a space,
                // reduce the pattern length by one and continue
                --length;
                continue;
            }
            // See this letter is unique in the remainder of the target copy,
            wstring remainder = spellingProcessed.substr(patternStart);
            if( count(remainder.begin(), remainder.end(), firstLetterOfPattern) == 1 ){
                // It is, so reduce the pattern length by one and continue.
                --length;
                continue;
            }
            // Otherwise, record this letter (using the target original) as MISSING.
            wstring missingLetter = spelling.substr(patternStart, 1);
            FillAnalysedWord( missingLetter, Missing );
            ++patternStart;// Advance the pattern start by one
        } // End of section dealing with NOT found
    }// End While
}


// TODO: need to apply spelling options to this situation.
// If a letter has been marked wrong, it hasn't been converted to correct case / correct diacritic.
// Therefore, it doesn't match the character that's Missing.
// Need to check the "base" letters of the missing and wrong are equal, and store the missing one (which will be correctly
// cased and symbolled... nice new words there.)
void SpellingAnalyser::SwapSearch( AnalysedWord& aw ){
    // Swap Search patterns
    // Step 1, search for Patterns 1&2 and replace with A CORRECT
    // Pattern 1: A WRONG / A MISSING
    // Pattern 2: A MISSING / A WRONG
    for( int i = 0; i < aw.size() - 1; ++i) {
        if( aw[i].letter_ == aw[i+1].letter_ &&
            ( (aw[i].status_ == Wrong && aw[i+1].status_ == Missing ) ||
              (aw[i].status_ == Missing && aw[i+1].status_ == Wrong ) ) ){
            aw[i].status_ = Null;
            aw[i+1].status_ = Correct;
            ++i; // extra advance to move past second letter
        }
    }
    aw.RemoveNull(); // Clean up any "deleted" AnalysedLetters
    // Step 2, search for Patterns 3 & 4, remove MISSING and replace WRONG and CORRECT with Swapped
    // Pattern 3: A MISSING / B CORRECT / A WRONG
    // Pattern 4: A WRONG / B CORRECT / A MISSING
    for( int i = 0; i < aw.size() - 2; ++i) {
        if( ((aw[i].status_ == Wrong && aw[i+2].status_ == Missing) ||
             (aw[i].status_ == Missing && aw[i+2].status_ == Wrong) ) &&
             ( aw[i+1].status_ == Correct ) &&
             ( aw[i].letter_ == aw[i+2].letter_ ) )
        {
            aw[i+1].status_ = Swapped;
            if( aw[i].status_ == Missing ){
                aw[i].status_ = Null;
                aw[i+2].status_ = Swapped;
            } else {
                aw[i+2].status_ = Null;
                aw[i].status_ = Swapped;
            }
        }
    }
    aw.RemoveNull();
}

void SpellingAnalyser::ThreeAwaySearch( AnalysedWord& aw ){
    for( int i = 0; i < aw.size() - 3; ++i){
        if( ( aw[i].letter_ == aw[i+3].letter_ ) &&
            ( (aw[i].status_ == Missing && aw[i+3].status_ == Wrong) || (aw[i].status_ == Wrong && aw[i+3].status_ == Missing) ) &&
            ( aw[i+1].status_ == aw[i+2].status_ ) &&
            ( ( aw[i+1].status_ == Correct || aw[i+1].status_ == Swapped ) ) ){
            
            if( aw[i].status_ == Wrong ){
                aw[i].status_ = ThreeAwayWrongF;
                aw[i+3].status_ = ThreeAwayMissingB;
            } else {
                aw[i].status_ = ThreeAwayMissingF;
                aw[i+3].status_ = ThreeAwayWrongB;
            }      
        }
    }
}

void SpellingAnalyser::ConstructAnalysedWordFromSpelling( const wstring s ){
    for( wstring::const_iterator iter = s.begin();
         iter != s.end();
         ++iter ){
        analysedWord_.Add( AnalysedLetter( *iter ) );
    }
}

void SpellingAnalyser::FillAnalysedWord(const std::wstring s, const LetterStatus stat){
    for( wstring::const_iterator iter = s.begin();
         iter != s.end();
         ++iter ){
        analysedWord_.Add( *iter, stat );
    }
}

void SpellingAnalyser::CheckBeyondWrong(){
    // RULE SET 1 (ALL the following must apply):
    // Difference between letters in Target and letters in Attempt > ?3?
    // Average chain link < {#letters of Target < 6? 2 : 3}
    if( analysedWord_.LengthDifference() > 3 ){
        size_t targetLength = spelling_.length();
        if( ( targetLength < 6 && analysedWord_.AverageLinkSize() < 2 ) ||
            ( analysedWord_.AverageLinkSize() < 3 ) )
        analysedWord_.SetAnalysisState( BEYONDWRONG1 );
        return;
    }
    
    // RULE SET 2 (ALL the following must apply):
    // No. of letters of Target > 3
    // No. of chain links <= 1
    // Average link <= 1
    if( spelling_.length() > 3 &&
        analysedWord_.NumLinks() <= 1 &&
        analysedWord_.AverageLinkSize() <= 1 ){
        analysedWord_.SetAnalysisState( BEYONDWRONG2 );
        return;
    }
    
    // RULE SET 3 – pointless extra letters (ALL the following must apply):
    // Difference between length of target and attempt >= 5
    // Correct = MAX
    if( analysedWord_.LengthDifference() >= 5 &&
        analysedWord_.CountStatus( Correct ) == spelling_.length() ){
        analysedWord_.SetAnalysisState( BEYONDWRONG3 );
        return;
    }
}

// WORDPRINTER
void WordPrinter::PrintWord(const Word *word, const Speller *speller,
                            ScreenPrinter *screenPrinter, BackBuffer *bb,
                            Gdiplus::Font *font, const Gdiplus::PointF position) const{

    // Paper should already be drawn by mode
    // Position may need to be calculated by this class
    // If Speller has "breakdown" option set, get first breakdown.
    const Spelling& spelling = word->GetMainSpelling();
    wstring spellingString = spelling.GetSpelling();
    
    Gdiplus::Color inkColour;
    Gdiplus::PointF workingPosition = position; // make a copy.
    
    // Start a loop from first letter to last letter of Word's main spelling.
    for( int i = 0; i < spellingString.size(); ++i ){
        Breakdown breakdown;
        Gdiplus::Color inkColour = speller->GetColour(Speller::PEN);
        // If Speller has "breakdown" option:
        if( speller->UseWordBreakdown() ){
            spelling.GetBreakdownAtPosition( i + 1, breakdown );
            // If current breakdown matches current letter (WARNING: zero-based vs. one-based)
            if( breakdown.position_ ){
                // Set colour to Speller's matching breakdown colour
                inkColour = speller->GetColour(breakdown.colourNum_);
                // Print EACH letter in breakdown using Speller's spacing setting // TODO:Spacing setting
                for( int j = i; j < i + breakdown.length_;){
                    workingPosition = screenPrinter->PrintLetter(spellingString[j], workingPosition, *font, inkColour);
                    ++j;
                    if( j > spellingString.size() )
                        break;
                }
                // Advance to next letter AFTER breakdown group, checking for overrun
                i += (breakdown.length_ - 1);
                if( i > spellingString.size() )
                    break;
            } else {
                workingPosition = screenPrinter->PrintLetter(spellingString[i], workingPosition, *font, inkColour);
            }
        } else {
        // else, print current letter with pen colour using Speller's spacing setting
        workingPosition = screenPrinter->PrintLetter(spellingString[i], workingPosition, *font, inkColour);
        }
            // Advance to next letter
    }
}

void WordPrinter::PrintAttempt(const std::wstring attempt,
                               const Speller *speller,
                               ScreenPrinter *screenPrinter, BackBuffer *bb,
                               Gdiplus::Font *font, const Gdiplus::PointF position) const{
    
    Gdiplus::Color inkColour = speller->GetColour(Speller::PEN);;
    Gdiplus::PointF workingPosition = position; // make a copy.
    
    for( int i = 0; i < attempt.size(); ++i ){
        workingPosition = screenPrinter->PrintLetter(attempt[i], workingPosition, *font, inkColour);
    }
}
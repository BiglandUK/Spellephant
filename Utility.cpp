// Utility.cpp
#include "Utility.h"
#include <deque>
#include <algorithm>
#include <limits>
#include "Speller.h"
using namespace std;

bool ltwstr:: operator()(const std::wstring s1, std::wstring s2) const
  {
    return s1.compare(s2) < 0;
  }

bool ClickInRegion(const Gdiplus::PointF* clickPoint,
                   const Gdiplus::PointF* regionStart,
                   const float regionWidth, const float regionHeight) {

    return( clickPoint->X >= regionStart->X &&
        clickPoint->Y >= regionStart->Y &&
        clickPoint->X <= regionStart->X + regionWidth &&
        clickPoint->Y <= regionStart->Y + regionHeight );
}

const std::wstring trim(const std::wstring& pString,
                       const std::wstring& pWhitespace)
{
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == std::wstring::npos)
    {
        // no content
        return L"";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}

const std::wstring reduce(const std::wstring& pString,
                         const std::wstring& pFill,
                         const std::wstring& pWhitespace)
{
    // trim first
    std::wstring result(trim(pString, pWhitespace));

    // replace sub ranges
    size_t beginSpace = result.find_first_of(pWhitespace);
    while (beginSpace != std::wstring::npos)
    {
        const size_t endSpace =
                        result.find_first_not_of(pWhitespace, beginSpace);
        const size_t range = endSpace - beginSpace;

        result.replace(beginSpace, range, pFill);

        const size_t newStart = beginSpace + pFill.length();
        beginSpace = result.find_first_of(pWhitespace, newStart);
    }

    return result;
}

std::wstring ToUpper( std::wstring s, const std::locale loc )
{
  for (std::wstring::iterator p = s.begin(); p != s.end(); ++p)
    if( *p < 192 || *p > 223){
        *p -= 32;    
    } else {
        *p = toupper( *p, loc );
    }
    
  return s;
}

wchar_t ToUpper( wchar_t c, bool stripDiacritic, const std::locale loc ){
    if( stripDiacritic )
        c = RemoveDiacritic(c);
    if( c < 192 || c > 223){
        c -= 32;    
    } else {
        c = toupper( c, loc );
    }
    return c;
}

std::wstring ToLower( std::wstring s, bool stripDiacritic , const std::locale loc)
{
  for (std::wstring::iterator p = s.begin(); p != s.end(); ++p){
    if( stripDiacritic)
        *p = RemoveDiacritic(*p);
    if( *p >= 192 && *p <= 223){
        *p += 32;    
    } else {
        *p = tolower( *p, loc );
    }
  }
  return s;
}

wchar_t ToLower( wchar_t c, bool stripDiacritic, const std::locale loc ){
    if( stripDiacritic )
        c = RemoveDiacritic(c);
    if( c >= 192 && c <= 223){
        c += 32;    
    } else {
        c = tolower( c, loc );
    }
    return c;
}



wchar_t RemoveDiacritic( const wchar_t c ){
    switch(c){
        case L'à':
        case L'á':
        case L'ä':
        case L'â':
        case L'å':
        case L'ã': {
            return L'a';
            break;
        }
        case L'À':
        case L'Á':
        case L'Ä':
        case L'Â':
        case L'Å':
        case L'Ã': {
            return L'A';
            break;
        }
        case L'è':
        case L'é':
        case L'ë':
        case L'ê': {
            return L'e';
            break;
        }
        case L'È':
        case L'É':
        case L'Ë':
        case L'Ê': {
            return L'E';
            break;
        }
        case L'ì':
        case L'í':
        case L'ï':
        case L'î': {
            return L'i';
            break;
        }
        case L'Ì':
        case L'Í':
        case L'Ï':
        case L'Î': {
            return L'I';
            break;
        }
        case L'ò':
        case L'ó':
        case L'ô':
        case L'ö':
        case L'õ': {
            return L'o';
            break;
        }
        case L'Ò':
        case L'Ó':
        case L'Ô':
        case L'Ö':
        case L'Õ': {
            return L'O';
            break;
        }
        case L'ù':
        case L'ú':
        case L'ü':
        case L'û': {
            return L'u';
            break;
        }
        case L'Ù':
        case L'Ú':
        case L'Ü':
        case L'Û': {
            return L'U';
            break;
        }
        case L'ç': {
            return L'c';
            break;
        }
        case L'Ç': {
            return L'C';
            break;
        }
        case L'ñ': {
            return L'n';
            break;
        }
        case L'Ñ': {
            return L'N';
            break;
        }
        case L'ÿ': {
            return 'y';
            break;
        }
        case L'Ÿ': {
            return L'Y';
            break;
        }
        default: {
            return c;
            break;
        }
    }
}

wstring RemoveDiacritics( const wstring s ){
    wstring temp = s;
    for( wstring::iterator i = temp.begin(); i != temp.end(); ++i ){
        *i = RemoveDiacritic( *i );
    }
    return temp;
}

//template <typename T>
//static bool deleteAll( T* theElement ) { delete theElement; return true; }

// FixedQueue
FixedQueue::FixedQueue(size_t maxSize, bool unique)
: maxSize_(maxSize), unique_(unique)
{}

unsigned int FixedQueue::Add(unsigned int newItem){
    if( maxSize_ == 0 ) return 0; //Don't add anything if size is zero.
    
    if(unique_ && IsInList(newItem) ) // Nothing added if item is already in list and only unique values allowed.
        return 0;
    
    queue_.push_front(newItem); // Add value.
    
    if( queue_.size() > maxSize_ ){
        unsigned int lastValue = queue_.back();
        queue_.pop_back();
        return lastValue; // return value leaving queue.
    }
    
    return 0; // no value to return
}

bool FixedQueue::IsInList(unsigned int newItem){
    return ( find(queue_.begin(), queue_.end(), newItem) != queue_.end() );
}

void FixedQueue::ChangeSize(size_t newSize ){
    if( maxSize_ == newSize ) return;
    if( maxSize_ > newSize )
        while( queue_.size() > newSize )
            queue_.pop_back();
    maxSize_ = newSize;
    
}

bool SelectionID::operator==(const unsigned int& id) const{
    return id_ == id;
}

// Functions for Weighting Calculator
int LevelWeighting( const int level ){
    if( level < 1 ) return 50;      // All negative levels
    if( level < 4 ) return 25;      // 1-3
    if( level < 6 ) return 10;      // 4-5
    if( level < 8 ) return 5;       // 6-7
    if( level < 10 ) return -5;     // 8-9
    if( level < 12 ) return -10;    // 10-11
    if( level < 15 ) return -25;    // 12-14
    return -50;                     // 15+
}

int FewestAttempts( const IDList& wordIDs, const Speller& speller ){
    if( wordIDs.empty() ) { return 0; }
    
    int fewest = INT_MAX; // Set to highest possible value.
    
    for( IDList::const_iterator iter = wordIDs.begin();
         iter != wordIDs.end();
         ++iter ){
        int attempts = speller.GetWordAttempts(*iter);    
        if( attempts < fewest )
            fewest = attempts;
    }
    return fewest;
}

/*
The Weighting Calculator works as follows:
1) From the list of word IDs currently being used by the Mode, find the fewest number of attempts
by the Speller.  This might be zero.
2) Subtract 1 from this number.  This is the "attemptsDeduction".  It might be -1.  The purpose of attemptsDeduction
is to "drag" all attempts equally so that the smallest #attempts is 1.
3) Going through the list of word IDs, create a Selection ID for populating the WorkingList that will
be used by the Mode.  Default values are the word ID, enabled = true and a temporary Weighting calculated thus:
    MULTIPLY:
    (a)the speller's no. of attempts for THIS word (minus the "attemptsDeduction" - which might increase
    the value by 1 if attemptsDeduction is -1)
    BY
    b) the basic LevelWeighting for the speller's current Level for THIS word. (the LevelWeighting is set
    in the similarly named function).
4) Once done for all words, the lowest value from step 3 is taken and reduced by one.  This "lowestWeighting" is
used to "drag" all weights equally so that the smallest final weight is 1.

The workingList of ids and weights, plus the total weight of all words, is stored in the referenced parameters.
*/
void WeightingCalculator( const IDList& wordIDs, const Speller& speller, WorkingList& workingList,
                          unsigned int& totalWeighting ){
    int attemptsDeduction = FewestAttempts( wordIDs, speller ) - 1;
    
    int lowestWeighting = INT_MAX; // Set to highest possible value.
    
    for( IDList::const_iterator iter = wordIDs.begin();
        iter != wordIDs.end();
        ++iter ){   
        SelectionID sid;
        sid.id_ = *iter;
        sid.enabled_ = true;
        sid.weighting_ = ( speller.GetWordAttempts( *iter ) - attemptsDeduction ) 
                          *
                         (LevelWeighting( speller.GetWordLevel( *iter ) ) );
        workingList.push_back( sid );
        if( sid.weighting_ < lowestWeighting )
            lowestWeighting = sid.weighting_;
    }
    
    --lowestWeighting; // need to do this to prevent the weight adjustment reducing a weight to zero.
    totalWeighting = 0;
    for( WorkingList::iterator iter = workingList.begin();
         iter != workingList.end();
         ++iter ){
        iter->weighting_ -= lowestWeighting; // adjust the weighting
        totalWeighting += iter->weighting_; // total up the final weightings.
    }
}

Gdiplus::Color GetFadeColour( const Gdiplus::Color& start, const Gdiplus::Color& dest, double ratio ){
    BYTE rPen = start.GetRed();
    BYTE gPen = start.GetGreen();
    BYTE bPen = start.GetBlue();
           
    BYTE rTarget = dest.GetRed();
    BYTE gTarget = dest.GetGreen();
    BYTE bTarget = dest.GetBlue();

    if( ratio > 1.0 )
        ratio = 1.0;
    if( ratio < 0.0 )
        ratio = 0.0;

    return Gdiplus::Color(rPen + static_cast<BYTE>( (rTarget - rPen) * ratio ),
                 gPen + static_cast<BYTE>( (gTarget - gPen) * ratio ),
                 bPen + static_cast<BYTE>( (bTarget - bPen) * ratio ) );
}

// Lives
Lives::Lives(int start, int max)
    : currentLives_(start), maxLives_(max)
{}

Lives::Lives(int max)
    : currentLives_(max), maxLives_(max)
{}

bool Lives::IsAlive() const{
    return currentLives_ > 0;
}

void Lives::Kill() {
    currentLives_ = 0;
}

void Lives::Add(const int lives ){
    currentLives_ += lives;
    if( currentLives_ > maxLives_ ){
        ReturnToMax();
    }
}

void Lives::Subtract( const int lives ){
    currentLives_ -= lives;
    if( currentLives_ < 0 ){
        Kill();
    }
}

void Lives::ReturnToMax() {
    currentLives_ = maxLives_;
}

int Lives::CurrentLives() const{
    return currentLives_;
}

int Lives::MaxLives() const{
    return maxLives_;
}


//Utility.h
// Contains global functions
#ifndef UTILITY_H
#define UTILITY_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <locale>
#include <deque>
#include <vector>
#include "Definitions.h"

// Forward declaration
class Speller;

// UNUSED?
// std::wstring doesn't appear to have built-in less than comparison, so this does
// the job required by std::map.
struct ltwstr{
  bool operator()(const std::wstring s1, std::wstring s2) const;
};

bool ClickInRegion(const Gdiplus::PointF* clickPoint,
                   const Gdiplus::PointF* regionStart,
                   const float regionWidth, const float regionHeight);

// Removes spaces from beginning and end of a string,
// and removes multiple adjacent spaces from within a string
// These two functions taken from:
//http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
const std::wstring trim(const std::wstring& pString,
                       const std::wstring& pWhitespace = L" \t");

const std::wstring reduce(const std::wstring& pString,
                         const std::wstring& pFill = L" ",
                         const std::wstring& pWhitespace = L" \t");

// Turns diacritic letters into "normal" letters.
// Optionally used by ToLower for standard English alphabetising.
// This list must contain all diacritics.
wchar_t RemoveDiacritic( const wchar_t c );
std::wstring RemoveDiacritics( const std::wstring s );


// These functions adapted from http://www.cplusplus.com/faq/sequences/strings/case-conversion/
std::wstring ToUpper( std::wstring s, const std::locale loc = std::locale() );
wchar_t ToUpper( wchar_t c, bool stripDiacritic = false, const std::locale loc = std::locale() );
std::wstring ToLower( std::wstring s, bool stripDiacritic = false, const std::locale loc = std::locale() );
wchar_t ToLower( wchar_t c, bool stripDiacritic = false, const std::locale loc = std::locale() );

template <typename T>
static bool deleteAll( T* theElement )
 { delete theElement; return true; }
 
// The FixedQueue keeps a list of integers (could be made into a template) up to a fixed size
// then starts removing the oldest.
class FixedQueue{
public:
    FixedQueue(std::size_t maxSize, bool unique = true);
    
    unsigned int Add(unsigned int newItem);
    void Clear();
    bool IsInList(unsigned int searchItem);
    void ChangeSize(size_t newSize);

private:
    std::deque<unsigned int> queue_;
    std::size_t              maxSize_; // maximum size of queue.
    bool                     unique_; // if true, stored values must be unique.

};

// SelectionID combines a word id, selection weighting and simple check if disabled.
struct SelectionID{
    int id_;        // id of word
    int weighting_;    // likelihood of selection
    bool enabled_;  // whether the item can be selected or not
    
    bool operator==(const unsigned int& id) const; // Checks if id exists.  Used as predicate.
    
};

typedef std::vector<SelectionID> WorkingList;

// These GLOBAL FUNCTIONS are used in the calculations for Random Weighting.
int LevelWeighting( const int level ); // converts a spelling record level into an initial weighting.
int FewestAttempts( const IDList& wordIDs, const Speller& speller ); // Finds the least number of attempts
                                                         // the Speller has for the supplied list of word IDs.
                                                         
// Populates the supplied workingList with Weighted values.
void WeightingCalculator(const IDList& wordIDs, const Speller& speller, WorkingList& workingList, unsigned int& totalWeighting );

//Supply a start and destination colour, and a ratio of how far between them, and this function
// returns a colour partway (matching the ratio) between them.
Gdiplus::Color GetFadeColour( const Gdiplus::Color& start, const Gdiplus::Color& dest, double ratio );

// Lives class - tracks the number of lives something has.
class Lives{
public:
    Lives( int start, int max );
    Lives( int max );
    
    bool IsAlive() const; // No. of lives
    void Kill(); // Reduce to zero lives.
    
    void Add( const int lives ); // will only add to maxlives
    void Subtract ( const int lives ); // will only reduce to zero, not negative values
    
    void ReturnToMax(); // Set lives to maxlives.
    
    int CurrentLives() const;
    int MaxLives() const;
private:
    int currentLives_;
    int maxLives_;    
};



#endif // UTILITY_H

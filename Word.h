//Word.h
/*
    This header is for the following classes:
    Tag
    TagList
    Breakdown
    Spelling
    Word
    AnalysedLetter
    AnalysedWord
    SpellingAnalyser
    WordPrinter
    
*/
#ifndef WORD_H
#define WORD_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <list>
#include <set>
#include "Definitions.h"


//Forward Declarations
class ScreenPrinter;
class BackBuffer;
class Speller;

class Tag{
public:
    Tag(unsigned int id, std::wstring name, bool active = true);
    
    unsigned int GetID() const;
    std::wstring GetName() const;
    bool IsActive() const;
    IDList& GetWords();
    
    void Toggle(); // toggles active_ bool
    void SetActive(bool active = true); // sets tag to specified state
    
    void SetName( std::wstring& name ); // changes name - assumes the name is valid
    
    bool EqualToID( const unsigned int id ) const; // Function to see if id passed in is equal.  Used as predicate.
    void AddWordID( unsigned int wordID );
    
    unsigned int WordCount() const;
    
private:
    unsigned int id_;       // Database identifier
    std::wstring name_;     // Tag word
    bool active_;           // Whether this tag is currently active (for the Word List)
    IDList wordIDList_;     // List of words with this tag.
};

/*BREAKDOWN*/
// Determines how the word is shown in breakdown mode (groups of letters are displayed in certain colours)
struct Breakdown {
    Breakdown();
    Breakdown(unsigned int position, unsigned int length, unsigned int colourNum);
    
    Breakdown& operator=( const Breakdown& rhs ); // Assignment operator
    
    bool operator< (const Breakdown& rhs) const;
    bool operator==(const unsigned int& pos) const; // Checks if pos exists.  Used as predicate.

    unsigned int position_; // Where in the spelling this breakdown point begins
    unsigned int length_;   // How many characters are included in this breakdown
    unsigned int colourNum_;// The colour palette number to use for this breakdown
    
};

/*SPELLING*/
class Spelling{
public:
    Spelling(unsigned int id, std::wstring spelling);
    bool operator== (const Spelling& rhs );
    bool EqualToID( const unsigned int id ) const; // Function to see if id passed in is equal.  Used as predicate.
    
    unsigned int GetID() const;
    std::wstring GetSpelling() const;
    size_t GetBreakdownListSize() const;
    void GetBreakdownAtPosition( unsigned int position, Breakdown& breakdown ) const;
    bool AddBreakdown(unsigned int position, unsigned int length, unsigned int colourNum);
    
private:
    std::wstring spelling_;  // How the word is actually spelled
    unsigned int id_;        // Database identifier - also distinguishes alternative spellings
    std::map<unsigned int, Breakdown> breakdownList_; // set of breakdown objects
        
};

typedef std::list<Spelling> SpellingList;


/*WORD*/
class Word{
public:
    Word(unsigned int id, unsigned int difficulty, bool confusable, unsigned int mainSpellingID);
    
   
    unsigned int GetID() const; // Return Word ID
    std::wstring GetMainSpellingString() const; // Returns main spelling string
    const Spelling& GetMainSpelling() const; // Returns const ref to main Spelling object.
    const SpellingList& GetSpellings() const; // Returns ref to spellinglist object
    int          GetNumSpellings() const; // returns number of spellings
    
    bool IsConfusable() const;
    unsigned int GetDifficulty() const;
    
    std::wstring GetSpeech() const; // returns first speech filename (empty if none)
    std::wstring GetRandomSpeech() const; // returns random speech filename (empty if none)
    
    std::wstring GetContext() const; // returns first context filename (empty if none)
    std::wstring GetRandomContext() const; // returns random context (empty if none)
    
    bool AddSpelling(Spelling spelling); // returns false if spelling already exists
    bool AddBreakdown(unsigned int spellingID, unsigned int position, unsigned int length, unsigned int colourNum); // returns false if spelling does not exist
    void AddTagID(unsigned int tagID); // No bool return, as checks should be made prior to call.
    
    bool SetDifficulty(int difficulty = 1); // returns false if invalid difficulty
    
    bool EqualToID( const unsigned int id ) const; // Function to see if id passed in is equal.  Used as predicate.
    
    IDList& GetTags(); // returns REF to tag id list.
    
    bool HasAudio() const;
    bool HasContext() const;
    bool SoundOnlyOK() const;
    

private:
    unsigned int id_;           // Database identifier
    unsigned int difficulty_;   // Difficulty level
    bool confusable_;           // whether this word is easily confused with another
    unsigned int mainSpellingID_;// Which id form spelling list is the current main spelling
    SpellingList spellingList_; // list of spellings
    FileNameList speechList_;   // list of speech filenames
    FileNameList contextList_;  // list of context filenames
    IDList       tagIDList_;    // list of IDs of Tags for this word

};

enum LetterStatus{ Null, Correct, Missing, Wrong, Swapped,
                 ThreeAwayMissingF, ThreeAwayMissingB, ThreeAwayWrongF, ThreeAwayWrongB };
// NOTE: the ThreeAwayMissing F and B are required for the animated feedback.
// The F and B denote Front and Back of the threeaway group.  If the missing letter is first in the analysed letters
// container, it gets an F(ront); if the ThreeAwayWrong appears first in the order, it gets a B(ack).
// This aids the AnimatedFeedback object to know whether a particular ThreeAwayMissing letter has already been
// animated, and should therefore be displayed in appropriate colours.
// IMPORTANT - this means the ThreeAwaySearch MUST be done in forward (not reverse) order.

// Stores a letter with its status after it has been Analysed (by Spelling Analyser)
struct AnalysedLetter{
    
    AnalysedLetter( wchar_t letter, LetterStatus status = Correct);
    wchar_t         letter_;
    LetterStatus    status_;

};

typedef std::vector<AnalysedLetter> AnalysedLetters;
typedef AnalysedLetters::iterator AWiter;
typedef AnalysedLetters::const_iterator AWConstIter;

// Helper class to check null status of AnalysedLetters.
class IsNull
{
public:
    bool operator() (const AnalysedLetter& l) const
    {
        return l.status_ == Null;
    }
};

enum AnalysisState{ NA, EXACT, ALTSPELLING, BEYONDWRONG1, BEYONDWRONG2, BEYONDWRONG3};

class AnalysedWord{
public:
    
    AnalysedWord(unsigned int numLetters);
    
    bool SortOrder(const AnalysedWord &rhs);
    
    // Add AnalysedLetters to mWord.
    void Add(const AnalysedLetter& al);
    void Add(wchar_t letter, LetterStatus status);
    
    // Accessor for checking contents of word_
    AnalysedLetter& operator[]( unsigned int i );
    
    // Variables accessors
    unsigned int LengthDifference() const;
    double       AverageLinkSize() const;
    unsigned int LargestLink() const;
    unsigned int NumLinks() const;
    int          Score() const;
    
    void Clear(); // Empty everything.
    void RemoveNull(); // Remove any letters with Null status
    void Reverse(); // Reverse contents of vector
    
    // Check vector is empty.
    bool empty() const;
    // Get size of vector
    size_t size() const;
    int CountStatus( LetterStatus status ) const;    
   
    // State of spelling - whether it's correct, exact, an alt spelling or beyond wrong.
    void SetAnalysisState( const AnalysisState& as );
    bool IsExact() const;
    bool IsCorrect() const;
    bool IsAlternateSpelling() const;
    bool IsBeyondWrong() const;
    
    void CalculateStats();
    
    int  NumCorrect() const; // return the number of correct letters in the attempt (calculated)
    int  NumErrors() const;         // return the number of errors in the attempt (calculated)
    
    std::wstring GetString() const; // Recreates the speller's original attempt as a wstring
    AnalysedLetters GetAnalysis() const; // Returns just the word_ part of AnalysedWord.
    
private:
    void CalculateScore();
    void CalculateNumLetters();
    void CalculateLinks();
    
private:

    AnalysedLetters word_;
    
    // These stats are required for creating a WrongSpelling, as well as some comparisons of algorithms.
    int score_; // the value of the attempted spelling
    unsigned int numLinks_;
    double averageLinkSize_;     // Average no. of letters in each consecutive correct letter chain
    unsigned int largestLink_;   // Largest no. of consecutive correct letters
    unsigned int attemptLength_; // How many letters in the attempt
    unsigned int originalLength_; // How many letters in the original word
    unsigned int lengthDifference_; // Difference between attempt length and original length
    AnalysisState analysisState_;
    

};

class SpellingAnalyser{
public:
    SpellingAnalyser(const std::wstring& attempt, const Word* word, Speller& speller,
                     AnalysedWord& analysedWord);
                     
private:
    bool SpellingsEqual(); // returns true if (processed) strings are the same
    std::wstring ApplyOptionsToString( const std::wstring s ); // returns a string cleaned of diacritics and/or capitals, depending on options
    
    // Algorithms
    bool ExactMatch(); // Checks if the (processed) strings are identical.
    void PatternMatching( const unsigned int distance,
                          const std::wstring& attemptProcessed, const std::wstring& attempt,
                          const std::wstring& spellingProcessed, const std::wstring& spelling,
                          AnalysedWord& aw, bool swapOnTheFly = false );
    void SwapSearch( AnalysedWord& aw );
    void ThreeAwaySearch( AnalysedWord& aw );
    
    void ConstructAnalysedWordFromSpelling( const std::wstring s );
    void FillAnalysedWord( const std::wstring s, const LetterStatus stat ); // Fill analysed word with string setting to specified status
    void CheckBeyondWrong();
    
private:
    const std::wstring attempt_;      // The speller's attempt at the spelling
    std::wstring attCopy_;      // The speller's processed attempt (diacritics / caps removed dependent on options)
    std::wstring spelling_;     // Original spelling
    std::wstring speCopy_;      // Original spelling with diacritics / caps removed dependent on options.
    Speller& speller_;
    const Word* pWord_;
    AnalysedWord& analysedWord_;    // Stores the result of the analysis.

    
};


// Prints a word using a Speller's stats, using ScreenPrinter.
class WordPrinter{
public:
    // Prints a word for READ mode, using Speller's Breakdown colours, depending on options.
    void PrintWord(const Word* word, const Speller* speller,
                   ScreenPrinter* screenPrinter, BackBuffer* bb,
                   Gdiplus::Font* font, const Gdiplus::PointF position ) const;
    
    // Prints the speller's attempt at a word.
    void PrintAttempt( const std::wstring attempt, const Speller* speller,
                       ScreenPrinter* screenPrinter, BackBuffer* bb,
                       Gdiplus::Font* font, const Gdiplus::PointF position ) const;
private:

};



#endif; // WORD_H


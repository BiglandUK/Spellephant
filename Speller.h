// Speller.h

#ifndef SPELLER_H
#define SPELLER_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include "Range.h"
#include "Definitions.h"
#include "Word.h"

class DBController;

struct WrongSpelling{
    WrongSpelling();
    WrongSpelling(const AnalysedWord& aw);
    WrongSpelling(std::wstring spelling,
                  int score, double aveLinkLength,
                  unsigned int longestLink, unsigned int lengthDifference );
    
    bool operator< (const WrongSpelling& rhs);
    
    std::wstring spelling_;         // The spelling attempt.
    int          score_;            // The score for this spelling
    double       aveLinkLength_;    // Average length of each "link" (consecutive correct letters)
    unsigned int longestLink_;      // The longest "link" (consecutive correct letters)
    unsigned int lengthDifference_; // Absolute difference in characters between target word and this attempt
    
    
    /*
    Comparison of wrong spellings is as follows:
    1.	The highest Score is a better spelling.  If equal: 
    2.	The largest average link length is better.  If equal:
    3.	The least difference between length of target and attempt is better.  If equal:
    4.	The largest "biggest link" is better.  If equal:
    5.	The algorithm results / wrong spellings are deemed equal.  The newer result replaces the oldest.    
    */
    
};

typedef std::vector<WrongSpelling> WrongSpellingList;

class Record{
public:
    Record();
    Record(unsigned int wordID, unsigned int numAttempts, int level);
    
    bool operator<(const Record& rhs);
    
    unsigned int GetAttempts() const;
    int          GetLevel() const;
    void         SetLevel( const int level );
    
    void AddAttempt();
    void AddWrongSpelling( WrongSpelling& ws ); // Used on loading - no database update
    void AddWrongSpelling( WrongSpelling& ws, unsigned int spellerID, DBController* db); // Used during running - changes to WrongSpellings.
    unsigned int GetNumWrongWords() const;
    StringVec GetWrongWords() const;
    
private:
    void RemoveExcessiveWrongSpellings( unsigned int spellerID, DBController* db );

private:
    static unsigned int MAXWRONGSPELLINGS;

    unsigned int wordID_;
    unsigned int numAttempts_;
    int          level_;
    WrongSpellingList wrongSpellings_;

};

class Speller{
public:
    enum{ PAPER, BREAKDOWN1, BREAKDOWN2, BREAKDOWN3, BREAKDOWN4, BREAKDOWN5,
          PEN, CORRECT, WRONG, MISSING, SWAPPED }; // colour codes
    Speller();
    Speller(unsigned int id, std::wstring name, Range difficulty, std::wstring avatarFilename,
            IDList stars, IDList tags);
    //bool SetName(std::wstring newName);
    
    std::wstring GetName() const;
    Range&  GetDifficulty();
    IDList& GetWordList();
    IDList& GetStarList();
    IDList& GetTagList();
    unsigned int GetID()   const;
    bool UseNaturalSpacing() const;
    bool UseWordBreakdown() const;
    bool UseAutoDiacritics() const;
    bool UseAutoCapitals() const;
    bool UseAnimatedFeedback() const;
    Gdiplus::Color GetColour(int colourCode ) const;
    
    //Record stuff
    unsigned int GetWordAttempts( const int wordID ) const; // returns the number of attempts for a particular word.
    unsigned int GetNumWrongWords( const int wordID ) const;
    StringVec    GetWrongWords( const int wordID ) const;
    int          GetWordLevel( const int wordID ) const; // returns speller's current level for particular word.
    bool         RecordExists( const int wordID ) const; // true if record exists for supplied word ID
    void         CreateRecord( const int wordID );
    void         AddRecord( const int wordID, unsigned int attempts, int level);
    void         IncreaseAttempts( const int wordID );
    void         SetLevel( const int wordID, const int level );
    void         AddWrongSpelling( int wordID, WrongSpelling& ws ); // used on loading.
    void         AddWrongSpelling( int wordID, WrongSpelling& ws, 
                                   DBController* db ); // tries to add a wrong spelling.
    
    void SetColour( int colourCode, Gdiplus::Color& newColour );
    
    void UpdateWordList( WordBank& wordBank );

private:
    void SetUpColours();

private:
    // General
    unsigned int id_;
    std::wstring name_;
    std::wstring avatarFilename_;
    bool isAdmin_;
    
    typedef std::map<int,Record> SpellingRecord;
    SpellingRecord spellingRecord_;
    
    //Spelling Options
    Range difficulty_;
    bool useAutoDiacritics_;
    bool useAutoCapitals_;
    
    // Display Options
    std::vector<Gdiplus::Color> colours_;
    bool useWordBreakdown_;
    bool allowSoundOnly_;
    bool useNaturalLetterSpacing_;
    bool useAnimatedFeedback_;
    
    // Lists
    IDList wordList_;
    IDList starList_;
    IDList tagList_;
};


#endif; //SPELLER_H
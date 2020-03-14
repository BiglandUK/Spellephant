// Mode.h
// Generic mode for menus, options, activities.
// Contains virtual functions for keypresses, mouse control

// Also contains MiniSpell class, which runs the three activities: QuickSpell, SpellingSpotting and WorkWorkout

#ifndef MODE_H
#define MODE_H

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include "Utility.h"
#include "Word.h"
#include "Keyboard.h"

class BackBuffer;
class Button;
class Speller;
class ScreenPrinter;
class Word;
class DBController;
class AnimatedFeedback;
class SSRegion;

class Mode {
public:

Mode(unsigned int& nextMode, unsigned int previousMode, unsigned int id = 0)
: nextMode_(nextMode),
  previousMode_(previousMode),
  modeID_(id) {}
virtual ~Mode() {};

virtual void Update( double dt, const Gdiplus::PointF* cursorPos)=0;
virtual void Display(BackBuffer* bb)=0;

virtual void LMBUp(const Gdiplus::PointF* cursorPos)=0;
virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0)=0;
virtual void KeyDown(unsigned int key)=0;
virtual void KeyUp()=0;
virtual void RMBUp(const Gdiplus::PointF* cursorPos)=0;
virtual void RMBDown(const Gdiplus::PointF* cursorPos)=0;
virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos)=0;

protected:
    unsigned int modeID_;    // Is this needed?!
    unsigned int& nextMode_; // This has to be a reference, as it is passed a value as a reference.
    unsigned int previousMode_;  // The "return to" mode, if required.

};

// MiniSpell class runs the QuickSpell, Spelling Spotting and Word Workout game modes
class MiniSpell : public Mode {
public:
    // enums
    enum{SPELLEROPTION=0, WORDOPTION, WORDLIST, READOPTION, COVEROPTION, WRITEOPTION, CHECKOPTION, EXIT,
         NEWWORD, KEYBOARD, DIACRITICS};   // buttons
    enum Game{QUICKSPELL, SPELLINGSPOTTING, WORDWORKOUT}; // Which game is being played
    enum State{WAIT, READ, WRITE, CHECK};         // Current game state - note: COVER not needed
    enum ListOption {NORMALLIST, STAR};
    enum ReadOption {READALWAYS, READORSOUND, SOUNDONLY};
    enum CoverOption {TOTAL, SPACES};
    enum WriteOption {NOHELP, LETTERS, LETTERSONCE};
    enum WeightingOption { NOWEIGHTING, WEIGHTING };
    enum WWAnalysis { NONE, CORRECT, WRONG, SWAPL, SWAPR }; // Word Workout analysis
    
    
    MiniSpell(unsigned int& nextMode, unsigned int previousMode, unsigned int id,
              WordBank &wordbank, Speller &speller, BackBuffer* bb, ScreenPrinter* sp, Gdiplus::Font* font,
              DBController* db,
              Game game = QUICKSPELL);
    
    virtual ~MiniSpell();
    
    virtual void Update( double dt, const Gdiplus::PointF* cursorPos);
    virtual void Display(BackBuffer* bb);

    virtual void LMBUp(const Gdiplus::PointF* cursorPos);
    virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    virtual void KeyDown(unsigned int key);
    virtual void KeyUp();
    virtual void RMBUp(const Gdiplus::PointF* cursorPos);
    virtual void RMBDown(const Gdiplus::PointF* cursorPos);
    virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos);
    
private:
    void CreateButtons();
    void SetUp();
    void SetUpWorkingList();
    void SetUpRead();
    unsigned int GetNewWord();
    void SetUpSSRegion(unsigned int id); // spelling spotting
    void ReenableID( unsigned int id );
    void SetUpWrite();
    void SetUpCheck();
    
    void ChangeSelectedList();
    void ChangeCoverOption();
    void ChangeWriteOption();
    
    void ReadDisplay(BackBuffer* bb);
    void WriteDisplay(BackBuffer* bb);
    void CheckDisplay(BackBuffer* bb);
    void DisplayAttempt(BackBuffer* bb, std::wstring printString);
    
    void ShowWord(BackBuffer* bb); // Display Word during READ.
    
    bool CharacterAccepted( const wchar_t character ); // Checks if a character can be added to the attempt, based on options.
    std::wstring ConvertSpelling( const std::wstring& original ); // Depending on options, removes capitals and/or diacritics
    wchar_t ConvertCharacter( const wchar_t& original ); // Uses ConvertSpelling to affect a single character only.
    
    void EditAttempt( unsigned int key ); // edits speller's attempt
    
    // Keyboard stuff
    void KeyboardSetup();
    void SwitchKeyboard(int num);
    void ToggleDiacriticKeyboard();
    void WriteOptionAllKeys();
    void WriteOptionSingleKey( unsigned int key );
    
    // QuickSpell stuff
    void SetUpAnimatedFeedback(AnalysedWord& aw);
    
    // Word Workout stuff
    void UpdateLetterTimings( double dt ); // Updates timing of fading colours
    void UpdateLetterAnalysis();           // Checks and updates analysis of attempt
    void UpdateLetterColours();            // Updates fading colours
    bool LettersEqual( wchar_t c1, wchar_t c2 ) const; // Sees if two letters are equal under current settings (caps and diacritics)
    void SetAutoDiacritic( wchar_t& attempt, wchar_t spelling ); // Changes a base letter to the matching diacritic, keeping the base case unchanged
    Gdiplus::Color GetFadeColour(int i);    // Gets the current fade colour, or the Pen colour if drawing spaces or the cursor
    Gdiplus::Color GetTargetColour( WWAnalysis analysis ); // Gets the colour the letter is fading to depending on analysis
    bool CheckForSwap(int pos); // Checks for swapped letters. Returns true if swapped.
    void CreateLetterData();
    void DeleteLetterData(); // Strips last entry for each of timings, analysis and colours.

    void IncreaseLevel(); // If word spelt/chosen correctly, adjust level to top level of the current rank.

private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button*> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images
    
    BackBuffer* bb_;
    Gdiplus::Font* mpFont_;
    ScreenPrinter* pScreenPrinter_;
    DBController*  pDB_;
    const double FADE_SPEED;
    
    Game game_; // Which game is being played.
    WordBank &wordBank_; // Ref to WordBank
    Speller &speller_; // Ref to speller.
    
    State state_;
    ListOption selectedList_;
    ReadOption readOption_;
    CoverOption coverOption_;
    WriteOption writeOption_;
    WeightingOption weightingOption_;
    
    WorkingList workingList_; // Active list used to select words.
    unsigned int totalWeighting_; // Stores total weight of words in workingList.
    Word* pWord_;
    FixedQueue usedWords_;
    
    std::wstring attempt_;
    unsigned int lengthLimit_; // either global limit or length of current word.
    
    // Keyboards
    Keyboard* keyboard_;    // Pointer to current keyboard
    Keyboard qwerty_;       // standard qwerty
    Keyboard abc_;          // alphabetical order
    Keyboard special_;      // special characters

    // QuickSpell stuff
    AnimatedFeedback* pAF_; // Pointer to Animated Feedback object.

    // Word Workout stuff
    typedef std::vector<double> TimingsList;
    TimingsList timings_;
    typedef std::vector<WWAnalysis> AnalysisList;
    AnalysisList analysis_; 
    typedef std::vector<Gdiplus::Color> ColourList;
    ColourList letterColours_; // stores the current colours for each letter - needed for Swapped letters
    
    // Spelling Spotting stuff
    SSRegion* pSSRegion_;

};

#endif // MODE_H
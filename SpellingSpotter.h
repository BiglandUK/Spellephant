// SpellingSpotter.h
/*  The following structs & classes are defined here:
    Structs 
        SSWord - a Spelling Spotting Word
        SSRow  - a Row, which stores SSWords
    Classes
        SSRegion - stores SSRows, and essentially provides and controls the SpellingSpotting interface.
*/

#ifndef SPELLINGSPOTTER_H
#define SPELLINGSPOTTER_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include "Definitions.h"

class BackBuffer;
class Speller;

struct SSWord{
    SSWord(std::wstring text, bool correct = false);       

    std::wstring text_; // text to display
    bool correct_; // whether this word is the correct one or not
    bool selected_; // whether this word was chosen by the speller or not
    float width_; // the width this word takes up, including padding and margins on both sides.

};

typedef std::vector<SSWord*> SSWordList;

struct SSRow{
    SSRow(float width);
    
    bool AddWord( SSWord* word ); // Store a pointer to word.  Returns false if the word won't fit.
    void RandomiseWordOrder(); // mix up the order the words are stored.
    bool IsEmpty() const;
    
    SSWordList words_; // list of SSWords
    float horizontalOffset_; // for centering - this is actually half the emptyWidth_ variable!
    float emptyWidth_; // How much space is left to fill with words
    
};

typedef std::vector<SSRow> SSRowList;

class SSRegion{
public:
    SSRegion(std::wstring correctSpelling, StringVec wrongSpellings,
             Gdiplus::PointF pos, BackBuffer* bb, Gdiplus::Font* font, Speller& speller);
             
    ~SSRegion();
    
    void SetUp(std::wstring& correctSpelling, StringVec& wrongSpellings, Speller& speller);
    
    void Display(); // display each row.
    void Update( double dt, const Gdiplus::PointF *cursorPos ); // borders while nothing selected; colour changes when selected
    
    bool Click( const Gdiplus::PointF* cursorPos ); // Check which word, if any, has been clicked
    

    
private:
    typedef std::pair<float,float> HeightAndWidth;
    HeightAndWidth MeasureWord(std::wstring& word); // Measure the height and width of a word
    
    int GetRandomRowIndex(); // Select an SSRow index at random
    
    bool IsInWord(const Gdiplus::PointF* cursorPos);
    void DrawHighlight( Gdiplus::Graphics& graphics, const Gdiplus::PointF& position );
    
private:
    // TODO: The items below should be constants once settled.
    float height_, width_; //The overall height and width of the region. TODO: make these constant once determined.
    float hPad_, vPad_; // The extra "paper" area around the words. TODO: make these constant once determined.
    float hMargin_, vMargin_; // The gap between each word and row.  TODO: make these constant as well.
    float highlightWidth_; // Thickness of highlight border
    Gdiplus::Color highlightColour_;
    
    float rowHeight_; // height of each row (calculated)
    float verticalOffset_; // for centering.
    
    SSRowList rows_; 
    SSWordList wordList_;
    std::pair<int,int> highlightWord_; // Stores which row and which word (within that row) is highlighted.
    
    unsigned int maxRows_; // calculated based on word with greatest measured height
    Gdiplus::PointF position_; // top left location on screen
    bool selected_; // whether a word has been chosen or not.
    BackBuffer* bb_;
    Gdiplus::Font* pFont_;
    double timer_;
    Gdiplus::Color paper_, pen_, correct_, wrong_;
    Gdiplus::Color correctFade_, wrongFade_;
    const double FADESPEED;
};

#endif; // SPELLINGSPOTTER_H
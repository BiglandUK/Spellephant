// Options.h

/* This header is for Options menus:
    Word List options
    Speller preferences
    Administrator options
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include "Mode.h"
#include <vector>
#include "Keyboard.h"
#include "Definitions.h"
#include "Word.h"
#include "Range.h"

//Forward Declarations
class BackBuffer;
class Button;
class ScrollBox;
class Dumbell;
class DBController;
class Speller;

class WordListOptions : public Mode {
public:
    enum{SAVE, CANCEL, TAGSALL, TAGSSWAP, SORTDIFF, SORTAZ, WORDFILTER};
    enum FilterState{ HIDEFILTERED = 1, SHOWFILTERED};
    enum SortState { SORTDIFFICULTY = 1, SORTALPHA, SORTRANK, SORTSTARS };
    WordListOptions(unsigned int& nextMode, unsigned int previousMode, unsigned int id, BackBuffer* bb,
                    Gdiplus::Font* font, WordBank& wordBank, TagList& tagList,
                    Speller* speller,
                    DBController* db);
    virtual ~WordListOptions();
    
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
    void SetUpTagScrollBox();
    void SetUpWordScrollBox();
    void SetUpWordTagData(unsigned int wordID);
    void SetUpDumbell();
    
    bool WordInDifficultyRange( unsigned int wordID );
    bool WordHasActiveTags( unsigned int wordID );
    void UpdateWords( int tagID );
    void UpdateWords();
    
    void ToggleWordFilter( int state );
    void SortWords( int state );
    
    void ChangeDifficulty( Range newDiff );
    
    void SaveChanges();
    void Cancel();
    
    void StarChange( unsigned int wordID );

private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button*> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images
    Gdiplus::Font* mpFont_;
    Gdiplus::Font* pTagFont_;
    Gdiplus::Font* pWordFont_;
    
    WordBank& wordBank_;
    TagList& tagList_;
    
    FilterState fState_;
    SortState sState_;
    
    //Tag ScrollBox
    ScrollBox* sbTagList_;          // Pointer to scrollbox for Tags
    TableData tagData_;             // Tags data
    
    //Word ScrollBox
    ScrollBox* sbWordList_;
    TableData wordData_;
    Gdiplus::Image* starIcons_;
    int selectedRow_;       // Used to check if a different word has been selected, for tag updates.
                            // Also checked when selecting stars.    
    // Difficulty Dumbell
    Dumbell* dbDifficulty_;
    
    // Database
    DBController* pDB_;
    
    Speller* speller_;
    // Speller Data (references)
    Range& refDifficulty_;
    IDList& refSpellerStars_;
    IDList& refSpellerTags_;
    // Speller Data (copies)
    unsigned int spellerID_;
    Range difficulty_;
    //IDList spellerStars_;
    
};


#endif;
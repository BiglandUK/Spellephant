// AnimatedFeedback.h
/*
This class controls the animated display showing the corrections (if any) to a spelling attempt.
It uses an AnalysedWord object as its "script".
It also includes media buttons for rewinding and pausing the animation.
*/

#ifndef ANIMATEDFEEDBACK_H
#define ANIMATEDFEEDBACK_H

#include <windows.h>
#include <gdiplus.h>
#include "Word.h"
#include "Slider.h"

class Button;
class BackBuffer;
class Speller;
class ScreenPrinter;

class AnimatedFeedback{

public:
    enum {PLAY, PAUSE, REWIND};
    enum ColourType{PAPER, PEN, CORRECT, WRONG, MISSING, SWAPPED};
    AnimatedFeedback(Speller& speller,
                     Gdiplus::Font* font,
                     AnalysedLetters& attempt,
                     BackBuffer* bb);
                     
    void ShowMediaControls();
    void HideMediaControls();
    void MediaControlsPosition( Gdiplus::PointF& newPos );
    void LMBUp(const Gdiplus::PointF* cursorPos);
    void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    
    void WordPosition( Gdiplus::PointF& newPos);
    
    void Update( double dt, const Gdiplus::PointF& mousePos );
    void Display( double dt );
    void Click( Gdiplus::PointF clickPos );
    
private:
    
    void CalculateAnimation();
    int  CalculateMWGroups(); // Number and location of missing/wrong groups for Stage 2
    int  CalculateMisplacedGroups(); // Number and location of swapped and three away groups for Stage 3
    void Stage1Update( double t );
    void Stage2Update( double t );
    void Stage3Update( double t );
    void Stage1Display();
    void Stage2Display();
    void Stage3Display();
    void CompleteDisplay();
    
    Gdiplus::Color GetFadeColour( const Gdiplus::Color& start, const Gdiplus::Color& dest, double ratio );
    double MeasureSpacer( std::wstring letters ); //calculates width of passed in letters

    void Rewind();
    void Pause();
    
private:
    // These colours come from the Speller Options - note there is no special colour for 3 Away letters.
    Gdiplus::Color  paper_,  // Background for the animation - a solid rectangle of this color. Can be transparent.
                    pen_,     // Colour for pen
                    correct_, // Colour for correct letters
                    wrong_,   // Colour for wrong letters
                    missing_, // Colour for missing letters
                    swapped_; // Colour for swapped letters
    
    bool showMediaControls_,// Show or hide the media controls
         useNaturalSpacing_,// From Speller Options
         paused_;           // Freeze animation

    enum Stage{ STAGE1, STAGE2, STAGE3, COMPLETE};
    Stage stage_;

    double spacerBefore_, spacerAfter_, currentSpacer_; // For stage 2, how far apart the letters either side of the animation need to be
    
    std::wstring  missingLetters_, // Missing letters dropping in from above
                  wrongLetters_;   // Wrong letters dropping out of the word   
         
    Gdiplus::Font* font_;
    
    double  timer_; // Identifies position in animation.

    Gdiplus::PointF wordPos_,       // Top left corner for word display
                    mediaPos_;    // Top left corner for media control panel TODO: use mediaPos
                    
    Gdiplus::Bitmap* rewindImage_, *pauseImage_; // button image TEMP
    Button* btnRewind_, *btnPause_; // Media buttons
                    
    AnalysedLetters attemptCopy_;  // Copy of the speller's attempt.

    // Timers - these control the length of each animation part and therefore overall animation length.  TODO:Convert to const
    double pause0_, pause1_, pause2_, stage1Speed_, stage2Speed_, stage3Speed_;
    double animationLength_; // Stores overall animation length.
    double dropDistance_; // TODO:convert to CONSTANT - this is the distance missing/wrong letters moved when dropping in/out
    double jumpDistance_; // as above.   This is used to control the max height of a jumping letter (Stage 3)
    double verticalOffset_; // Current vertical position for animated letters. (Stage 2)
    double leftLetterOffset_, rightLetterOffset_, currentLeft_, currentRight_,
            jumpedLettersOffset_, currentJumped_, flyingLetterOffset_ ; // For swapping letters (Stage 3)
    Gdiplus::PointF flyingLetter_; // Location of ThreeAway (Stage 3)
    typedef std::vector<std::pair<int,int> > Stage2Groups; // 1st=position in attemptCopy_. 2nd=no. of letters
    Stage2Groups stage2Groups_;
    enum MisplacedTypes{ SWAP, ARCFORWARD, ARCBACK };
    typedef std::vector<std::pair<int,MisplacedTypes> > Stage3Groups;
    Stage3Groups stage3Groups_;
    ScreenPrinter* pSP_;
    Gdiplus::Color correctTween_, wrongTween_, missingTween_, misplacedTween_; // colours used for fading
    int currentGroup_; // track changes to groups (stages 2 and 3) to save time with some calcs
    BackBuffer* bb_;
    Slider slider_;
    
};

#endif // ANIMATEDFEEDBACK_H
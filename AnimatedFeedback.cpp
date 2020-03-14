// AnimatedFeedback.cpp

#include "AnimatedFeedback.h"
#include "Speller.h"
#include "BackBuffer.h"
#include "ScreenPrinter.h"
#include "Button.h"
#include <algorithm>

using namespace std;
using namespace Gdiplus;

AnimatedFeedback::AnimatedFeedback(Speller& speller,
                     Gdiplus::Font* font,
                     AnalysedLetters& attempt,
                     BackBuffer* bb)
: font_(font), attemptCopy_(attempt), bb_(bb),
  showMediaControls_(true), wordPos_(PointF(0.0f, 0.0f)), mediaPos_(PointF(0.0f, 0.0f)),
  paused_(false), pSP_(0), slider_( 400.0f, PointF(200.0f, 500.0f) )
 {
    paper_      = speller.GetColour(Speller::PAPER);
    pen_        = speller.GetColour(Speller::PEN);
    correct_    = speller.GetColour(Speller::CORRECT);
    wrong_      = speller.GetColour(Speller::WRONG);
    missing_    = speller.GetColour(Speller::MISSING);
    swapped_    = speller.GetColour(Speller::SWAPPED);    
    
    useNaturalSpacing_ = speller.UseNaturalSpacing();
 
   
    CalculateAnimation();
    
    // Create media buttons
    rewindImage_ = new Bitmap(L"Images/RepeatFeedback.jpg");
    pauseImage_  = new Bitmap(L"Images/Pause.jpg");
    btnRewind_ = new Button( rewindImage_, PointF(100.0f, 400.0f));
    btnPause_  = new Button( pauseImage_, PointF( 150.0f, 400.0f));
    stage_ = STAGE1;

 }
 
void AnimatedFeedback::ShowMediaControls(){
    showMediaControls_ = true;
}

void AnimatedFeedback::HideMediaControls(){
    showMediaControls_ = false;
}

void AnimatedFeedback::MediaControlsPosition( Gdiplus::PointF& newPos ){
    mediaPos_ = newPos;
}

void AnimatedFeedback::LMBDown( const Gdiplus::PointF* cursorPos, const double time ){
    btnPause_->Click( cursorPos );
    btnRewind_->Click( cursorPos );
    slider_.LMBDown( *cursorPos );
}

void AnimatedFeedback::LMBUp(const Gdiplus::PointF *cursorPos){
    if( btnPause_->Release( cursorPos ) )
        Pause();
    if( btnRewind_->Release( cursorPos ) )
        Rewind();
    slider_.LMBUp();
}
 
void AnimatedFeedback::WordPosition(Gdiplus::PointF &newPos){
    wordPos_ = newPos;
 }
 
void AnimatedFeedback::Update( double dt, const Gdiplus::PointF& mousePos ){
    if( !paused_ )
        timer_ += dt;    
    
    slider_.Update( timer_ / animationLength_, mousePos );
    
    timer_ = slider_.Ratio() * animationLength_;
   
    double t = timer_; // Make a copy to be used to determine stage.
    
    if( t < pause0_ )
        return;
    t -= pause0_;
    
    if( t <= stage1Speed_ ){
        Stage1Update( t );
        return;
    }
    t -= stage1Speed_;
    
    if( t<= pause1_ )
        return;
    t -= pause1_;
    
    if( t <= ( stage2Speed_ * stage2Groups_.size() ) ) {
        Stage2Update( t );
        return;
    }
    t -= ( stage2Speed_ * stage2Groups_.size() );
    
    if( t<= pause2_ )
        return;
    t -= pause2_;
    
    if( t <= ( stage3Speed_ * stage3Groups_.size() ) ) {
        Stage3Update( t );
        return;
    }
    t -= ( stage3Speed_ * stage3Groups_.size() );
    stage_ = COMPLETE;
}


double AnimatedFeedback::MeasureSpacer( std::wstring letters ){
    HDC hdc = bb_->getDC();
    Gdiplus::Graphics graphics(hdc);
    PointF p;
    //Need the following StringFormat for accurate measurement.
    StringFormat* sf = (StringFormat::GenericTypographic())->Clone();
    sf->SetFormatFlags( StringFormatFlags::StringFormatFlagsMeasureTrailingSpaces );
    Gdiplus::RectF rec;
    graphics.MeasureString(letters.c_str(), -1, font_, p, sf, &rec);
    return static_cast<double>(rec.Width);
}

void AnimatedFeedback::Display( double dt ){
    pSP_ = new ScreenPrinter( bb_ );
    HDC hdc = bb_->getDC();
    int x = bb_->mOffX;
    int y = bb_->mOffY;
    
    btnPause_->Display( bb_ );
    btnRewind_->Display( bb_ );
    
    slider_.Display( bb_ );
    
    switch( stage_ ){
        case STAGE1:{
 
            // Display Correct, Wrong in fading colours.
            // Display Swapped, ThreeAwayWrong in Pen.
            // Don't display Missing or ThreeAwayMissing.
            Stage1Display();
            break;
        }
        case STAGE2:{
            Stage2Display();
            break;
        }
        case STAGE3:{
            Stage3Display();
            break;
        }
        default:{
            CompleteDisplay();
            paused_ = true;
            break;
        }
    }
    
    delete pSP_;
    pSP_ = 0;
}

void AnimatedFeedback::Click( Gdiplus::PointF clickPos ){

}

void AnimatedFeedback::Rewind(){
    timer_ = 0.0;
    stage_ = STAGE1;
}

void AnimatedFeedback::Pause(){
    paused_ = !paused_;
}

void AnimatedFeedback::Stage1Update( double t ){
    correctTween_ = GetFadeColour( pen_, correct_, t );
    wrongTween_   = GetFadeColour( pen_, wrong_, t );
    stage_ = STAGE1;
    
}

void AnimatedFeedback::Stage1Display(){
    PointF pos = wordPos_;
    for( int i = 0; i < static_cast<int>(attemptCopy_.size()); ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correctTween_ );
                break;
            }
            case Wrong:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, wrongTween_ );
                break;            
            }
            case Swapped:
            case ThreeAwayWrongF:
            case ThreeAwayWrongB:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for
}

void AnimatedFeedback::Stage2Update( double t ){
    if( stage2Groups_.empty() ) // No content for stage 2, so do nothing.
        return;
    int group = 0;
    while( t > stage2Speed_ ) {
        t -= stage2Speed_;
        ++group;
    }
    if( stage_ != STAGE2 || group != currentGroup_ ){
        // Important calcs for this stage / group not yet made.
        missingLetters_.clear();
        wrongLetters_.clear();
        for( int i = stage2Groups_[group].first; i < stage2Groups_[group].first + stage2Groups_[group].second; ++i ){
            switch( attemptCopy_[i].status_ ){
                case Missing:
                    missingLetters_.push_back( attemptCopy_[i].letter_ );
                    break;
                case Wrong:
                    wrongLetters_.push_back( attemptCopy_[i].letter_ );
                    break;
                default:
                    break;
            }
        }
        // Calculate Spacer target (new position of "after" letters to accommodate change of missing/wrong letters
        spacerBefore_ = MeasureSpacer( wrongLetters_ );
        spacerAfter_  = MeasureSpacer( missingLetters_ );
        //Store current group and stage
        currentGroup_ = group;
        stage_ = STAGE2;
    }
    double ratio = t / stage2Speed_;
    //Colours
    wrongTween_ = Color(static_cast<BYTE>(255.0 * (1.0-ratio)),
                        wrong_.GetR(), wrong_.GetG(), wrong_.GetB() );
    missingTween_ = Color( static_cast<BYTE>(255.0 * ratio),
                        missing_.GetR(), missing_.GetG(), missing_.GetB() );
    // Vertical offset (up and down)
    verticalOffset_ = dropDistance_ * ratio;
    // Spacer
    currentSpacer_ = spacerBefore_ - ( (spacerBefore_ - spacerAfter_) * ratio );
}

void AnimatedFeedback::Stage2Display(){
    PointF pos = wordPos_;
    // Print the letters before the animated part
    for( int i = 0; i < stage2Groups_[currentGroup_].first; ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                break;
            }
            case Missing:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, missing_ );
                break;            
            }
            case Swapped:
            case ThreeAwayWrongF:
            case ThreeAwayWrongB:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for
    PointF storePos = pos; // Store position so we can print both wrong and missing from same spot
    if( !missingLetters_.empty() ){ // If some missing letters, print those above.
        pos.Y = pos.Y - static_cast<REAL>(dropDistance_ - verticalOffset_);
        for( wstring::iterator iter = missingLetters_.begin();
              iter != missingLetters_.end();
              ++iter){
            pos = pSP_->PrintLetter( *iter, pos, *font_, missingTween_ );
        } // End for
        pos = storePos; // Restore pos
    } // end Missing
    if( !wrongLetters_.empty() ) {
        pos.Y += static_cast<REAL>( verticalOffset_ );
        for( wstring::iterator iter = wrongLetters_.begin();
              iter != wrongLetters_.end();
              ++iter){
            pos = pSP_->PrintLetter( *iter, pos, *font_, wrongTween_ );
        } // End for
        pos = storePos; // Restore pos
    } //  End Wrong
    pos.X += static_cast<REAL>( currentSpacer_ );
    // Print remainder of word
        for( int i = stage2Groups_[currentGroup_].first + stage2Groups_[currentGroup_].second;
             i < attemptCopy_.size(); ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                break;
            }
            case Wrong:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, wrong_ );
                break;            
            }
            case Swapped:
            case ThreeAwayWrongF:
            case ThreeAwayWrongB:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for
}

void AnimatedFeedback::Stage3Update( double t ){
    if( stage3Groups_.empty() ) // No content for stage 3, so do nothing.
        return;
    int group = 0;
    while( t > stage3Speed_ ) {
        t -= stage3Speed_;
        ++group;
    }
    if( stage_ != STAGE3 || group != currentGroup_ ){
        // Important calcs for this stage / group not yet made.
        wstring lettersToAnimate;
        if( stage3Groups_[group].second == SWAP ){
            // spacers
            lettersToAnimate.push_back( attemptCopy_[stage3Groups_[group].first].letter_ );
            rightLetterOffset_ = MeasureSpacer( lettersToAnimate ); // Width of Left letter, needed for right letter position
            lettersToAnimate.push_back( attemptCopy_[stage3Groups_[group].first + 1].letter_ );
            currentSpacer_ = MeasureSpacer( lettersToAnimate ); // Main (fixed) spacer
            leftLetterOffset_ = currentSpacer_ - rightLetterOffset_; // Width of Right letter, needed for left letter position
        } else {
            lettersToAnimate.push_back( attemptCopy_[stage3Groups_[group].first].letter_ );
            jumpedLettersOffset_ = MeasureSpacer(lettersToAnimate); // width of jumping letter
            // DOES IT MATTER THAT, UNDER ARCBACK, THESE LETTERS AREN'T IN THE "CORRECT" ORDER?
            lettersToAnimate.push_back( attemptCopy_[stage3Groups_[group].first + 1].letter_ );
            lettersToAnimate.push_back( attemptCopy_[stage3Groups_[group].first + 2].letter_ );
            currentSpacer_ = MeasureSpacer( lettersToAnimate ); //width of all three letters in animation.
            flyingLetterOffset_ = currentSpacer_ - jumpedLettersOffset_; // width of letter pair.
        }        
        //Store current group and stage
        currentGroup_ = group;
        stage_ = STAGE3;
    }
    double ratio = t / stage3Speed_;
    if( ratio > 1.0 ) ratio = 1.0; // shouldn't be needed
    //Colours
    misplacedTween_ = GetFadeColour( pen_, swapped_, ratio );
    // Other adjustments, based on misplaced type
    if( stage3Groups_[currentGroup_].second == SWAP ){
        currentLeft_ = leftLetterOffset_ * ratio; // Position of left letter (moving right)
        currentRight_ = rightLetterOffset_ * (1.0 - ratio); // Position of right letter (moving left)
    } else if( stage3Groups_[currentGroup_].second == ARCFORWARD ) {
        currentJumped_ = jumpedLettersOffset_ * ( 1 - ratio ); // Position of letter pair (moving left)
        flyingLetter_.X = flyingLetterOffset_ * ratio;
        flyingLetter_.Y = -(jumpDistance_ * flyingLetter_.X) * ( flyingLetterOffset_ - flyingLetter_.X ); // Position of flying letter
    } else {
        currentJumped_ = jumpedLettersOffset_ * ratio; // Position of letter pair (moving right)
        flyingLetter_.X = flyingLetterOffset_ * (1 - ratio);
        flyingLetter_.Y = -(jumpDistance_ * flyingLetter_.X) * ( flyingLetterOffset_ - flyingLetter_.X ); // Position of flying letter
    }
}

void AnimatedFeedback::Stage3Display(){
    // Print letters before animation
    PointF pos = wordPos_;
    for( int i = 0; i < stage3Groups_[currentGroup_].first; ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                break;
            }
            case Missing:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, missing_ );
                break;            
            }
            case Swapped:{
                pos = pSP_->PrintLetter( attemptCopy_[i+1].letter_, pos, *font_, swapped_ );
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, swapped_ );
                ++i;
                break;
            }
            case ThreeAwayMissingF:
            case ThreeAwayMissingB:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, swapped_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for
    
    // Animate swapping letters
    PointF storePos = pos; // Store position
    int groupSize = 2; // Default - for SWAP
    if( stage3Groups_[currentGroup_].second == SWAP ){
        //Print left letter
        pos.X += static_cast<REAL>( currentLeft_ );
        pSP_->PrintLetter( attemptCopy_[stage3Groups_[currentGroup_].first].letter_, pos, *font_, misplacedTween_ );
        pos.X = storePos.X + static_cast<REAL>( currentRight_ );
        pos = pSP_->PrintLetter( attemptCopy_[stage3Groups_[currentGroup_].first + 1].letter_, pos, *font_, misplacedTween_ );
        pos = storePos; // restore position
    } else { // ARCFORWARD or ARCBACK
        groupSize = 4; // Alter groupSize to reflect larger letter set in animation than SWAP
        double dir = ( stage3Groups_[currentGroup_].second == ARCFORWARD ) ? -1.0 : 1.0;
        // Print Jumped letters
        pos.X +=  static_cast<REAL>( currentJumped_ );
        for( int i = stage3Groups_[currentGroup_].first + 1; i < stage3Groups_[currentGroup_].first + 3; ++i ){
            switch( attemptCopy_[i].status_ ){
                case Correct:{
                    pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                    break;
                }
                case Swapped:{
                    pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                    break;
                }
                default:
                    break;
            }// end switch
        }// end for
        // Restore position
        pos = storePos;
        // Print Jumping / Flying letter
        pos.X += flyingLetter_.X;
        pos.Y += flyingLetter_.Y;
        pSP_->PrintLetter( attemptCopy_[stage3Groups_[currentGroup_].first].letter_, pos, *font_, misplacedTween_ );
        pos = storePos;
    } // End animated section
    pos.X += static_cast<REAL>( currentSpacer_ );
    
    // Print remainder of word
    for( int i = stage3Groups_[currentGroup_].first + groupSize;
             i < attemptCopy_.size(); ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                break;
            }
            case Missing:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, missing_ );
                break;            
            }
            case Swapped: {
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                break;
            }
            case ThreeAwayWrongF:{ 
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                break;
            }
            case ThreeAwayWrongB:{ // Only print this if it does not immediately follow a currently swapping pair.
                if( !( i - (stage3Groups_[currentGroup_].first + groupSize) == 0 &&
                       stage3Groups_[currentGroup_].second == SWAP ) ){
                     pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, pen_ );
                } 
                break;
            }
            case ThreeAwayMissingB:{// Only print this if it's the first letter after a SWAP pair AND the end of a previous group.
                if( i - (stage3Groups_[currentGroup_].first + groupSize) == 0 &&
                    stage3Groups_[currentGroup_].second == SWAP )
                    pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, swapped_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for

}

void AnimatedFeedback::CompleteDisplay(){
    PointF pos = wordPos_;
    for( int i = 0; i < attemptCopy_.size(); ++i ){
        switch( attemptCopy_[i].status_ ){
            case Correct:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, correct_ );
                break;
            }
            case Missing:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, missing_ );
                break;            
            }
            case Swapped:{
                pos = pSP_->PrintLetter( attemptCopy_[i+1].letter_, pos, *font_, swapped_ );
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, swapped_ );
                ++i;
                break;                 
            }
            case ThreeAwayMissingF:
            case ThreeAwayMissingB:{
                pos = pSP_->PrintLetter( attemptCopy_[i].letter_, pos, *font_, swapped_ );
                break;
            }
            default:
                break;
        }// end switch
    }// end for
}

Gdiplus::Color AnimatedFeedback::GetFadeColour( const Gdiplus::Color& start, const Gdiplus::Color& dest, double ratio ){
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

    return Color(rPen + static_cast<BYTE>( (rTarget - rPen) * ratio ),
                 gPen + static_cast<BYTE>( (gTarget - gPen) * ratio ),
                 bPen + static_cast<BYTE>( (bTarget - bPen) * ratio ) );
}


void AnimatedFeedback::CalculateAnimation(){
    // TODO: Convert to Constants
    pause0_ = pause1_ = 0.0;
    pause2_ = 0.0;
    stage1Speed_ = stage2Speed_ = 1.0;
    stage3Speed_ = 1.0;
    dropDistance_ = 150.0;
    jumpDistance_ = 0.07;
    
    animationLength_ = 0.0;
    animationLength_ += pause0_ + stage1Speed_ + pause1_;
    animationLength_ += CalculateMWGroups() * stage2Speed_;
    animationLength_ += pause2_;
    animationLength_ += CalculateMisplacedGroups() * stage3Speed_;
    
    // Set timer to zero.
    timer_ = 0.0;
}

int AnimatedFeedback::CalculateMWGroups(){
    int num = 0;
    int i = 0;
    
    while( i < attemptCopy_.size() ) {
        while(  i < attemptCopy_.size() &&
                attemptCopy_[i].status_ != Wrong &&
                attemptCopy_[i].status_ != Missing ){
            ++i;       
        }
        
        if( i == attemptCopy_.size() )
            break;
        
        ++num;
        int groupStart = i;
        int groupSize = 0;
        
        while(  i < attemptCopy_.size() &&
               ( attemptCopy_[i].status_ == Wrong || attemptCopy_[i].status_ == Missing ) ){
            ++i;
            ++groupSize;
        }
        stage2Groups_.push_back( make_pair( groupStart, groupSize ) );
        if( i == attemptCopy_.size() )
            break;
    }
    
    return num;
    
}

// ThreeAways HAVE to be "discovered" in sequential order.
int AnimatedFeedback::CalculateMisplacedGroups(){
    int num = 0;
    for( int i = 0;
         i < attemptCopy_.size();
         ++i) {
         if( attemptCopy_[i].status_ == ThreeAwayWrongF ){
            ++num;
            stage3Groups_.push_back( make_pair(i, ARCFORWARD) ); // This will require a letter jumping forward.
            continue;
         }
         if( attemptCopy_[i].status_ == ThreeAwayMissingF ){
            ++num;// Increase group count
            stage3Groups_.push_back( make_pair(i, ARCBACK) ); // This will require a letter jumping back.
            continue;
         }
         
         if( attemptCopy_[i].status_ == Swapped ){
            ++num;
            stage3Groups_.push_back( make_pair(i, SWAP) );
            ++i;
         }
    }
    
    return num;
    
}
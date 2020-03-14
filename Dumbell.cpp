// Dumbell.cpp
#include <math.h>
#include "Dumbell.h"
#include "BackBuffer.h"
using namespace Gdiplus;
using namespace std;

Dumbell::Dumbell() {}

// User can determine how to set up the dumbell.
// Option 1: RIGHTLIMIT - supply left and right limit (gapSize_ is calculated)
// Option 2: GAPSIZE    - supply left and gap size (rightLimit_ is calculated)
// Option 3: SCALEWIDTH - supply left and overall width (rightLimit_ and gapSize_ are calculated)
Dumbell::Dumbell(int numValues, int lowestValue,
                 double xPosition, double yPosition,
                 double size, Dumbell::SizeType sizeType,
                 double ballDiameter,
                 Gdiplus::Color& colour )
: numValues_(numValues)
, lowestValue_(lowestValue)
, yPosition_(yPosition)
, leftLimit_(xPosition)
, ballDiameter_(ballDiameter)
, colour_(colour)
, leftBall_(xPosition)
, isLeftBallGrabbed_(false)
, isRightBallGrabbed_(false)
{
    if( size <= 0.0 ) size = 10.0;
    if( numValues_ <= 1 ) numValues_ = 2; // prevent divide by zero
    
    switch( sizeType )
    {
        case RIGHTLIMIT: {
            rightLimit_ = size;
            if( rightLimit_ < leftLimit_ ) rightLimit_ = leftLimit_;
            rightBall_ = rightLimit_;
            gapSize_ = (rightLimit_ - leftLimit_) / (numValues_ - 1);
            break;
        }
        case GAPSIZE: {
            gapSize_ = size;
            rightLimit_ = leftLimit_ + (gapSize_ * (numValues_ - 1) );
            rightBall_ = rightLimit_;
            break;
        }
        case SCALEWIDTH: {
            rightLimit_ = size + leftLimit_;
            rightBall_ = rightLimit_;
            gapSize_ = (rightLimit_ - leftLimit_) / (numValues_ - 1);
            break;
        }
     }

}

void Dumbell::SetLeftBall( const double pos ) {
    leftBall_ = pos;
}

void Dumbell::SetLeftBall( const int value ) {
    leftBall_ = leftLimit_ + (gapSize_ * ( value - lowestValue_ ) );
}

void Dumbell::SetRightBall( const double pos ) {
    rightBall_ = pos;
}

void Dumbell::SetRightBall( const int value ) {
    rightBall_ = leftLimit_ + (gapSize_ * ( value - lowestValue_ ) );
}


void Dumbell::SetBallPositions( const double leftPos, const double rightPos ) {
    SetLeftBall(leftPos);
    SetRightBall(rightPos);
}

void Dumbell::SetBallPositions( const int leftValue, const int rightValue ) {
    SetLeftBall(leftValue);
    SetRightBall(rightValue);
}

void Dumbell::SetYPosition( const double pos ) {
    yPosition_ = pos;
}

void Dumbell::SetBallDiameter( const double diameter ) {
    ballDiameter_ = diameter;
}


void Dumbell::SetColour( const Gdiplus::Color& colour ) {
    colour_ = colour;
}

// Draws dumbell
void Dumbell::Display(BackBuffer& bb)
{
    Graphics graphics(bb.getDC());
    double x = bb.mOffX;
    double y = bb.mOffY;

    SolidBrush br(colour_);
    graphics.FillEllipse(&br, RectF(leftBall_ + x, yPosition_ + y, ballDiameter_, ballDiameter_ ));
    graphics.FillEllipse(&br, RectF(rightBall_ + x, yPosition_ + y, ballDiameter_, ballDiameter_ ));
    graphics.FillRectangle(&br, 
                           RectF( leftBall_  + x + ( 0.25 * ballDiameter_ ),
                                  yPosition_ + y + ( 0.375 * ballDiameter_ ),
                                  rightBall_ - leftBall_,
                                  0.25 * ballDiameter_ ));

}

//Updates ball positions (if grabbed) to follow mouse position
void Dumbell::Update(const Gdiplus::PointF& mousePos) {
    // Set moving end to current mouse x pos.
    if( isLeftBallGrabbed_ ) leftBall_ = (mousePos.X - (0.5 * ballDiameter_));    
    if( leftBall_ > rightBall_ ) rightBall_ = leftBall_;
    if( isRightBallGrabbed_ ) rightBall_ = mousePos.X - (0.5 * ballDiameter_);
    if( rightBall_ < leftBall_ ) leftBall_ = rightBall_;
    
    // Set limits on moving
    if( leftBall_ < leftLimit_ ) leftBall_ = leftLimit_;
    if( rightBall_ < leftLimit_ )
    {
        leftBall_ = rightBall_ = leftLimit_;
        isRightBallGrabbed_ = true;
        isLeftBallGrabbed_ = false;
    }


    if( rightBall_ >  rightLimit_ ) rightBall_ = rightLimit_;
    if( leftBall_ > rightLimit_ )
    {
        leftBall_ = rightBall_ = rightLimit_;
        isLeftBallGrabbed_ = true;
        isRightBallGrabbed_ = false;
    }
}


// Get the values represented by the dumbell, returning them in a pair<int,int>
pair<int, int> Dumbell::GetValues() {

    // Ensure each ball is snapped (in case this function called while ball is grabbed)
    SnapPosition(leftBall_);
    SnapPosition(rightBall_);
    
    //Calculate values
    double left = floor(((leftBall_ - leftLimit_) / gapSize_)+0.5);
    double right = floor(((rightBall_ - leftLimit_) / gapSize_)+0.5);
        
    return pair<int, int>(left + lowestValue_, right + lowestValue_);
}

//Get the values represented by the dumbell, and return in the supplied parameters.
//Uses the GetValues() function.
void Dumbell::GetValues(int& left, int& right) {

    pair<int, int> temp = GetValues();
    left = temp.first;
    right = temp.second;
}

double Dumbell::GetGapSize() const{
    return gapSize_;
}

// Checks if the user has clicked on one of the balls.
bool Dumbell::Grab(const Gdiplus::PointF& mousePos)
{
    // Has user grabbed one end of the Barbell?
    // Left end
    PointF LCentre(leftBall_ + 0.5 * ballDiameter_, yPosition_ + 0.5 * ballDiameter_ );
    double distanceBetween = sqrt(pow(LCentre.X - mousePos.X, 2) + pow(LCentre.Y - mousePos.Y, 2));
    if( distanceBetween <= ( ballDiameter_ / 2.0) ) // Has the user clicked within the circle?
    {
        isLeftBallGrabbed_ = true;
        return true;
    }
    isLeftBallGrabbed_ = false;

    // Right end
    PointF RCentre( rightBall_ + 0.5 * ballDiameter_, yPosition_ + 0.5 * ballDiameter_ );
    distanceBetween = sqrt(pow(RCentre.X - mousePos.X, 2) + pow(RCentre.Y - mousePos.Y, 2));
    if( distanceBetween <= (ballDiameter_ / 2.0) ) // Has the user clicked within the circle?
    {
        isRightBallGrabbed_ = true;
        return true;
    }
    isRightBallGrabbed_ = false;
    
    return false;
}

// Releases the balls
bool Dumbell::Release()
{
    if( !isLeftBallGrabbed_ && !isRightBallGrabbed_ )
        return false;
    
    isLeftBallGrabbed_ = false;
    isRightBallGrabbed_ = false;
    // Now "jump" the balls to the nearest matching "gap" position
    SnapPosition(leftBall_);
    SnapPosition(rightBall_);
    
    return true;
}

//Snaps the supplied ball to the nearest value
void Dumbell::SnapPosition(double& ballPos)
{
    // Extreme right position might not be exactly reachable by adding multiples of gapSize_
    // This first part is a SPECIAL CASE check; if a ball is at that rightLimit_, do nothing.
   // if( ballPos == rightLimit_ ) return; // NO LONGER NEEDED WITH NEW ALGORITHM BELOW
    
    double roundedPosition = floor(((ballPos - leftLimit_) / gapSize_) + 0.5);
    ballPos = leftLimit_ + (roundedPosition * gapSize_);
}
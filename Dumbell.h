// Dumbell.h
#ifndef DUMBELL_H
#define DUMBELL_H

#include <windows.h>
#include <gdiplus.h>
#include <utility>

class BackBuffer;

/* The Dumbell class creates and maintains a stretchy dumbell-shaped object intended to mark a range
   on a scale, increasing in INTEGER values from LEFT TO RIGHT.
   The scale is ZERO-BASED, but setting lowestValue_ saves the client having to do the offset calculations.
*/
class Dumbell{
public:
    enum SizeType { RIGHTLIMIT, GAPSIZE, SCALEWIDTH };

    Dumbell();
    Dumbell(int numValues, int lowestValue = 0, double xPosition = 0.0, double yPosition = 0.0, 
            double size = 100.0, SizeType sizeType = RIGHTLIMIT,
            double ballDiameter = 5.0,
            Gdiplus::Color& colour = Gdiplus::Color(0,0,0));
    
    //Set Balls by screen position or scale value
    void SetLeftBall( const double pos );
    void SetLeftBall( const int value );
    void SetRightBall( const double pos );
    void SetRightBall( const int value );
    void SetBallPositions( const double leftPos, const double rightPos );
    void SetBallPositions( const int leftValue, const int rightValue );
    
    void SetYPosition( const double pos );
    void SetBallDiameter( const double diameter );
    
    void SetColour( const Gdiplus::Color& colour );
            
    std::pair<int,int> GetValues();
    void GetValues(int& left, int& right);
    
    double GetGapSize() const; // Retrieves gap size.
    
    bool Grab(const Gdiplus::PointF& mousePos); // returns false if neither ball grabbed
    bool Release(); // Returns false if nothing to release.
    void Display(BackBuffer& bb);
    void Update(const Gdiplus::PointF& mousePos);

private:
    void SnapPosition(double& ballPos);


private:
    int    lowestValue_; // Leftmost value.
    double leftBall_;   // x Position of left ball
    double rightBall_;   // x Position of right ball
    double yPosition_;  // y Position of Dumbell
    double ballDiameter_; // Size of balls
    double leftLimit_, rightLimit_; // extreme positions
    int numValues_;                 // how many positions the balls can stop in
    double gapSize_;                 // the gap between each position
    
    bool   isLeftBallGrabbed_;
    bool   isRightBallGrabbed_;
    
    Gdiplus::Color colour_;
};

#endif // BARBELL_H
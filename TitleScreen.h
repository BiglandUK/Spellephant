// TitleScreen.h

#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include "Mode.h"

class BackBuffer;

class TitleScreen : public Mode {
public:

TitleScreen(unsigned int& nextMode, unsigned int previousMode, unsigned int id, BackBuffer* bb);
virtual ~TitleScreen();

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
    Gdiplus::Image* pBackground_;

};


#endif // TITLESCREEN_H
// Slider.h
#ifndef SLIDER_H
#define SLIDER_H

#include <windows.h>
#include <gdiplus.h>

class BackBuffer;

class Slider{
public:
    Slider( Gdiplus::REAL length, Gdiplus::PointF& position );
    
    void Update( double ratio, const Gdiplus::PointF& mousePos );
    void Display( BackBuffer* bb );
    
    void LMBDown( const Gdiplus::PointF& mousePos );
    void LMBUp();
    
    double Ratio();
    
private:
    Gdiplus::REAL knobCentre_;
    Gdiplus::RectF knob_, slot_;
    bool grabbed_;
    Gdiplus::Color knobColour_, slotColour_;
};


#endif
// SLIDER_H
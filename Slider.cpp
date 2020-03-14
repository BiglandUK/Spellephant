// Slider.cpp

#include "Slider.h"
#include "BackBuffer.h"
#include "Utility.h"

using namespace Gdiplus;

Slider::Slider( Gdiplus::REAL length, Gdiplus::PointF& position )
    : slot_( position.X, position.Y, length, 10.0f ),
      knobCentre_( position.X ),
      knob_( position.X - 10.0f, position.Y - 20.f, 20.0f, 40.0f ),
      grabbed_(false ),
      slotColour_(100,100,100),
      knobColour_(150,150,200)
{}

void Slider::Update(double ratio, const Gdiplus::PointF &mousePos){
    if( grabbed_ ){
        knobCentre_ = mousePos.X;
    } else {
        knobCentre_ = ratio * slot_.Width + slot_.X;
    }
    if( knobCentre_ > slot_.X + slot_.Width )
        knobCentre_ = slot_.X + slot_.Width;
    if( knobCentre_ < slot_.X )
        knobCentre_ = slot_.X;

    knob_.X = knobCentre_ - ( 0.5f * knob_.Width );
}

void Slider::Display( BackBuffer* bb ){
    Gdiplus::Graphics graphics( bb->getDC() );
    int x = bb->mOffX;
    int y = bb->mOffY;
    // Adjust for offset
    RectF slot = slot_;
    slot.X += x;
    slot.Y += y;
    RectF knob = knob_;
    knob.X += x;
    knob.Y += y;
    
    graphics.FillRectangle(&SolidBrush( slotColour_ ), slot );
    graphics.FillRectangle( &SolidBrush( knobColour_ ), knob );

}

void Slider::LMBDown( const Gdiplus::PointF& mousePos ){
    if( ClickInRegion( &mousePos, &PointF( knob_.X, knob_.Y ), knob_.Width, knob_.Height ) ) 
        grabbed_ = true;
}

void Slider::LMBUp(){
    grabbed_ = false;
}

double Slider::Ratio(){
    return (knobCentre_ - slot_.X) / slot_.Width;
}
    
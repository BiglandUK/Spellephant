#include "TitleScreen.h"
#include "BackBuffer.h"
#include "App.h"

using namespace Gdiplus;

TitleScreen::TitleScreen(unsigned int& nextMode, unsigned int previousMode, unsigned int id, BackBuffer* bb):
Mode(nextMode, previousMode, id)
{
    pBackground_ = new Image(L"Images/Title1.jpg");
}

TitleScreen::~TitleScreen(){

delete pBackground_;
pBackground_ = 0;
}

void TitleScreen::Update( double dt, const Gdiplus::PointF* cursorPos){
}


void TitleScreen::Display(BackBuffer* bb){
    Graphics graphics(bb->getDC());
    graphics.DrawImage(pBackground_,RectF(static_cast<float>( bb->mOffX),
                                          static_cast<float>( bb->mOffY),
                                          static_cast<float>( bb->width() ),
                                          static_cast<float>( bb->height() )));
}

void TitleScreen::LMBUp(const Gdiplus::PointF* cursorPos){}
void TitleScreen::LMBDown(const Gdiplus::PointF* cursorPos, const double time){
    nextMode_ = MENU;
}
void TitleScreen::KeyDown(unsigned int key){
    nextMode_ = MENU;
}

void TitleScreen::KeyUp(){}
void TitleScreen::RMBUp(const Gdiplus::PointF* cursorPos){}
void TitleScreen::RMBDown(const Gdiplus::PointF* cursorPos){}
void TitleScreen::Wheel(short zDelta, Gdiplus::PointF* mousePos){}
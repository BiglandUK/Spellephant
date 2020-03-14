// Button.cpp

#include "Button.h"
using namespace Gdiplus;
using namespace std;

extern bool ClickInRegion(const Gdiplus::PointF* clickPoint,
                   const Gdiplus::PointF* regionStart,
                   const float regionWidth, const float regionHeight);

Button::Button(){}

Button::Button(BitmapPointer pBitmap, Gdiplus::PointF position,
               float displayWidth, float displayHeight,
               int defaultState, int startState, 
               Gdiplus::Color highlight, float highlightThickness) 
: position_(position)
, defaultState_(defaultState)
, state_(startState)
, displayWidth_(displayWidth)
, displayHeight_ (displayHeight)
, highlight_(highlight)
, highlightThickness_(highlightThickness)
, pBitmap_(pBitmap)
{
    buttonWidth_ = pBitmap_->GetWidth()/3;
    if( displayWidth_ == 0.0f )
        displayWidth_ = static_cast<float>(buttonWidth_);
    if( displayHeight_ == 0.0f )
        displayHeight_ = static_cast<float>(pBitmap_->GetHeight());
}

Button::Button(BitmapPointer pBitmap, Gdiplus::PointF position, int numStates,
               float displayWidth, float displayHeight,
               int defaultState, int startState, 
               Gdiplus::Color highlight, float highlightThickness)
: position_(position)
, defaultState_(defaultState)
, state_(startState)
, displayWidth_(displayWidth)
, displayHeight_ (displayHeight)
, highlight_(highlight)
, highlightThickness_(highlightThickness)
, pBitmap_(pBitmap)
{
    buttonWidth_ = pBitmap_->GetWidth()/numStates;
    if( displayWidth_ == 0.0f )
        displayWidth_ = static_cast<float>(buttonWidth_);
    if( displayHeight_ == 0.0f )
        displayHeight_ = static_cast<float>(pBitmap_->GetHeight());
}

Button::~Button() {}

void Button::Update(const PointF* mousePos) {
    if( state_ < Button::Normal || state_ == Button::Clicked ) return;
    
    if( ClickInRegion(mousePos, &position_, displayWidth_, displayHeight_) ) {
            state_ = Button::Hover;
    }    
    else    
        state_ = Button::Normal; // Expect problems when key is Clicked.
}

void Button::Display(BackBuffer* bb){
    if( state_ == Button::Hidden )
        return;
        
    Graphics graphic(bb->getDC());
    
    //Highlight required?
    if( state_ > Button::Normal ) {
 
            RectF hilite(position_.X - highlightThickness_ + bb->mOffX, position_.Y - highlightThickness_ + bb->mOffY,
                         displayWidth_ + (2.0f * highlightThickness_), displayHeight_ + (2.0f * highlightThickness_));
            graphic.DrawRectangle(&(Pen(highlight_, highlightThickness_)), hilite);
        }   
    
    RectF rect(position_.X + bb->mOffX,
               position_.Y + bb->mOffY,
               displayWidth_, displayHeight_); // Rectangle to display button in.
    
    REAL x = 0.0f;
    if(state_ == Button::Clicked) {
        x = static_cast<float>(buttonWidth_);
    }
    else if (state_ == Disabled) {
        x = static_cast<float>( buttonWidth_ * 2 );
    }
    graphic.DrawImage(pBitmap_, rect, x, 0.0f,
                       static_cast<float>( buttonWidth_ ), static_cast<float>( pBitmap_->GetHeight() ),
                       UnitPixel); // Draw Button
}

bool Button::Click(const Gdiplus::PointF* mousePos) {
    if( state_ < Button::Normal ) return false;
    
    if( ClickInRegion(mousePos, &position_, displayWidth_, displayHeight_) ) {

        state_ = Clicked;
        return true;
    }
    return false;

}

// Button only fires if cursor is still within region.
bool Button::Release(const Gdiplus::PointF* mousePos) {
    if( ClickInRegion(mousePos, &position_, displayWidth_, displayHeight_) &&
        state_ == Button::Clicked ) {
        state_ = Button::Normal;
        return true;
    }
    
    if( state_ == Button::Clicked) { state_ = Button::Normal; }  //Still have to clear the button if it was clicked before.
    
    return false;
}

void Button::ForceClick(){
    if( state_ > Button::Disabled ){
        state_ = Button::Clicked;
    }
}

void Button::ForceHighlight() {
    if( state_ > Button::Disabled ) {
        state_ = Button::Hover;
    }
}

void Button::SetPosition(const Gdiplus::PointF& pos) {
    position_ = pos;
}

void Button::SetSize(float displayWidth, float displayHeight) {
    displayWidth_ = displayWidth;
    displayHeight_ = displayHeight;
}

void Button::SetHighlightColour(const Gdiplus::Color colour){
    highlight_ = colour;
}
 
void Button::SetHighlightThickness(const float thickness){
    highlightThickness_ = thickness;
}

void Button::Disable() {
    state_ = Button::Disabled;
}

void Button::Enable() {
    if( state_ < Button::Normal ) {
        state_ = Button::Normal;
    }
}

void Button::Hide() {
    state_ = Button::Hidden;
}

void Button::Show() {
    state_ = Button::Normal;
}

void Button::ResetState() {
    state_ = defaultState_;
}

float Button::Height() const {
    return displayHeight_;
}

float Button::Width() const {
    return displayWidth_;
}

int Button::GetState() const {
    return state_;
}

// ToggleButton

ToggleButton::ToggleButton() {}

ToggleButton::ToggleButton(BitmapPointer pBitmap, Gdiplus::PointF position,
               float displayWidth, float displayHeight,
               int defaultState, int startState, 
               Gdiplus::Color highlight, float highlightThickness,
               int maxButtons, int currentButton, int initialButton)
: Button(pBitmap, position, displayWidth, displayHeight, defaultState, startState,
         highlight, highlightThickness)
, maxButtons_(maxButtons)
, currentButton_(currentButton)
, initialButton_(initialButton)
{
    if( maxButtons_ < 1 ) maxButtons_ = 1;
    if( currentButton_ < 1 ) currentButton_ = 1;
    if( currentButton_ > maxButtons_ ) currentButton_ = maxButtons_;
    if( displayHeight_ == pBitmap_->GetHeight() &&
        maxButtons_ > 1 ) {
        displayHeight_ /= maxButtons_;
    }
}

ToggleButton::ToggleButton(BitmapPointer pBitmap, Gdiplus::PointF position, int numStates,
            float displayWidth, float displayHeight,
            int defaultState, int startState,
            Gdiplus::Color highlight, float highlightThickness,
            int maxButtons, int currentButton, int initialButton ) 
: Button(pBitmap, position, numStates, displayWidth, displayHeight, defaultState, startState,
         highlight, highlightThickness)
, maxButtons_(maxButtons)
, currentButton_(currentButton)
, initialButton_(initialButton)
{
    if( maxButtons_ < 1 ) maxButtons_ = 1;
    if( currentButton_ < 1 ) currentButton_ = 1;
    if( currentButton_ > maxButtons_ ) currentButton_ = maxButtons_;
    if( displayHeight_ == pBitmap_->GetHeight() &&
        maxButtons_ > 1 ) {
        displayHeight_ /= maxButtons_;
    }
}

void ToggleButton::Display(BackBuffer* bb){
    if( state_ == Button::Hidden )
        return;
        
    Graphics graphic(bb->getDC());
    
    //Highlight required?
    if( state_ > Button::Normal ) {
            RectF hilite(position_.X - highlightThickness_ + bb->mOffX, position_.Y - highlightThickness_ + bb->mOffY,
                         displayWidth_ + (2.0f * highlightThickness_), displayHeight_ + (2.0f * highlightThickness_));
            graphic.DrawRectangle(&(Pen(highlight_, highlightThickness_)), hilite);
        }   
    
    RectF rect(position_.X + bb->mOffX,
               position_.Y + bb->mOffY,
               displayWidth_, displayHeight_); // Rectangle to display button in.
    
    REAL x = 0.0f;
    if(state_ == Button::Clicked) {
        x = static_cast<float>(buttonWidth_);
    }
    else if (state_ == Disabled) {
        x = static_cast<float>( buttonWidth_ * 2 );
    }
    
    REAL y = static_cast<float>( (pBitmap_->GetHeight()/maxButtons_) * (currentButton_ - 1) );
    
    graphic.DrawImage(pBitmap_, rect, x, y,
                       static_cast<float>( buttonWidth_ ), static_cast<float>( pBitmap_->GetHeight() / maxButtons_ ),
                       UnitPixel); // Draw Button
}

// Uses Button::Release
bool ToggleButton::Release(const Gdiplus::PointF* mousePos) {
    if( Button::Release(mousePos) ) {
        ++currentButton_; // Advanced to next image
        if( currentButton_ > maxButtons_ ) currentButton_ = 1; //Back to first image if gone past last.
        return true;
    }
    return false;
}

int ToggleButton::GetCurrentButton() {
    return currentButton_;
}

void ToggleButton::ResetState() {
    Button::ResetState();
    currentButton_ = initialButton_;
}
// Button.h
#ifndef BUTTON_H
#define BUTTON_H

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <string>
#include "BackBuffer.h"

/*
 Bitmap is stored in client class, meaning multiple buttons can use the same image.
 Disable highlight by setting alpha colour to 0.
 Buttons are meant to consist of three pictures, horizontally aligned in the same bitmap.
 The first image is the normal button, the second is the button in a depressed position,
 and the third image is the disabled version of the button.
 Since some buttons won't need a disabled version, use the constructor with numstates,
 setting numstates to 1 or 2.
*/
class Button{
public:
    typedef Gdiplus::Bitmap* BitmapPointer;
    enum {Hidden, Disabled, Normal, Hover, Clicked };
    Button();
    Button(BitmapPointer pBitmap, Gdiplus::PointF position,
            float displayWidth = 0.0f, float displayHeight = 0.0f,
            int defaultState = Normal, int startState = Normal,
            Gdiplus::Color highlight = Gdiplus::Color(255,255,0), float highlightThickness = 5.0f);
    Button(BitmapPointer pBitmap, Gdiplus::PointF position,
            int numStates,
            float displayWidth = 0.0f, float displayHeight = 0.0f,
            int defaultState = Normal, int startState = Normal,
            Gdiplus::Color highlight = Gdiplus::Color(255,255,0), float highlightThickness = 5.0f);         
    
    virtual ~Button();
    
    virtual void Update(const Gdiplus::PointF* mousePos);
    virtual void Display(BackBuffer* bb);
    
    virtual bool Click (const Gdiplus::PointF* mousePos); // Check for a click on the button.
    virtual bool Release(const Gdiplus::PointF* mousePos); // Release the button. Returns true if button was clicked AND cursor still on button.

    virtual void ForceClick(); // Force a button (as long as it's enabled) into a Clicked state, regardless of the mouse position.
    virtual void ForceHighlight(); // As above, but Hover state.

    virtual void SetPosition(const Gdiplus::PointF& pos );
    virtual void SetSize( float displayWidth, float displayHeight );
    
    virtual void SetHighlightColour(const Gdiplus::Color colour);
    virtual void SetHighlightThickness(const float thickness);
    
    virtual void Disable();
    virtual void Enable();
    
    virtual void Hide();
    virtual void Show();
    
    virtual void ResetState();
    
    virtual float Height() const; // Returns DISPLAYED height
    virtual float Width() const;  // Returns DISPLAYED width
    virtual int   GetState() const; // Returns state of button

private:
    //Button(const Button& b); // Copy Constructor
    //Button operator=(const Button& rhs); // assignment operator
    
protected:
    int state_;                         // See ButtonState for information.
    int defaultState_;                 // The original status.
    Gdiplus::PointF position_;               // Position for this button.
    float displayWidth_, displayHeight_;
    BitmapPointer pBitmap_;
    int buttonWidth_;            // Actual width (pixels) of the button graphics
    Gdiplus::Color highlight_; // Highlight colour
    float highlightThickness_;
};


/* The Toggle button is a variation on the standard Button.
   This has several button images all joined together in the same bitmap.
   Each click of the button moves it to the next row's image.
   
   The provided image must have as many button image rows as maxButton_.
*/
class ToggleButton : public Button {
public:
    ToggleButton();
    ToggleButton(BitmapPointer pBitmap, Gdiplus::PointF position,
            float displayWidth = 0.0f, float displayHeight = 0.0f,
            int defaultState = Normal, int startState = Normal,
            Gdiplus::Color highlight = Gdiplus::Color(255,255,0), float highlightThickness = 5.0f,
            int maxButtons = 1, int currentButton = 1, int initialButton = 1);
    ToggleButton(BitmapPointer pBitmap, Gdiplus::PointF position, int numStates,
            float displayWidth = 0.0f, float displayHeight = 0.0f,
            int defaultState = Normal, int startState = Normal,
            Gdiplus::Color highlight = Gdiplus::Color(255,255,0), float highlightThickness = 5.0f,
            int maxButtons = 1, int currentButton = 1, int initialButton = 1);
    
    virtual void Display(BackBuffer* bb);
    
    virtual bool Release(const Gdiplus::PointF* mousePos);   // Release the button AND move button onto next image. Returns true if button was clicked.
    
    virtual void ResetState();
    
    virtual int GetCurrentButton(); // NOT zero-based!

private:
    ToggleButton(const ToggleButton& b); // Copy Constructor
    ToggleButton operator=(const ToggleButton& rhs); // assignment operator
    
protected:
    int maxButtons_;    // Stores maximum number of button types in cycle.
    int currentButton_; // Stores the current button (therefore vertical position within bitmap)
    int initialButton_; // Stores the original button choice (for ResetState)
};

#endif // BUTTON_H
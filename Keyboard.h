// Keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <fstream>
#include "BackBuffer.h"

// class Key

enum KeyStatus {Normal, Disabled, Hover, Clicked};
class Keyboard;

class Key {
    friend Keyboard;

public:
    Key();
    Key(int row, int col, wchar_t current, wchar_t alternate, KeyStatus status = Normal, std::wstring text=L"", bool displayIsChar = true, int scaleWidth=1, int scaleHeight=1);
    
    // Returns true if passed character is represented on this key, either in standard or alternate state.
    bool RepresentsCharacter( const wchar_t character ) const;

protected:
    void SetCase(bool upper=false);    
    void Update(const Gdiplus::PointF* mouse);
    void Calculate(float standardWidth, float standardHeight,
                   float keyGap, float rowGap, float offset, Gdiplus::PointF position );
    void SetPosition( Gdiplus::PointF pos );
    void SetSize( float standardWidth, float standardHeight, float keyGap, float rowGap );
    void ChangeCase();
    void Disable();
    void Enable();
    void ResetStatus();
    
    void Click (const Gdiplus::PointF* mousePos, wchar_t& enteredKey); // Key is clicked
    void Release(); // Key is released
    void Keystroke();


protected:

    wchar_t mCurrent, mAlternate;       // Stores the characters the key is for.
    std::wstring mDisplayText;          // Stores the text to display on the key.
    int mStatus;                        // See KeyStatus for information.
    int mDefaultStatus;                 // The original status - used for temporarily disabling a key.
    Gdiplus::PointF mPos;               // Position for this key - calculated as required.
    int mScaleWidth, mScaleHeight;      // How many standard keys wide and high it is (including gaps)
    float mWidth, mHeight;              // Actual height and width (pixels)
    bool mDisplayTextIsChar;            // Whether the text on the key is the character to enter when pressed.
    int mRow, mCol;                     // Position on the keyboard, if the keyboard were a grid.
    
};

typedef std::map<char,Key> KeyList;

class Keyboard {

public:
    // Constructors
    Keyboard();
    
    //Destructor
    ~Keyboard();
    
    /*  Key Graphics - set the bitmap file to use, the positions of the keys (they must be of equal
        dimensions and placed in a horizontal line), the colour of the text for each state and the
        colour of the highlight.
        Returns FALSE if graphics file fails.
    */ 
    bool SetGraphics(const std::wstring& fileName,
                      Gdiplus::REAL xPosNormal, Gdiplus::REAL xPosDisable, Gdiplus::REAL xPosClicked,
                      Gdiplus::REAL yPos, Gdiplus::REAL keyWidth, Gdiplus::REAL keyHeight,
                      Gdiplus::Color normalText, Gdiplus::Color disableText, Gdiplus::Color clickedText,
                      Gdiplus::Color highlightColour );

    // Alter size of keyboard
    void SetKeySize( const float width, const float height );
    void SetKeyGap ( const float gap );
    void SetRowGap ( const float gap );
    void SetPosition( const Gdiplus::PointF& pos );
    void SetFont( Gdiplus::Font* font);
    void SetFont( const Gdiplus::FontFamily* fontFamily, Gdiplus::REAL size );
    
    // Add to keyboard
    void AddRow(float offset = 0.0f);
    void AddKey(const Key& key);
    void AddKey(int row, int col,
                wchar_t current, wchar_t alternate, KeyStatus status = Normal,
                std::wstring text=L"", bool displayIsChar = true,
                int scaleWidth=1, int scaleHeight=1);
    
    // "In loop" functions
    void Update (const Gdiplus::PointF* mousePos);
    void Display( BackBuffer* bb );
    
    
    void CalculateKeys();   // (Re)calculates all key positions.
    void ChangeCase();      // Switch case of all keys - effectively swaps the current and alternate chars.
    bool IsUpper();         // Returns true if keyboard currently uppercase
   
    wchar_t KeyClick (const Gdiplus::PointF* mousePos); // Check for a click on a key.
    void    KeyRelease(); // Release a key.
    void Keystroke( unsigned int key );           // Mirror hardware key press.
    
    void HideKeyboard ();
    void ShowKeyboard ();
    void DisableAllKeys();  // Disables all keys.
    void EnableAllKeys();   // Sets ALL keys to Normal.
    void ResetKeys();       // Set all keys to their original status.
    
    bool DisableKey( wchar_t keyChar ); // These functions check if a key exists,
    bool EnableKey ( wchar_t keyChar ); // returning true if found, as well as changing the status.
    bool ResetKey  ( wchar_t keyChar ); // Return false if key isn't found.
    
    /* Load Keyboard from text file
        The txt file must be saved in "Unicode - Codepage 1200" encoding.
        To create a new row, type "newrow".  To offset the row, add a number (decimal) after "newrow".
        Each key has its data on a new line, separated by commas.
        To create keys on that row, the minimum to enter is a single lowercase character in quotes (single or double).
        For specific uppercase characters (and special characters), it must be followed by the
         uppercase character, also in quotes.
        Special codes (such as ENTER and BACKSPACE, as well as COMMA due to its use as separation character)
         must be entered as their code numbers (without quotes):
         ENTER (CR) = 13
         BACKSPACE = 8
         COMMA = 44
        The next data item is the status - enter "Disabled" (no quotes) to disable the key.  Anything else is interpreted
         as Normal.
        The next data item is the display text, if required.  Use this to set separate text than the characters.
        The final two data items are the width then height of the key, measured in standard key sizes (integers).
        EXAMPLE:
        "a","A",Disabled,"a",1,1    
    */
    bool LoadKeyboard( const std::string fileName );
    
private:
    bool KeyExists ( wchar_t& keyChar ); // Used to check if a key exists.
    std::wstring GetNextValue( std::wstring& str ); // Extracts information from a keyboard textfile
    bool Keyboard::BuildKey( wchar_t& current, wchar_t& alternate,
                         KeyStatus& status, std::wstring& displayText, bool& displayIsChar,
                         int& scaleWidth, int& scaleHeight, std::wstring& str );
    
public:
    KeyList mKeys; // stores each key
    std::vector<float> mRowOffsets; // stores the offsets for each row.  Offsets are a proportion of StandardKeyWidth.
    bool mUppercase;            // If true, keyboard is showing uppercase chars.

private:
    Gdiplus::Bitmap* mKeyGfx;                                    // Stores the key graphics
    Gdiplus::REAL mBrushXNormal, mBrushXDisable, mBrushXClick;   // Position to cut for specific key graphic
    Gdiplus::REAL mBrushY, mBrushWidth, mBrushHeight;            // Pixel locations of graphic boundaries
    Gdiplus::Color mTextNormal, mTextDisable, mTextClick, mHighlight; // Text & highlight colour for different key statuses
    float mHighlightThickness;

    Gdiplus::PointF mPosition;  // top left pixel of keyboard
    float mStandardKeyWidth;    // standard gfx width - adjust this to resize keys
    float mStandardKeyHeight;   // standard gfx height - adjust this to resize keys
    Gdiplus::FontFamily* mpFontFamily;
    Gdiplus::Font* mFont;        // font for text.
    float mRowGap;              // Gap between each row - proportion of StandardKeyHeight.
    float mKeyGap;              // Gap between each key - proportion of StandardKeyWidth.
    
    bool mUpdated;              // False means key positions need recalculating.
    bool mVisible;              // Whether entire keyboard is displayed.
    
    bool mShiftHeld;

};

#endif // KEYBOARD_H
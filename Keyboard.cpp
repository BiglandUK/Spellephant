#include "Keyboard.h"
//#include "resource.h"
#include <windowsx.h> // Needed for GetWindowInstance
#include <algorithm>
#include <sstream>
#include "Definitions.h"



using namespace Gdiplus;
using namespace std;

extern std::wstring ToWString(const std::string& s); // In main.cpp

Key::Key() {}

Key::Key(int row, int col, wchar_t current, wchar_t alternate, KeyStatus status, std::wstring text, bool displayIsChar, int scaleWidth, int scaleHeight)
: mRow(row), mCol(col), mCurrent(current), mAlternate(alternate), mDisplayText(text), 
  mStatus(status), mDefaultStatus(status), 
  mDisplayTextIsChar(displayIsChar),
  mScaleWidth(scaleWidth), mScaleHeight(scaleHeight) {
  
  if( mDisplayTextIsChar )
      mDisplayText = mCurrent;  

}

bool Key::RepresentsCharacter( const wchar_t character) const{
    return (mCurrent == character || mAlternate == character);
}


void Key::Update(const Gdiplus::PointF* mouse) {
    if( mStatus == Disabled || mStatus == Clicked ) return; // Do nothing for disabled or clicked keys.
                                                          // Clicked keys are dealt with in other ways.
    
    if( mouse->X >= mPos.X && mouse->Y >= mPos.Y &&
        mouse->X <= mPos.X + mWidth && mouse->Y <= mPos.Y + mHeight) {
            mStatus = Hover;
    }    
    else    
        mStatus = Normal; // Expect problems when key is Clicked.
}

void Key::SetPosition( Gdiplus::PointF pos ) {
    mPos = pos;
}

void Key::SetSize( float standardWidth, float standardHeight, float keyGap, float rowGap ) {
    mWidth = standardWidth * (mScaleWidth + (keyGap * (mScaleWidth - 1)));
    mHeight = standardHeight * (mScaleHeight + (rowGap * (mScaleHeight - 1)));
}

void Key::ChangeCase() { swap(mCurrent, mAlternate); }

void Key::Click(const Gdiplus::PointF* mousePos, wchar_t &enteredKey) {
    if( mStatus == Disabled ) return;
    
    if( mousePos->X >= mPos.X && mousePos->Y >= mPos.Y &&
        mousePos->X <= mPos.X + mWidth && mousePos->Y <= mPos.Y + mHeight ) {
        
        enteredKey = mCurrent;
        mStatus = Clicked;
    }
}

void Key::Calculate(float standardWidth, float standardHeight,
                    float keyGap, float rowGap, float offset, PointF position ) {
    SetSize(standardWidth, standardHeight, keyGap, rowGap);
    PointF p;
    p.X = position.X + (offset * standardWidth) + (mCol - 1)*(standardWidth*(keyGap + 1));
    p.Y = position.Y + (mRow - 1) * (standardHeight*(rowGap + 1));
        
    SetPosition(p);
}


void Key::Release () {
    if( mStatus == Clicked ) {
        mStatus = Normal;
    }
}

void Key::Keystroke() {
    if( mStatus != Disabled ) {
        mStatus = Clicked;
    }
}

void Key::Disable() {
    mStatus = Disabled;
}

void Key::Enable() {
    mStatus = Normal;
}

void Key::ResetStatus() {
    mStatus = mDefaultStatus;
}


// class Keyboard

Keyboard::Keyboard()
: mUpdated(false), mUppercase(false), mVisible(true), mFont(0),
  mStandardKeyWidth(50.0f), mStandardKeyHeight(50.0f), mKeyGap(0.1f), mRowGap(0.1f),
  mPosition(PointF(0.0f, 0.0f)), mKeyGfx(0),
  mBrushXNormal(0.0f), mBrushXDisable(20.0f), mBrushXClick(40.0f),
  mBrushY(0.0f), mBrushWidth(19.0f), mBrushHeight(19.0f), mHighlightThickness(0.0f),
  mTextNormal(Color(0,0,0)), mTextDisable(Color(200,200,200)), mTextClick(Color(255,255,0)),
  mHighlight(Color(0,0,0)),
  mShiftHeld(false)
  {}
 
 Keyboard::~Keyboard() {
    if( mKeyGfx ) {
        delete mKeyGfx;
        mKeyGfx = 0;
    }
 }

bool Keyboard::SetGraphics(const wstring& fileName,
                      Gdiplus::REAL xPosNormal, Gdiplus::REAL xPosDisable, Gdiplus::REAL xPosClicked,
                      Gdiplus::REAL yPos, Gdiplus::REAL keyWidth, Gdiplus::REAL keyHeight,
                      Gdiplus::Color normalText, Gdiplus::Color disableText, Gdiplus::Color clickedText,
                      Gdiplus::Color highlightColour ) {
    
    if( !mKeyGfx )
        delete mKeyGfx; // Key graphic already exists - delete
    mKeyGfx = new Bitmap( fileName.c_str() );
    if( mKeyGfx->GetLastStatus() != Gdiplus::Ok )
        return false;    
    
    mBrushXNormal = xPosNormal;
    mBrushXDisable = xPosDisable;
    mBrushXClick = xPosClicked;
    
    mBrushY = yPos;
    mBrushWidth = keyWidth;
    mBrushHeight = keyHeight;
    
    mTextNormal = normalText;
    mTextDisable = disableText;
    mTextClick = clickedText;
    mHighlight = highlightColour;
    
                      
    return true;                     
}


void Keyboard::SetKeySize(const float width, const float height ) {
    mStandardKeyWidth = width;
    mStandardKeyHeight = height;
    mUpdated = false;
 }

void Keyboard::SetKeyGap(const float gap) {
    mKeyGap = gap;
    mUpdated = false;
}

void Keyboard::SetRowGap(const float gap) {
    mRowGap = gap;
    mUpdated = false;
}

void Keyboard::SetPosition(const Gdiplus::PointF &pos) {
    if( mPosition.X == pos.X && mPosition.Y == pos.Y ) return;
    mPosition = pos;
    mUpdated = false;
}

void Keyboard::SetFont(Gdiplus::Font* font) {
    mFont = font;
    mUpdated = false;
}

// I think there's a leak here, but Font and FontFamily are so fucked up, I don't know what else to do.
void Keyboard::SetFont( const Gdiplus::FontFamily* fontFamily, Gdiplus::REAL size ) {
    if( mFont ) delete mFont;
    mFont = new Font( fontFamily, size );
    mUpdated = false;
}

void Keyboard::AddRow(float offset) {
    mRowOffsets.push_back(offset);
    mUpdated = false;
}

void Keyboard::AddKey(const Key& key) {
    mKeys[key.mCurrent] = key;
    mUpdated = false;
}

void Keyboard::AddKey(int row, int col,
                wchar_t current, wchar_t alternate, 
                KeyStatus status, std::wstring text, bool displayIsChar,
                int scaleWidth, int scaleHeight) {
    mKeys[current] = Key(row, col, current, alternate,  status, text, displayIsChar, scaleWidth, scaleHeight);
}

void Keyboard::Update(const Gdiplus::PointF* mousePos) {
    KeyList::iterator iter;
    for(iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
        iter->second.Update(mousePos);
    }
    
    // Checks keyboard case matches caps lock and shift positions
    if ( mUppercase == (GetKeyState(VK_CAPITAL) & 0x0001) == mShiftHeld ){
        ChangeCase();
    }
}

void Keyboard::Display(BackBuffer *bb) {
    if( !mKeyGfx || !mVisible || !mFont ) return;
    // Ensure keys size and position are up to date
    if( !mUpdated )
        CalculateKeys();  
    Graphics graphics(bb->getDC());
    KeyList::iterator iter;
    for( iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
        // Highlight required?
        if( iter->second.mStatus == Hover || iter->second.mStatus == Clicked ) {
            RectF hilite(iter->second.mPos.X - mHighlightThickness + bb->mOffX, iter->second.mPos.Y - mHighlightThickness + bb->mOffY,
                         iter->second.mWidth + (2.0f*mHighlightThickness), iter->second.mHeight + (2.0f*mHighlightThickness));
            graphics.DrawRectangle(&(Pen(mHighlight, mHighlightThickness)), hilite);
        }   

        // Actual Key
        RectF rect(iter->second.mPos.X+bb->mOffX, iter->second.mPos.Y+bb->mOffY, iter->second.mWidth, iter->second.mHeight); // Rectangle for key
        // Determine which part of KeyGfx to cut out, and which text colour to use.
        REAL x = mBrushXNormal;
        Color c = mTextNormal;
        
        if(iter->second.mStatus == Clicked) {
            x = mBrushXClick;
            c = mTextClick;
        }
        else if (iter->second.mStatus == Disabled) {
            x = mBrushXDisable;
            c = mTextDisable;
        }

        graphics.DrawImage(mKeyGfx, rect, x, mBrushY, mBrushWidth, mBrushHeight, UnitPixel); // Draw Key
        graphics.DrawRectangle(&(Pen(Color(0,0,0))),rect); // Border
        
        // Text on top
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        format.SetTrimming(Gdiplus::StringTrimmingNone);
        SolidBrush brush(c);
        if( iter->second.mDisplayTextIsChar)
            graphics.DrawString(&(iter->second.mCurrent), 1, mFont, rect, &format, &brush); // Show the "current" character
        else
            graphics.DrawString(iter->second.mDisplayText.c_str(), -1, mFont, rect, &format, &brush); // Show the display text
    }
}

void Keyboard::CalculateKeys() {

    KeyList::iterator keyIter;
    
    for(keyIter = mKeys.begin(); keyIter != mKeys.end(); ++keyIter) {
        float offset = 0.0f;
        if( keyIter->second.mRow <= static_cast<int>( mRowOffsets.size() ) )
            offset = mRowOffsets[(keyIter->second.mRow) - 1];
        keyIter->second.Calculate(mStandardKeyWidth, mStandardKeyHeight,
                           mKeyGap, mRowGap, offset, mPosition);
    }

    // Update the highlight thickness - this is half the width of the row or key gap,
    // whichever is the smaller.
    mHighlightThickness = ( (mKeyGap * mStandardKeyWidth) <= (mRowGap * mStandardKeyHeight) ? 
                                 (mKeyGap * mStandardKeyWidth) : (mRowGap * mStandardKeyHeight) )
                                 * 0.5f ;
    
    mUpdated = true;
}

void Keyboard::ChangeCase() {
    KeyList::iterator keyIter;
    
    for(keyIter = mKeys.begin(); keyIter != mKeys.end(); ++keyIter){
        keyIter->second.ChangeCase();
    }
    mUppercase = !mUppercase;
}

bool Keyboard::IsUpper() { return mUppercase; }

wchar_t Keyboard::KeyClick(const Gdiplus::PointF* mousePos) {
    wchar_t key = '\0';
    KeyList::iterator keyIter;
    for(keyIter = mKeys.begin(); keyIter != mKeys.end(); ++keyIter) {
        keyIter->second.Click(mousePos, key);
        if( key != '\0' )
            return key;
    }
    return key;
}

void Keyboard::KeyRelease() {
    KeyList::iterator keyIter;
    
    if( mShiftHeld ){
        mShiftHeld = false;
        ChangeCase();
    }
    
    for(keyIter = mKeys.begin(); keyIter != mKeys.end(); ++keyIter) {
        keyIter->second.Release();
    }
}

void Keyboard::Keystroke(unsigned int key) {
    // Shift check
    if( key == SHIFT && !mShiftHeld ) {
        mShiftHeld = true;
        ChangeCase();
    }
    // Changed the line below as it was failing to identify lowercase characters.
    wchar_t keyChar = static_cast<wchar_t>(key);//MapVirtualKey(key, MAPVK_VK_TO_CHAR);

    if( KeyExists(keyChar) ) // Note: keyChar may be changed by KeyExists, to the relevant map key...
        mKeys[keyChar].Keystroke();  // ...so that this line doesn't create a new member of map!
}

void Keyboard::HideKeyboard() {
    mVisible = false;
}

void Keyboard::ShowKeyboard() {
    mVisible = true;
}

void Keyboard::DisableAllKeys() {
    KeyList::iterator iter;
    for(iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
        iter->second.Disable();
    }
}

void Keyboard::EnableAllKeys() {
    KeyList::iterator iter;
    for(iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
        iter->second.Enable();
    }
}

void Keyboard::ResetKeys() {
    KeyList::iterator iter;
    for(iter = mKeys.begin(); iter != mKeys.end(); ++iter ) {
        iter->second.ResetStatus();
    }
}

bool Keyboard::DisableKey( wchar_t keyChar ) {
    if ( KeyExists( keyChar ) ) {
        mKeys[keyChar].Disable();
        return true;
    }
    return false;
}

bool Keyboard::EnableKey( wchar_t keyChar ) {
    if( KeyExists( keyChar ) ) {
        mKeys[keyChar].Enable();
        return true;
    }
    return false;
}

bool Keyboard::ResetKey(wchar_t keyChar) {
    if( KeyExists( keyChar ) ) {
        mKeys[keyChar].ResetStatus();
        return true;
    }
    return false;
}

// Private Functions
bool Keyboard::KeyExists( wchar_t& keyChar ) {
    size_t check = mKeys.count(keyChar); // see if there's a (map) key with this character
    
    //If that failed, see if the character is at least on the key
    if ( !check ) {
        for( KeyList::const_iterator iter = mKeys.begin();
             iter != mKeys.end() && !check;
             ++iter ){
            check = iter->second.RepresentsCharacter( keyChar );
            if( check )
                keyChar = iter->first; // Important - keyChar is set to the map key value for this key.
        }                              // It's used by the calling function KeyStroke.
    }
    
    if( check )
        return true;
    return false;
}

bool Keyboard::LoadKeyboard( const std::string fileName ) {
    // Get text file
    ifstream inFile(fileName.c_str());
    if( !inFile ) return false; // Couldn't load file.
    
    // Extract unicode text and store in wstring
    stringstream ss;
    ss << inFile.rdbuf() << '\0';
    wstring ws = wstring((wchar_t *)ss.str().c_str());
    
    if( ws.length() <= 1 ) return false; // not enough content for a meaningful keyboard!
    
    // Remove BOM marker from first position
    ws = ws.substr(1);
    
    // Set up key location variables
    int row, col;
    row = 0;
    col = 1;
    
    while( !ws.empty() ) {
        // Get a complete line from the text
        size_t eol = ws.find(L'\n');
        //if( eol == string::npos ) return false; // End of line not found.
        wstring sub = ws.substr(0,eol-1); //  copy everything except the carriage return
        ws = ws.substr(eol+1);      // Cut off this line (doesn't work for last line, hence the check below)
        if( ws == sub ) ws.clear(); // Last line has been read.
        // Check for "newrow"
        if ( sub.length() >= 6 && sub.substr(0,6) == L"newrow") {
            ++row;
            if( sub.length() >= 7 )
                sub = sub.substr(7);
            else
                sub.clear();
            float f = static_cast<float>( _wtof( sub.c_str() ) );
            AddRow(f);
            col = 1; // reset col to first position
        }
        else if( !sub.empty() ) { // Create a key
            if( row == 0 ) { ++row; AddRow(); } // Insert the first row, if the text file omits it.
            
            // Set up default Key variables
            wchar_t current =  L'';
            wchar_t alternate = L'';
            KeyStatus status = Normal;
            wstring displayText = L"";
            bool displayIsChar = true;
            int scaleWidth = 1;
            int scaleHeight = 1;
            if( BuildKey(current, alternate, status, displayText, displayIsChar, scaleWidth, scaleHeight, sub) ) {
                AddKey(row, col, current, alternate, status, displayText, displayIsChar, scaleWidth, scaleHeight);
                col = col + scaleHeight; // Advance col counter.
            }
        }   
    }
    return true;
}

wstring Keyboard::GetNextValue(std::wstring &str) {
    if( str.empty() ) return L"\0";
    
    size_t cutPos = str.find(L',');
    wstring value = str.substr(0, cutPos);
    if( cutPos == string.npos ) {
        str.clear();
    }
    else {
        str = str.substr(cutPos+1);
    }
    return value;
}

bool Keyboard::BuildKey( wchar_t& current, wchar_t& alternate,
                         KeyStatus& status, wstring& displayText, bool& displayIsChar,
                         int& scaleWidth, int& scaleHeight, wstring& str ) {

    // Find current character
    wstring temp = GetNextValue(str);
    if( temp.empty() ) return false; // No data found, so no key to build.
    if( temp[0] == '\'' || temp[0] == '\"' ) {// Quotes mean the character is explicitly given
        current = temp[1];
        displayText = current; // Set display text, in case there isn't one later.
    }
    else {
        current = (wchar_t)(_wtoi(temp.c_str()));  // Lack of quotes means a control character
    }
    
    // Find alternate character
    temp = GetNextValue(str);
    if( temp.empty() ) {
        alternate = toupper(current);
        return true; // Key only contains "current character"
    }
    if( temp[0] == '\'' || temp[0] == '\"' )
        alternate = temp[1];
    else
        alternate = (wchar_t)(_wtoi(temp.c_str() ) );
 
    //Find KeyStatus
    temp = GetNextValue(str);
    if( temp.empty() ) return true; // No more data found.
    if( temp == L"Disabled" )
        status = Disabled;

    //Find Display Text (and set displayIsChar)
    temp = GetNextValue(str);
    if( temp.empty() ) return true; // No more data found;
    displayText = temp;
    // Strip any quote marks.
    displayText.erase( remove( displayText.begin(), displayText.end(), L'\"'), displayText.end() );
    displayText.erase( remove( displayText.begin(), displayText.end(), L'\''), displayText.end() );
    // Check if display text matches current character
    wstring check = L"";
    check = current; 
    if( check != displayText )
        displayIsChar = false; // If not, the display text is not the character.
    
    // Find width
    temp = GetNextValue(str);
    if( temp.empty() ) return true; // No more data found;
    scaleWidth = _wtoi(temp.c_str());
    
    // Find height
    temp = GetNextValue(str);
    if( temp.empty() ) return true; // No more data found;
    scaleHeight = _wtoi(temp.c_str());

    return true;
}
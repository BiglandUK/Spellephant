// ScreenPrinter.h

#ifndef SCREENPRINTER
#define SCREENPRINTER

#include <windows.h>
#include <gdiplus.h>
#include <string>

class BackBuffer;

class ScreenPrinter{
public:

    ScreenPrinter(BackBuffer* bb);
    
    // Single Letter Printing:
    // Uses compact printing
    Gdiplus::PointF PrintLetter(wchar_t letter, Gdiplus::PointF& position, Gdiplus::Font& font,
                           Gdiplus::Color& colour);
    // Caller specifies string format
    Gdiplus::PointF PrintLetter(wchar_t letter, Gdiplus::PointF& position, const Gdiplus::Font& font,
                           const Gdiplus::Color& colour, Gdiplus::StringFormat& format);                           
    // Uses a capital W to determine a fixed width
    Gdiplus::PointF PrintLetterFixedWidth(wchar_t letter, Gdiplus::PointF& position, Gdiplus::Font& font,
                           Gdiplus::Color& colour, Gdiplus::REAL width);
                           
    // String Printing:
    // Uses compact printing
    void PrintTruncString(const std::wstring string, const Gdiplus::PointF& position, const Gdiplus::Font& font,
                        const Gdiplus::Color& colour, const Gdiplus::RectF& targetRec);
           
public:
    BackBuffer* bb_;
    int xOff_, yOff_;
};


#endif // SCREENPRINTER
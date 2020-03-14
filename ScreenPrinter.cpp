// ScreenPrinter.cpp
#include "ScreenPrinter.h"
#include <windows.h>
#include <gdiplus.h>
#include "BackBuffer.h"
#include <string>

using namespace Gdiplus;
using namespace std;

ScreenPrinter::ScreenPrinter(BackBuffer* bb)
: bb_(bb)
, xOff_(bb->mOffX)
, yOff_(bb->mOffY)
{}


Gdiplus::PointF ScreenPrinter::PrintLetter(wchar_t letter, Gdiplus::PointF& position, Gdiplus::Font& font,
                                        Gdiplus::Color& colour) {
    Gdiplus::StringFormat sfNormal(Gdiplus::StringFormat::GenericTypographic());
    sfNormal.SetFormatFlags(Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
    
    return PrintLetter(letter, position, font, colour, sfNormal);                                                                             
}

Gdiplus::PointF ScreenPrinter::PrintLetter(wchar_t letter, Gdiplus::PointF& position, const Gdiplus::Font& font,
                                        const Gdiplus::Color& colour, Gdiplus::StringFormat& format) {
    std::wstring ws(1,  letter);
    Gdiplus::PointF pos(position.X + xOff_, position.Y + yOff_);
    Gdiplus::SolidBrush brush( colour );
    Gdiplus::RectF rec;
    Gdiplus::Graphics graph(bb_->getDC());
 
    graph.DrawString(ws.c_str(), -1, &font, pos, &format, &brush);
    graph.MeasureString(ws.c_str(), -1, &font, pos, &format, &rec);
    
    return Gdiplus::PointF( rec.GetRight() - xOff_, rec.Y- yOff_ );                                                                               
}

Gdiplus::PointF ScreenPrinter::PrintLetterFixedWidth(wchar_t letter, Gdiplus::PointF& position, Gdiplus::Font& font,
                           Gdiplus::Color& colour, Gdiplus::REAL width){
    std::wstring ws(1,  letter);
    Gdiplus::PointF pos(position.X + xOff_, position.Y + yOff_);
    Gdiplus::SolidBrush brush( colour );
    Gdiplus::RectF rec;
    Gdiplus::Graphics graph(bb_->getDC());

    Gdiplus::StringFormat sfNormal(Gdiplus::StringFormat::GenericTypographic());
    sfNormal.SetFormatFlags(Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
    Gdiplus::StringFormat sfFixedWidth;
    sfFixedWidth.SetFormatFlags(sfNormal.GetFormatFlags() - Gdiplus::StringFormatFlagsNoFitBlackBox);

    graph.DrawString(ws.c_str(), -1, &font, pos, &brush);
    //MISMATCH
    return Gdiplus::PointF( pos.X + width - xOff_, pos.Y - yOff_ );                           
                           
}

// Prints string, ensuring the text fits in a specified rectangular area.
// If it doesn't, it truncates the string (from the FRONT) until it fits, or until there is only one
// character left in the string.
void ScreenPrinter::PrintTruncString(const std::wstring string, const Gdiplus::PointF &position,
                                const Gdiplus::Font &font, const Gdiplus::Color& colour,
                                const Gdiplus::RectF& targetRec ) {
    
    Gdiplus::StringFormat sfNormal(Gdiplus::StringFormat::GenericTypographic());
    std::wstring localStr = string; // Copy string
    Gdiplus::PointF pos(position.X + xOff_, position.Y + yOff_); // Adjust position for offset
    Gdiplus::SolidBrush brush( colour );
    //Gdiplus::RectF targetRec(pos.X, pos.Y, 400.0f, 50.0f);
    Gdiplus::RectF measuredRec; // This rectangle is used in the measuring process
    Gdiplus::Graphics graph(bb_->getDC());
    do{
        graph.MeasureString(localStr.c_str(), -1, &font, pos, &sfNormal, &measuredRec); // Check string fits.
        if( measuredRec.Width > targetRec.Width ) // If it doesn't...
            localStr = localStr.substr(1, localStr.length() - 1); // ...chop off the leading character...
    } while ( measuredRec.Width > targetRec.Width && localStr.length() > 1 ); //...until it fits, or there is only one char left.
    
    // Now print the string.
    graph.DrawString(localStr.c_str(), -1,  &font, pos, &sfNormal, &brush);
    
}
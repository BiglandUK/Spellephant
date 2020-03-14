// BackBuffer.h
// By Frank Luna
// August 24, 2004
// Modified by Dan Bigmore 2009-10

#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include <windows.h>

class BackBuffer
{
public:
    BackBuffer(HWND hWnd, int width, int height, int screenX, int screenY);
    ~BackBuffer();

    HDC getDC();

    int width();
    int height();

    void present();
    void Clear();

private:
    // Prevent copy and assignment operators being automatically generated.
    BackBuffer(const BackBuffer& rhs);
    BackBuffer& operator=(const BackBuffer& rhs);

private:
    HWND    mhWnd;          // Handle to the main window
    HDC     mhDC;           // Handle to SYSTEM MEMORY DEVICE CONTEXT, upon which we will
                            // associate the backbuffer bitmap.
    HBITMAP mhSurface;      // Handle to the bitmap that serves our backbuffer; the render target.
    HBITMAP mhOldObject;    // Handle to the previous bitmap, loaded by the device context.  It
                            // is "good form" to restore the original Graphics Device Interface
                            // when done with the new one.
    int     mWidth;         // Width of bitmap matrix in pixels.
    int     mHeight;        // Height of bitmap matrix in pixels.
public:
    int     mOffX;
    int     mOffY;

};

#endif // BACKBUFFER_H

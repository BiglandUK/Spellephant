// BackBuffer.cpp
// by Frank Luna
// August 24, 2004

#include "BackBuffer.h"

BackBuffer::BackBuffer(HWND hWnd, int width, int height, int screenX, int screenY)
{
    // save a copy of the main window handle
    mhWnd = hWnd;

    // get a handle to the device context associated with the window
    HDC hWndDC = GetDC(hWnd); // don't confuse with getDC(), a member of BackBuffer.

    // save dimensions of bitmap
    mWidth = width;
    mHeight = height;

    // Calculate offset
    if( screenX > mWidth ) mOffX = (screenX - mWidth)/2; else mOffX = 0;
    if( screenY > mHeight ) mOffY = (screenY - mHeight)/2; else mOffY = 0;

    // create system memory device context, compatible with the window one.
    mhDC = CreateCompatibleDC(hWndDC);

    // create backbuffer surface bitmap compatible with the window device context
    mhSurface = CreateCompatibleBitmap(hWndDC, width+mOffX, height+mOffY);

    // We're done with the window DC
    ReleaseDC(hWnd, hWndDC);

    // At this point, the backbuffer surface is uninitialised, so clear it to some
    // non-zero value.  It needs to be non-zero, otherwise it will mess up our sprite
    // blending logic.

    // select the backbuffer bitmap into the DC.
    mhOldObject = (HBITMAP)SelectObject(mhDC, mhSurface);

    Clear();
}

BackBuffer::~BackBuffer()
{
    // Need to clean up the SYSTEM MEMORY DEVICE CONTEXT, and the bitmap.
    SelectObject(mhDC, mhOldObject);
    DeleteObject(mhSurface);
    DeleteDC(mhDC);
}

HDC BackBuffer::getDC()
{
    return mhDC;
}

int BackBuffer::width()
{
    return mWidth;
}

int BackBuffer::height()
{
    return mHeight;
}

void BackBuffer::present()
{
    // Get a handle to the device context associated with the window
    HDC hWndDC = GetDC(mhWnd);

    // Copy backbuffer contents to window client area
    BitBlt(hWndDC, mOffX,mOffY, mWidth+mOffX, mHeight+mOffY, mhDC, mOffX, mOffY, SRCCOPY);

    // Always free window DC when done.
    ReleaseDC(mhWnd, hWndDC);
}

void BackBuffer::Clear()
{
    //Rectangle(mhDC, mOffX-1, mOffY-1, mOffX + mWidth+1, mOffY + mHeight+1);
        // select a white brush
    HBRUSH black    = (HBRUSH)GetStockObject(BLACK_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(mhDC, black);

    // clear the backbuffer rectangle
    Rectangle(mhDC, mOffX, mOffY, 1024 + mOffX, 768 + mOffY);

    // restore the original brush.
    SelectObject(mhDC, oldBrush);
}
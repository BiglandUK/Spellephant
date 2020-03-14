#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <stdio.h>
#include <tchar.h>
#include <vld.h>
#include <string>
//#include <strsafe.h>
//#include "Utility.h"
#include "BackBuffer.h"
#include "CApp.h"

using namespace Gdiplus;
using namespace std;

//-----------------------------------------------------------------------------
// Global Variable Definitions
//-----------------------------------------------------------------------------
 
CApp g_App;


//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
std::wstring ToWString(const std::string& s)
{
    wstring ws( s.length(), L' '); // Make room for characters
    // Copy string to wstring.
    std::copy(s.begin(), s.end(), ws.begin());
    return ws;
}



//-----------------------------------------------------------------------------
// Name : WinMain() (Application Entry Point)
// Desc : Entry point for program, App flow starts here.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpszCmdLine, int nCmdShow)
{
    int retCode;
    
    // Variables used to initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialise the engine.
	if (!g_App.InitInstance( hInstance, lpszCmdLine, nCmdShow )) return 0;
    
    // Begin the gameplay process. Will return when app due to exit.
    retCode = g_App.BeginGame();

    // Return the correct exit code.
    return retCode;


}
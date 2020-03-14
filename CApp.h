// CApp.h

#ifndef CAPP_H
#define CAPP_H


#include <vector>
#include <list>
#include <map>
#include <gdiplus.h>
#include <irrKlang.h>
#include "Word.h"
#include "Definitions.h"

class BackBuffer;
class Mode;
class DBController;
class ScreenPrinter;
class Speller;

using namespace irrklang;

// Main driver for the program
class CApp
{
public:

    CApp();
    ~CApp();

    //Mode enum
    //enum{ TITLE=1, MENU, QUIT, NEWSPELLER, SELECTSPELLER, WORDLISTOPTIONS, LOADSPELLER, SPELLERMENU };
    
    // Startup Functions
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool        InitInstance( HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow );
    int         BeginGame( );

    void Update(double dt);             // Update based on current status
    void Display(BackBuffer& bb);    // Draw stuff based on current status

    void LMBDown();
    void KeyStroke( UINT key );
    void KeyRelease();
    void LMBUp();
    void Wheel(short zDelta, Gdiplus::PointF& mousePos);
    
    bool QuitGame();
    
    
    

private: // private functions
    bool CreateDisplay(HINSTANCE hInstance, int nCmdShow);
    void SetUp(); // Various intialisations and extracting data from database.
    
    Gdiplus::PointF GetMousePosition();
    
    void SwitchMode();
    void DeleteSpeller();

public: // public members
    HWND ghWnd;
    //irrklang::ISoundEngine* gDevice;
    const int gWidth;
    const int gHeight;
    BackBuffer* gBackBuffer;
    Gdiplus::Font* mpFont;
    DBController* pDBController_;
    ScreenPrinter* pScreenPrinter_;
    
    Mode* pMode_; // Contains the current "mode" of the program (titlescreen, menus, game modes, etc.)
    unsigned int currentMode_; // 
    unsigned int gotoMode_; // When valid, this triggers a change of mode.
    unsigned int previousMode_; // The mode that was previously in used.
    
    unsigned int numSpellers_;
    unsigned int spellerID_;
    Speller* pSpeller_;
    
    TagList tagList_;
    WordBank wordBank_;
    
    double timeElapsed_;
    

private:

};

#endif // CAPP_H

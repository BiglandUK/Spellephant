// CApp.cpp
#define _WIN32_WINDOWS 0x501 // Needed for WM_MOUSEWHEEL
#define _WIN32_WINNT 0x0501// Needed for GET_WHEEL_DELTA_WPARAM
#include <string>
#include <algorithm>
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <tchar.h>
#include <ctime>
#include <cstdlib>
#include "CApp.h"
#include "BackBuffer.h"
#include "TitleScreen.h"
#include "Menus.h"
#include "Options.h"
#include "DBController.h"
#include "ScreenPrinter.h"
#include "Word.h"
#include "Speller.h"

using namespace std;
using namespace Gdiplus;

extern std::wstring ToWString(const std::string& s);

CApp::CApp()
: gWidth(1024), gHeight(768), currentMode_(0), gotoMode_(1), previousMode_(0), pDBController_(new DBController()),
    spellerID_(0), pSpeller_(0)
{
    // GDI+ initialization
    // Variables used to initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    SetUp();
    
    // Set the game mode (in this case, Titlescreen)
    SwitchMode();
}

CApp::~CApp()
{
    delete gBackBuffer; // Delete BackBuffer
    delete pMode_;
    delete pDBController_;
    delete pScreenPrinter_;
    delete mpFont;
    DeleteSpeller();
}

//-----------------------------------------------------------------------------
// Name : StaticWndProc () (Static Callback)
// Desc : This is the main messge pump for ALL display devices, it captures
//        the appropriate messages, and routes them through to the application
//        class for which it was intended, therefore giving full class access.
// Note : It is VITALLY important that you should pass your 'this' pointer to
//        the lpParam parameter of the CreateWindow function if you wish to be
//        able to pass messages back to that app object.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CApp::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    // If this is a create message, trap the 'this' pointer passed in and store it within the window.
    if ( Message == WM_CREATE ) SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

    // Obtain the correct destination for this message
    CApp* Destination = (CApp*)GetWindowLong( hWnd, GWL_USERDATA );
    
    // If the hWnd has a related class, pass it through
    if (Destination) return Destination->WndProc( hWnd, Message, wParam, lParam );
    
    // No destination found, defer to system...
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

LRESULT CApp::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CREATE:
        {
            HINSTANCE hInstance = GetWindowInstance(hwnd);
            int screenX =   ::GetSystemMetrics(SM_CXSCREEN);    // Screen width
            int screenY =   ::GetSystemMetrics(SM_CYSCREEN);    // Screen height
            gBackBuffer = new BackBuffer(hwnd, gWidth, gHeight, screenX, screenY);
            pScreenPrinter_ = new ScreenPrinter(gBackBuffer);
            
            break;
        }
    case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
    case WM_CHAR:   // Deal with character input
        {
            KeyStroke( wParam );
            break;
        }
    case WM_KEYDOWN: // Deal with functional keys
        {
            switch( wParam )
            {
            case VK_ESCAPE:
                {
                    QuitGame();
                    break;
                }
            case VK_SHIFT:
                {
                    KeyStroke( wParam );
                    break;
                }
            default:
                {
                    break; 
                }
            }
            break;
        }
     case WM_KEYUP:
        {
            KeyRelease();
            break;
        }
     //case WM_LBUTTONDBLCLK:
     //   {
     //       break;
     //   }
     case WM_LBUTTONDOWN:
      {
            LMBDown();
            break;
      }
     case WM_LBUTTONUP:
      {
          LMBUp();
          break;
      }
    case WM_RBUTTONDOWN:
    {
      break;
    }
    case WM_RBUTTONUP:
    {
        break;
    }
    case WM_MOUSEWHEEL: {
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        Gdiplus::PointF mousePos(static_cast<float>( GET_X_LPARAM(lParam) - gBackBuffer->mOffX ),
                                 static_cast<float>( GET_Y_LPARAM(lParam) - gBackBuffer->mOffY ) );
        Wheel(zDelta, mousePos);
        break;
    }
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool CApp::QuitGame()
{
    int response = ::MessageBox(ghWnd, _T("Are you sure you want to quit?"), _T("Quit Application?"), MB_YESNO);
    if(response == 6)
    {
        DestroyWindow( ghWnd );
        return true;
    }
    return false;
}

bool CApp::InitInstance(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow )
{
    // Create the primary display device
    if (!CreateDisplay(hInstance, nCmdShow)) { return false; }


    // Success!
	return true;
}

bool CApp::CreateDisplay(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASS wc = {
                    0,
                    StaticWndProc,
                    0,0,
                    hInstance,
                    LoadIcon(NULL,IDI_APPLICATION),
                    LoadCursor(NULL,IDC_ARROW),
                    (HBRUSH)GetStockObject(BLACK_BRUSH),
                    NULL,
                    _T("CApp") };
  
    RegisterClass(&wc);

    // Create window
    ghWnd = CreateWindowEx(
                0L,
                _T("CApp"),                  // Window class name
                _T("Window Title"),                 // Window Title
                WS_EX_TOPMOST,         // Flags
                0,                                  // Top Left (X)
                0,                                  // Top Right (Y)
                ::GetSystemMetrics(SM_CXSCREEN),    // Screen width
                ::GetSystemMetrics(SM_CYSCREEN),    // Screen height
                HWND_DESKTOP,
                NULL,
                hInstance,                          // App instance
                this);                              // "this" pointer instead of NULL to work with Static Window Process.

    // Adjustments for fullscreen
    LONG style = ::GetWindowLong(ghWnd, GWL_STYLE);
    style &= ~WS_DLGFRAME;
    style &= ~WS_BORDER;
    style &= ~WS_EX_CLIENTEDGE;
    style &= ~WS_EX_WINDOWEDGE;
    ::SetWindowLong(ghWnd, GWL_STYLE, style);
    ::SetWindowPos(ghWnd, HWND_TOP, 0, 0, 0, 0,
    SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Ready to show and update window
    ShowWindow(ghWnd,nCmdShow);
    UpdateWindow(ghWnd);

    return true;
}

void CApp::SetUp(){
    srand( static_cast<unsigned int>( time( 0 ) ) );
    
    // Get Tags
    pDBController_->GetTagList(tagList_);
    
    // Get Word Bank
    pDBController_->GetWordBank( wordBank_ );
    pDBController_->GetBreakdowns( wordBank_ );
    
    // Get Tag and Word links (which words match with which tags)
    pDBController_->GetWordToTag( wordBank_, tagList_ );
    // Assign Untagged tag to words without tags
    for(WordBank::iterator iter = wordBank_.begin(); iter != wordBank_.end(); ++iter){
        if( iter->second.GetTags().empty() ){
            iter->second.AddTagID(0);
            tagList_[0].AddWordID(iter->second.GetID() );
        }
    }
    
    // Set Font
    mpFont = new Gdiplus::Font(L"Arial", 30.0);
}


int CApp::BeginGame()
{

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Get the performance timer frequency.
	__int64 cntsPerSec = 0;
	bool perfExists = QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec)!=0;
	if( !perfExists )
	{
		MessageBox(0, _T("Performance timer does not exist!"), 0, 0);
		return 0;
	}

	double timeScale = 1.0 / (double)cntsPerSec;
	// Get the current time.
	__int64 lastTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	
	//unsigned double timeElapsed = 0.0f;

	while(msg.message != WM_QUIT)
	{
		// IF there is a Windows message then process it.
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// ELSE, do game stuff.
		else
        {
			// Get the time now.	
			__int64 currTime = 0; 
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
			// Compute time difference
			double deltaTime = (double)(currTime - lastTime) * timeScale;
			timeElapsed_ += (double)deltaTime;
            
            if( pMode_ ){
            
                // Update stuff
                Update( deltaTime );
                
                gBackBuffer->Clear();
                // Draw screen to back buffer
                Display( *gBackBuffer );

                // Present new buffer
                gBackBuffer->present();          
            }
            
            // Check for new mode.
            if( gotoMode_ ){
                SwitchMode();                
                deltaTime = 0.0; // Is this necessary?!
            }      
                
			// update time variables for next loop
			lastTime = currTime;

            // Slight pause
            Sleep(20);
        }
    }
	// Return exit code back to operating system.
	return (int)msg.wParam;
}



void CApp::Update(double dt){
    pMode_->Update(dt, &GetMousePosition() );
}

void CApp::Display(BackBuffer& bb)
{
    HDC hdc = bb.getDC();
    int x = bb.mOffX;
    int y = bb.mOffY;
    pMode_->Display(&bb);

}

void CApp::LMBDown()
{
    pMode_->LMBDown( &GetMousePosition(), timeElapsed_ );
}

void CApp::LMBUp()
{
    pMode_->LMBUp( &GetMousePosition() );
}

void CApp::KeyStroke( UINT key )
{
    pMode_->KeyDown(key);
}

void CApp::KeyRelease(){
    pMode_->KeyUp();
}

void CApp::Wheel(short zDelta, Gdiplus::PointF& mousePos){

    pMode_->Wheel(zDelta, &mousePos);
}

PointF CApp::GetMousePosition() {
    POINT p;
    GetCursorPos(&p);
    PointF mousePos(static_cast<Gdiplus::REAL>(p.x), static_cast<Gdiplus::REAL>(p.y));
    // Adjust position with regards to portal offset
    mousePos.X -= gBackBuffer->mOffX;
    mousePos.Y -= gBackBuffer->mOffY;
    return mousePos;
}

void CApp::SwitchMode() {
    unsigned int modeNumber = gotoMode_;
    gotoMode_ = 0;
    
    switch(modeNumber){
        case TITLE:{
        delete pMode_;
            pMode_ = new TitleScreen(gotoMode_, previousMode_, TITLE,gBackBuffer);
            previousMode_ = TITLE;
            break;
        }
        case MENU:{
        delete pMode_;
            int result = pDBController_->GetNumSpellers();
            if( result < 0) {
                //TODO: deal with fault
                result = 0;
            }
            numSpellers_ = result;
            pMode_ = new TopMenu(gotoMode_, previousMode_, MENU,gBackBuffer, numSpellers_);
            previousMode_ = MENU;
            break;
        }
        case QUIT:{
            QuitGame();
            break;
        }
        case NEWSPELLER:{
            delete pMode_;
            pMode_ = new NewSpeller(gotoMode_, previousMode_, NEWSPELLER, mpFont, pDBController_, pScreenPrinter_, spellerID_, tagList_);
            previousMode_ = NEWSPELLER;
            break;
        }
        case SELECTSPELLER:{
            delete pMode_;
            pMode_ = new SelectSpeller(gotoMode_, previousMode_, SELECTSPELLER, mpFont, pDBController_, spellerID_);
            previousMode_ = SELECTSPELLER;
            break;
        }
        case WORDLISTOPTIONS:{
            delete pMode_;
            pMode_ = new WordListOptions(gotoMode_, previousMode_, WORDLISTOPTIONS, gBackBuffer,
                                         mpFont, wordBank_, tagList_,
                                         pSpeller_,
                                         pDBController_ );
            previousMode_ = WORDLISTOPTIONS;
            break;
        }
        case LOADSPELLER:{
            DeleteSpeller();
            pSpeller_ = pDBController_->LoadSpeller(spellerID_);
            pSpeller_->UpdateWordList(wordBank_);
            gotoMode_ = SPELLERMENU;
            break;
        }
        case SPELLERMENU:{
            delete pMode_;
            pMode_ = new SpellerMenu(gotoMode_, previousMode_, SPELLERMENU, mpFont, pDBController_);
            previousMode_ = SPELLERMENU;
            break; 
        }
        case QUICKSPELL:{
            delete pMode_;
            pMode_ = new MiniSpell(gotoMode_, previousMode_, QUICKSPELL, wordBank_, *pSpeller_, gBackBuffer, pScreenPrinter_,
                                    mpFont, pDBController_);
            previousMode_ = QUICKSPELL;
            break;
        }
        case WORDWORKOUT:{
            delete pMode_;
            pMode_ = new MiniSpell(gotoMode_, previousMode_, WORDWORKOUT, wordBank_, *pSpeller_, gBackBuffer, pScreenPrinter_,
                                    mpFont, pDBController_);
            previousMode_ = WORDWORKOUT;
            break;
        }
        case SPELLINGSPOTTING:{
            delete pMode_;
            pMode_ = new MiniSpell(gotoMode_, previousMode_, SPELLINGSPOTTING, wordBank_, *pSpeller_, gBackBuffer, pScreenPrinter_,
                                    mpFont, pDBController_);
            previousMode_ = SPELLINGSPOTTING;
            break;
        }
        default:{
            delete pMode_;
            int result = pDBController_->GetNumSpellers();
            if( result < 0) {
                //TODO: deal with fault
                result = 0;
            }
            numSpellers_ = result;
            pMode_ = new TopMenu(gotoMode_, previousMode_, MENU,gBackBuffer, numSpellers_);
            break;
        }
    
    }
}

void CApp::DeleteSpeller(){
    if( pSpeller_ ) {
        delete pSpeller_;
        pSpeller_ = 0;
    }
}
//----------------------------------------------------------------------------
//  File:   app.cpp
//
//  Desc:   DirectShow sample code
//          Main module for customized video player in exclusive mode
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#include "project.h"
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>
#include <commctrl.h>

#include <stdarg.h>
#include <stdio.h>

#include "utils.h"

/* -------------------------------------------------------------------------
** Global variables that are initialized at run time and then stay constant.
** -------------------------------------------------------------------------
*/
HINSTANCE           g_hInst;
HICON               hIconVideoCd;
HWND                hwndApp;
HWND                g_hwndToolbar;
CMpegMovie          *pMpegMovie;
BOOL                g_bPlay = FALSE;

/* -------------------------------------------------------------------------
** True Globals - these may change during execution of the program.
** -------------------------------------------------------------------------
*/
TCHAR               g_achFileName[MAX_PATH];
DWORD               g_State = VCD_NO_CD;
BOOL                g_bFullScreen = FALSE;


struct SceneSettings g_ss;

const LONG  g_Style =  WS_POPUP | WS_CAPTION | WS_SYSMENU;


/* -------------------------------------------------------------------------
** Constants
** -------------------------------------------------------------------------
*/
const TCHAR szClassName[] = TEXT("VMRXCLPlayer_CLASS");
const TCHAR g_szNULL[]    = TEXT("\0");
const TCHAR g_szEmpty[]   = TEXT("");

typedef enum VMRXcl_menu
{
    evxShowInfo  = 0x0,
    evxMoviePause,
    evxMoviePlay,
    evxMovieRotateZ,
    evxMovieRotateY,
    evxMovieTwist,
    evxMovieOption4,
    evxExit,

} VMRXcl_menu;


void SetFullScreenMode(BOOL bMode);
BOOL IsFullScreenMode();
extern BOOL VerifyVMR(void);


/******************************Public*Routine******************************\
* WinMain
*
*
* Windows recognizes this function by name as the initial entry point
* for the program.  This function calls the application initialization
* routine, if no other instance of the program is running, and always
* calls the instance initialization routine.  It then executes a message
* retrieval and dispatch loop that is the top-level control structure
* for the remainder of execution.  The loop is terminated when a WM_QUIT
* message is received, at which time this function exits the application
* instance by returning the value passed by PostQuitMessage().
*
* If this function must abort before entering the message loop, it
* returns the conventional value NULL.
*
\**************************************************************************/
int PASCAL
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLineOld,
    int nCmdShow
    )
{
    USES_CONVERSION;
    int iRet=0;
    LPTSTR lpCmdLine = A2T(lpCmdLineOld);

    HRESULT hres = CoInitialize(NULL);
    if (hres == S_FALSE) {
        CoUninitialize();
    }

    if ( !hPrevInstance ) {
        if ( !InitApplication( hInstance ) ) {
            return FALSE;
        }
    }

    /*
    ** Perform initializations that apply to a specific instance
    */
    if ( !InitInstance( hInstance, nCmdShow ) ) {
        return FALSE;
    }

    // Verify that the Video Mixing Renderer is present (requires Windows XP).
    // Otherwise, this sample cannot continue.
    if (!VerifyVMR())
       return FALSE;

    /* Look for options */
    while (lpCmdLine && (*lpCmdLine == '-' || *lpCmdLine == '/')) {
        if (lpCmdLine[1] == 'P') {
            g_bPlay = TRUE;
            lpCmdLine += 2;
        } else {
            break;
        }
        while (lpCmdLine[0] == ' ') {
            lpCmdLine++;
        }
    }

    SetFullScreenMode(g_bPlay);

    // If the user selects a file, open it and start playing.
    // Otherwise, just exit the app.
    if (ProcessOpen(lpCmdLine, TRUE) == TRUE)
    {
        /*
        ** Acquire and dispatch messages until a WM_QUIT message is received.
        */
        iRet = DoMainLoop();
    }

    QzUninitialize();
    return iRet;
}

/*****************************Private*Routine******************************\
* DoMainLoop
*
* Process the main message loop
*
\**************************************************************************/
int
DoMainLoop(
    void
    )
{
    MSG         msg;
    HANDLE      ahObjects[8];;
    int         cObjects;
    HACCEL      haccel = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

    //
    // message loop lasts until we get a WM_QUIT message
    // upon which we shall return from the function
    //
    for ( ;; ) {

        if (pMpegMovie != NULL) {
            cObjects = 1;
            ahObjects[0] = pMpegMovie->GetMovieEventHandle();
        }
        else {
            ahObjects[0] = NULL;
            cObjects = 0;
        }

        if (ahObjects[0] == NULL) {
            WaitMessage();
        }
        else {

            //
            // wait for any message sent or posted to this queue
            // or for a graph notification
            //
            DWORD result;

            result = MsgWaitForMultipleObjects(cObjects, ahObjects, FALSE,
                                               INFINITE, QS_ALLINPUT);
            
            if (result != (WAIT_OBJECT_0 + cObjects)) {
                
                VideoCd_OnGraphNotify(result - WAIT_OBJECT_0);
                continue;
            }
        }

        //
        // When here, we either have a message or no event handle
        // has been created yet.
        //
        // read all of the messages in this next loop
        // removing each message as we read it
        //

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            switch( msg.message )
            {
                case WM_QUIT:
                    return (int) msg.wParam;
                    break;
            }

            if (!TranslateAccelerator(hwndApp, haccel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

} // DoMainLoop


/*****************************Private*Routine******************************\
* InitApplication(HANDLE)
*
* This function is called at initialization time only if no other
* instances of the application are running.  This function performs
* initialization tasks that can be done once for any number of running
* instances.
*
* In this case, we initialize a window class by filling out a data
* structure of type WNDCLASS and calling the Windows RegisterClass()
* function.  Since all instances of this application use the same window
* class, we only need to do this when the first instance is initialized.
*
\**************************************************************************/
BOOL
InitApplication(
    HINSTANCE hInstance
    )
{
    WNDCLASS  wc;

    /*
    ** Fill in window class structure with parameters that describe the
    ** main window.
    */
    hIconVideoCd     = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_VIDEOCD_ICON) );

    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = VideoCdWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = hIconVideoCd;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)NULL; // (COLOR_BTNFACE + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MAIN_MENU);
    wc.lpszClassName = szClassName;

    /*
    ** Register the window class and return success/failure code.
    */
    return RegisterClass( &wc );
}


/*****************************Private*Routine******************************\
* InitInstance
*
*
* This function is called at initialization time for every instance of
* this application.  This function performs initialization tasks that
* cannot be shared by multiple instances.
*
* In this case, we save the instance handle in a static variable and
* create and display the main program window.
*
\**************************************************************************/
BOOL
InitInstance(
    HINSTANCE hInstance,
    int nCmdShow
    )
{
    HWND    hwnd;

    /*
    ** Save the instance handle in static variable, which will be used in
    ** many subsequence calls from this application to Windows.
    */
    g_hInst = hInstance;

    /*
    ** Create a hidden window for this application instance.
    */
    hwnd = CreateWindow( szClassName, IdStr(STR_APP_TITLE), g_Style,
                         CW_USEDEFAULT, CW_USEDEFAULT, 
                         CW_USEDEFAULT, CW_USEDEFAULT, 
                         NULL, NULL, hInstance, NULL );

    /*
    ** If window could not be created, return "failure"
    */
    if ( NULL == hwnd ) {
        return FALSE;
    }
    hwndApp = hwnd;


    /*
    ** Keep the main window invisible, since all we need to do is
     * display a FileOpen dialog box.
    */

    return TRUE;
}


/*****************************Private*Routine******************************\
* GetMoviePosition
*
* Place the movie in the centre of the client window.  We do not stretch the
* the movie yet !
*
\**************************************************************************/
void
GetMoviePosition(
    HWND hwnd,
    long* xPos,
    long* yPos,
    long* pcx,
    long* pcy
    )
{
    RECT rc = {0, 0, 400, 300};

    GetClientRect(hwnd, &rc);

    *xPos = rc.left;
    *yPos = rc.top;

    *pcx = rc.right - rc.left;
    *pcy = rc.bottom - rc.top;
}


/******************************Public*Routine******************************\
* RepositionMovie
*
\**************************************************************************/
void
RepositionMovie(HWND hwnd)
{
    if (pMpegMovie) {

        IVMRWindowlessControl* lpDefWC = pMpegMovie->GetWLC();
        if (lpDefWC) {
            LONG w, h;

            lpDefWC->GetNativeVideoSize(&w, &h, NULL, NULL);

            RECT rc = {(640-w)/2, (480-h)/2, (640+w)/2, (480+h)/2};

            lpDefWC->SetVideoPosition(NULL, &rc);
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnMove
*
\**************************************************************************/
void
VideoCd_OnMove(
    HWND hwnd,
    int x,
    int y
    )
{
    if (pMpegMovie) {
        if (pMpegMovie->GetStateMovie() != State_Running) {
            RepositionMovie(hwnd);
        }
    }
}


/******************************Public*Routine******************************\
* VideoCdWndProc
*
\**************************************************************************/
LRESULT CALLBACK
VideoCdWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch ( message ) {

    HANDLE_MSG( hwnd, WM_CREATE,            VideoCd_OnCreate );
    HANDLE_MSG( hwnd, WM_PAINT,             VideoCd_OnPaint );
    HANDLE_MSG( hwnd, WM_CLOSE,             VideoCd_OnClose );
    HANDLE_MSG( hwnd, WM_DESTROY,           VideoCd_OnDestroy );
    HANDLE_MSG( hwnd, WM_SIZE,              VideoCd_OnSize );
    HANDLE_MSG( hwnd, WM_KEYUP,             VideoCd_OnKeyUp);
    HANDLE_MSG( hwnd, WM_MOVE,              VideoCd_OnMove );

    case WM_LBUTTONUP:
        {
            VideoCd_OnClick(hwnd, wParam, lParam);
        }
        break;
    case WM_RBUTTONUP:
        {
            VideoCd_OnClickHold(hwnd, wParam, lParam);
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0L;
}


/*****************************Private*Routine******************************\
* VideoCd_OnCreate
*
\**************************************************************************/
BOOL
VideoCd_OnCreate(
    HWND hwnd,
    LPCREATESTRUCT lpCreateStruct
    )
{
    return TRUE;
}

/*****************************Private*Routine******************************\
* VideoCd_OnKeyUp
*
\**************************************************************************/
void
VideoCd_OnKeyUp(
    HWND hwnd,
    UINT vk,
    BOOL fDown,
    int cRepeat,
    UINT flags
    )
{
    // Catch escape sequences to stop fullscreen mode
    if (vk == VK_ESCAPE) {

        if (pMpegMovie) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnClick
*
\**************************************************************************/
void
VideoCd_OnClick(
    HWND hwnd,
    WPARAM pParam,
    LPARAM lParam)
{
    int xPos;
    int yPos;
    int nMenuItem = -1;

    xPos = GET_X_LPARAM(lParam); 
    yPos = GET_Y_LPARAM(lParam); 

    g_ss.bShowHelp = false;
    g_ss.nXHelp = 0;
    g_ss.nYHelp = 0;
    lstrcpy( g_ss.achHelp, TEXT(""));

    if( xPos >= 3  && xPos <= 55 )
    {
        nMenuItem = (yPos -6)/53;
        if( nMenuItem == 8 )
        {
            nMenuItem = -1;
        }
    }

    switch( nMenuItem)
    {
        case evxMovieRotateZ:
            g_ss.bShowTwist = false;
            g_ss.nDy = 0;
            g_ss.bRotateZ = !(g_ss.bRotateZ);
            break;
        case evxMovieRotateY:
            g_ss.bShowTwist = false;
            g_ss.nDy = 0;
            g_ss.bRotateY = !(g_ss.bRotateY);
            break;
        case evxMovieTwist:
            g_ss.bShowTwist = !(g_ss.bShowTwist);
            break;
        case evxExit:
            if (pMpegMovie) 
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        case evxMoviePause:
            if( pMpegMovie )
            {
                pMpegMovie->PauseMovie();
            }
            break;
        case evxMoviePlay:
            if( pMpegMovie )
            {
                pMpegMovie->PlayMovie();
            }
            break;
        case evxShowInfo:
            g_ss.bShowStatistics = !(g_ss.bShowStatistics);
            break;
    }// switch
}

/*****************************Private*Routine******************************\
* VideoCd_OnClickHold
*
\**************************************************************************/
void
VideoCd_OnClickHold(
    HWND hwnd,
    WPARAM pParam,
    LPARAM lParam)
{
    int xPos;
    int yPos;
    int nMenuItem = -1;

    OutputDebugString("*** Event is captured\n");

    xPos = GET_X_LPARAM(lParam); 
    yPos = GET_Y_LPARAM(lParam); 

    g_ss.bShowHelp = false;
    g_ss.nXHelp = 0;
    g_ss.nYHelp = 0;
    lstrcpy( g_ss.achHelp, TEXT(""));

    if( xPos >= 3  && xPos <= 55 )
    {
        nMenuItem = (yPos -6)/53;
        g_ss.bShowHelp = true;
        if( nMenuItem == 8 )
        {
            nMenuItem = -1;
            g_ss.bShowHelp = false;
        }
        
    }
    if( g_ss.bShowHelp )
    {
        OutputDebugString("*** WILL show help\n");
    }
    else
    {
        OutputDebugString("*** WILL NOT show help\n");
    }

    switch( nMenuItem)
    {
        case evxMovieRotateZ:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Rotate around Z axis "));
            break;
        case evxMovieRotateY:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Rotate around Y axis "));
            break;
        case evxMovieTwist:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Non-linear transformation "));
            break;
        case evxExit:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Exit application "));
            break;
        case evxMoviePause:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Pause "));
            break;
        case evxMoviePlay:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Play "));
            break;
        case evxShowInfo:
            g_ss.nXHelp = 70;
            g_ss.nYHelp = yPos;
            lstrcpy( g_ss.achHelp, TEXT("Show statistics "));
            break;
    }// switch
}


/*****************************Private*Routine******************************\
* VideoCd_OnPaint
*
\**************************************************************************/
void
VideoCd_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT ps;
    HDC         hdc;
    RECT        rc2;

    /*
    ** Draw a frame around the movie playback area.
    */
    hdc = BeginPaint( hwnd, &ps );
    GetClientRect(hwnd, &rc2);
    FillRect(hdc, &rc2, (HBRUSH)(COLOR_BTNFACE + 1));
    EndPaint( hwnd, &ps );
}


/******************************Public*Routine******************************\
* VideoCd_OnDestroy
*
\**************************************************************************/
void
VideoCd_OnDestroy(
    HWND hwnd
    )
{
    PostQuitMessage( 0 );
}


/******************************Public*Routine******************************\
* VideoCd_OnClose
*
\**************************************************************************/
void
VideoCd_OnClose(
    HWND hwnd
    )
{
    if (pMpegMovie) {
        pMpegMovie->StopMovie();
        pMpegMovie->CloseMovie();

        delete pMpegMovie;
        pMpegMovie = NULL;
    }

    DestroyWindow( hwnd );
}


/******************************Public*Routine******************************\
* VideoCd_OnSize
*
\**************************************************************************/
void
VideoCd_OnSize(
    HWND hwnd,
    UINT state,
    int dx,
    int dy
    )
{
    RepositionMovie(hwnd);
}


/*****************************Private*Routine******************************\
* VideoCd_OnGraphNotify
*
* This is where we get any notifications from the filter graph.
*
\**************************************************************************/
void
VideoCd_OnGraphNotify(
    int stream
    )
{
    long    lEventCode;

    lEventCode = pMpegMovie->GetMovieEventCode();

    switch (lEventCode) {
        case EC_FULLSCREEN_LOST:
            break;

        case EC_COMPLETE:
            // when movie is complete, reposition it to the start position
            pMpegMovie->SeekToPosition(0L,FALSE);
            break;
        case EC_USERABORT:
        case EC_ERRORABORT:
            break;
    }
}


/******************************Public*Routine******************************\
* IdStr
*
* Loads the given string resource ID into the passed storage.
*
\**************************************************************************/
LPCTSTR
IdStr(
    int idResource
    )
{
    static TCHAR    chBuffer[ STR_MAX_STRING_LEN ];

    if (LoadString(g_hInst, idResource, chBuffer, STR_MAX_STRING_LEN) == 0) {
        return g_szEmpty;
    }

    return chBuffer;

}


/******************************Public*Routine******************************\
* SetFullScreenMode
*
\**************************************************************************/
void
SetFullScreenMode(BOOL bMode)
{
    g_bFullScreen = bMode;

    static HMENU hMenu;
    static LONG  lStyle;
    static int xs, ys, cxs, cys;

    HDC hdcScreen = GetDC(NULL);
    int cx = GetDeviceCaps(hdcScreen,HORZRES);
    int cy = GetDeviceCaps(hdcScreen,VERTRES);
    ReleaseDC(NULL, hdcScreen);

    if (bMode) {

        hMenu = GetMenu(hwndApp);
        lStyle = GetWindowStyle(hwndApp);

        WINDOWPLACEMENT wp;
        GetWindowPlacement(hwndApp, &wp);
        xs = wp.rcNormalPosition.left;
        ys = wp.rcNormalPosition.top;
        cxs = wp.rcNormalPosition.right - xs;
        cys = wp.rcNormalPosition.bottom - ys;
        ShowWindow(g_hwndToolbar, SW_HIDE);
        SetMenu(hwndApp, NULL);
        SetWindowLong(hwndApp, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE);
        SetWindowPos(hwndApp, HWND_TOP, 0, 0, cx, cy, SWP_NOACTIVATE);
        ShowCursor(FALSE);

    }
    else {
        ShowCursor(TRUE);
        ShowWindow(g_hwndToolbar, SW_SHOW);
        SetMenu(hwndApp, hMenu);
        SetWindowLong(hwndApp, GWL_STYLE, lStyle);
        SetWindowPos(hwndApp, HWND_TOP, xs, ys, cxs, cys, SWP_NOACTIVATE);
    }
}


/******************************Public*Routine******************************\
* IsFullScreenMode()
*
\**************************************************************************/
BOOL
IsFullScreenMode()
{
    return g_bFullScreen;
}

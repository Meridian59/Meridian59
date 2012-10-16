//------------------------------------------------------------------------------
// File: app.cpp
//
// Desc: DirectShow sample code - VMR-based Cube video player
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>
#include <commctrl.h>

#include <initguid.h>
#include "project.h"
#include "resource.h"

#include <stdarg.h>
#include <stdio.h>


/* -------------------------------------------------------------------------
** Global variables that are initialized at run time and then stay constant.
** -------------------------------------------------------------------------
*/
HINSTANCE           hInst;
HICON               hIconVideoCd;
HWND                hwndApp;
HWND                g_hwndToolbar;
CMpegMovie          *pMpegMovie;

BOOL m_bFullScreen = FALSE;

/* -------------------------------------------------------------------------
** True Globals - these may change during execution of the program.
** -------------------------------------------------------------------------
*/
DWORD g_State = VCD_NO_CD;

/* -------------------------------------------------------------------------
** Constants
** -------------------------------------------------------------------------
*/
const TCHAR szClassName[] = TEXT("VMR_CubePlayer_CLASS");
const TCHAR g_szNULL[]    = TEXT("\0");
const TCHAR g_szEmpty[]   = TEXT("");

/*
** User interface values
*/
      int   dyToolbar;
const int   dxBitmap        = 16;
const int   dyBitmap        = 15;
const int   dxButtonSep     = 8;
const TCHAR g_chNULL        = TEXT('\0');
const LONG  g_Style         = WS_THICKFRAME | WS_POPUP | WS_CAPTION  |
                              WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                              WS_CLIPCHILDREN;


const TBBUTTON tbButtons[DEFAULT_TBAR_SIZE] = {
    { IDX_SEPARATOR,    1,                    0,               TBSTYLE_SEP           },
    { IDX_1,            IDM_MOVIE_PLAY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_2,            IDM_MOVIE_PAUSE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_3,            IDM_MOVIE_STOP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    1,                    0,               TBSTYLE_SEP           },
    { IDX_4,            IDM_FULL_SCREEN,      TBSTATE_ENABLED, TBSTYLE_CHECK,  0, 0, 0, -1 }
};


/* -------------------------------------------------------------------------
** Local function prototypes
** -------------------------------------------------------------------------
*/
void SetFullScreenMode(BOOL bMode);
BOOL IsFullScreenMode();
LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


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

    HRESULT hres = CoInitialize(NULL);
    if(hres == S_FALSE)
    {
        CoUninitialize();
    }

    if(!hPrevInstance)
    {
        if(!InitApplication(hInstance))
        {
            return FALSE;
        }
    }

    /*
    ** Perform initializations that apply to a specific instance
    */
    if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    /* Verify that the VMR is present on this system */
    if(!VerifyVMR())
        return FALSE;

    /*
    ** Acquire and dispatch messages until a WM_QUIT message is received.
    */
    int iRet = DoMainLoop();
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
    HACCEL      haccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

    //
    // message loop lasts until we get a WM_QUIT message
    //
    for(;;)
    {
        if(pMpegMovie != NULL)
        {
            cObjects = 1;
            ahObjects[0] = pMpegMovie->GetMovieEventHandle();
        }
        else
        {
            ahObjects[0] = NULL;
            cObjects = 0;
        }

        if(ahObjects[0] == NULL)
        {
            WaitMessage();
        }
        else
        {
            //
            // wait for any message sent or posted to this queue
            // or for a graph notification
            //
            DWORD result;

            result = MsgWaitForMultipleObjects(cObjects, ahObjects, FALSE,
                INFINITE, QS_ALLINPUT);
            if(result != (WAIT_OBJECT_0 + cObjects))
            {
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
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                return (int) msg.wParam;
            }

            if(!TranslateAccelerator(hwndApp, haccel, &msg))
            {
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
    hIconVideoCd     = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_VIDEOCD_ICON));

    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = VideoCdWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = hIconVideoCd;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)NULL;
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAIN_MENU);
    wc.lpszClassName = szClassName;

    /*
    ** Register the window class and return success/failure code.
    */
    return RegisterClass(&wc);
}


/*****************************Private*Routine******************************\
* InitInstance
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
    RECT    rc;

    /*
    ** Save the instance handle in static variable, which will be used
    ** in many subsequent calls to Windows.
    */
    hInst = hInstance;
    rc.left = rc.top = 100;
    rc.bottom = rc.right = 400;

    /*
    ** Create a main window for this application instance.
    */
    hwnd = CreateWindow(szClassName, IdStr(STR_APP_TITLE), g_Style,
        rc.left, rc.top,
        rc.right, rc.bottom,
        NULL, NULL, hInstance, NULL);

    /*
    ** If window could not be created, return "failure"
    */
    if(NULL == hwnd)
    {
        return FALSE;
    }
    hwndApp = hwnd;

    /*
    ** Make the window visible; update its client area; and return "success"
    */
    SetPlayButtonsEnableState();
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return TRUE;
}


/*****************************Private*Routine******************************\
* GetMoviePosition
*
* Place the movie in the centre of the client window.  
* We do not stretch the the movie yet.
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
    RECT rc;

    GetAdjustedClientRect(&rc);

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
    if(pMpegMovie)
    {
        long xPos, yPos, cx, cy;

        GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);

        pMpegMovie->PutMoviePosition(xPos, yPos, cx, cy);
        InvalidateRect(hwnd, NULL, false);
        UpdateWindow(hwnd);
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
    if(pMpegMovie)
    {
        if(pMpegMovie->GetStateMovie() != State_Running)
        {
            RepositionMovie(hwnd);
        }
        else
        {
            long xPos, yPos, cx, cy;

            // Reposition movie but don't invalidate the rect, since
            // the next video frame will handle the redraw.
            GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
            pMpegMovie->PutMoviePosition(xPos, yPos, cx, cy);
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
    switch(message)
    {
        HANDLE_MSG(hwnd, WM_CREATE,            VideoCd_OnCreate);
        HANDLE_MSG(hwnd, WM_PAINT,             VideoCd_OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND,           VideoCd_OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE,             VideoCd_OnClose);
        HANDLE_MSG(hwnd, WM_DESTROY,           VideoCd_OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,              VideoCd_OnSize);
        HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE,    VideoCd_OnSysColorChange);
        HANDLE_MSG(hwnd, WM_INITMENUPOPUP,     VideoCd_OnInitMenuPopup);
        HANDLE_MSG(hwnd, WM_NOTIFY,            VideoCd_OnNotify);
        HANDLE_MSG(hwnd, WM_KEYUP,             VideoCd_OnKeyUp);
        HANDLE_MSG(hwnd, WM_MOVE,              VideoCd_OnMove);

        case WM_DISPLAYCHANGE:
        {
            if(pMpegMovie)
            {
                pMpegMovie->DisplayModeChanged();
            }
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
    InitCommonControls();

    /*
    ** Create the toolbar and statusbar.
    */
    g_hwndToolbar = CreateToolbarEx(hwnd,
                                    WS_VISIBLE | WS_CHILD |
                                    TBSTYLE_TOOLTIPS | CCS_NODIVIDER,
                                    ID_TOOLBAR, NUMBER_OF_BITMAPS,
                                    hInst, IDR_TOOLBAR, tbButtons,
                                    DEFAULT_TBAR_SIZE, dxBitmap, dyBitmap,
                                    dxBitmap, dyBitmap, sizeof(TBBUTTON));

    if(g_hwndToolbar == NULL)
    {
        return FALSE;
    }

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
    if((vk == VK_ESCAPE) || (vk == VK_RETURN))
    {
        if(pMpegMovie && IsFullScreenMode())
        {
            SetFullScreenMode(FALSE);
            SetPlayButtonsEnableState();
        }
    }
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
    RECT        rc1;
    RECT        rc2;

    /*
    ** Draw a frame around the movie playback area.
    */
    GetClientRect(hwnd, &rc2);

    hdc = BeginPaint(hwnd, &ps);

    if(pMpegMovie)
    {
        long xPos, yPos, cx, cy;
        GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
        SetRect(&rc1, xPos, yPos, xPos + cx, yPos + cy);

        HRGN rgnClient = CreateRectRgnIndirect(&rc2);
        HRGN rgnVideo  = CreateRectRgnIndirect(&rc1);
        CombineRgn(rgnClient, rgnClient, rgnVideo, RGN_DIFF);

        HBRUSH hbr = GetSysColorBrush(COLOR_BTNFACE);
        FillRgn(hdc, rgnClient, hbr);
        DeleteObject(hbr);
        DeleteObject(rgnClient);
        DeleteObject(rgnVideo);

        pMpegMovie->RepaintVideo(hwnd, hdc);
    }
    else
    {
        FillRect(hdc, &rc2, (HBRUSH)(COLOR_BTNFACE + 1));
    }

    EndPaint(hwnd, &ps);
}


/*****************************Private*Routine******************************\
* AboutDlgProc
*
\**************************************************************************/
LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if(wParam == IDOK)
            {
                EndDialog(hWnd, TRUE);
                return TRUE;
            }
            break;
    }
    return FALSE;
}


/*****************************Private*Routine******************************\
* VideoCd_OnCommand
*
\**************************************************************************/
void
VideoCd_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch(id)
    {
        case IDM_FILE_OPEN:
            if(VcdPlayerOpenCmd())
                VcdPlayerPlayCmd();
            break;

        case IDM_FILE_CLOSE:
            VcdPlayerCloseCmd();
            QzFreeUnusedLibraries();
            break;

        case IDM_FILE_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX),
                hwnd,  (DLGPROC) AboutDlgProc);
            break;

        case IDM_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_MOVIE_PLAY:
            VcdPlayerPlayCmd();
            break;

        case IDM_MOVIE_STOP:
            VcdPlayerStopCmd();
            VcdPlayerRewindCmd();
            break;

        case IDM_MOVIE_PAUSE:
            VcdPlayerPauseCmd();
            break;

        case IDM_FULL_SCREEN:
            if(pMpegMovie)
            {
                BOOL bFullScreen = (BOOL)SendMessage(g_hwndToolbar,
                    TB_ISBUTTONCHECKED, IDM_FULL_SCREEN, 0);
                SetFullScreenMode(bFullScreen);
            }
            break;
    }

    SetPlayButtonsEnableState();
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
    PostQuitMessage(0);
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
    VcdPlayerCloseCmd();
    DestroyWindow(hwnd);
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
    if(IsWindow(g_hwndToolbar))
        SendMessage(g_hwndToolbar, WM_SIZE, 0, 0L);

    RepositionMovie(hwnd);
}


/*****************************Private*Routine******************************\
* VideoCd_OnSysColorChange
*
\**************************************************************************/
void
VideoCd_OnSysColorChange(
    HWND hwnd
    )
{
    FORWARD_WM_SYSCOLORCHANGE(g_hwndToolbar, SendMessage);
}


/*****************************Private*Routine******************************\
* VideoCd_OnInitMenuPopup
*
\**************************************************************************/
void
VideoCd_OnInitMenuPopup(
    HWND hwnd,
    HMENU hMenu,
    UINT item,
    BOOL fSystemMenu
    )
{
    UINT uFlags;

    if(item == 0)
    { // File menu

        if(g_State & (VCD_IN_USE | VCD_NO_CD | VCD_DATA_CD_LOADED))
        {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        else
        {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }

        // Disable menu items until a movie is opened
        EnableMenuItem(hMenu, IDM_FILE_CLOSE, uFlags);
        EnableMenuItem(hMenu, IDM_MOVIE_STOP, uFlags);
        EnableMenuItem(hMenu, IDM_MOVIE_PLAY, uFlags);
        EnableMenuItem(hMenu, IDM_MOVIE_PAUSE, uFlags);
    }
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

    switch(lEventCode)
    {
        case EC_FULLSCREEN_LOST:
            SetPlayButtonsEnableState();
            break;

        case EC_USERABORT:
        case EC_ERRORABORT:
            VcdPlayerStopCmd();
            SetPlayButtonsEnableState();
            break;

        case EC_COMPLETE:
            VcdPlayerRewindCmd();
            break;

        default:
            break;
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnNotify
*
* This is where we get the text for tooltips
*
\**************************************************************************/
LRESULT
VideoCd_OnNotify(
    HWND hwnd,
    int idFrom,
    NMHDR FAR* pnmhdr
    )
{
    switch(pnmhdr->code)
    {
        case TTN_NEEDTEXT:
            {
                LPTOOLTIPTEXT   lpTt;
                lpTt = (LPTOOLTIPTEXT)pnmhdr;

                LoadString(hInst, (UINT) lpTt->hdr.idFrom, lpTt->szText,
                    sizeof(lpTt->szText));
            }
            break;
    }

    return 0;
}


/******************************Public*Routine******************************\
* SetPlayButtonsEnableState
*
* Sets the play buttons enable state to match the state of the current
* cdrom device.  See below...
*
*
*                 VCD Player buttons enable state table
* --------------------------------------------------------------------
* -E=Enabled D=Disabled      - Play - Pause - Eject - Stop  - Other -
* --------------------------------------------------------------------
* -Disk in use               -  D   -  D    -  D    -   D   -   D   -
* --------------------------------------------------------------------
* -No video cd or data cdrom -  D   -  D    -  E    -   D   -   D   -
* --------------------------------------------------------------------
* -Video cd (playing)        -  D   -  E    -  E    -   E   -   E   -
* --------------------------------------------------------------------
* -Video cd (paused)         -  E   -  D    -  E    -   E   -   E   -
* --------------------------------------------------------------------
* -Video cd (stopped)        -  E   -  D    -  E    -   D   -   E   -
* --------------------------------------------------------------------
*
\**************************************************************************/
void
SetPlayButtonsEnableState(
    void
    )
{
    BOOL    fEnable, fPress;
    BOOL    fVideoCdLoaded;

    /*
    ** Do we have a video cd loaded.
    */
    if(g_State & (VCD_NO_CD | VCD_DATA_CD_LOADED | VCD_IN_USE))
    {
        fVideoCdLoaded = FALSE;
    }
    else
    {
        fVideoCdLoaded = TRUE;
    }

    /*
    ** Do the play button
    */
    if(fVideoCdLoaded
        && ((g_State & VCD_STOPPED) || (g_State & VCD_PAUSED)))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PLAY, fEnable);

    /*
    ** Do the stop button
    */
    if(fVideoCdLoaded
        && ((g_State & VCD_PLAYING) || (g_State & VCD_PAUSED)))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STOP, fEnable);

    /*
    ** Do the pause button
    */
    if(fVideoCdLoaded && (g_State & VCD_PLAYING))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PAUSE, fEnable);

    /*
    ** Do the remaining buttons
    */
    /*
    ** Do the fullscreen button
    */
    fPress = (fVideoCdLoaded && IsFullScreenMode());

    SendMessage(g_hwndToolbar, TB_CHECKBUTTON, IDM_FULL_SCREEN, MAKELONG(fPress,0));
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_FULL_SCREEN, fVideoCdLoaded && (g_State & VCD_PLAYING));
}


/*****************************Private*Routine******************************\
* GetAdjustedClientRect
*
* Calculate the size of the client rect and then adjusts it to take into
* account the space taken by the toolbar and status bar.
*
\**************************************************************************/
void
GetAdjustedClientRect(
    RECT *prc
    )
{
    RECT    rcTool;

    GetClientRect(hwndApp, prc);

    if(IsWindowVisible(g_hwndToolbar))
    {
        GetWindowRect(g_hwndToolbar, &rcTool);
        prc->top += (rcTool.bottom - rcTool.top);
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

    if(LoadString(hInst, idResource, chBuffer, STR_MAX_STRING_LEN) == 0)
    {
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
    m_bFullScreen = bMode;

    // Defer until we activate the movie
    if(pMpegMovie->GetStateMovie() != State_Running)
    {
        if(bMode == TRUE)
            return;
    }

    static HMENU hMenu;
    static LONG  lStyle;
    static int xs, ys, cxs, cys;

    HDC hdcScreen = GetDC(NULL);
    int cx = GetDeviceCaps(hdcScreen,HORZRES);
    int cy = GetDeviceCaps(hdcScreen,VERTRES);
    ReleaseDC(NULL, hdcScreen);

    if(bMode)
    {
        hMenu = GetMenu(hwndApp);
        lStyle = GetWindowStyle(hwndApp);

        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwndApp, &wp);
        xs = wp.rcNormalPosition.left;
        ys = wp.rcNormalPosition.top;
        cxs = wp.rcNormalPosition.right - xs;
        cys = wp.rcNormalPosition.bottom - ys;
        ShowWindow(g_hwndToolbar, SW_HIDE);
        SetMenu(hwndApp, NULL);
        SetWindowLong(hwndApp, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwndApp, HWND_TOP, 0, 0, cx, cy, SWP_NOACTIVATE);
        ShowCursor(FALSE);
    }
    else
    {
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
    return m_bFullScreen;
}


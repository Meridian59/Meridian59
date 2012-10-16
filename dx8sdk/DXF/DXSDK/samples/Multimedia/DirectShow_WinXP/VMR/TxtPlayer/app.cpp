//------------------------------------------------------------------------------
// File: app.cpp
//
// Desc: DirectShow sample code - application code for TxtPlayer sample
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>
#include <commctrl.h>

#include <stdarg.h>
#include <stdio.h>

#include "project.h"
#include "mpgcodec.h"


/* -------------------------------------------------------------------------
** Global variables that are initialized at run time and then stay constant.
** -------------------------------------------------------------------------
*/
HINSTANCE           hInst;
HICON               hIconVideoCd;
HWND                hwndApp;
HWND                g_hwndToolbar;
HWND                g_hwndStatusbar;
HWND                g_hwndTrackbar;
CMpegMovie          *pMpegMovie;
double              g_TrackBarScale = 1.0;
BOOL                g_bPlay = FALSE;
int                 dyToolbar, dyStatusbar, dyTrackbar;
int                 FrameStepCount;


/* -------------------------------------------------------------------------
** True Globals - these may change during execution of the program.
** -------------------------------------------------------------------------
*/
TCHAR               g_achFileName[MAX_PATH];
OPENFILENAME        ofn;
DWORD               g_State = VCD_NO_CD;
LONG                lMovieOrgX, lMovieOrgY;
int                 g_TimeFormat = IDM_TIME;

extern RECENTFILES  aRecentFiles[MAX_RECENT_FILES];
extern int          nRecentFiles;

char                g_szLine21Text[4 * 41];

const char  g_Line21Msg[] =
/*        10        20        30
 0123456789012345678901234567890123456789
*/
"Here is some random text used to        "
"demonstrate how Line21 can be rendered  "
"using the new Video Mixing Renderer.    "
"The font used to render the text is     "
"resized as the video destination        "
"rectangle changes.  This means that the "
"text is always shown with the highest   "
"possible image quality.  Also, you will "
"notice that the text is Alpha Blended   "
"with the underlying video, which is     "
"only possible using the new VMR.        ";


LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


/* -------------------------------------------------------------------------
** Constants
** -------------------------------------------------------------------------
*/
const TCHAR szClassName[] = TEXT("VMRTxtPlayer_CLASS");
const TCHAR g_szNULL[]    = TEXT("\0");
const TCHAR g_szEmpty[]   = TEXT("");
const TCHAR g_szMovieX[]  = TEXT("MovieOriginX");
const TCHAR g_szMovieY[]  = TEXT("MovieOriginY");

const int STR_MAX_STRING_LEN = 256;

const int   dxBitmap        = 16;
const int   dyBitmap        = 15;
const int   dxButtonSep     = 8;
const TCHAR g_chNULL        = TEXT('\0');

const TBBUTTON tbButtons[DEFAULT_TBAR_SIZE] = {
    { IDX_SEPARATOR,    1,                    0,               TBSTYLE_SEP           },
    { IDX_1,            IDM_MOVIE_PLAY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_2,            IDM_MOVIE_PAUSE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_3,            IDM_MOVIE_STOP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    2,                    0,               TBSTYLE_SEP           },
    { IDX_4,            IDM_MOVIE_PREVTRACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_5,            IDM_MOVIE_SKIP_BACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_6,            IDM_MOVIE_SKIP_FORE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    3,                    0,               TBSTYLE_SEP           },
    { IDX_12,           IDM_MOVIE_STEP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 }
};


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
    LPTSTR lpCmdLine = A2T(lpCmdLineOld);

    HRESULT hres = CoInitialize(NULL);
    if(hres == S_FALSE)
    {
        CoUninitialize();
        return FALSE;
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

    /* Look for options */
    while(lpCmdLine && (*lpCmdLine == '-' || *lpCmdLine == '/'))
    {
        if ((lpCmdLine[1] == 'P') || (lpCmdLine[1] == 'p'))
        {
            g_bPlay = TRUE;
            lpCmdLine += 2;
        }
        else
        {
            break;
        }
        while(lpCmdLine[0] == ' ')
        {
            lpCmdLine++;
        }
    }


    if(lpCmdLine != NULL && lstrlen(lpCmdLine) > 0)
    {
        ProcessOpen(lpCmdLine, g_bPlay);
        SetPlayButtonsEnableState();
    }

    /*
    ** Acquire and dispatch messages until a WM_QUIT message is received.
    */
    return DoMainLoop();
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
    HANDLE      ahObjects[1];   // handles that need to be waited on
    const int   cObjects = 1;   // no of objects that we are waiting on
    HACCEL      haccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

    //
    // Message loop lasts until we get a WM_QUIT message
    //
    for(;;)
    {
        if(pMpegMovie != NULL)
        {
            ahObjects[0] = pMpegMovie->GetMovieEventHandle();
        }
        else
        {
            ahObjects[0] = NULL;
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
                if(result == WAIT_OBJECT_0)
                {
                    VideoCd_OnGraphNotify();
                }
                continue;
            }
        }

        //
        // When here, we either have a message or no event handle
        // has been created yet.
        //
        // read all of the messages in this next loop,
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
    wc.hbrBackground = (HBRUSH)NULL; // (COLOR_BTNFACE + 1);
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
    POINT   pt;

    /*
    ** Save the instance handle in static variable, which will be used in
    ** many subsequence calls from this application to Windows.
    */
    hInst = hInstance;

    if(! LoadWindowPos(&rc))
    {
        rc.left = rc.top = 0;
        rc.bottom = rc.right = 400;
    }

    /*
    ** Create a main window for this application instance.
    */
    hwnd = CreateWindow(szClassName, IdStr(STR_APP_TITLE),
                        WS_THICKFRAME | WS_POPUP | WS_CAPTION  |
                        WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                        WS_CLIPCHILDREN,
                        rc.left, rc.top,
                        rc.right - rc.left, rc.bottom - rc.top,
                        NULL, NULL, hInstance, NULL);

    /*
    ** If window could not be created, return "failure"
    */
    if(NULL == hwnd)
    {
        return FALSE;
    }

    hwndApp = hwnd;
    nRecentFiles = GetRecentFiles(nRecentFiles);

    pt.x = lMovieOrgX = ProfileIntIn(g_szMovieX, 0);
    pt.y = lMovieOrgY = ProfileIntIn(g_szMovieY, 0);

    // if we fail to get the working area (screen-tray), then assume
    // the screen is 640x480
    //
    if(!SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE))
    {
        rc.top = rc.left = 0;
        rc.right = 640;
        rc.bottom = 480;
    }

    if(!PtInRect(&rc, pt))
    {
        lMovieOrgX = lMovieOrgY = 0L;
    }

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
* Place the movie in the center of the client window.  We do not stretch the
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
    RECT rc;
    GetClientRect(hwnd, &rc);

    rc.top += (dyToolbar + dyTrackbar);
    rc.bottom -= dyStatusbar;

    *xPos = rc.left;
    *yPos = rc.top;
    *pcx = rc.right - rc.left;
    *pcy = rc.bottom - rc.top;
}


/*****************************Private*Routine******************************\
* RepositionMovie
*
\**************************************************************************/
void
RepositionMovie(HWND hwnd)
{
    if(pMpegMovie)
    {
        // Allow the video to play for a short time (otherwise, VMR has no way 
        // to reallocate the DDraw surface) and [possibly] return to the point 
        // where we started.
        REFTIME rtPausedPosition = 0L;
        BOOL    bNeedToRewind = FALSE;
        BOOL    bStopped = FALSE;

        if( MOVIE_PLAYING != pMpegMovie->StatusMovie() )
        {
            if( MOVIE_STOPPED == pMpegMovie->StatusMovie() )
                bStopped = TRUE;

            rtPausedPosition = pMpegMovie->GetCurrentPosition();
            bNeedToRewind = TRUE;
            pMpegMovie->PlayMovie();
        }

        long xPos, yPos, cx, cy;
        GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
        pMpegMovie->PutMoviePosition(xPos, yPos, cx, cy);
    
        // Repaint the video window
        HDC hdcWin = GetDC(NULL);
        pMpegMovie->RepaintVideo(hwnd, hdcWin);
        ReleaseDC(hwnd, hdcWin);

        // Stop or pause the video to return to its previous state.
        // Seek to the previously displayed frame.
        if( bNeedToRewind )
        {
            if( bStopped )
            {
                pMpegMovie->StopMovie();
            }
            else
            {
                pMpegMovie->PauseMovie();
            }
            pMpegMovie->SeekToPosition( rtPausedPosition, TRUE);
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
        HANDLE_MSG(hwnd, WM_QUERYENDSESSION,   VideoCd_OnQueryEndSession);
        HANDLE_MSG(hwnd, WM_DESTROY,           VideoCd_OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,              VideoCd_OnSize);
        HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE,    VideoCd_OnSysColorChange);
        HANDLE_MSG(hwnd, WM_MENUSELECT,        VideoCd_OnMenuSelect);
        HANDLE_MSG(hwnd, WM_INITMENUPOPUP,     VideoCd_OnInitMenuPopup);
        HANDLE_MSG(hwnd, WM_HSCROLL,           VideoCd_OnHScroll);
        HANDLE_MSG(hwnd, WM_TIMER,             VideoCd_OnTimer);
        HANDLE_MSG(hwnd, WM_NOTIFY,            VideoCd_OnNotify);
        HANDLE_MSG(hwnd, WM_DROPFILES,         VideoCd_OnDropFiles);
        HANDLE_MSG(hwnd, WM_MOVE,              VideoCd_OnMove);

        // Note: we do not use HANDLE_MSG here as we want to call
        // DefWindowProc after we have notifed the FilterGraph Resource Manager,
        // otherwise our window will not finish its activation process.

        case WM_ACTIVATE: VideoCd_OnActivate(hwnd, wParam, lParam);

            // IMPORTANT - let this drop through to DefWindowProc

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
    RECT rc;
    int Pane[2];

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

    g_hwndStatusbar = CreateStatusWindow(WS_VISIBLE | WS_CHILD | CCS_BOTTOM,
        TEXT("Example Text"), hwnd, ID_STATUSBAR);

    GetWindowRect(g_hwndToolbar, &rc);
    dyToolbar = rc.bottom - rc.top;

    GetWindowRect(g_hwndStatusbar, &rc);
    dyStatusbar = rc.bottom - rc.top;
    dyTrackbar = 30;

    GetClientRect(hwnd, &rc);
    Pane[0] = (rc.right - rc.left) / 2 ;
    Pane[1] = -1;
    SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);


    g_hwndTrackbar = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"),
        WS_CHILD | WS_VISIBLE |
        TBS_AUTOTICKS | TBS_ENABLESELRANGE,
        LEFT_MARGIN, dyToolbar - 1,
        (rc.right - rc.left) - (2* LEFT_MARGIN),
        dyTrackbar, hwnd, (HMENU)ID_TRACKBAR,
        hInst, NULL);

    SetDurationLength((REFTIME)0);
    SetCurrentPosition((REFTIME)0);
    SetTimer(hwnd, StatusTimer, 100, NULL);

    if(g_hwndStatusbar == NULL || g_hwndTrackbar == NULL)
    {
        return FALSE;
    }

    // accept filemanager WM_DROPFILES messages
    DragAcceptFiles(hwnd, TRUE);

    return TRUE;
}


/*****************************Private*Routine******************************\
* VideoCd_OnActivate
*
\**************************************************************************/

void
VideoCd_OnActivate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    // If gaining focus, tell the renderer to set focus and 
    // repaint the video (which is necessary if playback is paused)
    if((UINT)LOWORD(wParam))
    {
        // we are being activated - tell the Filter graph (for Sound follows focus)
        if(pMpegMovie)
        {
            pMpegMovie->SetFocus();
            RepositionMovie(hwnd);
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnHScroll
*
\**************************************************************************/
void
VideoCd_OnHScroll(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    )
{
    static BOOL fWasPlaying = FALSE;
    static BOOL fBeginScroll = FALSE;

    if(pMpegMovie == NULL)
        return;

    if(hwndCtl == g_hwndTrackbar)
    {
        REFTIME     rtCurrPos;
        REFTIME     rtTrackPos;
        REFTIME     rtDuration;

        pos = (int)SendMessage(g_hwndTrackbar, TBM_GETPOS, 0, 0);
        rtTrackPos = (REFTIME)pos * g_TrackBarScale;

        switch(code)
        {
            case TB_BOTTOM:
                rtDuration = pMpegMovie->GetDuration();
                rtCurrPos = pMpegMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(rtDuration - rtCurrPos);
                SetCurrentPosition(pMpegMovie->GetCurrentPosition());
                break;

            case TB_TOP:
                rtCurrPos = pMpegMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(-rtCurrPos);
                SetCurrentPosition(pMpegMovie->GetCurrentPosition());
                break;

            case TB_LINEDOWN:
                VcdPlayerSeekCmd(10.0);
                SetCurrentPosition(pMpegMovie->GetCurrentPosition());
                break;

            case TB_LINEUP:
                VcdPlayerSeekCmd(-10.0);
                SetCurrentPosition(pMpegMovie->GetCurrentPosition());
                break;

            case TB_ENDTRACK:
                fBeginScroll = FALSE;
                if(fWasPlaying)
                {
                    VcdPlayerPauseCmd();
                    fWasPlaying = FALSE;
                }
                break;

            case TB_THUMBTRACK:
                if(!fBeginScroll)
                {
                    fBeginScroll = TRUE;
                    fWasPlaying = (g_State & VCD_PLAYING);
                    if(fWasPlaying)
                    {
                        VcdPlayerPauseCmd();
                    }
                }

            // Fall through
            case TB_PAGEUP:
            case TB_PAGEDOWN:
                rtCurrPos = pMpegMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(rtTrackPos - rtCurrPos);
                SetCurrentPosition(pMpegMovie->GetCurrentPosition());
                break;
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnTimer
*
\**************************************************************************/
void
VideoCd_OnTimer(
    HWND hwnd,
    UINT id
    )
{
    if(pMpegMovie && pMpegMovie->StatusMovie() == MOVIE_PLAYING)
    {
        switch(id)
        {
            case StatusTimer:
            {
                REFTIME rt = pMpegMovie->GetCurrentPosition();
                TCHAR   szFmt[64];
                char    sz[64];
                long cx, cy;
                static int iCharCount = 0;
                static int iCharOffset = 40;

                pMpegMovie->GetNativeMovieSize(&cx, &cy);
#ifdef UNICODE
                wsprintfA(sz, "%ls", FormatRefTime(szFmt, rt));
#else
                wsprintfA(sz, "%s", FormatRefTime(szFmt, rt));
#endif
                //
                // Prepare the text array
                //
                memset(g_szLine21Text, ' ', sizeof(g_szLine21Text));
                g_szLine21Text[ 40] = '\n';
                g_szLine21Text[ 81] = '\n';
                g_szLine21Text[122] = '\n';
                g_szLine21Text[163] = '\0';

                // add time
                memcpy(g_szLine21Text, sz, lstrlenA(sz));


                char* lpL1 = &g_szLine21Text[ 41];
                char* lpL2 = &g_szLine21Text[ 82];
                char* lpL3 = &g_szLine21Text[123];

                if(iCharCount <= 40)
                {
                    memcpy(lpL3, g_Line21Msg, iCharCount);
                }
                else if(iCharCount <= 80)
                {
                    memcpy(lpL2, g_Line21Msg +  0,              40);
                    memcpy(lpL3, g_Line21Msg + 40, iCharCount - 40);
                }
                else if(iCharCount <= 120)
                {
                    memcpy(lpL1, g_Line21Msg +  0,              40);
                    memcpy(lpL2, g_Line21Msg + 40,              40);
                    memcpy(lpL3, g_Line21Msg + 80, iCharCount - 80);
                }
                else
                {
                    memcpy(lpL1, g_Line21Msg + iCharOffset +  0, 40);
                    memcpy(lpL2, g_Line21Msg + iCharOffset + 40, 40);
                    memcpy(lpL3, g_Line21Msg + iCharOffset + 80, iCharCount - (iCharOffset + 80));

                    if(iCharCount % 40 == 0)
                    {
                        iCharOffset += 40;
                    }
                }

                iCharCount++;

                if(iCharCount > (sizeof(g_Line21Msg) - 1))
                {
                    iCharCount = 1;
                    iCharOffset = 40;
                }

                pMpegMovie->SetAppText(g_szLine21Text);
                SetCurrentPosition(rt);
            }
            break;
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
    RECT        rc1, rc2;

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

        case IDM_MOVIE_SKIP_FORE:
            VcdPlayerSeekCmd(1.0);
            break;

        case IDM_MOVIE_SKIP_BACK:
            VcdPlayerSeekCmd(-1.0);
            break;

        case IDM_MOVIE_PREVTRACK:
            if(pMpegMovie)
            {
                VcdPlayerSeekCmd(-pMpegMovie->GetCurrentPosition());
            }
            break;

        case IDM_MOVIE_STEP:
            VcdPlayerPauseCmd();
            VcdPlayerStepCmd();
            break;

        case IDM_TIME:
        case IDM_FRAME:
        case IDM_FIELD:
        case IDM_SAMPLE:
        case IDM_BYTES:
            if(pMpegMovie)
            {
                g_TimeFormat = VcdPlayerChangeTimeFormat(id);
            }
            break;

        case IDM_HELP_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX),
                hwnd,  (DLGPROC) AboutDlgProc);
            break;

        default:
            if(id > ID_RECENT_FILE_BASE
                && id <= (ID_RECENT_FILE_BASE + MAX_RECENT_FILES + 1))
            {
                ProcessOpen(aRecentFiles[id - ID_RECENT_FILE_BASE - 1]);
                VcdPlayerPlayCmd();
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
    // stop accepting dropped filenames
    DragAcceptFiles(hwnd, FALSE);

    VcdPlayerCloseCmd();
    ProfileIntOut(g_szMovieX, lMovieOrgX);
    ProfileIntOut(g_szMovieY, lMovieOrgY);

    SaveWindowPos(hwnd);
    DestroyWindow(hwnd);
}

/******************************Public*Routine******************************\
* VideoCd_OnQueryEndSession
*
\**************************************************************************/
BOOL
VideoCd_OnQueryEndSession(
    HWND hwnd
    )
{
    SaveWindowPos(hwnd);
    return TRUE;
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
    if(IsWindow(g_hwndStatusbar))
    {
        int Pane[2] = {dx/2-8, -1};

        SendMessage(g_hwndStatusbar, WM_SIZE, 0, 0L);
        SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);
    }

    if(IsWindow(g_hwndTrackbar))
    {
        SetWindowPos(g_hwndTrackbar, HWND_TOP, LEFT_MARGIN, dyToolbar - 1,
            dx - (2 * LEFT_MARGIN), dyTrackbar, SWP_NOZORDER);
    }

    if(IsWindow(g_hwndToolbar))
    {
        SendMessage(g_hwndToolbar, WM_SIZE, 0, 0L);
    }

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
    FORWARD_WM_SYSCOLORCHANGE(g_hwndStatusbar, SendMessage);
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

    switch(item)
    {
        case 0: // File menu
            if(g_State & (VCD_IN_USE | VCD_NO_CD | VCD_DATA_CD_LOADED))
            {
                uFlags = (MF_BYCOMMAND | MF_GRAYED);
            }
            else
            {
                uFlags = (MF_BYCOMMAND | MF_ENABLED);
            }
            EnableMenuItem(hMenu, IDM_FILE_CLOSE, uFlags);
            break;
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
    void
    )
{
    long lEventCode;

    lEventCode = pMpegMovie->GetMovieEventCode();

    switch(lEventCode)
    {
        case EC_STEP_COMPLETE:
            g_State &= ~VCD_STEPPING;
            SetPlayButtonsEnableState();
            break;

        case EC_COMPLETE:
            VcdPlayerRewindCmd();
            break;

        case EC_USERABORT:
        case EC_ERRORABORT:
            VcdPlayerStopCmd();
            SetPlayButtonsEnableState();
            break;

        default:
            break;
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnNotify
*
* This is where we get the text for the little tooltips
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


/*****************************Private*Routine******************************\
* VideoCd_OnMenuSelect
*
\**************************************************************************/
void
VideoCd_OnMenuSelect(
    HWND hwnd,
    HMENU hmenu,
    int item,
    HMENU hmenuPopup,
    UINT flags
    )
{
    TCHAR szString[STR_MAX_STRING_LEN + 1];

    /*
    ** Is it time to end the menu help ?
    */
    if((flags == 0xFFFFFFFF) && (hmenu == NULL))
    {
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }

    /*
    ** Do we have a separator, popup, or the system menu ?
    */
    else if(flags & MF_POPUP)
    {
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }
    else if(flags & MF_SYSMENU)
    {
        switch(item)
        {
            case SC_RESTORE:
                lstrcpy(szString, IdStr(STR_SYSMENU_RESTORE));
                break;

            case SC_MOVE:
                lstrcpy(szString, IdStr(STR_SYSMENU_MOVE));
                break;

            case SC_MINIMIZE:
                lstrcpy(szString, IdStr(STR_SYSMENU_MINIMIZE));
                break;

            case SC_MAXIMIZE:
                lstrcpy(szString, IdStr(STR_SYSMENU_MAXIMIZE));
                break;

            case SC_TASKLIST:
                lstrcpy(szString, IdStr(STR_SYSMENU_TASK_LIST));
                break;

            case SC_CLOSE:
                lstrcpy(szString, IdStr(STR_SYSMENU_CLOSE));
                break;
        }

        SendMessage(g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
            (LPARAM)(LPTSTR)szString);
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 1, 0L);
        UpdateWindow(g_hwndStatusbar);

    }

    /*
    ** Hopefully it's one of ours
    */
    else
    {
        if((flags & MF_SEPARATOR))
        {
            szString[0] = g_chNULL;
        }
        else
        {
            lstrcpy(szString, IdStr(item + MENU_STRING_BASE));

        }

        SendMessage(g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
            (LPARAM)(LPTSTR)szString);
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 1, 0L);
        UpdateWindow(g_hwndStatusbar);
    }
}

/*****************************Private*Routine******************************\
* VideoCd_OnDropFiles
*
* -- handle a file-manager drop of a filename to indicate a movie we should
*    open.
*
\**************************************************************************/
void
VideoCd_OnDropFiles(
    HWND hwnd,
    HDROP hdrop)
{
    // if there is more than one file, simply open the first one

    // find the length of the path (plus the null
    int cch = DragQueryFile(hdrop, 0, NULL, 0) + 1;
    TCHAR * pName = new TCHAR[cch];

    DragQueryFile(hdrop, 0, pName, cch);

    // open the file
    ProcessOpen(pName);

    // update the toolbar state
    SetPlayButtonsEnableState();

    // free up used resources
    delete [] pName;
    DragFinish(hdrop);
}


/******************************Public*Routine******************************\
* SetPlayButtonsEnableState
*
* Sets the play buttons enable state to match the state of the current
* cdrom device.  See below...
*
*
*                 VCD Player buttons enable state table
* -------------------------------------------------------------------
* �E=Enabled D=Disabled      � Play � Pause � Eject � Stop  � Other �
* -------------------------------------------------------------------
* �Disk in use               �  D   �  D    �  D    �   D   �   D   �
* -------------------------------------------------------------------
* �No video cd or data cdrom �  D   �  D    �  E    �   D   �   D   �
* -------------------------------------------------------------------
* �Video cd (playing)        �  D   �  E    �  E    �   E   �   E   �
* -------------------------------------------------------------------
* �Video cd (paused)         �  E   �  D    �  E    �   E   �   E   �
* -------------------------------------------------------------------
* �Video cd (stopped)        �  E   �  D    �  E    �   D   �   E   �
* -------------------------------------------------------------------
*
*
\**************************************************************************/
void
SetPlayButtonsEnableState(
    void
    )
{
    BOOL    fEnable;
    BOOL    fVideoLoaded;

    /*
    ** Do we have a video cd loaded.
    */
    if(g_State & (VCD_NO_CD | VCD_DATA_CD_LOADED | VCD_IN_USE))
        fVideoLoaded = FALSE;
    else
        fVideoLoaded = TRUE;

    /*
    ** Do the play button
    */
    if(fVideoLoaded
        && ((g_State & VCD_STOPPED) || (g_State & VCD_PAUSED)))
        fEnable = TRUE;
    else
        fEnable = FALSE;

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PLAY, fEnable);

    /*
    ** Do the stop button
    */
    if(fVideoLoaded
        && ((g_State & VCD_PLAYING) || (g_State & VCD_PAUSED)))
        fEnable = TRUE;
    else
        fEnable = FALSE;

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STOP, fEnable);


    /*
    ** Do the pause button
    */
    if(fVideoLoaded && (g_State & VCD_PLAYING))
        fEnable = TRUE;
    else
        fEnable = FALSE;

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PAUSE, fEnable);


    fEnable = FALSE;
    if(fVideoLoaded && pMpegMovie->CanMovieFrameStep())
    {
        if(!(g_State & VCD_STEPPING))
            fEnable = TRUE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STEP, fEnable);

    /*
    ** Do the seeking buttons
    */
    if((g_State & VCD_PAUSED) || (!fVideoLoaded))
        fEnable = FALSE;
    else
        fEnable = TRUE;

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_SKIP_FORE, fEnable);

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_SKIP_BACK, fEnable);

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_PREVTRACK, fEnable);
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

    GetWindowRect(g_hwndToolbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndTrackbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndStatusbar, &rcTool);
    prc->bottom -= (rcTool.bottom - rcTool.top);
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


/*****************************Private*Routine******************************\
* SetDurationLength
*
* Updates pane 0 on the status bar
*
\**************************************************************************/
void
SetDurationLength(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64];

    g_TrackBarScale = 1.0;
    while(rt / g_TrackBarScale > 30000)
    {
        g_TrackBarScale *= 10;
    }

    SendMessage(g_hwndTrackbar, TBM_SETRANGE, TRUE,
        MAKELONG(0, (WORD)(rt / g_TrackBarScale)));

    SendMessage(g_hwndTrackbar, TBM_SETTICFREQ, (WPARAM)((int)(rt / g_TrackBarScale) / 9), 0);
    SendMessage(g_hwndTrackbar, TBM_SETPAGESIZE, 0, (LPARAM)((int)(rt / g_TrackBarScale) / 9));

    wsprintf(sz, TEXT("Length: %s"), FormatRefTime(szFmt, rt));
    SendMessage(g_hwndStatusbar, SB_SETTEXT, 0, (LPARAM)sz);
}


/*****************************Private*Routine******************************\
* SetCurrentPosition
*
* Updates pane 1 on the status bar
*
\**************************************************************************/
void
SetCurrentPosition(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64], szCurrent[64];

    SendMessage(g_hwndTrackbar, TBM_SETPOS, TRUE, (LPARAM)(rt / g_TrackBarScale));

    wsprintf(sz, TEXT("Elapsed: %s"), FormatRefTime(szFmt, rt));

    // Read the current status bar text
    SendMessage(g_hwndStatusbar, SB_GETTEXT, 1, (LPARAM)szCurrent);

    // If this time is different, update the status bar display
    if (_tcscmp(sz, szCurrent))
        SendMessage(g_hwndStatusbar, SB_SETTEXT, 1, (LPARAM)sz);
}


/*****************************Private*Routine******************************\
* FormatRefTime
*
* Formats the given RefTime into the passed in character buffer,
* returns a pointer to the character buffer.
*
\**************************************************************************/
TCHAR *
FormatRefTime(
    TCHAR *sz,
    REFTIME rt
    )
{
    // If we are not seeking in time then format differently

    if(pMpegMovie && pMpegMovie->GetTimeFormat() != TIME_FORMAT_MEDIA_TIME)
    {
        wsprintf(sz,TEXT("%s"),(LPCTSTR) CDisp((LONGLONG) rt,CDISP_DEC));
        return sz;
    }

    int hrs, mins, secs;

    rt += 0.49;

    hrs  =  (int)rt / 3600;
    mins = ((int)rt % 3600) / 60;
    secs = ((int)rt % 3600) % 60;

    wsprintf(sz, TEXT("%02d:%02d:%02d h:m:s"),hrs, mins, secs);

    return sz;
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


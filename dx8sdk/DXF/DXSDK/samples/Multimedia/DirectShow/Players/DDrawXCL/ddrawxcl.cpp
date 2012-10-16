//------------------------------------------------------------------------------
// File: DDrawXcl.cpp
//
// Desc: DirectShow sample code - DDraw Exclusive Mode Video Playback 
//       test/sample application.
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <windows.h>
#include <commdlg.h>

#include <DVDEvCod.h>
#include "VidPlay.h"
#include "DDrawObj.h"
#include "DDrawXcl.h"

//
// WinMain(): Entry point to our sample app.
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG      msg ;
    HACCEL   hAccelTable ;
    
    DbgInitialise(hInstance) ;
    
    CoInitialize(NULL) ;
    
    ghInstance = hInstance ;
    LoadString(ghInstance, IDS_APP_TITLE, gszAppTitle,  100) ;
    LoadString(ghInstance, IDS_APP_NAME,  gszAppName,   10) ;
    
    if (! InitApplication() ) 
    {
        DbgTerminate() ;
        return FALSE ;
    } 
    
    if (! InitInstance(nCmdShow) ) 
    {
        DbgTerminate() ;
        return FALSE ;
    } 
    
    hAccelTable = LoadAccelerators(hInstance, gszAppName) ;
    
    //
    // Create a DDraw object and init it
    //
    gpDDrawObj = new CDDrawObject(ghWndApp) ;
    if (NULL == gpDDrawObj)
    {
        DbgTerminate() ;
        return FALSE ;
    }
    
    gpPlayer = NULL ;           // Init Video playback object pointer to NULL
    geVideoType = Unspecified ; // no video type specified on start up
    gbAppActive = TRUE ;        // app is activated on start up
    
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (! TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }
    
    //
    // Release the video playback object, if any
    //
    if (gpPlayer)
        delete gpPlayer ;
    
    //
    // Release DDraw now
    //
    delete gpDDrawObj ;
    
    CoUninitialize() ;
    
    DbgTerminate() ;
    return (int) (msg.wParam) ;
}


//
// InitApplication(): Registers the class if no other instance of this app is 
// already running.
//
BOOL InitApplication(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's InitApplication() entered"))) ;
    
    WNDCLASSEX  wc ;
    
    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
    ghWndApp = FindWindow (gszAppName, gszAppTitle) ;
    if (ghWndApp) {
        // We found another version of ourself. Lets defer to it:
        if (IsIconic(ghWndApp)) {
            ShowWindow(ghWndApp, SW_RESTORE);
        }
        SetForegroundWindow(ghWndApp);
        
        // If this app actually had any functionality, we would
        // also want to communicate any action that our 'twin'
        // should now perform based on how the user tried to
        // execute us.
        return FALSE;
    }
    
    // Register the app main window class
    wc.cbSize        = sizeof(wc) ;
    wc.style         = CS_HREDRAW | CS_VREDRAW ;
    wc.lpfnWndProc   = (WNDPROC) WndProc ;
    wc.cbClsExtra    = 0 ;
    wc.cbWndExtra    = 0 ;
    wc.hInstance     = ghInstance ;
    wc.hIcon         = LoadIcon(ghInstance, gszAppName) ;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1) ;
    wc.lpszMenuName  = gszAppName ;
    wc.lpszClassName = gszAppName ;
    wc.hIconSm       = NULL ;
    if (0 == RegisterClassEx(&wc))
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: RegisterClassEx() for app class failed (Error %ld)"), 
            GetLastError())) ;
        return FALSE ;
    }
    
    return TRUE ;
}


//
// InitInstance(): Starts this instance of the app (creates the window).
//
BOOL InitInstance(int nCmdShow)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's InitInstance() entered"))) ;
    
    ghWndApp = CreateWindowEx(0, gszAppName, gszAppTitle, 
        WS_OVERLAPPEDWINDOW,
        0, 0, 
        DEFAULT_WIDTH, DEFAULT_HEIGHT,
        NULL, NULL, ghInstance, NULL);

    if (! ghWndApp ) {
        return FALSE ;
    }
    
    ShowWindow(ghWndApp, nCmdShow);
    UpdateWindow(ghWndApp) ;
    
    return TRUE ;
}


//
// MenuProc(): Handles menu choices picked by the user.
//
LRESULT CALLBACK MenuProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HMENU hMenu = GetMenu(hWnd) ;
    int   wmId    = LOWORD(wParam);
    // int   wmEvent = HIWORD(wParam);
    
    //Parse the menu selections:
    switch (wmId) {
        
    case IDM_SELECTDVD:
        if (FileSelect(hWnd, DVD))  // selection changed
        {
            CheckMenuItem(hMenu, IDM_SELECTDVD,  MF_CHECKED) ;
            CheckMenuItem(hMenu, IDM_SELECTFILE, MF_UNCHECKED) ;
        }
        break;
        
    case IDM_SELECTFILE:
        if (FileSelect(hWnd, File))  // selection changed
        {
            CheckMenuItem(hMenu, IDM_SELECTDVD,  MF_UNCHECKED) ;
            CheckMenuItem(hMenu, IDM_SELECTFILE, MF_CHECKED) ;
        }
        break;
        
    case IDM_ABOUT:
        DialogBox(ghInstance, TEXT("AboutBox"), ghWndApp, (DLGPROC) About);
        break;
        
    case IDM_EXIT:
        DestroyWindow(ghWndApp);
        break;
        
    case IDM_STARTPLAY:
        if (StartPlay(hWnd))  // playback started successfully
            gbAppActive = TRUE ;   // if we are playing, we must be active
        else                  // playback failed due to some reason
            MessageBox(hWnd, 
                STR_EXCLUSIVE_MODE_FAILURE,
                TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
        break;
        
    default:
        break ;
    }
    
    return 0 ;
}


//
// FileSelect(): Lets the user specify the file to play.
//
BOOL FileSelect(HWND hWnd, VIDEO_TYPE eType)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's FileSelect(%s) entered"),
        DVD == eType ? "DVD" : "File")) ;
    
    OPENFILENAME  ofn ;
    TCHAR         achFileName[MAX_PATH] ;
    
    // Init the filename buffer with either a filename or *.ifo
    if (DVD == eType)
        lstrcpy(achFileName, TEXT("*.ifo")) ;
    else
        lstrcpy(achFileName, TEXT("*.avi")) ;
    
    ZeroMemory(&ofn, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;
    if (DVD == eType)
    {
        ofn.lpstrTitle = TEXT("Select DVD-Video Volume\0") ;
        ofn.lpstrFilter = TEXT("IFO Files\0*.ifo\0All Files\0*.*\0\0") ;
    }
    else
    {
        ofn.lpstrTitle = TEXT("Select Video file\0") ;
        ofn.lpstrFilter = TEXT("AVI Files\0*.avi\0MPEG Files\0*.mpg\0All Files\0*.*\0\0") ;
    }
    ofn.nFilterIndex = 1 ;
    ofn.lpstrFile = achFileName ;
    ofn.nMaxFile = sizeof(achFileName) ;
    ofn.lpstrFileTitle = NULL ;
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir = NULL ;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY ;
    
    if (GetOpenFileName(&ofn)) // user specified a file
    {
        if (! CreatePlayer(eType)  ||  // creating player failed!!
            NULL == gpPlayer )         // how!?!
        {
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: Couldn't create %s player"),
                DVD == eType ? "DVD" : "File")) ;
            return FALSE ;
        }
        gpPlayer->SetFileName(achFileName) ;
        return TRUE ;  // user specified file name
    }
    
    // Either failed or user hit Esc.
    DbgLog((LOG_TRACE, 3, TEXT("GetOpenFileName() cancelled/failed (Error %lu)"), 
            CommDlgExtendedError())) ;
    return FALSE ; // DVD-Video volume not changed
}


BOOL IsVideoTypeKnown(void)
{
    return (Unspecified != geVideoType) ;
}


VIDEO_TYPE GetVideoType(void)
{
    return geVideoType ;
}


BOOL CreatePlayer(VIDEO_TYPE eType)
{
    if (geVideoType == eType)  // same type as before
    {
        if (gpPlayer)          // we have already have the player
            return TRUE     ;  // we'll use the same one; everything is OK
    }
    else                       // video type has changed
    {
        if (gpPlayer)          // we created a player before...
        {
            delete gpPlayer ;  // release it now
            gpPlayer = NULL ;
        }
    }
    
    // If we are here, we need to create a new player of the specified type
    if (DVD == eType)
        gpPlayer = new CDVDPlayer ;
    else if (File == eType)
        gpPlayer = new CFilePlayer ;
    else  // what then??
    {
        ASSERT(FALSE) ;
        return NULL ;
    }
    
    geVideoType = eType ;   // this our current video type
    
    return TRUE ;
}


//
// OnEndOfPlayback(): Releases everything on end of playback (but checks to
// avoid doing it too many times) as it "may be" called a little more than
// we would like to.
//
void OnEndOfPlayback(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's OnEndOfPlayback() entered"))) ;
    
    if (0 != guTimerID)  // if any timer is still set
    {
        BOOL bRes = KillTimer(hWndApp, TIMER_ID) ;  // don't need that timer anymore
        ASSERT(bRes) ;      bRes = bRes;  // Suppress C4189 warning
        guTimerID = 0 ;  // timer released
    }
    
    if (gpPlayer && gpPlayer->IsGraphReady())
    {
        DbgLog((LOG_TRACE, 5, TEXT("Turn off color keying before stopping the graph"))) ;
        gpDDrawObj->SetOverlayState(FALSE) ;   // don't paint color key in video's position
#ifndef NOFLIP
        gpDDrawObj->UpdateAndFlipSurfaces() ;  // flip the surface so that video doesn't show anymore
#endif // NOFLIP
        
        gpPlayer->Stop() ;
        
        // Remove the overlay callback interface for OverlayMixer
        HRESULT hr = gpPlayer->SetOverlayCallback(NULL) ;
        ASSERT(SUCCEEDED(hr)) ;     hr = hr;  // Suppress C4189 warning
        
        gpPlayer->ClearGraph() ;
    }
    
    if (gpDDrawObj->IsInExclusiveMode())
    {
        gpDDrawObj->StopExclusiveMode(hWndApp) ;

        // Resize main window to default size
        SetWindowPos(ghWndApp, HWND_TOP, 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);
    }
}


//
// WndPorc(): Message handles for our sample app.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC          hDC ;
    static PAINTSTRUCT  ps ;
    
    switch (message) {
        
    case WM_ACTIVATEAPP:
        //
        // NOTE: We don't try to recover playback on switching back into focus.
        //
        gbAppActive = (BOOL) wParam ;
        if (! gbAppActive )  // losing activation
        {
            DbgLog((LOG_TRACE, 2, TEXT("Got a WM_ACTIVATEAPP message with Active = %s"),
                gbAppActive ? "TRUE" : "FALSE")) ;
            OnEndOfPlayback(hWnd) ;  // should stop playback now
        }
        break ;
        
    case WM_TIMER:
        DbgLog((LOG_TRACE, 4, TEXT("Got a WM_TIMER message with ID = %ld (Active = %s)"), 
            wParam, gbAppActive ? "T" : "F")) ;
        if ( TIMER_ID != wParam  ||      // this is not the timer we have set  or
            ! gbAppActive ||             // the app isn't active anymore  or
            ! gpPlayer->IsGraphReady() ) // the graph is not currently ready
            // (...but we should have turned timer off then anyway!!)
            break ;     // don't do anything

        //
        // We could do some status update here that could be used by the
        // UpdateAndFlipSurfaces() call below.
        //
#ifndef NOFLIP
        gpDDrawObj->UpdateAndFlipSurfaces() ;
#endif // NOFLIP
        
        break;
        
    case WM_COMMAND:
        DbgLog((LOG_TRACE, 4, TEXT("Got a WM_COMMAND message with wParam = %ld"), wParam)) ;
        MenuProc(hWnd, wParam, lParam) ;
        break;
        
    case WM_PLAY_EVENT:
        DbgLog((LOG_TRACE, 4, TEXT("Got a WM_PLAY_EVENT message with wParam = %ld"), wParam)) ;
        if (1 == OnPlaybackEvent(hWnd, wParam, lParam))  // playback ended
            OnEndOfPlayback(hWnd) ;     // do the necessary things
        break ;
        
    case WM_SIZE_CHANGE:
        DbgLog((LOG_TRACE, 4, TEXT("Got a WM_SIZE_CHANGE message"))) ;
        if (gpPlayer->IsGraphReady()) // ONLY if the graph is ready
            SetVideoPosition(FALSE) ;
        else
            DbgLog((LOG_TRACE, 1, TEXT("WARNING: Got a WM_SIZE_CHANGE message after graph was released!!"))) ;
        break ;
        
    case WM_KEYUP:
        DbgLog((LOG_TRACE, 4, TEXT("Got a WM_KEYUP message with wParam = %ld"), wParam)) ;
        KeyProc(hWnd, wParam, lParam) ;
        break ;
        
    case WM_DESTROY:
        OnEndOfPlayback(hWnd) ;  // must stop playback before quitting
        PostQuitMessage(0);
        break;
        
    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    
    return 0 ;
}


//
// KeyProc(): Handles key presses to exit playback (on Esc) or move the ball
// using arrow keys.
//
LRESULT CALLBACK KeyProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's KeyProc() entered"))) ;
    
    switch (wParam)
    {
    case VK_ESCAPE:
        OnEndOfPlayback(hWnd) ;
        break ;
        
    case VK_UP:
        gpDDrawObj->MoveBallPosition(0, -BALL_STEP) ;
        break ;
        
    case VK_DOWN:
        gpDDrawObj->MoveBallPosition(0, BALL_STEP) ;
        break ;
        
    case VK_LEFT:
        gpDDrawObj->MoveBallPosition(-BALL_STEP, 0) ;
        break ;
        
    case VK_RIGHT:
        gpDDrawObj->MoveBallPosition(BALL_STEP, 0) ;
        break ;
        
    default:
        break ;
    }
    
    return 0 ;
}


//
// About(): Dialog box code for the About box.
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's About() entered"))) ;
    
    switch (message) {
        case WM_INITDIALOG:
            return TRUE;
        
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;
        
        default:
            break ;
    }
    
    return FALSE;
}


//
// OnPlaybackEvent(): Handles playback events, specially completed/stopped-on-error.
//
// Returns 0 if playback is continuing.
// Returns 1 if playback is over for any reason.
//
LRESULT OnPlaybackEvent(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5,
        TEXT("App's OnPlaybackEvent(0x%lx, 0x%lx) entered"),
        wParam, lParam)) ;
    
    IMediaEventEx  *pME = (IMediaEventEx *) lParam ;
    if (NULL == pME  || Playing != gpPlayer->GetState())
    {
        DbgLog((LOG_TRACE, 1, TEXT("Either pME = NULL or not playing anymore. Skip everything."))) ;
        return 0 ;  // or 1 ??
    }
    
    LONG     lEvent ;
    LONG_PTR   lParam1, lParam2 ;
    
    //
    //  Because the message mode for IMediaEvent may not be set before
    //  we get the first event it's important to read all the events
    //  pending when we get a window message to say there are events pending.
    //  GetEvent() returns E_ABORT when no more event is left.
    //
    while (SUCCEEDED(pME->GetEvent(&lEvent, &lParam1, &lParam2, 0))) // no wait
    {
        switch (lEvent)
        {
        //
        // First the DVD related events
        //
        case EC_DVD_STILL_ON:
            DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_STILL_ON"))) ;
            break ;
            
        case EC_DVD_STILL_OFF:
            DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_STILL_OFF"))) ;
            break ;
            
        case EC_DVD_DOMAIN_CHANGE:
            DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_DOMAIN_CHANGE, %ld"), lParam1)) ;
            switch (lParam1)
            {
                case DVD_DOMAIN_FirstPlay:  // = 1
                case DVD_DOMAIN_Stop:       // = 5
                    break ;
                
                case DVD_DOMAIN_VideoManagerMenu:  // = 2
                case DVD_DOMAIN_VideoTitleSetMenu: // = 3
                    // Inform the app to update the menu option to show "Resume" now
                    break ;
                
                case DVD_DOMAIN_Title:      // = 4
                    // Inform the app to update the menu option to show "Menu" again
                    break ;
                
                default: // hmmmm...
                    break ;
            }
            break ;
            
            case EC_DVD_BUTTON_CHANGE:
                DbgLog((LOG_TRACE, 5, TEXT("DVD Event: Button Changed to %d out of %d"),
                    lParam2, lParam1));
                break;
                
            case EC_DVD_TITLE_CHANGE:
                DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_TITLE_CHANGE"))) ;
                break ;
                
            case EC_DVD_CHAPTER_START:
                DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_CHAPTER_START"))) ;
                break ;
                
            case EC_DVD_CURRENT_TIME:
                DbgLog((LOG_TRACE, 5, TEXT("Playback Event: EC_DVD_CURRENT_TIME"))) ;
                break ;
                
            //
            // Then the general DirectShow related events
            //
            case EC_COMPLETE:
                DbgLog((LOG_TRACE, 5, TEXT("Playback Event: Playback complete"))) ;
                MessageBeep(MB_OK) ; // just to inform that the playback is over
                
                // Remember to free the event params
                pME->FreeEventParams(lEvent, lParam1, lParam2) ;
                
                // We don't do the release part here. That will be done in WndProc()
                // after return from this function.
                return 1 ;  // playback over
                
            case EC_USERABORT:
            case EC_ERRORABORT:
                DbgLog((LOG_TRACE, 5, TEXT("Playback Event: 0x%lx"), lEvent)) ;
                MessageBeep(MB_ICONEXCLAMATION) ; // to inform that the playback has errored out
                
                // Remember to free the event params
                pME->FreeEventParams(lEvent, lParam1, lParam2) ;
                
                // We don't do the release part here. That will be done in WndProc()
                // after return from this function.
                return 1 ;  // playback over
                
            default:
                DbgLog((LOG_TRACE, 5, TEXT("Ignored unknown playback event: 0x%lx"), lEvent)) ;
                break ;
        }
        
        //
        // Remember to free the event params
        //
        pME->FreeEventParams(lEvent, lParam1, lParam2) ;
        
    }  // end of while (GetEvent()) loop
    
    return 0 ;
}


typedef enum _AM_TRANSFORM
{
    AM_SHRINK,
    AM_STRETCH
} AM_TRANSFORM ;

void TransformRect(RECT *prRect, double dPictAspectRatio, AM_TRANSFORM transform)
{
    double dWidth, dHeight, dNewWidth, dNewHeight ;
    
    double dResolutionRatio = 0.0, dTransformRatio = 0.0 ;
    
    ASSERT(transform == AM_SHRINK || transform == AM_STRETCH) ;
    
    dNewWidth = dWidth = prRect->right - prRect->left ;
    dNewHeight = dHeight = prRect->bottom - prRect->top ;
    
    dResolutionRatio = dWidth / dHeight ;
    dTransformRatio = dPictAspectRatio / dResolutionRatio ;
    
    // shrinks one dimension to maintain the coorect aspect ratio
    if (transform == AM_SHRINK)
    {
        if (dTransformRatio > 1.0)
        {
            dNewHeight = dNewHeight / dTransformRatio ;
        }
        else if (dTransformRatio < 1.0)
        {
            dNewWidth = dNewWidth * dTransformRatio ;
        }
    }
    // stretches one dimension to maintain the coorect aspect ratio
    else if (transform == AM_STRETCH)
    {
        if (dTransformRatio > 1.0)
        {
            dNewWidth = dNewWidth * dTransformRatio ;
        }
        else if (dTransformRatio < 1.0)
        {
            dNewHeight = dNewHeight / dTransformRatio ;
        }
    }
    
    if (transform == AM_SHRINK)
    {
        ASSERT(dNewHeight <= dHeight) ;
        ASSERT(dNewWidth <= dWidth) ;
    }
    else
    {
        ASSERT(dNewHeight >= dHeight) ;
        ASSERT(dNewWidth >= dWidth) ;
    }
    
    // cut or add equal portions to the changed dimension
    
    prRect->left += (LONG)(dWidth - dNewWidth)/2 ;
    prRect->right = prRect->left + (LONG)dNewWidth ;
    
    prRect->top += (LONG)(dHeight - dNewHeight)/2 ;
    prRect->bottom = prRect->top + (LONG)dNewHeight ;
}


//
// SetVideoPosition(): Gets the original video size and positions it at the center.
// 
void SetVideoPosition(BOOL bSetBallPosition)
{
    DbgLog((LOG_TRACE, 5, TEXT("App's SetVideoPosition() entered"))) ;
    
    DWORD  dwVideoWidth, dwVideoHeight ;
    DWORD  dwARX, dwARY ;
    gpPlayer->GetNativeVideoData(&dwVideoWidth, &dwVideoHeight, &dwARX, &dwARY) ;
    DbgLog((LOG_TRACE, 5, TEXT("Native video size: %lu x %lu, Aspect Ratio: %lu x %lu"), 
        dwVideoWidth, dwVideoHeight, dwARX, dwARY)) ;
    
    // Update output size to make it aspect ratio corrected
    RECT  rectCorrected ;
    SetRect(&rectCorrected, 0, 0, dwVideoWidth, dwVideoHeight) ;
    TransformRect(&rectCorrected, (double)dwARX / (double)dwARY, AM_STRETCH) ;
    DbgLog((LOG_TRACE, 5, TEXT("Updated video size: %ld x %ld"), 
        RECTWIDTH(rectCorrected), RECTHEIGHT(rectCorrected))) ;
    
    RECT   ScrnRect ;
    gpDDrawObj->GetScreenRect(&ScrnRect) ;
    DbgLog((LOG_TRACE, 5, TEXT("Screen size is %ld x %ld"), 
            RECTWIDTH(ScrnRect), RECTHEIGHT(ScrnRect))) ;
    
    DWORD  dwVideoTop ;
    DWORD  dwVideoLeft ;
    if (RECTWIDTH(rectCorrected)  <= RECTWIDTH(ScrnRect)  &&  // video width less than screen
        RECTHEIGHT(rectCorrected) <= RECTHEIGHT(ScrnRect))    // video height less than screen
    {
        dwVideoLeft = (RECTWIDTH(ScrnRect)  - RECTWIDTH(rectCorrected)) / 2 ;
        dwVideoTop  = (RECTHEIGHT(ScrnRect) - RECTHEIGHT(rectCorrected)) / 2 ;
    }
    else                            // video width more than screen
    {
        rectCorrected = ScrnRect ;
        TransformRect(&rectCorrected, (double)dwARX / (double)dwARY, AM_SHRINK) ;
        dwVideoLeft = rectCorrected.left ;
        dwVideoTop  = rectCorrected.top ;
    }
    gpDDrawObj->SetVideoPosition(dwVideoLeft, dwVideoTop, 
        RECTWIDTH(rectCorrected), RECTHEIGHT(rectCorrected)) ;
    if (bSetBallPosition)           // if ball position should be (re)set
        gpDDrawObj->SetBallPosition(dwVideoLeft, dwVideoTop, 
                                RECTWIDTH(rectCorrected), RECTHEIGHT(rectCorrected)) ;
    else                            // don't reset the ball position, just...
        gpDDrawObj->MoveBallPosition(0, 0) ;  // ... make sure it's in the new video area
    gpPlayer->SetVideoPosition(dwVideoLeft, dwVideoTop, 
                                RECTWIDTH(rectCorrected), RECTHEIGHT(rectCorrected)) ;
    DbgLog((LOG_TRACE, 5, TEXT("Video is %ld x %ld at (%ld x %ld)"), 
            RECTWIDTH(rectCorrected), RECTHEIGHT(rectCorrected), dwVideoLeft, dwVideoTop)) ;
}


//
// StartPlay(): Switches to fulscreen exclusive mode, sets up for selected media
// playback, gets the video size and positions video at the center, starts playing
// and sets a timer to tell WndProc() every 1/10 second.
//
LRESULT StartPlay(HWND hWndApp)
{
    HRESULT  hr ;
    
    DbgLog((LOG_TRACE, 5, TEXT("App's StartPlay() entered"))) ;
    
    if (! IsVideoTypeKnown() )
    {
        MessageBox(hWndApp, 
            TEXT("No playback option (DVD/File) has been specified through the menu option yet.\nCan't run test."), 
            TEXT("Sorry"), MB_OK | MB_ICONINFORMATION) ;
        return 0 ;
    }
    
    //
    // Make sure that DShow components are installed etc so that
    // the graph building can be init-ed
    //
    if (! gpPlayer->Initialize() )
    {
        MessageBox(hWndApp, 
            TEXT("DShow components couldn't be initialized.\n\nCan't run test.\nPlease check DxMedia installation"), 
            TEXT("Sorry"), MB_OK | MB_ICONSTOP) ;
        return 0 ;
    }
    
    //
    // Go into fullscreen exclusive mode
    //
    hr = gpDDrawObj->StartExclusiveMode(hWndApp) ;
    if (FAILED(hr))  // error message shown by the above method
    {
        gpPlayer->ClearGraph() ;
        return 0 ;
    }
    
    //
    // Build video playback graph
    //
    hr = gpPlayer->BuildGraph(hWndApp, gpDDrawObj->GetDDObject(), gpDDrawObj->GetDDPrimary()) ;
    if (FAILED(hr))
    {
        gpPlayer->ClearGraph() ;
        gpDDrawObj->StopExclusiveMode(hWndApp) ;  // get out of exclusive mode
        return 0 ;
    }
    
    //
    // Specify the overlay callback interface for OverlayMixer to notify us
    //
    hr = gpPlayer->SetOverlayCallback(gpDDrawObj->GetCallbackInterface()) ;
    ASSERT(SUCCEEDED(hr)) ;
    
    //
    // Pause the video playback graph to get it ready to play
    //
    BOOL bSuccess = gpPlayer->Pause() ;
    if (!bSuccess)
    {
        gpPlayer->SetOverlayCallback(NULL) ;      // first remove overlay callback
        gpPlayer->ClearGraph() ;                  // then remove graph
        gpDDrawObj->StopExclusiveMode(hWndApp) ;  // then get out of exclusive mode
        return 0 ;
    }
    
    //
    // Get the color key info from the Player object and pass it to the DDraw object
    //
    DWORD   dwVideoColorKey ;
    gpPlayer->GetColorKey(&dwVideoColorKey) ;
    gpDDrawObj->SetColorKey(dwVideoColorKey) ;
    
    //
    // Get the video width and height, center it and pass the coordinates to 
    // the player and the DDraw object
    //
    SetVideoPosition(TRUE) ;
    
    //
    // Create the first screen on back buffer and then flip
    //
#ifndef NOFLIP
    gpDDrawObj->UpdateAndFlipSurfaces() ;
#endif // NOFLIP
    
    //
    // Play video now...
    //
    if (! gpPlayer->Play() )
    {
        gpPlayer->SetOverlayCallback(NULL) ;      // first remove overlay callback
        gpPlayer->ClearGraph() ;                  // then remove graph
        gpDDrawObj->StopExclusiveMode(hWndApp) ;  // then get out of exclusive mode
        return 0 ;
    }
    
    //
    // Now set a timer based on which we'll update the buffers and flip
    //
    guTimerID = (UINT) SetTimer(hWndApp, TIMER_ID, TIMER_RATE, NULL) ;
    ASSERT(0 != guTimerID) ;
    
    // We are done with starting the playback.  WndProc will stop the playback on
    // playback event messages or user hitting Esc key as well the timer based
    // actions will be taken in WM_TIMER handler there.
    
    return 1 ;
}

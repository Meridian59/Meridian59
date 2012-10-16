//------------------------------------------------------------------------------
// File: CutScene.cpp
//
// Desc: DirectShow sample code - simple interactive movie player.  Plays
//       a movie or game cutscene in fullscreen mode.  Supports simple user
//       input to enable ESC, spacebar, or ENTER to quit.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <dshow.h>
#include <stdio.h>
#include <atlbase.h>

#include "cutscene.h"

//
// Constants
//
#define KEYBOARD_SAMPLE_FREQ  100  // Sample user input on an interval
#define CUTSCENE_NAME   TEXT("Cutscene Player Sample")

//
// Globals
//
static IGraphBuilder  *pGB = NULL;
static IMediaControl  *pMC = NULL;
static IVideoWindow   *pVW = NULL;
static IMediaEvent    *pME = NULL;

static HWND g_hwndMain=0;
static BOOL g_bContinue=TRUE, g_bUserInterruptedPlayback=FALSE;


//
// Function prototypes
//
static HRESULT PlayMedia(LPTSTR lpszMovie, HINSTANCE hInstance);
static HRESULT GetInterfaces(void);
static HRESULT SetFullscreen(void);
static BOOL CreateHiddenWindow( HINSTANCE hInstance, TCHAR *szFile );
static LONG WINAPI WindowProc(HWND, UINT, WPARAM, LPARAM);
static void CloseApp();
static void CleanupInterfaces(void);
static void Msg(TCHAR *szFormat, ...);


//
// Helper Macros (Jump-If-Failed, Log-If-Failed)
//
#define RELEASE(i) {if (i) i->Release(); i = NULL;}

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); goto CLEANUP;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr); return hr;}



HRESULT PlayCutscene(LPTSTR szMovie, HINSTANCE hInstance)
{
    HRESULT hr;

    // Create the main hidden window to field keyboard input
    if (!CreateHiddenWindow(hInstance, szMovie))
        return E_FAIL;

    // Initialize COM
    if (FAILED(hr = CoInitialize(NULL)))
        return hr;

    // Get DirectShow interfaces
    if (FAILED(hr = GetInterfaces()))
    {
        CoUninitialize();
        return hr;
    }

    // Play the movie / cutscene
    hr = PlayMedia(szMovie, hInstance);

    // If the user interrupted playback and there was no other error,
    // return S_FALSE.
    if ((hr == S_OK) && g_bUserInterruptedPlayback)
        hr = S_FALSE;

    // Release DirectShow interfaces
    CleanupInterfaces();
    CoUninitialize();

    return hr;
}


BOOL CreateHiddenWindow( HINSTANCE hInstance, TCHAR *szFile )
{
    TCHAR szTitle[MAX_PATH];

    // Set up and register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = (WNDPROC) WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CUTSCENE_NAME;
    if (!RegisterClass(&wc))
        return FALSE;

    wsprintf(szTitle, TEXT("%s: %s"), CUTSCENE_NAME, szFile);

    // Create a window of zero size that will serve as the sink for
    // keyboard input.  If this media file has a video component, then
    // a second ActiveMovie window will be displayed in which the video
    // will be rendered.  Setting keyboard focus on this application window
    // will allow the user to move the video window around the screen, make
    // it full screen, resize, center, etc. independent of the application
    // window.  If the media file has only an audio component, then this will
    // be the only window created.
    g_hwndMain = CreateWindowEx(
        0, CUTSCENE_NAME, szTitle,
        0,            // not visible
        0, 0, 0, 0,
        NULL, NULL, hInstance, NULL );

    return (g_hwndMain != NULL);
}


LONG WINAPI WindowProc( HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
        // Monitor keystrokes for manipulating video window
        // and program options
        case WM_KEYDOWN:
            switch( wParam )
            {
                case VK_ESCAPE:
                case VK_SPACE:
                case VK_RETURN:
                    g_bUserInterruptedPlayback = TRUE;
                    CloseApp();
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return (LONG) DefWindowProc(hWnd, message, wParam, lParam);
}



HRESULT GetInterfaces(void)
{
    HRESULT hr = S_OK;

    // Instantiate filter graph interface
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
                         IID_IGraphBuilder, (void **)&pGB));

    // Get interfaces to control playback & screensize
    JIF(pGB->QueryInterface(IID_IMediaControl,  (void **)&pMC));
    JIF(pGB->QueryInterface(IID_IVideoWindow,   (void **)&pVW));

    // Get interface to allow the app to wait for completion of playback
    JIF(pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME));

    return S_OK;

    // In case of failure, the helper macro jumps here
CLEANUP:
    CleanupInterfaces();
    return(hr);
}


void CleanupInterfaces(void)
{
    // Release the DirectShow interfaces
    RELEASE(pGB);
    RELEASE(pMC);
    RELEASE(pVW);
    RELEASE(pME);

    DestroyWindow(g_hwndMain);
}


void CloseApp()
{
    // Stop playback and exit
    pMC->Stop();
    g_bContinue = FALSE;

    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
}



HRESULT PlayMedia(LPTSTR lpszMovie, HINSTANCE hInstance)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;
    WCHAR wFileName[MAX_PATH];
    BOOL bSleep=TRUE;

    wcscpy(wFileName, T2W(lpszMovie));

    // Allow DirectShow to create the FilterGraph for this media file
    hr = pGB->RenderFile(wFileName, NULL);
    if (FAILED(hr)) {
        Msg(TEXT("Failed(0x%08lx) in RenderFile(%s)!\r\n"), hr, lpszMovie);
        return hr;
    }

    // Set the message drain of the video window to point to our hidden
    // application window.  This allows keyboard input to be transferred
    // to our main window for processing.
    //
    // If this is an audio-only or MIDI file, then put_MessageDrain will fail.
    //
    hr = pVW->put_MessageDrain((OAHWND) g_hwndMain);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed(0x%08lx) to set message drain for %s.\r\n\r\n")
            TEXT("This sample is designed to play videos, but the file selected ")
            TEXT("has no video component."), hr, lpszMovie);
        return hr;
    }

    // Set fullscreen
    hr = SetFullscreen();
    if (FAILED(hr)) {
        Msg(TEXT("Failed(%08lx) to set fullscreen!\r\n"), hr);
        return hr;
    }

    // Display first frame of the movie
    hr = pMC->Pause();
    if (FAILED(hr)) {
        Msg(TEXT("Failed(%08lx) in Pause()!\r\n"), hr);
        return hr;
    }

    // Start playback
    hr = pMC->Run();
    if (FAILED(hr)) {
        Msg(TEXT("Failed(%08lx) in Run()!\r\n"), hr);
        return hr;
    }

    // Update state variables
    g_bContinue = TRUE;

    // Enter a loop of checking for events and sampling keyboard i6nput
    while (g_bContinue)
    {
        MSG msg;
        long lEventCode, lParam1, lParam2;

        // Reset sleep flag
        bSleep = TRUE;

        // Has there been a media event?  Look for end of stream condition.
        if(E_ABORT != pME->GetEvent(&lEventCode, (LONG_PTR *) &lParam1, 
                                    (LONG_PTR *) &lParam2, 0))
        {
            // Free the media event resources.
            hr = pME->FreeEventParams(lEventCode, lParam1, lParam2);
            if (FAILED(hr))
            {
                Msg(TEXT("Failed(%08lx) to free event params (%s)!\r\n"),
                    hr, lpszMovie);
            }

            // Is this the end of the movie?
            if (lEventCode == EC_COMPLETE)
            {
                g_bContinue = FALSE;
                bSleep = FALSE;
            }
        }

        // Give system threads time to run (and don't sample user input madly)
        if (bSleep)
            Sleep(KEYBOARD_SAMPLE_FREQ);

        // Check and process window messages (like our keystrokes)
        while (PeekMessage (&msg, g_hwndMain, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return hr;
}


HRESULT SetFullscreen(void)
{
    HRESULT hr=S_OK;
    LONG lMode;
    static HWND hDrain=0;

    if (!pVW)
        return S_FALSE;

    // Read current state
    LIF(pVW->get_FullScreenMode(&lMode));

    if (lMode == 0)  /* OAFALSE */
    {
        // Save current message drain
        LIF(pVW->get_MessageDrain((OAHWND *) &hDrain));

        // Set message drain to application main window
        LIF(pVW->put_MessageDrain((OAHWND) g_hwndMain));

        // Switch to full-screen mode
        lMode = -1;  /* OATRUE */
        LIF(pVW->put_FullScreenMode(lMode));
    }

    return hr;
}


void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[512];  // Large buffer for very long filenames (like with HTTP)

    va_list pArgs;
    va_start(pArgs, szFormat);
    _vstprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    // This sample uses a simple message box to convey warning and error
    // messages.   You may want to display a debug string or suppress messages
    // altogether, depending on your application.
    MessageBox(NULL, szBuffer, TEXT("PlayCutscene Error"), MB_OK);
}



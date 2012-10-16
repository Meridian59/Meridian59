//------------------------------------------------------------------------------
// File: BGMusic.cpp
//
// Desc: A simple playback applicaiton that plays a cyclic set of media
//       files of the same type. This is the code required to use DirectShow
//       to play compressed audio in the background of your title in a
//       seamless manner.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <dshow.h>
#include <tchar.h>
#include <malloc.h>
#include "resource.h"

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------
HRESULT GraphInit(void);
HWND AppInit(HINSTANCE hInstance);
void AppMessageLoop(void);
void AppCleanUp(void);
HRESULT SwapSourceFilter(void); 
void ShowCurrentFile(HWND hWnd);
const TCHAR* DXUtil_GetDXSDKMediaPath();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
VOID CALLBACK MyTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define CLASSNAME       TEXT("BGMusicPlayer")
#define APPNAME         TEXT("BGMusic Player")
#define APPWIDTH        200
#define APPHEIGHT       100
#define MEDIA_TIMEOUT   (10 * 1000)  // 10 seconds, represented in milliseconds

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
// DirectShow Graph, Filter & Pins used
IGraphBuilder *g_pGraphBuilder = NULL;
IMediaControl *g_pMediaControl = NULL;
IMediaSeeking *g_pMediaSeeking = NULL;
IBaseFilter   *g_pSourceCurrent = NULL;
IBaseFilter   *g_pSourceNext = NULL;
TCHAR          g_szCurrentFile[128];
HWND           g_hwndApp;

// File names & variables to track current file
LPCTSTR pstrFiles[] = 
{
    TEXT("track1.mp3\0"),
    TEXT("track2.mp3\0"),
    TEXT("track3.mp3\0"),
};

int g_iNumFiles = 3, g_iNextFile = 0;


//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Main Entry point for the app. Calls the Initialization routines and
//       then calls the main processing loop.
//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    // Initialize application window
    if (! AppInit(hInstance))
        return 0;

    // Initialize DirectShow components and build initial graph
    if (SUCCEEDED (GraphInit()))
    {
        // Main Message Loop
        AppMessageLoop();
    }

    // Clean up
    AppCleanUp();
    return 0;
}


//------------------------------------------------------------------------------
// Name: GraphInit()
// Desc: Initialization of DirectShow components and initial graph
//------------------------------------------------------------------------------
HRESULT GraphInit(void)
{
    HRESULT hr;
    // Initialize COM
    if (FAILED (hr = CoInitialize(NULL)) )
        return hr;

    // Create DirectShow Graph
    if (FAILED (hr = CoCreateInstance(CLSID_FilterGraph, NULL,
                                      CLSCTX_INPROC, IID_IGraphBuilder,
                                      reinterpret_cast<void **>(&g_pGraphBuilder))) )
        return hr;

    // Get the IMediaControl Interface
    if (FAILED (g_pGraphBuilder->QueryInterface(IID_IMediaControl,
                                 reinterpret_cast<void **>(&g_pMediaControl))))
        return hr;

    // Get the IMediaControl Interface
    if (FAILED (g_pGraphBuilder->QueryInterface(IID_IMediaSeeking,
                                 reinterpret_cast<void **>(&g_pMediaSeeking))))
        return hr;

    // Create Source Filter for first file
    g_iNextFile = 0;

    // Create the intial graph
    if (FAILED (SwapSourceFilter()))
        return hr;

    // Set a timer for switching the sources
    SetTimer(g_hwndApp, 0, MEDIA_TIMEOUT, (TIMERPROC) MyTimerProc);

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: AppInit()
// Desc: Initialization of application window
//------------------------------------------------------------------------------
HWND AppInit(HINSTANCE hInstance)
{
    WNDCLASS wc;

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BGMUSIC));
    if(!RegisterClass(&wc))
        return 0;

    // Create the main window without support for resizing
    g_hwndApp = CreateWindow(CLASSNAME, APPNAME,
                    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
                                  | WS_MINIMIZEBOX | WS_VISIBLE,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    APPWIDTH, APPHEIGHT,
                    0, 0, hInstance, 0);

    return g_hwndApp;
}


//------------------------------------------------------------------------------
// Name: WndProcLoop()
// Desc: Main Message Processor for the Application
//------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_PAINT:
            ShowCurrentFile(hWnd);
            break;

        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


//------------------------------------------------------------------------------
// Name: ShowCurrentFile()
// Desc: Display the currently playing media file in the main window
//------------------------------------------------------------------------------
void ShowCurrentFile(HWND hWnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    TCHAR szMsg[128];

    BeginPaint(hWnd, &ps);
    HDC hdc = GetDC(hWnd);
    GetWindowRect(hWnd, &rc);
    
    // Set the text color to bright green against black background
    SetTextColor(hdc, RGB(80, 255, 80));
    SetBkColor(hdc, RGB(0,0,0));

    // Decide where to place the text (centered in window)
    int X = (rc.right - rc.left) / 2;
    int Y = (rc.bottom - rc.top) / 3;
    SetTextAlign(hdc, TA_CENTER | VTA_CENTER);

    // Update the text string
    wsprintf(szMsg, _T("Playing %s\0"), g_szCurrentFile);
    ExtTextOut(hdc, X, Y, ETO_OPAQUE, NULL, szMsg, _tcslen(szMsg), 0);

    EndPaint(hWnd, &ps);
}


//------------------------------------------------------------------------------
// Name: AppMessageLoop()
// Desc: Main Message Loop for the Application
//------------------------------------------------------------------------------
void AppMessageLoop(void)
{
    MSG msg={0};

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (! TranslateAccelerator(msg.hwnd, NULL, &msg) )
        {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }

    return;
}


//------------------------------------------------------------------------------
// Name: AppCleanUp)
// Desc: Clean up the application
//------------------------------------------------------------------------------
void AppCleanUp(void)
{
    // Stop playback
    if (g_pMediaControl)
        g_pMediaControl->Stop();

    // Release all remaining pointers
    SAFE_RELEASE( g_pSourceNext);
    SAFE_RELEASE( g_pSourceCurrent);
    SAFE_RELEASE( g_pMediaSeeking);
    SAFE_RELEASE( g_pMediaControl);
    SAFE_RELEASE( g_pGraphBuilder);

    // Clean up COM
    CoUninitialize();
    return;
}


//------------------------------------------------------------------------------
// MyTimerProc - Callback when the timer goes off
//------------------------------------------------------------------------------
VOID CALLBACK MyTimerProc(
  HWND hwnd,     // handle to window
  UINT uMsg,     // WM_TIMER message
  UINT idEvent,  // timer identifier
  DWORD dwTime   // current system time
)
{
    SwapSourceFilter();

    // Update the "current file" text message
    RECT rc;
    GetWindowRect(hwnd, &rc);
    InvalidateRect(hwnd, &rc, TRUE);
}


//------------------------------------------------------------------------------
// Name: SwapSourceFilter()
// Desc: This routine is used to change the source file in the current graph.
//       First the graph is stopped, then the current source filter is removed.
//       The new source filter is added, the output pin on this filter is
//       rendered, and playback is restarted.
//
//       When this routine is called during initialization, there is no
//       currently running graph. In that case, Stop becomes a no-op. The source
//       filter is added to an empty graph. Then during the render call, all
//       necessary filters required to play this source are added to the graph.
//
//       On subsequent calls, Stopping the graph allows filters to be removed.
//       When the old source filter is removed, all other filters are still
//       left in the graph. The new source filter is added, and then the render
//       operation reconnects the graph. Since all of the necessary filters for
//       playback are already in the graph (if the two files have the same file
//       type), these filters are reused. Existing filters in the graph are
//       always used first, if possible, during a Render operation. This avoids
//       having to create new filter instances with each change.
//------------------------------------------------------------------------------
HRESULT SwapSourceFilter(void)
{
    HRESULT hr = S_OK;
    IPin *pPin = NULL;
    TCHAR szFilename[MAX_PATH];
    WCHAR wFileName[MAX_PATH];

    // Determine the file to load based on DirectX Media path (from SDK)
    _tcscpy( szFilename, DXUtil_GetDXSDKMediaPath() );
    _tcscat( szFilename, pstrFiles[g_iNextFile % g_iNumFiles]);
    _tcscpy( g_szCurrentFile, pstrFiles[g_iNextFile % g_iNumFiles]);
    g_iNextFile++;

    // Make sure that this file exists
    DWORD dwAttr = GetFileAttributes(szFilename);
    if (dwAttr == (DWORD) -1)
        return ERROR_FILE_NOT_FOUND;

    #ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wFileName, MAX_PATH);
    #else
        lstrcpy(wFileName, szFilename);
    #endif

    // OPTIMIZATION OPPORTUNITY
	// This will open the file, which is expensive. To optimize, this
    // should be done earlier, ideally as soon as we knew this was the
    // next file to ensure that the file load doesn't add to the
    // filter swapping time & cause a hiccup.
    //
    // Add the new source filter to the graph. (Graph can still be running)
    hr = g_pGraphBuilder->AddSourceFilter(wFileName, wFileName, &g_pSourceNext);

    // Get the first output pin of the new source filter. Audio sources 
    // typically have only one output pin, so for most audio cases finding 
    // any output pin is sufficient.
    if (SUCCEEDED(hr)) {
        hr = g_pSourceNext->FindPin(L"Output", &pPin);  
    }

    // Stop the graph
    if (SUCCEEDED(hr)) {
        hr = g_pMediaControl->Stop();
    }

    // Break all connections on the filters. You can do this by adding 
    // and removing each filter in the graph
    if (SUCCEEDED(hr)) {
        IEnumFilters *pFilterEnum = NULL;

        if (SUCCEEDED(hr = g_pGraphBuilder->EnumFilters(&pFilterEnum))) {
            int iFiltCount = 0;
            int iPos = 0;

            // Need to know how many filters. If we add/remove filters during the
            // enumeration we'll invalidate the enumerator
            while (S_OK == pFilterEnum->Skip(1)) {
                iFiltCount++;
            }

            // Allocate space, then pull out all of the 
            IBaseFilter **ppFilters = reinterpret_cast<IBaseFilter **>
                                      (_alloca(sizeof(IBaseFilter *) * iFiltCount));
            pFilterEnum->Reset();

            while (S_OK == pFilterEnum->Next(1, &(ppFilters[iPos++]), NULL));
            SAFE_RELEASE(pFilterEnum);

            for (iPos = 0; iPos < iFiltCount; iPos++) {
                g_pGraphBuilder->RemoveFilter(ppFilters[iPos]);
				// Put the filter back, unless it is the old source
				if (ppFilters[iPos] != g_pSourceCurrent) {
					g_pGraphBuilder->AddFilter(ppFilters[iPos], NULL);
                }
				SAFE_RELEASE(ppFilters[iPos]);
            }
        }
    }

    // We have the new ouput pin. Render it
    if (SUCCEEDED(hr)) {
        hr = g_pGraphBuilder->Render(pPin);
        g_pSourceCurrent = g_pSourceNext;
        g_pSourceNext = NULL;
    }

    SAFE_RELEASE(pPin);
    SAFE_RELEASE(g_pSourceNext); // In case of errors

    // Re-seek the graph to the beginning
    if (SUCCEEDED(hr)) {
        LONGLONG llPos = 0;
        hr = g_pMediaSeeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,
                                           &llPos, AM_SEEKING_NoPositioning);
    } 

    // Start the graph
    if (SUCCEEDED(hr)) {
        hr = g_pMediaControl->Run();
    }

    // Release the old source filter.
    SAFE_RELEASE(g_pSourceCurrent)
    return S_OK;   
}


//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath()
// Desc: Returns the DirectX SDK media path
//-----------------------------------------------------------------------------
const TCHAR* DXUtil_GetDXSDKMediaPath()
{
    static TCHAR strNull[2] = _T("");
    static TCHAR strPath[MAX_PATH];
    DWORD dwType;
    DWORD dwSize = MAX_PATH;
    HKEY  hKey;

    // Open the appropriate registry key
    LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\DirectX SDK"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != lResult )
        return strNull;

    lResult = RegQueryValueEx( hKey, _T("DX81SDK Samples Path"), NULL,
                              &dwType, (BYTE*)strPath, &dwSize );
    RegCloseKey( hKey );

    if( ERROR_SUCCESS != lResult )
        return strNull;

    _tcscat( strPath, _T("\\Media\\") );

    return strPath;
}


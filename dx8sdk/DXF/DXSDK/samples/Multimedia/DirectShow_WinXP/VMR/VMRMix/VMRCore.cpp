//------------------------------------------------------------------------------
// File: VMRCore.h
//
// Desc: DirectShow sample code
//       Header file and class description for CVMRCore,
//       "main" module to manage VMR and its interfaces
//       This class is called from CDemonstration;
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <objbase.h>
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD WINAPI WndlessControlThread(LPVOID lpvThreadParam);

CVMRCore::CVMRCore(CVMRMixDlg * pDlg,   // pointer to the 'parent' dialog
                   CMediaList * pML)  : // media play-list
m_tID(NULL),
m_hEventResumeCore(NULL),
m_hEventStopTest(NULL),
m_hEventCloseWindow(NULL),
m_hEventKillCore(NULL),
m_hWinThread(NULL),
m_pDlg(pDlg),
m_pMixerBitmap(NULL),
m_pML(pML),
m_pIMediaSeeking(NULL),
m_pIMixerControl(NULL),
m_nConnectedPins(0),
m_pIMonConfig(NULL),
m_hwnd(NULL),
m_pIMC(NULL),
m_pIVidWindow(NULL),
m_pIWndless(NULL),
m_dwVMRMode(NULL),
m_dwVMRPrefs(NULL),
m_pConfig(NULL),
m_pGraph(NULL),
m_pTestFilter(NULL),
m_lpDestRect(NULL),
m_lpSrcRect(NULL)
{
    // by default, set windowless mode and allow overlays
    m_dwVMRMode = VMRMode_Windowless;
    m_dwVMRPrefs =  RenderPrefs_AllowOverlays;
    m_dwID = (DWORD) rand();

    sprintf( m_szEventStopTest, "STOP_TEST_%ld", m_dwID);
    sprintf( m_szEventCloseWindow, "CLOSE_WINDOW_%ld", m_dwID);
    sprintf( m_szEventKillCore, "KILL_CORE_%ld", m_dwID);
    sprintf( m_szEventResumeCore, "RESUME_CORE_%ld", m_dwID);

    m_hEventResumeCore  = CreateEvent(NULL, FALSE, FALSE, m_szEventResumeCore);
    m_hEventCloseWindow = CreateEvent(NULL, FALSE, FALSE, m_szEventCloseWindow);
    m_hEventKillCore = CreateEvent(NULL, FALSE, FALSE, m_szEventKillCore);
    m_hEventStopTest = CreateEvent(NULL, FALSE, FALSE, m_szEventStopTest);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

//------------------------------------------------------------------------------
CVMRCore::CVMRCore( CVMRMixDlg * pDlg,      // 'parent dialog
                    DWORD dwVMRMode,        // rendering mode
                    DWORD dwVMRPrefs,       // rendering preferences
                    CMediaList * pML)   :   // play-list
m_tID(NULL),
m_hEventStopTest(NULL),
m_hEventCloseWindow(NULL),
m_hEventKillCore(NULL),
m_pDlg(pDlg),
m_pMixerBitmap(NULL),
m_pML( pML),
m_pIMediaSeeking(NULL),
m_pIMixerControl(NULL),
m_nConnectedPins(0),
m_hEventResumeCore(NULL),
m_hWinThread(NULL),
m_pIMonConfig(NULL),
m_hwnd(NULL),
m_pIMC(NULL),
m_pIVidWindow(NULL),
m_pIWndless(NULL),
m_dwVMRMode(NULL),
m_dwVMRPrefs(NULL),
m_pConfig(NULL),
m_pGraph(NULL),
m_pTestFilter(NULL),
m_lpDestRect(NULL),
m_lpSrcRect(NULL)
{

    (dwVMRPrefs) ? (m_dwVMRPrefs = dwVMRPrefs)
        :(m_dwVMRPrefs = RenderPrefs_AllowOverlays);

    (dwVMRMode) ? (m_dwVMRMode = dwVMRMode)
        : (m_dwVMRMode = VMRMode_Windowless);

    m_dwID = (DWORD) rand();

    sprintf( m_szEventStopTest, "STOP_TEST_%ld", m_dwID);
    sprintf( m_szEventCloseWindow, "CLOSE_WINDOW_%ld", m_dwID);
    sprintf( m_szEventKillCore, "KILL_CORE_%ld", m_dwID);
    sprintf( m_szEventResumeCore, "RESUME_CORE_%ld", m_dwID);

    m_hEventResumeCore = CreateEvent(NULL, FALSE, FALSE, m_szEventResumeCore);
    m_hEventCloseWindow = CreateEvent(NULL, FALSE, FALSE, m_szEventCloseWindow);
    m_hEventKillCore = CreateEvent(NULL, FALSE, FALSE, m_szEventKillCore);
    m_hEventStopTest = CreateEvent(NULL, FALSE, FALSE, m_szEventStopTest);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

//------------------------------------------------------------------------------
CVMRCore::~CVMRCore()
{
    ReleaseInterfaces();

    CloseHandle(m_hEventResumeCore);
    CloseHandle(m_hEventStopTest);
    CloseHandle(m_hEventKillCore);
    CloseHandle(m_hWinThread);

    CoUninitialize();
}

//------------------------------------------------------------------------------
// CreateGraph
// Desc:    Creates a valid filter graph for VMR
// Returns: S_OK if everything succeeds, else the hresult
//          returned by the API called that failed
//------------------------------------------------------------------------------
HRESULT CVMRCore::CreateGraph()
{
    // keeps track of any nonessential initializations
    HRESULT hrInit = S_OK;
    int     ncount; // counter for media files from the play list

    // graph has already been successfully created
    if (m_pGraph)
    {
        return S_FALSE;
    }

    HRESULT hr;

    // get the graph interface
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
        reinterpret_cast<void**>(&m_pGraph));
    if (FAILED(hr))
    {
        return hr;
    }

    // add VMR (called 'Default Video Renderer' in WindowsXP)
    hr = AddTestFilter();
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't add the Test Filter to the Graph\n");
        goto cleanup;
    }

    // convert filename from module to unicode and render
    if( m_pML->Size() < 1)
    {
        OutputDebugString("There is nothing to render!\n");
        goto cleanup;
    }

    // Render the files
    for( ncount =0; ncount < m_pML->Size(); ncount++)
    {
        hr = m_pGraph->RenderFile(m_pML->GetFileNameW(ncount), NULL);
        if (FAILED(hr))
        {
            char szMsg[MAX_PATH];
            sprintf( szMsg, "Error while rendering file, method returned %s\n",
                hresultNameLookup(hr) );
            OutputDebugString( szMsg);
            continue;
        }
        m_pML->GetItem(ncount)->m_bInUse = true;
        m_nConnectedPins++;
    }
    if( FAILED(hr))
    {
        goto cleanup;
    }

    // verify that there is only one renderer in the graph
    if (!ListFilters())
    {
        OutputDebugString("There is more than one renderer in the graph.\n");
        hr = E_FAIL;
        goto cleanup;
    }

    // get the media control so we can play
    hr = m_pGraph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&m_pIMC));
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't get IMediaControl interface!\n");
        goto cleanup;
    }

    // now that file is rendered, initialize the appropriate interfaces
    hr = InitRelevantInterfaces();
    if (FAILED(hr))
    {
        OutputDebugString("Not all interfaces were initialized.\n");
        hrInit = S_FALSE;
    }

    // create the window for the windowless control if needed
    if (m_dwVMRMode & VMRMode_Windowless)
    {
        // if windowless mode is desired, the window needs to be setup
        CreateWindowlessWindow();

        // only applicable for windoless control
        hr = m_pIWndless->SetVideoClippingWindow(m_hwnd);
        if (FAILED(hr))
        {
            OutputDebugString("Error while setting the Video Clipping Window\n");
            goto cleanup;
        }
    } // if (m_dwVMRMode & VMRMode_Windowless)

    return hrInit;

cleanup:
    // if we fail, release interfaces
    SAFERELEASE(m_pGraph);
    SAFERELEASE(m_pTestFilter);
    return hr;
}

//------------------------------------------------------------------------------
//  AddTestFilter
//  Desc:   Adds the VMR to the graph
//  Returns: S_OK if the filter is added, otherwise the corresponding
//           DShow COM error
//------------------------------------------------------------------------------
HRESULT CVMRCore::AddTestFilter()
{
    HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
        reinterpret_cast<void**>(&m_pTestFilter));
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't load VideoMixingRenderer Filter\n");
        return hr;
    }

    // now add this filter to the graph
    // add the VMR to the filter graph
    hr = m_pGraph->AddFilter(m_pTestFilter, L"Default Video Renderer");
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't add Video Mixing Renderer Filter to the graph\n");
        SAFERELEASE(m_pTestFilter);
        return hr;
    }

    // make dynamic reconnection possible
    IGraphConfig * pIGraphConfig = NULL;
    hr = m_pGraph->QueryInterface( __uuidof(IGraphConfig), (LPVOID *) &pIGraphConfig);
    if( SUCCEEDED(hr) )
    {
        hr = pIGraphConfig->SetFilterFlags(m_pTestFilter, AM_FILTER_FLAGS_REMOVABLE);
    }
    SAFERELEASE( pIGraphConfig );


    // the VMR requires certain setup procedures:
    // first get the pin config interface (that is IVMRFilterConfig and
    // set up the initialization settings for the VMR
    hr = m_pTestFilter->QueryInterface(__uuidof(IVMRFilterConfig), reinterpret_cast<void**>(&m_pConfig));
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't get IVMRFilterConfig interface\n");
        return hr;
    }

    // We MUST set the number of streams first
    // it sets VMR to mixing or non-mixing mode.
    // By specifying SetNumberOfStreams(1) we guarantee that the came copy
    // of the rendering filter will be used when rendering several sources
    int nSources;
    nSources = (m_pML->Size() >1) ? 16 : 1;

    hr = m_pConfig->SetNumberOfStreams(nSources);
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't Set the number of streams.\n");
        return hr;
    }

    // now set the rendering mode
    hr = m_pConfig->SetRenderingMode(m_dwVMRMode);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to set the rendering mode\n");
        return hr;
    }

    // now set the preferences
    hr = m_pConfig->SetRenderingPrefs(m_dwVMRPrefs);
    if (FAILED(hr))
    {
        char szMsg[MAX_PATH];
        sprintf( szMsg, "hr = %s\n", hresultNameLookup(hr));
        OutputDebugString( szMsg);
        OutputDebugString("Error while setting rendering preferences!\n");
        OutputDebugString("Attempting to continue rendering file\n");
    }
    return S_OK;
}

//------------------------------------------------------------------------------
// InitRelevantInterfaces
// Desc:    QIs for all the interfaces that are appropriate
// Returns: S_OK if the all of the relevant interfaces are gotten
//              correctly
// Notes:       This method also verifies which mode the VMR is in
//              and if it needs a window created that will be
//              by the windowless control
//------------------------------------------------------------------------------
HRESULT CVMRCore::InitRelevantInterfaces()
{
    HRESULT hr = E_FAIL;
    HRESULT hrComplete = S_OK;

    if (m_dwVMRMode & VMRMode_Windowless)
    {
        hr = m_pTestFilter->QueryInterface(__uuidof(IVMRWindowlessControl),
            reinterpret_cast<void**>(&m_pIWndless));
        if (FAILED(hr))
        {
            OutputDebugString("Couldn't get IVMRWindowlessControl interface\n");
            hrComplete = hr;
        }
        // now that the interface is ready, place the video on the screen
        RECT rect;
        if( !m_hwnd )
        {
            CreateWindowlessWindow();
        }
        ASSERT( m_hwnd);
        GetClientRect(m_hwnd, &rect);
        hr = m_pIWndless->SetVideoPosition(NULL, &rect);
    } // if mode is windowless

    hr = m_pTestFilter->QueryInterface(__uuidof(IVMRMonitorConfig),
        reinterpret_cast<void**>(&m_pIMonConfig));
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't get IVMRMonitorConfig interface\n");
        return hr;
    }

    hr = m_pTestFilter->QueryInterface( __uuidof(IVMRMixerControl), (LPVOID *) &m_pIMixerControl ) ;
    if( FAILED(hr))
    {
        OutputDebugString("Cannot find IVMRMixerControl interface\n");
        return hr;
    }
    hr = m_pTestFilter->QueryInterface( __uuidof(IVMRMixerBitmap), (LPVOID *) &m_pMixerBitmap ) ;
    if( FAILED(hr))
    {
        OutputDebugString("Cannot find IVMRMixerBitmap interface\n");
        return hr;
    }

    hr = GetIGraphBuilder()->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<void**>(&m_pIMediaSeeking));
    if( FAILED(hr))
    {
        OutputDebugString("Cannot find IMediaSeeking interface\n");
        return hr;
    }

    hr = m_pGraph->QueryInterface(IID_IVideoWindow,
        reinterpret_cast<void**>(&m_pIVidWindow));
    if (FAILED(hr))
    {
        OutputDebugString("Couldn't get IVideoWindow interface\n");
        hrComplete = hr;
    }
    return hrComplete;
}

//------------------------------------------------------------------------------
//  Function:   Play
//  Desc:       Begins to play the core video, checks for window initiziation
//  Parameter:  bool bDoNotRunYet, if true, then do not run IMediaControl
//  Returns:    HRESULT returned by the the MediaControlers call to run
//  Note:       We catch all unhandled exceptions here
//------------------------------------------------------------------------------
HRESULT CVMRCore::Play(bool bDoNotRunYet /* = false */)
{
    try
    {
        HRESULT hr = S_OK;

        // if there is no media control interface, get it created
        // return successfully with the graph running or the error
        if (!m_pIMC)
            hr = CreateGraph();
        if( bDoNotRunYet )
        {
            return hr;
        }
        hr = (SUCCEEDED(hr)) ? m_pIMC->Run() : hr;
        return hr;
    } // try
    catch (HRESULT hr)
    {
        if (hr == E_FAIL)
        {
            OutputDebugString("Unhandled exception in CVMRCore::Play\n");
            SetAbort();
            return E_FAIL;
        }
    }
    catch(...)
    {
        SetAbort();
        return E_FAIL;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
//  Function:   Pause
//  Purpose:    Begins the core video paused, checks for window initiziation
//  Returns:    HRESULT returned by the the MediaControlers call to run
//  Note:       We catch all unhandled exceptions here
//------------------------------------------------------------------------------
HRESULT CVMRCore::Pause()
{
    try
    {
        HRESULT hr = S_OK;

        // if there is no media control interface, get it created
        // return successfully with the graph running or the error
        if (!m_pIMC)
            hr = CreateGraph();
        return (SUCCEEDED(hr)) ? m_pIMC->Pause() : hr;
    } // try
    catch (HRESULT hr)
    {
        if (hr == E_FAIL)
        {
            OutputDebugString("Failed to pause the graph.\n");
            SetAbort();
            return E_FAIL;
        }
    }
    catch(...)
    {
        SetAbort();
        return E_FAIL;
    }
    return S_OK;
}


//------------------------------------------------------------------------------
//  Function:   Stop
//  Purpose:    Begins to the core video stopped, checks for window initiziation
//  Returns:    HRESULT returned by the the MediaControlers call to run
//  Note:       We catch all unhandled exceptions here
//------------------------------------------------------------------------------
HRESULT CVMRCore::Stop()
{
    try
    {
        HRESULT hr = S_OK;

        // if there is no media control interface, get it created
        // return successfully with the graph running or the error
        if (!m_pIMC)
            hr = CreateGraph();
        return (SUCCEEDED(hr)) ? m_pIMC->Stop() : hr;
    } // try
    catch (HRESULT hr)
    {
        if (hr == E_FAIL)
        {
            OutputDebugString("Failed to stop the graph.\n");
            SetAbort();
            return E_FAIL;
        }
    }
    catch(...)
    {
        SetAbort();
        return E_FAIL;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
// ReleaseInterfaces
// Desc:    Releases the IUnknown interfaces for all member variable
//          interfaces created by this object
//------------------------------------------------------------------------------
void CVMRCore::ReleaseInterfaces()
{
    // close the window if it exists
    KillWindow();
    SAFERELEASE(m_pMixerBitmap);
    SAFERELEASE(m_pIMediaSeeking);
    SAFERELEASE(m_pIMixerControl);
    SAFERELEASE(m_pIMonConfig);
    SAFERELEASE(m_pIWndless);
    SAFERELEASE(m_pConfig);
    SAFERELEASE(m_pIMC);
    SAFERELEASE(m_pTestFilter);
    SAFERELEASE(m_pGraph);
    SAFERELEASE(m_pIVidWindow);
}

//------------------------------------------------------------------------------
// IsActive
// Desc: checks if VMRCore is active
// return: false if m_hEventKillCore is set (user closes the window);
//         true otherwise
//------------------------------------------------------------------------------
bool CVMRCore::IsActive()
{
    DWORD dwRes = NULL;
    dwRes = WaitForSingleObject( m_hEventKillCore, 10);

    if( WAIT_OBJECT_0 != dwRes)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
// ListFilters
// Desc:    Logs to the debugger output the filters in our created filter graph
// Returns: true if one VMR filter was found; false otherwise
//------------------------------------------------------------------------------
bool CVMRCore::ListFilters()
{
    if (m_pGraph)
    {
        IEnumFilters    * ppEnum;
        ULONG cFilters, cFetched;
        IBaseFilter     *ppFilter[MAXFILTERS];
        FILTER_INFO     pInfo;
        HRESULT         hr;
        char            sLogBuffer[MAX_PATH];

        int iRendererCount = 0;

        char buffer[MAX_PATH];

        // get pointer to list of filters in graph
        hr = m_pGraph->EnumFilters(&ppEnum);
        cFilters = MAXFILTERS;  // number of filters to retrieve
        hr = ppEnum->Next(cFilters, &(ppFilter[0]), &cFetched);

        sprintf(buffer, "Filter List (%d found): \n", cFetched);
        OutputDebugString(buffer);
        for (UINT i = 0; i < cFetched; i++)
        {
            //get, covert and display filter name
            ppFilter[i]->QueryFilterInfo(&pInfo);

            WideCharToMultiByte(CP_ACP, 0, pInfo.achName, -1, buffer, MAX_PATH, NULL, NULL);

            // keep count of renderers to throw an exception when there is more than
            // one renderer
            if (IsRenderer(buffer))
                iRendererCount++;

            if( 2 == iRendererCount )
            {
                // with some cards, a glitch can occur and there will be several
                // VMR filters in the same graph. Here, we try to prevent it
                // (okay, if you do not like the line below, go trough the graph
                //  and reconnect all the spources to one VMR)
                hr = m_pGraph->RemoveFilter(ppFilter[i]);
                if(FAILED(hr))
                {
                    iRendererCount--;
                }
            }// if

            sprintf(sLogBuffer, "(%d) = %s\n", i+1 ,  buffer);
            OutputDebugString(sLogBuffer);

            // release any IUnknowns that were addrefed by successful calls of Enum and Query
            SAFERELEASE(pInfo.pGraph);
            SAFERELEASE(ppFilter[i]);
        } // for
        SAFERELEASE(ppEnum);

        // if more than one renderer is present, the wrong renderer may get tested
        if (iRendererCount > 1)
            return false;

        return true;
    } // if m_pGraph
    return false;
} // ListFilters

//------------------------------------------------------------------------------
// CreateWindowlessWindow
// Desc:    Creates the events and thread that will handle the windowless control's
//          playback window
// Returns: S_OK when the thread is completed
//------------------------------------------------------------------------------
HRESULT CVMRCore::CreateWindowlessWindow()
{
    if( NULL != m_hwnd)
    {
        return E_INVALIDARG;
    }

    // spin off and create the playback window on another thread so that
    // it will have its own message pump so that any messages that the
    // VMR is supposed to be handled can be set
    m_hWinThread = CreateThread(NULL, NULL, WndlessControlThread, this, NULL, &m_tID);
    ::WaitForSingleObject(m_hEventResumeCore, INFINITE);

    return S_OK;
}

//------------------------------------------------------------------------------
// WndlessControlThread
// Desc:        Thread code that creates a playback window for the VMR
// Arguments:   Pointer to thread paramters in this case a structure holding the hwnd and
//              a pointer to the core object that is calling it.
// Returns:     S_OK when the thread is completed
// Remarks:     Requires a call to the core's KillThread so that
//              this thread will end!
//------------------------------------------------------------------------------
DWORD WINAPI WndlessControlThread(LPVOID lpvThreadParam)
{
    // passed a to the thread
    CVMRCore * pCore = static_cast<CVMRCore *>(lpvThreadParam);
    HWND hwnd;

    if( !pCore )
    {
        return NULL;
    }
    // this code will create the window and run the windows proc for each test
    // each test will be on its own thread
    // register playback window
    WNDCLASSEX wndclass = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW * CS_VREDRAW,
        CVMRCore::WndProc,
        0,
        0,
        AfxGetInstanceHandle(),
        LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)),
        LoadCursor(NULL, IDC_ARROW),
        HBRUSH(COLOR_WINDOW+1),
        NULL,
        TEXT("EventWindow"),
        NULL
    };

    RegisterClassEx(&wndclass);

    // create the playback window
    // it is always in dlg client coordinates
    RECT rPlayback;

    int cx = GetSystemMetrics(SM_CXFULLSCREEN);
    int cy = GetSystemMetrics(SM_CYFULLSCREEN);

    if( !pCore->GetDlg()->IsFullScreen())
    {
        cx /=4;
        cy /=4;
    }
    rPlayback.left = 0;
    rPlayback.top = 0;
    rPlayback.right = cx;
    rPlayback.bottom = cy;

    // need to update member variable of class with this
    hwnd = CreateWindow(
        TEXT("EventWindow"),
        TEXT("VMRMix Playback Window"),
        WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
        rPlayback.left,
        rPlayback.top,
        rPlayback.right - rPlayback.left,
        rPlayback.bottom - rPlayback.top,
        NULL,
        NULL,
        AfxGetInstanceHandle(),
        pCore); // we send the this pointer here so we can retrieve it from WM_CREATE later

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCore));

    // for the windowless controls clipping window
    pCore->SetHwnd(hwnd);
    ShowWindow(hwnd , SW_SHOWNORMAL);
    UpdateWindow(hwnd ) ;

    // set an event here so that our other thread can continue now that the window is created
    HANDLE hCoreEvent = ::OpenEvent(EVENT_ALL_ACCESS, NULL, pCore->m_szEventResumeCore);
    SetEvent(hCoreEvent);
    CloseHandle(hCoreEvent);

    // now run a message loop so that this thread will not die
    MSG msg;
    HANDLE hTestOverEvent = ::OpenEvent(EVENT_ALL_ACCESS, NULL, pCore->m_szEventStopTest);
    HANDLE hWindowClose = ::OpenEvent(EVENT_ALL_ACCESS, NULL, pCore->m_szEventCloseWindow);
    while (WAIT_OBJECT_0 != WaitForSingleObject(hTestOverEvent, 1000))
    {
        OutputDebugString("WndlessControlThread:: Inside while Wait(hTestOverEvent)\n");
        while (::PeekMessage(&msg, NULL, 0, 0, 0))
        {
            OutputDebugString("WndlessControlThread:: Inside while (::PeekMessage\n");
            MSG msgCur;
            if (!::GetMessage(&msgCur, NULL, NULL, NULL))  // the quit message came
            {
                ::PostQuitMessage(0);
                break;
            }
            ::TranslateMessage(&msgCur);
            ::DispatchMessage(&msgCur);
        } ///while peeke
    }// while wait

    // trigger remaining events and close all locally opened handles
    SetEvent(hWindowClose);
    CloseHandle(hWindowClose);
    CloseHandle(hTestOverEvent);
    return S_OK;
}

//------------------------------------------------------------------------------
// WndProc
// Desc:    Window procedure for multithreaded implementation
// Returns: LRESULT
//------------------------------------------------------------------------------
LRESULT CALLBACK CVMRCore::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    // retrieve the pointer to the instance of CVMRCore that this window belongs to
    // because WndProc is a static method, it doesn't have a this pointer
    CVMRCore * pCore = NULL;
    BOOL bRetVal = false;

    pCore = reinterpret_cast<CVMRCore *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if( !pCore )
    {
        return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }

    switch (uMessage)
    {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            return pCore->OnSize(wParam, lParam);

        case WM_PAINT:
            return pCore->OnPaint(hWnd);

        case WM_CLOSE:
            OutputDebugString("message WM_CLOSE obtained by CVMRCore::WndProc\n");
            return pCore->KillWindow();

        case WM_DESTROY:
            PostQuitMessage(-1);
            return (0);
    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

//------------------------------------------------------------------------------
//  KillWindow
//  Desc:   Sets the event when multithreaded that the playback thread may close the
//          playback window
//------------------------------------------------------------------------------
LRESULT CVMRCore::KillWindow()
{
    if( m_pIMC)
    {
        m_pIMC->Stop();
    }
    if( !SetEvent( m_hEventStopTest ))
    {
        DWORD dwRes = GetLastError();
        OutputDebugString("KillWindow::Failed to set event m_hEventStopTest\n");
    }
    else
    {
        OutputDebugString("KillWindow::Event m_hEventStopTest is set\n");
    }
    if( this->m_hWinThread )
    {
        if( !PostThreadMessage(m_tID, WM_QUIT, NULL, NULL) )
        {
            DWORD dwRes = GetLastError();
        }

    }
    SetEvent( m_hEventKillCore );
    OutputDebugString("KillWindow::m_hEventKillCore is set ...\n");
    return 0;
}


//------------------------------------------------------------------------------
// Function:    OnSize
// Purpose:     Resets the video in the newly sized window
// Arguments:   lParam - height and width of the window
//              wparam - type of resizing requested
// Returns: Nothing
//------------------------------------------------------------------------------
LRESULT CVMRCore::OnSize(LPARAM lParam, WPARAM wParam)
{
    SetClientVideo();
    return 0;
}

//------------------------------------------------------------------------------
//  Function:   SetClientVideo
//  Purpose:    Sets the video position to the current client area size of the playback
//              window if the m_lpSrcRect and m_lpDestRect have been given valid values.
//  Arguments:  None
//  Returns:    True if the rectangles have been initialized, false if the client
//              window's default size is used
//  Notes:      One might wish to post a WM_PAINT message since this code will reset
//              any source and destination changes to the windowless control
//------------------------------------------------------------------------------
bool CVMRCore::SetClientVideo()
{
    if ( m_pIWndless)
    {
        // both of the rectangles have been set
        if (m_lpSrcRect && m_lpDestRect)
        {
            HRESULT hr = m_pIWndless->SetVideoPosition(m_lpSrcRect, m_lpDestRect);
            ASSERT(SUCCEEDED(hr));
            return true;
        }
    }
    RECT rect;
    // reset video window, this will change any previous settings
    GetClientRect(m_hwnd, &rect);
    ASSERT(m_pIWndless);

    if (m_pIWndless)
        HRESULT hr = m_pIWndless->SetVideoPosition(NULL, &rect);

    return false;
}

//------------------------------------------------------------------------------
//  Function:   SetAbort
//  Purpose:    emergency abort
//  Other:      Alerts the module to not allow any more test to be executed
//------------------------------------------------------------------------------
void CVMRCore::SetAbort()
{
    // an unhandled exception has been thrown
    ReleaseInterfaces();
}


//------------------------------------------------------------------------------
//  Function:   OnPaint
//  Purpose:    Paints the playback window's client area
//  Arguments:  hWnd of the client window
//------------------------------------------------------------------------------
LRESULT CVMRCore::OnPaint(HWND hWnd)
{
    if (NULL == m_pIWndless)
        return 0;

    RECT rect;
    GetClientRect(m_hwnd, &rect);

    PAINTSTRUCT ps;
    HDC hdc;

    hdc = BeginPaint(hWnd, &ps);
    HRESULT hr = m_pIWndless->RepaintVideo(hWnd, hdc);

    if(!SUCCEEDED(hr))
    {
        char szMsg[MAX_PATH];
        sprintf( szMsg, "%s", hresultNameLookup(hr));
    }

    EndPaint(hWnd, &ps);
    return 0;
}

//------------------------------------------------------------------------------
//  Function:   GetClientHwnd
//  Purpose:    Access to the private core's client's hwnd
//  Returns:    handle to the renderer's playback controlling window
//------------------------------------------------------------------------------
HWND CVMRCore::GetClientHwnd()
{
    // we may have already set this up in windowless control
    if (m_hwnd)
        return m_hwnd;

    HWND    hwnd;           // handle to the window we are looking for
    TCHAR   buffer[MAX_PATH];

    hwnd = GetTopWindow(NULL);
    while (hwnd)
    {
        // get title bar's window and compare
        if (GetWindowText(hwnd, buffer, MAX_PATH))
        {
            // old renderer, windowed mode of VMR
            if (!strcmp(buffer, "ActiveMovie Window"))
                break;          // title found save use hwnd
            if (!strcmp(buffer, "Windowless Control"))
                break;
        } //
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }  // while
    if (!hwnd)
    {
        OutputDebugString("Could not find window handle for playback window\n");
        return NULL;
    }
    m_hwnd = hwnd;
    return m_hwnd;
}


//------------------------------------------------------------------------------
//  Function:   IsRenderer
//  Purpose:    Defines if a given filter is a video renderer
//  Arguments:  strFilterName - NON wide string pointer that holds
//              the name of the filter being checked
//  Returns:    true if the filter name is a filter that is being sought, false otherwise
//  Other:      This function can be useful because sometimes with some videocards
//              several copies of VMR appear in the same graph.
//              Name 'Video renderer' corresponds to older version of venderer
//------------------------------------------------------------------------------
bool CVMRCore::IsRenderer(char *strFilterName)
{
    if (!strncmp(strFilterName, "Video Mixing", 12))
        return true;

    if (!strncmp(strFilterName, "Video Render", 12))
        return true;

    return false;
}


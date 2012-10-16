//------------------------------------------------------------------------------
// File: PlayCapMoniker.cpp
//
// Desc: DirectShow sample code - a very basic application using Capture
//       Devices.  It creates a window and uses the first available Capture
//       Device to render and preview video capture data.
//
//       Instead of building the capture graph manually using the
//       ICaptureGraphBuilder2 interface, this sample simply finds the moniker 
//       of the first available capture device, finds its display name, and 
//       uses RenderFile() to automatically build the graph.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <atlbase.h>
#include <windows.h>
#include <streams.h>
#include <stdio.h>
#include <comdef.h>

#include "PlayCapMoniker.h"

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#define REGISTER_FILTERGRAPH

//
// Global data
//
HWND ghApp=0;
DWORD g_dwGraphRegister=0;

IVideoWindow  * g_pVW = NULL;
IMediaControl * g_pMC = NULL;
IMediaEventEx * g_pME = NULL;
IGraphBuilder * g_pGraph = NULL;


HRESULT CaptureVideoByMoniker()
{
    HRESULT hr;
    IMoniker *pMoniker =NULL;
    USES_CONVERSION;

    // Get DirectShow interfaces
    hr = GetInterfaces();
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
        return hr;
    }

    // Use the system device enumerator and class enumerator to find
    // a moniker that representa a video capture/preview device, 
    // such as a desktop USB video camera.
    hr = FindCaptureDeviceMoniker(&pMoniker);
    if (FAILED(hr))
    {
        // FindCaptureDeviceMoniker will display an error message
        return hr;
    }

    // Get the display name of the moniker
    LPOLESTR strName=0;
    hr = pMoniker->GetDisplayName(NULL, NULL, &strName);
    pMoniker->Release();

    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't get moniker's display name!  hr=0x%x"), hr);
        return hr;
    }

    // We can call RenderFile on the moniker's name to build the graph.
    // This saves the trouble of building the capture graph manually.
    hr = g_pGraph->RenderFile(strName, NULL);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't render the capture graph!  hr=0x%x\r\n\r\n") 
            TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
            TEXT("to be installed and working properly.  The sample will now close."), hr);
        return hr;
    }            

#ifdef DEBUG
    // Get a human-readable string for evaluation during debugging
    TCHAR szMonikerName[256]={0};
    _tcscpy(szMonikerName, W2T(strName));
    //  Msg(TEXT("Moniker: %s\r\n"), szMonikerName);
#endif

    // Set the video window to be a child of the main window
    hr = g_pVW->put_Owner((OAHWND)ghApp);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't set video window owner!  hr=0x%x"), hr);
        return hr;
    }            

    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot(g_pGraph, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        g_dwGraphRegister = 0;
    }
#endif

    // Start previewing video data
    hr = g_pMC->Run();
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
        return hr;
    }

    return hr;   
}


HRESULT FindCaptureDeviceMoniker(IMoniker **ppMoniker)
{
    HRESULT hr;
    ULONG cFetched;
   
    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
        IID_ICreateDevEnum, (void ** ) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        MessageBox(ghApp,TEXT("No video capture device was detected.\r\n\r\n")
                   TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                   TEXT("to be installed and working properly.  The sample will now close."),
                   TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next (1, ppMoniker, &cFetched)))
    {
        return S_OK;
    }
    else
    {
        Msg(TEXT("Unable to access video capture device!"));   
        return E_FAIL;
    }

    return hr;
}


HRESULT GetInterfaces(void)
{
    HRESULT hr;

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
        IID_IGraphBuilder, (void **) &g_pGraph);
    if (FAILED(hr))
        return hr;

    // Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &g_pMC);
    if (FAILED(hr))
        return hr;

    hr = g_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *) &g_pVW);
    if (FAILED(hr))
        return hr;

    hr = g_pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *) &g_pME);
    if (FAILED(hr))
        return hr;

    // Set the window handle used to process graph events
    hr = g_pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0);

    return hr;
}


void CloseInterfaces(void)
{
    // Stop previewing data
    if (g_pMC)
        g_pMC->Stop();

    // Stop receiving events
    if (g_pME)
        g_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

    // Relinquish ownership (IMPORTANT!) of the video window.
    // Failing to call put_Owner can lead to assert failures within
    // the video renderer, as it still assumes that it has a valid
    // parent window.
    if(g_pVW)
    {
        g_pVW->put_Visible(OAFALSE);
        g_pVW->put_Owner(NULL);
    }

#ifdef REGISTER_FILTERGRAPH
    // Remove filter graph from the running object table   
    if (g_dwGraphRegister)
        RemoveGraphFromRot(g_dwGraphRegister);
#endif

    // Release DirectShow interfaces
    SAFE_RELEASE(g_pMC);
    SAFE_RELEASE(g_pME);
    SAFE_RELEASE(g_pVW);
    SAFE_RELEASE(g_pGraph);
}


#ifdef REGISTER_FILTERGRAPH

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}

void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

#endif


void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[512];

    va_list pArgs;
    va_start(pArgs, szFormat);
    _vstprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    MessageBox(NULL, szBuffer, TEXT("PlayCapMoniker Message"), MB_OK | MB_ICONERROR);
}


HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Process all queued events
    while(SUCCEEDED(g_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1,
                    (LONG_PTR *) &evParam2, 0)))
    {
        // Free event parameters to prevent memory leaks
        hr = g_pME->FreeEventParams(evCode, evParam1, evParam2);

        switch (evCode)
        {
            // When the user closes the capture window, close the app.
            case EC_COMPLETE:
            case EC_DEVICE_LOST:
            case EC_ERRORABORT:
            case EC_USERABORT:
                PostMessage(ghApp, WM_CLOSE, 0, 0);
                break;

            default:
                break;
        }
    }

    return hr;
}


LRESULT CALLBACK WndMainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_GRAPHNOTIFY:    // Process events from the filter graph
            HandleGraphEvent();
            break;

        case WM_CLOSE:            
            CloseInterfaces();  // Stop capturing and release interfaces
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc (hwnd , message, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg={0};
    WNDCLASS wc;
    HRESULT hr;

    // Initialize COM
    if(FAILED(CoInitialize(NULL)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));   
        exit(1);
    } 

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc   = WndMainProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDPREVIEW));
    if(!RegisterClass(&wc))
    {
        Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
        CoUninitialize();
        exit(1);
    }

    // Create the main window.  The WS_CLIPCHILDREN style is required.
    // Since we will render a moniker to begin capturing video, the capture 
    // graph will create its own window, so this main window will be hidden.
    ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
                    WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT,
                    0, 0, hInstance, 0);
    if(ghApp)
    {
        // Create DirectShow graph and start capturing video
        hr = CaptureVideoByMoniker();
        if (FAILED (hr))
        {
            CloseInterfaces();
            DestroyWindow(ghApp);
        }

        // Main message loop
        while(GetMessage(&msg,NULL,0,0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Release COM
    CoUninitialize();

    return ((int) msg.wParam);
}

//------------------------------------------------------------------------------
// File: vcdplyer.cpp
//
// Desc: DirectShow sample code - VMR-based Cube video player
//
// Copyright (c) 1994-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <mmreg.h>
#include <commctrl.h>
#include <atlbase.h>

#include "project.h"

#include <stdarg.h>
#include <stdio.h>

#define MY_USER_ID 0x1234ACDE

/******************************Public*Routine******************************\
* CMpegMovie
*
* Constructors and destructors
*
\**************************************************************************/
CMpegMovie::CMpegMovie(HWND hwndApplication)
    : CUnknown(NAME("Allocator Presenter"), NULL),
      m_hwndApp(hwndApplication),
      m_bInitCube(false),
      m_pDDSTextureMirror(NULL),
      m_MediaEvent(NULL),
      m_Mode(MOVIE_NOTOPENED),
      m_Fg(NULL),
      m_Gb(NULL),
      m_Mc(NULL),
      m_Me(NULL),
      m_Wc(NULL),
      m_Ms(NULL),
      m_dwRegister(0),
      m_dwTexMirrorWidth(0),
      m_dwTexMirrorHeight(0)
{
    AddRef();
}

CMpegMovie::~CMpegMovie() {
    ;
}

/******************************Public*Routine******************************\
* NonDelegatingQueryInterface
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::NonDelegatingQueryInterface(
    REFIID riid,
    void** ppv
    )
{
    if (riid == IID_IVMRImageCompositor) {
        return GetInterface((IVMRImageCompositor*)this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}

/*****************************Private*Routine******************************\
* SetRenderingMode
*
\**************************************************************************/
HRESULT
SetRenderingMode(
    IBaseFilter* pBaseFilter,
    VMRMode mode,
    int iNumStreams,
    IVMRImageCompositor* lpCompositor
    )
{
    IVMRFilterConfig* pConfig = NULL;
    HRESULT hr = S_OK;

    __try {

        CHECK_HR(hr = pBaseFilter->QueryInterface(IID_IVMRFilterConfig,
                                                  (LPVOID *)&pConfig));

        //
        // If you are pluging in a compositor you have to be in
        // mixing mode, that is, iNumStreams needs to be greater than 0.
        //
        if (lpCompositor && iNumStreams < 1) {
            iNumStreams = 1;
        }

        if (iNumStreams) {
            CHECK_HR(hr = pConfig->SetNumberOfStreams(iNumStreams));
        }

        if (lpCompositor) {
            CHECK_HR(hr = pConfig->SetImageCompositor(lpCompositor));
        }

        CHECK_HR(hr = pConfig->SetRenderingMode(mode));
    }
    __finally {
        RELEASE(pConfig);
    }

    return hr;
}


/*****************************Private*Routine******************************\
* AddVideoMixingRendererToFG()
*
\**************************************************************************/
HRESULT
CMpegMovie::AddVideoMixingRendererToFG(DWORD dwStreams)
{
    IBaseFilter* pBF = NULL;
    HRESULT hRes = S_OK;

    __try {
        CHECK_HR(hRes = CoCreateInstance(CLSID_VideoMixingRenderer,
                                NULL, CLSCTX_INPROC,IID_IBaseFilter,
                                (LPVOID *)&pBF));

        CHECK_HR(hRes = m_Fg->AddFilter(pBF, L"Video Mixing Renderer"));
        CHECK_HR(hRes = SetRenderingMode(pBF, VMRMode_Windowless,
                                         dwStreams, (IVMRImageCompositor*)this));
 
        CHECK_HR(hRes = pBF->QueryInterface(IID_IVMRWindowlessControl, (LPVOID *)&m_Wc));
        CHECK_HR(hRes = m_Wc->SetVideoClippingWindow(m_hwndApp));

        CHECK_HR(hRes = m_Wc->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX));
    }
    __finally {
        RELEASE(pBF);
    }

    return hRes;
}


HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}


/******************************Public*Routine******************************\
* OpenMovie
*
\**************************************************************************/
HRESULT
CMpegMovie::OpenMovie(
    TCHAR achFileName[][MAX_PATH],
    DWORD dwNumFiles
    )
{
    USES_CONVERSION;
    IUnknown        *pUnk = NULL;
    HRESULT         hres = S_OK;
    WCHAR           FileName[MAX_PATH];

    __try {
        CHECK_HR(hres = CoCreateInstance(CLSID_FilterGraph,
                                         NULL, CLSCTX_INPROC,
                                         IID_IUnknown, (LPVOID *)&pUnk));
        m_Mode = MOVIE_OPENED;
        CHECK_HR(hres = pUnk->QueryInterface(IID_IFilterGraph, (LPVOID *)&m_Fg));
        CHECK_HR(hres = AddVideoMixingRendererToFG(4));

        CHECK_HR(hres = pUnk->QueryInterface(IID_IGraphBuilder, (LPVOID *)&m_Gb));

        AddToRot(m_Gb, &m_dwRegister);

        for (DWORD i = 0; i < dwNumFiles; i++)
        {
            wcscpy(FileName, T2W(achFileName[i]));
            CHECK_HR(hres = m_Gb->RenderFile(FileName, NULL));
        }
        CHECK_HR(hres = pUnk->QueryInterface(IID_IMediaControl, (LPVOID *)&m_Mc));

        //
        // Not being able to get the IMediaEvent interface does
        // necessarly mean that we can't play the graph.
        //
        pUnk->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_Me);
        GetMovieEventHandle();
        pUnk->QueryInterface(IID_IMediaSeeking, (LPVOID *)&m_Ms);
    }
    __finally {

        if (FAILED(hres)) {
            RELEASE(m_Ms);
            RELEASE(m_Me);
            RELEASE(m_Mc);
            RELEASE(m_Gb);
            RELEASE(m_Fg);
        }

        RELEASE(pUnk);
    }

    return hres;
}


void RemoveFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;
    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}


/******************************Public*Routine******************************\
* CloseMovie
*
\**************************************************************************/
DWORD
CMpegMovie::CloseMovie(
    )
{
    m_Mode = MOVIE_NOTOPENED;

    RELEASE(m_Ms);
    RELEASE(m_Mc);
    RELEASE(m_Me);
    RELEASE(m_Gb);
    RELEASE(m_Fg);
    RELEASE(m_Wc);
    RELEASE(m_pDDSTextureMirror);

    if (m_dwRegister)
    {
        RemoveFromRot(m_dwRegister);
        m_dwRegister = 0;
    }

    return 0L;
}


/******************************Public*Routine******************************\
* RepaintVideo
*
\**************************************************************************/
BOOL
CMpegMovie::RepaintVideo(
    HWND hwnd,
    HDC hdc
    )
{
    BOOL bRet = FALSE;
    if (m_Wc) {
        bRet = (m_Wc->RepaintVideo(hwnd, hdc) == S_OK);
    }
    return bRet;
}


/******************************Public*Routine******************************\
* PutMoviePosition
*
\**************************************************************************/
BOOL
CMpegMovie::PutMoviePosition(
    LONG x,
    LONG y,
    LONG cx,
    LONG cy
    )
{
    RECT rc;
    SetRect(&rc, x, y, x + cx, y + cy);
    BOOL bRet = (m_Wc->SetVideoPosition(NULL, &rc) == S_OK);
    return bRet;
}



/******************************Public*Routine******************************\
* PlayMovie
*
\**************************************************************************/
BOOL
CMpegMovie::PlayMovie(
    )
{
    REFTIME rt;
    REFTIME rtAbs;
    REFTIME rtDur;

    rt = GetCurrentPosition();
    rtDur = GetDuration();

    //
    // If we are near the end of the movie seek to the start, otherwise
    // stay where we are.
    //
    rtAbs = rt - rtDur;
    if (rtAbs < (REFTIME)0) {
        rtAbs = -rtAbs;
    }

    if (rtAbs <= (REFTIME)1) {
        SeekToPosition((REFTIME)0,FALSE);
    }

    //
    // Change mode after setting m_Mode but before starting the graph
    //
    m_Mode = MOVIE_PLAYING;
    HRESULT hr = m_Mc->Run();
    ASSERT(SUCCEEDED(hr));
    return TRUE;
}


/******************************Public*Routine******************************\
* PauseMovie
*
\**************************************************************************/
BOOL
CMpegMovie::PauseMovie(
    )
{
    m_Mode = MOVIE_PAUSED;
    HRESULT hr = m_Mc->Pause();
    ASSERT(SUCCEEDED(hr));
    return TRUE;
}


/******************************Public*Routine******************************\
* GetStateMovie
*
\**************************************************************************/

OAFilterState
CMpegMovie::GetStateMovie(
    )
{
    OAFilterState State;
    HRESULT hr = m_Mc->GetState(INFINITE,&State);
    ASSERT(SUCCEEDED(hr));
    return State;
}


/******************************Public*Routine******************************\
* StopMovie
*
\**************************************************************************/
BOOL
CMpegMovie::StopMovie(
    )
{
    m_Mode = MOVIE_STOPPED;
    HRESULT hr = m_Mc->Stop();
    ASSERT(SUCCEEDED(hr));
    return TRUE;
}


/******************************Public*Routine******************************\
* GetMediaEventHandle
*
* Returns the IMediaEvent event hamdle for the filter graph iff the
* filter graph exists.
*
\**************************************************************************/
HANDLE
CMpegMovie::GetMovieEventHandle(
    )
{
    HRESULT hr;

    if (m_Me != NULL) {

        if ( m_MediaEvent == NULL) {
            hr = m_Me->GetEventHandle((OAEVENT *)&m_MediaEvent);
        }
    }
    else {
        m_MediaEvent = NULL;
    }

    return m_MediaEvent;
}


/******************************Public*Routine******************************\
* GetMovieEventCode
*
\**************************************************************************/
long
CMpegMovie::GetMovieEventCode()
{
    HRESULT hr;
    long    lEventCode;
    LONG_PTR    lParam1, lParam2;

    if (m_Me != NULL) {
        hr = m_Me->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
        if(SUCCEEDED(hr))
        {
            hr = m_Me->FreeEventParams(lEventCode, lParam1, lParam2);
            return lEventCode;
        }
    }

    return 0L;
}


/******************************Public*Routine******************************\
* GetDuration
*
* Returns the duration of the current movie
*
\**************************************************************************/
REFTIME
CMpegMovie::GetDuration()
{
    HRESULT hr;
    LONGLONG Duration;

    // Should we seek using IMediaSelection

    if (m_Ms != NULL && m_TimeFormat != TIME_FORMAT_MEDIA_TIME) {
        hr = m_Ms->GetDuration(&Duration);
        if (SUCCEEDED(hr)) {
            return double(Duration);
        }
    } else if (m_Ms != NULL) {
        hr = m_Ms->GetDuration(&Duration);
        if (SUCCEEDED(hr)) {
            return double(Duration) / UNITS;
        }
    }
    return 0;
}


/******************************Public*Routine******************************\
* GetCurrentPosition
*
* Returns the duration of the current movie
*
\**************************************************************************/
REFTIME
CMpegMovie::GetCurrentPosition()
{
    REFTIME rt = (REFTIME)0;
    HRESULT hr;
    LONGLONG Position;

    // Should we return a media position

    if (m_Ms != NULL && m_TimeFormat != TIME_FORMAT_MEDIA_TIME) {
        hr = m_Ms->GetPositions(&Position, NULL);
        if (SUCCEEDED(hr)) {
            return double(Position);
        }
    } else if (m_Ms != NULL) {
        hr = m_Ms->GetPositions(&Position, NULL);
        if (SUCCEEDED(hr)) {
            return double(Position) / UNITS;
        }
    }
    return rt;
}


/*****************************Private*Routine******************************\
* SeekToPosition
*
\**************************************************************************/
BOOL
CMpegMovie::SeekToPosition(
    REFTIME rt,
    BOOL bFlushData
    )
{
    HRESULT hr=S_OK;
    LONGLONG llTime =
        LONGLONG(m_TimeFormat == TIME_FORMAT_MEDIA_TIME ? rt * double(UNITS) : rt);

    if (m_Ms != NULL) {

        FILTER_STATE fs;
        hr = m_Mc->GetState(100, (OAFilterState *)&fs);

        hr = m_Ms->SetPositions(&llTime, AM_SEEKING_AbsolutePositioning, NULL, 0);

        // This gets new data through to the renderers
        if (fs == State_Stopped && bFlushData){
            hr = m_Mc->Pause();
            hr = m_Mc->GetState(INFINITE, (OAFilterState *)&fs);
            hr = m_Mc->Stop();
        }

        if (SUCCEEDED(hr)) {
            return TRUE;
        }
    }
    return FALSE;
}


/*****************************Private*Routine******************************\
* VerifyVMR
*
\**************************************************************************/
BOOL VerifyVMR(void)
{
    HRESULT hres;

    // Verify that the VMR exists on this system
    IBaseFilter* pBF = NULL;
    hres = CoCreateInstance(CLSID_VideoMixingRenderer,
        NULL,
        CLSCTX_INPROC,
        IID_IBaseFilter,
        (LPVOID *)&pBF);
    if(SUCCEEDED(hres))
    {
        pBF->Release();
        return TRUE;
    }
    else
    {
        MessageBox(hwndApp, 
            TEXT("This application requires the Video Mixing Renderer, which is present\r\n")
            TEXT("only on Windows XP.\r\n\r\n")
            TEXT("The Video Mixing Renderer (VMR) is also not enabled when viewing a \r\n")
            TEXT("remote Windows XP machine through a Remote Desktop session.\r\n")
            TEXT("You can run VMR-enabled applications only on your local machine.")
            TEXT("\r\n\r\nThis sample will now exit."),
            TEXT("Video Mixing Renderer capabilities are required"), MB_OK);

        return FALSE;
    }
}





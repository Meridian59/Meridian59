//----------------------------------------------------------------------------
//  File:   vcdplyer.cpp
//
//  Desc:   DirectShow sample code
//          Implementation of CMpegMovie, a customized video player
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------
#include "project.h"
#include <mmreg.h>
#include <commctrl.h>

#include <stdarg.h>
#include <stdio.h>

#define MY_USER_ID 0x1234ACDE

//----------------------------------------------------------------------------
// CMpegMovie
//
// Constructors and destructors
//
//----------------------------------------------------------------------------
CMpegMovie::CMpegMovie(HWND hwndApplication)
    : CUnknown(NAME("Allocator Presenter"), NULL),
      m_hwndApp(hwndApplication),
      m_MediaEvent(NULL),
      m_Mode(MOVIE_NOTOPENED),
      m_pBF(NULL),
      m_Fg(NULL),
      m_Gb(NULL),
      m_Mc(NULL),
      m_Ms(NULL),
      m_Me(NULL),
      m_Qp(NULL),
      m_lpDefSAN(NULL),
      m_iDuration(-1),
      m_TimeFormat(TIME_FORMAT_MEDIA_TIME),
      m_lpDefWC(NULL),
      m_lpDefIP(NULL),
      m_lpDefSA(NULL),
      m_AlphaBlt(NULL)

{
    AddRef();
}

CMpegMovie::~CMpegMovie() {
    ;
}

//----------------------------------------------------------------------------
// SetRenderingMode
//
// Set rendering mode of VMR (Windowless, Windowed, Renderless)
//----------------------------------------------------------------------------
HRESULT
SetRenderingMode(
    IBaseFilter* pBaseFilter,
    VMRMode mode
    )
{
    IVMRFilterConfig* pConfig;
    HRESULT hr = pBaseFilter->QueryInterface(__uuidof(IVMRFilterConfig),
                                             (LPVOID *)&pConfig);

    if( SUCCEEDED( hr )) {
        pConfig->SetRenderingMode( mode );
        pConfig->Release();
    }
    return hr;
}

HRESULT CMpegMovie::AllocateAlphaBlt(void)
{
    HRESULT hRes=S_OK;

    m_AlphaBlt = new CAlphaBlt(m_pDDObject.GetBB(), &hRes);
    return hRes;
}

//----------------------------------------------------------------------------
// AddVideoMixingRendererToFG()
//
// creates and adds VMR to the graph
//----------------------------------------------------------------------------
HRESULT
CMpegMovie::AddVideoMixingRendererToFG()
{
    HRESULT hRes = S_OK;

    m_AlphaBlt = NULL;

    __try {
        CHECK_HR(hRes = m_pDDObject.Initialize(m_hwndApp));

        CHECK_HR(hRes = CoCreateInstance(CLSID_VideoMixingRenderer,
                                NULL, CLSCTX_INPROC,IID_IBaseFilter,
                                (LPVOID *)&m_pBF));

        CHECK_HR(hRes = m_Fg->AddFilter(m_pBF, L"Video Mixing Renderer"));

        CHECK_HR(hRes = SetRenderingMode(m_pBF, VMRMode_Renderless));

        CHECK_HR(hRes = m_pBF->QueryInterface(__uuidof(IVMRSurfaceAllocatorNotify),
                                (LPVOID *)&m_lpDefSAN));

        hRes = AllocateAlphaBlt();
        if (FAILED(hRes)) {
            __leave;
        }


        CHECK_HR(hRes = CreateDefaultAllocatorPresenter(m_pDDObject.GetDDObj(),
                                                        m_pDDObject.GetBB()));

        CHECK_HR(hRes = m_lpDefSAN->AdviseSurfaceAllocator(MY_USER_ID, this));
    }
    __finally {

        if (FAILED(hRes)) {
            delete m_AlphaBlt;
            m_AlphaBlt= NULL;
            m_pDDObject.Terminate();
        }
    }

    return hRes;
}


//----------------------------------------------------------------------------
// OpenMovie
//
// creates the graph, adds VMR, QIs relevant interfaces and renders the file
//----------------------------------------------------------------------------
HRESULT
CMpegMovie::OpenMovie(
    TCHAR *lpFileName
    )
{
    IUnknown        *pUnk = NULL;
    HRESULT         hres = S_OK;

    WCHAR           FileName[MAX_PATH];

#ifdef UNICODE
    lstrcpy(FileName, lpFileName);
#else
    // wsprintfA((char*)FileName, "%hs", lpFileName);
    MultiByteToWideChar(CP_ACP,0,(const char*) lpFileName, -1, FileName, MAX_PATH);
#endif

    __try {
        CHECK_HR(hres = CoCreateInstance(CLSID_FilterGraph,
                                         NULL, CLSCTX_INPROC,
                                         IID_IUnknown, (LPVOID *)&pUnk));
        m_Mode = MOVIE_OPENED;
        CHECK_HR(hres = pUnk->QueryInterface(IID_IFilterGraph, (LPVOID *)&m_Fg));
        CHECK_HR(hres = AddVideoMixingRendererToFG());
        CHECK_HR(hres = pUnk->QueryInterface(IID_IGraphBuilder, (LPVOID *)&m_Gb));
        CHECK_HR(hres = m_Gb->RenderFile(FileName, NULL));
        CHECK_HR(hres = pUnk->QueryInterface(IID_IMediaControl, (LPVOID *)&m_Mc));
        CHECK_HR(hres = m_pBF->QueryInterface(IID_IQualProp, (LPVOID*)&m_Qp));

        //
        // Not being able to get the IMediaEvent interface does
        // necessarly mean that we can't play the graph.
        //
        HRESULT hr = pUnk->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_Me);
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

            m_pDDObject.Terminate();
        }

        RELEASE(pUnk);
    }

    return hres;
}


//----------------------------------------------------------------------------
//  CloseMovie
//
//  Releases client-provided allocator-presenter, exits sxclusive mode
//----------------------------------------------------------------------------
DWORD
CMpegMovie::CloseMovie(
    )
{
    m_Mode = MOVIE_NOTOPENED;

    RELEASE(m_Qp);
    RELEASE(m_Mc);
    RELEASE(m_Me);
    RELEASE(m_Ms);
    RELEASE(m_Gb);
    RELEASE(m_Fg);

    if (m_lpDefSAN) {
        m_lpDefSAN->AdviseSurfaceAllocator(0, NULL);
        m_lpDefSAN->Release();
    }

    RELEASE(m_pBF);
    RELEASE(m_lpDefWC);
    RELEASE(m_lpDefSA);
    RELEASE(m_lpDefIP);

    delete m_AlphaBlt;
    m_pDDObject.Terminate();

    return 0L;
}


//----------------------------------------------------------------------------
// RepaintVideo
//----------------------------------------------------------------------------
BOOL
CMpegMovie::RepaintVideo(
    HWND hwnd,
    HDC hdc
    )
{
    BOOL bRet = FALSE;
    if (m_lpDefWC) {
        bRet = (m_lpDefWC->RepaintVideo(hwnd, hdc) == S_OK);
    }

    return bRet;
}


//----------------------------------------------------------------------------
// PutMoviePosition
//----------------------------------------------------------------------------
BOOL
CMpegMovie::PutMoviePosition(
    LONG x,
    LONG y,
    LONG cx,
    LONG cy
    )
{
    HRESULT hr;
    BOOL bRet = FALSE;

    if (m_lpDefWC) {
        RECT rc;
        SetRect(&rc, x, y, x + cx, y + cy);
        hr = m_lpDefWC->SetVideoPosition(NULL, &rc);
        bRet = (hr == S_OK);
    }

    return bRet;
}


//----------------------------------------------------------------------------
//  PlayMovie
//
//  Just runs IMediaControl
//
//----------------------------------------------------------------------------
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
    return TRUE;
}


//----------------------------------------------------------------------------
// PauseMovie
//
//
//----------------------------------------------------------------------------
BOOL
CMpegMovie::PauseMovie(
    )
{
    m_Mode = MOVIE_PAUSED;
    m_Mc->Pause();
    return TRUE;
}


//----------------------------------------------------------------------------
// GetStateMovie
//
// returns state of the media control (running, paused, or stopped)
//----------------------------------------------------------------------------

OAFilterState
CMpegMovie::GetStateMovie(
    )
{
    OAFilterState State=0;
    if (m_Mc)
        m_Mc->GetState(INFINITE,&State);
    return State;
}


//----------------------------------------------------------------------------
// StopMovie
//
//----------------------------------------------------------------------------
BOOL
CMpegMovie::StopMovie(
    )
{
    m_Mode = MOVIE_STOPPED;
    m_Mc->Stop();
    return TRUE;
}


//----------------------------------------------------------------------------
// GetMediaEventHandle
//
// Returns the IMediaEvent event handle for the filter graph iff the
// filter graph exists.
//
//----------------------------------------------------------------------------
HANDLE
CMpegMovie::GetMovieEventHandle(
    )
{
    HRESULT     hr = S_OK;

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


//----------------------------------------------------------------------------
// GetMovieEventCode
//
// Retrieves notification events from the graph through IMediaEvent interface
//----------------------------------------------------------------------------
long
CMpegMovie::GetMovieEventCode()
{
    HRESULT hr;
    long    lEventCode;
    LONG_PTR    lParam1, lParam2;

    if (m_Me != NULL) {
        hr = m_Me->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
        if (SUCCEEDED(hr)) {
            return lEventCode;
        }
    }

    return 0L;
}


//----------------------------------------------------------------------------
// GetDuration
//
// Returns the duration of the current movie
// NOTE that time format may vary with different media types
//
//----------------------------------------------------------------------------
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


//----------------------------------------------------------------------------
// GetCurrentPosition
//
// Returns the duration of the current movie
// NOTE that time format may vary with different media types
//
//----------------------------------------------------------------------------
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


//----------------------------------------------------------------------------
// SeekToPosition
//
// NOTE that time format may vary with different media types
//
//----------------------------------------------------------------------------
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
        m_Mc->GetState(100, (OAFilterState *)&fs);

        hr = m_Ms->SetPositions(&llTime, AM_SEEKING_AbsolutePositioning, NULL, 0);

        // This gets new data through to the renderers

        if (fs == State_Stopped && bFlushData){
            m_Mc->Pause();
            hr = m_Mc->GetState(INFINITE, (OAFilterState *)&fs);
            m_Mc->Stop();
        }

        if (SUCCEEDED(hr)) {
            return TRUE;
        }
    }
    return FALSE;
}


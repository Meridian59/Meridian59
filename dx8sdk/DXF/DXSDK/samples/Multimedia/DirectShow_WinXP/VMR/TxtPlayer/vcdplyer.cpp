//------------------------------------------------------------------------------
// File: vcdplyer.cpp
//
// Desc: DirectShow sample code - VMR-enabled player app with text
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <mmreg.h>
#include <commctrl.h>
#include <atlbase.h>

#include "project.h"
#include "mpgcodec.h"

#include <stdarg.h>
#include <stdio.h>

extern int FrameStepCount;


/******************************Public*Routine******************************\
* CMpegMovie
*
* Constructors and destructors
*
\**************************************************************************/
CMpegMovie::CMpegMovie(HWND hwndApplication)
    : CUnknown(NAME("Allocator Presenter"), NULL),
      m_hwndApp(hwndApplication),
      m_MediaEvent(NULL),
      m_Mode(MOVIE_NOTOPENED),
      m_Fg(NULL),
      m_Gb(NULL),
      m_Mc(NULL),
      m_Ms(NULL),
      m_Me(NULL),
      m_Wc(NULL),
      m_SAN(NULL),
      m_bRndLess(TRUE),
      pMpegAudioDecoder(NULL),
      pVideoRenderer(NULL),
      m_TimeFormat(TIME_FORMAT_MEDIA_TIME),
      m_hFont(NULL),
      m_cxFont(0), m_cxFontImg(0),
      m_cyFont(0), m_cyFontImg(0),
      m_lpDDSFontCache(NULL)
{
    m_hMonitor = NULL;
    m_lpDDObj = NULL;
    m_lpPriSurf = NULL;
    m_lpBackBuffer = NULL;
    m_lpDDTexture = NULL;

    AddRef();
}
CMpegMovie::~CMpegMovie()
{
}


static HRESULT SetRenderingMode( IBaseFilter* pBaseFilter, VMRMode mode )
{
    // Test VMRConfig, VMRMonitorConfig
    IVMRFilterConfig* pConfig;
    HRESULT hr = pBaseFilter->QueryInterface(IID_IVMRFilterConfig, (LPVOID *)&pConfig);
    if(SUCCEEDED(hr))
    {
        pConfig->SetRenderingMode(mode);
        pConfig->SetRenderingPrefs(RenderPrefs_AllowOverlays);
        pConfig->Release();
    }
    return hr;
}

/******************************Public*Routine******************************\
* AddVideoMixingRendererToFG
*
\**************************************************************************/
HRESULT
CMpegMovie::AddVideoMixingRendererToFG()
{
    IBaseFilter* pBF = NULL;
    HRESULT hRes = CoCreateInstance(CLSID_VideoMixingRenderer,
        NULL,
        CLSCTX_INPROC,
        IID_IBaseFilter,
        (LPVOID *)&pBF);

    if(SUCCEEDED(hRes))
    {
        hRes = m_Fg->AddFilter(pBF, L"Video Mixing Renderer");

        if(SUCCEEDED(hRes))
        {
            if(m_bRndLess)
            {
                hRes = SetRenderingMode(pBF, VMRMode_Renderless);

                if(SUCCEEDED(hRes))
                {
                    hRes = pBF->QueryInterface(IID_IVMRSurfaceAllocatorNotify,
                        (LPVOID *)&m_SAN);
                }

                if(SUCCEEDED(hRes))
                {
                    hRes = m_SAN->AdviseSurfaceAllocator(0, this);
                }

                if(SUCCEEDED(hRes))
                {
                    hRes = m_SAN->SetDDrawDevice(m_lpDDObj, m_hMonitor);
                }
            }
            else
            {
                hRes = SetRenderingMode(pBF, VMRMode_Windowless);
                if(SUCCEEDED(hRes))
                {
                    hRes = pBF->QueryInterface(IID_IVMRWindowlessControl, (LPVOID *)&m_Wc);
                }

                if(SUCCEEDED(hRes))
                {
                    m_Wc->SetVideoClippingWindow(m_hwndApp);
                }
                else
                {
                    if(m_Wc)
                    {
                        m_Wc->Release();
                        m_Wc = NULL;
                    }
                }
            }
        }
    }

    if(pBF)
    {
        pBF->Release();
    }

    if(FAILED(hRes))
    {
        if(m_SAN)
        {
            m_SAN->Release();
            m_SAN = NULL;
        }
    }

    return hRes;
}

/******************************Public*Routine******************************\
* OpenMovie
*
\**************************************************************************/
HRESULT
CMpegMovie::OpenMovie(
    TCHAR *lpFileName
    )
{
    USES_CONVERSION;
    IUnknown        *pUnk;
    HRESULT         hres;
    WCHAR           FileName[MAX_PATH];

    wcscpy(FileName, T2W(lpFileName));

    hres = CoInitialize(NULL);
    if(hres == S_FALSE)
    {
        CoUninitialize();
    }

    Initialize3DEnvironment(m_hwndApp);

    hres = CoCreateInstance(
        CLSID_FilterGraph,
        NULL,
        CLSCTX_INPROC,
        IID_IUnknown,
        (LPVOID *)&pUnk);

    if(SUCCEEDED(hres))
    {
        m_Mode = MOVIE_OPENED;
        hres = pUnk->QueryInterface(IID_IFilterGraph, (LPVOID *)&m_Fg);
        if(FAILED(hres))
        {
            pUnk->Release();
            return hres;
        }

        hres = AddVideoMixingRendererToFG();
        if(FAILED(hres))
        {
            m_Fg->Release(); m_Fg = NULL;
            return hres;
        }

        hres = pUnk->QueryInterface(IID_IGraphBuilder, (LPVOID *)&m_Gb);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            return hres;
        }

        hres = m_Gb->RenderFile(FileName, NULL);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            if(m_Wc)
                m_Wc->Release(); m_Wc = NULL;
            m_Gb->Release(); m_Gb = NULL;
            return hres;
        }

        hres = pUnk->QueryInterface(IID_IMediaControl, (LPVOID *)&m_Mc);
        if(FAILED(hres))
        {
            pUnk->Release();
            m_Fg->Release(); m_Fg = NULL;
            m_Wc->Release(); m_Wc = NULL;
            m_Gb->Release(); m_Gb = NULL;
            return hres;
        }

        //
        // Not being able to get the IMediaEvent interface does not
        // necessarly mean that we can't play the graph.
        //
        pUnk->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_Me);
        pUnk->QueryInterface(IID_IMediaSeeking, (LPVOID *)&m_Ms);

        GetMovieEventHandle();
        GetPerformanceInterfaces();

        pUnk->Release();
        return S_OK;

    }
    else
    {
        m_Fg = NULL;
    }

    return hres;
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

    if(m_Mc)
    {
        if(pMpegAudioDecoder)
        {
            pMpegAudioDecoder->Release();
            pMpegAudioDecoder = NULL;
        }

        if(pVideoRenderer)
        {
            pVideoRenderer->Release();
            pVideoRenderer = NULL;
        }

        if(m_Me)
        {
            m_MediaEvent = NULL;
            m_Me->Release();
            m_Me = NULL;
        }

        if(m_Ms)
        {
            m_Ms->Release();
            m_Ms = NULL;
        }

        if(m_Wc)
        {
            m_Wc->Release();
            m_Wc = NULL;
        }

        m_Mc->Release();
        m_Mc = NULL;

        if(m_SAN)
        {
            m_SAN->Release();
            m_SAN = NULL;
        }

        if(m_Gb)
        {
            m_Gb->Release();
            m_Gb = NULL;
        }

        if(m_Fg)
        {
            m_Fg->Release();
            m_Fg = NULL;
        }
    }

    delete m_lpBltAlpha;

    RELEASE(m_lpDDObj);
    RELEASE(m_lpPriSurf);
    RELEASE(m_lpBackBuffer);
    RELEASE(m_lpDDTexture);
    RELEASE(m_lpDDAppImage);

    if(m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    RELEASE(m_lpDDSFontCache);

    QzUninitialize();
    return 0L;
}


/******************************Public*Routine******************************\
* CMpegMovie::GetNativeMovieSize
*
\**************************************************************************/
BOOL
CMpegMovie::GetNativeMovieSize(
    LONG *pcx,
    LONG *pcy
    )
{
    BOOL    bRet = FALSE;
    if(m_Wc)
    {
        bRet = (m_Wc->GetNativeVideoSize(pcx, pcy, NULL, NULL) == S_OK);
    }

    return bRet;
}


/******************************Public*Routine******************************\
* GetMoviePosition
*
\**************************************************************************/
BOOL
CMpegMovie::GetMoviePosition(
    LONG *px,
    LONG *py,
    LONG *pcx,
    LONG *pcy
    )
{
    BOOL    bRet = FALSE;

    if(m_Wc)
    {
        RECT src={0}, dest={0};
        HRESULT hr = m_Wc->GetVideoPosition(&src, &dest);
        *px = dest.left;
        *py = dest.right;
        *pcx = dest.right - dest.left;
        *pcy = dest.bottom - dest.top;
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
    BOOL    bRet = TRUE;

    RECT rc;
    SetRect(&rc, x, y, x + cx, y + cy);

    if(m_bRndLess)
    {
        CAutoLock Lock(&m_AppImageLock);
        MapWindowRect(m_hwndApp, HWND_DESKTOP, &rc);
        m_rcDst = rc;
        CreateFontCache(HEIGHT(&m_rcDst) / GRID_CY);
    }
    else
    {
        if(m_Wc)
        {
            bRet = (m_Wc->SetVideoPosition(NULL, &rc) == S_OK);
        }
    }

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
    if(rtAbs < (REFTIME)0)
    {
        rtAbs = -rtAbs;
    }

    if(rtAbs < (REFTIME)1)
    {
        SeekToPosition((REFTIME)0,FALSE);
    }

    //
    // Change mode after setting m_Mode but before starting the graph
    //
    m_Mode = MOVIE_PLAYING;

    //
    // Start playing from the begining of the movie
    //
    m_Mc->Run();
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
    m_Mc->Pause();
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
    m_Mc->GetState(INFINITE,&State);
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
    m_Mc->Stop();
    return TRUE;
}


/******************************Public*Routine******************************\
* StatusMovie
*
\**************************************************************************/
EMpegMovieMode
CMpegMovie::StatusMovie(
    )
{
    if(m_Mc)
    {
        FILTER_STATE    fs;
        HRESULT         hr;

        hr = m_Mc->GetState(100, (OAFilterState *)&fs);

        // Don't know what the state is so just stay at old state.
        if(hr == VFW_S_STATE_INTERMEDIATE)
        {
            return m_Mode;
        }

        switch(fs)
        {
            case State_Stopped:
                m_Mode = MOVIE_STOPPED;
                break;

            case State_Paused:
                m_Mode = MOVIE_PAUSED;
                break;

            case State_Running:
                m_Mode = MOVIE_PLAYING;
                break;
        }
    }

    return m_Mode;
}


/******************************Public*Routine******************************\
* CanMovieFrameStep
*
\**************************************************************************/
BOOL
CMpegMovie::CanMovieFrameStep()
{
    IVideoFrameStep* lpFS;
    HRESULT hr;

    hr = m_Fg->QueryInterface(__uuidof(IVideoFrameStep), (LPVOID *)&lpFS);
    if(SUCCEEDED(hr))
    {
        hr = lpFS->CanStep(0L, NULL);
        lpFS->Release();
    }

    return SUCCEEDED(hr);
}


/******************************Public*Routine******************************\
* FrameStepMovie
*
\**************************************************************************/
BOOL
CMpegMovie::FrameStepMovie()
{
    IVideoFrameStep* lpFS;
    HRESULT hr;

    hr = m_Fg->QueryInterface(__uuidof(IVideoFrameStep), (LPVOID *)&lpFS);
    if(SUCCEEDED(hr))
    {
        FrameStepCount++;

        hr = lpFS->Step(1, NULL);
        lpFS->Release();
    }

    return SUCCEEDED(hr);
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
    HRESULT     hr;

    if(m_Me != NULL)
    {
        if(m_MediaEvent == NULL)
        {
            hr = m_Me->GetEventHandle((OAEVENT *)&m_MediaEvent);
        }
    }
    else
    {
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

    if(m_Me != NULL)
    {
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

    if(m_TimeFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = m_Ms->GetDuration(&Duration);
        if(SUCCEEDED(hr))
        {
            return double(Duration);
        }
    }
    else if(m_Ms != NULL)
    {
        hr = m_Ms->GetDuration(&Duration);
        if(SUCCEEDED(hr))
        {
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

    if(m_TimeFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = m_Ms->GetPositions(&Position, NULL);
        if(SUCCEEDED(hr))
        {
            return double(Position);
        }
    }
    else if(m_Ms != NULL)
    {
        hr = m_Ms->GetPositions(&Position, NULL);
        if(SUCCEEDED(hr))
        {
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
    LONGLONG llTime = LONGLONG(m_TimeFormat == TIME_FORMAT_MEDIA_TIME ? rt * double(UNITS) : rt );

    if(m_Ms != NULL)
    {
        FILTER_STATE fs;
        m_Mc->GetState(100, (OAFilterState *)&fs);

        hr = m_Ms->SetPositions(&llTime, AM_SEEKING_AbsolutePositioning, NULL, 0);

        // This gets new data through to the renderers
        if(fs == State_Stopped && bFlushData)
        {
            m_Mc->Pause();
            hr = m_Mc->GetState(INFINITE, (OAFilterState *)&fs);
            m_Mc->Stop();
        }

        if(SUCCEEDED(hr))
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*****************************Private*Routine******************************\
* GetPerformanceInterfaces
*
\**************************************************************************/
void
CMpegMovie::GetPerformanceInterfaces(
    )
{
    FindInterfaceFromFilterGraph(IID_IMpegAudioDecoder, (LPVOID *)&pMpegAudioDecoder);
    FindInterfaceFromFilterGraph(IID_IQualProp, (LPVOID *)&pVideoRenderer);
}


HRESULT
CMpegMovie::FindInterfaceFromFilterGraph(
    REFIID iid, // interface to look for
    LPVOID *lp  // place to return interface pointer in
    )
{
    IEnumFilters*   pEF;    
    IBaseFilter*        pFilter;

    // Grab an enumerator for the filter graph.
    HRESULT hr = m_Fg->EnumFilters(&pEF);

    if(FAILED(hr))
    {
        return hr;
    }

    // Check out each filter.
    while(pEF->Next(1, &pFilter, NULL) == S_OK)
    {
        hr = pFilter->QueryInterface(iid, lp);
        pFilter->Release();

        if(SUCCEEDED(hr))
        {
            break;
        }
    }

    pEF->Release();

    return hr;
}


/*****************************Public*Routine******************************\
* IsTimeFormatSupported
*
\**************************************************************************/
BOOL
CMpegMovie::IsTimeFormatSupported(GUID Format)
{
    return m_Ms != NULL && m_Ms->IsFormatSupported(&Format) == S_OK;
}


/*****************************Public*Routine******************************\
* IsTimeSupported
*
\**************************************************************************/
BOOL
CMpegMovie::IsTimeSupported()
{
    return m_Ms != NULL && m_Ms->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK;
}


/*****************************Public*Routine******************************\
* GetTimeFormat
*
\**************************************************************************/
GUID
CMpegMovie::GetTimeFormat()
{
    return m_TimeFormat;
}

/*****************************Public*Routine******************************\
* SetTimeFormat
*
\**************************************************************************/
BOOL
CMpegMovie::SetTimeFormat(GUID Format)
{
    HRESULT hr = m_Ms->SetTimeFormat(&Format);
    if(SUCCEEDED(hr))
    {
        m_TimeFormat = Format;
    }
    return SUCCEEDED(hr);
}

/******************************Public*Routine******************************\
* SetFocus
*
\**************************************************************************/
void
CMpegMovie::SetFocus()
{
    if(m_Fg)
    {
        // Tell the resource manager that we are being made active.  This
        // will then cause the sound to switch to us.  This is especially
        // important when playing audio only files as there is no other
        // playback window.
        IResourceManager* pResourceManager;

        HRESULT hr = m_Fg->QueryInterface(IID_IResourceManager, (void**)&pResourceManager);

        if(SUCCEEDED(hr))
        {
            IUnknown* pUnknown;

            hr = m_Fg->QueryInterface(IID_IUnknown, (void**)&pUnknown);

            if(SUCCEEDED(hr))
            {
                pResourceManager->SetFocus(pUnknown);
                pUnknown->Release();
            }

            pResourceManager->Release();
        }
    }
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

    if(m_Wc)
    {
        bRet = (m_Wc->RepaintVideo(hwnd, hdc) == S_OK);
    }

    return bRet;
}


/******************************Public*Routine******************************\
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



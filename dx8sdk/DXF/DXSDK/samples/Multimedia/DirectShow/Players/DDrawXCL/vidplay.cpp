//------------------------------------------------------------------------------
// File: VidPlay.cpp
//
// Desc: DirectShow sample code - video (DVD and file) playback class.
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <windows.h>

#include "mpconfig.h"
#include "VidPlay.h"


//
// CBaseVideoPlayer constructor
//
CBaseVideoPlayer::CBaseVideoPlayer()
{
    m_eState = Uninitialized ;
    ZeroMemory(m_achFileName, sizeof(m_achFileName)) ;
    
    m_pGraph = NULL ;
    m_pMC = NULL ;
    m_pME = NULL ;
    
    m_dwColorKey = 253 ;  // magenta by default
}


//
// CBaseVideoPlayer destructor
//
CBaseVideoPlayer::~CBaseVideoPlayer()
{
    ReleaseInterfaces() ;
}


//
// CBaseVideoPlayer::GetInterfaces(): Gets the standard interfaces required for
// playing back through a filter graph
//
HRESULT CBaseVideoPlayer::GetInterfaces(HWND hWndApp)
{
    HRESULT  hr ;
    hr = m_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMC) ;
    ASSERT(SUCCEEDED(hr) && m_pMC) ;
    
    hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (LPVOID *)&m_pME) ;
    ASSERT(SUCCEEDED(hr) && m_pME) ;
    
    //
    // Also set up the event notification so that the main window gets
    // informed about all that we care about during playback.
    //
    hr = m_pME->SetNotifyWindow((OAHWND) hWndApp, WM_PLAY_EVENT, (LONG_PTR) m_pME) ;
    ASSERT(SUCCEEDED(hr)) ;
    
    m_eState = Stopped ;  // graph has been built
    
    return S_OK ;
}


//
// CBaseVideoPlayer::ReleaseInterfaces(): Let go of the standard interfaces taken for playback.
//
void CBaseVideoPlayer::ReleaseInterfaces(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CBaseVideoPlayer::ReleaseInterfaces() entered"))) ;

    if (m_pMC) {
        m_pMC->Release() ;
        m_pMC = NULL ;
    }
    if (m_pME) {
        // clear any already set notification arrangement
        m_pME->SetNotifyWindow(NULL, WM_PLAY_EVENT, (LONG_PTR) m_pME) ;
        m_pME->Release() ;
        m_pME = NULL ;
    }
    
    m_eState = Uninitialized ;  // no graph available
}


//
// CBaseVideoPlayer::WaitForState(): Wait for the state to change to the given one.
//
void CBaseVideoPlayer::WaitForState(FILTER_STATE State)
{
    // Make sure we have switched to the required state
    LONG   lfs ;
    do
    {
        m_pMC->GetState(10, &lfs) ;
    } while (State != lfs) ;
}


//
// CBaseVideoPlayer::Play(): Plays the filter graph (and waits to really start playing)
//
BOOL CBaseVideoPlayer::Play(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CBaseVideoPlayer::Play() entered"))) ;
    
    if (! IsGraphReady() )
    {
        DbgLog((LOG_TRACE, 1, TEXT("DVD-Video playback graph hasn't been built yet"))) ;
        return FALSE ;
    }
    HRESULT  hr = m_pMC->Run() ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: IMediaControl::Run() failed (Error 0x%lx)"), hr)) ;
        return FALSE ;
    }
    WaitForState(State_Running) ;
    
    // Some state changes now
    m_eState = Playing ;
    
    return TRUE ;  // success
}


//
// CBaseVideoPlayer::Pause(): Pauses/Cues the filter graph (and waits to really pause)
//
BOOL CBaseVideoPlayer::Pause(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CBaseVideoPlayer::Pause() entered"))) ;
    
    if (! IsGraphReady() )
    {
        DbgLog((LOG_TRACE, 1, TEXT("DVD-Video playback graph hasn't been built yet"))) ;
        return FALSE ;
    }
    HRESULT  hr = m_pMC->Pause() ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: IMediaControl::Pause() failed (Error 0x%lx)"), hr)) ;
        return FALSE ;
    }
    WaitForState(State_Paused) ;
    
    // Some state changes now
    m_eState = Paused ;
    
    return TRUE ;  // success
}


//
// CBaseVideoPlayer::Stop(): Stops the filter graph (and waits to really stop)
//
BOOL CBaseVideoPlayer::Stop(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CBaseVideoPlayer::Stop() entered"))) ;
    
    if (! IsGraphReady() )
    {
        DbgLog((LOG_TRACE, 1, TEXT("DVD-Video playback graph hasn't been built yet"))) ;
        return FALSE ;
    }
    HRESULT  hr = m_pMC->Stop() ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: IMediaControl::Stop() failed (Error 0x%lx)"), hr)) ;
        return FALSE ;
    }
    WaitForState(State_Stopped) ;
    
    // Some state changes now
    m_eState = Stopped ;
    
    return TRUE ;  // success
}


//
// CBaseVideoPlayer::GetColorKey(): Returns color key to the app so that other 
// components (here the DDraw object) can be told about it.
//
HRESULT CBaseVideoPlayer::GetColorKey(DWORD *pdwColorKey)
{
    DbgLog((LOG_TRACE, 5, TEXT("CBaseVideoPlayer::GetColorKey() entered"))) ;
    
    if (! IsGraphReady() )
        return E_UNEXPECTED ;
    
    *pdwColorKey = m_dwColorKey ;  // use the cached color key info
    return S_OK ;
}



//
// CDVDPlayer constructor
//
CDVDPlayer::CDVDPlayer()
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVDPlayer c-tor entered"))) ;
    
    m_pDvdGB = NULL ;
    m_pDvdC  = NULL ;
    m_pDvdI  = NULL ;
}


//
// CDVDPlayer destructor
//
CDVDPlayer::~CDVDPlayer()
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVDPlayer d-tor entered"))) ;
    
    ReleaseInterfaces() ;
    
    if (m_pDvdGB)
        m_pDvdGB->Release() ;
    
    DbgLog((LOG_TRACE, 3, TEXT("CDVDPlayer d-tor exiting..."))) ;
}


//
// CDVDPlayer::Initialize(): Creates a DVD graph builder object for later building
// a filter graph.
//
BOOL CDVDPlayer::Initialize(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::Initialize() entered"))) ;
    
    //
    // Now instantiate the DVD Graph Builder object and start working
    //
    HRESULT hr = CoCreateInstance(CLSID_DvdGraphBuilder, NULL, CLSCTX_INPROC, 
        IID_IDvdGraphBuilder, (LPVOID *)&m_pDvdGB) ;
    if (FAILED(hr) || NULL == m_pDvdGB)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: DirectShow DVD software not installed properly (Error 0x%lx)"), hr)) ;
        return FALSE ;
    }
    
    return TRUE;
}


//
// CDVDPlayer::BuildGraph(): Builds a filter graph for playing back the specified
// DVD title/file.  Also gets some interfaces that are required for controlling 
// playback.
//
HRESULT CDVDPlayer::BuildGraph(HWND hWndApp, LPDIRECTDRAW pDDObj, LPDIRECTDRAWSURFACE pDDPrimary)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::BuildGraph() entered"))) ;

    HRESULT     hr ;
    
    // First release any existing interface pointer(s)
    ReleaseInterfaces() ;
    SetColorKey(253) ;  // default magenta for 8bpp
    
    // Check if a DVD-Video volume name has been specified; if so, use that
    WCHAR    achwFileName[MAX_PATH] ;
    LPCWSTR  lpszwFileName = NULL ;  // by default
    if (lstrlen(GetFileName()) > 0)  // if something was specified before
    {
#ifdef UNICODE
        lstrcpy(achwFileName, GetFileName()) ;
#else
        MultiByteToWideChar(CP_ACP, 0, GetFileName(), -1, achwFileName, MAX_PATH) ;
#endif // UNICODE
        
        lpszwFileName = achwFileName ;
    }
    DbgLog((LOG_TRACE, 5, TEXT("DVD file <%s> will be played"), GetFileName())) ;
    
    // Set DDraw object and surface on DVD graph builder before starting to build graph
    IDDrawExclModeVideo  *pDDXMV ;
    hr = m_pDvdGB->GetDvdInterface(IID_IDDrawExclModeVideo, (LPVOID *)&pDDXMV) ;
    if (FAILED(hr) || NULL == pDDXMV)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: IDvdGB::GetDvdInterface(IDDrawExclModeVideo) failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    hr = pDDXMV->SetDDrawObject(pDDObj) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: IDDrawExclModeVideo::SetDDrawObject() failed (Error 0x%lx)"), hr)) ;
        pDDXMV->Release() ;  // release before returning
        return hr ;
    }
    hr = pDDXMV->SetDDrawSurface(pDDPrimary) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: IDDrawExclModeVideo::SetDDrawSurface() failed (Error 0x%lx)"), hr)) ;
        pDDXMV->SetDDrawObject(NULL) ;  // to reset
        pDDXMV->Release() ;  // release before returning
        return hr ;
    }
    pDDXMV->Release() ;  // done with the interface
    
    // Build the graph
    AM_DVD_RENDERSTATUS   Status ;
    TCHAR                 achBuffer[1000] ;
    hr = m_pDvdGB->RenderDvdVideoVolume(lpszwFileName,
        AM_DVD_HWDEC_PREFER, &Status) ;
    if (FAILED(hr))
    {
        AMGetErrorText(hr, achBuffer, sizeof(achBuffer)/sizeof(achBuffer[0])) ;
        MessageBox(hWndApp, achBuffer, TEXT("Error"), MB_OK) ;
        return hr ;
    }
    if (S_FALSE == hr)  // if partial success
    {
        if (0 == GetStatusText(&Status, achBuffer, sizeof(achBuffer)))
        {
            lstrcpy(achBuffer, TEXT("An unknown error has occurred")) ;
        }
        lstrcat(achBuffer, TEXT("\n\nDo you still want to continue?")) ;
        if (IDNO == MessageBox(hWndApp, achBuffer, TEXT("Warning"), MB_YESNO))
        {
            return E_FAIL ;
        }
    }
    
    GetInterfaces(hWndApp) ;
    
    hr = GetColorKeyInternal() ;
    ASSERT(SUCCEEDED(hr)) ;

    return S_OK ;
}


//
// CDVDPlayer::GetInterfaces(): Gets some interfaces useful for DVD title playback
//
HRESULT CDVDPlayer::GetInterfaces(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::GetInterfaces() entered"))) ;

    HRESULT  hr ;
    hr = m_pDvdGB->GetFiltergraph(&m_pGraph) ;
    ASSERT(SUCCEEDED(hr) && m_pGraph) ;
    
    // Now get the DVD-specific interfaces (though we don't use them in this app).
    hr = m_pDvdGB->GetDvdInterface(IID_IDvdControl2, (LPVOID *)&m_pDvdC) ;
    ASSERT(SUCCEEDED(hr) && m_pDvdC) ;
    
    hr = m_pDvdGB->GetDvdInterface(IID_IDvdInfo2, (LPVOID *)&m_pDvdI) ;
    ASSERT(SUCCEEDED(hr) && m_pDvdI) ;
    
    return CBaseVideoPlayer::GetInterfaces(hWndApp) ;  // get the standard interfaces
}


//
// CDVDPlayer::ClearGraph(): Tears down the filter graph created by BuildGraph()
// and also releases the DVD graph builder object.
//
HRESULT CDVDPlayer::ClearGraph()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::ClearGraph() entered"))) ;
    
    ReleaseInterfaces() ;  // let go of the other interfaces
    if (m_pDvdGB)
    {
        m_pDvdGB->Release() ;
        m_pDvdGB = NULL ;
    }
    
    return S_OK ;
    
}


//
// CDVDPlayer::GetColorKeyInternal(): Retrieves color key and stores it for later 
// queries from the app.
//
HRESULT CDVDPlayer::GetColorKeyInternal(IBaseFilter *pOvM /* = NULL */)
{
    ASSERT(NULL == pOvM) ; // we don't need pOvM passed in
    
    // Get color key from MixerPinConfig interface via IDvdGraphBuilder
    IMixerPinConfig2  *pMPC ;
    DWORD              dwColorKey ;
    HRESULT  hr = m_pDvdGB->GetDvdInterface(IID_IMixerPinConfig2, (LPVOID *) &pMPC) ;
    if (SUCCEEDED(hr))
    {
        hr = pMPC->GetColorKey(NULL, &dwColorKey) ;
        ASSERT(SUCCEEDED(hr)) ;
        SetColorKey(dwColorKey) ;

        //  Set mode to stretch - that way we don't fight the overlay
        //  mixer about the exact way to fix the aspect ratio
        pMPC->SetAspectRatioMode(AM_ARMODE_STRETCHED);
        pMPC->Release() ;
    }
    else
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("WARNING: IDvdGraphBuilder::GetDvdInterface(IID_IMixerPinConfig2) failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    return S_OK ;
}


//
// CDVDPlayer::GetNativeVideoData(): Retrieves the original video size from OverlayMixer's
// IDDrawExclModeVideo interface.
//
HRESULT CDVDPlayer::GetNativeVideoData(DWORD *pdwWidth, DWORD *pdwHeight, DWORD *pdwARX, DWORD *pdwARY)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::GetNativeVideoData() entered"))) ;
    
    IDDrawExclModeVideo  *pDDXMV ;
    HRESULT hr = m_pDvdGB->GetDvdInterface(IID_IDDrawExclModeVideo, (LPVOID *)&pDDXMV) ;
    if (FAILED(hr) || NULL == pDDXMV)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: IDvdGB::GetDvdInterface(IDDrawExclModeVideo) failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    hr = pDDXMV->GetNativeVideoProps(pdwWidth, pdwHeight, pdwARX, pdwARY) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, 
            TEXT("WARNING: IDDrawExclModeVideo::GetNativeVideoProps() failed (Error 0x%lx)"), hr)) ;
    }
    
    pDDXMV->Release() ;  // release before returning
    
    return hr ;
}


//
// CDVDPlayer::SetVideoPosition(): Sets the source and destination rects on the 
// OverlayMixer's IDDrawExclModeVideo interface .
//
HRESULT CDVDPlayer::SetVideoPosition(DWORD dwLeft, DWORD dwTop, DWORD dwWidth, DWORD dwHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::SetVideoPosition() entered"))) ;
    
    IDDrawExclModeVideo  *pDDXMV ;
    HRESULT hr = m_pDvdGB->GetDvdInterface(IID_IDDrawExclModeVideo, (LPVOID *)&pDDXMV) ;
    if (FAILED(hr) || NULL == pDDXMV)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: IDvdGB::GetDvdInterface(IDDrawExclModeVideo) failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    
    RECT  rectDest ;
    SetRect(&rectDest, dwLeft, dwTop, dwLeft + dwWidth, dwTop + dwHeight) ;
    // NULL or (0, 0, 10000, 10000) for full video
    hr = pDDXMV->SetDrawParameters(NULL, &rectDest) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, 
            TEXT("WARNING: IDDrawExclModeVideo::SetDrawParameters() failed (Error 0x%lx)"), hr)) ;
    }
    
    pDDXMV->Release() ;  // release before returning
    
    return hr ;
}


//
// CDVDPlayer::SetOverlayCallback(): Specify the pointer to the overlay notification
// callback object which the OverlayMixer will call during playback.
//
HRESULT CDVDPlayer::SetOverlayCallback(IDDrawExclModeVideoCallback *pCallback)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::SetOverlayCallback() entered"))) ;

    HRESULT  hr ;
    IDDrawExclModeVideo  *pDDXMV ;
    hr = m_pDvdGB->GetDvdInterface(IID_IDDrawExclModeVideo, (LPVOID *)&pDDXMV) ;
    if (FAILED(hr) || NULL == pDDXMV)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: IDvdGB::GetDvdInterface(IDDrawExclModeVideo) failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    hr = pDDXMV->SetCallbackInterface(pCallback, 0) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: IDDrawExclModeVideo::SetCallbackInterface() failed (Error 0x%lx)"), hr)) ;
    }
    pDDXMV->Release() ;  // done with the interface

    return hr ;
}


//
// CDVDPlayer::ReleaseInterfaces(): Let go of the interfaces taken in BuildGraph()
//
void CDVDPlayer::ReleaseInterfaces(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::ReleaseInterfaces() entered"))) ;
    
    if (m_pDvdC) {
        m_pDvdC->Release() ;
        m_pDvdC = NULL ;
    }
    if (m_pDvdI) {
        m_pDvdI->Release() ;
        m_pDvdI = NULL ;
    }
    if (m_pGraph) {
        m_pGraph->Release() ;
        m_pGraph = NULL ;
    }
    CBaseVideoPlayer::ReleaseInterfaces() ;  // release stanadrd interfaces
}


//
// CDVDPlayer::GetStatusText(): Composes a status message on failure to build DVD graph.
//
DWORD CDVDPlayer::GetStatusText(AM_DVD_RENDERSTATUS *pStatus,
                                LPTSTR lpszStatusText,
                                DWORD dwMaxText)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDPlayer::GetStatusText() entered"))) ;
    
    TCHAR    achBuffer[1000] ;
    
    if (IsBadWritePtr(lpszStatusText, sizeof(*lpszStatusText) * dwMaxText))
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetStatusText(): bad text buffer param"))) ;
        return 0 ;
    }
    
    int    iChars ;
    LPTSTR lpszBuff = achBuffer ;
    ZeroMemory(achBuffer, sizeof(TCHAR) * 1000) ;
    if (pStatus->iNumStreamsFailed > 0)
    {
        iChars = wsprintf(lpszBuff, 
            TEXT("* %d out of %d DVD-Video streams failed to render properly\n"), 
            pStatus->iNumStreamsFailed, pStatus->iNumStreams) ;
        lpszBuff += iChars ;
        
        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_VIDEO)
        {
            iChars = wsprintf(lpszBuff, TEXT("    - video stream\n")) ;
            lpszBuff += iChars ;
        }
        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_AUDIO)
        {
            iChars = wsprintf(lpszBuff, TEXT("    - audio stream\n")) ;
            lpszBuff += iChars ;
        }
        if (pStatus->dwFailedStreamsFlag & AM_DVD_STREAM_SUBPIC)
        {
            iChars = wsprintf(lpszBuff, TEXT("    - subpicture stream\n")) ;
            lpszBuff += iChars ;
        }
    }
    
    if (FAILED(pStatus->hrVPEStatus))
    {
        lstrcat(lpszBuff, TEXT("* ")) ;
        lpszBuff += lstrlen(TEXT("* ")) ;
        iChars = AMGetErrorText(pStatus->hrVPEStatus, lpszBuff, 200) ;
        lpszBuff += iChars ;
        lstrcat(lpszBuff, TEXT("\n")) ;
        lpszBuff += lstrlen(TEXT("\n")) ;
    }
    
    if (pStatus->bDvdVolInvalid)
    {
        iChars = wsprintf(lpszBuff, TEXT("* Specified DVD-Video volume was invalid\n")) ;
        lpszBuff += iChars ;
    }
    else if (pStatus->bDvdVolUnknown)
    {
        iChars = wsprintf(lpszBuff, TEXT("* No valid DVD-Video volume could be located\n")) ;
        lpszBuff += iChars ;
    }
    
    if (pStatus->bNoLine21In)
    {
        iChars = wsprintf(lpszBuff, TEXT("* The video decoder doesn't produce closed caption data\n")) ;
        lpszBuff += iChars ;
    }
    if (pStatus->bNoLine21Out)
    {
        iChars = wsprintf(lpszBuff, TEXT("* Decoded closed caption data not rendered properly\n")) ;
        lpszBuff += iChars ;
    }
    
    DWORD dwLength = (lpszBuff - achBuffer) * sizeof(*lpszBuff) ;
    dwLength = min(dwLength, dwMaxText) ;
    lstrcpyn(lpszStatusText, achBuffer, dwLength) ;
    
    return dwLength ;
}



//
// CFilePlayer constructor
//
CFilePlayer::CFilePlayer(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CFilePlayer c-tor entered"))) ;
    
    m_pDDXM = NULL ;
}


//
// CFilePlayer destructor
//
CFilePlayer::~CFilePlayer(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CFilePlayer d-tor entered"))) ;
    
    ReleaseInterfaces() ;
    
    DbgLog((LOG_TRACE, 3, TEXT("CFilePlayer d-tor exiting..."))) ;
}


//
// CFilePlayer::ReleaseInterfaces(): Let go of the interfaces taken in BuildGraph()
//
void CFilePlayer::ReleaseInterfaces(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::ReleaseInterfaces() entered"))) ;
    
    if (m_pDDXM) {
        m_pDDXM->Release() ;
        m_pDDXM = NULL ;
    }
    CBaseVideoPlayer::ReleaseInterfaces() ;
}


//
// CFilePlayer::Initialize(): Creates an empty filter graph object.
//
BOOL CFilePlayer::Initialize(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::Initialize() entered"))) ;
    
    //
    // Just instantiate the Filter Graph object
    //
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
        IID_IGraphBuilder, (LPVOID *)&m_pGraph) ;
    if (FAILED(hr) || NULL == m_pGraph)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: DirectShow is not installed properly (Error 0x%lx)"), hr)) ;
        return FALSE ;
    }
    
    return TRUE;
}


//
// CFilePlayer::IsOvMConnected(): Private method to detect if the video stream 
// is passing through the Overlay Mixer (i.e, is it connected?).
//
BOOL CFilePlayer::IsOvMConnected(IBaseFilter *pOvM)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::IsOvMConnected() entered"))) ;
    
    IEnumPins   *pEnumPins ;
    IPin        *pPin ;
    IPin        *pPin2 ;
    ULONG        ul ;
    HRESULT      hr ;
    BOOL         bConnected = FALSE ;

    pOvM->EnumPins(&pEnumPins) ;
    while (S_OK == pEnumPins->Next(1, &pPin, &ul) && 1 == ul)
    {
        hr = pPin->ConnectedTo(&pPin2) ;
        if (SUCCEEDED(hr) && pPin2)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Found pin %s connected to pin %s"), 
                    (LPCTSTR)CDisp(pPin), (LPCTSTR)CDisp(pPin2))) ;
            bConnected = TRUE ;
            pPin2->Release() ;
        }
        pPin->Release() ;
    }
    pEnumPins->Release() ;
    
    return bConnected ;
}



//
// CFilePlayer::GetVideoRendererInterface(): Private method to get the interface
// to the Video Rendererer filter. It also tells us if a file has any video stream.
//
HRESULT CFilePlayer::GetVideoRendererInterface(IBaseFilter **ppVR)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::IsVideoStreamPresent() entered"))) ;
    
    // Get Video Renderer filter pointer from the graph
    return m_pGraph->FindFilterByName(L"Video Renderer", ppVR) ;
}



//
// CFilePlayer::AddOvMToGraph(): Private method to instantiate OverlayMixer,
// add it to the filter graph and set the DDraw parameters specified by the
// app..
//
HRESULT CFilePlayer::AddOvMToGraph(IBaseFilter **ppOvM, LPDIRECTDRAW pDDObj, 
                                   LPDIRECTDRAWSURFACE pDDPrimary)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::AddOvMToGraph() entered"))) ;
    
    HRESULT  hr ;
    
    hr = CoCreateInstance(CLSID_OverlayMixer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)ppOvM) ;
    if (FAILED(hr) || NULL == *ppOvM)
    {
        DbgLog((LOG_ERROR, 0, TEXT("Can't instantiate Overlay Mixer (Error 0x%lx)"), hr)) ;
        return E_FAIL ;
    }
    hr = m_pGraph->AddFilter(*ppOvM, L"Overlay Mixer") ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Can't add Overlay Mixer to the graph (Error 0x%lx)"), hr)) ;
        (*ppOvM)->Release() ;
        *ppOvM = NULL ;
        return E_FAIL ;
    }
    
    // Set the DDraw params now
    hr = SetDDrawParams(*ppOvM, pDDObj, pDDPrimary) ;
    if (FAILED(hr))
    {
        m_pGraph->RemoveFilter(*ppOvM) ;  // remove from graph
        (*ppOvM)->Release() ;
        *ppOvM = NULL ;
        return hr ;
    }
    
    return S_OK ;
}


//
// CFilePlayer::SetDDrawParams(): Private method to set DDraw objeact and primary
// surface on the OverlayMixer via the IDDrawExclModeVideo interface.
//
HRESULT CFilePlayer::SetDDrawParams(IBaseFilter *pOvM, LPDIRECTDRAW pDDObj, 
                                    LPDIRECTDRAWSURFACE pDDPrimary)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::SetDDrawParams() entered"))) ;
    
    HRESULT                 hr ;
    IDDrawExclModeVideo    *pDDXM ;
    
    hr = pOvM->QueryInterface(IID_IDDrawExclModeVideo, (LPVOID *)&pDDXM) ;
    if (FAILED(hr) || NULL == pDDXM)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("Can't get IDDrawExclusiveMode interface from Overlay Mixer (Error 0x%lx)"), hr)) ;
        return E_FAIL ;
    }
    hr = pDDXM->SetDDrawObject(pDDObj) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("SetDDrawObject(0x%lx) failed (Error 0x%lx)"), pDDObj, hr)) ;
        pDDXM->Release() ;               // release interface
        return E_FAIL ;
    }
    hr = pDDXM->SetDDrawSurface(pDDPrimary) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("SetDDrawSurface(0x%lx) failed (Error 0x%lx)"), pDDPrimary, hr)) ;
        pDDXM->SetDDrawObject(NULL) ;    // get back to original DDraw object
        pDDXM->Release() ;               // release interface
        return E_FAIL ;
    }
    
    pDDXM->Release() ;               // release interface
    
    return S_OK ;
}


//
// CFilePlayer::PutVideoThroughOvM(): Private method to connect the decoded video 
// stream through the OverlayMixer.
//
HRESULT CFilePlayer::PutVideoThroughOvM(IBaseFilter *pOvM, IBaseFilter *pVR)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::PutVideoThroughOvM() entered"))) ;
    
    IEnumPins     *pEnumPins ;
    IPin          *pPinVR ;
    IPin          *pPinConnectedTo ;
    IPin          *pPinOvM ;
    ULONG          ul ;
    PIN_DIRECTION  pd ;
    HRESULT        hr ;
    
    // Get the video renderer's (only) in pin
    hr = pVR->EnumPins(&pEnumPins) ;
    ASSERT(SUCCEEDED(hr)) ;
    hr = pEnumPins->Next(1, &pPinVR, &ul) ;
    ASSERT(S_OK == hr  &&  1 == ul)  ;
    pPinVR->QueryDirection(&pd) ;
    ASSERT(PINDIR_INPUT == pd) ;
    pEnumPins->Release() ;
    
    // Disconnect VR from upstream filter and put OvMixer in between them
    hr = pPinVR->ConnectedTo(&pPinConnectedTo) ;
    ASSERT(SUCCEEDED(hr) && pPinConnectedTo) ;
    hr = m_pGraph->Disconnect(pPinVR) ;
    ASSERT(SUCCEEDED(hr)) ;
    hr = m_pGraph->Disconnect(pPinConnectedTo) ;
    ASSERT(SUCCEEDED(hr)) ;
    
    // Now connect the upstream filter's out pin to OvM's in pin 
    // (and remove Video Renderer from the graph).
    hr = pOvM->EnumPins(&pEnumPins) ;
    ASSERT(SUCCEEDED(hr)) ;
    int  iInConns  = 0 ;
    while (iInConns == 0  &&   // input pin not connected yet
        S_OK == pEnumPins->Next(1, &pPinOvM, &ul) && 1 == ul)  // pin left to try
    {
        pPinOvM->QueryDirection(&pd) ;
        if (PINDIR_INPUT == pd)  // OvM's in pin <- Upstream filter's pin
        {
            if (0 == iInConns) // We want to connect only one input pin
            {
                hr = m_pGraph->Connect(pPinConnectedTo, pPinOvM) ; // , NULL) ;  // direct??
                ASSERT(SUCCEEDED(hr)) ;
                if (FAILED(hr))
                    DbgLog((LOG_ERROR, 0, TEXT("ERROR: m_pGraph->Connect() failed (Error 0x%lx)"), hr)) ;
                else
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Pin %s connected to pin %s"), 
                        (LPCTSTR) CDisp(pPinConnectedTo), (LPCTSTR) CDisp(pPinOvM))) ;
                    iInConns++ ;
                }
            }
            // else ignore
        }
        else
            DbgLog((LOG_ERROR, 1, TEXT("OVMixer still has an out pin!!!"))) ;
        
        pPinOvM->Release() ;
    }
    pEnumPins->Release() ;
    
    pPinConnectedTo->Release() ;  // done with upstream pin
    pPinVR->Release() ; // done with VR pin
    m_pGraph->RemoveFilter(pVR) ;  // by force remove the VR from graph

    // Just to check...
    if (0 == iInConns)  // input pin not connected!!
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Couldn't connect any out pin to OvMixer's 1st in pin"))) ;
        return E_FAIL ;    // failure
    }
    
    return S_OK ;          // success!!
}


//
// CFilePlayer::BuildGraph(): Builds a AVI/MPEG/.. playback graph rendering via
// OverlayMixer which uses app's given DDraw params.
//
HRESULT CFilePlayer::BuildGraph(HWND hWndApp, LPDIRECTDRAW pDDObj, LPDIRECTDRAWSURFACE pDDPrimary)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::BuildGraph() entered"))) ;
    
    HRESULT       hr ;
    IBaseFilter  *pOvM ;
    WCHAR         achwFileName[MAX_PATH] ;
    LPWSTR        lpszwFileName = NULL ;
    IBaseFilter  *pVR ;
    
    // First release any existing interface pointer(s)
    ReleaseInterfaces() ;
    SetColorKey(253) ;  // default magenta for 8bpp
    
    // Check if a file name has been specified; if so, use that
    if (lstrlen(GetFileName()) > 0)  // if something was specified before
    {
#ifdef UNICODE
        lstrcpy(achwFileName, GetFileName()) ;
#else
        MultiByteToWideChar(CP_ACP, 0, GetFileName(), -1, achwFileName, MAX_PATH) ;
#endif // UNICODE
        
        lpszwFileName = achwFileName ;
    }
    else  // no file specified, but we should have detected it before!!!
        return E_FAIL ;
    
    //
    // Instantiate Overlay Mixer, add it to the graph and set DDraw params
    //
    hr = AddOvMToGraph(&pOvM, pDDObj, pDDPrimary) ;
    if (FAILED(hr))
    {
        return E_FAIL ;
    }
    
    //
    // First render the graph for the selected file
    //
    hr = m_pGraph->RenderFile(lpszwFileName, NULL) ;
    if (S_OK != hr)
    {
        DbgLog((LOG_ERROR, 0, 
            TEXT("Rendering the given file didn't succeed completely (Error 0x%lx)"), hr)) ;
        m_pGraph->RemoveFilter(pOvM) ;  // remove from graph
        pOvM->Release() ;               // release filter
        return E_FAIL ;
    }

    //
    // Because there are some AVI files which on some machines decide to rather go 
    // through the Color Space Converter filter, just making sure that the OverlayMixer
    // is actually being used.  Otherwise we have to do some more (bull)work.
    //
    if (! IsOvMConnected(pOvM) )
    {
        DbgLog((LOG_TRACE, 1, TEXT("OverlayMixer is not used in the graph. Try again..."))) ;
    
        //
        // Check that the  specified file has a video stream. Otherwise OverlayMixer
        // will never be used and DDraw exclusive mode playback doesn't make any sense.
        //
        if (FAILED(GetVideoRendererInterface(&pVR)))
        {
            DbgLog((LOG_TRACE, 1, TEXT("Specified file doesn't have any video stream. Aborting graph building."))) ;
            m_pGraph->RemoveFilter(pOvM) ;  // remove from graph
            pOvM->Release() ;               // release filter
            return E_FAIL ;
        }

        //
        // Put the video stream to go through the OverlayMixer.
        //
        hr = PutVideoThroughOvM(pOvM, pVR) ;
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE, 1, TEXT("Couldn't put video through the OverlayMixer."))) ;
            m_pGraph->RemoveFilter(pOvM) ;  // remove OvMixer from graph
            pOvM->Release() ;               // release OvMixer filter
            pVR->Release() ;                // release VR interface (before giving up)
            return E_FAIL ;
        }
        pVR->Release() ;    // done with VR interface
    }

    //
    // We are successful in building the graph. Now the rest...
    //
    GetInterfaces(hWndApp) ;
    
    // Get IDDrawExclModeVideo interface of the OvMixer and store it
    hr = pOvM->QueryInterface(IID_IDDrawExclModeVideo, (LPVOID *)&m_pDDXM) ;
    ASSERT(SUCCEEDED(hr)) ;
    
    // Get the color key to be used and store it
    hr = GetColorKeyInternal(pOvM) ;
    ASSERT(SUCCEEDED(hr)) ;
    
    pOvM->Release() ;  // done with it
    
    return S_OK ;
}


//
// CFilePlayer::GetInterfaces(): Gets some interfaces useful for DVD title playback
//
// NOTE: Don't really need this one.  We could have as well used the base classes'.
//
HRESULT CFilePlayer::GetInterfaces(HWND hWndApp)
{
    return CBaseVideoPlayer::GetInterfaces(hWndApp) ;  // get the standard interfaces
}

//
// CFilePlayer::GetColorKeyInternal(): Private method to query the color key
// value from teh first input pin of the OverlayMixer.
//
HRESULT CFilePlayer::GetColorKeyInternal(IBaseFilter *pOvM)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::GetColorKeyInternal() entered"))) ;
    
    if (NULL == pOvM)
        return E_INVALIDARG ;
    
    IEnumPins  *pEnumPins ;
    IPin       *pPin ;
    ULONG       ul ;
    PIN_DIRECTION  pd ;
    DWORD       dwColorKey ;
    IMixerPinConfig  *pMPC ;
    HRESULT  hr = pOvM->EnumPins(&pEnumPins) ;
    ASSERT(pEnumPins) ;
    while (S_OK == pEnumPins->Next(1, &pPin, &ul)  &&  1 == ul)  // try all pins
    {
        pPin->QueryDirection(&pd) ;
        if (PINDIR_INPUT == pd)  // only the 1st in pin
        {
            hr = pPin->QueryInterface(IID_IMixerPinConfig, (LPVOID *) &pMPC) ;
            ASSERT(SUCCEEDED(hr) && pMPC) ;
            hr = pMPC->GetColorKey(NULL, &dwColorKey) ;  // just get the physical color
            SetColorKey(dwColorKey) ;
            //  Set mode to stretch - that way we don't fight the overlay
            //  mixer about the exact way to fix the aspect ratio
            pMPC->SetAspectRatioMode(AM_ARMODE_STRETCHED);
            ASSERT(SUCCEEDED(hr)) ;
            pMPC->Release() ;
            pPin->Release() ; // exiting early; release pin
            break ;   // we are done
        }
        pPin->Release() ;
    }
    pEnumPins->Release() ;  // done with pin enum
    
    return S_OK ;
}


//
// CFilePlayer::ClearGraph(): Releases the filter graph built by BuildGraph()
// and the interfaces taken for playback.
//
HRESULT CFilePlayer::ClearGraph(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::ClearGraph() entered"))) ;
    
    ReleaseInterfaces() ;  // let go of the other interfaces
    if (m_pGraph)
    {
        m_pGraph->Release() ;  // let the graph go (we'll create a fresh one next time)
        m_pGraph = NULL ;
    }
    
    return S_OK ;
}

//
// CFilePlayer::SetVideoPosition(): Sets the source and dest rects for the video to
// be shown via IDDrawExclModeVideo interface of OverlayMixer.
//
HRESULT CFilePlayer::SetVideoPosition(DWORD dwLeft, DWORD dwTop, DWORD dwWidth, DWORD dwHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::SetVideoPosition() entered"))) ;
    
    RECT  rectDest ;
    SetRect(&rectDest, dwLeft, dwTop, dwLeft + dwWidth, dwTop + dwHeight) ;
    
    // NULL or (0, 0, 10000, 10000) for full video
    return m_pDDXM->SetDrawParameters(NULL, &rectDest) ;
}


//
// CFilePlayer::GetNativeVideoData(): Returns the actual video size to the app.
//
HRESULT CFilePlayer::GetNativeVideoData(DWORD *pdwWidth, DWORD *pdwHeight, DWORD *pdwARX, DWORD *pdwARY)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::GetNativeVideoData() entered"))) ;
    
    return m_pDDXM->GetNativeVideoProps(pdwWidth, pdwHeight, pdwARX, pdwARY) ;
}


//
// CFilePlayer::SetOverlayCallback(): Specify the pointer to the overlay notification
// callback object which the OverlayMixer will call during playback.
//
HRESULT CFilePlayer::SetOverlayCallback(IDDrawExclModeVideoCallback *pCallback)
{
    DbgLog((LOG_TRACE, 5, TEXT("CFilePlayer::SetOverlayCallback() entered"))) ;

    HRESULT  hr ;
    hr = m_pDDXM->SetCallbackInterface(pCallback, 0) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: IDDrawExclModeVideo::SetCallbackInterface() failed (Error 0x%lx)"), hr)) ;
    }

    return hr ;
}



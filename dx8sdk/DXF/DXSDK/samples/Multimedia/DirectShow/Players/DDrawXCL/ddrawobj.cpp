//------------------------------------------------------------------------------
// File: DDrawObj.cpp
//
// Desc: DirectShow sample code - implementation of DDrawObject class.
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <windows.h>

#include "DDrawObj.h"
#include "VidPlay.h"


//
// CDDrawObject constructor
//
CDDrawObject::CDDrawObject(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDDrawObject c-tor entered"))) ;
    
    m_pDDObject = NULL ;
    m_pPrimary = NULL ;
    m_pBackBuff = NULL ;
    
    // Default colors to be used for filling
    m_dwScrnColor = RGB(0, 0, 0) ;
    m_dwVideoKeyColor = RGB(255, 0, 255) ;
    
    // Rects for the screen area and video area
    SetRect(&m_RectScrn, 0, 0, 0, 0) ;
    SetRect(&m_RectVideo, 0, 0, 0, 0) ;
    
    // some flags, count, messages for buffer flipping
    m_bInExclMode = FALSE ;
    m_iCount = 0 ;
    m_bFrontBuff = TRUE ;
    m_szFrontMsg = TEXT("Front Buffer" );
    m_szBackMsg  = TEXT("Back Buffer" );
    m_szDirection = TEXT("Press the 'Arrow' keys to move the ball. Hit 'Esc' to stop playback." );
    
    // Create brush and pen for drawing the filled ball
    LOGBRUSH  lb ;
    lb.lbStyle = BS_HATCHED ;
    lb.lbColor = RGB(0, 255, 0) ;
    lb.lbHatch = HS_CROSS ;
    m_hBrush = CreateBrushIndirect(&lb) ;
    ASSERT(NULL != m_hBrush) ;
    
    m_hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255)) ;
    ASSERT(NULL != m_hPen) ;

    HRESULT  hr=S_OK ;
    m_pOverlayCallback = (IDDrawExclModeVideoCallback *) new COverlayCallback(this, hWndApp, &hr) ;
    ASSERT(SUCCEEDED(hr)) ;
    m_bOverlayVisible = FALSE ;  // don't draw color key until it's OK-ed by OverlayMixer
}


//
// CDDrawObject destructor
//
CDDrawObject::~CDDrawObject(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDDrawObject d-tor entered"))) ;

    // Let go of the overlay notification callback object now
    if (m_pOverlayCallback)
        m_pOverlayCallback->Release() ;  // done with callback object now

    // delete the pen and brush we created for drawing the ball
    if (m_hPen)
        DeleteObject(m_hPen) ;
    if (m_hBrush)
        DeleteObject(m_hBrush) ;
    
    if (m_pBackBuff)
        m_pBackBuff->Release() ;
    if (m_pPrimary)
        m_pPrimary->Release() ;
    
    if (m_pDDObject)
        m_pDDObject->Release() ;
    
    DbgLog((LOG_TRACE, 3, TEXT("CDDrawObject d-tor exiting..."))) ;
}


//
// CDDrawObject::Initialize(): Just creates a DDraw object.
//
BOOL CDDrawObject::Initialize(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::Initialize() entered"))) ;
    
    HRESULT hr = DirectDrawCreate(NULL, &m_pDDObject, NULL) ;
    if (FAILED(hr) || NULL == m_pDDObject)
    {
        MessageBox(hWndApp, 
            TEXT("Can't create a DirectDraw object.\nPress OK to end the app."), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return FALSE ;
    }
    
    return TRUE;
}


#define SCRN_WIDTH    800
#define SCRN_HEIGHT   600
#define SCRN_BITDEPTH   8

//
// CDDrawObject::StartExclusiveMode(): Gets into fullscreen exclusive mode, sets
// display mode to 800x600x8, creates a flipping primary surface with one back 
// buffer, gets the surface size and inits some variables.
//
// NOTE: Please don't bother about the "#ifndef NOFLIP"s.  They were there
// for testing.
//
HRESULT CDDrawObject::StartExclusiveMode(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::StartExclusiveMode() entered"))) ;
    
    HRESULT         hr ;
    DDSURFACEDESC   ddsd ;
    DDSCAPS         ddscaps ;
    
    if (! Initialize(hWndApp) )
        return E_FAIL ;

    // Get into fullscreen exclusive mode
#ifndef NOFLIP
    hr = m_pDDObject->SetCooperativeLevel(hWndApp,
        DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT) ;
    if (FAILED(hr))
    {
        MessageBox(hWndApp, 
            TEXT("SetCooperativeLevel() failed to go into exclusive mode"), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return hr ;
    }
#else
    hr = m_pDDObject->SetCooperativeLevel(hWndApp, DDSCL_NORMAL) ;
    if (FAILED(hr))
    {
        MessageBox(hWndApp, 
            TEXT("SetCooperativeLevel() failed to go into normal mode"), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return hr ;
    }
#endif // NOFLIP
    
#ifndef NOFLIP
    hr = m_pDDObject->SetDisplayMode(SCRN_WIDTH, SCRN_HEIGHT, SCRN_BITDEPTH) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: SetDisplayMode failed (Error 0x%lx)"), hr)) ;
        MessageBox(hWndApp, TEXT("SetDisplayMode(640, 480, 8) failed"), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return hr ;
    }
#endif // NOFLIP
    
    // Create the primary surface with 1 back buffer
    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    
#ifndef NOFLIP
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
        DDSCAPS_FLIP | 
        DDSCAPS_COMPLEX ;
    ddsd.dwBackBufferCount = 1 ;
#else
    ddsd.dwFlags = DDSD_CAPS ;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE ;
#endif  // NOFLIP
    
    hr = m_pDDObject->CreateSurface(&ddsd, &m_pPrimary, NULL) ;
    if (FAILED(hr))
    {
#ifndef NOFLIP
        MessageBox(hWndApp, TEXT("CreateSurface(Primary, flip(1), complex) failed"), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
#else
        MessageBox(hWndApp, TEXT("CreateSurface(Primary, noflip, no backbuffer) failed"), 
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
#endif // NOFLIP
        return hr ;
    }
    
#ifndef NOFLIP
    // Get a pointer to the back buffer
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER ;
    hr = m_pPrimary->GetAttachedSurface(&ddscaps, &m_pBackBuff) ;
    if (FAILED(hr))
    {
        MessageBox(hWndApp, TEXT("GetAttachedSurface() failed to get back buffer"),
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return hr ;
    }
#endif // NOFLIP
    
    // Get the screen size and save it as a rect
    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    hr = m_pPrimary->GetSurfaceDesc(&ddsd) ;
    if (! (SUCCEEDED(hr) && (ddsd.dwFlags & DDSD_WIDTH) && (ddsd.dwFlags & DDSD_HEIGHT)) )
    {
        MessageBox(hWndApp, TEXT("GetSurfaceDesc() failed to get surface width & height"),
            TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        return hr ;
    }
    
    SetRect(&m_RectScrn, 0, 0, ddsd.dwWidth, ddsd.dwHeight) ;
    
    // Reset some buffer drawing flags, values etc.
    m_iCount = 0 ;
    m_bFrontBuff = TRUE ;
    m_bInExclMode = TRUE ;
    
    return S_OK ;
}


//
// CDDrawObject::StopExclusiveMode(): Releases primary buffer, exits 
// exclusive mode, resets some variables/flags.
//
HRESULT CDDrawObject::StopExclusiveMode(HWND hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::StopExclusiveMode() entered"))) ;
    
    HRESULT hr ;
    
    if (m_pBackBuff)
    {
        hr = m_pBackBuff->Release() ;  // release back buffer
        m_pBackBuff = NULL ;  // no back buffer anymore
    }

#ifndef NOFLIP
    if (m_pDDObject)
    {
        // Restore display to previous mode (set with SetDisplayMode())
        hr = m_pDDObject->RestoreDisplayMode();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("ERROR: RestoreDisplayMode failed (Error 0x%lx)"), hr)) ;
            MessageBox(hWndApp, TEXT("RestoreDisplayMode failed"), 
                TEXT("Error"), MB_OK | MB_ICONSTOP) ;
        }
    }
#endif // NOFLIP

    // Release the primary surface (and attached buffer with it)
    if (m_pPrimary)
    {
        hr = m_pPrimary->Release() ;  // release primary surface
        m_pPrimary = NULL ;
    }
    
    // Get out of fullscreen exclusive mode
    if (m_pDDObject)
    {
        hr = m_pDDObject->SetCooperativeLevel(hWndApp, DDSCL_NORMAL) ;
        if (FAILED(hr))
        {
            MessageBox(hWndApp, 
                TEXT("SetCooperativeLevel() failed to go back to normal mode"), 
                TEXT("Error"), MB_OK | MB_ICONSTOP) ;
            return hr ;
        }

        m_pDDObject->Release() ;  // now release the DDraw object
        m_pDDObject = NULL ;      // no more DDraw object
    }
    
    // Reset the color key and screen, video rects.
    m_dwVideoKeyColor = RGB(255, 0, 255) ;
    SetRect(&m_RectScrn, 0, 0, 0, 0) ;
    SetRect(&m_RectVideo, 0, 0, 0, 0) ;
    
    m_bInExclMode = FALSE ;
    
    return S_OK ;
}


//
// CDDrawObject::DrawOnSurface(): Private method to draw stuff on back buffer
//
void CDDrawObject::DrawOnSurface(LPDIRECTDRAWSURFACE pSurface)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::DrawOnSurface() entered"))) ;
    
    HDC     hDC ;
    TCHAR   achBuffer[30] ;
    
    if (DD_OK == pSurface->GetDC(&hDC))
    {
        // write fonr/back to show buffer flipping
        SetBkColor(hDC, RGB(0, 0, 255)) ;
        SetTextColor(hDC, RGB(255, 255, 0)) ;
        wsprintf(achBuffer, TEXT(" %-12s: %6.6d "), 
                 m_bFrontBuff ? m_szFrontMsg : m_szBackMsg, m_iCount) ;
        TextOut(hDC, 20, 10, achBuffer, lstrlen(achBuffer)) ;
        
        // Write the instructions
        SetBkColor(hDC, RGB(0, 0, 0)) ;
        SetTextColor(hDC, RGB(0, 255, 255)) ;
        TextOut(hDC, m_RectScrn.left + 100, m_RectScrn.bottom - 20, m_szDirection, lstrlen(m_szDirection)) ;
        
        // Draw the ball on screen now
        HBRUSH hBrush = (HBRUSH) SelectObject(hDC, m_hBrush) ; // select special brush
        HPEN   hPen   = (HPEN) SelectObject(hDC, m_hPen) ;     // select special pen 
        Ellipse(hDC, m_iBallCenterX - BALL_RADIUS, m_iBallCenterY - BALL_RADIUS, 
            m_iBallCenterX + BALL_RADIUS, m_iBallCenterY + BALL_RADIUS) ;
        SelectObject(hDC, hBrush) ;  // restore original brush
        SelectObject(hDC, hPen) ;    // restore original pen
        
        pSurface->ReleaseDC(hDC) ;  // done now; let go of the DC
    }
}


//
// CDDrawObject::ConvertColorRefToPhysColor(): Converts a COLORREF to a
// DWORD physical color value based on current display mode.
//
// NOTE: This method is useful ONLY if you want to get the COLORREF from
// OverlayMixer (rather than the physical color as a DWORD) and convert
// it yourself.  But OverlayMixer does it for you anyway; so the utility
// of this method is quite minimal.
//
HRESULT CDDrawObject::ConvertColorRefToPhysColor(COLORREF rgb, DWORD *pdwPhysColor)
{
    HRESULT         hr ;
    COLORREF        rgbCurr ;
    HDC             hDC ;
    DDSURFACEDESC   ddsd ;
    
    ASSERT(rgb != CLR_INVALID) ;
    
    //  use GDI SetPixel to color match for us
    hr = m_pBackBuff->GetDC(&hDC) ;
    if (DD_OK != hr)
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pBackBuff->GetDC() failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    rgbCurr = GetPixel(hDC, 0, 0) ;             // save current pixel value
    SetPixel(hDC, 0, 0, rgb) ;               // set our value
    m_pBackBuff->ReleaseDC(hDC) ;
    
    // now lock the surface so we can read back the converted color
    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    while (DDERR_WASSTILLDRAWING == (hr = m_pBackBuff->Lock(NULL, &ddsd, 0, NULL)))  // until unlocked...
        ;               // ...just wait
    
    if (DD_OK == hr)
    {
        // get DWORD
        *pdwPhysColor = *(DWORD *) ddsd.lpSurface ;
        
        // mask it to bpp
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            *pdwPhysColor &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1 ;
        m_pBackBuff->Unlock(NULL) ;
    }
    else
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pBackBuff->Lock() failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    
    //  now restote the pixel to the color that was there
    hr = m_pBackBuff->GetDC(&hDC) ;
    if (DD_OK != hr)
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pBackBuff->GetDC() failed (Error 0x%lx)"), hr)) ;
        return hr ;
    }
    SetPixel(hDC, 0, 0, rgbCurr) ;
    m_pBackBuff->ReleaseDC(hDC) ;
    
    return S_OK ;
}


#if 0
//
// CDDrawObject::SetColorKey(): Creates a DWORD physical color out COLORREF.
// 
// NOTE: This method is useful only if you want to do the conversion. We have
// defined an inline version in ddrawobj.h file.
//
HRESULT CDDrawObject::SetColorKey(COLORKEY *pColorKey)
{
    if (NULL == pColorKey)
        return E_INVALIDARG ;
    
    DWORD    dwPhysColor ;
    HRESULT  hr = ConvertColorRefToPhysColor(pColorKey->LowColorValue, &dwPhysColor) ;
    if (FAILED(hr))
        return hr ;
    
    m_dwVideoKeyColor = dwPhysColor ;
    return S_OK ;
}
#endif // #if 0


//
// CDDrawObject::FillSurface(): Private method to fill the back buffer with black
// color and put the color key in teh video area.
//
// NOTE: Re-filling the back buffer every time is required ONLY IF the video 
// position keeps changing. Otherwise setting it once should be fine.
// 
HRESULT CDDrawObject::FillSurface(IDirectDrawSurface *pDDSurface)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::FillSurface() entered"))) ;

    if (NULL == pDDSurface)
        return E_INVALIDARG ;
    
    // Repaint the whole specified surface
    HRESULT  hr ;
    DDBLTFX  ddFX ;
    ZeroMemory(&ddFX, sizeof(ddFX)) ;
    ddFX.dwSize = sizeof(ddFX) ;
    ddFX.dwFillColor = m_dwScrnColor ;
    
    hr = pDDSurface->Blt(&m_RectScrn, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddFX) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("pDDSurface->Blt for screen failed (Error 0x%lx)"), hr)) ;
        DbgLog((LOG_ERROR, 0, TEXT("Destination Rect: (%ld, %ld, %ld, %ld), Color = 0x%lx"), 
            m_RectScrn.left, m_RectScrn.top, m_RectScrn.right, m_RectScrn.bottom, m_dwScrnColor)) ;
        return hr ;
    }
    
    // Draw color key on the video area of given surface, ONLY IF we are supposed
    // to paint color key in the video area.
    if (m_bOverlayVisible)
    {
        ddFX.dwFillColor = m_dwVideoKeyColor ;
        hr = pDDSurface->Blt(&m_RectVideo, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddFX) ;
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("pDDSurface->Blt for video failed (Error 0x%lx)"), hr)) ;
            DbgLog((LOG_ERROR, 0, TEXT("Destination Rect: (%ld, %ld, %ld, %ld), Color = 0x%lx"), 
                m_RectVideo.left, m_RectVideo.top, m_RectVideo.right, m_RectVideo.bottom, 
                m_dwVideoKeyColor)) ;
            return hr ;
        }
    }
    else
        DbgLog((LOG_TRACE, 5, TEXT("Color keying of video area skipped"))) ;
    
    return S_OK ;
}


//
// CDDrawObject::UpdateAndFlipSurfaces(): Prepares the back buffer and flips.
//
HRESULT CDDrawObject::UpdateAndFlipSurfaces(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::UpdateAndFlipSurfaces() entered"))) ;
    
    // Draw screen and color key on the current back buffer
    HRESULT  hr = FillSurface(m_pBackBuff) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("UpdateAndFlipSurfaces() skipped as FillSurface() failed"), hr)) ;
        return hr ;   // or return S_OK??
    }
    
    IncCount() ;                    // increment flip count first
    m_bFrontBuff = !m_bFrontBuff ;  // toggle flag
    DrawOnSurface(m_pBackBuff) ;  // draw next text on the back buffer
    
    // Keep trying to flip the buffers until successful
    while (1)
    {
        hr = m_pPrimary->Flip(NULL, 0) ;  // flip the surfaces
        if (DD_OK == hr)  // success!!
        {
            break ;
        }
        if (DDERR_SURFACELOST == hr)   // surface lost; try to restore
        {
            DbgLog((LOG_TRACE, 5, TEXT("DirectDraw surface was lost. Trying to restore..."))) ;
            hr = m_pPrimary->Restore() ;
            if (DD_OK != hr)  // couldn't restore surface
            {
                DbgLog((LOG_ERROR, 0, TEXT("IDirectDrawSurface::Restore() failed (Error 0x%lx)"), hr)) ;
                break ;
            }
        }
        if (DDERR_WASSTILLDRAWING != hr)  // some weird error -- bail out
        {
            DbgLog((LOG_ERROR, 0, TEXT("IDirectDrawSurface::Flip() failed (Error 0x%lx)"), hr)) ;
            break ;
        }
    }
    
    return hr ;
}


//
// CDDrawObject::SetVideoPosition(): Receives the video position in terms of screen
// coordinates.
//
void CDDrawObject::SetVideoPosition(DWORD dwVideoLeft, DWORD dwVideoTop, 
                                    DWORD dwVideoWidth, DWORD dwVideoHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::SetVideoPosition() entered"))) ;
    
    SetRect(&m_RectVideo, dwVideoLeft, dwVideoTop, dwVideoLeft + dwVideoWidth,
        dwVideoTop + dwVideoHeight) ;
}


//
// CDDrawObject::SetBallPosition(): Uses the video position to initialize the ball's
// position to the center of the video.
//
void CDDrawObject::SetBallPosition(DWORD dwVideoLeft, DWORD dwVideoTop, 
                                   DWORD dwVideoWidth, DWORD dwVideoHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::SetBallPosition() entered"))) ;
    
    // Place the ball at the middle of video on start
    m_iBallCenterX = dwVideoLeft + dwVideoWidth  / 2 ;
    m_iBallCenterY = dwVideoTop  + dwVideoHeight / 2 ;
}


//
// CDDrawObject::MoveBallPosition(): Moves the ball L/R/U/D and keeps it within
// the video area..
//
void CDDrawObject::MoveBallPosition(int iDirX, int iDirY)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDDrawObject::MoveBallPosition(%d, %d) entered"),
        iDirX, iDirY)) ;
    
    m_iBallCenterX += iDirX ;
    m_iBallCenterY += iDirY ;
    
    // Keep the ball inside the video rect
    if (m_iBallCenterX < m_RectVideo.left + BALL_RADIUS)
        m_iBallCenterX = m_RectVideo.left + BALL_RADIUS ;
    else if (m_iBallCenterX > m_RectVideo.right - BALL_RADIUS)
        m_iBallCenterX = m_RectVideo.right - BALL_RADIUS ;
    
    if (m_iBallCenterY < m_RectVideo.top + BALL_RADIUS)
        m_iBallCenterY = m_RectVideo.top + BALL_RADIUS ;
    else if (m_iBallCenterY > m_RectVideo.bottom - BALL_RADIUS)
        m_iBallCenterY = m_RectVideo.bottom - BALL_RADIUS ;
}



//
// Overlay Callback object implementation
//
COverlayCallback::COverlayCallback(CDDrawObject *pDDrawObj, HWND hWndApp, HRESULT *phr) :
CUnknown("Overlay Callback Object", NULL, phr),
m_pDDrawObj(pDDrawObj),
m_hWndApp(hWndApp)
{
    DbgLog((LOG_TRACE, 5, TEXT("COverlayCallback::COverlayCallback() entered"))) ;
    ASSERT(m_pDDrawObj) ;
    AddRef() ;    // increment ref count for itself
}


COverlayCallback::~COverlayCallback()
{
    DbgLog((LOG_TRACE, 5, TEXT("COverlayCallback::~COverlayCallback() entered"))) ;
}


HRESULT COverlayCallback::OnUpdateOverlay(BOOL  bBefore,
                                          DWORD dwFlags,
                                          BOOL  bOldVisible,
                                          const RECT *prcSrcOld,
                                          const RECT *prcDestOld,
                                          BOOL  bNewVisible,
                                          const RECT *prcSrcNew,
                                          const RECT *prcDestNew)
{
    DbgLog((LOG_TRACE, 5, TEXT("COverlayCallback::OnUpdateOverlay(%s, ...) entered"),
            bBefore ? "TRUE" : "FALSE")) ;

    if (NULL == m_pDDrawObj)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: NULL DDraw object pointer was specified"))) ;
        return E_POINTER ;
    }

    if (bBefore)  // overlay is going to be updated
    {
        DbgLog((LOG_TRACE, 5, TEXT("Just turn off color keying and return"))) ;
        m_pDDrawObj->SetOverlayState(FALSE) ;   // don't paint color key in video's position
#ifndef NOFLIP
        m_pDDrawObj->UpdateAndFlipSurfaces() ;  // flip the surface so that video doesn't show anymore
#endif // NOFLIP
        return S_OK ;
    }

    //
    // After overlay has been updated. Turn on/off overlay color keying based on 
    // flags and use new source/dest position etc.
    //
    if (dwFlags & (AM_OVERLAY_NOTIFY_VISIBLE_CHANGE |
                   AM_OVERLAY_NOTIFY_SOURCE_CHANGE  |
                   AM_OVERLAY_NOTIFY_DEST_CHANGE))   // it's a valid flag
        m_pDDrawObj->SetOverlayState(bNewVisible) ;  // paint/don't paint color key based on param

    if (dwFlags & AM_OVERLAY_NOTIFY_VISIBLE_CHANGE)  // overlay visibility state change
    {
        DbgLog((LOG_TRACE, 5, TEXT(".._NOTIFY_VISIBLE_CHANGE from %s to %s"), 
                bOldVisible ? "TRUE" : "FALSE", bNewVisible ? "TRUE" : "FALSE")) ;
    }

    if (dwFlags & AM_OVERLAY_NOTIFY_SOURCE_CHANGE)   // overlay source rect change
    {
        DbgLog((LOG_TRACE, 5, TEXT(".._NOTIFY_SOURCE_CHANGE from (%ld, %ld, %ld, %ld) to (%ld, %ld, %ld, %ld)"), 
                prcSrcOld->left, prcSrcOld->top, prcSrcOld->right, prcSrcOld->bottom,
                prcSrcNew->left, prcSrcNew->top, prcSrcNew->right, prcSrcNew->bottom)) ;
    }

    if (dwFlags & AM_OVERLAY_NOTIFY_DEST_CHANGE)     // overlay destination rect change
    {
        DbgLog((LOG_TRACE, 5, TEXT(".._NOTIFY_DEST_CHANGE from (%ld, %ld, %ld, %ld) to (%ld, %ld, %ld, %ld)"), 
                prcDestOld->left, prcDestOld->top, prcDestOld->right, prcDestOld->bottom,
                prcDestNew->left, prcDestNew->top, prcDestNew->right, prcDestNew->bottom)) ;
        m_pDDrawObj->SetVideoPosition(prcDestNew->left, prcDestNew->top, 
                                      RECTWIDTH(*prcDestNew), RECTHEIGHT(*prcDestNew)) ;
    }

    return S_OK ;
}


HRESULT COverlayCallback::OnUpdateColorKey(COLORKEY const *pKey,
                                           DWORD           dwColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("COverlayCallback::OnUpdateColorKey(.., 0x%lx) entered"), dwColor)) ;

    return S_OK ;
}


HRESULT COverlayCallback::OnUpdateSize(DWORD dwWidth, DWORD dwHeight, 
                                       DWORD dwARWidth, DWORD dwARHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("COverlayCallback::OnUpdateSize(%lu, %lu, %lu, %lu) entered"), 
            dwWidth, dwHeight, dwARWidth, dwARHeight)) ;

    PostMessage(m_hWndApp, WM_SIZE_CHANGE, 0, 0) ;

    return S_OK ;
}

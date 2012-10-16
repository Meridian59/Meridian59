//------------------------------------------------------------------------------
// File: allocpresenter.cpp
//
// Desc: DirectShow sample code - Custom Allocator Presenter for VMR sample
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <mmreg.h>
#include "project.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "alloclib.h"


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
    if(riid == IID_IVMRSurfaceAllocator)
    {
        return GetInterface((IVMRSurfaceAllocator*)this, ppv);
    }
    else if(riid == IID_IVMRImagePresenter)
    {
        return GetInterface((IVMRImagePresenter*)this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}


//////////////////////////////////////////////////////////////////////////////
//
// IVMRSurfaceAllocator
//
//////////////////////////////////////////////////////////////////////////////

/******************************Public*Routine******************************\
* AllocateSurfaces
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::AllocateSurface(
    DWORD_PTR x,
    VMRALLOCATIONINFO *w,
    DWORD* lpdwBuffer,
    LPDIRECTDRAWSURFACE7* lplpSurface
    )
{
    DWORD dwFlags = w->dwFlags;
    LPBITMAPINFOHEADER lpHdr = w->lpHdr;
    LPDDPIXELFORMAT lpPixFmt = w->lpPixFmt;
    LPSIZE lpAspectRatio = &w->szAspectRatio;
    DWORD dwMinBuffers = w->dwMinBuffers;
    DWORD dwMaxBuffers = w->dwMaxBuffers;

    if(!lpHdr)
    {
        return E_POINTER;
    }

    if(!lpAspectRatio)
    {
        return E_POINTER;
    }

    if(dwFlags & AMAP_PIXELFORMAT_VALID)
    {
        if(!lpPixFmt)
        {
            return E_INVALIDARG;
        }
    }

    HRESULT hr = AllocateSurfaceWorker(dwFlags, lpHdr, lpPixFmt,
                                       lpAspectRatio,
                                       dwMinBuffers, dwMaxBuffers,
                                       lpdwBuffer, lplpSurface);
    return hr;
}

/******************************Public*Routine******************************\
* AllocateSurfaceWorker
*
\**************************************************************************/
HRESULT
CMpegMovie::AllocateSurfaceWorker(
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpHdr,
    LPDDPIXELFORMAT lpPixFmt,
    LPSIZE lpAspectRatio,
    DWORD dwMinBuffers,
    DWORD dwMaxBuffers,
    DWORD* lpdwBuffer,
    LPDIRECTDRAWSURFACE7* lplpSurface
    )
{
    LPBITMAPINFOHEADER lpHeader = lpHdr;
    if(!lpHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Can't get bitmapinfoheader from media type!!")));
        return E_INVALIDARG;
    }

    DDSURFACEDESC2 ddsd;
    INITDDSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.dwWidth = abs(lpHeader->biWidth);
    ddsd.dwHeight = abs(lpHeader->biHeight);
    ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY /*| DDSCAPS_TEXTURE */;

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    if(lpHdr->biCompression <= BI_BITFIELDS &&
        m_DispInfo.bmiHeader.biBitCount <= lpHdr->biBitCount)
    {
        ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;

        if(lpHdr->biBitCount == 32)
        {
            ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
            ddsd.ddpfPixelFormat.dwRBitMask = 0xff0000;
            ddsd.ddpfPixelFormat.dwGBitMask = 0xff00;
            ddsd.ddpfPixelFormat.dwBBitMask = 0xff;
        }
        else if(lpHdr->biBitCount == 16)
        {
            ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
            ddsd.ddpfPixelFormat.dwRBitMask = 0xF800;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x07e0;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x001F;
        }
    }
    else if(lpHdr->biCompression > BI_BITFIELDS)
    {
        const DWORD dwCaps = (DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH);
        if((dwCaps & m_ddHWCaps.dwCaps) != dwCaps)
        {
            DbgLog((LOG_ERROR, 1,
                TEXT("Can't BLT_FOURCC | BLT_STRETCH!!")));
            return E_FAIL;
        }
        ddsd.ddpfPixelFormat.dwFourCC = lpHdr->biCompression;
        ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd.ddpfPixelFormat.dwYUVBitCount = lpHdr->biBitCount;
    }
    else
    {
        return E_FAIL;
    }


    // Adjust width and height, if the driver requires it
    DWORD dwWidth  = ddsd.dwWidth;
    DWORD dwHeight = ddsd.dwHeight;

    HRESULT hr = m_lpDDObj->CreateSurface(&ddsd, &m_lpDDTexture, NULL);

    if(SUCCEEDED(hr))
    {
        m_VideoAR = *lpAspectRatio;

        m_VideoSize.cx = abs(lpHeader->biWidth);
        m_VideoSize.cy = abs(lpHeader->biHeight);

        SetRect(&m_rcDst, 0, 0, m_VideoSize.cx, m_VideoSize.cy);
        m_rcSrc = m_rcDst;

        hr = PaintDDrawSurfaceBlack(m_lpDDTexture);

        *lplpSurface = m_lpDDTexture;
        *lpdwBuffer = 1;
    }

    return hr;
}


/******************************Public*Routine******************************\
* FreeSurfaces()
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::FreeSurface(DWORD_PTR w)
{
    if(m_lpDDTexture)
    {
        m_lpDDTexture->Release();
        m_lpDDTexture = NULL;
    }

    return S_OK;
}


/******************************Public*Routine******************************\
* PrepareSurface
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::PrepareSurface(
    DWORD_PTR w,
    LPDIRECTDRAWSURFACE7 lplpSurface,
    DWORD dwSurfaceFlags
    )
{
    return S_OK;
}

/******************************Public*Routine******************************\
* AdviseNotify
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::AdviseNotify(
    IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify
    )
{
    return E_NOTIMPL;
}


//////////////////////////////////////////////////////////////////////////////
//
// IVMRImagePresenter
//
//////////////////////////////////////////////////////////////////////////////

/******************************Public*Routine******************************\
* StartPresenting()
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::StartPresenting(DWORD_PTR w)
{
    return S_OK;
}

/******************************Public*Routine******************************\
* StopPresenting()
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::StopPresenting(DWORD_PTR w)
{
    return S_OK;
}

/******************************Public*Routine******************************\
* PresentImage
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::PresentImage(
    DWORD_PTR w,
    VMRPRESENTATIONINFO* p
    )
{
    //
    // Call the app specific function to render the scene
    //
    LPDIRECTDRAWSURFACE7 lpSurface = p->lpSurf;
    const REFERENCE_TIME rtNow = p->rtStart;
    const DWORD dwSurfaceFlags = p->dwFlags;

    CAutoLock Lock(&m_AppImageLock);

    RECT rSrc = {0, 0, m_VideoSize.cx, m_VideoSize.cy};
    RECT rDst = {0, 0, WIDTH(&m_rcDst), HEIGHT(&m_rcDst)};

    //PaintDDrawSurfaceBlack(m_lpBackBuffer);
    //m_lpBltAlpha->AlphaBlt(&rDst, m_lpDDTexture, &rSrc, 0xFF);
    m_lpBackBuffer->Blt(&rDst, m_lpDDTexture, &rSrc, DDBLT_WAIT, NULL);

    rDst.left   = m_cxFontImg;
    rDst.top    = m_cyFontImg * 1;
    rDst.right  = rDst.left + (m_cxFontImg * 40);
    rDst.bottom = rDst.top  + (m_cyFontImg * 4);

    rSrc.left   = 0;
    rSrc.top    = 0;
    rSrc.right  = (m_cxFontImg * 40);
    rSrc.bottom = (m_cyFontImg * 4);
    m_lpBltAlpha->AlphaBlt(&rDst, m_lpDDAppImage, &rSrc, 0x80);


    RECT rc = m_rcDst;
    RECT rcSrc;

    SetRect(&rcSrc, 0, 0, WIDTH(&rc), HEIGHT(&rc));
    m_lpPriSurf->Blt(&rc, m_lpBackBuffer, &rcSrc, DDBLT_WAIT, NULL);

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
//
// Allocator Presenter helper functions
//
//////////////////////////////////////////////////////////////////////////////


/*****************************Private*Routine******************************\
* InitDisplayInfo
*
\**************************************************************************/
BOOL
InitDisplayInfo(
    AMDISPLAYINFO* lpDispInfo
    )
{
    static char szDisplay[] = "DISPLAY";
    ZeroMemory(lpDispInfo, sizeof(*lpDispInfo));

    HDC hdcDisplay = CreateDCA(szDisplay, NULL, NULL, NULL);
    HBITMAP hbm = CreateCompatibleBitmap(hdcDisplay, 1, 1);

    lpDispInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    GetDIBits(hdcDisplay, hbm, 0, 1, NULL, (BITMAPINFO *)lpDispInfo, DIB_RGB_COLORS);
    GetDIBits(hdcDisplay, hbm, 0, 1, NULL, (BITMAPINFO *)lpDispInfo, DIB_RGB_COLORS);

    DeleteObject(hbm);
    DeleteDC(hdcDisplay);

    return TRUE;
}

/*****************************Private*Routine******************************\
* DDARGB32SurfaceInit
*
\**************************************************************************/
HRESULT
CMpegMovie::DDARGB32SurfaceInit(
    LPDIRECTDRAWSURFACE7* lplpDDS,
    BOOL bTexture,
    DWORD cx,
    DWORD cy
    )
{
    DDSURFACEDESC2 ddsd;
    HRESULT hRet;

    *lplpDDS = NULL;

    INITDDSTRUCT(ddsd);

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
    if(bTexture)
    {
        ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
    }
    ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
    if(bTexture)
    {
        ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
    }
    ddsd.ddpfPixelFormat.dwRBitMask        = 0x00FF0000;
    ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000FF00;
    ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000FF;


    if(bTexture)
    {
        ddsd.ddsCaps.dwCaps =  DDSCAPS_TEXTURE;
        ddsd.ddsCaps.dwCaps2 = (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTDYNAMIC);
    }
    else
    {
        ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
    }
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.dwBackBufferCount = 0;

    if(bTexture)
    {
        for(ddsd.dwWidth=1;  cx>ddsd.dwWidth;   ddsd.dwWidth<<=1);
        for(ddsd.dwHeight=1; cy>ddsd.dwHeight; ddsd.dwHeight<<=1);
    }
    else
    {
        ddsd.dwWidth=cx;
        ddsd.dwHeight=cy;
    }


    // Attempt to create the surface with theses settings
    hRet = m_lpDDObj->CreateSurface(&ddsd, lplpDDS, NULL);

    return hRet;
}


/*****************************Private*Routine******************************\
* CreateFontCache
*
\**************************************************************************/
HRESULT
CMpegMovie::CreateFontCache(
    int cyFont
    )
{
    //
    // Initialize the LOGFONT structure - we want to
    // create an "anti-aliased" Lucida Consol font
    //
    LOGFONT lfChar;
    ZeroMemory(&lfChar, sizeof(lfChar));
    lfChar.lfHeight = -cyFont;
    lfChar.lfCharSet = OEM_CHARSET ;
    lfChar.lfPitchAndFamily = FIXED_PITCH | FF_MODERN ;
    lstrcpy(lfChar.lfFaceName, TEXT("Lucida Console")) ;
    lfChar.lfWeight = FW_NORMAL ;
    lfChar.lfOutPrecision = OUT_STRING_PRECIS ;
    lfChar.lfClipPrecision = CLIP_STROKE_PRECIS ;
    lfChar.lfQuality = ANTIALIASED_QUALITY;


    HFONT hFont = CreateFontIndirect(&lfChar) ;
    if(!hFont)
    {
        return E_OUTOFMEMORY;
    }

    //
    // The following magic is necessary to get GDI to rasterize
    // the font with anti-aliasing switched on when we later use
    // the font in a DDraw Surface.  The doc's say that this is only
    // necessary in Win9X - but Win2K seems to require it too.
    //
    SIZE size;
    HDC hdcWin = GetDC(NULL);
    hFont = (HFONT)SelectObject(hdcWin, hFont);
    GetTextExtentPoint32(hdcWin, TEXT("A"), 1, &size);
    hFont = (HFONT)SelectObject(hdcWin, hFont);
    ReleaseDC(NULL, hdcWin);

    //
    // Make sure that the font doesn't get too big.
    //
    if(size.cx * GRID_CX > 1024)
    {
        return S_OK;
    }


    //
    // Delete the old font and assign the new one
    //
    RELEASE(m_lpDDSFontCache);
    if(m_hFont)
    {
        DeleteObject(m_hFont);
    }
    m_cxFont = size.cx; m_cyFont = size.cy;
    m_hFont = hFont;


    //
    // Create the DDraw ARGB32 surface that we will use
    // for the font cache.
    //

    HRESULT hr = DDARGB32SurfaceInit(&m_lpDDSFontCache, TRUE, 16 * size.cx, 6 * size.cy);
    if(hr == DD_OK)
    {

        HDC hdcDest;

        m_lpDDSFontCache->GetDC(&hdcDest);

        //
        // Select the font into the DDraw surface and draw the characters
        //
        m_hFont = (HFONT)SelectObject(hdcDest, m_hFont);
        SetTextColor(hdcDest, RGB(255,255,255));
        SetBkColor(hdcDest, RGB(0,0,0));
        SetBkMode(hdcDest, OPAQUE);

        int row, col; TCHAR ch = (TCHAR)32;
        for(row = 0; row < 6; row++)
        {
            for(col = 0; col < 16; col++)
            {
                TextOut(hdcDest, col * size.cx, row * size.cy, &ch, 1);
                ch++;
            }
        }
        m_hFont = (HFONT)SelectObject(hdcDest, m_hFont);
        m_lpDDSFontCache->ReleaseDC(hdcDest);

        DDSURFACEDESC2 surfDesc;
        INITDDSTRUCT(surfDesc);
        HRESULT hr = m_lpDDSFontCache->Lock(NULL, &surfDesc, DDLOCK_WAIT, NULL);
        if(hr == DD_OK)
        {

            LPDWORD lpDst = (LPDWORD)surfDesc.lpSurface;
            for(row = 0; row < 6 * size.cy; row++)
            {

                LPDWORD lp = lpDst;
                for(col = 0; col < 16 * size.cx; col++)
                {

                    DWORD dwPel = *lp;
                    if(dwPel)
                    {
                        dwPel <<= 24;
                        dwPel |= 0x00FFFFFF;
                    }
                    else
                    {
                        dwPel = 0x80000000;
                    }

                    *lp++ = dwPel;
                }
                lpDst += (surfDesc.lPitch / 4);
            }
            m_lpDDSFontCache->Unlock(NULL);
        }
    }

    return S_OK;
}


/*****************************Private*Routine******************************\
* Initialize3DEnvironment
*
\**************************************************************************/
HRESULT
CMpegMovie::Initialize3DEnvironment(
    HWND hWnd
    )
{
    HRESULT hr;

    //
    // Create the IDirectDraw interface. The first parameter is the GUID,
    // which is allowed to be NULL. If there are more than one DirectDraw
    // drivers on the system, a NULL guid requests the primary driver. For
    // non-GDI hardware cards like the 3DFX and PowerVR, the guid would need
    // to be explicity specified . (Note: these guids are normally obtained
    // from enumeration, which is convered in a subsequent tutorial.)
    //

    m_hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

    hr = DirectDrawCreateEx(NULL, (VOID**)&m_lpDDObj, IID_IDirectDraw7, NULL);
    if(FAILED(hr))
        return hr;

    //
    // get the h/w caps for this device
    //
    INITDDSTRUCT(m_ddHWCaps);
    hr = m_lpDDObj->GetCaps(&m_ddHWCaps, NULL);
    if(FAILED(hr))
        return hr;
    InitDisplayInfo(&m_DispInfo);

    //
    // Set the Windows cooperative level. This is where we tell the system
    // whether we will be rendering in fullscreen mode or in a window. Note
    // that some hardware (non-GDI) may not be able to render into a window.
    // The flag DDSCL_NORMAL specifies windowed mode. Using fullscreen mode
    // is the topic of a subsequent tutorial. The DDSCL_FPUSETUP flag is a
    // hint to DirectX to optomize floating points calculations. See the docs
    // for more info on this. Note: this call could fail if another application
    // already controls a fullscreen, exclusive mode.
    //
    hr = m_lpDDObj->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
    if(FAILED(hr))
        return hr;

    //
    // Initialize a surface description structure for the primary surface. The
    // primary surface represents the entire display, with dimensions and a
    // pixel format of the display. Therefore, none of that information needs
    // to be specified in order to create the primary surface.
    //
    DDSURFACEDESC2 ddsd;
    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    //
    // Create the primary surface.
    //
    hr = m_lpDDObj->CreateSurface(&ddsd, &m_lpPriSurf, NULL);
    if(FAILED(hr))
        return hr;

    //
    // Create a clipper object which handles all our clipping for cases when
    // our window is partially obscured by other windows. This is not needed
    // for apps running in fullscreen mode.
    //
    LPDIRECTDRAWCLIPPER pcClipper;
    hr = m_lpDDObj->CreateClipper(0, &pcClipper, NULL);
    if(FAILED(hr))
        return hr;

    //
    // Associate the clipper with our window. Note that, afterwards, the
    // clipper is internally referenced by the primary surface, so it is safe
    // to release our local reference to it.
    //
    pcClipper->SetHWnd(0, hWnd);
    m_lpPriSurf->SetClipper(pcClipper);
    pcClipper->Release();

    //
    // Before creating the device, check that we are NOT in a palettized
    // display. That case will cause CreateDevice() to fail, since this simple
    // tutorial does not bother with palettes.
    //
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    m_lpDDObj->GetDisplayMode(&ddsd);
    if(ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
        return DDERR_INVALIDMODE;

    DWORD dwRenderWidth  = ddsd.dwWidth;
    DWORD dwRenderHeight = ddsd.dwHeight;


    //
    // Setup a surface description to create a backbuffer. This is an
    // offscreen plain surface with dimensions equal to the current display
    // size.

    // The DDSCAPS_3DDEVICE is needed so we can later query this surface
    // for an IDirect3DDevice interface.
    //
    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
    ddsd.dwWidth = dwRenderWidth;
    ddsd.dwHeight = dwRenderHeight;


    //
    // Create the backbuffer. The most likely reason for failure is running
    // out of video memory. (A more sophisticated app should handle this.)
    //
    hr = m_lpDDObj->CreateSurface(&ddsd, &m_lpBackBuffer, NULL);
    if(FAILED(hr))
        return hr;

    //
    // Create the textbuffer.
    //
    // The text buffer should be RGB32 (for now - later I'll try
    // ARGB16:4:4:4:4, but that is a lot more work).
    //
    hr = DDARGB32SurfaceInit(&m_lpDDAppImage, TRUE,
        1024, MulDiv(4, (int)dwRenderHeight, GRID_CY));
    if(FAILED(hr))
        return hr;

    PaintDDrawSurfaceBlack(m_lpDDAppImage);


    //
    // Create the device. The device is created off of our back buffer, which
    // becomes the render target for the newly created device. Note that the
    // z-buffer must be created BEFORE the device
    //
    m_lpBltAlpha = new CAlphaBlt(m_lpBackBuffer, &hr);
    if(m_lpBltAlpha == NULL || hr != DD_OK)
    {
        if(m_lpBltAlpha == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        delete m_lpBltAlpha;
    }

    hr = CreateFontCache(32);

    return hr;
}


POINT LookUpChar(char ch, int cxFont, int cyFont)
{
    ch -= 32;

    int row = ch / 16;
    int col = ch % 16;

    POINT pt;

    pt.x = col * cxFont;
    pt.y = row * cyFont;

    return pt;
}


/******************************Public*Routine******************************\
* SetAppText
*
\**************************************************************************/
BOOL
CMpegMovie::SetAppText(
    char* sz
    )
{
    DDBLTFX ddFX;
    INITDDSTRUCT(ddFX);
    ddFX.dwFillColor =  0xFF80FF80;
    CAutoLock Lock(&m_AppImageLock);
    m_lpDDAppImage->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX);

    m_cxFontImg = m_cxFont;
    m_cyFontImg = m_cyFont;
    RECT rcDst = {0, 0, m_cxFont, m_cyFont};

    while(*sz)
    {
        if(*sz == '\n')
        {
            OffsetRect(&rcDst, 0, m_cyFont);
            rcDst.left = 0;
            rcDst.right = m_cxFont;
        }
        else
        {
            POINT pt = LookUpChar(*sz, m_cxFont, m_cyFont);

            RECT rcSrc;
            rcSrc.left = pt.x;
            rcSrc.top  = pt.y;
            rcSrc.right = pt.x + m_cxFont;
            rcSrc.bottom  = pt.y + m_cyFont;

            m_lpDDAppImage->Blt(&rcDst, m_lpDDSFontCache, &rcSrc, DDBLT_WAIT, NULL);
            OffsetRect(&rcDst, m_cxFont, 0);
        }
        sz++;
    }

    return TRUE;
}



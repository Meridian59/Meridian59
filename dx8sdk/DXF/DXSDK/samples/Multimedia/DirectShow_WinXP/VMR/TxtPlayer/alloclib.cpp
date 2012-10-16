//------------------------------------------------------------------------------
// File: alloclib.cpp
//
// Desc: DirectShow sample code
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <malloc.h>

#include "AllocLib.h"


/*****************************Private*Routine******************************\
* YV12PaintSurfaceBlack
*
\**************************************************************************/
HRESULT
YV12PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("YV12PaintSurfaceBlack")));
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

    // now lock the surface so we can start filling the surface with black
    ddsd.dwSize = sizeof(ddsd);

    for ( ;; ) {

        hr = pDDrawSurface->Lock(NULL, &ddsd,
                                 DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);

        if (hr == DD_OK || hr != DDERR_WASSTILLDRAWING) {
            break;
        }

        Sleep(1);
    }

    if (hr == DD_OK)
    {
        DWORD y;
        LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;
        DWORD VSize = ddsd.dwHeight;
        DWORD HSize = ddsd.dwWidth;

        // Y Component
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x10);     // 1 line at a time
            pDst += OutStride;
        }

        HSize /= 2;
        VSize /= 2;
        OutStride /= 2;

        // Cb Component
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);     // 1 line at a time
            pDst += OutStride;
        }

        // Cr Component
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);     // 1 line at a time
            pDst += OutStride;
        }

        pDDrawSurface->Unlock(NULL);
    }

    return hr;
}


/******************************Public*Routine******************************\
* YUV16PaintSurfaceBlack
*
\**************************************************************************/
HRESULT
YUV16PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pdds,
    DWORD dwBlack
    )
{
    AMTRACE((TEXT("YUV16PaintSurfaceBlack")));
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

    // now lock the surface so we can start filling the surface with black
    ddsd.dwSize = sizeof(ddsd);

    for ( ;; ) {

        hr = pdds->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);

        if (hr == DD_OK || hr != DDERR_WASSTILLDRAWING) {
            break;
        }

        Sleep(1);
    }

    if (hr == DD_OK)
    {
        DWORD y, x;
        LPDWORD pDst = (LPDWORD)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;

        for (y = 0; y < ddsd.dwHeight; y++) {

            for (x = 0; x < ddsd.dwWidth / 2; x++) {
                pDst[x] = dwBlack;
            }

            // Dont forget that the stride is a byte count
            *((LPBYTE*)&pDst) += OutStride;
        }

        pdds->Unlock(NULL);
    }

    return hr;
}


/*****************************Private*Routine******************************\
* BlackPaintProc
*
\**************************************************************************/
HRESULT
BlackPaintProc(
    LPDIRECTDRAWSURFACE7 pDDrawSurface,
    DDSURFACEDESC2* lpddSurfaceDesc
    )
{
    AMTRACE((TEXT("BlackPaintProc")));

    //
    // If the surface is YUV take care of the types that we
    // know the pixel format for.  Those surfaces that we don't know
    // about will get painted '0' which may be bright green for
    // YUV surfaces.
    //

    if (lpddSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC) {

        //
        // compute the black value if the fourCC code is suitable,
        // otherwise can't handle it
        //

        switch (lpddSurfaceDesc->ddpfPixelFormat.dwFourCC) {

        case mmioFOURCC('Y','V','1','2'):
        case mmioFOURCC('I','4','2','0'):
        case mmioFOURCC('I','Y','U','V'):
            return YV12PaintSurfaceBlack(pDDrawSurface);

        case mmioFOURCC('Y','U','Y','2'):
            return YUV16PaintSurfaceBlack(pDDrawSurface, 0x80108010);

        case mmioFOURCC('U','Y','V','Y'):
            return YUV16PaintSurfaceBlack(pDDrawSurface, 0x10801080);
        }
    }

    DDBLTFX ddFX;
    INITDDSTRUCT(ddFX);
    return pDDrawSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX);
}


/*****************************Private*Routine******************************\
* PaintSurfaceBlack
*
\**************************************************************************/
HRESULT
PaintDDrawSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("PaintDDrawSurfaceBlack")));

    LPDIRECTDRAWSURFACE7 *ppDDrawSurface = NULL;
    DDSCAPS2 ddSurfaceCaps;
    DDSURFACEDESC2 ddSurfaceDesc;
    DWORD dwAllocSize;
    DWORD i = 0, dwBackBufferCount = 0;

    // get the surface description
    INITDDSTRUCT(ddSurfaceDesc);
    HRESULT hr = pDDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (SUCCEEDED(hr)) {

        if (ddSurfaceDesc.dwFlags & DDSD_BACKBUFFERCOUNT) {
            dwBackBufferCount = ddSurfaceDesc.dwBackBufferCount;
        }

        hr = BlackPaintProc(pDDrawSurface, &ddSurfaceDesc);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("pDDrawSurface->Blt failed, hr = 0x%x"), hr));
            return hr;
        }

        if (dwBackBufferCount > 0) {

            dwAllocSize = (dwBackBufferCount + 1) * sizeof(LPDIRECTDRAWSURFACE);
            ppDDrawSurface = (LPDIRECTDRAWSURFACE7*)_alloca(dwAllocSize);

            ZeroMemory(ppDDrawSurface, dwAllocSize);
            ZeroMemory(&ddSurfaceCaps, sizeof(ddSurfaceCaps));
            ddSurfaceCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX;

            if( DDSCAPS_OVERLAY & ddSurfaceDesc.ddsCaps.dwCaps ) {
                ddSurfaceCaps.dwCaps |= DDSCAPS_OVERLAY;
            }

            for (i = 0; i < dwBackBufferCount; i++) {

                LPDIRECTDRAWSURFACE7 pCurrentDDrawSurface = NULL;
                if (i == 0) {
                    pCurrentDDrawSurface = pDDrawSurface;
                }
                else {
                    pCurrentDDrawSurface = ppDDrawSurface[i];
                }
                ASSERT(pCurrentDDrawSurface);


                //
                // Get the back buffer surface and store it in the
                // next (in the circular sense) entry
                //

                hr = pCurrentDDrawSurface->GetAttachedSurface(
                        &ddSurfaceCaps,
                        &ppDDrawSurface[i + 1]);

                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR,1,
                            TEXT("Function pDDrawSurface->GetAttachedSurface ")
                            TEXT("failed, hr = 0x%x"), hr ));
                    break;
                }

                ASSERT(ppDDrawSurface[i+1]);

                //
                // Peform a DirectDraw colorfill BLT
                //

                hr = BlackPaintProc(ppDDrawSurface[i + 1], &ddSurfaceDesc);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,1,
                            TEXT("ppDDrawSurface[i + 1]->Blt failed, ")
                            TEXT("hr = 0x%x"), hr));
                    break;
                }
            }
        }
    }

    if (ppDDrawSurface) {
        for (i = 0; i < dwBackBufferCount + 1; i++) {
            if (ppDDrawSurface[i]) {
                ppDDrawSurface[i]->Release();
            }
        }
    }

    if (hr != DD_OK) {
        DbgLog((LOG_ERROR, 1, TEXT("PaintSurfaceBlack failed")));
        hr = S_OK;
    }

    return hr;
}

//----------------------------------------------------------------------------
//  File:   DDrawSupport.cpp
//
//  Desc:   DirectShow sample code
//          Implementation of DDrawObject that provides basic DDraw functionality
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------
#include "project.h"
#include <d3d.h>

//----------------------------------------------------------------------------
//  Initialize
//
//  Initialize DDrawObject by getting to fullscreen exclusive mode and 
//  setting front and back buffer
//----------------------------------------------------------------------------
HRESULT
CDDrawObject::Initialize(HWND hWndApp)
{
    HRESULT         hr;
    DDSURFACEDESC2  ddsd;

    m_hwndApp = hWndApp;

    hr = DirectDrawCreateEx(NULL, (LPVOID *)&m_pDDObject,
                            IID_IDirectDraw7, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    // Get into fullscreen exclusive mode
    hr = m_pDDObject->SetCooperativeLevel(hWndApp,
                                          DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN |
                                          DDSCL_ALLOWREBOOT);
    if (FAILED(hr)) {
        Terminate();
        return hr;
    }

    hr = m_pDDObject->SetDisplayMode(SCRN_WIDTH, SCRN_HEIGHT, SCRN_BITDEPTH,
                                     0, DDSDM_STANDARDVGAMODE);

    if (FAILED(hr)) {
        Terminate();
        return hr;
    }


    // Create the primary surface with 1 back buffer
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
                          DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
    ddsd.dwBackBufferCount = 2;
    hr = m_pDDObject->CreateSurface(&ddsd, &m_pPrimary, NULL);
    if (FAILED(hr)) {
        Terminate();
        return hr;
    }

    // Get a pointer to the back buffer
    DDSCAPS2  ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
    hr = m_pPrimary->GetAttachedSurface(&ddscaps, &m_pBackBuff);
    if (FAILED(hr)) {
        Terminate();
        return hr;
    }


    // Get the screen size and save it as a rect
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    hr = m_pPrimary->GetSurfaceDesc(&ddsd);
    if (!(SUCCEEDED(hr) &&
         (ddsd.dwFlags & DDSD_WIDTH) && (ddsd.dwFlags & DDSD_HEIGHT))) {

        Terminate();
        return hr;
    }

    SetRect(&m_RectScrn, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

    return S_OK;
}

//----------------------------------------------------------------------------
// Terminate
//
// return from exclusive mode
//----------------------------------------------------------------------------
HRESULT
CDDrawObject::Terminate()
{

    if (m_pBackBuff) {
        m_pBackBuff->Release();
        m_pBackBuff = NULL;
    }

    if (m_pPrimary) {
        m_pPrimary->Release();
        m_pPrimary = NULL;
    }

    if (m_pDDObject) {

        m_pDDObject->SetCooperativeLevel(m_hwndApp, DDSCL_NORMAL) ;
        m_pDDObject->Release();
        m_pDDObject = NULL;
    }

    return S_OK;
}

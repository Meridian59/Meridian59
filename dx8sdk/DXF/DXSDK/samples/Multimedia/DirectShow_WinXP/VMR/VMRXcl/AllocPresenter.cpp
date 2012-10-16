//----------------------------------------------------------------------------
//  File:   AllocPresenter.cpp
//
//  Desc:   DirectShow sample code
//          Implementation of user-provided allocator-presenter for VMR
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#include "project.h"
#include <mmreg.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "resrc1.h"
#include "D3DTextr.h"
#include "utils.h"

#ifndef __INITDDSTRUCT_DEFINED
#define __INITDDSTRUCT_DEFINED
template <typename T>
__inline void INITDDSTRUCT(T& dd)
{
    ZeroMemory(&dd, sizeof(dd));
    dd.dwSize = sizeof(dd);
}
#endif


//----------------------------------------------------------------------------
// CreateDefaultAllocatorPresenter
//
// creates user-provides allocator presenter
// 
// Usually you have to actually override several functions of AllocatorPresenter;
// For the rest, QI IVMRImagePresenter and call default functions
//----------------------------------------------------------------------------
HRESULT
CMpegMovie::CreateDefaultAllocatorPresenter(
    LPDIRECTDRAW7 lpDD,
    LPDIRECTDRAWSURFACE7 lpPS
    )
{
    HRESULT hr = S_OK;
    IVMRImagePresenterExclModeConfig* lpConfig = NULL;

    __try {
        // for exclusive mode, we do need AllocPresenterDDXclMode of IVMRSurfaceAllocator
        CHECK_HR(hr = CoCreateInstance(CLSID_AllocPresenterDDXclMode, NULL,
                              CLSCTX_INPROC_SERVER,
                              __uuidof(IVMRSurfaceAllocator),
                              (LPVOID*)&m_lpDefSA));

        CHECK_HR(hr = m_lpDefSA->QueryInterface(__uuidof(IVMRImagePresenterExclModeConfig),
                                                (LPVOID*)&lpConfig));

        CHECK_HR(hr = lpConfig->SetRenderingPrefs(RenderPrefs_ForceOffscreen));
        
        // this sets exclusive mode
        CHECK_HR(hr = lpConfig->SetXlcModeDDObjAndPrimarySurface(lpDD, lpPS));

        CHECK_HR(hr = m_lpDefSA->QueryInterface(__uuidof(IVMRImagePresenter),
                                                (LPVOID*)&m_lpDefIP));

        CHECK_HR(hr = m_lpDefSA->QueryInterface(__uuidof(IVMRWindowlessControl),
                                                (LPVOID*)&m_lpDefWC));

        CHECK_HR(hr = m_lpDefWC->SetVideoClippingWindow(m_hwndApp));
        CHECK_HR(hr = m_lpDefSA->AdviseNotify(this));
    }
    __finally {

        RELEASE(lpConfig);

        if (FAILED(hr)) {
            RELEASE(m_lpDefWC);
            RELEASE(m_lpDefIP);
            RELEASE(m_lpDefSA);
        }
    }

    return hr;
}


//----------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::NonDelegatingQueryInterface(
    REFIID riid,
    void** ppv
    )
{
    if (riid == __uuidof(IVMRSurfaceAllocator)) {
        return GetInterface((IVMRSurfaceAllocator*)this, ppv);
    }
    else if (riid == __uuidof(IVMRImagePresenter)) {
        return GetInterface((IVMRImagePresenter*)this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}

//////////////////////////////////////////////////////////////////////////////
//
// IVMRSurfaceAllocator-overriden functions
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// AllocateSurfaces
//
// call default AllocateSurface
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::AllocateSurface(
    DWORD_PTR dwUserID,
    VMRALLOCATIONINFO* lpAllocInfo,
    DWORD* lpdwBuffer,
    LPDIRECTDRAWSURFACE7* lplpSurface
    )
{
    HRESULT hr = m_lpDefSA->AllocateSurface(dwUserID, lpAllocInfo,
                                      lpdwBuffer, lplpSurface);
    if( SUCCEEDED(hr))
    {
        m_lpSurf = *lplpSurface;
    }
    return hr;
}


//----------------------------------------------------------------------------
// FreeSurfaces()
//
// Call default FreeSurface
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::FreeSurface(
    DWORD_PTR dwUserID
    )
{
    HRESULT hr = m_lpDefSA->FreeSurface(dwUserID);

    if( SUCCEEDED(hr))
    {
        m_lpSurf = NULL;
    }
    return hr;
}



//----------------------------------------------------------------------------
// PrepareSurface
//
// call default PrepareSurface
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::PrepareSurface(
    DWORD_PTR dwUserID,
    LPDIRECTDRAWSURFACE7 lplpSurface,
    DWORD dwSurfaceFlags
    )
{
    return m_lpDefSA->PrepareSurface(dwUserID, lplpSurface, dwSurfaceFlags);
}



//----------------------------------------------------------------------------
// AdviseNotify
//
// call default AdviseNotify
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::AdviseNotify(
    IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify
    )
{
    return m_lpDefSA->AdviseNotify(lpIVMRSurfAllocNotify);
}


//////////////////////////////////////////////////////////////////////////////
//
// IVMRSurfaceAllocatorNotify-overriden functions
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// AdviseSurfaceAllocator
//
// standard AdviseSurfaceAllocator
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::AdviseSurfaceAllocator(
    DWORD_PTR dwUserID,
    IVMRSurfaceAllocator* lpIVRMSurfaceAllocator
    )
{
    return m_lpDefSAN->AdviseSurfaceAllocator(dwUserID, lpIVRMSurfaceAllocator);
}


//----------------------------------------------------------------------------
// SetDDrawDevice
//
// standard SetDDrawDevice
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::SetDDrawDevice(LPDIRECTDRAW7 lpDDrawDevice,HMONITOR hMonitor)
{
    return m_lpDefSAN->SetDDrawDevice(lpDDrawDevice, hMonitor);
}


//----------------------------------------------------------------------------
// ChangeDDrawDevice
//
// standard ChangeDDrawDevice
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::ChangeDDrawDevice(LPDIRECTDRAW7 lpDDrawDevice,HMONITOR hMonitor)
{
    return m_lpDefSAN->ChangeDDrawDevice(lpDDrawDevice, hMonitor);
}


//----------------------------------------------------------------------------
// RestoreDDrawSurfaces
//
// standard RestoreDDrawSurfaces
//----------------------------------------------------------------------------
STDMETHODIMP CMpegMovie::RestoreDDrawSurfaces()
{
    // Make sure that the menu is redrawn
    if( m_AlphaBlt )
        m_AlphaBlt->SetMenuRestoreFlag();

    return m_lpDefSAN->RestoreDDrawSurfaces();
}

//----------------------------------------------------------------------------
// NotifyEvent
//
// standard NotifyEvent
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::NotifyEvent(LONG EventCode, LONG_PTR lp1, LONG_PTR lp2)
{
    return m_lpDefSAN->NotifyEvent(EventCode, lp1, lp2);
}


//----------------------------------------------------------------------------
// SetBorderColor
//
// default SetBorderColor
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::SetBorderColor(
    COLORREF clr
    )
{
    return m_lpDefSAN->SetBorderColor(clr);
}


//////////////////////////////////////////////////////////////////////////////
//
// IVMRImagePresenter overriden functions
// we perform all user customization here
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// StartPresenting()
//
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::StartPresenting(DWORD_PTR dwUserID)
{
    return m_lpDefIP->StartPresenting(dwUserID);
}


//----------------------------------------------------------------------------
// StopPresenting()
//
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::StopPresenting(DWORD_PTR dwUserID)
{
    return m_lpDefIP->StopPresenting(dwUserID);
}


//----------------------------------------------------------------------------
// PresentImage
//
// Here all the fun happens. lpPresInfo contains surface with current video image
// Call m_AlphaBlt->AlphaBlt to perform all the necessary transformation
//----------------------------------------------------------------------------
STDMETHODIMP
CMpegMovie::PresentImage(
    DWORD_PTR dwUserID,
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    // clear the background
    DDBLTFX ddFX;
    INITDDSTRUCT(ddFX);

    RECT rcS = {0, 0, 640, 480};
    RECT rcD = {128, 96, 512, 384};
    RECT rcDt ={128, 0, 512, 288};

    m_lpDefWC->GetVideoPosition(&rcS, NULL);
    if( g_ss.bShowTwist )
    {
        m_AlphaBlt->AlphaBlt(&rcDt, lpPresInfo->lpSurf, &rcS, 0xFF);
    }
    else
    {
        m_AlphaBlt->AlphaBlt(&rcD, lpPresInfo->lpSurf, &rcS, 0xFF);
    }

    m_lpSurf->Flip(NULL,0);

    if( g_ss.bShowStatistics && m_Qp)
    {
        // call IQualProp functions here to get performance statistics
        GetPerformance();
    }

    // Show the scene
    m_pDDObject.GetFB()->Flip(NULL, /*DDFLIP_WAIT*/ 0);

    return S_OK;
}

//----------------------------------------------------------------------------
//  GetPerformance
//
//  Calls IQualProp::get_AvgFrameRate   
//  every 25 frames (to not overload VMR with senseless calculations)
//  and saves this value to g_ss, global SceneSettings structure
// 
//----------------------------------------------------------------------------
void 
CMpegMovie::GetPerformance()
{
    
    static int nCounter = 0;
    static int nAvgFrameRate = 0;
    char szFrameRate[MAX_PATH];
    
    nCounter++;
    if( 25 == nCounter )
    {
        m_Qp->get_AvgFrameRate( &nAvgFrameRate);
        nCounter = 0;
    }
    sprintf( szFrameRate, "FPS: %f  ", (float)nAvgFrameRate/100.f);
    lstrcpy(g_ss.achFPS, TEXT(szFrameRate));
    return;
}


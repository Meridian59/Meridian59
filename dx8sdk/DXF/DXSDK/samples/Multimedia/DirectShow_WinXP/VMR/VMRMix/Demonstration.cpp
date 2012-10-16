//------------------------------------------------------------------------------
// File: Demonstration.CPP
//
// Desc: DirectShow sample code
//       Implementation of CDemonstration, 
//       "special effects" module to represent capabilities of VMR.
//       This class is called from CVMRMixDlg by "Play" button.
//       CDemonstration contains CVMRCore member (VMR engine) through which 
//       it performs initialization of the graph builder and presentation.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Demonstration.h"
#include <math.h>

#define DO_NOT_RUN_YET true

//------------------------------------------------------------------------------
// Perform 
// Desc: Performs presentation. Just call it from the 'parent' dialog
// Return HRESULT
//------------------------------------------------------------------------------
HRESULT CDemonstration::Perform()
{
    HRESULT hr = S_OK;
    clock_t tStart;
    clock_t tCurrent;
    clock_t tUpdate;

    if( false == m_bInitialized )
    {
        hr = Initialize();
    }
    if( FAILED(hr) )
    {
        return hr;
    }

    try
    {
        hr = m_pCore->Play();
        if( FAILED(hr) )
        {
            sprintf( m_szMsg, "Failed to run VMR, method returned %s\n", hresultNameLookup(hr));
            OutputDebugString(m_szMsg);
            return hr;
        }
    }
    catch(...)
    {
        OutputDebugString("Unhandled exception in CDemonstration::Perform()\n");
        ShellAbort(m_pCore);
        return E_POINTER;
    }
    
    tStart = clock();
    tCurrent = clock();
    tUpdate = clock();


    // presentation loop: wait until presentation is over
    // or user closed the window and VMRCore is inactive
    while( (tCurrent - tStart) / CLOCKS_PER_SEC <  m_MList.GetAvgDuration() / 10000000L &&
            m_pCore->IsActive() )
    {
        Sleep(10);
        UpdateStreams(tStart);
        tCurrent = clock();
    }// while

    return S_OK;
}

//------------------------------------------------------------------------------
// Initialize()
// Desc: Initializes demonstration; fills start parameters, 
//       creates the graph for VMR; renders files
// Return: HRESULT
//------------------------------------------------------------------------------
HRESULT CDemonstration::Initialize()
{
    HRESULT hr = S_OK;
    LONGLONG llCurrent = 0L;
    LONGLONG llStop = 0L;
    LONGLONG llDelay = 0L;
    clock_t tStart;

    // calculate playback time
    m_MList.AdjustDuration();   
                                
    m_MList.SetInitialParameters();

    // create VMRCore
    m_pCore = new CVMRCore(m_pDlg, &m_MList ); 

    tStart = clock();

    if( !m_pCore)
    {
        return E_OUTOFMEMORY;
    }

    // here, VMRCore creates the graph and if successfully, runs IMediaControl
    // for the first file from the media list
    hr = m_pCore->Play(DO_NOT_RUN_YET); 
    if(FAILED(hr))
    {
        return hr;
    }

    // if user selected "show bitmap" option, load the bitmap
    // from the resource and apply color key
    if( m_pDlg->IsBitmapToUse())
    {
        hr = SetAlphaBitmapColorKey(IDB_BITMAP_LOGO);

        if(FAILED(hr))
        {
            OutputDebugString("Failed to SetAlphaBitmapColorKey() in CDemonstration::Initialize()\n");
            return E_INVALIDARG;
        }
    }

    m_bInitialized = true;

    return S_OK;
}

//------------------------------------------------------------------------------
//  SetAlphaBitmapColorKey
//  Desc: initializes proper members of VMRALPHABITMAP for solor key support
//  Parameters: [in]    UINT ImageID - resource ID
//  Return: HRESULT
//------------------------------------------------------------------------------
HRESULT CDemonstration::SetAlphaBitmapColorKey(UINT ImageID)
{
    HRESULT hr;

    if( ! m_pCore )
    {
        OutputDebugString("Function SetAlphaBitmapColorKey() is called before initializing m_pCore\n");
        return E_INVALIDARG;
    }
    // prepare the valid default bitmap 
    if (GetValidVMRBitmap(ImageID) != FNS_PASS)
    {
        OutputDebugString("Unable to get a valid VMRALPHABITMAP\n");
        return E_INVALIDARG;
    }

    m_sBmpParams.fAlpha = 1.0f;
    m_sBmpParams.clrSrcKey = RGB(255,255,255);
    m_sBmpParams.dwFlags |= VMRBITMAP_SRCCOLORKEY;

    try 
    {
        hr = m_pCore->GetMixerBitmap()->SetAlphaBitmap(&m_sBmpParams);

        if( FAILED(hr) )
        {
            return hr;
        }
    } // try
    catch(...)
    {
        return ShellAbort(m_pCore); 
    }
    return S_OK;
}

//------------------------------------------------------------------------------
//  GetValidVMRBitmap
//  Desc: creates bitmap compatible with VMR
//  Parameters: [in]    UINT ImageID - resource ID
//              [out]   VMRALPHABITMAP * vmrBmp - what IVMRMixerBitmap uses 
//  Return: HRESULT
//------------------------------------------------------------------------------
HRESULT CDemonstration::GetValidVMRBitmap(UINT ImageID)
{
    HRESULT hr = S_OK;
    long cx, cy;
    HDC hdc;
    HBITMAP hbmpVmr;
    BITMAP bmp;

    hr = m_pCore->GetVMRWndless()->GetNativeVideoSize(&cx, &cy, NULL, NULL);
    if( FAILED(hr) )
    {
        OutputDebugString("Failed in GetNativeVideoSize()\n");
        return hr;
    }

    // get compatible DC
    hdc = GetDC(m_pCore->GetClientHwnd());
    
    m_hbmp = CreateCompatibleBitmap(hdc, cx, cy); /*** RELEASE ***/
    
    hbmpVmr = (HBITMAP)LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(ImageID),IMAGE_BITMAP,0,0,/*LR_LOADFROMFILE|*/LR_CREATEDIBSECTION);
    if( !hbmpVmr )
    {
        OutputDebugString("Failed to load resource\n");     
        DeleteObject(m_hbmp);
        return E_INVALIDARG;
    }

    // Get size of the bitmap
    GetObject( hbmpVmr, sizeof(bmp), &bmp );
    
    HDC hdcBmp = CreateCompatibleDC(hdc); /*** RELEASE ***/
    HDC hdcVMR = CreateCompatibleDC(hdc);

    ReleaseDC(m_pCore->GetClientHwnd(), hdc);

    HBITMAP hbmpold = (HBITMAP)SelectObject(hdcBmp, m_hbmp);// in hdcBmp, select hbmp
    hbmpVmr = (HBITMAP)SelectObject(hdcVMR, hbmpVmr);// in hdcVmr, select hbmpVmr (the pic we loaded)

    BitBlt(hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcVMR, 0, 0, SRCPAINT);// put loaded pic from hdcVmr to hdcBmp
    DeleteObject(SelectObject(hdcVMR, hbmpVmr));// ok, we do not need hbmpVmr any more
    DeleteDC(hdcVMR);

    RECT rc;

    ZeroMemory(&m_sBmpParams, sizeof(VMRALPHABITMAP) );
    m_sBmpParams.dwFlags = VMRBITMAP_HDC;
    m_sBmpParams.hdc = hdcBmp;

    // set source rectangle (entire original bitmap)
    SetRect(&rc, 0, 0, bmp.bmWidth, bmp.bmHeight);
    m_sBmpParams.rSrc = rc;

    float fCoeff = 0.2f + 1.7f * (float)rand()/RAND_MAX;
    // set destination rectangle (keeping aspect ratio of the original image)
    // please note that normalized rect is always in [0.0, 1.0] range for
    // all its members
    m_sBmpParams.rDest.left = 0.f;  
    m_sBmpParams.rDest.top = 0.f;
    m_sBmpParams.rDest.right = 0.9f*(float)bmp.bmWidth / (float)cx;
    m_sBmpParams.rDest.bottom = 0.9f*(float)bmp.bmHeight / (float)cy;
    m_sBmpParams.fAlpha = 0.5f;

    // quite important, otherwise VMR would give error
    m_sBmpParams.pDDS = NULL;

    // Note: this demo uses bitmap directly, but often it is better to create
    // DirectDrawSurface of appropriate format and set m_sBmpParams.pDDS to it 
    // (especially if you experience performance issues)

    return S_OK;
}


//------------------------------------------------------------------------------
//  UpdateStreams
//  Desc: updates presentation parameters (destination rectangles and alpha level)
//        for each media file
//  Parameters: clock_t tStart -- presentation start; used as a 'time' variable 
//              in calculations
//  Return: HRESULT
//------------------------------------------------------------------------------
HRESULT CDemonstration::UpdateStreams(clock_t tStart)
{
    HRESULT hr = S_OK;
    int i;
    clock_t tCurrent;
    NORMALIZEDRECT rectD0;
    NORMALIZEDRECT rectD;
    NORMALIZEDRECT rectDRes; 
    double Alpha0;
    double Alpha;

    ASSERT( tStart >0 );

    for( i=0; i< this->m_MList.Size(); i++)
    {
        if( false == m_MList.GetItem(i)->m_bInUse)
            continue;

        tCurrent = clock() - tStart;

        rectD0 = m_MList.GetItem(i)->m_rD;
        Alpha0 = m_MList.GetItem(i)->m_fAlpha;
        
        Alpha = 1.f;
        rectD.left = rectD.top = 0.f;
        rectD.right = rectD.bottom = 1.f;

        FountainPath(   tCurrent, 
                        (long)(m_MList.GetAvgDuration() / 10000000 * CLOCKS_PER_SEC),
                        i,
                        rectD0,
                        Alpha0, 
                        &rectD, 
                        &Alpha);
            
        hr = m_pCore->GetMixerControl()->SetAlpha(i, (float)Alpha);
        if( !SUCCEEDED(hr))
        {
            sprintf( m_szMsg, "Failure in CDemonstration::UpdateStreams, GetMixerControl()->SetAlpha, method returned %s\n",
                hresultNameLookup(hr));
            OutputDebugString(m_szMsg);
        }
        hr = m_pCore->GetMixerControl()->SetOutputRect(i, &rectD);
        if( !SUCCEEDED(hr))
        {
            sprintf( m_szMsg, "Failure in CDemonstration::UpdateStreams, GetMixerControl()->SetOutputRect, method returned %s\n",
                hresultNameLookup(hr));
            OutputDebugString(m_szMsg);
        }

        hr = m_pCore->GetMixerControl()->GetOutputRect(i, &rectDRes);
        if( !SUCCEEDED(hr))
        {
            sprintf( m_szMsg, "Failure in CDemonstration::UpdateStreams, GetMixerControl()->GetOutputRect, method returned %s\n",
                hresultNameLookup(hr));
            OutputDebugString(m_szMsg);
        }
        else
        {
            if( fabs(rectD.top - rectDRes.top) > 0.01 ||
                fabs(rectD.bottom - rectDRes.bottom) > 0.01 ||
                fabs(rectD.left - rectDRes.left) > 0.01 ||
                fabs(rectD.right - rectDRes.right) > 0.01  )
            {
                sprintf( m_szMsg, "Failed invalidation of SetOutputRect(): required [l,t,r,b] = [%f,%f,%f,%f] and returned [%f,%f,%f,%f]\n",
                    rectD.left, rectD.top, rectD.right, rectD.bottom,
                    rectDRes.left, rectDRes.top, rectDRes.right, rectDRes.bottom );
                OutputDebugString(m_szMsg);
            }
        }// else

    }// for

    return hr;
}


//------------------------------------------------------------------------------
// Name: FountainPath
// Purpose: creates 'movie fountain' effect; for each stream, center point 
//          moves by a random ellipse around the center of a playback window;
//          size and alpha-level of stream's output rect changes by cosine with 
//          some random initial delay (to desynchronize streams) 
// Parameters:  
//              [IN]    long t - current timestamp (any units, we use relative time)
//              [IN]    long T - expected total playback time (in the same units as t)
//              [IN]    int n - id of particular stream we want to set
//              [IN]    NORMALIZEDRECT r0 - original OutputRect of the stream
//              [IN]    double A0 - original alpha level (used as a parameter for smoothly 
//                                  changing alpha level)
//              [OUT]   NORMALIZEDRECT * pR - dest. part of playback window
//              [OUT]   double * pA - new alpha level
//------------------------------------------------------------------------------
void CDemonstration::FountainPath(  long t, 
                                    long T, 
                                    int n, 
                                    NORMALIZEDRECT r0,  
                                    double A0, 
                                    NORMALIZEDRECT * pR, 
                                    double * pA)
{
    double cx0;     // original center of the output rect, in normalized coordinates
    double cy0;
    double cx;      // new center of the output rect, in normalized coordinates
    double cy;
    double cx1;     // auxiliary center of the output rect, in normalized coordinates
    double cy1;
    double L0;      // original half-diagonal measure of the rectangle
    double w;       // orig. rect width
    double h;       // orig. rect height
    double beta;    // shift in cosine for L(t). see below
    double L;       // new half-diagonal measure of the rectangle
    double tau;     // time in relative units
    double gamma;   // shift in sine for A(t). see below
    double coeff;   // coefficient that is used to create 'mirror-flip' effect
    double dx;      // new half-width
    double dy;      // new half-heights

    // relative time,  to have about 3 periods over the total playtime
    tau = 18.0 * (double)t / T;

    // alpha level, A = 0.2 + 0.8 sin( tau + gamma) where gamma is such that A(0)=A0
    gamma = (A0 - 0.2)/0.8;
    gamma = (gamma < -1.) ? -1. : gamma;
    gamma = (gamma > 1. ) ?  1. : gamma;
    gamma = asin( gamma);
    *pA = 0.6 + 0.4 * sin(tau + gamma + A0);

    cx0 = (r0.left + r0.right)/2.;
    cy0 = (r0.top + r0.bottom)/2.;
    w = r0.right - r0.left;
    h = r0.bottom - r0.top;
    L0 = 0.5 * sqrt(w*w + h*h);

    L0 = (L0 < 0.0001) ? 0.1 : L0;
    // now rectangle. Its half-diagonal measure L = 0.1 + 0.7 cos( tau + beta)
    // where beta is such that L(0) = LO;
    beta = (L0 - 0.1)/0.7;
    beta = (beta < -1.) ? -1. : beta;
    beta = (beta >  1.) ?  1. : beta;
    beta = acos( beta);
    L = 0.35 + 0.45 * cos( tau + beta + 3.*A0);

    // center of the rectangle is moving by ellips
    // cx = cx0 + (-1)^n 0.1 sin(tau);
    // cy = cy0 - 0.2 + 0.2 cos( tau);
    // and turn it buy... say, theta = 7.85 A0 - 1.57;
    coeff = (n%2) ? -1. : 1.;
    cx1 = cx0 + coeff * 0.2 * sin(tau + A0);
    cy1 = cy0 - 0.05 + 0.2 * cos( tau + A0);

    // the lines below are unnecessary, but we could want some
    // additional transformation here, like turn trajectory ellipse
    // by some angle
    cx = cx1;
    cy = cy1;

    dx = L * w / L0;
    dy = L * h / L0;

    pR->left    = (float)(cx - dx);
    pR->right   = (float)(cx + dx);
    pR->top     = (float)(cy - dy);
    pR->bottom  = (float)(cy + dy);
}



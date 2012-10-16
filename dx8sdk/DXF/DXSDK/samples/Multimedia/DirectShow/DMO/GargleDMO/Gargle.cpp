//------------------------------------------------------------------------------
// File: Gargle.cpp
//
// Desc: DirectShow sample code - implementation of CGargle class.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmobase.h>
#include <initguid.h> // needed to define GUID_TIME_REFERENCE from medparam.h
#include <param.h>
#include "Gargle.h"
#include <uuids.h>    // DirectShow media type guids

#define DEFAULT_GARGLE_RATE 20

#define CHECK_PARAM(lo, hi) \
    if (value < lo || value > hi) {return E_INVALIDARG;} ;

/////////////////////////////////////////////////////////////////////////////
//
// CGargle
//
CGargle::CGargle( ) :
    m_ulShape(0),
    m_ulGargleFreqHz(DEFAULT_GARGLE_RATE),
    m_fDirty(true),
    m_bInitialized(FALSE)
{
    m_pUnkMarshaler = NULL;

    #ifdef DEBUG
        HRESULT hr = Init();
        assert( SUCCEEDED( hr ) );
    #else
        Init();
    #endif
}

const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
//  index           type        caps        min,                        max,                        neutral,                    unit text,  label,          pwchText??
    GFP_Rate,       MPT_INT,    g_capsAll,  GARGLE_FX_RATEHZ_MIN,      GARGLE_FX_RATEHZ_MAX,      20,                         L"Hz",      L"Rate",        L"",
    GFP_Shape,      MPT_ENUM,   g_capsAll,  GARGLE_FX_WAVE_TRIANGLE,   GARGLE_FX_WAVE_SQUARE,     GARGLE_FX_WAVE_TRIANGLE,   L"",        L"WaveShape",   L"Triangle,Square",
};

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::Init
//
HRESULT CGargle::Init()
{
    HRESULT hr = S_OK;
    if( !m_bInitialized )
    {
        hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    }

    if( SUCCEEDED( hr ) )
    {
        // compute the period
        m_ulPeriod = m_ulSamplingRate / m_ulGargleFreqHz;
        m_bInitialized = TRUE;
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::Clone
//
HRESULT CGargle::Clone(IMediaObjectInPlace **ppCloned) 
{
    if (!ppCloned)
        return E_POINTER;

    HRESULT hr = S_OK;
    CGargle * pNewGargle = new CComObject<CGargle>;
    if( !pNewGargle )
        hr = E_OUTOFMEMORY;

    hr = pNewGargle->Init();

    IMediaObject * pCloned = NULL;
    if( SUCCEEDED( hr ) )
    {
        IUnknown *pUnk;
        hr = pNewGargle->QueryInterface( IID_IUnknown, (void **) &pUnk );
        if( SUCCEEDED( hr ) )
        {
            hr = pUnk->QueryInterface( IID_IMediaObject, (void **) &pCloned );
            pUnk->Release();
        }
    }
    else
    {
        return hr;
    }

    //
    // Copy parameter control information
    //
    if (SUCCEEDED(hr))
        hr = pNewGargle->CopyParamsFromSource((CParamsManager *) this);

    // Copy current parameter values
    GargleFX params;
    if (SUCCEEDED(hr))
        hr = GetAllParameters(&params);

    if (SUCCEEDED(hr))
        hr = pNewGargle->SetAllParameters(&params);

    if (SUCCEEDED(hr))
    {
        // Copy the input and output types
        DMO_MEDIA_TYPE mt;
        DWORD cInputStreams = 0;
        DWORD cOutputStreams = 0;
        GetStreamCount(&cInputStreams, &cOutputStreams);

        for (DWORD i = 0; i < cInputStreams && SUCCEEDED(hr); ++i)
        {
            hr = GetInputCurrentType(i, &mt);
            if (hr == DMO_E_TYPE_NOT_SET)
            {
                hr = S_OK; // great, don't need to set the cloned DMO
            }
            else if (SUCCEEDED(hr))
            {
                hr = pCloned->SetInputType(i, &mt, 0);
                MoFreeMediaType( &mt );
            }
        }

        for (i = 0; i < cOutputStreams && SUCCEEDED(hr); ++i)
        {
            hr = GetOutputCurrentType(i, &mt);
            if (hr == DMO_E_TYPE_NOT_SET)
            {
                hr = S_OK; // great, don't need to set the cloned DMO
            }
            else if (SUCCEEDED(hr))
            {
                hr = pCloned->SetOutputType(i, &mt, 0);
                MoFreeMediaType( &mt );
            }
        }

        if (SUCCEEDED(hr))
            hr = pCloned->QueryInterface(IID_IMediaObjectInPlace, (void**)ppCloned);

        // Release the object's original ref.  If clone succeeded (made it through QI) then returned pointer
        // has one ref.  If we failed, refs drop to zero, freeing the object.
        pCloned->Release();
    }
    return hr;
}


//////////////////////////////////////////////////////////////////////////////
//
// CGargle::GetLatency
//
STDMETHODIMP CGargle::GetLatency(THIS_ REFERENCE_TIME *prt)
{
    *prt = 0;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::Discontinuity
//
HRESULT CGargle::Discontinuity() {
    m_ulPhase = 0;
    return NOERROR;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::FBRProcess
//
HRESULT CGargle::FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut) {
    if (!m_bInitialized)
        return DMO_E_TYPE_NOT_SET;

    DWORD cSample, cChannel;
    for (cSample = 0; cSample < cQuanta; cSample++) 
    {
        // If m_Shape is 0 (triangle) then we multiply by a triangular waveform
        // that runs 0..Period/2..0..Period/2..0... else by a square one that
        // is either 0 or Period/2 (same maximum as the triangle) or zero.
        //
        // m_Phase is the number of samples from the start of the period.
        // We keep this running from one call to the next,
        // but if the period changes so as to make this more
        // than Period then we reset to 0 with a bang.  This may cause
        // an audible click or pop (but, hey! it's only a sample!)
        //
        ++m_ulPhase;
        if (m_ulPhase > m_ulPeriod)
            m_ulPhase = 0;

        ULONG ulM = m_ulPhase;      // m is what we modulate with

        if (m_ulShape == 0) {   // Triangle
            if (ulM > m_ulPeriod / 2)
                ulM = m_ulPeriod - ulM;  // handle downslope
        } else {             // Square wave
            if (ulM <= m_ulPeriod / 2)
                ulM = m_ulPeriod / 2;
            else
                ulM = 0;
        }

        for (cChannel = 0; cChannel < m_cChannels; cChannel++) {
            if (m_b8bit) {
                // sound sample, zero based
                int i = pIn[cSample * m_cChannels + cChannel] - 128;
                // modulate
                i = (i * (signed)ulM * 2) / (signed)m_ulPeriod;
                // 8 bit sound uses 0..255 representing -128..127
                // Any overflow, even by 1, would sound very bad.
                // so we clip paranoically after modulating.
                // I think it should never clip by more than 1
                //
                if (i > 127)
                    i = 127;
                if (i < -128)
                    i = -128;
                // reset zero offset to 128
                pOut[cSample * m_cChannels + cChannel] = (unsigned char)(i + 128);
   
            } else {
                // 16 bit sound uses 16 bits properly (0 means 0)
                // We still clip paranoically
                //
                int i = ((short*)pIn)[cSample * m_cChannels + cChannel];
                // modulate
                i = (i * (signed)ulM * 2) / (signed)m_ulPeriod;
                // clip
                if (i > 32767)
                    i = 32767;
                if (i < -32768)
                    i = -32768;
                ((short*)pOut)[cSample * m_cChannels + cChannel] = (short)i;
            }
        }
   }
   return NOERROR;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetClassID
//
HRESULT CGargle::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = CLSID_Gargle;
    return NOERROR;

} // GetClassID

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::GetPages
//
HRESULT CGargle::GetPages(CAUUID * pPages)
{
    pPages->cElems = 1;
    pPages->pElems = static_cast<GUID *>(CoTaskMemAlloc(sizeof(GUID)));
    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_GargDMOProp;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::SetAllParameters
//
STDMETHODIMP CGargle::SetAllParameters(THIS_ LPCGargleFX pParm)
{
    HRESULT hr = S_OK;
    
    // Check that the pointer is not NULL
    if (pParm == NULL) hr = E_POINTER;

    // Set the parameters
    if (SUCCEEDED(hr)) hr = SetParam(GFP_Rate, static_cast<MP_DATA>(pParm->dwRateHz));
    if (SUCCEEDED(hr)) hr = SetParam(GFP_Shape, static_cast<MP_DATA>(pParm->dwWaveShape));
            
    m_fDirty = true;
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::GetAllParameters
//
STDMETHODIMP CGargle::GetAllParameters(THIS_ LPGargleFX pParm)
{   
    HRESULT hr = S_OK;
    MP_DATA var;

    if (pParm == NULL)
    {
        return E_POINTER;
    }

#define GET_PARAM_DWORD(x,y) \
    if (SUCCEEDED(hr)) { \
        hr = GetParam(x, &var); \
        if (SUCCEEDED(hr)) pParm->y = (DWORD)var; \
    }
    
    GET_PARAM_DWORD(GFP_Rate, dwRateHz);
    GET_PARAM_DWORD(GFP_Shape, dwWaveShape);
    
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::SetParam
//
HRESULT CGargle::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    switch (dwParamIndex)
    {
    case GFP_Rate:
        CHECK_PARAM(GARGLE_FX_RATEHZ_MIN,GARGLE_FX_RATEHZ_MAX);
        m_ulGargleFreqHz = (unsigned)value;
        if (m_ulGargleFreqHz < 1) m_ulGargleFreqHz = 1;
        if (m_ulGargleFreqHz > 1000) m_ulGargleFreqHz = 1000;
        
        // Init is where m_ulPeriod is updated, so call it here
        // Would be better to do this outside of Init though
        Init();          
        break;

    case GFP_Shape:
        CHECK_PARAM(GARGLE_FX_WAVE_TRIANGLE,GARGLE_FX_WAVE_SQUARE);
        m_ulShape = (unsigned)value;
        break;
    }

    // Let base class set this so it can handle all the rest of the param calls.
    // Skip the base class if fSkipPasssingToParamManager.  This indicates that we're calling the function
    //    internally using values that came from the base class -- thus there's no need to tell it values it
    //    already knows.
    return fSkipPasssingToParamManager ? S_OK : CParamsManager::SetParam(dwParamIndex, value);
}

//////////////////////////////////////////////////////////////////////////////
//
// CGargle::Process
//
HRESULT CGargle::Process(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
    // Update parameter values from any curves that may be in effect.
    // We pick up the current values stored in the CParamsManager helper for time rtStart. 

    // Note that we are using IMediaParams in a less than
    // perfect way. We update at the beginning of every time slice instead of smoothly over the curve.
    // This is okay for an effect like gargle as long as the time slice is consistently small (which 
    // it conveniently is when hosted in DSound.)
    // However, in the future we will update this sample to use a more appropriate and accurate
    // mechanism.
    // Here are some suggestions of how it can be done, with increasing degree of accuracy. Different
    // types of effects and effect parameters require different levels of accuracy, so no solution is the best 
    // solution for all (especially if you are concerned about CPU cost.)
    // 1) Break the time slice up into mini pieces of some number of milliseconds
    // each and run through all the steps in Process for each sub slice. This guarantees the
    // stair stepping is small enough not to be noticable. This approach will work well for parameters
    // that don't create an audible stair stepping noise (or "zipper") noise when controled in this way. 
    // Control over volume, for example, does not work well.
    // 2) Use the above mechanism, but pass the start and end values for each parameter to the
    // processing engine. It, in turn, applies linear interpolation to each parameter. This results
    // in a smooth approximation of the parameter curve and removes all but the most subtle aliasing noise.
    // 3) Pass the curves directly to the processing engine, which accurately calculates each sample
    // mathematically. This is obviously the best, but most complex and CPU intensive.

    this->UpdateActiveParams(rtStart, *this);

    DMO_MEDIA_TYPE mt;
    HRESULT hr = GetInputCurrentType(0, &mt);
    if( FAILED( hr ) )
        return hr;

    // convert bytes to samples for the FBRProcess call
    assert(mt.formattype == FORMAT_WaveFormatEx);
    ulQuanta /= LPWAVEFORMATEX(mt.pbFormat)->nBlockAlign;
    MoFreeMediaType( &mt );
    return FBRProcess(ulQuanta, pcbData, pcbData);
}


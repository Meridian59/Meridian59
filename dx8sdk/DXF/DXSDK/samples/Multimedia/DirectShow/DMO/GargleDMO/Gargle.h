//------------------------------------------------------------------------------
// File: Gargle.h
//
// Desc: DirectShow sample code - definition of CGargle class.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------


#ifndef __GARGLE_H_
#define __GARGLE_H_

#include "resource.h"       // main symbols
#include "igargle.h"        // interface to us
            
// {458F93F3-4769-4370-8A7D-D055EE97679C}
DEFINE_GUID(CLSID_Gargle, 
0x458f93f3, 0x4769, 0x4370, 0x8a, 0x7d, 0xd0, 0x55, 0xee, 0x97, 0x67, 0x9c);

// {3E7BB80F-2245-4483-ACC5-3A2EFDE0ACA6}
DEFINE_GUID(CLSID_GargDMOProp, 
0x3e7bb80f, 0x2245, 0x4483, 0xac, 0xc5, 0x3a, 0x2e, 0xfd, 0xe0, 0xac, 0xa6);

enum GargleFilterParams
{
    GFP_Rate = 0,
    GFP_Shape,
    GFP_MAX
};

/////////////////////////////////////////////////////////////////////////////
// CGargle
class ATL_NO_VTABLE CGargle : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGargle, &CLSID_Gargle>,
    public CPCMDMO,
    public ISpecifyPropertyPages,
    public IPersistStream,
    public IMediaObjectInPlace,
    public CParamsManager,
    public CParamsManager::UpdateCallback,
    public IGargleDMOSample // property page uses this to talk to us
{
public:
	CGargle();
    HRESULT Init();

DECLARE_REGISTRY_RESOURCEID(IDR_GARGLE)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGargle)
	COM_INTERFACE_ENTRY(IMediaObject)
    COM_INTERFACE_ENTRY(IMediaObjectInPlace)
    COM_INTERFACE_ENTRY(IMediaParams)
    COM_INTERFACE_ENTRY(IMediaParamInfo)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY_IID(IID_IGargleDMOSample, IGargleDMOSample)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;
    
    HRESULT FBRProcess(DWORD cSamples, BYTE *pIn, BYTE *pOut);

    // override
    HRESULT Discontinuity();

    bool m_fDirty;

    // IGargleDMOSample
    STDMETHOD(SetAllParameters)     (THIS_ LPCGargleFX);
    STDMETHOD(GetAllParameters)     (THIS_ LPGargleFX);

    // IMediaObjectInPlace
    STDMETHODIMP Process(THIS_ ULONG ulSize, BYTE *pData, REFERENCE_TIME rtStart, DWORD dwFlags);
    STDMETHODIMP GetLatency(THIS_ REFERENCE_TIME *prt);
    STDMETHODIMP Clone(IMediaObjectInPlace **pp); 

    // SetParam handling
    STDMETHODIMP SetParam(DWORD dwParamIndex,MP_DATA value) { return SetParamInternal(dwParamIndex, value, false); }
    HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) { return SetParamInternal(dwParamIndex, value, true); }
    HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager);

    // ISpecifyPropertyPages
    STDMETHODIMP GetPages(CAUUID * pPages);

    /* IPersist */
    STDMETHODIMP GetClassID(THIS_ CLSID *pClassID);
    
    // IPersistStream
    STDMETHOD(IsDirty)(void) { return m_fDirty ? S_OK : S_FALSE; }

    STDMETHOD(Load)(IStream *pStm) 
    { 
        ULONG cbRead;
        HRESULT hr = S_OK;

        if (pStm==NULL)
        	return E_POINTER;

        GargleFX garglefx;
        hr = pStm->Read((void *)&garglefx, sizeof(garglefx), &cbRead);
        if (hr != S_OK || cbRead < sizeof(garglefx))
            return E_FAIL;
        hr = SetAllParameters(&garglefx);
        m_fDirty = false;
        return hr;
    }

    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) 
    { 
        HRESULT hr = S_OK; 

        if (pStm==NULL)
        	return E_POINTER;

        GargleFX garglefx;
        hr = GetAllParameters(&garglefx);
        if (FAILED(hr))
            return hr;

        ULONG cbWritten;
        hr = pStm->Write((void *)&garglefx, sizeof(garglefx), &cbWritten);
        if (hr != S_OK || cbWritten < sizeof(garglefx))
            return E_FAIL;

        if (fClearDirty)
            m_fDirty = false;

        return S_OK;
    }

    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) 
    { 
        if (!pcbSize) 
            return E_POINTER; 

        pcbSize->QuadPart = sizeof(GargleFX); 
        return S_OK; 
    }

private:
   // gargle params
   ULONG m_ulShape;
   ULONG m_ulGargleFreqHz;

   // gargle state
   ULONG m_ulPeriod;
   ULONG m_ulPhase;

   BOOL m_bInitialized;

};

#endif //__GARGLE_H_

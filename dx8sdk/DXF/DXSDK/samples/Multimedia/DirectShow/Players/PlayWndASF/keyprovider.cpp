//------------------------------------------------------------------------------
// File: KeyProvider.cpp
//
// Desc: DirectShow sample code - provides a class to unkey Windows Media
//       for use with ASF, WMA, WMV media files.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <atlbase.h>
#include <atlimpl.cpp>
#include <stdio.h>

#include <dshowasf.h>
#include "keyprovider.h"


//
// Build warning to remind developers of the dependency on the 
// Windows Media Format SDK libraries, which do not ship with
// the DirectX SDK.
//
#pragma message("NOTE: To link and run this sample, you must install the Windows Media Format SDK.")
#pragma message("After signing a license agreement with Microsoft, you will receive a")
#pragma message("unique version of WMStub.LIB, which should be added to this VC++ project.")
#pragma message("Without this library, you will receive linker errors for the following:")
#pragma message("       WMCreateCertificate")
#pragma message("You must also add WMVCore.LIB to the linker settings to resolve the following:")
#pragma message("       WMCreateProfileManager")


CKeyProvider::CKeyProvider() : m_cRef(0)
{
}

//////////////////////////////////////////////////////////////////////////
//
// IUnknown methods
//
//////////////////////////////////////////////////////////////////////////

ULONG CKeyProvider::AddRef()
{
    return ++m_cRef;
}

ULONG CKeyProvider::Release()
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if(m_cRef == 0)
    {
        delete this;

        // don't return m_cRef, because the object doesn't exist anymore
        return((ULONG) 0);
    }

    return(m_cRef);
}

//
// QueryInterface
//
// We only support IUnknown and IServiceProvider
//
HRESULT CKeyProvider::QueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IServiceProvider || riid == IID_IUnknown)
    {
        *ppv = (void *) static_cast<IServiceProvider *>(this);
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP CKeyProvider::QueryService(REFIID siid, REFIID riid, void **ppv)
{
    if(siid == __uuidof(IWMReader) && riid == IID_IUnknown)
    {
        IUnknown *punkCert;

        HRESULT hr = WMCreateCertificate(&punkCert);

        if(SUCCEEDED(hr))
            *ppv = (void *) punkCert;
        else
            printf("CKeyProvider::QueryService failed to create certificate!  hr=0x%x\n", hr);
    
        return hr;
    }

    return E_NOINTERFACE;
}


//------------------------------------------------------------------------------
// File: GargleDMO.cpp
//
// Desc: DirectShow sample code - implementation of DLL exports.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f GargleDMOps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmobase.h>
#include <param.h>
#include "Gargle.h"
#include "GargDMOProp.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Gargle, CGargle)
OBJECT_ENTRY(CLSID_GargDMOProp, CGargDMOProp)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // Register ourselves as a pcm audio effects DMO
    DMO_PARTIAL_MEDIATYPE mt;
    mt.type    = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;

    DMORegister(L"Gargle DMO Sample", 
                CLSID_Gargle, 
                DMOCATEGORY_AUDIO_EFFECT,
                0,
                1,
                &mt,
                1,
                &mt);

    // registers object, but we're not using a typelib
    return _Module.RegisterServer(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}



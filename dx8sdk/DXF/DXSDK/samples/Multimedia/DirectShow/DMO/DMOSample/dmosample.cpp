//------------------------------------------------------------------------------
// File: DMOSample.cpp
//
// Desc: DirectShow sample code - implementation of DLL exports.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"

#define FIX_LOCK_NAME
#include <dmo.h>

#include <dmoimpl.h>
#include "resource.h"
#include <initguid.h>
#include <limits.h>
#include "state.h"
#include "Sample.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Sample, CSample)
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
	// Register ourselves as a DMO with no types
	// RGS file also registers with a merit of 0 so DShow won't try to use us
    DMORegister(L"DMO Sample", 
                CLSID_Sample, 
                DMOCATEGORY_VIDEO_DECODER,
                0,
                0,
                NULL,
                0,
                NULL);

    // registers object
    return _Module.RegisterServer();
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	DMOUnregister(CLSID_Sample, DMOCATEGORY_VIDEO_DECODER);
    return _Module.UnregisterServer();
}



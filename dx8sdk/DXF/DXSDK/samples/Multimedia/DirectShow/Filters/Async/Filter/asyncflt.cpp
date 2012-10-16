//------------------------------------------------------------------------------
// File: AsyncFlt.cpp
//
// Desc: DirectShow sample code - implementation of CAsyncFilter.
//
// Copyright (c) 1996-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <asyncio.h>
#include <asyncrdr.h>
#include <initguid.h>   // Make our GUID get defined

#pragma warning(disable:4710)  // 'function' not inlined (optimization)
#include "asyncflt.h"

// c553f2c0-1529-11d0-b4d1-00805f6cbbea
DEFINE_GUID(CLSID_AsyncSample,
0xc553f2c0, 0x1529, 0x11d0, 0xb4, 0xd1, 0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);

// Our media subtype guid
// 03b01b20-1534-11d0-b4d1-00805f6cbbea
DEFINE_GUID(MEDIASUBTYPE_SampleAsync,
0x03b01b20, 0x1534, 0x11d0, 0xb4, 0xd1, 0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);


//
// Setup data for filter registration
//
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ &MEDIATYPE_Stream     // clsMajorType
, &MEDIASUBTYPE_NULL }; // clsMinorType

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"          // strName
, FALSE              // bRendered
, TRUE               // bOutput
, FALSE              // bZero
, FALSE              // bMany
, &CLSID_NULL        // clsConnectsToFilter
, L"Input"           // strConnectsToPin
, 1                  // nTypes
, &sudOpPinTypes };  // lpTypes

const AMOVIESETUP_FILTER sudAsync =
{ &CLSID_AsyncSample              // clsID
, L"Sample File Source (Async.)"  // strName
, MERIT_UNLIKELY                  // dwMerit
, 1                               // nPins
, &sudOpPin };                    // lpPin


//
//  Object creation template
//
CFactoryTemplate g_Templates[1] = {
    { L"Sample File Source (Async.)"
    , &CLSID_AsyncSample
    , CAsyncFilter::CreateInstance
    , NULL
    , &sudAsync }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//
// Filter registration functions
//
HRESULT DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}




/* Create a new instance of this class */
CUnknown *CAsyncFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    /*  DLLEntry does the right thing with the return code and
        returned value on failure
    */
    return new CAsyncFilter(pUnk, phr);
}


BOOL CAsyncFilter::ReadTheFile(LPCTSTR lpszFileName)
{
    /*  Read the file */
    HANDLE hFile = CreateFile(lpszFileName,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), lpszFileName));
        return FALSE;
    }

    ULARGE_INTEGER uliSize;
    uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

    PBYTE pbMem = new BYTE[uliSize.LowPart];
    if (pbMem == NULL) {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD dwBytesRead;
    if (!ReadFile(hFile,
                  (LPVOID)pbMem,
                  uliSize.LowPart,
                  &dwBytesRead,
                  NULL) ||
        dwBytesRead != uliSize.LowPart) {
        DbgLog((LOG_TRACE, 1, TEXT("Could not read file\n")));
        delete [] pbMem;
        CloseHandle(hFile);
        return FALSE;
    }

    m_pbData = pbMem;
    m_llSize = (LONGLONG)uliSize.QuadPart;
    CloseHandle(hFile);

    return TRUE;
}


//------------------------------------------------------------------------------
// File: stdafx.h
//
// Desc: Include file for standard system include files
//       or project specific include files that are used frequently
//       but are changed infrequently
//
// Copyright (c) 2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_STDAFX_H__CC50DE5B_97DF_4014_9C89_5E079D7AA958__INCLUDED_)
#define AFX_STDAFX_H__CC50DE5B_97DF_4014_9C89_5E079D7AA958__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>

// Disable warning message for C4201 - use of nameless struct/union
// Otherwise, strmif.h will generate warnings for Win32 debug builds
#pragma warning( disable : 4201 )  

// You may derive a class from CComModule and use it if you want to override
// something, but do not change the name of _Module
class CExeModule : public CComModule
{
public:
    LONG Unlock();
    DWORD dwThreadID;
    HANDLE hEventShutdown;
    
    void MonitorShutdown();
    bool StartMonitor();
    bool bActivity;
};

extern CExeModule _Module;

#include <atlcom.h>
#include <atlhost.h>

// Disable C4701: Variable may be used without having been initialized
// This warning is generated in atlctl.h, which is not in our control
#pragma warning( disable : 4701 )

#include <atlctl.h>

#pragma warning( default : 4701 )

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CC50DE5B_97DF_4014_9C89_5E079D7AA958__INCLUDED)

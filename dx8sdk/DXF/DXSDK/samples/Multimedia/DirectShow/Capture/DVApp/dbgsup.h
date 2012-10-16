//------------------------------------------------------------------------------
// File: DbgSup.h
//
// Desc: DirectShow sample code - DV control/capture example
//       Debug macros and supporting functions for Windows programs
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifdef _DEBUG

// dump a string to debug output
#define Dump(tsz) \
    OutputDebugString(tsz)

// dump a string with a parameter value to debug output
TCHAR dbgsup_tszDump[256];
#define Dump1(tsz, arg) \
    { wsprintf(dbgsup_tszDump, (tsz), (arg)); \
      OutputDebugString(dbgsup_tszDump); }


#define CHECK_ERROR(tsz,hr)                     \
{   if( S_OK != hr)                             \
    {                                           \
        wsprintf(dbgsup_tszDump, (tsz), (hr));  \
        OutputDebugString(dbgsup_tszDump);      \
        return hr;                              \
    }                                           \
}
    
#ifndef DBGSUPAPI
#define DBGSUPAPI __declspec(dllimport)
#endif

// dump a Windows message to debug output
DBGSUPAPI void DumpMsg(
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);


#include <assert.h>

// assert an expression
#define Assert(exp)     assert(exp)

#else

// do nothing in retail version
#define Dump(sz)
#define Dump1(sz, arg)
#define DumpMsg(msg, wp, lp)
#define Assert(exp)

#define CHECK_ERROR(tsz,hr)                     \
{   if( S_OK != hr)                             \
        return hr;                              \
}

#endif  // _DEBUG

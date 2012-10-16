//------------------------------------------------------------------------------
// File: Stdafx.h
//
// Desc: DirectShow sample code
//       Include file for standard system include files or project-specific 
//       include files that are used frequently, but are changed infrequently.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_STDAFX_H__62FCB452_013F_45F5_ADDE_C3314ACD44C9__INCLUDED_)
#define AFX_STDAFX_H__62FCB452_013F_45F5_ADDE_C3314ACD44C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif

#define D3D_OVERLOADS

// included files
#pragma warning(push,3)
#include <streams.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <strmif.h>
#include <combase.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <uuids.h>
#pragma warning(pop)
#pragma warning(disable:4100 4189)

#include "resource.h"
#include "sourceinfo.h"
#include "VMRMixDlg.h"
#include "vmrcore.h"
#include "utils.h"


#define DllImport   __declspec( dllimport )

// needed for the surface tests
DEFINE_GUID(IID_IDirectDraw7,
            0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);

// various constants

const ULONG MAXFILTERS = 128;
#define FNS_PASS 0
#define FNS_FAIL 1

// macros
#define SAFERELEASE(punk)    if (NULL != punk)  (punk)->Release(), (punk) = NULL;  else 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__62FCB452_013F_45F5_ADDE_C3314ACD44C9__INCLUDED_)

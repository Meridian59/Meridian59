//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Includes windowing system headers, with necessary macros defined
//----------------------------------------------------------------------------

#if !defined(OWL_WSYSINC_H)
#define OWL_WSYSINC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/private/wsysinc.h>

// !BB Is there a better location for these??
//
// Win 4.0 styles
//
#if !defined(WS_EX_MDICHILD)
#define WS_EX_MDICHILD          0x00000040L
#endif
#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW        0x00000080L
#endif
#if !defined(WS_EX_WINDOWEDGE)
#define WS_EX_WINDOWEDGE        0x00000100L
#endif
#if !defined(WS_EX_CLIENTEDGE)
#define WS_EX_CLIENTEDGE        0x00000200L
#endif
#if !defined(WS_EX_CONTEXTHELP)
#define WS_EX_CONTEXTHELP       0x00000400L
#endif
#if !defined(WS_EX_RIGHT)
#define WS_EX_RIGHT             0x00001000L
#endif
#if !defined(WS_EX_LEFT)
#define WS_EX_LEFT              0x00000000L
#endif
#if !defined(WS_EX_RTLREADING)
#define WS_EX_RTLREADING        0x00002000L
#endif
#if !defined(WS_EX_LTRREADING)
#define WS_EX_LTRREADING        0x00000000L
#endif
#if !defined(WS_EX_LEFTSCROLLBAR)
#define WS_EX_LEFTSCROLLBAR     0x00004000L
#endif
#if !defined(WS_EX_RIGHTSCROLLBAR)
#define WS_EX_RIGHTSCROLLBAR    0x00000000L
#endif
#if !defined(WS_EX_CONTROLPARENT)
#define WS_EX_CONTROLPARENT     0x00010000L
#endif
#if !defined(WS_EX_STATICEDGE)
#define WS_EX_STATICEDGE        0x00020000L
#endif
#if !defined(WS_EX_APPWINDOW)
#define WS_EX_APPWINDOW         0x00040000L
#endif
#if !defined(WS_EX_OVERLAPPEDWINDOW)
#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#endif
#if !defined(WS_EX_PALETTEWINDOW) && defined(WS_EX_TOPMOST)
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
#endif

#endif  // OWL_WSYSINC_H

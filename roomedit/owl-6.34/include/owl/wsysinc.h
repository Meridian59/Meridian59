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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
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


////////////////////////////////////////////////////////////////////////////
// mouse wheel support 
#ifdef UNICODE
#define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#else
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif
   // wParam = wheel rotation expressed in multiples of WHEEL_DELTA
   // lParam is the mouse coordinates

#define WHEEL_DELTA      120      // Default value for rolling one notch

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported
                                        // by the OS 
#endif
// MSH_SCROLL_LINES
//    wParam - not used 
//    lParam - not used
//    returns int  - number of lines to scroll on a wheel roll

#ifndef  WHEEL_PAGESCROLL  
#define WHEEL_PAGESCROLL  (UINT_MAX)   // signifies to scroll a page, also
                                       // defined in winuser.h in the
                                       // NT4.0 SDK
#endif 

#ifndef SPI_SETWHEELSCROLLLINES
#define SPI_SETWHEELSCROLLLINES   105  // Also defined in winuser.h in the
                                       // NT4.0 SDK, please see the NT4.0 SDK
                                       // documentation for NT4.0 implementation
                                       // specifics.
                                       // For Win95 and WinNT3.51, 
                                       // Mswheel broadcasts the message
                                       // WM_SETTINGCHANGE (equivalent to
                                       // WM_WININICHANGE) when the scroll
                                       // lines has changed.  Applications
                                       // will recieve the WM_SETTINGCHANGE
                                       // message with the wParam set to
                                       // SPI_SETWHEELSCROLLLINES.  When
                                       // this message is recieved the application
                                       // should query Mswheel for the new 
                                       // setting.
#endif


#endif  // OWL_WSYSINC_H

//
/// \file
/// Compiler-specific defines for Borland C++ and C++Builder
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#if !defined(OWL_PRIVATE_BORLANDC_H)
#define OWL_PRIVATE_BORLANDC_H

//
// Define specific name of compiler, BI_COMP_xxxx
//
#define BI_COMP_BORLANDC __BORLANDC__

#if defined(OWL5_COMPAT) && !defined(UNICODE)
#define OWL_SUPPORT_BWCC
#define OWL_SUPPORT_CTL3D
#endif

//
// Version-agnostic configuration
//
#define BI_MULTI_THREAD
#if defined(__MT__)
# define BI_MULTI_THREAD_RTL
#endif

//
// Version-specific configuration
//
// Supported versions:
//
//  0x550 - Borland C++ Builder 5.0/Free Borland C++ 5.5
//  0x560 - Borland C++ Builder 6.0
//  0x570 - Borland C++ BuilderX
//  0x580 - Borland C++ Developer Studio 2006 / TC++2006
//  0x590 - CodeGear RAD Studio 2007
//  0x610 - CodeGear RAD Studio 2009
//  0x620 - CodeGear RAD Studio 2010
//  0x630 - Embarcadero RAD Studio XE
//  0x640 - Embarcadero RAD Studio XE2
//  0x660 - Embarcadero RAD Studio XE4
//  0x670 - Embarcadero RAD Studio XE5
//  0x680 - Embarcadero RAD Studio XE6
//  0x690 - Embarcadero RAD Studio XE7
//
#if (__BORLANDC__ < 0x550)
# error Unsupported Borland C++ compiler version 
#endif

#if (__BORLANDC__ < 0x590)
#define BI_NO_STD_OVERLOAD_RESOLUTION
#endif

#if (__BORLANDC__ >= 0x560)
#ifdef UNICODE
# define _USE_OLD_RW_STL      // use old RW STL  for now and only for unicode!!!!!!!!!!!!!!!!!
#endif
#if !defined(BI_OWL_PACKING)
# define BI_OWL_PACKING 8
#endif
#endif

#if (__BORLANDC__ < 0x570)
#define BI_NEED_UXTHEME_H
#endif

#if (__BORLANDC__ < 0x560)
#define BI_NEED_ZMOUSE_H
#endif

#if (__BORLANDC__ >= 0x580)
#define BI_HAS_STREAMWCHAROPEN
#endif

#if (__BORLANDC__ < 0x590)
#define OWL_BREAK {__emit__(0xCC);}
#endif

#if (__BORLANDC__ >= 0x620)
#define BI_HAS_PRAGMA_ONCE
#endif

//
// Debug trap macro and tracing
//
#if !defined(OWL_BREAK)
#define OWL_BREAK {DebugBreak();}
#endif
#define OWL_OUTPUT_DEBUG_STRING(lpsz) ::OutputDebugString(lpsz)

//
// Include link directives
//
#if !defined(_BUILDOWLDLL)
#include <owl/private/owllink.h>
#endif

#endif

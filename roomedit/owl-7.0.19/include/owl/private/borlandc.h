//
/// \file
/// Compiler-specific defines for Clang-based Embarcadero compilers.
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_PRIVATE_BORLANDC_H)
#define OWL_PRIVATE_BORLANDC_H

#if !defined(__clang__)
#error OWLNext: Unsupported compiler.
#endif

//
// Define specific name of compiler, BI_COMP_xxxx
//
#define BI_COMP_BORLANDC __BORLANDC__
#define BI_COMP_CLANG

#if defined(OWL5_COMPAT) && !defined(UNICODE)
#define OWL_SUPPORT_BWCC
#define OWL_SUPPORT_CTL3D
#endif

#define BI_DEFINE_MINMAX_TMPL
#define BI_MULTI_THREAD
#if defined(__MT__)
# define BI_MULTI_THREAD_RTL
#endif

#ifdef UNICODE
# define _USE_OLD_RW_STL      // use old RW STL  for now and only for unicode!!!!!!!!!!!!!!!!!
#endif
#if !defined(BI_OWL_PACKING)
# define BI_OWL_PACKING 8
#endif

#define BI_HAS_PRAGMA_ONCE

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

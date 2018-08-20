//----------------------------------------------------------------------------
// Borland BIDS Container Library Compatibility header
// Copyright (c) 1998, Yura Bidus, All Rights Reserved
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined(CLASSLIB_DEFS_H)
#define CLASSLIB_DEFS_H

#pragma message ("Included obsolete <classlib/defs.h>")

#if !defined(__cplusplus)
# error Must compile Borland Class Library as C++
#endif

#include <owl/defs.h>

//----------------------------------------------------------------------------
// We must use all libraries in DLLs if we are using BIDS in a DLL
//
// Define _BIDSDLL with -WSE (-WE for 32bit) to use BIDS & RTL in dlls.
//
// Be sure _BUILDBIDSDLL is defined when building an actual BIDS dll
//
#if defined(_BIDSDLL) || defined(_CLASSDLL) || defined(_BUILDBIDSDLL) || defined(_RTLDLL)
# if !defined(_BIDSDLL)
#   define _BIDSDLL
# endif
# if !defined(_SVCSDLL)
#   define _SVCSDLL
# endif
# if !defined(_RTLDLL)
#   define _RTLDLL
# endif
#endif

//----------------------------------------------------------------------------
//
//  To restore the old BIDS naming convention of data and function names
//  beginning with lowercase letters, in addition to the new convention of
//  uppercase letters, remove the // from the #define below or define on the
//  cmd line. The libraries do not need to be rebuilt.
//

//#define BI_OLDNAMES         // add names with initial lowercase

//----------------------------------------------------------------------------
//
//  Common definitions for pointer size and calling conventions.
//
//  Calling conventions:
//
//  _BIDSENTRY      Specifies the calling convention used by BIDS.
//
//
//  Export (and size for DOS) information:
//
//  _BIDSCLASS      Exports class if building DLL version of library.
//                  For DOS16 also provides size information.
//
//  _BIDSDATA       Exports data if building DLL version of library.
//
//  _BIDSFUNC       Exports function if building DLL version of library.
//                  For DOS16 also provides size information
//
//----------------------------------------------------------------------------

#define _BIDSCLASS       _OWLCLASS             // _export/import
#define _BIDSCLASS_RTL   _OWLCLASS_RTL         // _export/import
#define _BIDSTEMPL       _OWLCLASS_RTL         // _export/import
#define _BIDSDATA        _OWLCLASS_RTL         // _export/import
#define _BIDSFUNC        _OWLCLASS_RTL         // _export/import

#endif  // CLASSLIB_DEFS_H

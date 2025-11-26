//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// OWLEXT's macro magic and conditional defines
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_DEFS_H)
#define __OWLEXT_DEFS_H

//-------------------------------------------------------------------
// OWLEXT can't be compiled as anything other than C++
#if !defined(__cplusplus)
# error Must compile OWLExt Class Library as C++
#endif

//-------------------------------------------------------------------
// Pick up subsystem headers
#if !defined (OWL_DEFS_H)
# include <owl/defs.h>
#endif

//-------------------------------------------------------------------
// Handle the OWLEXTCLASS macro magic
#if defined (_BUILDOWLEXTDLL)
// OWLEXT being built as a DLL
# define OWLEXTCLASS _OWLEXPORTCLASS
#  define OWLEXTFUNC  _OWLEXPORTFUNC
#  define OWLCEXTFUNC _OWLEXPORTCFUNC
#  define OWLEXTDATA   _OWLEXPORTDATA
// We must use all libraries in DLLs if we are using OWLEXT in a DLL
# if !defined (_OWLDLL)
#  define _OWLDLL
# endif

#elif (defined (BUILD_OWLEXTLIB) || defined (USE_OWLEXTLIB))
// OWLEXT being built or used as a static library
# define OWLEXTCLASS
# define OWLEXTFUNC(p) p
# define OWLEXTDATA(p) p

#elif defined (USE_OWLEXTDLL)
// OWLEXT being used as a DLL
# define OWLEXTCLASS    _OWLIMPORTCLASS
# define OWLEXTFUNC     _OWLIMPORTFUNC
# define OWLCEXTFUNC     _OWLIMPORTCFUNC
# define OWLEXTDATA     _OWLIMPORTDATA
#else
// User forgot to tell us which version they're using
# error You must define either USE_OWLEXTLIB or USE_OWLEXTDLL.

#endif

#if defined(USE_OWLEXTLIB) && defined(USE_OWLEXTDLL)
# error "Both USE_OWLEXTLIB and USE_OWLEXTDLL are defined."
#endif

#if defined(BI_COMP_BORLANDC)
extern "C" OWLEXTFUNC(owl::TModule* WINAPI) GetOWLEXTModule();
#else
STDAPI_(owl::TModule*) GetOWLEXTModule();
#endif

// include libraries
#if !defined(_BUILDOWLEXTDLL)
#include <owlext/owlxlink.h>
#endif

#endif  // OWLEXT_DEFS_H


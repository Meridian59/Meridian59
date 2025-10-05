//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// ObjectComponents fundamental definitions
//----------------------------------------------------------------------------

#if !defined(OCF_DEFS_H)
#define OCF_DEFS_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h> // Includes <windows.h>.

//
// We must use all libraries as DLLs if we are using OCF as a DLL
//
// Define _OCFDLL with -WSE (-WE for 32bit) to use BIDS & RTL in dlls.
//
// Be sure _BUILDOCFDLL is defined when building an actual OCF dll
//
#if defined(_OCFDLL) || defined(_BUILDOCFDLL)
# if !defined(_OCFDLL)
#   define _OCFDLL
# endif
#endif

//----------------------------------------------------------------------------
// Setup class, function and data modifier macros for OCF
//
#if defined(_BUILDOCFDLL)
# define _OCFCLASS  _OWLEXPORTCLASS
# define _OCFDATA   _OWLEXPORTDATA
# define _OCFFUNC   _OWLEXPORTFUNC
# define _OCFCFUNC  _OWLEXPORTCFUNC
#elif defined(_OCFDLL)
# define _OCFCLASS   _OWLIMPORTCLASS
# define _OCFDATA   _OWLIMPORTDATA
# define _OCFFUNC   _OWLIMPORTFUNC
#  define _OCFCFUNC  _OWLIMPORTCFUNC
#else
# define _OCFCLASS
# define _OCFDATA(p) p
# define _OCFFUNC(p) p
#  define _OCFCFUNC(p) p _CCONV
#endif

// only for OWL
#if defined(_BUILDOCFDLL) || defined(_OCFDLL)
#  define DECLARE_STREAMABLE_OCF( cls, ver )                     \
    DECLARE_STREAMABLE( _OCFCLASS, cls, ver )
# define DECLARE_STREAMABLE_FROM_BASE_OCF( cls, base )          \
    DECLARE_STREAMABLE_FROM_BASE( _OCFCLASS, cls, base )
#define DECLARE_ABSTRACT_STREAMABLE_OCF(cls, ver )              \
    DECLARE_ABSTRACT_STREAMABLE( _OCFCLASS, cls, ver )

#else
#  define DECLARE_STREAMABLE_OCF(cls, ver )                      \
    DECLARE_STREAMABLE( _OCFCLASS, cls, ver )
# define DECLARE_STREAMABLE_FROM_BASE_OCF( cls, base )          \
    DECLARE_STREAMABLE_FROM_BASE( _OCFCLASS, cls, base )
#define DECLARE_ABSTRACT_STREAMABLE_OCF(cls, ver )              \
    DECLARE_ABSTRACT_STREAMABLE( _OCFCLASS, cls, ver )

#endif

//
// Useful HRESULT constants and test functions
//
#define HR_NOERROR             NOERROR
#define HR_OK                  ResultFromScode(S_OK)  // same as NOERROR
#define HR_FALSE               ResultFromScode(S_FALSE)
#define HR_NOTIMPL             ResultFromScode(E_NOTIMPL)
#define HR_OUTOFMEMORY         ResultFromScode(E_OUTOFMEMORY)
#define HR_INVALIDARG          ResultFromScode(E_INVALIDARG)
#define HR_NOINTERFACE         ResultFromScode(E_NOINTERFACE)
#define HR_POINTER             ResultFromScode(E_POINTER)
#define HR_HANDLE              ResultFromScode(E_HANDLE)
#define HR_ABORT               ResultFromScode(E_ABORT)
#define HR_FAIL                ResultFromScode(E_FAIL)

#define HR_CLASS_NOAGGREGATION ResultFromScode(CLASS_E_NOAGGREGATION)
#define HR_CLASS_CLASSNOTAVAILABLE ResultFromScode(CLASS_E_CLASSNOTAVAILABLE)

#define HR_DISP_MEMBERNOTFOUND ResultFromScode(DISP_E_MEMBERNOTFOUND)
#define HR_DISP_BADPARAMCOUNT  ResultFromScode(DISP_E_BADPARAMCOUNT)
#define HR_DISP_BADVARTYPE     ResultFromScode(DISP_E_BADVARTYPE)
#define HR_DISP_OVERFLOW       ResultFromScode(DISP_E_OVERFLOW)
#define HR_DISP_TYPEMISMATCH   ResultFromScode(DISP_E_TYPEMISMATCH)
#define HR_DISP_PARAMNOTFOUND  ResultFromScode(DISP_E_PARAMNOTFOUND)
#define HR_DISP_PARAMNOTOPTIONAL ResultFromScode(DISP_E_PARAMNOTOPTIONAL)
#define HR_DISP_EXCEPTION      ResultFromScode(DISP_E_EXCEPTION)
#define HR_DISP_OVERFLOW       ResultFromScode(DISP_E_OVERFLOW)
#define HR_DISP_UNKNOWNINTERFACE ResultFromScode(DISP_E_UNKNOWNINTERFACE)
#define HR_DISP_UNKNOWNNAME    ResultFromScode(DISP_E_UNKNOWNNAME)

#define HR_TYPE_INVALIDSTATE   ResultFromScode(TYPE_E_INVALIDSTATE)
#define HR_TYPE_WRONGTYPEKIND  ResultFromScode(TYPE_E_WRONGTYPEKIND)
#define HR_TYPE_LIBNOTREGISTERED ResultFromScode(TYPE_E_LIBNOTREGISTERED)
#define HR_TYPE_ELEMENTNOTFOUND ResultFromScode(TYPE_E_ELEMENTNOTFOUND)
#define HR_TYPE_UNSUPFORMAT    ResultFromScode(TYPE_E_UNSUPFORMAT)

#define HR_DATA_FORMATETC      ResultFromScode(DATA_E_FORMATETC)
#define HR_DV_FORMATETC        ResultFromScode(DV_E_FORMATETC)
#define HR_DATA_SAMEFORMATETC  ResultFromScode(DATA_S_SAMEFORMATETC)

#define HR_MK_NOOBJECT         ResultFromScode(MK_E_NOOBJECT)
#define HR_MK_NOSTORAGE        ResultFromScode(MK_E_NOSTORAGE)
#define HR_MK_UNAVAILABLE      ResultFromScode(MK_E_UNAVAILABLE)

#define HR_OLE_STATIC          ResultFromScode(OLE_E_STATIC)
#define HR_OLE_NOT_INPLACEACTIVE ResultFromScode(OLE_E_NOT_INPLACEACTIVE)
#define HR_OLEOBJ_INVALIDVERB  ResultFromScode(OLEOBJ_S_INVALIDVERB)
#define HR_OLE_USEREG          ResultFromScode(OLE_S_USEREG)

#define HR_RPC_CALL_REJECTED   ResultFromScode(RPC_E_CALL_REJECTED)
#define HR_STG_CANTSAVE        ResultFromScode(STG_E_CANTSAVE)

namespace ocf {

// Return result codes based on bool expression
//
inline HRESULT HRFailIfZero(int bexpr) {return bexpr ? HR_NOERROR : HR_FAIL;}
inline HRESULT HRFalseIfZero(int bexpr) {return bexpr ? HR_OK : HR_FALSE;}

// Return bool based on hresult test
//
inline bool HRSucceeded(HRESULT hr) {return SUCCEEDED(hr);}
inline bool HRIsOK(HRESULT hr) {return static_cast<bool>(hr == HR_OK);}

} // OCF namespace

#if !defined(_BUILDOCFDLL)
#include <ocf/ocflink.h>
#endif

//
// Provide alias for old all-uppercase namespace name.
//
#if defined(OWL5_COMPAT)
#define OCF ocf
#endif

#endif  // OCF_DEFS_H

//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// Compiler-specific defines for Microsoft Visual C/C++. Included only by
// defs.h
//
// _MSC_VER versions:
//
//   1100 - Microsoft Visual C++ 5.0 - not supported
//   1200 - Microsoft Visual C++ 6.0 - no longer supported (since OWLNext 6.31)
//   1300 - Microsoft Visual C++ 2002 (7.0) - no longer supported (since OWLNext 6.33)
//   1310 - Microsoft Visual C++ 2003 (7.1) - no longer supported (since OWLNext 6.33)
//   1400 - Microsoft Visual C++ 2005 (8.0) - no longer supported (since OWLNext 6.33)
//   1500 - Microsoft Visual C++ 2008 (9.0)
//   1600 - Microsoft Visual C++ 2010 (10.0)
//   1700 - Microsoft Visual C++ 2012 (11.0)
//
//----------------------------------------------------------------------------
#if !defined(OWL_PRIVATE_MSC_H)
#define OWL_PRIVATE_MSC_H

#pragma once

//
// Check compiler support.
//
#if _MSC_VER < 1500
# error OWLNext 6.33 and later require version 9 or later of this compiler.
#endif

//
// Specific name of compiler, BI_COMP_xxxx
//
#define BI_COMP_MSC

#if !defined(__FLAT__)
# define __FLAT__
#endif

//
// Define library-wide function calling conventions. Must rebuild everything
// if changed, and must use appropriate modifier on derived classes.
//

#if !defined(_CCONV)
# define _CCONV __cdecl
#endif

//
// Only enable multi-thread classes when the OS supports them, and/or when the
// runtime library support them
//
#define BI_MULTI_THREAD

// if RTLDLL we must been multitreaded
#if defined(_MT) || defined(BI_APP_DLL)
# define BI_MULTI_THREAD_RTL
#endif

//
// Compiler-specific macros
//

#define _RTLENTRY
#define _USERENTRY
#define _EXPCLASS _CRTIMP
#define _EXPDATA _CRTIMP 
#define _EXPFUNC _CRTIMP 

#ifdef _MBCS
# if !defined(BI_DBCS_SUPPORT)
#   define BI_DBCS_SUPPORT
# endif
#endif //_MBCS

#if defined(BI_DBCS_SUPPORT)
# if !defined(_MBCS)
#   define _MBCS
# endif
#endif

#if defined(OWL5_COMPAT) && !defined(UNICODE) && defined(_M_IX86)
#define OWL_SUPPORT_BWCC
#define OWL_SUPPORT_CTL3D
#endif


//
//  C++ language features (see "defs.h" for preprocessor symbol descriptions)
//
//  Language feature:          Description                  SERVICES header
//  ~~~~~~~~~~~~~~~~           ~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~
#define BI_GLOBALNM_BUG     // Global namespace bug in function member int mfunc(int name = ::someName);
#define BI_NOTEMPLATE_H     // No include <owl/template.h>

#if !defined(_CPPRTTI)
# error You have to enable RTTI support.
#endif
#define _RTTI                 // without __rtti keyword

#if !defined(_CPPUNWIND)
# error You have to enable exception support.
#endif

#define NO_AFTER_RETPARAM     // ??
#define BI_NO_LVAL_CASTS            // Can cast lvalues
#define BI_VIRTUAL_BUG            //problem when derived class add virtual table
#define BI_TYPECASTOPERATOR_BUG   // problem with type cast operator
#define BI_HAS_PRAGMA_ONCE


//
//  C++ library features
//

#define BI_STDIO_CTRFILE

// 
// Misc
//

#define OWL_BREAK {__debugbreak();}
#define BI_OWL_PACKING  8

#if !defined(_BUILDOWLDLL)
# include <owl/private/owllink.h>
#endif

#define OWL_OUTPUT_DEBUG_STRING(lpsz) ::OutputDebugString(lpsz) 

//
// Disable some MSC warnings, & move some to level 4 since they really are OK
// TODO: Should not disable warnings in public headers; ideally rewrite the affected code.
// If that's not feasable, then disable warnings permanently only in source files, and
// use warning(push/pop) in headers to temporarily turn warnings off for the relevant code.
//

#pragma warning(disable:4068)  // unknown pragma, compiler should ignore
#pragma warning(disable:4250)  // inherits <base> via dominance
#pragma warning(disable:4251)  // class 'type' needs to have dll-interface to be used by clients of class 'type2'
#pragma warning(disable:4293)  // disable warning new in default argument will not free memory
#pragma warning(disable:4800)  // disable warning forsing value to bool
//#pragma warning(disable:4003)  // not enough actual parameters for macro
#pragma warning(disable:4275)  // non dll-interface class used as base for dll-interface class
//#pragma warning(disable:4243)  // type cast conversion from '' to '' exists, but is inaccessible
#pragma warning(disable:4355)  // 'this' used in base member initializer list
#pragma warning(disable:4273)  // 'identifier' : inconsistent DLL linkage. dllexport assumed
#pragma warning(disable:4786)  // 'identifier' : identifier was truncated to 'number' characters in the debug information

// SIR June 25th 2007
#if _MSC_VER == 1400
#pragma warning(disable:4996)  // The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name.
#endif



/*
#pragma warning(disable:4514)  // unreferenced inline, compiler internal noise
#pragma warning(disable:4201)  // complains about valid anonymouse unions!
#pragma warning(disable:4238)  // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4290)  // no warning about ignoring throw specification
#pragma warning(disable:4511)  // 'class' : copy constructor could not be generated
#pragma warning(disable:4512)  // 'class' : assignment operator could not be generated
#pragma warning(disable:4516)  // 'class::symbol' : access-declarations are deprecated; member using-declarations provide a better alternative
*/


//
// Turn off reference tracking for certain often used symbols
//

#pragma component(browser, off, references, "CHECK")
#pragma component(browser, off, references, "CHECKX")
#pragma component(browser, off, references, "TRACE")
#pragma component(browser, off, references, "TRACEX")
#pragma component(browser, off, references, "PRECONDITION")
#pragma component(browser, off, references, "PRECONDITIONX")
#pragma component(browser, off, references, "uint8")
#pragma component(browser, off, references, "int8")
#pragma component(browser, off, references, "uint16")
#pragma component(browser, off, references, "int16")
#pragma component(browser, off, references, "unit32")
#pragma component(browser, off, references, "int32")
#pragma component(browser, off, references, "uint")
#pragma component(browser, off, references, "LPSTR")
#pragma component(browser, off, references, "LPTSTR")
#pragma component(browser, off, references, "LPCSTR")
#pragma component(browser, off, references, "LPCTSTR")
#pragma component(browser, off, references, "NULL")
#pragma component(browser, off, references, "WINAPI")
#pragma component(browser, off, references, "_OWLCLASS") 
#pragma component(browser, off, references, "_OWLFUNC")
#pragma component(browser, off, references, "_OWLDATA")


#endif  // OWL_PRIVATE_MSC_H

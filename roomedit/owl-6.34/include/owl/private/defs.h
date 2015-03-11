//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_DEFS_H)
#define OWL_PRIVATE_DEFS_H
#define OSL_DEFS_H
#define SERVICES_DEFS_H
#define WINSYS_DEFS_H

#if !defined(__cplusplus)
#error OWLNext requires C++ compilation.
#endif

#if !defined(_WIN32)
# error OWLNext requires _WIN32 (_WIN16 is no longer supported).
#endif

//
//  Define the type of application being built. Must be one of:
//
//      BI_APP_EXECUTABLE
//      BI_APP_DLL
//
#if defined(__DLL__) || defined(_DLL) || defined(_WINDLL)
# define BI_APP_DLL
#else
# define BI_APP_EXECUTABLE
#endif

//
//  Only enable multi-thread classes when the OS supports them, and/or when the
//  runtime library support them
//
// BI_MULTI_THREAD
// BI_MULTI_THREAD_RTL
//
// Specific name of compiler, BI_COMP_xxxx
//
// BI_COMP_BORLANDC  or BI_COMP_MSC or BI_COMP_GNU
//
// BI_NO_LVAL_CASTS    // Can cast lvalues
//
// _WIN32
//
// OWL_BREAK                   -> debug trap instruction 
//
//
//  BI_NOTHUNK                  -> don't use thunks (WARNING! Not safe! Deprecated.)
//
// Include the appropriate compiler-specific header based on known compiler
// defined preprocessor constants
//
// NO_AFTER_RETPARAM            -> if defined _export/_import keyword placed
//                                 before return parameter :
//                                 for Borland:    void __declspec(dllexport) MFunc()
//                                 for Microsoft:  __declspec(dllexport) void MFunc()
// BI_VIRTUAL_BUG               -> virtual bug  with micro$oft, no bug but different 
//                                 handling of C++ ???
// BI_OWL_PACKING               -> aligment if not defined default 1
// BI_TYPECASTOPERATOR_BUG      -> if problem with type cast operator like: operator HWND()
// BI_GLOBALNM_BUG              -> Global namespace bug in MSVC like mfunc(int name = ::someName);
// BI_NOTEMPLATE_H              -> No include <owl/template.h> -> not defined
// BI_HAS_PRAGMA_ONCE            -> support for pragma once
//----------------------------------------------------------------------------
// We must use all libraries in DLLs if we are using Owl in a DLL
//
// Define _OWLDLL with -WSE (-WE for 32bit) to use Owl, bids & RTL in dlls.
//
// Be sure _BUILDOWLDLL is defined when building an actual owl dll
//

//JJH use only if not using msvcrt's stdlib.h
#if defined(BI_NEED_PRIVATE_OWLPORT) //for Syncad boys, probably should be obsolote right now.
#include <port/owl-port.h>
#endif

#if defined(_OWLDLL) || defined(_BUILDOWLDLL)
# if !defined(_OWLDLL)
#   define _OWLDLL
# endif

# if !defined(_RTLDLL)
#   define _RTLDLL
# endif
#endif

#if defined(_DEBUG)
#if !defined(__DEBUG)
# define __DEBUG 1
#endif
#endif

#if defined(__DEBUG) && __DEBUG > 0
# if !defined(_DEBUG)
#   define _DEBUG
# endif
#endif

#if defined(UNICODE)
# if !defined(_UNICODE)
#   define _UNICODE
# endif
#endif

#if defined(_UNICODE)
# if !defined(UNICODE)
#   define UNICODE
# endif
#endif

#if !defined(__FORTE__)
#define __endexcept
#endif

#if !defined(OWL_STACKWALKER)
# define OWL_STACKWALKER 0
#else
# if OWL_STACKWALKER && !(defined(_MSC_VER) && __DEBUG)
#   error OWL_STACKWALKER is only supported in debug mode for the Microsoft compiler.
# endif
#endif

#if defined(__BORLANDC__)
# include <owl/private/borlandc.h>
#elif defined(_MSC_VER)
# include <owl/private/msc.h>
#elif defined(__GNUC__)
# include <owl/private/gnuc.h>
#elif defined(__WATCOMC__)
# error Watcom currently not supported
#elif defined(__FORTE__)
# error Forte currently not supported
#elif defined(HPCOMPNOW)
# include <owl/private/acchpux.h>
#else
#error Unknown compiler
#endif

#if defined(BI_COMP_WATCOM)
#pragma read_only_file
#endif

//----------------------------------------------------------------------------
//
// Defines macro to force a debug trap.
//
#if !defined(OWL_BREAK)
# include <assert.h>
# define OWL_BREAK assert("OWLNext Debug Break" && false);
#endif

//------------------------------------------------------------------------------
// default BI_OWL_PACKING = 1
#if !defined(BI_OWL_PACKING)
#define BI_OWL_PACKING 1
#endif

//----------------------------------------------------------------------------
#if defined(NO_OWL_NATIVECTRL_ALWAYS)
#error OWLNext: NO_OWL_NATIVECTRL_ALWAYS is obsolete build options.
#endif

#if defined(OWL_NATIVECTRL_NEVER)
#error OWLNext: OWL_NATIVECTRL_NEVER is obsolete build options.
#endif

//
// Compatibility flags
// Setting a lower compatility flag implies all the higher ones, so ensure they are set.
//
#if defined(OWL1_COMPAT) || defined(OWL2_COMPAT) || defined(NO_OWL1_COMPAT) || defined(NO_OWL2_COMPAT) || defined(NO_OWL5_COMPAT)
#error OWLNext: OWL1_COMPAT and OWL2_COMPAT (as well as NO_OWL?_COMPAT variants) are obsolete build options.
#endif

#if defined(NO_OWL5_COMPAT)
#pragma message("OWLNext: The NO_OWL5_COMPAT build option is obsolete.")
#endif

//----------------------------------------------------------------------------
// _RTTI provides a convenient macro for switching on Borland's __rtti
// keyword for finer grained control over generation of runtime type
// information.
//

#if !defined(_RTTI)
#  define _RTTI __rtti
#endif

//----------------------------------------------------------------------------
//  These CAST macros encapsulate the new cast syntax in the ANSI/ISO
//  working paper. Note that TYPESAFE_DOWNCAST isn't as general as
//  dynamic_cast -- it only works on pointers.
//
//  Usage:
//
//  TYPESAFE_DOWNCAST(object,toClass)
//      Converts the pointer referred to by 'object' into a pointer to
//      an object of type 'toClass'. Note that the macro parameters to
//      TYPESAFE_DOWNCAST are in the opposite order from the rest of
//      the macros here. When using a compiler that supports new style
//      casts and runtime type information this is done with
//      dynamic_cast<> and will return 0 if the cast cannot be done.
//      When using a compiler that does not support new style casts and
//      runtime type information this is done with fake runtime type
//      information generated by the IMPLEMENT_CASTABLE macro.
//
//  STATIC_CAST(targetType,object)
//      Converts the data object referred to by 'object' into the type
//      referred to by 'targetType'. When using a compiler that supports
//      new style casts, this is done with static_cast<> and will fail
//      if the cast cannot be done without runtime type information.
//      When using a compiler that does not support new style casts, this
//      is done with an old style dangerous cast.
//
//  CONST_CAST(targetType,object)
//      Converts the data object referred to by 'object' into the type
//      referred to by 'targetType'. When using a compiler that supports
//      new style casts, this is done with const_cast<> and will fail
//      if the cast changes the type of the object in any way other than
//      adding or removing const and volatile qualifiers.
//      When using a compiler that does not support new style casts, this
//      is done with an old style dangerous cast.
//
//  REINTERPRET_CAST(targetType,object)
//      Converts the data object referred to by 'object' into the type
//      referred to by 'targetType'. When using a compiler that supports
//      new style casts, this is done with reinterpret_cast<>.
//      When using a compiler that does not support new style casts, this
//      is done with an old style dangerous cast.
//

# define TYPESAFE_DOWNCAST(object,toClass)\
        dynamic_cast<toClass * >(object)
# define STATIC_CAST(targetType,object)   \
        static_cast<targetType >(object)
# define CONST_CAST(targetType,object)    \
        const_cast<targetType >(object)
# define REINTERPRET_CAST(targetType,object) \
        reinterpret_cast<targetType >(object)

#  define GET_CLASS_CASTABLE_ID(classtype) typeid( classtype ).name()
#  define GET_OBJECT_CASTABLE_ID(object)   typeid( object ).name()

//----------------------------------------------------------------------------
// Provide expansions for mutable and bool when appropriate.
//


namespace owl {

  typedef bool TBool;
# define BI_UNIQUE_BOOL

  template<class T> inline bool ToBool(const T& t) {
    return STATIC_CAST(bool,t);
  }

} // OWL namespace


//----------------------------------------------------------------------------
// Setup class, function and data modifier macros for OWL.
//
#if defined(NO_AFTER_RETPARAM)
# define _OWLEXPORTDATA(p)   __declspec(dllexport) p
# define _OWLEXPORTFUNC(p)   __declspec(dllexport) p 
# define _OWLEXPORTCFUNC(p)  __declspec(dllexport) p _CCONV    // owl _cdecl
# define _OWLIMPORTDATA(p)   __declspec(dllimport) p
# define _OWLIMPORTFUNC(p)   __declspec(dllimport) p 
# define _OWLIMPORTCFUNC(p)  __declspec(dllimport) p _CCONV
# define _OWLSTATICDATA(p)   p
# define _OWLSTATICFUNC(p)   p 
# define _OWLSTATICCFUNC(p)  p _CCONV
#else
# define _OWLEXPORTDATA(p)   p __declspec(dllexport)
# define _OWLEXPORTFUNC(p)   p __declspec(dllexport) 
# define _OWLEXPORTCFUNC(p)  p __declspec(dllexport) _CCONV
# define _OWLIMPORTDATA(p)   p __declspec(dllimport)
# define _OWLIMPORTFUNC(p)   p __declspec(dllimport) 
# define _OWLIMPORTCFUNC(p)  p __declspec(dllimport) _CCONV
# define _OWLSTATICDATA(p)   p
# define _OWLSTATICFUNC(p)   p _CCONV
# define _OWLSTATICCFUNC(p)  p _CCONV
#endif

#define _OWLEXPORTCLASS       __declspec(dllexport) 
#define _OWLEXPORTCLASS_RTL   __declspec(dllexport)
#define _OWLIMPORTCLASS       __declspec(dllimport) 
#define _OWLIMPORTCLASS_RTL   __declspec(dllimport)
#define _OWLSTATICCLASS       
#define _OWLSTATICCLASS_RTL

#if defined(_BUILDOWLDLL)
//JJH following guard for _OWLCLASS is here for the sake of 
//owl/include/private/winelib.h:19
# if !defined(_OWLCLASS)
# define _OWLCLASS      _OWLEXPORTCLASS
# endif
//JJH the same as for _OWLCLASS
# if !defined(_OWLCLASS_RTL)
# define _OWLCLASS_RTL  _OWLEXPORTCLASS_RTL
# endif
# define _OWLDATA       _OWLEXPORTDATA
# define _OWLFUNC       _OWLEXPORTFUNC
# define _OWLCFUNC      _OWLEXPORTCFUNC
#elif defined(_OWLDLL)
# define _OWLCLASS     _OWLIMPORTCLASS
# define _OWLCLASS_RTL _OWLIMPORTCLASS_RTL
# define _OWLDATA     _OWLIMPORTDATA
# define _OWLFUNC     _OWLIMPORTFUNC
# define _OWLCFUNC    _OWLIMPORTCFUNC
#else
# define _OWLCLASS
# define _OWLCLASS_RTL
# define _OWLDATA(p)   p
# define _OWLFUNC(p)   p 
# define _OWLCFUNC(p)  p _CCONV
#endif

#if defined(NO_AFTER_RETPARAM)
#define DECLARE_FUNC(rparm,ctype,fname)  rparm (ctype fname)
#else
#define DECLARE_FUNC(rparm,ctype,fname)  rparm ctype (fname)
#endif


//----------------------------------------------------------------------------
// Setup class modifier for user classes derived from Owl classes.
//  Force data segment load on method entry in case called back from Owl for
//  16bit windows DLLs using Owl in a DLL
//
# define _USERCLASS

//----------------------------------------------------------------------------

//
// Define library-wide function calling conventions. Must rebuild everything
// if changed, and must use appropriate modifier on derived classes.
//
#if !defined(_CCONV)
#define _CCONV __cdecl
#endif

#if !defined(BI_NO_SWAP_TMPL)
template <class T> inline void Swap( T &a, T &b ) {
  a ^= b;  b ^= a;  a ^= b;
}
#endif

//
/// Array element count
/// Important: Only use this with an argument of array type.
/// Passing a pointer to the array/first element will give the wrong results.
/// Consider using boost::size or an equivalent function instead for safety.
//
#define COUNTOF(s)   (sizeof(s) / sizeof((s)[0]))

#if defined(BI_NOTHUNK)
#pragma message("OWLNext: BI_NOTHUNK is unsafe and deprecated.")
#endif

#include <limits.h>

#endif  // OWL_PRIVATE_DEFS_H

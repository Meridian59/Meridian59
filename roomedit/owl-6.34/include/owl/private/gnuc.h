//----------------------------------------------------------------------------
// ObjectWindows, OWL NExt
// Copyright (c) 1998-1999 by Yura Bidus, All Rights Reserved
//
// Compiler-specific defines for GNU C/C++. Included only by
// defs.h
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_GNUC_H)
#define OWL_PRIVATE_GNUC_H

#ifdef __CYGWIN__
#error only mingw supported now
#endif

//////////////////////////
// let STL do some work
#undef min
#undef max
#include <algorithm>
using std::min;
using std::max;
#define __MINMAX_DEFINED   //avoid use definition of min max from Borland/others headers

#include <stddef.h>
#include <exception>

#if !defined(__FLAT__)
#  define __FLAT__
#endif

// Define library-wide function calling conventions. Must rebuild everything
// if changed, and must use appropriate modifier on derived classes.
//

#if !defined(_CCONV)
#  define _CCONV __cdecl
#endif


//
//  Only enable multi-thread classes when the OS supports them, and/or when the
//  runtime library support them
//
#define BI_MULTI_THREAD

// GNU suport multithreading ???
#if defined(__STL_WIN32THREADS)
#define BI_MULTI_THREAD_RTL
#endif

//----------------------------------------------------------------------------
//  Compiler-specific flags.
//----------------------------------------------------------------------------
// MinGW uses __declspecs, so just turn import/export into those
// MSVC uses __declspecs, so just turn import/export into those


//
// Specific name of compiler, BI_COMP_xxxx
//
#define BI_COMP_GNUC
#define _RTLENTRY
#define _USERENTRY
// uses Microsoft RTL under Windows 
#define _EXPCLASS __declspec(dllimport)
#define _EXPDATA(type)  type __declspec(dllimport)
#define _EXPFUNC(type)  type __declspec(dllimport)

// Debug trap macro
#define OWL_BREAK __asm__ ("int3");


//
//  C++ language features
//
//  When compiling with a compiler that doesn't support these
//  features directly, remove the // from the appropriate
//  #define below or define the macro in the build environment.
//

//
//  Language feature:          Description                  SERVICES header
//  ~~~~~~~~~~~~~~~~           ~~~~~~~~~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~
#if __GNUC__ <4 
# error OWLNext 6.33 and later require version 4 or later of this compiler.
#endif

//#define BI_NO_NEW_CASTS     // New style casts              CASTABLE.H

#if !defined(__STL_USE_EXCEPTIONS)
# define __STL_USE_EXCEPTIONS
#endif

// change __stl_assert -> OWL alwas GUI application
#ifdef __STL_ASSERTIONS
#undef __stl_assert
# define __stl_assert(expr) CHECK(expr)
#endif

#define BI_NO_OBJ_STREAMING    // currently

#define _RTTI                 // without __rtti keyword  //???????????????
//#define BI_NOTHUNK            // not used chunk

#define NO_AFTER_RETPARAM        // __import int FuncName(...)
//#define BI_NO_LVAL_CASTS      // Can cast lvalues
//#endif

//#define BI_VIRTUAL_BUG            //??????????????????????????

//JJH just to get rid of warning in wine's basetsd.h header file
//#if defined(WINELIB)
//typedef long long __int64;
//#else
#if !defined(__int64)
#define __int64 long long 
#endif
//#endif

//#define BI_TYPECASTOPERATOR_BUG   // problem with type cast operator
//
// Aligning for OWL
//
#if !defined BI_OWL_PACKING
#define BI_OWL_PACKING 4 // for ??? default packing 
#endif


//#define seek_dir  seekdir   //conflicts with ios_base.h

#if defined(__MINGW32__)
#  include <owl/private/mingw32.h>
#elif defined(__CYGWIN__)
#  include <owl/private/cygwin.h>
#error structured exception handling not yet supported
#elif defined(WINELIB) //JJH
#  include <owl/private/winelib.h>
#else
#error Unknown GNU Compiler
#endif

#define OWL_OUTPUT_DEBUG_STRING(lpsz) ::OutputDebugString(lpsz) 

//need again to undef min/max (check later private/winelib.h->winsock2.h use)
#undef min
#undef max


#if !defined(_MAX_PATH)
#define _MAX_PATH MAX_PATH
#endif

//C runtime equivalents
#define _stricmp strcasecmp 
#define _strnicmp strncasecmp

//both _fullpath and _wfullpath are in the msvcrt implementation of Winelib (stdlib.h), 
//but we prefer a to use only functions of gnu lib
#define _fullpath myfullpath  //used in file.cpp, implemented in unix.cpp

char *myfullpath(char *,const char *,int );


#endif  // OWL_PRIVATE_GNUC_H

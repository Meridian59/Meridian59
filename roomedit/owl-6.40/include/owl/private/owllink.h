//----------------------------------------------------------------------------
// Object Windows Library
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
// Copyright (c) 2008-2013 Vidar Hasfjord
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_OWLLINK_H)
#define OWL_PRIVATE_OWLLINK_H

#include <owl/version.h>

#define OWL_LIB_NAME "owl"

#define OWL_LIB_DELIMITER "-"

#define OWL_LIB_VERSION OWL_PRODUCTVERSION_STRING

#if defined(__BORLANDC__)
#define OWL_LIB_COMPILER "b"

#if (__BORLANDC__ >= 0x690) // XE7
#define OWL_LIB_COMPILER_VERSION "18"
#elif (__BORLANDC__ >= 0x680) // XE6
#define OWL_LIB_COMPILER_VERSION "17"
#elif (__BORLANDC__ >= 0x670) // XE5
#define OWL_LIB_COMPILER_VERSION "16"
#elif (__BORLANDC__ >= 0x660) // XE4
#define OWL_LIB_COMPILER_VERSION "15"
#elif (__BORLANDC__ >= 0x650) // XE3
#define OWL_LIB_COMPILER_VERSION "14"
#elif (__BORLANDC__ >= 0x640) // XE2
#define OWL_LIB_COMPILER_VERSION "13"
#elif (__BORLANDC__ >= 0x630) // XE
#define OWL_LIB_COMPILER_VERSION "12"
#else
#error OWLNext: Unsupported compiler.
#endif

#elif defined(_MSC_VER)	//#if defined(__BORLANDC__)

#define OWL_LIB_COMPILER "v"

#if _MSC_VER >= 1800 // VS 2013
#define OWL_LIB_COMPILER_VERSION "12" 
#elif _MSC_VER >= 1700 // VS 2012
#define OWL_LIB_COMPILER_VERSION "11"
#elif _MSC_VER >= 1600 // VS 2010
#define OWL_LIB_COMPILER_VERSION "10"
#else
#error OWLNext: Unsupported compiler.
#endif

#endif	//#if defined(__BORLANDC__)

// Platform processor architecture (ISA)

#if defined(_M_AMD64)
#define OWL_LIB_PLATFORM "x64"
#elif defined(_M_IX86)
#define OWL_LIB_PLATFORM "x86"
#else
#error OWLNext: Unsupported platform.
#endif

// Compatibility modes

#if defined(OWL5_COMPAT)
#define OWL_LIB_COMPAT "5"
#else
#define OWL_LIB_COMPAT ""
#endif

// Debugging modes

#if defined(__CODEGUARD__)
#define OWL_LIB_DEBUG "g"
#elif (__DEBUG > 0)
#define OWL_LIB_DEBUG "d"
#else
#define OWL_LIB_DEBUG ""
#endif

// Threading (concurrency) modes

#if defined(INC_VCL)
#define OWL_LIB_THREADING "v"
#elif defined(__MT__) || defined(_MT)
#define OWL_LIB_THREADING "t"
#else
#define OWL_LIB_THREADING "s"
#endif

// Character type modes

#if defined(UNICODE)
#define OWL_LIB_CHARACTERTYPE "u"
#else
#define OWL_LIB_CHARACTERTYPE ""
#endif

// Link (import) modes

#if defined(_OWLDLL)
#define OWL_LIB_IMPORT "i"
#else
#define OWL_LIB_IMPORT ""
#endif

// Compose the appropriate pragma link directive.

/*#pragma comment(lib, \
	  OWL_LIB_NAME \
	  OWL_LIB_DELIMITER \
	  OWL_LIB_VERSION \
	  OWL_LIB_DELIMITER \
	  OWL_LIB_COMPILER \
	  OWL_LIB_COMPILER_VERSION \
	  OWL_LIB_DELIMITER \
	  OWL_LIB_PLATFORM \
	  OWL_LIB_DELIMITER \
	  OWL_LIB_COMPAT \
	  OWL_LIB_DEBUG \
	  OWL_LIB_THREADING \
	  OWL_LIB_CHARACTERTYPE \
	  OWL_LIB_IMPORT \
    ".lib")*/

#endif // OWL_PRIVATE_OWLLINK_H

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

#if (__BORLANDC__ >= 0x690)
#define OWL_LIB_COMPILER_VERSION "18"
#elif (__BORLANDC__ >= 0x680)
#define OWL_LIB_COMPILER_VERSION "17"
#elif (__BORLANDC__ >= 0x670)
#define OWL_LIB_COMPILER_VERSION "16"
#elif (__BORLANDC__ >= 0x660)
#define OWL_LIB_COMPILER_VERSION "15"
#elif (__BORLANDC__ >= 0x650)
#define OWL_LIB_COMPILER_VERSION "14"
#elif (__BORLANDC__ >= 0x640)
#define OWL_LIB_COMPILER_VERSION "13"
#elif (__BORLANDC__ >= 0x630)
#define OWL_LIB_COMPILER_VERSION "12"
#elif (__BORLANDC__ >= 0x620)
#define OWL_LIB_COMPILER_VERSION "11"
#elif (__BORLANDC__ >= 0x610)
#define OWL_LIB_COMPILER_VERSION "10"
#elif (__BORLANDC__ >= 0x590)
#define OWL_LIB_COMPILER_VERSION "9"
#elif (__BORLANDC__ >= 0x580)
#define OWL_LIB_COMPILER_VERSION "8"
#elif (__BORLANDC__ >= 0x570)
#define OWL_LIB_COMPILER_VERSION "7"
#elif (__BORLANDC__ >= 0x560)
#define OWL_LIB_COMPILER_VERSION "6"
#elif (__BORLANDC__ >= 0x550)
#define OWL_LIB_COMPILER_VERSION "5"
#endif

#elif defined(_MSC_VER)	//#if defined(__BORLANDC__)

#define OWL_LIB_COMPILER "v"

#if _MSC_VER >= 1800
#define OWL_LIB_COMPILER_VERSION "12"
#elif _MSC_VER >= 1700
#define OWL_LIB_COMPILER_VERSION "11"
#elif _MSC_VER >= 1600
#define OWL_LIB_COMPILER_VERSION "10"
#elif _MSC_VER >= 1500
#define OWL_LIB_COMPILER_VERSION "9"
#elif _MSC_VER >= 1400
#define OWL_LIB_COMPILER_VERSION "8"
#elif _MSC_VER >= 1300
#define OWL_LIB_COMPILER_VERSION "7"
#elif _MSC_VER >= 1200
#define OWL_LIB_COMPILER_VERSION "6"
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
	
#endif // OWL_PRIVATE_OWLLINK_H

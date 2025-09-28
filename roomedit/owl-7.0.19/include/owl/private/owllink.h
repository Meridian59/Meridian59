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

#if defined(__BORLANDC__) && defined(__clang__)

#define OWL_LIB_COMPILER "c"

#if __BORLANDC__ >= 0x780
#error OWLNext: Unsupported compiler version (too new; please add support).
#elif __BORLANDC__ >= 0x770 // RAD Studio/C++Builder 12 Athens (RX/CX)
#define OWL_LIB_COMPILER_VERSION "770"
#elif __BORLANDC__ >= 0x760 // RAD Studio/C++Builder 11 Alexandria (RX/CX)
#define OWL_LIB_COMPILER_VERSION "760"
#elif __BORLANDC__ >= 0x750 // RAD Studio/C++Builder 10.4 Sydney (RX/CX)
#define OWL_LIB_COMPILER_VERSION "750"
#elif __BORLANDC__ >= 0x740 // RAD Studio/C++Builder 10.3 Rio (RX/CX)
#define OWL_LIB_COMPILER_VERSION "740"
#elif (__BORLANDC__ >= 0x730) // RAD Studio/C++Builder 10.2 Tokyo (RX/CX)
#define OWL_LIB_COMPILER_VERSION "730"
#else
#error OWLNext: Unsupported compiler version (too old; update to a newer version).
#endif

#elif defined(_MSC_VER)

#define OWL_LIB_COMPILER "v"

#if _MSC_VER >= 1950
#error OWLNext: Unsupported compiler version (too new; please add support).
#elif _MSC_VER >= 1930 // VS 2022 (_MSC_VER: 1930-1949)
#define OWL_LIB_COMPILER_VERSION "1930" // Platform toolset "v143" (14.30-14.49)
#elif _MSC_VER >= 1920 // VS 2019
#define OWL_LIB_COMPILER_VERSION "1920"
#elif _MSC_VER >= 1910 // VS 2017
#define OWL_LIB_COMPILER_VERSION "1910"
#else
#error OWLNext: Unsupported compiler version (too old; update to a newer version).
#endif

#else

#error OWLNext: Unsupported compiler.

#endif

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
#elif defined(__DEBUG) && (__DEBUG > 0)
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

// Disable auto-linking with a very specific name.
// See https://sourceforge.net/p/owlnext/discussion/97175/thread/2ebbc20f4e/
//#pragma comment(lib,                          \
//  OWL_LIB_NAME \
//  OWL_LIB_DELIMITER \
//  OWL_LIB_VERSION \
//  OWL_LIB_DELIMITER \
//  OWL_LIB_COMPILER \
//  OWL_LIB_COMPILER_VERSION \
//  OWL_LIB_DELIMITER \
//  OWL_LIB_PLATFORM \
//  OWL_LIB_DELIMITER \
//  OWL_LIB_COMPAT \
//  OWL_LIB_DEBUG \
//  OWL_LIB_THREADING \
//  OWL_LIB_CHARACTERTYPE \
//  OWL_LIB_IMPORT \
//  ".lib")

#endif // OWL_PRIVATE_OWLLINK_H

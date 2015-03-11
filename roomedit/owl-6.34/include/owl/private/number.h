//----------------------------------------------------------------------------
// Object Windows Library
// OWL NExt
// Copyright (c) 1998 Yura Bidus
//
// Commonly used fixed-size integer types
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_NUMBER_H)
#define OWL_PRIVATE_NUMBER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

namespace owl {

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef __int64 int64;

// Use long long for WINELIB to get rid of warning in "basetsd.h".
//
#if defined(WINELIB)
typedef unsigned long long uint64;
#else
typedef unsigned __int64 uint64;
#endif

} // OWL namespace

#endif // OWL_PRIVATE_NUMBER_H

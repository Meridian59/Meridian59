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

#if !defined( CLASSLIB_DLISTIMP_H )
#define CLASSLIB_DLISTIMP_H

#pragma message ("Included obsolete file: <classlib/dlistimp.h> use STL instead")
#include <owl/template.h>

// include compatibility classes
#include <classlib/compat.h>

#endif  // CLASSLIB_DLISTIMP_H


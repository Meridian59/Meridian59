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

#if !defined(CLASSLIB_STDTEMPL_H)
#define CLASSLIB_STDTEMPL_H

#pragma message ("Included obsolete file <classlib/stdtempl.h> use STL instead")

// include compatibility classes
#include <classlib/compat.h>

#endif  // CLASSLIB_STDTEMPL_H

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

#if !defined( CLASSLIB_VECTIMP_H )
#define CLASSLIB_VECTIMP_H

#pragma message ("included obsolete file <classlib/vectimp.h>  use STL or <owl/template.h>")
#include <owl/template.h>

// include compatibility classes
#include <classlib/compat.h>


#endif  // CLASSLIB_VECTIMP_H

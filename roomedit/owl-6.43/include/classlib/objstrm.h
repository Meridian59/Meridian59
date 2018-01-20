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

#if !defined(CLASSLIB_OBJSTRM_H)
#define CLASSLIB_OBJSTRM_H

#pragma message ("<classlib/objstrm.h> has been replaced by <owl/objstrm.h>")
#include <owl/defs.h>
#include <owl/geometry.h>
#include <owl/objstrm.h>


#endif  // CLASSLIB_OBJSTRM_H

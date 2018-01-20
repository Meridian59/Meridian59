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

#if !defined(CLASSLIB_POINTER_H)
#define CLASSLIB_POINTER_H


#pragma message ("<classlib/pointer.h> has been replaced by <owl/pointer.h>")
#include <owl/defs.h>
#include <owl/pointer.h>


#endif  // CLASSLIB_POINTER_H

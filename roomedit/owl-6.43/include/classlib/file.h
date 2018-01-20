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

#if !defined(CLASSLIB_FILE_H)
#define CLASSLIB_FILE_H

#pragma message ("<classlib/file.h> has been replaced by <owl/file.h>")
#include <owl/file.h>


#endif  // CLASSLIB_FILE_H

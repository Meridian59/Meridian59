//----------------------------------------------------------------------------
// Borland WinSys Library Compatibility header
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined(WINSYS_LCLSTRNG_H)
#define WINSYS_LCLSTRNG_H

#pragma message ("<winsys/lclstrng.h> has been replaced by <owl/lclstrng.h>")
#include <owl/lclstrng.h>


#endif  // WINSYS_LCLSTRNG_H

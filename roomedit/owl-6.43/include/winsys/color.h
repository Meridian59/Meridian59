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

#if !defined(WINSYS_COLOR_H)
#define WINSYS_COLOR_H

#pragma message ("<winsys/color.h> has been replaced by <owl/color.h>")
#include <owl/color.h>
#endif  // WINSYS_COLOR_H

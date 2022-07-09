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

#if !defined(WINSYS_SYSTEM_H)
#define WINSYS_SYSTEM_H

#pragma message ("<winsys/system.h> has been replaced by <owl/system.h>")
#include <owl/system.h>


#endif  // WINSYS_SYSTEM_H

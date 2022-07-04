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

#if !defined(WINSYS_PROFILE_H)
#define WINSYS_PROFILE_H

#pragma message ("<winsys/profile.h> has been replaced by <owl/profile.h>")
#include <owl/profile.h>


#endif  // WINSYS_PROFILE_H

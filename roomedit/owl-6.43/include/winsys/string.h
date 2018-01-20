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

#if !defined(WINSYS_STRING_H)
#define WINSYS_STRING_H

#pragma message ("<winsys/string.h> has been replaced by <owl/string.h>")
#include <owl/string.h>


#endif  // WINSYS_STRING_H

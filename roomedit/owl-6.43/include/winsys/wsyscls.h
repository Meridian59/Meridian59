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

#if !defined(WINSYS_WINCLASS_H)
#define WINSYS_WINCLASS_H

#pragma message ("<winsys/wsyscls.h> has been replaced by <owl/wsyscls.h>")
#include <owl/defs.h>
#include <owl/wsyscls.h>


#endif  // WINSYS_WINCLASS_H

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

#if !defined(WINSYS_MSGTHRED_H)
#define WINSYS_MSGTHRED_H

#pragma message ("<winsys/msgthred.h> has been replaced by <owl/msgthred.h>")
#include <owl/msgthred.h>


#endif  // WINSYS_MSGTHRED_H

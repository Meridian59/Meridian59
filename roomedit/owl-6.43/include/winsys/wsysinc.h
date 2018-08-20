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

#if !defined(WINSYS_WSYSINC_H)
#define WINSYS_WSYSINC_H

#pragma message ("<winsys/wsysinc.h> has been replaced by <owl/wsysinc.h>")
#include <owl/wsysinc.h>


#endif  // WINSYS_WSYSINC_H

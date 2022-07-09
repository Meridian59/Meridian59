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

#if defined(_WSYSPCH) && !defined(WINSYS_PCH_H)
#define WINSYS_PCH_H

#pragma message ("Warning!!! Included obsolete file: <winsys/pch.h>")

#endif  // WINSYS_PCH_H

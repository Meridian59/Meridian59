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

#if !defined(WINSYS_GEOMETRY_H)
#define WINSYS_GEOMETRY_H

#pragma message ("<winsys/geometry.h> has been replaced by <owl/geometry.h>")
#include <owl/defs.h>
#include <owl/geometry.h>

#endif  // WINSYS_GEOMETRY_H

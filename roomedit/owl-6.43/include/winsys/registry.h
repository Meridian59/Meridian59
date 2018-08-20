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

#if !defined(WINSYS_REGISTRY_H)
#define WINSYS_REGISTRY_H

#pragma message ("<winsys/registry.h> has been replaced by <owl/registry.h>")
#include <owl/registry.h>

#endif  // WINSYS_REGISTRY_H

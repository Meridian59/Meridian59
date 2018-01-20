//----------------------------------------------------------------------------
// Borland Services Library Compatibility header
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined(SERVICES_MEMORY_H)
#define SERVICES_MEMORY_H

#pragma message ("services/memory.h has been replaced by owl/private/memory.h")
#include <owl/private/memory.h>

#endif  // SERVICES_MEM_H

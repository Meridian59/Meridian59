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

#if !defined(SERVICES_DIR_H)
#define SERVICES_DIR_H

#pragma message ("services/dir.h has been replaced by owl/private/dir.h")
#include <owl/private/dir.h>

#endif  //  SERVICES_DIR_H

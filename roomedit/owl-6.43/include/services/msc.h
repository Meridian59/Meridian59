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

#if !defined(SERVICES_MSC_H)
#define SERVICES_MSC_H

#pragma message ("services/msc.h has been replaced by owl/private/msc.h")
#include <owl/private/msc.h>

#endif  // SERVICES_MSC_H

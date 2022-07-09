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

#if !defined(SERVICES_BORLANDC_H)
#define SERVICES_BORLANDC_H

#pragma message ("services/borland.h has been replaced by owl/private/borland.h")
#include <owl/private/borland.h>
#endif  // SERVICES_BORLANDC_H

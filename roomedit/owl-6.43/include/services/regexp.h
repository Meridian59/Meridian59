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

#if !defined(SERVICES_REGEX_H)
#define SERVICES_REGEX_H

#pragma message ("services/regexp.h has been replaced by owl/private/regexp.h")
#include <owl/private/regexp.h>

#endif  // SERVICES_REGEX_H

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

#if !defined(SERVICES_EXCEPT_H)
#define SERVICES_EXCEPT_H


#pragma message ("services/except.h has been replaced by owl/private/except.h")
#include <owl/defs.h>
#include <owl/private/except.h>

#endif  // SERVICES_EXCEPT_H

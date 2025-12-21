//----------------------------------------------------------------------------
// Borland Services Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
// Includes OLE2 headers in the right order with necessary macros defined
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_OLE2INC_H)
#define OWL_PRIVATE_OLE2INC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/private/wsysinc.h>
#include <ole2.h>

#if !defined(__OLE2_H) && !defined(_OLE2_H_) && !defined(_OLE2_H)
#error OWLNext requires OLE2; make sure INC_OLE1 is not defined.
#endif

#endif  // SERVICES_OLE2INC_H

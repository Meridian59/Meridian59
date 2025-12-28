//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
// Copyright (c) 2008-2013 Vidar Hasfjord
//----------------------------------------------------------------------------

#if !defined(OCF_OCFLINK_H)
#define OCF_OCFLINK_H

#include <owl/private/owllink.h>

#define OCF_LIB_NAME "ocf"

// Link (import) modes

#if defined(_OCFDLL)
#define OCF_LIB_IMPORT "i"
#else
#define OCF_LIB_IMPORT ""
#endif

// Compose the appropriate pragma link directive.

#pragma comment(lib, \
    OCF_LIB_NAME \
    OWL_LIB_DELIMITER \
    OWL_LIB_VERSION \
    OWL_LIB_DELIMITER \
    OWL_LIB_COMPILER \
    OWL_LIB_COMPILER_VERSION \
    OWL_LIB_DELIMITER \
    OWL_LIB_PLATFORM \
    OWL_LIB_DELIMITER \
    OWL_LIB_COMPAT \
    OWL_LIB_DEBUG \
    OWL_LIB_THREADING \
    OWL_LIB_CHARACTERTYPE \
    OCF_LIB_IMPORT \
    ".lib")

#endif // OCF_OWLLINK_H



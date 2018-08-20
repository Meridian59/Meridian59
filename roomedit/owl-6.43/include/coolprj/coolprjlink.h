//-------------------------------------------------------------------
// CoolPrj Class Library
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
// Copyright (c) 2008-2013 Vidar Hasfjord
//-------------------------------------------------------------------

#if !defined(COOLPRJ_COOLPRJLINK_H)
#define COOLPRJ_COOLPRJLINK_H

#include <owl/private/owllink.h>

#define COOLPRJ_LIB_NAME "coolprj"

// Link (import) modes

#if defined(_COOLPRJDLL)
#define COOLPRJ_LIB_IMPORT "i"
#else
#define COOLPRJ_LIB_IMPORT ""
#endif

// Compose the appropriate pragma link directive.

#pragma comment(lib, \
    COOLPRJ_LIB_NAME \
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
    COOLPRJ_LIB_IMPORT \
    ".lib")

#endif // COOLPRJ_COOLPRJLINK_H



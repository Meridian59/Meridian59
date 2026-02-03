//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
// Copyright (c) 2008-2013 Vidar Hasfjord
//-------------------------------------------------------------------

#if !defined(OWLEXT_OWLEXTLINK_H)
#define OWLEXT_OWLEXTLINK_H

#include <owl/private/owllink.h>

#define OWLEXT_LIB_NAME "owlext"

// Link (import) modes

#if defined(USE_OWLEXTDLL)
#define OWLEXT_LIB_IMPORT "i"
#else
#define OWLEXT_LIB_IMPORT ""
#endif

// Compose the appropriate pragma link directive.

#pragma comment(lib, \
    OWLEXT_LIB_NAME \
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
    OWLEXT_LIB_IMPORT \
    ".lib")

#endif // OWLEXT_OWLEXTLINK_H



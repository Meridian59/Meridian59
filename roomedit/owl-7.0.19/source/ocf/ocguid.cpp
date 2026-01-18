//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Actual BOcOle GUIDs spit out here
//
//----------------------------------------------------------------------------

// NOTE: This file must be built without precompiled headers, since it sets the 
// flags INITGUID and INIT_BGUID before including headers to force GUID generation.
//
#undef _OWLPCH
#undef _OWLALLPCH
#undef _OCFPCH
#define INITGUID              // Force OLE GUIDs to be defined
#define INIT_BGUID            // Force Bolero GUIDs to be defined

#include <ocf/defs.h>         // Ole basics & GUIDs
#include <ocf/ocbocole.h>    // Bolero GUIDs
#include <olectl.h>         // Ole control GUIDs

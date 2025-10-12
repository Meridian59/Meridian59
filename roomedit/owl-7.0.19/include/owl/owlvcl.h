//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Borland International, All Rights Reserved
//
// Include for ObjectWindows, gets owlcore.h or owlall.h when precompiled
// headers are enabled.
//----------------------------------------------------------------------------

#if !defined(OWL_OWLVCL_H)
#define OWL_OWLVCL_H

#ifndef STRICT
#define STRICT
#endif

// NO_WIN32_LEAN_AND_MEAN is needed so <vcl.h> will include
// all of <windows.h> as OWL expects.
#define NO_WIN32_LEAN_AND_MEAN

// This addresses ambiguity problems in VCL headers if _OWLALLPCH
// is defined, but is something of a sledgehammer.
#if defined(_OWLALLPCH)
# define INC_ALLVCL_HEADERS
#endif

#include <vcl.h>

#endif

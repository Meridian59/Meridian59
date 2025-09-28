//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
// Include for ObjectWindows, gets owlcore.h or owlall.h when precompiled
// headers are enabled.
//----------------------------------------------------------------------------

#if defined(_OWLVCLPCH)
#include <owl/owlvcl.h>
#endif

#if defined(_OWLALLPCH)
#include <owl/owlall.h>
#elif defined(_OWLPCH)
#include <owl/owlcore.h>
#endif

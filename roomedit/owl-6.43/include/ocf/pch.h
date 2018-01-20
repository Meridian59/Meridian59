//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Include for OC, gets common headers when precompiled headers are enabled.
//----------------------------------------------------------------------------

#if !defined(OCF_PCH_H)
#define OCF_PCH_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif


#if defined(_OCFPCH)
# include <owl/pch.h>
# include <ocf/defs.h>
# include <ocf/oleutil.h>
# include <ocf/autodefs.h>
# include <ocf/automacr.h>
# include <ocf/ocobject.h>
# include <ocf/ocstorag.h>
# include <ocf/ocreg.h>
# include <ocf/ocapp.h>
# include <ocf/ocdoc.h>
# include <ocf/ocpart.h>
# include <ocf/ocview.h>
# include <ocf/ocremvie.h>
# include <ocf/oledialg.h>
# include <ocf/except.h>
# include <ocf/oledoc.h>
# include <ocf/olefacto.h>
# include <ocf/oleframe.h>
# include <ocf/olemdifr.h>
# include <ocf/oleview.h>
# include <ocf/olewindo.h>
# include <ocf/stgdoc.h>
#endif

#endif



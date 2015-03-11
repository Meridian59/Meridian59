//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
// Include for ObjectWindows, gets owlcore.h or owlall.h when precompiled
// headers are enabled.
//----------------------------------------------------------------------------
//#ifdef _PCH_H
//#define _PCH_H


#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if defined(_OWLVCLPCH)
# include <owl/owlvcl.h>
#endif

#if defined(_OWLALLPCH)
#  include <owl/owlall.h>
#elif defined(_OWLPCH)
#  include <owl/owlcore.h>
#endif


//#endif // _PCH_H

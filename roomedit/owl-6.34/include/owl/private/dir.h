//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_DIR_H)
#define OWL_PRIVATE_DIR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if defined(BI_COMP_BORLANDC)
# include <dir.h>
#elif defined(BI_COMP_MSC)
# include <direct.h>
#elif defined(BI_COMP_GNUC)
#else
#error Need <dir.h> equivalent header file to be included
#endif

#endif  //  OWL_PRIVATE_DIR_H


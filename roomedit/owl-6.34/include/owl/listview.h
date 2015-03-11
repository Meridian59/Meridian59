//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TListView
//----------------------------------------------------------------------------

#if !defined(OWL_LISTVIEW_H)
#define OWL_LISTVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/listboxview.h>

#pragma message ("included obsolete file: <owl/listview.h> ")
#pragma message ("<owl/listboxview.h> should be used instead")

namespace owl {

// Alias for backwards compatibility
typedef TListBoxView TListView;

} // OWL namespace


#endif  // OWL_LISTVIEW_H

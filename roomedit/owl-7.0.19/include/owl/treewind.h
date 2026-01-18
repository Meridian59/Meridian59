//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Declares aliases TTreeWindow and TTreeItem for backwards compatibility
//----------------------------------------------------------------------------

#if !defined(OWL_TREEWIND_H)
#define OWL_TREEWIND_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/treeviewctrl.h>

#pragma message ("included obsolete file: <owl/treewind.h> ")
#pragma message ("<owl/treeviewctrl.h> should be used instead")


namespace owl {

typedef TTreeViewCtrl TTreeWindow;
typedef TTvItem TTreeItem; 

} // OWL namespace



#endif  // OWL_TREEWIND_H

//
/// \file
/// Definition of TListWindow class
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright © 2011 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#if !defined(OWL_LISTWIND_H)
#define OWL_LISTWIND_H

#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#pragma message ("included obsolete file: <owl/listwind.h> ")
#pragma message ("<owl/listviewctrl.h> should be used instead")

#include <owl/listviewctrl.h>

namespace owl {

typedef TLvFindInfo TLwFindInfo; // Old alternative name - deprecated.

typedef TLvHitTestInfo TLwHitTestInfo; // Old alternative name - deprecated.

typedef TLvItem TListWindItem; // Old alternative name - deprecated.

typedef TLvColumn TListWindColumn; // Old alternative name - deprecated.

typedef TListViewCtrl TListWindow; // Old alternative name - deprecated.

typedef TListViewCtrl::TCompareFunc TLwComparator; // Old alternative name - deprecated.

} // OWL namespace

#endif




//----------------------------------------------------------------------------
//  ObjectWindows
//  OWL NExt
//  Copyright (c) 1999 by Yura Bidus . All Rights Reserved.
//
/// \file
///  Source file for implementation of TTreeViewCtrlView (TTreeViewCtrl).
///  Source file for implementation of TListViewCtrlView (TListViewCtrl).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/commview.h>

namespace owl {

//----------------------------------------------------------------------------
// TTreeViewCtrlView Implementation
//

//
// Build a response table for all messages/commands handled
// by TTreeViewCtrlView derived from TTreeViewCtrl.
//
DEFINE_RESPONSE_TABLE1(TTreeViewCtrlView, TTreeViewCtrl)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;


TTreeViewCtrlView::TTreeViewCtrlView(TDocument& doc, TWindow* parent, int id)
:
  TTreeViewCtrl(parent, id, 0, 0, 0, 0),
  TView(doc)
{
}
#if OWL_PERSISTENT_STREAMS
/*
//
//
void*
TTreeViewCtrlView::Streamer::Read(ipstream& is, uint32 version) const
{
  ReadBaseObject((TTreeViewCtrl*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TWindowView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TTreeViewCtrl*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}
*/
#endif

//----------------------------------------------------------------------------
// TListViewCtrlView Implementation
//

//
// Build a response table for all messages/commands handled
// by TListViewCtrlView derived from TListViewCtrl.
//
DEFINE_RESPONSE_TABLE1(TListViewCtrlView, TListViewCtrl)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;


TListViewCtrlView::TListViewCtrlView(TDocument& doc, TWindow* parent, int id)
:
  TListViewCtrl(parent, id, 0, 0, 0, 0),
  TView(doc)
{
}
#if OWL_PERSISTENT_STREAMS
/*
//
//
void*
TListViewCtrlView::Streamer::Read(ipstream& is, uint32 version) const
{
  ReadBaseObject((TListViewCtrl*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TListViewCtrlView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TListViewCtrl*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}
*/
#endif


} // OWL namespace


//==============================================================================

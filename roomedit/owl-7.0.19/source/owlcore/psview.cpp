//----------------------------------------------------------------------------
//  ObjectWindows
//  OWL NExt
//  Copyright (c) 1999 by Yura Bidus . All Rights Reserved.
//
/// \file
///  Source file for implementation of TPaneSplitterView (TPaneSplitter).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/doctpl.h>
#include <owl/psview.h>


namespace owl {

//{{TPaneSplitterView Implementation}}

//
// Build a response table for all messages/commands handled
// by TPaneSplitterView derived from TPaneSplitter.
//
DEFINE_RESPONSE_TABLE1(TPaneSplitterView, TPaneSplitter)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;


//--------------------------------------------------------
// TPaneSplitterView
//
TPaneSplitterView::TPaneSplitterView(TDocument& doc, TWindow* parent)
:
  TPaneSplitter(parent,_T(""),0,doc.GetTemplate()->GetModule()),
  TView(doc)
{
  // INSERT>> Your constructor code here.
}
//
TPaneSplitterView::~TPaneSplitterView()
{
  // INSERT>> Your destructor code here.
}

} // OWL namespace


//==============================================================================

//----------------------------------------------------------------------------
//  ObjectWindows
//  OWL NExt
//  Copyright © 1999 by Yura Bidus . All Rights Reserved.
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
//

//IMPLEMENT_STREAMABLE2(TPaneSplitterView, TPaneSplitter, TView);
//#if !defined(BI_NO_OBJ_STREAMING)
////
////
//void*
//TPaneSplitterView::Streamer::Read(ipstream& is, uint32 version) const
//{
//  ReadBaseObject((TPaneSplitter*)GetObject(), is);
//  ReadBaseObject((TView*)GetObject(), is);
//  return GetObject();
//}
////
////
////
////
//void
//TPaneSplitterView::Streamer::Write(opstream& os) const
//{
//  WriteBaseObject((TPaneSplitter*)GetObject(), os);
//  WriteBaseObject((TView*)GetObject(), os);
//}
//#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace

  
//==============================================================================

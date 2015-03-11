//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TControl.  This defines the basic behavior of all
/// controls.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/control.h>

namespace owl {

OWL_DIAGINFO;

// from window.cpp
//
TDrawItem* ItemData2DrawItem(ULONG_PTR data);


//
// General Controls diagnostic group
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlControl, 1, 0);


DEFINE_RESPONSE_TABLE1(TControl, TWindow)
  EV_WM_PAINT,
  EV_WM_COMPAREITEM,
  EV_WM_DELETEITEM,
  EV_WM_DRAWITEM,
  EV_WM_MEASUREITEM,
END_RESPONSE_TABLE;


//
/// Invokes TWindow's constructor, passing it parent (parent window), title (caption
/// text), and module. Sets the control attributes using the supplied library ID
/// (Id), position (x, y) relative to the origin of the parent window's client area,
/// width (w), and height (h) parameters. It sets the control style to WS_CHILD |
/// WS_VISIBLE | WS_GROUP | WS_TABSTOP.
//
TControl::TControl(TWindow*        parent,
                   int             id,
                   LPCTSTR         title,
                   int x, int y, int w, int h,
                   TModule*        module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, title, module);

  Attr.Id = id;
  Attr.X = x;
  Attr.Y = y;
  Attr.W = w;
  Attr.H = h;
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP;

  TRACEX(OwlControl, OWL_CDLEVEL, "TControl constructed @" << (void*)this);
}

//
/// String-aware overload
//
TControl::TControl(TWindow* parent,
                   int id,
                   const tstring& title,
                   int x, int y, int w, int h,
                   TModule* module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, title, module);

  Attr.Id = id;
  Attr.X = x;
  Attr.Y = y;
  Attr.W = w;
  Attr.H = h;
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP;

  TRACEX(OwlControl, OWL_CDLEVEL, "TControl constructed @" << (void*)this);
}


//
/// Constructs an object to be associated with an interface control of a TDialog
/// object. Invokes the  TWindow constructor, then enables the data transfer
/// mechanism by calling TWindow::EnableTransfer.
/// The id parameter must correspond to a control interface resource that you
/// define.
//
TControl::TControl(TWindow*   parent,
                   int        resourceId,
                   TModule*   module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, 0, module);

  SetFlag(wfFromResource);
  memset(&Attr, 0x0, sizeof(Attr));
  Attr.Id = resourceId;
  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TControl constructed from resource @" << (void*)this);
}

//
/// Overload for controls loaded from resource
/// Allows the title specified in the resource data to be overridden.
//
TControl::TControl(TWindow* parent, int resourceId, const tstring& title, TModule* module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, title, module);

  SetFlag(wfFromResource);
  memset(&Attr, 0x0, sizeof(Attr));
  Attr.Id = resourceId;
  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TControl constructed from resource @" << (void*)this);
}


//
/// Constructor to alias a non-OWL control
//
TControl::TControl(THandle handle, TModule* module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(handle, module);

  TRACEX(OwlControl, OWL_CDLEVEL, "TControl aliased @" << (void*)this <<
    " to handle" << uint(handle));
}


//
//
//
TControl::~TControl()
{
  TRACEX(OwlControl, OWL_CDLEVEL, "TControl destructed @" << (void*)this);
}

//
/// Intercept WM_PAINT to redirect from TWindow to the underlying control if
/// this Owl object is just a wrapper for a predefined class.
//
/// If the control has a predefined class, EvPaint calls TWindow::DefaultProcessing
/// for  painting. Otherwise, it calls TWindow::EvPaint.
//
void
TControl::EvPaint()
{
  if (IsFlagSet(wfPredefinedClass))
    DefaultProcessing();  // don't call TWindow::EvPaint()

  else
    TWindow::EvPaint();
}

//
/// Handles WM_COMPAREITEM message (for owner draw controls) by calling the
/// corresponding virtual function.
//
int
TControl::EvCompareItem(uint /*ctrlId*/, const COMPAREITEMSTRUCT& compareInfo)
{
  COMPAREITEMSTRUCT i(compareInfo); // Create copy to support legacy non-const virtual CompareItem.
  return CompareItem(i);
}
//

//
/// Function called when a WM_COMPAREITEM is sent to parent on our behalf
//
/// Also used with owner-draw buttons and list boxes, CompareItem compares two
/// items. The derived class supplies the compare logic.
//
int
TControl::CompareItem(COMPAREITEMSTRUCT & compareInfo)
{
  TRACEX(OwlControl, 1, "TControl::CompareItem() @" << (void*)this);
  if (!GetHandle() || compareInfo.itemData1 == compareInfo.itemData2) 
    return 0;
  if(compareInfo.itemData1){
    TDrawItem* item = ItemData2DrawItem(compareInfo.itemData1);
    if(item)
      return item->Compare(compareInfo);
  }
  return 0;
}

//
/// Handles WM_DELETEITEM message (for owner draw controls) by calling the
/// corresponding virtual function.
//
void
TControl::EvDeleteItem(uint /*ctrlId*/, const DELETEITEMSTRUCT& deleteInfo)
{
  DELETEITEMSTRUCT i(deleteInfo); // Create copy to support legacy non-const virtual DeleteItem.
  DeleteItem(i);
}

//
/// Function called when a WM_DELETEITEM is sent to parent on our behalf
//
/// Used with owner-draw buttons and list boxes. In such cases, the derived class
/// supplies the delete logic.
//
void
TControl::DeleteItem(DELETEITEMSTRUCT &)
{
  TRACEX(OwlControl, 1, "TControl::DeleteItem() @" << (void*)this);
}

//
/// Handles WM_MEASUREITEM message (for owner draw controls) by calling the
/// corresponding virtual function.
//
void
TControl::EvMeasureItem(uint /*ctrlId*/, MEASUREITEMSTRUCT & measureInfo)
{
  MeasureItem(measureInfo);
}

//
/// Function called when a WM_MEASUREITEM is sent to parent on our behalf
//
/// Used by owner-drawn controls to set the dimensions of the specified item. For
/// list boxes and control boxes, this function applies to specific items; for other
/// owner-drawn controls, this function is used to set the total size of the
/// control.
//
void
TControl::MeasureItem(MEASUREITEMSTRUCT & measureInfo)
{
  TRACEX(OwlControl, 1, "TControl::MeasureItem() @" << (void*)this);
  if(measureInfo.itemData){
    TDrawItem* item = ItemData2DrawItem(measureInfo.itemData);
    if(item)
      item->Measure(measureInfo);
  }
}

//
/// Handles WM_DRAWITEM message (for owner draw controls) by calling the
/// corresponding virtual function.
//
void
TControl::EvDrawItem(uint /*ctrlId*/, const DRAWITEMSTRUCT& drawInfo)
{
  DRAWITEMSTRUCT i(drawInfo); // Create copy to support legacy non-const virtual DrawItem.
  DrawItem(i);
}

//
/// Function called when a WM_DRAWITEM is sent to parent on our behalf. This
/// is in turn broken up into one of three draw events.
//
/// Responds to a message forwarded by TWindow to a drawable control when the
/// control needs to be drawn. DrawItem calls one of the following: ODADrawEntire() if
/// the entire control needs to be drawn, ODASelect() if the selection state of the
/// control has changed, or ODAFocus() if the focus has been shifted to or from the
/// control.
//
void
TControl::DrawItem(DRAWITEMSTRUCT & drawInfo)
{
  switch (drawInfo.itemAction) {
    case ODA_DRAWENTIRE:
      ODADrawEntire(drawInfo);
      break;

    case ODA_FOCUS:
      ODAFocus(drawInfo);
      break;

    case ODA_SELECT:
      ODASelect(drawInfo);
      break;
  }
}

//
/// Responds to a notification message sent to a drawable control when the control
/// needs to be drawn. ODADrawEntire can be redefined by a drawable control to
/// specify the manner in which it is to be drawn.
//
void
TControl::ODADrawEntire(DRAWITEMSTRUCT & drawInfo)
{
  TRACEX(OwlControl, 1, "TControl::ODADrawEntire() @" << (void*)this);
  if(drawInfo.itemData && drawInfo.itemID != (uint16)-1){
    TDrawItem* item = ItemData2DrawItem(drawInfo.itemData);
    if(item)
      item->Draw(drawInfo);
  }
}

//
/// Responds to a notification sent to a drawable control when the focus has shifted
/// to or from the control. ODAFocus can be redefined by a drawable control to
/// specify the manner in which it is to be drawn when losing or gaining the focus.
//
void
TControl::ODAFocus(DRAWITEMSTRUCT & drawInfo)
{
  TRACEX(OwlControl, 1, "TControl::ODAFocus() @" << (void*)this);
  if(drawInfo.itemData){
    TDrawItem* item = ItemData2DrawItem(drawInfo.itemData);
    if(item)
      item->Draw(drawInfo);
  }
}

//
/// Responds to a notification sent to a drawable control when the selection state
/// of the control changes. ODASelect can be redefined by a drawable control to
/// specify the manner in which it is drawn when its selection state changes.
//
void
TControl::ODASelect(DRAWITEMSTRUCT & drawInfo)
{
  TRACEX(OwlControl, 1, "TControl::ODASelect() @" << (void*)this);
  if(drawInfo.itemData){
    TDrawItem* item = ItemData2DrawItem(drawInfo.itemData);
    if(item)
      item->Draw(drawInfo);
  }
}


IMPLEMENT_STREAMABLE1(TControl, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TControl::Streamer::Read(ipstream& is, uint32 version) const
{
  TControl* o = GetObject();
  ReadBaseObject((TWindow*)o, is);

  if (version > 1) {
    int nu;
    is >> nu; //Old NativeUse flag. Ignored in 6.32 and later versions
    static_cast<void>(nu); // Eliminate 'never used' warning.
  }

  return o;
}

//
//
//
void
TControl::Streamer::Write(opstream& os) const
{
  TControl* o = GetObject();
  WriteBaseObject((TWindow*)o, os);
  os << 0; //Old NativeUse flag. Ignored in 6.32 and later versions
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace


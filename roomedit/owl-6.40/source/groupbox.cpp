//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TGroupBox.  This defines the basic behavior for all
/// group boxes.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/groupbox.h>
#include <owl/applicat.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

namespace owl {

OWL_DIAGINFO;


//
/// Constructs a group box object with the supplied parent window (Parent), control
/// ID (Id), associated text (text), position (x, y) relative to the origin of the
/// parent window's client area, width (w), and height (h). Invokes the TControl
/// constructor with similar parameters, then modifies Attr.Style, adding
/// BS_GROUPBOX and removing WS_TABSTOP. NotifyParent is set to true; by default,
/// the group box's parent is notified when a selection change occurs in any of the
/// group box's controls.
//
TGroupBox::TGroupBox(TWindow*        parent,
                     int             id,
                     LPCTSTR         text,
                     int x, int y, int w, int h,
                     TModule*        module)
:
  TControl(parent, id, text, x, y, w, h, module)
{
  NotifyParent = true;
  Attr.Style = (Attr.Style | BS_GROUPBOX) & ~WS_TABSTOP;
}

//
/// Constructs a group box object with the supplied parent window (Parent), control
/// ID (Id), associated text (text), position (x, y) relative to the origin of the
/// parent window's client area, width (w), and height (h). Invokes the TControl
/// constructor with similar parameters, then modifies Attr.Style, adding
/// BS_GROUPBOX and removing WS_TABSTOP. NotifyParent is set to true; by default,
/// the group box's parent is notified when a selection change occurs in any of the
/// group box's controls.
//
TGroupBox::TGroupBox(TWindow* parent,
                     int id,
                     const tstring& text,
                     int x, int y, int w, int h,
                     TModule* module)
:
  TControl(parent, id, text, x, y, w, h, module)
{
  NotifyParent = true;
  Attr.Style = (Attr.Style | BS_GROUPBOX) & ~WS_TABSTOP;
}

//
/// GetClassName returns the name of TGroupBox's Windows registration class, "BUTTON" 
//
TWindow::TGetClassNameReturnType
TGroupBox::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled())
    return SHADE_CLASS;
  else
#endif
    return _T("BUTTON");
}

//
/// Constructs a TGroupBox object to be associated with a group box control of a
/// TDialog. object. Invokes the TControl constructor with identical parameters.
/// resourceID must correspond to a group box resource that you define.
///
/// By default, the parent window is notified when the state of the group box's
/// selection boxes has changed
///
/// Disables transfer of state data for the TGroupBox
//
TGroupBox::TGroupBox(TWindow*   parent,
                     int        resourceId,
                     TModule*   module)
:
  TControl(parent, resourceId, module)
{
  NotifyParent = true;
  DisableTransfer();
}

//
/// If NotifyParent is true, SelectionChanged notifies the parent window of the
/// group box that one of its selections has changed by sending it a child-ID-based
/// message. This member function can be redefined to allow the group box to handle
/// selection changes in its group of controls.
//
void
TGroupBox::SelectionChanged(int controlId)
{
  if (NotifyParent)
    GetParentO()->PostMessage(WM_COMMAND, MkParam1(Attr.Id, controlId),
                        TParam2(GetHandle()));
}

struct TGroupBoxGetText
{
  TGroupBox& groupbox;
  TGroupBoxGetText(TGroupBox& g) : groupbox(g) {}

  int operator()(LPTSTR buf, int buf_size)
  {return groupbox.GetText(buf, buf_size);}
};

//
/// String-aware overload
//
tstring TGroupBox::GetText() 
{
  return CopyText(GetTextLen(), TGroupBoxGetText(*this));
}




IMPLEMENT_STREAMABLE1(TGroupBox, TControl);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TGroupBox from the passed ipstream
//
void*
TGroupBox::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  is >> GetObject()->NotifyParent;
  return GetObject();
}

//
// writes the TGroupBox to the passed opstream
//
void
TGroupBox::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
  os << GetObject()->NotifyParent;
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TRadioButton.  This defines the basic behavior for
/// all radio buttons.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/radiobut.h>
#include <owl/groupbox.h>
#include <owl/applicat.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TRadioButton, TCheckBox)
  EV_NOTIFY_AT_CHILD(BN_CLICKED, BNClicked),
END_RESPONSE_TABLE;

//
/// Constructs a radio button object with the supplied parent window (parent),
/// control ID (id), associated text (title), position (x, y) relative to the origin
/// of the parent window's client area, width (w), height (h), and associated group
/// box (group). Invokes the TCheckBox constructor with similar parameters. The
/// style is set to WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON.
//
TRadioButton::TRadioButton(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h, TGroupBox* group, TModule* module)
:
  TCheckBox(parent, id, title, x, y, w, h, group, module)
{
  Attr.Style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
}

//
/// String-aware overload
//
TRadioButton::TRadioButton(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h, TGroupBox* group, 
  TModule* module)
:
  TCheckBox(parent, id, title, x, y, w, h, group, module)
{
  Attr.Style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
}

//
/// Constructs a TRadioButton object to be associated with a radio button control of
/// a TDialog object. Invokes the TCheckBox constructor with identical parameters.
/// The resourceId parameter must correspond to a radio button resource that you
/// define.
//
TRadioButton::TRadioButton(TWindow* parent, int resourceId, TGroupBox* group, TModule* module)
:
  TCheckBox(parent, resourceId, group, module)
{}

//
/// Constructs a TRadioButton object to encapsulate (alias) an existing control.
//
TRadioButton::TRadioButton(THandle hWnd, TModule* module)
:
  TCheckBox(hWnd, module)
{}

//
/// Returns "BUTTON", the name of the predefined radio button class.
//
TWindow::TGetClassNameReturnType
TRadioButton::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled())
    return RADIO_CLASS;
  else
#endif
    return _T("BUTTON");
}

//
/// Responds to an incoming BN_CLICKED message.
//
/// \note Need to see if it's checked because Windows generates two BN_CLICKED
/// messages on keyboard input such as up arrow(but only one on mouse input),
/// and we should only handle the one after it's checked
//
void
TRadioButton::BNClicked()
{
  if (GetCheck())
    TCheckBox::BNClicked();

  else
    DefaultProcessing();
}


IMPLEMENT_STREAMABLE1(TRadioButton, TCheckBox);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TRadioButton from the passed ipstream
//
void*
TRadioButton::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TCheckBox*)GetObject(), is);
  return GetObject();
}

//
// writes the TRadioButton to the passed opstream
//
void
TRadioButton::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TCheckBox*)GetObject(), os);
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


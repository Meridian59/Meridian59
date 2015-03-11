//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TButton.  This defines the basic behavior of all
/// buttons.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/button.h>
#include <owl/applicat.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);


DEFINE_RESPONSE_TABLE1(TButton, TControl)
  EV_WM_GETDLGCODE,
  EV_MESSAGE(BM_SETSTYLE, BMSetStyle), //to analize later why GCC reports a bug here
END_RESPONSE_TABLE;


//
/// Constructs a button object with the supplied parent window (parent), control ID
/// (Id), associated text (text), position (X,Y) relative to the origin of the
/// parent window's client area, width (W), and height (H). If IsDefault is true,
/// the constructor adds BS_DEFPUSHBUTTON to the default styles set for the TButton
/// (in Attr.Style). Otherwise, it adds BS_PUSHBUTTON.
//
TButton::TButton(TWindow* parent, int id, LPCTSTR text, int x, int y, int w, int h, bool isDefault, TModule* module)
:
  TControl(parent, id, text, x, y, w, h, module)
{
  IsCurrentDefPB = false;    // not used for buttons in windows
  IsDefPB = false;           // not used for buttons in windows

  if (isDefault)
    Attr.Style |= BS_DEFPUSHBUTTON;

  else
    Attr.Style |= BS_PUSHBUTTON;
  TRACEX(OwlControl, OWL_CDLEVEL, "TButton constructed @" << (void*)this);
}

//
/// String-aware overload
//
TButton::TButton(TWindow* parent, int id, const tstring& text, int x, int y, int w, int h, bool isDefault, TModule* module)
:
  TControl(parent, id, text, x, y, w, h, module)
{
  IsCurrentDefPB = false;    // not used for buttons in windows
  IsDefPB = false;           // not used for buttons in windows

  if (isDefault)
    Attr.Style |= BS_DEFPUSHBUTTON;

  else
    Attr.Style |= BS_PUSHBUTTON;
  TRACEX(OwlControl, OWL_CDLEVEL, "TButton constructed @" << (void*)this);
}

//
/// Constructs a TButton object to be associated with a button control of a TDialog
/// object. Calls DisableTransfer to exclude the button from the transfer mechanism
/// because there is no data to be transferred.
/// The resId parameter must correspond to a button resource that you define.
//
TButton::TButton(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  DisableTransfer();
  IsDefPB = false;         // needed for drawable buttons
  IsCurrentDefPB = false;  // needed for drawable buttons
  TRACEX(OwlControl, OWL_CDLEVEL, "TButton constructed from resource @" << (void*)this);
}

//
/// Constructs a TButton object to encapsulate (alias) an existing control.
//
TButton::TButton(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  // TODO: Test the effect of these initializations.
  //
  IsDefPB = GetStyle() & BS_DEFPUSHBUTTON; 
  IsCurrentDefPB = false;
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TButton alias constructed @") << (void*)this);
}

//
/// Output a debug message if using the diagnostic libraries.
//
TButton::~TButton()
{
  TRACEX(OwlControl, OWL_CDLEVEL, "TButton destructed @" << (void*)this);
}

//
/// Return name of predefined Windows button class
//
/// Overrides TWindow's GetClassName function. Returns the name "BUTTON".
//
TWindow::TGetClassNameReturnType
TButton::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled()) {
    TRACEX(OwlControl, 1, "BWCC button used for classname @" << (void*)this);
    return BUTTON_CLASS;
  }
#endif
  TRACEX(OwlControl, 1, "Regular button used for classname @" << (void*)this);
  return _T("BUTTON");
}

//
/// Overrides TWindow's SetupWindow function. If the button is the default push
/// button and an owner-drawn button, SetupWindow sends a DM_SETDEFID message to the
/// parent window.
//
/// \note this only works (and IsDefPB should only be true) if Parent is a dialog
//
void
TButton::SetupWindow()
{
  TRACEX(OwlControl, 1, "TButton::SetupWindow() @" << (void*)this);
  if (IsDefPB && ((Attr.Style & BS_OWNERDRAW) == BS_OWNERDRAW))
    Parent->HandleMessage(DM_SETDEFID, Attr.Id);
}

//
/// Responds to WM_GETDLGCODE messages that are sent to a dialog box associated with
/// a control. EvGetDlgCode allows the dialog manager to intercept a message that
/// would normally go to a control and then ask the control if it wants to process
/// this message. If not, the dialog manager processes the message. The msg
/// parameter indicates the kind of message (for example, a control, command, or
/// button message) sent to the dialog box manager.
/// 
/// EvGetDlgCode returns a code that indicates how the button is to be treated. If
/// this is the currently used push button, EvGetDlgCode  returns either
/// DLGC_DEFPUSHBUTTON or DLGC_UNDEFPUSHBUTTON.
///
/// if this is a drawable button we tell Windows whether we want to
/// be treated as the current default push button or not
//
uint
TButton::EvGetDlgCode(const MSG* msg)
{
  if ((Attr.Style & BS_OWNERDRAW) != BS_OWNERDRAW)
    return TControl::EvGetDlgCode(msg);

  else if (IsCurrentDefPB)
    return DLGC_BUTTON | DLGC_DEFPUSHBUTTON;

  else
    return DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON;
}

//
/// Because a button can't have both owner-drawn and push button styles, BMSetStyle
/// keeps track of the style if the button is owner-drawn and the system tries to
/// set the style to BS_DEFPUSHBUTTON. BMSetStyle sets IsCurrentDefPB to true if the
/// button style is BS_DEFPUSHBUTTON.
//
TResult
TButton::BMSetStyle(TParam1 param1, TParam2)
{
  if ((Attr.Style & BS_OWNERDRAW) != BS_OWNERDRAW)
    DefaultProcessing();
  else if (param1 == BS_DEFPUSHBUTTON) {
    IsCurrentDefPB = true;
    Invalidate();
  }
  else if (param1 == BS_PUSHBUTTON) {
    IsCurrentDefPB = false;
    Invalidate();
  }
  else {
    DefaultProcessing();
  }

  return 0;
}

IMPLEMENT_STREAMABLE1(TButton, TControl);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TButton::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  is >> GetObject()->IsDefPB;
  return GetObject();
}

//
//
//
void
TButton::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
  os << GetObject()->IsDefPB;
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace


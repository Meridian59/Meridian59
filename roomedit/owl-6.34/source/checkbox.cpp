//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implemenation of class TCheckBox.  This defines the basic behavior for all
/// check boxes.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/checkbox.h>
#include <owl/groupbox.h>
#include <owl/applicat.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);


DEFINE_RESPONSE_TABLE1(TCheckBox, TButton)
  EV_WM_GETDLGCODE,
  EV_NOTIFY_AT_CHILD(BN_CLICKED, BNClicked),
END_RESPONSE_TABLE;

//
/// constructor for a TCheckBox object
//
/// Constructs a check box object with the specified parent window (parent), control
/// ID (Id), associated text (title), position relative to the origin of the parent
/// window's client area (x, y), width (w), height (h), associated group box
/// (group), and owning module (module). Invokes the TButton constructor with
/// similar parameters. Sets the check box's style to WS_CHILD | WS_VISIBLE |
/// WS_TABSTOP | BS_AUTOCHECKBOX.
//
TCheckBox::TCheckBox(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h, TGroupBox* group, TModule* module)
:
  TButton(parent, id, title, x, y, w, h, false, module)
{
  Group = group;
  //
  // Don't use TButton's inherited style - it conflicts with BS_AUTOCHECKBOX
  //
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;

  TRACEX(OwlControl, OWL_CDLEVEL, _T("TCheckBox constructed @") << (void*)this);
}

//
/// String-aware overload
//
TCheckBox::TCheckBox(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h, TGroupBox* group, 
  TModule* module)
:
  TButton(parent, id, title, x, y, w, h, false, module)
{
  Group = group;
  //
  // Don't use TButton's inherited style - it conflicts with BS_AUTOCHECKBOX
  //
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;

  TRACEX(OwlControl, OWL_CDLEVEL, _T("TCheckBox constructed @") << (void*)this);
}

//
/// Constructs a TCheckButton object to encapsulate (alias) an existing control.
//
TCheckBox::TCheckBox(THandle hWnd, TModule* module)
:
  TButton(hWnd, module),
  Group(0)
{
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TCheckBox alias constructed @") << (void*)this);
}


//
/// Constructs an associated TCheckBox object for the check box control with a
/// resource ID of resourceId in the parent dialog box. Sets Group to group then
/// enables the data transfer mechanism by calling TWindow::EnableTransfer.
//
TCheckBox::TCheckBox(TWindow*   parent,
                     int        resourceId,
                     TGroupBox* group,
                     TModule*   module)
:
  TButton(parent, resourceId, module)
{
  Group = group;
  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, _T("TCheckBox constructed from resource @") << (void*)this);
}

//
/// Output a trace message if using the diagnostic libraries.
//
TCheckBox::~TCheckBox()
{
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TCheckBox destructed @") << (void*)this);
}

//
/// Return name of Windows check box class
//
/// returns "BUTTON."
//
TWindow::TGetClassNameReturnType
TCheckBox::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled()) {
    TRACEX(OwlControl, 1, _T("BWCC checkbox used for classname @") << (void*)this);
    // !CQ to be consistent, we should do this trace for ALL bwcc-able controls,
    // !CQ or none.
    return CHECK_CLASS;
  }
#endif
  TRACEX(OwlControl, 1, _T("Regular checkbox used for classname @") << (void*)this);
  return _T("BUTTON");
}

//
/// transfers state information for the TCheckBox
//
/// Overrides TWindow::Transfer. Transfers the check state of the check box to or
/// from buffer, using the values specified in the table in GetCheck(). If direction
/// is tdGetDate, the check box state is transferred into the buffer. If direction
/// is tdSetData, the check box state is changed to the settings in the transfer
/// buffer.
/// Transfer() returns the size of the transfer data in bytes. To get the size without
/// actually transferring the check box, use tdSizeData as the direction argument.
//
uint
TCheckBox::Transfer(void* buffer, TTransferDirection direction)
{
  if (direction == tdGetData)
    *(uint16*)buffer = (uint16)GetCheck();

  else if (direction == tdSetData)
    SetCheck(*(uint16*)buffer);

  return sizeof(uint16);
}

//
/// Forces the check box into the state specified by check.
//
/// unchecks, checks, or grays the checkbox (if 3-state) according to the
/// CheckFlag passed (pass BF_UNCHECKED(0), BF_CHECKED(1), or BF_GRAYED(2))
//
/// if a Group has been specified for the TCheckBox, notifies the Group that
/// the state of the check box has changed
//
void
TCheckBox::SetCheck(uint check)
{
  PRECONDITION(GetHandle());
  SendMessage(BM_SETCHECK, check);

  if (Group)
    Group->SelectionChanged(Attr.Id);
}

//
/// Toggles the check state of the check box
//
/// Toggles the check box between checked and unchecked if it is a two-state check
/// box; toggles it between checked, unchecked, and gray if it is a three-state
/// check box.
//
void
TCheckBox::Toggle()
{
  if ((GetWindowLong(GWL_STYLE) & BS_AUTO3STATE) == BS_AUTO3STATE)
    SetCheck((GetCheck() + 1) % 3);

  else
    SetCheck((GetCheck() + 1) % 2);
}

//
/// Responds to an incoming BN_CLICKED message
//
/// Responds to notification message BN_CLICKED, indicating that the user clicked
/// the check box. If Group isn't 0, BNClicked() calls the group box's
/// SelectionChanged() member function to notify the group box that the state has
/// changed.
//
void
TCheckBox::BNClicked()
{
  if (Group)
    Group->SelectionChanged(Attr.Id);

  DefaultProcessing();  // give parent an opportunity to handle...
}

IMPLEMENT_STREAMABLE1(TCheckBox, TButton);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// reads an instance of TCheckBox from the passed ipstream
//
void*
TCheckBox::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TButton*)GetObject(), is);
  is >> GetObject()->Group;
  return GetObject();
}

//
/// writes the TCheckBox to the passed opstream
//
void
TCheckBox::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TButton*)GetObject(), os);
  os << GetObject()->Group;
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace
///////////////////////////////////////


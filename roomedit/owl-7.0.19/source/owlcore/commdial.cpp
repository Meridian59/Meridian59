//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of Common Dialog abstract base class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/commdial.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

// Diagnostic group for Common Dialog
OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlCommDialog, 1, 0);

DEFINE_RESPONSE_TABLE1(TCommonDialog, TDialog)
  EV_WM_CLOSE,
  EV_COMMAND(IDOK, CmOkCancel),
  EV_COMMAND(IDCANCEL, CmOkCancel),
END_RESPONSE_TABLE;

//
/// Invokes a TWindow constructor, passing the parent window parent, and constructs
/// a common dialog box that you can modify to suit your specifications. You can
/// indicate the module ID (module) and window caption (title), which otherwise
/// default to 0 and blank, respectively.
//
TCommonDialog::TCommonDialog(TWindow*  parent,
                             LPCTSTR   title,
                             TModule*  module)
:
  TDialog(parent, 0, module),
  CDTitle(title ? title : _T(""))
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog constructed @" << (void*)this);
}

//
/// String-aware overload
//
TCommonDialog::TCommonDialog(TWindow*  parent, const tstring& title, TModule*  module)
  : TDialog(parent, title.c_str(), module),
  CDTitle(title)
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog constructed @" << (void*)this);
}

//
/// Generate message in diagnostic libraries.
//
TCommonDialog::~TCommonDialog()
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog destructed @" << (void*)this);
}

//
/// Assigns the caption of the dialog box to CDTitle if CDTitle is non-blank.
///
/// Override virtual to set the caption.
//
void
TCommonDialog::SetupWindow()
{
  TRACEX(OwlCommDialog, 1, "TCommonDialog::SetupWindow() @" << (void*)this);

  TDialog::SetupWindow();
  if (!CDTitle.empty())
    SetCaption(CDTitle);
}

//
/// Default handler for a modeless common dialog.
/// Wrong usage if here.
//
TWindow::THandle
TCommonDialog::PerformCreate()
{
  PRECONDITIONX(false, _T("Wrong usage for a modal common dialog. Use Execute() instead."));
  return nullptr;
}

//
/// Default handler for a modal common dialog.
/// Wrong usage if here.
//
/// Called by Execute, DoExecute executes a modal dialog box. If the dialog box
/// execution is canceled or unsuccessful, DoExecute returns IDCANCEL.
//
int TCommonDialog::DoExecute()
{
  TRACEX(OwlCommDialog, 0, "Wrong usage for a modeless common dialog. Use Create() instead.");
  return IDCANCEL;
}

} // OWL namespace
/* ========================================================================== */

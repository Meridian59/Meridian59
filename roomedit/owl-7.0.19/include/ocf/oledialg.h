//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of class TOleDialog, a TDialog that can host OLE controls.
//----------------------------------------------------------------------------

#if !defined(OCF_OLEDIALG_H)
#define OCF_OLEDIALG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

/// \cond

// Class name of stub control within a dialog which TOleDialog replaces with
// an OCX at runtime. The caption of the control contains the ProgID of the
// OCX to be created by TOleDialog.
//
#if !defined(OCX_STUB_CLASS)
# define OCX_STUB_CLASS _T("OCX")
#endif

/// \endcond

#if !defined(RC_INVOKED)

#include <owl/dialog.h>
#include <ocf/olewindo.h>


namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TOleDialog
// ~~~~~ ~~~~~~~~~~
/// Provides OLE support to dialog boxes.
class _OCFCLASS TOleDialog :  public TOleWindow, public owl::TDialog
{
  public:
    TOleDialog(owl::TWindow* parent, owl::TResId resId, owl::TModule* module = 0);
   ~TOleDialog();

    void SetupWindow() override;
    auto IdleAction(long idleCount) -> bool override;
    auto PreProcessMsg(MSG&) -> bool override;

  protected:
    bool      EvOcViewSetSiteRect(owl::TRect * rect);

  protected:
    HWND      CheckChild(HWND);
    void      LoadControl(HWND hControl);

    /// Creates a TOcControl object that is initialized with the control's class
    /// (calculated from ProgId) and position.
    /// \todo This function is not implemented
    void      LoadControl(owl::TString ProgId, const owl::TRect& position);

  DECLARE_RESPONSE_TABLE(TOleDialog);
};

//
/// \class TRegisterOcxWnd
// ~~~~~ ~~~~~~~~~~~~~~~
/// Registers an OCX window for a subsequent call to the TOleDialog::SetupWindow
/// function.
//
class _OCFCLASS TRegisterOcxWnd {
  public:
    TRegisterOcxWnd(HINSTANCE);
   ~TRegisterOcxWnd();

  protected:
/// The application instance for which the OCX window is registered.
    HINSTANCE HAppInst;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


} // OCF namespace


#endif  // !RC_INVOKED

#endif  // OCF_OLEDIALG_H

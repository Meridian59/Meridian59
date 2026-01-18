//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_ROLLDIAL_H)
#define OWL_ROLLDIAL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/dialog.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup dlg
/// @{

//
/// \class TRollDialog
// ~~~~~ ~~~~~~~~~~~
/// Allow a dialog to "roll" up and down, similar to the Corel interface.
/// This class is best used for modeless dialogs.
///
/// When the dialog is created, a menuitem is appended to the system menu.
/// This menu choice will be either Shrink or Expand depending upon which
/// state the dialog is currently in.
///
/// If the dialog contains minimize or maximize buttons, the behaviour
/// of those buttons map to shrink and expand respectively.
//
class _OWLCLASS TRollDialog : public TDialog {
  public:
    TRollDialog(TWindow* parent, TResId resId, bool animated = true,
                bool fullSize = true, TModule* module = 0);

    // Override virtuals
    //
    void SetupWindow() override;

  protected:
    void Shrink();
    void Expand();
    void EvSysCommand(uint, const TPoint&);
    bool IsFullSize;          ///< track if the dialog is currently full size

  private:
    /// Will need some enums to know when the mouse is over the non-client
    /// button or when the user selects the system menu for rolling.
    //
    enum {
      SC_ROLLDIALOG = 0xFF00,
    };

    void Resize(bool currentlyFullSize);

    int  Height;
    bool InitialFullSize;     ///< save constructor parameter
    bool HasSystemMenu;
    bool WantAnimation;

  DECLARE_RESPONSE_TABLE(TRollDialog);
};

/// @}

#include <owl/posclass.h>


} // OWL namespace


#endif // OWL_ROLLDIAL_H

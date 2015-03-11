//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes TPickListPopup & TPickListDialog
//----------------------------------------------------------------------------

#if !defined(OWL_PICKLIST_H)
#define OWL_PICKLIST_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/dialog.h>
#include <owl/listbox.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TPickListPopup
// ~~~~~ ~~~~~~~~~~~~~~
/// The PickList allows selection of an item from a popup list. The list can
/// have an optional title, provided by string or string resource id.
///
/// Strings can be added after construction using AddString()
///
/// The 0-based selection is returned from Execute(), or can be retrieved later
/// using GetResult()
//
class _OWLCLASS TPickListPopup : public TWindow {
  public:
    TPickListPopup(TWindow* parent, LPCTSTR title = 0);
    TPickListPopup(TWindow* parent, const tstring& title);
    TPickListPopup(TWindow* parent, uint titleId);

    void        ClearStrings();
    int         AddString(LPCTSTR str);
    int         AddString(const tstring& str) {return AddString(str.c_str());}
    int         Execute(); // Returns index picked (1 based), 0 if cancelled
    int         GetResult() const;

  protected:
    TResult     EvCommand(uint id, THandle hWndCtl, uint notifyCode);

    TPopupMenu  Popup;
    int         Count;

  private:
    int         Result;
};

/// @}

/// \addtogroup ctrl
/// @{
//
/// \class TPickListDialog
// ~~~~~ ~~~~~~~~~~~~~~~
/// The PickListDialog allows selection of an item from a list in a dialog with
/// OK and Cancel buttons. An initial string list can be provided, and an
/// initial selection. Also, the dialog template and title can be overriden.
///
/// Strings can be added after construction using AddString()
///
/// The 0-based selection is returned from Execute(), or can be retrieved later
/// using GetResult()
//
class _OWLCLASS TPickListDialog : public TDialog {
  public:
    TPickListDialog(TWindow*      parent,
                    TStringArray* strings = 0,
                    int           initialSelection = 0,
                    TResId        templateId = 0,
                    LPCTSTR title = 0,
                    TModule*      module = 0);

    TPickListDialog(
      TWindow* parent,
      TStringArray* strings,
      int initialSelection,
      TResId templateId,
      const tstring& title,
      TModule* module = 0);

   ~TPickListDialog();

    void          ClearStrings();
    int           AddString(LPCTSTR str);
    int           AddString(const tstring& str) {return AddString(str.c_str());}
    int           GetResult() const;

  protected:
    void          SetupWindow();
    void          CmOK();
    void          CmCancel();

  private:
    TListBox      List;
    int           Result;
    TStringArray* Strings;
    bool          NewedStrings;

  DECLARE_RESPONSE_TABLE(TPickListDialog);
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the result of the selection.
//
inline int
TPickListPopup::GetResult() const
{
  return Result;
}

//
/// Returns the result of the selection.
//
inline int
TPickListDialog::GetResult() const
{
  return Result;
}

} // OWL namespace


#endif // OWL_PICKLIST_H

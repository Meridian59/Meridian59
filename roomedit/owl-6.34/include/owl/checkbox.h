//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
///\file 
/// Definition of class TCheckBox. This defines the basic behavior for all check
/// boxes.
//----------------------------------------------------------------------------

#if !defined(OWL_CHECKBOX_H)
#define OWL_CHECKBOX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/button.h>

namespace owl {

class _OWLCLASS TGroupBox;

//
/// Check box and radio button objects use the button flag constants to indicate the
/// state of a selection box.
//
enum {
  BF_CHECKED   = 0x01,	///< Item is checked
  BF_GRAYED    = 0x02,	///< Item is grayed
  BF_UNCHECKED = 0x00,	///< Item is unchecked
};

#include <owl/preclass.h>

//
/// \addtogroup ctrl
/// @{

//
/// \class TCheckBox
/// TCheckBox is a streamable interface class that represents a check box control.
/// Use TCheckBox to create a check box control in a parent window. You can also use
/// TCheckBox objects to more easily manipulate check boxes you created in a dialog
/// box resource.
/// Two-state check boxes can be checked or unchecked; three-state check boxes have
/// an additional grayed state. TCheckBox member functions let you easily control
/// the check box's state. A check box can be in a group box (a TGroupBox object)
/// that groups related controls.
//
class _OWLCLASS TCheckBox 
  : public TButton 
{
  public:

    TCheckBox(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h, TGroupBox* = 0, TModule* = 0);
    TCheckBox(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h, TGroupBox* = 0, TModule* = 0);
    TCheckBox(TWindow* parent, int resId, TGroupBox* = 0, TModule* = 0);
    TCheckBox(THandle hWnd, TModule* = 0);

    ~TCheckBox();

    void Check(bool check = true);
    void Uncheck();
    void Toggle();

    bool IsChecked() const {return GetCheck() == BST_CHECKED;}
    bool IsUnchecked() const {return GetCheck() == BST_UNCHECKED;}
    bool IsIndeterminate() const {return GetCheck() == BST_INDETERMINATE;}

    uint GetCheck() const;
    void SetCheck(uint check);

    TGroupBox* GetGroup() const;
    void SetGroup(TGroupBox* group);

    /// Safe overload
    //
    void Transfer(WORD& state, TTransferDirection direction) {Transfer(&state, direction);}

  protected:

    ///  Override TButton's processing so drawable check boxes and radio
    ///  buttons work properly
    //
    uint       EvGetDlgCode(const MSG*);

    // Child id notification
    //
    void       BNClicked();  // BN_CLICKED

    // Override TWindow virtual member functions
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);
    virtual TGetClassNameReturnType GetClassName();

  public_data:

    /// If the check box belongs to a group box (a TGroupBox object), Group points to
    /// that object. If the check box is not part of a group, Group is zero.
    //
    TGroupBox* Group;

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TCheckBox(const TCheckBox&);
    TCheckBox& operator =(const TCheckBox&);

  DECLARE_RESPONSE_TABLE(TCheckBox);
  DECLARE_STREAMABLE_OWL(TCheckBox, 1);
};

/// @}

DECLARE_STREAMABLE_INLINES( owl::TCheckBox );

#include <owl/posclass.h>

//
/// Forces the check box to be checked by calling SetCheck() with the value
/// BF_CHECKED. Notifies the associated group box, if there is one, that the state
/// was changed.
//
inline void TCheckBox::Check(bool check) {
  SetCheck(check ? BF_CHECKED : BF_UNCHECKED);
}

//
/// Forces the check box to be unchecked by calling SetCheck() with a value of
/// BF_UNCHECKED. Notifies the associated group box, if there is one, that the state
/// has changed.
//
inline void TCheckBox::Uncheck() {
  SetCheck(BF_UNCHECKED);
}

//
/// Returns the group with which the checkbox is associated.
//
inline TGroupBox* TCheckBox::GetGroup() const {
  return Group;
}

//
/// Sets the group with which the checkbox is associated.
//
inline void TCheckBox::SetGroup(TGroupBox* group) {
  Group = group;
}

//
/// Overrides TButton's response to the WM_GETDLGCODE message, an input procedure
/// associated with a control that is not a check box, by calling DefaultProcessing.
/// The msg parameter indicates the kind of message sent to the dialog box manager,
/// such as a control message, a command message, or a check box message.
/// EvGetDlgCode returns a code that indicates how the check box is to be treated.
//
inline uint TCheckBox::EvGetDlgCode(const MSG*) {
  return (uint)DefaultProcessing();
}

//
/// Returns the state of the check box.
//
inline uint TCheckBox::GetCheck() const {
  PRECONDITION(GetHandle());
  return (uint)CONST_CAST(TCheckBox*, this)->SendMessage(BM_GETCHECK);
}


} // OWL namespace


#endif  // OWL_CHECKBOX_H

//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TRadioButton.  This defines the basic behavior for all
/// radio buttons.
//----------------------------------------------------------------------------

#if !defined(OWL_RADIOBUT_H)
#define OWL_RADIOBUT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/checkbox.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TRadioButton
// ~~~~~ ~~~~~~~~~~~~
/// Defines an interface object that represents a corresponding radio button element
/// in Windows. Use TRadioButton to create a radio button control in a parent
/// TWindow object. A TRadioButton object can also be used to facilitate
/// communication between your application and the radio button controls of a
/// TDialog. object.
///
/// Radio buttons have two states: checked and unchecked. TRadioButton inherits its
/// state management member functions from its base class, TCheckBox. Optionally, a
/// radio button can be part of a group (TGroupBox) that visually and logically
/// groups its controls. TRadioButton is a streamable class.
//
class _OWLCLASS TRadioButton
  : public TCheckBox
{
  public:

    TRadioButton(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h, TGroupBox* = 0, TModule* = 0);
    TRadioButton(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h, TGroupBox* = 0, TModule* = 0);
    TRadioButton(TWindow* parent, int resId, TGroupBox* = 0, TModule* = 0);
    TRadioButton(THandle hWnd, TModule* = 0);

  protected:
    // Child id notification handled at the child
    //
    void       BNClicked();  // BN_CLICKED
    auto GetWindowClassName() -> TWindowClassName override;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TRadioButton(const TRadioButton&);
    TRadioButton& operator=(const TRadioButton&);

  DECLARE_RESPONSE_TABLE(TRadioButton);
  DECLARE_STREAMABLE_OWL(TRadioButton, 1);
};

DECLARE_STREAMABLE_INLINES(owl::TRadioButton);

/// @}

#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_RADIOBUT_H

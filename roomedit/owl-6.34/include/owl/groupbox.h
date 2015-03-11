//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TGroupBox.  This defines the basic behavior for all
/// group boxes.
//----------------------------------------------------------------------------

#if !defined(OWL_GROUPBOX_H)
#define OWL_GROUPBOX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TGroupBox
// ~~~~~ ~~~~~~~~~
/// An instance of a TGroupBox is an interface object that represents a
/// corresponding group box element. Generally, TGroupBox objects are not used in
/// dialog boxes or dialog windows (TDialog), but are used when you want to create a
/// group box in a window.
/// 
/// Although group boxes do not serve an active purpose onscreen, they visually
/// unify a group of selection boxes such as check boxes and radio buttons or other
/// controls. Behind the scenes, however, they can take an important role in
/// handling state changes for their group of controls (normally check boxes or
/// radio buttons).
/// 
/// For example, you might want to respond to a selection change in any one of a
/// group of radio buttons in a similar manner. You can do this by deriving a class
/// from TGroupBox that redefines the member function SelectionChanged.
/// 
/// Alternatively, you could respond to selection changes in the group of radio
/// buttons by defining a response for the group box's parent. To do so, define a
/// child-ID-based response member function using the ID of the group box. The group
/// box will automatically send a child-ID-based message to its parent whenever the
/// radio button selection state changes. This class is streamable.
//
class _OWLCLASS TGroupBox : public TControl {
  public:
    TGroupBox(TWindow*        parent,
              int             id,
              LPCTSTR         text,
              int X, int Y, int W, int H,
              TModule*        module = 0);
    TGroupBox(TWindow* parent,
              int id,
              const tstring& text,
              int X, int Y, int W, int H,
              TModule* module = 0);

    TGroupBox(TWindow* parent, int resourceId, TModule* module = 0);

    /// Returns the length of the control's text
    //
    int   GetTextLen();

    /// Fills the given string with the text of the control.  Returns the
    /// number of characters copied.
    int   GetText(LPTSTR str, int maxChars);
    tstring GetText();
    /// Sets the contents of the associated static text control to the given string.
    void  SetText(LPCTSTR str);
    void  SetText(const tstring& str) {SetText(str.c_str());}

    virtual void SelectionChanged(int controlId);

    bool   GetNotifyParent() const;
    void   SetNotifyParent(bool notifyparent);

  protected:
    virtual TGetClassNameReturnType GetClassName();

  public_data:
/// Flag that indicates whether parent is to be notified when the state of the group
/// box's selection boxes has changed. NotifyParent is true by default.
    bool  NotifyParent;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TGroupBox(const TGroupBox&);
    TGroupBox& operator =(const TGroupBox&);

  //DECLARE_STREAMABLE(_OWLCLASS, owl::TGroupBox, 1);
  DECLARE_STREAMABLE_OWL(TGroupBox, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TGroupBox );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
inline int TGroupBox::GetTextLen() {
  return ::GetWindowTextLength(GetHandle());
}

//
inline int TGroupBox::GetText(LPTSTR str, int maxChars) {
  return ::GetWindowText(GetHandle(), str, maxChars);
}

//
inline void TGroupBox::SetText(LPCTSTR str) {
  ::SetWindowText(GetHandle(), str);
}

//
/// Returns the flag that indicates whether or not the parent is notified.
//
inline bool TGroupBox::GetNotifyParent() const
{
  return NotifyParent;
}

//
/// Sets the flag that indicates whether or not the parent is notified.
//
inline void TGroupBox::SetNotifyParent(bool notifyparent)
{
  NotifyParent = notifyparent;
}

} // OWL namespace


#endif  // OWL_GROUPBOX_H

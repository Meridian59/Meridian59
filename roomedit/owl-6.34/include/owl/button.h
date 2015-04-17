//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TButton. This defines the basic behavior of all buttons.
//----------------------------------------------------------------------------

#if !defined(OWL_BUTTON_H)
#define OWL_BUTTON_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>

namespace owl {

#include <owl/preclass.h>

//
/// \addtogroup ctrl
/// @{

//
/// \class TButton
/// TButton is an interface class derived from TControl that represents a pushbutton
/// interface element. You must use a TButton to create a button control in a parent
/// TWindow object. You can also use a TButton to facilitate communication between
/// your application and the button controls of a TDialog object. This class is
/// streamable.
/// 
/// There are two types of pushbuttons: regular and default. Regular buttons have a
/// thin border. Default buttons (which represent the default action of the window)
/// have a thick border.
//
class _OWLCLASS TButton 
  : public TControl 
{
  public:

    TButton(TWindow* parent, int id, LPCTSTR text, int X, int Y, int W, int H, bool isDefault = false, TModule* = 0);
    TButton(TWindow* parent, int id, const tstring& text, int X, int Y, int W, int H, bool isDefault = false, TModule* = 0);
    TButton(TWindow* parent, int resourceId, TModule* = 0);
    TButton(THandle hWnd, TModule* = 0);

    virtual ~TButton();

    bool GetIsDefPB() const;
    void SetIsDefPB(bool isdefpb);

    bool GetIsCurrentDefPB() const;
    void SetIsCurrentDefPB(bool is);

    uint GetState() const;
    void SetState(uint state);

    void SetStyle(uint style, bool redraw);

    void Click();

    /// For use with SetImage method.
    //
    enum TImageType {Bitmap=IMAGE_BITMAP, Icon=IMAGE_ICON};

    HANDLE GetImage() const;
    HANDLE SetImage(HANDLE image, TImageType imageType = Bitmap);

  protected:

    // Message response functions
    //
    uint EvGetDlgCode(const MSG*);
    TResult BMSetStyle(TParam1, TParam2);

    // Override TWindow member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    virtual void SetupWindow();

  public_data:

    /// Indicates whether the button is to be considered the default push button. Used
    /// for owner-draw buttons, IsDefPB is set by a TButton constructor based on
    /// BS_DEFPUSHBUTTON style setting.
    //
    bool IsDefPB;

  protected_data:

    /// Indicates whether the current button is the default push button.
    //
    bool IsCurrentDefPB;

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TButton(const TButton&);
    TButton& operator=(const TButton&);

  DECLARE_RESPONSE_TABLE(TButton);
  DECLARE_STREAMABLE_OWL(TButton, 1);
};

/// @}

DECLARE_STREAMABLE_INLINES( owl::TButton );

#include <owl/posclass.h>

//
// button notification response table entry macros, methods are: void method()
//
// EV_BN_CLICKED(id, method)
// EV_BN_DISABLE(id, method)
// EV_BN_DOUBLECLICKED(id, method)
// EV_BN_HILITE(id, method)
// EV_BN_PAINT(id, method)
// EV_BN_UNHILITE(id, method)

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Return true if this button is the default pushbutton.
//
inline bool TButton::GetIsDefPB() const
{
  return IsDefPB;
}

//
/// Set this button the default pushbutton.
//
inline void TButton::SetIsDefPB(bool isdefpb)
{
  IsDefPB = isdefpb;
}

//
/// Return true if this button is the current default pushbutton.
//
inline bool TButton::GetIsCurrentDefPB() const
{
  return IsCurrentDefPB;
}

//
/// Set this button the current default push button.
//
inline void TButton::SetIsCurrentDefPB(bool is)
{
  IsCurrentDefPB = is;
}

//
/// Simulate clicking of this button
//
/// Sends a BM_CLICK message to simulate the user clicking a button.
/// This message causes the button to receive a WM_LBUTTONDOWN and a WM_LBUTTONUP
/// message, and the button's parent window to receive a BN_CLICKED notification
/// message.
//
inline void TButton::Click()
{
  PRECONDITION(GetHandle());
  SendMessage(BM_CLICK);
}

//
/// Return the handle (icon or bitmap) of the image associated with the
/// button; NULL if there is no associated image.
//
inline HANDLE TButton::GetImage() const
{
  PRECONDITION(GetHandle());
  return (HANDLE)CONST_CAST(TButton*, this)->SendMessage(BM_GETIMAGE);
}

//
/// Associates a new image (icon or bitmap) with the button. newImage is
/// the handle of the new image. imageType specifies the type of image. The return
/// value is the handle of the image previously associated with the button, if any;
/// otherwise, it is NULL.
//
inline HANDLE TButton::SetImage(HANDLE newImage, TImageType imageType)
{
  PRECONDITION(GetHandle());
  return (HANDLE)SendMessage(BM_SETIMAGE, (TParam1)imageType, (TParam2)newImage);
}

//
/// Set the highlight state of a button. The highlight state indicates whether the
/// button is highlighted as if the user had pushed it. If state is true  the button
/// is highlighted; a value of false removes any highlighting. 
//
inline void TButton::SetState(uint state)
{
  PRECONDITION(GetHandle());
  SendMessage(BM_SETSTATE, state);
}

//
/// Returns the current state of the button. The following bit masks can be used to
/// determine the state:
/// 
/// - \c \b  BST_CHECKED	Indicates the button is checked.
/// - \c \b  BST_FOCUS		Specifies the focus state. A nonzero value indicates that the button has the keyboard focus.
/// - \c \b  BST_INDETERMINATE	Indicates the button is grayed because the state of the button
/// is indeterminate. This value applies only if the button has the BS_3STATE or BS_AUTO3STATE style.
/// - \c \b  BST_PUSHED		Specifies the highlight state. A nonzero value indicates that the
/// button is highlighted. A button is automatically highlighted when the user
/// positions the cursor over it and presses and holds the left mouse button. The
/// highlighting is removed when the user releases the mouse button.
/// - \c \b  BST_UNCHECKED	Indicates the button is unchecked. Same as the Windows NT return
/// value of zero.
//
inline uint TButton::GetState() const
{
  PRECONDITION(GetHandle());
  return (uint)CONST_CAST(TButton*, this)->SendMessage(BM_GETSTATE);
}

//
/// Change the style of a button. If redraw is true  the button is redrawn. The
/// style parameter can be one of the following:
/// 
/// - \c \b  BS_3STATE	Creates a button that is the same as a check box, except that the box
/// can be grayed as well as checked or unchecked. Use the grayed state to show that
/// the state of the check box is not determined.
/// - \c \b  BS_AUTO3STATE	Creates a button that is the same as a three-state check box,
/// except that the box changes its state when the user selects it. The state cycles
/// through checked, grayed, and unchecked.
/// - \c \b  BS_AUTOCHECKBOX	Creates a button that is the same as a check box, except that
/// the check state automatically toggles between checked and unchecked each time
/// the user selects the check box.
/// - \c \b  BS_AUTORADIOBUTTON	Creates a button that is the same as a radio button, except
/// that when the user selects it, Windows automatically sets the button's check
/// state to checked and automatically sets the check state for all other buttons in
/// the same group to unchecked.
/// - \c \b  BS_CHECKBOX	Creates a small, empty check box with text. By default, the text is
/// displayed to the right of the check box. To display the text to the left of the
/// check box, combine this flag with the BS_LEFTTEXT style (or with the equivalent
/// BS_RIGHTBUTTON style).
/// - \c \b  BS_DEFPUSHBUTTON	Creates a push button that behaves like a BS_PUSHBUTTON style
/// button, but also has a heavy black border. If the button is in a dialog box, the
/// user can select the button by pressing the ENTER key, even when the button does
/// not have the input focus. This style is useful for enabling the user to quickly
/// select the most likely (default) option.
/// - \c \b  BS_GROUPBOX	Creates a rectangle in which other controls can be grouped. Any text
/// associated with this style is displayed in the rectangle's upper left corner.
/// - \c \b  BS_LEFTTEXT	Places text on the left side of the radio button or check box when
/// combined with a radio button or check box style. Same as the BS_RIGHTBUTTON
/// style.
/// - \c \b  BS_OWNERDRAW	Creates an owner-drawn button. The owner window receives a
/// WM_MEASUREITEM message when the button is created and a WM_DRAWITEM message when
/// a visual aspect of the button has changed. Do not combine the BS_OWNERDRAW style
/// with any other button styles.
/// - \c \b  BS_PUSHBUTTON	Creates a push button that posts a WM_COMMAND message to the owner
/// window when the user selects the button.
/// - \c \b  BS_RADIOBUTTON	Creates a small circle with text. By default, the text is
/// displayed to the right of the circle. To display the text to the left of the
/// circle, combine this flag with the BS_LEFTTEXT style (or with the equivalent
/// BS_RIGHTBUTTON style). Use radio buttons for groups of related, but mutually
/// exclusive choices.
/// - \c \b  BS_USERBUTTON	Obsolete, but provided for compatibility with 16-bit versions of
/// Windows. Win32-based applications should use BS_OWNERDRAW instead.
/// - \c \b  BS_BITMAP	Specifies that the button displays a bitmap.
/// - \c \b  BS_BOTTOM	Places text at the bottom of the button rectangle.
/// - \c \b  BS_CENTER	Centers text horizontally in the button rectangle.
/// - \c \b  BS_ICON	Specifies that the button displays an icon.
/// - \c \b  BS_LEFT	Left-justifies the text in the button rectangle. However, if the button
/// is a check box or radio button that does not have the BS_RIGHTBUTTON style, the
/// text is left justified on the right side of the check box or radio button.
/// - \c \b  BS_MULTILINE	Wraps the button text to multiple lines if the text string is too
/// long to fit on a single line in the button rectangle.
/// - \c \b  BS_NOTIFY	Enables a button to send BN_DBLCLK, BN_KILLFOCUS, and BN_SETFOCUS
/// notification messages to its parent window. Note that buttons send the
/// - \c \b  BN_CLICKED notification message regardless of whether it has this style.
/// - \c \b  BS_PUSHLIKE	Makes a button (such as a check box, three-state check box, or radio
/// button) look and act like a push button. The button looks raised when it isn't
/// pushed or checked, and sunken when it is pushed or checked.
/// - \c \b  BS_RIGHT	Right-justifies text in the button rectangle. However, if the button
/// is a check box or radio button that does not have the BS_RIGHTBUTTON style, the
/// text is right justified on the right side of the check box or radio button.
/// - \c \b  BS_RIGHTBUTTON	Positions a radio button's circle or a check box's square on the
/// right side of the button rectangle. Same as the BS_LEFTTEXT style.
/// - \c \b  BS_TEXT	Specifies that the button displays text.
/// - \c \b  BS_TOP	Places text at the top of the button rectangle.
/// - \c \b  BS_VCENTER	Places text in the middle (vertically) of the button rectangle.
//
inline void TButton::SetStyle(uint style, bool redraw)
{
  PRECONDITION(GetHandle());
  SendMessage(BM_SETSTYLE, style, MkParam2(redraw,0));
}


} // OWL namespace


#endif  // OWL_BUTTON_H


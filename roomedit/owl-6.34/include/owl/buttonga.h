//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TButtonGadget.
//----------------------------------------------------------------------------

#if !defined(OWL_BUTTONGA_H)
#define OWL_BUTTONGA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadget.h>

namespace owl {

class _OWLCLASS TCelArray;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TButtonGadget
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TGadget, TButtonGadget represent buttons that you can click on or
/// off. You can also apply attributes such as color, style, and shape (notched or
/// unnotched) to your button gadgets.
/// 
/// In general, button gadgets are classified as either command or attribute
/// buttons. Attribute buttons include radio buttons (which are considered
/// exclusive), or check boxes (which are nonexclusive). The public data member,
/// TType, enumerates these button types.
/// TButtonGadget contains several functions that let you change the style of a
/// button. Use SetAntialiasEdges to turn antialiasing on and off, SetNotchCorners
/// to control corner notching, and SetShadowStyle to change the style of the button
/// shadow.
/// 
/// TButtonGadget objects respond to mouse events in the following manner:  when a
/// mouse button is pressed, the button is pressed; when the mouse button is
/// released, the button is released. Commands can be entered only when the mouse
/// button is in the "up" state. When the mouse is pressed, TButtonGadget objects
/// capture the mouse and reserve all mouse messages for the current window. When
/// the mouse button is up, button gadgets release the capture for the current
/// window. The public data member, TState, enumerates the three button states.
///
/// Buttons begin highlighting and do a capture when pressed (the mouse down
/// occurs). they cancel highlighting when the mouse exits, but begin
/// highlighting again when the mouse re-enters. when the mouse goes up the
/// capture is released
//
/// There are two basic type of buttons: commands and settings (attribute
/// buttons). Settings can be exclusive (like a radio button) or non-exclusive
/// (like a check box), or SemiExclusive where they act like both
//
/// There are three normal button states: up, down, and indeterminate. in
/// addition the button can be highlighted (pressed) in all three states
//
/// Commands can only be in the "up" state. Settings can be in all three states
//
/// \todo Move NotchCorners to OWL5COMPAT, implement repeating commands
class _OWLCLASS TButtonGadget : public TGadget {
  public:
/// Enumerates the types of button gadgets. An exclusive button is one that works in
/// conjunction with other buttons such that one button is activated at a time.
    enum TType {     ///< Basic type of this button
      Command,       ///< Sends a command when pressed.
      Exclusive,     ///< Stays down when pressed and causes other buttons in the group to pop back up.
      NonExclusive,  ///< Toggles its state when pressed and ignores other buttons.
      SemiExclusive, ///< Same as exclusive, except that it also pops back up if pressed while it is down.
      RepeatCmd,     ///< Auto-repeating command button.
    };

/// TState enumerates the three button positions during which the button can be
/// pressed: up (0), down (1), and an indeterminate state (2). A nonzero value
/// indicates a highlighted button.
    enum TState {    ///< Current state of this button
      Up,            ///< Button is up, i.e. unchecked
      Down,          ///< Button is down, i.e. checked
      Indeterminate, ///< Button is neither checked nor unchecked
    };

#if defined(OWL5_COMPAT)
/// Enumerates button shadow styles--either single (1) or double (2) shadow borders.
    enum TShadowStyle {  ///< Bottom & right side shadow width for old UI style
      SingleShadow = 1,
      DoubleShadow = 2,
    };
#endif

    TButtonGadget(TResId glyphResIdOrIndex,
                  int    id,
                  TType  type = Command,
                  bool   enabled = false, // initial state before cmd enabling
                  TState state = Up,
                  bool   sharedGlyph = false);
     ~TButtonGadget();

    // Some accessors
    //
    void          SetButtonState(TState newState);
    TState        GetButtonState() const;

    TType         GetButtonType() const;

    // A few button style options. These styles may be ignores on some
    // UI platforms
    //
    bool          GetNotchCorners() const;
    void          SetNotchCorners(bool notchCorners=true);

#if defined(OWL5_COMPAT)
    TShadowStyle  GetShadowStyle() const;
    void          SetShadowStyle(TShadowStyle style=DoubleShadow);
#endif

    bool          GetAntialiasEdges() const;
    void          SetAntialiasEdges(bool anti=true);

    // TGadget overrides
    //
    virtual void  GetDesiredSize(TSize& size);
    virtual void  SetBounds(const TRect& r);
    virtual void  CommandEnable();
    virtual void  SysColorChange();

  protected:

    // TGadget overrides
    //
    virtual void  Paint(TDC& dc);
    virtual void  PaintBorder(TDC& dc);
    virtual void  MouseEnter(uint modKeys, const TPoint& p);
    virtual void  MouseMove(uint modKeys, const TPoint& p);
    virtual void  MouseLeave(uint modKeys, const TPoint& p);
    virtual void  LButtonDown(uint modKeys, const TPoint& p);
    virtual void  LButtonUp(uint modKeys, const TPoint& p);

    /// Glyph types and construction functions
    //
		/// Contains values that allow a glyph button to display different glyphs when its
		/// state changes. For example, the CelPressed constant could be used to toggle a
		/// glyph button between a happy face and a sad face when it is pressed.
		/// \note Unused at the moment.
		/// \todo Implement the functionality to allow a glyph button to display different glyphs when its
		/// state changes
    //
    enum {
      //CelMask,
      CelNormal,    ///< Displayed under normal circumstances.
      CelDisabled,  ///< Displayed when the button is disabled (grayed).
      CelIndeterm,  ///< Displayed when an ambiguous or indeterminate state is encountered.
      CelDown,      ///< Displayed when the button is down or checked.
      CelPressed,   ///< Displayed when the button is pressed.
      CelsTotal,
    };

    // New virtuals
    //
    virtual void  PaintFace(TDC& dc, const TRect& rect);
    virtual TDib* GetGlyphDib();
    virtual void  ReleaseGlyphDib(TDib* glyph);
    virtual void  BuildCelArray();

    // Button protocol
    // ~~~~~~ ~~~~~~~~

    // Invoked by mouse-down & mouse enter events. sets member data "Pressed"
    // to true and highlights the button
    //
    virtual void  BeginPressed(const TPoint& p);

    // Invoked by mouse exit events. sets member data "Pressed" to false and
    // paints the button in its current state.
    // Sets gadget InMouse member to mstate.
    //
    virtual void  CancelPressed(const TPoint& p, bool mstate=false);

    // The action method called on a completed 'click', generates WM_COMMAND
    //
    virtual void  Activate(const TPoint& p);

  protected_data:
// !CQ some of these need accessors of some sort...
    TResId        ResId;            ///< Holds the resource ID for this button gadget's bitmap.
    TCelArray*    CelArray;         ///< CelArray used to cache glyph states

    TPoint        BitmapOrigin;     ///< Points to the x and y coordinates of the bitmap used for this button gadget.
    TState        State         :4; ///< Current state of button
    TType         Type          :4; ///< Type of this button
#if defined(OWL5_COMPAT)
    TShadowStyle  ShadowStyle   :4; ///< Shadow style, may be ignored w/ 3dUI
#endif
// !CQ    bool          Repeat        :1; // Does this button auto-repeat
    bool          NotchCorners  :1; ///< Notch (round) corners? ignored w/ 3dUI
    bool          Pressed       :1; ///< Initialized to false, Pressed is true if the button is pushed or false if it is released.
    bool          AntialiasEdges:1; ///< Should border edges be antialiased?
    bool          SharingGlyph;     ///< Should the button share glyphs with its gadget window?
    int           GlyphIndex;       ///< Base index for our glyph bitmap

  private:
    int           GlyphCount;       ///< How many glyphs are we using (1,3,4,5)

  protected:
    void          SetButtonType(TType newType);
    void          SetGlyphIndex(int index);
    bool          IsPressed() const;
    void          CheckExclusively();
    TResId        GetResId() const;

    TCelArray*    GetCelArray();
    const TCelArray* GetCelArray() const;

    TPoint&       GetBitmapOrigin(); 
    const TPoint& GetBitmapOrigin() const; 
    void SetBitmapOrigin(const TPoint& bitmapOrigin);

  private:
    friend class TBarDescr;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the state of the button. 
//
inline TButtonGadget::TState TButtonGadget::GetButtonState() const {
  return State;
}

//
/// Return the type of the button.
//
inline TButtonGadget::TType TButtonGadget::GetButtonType() const {
  return Type;
}

//
/// Returns true if the button has rounded corners. This style may be ignores on
/// some UI platforms.
//
inline bool TButtonGadget::GetNotchCorners() const {
  return NotchCorners;
}

//
/// By default, SetNotchCorners implements notched corners for buttons. To repaint
/// the frame of the button if the window has already been created, call
/// InvalidateRect with the Bounds rectangle. This style may be ignores on some UI
/// platforms.
//
inline void TButtonGadget::SetNotchCorners(bool notchCorners) {
  NotchCorners = notchCorners;
}

//
/// Return the current shadow style of the button.
//
#if defined(OWL5_COMPAT)
inline TButtonGadget::TShadowStyle TButtonGadget::GetShadowStyle() const {
  return ShadowStyle;
}

/// Sets the button style to a shadow style which, by default, is DoubleShadow. Sets
/// the left and top borders to 2 and the right and bottom borders to ShadowStyle +
/// 1.
/// \note This is obsolete under modern versions of Windows
inline void TButtonGadget::SetShadowStyle(TShadowStyle) {
}
#endif

//
/// Returns true if the border edges are antialiased.
//
inline bool TButtonGadget::GetAntialiasEdges() const {
  return AntialiasEdges;
}

//
/// Turns the antialiasing of the button bevels on or off. By default, antialiasing
/// is on.
//
inline void TButtonGadget::SetAntialiasEdges(bool anti) {
  AntialiasEdges=anti;
}

//
/// Sets the type of the button.
//
inline void TButtonGadget::SetButtonType(TType newType) 
{
  Type = newType;
}

/// Returns the resource ID for this button gadget's bitmap.
inline TResId TButtonGadget::GetResId() const 
{
  return ResId;
}

/// Returns the CelArray used to cache glyph states
inline TCelArray* TButtonGadget::GetCelArray()
{
	return CelArray;	
}

/// Returns the CelArray used to cache glyph states
inline const TCelArray* TButtonGadget::GetCelArray() const
{
	return CelArray;	
}

/// Returns the x and y coordinates of the bitmap used for this button gadget.
inline TPoint& TButtonGadget::GetBitmapOrigin() 
{
	return BitmapOrigin;
}

/// Returns the x and y coordinates of the bitmap used for this button gadget.
inline const TPoint& TButtonGadget::GetBitmapOrigin() const
{
	return BitmapOrigin;
}

/// Sets the x and y coordinates of the bitmap used for this button gadget.
inline void TButtonGadget::SetBitmapOrigin(const TPoint& bitmapOrigin)
{
	BitmapOrigin = bitmapOrigin;
}

/// Returns true if the button is pushed or false if it is released
inline bool TButtonGadget::IsPressed() const
{
	return Pressed;
}


} // OWL namespace


#endif  // OWL_BUTTONGA_H

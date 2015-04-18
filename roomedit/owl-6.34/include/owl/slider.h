//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes TSlider & TVSlider.  This defines the basic behavior
/// of slider controls.
//----------------------------------------------------------------------------

#if !defined(OWL_SLIDER_H)
#define OWL_SLIDER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/scrollba.h>
#include <owl/color.h>
#include <owl/commctrl.h>
#include <owl/slider.rh>

namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TDC;
class _OWLCLASS TRegion;
class _OWLCLASS TBrush;

//
/// \class TSlider
/// An abstract base class derived from TScrollBar, TSlider defines the basic
/// behavior of sliders (controls that are used for providing nonscrolling, position
/// information). Like scroll bars, sliders have minimum and maximum positions as
/// well as line and page magnitude. Sliders can be moved using either the mouse or
/// the keyboard. If you use a mouse to move the slider, you can drag the thumb
/// position, click on the slot on either side of the thumb position to move the
/// thumb by a specified amount (PageMagnitude), or click on the ruler to position
/// the thumb at a specific spot on the slider. The keyboard's Home and End keys
/// move the thumb position to the minimum (Min) and maximum (Max) positions on the
/// slider.
/// 
/// You can use TSlider 's member functions to cause the thumb positions to
/// automatically align with the nearest tick positions. (This is referred to as
/// snapping.) You can also specify the tick gaps (the space between the lines that
/// separate the major divisions of the X- or Y-axis).
/// 
/// The sample program SLIDER.CPP on BC5.0x distribution disk displays a
/// thermostat that uses sliders:
/// \image html bm257.BMP
//
class _OWLCLASS TSlider 
  : public TScrollBar 
{
  public:

    TSlider(TWindow* parent, int id, int x, int y, int w, int h, TResId thumbResId, TModule* module = 0);
    TSlider(TWindow* parent, int resId, TResId thumbResId, TModule* module = 0);
    TSlider(THandle hWnd, TModule* module = 0);

   ~TSlider();

    /// \name Overload TScrollBar virtual functions
    /// @{
    void GetRange(int& minValue, int& maxValue) const;
    int GetPosition() const;
    void SetRange(int minValue, int maxValue, bool redraw = true);
    void SetPosition(int thumbPos);
    void SetPosition(int thumbPos, bool redraw);
    /// @}

    /// \name New settings introduced by sliders 
    /// @{
    void SetRuler(int ticGap, bool snap = false);
    void SetRuler(int tics[], int ticCount, bool snap = false);
    void SetSel(int start, int end, bool redraw = true);
    void GetSel(int& start, int& end);
    /// @}

    /// \name New messages (version 4.70)
    /// @{
    HWND GetBuddy(bool loc=true) const;
    void SetBuddy(HWND buddy, bool loc=true);
    HWND GetTooltips() const;
    void SetTooltips(HWND tooltip);
    void SetTipSide(int loc);
    /// @}

  protected:

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    virtual void SetupWindow();

    int SnapPos(int pos);

    /// \name Event handlers
    /// @{
    void EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    void EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    /// @}

  protected_data:

    int Min; ///< Minimum position value
    int Max; ///< Maximum position value
    uint Range; ///< Positive range
    int Pos; ///< Current position value
    TResId ThumbResId; ///< Bitmap res id for thumb knob
    TRect ThumbRect; ///< Bounding rect of Thumb in pixels
    TRegion* ThumbRgn; ///< Optional clipping/hit test region
    TRect CaretRect; ///< Bounding rect of Thumb's blink caret
    int SlotThick; ///< Thickness(width or height) of slot
    int TicGap; ///< Gap between tics in pos units
    int* Tics; ///< Array of specific tics
    int TicCount; ///< Size of the array of specific tics
    bool Snap; ///< Snap Thumb to tics
    int SelStart; ///< Selection start & end
    int SelEnd;

    bool Sliding; ///< True if the thumb is sliding. Otherwise, false. 
    TColor BkColor; ///< Color to use to paint background

  DECLARE_RESPONSE_TABLE(TSlider);
  DECLARE_ABSTRACT_STREAMABLE_OWL(TSlider, 2);
};

DECLARE_STREAMABLE_INLINES(owl::TSlider);

//
/// \class THSlider
/// Derived from TSlider, THSlider provides implementation details for horizontal sliders.
/// The sample program SLIDER.CPP on BC5.0x distribution disk displays a
/// thermostat that uses a horizontal slider.
//
class _OWLCLASS THSlider 
  : public TSlider 
{
  public:

    THSlider(TWindow* parent, int id, int x, int y, int w, int h, TResId thumbResId = IDB_HSLIDERTHUMB, TModule* = 0);
    THSlider(TWindow* parent, int resId, TResId thumbResId = IDB_HSLIDERTHUMB, TModule* = 0);
    THSlider(THandle hWnd, TModule* = 0);

  private:

    // Hidden to prevent accidental copying or assignment
    //
    THSlider(const THSlider&);
    THSlider& operator =(const THSlider&);

  DECLARE_STREAMABLE_OWL(THSlider, 1);
};

DECLARE_STREAMABLE_INLINES( owl::THSlider );

//
/// \class TVSlider
/// Derived from TSlider, TVSlider provides implementation details for vertical
/// sliders. See TSlider for an illustration of a vertical slider.
//
class _OWLCLASS TVSlider 
  : public TSlider 
{
  public:

    TVSlider(TWindow* parent, int id, int x, int y, int w, int h, TResId thumbResId = IDB_VSLIDERTHUMB, TModule* = 0);
    TVSlider(TWindow* parent, int resId, TResId thumbResId = IDB_VSLIDERTHUMB, TModule* = 0);
    TVSlider(THandle hWnd, TModule* = 0);

  protected:

  private:
    TVSlider(const TVSlider&);
    TVSlider& operator =(const TVSlider&);

  DECLARE_STREAMABLE_OWL(TVSlider, 1);
};

DECLARE_STREAMABLE_INLINES(owl::TVSlider);

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the end values of the present range of slider thumb positions in minValue and
/// maxValue. Overloads TScrollBar's virtual function.
inline void TSlider::GetRange(int& minValue, int& maxValue) const
{
  minValue = Min; maxValue = Max;
}

//
/// Returns the slider's current thumb position. Overloads TScrollBar's virtual
/// function.
inline int TSlider::GetPosition() const
{
  return Pos;
}


//
// Version 4.70
//
inline HWND TSlider::GetBuddy(bool loc) const
{
  return HWND(CONST_CAST(TSlider*,this)->SendMessage(TBM_GETBUDDY,TParam1(loc)));
}

//
// Version 4.70
//
inline void  TSlider::SetBuddy(HWND buddy, bool loc)
{
  SendMessage(TBM_SETBUDDY, TParam1(loc), TParam2(buddy));
}

//
// Version 4.70
//
inline HWND TSlider::GetTooltips() const
{
  return HWND(CONST_CAST(TSlider*,this)->SendMessage(TBM_GETTOOLTIPS));
}

//
// Version 4.70
//
inline void  TSlider::SetTooltips(HWND tooltip)
{
  SendMessage(TBM_SETTOOLTIPS, TParam1(tooltip));
}

//
// Version 4.70
//
inline void  TSlider::SetTipSide(int loc)
{
  SendMessage(TBM_SETTIPSIDE, loc);
}

} // OWL namespace


#endif  // OWL_SLIDER_H

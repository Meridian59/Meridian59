//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TScroller.
//----------------------------------------------------------------------------

#if !defined(OWL_SCROLLER_H)
#define OWL_SCROLLER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/objstrm.h>
#include <owl/window.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

#if !defined(OWL_GEOMETRY_H)
 class _OWLCLASS TRect;
#endif
class _OWLCLASS TDC;

//
/// \class TScroller
// ~~~~~ ~~~~~~~~~
/// Class TScroller implements the actual scroller object. All functions
/// are inline or virtual to avoid pulling in code when no scrollers have
/// been constructed.
//
/// TScroller supports an automatic window-scrolling mechanism (referred to as
/// autoscrolling) that works in conjunction with horizontal or vertical window
/// scroll bars. (It also works if there are no scroll bars.) When autoscrolling is
/// activated, the window automatically scrolls when the mouse is dragged from
/// inside the client area of the window to outside that area. If the AutoMode data
/// member is true, TScroller performs autoscrolling.
/// 
/// To use TScroller, set the Scroller member of your TWindow descendant to a
/// TScroller object instantiated in the constructor of your TWindow descendant.
//
class _OWLCLASS TScroller : public TStreamableBase {
  public:
    TScroller(TWindow* window,
              int      xUnit,  int  yUnit,
              long     xRange, long yRange);
    virtual ~TScroller();

    virtual void  SetWindow(TWindow* win);
    virtual void  SetUnits(int xUnit, int yUnit);
    virtual void  SetPageSize();
    virtual void  SetSBarRange();
    virtual void  SetRange(long xRange, long yRange);
    virtual void  SetTotalRangeOfUnits(long xTotalUnits, long yTotalUnits);

    virtual void  BeginView(TDC& dc, TRect& rect);
    virtual void  EndView();
    virtual void  VScroll(uint scrollEvent, int thumbPos);
    virtual void  HScroll(uint scrollEvent, int thumbPos);
    virtual void  ScrollTo(long x, long y);

    virtual bool   EnableScrollBar(uint flags=SB_BOTH, uint arrow=ESB_ENABLE_BOTH);
    virtual void   ShowScrollBar(int code, bool show=true);

    /// Scrolls to a position calculated using the passed delta values
    //
    void          ScrollBy(long dx, long dy);

    virtual bool  IsAutoMode();
    virtual void  AutoScroll();

    /// Returns a bool value indicating whether or not the passed
    /// area (in units) is currently visible
    //
    bool          IsVisibleRect(long x, long y, int xExt, int yExt);

  protected:
    virtual void  SetScrollPage(int bar, int page, bool redraw = true);
    virtual int   GetScrollPage(int bar) const;
    virtual void  GetScrollRange(int bar, int& minPos, int& maxPos) const;
    virtual void  SetScrollRange(int  bar, int  minPos, int  maxPos, bool redraw = true);
    virtual int   GetScrollPos(int bar) const;
    virtual int   SetScrollPos(int bar, int pos, bool redraw = true);
    virtual int GetScrollTrackPos(int bar) const;

  public:
    TWindow*  Window;         ///< Points to the window whose client area scroller is to be managed.
    long      XPos;           ///< Specifies the current position of the rectangle in horizontal scroll units.
    long      YPos;           ///< Specifies the current position of the rectangle in vertical scroll units.
    long      XRange;         ///< Specifies the number of horizontal scroll units.
    long      YRange;         ///< Specifies the number of vertical scroll units.
    long      XTotalUnits;    ///< Total number of horizontal scroll units
    long      YTotalUnits;    ///< Total number of vertical scroll units
    int       XUnit;          ///< Specifies the amount (in logical device units) to scroll the rectangle in the horizontal (X) direction. 
                              ///< The rectangle is scrolled right if XUnit is positive and left if XUnit is negative.
    int       YUnit;          ///< Specifies the amount (in logical device units) to scroll the rectangle in the vertical (Y) direction.
                              ///< The rectangle is scrolled down if YUnit is positive and up if YUnit is negative. 
    int       XLine;          ///< Specifies the number of logical device units per line to scroll the rectangle in the horizontal (X) direction.
    int       YLine;          ///< Specifies the number of logical device units per line to scroll the rectangle in the vertical (Y) direction.
    int       XPage;          ///< Specifies the number of logical device units per page to scroll the rectangle in the horizontal (X) direction.
    int       YPage;          ///< Specifies the number of logical device units per page to scroll the rectangle in the vertical (Y) direction.
    bool      AutoMode;       ///< Is true if automatic scrolling is activated.
    bool      TrackMode;      ///< Is true if track scrolling is activated.
    bool      AutoOrg;        ///< Is true if scroller offsets origin.
    bool      HasHScrollBar;	///< Is true if scroller has horizontal scroll.
    bool      HasVScrollBar;  ///< Is true if scroller has vertical scroll.

  //DECLARE_STREAMABLE(_OWLCLASS, owl::TScroller, 1);
  DECLARE_STREAMABLE_OWL(TScroller, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TScroller );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Sets the owning window to win.
inline void TScroller::SetWindow(TWindow* win){
  Window = win;
}

//
/// Scrolls to a position calculated using the passed delta values (dx and dy). A
/// positive delta position moves the thumb position down or right. A negative delta
/// position moves the thumb up or left.
inline void TScroller::ScrollBy(long dx, long dy){
  ScrollTo(XPos+dx, YPos+dy);
}

//
/// Is true if the rectangle (x, y, xExt, and yExt) is visible.
inline bool TScroller::IsVisibleRect(long x, long y, int xExt, int yExt){
  return (x + xExt > XPos) && (y + yExt > YPos) &&
      (x <= XPos + XPage + 1) && (y <= YPos + YPage + 1);
}

//
inline void TScroller::SetScrollPage(int bar, int page, bool redraw){
  Window->SetScrollPage(bar, page, redraw);
}

//
inline int TScroller::GetScrollPage(int bar) const {
  return Window->GetScrollPage(bar);
}

//
inline void TScroller::GetScrollRange(int bar, int& minPos, int& maxPos) const{
  Window->GetScrollRange(bar,minPos,maxPos);
}

//
inline void TScroller::SetScrollRange(int  bar, int  minPos, int  maxPos, bool redraw){
  Window->SetScrollRange(bar,minPos,maxPos,redraw);
}

//
inline bool TScroller::EnableScrollBar(uint index, uint arrow){
  return Window->EnableScrollBar(index, arrow);
}

//
inline void TScroller::ShowScrollBar(int code, bool show){
  Window->ShowScrollBar(code, show);
}

//
inline int TScroller::GetScrollPos(int bar) const{
  return Window->GetScrollPos(bar);
}

//
inline int TScroller::SetScrollPos(int bar, int pos, bool redraw){
  return Window->SetScrollPos(bar, pos, redraw);
}

//
inline int TScroller::GetScrollTrackPos(int bar) const {
  return Window->GetScrollTrackPos(bar);
}

} // OWL namespace

#endif  // OWL_SCROLLER_H

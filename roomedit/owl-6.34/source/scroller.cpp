//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TScroller.
/// Including bug fixes by Rich Goldstein
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/scroller.h>
#include <owl/window.h>
#include <owl/dc.h>
#include <stdlib.h>

#include <algorithm>

namespace owl {


OWL_DIAGINFO;

//
/// Constructs a TScroller object with window as the owner window, and xUnit, yUnit,
/// xRange, and yRange as xUnit, yUnit, xRange and yRange, respectively. Initializes
/// data members to default values. HasHScrollBar and HasVScrollBar are set
/// according to the scroll bar attributes of the owner window.
//
TScroller::TScroller(TWindow* window,
                     int      xUnit,
                     int      yUnit,
                     long     xRange,
                     long     yRange)
{
  Window = window;
  XPos = YPos = 0;
  XUnit = xUnit;
  YUnit = yUnit;
  XRange = xRange;
  YRange = yRange;
  XTotalUnits = 0;
  YTotalUnits = 0;
  XLine = 1;  YLine = 1;
  XPage = 1;  YPage = 1;
  AutoMode = true;
  TrackMode = true;
  AutoOrg = true;
  HasHScrollBar = ToBool(Window && (Window->GetWindowAttr().Style&WS_HSCROLL));
  HasVScrollBar = ToBool(Window && (Window->GetWindowAttr().Style&WS_VSCROLL));
}

//
/// Destructs a TScroller object. Sets owning window's Scroller number variable to 0.
//
TScroller::~TScroller()
{
  if (Window && Window->GetScroller() == this)
    Window->SetScroller(0);
}

//
/// Sets the XPage and YPage data members (amount by which to scroll on a page
/// scroll request) to the width and height (in XUnits and
/// YUnits) of the owner window's client area.
//
void
TScroller::SetPageSize()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (Window && Window->GetHandle()) {
    TRect  clientRect;
    Window->GetClientRect(clientRect);
    int width = clientRect.Width();
    int height = clientRect.Height();

    if (width && XUnit > 0) {
      XPage = std::max(1, (width+1) / XUnit - 1);
      if (XTotalUnits && XTotalUnits > width) {
        SetScrollPage(SB_HORZ, (width * XRange) / XTotalUnits, true);
      }
      else
        SetScrollPage(SB_HORZ, XPage, false); 
    }

    if (height && YUnit > 0) {
      YPage = std::max(1, (height+1) / YUnit - 1);
      if (YTotalUnits && YTotalUnits > height) {
        SetScrollPage(SB_VERT, (height * YRange) / YTotalUnits, true);
      }
      else
        SetScrollPage(SB_VERT, YPage, false); 
    }
  }
}

//
/// Sets the xRange and yRange of the TScroller to the parameters specified. Then
/// calls SetSBarRange to synchronize the range of the owner window's scroll bars.
//
void
TScroller::SetRange(long xRange, long yRange)
{
  XRange = xRange;
  YRange = yRange;
  SetSBarRange();
  ScrollTo(XPos, YPos);
}

//
// Sets the total units of the window
//
void
TScroller::SetTotalRangeOfUnits(long xTotalUnits, long yTotalUnits)
{
  XTotalUnits = xTotalUnits;
  YTotalUnits = yTotalUnits;
  SetSBarRange();
  ScrollTo(XPos, YPos);
}

//
/// Sets the XUnit and YUnit data members to TheXUnit and TheYUnit, respectively.
/// Updates XPage and YPage by calling SetPageSize.
//
void
TScroller::SetUnits(int xUnit, int yUnit)
{
  XUnit = xUnit;
  YUnit = yUnit;
  SetPageSize();
}

//
/// Sets the range of the owner window's scroll bars to match the range of the
/// TScroller and repaints as necessary
//
void
TScroller::SetSBarRange()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (Window && Window->GetHandle()) {
    if (HasHScrollBar) {
      int curMin, curMax;
      GetScrollRange(SB_HORZ, curMin, curMax);
      int newMax = std::max(0, std::min(int(XRange - 1), (int)INT_MAX));
      if (newMax != curMax || curMin != 0)
        SetScrollRange(SB_HORZ, 0, newMax, true);
    }

    if (HasVScrollBar) {
      int curMin, curMax;
      GetScrollRange(SB_VERT, curMin, curMax);
      int newMax = std::max(0, std::min(int(YRange - 1), (int)INT_MAX));
      if (newMax != curMax || curMin != 0)
        SetScrollRange(SB_VERT, 0, newMax, true);
    }
  }
}

//
/// If TScroller_AutoOrg is true (default condition), BeginView automatically
/// offsets the origin of the logical coordinates of the client area by XPos, YPos
/// during a paint operation. If AutoOrg is false (for example, when the scroller is
/// larger than 32,767 units), you must set the offset manually.
//
void
TScroller::BeginView(TDC& dc, TRect& rect)
{
  long  xOrg = XPos * XUnit;
  long  yOrg = YPos * YUnit;

  if (AutoOrg && xOrg <= INT_MAX && yOrg <= INT_MAX) {
    TPoint offset(int(-xOrg), int(-yOrg));
    dc.SetViewportOrg(offset);
    rect -= offset;
  }
}

//
/// Updates the position of the owner window's scroll bars to be coordinated with
/// the position of the TScroller.
//
void
TScroller::EndView()
{
  if (Window) {
    if (HasHScrollBar) {
      int newPos = XPos;
      if (newPos != GetScrollPos(SB_HORZ))
        SetScrollPos(SB_HORZ, newPos, true);
    }

    if (HasVScrollBar) {
      int newPos = YPos;
      if (newPos != GetScrollPos(SB_VERT))
        SetScrollPos(SB_VERT, newPos, true);
    }
  }
}

//
/// Responds to the specified vertical scrollEvent by calling ScrollBy or ScrollTo.
/// The type of scroll event is identified by the corresponding SB_ constants.
/// The 16-bit thumbPos argument is ignored. The function retrieves the 32-bit value 
/// directly from the scroll bar instead.
//
void
TScroller::VScroll(uint scrollEvent, int)
{
  switch (scrollEvent) {
    case SB_LINEDOWN:
      ScrollBy(0, YLine);
      break;

    case SB_LINEUP:
      ScrollBy(0, -YLine);
      break;

    case SB_PAGEDOWN:
      ScrollBy(0, YPage);
      break;

    case SB_PAGEUP:
      ScrollBy(0, -YPage);
      break;

    case SB_TOP:
      ScrollTo(XPos, 0);
      break;

    case SB_BOTTOM:
      ScrollTo(XPos, YRange);
      break;

    case SB_THUMBPOSITION:
      ScrollTo(XPos, GetScrollTrackPos(SB_VERT));
      break;

    case SB_THUMBTRACK:
    {
      int thumbPos32 = GetScrollTrackPos(SB_VERT);
      if (TrackMode)
        ScrollTo(XPos, thumbPos32);
      if (Window && HasVScrollBar)
        SetScrollPos(SB_VERT, thumbPos32, true);
      break;
    }
  }
}

//
/// Responds to the specified horizontal scrollEvent by calling ScrollBy or ScrollTo. 
/// The type of scroll event is identified by the corresponding SB_ constants. 
/// The 16-bit thumbPos argument is ignored. The function retrieves the 32-bit value 
/// directly from the scroll bar instead.
//
void
TScroller::HScroll(uint scrollEvent, int)
{
  switch (scrollEvent) {
    case SB_LINEDOWN:
      ScrollBy(XLine, 0);
      break;

    case SB_LINEUP:
      ScrollBy(-XLine, 0);
      break;

    case SB_PAGEDOWN:
      ScrollBy(XPage, 0);
      break;

    case SB_PAGEUP:
      ScrollBy(-XPage, 0);
      break;

    case SB_TOP:
      ScrollTo(0, YPos);
      break;

    case SB_BOTTOM:
      ScrollTo(XRange, YPos);
      break;

    case SB_THUMBPOSITION:
      ScrollTo(GetScrollTrackPos(SB_HORZ), YPos);
      break;

    case SB_THUMBTRACK:
    {
      int thumbPos32 = GetScrollTrackPos(SB_HORZ);
      if (TrackMode)
        ScrollTo(thumbPos32, YPos);
      if (Window && HasHScrollBar)
        SetScrollPos(SB_HORZ, thumbPos32, true);
      break;
    }
  }
}

//
/// Scrolls the rectangle to the position specified in x and y after checking boundary conditions.
/// Causes a WM_PAINT message to be sent
///
/// First scrolls the contents of the client area, if a portion of the client
/// area will remain visible
//
void
TScroller::ScrollTo(long x, long y)
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (Window && Window->GetHandle()) {
    long  newXPos = std::max(0L, std::min(x, long(XRange - XPage)));
    long  newYPos = std::max(0L, std::min(y, long(YRange - YPage)));

    if (newXPos != XPos || newYPos != YPos) {
      //
      // scaling isn't needed here. if condition is met, ScrollWindow()
      // will succeed since XPage and YPage are ints
      //
      // if condition isn't met, ScrollWindow() is called in EndScroll()
      // as a result of calling UpdateWindow()
      //
      // EndScroll() performs the necessary scaling
      //
      if (AutoOrg || abs(YPos-newYPos) < YPage && abs(XPos-newXPos) < XPage)
        Window->ScrollWindow((int)(XPos - newXPos) * XUnit,
                (int)(YPos - newYPos) * YUnit, 0, 0);

      else
        Window->Invalidate();

      XPos = newXPos;
      YPos = newYPos;
      Window->UpdateWindow();
    }
  }
}

//
/// IsAutoMode is true if automatic scrolling is activated.
//
bool
TScroller::IsAutoMode()
{
  return AutoMode;
}

//
/// Performs "auto-scrolling" (dragging the mouse from within the client
/// client area of the Window to without results in auto-scrolling when
/// the AutoMode data member of the Scroller is true)
///
/// Scrolls the owner window's display in response to the mouse being dragged from
/// inside to outside the window. The direction and the amount by which the display
/// is scrolled depend on the current position of the mouse.
//
void
TScroller::AutoScroll()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (AutoMode && Window) {
    TRect  clientRect;
    TPoint cursorPos;
    long  dx = 0, dy = 0;

    GetCursorPos(&cursorPos);
    Window->ScreenToClient(cursorPos);
    Window->GetClientRect(clientRect);

    if (cursorPos.y < 0)
      dy = std::min((long)-YLine, std::max((long)-YPage, long(cursorPos.y / 10 * YLine)));

    else if (cursorPos.y > clientRect.bottom)
      dy = std::max((long)YLine, std::min((long)YPage, long((cursorPos.y-clientRect.bottom)/10 * YLine)));

    if (cursorPos.x < 0)
      dx = std::min((long)-XLine, std::max((long)-XPage, long(cursorPos.x / 10 * XLine)));

    else if (cursorPos.x > clientRect.right)
      dx = std::max((long)XLine, std::min((long)XPage, (cursorPos.x-clientRect.right)/10 * XLine));

    ScrollBy(dx, dy);
  }
}


IMPLEMENT_STREAMABLE(TScroller);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TScroller from the passed ipstream
//
void*
TScroller::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TScroller* o = GetObject();
  is >> o->XPos >> o->YPos >>
      o->XUnit >> o->YUnit >>
      o->XRange >> o->YRange >>
      o->XLine >> o->YLine >>
      o->XPage >> o->YPage >>
      o->AutoMode >> o->TrackMode >>
      o->AutoOrg >>
      o->HasHScrollBar >> o->HasVScrollBar;

  o->Window = 0;
  return o;
}

//
// writes the TScroller to the passed opstream
//
void
TScroller::Streamer::Write(opstream& os) const
{
  TScroller* o = GetObject();
  os << o->XPos << o->YPos <<
      o->XUnit << o->YUnit <<
      o->XRange << o->YRange <<
      o->XLine << o->YLine <<
      o->XPage << o->YPage <<
      o->AutoMode << o->TrackMode <<
      o->AutoOrg <<
      o->HasHScrollBar << o->HasVScrollBar;
}


#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

/* ========================================================================== */


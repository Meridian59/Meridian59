//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TScrollBar.  This defines the basic behavior of all
/// scrollbar controls.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/scrollba.h>
#include <owl/uimetric.h>

namespace owl {

OWL_DIAGINFO;

DEFINE_RESPONSE_TABLE1(TScrollBar, TControl)
  EV_WM_VSCROLL,
  EV_WM_HSCROLL,
END_RESPONSE_TABLE;

//
/// Constructs and initializes a TScrollBar object with the given parent window
/// (parent), a control ID (id), a position (x, y), and a width and height (w, h).
/// Invokes the TControl constructor with similar parameters. If isHScrollBar is
/// true, the constructor adds SBS_HORZ to the window style. If it is false, the
/// constructor adds SBS_VERT. If the supplied height for a horizontal scroll bar or
/// the supplied width for a vertical scroll bar is 0, a standard system metric value is used.
/// LineMagnitude is initialized to 1 and PageMagnitude is set to 10.
//
TScrollBar::TScrollBar(TWindow*        parent,
                       int             id,
                       int x, int y, int w, int h,
                       bool            isHScrollBar,
                       TModule*        module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  LineMagnitude = 1;
  PageMagnitude = 10;

  if (isHScrollBar) {
    Attr.Style |= SBS_HORZ;

    if (Attr.H == 0)
      Attr.H = TUIMetric::CyHScroll;
  }
  else {
    Attr.Style |= SBS_VERT;

    if (Attr.W == 0)
      Attr.W = TUIMetric::CxVScroll;
  }
}

//
/// Constructs a TScrollBar object to be associated with a scroll bar control of a
/// TDialog object. Invokes the TControl constructor with identical parameters.
/// The resourceId parameter must correspond to a scroll bar resource that you
/// define.
///
/// The LineMagnitude is set to 1 by default.
/// The PageMagnitude is set to 10 by default.
//
TScrollBar::TScrollBar(TWindow*   parent,
                       int        resourceId,
                       TModule*   module)
:
  TControl(parent, resourceId, module)
{
  LineMagnitude = 1;
  PageMagnitude = 10;
}

//
/// Constructs a scroll bar object to encapsulate (alias) an existing control.
//
TScrollBar::TScrollBar(THandle hWnd, TModule* module)
:
  TControl(hWnd, module),
  LineMagnitude(1)
{
  SCROLLINFO info;
  ZeroMemory(&info, sizeof info);
  info.cbSize = sizeof info;
  info.fMask = SIF_PAGE;
  GetScrollInfo(&info);
  PageMagnitude = info.nPage;
}

//
/// Transfers scroll-bar data to or from the transfer buffer pointed to by buffer,
/// which is expected to point to a TScrollBarData structure.
/// Data is transferred to or from the transfer buffer if tdGetData or tdSetData is
/// supplied as the direction.
/// Transfer always returns the size of the transfer data (the size of the
/// TScrollBarData structure). To retrieve the size of this data without
/// transferring data, pass tdSizeData as the direction.
//
uint
TScrollBar::Transfer(void* buffer, TTransferDirection direction)
{
  TScrollBarData* scrollBuff = (TScrollBarData*)buffer;

  if (direction == tdGetData) {
    GetRange(scrollBuff->LowValue, scrollBuff->HighValue);
    scrollBuff->Position = GetPosition();
  }
  else if (direction == tdSetData) {
    SetRange(scrollBuff->LowValue, scrollBuff->HighValue);
    SetPosition(scrollBuff->Position);
  }

  return sizeof(TScrollBarData);
}

//
/// Returns the name of TScrollBar's registration class, "SCROLLBAR".
//
TWindow::TGetClassNameReturnType
TScrollBar::GetClassName()
{
  return _T("SCROLLBAR");
}

//
/// Sets the scroll bar's range to 0, 100. To redefine this range, call SetRange.
//
void
TScrollBar::SetupWindow()
{
  SetRange(0, 100);
  TControl::SetupWindow();
}

//
/// Sets scrollbar info
//
void
TScrollBar::SetScrollInfo(SCROLLINFO* info, bool redraw)
{
  PRECONDITION(GetHandle());
  ::SetScrollInfo(GetHandle(), SB_CTL, info, redraw);
}

//
/// Retrieves the scroll info.
//
void
TScrollBar::GetScrollInfo(SCROLLINFO* info) const
{
  PRECONDITION(GetHandle());
  ::GetScrollInfo(GetHandle(), SB_CTL, info);
}

//
/// Returns the scroll bar's current thumb position.
//
int
TScrollBar::GetPosition() const
{
  PRECONDITION(GetHandle());
  SCROLLINFO info;
  ZeroMemory(&info, sizeof info);
  info.cbSize = sizeof info;
  info.fMask = SIF_POS;
  GetScrollInfo(&info);
  return info.nPos;
}

//
/// Moves the thumb to the position specified in thumbPos. If thumbPos is outside
/// the present range of the scroll bar, the thumb is moved to the closest position
/// within range.
//
void
TScrollBar::SetPosition(int thumbPos, bool redraw)
{
  PRECONDITION(GetHandle());
  int  minValue, maxValue;
  GetRange(minValue, maxValue);

  // Constrain "thumbPos" to be in the range "minValue .. maxValue"
  //
  if (thumbPos > maxValue)
    thumbPos = maxValue;

  else if (thumbPos < minValue)
    thumbPos = minValue;

  if (thumbPos != GetPosition()) {
    SCROLLINFO info;
    ZeroMemory(&info, sizeof info);
    info.cbSize = sizeof info;
    info.fMask = SIF_POS;
    info.nPos = thumbPos;
    SetScrollInfo(&info, redraw);
  }
}

//
/// Returns the current delta to move the thumb when page up/page down is received.
//
int
TScrollBar::GetPageMagnitude() const
{
  return PageMagnitude;
}

//
/// Sets the delta to move the thumb when page up/page down is received.
//
void
TScrollBar::SetPageMagnitude(int pagemagnitude)
{
  PageMagnitude = pagemagnitude;
  PRECONDITION(GetHandle());
  SCROLLINFO info;
  ZeroMemory(&info, sizeof info);
  info.cbSize = sizeof info;
  info.fMask = SIF_PAGE;
  info.nPage = pagemagnitude;
  SetScrollInfo(&info);
}

//
/// Returns the end values of the present range of scroll bar thumb positions in minValue and maxValue.
//
void
TScrollBar::GetRange(int& minValue, int& maxValue) const
{
  PRECONDITION(GetHandle());
  SCROLLINFO info;
  ZeroMemory(&info, sizeof info);
  info.cbSize = sizeof info;
  info.fMask = SIF_RANGE;
  GetScrollInfo(&info);
  minValue = info.nMin;
  maxValue = info.nMax;
}

//
/// Sets the scroll bar to the range between min and max.
//
void
TScrollBar::SetRange(int minValue, int maxValue, bool redraw)
{
  PRECONDITION(GetHandle());
  SCROLLINFO info;
  ZeroMemory(&info, sizeof info);
  info.cbSize = sizeof info;
  info.fMask = SIF_RANGE;
  info.nMin = minValue;
  info.nMax = maxValue;
  SetScrollInfo(&info,redraw);
}


//
/// Calls SetPosition to change the scroll bar's thumb position by the value
/// supplied in delta. A positive delta moves the thumb down or right. A negative
/// delta value moves the thumb up or left. DeltaPos returns the new thumb position.
//
int
TScrollBar::DeltaPos(int delta)
{
  if (delta != 0)
    SetPosition(GetPosition() + delta);

  return GetPosition();
}

//
/// Calls SetPosition to move the thumb up or left (by LineMagnitude units).
/// SBLineUp is called to respond to a click on the top or left arrow of the scroll
/// bar.
//
void
TScrollBar::SBLineUp()
{
  DeltaPos(-LineMagnitude);
}

//
/// Calls SetPosition to move the thumb down or right (by LineMagnitude units).
/// SBLineDown is called to respond to a click on the bottom or right arrow of the
/// scroll bar.
//
void
TScrollBar::SBLineDown()
{
  DeltaPos(LineMagnitude);
}

//
/// Calls SetPosition to move the thumb up or left (by PageMagnitude units).
/// SBPageUp is called to respond to a click in the top or left scrolling area of
/// the scroll bar.
//
void
TScrollBar::SBPageUp()
{
  DeltaPos(-PageMagnitude);
}

//
/// Calls SetPosition to move the thumb down or right (by PageMagnitude units).
/// SBPageDown is called to respond to a click in the bottom or right scrolling area
/// of the scroll bar.
//
void
TScrollBar::SBPageDown()
{
  DeltaPos(PageMagnitude);
}

//
/// Calls SetPosition to move the thumb. SBThumbPosition is called to respond when
/// the thumb is set to a new position.
//
void
TScrollBar::SBThumbPosition(int thumbPos)
{
  SetPosition(thumbPos);
}

//
/// Calls SetPosition to move the thumb as it is being dragged to a new position.
//
void
TScrollBar::SBThumbTrack(int thumbPos)
{
  SetPosition(thumbPos);
}

//
/// Calls SetPosition to move the thumb to the top or right of the scroll bar. SBTop
/// is called to respond to the thumb's being dragged to the top or rightmost
/// position on the scroll bar.
//
void
TScrollBar::SBTop()
{
  int  minValue, maxValue;
  GetRange(minValue, maxValue);
  SetPosition(minValue);
}

//
/// Calls SetPosition to move the thumb to the bottom or right of the scroll bar.
/// SBBottom is called to respond to the thumb's being dragged to the bottom or
/// rightmost position of the scroll bar.
//
void
TScrollBar::SBBottom()
{
  int  minValue, maxValue;
  GetRange(minValue, maxValue);
  SetPosition(maxValue);
}

//
/// User released the mouse after scrolling.
//
void
TScrollBar::SBEndScroll()
{
}

//
/// Response table handler that calls the virtual function (SBBottom, SBLineDown and
/// so on) in response to messages sent by TWindow::DispatchScroll.
//
void
TScrollBar::EvHScroll(uint scrollCode, uint thumbPos, THandle)
{
  // Note that the passed 'thumbPos' only contains 16-bit position data.
  // See MSDN (http://msdn.microsoft.com/en-us/library/windows/desktop/bb787575.aspx).
  // We call GetScrollPos or GetScrollTrackPos as appropriate to try to get 32-bit data.
  // Also see [bugs:#227](http://sourceforge.net/p/owlnext/bugs/227).
  //
  switch (scrollCode) {
    case SB_LINEDOWN:      SBLineDown(); break;
    case SB_LINEUP:        SBLineUp(); break;
    case SB_PAGEDOWN:      SBPageDown(); break;
    case SB_PAGEUP:        SBPageUp(); break;
    case SB_TOP:           SBTop(); break;
    case SB_BOTTOM:        SBBottom(); break;
    case SB_THUMBPOSITION:
    {
      int thumbPos32 = GetScrollPos(SB_CTL); // May fail and return 0.
      int p = (thumbPos32 != 0) ? thumbPos32 : thumbPos;
      SBThumbPosition(p); 
      break;
    }
    case SB_THUMBTRACK:
    {
      int thumbPos32 = GetScrollTrackPos(SB_CTL); // May fail and return 0.
      int p = (thumbPos32 != 0) ? thumbPos32 : thumbPos;
      SBThumbTrack(p); 
      break;
    }
    case SB_ENDSCROLL:     SBEndScroll();
  }
}

//
/// Response table handler that calls the virtual function (SBBottom, SBLineDown and
/// so on) in response to messages sent by TWindow::DispatchScroll.
//
void
TScrollBar::EvVScroll(uint scrollCode, uint thumbPos, THandle hCtl)
{
  // Simply forward to EvHScroll, since the code is identical.
  //
  EvHScroll(scrollCode, thumbPos, hCtl);
}

IMPLEMENT_STREAMABLE1(TScrollBar, TControl);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads an instance of TScrollBar from the passed ipstream.
//
void*
TScrollBar::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  is >> GetObject()->LineMagnitude
     >> GetObject()->PageMagnitude;
  return GetObject();
}

//
// Writes the TScrollBar to the passed opstream.
//
void
TScrollBar::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
  os << GetObject()->LineMagnitude
     << GetObject()->PageMagnitude;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


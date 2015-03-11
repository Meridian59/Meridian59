//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TGadget, TSeparatorGadget and TSizeGripGadget
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/gadget.h>
#include <owl/gadgetwi.h>
#include <owl/framewin.h>
#include <owl/tooltip.h>
#include <owl/uihelper.h>
#include <owl/uimetric.h>
#include <owl/theme.h>

using namespace std;

namespace owl {


OWL_DIAGINFO;

//
// Convert layout units to pixels using a given font height
//
// A layout unit is defined by dividing the font "em" into 8 vertical and 8
// horizontal segments
//
static int
layoutUnitsToPixels(int units, int fontHeight)
{
  const long  unitsPerEM = 8;

  return int((long(units) * fontHeight + unitsPerEM / 2) / unitsPerEM);
}

//
// Return true if the Id is a predefined gadget id.
//
static bool
predefinedGadgetId(int id)
{
  if (id == 0 || id == -1 || (IDG_FIRST <= id && id < IDG_LAST)) {
    return true;
  }
  return false;
}

//
// Retrieve the sizes of the 4 margins in pixels given a font height
//
void
TMargins::GetPixels(int& left, int& right, int& top, int& bottom, int fontHeight) const
{
  switch (Units) {
    case Pixels:
      left = Left;
      top = Top;
      right = Right;
      bottom = Bottom;
      break;

    case LayoutUnits:
      left = layoutUnitsToPixels(Left, fontHeight);
      top = layoutUnitsToPixels(Top, fontHeight);
      right = layoutUnitsToPixels(Right, fontHeight);
      bottom = layoutUnitsToPixels(Bottom, fontHeight);
      break;

    case BorderUnits:
      int  cxBorder = TUIMetric::CxBorder;
      int  cyBorder = TUIMetric::CyBorder;

      left = Left * cxBorder;
      top = Top * cyBorder;
      right = Right * cxBorder;
      bottom = Bottom * cyBorder;
      break;
  }
}

//
/// Construct a gadget with a given id and border style. Used by derived
/// classes.
//
TGadget::TGadget(int id, TBorderStyle borderStyle)
{
  Window = 0;
  Bounds = TRect(0, 0, 0, 0);
  Flags = Enabled | Visible;
  TrackMouse = false;
  MouseInGadget = false;
  Clip = false;
  WideAsPossible = false;
  ShrinkWrapWidth = ShrinkWrapHeight = true;
  Next = 0;
  Id = id;

  SetBorderStyle(borderStyle);
}

//
/// Destroys a TGadget interface object and removes it from its associated window.
//
TGadget::~TGadget()
{
  // If we're in a window, remove ourselves.
  //
  if (Window)
    Window->Remove(*this);
}

//
/// Called during idle time to allow the gadget to perform any idle actions. TGadget
/// performs command enabling on first call in each idle period.
//
bool
TGadget::IdleAction(long idleCount)
{
  if (idleCount == 0)
    CommandEnable();
  return false;
}

//
/// Provided so that the gadget can perform command enabling (so it can handle an
/// incoming message if it's appropriate to do so).
//
void
TGadget::CommandEnable()
{
}

//
/// Called when the system colors have been changed so that gadgets can rebuild and
/// repaint, if necessary.
//
void
TGadget::SysColorChange()
{
}

//
// A way to detect whether a button is created and visible
//
bool
TGadget::IsWindowVisible() const
{
  if (Window)
    return Window->IsWindowVisible(); 
  else
    return false;
}

//
/// Simple set accessor to set whether shrinkwrapping is performed horizontally
/// and/or vertically.
///
/// Your derived class can call TGadgetWindow::GadgetChangedSize() 
/// if you want to change the size of the gadget.
//
void
TGadget::SetShrinkWrap(bool shrinkWrapWidth, bool shrinkWrapHeight)
{
  ShrinkWrapWidth = shrinkWrapWidth;
  ShrinkWrapHeight = shrinkWrapHeight;
}

//
/// Alters the size of the gadget and then calls TGadgetWindow::GadgetChangedSize()
/// for the size change to take effect.
/// This function is needed only if you have turned off shrink-wrapping in one or
/// both dimensions; otherwise, use the GetDesiredSize() member function to return the
/// shrink-wrapped size.
//
void
TGadget::SetSize(const TSize& size)
{
  Bounds.right = Bounds.left + size.cx;
  Bounds.bottom = Bounds.top + size.cy;

  if (Window)
    Window->GadgetChangedSize(*this);
}

//
/// Enables or disables keyboard and mouse input for the gadget. By default, the
/// gadget is disabled when it is created and must be enabled before it can be
/// activated.
//
void
TGadget::SetEnabled(bool enabled)
{
  if (ToBool(Flags & Enabled) != enabled) {
    if (enabled)
      Flags |= Enabled;
    else
      Flags &= ~Enabled;
    Invalidate(true);
  }
}

//
/// Called by the gadget window to inform the gadget of a change in its bounding
/// rectangle. Default behavior here just updates instance variable "Bounds" but
/// derived classes might override this method to update other internal state.
//
void
TGadget::SetBounds(const TRect& rect)
{
  if (rect != Bounds) {
    Bounds = rect;
    Moved();
  }
}

//
/// Sets the borders for the gadget. If the borders are changed, SetBorders calls
/// TGadgetWindow::GadgetChangedSize() to notify the gadget window of the change.
//
void
TGadget::SetBorders(const TBorders& borders)
{
  Borders = borders;

  if (Window)
    Window->GadgetChangedSize(*this);
}

//
/// Sets the margins of the gadget. If the margins are changed, SetMargins calls
/// TGadgetWindow::GadgetChangedSize() to notify the gadget window.
//
void
TGadget::SetMargins(const TMargins& margins)
{
  Margins = margins;

  if (Window)
    Window->GadgetChangedSize(*this);
}

//
/// Set the border style used by this gadget. Internal Border members are
/// updated and owning Window is notified of a size change.
//
void
TGadget::SetBorderStyle(TBorderStyle borderStyle)
{
  BorderStyle = borderStyle;

  int  edgeThickness;
  switch (BorderStyle) {
    default:
    case None:
      edgeThickness = 0;
      break;

    case Plain:
    case Raised:
    case Recessed:
      edgeThickness = 1;
      break;

    case Embossed:
//      edgeThickness = 3;
    case Grooved:
    case ButtonUp:
    case ButtonDn:
    case WndRaised:
    case WndRecessed:
      edgeThickness = 2;
      break;
  }

  Borders.Left = Borders.Top = Borders.Right = Borders.Bottom = edgeThickness;

  if (Window)
    Window->GadgetChangedSize(*this);
}

//
/// Determines if the point is within the receiver's bounding rectangle and returns
/// true if this is the case; otherwise, returns false.
//
bool
TGadget::PtIn(const TPoint& point)
{
  return IsVisible() &&
         point.x >= 0 && point.y >= 0 &&
         point.x < Bounds.Width() && point.y < Bounds.Height();
}

//
/// This is the virtual called after the window holding a gadget has been created.
//
void
TGadget::Created()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  // Register ourself with the tooltip window (if there's one)
  //
  TTooltip* tooltip = Window->GetTooltip();
  if (tooltip && tooltip->IsWindow()) {

    // Don't register gadget's with Id's of 0 or -1.
    // Typically, 0 is reserved by separators and -1 could
    // be used for dumb text gadgets...
    //
    if (!predefinedGadgetId(Id)) {
      TToolInfo toolInfo(Window->GetHandle(), Bounds, Id);
      tooltip->AddTool(toolInfo);
    }
  }
}

//
/// Called after a gadget is inserted into a window.
//
void
TGadget::Inserted()
{
}

//
/// Virtual called after a gadget is removed from a window.
//
void
TGadget::Removed()
{
  // Unregister ourself with the tooltip window (if there's one)
  //
  if (Window && Window->GetHandle()) {
    TTooltip* tooltip = Window->GetTooltip();
    if (tooltip && tooltip->IsWindow()) {

      // Don't bother with gadgets with Id's of 0 or -1.
      // Typically, 0 is reserved by separators and -1 could
      // be used for dumb text gadgets...
      //
      if (!predefinedGadgetId(Id)) {
        TToolInfo toolInfo(Window->GetHandle(), Bounds, Id);
        tooltip->DeleteTool(toolInfo);
      }
    }
  }
}

//
/// This is the virtual called when a gadget is relocated.
//
void
TGadget::Moved()
{
  // Send tooltip window our updated location
  //
  if (Window && Window->GetHandle()) {
    TTooltip* tooltip = Window->GetTooltip();
    if (tooltip && tooltip->IsWindow()) {

      // Don't bother with gadgets with Id's of 0 or -1.
      // Typically, 0 is reserved by separators and -1 could
      // be used for dumb text gadgets...
      //
      if (!predefinedGadgetId(Id)) {

        // First retrieve information about the tool
        //
        TToolInfo ti(true);
        ti.SetToolInfo(*Window, Id);
        if (tooltip->GetToolInfo(ti)) {

          // Update the tooltip info if we've moved
          //
          if (TRect(ti.rect) != Bounds)
            tooltip->NewToolRect(TToolInfo(Window->GetHandle(), Bounds, Id));
        }

#if 0   // POSSIBLE ENHANCEMENT /////////////////////////////////////////////
        // If the tool information could not be retrieved, should we just
        // go ahead and add a brand new tool? We should really never get
        // here unless the tooltip of the gadget window was manipulated
        // behind our back or replaced with a new one...
        //
        else {
          TToolInfo toolInfo(Window->GetHandle(), Bounds, Id);
          tooltip->AddTool(toolInfo);
        }
#endif  /////////////////////////////////////////////////////////////////////
      }
    }
  }
}

//
/// Invalidate a rectangle in our containing window. Rectangle is specified
/// in gadget coordinates.
//
void
TGadget::InvalidateRect(const TRect& rect, bool erase)
{
  if (Window && Window->GetHandle()) {
    TRect  updateRect(rect.left + Bounds.left, rect.top + Bounds.top,
                      rect.right + Bounds.left, rect.bottom + Bounds.top);

    Window->InvalidateRect(updateRect, erase);
  }
}

//
/// Used to invalidate the active (usually nonborder) portion of the gadget,
/// Invalidate calls InvalidateRect and passes the boundary width and height of the
/// area to erase.
//
void
TGadget::Invalidate(bool erase)
{
  InvalidateRect(TRect(0, 0, Bounds.Width(), Bounds.Height()), erase);
}

//
/// Repaints the gadget if possible.
//
void
TGadget::Update()
{
  if (Window && Window->GetHandle())
    Window->UpdateWindow();
}

//
/// Used to paint the border, PaintBorder determines the width and height of the
/// gadget and uses the color returned by GetSyscolor to paint or highlight the area
/// with the specified brush. Depending on whether the border style is raised,
/// embossed, or recessed, PaintBorder paints the specified boundary. You can
/// override this function if you want to implement a border style that is not
/// supported by ObjectWindows' gadgets.
//
void
TGadget::PaintBorder(TDC& dc)
{
  if (BorderStyle != None) {
    int  xB = TUIMetric::CxBorder;
    int  yB = TUIMetric::CyBorder;

    TRect boundsRect(0,0,Bounds.Width(),Bounds.Height());
    if (BorderStyle == Plain) {
      TBrush winBru(TColor::SysWindowFrame);
      dc.OWLFastWindowFrame(winBru, boundsRect,xB, yB);
      dc.RestoreBrush();
    }
    else {
      if(Window->GetFlatStyle()&TGadgetWindow::FlatStandard){
        TUIBorder(boundsRect, (uint)BorderStyle == (uint)TUIBorder::Recessed ?
                  TUIBorder::SunkenOuter : TUIBorder::RaisedInner,
                  TUIBorder::Rect).Paint(dc);
      }
      else{
        // Use the 1:1 mapping of BorderStyle to TUIBorder::TStyle
        TUIBorder(boundsRect, TUIBorder::TStyle(BorderStyle)).Paint(dc);
      }
    }
  }
}

//
/// Calls PaintBorder() to paint the indicated device context.
//
void
TGadget::Paint(TDC& dc)
{
  PaintBorder(dc);
}

//
/// Request by the gadget window to query the gadget's desired size.
//
/// Determines how big the gadget can be. The gadget window sends this message to
/// query the gadget's size. If shrink-wrapping is requested, GetDesiredSize returns
/// the size needed to accommodate the borders and margins. If shrink-wrapping is
/// not requested, it returns the gadget's current width and height. TGadgetWindow
/// needs this information to determine how big the gadget needs to be, but it can
/// adjust these dimensions if necessary. If WideAsPossible is true, then the width
/// parameter (size.cx) is ignored.
//
void
TGadget::GetDesiredSize(TSize& size)
{
  int left  = 0;
  int right = 0;
  int top   = 0;
  int bottom = 0;
  GetOuterSizes(left, right, top, bottom);

  size.cx = ShrinkWrapWidth ? left+right : Bounds.Width();
  size.cy = ShrinkWrapHeight ? top+bottom : Bounds.Height();
}

//
/// Get the four total outer sizes in pixels which consists of the margins
/// plus the borders.
//
void
TGadget::GetOuterSizes(int& left, int& right, int& top, int& bottom)
{
  if (Window) {
    int  xBorder = TUIMetric::CxBorder;
    int  yBorder = TUIMetric::CyBorder;

    Window->GetMargins(Margins, left, right, top, bottom);
    left += Borders.Left * xBorder;
    right += Borders.Right * xBorder;
    top += Borders.Top * yBorder;
    bottom += Borders.Bottom * yBorder;
  }
}

//
/// Computes the area of the gadget's rectangle excluding the borders and margins.
//
void
TGadget::GetInnerRect(TRect& innerRect)
{
  int left  = 0;
  int right = 0;
  int top   = 0;
  int bottom = 0;
  GetOuterSizes(left, right, top, bottom);

  innerRect.left = left;
  innerRect.right = Bounds.Width() - right;
  innerRect.top = top;
  innerRect.bottom = Bounds.Height() - bottom;
}

//
// Mouse response functions
//

//
/// Mouse is entering this gadget. Called by gadget window if no other gadget
/// has capture.
///
/// For toolbars with the flat style we want to allow the gadget to redraw with 
/// a slightly different style (e.g. border) when the mouse is hovering over 
/// the gadget. We handle this here by setting a flag (MouseInGadget) and then
/// invalidating the gadget. 
//
void
TGadget::MouseEnter(uint /*modKeys*/, const TPoint&)
{
  PRECONDITION(Window);
    MouseInGadget = true;
    if(Window->GetFlatStyle()&TGadgetWindow::FlatStandard)
      Invalidate(false); // is flat style -> repaint gadget TWindow
}

//
/// Mouse is moving over this gadget. Called by gadget window only if this
/// gadget has captured the mouse
///
/// point is located in the receiver's coordinate system.
//
void
TGadget::MouseMove(uint /*modKeys*/, const TPoint&)
{
}

//
/// Mouse is leaving this gadget. Called by gadget window if no other gadget
/// has capture
//
void
TGadget::MouseLeave(uint /*modKeys*/, const TPoint&)
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  // Send a fake WM_MOUSEMOVE to the tooltip in case the user moved away
  // from the gadget *very* quick without the window detecting mouse move
  // messages.
  //
  TTooltip* tooltip = Window->GetTooltip();
  if (tooltip && tooltip->IsWindow()) {

    TPoint crsPoint;
    GetCursorPos(&crsPoint);
    HWND hwnd = WindowFromPoint(crsPoint);
    if (hwnd)
      ::MapWindowPoints(HWND_DESKTOP, hwnd, LPPOINT(&crsPoint), 1);
    else
      hwnd = GetDesktopWindow();

    MSG msg;
    msg.hwnd    = hwnd;
    msg.message = WM_MOUSEMOVE;
    msg.wParam  = 0;
    msg.lParam  = MkUint32((uint16)crsPoint.x, (uint16)crsPoint.y);
    tooltip->RelayEvent(msg);
  }
  //
  // for flat a la IE 3.0 toolbar
  //
  if(MouseInGadget){
    MouseInGadget = false;
    if(Window->GetFlatStyle()&TGadgetWindow::FlatStandard)
      Invalidate(true); // is flat style -> repaint gadget
  }
}

//
/// Captures the mouse if TrackMouse is set. point is located in the gadget's
/// coordinate system.
//
void
TGadget::LButtonDown(uint /*modKeys*/, const TPoint&)
{
  if (TrackMouse)
    Window->GadgetSetCapture(*this);
}

//
/// Releases the mouse capture if TrackMouse is set. point is located in the
/// gadget's coordinate system.
//
void
TGadget::LButtonUp(uint /*modKeys*/, const TPoint&)
{
  if (TrackMouse)
    Window->GadgetReleaseCapture(*this);
}

//
//
//
void
TGadget::RButtonDown(uint /*modKeys*/, const TPoint&)
{
  // TGadget does nothing with right mouse messages.
  // However, a derived gadgets may catch this event
}

//
//
//
void
TGadget::RButtonUp(uint /*modKeys*/, const TPoint&)
{
  // TGadget does nothing with right mouse messages.
  // However, a derived gadgets may catch this event
}

//----------------------------------------------------------------------------
const int DefaultGripSize = 6;


//
/// Used for both the width and the height of the separator, the default size is
/// TUIMetric::CxSizeFrame times two. id is the ID of the TGadget object.
//
TSeparatorGadget::TSeparatorGadget(int size, int id, bool showsep)
:
  TGadget(id),
  ShowSeparator(showsep)
{
  ShrinkWrapWidth = ShrinkWrapHeight = false;
  SetEnabled(false);
  SetVisible(false);

  // Default size to a sysMetric based value
  //
  if (!size)
    size = TUIMetric::CxSizeFrame * 2;
  Bounds.right  = size;
  Bounds.bottom = size;
}

//
/// This is an overridden virtual, called after a gadget is inserted into a window.
//
void
TSeparatorGadget::Inserted()
{
  BorderStyle = None;  // Prohibit our style from being changed
}

//
//
//
void
TSeparatorGadget::Paint(TDC& dc)
{
  TGadget::Paint(dc); // Does nothing since BorderStyle is None.

  bool flat = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatStandard;
  if (flat && ShowSeparator)
  {
    const bool horizontal = GetGadgetWindow()->GetDirection() == TGadgetWindow::Horizontal;

    // Calculate border.
    //
    TSize border;
    bool xpstyle = GetGadgetWindow()->IsThemed();
    if (xpstyle)
      border = TSize(TUIMetric::CxBorder, TUIMetric::CyBorder);
    else
      border = TSize(TUIMetric::CxSizeFrame, TUIMetric::CySizeFrame);

    // Expand the bounds rectangle around the separator so that its
    // height (horizontal) or width (vertical) matches the gadget window.
    //
    TRect bounds = Bounds.MovedTo(0, 0);
    const TRect bar = GetGadgetWindow()->GetClientRect();
    bounds.Inflate
      (
      horizontal ? 0 : (bar.Width() - Bounds.Width())/2 - border.cx,
      !horizontal ? 0 : (bar.Height() - Bounds.Height())/2 - border.cy
      );

    // Draw the separator in the active style.
    //
    if (xpstyle)
    {
      int part_id = horizontal ? TP_SEPARATOR : TP_SEPARATORVERT;
      TThemePart part(GetGadgetWindow()->GetHandle(), (LPCWSTR) L"TOOLBAR", part_id, TS_NORMAL);
      part.DrawTransparentBackground(GetGadgetWindow()->GetHandle(), dc, bounds);
    }
    else // old flat style
    {
      // Narrow the drawing rectangle to 2 pixels in the center.
      //
      if (horizontal)
      {
        bounds.left = bounds.Width()/2-1;
        bounds.right = bounds.left+2;
      }
      else // vertical
      {
        bounds.top = bounds.Height()/2-1;
        bounds.bottom = bounds.top+2;
      }
      uint flags = horizontal ? TUIBorder::Right : TUIBorder::Top;
      TUIBorder::DrawEdge(dc, bounds, TUIBorder::Embossed, flags);
    }
  }
}

//
//------------------------------------------------------------------------------
//
TFlatHandleGadget::TFlatHandleGadget(int id)
:
  TSeparatorGadget(TUIMetric::CySizeFrame > TUIMetric::CxSizeFrame ?
                   TUIMetric::CySizeFrame :
                   TUIMetric::CxSizeFrame, id)
{
  // Enable gadget so mouse hit testing handler will let cursor change
  // when user moves over the size grip.
  //
  SetEnabled(true);
}


//
void
TFlatHandleGadget::GetDesiredSize(TSize& size)
{
  TSeparatorGadget::GetDesiredSize(size);

  if(GetGadgetWindow()->GetDirection() == TGadgetWindow::Rectangular)
    size.cx = size.cy = 0;
  else{
    TSize maxsize;
    for (TGadget* gadg = GetGadgetWindow()->FirstGadget(); gadg; gadg = GetGadgetWindow()->NextGadget(*gadg)){
      int id = gadg->GetId();
      if (id != IDG_FLATHANDLE && gadg != this){
        TSize gsize(0, 0);
        gadg->GetDesiredSize(gsize);
        maxsize.cx = std::max(gsize.cx,maxsize.cx);
         maxsize.cy = std::max(gsize.cy,maxsize.cy);
      }
    }

    if (Window->GetDirection()==TGadgetWindow::Horizontal){
      size.cx = DefaultGripSize;
      size.cy = maxsize.cy;
    }
    else if (Window->GetDirection()==TGadgetWindow::Vertical){
      size.cx = maxsize.cx;
      size.cy = DefaultGripSize+1;
    }
  }
}

// Draw the resize gadget.
//
void
TFlatHandleGadget::Paint(TDC& dc)
{
  if(Window->GetDirection()==TGadgetWindow::Horizontal){
    TRect boundsRect = Bounds;
    dc.DPtoLP((TPoint*)&boundsRect, 2);
    boundsRect.right = boundsRect.left+3;
    if(Window->GetFlatStyle()&TGadgetWindow::FlatSingleDiv){
      boundsRect.Offset(-2,0);
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
    }
    else{
      boundsRect.Offset(-3,0);
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
      boundsRect.Offset(3,0);
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
    }
  }
  else if(Window->GetDirection()==TGadgetWindow::Vertical){
    TRect boundsRect = Bounds;
    dc.DPtoLP((TPoint*)&boundsRect, 2);
    boundsRect.bottom = boundsRect.top+3;
    if(Window->GetFlatStyle()&TGadgetWindow::FlatSingleDiv)
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
    else{
      boundsRect.Offset(0,-1);
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
      boundsRect.Offset(0,3);
      TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::RaisedInner, TUIBorder::Rect);
    }
  }
}
//------------------------------------------------------------------------------

//
/// Constructs a gadget that can be grabbed to resize the frame.
//
TSizeGripGadget::TSizeGripGadget(int id)
:
  TSeparatorGadget(TUIMetric::CxHScroll > TUIMetric::CyVScroll ?
                   TUIMetric::CxHScroll :
                   TUIMetric::CyVScroll, id)
{
  // Enable gadget so mouse hit testing handler will let cursor change
  // when user moves over the size grip.
  //
  SetEnabled(true);
}

//
/// Draws the resize gadget.
//
void
TSizeGripGadget::Paint(TDC& dc)
{
  int left  = 0;
  int right = 0;
  int top   = 0;
  int bottom = 0;
  GetOuterSizes(left, right, top, bottom);

  TRect innerRect;
  innerRect.left = left;
  innerRect.top = top;
  innerRect.right = Bounds.Width() + 1;
  innerRect.bottom = Bounds.Height() + 1;

///TH  ::DrawFrameControl(dc, &innerRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
  TUIPart part;
  part.Paint(dc, innerRect, TUIPart::uiScroll, TUIPart::ScrollSizeGrip);
}

} // OWL namespace
/* ========================================================================== */


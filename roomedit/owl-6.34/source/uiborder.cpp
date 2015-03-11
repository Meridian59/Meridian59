//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/uihelper.h>
#include <owl/gdiobjec.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a UIBorder object given a frame and a high-level style type.
/// Calculates Edge and modifier flags internally, as needed.
//
TUIBorder::TUIBorder(const TRect& frame, TUIBorder::TStyle style, uint flags)
:
  Frame(frame)
{
  PRECONDITION(style >= 0 && style <= WellSet);

  static struct {
    uint Edge;
    uint Flags;
  }
  styleMap[] = {
    {0,                         0},    // None
    {0,                         0},    // Plain
    {RaisedOuter,               0},    // Raised
    {SunkenOuter,               0},    // Recessed
    {RaisedOuter | SunkenInner, 0},    // Embossed
    {SunkenOuter | RaisedInner, 0},    // Grooved
    {RaisedOuter | RaisedInner, Soft}, // ButtonUp
    {SunkenOuter | SunkenInner, Soft}, // ButtonDn
    {RaisedOuter | RaisedInner, 0},    // WndRaised
    {SunkenOuter | SunkenInner, 0},    // WndRecessed
    {SunkenOuter | RaisedInner, 0},    // WellSet ???
  };
  Edge = styleMap[style].Edge;
  Flags = styleMap[style].Flags | flags;
  if (!(Flags & Rect))
    Flags |= Rect;
}

//
/// Constructs a UIBorder object given a frame, edge, and modifier flags.
//
TUIBorder::TUIBorder(const TRect& frame, TEdge edge, uint flags)
:
  Frame(frame),
  Edge(edge),
  Flags(flags)
{
  if (!(Flags & Rect))
    Flags |= Rect;
}

//
/// Moves the frame rect by (dx,dy).
//
void
TUIBorder::Move(int dx, int dy)
{
  Frame.Offset(dx, dy);
}

//
/// Moves the frame rect to (x,y).
//
void
TUIBorder::MoveTo(int x, int y)
{
  Frame.Offset(x-Frame.left, y-Frame.top);
}

//
/// Resizes the frame rect to (w,h).
//
void
TUIBorder::Size(int w, int h)
{
  Frame.right = Frame.left + w;
  Frame.top = Frame.top + h;
}

//
/// Calculates the outside frame rectangle.
//
TRect
TUIBorder::GetBoundingRect() const
{
//  return Style == WellSet ?  Frame.InflatedBy(1,1) : Frame;
  return Frame;
}

//
/// Calculates the rectangle within the border.
//
TRect
TUIBorder::GetClientRect() const
{
  int count = ((Edge & EdgeOuter) ? 1 : 0) + ((Edge & EdgeInner) ? 1 : 0);
  return Frame.InflatedBy(-count,-count);
}

//
/// Paints this UIBorder object onto a given device context.
//
void
TUIBorder::Paint(TDC& dc) const
{
  DrawEdge(dc, Frame, Edge, Flags);

//  case WellSet:
//      PaintWT(dc, Frame.InflatedBy(1,1));
//      Paint3H(dc, Frame);
//      PaintWT(dc, Frame.InflatedBy(-1,-1));
}

//
/// This is a static function that performs the actual drawing of edges for a
/// UIBorder or an external client. It uses the system ::DrawEdge if available.
//
bool
TUIBorder::DrawEdge(TDC& dc, const TRect& frame, uint edge, uint flags)
{
  static int hasDrawEdge = true;

  // Try once to see if the API call is available. If not, do ourselves.
  //
  if (hasDrawEdge) {
    if (::DrawEdge(dc, (LPRECT)&frame, edge, flags))
      return true;
    if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
      hasDrawEdge = false;
    else
      return false;
  }

  // ::DrawEdge is not available, do the drawing ourselves
  //
  TRect f(frame);  // working frame rectangle

  // If mono is set, draw a thin, flat, black (windowFrame) frame
  //
  if (flags & Mono) {
    if (edge & EdgeOuter) {
      PaintFrame(dc, f, flags, TColor::SysWindowFrame, TColor::SysWindowFrame);
      f.Inflate(-1,-1);
    }
    if (flags & Fill) { // !CQ repeated code--nest else?
      TBrush brsh(TColor::SysWindow);
      dc.SelectObject(brsh);
      dc.PatBlt(f);
      dc.RestoreBrush();
    }
    return true;
  }

  // If flat is set, draw a thin, flat, shadow frame
  //
  if (flags & Flat) {
    if (edge & EdgeOuter) {
      PaintFrame(dc, f, flags, TColor::Sys3dShadow, TColor::Sys3dShadow);
      f.Inflate(-1,-1);
    }
    if (flags & Fill) { // !CQ repeated code--nest else?
      TBrush brsh(TColor::Sys3dFace);
      dc.SelectObject(brsh);
      dc.PatBlt(f);
      dc.RestoreBrush();
    }
    return true;
  }

  // Draw outer edge if indicated, adjusting rect afterwards
  //
  if (edge & EdgeOuter) {
    static TColor tlColors[] = {
      TColor::Sys3dLight,       // EdgeRaised
      TColor::Sys3dHilight,     // EdgeRaised + Soft
      TColor::Sys3dShadow,      // EdgeSunken
      TColor::Sys3dDkShadow,    // EdgeSunken + Soft
    };
    static TColor brColors[] = {
      TColor::Sys3dDkShadow,    // EdgeRaised
      TColor::Sys3dDkShadow,    // EdgeRaised + Soft
      TColor::Sys3dHilight,     // EdgeSunken
      TColor::Sys3dHilight,     // EdgeSunken + Soft
    };
    int ci = ((edge & SunkenOuter) ? 2 : 0) | ((flags & Soft) ? 1 : 0);
    PaintFrame(dc, f, flags, tlColors[ci], brColors[ci]);
    f.Inflate(-1,-1);
  }

  // Draw inner edge if indicated, adjusting rect afterwards
  //
  if (edge & EdgeInner) {
    static TColor tlColors[] = {
      TColor::Sys3dHilight,     // EdgeRaised
      TColor::Sys3dLight,       // EdgeRaised + Soft
      TColor::Sys3dDkShadow,    // EdgeSunken
      TColor::Sys3dShadow,      // EdgeSunken + Soft
    };
    static TColor brColors[] = {
      TColor::Sys3dShadow,      // EdgeRaised
      TColor::Sys3dShadow,      // EdgeRaised + Soft
      TColor::Sys3dLight,       // EdgeSunken
      TColor::Sys3dLight,       // EdgeSunken + Soft
    };
    int ci = ((edge & SunkenOuter) ? 2 : 0) | ((flags & Soft) ? 1 : 0);
    PaintFrame(dc, f, flags, tlColors[ci], brColors[ci]);
    f.Inflate(-1,-1);
  }

  // Fill interior if indicated
  //
  if (flags & Fill) {
    TBrush brsh(TColor::Sys3dFace);
    dc.SelectObject(brsh);
    dc.PatBlt(f);
    dc.RestoreBrush();
  }

// !CQ not really usefull since frame is not returned
//  if (flags & Adjust)
//    frame = f;

  return true;
}

//
/// Paints a 2-color single pixel-thick frame. Bevel corners get brush color.
//
void
TUIBorder::PaintFrame(TDC& dc, const TRect& fr, uint flags, const TColor& tlColor, const TColor& brColor)
{
  if (flags & (Top | Left)) {
    TBrush brsh(tlColor);
    dc.SelectObject(brsh);
    if (flags & Top)
      dc.PatBlt(fr.left, fr.top, fr.Width()-1, 1);
    if (flags & Left)
      dc.PatBlt(fr.left, fr.top+1, 1, fr.Height()-2);
    dc.RestoreBrush();
  }

  if (flags & (Bottom | Right)) {
    TBrush brsh(brColor);
    dc.SelectObject(brsh);
    if (flags & Bottom)
      dc.PatBlt(fr.left, fr.bottom-1, fr.Width(), 1);
    if (flags & Right)
      dc.PatBlt(fr.right-1, fr.top, 1, fr.Height()-1);
    dc.RestoreBrush();
  }
}

//
/// Paints a 2-color single pixel-thick frame. Bevel corners get their own color.
//
void
TUIBorder::PaintFrameC(TDC& dc, const TRect& fr, uint flags, const TColor& tlColor, const TColor& brColor, const TColor& bcColor)
{
  if (flags & (Top | Left)) {
    TBrush brsh(tlColor);
    dc.SelectObject(brsh);
    if (flags & Top) {
      dc.PatBlt(fr.left, fr.top, fr.Width()-2, 1);
      dc.SetPixel(fr.right-1, fr.top, bcColor);
    }
    if (flags & Left)
      dc.PatBlt(fr.left, fr.top+1, 1, fr.Height()-2);
    dc.RestoreBrush();
  }

  if (flags & (Bottom | Right)) {
    TBrush brsh(brColor);
    dc.SelectObject(brsh);
    if (flags & Bottom) {
      dc.SetPixel(fr.left, fr.bottom-1, bcColor);
      dc.PatBlt(fr.left+1, fr.bottom-1, fr.Width(), 1);
    }
    if (flags & Right)
      dc.PatBlt(fr.right-1, fr.top, 1, fr.Height()-1);
    dc.RestoreBrush();    
  }
}

#if 0
//
//
//
void
TUIBorder::PaintWT(TDC& dc, const TRect& frame)
{
  PaintFrame(dc, frame, TColor::SysWindowText, TColor::SysWindowText);
}

//
//
//
void
TUIBorder::Paint3H(TDC& dc, const TRect& frame)
{
  PaintFrame(dc, frame, TColor::Sys3dHilight, TColor::Sys3dHilight);
}
#endif

} // OWL namespace
/* ========================================================================== */


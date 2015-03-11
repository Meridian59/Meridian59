//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TGauge, a gauge user interface widget
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gauge.h>
#include <owl/dc.h>
#include <owl/uimetric.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

DEFINE_RESPONSE_TABLE1(TGauge, TControl)
  EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

//
/// General constructor for a TGauge object - Use this constructor if you
/// are creating an object whose features might require that OWL provides
/// the underlying implementation. For example, a vertical progress bar.
///
/// Constructs a TGauge object with borders that are determined by using the value
/// of SM_CXBORDER. Sets IsHorizontal to isHorizontal. Sets border thickness and
/// spacing between dashed borders (LEDs) to 0. Sets the range of possible values
/// from 0 to 100.
//
/// \note This constructor default to a LedSpacing and LedThickness of 0,
///       which OWL interpretes as a request for a solid/filled progress bar.
///       You must explicitly invoke 'SetLed' with non-zero values if you
///       want otherwise.
//
TGauge::TGauge(TWindow*        parent,
               LPCTSTR         title,
               int             id,
               int x, int y, int w, int h,
               bool            isHorizontal,
               int             margin,
               TModule*        module)
:
  TControl(parent, id, title, x, y, w, h ? h : int(TUIMetric::CyVScroll), module)
{
  Init(isHorizontal, margin);
  LedSpacing = 0;
  LedThick = 0;
}

//
/// String-aware overload
//
TGauge::TGauge(
  TWindow* parent,
  const tstring& title,
  int id,
  int x, int y, int w, int h,
  bool isHorizontal,
  int margin,
  TModule* module
  )
  : TControl(parent, id, title, x, y, w, h ? h : int(TUIMetric::CyVScroll), module)
{
  Init(isHorizontal, margin);
  LedSpacing = 0;
  LedThick = 0;
}

//
/// Simplified constructor for a TGauge object. Creates a horizontal LED style gauge.
///
TGauge::TGauge(TWindow*        parent,
               int             id,
               int x, int y, int w, int h,
               TModule*        module)
:
  TControl(parent, id, _T(""), x, y, w, h ? h : int(TUIMetric::CyVScroll), module)
{
  Init(true, 1);
  LedSpacing = ((Attr.H - 2*Margin) * 2) / 3 + 2*Margin;
  LedThick = 100 * LedSpacing / (LedSpacing - 2*Margin);
}

//
// Constructor for a resource gauge object.
/// \note For this constructor to work correctly, the resource control must be "OWL_Gauge". For example: 
/// \code
///  CONTROL "ProgressBar2", IDC_PROGRESSBAR2, "OWL_Gauge", WS_CHILD | WS_VISIBLE | WS_BORDER, 64, 120, 104, 12
/// \endcode
//
TGauge::TGauge(TWindow*        parent,
               int             resId,
               TModule*        module)
:
  TControl(parent, resId, module)
{
  Init(true, 1);
  LedSpacing = 10;
  LedThick = 90;
}

void TGauge::Init(bool isHorizontal, int margin)
{
  Min = 0;
  Max = 100;
  Step = 10;
  Value = 0;
  Margin = margin * TUIMetric::CxBorder;
  IsHorizontal = isHorizontal;
  BarColor = TColor::None;
  Attr.Style &= ~WS_TABSTOP;
}

//
/// Returns the class name of the gauge class
//
TWindow::TGetClassNameReturnType
TGauge::GetClassName()
{
  return _T("OWL_Gauge");
}

//
/// Sets the Min and Max data members to minValue and maxValue values returned by the
/// constructor. If Max is less than or equal to Min, SetRange resets Max to Min +
/// 1.
//
void
TGauge::SetRange(int minValue, int maxValue)
{
  if (maxValue <= minValue)
    maxValue = minValue+1;

  Min = minValue;
  Max = maxValue;
}

//
/// Sets the BarColor data member to the value specified in color.
//
void
TGauge::SetColor(const TColor& color)
{
  BarColor = color;
  if(color == TColor((COLORREF)CLR_DEFAULT))
    BarColor = TColor::None;
  // add
}

//
void
TGauge::SetBkgndColor(const TColor& color)
{
  BkgndColor = color;
  if(color == TColor((COLORREF)CLR_DEFAULT))
    BkgndColor = TColor::None;
}

//
/// Sets the Step amount of the gauge for StepIt operations
//
void
TGauge::SetStep(int step)
{
  Step = step;
}

//
/// Set the value of the gauge
//
/// Restricts value to be within the minimum and maximum values established for the
/// gauge. If the current value has changed, SetValue marks the old position for
/// repainting. Then, it sets the data member Value to the new value.
//
void
TGauge::SetValue(int value)
{
  // Constrain value to be in the range "Min..Max"
  //
  if (value > Max)
    value = Max;

  else if (value < Min)
    value = Min;

  // Paint to new position, converting value to pixels
  //
  if (value != Value) {
    if (GetHandle()) {
      Invalidate(false);
    }
    Value = value;
  }
}

//
/// Changes the value of the gauge by the given delta.
//
void
TGauge::DeltaValue(int delta)
{
  if (!delta)
    return;

  // Constrain delta such that Value stays in the range "Min..Max"
  //
  if (delta + Value > Max)
    delta = Max - Value;

  else if (delta + Value < Min)
    delta = Min - Value;

  if (GetHandle()) {
    // Paint to new position, converting value to pixels
    //
    Invalidate(false);
  }

  Value += delta;
}

//
/// Adjusts the active gauge value by the Step increment. If the new value exceeds
/// the Max value of the gauge, StepIt wraps the setting of the gauge to its Min
/// value.
//
void
TGauge::StepIt()
{
  if (GetHandle()) {
    if (Value + Step < Max)
      DeltaValue(Step);
    else
      SetValue(Min);
  }
}

//
/// Sets the LedSpacing and LedThick data members to the values spacing and thick.
//
void
TGauge::SetLed(int spacing, int thickPercent)
{
  LedSpacing = spacing;
  LedThick = thickPercent;
}

//
/// If a system control is being used, updates it to match our member settings.
//
void
TGauge::SetupWindow()
{
  TControl::SetupWindow();
}

//
/// Paints the border (bevel and margin).
///
/// Paints the gauge border using the specified device context. Depending on whether
/// the border style is raised, embossed, or recessed, PaintBorder paints the
/// specified boundary. You can override this function if you want to implement a
/// border style that is not supported by ObjectWindows' gauges.
//
void
TGauge::PaintBorder(TDC& dc)
{
  int   xBorder = TUIMetric::CxBorder;
  int   yBorder = TUIMetric::CyBorder;
  TRect cr(GetClientRect());
  int   w = cr.right;
  int   h = cr.bottom;

  TBrush shadowBrush(TColor::Sys3dShadow);
  dc.SelectObject(shadowBrush);
  dc.PatBlt(0, 0, w, yBorder);
  dc.PatBlt(0, yBorder, xBorder, h-yBorder);

  TBrush hiliteBrush(TColor::Sys3dHilight);
  dc.SelectObject(hiliteBrush);
  dc.PatBlt(xBorder, h-yBorder, w-xBorder, h-yBorder);
  dc.PatBlt(w-xBorder, yBorder, xBorder, h-yBorder-yBorder);

  TBrush  faceBrush(TColor::Sys3dFace);
  TRect   innerRect(xBorder, yBorder, w-xBorder, h-yBorder);

  // Walk in from the bevel painting frames as we go
  //
  for (int i = 0; i < Margin; i++) {
    dc.FrameRect(innerRect, faceBrush);
    innerRect.Inflate(-1, -1);
  }
}

//
/// Overrides TWindow::Paint and paints the area and the border of the gauge. Uses
/// the values supplied in rect and dc to paint the given rectangle on the given
/// device context. Uses the values in LedSpacing and IsHorizontal to draw a
/// horizontal or vertical gauge with solid or broken bars.
//
void
TGauge::Paint(TDC& dc, bool /*erase*/, TRect&)
{
  PaintBorder(dc);

  // Prepare to paint the bar or LED sequence in the well
  //
  int    xBorder = TUIMetric::CxBorder;
  int    yBorder = TUIMetric::CyBorder;
  TRect cr(GetClientRect());
  int   w = cr.right;
  int   h = cr.bottom;

  TColor ledcolor = BarColor;
  if(ledcolor == TColor::None)
    ledcolor = TColor::SysHighlight;
  TColor bkcolor = BkgndColor;
  if(bkcolor == TColor::None)
    bkcolor = TColor::Sys3dFace;

  TBrush  barBrush(ledcolor);
  TBrush  faceBrush(bkcolor);
  TRect   innerRect(xBorder+Margin, yBorder+Margin,
                    w-xBorder-Margin, h-yBorder-Margin);

  // Draw either LEDs or a solid bar as indicated by LedSpacing
  //
  if (LedSpacing) {
    if (IsHorizontal) {
      int ledStep = (innerRect.Width()*LedSpacing)/(Max-Min);
      int ledWidth = (ledStep*LedThick)/100;
      int gapWidth = ledStep - ledWidth;
      int x = innerRect.left;
      int right = innerRect.left +
                  int((long(Value-Min)*innerRect.Width())/(Max-Min));
      for (; x < right; x += ledStep) {
        dc.FillRect(x, innerRect.top, x+ledWidth, innerRect.bottom, barBrush);
        dc.FillRect(x+ledWidth, innerRect.top, x+ledWidth+gapWidth, innerRect.bottom, faceBrush);
      }
      dc.FillRect(x, innerRect.top, innerRect.right, innerRect.bottom, faceBrush);
    }
    else {
      int ledStep = int((long(innerRect.Height())*LedSpacing)/(Max-Min));
      int ledHeight = int((long(ledStep)*LedThick)/100);
      int gapHeight = ledStep - ledHeight;
      int y = innerRect.bottom;
      int top = innerRect.top + innerRect.Height() -
                int((long(Value-Min)*innerRect.Height())/(Max-Min));
      for (; y > top; y -= ledStep) {
        dc.FillRect(innerRect.left, y-ledHeight, innerRect.right, y, barBrush);
        dc.FillRect(innerRect.left, y-ledHeight-gapHeight, innerRect.right, y-ledHeight, faceBrush);
      }
      dc.FillRect(innerRect.left, innerRect.top, innerRect.right, y, faceBrush);
    }
  }
  else {
    TRect barRect(innerRect);
    TRect emptyRect(innerRect);
    if (IsHorizontal) {
      int w = int((long(Value-Min)*innerRect.Width())/(Max-Min));
      barRect.right = emptyRect.left = innerRect.left+w;
    }
    else {
      int h = innerRect.Height() -
              int((long(Value-Min)*innerRect.Height())/(Max-Min));
      barRect.top = emptyRect.bottom = innerRect.top+h;
    }
    dc.FillRect(emptyRect, faceBrush);
    dc.FillRect(barRect, barBrush);

    LPCTSTR c = GetCaption();
    if (c && *c) {
      tchar buff[32];
      wsprintf(buff, c, Value);

      int   len = ::_tcslen(buff);
      TSize extent = dc.GetTextExtent(&buff[0], len);
      int   x = innerRect.left;
      int   y = innerRect.top;

      if (extent.cx < innerRect.Width())
        x += (innerRect.Width() - extent.cx) / 2;  // center text horizontally

      if (extent.cy < innerRect.Height())
        y += (innerRect.Height() - extent.cy) / 2; // center text vertically

      // use ExtTextOut() to paint the text in contrasting colors to the bar
      // and background
      //
      dc.SetBkMode(TRANSPARENT);
      dc.SetTextColor(bkcolor);
      dc.ExtTextOut(x, y, ETO_CLIPPED, &barRect, &buff[0], ::_tcslen(buff));
      dc.SetTextColor(ledcolor);
      dc.ExtTextOut(x, y, ETO_CLIPPED, &emptyRect, &buff[0], ::_tcslen(buff));
    }
  }
}

//
/// Overrides the TWindow::EvEraseBkgnd function and erases the background of the
/// gauge. Whenever the background is repainted, EvEraseBkgnd is called to avoid
/// flickering.
//
bool
TGauge::EvEraseBkgnd(HDC)
{
  return true;
}

} // OWL namespace
/* ========================================================================== */


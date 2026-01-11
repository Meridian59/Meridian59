//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Provides TGaugeGadget, a gadget multiply-derived from TGauge and TGadget,
// for use in Gadget Windows.
//
// Original code:
// Copyright (c) 1997 Rich Goldstein, MD
// goldstei@interport.net
// May be used without permission, if appropriate credit is given
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/gaugegad.h>
#include <owl/gadgetwi.h>

using namespace owl;

namespace OwlExt {


TGaugeGadget::TGaugeGadget(int id, LPCTSTR title, int width, TBorderStyle style)
:
TGadget(id, style),
TGauge(0, title, id, 0, 0, width, 1, true, 0),
_width(width)
{
  ModifyStyle(0,WS_CLIPSIBLINGS);
  WideAsPossible = false;
  Margins.Left = Margins.Right = TUIMetric::CxFixedFrame;
  TGauge::SetLed(0);
  TGadget::SetShrinkWrap(true, true);
}

TGaugeGadget::~TGaugeGadget()
{
  TGauge::Destroy(0);
}

void
TGaugeGadget::SetBounds(const TRect& bounds)
{
  // Set the gadget bounds, then move, size & repaint the control
  //
  TGadget::SetBounds(bounds);
  TGauge::SetWindowPos(0, Bounds, SWP_NOACTIVATE|SWP_NOZORDER);
}


// Calc the desired size base on the gadget windows font
// to make it look like the text gadgets
void
TGaugeGadget::GetDesiredSize(TSize& size)
{
  TGadget::GetDesiredSize(size);
  int height = 12;
  const TFont* font = &(Window->GetFont());

  if (font)
    height = font->GetHeight();


  if (ShrinkWrapWidth)
    size.cx += _width;
  if (ShrinkWrapHeight)
    size.cy += height + 2;
}

void
TGaugeGadget::Created()
{
  // Create control is necessary
  //
  TGauge::SetParent(Window);
  if(Window->GetHandle() && !TGauge::GetHandle()){
    TGauge::Create();
    TGauge::ShowWindow(SW_SHOWNA);
  }
  // Not registered for tooltips
}

void
TGaugeGadget::Inserted()
{
  TGauge::SetParent(Window);
  if (Window->GetHandle() && !TGauge::GetHandle()){
    TGauge::Create();
    TGauge::ShowWindow(SW_SHOWNA);
  }
}

void
TGaugeGadget::Removed()
{
  TGauge::SetParent(0);
}

void
TGaugeGadget::InvalidateRect(const TRect& rect, bool erase)
{
  TGadget::InvalidateRect(rect, erase);
  if (TGauge::GetHandle())
    TGauge::InvalidateRect(rect, erase);
}

void
TGaugeGadget::Update()
{
  if(TGauge::GetHandle())
    TGauge::UpdateWindow();
}

void
TGaugeGadget::Paint(TDC& dc, bool erase, TRect &rect)
{
  // Select the approprate font
  dc.SelectObject(Window->GetFont());
  TGauge::Paint(dc, erase, rect);
  dc.RestoreFont();
}

} // OwlExt namespace


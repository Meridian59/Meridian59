//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TControlBar.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/controlb.h>
#include <owl/uimetric.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlWin);

//
/// Constructs a TControlBar interface object with the specified direction (either
/// horizontal or vertical) and window font.
//
TControlBar::TControlBar(TWindow*        parent,
                         TTileDirection  direction,
                         TFont*          font,
                         TModule*        module)
:
  TGadgetWindow(parent, direction, font, module)
{
  GetMargins().Units = TMargins::BorderUnits;

  if (GetDirection() == Horizontal){
    GetMargins().Left = GetMargins().Right = TUIMetric::CxFixedFrame;
    GetMargins().Top = GetMargins().Bottom = TUIMetric::CyFixedFrame;
  }
  else {
    GetMargins().Left = GetMargins().Right = TUIMetric::CxFixedFrame;
    GetMargins().Top = GetMargins().Bottom = TUIMetric::CyFixedFrame;
  }

//  Margins.Left = Margins.Right = TUIMetric::CxSizeFrame + 2;  // (6) fixed?
//  Margins.Top = Margins.Bottom = TUIMetric::CyFixedFrame;     // (2) fixed?

  // Toolbars default to having tooltips
  //
  SetWantTooltip(true);

  TRACEX(OwlWin, OWL_CDLEVEL, "TControlBar constructed @" << (void*)this);
}

//
//
//
TControlBar::~TControlBar()
{
  TRACEX(OwlWin, OWL_CDLEVEL, "TControlBar destructed @" << (void*)this);
}

} // OWL namespace
/* ========================================================================== */


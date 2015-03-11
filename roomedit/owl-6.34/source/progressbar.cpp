//
/// \file progressbar.cpp
/// Encapsulation for Windows ProgressBar common control
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright © 2010-2011 Jogy 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>
#include <owl/progressbar.h>
#include <owl/commctrl.h>
#include <owl/uimetric.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
/// Simplified constructor for a TProgressBar object. Creates a horizontal LED progressbar
///
TProgressBar::TProgressBar(TWindow*        parent,
               int             id,
               int x, int y, int w, int h,
               TModule*        module)
:
  TControl(parent, id, _T(""), x, y, w, h ? h : int(TUIMetric::CyVScroll), module)
{
  Min = 0;
  Max = 100;
  Step = 10;
  Value = 0;
  Margin = TUIMetric::CxBorder;
  Attr.Style &= ~WS_TABSTOP;
}

//
// Constructor for a resource gauge object.
//
TProgressBar::TProgressBar(TWindow*        parent,
               int             resId,
               TModule*        module)
:
  TControl(parent, resId, module)
{
  Min = 0;
  Max = 100;
  Step = 10;
  Value = 0;
  Margin = TUIMetric::CxBorder;
}

//
/// Returns the class name of the native control 
//
TWindow::TGetClassNameReturnType
TProgressBar::GetClassName()
{
  return PROGRESS_CLASS;
}

//
/// Sets the Min and Max data members to minValue and maxValue values returned by the
/// constructor. If Max is less than or equal to Min, SetRange resets Max to Min + 1.
//
void
TProgressBar::SetRange(int minValue, int maxValue)
{
  if (maxValue <= minValue)
    maxValue = minValue+1;

  if (GetHandle()) {
    if (!SendMessage(PBM_SETRANGE, 0, MkParam2(minValue, maxValue)))
      return;
  }

  Min = minValue;
  Max = maxValue;
}

//
/// Sets the BarColor data member to the value specified in color.
//
void
TProgressBar::SetColor(const TColor& color)
{
  if(GetHandle() && TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    SendMessage(PBM_SETBARCOLOR,0,TParam2(color==TColor::None ?
                (COLORREF)CLR_DEFAULT : (COLORREF)color));
}

//
void
TProgressBar::SetBkgndColor(const TColor& color)
{
  if(GetHandle() && TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    SendMessage(PBM_SETBKCOLOR,0,TParam2(color==TColor::None ?
                (COLORREF)CLR_DEFAULT : (COLORREF)color));
}

//
/// Sets the Step amount of the gauge for StepIt operations
//
void
TProgressBar::SetStep(int step)
{
  if (GetHandle())
    SendMessage(PBM_SETSTEP, step);

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
TProgressBar::SetValue(int value)
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
      SendMessage(PBM_SETPOS, value);
    }
    Value = value;
  }
}

//
/// Changes the value of the gauge by the given delta.
//
void
TProgressBar::DeltaValue(int delta)
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
      Value = (int)SendMessage(PBM_DELTAPOS, delta) + delta; // Take oportunity to sync
      return;  // Bypass Value update below
  }

  Value += delta;
}

//
/// Adjusts the active gauge value by the Step increment. If the new value exceeds
/// the Max value of the gauge, StepIt wraps the setting of the gauge to its Min
/// value.
//
void
TProgressBar::StepIt()
{
  if (GetHandle()) {
    SendMessage(PBM_STEPIT);
  }
  else {
    if (Value + Step < Max)
      DeltaValue(Step);
    else
      SetValue(Min);
  }
}

//
/// If a system control is being used, updates it to match our member settings.
//
void
TProgressBar::SetupWindow()
{
  TControl::SetupWindow();
    SendMessage(PBM_SETRANGE, 0, MkParam2(Min, Max));
    SendMessage(PBM_SETSTEP, Step);
    SendMessage(PBM_SETPOS, Value);
/*
    if(TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4){
      if(BarColor != TColor::None)
        SendMessage(PBM_SETBARCOLOR,0,TParam2(BarColor));
      if(BkgndColor != TColor::None)
        SendMessage(PBM_SETBKCOLOR,0,TParam2(BkgndColor));
    }
*/
}


} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Defines TMenuGadget
///
/// A TMenuGadget is a text gadget that when pressed, it acts as a popup menu.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/menugadg.h>

namespace owl {

OWL_DIAGINFO;

const int MaxMenuTextLen = 512;

//
/// Creates the pop-up menu and initializes the text gadget.
//
TMenuGadget::TMenuGadget(TMenu& menu, TWindow* window, int id,
                         TBorderStyle borderStyle, LPTSTR text, TFont* font)
:
  TTextGadget(id, borderStyle, TTextGadget::Left, 15, text, font),
  CmdTarget(window)
{
  SetShrinkWrap(true, true);
  PopupMenu = new TPopupMenu(menu);

  // initialize Text data member
  //
  tchar menuText[MaxMenuTextLen];
  if (PopupMenu->GetMenuString(0, menuText, MaxMenuTextLen, MF_BYPOSITION))
    SetText(menuText);
}

//
/// Deletes the allocated pop-up menu.
//
TMenuGadget::~TMenuGadget()
{
  delete PopupMenu;
}

//
/// Shows a pop-up menu on LButtonDown.
//
void
TMenuGadget::LButtonDown(uint modKeys, const TPoint& p)
{
  TGadget::LButtonDown(modKeys, p);
  TRect rect = GetBounds();
//  GetInnerRect(rect);
  TPoint p2(rect.TopLeft());
  GetGadgetWindow()->ClientToScreen(p2);
  PopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, p2, 0, *CmdTarget);
}

//
//
//
void
TMenuGadget::GetDesiredSize(TSize& size)
{
  // for flat style set borders minimum 1
  if((GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard) &&
      GetBorderStyle()==None && Borders.Left == 0 && Borders.Top == 0 &&
      Borders.Right==0 && Borders.Bottom==0)
    Borders.Left = Borders.Top = Borders.Right = Borders.Bottom = 1;

  TTextGadget::GetDesiredSize(size);
}

//
//
//
void
TMenuGadget::PaintBorder(TDC& dc)
{
  if(GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard){
    TBorderStyle oldStyle = BorderStyle;
    if(GetEnabled() && IsHaveMouse())
      BorderStyle = Raised;
    TTextGadget::PaintBorder(dc);
    BorderStyle = oldStyle;
  }
  else
    TTextGadget::PaintBorder(dc);
}

//
//
//
TColor
TMenuGadget::GetEnabledColor() const
{
  if((GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatHotText) && IsHaveMouse())
     return TColor::LtBlue;
  return TColor::SysBtnText;
}

} // OWL namespace
/* ========================================================================== */


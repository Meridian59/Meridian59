// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// gadgctrl.cpp: implementation file
// Version:      1.5
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owl/tooltip.h>
#include <owl/gadgetwi.h>
#include <owl/uihelper.h>

#ifdef CTXHELP
#include <owlext/ctxhelpm.h>
#endif

#include <owlext/util.h>
#include <owlext/harborex.h>
#include <owlext/dockingex.h>
#include <owlext/gadgctrl.h>

using namespace owl;

namespace OwlExt {
// ************************* TGadgetComboBox **********************************

bool TGadgetComboBox::AutoMainWindowNotify = true;
uint TGadgetComboBox::GadgetComboBoxMessage = 0;

TGadgetComboBox::TGadgetComboBox(TWindow* parent, int id, int x, int y,
                 int w, int h, uint32 style, uint textLimit, TModule* module):
TComboBox(parent, id, x, y, w, h, style, textLimit, module)
{
  OldIdx = -1;
  OldString = 0;
  MouseInControl = false;
  hwndEdit = 0;
  GadgetComboBoxMessage = ::RegisterWindowMessage(TGadgetComboBoxMessage);
}

TGadgetComboBox::~TGadgetComboBox()
{
  delete[] OldString;
  Destroy(IDCANCEL);
}

void TGadgetComboBox::SetupWindow()
{
  TComboBox::SetupWindow();
  TComboBox::SetExtendedUI(true);
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);

  if (!((Attr.Style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST))
    hwndEdit = GetWindow(GW_CHILD);
}

HBRUSH TGadgetComboBox::EvCtlColor(HDC hDC, HWND hWndChild, uint ctlType)
{
  PRECONDITION(ctlType == CTLCOLOR_EDIT);

  // Add a toottip for the edit window of the combo box
  // This code is stolen from Jo Parrello. Very special thanks.
  //
  if (hwndEdit) {
    TWindow* tWin = GetParentO();
    if (tWin) {
      TGadgetWindow* gWin = TYPESAFE_DOWNCAST(tWin, TGadgetWindow);
      if (gWin) {
        TTooltip* tTip = gWin->GetTooltip();
        owl::tstring text = GetHintText(this, GetId());
        if (tTip && text.length()) {
          TToolInfo toolInfo(gWin->GetHandle(), hwndEdit);
          toolInfo.SetText(text.c_str());
          tTip->AddTool(toolInfo);
        }
      }
    }
  }
  return TComboBox::EvCtlColor(hDC, hWndChild, ctlType);
}

void TGadgetComboBox::Ok()
{
  if (!((Attr.Style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)) {

    // if the edit field is empty send a cancel command
    //
    if (GetWindowTextLength() <= 0) {
      Cancel();
      return;
    }
  }

  if (Attr.Style & CBS_DROPDOWNLIST)
    Invalidate(false);

  if (AutoMainWindowNotify) {
    TFrameWindow* win = GetApplication()->GetMainWindow();
    win->GetClientWindow()->SetFocus();
    win->PostMessage(GadgetComboBoxMessage, (WPARAM)Attr.Id, CBN_SELENDOK);
  }
}

void TGadgetComboBox::Cancel()
{
  if (AutoMainWindowNotify) {
    if (!((Attr.Style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)) {
      if (OldString != 0)
        TComboBox::SetText(OldString);
    }
    else {
      if (OldIdx != -1)
        TComboBox::SetSelIndex(OldIdx);
    }
  }

  if (Attr.Style & CBS_DROPDOWNLIST)
    Invalidate(false);

  if (AutoMainWindowNotify) {
    TFrameWindow* win = GetApplication()->GetMainWindow();
    win->GetClientWindow()->SetFocus();
    win->PostMessage(GadgetComboBoxMessage, (WPARAM)Attr.Id, CBN_SELENDCANCEL);
  }
}

/*******************************************************************************
*   This method checks if mouse is inside our combobox.                        *
*   Then we draw the borders of the combobox according to this checking.       *
*******************************************************************************/
bool TGadgetComboBox::IdleAction(long idleCount)
{
  // Most of this code is stolen from Jo Parrello. Very special thanks
  //
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(GetParentO(),
    TDockableControlBarEx);
  if (idleCount == 0 && IsWindowEnabled() &&  dcb &&
    (dcb->GetFlatStyle()&TGadgetWindow::FlatStandard)) {
      TPoint point;
      GetCursorPos(point);
      ScreenToClient(point);
      TRect rect = GetClientRect();
      if (PtInRect(&rect, point)) {
        if (MouseInControl == false) {
          MouseInControl = true;
          Invalidate(false);
        }
      }
      else {
        if (MouseInControl == true) {
          MouseInControl = false;
          Invalidate(false);
        }
      }
  }
  return false;
}

void TGadgetComboBox::EvSetFocus(HWND hWndLostFocus)
{
  THarborManagement* hm = TYPESAFE_DOWNCAST(GetApplication(), THarborManagement);
  if (hm && hm->GetHarbor() && hm->GetHarbor()->IsCustomizeMode()) {

    // If we are in Customize mode we don't get the focus
    //
    GetApplication()->GetMainWindow()->SetFocus();
  }
  else {
    if (!((Attr.Style & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)) {
      delete[] OldString;
      OldString = new TCHAR [TextLimit];
      TComboBox::GetText(OldString, TextLimit);
    }
    else {
      OldIdx = TComboBox::GetSelIndex();
    }
    TComboBox::EvSetFocus(hWndLostFocus);
  }
}

void TGadgetComboBox::CbnCloseUp()
{
  TPoint mouse;
  GetCursorPos(mouse);
  TRect WindowRect = GetWindowRect();
  if (WindowRect.Contains(mouse) == false) {

    if (Attr.Style & CBS_DROPDOWNLIST)
      Invalidate(false);

    if (AutoMainWindowNotify)
      GetApplication()->GetMainWindow()->GetClientWindow()->SetFocus();
  }
}

void TGadgetComboBox::CbnSelChange()
{
  if (Attr.Style & CBS_DROPDOWNLIST)
    Invalidate(false);
  if (AutoMainWindowNotify && GetApplication() &&
    GetApplication()->GetMainWindow())
    GetApplication()->GetMainWindow()->PostMessage(GadgetComboBoxMessage,
    (WPARAM)Attr.Id, CBN_SELCHANGE);
}

bool TGadgetComboBox::PreProcessMsg(MSG& msg)
{
  TPoint mouse;
  TRect WindowRect;
  switch (msg.message) {
  case WM_KEYDOWN:
    if (msg.wParam == VK_RETURN) {
      Ok();
      return true; // stop default processing
    }
    if (msg.wParam == VK_ESCAPE) {
      Cancel();
      return true; // stop default processing
    }
    break;

  case WM_RBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
#ifdef CTXHELP
    TCtxHelpFileManager* ctxHelpM = TYPESAFE_DOWNCAST(GetApplication(),
      TCtxHelpFileManager);
    if (ctxHelpM && ctxHelpM->IsContextHelp()) {
      ctxHelpM->WinHelp(GetId());
      return true; // stop default processing
    }
#endif
    break;
  }
  return TComboBox::PreProcessMsg(msg);
}

/*******************************************************************************
*   This method draws the border of the combobox to make it "flat".            *
*******************************************************************************/
void TGadgetComboBox::RedrawBorder()
{
  // This code is stolen from Jo Parrello. Very special thanks
  //
  TWindowDC mydc(GetHandle());
  TPoint point;
  GetCursorPos(point);
  ScreenToClient(point);
  TRect rect = GetClientRect();

  bool customizeMode = false;
  bool customizeGadget = false;
  THarborManagement* hm = TYPESAFE_DOWNCAST(GetApplication(), THarborManagement);
  if (hm && hm->GetHarbor() && hm->GetHarbor()->IsCustomizeMode()) {
    customizeMode = true;
    if (hm->GetHarbor()->GetCustGadgetType() == THarborEx::IsToolbarGadget)
      if (hm->GetHarbor()->GetCustGadget() &&
        hm->GetHarbor()->GetCustGadget()->GetId() == GetId())
        customizeGadget = true;
  }

  // Paint black border direct into the control
  //
  if (customizeGadget) {
    TBrush brush (TColor::Black);
    mydc.FrameRect(rect,brush);
    rect.Inflate(-1, -1);
    mydc.FrameRect(rect,brush);
  }
  else {

    // We draw flat style combobox
    //
    TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(GetParentO(),
      TDockableControlBarEx);
    if (dcb && (dcb->GetFlatStyle()&TGadgetWindow::FlatStandard)) {
      if (MouseInControl && customizeMode == false && IsWindowEnabled()) {

        // We draw the "tracked" situation
        //
        TBrush brush (TColor::Sys3dFace);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        rect = GetClientRect();
        ::InflateRect (&rect, -1, -1);
        TUIBorder border(rect, TUIBorder::TStyle(TUIBorder::Recessed));
        border.Paint(mydc);
      }
      else {

        // We draw the "untracked" situation
        //
        TBrush brush (TColor::Sys3dFace);
        TBrush brush2 (TColor::SysWindow);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);

        if (IsWindowEnabled()) {
          mydc.FrameRect(rect,brush);
          ::InflateRect (&rect, -1, -1);
          mydc.FrameRect(rect,brush2);
        }
        else {

          // We draw the white border one pixel more outside than in
          // the untracked situation, because we can't draw inside the
          // edit control of the disabled combobox. This looks not
          // exactly MS-Office style, but good enough
          //
          mydc.FrameRect(rect,brush2);
        }

        int thumbx = ::GetSystemMetrics(SM_CXHTHUMB);
        rect.left = rect.right - thumbx + 1;
        mydc.FrameRect(rect,brush2);
      }
    }
  }
}

/*******************************************************************************
*   This method paints the combobox and then the flat borders.                 *
*******************************************************************************/
void TGadgetComboBox::EvPaint()
{
  TComboBox::EvPaint();
  RedrawBorder();
}

DEFINE_RESPONSE_TABLE1(TGadgetComboBox, TComboBox)
  EV_NOTIFY_AT_CHILD(CBN_SELCHANGE, CbnSelChange),
  EV_NOTIFY_AT_CHILD(CBN_CLOSEUP, CbnCloseUp),
  EV_WM_SETFOCUS,
  EV_WM_PAINT,
  EV_WM_CTLCOLOREDIT(EvCtlColor),
END_RESPONSE_TABLE;

// ******************** TGadgetEdit *******************************************

bool TGadgetEdit::AutoMainWindowNotify = true;
uint TGadgetEdit::GadgetEditMessage = 0;

TGadgetEdit::TGadgetEdit(TWindow* parent, int id, LPCTSTR text,
             int x, int y, int w, int h, uint textLimit, bool multiline, TModule* module):
TEdit(parent, id, text, x, y, w, h, textLimit, multiline, module)
{
  OldString = 0;
  MouseInControl = false;
  GadgetEditMessage = ::RegisterWindowMessage(TGadgetEditMessage);
}

TGadgetEdit::~TGadgetEdit()
{
  delete[] OldString;
  Destroy(IDCANCEL);
}

void TGadgetEdit::Ok()
{
  // if the edit field is empty send a cancel command
  //
  if (GetWindowTextLength() <= 0) {
    Cancel();
    return;
  }

  if (AutoMainWindowNotify) {
    TFrameWindow* win = GetApplication()->GetMainWindow();
    win->GetClientWindow()->SetFocus();
    win->PostMessage(GadgetEditMessage, (WPARAM)Attr.Id, 0);
  }
}

void TGadgetEdit::Cancel()
{
  if (OldString != 0)
    TEdit::SetText(OldString);

  if (AutoMainWindowNotify)
    GetApplication()->GetMainWindow()->GetClientWindow()->SetFocus();
}

bool TGadgetEdit::IdleAction(long idleCount)
{
  // Most of this code is stolen from Jo Parrello. Very special thanks
  //
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(GetParentO(),
    TDockableControlBarEx);
  if (idleCount == 0 && IsWindowEnabled() && dcb &&
    (dcb->GetFlatStyle()&TGadgetWindow::FlatStandard)) {
      TPoint point;
      GetCursorPos(point);
      ScreenToClient(point);
      TRect rect = GetClientRect();
      if (PtInRect(&rect, point)) {
        if (MouseInControl == false) {
          MouseInControl = true;
          Invalidate(false);
        }
      }
      else {
        if (MouseInControl == true) {
          MouseInControl = false;
          Invalidate(false);
        }
      }
  }
  return false;
}

void TGadgetEdit::SetupWindow()
{
  TEdit::SetupWindow();
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);
}

bool TGadgetEdit::PreProcessMsg(MSG& msg)
{
  TPoint mouse;
  TRect WindowRect;
  switch (msg.message) {
  case WM_KEYDOWN:
    if (msg.wParam == VK_RETURN) {
      Ok();
    }
    if (msg.wParam == VK_ESCAPE) {
      Cancel();
    }
    break;

  case WM_RBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
#ifdef CTXHELP
    TCtxHelpFileManager* ctxHelpM = TYPESAFE_DOWNCAST(GetApplication(),
      TCtxHelpFileManager);
    if (ctxHelpM && ctxHelpM->IsContextHelp()) {
      ctxHelpM->WinHelp(GetId());
      return true; // stop default processing
    }
#endif
    break;
  }
  return TEdit::PreProcessMsg(msg);
}

/*******************************************************************************
* This method draws the border of the edit field to make it "flat".            *
*******************************************************************************/
void TGadgetEdit::RedrawBorder()
{
  // This code is stolen from Jo Parrello. Very special thanks
  //
  TWindowDC mydc(GetHandle());
  TPoint point;
  GetCursorPos(point);
  ScreenToClient(point);
  TRect rect = GetClientRect();

  bool customizeMode = false;
  bool customizeGadget = false;
  THarborManagement* hm = TYPESAFE_DOWNCAST(GetApplication(), THarborManagement);
  if (hm && hm->GetHarbor() && hm->GetHarbor()->IsCustomizeMode()) {
    customizeMode = true;
    if (hm->GetHarbor()->GetCustGadgetType() == THarborEx::IsToolbarGadget)
      if (hm->GetHarbor()->GetCustGadget() &&
        hm->GetHarbor()->GetCustGadget()->GetId() == GetId())
        customizeGadget = true;
  }

  // Paint black border direct into the control
  //
  if (customizeGadget) {
    TBrush brush (TColor::Black);
    mydc.FrameRect(rect,brush);
    rect.Inflate(-1, -1);
    mydc.FrameRect(rect,brush);
  }
  else {

    // We draw flat style edit field
    //
    TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(GetParentO(),
      TDockableControlBarEx);
    if (dcb && (dcb->GetFlatStyle()&TGadgetWindow::FlatStandard)) {
      if (MouseInControl && customizeMode == false && IsWindowEnabled()) {

        // We draw the "tracked" situation
        //
        TBrush brush (TColor::Sys3dFace);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        rect = GetClientRect();
        ::InflateRect (&rect, -1, -1);
        TUIBorder border(rect, TUIBorder::TStyle(TUIBorder::Recessed));
        border.Paint(mydc);
      }
      else {

        // We draw the "untracked" situation
        //
        TBrush brush (TColor::Sys3dFace);
        mydc.FrameRect(rect,brush);
        ::InflateRect (&rect, -1, -1);
        mydc.FrameRect(rect,brush);
        if (IsWindowEnabled()) {
          rect.bottom--;
          mydc.FrameRect(rect,brush);
          ::InflateRect (&rect, -1, -1);
        }
        else {
          rect.bottom--;
          rect.right--;
        }
        TBrush brush2 (TColor::SysWindow);
        mydc.FrameRect(rect,brush2);
      }
    }
    else {
      // We draw the "tracked" situation
      //
      rect = GetClientRect();
      TUIBorder border(rect, TUIBorder::TStyle(TUIBorder::WndRecessed));
      border.Paint(mydc);
    }
  }
}

void TGadgetEdit::EvPaint()
{
  TEdit::EvPaint();
  RedrawBorder();
}

void TGadgetEdit::EvSetFocus(HWND hWndLostFocus)
{
  THarborManagement* hm = TYPESAFE_DOWNCAST(GetApplication(), THarborManagement);
  if (hm && hm->GetHarbor() && hm->GetHarbor()->IsCustomizeMode()) {

    // If we are in Customize mode we don't get the focus
    //
    if (GetApplication()->GetMainWindow())
      GetApplication()->GetMainWindow()->SetFocus();
  }
  else {
    delete[] OldString;
    OldString = new TCHAR [TextLimit];
    TEdit::GetText(OldString, TextLimit);
    TEdit::EvSetFocus(hWndLostFocus);
  }
}

DEFINE_RESPONSE_TABLE1(TGadgetEdit, TEdit)
EV_WM_PAINT,
EV_WM_SETFOCUS,
END_RESPONSE_TABLE;

} // OwlExt namespace
//======================================================================================

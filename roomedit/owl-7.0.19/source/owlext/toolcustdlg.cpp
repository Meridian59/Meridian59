// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// toolcustdlg.cpp: implementation file
// Version:         1.6
// Date:            08.11.1998
// Author:          Dieter Windau
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
//
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owl/inputdia.rh>

#include <owlext\util.h>
#include <owlext\harborex.h>
#include <owlext\gadgetex.h>
#include <owlext\toolcustdlg.h>
#include <owlext\menubtn.h>

#include <owl/propsht.rh>
#include <owlext\dockingex.rh>

using namespace owl;

namespace OwlExt {

// ****************** TToolbarInputDialog *************************************

TToolbarInputDialog::TToolbarInputDialog(THarborEx* harbor, TWindow* parent,
                     LPCTSTR title, LPCTSTR prompt, LPTSTR buffer, int bufferSize,
                     TModule* module):
TInputDialog(parent, title, prompt, buffer, bufferSize, module)
{
  Harbor = harbor;
}

TToolbarInputDialog::~TToolbarInputDialog()
{
}

void TToolbarInputDialog::EnableOkButton()
{
  GetDlgItemText(ID_INPUT, Buffer, BufferSize);
  ::EnableWindow(GetDlgItem(IDOK), (*Buffer == 0) ? false : true);
}

void TToolbarInputDialog::CmOk()
{
  GetDlgItemText(ID_INPUT, Buffer, BufferSize);
  if (Harbor->IsDCBTitleValid(Buffer) == false)
  {
    MessageBox(IDS_TOOLBAREXIST, GetModule()->GetName(), MB_OK | MB_ICONEXCLAMATION);
  }
  else
    TInputDialog::CmOk();
}

DEFINE_RESPONSE_TABLE1(TToolbarInputDialog, TInputDialog)
EV_EN_CHANGE(ID_INPUT, EnableOkButton),
EV_BN_CLICKED(IDOK, CmOk),
END_RESPONSE_TABLE;

// ****************** TToolbarPage ********************************************

DEFINE_RESPONSE_TABLE1(TToolbarPage, TPropertyPage)
EV_CHILD_NOTIFY(IDC_LB_TOOLBARS, CBN_SELCHANGE, CmToolbar),
EV_BN_CLICKED(IDC_CB_SHOWTOOLTIPS, CmShowTooltips),
EV_BN_CLICKED(IDC_CB_WITHSHORTCUTKEYS, CmWithAccel),
EV_BN_CLICKED(IDC_CB_SHOWGRIPPER, CmDrawGripper),
EV_BN_CLICKED(IDC_CB_FLATSTYLE, CmFlatStyle),
EV_BN_CLICKED(IDC_RB_NOHINTS, CmNoHints),
EV_BN_CLICKED(IDC_RB_PRESSHINTS, CmPressHints),
EV_BN_CLICKED(IDC_RB_ENTERHINTS, CmEnterHints),
EV_BN_CLICKED(IDC_PB_ADDTOOLBAR, CmAddToolbar),
EV_BN_CLICKED(IDC_PB_DELETETOOLBAR, CmDeleteToolbar),
EV_BN_CLICKED(IDC_PB_RENAMETOOLBAR, CmRenameToolbar),
EV_BN_CLICKED(IDC_PB_RESETTOOLBAR, CmResetToolbar),
END_RESPONSE_TABLE;

TToolbarPage::TToolbarPage(TToolbarCustomizeSheet* parent, TResId iconRes,TModule* module)
:
TPropertyPage(parent, IDD_TOOLBARS, 0, iconRes, module),
CheckListItems(5,0,5)
{
  PRECONDITION(parent->Harbor);
  Harbor = parent->Harbor;
  for (uint i=0; i<Harbor->DCBData.GetItemsInContainer(); i++) {
    TCheckListItem* item = new TCheckListItem(
      Harbor->DCBData[i]->Title, Harbor->DCBData[i]->DCB->IsWindowVisible());
    CheckListItems.Add(item);
  }
  CheckList = new TCheckList(this, IDC_LB_TOOLBARS, CheckListItems);
  PageInfo.dwFlags &= ~PSP_HASHELP;
}

TToolbarPage::~TToolbarPage()
{
}

void TToolbarPage::Update()
{
  if (Harbor->DCBData.GetItemsInContainer() ==
    CheckListItems.GetItemsInContainer()) {
      for (uint i=0; i<CheckListItems.GetItemsInContainer(); i++) {
        if (Harbor->DCBData[i]->DCB->IsWindowVisible() == true)
          CheckListItems[i]->Check();
        else
          CheckListItems[i]->Uncheck();
      }
      CheckList->Invalidate();
      CheckList->UpdateWindow();
  }
}

void TToolbarPage::SetupWindow()
{
  TPropertyPage::SetupWindow();
  CheckList->SetSelIndex(0);

  CheckDlgButton(IDC_CB_SHOWTOOLTIPS, Harbor->IsShowTooltips());
  CheckDlgButton(IDC_CB_WITHSHORTCUTKEYS, Harbor->IsWithAccel());
  CheckDlgButton(IDC_CB_SHOWGRIPPER, Harbor->IsDrawGripper());
  if (Harbor->DCBData.GetItemsInContainer() > 0)
    CheckDlgButton(IDC_CB_FLATSTYLE, Harbor->DCBData[0]->DCB->GetFlatStyle() & TGadgetWindow::FlatStandard);
  else
    CheckDlgButton(IDC_CB_FLATSTYLE, TRUE);
  switch (Harbor->GetHintMode()) {
  case TGadgetWindow::NoHints:    CheckDlgButton(IDC_RB_NOHINTS, true);
    break;
  case TGadgetWindow::PressHints: CheckDlgButton(IDC_RB_PRESSHINTS, true);
    break;
  case TGadgetWindow::EnterHints: CheckDlgButton(IDC_RB_ENTERHINTS, true);
    break;
  }
  DisableButtons();
}

void TToolbarPage::CmToolbar()
{
  int idx = CheckList->GetSelIndex();
  TCheckListItem* item = CheckList->GetItem(idx);
  if (item->IsChecked() != Harbor->DCBData[idx]->DCB->IsWindowVisible()) {
    if (item->IsChecked())
      Harbor->DCBData[idx]->DCB->Show();
    else
      Harbor->DCBData[idx]->DCB->Hide();
  }
  DisableButtons();
}

void TToolbarPage::CmShowTooltips()
{
  Harbor->SetShowTooltips(Harbor->IsShowTooltips() ? false : true);
}

void TToolbarPage::CmWithAccel()
{
  Harbor->SetWithAccel(Harbor->IsWithAccel() ? false : true);
}

void TToolbarPage::CmDrawGripper()
{
  Harbor->SetDrawGripper(Harbor->IsDrawGripper() ? false : true);
}

void TToolbarPage::CmFlatStyle()
{
  if (Harbor->DCBData.GetItemsInContainer() > 0)
    TGadgetWindow::EnableFlatStyle(Harbor->DCBData[0]->DCB->GetFlatStyle() == TGadgetWindow::FlatStandard ? TGadgetWindow::NonFlatNormal : TGadgetWindow::FlatStandard);
  else
    TGadgetWindow::EnableFlatStyle(TGadgetWindow::FlatStandard);

  for (uint i=0; i<Harbor->DCBData.GetItemsInContainer(); i++) {
    Harbor->DCBData[i]->DCB->Invalidate();
    Harbor->DCBData[i]->DCB->UpdateWindow();
  }
}

void TToolbarPage::CmNoHints()
{
  Harbor->SetHintMode(TGadgetWindow::NoHints);
}

void TToolbarPage::CmPressHints()
{
  Harbor->SetHintMode(TGadgetWindow::PressHints);
}

void TToolbarPage::CmEnterHints()
{
  Harbor->SetHintMode(TGadgetWindow::EnterHints);
}

void TToolbarPage::CmAddToolbar()
{
  TCHAR title[255], prompt[255], buffer[MaxTitleLen];
  *buffer = 0;
  GetModule()->LoadString(IDS_NEWTOOLBAR, title, 255);
  GetModule()->LoadString(IDS_PROMPTTOOLBAR, prompt, 255);
  if (TToolbarInputDialog(Harbor, this, title, prompt,
    buffer, MaxTitleLen).Execute() == IDOK) {

      // Create new TCheckListBoxItem and insert them
      //
      TCheckListItem* item = new TCheckListItem(&buffer[0], true);
      int id = Harbor->GetFreeDCBId();
      if (id != -1) {
        // Create a new non default TDockableControlBarEx with title = buffer,
        // and no help, insert a dummy gadget and move them to the top left corner
        // of the TDecoratedFrame child window
        //
        TDockableControlBarEx* dcb = new TDockableControlBarEx(
          id, buffer, &Harbor->DecoratedFrame, false);
        dcb->Insert(*new TInvisibleGadgetEx());
        TRect rect = Harbor->DecoratedFrame.GetClientWindow()->GetWindowRect();
        Harbor->Insert(*dcb, alNone, &rect.TopLeft());
        Harbor->GetDCBData();

        for (uint i=0; i<Harbor->DCBData.GetItemsInContainer(); i++) {
          if (Harbor->DCBData[i]->DCB->GetId() == id) {
            CheckList->InsertItem(item, i);
            CheckListItems.AddAt(item, i);
            CheckList->SetSelIndex(i);
            DisableButtons();
            return;
          }
        }
      }
  }
}

void TToolbarPage::CmDeleteToolbar()
{
  int idx = CheckList->GetSelIndex();
  if (idx == -1)
    return;

  // Show a confirmation message box
  //
  if (MessageBox(IDS_DELETETOOLBAR, 0, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK)
  {
    CheckList->DetachItem(idx);
    CheckListItems.Destroy(idx);

    // Remove the DCB from harbor, realy delete them, and actualize DCBData
    //
    TDockableControlBarEx* dcb = Harbor->DCBData[idx]->DCB;
    Harbor->Remove(*dcb);
    delete dcb;
    Harbor->DCBData.Destroy(idx);

    if (idx > 0)
      CheckList->SetSelIndex(idx-1);
    else
      CheckList->SetSelIndex(0);
    DisableButtons();
  }
}

void TToolbarPage::CmRenameToolbar()
{
  TCHAR title[255], prompt[255], buffer[MaxTitleLen];
  int idx = CheckList->GetSelIndex();
  if (idx == -1)
    return;

  _tcscpy(buffer, Harbor->DCBData[idx]->Title);
  GetModule()->LoadString(IDS_RENAMETOOLBAR, title, 255);
  GetModule()->LoadString(IDS_PROMPTTOOLBAR, prompt, 255);
  if (TToolbarInputDialog(Harbor, this, title, prompt,
    buffer, MaxTitleLen).Execute() == IDOK) {

      CheckListItems[idx]->SetText(buffer);
      CheckList->Invalidate();
      CheckList->UpdateWindow();

      _tcscpy(Harbor->DCBData[idx]->Title, buffer);
      Harbor->DCBData[idx]->DCB->SetCaption(buffer);

      if (Harbor->DCBData[idx]->Slip != 0) {

        // If the DCB is in a floating slip and this slip is visible, set the
        // caption of the floating slip too, because OWL don't synchronize them
        //
        TFloatingSlipEx* floatSlip = TYPESAFE_DOWNCAST(
          Harbor->DCBData[idx]->Slip, TFloatingSlipEx);
        if (floatSlip && floatSlip->IsWindowVisible())
          floatSlip->SetCaption(buffer);
      }
      DisableButtons();
  }
}

void TToolbarPage::CmResetToolbar()
{
  int idx = CheckList->GetSelIndex();
  if (idx == -1)
    return;

  if (Harbor)
    Harbor->InsertDefaultGadget(idx);
}

void TToolbarPage::DisableButtons()
{
  int idx = CheckList->GetSelIndex();
  bool state;
  if (idx >= 0 && Harbor->DCBData[idx]->DCB->IsDefault())
    state = false;
  else
    state = true;

  ::EnableWindow(GetDlgItem(IDC_PB_DELETETOOLBAR), state);
  ::EnableWindow(GetDlgItem(IDC_PB_RENAMETOOLBAR), state);
  ::EnableWindow(GetDlgItem(IDC_PB_RESETTOOLBAR), !state);
}

// ****************** TCommandsListBox ****************************************

DEFINE_RESPONSE_TABLE1(TCommandsListBox, TListBox)
EV_WM_ERASEBKGND,
EV_WM_LBUTTONDOWN,
END_RESPONSE_TABLE;

TCommandsListBox::TCommandsListBox(TCommandsPage* parent, int resourceId,
                   THarborEx* harbor, TModule* module):
TListBox(parent, resourceId, module)
{
  Harbor = harbor;
  CommandsPage = parent;
}

bool TCommandsListBox::EvEraseBkgnd(HDC hdc)
{
  TRect cRect = GetClientRect();
  TDC dc(hdc);
  dc.FillRect(cRect, TBrush(TColor::Sys3dFace));
  return true;
}

// Mouse handlers - needed for drag and drop support
//
void TCommandsListBox::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TListBox::EvLButtonUp(modKeys, point);

  int idx = GetSelIndex();
  if (idx >= 0) {
    TGadgetDesc* desc = (TGadgetDesc*)GetItemData(idx);
    if (desc) {
      Harbor->GadgetDraggingBegin();
      Harbor->SetCustGadget(desc->Id);
    }
  }
}

void TCommandsListBox::SetupWindow()
{
  TListBox::SetupWindow();
  SetItemHeight(0, 19);
}

void TCommandsListBox::DrawItem(DRAWITEMSTRUCT & drawInfo)
{
  // Prepare DC
  //
  TDC dc(drawInfo.hDC);

  TRect rect(drawInfo.rcItem);
  TBrush bkgnd(TColor::Sys3dFace);
  dc.FillRect(rect, bkgnd);
  TRect textRect = rect;

  TGadgetDesc* desc = (TGadgetDesc*)drawInfo.itemData;
  if (desc && desc->Attr & GADG_ISCOMMAND) {

    TButtonTextGadgetDesc* bgd = TYPESAFE_DOWNCAST(desc, TButtonTextGadgetDesc);
    if (bgd) {

      bool sel = (drawInfo.itemState & ODS_SELECTED) ? true : false;
      TBitmap bmp = TBitmap(GetModule()->LoadBitmap(
        bgd->BmpResId), AutoDelete);
      if ((HBITMAP)bmp) {
        TMemoryDC ddc;
        ddc.SelectObject(bmp);
        rect.left+= 2;
        rect.top++;
        dc.BitBlt(rect, ddc, TPoint(0,0));
        rect.top--;
        rect.left -= 2;

        textRect.left += bmp.Size().cx + 3;
        textRect.top++;
        textRect.bottom--;
        textRect.right--;
      }

      if (drawInfo.itemState & ODS_FOCUS){
        dc.DrawFocusRect(rect);
      }

      // Draw Text
      //
      int oldbkmode = dc.SetBkMode(TRANSPARENT);
      TColor textcol = sel ? TColor::SysHighlightText : TColor::SysWindowText;
      TColor oldtextcol = dc.SetTextColor(textcol);
      TBrush fillBrush(sel ? TColor::SysHighlight : TColor::Sys3dFace);
      dc.FillRect(textRect, fillBrush);
      dc.DrawText(bgd->Text.c_str(), -1, textRect, DT_SINGLELINE | DT_VCENTER);
      dc.SetTextColor(oldtextcol);
      dc.SetBkMode(oldbkmode);

      if (bgd->IsPopupButtonGadget()) {

        // paint separator between text and arrow
        //
        TRect arrowRect = textRect;
        arrowRect.left = arrowRect.right - 12;
        TPen pen(TColor::Sys3dShadow);
        dc.SelectObject(pen);
        dc.MoveTo(arrowRect.left-1, arrowRect.top-1);
        dc.LineTo(arrowRect.left-1, arrowRect.bottom+1);
        dc.RestorePen();
        TPen pen2(TColor::Sys3dHilight);
        dc.SelectObject(pen2);
        dc.MoveTo(arrowRect.left, arrowRect.top-1);
        dc.LineTo(arrowRect.left, arrowRect.bottom+1);
        dc.RestorePen();

        // paint arrow
        //
        int middleY = arrowRect.top + arrowRect.Height()/2;
        TPoint arrowPt[3];
        arrowPt[0].x = arrowPt[2].x = arrowRect.left+4;
        arrowPt[1].x = arrowRect.right-4;
        arrowPt[0].y = middleY-4;
        arrowPt[1].y = middleY;
        arrowPt[2].y = middleY+4;
        TBrush arrowBrush(textcol);
        TPen   arrowPen(textcol);
        dc.SelectObject(arrowBrush);
        dc.SelectObject(arrowPen);
        dc.Polygon(&arrowPt[0], 3);
        dc.RestoreBrush();
        dc.RestorePen();
      }
    }
  }
}

// ****************** TCommandsPage *******************************************

DEFINE_RESPONSE_TABLE1(TCommandsPage, TPropertyPage)
EV_CHILD_NOTIFY(IDC_LB_CATEGORY, CBN_SELCHANGE, CmCategory),
END_RESPONSE_TABLE;

TCommandsPage::TCommandsPage(TToolbarCustomizeSheet* parent, TResId iconRes,
               TModule* module):
TPropertyPage(parent, IDD_COMMANDS, 0, iconRes, module)
{
  PRECONDITION(parent->Harbor);
  Harbor = parent->Harbor;
  DescId = 0;

  CategoryListBox = new TListBox(this, IDC_LB_CATEGORY);
  CommandsListBox = new TCommandsListBox(this, IDC_LB_BUTTONS, Harbor);
  Description = new TStatic(this, IDC_DESCRIPTION, 128);
  ModifySelectionButton = new TMenuButton(this, IDC_PB_MODIFYSELECTION);

  GadgetMenu = Harbor->GetGadgetMenu();
  if (GadgetMenu && ::GetSubMenu(GadgetMenu, 0)) {
    ModifySelectionButton->SetMenu(::GetSubMenu(GadgetMenu, 0),
      GetApplication()->GetMainWindow()->GetHandle());
  }
}

TCommandsPage::~TCommandsPage()
{
  if (GadgetMenu)
    ::DestroyMenu(GadgetMenu);
}

void TCommandsPage::Update()
{
  // do nothing
}

void TCommandsPage::SetDescription(int id)
{
  if (id != DescId) {
    DescId = id;

    owl::tstring str = GetHintText(this, id, htStatus);
    if (str.length() && id != 0)
      Description->SetWindowText(str.c_str());
    else
      Description->SetWindowText(_T(""));
  }
}

void TCommandsPage::SetupWindow()
{
  PRECONDITION(GadgetDescriptors);
  TPropertyPage::SetupWindow();

  uint i, j;
  uint num = GadgetDescriptors->Categories.GetItemsInContainer();
  if (num > 0) {
    for (i=0; i<num; i++) {
      CategoryListBox->AddString(
        GadgetDescriptors->Categories[i]->Name.c_str());
    }
    ShowCommands(GadgetDescriptors->Categories[0]->Name);
  }
  else {
    ::EnableWindow(GetDlgItem(IDC_LB_CATEGORY), false);
    ::EnableWindow(GetDlgItem(IDC_CATEGORY), false);
    for (i=0, j=0; i<GadgetDescriptors->Array.GetItemsInContainer(); i++) {
      TGadgetDesc* desc = GadgetDescriptors->Array[i];
      if (desc && desc->Attr & GADG_ISCOMMAND) {
        CommandsListBox->AddString(_T(""));
        CommandsListBox->SetItemData(j, reinterpret_cast<LPARAM>(desc));
        j++;
      }
    }
  }
}

void TCommandsPage::CmCategory()
{
  TCHAR name[255];
  if (CategoryListBox->GetSelString(name, 255) > 0) {
    CommandsListBox->ClearList();
    if (Harbor->GetCustGadgetType() == THarborEx::IsRefGadget)
      Harbor->SetCustGadgetNull();
    ShowCommands(name);
  }
}

void TCommandsPage::ShowCommands(owl::tstring name)
{
  uint i,j;
  for (i=0, j=0; i<GadgetDescriptors->Array.GetItemsInContainer(); i++) {
    TGadgetDesc* desc = GadgetDescriptors->Array[i];
    if (desc && desc->Attr & GADG_ISCOMMAND) {
      TCommandCategory* cc = GadgetDescriptors->FindCategory(desc->Id);
      if (cc && cc->Name == name) {
        CommandsListBox->AddString(_T(""));
        CommandsListBox->SetItemData(j, reinterpret_cast<LPARAM>(desc));
        j++;
      }
    }
  }
}

bool TCommandsPage::IdleAction(long idleCount)
{
  if(!GetHandle())
    return false;
  switch (Harbor->GetCustGadgetType()) {
  case THarborEx::IsNoGadget:
    SetDescription(0);
    if (ModifySelectionButton->IsWindowEnabled())
      ModifySelectionButton->EnableWindow(false);
    break;

  case THarborEx::IsToolbarGadget:
    if (Harbor->GetCustGadget()) {
      if (CommandsListBox->GetSelIndex() >= 0)
        CommandsListBox->SetSelIndex(-1);
      SetDescription(Harbor->GetCustGadget()->GetId());
      if (!ModifySelectionButton->IsWindowEnabled())
        ModifySelectionButton->EnableWindow(true);
    }
    break;

  case THarborEx::IsRefGadget:
    SetDescription(Harbor->GetCustGadgetRef());
    if (!ModifySelectionButton->IsWindowEnabled())
      ModifySelectionButton->EnableWindow(true);
    break;
  }
  return TPropertyPage::IdleAction(idleCount);
}

// ****************** TToolbarCustomizeSheet **********************************

DEFINE_RESPONSE_TABLE1(TToolbarCustomizeSheet, TPropertySheet)
END_RESPONSE_TABLE;

TToolbarCustomizeSheet::TToolbarCustomizeSheet(TWindow* parent,
                         THarborEx* harbor, TModule* module):
TPropertySheet(parent, 0, 0, false,
         PSH_DEFAULT | PSH_NOAPPLYNOW | PSH_MODELESS, module)
{
  PRECONDITION(harbor);
  Harbor = harbor;
  ToolbarPage = new TToolbarPage(this);
  CommandsPage = new TCommandsPage(this);

  // set caption
  TCHAR str[255];
  GetModule()->LoadString(IDS_CUSTOMIZE, str, 255);
  SetCaption(str);
}

TToolbarCustomizeSheet::~TToolbarCustomizeSheet()
{
  delete ToolbarPage;
}

void TToolbarCustomizeSheet::Update()
{
  ToolbarPage->Update();
  CommandsPage->Update();
}

void TToolbarCustomizeSheet::SetupWindow()
{
  TPropertySheet::SetupWindow();

  CenterWindowOver(GetHandle(), GetParentO()->GetHandle());

  // Hide the ? in title bar, the OK button
  //
  ModifyExStyle(WS_EX_CONTEXTHELP,0);
  ::ShowWindow(GetDlgItem(IDOK), SW_HIDE);

  // Change Cancel button text to Close button text
  //CancelToClose();//?????????????????????????????
  TCHAR str[128];
  GetModule()->LoadString(IDS_CLOSE, str, 128);
  ::SetDlgItemText(GetHandle(), IDCANCEL, str);

  // Make Cancel Button to default push button
  //
  LONG style = ::GetWindowLong(GetDlgItem(IDCANCEL), GWL_STYLE);
  style &= ~(BS_PUSHBUTTON);
  style |= BS_DEFPUSHBUTTON;
  ::SetWindowLong(GetDlgItem(IDCANCEL), GWL_STYLE, style);

  Harbor->SetCustomizeMode(true);
}

void TToolbarCustomizeSheet::Destroy(int retVal)
{
  // Call only if harbor is valid. The Harbor is invalid if the parent
  // parent window is invalid (e.g. the application will be terminated)
  //
  if (Parent && Parent->GetHandle())
    if (Harbor/* && Harbor->DecoratedFrame*/)
      Harbor->SetCustomizeMode(false);
  TPropertySheet::Destroy(retVal);
}

} // OwlExt namespace

//==============================================================================================

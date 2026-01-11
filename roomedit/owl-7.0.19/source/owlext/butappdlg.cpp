// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// butappdlg.cpp: implementation file
// Version:       1.6
// Date:          08.11.1998
// Author:        Dieter Windau
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

#include <owl/celarray.h>

#include <owlext/gadgetex.h>
#include <owlext/butappdlg.h>
#include <owlext/util.h>

#include <owlext/dockingex.rh>


using namespace owl;

namespace OwlExt {

const int NumberOfImages = 49;


// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

// ************************** TImageListBox ***********************************

DEFINE_RESPONSE_TABLE1(TImageListBox, TListBox)
EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

TImageListBox::TImageListBox(TWindow* parent, int resourceId, TCelArray* array,
               TIntArray* idsaray, TModule* module)
               :
TListBox(parent, resourceId, module),
CelArray(array),
IdsArray(idsaray)
{
}

TImageListBox::~TImageListBox()
{
  delete CelArray;
  delete IdsArray;
}

//
// Set the  bitmaps by DWORD data. Call this function if all items are added
void TImageListBox::BuildCelArray()
{
  PRECONDITION(this);
  if(!CelArray){
    for (int i=0; i < GetCount(); i++) {
      TModule* module = FindResourceModule(Parent,GetModule(),
        TResId(reinterpret_cast<LPCTSTR>(GetItemData(i))), RT_BITMAP);
      if(module){
        TBitmap bmp(*module, TResId(reinterpret_cast<LPCTSTR>(GetItemData(i))));
        if(!CelArray)
          CelArray = new TCelArray(bmp.Size(), 0, 5, 5);
        CelArray->Add(bmp);
      }
    }
  }
  if(!CelArray)
    TXGdi::Raise(static_cast<uint>(GetItemData(0)));
  SetItemHeight(0, CelArray->CelSize().cy+4);
  SetColumnWidth(CelArray->CelSize().cx+4);
}
//
void TImageListBox::BuildIdsArray(TResId& resId, int maxitems)
{
  PRECONDITION(this);

  if(IdsArray){
    int i;
    for (i=0; i < (int)IdsArray->Size(); i++) {
      AddString(_T(""));
      SetItemData(i, (*IdsArray)[i]);
    }
    if(CelArray){
      AddString(_T(""));
      SetItemData(i, reinterpret_cast<LPARAM>(resId.GetPointerRepresentation()));
      TModule* module = FindResourceModule(Parent,GetModule(),
        resId, RT_BITMAP);
      if(module){
        TBitmap bmp(*module, resId);
        CelArray->Add(bmp);
      }
    }
  }
  else{
    AddString(_T(""));
    SetItemData(0, reinterpret_cast<LPARAM>(resId.GetPointerRepresentation()));
    for (int i=0; i < maxitems; i++) {
      AddString(_T(""));
      SetItemData(i+1, i+FIRSTBITMAP);
    }
  }
}
//
bool TImageListBox::EvEraseBkgnd(HDC hdc)
{
  PRECONDITION (this);
  TRect cRect = GetClientRect();
  TDC dc(hdc);
  dc.FillRect(cRect, TBrush(TColor::Sys3dFace));
  return true;
}
//
void TImageListBox::DrawItem(DRAWITEMSTRUCT & drawInfo)
{
  PRECONDITION (this);
  PRECONDITION (CelArray);

  // Prepare DC
  //
  TDC dc(drawInfo.hDC);

  // Erase entire line
  //
  TRect rect(drawInfo.rcItem);
  TBrush bkgnd(TColor::Sys3dFace);
  dc.FillRect(rect, bkgnd);

  CelArray->BitBlt(drawInfo.itemID, dc, rect.left+2, rect.top+2);

  if (drawInfo.itemState & ODS_DISABLED) {
    DrawDisabledButton(dc, rect);
  }
  else {

    // Draw select states
    //
    if ((drawInfo.itemState & ODS_SELECTED) &&
      !(drawInfo.itemState & ODS_DISABLED)) {

        // Draw focus states
        //
        if (drawInfo.itemState & ODS_FOCUS) {
          TPen pen(TColor::SysHighlight, 2);
          dc.SelectObject(pen);
          dc.SelectStockObject(NULL_BRUSH);
          rect.top++;
          rect.left++;
          dc.Rectangle(rect);
          dc.RestoreBrush();
          dc.RestorePen();
        }
        else {
          dc.DrawFocusRect(rect);
          rect.Inflate(-1,-1);
          dc.DrawFocusRect(rect);
        }
    }
  }
}
// ****************** TButtonAppearanceDialog *********************************
//
DEFINE_RESPONSE_TABLE1(TButtonAppearanceDialog, TDialog)
EV_BN_CLICKED(IDOK, CmOk),
EV_BN_CLICKED(IDC_RB_IMAGE, CmImage),
EV_BN_CLICKED(IDC_RB_TEXT, CmText),
EV_BN_CLICKED(IDC_RB_IMAGETEXT, CmImageText),
EV_EN_CHANGE(IDC_EDIT_BUTTONTEXT, CmButtonText),
END_RESPONSE_TABLE;
//
TButtonAppearanceDialog::TButtonAppearanceDialog(TWindow* parent,
                         TButtonTextGadgetEx* buttonTextGadget, TCelArray* celaray,
                         TIntArray* idsarray, TModule* module)
                         :
TDialog(parent, IDD_BUTTON_PROPS, module)
{
  PRECONDITION(buttonTextGadget);

  ButtonTextGadget = buttonTextGadget;

  ButtonText = ButtonTextGadget->GetCommandText();
  DisplayType = ButtonTextGadget->GetDisplayType();
  BmpResId = ButtonTextGadget->GetResId();

  ButtonEdit = new TEdit(this, IDC_EDIT_BUTTONTEXT, 255);
  ImageListBox = new TImageListBox(this, IDC_LB_IMAGES, celaray, idsarray);
}
//
void TButtonAppearanceDialog::SetupWindow()
{
  PRECONDITION (this);

  TDialog::SetupWindow();

  CenterWindowOver(GetHandle(), Parent->GetHandle());
  CheckDlgButton(IDC_RB_IMAGE+(int)DisplayType, true);
  ButtonEdit->SetText(ButtonText.c_str());
  EnableControls();

  ImageListBox->BuildIdsArray(BmpResId, NumberOfImages);
  ImageListBox->BuildCelArray();
  ImageListBox->SetSelIndex(0);
}
//
void TButtonAppearanceDialog::CmOk()
{
  PRECONDITION (this);

  TCHAR text[MAX_PATH];
  ButtonEdit->GetText(text, MAX_PATH);

  if (_tcslen(text) > 0)
    ButtonTextGadget->SetCommandText(owl::tstring(text));

  ButtonTextGadget->SetDisplayType(DisplayType);
  LPARAM data = ImageListBox->GetItemData(ImageListBox->GetSelIndex());
  ButtonTextGadget->SetResId(static_cast<int>(data));
  TDialog::CmOk();
}
//
void TButtonAppearanceDialog::CmImage()
{
  PRECONDITION (this);
  if (IsDlgButtonChecked(IDC_RB_IMAGE)){
    DisplayType = Bitmap;
    EnableControls();
  }
}
//
void TButtonAppearanceDialog::CmText()
{
  PRECONDITION (this);
  if (IsDlgButtonChecked(IDC_RB_TEXT)){
    DisplayType = Text;
    EnableControls();
  }
}
//
void TButtonAppearanceDialog::CmImageText()
{
  PRECONDITION (this);
  if (IsDlgButtonChecked(IDC_RB_IMAGETEXT)){
    DisplayType = BitmapText;
    EnableControls();
  }
}
//
void TButtonAppearanceDialog::CmButtonText()
{
  PRECONDITION (this);
  TCHAR text[MAX_PATH];
  ButtonEdit->GetText(text, MAX_PATH);
  ::EnableWindow(GetDlgItem(IDOK), _tcslen(text) > 0);
}
//
void TButtonAppearanceDialog::EnableControls()
{
  PRECONDITION (this);
  bool enableText  = DisplayType != Bitmap;
  bool enableImage = DisplayType != Text;
  ImageListBox->EnableWindow(enableImage);
  ::EnableWindow(GetDlgItem(IDC_GB_IMAGES), enableImage);
  ButtonEdit->EnableWindow(enableText);
  ::EnableWindow(GetDlgItem(IDC_TEXT_BUTTONTEXT), enableText);
}

} // OwlExt namespace
//=====================================================================================

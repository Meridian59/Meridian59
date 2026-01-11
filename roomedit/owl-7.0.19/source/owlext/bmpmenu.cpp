// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// bmpmenu.cpp: implementation file
// Version:     1.7
// Date:        04/24/1999
// Author:      Dieter Windau
//
// TBmpMenu is a freeware OWL class that shows bitmaps
// on the left side of menus like MS Office 97
//
// Portions of code are based on MFC class CMenuSpawn written by Iuri
// Apollonio and BCMenu written by Brent Corkum. Very special thanks.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// The code was tested using Microsoft Visual C++ 6.0 SR2 with OWL6 patch 5
// and Borland C++ 5.02 with OWL 5.02. Both under Windows NT 4.0 SP4.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// Many thanks to Riho Ellermaa, Michael Mogensen, Jurgen Welzenbach,
// Rob Beckers, Jo Parrello, Mark Hatsell an Yura Bidus for their help
// in testing and fixing the TBmpMenu classes.
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/softengage/index.htm
// ****************************************************************************
#include <owlext\pch.h>
#pragma hdrstop

#if  !defined(OWL_UIHELPER_H)
#include <owl/uihelper.h>
#endif
#if  !defined(OWL_IMAGELST_H)
#include <owl/imagelst.h>
#endif
#if !defined(OWL_CELARRAY_H)
#include <owl/celarray.h>
#endif
#include <owl/appdict.h>

#include <owlext/bmpmenu.h>
#include <owlext/util.h>

const int DefaultBmpMenuWidth  = 19;  // if you use default borland size,
const int DefaultBmpMenuHeight = 18;  // change the values to 20 x 20

using namespace owl;

namespace OwlExt {

DIAG_DEFINE_GROUP_INIT(OWL_INI, BmpMenu, 1, 0);



//using namespace owl;

///////////////////////////////////////////////////////////
// Internall OWLExt function
// ~~~~~~~~~~~~~~~~~~~~~~~~~
TModule* FindResourceModule(TWindow* parent, TModule* module,
              TResId resId, LPCTSTR type);


// ********************* TBmpMenuItem *****************************************

bool TBmpMenuItem::operator ==(const TBmpMenuItem& d) const
{
  return memcmp(this, &d, sizeof(d)) == 0 ? true : false;
}

// ********************* TBmpMenuImageItem ************************************

TBmpMenuImageItem::TBmpMenuImageItem(int imageIdx, int cmd)
{
  ImageIdx = imageIdx;
  Cmd = cmd;
}

bool TBmpMenuImageItem::operator ==(const TBmpMenuImageItem& d) const
{
  return (ImageIdx == d.ImageIdx && Cmd == d.Cmd) ? true : false;
}

// *************************** TBmpMapEntry ***********************************

TBmpMapEntry::TBmpMapEntry(int commandId, int bmpId)
{
  CommandId = commandId;
  BmpId = bmpId;
}

bool TBmpMapEntry::operator ==(const TBmpMapEntry& d) const
{
  return (CommandId == d.CommandId && BmpId == d.BmpId) ? true : false;
}

// ******************** TBmpMenu *********************************************

TBmpMenu::TBmpMenu(TModule* module)
: // create and initialize
bShowAccInTooltips(true),
bDrawRecessed(true),
bIsPopup(false),
ImageSize(DefaultBmpMenuWidth-1, DefaultBmpMenuHeight-1),
ImageList(new TImageList(ImageSize, ILC_COLOR4|ILC_MASK, 0, 1)),
BackBitmap(0),
RadioBitmapId(0),
CheckBitmapId(0),
MenuFont(0),
MaskColor(TColor::LtGray),
Old3dFaceColor(TColor::Sys3dFace.Rgb()),
Module(module ? module : GetApplicationObject()),
MapEntries(new TIPtrArray<TBmpMapEntry*>),
ExcludedBitmaps(new TIntArray),
MenuItems(new TIPtrArray<TBmpMenuItem*>),
ImageItems(new TIPtrArray<TBmpMenuImageItem*>)
{

  // The size of the bitmaps on the left side are based on the
  // default bitmap size. Cut the right and bottom side, because
  // it's always gray. Change the constants at the top of
  // source, if you use default Borland size (20 x 20).
  //

  //  celArray = new TCelArray(ImageSize, ILC_COLOR4, 10, 5);
  ImageList->SetBkColor(Old3dFaceColor);
  //  celArray->SetBkColor(Old3dFaceColor);

  TRACEX(BmpMenu, 0, _T("TBmpMenu constructed @") << (void*)this);
}

TBmpMenu::~TBmpMenu()
{
  delete MapEntries;
  delete ExcludedBitmaps;
  delete MenuItems;
  delete ImageItems;

  ImageList->RemoveAll();
  delete ImageList;
  delete BackBitmap;
  delete MenuFont;
  //  delete celArray;
  TRACEX(BmpMenu, 0, "TBmpMenu destructed @" << (void*)this);
}

void TBmpMenu::RemapMenu(HMENU hMenu, bool isPopup)
// Remap a simple menu created with Windows API functions or with OWL TMenu
// class to a menu with bitmaps on the left side. Should be called from e.g:
// SetupWindow(), EvInitMenu(), EvInitMenuPopup() functions and before the
// TrackPopupMenu() function.
// If the hMenu is a popupMenu set isPopup = true
{
  PRECONDITION(hMenu);
  TRACEX(BmpMenu, 0, _T("RemapMenu @") << (void*)hMenu << _T(" isPopup=") << isPopup);
  SyncronizeAllMenuItems();
  if (::IsMenu(hMenu)){
    bIsPopup = isPopup;
    RemapMenuRecursiv(hMenu);
  }
}

#define DT_DRAW_OPTIONS (DT_VCENTER|DT_CENTER|DT_SINGLELINE)
bool TBmpMenu::DrawItem(DRAWITEMSTRUCT & DIS)
// To be called from the TWindow EvDrawItem(..)
// Call the base class if it return false
{
  TColor ocr, obkc;
  bool res = false;
  if (DIS.CtlType == ODT_MENU)
  {
    if (Old3dFaceColor.Rgb() != TColor::Sys3dFace.Rgb())
    {
      // if 3dFace color change, change the background of all bitmaps in list
      //
      Old3dFaceColor = TColor(TColor::Sys3dFace.Rgb());
      // celArray->SetBkColor(Old3dFaceColor);
      ImageList->SetBkColor(Old3dFaceColor);
    }

    UINT state = DIS.itemState;
    bool bEnab =  !(state & ODS_DISABLED);
    bool bSelect = (state & ODS_SELECTED) ? true : false;
    bool bChecked = (state & ODS_CHECKED) ? true : false;
    bool bItemInfoValid;
    bool bDefault;
    bool bRadioCheck;

    bItemInfoValid = bDefault = bRadioCheck = false;
    TBmpMenuItem* pItem = (TBmpMenuItem*)DIS.itemData;
    if (pItem)
    {
      // if it is not a submenu item, get the menu item info
      //
      TMenuItemInfo mii(MIIM_STATE | MIIM_TYPE);
      if (pItem->Cmd >= 0)
      {
        if ((bItemInfoValid = ::GetMenuItemInfo((HMENU)DIS.hwndItem, DIS.itemID,
          MF_BYCOMMAND, &mii)) == true)
        {
          if (mii.fState & MFS_DEFAULT)
            bDefault = true;
          if (mii.fType & MFT_RADIOCHECK)
            bRadioCheck = true;
        }
      }

      TDC dc(DIS.hDC);

      TFont* pft = NULL;
      if (!MenuFont){
        NONCLIENTMETRICS nm;
        nm.cbSize = sizeof (NONCLIENTMETRICS);
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0)){
          // get the actual windows wide menu font
          if (bItemInfoValid && bDefault)
            nm.lfMenuFont.lfWeight = FW_BOLD;
          pft = new TFont(nm.lfMenuFont);
        }
        else
          pft = new TFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
        CHECK(pft);
        dc.SelectObject(*pft);
      }
      else
        dc.SelectObject(*MenuFont);
      int obk = dc.SetBkMode(TRANSPARENT);

      TRect rc(DIS.rcItem);
      TRect rcImage(rc);
      TRect rcText(rc);
      rcImage.right = rcImage.left + rc.Height();

      if (BackBitmap){
        // draw background bitmap
        //
        TMemoryDC tempDC(dc);
        tempDC.FillRect(rc, TBrush(TColor::SysMenu));
        tempDC.SelectObject(*BackBitmap);
        dc.FillRect(rc, TBrush(TColor::SysMenu));

        // draw bitmap tiled
        //
        int xBegin = (int)(rc.left/BackBitmap->Width());
        int xEnd   = (int)(rc.right/BackBitmap->Width())+1;
        int yBegin = (int)(rc.top/BackBitmap->Height());
        int yEnd   = (int)(rc.bottom/BackBitmap->Height())+1;
        int i,j;
        for (i = xBegin; i < xEnd; i++){
          for (j = yBegin; j < yEnd; j++) {
            tempDC.SetWindowOrg(TPoint(i*BackBitmap->Width(), j*BackBitmap->Height()));
            dc.BitBlt(rc, tempDC, rc.TopLeft(), SRCCOPY);
          }
        }
      }

      if (pItem->Cmd == -3) // is a separator
      {
        TPen pnDk(TColor::Sys3dShadow);
        TPen pnLt(TColor::Sys3dHilight);
        dc.SelectObject(pnDk);
        dc.MoveTo(rc.left + 2, rc.top + 2);
        dc.LineTo(rc.right - 2, rc.top + 2);
        dc.RestorePen();
        dc.SelectObject(pnLt);
        dc.MoveTo(rc.left + 2, rc.top + 3);
        dc.LineTo(rc.right - 2, rc.top + 3);
        dc.RestorePen();
      }
      else if (pItem->Cmd == -4){ // ia a title item
        owl::tstring cs(pItem->Text);
        TRect rcBdr(rcText);

        if (bDrawRecessed){
          // draw recessed title item
          //
          if (bSelect && bEnab){
            rcText.top ++;
            rcText.left += 2;
          }

          if (!BackBitmap)
            dc.FillRect(rcText, TBrush(TColor::SysMenu));

          if (bEnab) {
            ocr = dc.SetTextColor(TColor::SysMenuText);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
          }
          else {
            rcText.Offset(1,1);
            ocr = dc.SetTextColor(TColor::Sys3dHilight);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
            rcText.Offset(-1,-1);
            dc.SetTextColor(TColor::SysGrayText);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
          }
          dc.SetTextColor(ocr);

          if (bSelect && bEnab)
            Draw3dRect(dc, rcBdr, TColor::Sys3dShadow, TColor::Sys3dHilight);

        }
        else {
          // draw normal title item
          //
          if (!BackBitmap)
            dc.FillRect(rcText, TBrush(TColor::SysMenu));

          if (bEnab) {
            if (bSelect) {
              ocr = dc.SetTextColor(TColor::SysHighlightText);
              obkc = dc.SetBkColor(TColor::SysHighlight);
              dc.FillRect(rcText, TBrush(TColor::SysHighlight));
            }
            else
              ocr = dc.SetTextColor(TColor::SysMenuText);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
            if (bSelect)
              dc.SetBkColor(obkc);
          }
          else {
            rcText.Offset(1,1);
            dc.SetTextColor(TColor::Sys3dHilight);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
            rcText.Offset(-1,-1);
            dc.SetTextColor(TColor::SysGrayText);
            dc.DrawText(cs.c_str(), -1, rcText, DT_DRAW_OPTIONS);
          }
          dc.SetTextColor(ocr);
        }
      }
      else {
        // draw menu item
        //
        rcText.left += rcImage.Width() + 1;

        if (bSelect){
          if (pItem->ImageIdx >= 0 || bChecked) {
            dc.FillRect(rcText, TBrush(TColor::SysHighlight));
            if (bChecked)
              dc.FillRect(rcImage, TBrush(TColor::SysMenu));
          }
          else
            dc.FillRect(rc, TBrush(TColor::SysHighlight));
          ocr = dc.SetTextColor(TColor::SysHighlightText);
        }
        else{
          if (pItem->ImageIdx >= 0 || bChecked){
            if (!BackBitmap)
              dc.FillRect(rcText, TBrush(TColor::SysMenu));
            if (bChecked && bEnab){
              // draw 50% mask
              //
              rcImage.Inflate(-1, -1);
              FillMaskRect(dc, rcImage);
              rcImage.Inflate(1, 1);
            }
          }
          else if (!BackBitmap)
            dc.FillRect(rc, TBrush(TColor::SysMenu));
          ocr = dc.SetTextColor(TColor::SysMenuText);
        }

        int ay = (rcImage.Height() - ImageSize.cy) / 2;
        int ax = (rcImage.Width()  - ImageSize.cx) / 2;

        if (pItem->ImageIdx >= 0){
          if (bSelect && bEnab){
            if (bChecked)
              Draw3dRect(dc, rcImage, TColor::Sys3dShadow, TColor::Sys3dHilight);
            else
              Draw3dRect(dc, rcImage, TColor::Sys3dHilight, TColor::Sys3dShadow);
          }
          else{
            if (bChecked)
              Draw3dRect(dc, rcImage, TColor::Sys3dShadow, TColor::Sys3dHilight);
            else
              Draw3dRect(dc, rcImage, TColor::SysMenu, TColor::SysMenu);
          }

          ImageList->Draw(pItem->ImageIdx, dc, rcImage.left + ax,
            rcImage.top + ay, (bChecked) ? ILD_TRANSPARENT : ILD_NORMAL);

          if (!bEnab){
            TRect strictImgRc(rcImage.left + ax, rcImage.top + ay,
              rcImage.left + ax + ImageSize.cx, rcImage.top + ay + ImageSize.cy);
            DrawDisabledButton(dc, strictImgRc);
          }
        }
        else
        {
          if (bChecked)
          {
            rcImage.Inflate(-1,-1);
            Draw3dRect(dc, rcImage, TColor::Sys3dShadow, TColor::Sys3dHilight);

            if (bItemInfoValid && bRadioCheck)
              DrawRadioDot(dc, rcImage.left + ax, rcImage.top + ay,
              bEnab, bSelect);
            else
              DrawCheckmark(dc, rcImage.left + ax, rcImage.top + ay,
              bEnab, bSelect);
          }
        }

        owl::tstring cs1;
        owl::tstring cs(pItem->Text);
        TSize sz = dc.GetTextExtent(cs, static_cast<int>(cs.size()));
        int ay1 = (rcText.Height() - sz.cy) / 2;
        rcText.top += ay1;
        rcText.left += 2;
        rcText.right -= 15;

        auto tf = cs.find(_T("\t"));

        // \a in resource is a tab too
        //
        // if (tf == cs.npos)
        //   tf = cs.find(_T("\a"));

        // Accelerator can be \b instead of \t (NOTE: \a in .rc make a \b in res)
        //
        if (tf == cs.npos)
          tf = cs.find(_T("\b"));

        if (tf != cs.npos){
          cs1 = cs.substr(tf+1);
          cs =  cs.substr(0, tf);
          if (!bEnab){
            if (!bSelect){
              TRect rcText1(rcText);
              rcText1.Inflate(-1,-1);
              dc.SetTextColor(TColor::Sys3dHilight);
              dc.DrawText(cs.c_str(),  -1, rcText1, DT_VCENTER|DT_LEFT);
              dc.DrawText(cs1.c_str(), -1, rcText1, DT_VCENTER|DT_RIGHT);
              dc.SetTextColor(TColor::SysGrayText);
              dc.DrawText(cs.c_str(),  -1, rcText, DT_VCENTER|DT_LEFT);
              dc.DrawText(cs1.c_str(), -1, rcText, DT_VCENTER|DT_RIGHT);
            }
            else{
              dc.SetTextColor(TColor::SysMenu);
              dc.DrawText(cs.c_str(),  -1, rcText, DT_VCENTER|DT_LEFT);
              dc.DrawText(cs1.c_str(), -1, rcText, DT_VCENTER|DT_RIGHT);
            }
          }
          else{
            dc.DrawText(cs.c_str(),  -1, rcText, DT_VCENTER|DT_LEFT);
            dc.DrawText(cs1.c_str(), -1, rcText, DT_VCENTER|DT_RIGHT);
          }
        }
        else{
          if (!bEnab){
            if (!bSelect){
              TRect rcText1(rcText);
              rcText1.Inflate(-1,-1);
              dc.SetTextColor(TColor::Sys3dHilight);
              dc.DrawText(cs.c_str(), -1,  rcText1, DT_VCENTER|DT_LEFT|DT_EXPANDTABS);
              dc.SetTextColor(TColor::SysGrayText);
              dc.DrawText(cs.c_str(), -1, rcText, DT_VCENTER|DT_LEFT|DT_EXPANDTABS);
            }
            else{
              dc.SetTextColor(TColor::SysMenu);
              dc.DrawText(cs.c_str(), -1,  rcText, DT_VCENTER|DT_LEFT|DT_EXPANDTABS);
            }
          }
          else
            dc.DrawText(cs.c_str(), -1, rcText, DT_VCENTER|DT_LEFT|DT_EXPANDTABS);
        }
        dc.SetTextColor(ocr);
      }

      dc.SetBkMode(obk);
      dc.RestoreFont();
      delete pft;
    }
    res = true;
  }
  return res;
}

bool TBmpMenu::MeasureItem(MEASUREITEMSTRUCT & MIS)
// To be called from the TWindow EvMeasureItem(..)
// Call the base class if it return false
{
  bool res = false;
  if (MIS.CtlType == ODT_MENU)
  {
    TBmpMenuItem* pItem = (TBmpMenuItem*)MIS.itemData;
    if (pItem)
    {
      if (pItem->Cmd == -3) // is a separator
      {
        MIS.itemWidth  = 10;
        MIS.itemHeight = 6;
      }
      else
      {
        owl::tstring cs(pItem->Text);
        if (cs != _T(""))
        {
          TClientDC dc(0);

          // get the actual windows wide menu font
          //
          TFont* pft = 0;
          if (MenuFont == 0)
          {
            NONCLIENTMETRICS nm;
            nm.cbSize = sizeof (NONCLIENTMETRICS);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0))
              pft = new TFont(nm.lfMenuFont);
            else
              pft = new TDefaultGUIFont();
            CHECK(pft);
            dc.SelectObject(*pft);
          }
          else
            dc.SelectObject(*MenuFont);
          if (pItem->Cmd == -4) // is a title item
          {
            TRect rci(0,0,0,0);
            dc.DrawText(cs.c_str(), -1, rci, DT_CALCRECT|DT_TOP|DT_VCENTER|DT_SINGLELINE);
            MIS.itemHeight = rci.Height();
            MIS.itemWidth = rci.Width();
          }
          else
          {
            int h = GetSystemMetrics(SM_CYMENU);
            MIS.itemHeight = h>ImageSize.cy+3 ? h : ImageSize.cy+3;

            TSize sz = dc.GetTabbedTextExtent(cs, static_cast<int>(cs.size()), 0, 0);
            if (MIS.itemHeight < static_cast<uint>(sz.cy + 3))
              MIS.itemHeight = sz.cy + 3;
            MIS.itemWidth = MIS.itemHeight + 2 + sz.cx;

            // make the menu bigger, because sometimes the calculated
            // width is not enough
            //
            MIS.itemWidth += 2 * (sz.cx / static_cast<uint>(cs.size()));
          }

          dc.RestoreFont();
          delete pft;
        }
        else
        {
          MIS.itemHeight = ImageSize.cy + 3;
          MIS.itemWidth  = 100;
        }
      }
    }
    res = true;
  }
  return res;
}

bool TBmpMenu::FindKeyboardShortcut(UINT nChar, UINT, HMENU hMenu, LRESULT& lRes)
// To be called from the TWindow EvMenuChar(..)
// Call the base class if it return false
{
  PRECONDITION(hMenu);
  int nItem = ::GetMenuItemCount(hMenu);
  owl::tstring csChar;
  csChar = (_TCHAR)nChar;
  _tcslwr((_TCHAR*)csChar.c_str());
  while ((--nItem)>=0){
    UINT itemId = ::GetMenuItemID(hMenu, nItem);
    if (itemId != 0){
      TMenuItemInfo mii(MIIM_DATA|MIIM_TYPE);
      if (::GetMenuItemInfo(hMenu, nItem, TRUE, &mii)){
        if (mii.fType&MFT_OWNERDRAW){
          TBmpMenuItem* pItem = (TBmpMenuItem*)mii.dwItemData;
          if (pItem){
            owl::tstring csItem(pItem->Text);
            _tcslwr((_TCHAR*)csItem.c_str());
            const auto iAmperIdx = csItem.find(_T("&"));
            if (iAmperIdx != csItem.npos)
            {
              csItem = csItem.substr(iAmperIdx + 1, 1);
              if (csItem == csChar){
                lRes = MAKELONG((uint16)nItem, 2);
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

int TBmpMenu::PreGetHintText(HMENU hmenu, uint id, LPTSTR buf, int size,
               THintText hintType)
               // Retrieves the hint text associated with a particular Id
               // To be called from DecoratedFrame GetHintText(...)
               // Call the base class if it return 0.
{
  int numBytes = 0;

  if (hintType == htTooltip){
    if (hmenu && ::IsMenu(hmenu)) {
      // For tooltips, we first look if the same id exists on the
      // bitmap menu and use the associated menu string.
      //
      bool found = false;
      bool accValid = true;
      for (uint j = 0; j < MenuItems->GetItemsInContainer(); j++) {
        if ((*MenuItems)[j]->Cmd == (int)id){
          if (_tcslen((*MenuItems)[j]->Text) < (uint)size)
            _tcscpy(buf, (*MenuItems)[j]->Text);
          else
          {
            _tcsncpy(buf, (*MenuItems)[j]->Text, size-1);
            buf[size-1] = 0;
            accValid = false;
          }
          found = true;
          break;
        }
      }
      if (found == false)
        return numBytes;

      // Trim accelerator text e.g., '\tCtrl+X' (if any)
      // Accelerator can be \b instead of \t (NOTE: \a in .rc make a \b in res)
      //
      LPTSTR acc = new TCHAR[size];
      *acc = 0;
      LPTSTR c = _tcschr(buf, _T('\t'));
      if (c == 0)
        c = _tcschr(buf, _T('\b'));
      if (c)
      {
        if (bShowAccInTooltips && accValid)
        {
          // save the accelerator string for showing in tooltips
          //
          _tcscpy(acc, c+1);
        }
        *c = 0;
      }

      // Trim trailing dots, e.g., '...' (if any)
      //
      int i;
      for (i = static_cast<int>(_tcslen(buf)) - 1; i >= 0; i--)
        if (buf[i] == _T('.'))
          buf[i] = 0;
        else
          break;

      // Eliminate '&' (just emliminate first one)
      //
      i = -1;
      while(buf[++i])
        if (buf[i] == _T('&')) {
          do {
            buf[i] = buf[i+1];
            i++;
          } while (buf[i]);
          break;
        }

        // Show the accelerator in tooltip
        //
        if (bShowAccInTooltips && accValid && *acc != 0 &&
          (_tcslen(buf) + _tcslen(acc) + 3) < (uint)size)
        {
          _tcscat(buf, _T(" ("));
          _tcscat(buf, acc);
          _tcscat(buf, _T(")"));
        }

        delete[] acc;

        numBytes = static_cast<int>(_tcslen(buf));
    }
  }
  return numBytes;
}

bool TBmpMenu::IsDrawRecessed()
{
  return bDrawRecessed;
}

void TBmpMenu::SetDrawRecessed(bool b)
{
  bDrawRecessed = b;
}

bool TBmpMenu::IsWithAccel()
{
  return bShowAccInTooltips;
}

void TBmpMenu::SetWithAccel(bool b)
{
  bShowAccInTooltips = b;
}

bool TBmpMenu::IsBackBitmap()
{
  return BackBitmap !=0;
}

bool TBmpMenu::SetBackBitmap(uint resId)
// Set the background bitmap. If resId == 0 all menus will be show normal.
// return true if successful
{
  PRECONDITION(this);
  PRECONDITION(Module);

  delete BackBitmap;
  BackBitmap = 0;

  if (resId && resId != (uint)-1) {
    // 1: module, 2: Application module, 3: OWLExt module, 4 OWL Module
    TModule* module = FindResourceModule(0,Module,resId,RT_BITMAP);
    if (module)
      BackBitmap = new TBitmap(*module, TResId(resId));
  }
  return BackBitmap!=0;
}

bool TBmpMenu::IsRadioBitmap()
{
  return RadioBitmapId!=0;
}

bool TBmpMenu::SetRadioBitmap(uint resId)
// Set the radio dot bitmap. If resId == 0 a default radio dot will be draw.
// return true if successful
{
  PRECONDITION(Module);
  RadioBitmapId = 0;
  if (resId){
    TModule* module = FindResourceModule(0,Module,resId,RT_BITMAP);
    if (module)
      RadioBitmapId = resId;
  }
  return RadioBitmapId != 0;
}

bool TBmpMenu::IsCheckBitmap()
{
  return CheckBitmapId!=0;
}

bool TBmpMenu::SetCheckBitmap(uint resId)
// Set the checkbox bitmap. If resId == 0 a default checkbox will be draw.
// return true if successful
{
  PRECONDITION(Module);

  CheckBitmapId = 0;
  if (resId){
    TModule* module = FindResourceModule(0,Module,resId,RT_BITMAP);
    if (module)
      CheckBitmapId = resId;
  }
  return CheckBitmapId!=0;
}

//
TFont* TBmpMenu::GetMenuFont()
{
  return MenuFont;
}

void TBmpMenu::SetMenuFont(TFont* menuFont)
// Get/Set the menu font. The font will be deleted in this class
// By default the windows font for menus is used
{
  delete MenuFont;
  MenuFont = 0;
  MenuFont = menuFont;
}

TColor TBmpMenu::GetMaskColor()
{
  return MaskColor;
}

void TBmpMenu::SetMaskColor(const TColor& maskColor)
{
  MaskColor = maskColor;
}
//
void TBmpMenu::ExcludeBitmap(int resId)
// Exclude the bitmap with resId. It will not be shown in menus
{
  if (ExcludedBitmaps->Find(resId) == ExcludedBitmaps->NPOS)
    ExcludedBitmaps->Add(resId);
}

void TBmpMenu::IncludeBitmap(int resId)
// Include the bitmap with resId. By default all bitmaps in module
// will be shown in menus
{
  int idx = ExcludedBitmaps->Find(resId);
  if (idx != ExcludedBitmaps->NPOS)
    ExcludedBitmaps->Destroy(idx);
}

int TBmpMenu::AddMapEntry(TBmpMapEntry* entry)
// Adds a entry into the array MapEntries
{
  PRECONDITION(entry);
  return MapEntries->Add(entry);
}

void TBmpMenu::RemoveMapEntry(int loc)
// Removes entry at loc, and reduces the array by one element
{
  MapEntries->RemoveEntry(loc);
}

bool TBmpMenu::SetMenuItemText(HMENU hMenu, UINT pos, LPCTSTR txt)
// Set the owner draw menu item text like Win API function:
//   ModifyMenu(hMenu, pos, MF_BYPOSITION | MF_STRING, .., txt)
{
  PRECONDITION(hMenu);
  TCHAR str[MaxMenuStringLen];
  _tcscpy(str, txt ? txt : _T(""));

  TMenuItemInfo mii(MIIM_TYPE | MIIM_DATA | MIIM_STATE | MIIM_ID | MIIM_SUBMENU);
  if (::GetMenuItemInfo(hMenu, pos, true, &mii)){
    if (mii.fType&MFT_OWNERDRAW && mii.dwItemData != 0){
      // search menu item and change the text
      //
      for (int i=0; i < (int)MenuItems->GetItemsInContainer(); i++){
        if ((*MenuItems)[i]->Menu == hMenu && (*MenuItems)[i]->Pos == pos){
          if ((*MenuItems)[i]->Cmd > 0){
            if ((UINT)(*MenuItems)[i]->Cmd == mii.wID){
              if (_tcscmp ((*MenuItems)[i]->Text, str) == 0)
                return true; // nothing to change
              _tcscpy((*MenuItems)[i]->Text, str);
              return ::ModifyMenu(hMenu, pos, MF_BYPOSITION|MF_OWNERDRAW|mii.fState,
                (LPARAM)mii.wID, (LPCTSTR)(*MenuItems)[i]);
            }
          }
          else{
            if (mii.hSubMenu != 0){
              if (_tcscmp ((*MenuItems)[i]->Text, str) == 0)
                return true; // nothing to change
              _tcscpy((*MenuItems)[i]->Text, str);
              return ::ModifyMenu(hMenu, pos, MF_BYPOSITION|MF_OWNERDRAW|mii.fState,
                (LPARAM)-1, (LPCTSTR)(*MenuItems)[i]);
            }
          }
        }
      }
    }
  }
  return false;
}

void TBmpMenu::RemapMenuRecursiv(HMENU hMenu)
{
  static int iRecurse = 0;
  iRecurse ++;

  int nItem = ::GetMenuItemCount(hMenu);
  while ((--nItem)>=0)
  {
    UINT itemId = ::GetMenuItemID(hMenu, nItem);
    if (itemId == (UINT) -1)
    {
      HMENU pops = ::GetSubMenu(hMenu, nItem);
      if (pops)
        RemapMenuRecursiv(pops);
      if (bIsPopup || iRecurse>0)
      {
        TCHAR cs[MaxMenuStringLen];
        if (::GetMenuString(hMenu, nItem, cs, MaxMenuStringLen, MF_BYPOSITION) > 0)
        {
          TBmpMenuItem* pItem = AddMenuItem(hMenu, cs,
            (bIsPopup == false && iRecurse == 1)?-4:-2, nItem);
          ::ModifyMenu(hMenu, nItem, MF_BYPOSITION|MF_OWNERDRAW,
            (UINT)-1, (LPCTSTR)pItem);
        }
      }
    }
    else
    {
      TBmpMenuItem* pItem;
      TCHAR cs[MaxMenuStringLen];
      UINT oldState = ::GetMenuState(hMenu, nItem, MF_BYPOSITION);
      if (!(oldState&MF_OWNERDRAW) && !(oldState&MF_BITMAP))
      {
        if ((oldState&MF_SEPARATOR) || itemId == 0)
        {
          // Insert separator items only if a background bitmap is drawn
          //
          if (BackBitmap){
            pItem = AddMenuItem(hMenu, _T(""), -3, nItem);
            ::ModifyMenu(hMenu, nItem, MF_BYPOSITION|MF_OWNERDRAW|oldState,
              (LPARAM)itemId, (LPCTSTR)pItem);
          }
        }
        else
        {
          if (::GetMenuString(hMenu, nItem, cs, MaxMenuStringLen, MF_BYPOSITION) > 0)
          {
            TryToLoadBmpResource(itemId);
            pItem = AddMenuItem(hMenu, cs, itemId, nItem);
            ::ModifyMenu(hMenu, nItem, MF_BYPOSITION|MF_OWNERDRAW|oldState,
              (LPARAM)itemId, (LPCTSTR)pItem);
          }
        }
      }
    }
  }
  iRecurse --;
}

void TBmpMenu::DrawCheckmark(TDC& dc, int x, int y, bool enable, bool select)
{
  if (CheckBitmapId){

    TPointer<TDib> bitmap(new TDib(*Module, TResId(CheckBitmapId)));

    bitmap->MapUIColors(TDib::MapText|TDib::MapShadow|TDib::MapHighlight);
    bitmap->MapColor(MaskColor, TColor::Sys3dFace);

    TCelArray celArray(*(TDib*)bitmap, 1);

    TUIFace uiFace(TRect(TPoint(x, y), TSize(bitmap->Width(),
      bitmap->Height())), celArray);
    if (enable)
      uiFace.Paint(dc, TPoint(0, 0), select ? TUIFace::Normal :
      TUIFace::Down, true);
    else
      uiFace.Paint(dc, TPoint(0, 0), TUIFace::Disabled, false);
  }
  else  {
    // ......X
    // .....XX
    // X...XXX
    // XX.XXXo
    // XXXXXo.
    // .XXXo..
    // ..Xo...
    // ..o....
    x += 4;
    y += 4;

    TColor color = (enable) ? TColor::SysMenuText : TColor::Sys3dShadow;

    dc.SetPixel(x,y+2,color);
    dc.SetPixel(x,y+3,color);
    dc.SetPixel(x,y+4,color);
    dc.SetPixel(x+1,y+3,color);
    dc.SetPixel(x+1,y+4,color);
    dc.SetPixel(x+1,y+5,color);
    dc.SetPixel(x+2,y+4,color);
    dc.SetPixel(x+2,y+5,color);
    dc.SetPixel(x+2,y+6,color);
    dc.SetPixel(x+3,y+3,color);
    dc.SetPixel(x+3,y+4,color);
    dc.SetPixel(x+3,y+5,color);
    dc.SetPixel(x+4,y+2,color);
    dc.SetPixel(x+4,y+3,color);
    dc.SetPixel(x+4,y+4,color);
    dc.SetPixel(x+5,y+1,color);
    dc.SetPixel(x+5,y+2,color);
    dc.SetPixel(x+5,y+3,color);
    dc.SetPixel(x+6,y,color);
    dc.SetPixel(x+6,y+1,color);
    dc.SetPixel(x+6,y+2,color);
    if (enable == false)
    {
      TColor shadowcolor = TColor::Sys3dHilight;
      dc.SetPixel(x+2,y+7,shadowcolor);
      dc.SetPixel(x+3,y+6,shadowcolor);
      dc.SetPixel(x+4,y+5,shadowcolor);
      dc.SetPixel(x+5,y+4,shadowcolor);
      dc.SetPixel(x+6,y+3,shadowcolor);
    }
  }
}

//
void TBmpMenu::DrawRadioDot(TDC& dc, int x, int y, bool enable, bool select)
{
  if (RadioBitmapId){

    TPointer<TDib> bitmap(new TDib(*Module, TResId(RadioBitmapId)));

    bitmap->MapUIColors(TDib::MapText|TDib::MapShadow|TDib::MapHighlight);
    bitmap->MapColor(MaskColor, TColor::Sys3dFace);

    TCelArray celArray(*(TDib*)bitmap, 1);

    TUIFace uiFace(TRect(TPoint(x, y), TSize(bitmap->Width(),
      bitmap->Height())), celArray);
    if (enable)
      uiFace.Paint(dc, TPoint(0, 0), select ? TUIFace::Normal :
      TUIFace::Down, true);
    else
      uiFace.Paint(dc, TPoint(0, 0), TUIFace::Disabled, false);
  }
  else
  {
    x += 4;
    y += 4;
    TColor color = enable ? TColor::SysMenuText : TColor::Sys3dShadow;
    if (enable == false) {
      TRect rcDot(x, y, x+8, y+8);
      TBrush brush(TColor::Sys3dHilight);
      TPen pen(TColor::Sys3dHilight,0);
      dc.SelectObject(brush);
      dc.SelectObject(pen);
      dc.Ellipse(rcDot);
      dc.RestoreBrush();
      dc.RestorePen();
    }
    TRect rcDot(x, y, x+7, y+7);
    TBrush brush(color);
    TPen pen(color,0);
    dc.SelectObject(brush);
    dc.SelectObject(pen);
    dc.Ellipse(rcDot);
    dc.RestoreBrush();
    dc.RestorePen();
  }
}

//
int TBmpMenu::FindImageItem(int cmd)
{
  for (uint i = 0; i < ImageItems->Size(); i++)
    if ((*ImageItems)[i]->Cmd == cmd)
      return (*ImageItems)[i]->ImageIdx;

  return -1;
}
//
TBmpMenuItem* TBmpMenu::AddMenuItem(HMENU hMenu, LPCTSTR txt, int cmd, uint pos)
{
  PRECONDITION(hMenu);

  TCHAR str[MaxMenuStringLen];
  if (txt)
    _tcscpy(str, txt);
  else
    _tcscpy(str, _T(""));

  // if the same item exist, do not insert the menu Item
  //
  for (int i=0; i<(int)MenuItems->GetItemsInContainer(); i++)
  {
    if ((*MenuItems)[i]->Menu == hMenu &&
      (*MenuItems)[i]->Cmd == cmd &&
      (*MenuItems)[i]->Pos == pos &&
      _tcscmp((*MenuItems)[i]->Text, txt) == 0)
      return (*MenuItems)[i];
  }

  TBmpMenuItem* pItem = new TBmpMenuItem;
  //CHECK(pItem); will throw if error

  // set the menu item data
  //
  pItem->Menu = hMenu;
  pItem->Pos = pos;
  pItem->Cmd = cmd;
  if (cmd >= 0)
    pItem->ImageIdx = FindImageItem(cmd);
  else
    pItem->ImageIdx = cmd;

  _tcscpy(pItem->Text, str);

  MenuItems->Add(pItem);
  return pItem;
}
//
void TBmpMenu::AddImageItem(int idx, WORD cmd)
{
  if (idx < 0)
    return;

  ImageItems->Add(new TBmpMenuImageItem(idx, cmd));
}
//
bool TBmpMenu::TryToLoadBmpResource(unsigned int resId)
{
  PRECONDITION(Module);
  unsigned int mapResId = resId;

  // Check if the command id is mapped to another bitmap id
  //
  for (int i=0; i < (int)MapEntries->GetItemsInContainer(); i++) {
    if ((*MapEntries)[i]->CommandId == (int)resId){
      mapResId = (*MapEntries)[i]->BmpId;
      break;
    }
  }

  // Check if the bitmap is excluded
  // If a bitmap is exculuded, that is associated to more commands,
  // all commands are not shown with bitmap
  //
  if (ExcludedBitmaps->Find(mapResId) != ExcludedBitmaps->NPOS)
    return false;

  // Add only one bitmap for each command, because a command can be
  // in more than one menu
  //
  if (FindImageItem(resId) < 0){
    // Load bitmap from module and add them to image list
    //
    TModule* module = FindResourceModule(0,Module,mapResId,RT_BITMAP);
    if (module){
      int nBmpItems = ImageList->Add(TBitmap(*module, TResId(mapResId)), MaskColor);
      //int nBmpItems = celArray->Add(srcBmp);
      AddImageItem(nBmpItems, (WORD)resId);
      return true;
    }
  }
  return false;
}

void TBmpMenu::SyncronizeAllMenuItems()
// Syncronize the intern menu items and the real windows menus and items
{
  UINT i, num=MenuItems->GetItemsInContainer();
  TBmpMenuItem* item;
  for (i=num; i>0; i--)
  {
    // Delete a intern menu item, if the hMenu member
    // is not valid (e.g. the associated menu is destroyed).
    //
    item = (*MenuItems)[i-1];
    if (::IsMenu(item->Menu) == false)
      MenuItems->Destroy(i-1);
  }

  num=MenuItems->GetItemsInContainer();
  for (i=num; i>0; i--)
  {
    bool bRet, bDestroy = false;
    TMenuItemInfo mii(MIIM_TYPE | MIIM_DATA | MIIM_STATE | MIIM_ID | MIIM_SUBMENU);
    item = (*MenuItems)[i-1];
    if (item->Cmd > 0)
      bRet = ::GetMenuItemInfo(item->Menu, item->Cmd, false, &mii);
    else
      bRet = ::GetMenuItemInfo(item->Menu, item->Pos, true, &mii);
    if (bRet == false)
    {
      // if windows menu item not present (e.g. removed), delete intern item
      //
      MenuItems->Destroy(i-1);
    }
    else
    {
      if (item->Cmd > 0) // intern item is a simple item
      {
        if (mii.hSubMenu != 0)
        {
          // if windows menu item is a submenu, delete intern item
          //
          bDestroy = true;
        }
        else if ((UINT)item->Cmd != mii.wID)
        {
          // if windows command change, delete intern item
          //
          bDestroy = true;
        }
      }
      else
      {
        if (item->Cmd == -3)
        {
          // if intern item is a separator and there is no background bitmap,
          // delete intern item
          //
          if (!BackBitmap)
            bDestroy = true;
        }
        else if (mii.hSubMenu == 0)
        {
          // if intern item is a submenu and windows menu is a simple item,
          // delete intern item
          //
          bDestroy = true;
        }
      }

      if (bDestroy == false &&
        !(mii.fType&MFT_OWNERDRAW)) // windows item is not ownerdrawn
      {
        if ((mii.fType&MFT_STRING) && mii.dwTypeData != 0)
        {
          // if windows item is a string item, try to syncronize the items
          //
          _tcscpy(item->Text, mii.dwTypeData);
          uint cmd = (item->Cmd <= 0) ? (UINT)-1 : item->Cmd;
          ::ModifyMenu(item->Menu, item->Pos,
            MF_BYPOSITION|MF_OWNERDRAW|mii.fState, cmd, (LPCTSTR)item);
        }
      }

      // if windows item is ownerdrawn and have no valid datas, delete itern item
      //
      if ((mii.fType&MFT_OWNERDRAW) && mii.dwItemData == 0)
        bDestroy = true;

      // check menu type before destroy the intern item
      //
      if (bDestroy && !(mii.fType&MFT_OWNERDRAW))
        MenuItems->Destroy(i-1);
    }
  }
}

} // OwlExt namespace
//


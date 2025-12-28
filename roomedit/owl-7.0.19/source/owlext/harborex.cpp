// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// harborex.cpp:  implementation file
// Version:       1.5
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
//   EMail: dieter.windau@kvwl.de
//   Web:   http://members.aol.com/softengage/index.htm
//
// ****************************************************************************
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/uihelper.h>      // for TUIBorder edge painting
#include <owl/registry.h>
#include <owl/profile.h>
#include <owl/filename.h>

#include <owlext/harborex.h>
#include <owlext/dockingex.h>
#include <owlext/gadgetex.h>
#include <owlext/toolcustdlg.h>
#include <owlext/butappdlg.h>
#include <owlext/gadgctrl.h>

#include <algorithm>

using namespace owl;

namespace OwlExt {

// ******************************* constants ***********************************

const int MaxDCBMenuItems = 10;// Max number of menu items to show/hide controlbars
const int DefaultMainWindowLeft = 0;
const int DefaultMainWindowTop = 0;
const int DefaultMainWindowRight = 640;
const int DefaultMainWindowBottom = 480;
const int DockingExVersion = 5; // Intern Version that will be saved with settings
// This is needed for future

// ***************** Section strings for use in registry **********************

const TCHAR* MainWindowSection = TEXT("MainWindow");
const TCHAR* ColorPickerSection = TEXT("ColorPicker");
const TCHAR* LeftSection = TEXT("Left");
const TCHAR* TopSection = TEXT("Top");
const TCHAR* RightSection = TEXT("Right");
const TCHAR* BottomSection = TEXT("Bottom");
const TCHAR* VersionSection = TEXT("Version");
const TCHAR* CmdShowSection = TEXT("CmdShow");
const TCHAR* ToolbarsSection = TEXT("Toolbars");
const TCHAR* TooltipsSection = TEXT("Tooltips");
const TCHAR* AccKeysSection = TEXT("AccKeys");
const TCHAR* DrawGripperSection = TEXT("DrawGripper");
const TCHAR* FlatStyleSection = TEXT("FlatStyle");
const TCHAR* HintModeSection = TEXT("HintMode");
const TCHAR* IDSection = TEXT("ID");
const TCHAR* ToolbarSection = TEXT("Toolbar");
const TCHAR* NameSection = TEXT("Name");
const TCHAR* LocationSection = TEXT("Location");
const TCHAR* VisibleSection = TEXT("Visible");
const TCHAR* DefaultSection = TEXT("Default");
const TCHAR* GadgetCountSection = TEXT("GadgetCount");
const TCHAR* CountSection = TEXT("Count");
const TCHAR* ButtonSection = TEXT("Button");
const TCHAR* BmpIdSection = TEXT("BmpId");
const TCHAR* DisplayTypeSection = TEXT("DisplayType");
const TCHAR* TextSection = TEXT("Text");

// ************************ THarborManagement *********************************

DEFINE_RESPONSE_TABLE1(THarborManagement, TEventHandler)
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 0, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 1, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 2, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 3, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 4, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 5, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 6, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 7, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 8, CmShowHideCtlBar),
EV_COMMAND_AND_ID(CM_SHOWHIDECTLBAR + 9, CmShowHideCtlBar),

EV_COMMAND(CM_CUSTOMIZE, CmCustomize),

EV_COMMAND(CM_GADGETRESET, CmGadgetReset),
EV_COMMAND(CM_GADGETDELETE, CmGadgetDelete),
EV_COMMAND(CM_GADGETCOPYIMAGE, CmGadgetCopyImage),
EV_COMMAND(CM_GADGETPASTEIMAGE, CmGadgetPasteImage),
EV_COMMAND(CM_GADGETAPPEARANCE, CmGadgetAppearance),
EV_COMMAND(CM_GADGETIMAGE, CmGadgetImage),
EV_COMMAND(CM_GADGETTEXT, CmGadgetText),
EV_COMMAND(CM_GADGETIMAGETEXT, CmGadgetImageText),
EV_COMMAND(CM_GADGETBEGINGROUP, CmGadgetBeginGroup),

EV_COMMAND_ENABLE(CM_GADGETRESET, CeGadgetReset),
EV_COMMAND_ENABLE(CM_GADGETDELETE, CeGadgetDelete),
EV_COMMAND_ENABLE(CM_GADGETCOPYIMAGE, CeGadgetCopyImage),
EV_COMMAND_ENABLE(CM_GADGETPASTEIMAGE, CeGadgetPasteImage),
EV_COMMAND_ENABLE(CM_GADGETAPPEARANCE, CeGadgetAppearance),
EV_COMMAND_ENABLE(CM_GADGETIMAGE, CeGadgetImage),
EV_COMMAND_ENABLE(CM_GADGETTEXT, CeGadgetText),
EV_COMMAND_ENABLE(CM_GADGETIMAGETEXT, CeGadgetImageText),
EV_COMMAND_ENABLE(CM_GADGETBEGINGROUP, CeGadgetBeginGroup),
END_RESPONSE_TABLE;

int THarborManagement::InternVersion = 0; // by default no version

THarborManagement::THarborManagement(LPCTSTR registryName):
TEventHandler()
{
  PRECONDITION(registryName);
  RegistryName = registryName;
  PopupMenu = 0;
  Harbor = 0;
  InternVersion = 0;
}

THarborManagement::~THarborManagement()
{
  if (PopupMenu && ::IsMenu(PopupMenu))
    ::DestroyMenu(PopupMenu);
}

void THarborManagement::InsertToolbarMenuItem(TCommandEnabler& ce, int offset)
{
  TMenuItemEnabler* me = TYPESAFE_DOWNCAST(&ce, TMenuItemEnabler);
  if (Harbor && me && me->GetMenu()) {

    // Delete old toolbar menu item from menu and destroy the menu in windows
    //
    if (PopupMenu && ::IsMenu(PopupMenu)) {
      ::DeleteMenu(me->GetMenu(), me->GetPosition()+offset, MF_BYPOSITION);
      if (PopupMenu && ::IsMenu(PopupMenu))
        ::DestroyMenu(PopupMenu);
    }

    // Get the actual toolbar menu created by THarborEx and insert them
    //
    PopupMenu = Harbor->GetToolbarMenu();
    if (PopupMenu) {
      TCHAR str[255];
      if (::GetMenuString(PopupMenu, 0, str, 255, MF_BYPOSITION) != 0) {
        ::InsertMenu(me->GetMenu(), me->GetPosition()+offset,
          MF_BYPOSITION | MF_POPUP, reinterpret_cast<UINT_PTR>(GetSubMenu(PopupMenu, 0)), str);
      }
    }
  }
}

void THarborManagement::LoadSettings()
{
  TRect r;
  uint32 color;
  uint32 size = sizeof(uint32);

  owl::tstring str = RegistryName + owl::tstring(_T("\\")) + MainWindowSection;
  TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());

  if (regKey.QueryValue(VersionSection, 0, (uint8*)&InternVersion, &size) !=  ERROR_SUCCESS)
    InternVersion = 0; // by default no version
  if (regKey.QueryValue(LeftSection, 0, (uint8*)&r.left, &size) != ERROR_SUCCESS)
    r.left = DefaultMainWindowLeft;
  if (regKey.QueryValue(TopSection, 0, (uint8*)&r.top, &size) != ERROR_SUCCESS)
    r.top = DefaultMainWindowTop;
  if (regKey.QueryValue(RightSection, 0, (uint8*)&r.right, &size) != ERROR_SUCCESS)
    r.right = DefaultMainWindowRight;
  if (regKey.QueryValue(BottomSection, 0, (uint8*)&r.bottom, &size) != ERROR_SUCCESS)
    r.bottom = DefaultMainWindowBottom;

  int CmdShow;
  if (regKey.QueryValue(CmdShowSection, 0, (uint8*)&CmdShow, &size) != ERROR_SUCCESS)
    CmdShow = SW_SHOW;

  if (Harbor) {
    if (CmdShow != SW_SHOW)
      Harbor->DecoratedFrame.ShowWindow(CmdShow);
    else
      Harbor->DecoratedFrame.SetWindowPos(0, r, SWP_NOZORDER);
  }

  // Load user defined colors
  //
  if (InternVersion >= 5) {
    owl::tstring str2 = RegistryName + owl::tstring(_T("\\")) + ColorPickerSection;
    TRegKey regKey2(TRegKey::GetCurrentUser(), str2.c_str());
    for (int i=0; i<NumCustColors; i++) {
      TCHAR posstr[10];
      wsprintf(posstr, TEXT("%d"), i+1);
      if (regKey2.QueryValue(posstr, 0, (uint8*)&color, &size) !=  ERROR_SUCCESS)
        color = 0xff000000;
      TColorPicker::SetCustomColorsValue(i, TColor(color));
    }
  }

  if (Harbor)
    Harbor->LoadSettings(*this);
}

void THarborManagement::SaveSettings()
{
  if (Harbor) {

    // delete the MainWindow tree and create new one
    //
    owl::tstring str = RegistryName + owl::tstring(_T("\\")) + MainWindowSection;
    TRegKey::GetCurrentUser().NukeKey(str.c_str());
    TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());

    regKey.SetValue(VersionSection, DockingExVersion);

    TRect r = Harbor->DecoratedFrame.GetWindowRect();
    regKey.SetValue(LeftSection, r.left);
    regKey.SetValue(TopSection, r.top);
    regKey.SetValue(RightSection, r.right);
    regKey.SetValue(BottomSection, r.bottom);

    int CmdShow = SW_SHOW;
    if (Harbor->DecoratedFrame.IsIconic())
      CmdShow = SW_MINIMIZE;
    else if (Harbor->DecoratedFrame.IsZoomed())
      CmdShow = SW_MAXIMIZE;
    regKey.SetValue(CmdShowSection, CmdShow);
  }

  // Save user defined colors
  //
  owl::tstring str2 = RegistryName + owl::tstring(_T("\\")) + ColorPickerSection;
  TRegKey::GetCurrentUser().NukeKey(str2.c_str());
  TRegKey regKey2(TRegKey::GetCurrentUser(), str2.c_str());
  for (int i=0; i<NumCustColors; i++) {
    TCHAR posstr[10];
    wsprintf(posstr, _T("%d"), i+1);
    regKey2.SetValue(posstr, (uint32)TColorPicker::GetCustomColorsValue(i));
  }

  if (Harbor)
    Harbor->SaveSettings(*this);
}

// ****************** TDCBData ************************************************

TDCBData& TDCBData::operator =(const TDCBData& d)
{
  DCB   = d.DCB;
  Slip  = d.Slip;
  Rect  = d.Rect;
  _tcscpy(Title, d.Title);
  return *this;
}

// ****************** THarborEx ***********************************************

DEFINE_RESPONSE_TABLE1(THarborEx, THarbor)
EV_WM_MOUSEMOVE,
EV_WM_LBUTTONDBLCLK,
EV_WM_LBUTTONUP,
END_RESPONSE_TABLE;

THarborEx::THarborEx(TDecoratedFrame& df, bool leftSlip, bool topSlip,
           bool rightSlip, bool bottomSlip)
           :
THarbor(df),
DecoratedFrame(df),
DCBData(5,0,5)
// If you create one of the four TEdgeSlips in THarbor before you insert
// decorations in TDecoratedFrame all the TDockable's in THarbor insert near
// the main window borders. Some examples:
// - if you don't create top and left slip, e.g. a inserted ruler
//   is not near the client window of the decorated frame
// - if you create bottom slip here, e.g. a inserted statusbar
//   is not always at bottom of the decorated frame
{
  ShowTooltips = true;  // show tooltips for all dockable control bars
  WithAccel = true;     // show accelerator key in tooltips
  DrawGripper = true;   // draw gripper for all edge slips
  HintMode = TGadgetWindow::PressHints;

  CustomizeMode = false;// true, if the customize dialog is open
  CustomizeDlg = 0;
  GadgetDragging = false;
  CutPasteCursor = GetGlobalModule().LoadCursor(IDC_CUTPASTEGADGET);
  CopyCursor = GetGlobalModule().LoadCursor(IDC_COPYGADGET);
  DeleteCursor = GetGlobalModule().LoadCursor(IDC_DELETEGADGET);
  OldCursor = 0;     // Cursor, before gadget dragging started

  MemBmp = 0;
  MemDC = 0;
  DragScreenDC = 0;
  DragDropP1 = DragDropP2 = TPoint(0,0);
  DragDropLineVisible = false;
  DragDropGadget = 0;
  DragDropWindow = 0;
  DragDropPlacement = TGadgetWindow::After;
  DragDropGadgetState = IsNone;

  SetCustGadgetNull();
  CopiedBmpResId = 0;

  TEdgeSlip* EdgeSlip;
  // Create the top slip
  //
  if (topSlip) {
    EdgeSlip = ConstructEdgeSlip(DecoratedFrame, alTop);
    SetEdgeSlip(alTop, EdgeSlip);
    DecoratedFrame.Insert(*EdgeSlip, (TDecoratedFrame::TLocation)alTop);
  }

  // Create the left slip
  //
  if (leftSlip) {
    EdgeSlip = ConstructEdgeSlip(DecoratedFrame, alLeft);
    SetEdgeSlip(alLeft, EdgeSlip);
    DecoratedFrame.Insert(*EdgeSlip, (TDecoratedFrame::TLocation)alLeft);
  }

  // Create the right slip
  //
  if (rightSlip) {
    EdgeSlip = ConstructEdgeSlip(DecoratedFrame, alRight);
    SetEdgeSlip(alRight, EdgeSlip);
    DecoratedFrame.Insert(*EdgeSlip, (TDecoratedFrame::TLocation)alRight);
  }

  // Create the bottom slip
  //
  if (bottomSlip) {
    EdgeSlip = ConstructEdgeSlip(DecoratedFrame, alBottom);
    SetEdgeSlip(alBottom, EdgeSlip);
    DecoratedFrame.Insert(*EdgeSlip, (TDecoratedFrame::TLocation)alBottom);
  }
}

THarborEx::~THarborEx()
{
  if (CustomizeDlg) {
    CustomizeDlg->CloseWindow(IDCANCEL);
    delete CustomizeDlg;
  }
  Destroy(IDCANCEL);
}

bool THarborEx::GadgetDraggingBegin()
// Called by dockable control bars
{
  GadgetDragging = true;
  OldCursor = GetCursor();
  SetCapture();
  DragScreenDC = new TScreenDC;
  DragScreenDC->SelectStockObject(BLACK_BRUSH);
  DragScreenDC->SelectStockObject(BLACK_PEN);
  return true;
}

void THarborEx::SetHintMode(TGadgetWindow::THintMode hintMode)
{
  if (HintMode != hintMode) {
    GetDCBData();
    HintMode = hintMode;
    for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
      // change the hint mode for all visible and hidden dockable controlbars
      //
      TDockableControlBarEx* dcb = DCBData[i]->DCB;
      if (dcb && dcb->GetHandle())
        dcb->SetHintMode(HintMode);
    }
  }
}

void THarborEx::SetDrawGripper(bool drawGripper)
{
  if (DrawGripper != drawGripper) {
    GetDCBData();
    DrawGripper = drawGripper;
    for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
      // if the docking slip is a valid TEdgeSlip and has a valid non static
      // dockable controlbar as parent
      //
      TDockableControlBarEx* cb = DCBData[i]->DCB;
      TEdgeSlipEx* edgeSlip = TYPESAFE_DOWNCAST(DCBData[i]->Slip, TEdgeSlipEx);
      if (edgeSlip && edgeSlip->GetHandle() && cb && cb->GetHandle()) {
        // set the margins for the gripper (invisible)
        //
        cb->ShowWindow(SW_HIDE);
        edgeSlip->SetGripperMargins(cb);
        cb->ShowWindow(SW_SHOWNA);
      }
    }
  }
}

bool THarborEx::PreProcessAppMsg(MSG& msg)
{
  switch (msg.message)
  {
    //    case WM_COMMAND:
    //    case WM_SYSCOMMAND:
    //    if (CustomizeMode)
    //      MessageBeep(0);//return true;   // do nothing
    //      break;

  case WM_RBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
    GetDCBData();
    TPoint pt;
    pt = TPoint(msg.lParam);

    if (CustomizeMode == false) {
      if (msg.message == WM_RBUTTONDOWN) {
        // if the user makes a right click in TDockableControlBar areas
        // a dropdown menu will be shown. Here you can show/hide controlbars
        // or customize the controlbars
        //
        ::ClientToScreen(msg.hwnd, &pt);

        TRect ClientRect, WinRect;
        DecoratedFrame.GetWindowRect(WinRect);
        DecoratedFrame.GetClientWindow()->GetWindowRect(ClientRect);

        TRect TR = TRect(WinRect.left, 32000, WinRect.right, 0);
        TRect BR = TR;
        TRect LR = TRect(32000, ClientRect.top, 0, ClientRect.bottom);
        TRect RR = LR;

        bool hit;
        for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
          TDCBData* data = DCBData[i];
          if (data && data->Slip && data->DCB) {
            switch(data->Slip->GetLocation()) {
  case alTop:     TR |= data->Rect;
    hit = TR.Contains(pt);
    break;
  case alBottom:  BR |= data->Rect;
    hit = BR.Contains(pt);
    break;
  case alLeft:    LR |= data->Rect;
    hit = LR.Contains(pt);
    break;
  case alRight:   RR |= data->Rect;
    hit = RR.Contains(pt);
    break;
  case alNone:    hit = data->Rect.Contains(pt);
    break;
            }
            if (hit) {

              // Special feature: If you pressed the CTRL key the customize
              // mode will be started and then a right button click will be
              // send to the toolbar
              //
              if (::GetKeyState(VK_CONTROL) < 0) {

                TDockableControlBarEx* dcb = data->DCB;

                CustomizeDlg = CreateCustomizeDlg(DecoratedFrame);
                CustomizeDlg->Create();
                CustomizeDlg->ShowWindow(SW_SHOW);

                //D: This work not always correct
                //
                ::ScreenToClient(dcb->GetHandle(), &pt);
                dcb->SendMessage(WM_RBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
              }
              else {
                HMENU hMenu = GetToolbarMenu(false);
                if (hMenu && ::GetSubMenu(hMenu, 0) && DecoratedFrame.GetHandle()) {
                  ::TrackPopupMenu(::GetSubMenu(hMenu, 0),
                    TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pt.x, pt.y, 0, DecoratedFrame.GetHandle(), 0);
                  ::DestroyMenu(hMenu);
                }
              }
              return true;
            }
          }
        }
      }
    }
    else {
      for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
        TPoint mapPoint = pt;
        ::MapWindowPoints(msg.hwnd, DCBData[i]->DCB->GetHandle(), &mapPoint, 1);
        TGadget* gadget = DCBData[i]->DCB->GadgetFromPoint(mapPoint);
        if (gadget) {
          TGadgetDesc* desc = GadgetDescriptors->Find(gadget->GetId());
          if (desc && desc->Type == TGadgetDesc::ControlGadgetDesc) {

            // Forward the msg to the control gadget
            //
            DCBData[i]->DCB->SendMessage(msg.message, 0,
              MAKELPARAM(mapPoint.x, mapPoint.y));

            return true;
          }
        }
      }
    }
    break;
  }
  return false;
}

int THarborEx::PostGetHintText(uint /*id*/, LPTSTR buf, int /*size*/,
                 THintText hintType)
                 // Retrieves the hint text associated with a particular Id
                 // To be called from TDecoratedFrame GetHintText(...)
                 // If ShowTooltips == false, the tooltips will be deleted
                 // If WithAccel == false, the accelerator keys defined by resource in style:
                 // " (key)" will be hidden.
{
  if (buf) {
    int len = static_cast<int>(_tcslen(buf));
    if (ShowTooltips == false && len > 0 && hintType == htTooltip) {
      *buf = 0;
      return 0;
    }
    if (len > 0 && hintType == htTooltip) {
      TCHAR* chr = _tcschr(buf, _T('\t'));
      if (chr == 0) {
        // NOTE: I'm not sure \a in .rc make a \b in res
        chr = _tcschr(buf, _T('\a'));
      }
      if (chr == 0) {
        // NOTE: I'm not sure \a in .rc make a \b in res
        chr = _tcschr(buf, _T('\b'));
      }
      if (chr) {
        *chr = (WithAccel == false) ? (TCHAR)0 : _T(' ');
        return static_cast<int>(_tcslen(buf));
      }
#ifdef BMPMENU
      // If the BmpMenu isn't integrated in dockingex classes
      // this is a special solution
      else {
        chr = _tcschr(buf, _T('('));
        if (chr) {
          // -1 for space before the '('
          *(chr-1) = (WithAccel == false) ? (TCHAR)0 : (TCHAR)' ';
          return static_cast<int>(strlen(buf));
        }
      }
#endif
    }
    return len;
  }
  return 0;
}

bool THarborEx::IsDCBTitleValid(LPCTSTR title)
// return true, if the title don't exist. Title should be a unique identifier
{
  PRECONDITION(title);
  GetDCBData();
  for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
    if (_tcscmp(title, DCBData[i]->Title) == 0)
      return false;
  }
  return true;
}

int THarborEx::GetFreeDCBId()
// returns a unused id for a new dockable control bar. -1 if  an error occurs
{
  GetDCBData();
  for (int id=1; id<10000; id++) {
    bool found = true;
    for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
      if (DCBData[i]->DCB->Attr.Id == id)
        found = false;
    }
    if (found)
      return id;
  }
  return -1;
}

void THarborEx::SetCustGadget(TGadgetWindow* gw, TGadget* gadget)
{
  PRECONDITION(gw);
  PRECONDITION(gadget);

  if (CustGadgetType == IsToolbarGadget)
    EraseCustomizeFrame();

  CustGadgetType = IsToolbarGadget;
  CustWindow = gw;
  CustGadget = gadget;
  CustGadgetRef = -1;

  CustWindow->InvalidateRect(CustGadget->GetBounds());
  CustWindow->UpdateWindow();
}

void THarborEx::SetCustGadget(int refId)
{
  PRECONDITION(refId > 0);

  if (CustGadgetType == IsToolbarGadget)
    EraseCustomizeFrame();

  CustGadgetType = IsRefGadget;
  CustWindow = 0;
  CustGadget = 0;
  CustGadgetRef = refId;
}

void THarborEx::SetCustGadgetNull()
{
  CustGadgetType = IsNoGadget;
  CustWindow = 0;
  CustGadget = 0;
  CustGadgetRef = -1;
}

TEdgeSlip* THarborEx::ConstructEdgeSlip(TDecoratedFrame& df, TAbsLocation location)
// Factory function overridden to use TEdgeSlipEx in lace of TEdgeSlip
{
  // This is NOT a typesafe cast, but should be OK since public members are
  // exactly the same as for TEdgeSlip.
  //
  return (TEdgeSlip*)(new TEdgeSlipEx(df, location));
}

TFloatingSlip* THarborEx::ConstructFloatingSlip(TDecoratedFrame& df,
                        int x, int y, TWindow* dockableWindow)
                        // Factory function overridden to use TFloatingSlipEx in lace of TFloatingSlip
{
  return TYPESAFE_DOWNCAST(new TFloatingSlipEx(
    &df, x, y, dockableWindow), TFloatingSlip);
}

//
TToolbarCustomizeSheet*
THarborEx::CreateCustomizeDlg(TDecoratedFrame& frame)
{
  return new TToolbarCustomizeSheet(&frame, this);
}
//
void THarborEx::EvMouseMove(uint modKeys, const TPoint& point_)
{
  if (GadgetDragging && DragScreenDC) {
    TPoint originalPoint = point_;
    TPoint point(point_);
    MapWindowPoints(0, &point, 1);
    DragDropGadgetState = IsNone;

    if (CustGadgetType != IsNoGadget) {
      TDockableControlBarEx* dcb = GetHitDCB(point);
      if (dcb) {
        DragDropGadgetState = IsCut;
        if (CustGadget) {
          TGadgetDesc* desc = GadgetDescriptors->Find(CustGadget->GetId());
          if (desc && (modKeys & MK_CONTROL))
            DragDropGadgetState = IsCopy;
          if (desc && (desc->Attr & GADG_NOCOPY))
            DragDropGadgetState = IsCut;
          if (desc && (desc->Attr & GADG_NOCUT))
            DragDropGadgetState = IsNone;
        }
        else
          DragDropGadgetState = IsCopy;

        MapWindowPoints(dcb->GetHandle(), &originalPoint, 1);
        TPoint p1, p2;
        DragDropWindow = 0;
        DragDropGadget = dcb->GetDragDropGadget(
          originalPoint, p1, p2, DragDropPlacement);
        if (DragDropGadget) {

          dcb->MapWindowPoints(0, &p1, 1);
          dcb->MapWindowPoints(0, &p2, 1);

          if (DragDropGadgetState == IsCut && CustWindow == dcb) {
            if ((DragDropGadget == CustGadget) ||
              (DragDropPlacement == TGadgetWindow::After &&
              dcb->NextGadget(*DragDropGadget) == CustGadget) ||
              (DragDropPlacement == TGadgetWindow::Before &&
              dcb->PrevGadget(DragDropGadget) == CustGadget)) {
                DragDropGadgetState = IsNone;
            }
          }

          if (DragDropGadgetState != IsNone) {
            DragDropWindow = dcb;
            if (p1 != DragDropP1 || p2 != DragDropP2) {
              DragDropP1 = p1;
              DragDropP2 = p2;
              ShowDragDropLine(true);
            }
          }
        }
      }
      else {
        if (CustGadget)
          DragDropGadgetState = IsDelete;
      }
    }

    if (DragDropGadgetState == IsCopy)
      ::SetCursor(CopyCursor);
    else if (DragDropGadgetState == IsDelete) {
      ::SetCursor(DeleteCursor);
      ShowDragDropLine(false); // Hide them
    }
    else if (DragDropGadgetState == IsNone) {
      ::SetCursor(::LoadCursor(0, IDC_NO));
      ShowDragDropLine(false); // Hide them
    }
    else
      ::SetCursor(CutPasteCursor); // Show Cut/Paste cursor for no action
  }
  else
    THarbor::EvMouseMove(modKeys, point_);
}

void THarborEx::EvLButtonUp(uint modKeys, const TPoint& point)
{
  if (GadgetDragging && DragScreenDC) {
    GadgetDragging = false;

    ShowDragDropLine(false); // Hide them

    if (DragDropGadgetState != IsNone) {
      if (DragDropGadgetState == IsDelete) {
        CmGadgetDelete();
      }
      else {
        if (DragDropGadget && DragDropWindow) {
          TDockableControlBarEx* pdcb = TYPESAFE_DOWNCAST(DragDropWindow,
            TDockableControlBarEx);
          if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
            if (DragDropGadgetState == IsCut) {
              TDockableControlBarEx* cdcb = TYPESAFE_DOWNCAST(CustWindow,
                TDockableControlBarEx);
              TGadget* cutGadget = cdcb->RemoveEx(CustGadget, false);
              if (cutGadget) {
                pdcb->InsertEx(*cutGadget, DragDropPlacement, DragDropGadget);
                SetCustGadget(pdcb, cutGadget);
              }
            }
            else {
              TGadget* gadget = GadgetDescriptors->ConstructGadget(
                CustGadget->GetId());
              if (gadget) {
                pdcb->InsertEx(*gadget, DragDropPlacement, DragDropGadget);
                SetCustGadget(pdcb, gadget);
              }
            }
            CustWindow = pdcb;
          }
          else if (CustGadgetType == IsRefGadget) {
            TGadget* gadget = GadgetDescriptors->ConstructGadget(CustGadgetRef);
            if (gadget)
              pdcb->InsertEx(*gadget, DragDropPlacement, DragDropGadget);
          }
        }
        DragDropWindow = 0;
        DragDropGadget = 0;
      }
    }

    ::SetCursor(OldCursor);
    OldCursor = 0;
    ReleaseCapture();
    DragScreenDC->RestoreBrush();
    DragScreenDC->RestorePen();
    delete DragScreenDC;
    DragScreenDC = 0;
  }
  else
    THarbor::EvLButtonUp(modKeys, point);
}

void THarborEx::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  THarbor::EvLButtonDblClk(modKeys, point);
}

void THarborEx::SetCustomizeMode(bool b)
{
  if (CustomizeMode != b) {
    CustomizeMode = b;
    if (CustomizeMode == false) {

      if (CustGadgetType == IsToolbarGadget)
        EraseCustomizeFrame();

      SetCustGadgetNull();
    }
  }
}

void THarborEx::UpdateShowHideCtlBar()
{
  if (CustomizeMode)
    CustomizeDlg->Update();
}

TDockableControlBarEx* THarborEx::GetHitDCB(const TPoint& screenPoint)
// returns the dockable control bar if the screenPoint is in it.
// returns 0 if no dockable control bar hit the point
{
  for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
    TDCBData* data = DCBData[i];
    if (data && data->Slip && data->DCB) {
      TRect    dcbScreenRect;
      data->DCB->GetRect(dcbScreenRect);
      TMargins margins = data->DCB->Margins;
      dcbScreenRect.left   += margins.Left;
      dcbScreenRect.top    += margins.Top;
      dcbScreenRect.right  -= margins.Right;
      dcbScreenRect.bottom -= margins.Bottom;
      if (dcbScreenRect.Contains(screenPoint))
        return data->DCB;
    }
  }
  return 0;
}

void THarborEx::ShowDragDropLine(bool bShow)
{
  if (DragDropLineVisible && MemDC) {
    DragDropLineVisible = false;
    DragScreenDC->BitBlt(DragDropRect, *MemDC, TPoint(0,0), SRCCOPY);
    MemDC->RestoreBitmap();
    delete MemBmp;
    delete MemDC;
    MemBmp = 0;
    MemDC = 0;
  }

  if (bShow && DragDropLineVisible == false) {
    if (DragDropP1.x == DragDropP2.x)
      DragDropRect = TRect(DragDropP1.x-5, DragDropP1.y-1,
      DragDropP1.x+5, DragDropP2.y+1);
    else
      DragDropRect = TRect(DragDropP1.x-1, DragDropP1.y-5,
      DragDropP2.x+1, DragDropP1.y+5);

    // Save Screen area in memory dc / memory bitmap
    // Very special thanks to Roger Lathrop
    //
    if (MemDC)
      delete MemDC;
    if (MemBmp)
      delete MemBmp;
    MemDC = new TMemoryDC(*DragScreenDC);
    MemBmp = new TBitmap(*DragScreenDC, DragDropRect.Width(), DragDropRect.Height());
    MemDC->SelectObject(*MemBmp);
    MemDC->BitBlt(0,0,DragDropRect.Width(), DragDropRect.Height(),
      *DragScreenDC, DragDropRect.left, DragDropRect.top, SRCCOPY);

    // Draw Drag & Drop figure
    //
    TPoint* ptArray = new TPoint[8];
    TRect outerRect = DragDropRect;
    outerRect.Inflate(-1, -1);
    TRect innerRect = DragDropRect;
    innerRect.Inflate(-4, -4);
    ptArray[0] = outerRect.TopLeft();
    ptArray[2] = innerRect.TopRight();
    ptArray[4] = outerRect.BottomRight();
    ptArray[6] = innerRect.BottomLeft();
    if (DragDropP1.x == DragDropP2.x) {
      ptArray[1] = outerRect.TopRight();
      ptArray[3] = innerRect.BottomRight();
      ptArray[5] = outerRect.BottomLeft();
      ptArray[7] = innerRect.TopLeft();
    }
    else {
      ptArray[1] = innerRect.TopLeft();
      ptArray[3] = outerRect.TopRight();
      ptArray[5] = innerRect.BottomRight();
      ptArray[7] = outerRect.BottomLeft();
    }
    DragScreenDC->Polygon(ptArray, 8);
    delete[] ptArray;
    DragDropLineVisible = true;
  }
}

void THarborEx::EraseCustomizeFrame()
{
  // Delete black rectangle from old customize gadget
  //
  if (CustGadget && CustWindow) {
    TGadget* oldgadget = CustGadget;
    TGadgetWindow* oldwindow = CustWindow;
    CustGadget = 0;
    CustWindow = 0;

    // If the old gadget is a control gadget, the control inside the gadget
    // must be updated directly, because the InvalidateRect here comes not
    // to the control gadget.
    //
    TControlGadgetEx* ctlgad = TYPESAFE_DOWNCAST(oldgadget, TControlGadgetEx);
    if (ctlgad && ctlgad->GetControl() && ctlgad->GetControl()->GetHandle()) {
      ctlgad->GetControl()->Invalidate();
      ctlgad->GetControl()->UpdateWindow();
    }
    else {
      oldwindow->InvalidateRect(oldgadget->GetBounds());
      oldwindow->UpdateWindow();
    }
  }
}

void THarborEx::GetDCBData()
{
  DCBData.Flush();

  // search in all child windows of TDecoratedFrame valid TDockingSlip windows
  //
  TRect rect = TRect(0,0,0,0);
  TWindow* first = DecoratedFrame.GetFirstChild();
  if (first) {
    TWindow* child = first;
    do {
      TDockableControlBarEx* cb;
      TDockingSlip* dockSlip = TYPESAFE_DOWNCAST(child, TDockingSlip);
      TEdgeSlipEx* edgeSlip = TYPESAFE_DOWNCAST(child, TEdgeSlipEx);
      TFloatingSlipEx* floatSlip = TYPESAFE_DOWNCAST(child, TFloatingSlipEx);

      if (dockSlip && edgeSlip && edgeSlip->GetHandle()) {
        TWindow* edgeFirst = edgeSlip->GetFirstChild();
        if (edgeFirst){
          TWindow* edgeChild = edgeFirst;
          do {
            cb = TYPESAFE_DOWNCAST(edgeChild, TDockableControlBarEx);
            InsertDCBData(cb, dockSlip);
            edgeChild = edgeChild->Next();
          }
          while (edgeChild != edgeFirst);
        }
      }
      else if (dockSlip && floatSlip && floatSlip->GetHandle()) {
        cb = TYPESAFE_DOWNCAST(floatSlip->GetFirstChild(), TDockableControlBarEx);
        InsertDCBData(cb, dockSlip);
      }
      child = child->Next();
    }
    while (child != first);
  }

  first = GetFirstChild();
  if (first) {
    TWindow* child = first;
    do {
      TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(child, TDockableControlBarEx);
      InsertDCBData(cb, 0);
      child = child->Next();
    }
    while (child != first);
  }
}

void THarborEx::InsertDCBData(TDockableControlBarEx* cb, TDockingSlip* slip)
{
  TRect rect;
  if (cb && cb->GetHandle()) {
    cb->GetRect(rect);
    int len = cb->GetWindowTextLength();
    TCHAR title[MaxTitleLen];
    TDCBData* data = new TDCBData();
    if (len > 0) {
      cb->GetWindowText(title, MaxTitleLen);
      title[MaxTitleLen-1] = 0;
      _tcscpy(data->Title, title);
    }
    else
      *data->Title=0;
    data->DCB  = cb;
    data->Slip = slip;
    data->Rect = rect;

    // Insert the data. First the default sorted by id.
    // Then the non default sorted by id
    //
    uint i;
    if (data->DCB->IsDefault()) {
      for (i=0; i<DCBData.GetItemsInContainer(); i++) {
        if (DCBData[i]->DCB->IsDefault()) {
          if (data->DCB->Attr.Id < DCBData[i]->DCB->Attr.Id) {
            DCBData.AddAt(data, i);
            return;
          }
        }
        else
          break;
      }
      DCBData.AddAt(data, i);
      return;
    }
    else {
      for (i=0; i<DCBData.GetItemsInContainer(); i++) {
        if (DCBData[i]->DCB->IsDefault() == false) {
          if (data->DCB->Attr.Id < DCBData[i]->DCB->Attr.Id) {
            DCBData.AddAt(data, i);
            return;
          }
        }
      }
      DCBData.Add(data);
    }
  }
}

void THarborEx::CheckBeforeInsertDefaultGadget(int id)
// If the inserted gadget someone, that can be cut/paste and not
// copied, then search for them in all gadget windows and remove them
{
  TGadgetDesc* desc = GadgetDescriptors->Find(id);
  if (desc && !(desc->Attr & GADG_NOCUT) && (desc->Attr & GADG_NOCOPY)) {
    for (uint j=0; j<DCBData.GetItemsInContainer(); j++) {
      TDockableControlBarEx* dcb = DCBData[j]->DCB;
      TGadget* gad = dcb->FirstGadget();
      while (gad) {
        if (gad->GetId() == id) {
          dcb->RemoveEx(gad, true);
          return;
        }
        gad = gad->NextGadget();
      }
    }
  }
}

void THarborEx::InsertDefaultGadgets()
// Insert the default gadgets for all default toolbars
{
  GetDCBData();
  for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
    InsertDefaultGadget(i);
  }
}

void THarborEx::InsertDefaultGadget(int idx)
// Insert the default gadgets for toolbars with idx
{
  if (idx >= 0 && idx < (int)DCBData.GetItemsInContainer() &&
    DCBData[idx]->DCB->IsDefault()) {
      DCBData[idx]->DCB->InsertDefaultGadgets();
      DCBData[idx]->DCB->Invalidate();
      DCBData[idx]->DCB->UpdateWindow();
  }
}

HMENU THarborEx::GetToolbarMenu(bool getDCBData)
// Create the actual PopupMenu and return them
// Don't forget to destroy the menu by caller
{
  // if getDCBData == true a new DCBData Array will be created
  //
  if (getDCBData)
    GetDCBData();

  TMenu PopupMenu(GetModule()->LoadMenu(IDM_SHOWHIDECONTROLBARS));
  if ((HMENU)PopupMenu && PopupMenu.GetSubMenu(0)) {
    uint num = std::min(DCBData.GetItemsInContainer(), (uint)MaxDCBMenuItems);
    for (uint i=0; i<num; i++) {
      // &Toolbar > &0 controlbar title 0
      //            &1 controlbar title 1
      //            ...
      //            -------------------
      //            &Customize...
      int len = static_cast<int>(_tcslen(DCBData[i]->Title));
      TCHAR* title = new TCHAR[len+4]; // +4 for "&x " and terminator
      wsprintf(title, _T("&%d %s"), i, DCBData[i]->Title);

      UINT flags = MF_STRING | MF_BYPOSITION;
      if (DCBData[i]->DCB->IsWindowVisible())
        flags |= MF_CHECKED;

      ::InsertMenu(PopupMenu.GetSubMenu(0), i, flags, CM_SHOWHIDECTLBAR+i, title);
      delete[] title;
    }
  }
  return (HMENU)PopupMenu;
}

HMENU THarborEx::GetGadgetMenu()
{
  TMenu PopupMenu(GetModule()->LoadMenu(IDM_CUSTOMIZEGADGET));
  return (HMENU)PopupMenu;
}

void THarborEx::CmShowHideCtlBar(uint id)
{
  uint num = std::min(DCBData.GetItemsInContainer(), (uint)MaxDCBMenuItems);
  for (uint i=0; i<num; i++) {
    TDockableControlBarEx* cb = DCBData[i]->DCB;
    if (cb && cb->GetHandle()) {
      if (i == (id-CM_SHOWHIDECTLBAR)) {
        if (cb->IsWindowVisible())
          cb->Hide();
        else
          cb->Show();
        return;
      }
    }
  }
}

void THarborEx::CmCustomize()
{
  GetDCBData();
  CustomizeDlg = CreateCustomizeDlg(DecoratedFrame);
  CustomizeDlg->Create();
  CustomizeDlg->ShowWindow(SW_SHOW);
}

void THarborEx::CmGadgetReset()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonGadgetEx* bg = TYPESAFE_DOWNCAST(CustGadget, TButtonGadgetEx);
    if (bg) {
      bg->Reset();
    }
  }
}

void THarborEx::CmGadgetDelete()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(CustWindow,
      TDockableControlBarEx);
    if (dcb)
      dcb->RemoveEx(CustGadget, true);
  }
}

void THarborEx::CmGadgetCopyImage()
{
  if (CustGadgetType != IsNoGadget) {
    if (CustGadgetType == IsToolbarGadget) {
      TButtonGadgetEx* bg = TYPESAFE_DOWNCAST(CustGadget, TButtonGadgetEx);
      if (bg)
        CopiedBmpResId = bg->GetResId();
    }
    else {
      TButtonGadgetDesc* desc = TYPESAFE_DOWNCAST(
        GadgetDescriptors->Find(CustGadgetRef), TButtonGadgetDesc);
      if (desc)
        CopiedBmpResId = desc->BmpResId;
    }
  }
}

void THarborEx::CmGadgetPasteImage()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonGadgetEx* bg = TYPESAFE_DOWNCAST(CustGadget, TButtonGadgetEx);
    if (bg)
      bg->SetResId(CopiedBmpResId);
  }
}
//
void THarborEx::ExecuteGadgetDialog(TDecoratedFrame& frame, TButtonTextGadgetEx* btg)
{
  TButtonAppearanceDialog(&frame, btg).Execute();
}
//
void THarborEx::CmGadgetAppearance()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget, TButtonTextGadgetEx);
    if (btg)
      ExecuteGadgetDialog(DecoratedFrame, btg);
  }
}

void THarborEx::CmGadgetImage()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg)
      btg->SetDisplayType(Bitmap);
  }
}

void THarborEx::CmGadgetText()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg)
      btg->SetDisplayType(Text);
  }
}

void THarborEx::CmGadgetImageText()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg)
      btg->SetDisplayType(BitmapText);
  }
}

void THarborEx::CmGadgetBeginGroup()
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(CustWindow,
      TDockableControlBarEx);
    if (dcb) {
      bool beginGroup = false;
      TGadget* prevGad = dcb->PrevGadget(CustGadget);
      if (prevGad) {
        TGadgetDesc* desc = GadgetDescriptors->Find(prevGad->GetId());
        if (desc && desc->Type == TGadgetDesc::SeparatorGadgetDesc)
          beginGroup = true;
      }
      if (beginGroup)
        delete dcb->RemoveEx(prevGad, true);
      else {
        TGadget* gadget = GadgetDescriptors->ConstructGadget(0);
        if (gadget)
          dcb->InsertEx(*gadget, TGadgetWindow::After, prevGad);
      }
    }
  }
}

void THarborEx::CeGadgetReset(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg && btg->HasDefaultValues() == false) {
      TGadgetDesc* gd = GadgetDescriptors->Find(btg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        return;
      }
    }
  }
  ce.Enable(false);
}

void THarborEx::CeGadgetDelete(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TGadgetDesc* desc = GadgetDescriptors->Find(CustGadget->GetId());
    if (desc && !(desc->Attr & GADG_NODELETE)) {
      ce.Enable(true);
      return;
    }
  }
  ce.Enable(false);
}

void THarborEx::CeGadgetCopyImage(TCommandEnabler& ce)
{
  TGadgetDesc* desc = 0;
  if (CustGadgetType == IsToolbarGadget && CustGadget) {
    desc = GadgetDescriptors->Find(CustGadget->GetId());
  }
  else if (CustGadgetType == IsRefGadget) {
    desc = GadgetDescriptors->Find(CustGadgetRef);
  }

  if (desc) {
    TButtonGadgetDesc* bgd = TYPESAFE_DOWNCAST(desc, TButtonGadgetDesc);
    if (bgd && (bgd->Attr & GADG_BUTAPPEARANCE)) {
      ce.Enable(true);
      return;
    }
  }
  ce.Enable(false);
}

void THarborEx::CeGadgetPasteImage(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonGadgetEx* bg = TYPESAFE_DOWNCAST(CustGadget, TButtonGadgetEx);
    if (bg && CopiedBmpResId != 0) {
      TGadgetDesc* gd = GadgetDescriptors->Find(bg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        return;
      }
    }
  }
  ce.Enable(false);
}

void THarborEx::CeGadgetAppearance(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg) {
      TGadgetDesc* gd = GadgetDescriptors->Find(btg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        return;
      }
    }
  }
  ce.Enable(false);
}

void THarborEx::CeGadgetImage(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg) {
      TGadgetDesc* gd = GadgetDescriptors->Find(btg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        TDisplayType dispType = btg->GetDisplayType();
        ce.SetCheck((dispType == Bitmap) ? true : false);
        return;
      }
    }
  }
  ce.Enable(false);
  ce.SetCheck(false);
}

void THarborEx::CeGadgetText(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg) {
      TGadgetDesc* gd = GadgetDescriptors->Find(btg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        TDisplayType dispType = btg->GetDisplayType();
        ce.SetCheck((dispType == Text) ? true : false);
        return;
      }
    }
  }
  ce.Enable(false);
  ce.SetCheck(false);
}

void THarborEx::CeGadgetImageText(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(CustGadget,
      TButtonTextGadgetEx);
    if (btg) {
      TGadgetDesc* gd = GadgetDescriptors->Find(btg->GetId());
      if (gd->Attr & GADG_BUTAPPEARANCE) {
        ce.Enable(true);
        TDisplayType dispType = btg->GetDisplayType();
        ce.SetCheck((dispType == BitmapText) ? true : false);
        return;
      }
    }
  }
  ce.Enable(false);
  ce.SetCheck(false);
}

void THarborEx::CeGadgetBeginGroup(TCommandEnabler& ce)
{
  if (CustGadgetType == IsToolbarGadget && CustWindow && CustGadget) {

    TDockableControlBarEx* CustDCB = TYPESAFE_DOWNCAST(CustWindow,
      TDockableControlBarEx);
    if (CustDCB) {

      // Find out if the gadget is the first gadget,
      // the gadget before is a separator (a group begin)
      //
      ce.Enable(CustGadget != CustWindow->FirstGadget() ? true : false);
      TGadget* gad = CustDCB->PrevGadget(CustGadget);
      if (gad) {
        TGadgetDesc* desc = GadgetDescriptors->Find(gad->GetId());
        if (desc && desc->Type == TGadgetDesc::SeparatorGadgetDesc)
          ce.SetCheck(true);
        else
          ce.SetCheck(false);
      }
      return;
    }
  }
  ce.Enable(false);
}

void THarborEx::LoadSettings(THarborManagement& harborMan)
{
  uint j;
  int Count, Id, hintMode;
  uint flatStyle;
  TCHAR posstr[80];
  uint32 size = sizeof(uint);
  GetDCBData();

  owl::tstring str = harborMan.GetRegistryName() + owl::tstring(_T("\\")) + ToolbarsSection;
  TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());
  if (regKey.QueryValue(CountSection, 0, (uint8*)&Count,&size)!=ERROR_SUCCESS) {
    InsertDefaultGadgets();
  }
  if (regKey.QueryValue(TooltipsSection, 0, (uint8*)&ShowTooltips, &size) != ERROR_SUCCESS)
    return;
  if (regKey.QueryValue(AccKeysSection, 0, (uint8*)&WithAccel, &size) != ERROR_SUCCESS)
    return;
  if (regKey.QueryValue(DrawGripperSection, 0, (uint8*)&DrawGripper, &size) !=  ERROR_SUCCESS)
    return;
  if (regKey.QueryValue(FlatStyleSection, 0, (uint8*)&flatStyle, &size) != ERROR_SUCCESS)
    return;
  if (regKey.QueryValue(HintModeSection, 0, (uint8*)&hintMode, &size) != ERROR_SUCCESS)
    return;

  TGadgetWindow::EnableFlatStyle(flatStyle);

  SetHintMode((TGadgetWindow::THintMode)hintMode);

  for (int i=0; i<Count; i++) {
    wsprintf(posstr, _T("%d"), i+1);
    TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());
    if (regKey.QueryValue(posstr, 0, (uint8*)&Id, &size) != ERROR_SUCCESS)
      return;

    bool found = false;
    for (j=0; j<DCBData.GetItemsInContainer(); j++) {
      if (DCBData[j]->DCB->Attr.Id == Id) {
        DCBData[j]->DCB->LoadSettings(harborMan);
        found = true;
      }
    }

    if (found == false) {
      // Create new Toolbar
      //
      TDockableControlBarEx* dcb = new TDockableControlBarEx(Id, _T(""),
        &DecoratedFrame, false);
      Insert(*dcb, alNone);
      dcb->LoadSettings(harborMan);
    }
  }
}

void THarborEx::SaveSettings(THarborManagement& harborMan)
{
  GetDCBData();

  // delete the Toolbars tree and create new one
  //
  owl::tstring str = harborMan.GetRegistryName() + owl::tstring(_T("\\")) + ToolbarsSection;
  TRegKey::GetCurrentUser().NukeKey(str.c_str());
  TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());

  regKey.SetValue(CountSection, DCBData.GetItemsInContainer());
  regKey.SetValue(TooltipsSection, ShowTooltips);
  regKey.SetValue(AccKeysSection, WithAccel);
  regKey.SetValue(DrawGripperSection, DrawGripper);
  if (DCBData.GetItemsInContainer() > 0)
    regKey.SetValue(FlatStyleSection, DCBData[0]->DCB->GetFlatStyle());
  else
    regKey.SetValue(FlatStyleSection, TGadgetWindow::FlatStandard);

  regKey.SetValue(HintModeSection, HintMode);

  for (uint i=0; i<DCBData.GetItemsInContainer(); i++) {
    TCHAR posstr[80];
    wsprintf(posstr, _T("%d"), i+1);
    TRegKey regKey(TRegKey::GetCurrentUser(), str.c_str());
    regKey.SetValue(posstr, DCBData[i]->DCB->Attr.Id);
    DCBData[i]->DCB->SaveSettings(harborMan);
  }
}


} // OwlExt namespace
//======================================================================================

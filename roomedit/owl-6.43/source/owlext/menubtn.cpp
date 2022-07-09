// ****************************************************************************
// Copyright (C) 1997, 98 by Dieter Windau
// All rights reserved
//
// menubtn.cpp:  implementation file
// Version:      1.1
// Date:         24.10.1998
// Author:       Dieter Windau
//
// Collection of freeware OWL button classes
// TMenuButton shows a popup menu when the user click on the button
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02 and Windows NT 4.0 SP3
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/menubtn.rh>
#include <owlext/menubtn.h>

using namespace owl;

namespace OwlExt {

// ********************* TMenuButton ******************************************

DEFINE_RESPONSE_TABLE1(TMenuButton, TGlyphButton)
EV_MESSAGE(BM_SETSTATE, BmSetState),
END_RESPONSE_TABLE;

TMenuButton::TMenuButton(TWindow* parent, int Id, LPCTSTR text, int X, int Y,
             int W, int H, bool isDefault, TModule* module):
TGlyphButton(parent, Id, text, X, Y, W, H, btCustom, isDefault, module)
{
  HMenu = 0;
  CmdTarget = 0;
  SetGlyph(IDB_DOWNARROW, module);
  SetLayoutStyle(lsH_STSG);
}

TMenuButton::TMenuButton(TWindow* parent, int resourceID, TModule* module)
:
TGlyphButton(parent, resourceID, btCustom, module)
{
  HMenu = 0;
  CmdTarget = 0;
  SetGlyph(IDB_DOWNARROW, module);
  SetLayoutStyle(lsH_STSG);
}

void TMenuButton::SetMenu(HMENU hMenu)
// Set the popupMenu. Command Target is the parent window.
{
  PRECONDITION(hMenu);
  PRECONDITION(Parent);
  PRECONDITION(Parent->GetHandle());
  if (hMenu && Parent && Parent->GetHandle()) {
    HMenu = hMenu;
    CmdTarget = Parent->GetHandle();
  }
}

void TMenuButton::SetMenu(HMENU hMenu, HWND cmdTarget)
// Set the popupMenu and the commando target window
{
  PRECONDITION(hMenu);
  PRECONDITION(cmdTarget);
  if (hMenu && cmdTarget) {
    HMenu = hMenu;
    CmdTarget = cmdTarget;
  }
}

TResult TMenuButton::BmSetState(TParam1 param1, TParam2 /*param2*/)
{
  if (param1) {

    // Needs hilight look
    //
    if (!IsSet(biPushed)) {
      Set(biPushed);

      // It can be that we don't have the focus (e.g. the user use Alt+key)
      //
      if (!IsSet(biFocus))
        SetFocus();

      PaintNow();
      TrackPopupMenu();
    }
  }
  else {
    // Needs normal look
    //
    if (IsSet(biPushed)) {
      Clear(biPushed);
      PaintNow();
    }
  }
  return 0;
}

void TMenuButton::TrackPopupMenu()
{
  TPoint tlp = GetClientRect().TopLeft();
  TPoint brp = GetClientRect().BottomRight();
  ClientToScreen(tlp);
  if (HMenu && CmdTarget) {
    ClientToScreen(brp);
    TPMPARAMS tpmParams;
    tpmParams.cbSize = sizeof(TPMPARAMS);
    tpmParams.rcExclude.top = tlp.y;
    tpmParams.rcExclude.left = 0;
    tpmParams.rcExclude.bottom = brp.y;
    tpmParams.rcExclude.right = 32000;
    ::TrackPopupMenuEx(HMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
      tlp.x, tlp.y, CmdTarget, &tpmParams);
    SendMessage(BM_SETSTATE, false);
  }
}

} // OwlExt namespace

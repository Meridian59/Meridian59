// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1997, 98 by Dieter Windau
// All rights reserved
//
// dlgtoolbox.h: header file
// Version:      1.0
// Date:         24.09.1998
// Author:       Dieter Windau
//
// Collection of freeware OWL button classes
// TDialogToolBox is derived from TToolBox and can be used inside a dialog
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

#include <owlext/dlgtoolb.h>

namespace OwlExt {

using namespace owl;

// ********************* TDialogToolBox ***************************************

TDialogToolBox::TDialogToolBox(TWindow* parent, uint resourceId, int numColumns,
                 int numRows, TTileDirection direction)
                 :
TToolBox(parent, numColumns, numRows, direction, 0)
{
  ResourceId = resourceId;
  WantTooltip = true;
  HintMode = EnterHints;
}

TDialogToolBox::~TDialogToolBox()
{
}

void TDialogToolBox::SetupWindow()
{
  TToolBox::SetupWindow();

  TRect Rect;
  ::GetWindowRect(::GetDlgItem(Parent->GetHandle(), ResourceId), &Rect);
  Parent->ScreenToClient(Rect.TopLeft());

  Rect.left--;  // hide rect border
  Rect.top--;
  SetWindowPos(0, Rect.left, Rect.top, 0, 0,
    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

TResult TDialogToolBox::EvCommand(uint id, THandle hWndCtl, uint notifyCode)
{
  // Prior versions of TGadgetWindow relied on TWindow's EvCommand for
  // dispatching WM_COMMAND events. This required that one derives from
  // a decoration class (eg. TControlbar, TToolbox) to handle control
  // notifications. The current version uses a more generalized logic
  // involving the CommandTarget and a frame ancestor class. This allows
  // a client window to handle notifications of a control in a toolbar
  // without using a TControlbar-derived class.
  // However, if you need to previous behaviour, simply invoke TWindow's
  // EvCommand from this handler.

  SendNotification(Parent->GetHandle(), id, notifyCode, hWndCtl);
  return TResult(true);
}

} // OwlExt namespace
//==============================================================================

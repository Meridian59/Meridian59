//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TRollDialog, an automatic roll-up dialog
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/rolldial.h>
#include <owl/menu.h>

namespace owl {

OWL_DIAGINFO;

//
// Fibonacci sequence used for accelerated animation.
//
static uint32 Fibonacci(int n)
{
  if (n == 0 || n == 1) {
    return 1;
  }
  uint32 older  = 1;
  uint32 old    = 1;
  uint32 answer = older + old;

  for (int i = 2; i < n; i++) {
    older  = old;
    old    = answer;
    answer = old + older;
  }
  return answer;
}


DEFINE_RESPONSE_TABLE1(TRollDialog, TDialog)
  EV_WM_SYSCOMMAND,
END_RESPONSE_TABLE;

//
/// Sets up data members for the various properties of the dialog.
//
TRollDialog::TRollDialog(TWindow* parent, TResId resId, bool animated,
                         bool fullSize, TModule* module)
:
  TDialog(parent, resId, module),
  IsFullSize(true),
  InitialFullSize(fullSize),
  WantAnimation(animated)
{
}

//
/// Adds the shrink system menu option, if desired. This also shrinks the dialog if
/// that option was chosen.
//
void
TRollDialog::SetupWindow()
{
  TDialog::SetupWindow();

  // Add to system menu if present
  //
  if (GetStyle() & WS_SYSMENU) {
    TSystemMenu menu(*this);
#if BI_MSG_LANGUAGE == 0x0411
    menu.AppendMenu(MF_STRING, SC_ROLLDIALOG, "Žûk(&S)");
#else
    menu.AppendMenu(MF_STRING, SC_ROLLDIALOG, _T("&Shrink"));
#endif
    HasSystemMenu = true;
  }

  if (!InitialFullSize)
    Shrink();
}

//
/// Resize the dialog to either full size or minimal size.
//
void
TRollDialog::Resize(bool currentlyFullSize)
{
  TRect r = GetWindowRect();
  int oldBottom = r.bottom;

  if (WantAnimation) {
    int delta = 0;
    for (int step = 0; delta < Height; step++) {
      if (currentlyFullSize)
        r.bottom = oldBottom - delta;
      else
        r.bottom = oldBottom + delta;
      MoveWindow(r, true);     // !CQ Use SetWindowPos(...,SWP_NOMOVE) ?
      delta = (int)Fibonacci(step);
    }
  }

  if (currentlyFullSize)
    r.bottom = oldBottom - Height;
  else
    r.bottom = oldBottom + Height;

  MoveWindow(r, true);
}

//
/// Event handler for the system menu option "shrink". Toggles the system menu
/// choice to "expand".
//
void
TRollDialog::Shrink()
{
  if (IsFullSize) {
    Height = GetClientRect().Height() + 1;

    Resize(IsFullSize);
    IsFullSize = false;

    if (HasSystemMenu) {
      TSystemMenu menu(*this);
      menu.ModifyMenu(SC_ROLLDIALOG, MF_BYCOMMAND | MF_STRING, SC_ROLLDIALOG,
#if BI_MSG_LANGUAGE == 0x0411
          "“WŠJ(&E)");
#else
          _T("&Expand"));
#endif
    }
  }
}

//
/// Event handler for the system menu option "expand". Toggles the system menu
/// choice to "shrink".
//
void
TRollDialog::Expand()
{
  if (!IsFullSize) {
    Resize(IsFullSize);
    IsFullSize = true;

    if (HasSystemMenu) {
      TSystemMenu menu(*this);
      menu.ModifyMenu(SC_ROLLDIALOG, MF_BYCOMMAND | MF_STRING, SC_ROLLDIALOG,
#if BI_MSG_LANGUAGE == 0x0411
          "Žûk(&S)");
#else
          _T("&Shrink"));
#endif
    }
  }
}

//
/// Event handler for the system menu choice. Calls either TRollDialog::Shrink or
/// TRollDialog::Expand.
//
void
TRollDialog::EvSysCommand(uint cmdType, const TPoint& point)
{
  uint command = cmdType & 0xFFF0;

  if (command == SC_ROLLDIALOG || command == SC_RESTORE) {
    if (IsFullSize)
      Shrink();
    else
      Expand();
  }
  else if (command == SC_MINIMIZE)
    Shrink();
  else if (command == SC_MAXIMIZE)
    Expand();
  else
    TDialog::EvSysCommand(cmdType, point);
}


} // OWL namespace
/* ========================================================================== */


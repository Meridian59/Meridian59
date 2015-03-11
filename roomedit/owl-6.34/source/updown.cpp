//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of the TUpDown class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/updown.h>
#include <owl/gdiobjec.h>
#include <owl/uihelper.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

//
// Constants used when ObjectWindows provides the underlying implementation of
// the Up-down control...
//
const uint UpDownTimerID1 = 0x1000;     // Initial timer (for startup delay)
const uint UpDownTimerID2 = 0x1001;     // Regular timer (for notifications)
const uint InitDelay      = 500;        // Initial delay before notifying
const uint RepeatDelay    = 50;         // Interval between notifications

DEFINE_RESPONSE_TABLE1(TUpDown, TControl)
  EV_WM_VSCROLL,
  EV_WM_HSCROLL,
END_RESPONSE_TABLE;

//
/// Constructor of UpDown control.
//
TUpDown::TUpDown(TWindow* parent, int id, int x, int y, int w, int h,
                 TWindow* buddy, TModule* module)
:
  TControl(parent, id, _T(""), x, y, w, h, module),
  Buddy(buddy),
  Lower(0),
  Upper(100),
  Pos(0)
{
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP;

  CHECK(TCommCtrl::IsAvailable());
}

//
/// Constructor to an alias of an up-down control that is part of a dialog resource.
//
TUpDown::TUpDown(TWindow* parent, int resourceId, TWindow* buddy, TModule* module)
:
  TControl(parent, resourceId, module),
  Buddy(buddy),
  Lower(0),
  Upper(100),
  Pos(0)
{
  CHECK(TCommCtrl::IsAvailable());
}

//
/// Returns the ClassName of the underlying control.
/// \note The name returned depends upon whether you're using an operating system
/// which provides the underlying implementation of UPDOWN controls. Also, when
/// emulating, we choose to return a distinct class name. Although this is not
/// strictly necessary with ObjectWindows, it facilitates the debugging process.
//
TWindow::TGetClassNameReturnType
TUpDown::GetClassName()
{
  PRECONDITION(TCommCtrl::IsAvailable());
  return UPDOWN_CLASS;

}

//
/// Overriden to invoke the OS' 'CreateUpDownControl' method on systems that use the
/// native implementation of UPDOWN controls.
//
TWindow::TPerformCreateReturnType
TUpDown::PerformCreate(int arg)
{
  InUse(arg);

  PRECONDITION(TCommCtrl::IsAvailable());
  THandle h = TCommCtrl::Dll()->CreateUpDownControl(
    Attr.Style,
    Attr.X, Attr.Y, Attr.W, Attr.H,
    Parent ? Parent->GetHandle() : 0,
    Attr.Id,
    *GetModule(),
    Buddy ? Buddy->GetHandle() : 0,
    Upper, Lower, Pos
    );
  OWL_SET_OR_RETURN_HANDLE(h);
}

//
/// Keeps TWindow from rerouting these. Must be left as is for updown control.
//
void
TUpDown::EvVScroll(uint, uint, HWND)
{
  DefaultProcessing();
}

//
/// Keeps TWindow from rerouting these; must be left as is for updown control.
//
void
TUpDown::EvHScroll(uint, uint, HWND)
{
  DefaultProcessing();
}



} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of the TAnimateCtrl class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/animctrl.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);

/// Constructor for a TAnimateCtrl to be associated with a new underlying
/// control.
//
TAnimateCtrl::TAnimateCtrl(TWindow* parent, int id, int x, int y, int w, int h,
                           TModule* module)
:
  TControl(parent, id, _T(""), x, y, w, h, module)
{
  InitializeCommonControls(ICC_ANIMATE_CLASS);
  Attr.Style = WS_CHILD | WS_VISIBLE | ACS_CENTER | ACS_TRANSPARENT;
  TRACEX(OwlCommCtrl, OWL_CDLEVEL, _T("TAnimateCtrl constructed @") << (void*)this);
}


/// Constructor to alias a control defined in a dialog resource
//
TAnimateCtrl::TAnimateCtrl(TWindow* parent, int resourceId, TModule* module)
:
 TControl(parent, resourceId, module)
{
  InitializeCommonControls(ICC_ANIMATE_CLASS);
  TRACEX(OwlCommCtrl, OWL_CDLEVEL, _T("TAnimateCtrl constructed @") << (void*)this);
}

/// Destructor
//
TAnimateCtrl::~TAnimateCtrl()
{
  TRACEX(OwlCommCtrl, OWL_CDLEVEL, _T("TAnimateCtrl deleted @") << (void*)this);
}

//
/// Returns the predefined class registered by the Common Control library
/// for the Animation control.
//
auto TAnimateCtrl::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{ANIMATE_CLASS};
}

//
/// Opens an .AVI file and displays the first frame.
/// The 'res' parameter may be a resourceId if the .AVI is from a resource.
/// It may also be a full path of an .AVI file.
/// Use NULL (0) as 'res' to close any previously opened .AVI file.
/// If 'module' is not NULL, the resource is opened from the given module.
/// If 'module' is NULL, the resource is opened from the module associated with the control, i.e.
/// the module returned by GetModule.
//
bool
TAnimateCtrl::Open(LPCTSTR res, TModule* module)
{
  const auto moduleHandle = module ? module->GetHandle() : 
    GetModule() ? GetModule()->GetHandle() : 
    nullptr;
  return SendMessage(ACM_OPEN, reinterpret_cast<TParam1>(moduleHandle), TParam2(res)) != 0;
}


/// Plays the .AVI file from frame index 'start' to frame index 'end'.
/// The 'repeat' parameter is the number of times to play the frames.
/// Use -1 for repeat to play indefinitely.
/// \note The control plays the clip in the backgroud while the
///       current thread continues executing.
//
bool
TAnimateCtrl::Play(uint16 start, uint16 end, uint repeat)
{
  return SendMessage(ACM_PLAY, repeat, MkUint32(start, end)) != 0;
}

/// Stops playing the current AVI file.
//
void
TAnimateCtrl::Stop()
{
  SendMessage(ACM_STOP);
}


/// Seek to frame index 'frame'. The value is zero-based and must not
/// exceed 65,536.
//
bool
TAnimateCtrl::Seek(uint16 frame)
{
  return Play(frame, frame, 0);
}


} // OWL namespace
/* ========================================================================== */


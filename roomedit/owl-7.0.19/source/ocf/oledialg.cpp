//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TOleDialog. This defines the basic behavior of all
/// Ole dialogs.
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/oledialg.h>

namespace ocf {

using namespace owl;

OWL_DIAGINFO;

DEFINE_RESPONSE_TABLE2(TOleDialog, TOleWindow, TDialog)
  EV_OC_VIEWSETSITERECT,
END_RESPONSE_TABLE;

//
/// Creates a TOleDialog object. Registers an OCX window class for the application
/// by calling TRegisterOcxWnd. Sets the wfAutoCreate flag for the object.
//
TOleDialog::TOleDialog(TWindow* parent, TResId resId, TModule* module)
           :TOleWindow(parent, module), TDialog(parent, resId, module)
{
  // Register OCX class
  //
  static TRegisterOcxWnd regWnd(GetApplication()->GetHandle());

  // Override TDialog's desire to not be autocreated
  //
  SetFlag(wfAutoCreate);
}

//
/// Destroys the TOleDialog object.
//
TOleDialog::~TOleDialog()
{
  Destroy();
}

//
/// Calls the SetupWindow functions of the TOleWindow and TDialog base classes. Then
/// calls CheckChild on the dialog box handle and child window handles, destroying
/// any corresponding OCX windows. (The dummy OCX windows are replaced by OCX
/// controls.)
//
void
TOleDialog::SetupWindow()
{
  // Call parents setup
  //
  TOleWindow::SetupWindow();
  TDialog::SetupWindow();

  HWND hWnd = ::GetWindow(GetHandle(), GW_CHILD);
  if (hWnd) {
    // Check if it is a control class and if so, load it
    //
    HWND hWndControl = CheckChild(hWnd);

    // Loop through all the children and look for 'stub' windows
    // that should be replaced with an OCX control...
    //
    while ((hWnd = ::GetWindow(hWnd, GW_HWNDNEXT)) != 0) {
      if (hWndControl)
        ::DestroyWindow(hWndControl);
      hWndControl = CheckChild(hWnd);
    }

    if (hWndControl)
      ::DestroyWindow(hWndControl);
  }
}

//
/// Called when no messages are waiting to be processed. Calls the IdleAction
/// functions of the TOleWindow and TDialog base classes.
//
bool
TOleDialog::IdleAction(long idleCount)
{
  TOleWindow::IdleAction(idleCount);
  return TDialog::IdleAction(idleCount);
}

/// Called before a Windows message (stored in msg) is dispatched. Calls the
/// PreProcessMsg functions of the TOleWindow and TDialog base classes.
bool
TOleDialog::PreProcessMsg(MSG& msg)
{
  if (TOleWindow::PreProcessMsg(msg) || TDialog::PreProcessMsg(msg))
    return true;
  return false;
}

//
/// Handles OC_VIEWSITERECT messages. Calls the EvOcViewSetSiteRect function of the
/// TOleWindow base class. Then returns false, forcing OCX controls to keep the
/// sizes and positions defined in their resource scripts.
///
/// Overrides SetSiteRect from TOleWindow since we want to force
/// our OCX Controls to keep the size and position defined in
/// the resource file. This is done by simply returning false
//
bool
TOleDialog::EvOcViewSetSiteRect(TRect * rect)
{
  TOleWindow::EvOcViewSetSiteRect(rect);
  return false;
}

//
/// Checks to see if a window handle (HWND) corresponds to an OCX window. If so,
/// CheckChild calls LoadControl on the HWND and returns the HWND. (The HWND should
/// be used by the calling procedure to destroy the window, which has been replaced
/// by an OCX control.) Otherwise, CheckChild returns 0.
/// The CheckChild function is used by the TOleDialog::SetupWindow function.
//
HWND
TOleDialog::CheckChild(HWND hWndChild)
{
  TCHAR className [50];
  if (::GetClassName(hWndChild, className, 50)) {
    if (_tcscmp(className, OCX_STUB_CLASS) == 0) {
      LoadControl(hWndChild);
      return hWndChild;
    }
  }
  return 0;
}

//
/// Determines the ProgId and position of the window corresponding to hControl. Then
/// creates a TOcControl object that is initialized with the control's class
/// (calculated from ProgId) and position.
//
void
TOleDialog::LoadControl(HWND hControl)
{
  // Get stub control's location
  //
  TRect ControlRect;
  ::GetWindowRect(hControl, &ControlRect);
  TPoint TopLeft(ControlRect.left, ControlRect.top);
  ::ScreenToClient(GetHandle(), &TopLeft);
  TPoint BottomRight(ControlRect.right, ControlRect.bottom);
  ::ScreenToClient(GetHandle(), &BottomRight);
  ControlRect.Set(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y);

  // Get ProgId
  //
  TCHAR progId [80];
  ::GetWindowText(hControl, progId, 80);

  // Create control with specified progid at location
  //
  InsertControl(TString(progId), &ControlRect, ::GetWindowLong(hControl, GWL_ID));
}

//
/// Registers an OCX window for the specified instance of the application.
//
TRegisterOcxWnd::TRegisterOcxWnd(HINSTANCE hInst)
                :HAppInst(hInst)
{
  // Registration for OCX window
  //
  WNDCLASS wndClass;

  wndClass.style = CS_GLOBALCLASS;   // Global registration
  wndClass.lpfnWndProc = ::DefWindowProc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = 0;
  wndClass.hInstance = HAppInst;
  wndClass.hIcon = 0;
  wndClass.hCursor = 0;
  wndClass.hbrBackground = 0;
  wndClass.lpszMenuName = 0;
  wndClass.lpszClassName = OCX_STUB_CLASS;

  RegisterClass(&wndClass);
}

//
/// Unregister OCX stub-window class
//
TRegisterOcxWnd::~TRegisterOcxWnd()
{
  UnregisterClass(_T("OCX"), HAppInst);
}


} // OCF namespace

//==============================================================================



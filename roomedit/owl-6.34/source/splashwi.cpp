//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TSplashWindow
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/splashwi.h>
#include <owl/gdiobjec.h>
#include <owl/layoutwi.h>
#include <owl/pictwind.h>
#include <owl/static.h>
#include <owl/gauge.h>
#include <owl/uimetric.h>

namespace owl {

OWL_DIAGINFO;

//
// ID for creating a timer.
//
const int TimerId = 1;

//
// Threshold value for removing the splash screen.
//
const int PercentThreshold = 95;

//
// Response table for the splash window.
//
DEFINE_RESPONSE_TABLE1(TSplashWindow, TLayoutWindow)
  EV_WM_LBUTTONDOWN,
  EV_WM_TIMER,
END_RESPONSE_TABLE;

//
/// Constructor to create a splash screen. The parameters width and height are the
/// dimensions of the window unless the style ShrinkToFit is used (in which case,
/// the size of the DIB is used). The timeOut parameter is the number of
/// milliseconds to wait until the splash screen closes itself. Use 0 to not
/// automatically close the splash screen. The splash screen does not assume
/// ownership of the DIB; a private copy is made. 
//
TSplashWindow::TSplashWindow(const TDib& dib, int width, int height,
  int style, uint timeOut, LPCTSTR title, TModule* module)
:
  TLayoutWindow(0, title, module),
  Style(style),
  Static(0),
  Gauge(0),
  PictWindow(0),
  TimeOut(timeOut),
  CapturedMouse(false)
{
  if (HasStyle(ShrinkToFit)) {
    Attr.W = dib.Width();
    Attr.H = dib.Height();
  }
  else {
    Attr.W = width;
    Attr.H = height;
  }

  Attr.Style = WS_POPUP | WS_VISIBLE | WS_BORDER;

  // If there's a title add the WS_CAPTION style. If ShrinkToFit is set
  // we also want to adjust the height of the window to account for the
  // caption.
  //
  if( title && ::_tcslen(title)){
    Attr.Style |= WS_CAPTION;
    if( HasStyle(ShrinkToFit) )
      Attr.H += ::GetSystemMetrics( SM_CYCAPTION );
  }

  // Ready the layout metrics
  //
  int heightPercentDW = 100;
  TLayoutMetrics lmPictWindow;
  TLayoutMetrics lmGauge;
  TLayoutMetrics lmStatic;

  // Center the dib by default unless ShrinktoFit is set.
  //
  const TPictureWindow::TDisplayHow dispHow = HasStyle(ShrinkToFit) ?
    TPictureWindow::UpperLeft : TPictureWindow::Center;

  // Create a a picture window to handle the rendering of the bitmap.
  // Note that we must make a copy of the DIB since TPictureWindow grabs ownership.
  //
  PictWindow = new TPictureWindow(this, new TDib(dib), dispHow, _T(""), module);

  lmPictWindow.X.SameAs(lmParent, lmLeft);
  lmPictWindow.Y.SameAs(lmParent, lmTop);
  lmPictWindow.Width.SameAs(lmParent, lmWidth);

  // Create optional static control
  //
  if (HasStyle(MakeStatic)) {
    const int StaticPercent = 10;
    Static = new TStatic(this, 1, _T(""), 0, 0, 0, 0, 0, module);
    Static->GetWindowAttr().Style |= SS_CENTER;
    heightPercentDW -= StaticPercent;
    lmStatic.X.SameAs(lmParent, lmLeft);
    lmStatic.Width.SameAs(lmParent, lmWidth);
    lmStatic.Height.PercentOf(lmParent, StaticPercent, lmHeight);

    lmStatic.Y.Below(PictWindow, 1);
    SetChildLayoutMetrics(*Static, lmStatic);
  }

  // Create optional gauge control
  //
  if (HasStyle(MakeGauge)) {
    const int GaugePercent = 10;
    Gauge = new TGauge(this, _T("%d%%"), 2, 0, 0, 0, 0, true, 0, module);
    heightPercentDW -= GaugePercent;
    lmGauge.X.SameAs(lmParent, lmLeft);
    lmGauge.Width.SameAs(lmParent, lmWidth);
    lmGauge.Height.PercentOf(lmParent, GaugePercent, lmHeight);

    if (HasStyle(MakeStatic))
      lmGauge.Y.Below(Static, 1);
    else
      lmGauge.Y.Below(PictWindow, 1);

    SetChildLayoutMetrics(*Gauge, lmGauge);
  }

  lmPictWindow.Height.PercentOf(lmParent, heightPercentDW, lmHeight);
  SetChildLayoutMetrics(*PictWindow, lmPictWindow);
}

//
/// Deletes the child controls.
//
TSplashWindow::~TSplashWindow()
{
  delete Static;
  delete Gauge;
  delete PictWindow;
}

//
/// After the window has been created, this centers the window and makes it topmost.
//
void
TSplashWindow::SetupWindow()
{
  TLayoutWindow::SetupWindow();

  // Center window, make topmost and adjust size to accomidate static
  // and gauge.
  //
  TRect r  = GetWindowRect();

  if( HasStyle(MakeGauge) && HasStyle(MakeStatic) )  {
    int deltaH = Gauge->GetWindowAttr().H;
    r.Inflate( 0, deltaH );
  }

  TRect fullRect(0, 0, TUIMetric::CxScreen, TUIMetric::CyScreen);

  int x = (fullRect.Width() - r.Width()) / 2;
  int y = (fullRect.Height() - r.Height()) / 2;
  r.Offset(x, y);
  SetWindowPos(HWND_TOPMOST, r, SWP_SHOWWINDOW);

  if (HasStyle(MakeGauge)) {
    // Set the range
    //
    GetGauge()->SetRange(0, 100);
  }

  if (GetTimeOut() != 0) {
    // Create the timer
    //
    SetTimer(TimerId, GetTimeOut());
  }

  // Trap the mouse click
  //
  if (HasStyle(CaptureMouse)) {
    SetCapture();
    SetCursor(0, IDC_ARROW);
    CapturedMouse = true;
  }
}

//
/// Before the window closes, and if the mouse has been captured, this releases it now.
//
void
TSplashWindow::CleanupWindow()
{
  if (CapturedMouse) {
    ReleaseCapture();
    CapturedMouse = false;
  }

  if (GetTimeOut() != 0)
    KillTimer(TimerId);
}

//
// Overload Create so that we can force the window to paint asap.
//
bool
TSplashWindow::Create()
{
   bool retval = TLayoutWindow::Create();
   UpdateWindow();
   return retval;
}

//
/// Changes the text within the static control. If the splash screen does not have a
/// static control, this doesn't do anything.
//
void
TSplashWindow::SetText(LPCTSTR text)
{
  if (HasStyle(MakeStatic))
    if (GetStatic() && GetStatic()->IsWindow()) {
      GetStatic()->SetText(text);
      GetApplication()->PumpWaitingMessages();
    }
}

//
/// Sets the percentage done for the gauge control. If the splash screen does not
/// have a gauge control, this doesn't do anything.
//
void
TSplashWindow::SetPercentDone(int percent)
{
  if (HasStyle(MakeGauge) && IsWindow()) {
    if (GetGauge())
      GetGauge()->SetValue(percent);

    if (percent > PercentThreshold) {
      // Set up the timer
      //
      if (GetTimeOut() != 0)
        SetTimer(TimerId, GetTimeOut());
    }
    // and last
    if (GetGauge())
      GetApplication()->PumpWaitingMessages();
  }
}

//
/// If the user clicks on the splash screen and the CaptureMouse style is on, this
/// closes the splash screen.
//
void
TSplashWindow::EvLButtonDown(uint /*modKeys*/, const TPoint& /*point*/)
{
  if (HasStyle(CaptureMouse)) {
    if (HasStyle(MakeGauge))
      if (GetGauge()->GetValue() < PercentThreshold)
        return;
// !BB  CloseWindow();
      SendMessage(WM_CLOSE);
  }
}

//
/// Handler for the timer event. Closes the window.
//
void
TSplashWindow::EvTimer(uint /*timerId*/)
{
  if (HasStyle(MakeGauge)) {
    if (GetGauge()->GetValue() < PercentThreshold) {
      // If less than 90% and has a gauge, immediately return
      //
      return;
    }
  }
// !BB  CloseWindow();
  SendMessage(WM_CLOSE);
}

} // OWL namespace
/* ========================================================================== */


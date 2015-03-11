//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TTinyCaption
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/tinycapt.h>
#include <owl/gdiobjec.h>
#include <owl/uimetric.h>

#if !defined(WS_EX_TOOLWINDOW)
# define WS_EX_TOOLWINDOW         0x00000080L
#endif

namespace owl {

OWL_DIAGINFO;


//
// We only want to search this mixin for events, so don't include any base
// classes in Find()
//
DEFINE_RESPONSE_TABLE(TTinyCaption)
  EV_WM_NCHITTEST,
  EV_WM_NCPAINT,
  EV_WM_NCCALCSIZE,
  EV_WM_NCLBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONUP,
  EV_WM_NCACTIVATE,
  EV_WM_SYSCOMMAND,
END_RESPONSE_TABLE;

//
/// Constructs a TTinyCaption object attached to the given parent window.
/// Initializes the caption font to 0 and TCEnabled to false so that the tiny
/// caption bar is not displayed automatically.
///
/// Rely on TWindow's default ctor since we will always be mixed-in and another
/// window will perform Init()
//
TTinyCaption::TTinyCaption()
{
  TCEnabled = false;
  CaptionFont = 0;
}

//
/// Destroys a TTinyCaption object and deletes the caption font.
//
TTinyCaption::~TTinyCaption()
{
  delete CaptionFont;
}

//
/// Activates the tiny caption bar. By default, EnableTinyCaption replaces the
/// system window with a tiny caption window that does not close when the system
/// window is clicked. If the closeBox argument is true, clicking on the system menu
/// will close the window instead of bringing up the menu. You can use
/// EnableTinyCaption to hide the window if you are using a tiny caption in a
/// derived class. To diminish the tiny caption bar, try the following values:
/// \code
/// EnableTinyCaption(30, true);
/// \endcode
/// To maximize the tiny caption bar, use these values:
/// \code
/// EnableTinyCaption(48, true);
/// \endcode
//
void
TTinyCaption::EnableTinyCaption(int captionHeight, bool closeBox)
{
  Border.cx = TUIMetric::CxBorder;
  Border.cy = TUIMetric::CyBorder;

  // Get width of window borders, these will vary with type of window
  // Really 3 styles are supported:
  //   Fixed frame - WS_DLGFRAME, no WS_BORDER, WS_THICKFRAME optional and
  //                 only affects sizability, not look
  //   Thick frame - WS_BORDER + WS_THICKFRAME, visible thick frame for sizing
  //   Thin frame  - WS_BORDER only. No sizing.
  //
  if ((Attr.Style & WS_CAPTION) == WS_DLGFRAME) {
    Frame.cx = TUIMetric::CxFixedFrame;
    Frame.cy = TUIMetric::CyFixedFrame;
  }
  else {
    Attr.Style |= WS_BORDER;     // must have at least a border
    Attr.Style &= ~WS_DLGFRAME;  // but never a caption (WS_BORDER+WS_DLGFRAME)
    if (Attr.Style & WS_THICKFRAME) {
      Frame.cx = TUIMetric::CxSizeFrame;
      Frame.cy = TUIMetric::CySizeFrame;
    }
    else {
      Frame = Border;
    }
  }

  CloseBox = closeBox;

  // If running under Win95 or NT NewShell, then use its extended style to
  // become a tiny caption
  //
  Attr.Style |= WS_CAPTION;
  Attr.ExStyle |= WS_EX_TOOLWINDOW;
  if (closeBox)
    Attr.Style |= WS_SYSMENU;
  CaptionHeight = captionHeight ? captionHeight : TUIMetric::CySmCaption.Get();
  // !CQ Need to keep CaptionHeight sync'd on sys changes?
  return;
}

//
/// Return where in the non client area we are.  We only handle caption
/// bar area
//
/// Responds to a cursor move or press of a mouse button by calling DoNCHitTest. If
/// DoNCHitTest does not return esComplete, EvNCHitTest calls TWindow::EvNCHitTest.
//
uint
TTinyCaption::EvNCHitTest(const TPoint& screenPt)
{
  uint er = 0;
  if (DoNCHitTest(screenPt, er) == esComplete)
    return er;
  return TWindow::EvNCHitTest(screenPt);
}

//
/// If the caption bar is not enabled, returns esPartial. Otherwise, sends a message
/// to the caption bar that the mouse or the cursor has moved, and returns
/// esComplete.
//
TEventStatus
TTinyCaption::DoNCHitTest(const TPoint& screenPt, uint& evRes)
{
  if (!TCEnabled)
    return esPartial;

  // Check style bits to see what to paint
  //
  long style = GetStyle();
  bool hasSysMenu, hasMaximize, hasMinimize;
  hasSysMenu = hasMaximize = hasMinimize = false;
  if (style & WS_SYSMENU)
    hasSysMenu = true;
  if (style & WS_MAXIMIZEBOX)
    hasMaximize = true;
  if (style & WS_MINIMIZEBOX)
    hasMinimize = true;

  // Convert to window coordinates
  //
  TPoint winPt = screenPt - GetWindowRect().TopLeft();

  if ((hasSysMenu || CloseBox) && GetSysBoxRect().Contains(winPt)) {
    evRes = HTSYSMENU;
    return esComplete;
  }
  else if (hasMinimize && GetMinBoxRect().Contains(winPt)) {
    evRes = HTMINBUTTON;
    return esComplete;
  }
  else if (hasMaximize && GetMaxBoxRect().Contains(winPt)) {
    evRes = HTMAXBUTTON;
    return esComplete;
  }

  // CaptionRect includes buttons so make sure it's last checked
  // Should modify this one to allow clicking in left, top, right thin
  // borders of caption
  //
  else if (GetCaptionRect().Contains(winPt)) {
    evRes = HTCAPTION;
    return esComplete;
  }
  else {
    return esPartial;
  }
}

//
/// Responds to a request to change a title bar or icon. Paints the indicated device
/// context or display screen and does any special painting required for the caption.
/// 
/// We only need to paint the caption. Someone else will paint the borders
//
void TTinyCaption::EvNCPaint(HRGN /*region*/)
{
  DefaultProcessing();   // Default border painting
  DoNCPaint();           // Then our special caption painting
}

//
/// If the caption bar is not enabled or is iconized, returns esPartial. Otherwise,
/// gets the focus, paints the caption, and returns esPartial, thus indicating that
/// a separate paint function must be called to paint the borders of the caption.
//
TEventStatus
TTinyCaption::DoNCPaint()
{
  if (!TCEnabled || IsIconic())
    return esPartial;  // We don't do anything special for an Icon

  // If we have focus or our children have focus, then we're active
  // Note: We can't rely only on GetFocus here because when we're being
  //       restored from an icon, we're active, but don't yet have focus!
  //
  THandle focus = GetFocus();
  PaintCaption(GetActiveWindow() == GetHandle() || focus == GetHandle() || IsChild(focus));
  return esPartial;       // Caller must call function to paint borders
}

//
/// Calculates the size of the command window including the caption and border so
/// that it can fit within the window.
//
uint
TTinyCaption::EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize)
{
  uint er = TWindow::EvNCCalcSize(calcValidRects, calcSize);
  DoNCCalcSize(calcValidRects, calcSize, er);
  return er;
}

//
/// Return the size of our client area, leaving room for caption bar
//
/// If the caption bar is not enabled or is iconic, returns esPartial. Otherwise,
/// calculates the dimensions of the caption and returns esComplete.
//
TEventStatus
TTinyCaption::DoNCCalcSize(bool /*calcValidRects*/,
                           NCCALCSIZE_PARAMS & calcSize, uint& /*evRes*/)
{
  if (!TCEnabled || IsIconic())
    return esPartial;

  calcSize.rgrc[0].top += GetCaptionRect().Height();

  return esComplete;
}

//
/// Responds to a press of the left mouse button while the cursor is within the
/// nonclient area of the caption bar by calling DoNCLButtonDown. If DoNCLButtonDown
/// does not return esComplete, EvNCLButtonDown calls TWindow::EvNCLButtonDown.
//
void
TTinyCaption::EvNCLButtonDown(uint hitTest, const TPoint& screenPt)
{
  // Display system menu, invert min/max icons (not), etc
  //
  if (DoNCLButtonDown(hitTest, screenPt) == esPartial)
    TWindow::EvNCLButtonDown(hitTest, screenPt);
}

//
/// If the caption bar is not enabled, returns esPartial. Otherwise, determines if
/// the user released the button outside or inside a menu, and returns esComplete.
//
TEventStatus
TTinyCaption::DoNCLButtonDown(uint hitTest, const TPoint& screenPt)
{
  if (!TCEnabled)
    return esPartial;

  TWindowDC wdc(*this);
  switch (hitTest) {
    case HTSYSMENU:
      DownHit = HTSYSMENU;
      if (CloseBox) {
        IsPressed = true;
        SetCapture();
        TRect rect(GetSysBoxRect());
        PaintCloseBox(wdc, rect, IsPressed);
      }
      else {
        TRect sysBoxRect = GetSysBoxRect().InflatedBy(-1,-1);
        sysBoxRect.right += 1;

        wdc.PatBlt(sysBoxRect, PATINVERT);

        // Display sys menu on button down
        // Need to lock sys menu until user clicks outside

        // Set flag to indicate we're expecting a sys command, & then send
        // message to popup sys menu
        //
        WaitingForSysCmd = true;
        SendMessage(WM_SYSCOMMAND, SC_MOUSEMENU|HTSYSMENU,
                    MkParam2(screenPt.x,screenPt.y));

        // If we didn't execute a command, user released btn outside of menu
        // If it was released in sys menu box, then redisplay menu as if it
        // were brought up with a keystroke
        //
        if (WaitingForSysCmd) {
          uint hitTest;
          TPoint pt;
          GetCursorPos(pt);
          DoNCHitTest(pt, hitTest);
          if (hitTest == HTSYSMENU) 
            SendMessage(WM_SYSCOMMAND, SC_KEYMENU|HTSYSMENU);
        }
        if (GetHandle())
          wdc.PatBlt(sysBoxRect, PATINVERT);
      }
      return esComplete;

    case HTMINBUTTON: {
        DownHit = HTMINBUTTON;
        IsPressed = true;
        SetCapture();
        TRect rect(GetMinBoxRect());
        PaintMinBox(wdc, rect, IsPressed);
        return esComplete;
      }

    case HTMAXBUTTON:{
        DownHit = HTMAXBUTTON;
        IsPressed = true;
        SetCapture();
        TRect rect(GetMaxBoxRect());
        PaintMaxBox(wdc, rect, IsPressed);
        return esComplete;
      }
  }
  DownHit = HTNOWHERE;
  return esPartial;
}

//
/// Responds to a mouse-move message by calling DoMouseMove. If DoMouseMove does not
/// return IsComplete, EvMouseMove calls TWindow::EvMouseMove.
//
void
TTinyCaption::EvMouseMove(uint modKeys, const TPoint& pt)
{
  if (DoMouseMove(modKeys, pt) == esPartial)
    TWindow::EvMouseMove(modKeys, pt);
}

//
/// Returns TEventStatus.
//
TEventStatus
TTinyCaption::DoMouseMove(uint /*modKeys*/, const TPoint& pt)
{
  if (TCEnabled && DownHit != HTNOWHERE) {
    uint   hitTest;
    TPoint screenPt = pt;
    ClientToScreen(screenPt);    // Cvt to screen coord
    DoNCHitTest(screenPt, hitTest);
    bool isNowPressed = hitTest == DownHit;

    if (isNowPressed != IsPressed) {
      IsPressed = isNowPressed;
      TWindowDC wdc(*this);
      switch (DownHit) {
        case HTSYSMENU:
          if (CloseBox){
            TRect rect(GetSysBoxRect());
            PaintCloseBox(wdc, rect, IsPressed);
          }
          return esComplete;
        case HTMINBUTTON:{
            TRect rect(GetMinBoxRect());
            PaintMinBox(wdc, rect, IsPressed);
          }
          return esComplete;
        case HTMAXBUTTON:{
            TRect rect(GetMaxBoxRect());
            PaintMaxBox(wdc, rect, IsPressed);
          }
          return esComplete;
      }
    }
  }
  return esPartial;
}

//
/// Responds to a mouse button-up message by calling DoLButtonUp. If DoLButtonUp
/// does not return IsComplete, EvLButtonUp calls TWindow::EvLButtonUp.
//
void
TTinyCaption::EvLButtonUp(uint modKeys, const TPoint& pt)
{
  // If we're still in area where buton went down, then do it
  //
  if (DoLButtonUp(modKeys, pt) == esPartial)
    TWindow::EvLButtonUp(modKeys, pt);
}

//
/// Releases the mouse capture if the caption bar is enabled and a mouse button is
/// pressed. Sets hitTest, indicating the mouse button has been pressed. Captures
/// the mouse message and repaints the smaller buttons before returning esComplete.
//
TEventStatus
TTinyCaption::DoLButtonUp(uint modKeys, const TPoint& pt)
{
  if (TCEnabled && DownHit != HTNOWHERE) {
    ReleaseCapture();
    DoMouseMove(modKeys, pt);

    uint   hitTest;
    TPoint screenPt = pt;
    ClientToScreen(screenPt);    // Cvt to screen coord
    DoNCHitTest(screenPt, hitTest);

    if (hitTest == DownHit) {
      DownHit = HTNOWHERE;
      switch (hitTest) {
        case HTSYSMENU:
          if (CloseBox)
            PostMessage(WM_CLOSE);
          return esComplete;

        // We have to handle these buttons also to prevent defproc from
        // painting the standard big min/max buttons when left mouse button is
        // pressed
        //
        case HTMINBUTTON:
          HandleMessage(WM_SYSCOMMAND, SC_MINIMIZE);
          return esComplete;

        case HTMAXBUTTON:
          HandleMessage(WM_SYSCOMMAND, IsZoomed() ? SC_RESTORE : SC_MAXIMIZE);
          return esComplete;
      }
    }
    DownHit = HTNOWHERE;
  }
  return esPartial;
}

//
/// Responds to a request to change a title bar or icon by calling DoNCActivate. If
/// DoNCActivate does not return esComplete, EvNCActivate calls
/// TWindow::EvNCActivate.
//
bool
TTinyCaption::EvNCActivate(bool active)
{
  bool er;
  if (DoNCActivate(active, er) == esPartial)
    er = TWindow::EvNCActivate(active);
  return er;
}

//
/// If the tiny caption is not enabled or is iconic, returns esPartial. Otherwise,
/// repaints the caption as an active caption and returns esComplete.
//
TEventStatus
TTinyCaption::DoNCActivate(bool active, bool& evRes)
{
  if (!TCEnabled || IsIconic())
    return esPartial;  // Let default do it's thing

  PaintCaption(active);
  evRes = true;
  return esComplete;
}

//
/// EvCommand provides extra processing for commands, but lets the focus window and
/// its parent windows handle the command first.
//
TResult
TTinyCaption::EvCommand(uint id, THandle hCtl, uint notifyCode)
{
  TResult er;
  if (DoCommand(id, hCtl, notifyCode, er) == esComplete)
    return er;
  return TWindow::EvCommand(id, hCtl, notifyCode);
}

//
/// Displays the system menu using ::TrackPopup so that TTinyCaption sends
/// WM_COMMAND instead of WM_SYSCOMMAND messages. If a system menu command is
/// received, it's then transformed into a WM_SYSCOMMAND message. If the tiny
/// caption bar is false, DoCommand returns esPartial.
//
TEventStatus
TTinyCaption::DoCommand(uint id, THandle /*hCtl*/, uint notifyCode, TResult& evRes)
{
  // We're displaying system menu using TrackPopup...
  // This will send us WM_COMMAND messages instead of WM_SYSCOMMAND msgs
  // If we get a system menu command then transform it into a WM_SYSCOMMAND
  //
  if (!TCEnabled)
    return esPartial;

  if (id >= 0xF000) {
    WaitingForSysCmd = false;  // Let LButtonDown handler know that a command was sent
    evRes = HandleMessage(WM_SYSCOMMAND, id, notifyCode);
    return esComplete;
  }
  else {
    evRes = 0;
    return esPartial;
  }
}

//
/// Responds to a WM_SYSCOMMAND message by calling DoSysCommand. If DoSysCommand
/// returns esPartial, EvSysCommand calls TWindow::EvSysCommand.
//
void
TTinyCaption::EvSysCommand(uint cmdType, const TPoint& p)
{
  if (DoSysCommand(cmdType,p) == esPartial)
    TWindow::EvSysCommand(cmdType, p);
}

//
/// Handle WM_SYSCOMMAND to make sure that SC_KEYMENU and SC_MOUSEMENU bring up
/// our sys menu at the right coord w/ respect to the tiny sys box.
//
/// If iconic, then let default windows processing deal with the menu
//
/// If the caption bar is not enabled, returns esPartial. If the caption bar is
/// iconized and the user clicks the icon, calls DoSysMenu to display the menu in
/// its normal mode and returns esComplete.
//
TEventStatus
TTinyCaption::DoSysCommand(uint cmdType, const TPoint&)
{
  if (!TCEnabled)
    return esPartial;

  cmdType &= 0xFFF0;
  if ((cmdType == SC_KEYMENU || cmdType == SC_MOUSEMENU) && !IsIconic()) {
    DoSysMenu();
    return esComplete;
  }
  return esPartial;
}

//
/// Paints a blank button.
//
void
TTinyCaption::PaintButton(TDC& dc, TRect& r, bool pressed)
{
   TBrush winFrameBr(TColor::SysWindowFrame);
//  dc.OWLFastWindowFrame(winFrameBr, r, 1, 1);
  dc.FrameRect(r, winFrameBr);

  r.Inflate(-1,-1);
  dc.TextRect(r, TColor::Sys3dFace);
  if (r.Width() > 4 && r.Height() > 4) {
    if (pressed) {
      dc.TextRect(r.left, r.top, r.right, r.top+1, TColor::Sys3dShadow);
      dc.TextRect(r.left, r.top+1, r.left+1, r.bottom, TColor::Sys3dShadow);
    }
    else {
      dc.TextRect(r.left, r.top, r.right-1, r.top+1, TColor::Sys3dHilight);
      dc.TextRect(r.left, r.top+1, r.left+1, r.bottom-1, TColor::Sys3dHilight);
      dc.TextRect(r.right-1, r.top+1, r.right, r.bottom, TColor::Sys3dShadow);
      dc.TextRect(r.left+1, r.bottom-1, r.right-1, r.bottom, TColor::Sys3dShadow);
    }
  }
}

//
/// Paints a close box on the tiny caption bar. You can override the default box if
/// you want to design your own close box.
//
void
TTinyCaption::PaintCloseBox(TDC& dc, TRect& boxRect, bool pressed)
{
  // Fill the box with light gray & draw bevel if possible
  //
  PaintButton(dc, boxRect, pressed);

  if (pressed)
    boxRect.Offset(1,1);

  // Do something different to differentiate from standard system menu--
  // draw a recessed black box glyph about half the button size, centered
  //
  int glyphWidth  = boxRect.Width() > 7 ?
                      boxRect.Width()-boxRect.Width()/2-1 : boxRect.Width()-3;
  int glyphHeight = boxRect.Height() > 7 ?
                      boxRect.Height()-boxRect.Height()/2-1 : boxRect.Height()-3;
  if (glyphWidth > 1 && glyphHeight > 1) {
    TRect glyphRect(0, 0, glyphWidth, glyphHeight);
    glyphRect.Offset(boxRect.left + (boxRect.Width()-glyphWidth-1)/2,
                     boxRect.top + (boxRect.Height()-glyphHeight-1)/2);

    dc.TextRect(glyphRect, TColor::Sys3dShadow);
    glyphRect.Offset(1,1);
    dc.TextRect(glyphRect, TColor::Sys3dHilight);
    glyphRect.BottomRight().Offset(-1,-1);
    dc.TextRect(glyphRect, TColor::SysBtnText);
  }
}

//
/// Paints the system box.
//
void
TTinyCaption::PaintSysBox(TDC& dc, TRect& boxRect, bool /*pressed*/)
{
  // Dont paint over the left & top borders
  //
  boxRect.left++;
  boxRect.top++;

  // Fill the box with 3d face
  //
  dc.TextRect(boxRect, TColor::Sys3dFace);

  // Draw the ventilator (sysmenu) box, with shadow
  //
  TPoint begPt = boxRect.TopLeft().OffsetBy(2, (boxRect.Height()-3)/2);
  TRect ventRect(begPt, TSize(boxRect.Width()-5, 3));

  // Draw shadow down and right 1
  //
  dc.TextRect(ventRect.left+1, ventRect.top+1,
              ventRect.right+1, ventRect.bottom+1, TColor::Sys3dShadow);

  // Draw ventilator rectangle
  //
  TBrush btnTextBr(TColor::SysBtnText);
  dc.FrameRect(ventRect, btnTextBr);

  // Draw white interior of ventilator
  //
  dc.TextRect(ventRect.left+1, ventRect.top+1,
              ventRect.right-1, ventRect.top+2, TColor::Sys3dHilight);

  dc.TextRect(boxRect.right, boxRect.top,
              boxRect.right+1, boxRect.bottom, TColor::SysBtnText);
}

//
/// Paints a minimize box on the tiny caption bar.
//
void
TTinyCaption::PaintMinBox(TDC& dc, TRect& boxRect, bool pressed)
{
  // Fill the box with light gray & draw bevel if possible
  //
  PaintButton(dc, boxRect, pressed);

  if (pressed)
    boxRect.Offset(1,1);

  int bh = boxRect.Height();
  int bw = boxRect.Width();

  TPoint begPt = boxRect.TopLeft().OffsetBy((bw+1)/4, (bh+2)/3);
  TPoint endPt = begPt.OffsetBy((bw+1)/2,0);
  while (begPt.x < endPt.x) {
    dc.MoveTo(begPt);
    dc.LineTo(endPt);
    begPt.Offset(1,1);
    endPt.Offset(-1,1);
  }
}

//
/// Paints a maximize box on the tiny caption bar.
//
void
TTinyCaption::PaintMaxBox(TDC& dc, TRect& boxRect, bool pressed)
{
  // Fill the box with light gray & draw bevel if possible
  //
  PaintButton(dc, boxRect, pressed);

  if (pressed)
    boxRect.Offset(1,1);

  // Down triangle
  //
  int bh = boxRect.Height();
  int bw = boxRect.Width();

  if (IsZoomed()) {
    TPoint begPt = boxRect.BottomLeft().OffsetBy((bw+1)/4, -bh*3/8);
    TPoint endPt = begPt.OffsetBy((bw+1)/2, 0);
    while (begPt.x < endPt.x) {
      dc.MoveTo(begPt);
      dc.LineTo(endPt);
      begPt.Offset(1,1);
      endPt.Offset(-1,1);
    }
  }

  // Up triangle
  //
  {
    TPoint begPt = boxRect.TopLeft().OffsetBy((bw+1)/4, IsZoomed() ? bh*3/8 : bh*2/3);
    TPoint endPt = begPt.OffsetBy((bw+1)/2, 0);
    while (begPt.x < endPt.x) {
      dc.MoveTo(begPt);
      dc.LineTo(endPt);
      begPt.Offset(1, -1);
      endPt.Offset(-1, -1);
    }
  }
}

//
/// Calls dc.SelectObject() to select the given rectangle and dc.PatBlt() to paint the
/// tiny caption bar using the currently selected brush for this device context.
//
void
TTinyCaption::PaintCaption(bool active)
{
  // Paint caption background and caption text if any.
  //
  TWindowDC dc(*this);
  TRect captRect = GetCaptionRect();

  dc.SetTextColor(active ? TColor::SysCaptionText :
                           TColor::SysInactiveCaptionText);

  // Could use a TBrush and PatBlt instead, but text backgrounds are always
  // solid so this works well.
  //
  dc.TextRect(captRect, active ? TColor::SysActiveCaption :
                                 TColor::SysInactiveCaption);

  CHECK(CaptionFont);
  dc.SelectObject(*CaptionFont);
  dc.SetBkMode(TRANSPARENT);
  LPCTSTR c = GetCaption() ? GetCaption() : _T("");
  TSize textSize = dc.GetTextExtent(c, static_cast<int>(::_tcslen(c)));

  // Calc x coord for text, so that text is centered between caption buttons
  //
  int  xOrg = captRect.right - captRect.left;
  long style = GetWindowLong(GWL_STYLE);

  if (style & WS_MINIMIZEBOX)
    xOrg -= GetMinBoxRect().Width();

  if (style & WS_MAXIMIZEBOX)
    xOrg -= GetMaxBoxRect().Width();

  if ((style & WS_SYSMENU) || CloseBox)
    xOrg -= GetSysBoxRect().Width();

  xOrg -= textSize.cx;
  if (xOrg<0)
    xOrg = 0;
  else
    xOrg = xOrg/2;

  xOrg += captRect.left;

  if ((style & WS_SYSMENU) || CloseBox)
    xOrg += GetSysBoxRect().Width();

  dc.ExtTextOut(xOrg, captRect.top-Border.cy,
    ETO_CLIPPED,
    &captRect,
    c,
    static_cast<int>(::_tcslen(c))
  );
  dc.RestoreFont();

  // Paint widgets: sysmenu or close button, minimize button, maximize button
  // They currently all use a black pen
  //
  dc.SelectStockObject(BLACK_PEN);

  // Paint system menu or close button
  //
  if (CloseBox){
    TRect rect(GetSysBoxRect());
    PaintCloseBox(dc, rect, false);
  }
  else if (style & WS_SYSMENU){
    TRect rect(GetSysBoxRect());
    PaintSysBox(dc, rect, false);
  }

  // Paint minimize button
  //
  if (style & WS_MINIMIZEBOX){
    TRect rect(GetMinBoxRect());
    PaintMinBox(dc, rect, false);
  }

  // Paint maximize button
  //
  if (style & WS_MAXIMIZEBOX){
    TRect rect(GetMaxBoxRect());
    PaintMaxBox(dc, rect, false);
  }

  // Draw window-frame color line under caption
  //
  TBrush winFrameBr(TColor::SysWindowFrame);
  dc.FrameRect(captRect.left, captRect.bottom-1, captRect.right, captRect.bottom,
               winFrameBr);
}

//
/// Gets the area of the caption for changing or repainting.
/// \note GetCaptionRect and GetSysBoxRect must be kept in sync!
//
TRect
TTinyCaption::GetCaptionRect()
{
  // Get caption rect converted to window relative coordinates
  //
  TRect captRect(GetWindowRect());
  captRect -= captRect.TopLeft();

  captRect.left +=  Frame.cx;
  captRect.top +=   Frame.cy;
  captRect.right -= Frame.cx;
  captRect.bottom = captRect.top + CaptionHeight;

  return captRect;
}

//
/// Returns the size of the system box rectangle.
//
TRect
TTinyCaption::GetSysBoxRect()
{
  TRect boxRect(GetCaptionRect());
  boxRect.right = boxRect.left + CaptionHeight;
  boxRect.left -= 1;
  boxRect.top -= 1;
  return boxRect;
}

//
/// Returns the size of the minimize box rectangle.
//
TRect
TTinyCaption::GetMinBoxRect()
{
  // Far right on caption if no max box, else next to max box
  //
  TRect boxRect(GetMaxBoxRect());
  if (GetWindowLong(GWL_STYLE) & WS_MAXIMIZEBOX)
    boxRect.Offset(-CaptionHeight, 0);
  return boxRect;
}

//
/// Returns the size of the maximize box rectangle.
//
TRect
TTinyCaption::GetMaxBoxRect()
{
  TRect boxRect(GetCaptionRect());
  boxRect.left = boxRect.right - CaptionHeight;
  boxRect.top -= 1;
  boxRect.right += 1;
  return boxRect;
}

//
/// Gets the system menu and sets up menu items. DoSysMenu is also responsible for
/// displaying and tracking the status of the menu.
//
void
TTinyCaption::DoSysMenu()
{
  HMENU hSysMenu = GetSystemMenu();
  if (hSysMenu) {
    uint32 style = GetWindowLong(GWL_STYLE);
    EnableMenuItem(hSysMenu, SC_RESTORE, (IsIconic() || IsZoomed()) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hSysMenu, SC_MOVE, (1/*style & WS_CAPTION*/) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hSysMenu, SC_SIZE, (style & WS_THICKFRAME) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hSysMenu, SC_MINIMIZE, ((style&WS_MINIMIZEBOX) && !IsIconic()) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hSysMenu, SC_MAXIMIZE, ((style&WS_MAXIMIZEBOX) && !IsZoomed()) ? MF_ENABLED : MF_GRAYED);
    TRect r(GetSysBoxRect());
    ClientToScreen(r.TopLeft());     // Cvt pt to screen coord
    ClientToScreen(r.BottomRight());
    TrackPopupMenu(hSysMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,
                   r.left-Frame.cx, r.top-Frame.cy, 0, GetHandle(), &r);
  }
}


#if !defined(BI_NO_OBJ_STREAMING)

IMPLEMENT_STREAMABLE(TTinyCaption);

//
// Reads an instance of TTinyCaption from the given ipstream
//
void*
TTinyCaption::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TTinyCaption* o = GetObject();

  o->CaptionFont = 0;
  is >> o->TCEnabled;
  if (o->TCEnabled) {
    int captionHeight;
    is >> captionHeight >> o->CloseBox;;
    o->EnableTinyCaption(captionHeight, o->CloseBox);
  }
  return o;
}

//
// Writes the TTinyCaption to the given opstream
//
void
TTinyCaption::Streamer::Write(opstream& os) const
{
  TTinyCaption* o = GetObject();

  os << o->TCEnabled;
  if (o->TCEnabled) {
    int captionHeight = (100*o->CaptionHeight) /
                          (o->Border.cy+TUIMetric::CyCaption);
    os << captionHeight << o->CloseBox;
  }
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */



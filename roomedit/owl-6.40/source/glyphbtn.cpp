//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TGlyphButton
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/glyphbtn.h>
#include <owl/uihelper.h>
#include <owl/celarray.h>
#include <owl/gdiobjec.h>
#include <owl/color.h>
#include <owl/system.h>
#include <owl/theme.h>
#include <owl/commctrl.h>
#include <owl/glyphbtn.rh>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

const int  LayoutMargin         = 4;
const int  FaceToFocusRectDelta = 1;
const int  BUTTONSTATE_PUSHED   = 0x0004;
const int  BUTTONSTATE_FOCUS    = 0x0008;
const int  BUTTONSTATE_HOT      = 0x0010;
const uint32 BUTTONSTYLE_MASK = 0x000000FFL;
const long RopDSPDxax           = 0x00E20746L;

//
// Helper function - extracts the button style bits.
//
static inline uint32 
ButtonStyle(uint32 style) 
{
  return style & BUTTONSTYLE_MASK;
}



OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);        // General Controls diagnostic group


// Hard-coded color constant that a bitmap designer can use as background.
// This color RGB(192, 192, 192) can then be mapped to the BTNFACE color
// at runtime.
//
const TColor TBtnBitmap::DefaultFaceColor = TColor::LtGray;


//
/// Constructor of BtnBitmap - loads the specified bitmap and updates the face color
/// if necessary.
//
TBtnBitmap::TBtnBitmap(HINSTANCE hInstance, TResId resId,
                       const TColor& faceColor)
:
  TBitmap(hInstance, resId),
  FaceColor(faceColor)
{
  UpdateFaceColor();
}


//
/// Constructor of BtnBitmap - aliases the specified bitmap handle and updates the
/// face color if necessary.
//
TBtnBitmap::TBtnBitmap(HBITMAP hBitmap, const TColor& faceColor,
                       TAutoDelete autoDelete)
:
  TBitmap(hBitmap, autoDelete),
  FaceColor(faceColor)
{
  UpdateFaceColor();
}


//
/// Updates the face color of the associated bitmap if the current face color does
/// not match the 3DFACE system color.
//
void
TBtnBitmap::UpdateFaceColor()
{
  if (FaceColor != TColor::Sys3dFace) {
    TBitmap copy(*this);
    TMemoryDC dstDC, srcDC;
    dstDC.SelectObject(*this);
    srcDC.SelectObject(copy);
    MapColor(dstDC, srcDC, TColor::Sys3dFace, FaceColor, Width(), Height());
    dstDC.RestoreBitmap();
    srcDC.RestoreBitmap();
    FaceColor = TColor::Sys3dFace;
  }
}


//
/// Helper routine used to map the face color of the underlying bitmap.
//
void
TBtnBitmap::MapColor(TDC& dc, TDC& srcDC, const TColor& toColor,
                     const TColor& fromColor, int width, int height)
{
  // Create a monochrome mask
  //
  TMemoryDC hMemDC(srcDC);
  TBitmap hMask(width, height);
  hMemDC.SelectObject(hMask);

  // Build a mask where the source color is zeroed
  //
  TColor crBack = srcDC.SetBkColor(fromColor);
  hMemDC.BitBlt(0, 0, width, height, srcDC, 0, 0, SRCCOPY);
  srcDC.SetBkColor(crBack);

  // Make a brush out of the destination color
  //
  TBrush brush(toColor);
  dc.SelectObject(brush);

  // Copy the mask to the destination
  //
  dc.BitBlt(0, 0, width, height, srcDC, 0, 0, SRCCOPY);

  // Copy the source bitmap to the destination by applying the
  // brush where pixels are zeroed
  //
  dc.BitBlt(0, 0, width, height, hMemDC, 0, 0, RopDSPDxax);
  dc.RestoreBrush();
}


namespace
{

  //
  // Returns `true` if themes are enabled for this application and
  // themed Common Controls (version 6 or later) are in use.
  //
  // Important: This function must be called after the creation of the main 
  // window, otherwise it may always return `false`.
  //
  // Note that IsAppThemed will return `false` if either (a) themes have been
  // disabled for the application by selecting "Disable visual themes" in the
  // Compatibility tab in the Properties dialog for the executable, or (b)
  // themes have been deactivated by selecting the Windows Classic style in 
  // the Windows XP/7/Vista Control Panel (not available in Windows 8).
  // Note that (b) may change at run-time.
  //
  // Note we do not have to use IsThemeActive here. This function only reports
  // the state of the Control Panel setting (Classic vs themed).
  //
  bool IsThemed()
  {
    return TThemeModule::GetInstance().IsAppThemed() &&
      TCommCtrl::Dll()->GetCtrlVersion() >= 0x60000;
  }

} // namespace

//
/// Constructor of TGlyphButton - Use this constructor to create a GlyphBtn from
/// scratch.
//
TGlyphButton::TGlyphButton(TWindow* parent, int id, LPCTSTR text,
                           int X, int Y, int W, int H, TBtnType type,
                           bool isDefault, TModule* module)
:
  TButton(parent, id, text, X, Y, W, H, isDefault, module)
{
  InitVars();
  BtnType = type;
  if(type != btCustom)
    InitGlyp(type);
}

//
/// String-aware overload
//
TGlyphButton::TGlyphButton(
  TWindow* parent, 
  int id, 
  const tstring& text,
  int X, int Y, int W, int H, 
  TBtnType type,
  bool isDefault, 
  TModule* module
  )
  : TButton(parent, id, text, X, Y, W, H, isDefault, module)
{
  InitVars();
  BtnType = type;
  if(type != btCustom)
    InitGlyp(type);
}


//
/// Constructor of TGlyphButton - Use this constructor to alias a glyph button
/// control specified in a dialog template.
/// For more information how to use glyph buttons from a resource, see \ref abouttheglyphbuttoncontrol 
//
TGlyphButton::TGlyphButton(TWindow* parent, int resourceId,
                           TBtnType type, TModule* module)
:
  TButton(parent, resourceId, module)
{
  InitVars();
  BtnType = type;
  if(type != btCustom)
    InitGlyp(type);
}


//
/// Method used to initialized variables used by GlyphButton's implementation.
//
void
TGlyphButton::InitVars()
{
  CelArray = 0;
  BtnFont = 0;
  xText = yText = -1;
  xGlyph = yGlyph = -1;
  LayStyle = lsH_GST;
  BtnType = btCustom;
  Set(biShowText);
}

//
/// Destructor - Cleans up resources used by Glyph Button object.
//
TGlyphButton::~TGlyphButton()
{
  delete CelArray;
  delete BtnFont;
}


//
//init Glip
//
void TGlyphButton::InitGlyp(TBtnType type)
{
  uint ResIdUp = 0;
  switch(type){
    case btOk:
    case btYes:
    case btAbort:
      ResIdUp = IDB_BORBTNII_OK;
      break;
    case btCancel:
    case btClose:
    case btIgnore:
      ResIdUp = IDB_BORBTNII_CANCEL;
      break;
    case btNo:
      ResIdUp = IDB_BORBTNII_NO;
      break;
    case btHelp:
      ResIdUp = IDB_BORBTNII_HELP;
      break;
    case btApply:
      ResIdUp = IDB_BORBTNII_APPLY;
      break;
    case btBrowse:
      ResIdUp = IDB_BORBTNII_BROWSE;
      break;
    case btRevert:
      ResIdUp = IDB_BORBTNII_REVERT;
      break;
    case btAdd :
      ResIdUp = IDB_BORBTNII_ADD;
      break;
    case btDelete:
      ResIdUp = IDB_BORBTNII_DEL;
      break;
    case btEdit:
      ResIdUp = IDB_BORBTNII_EDIT;
      break;
    case btKey:
      ResIdUp = IDB_BORBTNII_KEY;
      break;
    case btSetup:
      ResIdUp = IDB_BORBTNII_SETUP;
      break;
    case btCustom:
    default:                 //JJH added empty default construct
      break;
  }
  if(ResIdUp){
    delete CelArray;
    CelArray = new TCelArray(new TBtnBitmap(GetGlobalModule(), ResIdUp), 3); //CelsTotal);
    Set(biGtUp|biGtDown|biGtDisabled|biShowGlyph);
  }
  return;
}


//
// Response Table
//
DEFINE_RESPONSE_TABLE1(TGlyphButton, TButton)
  EV_WM_PAINT,
  EV_WM_ERASEBKGND,
  EV_WM_SETFOCUS,
  EV_WM_KILLFOCUS,
  EV_WM_GETFONT,
  EV_WM_SETFONT,
  EV_WM_GETDLGCODE,
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONDBLCLK,
  EV_WM_LBUTTONUP,
  EV_WM_MOUSEMOVE,
  EV_WM_KEYDOWN,
  EV_WM_KEYUP,
  EV_WM_ENABLE,
  EV_WM_CANCELMODE,
  EV_WM_SYSCOLORCHANGE,
  EV_MESSAGE(BM_SETSTATE, BmSetState),
  EV_MESSAGE(BM_GETSTATE, BmGetState),
  EV_MESSAGE(BM_SETSTYLE, BmSetStyle),
  EV_WM_MOUSELEAVE,
END_RESPONSE_TABLE;

//
// Window proc. of control to handle messages sent before OWL thunks
//
LRESULT CALLBACK
BButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
    case WM_GETDLGCODE: {
      uint32 btnStyle = ButtonStyle(GetWindowLong(hwnd, GWL_STYLE));
      if (btnStyle == BS_DEFPUSHBUTTON)
        return DLGC_BUTTON|DLGC_DEFPUSHBUTTON;
      else
        return DLGC_BUTTON|DLGC_UNDEFPUSHBUTTON;
    }
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

//
/// Overriden virtual of TWindow - Fills out information about the Window class
/// associated with a glyph button.
/// \note The class information is based on the system's "BUTTON" class.
//
void
TGlyphButton::GetWindowClass(WNDCLASS& wndClass)
{
  // Grab a the attributes of the native "BUTTON" control
  //
  if (::GetClassInfo(0, _T("BUTTON"), &wndClass)) {
    wndClass.hInstance = *GetModule();
    wndClass.lpszClassName = GetClassName();
    wndClass.lpfnWndProc = BButtonProc;
  }
  else {
    TControl::GetWindowClass(wndClass);
    wndClass.style = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC;
    wndClass.hbrBackground = HBRUSH(COLOR_BTNFACE+1);
  }
}

//
/// Returns name of window class associated with a glyph button control.
//
TWindow::TGetClassNameReturnType
TGlyphButton::GetClassName()
{
  return OWL_GLYPHBTN;
}

//
/// Overriden virtual of TWindow - Updates internal flags based on style of
/// underlying window.
//
void
TGlyphButton::SetupWindow()
{
  TButton::SetupWindow();

  // Update state flags based on current style
  //
  if (ButtonStyle(GetStyle()) == BS_DEFPUSHBUTTON)
    Set(biDefault);
  if (GetStyle() & WS_DISABLED)
    Set(biDisabled);

  // Get Parent font -> dialog font
  //
  delete BtnFont;
  HFONT hFont = GetParentO()->GetWindowFont();
  BtnFont = hFont ? new TFont(hFont) : new TDefaultGUIFont;
}


//
/// Specifies a bitmap to be used as glyph.
//
void
TGlyphButton::SetGlyph(HBITMAP hBitmap, TGlyphType type, TAutoDelete autoDelete)
{
  PRECONDITION(!hBitmap || ::GetObjectType(hBitmap) == OBJ_BITMAP);

  SetGlyph(hBitmap ? new TBtnBitmap(hBitmap, TBtnBitmap::DefaultFaceColor, autoDelete) : (TBitmap*)0, type);
}


//
/// Specifies the resource identifier of a bitmap to be used as glyph.
//
void
TGlyphButton::SetGlyph(TResId resId, TModule* module, TGlyphType type)
{
  PRECONDITION(module != 0 || GetModule());
  SetGlyph(new TBtnBitmap(module ? *module : *GetModule(), resId), type,
            AutoDelete);
}


//
// Create CellArray with 3 elements
//
void
TGlyphButton::BuildCelArray(const TBitmap* cell)
{
  CelArray = new TCelArray(cell->Size(), 0, 3,2);
  CelArray->Add(*cell);
  CelArray->Add(*cell);
  CelArray->Add(*cell);
}


//
/// Specifies a bitmap object to be used as glyph.
/// \note The 'bitmap' parameter can be 0 to reset the glyph stored by the glyph
/// button object.
//
void
TGlyphButton::SetGlyph(TBitmap* bitmap, TGlyphType type, TAutoDelete autoDelete)
{
  PRECONDITION(!bitmap || bitmap->IsGDIObject());
  if(!CelArray && bitmap && type != gtCellArray)
    BuildCelArray(bitmap);

  switch (type) {
    case gtUp:
      if(bitmap){
         CelArray->Replace(0, *bitmap);
        if(autoDelete==AutoDelete)
          delete bitmap;
        Set(biGtUp);
      }
      else
        Clear(biGtUp);
      break;

    case gtDown:
      if(bitmap){
         CelArray->Replace(1, *bitmap);
        if(autoDelete==AutoDelete)
          delete bitmap;
        Set(biGtDown);
      }
      else
        Clear(biGtDown);
      break;

    case gtDisabled:
      if(bitmap){
         CelArray->Replace(2, *bitmap);
        if(autoDelete==AutoDelete)
          delete bitmap;
        Set(biGtDisabled);
      }
      else
        Clear(biGtDisabled);
      break;

    case gtCellArray:
      if(bitmap){
        delete CelArray;
        CelArray = new TCelArray((autoDelete==AutoDelete?
                                  bitmap:new TBitmap(*bitmap)),3);
        Set(biGtUp|biGtDown|biGtDisabled);
      }
      else
        Clear(biGtUp|biGtDown|biGtDisabled);
      break;

    default:
      break;
  }

  // Update status flag
  //
  if (IsSet(biGtUp))
    Set(biShowGlyph);
  else
    Clear(biShowGlyph);
}


//
/// WM_PAINT handler - Invokes 'Paint' method to display glyph and/or text.
//
void
TGlyphButton::EvPaint()
{
  TPaintDC dc(*this);
  TRect& rect = *(TRect*)&dc.Ps.rcPaint;
  Paint(dc, dc.Ps.fErase, rect);
}

//
/// Invokes 'PaintButton' to display glyph and/or text.
//
void
TGlyphButton::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
  PaintButton(dc);
}


//
/// WM_ERASEBKGND handler - Returns true to prevent background from being erased.
/// (WM_PAINT handler paints whole client area).
//
bool
TGlyphButton::EvEraseBkgnd(HDC /*dc*/)
{
  return true;
}

//
/// WM_SETFOCUS handler - Updates internal flag and forces button to redraw.
//
void
TGlyphButton::EvSetFocus(THandle /*hWndLostFocus*/)
{
  Set(biFocus);
  Invalidate(true);
}

//
/// WM_KILLFOCUS handler - Updates internal flag and forces button to redraw.
//
void
TGlyphButton::EvKillFocus(THandle /*hWndGetFocus*/)
{
  Clear(biFocus);
  if (IsSet(biPushed))
    ClearCapture();
  else
    Invalidate(true);
}

//
/// WM_GETFONT handler - Returns font used by control if one was specified earlier.
/// Otherwise, returns 0.
//
HFONT
TGlyphButton::EvGetFont()
{
  PRECONDITION(!BtnFont || BtnFont->IsGDIObject());
  return BtnFont ? HFONT(*BtnFont) : HFONT(0);
}

//
/// WM_SETFONT Handler. Deletes any cached font and stores a copy of the new one.
//
void
TGlyphButton::EvSetFont(HFONT hFont, bool redraw)
{
  delete BtnFont;
  BtnFont = new TFont(hFont);
  if (redraw)
    Invalidate();
}

//
/// WM_GETDLGCODE handler. Informs dialog manager of a 'normal' push button or the
/// default push button according to the style.
//
uint
TGlyphButton::EvGetDlgCode(const MSG* /*msg*/)
{
  if (IsSet(biDefault))
    return DLGC_BUTTON|DLGC_DEFPUSHBUTTON;
  else
    return DLGC_BUTTON|DLGC_UNDEFPUSHBUTTON;
}

//
/// WM_LBUTTONDOWN handler. Grabs focus and updates button's state to be in 'pushed'
/// mode.
//
void
TGlyphButton::EvLButtonDown(uint /*modKeys*/, const TPoint& /*point*/)
{
  SetCapture();
  SendMessage(BM_SETSTATE, TRUE);
  if(!IsSet(biFocus))
    SetFocus();
}

//
/// WM_LBUTTONDBLCLK handler. Simply forwards to LBUTTONDOWN handler.
//
void
TGlyphButton::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  EvLButtonDown(modKeys, point);
}

//
/// WM_LBUTTONUP handler. Restores state of button and notifies parent with a
/// CLICKED message if necessary.
//
void
TGlyphButton::EvLButtonUp(uint /*modKeys*/, const TPoint& point)
{
  if (GetCapture() == *this) {
    ReleaseCapture();

    SendMessage(BM_SETSTATE, FALSE);

    TRect rect;
    GetClientRect(rect);

    if (rect.Contains(point)) {
      SendNotification(::GetParent(*this), GetDlgCtrlID(), BN_CLICKED, *this);
    }
  }
}

//
/// WM_MOUSEMOVE handler. Updates state of button if it is in 'capture' mode.
//
void
TGlyphButton::EvMouseMove(uint modKeys, const TPoint& point)
{
  TRect rect;
  GetClientRect(rect);
  if (modKeys & MK_LBUTTON && GetCapture() == *this) {

    if (rect.Contains(point))
      SendMessage(BM_SETSTATE, 1);
    else
      SendMessage(BM_SETSTATE, 0);
  }
  else if (IsThemed() && rect.Contains(point)) 
  {
    if (!IsSet(biHot))
    {
      SendMessage(BM_SETSTATE, 2); //HOT
      TrackMouseEvent(TME_LEAVE); // Order the WM_MOUSELEAVE message.
    }
  }
}

//
/// WM_KEYDOWN handler. Updates the state of the button upon detecting that the user
/// pressed the space bar.
//
void
TGlyphButton::EvKeyDown(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  if (key == VK_SPACE)
    SendMessage(BM_SETSTATE, TRUE);
}

//
/// WM_KEYUP handler. Restores state of button and notify parent.
//
void
TGlyphButton::EvKeyUp(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  if (IsSet(biPushed) && key == VK_SPACE) {
    SendMessage(BM_SETSTATE, FALSE);

    SendNotification(::GetParent(*this), GetDlgCtrlID(),
                      BN_CLICKED, *this);
  }
}

//
/// WM_ENABLE handler. Updates internal flags and invalidates control if  necessary.
//
void
TGlyphButton::EvEnable(bool enabled)
{
  if (enabled) {
    Clear(biDisabled);
  }
  else {
    ClearCapture();
    Set(biDisabled);
  }
  Invalidate(true);
}

//
/// WM_CANCELMODE handler. Releases capture if currently in capture mode and
/// terminates any internal operation on the button.
//
void
TGlyphButton::EvCancelMode()
{
  ClearCapture();
}

/// WM_SYSCOLORCHANGE handler – Reloads buttons.
void
TGlyphButton::EvSysColorChange()
{
  TButton::EvSysColorChange();
  if(BtnType != btCustom)
    InitGlyp(BtnType);
}

//
/// Cancels button Hot state.
//
void
TGlyphButton::EvMouseLeave()
{
  if (IsSet(biHot))
    SendMessage(BM_SETSTATE, 0);
}

//
/// BM_GETSTATE handler. Returns the current state of the window.
//
TResult
TGlyphButton::BmGetState(TParam1, TParam2)
{
  TResult result = 0;
  if (IsSet(biPushed))
    result |= BUTTONSTATE_PUSHED;

  if (IsSet(biFocus))
    result |= BUTTONSTATE_FOCUS;

  if (IsSet(biHot))
    result |= BUTTONSTATE_HOT;

  return result;
}

//
/// BM_SETSTATE handler. Updates internal state flags based on parameters and
/// redraws control if necessary.
//
TResult
TGlyphButton::BmSetState(TParam1 param1, TParam2)
{
  switch (param1)
  {
  case 1: // Needs hilight look
    if (!IsSet(biPushed))
    {
      Set(biPushed);
      PaintNow();
    }
    break;

  case 2: //New: Hot state
    if (!IsSet(biHot)) 
    {
      Set(biHot);
      PaintNow();
    }
    break;

  default: // Needs normal look
    if (IsSet(biPushed)||IsSet(biHot))
    {
      Clear(biPushed);
      Clear(biHot);
      PaintNow();
    }
    break;
  }
  return 0;
}

//
/// BM_SETSTYLE handler. Updates internal flags to match specified parameters and
/// invalidates the window if necessary.
//
TResult
TGlyphButton::BmSetStyle(TParam1 param1, TParam2)
{
  struct Local
  {
    static void SetButtonStyle(TGlyphButton* b, uint32 btnStyle)
    {
      using namespace owl; // Circumvents compiler issue.

      uint32 bs = btnStyle & BUTTONSTYLE_MASK;
      uint32 other = b->GetStyle() & ~BUTTONSTYLE_MASK;
      b->TWindow::SetStyle(bs | other);
    }
  };

  // Grab and splice the styles
  //
  uint32 btnStyle = ButtonStyle(GetStyle());
  uint32 newBtnStyle = ButtonStyle(static_cast<uint32>(param1));

  // Check against passed in parameter
  // NOTE: We only cater to PUSHBUTTON and DEFPUSHBUTTON
  //       The current definition so BS_PUSHBUTTON is 0L
  //
  if (newBtnStyle == BS_PUSHBUTTON && btnStyle != BS_PUSHBUTTON) 
  {
    // Make 'simple' push button
    //
    Local::SetButtonStyle(this, newBtnStyle);
    Clear(biDefault);
    Invalidate(true);
  }
  else if (newBtnStyle == BS_DEFPUSHBUTTON && btnStyle != BS_DEFPUSHBUTTON)
  {
    // Make 'default' push button
    //
    Local::SetButtonStyle(this, newBtnStyle);
    Set(biDefault);
    Invalidate(true);
  }

#if defined(__DEBUG) || defined(__TRACE) || defined(__WARN)
  if (newBtnStyle != BS_PUSHBUTTON && newBtnStyle != BS_DEFPUSHBUTTON)
    TRACEX(OwlControl, 0, "BmSetStyle: Invalid style specified");
#endif

  return 0;
}

//
/// Releases caption if we are in 'capture' mode. Resets internal flags
/// appropriately.
//
void
TGlyphButton::ClearCapture()
{
  if (GetCapture() == *this)
    ReleaseCapture();
  Clear(biPushed);
  Invalidate(true);
}

//
/// Paints the button into a memory DC and bitblt the final rendering to the
/// specified 'dc'.
//
void
TGlyphButton::PaintButton(TDC& dc)
{
  TRect rect;
  GetClientRect(rect);

  if (IsThemed())
  {
    // Draw themed background. This ignores all our border styles.
    // PBS_NORMAL = 1, PBS_HOT = 2, PBS_PRESSED = 3, PBS_DISABLED = 4, PBS_DEFAULTED = 5, PBS_DEFAULTED_ANIMATING = 6,
    int state = IsSet(biDisabled) ? PBS_DISABLED : 
      IsSet(biPushed) ? PBS_PRESSED : 
      IsSet(biHot) ? PBS_HOT : 
      IsSet(biFocus) ? PBS_DEFAULTED : 
      PBS_NORMAL;
    TThemePart part(GetHandle(), L"BUTTON", BP_PUSHBUTTON, state);
    part.DrawTransparentBackground(GetHandle(), dc, rect);
  }
  else 
  {
    // Themes are not available. Draw classic style.
    //
    PaintDefaultRect(dc, rect);
    PaintFrame(dc, rect);
  }
  PaintFace(dc, rect);
}

//
/// Draws the border of the button.
//
void
TGlyphButton::PaintFrame(TDC& dc, TRect& rect)
{
  // Paint the border
  //
  TUIBorder uiBorder(rect, IsSet(biPushed) ? TUIBorder::ButtonDn :
                                             TUIBorder::ButtonUp);
  uiBorder.Paint(dc);

  // Shrink the rectangle to leave the face
  //
  rect = uiBorder.GetClientRect();
}

//
/// Draws a frame around the button if it's a default push button.
//
void
TGlyphButton::PaintDefaultRect(TDC& dc, TRect& rect)
{
  if (IsSet(biDefault)){
    if (rect.Width() > 2 && rect.Height() > 2) {
      TPen framePen(TColor::SysWindowFrame);
      dc.SelectObject(framePen);
      dc.SelectStockObject(NULL_BRUSH);

#if 0
      rect.Inflate(-1, -1);
      dc.Rectangle(rect);
#else
      dc.Rectangle(rect);
      rect.Inflate(-1, -1);
#endif
      dc.RestoreBrush();
      dc.RestorePen();
    }
  }
}

//
/// Draws the face of the button [i.e. text and glyph portions].
//
void
TGlyphButton::PaintFace(TDC& dc, const TRect& rect)
{
  const bool isThemed = IsThemed();
  if (!isThemed)
  {
    // Fill the background with the face color
    //
    TBrush brush(TColor::Sys3dFace);
    dc.FillRect(rect, brush);
  }

  // Grab the glyph and it's size
  //
  int index = -1;              // index into array
  TRect glyphRect;             // Size of glyph. (default constructor now zerroed all members) !!!!!!!!!1
  if(IsSet(biShowGlyph)){

    // Start with the up bitmap
    index = 0;

    // Switch to more appropriate bitmap if applicable
    //
    if (IsSet(biPushed) && IsSet(biGtDown))
      index = 1;
    else if (IsSet(biDisabled) && IsSet(biGtDisabled))
      index = 2;

    CHECK(CelArray);
    TSize cell = CelArray->CelSize();
     glyphRect.Set(0, 0, cell.cx, cell.cy);
  }

  // Grab some information about the text/caption
  //
  int len = 0;                    // Length of Caption
  TRect textRect;                 // Size of text (default constructor now zerroed all members) !!!!!!
  TAPointer<tchar> text;          // Pointer to caption dynamic buffer
  TPointer<TFont>  tmpFnt;        // Object wrapping font handle
  TColor textColor;               // Color used for button's text

  if (IsSet(biShowText)) {
    len = GetWindowTextLength();
    if (len){

      // Select the font
      //
      if (!BtnFont){
        HFONT hFont = HFONT(::SendMessage(::GetParent(*this), WM_GETFONT,
                                          0, 0));
#if defined(BI_DBCS_SUPPORT)
        if (!hFont)
          tmpFnt = (TFont*)new TDefaultGUIFont();
#else
        if(!hFont)
          hFont = HFONT(GetStockObject(ANSI_VAR_FONT));
        if(hFont)
          tmpFnt = new TFont(hFont);
#endif
      }
      if (BtnFont){
        CHECK(BtnFont->IsGDIObject());
        dc.SelectObject(*BtnFont);
      }
      else if (tmpFnt) {
        CHECK(tmpFnt->IsGDIObject());
        dc.SelectObject(*tmpFnt);
      }

      text = new tchar[len+1];
      GetWindowText(text, len+1);
      textRect.Set(0, 0, rect.Width() - glyphRect.Width(), SHRT_MAX);

      // Display text in proper color
      //
      // Under NT 3.51 and Window 3.1, the system inexplicably returns values
      // for gray text that are the same as either button text (so text never
      // looks grayed) or as button face (so grayed text is invisible).  The
      // extra if statement guards against those problems.
      //
      if (IsSet(biDisabled)) {
#if 0
        textColor = TColor::SysGrayText;
        if ((textColor == TColor::Sys3dFace.GetValue()) ||
            (textColor == TColor::SysBtnText.GetValue())) {
          textColor = TColor::Sys3dShadow;
        }
#else
        textColor = TColor::Sys3dShadow;
#endif
      }
      else
        textColor = TColor::SysBtnText;

      dc.DrawText(&text[0], len, textRect, DT_WORDBREAK|DT_CALCRECT);
    }
  }

  // If we have text and/or glyph, lay them out and paint them
  //
  if (!textRect.IsNull() || !glyphRect.IsNull()) {

    LayoutTextGlyph(rect, textRect, glyphRect);

    // Offset things to the lower right if we're in down
    //
    if (IsSet(biPushed)) {
      if (!glyphRect.IsNull() && index != 1)
        glyphRect.Offset(1, 1);
      if (!textRect.IsNull())
        textRect.Offset(1, 1);
    }

    // Paint the components of the button
    //
    if (!glyphRect.IsNull()) {
      PRECONDITION(CelArray);

      // Create a UI Face object for this button & let it paint the button face.
      //
      TRect srcRect(CelArray->CelRect(index));
      TUIFace face(rect, *CelArray, TColor::LtGray, TColor::LtGray);
      const TPoint dstPt = glyphRect.TopLeft();
      const TUIFace::TState state = TUIFace::Normal;
      const bool pressed = false;
      face.Paint(dc, srcRect, dstPt, state, pressed, false); // no fill (transparent)
    }
    if (!textRect.IsNull()) {
      // Paint disabled text in 3d style
      if (IsSet(biDisabled)) {
        TUIFace(textRect, &text[0], TColor::LtGray,
            DT_WORDBREAK|DT_VCENTER|DT_CENTER).Paint(dc, textRect.TopLeft(),
            TUIFace::Disabled, false, false);
      }
      else{
        int mode = dc.SetBkMode(TRANSPARENT);
        TColor oldColor = dc.SetTextColor(textColor);
        dc.DrawText(&text[0], len, textRect, DT_WORDBREAK|DT_VCENTER|DT_CENTER);
        dc.SetTextColor(oldColor);
        dc.SetBkMode(mode);
      }
    }
  }

  // Paint the focus rect [if necessary]
  //
  if (IsSet(biFocus))
  {
    if (isThemed || textRect.IsNull()) 
      PaintFocusRect(dc, rect.InflatedBy(-LayoutMargin, -LayoutMargin));
    else 
      PaintFocusRect(dc, textRect);
  }

  // Restore font
  //
  if (len && (BtnFont || tmpFnt))
    dc.RestoreFont();
}

//
/// Virtual routine invoked to retrieve the placement of text and glyph when drawing
/// the button.
///
/// Override this routine to customize the layout logic and support
/// custom layout styles.
//
void
TGlyphButton::LayoutTextGlyph(const TRect& faceRect, TRect& textRect,
                              TRect& glyphRect)
{
  // Must have either text or a glyph
  //
  PRECONDITION(!textRect.IsNull() || !glyphRect.IsNull());

  // First check for the case where we've got either
  // text or glyph - but not both
  //
  if (textRect.IsNull() || glyphRect.IsNull()) {
    TRect& centerRect = textRect.IsNull() ? glyphRect : textRect;

    centerRect.Offset(faceRect.left, faceRect.top);
    if (centerRect.Width() < faceRect.Width())
      centerRect.Offset((faceRect.Width() - centerRect.Width())/2, 0);
    else
      centerRect.right = faceRect.right;
    if (centerRect.Height() < faceRect.Height())
      centerRect.Offset(0, (faceRect.Height() - centerRect.Height())/2);
    else
      centerRect.bottom = faceRect.bottom;
  }

  // Here we attempt to layout both the glyph and text
  //
  else {
    // Align upper left corners of face, text and glyph rectangles
    //
    glyphRect.Offset(faceRect.left, faceRect.top);
    textRect.Offset(faceRect.left, faceRect.top);

    // Compute amount of 'extra' space, if any, and how to partition it
    // between the two items
    //
    int hspace  = faceRect.Width() - glyphRect.Width()- textRect.Width() -
                 LayoutMargin*2;
    int vspace  = faceRect.Height() - glyphRect.Height()- textRect.Height() -
                 LayoutMargin*2;
    int gDelta=0;
    int tDelta=0;

    switch (LayStyle) {
      case lsH_SGST: {
           gDelta = hspace >= 0 ? LayoutMargin + hspace/3 :
                                 LayoutMargin + hspace/2;
          tDelta = hspace >= 0 ? gDelta + glyphRect.Width() + hspace/3 :
                                 gDelta + glyphRect.Width();
        }
        break;
      case lsH_GST: {
          gDelta = hspace >= 0 ? LayoutMargin : LayoutMargin + hspace/2;
          tDelta = hspace >= 0 ? gDelta + glyphRect.Width() + hspace/2 :
                                    gDelta + glyphRect.Width();
        }
        break;
       case lsH_STSG: {
           tDelta = hspace >= 0 ? LayoutMargin + hspace/3 :
                                 LayoutMargin + hspace/2;
          gDelta = hspace >= 0 ? tDelta + textRect.Width() + hspace/3 :
                                 tDelta + textRect.Width();
        }
        break;
      case lsH_TSGS: {
          tDelta = 0;
          gDelta = hspace >= 0 ? textRect.Width() + hspace/3 : textRect.Width();
          if(IsSet(biDefault))
            tDelta--;
        }
        break;
      case lsV_SGST: {
           gDelta = vspace >= 0 ? LayoutMargin + vspace/3 :
                                 LayoutMargin + vspace/2;
          tDelta = vspace >= 0 ? gDelta + glyphRect.Height() + vspace/3 :
                                 gDelta + glyphRect.Height();
        }
        break;
      case lsV_GST: {
          gDelta = vspace >= 0 ? LayoutMargin : LayoutMargin + vspace/2;
          tDelta = vspace >= 0 ? gDelta + glyphRect.Height() + vspace/2 :
                                    gDelta + glyphRect.Height();
        }
        break;
      case lsV_STSG: {
           tDelta = vspace >= 0 ? LayoutMargin + vspace/3 :
                                 LayoutMargin + vspace/2;
          gDelta = vspace >= 0 ? tDelta + textRect.Height() + vspace/3 :
                                 tDelta + textRect.Height();
        }
        break;
      case lsV_TSGS: {
          tDelta = 0;
          gDelta = vspace >= 0 ? textRect.Height() + vspace/3 : textRect.Height();
          if(IsSet(biDefault))
            tDelta--;
        }
        break;
       default:
        break;
    }

    int gx=0,gy=0;
    if (LayStyle == lsH_SGST || LayStyle == lsH_GST ||
        LayStyle == lsH_STSG || LayStyle == lsH_TSGS ) {

      // Center vertically
      //
      if (textRect.Height() < faceRect.Height())
        textRect.Offset(0, (faceRect.Height() - textRect.Height())/2);
      if (glyphRect.Height() < faceRect.Height()){
        gy = (faceRect.Height() - glyphRect.Height())/2;
        if(IsSet(biDefault)){
          gx--;
         if(LayStyle == lsH_STSG || LayStyle == lsH_TSGS)
           gx+=2;
        }
      }
      else{
        gy = (faceRect.Height() - glyphRect.Height())/2;
        if(IsSet(biDefault))
           gx--,gy--;
      }

      // Layout horizontally
      //
      glyphRect.Offset(gx+gDelta, gy);
      textRect.Offset(tDelta, 0);
    }
    else if (LayStyle == lsV_SGST || LayStyle == lsV_GST ||
             LayStyle == lsV_STSG || LayStyle == lsV_TSGS ) {

      // Or center horizontally
      if (textRect.Width() < faceRect.Width())
        textRect.Offset((faceRect.Width() - textRect.Width())/2, 0);
      if (glyphRect.Width() < faceRect.Width()){
        gx += (faceRect.Width() - glyphRect.Width())/2;
        if(IsSet(biDefault)){
          gy--;
         if(LayStyle == lsV_STSG || LayStyle == lsV_TSGS)
           gy+=2;
        }
      }
      else{
        gy = (faceRect.Height() - glyphRect.Height())/2;
        if(IsSet(biDefault))
          gx--,gy--;
      }
      // Layout vertically
      glyphRect.Offset(gx, gy+gDelta);
      textRect.Offset(0, tDelta);
    }
  }
}

//
/// Specifies a 'style' describing how text and glyph should be laid out.
/// Invalidates the window if necessary.
//
void
TGlyphButton::SetLayoutStyle(TLayoutStyle style)
{
  if (style != LayStyle) {
    LayStyle = style;
    if (GetHandle())
      Invalidate();
  }
}


//
/// Sets text coordinates and invalidates window if necessary.
//
void
TGlyphButton::SetTextOrigin(int x, int y)
{
  if (x != xText || y != yText) {
    xText = x;
    yText = y;
    if (GetHandle())
      Invalidate();
  }
}


//
/// Sets the upper left corner of glyphs and invalidates window if necessary.
//
void
TGlyphButton::SetGlyphOrigin(int x, int y)
{
  if (x != xGlyph || y != yGlyph) {
    xGlyph = x;
    yGlyph = y;
    if (GetHandle())
      Invalidate();
  }
}


//
/// Displays a focus rectangle.
//
void
TGlyphButton::PaintFocusRect(TDC& dc, const TRect& faceRect)
{
  PRECONDITION(IsSet(biFocus));
  TRect focusRect = faceRect;
  focusRect.Inflate(FaceToFocusRectDelta, FaceToFocusRectDelta);
  dc.DrawFocusRect(focusRect);
}

//
/// Repaints window right away by retrieving a client DC and invoking the 'Paint'
/// method.
//
void
TGlyphButton::PaintNow()
{
  TRect rect;
  GetClientRect(rect);
  TClientDC dc(*this);
  Paint(dc, false, rect);
}

} // OWL namespace
/* ========================================================================== */


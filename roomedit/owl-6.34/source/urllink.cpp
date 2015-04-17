//------------------------------------------------------------------------------
// ObjectWindows
// Bidus Yura 1998
//
/// \file
/// Implementation of class TUrlLink
//
//------------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/registry.h>
#include <owl/tooltip.h>
#include <owl/commctrl.h>
#include <owl/shellitm.h>

#include <owl/urllink.h>

#include <owl/urllink.rh>

#define TOOLTIP_ID 1003

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);        // General Controls diagnostic group


//----------------------------------------------------------------------------
DEFINE_RESPONSE_TABLE1(TUrlLink, TStatic)
  EV_WM_PAINT,
  EV_WM_LBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_CTLCOLOR,
  EV_WM_GETFONT,
  EV_WM_SETFONT,
  EV_WM_GETDLGCODE,
END_RESPONSE_TABLE;


/// Constructs a TUrlLink control where parent points to the parent window, id is
/// the control's ID, title is the control's caption, x and y are the coordinates in
/// the parent window where the controls are to be placed, w and h are the width and
/// height of the control, textLimit is the maximum amount of text reserved for the
/// caption, and module is the library resource ID for the control.
TUrlLink::TUrlLink(TWindow* parent, int id, LPCTSTR title,
             int x, int y, int w, int h, uint textLimit/* = 0*/,
             TModule* module/* = 0*/)
:
  TStatic(parent, id, title,x, y, w, h, textLimit,module),
  LinkFont(0),
  Tooltip(0),
  LinkColor(TColor::LtBlue),//
  VisitedColor(85,  26, 139),
  HoverColor(TColor::LtRed),
  bUnderline(true),
  bVisited(false),
  bOverControl(false)
{
  SetupCursor();
}

//
/// String-aware overload
//
TUrlLink::TUrlLink(
  TWindow* parent, 
  int id, 
  const tstring& title,
  int x, int y, int w, int h, 
  uint textLimit,
  TModule* module
  )
  : TStatic(parent, id, title, x, y, w, h, textLimit, module),
  LinkFont(0),
  Tooltip(0),
  LinkColor(TColor::LtBlue),
  VisitedColor(85, 26, 139),
  HoverColor(TColor::LtRed),
  bUnderline(true),
  bVisited(false),
  bOverControl(false)
{
  SetupCursor();
}

/// If a TUrlLink control is part of a dialog resource, its ID can be used to
/// construct a corresponding (or alias) ObjectWindows object. You can use this
/// constructor if a TUrlLink control has already been defined in the application's
/// resource file. resourceId is the resource ID of the TUrlLink control in the
/// resource file. textLimit is the maximum amount of text reserved for the caption.
TUrlLink::TUrlLink(TWindow* parent, int resourceId, uint textLimit/* = 0*/,
                   TModule* module/* = 0*/)
:
  TStatic(parent, resourceId, textLimit, module),
  LinkFont(0),
  Tooltip(0),
  LinkColor(TColor::LtBlue),
  VisitedColor(85,  26, 139),
  HoverColor(TColor::LtRed),
  bUnderline(true),
  bVisited(false),
  bOverControl(false)
{
  SetupCursor();
}

/// Destroys the TUrlLink object.
TUrlLink::~TUrlLink()
{
  delete LinkFont;
}

//
/// Override TWindow virtual member function to get the Window class name:
/// "OWL_UrlLink"
//
TWindow::TGetClassNameReturnType
TUrlLink::GetClassName()
{
  return OWL_URLLINK;
}

/// Override TWindow virtual member function to set up the font, set the text, and
/// enable the tool tip.
//
void
TUrlLink::SetupWindow()
{
  TStatic::SetupWindow();

  // Try to get our parent's font.
  //
  HFONT font = GetParentO()->GetWindowFont();

  // Settle for the ANSI proportional one.
  //
  if (font)
  {
    LOGFONT logFont = TFont(font).GetObject();
    logFont.lfUnderline = bUnderline;
    LinkFont = new TFont(logFont);
  }
  else
  {
    LOGFONT logFont = TDefaultGUIFont().GetObject();
    logFont.lfUnderline = bUnderline;
    LinkFont = new TFont(logFont);
  }
  CHECK(LinkFont != 0);

  // If no URL is given, get it from the window text.
  // Visa versa, if the window text is empty, update it using the URL.
  //
  tstring text = GetText();
  if (UrlAddress.empty())
    UrlAddress = text;
  else if (text.empty())
    SetText(UrlAddress);
  WARNX(OwlControl, UrlAddress.empty(), 0, 
    "Both control text and UrlAddress are empty in control #" << GetId() << ".");

  PositionWindow();

  if (Tooltip && !Tooltip->GetHandle()){
    // Make sure tooltip is disabled so it does not take input focus
    Tooltip->ModifyStyle(0,WS_DISABLED);
    Tooltip->Create();
  }
  if(Tooltip){
    TRect rect;
     GetClientRect(rect);
    TToolInfo ti(*this, rect, TOOLTIP_ID);
    ti.SetText(UrlAddress.c_str());
    Tooltip->AddTool(ti);
  }
}
//
void       
TUrlLink::EnableTooltip(bool enable)
{
  if (!Tooltip) {
    // Create and initialize tooltip
    //
    SetTooltip(new TTooltip(this));
  }
  else {
    if (Tooltip->GetHandle())
      Tooltip->Activate(enable);
  }
}
//
void      
TUrlLink::SetTooltip(TTooltip* tooltip)
{
  // Cleanup; via Condemned list if tooltip was created
  //
  if (Tooltip) {
    if (Tooltip->GetHandle())
      Tooltip->SendMessage(WM_CLOSE);
    else
      delete Tooltip;
  }

  // Store new tooltip and create if necessary
  //
  Tooltip = tooltip;
  if (Tooltip) {
    if(GetHandle() && !Tooltip->GetHandle()) {
      // Make sure tooltip is disabled so it does not take input focus
      Tooltip->ModifyStyle(0,WS_DISABLED);
      Tooltip->Create();
      TRect rect;
       GetClientRect(rect);
      TToolInfo ti(*this, rect, TOOLTIP_ID);
      ti.SetText(UrlAddress.c_str());
      Tooltip->AddTool(ti);
    }
  }
}

//
/// Relay 'interesting' messages to the tooltip window
//
bool
TUrlLink::PreProcessMsg(MSG& msg)
{
  // Check if this message might be worth relaying to the tooltip window
  //
  if (Tooltip && Tooltip->IsWindow()) {
    if (msg.hwnd == *this || IsChild(msg.hwnd)) {
      if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
        Tooltip->RelayEvent(msg);
      }
    }
  }

  // Always let event go through.
  //
  return TStatic::PreProcessMsg(msg);
}

//
/// Window proc. of control to handle messages sent before OWL thunks
//
LRESULT CALLBACK
UrlLinkProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg) {
    case WM_GETDLGCODE:
      return DLGC_STATIC|DLGC_WANTMESSAGE;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}


/// Override TWindow virtual member function to fills out information about the
/// Window class associated with a TUrlLink control. 
/// \note The class information is based on the system's "STATIC" class.
//
void
TUrlLink::GetWindowClass(WNDCLASS& wndClass)
{
  // Grab a the attributes of the native "STATIC" control
  if (::GetClassInfo(0, _T("STATIC"), &wndClass)){
    wndClass.hInstance       = *GetModule();
    wndClass.style           = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC;
    wndClass.lpszClassName   = GetClassName();
    wndClass.lpfnWndProc     = UrlLinkProc;
    wndClass.hCursor         = HCursor;
    //wndClass.hbrBackground  = HBRUSH(COLOR_BTNFACE+1);
    wndClass.hbrBackground  = NULL; // Use the background of the parent
  }
  else {
    TStatic::GetWindowClass(wndClass);
    wndClass.style           = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC;
    //wndClass.hbrBackground  = HBRUSH(COLOR_BTNFACE+1);
    wndClass.hbrBackground  = NULL; // Use the background of the parent
    wndClass.hCursor         = HCursor;
  }
}

//
/// Override TWindow virtual member function to paint the control.
//
void
TUrlLink::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
  TPointer<TFont>  tmpFnt; // Object wrapping font handle
  TAPointer<tchar> text;   // Pointer to caption dynamic buffer
  // Select the font
  if(!LinkFont){
    HFONT hFont = HFONT(::SendMessage(::GetParent(*this), WM_GETFONT, 0, 0));
    if (!hFont)
      hFont = HFONT(GetStockObject(ANSI_VAR_FONT));
    if (hFont)
      tmpFnt = new TFont(hFont);
  }
  if (LinkFont) {
    CHECK(LinkFont->IsGDIObject());
    dc.SelectObject(*LinkFont);
  }
  else if (tmpFnt) {
    CHECK(((TFont&)tmpFnt).IsGDIObject());
    dc.SelectObject((TFont&)tmpFnt);
  }
  int len = GetWindowTextLength();
  text = new tchar[len+1];
  GetWindowText(text, len+1);
  TRect textRect;
  GetClientRect(textRect);

  TColor textColor = LinkColor;
  if (bOverControl)
    textColor = HoverColor;
  else if (bVisited)
    textColor = VisitedColor;

  TColor oldColor = dc.SetTextColor(textColor);
  int oldMode = dc.SetBkMode(TRANSPARENT);
  dc.DrawText(&text[0], len, textRect, 0);
  dc.SetBkMode(oldMode);
  dc.SetTextColor(oldColor);

  // Restore font
  if (LinkFont || tmpFnt)
    dc.RestoreFont();
}


//
/// Handles WM_PAINT messages to allow control to paint according to its current state.
//
void
TUrlLink::EvPaint()
{
  TPaintDC dc(*this);
  TRect&   rect = *(TRect*)&dc.Ps.rcPaint;
  Paint(dc, dc.Ps.fErase, rect);
}
//
/// Handles WM_CTLCOLOR. If a background color is set it is passed to the control.
//
HBRUSH
TUrlLink::EvCtlColor(HDC hDC, HWND hWndChild, uint ctlType)
{
  TDC dc(hDC);
  if (bOverControl)
    dc.SetTextColor(HoverColor);
  else if (bVisited)
    dc.SetTextColor(VisitedColor);
  else
    dc.SetTextColor(LinkColor);
  return TStatic::EvCtlColor(hDC, hWndChild, ctlType);
}

/// Handles WM_LBUTTONDOWN to process URL requests
//
void
TUrlLink::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TStatic::EvLButtonDown(modKeys, point);
  // set url !!!!!!!!!!!!!!!!!!1
  int result = (int)GotoURL(UrlAddress.c_str(), SW_SHOW);
  if (result > (int)HINSTANCE_ERROR){
    ReleaseCapture();
    bOverControl = false;
    Invalidate();
    SetVisited();                        // Repaint to show visited colour
  }
  else{
    MessageBeep(MB_ICONEXCLAMATION);     // Unable to follow link
    ReportError(result);
  }
}

//
/// Handles WM_MOUSEMOVE to monitor mouse location when processing mouse down/dblclk
/// requests. Updates state of button if we're in 'capture' mode.
//
void
TUrlLink::EvMouseMove(uint modKeys, const TPoint& point)
{
  TStatic::EvMouseMove(modKeys, point);
  if(bOverControl){
    TRect rect;
    GetClientRect(rect);
    if(!rect.Contains(point)){
      ReleaseCapture();
      bOverControl = false;
      Invalidate();
    }
  }
  else{
     SetCapture();
    bOverControl = true;
    Invalidate();
  }
}

//
/// Handles WM_GETFONT. Returns font used by control if one was specified earlier.
/// Otherwise, returns 0.
//
HFONT
TUrlLink::EvGetFont()
{
  PRECONDITION(!LinkFont || LinkFont->IsGDIObject());
  return LinkFont ? HFONT(*LinkFont) : HFONT(0);
}


//
/// Handles WM_SETFONT. Deletes any cached font and stores copy of new one.
//
void
TUrlLink::EvSetFont(HFONT hFont, bool redraw)
{
  TStatic::EvSetFont(hFont, redraw);
  try
  {
    LOGFONT logFont = TFont(hFont).GetObject();
    logFont.lfUnderline = bUnderline;
    delete LinkFont;
    LinkFont = new TFont(logFont);
    if (redraw)
      Invalidate();  
  }
  catch(const TXGdi&)
  {
    WARN(true, "TUrlLink::EvSetFont failed.");
  }
}

/// Handles WM_GETDLGCODE.
//
uint
TUrlLink::EvGetDlgCode(const MSG* /*msg*/)
{
  return DLGC_STATIC|DLGC_WANTMESSAGE;
}

//
/// Sets the caption of the control.
//
void
TUrlLink::SetURL(LPCTSTR str)
{
  UrlAddress = str;

  if(Tooltip && GetHandle()){
    TRect rect;
     GetClientRect(rect);
    TToolInfo info(*this, rect, TOOLTIP_ID);
    Tooltip->GetToolInfo(info);
    info.SetText(str);
    Tooltip->UpdateTipText(info);
  }
}

//
/// Loads a "hand" cursor from the winhlp32.exe module.
//
/// \note It appeared in Paul DiLascia's Jan 1998 MSJ articles.
void
TUrlLink::SetupCursor()
{
  SetCursor(GetModule(), IDC_HANDCURSOR);
  if(HCursor == 0){
    // if was problem try load cursor from winhlp32.exe
    // Get the windows directory
    TAPointer<tchar> Buffer(new tchar[MAX_PATH]);
    ::GetWindowsDirectory(Buffer, MAX_PATH);
     _tcscat(Buffer,_T("\\winhlp32.exe"));
    // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
    HMODULE hModule = ::LoadLibrary(Buffer);
    if (hModule) {
      HCURSOR hHandCursor = ::LoadCursor(hModule, TResId(106));
      if (hHandCursor)
        HCursor = CopyCursor(hHandCursor); // it is a macro in Win32

      ::FreeLibrary(hModule);
    }
  }
}

//
/// Sets whether the hyperlink has been visited.
//
void
TUrlLink::SetVisited(bool visited/* = true*/)
{
  bVisited = visited;
  if (GetHandle())
    Invalidate();
}

//
/// Sets whether the caption should be displayed in underlined text
//
void
TUrlLink::SetUnderline(bool bUnderline/* = true */)
{
  if(bUnderline == bUnderline)
    return;
  bUnderline = bUnderline;
  if (GetHandle())
  {
    LOGFONT logFont = LinkFont->GetObject();
    logFont.lfUnderline = bUnderline;
    delete LinkFont;
    LinkFont = new TFont(logFont);
    Invalidate();
  }
}

//
/// Sets the color used to indicate this control is a hyperlink.
//
void
TUrlLink::SetLinkColor(TColor& color)
{
  LinkColor = color;
  if (GetHandle())
    Invalidate();
}

//
/// Sets the color used to display the control after it has been used.
//
void
TUrlLink::SetVisitedColor(TColor& color)
{
  VisitedColor = color;
  if (GetHandle())
    Invalidate();
}

//
/// Sets the color used to display the control when the mouse cursor is over the
/// control.
//
void
TUrlLink::SetHoverColor(TColor& color)
{
  HoverColor = color;
  if (GetHandle())
    Invalidate();
}

/// Move and resize the window so that the window is the same size as the hyperlink
/// text. This stops the hyperlink cursor being active when it is not directly over
/// the text. If the text is left justified then the window is merely shrunk, but if
/// it is centred or right justified then the window will have to be moved as well.
//
// Suggested by Pål K. Tønder
void
TUrlLink::PositionWindow()
{
  // Get the current window position
  TRect rect;
  GetWindowRect(rect);

  TWindow* pParent = GetParentO();
  if(pParent){
    pParent->ScreenToClient(rect.TopLeft());
    pParent->ScreenToClient(rect.BottomRight());
  }

  // Get the size of the window text
  TTmpBuffer<tchar> buffer(MAX_PATH);
  GetText(buffer, MAX_PATH);

  TSize size = LinkFont->GetTextExtent(&buffer[0]);

  // Get the text justification via the window style
  uint32 style = GetStyle(); //TWindow

  // Recalc the window size and position based on the text justification
  if (style & SS_CENTERIMAGE)
    rect.Inflate(0, -(rect.Height() - size.cy)/2);
  else
    rect.bottom = rect.top + size.cy;

   if (style & SS_CENTER)
    rect.Inflate( -(rect.Width() - size.cx)/2, 0);
  else if (style & SS_RIGHT)
    rect.left  = rect.right - size.cx;
  else // SS_LEFT = 0, so we can't test for it explicitly
    rect.right = rect.left + size.cx;

  // Move the window
  SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
}

//
/// "ShellExecute" is called to open the URL, but if this fails, then the registry
/// is examined in order to find a likely candidate for .html files. If one is found
/// then this it is launched with the hope that it can handle the URL string
/// supplied.  
//
HINSTANCE
TUrlLink::GotoURL(LPCTSTR url, int showcmd)
{
  // First try ShellExecute()
  HINSTANCE result = TShell::ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

  // If it failed, get the .htm regkey and lookup the program
   if ((uint)result <= (uint)HINSTANCE_ERROR){
    tstring tmp_str = _T(".htm");
    tstring key = TRegValue(TRegKey::GetClassesRoot(),tmp_str.c_str()).GetName();
    if(!key.empty()){
      tmp_str = key;
      tmp_str += _T("\\shell\\open\\command");
      key = TRegValue(TRegKey::GetClassesRoot(),tmp_str.c_str()).GetName();
      if(!key.empty()){
        size_t pos = key.find(tstring(_T("\"%1\"")));
        if(pos == NPOS){
          pos = key.find(tstring(_T("%1")));
          if(pos != NPOS)
            key.erase(pos);
        }
        else
          key.erase(pos);
        key += _T(" ");
        key += url;
        _USES_CONVERSION;
        result = (HINSTANCE) WinExec(_W2A(key.c_str()), showcmd);
      }
    }
  }
  return result;
}

//
/// Displays a message box with an error message loaded from the resource file.
//
void
TUrlLink::ReportError(int error)
{
  uint resId;
  switch (error) {
    case 0:                       resId = IDS_OUTOFMEMORY; break;
    case SE_ERR_PNF:              resId = IDS_PATHNOTFOUND; break;
    case SE_ERR_FNF:              resId = IDS_FILENOTFOUIND; break;
    case ERROR_BAD_FORMAT:        resId = IDS_FILEINVALID; break;
    case SE_ERR_ACCESSDENIED:     resId = IDS_ACCESSDENIED; break;
    case SE_ERR_ASSOCINCOMPLETE:  resId = IDS_INVALIDASSOC; break;  //  Win32/Win16
    case SE_ERR_DDEBUSY:          resId = IDS_DDEBISY; break;        //  Win32/Win16
    case SE_ERR_DDEFAIL:          resId = IDS_DDEFAILED; break;      //  Win32/Win16
    case SE_ERR_DDETIMEOUT:       resId = IDS_DDETYMEOUT; break;    //  Win32/Win16
    case SE_ERR_DLLNOTFOUND:      resId = IDS_FILENOTFOUIND; break;
    case SE_ERR_NOASSOC:          resId = IDS_NOINSTANCE; break;    //  Win32/Win16
    case SE_ERR_OOM:              resId = IDS_OUTOFMEMORY; break;
    case SE_ERR_SHARE:            resId = IDS_SHAREDERROR; break;    //  Win32/Win16
    default:{
      FormatMessageBox(IDS_UNCKNOWNERR, 0, MB_ICONEXCLAMATION | MB_OK, error);
      return;
    }
  }
  //str = "Unable to open hyperlink:\n\n" + str;
  MessageBox(resId, _T(""), MB_ICONEXCLAMATION | MB_OK);
  return;
}

IMPLEMENT_STREAMABLE1(TUrlLink, TStatic);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TUrlLink::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TStatic*)GetObject(), is);

  GetObject()->LinkFont = 0;

  is >> GetObject()->LinkColor;
  is >> GetObject()->VisitedColor;
  is >> GetObject()->HoverColor;
  is >> GetObject()->bUnderline;
  is >> GetObject()->bVisited;
  is >> GetObject()->bOverControl;

  GetObject()->SetupCursor();

  return GetObject();
}

//
//
//
void
TUrlLink::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TStatic*)GetObject(), os);

  os << GetObject()->LinkColor;
  os << GetObject()->VisitedColor;
  os << GetObject()->HoverColor;
  os << GetObject()->bUnderline;
  os << GetObject()->bVisited;
  os << GetObject()->bOverControl;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


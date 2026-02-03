// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// HLinkCtl.cpp: implementation file
// Version:      1.0
// Date:         17.06.1998
// Author:       Dieter Windau
//
// THLinkCtrl is a freeware OWL class that supports Internet Hyperlinks from
// standard Windows applications just like they are displayed in Web browsers.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02 under Windows NT 4.0 SP3 but I think
// the class would work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This code is based on MFC class CHLinkCtrl by PJ Naughter
// Very special thanks to PJ Naughter:
//   EMail: pjn@indigo.ie
//   Web:   http://indigo.ie/~pjn
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/SoftEngage
// ****************************************************************************
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/hlinkctl.h>

#ifndef HLINK_NOOLE
#define INITGUID
#endif

#ifndef HLINK_NOOLE
#include <initguid.h>
#endif

#include <winnetwk.h>
#include <winnls.h>
#include <shlobj.h>

#include <algorithm>

#ifndef HLINK_NOOLE
#include <intshcut.h>
#endif

#include <owlext/hlinkctl.rh>

namespace OwlExt {

using namespace owl;

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

// ***************************** THLinkCtrl ***********************************
DEFINE_RESPONSE_TABLE1(THLinkCtrl, TEdit)
EV_WM_CONTEXTMENU,
EV_WM_SETCURSOR,
EV_WM_ERASEBKGND,
EV_WM_PAINT,
EV_WM_LBUTTONDOWN,
EV_WM_SETFOCUS,
EV_WM_MOUSEMOVE,
EV_COMMAND(ID_POPUP_COPYSHORTCUT, EvCopyShortcut),
EV_COMMAND(ID_POPUP_PROPERTIES, EvProperties),
EV_COMMAND(ID_POPUP_OPEN, EvOpen),
#ifndef HLINK_NOOLE
EV_COMMAND(ID_POPUP_ADDTOFAVORITES, EvAddToFavorites),
EV_COMMAND(ID_POPUP_ADDTODESKTOP, EvAddToDesktop),
#endif
END_RESPONSE_TABLE;

THLinkCtrl::THLinkCtrl(TWindow* parent, int Id, LPCTSTR text, int x,
             int y, int w, int h, uint textLimit, bool multiline, TModule* module)
             :
TEdit(parent, Id, text, x, y, w, h, textLimit, multiline, module)
{
  Init();
}

THLinkCtrl::THLinkCtrl(TWindow* parent, int resourceId, uint textLimit,
             TModule* module)
             :
TEdit(parent, resourceId, textLimit, module)
{
  Init();
}

THLinkCtrl::~THLinkCtrl()
{
}

void THLinkCtrl::Init()
{
  m_Color = RGB(0, 0, 255);
  m_VisitedColor = RGB(128, 0, 128);
  m_HighlightColor = RGB(255, 0, 0);

  m_bShrinkToFit = true;
  m_bUseHighlight = true;
  m_bUseShadow = true;
  m_State = ST_NOT_VISITED;
  m_OldState = ST_NOT_VISITED;
  m_bUnderline = true;
  m_bShowingContext = false;
  m_bShowPopupMenu = true;

  // Load up the cursors
  m_hLinkCursor = (FindResourceModule(Parent,GetModule(),IDC_HLINK,RT_CURSOR))->LoadCursor(IDC_HLINK);
  m_hArrowCursor = ::LoadCursor(0, TResId(IDC_ARROW));

  // Control should be created read only
  CHECK(GetStyle() & ES_READONLY);

  //  Control should not be in the tab order
  CHECK(!(GetStyle() & WS_TABSTOP));
}

void THLinkCtrl::ShrinkToFitEditBox()
{
  TWindow* pParent = GetParentO();
  if (pParent == 0)
    return;

  TClientDC dc(GetHandle());
  TSize TextSize = dc.GetTextExtent(GetHyperLinkDescription(),
    static_cast<int>(GetHyperLinkDescription().size()));

  TRect ControlRect = GetWindowRect();
  pParent->ScreenToClient(ControlRect.TopLeft());
  pParent->ScreenToClient(ControlRect.BottomRight());

  int width = std::min(ControlRect.Size().cx, TextSize.cx);
  if (ControlRect.Size().cx != width){
    // we need to Shink the edit box
    if (GetStyle() & ES_RIGHT){
      // keep everything the same except the width. Set that to the width of text
      MoveWindow(ControlRect.right-width,
        ControlRect.top, width, ControlRect.Height());
    }
    else if (GetStyle() & ES_CENTER){
      MoveWindow(ControlRect.left+(ControlRect.Width()-width)/2,
        ControlRect.top, width, ControlRect.Height());
    }
    else
    {
      MoveWindow(ControlRect.left,
        ControlRect.top, width, ControlRect.Height());
    }
  }
}

void THLinkCtrl::SetHyperLink(const owl::tstring& sActualLink)
{
  SetActualHyperLink(sActualLink);

  // if the edit field has no caption, set the description with the hyperlink
  //
  owl::tstring str = GetHyperLinkDescription();
  if (_tcslen(str.c_str()) == 0)
    SetHyperLinkDescription(sActualLink);

  if (m_bShrinkToFit)
    ShrinkToFitEditBox();
}

void THLinkCtrl::SetHyperLink(const owl::tstring& sActualLink,
                const owl::tstring& sLinkDescription, bool bShrinkToFit)
{
  m_bShrinkToFit = bShrinkToFit;
  SetActualHyperLink(sActualLink);
  SetHyperLinkDescription(sLinkDescription);
  if (m_bShrinkToFit)
    ShrinkToFitEditBox();
}

void THLinkCtrl::SetActualHyperLink(const owl::tstring& sActualLink)
{
  m_sActualLink = sActualLink;
}

void THLinkCtrl::SetHyperLinkDescription(const owl::tstring& sLinkDescription)
{
  // if empty do nothing
  if (_tcslen(sLinkDescription.c_str()) != 0)
  {
    //  set the text on this control
    SetWindowText(sLinkDescription.c_str());
  }
}

owl::tstring THLinkCtrl::GetHyperLinkDescription() const
{
  int len = GetWindowTextLength();
  if (len > 0)
  {
    _TCHAR* str = new _TCHAR[len+1];
    GetWindowText(str, len+1);
    owl::tstring sDescription(str);
    delete[] str;
    return sDescription;
  }
  else
  {
    owl::tstring str(_T(""));
    return str;
  }
}

bool THLinkCtrl::EvSetCursor(THandle /*hWndCursor*/, uint /*hitTest*/,
               uint /*mouseMsg*/)
{
  if (m_bShowingContext)
    ::SetCursor(m_hArrowCursor);
  else
    ::SetCursor(m_hLinkCursor);
  return true;
}

void THLinkCtrl::EvLButtonDown(uint /*modKeys*/, const TPoint& /*point*/)
{
  PostMessage(WM_COMMAND, ID_POPUP_OPEN);
}

void THLinkCtrl::EvSetFocus(HWND /*hWndLostFocus*/)
{
  // TEdit::EvSetFocus(pOldWnd);  // Eat the message
}

void THLinkCtrl::EvOpen()
{
  if (Open())
    m_State = ST_VISITED;
}

void THLinkCtrl::SetLinkColor(const TColor& color)
{
  m_Color = color;
  UpdateWindow();
}

void THLinkCtrl::SetVisitedLinkColor(const TColor& color)
{
  m_VisitedColor = color;
  UpdateWindow();
}

void THLinkCtrl::SetHighlightLinkColor(const TColor& color)
{
  m_HighlightColor = color;
  UpdateWindow();
}

void THLinkCtrl::SetUseUnderlinedFont(bool bUnderline)
{
  m_bUnderline = bUnderline;
  UpdateWindow();
}

void THLinkCtrl::SetUseShadow(bool bUseShadow)
{
  m_bUseShadow = bUseShadow;
  UpdateWindow();
}

void THLinkCtrl::EvMouseMove(UINT modKeys, const TPoint& point)
{
  if (!m_bUseHighlight)
    return;

  TRect rc;
  GetClientRect(rc);
  if (rc.Contains(point)){
    if (m_State != ST_HIGHLIGHTED){
      SetCapture();
      HighLight(true);
    }
  }
  else{
    if (m_State == ST_HIGHLIGHTED){
      HighLight(false);
      ReleaseCapture();
    }
  }
  TEdit::EvMouseMove(modKeys, point);
}

void THLinkCtrl::HighLight(bool state)
{
  if (state){
    if (m_State != ST_HIGHLIGHTED){
      m_OldState = m_State;
      m_State = ST_HIGHLIGHTED;
      Invalidate();
    }
  }
  else{
    if (m_State == ST_HIGHLIGHTED){
      m_State = m_OldState;
      Invalidate();
    }
  }
}

bool THLinkCtrl::EvEraseBkgnd(HDC hdc)
{
  bool bSuccess = TEdit::EvEraseBkgnd(hdc);

  TRect r = GetClientRect();
  TDC dc(hdc);

  if ((m_State == ST_HIGHLIGHTED) && m_bUseShadow){
    // draw the drop shadow effect if highlighted
    TFont wFont(GetWindowFont());
    LOGFONT lf = wFont.GetObject();
    if (m_bUnderline)
      lf.lfUnderline = true;
    TFont font(lf);
    dc.SelectObject(font);
    int nOldMode = dc.SetBkMode(TRANSPARENT);
    _TCHAR sText[255];
    GetWindowText(sText, 255);
    TColor OldColor = dc.SetTextColor(GetSysColor(COLOR_3DSHADOW));
    r.Offset(2,2);
    uint16 format = DT_VCENTER;
    if (GetStyle() & ES_RIGHT)
    {
      format |= DT_RIGHT;
      r.right-=2;
    }
    else if (GetStyle() & ES_CENTER)
      format |= DT_CENTER;
    else
      format |= DT_LEFT;
    dc.DrawText(sText, static_cast<int>(_tcslen(sText)), r, format);
    dc.SetTextColor(OldColor);
    dc.SetBkMode(nOldMode);
    dc.RestoreFont();
  }
  else
  {
    dc.FillRect(r, TBrush(GetSysColor(COLOR_3DFACE)));
  }

  return bSuccess;
}

void THLinkCtrl::EvPaint()
{
  TPaintDC dc(GetHandle());

  // Make the font underline just like a URL
  TFont wFont = GetWindowFont();
  LOGFONT lf = wFont.GetObject();
  lf.lfUnderline = m_bUnderline;
  TFont font(lf);
  dc.SelectObject(font);
  int nOldMode = dc.SetBkMode(TRANSPARENT);

  TColor OldColor;
  switch (m_State){
    case ST_NOT_VISITED:
      OldColor = dc.SetTextColor(m_Color);
      break;
    case ST_VISITED:
      OldColor = dc.SetTextColor(m_VisitedColor);
      break;
    case ST_HIGHLIGHTED:
      OldColor = dc.SetTextColor(m_HighlightColor);
      break;
  }

  TRect r = GetClientRect();
  uint16 format = DT_VCENTER;
  if (GetStyle() & ES_RIGHT) {
    format |= DT_RIGHT;
    r.right-=2;
  }
  else if (GetStyle() & ES_CENTER)
    format |= DT_CENTER;
  else
    format |= DT_LEFT;

  dc.DrawText(GetHyperLinkDescription(),
    static_cast<int>(GetHyperLinkDescription().size()), r, format);

  dc.SetTextColor(OldColor);
  dc.SetBkMode(nOldMode);
  dc.RestoreFont();
}

void THLinkCtrl::EvContextMenu(HWND /*childHwnd*/, int x, int y)
{
  if (m_bShowPopupMenu == false)
    return;

  HighLight(false);
  ReleaseCapture();

  if (x == -1 && y == -1)
  {
    //  keystroke invocation
    TRect rect = GetClientRect();
    ClientToScreen(rect.TopLeft());
    x = rect.left + 5;
    y = rect.top + 5;
  }

  TMenu menu(*FindResourceModule(Parent,GetModule(),IDM_HLINK_POPUP,RT_MENU),
    IDM_HLINK_POPUP);

  HMENU pPopup = menu.GetSubMenu(0);

  PRECONDITION(pPopup);
  m_bShowingContext = true;

  ::TrackPopupMenu(pPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, GetHandle(), 0);

  m_bShowingContext = false;
}

void THLinkCtrl::EvCopyShortcut()
{
  if (::OpenClipboard(GetHandle()))
  {
    int nBytes = static_cast<int>(sizeof(TCHAR) * (m_sActualLink.size() + 1));
    HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nBytes);
    TCHAR* pData = (TCHAR*) GlobalLock(hMem);
    _tcscpy(pData, (LPCTSTR) m_sActualLink.c_str());
    GlobalUnlock(hMem);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
  }
}

void THLinkCtrl::EvProperties()
{
  ShowProperties();
}

void THLinkCtrl::ShowProperties() const
{
  THLinkSheet propSheet(GetParentO());
  propSheet.SetBuddy(this);
  propSheet.Execute();
}

bool THLinkCtrl::Open() const
{
  // Give the user some feedback
  TWaitCursor cursor;

#ifndef HLINK_NOOLE
  // First try to open using IUniformResourceLocator
  bool bSuccess = OpenUsingCom();

  // As a last resort try ShellExecuting the URL, may
  // even work on Navigator!
  if (!bSuccess)
    bSuccess = OpenUsingShellExecute();
#else
  bool bSuccess = OpenUsingShellExecute();
#endif
  return bSuccess;
}

#ifndef HLINK_NOOLE
bool THLinkCtrl::OpenUsingCom() const
{
  // Get the URL Com interface
  IUniformResourceLocator* pURL;
  HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, 0,
    CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pURL);
  if (!SUCCEEDED(hres)){
    TRACE("Failed to get the IUniformResourceLocator interface\n");
    return false;
  }

  hres = pURL->SetURL(m_sActualLink.c_str(), IURL_SETURL_FL_GUESS_PROTOCOL);
  if (!SUCCEEDED(hres)){
    TRACE("Failed in call to SetURL\n");
    pURL->Release();
    return false;
  }

  //  Open the URL by calling InvokeCommand
  URLINVOKECOMMANDINFO ivci;
  ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
  ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
  ivci.hwndParent = GetParentH();
  ivci.pcszVerb = _T("open");
  hres = pURL->InvokeCommand(&ivci);
  if (!SUCCEEDED(hres)){
    TRACE("Failed to invoke URL using InvokeCommand\n");
    pURL->Release();
    return false;
  }

  //  Release the pointer to IUniformResourceLocator.
  pURL->Release();
  return true;
}
#endif

bool THLinkCtrl::OpenUsingShellExecute() const
{
  HINSTANCE hRun = ShellExecute(GetParentH(), _T("open"),
    m_sActualLink.c_str(), 0, 0, SW_SHOW);
  if (reinterpret_cast<INT_PTR>(hRun) <= 32){
    TRACE("Failed to invoke URL using ShellExecute\n");
    return false;
  }
  return true;
}

#ifndef HLINK_NOOLE
bool THLinkCtrl::AddToSpecialFolder(int nFolder) const
{
  // Give the user some feedback
  TWaitCursor cursor;

  // Get the shell's allocator.
  IMalloc* pMalloc;
  if (!SUCCEEDED(SHGetMalloc(&pMalloc))){
    TRACE("Failed to get the shell's IMalloc interface\n");
    return false;
  }

  // Get the location of the special Folder required
  LPITEMIDLIST pidlFolder;
  HRESULT hres = SHGetSpecialFolderLocation(0, nFolder, &pidlFolder);
  if (!SUCCEEDED(hres)){
    TRACE("Failed in call to SHGetSpecialFolderLocation\n");
    pMalloc->Release();
    return false;
  }

  // convert the PIDL to a file system name and
  // add an extension of URL to create an Internet
  // Shortcut file
  TCHAR sFolder[_MAX_PATH];
  if (!SHGetPathFromIDList(pidlFolder, sFolder)){
    TRACE("Failed in call to SHGetPathFromIDList");
    pMalloc->Release();
    return false;
  }
  TCHAR sShortcutFile[_MAX_PATH];

  owl::tstring linkDescription = GetHyperLinkDescription();
  _tmakepath(sShortcutFile, 0, sFolder, linkDescription.c_str(), _T("URL"));

  // Free the pidl
  pMalloc->Free(pidlFolder);

  // Do the actual saving
  bool bSuccess = Save(sShortcutFile);

  // Release the pointer to IMalloc
  pMalloc->Release();

  return bSuccess;
}
#endif

#ifndef HLINK_NOOLE
void THLinkCtrl::EvAddToFavorites()
{
  AddToSpecialFolder(CSIDL_FAVORITES);
}
#endif

#ifndef HLINK_NOOLE
void THLinkCtrl::EvAddToDesktop()
{
  AddToSpecialFolder(CSIDL_DESKTOP);
}
#endif

#ifndef HLINK_NOOLE
bool THLinkCtrl::Save(const owl::tstring& sFilename) const
{
  // Get the URL Com interface
  IUniformResourceLocator* pURL;
  HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, 0, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**) &pURL);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed to get the IUniformResourceLocator interface\n");
    return false;
  }

  hres = pURL->SetURL(m_sActualLink.c_str(), IURL_SETURL_FL_GUESS_PROTOCOL);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed in call to SetURL\n");
    pURL->Release();
    return false;
  }

  // Get the IPersistFile interface for
  // saving the shortcut in persistent storage.
  IPersistFile* ppf;
  hres = pURL->QueryInterface(IID_IPersistFile, (void **)&ppf);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed to get the IPersistFile interface\n");
    pURL->Release();
    return false;
  }

  // Save the shortcut via the IPersistFile::Save member function.
#ifndef _UNICODE
  wchar_t wsz[_MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, sFilename.c_str(), -1, wsz, _MAX_PATH);
  hres = ppf->Save(wsz, true);
#else
  hres = ppf->Save(sFilename.c_str(), true);
#endif
  if (!SUCCEEDED(hres))
  {
    TRACE("IPersistFile::Save failed!\n");
    ppf->Release();
    pURL->Release();
    return false;
  }

  // Release the pointer to IPersistFile.
  ppf->Release();

  // Release the pointer to IUniformResourceLocator.
  pURL->Release();

  return true;
}
#endif

THLinkPage::THLinkPage(TPropertySheet* parent)
:
TPropertyPage(parent, IDD_HLINK_PROPERTIES,0,0,
        FindResourceModule(parent,0,IDD_HLINK_PROPERTIES,RT_DIALOG))
{
}

THLinkPage::~THLinkPage()
{
}

void THLinkPage::SetupWindow()
{
  TPropertyPage::SetupWindow();

  PRECONDITION(m_pBuddy);
  ::SetWindowText(::GetDlgItem(GetHandle(), IDC_NAME),
    m_pBuddy->GetHyperLinkDescription().c_str());
  ::SetWindowText(::GetDlgItem(GetHandle(), IDC_URL),
    m_pBuddy->GetActualHyperLink().c_str());
}

THLinkSheet::THLinkSheet(TWindow* parent)
:
TPropertySheet(parent, 0)
{
  m_page1 = new THLinkPage(this);
  SetCaption(FindResourceModule(Parent,GetModule(),IDS_HLINK_PROPERTIES,
    RT_STRING)->LoadString(IDS_HLINK_PROPERTIES).c_str());
  ModifyStyle(0,PSH_NOAPPLYNOW);
}

THLinkSheet::~THLinkSheet()
{
}
} // OwlExt namespace

//==============================================================================

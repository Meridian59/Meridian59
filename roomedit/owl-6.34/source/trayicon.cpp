///////////////////////////////////////////////////////////////////////////// 
// ObjectWindows 
// Copyright (c) 1998 by Bidus Yura 
//
/// \file
/// This is a conglomeration of ideas from the MSJ "Webster" application,
/// sniffing round the online docs, and from other implementations such
/// as PJ Naughter's "CTrayIconifyIcon" (http://indigo.ie/~pjn/ntray.html)
/// especially the "CTrayIcon::OnTrayNotification" member function.
///
/// This class is a light wrapper around the windows system tray stuff. It
/// adds an icon to the system tray with the specified ToolTip text and
/// callback notification value, which is sent back to the Parent window.
///
/// The tray icon can be instantiated using either the constructor or by
/// declaring the object and creating (and displaying) it later on in the
/// program. eg.
/// \code
///    TTrayIcon* Trayicon;  // Member variable of some class
///
///    ...
///    // in some member function maybe...
///    Trayicon = new TTrayIcon(... ... ... ... );
///    ...
///    //in SetupWindow()  or after where hWnd is valid
///    Trayicon->Create();
/// \endcode
///  or
/// \code
///    TTrayIcon Trayicon;  // Member variable of some class
///  ...
///    // in some member function maybe...
///    Trayicon.Init(........);
///  ...
///    //in SetupWindow()  or after where hWnd is valid
///    Trayicon.Create();
/// \endcode
//
// Clobbered together by Chris Maunder.
//
// Rewrited for OWL by Bidus Yura;
//
////////////////////////////////////////////////////////////////////////////////
#include <owl/pch.h>
#include <owl/shellitm.h>

#include <owl/trayicon.h>
#include <owl/window.h>

namespace owl {

//
// TTrayIcon construction/creation/destruction
//
TTrayIcon::TTrayIcon(TModule* module)
:
  Module(module),
  Icon(0),
  ShouldDelete(false)
{
   tstring tos(_T(""));
   Init(0, 0, tos, 0, 0);
}

//
//
//
TTrayIcon::TTrayIcon(TWindow* parent, TMsgId msg, const tstring& tip, TIcon& icon,
                     uint id, TModule* module)
:
  Module(module),
  Icon(0),
  ShouldDelete(false)
{
  Init(parent, msg, tip, &icon, id);
  bHidden = false;
}
//
TTrayIcon::TTrayIcon(TWindow* parent, TMsgId msg, const tstring& tip, 
                     TResId icon, uint id, TModule* module)
:
  Module(module),
  Icon(0),
  ShouldDelete(false)
{

  Init(parent,msg,tip,new TIcon(Module?(HINSTANCE)*Module:HINSTANCE(0),icon),id);
  ShouldDelete = true;
  bHidden = false;
}
//
//
//
//
void TTrayIcon::Init(TWindow* parent, TMsgId msg, const tstring& tip, 
                     TIcon* icon, uint id)
{
  //Make sure we avoid conflict with other messages
  bEnabled         = false;
  bHidden          = false;
  Parent           = parent;

  if(Icon && ShouldDelete)
    delete Icon;
  Icon             = icon;

  cbSize           = sizeof(NOTIFYICONDATA);
  hWnd             = 0;
  uFlags            = 0;//NIF_MESSAGE|NIF_ICON|NIF_TIP;
  uID              = id;
  uCallbackMessage = 0;
  if(msg){
    PRECONDITION(msg >= WM_USER);
    uCallbackMessage = msg;
    uFlags            |= NIF_MESSAGE;
  }
  hIcon             = 0;
  if(Icon){
    hIcon             = *Icon;
    uFlags            |= NIF_ICON;
  }
  memset(szTip, 0, sizeof(szTip));
  if(::_tcslen(tip.c_str())){
    if(::_tcslen(tip.c_str()) > 64){
      _tcsncpy(szTip, tip.c_str(), 64);
      szTip[64] = 0;
    }
    else
      ::_tcscpy(szTip, tip.c_str());
    uFlags   |= NIF_TIP;
  }
  CHECK(uFlags);
}

//
//
//
bool TTrayIcon::Create()
{
  //Make sure Notification window is valid
  if(!Parent->GetHandle() || !Parent->IsWindow())
    return false;

  hWnd = *Parent;

  // Set the tray icon
  if(!TShell::Shell_NotifyIcon(NIM_ADD, this))
    return false;;
  bEnabled = true;
  return true;
}

//
//
//
TTrayIcon::~TTrayIcon()
{
  RemoveIcon();
  if(ShouldDelete)
    delete Icon;
}

//
// CTrayIcon icon manipulation
//
void TTrayIcon::MoveToRight()
{
  HideIcon();
  ShowIcon();
}

//
//
//
void TTrayIcon::RemoveIcon()
{
  if(!bEnabled)
    return;

  uFlags = 0;
  TShell::Shell_NotifyIcon(NIM_DELETE, this);
  bEnabled = false;
}

//
//
//
void TTrayIcon::HideIcon()
{
  if(bEnabled && !bHidden) {
    uFlags &= ~NIF_ICON;
    TShell::Shell_NotifyIcon (NIM_DELETE, this);
    bHidden = true;
  }
}

//
//
tstring TTrayIcon::GetText() const
{
  return tstring(szTip);
}

//
//
//
void TTrayIcon::ShowIcon()
{
  if (bEnabled && bHidden){
    uFlags = 0;
    if(uCallbackMessage)
      uFlags |= NIF_MESSAGE;
    if(Icon)
      uFlags |= NIF_ICON;
    if(::_tcslen(szTip))
      uFlags |= NIF_TIP;
    TShell::Shell_NotifyIcon(NIM_ADD, this);
    bHidden = false;
  }
}

//
//
//
bool TTrayIcon::SetIcon(TIcon& icon, TAutoDelete shouldDelete)
{
  if(Icon && ShouldDelete)
    delete Icon;
  ShouldDelete  = shouldDelete == AutoDelete;
  Icon = &icon;

  if(!bEnabled)
    return true;

  uFlags |= NIF_ICON;
  hIcon = *Icon;

  return TShell::Shell_NotifyIcon(NIM_MODIFY, this) == TRUE;
}

//
// TTrayIcon tooltip text manipulation
//
bool TTrayIcon::SetText(const tstring& text)
{
  if(_tcscmp(text.c_str(),szTip)!= 0){
    uFlags |= NIF_TIP;
    if(::_tcslen(text.c_str()) > 64){
      _tcsncpy(szTip, text.c_str(), 64);
      szTip[64] = 0;
    }
    else if(::_tcslen(text.c_str()))
      ::_tcscpy(szTip, text.c_str());
    else{
      szTip[0] = 0;
      uFlags &= ~NIF_TIP;
    }
    if (!bEnabled)
      return true;
    return TShell::Shell_NotifyIcon(NIM_MODIFY, this);
  }
  return true;
}

//
// TTrayIcon notification window stuff
//
bool TTrayIcon::SetParent(TWindow* window)
{
  Parent = window;

  if (!bEnabled)
    return true;

  //Make sure Notification window is valid
  if(!Parent->GetHandle() || !Parent->IsWindow())
    return true;

  hWnd = *Parent;

  return TShell::Shell_NotifyIcon(NIM_MODIFY, this);
}

//
// TTrayIcon implentation of EvTrayNotification
// call this functionon
TResult TTrayIcon::EvTrayNotification(TParam1 id, TParam2 event)
{
  //Return quickly if its not for this tray icon
  if (uID != id)
    return 0L;

  // Clicking with right button brings up a context menu
  if (LOWORD(event) == WM_RBUTTONUP){
    TMenu menu(*Module, uID);
    if(!menu.IsOK())
      return 0;
    //Display and track the popup menu
    TPoint pos;
    GetCursorPos(&pos);
    ::SetForegroundWindow(*Parent);
    TPopupMenu popup(menu);
    // Make first menu item the default (bold font)
    popup.SetDefaultItem(0, true);
    popup.TrackPopupMenu(TPM_LEFTBUTTON, pos, 0, *Parent);
  }
  else if (LOWORD(event) == WM_LBUTTONDBLCLK){
    TMenu menu(*Module, uID);
    if(!menu.IsOK())
      return 0;
    // double click received, the default action is to execute first menu item
    ::SetForegroundWindow(*Parent);
    Parent->SendMessage(WM_COMMAND, menu.GetMenuItemID(0), 0);
  }
  return 1;
}

} // OWL namespace
/* ========================================================================== */


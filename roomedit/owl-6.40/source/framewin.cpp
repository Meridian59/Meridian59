//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TFrameWindow, a TWindow with additional features
/// for frames, such as client window, menus, icons, etc.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <stdio.h>

#include <owl/docking.h>
#include <owl/framewin.h>
#include <owl/applicat.h>
#include <owl/menu.h>
#include <owl/uimetric.h>
#include <owl/bardescr.h>

using namespace std;

namespace owl {

#if !defined(WM_SETICON)
# define WM_SETICON 0x0080
#endif

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCmd);


DEFINE_RESPONSE_TABLE1(TFrameWindow, TWindow)
  EV_WM_PAINT,
  EV_WM_ERASEBKGND,
  EV_WM_QUERYDRAGICON,
  EV_WM_INITMENUPOPUP,
  EV_WM_SETFOCUS,
  EV_WM_SIZE,
  EV_WM_PARENTNOTIFY,
  EV_WM_QUERYNEWPALETTE,
  EV_WM_PALETTECHANGED,
END_RESPONSE_TABLE;

//
/// Overrides TCommandEnable::Enable. Enables or disables the menu options that
/// control the appearance of the corresponding menu item.
//
void
TMenuItemEnabler::Enable(bool enable)
{
  TCommandEnabler::Enable(enable);
  ::EnableMenuItem(HMenu, Position,
                   MF_BYPOSITION | (enable ? MF_ENABLED : MF_GRAYED));
}

//
/// Overrides TCommandEnable::SetText. Changes the text of the corresponding menu
//
void
TMenuItemEnabler::SetText(LPCTSTR str)
{
  ::ModifyMenu(HMenu, Position, MF_BYPOSITION | MF_STRING, Id, str);
}

//
/// Overrides TCommandEnable::SetCheck. Checks or unchecks the corresponding menu
/// item. The state parameter reflects the menu item's state, which can be checked,
/// unchecked, or indeterminate. 
//
void
TMenuItemEnabler::SetCheck(int state)
{
  ::CheckMenuItem(HMenu, Position,
                  MF_BYPOSITION | (state == Checked ? MF_CHECKED : MF_UNCHECKED));
}

//----------------------------------------------------------------------------

//
/// Constructs a window object with the parent window supplied in parent, which is
/// zero if this is the main window. title, which by default is zero, contains the
/// title displayed in the window's caption bar. clientWnd is the client window for
/// this frame window or zero if none exists. shrinkToClient controls whether the
/// client window will size to fit the frame or the frame window will fit the
/// client. Note that this parameter only affects the size of the main window. When
/// a client window is used in a frame window that doesn't have shrinktoClient set,
/// the client window resizes to fit the frame window. When a client window is used
/// in a frame window that has the shrinktoClient set, the frame window shrinks to
/// fit the size of the client window.
//
TFrameWindow::TFrameWindow(TWindow*    parent,
                           LPCTSTR    title,
                           TWindow*   clientWnd,
                           bool       shrinkToClient,
                           TModule*   module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, title, module);


  IconResId = 0;  // remember that we still need to init
  Init(clientWnd, shrinkToClient);
}

//
/// String-aware overload
//
TFrameWindow::TFrameWindow(
  TWindow* parent,
  const tstring& title,
  TWindow* client,
  bool shrinkToClient,
  TModule* module)
{
  TWindow::Init(parent, title, module);
  IconResId = 0;
  Init(client, shrinkToClient);
}

//
/// Constructor for a TFrameWindow that is being used as an alias for a
/// non-ObjectWindows window. hWnd is the handle to the existing window object that
/// TFrameWindow controls; module contains the module passed to the base class's
/// contructor.
///
/// This constructor is generally not used by derived
/// classes, only as generic alias to a framewindow-like HWND
//
TFrameWindow::TFrameWindow(HWND THandle, TModule* module)
:
  TWindow(THandle, module)
{
  Init(0);
}

//
/// Protected constructor for use by immediate virtually derived classes.
/// Immediate derivitives must call Init() before constructions are done.
//
TFrameWindow::TFrameWindow()
{
  IconResId = 0;  // Zero this member to remember that we still need to init
}

//
/// Normal initialization of a default constructed TFrameWindow. Is ignored
/// if called more than once.
///
/// This initialize function is for use with virtually derived classes, which must
/// call Init before construction is completed. This procedure provides necessary
/// data to virtually derived classes and takes care of providing the data in the
/// appropriate sequence.
//
void
TFrameWindow::Init(TWindow* clientWnd, bool shrinkToClient)
{
  if (!IconResId) {
    Attr.Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    Attr.X = Attr.W = CW_USEDEFAULT;

    if (clientWnd)
      Attr.Style |= WS_CLIPCHILDREN;

    if (shrinkToClient)
      SetFlag(wfShrinkToClient);

    Init(clientWnd);
  }
}

//
// Private initializer does a bulk of the common frame window initialization
//
void
TFrameWindow::Init(TWindow* clientWnd)
{
  HWndRestoreFocus = 0;
  KeyboardHandling = false;
  ClientWnd        = clientWnd;

  MenuDescr    = 0;
  BarDescr     = 0;
  DeleteBar    = false;
  MergeModule  = 0;

  CurIcon      = 0;
  CurIconSm    = 0;
  IconModule   = 0;
  IconSmModule = 0;
  SetIcon(&GetGlobalModule(), IDI_OWLAPP);
  #if !defined( MAINWIN )
  SetIconSm(&GetGlobalModule(), IDI_OWLAPPSM);
  #endif

  MinimizedPos = TPoint(-1,-1);  // Windows convention for never minimized

  if (ClientWnd) {
    ClientWnd->SetParent(this);
    ClientWnd->EnableAutoCreate();  // in case client is a dialog
    SetBkgndColor(NoErase);         // no need to erase client area
  }
}

//
/// Destructor for a TFrameWindow
//
/// Deletes any associated menu descriptor.
//
TFrameWindow::~TFrameWindow()
{
  if (IsFlagSet(wfMainWindow))
    if (GetApplication()->GetMainWindow() == this)
      GetApplication()->ClearMainWindow();

  delete MenuDescr;
  if(DeleteBar)
    delete BarDescr;
}

//
// Return a state mask representing the enabled menu items (up to 32)
//
static uint32
GetMenuStateBits(HMENU hmenu, int count)
{
  uint32 bit = 1;
  uint32 result = 0;

  for (int i = 0; i < count; i++) {
    uint state = GetMenuState(hmenu, i, MF_BYPOSITION);
    if (state != (uint)-1) {
      if (!(state  & (MF_DISABLED | MF_GRAYED))) {
        result |= bit;
      }
    }
    bit <<= 1;
  }

  return result;
}

//
/// Responds to WM_INITMENUPOPUP by performing a command enable run on each
/// of the menu items in the popup menu
///
/// Sent before a pop-up menu is displayed, EvInitMenuPopup lets an application
/// change the items on the menu before the menu is displayed. EvInitMenuPopup
/// controls whether the items on the pop-up menu are enabled or disabled, checked
/// or unchecked, or strings. HMENU indicates the menu handle. index is the index of
/// the pop-up menu. sysMenu indicates if the pop-up menu is the system menu.
//
void
TFrameWindow::EvInitMenuPopup(HMENU hPopupMenu, uint index, bool sysMenu)
{
  if (!sysMenu && hPopupMenu) {
    const int count = ::GetMenuItemCount(hPopupMenu);

    // Save current state of visible top level menus
    //
    uint32 preState = 0;
    if (hPopupMenu == GetMenu())
      preState = GetMenuStateBits(hPopupMenu, count);

    TWindow::EvInitMenuPopup(hPopupMenu, index, sysMenu);

    // If the top level menu state changes, redraw the menu bar
    //
    if (hPopupMenu == GetMenu())
      if (GetMenuStateBits(hPopupMenu, count) != preState)
        DrawMenuBar();
  }
}

//
/// Overrides TWindow's virtual function. TApplication calls the main window's
/// IdleAction when no messages are waiting to be processed. TFrameWindow uses this
/// idle time to perform command enabling for the menu bar. It also forwards
/// IdleAction to each of its children. IdleAction can be overridden to do
/// background processing.
//
bool
TFrameWindow::IdleAction(long idleCount)
{
  if (idleCount == 0) {
    // do command enabling for the menu bar if this is the active task
    //
    if (GetFocus()) {
      long style = ::GetWindowLong(*this, GWL_STYLE);
      if (!(style & WS_CHILD)) {
        if (IsWindow()) {
          HMENU hMenu = ::GetMenu(*this);
          if (IsMenu(hMenu))
            HandleMessage(WM_INITMENUPOPUP, TParam1(hMenu));
        }
      }
    }
  }
  // give child windows an opportunity to do any idle processing
  //
  return TWindow::IdleAction(idleCount);
}

//
/// Locates and returns the child window that is the target of the command and
/// command enable messages. If the current application does not have focus or if
/// the focus is within a toolbar in the application, GetCommandTarget returns the
/// most recently active child window.
/// 
/// If an alternative form of command processing is desired, a user's main window
/// class can override this function. TFrameWindow's EvCommand and EvCommandEnable
/// functions use GetCommandTarget to find the command target window. This member is
/// not available under Presentation Manager.
//
HWND
TFrameWindow::GetCommandTarget()
{
  // Retrieve the focus window and our client
  //
  HWND hFocus = ::GetFocus();
  TWindow* client = GetClientWindow();

  // 1. The first candidate is a focus window that's a child of our client
  //
  if (hFocus && client && client->IsChild(hFocus)) {
    TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - focus, "\
                      "child of client: " << hex << uint(hFocus));
    return hFocus;
  }

  // 2. The next option is our client window itself
  //
  if (client) {
    TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - client: " << *client);
    return *client;
  }

  // 3. The next option is a focus window that's a child of ours
  //
  if (hFocus && IsChild(hFocus)) {
    TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - focus, "\
                       << hex << uint(hFocus));
    return hFocus;
  }

  // 4. If all of the above fail, resort to the last focus child of ours
  //
  if (HWndRestoreFocus) {
#if defined(__TRACE) || defined(__WARN)
    TWindow* win = GetWindowPtr(HWndRestoreFocus);
    if (!win) {
      TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - HwndRestoreFocus, "\
                        << hex << uint(HWndRestoreFocus));
    } else {
      TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - HwndRestoreFocus, "\
                        << *win);
    }
#endif
    return HWndRestoreFocus;
  }

  // 5. When all else fails, send ourselves in
  //
  TRACEX(OwlCmd, 1, "TFrameWindow::GetCommandTarget - self, " << *this);
  return *this;
}

//
/// Handle WM_COMMAND to provide extra processing for commands:
/// Extra processing for commands: starts with the command target window
/// (usually the focus window) and gives it and its parent windows an
/// opportunity to handle the command.
//
TResult
TFrameWindow::EvCommand(uint id, HWND hCtl, uint notifyCode)
{
  TRACEX(OwlCmd, 1, "TFrameWindow::EvCommand - id(" << id << "), ctl(" <<\
                     hex << uint(hCtl) << "), code(" << notifyCode  << ")");

  // Walk the command chain from the command target back up to us or until
  // we hit a child that is an owl window. Delegate to owl-child or forward to
  // our base if no child is found.
  //
  if (hCtl == 0) {
    HWND  hCmdTarget = GetCommandTarget();

    // Check owl parentage too in case the HWNDs were reparented
    //
    while (hCmdTarget && hCmdTarget != GetHandle()) {
      TWindow*  cmdTarget = GetWindowPtr(hCmdTarget);

      if (cmdTarget)
        return cmdTarget->EvCommand(id, hCtl, notifyCode);

      hCmdTarget = ::GetParent(hCmdTarget);
    }
  }

  return TWindow::EvCommand(id, hCtl, notifyCode);
}

//
/// Handle WM_COMMAND_ENABLE to provide command enable distribution and default
/// support for windows without command enable handlers.
///
/// Handles checking and unchecking of the frame window's menu items.
/// EvCommandEnable uses TWindow's RouteCommandEnable member function to perform the
/// majority of this command enabling work.
//
void
TFrameWindow::EvCommandEnable(TCommandEnabler& commandEnabler)
{
  // Don't process for windows out of our window tree (esp. other apps)
  //
  RouteCommandEnable(GetCommandTarget(), commandEnabler);
}

//
/// Overrides TWindow's virtual function. Performs preprocessing of window messages.
/// If the child window has requested keyboard navigation, PreProcessMsg handles any
/// accelerator key messages and then processes any other keyboard messages.
//
bool
TFrameWindow::PreProcessMsg(MSG& msg)
{
  if (TWindow::PreProcessMsg(msg))
    return true;  // Processed accelerators

  if (KeyboardHandling && msg.message >= WM_KEYFIRST &&
                          msg.message <= WM_KEYLAST)
  {
    HWND parent = ::GetParent(msg.hwnd);

    // Retrieve the COMBO handle if we're in the EDIT ctrl parented to the
    // combobox
    //
    tchar szClassName[0x10];
    ::GetClassName(parent, szClassName, COUNTOF(szClassName));
    if (!_tcsicmp(szClassName, _T("COMBOBOX")))
      parent = ::GetParent(parent);
    if (parent && ::IsDialogMessage(parent, &msg))
      return true;
  }

  return false;
}

//
/// Overrides TWindow's non-virtual SetMenu function, thus allowing derived classes
/// the opportunity to implement this function differently from TWindow. SetMenu
/// sets the window's menu to the menu indicated by newMenu. If newMenu is 0, the
/// window's current menu is removed. SetMenu returns 0 if the menu remains
/// unchanged; otherwise, it returns a nonzero value.
///
/// It also calls the application's PreProcessMenu() if it is the main window
/// to let it make any changes just before setting.
//
bool
TFrameWindow::SetMenu(HMENU newMenu)
{
  if (IsFlagSet(wfMainWindow))
    GetApplication()->PreProcessMenu(newMenu);

  return TWindow::SetMenu(newMenu);
}

//
/// Perform a high-level menu assignment either before or after the HWND for the
/// window has been created.
///
/// Sets Attr.Menu to the supplied menuResId and frees any previous strings pointed
/// to by Attr.Menu. If HWindow is nonzero, loads and sets the menu of the window,
/// destroying any previously existing menu.
///
/// Returns true if successful; false otherwise
//

bool
TFrameWindow::AssignMenu(TResId menuResId)
{

  if (menuResId != Attr.Menu) {
    if (Attr.Menu.IsString())
      delete[] Attr.Menu.GetString();

    Attr.Menu = menuResId.IsString() ? TResId(strnewdup(menuResId)) : menuResId;
  }

  // If the window has been created then load and set the new menu and destroy
  // the old menu
  //
  if (!GetHandle())
    return true;

  HMENU curMenu = GetMenu();
  HMENU newMenu = LoadMenu(Attr.Menu);

  if (!SetMenu(newMenu))
    return false;

  if (curMenu)
    ::DestroyMenu(curMenu);

  return true;
}

//
/// Sets the icon in the module specified in iconModule to the resource ID specified
/// in iconResId. See the sample file BMPVIEW.CPP for an example of painting an icon
/// from a bitmap. You can set the iconResId to one of these predefined values as
/// well as user-defined values:
/// - \c \b  IDI_APPLICATION	Default icon used for applications
/// - \c \b  IDI_ASTERISK	Asterisk used for an informative message
/// - \c \b  IDI_EXCLAMATION	Exclamation mark used for a warning message
/// - \c \b  IDI_HAND		Hand used for warning messages
/// - \c \b  IDI_QUESTION	Question mark used for prompting a response
//
bool
TFrameWindow::SetIcon(TModule* module, TResId resId)
{
  // Delete old icon if not system icon
  //
  if (CurIcon && IconModule) {
    TUser::DestroyIcon(CurIcon);
    CurIcon = 0;
  }

  IconModule = module;
  IconResId = resId;

  HINSTANCE hInstance = IconModule ? HINSTANCE(*IconModule) : HINSTANCE(0);

  if (IconResId != 0)
    CurIcon = TUser::LoadIcon(hInstance, IconResId);

  if (CurIcon && IsWindow())
    SendMessage(WM_SETICON, true, (LPARAM)(HICON)CurIcon);

  return true;
}

//
/// Set the Small Icon (16 x 16)
//
bool
TFrameWindow::SetIconSm(TModule* module, TResId resId)
{
  // Delete old small icon
  //
  if (CurIconSm && IconSmModule) {
    TUser::DestroyIcon(CurIconSm);
    CurIconSm = 0;
  }

  IconSmModule = module;
  IconSmResId  = resId;

  HINSTANCE hInstance = IconSmModule ? HINSTANCE(*IconSmModule) : HINSTANCE(0);
  if (IconSmResId != 0) {
    CurIconSm = (HICON)::LoadImage(hInstance, IconSmResId, IMAGE_ICON,
                                   TUIMetric::CxSmIcon, TUIMetric::CySmIcon,
                                   LR_DEFAULTCOLOR);
    if (!CurIconSm)
      CurIconSm = TUser::LoadIcon(hInstance, IconSmResId);
  }

  if (CurIconSm && IsWindow())
    SendMessage(WM_SETICON, false, (LPARAM)(HICON)CurIconSm);

  return true;
}

//
/// Returns a pointer to the client window. If you are trying to access a
/// window-based object in a TMDIChild (which is a frame window), you can use this
/// function.
//
TWindow*
TFrameWindow::GetClientWindow()
{
  return ClientWnd;
}

//
/// Sets the client window to the specified window. Users are responsible for
/// destroying the old client window if they want to eliminate it.
///
/// Assume clientWnd was parented to us.
//
TWindow*
TFrameWindow::SetClientWindow(TWindow* clientWnd)
{
  TWindow* oldClientWnd = ClientWnd;
  HWND oldWnd = oldClientWnd ? oldClientWnd->GetHandle() : (HWND)0;
  RemoveChild(ClientWnd);

  if (HWndRestoreFocus == oldWnd)
    HWndRestoreFocus = 0;

  ClientWnd = clientWnd;

  if (ClientWnd) {
    ClientWnd->SetParent(this);
    if (GetHandle()) {
      if (!ClientWnd->GetHandle())
        ClientWnd->Create();
      ClientWnd->ShowWindow(SW_SHOWNOACTIVATE);
    }
    SetBkgndColor(NoErase);         // no need to erase client area
    ResizeClientWindow(true);       // !CQ defer repaint?
  }
  else
    SetBkgndColor(NoColor);         // will need to erase client area

  // Pass the focus to the new client, but only if we have it currently
  //
  if (ClientWnd && ClientWnd->GetHandle() && GetFocus() == GetHandle()) {
    ClientWnd->SetFocus();
    HWndRestoreFocus = ClientWnd->GetHandle();
  }

  return oldClientWnd;
}

//
/// If someone removes our client with a RemoveChild() call, update our client
/// and restore focus ptrs.
//
void
TFrameWindow::RemoveChild(TWindow* child)
{
  TWindow::RemoveChild(child);
  if (child) {
    if (child == ClientWnd)
      ClientWnd = 0;
    if (child->GetHandle() == HWndRestoreFocus) {
      HWndRestoreFocus = 0;
    }
  }
}

//
/// Overrides TWindow's virtual function. Pastes the number of the view into the
/// caption and then shows the number on the screen. This function can be overridden
/// if you don't want to use the default implementation, which displays a number on
/// the screen. That is, you might want to write "Two" instead of ":2" on the
/// screen. For an example of the behavior of this function, see step 12 of the
/// ObjectWindows tutorial, which renumbers the views if one of them is closed.
///
/// Generates a composite title based on the caption, docname, and index
/// if it is > 0.
/// \code
///  [<Title> - ]<docname>[:<index>]
/// \endcode
//
bool
TFrameWindow::SetDocTitle(LPCTSTR docname, int index)
{
  if (index >= 0) {
    tstring title;

    LPCTSTR c = GetCaption();
    if (c && *c) {
      title = c;
      title += _T(" - ");
    }

    if (docname)
      title += docname;

    if (index > 0) {
      title += _T(":");
      tchar num[10];
      _stprintf(num, _T("%d"), index );
      title += num;
    }

    SetWindowText(title.c_str());
  }// else if index negative, simply acknowledge that title will display
  return true;
}

//
// Obtain the real windows application icon. The IDI_APPLICATION icon is an
// ugly black & white box, but when a class is registered with this icon it
// gets substituted with a better windows icon. Worse case we end up with the
// ugly box icon.
//
static HICON
getAppIcon()
{
  static HICON hRealAppIcon = 0;
  if (!hRealAppIcon) {
    WNDCLASS wndClass;
    static tchar className[] = _T("IconSnarfer");
    memset(&wndClass, 0, sizeof wndClass);
    wndClass.hInstance = GetGlobalModule().GetHandle();
    wndClass.hIcon = ::LoadIcon(0, IDI_APPLICATION);
    wndClass.lpszClassName = className;
    wndClass.lpfnWndProc = ::DefWindowProc;
    ::RegisterClass(&wndClass);
    ::GetClassInfo(GetGlobalModule().GetHandle(), className, &wndClass);
    hRealAppIcon = wndClass.hIcon;
    ::UnregisterClass(className, GetGlobalModule().GetHandle());
  }
  return hRealAppIcon ? hRealAppIcon : ::LoadIcon(0, IDI_APPLICATION);
}

//
/// Responds to a WM_PAINT message in the client window in order to paint the iconic
/// window's icon or to allow client windows a change to paint the icon.
///
/// If iconic, and an icon has been defined then draw that.
/// Or, if iconic & there is a client window, then call its paint function.
///
/// If not iconic, forwards to TWindow for normal paint processing
//
void
TFrameWindow::EvPaint()
{
  if (IsIconic() && (IconResId || ClientWnd)) {
    TPaintDC  dc(GetHandle());

    if (IconResId) {
      SendMessage(WM_ICONERASEBKGND, TParam1(HDC(dc)));
      ::DrawIcon(dc, 0, 0, CurIcon ? CurIcon : getAppIcon());
    }
    else
      ClientWnd->Paint(dc, dc.Ps.fErase, *(TRect*)&dc.Ps.rcPaint);
  }
  else
    TWindow::EvPaint();
}

//
/// Response method for an incoming WM_ERASEBKGND message.
/// EvEraseBkgnd erases the background of the window specified in HDC. It returns
/// true if the background is erased; otherwise, it returns false.
///
/// If this frame window is iconic, and there is a client window, then give it
/// a chance to erase the background since it may want to take over painting.
///
/// If not iconic, forward to TWindow for normal erase background processing
//
bool
TFrameWindow::EvEraseBkgnd(HDC hDC)
{
  if (IsIconic()) {
    if (!IconResId && ClientWnd)
      return (bool)ClientWnd->HandleMessage(WM_ERASEBKGND, TParam1(hDC));

    HandleMessage(WM_ICONERASEBKGND, TParam1(hDC));
    return true;
  }
  else
    return TWindow::EvEraseBkgnd(hDC);
}

//
/// Responds to a WM_QUERYDRAGICON message sent to a minimized (iconic) window that
/// is going to be dragged. Instead of the default icon, EvQueryDragIcon uses the
/// icon that was set using SetIcon.
/// This member is not available under Presentation Manager.
//
HANDLE
TFrameWindow::EvQueryDragIcon()
{
  // !JK Consider the following problems:
  // !JK (1) If a derived class sets CurIcon (instantiated with something other
  // !JK     than a module & res id), this function will ignore it!  Why do a
  // !JK     ::LoadIcon again?  It won't actually load again anyway (see MS
  // !JK     Win16/Win32 doc).  It doesn't reference count either.
  // !JK
  // !JK (2) If IconResId is non-zero but bad (i.e., ::LoadIcon fails), getAppIcon()
  // !JK     is returned; but if IconResId is zero, TWindow::EvQueryDragIcon() is
  // !JK     returned.  What is the rationale behind returning one icon in the case
  // !JK     of a bad res id and a different icon in the case of a zero res id?
  // !JK
  // !JK This function body should be:
  // !JK   return (CurIcon)? CurIcon: TWindow::EvQueryDragIcon();
  // !JK   -or-
  // !JK   return (CurIcon)? CurIcon: getAppIcon();
  if (IconResId) {
    HINSTANCE hInstance = IconModule ? HINSTANCE(*IconModule) : HINSTANCE(0);
    HICON hIcon = TUser::LoadIcon(hInstance, IconResId);
    return hIcon ? hIcon : getAppIcon();
    // !CQ This LoadIcon() may be causing a resource leak. May need to keep icon
    // !CQ We are keeping it!!! CurIcon!!!
  }
  else
    return TWindow::EvQueryDragIcon();
}

static inline bool
IsEnabledVisibleChild(long style)
{
  return (style & (WS_CHILD | WS_VISIBLE | WS_DISABLED)) == (WS_CHILD | WS_VISIBLE);
}

static TWindow*
SearchForChildWithTab(TWindow* win)
{
  TWindow*  firstChild = win->GetFirstChild();

  if (firstChild) {
    TWindow*  child = firstChild;

    do {
      if (child->GetHandle()) {
        long  style = child->GetWindowLong(GWL_STYLE);

        if (IsEnabledVisibleChild(style)) {
          if (style & WS_TABSTOP)
            return child;

          else {
            TWindow*  result = SearchForChildWithTab(child);
            if (result)
              return result;
          }
        }
      }
      child = child->Next();
    } while (child != firstChild);
  }

  return 0;
}

static bool
EnabledVisibleChild(TWindow* win, void*)
{
  return win->GetHandle() ? IsEnabledVisibleChild(win->GetWindowLong(GWL_STYLE)) :
                        false;
}

//
// If the receiver doesn't have any children then returns 0. Otherwise
// we search for the first child with WS_TABSTOP; If no child has WS_TABSTOP
// then we return the first enabled visible child
//
// Does a depth-first search of nested child windows
//
// NOTE: we stop at the first child with WS_TABSTOP and do not search its
//       children...
//
TWindow*
TFrameWindow::FirstChildWithTab()
{
  TWindow*  win = SearchForChildWithTab(this);

  return win ? win : FirstThat(EnabledVisibleChild);
}

//
/// Overrides TWindow's virtual function. Responds to a request by a child window to
/// hold its HWND when it is losing focus. Stores the child's HWND in
/// HwndRestoreFocus.
///
/// return true if caller can stop searching for a window to hold its handle.
//
bool
TFrameWindow::HoldFocusHWnd(HWND hWndLose, HWND hWndGain)
{
  if (IsChild(hWndLose)) {
    if (!hWndGain || !IsChild(hWndGain))
      HWndRestoreFocus = hWndLose;
    return true;
  }
  return hWndLose == GetHandle();
}

//
/// Restores the focus to the active window. hWndLostFocus contains the handle for
/// the window that lost focus.
///
/// Handle WM_SETFOCUS to return focus back to the child that had it most
/// recently, or find the best one to give it to otherwise.
//
void
TFrameWindow::EvSetFocus(HWND hWndLostFocus)
{
  TWindow::EvSetFocus(hWndLostFocus);

  if (!HWndRestoreFocus) {
    HWND cmdTgt = GetCommandTarget();
    if (cmdTgt && IsChild(cmdTgt))
      HWndRestoreFocus = cmdTgt;
  }

//  if (HWndRestoreFocus == hWndLostFocus)
//    HWndRestoreFocus = GetHandle();

  if (HWndRestoreFocus) {
    // Set focus to the saved HWND as long as it is still a valid window handle
    //
    if (::IsWindow(HWndRestoreFocus))
      ::SetFocus(HWndRestoreFocus);
    else
      HWndRestoreFocus = 0;
  }
}

//
/// Responds to a message to notify the parent window that a given event has
/// occurred. If the client window is destroyed, closes the parent window. If
/// shrinkToClient is set and the child window has changed size, the frame is
/// adjusted.
/// When a TFrameWindow's client window is destroyed, the TFrameWindow object sees
/// the WM_PARENTNOTIFY message and posts a close message to itself. Without this
/// message, an empty frame would remain and the client window would then have to
/// determine how to destroy the frame. If you don't want this to happen, you can
/// derive from the frame window and have your application handle the EvParentNotify
/// or EvClose messages.
//
void
TFrameWindow::EvParentNotify(uint event, TParam1, TParam2 p2)

{
  if (event == WM_DESTROY) 
  {
    HWND child = reinterpret_cast<HWND>(p2);
    if (ClientWnd && ClientWnd->GetHandle() == child)
      PostMessage(WM_CLOSE);  // using ShutDownWindow() has side effects

    TWindow* c = GetWindowPtr(child);
    if (c)
      c->ClearFlag(wfFullyCreated);
  }
  else if (event == WM_SIZE) 
  {
    HWND child = reinterpret_cast<HWND>(p2);
    if (IsFlagSet(wfShrinkToClient)
        && ClientWnd
        && ClientWnd->GetHandle() == child
        && !IsIconic())
      ResizeClientWindow(true); // !CQ defer repaint?
  }
  DefaultProcessing();
}

//
/// Forwards the WM_QUERYNEWPALETTE message to the client window.
//
bool
TFrameWindow::EvQueryNewPalette()
{
  if (GetClientWindow())
    return GetClientWindow()->ForwardMessage();
  else
    return TWindow::EvQueryNewPalette();
}

//
/// Forwards the WM_PALETTECHANGED message to the client window.
//
void
TFrameWindow::EvPaletteChanged(HWND hWndPalChg)
{
  if (GetClientWindow())
    GetClientWindow()->ForwardMessage();
  else
    TWindow::EvPaletteChanged(hWndPalChg);
}

//
// Resize & reposition the client window to fit in this frames client area
// or resize the frame to fit around the client's client area if
// wfShrinkToClient
// Return true if a client was actualy resized.
// Adjust clients styles & make sure they get set.
//
bool
TFrameWindow::ResizeClientWindow(bool repaint)
{
///  bool hasThickFrame = Attr.Style & WS_THICKFRAME;
///  Attr.Style |= WS_THICKFRAME;

  // Nothing to resize if there's not Client window
  //
  if (!ClientWnd)
    return false;

  // Prevent recursion during resize by ignore calls from EvParentNotify and
  // EvSize when we have already been called.
  // Do this by disabling notifications while resizing using the
  // wfShrinkToClient flag as a semaphore on the client
  //
  if (ClientWnd->IsFlagSet(wfShrinkToClient))
    return true;
  ClientWnd->SetFlag(wfShrinkToClient);

  bool  clientResized = false;
  TSize clientAreaSize = GetClientRect().Size();
  TSize childSize = ClientWnd->GetWindowRect().Size();

  // First time through, strip client window of thick borders.
  // If shrink-to-client, then must measure the client size first
  // If the client has scrolls bars, we must hide them to obtain the correct
  // size.
  // Border style is left on & dealt with by hand below
  //
  const uint32 badClientStyles = WS_DLGFRAME | WS_THICKFRAME | // bad borders
                                 WS_POPUP | WS_OVERLAPPED;     // bad parenting
  const uint32 badClientExStyles = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                                   WS_EX_STATICEDGE;      // more bad borders

  if ((ClientWnd->GetStyle() & badClientStyles) ||
      (ClientWnd->GetExStyle() & badClientExStyles)) {
    if (IsFlagSet(wfShrinkToClient)) {
      TSize tstSize = ClientWnd->GetClientRect().Size();
      ClientWnd->ShowScrollBar(SB_BOTH, false);
      childSize = ClientWnd->GetClientRect().Size();
      if (childSize != tstSize) {
        int restore = SB_BOTH;
        if (childSize.cx == tstSize.cx)
          restore = SB_HORZ;
        if (childSize.cy == tstSize.cy)
          restore = SB_VERT;
        ClientWnd->ShowScrollBar(restore, true);
      }
    }
    if (ClientWnd->GetStyle() & badClientStyles) {
      bool reparent = (ClientWnd->GetStyle() & (WS_POPUP|WS_OVERLAPPED)) != 0;
      uint32 style = ClientWnd->GetStyle();
      style &= ~badClientStyles;
      style |= WS_CHILD | WS_BORDER | WS_VISIBLE;
      ClientWnd->SetStyle( style );
      if (reparent)
        ::SetParent(*ClientWnd, *this);
    }
    if (ClientWnd->GetExStyle() & badClientExStyles) {
      uint32 exStyle = ClientWnd->GetExStyle();
      exStyle &= ~badClientExStyles;
      ClientWnd->SetExStyle( exStyle );
    }
  }
  if (ClientWnd->GetStyle() & WS_BORDER) {
    childSize = ClientWnd->GetClientRect().Size();
  }
  if (childSize != clientAreaSize) {
    if (IsFlagSet(wfShrinkToClient)) {
      TRect outside = GetWindowRect();
      TSize border = outside.Size() - clientAreaSize;
      SetWindowPos(0, 0, 0,
                   childSize.cx + border.cx, childSize.cy + border.cy,
                   SWP_NOACTIVATE|SWP_NOMOVE| (repaint ? 0 : SWP_NOREDRAW));
      clientAreaSize = childSize; // Must move client, will not cause an EvSize
    }
    else {
      clientResized = true;       // Client will get resized
    }
  }
  // If frame is sizeable, turn off flag so that user can then resize
  // after initial setup
  //
  if (Attr.Style & WS_THICKFRAME && !TYPESAFE_DOWNCAST(this, TFloatingSlip))
    ClearFlag(wfShrinkToClient);


///  if (!hasThickFrame) {
///    Attr.Style &= ~WS_THICKFRAME;
///  }

  // Handle simple border style by shoving the client's borders under the frame
  // This code MUST not resize the client if shrinkToClient
  // !CQ use SetWindowPos() to get at SWP_NOSIZE?
  //
  if (ClientWnd->GetStyle() & WS_BORDER) {
    int bx = TUIMetric::CxBorder;
    int by = TUIMetric::CyBorder;
    ClientWnd->MoveWindow(-bx, -by, clientAreaSize.cx+bx+bx,
                                    clientAreaSize.cy+by+by, repaint);
  }
  else
    ClientWnd->MoveWindow(0, 0, clientAreaSize.cx, clientAreaSize.cy, repaint);

  // Turn off semaphore bit
  //
  ClientWnd->ClearFlag(wfShrinkToClient);
  return clientResized;
}

//
/// Calls TWindow::SetUpWindow to create windows in a child list. SetupWindow
/// performs the initial adjustment of the client window if one exists, assigns the
/// frame's menu based on the menu descriptor, and initializes HwndRestoreFocus.
//
void
TFrameWindow::SetupWindow()
{
  // Create windows in child list (this includes the client window)
  //
  TWindow::SetupWindow();

  ResizeClientWindow(true); // !CQ defer repaint?

  if (MinimizedPos != TPoint(-1,-1)) {
    WINDOWPLACEMENT windata;
    windata.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&windata);
    windata.flags = WPF_SETMINPOSITION;
    windata.showCmd = SW_SHOWNA;
    windata.ptMinPosition = MinimizedPos;
    SetWindowPlacement(&windata);
  }

  // If SetMenuDescr() was called before window created, update the menu now
  //
  if (IsFlagSet(wfMainWindow) && MenuDescr) {
    HMENU curMenu = GetMenu();
    TMenu newMenu(*MenuDescr, NoAutoDelete);
    if (SetMenu(newMenu)) {
      if (curMenu)
        ::DestroyMenu(curMenu);
    }
    else
      ::DestroyMenu(newMenu);
  }

  // If we haven't set THandleRestoreFocus then pick the first child with tabstop
  //
  if (!HWndRestoreFocus) {
///TH Previous version would be to search for first child with tabstop.
///TH Why not use CommandTarget?
// !BB Because it breaks when GetCommandTarget returns a non-child window
// !BB Please, leave this code AS IS?
#if 1
    TWindow*  win = FirstChildWithTab();
    HWndRestoreFocus = win ? win->GetHandle() : GetHandle();
#else
    HWND cmdTgt = GetCommandTarget();
    if (cmdTgt && IsChild(cmdTgt))
      HWndRestoreFocus = cmdTgt;
#endif
  }

  if (CurIcon)
    SendMessage(WM_SETICON, true, (LPARAM)(HICON)CurIcon);

  if (CurIconSm)
    SendMessage(WM_SETICON, false, (LPARAM)(HICON)CurIconSm);
}

//
/// Cleans up any associated icons.
//
void
TFrameWindow::CleanupWindow()
{
  // icon cleanup
  //
  SetIcon(0, 0);
  SetIconSm(0, 0);

  TWindow::CleanupWindow();
}

//
/// Tell child windows frame has minimized/maximized/restored
/// (They may want to change enabled state or release/restore resources)
//
void
TFrameWindow::BroadcastResizeToChildren(uint sizeType, const TSize& size)
{
  if (sizeType == SIZE_MINIMIZED
     || sizeType == SIZE_MAXIMIZED
     || sizeType == SIZE_RESTORED)
  {
    ChildBroadcastMessage(WM_OWLFRAMESIZE, sizeType, reinterpret_cast<TParam2>(&size));
  }
}

//
/// Response method for an incoming WM_SIZE message
///
/// Resizes the client window' so that it is equivalent to the client rectangle's
/// size. Calls TWindow::EvSize() in response to an incoming WM_SIZE message.
///
/// If no WM_SIZE sent, forwards WM_SIZE message to client so it can recalc.
//
void
TFrameWindow::EvSize(uint sizeType, const TSize& size)
{
  TWindow::EvSize(sizeType, size);
  TSize newSize = size;

  if (ClientWnd) {
    bool sizeSent = false;
    if (sizeType != SIZE_MINIMIZED) {
      sizeSent = ResizeClientWindow(true); // !CQ defer repaint?
      newSize = ClientWnd->GetClientRect().Size();
    }
    if (!sizeSent)
      ClientWnd->ForwardMessage();
  }

  BroadcastResizeToChildren(sizeType, newSize);
}


//
/// Sets the menu descriptor to the new menu descriptor.
//
void
TFrameWindow::SetMenuDescr(const TMenuDescr& menuDescr)
{
  delete MenuDescr;
  MenuDescr = new TMenuDescr(menuDescr);

  if (IsFlagSet(wfMainWindow) && GetHandle()) {
    HMENU curMenu = GetMenu();
    TMenu newMenu(*MenuDescr, NoAutoDelete);
    if (SetMenu(newMenu))
      ::DestroyMenu(curMenu);
    else
      ::DestroyMenu(newMenu);
  }
}

//
/// Sets the control bar descriptor to the new bar descriptor.
//
void
TFrameWindow::SetBarDescr(TBarDescr* barDescr, TAutoDelete delonClose)
{
  if(DeleteBar)
    delete BarDescr;
  BarDescr = barDescr;
  DeleteBar = delonClose == AutoDelete;
  if (IsFlagSet(wfMainWindow) && GetHandle())
    RestoreBar();
}

//
/// Merges the given menu descriptor with this frame's own menu descriptor and
/// displays the resulting menu in this frame. See TMenuDescr for a description of
/// menu bar types that can be merged.
///
/// Optionally use an existing HMENU to merge into & set
//
bool
TFrameWindow::MergeMenu(const TMenuDescr& childMenuDescr)
{
  if (!MenuDescr || !GetHandle())
    return false;

  MergeModule = childMenuDescr.GetModule();
  TMenu  curMenu(*this, NoAutoDelete);
  TMenu  newMenu(NoAutoDelete);

  MenuDescr->Merge(childMenuDescr, newMenu);

  if (IsFlagSet(wfMainWindow))
    GetApplication()->PreProcessMenu(newMenu);

  if (SetMenu(newMenu)) {
    ::DestroyMenu(curMenu);
    return true;

  }
  else {
    ::DestroyMenu(newMenu);
    return false;
  }

}

//
// Restores this frame window's menu to the one described by our menu
// descriptor
//
bool
TFrameWindow::RestoreMenu()
{
  if (!MenuDescr)
    return false;

  HMENU curMenu = GetMenu();
  TMenu newMenu(*MenuDescr, NoAutoDelete);
  if (SetMenu(newMenu)) {
    MergeModule = 0;
    ::DestroyMenu(curMenu);
  }
  else
    ::DestroyMenu(newMenu);
  return true;
}



IMPLEMENT_STREAMABLE1(TFrameWindow, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads data of the uninitialized TFrameWindow from the passed ipstream
//

void*
TFrameWindow::Streamer::Read(ipstream& is, uint32 version) const
{
  TFrameWindow* o = GetObject();
  ReadVirtualBase((TWindow*)o, is);
  if (o->IsFlagSet(wfMainWindow))
    return o;

  is >> o->ClientWnd;
  is >> o->KeyboardHandling;
  o->HWndRestoreFocus = 0;

  bool hasMenuDescr = is.readByte();
  if (hasMenuDescr) {
    o->MenuDescr = new TMenuDescr;
    is >> *o->MenuDescr;
  }
  else
    o->MenuDescr = 0;

  o->BarDescr = 0;
  if(version > 2){
    bool hasBarDescr = is.readByte();
    if (hasBarDescr) {
      o->BarDescr = new TBarDescr;
      is >> *o->BarDescr;
    }
  }

  // stream in window icon information
  //
  is >> o->IconModule;
  is >> o->IconResId;
  if (version > 1) {
    is >> o->IconSmModule;
    is >> o->IconSmResId;
  }
  else {
    o->IconSmModule = 0;
    o->IconSmResId = 0;
  }

  // load the window's icons
  //
  o->CurIcon   = 0;
  o->CurIconSm = 0;
  o->SetIcon(o->IconModule, o->IconResId);
  o->SetIconSm(o->IconSmModule, o->IconSmResId);

  is >> o->MergeModule;
  is >> o->MinimizedPos;

  return o;
}

//
// Writes data of the TFrameWindow to the passed opstream
//
void
TFrameWindow::Streamer::Write(opstream& os) const
{
  TFrameWindow* o = GetObject();
  WriteVirtualBase((TWindow*)o, os);
  if (o->IsFlagSet(wfMainWindow))
    return;

  os << o->ClientWnd;
  os << o->KeyboardHandling;

  os.writeByte(uint8(o->MenuDescr ? 1 : 0));
  if (o->MenuDescr)
    os << *o->MenuDescr;

  // added in stream ver 3
  os.writeByte(uint8(o->BarDescr ? 1 : 0));
  if (o->BarDescr)
    os << *o->BarDescr;

  os << o->IconModule;
  os << o->IconResId;
  os << o->IconSmModule;      // added in stream ver 2
  os << o->IconSmResId;       // added in stream ver 2

  os << o->MergeModule;
  WINDOWPLACEMENT windata;
  windata.length = sizeof(WINDOWPLACEMENT);
  o->GetWindowPlacement(&windata);
  os << TPoint(windata.ptMinPosition);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace


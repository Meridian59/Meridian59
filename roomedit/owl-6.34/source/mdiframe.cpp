//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMDIFrame.  This defines the basic behavior of
/// all MDI frame windows.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/mdi.h>
#include <owl/applicat.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlWin);
DIAG_DECLARE_GROUP(OwlCmd);


DEFINE_RESPONSE_TABLE1(TMDIFrame, TFrameWindow)
END_RESPONSE_TABLE;

//
/// Constructs an MDI frame window object using the caption (title) and resource ID
/// (menuResId). If no client window is specified (clientWnd), then an instance of
/// TMDIClient is created automatically and used as the client window of the frame.
/// The supplied library ID (module) is passed to the TFrameWindow constructor along
/// with a null parent window pointer, caption, client window, and a flag indicating
/// that the client window is not to be resized to fit. The TWindow constructor is
/// also invoked; it passes the supplied caption and library ID, as well as a null
/// parent window pointer. Then the child menu position is initialized to be the
/// leftmost menu item, and the supplied menu resource ID is used in a call to
/// TFrameWindow::AssignMenu.
//
TMDIFrame::TMDIFrame(LPCTSTR         title,
                     TResId          menuResId,
                     TMDIClient&     clientWnd,
                     TModule*        module)
{
  // Initialize virtual bases, in case the derived-most used default ctor
  //
  TWindow::Init(0, title, module);
  TFrameWindow::Init(&clientWnd, false);

  if (menuResId)
    AssignMenu(menuResId);
}

//
/// String-aware overload
//
TMDIFrame::TMDIFrame(
  const tstring& title,
  TResId menuResId,
  TMDIClient& client,
  TModule* module)
{
  // Initialize virtual bases, in case the derived-most used default ctor
  //
  TWindow::Init(0, title, module);
  TFrameWindow::Init(&client, false);
  if (menuResId)
    AssignMenu(menuResId);
}

//
/// Constructs an MDI frame window using an already created non-ObjectWindows
/// window.  Invokes the TFrameWindow and TWindow constructors, passing in the
/// window handle (hWindow) and library ID (module). Initializes the child menu
/// position to the leftmost menu item and constructs a TMDIClient object that
/// corresponds to the supplied clientHWnd.
//
TMDIFrame::TMDIFrame(THandle    handle,
                     THandle    clientHandle,
                     TModule*   module)
:
  TWindow(handle, module),
  TFrameWindow(handle, module)
{
  CHECK(::GetParent(clientHandle) == handle);

  // NOTE: Attr.Menu set in TWindow's constructor
  //
  ClientWnd = new TMDIClient(clientHandle);
  ClientWnd->SetParent(this);
}

//
/// Overrides TWindow's virtual PerformCreate function. Creates the interface
/// element associated with the MDI frame window.
///
//
TWindow::TPerformCreateReturnType
TMdiFrame::PerformCreate(int arg)
{
  PRECONDITIONX(!(OWL_STRICT && arg), _T("The deprecated argument to PerformCreate is disallowed.")); InUse(arg);
  PRECONDITION(GetModule());
  //
  // An MDI frame must have a menu. Give it an empty one if none is supplied.
  // Use RAII to ensure the menu is released in case of failure.
  //
  struct TMenuGuard  
  {
    HMENU m;
    TMenuGuard(HMENU m_) : m(m_) {}
    ~TMenuGuard() {if (m) DestroyMenu(m);}
    operator HMENU() {return m;}
    HMENU RelinquishOwnership() {HMENU tmp = m; m = 0; return m;}
  }
  menu(Attr.Menu ? LoadMenu(Attr.Menu) : CreateMenu());
  WARNX(OwlWin, !menu, 0, _T("Unable to load menu: ") << Attr.Menu << _T(" from ") << *GetModule());
  THandle h = CreateWindowEx(
    Attr.ExStyle,
    GetClassName(),
    Title,
    Attr.Style,
    Attr.X, Attr.Y, Attr.W, Attr.H,
    Parent ? Parent->GetHandle() : 0,
    menu,
    *GetModule(),
    Attr.Param
    );
  if (h) menu.RelinquishOwnership(); // The menu belongs to the window now.
  OWL_SET_OR_RETURN_HANDLE(h);
}

//
/// FindChildMenu searches,  from right to left, the pop-up child menus contained in
/// the newMenu menu resource for a child menu containing command items with one of
/// the following identifiers: CM_CASCADECHILDREN, CM_TITLECHILDREN, or
/// CM_ARRANGEICONS. The return value of FindChildMenu is the HMENU of the first
/// child menu containing one of these identifiers. If one of these identifiers is
/// not found, FindChildMenu returns 0.
/// FindChildMenu is used to locate the menu to which  the MDI child window list
/// will be appended. This call to FindChildMenu is made from within the TMDIClient
/// class.
//
HMENU
TMDIFrame::FindChildMenu(HMENU menu)
{
  if (menu) {
    int   numItems = ::GetMenuItemCount(menu);
    for (int i = numItems-1; i >= 0; i--) {
      HMENU childMenu = ::GetSubMenu(menu, i);
      if (childMenu &&
          (TUser::GetMenuState(childMenu, CM_CASCADECHILDREN, MF_BYCOMMAND) != (uint)-1 ||
           TUser::GetMenuState(childMenu, CM_TILECHILDREN, MF_BYCOMMAND) != (uint)-1 ||
           TUser::GetMenuState(childMenu, CM_ARRANGEICONS, MF_BYCOMMAND) != (uint)-1)) {
        return childMenu;
      }
    }
  }
  return 0;
}

//
/// Searches for the MDI child menu in the new menu bar and updates the child menu
/// position with the specified menu index. Then sends the client window an
/// WM_MDISETMENU message to set the new menu and invokes TWindow::DrawMenuBar to
/// redraw the menu. Returns false if the MDI client indicates that there was no
/// previous menu; otherwise returns true.
//
bool
TMDIFrame::SetMenu(HMENU newMenu)
{
  PRECONDITION(newMenu);

  if (IsFlagSet(wfMainWindow))
    GetApplication()->PreProcessMenu(newMenu);

  if (GetHandle()) {
    HMENU childMenu = FindChildMenu(newMenu);
    HMENU oldMenu = (HMENU)ClientWnd->HandleMessage(WM_MDISETMENU,
                                                    TParam1(newMenu),
                                                    TParam2(childMenu));
    DrawMenuBar();
    if (!oldMenu)
      return false;
  }
  return true;
}

//
/// Returns a pointer to the MDI client window.
//
TMDIClient*
TMDIFrame::GetClientWindow()
{
  return TYPESAFE_DOWNCAST(ClientWnd,TMDIClient);
}

//
/// Locates and returns the child window that is the target of the command and the
/// command enabling messages. If the current application does not have focus or if
/// the focus is within a toolbar in the application, GetCommandTarget returns the
/// most recently active child window. If an alternative form of command processing
/// is desired, a user's main window class can override this function.
//
TWindow::THandle
TMDIFrame::GetCommandTarget()
{
  TFrameWindow* mdiChild = GetClientWindow()->GetActiveMDIChild();

  TRACEX(OwlCmd, 1, "TMDIFrame::GetCommandTarget - returns " << \
                    (mdiChild ? "ActiveMDIChild->GetCommandTarget()" \
                              : "TFrameWindow::GetCommandTarget()"));

  return mdiChild ? mdiChild->GetCommandTarget() : TFrameWindow::GetCommandTarget();
}

//
/// Overrides TWindow::DefWindowProc and provides default processing for any
/// incoming message the MDI frame window does not process.
//
TResult
TMDIFrame::DefWindowProc(TMsgId message, TParam1 param1, TParam2 param2)
{
  return ::DefFrameProc(GetHandle(), ClientWnd ? ClientWnd->GetHandle() : 0,
                        message, param1, param2);
}


IMPLEMENT_STREAMABLE2(TMDIFrame, TFrameWindow, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads an instance of TMDIFrame from the passed ipstream
//
void*
TMDIFrame::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadVirtualBase<TFrameWindow>((TFrameWindow*)GetObject(), is);
  GetObject()->AssignMenu(GetObject()->Attr.Menu);
  return GetObject();
}

//
// Writes the TMDIFrame to the passed opstream
//
void
TMDIFrame::Streamer::Write(opstream& os) const
{
  WriteVirtualBase<TFrameWindow>((TFrameWindow*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


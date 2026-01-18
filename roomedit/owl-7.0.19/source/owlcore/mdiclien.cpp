//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMDIClient.  This defines the basic behavior
/// of all MDI client windows.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/mdi.h>
#include <algorithm>
#include <vector>

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TMDIClient, TWindow)
  EV_COMMAND(CM_CREATECHILD, CmCreateChild),
  EV_COMMAND(CM_TILECHILDREN, CmTileChildren),
  EV_COMMAND(CM_TILECHILDRENHORIZ, CmTileChildrenHoriz),
  EV_COMMAND(CM_CASCADECHILDREN, CmCascadeChildren),
  EV_COMMAND(CM_ARRANGEICONS, CmArrangeIcons),
  EV_COMMAND(CM_CLOSECHILDREN, CmCloseChildren),
  EV_COMMAND_ENABLE(CM_TILECHILDREN, CmChildActionEnable),
  EV_COMMAND_ENABLE(CM_TILECHILDRENHORIZ, CmChildActionEnable),
  EV_COMMAND_ENABLE(CM_CASCADECHILDREN, CmChildActionEnable),
  EV_COMMAND_ENABLE(CM_ARRANGEICONS, CmChildActionEnable),
  EV_COMMAND_ENABLE(CM_CLOSECHILDREN, CmChildActionEnable),
  EV_WM_MDICREATE,
  EV_WM_MDIDESTROY,
  EV_WM_DROPFILES,
END_RESPONSE_TABLE;

//
/// Constructor for a TMDIClient
//
/// Creates an MDI client window object by invoking the base class TWindow
/// constructor, passing it a null parent window, a null title, and the specified
/// library ID. Sets the default client window identifier (IDW_MDICLIENT) and sets
/// the style to include MDIS_ALLCHILDSTYLES, WS_GROUP, WS_TABSTOP, WS_CLIPCHILDREN,
/// WS_VSCROLL, and WS_HSCROLL. Initializes the ClientAttr data member, setting its
/// idFirstChild member to IDW_FIRSTMDICHILD.
//
/// Allocates a CLIENTCREATESTRUCT on the heap and
/// ClientAttr to point to this space
//
TMDIClient::TMDIClient(TModule* module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(nullptr, nullptr, module);

  Attr.Id = IDW_MDICLIENT;

  // Allow client area to grow scroll bars if necessary
  //
  Attr.Style |= MDIS_ALLCHILDSTYLES | WS_GROUP | WS_TABSTOP | WS_CLIPCHILDREN|
                WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL | WS_BORDER;

  Attr.Style &= ~WS_BORDER;
  Attr.ExStyle |= WS_EX_CLIENTEDGE;

  ClientAttr = new CLIENTCREATESTRUCT;
  ClientAttr->hWindowMenu = nullptr;
  ClientAttr->idFirstChild = IDW_FIRSTMDICHILD;

  Attr.Param = reinterpret_cast<LPVOID>(ClientAttr);
  SetFlag(wfStreamTop);
}

//
/// Constructor for a TMDIClient which is being used in a DLL as an alias
/// for a non-OWL window. This ctor is generally not used by derived
/// classes
//
TMDIClient::TMDIClient(THandle handle, TModule* module)
:
  TWindow(handle, module)
{
  ClientAttr = 0;
  SetFlag(wfStreamTop);
}

//
/// Frees the memory associated with ClientAttr
//
TMDIClient::~TMDIClient()
{
  delete ClientAttr;
}

//
/// Returns TMDIClient's registration class name, "MDICLIENT."
//
auto TMDIClient::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{_T("MDICLIENT")};
}

//
/// Use the Windows message to get the active mdi child, and then down cast
/// to our MDI child derived class before returning it.
//
TMDIChild*
TMDIClient::GetActiveMDIChild()
{
  THandle hWnd = THandle(HandleMessage(WM_MDIGETACTIVE));

  return TYPESAFE_DOWNCAST(GetWindowPtr(hWnd), TMDIChild);
}

//
/// Arranges the MDI child window icons at the bottom of the MDI client window.
//
void
TMDIClient::ArrangeIcons()
{
  HandleMessage(WM_MDIICONARRANGE);
}

//
/// Sizes and arranges all of the non-iconized MDI child windows within the MDI
/// client window. The children are overlapped, although each title bar is visible.
//
void
TMDIClient::CascadeChildren()
{
  HandleMessage(WM_MDICASCADE);
}

//
/// Sizes and arranges all of the non-iconized MDI child windows within the MDI
/// client window. The children fill up the entire client area without overlapping.
//
void
TMDIClient::TileChildren(int tile)
{
  HandleMessage(WM_MDITILE, tile);
}

//
/// Preprocess messages in order to translate MDI accelerator keys
///
/// If the specified msg is one of WM_KEYDOWN or WM_SYSKEYDOWN, then the keyboard
/// accelerators are translated for the MDI client.
//
bool
TMDIClient::PreProcessMsg(MSG& msg)
{
  if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
    return TranslateMDISysAccel(GetHandle(), &msg);

  else
    return false;
}

//
/// Creates the interface element associated with the MDI client window. Calls
/// TWindow::Create after first setting the child window menu in ClientAttr to the
/// parent frame window's child menu.
//
bool
TMDIClient::Create()
{
  TMDIFrame*  frame = TYPESAFE_DOWNCAST(GetParentO(),TMDIFrame);

  CHECK(frame);

  ClientAttr->hWindowMenu = frame->FindChildMenu(frame->GetMenu());
  return TWindow::Create();
}

//
/// Initializes and creates a new TMDIChild by calling InitChild and Create.
//
TMDIChild*
TMDIClient::CreateChild()
{
  TMDIChild* child = InitChild();
  CHECK(child);
  if (child->Create())
    return child;
  return nullptr;
}

//
/// Constructs an instance of a TMDIChild with this TMDIClient as parent.
/// Override this virtual function in your derived MDI client class to construct an instance
/// of a derived MDI child class. For example,
/// \code
/// TMyMDIChild* TMyMDIClient::InitChild() // override
/// {
///   return new TMyMDIChild(this);
/// }
/// \endcode
//
TMDIChild*
TMDIClient::InitChild()
{
  return new TMDIChild(*this);
}

//
/// Closes the TMDIChild windows owned by this MDI client.
/// First calls TWindow::CanClose on each of the children, then if all return true, closes them.
///
/// \returns `true` is returned, if the children are closed, or if this client has no children.
/// Otherwise, `false` is returned.
///
/// \note The implementation of this function assumes that the children were dynamically allocated,
/// and have delegated ownership to the MDI parent. If a child is not dynamically allocated, or it
/// has shared ownership, it must be removed from the children list, before this function is called.
//
bool
TMDIClient::CloseChildren()
{
  auto c = GetChildren();
  const auto canClose = std::all_of(c.begin(), c.end(), [](TWindow& w) { return w.CanClose(); });
  if (canClose)
  {
    // Note that the old implementation deleted the child while traversing the linked list of
    // children. However, while the implementation of the iteration may be written to allow this, it
    // is poor programming style to assume that iterators remain valid while mutating the underlying
    // structure. So, to make the code less brittle, we now collect the children into a vector before
    // deleting them.
    //
    // Also note that the code here assumes that the child was dynamically allocated. If not, the
    // child must be taken out of the children list before this function is called.
    //
    auto cv = std::vector<TWindow*>{};
    for (auto& w : c)
      cv.push_back(&w);
    for (auto& w : cv)
    {
      w->Destroy();
      delete w;
    }
  }
  return canClose;
}

//
/// Intercepts the WM_MDICREATE message sent when MDI child windows are created,
/// and, if the client's style includes MDIS_ALLCHILDSTYLES, and the child window's
/// specified style is 0, then changes the child window style attributes to
/// WS_VISIBLE, WS_CHILD, WS_CLIPSIBLINGS, WS_CLIPCHILDREN, WS_SYSMENU, WS_CAPTION,
/// WS_THICKFRAME, WS_MINIMIZEBOX, and WS_MAXIMIZEBOX.
//
HWND
TMDIClient::EvMDICreate(MDICREATESTRUCT & createStruct)
{
  // Fill in default child window styles if they request style 0 since this
  // client by default has set allchildstyles
  //
  if ((Attr.Style&MDIS_ALLCHILDSTYLES) && !createStruct.style)
    createStruct.style =
               WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
               WS_SYSMENU | WS_CAPTION | WS_THICKFRAME |
               WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

  // Work around a Windows MDI bug w/ bad menus if MDI child is created
  // maximized, by hiding child now & maximizing later
  //
  uint32 origStyle = createStruct.style;
  if (createStruct.style & WS_MAXIMIZE)
    createStruct.style &= ~(WS_MAXIMIZE | WS_VISIBLE);

  TResult result = DefaultProcessing();

  // Finish up maximized MDI child workaround
  //
  if (THandle(result) && (origStyle & WS_MAXIMIZE)) {
    HandleMessage(WM_MDIMAXIMIZE, TParam1(result));
    HandleMessage(WM_MDIREFRESHMENU);
  }

  return reinterpret_cast<HWND>(result);
}

//
/// Destroys an MDI child window.
//
void
TMDIClient::EvMDIDestroy(HWND w)
{
  // When an MDI child is destroyed while other children are hidden or disabled,
  // the Windows MDI child management gets confused causing subsequent failure.
  // To prevent this, we temporarily unhide and enable siblings during destroy.
  //
  const auto unhide = [&](TWindow& child)
  {
    if (child.GetHandle() == w || !child.IsWindow())
      return;
    if (!child.IsWindowVisible())
    {
      child.SetWindowPos(nullptr, TRect(), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
      child.SetFlag(wfUnHidden);
    }
    if (!child.IsWindowEnabled())
    {
      child.EnableWindow(true);
      child.SetFlag(wfUnDisabled);
    }
  };

  const auto rehide = [&](TWindow& child)
  {
    if (child.GetHandle() == w || !child.IsWindow())
      return;
    if (child.IsFlagSet(wfUnHidden))
    {
      child.ClearFlag(wfUnHidden);
      child.ShowWindow(SW_HIDE);
    }
    if (child.IsFlagSet(wfUnDisabled))
    {
      child.ClearFlag(wfUnDisabled);
      child.EnableWindow(false);
    }
  };

  for (auto& w : GetChildren()) unhide(w); 
  DefaultProcessing();
  for (auto& w : GetChildren()) rehide(w);
  HandleMessage(WM_MDIREFRESHMENU);
}

//
/// Forwards dropped file messages by default to the parent (MDI Frame) where they
/// can be handled, or be allowed to forward to the application where they can be
/// handled more easily.
//
void
TMDIClient::EvDropFiles(TDropInfo)
{
  Parent->ForwardMessage();
}

//
/// If there are MDI child windows, CmChildActionEnalbe enables any one of the child
/// window action menu items.
//
void
TMDIClient::CmChildActionEnable(TCommandEnabler& commandEnabler)
{
  commandEnabler.Enable(GetFirstChild() != 0);
}



IMPLEMENT_STREAMABLE1(TMDIClient, TWindow);

#if OWL_PERSISTENT_STREAMS

//
/// Reads an instance of TMDIClient from the passed ipstream
//
void*
TMDIClient::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TWindow*)GetObject(), is);

  if (GetObject()->ClientAttr == 0)
    GetObject()->ClientAttr = new CLIENTCREATESTRUCT;  //

  uint idFirstChild;  // Need temp for near data model since ClientAttr is
  is >> idFirstChild;
  GetObject()->ClientAttr->idFirstChild = idFirstChild;
  GetObject()->ClientAttr->hWindowMenu = (HMENU) 0;
  GetObject()->Attr.Param = reinterpret_cast<LPVOID>(GetObject()->ClientAttr);

  return GetObject();
}

//
/// Writes the TMDIClient to the passed opstream
//
void
TMDIClient::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TWindow*)GetObject(), os);
  os << GetObject()->ClientAttr->idFirstChild;
}

#endif

} // OWL namespace
/* ========================================================================== */


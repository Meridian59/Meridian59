//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TMDIChild. This defines the basic behavior of all MDI
/// child windows
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/mdichild.h>
#include <owl/mdi.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlWin);  // diagnostic group for windows

#if !defined(WS_EX_MDICHILD)
# define WS_EX_MDICHILD          0x00000040L
#endif


DEFINE_RESPONSE_TABLE1(TMDIChild, TFrameWindow)
  EV_WM_ENTERIDLE,
  EV_WM_MDIACTIVATE,
  EV_WM_MENUSELECT,
  EV_WM_NCACTIVATE,
END_RESPONSE_TABLE;

namespace
{

  // Initializes attributes (helper used by the constructors).
  //
  void InitAttributes(TWindowAttr& a)
  {
    a.Y = a.H = CW_USEDEFAULT;
    a.Style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
      WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    a.ExStyle |= WS_EX_MDICHILD;
  }

} // namespace

//
/// Constructor for a TMDIChild
///
/// Creates an MDI child window of the MDI client window specified by
/// parent, using the specified title, client window (clientWnd) and instance
/// (inst). Invokes the TFrameWindow base class constructor, supplying parent,
/// title, clientWnd, inst, and indicating that the child window is not to be
/// resized to fit. Invokes the TWindow base class constructor, specifying parent,
/// title, and inst. The window attributes are then adjusted to include WS_VISIBLE,
/// WS_CHILD, WS_CLIPSIBLINGS, WS_CLIPCHILDREN, WS_SYSMENU, WS_CAPTION,
/// WS_THICKFRAME, WS_MINIMIZEBOX, and WS_MAXIMIZEBOX. The dimensions of the window
/// are set to the system default values.
//
TMDIChild::TMDIChild(TMDIClient&     parent,
                     LPCTSTR         title,
                     TWindow*        clientWnd,
                     bool            shrinkToClient,
                     TModule*        module)
{
  TWindow::Init(&parent, title, module);
  TFrameWindow::Init(clientWnd, shrinkToClient);
  InitAttributes(Attr);
}

//
/// String-aware overload
//
TMDIChild::TMDIChild(
  TMDIClient& parent,
  const tstring& title,
  TWindow* clientWnd,
  bool shrinkToClient,
  TModule* module)
{
  TWindow::Init(&parent, title, module);
  TFrameWindow::Init(clientWnd, shrinkToClient);
  InitAttributes(Attr);
}

//
/// Constructor for a TMDIChild which is being used in a DLL as an alias for a non-OWL window
///
/// Creates an MDI child window object from a preexisting window, specified
/// by hWnd. The base class TFrameWindow constructor is invoked, specifying this
/// hWnd, as well as the specified inst. The base class TWindow constructor is
/// invoked, supplying the hWnd and inst parameters.
//
TMDIChild::TMDIChild(THandle handle, TModule* module)
:
  TWindow(handle, module),
  TFrameWindow(handle, module)
{
  Attr.Style = WS_CLIPSIBLINGS;
  Attr.ExStyle |= WS_EX_MDICHILD;
}

//
// Destructor for a MDI child
//
TMDIChild::~TMDIChild()
{
}

//
/// Responds to a request to change a title bar or icon.
///
/// Don't allow NC Activation if we are only temporarily unhidden for window
/// menu maintainance
//
bool
TMDIChild::EvNCActivate(bool active)
{
  return IsFlagSet(wfUnHidden) ? false : TFrameWindow::EvNCActivate(active);
}

//
/// Overrides TWindow's virtual function. Displays a child window according to the
/// value of cmdShow.
///
/// Perform special processing for showing MDI children to make sure that
/// the MDI client maintains the window list correctly
//
bool
TMDIChild::ShowWindow(int cmdShow)
{
  int  retVal = TFrameWindow::ShowWindow(cmdShow);     // 0 if had been hidden

  // Process only if visible state has changed
  //
  if ((retVal != 0) != (cmdShow != SW_HIDE)) {
    if ((THandle)GetParentO()->HandleMessage(WM_MDIGETACTIVE) == *this) {
      if (cmdShow == SW_HIDE)
        GetParentO()->HandleMessage(WM_MDINEXT, TParam1(GetHandle()));
      else
        HandleMessage(WM_NCACTIVATE, true); // resend suppressed message
    }
    GetParentO()->HandleMessage(WM_MDIREFRESHMENU);
  }
  return retVal;
}

//
/// Overrides TWindow's virtual function. Enables a child window.
///
/// Perform special processing for enabling MDI children to make sure that
/// the MDI client maintains the window list correctly
//
bool
TMDIChild::EnableWindow(bool enable)
{
  int retVal = TFrameWindow::EnableWindow(enable);  // 0 if previously enabled

  // Process only if disabled state has actually changed
  //
  if ((retVal!=0) != (enable==0)) {
    if (!enable && (THandle)GetParentO()->HandleMessage(WM_MDIGETACTIVE) == *this)
      GetParentO()->HandleMessage(WM_MDINEXT, TParam1(GetHandle()));
    GetParentO()->HandleMessage(WM_MDIREFRESHMENU);
  }
  return retVal;
}

//
/// Performs preprocessing of window messages for the MDI child window. If keyboard
/// handling is enabled the parent client window's TMDIClient_PreProcessMsg member
/// function is called to preprocess messages. In this case, the return value is
/// true. Otherwise, TFrameWindow::PreProcessMsg is called and its return value
/// becomes the return value of this member function.
///
/// If the MDI child has requested keyboard navigation then TFrameWindow's
/// PreProcessMsg() member function will call ::IsDialogMessage() which will
/// eat the event and the MDI client window won't get a chance to do MDI
/// accelerator processing
///
/// So, we will do it here to make sure it gets done
//
bool
TMDIChild::PreProcessMsg(MSG& msg)
{
  if (KeyboardHandling && GetParentO()->PreProcessMsg(msg))
    return true;
  //????????????????????????????????????????????????????????????????????????????
  // Parent->Parent-> - this is MDIFrame ???
  if (HAccel && ::TranslateAccelerator(GetParentO()->GetParentO()->GetHandle(), HAccel, &msg))
    return true;

  return TFrameWindow::PreProcessMsg(msg);
}

void
TMDIChild::EvMenuSelect(uint menuItemId, uint flags, HMENU hMenu)
{
  TFrameWindow::EvMenuSelect(menuItemId, flags, hMenu);
  // if system message forward it to MainWindows() to display help message
  if ((flags & MF_SYSMENU)>0){
    TFrameWindow* frameWnd = TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TFrameWindow);
    if(frameWnd)
      frameWnd->ForwardMessage(true);
  }
}

//
// Handle WM_ENTERIDLE in order to display help hints on the messsage bar if
// there is a hint pending & this frame has a message bar.
//
void
TMDIChild::EvEnterIdle(uint source, HWND hDlg)
{
  if (source == MSGF_MENU){
    TFrameWindow* frameWnd = TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TFrameWindow);
    if(frameWnd)
      frameWnd->ForwardMessage(true);
  }
  TFrameWindow::EvEnterIdle(source, hDlg);
}

//
/// Destroys the interface element associated with the TMDIChild. Calls
/// EnableAutoCreate for each window in the child list so that the children are also
/// re-created when the parent window is re-created.
//
void
TMDIChild::Destroy(int)
{
  if (GetHandle()) {
    ForEach(owl::DoEnableAutoCreate);  // use iterator function in window.cpp

    if (GetParentO()) {
      // Send destroy message to MDI client window to have it destroy this THandle
      //
      GetParentO()->HandleMessage(WM_MDIDESTROY, TParam1(GetHandle()));
      SetHandle(0);  // Assume success
    }
    else {
      if (::DestroyWindow(GetHandle()))
        SetHandle(0);
      GetApplication()->ResumeThrow();
      WARNX(OwlWin, GetHandle(), 0, "::DestroyWindow(" << (uint)GetHandle() << ") failed");
    }
  }
}

//
/// Creates the interface element associated with the MDI child window. Otherwise,
/// it notifies the parent MDI client window to create the child window's interface
/// element. The supplied menuOrId parameter is ignored because MDI child windows
/// cannot have menus.
///
/// An MDI Child creates its GetHandle() by sending an MDI Create packet to the MDI
/// client.
//
TWindow::TPerformCreateReturnType
TMDIChild::PerformCreate(int)
{
  PRECONDITION(Parent);

  MDICREATESTRUCT  createStruct;
  createStruct.szClass = GetClassName();
  createStruct.szTitle = GetCaption();
  createStruct.hOwner = *GetModule();
  createStruct.x = Attr.X;
  createStruct.y = Attr.Y;
  createStruct.cx = Attr.W;
  createStruct.cy = Attr.H;
  createStruct.style = Attr.Style;
  createStruct.lParam = TParam2(Attr.Param);

  // Work around a Windows MDI bug w/ bad menus if MDI child is created
  // maximized, by hiding child now & maximizing later
  //
  uint32 origStyle = Attr.Style;
  if (createStruct.style & WS_MAXIMIZE)
    createStruct.style &= ~(WS_MAXIMIZE | WS_VISIBLE);

  TResult r = GetParentO()->HandleMessage(WM_MDICREATE, 0, TParam2(&createStruct));
  THandle h = reinterpret_cast<THandle>(r);

  // Finish up maximized MDI child workaround
  //
  if (h && (origStyle & WS_MAXIMIZE)) {
    GetParentO()->HandleMessage(WM_MDIMAXIMIZE, TParam1(GetHandle()));
    GetParentO()->HandleMessage(WM_MDIREFRESHMENU);
  }
  OWL_SET_OR_RETURN_HANDLE(h);
}

//
/// Instructs a client window to activate or deactivate an MDI child window and then
/// sends a message to the child window being activated and the child window being
/// deactivated.
//
void
TMDIChild::EvMDIActivate(THandle hActivated, THandle hDeactivated)
{
  if (GetHandle() == hActivated) {

    // A bug in Windows MDI causes the first MDI child to not get a
    // WM_SETFOCUS. Simulate it now
    //
    if(!GetWindow(GW_HWNDNEXT) &&  GetFocus() != GetHandle())
      HandleMessage(WM_SETFOCUS, TParam1(GetHandle()));

    // Merge this windows menubar with the MDI frame's if there is a
    // MenuDescr assigned
    //
    TFrameWindow* frame = TYPESAFE_DOWNCAST(GetParentO()->GetParentO(),TFrameWindow);
    if (GetMenuDescr()){
      if (frame)
        frame->MergeMenu(*GetMenuDescr());
    }
    if (frame){
      if(GetBarDescr())
        frame->MergeBar(*GetBarDescr());
      else
         frame->RestoreBar();
    }
  }
  else {
    // Restore the MDI frame's menubar if there is no other MDI child being
    // activated
    //
    if(!hActivated){
      TFrameWindow* frame = TYPESAFE_DOWNCAST(GetParentO()->GetParentO(),TFrameWindow);
      if (GetMenuDescr()) {
        if (frame)
          frame->RestoreMenu();
      }
      if (GetBarDescr()){
        if (frame)
          frame->RestoreBar();
      }
    }
  }

  // Forward MDI child activation to our client (if we have one) so that it can
  // perform any type of activate/deactivate processing that it needs
  //
  TWindow* w = GetClientWindow();
  if (w && w->IsWindow())
    w->HandleMessage(WM_MDIACTIVATE, TParam1(hDeactivated), TParam2(hActivated));

  DefaultProcessing();
}

//
/// Overrides TWindow::DefWindowProc to provide default processing for any incoming
/// message the MDI child window does not process. In addition, DefWindowProc
/// handles the following messages: WM_CHILDACTIVATE, WM_GETMINMAXINFO, WM_MENUCHAR,
/// WM_MOVE, WM_SETFOCUS, WM_SIZE, and WM_SYSCOMMAND.
//
TResult
TMDIChild::DefWindowProc(TMsgId msg, TParam1 param1, TParam2 param2)
{
  if (IsFlagSet(wfAlias))
    return TWindow::DefWindowProc(msg, param1, param2);

  return ::DefMDIChildProc(GetHandle(), msg, param1, param2);
}



IMPLEMENT_STREAMABLE2(TMDIChild, TFrameWindow, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads data of the TMDIChild from the passed opstream
//
void*
TMDIChild::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadVirtualBase((TFrameWindow*)GetObject(), is);
  return GetObject();
}

//
// Writes data of the TMDIChild to the passed opstream
//
void
TMDIChild::Streamer::Write(opstream& os) const
{
  WriteVirtualBase((TFrameWindow*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


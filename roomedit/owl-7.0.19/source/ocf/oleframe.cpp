//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TOleFrame.
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <owl/decmdifr.h>
#include <owl/statusba.h>
#include <ocf/ocfevent.h>
#include <ocf/oleframe.h>
#include <ocf/olewindo.h>
#include <owl/commctrl.h>

namespace ocf {

using namespace owl;

using namespace std;

OWL_DIAGINFO;
//DIAG_DECLARE_GROUP(OwlCmd);
DIAG_DEFINE_GROUP_INIT(OWL_INI, OcfOleMenu, 1, 0);

//#define DEBUG_PADS
// define to make pads paint red for debugging time in MS for idle action polling
const int DllIdleTime = 200;

// Four edge locations for inplace-server space negotiation
//
const TDecoratedFrame::TLocation SpaceLoc[] = {
  TDecoratedFrame::Left,
  TDecoratedFrame::Top,
  TDecoratedFrame::Right,
  TDecoratedFrame::Bottom
};

//----------------------------------------------------------------------------
/// An empty no-erase window that inserts itself into its decorated frame
/// parent
//

class TPadWindow : public TWindow {
  public:
    TPadWindow(TWindow* parent, int edge, TModule* module = 0)
      : TWindow(parent, 0, module)
    {
      Attr.Id = IDW_PADDECORATION+edge;
      Attr.Style = WS_CHILD | WS_VISIBLE;
      Attr.W = Attr.H = 1;             // takeup no space until needed
                                       // need 1 since layout overlapps borders
#if defined(DEBUG_PADS)
      SetBkgndColor(TColor(255,0,0));  // to help Debug toolbar negotiations
      strstream s;
      s << _T("Pad") << (_T('0') + edge);
      Title = strnewdup(s.str());
#else
      SetBkgndColor(NoErase);          // Don't erase--to avoid flicker
#endif
    }
};

//
/// \class TRemViewBucket
// ~~~~~ ~~~~~~~~~~~~~~
/// A small window class to act as a holder for the unattached remote server
/// view windows
//
class  TRemViewBucket : public TFrameWindow {
  public:
    TRemViewBucket(TWindow* parent);

    // Overriden virtuals of base class
    //
    bool      SetDocTitle(LPCTSTR, int);

    // Override handlers to act as pass-thru on behalf of remove server view
    //
    TResult   EvCommand(uint id, THandle hWndCtl, uint notifyCode);
    TResult   EvNotify(uint id, TNotify & notifyInfo);
    void      EvCommandEnable(TCommandEnabler& ce);
    TWindow*  FindRemoteWindow();
};

//
//
//
TRemViewBucket::TRemViewBucket(TWindow* parent)
               :TFrameWindow(parent, _T("RemViewBucket"))
{
  Attr.Style = WS_CHILD | WS_DISABLED;
  Attr.Id = IDW_REMVIEWBUCKET;
}

//
//
//
bool
TRemViewBucket::SetDocTitle(LPCTSTR, int)
{
  return true;
}

//
//
//
TWindow*
TRemViewBucket::FindRemoteWindow()
{
  // Check if we can find the object representing the remote window object by
  // traversing the focus window tree
  //
  TWindow* remote = 0;
  HWND focusHwnd  = GetFocus();
  HWND hwnd       = focusHwnd;
  while (hwnd) {
    remote = GetWindowPtr(hwnd);
    if (remote) {
      // The remote window has the remViewBucket as its parent
      //
      if (remote->GetParentO() == this)
        break;
      else
        remote = 0;
    }
    hwnd = ::GetParent(hwnd);
  }
  return remote;
}

//
//
//
TResult
TRemViewBucket::EvCommand(uint id, THandle hWndCtl, uint notifyCode)
{
  // Find remote view
  //
  TWindow* handler = FindRemoteWindow();

  // Default to parent if none was found
  //
  if (!handler)
    handler = GetParentO();

  // Forward event
  //
  CHECK(handler);
  return handler->EvCommand(id, hWndCtl, notifyCode);
}

//
//
//
TResult
TRemViewBucket::EvNotify(uint id, TNotify & notifyInfo)
{
  // Find remote view
  //
  TWindow* handler = FindRemoteWindow();

  // Default to parent if none was found
  //
  if (!handler)
    handler = GetParentO();

  // Forward event
  //
  CHECK(handler);
  return handler->EvNotify(id, notifyInfo);
}

//
//
//
void
TRemViewBucket::EvCommandEnable(TCommandEnabler& ce)
{
  // Find remote view
  //
  TWindow* handler = FindRemoteWindow();

  // Default to parent if none was found
  //
  if (!handler)
    handler = GetParentO();

  // Forward event
  //
  CHECK(handler);
  handler->EvCommandEnable(ce);
}

DEFINE_RESPONSE_TABLE1(TOleFrame, TDecoratedFrame)
  EV_WM_SIZE,
  EV_WM_TIMER,
  EV_WM_ACTIVATEAPP,

  EV_MESSAGE(WM_OCEVENT, EvOcEvent),
  EV_OC_APPINSMENUS,
  EV_OC_APPMENUS,
  EV_OC_APPPROCESSMSG,
  EV_OC_APPFRAMERECT,
  EV_OC_APPBORDERSPACEREQ,
  EV_OC_APPBORDERSPACESET,
  EV_OC_APPSTATUSTEXT,
  EV_OC_APPRESTOREUI,
  EV_OC_APPSHUTDOWN,
END_RESPONSE_TABLE;

//
/// Constructs a TOleFrame object with the specified caption for the frame window
/// (title), the client window (clientWnd), and module instance. The
/// trackMenuSelection parameter indicates whether or not the frame window should
/// track menu selections (that is, display hint text in the status bar of the
/// window when a menu item is highlighted).
//
TOleFrame::TOleFrame(LPCTSTR   title,
                     TWindow* clientWnd,
                     bool     trackMenuSelection,
                     TModule* module)
:
  TDecoratedFrame(0, title, clientWnd, trackMenuSelection, module),
  TFrameWindow(0, title, clientWnd, false, module),
  TWindow(0, title, module),
  HOldMenu(0),
  OcApp(0),
  StashCount(0),
  OcShutDown(DontCare)
{
  new TRemViewBucket(this);  // Construct bucket to hold hidden server windows

  // Retrieve the OcApp ptr from our owning application if it is a TOcAppHost
  //
  TOcAppHost* ocm = TYPESAFE_DOWNCAST(GetApplication(), TOcAppHost);
  if (ocm)
    SetOcApp(ocm->OcApp);
}

//
/// Destroys a TOleFrame object.
//
/// Let the OC app go. It will delete itself when it can
//
TOleFrame::~TOleFrame()
{
}

//
/// Initial set of OcApp being passed to us to use.
//
/// Sets the ObjectComponents application associated with this frame window to the
/// applications specified in the app parameter.
//
void
TOleFrame::SetOcApp(TOcApp* ocApp)
{
  PRECONDITION(ocApp);
  OcApp = ocApp;

  // Initialize OLE 2 clipboard format names
  //
  TCHAR f[] = _T("%s");
  AddUserFormatName(f, f, ocrEmbedSource);
  AddUserFormatName(f, f, ocrEmbeddedObject);
  AddUserFormatName(f, f, ocrLinkSource);
}

//
/// Associates the ObjectComponents application with the window's HWND so that the
/// TOcApp and the window can communicate. Prepares a place to insert the server's
/// toolbars when in-place editing of the embedded object occurs.
//
void
TOleFrame::SetupWindow()
{
  PRECONDITION(OcApp);
  TDecoratedFrame::SetupWindow();
  OcApp->SetupWindow(GetHandle());

  // Insert the four pad windows for in-place server toolbars. Inserting last
  // will place them as the inner-most decorations, which is needed with the
  // status bar
  //
  for (int edge = 0; edge < 4; edge++)
    Insert(*new TPadWindow(this, edge), ::ocf:: SpaceLoc[edge]);

  // Create a timer to allow us to poll for idle time when we are a dll server
  //
  if (!OcApp->IsOptionSet(amExeMode))
    SetTimer(IDT_DLLIDLE, DllIdleTime);
}

//
/// Performs normal window cleanup of any HWND-related resources. For DLL servers,
/// CleanupWindow destroys the idle timer.
//
void
TOleFrame::CleanupWindow()
{
  if (!OcApp->IsOptionSet(amExeMode))
    KillTimer(IDT_DLLIDLE);
}

//
/// Adds user defined formats and the corresponding names to the list of Clipboard
/// formats. Use this function if you want to associate a Clipboard data format
/// (name) with the description of the data format as it appears to users in the
/// Help text of the Paste Special dialog box (resultName). To use a standard
/// Clipboard format, set the id parameter to an appropriate constant (for example,
/// CF_TEXT). Otherwise, if the format is identified by a string, pass the string as
/// the name and omit the ID.
//
void
TOleFrame::AddUserFormatName(LPCTSTR name, LPCTSTR resultName,
                             LPCTSTR id)
{
  PRECONDITION(OcApp);
  OcApp->AddUserFormatName(name, resultName, id);
}

//
/// Sets the OcShutDown data member to ViewInitiated (if close is true and
/// OcShutDown = DontCare) or DontCare (if close if false and OcShutDown =
/// ViewInitiated.
//
void
TOleFrame::OleViewClosing(bool close)
{
  if (close && OcShutDown == DontCare) {
    OcShutDown = ViewInitiated;
  }
  else if (!close && OcShutDown == ViewInitiated) {
    OcShutDown = DontCare;
  }
}

namespace {

/// Disconnect document servers with their clients.
/// Document servers can be documents with objects copied on the clipboard or
/// documents brought up though linking.
//
void DisconnectDocServer_(TWindow& win)
{
  for (auto& child : win.GetChildren())
    DisconnectDocServer_(child);

  TOleWindow* oleWin = TYPESAFE_DOWNCAST(&win, TOleWindow);
  if (oleWin)
    oleWin->OleShutDown();
}

} // namespace

//
/// Checks with all the connected servers to ensure that they can close before
/// destroying the frame window. If the user closes the application with objects
/// still embedded, Destroy hides the frame window instead of destroying it.
//
void
TOleFrame::Destroy(int retVal)
{
  if (!GetHandle() || OcShutDown == UserInitiated)
    return;

  // Disconnect document servers with their clients if user shuts down the app
  //
  if (OcShutDown == DontCare) {
    OcShutDown = UserInitiated;
    for (auto& child : GetChildren())
      DisconnectDocServer_(child);
  }

  if (!OcApp->CanClose()) {
    OcApp->SetOption(amEmbedding, true);
    OcShutDown = DontCare; // reset the shutdown flag
    ShowWindow(SW_HIDE);
  }
  else {
    bool dllServer = !OcApp->IsOptionSet(amExeMode);

    TDecoratedFrame::Destroy(retVal);

    // If user shuts down the DLL server (as in the case of open-editing,
    // we need to set the mainwindow flag to 0 so that only the application
    // will be destroyed.  All windows are destroyed in previous calls.
    //
    if (dllServer && OcShutDown != ServerInitiated) {
//      GetApplication()->SetMainWindow(0);
      GetApplication()->ClearMainWindow();
      delete GetApplication();
    }
  }
}

//
/// Passes a WM_SIZE message to TLayoutWindow.
//
/// Responds to an EV_WM_SIZE message indicating a change in the frame window's size
/// and forwards this information to the TOcApp, which then notifies any in-place
/// server. The server uses this information to change the size of its toolbar, if
/// necessary.
//
void
TOleFrame::EvSize(uint sizeType, const TSize& size)
{
  TDecoratedFrame::EvSize(sizeType, size);
  if (OcApp)
    OcApp->EvResize();
}

//
/// Responds to a EV_WM_ACTIVATEAPP message sent when a window is activated or
/// deactivated. If active is true, the window is being activated.
/// This message is sent to the top-level window being deactivated before it is sent
/// to the top-level window being activated. hTask is a handle to the current
/// process.
/// This event is forwarded to the TOcApp object, which activates an in-place server
/// if one exists.
//
void
TOleFrame::EvActivateApp(bool active, DWORD threadId)
{
  OcApp->EvActivate(active);
  TDecoratedFrame::EvActivateApp(active, threadId);
}

//
/// If this is a DLL server, EvTimer responds to a timer message by running an idle
/// loop if the message queue is empty.
//
void
TOleFrame::EvTimer(uint timerId)
{
  if (timerId == IDT_DLLIDLE)
    GetApplication()->PostDispatchAction();
  TWindow::EvTimer(timerId);
}

//
/// Responds to a WM_OCEVENT message and subdispatches it to one of the EvOcXxxx
/// event-handling functions based on the value of wParam. WM_OCEVENT messages are
/// sent by ObjectComponents when it needs to communicate with an OLE-generated
/// event; for example, if a server wants to display toolbars.
//
TResult
TOleFrame::EvOcEvent(TParam1 param1, TParam2 param2)
{
  TEventHandler::TEventInfo eventInfo(WM_OCEVENT, static_cast<uint>(param1));

  // Give the client window the first chance at it
  //
  TWindow* receiver = GetClientWindow();
  if (receiver->Find(eventInfo))
    return receiver->Dispatch(eventInfo, param1, param2);

  // Then try this frame
  //
  if (Find(eventInfo))
    return Dispatch(eventInfo, param1, param2);

  // Last, try the application in case it wants to override events
  //
  if (GetApplication()->Find(eventInfo))
    return GetApplication()->Dispatch(eventInfo, param1, param2);
  return 0;
}

//
/// Responds to an EV_OC_APPINSMENUS message by merging the container's menu into
/// the shared menu. The sharedMenu parameter refers to this merged menu.
//
bool
TOleFrame::EvOcAppInsMenus(TOcMenuDescr & sharedMenu)
{
  if (HOldMenu) {
    TRACEX(OcfOleMenu, 0, _T("EvOcAppInsMenus called while HOldMenu is ") << hex <<
      static_cast<void*>(HOldMenu));
    return true;
  }

  // Recreate a temporary composite menu for frame
  //
  TMenuDescr compMenuDesc; // empty menudescr
  if (GetMenuDescr()) {
    compMenuDesc.Merge(*GetMenuDescr());

    // Mask off the server menus
    //
    compMenuDesc.Merge(TMenuDescr(0,  0, -1, 0, -1, 0, -1));
  }
  else
    return false; // stop menu negotiation if we don't have menu

  // Merge into the OLE shared menubar
  //
  TMenuDescr shMenuDescr(sharedMenu.HMenu,
                         sharedMenu.Width[0],
                         sharedMenu.Width[1],
                         sharedMenu.Width[2],
                         sharedMenu.Width[3],
                         sharedMenu.Width[4],
                         sharedMenu.Width[5]);
  shMenuDescr.Merge(compMenuDesc);

  // Copy the shared menu widths back to the OC struct
  //
  for (int i = 0; i < 6; i++)
    sharedMenu.Width[i] = shMenuDescr.GetGroupCount(i);

  // Save the container popups so they can be destroyed later
  //
  StashContainerPopups(shMenuDescr);

  TRACEX(OcfOleMenu, 0, _T("Merged menu ") << hex << static_cast<void*>(sharedMenu.HMenu));
  return true;
}

//
/// Responds to an OC_OCAPPMENUS sent to the container. The response is to install a
/// merged menu bar.
//
bool
TOleFrame::EvOcAppMenus(TOcMenuDescr & appMenus)
{
  if (!appMenus.HMenu) {
    if (HOldMenu) {
      TRACEX(OcfOleMenu, 0, _T("EvOcAppMenus(0) resetting Old ") << hex <<
        static_cast<void*>(HOldMenu));
      TMenu oleMenu(GetHandle());
      SetMenu(HOldMenu);     // assumes we are just restoring the old owl menu
      HOldMenu = 0;
    }
    DestroyStashedPopups();  // destroy the popup copies we made
    return true;
  }

  // Don't set the menu again if we are holding a merged one already
  //
  if (HOldMenu) {
    TRACEX(OcfOleMenu, 0, _T("EvOcAppMenus called while HOldMenu is ") << hex <<
      static_cast<void*>(HOldMenu));
    return true;
  }

  HOldMenu = GetMenu();
  TRACEX(OcfOleMenu, 0, _T("Saved Old ") << hex << static_cast<void*>(HOldMenu));

  SetMenu(appMenus.HMenu);
  TRACEX(OcfOleMenu, 0, _T("Set merged ") << hex << static_cast<void*>(appMenus.HMenu));

  return true;
}

//
/// Responds to an EV_OC_APPROCESSMSG message sent to the container asking the server
/// to process accelerators and other messages from the container's message queue.
//
bool
TOleFrame::EvOcAppProcessMsg(MSG * msg)
{
  return GetApplication()->ProcessAppMsg(*msg);
}

//
/// Responds to an OC_APPFRAMERECT message sent to a container. The response is to
/// get the coordinates of the client area rectangle of the application's main window.
//
bool
TOleFrame::EvOcAppFrameRect(TRect * rect)
{
  PRECONDITION(rect);
  *rect = GetClientRect();
  TWindow*  sb = ChildWithId(IDW_STATUSBAR);
  if (sb) {
    TRect sbr = sb->GetClientRect();
    rect->bottom -= sbr.bottom+1;
  }
  return true;
}

//
/// Responds to an OC_APPBORDERSPACEREQ message sent to a container. The response is
/// to ask the container if it can give border space in its frame window to the
/// server.
//
bool
TOleFrame::EvOcAppBorderSpaceReq(TRect * /*space*/)
{
  return true;
}

//
/// Responds to an OC_APPBORDERSPACESET message by making room in the container's
/// frame window for the border space that the server has requested.
//
bool
TOleFrame::EvOcAppBorderSpaceSet(TRect * space)
{
  // Resize pad decorations based on edges requested
  //
  int* edges = (int*)space;   // treat space as array of 4 int edges
  bool needLayout = false;
  for (int i = 0; i < 4; i++) {
    TWindow*  pad = ChildWithId(IDW_PADDECORATION+i);
    if (pad) {
      int edge = edges && edges[i] ? edges[i]+1 : 1;
      if ((i%2 == 0 && pad->GetWindowAttr().W != edge) ||   // left & right edge
          (i%2 == 1 && pad->GetWindowAttr().H != edge)) {   // top & bottom edge
        TLayoutMetrics m;
        GetChildLayoutMetrics(*pad, m);
        pad->GetWindowAttr().H = pad->GetWindowAttr().W = edge;  // set both axis, one will be stretched
        SetChildLayoutMetrics(*pad, m);
        needLayout = true;
      }
    }
  }

  // Turn on/off control bar as needed
  //
  TWindow*  tb = ChildWithId(IDW_TOOLBAR);
  if (tb)
    if ((space && tb->IsWindowVisible()) || (!space && !tb->IsWindowVisible())) {
      SendMessage(WM_COMMAND, IDW_TOOLBAR);  // toggle tool bar on/off
      needLayout = false;  // layout already done now by decorated frame.
    }

  // Now do layout once at the end to reduce repaint
  //
  if (needLayout)
    Layout();

  return true;
}

//
/// Responds to an OC_APPSTATUSTEXT message by displaying text from the server on
/// this container's status bar.
//
void
TOleFrame::EvOcAppStatusText(LPCTSTR text)
{
  PRECONDITION(text);
  TMessageBar* mb = TYPESAFE_DOWNCAST(ChildWithId(IDW_STATUSBAR), TMessageBar);
  if (mb) {
    //mb->SetHintText(text);   // is the text a hint, or general status??
    mb->SetText(text);
  }
}

//
/// Stores a local copy of the pop-up menus so they can be used for menu merging and
/// then destroyed later by DestroyStashedPopups. shMenuDescr is the shared menu
/// descriptor to be stored. Increments StashCount each time the pop-up menus are saved.
//
void TOleFrame::StashContainerPopups(const TMenuDescr& shMenuDescr)
{
  StashCount++;

  int m = 0;
  for (int i = 0; i < 6; i++) {
    if (i%2 == 0)
      for (int j = 0; j < shMenuDescr.GetGroupCount(i); j++) {
        uint  state = shMenuDescr.GetMenuState(m+j, MF_BYPOSITION);
        if (state == uint(-1))
          continue;
        TRACEX(OcfOleMenu, 1, _T("Stashing ") << hex << static_cast<void*>(shMenuDescr.GetSubMenu(m+j)));
        StashedContainerPopups.AppendMenu(state, TMenu(shMenuDescr.GetSubMenu(m+j)), _T(""));
      }
    m += shMenuDescr.GetGroupCount(i);
  }
}

//
/// Destroys the previously stored shared pop-up menus. Checks to see if StashCount
/// is 0 before destroying the menus.
//
void TOleFrame::DestroyStashedPopups()
{
  if (--StashCount)
    return;

  while (StashedContainerPopups.GetMenuItemCount()) {
    TRACEX(OcfOleMenu, 1, _T("Destroying ") << hex << static_cast<void*>(StashedContainerPopups.GetSubMenu(0)));
    StashedContainerPopups.DeleteMenu(0, MF_BYPOSITION);
  }
}

//
/// Responds to an OC_APPRESTOREUI message by restoring the container's normal menu
/// and borders because in-place editing has finished.
//
void
TOleFrame::EvOcAppRestoreUI()
{
  // Only restore the old menu if we are holding a merged one
  //
  if (HOldMenu) {
    TRACEX(OcfOleMenu, 0, _T("EvOcAppRestoreUI resetting Old ") << hex <<
      static_cast<void*>(HOldMenu));
    TMenu oleMenu(GetHandle());
    SetMenu(HOldMenu);    // assumes we are just restoring the old owl menu
    HOldMenu = 0;
  }

  // Remove pad decorations & restore our toobar if we have one
  //
  EvOcAppBorderSpaceSet(0);
}

//
/// Responds to an OC_APPSHUTDOWN message indicating that the last embedded object
/// has been closed. The response is to shut down the server.
//
bool
TOleFrame::EvOcAppShutdown()
{
  // If TOleFrame was created purely for embedded server, then
  // we want to shut down the app when nobody is using the server.
  // The amEmbedding flag will be set to false if user created normal
  // document in this frame.
  //
  if (OcShutDown == DontCare && OcApp->IsOptionSet(amEmbedding)) {
    // The shut down is initiated by OCF
    //
    OcShutDown = ServerInitiated;

    // Post frame close to kill the app later
    //
    PostMessage(WM_CLOSE);

    return true;
  }
  else {

    // If the last view closing caused the ocapp to shutdown, then close this
    // frame.
    //
    if (OcApp->IsOptionSet(amEmbedding) && OcShutDown == ViewInitiated)
      PostMessage(WM_CLOSE);

    return false; // Shut down initiated by user
  }
}

} // OCF namespace

//==============================================================================


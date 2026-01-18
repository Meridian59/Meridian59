//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TWindow.  This defines the basic behavior of all Windows.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/window.h>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/scroller.h>
#include <owl/gdiobjec.h>
#include <owl/menu.h>
#include <owl/framewin.h>
#include <owl/commctrl.h>
#include <owl/shellitm.h>
#include <owl/tooltip.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <owl/registry.h>

#include <owl/hlpmanag.h> //THelpHitInfo
#include <array>
#include <vector>
#include <algorithm>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMsg);  // diagnostic group for message tracing
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlWin, 1, 0);  // diag. group for windows
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlCmd, 1, 0);  // diag. group for commands

namespace {

  //
  // Attempts to retrieve the system setting for mouse wheel scrolling granularity.
  // If retrieval fails, the failure is logged (in debug mode) and the system default is returned.
  //
  auto GetWheelScrollLines_() -> uint
  {
    auto n = 3u; // System default.
    const auto r = ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &n, 0);
    WARN(!r, _T("SystemParametersInfo(SPI_GETWHEELSCROLLLINES) failed. LastError: ") << GetLastError()); InUse(r);
    return n;
  };

} // namespace

} // OWL namespace

//
// Define to use rtti to create unique window ids for the message cache
//
#define OWL_RTTI_MSGCACHE
#define TYPE_UNIQUE_ID(t) reinterpret_cast<UINT_PTR>(typeid(t).name())

bool gBatchMode = false; // true if we were invoked to do batch processing
                         // which means windows should be invisible and whatnot
                         // currently this is only set if we have an output parameter

namespace owl {

//
// Externs defined in owl.cpp
//
extern _OWLFUNC(void) SetCreationWindow(owl::TWindow*);
extern _OWLFUNC(owl::TWindow*) GetCreationWindow();
extern _OWLDATA(uint) GetWindowPtrMsgId;

DEFINE_RESPONSE_TABLE(TWindow)
  EV_WM_SETCURSOR,
  EV_WM_SIZE,
  EV_WM_MOVE,
  EV_WM_COMPAREITEM,
  EV_WM_DELETEITEM,
  EV_WM_DRAWITEM,
  EV_WM_MEASUREITEM,
  EV_WM_VSCROLL,
  EV_WM_HSCROLL,
  EV_WM_CHILDINVALID,
  EV_WM_ERASEBKGND,
  EV_WM_CTLCOLORBTN(EvCtlColor),
  EV_WM_CTLCOLORDLG(EvCtlColor),
  EV_WM_CTLCOLOREDIT(EvCtlColor),
  EV_WM_CTLCOLORLISTBOX(EvCtlColor),

#if defined(OWL5_COMPAT)

  EV_WM_CTLCOLORMSGBOX(EvCtlColor), // This entry is deprecated. See TDispatch<WM_CTLCOLORMSGBOX>.

#endif

  EV_WM_CTLCOLORSCROLLBAR(EvCtlColor),
  EV_WM_CTLCOLORSTATIC(EvCtlColor),
  EV_WM_PAINT,
  EV_WM_LBUTTONDOWN,
  EV_WM_KILLFOCUS,
  EV_WM_CREATE,
  EV_WM_CLOSE,
  EV_WM_DESTROY,
  EV_WM_NCDESTROY,
  EV_WM_QUERYENDSESSION,
  EV_WM_ENDSESSION,
  EV_WM_SYSCOLORCHANGE,
  EV_WM_INITMENUPOPUP,
  EV_WM_CONTEXTMENU,
  EV_WM_ENTERIDLE,
  EV_WM_MOUSEWHEEL,
  EV_WM_MOUSEHWHEEL,
  EV_WM_SETFONT,
END_RESPONSE_TABLE;



/// \class TCommandEnabler
///
/// An abstract base class used for automatic enabling and disabling of commands,
/// TCommandEnabler is a class from which you can derive other classes, each one
/// having its own command enabler. For example, TButtonGadgetEnabler is a derived
/// class that's a command enabler for button gadgets, and TMenuItemEnabler is a
/// derived class that's a command enabler for menu items. Although your derived
/// classes are likely to use only the functions Enable, SetCheck, and GetHandled,
/// all of TCommandEnabler's functions are described so that you can better
/// understand how ObjectWindows uses command processing. The following paragraphs
/// explain the dynamics of command processing.
///
///
/// <b>Handling command messages</b>
///
/// Commands are messages of the windows WM_COMMAND type that have associated
/// command identifiers (for example, CM_FILEMENU). When the user selects an item
/// from a menu or a toolbar, when a control sends a notification message to its
/// parent window, or when an accelerator keystroke is translated, a WM_COMMAND
/// message is sent to a window.
///
///
/// <b>Responding to command messages</b>
///
/// A command is handled differently depending on which type of command a window
/// receives. Menu items and accelerator commands are handled by adding a command
/// entry to a message response table using the EV_COMMAND macro. The entry requires
/// two arguments:
/// - A command identifier (for example, CM_LISTUNDO)
/// - A member function (for example, CMEditUndo)
///
/// Child ID notifications, messages that a child window sends to its parent window,
/// are handling by using one of the notification macros defined in the header file
/// windowev.h.
/// It is also possible to handle a child ID notification at the child window by
/// adding an entry to the child's message response table using the
/// EV_NOTIFY_AT_CHILD macro. This entry requires the following arguments:
/// -  A notification message (for example, LBN_DBLCLK)
/// -  A member function (for example, CmEditItem)
///
/// <b>TWindow command processing</b>
///
/// One of the classes designed to handle command processing, TWindow performs basic
/// command processing according to these steps:
/// - 1.  The member function WindowProc calls the virtual member function EvCommand.
/// - 2.  EvCommand checks to see if the window has requested handling the command by
/// looking up the command in the message response table.
/// - 3.  If the window has requested handling the command identifier by using the
/// EV_COMMAND macro, the command is dispatched.
///
/// TWindow also handles Child ID notifications at the child window level.
///
///
/// <b>TFrameWindow command processing</b>
///
/// TFrameWindow provides specialized command processing by overriding its member
/// function EvCommand and sending the command down the command chain (that is, the
/// chain of windows from the focus window back up to the frame itself, the original
/// receiver of the command message).
///
/// If no window in the command chain handles the command, TFrameWindow delegates
/// the command to the application object. Although this last step is theoretically
/// performed by the frame window, it is actually done by TWindow's member function,
/// DefaultProcessing().
///
///
/// <b>Invoking EvCommand</b>
///
/// When TFrameWindow sends a command down the command chain, it does not directly
/// dispatch the command; instead, it invokes the window's EvCommand member
/// function. This procedure gives the windows in the command chain the flexibility
/// to handle a command by overriding the member function EvCommand instead of being
/// limited to handling only the commands requested by the EV_COMMAND macro.
///
///
/// <b>Handling command enable messages</b>
///
/// Most applications expend considerable energy updating menu items and tool bar
/// buttons to provide the necessary feedback indicating that a command has been
/// enabled. In order to simplify this procedure, ObjectWindows lets the event
/// handler that is going to handle the command make the decision about whether or
/// not to enable or disable a command.
///
/// Although the WM_COMMAND_ENABLE message is sent down the same command chain as
/// the WM_COMMAND event; exactly when the WM_COMMAND_ENABLE message is sent depends
/// on the type of command enabling that needs to be processed.
///
///
/// <b>Command enabling for menu items</b>
///
/// TFrameWindow performs this type of command enabling when it receives a
/// WM_INITMENUPOPUP message. It sends this message before a menu list appears.
/// ObjectWindows then identifies the menu commands using the command IDs and sends
/// requests for the commands to be enabled.
///
/// Note that because Object Windows actively maintains toolbars and menu items, any
/// changes made to the variables involved in the command enabling functions are
/// implemented dynamically and not just when a window is activated.
///
///
/// <b>Command enabling for toolbar buttons</b>
///
/// This type of command enabling is performed during idle processing (in the
/// IdleAction function). The Default Message-Processing Flowchart that accompanies
/// TWindow::DefaultProcessing is a graphical illustration of this process.
///
///
/// <b>Creating specialized command enablers</b>
///
/// Associated with the WM_COMMAND_ENABLE message is an object of the
/// TCommandEnabler type. This family of command enablers includes specialized
/// command enablers for menu items and toolbar buttons.
///
///  As you can see from TCommandEnabler's class declaration, you can do
/// considerably more than simply enable or disable a command using the command
/// enabler. For example, you have the ability to change the text associated with
/// the command as well as the state of the command.
///
///
/// <b>Using the EV_COMMAND_ENABLE macro</b>
///
/// You can use the EV_COMMAND_ENABLE macro to handle WM_COMMAND_ENABLE  messages.
/// Just as you do with the EV_COMMAND macro, you specify the command identifier
/// that you want to handle and the member function you want to invoke to handle the
/// message.
///
///
/// <b>Automatically enabling and disabling commands</b>
///
/// ObjectWindows simplifies enabling and disabling of commands by automatically
/// disabling commands for which there are no associated handlers. TFrameWindow's
/// member function EvCommandEnable performs this operation, which involves
/// completing a two-pass algorithm:
/// - 1.  The first pass sends a WM_COMMAND_ENABLE message down the command chain
/// giving each window an explicit opportunity to do the command enabling.
/// - 2.  If no window handles the command enabling request, then ObjectWindows checks
/// to see whether any windows in the command chain are going to handle the command
/// through any associated EV_COMMAND entries in their response tables. If there is
/// a command handler in one of the response tables, then the command is enabled;
/// otherwise it is disabled.
///
/// Because of this implicit command enabling or disabling, you do not need to (and
/// actually should not) do explicit command enabling unless you want to change the
/// command text, change the command state, or conditionally enable or disable the
/// command.
///
/// If you handle commands indirectly by overriding the member function EvCommand
/// instead of using the  EV_COMMAND macro to add a response table entry, then
/// ObjectWindows will not be aware that you are handling the command. Consequently,
/// the command may be automatically disabled. Should this occur, the appropriate
/// action to take is to also override the member function EvCommandEnable and
/// explicitly enable the command.


//
/// Constructs the TCommandEnabler object with the specified command ID. Sets the
/// message responder (hWndReceiver) to zero.
//
TCommandEnabler::TCommandEnabler(uint id, HWND hReceiver)
:
  Id(id),
  HWndReceiver(hReceiver)
{
  Flags = 0;
}

//
/// Enables or disables the command sender. When Enable is called, it sets the
/// Handled flag.
//
void
TCommandEnabler::Enable(bool)
{
  Flags |= WasHandled;
}

void TWindow::TraceWindowPlacement()
{
  auto wp = GetWindowPlacement();
  TRACEX(OwlWin, 1,
    "PLACEMENT=" << TRect{wp.rcNormalPosition} <<
    " handle=" << GetHandle() <<
    " min=" << TPoint{wp.ptMinPosition} <<
    " max=" << TPoint{wp.ptMaxPosition} <<
    " showCmd=" << wp.showCmd <<
    " flags=" << wp.flags);
}

//
/// Adds this to the child list of parent if nonzero, and calls EnableAutoCreate so
/// that this will be created and displayed along with parent. Also sets the title
/// of the window and initializes the window's creation attributes.
///
/// The following paragraphs describe procedures common to both constructor
/// syntaxes. module specifies the application or DLL that owns the TWindow
/// instance. ObjectWindows needs the correct value of module to find needed
/// resources. If module is 0, TWindow sets its module according to the following
/// rules:
/// - If the window has a parent, the parent's module is used.
/// - If the TWindow constructor is invoked from an application, the
/// module is set to the application.
/// - If the TWindow constructor is invoked from a DLL that is
/// dynamically linked with the ObjectWindows DLL and the currently running
/// application is linked the same way, the module is set to the currently running
/// application.
/// - If the TWindow constructor is invoked from a DLL that is
/// statically linked with the ObjectWindows library or the invoking DLL is
/// dynamically linked with ObjectWindows DLL but the currently running application
/// is not, no default is used for setting the module. Instead, a TXInvalidModule
/// exception is thrown and the object is not created.
//
TWindow::TWindow(TWindow* parent, LPCTSTR title, TModule* module)
  : Handle{nullptr}, Title{nullptr}, DefaultProc{nullptr}, InstanceProc{nullptr}, Module{nullptr}
{
  Init(parent, title, module);
}

//
/// String-aware overload
//
TWindow::TWindow(TWindow* parent, const tstring& title, TModule* module)
  : Handle{nullptr}, Title{nullptr}, DefaultProc{nullptr}, InstanceProc{nullptr}, Module{nullptr}
{
  Init(parent, title, module);
}

//
/// Constructs a TWindow that is used as an alias for a non-ObjectWindows window,
/// and sets wfAlias. Because the HWND is already available, this constructor,
/// unlike the other TWindow constructor, performs the "thunking" and extraction of
/// HWND information instead of waiting until the function Create creates the
/// interface element.
///
/// The following paragraphs describe procedures common to both constructor
/// syntaxes. module specifies the application or DLL that owns the TWindow
/// instance. ObjectWindows needs the correct value of module to find needed
/// resources. If module is 0, TWindow sets its module according to the following
/// rules:
/// - If the window has a parent, the parent's module is used.
/// - If the TWindow constructor is invoked from an application, the
/// module is set to the application.
/// - If the TWindow constructor is invoked from a DLL that is
/// dynamically linked with the ObjectWindows DLL and the currently running
/// application is linked the same way, the module is set to the currently running
/// application.
/// - If the TWindow constructor is invoked from a DLL that is
/// statically linked with the ObjectWindows library or the invoking DLL is
/// dynamically linked with ObjectWindows DLL but the currently running application
/// is not, no default is used for setting the module. Instead, a TXInvalidModule
/// exception is thrown and the object is not created.

TWindow::TWindow(HWND handle, TModule* module)
  : Handle{nullptr}, Title{nullptr}, DefaultProc{nullptr}, InstanceProc{nullptr}, Module{nullptr}
{
  PRECONDITION(handle);
  Init(handle, module);
}

//
/// Protected constructor for use by immediate virtually derived classes.
/// Immediate derivitives must call an Init() before constructions are done.
//
TWindow::TWindow()
  : Handle{nullptr}, Title{nullptr}, DefaultProc{nullptr}, InstanceProc{nullptr}, Module{nullptr}
{}

//
/// Normal initialization of a default constructed TWindow. Is ignored if
/// called more than once.
//
void
TWindow::Init(TWindow* parent, LPCTSTR title, TModule* module)
{
  if (!InstanceProc) {
    SetCaption(title);
    PerformInit(parent, module);
    EnableAutoCreate();

    // Initialize creation attributes
    //
    Attr.Style = WS_CHILD | WS_VISIBLE;
    Attr.X = Attr.Y = Attr.W = Attr.H = 0;
    Attr.ExStyle = 0;
    Attr.Id = 0;
  }
}

//
/// Wrapper initialization of a default constructed TWindow. Is ignored if
/// called more than once.
//
void
TWindow::Init(HWND handle, TModule* module)
{
  PRECONDITION(handle);

  if (!InstanceProc) {
    // Perform preliminary initialization
    //
    SetHandle(handle);
    SetCaption(LPCTSTR(nullptr));

    // If we've been given a module, then setup that and the app now if
    // possible so that GetWindowPtr below has some context. Otherwise at least
    // set it to 0.
    //
    Application = TYPESAFE_DOWNCAST(module,TApplication);

    // If the receiver's parent is an OWL window then pass the window to
    // PerformInit so the receiver can be added to the parent's list of children.
    //
    HWND   hParent = GetParentH();
    TWindow*  parent = hParent ? GetWindowPtr(hParent) : nullptr;
    PerformInit(parent, module);

    // Install the instance window proc.
    //
    SubclassWindowFunction();

    // Copy over the title, styles, the coordinates & the id
    //
    GetWindowTextTitle();
    GetHWndState(true); //DLN now we force resysnc of style settings for non-owl windows.

    // Keep track that this is an alias object & that it is already created
    //
    SetFlag(wfAlias | wfFullyCreated);

    // Unique id may have been set inadvertantly to TWindow by the above
    // GetWindowTextTitle, et. al. Reset it just in case
    //
    SetUniqueId();
  }
}

//
// Private initializer function that does the bulk of the work for the
// protected Init()s
//
void
TWindow::PerformInit(TWindow* parent, TModule* module)
{
  // Initialize members.
  //
  ZOrder = 0;
  ChildList = nullptr;
  Flags = wfDeleteOnClose;
  TransferBuffer = nullptr;
  TransferBufferSize = 0;
  Parent = parent;
  Attr.Param = nullptr;
  Attr.Menu = 0;
  Attr.AccelTable = 0;
  HAccel = nullptr;
  Scroller = 0;
  ContextPopupMenu = nullptr;
  Tooltip = nullptr;
  CursorModule = nullptr;
  CursorResId = 0;
  HCursor = nullptr;
  BkgndColor = TColor::None;
  TextColor = TColor::None;
  Font.reset();

  // Set the window proc for this window instance.
  //
  InstanceProc = CreateInstanceProc();

  // Link to parent.
  //
  if (Parent)
    Parent->AddChild(this);
  else
    SiblingList = nullptr;

  // Use passed module if one, else get parent's. If no parent, use app
  //
  if (Parent) {
    Application = Parent->GetApplication();
    Module = module ? module : Parent->GetModule();
  }
  else {
    Module = module ? module : nullptr;
    Application = TYPESAFE_DOWNCAST(Module,TApplication);
    if (!Application)
    {
      //Application = OWLGetAppDictionary().GetApplication(0);
      Application = GetApplicationObject();
      if (!Module)
        Module = Application;
    }
    CHECK(Application);
  }
  SetUniqueId();

  TRACEX(OwlWin, 1, _T("TWindow constructed @") << (void*)this << *this);
}

namespace
{

//
// Function prototype
//
void CacheFlush(UINT_PTR id);

}

//
/// Destroys this window (unless this is an alias) and the children.
/// First, restores the underlying window's WindowProc (i.e. reverts the subclassing).
/// Then, destroys the children and, in turn, the underlying window (unless aliased).
/// Finally, dissociates this from the parent and application, and deallocates acquired resources.
//
TWindow::~TWindow()
{
  // Clean up the instance WindowProc early; we're in the destructor, so we don't want to handle
  // any more messages.
  //
  if (GetHandle())
  {
    WARNX(OwlWin, GetWindowProc() != GetInstanceProc(), 0,
      _T("Restoring old WndProc after foreign subclass of:") << *this);
    SetWindowProc(DefaultProc ? DefaultProc : &::DefWindowProc);
  }
  FreeInstanceProc();
  InstanceProc = nullptr;

  // Flush the cache, so that messages will not be dispatched to a destructed derived class.
  //
  owl::CacheFlush(UniqueId);

  // Destroy the children first, so that they get a chance to clean up.
  //
  // Note that the old implementation deleted the child while traversing the linked list of
  // children. However, while the implementation of the iteration may be written to allow this, it
  // is poor programming style to assume that iterators remain valid while mutating the underlying
  // structure. So, to make the code less brittle, we now collect the children into a vector before
  // deleting them.
  //
  // Also note that the code here assumes that the child was dynamically allocated. If not, the
  // child must be taken out of the children list before the parent is destructed.
  //
  auto c = vector<TWindow*>{};
  for (auto& w : GetChildren())
    c.push_back(&w);
  for (auto w : c)
    try 
    { 
      w->Destroy(); 
      delete w;
    }
    catch (...)
    {
      TRACEX(OwlWin, 0, _T("~TWindow: Swallowed exception while destroying child: ") << *w);
    }

  // If the handle is still around, and this is not an alias, then destroy the window.
  //
  if (GetHandle() && !IsFlagSet(wfAlias))
  {
    WARNX(OwlWin, GetHandle(), 0, _T("Destroying from TWindow::~TWindow: ") << *this);
    try { Destroy(); }
    catch (...)
    {
      TRACEX(OwlWin, 0, _T("~TWindow: Swallowed exception while destroying this: ") << *this);
    }
  }

  // Remove external references to this.
  //
  if (Parent)
    Parent->RemoveChild(this);
  if (Application)
    Application->Uncondemn(this);

  // Deallocate resources.
  //
  SetCaption(nullptr);
  SetCursor(nullptr, TResId{0});
  SetScroller(nullptr);
  delete ContextPopupMenu;
  if (Attr.Menu.IsString())
    delete[] Attr.Menu.GetString();

  TRACEX(OwlWin, 1, _T("TWindow destructed @") << (void*)this << *this);
}

#if defined(BI_MULTI_THREAD_RTL)
//
/// Cracks and dispatches a TWindow  message. The info parameter is the
/// event-handling function. The wp and lp parameters are the message parameters the
/// dispatcher cracks.
/// \note Overrides TEventHandler::Dispatch() to handle multi-thread synchronization
//
TResult TWindow::Dispatch(TEventInfo& info, TParam1 p1, TParam2 p2)
{
  TApplication::TQueueLock lock(*GetApplication());
  return TEventHandler::Dispatch(info, p1, p2);
}

#endif

//
/// Called from TApplication::ProcessAppMsg() to give the window an
/// opportunity to perform preprocessing of the Windows message
///
/// If you return true, further processing of the message is halted
///
/// Allows preprocessing of queued messages prior to dispatching. If you override
/// this method in a derived class, be sure to call the base class's PreProcessMsg
/// because it handles the translation of accelerator keys.
//
bool
TWindow::PreProcessMsg(MSG& msg)
{
  PRECONDITION(GetHandle());

  // Check if this message might be worth relaying to the tooltip window
  //
  TTooltip* tooltip = GetTooltip();
  if (tooltip && tooltip->IsWindow()) {
    if (msg.hwnd == *this || IsChild(msg.hwnd)) {
      if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
        tooltip->RelayEvent(msg);
      }
    }
  }

  return HAccel ? ::TranslateAccelerator(GetHandle(), HAccel, &msg) : false;
}

//
/// Called when no messages are waiting to be processed, IdleAction performs idle
/// processing as long as true is returned. idleCount specifies the number of times
/// idleAction has been called between messages.
///
/// Propagate idle action to all children if count==0, and to any children that
/// previously said they wanted more time.
//
bool
TWindow::IdleAction(long idleCount)
{
  bool wantMore = false;
  TWindow* win = GetFirstChild();
  if (win) {
    do {
      if (idleCount == 0 || win->IsFlagSet(wfPropagateIdle)) {
        if (win->IdleAction(idleCount)) {
          win->SetFlag(wfPropagateIdle);
          wantMore = true;
        }
        else {
          win->ClearFlag(wfPropagateIdle);
        }
      }
      win = win->Next();
    } while (win && win != GetFirstChild());
  }
  return wantMore;
}

//
/// Respond to WM_SYSCOLORCHANGE by broadcasting it to all children
//
void
TWindow::EvSysColorChange()
{
  ChildBroadcastMessage(WM_SYSCOLORCHANGE);
  DefaultProcessing();
}

//
/// Removes a child window. Uses the ObjectWindows list of objects rather the
/// Window's HWND list.
//
void
TWindow::RemoveChild(TWindow* child)
{
  if (child && ChildList) {
    TWindow*  lastChild = ChildList;
    TWindow*  nextChild = lastChild;

    while (nextChild->SiblingList != lastChild &&
           nextChild->SiblingList != child) {
      nextChild = nextChild->SiblingList;
    }

    if (nextChild->SiblingList == child) {
      if (nextChild->SiblingList == nextChild)
        ChildList = nullptr;

      else {
        if (nextChild->SiblingList == ChildList)
          ChildList = nextChild;

        nextChild->SiblingList = nextChild->SiblingList->SiblingList;
      }
    }
  }
}

//
/// Sets the parent for the specified window by setting Parent to the specified new
/// Parent window object. Removes this window from the child list of the previous
/// parent window, if any, and adds this window to the new parent's child list.
//
void
TWindow::SetParent(TWindow* newParent)
{
  if (Parent != newParent) {
    if (Parent)
      Parent->RemoveChild(this);

    SiblingList = nullptr;

    Parent = newParent;

    if (Parent)
      Parent->AddChild(this);
  }
  // Tell Windows about the change too, if appropriate
  //
  if (GetHandle() && Parent && GetParentH() != Parent->GetHandle()) {
    if (newParent) {
      if (newParent->GetHandle())
        ::SetParent(GetHandle(), newParent->GetHandle());
    }
    else
      ::SetParent(GetHandle(), nullptr);
  }
}

//
/// Default behavior for updating document title is to pass it to parent frame
///
/// Stores the title of the document (docname). index is the number of the view
/// displayed in the document's caption bar. In order to determine what the view
/// number should be, SetDocTitle makes two passes: the first pass checks to see if
/// there's more than one view, and the second pass, if there is more than one view,
/// assigns the next number to the view. If there is only one view, index is 0;
/// therefore, the document does not display a view number. When TDocument is
/// checking to see if more than one view exists, index is -1. In such cases, only
/// the document's title is displayed in the caption bar.
/// SetDocTitle returns true if there is more than one view and TDocument displays
/// the number of the view passed in index.
//
bool
TWindow::SetDocTitle(LPCTSTR docname, int index)
{
  return Parent ? Parent->SetDocTitle(docname, index) : false;
}

//
/// Sets the scroller object for this window. This window assumes ownership of the
/// scroller object, and will delete it when done and on subsequent sets.
//
void
TWindow::SetScroller(TScroller* scroller)
{
  // This temporary variable is needed, because TScroller::~TScroller() calls TWindow::SetScroller(0)
  TScroller* temp = Scroller;
  Scroller = scroller;
  delete temp;
}

//
// Wildcard check used for child id notifications
//
static bool wildcardCheck(const TGenericTableEntry & entry, const TEventHandler::TEventInfo& info)
{
  return entry.Id == info.Id && (entry.Msg == UINT_MAX || entry.Msg == info.Msg);
}

//
/// Handles default processing of events, which includes continued processing
/// of menu/accelerators commands and enablers, as well as notifications
//
/// Serves as a general-purpose default processing function that handles a variety
/// of messages. After being created and before calling DefaultProcessing, however,
/// a window completes the following sequence of events (illustrated in the Default
/// Message-Processing Flowchart).
/// - If the window is already created, SubclassWindowFunction is used to
/// install a thunk in place of the window's current procedure. The previous
/// window procedure is saved in DefaultProc.
/// - If the window has not been created, InitWndProc is set up as the
/// window proc in the class. Then, when the window first receives a message,
/// InitWndProc calls GetWindowProc to get the window's thunk (created by the
/// constructor by calling CreateInstanceProc). InitWndProc then switches the
/// the window procedure to the thunk.
/// - After this point, the thunk responds to incoming messages by calling ReceiveMessage
/// which calls the virtual function WindowProc to process the message. ObjectWindows uses
/// the special registered message GetWindowPtrMsgId to get the this-pointer of an HWND.
/// ReceiveMessage responds to this message by returning the this pointer obtained
/// from the thunk.
///
/// If the incoming message is not a command or command enable message, WindowProc
/// immediately searches the window's response table for a matching entry. If the
/// incoming message is a command or command enable message, WindowProc calls
/// EvCommand or EvCommandEnable. EvCommand and EvCommandEnable begin searching for
/// a matching entry in the focus window's response table. If an entry is found, the
/// corresponding function is dispatched; otherwise ObjectWindows calls
/// DefaultProcessing to finish the recursive walk back up the parent chain,
/// searching for a match until the receiving window (the window that initially
/// received the message) is reached. At this point, one of the following actions
/// occurs:
/// - If there is still no match and this is the MainWindow of the
/// application, the window searches the application's response table.
/// - If there are no matches and this is a command, DefWindowProc is
/// called.
/// - If this is a CommandEnable message, no further action is taken.
/// - If this is not a command, and if a response table entry exists for
/// the window, WindowProc dispatches the corresponding EvXxxx function to handle
/// the message.
/// - If this is the application's MainWindow, and the message is
/// designed for the application, the message is forwarded to the application.
/// - For any other cases, the window calls DefWindowProc.
///
/// The following diagram illustrates this sequence of message-processing events:
/// \image html bm282.BMP
//
TResult
TWindow::DefaultProcessing()
{
  TCurrentEvent& currentEvent = GetCurrentEvent();

  if (currentEvent.Message == WM_COMMAND_ENABLE) {
    if (currentEvent.Win != this) {
      TWindow*         receiver = Parent ? Parent : currentEvent.Win;
      TCommandEnabler& commandEnabler = *reinterpret_cast<TCommandEnabler*>(currentEvent.Param2);
      TEventInfo       eventInfo(WM_COMMAND_ENABLE, commandEnabler.GetId());

      if (receiver->Find(eventInfo))
        return receiver->Dispatch(eventInfo, 0, currentEvent.Param2);
    }

    return 0;
  }

  // Handle normal message default processing by routing directly to the
  // virtual DefWindowProc() for the window.
  //
  if (currentEvent.Message != WM_COMMAND && currentEvent.Message != WM_NOTIFY)
    return DefWindowProc(currentEvent.Message,
                         currentEvent.Param1,
                         currentEvent.Param2);

  // currentEvent.Message == WM_COMMAND or WM_NOTIFY
  //
  uint    notifyCode;
  uint    id;
  HWND hWndCtl;

  // Get notify code, control id and control handle from packed params.
  // Unpacking based on message & platform.
  //
  if (currentEvent.Message == WM_COMMAND) {
    notifyCode = HiUint16(currentEvent.Param1);
    id = LoUint16(currentEvent.Param1);
    hWndCtl = reinterpret_cast<HWND>(currentEvent.Param2);
  }
  else {
    TNotify& _not = *reinterpret_cast<TNotify*>(currentEvent.Param2);
    notifyCode = _not.code;
    id = static_cast<uint>(_not.idFrom); //currentEvent.Param1; // Y.B. In help written -> use not.idFrom.
    if (_not.code == static_cast<uint>(TTN_NEEDTEXT) &&
        (reinterpret_cast<TTooltipText*>(&_not))->uFlags&TTF_IDISHWND){
      id = ::GetDlgCtrlID(reinterpret_cast<HWND>(_not.idFrom));
    }
    hWndCtl = _not.hwndFrom;
  }

  // If all special routing is done, then process the command/notify for this
  // window
  //
  if (currentEvent.Win == this) {
    // Menu command originally destined for the receiver, defer to the app.
    //
    if (hWndCtl == nullptr) {
      TEventInfo     eventInfo(0, id);
      TApplication*  app = GetApplication();

      if (app->Find(eventInfo)) {
        app->Dispatch(eventInfo, eventInfo.Id);
        return 0;
      }
      WARNX(OwlMsg, notifyCode<=1, 0, _T("Unprocessed WM_COMMAND(id=") << id << _T(")"));
    }
    // Originally destined for the receiver and the receiver has called us so
    // default processing can occur.
    // Unpack the original parameters and call DefWindowProc()
    //
    return DefWindowProc(currentEvent.Message, currentEvent.Param1, currentEvent.Param2);
  }

  // Perform special routing for commands and notifications
  //
  else {
    TWindow*        receiver;
    TEqualOperator  equal = 0;

    if (hWndCtl == nullptr) {
      // Menu/accelerator/push button
      // Either give the message to the receiver's parent or the original
      // receiver of the message
      //
      receiver = Parent ? Parent : currentEvent.Win;

      // "notifyCode" is either 0 or 1 depending on whether this is from an
      // accelerator; however, we want to explicitly look for a 0...
      //
      notifyCode = 0;
    }
    else {
      // Child id notification (either WM_COMMAND or WM_NOTIFY) sent to the
      // child and the child has called us.
      // Offer the parent an opportunity to handle the notification
      //   NOTE: use equal function that will do a wildcard search
      //
      equal = wildcardCheck;
      receiver = currentEvent.Win;

      // The child window identifier shouldn't be 0, but if it is then it will
      // look like we are searching for a WM_ message with value "notifyCode",
      // in that case just give up and call DefWindowProc() for the window.
      //
      if (receiver->IsFlagSet(wfAlias) || id == 0)
        return receiver->DefWindowProc(currentEvent.Message,
                                       currentEvent.Param1,
                                       currentEvent.Param2);
    }

    // Now dispatch the command or notification to the receiver window
    //
    TEventInfo  eventInfo(notifyCode, id);

    // NOTE: The ResponseTableEntry of handlers which expect an id (e.g.
    //       EV_COMMAND_AND_ID) have a zero in their first field. The
    //       ResponseTableEntry of handlers which expect a notification code
    //       (e.g. EV_CHILD_NOTIFY_AND_CODE and EV_NOTIFY_ALL_CODES) contain
    //       either the notifyCode or UINT_MAX in that field. Hence, we can
    //       dispatch the expected information based on the contents of that
    //       field.
    //
    //       However, this logic will fail to disambiguate cases where a
    //       notification code is 0. The only standard control with such a
    //       notification is BUTTON /w BN_CLICKED. So for button handlers
    //       expecting the id, you can use EV_COMMAND_AND_ID. For handlers
    //       expecting the notification code, you can use EV_NOTIFY_ALL_CODES,
    //       (*NOT* EV_CHILD_NOTIFY_AND_CODE(Id, BN_CLICKED, xxxx,...))
    //
    if (receiver->Find(eventInfo, equal)) {
      TParam1 param1 = eventInfo.Entry->NotifyCode ? notifyCode : id;
      return receiver->Dispatch(eventInfo, param1, currentEvent.Param2);
    }
    else
      return receiver->DefaultProcessing();
  }
}

//
/// WindowProc calls EvCommand to handle WM_COMMAND messages. id is the identifier
/// of the menu item or control. hWndCtl holds a value that represents the control
/// sending the message. If the message is not from a control, it is 0. notifyCode
/// holds a value that represents the control's notification message. If the message
/// is from an accelerator, notifyCode is 1; if the message is from a menu,
/// notifyCode is 0.
//
TResult
TWindow::EvCommand(uint id, HWND hWndCtl, uint notifyCode)
{
  TRACEX(OwlCmd, 1, _T("TWindow::EvCommand - id(") << id << _T("), ctl(") <<\
                     hex << reinterpret_cast<void*>(hWndCtl) << _T("), code(") << notifyCode  << _T(")"));

  TWindow*        receiver = this;
  TEqualOperator  equal = 0;

  // Menu/accelerator
  //
  if (hWndCtl == nullptr) {
    // "notifyCode" is either 0 or 1 depending on whether this is from an
    // accelerator; however, we want to explicitly look for a 0 in the tables
    //
    notifyCode = 0;
  }

  // Child id notification
  //
  else {
    TWindow*  child = GetWindowPtr(hWndCtl);

    if (child) {
      // Give the child first crack at the event
      //
      receiver = child;
      id = UINT_MAX;  // special designator for child Id notifications that are
                      // handled at the child
    }
    else {
      // Offer the parent an opportunity to handle the notification
      //
      // NOTE: use equal function that will do a wildcard search
      //
      equal = wildcardCheck;

      // The child window identifier shouldn't be 0, but if it is then it will
      // look like we are searching for a WM_ message with value "notifyCode"
      //
      if (id == 0)
        return DefaultProcessing();
    }
  }

  TEventInfo  eventInfo(notifyCode, id);

  // NOTE: The ResponseTableEntry of handlers which expect an id
  //       (e.g. EV_COMMAND_AND_ID) have a zero in their first field.
  //       The ResponseTableEntry of handlers which expect a notification
  //       code (e.g. EV_CHILD_NOTIFY_AND_CODE and EV_NOTIFY_ALL_CODES)
  //       contain either the notifyCode or UINT_MAX in that field.
  //       Hence, we can dispatch the expected information based on the
  //       contents of that field.
  //
  //       However, this logic will fail to disambiguate cases where
  //       a notification code is 0. The only standard control with
  //       such a notification is BUTTON /w BN_CLICKED. So for button
  //       handlers expecting the id, you can use EV_COMMAND_AND_ID.
  //       For handlers expecting the notification code, you can use
  //       EV_NOTIFY_ALL_CODES.
  //
  //       Do *NOT* use "EV_CHILD_NOTIFY_AND_CODE(Id, BN_CLICKED, xxxx,...)"
  //
  if (receiver->Find(eventInfo, equal))
    return receiver->Dispatch(eventInfo,
                              eventInfo.Entry->NotifyCode ? notifyCode : id);
  else
    return receiver->DefaultProcessing();
}

//
/// Handles WM_NOTIFY and subdispatch messages from child controls.  This is the
/// default message handler for WM_NOTIFY.
// !CQ NOTE: similarity between EvCommand child notifies--merge?
//
TResult
TWindow::EvNotify(uint ctlId, TNotify& notifyInfo)
{
  // Intercept requests for tooltip texts and turn the request into
  // a command enabler. This mechanism allows us to route the request
  // the same way commands are routed in OWL. Therefore, the object that
  // handles a command is the object that gets first crack at providing
  // the tip text for that command.
  //
  if (notifyInfo.code == TTN_NEEDTEXT)
  {
    auto& ttText = reinterpret_cast<TTooltipText&>(notifyInfo);
    auto enabler = TTooltipEnabler{ttText, GetHandle()};
    const auto cmdTarget = GetParent() ? GetParent() : this;
    RouteCommandEnable(cmdTarget->GetHandle(), enabler);
    const auto handled = enabler.GetHandled();
    WARNX(OwlWin, !handled, 1, _T("No command enabler found for TTN_NEEDTEXT (Id: ") << enabler.GetId() << _T(")."));
    if (handled)
      return 0;
  }

  TWindow*        receiver = this;
  TEqualOperator  equal = 0;
  TWindow*         child = GetWindowPtr(notifyInfo.hwndFrom);

  if (child) {
    // Give the Owl child first crack at the event
    //
    receiver = child;
    ctlId = UINT_MAX;// special designator for child Id notifications that are
                     // handled at the child // !CQ may need a different value
  }
  else {
    // Offer the parent an opportunity to handle the notification
    //   NOTE: use equal function that will do a wildcard search
    //
    equal = wildcardCheck;

    // The child window identifier shouldn't be 0, but if it is then it will
    // look like we are searching for a WM_ message with value "notifyCode"
    //
    if (ctlId == 0)
      return DefaultProcessing();
  }

  TEventInfo  eventInfo(notifyInfo.code, ctlId);

  // Pass the "notifyCode" along in case the user wants it...
  //
  if (receiver->Find(eventInfo, equal))
    return receiver->Dispatch(eventInfo, notifyInfo.code, TParam2(&notifyInfo));
  else
    return receiver->DefaultProcessing();
}

//
/// Called by WindowProc to handle WM_COMMAND_ENABLE messages, EvCommandEnable calls
/// the CmXxxx command-handling function or calls DefaultProcessing to handle the
/// incoming message.
//
void
TWindow::EvCommandEnable(TCommandEnabler& commandEnabler)
{

// code copied from old unix owl (JAM 04-16-01)
//DLN test replace of DispatchMsg for CC 5.1
  TEventInfo eventInfo(WM_COMMAND_ENABLE,commandEnabler.GetId());
  if (Find(eventInfo))
    Dispatch(eventInfo,0,TParam2(&commandEnabler));

//  DispatchMsg(WM_COMMAND_ENABLE, commandEnabler.Id, 0, TParam2(&commandEnabler));
}

//
/// Walks the chain of windows from the initial target window to this window. If it
/// finds a window to receive the message, RouteCommandEnable dispatches the command
/// enabler to that window. hInitCmdTarget is the handle to the initial command
/// target window, which can be focus window but does not need to be. ce is a
/// reference to the command enabler.
/// Other classes use this function to perform particular command enabling tasks:
/// For example, TFrameWindow calls RouteCommandEnable to perform the majority of
/// its menu command enabling tasks. When it is an embedded window, TOleWindow also
/// uses RouteCommandEnable to perform command enabling.
///
/// Don't process for windows out of our window tree (esp. other apps)
//
void
TWindow::RouteCommandEnable(HWND hInitCmdTarget, TCommandEnabler& commandEnabler)
{
  // Extra processing for commands & commandEnablers: send the command down the
  // command chain if we are the original receiver
  //
  if (commandEnabler.IsReceiver(*this)) {
    HWND  hCmdTarget = hInitCmdTarget;
    while (hCmdTarget && hCmdTarget != *this) {
      TWindow*  cmdTarget = GetWindowPtr(hCmdTarget);

      if (cmdTarget) {
        cmdTarget->EvCommandEnable(commandEnabler);

        if (commandEnabler.GetHandled())
          return;
      }
      hCmdTarget = ::GetParent(hCmdTarget);
    }
  }

  // Always call base handler
  //
  TWindow::EvCommandEnable(commandEnabler);

  // No one explicitly enabled/disabled the command via the enabler, so run up
  // the command chain checking for any one who is going to handle the command
  // itself; if not then disable it...
  // Don't do this for command senders that don't actually generate commands,
  // like popup menu items.
  //
  if (commandEnabler.IsReceiver(*this) && !commandEnabler.GetHandled()
      && commandEnabler.SendsCommand()) {
    bool        enable = false;
    TEventInfo  eventInfo(0, commandEnabler.GetId());

    HWND  hCmdTarget = hInitCmdTarget;
    while (true) {
      TWindow*  cmdTarget = GetWindowPtr(hCmdTarget);

      if (cmdTarget && cmdTarget->Find(eventInfo)) {
        enable = true;  // command will be handled, leave sender alone
        break;
      }
      if (!hCmdTarget || hCmdTarget == *this)
        break;

      hCmdTarget = ::GetParent(hCmdTarget);
    }

    if (!enable) {
      // Check if the app wants to handle it
      //
      TEventInfo    enableInfo(WM_COMMAND_ENABLE, commandEnabler.GetId());
      TApplication* app = GetApplication();
      if (app->Find(enableInfo)) {
        app->Dispatch(enableInfo, 0, TParam2(&commandEnabler));
        if (commandEnabler.GetHandled())
          return;
      }
      enable = app->Find(eventInfo);
    }

    commandEnabler.Enable(enable);
  }
}

//
/// Virtual function provides final default processing for an incoming message
/// Calls original window proc that was subclassed, using ::CallWindowProc to
/// make sure that registers get setup correctly.
//
/// Performs default Windows processing and passes the incoming Windows message. You
/// usually do not need to call this function directly. Classes such as TMDIFrame
/// and TMDIChild override this function to perform specialized default processing.

TResult
TWindow::DefWindowProc(TMsgId message, TParam1 param1, TParam2 param2)
{
  PRECONDITION(DefaultProc);

  bool priorException = Application && Application->HasSuspendedException();
  TResult result = ::CallWindowProc(DefaultProc, GetHandle(), message, param1, param2);
  if (!priorException) // Don't rethrow exceptions if we are in a clean-up phase.
    GetApplication()->ResumeThrow();
  return result;
}

//namespace owl {

/// \addtogroup internal
/// @{


//
// Message cache
//
static const int     msgCacheSize = 31;
struct TCacheEntry{
  typedef UINT_PTR TId;
  TId UniqueId;
  const TGenericTableEntry* Entry;
  TMsgId                         Msg;
  int                          Delta;  // adjustment to "this" pointer

  //30.05.2007 - Submitted by Frank Rast: Initialization of TCacheEntry data members was missing.
  TCacheEntry() : UniqueId(0), Entry(nullptr), Msg(0), Delta(0) {}

  void Set(TId uniqueId, TMsgId msg, const TGenericTableEntry* entry, int delta = 0) {
    UniqueId = uniqueId;
    Entry = entry;
    Msg = msg;
    Delta = delta;
  }
  bool  Hit(TMsgId msg, TId uniqueId) {return msg == Msg && uniqueId == UniqueId;}

  static int Key(TId id, TMsgId msg) {return (id ^ static_cast<TId>(msg)) % msgCacheSize;}
};


struct TCacheEntryStr
#if defined(BI_MULTI_THREAD_RTL)
: public TLocalObject
#endif
{
  TCacheEntryStr():Enabled(true)
    {
    }
  ~TCacheEntryStr()
    {
    }

  TCacheEntry  Cache[msgCacheSize];
  bool         Enabled;

  void CacheFlush(TCacheEntry::TId id);
  void Set(int index, TCacheEntry::TId uniqueId, TMsgId, const TGenericTableEntry* entry, int delta = 0);
  bool Check(int index, TMsgId, TCacheEntry::TId id);

#if defined(BI_MULTI_THREAD_RTL)
//  TMRSWSection  Lock;
#endif
};

TCacheEntry::TId TWindow::LastUniqueId = 0;

static TCacheEntryStr& GetCache()
{
#if defined(BI_MULTI_THREAD_RTL)
  static TTlsContainer<TCacheEntryStr> cacheEntry;
  return cacheEntry.Get();
#else
  static TCacheEntryStr cacheEntry;
  return cacheEntry;
#endif
};

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TCacheEntryStr& InitCacheEntryStr = GetCache();
}

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKCACHE //TMRSWSection::TLock Lock(GetCache().Lock);
#else
#define LOCKCACHE
#endif
//
void TCacheEntryStr::CacheFlush(TCacheEntry::TId id)
{
  LOCKCACHE
  for (int i = 0; i < msgCacheSize; i++)
    if (Cache[i].UniqueId == id)
      Cache[i].Msg = 0;
}
//
void TCacheEntryStr::Set(int index, TCacheEntry::TId uniqueId, TMsgId msg, const TGenericTableEntry* entry, int delta)
{
  LOCKCACHE
  Cache[index].Set(uniqueId, msg, entry, delta);
}
//
bool TCacheEntryStr::Check(int index, TMsgId msg, TCacheEntry::TId id)
{
  LOCKCACHE
  return Enabled && Cache[index].Hit(msg, id);
}

namespace
{

//
void CacheFlush(UINT_PTR id)
{
  GetCache().CacheFlush(id);
}

}

/// @}

//
// If rtti is available, then use it get an id for this window that is unique
// on a per-class basis.
//
// Without rtti, use a per-instance unique id. Less efficient, but safe.
//
void
TWindow::SetUniqueId()
{
#if defined(OWL_RTTI_MSGCACHE)
  UniqueId = 0;
#else
  if (++LastUniqueId == 0) {
    //
    // numbers wrapped around. disable the cache to be safe...
    //
    LastUniqueId = 1;
    GetCache().Enabled = false;
  }
  UniqueId = LastUniqueId;
#endif
}

//
/// Dispatches the given message using the response table. Similar to SendMessage
/// but goes directly to the OWL window, bypassing the Windows message queue.
//
TResult
TWindow::HandleMessage(TMsgId msg, TParam1 p1, TParam2 p2)
{
  // ReceiveMessage suspends any exception, so we need to rethrow it after the call.
  //
  TResult r = ReceiveMessage(Handle, msg, p1, p2);
  GetApplication()->ResumeThrow();
  return r;
}

//
/// First virtual function called to handling incoming messages to a TWindow
//
/// Processes incoming messages by calling EvCommand to handle WM_COMMAND messages,
/// EvCommandEnable to handle WM_COMMAND_ENABLE messages, and dispatching for all
/// other messages.
//
TResult
TWindow::WindowProc(TMsgId msg, TParam1 param1, TParam2 param2)
{
  PRECONDITION(GetHandle());  // Should never get here without a handle

  // Handle WM_COMMAND_ENABLE command enable msgs by directly calling the
  // virtual EvCommandEnable
  //
  if (msg == WM_COMMAND_ENABLE) {
    TRACEX(OwlMsg, 1, _T("WM_COMMAND_ENABLE") << _T("! => ") << *this);
    EvCommandEnable(*reinterpret_cast<TCommandEnabler*>(param2));
    return 0;
  }
  // Handle WM_COMMAND command msgs by directly calling the
  // virtual EvCommand
  //
  else if (msg == WM_COMMAND) {
    TRACEX(OwlMsg, 1, _T("WM_COMMAND, ") << LoUint16(param1) << _T(" ! => ") << *this);
    return EvCommand(LoUint16(param1), reinterpret_cast<HWND>(param2), HiUint16(param1));
  }
  // Handle WM_NOTIFY msgs by directly calling the virtual EvNotify
  // !CQ why not use response table dispatch? For above too?
  //
  else if (msg == WM_NOTIFY) {

    // We've received reports of some controls (out there) sending
    // WM_NOTIFY with a NULL LPARAM !!
    //
    TNotify dumbNMHDR;
    //DLN SET ID to 0 for these NULL LPARAM to avoid bad resource access
    dumbNMHDR.idFrom = static_cast<UINT>(-1);
    TNotify* notify = param2 ? reinterpret_cast<TNotify*>(param2) : &dumbNMHDR;

    TRACEX(OwlMsg, 1, _T("WM_NOTIFY, ") << notify->idFrom   << _T(", ")\
                                    << notify->code     << _T(", ")\
                                    << hex << reinterpret_cast<void*>(notify->hwndFrom) << _T(", ")\
                                    << _T(" ! => ") << *this);
    return EvNotify(static_cast<uint>(param1), *notify);
  }

  // Handle all other messages by looking up and dispatching using the
  // response tables
  //
  else {
#if defined(OWL_RTTI_MSGCACHE)
    if (!UniqueId)
      UniqueId = TYPE_UNIQUE_ID(*this);
#endif
    int key = TCacheEntry::Key(UniqueId, msg);
    TEventInfo  eventInfo(msg);

    // Check the cache. A cache hit may be an RT handler, or an RT miss.
    //
    TCacheEntryStr& cache = GetCache();
    if(cache.Check(key, msg, UniqueId)) {
      eventInfo.Entry = cache.Cache[key].Entry;
      if (eventInfo.Entry) {
        TRACEX(OwlMsg, 1, TMsgName(msg) << _T("* => ") << *this);
        eventInfo.Object = reinterpret_cast<TGeneric*>(((reinterpret_cast<char*>(this)) + cache.Cache[key].Delta));
        return Dispatch(eventInfo, param1, param2);

      } // else fall out & do default below
    }
    // Perform the lookup on this window.
    //
    else if (this->Find(eventInfo)) {
      TRACEX(OwlMsg, 1, TMsgName(msg) << _T("  => ") << *this);
      const int delta = static_cast<int>(reinterpret_cast<INT_PTR>(eventInfo.Object) - reinterpret_cast<INT_PTR>(this));
      cache.Set(key,UniqueId, msg, eventInfo.Entry, delta);
      return Dispatch(eventInfo, param1, param2);
    }
    else  // not found
      cache.Set(key,UniqueId, msg, nullptr);  // Cache no-handler entries too

    // Behavior for messages that have no handler. If this is the main window,
    // then give the app a chance to handle the message. If not the main
    // window, or if the app had no handler, just call DefWindowProc() to
    // pass the message back to whomever we subclassed
    //
    if (IsFlagSet(wfMainWindow)) {
      TEventInfo cmdEventInfo(msg, static_cast<uint>(param1));
      if (GetApplication()->Find(cmdEventInfo))
        return GetApplication()->Dispatch(cmdEventInfo, param1, param2);
      if (GetApplication()->Find(eventInfo))
        return GetApplication()->Dispatch(eventInfo, param1, param2);
    }
    return DefWindowProc(msg, param1, param2);
  }
}

#if !defined(BI_COMP_GNUC)
#pragma warn -par
#endif

//
/// Handles messages sent to the window.
/// May be called directly by the windows message dispatch mechanism,
/// or manually via HandleMessage.
/// If the message is a GetWindowPtr message, it is handled immediately,
/// otherwise the current event is saved and the call forwarded to WindowProc.
/// Any unhandled exception in WindowProc is suspended (see TApplication::SuspendThrow).
//
TResult
TWindow::ReceiveMessage(HWND hwnd, TMsgId msg, TParam1 param1, TParam2 param2) noexcept
{
  WARNX(OwlWin, hwnd != Handle, 0, "ReceiveMessage: The passed handle does not match this window.");
  InUse(hwnd);

  // If it's a "GetWindowPtr" message, then return pointer to this window
  //
  if (msg == GetWindowPtrMsgId && (!param2 || param2 == TParam2(Application)))
    return TResult(this);

  // Save event away in "CurrentEvent"
  //
  TCurrentEvent& currentEvent = GetCurrentEvent();
  TCurrentEvent  saveEvent = currentEvent;  // for nested calls
  currentEvent.Win = this;
  currentEvent.Message = msg;
  currentEvent.Param1 = param1;
  currentEvent.Param2 = param2;

  // Call window object's WindowProc virtual function
  //
  TResult result = 0;
  try
  {
    result = WindowProc(msg, param1, param2);
  }
  catch (...)
  {
    TRACEX(OwlWin, 0, _T("TWindow::ReceiveMessage: Suspending unhandled exception for message: ") << msg);

    GetApplication()->SuspendThrow(std::current_exception());
  }
  currentEvent = saveEvent;  // restore previous event to current event
  return result;
}

#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

//
/// Determine the object pointer by sending a GetWindowPtrMsgId message to the window.
/// When TWindow::ReceiveMessage receives this message it returns a pointer to the object.
/// If app is non-zero, then the process makes sure that the corresponding
/// TWindow is returned.
//
_OWLFUNC(TWindow*) GetWindowPtr(HWND hWnd, const TApplication* app)
{
  if (!hWnd /* && ::IsWindow(hWnd) */)  // Could also check handle validity
    return nullptr;

  TParam2 param2 = TParam2(app);

  // Avoid SendMessage to cutdown the overhead & message spy tool flooding
  //
  // Under Win32 don't even try if it is not our process. Some Win32's will
  // return a wndProc that crashes.
  //
  DWORD processId;
  ::GetWindowThreadProcessId(hWnd, &processId);
  if (processId != ::GetCurrentProcessId())
    return nullptr;

  WNDPROC wndProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC));
  if (!wndProc)
    return nullptr;
  return reinterpret_cast<TWindow*>(::CallWindowProc(wndProc, hWnd, GetWindowPtrMsgId, 0, param2));
}

//
/// Response method for an incoming WM_CREATE message
///
/// Performs setup and data transfer now that we are created & have a Handle.
/// Should return true if the
//
bool
TWindow::EvCreate(CREATESTRUCT &)
{
  PerformSetupAndTransfer(); // TODO: Add exception handling and return false on exceptions.
  TResult r = DefaultProcessing();
  return r == 0;
}

//
/// Responds to a request by a child window to hold its HWND when it is losing
/// focus. Stores the child's HWND in HoldFocusHwnd.
//
/// \note Regular windows never hold focus child handles--just say no.
//
bool
TWindow::HoldFocusHWnd(HWND /*hWndLose*/, HWND /*hWndGain*/)
{
  return false;
}

//
/// Handle WM_KILLFOCUS so that we can have a parent window hold onto our
/// Handle and possibly restore focus later.
//
void
TWindow::EvKillFocus(HWND getFocus)
{
  // Follow the parent chain back until a window volunteers to hold our handle
  //
  if (IsFlagSet(wfFullyCreated)) {
    TWindow* holdWin = Parent;
    while (holdWin && !holdWin->HoldFocusHWnd(GetHandle(), getFocus))
      holdWin = holdWin->Parent;
  }

  DefaultProcessing();
}

//
/// Response method for an incoming WM_SIZE message
///
/// Saves the normal size of the window in Attr.
/// Also calls the SetPageSize() and SetBarRange() methods of the TWindow's
/// scroller, if it has been constructed.
//
void
TWindow::EvSize(uint sizeType, const TSize&)
{
  TraceWindowPlacement();
  static bool inScroller = false;
  if (!inScroller && Scroller && sizeType != SIZE_MINIMIZED) {
    inScroller = true;
    Scroller->SetPageSize();
    Scroller->SetSBarRange();
    inScroller = false;
  }

  if (sizeType == SIZE_RESTORED) {
    TRect  wndRect;
    GetWindowRect(wndRect);

    Attr.W = wndRect.Width();
    Attr.H = wndRect.Height();
  }

  // Added Owl functionality: notify parent of a resize in case it wants to
  // adjust accordingly
  //
  if (Parent && !(GetExStyle() & WS_EX_NOPARENTNOTIFY))
    Parent->SendMessage(WM_PARENTNOTIFY, WM_SIZE, TParam2(GetHandle()));

  DefaultProcessing();
}

//
/// Save the normal position of the window.
/// If IsIconic() or IsZoomed() ignore the position since it does not reflect
/// the normal state
//
void
TWindow::EvMove(const TPoint&)
{
  if (!IsIconic() && !IsZoomed()) {
    TRect wndRect;

    GetWindowRect(wndRect);

    if ((GetWindowLong(GWL_STYLE) & WS_CHILD) == WS_CHILD && Parent &&
         Parent->GetHandle())
      Parent->ScreenToClient(wndRect.TopLeft());

    Attr.X = wndRect.left;
    Attr.Y = wndRect.top;
  }

  DefaultProcessing();
}

//
/// Handles WM_COMPAREITEM message (for owner draw controls) by forwarding
/// message to control itself
//
int
TWindow::EvCompareItem(uint /*ctrlId*/, const COMPAREITEMSTRUCT& compareInfo)
{
  TWindow* w = GetWindowPtr(compareInfo.hwndItem);
  TResult r = w ? w->ForwardMessage() : DefaultProcessing();
  return static_cast<int>(r);
}

//
/// Handles WM_DELETEITEM message (for owner draw controls) by forwarding
/// message to control itself
//
void
TWindow::EvDeleteItem(uint /*ctrlId*/, const DELETEITEMSTRUCT& deleteInfo)
{
  TWindow* win = GetWindowPtr(deleteInfo.hwndItem);
  if (deleteInfo.hwndItem != GetHandle() && win)
    win->ForwardMessage();
  else
    DefaultProcessing();
}
//
/// Handles WM_DRAWITEM message (for owner draw controls & menus) by forwarding
/// message to control itself
//
TDrawItem* ItemData2DrawItem(ULONG_PTR data);
void
TWindow::EvDrawItem(uint /*ctrlId*/, const DRAWITEMSTRUCT& drawInfo)
{
  if (drawInfo.CtlType == ODT_MENU) {
    TDrawItem* item = ItemData2DrawItem(drawInfo.itemData);
    if(item){
      DRAWITEMSTRUCT i(drawInfo); // Create copy to support legacy non-const virtual TDrawItem::Draw.
      item->Draw(i);
      return;
    }
  }
  else {
    TWindow* win = GetWindowPtr(drawInfo.hwndItem);
    if (drawInfo.hwndItem != GetHandle() && win) {
      win->ForwardMessage();
      return;
    }
  }
  DefaultProcessing();
}

//
/// Handles WM_MEASUREITEM message (for owner draw controls & menus) by
/// forwarding message to control itself
//
void
TWindow::EvMeasureItem(uint ctrlId, MEASUREITEMSTRUCT & measureInfo)
{
  if (measureInfo.CtlType == ODT_MENU) {
    TDrawItem* item = ItemData2DrawItem(measureInfo.itemData);
    if(item){
      item->Measure(measureInfo);
      return;
    }
  }
  else {
    HWND  hCtl = GetDlgItem(measureInfo.CtlID);  // hWnd not in struct, get
    TWindow* win = GetWindowPtr(hCtl);

    // If the GetWindowPtr failed, & CreationWindow is non-zero, then this
    // WM_MEASUREITEM is probably for the ctrl which is not yet subclassed.
    // Route the message directly to creation window.
    // NOTE: Msg. may be sent before OWL has had a chance to subclass the ctrl.
    //
    if (!win) {
      TWindow* creationWindow = GetCreationWindow();
      if (creationWindow) {
        if (creationWindow != this)   // Don't cause a recursion loop
          win = creationWindow;
      }
      else
        win = ChildWithId(ctrlId);
    }
    if(win){
      // 970921 MILI 11 The first WM_MEASUREITEM message for a control may
      // be is sent before OWL has had any chance to grab the handle for that
      // control. In that case we use Find+Dispatch instead of SendMessage to
      // forward the message to the control.
      if(win->GetHandle()){
// !CQ Handle causes bad behavior on DefWindowProc of control in some cases
///   win->HandleMessage(WM_MEASUREITEM, ctrlId, TParam2(&measureInfo));
///   win->ForewardMessage();
        win->SendMessage(WM_MEASUREITEM, ctrlId, TParam2(&measureInfo));
        return;
      }
      else{
        TEventInfo eventInfo(WM_MEASUREITEM);
        if (win->Find(eventInfo)){
          win->Dispatch(eventInfo, ctrlId, TParam2(&measureInfo));
           return;
        }
      }
    }
  }
  DefaultProcessing();
}

//
/// Called by EvHScroll and EvVScroll to dispatch messages from scroll bars.
//
void
TWindow::DispatchScroll(uint scrollCode, uint /*thumbPos*/, HWND hWndCtrl)
{
  if (hWndCtrl) {
    TWindow* win = GetWindowPtr(hWndCtrl);
    if (win)
      win->ForwardMessage();

    // Adjust "CurrentEvent" to allow DefaultProcessing to work
    //
    uint16 id = static_cast<uint16>(::GetDlgCtrlID(hWndCtrl));
    TCurrentEvent& currentEvent = GetCurrentEvent();
    currentEvent.Message = WM_COMMAND;
      currentEvent.Param1 = MkParam1(id, scrollCode);
      currentEvent.Param2 = TParam2(hWndCtrl);

    EvCommand(id, hWndCtrl, scrollCode);
    return;
  }
  DefaultProcessing();
}

//
/// Event handler for WM_MOUSEHWHEEL.
/// Responds to horizontal mouse wheel event if Scroller exists.
//
void
TWindow::EvMouseHWheel(uint modKeys, int zDelta, const TPoint& /*point*/)
{
  if (Scroller && !(modKeys & (MK_SHIFT | MK_CONTROL)))
  {
    const auto n = GetWheelScrollLines_();
    const auto dx = (n == WHEEL_PAGESCROLL) ?
      ((zDelta > 0) ? -Scroller->XPage : Scroller->XPage) :
      ::MulDiv(-zDelta, n, WHEEL_DELTA) * Scroller->XLine;
    Scroller->ScrollBy(dx, 0);
  }
  else
    DefaultProcessing();
}

//
/// Event handler for WM_MOUSEWHEEL.
/// Responds to vertical mouse wheel event if Scroller exists.
//
void
TWindow::EvMouseWheel(uint modKeys, int zDelta, const TPoint& /*point*/)
{
  if (Scroller && !(modKeys & (MK_SHIFT | MK_CONTROL)))
  {
    const auto n = GetWheelScrollLines_();
    const auto dy = (n == WHEEL_PAGESCROLL) ?
      ((zDelta > 0) ? -Scroller->YPage : Scroller->YPage) :
      ::MulDiv(-zDelta, n, WHEEL_DELTA) * Scroller->YLine;
    Scroller->ScrollBy(0, dy);
  }
  else
    DefaultProcessing();
}

//
/// Response method for an incoming WM_HSCROLL message
///
/// If the message is from a scrollbar control, calls DispatchScroll()
/// otherwise passes the message to the TWindow's scroller if it has been
/// constructed, else calls DefaultProcessing()
///
/// Assumes, because of a Windows bug, that if the window has the scrollbar
/// style, it will not have scrollbar controls
//
void
TWindow::EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  if (!(GetWindowLong(GWL_STYLE) & WS_HSCROLL) && !Scroller)
  {
    DispatchScroll(scrollCode, thumbPos, hWndCtl);  // from scrollbar control
  }
  else if (Scroller)
  {
    Scroller->HScroll(scrollCode, thumbPos);
  }
  else
  {
    DefaultProcessing();
  }
}

//
/// Response method for an incoming WM_VSCROLL message
///
/// If the message is from a scrollbar control, calls DispatchScroll()
/// otherwise passes the message to the TWindow's scroller if it has been
/// constructed, else calls DefaultProcessing()
///
/// Assumes, because of a Windows bug, that if the window has the scrollbar
/// style, it will not have scrollbar controls
//
void
TWindow::EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  if (!(GetWindowLong(GWL_STYLE) & WS_VSCROLL) && !Scroller)
    DispatchScroll(scrollCode, thumbPos, hWndCtl);

  else if (Scroller)
    Scroller->VScroll(scrollCode, thumbPos);

  else
    DefaultProcessing();
}

//
/// Handler for WM_ERASEBKGND.
///
/// If the background color (see TWindow::SetBkgndColor) is not set, or
/// it has been set to TColor::None, then this function simply forwards the
/// message to DefaultProcessing. The default processing uses the brush
/// specified in the window class (WNDCLASS:hbrBackground) to clear the client
/// area. If the background color is set to TColor::Transparent, no clearing is
/// done, and the function simply returns `true`. Otherwise, the current
/// background color is used to clear the client area, and `true` is returned,
/// indicating no further erasing is required.
///
/// If you want to handle erasure in your Paint override, then override this
/// handler and return false. Paint will then receive `true` in its `erase`
/// parameter, and you must ensure that all of the background is painted within
/// the invalidated region.
///
/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms648055.aspx
//
bool
TWindow::EvEraseBkgnd(HDC hDC)
{
  if (BkgndColor == TColor::None)
    return static_cast<bool>(DefaultProcessing()); // Use WNDCLASS::hbrBackground.

  if (BkgndColor == TColor::Transparent)
    return true; // We want no erasing, please.

  TDC(hDC).FillSolidRect(GetClientRect(), BkgndColor);
  return true;
}

//
/// Handler for control color messages (WM_CTLCOLOR family).
///
/// If the passed control `ctl` is a child encapsulated by OWL, then the text and background color
/// of the control is used. If the control is not encapsulated by OWL, or the control does not
/// specify a color (TColor::None), then colors are assigned by the parent (dialog). If the parent
/// has no colors set, default colors will be used.
///
/// If the background color is set to TColor::Transparent, then the background mode of the device
/// context `hDC` is set to TRANSPARENT and NULL_BRUSH is returned. Otherwise a solid brush of the
/// background color is returned.
///
/// \note Parent colors will not override defaults for Edit and List Box controls. For these
/// control types, the colors can only be set by the controls themselves. To set the colors for an
/// Edit or List Box control, encapsulate the control using the corresponding OWL class, and call
/// SetTextColor and SetBkgndColor on the object. For other controls, such as the Static control,
/// the parent will set the control's colors to match the parent's colors, unless explicitly set by
/// the control itself. This ensures that a dialog box is drawn properly. For example, a dialog
/// with labels and fields is drawn with the parent's colors for the labels, but with the system
/// default colors for the fields, unless colors are explicitly set by the controls.
/// Also, setting colors for a Combo Box control has no effect, since this control uses subcontrols
/// for the Edit field and List Box components. Hence, combo boxes need to be treated specially by
/// the parent by handling WM_CTLCOLOREDIT and WM_CTLCOLORLISTBOX, and testing the passed `ctl`
/// window handle for a match against the subcontrols. To determine the handles of the subcontrols,
/// use TComboBox::GetEditHandle and TComboBox::GetListHandle (or TComboBox::GetInfo to get both).
/// These functions are encapsulations of the Windows API function GetComboBoxInfo.
///
/// \code{.cpp}
/// // TComboBox ComboBox;
///
/// DEFINE_RESPONSE_TABLE1(TMyDlg, TDialog)
/// //...
///   EV_WM_CTLCOLOREDIT(EvCtlColor),
///   EV_WM_CTLCOLORLISTBOX(EvCtlColor),
/// END_RESPONSE_TABLE;
///
/// auto TMyDlg::EvCtlColor(HDC hdc, HWND ctl, uint ctlType) -> HBRUSH
/// {
///   const auto i = ComboBox.GetInfo();
///   const auto w = (ctl == i.hwndItem || ctl == i.hwndList) ? ComboBox.GetHandle() : ctl;
///   return TWindow::EvCtlColor(hdc, w, ctlType);
/// }
/// \endcode
///
/// \sa SetBkgndColor, SetTextColor
//
HBRUSH
TWindow::EvCtlColor(HDC hDC, HWND ctl, uint ctlType)
{
  // Start by letting the default processing handle the message. This ensures that we get the
  // system defaults for the attributes we do not override.
  //
  const auto defaultErasureBrush = reinterpret_cast<HBRUSH>(DefaultProcessing());

  // Now we need to determine which colors to use. If we are handling colors for a child, i.e. the
  // ctlType is not CTLCOLOR_DLG, then we let the child specify the color. If the child is not
  // encapsulated by OWL, or does not specify a color, then we use our own color. If we do not have
  // a color, we use the system default (as provided by the default processing).
  //
  // Note: We leave the colors of Edit and Listbox controls alone, unless the control is
  // encapsulated by an OWL class and explicitly specifies a color (i.e. other than TColor::None).
  //
  const auto owlChild = (ctlType != CTLCOLOR_DLG) ? GetWindowPtr(ctl) : nullptr;
  const auto shouldIgnoreControl = ctlType == CTLCOLOR_EDIT || ctlType == CTLCOLOR_LISTBOX;
  const auto textColor = (owlChild && owlChild->GetTextColor() != TColor::None) ? owlChild->GetTextColor() :
    shouldIgnoreControl ? TColor::None :
    TextColor;
  const auto bkgndColor = (owlChild && owlChild->GetBkgndColor() != TColor::None) ? owlChild->GetBkgndColor() :
    shouldIgnoreControl ? TColor::None :
    BkgndColor;

  // Override the control text color, if set.
  //
  if (textColor != TColor::None)
  {
    CHECK(textColor != TColor::Transparent);
    ::SetTextColor(hDC, textColor);
  }

  // Override the background color, if set, and return a corresponding erasure brush.
  //
  if (bkgndColor == TColor::None)
  {
    return defaultErasureBrush;
  }
  else if (bkgndColor == TColor::Transparent)
  {
    ::SetBkMode(hDC, TRANSPARENT);
    return reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));
  }
  else
  {
    ::SetBkColor(hDC, bkgndColor);
    return TBrush{bkgndColor}; // HBRUSH will stay in cache.
  }
}

//
/// Response method for an incoming WM_PAINT message
///
/// Calls Paint(), performing Windows-required paint setup and cleanup before
/// and after. if the TWindow has a TScroller, also calls its BeginView() and
/// EndView() methods before and after call to Paint()
//
void
TWindow::EvPaint()
{
  if (IsFlagSet(wfAlias))
    DefaultProcessing();  // use application-defined wndproc

  else {
    TPaintDC dc(*this);
    TRect&   rect = *static_cast<TRect*>(&dc.Ps.rcPaint);

    if (Scroller)
      Scroller->BeginView(dc, rect);

    Paint(dc, dc.Ps.fErase, rect);

    if (Scroller)
      Scroller->EndView();
  }
}

//
/// Repaints the client area (the area you can use for drawing) of a window. Called
/// by base classes when responding to a WM_PAINT message, Paint serves as a
/// placeholder for derived types that define Paint member functions. Paint is
/// called by EvPaint and requested automatically by Windows to redisplay the
/// window's contents. dc is the paint display context supplied to text and graphics
/// output functions. The supplied reference to the rect structure is the bounding
/// rectangle of the area that requires painting. erase indicates whether the
/// background needs erasing.
//
void
TWindow::Paint(TDC&, bool /*erase*/, TRect&)
{
}

//
/// Response method for an incoming WM_SETCURSOR message.
/// If a cursor has been set for this window using TWindow::SetCursor, and the mouse is over the
/// client area, the cursor is used. Otherwise, the cursor for the window class is used (default).
//
bool
TWindow::EvSetCursor(HWND hWndCursor, uint codeHitTest, TMsgId /*mouseMsg*/)
{
  if (hWndCursor == GetHandle() && codeHitTest == HTCLIENT && HCursor) {
    ::SetCursor(HCursor);
    return true;
  }
  return static_cast<bool>(DefaultProcessing());
}

//
/// Response method for an incoming WM_LBUTTONDOWN message
///
/// If the TWindow's Scroller has been constructed and if auto-scrolling
/// has been requested, captures mouse input, loops until a WM_LBUTTONUP
/// message comes in calling the Scroller's AutoScroll method, and then
/// releases capture on mouse input.
/// Will also break if a WM_MOUSEMOVE comes in without the left button down
/// indicating a lost WM_LBUTTONUP
//
void
TWindow::EvLButtonDown(uint /*modKeys*/, const TPoint& /*pt*/)
{
  if (Scroller && Scroller->IsAutoMode()) {
    MSG  loopMsg;

    loopMsg.message = 0;
    SetCapture();

    while (loopMsg.message != WM_LBUTTONUP && Scroller->IsAutoMode() &&
          (loopMsg.message != WM_MOUSEMOVE || (loopMsg.wParam&MK_LBUTTON))) {
      if (::PeekMessage(&loopMsg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&loopMsg);
        ::DispatchMessage(&loopMsg);

        // Rethrow any exception suspended in the handler, in which case we need to release
        // the mouse capture.
        //
        try
        {
          GetApplication()->ResumeThrow();
        }
        catch (...)
        {
          ReleaseCapture();
          throw;
        }
      }
      Scroller->AutoScroll();
    }
    ReleaseCapture();
  }

  DefaultProcessing();
}

//namespace owl {
//
//
//
void
DoEnableAutoCreate(TWindow* win, void* /*retVal*/)
{
  if (win->GetHandle())
    win->EnableAutoCreate();
}

//} // OWL namespace

//
/// Destroys an MS-Windows element associated with the TWindow.
//
/// First, Destroy calls EnableAutoCreate for each window in the child list to
/// ensure that windows in the child list will be re-created if this is re-created.
/// Then, it destroys the associated interface element.
/// If a derived window class expects to be destructed directly, it should call
/// Destroy as the first step in its destruction so that any virtual functions and
/// event handlers can be called during the destroy sequence.
//
void
TWindow::Destroy(int ret)
{
  if (GetHandle())
  {
    for (auto& w : GetChildren())
    {
      if (w.GetHandle())
        w.EnableAutoCreate();
    }

    if (IsFlagSet(wfModalWindow)) {
      GetApplication()->EndModal(ret);
      ClearFlag(wfModalWindow);
      if (Parent && Parent->GetHandle())
        Parent->SetFocus();
    }

    if (!IsFlagSet(wfAlias)) {
      if (::DestroyWindow(GetHandle()))
        SetHandle(nullptr);
    }

    GetApplication()->ResumeThrow();
    WARNX(OwlWin, GetHandle(), 0, _T("::DestroyWindow(") << static_cast<void*>(GetHandle()) << _T(") failed"));
  }
}

//
/// Redefined by derived classes, GetWindowClass fills the supplied MS-Windows
/// registration class structure with registration attributes, thus, allowing
/// instances of TWindow to be registered. This function, along with GetWindowClassName,
/// allows Windows classes to be used for the specified ObjectWindows class and its
/// derivatives. It sets the fields of the passed WNDCLASS parameter to the default
/// attributes appropriate for a TWindow. The fields and their default attributes
/// for the class are the following:
/// - \c \b cbClsExtra  0 (the number of extra bytes to reserve after the Window class
/// structure). This value is not used by ObjectWindows.
/// - \c \b cbWndExtra  0 (the number of extra bytes to reserve after the Window instance).
/// This value is not used by ObjectWindows.
/// - \c \b hInstance  The instance of the class in which the window procedure exists
/// - \c \b hIcon  0 (Provides a handle to the class resource.) By default, the application
/// must create an icon if the application's window is minimized.
/// - \c \b hCursor  IDC_ARROW (provides a handle to a cursor resource)
/// - \c \b hbrBackground  COLOR_WINDOW + 1 (the system background color)
/// - \c \b lpszMenuName  0 (Points to a string that contains the name of the class's menu.)
/// By default, the windows in this class have no assigned menus.
/// - \c \b lpszClassName  Points to a string that contains the name of the window class.
/// - \c \b lpfnWndProc  The address of the window procedure. This value is not used by
/// ObjectWindows.
/// - \c \b style  Style field.
///
/// The style field can contain one or more of the following values:
/// - \c \b CS_BYTEALIGNCLIENT  Aligns the window's client on a byte boundary in the x
/// direction. This alignment, designed to improve performance, determines the width
/// and horizontal position of the window.
/// - \c \b CS_BYTEALIGNWINDOW  Aligns a window on a byte boundary in the x direction. This
/// alignment, designed to improve performance, determines the width and horizontal
/// position of the window.
/// - \c \b CS_CLASSDC  Allocates a single device context (DC) that's going to be shared by
/// all of the window in the class. This style controls how multi-threaded
/// applications that have windows belonging to the same class share the same DC.
/// - \c \b CS_DBLCLKS  Sends a double-click mouse message to the window procedure when the
/// mouse is double-clicked on a window belonging to this class.
/// - \c \b CS_GLOBALCLASS  Allows an application to create a window class regardless of the
/// instance parameter. You can also create a global class by writing a DLL that
/// contains the window class.
/// - \c \b CS_HREDRAW  If the size of the window changes as a result of some movement or
/// resizing, redraws the entire window.
/// - \c \b CS_NOCLOSE  Disables the Close option on this window's system menu.
/// - \c \b CS_OWNDC  Enables each window in the class to have a different DC.
/// - \c \b CS_PARENTDC  Passes the parent window's DC to the child windows.
/// - \c \b CS_SAVEBITS  Saves the section of the screen as a bitmap if the screen is covered
/// by another window. This bitmap is later used to recreate the window when it is
/// no longer obscured by another window.
/// - \c \b CS_VREDRAW  If the height of the client area is changed, redraws the entire
/// window.
///
/// After the Windows class structure has been filled with default values by the
/// base class, you can override this function to change the values of the Windows
/// class structure. For example, you might want to change the window's colors or
/// the cursor displayed.
///
/// Register unique classes for windows that want system background colors so
/// that full-drag erasing uses the right color (NT fails to let window erase
/// itself)
///
void
TWindow::GetWindowClass(WNDCLASS& wndClass)
{
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = 0;
  wndClass.hInstance = *GetModule();
  wndClass.hIcon = nullptr;
  wndClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);

  if (BkgndColor.IsSysColor() &&
      BkgndColor != TColor::None && BkgndColor != TColor::Transparent)
    wndClass.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(BkgndColor.Index() + 1));
  else
    wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

  wndClass.lpszMenuName = nullptr;
  wndClass.lpszClassName = GetWindowClassName().GetPointerRepresentation();
  wndClass.style = CS_DBLCLKS;
  wndClass.lpfnWndProc = InitWndProc;
}

//
// Return the classname for a generic owl window. Assume instance private class
// registration so that no instance mangling is needed.
//
// Register unique classnames for windows that want system background colors
//
auto TWindow::GetWindowClassName() -> TWindowClassName{
  static const tchar baseClassName[] = _T("OWL_Window");

  if (BkgndColor.IsSysColor() &&
      BkgndColor != TColor::None && BkgndColor != TColor::Transparent) {
    static tchar namebuff[COUNTOF(baseClassName) + 1 + 10 + 1 + 4]; // ?? How was this formula calculated
    _stprintf(namebuff, _T("%s:%X"), baseClassName, BkgndColor.Index());
    // !CQ could hash in classStyle too.
    return TWindowClassName{namebuff};
  }
  return TWindowClassName{baseClassName};
}

//
// Set this window's accelerator table, performing the load also if this window
// is already created
//
void
TWindow::SetAcceleratorTable(TResId resId)
{
  Attr.AccelTable = resId;
  if (GetHandle())
    LoadAcceleratorTable();
  else
    HAccel = nullptr;
}

//
/// Loads a handle to the window's accelerator table specified in the TWindowAttr
/// structure (Attr.AccelTable). If the accelerator does not exist,
/// LoadAcceleratorTable produces an "Unable to load accelerator table" diagnostic
/// message.
//
void
TWindow::LoadAcceleratorTable()
{
  if (Attr.AccelTable) {
    HAccel = LoadAccelerators(Attr.AccelTable);
    WARNX(OwlWin, !HAccel, 0,
          _T("Unable to load accelerators ") << Attr.AccelTable
          << _T(" from ") << *GetModule());
  }
}

//
// Helper used by Create and PerformCreate to make a menu-or-id union parameter
// for CreateWindowEx; see Windows SDK documentation.
//
HMENU
TWindow::MakeMenuOrId()
{
  PRECONDITION(GetModule());

  HMENU m = Attr.Menu ? LoadMenu(Attr.Menu) : reinterpret_cast<HMENU>(static_cast<INT_PTR>(Attr.Id));
  WARNX(OwlWin, Attr.Menu && !m, 0,
    _T("Unable to load menu: ") << Attr.Menu << _T(" from ") << *GetModule());
  return m;
}


//
/// Called from Create to perform the final step in creating an Windows interface
/// element to be associated with a TWindow. PerformCreate can be overridden to
/// provide alternate implementations.
///
/// In strict mode we ignore the argument passed, and we build the menu-or-id
/// parameter to CreateWindowEx purely based on TWindow data members. In old
/// mode we treat the argument like before and assume that it is already a
/// composed menu-or-id union. In case it represents a menu, we take ownership
/// of the allocated menu (which should have been created by the caller).
//
TWindow::THandle
TWindow::PerformCreate()
{
  PRECONDITION(GetModule());
  //
  // Use RAII to ensure the menu is released in case of failure.
  //
  struct TMenuGuard
  {
    HMENU m;
    bool is_mine;

    TMenuGuard(HMENU m_, bool is_mine_) : m(m_), is_mine(is_mine_) {}
    ~TMenuGuard() {if (is_mine) DestroyMenu(m);}
    operator HMENU() {return m;}
    HMENU RelinquishOwnership() {is_mine = false; return m;}
  }
  menuOrId(MakeMenuOrId(), Attr.Menu != 0);
  THandle h = CreateWindowEx(
    Attr.ExStyle,
    GetWindowClassName().GetPointerRepresentation(),
    Title,
    Attr.Style,
    Attr.X, Attr.Y, Attr.W, Attr.H,
    Parent ? Parent->GetHandle() : nullptr,
    menuOrId,
    *GetModule(),
    Attr.Param
    );
  if (h) menuOrId.RelinquishOwnership(); // The menu belongs to the window now.
  return h;
}

//
/// Creates the window interface element to be associated with this ObjectWindows
/// interface element. Specifically, Create performs the following window creation
/// tasks:
/// - 1.  If the HWND already exists, Create returns true. (It is perfectly valid to
/// call Create even if the window currently exists.)
/// - 2.  If the wfFromResource flag is set, then Create grabs the HWND based on the
/// window ID. Otherwise, Create registers the window, sets up the window thunk,
/// loads accelerators and menus, and calls PerformCreate in the derived class to
/// create the HWND.
/// - 3.  If class registration fails for the window, Create calls TXWindow with
/// IDS_CLASSREGISTERFAIL. If the window creation fails, Create calls TXWindow  with
/// IDS_WINDOWCREATEFAIL.
/// - 4.  If the window is created for a predefined Window class (for example, a
/// button or dialog class) registered outside of ObjectWindows, then ObjectWindows
/// thunks the window so that it can intercept messages and obtains the state of the
/// window (the window's attributes) from the HWND.
///
/// \note Since this member function now throws an exception on error, it always
/// returns true.
//
bool
TWindow::Create()
{
  if (GetHandle())
    return true;

  DisableAutoCreate();

  // If this is the main window, make sure it is treated as one by the shell.
  //
  if (IsFlagSet(wfMainWindow))
    ModifyExStyle(0, WS_EX_APPWINDOW);

  if (IsFlagSet(wfFromResource))
    SetHandle(Parent ? Parent->GetDlgItem(Attr.Id) : nullptr);  // Windows already created it.
  else
  {
    // Make sure the window class is registered.
    //
    if (!Register())
      TXWindow::Raise(this, IDS_CLASSREGISTERFAIL);

    // Perform necessary steps to create the window and attach the window procedure.
    //
    SetCreationWindow(this);
    LoadAcceleratorTable();
    CHECK(!GetHandle()); // Ensure handle is NULL in case of exceptions.

    // In the new API, PerformCreate takes no arguments.
    // Also PerformCreate now returns the handle instead of calling SetHandle. 
    //
    SetHandle(PerformCreate());
    GetApplication()->ResumeThrow();
  }
  WARNX(OwlWin, !GetHandle(), 0, _T("TWindow::Create failed: ") << *this);
  if (!GetHandle())
    TXWindow::Raise(this, IDS_WINDOWCREATEFAIL);

  // Here we deal with non-subclassed handles. This may be caused by two scenarios:
  //
  // 1. Predefined window class (non-owl) windows.
  // 2. OWL controls which were created from resource [Although these
  //    are registered with InitWndProc, they did not get subclassed since
  //    'CreationWindow' was not set when they received their first messages].
  //
  if (!GetWindowPtr(GetHandle()))
  {

    // If we have a title, then overwrite the current window text, if any.
    // Otherwise sync the title with the current window text.
    //
    if (Title)
      SetCaption(Title);
    else
      GetWindowTextTitle();

    // Grab the state info.
    //
    GetHWndState();

    // If it's a 'predefinedClass' window, subclass it.
    //
    if (GetWindowProc() != InitWndProc)
    {
      SubclassWindowFunction();

      // Reset the 'CreationWindow' pointer [if necessary].
      //
      if (GetCreationWindow() == this)
        SetCreationWindow(nullptr);

      // Set status flag [since we missed EvCreate].
      //
      SetFlag(wfPredefinedClass);
    }
    else
    {
      // This window's WNDPROC is 'InitWndProc' - However, it has not
      // been subclassed since 'CreationWindow' was not set when it received
      // its first messages [it was probably created from a resource but
      // registered by OWL]. We'll set 'CreationWindow' and send a dummy
      // message to allow subclassing to take place.
      //
      if (!GetCreationWindow())
        SetCreationWindow(this);
      SendMessage(WM_USER+0x4000, 0, 0);
    }

    // Perform setup and transfer now, since 'EvCreate' was missed earlier.
    //
    PerformSetupAndTransfer();
  }
  return true;
}

//
/// Creates the underlying HWND and makes it modal with the help of TApplication's
/// BeginModal support.
//
int
TWindow::Execute()
{
  return DoExecute();
}

//
/// Do actual modal execution using the Begin/End Modal support of TApplication.
/// \note Defaults to 'TASKMODAL'.
//
int
TWindow::DoExecute()
{
  // Attempting to go modal when one's a child is indicative of
  // suicidal tendencies!!
  //
  PRECONDITION((GetStyle() & WS_CHILD) == 0);

  if (GetApplication()) {
    if (Create()) {
      SetFlag(wfModalWindow);
      return GetApplication()->BeginModal(this, MB_TASKMODAL);
    }
  }
  return -1;
}

//
/// Ensures that the window is fully set up; then transfers data into the window.
//
void
TWindow::PerformSetupAndTransfer()
{
  SetupWindow();
  SetFlag(wfFullyCreated);

  // Note that transfer has already happened in SetupWindow if the library is
  // built in backwards compatibility mode. See SetupWindow for details.
  //
#if !defined(OWL5_COMPAT)
  TransferData(tdSetData);
#endif
}

//
/// Performs setup following creation of an associated MS-Windows window.
///
/// The first virtual function called when the HWindow becomes valid. TWindow's
/// implementation performs window setup by iterating through the child list,
/// attempting to create an associated interface element for each child window
/// object for which autocreation is enabled. (By default, autocreation is enabled
/// for windows and controls, and disabled for dialog boxes.)
/// If a child window cannot be created, SetupWindow calls TXWindow
/// with an IDS_CHILDCREATEFAIL message.
///
/// If the receiver has a TScroller object, calls the scroller's SetBarRange()
/// method.
///
/// SetupWindow can be redefined in derived classes to perform additional special
/// initialization. Note that the HWindow is valid when the overridden SetupWindow
/// is called, and that the children's HWindows are valid after calling the base
/// classes' SetupWindow function.
///
/// The following example from the sample program, APPWIN.CPP, illustrates the use
/// of an overridden SetupWindow to setup a window, initialize .INI entries, and
/// tell Windows that we want to accept drag and drop transactions:
/// \code
/// void TAppWindow::SetupWindow()
/// {
///   TFloatingFrame::SetupWindow();
///   InitEntries();   // Initialize .INI entries.
///   RestoreFromINIFile(); // from APPLAUNC.INI in the startup directory
///   UpdateAppButtons();
///   DragAcceptFiles(true);
/// }
/// \endcode
//
void
TWindow::SetupWindow()
{
  TRACEX(OwlWin, 1, _T("TWindow::SetupWindow() @") << (void*)this << *this);

  // Update scrollbar - if a scroller was attached to the window
  //
  if (Scroller){
    Scroller->SetWindow(this);
    Scroller->SetSBarRange();
  }

  // If this is main Window and GetAplication()->GetTooltip() != 0; create it.
  if (IsFlagSet(wfMainWindow)){
    TTooltip* tooltip = GetApplication()->GetTooltip();
    if(tooltip){
      if(!tooltip->GetParentO())
        tooltip->SetParent(this);
      if(!tooltip->GetHandle()){
        // Make sure tooltip is disabled so it does not take input focus
        tooltip->ModifyStyle(0,WS_DISABLED);
        tooltip->Create();
      }
    }
  }

  // NOTE: CreateChildren will throw a TXWindow exception if one of the
  //       child objects could not be created.
  //
  CreateChildren();

  // Transfer data here for legacy compatibility.
  // Note that this may be before any setup in a derived class has completed,
  // if that class overrides SetupWindow by calling this base version first,
  // as is the usual idiom. This problem has now been fixed, and TransferData
  // is now normally called from PerformSetupAndTransfer, thus ensuring that setup
  // has been fully completed before transfer. But we retain the old behaviour
  // here for compatibility modes to support legacy applications.
  //
#if defined(OWL5_COMPAT)
  TransferData(tdSetData);
#endif
}

//
/// Always called immediately before the HWindow becomes invalid, CleanupWindow
/// gives derived classes an opportunity to clean up HWND related resources. This
/// function is the complement to SetupWindow.
///
/// Override this function in your derived class to handle window cleanup. Derived
/// classes should call the base class's version of CleanupWindow as the last step
/// before returning. The following example from the sample program, APPWIN.CPP,
/// illustrates this process:
/// \code
/// //Tell windows that we are not accepting drag and drop transactions any more and
/// //perform other window cleanup.
/// void
/// TAppWindow::CleanupWindow()
/// {
///   AppLauncherCleanup();
///   DragAcceptFiles(false);
///   TWindow::CleanupWindow();
/// }
/// \endcode
//
void
TWindow::CleanupWindow()
{
  TRACEX(OwlWin, 1, _T("TWindow::CleanupWindow() @") << (void*)this << *this);
}

//
/// Transfers data to or from any window with or without children and returns the
/// total size of the data transferred. Transfer is a general mechanism for data
/// transfer that can be used with or without using TransferData. The direction
/// supplied specifies whether data is to be read from or written to the supplied
/// buffer, or whether the size of the transfer data is simply to be returned. Data
/// is not transferred to or from any child windows whose wfTransfer flag is not
/// set. The return value is the size (in bytes) of the transfer data.
//
uint
TWindow::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer && direction != tdSizeData) return 0;

  //
  // Transfer window 'data' to/from the passed data buffer.  Used to initialize
  // windows and get data in or out of them.
  //
  // The direction passed specifies whether data is to be read from or written
  // to the passed buffer, or whether the data element size is simply to be
  // returned
  //
  // The return value is the size (in bytes) of the transfer data.  this method
  // recursively calls transfer on all its children that have wfTransfer set.
  //
  auto p = buffer;
  for (auto& w : GetChildren())
  {
    if (w.IsFlagSet(wfTransfer))
      p = static_cast<char*>(p) + w.Transfer(p, direction);
  }
  return static_cast<uint>(reinterpret_cast<char*>(p) - reinterpret_cast<char*>(buffer));
}

//
/// Transfers data between the TWindow's data buffer and the child
/// windows in its ChildList (data is not transfered between any child
/// windows whose wfTransfer flag is not set)
///
/// A window usually calls TransferData during setup and closing of windows and
/// relies on the constructor to set TransferBuffer to something meaningful.
/// TransferData calls the Transfer member function of each participating child
/// window, passing a pointer to TransferBuffer as well as the direction specified
/// in direction (tdSetData, tdGetData, or tdSizeData).
//
void
TWindow::TransferData(TTransferDirection direction)
{
  if (!TransferBuffer) return; // nothing to do
  uint size = Transfer(TransferBuffer, tdSizeData);
  if (direction == tdSizeData) return; // done
  if (TransferBufferSize > 0 && size != TransferBufferSize)
    TXWindow::Raise(this, IDS_TRANSFERBUFFERSIZEMISMATCH);
  WARN(TransferBufferSize == 0,
    _T("TWindow::TransferData: Unsafe transfer is performed! ")
    _T("Use one of the safe overloads of SetTransferBuffer to enable buffer checks."));
  Transfer(TransferBuffer, direction);
}

//
// Checks whether the given HWND belongs to this process.
// Internal function.
//
inline static bool BelongsToCurrentProcess (HWND h)
{
  DWORD processId = 0;
  ::GetWindowThreadProcessId(h, &processId);
  return processId == ::GetCurrentProcessId();
}

//
/// Installs the instance thunk as the WindowProc and saves the old window function
/// in DefaultProc.
//
void
TWindow::SubclassWindowFunction()
{
  PRECONDITION(GetHandle());
  PRECONDITION(GetInstanceProc());

  if (!BelongsToCurrentProcess(GetHandle())) return;

  // If the window already has the window proc we want,
  // then just ensure that it has a default proc and return.
  //
  if (GetInstanceProc() == GetWindowProc())
  {
    if (!DefaultProc) DefaultProc = ::DefWindowProc;
    return;
  }

  // Initialize the instance proc and install it.
  //
  InitInstanceProc();
  DefaultProc = SetWindowProc(GetInstanceProc());
}

//
/// Registers the Windows registration class of this window, if this window is not
/// already registered. Calls GetWindowClassName and GetWindowClass to retrieve the
/// Windows registration class name and attributes of this window. Register returns
/// true if this window is registered.
//
bool
TWindow::Register()
{
  auto& m = *GetModule();
  const auto c = GetWindowClassName();
  if (m.IsRegisteredClass(c)) return true;
  auto w = WNDCLASS{};
  GetWindowClass(w);
  CHECK(c.GetPointerRepresentation() == w.lpszClassName);
  return m.IsRegisteredClass(c) ? true : ::RegisterClass(&w);
}

//
/// Use this function to determine if it is okay to close a window. Returns true if
/// the associated interface element can be closed. Calls the CanClose member
/// function of each of its child windows. Returns false if any of the CanClose
/// calls returns false.
/// In your application's main window, you can override TWindow's CanClose and call
/// TWindow::MessageBox to display a YESNOCANCEL message prompting the user as
/// follows:
/// - \c \b YES   Save the data
/// - \c \b NO   Do not save the data, but close the window
/// - \c \b CANCEL   Cancel the close operation and return to the edit window
///
/// The following example shows how to write a CanClose function that displays a
/// message box asking if the user wants to save a drawing that has changed. To save
/// time, CanClose uses the IsDirty flag to see if the drawing has changed. If so,
/// CanClose queries the user before closing the window.
/// \code
/// bool TMyWindow::CanClose()
/// {
///   if (IsDirty)
///     switch(MessageBox("Do you want to save?", "Drawing has changed.",
///                       MB_YESNOCANCEL | MB_ICONQUESTION)) {
///       case IDCANCEL:
///         // Choosing Cancel means to abort the close -- return false.
///         return false;
///
///       case IDYES:
///         // Choosing Yes means to save the drawing.
///         CmFileSave();
///     }
///   return true;
/// }
/// \endcode
//
bool
TWindow::CanClose()
{
  auto c = GetChildren();
  return all_of(c.begin(), c.end(), [](TWindow& w) { return !w.GetHandle() || w.CanClose(); });
}

//
/// Determines if it is okay to close a window before actually closing the window.
/// If this is the main window of the application, calls GetApplication->CanClose.
/// Otherwise, calls this->CanClose to determine whether the window can be closed.
/// After determining that it is okay to close the window, CloseWindow calls Destroy
/// to destroy the HWND.
//
void
TWindow::CloseWindow(int retVal)
{
  bool  willClose;

  if (IsFlagSet(wfMainWindow))
    willClose = GetApplication()->CanClose();

  else
    willClose = CanClose();

  if (willClose)
    Destroy(retVal);
}

//
/// The default response to a WM_CLOSE message is to call CloseWindow()
/// and then have the window deleted if the Handle was really destroyed.
//
void
TWindow::EvClose()
{
  if (IsFlagSet(wfAlias))
    DefaultProcessing();

  else {
    CloseWindow();
    if (!GetHandle() && IsFlagSet(wfDeleteOnClose))
      GetApplication()->Condemn(this);  // Assumes delete
  }
}

//
/// Responds to an incoming WM_DESTROY message
///
/// Calls CleanupWindow() to let derived classes cleanup
/// Clears the wfFullyCreated flag since this window is no longer fully created
///
/// If the TWindow is the application's main window posts a 'quit' message to
/// end the application, unless already in ~TApplication() (MainWindow == 0)
//
void
TWindow::EvDestroy()
{
  ClearFlag(wfFullyCreated);
  CleanupWindow();

  if (!IsFlagSet(wfAlias)) {
    if (IsFlagSet(wfMainWindow) && GetApplication()->IsRunning())
      ::PostQuitMessage(0);
  }

  DefaultProcessing();
}

//
/// Responds to an incoming WM_NCDESTROY message, the last message
/// sent to an MS-Windows interface element
///
/// Sets the Handle data member of the TWindow to zero to indicate that an
/// interface element is no longer associated with the object
//
void
TWindow::EvNCDestroy()
{
  DefaultProcessing();
  SetHandle(nullptr);
}

//
/// Respond to Windows attempt to close down. Determines if this app or window
/// is ready to close.
//
bool
TWindow::EvQueryEndSession(uint /*flags*/)
{
  if (IsFlagSet(wfAlias))
    return static_cast<bool>(DefaultProcessing());

  else if (IsFlagSet(wfMainWindow))
    return GetApplication()->CanClose();

  else
    return CanClose();
}

//
/// Provides default handling for WM_ENDSESSION.
/// If the session is ending, throws the exception TXEndSession, thus shutting down the
/// entire applicaton.
//
void
TWindow::EvEndSession(bool endSession, uint /*flags*/)
{
  if (endSession)
    throw TXEndSession();
  else
    DefaultProcessing();
}


//
/// Handle message posted to us by a control needing assistance in dealing with
/// invalid inputs
///
/// Responds to a WM_CHILDINVALID message posted by a child edit control. Indicates
/// that the contents of the child window are invalid.
//
void
TWindow::EvChildInvalid(HWND handle)
{
  PRECONDITION(handle);

  ::SendMessage(handle, WM_CHILDINVALID, 0, 0);
}

//----------------------------------------------------------------------------
// Non-virtuals
//

void
TWindow::AttachHandle(HWND handle)
{
  ClearFlag(wfDetached);
  FreeInstanceProc();
  InstanceProc = nullptr;
  Init(handle, GetModule());
}


void
TWindow::DetachHandle()
{
  // NOTE: This is by no means a complete way of detaching the window...
  //       The following is logic allows Delphi/OWL interaction..
  //
  ClearFlag(wfFullyCreated);
  SetHandle(nullptr);
  SetFlag(wfDetached);
}


//
/// Returns the number of child windows of the window.
//
unsigned
TWindow::NumChildren() const
{
  return IndexOf(ChildList) + 1;
}

//
// Walks over children and assigns a z-order index to the ZOrder member
//
void
TWindow::AssignZOrder()
{
  TWindow*  wnd;
  HWND   curWindow = GetHandle();

  if (curWindow) {
    curWindow = ::GetWindow(curWindow, GW_CHILD);

    if (curWindow) {
      int  i = 1;

      for (curWindow = ::GetWindow(curWindow, GW_HWNDLAST);
           curWindow;
           curWindow = ::GetWindow(curWindow, GW_HWNDPREV))
      {
        wnd = GetWindowPtr(curWindow);

        if (wnd)
          wnd->ZOrder = static_cast<uint16>(i++);
      }
    }
  }
}

//
/// Creates the child windows in the child list whose auto-create flags (with
/// wfAutoCreate mask) are set. If all of the child windows are created
/// successfully, CreateChildren returns true.
/// \note Throws an exception (TXWindow) if a child object could not be
///       created.
//
bool
TWindow::CreateChildren()
{
  // Create children first to restore creation order.
  //
  for (auto& child : GetChildren())
  {
    //
    if (child.GetHandle())
      continue;

    bool autoCreate = child.IsFlagSet(wfAutoCreate);
    WARNX(OwlWin, !autoCreate, 0, _T("Child window(Id=") << child.GetId() << _T(") not autocreated"));
    if (!autoCreate)
      continue;

    // This call will only fail if a user-defined Create() returns false.
    // OwlNext's Create-implementations always throw exceptions.
    //
    if (!child.Create())
      throw TXWindow{&child, IDS_WINDOWCREATEFAIL};

    // Get & set the window text for the child if it is iconic.
    // TODO: Review if this hack is still necessary.
    //
    if (child.IsIconic())
      child.SetWindowText(child.GetWindowText());
  }

  // Restore Z-ordering for children that have Z-ordering recorded.
  //
  HWND above = HWND_TOP;
  for (int top = NumChildren(); top; top--)
  {
    auto c = GetChildren();
    const auto i = find_if(c.begin(), c.end(), [&top](TWindow& w) { return w.ZOrder == top; });
    if (i != c.end())
    {
      TWindow& child = *i;
      child.SetWindowPos(above, TRect{}, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
      above = child.GetHandle();
    }
  }
  return true;
}

//
// adds the passed pointer to a child window to the linked list
// of sibling windows which ChildList points to
//
void
TWindow::AddChild(TWindow* child)
{
  if (!child) return;
  if (ChildList)
  {
    child->SiblingList = ChildList->SiblingList;
    ChildList->SiblingList = child;
    ChildList = child;
  }
  else
  {
    ChildList = child;
    child->SiblingList = child;
  }
}

//
/// Returns a pointer to the TWindow's previous sibling (the window previous to
/// the TWindow in its parent's child window list)
///
/// If the TWindow was the first child added to the list, returns a pointer to
/// the last child added
//
TWindow*
TWindow::Previous()
{
  if (!SiblingList) {
    return nullptr;
  }
  else {
    TWindow*  CurrentIndex = this;

    while (CurrentIndex->Next() != this)
      CurrentIndex = CurrentIndex->Next();

    return CurrentIndex;
  }
}

#if defined(OWL5_COMPAT)

//
/// Returns a pointer to the first TWindow in the ChildList that meets some
/// specified criteria
///
/// If no child in the list meets the criteria, 0 is returned
///
/// The Test parameter which defines the criteria, is a pointer to a
/// function that takes a TWindow pointer & a pointer to void
///
/// The TWindow* will be used as the pointer to the child window and
/// the void* as a pointer to the Test function's additional parameters
///
/// The Test function must return a Boolean value indicating whether the
/// child passed meets the criteria
///
/// In the following example, GetFirstChecked calls FirstThat to obtain a pointer
/// (p) to the first check box in the child list that is checked:
/// \code
/// bool IsThisBoxChecked(TWindow* p, void*) {
///    return ((TCheckBox*)p)->GetCheck() == BF_CHECKED;
/// }
/// TCheckBox* TMyWindow::GetFirstChecked() {
///    return FirstThat(IsThisBoxChecked);
/// }
/// \endcode
//
TWindow*
TWindow::FirstThat(TCondFunc test, void* paramList) const
{
  if (ChildList) {
    TWindow*  nextChild = ChildList->Next();
    TWindow*  curChild;

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();

      //
      // Test curChild for okay
      //
      if (test(curChild, paramList))
        return curChild;
    } while (curChild != ChildList && ChildList);
  }
  return 0;
}

//
/// Iterates over each child window in the TWindow's ChildList,
/// calling the procedure whose pointer is passed as the Action to be
/// performed for each child
///
/// A pointer to a child is passed as the first parameter to the iteration
/// procedure
///
/// In the following example, CheckAllBoxes calls ForEach, checking all the check
/// boxes in the child list:
/// \code
/// void CheckTheBox(TWindow* p, void*) {
///  ((TCheckBox*)p)->Check();
/// }
/// void CheckAllBoxes() {
///   ForEach(CheckTheBox);
/// }
/// \endcode
//
void
TWindow::ForEach(TActionFunc action, void* paramList)
{
  if (ChildList) {
    TWindow*  curChild;
    TWindow*  nextChild = ChildList->Next();  // Allows ForEach to delete children

    do {
      curChild = nextChild;
      nextChild = nextChild->Next();
      action(curChild, paramList);
    } while (curChild != ChildList && ChildList);
  }
}

//
/// Returns a pointer to the first TWindow in the ChildList that
/// meets some specified criteria
///
/// If no child in the list meets the criteria, 0 is returned
///
/// The Test parameter which defines the criteria, is a pointer to a member
/// function (this is how it's different from FirstThat above) that takes a
/// pointer to a TWindow & a pointer to void
///
/// The TWindow pointer will be used as the pointer to the child window and the
/// void pointer as a pointer to the Test function's additional parameters
///
/// The Test function must return a Boolean value indicating whether the child
/// passed meets the criteria
TWindow*
TWindow::FirstThat(TCondMemFunc test, void* paramList)
{
  if (ChildList) {
    TWindow*  nextChild = ChildList->Next();
    TWindow*  curChild;

    do {
      CHECKX(nextChild, _T("The child list was corrupted/modified while traversing it"));
      curChild = nextChild;
      nextChild = nextChild->Next();

      if ((this->*test)(curChild, paramList))
        return curChild;
    } while (curChild != ChildList && ChildList);
  }
  return 0;
}

//
/// Iterates over each child window in the TWindow's ChildList,
/// calling the member function (unlike ForEach above which takes pointer
/// to non-member function) whose pointer is passed as the Action to
/// be performed for each child
///
/// A pointer to a child is passed as the first parameter to the iteration
/// procedure
//
void
TWindow::ForEach(TActionMemFunc action, void* paramList)
{
  if (ChildList) {
    TWindow*  nextChild = ChildList->Next();
    TWindow*  curChild;

    do {
      CHECKX(nextChild, _T("The child list was corrupted/modified while traversing it"));
      curChild = nextChild;
      nextChild = nextChild->Next();
      (this->*action)(curChild, paramList);
    } while (curChild != ChildList && ChildList);
  }
}

#endif

//
/// Returns the position at which the passed child window appears in the TWindow's ChildList.
/// Position 0 references the first child, as returned by TWindow::GetFirstChild.
///
/// \returns -1 is returned, if the child does not appear in the list.
//
int TWindow::IndexOf(const TWindow* child) const
{
  auto i = 0;
  for (auto& c : GetChildren())
    if (&c == child)
      return i;
    else
      ++i;
  return -1;
}

//
/// Returns the child at the passed position in the TWindow's ChildList
/// Position 0 references the first child, as returned by TWindow::GetFirstChild.
///
/// \returns `nullptr` is returned, if the position is negative.
///
/// \note Passing a position larger or equal to the number of children, i.e. `At(i)` for *i* larger
/// or equal to *n*, where *n* equals the value returned by TWindow::NumChildren, will be
/// equivalent to `At(i % n)`. This behaviour is for backwards compatibility reasons, for legacy
/// code that depends on the original implementation, which did circular traversal. However, the
/// list is never traversed circularly in the current implementation. Instead, `i % n` is
/// calculated as the number of links to traverse.
//
TWindow* TWindow::At(int position)
{
  if (position < 0) return nullptr;
  position %= NumChildren();
  auto i = 0;
  for (auto& w : GetChildren())
    if (i == position)
      return &w;
    else
      ++i;
  return nullptr; // Should never get here, though.
}

//
/// Returns a pointer to the window in the child window list that has the supplied
/// id. Returns 0 if no child window has the indicated id.
//
auto TWindow::ChildWithId(int id) -> TWindow*
{
  auto c = GetChildren();
  const auto i = find_if(c.begin(), c.end(), [id](TWindow& w) { return w.GetId() == id; });
  return (i != c.end()) ? &(*i) : nullptr;
}

//
/// Sends a message (msg) to a specified window or windows. After it calls the
/// window procedure, it waits until the window procedure has processed the message
/// before returning.
//
TResult
TWindow::SendMessage(TMsgId msg, TParam1 param1, TParam2 param2) const
{
  PRECONDITION(GetHandle());

  TResult result = ::SendMessage(GetHandle(), msg, param1, param2);
  GetApplication()->ResumeThrow();
  return result;
}

//
/// Forwards the window's current message. Calls SendMessage if send is true;
/// otherwise calls PostMessage.
//
TResult
TWindow::ForwardMessage(HWND handle, bool send)
{
  if (!handle)
    return 0;

  TCurrentEvent& currentEvent = GetCurrentEvent();
  if (send) {
    TResult result = ::SendMessage(handle, currentEvent.Message,
                                   currentEvent.Param1,
                                   currentEvent.Param2);
    GetApplication()->ResumeThrow();
    return result;
  }
  else
    return ::PostMessage(handle, currentEvent.Message,
                         currentEvent.Param1,
                         currentEvent.Param2);
}

//
/// Forwards the window's current message. Calls SendMessage if send is true;
/// otherwise calls PostMessage.
//
TResult
TWindow::ForwardMessage(bool send)
{
  TCurrentEvent& currentEvent = GetCurrentEvent();
  if (send)
    return HandleMessage(currentEvent.Message, currentEvent.Param1,
                         currentEvent.Param2);
  return ForwardMessage(GetHandle(), send);
}

//
/// Sends the specified message to all immediate children using SendMessage.
/// \note Includes non-object windows
//
void
TWindow::ChildBroadcastMessage(TMsgId msg, TParam1 param1, TParam2 param2)
{
  for (HWND hWndChild = GetWindow(GW_CHILD); hWndChild; ) {
    HWND hWndNext = ::GetWindow(hWndChild, GW_HWNDNEXT);
    ::SendMessage(hWndChild, msg, param1, param2);
    GetApplication()->ResumeThrow();
    hWndChild = hWndNext;
  }
}

//
/// Posts messages when the mouse pointer leaves a window or hovers over a
/// window for a specified amount of time.
/// Encapsulates the eponymous Windows API function.
/// http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646265.aspx
//
bool
TWindow::TrackMouseEvent(uint flags, int hoverTime)
{
  TRACKMOUSEEVENT a = {sizeof(TRACKMOUSEEVENT), flags, GetHandle(), static_cast<DWORD>(hoverTime)};
  return ::TrackMouseEvent(&a) != FALSE;
}

//
/// Encapsulates a call to TrackMouseEvent, passing the TME_CANCEL flag.
/// See TrackMouseEvent.
//
bool
TWindow::CancelMouseEvent(uint flags)
{
  return TrackMouseEvent(flags | TME_CANCEL);
}

//
/// Returns the current state of mouse event tracking initiated by TrackMouseEvent.
/// Encapsulates a call to ::TrackMouseEvent, passing the TME_QUERY flag.
/// See TrackMouseEvent and Windows API structure TRACKMOUSEEVENT.
/// http://msdn.microsoft.com/en-gb/library/windows/desktop/ms645604.aspx
//
TRACKMOUSEEVENT
TWindow::QueryMouseEventTracking() const
{
  TRACKMOUSEEVENT a = {sizeof(TRACKMOUSEEVENT), TME_QUERY, GetHandle()};
  BOOL r = ::TrackMouseEvent(&a);
  CHECK(r); InUse(r);
  return a;
}

//
/// This version of ShutDownWindow unconditionally shuts down a given window, calls
/// Destroy on the interface element, and then deletes the interface object. Instead
/// of using ShutDownWindow, you can call Destroy directly and then delete the
/// interface object.
///
/// \note This function is static to avoid side effects of deleting 'this'.
//
void
TWindow::ShutDownWindow(TWindow* win, int retVal)
{
  win->Destroy(retVal);
  delete win;
}

//
/// Copies title to an allocated string pointed to by title. Sets the caption of the
/// interface element to title. Deletes any previous title.
/// If the given title is 0, then the internal caption is initialized to 0, and no update
/// of the interface element is done.
//
void
TWindow::SetCaption(LPCTSTR title)
{
  if (Title != title) {
    if (Title)
      delete[] Title;
    Title = title ? strnewdup(title) : nullptr;
  }

  if (Title && GetHandle())
    ::SetWindowText(GetHandle(), Title);
}

//
/// Sets the window title to the resource string identified by the given id.
//
void
TWindow::SetCaption(uint resourceStringId)
{
  SetCaption(LoadString(resourceStringId));
}

//
/// Updates the TWindow internal caption (Title) from the current window's caption.
/// GetWindowTextTitle is used to keep Title synchronized with the actual window
/// state when there is a possibility that the state might have changed.
//
void
TWindow::GetWindowTextTitle()
{
  // NB! Previously (<= 6.30) Title was here checked against 0xXXXXFFFF; a flag for "don't-change".
  // See comment in TDialog::Init for more details.

  if (Title)
    delete[] Title;

  int titleLength = GetWindowTextLength();
  if (titleLength < 0) {
    Title = strnewdup(_T(""));
  }
  else {
    Title = new  tchar[titleLength + 1];
    Title[0] = 0;
    Title[titleLength] = 0;
    GetWindowText(Title, titleLength + 1);
  }
}

//
/// Copies the style, coordinate, and the resource id (but not the title) from the
/// existing HWnd into the TWindow members.
/// \note The title is not copied here, but in GetWindowTextTitle()
//
void
TWindow::GetHWndState(bool forceStyleSync)
{
  // Retrieve Attr.Style and Attr.ExStyle
  //
  // NOTE: some windows controls (e.g. EDIT) change the style bits
  // (e.g. WS_BORDER) from their original values.  if we always reset
  // Attr.Style and Attr.ExStyle by extracting their values from
  // Windows, we will lose some of the style bits we supplied
  // in the CreateWindowEx call.  in the case of the ResourceId
  // constructors, of course, we must retrieve these values.
  //
  if (IsFlagSet(wfFromResource) || forceStyleSync) {
    Attr.Style = GetWindowLong(GWL_STYLE);
    Attr.ExStyle = GetWindowLong(GWL_EXSTYLE);
  }

  // Retrieve Attr.X, Attr.Y, Attr.W and Attr.H
  //
  TRect  wndRect;

  GetWindowRect(wndRect);
  Attr.H = wndRect.Height();
  Attr.W = wndRect.Width();

  HWND  hParent = GetParentH();
  if ((Attr.Style & WS_CHILD) && hParent)
    ::ScreenToClient(hParent, &wndRect.TopLeft());

  Attr.X = wndRect.left;
  Attr.Y = wndRect.top;

  Attr.Id = GetWindowLong(GWL_ID);
}

//
/// Translates the text of a specified control into an integer value and returns it.
/// The parameter 'translated' points to a variable that is set to 'true' on success, 'false' otherwise.
/// The parameter 'isSigned' indicates that the retrieved value is signed (the default).
/// \note Wraps the corresponding function in the Windows API.
//
uint
TWindow::GetDlgItemInt(int childId, bool* translated, bool isSigned) const
{
  PRECONDITION(GetHandle());
  BOOL tempTranslated;
  uint retVal = ::GetDlgItemInt(GetHandle(), childId, &tempTranslated, isSigned);
  if (translated)
    *translated = tempTranslated;
  return retVal;
}

//
// Implementation functions for style getters and mutators.
//
namespace {

template <int GwlStyle, uint32 TWindowAttr::*StyleMem>
auto GetStyle_(const TWindow* w) -> uint32
{
  return w->GetHandle() ? w->GetWindowLong(GwlStyle) : (w->Attr.*StyleMem);
}

template <int GwlStyle, uint32 TWindowAttr::*StyleMem>
auto SetStyle_(TWindow* w, uint32 style) -> uint32
{
  const auto setStyleMem = [](TWindow* w, uint32 style)
  {
    const auto oldstyle = w->Attr.*StyleMem;
    w->Attr.*StyleMem = style;
    return oldstyle;
  };
  return w->GetHandle() ? w->SetWindowLong(GwlStyle, style) : setStyleMem(w, style);
}

template <uint32 (TWindow::*GetStyleMemFun)() const, uint32 (TWindow::*SetStyleMemFun)(uint32)>
auto ModifyStyle_(TWindow* w, uint32 offBits, uint32 onBits, uint swpFlags) -> bool
{
  const auto style = (w->*GetStyleMemFun)();
  const auto newStyle = (style & ~offBits) | onBits;
  const auto isDifferent = style != newStyle;
  if (isDifferent)
  {
    (w->*SetStyleMemFun)(newStyle);
    if (swpFlags != 0)
      w->SetWindowPos(nullptr, {}, swpFlags | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
  }
  return isDifferent;
}

} // anonymous namespace

//
/// Gets the style bits of the underlying window or the 'Style' member of the
/// attribute structure associated with this TWindow object.
//
auto TWindow::GetStyle() const -> uint32
{
  return GetStyle_<GWL_STYLE, &TWindowAttr::Style>(this);
}

//
/// Sets the style bits of the underlying window or the 'Style' member of the
/// attribute structure associated with this TWindow object.
//
auto TWindow::SetStyle(uint32 style) -> uint32
{
  return SetStyle_<GWL_STYLE, &TWindowAttr::Style>(this, style);
}

//
/// Gets the extra style bits of the window.
//
auto TWindow::GetExStyle() const -> uint32
{
  return GetStyle_<GWL_EXSTYLE, &TWindowAttr::ExStyle>(this);
}

//
/// Sets the extra style bits of the window.
//
auto TWindow::SetExStyle(uint32 style) -> uint32
{
  return SetStyle_<GWL_EXSTYLE, &TWindowAttr::ExStyle>(this, style);
}

//
/// Modifies the style bits of the window.
//
auto TWindow::ModifyStyle(uint32 offBits, uint32 onBits, uint swpFlags) -> bool
{
  return ModifyStyle_<&TWindow::GetStyle, &TWindow::SetStyle>(this, offBits, onBits, swpFlags);
}

//
/// Modifies the style bits of the window.
//
auto TWindow::ModifyExStyle(uint32 offBits, uint32 onBits, uint swpFlags) -> bool
{
  return ModifyStyle_<&TWindow::GetExStyle, &TWindow::SetExStyle>(this, offBits, onBits, swpFlags);
}

//
/// Gets the screen coordinates of the window's rectangle and copies them into rect.
/// \note Gets the window rectangle whether it has been created or not
//
void
TWindow::GetWindowRect(TRect& rect) const {
  if (GetHandle()) {
    ::GetWindowRect(GetHandle(), &rect);
  }
  else {
    rect.SetWH(Attr.X, Attr.Y, Attr.W, Attr.H);
  }
}

//
/// Gets the coordinates of the window's client area and then copies them into the
/// object referred to by TRect.
/// \note Gets the window's client rectangle whether it has been created or not
//
void
TWindow::GetClientRect(TRect& rect) const {
  if (GetHandle()) {
    ::GetClientRect(GetHandle(), &rect);
  }
  else {
    rect.Set(0, 0, Attr.W, Attr.H);
  }
}

//
/// Set the new window position.
//
/// Changes the size of the window pointed to by x, y, w, and h. flags contains one
/// of the SWP_Xxxx  Set Window Position constants that specify the size and
/// position of the window. If flags is set to SWP_NOZORDER, SetWindowPos ignores
/// the hWndInsertAfter parameter and retains the current ordering of the child,
/// pop-up, or top-level windows.
///
/// SWP_Xxxx Set Window Position Constants
///
/// - \c \b SWP_DRAWFRAME  Draws a frame around the window.
/// - \c \b SWP_FRAMECHANGED  Sends a message to the window to recalculate the window's
/// size. If this flag is not set, a recalculate size message is sent only at the
/// time the window's size is being changed.
/// - \c \b SWP_HIDEWINDOW  Hides the window.
/// - \c \b SWP_NOACTIVATE  Does not activate the window. If this flag is not set, the
/// window is activated and moved to the top of the stack of windows.
/// - \c \b SWP_NOCOPYBITS  Discards the entire content area of the client area of the
/// window. If this flag is not set, the valid contents are saved and copied into
/// the window after the window is resized or positioned.
/// - \c \b SWP_NOMOVE  Remembers the window's current position.
/// - \c \b SWP_NOSIZE  Remembers the window's current size.
/// - \c \b SWP_NOREDRAW  Does not redraw any changes to the window. If this flag is set, no
/// repainting of any window area (including client, nonclient, and any window part
/// uncovered as a result of a move) occurs. When this flag is set, the application
/// must explicitly indicate if any area of the window is invalid and needs to be
/// redrawn.
/// - \c \b SWP_NOZORDER  Remembers the current Z-order (window stacking order).
/// - \c \b SWP_SHOWWINDOW  Displays the window.
//
bool
TWindow::SetWindowPos(HWND hWndInsertAfter,
                      int x, int y, int w, int h,
                      uint flags)
{
  if (GetHandle())
    return ::SetWindowPos(GetHandle(), hWndInsertAfter, x, y, w, h, flags);

  if (!(flags & SWP_NOMOVE)) {
    Attr.X = x;
    Attr.Y = y;
  }
  if (!(flags & SWP_NOSIZE)) {
    Attr.W = w;
    Attr.H = h;
  }

  // !CQ Can't do much with Z-Order or owner Z-Order

  if (flags & SWP_SHOWWINDOW)
    Attr.Style |= WS_VISIBLE;
  else if (flags & SWP_HIDEWINDOW)
    Attr.Style &= ~WS_VISIBLE;

  return true;
}

//
/// Displays this TWindow in a given state. Can be called either before or after
/// the Window is created. If before, the show state is placed into Attr for use
/// at creation
///
/// After ensuring that the TWindow interface element has a valid handle, ShowWindow
/// displays the TWindow on the screen in a manner specified by cmdShow, which can
/// be one of the following SW_Xxxx Show Window constants:
/// - \c \b SW_SHOWDEFAULT  Show the window in its default configuration. Should be used at
/// startup.
/// - \c \b SW_HIDE  Hide the window and activate another window.
/// - \c \b SW_MINIMIZE  Minimize the window and activate the top-level window in the list.
/// - \c \b SW_RESTORE  Same as SW_SHOWNORMAL.
/// - \c \b SW_SHOW  Show the window in the window's current size and position.
/// - \c \b SW_SHOWMAXIMIZED  Activate and maximize the window.
/// - \c \b SW_SHOWMINIMIZED  Activate window as an icon.
/// - \c \b SW_SHOWNA  Display the window as it is currently.
/// - \c \b SW_SHOWMINNOACTIVE  Display the window as an icon.
/// - \c \b SW_SHOWNORMAL  Activate and display the window in its original size and position.
/// - \c \b SW_SHOWSMOOTH  Show the window after updating it in a bitmap.
//
bool
TWindow::ShowWindow(int cmdShow)
{
  // If the window is being minimzed send a WM_SYSCOMMAND; this way the
  // frame window focus saving works properly
  // !CQ do we still need this with final owl2 focus saving?
  //
  if (gBatchMode)
    return true;
  if (GetHandle()) {
    if (cmdShow == SW_MINIMIZE)
      return HandleMessage(WM_SYSCOMMAND, SC_MINIMIZE);

    else
      return ::ShowWindow(GetHandle(), cmdShow);
  }

  switch (cmdShow) {
    case SW_HIDE:
      Attr.Style &= ~WS_VISIBLE;
      break;
    case SW_SHOWNORMAL:
    case SW_RESTORE:
      Attr.Style |= WS_VISIBLE;
      Attr.Style &= ~(WS_MINIMIZE | WS_MAXIMIZE);
      break;
    case SW_SHOWMINIMIZED:
    case SW_MINIMIZE:
    case SW_SHOWMINNOACTIVE:
      Attr.Style |= WS_VISIBLE;
      Attr.Style |= WS_MINIMIZE;
      break;
    case SW_SHOWMAXIMIZED:
      Attr.Style |= WS_VISIBLE;
      Attr.Style |= WS_MAXIMIZE;
      break;
    case SW_SHOWNOACTIVATE:
    case SW_SHOW:
    case SW_SHOWNA:
      Attr.Style |= WS_VISIBLE;
      break;
  }
  return true;
}

//
/// Sets the mouse cursor for the window, loading the given `resId` from the given `module`.
/// If `module` is `nullptr`, then `resId` can be one of the IDC_xxxx constants that represent
/// different kinds of cursors. See the Windows documentation for a list of valid values.
/// If the mouse is over the client area, the function immediately updates the cursor.
/// If `resId` is 0, then the cursor for the window class is used, according to the default
/// processing for the WM_SETCURSOR message (see EvSetCursor).
//
bool
TWindow::SetCursor(TModule* module, TResId resId)
{
  if (module == CursorModule && resId == CursorResId)
    return false;

  HCURSOR hOldCursor = (HCursor && CursorModule) ? HCursor : nullptr;

  CursorModule = module;
  CursorResId = resId;
  if (CursorResId)
    if (CursorModule)
      HCursor = CursorModule->LoadCursor(CursorResId);
    else
      HCursor = ::LoadCursor(nullptr, CursorResId.GetPointerRepresentation());
  else
    HCursor = nullptr;

  // If the cursor is in our client window then set it now
  //
  if (GetHandle()) {
    TPoint p;
    GetCursorPos(p);
    ScreenToClient(p);
    if (GetClientRect().Contains(p))
    {
      auto getClassCursor = [&] { return reinterpret_cast<HCURSOR>(::GetClassLongPtr(GetHandle(), GCLP_HCURSOR)); };
      ::SetCursor(HCursor ? HCursor : getClassCursor());
    }
  }

  // Destroy old cursor if there was one & it was not loaded from USER
  //
  if (hOldCursor)
    ::DestroyCursor(hOldCursor);
  return true;
}

//
/// Handle WM_INITMENUPOPUP while embeded to generate command enable messages
/// for our server menu items. Very similar to TFrameWindow::EvInitMenuPopup;
/// could rearrange code to share better.
//
void
TWindow::EvInitMenuPopup(HMENU hPopupMenu, uint /*index*/, bool isSysMenu)
{
  if (IsFlagSet(wfAlias))
    DefaultProcessing();

  else if (!isSysMenu && hPopupMenu) {
    const int count = ::GetMenuItemCount(hPopupMenu);

    for (int pos = 0; pos < count; pos++) {
      uint  id;

      if (hPopupMenu == GetMenu()) // top level menu
        id = ::GetMenuItemID(hPopupMenu, pos);

      else {
        // For second level and below menus, return the implied id for popup
        // sub-menus. The implied id for a sub-menu is the id of the first item
        // in the popup sub-menu less 1. If there are more than one level of
        // popup menus, it will recursively look into those sub-menus as well.
        //
        TMenu popupMenu(hPopupMenu);
        id = popupMenu.GetMenuItemID(pos);
      }

      // Ignore separators
      //
      if (id == 0 || uint16(id) == uint16(-1))
        continue;

      // Skip the rest if it is the mdi child list, or system commands
      //
      if (id == static_cast<uint>(IDW_FIRSTMDICHILD) || id > 0xF000)
        break;

      TMenuItemEnabler mie(hPopupMenu, id, GetHandle(), pos);
      EvCommandEnable(mie);
    }
  }
}

//
/// Sets the font that a control uses to draw text. 
/// If the given font owns the underlying font handle (HFONT), the window will share ownership. If
/// the given font does not own the handle, neither will the window. In any case, the given TFont
/// object does not need to outlive the window. However, if the underlying font handle is not
/// owned, the caller must make sure that the font handle outlives the window. In this last case,
/// the call is equivalent to `SetWindowFont (font.GetHandle(), redraw)`.
/// \note Wrapper for Windows API.
//
void
TWindow::SetWindowFont(const TFont& font, bool redraw)
{
  WARN(!font.IsHandleOwner(), _T("The given font has (a font handle with) unmanaged lifetime."));
  if (!font.IsHandleOwner())
    return SetWindowFont(font.GetHandle(), redraw);

  // Note: The TFont copy constructor shares the font handle. It does not create a new font.
  //
  Font = make_unique<TFont>(font);
  SetWindowFont(Font->GetHandle(), redraw);
}

//
/// Associates a pop-up menu with the window so that it can automatically handle a
/// WM_CONTEXTMENU message.
//
// !CQ Is this unusual/confusing to take the pointer & own it, & not a ref
// !CQ & copy it???
//
void
TWindow::AssignContextMenu(TPopupMenu* popupMenu)
{
  delete ContextPopupMenu;
  ContextPopupMenu = popupMenu;
}

//
/// The default message handler for WM_CONTEXTMENU.
/// Respond to a right button click or VK_APPS key press in the window.
/// If a context menu is set, display it.
//
void
TWindow::EvContextMenu(HWND child, int x, int y)
{
  TPopupMenu* m = GetContextMenu();
  if (!m)
  {
    DefaultProcessing();
    return;
  }

  TPoint p(x, y);
  bool invalidPos = (x < 0 && y < 0);
  if (invalidPos)
  {
    // The message was probably generated by the VK_APPS key (sets x = y = -1).
    // See the Windows API documentation for WM_CONTEXTMENU.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms647592.aspx
    // In this case, we open the menu in the upper left corner of the given window.
    //
    p = TPoint(0, 0);
    ::ClientToScreen(child, &p);
  }
  else
  {
    // Provide additional help support by reporting the click position to the main window.
    //
    THelpHitInfo hit(p, this);
    GetApplication()->GetMainWindow()->HandleMessage(WM_OWLHELPHIT, 0, reinterpret_cast<TParam2>(&hit));
  }
  m->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, p, 0, GetHandle());
}

//
/// The default message handler for WM_ENTERIDLE.
//
void
TWindow::EvEnterIdle(uint source, HWND hWndDlg)
{
  // If we're the main window, let it rip from the receiver
  //
  if (source == MSGF_DIALOGBOX) {
    if (IsFlagSet(wfMainWindow))
      IdleAction(0);
    else {
      TWindow* win = GetWindowPtr(hWndDlg);
      if (win)
        win->IdleAction(0);
    }
  }

  // Let original proc. have a crack at msg.
  //
  DefaultProcessing();
}

//
/// Wrapper for Windows API.
/// Returns display and placement information (normal, minimized, and maximized) about the window.
/// Throws TXOwl on failure. To get extended error information, call GetLastError.
//
auto TWindow::GetWindowPlacement() const -> WINDOWPLACEMENT
{
  PRECONDITION(GetHandle());
  auto wp = WINDOWPLACEMENT{sizeof(WINDOWPLACEMENT)};
  const auto r = ::GetWindowPlacement(GetHandle(), &wp);
  if (!r) throw TXOwl{_T("TWindow::GetWindowPlacement failed")};
  return wp;
}

//
/// Wrapper for Windows API.
/// Sets the window to a display mode and screen position. `place` points to a window
/// placement structure that specifies whether the window is to be hidden, minimized
/// or displayed as an icon, maximized, restored to a previous position, activated
/// in its current form, or activated and displayed in its normal position.
/// Throws TXOwl on failure. To get extended error information, call GetLastError.
//
void TWindow::SetWindowPlacement(const WINDOWPLACEMENT& place)
{
  PRECONDITION(GetHandle());
  auto wp = place;
  wp.length = sizeof(WINDOWPLACEMENT);
  const auto r = ::SetWindowPlacement(GetHandle(), &wp);
  if (!r) throw TXOwl{_T("TWindow::SetWindowPlacement failed")};
}

#if defined(OWL5_COMPAT)

//
/// Overload for backwards compatibility.
/// Returns `true` if successful and `false` on failure.
/// To get extended error information, call GetLastError.
//
bool TWindow::GetWindowPlacement(WINDOWPLACEMENT* place) const
{
  PRECONDITION(place);
  try
  {
    *place = GetWindowPlacement();
    return true;
  }
  catch (const TXOwl&)
  {
    return false;
  }
}

//
/// Overload for backwards compatibility.
/// Returns `true` if successful and `false` on failure.
/// To get extended error information, call GetLastError.
//
bool TWindow::SetWindowPlacement(const WINDOWPLACEMENT* place)
{
  PRECONDITION(place);
  try
  {
    SetWindowPlacement(*place);
    return true;
  }
  catch (const TXOwl&)
  {
    return false;
  }
}

#endif

//
/// Overload for TRegion.
/// \note The system takes ownership of the region handle, hence you must pass a moveable argument.
/// For example, `SetWindowRgn(TRegion{...})` or `SetWindowRegion(std::move(rgn))`.
/// \sa TWindow::SetWindowRgn(HRGN, bool)
//
auto TWindow::SetWindowRgn(TRegion&& r, bool repaint) -> bool
{
  PRECONDITION(GetHandle());
  return ::SetWindowRgn(GetHandle(), r.Release(), repaint) != 0;
}

//
/// Retrieves the properties of the given scroll bar.
/// The 'scrollInfo' parameter must be properly initialized according
/// to the Windows API documentation for SCROLLINFO.
/// Returns true on success.
/// \note Wrapper for Windows API.
//
bool
TWindow::GetScrollInfo(int bar, SCROLLINFO* scrollInfo) const
{
  PRECONDITION(GetHandle());
  return ::GetScrollInfo(GetHandle(), bar, scrollInfo);
}

//
/// Function-style overload
/// Returns selected properties of the given scroll bar.
/// Valid values for the 'mask' parameter are the same as for the
/// SCROLLINFO::fMask member documented by the Windows API.
/// \note On failure, SCROLLINFO::nMin, nMax, nPage, nPos and nTrackPos
/// are all left value-initialized (0).
//
SCROLLINFO
TWindow::GetScrollInfo(int bar, uint mask) const
{
  SCROLLINFO i = {sizeof(SCROLLINFO), mask};
  bool r = GetScrollInfo(bar, &i);
  WARNX(OwlWin, !r, 0, _T("GetScrollInfo failed.")); InUse(r);
  return i;
}

//
/// Sets the properties of the given scroll bar.
/// Returns the current position of the scroll bar thumb.
/// \note Wrapper for Windows API.
//
int
TWindow::SetScrollInfo(int bar, SCROLLINFO* scrollInfo, bool redraw)
{
  PRECONDITION(GetHandle());
  return ::SetScrollInfo(GetHandle(), bar, scrollInfo, redraw);
}

//
/// Returns the thumb position in the scroll bar. The position returned is relative
/// to the scrolling range. If bar is SB_CTL, it returns the position of a control
/// in the scroll bar. If bar is SB_HORZ, it returns the position of a horizontal
/// scroll bar. If bar is SB_VERT, it returns the position of a vertical scroll bar.
//
int
TWindow::GetScrollPos(int bar) const
{
  return GetScrollInfo(bar, SIF_POS).nPos;
}

//
/// Sets the thumb position in the scroll bar. Parameter 'bar' identifies the position
/// (horizontal, vertical, or scroll bar control) to return and can be one of the
/// SB_Xxxx scroll bar constants.
/// Returns the current position of the scroll bar thumb.
//
int
TWindow::SetScrollPos(int bar, int pos, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_POS, 0, 0, 0, pos};
  return SetScrollInfo(bar, &i, redraw);
}

//
/// Returns the thumb track position in the scroll bar. Call this function only during
/// the processing of a scroll message with the SB_THUMBTRACK or SB_THUMBPOSITION code.
/// See GetScrollPos for valid values for the 'bar' parameter.
//
int
TWindow::GetScrollTrackPos(int bar) const
{
  return GetScrollInfo(bar, SIF_TRACKPOS).nTrackPos;
}

//
/// Returns the minimum and maximum positions in the scroll bar. If bar is SB_CTL,
/// it returns the position of a control in the scroll bar. If bar is SB_HORZ, it
/// returns the position of a horizontal scroll bar. If bar is SB_VERT, it returns
/// the position of a vertical scroll bar. minPos and maxPos hold the lower and
/// upper range, respectively, of the scroll bar positions. If there are no scroll
/// bar controls, or if the scrolls are non-standard, minPos and maxPos are zero.
//
void
TWindow::GetScrollRange(int bar, int& minPos, int& maxPos) const
{
  SCROLLINFO i = GetScrollInfo(bar, SIF_RANGE);
  minPos = i.nMin;
  maxPos = i.nMax;
}

//
/// Function-style overload
//
TWindow::TScrollRange
TWindow::GetScrollRange(int bar) const
{
  SCROLLINFO i = GetScrollInfo(bar, SIF_RANGE);
  return TScrollRange(i.nMin, i.nMax);
}

//
/// Sets the thumb position in the scroll bar. bar identifies the position
/// (horizontal, vertical, or scroll bar control) to set and can be one of the
/// SB_Xxxx scroll bar constants. minPos and maxPos specify the lower and upper
/// range, respectively, of the scroll bar positions.
//
void
TWindow::SetScrollRange(int bar, int minPos, int maxPos, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_RANGE, minPos, maxPos};
  SetScrollInfo(bar, &i, redraw);
}

//
/// Overload taking the range as a pair
//
void
TWindow::SetScrollRange(int bar, const TScrollRange& r, bool redraw)
{
  SetScrollRange(bar, r.first, r.second, redraw);
}

//
/// Returns the page property (SCROLLINFO::nPage) of the given scroll bar.
//
int
TWindow::GetScrollPage(int bar) const
{
  return GetScrollInfo(bar, SIF_PAGE).nPage;
}

//
/// Sets the page property (SCROLLINFO::nPage) of the given scroll bar.
//
void
TWindow::SetScrollPage(int bar, int page, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_PAGE, 0, 0, static_cast<UINT>(page)};
  SetScrollInfo(bar, &i, redraw);
}

//
//
//
void
TWindow::EnableTooltip(bool enable)
{
  if (!Tooltip) {

    // To circumvent this scenario, we'll look for a window which is fairly
    // stable/rooted as owner of the tooltip. Ideally, we'll get the
    // application's main window.
    //
    TWindow* tipParent = this;

/*  // check it what if window -> is closed but tooltip live?????????
    // it is for gastget only ????????????????????
    while (tipParent->GetParentO()){
      tipParent = tipParent->GetParentO();
      if (tipParent->IsFlagSet(wfMainWindow))
        break;
    }
*/
    // Create and initialize tooltip
    //
    SetTooltip(new TTooltip(tipParent));
  }
  else {
    if (Tooltip->GetHandle())
      Tooltip->Activate(enable);
  }
}

//
// Set a specific tooltip for this window. Assume we now own the ToolTip
//
void
TWindow::SetTooltip(TTooltip* tooltip)
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
    if (!Tooltip->GetHandle()) {

      // Make sure tooltip is disabled so it does not take input focus
      Tooltip->Attr.Style |= WS_DISABLED;
      Tooltip->Create();
    }
  }
}

//
/// Copies a window's update region into a region specified by region. If erase is
/// true, GetUpdateRgn erases the background of the updated region and redraws
/// nonclient regions of any child windows. If erase is false, no redrawing occurs.
/// If the call is successful, GetUpdateRgn returns a value indicating the kind of
/// region that was updated. If the region has no overlapping borders, it returns
/// SIMPLEREGION; if the region has overlapping borders, it returns COMPLEXREGION;
/// if the region is empty, it returns NULLREGION; if an error occurs, it returns
/// ERROR.
///
/// \note Not inline to avoid requiring gdiobjec.h by window.h just to get TRegion's
/// conversion operator
//
int
TWindow::GetUpdateRgn(TRegion& region, bool erase) const
{
  PRECONDITION(GetHandle());

  return ::GetUpdateRgn(GetHandle(), region, erase);
}


//
/// If a window can process dropped files, DragAcceptFiles sets accept to true.
/// \note Wrapper for Windows API.
//
void
TWindow::DragAcceptFiles(bool accept)
{
  PRECONDITION(GetHandle());
  TShell::DragAcceptFiles(GetHandle(), accept);
}

//
/// Creates and displays a message box that contains a message (text), a title
/// (caption), and icons or push buttons (type). If caption is 0, the default title
/// is displayed. Although flags is set to one push button by default, it can contain
/// a combination of the  MB_Xxxx message constants. This function returns one of
/// the following constants:
/// - \c \b IDABORT  User selected the abort button.
/// - \c \b IDCANCEL  User selected the cancel button.
/// - \c \b IDIGNORE  User selected the ignore button.
/// - \c \b IDNO  User selected the no button.
/// - \c \b IDOK  User selected the OK button
/// - \c \b IDRETRY  User selected the retry button.
/// - \c \b IDYES  User selected the yes button.
//
int
TWindow::MessageBox(LPCTSTR text, LPCTSTR caption, uint flags) const
{
  PRECONDITION(GetApplication());
  PRECONDITION(GetHandle());
  return GetApplication()->MessageBox(*this, text, caption, flags);
}

int
TWindow::MessageBox(uint resId, LPCTSTR caption, uint flags) const
{
  PRECONDITION(GetApplication());
  PRECONDITION(GetHandle());
  PRECONDITION(resId);

  tstring text = LoadString(resId);
  return MessageBox(text.c_str(), caption, flags);
}

int
TWindow::MessageBox(uint resId, const tstring& caption, uint flags) const
{
  PRECONDITION(GetApplication());
  PRECONDITION(GetHandle());
  PRECONDITION(resId);

  tstring text = LoadString(resId);
  return MessageBox(text.c_str(), caption.c_str(), flags);
}

//
/// Displays a standard message box.
/// Wrapper for Windows API.
/// \sa http://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-messageboxindirecta
//
auto TWindow::MessageBoxIndirect(HINSTANCE resourceModule, TResId text, TResId caption, uint styleFlags, TResId icon, DWORD_PTR contextHelpId, MSGBOXCALLBACK callback, DWORD languageId) const -> int
{
  auto p = MSGBOXPARAMS
  {
    sizeof(MSGBOXPARAMS), 
    GetHandle(), 
    resourceModule, 
    text, 
    caption,
    styleFlags,
    icon,
    contextHelpId,
    callback,
    languageId
  };
  return ::MessageBoxIndirect(&p);
}

//
/// Displays a standard message box using the given icon.
/// The flag MB_USERICON is automatically combined with the given flags, so there is no need to specify it.
/// Wrapper for Windows API.
///
/// \note The specified icon resource is assumed to reside in the same module as the window, i.e. the
/// HINSTANCE handle passed to ::MessageBoxIndirect is the value returned by `this->GetModule()->GetHandle()`.
///
/// \sa http://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-messageboxindirecta
//
auto TWindow::MessageBoxIndirect(TResId icon, const tstring& text, const tstring& caption, uint flags) const -> int
{
  PRECONDITION(GetModule() && GetModule()->GetHandle());
  return MessageBoxIndirect(
    GetModule()->GetHandle(), 
    TResId{text.c_str()}, 
    TResId{caption.c_str()},
    flags | MB_USERICON, 
    icon,
    0,
    nullptr,
    0
  );
}

auto TWindow::FormatMessageBox(const tstring& formatString, const tstring& caption, uint flags, ...) const -> int
{
  va_list argp;
  va_start(argp, flags);
  try
  {
    auto format = [](const tstring& fmt, va_list argp) -> tstring
    {
      auto buffer = array<tchar, 1024>{};
      const auto r = _vsntprintf(buffer.data(), buffer.size(), fmt.c_str(), argp);
      if (r == -1)
      {
        buffer.back() = _T('\0');
        WARNX(OwlWin, r == -1, 0, _T("TWindow::FormatMessageBox: Message was truncated."));
      }
      return buffer.data();
    };
    const auto s = format(formatString, argp);
    const auto r = MessageBox(s, caption, flags);
    va_end(argp);
    return r;
  }
  catch (...)
  {
    WARNX(OwlWin, true, 0, _T("TWindow::FormatMessageBox failed."));
    va_end(argp);
    throw;
  }
}

//
//
//
void TWindow::SethAccel(HACCEL _hAccel)
{
  HAccel = _hAccel;
}

//
/// For use with CopyText.
//
struct TWindowGetWindowText
{
  const TWindow& win;
  TWindowGetWindowText(const TWindow& w) : win(w) {}

  int operator()(LPTSTR buf, int buf_size)
  {return win.GetWindowText(buf, buf_size);}
};

//
/// String-aware overload
//
tstring
TWindow::GetWindowText() const
{
  return CopyText(GetWindowTextLength(), TWindowGetWindowText(*this));
}

//
/// For use with CopyText.
//
struct TWindowGetDlgItemText
{
  const TWindow& win;
  int id;
  TWindowGetDlgItemText(const TWindow& w, int id_) : win(w), id(id_) {}

  int operator()(LPTSTR buf, int buf_size)
  {return win.GetDlgItemText(id, buf, buf_size);}
};

//
/// String-aware overload
//
tstring
TWindow::GetDlgItemText(int childId) const
{
  return CopyText(::GetWindowTextLength(GetDlgItem(childId)), TWindowGetDlgItemText(*this, childId));
}

//
//
//
#if defined(__TRACE) || defined(__WARN)
//namespace owl {
  ostream& operator <<(ostream& os, const TWindow& w)
  {
    os << '(';
    os << _OBJ_FULLTYPENAME(&w) << ',';
    os << static_cast<void*>(w.GetHandle()) << ',';
    if (w.GetCaption())
      os << '\'' << w.GetCaption() << '\'' << ',';
    if (w.GetParentO())
      os << "id=" << w.GetId();
    os << ')';
    return os;
  }
//} // OWL namespace
#endif

///////////////////////////////////////////////////////////////////////////////////////
//
TDrawItem* ItemData2DrawItem(ULONG_PTR data)
{
  // 0. If 0
  if(data==0)
    return 0;
  // 1. check to make sure the pointer is valid
  if(IsBadReadPtr((void*)data, sizeof(TDrawItem)))
    return 0;     // quick escape

  // 2. check signature -> starting from 5's byte
#if defined(_WIN64)
  uint32* itemId = (uint32*)((uint8*)data+8);
#else
  uint32* itemId = (uint32*)((uint8*)data+4);
#endif
  if(*itemId != TDrawItemBase::drawId)
    return 0;

  // 3. check to make sure the VTable pointer is valid
  if(IsBadReadPtr(*(void**)data, sizeof(void*)))
    return 0;     // quick escape

#if 0 //?? maby simple casting enauph after checking itemId??
  TDrawItem* item;
  try{
    item = localTryCatchBlock(data);
  }
  catch(...){
    return 0;
  }
  return item;
#else
  return (TDrawItem*)(void*)data;
#endif
}
///////////////////////////////////////////////////////////////////////////////////////


//
// Converts the given integer to a string.
// TODO: Relocate somewhere else more convenient for reuse.
//
static tstring
ToString(int v)
{
  tchar buf[64];
  _stprintf(buf, _T("%d"), v);
  return tstring(buf);
}

//
// Returns a string which kind-of identifies the window (used during autopsy
// and vivisection of dead/dying window).
//
static tstring
GetSuspectDescription(TWindow* w)
{
  _USES_CONVERSION;
  if (!w) return _T("");

  tstring caption = w->GetCaption() ? w->GetCaption() : _T("");
  tstring id = ToString(w->GetWindowAttr().Id);
  tstring type = _A2W(_OBJ_FULLTYPENAME(w));

  tstring s;
  s += _T("\"") + caption + _T("\"");
  s += _T(", ID: ") + id;
  s += _T(", window ") + type;
  s += _T(".");
  return s;
}

//
// Formats an error message defined by the given resource id.
// Appends the last system error message if any.
//
// NB! Note that the TSystemMessage probably is too far from the problem site to give a
// meaningful error message; intermediate system calls have probably happened since
// the exception was thrown.
//
static tstring
MakeTXWindowMessage(TWindow* win, uint resId)
{
  TSystemMessage m;
  tstring s = TXOwl::MakeMessage(resId, GetSuspectDescription(win).c_str());
  if (m.SysError() != ERROR_SUCCESS)
    s += _T("\n\nSystem error: ") + m.SysMessage();
  return s;
}

//
/// Constructs a TXWindow object with a default resource ID of IDS_INVALIDWINDOW.
//
TXWindow::TXWindow(TWindow* win, uint resId)
:
  TXOwl(MakeTXWindowMessage(win, resId), resId),
  Window(win)
{
}

//
/// Copy the exception object.
//
TXWindow::TXWindow(const TXWindow& src)
:
  TXOwl(src),
  Window(src.Window)
{
}

//
/// Unhandled exception.
///
/// Called if an exception caught in the window's message loop has not been handled.
/// Unhandled() deletes the window. This type of exception can occur if a window
/// cannot be created.
//
int
TXWindow::Unhandled(TModule* app, uint promptResId)
{
  Window = 0;
  return TXOwl::Unhandled(app, promptResId);
}

//
/// Clone the exception object for safe-throwing.
//
TXWindow*
TXWindow::Clone()
{
  return new TXWindow(*this);
}


//
/// Throws the exception object. Throw() must be implemented in any class derived
/// from TXOwl.
//
void
TXWindow::Throw()
{
  throw *this;
}

//
/// Creates the TXWindow exception and throws it.
//
void
TXWindow::Raise(TWindow* win, uint resourceId)
{
  TXWindow(win, resourceId).Throw();
}

//
// Returns the window causing the exception.
//
TWindow*
TXWindow::GetWindow() const
{
  return Window;
}

} // OWL namespace

/* ========================================================================== */

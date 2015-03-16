//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TApplication. This defines the basic behavior
/// for ObjectWindows applications.
//----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/applicat.h>
#include <owl/framewin.h>
#include <owl/docmanag.h>
#include <owl/appdict.h>
#include <owl/messageb.h>
#include <owl/window.rh>
#include <owl/tooltip.h>
#include <owl/tracewnd.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif


#if defined(BI_MULTI_THREAD_RTL)
#  include <owl/thread.h>
#endif

using namespace std;

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-inl // Disable warning in standard library.
#endif

namespace owl {
OWL_DIAGINFO;
} // OWL namespace


namespace owl {

/// \addtogroup internal
/// @{


/////////////////////////////////////////////////////
/// global definitions for both modules if sectioning
struct TEnumInfo {
  HWND      ModalWnd;    ///< The window being made modal if needed
  short     Count;      ///< Count of windows in Wnds below
  HWND*     Wnds;        ///< list of windows that were disabled
  WNDPROC*  PrevWndProc;//
};

//////////////////////////////////////////////////////////////////////////////////
// Multithread support 
//
class TWaitHook{
  public:
    TWaitHook();
    ~TWaitHook();
  
    void SetCursor(TCursor* cursor, TAutoDelete = AutoDelete);

  private:
    void FreeCursors();
    void SetWaitCursor();

  private:
    TEnumInfo    Subclass;
    TEnumInfo*  LastStackTop;
    uint        Count;
    uint        HideCaretCount;
    TCursor*    Cursor;
    bool        DeleteOnClose;

    TResult WaitWndMethod(HWND, TMsgId, TParam1, TParam2);
    TResult DefWndProc(HWND, TMsgId, TParam1, TParam2);

  public:
    static LRESULT CALLBACK WaitWndProc(HWND, UINT, WPARAM, LPARAM);
};

//
struct TEnumInfoStr 
#if defined(BI_MULTI_THREAD_RTL)
: public TLocalObject
#endif
{
  
  TEnumInfoStr():Info(0),Hook(0),Top(0)
  {
  }
  ~TEnumInfoStr()
    {
    }

  TEnumInfo* GetInfo()                { return Info; }
  void       SetInfo(TEnumInfo* info) { Info = info; }

  TEnumInfo*    Info;
  TWaitHook*    Hook;
  TWaitCursor*  Top;

#if defined(BI_MULTI_THREAD_RTL)
  TMRSWSection  Lock;  
#endif
};
//
//
static TEnumInfoStr& GetEnumInfo();

//
//
//
#if !defined(BI_MULTI_THREAD_RTL)
struct TApplicatData {
#else
struct TApplicatData : public TLocalObject {
#endif
  TApplicatData();
  ~TApplicatData();
  
  TMessageBox OwlMsgBox;

#if defined(BI_MULTI_THREAD_RTL)
  TMRSWSection  Lock;
#endif
};

//
static TApplicatData& GetApplicatData();

//
//
//
#if defined(BI_MULTI_THREAD_RTL) //TMRSWSection::TLock lock(GetEnumInfo().Lock);
#define LOCKENUMINFO(l,s) TMRSWSection::TLock __lock(l,s) 
#else
#define LOCKENUMINFO(l,s)
#endif

//
//
//
#if defined(BI_MULTI_THREAD_RTL) //GetApplicatData().Lock,true
#define LOCKAPPLDATA(l,s) TMRSWSection::TLock __lock(l,s)
#else
#define LOCKAPPLDATA(l,s)
#endif

/// @}


DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlApp, 1, 0);

// -----------------------------------------------------------------------------
static int 
OWLDefaultMsgBox(HWND wnd, LPCTSTR text, LPCTSTR caption, uint type)
{
  TApplication* app = OWLGetAppDictionary().GetApplication(0);
  return ::MessageBoxEx(wnd, text, caption, type, app ? app->GetLangId() : LangNeutral);
}
// -----------------------------------------------------------------------------
TApplicatData::TApplicatData()
: OwlMsgBox(OWLDefaultMsgBox)
{
}
// -----------------------------------------------------------------------------
TApplicatData::~TApplicatData()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// multithread support
//
static 
TApplicatData& GetApplicatData()
{
//#if defined(BI_MULTI_THREAD_RTL)
//  static TProcessContainer<TApplicatData> __ApplData;
//#else
  static TApplicatData __ApplData;
//#endif
  return __ApplData;
}
//
static 
TEnumInfoStr& GetEnumInfo()
{
//#if defined(BI_MULTI_THREAD_RTL)
//  static TProcessContainer<TEnumInfoStr> __EnumInfo;
//#else
  static TEnumInfoStr  __EnumInfo;
//#endif
  return __EnumInfo;
}

// -----------------------------------------------------------------------------
//
// Static members for initialization of app prior to initial construction
//
HINSTANCE  TApplication::InitHInstance;
HINSTANCE  TApplication::InitHPrevInstance;
int        TApplication::InitCmdShow;


// -----------------------------------------------------------------------------
_OWLFUNC(TMessageBox) 
SetDefMessageBox(TMessageBox newMsgBox)
{
  TApplicatData& data = GetApplicatData();
  LOCKAPPLDATA(data.Lock,true);
  TMessageBox msgBox = data.OwlMsgBox;
  data.OwlMsgBox = newMsgBox;
  return msgBox;
}
// -----------------------------------------------------------------------------
_OWLFUNC(int) 
OWLMessageBox(HWND wnd, const tstring& text, const tstring& caption, uint type)
{
  return OWLMessageBox(wnd, text.c_str(), caption.c_str(), type);
}
// -----------------------------------------------------------------------------
_OWLFUNC(int) 
OWLMessageBox(HWND wnd, LPCTSTR text, LPCTSTR caption, uint type)
{
  // If no parent is supplied need to use task modal to disable all toplevel
  // windows in this task.
  //
  if (!wnd && !(type & MB_SYSTEMMODAL))
    type |= MB_TASKMODAL;
  
  LPCTSTR _caption = caption; // Older compilers like Borland C++ 5.5 do not allow assigning new value to caption
  if (_caption == 0)
    _caption = OWLGetAppDictionary().GetApplication(0)->GetName();
  return (GetApplicatData().OwlMsgBox)(wnd, text, _caption, type);
}
// -----------------------------------------------------------------------------
_OWLFUNC(int)
OWLMessageBox(TWindow* wnd, TResId resId, const tstring& caption, uint type, TModule* module)
{
  return OWLMessageBox(wnd, resId, caption.c_str(), type, module);
}
// -----------------------------------------------------------------------------
_OWLFUNC(int)
OWLMessageBox(TWindow* wnd, TResId resId, LPCTSTR caption, uint type, TModule* module)
{
  TModule& m = module ? *module : wnd && wnd->GetModule() ? *wnd->GetModule() : GetGlobalModule();
  tstring text = m.LoadString(resId.GetInt());
  LPCTSTR _caption = caption; // Older compilers like Borland C++ 5.5 do not allow assigning new value to caption
  if (_caption == 0 && wnd && wnd->GetApplication())
    _caption = wnd->GetApplication()->GetName();
    
  HWND h = wnd ? wnd->GetHandle() : static_cast<HWND>(0);
  return OWLMessageBox(h, text, _caption, type);
}

// -----------------------------------------------------------------------------
//
/// Gets WinMain's 3rd param.
//
tstring&
TApplication::GetInitCmdLine()
{
  static tstring InitCmdLine;
  return InitCmdLine;
};

//
/// Constructor for use in OwlMain(). Gets members from statics set earlier by
/// a call to InitWinMainParams() in Owl's WinMain.
//
/// Creates a new TApplication object named name. You can use owl::Module to specify the
/// global module pointer that points to this application. The  appDict parameter
/// specifies which dictionary this application will insert itself into. To override
/// the default ObjectWindows TAppDictionary object, pass a pointer to a
/// user-supplied appDict object.
//
TApplication::TApplication(const tstring& name, TModule*& module, TAppDictionary* appDict)
:
  TModule(name, InitHInstance, GetInitCmdLine()),
  TMsgThread(TMsgThread::Current),
  // Copy over values that were stashed in static members before this instance
  // was constructed.
  //
  hPrevInstance(InitHPrevInstance), nCmdShow(InitCmdShow),
  DocManager(0), MainWindow(0),
  LangId(LangUserDefault),
  Tooltip(0),
  WaitCount(0),
  WaitHandles(NULL),
  CmdLine(GetInitCmdLine()),
#if defined(OWL_SUPPORT_BWCC)
  BWCCOn(false),BWCCModule(0),
#endif
#if defined(OWL_SUPPORT_CTL3D)
  Ctl3dOn(false),Ctl3dModule(0),
#endif
  CondemnedWindows(0),
  XState(0), XBase(), XWhat(),
  Dictionary(appDict ? appDict : &(OWLGetAppDictionary()))
{
  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication constructing @") << (void*)this);

  Dictionary->Add(this);
  module = this;

  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication constructed @") << (void*)this);
}

//
/// Constructor for use in user defined WinMain() when all the args are
/// available
//
/// Creates a TApplication object with the application name (name), the application
/// instance handle (instance), the previous application instance handle
/// (prevInstance), the command line invoked (cmdLine), and the main window show
/// flag (cmdShow). The  appDict parameter specifies which dictionary this
/// application will insert itself into. To override the default ObjectWindows
/// TAppDictionary object, pass a pointer to a user-supplied appDict object.
/// 
/// If you want to create your own WinMain, use this constructor because it provides
/// access to the various arguments provided by WinMain. You can use module to to
/// specify the global module pointer that points to this application.
//
TApplication::TApplication
  (
    const tstring& name,
    HINSTANCE instance,
    HINSTANCE prevInstance,
    const tstring& cmdLine,
    int cmdShow,
    TModule*& module,
    TAppDictionary* appDict
  )
:
  TModule(name, instance, cmdLine),
  TMsgThread(TMsgThread::Current),
  hPrevInstance(prevInstance), nCmdShow(cmdShow),
  DocManager(0), MainWindow(0),
  LangId(LangUserDefault),
  Tooltip(0),
  WaitCount(0),
  WaitHandles(NULL),
  CmdLine(cmdLine),
#if defined(OWL_SUPPORT_BWCC)
  BWCCOn(false),BWCCModule(0),
#endif
#if defined(OWL_SUPPORT_CTL3D)
  Ctl3dOn(false),Ctl3dModule(0),
#endif
  CondemnedWindows(0),
  XState(0), XBase(), XWhat(),
  Dictionary(appDict ? appDict : &(OWLGetAppDictionary()))
{
  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication constructing @") << (void*)this);

  Dictionary->Add(this);
  module = this;

  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication constructed @") << (void*)this);
}


//
/// ~TApplication destroys the TApplication object.
//
TApplication::~TApplication()
{
  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication destructing @") << (void*)this);

  DeleteCondemned();

#if defined(OWL_SUPPORT_CTL3D)
  // Unregister ourselves from the Ctl3d DLL if it is loaded.
  //
  if (Ctl3dModule) {
    Ctl3dModule->Unregister(*this);
    delete Ctl3dModule;
  }
#endif
#if defined(OWL_SUPPORT_BWCC)
  // Unregister ourselves from the BWCC DLL if it is loaded.
  if (BWCCModule) {
    BWCCModule->IntlTerm();
    delete BWCCModule;
  }
#endif

  // Delete the main window if still present, may be destroyed but not deleted
  // Set MainWindow to 0 first to prevent it from calling ::PostQuitMessage
  //
  TWindow* mainWindow = SetMainWindow(0);
  if (mainWindow) {
    mainWindow->Destroy();
    delete mainWindow;
    delete Tooltip; // if SetMainWindow(0) need manually delete Tooltip
  }

  delete DocManager;

  // Remove this app from the application dictionary that it is in
  //
  Dictionary->Remove(this);

  TRACEX(OwlApp, OWL_CDLEVEL, _T("TApplication destructed @") << (void*)this);
}


//
/// Open a modal message box, Under WIN32 the language id setting is used.
//
int
TApplication::MessageBox(HWND wnd, LPCTSTR text, LPCTSTR caption, uint type)
{
  // Default caption to this application's name
  //
  if (caption == 0)
    caption = GetName();

  // If no parent is supplied need to use task modal to disable all toplevel
  // windows in this task.
  //
  if (!wnd && !(type & MB_SYSTEMMODAL))
    type |= MB_TASKMODAL;

#if defined(OWL_SUPPORT_BWCC)
  // Use the BWCC message box if BWCC is enabled
  //
  if (BWCCEnabled() && GetBWCCModule()) {
    return GetBWCCModule()->MessageBox( wnd, text, caption, type);
  }
  else
#endif
  // Otherwise, 3d-ize the message box if ctl3d is enabled
  //
#if defined(OWL_SUPPORT_CTL3D)
  {
    EnableCtl3dAutosubclass(true);
    int retValue = (GetApplicatData().OwlMsgBox)(wnd, text, caption, type);
    EnableCtl3dAutosubclass(false);
    return retValue;
  }
#else
  return (GetApplicatData().OwlMsgBox)(wnd, text, caption, type);
#endif
}

int 
TApplication::MessageBox(HWND wnd, uint resId, const tstring& caption, uint type, ...)
{
  PRECONDITION(resId);

  TTmpBuffer<tchar> buffer(MAX_PATH);
  tstring text = LoadString(resId);
  uint* nextArgument = &type;
  ++nextArgument;
  wvsprintf(buffer, text.c_str(), (va_list)(nextArgument));

  return MessageBox(wnd, &buffer[0], caption, type);
}

//
//
//
int 
TApplication::MessageBox(HWND wnd, uint resId, LPCTSTR caption, uint type, ...)
{
  PRECONDITION(resId);

  TTmpBuffer<tchar> buffer(MAX_PATH);
  tstring text = LoadString(resId);
  uint* nextArgument = &type;
  ++nextArgument;
  wvsprintf(buffer, text.c_str(), (va_list)(nextArgument));

  return MessageBox(wnd, &buffer[0], caption, type);
}


//
/// Handle initialization for the first executing instance of the OWL
/// application. Under Win32, every app instance is treated as the first.
//
/// Derived classes can override this to perform app initialization, or they
/// can use the derived class constructor.
//
/// The following sample program calls InitApplication the first time an instance of
/// the program begins.
/// \code 
/// class TTestApp : public TApplication {
///    public:
///      TTestApp(): TApplication("Instance Tester")
///        { strcpy(WindowTitle, "Additional Instance");}
///    protected:
///       char WindowTitle[20];
///       void InitApplication() { strcpy(WindowTitle, "First Instance"); }
///       void InitMainWindow() { MainWindow = new TFrameWindow(0, WindowTitle); }
/// };
/// static TTestApp App;
/// \endcode
//
void
TApplication::InitApplication()
{
  TRACEX(OwlApp, 1, _T("TApplication::InitApplication() called @") << (void*)this);
}


//
/// Performs each instance initialization necessary for the application. Unlike
/// InitApplication(), which is called for the first instance of an application,
/// InitInstance is called whether or not there are other executing instances of the
/// application. InitInstance calls InitMainWindow(), and then creates and shows the
/// main window element by TWindow::Create and TWindow::Show. If the main window
/// cannot be created, a TXInvalidMainWindow exception is thrown.
//
/// \note If you redefine this member function, be sure to explicitly call
/// TApplication::InitInstance.
//
void
TApplication::InitInstance()
{
  TMsgThread::InitInstance();

  TRACEX(OwlApp, 1, _T("TApplication::InitInstance() called @") << (void*)this);

  InitMainWindow();

  if (MainWindow) {
    MainWindow->SetFlag(wfMainWindow);
    MainWindow->Create();

#if defined(__TRACE) || defined(__WARN)
    TSystemMenu sysMenu (MainWindow->GetHandle());
    if(sysMenu.IsOK()){
      sysMenu.AppendMenu (MF_SEPARATOR);
      sysMenu.AppendMenu (MF_STRING, SCM_TRACE, _T("Diagnostic Window"));
    }
#endif

    MainWindow->ShowWindow(nCmdShow);
  }
  else
    TXInvalidMainWindow::Raise();
}


//
/// Initialize the application's main window. Derived classes should override
/// this to construct, initialize and set the main window using SetMainWindow().
//
/// By default, InitMainWindow constructs a generic TFrameWindow object with the
/// name of the application as its caption. You can redefine InitMainWindow to
/// construct a useful main window object of TFrameWindow (or a class derived from
/// TFrameWindow) and store it in MainWindow. The main window must be a top-level
/// window; that is, it must be derived from TFrameWindow. A typical use is
/// \code
/// virtual void TMyApp_InitMainWindow(){
///   SetMainWindow(TMyWindow(NULL, Caption));
/// }
/// \endcode
/// InitMainWindow can be overridden to create your own custom window.
//
void
TApplication::InitMainWindow()
{
  SetMainWindow(new TFrameWindow(0, GetName()));
}


//
/// Handle termination for each executing instance of the application. Called
/// at the end of a Run() with the final return status.
//
int
TApplication::TermInstance(int status)
{
  TRACEX(OwlApp, 1, _T("TApplication::TermInstance() called @") << (void*)this);

#if defined(__TRACE) || defined(__WARN)
  // the main window is being destroyed, so destroy the trace window...
  if (TTraceWindow::TraceWindow())
    delete TTraceWindow::TraceWindow();
#endif

  return TMsgThread::TermInstance(status);
}

//
/// Set (or reset) the main window. Return, but do not destroy the previous
/// main window.
//
TFrameWindow*
TApplication::SetMainWindow(TFrameWindow* window)
{
  if (MainWindow) {
    MainWindow->ClearFlag(wfMainWindow);
    uint32 style = MainWindow->GetExStyle();
    if (style & WS_EX_APPWINDOW)
      MainWindow->SetExStyle(style & ~WS_EX_APPWINDOW);
    if(Tooltip && Tooltip->GetParentO()==MainWindow)
      Tooltip->SetParent(0);
  }

  TFrameWindow* oldMainWindow = MainWindow;
  MainWindow = window;

  if (MainWindow) {
    MainWindow->SetFlag(wfMainWindow);
    uint32 style = MainWindow->GetExStyle();
    if (!(style & WS_EX_APPWINDOW))
      MainWindow->SetExStyle(style | WS_EX_APPWINDOW);
    // set new parent only if MainWindow created
    if(Tooltip && Tooltip->GetParentO()==0 && MainWindow->GetHandle())
      Tooltip->SetParent(MainWindow);
  }
  return oldMainWindow;
}


//
/// Set (or resets) the document manager, return the previous one if present
//
TDocManager*
TApplication::SetDocManager(TDocManager* docManager)
{
  TDocManager* oldDocManager = DocManager;
  DocManager = docManager;
  return oldDocManager;
}


#if defined(BI_MULTI_THREAD_RTL)
//
/// BI_MULTI_THREAD_RTL only: Overrides TEventHandler::Dispatch() to handle
/// multi-thread synchonization.
//
TResult TApplication::Dispatch(TEventInfo& info, TParam1 p1, TParam2 p2)
{
  TApplication::TQueueLock lock(*this);
  return TEventHandler::Dispatch(info, p1, p2);
}
#endif

/// Jogy  to prevent error C3204: '_alloca' cannot be called from within a catch block under VC.NET
tstring TApplication::ConvertA2W(LPCSTR string)
{
   _USES_CONVERSION;
   return (tstring(_A2W(string)));
}


//
/// Run this application, return when application terminates
//
/// Initialize instances, create and display their main window (calls
/// InitApplication for the first executing instance and calls InitInstance for
/// all instances). Run the application's message loop. Each of the virtual
/// functions called are expected to throw an exception if there is an error.
///
/// Exceptions are propagated out of this function, but the message queue is still 
/// flushed and TermInstance called in the event of an exception.
//
int
TApplication::Run()
{
  try 
  {
    if (!MainWindow) 
    {
      if (!hPrevInstance)
        InitApplication();
      InitInstance();
    }
    LoopRunning = true;
    int status = MessageLoop();
    FlushQueue();
    LoopRunning = false;
    return TermInstance(status);
  }
  catch (...) 
  {
    FlushQueue();
    LoopRunning = false;
    TermInstance(-1);
    throw;
  }
}


//
/// Start this application and return immediately. Used for component DLLs
//
/// Initializes instances, creating and displaying their main window (calls
/// InitApplication for the first executing instance and calls InitInstance for
/// all instances). Each of the virtual functions called are expected to throw
/// an exception if there is an error. Any exceptions are handled and converted
/// to an error code which is then returned. Returns 0 on success.
/// Does not run message loop.
//
int
TApplication::Start() throw()
{
  int status = 0;
  try {
    if (!hPrevInstance)
      InitApplication();
    InitInstance();
  }
  catch (TXOwl& x) {status = x.Unhandled(this, 0);}
  catch (TXBase& x) {status = Error(x, 0);}
  catch (...) {status = -1;}
  return status;
}


//
/// Operates the application's message loop, which runs during the lifetime of the
/// application. MessageLoop queries for messages. If one is received, it processes
/// it by calling ProcessAppMsg. If the query returns without a message, MessageLoop
/// calls IdleAction to perform some processing during the idle time. 
///
/// MessageLoop calls PumpWaitingMessages to get and dispatch waiting messages. 
/// MessageLoop can be broken if BreakMessageLoop is set by EndModal.
//
int
TApplication::MessageLoop()
{
  long idleCount = 0;
  MessageLoopResult = 0;
  while (!BreakMessageLoop) 
  {
    try 
    {
      if (!IdleAction(idleCount++))
      {
        DWORD r = MsgWaitForMultipleObjects(WaitCount, WaitHandles, FALSE, INFINITE, QS_ALLINPUT);
        if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + WaitCount)
          ObjectSignaled(WaitHandles[r - WAIT_OBJECT_0], false);
        else if (r >= WAIT_ABANDONED_0 && r < WAIT_ABANDONED_0 + WaitCount)
          ObjectSignaled(WaitHandles[r - WAIT_ABANDONED_0], true);
      }
      if (PumpWaitingMessages())
        idleCount = 0;
      ResumeThrow();
    }
    catch (...) 
    {
      BreakMessageLoop = true;
      MessageLoopResult = -1;
      throw;
    }
  }
  BreakMessageLoop = false;
  return MessageLoopResult;
}


//
/// Called each time there are no messages in the queue. Idle count is
/// incremented each time, & zeroed when messages are pumped. Return
/// whether or not more processing needs to be done.
//
/// Default behavior is to give the main window an opportunity to do idle
/// processing by invoking its IdleAction() member function when
/// "idleCount" is 0
//
bool
TApplication::IdleAction(long idleCount)
{
  TRACEX(OwlApp, 1, _T("TApplication::IdleAction() called @") << (void*)this <<
                    _T(" idleCount ") << idleCount);
  bool more = false;
  if (MainWindow)
    more = MainWindow->IdleAction(idleCount);

#if defined(__TRACE) || defined(__WARN)
  if (DiagIdleAction(idleCount))
    more = true;
#endif
  return more;
}

//
/// Called for each message that is pulled from the queue, to perform all
/// translation & dispatching.
/// 
/// Return true to drop out of pump
//
bool
TApplication::ProcessMsg(MSG& msg)
{
  // Let the app preprocess the message. If it is not eaten, then translate
  // it, & dispatch it. If no HWND, then first find/dispatch it directly
  // to the app (for PostAppMessage() functionality)
  //
  if (!ProcessAppMsg(msg)) {
    ::TranslateMessage(&msg);
    if (!msg.hwnd) {
      TEventInfo cmdEventInfo(msg.message, static_cast<uint>(msg.wParam));
      if (Find(cmdEventInfo)) {
        Dispatch(cmdEventInfo, msg.wParam, msg.lParam);
        return true;
      }
      else {
        TEventInfo eventInfo(msg.message);
        if (Find(eventInfo)) {
          Dispatch(eventInfo, msg.wParam, msg.lParam);
          return true;
        }
      }
    }
    ::DispatchMessage(&msg);
    DeleteCondemned();
    ResumeThrow();
  }
  return false;
}

//
/// Called after each message is pulled from the queue, and before it is
/// dispatched. Return true if the message was handled completely here.
//
/// Checks for any special processing that is required for modeless dialog box,
/// accelerator, and MDI accelerator messages. Calls the virtual
/// TWindow::PreProcessMsg function of the window receiving the message. If your
/// application does not create modeless dialog boxes, does not respond to
/// accelerators, and is not an MDI application, you can improve performance by
/// overriding this member function to return false.
//
bool
TApplication::ProcessAppMsg(MSG& msg)
{
#if defined(__TRACE) || defined(__WARN)
  if (msg.message == WM_SYSCOMMAND && (msg.wParam & 0xFFF0) == SCM_TRACE){
    SystemCommandTrace();
    return true;
  }
#endif

  // Check if this message might be worth relaying to the tooltip window
  //
  if (Tooltip && Tooltip->IsWindow()) {
    if (msg.hwnd == *MainWindow || MainWindow->IsChild(msg.hwnd)) {
      if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
        Tooltip->RelayEvent(msg);
      }
    }
  }

  // Start with the window that the event was destined for and allow it
  // and its parent (and its parent...) an opportunity to preprocess the
  // event
  //
  for (HWND hWnd = msg.hwnd; hWnd; hWnd = ::GetParent(hWnd)) {
    // NT seems very picky about calls to ::GetWindowThreadProcessId
    // with the HWND of the desktop. Hence we'll abort this loop
    // when encountering the desktop. Specially useful when dealing with
    // DropDown[List] ComboBoxes thunked by OWL when hit the ComboLBox
    // window which is parented to the Desktop.
    //
    static HWND deskTopHwnd = ::GetDesktopWindow();    
    if (hWnd == deskTopHwnd)
      break;

    TWindow*  win = GetWindowPtr(hWnd);

    if (win && win->PreProcessMsg(msg))
      return true;
  }

  return false;
}

void TApplication::WaitOnObject(HANDLE handle, bool wait)
{
  bool exists = false;
  int index;

  if (WaitHandles)
    {
    for (int i = 0; i < static_cast<int>(WaitCount); i++) //JJH added static_cast
      if (WaitHandles[i] == handle)
        {
        exists = true;
        index = i;
        }
    }

  if (wait)
    {
    if (!exists)
      {
      LPHANDLE newHandles = new HANDLE[WaitCount + 1];
      for (int i = 0; i < static_cast<int>(WaitCount); i++) //JJH added static_cast
        newHandles[i] = WaitHandles[i];
      delete[] WaitHandles;
      WaitHandles = newHandles;
      WaitHandles[WaitCount] = handle;
      WaitCount++;
      }
    }
  else
    {
    if (exists)
      {
      if (WaitCount == 1)
        {
        delete[] WaitHandles;
        WaitHandles = NULL;
        WaitCount = 0;
        }
      else
        {
        LPHANDLE newHandles = new HANDLE[WaitCount - 1];

        int i;
        for (i = 0; i < index; i++)
          newHandles[i] = WaitHandles[i];
        for (i = index + 1; i < static_cast<int>(WaitCount); i++) //JJH added static_cast
          newHandles[i-1] = WaitHandles[i];
        delete[] WaitHandles;
        WaitHandles = newHandles;
        WaitCount--;
        }
      }
    } 
}

/// The SuspendThrow overloads store away a caught exception to allow foreign code
/// to be re-entered. Call ResumeThrow to rethrow the exception, if any, upon
/// return from the foreign code.
/// \name Exception propagation mechanism
/// @{

//
/// Saves off a copy of a TXBase exception to be rethrown later.
/// Calls PostQuitMessage to break the application message loop, thus ensuring that the
/// application will not linger in a corrupt state.
//
void
TApplication::SuspendThrow(const TXBase& x) throw()
{
  XState |= xsBase;
  XBase.reset(x.Clone()); // May fail due to lack of memory!
  ::PostQuitMessage(-1);
}

//
/// Saves off a TXEndSession exception to be rethrown later.
/// Calls PostQuitMessage to break the application message loop, thus ensuring that the
/// application will shut down.
//
void
TApplication::SuspendThrow(const TXEndSession&) throw()
{
  XState |= xsEndSession;
  ::PostQuitMessage(0);
}

//
/// Saves off exception information to be rethrown later.
/// The exception is rethrown as std::runtime_error (x.what()).
/// All exception information, except for the message, is lost.
/// Calls PostQuitMessage to break the application message loop, thus ensuring that the
/// application will not linger in a corrupt state.
//
void
TApplication::SuspendThrow(const exception& x) throw()
{
  XState |= xsStandard;
  XWhat = x.what(); // May fail due to lack of memory!
  ::PostQuitMessage(-1);
}

//
/// Saves off an unknown exception information to be rethrown later.
/// The unknown exception is rethrown as std::runtime_error ("Unknown").
/// All exception information is lost.
/// Calls PostQuitMessage to break the application message loop, thus ensuring that the
/// application will not linger in a corrupt state.
//
void
TApplication::SuspendThrow() throw()
{
  XState |= xsUnknown;
  ::PostQuitMessage(-1);
}


//
/// Rethrows any previously suspended exception, otherwise does nothing.
//
void
TApplication::ResumeThrow()
{
  if (XState != 0)
  {
    TRACEX(OwlApp, 0, _T("TApplication::ResumeThrow: Rethrowing suspended exception."));
    if (XState & xsEndSession)
    {
      XState &= ~xsEndSession;
      throw TXEndSession();
    }
    else if (XState & xsBase) 
    {
      XState &= ~xsBase;
      XBase->Throw();
    }
    else if (XState & xsStandard)
    {
      XState &= ~xsStandard;
      throw std::runtime_error(XWhat);
    }
    else if (XState & xsUnknown)
    {
      XState &= ~xsUnknown;
      throw std::runtime_error("Unknown");
    }
  }
  CHECKX(XState == 0, _T("Unhandled XState exception flag!"));
}

/// @}


//
/// If TApplication's message loop is not used, this function should be called after
/// each message is dispatched
//
void
TApplication::PostDispatchAction()
{
  DeleteCondemned();
  ResumeThrow();

  MSG msg;
  if (!::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
    IdleAction(0);
}


//
//
//
void
TApplication::EnableTooltip(bool enable)
{
  if (!Tooltip) {
    // To circumvent this scenario, we'll look for a window which is fairly
    // stable/rooted as owner of the tooltip. Ideally, we'll get the
    // application's main window.
    //
    TWindow* tipParent = MainWindow;
    if(!MainWindow->GetHandle())
      tipParent = 0;
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
/// Set tooltip . Assume we now own the ToolTip
//
void
TApplication::SetTooltip(TTooltip* tooltip)
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
  // Create only if MainWindows already created
  Tooltip = tooltip;
  if (Tooltip && !Tooltip->GetHandle() && 
      MainWindow && MainWindow->GetHandle()) {
    if(!Tooltip->GetParentO())
      Tooltip->SetParent(MainWindow);
    // Make sure tooltip is disabled so it does not take input focus
    Tooltip->ModifyStyle(0,WS_DISABLED);
    Tooltip->Create();
  }
}


DEFINE_RESPONSE_TABLE_ENTRIES(TApplication)
  EV_WM_ACTIVATEAPP,
  EV_COMMAND(CM_EXIT, CmExit),
END_RESPONSE_TABLE;

//
/// Catch the EV_WM_ACTIVATEAPP message, and activate main window
//
void 
TApplication::EvActivateApp(bool active, HTASK /*hTask*/)
{
  if(active && GetMainWindow())
    GetMainWindow()->SetWindowPos(NULL,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

//
/// Close down main window if application receives a CM_EXIT command.
//
void
TApplication::CmExit()
{
  TFrameWindow* frame = GetMainWindow();
  if (frame) {
    frame->SendMessage(WM_CLOSE);
    DeleteCondemned();
  }
}

//
/// Because TApplication has no event table itself, it defers event handling to the
/// DocManager. If a DocManager has been installed, Find calls TDocManager to handle
/// events.
//
bool
TApplication::Find(TEventInfo &eventInfo, TEqualOperator equal)
{
  // Doc manager gets a chance to handle the event first
  //
  bool retVal = DocManager ? DocManager->Find(eventInfo, equal) : false;
  if (retVal == false) {
    eventInfo.Object = (TGeneric*)this;
    retVal = SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||
             TEventHandler::Find(eventInfo, equal);
  }
  return retVal;
}

//
/// Determine whether the application can be closed.
/// Makes sure the MainWindow can close & doc manager can close.
//
/// Returns true if it is OK for the application to close. By default, CanClose
/// calls the CanClose member function of its main window and returns true if both
/// the main window and the document manager (TDocManager) can be closed. If any of
/// the CanClose functions return false, the application does not close.
/// 
/// This member function is seldom redefined; closing behavior is usually redefined
/// in the main window's CanClose member function, if needed.
//
bool
TApplication::CanClose()
{
  TEventInfo eventInfo(WM_OWLCANCLOSE);
  return (!MainWindow || MainWindow->CanClose())
      && (!DocManager ||!DocManager->Find(eventInfo)
                      || DocManager->Dispatch(eventInfo, 0, 0));
}

//
/// Called by the main window to provide an oportunity to preprocess the main
/// window's menubar before it is installed.
/// Normally delegated to the doc manager to install a file menu as needed
//
void
TApplication::PreProcessMenu(HMENU hMenubar)
{
  TEventInfo eventInfo(WM_OWLPREPROCMENU);
  if (DocManager && DocManager->Find(eventInfo)) {
    DocManager->Dispatch(eventInfo, TParam1(hMenubar), 0);
    MainWindow->DrawMenuBar();
  }
}

//
/// Condemn a window to be deleted the at the next available safe time.
/// Adds the window to a normal single linked list
//
/// Condemned windows should be removed if they are destructed in the mean 
/// time thru some other mechanism (i.e. stack, aggregate, etc)
//
void
TApplication::Condemn(TWindow* win)
{
  TRACEX(OwlApp, 1, _T("Condemning window @") << (void*)win << *win);
  win->SetParent(0);

#if 0

  // The following logic is from previous versions of ObjectWindows.
  // It results in LIFO destructions which is somewhat unfair.
  // However, we'll keep this code around in case previous applications 
  // relied on this destruction order
  //
  win->SetNext(CondemnedWindows);
  CondemnedWindows = win;

#else

  // Insert the new window to be deleted at the end of the list
  //
  win->SetNext(0);
  if (CondemnedWindows) {
    TWindow* eol = CondemnedWindows;
    while (eol->Next())
      eol = eol->Next();
    eol->SetNext(win);
  } 
  else{
    CondemnedWindows = win;
  }

#endif
}

//
/// Remove a condemned window from the list.
//
void
TApplication::Uncondemn(TWindow* win)
{
  if (win && CondemnedWindows) {
    TWindow* w = 0;
    if (CondemnedWindows != win)
      for (w = CondemnedWindows; w->Next() != win; w = w->Next())
        if (!w->Next())
          return;

    TRACEX(OwlApp, 1, _T("Uncondemning window @") << (void*)win << *win);
    if (w)
      w->SetNext(win->Next());
    else
      CondemnedWindows = win->Next();
  }
}

//
/// Walk the condemned window list & delete each window. Assumes that the
/// windows were constructed using 'new'
//
void
TApplication::DeleteCondemned()
{
  while (CondemnedWindows) {
    TRACEX(OwlApp, 1, _T("Deleting condemned window @") << CondemnedWindows << *CondemnedWindows);
    TWindow* next = CondemnedWindows->Next();
    delete CondemnedWindows;
    CondemnedWindows = next;
  }
}



//
/// Constructs a TXInvalidMainWindow object with a default IDS_INVALIDMAINWINDOW
//
TXInvalidMainWindow::TXInvalidMainWindow()
:
  TXOwl(IDS_INVALIDMAINWINDOW)
{
}

TXInvalidMainWindow*
TXInvalidMainWindow::Clone() const
{
  return new TXInvalidMainWindow(*this);
}

//
/// Throws the exception object. Throw must be implemented in any class derived from
/// TXOwl.
//
void
TXInvalidMainWindow::Throw()
{
  throw *this;
}

//
/// Throws a TXInvalidMainWindow exception.
//
void
TXInvalidMainWindow::Raise()
{
  TXInvalidMainWindow().Throw();
}


//
/// Enum[Thread/Task]Windows callback. Counts or disables given window based on
/// whether or not window list has been allocated yet.
//
static bool CALLBACK
disableWnds(HWND hWnd, TEnumInfo * ei)
{
  if (!(::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)) {
    if (hWnd != ei->ModalWnd && ::IsWindowEnabled(hWnd)) {
      if (!ei->Wnds) {
        ei->Count++;  // no buffer yet, we are just counting
      }
      else {
        *(ei->Wnds++) = hWnd;
        ::EnableWindow(hWnd, false);
      }
    }
  }
  return true;
}

//
/// Terminate the modal state initiated by BeginModal. Needs topmost ei block,
/// and cleans the block up as needed inorder to be safe to be called twice.
//
static void termModal(TEnumInfo& ei)
{
  // Re-enable window(s) that are disabled in BeginModal()
  //
  if (ei.Wnds) {
    for (HWND* hWnd = ei.Wnds; *hWnd; hWnd++)
      ::EnableWindow(*hWnd, true);
    delete[] ei.Wnds;
    ei.Wnds = 0;
  }
  else {
    if (ei.ModalWnd && IsWindow(ei.ModalWnd)) {
      ::EnableWindow(ei.ModalWnd, true);
      ei.ModalWnd = 0;
    }
  }
}

//
/// Called to begin a modal window's modal message loop. After determining which
/// window to disable, BeginModal saves the current status of the window, disables
/// the window, calls MessageLoop, and then reenables the window when the message
/// loop is finished. The flags determine how BeginModal treats the window. flags
/// can be one of the following values:
/// 
/// - \c \b  MB_APPLMODAL	The window to be disabled (which is usually an ancestor of the
/// modal window) is identified by window. If window is 0, no window is disabled.
/// - \c \b  MB_SYSTEMMODAL	The window to become system modal is identified by window.
/// - \c \b  MB_TASKMODAL	All top-level windows are disabled, and window is ignored.
///
/// BeginModal returns -1 if an error occurs.
//
int
TApplication::BeginModal(TWindow* window, int flags)
{
  // lock enuminfo if multithreading
  TEnumInfo  ei = { 0, 0, 0, 0};
  TEnumInfo* lastStackTop;
  {
    TEnumInfoStr& data = GetEnumInfo();
    LOCKENUMINFO(data.Lock, false); // Y.B. do we have do lock here ?????????
    lastStackTop = data.GetInfo();  // Keep last stackTop to restore later
    data.SetInfo(&ei);              // Point stackTop to topmost ei
  }

  // The concept of SYSTEM MODAL applies only to the 16-bit environment of
  // Windows. The semantics of TASKMODAL is the closest to SYSMODAL in
  // 32-bit - specially in relation to the meaning of the 'window' parameter.
  // So we'll coerce SYSTEM MODAL to TASK MODAL
  //
  if (flags & MB_SYSTEMMODAL) {
    flags &= ~MB_SYSTEMMODAL;
    flags |=  MB_TASKMODAL;
  }

  // Check modal state
  //
  if (flags & MB_TASKMODAL) {
    LOCKENUMINFO(GetEnumInfo().Lock, false); // lock data
    // Save the window to make modal
    //
    if (window)
      ei.ModalWnd = window->GetHandle();

    // Count windows to disable
    //
    if (!EnumThreadWindows(GetCurrentThreadId(), (WNDENUMPROC)disableWnds,
                           TParam2((TEnumInfo *)&ei)))
      return -1;

    // Allocate list of windows to disable, disable windows that are
    // enabled and then stuff them into the list.
    //
    HWND* hWnds = ei.Wnds = new HWND[ei.Count + 1];
    memset(ei.Wnds, 0, sizeof(TModule::THandle)*(ei.Count + 1));

    EnumThreadWindows(GetCurrentThreadId(), (WNDENUMPROC)disableWnds,
                      TParam2((TEnumInfo *)&ei));

    ei.Wnds = hWnds;  // Restore alloc'd pointer since enumerator bumps it
  }

  else if (window) {

    // In MB_APPMODAL mode, we simply disable the window specified
    //
    ei.ModalWnd = window->GetHandle();  // Remember who to re-enable later
    CHECK(ei.ModalWnd);
    window->EnableWindow(false);
  }

  // Enter message loop, saving & restoring the current status & getting the
  // returned result.
  //
  int result;
  try {
    result = MessageLoop();
  }
  catch (...) {
    TEnumInfoStr& data = GetEnumInfo();
    LOCKENUMINFO(data.Lock, false);
    termModal(ei);
    data.SetInfo(lastStackTop);
    throw;
  }

  {
    TEnumInfoStr& data = GetEnumInfo();
    LOCKENUMINFO(data.Lock, false);
    data.SetInfo(lastStackTop);
    termModal(ei);            // Just in case termModal was missed in EndModal
  }

  // Return the result from the modal window's EndModal call
  //
  return result;
}

//
/// Cause the current modal message loop to break and have it return a result
/// Re-enable the disabled windows here, if the EnumInfo is available.
//
void
TApplication::EndModal(int result)
{
  MessageLoopResult = result;
  BreakMessageLoop = true;
  TEnumInfoStr& data = GetEnumInfo();
  if (data.GetInfo()){
    LOCKENUMINFO(data.Lock, false);
    termModal(*data.GetInfo());
  }
}

//----------------------------------------------------------------------------
#if defined(OWL_SUPPORT_BWCC)

#if !defined(BWCCVERSION)  // solely foe WARNX
#define BWCCVERSION 0x0200   // version 2.00
#endif

//
/// Loads and registers either BWCC.DLL if you are running 16-bit applications or
/// BWCC32.DLL if you are running 32-bit applications. 
/// To disable BWCC, set enable to false.
/// Library is not free'd on disable to reduce re-loads if enabling on the fly
/// \note BWCC is now obsolete and should be used only for backward compatibility
void
TApplication::EnableBWCC(bool enable, uint language)
{
  if (enable) {
    if (!BWCCModule) {
      try {
        BWCCModule = new TBwccDll();
        BWCCModule->IntlInit(language);
        BWCCModule->Register(GetHandle());

        WARNX(OwlApp, BWCCModule->GetVersion() < BWCCVERSION, 0, \
              _T("Old version of BWCC DLL found"));
      }
      catch (...) {
        TRACEX(OwlApp, 0, _T("Unable to create instance of TBwccDll"));
        return;
      }
    }
  }
  BWCCOn = enable;
}

#endif //#if defined(OWL_SUPPORT_BWCC)
//----------------------------------------------------------------------------
#if defined(OWL_SUPPORT_CTL3D)

//
/// Enables or disables the use of the CTL3D DLL. If enable is true, EnableCtl3d
/// loads and registers the CTL3D.DLL if it is not already enabled.
/// \note Usage of Ctl3d is now obsolete and should be used only for backward compatibility
//
void
TApplication::EnableCtl3d(bool enable)
{
  // As per Article Q125684 of Microsoft Development Library:
  // "If major version is 4 or greater, the application should not
  //  implement CTL3D".
  //      'How to Use CTL3D Under the Windows 95 Operating System'
  //
  if (TSystem::GetMajorVersion() >= 4)
    return;

  // Load the Ctl3d DLL if needed & register our instance
  //
  if (enable) {
    if (!Ctl3dModule) {
      try {
        Ctl3dModule = new TCtl3dDll();
        Ctl3dModule->Register(*this);
      }
      catch (...) {
        TRACEX(OwlApp, 0, _T("Unable to create instance of TCtl3dDll"));
        return;
      }
    }
  }
  Ctl3dOn = enable;
}

//
/// Enable or disable Ctl3d's use of auto-subclassing.
//
/// Caller should turn on autosubclassing before creating a non-owl dialog to
/// make it 3d, & turn it off immediatly after.
///
/// Enables or disables CTL3D's use of autosubclassing if CTL3D is already enabled
/// using Ctl3dEnabled. If autosubclassing is enabled, any non-ObjectWindows dialog
/// boxes have a 3-D effect. The common dialog classes and TDocManager use this
/// function both to turn on autosubclassing before creating a non-ObjectWindows
/// dialog box to make it three-dimensional and to turn off autosubclassing
/// immediately after the dialog box is destroyed.
//
void
TApplication::EnableCtl3dAutosubclass(bool enable)
{
  if (Ctl3dEnabled()) {
    if (enable) {
      Ctl3dModule->Register(*this);
      Ctl3dModule->AutoSubclass(*this);
    }
    else {
      Ctl3dModule->Unregister(*this);
    }
  }
}

#endif //#if defined(OWL_SUPPORT_CTL3D)
//----------------------------------------------------------------------------
#if defined(OWL_SUPPORT_BWCC)

//
/// Predefined DLLs that TApplication knows how to find.
//
  const tchar BwccDllName[]  = _T("BWCC32.DLL");

//
/// Load the BWCC DLL dynamically.
/// Bind the member functions to the exported functions.
//
/// This is how TApplication::EnableBWCC uses TBwccDll:
///  \code
/// if (!BWCCModule) { 
///   try { 
///     BWCCModule = new TBwccDll();
///     BWCCModule->IntlInit(language);
///     BWCCModule->Register(GetHandle());
/// 
///     WARNX(OwlApp, BWCCModule->GetVersion() < BWCCVERSION, 0, \\
///           _T("Old version of BWCC DLL found"));
///   }
///   catch (...) { 
///     TRACEX(OwlApp, 0, _T("Unable to create instance of TBwccDll"));
///     return;
///   }
/// \endcode
//
TBwccDll::TBwccDll()
:
  TModule(BwccDllName, true, true, false), // shouldLoad, mustLoad and !addToList

  IntlInit(*this, "BWCCIntlInit"),
  Register(*this, "BWCCRegister"),
  IntlTerm(*this, "BWCCIntlTerm"),

  SpecialLoadDialog(*this, "SpecialLoadDialog"),
  MangleDialog(*this, "MangleDialog"),
  DefDlgProc(*this, "BWCCDefDlgProc"),
  DefGrayDlgProc(*this, "BWCCDefGrayDlgProc"),
  DefWindowProc(*this, "BWCCDefWindowProc"),
  DefMDIChildProc(*this, "BWCCDefMDIChildProc"),
  MessageBox(*this, "BWCCMessageBox"),
  GetPattern(*this, "BWCCGetPattern"),
  GetVersion(*this, "BWCCGetVersion")
{
}

#endif //#if defined(OWL_SUPPORT_BWCC)
//----------------------------------------------------------------------------
#if defined(OWL_SUPPORT_CTL3D)

//
/// Predefined DLLs that TApplication knows how to find.
//
  const tchar Ctl3dDllName[] = _T("CTL3D32.DLL");

//
/// Load the Ctl3d DLL dynamically.
/// Bind the member functions to the exported functions.
//
TCtl3dDll::TCtl3dDll()
:
  TModule(Ctl3dDllName, true, true, false), // shouldLoad, mustLoad and !addToList

  Register(*this, "Ctl3dRegister"),
  Unregister(*this, "Ctl3dUnregister"),
  AutoSubclass(*this, "Ctl3dAutoSubclass"),
  CtlColorEx(*this, "Ctl3dCtlColorEx"),
  SubclassDlg(*this, "Ctl3dSubclassDlg"),
  SubclassDlgEx(*this, "Ctl3dSubclassDlgEx"),
  GetVer(*this, "Ctl3dGetVer"),
  Enabled(*this, "Ctl3dEnabled"),
  ColorChange(*this, "Ctl3dColorChange"),
  SubclassCtl(*this, "Ctl3dSubclassCtl"),
  DlgFramePaint(*this, "Ctl3dDlgFramePaint"),
  WinIniChange(*this, "Ctl3dWinIniChange")
{
}

#endif //#if defined(OWL_SUPPORT_CTL3D)
//----------------------------------------------------------------------------

#if !defined(BI_NO_OBJ_STREAMING)

// Broken apart: IMPLEMENT_STREAMABLE1(TApplication, TModule);
// to replace the ctor
//
IMPLEMENT_STREAMABLE_CLASS(TApplication);
IMPLEMENT_STREAMER(TApplication);
IMPLEMENT_STREAMABLE_POINTER(TApplication)

// IMPLEMENT_STREAMABLE_CTOR1(TApplication, TModule), plus TMsgThread init
//
TApplication::TApplication(StreamableInit)
:
  TModule(streamableInit),
  TMsgThread(TMsgThread::Current)
{
}

//
//
//
void*
TApplication::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TApplication* o = GetObject();
  if (o != owl::Module)
    is >> *owl::Module;   // set reference to OWL module
  return o;
}

//
//
//
void
TApplication::Streamer::Write(opstream& os) const
{
  TApplication* o = GetObject();
  if (o != owl::Module)
    os << *owl::Module;    // write out reference to OWL module, no data written
}

#else

IMPLEMENT_STREAMABLE1(TApplication, TModule);

#endif  // if !defined(BI_NO_OBJ_STREAMING)

//----------------------------------------------------------------------------

//
/// Called automatically from IdleAction if compiled with __TRACE or __WARN flag. If
/// you use the retail version of OWL and would like to enable Diagnostic Windows
/// you should override IdleAction and call this function.
//
bool
TApplication::DiagIdleAction(long idleCount)
{
  LOCKAPPLDATA(GetApplicatData().Lock,true);
  bool more = false;
  if (TTraceWindow::TraceWindow() &&
    TTraceWindow::TraceWindow()->IdleAction (idleCount)){
      more = true;
  }
  return more;
}

//
/// Creates or destroyes the Diagnostic Windows. This function called from
/// ProcessAppMsg() when OWL is compiled with __TRACE or __WARN options. If you
/// would like to use Diagnostic windows with the release version of OWL you should
/// add a menu item in System Menu with Id: SCM_TRACE =0xFEC0, and if the user
/// presses it call SystemCommandTrace() to create the Diagnostic window.
//
void
TApplication::SystemCommandTrace()
{
  LOCKAPPLDATA(GetApplicatData().Lock,false);
  if (TTraceWindow::TraceWindow())
    delete TTraceWindow::TraceWindow();
  else{
    new TTraceWindow;
    TTraceWindow::TraceWindow()->Create ();
    TTraceWindow::TraceWindow()->ShowWindow (SW_SHOWNORMAL);
  }
}


// TWaitCursor data

inline bool FilterWindow (HWND hWnd){
  return !::IsWindowEnabled (hWnd) || !::IsWindowVisible (hWnd);
}
//
/// Enum[Thread/Task]Windows callback. Counts or subclasses given window based on
/// whether or not window list has been allocated yet.
//
static bool CALLBACK
subclassWnds(HWND hWnd, TEnumInfo * ei)
{
  if (!FilterWindow (hWnd)) {
    if (!ei->Wnds) {
      ei->Count++;  // no buffer yet, we are just counting
    }
    else {
      *(ei->Wnds++) = hWnd;
      *(ei->PrevWndProc++) = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)TWaitHook::WaitWndProc);
    }
  }
  return true;
}
//
enum{
    WM_NCMOUSEFIRST = WM_NCMOUSEMOVE,
    WM_NCMOUSELAST  = WM_NCMBUTTONDBLCLK
};
//
TWaitHook::TWaitHook()
:
  LastStackTop(0),
  HideCaretCount(0),
  Cursor(0),
  DeleteOnClose(false)
{
  memset(&Subclass,0,sizeof(Subclass));

  TEnumInfoStr& data = GetEnumInfo();
  LastStackTop = data.GetInfo(); // Keep last stackTop to restore later
  data.SetInfo(&Subclass);       // Point stackTop to topmost ei

  // Count windows to subclass
  //
  if (!::EnumThreadWindows(GetCurrentThreadId(), (WNDENUMPROC)subclassWnds,
                         TParam2((TEnumInfo *)&Subclass)))
      return;

   // Allocate list of windows to disable, disable windows that are
   // enabled and then stuff them into the list.
   //
   HWND* hWnds = Subclass.Wnds = new HWND[Subclass.Count + 1];
   WNDPROC* fProc = Subclass.PrevWndProc = new WNDPROC[Subclass.Count + 1];
   memset(Subclass.Wnds, 0, sizeof(HWND)*(Subclass.Count + 1));
   memset(Subclass.PrevWndProc, 0, sizeof(WNDPROC)*(Subclass.Count + 1));

   EnumThreadWindows(GetCurrentThreadId(), (WNDENUMPROC)subclassWnds,
                     TParam2((TEnumInfo *)&Subclass));

  Subclass.Wnds          = hWnds;  // Restore alloc'd pointer since enumerator bumps it
  Subclass.PrevWndProc  = fProc;  // Restore alloc'd pointer 

  HideCaret(0);
  HideCaretCount = 1;
  Cursor = new TCursor(::LoadCursor (0, IDC_WAIT));
  DeleteOnClose = true;
  SetWaitCursor();
}
//
TWaitHook::~TWaitHook()
{
  for (int i = 0; i < Subclass.Count; i++)
    ::SetWindowLongPtr(Subclass.Wnds[i], GWLP_WNDPROC, (LONG_PTR)Subclass.PrevWndProc[i]);

  delete [] Subclass.Wnds;
  delete [] Subclass.PrevWndProc;

  while(HideCaretCount--)
    ShowCaret(0);

  TPoint  pt;
  ::GetCursorPos(&pt);
  ::SetCursorPos(pt.x, pt.y);

  GetEnumInfo().SetInfo(LastStackTop); // Restore stackttop
  
  FreeCursors();
}
//
void TWaitHook::FreeCursors()
{
  if(DeleteOnClose)
    delete Cursor;
  Cursor    = 0;
}
//
void TWaitHook::SetCursor(TCursor* cursor, TAutoDelete del)
{
  FreeCursors();
  Cursor        = cursor;
  DeleteOnClose  = del == AutoDelete;
}
//
void TWaitHook::SetWaitCursor()
{
  ::SetCursor(*Cursor);
}
//
LRESULT CALLBACK
#if defined(BI_COMP_BORLANDC) && __BORLANDC__ < 0x0630
__declspec(dllexport)
#endif
TWaitHook::WaitWndProc(HWND wnd, UINT msg, WPARAM param1, LPARAM param2)
{
  return GetEnumInfo().Hook->WaitWndMethod(wnd, msg, param1, param2);
}
//
TResult TWaitHook::WaitWndMethod(HWND wnd, TMsgId msg, TParam1 param1, TParam2 param2)
{
  HWND    hTopMostWindow;
  switch (msg){
    case WM_SETCURSOR:
      SetWaitCursor();
      return TRUE;

    case WM_MOUSEACTIVATE:
      hTopMostWindow = (HWND)param1;

      if (hTopMostWindow == wnd)
        return MA_ACTIVATEANDEAT;

       return ::DefWindowProc(wnd, msg, param1, param2);

    case WM_SETFOCUS:
       DefWndProc (wnd, msg, param1, param2);
       HideCaret(0);
       ++HideCaretCount;
       return 0;
  }

  if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) ||
     (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) ||
     (msg >= WM_NCMOUSEFIRST && msg <= WM_NCMOUSELAST))
  {
    return 0; // ::DefWindowProc (wnd, msg, param1, param2);
  }
  return DefWndProc (wnd, msg, param1, param2);
}
//
TResult TWaitHook::DefWndProc(HWND wnd, TMsgId msg, TParam1 param1, TParam2 param2)
{
  for(int i = 0; i < (int)Subclass.Count; i++){
    if (Subclass.Wnds[i] == wnd)
      return CallWindowProc(Subclass.PrevWndProc[i], wnd, msg, param1, param2);
  }
  PRECONDITION(0);
  return 0;
}

//----------------------------------------------------------------------------
//
/// Changes the cursor to cursor.  If TAutoDelete is set to AutoDelete the cursor
/// resource is deleted when the wait cursor is destoryed.
//
void TWaitCursor::SetCursor(TCursor* cursor, TAutoDelete del)
{
  TEnumInfoStr& data = GetEnumInfo();
  LOCKENUMINFO(data.Lock, false); 
  if(data.Hook)
    data.Hook->SetCursor(cursor,del);
}

//----------------------------------------------------------------------------

//
/// Called by the constructors to add this cursor into the applications chain.
//
void TWaitCursor::Init()
{
  TEnumInfoStr& data = GetEnumInfo();
  LOCKENUMINFO(data.Lock, false); 
  Next      = data.Top;
  data.Top  = this;
  if(!Next){
    PRECONDITION(data.Hook == 0);
    data.Hook = new TWaitHook;
  }
}
//
/// Restores the previous cursor.
//
TWaitCursor::~TWaitCursor()
{
  TEnumInfoStr& data = GetEnumInfo();
  LOCKENUMINFO(data.Lock, false); 
  
  PRECONDITION(data.Top == this);  // must be destructed in reverse order...

  if(data.Top){
    data.Top  = Next;
    if(!Next){
      delete data.Hook;
      data.Hook = 0;
    }
  }
}
//
/// Restores old state of cursor.
//
void TWaitCursor::Restore()
{
  TEnumInfoStr& data = GetEnumInfo();
  LOCKENUMINFO(data.Lock, false); 

  delete data.Hook;
  data.Hook = 0;
  data.Top  = 0;
}
//----------------------------------------------------------------------------
//
/// Sends text to the applications TMessageBar::SetHintText function if it has one.
//
void TWaitCursor::Message(const tstring& msg)
{
  TEnumInfoStr& data = GetEnumInfo();
  LOCKENUMINFO(data.Lock, true); 
  if(data.Top == this && data.Hook){
    TApplication* app = OWLGetAppDictionary().GetApplication(0);
    if(!app)
      return;
    
    TFrameWindow* frame= app->GetMainWindow();
    if (frame){
      TMessageBar* messageBar = 
        TYPESAFE_DOWNCAST(frame->ChildWithId(IDW_STATUSBAR), TMessageBar);
      if(messageBar){
        messageBar->SetHintText(msg);       
        messageBar->UpdateWindow();
      }
    }
  }
}
//-----------------------------------------------------------------------------
//
TLangModule::TLangModule(const tstring& prefix, const TApplication& appl)
:
  Prefix(prefix),
  Module(0),
  Application(&(TApplication&)appl),
  LangId(static_cast<unsigned short>(-1)) //JJH added static_cast
{
  SetLanguage(appl.GetLangId());
}
//-----------------------------------------------------------------------------
TLangModule::~TLangModule()
{
  if(Module && Module != Application)
    delete Module;
}
//-----------------------------------------------------------------------------
void TLangModule::SetLanguage(const TLangId& langId)
{
  tstring mname = Prefix;
  
  tchar szLocName[10];
  int retval = ::GetLocaleInfo(langId,LOCALE_SABBREVLANGNAME, szLocName, 10);
  if(retval)
    mname += szLocName;
  else
    mname += _T("ENU");
  mname += _T(".DLL");
  TModule* module = new TModule(mname.c_str(),true, false, true); // shouldLoad, !mustLoad and addToList
  if(module->GetHandle() <= HINSTANCE(HINSTANCE_ERROR)){
    delete module;
    tstring locname = Prefix;
    locname += _T("ENU.DLL");
    module = new TModule(locname.c_str(),true, false, true); // shouldLoad, !mustLoad and addToList
    if(module->GetHandle() > HINSTANCE(HINSTANCE_ERROR))
      mname = locname;
    else{
      delete module;
      module = Application;
    }
  }
  if(Module && Module != Application)
    delete Module;
  Module = module;

  // init LangId
  InitLanguage pProc = (InitLanguage)Module->GetProcAddress("InitLanguage");
  if(pProc)
    pProc();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

} // OWL namespace
//===============================================================================
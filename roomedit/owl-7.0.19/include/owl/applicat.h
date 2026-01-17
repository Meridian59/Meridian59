//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TApplication. This defines the basic behavior for OWL
/// applications. Also definition for TXInvalidMainWindow
//----------------------------------------------------------------------------

#if !defined(OWL_APPLICAT_H)
#define OWL_APPLICAT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/module.h>
#include <owl/eventhan.h>
#include <owl/msgthred.h>
#include <owl/gdibase.h>
#include <owl/lclstrng.h>

#include <stdexcept>
#include <memory>

namespace owl {

#if defined(OWL_SUPPORT_BWCC)
//
// Language defines for TApplication::EnableBWCC()
//
#define BWCC_LANG_GERMAN 0x07
#define BWCC_LANG_US     0x09
#define BWCC_LANG_FRENCH 0x0c
#endif

class _OWLCLASS TWindow;
class _OWLCLASS TFrameWindow;
class _OWLCLASS TDocManager;
class _OWLCLASS TAppDictionary;
class _OWLCLASS TXInvalidMainWindow;

#if defined(OWL_SUPPORT_BWCC)
class _OWLCLASS TBwccDll;
#endif

#if defined(OWL_SUPPORT_CTL3D)
class _OWLCLASS TCtl3dDll;
#endif

class _OWLCLASS TCursor;
class _OWLCLASS TTooltip;
class TWaitHook;

typedef int (*TMessageBox)(HWND wnd, LPCTSTR text, const LPCTSTR caption, uint type);

_OWLFUNC(TMessageBox) SetDefMessageBox(TMessageBox);

_OWLFUNC(int) OWLMessageBox(HWND wnd, const tstring& text, const tstring& caption, uint type);
_OWLFUNC(int) OWLMessageBox(HWND wnd, const LPCTSTR text, const LPCTSTR caption, uint type);
_OWLFUNC(int) OWLMessageBox(TWindow* wnd, TResId resId, const tstring& caption, uint type, TModule* module = nullptr);
_OWLFUNC(int) OWLMessageBox(TWindow* wnd, TResId resId, const LPCTSTR caption, uint type, TModule* module = nullptr);

#include <owl/preclass.h>
//
/// \class TWaitCursor
// ~~~~~ ~~~~~~~~~~~
//
/// TWaitCursor is a simple class used to display a wait cursor. Wait cursors are
/// typically created on the stack so that the prevous cursor is displayed when the
/// wait cursor goes out of scope.
///
/// Wait cursors can be nested, but there destruction sequence must be opposite to
/// their construction sequence.
//
/// Typical Use
/// \code
/// ...
///  {
///   TWaitCursor wc;  // Wait cursor displayed.
///   ...
///   ...
///  }
///  // Wait cursor out of scope so previous cursor restored.
/// \endcode
//
class _OWLCLASS TWaitCursor {
  public:
    TWaitCursor();
    TWaitCursor(const tstring& msg);
    TWaitCursor(TCursor* cursor, TAutoDelete = AutoDelete);
    ~TWaitCursor();

    void Restore();
    void SetCursor(TCursor* cursor, TAutoDelete = AutoDelete);
    void Message(const tstring& text);

  private:
    void Init();

    TWaitCursor*  Next;  ///< Pointer to previous cursor in the applications chain.

    friend class TWaitHook;
};
//
/// \struct TCurrentEvent
// ~~~~~~ ~~~~~~~~~~~~~
/// Current event structure for windows events
//
struct TCurrentEvent
{
  TWindow* Win; ///< Window that message was sent/dispatched to
  TMsgId Message; ///< Message ID
  TParam1 Param1; ///< First parameter (WPARAM)
  TParam2 Param2; ///< Second parameter (LPARAM)

  TCurrentEvent() : Win(nullptr), Message(0), Param1(0), Param2(0) {}
};

/// \addtogroup module
/// @{
/// \class TApplication
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TModule and TMsgThread and virtually derived from TEventHandler,
/// TApplication acts as an object-oriented stand-in for an application module.
/// TApplication and TModule supply the basic behavior required of an application.
/// TApplication member functions create instances of a class, create main windows,
/// and process messages.
///
/// To create an OLE-enabled Doc/View application, you need to derive your
/// application from both TApplication and TOcAppHost.
///
/// \note If a document manager is installed (see SetDocManager), TApplication will give the
/// document manager the first stab at event handling. See the TApplication::Find implementation.
//
class _OWLCLASS TApplication : virtual public TEventHandler,
                                       public TModule,
                                       public TMsgThread
{
  public:
    // Constructors for TApplication. Default args for the ctor allow
    // TApplication to access global pointers in the user exe/dll.
    // Default OwlAppDictionary can be overridden by passing non-0 appDict arg
    //
    TApplication
      (
        LPCTSTR name = nullptr,
      TModule*& = owl::Module,
        TAppDictionary* = nullptr
      );

    TApplication
      (
      const tstring& name,
      TModule*& = owl::Module,
        TAppDictionary* = nullptr
      );

    TApplication
      (
      LPCTSTR name,
      HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      const tstring& cmdLine,
      int cmdShow,
      TModule*& = owl::Module,
        TAppDictionary* = nullptr
      );

    TApplication
      (
      const tstring& name,
      HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      const tstring& cmdLine,
      int cmdShow,
      TModule*& = owl::Module,
        TAppDictionary* = nullptr
      );

   ~TApplication() override;

    TFrameWindow*    GetMainWindow();
    TDocManager*     GetDocManager();
    TLangId          GetLangId() const;
    void             SetLangId(TLangId landid);

    void             ClearMainWindow(); // Called by the main window destructor

    static void SetWinMainParams
      (
      HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      const tstring& cmdLine,
      int cmdShow
      );

    void             GetWinMainParams();

    HINSTANCE        GetPrevInstance() const;
    void             SetPrevInstance(HINSTANCE pi);

    int              GetCmdShow() const;
    void             SetCmdShow(int cmdshow);

    static tstring& GetCmdLine();
    TCurrentEvent&   GetCurrentEvent();

    virtual bool     CanClose();
    auto Run() -> int override;
    virtual int Start() noexcept;

    /// \name Message queue thread synchronization mechanism
    /// @{
#if defined(BI_MULTI_THREAD_RTL)
    typedef TMsgThread::TQueueLock TAppLock;

    /// Override TEventHandler::Dispatch() to handle multi-thread
    /// synchronization
    //
    virtual TResult  Dispatch(TEventInfo& info, TParam1, TParam2 = 0);
#endif
    /// @}

    /// \name Message queue loop & response functions
    /// @{
    auto MessageLoop() -> int override;
    auto IdleAction(long idleCount) -> bool override;
    auto ProcessMsg(MSG&) -> bool override;
    virtual bool     ProcessAppMsg(MSG& msg);
    /// @}

    virtual void     WaitOnObject(HANDLE handle, bool wait);
    virtual void     ObjectSignaled(HANDLE /*handle*/, bool /*abandoned*/) {}

    /// \name Exception propagation mechanism
    /// @{
    void SuspendThrow(std::exception_ptr);
    void ResumeThrow();
    bool HasSuspendedException() const {return !(CurrentException == std::exception_ptr());}
    /// @}

    /// Get the TWindow pointer belonging to this app given an hWnd
    //
    TWindow*         GetWindowPtr(HWND hWnd) const;

    /// \name Begin and end of a modal window's modal message loop
    /// @{
    int              BeginModal(TWindow* window, int flags=MB_APPLMODAL);
    void             EndModal(int result);
    virtual void     PreProcessMenu(HMENU hMenubar);
    /// @}

    /// \name Dead TWindow garbage collection
    /// @{
    void             Condemn(TWindow* win);
    void             Uncondemn(TWindow* win);
    /// @}

    /// Call this function after each msg dispatch if TApplication's message
    /// loop is not used.
    //
    void             PostDispatchAction();

    /// \name Control of UI enhancing libraries
#if defined(OWL_SUPPORT_BWCC)
    /// @{
    void             EnableBWCC(bool enable = true, uint language = 0);
    bool             BWCCEnabled() const;
    TBwccDll*        GetBWCCModule() const;
#endif

#if defined(OWL_SUPPORT_CTL3D)
    void             EnableCtl3d(bool enable = true);
    void             EnableCtl3dAutosubclass(bool enable) const;
    bool             Ctl3dEnabled() const;
    TCtl3dDll*       GetCtl3dModule() const;
#endif
    /// @}

    // Opens a modal message box
    //
    int MessageBox
      (
      HWND wnd,
      const tstring& text,
      const tstring& caption = tstring(),
      uint type = MB_OK
      ) const;

    virtual int MessageBox
      (
      HWND wnd,
      LPCTSTR text,
        LPCTSTR caption = nullptr,
      uint type = MB_OK
      ) const;

    // Retrieves/enables tooltip
    //
    virtual TTooltip* GetTooltip() const;
    virtual void      EnableTooltip(bool enable=true);

    /// \name TEventHandler overrides
    /// @{

    auto Find(TEventInfo&, TEqualOperator = nullptr) -> bool override;

    /// @}

  protected:
    virtual void     InitApplication();
    void InitInstance() override;
    virtual void     InitMainWindow();
    auto TermInstance(int status) -> int override;

    /// Assigns tooltip
    void SetTooltip(TTooltip* tooltip);

    // (Re)set a new main-window and DocManager either at construction or
    // sometime later
    //
    TFrameWindow*    SetMainWindow(TFrameWindow* window);
    auto SetMainWindow(std::unique_ptr<TFrameWindow>) -> std::unique_ptr<TFrameWindow>;
    TDocManager*     SetDocManager(TDocManager* docManager);
    auto SetDocManager(std::unique_ptr<TDocManager>) -> std::unique_ptr<TDocManager>;

    // Member data -- use accessors to get at these
    //
  public_data:
    HINSTANCE     hPrevInstance;
    int           nCmdShow;
    TDocManager*  DocManager;
    TFrameWindow* MainWindow;
    TLangId       LangId;
    TTooltip*     Tooltip;

  protected_data:
    tstring    CmdLine;      ///< string object copy of cmd line

    DWORD    WaitCount;
    LPHANDLE WaitHandles;

  private:
#if defined(OWL_SUPPORT_BWCC)
    bool          BWCCOn;
    TBwccDll*     BWCCModule;
#endif

#if defined(OWL_SUPPORT_CTL3D)
    bool          Ctl3dOn;
    TCtl3dDll*    Ctl3dModule;
#endif

    TCurrentEvent CurrentEvent;
    std::exception_ptr CurrentException;

    /// \name Condemned TWindow garbage collection
    /// @{
    void          DeleteCondemned();
    TWindow*      CondemnedWindows;  ///< List of comdemned windows.
    /// @}

    /// The dictionary that this app is in
    //
    TAppDictionary*     Dictionary;

    // Static application initialization parameters cached here before app
    // is actually constructed
    //
    static HINSTANCE    InitHInstance;          ///< WinMain's 1st param
    static HINSTANCE    InitHPrevInstance;      ///< WinMain's 2nd param
    static tstring&  GetInitCmdLine();       ///< WinMain's 3rd param
    static int          InitCmdShow;            ///< WinMain's 4th param

    /// \name Event handlers for response table entries
    /// @{

    void EvSysCommand(uint cmd, const TPoint&);
    void CmExit();            ///< Exit from file menu

    /// @}

    // Hidden to prevent accidental copying or assignment
    //
    TApplication(const TApplication&);
    TApplication& operator =(const TApplication&);

  DECLARE_STREAMABLE_OWL(TApplication, 1);
};

/// @}

DECLARE_STREAMABLE_INLINES( TApplication );

#if defined(OWL_SUPPORT_BWCC)
/// \addtogroup module
/// @{
/// \class TBwccDll
// ~~~~~ ~~~~~~~~
/// Wrapper for the BWCC Dll
//
/// The TBwccDll class encapsulates the Borland Windows Custom Control (BWCC) DLL
/// (BWCC[32].DLL). It provides an easy method to dynamically test for the
/// availability of the DLL and bind to its exported functions at runtime. By using
/// the TBwccDll class instead of direct calls to the BWCC DLL, ObjectWindows
/// applications can provide the appropriate behavior when running in an environment
/// where the DLL is not available.
///
/// Each data member of the TBwccDll class corresponds to the API with a similar
/// name exposed by the BWCC DLL. For example, TBwccDll::MessageBox corresponds to
/// the BWCCMessageBox API exported by the BWCC DLL.
//
/// \note In modern applications BWCC should not be used. It is retained only for
/// backwards compatibility and porting of legacy applications.
//
class _OWLCLASS TBwccDll : public TModule {
  public:
    TBwccDll();

    // Used by TApplication
    //
    TModuleProc1<bool,uint>      IntlInit;
    TModuleProc1<bool,HINSTANCE> Register;
    TModuleProc0<bool>           IntlTerm;

    // Not used by OWL
    TModuleProc3<HGLOBAL,HINSTANCE,LPCTSTR,DLGPROC> SpecialLoadDialog;
    TModuleProc3<HGLOBAL,HGLOBAL,HINSTANCE,DLGPROC> MangleDialog;
    TModuleProc4<LRESULT,HWND,UINT,WPARAM,LPARAM>   DefDlgProc;
    TModuleProc4<LRESULT,HWND,UINT,WPARAM,LPARAM>   DefGrayDlgProc;
    TModuleProc4<LRESULT,HWND,UINT,WPARAM,LPARAM>   DefWindowProc;
    TModuleProc4<LRESULT,HWND,UINT,WPARAM,LPARAM>   DefMDIChildProc;
    TModuleProc4<int,HWND,LPCTSTR,LPCTSTR,UINT>     MessageBox;
    TModuleProc0<HBRUSH> GetPattern;
    TModuleProc0<DWORD>  GetVersion;
};

/// @}
#endif

#if defined(OWL_SUPPORT_CTL3D)
/// \addtogroup module
/// @{
/// \class TCtl3dDll
// ~~~~~ ~~~~~~~~~
/// Wrapper for the Control 3D Dll
//
/// The TCtl3dDll class encapsulates the Control 3D DLL (CTL3D[V2|32].DLL). It
/// provides an easy method to dynamically test for the availability of the DLL and
/// bind to its exported functions at runtime. By using the TCtl3dDll class instead
/// of direct calls to the Control 3D DLL, ObjectWindows applications can provide
/// the appropriate behavior when running in an environment where the DLL is not
/// available.
///
/// Each data member of the TCtl3dDll class corresponds to the API with a similar
/// name exposed by the Control 3D DLL. For example, TCtl3dDll::AutoSubclass
/// corresponds to the Ctl3dAutoSubclass API exported by the Control 3D DLL.
//
class _OWLCLASS TCtl3dDll : public TModule {
  public:
    TCtl3dDll();

    // Used by TApplication
    TModuleProc1<BOOL,HANDLE> Register;
    TModuleProc1<BOOL,HANDLE> Unregister;
    TModuleProc1<BOOL,HANDLE> AutoSubclass;

    // Used by TDialog
    TModuleProc3<HBRUSH,uint,WPARAM,LPARAM> CtlColorEx;
    TModuleProc2<BOOL,HWND,uint16>          SubclassDlg;

    // Not used by OWL
    TModuleProc2<BOOL,HWND,DWORD> SubclassDlgEx;
    TModuleProc0<WORD> GetVer;
    TModuleProc0<BOOL> Enabled;
    TModuleProc0<BOOL> ColorChange;
    TModuleProc1<BOOL,HWND> SubclassCtl;
    TModuleProc4<LONG,HWND,UINT,WPARAM,LPARAM> DlgFramePaint;

    TModuleProc0<int> WinIniChange;
};
/// @}
#endif

/// \addtogroup except
/// @{
// class TXInvalidMainWindow
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
//
/// A nested class, TXInvalidMainWindow describes an exception that results from an
/// invalid Window. This exception is thrown if there is not enough memory to create
/// a window or a dialog object. TApplication::InitInstance throws this exception if
/// it can't initialize an instance of an application object.
//
class _OWLCLASS TXInvalidMainWindow : public TXOwl {
  public:
    TXInvalidMainWindow();

    TXInvalidMainWindow* Clone();
    void Throw();

    static void Raise();
};
/// @}


/// \addtogroup module
/// @{
/// \class TLangModule
// ~~~~~ ~~~~~~~~~~~
/// International resource support:  [APP]ENG.DLL - default application module.
//
/// Module == TApplication if not found any resource module.
//
/// Usage:
/// \code
/// static TLangModule* langModulePtr = 0;
/// TLangModule* GetResModule()
/// {
///    return langModulePtr;
/// }
///
/// int OwlMain(int,LPCTSTR*)
/// {
///   TMyAppl appl;
///   TLangModule langModule("MRES_",appl); // will be MRES_###.dll
///   langModulePtr = &langModule;
///   return appl.Run();
/// }
/// \endcode
class _OWLCLASS TLangModule {
  public:
    // export function: void __stdcall InitLanguage();
    typedef void (*InitLanguage)();
    // Constructors & destructor
    //
    TLangModule(const tstring& prefix, const TApplication& appl);
    virtual ~TLangModule();

    operator TModule*()         { return Module; }
    TModule*  GetModule()       { return Module; }

    void SetLanguage(const TLangId& langId);
    TLangId GetLanguage() const { return LangId;}

  protected:
    tstring    Prefix;
    TModule*      Module;
    TApplication* Application;
    TLangId       LangId;
};
/// @}


#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//
/// Activates the wait cursor.
//
inline TWaitCursor::TWaitCursor()
{
  Init();
}
//
/// Activates the wait cursor and sends msg to the applications
/// TMessageBar::SetHintText function if it has one.
//
inline TWaitCursor::TWaitCursor(const tstring& msg)
{
  Init();
  Message(msg);
}
//
/// Activates cursor. If TAutoDelete is set to AutoDelete the cursor resource is
/// deleted when the wait cursor is destoryed.
//
inline TWaitCursor::TWaitCursor(TCursor* cursor, TAutoDelete del)
{
  Init();
  SetCursor(cursor, del);
}

////////////////////
//
/// Return the current main window.
//
inline TFrameWindow* TApplication::GetMainWindow()
{
  return MainWindow;
}

//
/// Return the current document manager.
//
inline TDocManager* TApplication::GetDocManager()
{
  return DocManager;
}

//
/// Return the HINSTANCE of the previous running instance.
//
inline HINSTANCE TApplication::GetPrevInstance() const
{
  return hPrevInstance;
}

//
/// Set the previous instance.
/// This should not be called by normal programs.
//
inline void TApplication::SetPrevInstance(HINSTANCE pi)
{
  hPrevInstance = pi;
}

//
/// Retrieve the initial state of the main window.
//
inline int TApplication::GetCmdShow() const
{
  return nCmdShow;
}

//
/// Sets the initial state of the main window.
/// Typically passed by the operating system.
//
inline void TApplication::SetCmdShow(int cmdshow)
{
  nCmdShow = cmdshow;
}

//
/// Return the command line of the application.
/// Most programs do not need to call this because OwlMain has the
/// parameters already parsed.
//
inline tstring& TApplication::GetCmdLine()
{
  return GetInitCmdLine();
}

//
/// Return the current event from the message queue.
//
inline TCurrentEvent& TApplication::GetCurrentEvent()
{
  return CurrentEvent;
}

extern _OWLFUNC(TWindow*) GetWindowPtr(HWND, const TApplication*);

//
/// Return the window pointer given a window's handle.
//
/// Retrieves a TWindow pointer associated with the handle to a window (hWnd),
/// allowing more than one application to share the same HWND.
//
inline TWindow* TApplication::GetWindowPtr(HWND hWnd) const
{
  return ::owl::GetWindowPtr(hWnd, this);
}

//
/// Returns identifier of language used by application.
//
inline TLangId TApplication::GetLangId() const{
  return LangId;
}
//
/// Sets new language for use with application. Note that it only sets
/// the variable LangId.
//
inline void TApplication::SetLangId(TLangId landid){
  LangId = landid;
}

//
/// Set the data members with data from WinMain.
//
/// The ObjectWindows default WinMain function calls SetMainWinParams so that
/// TApplication can store the parameters for future use. To construct an
/// application instance, WinMain calls the OwlMain function that is in the user's
/// code. As it is being constructed, the application instance can fill in the
/// parameters using those set earlier by SetMainWinParams.
//
inline void TApplication::SetWinMainParams
(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  const tstring& cmdLine,
  int cmdShow
)
{
  InitHInstance = hInstance;
  InitHPrevInstance = hPrevInstance;
  GetInitCmdLine() = cmdLine;
  InitCmdShow = cmdShow;
}

//
/// Retrieve the WinMain parameters.
//
/// Initializes a static instance of an application. ObjectWindows OwlMain uses this
/// function to support static application instances.
//
inline void TApplication::GetWinMainParams()
{
  InitModule(InitHInstance, GetInitCmdLine());
  hPrevInstance = InitHPrevInstance;
  nCmdShow = InitCmdShow;
}

#if defined(OWL_SUPPORT_BWCC)
//
/// Indicates if the Borland Custom Controls library (BWCC) is enabled. Returns true
/// if BWCC is enabled and false if BWCC is disabled.
//
inline bool TApplication::BWCCEnabled() const
{
  return BWCCOn;
}

//
/// Returns a pointer to the enabled Borland Windows Custom Controls library (BWCC)
/// module.
//
inline TBwccDll* TApplication::GetBWCCModule() const
{
  return BWCCModule;
}
#endif

#if defined(OWL_SUPPORT_CTL3D)
//
/// Returns true if the Microsoft 3-D Controls Library DLL is enabled. This DLL
/// gives controls a three-dimensional look and feel.
//
inline bool TApplication::Ctl3dEnabled() const
{
  return Ctl3dOn;
}

//
/// If Ctl3D is enabled, return the module associated with it.
//
inline TCtl3dDll* TApplication::GetCtl3dModule() const
{
  return Ctl3dModule;
}
#endif

//
/// Get Tooltip
//
inline TTooltip* TApplication::GetTooltip() const
{
  return Tooltip;
}

/// Called by the main window (TFrameWindow) destructor to zero the MainWindow member
inline void TApplication::ClearMainWindow()
{
  MainWindow = nullptr;
}

inline int
TApplication::MessageBox(HWND wnd, const tstring& text, const tstring& caption, uint type) const
{
  return MessageBox(wnd, text.c_str(), caption.empty() ? nullptr : caption.c_str(), type);
}


} // OWL namespace

#endif  // OWL_APPLICAT_H

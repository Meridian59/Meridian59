//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcApp application connection class
//----------------------------------------------------------------------------

#if !defined(OCF_OCAPP_H)
#define OCF_OCAPP_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/ocbocole.h>
#include <ocf/ocobject.h>
#include <ocf/ocreg.h>
#include <ocf/autodefs.h>
#include <ocf/ocdata.h>
#include <ocf/ocapp.rh>
#include <owl/dispatch.h>
#include <owl/contain.h>

namespace owl {class _OWLCLASS TRegLink;};
namespace owl {class _OWLCLASS TRegList;};

namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

class _ICLASS   TOcPart;
class _ICLASS   TOcClassMgr;
class _ICLASS   TOcApp;

//
// OCF Mixin classes for users derived application class
//
#if !defined(_OCMCLASS)
# define _OCMCLASS
#endif
class _OCMCLASS  TOcAppHost;
class _OCMCLASS  TOcAppFrameHost;

//
/// \class TOcFormatName
// ~~~~~ ~~~~~~~~~~~~~
/// Clipboard format name
//
class _OCFCLASS TOcFormatName {
  public:
    TOcFormatName();
    TOcFormatName(LPCTSTR fmtName, LPCTSTR fmtResultName, LPCTSTR id = 0);

    bool operator ==(const TOcFormatName& other) const {return owl::ToBool(&other == this);}
    LPCTSTR  GetId() const {return Id.c_str();}
    LPCTSTR GetName() const {return Name.c_str();}
    LPCTSTR GetResultName() const {return ResultName.c_str();}

  private:
    owl::tstring Id;          // internal clipboard format name
    owl::tstring Name;        // external name of clipboard format (used in list box)
    owl::tstring ResultName;  // string to be used in the dialog help text
};

//
/// \class TOcNameList
// ~~~~~ ~~~~~~~~~~~
/// Clipboard format names
//
class _OCFCLASS TOcNameList : protected owl::TIPtrArray<TOcFormatName*>
{
  public:
    TOcNameList();
   ~TOcNameList();

    TOcFormatName*& operator [](unsigned index) {   return Base::operator[](index);}
    TOcFormatName*  operator [](LPTSTR);
    void    Clear(int del = 1)                                      { Base::Flush(del==1);}
    int     Add(TOcFormatName* name)                            { return Base::Add(name);}
    bool    IsEmpty() const                                             {   return Base::Empty();}
    bool    Find(const TOcFormatName* name)       { return Base::Find((TOcFormatName*)name); }
    owl::uint    Count() const                                                   {   return Base::Size();}
    bool    Detach(TOcFormatName* item, bool del = false)
                        {
                            return del ? Base::DestroyItem(item) : Base::DetachItem(item);
            }

  private:
    typedef owl::TIPtrArray<TOcFormatName*> Base;
};

#if defined(_OCFDLL) || defined(BI_APP_DLL)
  //
  // Export template of owl::TIPtrArray<TOcFormatName*> when building ObjectWindows
  // DLL and provide import declaration of DLL instance for users of the class.
  //
  template class owl::TIPtrArray<TOcFormatName*>;
#endif


//
/// \class TOcRegistrar
// ~~~~~ ~~~~~~~~~~~~
/// Linking & embeding version of the Registrar
//
class _OCFCLASS TOcRegistrar : public TRegistrar {
  public:
    TOcRegistrar(owl::TRegList& regInfo, TComponentFactory callback,
                 owl::uint32 preselectedOptions, owl::tstring& cmdLine, 
                 owl::TRegLink* linkHead = 0,
                 owl::TModule* module = & owl::GetGlobalModule());
   ~TOcRegistrar();

    void            CreateOcApp(owl::uint32 options, TOcAppHost* host,
                                TOcAppFrameHost* frameHost);
    void            CreateOcApp(owl::uint32 options, TOcApp*& ret);
    IBClassMgr*     CreateBOleClassMgr();

    void                        SetApp(TOcApp* app);

  protected:
    void * GetFactory(const GUID& clsid, const GUID & iid);
    bool      CanUnload();
    void      LoadBOle();

  private:
    HINSTANCE      BOleInstance;// BOle DLL instance
    IBClassMgr*    BCmI;        //
    TOcClassMgr*   OcClassMgr;  // our IBClassMgr implementation
    int            AppCount;    // TOcApp instance count
  friend class _ICLASS TOcApp;
};

//
/// \class TOcApp
// ~~~~~ ~~~~~~
/// OCF Application class.
//
class _ICLASS TOcApp : public TUnknown,
                       private IBApplication,
                       private IBClassMgr {
  public:
    // Constructor for OcApp & 2nd part of initialization
    //   host & frameHost are interfaces that OcApp talks back to.
    //
    TOcApp(TOcRegistrar& registrar, owl::uint32 options, TOcAppHost* host,
           TOcAppFrameHost* frameHost);
    void    SetupWindow(TOcAppFrameHost* frameHost);

    // !CQ compatibility only
    // Constructor for OcApp & 2nd part of initialization
    //   retOcApp is location holding unrefcounted pointer to this OcApp
    //
    TOcApp(TOcRegistrar& registrar, owl::uint32 options, TOcApp*& retOcApp);
    void    SetupWindow(HWND frameWnd);

    // Public accessors
    //
    TOcRegistrar& GetRegistrar() {return Registrar;}
    owl::tstring  GetName() const {return (LPCTSTR)Name;}
    bool    IsOptionSet(owl::uint32 option) const;
    void    SetOption(owl::uint32 bit, bool state);

    // Object reference & lifetime managment
    //
    virtual void   ReleaseObject();

    // Runtime class factory [un]registration
    //
    void RegisterClasses();
    void UnregisterClasses();

    // Clipboard
    //
    void         AddUserFormatName(LPCTSTR name, LPCTSTR resultName,
                                   LPCTSTR id = 0);
    TOcNameList& GetNameList() {return NameList;}

    // App side exposure of selected IBService functions
    //
    bool     UnregisterClass(const owl::tstring& progid);
    void     EvResize();
    void     EvActivate(bool);
    bool     EvSetFocus(bool set) {return HRSucceeded(BServiceI->OnSetFocus(set));}
    bool     RegisterClass(const owl::tstring& progid, BCID classId, bool multiUse);
    bool     TranslateAccel(MSG * msg)
               {return HRSucceeded(BServiceI->TranslateAccel(msg));}
    bool     ModalDialog(bool enable)
                 {return HRSucceeded(BServiceI->OnModalDialog(enable));}
    TOcHelp  AppHelpMode(TOcHelp newMode)
                 {return BServiceI->HelpMode(newMode);}
    bool     CanClose();

    bool     Browse(TOcInitInfo& initInfo);
    bool     BrowseControls(TOcInitInfo& initInfo);
    bool     BrowseClipboard(TOcInitInfo& initInfo);
    bool     Paste(TOcInitInfo& initInfo);
    bool     Copy(TOcPart* ocPart);
    bool     Copy(TOcDataProvider* ocData);
    bool     Drag(TOcDataProvider* ocData, TOcDropAction inAction, TOcDropAction& outAction);
    bool     Drag(TOcPart* ocData, TOcDropAction inAction, TOcDropAction& outAction);
    bool     Convert(TOcPart* ocPart, bool b);
    owl::uint     EnableEditMenu(TOcMenuEnable enable, IBDataConsumer * ocview);

    // Exposure of BOle's componentCreate for internal OCF support
    //
    HRESULT BOleComponentCreate(IUnknown * * retIface,
                                IUnknown * outer, owl::uint32 idClass);

    // Internal reference management
    //
    owl::ulong _IFUNC   AddRef() {return GetOuter()->AddRef();}
    owl::ulong _IFUNC   Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}
  protected:
   ~TOcApp();

    owl::uint32   Options;
    bool     Registered;

    TOcAppHost*      Host;
    TOcAppFrameHost* FrameHost;

// !CQ Ripping out the next guys
    TOcApp** OcAppPtr;   // Pointer to this instance, must zero on destruction
// !CQ    owl::TResult ForwardEvent(int eventId, const void* param);
// !CQ    owl::TResult ForwardEvent(int eventId, owl::TParam2 param = 0);

    // TUnknown virtual overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

  private:

    // IBWindow implementation
    //
    HWND     _IFUNC GetWindow();
    HRESULT  _IFUNC GetWindowRect(owl::TRect * r);
    LPCOLESTR _IFUNC GetWindowTitle();
    void     _IFUNC AppendWindowTitle(LPCOLESTR title);
    HRESULT  _IFUNC SetStatusText(LPCOLESTR text);

    HRESULT  _IFUNC RequestBorderSpace(const owl::TRect *);
    HRESULT  _IFUNC SetBorderSpace(const owl::TRect *);

    HRESULT  _IFUNC InsertContainerMenus(HMENU hMenu, TOcMenuWidths * omw);
    HRESULT  _IFUNC SetFrameMenu(HMENU hMenu);
    void     _IFUNC RestoreUI();
    HRESULT  _IFUNC Accelerator(MSG * msg);
    HRESULT  _IFUNC GetAccelerators(HACCEL *, int *);

    // IBApplication implementation
    //
    LPCOLESTR _IFUNC GetAppName();
    TOcHelp  _IFUNC HelpMode(TOcHelp newMode);
    HRESULT  _IFUNC CanLink();
    HRESULT  _IFUNC CanEmbed();
    HRESULT  _IFUNC IsMDI();
    HRESULT  _IFUNC OnModalDialog(BOOL svrModal);
    void     _IFUNC DialogHelpNotify(TOcDialogHelp);
    void     _IFUNC ShutdownMaybe();

    // IBClassMgr implementation
    //
    HRESULT  _IFUNC ComponentCreate(IUnknown * * ret,
                                    IUnknown * outer, owl::uint32 classId);
    HRESULT  _IFUNC ComponentInfoGet(IUnknown * * info,
                                     IUnknown * outer, owl::uint32 classId);

    void   Init();

    TOcRegistrar&  Registrar;   // Overall application description object

    // BOle side support
    //
    IBClassMgr*         BCmI;        // Our private class maneger
    IUnknown*               BService;    // BOle service object & interface
    IBService2*         BServiceI;   //

        owl::TString                 Name;
    TOcNameList         NameList;
    bool                        DisableDlgs; // Server went modal--disable our dialogs

  friend class _ICLASS TOcDocument;
  friend class _ICLASS TOcPart;
  friend class _ICLASS TOcRemView;
  friend class _ICLASS TOcView;
  friend class _ICLASS TOcClassMgr;
};

//
//inline void TOcApp::SetupWindow(TOcAppFrameSocket* frameSocket)
//{
//  FrameSocket = frameSocket;
//}

//
inline bool TOcApp::IsOptionSet(owl::uint32 option) const
{
  return owl::ToBool(Options & option);
}

//
inline void TOcApp::SetOption(owl::uint32 bit, bool state)
{
  if (state)
    Options |= bit;
  else
    Options &= ~bit;
}

//
inline void TOcRegistrar::CreateOcApp(owl::uint32 options, TOcAppHost* host,
                                      TOcAppFrameHost* frameHost)
{
  new TOcApp(*this, options, host, frameHost);
}

//
// Compatibility version of TOcApp creator
//
inline void TOcRegistrar::CreateOcApp(owl::uint32 options, TOcApp*& retOcApp)
{
  new TOcApp(*this, options, retOcApp);
}

//----------------------------------------------------------------------------

//
// ObjectComponents message and subdispatch IDs
//
#define WM_BCXNAME          (0x7FFF-8)     // (WM_VBXNAME) BCX event fire by name
#define WM_OCEVENT          (WM_BCXNAME-1) // OC -> app messages

//
// Subdispatch IDs for TOcApp clients
//
#define OC_APPINSMENUS        0x0100  // Main window insert menus
#define OC_APPMENUS           0x0101  // Main window set menu
#define OC_APPPROCESSMSG      0x0102  // Process msg for accel, etc
#define OC_APPFRAMERECT       0x0103  // Get inner rect of main window
#define OC_APPBORDERSPACEREQ  0x0104  // Request/set app frame border space
#define OC_APPBORDERSPACESET  0x0105  // Request/set app frame border space
#define OC_APPSTATUSTEXT      0x0106  // Set the status text
#define OC_APPRESTOREUI       0x0107  // Have the app frame restore its UI
#define OC_APPDIALOGHELP      0x0108  // Ole dialog help button pressed
#define OC_APPSHUTDOWN        0x0109  // Shutdown app frame window
#define OC_APPGETACCEL        0x010A  // Get accelerators for menu merge

//
// struct TOcGetAccel
// ~~~~~~ ~~~~~~~~~~~
struct TOcGetAccel {
  HACCEL  Accel;
  int     Count;
};

//
/// \class TOcAppHost
// ~~~~~ ~~~~~~~~~~
/// OCF Application host class. Owner of & host for a TOcApp object
//
class _OCMCLASS TOcAppHost {
  public:
    TOcAppHost() : OcApp(0) {} // Must call OcInit after construction
   ~TOcAppHost();

    void OcInit(TOcRegistrar& registrar, owl::uint32 options);
    TRegistrar& GetRegistrar() {return OcApp->GetRegistrar();}

    bool IsOptionSet(owl::uint32 option) const;

    virtual void AttachHost(TOcApp* ocApp) {OcApp = ocApp;}
    virtual void ReleaseOcObject() {OcApp = 0;}

//  protected:
    TOleAllocator OleMalloc;   // default to task allocator
    TOcApp*       OcApp;

  private:
    owl::uint32 InitOptions;
};

// Old name don't use it;
// typedef not work with dynamic_cast of VC ++
//typedef TOcAppHost TOcModule;
class TOcModule: public TOcAppHost {
    public:
        TOcModule():TOcAppHost(){}
        ~TOcModule(){}
};

//
/// \class TOcAppFrameHost
// ~~~~~ ~~~~~~~~~~~~~~~
/// OCF Application frame window host class. Receives window events from OcApp
//
class _OCMCLASS TOcAppFrameHost {
  public:
    TOcAppFrameHost(TOcApp* ocApp=0) : OcApp(ocApp) {}

    virtual void  ReleaseOcObject() {OcApp = 0;}

    virtual bool  EvOcAppInsMenus(TOcMenuDescr & sharedMenu) = 0;
    virtual bool  EvOcAppMenus(TOcMenuDescr & md) = 0;
    virtual bool  EvOcAppProcessMsg(MSG * msg) = 0;
    virtual bool  EvOcAppFrameRect(owl::TRect * rect) = 0;
    virtual bool  EvOcAppBorderSpaceReq(owl::TRect * rect) = 0;
    virtual bool  EvOcAppBorderSpaceSet(owl::TRect * rect) = 0;
    virtual void  EvOcAppStatusText(const char * rect) = 0;
    virtual void  EvOcAppRestoreUI() = 0;
    virtual void  EvOcAppDialogHelp(TOcDialogHelp & dh) = 0;
    virtual bool  EvOcAppShutdown() = 0;
    virtual bool  EvOcAppGetAccel(TOcGetAccel * acc) = 0;

    virtual HWND  EvOcGetWindow() const = 0;

    // For compatibility
    virtual void  SetWindow(HWND) {}

  protected:
    TOcApp* OcApp;  // !CQ really want this?
};

//
/// \class TOcAppFrameHostMsg
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Default implementation of frame window host that uses messages
//
class _OCMCLASS TOcAppFrameHostMsg : public TOcAppFrameHost {
  public:
    TOcAppFrameHostMsg(TOcApp* ocApp) : Wnd(0), TOcAppFrameHost(ocApp) {}

    void   ReleaseOcObject() {delete this;}  // We are no longer needed by Oc

    bool   EvOcAppInsMenus(TOcMenuDescr & sharedMenu);
    bool   EvOcAppMenus(TOcMenuDescr & md);
    bool   EvOcAppProcessMsg(MSG * msg);
    bool   EvOcAppFrameRect(owl::TRect * rect);
    bool   EvOcAppBorderSpaceReq(owl::TRect * rect);
    bool   EvOcAppBorderSpaceSet(owl::TRect * rect);
    void   EvOcAppStatusText(const char * rect);
    void   EvOcAppRestoreUI();
    void   EvOcAppDialogHelp(TOcDialogHelp & dh);
    bool   EvOcAppShutdown();
    bool   EvOcAppGetAccel(TOcGetAccel * acc);

    HWND   EvOcGetWindow() const {return Wnd;}

    void   SetWindow(HWND hWnd) {Wnd = hWnd;}

  protected:
    owl::TResult ForwardEvent(int eventId, const void* param);
    owl::TResult ForwardEvent(int eventId, owl::TParam2 param = 0);

  private:
    HWND   Wnd;
};

//
inline TOcAppHost::~TOcAppHost()
{
  if (OcApp)
    OcApp->ReleaseObject();  // don't delete, just release OC object
}

inline bool TOcAppHost::IsOptionSet(owl::uint32 option) const
{
  return OcApp ? OcApp->IsOptionSet(option) : (InitOptions & option) != 0;
}

inline void TOcAppHost::OcInit(TOcRegistrar& registrar, owl::uint32 options)
{
  InitOptions = options;
  registrar.CreateOcApp(options, OcApp);
}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OCF namespace


#endif  // OCF_OCAPP_H

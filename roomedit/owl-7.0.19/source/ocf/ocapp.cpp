//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
///   Implementation of TOcApp Class
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/autodefs.h>
#include <ocf/appdesc.h>
#include <ocf/ocreg.h>
#include <ocf/ocapp.h>
#include <ocf/ocpart.h>
# include <winver.h>

namespace ocf {

using namespace owl;

using namespace std;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OcfApp, 1, 0);
DIAG_DECLARE_GROUP(OcfDll);
DIAG_DECLARE_GROUP(OcfRefCount);

//
/// Compatibility constructor & SetupWindow for TOcApp
//
TOcApp::TOcApp(TOcRegistrar& registrar, uint32 options, TOcApp*& retOcApp)
:
  Host(0),
  FrameHost(new TOcAppFrameHostMsg(this)), // Use default, msg based host
  OcAppPtr(&retOcApp),
  Options(options),
  DisableDlgs(false),
  Registrar(registrar),
  Registered(false)
{
  Registrar.AppCount++;
  Init();
  RegisterClasses();

  *OcAppPtr = this;    // Setup the client's pointer last if all else is OK
  TRACEX(OcfRefCount, 1, "TOcApp() @" << (void*)this);

  // !CQ Could calculate Host from OcAppPtr, assuming a TOcAppHost
  // Host = (char*)OcAppPtr - int(&(TOcAppHost*)0->OcAppPtr);
}

//
void TOcApp::SetupWindow(HWND frameWnd)
{
  FrameHost->SetWindow(frameWnd);  // Pass frame's hWnd over to frame host

  Registrar.SetApp(this);
}

//
/// New constructor for TOcApp using host interfaces
//
TOcApp::TOcApp(TOcRegistrar& registrar, uint32 options, TOcAppHost* host,
               TOcAppFrameHost* frameHost)
:
  Host(host),
  FrameHost(frameHost),
  OcAppPtr(0),
  Options(options),
  DisableDlgs(false),
  Registrar(registrar),
  Registered(false)
{
  PRECONDITION(&host);
  PRECONDITION(&frameHost);

  Registrar.AppCount++;
  Init();
  RegisterClasses();

  if (Host)
    Host->AttachHost(this);

  TRACEX(OcfRefCount, 1, "TOcApp() @" << (void*)this);
}

//
void TOcApp::SetupWindow(TOcAppFrameHost* frameHost)
{
  FrameHost = frameHost;
}

//
/// Common constructor initialization
//
void
TOcApp::Init()
{
  // Initialize BOle service ptrs that may or may not get setup later
  //
  BService = 0;
  BServiceI = 0;

  AddRef();  // TUnknown defaults to 0, we need 1

  // Create a BOle class manager for this app
  //
  BCmI = Registrar.CreateBOleClassMgr();

  // Create BOle service object & get its interface
  //
  BOleComponentCreate(&BService, GetOuter(), cidBOleService);
  if (BService && HRSucceeded(BService->QueryInterface(IID_IBService2,
                                                       (LPVOID *)&BServiceI))) {
    Release();
    BServiceI->Init(this);
  }
  else
    TXObjComp::Throw(TXObjComp::xBOleBindFail); // give up if no IBService2

  // Get the clipboard format strings
  //
  for (uint i = IDS_CFFIRST; i <= IDS_CFLAST; i++) {
    TCHAR name[128];
    if(GetGlobalModule().LoadString(i, name, 128)){
      _TCHAR* resultName = _tcschr(name, _T('\n'));
      *resultName++ = 0;
       NameList.Add(new TOcFormatName(name, resultName));
    }
  }

  // Get & copy the appname from the reginfo
  //
  Name = Registrar.GetAppDescriptor().GetAppName(LangSysDefault);
}

//
/// Clean up this object be releasing all helpers.
//
TOcApp::~TOcApp()
{
  // We're gone, make sure nobody calls us through unref'd ptr
  //
  if (OcAppPtr)     // !CQ OcAppPtr for compatibility
    *OcAppPtr = 0;
  if (Host) {
    Host->ReleaseOcObject();
    Host = 0;
  }

  UnregisterClasses();

  // RefCnt was held by controller
  //
  if (FrameHost) {
    if (FrameHost->EvOcAppShutdown() &&
        IsOptionSet(amServedApp) && !IsOptionSet(amExeModule)) {
      Registrar.Shutdown(0, Options);
    }
    FrameHost->ReleaseOcObject();
    FrameHost = 0;
  }

  if (BService)
    BService->Release();
  Registrar.AppCount--;
  if (BCmI)
    BCmI->Release();
}

//
/// Should only be called by the owner/creator of this object
//
void
TOcApp::ReleaseObject()
{
  if (Host) {
    Host->ReleaseOcObject();
    Host = 0;
  }
  if (FrameHost) {
    FrameHost->ReleaseOcObject();
    FrameHost = 0;
  }
  if (!IsOptionSet(amServedApp))
    Release();   // if our container app holds the refcnt, then release it
}

//
/// Callback from TUnknown's implementation of QueryInterface
//
HRESULT
TOcApp::QueryObject(const IID & iid, void * * iface)
{
  HRESULT hr;

  static_cast<void>
  (
  // interfaces
     SUCCEEDED(hr = IBApplication_QueryInterface(this, iid, iface))
  || SUCCEEDED(hr = IBClassMgr_QueryInterface(this, iid, iface))

  // helpers
  || (BService && SUCCEEDED(hr = BService->QueryInterface(iid, iface)))
  );
  return hr;
}

//----------------------------------------------------------------------------

//
/// Create a BOle helper for one of our OC objects in this app
//
HRESULT
TOcApp::BOleComponentCreate(IUnknown * * iface, IUnknown* outer, BCID idClass)
{
  return BCmI->ComponentCreate(iface, outer, idClass);
}

//----------------------------------------------------------------------------
// Runtime document factory registration

//
/// Register doc factories based on their 'progid' and their templates
//
void
TOcApp::RegisterClasses()
{
  if (Registered)
    return;

  // No class registration for InProc Servers
  //
  if (!(IsOptionSet(amExeModule)))
    return;

  // No class registration for single use apps unless embedded
  //
  if (IsOptionSet(amSingleUse) &&
     (IsOptionSet(amAutomation)|| !IsOptionSet(amEmbedding)))
    return;

  // Loop thru all templates, registering registerable classes
  //
  const TRegLink* link = GetRegistrar().GetAppDescriptor().GetRegLinkHead();
  for ( ; link; link = link->GetNext()) {
    TRegList&   regList = link->GetRegList();
    const _TCHAR* progid = regList[IsOptionSet(amDebug) ? "debugprogid" : "progid" ];
    if (progid) {

      // Don't register container classes unless embedding
      //
      if (!IsOptionSet(amEmbedding) &&
          !(const _TCHAR*)regList["insertable"])
        continue;

      bool multiUse = !IsOptionSet(amSingleUse);
      if (multiUse) {
        const _TCHAR* usage = regList.Lookup("usage");
        _TCHAR su[] = ocrSingleUse;
        multiUse = ToBool(!(usage && *usage == *su));
      }
      if (!RegisterClass(progid, reinterpret_cast<BCID>(link), multiUse))
        TXObjComp::Throw(TXObjComp::xDocFactoryFail);
    }
  }
  Registered = true;
}

//
/// Unregister doc class factories based on their 'progid' and their templates
//
void
TOcApp::UnregisterClasses()
{
  if (!Registered)
    return;

  // Loop thru all templates, unregistering registerable classes
  //
  const TRegLink* link = GetRegistrar().GetAppDescriptor().GetRegLinkHead();
  for ( ; link; link = link->GetNext()) {
    TRegList& regList = link->GetRegList();
    const _TCHAR* progid = regList[IsOptionSet(amDebug) ? "debugprogid" : "progid" ];
    if (progid)
      if (!UnregisterClass(progid))
        TXObjComp::Throw(TXObjComp::xDocFactoryFail);
  }
  Registered = false;
}

//
/// Add a user defined clipboard format name
//
void
TOcApp::AddUserFormatName(LPCTSTR name, LPCTSTR resultName, LPCTSTR id)
{
  NameList.Add(new TOcFormatName(name, resultName, id));
}


//----------------------------------------------------------------------------
// OC side exposure of selected IBService functions

bool
TOcApp::UnregisterClass(const owl::tstring& className)
{
  if (BServiceI)
    return HRSucceeded(BServiceI->UnregisterClass(OleStr(className.c_str())));
  return false;
}

//
/// Let BOle know that the main window has resized. In-place servers may need to
/// adjust their toolbars
//
void
TOcApp::EvResize()
{
  if (BServiceI)
    BServiceI->OnResize();
}

//
/// Let BOle know that the main window has [de]activated.
//
void
TOcApp::EvActivate(bool active)
{
  if (BServiceI)
    BServiceI->OnActivate(active);
}

bool
TOcApp::RegisterClass(const owl::tstring& className, BCID classId, bool multiUse)
{
  // Self-embedding works only if the app is multi-use
  //
  if (BServiceI)
    return HRSucceeded(BServiceI->RegisterClass(OleStr(className.c_str()),
                       this, classId, ToBool(multiUse), ToBool(!multiUse)));
  return false;
}

bool
TOcApp::CanClose()
{
  return BServiceI ? HRIsOK(BServiceI->CanClose()) : true;  // there are no servers running
}

uint
TOcApp::EnableEditMenu(TOcMenuEnable menuEnable, IBDataConsumer* ocView)
{
  return BServiceI ? BServiceI->EnableEditMenu(menuEnable, ocView) : 0;
}

bool
TOcApp::Browse(TOcInitInfo& init)
{
  return BServiceI ? HRIsOK(BServiceI->Browse(&init)) : false;
}

bool
TOcApp::BrowseControls(TOcInitInfo& init)
{
  return BServiceI ? HRIsOK(BServiceI->BrowseControls(&init)) : false;
}

bool
TOcApp::BrowseClipboard(TOcInitInfo& init)
{
  return BServiceI ? HRIsOK(BServiceI->BrowseClipboard(&init)): false;
}

bool
TOcApp::Paste(TOcInitInfo& init)
{
  return BServiceI ? HRIsOK(BServiceI->Paste(&init)) : false;
}

//
/// Copy Selected embedded object
//
bool
TOcApp::Copy(TOcPart* ocPart)
{
  IBPart* bPartI;
  if (ocPart && SUCCEEDED(ocPart->QueryInterface(IID_IBPart, (LPVOID *)&bPartI))) {
    ocPart->Release();

    // Copy part with delayed rendering done by Bolero
    //
    return BServiceI ? HRIsOK(BServiceI->Clip(bPartI, true, true, true)) : false;
  }
  return false;
}

//
/// Copy a selection in server document
//
bool
TOcApp::Copy(TOcDataProvider* ocData)
{
  PRECONDITION(ocData);

  if (BServiceI)
    BServiceI->Clip(0, false, false, false);
  else
    return false;

  return HRIsOK(BServiceI->Clip(ocData, true, true, false));
}

//
/// Drag a selection
//
bool
TOcApp::Drag(TOcDataProvider* ocData, TOcDropAction inAction, TOcDropAction& outAction)
{
  PRECONDITION(ocData);

  return BServiceI? HRSucceeded(BServiceI->Drag(ocData, inAction, &outAction)) : false;
}

//
/// Drag an embedded object
//
bool
TOcApp::Drag(TOcPart* ocPart, TOcDropAction inAction, TOcDropAction& outAction)
{
#if 1
  IBPart * bPartI;
  if (ocPart && SUCCEEDED(ocPart->QueryInterface(IID_IBPart, (LPVOID *)&bPartI))) {
    ocPart->Release();
    // Drag part with delayed rendering done by Bolero
    //
    return HRSucceeded(BServiceI->Drag(bPartI, inAction, &outAction));
  }
  return false;
#else
  return BServiceI? HRSucceeded(BServiceI->Drag(ocPart, inAction, &outAction)) : false;
#endif
}

//
/// Open the Convert dialog, and perform the conversion if OK. return true if
/// conversion was perfromed successfully.
//
bool
TOcApp::Convert(TOcPart* ocPart, bool b)
{
  PRECONDITION(ocPart);

  if (BServiceI == 0)
    return false;

  // The Convert dialog is split into two pieces: one to run the dialog box
  // and one to do the actual work. This way, the caller can record the
  // actions of the dialog box for playback later.
  //
  TOcConvertInfo ci;
  if (HRIsOK(BServiceI->ConvertUI(ocPart->GetBPartI(), b, &ci)))
    return HRSucceeded(BServiceI->ConvertGuts(ocPart->GetBPartI(), b, &ci));

  return false;
}

//----------------------------------------------------------------------------
// IBApplication implementation

//
/// Return the application's name
//
LPCOLESTR _IFUNC
TOcApp::GetAppName()
{
  return Name;
}

TOcHelp _IFUNC
TOcApp::HelpMode(TOcHelp /*newMode*/)
{
  // No built in help support
  //
  return hlpExit;
}

//
/// Insert the container's menus into a provided menubar
//
HRESULT _IFUNC
TOcApp::InsertContainerMenus(HMENU hMenu, TOcMenuWidths * omw)
{
  PRECONDITION(omw);
  if (!hMenu)
    return HR_NOERROR;

  TOcMenuDescr md;
  md.HMenu = hMenu;

  int i;
  for (i = 0; i < 6; i++) {
    md.Width[i]    = 0;  // make sure the server's are zeroed
    omw->Width[i] = 0;  // make sure the server's are zeroed
    i++;
    md.Width[i] = (int)omw->Width[i];
  }

  if (!FrameHost || !FrameHost->EvOcAppInsMenus(md))
    return HR_FAIL;

  for (i = 0; i < 6; i++)
    omw->Width[i] = md.Width[i];

  return HR_NOERROR;
}

//
/// Now set the provided menubar into the container's main frame window
//
HRESULT _IFUNC
TOcApp::SetFrameMenu(HMENU hMenu)
{
  TOcMenuDescr md;
  md.HMenu = hMenu;
  return HRFailIfZero(FrameHost && FrameHost->EvOcAppMenus(md));
}

//
//
//
HRESULT _IFUNC
TOcApp::Accelerator(MSG* msg)
{
  PRECONDITION(msg);

  return HRFailIfZero(FrameHost && FrameHost->EvOcAppProcessMsg(msg));
}

//
/// Let BOle know if we (container app) have an accelerator table
//
HRESULT _IFUNC
TOcApp::GetAccelerators(HACCEL * phAccel, int * pcAccel)
{
  TOcGetAccel acc;
  if (FrameHost) {
    if (FrameHost->EvOcAppGetAccel(&acc)) {
      *phAccel = acc.Accel;
      *pcAccel = acc.Count;
      return HR_NOERROR;
    }
  }
  return HR_FAIL;  // would retrieve or generate an accelerator table here
}

//
/// Let BOle know if this app can/will accept links
//
HRESULT _IFUNC
TOcApp::CanLink()
{
  return HR_OK;  // return HR_FAIL to disallow Linking
}

//
/// Let BOle know if this app can/will accept embeddings
//
HRESULT _IFUNC
TOcApp::CanEmbed()
{
  return HR_OK;  // return HR_FAIL to disallow Embedding
}

//
/// Get and return the app frame's HWND
//
HWND _IFUNC
TOcApp::GetWindow()
{
  return FrameHost ? FrameHost->EvOcGetWindow() : 0;
}

//
// Get client rectangle of app's main frame window
//
HRESULT _IFUNC
TOcApp::GetWindowRect(TRect* r)
{
  PRECONDITION(r);

  return HRFailIfZero(FrameHost && FrameHost->EvOcAppFrameRect(r));
}

//
/// Return the app's title, same as GetAppName()
//
LPCOLESTR _IFUNC
TOcApp::GetWindowTitle()
{
  return Name;
}

//
/// The server is asking for space along the app borders to put toolbars, etc.
/// This call is used to determine whether the container is willing and able to
/// provide a given combination.
//
HRESULT _IFUNC
TOcApp::RequestBorderSpace(const TRect* space)
{
  TRect* nc_space = CONST_CAST(TRect*, space); /// Ack!
  return HRFailIfZero(FrameHost && FrameHost->EvOcAppBorderSpaceReq(nc_space));
}

//
/// Now, actually provide the space along the app frame borders for inplace
/// server adornments
//
HRESULT _IFUNC
TOcApp::SetBorderSpace(const TRect* space)
{
  TRect * nc_space = CONST_CAST(TRect*, space); /// Ack!
  return HRFailIfZero(FrameHost && FrameHost->EvOcAppBorderSpaceSet(nc_space));
}

//
/// Append supplied Ole title to frame's title, saving old title
//
void _IFUNC
TOcApp::AppendWindowTitle(LPCOLESTR /*title*/)
{
  ///
}

//
/// Pass status bar text to container app to have app display it
//
HRESULT _IFUNC
TOcApp::SetStatusText(LPCOLESTR text)
{
  // Convert OLE str into ANSI
  //
  if (FrameHost) {
    FrameHost->EvOcAppStatusText(OleStr(text));
    return HR_OK;
  }
  return HR_FAIL;
//  return HRFailIfZero(FrameHost && FrameHost->EvOcAppStatusText(OleStr(text)));
}

//
/// Respond to let BOle know if our app is MDI or not
//
HRESULT _IFUNC
TOcApp::IsMDI()
{
  // Since this flag is used only to do toolbar negotiation,
  // we're always MDI as far as BOle is concerned.
  //
  return HR_NOERROR;
}

//
/// The server is entering or leaving a modal state. Keep track so that we don't
/// interfere when it is modal.
//
HRESULT _IFUNC
TOcApp::OnModalDialog(BOOL svrModal)
{
  DisableDlgs = (bool)svrModal;
  return HR_NOERROR;
}

//
/// The in-place server is done. Tell the container to restore its normal UI.
/// We can handle the window text here, let the app do the rest.
//
void _IFUNC
TOcApp::RestoreUI()
{
  SetStatusText(0);
  if (FrameHost)
    FrameHost->EvOcAppRestoreUI();
}

//
//
//
void _IFUNC
TOcApp::DialogHelpNotify(TOcDialogHelp help)
{
  if (FrameHost)
    FrameHost->EvOcAppDialogHelp(help);
}

//
/// Called by BOle when last embedding is closed
/// If that's the only reason the app is up we need to shut ourselves down
//
void _IFUNC
TOcApp::ShutdownMaybe()
{
  TRACEX(OcfApp, 1,
         "ShutdownMaybe() on " << (void*)this <<
         " Embedding:" << (int)ToBool(IsOptionSet(amEmbedding)) <<
         " Win:" << hex << static_cast<void*>(GetWindow()));

  // Check first to see if TOcApp should initiate a shutdown
  //
  if (!FrameHost || FrameHost->EvOcAppShutdown() || !GetWindow()) {
    // The server initiated the shutdown
    //
    if (!IsOptionSet(amExeMode)) {  // DLL server
      AddRef();    // Prevent destroying ourselves yet
      Registrar.Shutdown((IUnknown*)(void*)this, Options);
      Release();   // This should do us in now
    }
  }
}

//-----------------------------------------------------------------------------
// TOcClassMgr, IBClassMgr implementation for TOcRegistrar
//

class _ICLASS TOcClassMgr : private TUnknown, public IBClassMgr {
  public:
    TOcClassMgr(TComponentFactory cc, uint32 options);
    ~TOcClassMgr();
    ulong _IFUNC   AddRef() ;
    ulong _IFUNC   Release();

    void            SetApp(TOcApp* app) {OcApp = app;}

  private:
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}
    HRESULT _IFUNC ComponentCreate(IUnknown * * iface,
                                   IUnknown * outer, BCID classId);
    HRESULT _IFUNC ComponentInfoGet(IUnknown * * info,
                                    IUnknown * outer, BCID classId);
    // TUnknown virtual overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

    TComponentFactory   OcCallback;    // Callback for creating component
    uint32              Options;       // Options flags from TOcRegistrar
    TOcApp*    OcApp;
    friend class TOcApp;  // could delegate the interface instead...
};

TOcClassMgr::TOcClassMgr(TComponentFactory cc, uint32 options)
:
  OcCallback(cc),
  Options(options),
  OcApp (0)
{
}

TOcClassMgr::~TOcClassMgr()
{
}

ulong _IFUNC
TOcClassMgr::AddRef()
{
  return GetOuter()->AddRef();
}

ulong _IFUNC
TOcClassMgr::Release()
{
  return GetOuter()->Release();
}

//
// IBClassMgr implementation for TOcRegistrar
//
HRESULT _IFUNC
TOcClassMgr::ComponentCreate(IUnknown * * retIface, IUnknown * outer, BCID idClass)
{
  PRECONDITION(idClass && retIface);

  *retIface = 0;
  if (!OcCallback)
    return HR_FAIL;

  try {

    // Test for special condition to force run as an EXE
    //
    void * v;
    if (outer && !(Options & amExeModule) && outer->QueryInterface(IID_NULL, &v) == HR_NOERROR)
      *retIface = OcCallback(0, Options | amExeMode | amRun, idClass);
    else
      *retIface = OcCallback(outer, Options | amEmbedding, idClass);
    return *retIface ? HR_OK : HR_FAIL;
  }
  catch (...) {  // we can't throw any exception through OLE
    // if an exception occur shutdown the application if it needs to be so
    if (OcApp)
      OcApp->ShutdownMaybe ();

    return HR_OUTOFMEMORY;  // probably a resource problem, better error code?
  }
}

HRESULT _IFUNC
TOcClassMgr::ComponentInfoGet(IUnknown * * info, IUnknown * /*outer*/,
                              BCID /*idClass*/)
{
  *info = 0;
  return HR_FAIL;
}

HRESULT
TOcClassMgr::QueryObject(const IID & iid, void * * iface)
{
  HRESULT hr;

  // interfaces
  HRSucceeded(hr = IBClassMgr_QueryInterface(this, iid, iface))
  ;
  return hr;
}

//----------------------------------------------------------------------------
// IBClassMgr implementation for TOcApp
//

HRESULT _IFUNC
TOcApp::ComponentCreate(IUnknown * * ret, IUnknown * outer, BCID classId)
{
  return Registrar.OcClassMgr->ComponentCreate(ret, outer, classId);
}

HRESULT _IFUNC
TOcApp::ComponentInfoGet(IUnknown * * info, IUnknown * outer, BCID classId)
{
  return Registrar.OcClassMgr->ComponentInfoGet(info, outer, classId);
}

//-----------------------------------------------------------------------------
// TOcRegistrar
//
/// The 'preselectedOptions' parameter enables to preserve old (pre 6.40)
/// behaviour (see Bug #376):
/// Set to amQuietReg, no exception is thrown on registration failures 
/// which nowadays occur frequently due to restricted user rights during
/// registration updates.
/// This option should only be set when cmdLine is empty, otherwise wanted 
/// exceptions on registration failures will be suppressed too.
/// The amNoRegValidate bit may be set to suppress the automatic registration
/// update at all.
//
TOcRegistrar::TOcRegistrar(TRegList& regInfo, TComponentFactory callback,
                           owl::uint32 preselectedOptions, 
                           owl::tstring& cmdLine, TRegLink* linkHead,
                           TModule* module)
:
  TRegistrar(*new TAppDescriptor(regInfo, callback, cmdLine, module, linkHead, preselectedOptions)),
  BOleInstance(0),
  BCmI(0),
  OcClassMgr(0),
  AppCount(0)
{
  OcClassMgr = new TOcClassMgr(callback, GetOptions());
  OcClassMgr->AddRef();
}

TOcRegistrar::~TOcRegistrar()
{
  if (BCmI)
    BCmI->Release();
  if (OcClassMgr)
    OcClassMgr->Release();
  if (BOleInstance > HINSTANCE(32))
    ::FreeLibrary(BOleInstance);
}

//
// Create and return a BOle class manager helper interface with 1 ref on it
//

typedef HRESULT (PASCAL *TCreateClassMgr)(IUnknown**, IUnknown*, IMalloc*);

IBClassMgr*
TOcRegistrar::CreateBOleClassMgr()
{
  if (!BOleInstance)
    LoadBOle();
  TCreateClassMgr createClassMgr = (TCreateClassMgr)::GetProcAddress(BOleInstance, BOLEBIND);
  if (createClassMgr) {

    // Call thru the exported function to get a BOle class manager
    // Don't aggregate it to anything
    //
    IUnknown*   bcm;
    createClassMgr(&bcm, 0, 0);
    if (bcm) {
      IBClassMgr* bcmi = nullptr;
      bcm->QueryInterface(IID_IBClassMgr, (LPVOID  *)&bcmi);
      bcm->Release();
      if (bcmi)
        return bcmi;
    }
  }
  TXObjComp::Throw(TXObjComp::xBOleBindFail);
  return 0; // never reached
}

//
/// Override TRegistrar's GetFactory to provide additional factory support
/// using BOle factories
//
void*
TOcRegistrar::GetFactory(const GUID& clsid, const GUID& iid)
{
  void* factory = TRegistrar::GetFactory(clsid, iid);
  if (factory)
    return factory;

  if (!BCmI)
    BCmI = CreateBOleClassMgr();

  IUnknown* objFactory = 0;
  IBClass* classMgr    = 0;

  TRegLink* link = GetAppDescriptor().GetRegLink(clsid);
  if (!link)
    return 0;

  TRegList&   regList = link->GetRegList();
  LPCTSTR progid = regList[IsOptionSet(amDebug) ? "debugprogid" : "progid" ];

  // Create BoleFactory helper object & init it, giving it our OcClassMgr
  // object to work with
  //
  if (!(HRSucceeded(BCmI->ComponentCreate(&objFactory, 0, cidBOleFactory)) &&
      HRSucceeded(objFactory->QueryInterface(IID_IBClass, (LPVOID *)&classMgr)) &&
      HRSucceeded(classMgr->Init(false, OleStr(progid), OcClassMgr, reinterpret_cast<BCID>(link))) &&
      HRSucceeded(classMgr->QueryInterface(iid, &factory)))) {
    if (objFactory)
      objFactory->Release();
    if (classMgr)
      classMgr->Release();

    return 0;
  }

  return factory;
}

bool
TOcRegistrar::CanUnload()
{
  TRACEX(OcfDll, 1, "CanUnload() AppCount:" << AppCount);
  return TRegistrar::CanUnload() && AppCount == 0;
}

void
TOcRegistrar::SetApp(TOcApp* app)
{
  OcClassMgr->SetApp(app);
}

static bool
sGetFileVersionInfo(LPCTSTR fileName, VS_FIXEDFILEINFO& vInfo)
{
  OLECHAR* viBuff;      // version buffer
  uint32   infoSize;    // Size of version info resource in file

  // Find out how big the file version info buffer is supposed to be and
  // create a buffer of that size
  //
  uint32 infoHandle;
  infoSize = ::GetFileVersionInfoSize(OleStr(fileName), &infoHandle);
  if (infoSize == 0)
    return false;

  viBuff = new OLECHAR[(int)infoSize];

  // Copy the file version info buffer from the file into viBuff
  //
  if (::GetFileVersionInfo(OleStr(fileName), 0, infoSize, viBuff)) {

    // Perform some magic on the phantom buffer to get an actual structure with
    // the version information
    //
    uint vInfoLen;
    VS_FIXEDFILEINFO * vInfoPtr;
    if (::VerQueryValue(viBuff, _T("\\"), (LPVOID *)&vInfoPtr, &vInfoLen)) {
      vInfo = *vInfoPtr;
      delete[] viBuff;
      return true;
    }
  }
  delete[] viBuff;
  return false;
}

//
/// Dynamically load the OcOle Dll, get the one entry point that we need &
/// make the class manager object that we use
//
void
TOcRegistrar::LoadBOle()
{
  // Check BOle DLL existance & version first, failing if it is incompatible
  // (old)
  //
  char name[30];
  OFSTRUCT ofs;
  ofs.cBytes = sizeof ofs;

#ifdef NT_PREFERS_UNICODE_INSTEAD_OF_ANSI
  bool winNT = !ToBool(::GetVersion()&0x80000000);
  if (winNT) // NT platform
  {
    lstrcpy(name, BOLEDLLW);

    // Try the ANSI dll if couldn't find Unicode version
    if (::OpenFile(name, &ofs, OF_EXIST) == HFILE_ERROR)
      lstrcpy(name, BOLEDLL);
  }
  else
#endif
    _USES_CONVERSION;
    strcpy(name, _W2A(BOLEDLL));

  if (::OpenFile(name, &ofs, OF_EXIST) >= 0) {
    VS_FIXEDFILEINFO vInfo;
    if (!sGetFileVersionInfo(_A2W(name), vInfo) ||
      vInfo.dwFileVersionMS < BOLE_FILEVER_MS ||
      (
        vInfo.dwFileVersionMS == BOLE_FILEVER_MS &&
        vInfo.dwFileVersionLS < BOLE_FILEVER_LS
      ))
      TXObjComp::Throw(TXObjComp::xBOleVersFail);

    BOleInstance = ::LoadLibraryA(ofs.szPathName);
  }

  // If we failed to load the DLL, throw a general cannot-load exception.
  // Otherwise get the class manager interface
  //
  if (BOleInstance <= HINSTANCE(32))
    TXObjComp::Throw(TXObjComp::xBOleLoadFail);
}

//-----------------------------------------------------------------------------
// TOcFormatName
//

TOcFormatName::TOcFormatName()
{
}

TOcFormatName::TOcFormatName(LPCTSTR name, LPCTSTR resultName, LPCTSTR id)
:
  Name(name),
  ResultName(resultName),
  Id( id ? id : _T(""))
{
}

//----------------------------------------------------------------------------
// TOcNameList
//

TOcNameList::TOcNameList()
{
}

TOcNameList::~TOcNameList()
{
  Clear();
}

//
/// Find the format name with the corresponding id
//
TOcFormatName*
TOcNameList::operator [](LPTSTR id)
{
  for (uint i = 0; i < Count(); i++) {
    TOcFormatName* formatName = (*this)[i];
    if (_tcscmp(formatName->GetId(), id) == 0)
      return formatName;
  }

  return 0;
}

//----------------------------------------------------------------------------
// TOcInitInfo
//

TOcInitInfo::TOcInitInfo(IBContainer * container)
:
  How(ihEmbed),
  Where(iwNew),
  Container(container),
  HIcon(0),
  Storage(0)
{
}

TOcInitInfo::TOcInitInfo(TOcInitHow how, TOcInitWhere where, IBContainer * container)
:
  How(how),
  Where(where),
  Container(container),
  HIcon(0),
  Storage(0)
{
}

//----------------------------------------------------------------------------
// Default FrameHost class implementation for compatibility. Forwards events
// to FrameWindow using windows messages
//

//
bool
TOcAppFrameHostMsg::EvOcAppInsMenus(TOcMenuDescr & sharedMenu)
{
  return (bool)ForwardEvent(OC_APPINSMENUS, &sharedMenu);
}

bool
TOcAppFrameHostMsg::EvOcAppMenus(TOcMenuDescr & md)
{
  return (bool)ForwardEvent(OC_APPMENUS, &md);
}

bool
TOcAppFrameHostMsg::EvOcAppProcessMsg(MSG * msg)
{
  return (bool)ForwardEvent(OC_APPPROCESSMSG, msg);
}

bool
TOcAppFrameHostMsg::EvOcAppFrameRect(TRect * rect)
{
  return (bool)ForwardEvent(OC_APPFRAMERECT, rect);
}

bool
TOcAppFrameHostMsg::EvOcAppBorderSpaceReq(TRect * space)
{
  return (bool)ForwardEvent(OC_APPBORDERSPACEREQ, space);
}

bool
TOcAppFrameHostMsg::EvOcAppBorderSpaceSet(TRect * space)
{
  return (bool)ForwardEvent(OC_APPBORDERSPACESET, space);
}

void
TOcAppFrameHostMsg::EvOcAppStatusText(const char * text)
{
  ForwardEvent(OC_APPSTATUSTEXT, text);
}

void
TOcAppFrameHostMsg::EvOcAppRestoreUI()
{
  ForwardEvent(OC_APPRESTOREUI);
}

void
TOcAppFrameHostMsg::EvOcAppDialogHelp(TOcDialogHelp & help)
{
  ForwardEvent(OC_APPDIALOGHELP, help);
}

bool
TOcAppFrameHostMsg::EvOcAppShutdown()
{
  return (bool)ForwardEvent(OC_APPSHUTDOWN);
}

bool
TOcAppFrameHostMsg::EvOcAppGetAccel(TOcGetAccel * acc)
{
  return ForwardEvent(OC_APPGETACCEL, acc);
}

TResult
TOcAppFrameHostMsg::ForwardEvent(int eventId, const void* param)
{
  HWND hWnd = EvOcGetWindow();
  if (::IsWindow(hWnd))
    return ::SendMessage(hWnd, WM_OCEVENT, eventId, (LPARAM)param);

  return 0;
}

TResult
TOcAppFrameHostMsg::ForwardEvent(int eventId, TParam2 param)
{
  HWND hWnd = EvOcGetWindow();
  if (::IsWindow(hWnd))
    return ::SendMessage(hWnd, WM_OCEVENT, eventId, param);

  return 0;
}

} // OCF namespace

//==============================================================================


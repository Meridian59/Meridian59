//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Application class factory and type library implementation
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/appdesc.h>
#include <ocf/ocreg.h>
#include <owl/cmdline.h>
#include <owl/private/strmdefs.h>

namespace ocf {

using namespace owl;
using namespace std;


DIAG_DECLARE_GROUP(OcfDll);

static TLangId ParseLangId(LPCTSTR text);

//----------------------------------------------------------------------------
// TAppDescriptor implementation
//

// Provides access to the AppDescriptor of this component
//
TAppDescriptor* TAppDescriptor::This = 0;

//
//
//
TAppDescriptor::TAppDescriptor(TRegList& regInfo, TComponentFactory callback,
                               owl::tstring& cmdLine, TModule* module,
                               const TRegLink* linkHead)
:
  AppClassId(regInfo), RegInfo(regInfo),
  Module(module), LinkHead(linkHead),
  FactoryCallback(callback),
  LicKeyFactoryCallback(0)
{
  Init(0);
  ProcessCmdLine(cmdLine);
}

//
//
//
TAppDescriptor::TAppDescriptor(TRegList& regInfo, TComponentFactory callback,
                               TLicenseKeyFactory* factory,
                               owl::tstring& cmdLine, TModule* module,
                               const TRegLink* linkHead)
:
  AppClassId(regInfo), RegInfo(regInfo),
  Module(module), LinkHead(linkHead),
  FactoryCallback(callback),
  LicKeyFactoryCallback(factory)
{
  Init(0);
  ProcessCmdLine(cmdLine);
}

//
//
//
void
TAppDescriptor::Init(IMalloc*)
{
  AppProgId= &RegInfo.LookupRef("progid");
  AppName  = &RegInfo.LookupRef("appname");
  AppDoc   = &RegInfo.LookupRef("description");
  HelpFile = &RegInfo.LookupRef("typehelp");

  // Provide direct access for destructors from component main module
  //
  This = this;
  RegClassHdl = RegObjectHdl = ClassCount = LinkGuidCount = 0;
  DebugGuidOffset = LinkGuidOffset = LibGuidOffset = 0;
  ServedList = ActiveObject = 0;
  RefCnt = 0;
  LockCount = 0;
  TypeLib = 0;
  Options = 0;
  ClassList = 0;
  //-------
  Creator = 0;

  // Determine if we are running an EXE or a DLL, initialize OLE if an EXE
  //
  if (::GetModuleHandle(0) == Module->GetHandle()) { // check instance handle for EXE
    Options = amExeModule | amExeMode;
    OLECALL(OleInitialize(0), _T("OleInitialize"));
  }

  // Set the initial usage mode based on the reglist entry and module type
  // NOTE: InProc servers are multiple-user
  //
  if (Options & amExeModule) {
    LPCTSTR usage = RegInfo.Lookup("usage");
    TCHAR su[] = ocrSingleUse;
    if (usage && *usage == *su)
      Options |= amSingleUse;
  }

  // Set the app language to the one in the reglist if provided
  //
  LPCTSTR regLang = RegInfo.Lookup("language");
  AppLang = regLang ? ParseLangId(regLang) : TLocaleString::UserDefaultLangId;

  // Lookup the version, providing a default of 1.0
  //
  Version = RegInfo.Lookup("version", AppLang);
  if (!Version)
    Version = _T("1.0");

  // Assign GUID for debug registration
  // Check if alternate debug registration has been provided and allocated GUID
  //
  if ((Options & amExeModule) && RegInfo.Lookup("debugprogid"))
    DebugGuidOffset = AppClassId.AllocId();

  // Generate array of all automated classes and assign GUIDs
  //
  MergeAutoClasses();
}

//
//
//
TAppDescriptor::~TAppDescriptor()
{
  UnregisterClass();
  delete TypeLib;
  delete[] ClassList;
  This = 0;

  // Following should not normally happen - but just in case
  //
  if (RefCnt > 0)
    ::CoDisconnectObject(this,0);

  // And last, uninitialize OLE
  //
  if (Options & amExeModule)
    ::OleUninitialize();
}

/// Creates a TServedObject helper object which implements an IDispatch
/// implementation on behalf of the 'Application' object.
//
TUnknown*
TAppDescriptor::CreateAutoApp(TObjectDescriptor objDesc, uint32 options,
                              IUnknown* outer)
{
  if (objDesc.Destruct == TObjectDescriptor::Delete) {
    if (!(options & (amServedApp | amAutomation)))
      objDesc.Destruct = TObjectDescriptor::Quiet;
    else if (options & amExeMode)
      objDesc.Destruct = TObjectDescriptor::PostQuit;
  }

  // NOTE: Creator is destroyed via delegation
  //
  Creator = new TServedObjectCreator(*this);

  // NOTE : Object is created with ref. count of '0'
  //
  TUnknown* obj = Creator->CreateObject(objDesc, outer);

  // NOTE: OLE's 'RegisterActiveObject' will do an 'AddRef()'
  //
  RegisterObject(objDesc);

  // NOTE: Object will be RefCnt++ when aggregated or converted to IUnknown*
  //
  return obj;
}

/// Releases the helper object (TServedObject) implementing IDispatch on behalf
/// of the 'Application' object.
//
void
TAppDescriptor::ReleaseAutoApp(TObjectDescriptor app)
{
  TServedObject* obj;
  if (ActiveObject && (obj = FindServed(app)) != 0 && obj == ActiveObject)
    UnregisterObject();
}

/// Creates a TServedObject helper which implements IDispatch on behalf of
/// the class described via the 'objDesc' parameter.
/// \note The returned object initial ref. count is '0'.
//
TUnknown*
TAppDescriptor::CreateAutoObject(TObjectDescriptor objDesc, TServedObject& app,
                                 IUnknown* outer)
{
  return app.Creator.CreateObject(objDesc, outer);
}

/// Creates a TServedObject helper which implements IDispatch on behalf of the
/// class instance passed in via 'obj' and 'objInfo' parameters.
//
TUnknown*
TAppDescriptor::CreateAutoObject(const void* obj, const std::type_info& objInfo,
                                 const void* app, const std::type_info& appInfo,
                                 IUnknown* outer)
{
  TServedObject* autoApp = FindServed(MostDerived(app, appInfo));  //?????????????????????????????
  TServedObjectCreator *creator = autoApp ? &(autoApp->Creator) : Creator;
  if (!creator) {
    // NOTE: Destroyed through delegation
    //
    Creator = new TServedObjectCreator(*this);
    creator = Creator;
  }

  TAutoClass::TAutoClassRef* ref = ClassList;
  for (int i = 0; i < ClassCount; i++, ref++)
    if (objInfo == ref->Class->TypeInfo)
      return creator->CreateObject(TObjectDescriptor(obj, *ref->Class), outer);
  return 0;
}

/// QueryInterface: Hands out IUnknown or IClassFactory
//
HRESULT _IFUNC
TAppDescriptor::QueryInterface(const IID& riid, void** retIface)
{
  if (riid == IID_IUnknown || riid == IID_IClassFactory || riid == IID_IClassFactory2) {
    AddRef();
    *retIface = (IUnknown*)this;
    return HR_NOERROR;
  }
  *retIface = 0;
  return HR_NOINTERFACE;
}

/// Usual IUnknown 'AddRef'
//
ulong _IFUNC
TAppDescriptor::AddRef()
{
  return ++RefCnt;
}

/// Usual IUnknown 'Release'
/// \note This object does *NOT* delete itself
/// \todo !BB Why?? Find out
//
ulong _IFUNC
TAppDescriptor::Release()
{
  return --RefCnt;
}

//
//
//
HRESULT _IFUNC
TAppDescriptor::CreateInstance(IUnknown* outer, const IID& riid,
                               void** ppv)
{

#if   0  // !BB Tweak while playing with Active Controls

  // Display which interface is being asked for
  //
  const char* p = OCFStringFromIID(riid);

  TOcOleCtl* oleCtl = new TOcOleCtl(outer);
  return SUCCEEDED(oleCtl->QueryInterface(riid, ppv));

#else

  *ppv = 0;
  IUnknown* obj;
  if (!FactoryCallback)
    return HR_FAIL;

  try {
    // Test for special condition to force run DLL as an EXE
    //
    if (outer && outer->QueryInterface(IID_NULL, ppv) == HR_NOERROR)
      obj = FactoryCallback(0, Options | amServedApp | amExeMode | amRun, 0);
    else
      obj = FactoryCallback(outer, Options | amServedApp, 0);
    if (!obj)
      return HR_FAIL;
    if (Options & amSingleUse)
      UnregisterClass();

    // Cannot return outer if aggregated
    //
    if (riid == IID_IUnknown) {
      *ppv = obj;
      return HR_OK;
    }
    HRESULT stat = obj->QueryInterface(riid, ppv);
    obj->Release();
    return stat;
  }
  catch (...) {
    // NOTE: Cannot throw any exception through OLE
    //       We'll assume a resource problem;
    //       Is there a better error code to express an exception ??
    //
    return HR_OUTOFMEMORY;
  }

#endif
}

// LockServer [IClassFactory]
//
HRESULT _IFUNC
TAppDescriptor::LockServer(int lock)
{
  if (lock)
    LockCount++;
  else {
    // NOTE: Should we notify app when count reaches 0?
    //
    LockCount--;
  }

  TRACEX(OcfDll, 1, _T("LockServer(") << lock << _T(") Count:") << LockCount);

  return HR_NOERROR;
}

// GetLicInfo [IClassFactory2]
//
HRESULT _IFUNC
TAppDescriptor::GetLicInfo( LICINFO* licInfo)
{
  if(LicKeyFactoryCallback)
    return LicKeyFactoryCallback->GetLicInfo(licInfo);
  return E_UNEXPECTED;
}

// RequestLicKey [IClassFactory2]
//
HRESULT _IFUNC
TAppDescriptor::RequestLicKey(DWORD /*reserved*/,  BSTR* retKey)
{
  if(LicKeyFactoryCallback)
    return LicKeyFactoryCallback->RequestLicKey(retKey);
  return E_NOTIMPL;
}
// CreateInstanceLic [IClassFactory2]
//
HRESULT _IFUNC
TAppDescriptor::CreateInstanceLic(IUnknown* outer,  IUnknown* /*reserved*/,
                                     const IID & iid, BSTR key,
                                     void** retObject )
{
  if(LicKeyFactoryCallback){
    HRESULT hr = LicKeyFactoryCallback->QueryLicKey(key);
    if(SUCCEEDED(hr))
      return CreateInstance(outer, iid, retObject);
    return hr;
  }
  return E_NOTIMPL;
}
/// Returns version as a whole number
//
uint16
TAppDescriptor::GetVersionField(unsigned field)
{
  LPCTSTR cp = Version;
  TCHAR c;
  while (field--) {
    while ((c = *cp++) != _T('.')) {
      if (c < _T('0') || c > _T('9'))
        return 0;    // terminate on null or invalid digit
    }
  }
  uint16 ver = 0;
  while ((c = *cp++) >= _T('0') && c <= _T('9'))
    ver = uint16(ver * 10 + c - _T('0'));
  return ver;
}


/// Register an EXE class object with OLE so that other applications can connect to it
//
void
TAppDescriptor::RegisterClass()
{
  int guidOffset = IsOptionSet(amDebug) ? DebugGuidOffset : 0;
  if (!RegClassHdl) {
    OLECALL(::CoRegisterClassObject(AppClassId[guidOffset], this,
                                    CLSCTX_LOCAL_SERVER,
                                    (Options & amSingleUse) ? REGCLS_SINGLEUSE :
                                                              REGCLS_MULTIPLEUSE,
                                    &RegClassHdl),
                                    _T("Register App class"));
  }
}

/// Invalidates application class registered earlier with 'RegisterClass'
//
void
TAppDescriptor::UnregisterClass()
{
  if (RegClassHdl) {
    OLECALL(::CoRevokeClassObject(RegClassHdl), _T("Unregister App class"));
    RegClassHdl = 0;
  }
}

/// Registers the helper object (TServedObject) of the class described via
/// the 'app' parameter as the active object for its classId.
//
bool
TAppDescriptor::RegisterObject(TObjectDescriptor app)
{
  if (ActiveObject)
    return false;
  TServedObject* obj = FindServed(app);
  if (!obj)
    return false;
  int guidOffset = IsOptionSet(amDebug) ? DebugGuidOffset : 0;

  // Refcnt may be zero, protect against AddRef/Release
  //
  obj->AdjustRefCount(+1);

  // Register object with Ole. OLE 2.02 suggests ACTIVEOBJECT_WEAK, but that
  // does not appear to provide adequate locking.
  //
  IUnknown* unkObj = &(IUnknown&)*(TUnknown*)obj;
//  OLECALL(::RegisterActiveObject(&(IUnknown&)*obj, // BUG_BUG_BUG
  OLECALL(::RegisterActiveObject(unkObj, // BUG_BUG_BUG
                                 AppClassId[guidOffset],
                                 ACTIVEOBJECT_STRONG,
                                 &RegObjectHdl),
          _T("Register App as active"));

  obj->AdjustRefCount(-1);
  ActiveObject = obj;
  return true;
}

/// Invalidates a helper object registers with RegisterObject earlier
//
void
TAppDescriptor::UnregisterObject()
{
  if (RegObjectHdl) {
    // NOTE: Zero before OLE call in case obj. destructor is invoked
    //
    ActiveObject = 0;
    OLECALL(::RevokeActiveObject(RegObjectHdl, 0), _T("Unregister App"));
    RegObjectHdl = 0;
  }
}

/// Returns the index of a 'TAutoClass' instance
//
int
TAppDescriptor::GetClassIndex(TAutoClass* cls)
{
  TAutoClass::TAutoClassRef* ref = ClassList;
  for (int index = 0; index < ClassCount; ref++, index++) {
    if (ref->Class == cls)
      return index;
  }
  return -1;
}

/// Retrieves the GUID of the specified TAutoClass instance.
/// \note Passing '0' as the 'cls' return the GUID of the type Library.
///
/// Returns 'true' if successful, or 'false' otherwise.
//
bool
TAppDescriptor::GetClassId(TAutoClass* cls, GUID& retId)
{
  int offset;
  if (cls) {
    int index = GetClassIndex(cls);
    if (index == -1)
      return false;
    offset = ClassList[index].GuidOffset;
  } else {
    offset = LibGuidOffset;
  }
  retId = AppClassId[offset];
  return true;
}

/// Returns the GUID allocated as the specified index.
/// \note Will allocate a new GUID if 'index' has not been allocated yet.
//
TClassId
TAppDescriptor::GetLinkGuid(int index)
{
  while (index >= LinkGuidCount) {
    int id = AppClassId.AllocId();
    if (!LinkGuidOffset)
      LinkGuidOffset = id;
    LinkGuidCount++;
  }
  return AppClassId[LinkGuidOffset + index];
}

/// Returns the TAutoClass instance as the specified index.
//
TAutoClass*
TAppDescriptor::GetAutoClass(unsigned index)
{
  if (index >= (unsigned)ClassCount)
    return 0;
  return ClassList[index].Class;
}

/// Returns the TAutoClass instance assigned the specified GUID.
//
TAutoClass*
TAppDescriptor::GetAutoClass(const GUID& clsid)
{
  int offset = AppClassId.GetOffset(clsid);
  if (offset != -1) {
    TAutoClass::TAutoClassRef* ref = ClassList;
    for (int count = ClassCount; count--; ref++) {
      if (ref->GuidOffset == offset)
        return ref->Class;
    }
  }
  return 0;
}

/// Get the document template with the given GUID
//
TRegLink*
TAppDescriptor::GetRegLink(const GUID & clsid)
{
  int linkGuidIndex = 0;
  for (const TRegLink* link = LinkHead; link; link=link->GetNext()) {
    TRegList& regList = link->GetRegList();
    if (regList["progid"]) {
      LPCTSTR id = regList["clsid"];
      if (!id) {
        if (clsid == /*(GUID&)*/ GetLinkGuid(linkGuidIndex++))
          return const_cast<TRegLink*>(link);
      }
      else {
        TClassId classId(id);
        if (clsid == (GUID&) classId)
          return const_cast<TRegLink*>(link);
      }
    }
  }

  return 0;
}

/// Returns the 'ITypeLib' interface pointer describing the objects exposed by
/// this instance of 'TAppDescriptor'.
//
ITypeLib*
TAppDescriptor::GetTypeLibrary()
{
  if (!ClassCount)
    return 0;
  if (!TypeLib) {
    TypeLib = new TTypeLibrary(*this, AppLang);
  }
  ((IUnknown&)*TypeLib).AddRef();
  return TypeLib;
}

//----------------------------------------------------------------------------
// TServedObject list management
//

//
//
//
TServedObject*
TAppDescriptor::FindServed(const void * mostDerivedObj)
{
  TServedObject* p = ServedList;
  for (; p; p = p->Next)
    if (p->RootObject == mostDerivedObj)
      break;
  return p;
}

//
//
//
TServedObject*
TAppDescriptor::FindServed(TObjectDescriptor& objDesc)
{
  if (objDesc.Object)
    return FindServed(objDesc.MostDerived());
  for (TServedObject* p = ServedList; p; p = p->Next)
    if (p->Object == 0 && p->Class == objDesc.Class)
      return p;
  return 0;
}

//
//
//
void
TAppDescriptor::AddServed(TServedObject& obj)
{
  if (ServedList)
    ServedList->Prev = &obj.Next;
  obj.Next = ServedList;
  obj.Prev = &ServedList;
  ServedList = &obj;
}

//
//
//
void
TAppDescriptor::RemoveServed(TServedObject& obj)
{
  *obj.Prev = obj.Next;
  if (obj.Next)
    obj.Next->Prev = obj.Prev;
}

//
//
//
void
TAppDescriptor::FlushServed()
{
  while (ServedList)
    delete ServedList;
}


//
//
//
void
TAppDescriptor::DeleteCreator()
{
  if (Creator)
  {
    delete Creator;
    Creator = 0;
  }
}


//
//
//
void
TAppDescriptor::InvalidateObject(const void * obj)
{
  TServedObject* p = FindServed(obj);
  if (p) {
    p->RootObject = 0;
    p->Object = 0;
  }
}

//
//
//
void
TAppDescriptor::ReleaseObject(const void * obj)
{
  TServedObject* p = FindServed(obj);
  if (p) {
    p->RootObject = 0;
    p->Object = 0;
    ((IUnknown*)(void*)p)->Release();   // destructs if no external connections ????????????????
  }
}

//
//
//
ITypeInfo*
TAppDescriptor::CreateITypeInfo(TAutoClass& cls)
{
  TServedObjectCreator& creator = ServedList ? ServedList->Creator
                                             : *new TServedObjectCreator(*this);
  TUnknown* obj = creator.CreateObject(TObjectDescriptor(0, cls));
  IUnknown& unk = (IUnknown&)*obj;
  ITypeInfo* typeInfo = 0;
  unk.QueryInterface(IID_ITypeInfo, (void * *) &typeInfo);
  return typeInfo;
}

//
/// Parse command line for Ole flags.
/// Check for [-/] and remove options that are found. Gets option arguments
/// and performs any immediate option actions
//
void
TAppDescriptor::RegisterServer(TLangId lang, LPCTSTR regFilename)
{
  try {
    tstringstream regStrm;
    tostream* strm = &regStrm;
    TRegItem regDebug[DebugRegCount];

    // Check if registration output to Registry or to user-specified reg file
    //
    _TCHAR guidStr[40];
    bool alwaysReg = IsOptionSet(amRegServer);
#ifdef UNICODE
    wofstream  fileStrm;
#else
    ofstream  fileStrm;
#endif
    if (regFilename) {
      _USES_CONVERSION;
      fileStrm.open(_W2A(regFilename));
      if (!fileStrm.good())
        throw TXRegistry(regFilename, _T("file"));
      strm = &fileStrm;
      fileStrm << _T("REGEDIT\n");  // write registration file header
      alwaysReg = true;
    }
    SetOption(amUnregServer, false);// cancel unregister on reregister

    // Make sure that the app reginfo is in the registry
    //
    bool forceReg = alwaysReg;
    if (AppProgId) {       // don't attempt register app if no progid reg key
      if (!forceReg) {
        tstringstream vstrm;
        ocf::OcRegisterClass(RegInfo, Module, vstrm, lang, _T("\001\002\006"));
        if (ocf::OcRegistryValidate(vstrm) != 0)
          forceReg = true;
      }
      if (forceReg) {
        _TCHAR* debugGuid = 0;
        if (DebugGuidOffset) {                // setup reg item for debug reg
          TClassId debugClsid(AppClassId[DebugGuidOffset]);
          _tcscpy(guidStr, debugClsid);
          debugGuid = guidStr;
          if (ocf::OcSetupDebugReg(RegInfo, regDebug, lang, debugGuid)) {
            ocf::OcRegisterClass(RegInfo, Module, *strm, lang, 0, 0, OcRegNoDebug);
            ocf::OcRegisterClass(RegInfo, Module, *strm, lang, AppDebugFilter,
                                                                    0, regDebug);
          }
          else {
            debugGuid = 0;  // if debug registration fails, use normal
          }
        }
        if (!debugGuid)
          ocf::OcRegisterClass(RegInfo, Module, *strm, lang, 0, 0,
                            (Options & amExeModule) ? 0 : OcRegNotDll);
      }
    }

    // Write templates to registration file as needed
    //
    int linkGuidIndex = 0;
    TRegItem regAppName[2] = {{"appname",{RegInfo["appname"]}}, {0,{0}}};
    for (const TRegLink* link = LinkHead; link; link=link->GetNext()) {
      TRegList& regList = link->GetRegList();
      if (regList["progid"]) {
        _TCHAR guidStr[40];
        int debugStat = (Options & amExeModule) ?
                        ocf::OcSetupDebugReg(regList, regDebug, lang, guidStr) : 0;
        TRegItem regClsid[3] = {{"debugger",{_T("")}}, {"clsid",{guidStr}}, {0,{0}}};
        TRegItem* clsInfo = regClsid;
        if (!debugStat && Options & amExeModule)
          clsInfo++;     // no cancel debugger if no debugprogid
        if (!regList["clsid"]) {      // check if GUID needs to be assigned
          TClassId linkClsid(GetLinkGuid(linkGuidIndex++));
          _tcscpy(guidStr, linkClsid);
        }
        else {
          regClsid[1].Key = 0;  // shorten list to exclude auto-assigned clsid
        }
        if (!alwaysReg) {
          tstringstream checkStrm;
          ocf::OcRegisterClass(regList, Module, checkStrm, lang,
                            _T("\001\002\006"), 0, clsInfo);
          if (ocf::OcRegistryValidate(checkStrm) == 0)
            continue;
        }
        if (debugStat) {
          ocf::OcRegisterClass(regList, Module, *strm, lang, 0, regAppName, regClsid);
          if (debugStat == 1) {   // needs generated guid string
            TClassId linkClsid(GetLinkGuid(linkGuidIndex++));
            _tcscpy(guidStr, linkClsid);
          }
          ocf::OcRegisterClass(regList, Module, *strm, lang, DocDebugFilter,
                                                        regAppName, regDebug);
        }
        else {
          ocf::OcRegisterClass(regList, Module, *strm, lang, 0, regAppName, clsInfo);
        }
        forceReg = true;
      }
    }
    if (forceReg && !regFilename)
      ocf::OcRegistryUpdate(regStrm);
    if (forceReg)
      TRegKey::GetClassesRoot().DeleteKey(_T("OcErr_RegServer"));
  }
  catch (TXBase& ) {
    TRegKey::GetClassesRoot().SetValue(_T("OcErr_RegServer"),REG_SZ, (uint8*)(LPCTSTR)AppClassId, 0);
    if (!(Options & amQuietReg))
      throw;
  }
  return;
}

//
//
//
void
TAppDescriptor::UnregisterServer(TLangId, LPCTSTR)
{
  // Remove application and all type library info from registry
  //
  _TCHAR guidStr[40];
  TRegItem debugItem = {"debugclsid", {guidStr}};
  TRegItem* debugInfo = 0;              // init to no debug registration
  if (AppProgId) {          // don't attempt unregister app if no progid reg key
    if (DebugGuidOffset) {                // setup reg item for debug reg
      TClassId debugClsid(AppClassId[DebugGuidOffset]);
      _tcscpy(guidStr, debugClsid);
      debugInfo = &debugItem;
    }
    ocf::OcUnregisterClass(RegInfo, debugInfo);   // unregister app
    if (LibGuidOffset)
      TRegKey(TRegKey::GetClassesRoot(),_T( "TypeLib")).DeleteKey(AppClassId[LibGuidOffset]);
    if (ClassCount) {
      TRegKey regKey(TRegKey::GetClassesRoot(),_T("Interface"));
      for (int i= 0; i < ClassCount; i++)
        regKey.DeleteKey(AppClassId[ClassList[i].GuidOffset]);
    }
  }

  // Remove templates from registration file as needed
  //
  int linkGuidIndex = 0;
  for (const TRegLink* link = LinkHead; link; link=link->GetNext()) {
    TRegList& regList = link->GetRegList();
    if (regList["progid"]) {
      if (!regList["clsid"]) {      // check if GUID needs to be computed
        TClassId linkClsid(GetLinkGuid(linkGuidIndex++));
        _tcscpy(guidStr, linkClsid);
        TRegItem clsItem = {"clsid", {guidStr}};
        ocf::OcUnregisterClass(regList, &clsItem);
      }
      else
        ocf::OcUnregisterClass(regList);
    }
  }
}

//
//
//
void
TAppDescriptor::SetLangId(TLangId /*prevLang*/, LPCTSTR langIdStr)
{
  AppLang = ParseLangId(langIdStr);
}

//
//
//
void
TAppDescriptor::MakeTypeLib(TLangId lang, LPCTSTR typeLibName)
{
  if (!ClassCount)
    return;

  // Write the typelib file to <typeLibName>
  //
  TCHAR fullPath[_MAX_PATH];
  TCHAR exeDrive[_MAX_DRIVE];
  TCHAR exeDir  [_MAX_DIR];
  TCHAR exeFName[_MAX_FNAME];
  TCHAR exeExt  [_MAX_EXT];
  TCHAR libDrive[_MAX_DRIVE];
  TCHAR libDir  [_MAX_DIR];
  TCHAR libFName[_MAX_FNAME];
  TCHAR libExt  [_MAX_EXT];
  ::GetModuleFileName(*Module, fullPath, COUNTOF(fullPath));
  _tsplitpath(typeLibName? typeLibName:_T(""), libDrive, libDir, libFName, libExt);
  _tsplitpath(fullPath, exeDrive, exeDir, exeFName, exeExt);
  _tmakepath(fullPath,
             libDrive[0] ? libDrive : exeDrive,
             libDir[0]   ? libDir   : exeDir,
             libFName[0] ? libFName : exeFName,
             libExt[0]   ? libExt   : _T("OLB"));

  try {
    WriteTypeLibrary(lang, fullPath);
    TRegKey::GetClassesRoot().DeleteKey(_T("OcErr_Typelib"));
  }
  catch (TXOle&) {
    TRegKey::GetClassesRoot().SetValue(_T("OcErr_Typelib"), REG_SZ, (uint8*)(LPCTSTR)AppClassId, 0);
    if (!(Options & amQuietReg))
      throw;
  }
}

// NOTE: 'TAppDescriptor::WriteTypeLibrary' is implemented in the
//        module TYPELIB.CPP
//

//
//
//
void
TAppDescriptor::ProcessCmdLine(owl::tstring& cmdLine)
{
  struct {
    uint32 Flag;
    const TCHAR* String;
    void   (TAppDescriptor::*Action)(TLangId, LPCTSTR);
  }
  optionTbl[] = {
    { amRegServer,    _T("RegServer"),     &TAppDescriptor::RegisterServer },
    { amUnregServer,  _T("UnregServer"),   &TAppDescriptor::UnregisterServer },
    { amAutomation,   _T("Automation"),    0 },
    { amEmbedding,    _T("Embedding"),     0 },
    { amLangId,       _T("Language"),      &TAppDescriptor::SetLangId },
    { amTypeLib,      _T("TypeLib"),       &TAppDescriptor::MakeTypeLib },
    { amNoRegValidate,_T("NoRegValidate"), 0 },
    { amQuietReg ,    _T("QuietReg"),      0 },
    { amDebug,        _T("Debug"),         0 },
    { amUnregServer,  _T("Unregister"),    &TAppDescriptor::UnregisterServer },
  };
  const int optionTblCount = sizeof(optionTbl)/sizeof(optionTbl[0]);
  TCmdLine cmd(cmdLine.c_str());

  while (cmd.GetTokenKind() != TCmdLine::Done) {
    switch (cmd.GetTokenKind()) {
      default: {
        cmd.NextToken();        // ignore token, not one of ours
        break;
      }
      case TCmdLine::Option: {
        int i;
        for (i = 0; i < optionTblCount; i++) {
          if (_tcsicmp(cmd.GetToken().c_str(), optionTbl[i].String) == 0) {
            Options |= optionTbl[i].Flag;
            cmd.NextToken(true);    // eat token & get next
            if (cmd.GetTokenKind() == TCmdLine::Value) {
              owl::tstring optionArg(cmd.GetToken());
              while (cmd.NextToken(true) == TCmdLine::Value)
                ; // eat token & get next (keep going if more Values are there)
              if (optionTbl[i].Action)
                (this->*optionTbl[i].Action)(AppLang, optionArg.c_str());
            }
            else {
              if (optionTbl[i].Action)
                (this->*optionTbl[i].Action)(AppLang, 0);
            }
            break; // out of for loop
          }
        }
        if (i >= optionTblCount)
          cmd.NextToken();        // ignore token, wasn't one of ours
        break;
      }
    }
  }
  cmdLine = cmd.GetLine();

  // Set single use if this is an automated exe server
  //
  if ((Options & (amAutomation | amExeModule)) == (amAutomation | amExeModule))
    Options |= amSingleUse;

  // Perform normal registry update if no other registry option was specified
  //
  if (!(Options & (amRegServer | amUnregServer | amNoRegValidate)))
    RegisterServer(0);
}

//
// //?????????????????????????? What with UNICODE
//
static TLangId ParseLangId(LPCTSTR text)
{
  static _TCHAR HexChar[] = _T("0123456789ABCDEF0123456789abcdef");
  TLangId lang = 0;
  _TUCHAR c;
  _TCHAR* pc;
  while (text && (c = *text++) != 0) {
    if ((pc = _tcschr(HexChar, c)) == 0)
      return 0;   // illegal character
    lang = TLangId((lang << 4) + (short(pc-HexChar) & 15));//????????????????????
  }
  return lang ? lang : TLocaleString::UserDefaultLangId;
}

//
//
//
void
TAppDescriptor::MergeAutoClasses()
{
  int oldCount = ClassCount;
  ClassCount = TAutoClass::ClassList.CountAutoClasses();
  if (ClassCount) {

    // Allocated GUID for type Library
    //
    if (!LibGuidOffset)
      LibGuidOffset = AppClassId.AllocId();

    TAutoClass::TAutoClassRef* oldList  = ClassList;
    ClassList = new TAutoClass::TAutoClassRef[ClassCount];
    TAutoClass::ClassList.MergeAutoClasses(ClassList);
    TAutoClass::TAutoClassRef* ref = ClassList;
    for (int count = ClassCount; count--; ref++) {
      if (oldCount) {
        ref->GuidOffset = oldList->GuidOffset;
        oldCount--;
        oldList++;
      }
      else {
        ref->GuidOffset = AppClassId.AllocId();
      }
    }
  }
}

} // OCF namespace

//==============================================================================


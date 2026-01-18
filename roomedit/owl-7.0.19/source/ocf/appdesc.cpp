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
#include <algorithm>
#include <utility>
#include <optional>
#include <type_traits>

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
                               const TRegLink* linkHead,
                               owl::uint32 preselectedOptions)
:
  AppClassId(regInfo), RegInfo(regInfo),
  Module(module), LinkHead(linkHead),
  FactoryCallback(callback),
  LicKeyFactoryCallback(0)
{
  Init(0);
  Options |= preselectedOptions;
  ProcessCmdLine(cmdLine);
}

//
//
//
TAppDescriptor::TAppDescriptor(TRegList& regInfo, TComponentFactory callback,
                               TLicenseKeyFactory* factory,
                               owl::tstring& cmdLine, TModule* module,
                               const TRegLink* linkHead,
                               owl::uint32 preselectedOptions)
:
  AppClassId(regInfo), RegInfo(regInfo),
  Module(module), LinkHead(linkHead),
  FactoryCallback(callback),
  LicKeyFactoryCallback(factory)
{
  Init(0);
  Options |= preselectedOptions;
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

void TAppDescriptor::SetLangId(TLangId, LPCTSTR langIdStr)
{
  AppLang = ParseLangId(langIdStr);
}

//
/// Writes application and type library information to the given file.
/// If the given \p regFilename is `nullptr`, then the information is written to the registry.
/// 
/// \exception TXRegistry may be thrown, if access to the registry is denied, unless the option
/// \ref amQuietReg is specified, in which case such exceptions will be suppressed. Note that
/// \ref amQuietReg will also suppress any other exception derived from \ref TXBase.
//
void TAppDescriptor::RegisterServer(TLangId, LPCTSTR regFilename)
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
        ocf::OcRegisterClass(RegInfo, Module, vstrm, AppLang, _T("\001\002\006"));
        if (ocf::OcRegistryValidate(vstrm) != 0)
          forceReg = true;
      }
      if (forceReg) {
        _TCHAR* debugGuid = 0;
        if (DebugGuidOffset) {                // setup reg item for debug reg
          TClassId debugClsid(AppClassId[DebugGuidOffset]);
          _tcscpy(guidStr, debugClsid);
          debugGuid = guidStr;
          if (ocf::OcSetupDebugReg(RegInfo, regDebug, AppLang, debugGuid)) {
            ocf::OcRegisterClass(RegInfo, Module, *strm, AppLang, 0, 0, OcRegNoDebug);
            ocf::OcRegisterClass(RegInfo, Module, *strm, AppLang, AppDebugFilter,
                                                                    0, regDebug);
          }
          else {
            debugGuid = 0;  // if debug registration fails, use normal
          }
        }
        if (!debugGuid)
          ocf::OcRegisterClass(RegInfo, Module, *strm, AppLang, 0, 0,
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
                        ocf::OcSetupDebugReg(regList, regDebug, AppLang, guidStr) : 0;
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
          ocf::OcRegisterClass(regList, Module, checkStrm, AppLang,
                            _T("\001\002\006"), 0, clsInfo);
          if (ocf::OcRegistryValidate(checkStrm) == 0)
            continue;
        }
        if (debugStat) {
          ocf::OcRegisterClass(regList, Module, *strm, AppLang, 0, regAppName, regClsid);
          if (debugStat == 1) {   // needs generated guid string
            TClassId linkClsid(GetLinkGuid(linkGuidIndex++));
            _tcscpy(guidStr, linkClsid);
          }
          ocf::OcRegisterClass(regList, Module, *strm, AppLang, DocDebugFilter,
                                                        regAppName, regDebug);
        }
        else {
          ocf::OcRegisterClass(regList, Module, *strm, AppLang, 0, regAppName, clsInfo);
        }
        forceReg = true;
      }
    }
    if (forceReg && !regFilename)
      ocf::OcRegistryUpdate(regStrm);
    if (forceReg && TRegKey::GetClassesRoot().HasSubkey(_T("OcErr_RegServer")))
      TRegKey::GetClassesRoot().DeleteKey(_T("OcErr_RegServer"));
  }
  catch ([[maybe_unused]] const TXBase& x)
  {
    TRACEX(OcfDll, 0, _T("TAppDescriptor::RegisterServer: Exception: ") << x.why());
    try
    {
      TRegKey::GetClassesRoot().SetValue(_T("OcErr_RegServer"), tstring{AppClassId});
    }
    catch (...)
    {
      WARNX(OcfDll, true, 0, _T("TAppDescriptor::RegisterServer: Failed to set HKEY_CLASSES_ROOT\\OcErr_RegServer."));
    }
    if (!IsOptionSet(amQuietReg))
      throw;
    WARNX(OcfDll, true, 0, _T("TAppDescriptor::RegisterServer: Exception suppressed."));
  }
}

//
/// Removes all application and type library information from the registry.
/// 
/// \exception TXRegistry may be thrown, if access to the registry is denied, unless the option
/// \ref amQuietReg is specified, in which case such exceptions will be suppressed. Note that
/// \ref amQuietReg will also suppress any other exception derived from \ref TXBase.
//
void TAppDescriptor::UnregisterServer(TLangId, LPCTSTR)
{
  try
  {
    const auto ocUnregisterClass = [&](TRegList& regInfo, const TRegItem& overrides)
    {
      return OcUnregisterClass(regInfo, overrides.Key ? const_cast<TRegItem*>(&overrides) : nullptr);
    };

    // Unregister the app, provided an application ProgId is defined.
    //
    if (AppProgId)
    {
      const auto deleteSubkeyIfExists = [](TRegKey& key, LPCTSTR subkey)
      {
        if (key.HasSubkey(subkey))
          key.DeleteKey(subkey);
      };

      ocUnregisterClass(RegInfo, DebugGuidOffset ? TRegItem{"debugclsid", AppClassId[DebugGuidOffset]} : TRegItem{}); // AppClassId[] returns a temporary to which TRegItem refers, so this needs to be performed in one statement!
      if (LibGuidOffset)
        if (auto typeLibKey = TRegKey::GetClassesRoot().GetSubkey(_T("TypeLib")))
          deleteSubkeyIfExists(*typeLibKey, AppClassId[LibGuidOffset]);
      if (ClassCount > 0)
        if (auto interfaceKey = TRegKey::GetClassesRoot().GetSubkey(_T("Interface")))
          for (auto i = 0; i != ClassCount; ++i)
            deleteSubkeyIfExists(*interfaceKey, AppClassId[ClassList[i].GuidOffset]);
    }

    // Remove any defined templates from the registry.
    //
    auto linkGuidIndex = 0;
    for (auto link = LinkHead; link; link = link->GetNext())
    {
      auto& regList = link->GetRegList();
      if (regList["progid"])
        ocUnregisterClass(regList, !regList["clsid"] ? TRegItem{"clsid", GetLinkGuid(linkGuidIndex++)} : TRegItem{}); // GetLinkGuid returns a temporary to which TRegItem refers, so this needs to be performed in one statement!
    }
  }
  catch ([[maybe_unused]] const TXBase& x)
  {
    TRACEX(OcfDll, 0, _T("TAppDescriptor::UnregisterServer: Exception: ") << x.why());
    if (!IsOptionSet(amQuietReg))
      throw;
    WARNX(OcfDll, true, 0, _T("TAppDescriptor::UnregisterServer: Exception suppressed."));
  }
}

//
/// Writes a type library description for this application to the given file.
/// 
/// \exception TXRegistry may be thrown, if access to the registry is denied, unless the option
/// \ref amQuietReg is specified, in which case such exceptions will be suppressed. Note that
/// \ref amQuietReg will also suppress any other exception derived from \ref TXBase.
//
void TAppDescriptor::MakeTypeLib(TLangId, LPCTSTR typeLibName)
{
  if (ClassCount <= 0) return;
  try
  {
    TCHAR fullPath[_MAX_PATH];
    TCHAR exeDrive[_MAX_DRIVE];
    TCHAR exeDir[_MAX_DIR];
    TCHAR exeFName[_MAX_FNAME];
    TCHAR exeExt[_MAX_EXT];
    TCHAR libDrive[_MAX_DRIVE];
    TCHAR libDir[_MAX_DIR];
    TCHAR libFName[_MAX_FNAME];
    TCHAR libExt[_MAX_EXT];
    ::GetModuleFileName(*Module, fullPath, COUNTOF(fullPath));
    _tsplitpath(typeLibName ? typeLibName : _T(""), libDrive, libDir, libFName, libExt);
    _tsplitpath(fullPath, exeDrive, exeDir, exeFName, exeExt);
    _tmakepath(fullPath,
      libDrive[0] ? libDrive : exeDrive,
      libDir[0] ? libDir : exeDir,
      libFName[0] ? libFName : exeFName,
      libExt[0] ? libExt : _T("OLB"));
    WriteTypeLibrary(AppLang, fullPath); // Note: Implemented in "typelib.cpp".
    if (TRegKey::GetClassesRoot().HasSubkey(_T("OcErr_Typelib")))
      TRegKey::GetClassesRoot().DeleteKey(_T("OcErr_Typelib"));
  }
  catch ([[maybe_unused]] const TXBase& x)
  {
    TRACEX(OcfDll, 0, _T("TAppDescriptor::MakeTypeLib: Exception: ") << x.why());
    try
    {
      TRegKey::GetClassesRoot().SetValue(_T("OcErr_Typelib"), tstring{AppClassId});
    }
    catch (...)
    {
      WARNX(OcfDll, true, 0, _T("TAppDescriptor::MakeTypeLib: Failed to set HKEY_CLASSES_ROOT\\OcErr_Typelib."));
    }
    if (!IsOptionSet(amQuietReg))
      throw;
    WARNX(OcfDll, true, 0, _T("TAppDescriptor::MakeTypeLib: Exception suppressed."));
  }
}

//
/// Processes known options on the command line.
///
/// Registry options (one allowed):
/// 
/// - RegServer [&lt;regfile&gt;]
/// - UnregServer/Unregister
/// - NoRegValidate
/// 
/// Auxiliary options:
/// 
/// - Automation
/// - Debug
/// - Embedding
/// - Language &lt;langid&gt;
/// - QuietReg
/// - TypeLib [&lt;name&gt;]
/// 
/// On return, the given \p cmdLine will have the processed options removed.
/// 
/// \note If no registry option (\p RegServer, \p UnregServer or \p NoRegValidate) is specified on
/// the command line, then validation of the registry entries will be performed. If validation
/// fails, then the registry is updated as if registration was specified. Pass \p NoRegValidate to
/// suppress this behaviour.
/// 
/// \exception TXRegistry may be thrown, if access to the registry is denied, unless the option
/// \p QuietReg is specified, in which case such exceptions will be suppressed. Note that
/// \p QuietReg will also suppress any other exception derived from \ref TXBase.
//
void TAppDescriptor::ProcessCmdLine(owl::tstring& cmdLine)
{
  struct
  {
    TOcAppMode Flag;
    LPCTSTR Option;
    bool IsAction;
  }
  const optionTbl[] =
  {
    // Registry options (one allowed):
    //
    {amRegServer, _T("RegServer"), true},
    {amUnregServer, _T("UnregServer"), true},
    {amUnregServer, _T("Unregister"), true},
    {amNoRegValidate, _T("NoRegValidate"), false},

    // Auxiliary options:
    //
    {amAutomation, _T("Automation"), false},
    {amDebug, _T("Debug"), false},
    {amEmbedding, _T("Embedding"), false},
    {amLangId, _T("Language"), false},
    {amQuietReg, _T("QuietReg"), false},
    {amTypeLib, _T("TypeLib"), true},
  };

  const auto isRegistryOptionSet = [](uint32 flags)
  { 
    return (flags & (amRegServer | amUnregServer | amNoRegValidate)) != 0; 
  };

  // Parse the command line.
  //
  using TAction = pair<TOcAppMode, tstring>; // Option flag and argument.
  auto actions = vector<TAction>{};
  auto cl = TCmdLine{cmdLine};
  while (cl.GetTokenKind() != TCmdLine::Done)
    switch (cl.GetTokenKind())
    {
    case TCmdLine::Option:
      {
        const auto option = cl.GetToken();
        const auto i = find_if(begin(optionTbl), end(optionTbl), [&option](const auto& entry)
          { return _tcsicmp(entry.Option, option.c_str()) == 0; });
        if (i == end(optionTbl))
        {
          WARNX(OcfDll, true, 1, _T("TAppDescriptor::ProcessCmdLine: Unknown option: ") << option);
          cl.NextToken(false); // Skip this option, but leave it on the command line.
          break;
        }

        // Record this option.
        //
        const auto hasMultipleRegistryOptions = (isRegistryOptionSet(Options) && isRegistryOptionSet(i->Flag));
        WARNX(OcfDll, hasMultipleRegistryOptions, 0, _T("TAppDescriptor::ProcessCmdLine: Multiple registry options are specified."));
        SetOption(i->Flag, true);
        cl.NextToken(true); // Remove the option from the command line.
        const auto arg = (cl.GetTokenKind() == TCmdLine::Value) ? cl.GetToken() : tstring{};
        if (i->IsAction)
          actions.push_back(make_pair(i->Flag, arg));
        else if (i->Flag == amLangId && !arg.empty())
          SetLangId(AppLang, arg.c_str());
        while (cl.GetTokenKind() == TCmdLine::Value)
          cl.NextToken(true); // Remove all arguments following the option.
        break;
      }
    default:
      cl.NextToken(); // Skip.
      break;
    }

  // Set single use, if this is an automated exe server.
  //
  if (IsOptionSet(amAutomation) && IsOptionSet(amExeModule))
    SetOption(amSingleUse, true);

  // Perform specified actions.
  //
  for (const auto& a : actions)
  {
    const auto arg = a.second.empty() ? nullptr : a.second.c_str();
    switch (a.first)
    {
    case amRegServer: RegisterServer(AppLang, arg); break;
    case amUnregServer: UnregisterServer(); break;
    case amTypeLib: MakeTypeLib(AppLang, arg); break;
    default: CHECKX(false, _T("TAppDescriptor::ProcessCmdLine: Unexpected action flag: ") + to_tstring(a.first));
    }
  }
  cmdLine = cl.GetLine(); // Update the command line with processed options removed.

  // Perform registry validation, if no registry option was specified.
  //
  // Note: RegisterServer executes in validation mode when amRegServer is not set. If validation
  // fails, the registry will be updated, as if registration was specified. See the implementation
  // for details. The registry option NoRegValidate can be specified on the command line to
  // suppress this validation call.
  //
  if (!isRegistryOptionSet(Options))
    RegisterServer(0); // Validate registry entries.
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


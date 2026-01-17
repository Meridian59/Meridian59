//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
///   OLE Registration implementation
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/ocreg.h>
#include <ocf/appdesc.h>
#include <owl/pointer.h>
#include <owl/module.h>
#include <shellapi.h>
#include <string.h>
#include <stdlib.h>

namespace ocf {

DIAG_DEFINE_GROUP_INIT(OWL_INI, OcfDll, 1, 0);
DIAG_DECLARE_GROUP(OcfRefCount);

using namespace owl;

#if defined(BI_APP_DLL)
  extern TRegistrar* DllRegistrar;
#endif

//----------------------------------------------------------------------------
// Internal registration template table and registration item key table
//

//
/// Registration template table, ordering determines sequence of registration.
///  Also, these are all based on HKEY_CLASSES_ROOT
/// \note the parameter table is dependent on template indices in this table
//
static const _TCHAR* OcRegTemplates[] = {
/* 001 */ _T("CLSID\\<clsid>\\<serverctx>Server32 = <debugger> <path> <cmdline><extraopt>"),
/* 002 */ _T("CLSID\\<clsid>\\ProgID = <progid>"),
/* 003 */ _T("CLSID\\<clsid> = <description>"),
/* 004 */ _T("CLSID\\<clsid>\\DefaultIcon = <path>,<iconindex>"),
/* 005 */ _T("CLSID\\<clsid>\\InprocHandler32 = <handler>"),
/* 006 */ _T("<progid>\\CLSID = <clsid>"),
/* 007 */ _T("<progid> = <description>"),

/* 010 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\0 = <verb0>"),
/* 011 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\1 = <verb1>"),
/* 012 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\2 = <verb2>"),
/* 013 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\3 = <verb3>"),
/* 014 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\4 = <verb4>"),
/* 015 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\5 = <verb5>"),
/* 016 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\6 = <verb6>"),
/* 017 */ _T("<progid>\\protocol\\StdFileEditing\\verb\\7 = <verb7>"),

/* 020 */ _T("CLSID\\<clsid>\\Verb\\0 = <verb0>,<verb0opt>"),
/* 021 */ _T("CLSID\\<clsid>\\Verb\\1 = <verb1>,<verb1opt>"),
/* 022 */ _T("CLSID\\<clsid>\\Verb\\2 = <verb2>,<verb2opt>"),
/* 023 */ _T("CLSID\\<clsid>\\Verb\\3 = <verb3>,<verb3opt>"),
/* 024 */ _T("CLSID\\<clsid>\\Verb\\4 = <verb4>,<verb4opt>"),
/* 025 */ _T("CLSID\\<clsid>\\Verb\\5 = <verb5>,<verb5opt>"),
/* 026 */ _T("CLSID\\<clsid>\\Verb\\6 = <verb6>,<verb6opt>"),
/* 027 */ _T("CLSID\\<clsid>\\Verb\\7 = <verb7>,<verb7opt>"),

/* 030 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\0 = <format0>"),
/* 031 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\1 = <format1>"),
/* 032 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\2 = <format2>"),
/* 033 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\3 = <format3>"),
/* 034 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\4 = <format4>"),
/* 035 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\5 = <format5>"),
/* 036 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\6 = <format6>"),
/* 037 */ _T("CLSID\\<clsid>\\DataFormats\\GetSet\\7 = <format7>"),

/* 040 */ _T("CLSID\\<clsid>\\MiscStatus = <aspectall>"),
/* 041 */ _T("CLSID\\<clsid>\\MiscStatus\\1 = <aspectcontent>"),
/* 042 */ _T("CLSID\\<clsid>\\MiscStatus\\2 = <aspectthumbnail>"),
/* 043 */ _T("CLSID\\<clsid>\\MiscStatus\\4 = <aspecticon>"),
/* 044 */ _T("CLSID\\<clsid>\\MiscStatus\\8 = <aspectdocprint>"),
/* 045 */ _T("CLSID\\<clsid>\\AuxUserType\\2 = <menuname>"),
/* 046 */ _T("CLSID\\<clsid>\\AuxUserType\\3 = <appname>"),
/* 047 */ _T("CLSID\\<clsid>\\Insertable"),

/* 050 */ _T("<permid> = <permname>"),
/* 051 */ _T("<permid>\\CLSID = <clsid>"),
/* 052 */ _T("<permid>\\CurVer = <progid>"),
/* 053 */ _T("CLSID\\<clsid>\\VersionIndependentProgID = <permid>"),

/* 054 */ _T("CLSID\\<clsid>\\Version = <version>"),
/* 055 */ _T("CLSID\\<clsid>\\DataFormats\\DefaultFile = <filefmt>"),
/* 056 */ _T("CLSID\\<clsid>\\Ole1Class = <progid>"),        /// is this really needed?
/* 057 */ _T(".<extension> = <progid>"),

/* 060 */ _T("<progid>\\Shell\\Print\\Command = <path> \"%1\""),
/* 061 */ _T("<progid>\\Shell\\Open\\Command = <path> \"%1\""),
/* 062 */ _T("<progid>\\protocol\\StdFileEditing\\server = <debugger> <path><extraopt>"),
/* 063 */ _T("<progid>\\Insertable"),

// Following entries are specific to controls
//

/* 064 */ _T("CLSID\\<clsid>\\Control"),
/* 065 */ _T("CLSID\\<clsid>\\ToolBoxBitmap32 = <path>,<toolbmpindex>"),
          0,
};

//
/// Registration parameter table, ordering is arbitrary, parameters lowercase
//
static const TRegParamList::TEntry OcRegParams[] = {
  {_T("clsid"),             0, _T("")               },// GUID standard string form
  {_T("progid"),            0, _T("\001\002\003\005\006\007")},// A unique name
  {_T("insertable"),        0, _T("\046\047\062\063")},// Defined to publish server
  {_T("control"),           0, _T("\064\065")       },// Defined to publish control
  {_T("usage"),  ocrSingleUse, _T("")               },// Class factory registration
  {_T("description"),       0, _T("")               },// Human readable, 40 chars max
  {_T("handler"), _T("ole32.dll"), _T("\005")        },// Inproc handler
  {_T("serverctx"),         0, _T("\001")           },// "Local"(EXE) or "Inproc"(DLL)
  {_T("path"),              0, _T("")               },// OC defaults to load path
  {_T("cmdline"),      _T(""), _T("")               },// /Automation if app class
  {_T("debugger"),     _T(""), _T("")               },// Debugger to invoke server
  {_T("permid"),            0, _T("\050\051\053")   },// Version independent clsid
  {_T("permname"),          0, _T("\052")           },// Version independent progid
  {_T("iconindex"),   _T("0"), _T("\004")           },// Zero-based resource icon
  {_T("toolbmpindex"),_T("1"), _T("\065")           },// Index of resource bitmap for palette
  {_T("menuname"),     _T(""), _T("\045")           },// 15 characters maximum
  {_T("appname"),           0, _T("")               },// AppName, AuxUserType/3
  {_T("verb0"),             0, _T("\010\020")       },// Name of primary verb
  {_T("verb1"),             0, _T("\011\021")       },
  {_T("verb2"),             0, _T("\012\022")       },
  {_T("verb3"),             0, _T("\013\023")       },
  {_T("verb4"),             0, _T("\014\024")       },
  {_T("verb5"),             0, _T("\015\025")       },
  {_T("verb6"),             0, _T("\016\026")       },
  {_T("verb7"),             0, _T("\017\027")       },
  {_T("verb8"),             0, _T("\018\028")       },
  {_T("verb0opt"),  _T("0,2"), _T("\020")           },// From REGVERBOPT macro
  {_T("verb1opt"),  _T("0,2"), _T("\021")           },
  {_T("verb2opt"),  _T("0,2"), _T("\022")           },
  {_T("verb3opt"),  _T("0,2"), _T("\023")           },
  {_T("verb4opt"),  _T("0,2"), _T("\024")           },
  {_T("verb5opt"),  _T("0,2"), _T("\025")           },
  {_T("verb6opt"),  _T("0,2"), _T("\026")           },
  {_T("verb7opt"),  _T("0,2"), _T("\027")           },
  {_T("verb8opt"),  _T("0,2"), _T("\028")           },
  {_T("format0"),           0, _T("\030")           },// From REGFORMAT macro
  {_T("format1"),           0, _T("\031")           },
  {_T("format2"),           0, _T("\032")           },
  {_T("format3"),           0, _T("\033")           },
  {_T("format4"),           0, _T("\034")           },
  {_T("format5"),           0, _T("\035")           },
  {_T("format6"),           0, _T("\036")           },
  {_T("format7"),           0, _T("\037")           },
  {_T("format8"),           0, _T("\038")           },
  {_T("filefmt"),           0, _T("\055")           },// Default file format name
  {_T("aspectall"),   _T("0"), _T("\040")           },// Option flags for all aspects
  {_T("aspectcontent"),     0, _T("\041")           },// Option flags for content view
  {_T("aspectthumbnail"),   0, _T("\042")           },// Option flags thumbnail view
  {_T("aspecticon"),        0, _T("\043")           },// Option flags for icon view
  {_T("aspectdocprint"),    0, _T("\044")           },// Option flags docprint view
  {_T("extension"),         0, _T("\057\060\061")   },// Extension for shell loading
  {_T("version"),           0, _T("\054")           },// App/typelib version string
  {_T("helpdir"),           0, _T("")               },// Help directory for typehelp
  {_T("typehelp"),          0, _T("")               },// Help file for type library
  {_T("language"),          0, _T("")               },// Set internally from func arg
  {_T("docflags"),    _T("0"), _T("")               },
  {_T("directory"),         0, _T("")               },
  {_T("docfilter"),         0, _T("")               },
  {_T("debugclsid"),        0, _T("")/*INTERNAL GEN*/},// Internal,NOT USER SPECIFIED
  {_T("extraopt"),     _T(""), _T("")/*INTERNAL USE*/},// Cmdline debug/non-debug flag
  {_T("debugprogid"),       0, _T("")               },// Define to force debug reg
  {_T("debugdesc"),         0, _T("")               },// Different string for debug
  {0,                       0,    0                 },// End
};

//
/// Generate OLE registration file image to an output stream
///
/// Process a given regInfo table to produce an ascii registration list out to
/// an ostream. Arguments are:
/// \param regInfo  - main registration list to work from
/// \param  module   - hinstance of program module for 'path' & 'serverctx' entries
/// \param  out      - output stream to receive reg entry lines
/// \param  lang     - language id for 'language' entry & string translation
/// \param  filter   - optional template restricting filter
/// \param  defaults - optional reg entries processed before main list
/// \param  overrides- optional reg entries processed after main list
//
long OcRegisterClass(TRegList& regInfo, TModule* module, tostream& out,
                     TLangId lang, LPCTSTR filter, TRegItem* defaults,
                     TRegItem* overrides)
{
  // Use OCF global registration templates & parameters
  //
  TRegSymbolTable symbolTable(TRegKey::GetClassesRoot(), OcRegTemplates, OcRegParams);

  // Initialize the symbol table
  //
  symbolTable.Init(filter);

  // Check for language defaulted to system or user configuration &
  // assign language parameter based on TLocaleString's setting
  //
  if (lang == LangSysDefault)
    lang = TLocaleString::SystemDefaultLangId;
  else if (lang == LangUserDefault)
    lang = TLocaleString::UserDefaultLangId;

  int langIdx = symbolTable.Params.Find(_T("language"));
  _TCHAR langBuf[10];
  wsprintf(langBuf, _T("%X"), lang);
  symbolTable.Params.Value(langIdx) = langBuf;

  // Assign path parameter initially to module load path of the given HINSTANCE
  //
  const int pathSize = 512;
  TAPointer<_TCHAR> path(new _TCHAR[pathSize]);
  if(!module)
    module = &GetGlobalModule();
  module->GetModuleFileName(path, pathSize);
  symbolTable.Params.Value(symbolTable.Params.Find(_T("path"))) = path;

  // Assign server context based on whether it is an EXE or a DLL
  //
  // NOTE: Assumes that this code is linked into server's module
  //
  {
    int svrIdx = symbolTable.Params.Find(_T("serverctx"));
    if (svrIdx >= 0)
    {
      if (::GetModuleHandle(0) == module->GetHandle()) // Check instance handle for 32bit EXE
        symbolTable.Params.Value(svrIdx) = _T("Local");
      else
        symbolTable.Params.Value(svrIdx) = _T("Inproc");
    }
  }

  // Build params from the 3 lists provided in this order:
  //
  //  0: defaults list (defaults)
  //  1: user list of parameters and custom keys (regList)
  //  2: and overrides list (overrides)
  //
  symbolTable.UpdateParams(lang, defaults);
  symbolTable.UpdateParams(lang, regInfo.Items);
  if (!defaults) { //SL: fix for progId without description, Jul-2009
  bool hasProgId=false;
  bool hasDescription=false;
  TRegItem* regItem;
  for (regItem = regInfo.Items; regItem->Key != 0 && !hasProgId; regItem++) {
    if (strcmp(regItem->Key, "progid") == 0)
      hasProgId=true;
  }
  for (regItem = regInfo.Items; regItem->Key != 0 && !hasDescription; regItem++) {
    if (strcmp(regItem->Key, "description") == 0)
      hasDescription=true;
  }
//  if (regInfo.Lookup("progid") && !regInfo.Lookup("description"))
  if (hasProgId && !hasDescription)
  {
    TRegItem auxAppReg_list[] = {
      { "description", {_T("OCF default description")}},
      {0,{0}} };
    symbolTable.UpdateParams(lang, auxAppReg_list);
  }
  }
  symbolTable.UpdateParams(lang, overrides);

  symbolTable.StreamOut(regInfo.Items, out);

  // Decode & return document template flags
  //
  int i = symbolTable.Params.Find(_T("docflags"));
  if (i >= 0)
    return _ttol(symbolTable.Params.Value(i));
  return 0;
}

//
/// Unregister application or class from registration database
///
/// These params are the only ones needed to unregister an OLE program or class
//
int OcUnregisterClass(TRegList& regInfo, TRegItem* overrides)
{
  static const TRegistry::TUnregParams unregParams[] = {
    { _T('.'),_T("extension"),   &TRegKey::GetClassesRoot() },
    {     0,  _T("debugclsid"),  &TRegKey::GetClassesRootClsid() },
    {      0,  _T("debugprogid"), &TRegKey::GetClassesRoot() },
    {      0,  _T("clsid"),       &TRegKey::GetClassesRootClsid() },
    {      0,  _T("progid"),      &TRegKey::GetClassesRoot() },
    {      0,  _T("permid"),      &TRegKey::GetClassesRoot() },
    {      0,      0,              0 },
  };
  return TRegistry::Unregister(regInfo, unregParams, overrides);
}

//----------------------------------------------------------------------------
// Separate debugging registration for application and documents
// Overrides non-debug values with values from debug keys
//

//
//
//
TRegItem OcRegNoDebug[] = { {"debugger", {_T("")}}, {0,{0}} };
TRegItem OcRegNotDll[]  = { {"cmdline", {_T("")}}, {"debugger", {_T("")}}, {0,{0}} };

//
// Filters limit the templates used when setting up debug registration
//
_TCHAR AppDebugFilter[] = _T("\001\002\003\005\006\007");
_TCHAR DocDebugFilter[] = _T("\001\002\003\005\006\007")\
                        _T("\020\021\022\023\024\025\026\027")\
                        _T("\030\031\032\033\034\035\036\037")\
                        _T("\040\041\042\043\044\045\046\047")\
                        _T("\054\055\062\063");

//
//
//
struct TDebugRegKey { enum { progid,        clsid,   description,   extraopt};};
const _TCHAR* OcReplaceKeys[] =  { _T("progid"), _T("clsid"), _T("description"), _T("extraopt")};
const _TCHAR* OcDebugKeys[] = {_T("debugprogid"), _T("debugclsid"), _T("debugdesc"), _T("debugopt")};

//
/// Walk thru a reginfo list using debug entries to create a parallel non-debug
/// set for alternate, debug registration.
/// Return:
/// -   0 - No debug registration needed / available
/// -   1 - Debug registration using passed classid
/// -  -1 - Debug registration using classid found in reginfo
//
int OcSetupDebugReg(TRegList& regInfo, TRegItem* regDebug, TLangId lang, LPTSTR clsid)
{
  LPCTSTR* oldKey = OcReplaceKeys;
  LPCTSTR* newKey = OcDebugKeys;

  // Walk thru the special keys with debug alternates
  //
  int stat = -1;         // Initialize for regInfo-supplied clsid
  for (int i = 0; i < COUNTOF(OcReplaceKeys); i++,oldKey++,newKey++,regDebug++) {
    _USES_CONVERSION;
    LPCTSTR value = regInfo.Lookup(_W2A(*newKey), lang);

    // If not value found for a given key, supply a default if possible, else
    // throw an exception.
    //
    if (!value) {
      switch (i) {
        case TDebugRegKey::progid:
          return 0;                   // No debug registration if progid missing
        case TDebugRegKey::extraopt:
          value = _T(" /Debug");      // Default to "/Debug" on command line
          break;
        case TDebugRegKey::clsid:
          if (clsid) {
            value = clsid;
            stat = 1;                 // Flag use of supplied clsid
            break;
          }       // else fall through to throw exception--clsid is required
        case TDebugRegKey::description:
          throw TXRegistry{_T("ocf::OcSetupDebugReg: incomplete debug registration"), *newKey};
      }
    }
    regDebug->Key   = _W2A(*oldKey);       // Write normal key + debug value to
    regDebug->Value = value;           // output reg list
  }
  regDebug->Key = 0;                   // 0-terminate reg list
  return stat;
}


//----------------------------------------------------------------------------
// TRegistrar - Application registration manager interface class
//

//
//
//
TRegistrar* TRegistrar::RegistrarList = 0;

//
//
//
TRegistrar::TRegistrar(TRegList& regInfo, TComponentFactory callback,
                       owl::tstring& cmdLine, TModule* module)
:
  AppDesc(*new TAppDescriptor(regInfo, callback, cmdLine, module))
{
  Next = RegistrarList;
  RegistrarList = this;
  TRACEX(OcfRefCount, 1, "TRegistrar() @" << (void*)this);
}

//
//
//
TRegistrar::TRegistrar(TAppDescriptor& appDesc)
:
  AppDesc(appDesc)
{
  Next = RegistrarList;
  RegistrarList = this;
}

//
//
//
TRegistrar::~TRegistrar()
{
  // Since TRegistrars are destroyed when the module is shutdown
  // we do not need to detach each registrar from the linked list
  //
  delete &AppDesc;
}

//
//
//
TRegistrar* TRegistrar::GetNext(TRegistrar* reg)
{
  return reg ? reg->Next : RegistrarList;
}

//
//
//
void * TRegistrar::GetFactory(const GUID& clsid, const GUID & iid)
{
  void * retObj = 0;
  if (clsid != AppDesc.AppClassId)
    return 0;
  ((IUnknown&)AppDesc).QueryInterface(iid, &retObj);
  return retObj;   // QueryInterface sets to null if fails
}

//
//
//
bool TRegistrar::CanUnload()
{
  return static_cast<bool>(!AppDesc.IsBound());
}

//
//
//
int TRegistrar::Run()
{
  if (!IsOptionSet(amExeModule))
    return 0;    // inproc server waits until class factory created and called
  TComponentFactory factory = AppDesc.GetFactory();
  if (!factory)
    return 1;
  if (AppDesc.IsAutomated() && IsOptionSet(amAutomation))
    RegisterAppClass();
  IUnknown* ifc = (factory)(0, GetOptions() | amRun, 0);  // create app and run it

  // App is now running its message loop, factory may get called again by OLE
  //
  ifc = (factory)(ifc, GetOptions() | amShutdown, 0);  // EXE finished, destroy it
  if (ifc && !(GetOptions() & amServedApp))
    ifc->Release();  // we own the reference count, else container released it
  return 0;
}

//
//
//
void TRegistrar::Shutdown(IUnknown* releasedObj, uint32 options)
{
  if (!AppDesc.GetFactory())
    return;
  releasedObj = AppDesc.GetFactory()(releasedObj, options|amShutdown, 0);
  if (releasedObj)
    releasedObj->Release();
}

} // OCF namespace

//==============================================================================

using namespace ocf;

//----------------------------------------------------------------------------
// DLL server entry points
//

//
/// Entry point for complete registration management via command line
/// Don't let any exceptions blow back to OLE.
//
#  if defined(BI_COMP_MSC)
STDAPI
#else
STDAPI __declspec(dllexport)
#endif
DllRegisterCommand(LPCTSTR cmdLine)
{
  try {
    bool isDebug = false;
    TRegistrar* registrar = 0;

    // Need to set up typelibrary state information in case multiple components
    //
    while ((registrar = TRegistrar::GetNext(registrar)) != 0) {
      owl::tstring cmd(cmdLine);
      registrar->ProcessCmdLine(cmd);
      if (registrar->IsOptionSet(amDebug))
        isDebug = true;
    }
    if (isDebug)
      OWL_BREAK;
    return HR_OK;
  }
  catch (...) {
    return HR_FAIL;
  }
}

//
/// OLE 2.0 entry point for obtaining a class factory for a particular CLSID
/// Don't let any exceptions blow back to OLE.
//
STDAPI DllGetClassObject(const GUID& clsid, const GUID& iid, void** retObj)
{
  try {
    TRegistrar* registrar = 0;
    while ((registrar = TRegistrar::GetNext(registrar)) != 0) {
      *retObj = registrar->GetFactory(clsid, iid);
      if (*retObj)
        return HR_OK;
    }
    return HR_FAIL;
  }
  catch (...) {
    return HR_FAIL;
  }
}

//
/// OLE 2.0 entry point for checking if DLL has no clients and can be unloaded
//
STDAPI DllCanUnloadNow()
{
  TRegistrar* registrar = 0;
  while ((registrar = TRegistrar::GetNext(registrar)) != 0) {
    if (!registrar->CanUnload()) {
      TRACEX(OcfDll, 1, "DllCanUnloadNow returning HR_FALSE");
      return HR_FALSE;
    }
  }
  TRACEX(OcfDll, 1, "DllCanUnloadNow returning HR_OK");
  return HR_OK;
}

//
/// OLE 2.0 entry point for registering DLL, no locale info passed
//
STDAPI DllRegisterServer()
{
  return DllRegisterCommand(_T("/RegServer"));
}

//
/// OLE 2.0 entry point for unregistering DLL
//
STDAPI DllUnregisterServer()
{
  return DllRegisterCommand(_T("/UnregServer"));
}

#if defined(BI_COMP_MSC)
extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;

//Jogy - this is not correct!!
#if !defined(OWL_DLLNAME)
#  if defined (_UNICODE)
#     if defined(__TRACE) || defined(__WARN)
#     define OWL_DLLNAME "owl620vud.dll"
#     else
#     define OWL_DLLNAME "owl620vu.dll"
#     endif
#  else
#  if defined(__TRACE) || defined(__WARN)
#    define OWL_DLLNAME "owl620vd.dll"
#  else
#    define OWL_DLLNAME "owl620v.dll"
#  endif
#endif
#endif

extern "C"
BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
  if (dwReason == DLL_PROCESS_ATTACH){
    // Prevent the OWL DLL from being unloaded prematurely
    ::LoadLibraryA(OWL_DLLNAME);
  }
  else if (dwReason == DLL_PROCESS_DETACH){
    // Now it's OK for the OWL DLLs to be unloaded (see LoadLibrary above)
    ::FreeLibrary(GetModuleHandleA(OWL_DLLNAME));
  }
  return TRUE;    // ok
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, uint32 reason, LPVOID)
#else
int WINAPI DllEntryPoint(HINSTANCE hInstance, uint32 reason, LPVOID)
#endif
{
  //TRACEX(OwlMain, 0, "DllEntryPoint(" << hInstance << ", " << reason << ", " <<
  //       ") called");

  switch (reason) {
    case DLL_PROCESS_ATTACH:

      // If you provide your own version of DLL EntryPoint, make sure
      // you call OwlInitUserDLL to allow OWL to initialize the global module.
      // Second parameter - command line, if it = 0 std::string will crash //Y.B.
      return OWLInitUserDLL(hInstance, _T("")) ? 1 : 0;
  }
  return 1;
}




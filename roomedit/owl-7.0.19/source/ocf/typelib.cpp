//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// TServedObject implementation and OLE Type library generation
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/appdesc.h>
#include <ocf/occtrl.h>
#include <owl/filename.h>

#define OLE_TYPELIB_FILE L"stdole32.tlb"

namespace ocf {

using namespace owl;

//____________________________________________________________________________
//
// TypeLibrary generation
//____________________________________________________________________________

/// local class to hold ICreateTypeLib interface pointers for each class

struct TOleCreateInfo {
  int FuncCount;
  int VarCount;
  int ImplCount;
  TYPEKIND TypeKind;
  ITypeInfo*       OurTypeInfo;  // our type info implementation
  ITypeInfo*       OleTypeInfo;  // type info obtained from ICreatTypeInfo
  ICreateTypeInfo* CreateInfo;   // interface obtained from ICreateTypeLib
  TOleCreateInfo() : OurTypeInfo(0), OleTypeInfo(0), CreateInfo(0) {}
 ~TOleCreateInfo();
};

TOleCreateInfo::~TOleCreateInfo()
{
  if (OurTypeInfo)
    OurTypeInfo->Release();
  if (OleTypeInfo)
    OleTypeInfo->Release();
  if (CreateInfo)
    CreateInfo->Release();
}

struct TOleCreateList {
  int             Count;
  ITypeLib*       TypeLib;
  LPCTSTR          FileName;
  TOleCreateInfo* List;
  ITypeLib*       OleLib;
  ITypeComp*      OleComp;
  ITypeInfo*      OleInfo;
  ICreateTypeLib* CreateLib;
  TLIBATTR *   LibAttr;
  TYPEATTR *   TypeAttr;
  ITypeInfo*      AttrTypeInfo;  // valid only when TypeAttr!=0, not refcnt'd
  TBSTR*          FuncNames;     // temporary transfer of BSTR name arrays

  TOleCreateList(ITypeLib* typeLib, LPCTSTR fileName);
  void FixupTypeDescRef(int typeIndex, TYPEDESC & typeDesc);
  void Close(LPCTSTR helpDir);
  void Clear();
 ~TOleCreateList() {Clear();}
};

TOleCreateList::TOleCreateList(ITypeLib* typeLib, LPCTSTR fileName)
:
  TypeLib(typeLib),
  FileName(fileName),
  OleLib(0), OleComp(0), OleInfo(0),
  CreateLib(0), TypeAttr(0), LibAttr(0), List(0), FuncNames(0)
{
  TBSTR libName;
  TBSTR libDoc;
  TBSTR helpFile;
  ulong helpId;
  TypeLib->AddRef();
  OLECALL(::LoadTypeLib(OLE_TYPELIB_FILE, &OleLib), _T("Load stdole.tlb"));
  OLECALL(OleLib->GetTypeComp(&OleComp), _T("GetTypeComp"));
  ITypeComp* tempComp;     // required as reference arg, but always set to 0
  OLECALL(
    OleComp->BindType(
      const_cast<LPOLESTR>(OleText("IDispatch")),
      ::LHashValOfName(0, OleText("IDispatch")),
      &OleInfo,
      &tempComp
      ),
    _T("BindType"));
  OLECALL(TypeLib->GetLibAttr(&LibAttr), _T("GetLibAttr"));
  OLECALL(TypeLib->GetDocumentation(-1, libName, libDoc, &helpId, helpFile),
                                         _T("GetDocumentation for library"));
  OLECALL(::CreateTypeLib(LibAttr->syskind, OleStr((LPTSTR)FileName), &CreateLib), _T("CreateTypeLib"));
  OLECALL(CreateLib->SetGuid(LibAttr->guid), _T("Set TypeLib GUID"));
  OLECALL(CreateLib->SetLcid(LibAttr->lcid), _T("Set TypeLib language"));
  OLECALL(CreateLib->SetLibFlags(LibAttr->wLibFlags), _T("Set TypeLib flags"));
  if (!!libName)
    OLECALL(CreateLib->SetName(libName), _T("Set TypeLib name"));
  if (!!libDoc)
    OLECALL(CreateLib->SetDocString(libDoc), _T("Set TypeLib doc"));
  if (!!helpFile)
  {
    OLECALL(CreateLib->SetHelpFileName(helpFile), _T("SetHelpFileName"));
    OLECALL(CreateLib->SetHelpContext(helpId), _T("HelpContext"));
  }
  OLECALL(CreateLib->SetVersion(LibAttr->wMajorVerNum,
                                LibAttr->wMinorVerNum), _T("Set TypeLib version"));
  //  allocate all ITypeInfo's upfront to force consistent references
  Count = TypeLib->GetTypeInfoCount();
  List = new TOleCreateInfo[Count];
  for (int nInfo = 0; nInfo < Count; nInfo++) {
    TOleCreateInfo& info = List[nInfo];
    ICreateTypeInfo* newInfo;
    OLECALL(TypeLib->GetTypeInfo(nInfo, &AttrTypeInfo), _T("GetTypeInfo"));
    info.OurTypeInfo = AttrTypeInfo;
    OLECALL(info.OurTypeInfo->GetTypeAttr(&TypeAttr), _T("GetTypeAttr"));
    OLECALL(TypeLib->GetDocumentation(nInfo,libName,libDoc,&helpId,helpFile),
                                             _T("GetDocumentation for TypeInfo"));
    OLECALL(CreateLib->CreateTypeInfo(libName, TypeAttr->typekind, &newInfo),
                                   _T("Create CreateTypeInfo"));
    info.CreateInfo = newInfo;
    OLECALL(newInfo->QueryInterface(IID_ITypeInfo,
           (void **)&info.OleTypeInfo), _T("QueryInterface for ITypeInfo"));
    if (!!libDoc)
      OLECALL(newInfo->SetDocString(libDoc), _T("Set TypeInfo doc"));
    OLECALL(newInfo->SetHelpContext(helpId), _T("SetHelpContext"));
    OLECALL(newInfo->SetVersion(TypeAttr->wMajorVerNum,
                                  TypeAttr->wMinorVerNum),_T("Set TypeInfo version"));
    OLECALL(newInfo->SetGuid(TypeAttr->guid), _T("SetTypeInfo GUID"));
    OLECALL(newInfo->SetTypeFlags(TypeAttr->wTypeFlags), _T("SetTypeFlags"));
    if (TypeAttr->typekind == TKIND_DISPATCH) {
      HREFTYPE hreftype;
      OLECALL(newInfo->AddRefTypeInfo(OleInfo, &hreftype), _T("AddRefTypeInfo"));
      OLECALL(newInfo->AddImplType(0, hreftype), _T("AddImplType"));
    }
    info.TypeKind  = TypeAttr->typekind;
    info.FuncCount = TypeAttr->cFuncs;
    info.VarCount  = TypeAttr->cVars;
    info.ImplCount = TypeAttr->cImplTypes;
    info.OurTypeInfo->ReleaseTypeAttr(TypeAttr), TypeAttr = 0;
  }
}

void TOleCreateList::Close(LPCTSTR helpDir)
{
  OLECALL(CreateLib->SaveAllChanges(), _T("Write and close TypeLib file"));
  CreateLib->Release();
  CreateLib = 0;
  OLECALL(::RegisterTypeLib(TypeLib,
                            OleStr((LPTSTR)FileName),
                            OleStr((LPTSTR)helpDir)),
          _T("Register type library"));
}

void TOleCreateList::Clear()
{
  delete[] List;  // releases all interface pointers
  delete[] FuncNames;  // in case exception thrown while in use
  if (LibAttr)
    TypeLib->ReleaseTLibAttr(LibAttr);
  if (TypeAttr)
    AttrTypeInfo->ReleaseTypeAttr(TypeAttr);
  if (OleInfo)
    OleInfo->Release();
  if (OleComp)
    OleComp->Release();
  if (OleLib)
    OleLib->Release();
  if (CreateLib)
    CreateLib->Release();
  TypeLib->Release();  // finally release typelib called at constructor
}

void
TOleCreateList::FixupTypeDescRef(int typeIndex, TYPEDESC & typeDesc)
{
  if (typeDesc.vt == VT_USERDEFINED) {
    ITypeInfo* refInfo;
    OLECALL(List[typeIndex].OurTypeInfo->GetRefTypeInfo(typeDesc.hreftype,
                                                 &refInfo), _T("GetRefTypeInfo"));
    refInfo->Release();   // ok to release here, we're only using its pointer
    for (int nInfo = 0; nInfo < Count; nInfo++) {
       if (List[nInfo].OurTypeInfo == refInfo) {
         OLECALL(List[typeIndex].CreateInfo->AddRefTypeInfo(List[nInfo].OleTypeInfo,
                                       &typeDesc.hreftype), _T("AddRefTypeInfo"));
         return;
       }
    }
    OLECALL(HR_TYPE_ELEMENTNOTFOUND, _T("Unknown reference type"));
  }
}

void
TAppDescriptor::WriteTypeLibrary(TLangId lang, LPCTSTR file)
{
  TServedObject* servedObject;
  TBSTR libName;
  TBSTR libDoc;
  ulong helpId;
  FUNCDESC * funcDesc = 0;
  VARDESC * varDesc = 0;
  TOleCreateList typeList(new TTypeLibrary(*this, lang), file);
  for (int nInfo = 0; nInfo < typeList.Count; nInfo++) {
    TOleCreateInfo& curInfo = typeList.List[nInfo];
    ITypeInfo* typeInfo = curInfo.OurTypeInfo;
    ICreateTypeInfo* newInfo  = curInfo.CreateInfo;
    int index;
    for (index = 0; index < curInfo.FuncCount; index++) {
      OLECALL(typeInfo->GetFuncDesc(index, &funcDesc), _T("GetFuncDesc"));
      // Using the GetDocumentation call fails when creating a typelib, so attempt to
      // use the function GetFuncDocFromIndex to get around this problem
      servedObject = dynamic_cast<TServedObject*>(typeInfo);
      if (servedObject)
        OLECALL(servedObject->GetFuncDocFromIndex(index, libName, libDoc,
                                                  &helpId, 0), _T("Get method name and doc"));
      else
        OLECALL(typeInfo->GetDocumentation(funcDesc->memid, libName, libDoc,
                                           &helpId, 0), _T("Get method name and doc"));
      for (int nArg = funcDesc->cParams; nArg-- >=0; ) {
        ELEMDESC * elem = nArg < 0 ? &funcDesc->elemdescFunc
                                      : &funcDesc->lprgelemdescParam[nArg];
        typeList.FixupTypeDescRef(nInfo, elem->tdesc);
      }
      OLECALL(newInfo->AddFuncDesc(index, funcDesc), _T("AddFuncDesc"));
      unsigned nNames = funcDesc->cParams + 1;
      typeList.FuncNames = new TBSTR[nNames];
      OLECALL(typeInfo->GetNames(funcDesc->memid, (BSTR*)typeList.FuncNames,
                              nNames, &nNames), _T("Get method parameter names"));
      OLECALL(newInfo->SetFuncAndParamNames(index, (BSTR*)typeList.FuncNames,
                                       nNames), _T("Set method parameter names"));
      delete[] typeList.FuncNames;
      typeList.FuncNames = 0;
      if (!!libDoc)
        OLECALL(newInfo->SetFuncDocString(index, libDoc),_T("Set method doc"));
      OLECALL(newInfo->SetFuncHelpContext(index, helpId), _T("HelpContext"));
      typeInfo->ReleaseFuncDesc(funcDesc), funcDesc = 0;
    }
    for (index = 0; index < curInfo.VarCount; index++) {
      OLECALL(typeInfo->GetVarDesc(index, &varDesc), _T("GetVarDesc"));
      // Using the GetDocumentation call fails when creating a typelib, so attempt to
      // use the function GetVarDocFromIndex to get around this problem
      servedObject = dynamic_cast<TServedObject*>(typeInfo);
      if (servedObject)
        OLECALL(servedObject->GetVarDocFromIndex(index, libName, libDoc,
                                                 &helpId, 0), _T("Get propery name and doc"));
      else
        OLECALL(typeInfo->GetDocumentation(varDesc->memid, libName, libDoc,
                                           &helpId, 0), _T("Get propery name and doc"));
      typeList.FixupTypeDescRef(nInfo, varDesc->elemdescVar.tdesc);
      OLECALL(newInfo->AddVarDesc(index, varDesc), _T("AddVarDesc"));
      OLECALL(newInfo->SetVarName(index, libName),_T("Set property name"));
      if (!!libDoc)
        OLECALL(newInfo->SetVarDocString(index, libDoc),_T("Set property doc"));
      OLECALL(newInfo->SetVarHelpContext(index, helpId), _T("HelpContext"));
      typeInfo->ReleaseVarDesc(varDesc), varDesc = 0;
    }
    if (curInfo.TypeKind == TKIND_COCLASS) {
      for (index = 0; index < curInfo.ImplCount; index++) {
        HREFTYPE hreftype;
        ITypeInfo* refInfo;
        OLECALL(typeInfo->GetRefTypeOfImplType(index, &hreftype),_T("GetCoClassRef"));
        OLECALL(typeInfo->GetRefTypeInfo(hreftype, &refInfo), _T("GetCoClassTypeInfo"));
        refInfo->Release();   // ok to release here, only using its pointer
        for (int iInfo = 0; iInfo < typeList.Count; iInfo++) {
          if (typeList.List[iInfo].OurTypeInfo == refInfo) {
            OLECALL(newInfo->AddRefTypeInfo(typeList.List[iInfo].OleTypeInfo, &hreftype), _T("AddRefTypeInfo"));
            OLECALL(newInfo->AddImplType(index, hreftype), _T("AddImplType"));
            int implflags;
            OLECALL(typeInfo->GetImplTypeFlags(index, &implflags), _T("GetImplTypeFlags"));
            OLECALL(newInfo->SetImplTypeFlags(index, implflags), _T("SetImplTypeFlags"));
          }
        }
      }
    }
    OLECALL(newInfo->LayOut(), _T("Layout typeinfo"));
  }
  tstring helpDir = RegInfo.Lookup("helpdir");
  if (helpDir.empty()){
    _TCHAR path [_MAX_PATH];
    Module->GetModuleFileName(path, sizeof(path));
    helpDir = TFileName(path).GetParts(TFileName::Server|TFileName::Device|TFileName::Path);
  }
  typeList.Close(helpDir.c_str());

  int iGuid = 0;   // first pass for app, second for debug app if present
  do {
    _TCHAR buf[80];
    _tcscpy(buf, _T("CLSID\\"));
    _tcscat(buf, AppClassId[iGuid]);
    _tcscat(buf, _T("\\TypeLib"));
    TRegKey::GetClassesRoot().SetValue(buf,REG_SZ, (uint8*)(LPCTSTR)AppClassId[LibGuidOffset], 0);
    iGuid ^= DebugGuidOffset;    // remains 0 if no debug guid assigned
  } while (iGuid);
}

//____________________________________________________________________________
//
// TTypeLibrary implementation
//____________________________________________________________________________

TTypeLibrary::TTypeLibrary(TAppDescriptor& appDesc, TLangId lang)
               : AppDesc(appDesc), Lang(lang), RefCnt(0)
{
  CoClassFlags = 0;
  CoClassImplCount = 0;
  TAutoClass::TAutoClassRef* ref = AppDesc.ClassList;
  for (int index = 0; index < AppDesc.ClassCount; index++, ref++) {
    TAutoClass* cls = ref->Class;
    int    implFlags = cls->GetImplTypeFlags();
    uint16 typeFlags = cls->GetCoClassFlags();
    if (implFlags != 0 || typeFlags != 0) {
      CoClassImplCount++;
      CoClassFlags |= typeFlags;
    }
  }
}

TTypeLibrary::~TTypeLibrary()
{
  if (AppDesc.TypeLib == this)
    AppDesc.TypeLib = 0;    // remove pointer to this
  if (RefCnt > 0)
    ::CoDisconnectObject(this,0);   // should not normally happen
}

ITypeInfo*
TTypeLibrary::CreateCoClassInfo()
{
  ITypeInfo* ifc = new TCoClassInfo(AppDesc, CoClassFlags, CoClassImplCount);
  ifc->AddRef();
  return ifc;
}

HRESULT _IFUNC
TTypeLibrary::QueryInterface(const IID & riid, void * * retIface)
{
  if (riid == IID_IUnknown || riid == IID_ITypeLib) {
    AddRef();
    *retIface = (IUnknown*)this;
    return HR_NOERROR;
  }
  *retIface = 0;
  return HR_NOINTERFACE;
}

unsigned long _IFUNC

TTypeLibrary::AddRef()
{
  return ++RefCnt;
}

unsigned long _IFUNC
TTypeLibrary::Release()
{
  if (--RefCnt != 0)
    return RefCnt;
  delete this;
  return 0;
}

unsigned int _IFUNC
TTypeLibrary::GetTypeInfoCount()
{
  return AppDesc.GetClassCount() + (CoClassImplCount > 0);  // +1 for CoClass
}

HRESULT _IFUNC
TTypeLibrary::GetTypeInfo(unsigned index, ITypeInfo* * retInfo)
{
  if (CoClassImplCount > 0 && index == (uint)AppDesc.GetClassCount()) {
    *retInfo = CreateCoClassInfo();
  }
  else {
    TAutoClass* cls = AppDesc.GetAutoClass(index);
    if (!cls)
      return HR_TYPE_ELEMENTNOTFOUND;
    *retInfo = AppDesc.CreateITypeInfo(*cls);
  }
  return HR_NOERROR;
}

HRESULT _IFUNC
TTypeLibrary::GetTypeInfoType(unsigned index, TYPEKIND * retKind)
{
  unsigned int count = AppDesc.GetClassCount();
  if (index > count)
    return HR_TYPE_ELEMENTNOTFOUND;
  *retKind = (index == count ? TKIND_COCLASS : TKIND_DISPATCH);
  return HR_NOERROR;
}

HRESULT _IFUNC
TTypeLibrary::GetTypeInfoOfGuid(const GUID & guid, ITypeInfo* * retInfo)
{
  if (AppDesc.AppClassId.GetOffset(guid) == 0) {
    *retInfo = CreateCoClassInfo();
  } else {
    TAutoClass* cls = AppDesc.GetAutoClass(guid);
    if (!cls)
      return HR_TYPE_ELEMENTNOTFOUND;
    *retInfo = AppDesc.CreateITypeInfo(*cls);
  }
  return HR_NOERROR;
}

HRESULT _IFUNC
TTypeLibrary::GetLibAttr(TLIBATTR * * retAttr)
{
  TLIBATTR* libAttr = new TLIBATTR;
  memset(libAttr, 0, sizeof(TLIBATTR));
  libAttr->syskind = SYS_WIN32;
  libAttr->lcid = Lang;
  AppDesc.GetClassId(0, libAttr->guid);
  libAttr->wMajorVerNum = AppDesc.GetVersionField(0);
  libAttr->wMinorVerNum = AppDesc.GetVersionField(1);
  *retAttr = libAttr;
  return HR_NOERROR;
}

void _IFUNC
TTypeLibrary::ReleaseTLibAttr(TLIBATTR * attr)
{
  delete attr;
}

HRESULT _IFUNC
TTypeLibrary::GetTypeComp(ITypeComp* * /*retComp*/)
{
  return HR_TYPE_UNSUPFORMAT;
}

HRESULT _IFUNC
TTypeLibrary::GetDocumentation(int index, BSTR * retName,
                               BSTR * retDoc,
                               ulong * retHelpContext,
                               BSTR * retHelpFile)
{
  if (retHelpFile)
        *retHelpFile = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetHelpFile(Lang));
  if (retHelpContext)
     *retHelpContext = 0;
  if (index == -1 || index == AppDesc.GetClassCount()) { // library itself
    if (retName)
      *retName = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetAppName(Lang));
    if (retDoc)
      *retDoc  = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetAppDoc(Lang));
    if (retHelpContext)
      *retHelpContext = 0;
  } else {
    TAutoClass* cls = AppDesc.GetAutoClass(index);
    if (!cls)
      return HR_TYPE_ELEMENTNOTFOUND;
    if (retName)
      *retName = TOleAuto::SysAllocString((const OLECHAR *)cls->GetName(Lang));
    if (retDoc)
      *retDoc  = TOleAuto::SysAllocString((const OLECHAR *)cls->GetDoc(Lang));
    if (retHelpContext)
      *retHelpContext = cls->GetHelpId();
  }
  return HR_NOERROR;
}

HRESULT _IFUNC
TTypeLibrary::IsName(OLECHAR * nameBuf, ulong /*hashVal*/, int * retFound)
{
  TAutoClass::TAutoClassRef* ref = AppDesc.ClassList;
  for (int index = 0; index < AppDesc.ClassCount; index++, ref++) {
    TAutoClass* cls = ref->Class;
    // not clear from doc if we should check names of classes as well as members
    long id;
    TAutoSymbol* sym = cls->Lookup(OleStr(nameBuf), Lang, asOleType, id);
    if (sym) {
      lstrcpyW(nameBuf, OleStr(sym->Name));
      *retFound = 1;
      return HR_NOERROR;
    }
  }
  return ResultFromScode(TYPE_E_ELEMENTNOTFOUND);
}

HRESULT _IFUNC
TTypeLibrary::FindName(OLECHAR * nameBuf, ulong /*lHashVal*/,
                       ITypeInfo* * retInfo, MEMBERID * retId,
                       unsigned short * inoutCount)
{
  unsigned short found = 0;
  TAutoClass::TAutoClassRef* ref = AppDesc.ClassList;
  for (int index = 0; index < AppDesc.ClassCount && found < *inoutCount;
                      index++, ref++) {
    TAutoClass* cls = ref->Class;
    long id;
    TAutoSymbol* sym = cls->Lookup(OleStr(nameBuf), Lang, asOleType, id);
    if (sym) {
      retId[found] = id;
      retInfo[found] = AppDesc.CreateITypeInfo(*cls);
      found++;
    }
  }
  *inoutCount = found;
  return found ? HR_NOERROR : HR_TYPE_ELEMENTNOTFOUND;
}

//____________________________________________________________________________
//
// TCoClassInfo implementation
//____________________________________________________________________________

TCoClassInfo::TCoClassInfo(TAppDescriptor& appDesc, uint16 typeFlags, int implCount)
: AppDesc(appDesc), RefCnt(0), TypeFlags(typeFlags), ImplCount(implCount)
{
  ImplList = new unsigned[implCount];
  TAutoClass::TAutoClassRef* ref = AppDesc.ClassList;
  int iapp     = -1;
  int ievent   = -1;
  Default      = -1;
  DefaultEvent = -1;
  int iclass   =  0;
  for (int index = 0; iclass < implCount; index++, ref++) {
    TAutoClass* cls = ref->Class;
    int    implFlags = cls->GetImplTypeFlags();
    uint16 typeFlags = cls->GetCoClassFlags();
    if (implFlags || typeFlags) {
      if (implFlags & IMPLTYPEFLAG_FSOURCE) {
        ievent = iclass;
        if (implFlags & IMPLTYPEFLAG_FDEFAULT)
          DefaultEvent = iclass;
      } else {
        if (typeFlags & TYPEFLAG_FAPPOBJECT)
          iapp = iclass;
        if (implFlags & IMPLTYPEFLAG_FDEFAULT)
          Default = iclass;
      }
      ImplList[iclass++] = index;
    }
  }
  if (Default == -1)
    Default = iapp;
  if (DefaultEvent == -1)
    DefaultEvent = ievent;
}

TCoClassInfo::~TCoClassInfo()
{
  delete ImplList;
}

HRESULT _IFUNC
TCoClassInfo::QueryInterface(const IID & riid, void * * retIface)
{
  if (riid == IID_IUnknown || riid == IID_ITypeInfo) {
    AddRef();
    *retIface = (IUnknown*)this;
    return HR_NOERROR;
  }
  *retIface = 0;
  return HR_NOINTERFACE;
}

unsigned long _IFUNC

TCoClassInfo::AddRef()
{
  return ++RefCnt;
}

unsigned long _IFUNC
TCoClassInfo::Release()
{
  if (--RefCnt != 0)
    return RefCnt;
  delete this;
  return 0;
}

HRESULT _IFUNC
TCoClassInfo::GetTypeAttr(TYPEATTR * * retTypeAttr)
{
  TYPEATTR* ta = (TYPEATTR*)new uint8[sizeof(TYPEATTR)];
  memset(ta, 0, sizeof(TYPEATTR));
  ta->guid = AppDesc.AppClassId;
  ta->lcid = AppDesc.GetAppLang();
  ta->typekind = TKIND_COCLASS;
  ta->cImplTypes = (unsigned short)ImplCount;
  ta->wMajorVerNum = AppDesc.GetVersionField(0);
  ta->wMinorVerNum = AppDesc.GetVersionField(1);
  ta->wTypeFlags = TypeFlags;
  *retTypeAttr = ta;
  return HR_NOERROR;
}

void _IFUNC
TCoClassInfo::ReleaseTypeAttr(TYPEATTR * ptypeattr)
{
  delete [] (uint8*)ptypeattr;
}

HRESULT _IFUNC
TCoClassInfo::GetDocumentation(MEMBERID memid,
                                BSTR * retName, BSTR * retDoc,
                                ulong * retHelpContext,
                                BSTR * retHelpFile)
{
  if (retHelpFile)
    *retHelpFile = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetHelpFile(AppDesc.GetAppLang()));
  if (memid == -1) {       // request info on type library itself
    if (retName)
      *retName = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetAppName(AppDesc.GetAppLang()));
    if (retDoc)
      *retDoc  = TOleAuto::SysAllocString((const OLECHAR *)AppDesc.GetAppDoc(AppDesc.GetAppLang()));
    if (retHelpContext)
      *retHelpContext = 0;
  } else {
    return HR_TYPE_WRONGTYPEKIND;
  }
  return HR_NOERROR;
}

HRESULT _IFUNC
TCoClassInfo::CreateInstance(IUnknown* /*punkOuter*/, const IID & /*riid*/,
                              void * * /*ppvObj*/)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::GetContainingTypeLib(ITypeLib* * retLib,
                                                unsigned int * retIndex)
{
  *retLib = AppDesc.GetTypeLibrary();
  if (retIndex)
    *retIndex = AppDesc.GetClassCount();
  return HR_NOERROR;
}

HRESULT _IFUNC
TCoClassInfo::GetRefTypeInfo(HREFTYPE hreftype, ITypeInfo* * retInfo)
{
  TAutoClass* cls = (TAutoClass*)hreftype;
  if (AppDesc.GetClassIndex(cls) == -1)  // validate pointer to avoid crash
    return HR_TYPE_WRONGTYPEKIND;
  *retInfo = AppDesc.CreateITypeInfo(*cls);
  return HR_NOERROR;
}

HRESULT _IFUNC
TCoClassInfo::GetImplTypeFlags(unsigned int index, int * retflags)
{
  TAutoClass* cls = index < (uint)ImplCount ?
                     AppDesc.GetAutoClass(ImplList[index]) : 0;
  if (!cls)
    return HR_TYPE_ELEMENTNOTFOUND;
  int implFlags = cls->GetImplTypeFlags();
  if (implFlags & IMPLTYPEFLAG_FSOURCE) {
    if (index == (uint)DefaultEvent)
      implFlags |= IMPLTYPEFLAG_FDEFAULT;
  }
  else {
    if (index == (uint)Default)
      implFlags |= IMPLTYPEFLAG_FDEFAULT;
  }
  *retflags = implFlags;
  return HR_NOERROR;
}

HRESULT _IFUNC
TCoClassInfo::GetRefTypeOfImplType(unsigned int index,
                                    HREFTYPE * retreftype)
{
  TAutoClass* cls = index < (uint)ImplCount ?
                    AppDesc.GetAutoClass(ImplList[index]):0;
  *retreftype = (HREFTYPE)cls;
  return cls ? HR_NOERROR : HR_TYPE_ELEMENTNOTFOUND;
}

// The following methods of ITypeInfo are not relevant for a COCLASS typeinfo

HRESULT _IFUNC
TCoClassInfo::GetFuncDesc(unsigned int, FUNCDESC * *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

void _IFUNC
TCoClassInfo::ReleaseFuncDesc(FUNCDESC *)
{
}

HRESULT _IFUNC
TCoClassInfo::GetVarDesc(unsigned int, VARDESC * *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

void _IFUNC
TCoClassInfo::ReleaseVarDesc(VARDESC *)
{
}

HRESULT _IFUNC
TCoClassInfo::GetNames(MEMBERID, BSTR *, unsigned int, unsigned int *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::GetIDsOfNames(OLECHAR * *, uint, MEMBERID *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::Invoke(void *, MEMBERID, unsigned short, DISPPARAMS *,
                      VARIANT *, EXCEPINFO *, unsigned int *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::GetTypeComp(ITypeComp* *)
{
  return HR_TYPE_LIBNOTREGISTERED;
}

HRESULT _IFUNC
TCoClassInfo::GetMops(MEMBERID, BSTR *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::GetDllEntry(MEMBERID, INVOKEKIND, BSTR *, BSTR *,
                          unsigned short *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

HRESULT _IFUNC
TCoClassInfo::AddressOfMember(MEMBERID, INVOKEKIND, void * *)
{
  return HR_TYPE_WRONGTYPEKIND;
}

} // OCF namespace

//==============================================================================


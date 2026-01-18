//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// TAppDescriptor - OLE application descriptor definitions
//----------------------------------------------------------------------------

#if !defined(OCF_APPDESC_H)
#define OCF_APPDESC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/autodefs.h>

namespace owl {class _OWLCLASS TRegLink;};
namespace owl {class _OWLCLASS TModule;};
namespace owl {class _OWLCLASS TRegList;};

namespace ocf {

//----------------------------------------------------------------------------
// TAppDescriptor - application registration and browsing facilities
//

class             TServedObjectCreator;
class _ICLASS     TServedObject;
class _ICLASS     TTypeLibrary;
class _ICLASS     TCoClassInfo;

// Factory routine which creates OLE object
//
typedef IUnknown* (*TComponentFactory)(IUnknown* outer, owl::uint32 options, owl::uint32 id);

//
// license key support
//
// User defined class callback
//
struct TLicenseKeyFactory {
    virtual HRESULT _IFUNC GetLicInfo( LICINFO* licInfo) = 0;
    virtual HRESULT _IFUNC RequestLicKey(BSTR* retKey) = 0;
    virtual HRESULT _IFUNC QueryLicKey(BSTR key) = 0;
};

//
// class TAppDescriptor
// ~~~~~ ~~~~~~~~~~~~~~
//
class _ICLASS TAppDescriptor : public IClassFactory2 {
  public:
    TAppDescriptor(owl::TRegList& regInfo, TComponentFactory callback,
                   owl::tstring& cmdLine, owl::TModule* module = & owl::GetGlobalModule(),
                   const owl::TRegLink* regLink = 0, owl::uint32 preselectedOptions = 0);
    TAppDescriptor(owl::TRegList& regInfo, TComponentFactory callback,
                   TLicenseKeyFactory* factory, owl::tstring& cmdLine,
                   owl::TModule* module = & owl::GetGlobalModule(), const owl::TRegLink* regLink = 0,
                   owl::uint32 preselectedOptions = 0);
   ~TAppDescriptor();

    operator          IUnknown*()            {return this;}
    bool              IsAutomated() const    {return ClassCount > 0;}
    TComponentFactory GetFactory() const     {return FactoryCallback;}
    const owl::TRegLink*   GetRegLinkHead() const {return LinkHead;}
    const TLicenseKeyFactory* GetLicKeyFactory() const {return LicKeyFactoryCallback;}

    // Command line options accessors
    //
    void          ProcessCmdLine(owl::tstring& cmdLine);
    void          SetLangId(owl::TLangId prevLang, LPCTSTR langIdStr);
    bool          IsOptionSet(owl::uint32 option) const  {return (Options & option) != 0;}
    owl::uint32        GetOptions() const                {return Options;}
    void          SetOption(owl::uint32 bit, bool state)
                    {
                      if (state)
                        Options |= bit;
                      else
                        Options &= ~bit;
                    }

    // Registration management functions
    //
    void          RegisterClass();
    void          UnregisterClass();
    bool          RegisterObject(TObjectDescriptor app);
    void          UnregisterObject();
    TClassId      GetLinkGuid(int index);
    void          RegisterServer(owl::TLangId lang, LPCTSTR regFile = 0);
    void          UnregisterServer(owl::TLangId lang = 0, LPCTSTR str = 0);
    void          MakeTypeLib(owl::TLangId lang, LPCTSTR str = 0);

    // TAutoClass management functions
    //
    void          MergeAutoClasses();

    // TypeLibrary management functions
    //
    ITypeLib*     GetTypeLibrary();                  //NOTE: Caller MUST do a Release()
    ITypeInfo*    CreateITypeInfo(TAutoClass& cls);  //NOTE: Caller MUST do a Release()
    int           GetClassCount()      {return ClassCount;}
    owl::TLangId       GetAppLang()         {return AppLang;}
    int           GetClassIndex(TAutoClass* cls);  // returns -1 if not found
    bool          GetClassId(TAutoClass* cls, GUID& retId);
    TAutoClass*   GetAutoClass(unsigned index);
    TAutoClass*   GetAutoClass(const GUID& clsid);
    owl::TRegLink*     GetRegLink(const GUID& clsid);
    owl::uint16        GetVersionField(owl::uint field);
    LPCTSTR        GetAppName(owl::TLangId lang) {return AppName->Translate(lang);}
    LPCTSTR        GetAppDoc(owl::TLangId lang)  {return AppDoc->Translate(lang);}
    LPCTSTR        GetHelpFile(owl::TLangId lang){return HelpFile->Translate(lang);}
    void          WriteTypeLibrary(owl::TLangId lang, LPCTSTR file);

    // TServedObject management functions
    //
    TUnknown*      CreateAutoApp(TObjectDescriptor app, owl::uint32 options,
                                 IUnknown* outer=0);
    void          ReleaseAutoApp(TObjectDescriptor app);
    TUnknown*      CreateAutoObject(TObjectDescriptor obj, TServedObject& app,
                                    IUnknown* outer=0);
    TUnknown*      CreateAutoObject(const void* obj, const std::type_info& objInfo,
                                    const void* app, const std::type_info& appInfo,
                                    IUnknown* outer=0);
    TServedObject*  FindServed(const void * mostDerivedObj);
    TServedObject*  FindServed(TObjectDescriptor& objDesc);
    void            AddServed(TServedObject& obj);
    void            RemoveServed(TServedObject& obj);
    void            InvalidateObject(const void* obj);
    void            ReleaseObject(const void * obj);
    bool            IsBound() {return static_cast<bool>(ServedList != 0 || LockCount != 0);}
    bool            IsActiveObject(TServedObject* obj) {return ActiveObject == obj;}
    void            FlushServed();
    void            DeleteCreator();

    TBaseClassId    AppClassId;

  private:
    void            Init(IMalloc* alloc = 0);  // called only from constructor
    TServedObject*  ServedList;     // List of outstanding IDispatch/ITypeInfo objs
    TServedObject*  ActiveObject;   // Currently registered active object
    TServedObjectCreator* Creator; // Served Object creator class
    owl::TRegList&        RegInfo;        // Registration list for application
    LPCTSTR          Version;        // Library version, stored as text string
    owl::TLocaleString*  AppProgId;      // Progid of application
    owl::TLocaleString*  AppName;        // Name of application, localizable
    owl::TLocaleString*  AppDoc;         // Description of application, localizable
    owl::TLocaleString*  HelpFile;       // Name of help file, localizable
    TAutoClass::TAutoClassRef*
                    ClassList;      // Array of automated class references
    int              ClassCount;     // Count of classes, set when first scanned
    int              DebugGuidOffset;// GUID offset for /Debug registration
    int              LibGuidOffset;  // GUID offset for the app library
    int              LinkGuidOffset; // GUID offset for the first doc template
    int              LinkGuidCount;  // count of GUIDs assigned to doc templates
    owl::TLangId          AppLang;        // Language Id for this running instance
    TTypeLibrary*    TypeLib;        // Type library, only while referenced
    const owl::TRegLink* LinkHead;      // Reglink list of reglists to register
    TComponentFactory FactoryCallback;  // callback to create/destroy instance
    TLicenseKeyFactory* LicKeyFactoryCallback; // callback to get/check license info
    owl::TModule*        Module;
    unsigned long    RefCnt;
    unsigned        LockCount;
    unsigned long    RegClassHdl;    // For unregistration
    unsigned long    RegObjectHdl;   // For unregistration
    owl::uint32          Options;        // Command line option bit flags

    // For EXE or DLL component main, provide direct access for destructors
    // For auxilliary DLLs, a table must be maintained of AppDesc vs. task Id
    //
    static TAppDescriptor* This;   // set by constructor to component instance

    // IUnknown interface
    //
    HRESULT       _IFUNC QueryInterface(const IID & iid, void** retIface);
    unsigned long _IFUNC AddRef();
    unsigned long _IFUNC Release();

  public:

    // IClassFactory interface
    //
    HRESULT _IFUNC CreateInstance(IUnknown* outer, const IID & iid, void** retObject);
    HRESULT _IFUNC LockServer(int lock);

    // IClassFactory2 interface
    //
    HRESULT _IFUNC GetLicInfo( LICINFO* licInfo);
    HRESULT _IFUNC RequestLicKey(DWORD reserved,  BSTR* retKey);
    HRESULT _IFUNC CreateInstanceLic(IUnknown* outer,  IUnknown* reserved,
                                     const IID & iid, BSTR key,
                                     void** retObject );

  friend class _ICLASS TTypeLibrary;
  friend class _ICLASS TCoClassInfo;
  friend TAppDescriptor* GetAppDescriptor();
};

//
// For EXE and DLL component main module, provide direct access to descriptor
// For auxilliary DLLs, must use table of app descriptor vs. task Id
//
inline TAppDescriptor* GetAppDescriptor() {
  return TAppDescriptor::This;
}

//
// class TTypeLibrary
// ~~~~~ ~~~~~~~~~~~~
//
class _ICLASS TTypeLibrary : public ITypeLib {
  public:
    TTypeLibrary(TAppDescriptor& appDesc, owl::TLangId lang);
   ~TTypeLibrary();

// !BB    operator IUnknown&() {return *this;}  // !CQ compiler doesn't allow this!?!

    // Component typeinfo - NOTE: Caller MUST Release()
    //
    ITypeInfo*      CreateCoClassInfo();

  private:
    TAppDescriptor& AppDesc;
    unsigned long   RefCnt;
    owl::TLangId         Lang;
    owl::uint16          CoClassFlags;      // Type flags combined from autoclasses
    int             CoClassImplCount;  // Number of interfaces in coclass

    // IUnknown interface
    //
    HRESULT       _IFUNC QueryInterface(const IID & iid, void** retIface);
    unsigned long _IFUNC AddRef();
    unsigned long _IFUNC Release();

    // ITypeLib interface
    //
    unsigned int _IFUNC GetTypeInfoCount();
    HRESULT      _IFUNC GetTypeInfo(unsigned index, ITypeInfo** retInfo);
    HRESULT      _IFUNC GetTypeInfoType(unsigned index, TYPEKIND * retKind);
    HRESULT      _IFUNC GetTypeInfoOfGuid(const GUID& guid, ITypeInfo** retInfo);
    HRESULT      _IFUNC GetLibAttr(TLIBATTR** retAttr);
    HRESULT      _IFUNC GetTypeComp(ITypeComp** retComp);
    HRESULT      _IFUNC GetDocumentation(int index, BSTR* retName,
                                         BSTR* retDoc,
                                         unsigned long* retHelpContext,
                                         BSTR* retHelpFile);
    HRESULT      _IFUNC IsName(OLECHAR* nameBuf, unsigned long hashVal,
                               int* retFound);
    HRESULT      _IFUNC FindName(OLECHAR* nameBuf, unsigned long lHashVal,
                                 ITypeInfo** retInfo, MEMBERID* retId,
                                 unsigned short * inoutCount);
    void         _IFUNC ReleaseTLibAttr(TLIBATTR* attr);
};

//
// class TCoClassInfo
// ~~~~~ ~~~~~~~~~~~~
//
class _ICLASS TCoClassInfo : public ITypeInfo {
  public:
    TCoClassInfo(TAppDescriptor& appDesc, owl::uint16 typeFlags, int implCount);
   ~TCoClassInfo();

// !BB    operator IUnknown&() {return *this;}  // !CQ compiler doesn't allow this!?!

  private:
    TAppDescriptor& AppDesc;
    unsigned long   RefCnt;
    owl::uint16          TypeFlags;   // type flags combined from autoclasses
    int             ImplCount;   // number of interfaces in coclass
    unsigned*       ImplList;    // array of indices to autoclasses
    int             Default;     // interface marked as default
    int             DefaultEvent;// event interface marked as default,source

    // IUnknown interface
    //
    HRESULT       _IFUNC QueryInterface(const IID & iid, void** retIface);
    unsigned long _IFUNC AddRef();
    unsigned long _IFUNC Release();

    // ITypeInfo implementation
    //
    HRESULT _IFUNC GetTypeAttr(TYPEATTR** pptypeattr);
    HRESULT _IFUNC GetTypeComp(ITypeComp** pptcomp);
    HRESULT _IFUNC GetFuncDesc(unsigned int index, FUNCDESC** ppfuncdesc);
    HRESULT _IFUNC GetVarDesc(unsigned int index, VARDESC * * ppvardesc);
    HRESULT _IFUNC GetNames(MEMBERID memid, BSTR * rgbstrNames,
                            unsigned int cMaxNames,
                            unsigned int * pcNames);
    HRESULT _IFUNC GetRefTypeOfImplType(unsigned int index, HREFTYPE * phreftype);
    HRESULT _IFUNC GetImplTypeFlags(unsigned int index, int * pimpltypeflags);
    HRESULT _IFUNC GetIDsOfNames(OLECHAR * * rgszNames,
                                 unsigned int cNames,
                                 MEMBERID * rgmemid);
    HRESULT _IFUNC Invoke(void * pvInstance, MEMBERID memid,
                          unsigned short wFlags,
                          DISPPARAMS  *pdispparams,
                          VARIANT  *pvarResult,
                          EXCEPINFO  *pexcepinfo,
                          unsigned int  *puArgErr);
    HRESULT _IFUNC GetDocumentation(MEMBERID memid,
                                    BSTR * pbstrName,
                                    BSTR * pbstrDocString,
                                    owl::ulong * pdwHelpContext,
                                    BSTR * pbstrHelpFile);
    HRESULT _IFUNC GetDllEntry(MEMBERID memid, INVOKEKIND invkind,
                               BSTR * pbstrDllName,
                               BSTR * pbstrName,
                               unsigned short * pwOrdinal);
    HRESULT _IFUNC GetRefTypeInfo(HREFTYPE hreftype,
                                  ITypeInfo* * pptinfo);
    HRESULT _IFUNC AddressOfMember(MEMBERID memid, INVOKEKIND invkind,
                                   void * * ppv);
    HRESULT _IFUNC CreateInstance(IUnknown* punkOuter, const IID & riid,
                                  void * * ppvObj);
    HRESULT _IFUNC GetMops(MEMBERID memid, BSTR * pbstrMops);
    HRESULT _IFUNC GetContainingTypeLib(ITypeLib* * pptlib,
                                        owl::uint * pindex);
    void _IFUNC ReleaseTypeAttr(TYPEATTR * ptypeattr);
    void _IFUNC ReleaseFuncDesc(FUNCDESC * pfuncdesc);
    void _IFUNC ReleaseVarDesc(VARDESC * pvardesc);
};

//
// external entry points for DLL servers
//
typedef DECLARE_FUNC(HRESULT,STDAPICALLTYPE,*TDllCanUnloadNow)();
typedef DECLARE_FUNC(HRESULT,STDAPICALLTYPE,*TDllGetClassObject)(const GUID& clsid, const GUID& iid, void** retObj);
typedef DECLARE_FUNC(HRESULT,STDAPICALLTYPE,*TDllRegisterServer)();
typedef DECLARE_FUNC(HRESULT,STDAPICALLTYPE,*TDllUnregisterServer)();
typedef DECLARE_FUNC(HRESULT,STDAPICALLTYPE,*TDllRegisterCommand)(const char* cmdLine);

} // OCF namespace


#endif // OCF_APPDESC_H



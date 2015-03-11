//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TModule.  TModule defines the basic behavior for OWL
/// libraries and applications.
//----------------------------------------------------------------------------

#if !defined(OWL_MODULE_H)
#define OWL_MODULE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/objstrm.h>
#include <owl/except.h>
#include <owl/pointer.h>
#include <owl/lclstrng.h>
#include <owl/private/gmodule.h>
#include <owl/wsyscls.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TWindow;
class _OWLCLASS TDialog;
class _OWLCLASS TXInvalidModule;
class _OWLCLASS TXModuleVersionInfo; // [VH 2005-04-03]

/// \addtogroup module
/// @{
/// \class TModule
// ~~~~~ ~~~~~~~
/// ObjectWindows dynamic-link libraries (DLLs) construct an instance of TModule,
/// which acts as an object-oriented stand-in for the library (DLL) module. TModule
/// defines behavior shared by both library and application modules. ObjectWindows
/// applications construct an instance of TApplication, derived from TModule.
/// TModule's constructors manage loading and freeing of external DLLs, and the
/// member functions provide support for default error handling.
class _OWLCLASS TModule : public TStreamableBase {
  public:
    // Class scoped types
    //
    typedef HINSTANCE THandle;  ///< TModule encapsulates an HINSTANCE

    // Constructors & destructor
    //
    TModule(const tstring& name, bool shouldLoad = true, bool mustLoad = true, bool addToList=true);
    TModule(const tstring& name, THandle handle, bool addToList=true);
    TModule(const tstring& name, THandle handle, const tstring& cmdLine, bool addToList=true);
    virtual ~TModule();

    /// Finish-up initialization of a module
    //
    void          InitModule(THandle handle, const tstring& cmdLine);

    /// \name Get & set members. Use these instead of directly accessing members
    /// @{
    LPCTSTR       GetName() const;
    void          SetName(const tstring& name);

    THandle       GetHandle() const;      // Get the module instance handle
    operator      THandle() const;
    bool operator ==(const TModule& m) const;
    bool          IsLoaded() const;
    /// @}

    // Module wide error handler. Called when fatal exceptions are caught.
    //
    virtual int   Error(TXBase& x, uint captionResId, uint promptResId=0);

    /// \name Windows HINSTANCE/HMODULE related API functions encapsulated
    /// @{
    int           GetModuleFileName(LPTSTR buff, int maxChars) const;
    tstring    GetModuleFileName() const;

    FARPROC       GetProcAddress(TNarrowResId) const;

    HRSRC         FindResource(TResId id, TResId type) const;
    HRSRC         FindResourceEx(TResId id, TResId type, TLangId langId=LangNeutral) const;
    HGLOBAL       LoadResource(HRSRC hRsrc) const;
    uint32        SizeofResource(HRSRC hRsrc) const;

    int           LoadString(uint id, LPTSTR buf, int maxChars) const;
    tstring    LoadString(uint id) const;
    HBITMAP       LoadBitmap(TResId id) const;
    bool          GetClassInfo(TResId name, WNDCLASS* wndclass) const;
    HACCEL        LoadAccelerators(TResId id) const;
    HMENU         LoadMenu(TResId id) const;
    HCURSOR       LoadCursor(TResId id) const;
    HICON         LoadIcon(TResId name) const;
    std::string LoadHtml(TResId) const;

    HICON         CopyIcon(HICON hIcon) const;
		/// @}

    //
    /// \name Global  search for  resources
    /// @{
    static TModule* FindResModule(TResId id, TResId type);
    static TModule* NextModule(TModule* module = 0);
    /// @}

  protected:
    void          SetHandle(THandle handle);  ///< Set the module instance handle

  protected_data:
    tstring Name; ///< Name of the module
    THandle Handle; ///< Module handle

  private:
    bool          ShouldFree; ///< Should free the module when done?

    // Hidden to prevent accidental copying or assignment
    //
    TModule(const TModule&);
    TModule& operator =(const TModule&);

  friend _OWLCFUNC(std::ostream&) operator <<(std::ostream& os, const TModule& m);
  DECLARE_STREAMABLE_OWL(TModule, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TModule);

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//
// Bring in the system's version info header if not already included
//
namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TModuleVersionInfo
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// TModuleVersionInfo provides access to a TModule's VERSIONINFO resource.
//
class _OWLCLASS TModuleVersionInfo {
  public:
    /// TFileOS values are returned by GetFileOS()
    enum TFileOS { OSUnknown    = VOS_UNKNOWN,
                   DOS          = VOS_DOS,
                   OS216        = VOS_OS216,
                   OS232        = VOS_OS232,
                   NT           = VOS_NT,
                   Windows16    = VOS__WINDOWS16,
                   PM16         = VOS__PM16,
                   PM32         = VOS__PM32,
                   Windows32    = VOS__WINDOWS32,
                   DosWindows16 = VOS_DOS_WINDOWS16,
                   DosWindows32 = VOS_DOS_WINDOWS32,
                   OS216PM16    = VOS_OS216_PM16,
                   OS232PM32    = VOS_OS232_PM32,
                   NTWindows32  = VOS_NT_WINDOWS32
    };
    /// TFileType is returned by GetFileType()
    enum TFileType { TypeUnknown = VFT_UNKNOWN,
                     App         = VFT_APP,
                     DLL         = VFT_DLL,
                     DevDriver   = VFT_DRV,
                     Font        = VFT_FONT,
                     VirtDevice  = VFT_VXD,
                     StaticLib   = VFT_STATIC_LIB
    };
    /// TFileSubType values are returned by GetFileSubType() if GetFileType
    // returned DevDriver or Font
    enum TFileSubType { UnknownDevDriver,  ///< VFT2_UNKNOWN
                        PtrDriver,         ///< VFT2_DRV_PRINTER
                        KybdDriver,        ///< VFT2_DRV_KEYBOARD
                        LangDriver,        ///< VFT2_DRV_LANGUAGE
                        DisplayDriver,     ///< VFT2_DRV_DISPLAY
                        MouseDriver,       ///< VFT2_DRV_MOUSE
                        NtwkDriver,        ///< VFT2_DRV_NETWORK
                        SysDriver,         ///< VFT2_DRV_SYSTEM
                        InstallableDriver, ///< VFT2_DRV_INSTALLABLE
                        SoundDriver,       ///< VFT2_DRV_SOUND
                        UnknownFont,       ///< VFT2_UNKNOWN
                        RasterFont,        ///< VFT2_FONT_RASTER
                        VectorFont,        ///< VFT2_FONT_VECTOR
                        TrueTypeFont       ///< VFT2_FONT_TRUETYPE
    };
    TModuleVersionInfo(TModule::THandle module);
    TModuleVersionInfo(const tstring& modFName);
   ~TModuleVersionInfo();

    VS_FIXEDFILEINFO & GetFixedInfo();

    uint32    GetSignature() const;
    uint32    GetStrucVersion() const;
    uint32    GetFileVersionMS() const;
    uint32    GetFileVersionLS() const;
    uint32    GetProductVersionMS() const;
    uint32    GetProductVersionLS() const;
    bool      IsFileFlagSet(uint32 flag) const;
    uint32    GetFileFlagsMask() const;
    uint32    GetFileFlags() const;
    bool      IsDebug() const;
    bool      InfoInferred() const;
    bool      IsPatched() const;
    bool      IsPreRelease() const;
    bool      IsPrivateBuild() const;
    bool      IsSpecialBuild() const;
    uint32    GetFileOS() const;  ///< returns TFileOS values
    TFileType GetFileType() const;
    uint32    GetFileSubType() const;
    FILETIME  GetFileDate() const;

    bool GetInfoString(LPCTSTR str, LPCTSTR& value, uint lang=0);

    bool GetFileDescription(LPCTSTR& fileDesc, uint lang=0);
    bool GetFileVersion(LPCTSTR& fileVersion, uint lang=0);
    bool GetInternalName(LPCTSTR& internalName, uint lang=0);
    bool GetLegalCopyright(LPCTSTR& copyright, uint lang=0);
    bool GetOriginalFilename(LPCTSTR& originalFilename, uint lang=0);
    bool GetProductName(LPCTSTR& prodName, uint lang=0);
    bool GetProductVersion(LPCTSTR& prodVersion, uint lang=0);
    bool GetSpecialBuild(LPCTSTR& debug, uint lang=0);

    // String versions of the query functions. [VH 2005-04-03]
    // These functions throw TXModuleVersionInfo on error.

    tstring GetInfoString(const tstring& str, uint lang=0);

    tstring GetFileDescription(uint lang=0);
    tstring GetFileVersion(uint lang=0);
    tstring GetInternalName(uint lang=0);
    tstring GetLegalCopyright(uint lang=0);
    tstring GetOriginalFilename(uint lang=0);
    tstring GetProductName(uint lang=0);
    tstring GetProductVersion(uint lang=0);
    tstring GetSpecialBuild(uint lang=0);

    uint        GetLanguage() const;
    tstring  GetLanguageName() const;

    static tstring GetLanguageName(uint language);

  protected:
    void Init(LPCTSTR modFName);

    uint8 *            Buff;       ///<  new'd File version info buffer
    uint32                Lang;       ///<  Default language translation
    VS_FIXEDFILEINFO * FixedInfo;  ///<  Fixed file info structure

  private:
    // Don't allow this object to be copied.
    //
    TModuleVersionInfo(const TModuleVersionInfo&);
    TModuleVersionInfo& operator =(const TModuleVersionInfo&);
};

//
/// \class TXInvalidModule
// ~~~~~ ~~~~~~~~~~~~~~~
/// A nested class, TXInvalidModule describes an exception that results from an
/// invalid module. A window throws this exception if it can't create a valid
/// TModule object.
class _OWLCLASS TXInvalidModule : public TXOwl {
  public:
    TXInvalidModule(const tstring& name = tstring());

    virtual TXInvalidModule* Clone() const; // override
    void Throw();

    static void Raise(const tstring& name = tstring());
};

//
/// Exception class for TModuleVersionInfo. [VH 2005-04-03]
//
class _OWLCLASS TXModuleVersionInfo : public TXOwl {
  public:
    TXModuleVersionInfo(const tstring& name = tstring());

    virtual TXModuleVersionInfo* Clone() const; // override
    void Throw();

    static void Raise(const tstring& name = tstring());
};


//
//
/// \class TSystemMessage
//~~~~~~~~~~~~~~~~~~~~~
//
class _OWLCLASS TSystemMessage {
  public:
    TSystemMessage(); ///< default errorId, def language
    TSystemMessage(uint32 error, TLangId langId = LangNeutral);

    int MessageBox(TWindow* wnd, const tstring& msg, const tstring& title, uint flags = MB_OK);
    const tstring& SysMessage() const;
    const uint32      SysError() const;

  protected:
    void        Init(TLangId langId);
    uint32      Error;
    tstring   Message;
};

//
/// \class TErrorMode
// ~~~~~ ~~~~~~~~~~
/// Simple encapsulation of the SetErrorMode call. Manages putting the error
/// mode back to its previous state on destruction, thus is exception safe.
//
class TErrorMode {
  public:
    TErrorMode(uint mode);
   ~TErrorMode();

  private:
    uint PrevMode;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//----------------------------------------------------------------------------
// Global variable and functions

//
// Exported pointers from OWL modules, implemented in GLOBAL.CPP
// Unmanagled to allow easy loading via LoadLibrary
//
namespace owl {class _OWLCLASS TDocTemplate;};
#if defined(BI_COMP_BORLANDC) 
extern "C" {
  owl::TDocTemplate** PASCAL GetDocTemplateHead(owl::uint32 version);
  owl::TModule** PASCAL GetModulePtr(owl::uint32 version);
}
#else
STDAPI_(owl::TDocTemplate**) GetDocTemplateHead(owl::uint32 version);
STDAPI_(owl::TModule**) GetModulePtr(owl::uint32 version);
#endif
//
/// Main entry point for an Owl application
//
int OwlMain(int argc, _TCHAR* argv[]);

extern "C" {

//
/// Initialization routine that must be called from User DLL if DLL
/// provides it's own entry point [i.e. LibMain or DllEntryPoint]
//
bool OWLInitUserDLL(HINSTANCE hInstance, LPTSTR cmdLine);
}

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//----------------------------------------------------------------------------
// Definition of TDllLoader template, and TModuleProc class & derived
// templates. 
// TModuleProcX<>s provide dynamic binding & call access to exported module
// procs
//

//
/// \class TDllLoader<>
// ~~~~~ ~~~~~~~~~~~~
/// TDllLoader<> provides an easy way to load one instance of a DLL on demand.
//
template <class T> class TDllLoader {
  public:
    TDllLoader();
   ~TDllLoader();
    static bool IsAvailable();
    static T*   Dll();

#if 0
    // !BB Static data members in template is a problem when
    // !BB exporting/importing templates. Basically, the initialization of
    // !BB the static member [see the statement which initializes Dll to 0
    // !BB below] must only be seen when exporting the template and causes an
    // !BB error when one is importing the template: 'Cannot initialize
    // !BB extern'. The error message makes sense since to the importer, the
    // !BB data member is an extern variable. However, there's a catch 22:
    // !BB Only the user of the template header knows whether it's
    // !BB importing or exporting a particular instance of the template. So
    // !BB how does the user communicate to the header whether or not to have
    // !BB the initialization code?? Furthermore, what if a particular user of
    // !BB the template is importing the template for Type1 and exporting it
    // !BB for Type2. Now, we have a problem either way.
    // !BB
    // !BB Ideally the compiler would simply ignore the initialization of
    // !BB a static data member when the code including the template is
    // !BB actually importing the template. For now though, we'll avoid
    // !BB static data members and use a static variable within a static
    // !BB method instead. It's less elegant but it solves this problem.

    static T*   Dll;
#endif
  private:
    static T*&  DllPtrRef();
};


#if 0
// !BB See comment about static data members and importing/exporting
// !BB templates above.

//
/// One static pointer to the dll object, loaded on demand
//
template <class T> T* TDllLoader<T>::Dll = 0;
#endif

//
/// Creates a dynamic copy of the DLL object (i.e. an instance of T) if we
/// do not have one already...
//
template <class T> TDllLoader<T>::TDllLoader()
{
  T*& dll = DllPtrRef();
  //WARN(dll != 0, _T("Multiple copies of DllLoaders for DLL: ") << *dll);
  if (dll == 0) {
    try {
      dll = new T;
    }
    catch (...) {
    }
  }
}

//
/// Deletes the dll object when we go away to release the dll from memory
//
template <class T> TDllLoader<T>::~TDllLoader()
{
#if 0 // !BB
  delete Dll;
  Dll = 0;
#endif
  T*& dll = DllPtrRef();
  delete dll;
  dll = 0;
}

//
/// Load the dll on demand, returning true if it was loaded OK
//
//#if !defined(WINELIB)//JJH
template <class T> bool TDllLoader<T>::IsAvailable()
{
  static TDllLoader<T> ThisLoader;
  return DllPtrRef() != 0;
}
//#endif

//
//
//
template <class T> T* TDllLoader<T>::Dll()
{
  PRECONDITION(DllPtrRef() != 0);
  return DllPtrRef();
}

//
/// Method encapsulating single instance of pointer to DLL objecct
//
//#if !defined(WINELIB)//JJH
template <class T> T*& TDllLoader<T>::DllPtrRef()
{
  static T* ThisDll = 0;
  return ThisDll;
}
//#endif

//
/// \class TModuleProc
/// TModuleProc is a base class that does inital binding of a function name or ordinal to the 
/// corresponding function in the given module (DLL).
/// It is required and assumed that the bound function is using the WINAPI calling convention.
/// Derived class templates perform type-safe parameter passing on call using
/// a different template for each number of parameters, 'V' version for void return, i.e.
/// TModuleProcV0, TModuleProc0, TModuleProcV1, TModuleProc1, etc.
/// The constructor throws an exception if it cannot bind.
//
class _OWLCLASS TModuleProc {
  protected:
    TModuleProc(const TModule& module, TNarrowResId id);

  protected:
/// Derived template classes perform type-safe parameter passing on call. Different
/// class for each number of parameters, 'V' version for void return.
    FARPROC Proc;
};

//
/// \cond
//
class TModuleProcV0 : public TModuleProc {
  public:
    TModuleProcV0(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)();
    void operator ()() {
      ((TProc)Proc)();
    }
};

//
template <class R>
class TModuleProc0 : public TModuleProc {
  public:
    TModuleProc0(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)();
    R operator ()() {
      return ((TProc)Proc)();
    }
};

//
template <class P1>
class TModuleProcV1 : public TModuleProc {
  public:
    TModuleProcV1(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1);
    void operator ()(P1 p1) {
      ((TProc)Proc)(p1);
    }
};

//
template <class R, class P1>
class TModuleProc1 : public TModuleProc {
  public:
    TModuleProc1(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1);
    R operator ()(P1 p1) {
      return ((TProc)Proc)(p1);
    }
};

//
template <class P1, class P2>
class TModuleProcV2 : public TModuleProc {
  public:
    TModuleProcV2(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 a2);
    void operator ()(P1 p1, P2 a2) {
      ((TProc)Proc)(p1, a2);
    }
};

//
template <class R, class P1, class P2>
class TModuleProc2 : public TModuleProc {
  public:
    TModuleProc2(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2);
    R operator ()(P1 p1, P2 p2) {
      return ((TProc)Proc)(p1, p2);
    }
};

//
template <class P1, class P2, class P3>
class TModuleProcV3 : public TModuleProc {
  public:
    TModuleProcV3(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3);
    void operator ()(P1 p1, P2 p2, P3 p3) {
      ((TProc)Proc)(p1, p2, p3);
    }
};

//
template <class R, class P1, class P2, class P3>
class TModuleProc3 : public TModuleProc {
  public:
    TModuleProc3(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3);
    R operator ()(P1 p1, P2 p2, P3 p3) {
      return ((TProc)Proc)(p1, p2, p3);
    }
};

//
template <class P1, class P2, class P3, class P4>
class TModuleProcV4 : public TModuleProc {
  public:
    TModuleProcV4(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4) {
      ((TProc)Proc)(p1, p2, p3, p4);
    }
};

//
template <class R, class P1, class P2, class P3, class P4>
class TModuleProc4 : public TModuleProc {
  public:
    TModuleProc4(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4) {
      return ((TProc)Proc)(p1, p2, p3, p4);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5>
class TModuleProcV5 : public TModuleProc {
  public:
    TModuleProcV5(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
      ((TProc)Proc)(p1, p2, p3, p4, p5);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5>
class TModuleProc5 : public TModuleProc {
  public:
    TModuleProc5(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6>
class TModuleProcV6 : public TModuleProc {
  public:
    TModuleProcV6(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
class TModuleProc6 : public TModuleProc {
  public:
    TModuleProc6(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6, class P7>
class TModuleProcV7 : public TModuleProc {
  public:
    TModuleProcV7(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                     P7 p7);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
      ((TProc)Proc) (p1, p2, p3, p4, p5, p6, p7);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7>
class TModuleProc7 : public TModuleProc {
  public:
    TModuleProc7(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8>
class TModuleProcV8 : public TModuleProc {
  public:
    TModuleProcV8(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                     P7 p7, P8 p8);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8>
class TModuleProc8 : public TModuleProc {
  public:
    TModuleProc8(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7, P8 p8);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9>
class TModuleProcV9 : public TModuleProc {
  public:
    TModuleProcV9(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                    P7 p7, P8 p8, P9 p9);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9>
class TModuleProc9 : public TModuleProc {
  public:
    TModuleProc9(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7, P8 p8, P9 p9);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10>
class TModuleProcV10 : public TModuleProc {
  public:
    TModuleProcV10(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                     P7 p7, P8 p8, P9 p9, P10 p10);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10>
class TModuleProc10 : public TModuleProc {
  public:
    TModuleProc10(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7, P8 p8, P9 p9, P10 p10);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11>
class TModuleProcV11 : public TModuleProc {
  public:
    TModuleProcV11(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                     P7 p7, P8 p8, P9 p9, P10 p10, P11 p11);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11>
class TModuleProc11 : public TModuleProc {
  public:
    TModuleProc11(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7, P8 p8, P9 p9, P10 p10, P11 p11);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12>
class TModuleProcV12 : public TModuleProc {
  public:
    TModuleProcV12(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                     P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11, P12 p12) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12>
class TModuleProc12 : public TModuleProc {
  public:
    TModuleProc12(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                  P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11, P12 p12) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
    }
};

//
template <class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13>
class TModuleProcV13 : public TModuleProc {
  public:
    TModuleProcV13(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                    P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,
                                    P12 p12, P13 p13);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11, P12 p12, P13 p13) {
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
    }
};

//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13>
class TModuleProc13 : public TModuleProc {
  public:
    TModuleProc13(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
                                    P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,
                                    P12 p12, P13 p13);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
                  P10 p10, P11 p11, P12 p12, P13 p13) {
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,class P14>
class TModuleProcV14 : public TModuleProc {
  public:
    TModuleProcV14(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12,p13,p14);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14>
class TModuleProc14 : public TModuleProc {
  public:
    TModuleProc14(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12,p13,p14);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,class P14, 
          class P15>
class TModuleProcV15 : public TModuleProc {
  public:
    TModuleProcV15(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,p15);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15>
class TModuleProc15 : public TModuleProc {
  public:
    TModuleProc15(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16>
class TModuleProcV16 : public TModuleProc {
  public:
    TModuleProcV16(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,
                    p15,p16);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16>
class TModuleProc16 : public TModuleProc {
  public:
    TModuleProc16(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15,p16);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17>
class TModuleProcV17 : public TModuleProc {
  public:
    TModuleProcV17(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,
                    p15,p16,p17);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17>
class TModuleProc17 : public TModuleProc {
  public:
    TModuleProc17(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15,p16,p17);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18>
class TModuleProcV18 : public TModuleProc {
  public:
    TModuleProcV18(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, P18 p18){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,
                    p15,p16,p17,p18);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18>
class TModuleProc18 : public TModuleProc {
  public:
    TModuleProc18(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, P18 p18){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15,p16,p17,p18);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18, class P19>
class TModuleProcV19 : public TModuleProc {
  public:
    TModuleProcV19(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18, P19 p19);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, 
         P18 p18, P19 p19){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,
                    p15,p16,p17,p18,p19);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18, class P19>
class TModuleProc19 : public TModuleProc {
  public:
    TModuleProc19(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18, P19 p19);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, 
         P18 p18, P19 p19){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15,p16,p17,p18,p19);
    }
};
//
template <class P1, class P2, class P3, class P4, class P5, class P6,class P7, 
          class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18, class P19, 
          class P20>
class TModuleProcV20 : public TModuleProc {
  public:
    TModuleProcV20(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef void ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18, P19 p19, P20 p20);
    void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, 
         P18 p18, P19 p19, P20 p20){
      ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13,p14,
                    p15,p16,p17,p18,p19,p20);
    }
};
//
template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class P8, class P9, class P10, class P11, class P12, class P13,
          class P14, class P15, class P16, class P17, class P18, class P19, 
          class P20>
class TModuleProc20 : public TModuleProc {
  public:
    TModuleProc20(const TModule& module, TNarrowResId id) : TModuleProc(module, id) {}

    typedef R ( WINAPI* TProc)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6,
         P7 p7, P8 p8, P9 p9, P10 p10, P11 p11,P12 p12, P13 p13, P14 p14,
         P15 p15, P16 p16, P17 p17, P18 p18, P19 p19, P20 p20);
    R operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,
         P10 p10, P11 p11, P12 p12, P13 p13, P14 p14,P15 p15, P16 p16, P17 p17, 
         P18 p18, P19 p19, P20 p20){
      return ((TProc)Proc)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, 
                           p13,p14,p15,p16,p17,p18,p19,p20);
    }
};

/// \endcond

//----------------------------------------------------------------------------
// System DLL Wrappers
//


//
/// \class TUser 
// ~~~~~ ~~~~~
/// delay loading USER :), just for some memory/resource check programs
class _OWLCLASS TUser {
  public:
    static  HICON      LoadIcon(HINSTANCE, LPCTSTR);
    static  BOOL      DestroyIcon(HICON);
    static  BOOL      GetClassInfo(HINSTANCE, LPCTSTR, LPWNDCLASS);
    static  int        GetMenuString(HMENU, UINT, LPTSTR, int, UINT);
    static  UINT      GetMenuState(HMENU, UINT, UINT);
    static  TModule&  GetModule();
};

//
/// \class TVersion 
// ~~~~~ ~~~~~~~~
/// delay loading VERSION.DLL
class _OWLCLASS TVersion {
  public:
    static  BOOL  GetFileVersionInfo(LPTSTR,DWORD,DWORD,LPVOID);
    static  DWORD  GetFileVersionInfoSize(LPTSTR, LPDWORD);
    static  BOOL  VerQueryValue(const LPVOID,LPTSTR,LPVOID,uint *);
    static  DWORD VerLanguageName(DWORD,LPTSTR,DWORD);
    static  TModule&  GetModule();
};

//
/// \class TOle 
// ~~~~~ ~~~~~~~~
/// delay loading Ole32.DLL/COMPOBJ.DLL
class _OWLCLASS TOle {
  public:
    static  HRESULT    CoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
    static  TModule&  GetModule();
};

//
/// \class TOleAuto 
// ~~~~~ ~~~~~~~~
/// delay loading OLEAUT32.DLL/OLE2DISP.DLL
class _OWLCLASS TOleAuto {
  public:
    static  HRESULT    SysFreeString(BSTR);
    static  UINT      SysStringLen(BSTR);
    static  BSTR      SysAllocString(const OLECHAR *);
    static  TModule&  GetModule();
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the name of the module.
//
inline LPCTSTR TModule::GetName() const
{
  return Name.c_str();
}

//
/// Return the instance handle of the library module represented by the
/// TModule obect.
//
inline TModule::THandle TModule::GetHandle() const
{
  return Handle;
}

//
/// Returns the handle of the application or DLL module represented by this TModule.
/// The handle must be supplied as a parameter to Windows when loading resources.
//
inline TModule::operator TModule::THandle() const
{
  return GetHandle();
}

//
/// Returns true if this instance is equal to the other instance; otherwise, returns
/// false.
//
inline bool TModule::operator ==(const TModule& m) const
{
  return GetHandle() == m.GetHandle();
}

//
/// Returns a nonzero value if the instance handle is loaded. Use this function
/// primarily to ensure that a given instance is loaded.
//
inline bool TModule::IsLoaded() const
{
  return GetHandle() > HINSTANCE(HINSTANCE_ERROR);
}

//
/// Returns the expanded file name (path and file name) of the file from which this
/// module was loaded. buff points to a buffer that holds the path and file name.
/// maxChars specifies the length of the buffer. The expanded filename is truncated
/// if it exceeds this limit. GetModeFileName returns 0 if an error occurs.
//
inline int TModule::GetModuleFileName(LPTSTR buff, int maxChars) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return ::GetModuleFileName(Handle, buff, maxChars);
}

//
/// Returns the entry-point address of the specified exported function if found,
/// otherwise returns NULL.
//
inline FARPROC TModule::GetProcAddress(TNarrowResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return ::GetProcAddress(Handle, id.GetPointerRepresentation());
}


//
/// Wrapper for the Windows API to find a particular resource.
//
/// Finds the resource indicated by id and type and, if successful, returns a handle
/// to the specified resource. If the resource cannot be found, the return value is
/// zero. The id and type parameters either point to zero-terminated strings or
/// specify an integer value. type can be one of the standard resource types defined
/// below.
/// 
/// - \c \b  RT_ACCELERATOR	Accelerator table
/// - \c \b  RT_BITMAP	Bitmap
/// - \c \b  RT_CURSOR	Cursor
/// - \c \b  RT_DIALOG 	Dialog box
/// - \c \b  RT_FONT	Font
/// - \c \b  RT_FONTDIR	Font directory
/// - \c \b  RT_ICON	Icon
/// - \c \b  RT_MENU	Menu
/// - \c \b  RT_RCDATA	User-defined resource
/// - \c \b  RT_STRING	String
//
inline HRSRC TModule::FindResource(TResId id, TResId type) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return ::FindResource(Handle, id, type);
}

//
/// Wrapper for the Windows API to find a particular resource.
//
/// WIN32: Finds the resource indicated by id and type and, if successful, returns a
/// handle to the specified resource. If the resource cannot be found, the return
/// value is zero. The id and type parameters either point to zero-terminated
/// strings or specify an integer value. type can be one of the standard resource
/// types defined below.
/// 
/// - \c \b  RT_ACCELERATOR	Accelerator table
/// - \c \b  RT_BITMAP	Bitmap
/// - \c \b  RT_CURSOR	Cursor
/// - \c \b  RT_DIALOG 	Dialog box
/// - \c \b  RT_FONT	Font
/// - \c \b  RT_FONTDIR	Font directory
/// - \c \b  RT_ICON	Icon
/// - \c \b  RT_MENU	Menu
/// - \c \b  RT_RCDATA	User-defined resource
/// - \c \b  RT_STRING	String
//
inline HRSRC TModule::FindResourceEx(TResId id, TResId type, TLangId langId) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
//  return ::FindResourceEx(Handle, id, type, langId);  // The order of parameters is reversed
  return ::FindResourceEx(Handle, type, id, langId);
}
//
/// Wrapper for the Windows API.
//
/// Loads a resource indicated by hRsrc into memory and returns a handle to the
/// memory block that contains the resource. If the resource cannot be found, the
/// return value is 0. The hRsrc parameter must be a handle created by FindResource.
/// 
/// LoadResource loads the resource into memory only if it has not been previously
/// loaded. If the resource has already been loaded, LoadResource increments the
/// reference count by one and returns a handle to the existing resource. The
/// resource remains loaded until it is discarded.
//
inline HGLOBAL TModule::LoadResource(HRSRC hRsrc) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return ::LoadResource(Handle, hRsrc);
}

//
/// Wrapper for the Windows API.
//
/// Returns the size, in bytes, of the resource indicated by hRscr. The resource
/// must be a resource handle created by FindResource. If the resource cannot be
/// found, the return value is 0.
/// 
/// Because of alignment in the executable file, the returned size might be larger
/// than the actual size of the resource. An application cannot rely on
/// SizeofResource for the exact size of a resource.
//
inline uint32 TModule::SizeofResource(HRSRC hRsrc) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return ::SizeofResource(Handle, hRsrc);
}


//
/// Wrapper for the Windows API.
//
/// Used particularly for subclassing, GetClassInfo gets information about the
/// window class specified in wndclass. name points to a 0-terminated string that
/// contains the name of the class. wndclass points to the WNDCLASS structure that
/// receives information about the class. If successful, GetClassInfo returns
/// nonzero. If a matching class cannot be found, GetClassInfo returns zero.
//
inline bool TModule::GetClassInfo(TResId name, WNDCLASS* wndclass) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  return TUser::GetClassInfo(Handle, name, wndclass);
}


//
/// Wrapper for the Windows API.
///
/// Copies the icon specified in hIcon. The return value is a handle to the icon or
/// 0 if unsuccessful. When no longer required, the duplicate icon should be
/// destroyed.
//
inline HICON TModule::CopyIcon(HICON hIcon) const
{
  return ::CopyIcon(hIcon);
}

//
/// Return the version information about this module.
//
inline VS_FIXEDFILEINFO & TModuleVersionInfo::GetFixedInfo()
{
  PRECONDITION(FixedInfo);
  return *FixedInfo;
}

//
inline uint32 TModuleVersionInfo::GetSignature() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwSignature;
}

//
inline uint32 TModuleVersionInfo::GetStrucVersion() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwStrucVersion;
}

//
/// Get the major file version (first 32-bits).
//
inline uint32 TModuleVersionInfo::GetFileVersionMS() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwFileVersionMS;
}

//
/// Get the minor file version (last 32-bits).
//
inline uint32 TModuleVersionInfo::GetFileVersionLS() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwFileVersionLS;
}

//
/// Get the major product version number (first 32-bits).
//
inline uint32 TModuleVersionInfo::GetProductVersionMS() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwProductVersionMS;
}

//
/// Get the minor product version number (last 32-bits).
//
inline uint32 TModuleVersionInfo::GetProductVersionLS() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwProductVersionLS;
}

//
/// Return true if the flag has been set in the version info.
//
inline bool TModuleVersionInfo::IsFileFlagSet(uint32 flag) const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlagsMask & flag) && (FixedInfo->dwFileFlags & flag);
}

//
inline uint32 TModuleVersionInfo::GetFileFlagsMask() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwFileFlagsMask;
}

//
inline uint32 TModuleVersionInfo::GetFileFlags() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwFileFlags;
}

//
inline bool TModuleVersionInfo::IsDebug() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_DEBUG) ?
         true : false;
}

//
inline bool TModuleVersionInfo::InfoInferred() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_INFOINFERRED) ?
          true : false;
}

//
inline bool TModuleVersionInfo::IsPatched() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_PATCHED) ?
          true : false;
}

//
inline bool TModuleVersionInfo::IsPreRelease() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_PRERELEASE) ?
          true : false;
}

//
inline bool TModuleVersionInfo::IsPrivateBuild() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_PRIVATEBUILD) ?
          true : false;
}

//
inline bool TModuleVersionInfo::IsSpecialBuild() const
{
  PRECONDITION(FixedInfo);
  return (FixedInfo->dwFileFlags & FixedInfo->dwFileFlagsMask & VS_FF_SPECIALBUILD) ?
          true : false;
}

/// returns TFileOS values
inline uint32 TModuleVersionInfo::GetFileOS() const
{
  PRECONDITION(FixedInfo);
  return FixedInfo->dwFileOS;
}

//
inline TModuleVersionInfo::TFileType TModuleVersionInfo::GetFileType() const
{
  PRECONDITION(FixedInfo);
  return (TFileType)FixedInfo->dwFileType;
}

//
/// Return the language id of this module.
//
inline uint TModuleVersionInfo::GetLanguage() const
{
  return uint(Lang);
}

//
/// Return the language name of this module.
//
inline tstring TModuleVersionInfo::GetLanguageName() const
{
  return GetLanguageName(GetLanguage());
}
inline const tstring& TSystemMessage::SysMessage() const {
  return Message;
}
inline const uint32 TSystemMessage::SysError() const {
  return Error;
}

//
/// Constructs a TErrorMode object which invokes the 'SetErrorMode' API
/// function to control how/whether Windows handles interrupt 24h errors.
//
inline TErrorMode::TErrorMode(uint mode)
{
  PrevMode = ::SetErrorMode(mode);
}

//
/// Destructor of TErrorMode object - restores the state of the error mode
/// saved during construction of the object.
//
inline TErrorMode::~TErrorMode()
{
  ::SetErrorMode(PrevMode);
}

} // OWL namespace

#endif  // OWL_MODULE_H

//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// OLE Automation Class Definitions
//----------------------------------------------------------------------------

#if !defined(OCF_AUTODEFS_H)
#define OCF_AUTODEFS_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <owl/lclstrng.h>   // TLocaleString and TRegItem/TRegList
#include <owl/string.h>     // TString and TUString
#include <ocf/oleutil.h>

#if !defined(_OLECTL_H_)
# if defined(__BIVBX_H) && !defined(NO_VBX_PICTURES)
#   error BIVBX.H LPPICTURE is incompatible with OLECTL.H - define NO_VBX_PICTURES
# else
#   define  NO_VBX_PICTURES     // Make sure we're BIVBX's compatible
#   include <olectl.h>          // Ole Controls definitions
# endif
#endif



namespace ocf {

//
// Class modifier for inherited classes or members within user C++ classes
// if ambient class model is not used, this macro must be defined to match
//
#if !defined(_AUTOCLASS)
# define  _AUTOCLASS
#endif

//
// Forward class references
//
class _AUTOCLASS TAutoDetach;
class _AUTOCLASS TAutoBase;
class _ICLASS TAppDescriptor;  // defined in appdesc.h
class _ICLASS TServedObject;
class _ICLASS TAutoIterator;
class    TAutoCreator;
class    TAutoCommand;
struct  TAutoSymbol;
class    TAutoString;
class    TAutoStack;
class  TAutoVal;
class    TXAuto;
class    TAutoEnum;

_OCFFUNC(const void *) DynamicCast(const void * obj, const std::type_info& src,
                                                  const std::type_info& dst);

_OCFFUNC(const void *) MostDerived(const void * obj, const std::type_info& src);
//
// Global function to invalidate external references, class model-independent
//
_OCFFUNC(void) SendObituary(const void * obj, const std::type_info& typeInfo);

//----------------------------------------------------------------------------
// Automation typedefs
//

typedef unsigned long TLocaleId;   // locale ID - same as NT LCID

class _AUTOCLASS TAutoBase {   // default base for automatable objects
  public:
    virtual ~TAutoBase();// only member is virtual destructor for unregistration
};

class _AUTOCLASS TAutoDetach { // default base for detach notification objects
  protected:
    void Notify(int offset, const std::type_info& typeInfo);  // sends obituary
};          // the size of this object must be 0 to not affect enclosing class

typedef void* ObjectPtr;  // generic opaque object pointer

//
// Generic conversion function for changing between inherited classes

//
template <class T, class B> struct TTypeConvert {
  static ObjectPtr Cast(ObjectPtr from) { return
   reinterpret_cast<ObjectPtr>( dynamic_cast<T*>(reinterpret_cast<B*>(from)));
  }
};

typedef ObjectPtr      (*TAutoSymTypeConvert) (ObjectPtr obj);
typedef TAutoCommand*  (*TAutoCommandBuild)(ObjectPtr obj,int attr,TAutoStack&);
typedef TAutoCommand*  (*TAutoCommandBuildDtr)(ObjectPtr obj, int attr);
typedef TAutoIterator* (*TAutoIteratorBuild)(ObjectPtr obj,
                                             TAutoCreator& creator,
                                             IUnknown* owner, owl::TLangId lang);
typedef IUnknown&      (*TAggregator)(ObjectPtr obj, TUnknown& inner);

#if defined(BI_COMP_BORLANDC)
extern "C" const GUID __cdecl IID_TServedObject;
#else
EXTERN_C const GUID  IID_TServedObject; // check it !!!!!!!!!!!!!!!!!!!!!!!!
#endif

//
/// \class TXAuto
// ~~~~~ ~~~~~~
/// Automation exception object
//
class _OCFCLASS TXAuto : public owl::TXBase {
  public:
    enum TError {
      xNoError,                 //  0
      xConversionFailure,       //  1
      xNotIDispatch,            //  2
      xForeignIDispatch,        //  3
      xTypeMismatch,            //  4
      xNoArgSymbol,             //  5
      xParameterMissing,        //  6
      xNoDefaultValue,          //  7
      xValidateFailure,         //  8
      xExecutionFailure,        //  9
      xErrorStatus,             // 10
    };
    TXAuto(TError err);
           void Throw();
    static void  Raise(TError err);

  public:
    TError ErrorCode;
};

//----------------------------------------------------------------------------
// TAutoType, TAutoClass - automation type/class descriptor
//

struct _OCFCLASS TAutoType {
    owl::uint16 GetType() const {return Type;}
    owl::uint16 Type;
};

class _OCFCLASS TAutoClass : public TAutoType {
  public:
    TAutoClass(TAutoSymbol* table, TAutoSymbol* classSymbol,
               const std::type_info& typeInfo, TAggregator aggregator=0);
   ~TAutoClass();

    short         CountCommands();    // forces counts to be set on first call
    TAutoSymbol*  Lookup(LPTSTR name, owl::TLangId lang, short symFlags, long & id);
    TAutoSymbol*  LookupArg(LPTSTR name, owl::TLangId lang, TAutoSymbol* cmd,
                            long & retid);
    TAutoSymbol*  FindId(long id, ObjectPtr& objptr);
    TAutoSymbol*  FindFunction(unsigned index, MEMBERID& retId);
    TAutoSymbol*  FindVariable(unsigned index, MEMBERID& retId);
    TXAuto::TError Dispatch(ObjectPtr obj, TAutoCreator& creator,
                            TUnknown& owner, int attr, TAutoStack& args,
                            TAutoVal* retval);
    TAutoSymbol*          GetTable() const;
    TAutoSymbol*          GetClassSymbol() const;
    short                 GetArgCount(TAutoSymbol& sym);
    TAutoCommandBuildDtr  GetDestructor() const;
    const std::type_info& GetTypeInfo() const;
    LPCTSTR               GetName(owl::TLangId lang = owl::TLocaleString::UserDefaultLangId) const;
    LPCTSTR               GetDoc (owl::TLangId lang = owl::TLocaleString::UserDefaultLangId) const;
    unsigned long         GetHelpId() const;
    unsigned short        GetTypeFlags() const;
    int                   GetImplTypeFlags() const;
    unsigned short        GetCoClassFlags() const;
    TAggregator           GetAggregator() const;
    IUnknown&             Aggregate(ObjectPtr obj, TUnknown& inner);

  protected:
    TAggregator Aggregator;  ///< aggregator function for C++ COM classes
    short CommandCount;      ///< command count including bases, 0 if uncounted
    short FunctionCount;     ///< number of symbols exposed as typelib functions
    short VariableCount;     ///< number of symbols exposed as typelib variables
    bool  AutoIds;           ///< generate ID's automatically if true (default)
    TAutoSymbol* Table;      ///< pointer to array of symbol entries
    TAutoSymbol* ClassSymbol;///< pointer to class symbol (terminator)
    const std::type_info& TypeInfo;///< for validating type of data pointers
    friend class TServedObject;

  private:
    struct TAutoClassRef {   ///< element used for array of TAutoClass objects
      TAutoClass* Class;
      int         GuidOffset;
    };
    struct TExtLink;      // fwd declaration
    static struct TClassList { ///< module-static TAutoClass data
      int  CountAutoClasses();  // walks list in attatched DLLs
      TAutoClassRef* MergeAutoClasses(TAutoClassRef* array);
      TAutoClass* List;   ///< chain of autoclasses in THIS module
      int         Count;  ///< count of classes in THIS module
      TExtLink*   Link;   ///< chain of TAutoClass data for loaded DLLs
    } ClassList;
    struct TExtLink {      // doubly-linked list of external DLL data
      TExtLink(TClassList* list, HINSTANCE module);
     ~TExtLink();
      TClassList* Classes;
      HINSTANCE   Module; // module handle if dynmaically loaded, else 0
      TExtLink*   Next;
      TExtLink**  Prev;
    };
    TAutoClass*   NextClass;          ///< link to next class
    friend struct TExtLink;            ///< access to ClassList
    friend struct TClassList;          ///< access to TAutoClassRef
    friend class  TAppDescriptor;      ///< access to linkage information
    friend class _ICLASS TTypeLibrary;///< access to TAutoClassRef
    friend class _ICLASS TCoClassInfo;///< access to TAutoClassRef
    friend class _ICLASS TOcControlEvent;     ///< access to AutoIds bool
};

//----------------------------------------------------------------------------
// TAuto(type) automation datatype encapsulations
//

//
// Class to implement assignent of void to TAutoVal
//
struct _OCFCLASS TAutoVoid {
  static TAutoType ClassInfo;
};

//
// Class to implement assignment of VT_ERROR/DISP_E_PARAMNOTFOUND
// (i.e. Optional arguments) to TAutoVal
//
struct _OCFCLASS TNoArg {
   //
   // Dummy structure
};

//
// Wrapper class to disambiguate automation date type from double
//
#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
struct _OCFCLASS TAutoDate {
  TAutoDate() : Date(0) {}
  TAutoDate(double d) : Date(d) {}
  operator double() { return Date; }
  double Date;      // same as OLE DATE
  static TAutoType ClassInfo;
};

//struct TAutoCurrency : public CY {
struct _OCFCLASS TAutoCurrency {
  owl::uint32 Lo;
  owl::int32  Hi;

  operator CY& () {return *reinterpret_cast<CY*>(this);}

  static TAutoType ClassInfo;
};

struct _OCFCLASS TAutoBool      { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoBoolRef   { static TAutoType ClassInfo; };

//
// The following types do not hold data, but only provide static type codes
//
struct _OCFCLASS TAutoShort      { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoDouble     { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoFloat      { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoLong       { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoByte       { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoUnknown    { static TAutoType ClassInfo; };  // for raw IUnknown*
struct _OCFCLASS TAutoDispatch   { static TAutoType ClassInfo; };  // for raw IDispatch*
struct _OCFCLASS TAutoVariant    { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoShortRef   { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoLongRef    { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoFloatRef   { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoDoubleRef  { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoCurrencyRef{ static TAutoType ClassInfo; };
struct _OCFCLASS TAutoDateRef    { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoStringRef  { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoVariantRef { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoSafeArray  { static TAutoType ClassInfo; };
struct _OCFCLASS TAutoByteRef    { static TAutoType ClassInfo; };
//#if (MAXINT==MAXSHORT)
//  typedef TAutoShort    TAutoInt;
//  typedef TAutoShortRef TAutoIntRef;
//#else
  typedef TAutoLong    TAutoInt;
  typedef TAutoLongRef TAutoIntRef;
//#endif

//----------------------------------------------------------------------------
/// Attribute flags for automation symbols and command objects
//
enum AutoSymFlag {
  asAnyCommand     = 0x0017,  ///< any command: method, property get/set, build
  asOleType        = 0x0007,  ///< method or property exposed for OLE
  asMethod         = 0x0001,  ///< method     (same as OLE INVOKE_FUNC)
  asGet            = 0x0002,  ///< returns property value (INVOKE_PROPERTYGET)
  asIterator       = 0x000A,  ///< iterator property (_NewEnum)
  asSet            = 0x0004,  ///< set property value     (INVOKE_PROPERTYSET)
  asGetSet         = 0x0006,  ///< can get or set property(...GET + ...SET)
  asBuild          = 0x0010,  ///< contructor command (unsupported by OLE 2.01)
  asFactory        = 0x0020,  ///< for creating objects or class determination
  asClass          = 0x0040,  ///< extension to another class symbol table
  asArgument       = 0x0080,  ///< property returning an object
  asNotTerminator  = 0x00FF,  ///< any symbol except terminator with class info

  asBindable       = 0x0400,  ///< sends OnChanged notification
  asRequestEdit    = 0x0800,  ///< sends OnRequest edit before change
  asDisplayBind    = 0x1000,  ///< user-display of bindable
  asDefaultBind    = 0x2000,  ///< this property only is the default (redundant)
  asHidden         = 0x4000,  ///< not visible to normal browsing
  asPersistent     = 0x8000,  ///< property is persistent
};

//
/// Automation datatypes and flags - same as OLE 2 definitions
//
enum AutoDataType {
  atVoid              = VT_EMPTY,     //  0x0000,   //  void
  atNull              = VT_NULL,      //  0x0001,   //  SQL style Null
  atShort             = VT_I2,        //  0x0002,   //  2 byte signed int
  atLong              = VT_I4,        //  0x0003,   //  4 byte signed int
  atFloat             = VT_R4,        //  0x0004,   //  4 byte real
  atDouble            = VT_R8,        //  0x0005,   //  8 byte real
  atCurrency          = VT_CY,        //  0x0006,   //  currency
  atDatetime          = VT_DATE,      //  0x0007,   //  datetime as double
  atString            = VT_BSTR,      //  0x0008,   //  String preceeded by length
  atObject            = VT_DISPATCH,  //  0x0009,   //  IDispatch*
  atError             = VT_ERROR,     //  0x000A,   //  SCODE
  atBool              = VT_BOOL,      //  0x000B,   //  True=-1, False=0
  atVariant           = VT_VARIANT,   //  0x000C,   //  VARIANT*
  atUnknown           = VT_UNKNOWN,   //  0x000D,   //  IUnknown*
  atByte              = VT_UI1,       //  0x0011,   //  byte, unsigned char
  atArray             = VT_ARRAY,     //
  atObjectDesc        = 0x001D,   //  object precursor state (internal use only)
  atLoanedBSTR        = 0x001A,   //  BSTR owned by TUString (internal use only)
  atTypeMask          = 0x001F,   //  base type code without bit flags
  atOLE2Mask          = 0x601F,   //  type code with bit flags
};
const owl::uint16 atSafeArray= 0x2000;
const owl::uint16 atByRef    = 0x4000;
const owl::uint16 atEnum     = 0x1000; // non-OLE, enumeration for data of a type
const owl::uint16 atAutoClass= 0x0800; // non-OLE, class object is a TAutoClass

//----------------------------------------------------------------------------
// Automated class type info flags - corresponding to OLE 2.02 definitions
//    flags set in END_AUTOCLASS, END_AUTOAGGREGATE, END_AUTOEVENTCLASS macros
//

//
// Flags set on any autoclass, but apply only to enclosing coclass type info.
// Autoclasses with any of these bits set will be members of generated coclass.
// These flags from all autoclasses are combined to form the coclass type flags

//
const owl::uint16 tfAppObject     =   1; // TYPEFLAG_FAPPOBJECT, set on COCLASS
const owl::uint16 tfCanCreate     =   2; // TYPEFLAG_FCANCREATE, set on COCLASS
const owl::uint16 tfLicensed      =   4; // TYPEFLAG_FLICENSED,  set on COCLASS
const owl::uint16 tfPredeclared   =   8; // TYPEFLAG_FPREDECLID, set on COCLASS
const owl::uint16 tfControl       =  32; // TYPEFLAG_FCONTROL,   set on COCLASS
const owl::uint16 tfCoClassXfer   =  tfAppObject|tfCanCreate|tfLicensed|tfControl|tfPredeclared;

//
// Flags set on any autoclass, but used when class is a member of a coclass.
// Autoclasses with any of these bits set will be members of generated coclass.
// Flags from each class are set on the corresponding interfaces in the coclass

//
const owl::uint16 tfDefault      = (1<<12); // IMPLTYPEFLAG_FDEFAULT << 12
const owl::uint16 tfEventSource  = (2<<12); // IMPLTYPEFLAG_FSOURCE << 12
const owl::uint16 tfRestricted   = (4<<12); // IMPLTYPEFLAG_FRESTRICTED << 12
const owl::uint16 tfImplFlagXfer = (tfDefault | tfEventSource | tfRestricted);

//
// Flags set on individual autoclasses, transferred to corresponding __OWL_TYPEINFOs

//
const owl::uint16 tfHidden        =  16;  // TYPEFLAG_FHIDDEN
const owl::uint16 tfNonextensible = 128;  // TYPEFLAG_FNONEXTENSIBLE
const owl::uint16 tfAutoClassMask =  tfHidden|tfNonextensible;

//
// Flags defined by OLE, but not applicable to dispatch interfaces

//
const owl::uint16 tfDual          =  64;  // TYPEFLAG_FDUAL
const owl::uint16 tfAutomation    = 256;  // TYPEFLAG_FOLEAUTOMATION

//
// Default typeflags value for autoclass not exposed as part of the coclass

//
const owl::uint16 tfNormal        =   0;  // automated classes not at app level

//
/// \class TAutoSymbol
// ~~~~~ ~~~~~~~~~~~
/// Symbol table element
//
struct _OCFCLASS TAutoSymbol {
  owl::TLocaleString Name;         ///< name of symbol, initially untranslated
  owl::TLocaleString Doc;          ///< documentation string, initially untranslated
  owl::uint32 Attr;                ///< attributes: enum AutoSymFlag, as???? (could be owl::uint16)
  union {
    long  DispId;             ///< reserved dispatch ID, if not 0
    short SymCount;           ///< asClass only: symbol count, 0 = uncounted yet
    unsigned short TypeFlags; ///< class symbol only (terminator)
  };
  TAutoType* Type;                  ///< pointer to class/type/enum descriptor
  union {
    TAutoCommandBuild  Build;       ///< asAnyCommand only: address of command builder
    TAutoIteratorBuild BuildIter;   ///< asIterator only: address of iterator builder
    TAutoSymTypeConvert Convert;    ///< asFactory/asClass: function to perform cast
    TAutoCommandBuildDtr BuildDtr;  ///< terminator only: destructor command builder
    void*      DefaultArg;          ///< asArgument: default value, extends to 8 bytes
  };
  owl::ulong HelpId;                     ///< help context ID, not for asArgument

  // Inline data member accessor functions
  //
  void         SetFlag(owl::uint16 mask);
  void         ClearFlag(owl::uint16 mask);
  bool         TestFlag(owl::uint16 mask) const;
  owl::uint16       GetFlags() const;
  bool         IsEnum() const;
  bool         IsByRef() const;
  bool         IsArray() const;
  bool         IsIterator() const;
  bool         IsTerminator() const;
  owl::uint16       GetDataType() const;
  TAutoClass*  GetClass() const;
  TAutoEnum*   GetEnum() const;

  // Inline static functions solely to provide type-safety initializing symbols
  //
  static TAutoCommandBuild InitTypeConvert(TAutoSymTypeConvert f);
  static TAutoCommandBuild InitAutoIterator(TAutoIteratorBuild f);
  static TAutoCommandBuild InitAutoDestructor(TAutoCommandBuildDtr f);
};

//
/// \struct TObjectDescriptor
// ~~~~~~ ~~~~~~~~~~~~~~~~~
/// Precursor to TServedObject containing object info
/// \note Cannot have a constructor and must limit to 8 bytes for TAutoVal union
//
struct _OCFCLASS TObjectDescBase {   ///< ONLY for use when storing in TAutoVal union
  const void* Object;      ///< pointer to C++ object
  TAutoClass* Class;       ///< class descriptor, contains type info
  const void* MostDerived() {
    return ocf::MostDerived(Object, Class->GetTypeInfo());
  }
};

// Double check size of TObjectDescBase
//
#if defined(BI_COMP_BORLANDC) && !defined(__clang__)
  #if sizeof(TObjectDescBase) > 8
    #error Sizeof TObjectDescBase must be 8 bytes - it is a TAutoVal union member
  #endif
#endif

//
// struct TObjectDescriptor
// ~~~~~~ ~~~~~~~~~~~~~~~~~
struct _OCFCLASS TObjectDescriptor : public TObjectDescBase {
  enum TDestruct     { ///< behavior when an automation helper is freed
    Quiet = 0,         ///< automation object quietly goes away without notice
    Delete = 1,        ///< automation object deletes the C++ object serviced
    PostQuit = 2       ///< automation object posts a quit message to application
  };
  TDestruct Destruct;
  TObjectDescriptor(const void* obj, TAutoClass& classobj,
                    TDestruct destruct = Quiet)
                    { Object = obj; Class = &classobj; Destruct = destruct; }
  private:
    TObjectDescriptor() {}    // uninitialized struct
  friend class TAutoClass;    // restrict access to uninitialized constructor
  friend class TAutoIterator;
};


//
/// \class TAutoString
// ~~~~~ ~~~~~~~~~~~
/// Based on reference counted TString with added automation functionality
//
class _OCFCLASS TAutoString : public owl::TString {
  public:
    TAutoString(const char* s = 0) : owl::TString(s) {}
    TAutoString(const wchar_t* s)      : owl::TString(s) {}
    TAutoString(BSTR s, bool loan)     : owl::TString(s) {}
    TAutoString(const owl::tstring& s) : owl::TString(s) {}
    TAutoString(owl::TUString* s)     : owl::TString(s) {}
    explicit TAutoString(TAutoVal& val);
    TAutoString(const TAutoString& src) : owl::TString(src.S) {++*S;}

    TAutoString& operator =(const char* s) {S = S->Assign(s); return *this;}
    TAutoString& operator =(const TAutoString& s) {S = S->Assign(*s.S); return *this;}
    TAutoString& operator =(char* s)           {S = S->Assign(s); return *this;}
    TAutoString& operator =(const wchar_t* s)  {S = S->Assign(s); return *this;}
    TAutoString& operator =(wchar_t* s)        {S = S->Assign(s); return *this;}
    TAutoString& operator =(TAutoVal& val);

    static TAutoType ClassInfo;              // automation type code
};


//
/// \class TAutoVal
// ~~~~~ ~~~~~~~~
/// Automation data element (same data as OLE/BASIC VARIANT)
//
/// This class simply duplicates and adds access methods to the system VARIANT
/// Data members or virtual functions cannot be added.
/// We rely on the fact that we can cast a VARIANT safely to/from a TAutoVal.
//

#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
class _OCFCLASS TAutoVal {
  public:

    // Default constructors/assignment operators/destructors
    //
    TAutoVal();
    TAutoVal(const TAutoVal& src);
  const TAutoVal& operator =(const TAutoVal& src);
    ~TAutoVal();

    // Constructors to create a TAutoVal/VARIANT from a C++ type
    //
    TAutoVal(unsigned char      i){ *this = i; };
    TAutoVal(signed char        i){ *this = i; };
    TAutoVal(char               i){ *this = i; };
    explicit TAutoVal(unsigned char* p){ *this = p; };
    explicit TAutoVal(signed char*   p){ *this = p; };
    explicit TAutoVal(char*          p){ *this = p; };
    TAutoVal(const char*        p){ *this = p; };
    TAutoVal(int                i){ *this = i; };
    TAutoVal(int*               p){ *this = p; };
    TAutoVal(unsigned int       i){ *this = i; };
    TAutoVal(unsigned int*      p){ *this = p; };
    TAutoVal(unsigned short     i){ *this = i; };
    TAutoVal(unsigned short*    p){ *this = p; };
    TAutoVal(long               i){ *this = i; };
    TAutoVal(long*              p){ *this = p; };
    TAutoVal(unsigned long      i){ *this = i; };
    TAutoVal(unsigned long*     p){ *this = p; };
    TAutoVal(short              i){ *this = i; };
    TAutoVal(short*             p){ *this = p; };
    TAutoVal(float              i){ *this = i; };
    TAutoVal(float*             p){ *this = p; };
    TAutoVal(double             i){ *this = i; };
    TAutoVal(double*            p){ *this = p; };
    TAutoVal(void*              p){ *this = p; };
    TAutoVal(owl::TBool         i){ *this = i; };
    TAutoVal(owl::TBool*        p){ *this = p; };
    TAutoVal(const owl::tstring& s){ *this = s; };
#pragma warn -inl
    TAutoVal(TAutoString        s){ *this = s; };
#pragma warn .inl
    TAutoVal(owl::TString       s){ *this = s; };
    TAutoVal(TAutoCurrency      i){ *this = i; };
    TAutoVal(TAutoCurrency*     p){ *this = p; };
    TAutoVal(TAutoDate          i){ *this = i; };
    TAutoVal(TAutoDate*         i){ *this = i; };
    TAutoVal(IDispatch*       ifc){ *this = ifc; };
    TAutoVal(IDispatch**      ifc){ *this = ifc; };
    TAutoVal(IUnknown*        ifc){ *this = ifc; };
    TAutoVal(IUnknown**       ifc){ *this = ifc; };
    TAutoVal(IPictureDisp*    ifc){ *this = ifc; };
    TAutoVal(IFontDisp*       ifc){ *this = ifc; };
    TAutoVal(VARIANT&           v){ *this = v; };
    TAutoVal(const TNoArg&      n){ *this = n; };

    // The following constructors are added to complement the functionality of
    // TAutoVal
    TAutoVal(BSTR*              p){ *this = p; };
    TAutoVal(TBSTR*             p){ *this = p; };
    TAutoVal(TAutoVal*          p){ *this = p; };
    TAutoVal(LARGE_INTEGER      v){ *this = v; };
    TAutoVal(ULARGE_INTEGER     v){ *this = v; };
    TAutoVal(SAFEARRAY*         v) { *this = v; }
#if defined(_WIN64)
    TAutoVal(ULONG_PTR          v){ *this = v; };
#endif    

    // Operators to extract a C++ type from a TAutoVal/VARIANT
    //
    operator unsigned char();
    operator signed char();
    operator char();
    operator unsigned char*();
    operator signed char*();
    operator char*();
    operator int();
    operator int*();
    operator unsigned int();
    operator short();
    operator short*();
    operator unsigned short();
    operator unsigned short*();
    operator long();
    operator long*();
    operator unsigned long();
    operator unsigned long*();
    operator float();
    operator float*();
    operator double();
    operator double*();
    operator void*();
    operator owl::TBool();
    operator owl::TBool*();
    operator TAutoCurrency();
    operator TAutoCurrency*();
    operator TAutoDate();
    operator TAutoDate*();
    operator owl::tstring();
    operator owl::TUString*();
    operator owl::TString();
    operator TAutoString();
    operator IUnknown*();
    operator IDispatch*();
    operator IPictureDisp*();
    operator IFontDisp*();
    operator IPictureDisp&();
    operator IFontDisp&();
    operator IUnknown&();
    operator IDispatch&();
    // The following operators are added to complement the functionality of
    // TAutoVal
    operator BSTR();
    operator BSTR*();
    operator TBSTR*();
    operator SAFEARRAY*();
    operator IUnknown**();
    operator IDispatch**();
    operator TAutoVal*();
    operator LARGE_INTEGER();
    operator ULARGE_INTEGER();
#if defined(_WIN64)
    operator ULONG_PTR();
#endif

    // Assignment operators to initialize a TAutoVal/VARIANT with the contents
    // of a instance of a C++ type.
    //
    void operator =(unsigned char      i);
    void operator =(signed char        i);
    void operator =(char               i);
    void operator =(unsigned char*     p);
    void operator =(signed char*       p);
    void operator =(char*              p);
    void operator =(const char*        p);
    void operator =(int                i);
    void operator =(int*               p);
    void operator =(unsigned int       i);
    void operator =(unsigned int*      p);
    void operator =(unsigned short     i);
    void operator =(unsigned short*    p);
    void operator =(long               i);
    void operator =(long*              p);
    void operator =(unsigned long      i);
    void operator =(unsigned long*     p);
    void operator =(short              i);
    void operator =(short*             p);
    void operator =(float              i);
    void operator =(float*             p);
    void operator =(double             i);
    void operator =(double*            p);
    void operator =(void*              p);
    void operator =(owl::TBool         i);
    void operator =(owl::TBool*        p);
    void operator =(const owl::tstring& s);
    void operator =(TAutoString        s);
    void operator =(owl::TString       s);
    void operator =(TAutoCurrency      i);
    void operator =(TAutoCurrency*     p);
    void operator =(TAutoDate          i);
    void operator =(TAutoDate*         i);
    void operator =(IUnknown*        ifc);
    void operator =(IDispatch*       ifc);
    void operator =(IPictureDisp*    ifc);
    void operator =(IFontDisp*       ifc);
    void operator =(VARIANT&           v);
    void operator =(TObjectDescriptor od);
    void operator =(TAutoVoid           );
    void operator =(const TNoArg&      n);
    // The following operators are added to complement the functionality of
    // TAutoVal
    void operator =(BSTR*              p);
    void operator =(TBSTR*             p);
    void operator =(IUnknown**         p);
    void operator =(IDispatch**        p);
    void operator =(TAutoVal*          p);
    void operator =(LARGE_INTEGER      v);
    void operator =(ULARGE_INTEGER     v);
    void operator =(SAFEARRAY*         p);
#if defined(_WIN64)
    void operator =(ULONG_PTR          v);
#endif    

    // Query/Manipulate VARIANT object
    //
    int         GetDataType() const;   // Returns variant type
    void        Init();                // Initializes VARIANT/TAutoVal to 0s
    void        Clear();               // Free refs, set type to atVoid
    void        Restore();             // Revert non-OLE changes to original types
    void        Copy(const TAutoVal& copy);
    HRESULT     ChangeType(VARTYPE type, TAutoVal* src = 0);
    bool        IsRef() const;

    // Retrieve VARIANT object catering for VT_VARIANT|VT_BYREF
    //
    TAutoVal*   DereferenceVariant();

    void        SetLocale(TLocaleId);   // Associate locale with appropriate pointers
    TLocaleId    GetLocale() const;        // Retrieve locale from appropriate pointers
    owl::TLangId      GetLanguage() const;      // Retrieve language from pointer types
    owl::TString      StrVal();                 // Returns internal string pointer if atString
    bool        GetObjDesc(TObjectDescriptor&); // Return temp obj info or false
    SAFEARRAY*  GetArray();               // Return array pointer, or 0
    void*       SetByRef(AutoDataType);   // Set type, return pointer to data loc

  private:

    // The following members follow the layout of the actual VARIANT type
    //
    unsigned short vt;
    unsigned short Reserved1;
    unsigned short Reserved2;
    unsigned short Reserved3;
    union {
      unsigned char bVal;             // VT_UI1
      short         iVal;             // VT_I2
      long          lVal;             // VT_I4
      float         fltVal;           // VT_R4
      double        dblVal;           // VT_R8
      VARIANT_BOOL  boolVal;          // VT_BOOL
      SCODE         scode;            // VT_ERROR
      TAutoCurrency cyVal;            // VT_CY
      DATE          date;             // VT_DATE
      BSTR          bstrVal;          // VT_BSTR
      IUnknown*     punkVal;          // VT_UNKNOWN
      IDispatch*    pdispVal;         // VT_DISPATCH
      SAFEARRAY* parray;              // VT_ARRAY
      unsigned char* pbVal;           // VT_BYREF|VT_UI1
      short        * piVal;           // VT_BYREF|VT_I2
      long         * plVal;           // VT_BYREF|VT_I4
      float        * pfltVal;         // VT_BYREF|VT_R4
      double       * pdblVal;         // VT_BYREF|VT_R8
      VARIANT_BOOL * pbool;           // VT_BYREF|VT_BOOL
      SCODE        * pscode;          // VT_BYREF|VT_ERROR
      TAutoCurrency* pcyVal;          // VT_BYREF|VT_CY
      DATE         * pdate;           // VT_BYREF|VT_DATE
      BSTR         * pbstrVal;        // VT_BYREF|VT_BSTR
      IUnknown*    * ppunkVal;        // VT_BYREF|VT_UNKNOWN
      IDispatch*   * ppdispVal;       // VT_BYREF|VT_DISPATCH
      SAFEARRAY*   * pparray;         // VT_BYREF|VT_ARRAY
      VARIANT      * pvarVal;         // VT_BYREF|VT_VARIANT
      void         * byref;           // Generic ByRef

      // The following entries are enhancements to the VARIANT type allowing
      // OCF to 'carry' additional information. NOTE: These members make it
      // impractical to have TAutoVal derive from the VARIANT type.
      //
      struct {                       ///< Added locale info for BSTR/IDispatch
        void* Val;
        LCID  Locale;                ///< Unused 4 bytes for pointer types
      } p;

      struct {                      ///< Added info when loaned BSTR ownership
        BSTR      Val;
        owl::TUString* Holder;     ///< String holder sharing current BSTR
      } s;

      TObjectDescBase ObjDesc;       ///< Temporary space for returning object

      LARGE_INTEGER  hVal;           ///< Also support LARGE_INTEGER
      ULARGE_INTEGER uhVal;          ///< Also support ULARGE_INTEGER
#if defined(_WIN64)
      ULONG_PTR uptrVal;          ///< Also support ULONG_PTR
#endif      
    };

    void ConvRef(int type);
};
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif


// Check that TAutoVal is 'size-wise' VARIANT compatible
//
#if defined(BI_COMP_BORLANDC) && !defined(__clang__)
# if sizeof(TAutoVal) != sizeof(VARIANT)
#    Error Sizeof(TAutoVal) must match Sizeof(VARIANT)
# endif
#endif


//
/// \class TAutoStack
// ~~~~~ ~~~~~~~~~~
/// Automation argument stack abstraction
//
class _OCFCLASS TAutoStack {
  public:
    enum {SetValue = -3};     // Special arg index for property set value
    TAutoStack(DISPID dispid, VARIANT* stack, TLocaleId locale,
               int argcount,
               int namedcount, long* map, TServedObject* owner);
   ~TAutoStack();
    TAutoVal& operator[](int index);

    TAutoSymbol*    Symbol;      ///< Symbol of method/prop, args follow
    int             CurrentArg;  ///< Index of last arg requested by operator[]
    int             ArgCount;
    int             ArgSymbolCount;
    DISPID          DispId;
    owl::TLangId    LangId;
    TServedObject*  Owner;
    long            ErrorCode;   ///< Set if TXAuto::xErrorStatus returned
    LPCTSTR         ErrorMsg;    ///< Set if TXAuto::xErrorStatus returned

  protected:
    int              NamedCount;
    long*            NamedIds;
    TAutoVal         Default;
    TAutoVal*        Stack;
};

struct _OCFCLASS TAutoTransfer;              // Persistence transfer parameter structure

//
/// \class TAutoCommand
// ~~~~~ ~~~~~~~~~~~~
/// Automation abstract base class for command objects
//
class _OCFCLASS TAutoCommand {
  public:
    TAutoCommand(int attr);          // Construtor called from derived classes

    virtual ~TAutoCommand() {}
    virtual TAutoCommand* Undo();             // generate command for undo stack
    virtual int           Record(TAutoStack& q); // record command and arguments
    virtual TAutoCommand& Invoke();           // invoke all command processing
    virtual bool          Validate();         // validate parameters
    virtual void          Execute();          // perform action on C++ object
    virtual long          Report();           // check result of execution
    virtual void          Return(TAutoVal& v);// convert return value to variant
    virtual void          Transfer(TAutoTransfer& x); // stream data in or out
            void          Fail(TXAuto::TError);// throw designated exception

    // Non-virtual hook functions
    //
    typedef LPCTSTR (*TErrorMsgHook)(long errCode);
    static TErrorMsgHook SetErrorMsgHook(TErrorMsgHook callback);

    typedef bool (*TCommandHook)(TAutoCommand& cmdObj);
    static TCommandHook SetCommandHook(TCommandHook callback);

    // inline data member accessor functions
    //
    static LPCTSTR LookupError(long errCode);
    void SetFlag(int mask);
    void ClearFlag(int mask);
    bool TestFlag(int mask);
    bool IsPropSet();
    TAutoSymbol* GetSymbol();
    void SetSymbol(TAutoSymbol* sym);

  protected:
    TAutoSymbol* Symbol;      // Symbol entry generating this command
    int          Attr;        // Attribute and state flags, asXXXXXX
};
extern _OCFDATA(TAutoCommand::TErrorMsgHook) TAutoCommand_ErrorLookup;
extern _OCFDATA(TAutoCommand::TCommandHook)  TAutoCommand_InvokeHook;

//
// Build function for application Quit command implementation
//
_OCFFUNC(TAutoCommand*) AutoQuitBuild(ObjectPtr obj, int attr, TAutoStack& args);

//----------------------------------------------------------------------------
// TAutoEnum - automation enumeration descriptor
//

template <class T> struct TAutoEnumVal {
  owl::TLocaleString Name;         ///< Name of symbol, enumeration text
  T             Val;          ///< Enumeration internal value
};

class TAutoEnum : public TAutoType {
  public:
    virtual bool Convert(TAutoVal& txtVal, TAutoVal& numVal) = 0;
    virtual bool Convert(TAutoVal& numVal, owl::TLangId langId) = 0;

  protected:
    TAutoEnum(int count, int type);
    int Count;              ///< length of this symbol table
};

template <class T> class TAutoEnumT : public TAutoEnum {
  public:
    TAutoEnumT(TAutoEnumVal<T>* table, int symcount, int type);
    bool Convert(TAutoVal& txtVal, TAutoVal& numVal);
    bool Convert(TAutoVal& numVal, owl::TLangId langId);

  protected:
    TAutoEnumVal<T>* Table; ///< pointer to array of symbol entries
};

//____________________________________________________________________________
//
/// \class TAutoCreator
/// Object responsible for creating automation COM object
class TAutoCreator {
  public:
    TAutoCreator() {}
    virtual TUnknown*  CreateObject(TObjectDescriptor objDesc,
                                    IUnknown* outer = 0) = 0;
    virtual IDispatch* CreateDispatch(TObjectDescriptor objDesc,
                                      IUnknown* outer = 0) = 0;
    virtual void       Attach(TServedObject& obj) {}
    virtual void       Detach(TServedObject& obj) {}
};

class _OCFCLASS TServedObjectCreator : public TAutoCreator {
  public:
    TServedObjectCreator(TAppDescriptor& appDesc);
    TUnknown*  CreateObject(TObjectDescriptor objDesc, IUnknown* outer = 0);
    IDispatch* CreateDispatch(TObjectDescriptor objDesc, IUnknown* outer = 0);
    void       Attach(TServedObject& obj);
    void       Detach(TServedObject& obj);

    TAppDescriptor& GetAppDesc()    { return AppDesc;    }
    TServedObject*  GetAppObject()  { return AppObject;  }

  protected:
    TAppDescriptor& AppDesc;
    TServedObject*  AppObject;

  private:
    int ObjCount;

  friend class TServedObject;
};

class _ICLASS TDispatch;

class _OCFCLASS TDispatchCreator : public TAutoCreator {
  public:
    TDispatchCreator() {}
    TUnknown*  CreateObject(TObjectDescriptor objDesc, IUnknown* outer = 0);
    IDispatch* CreateDispatch(TObjectDescriptor objDesc, IUnknown* outer = 0);
};

//----------------------------------------------------------------------------
/// \class TServedObject
/// OLE object exposed for automated access of internal object
//

DECLARE_COMBASES2(TServedCOM, IDispatch, ITypeInfo)

class _ICLASS TServedObject : public TServedCOM {
  public:
    TServedObject(TObjectDescriptor& obj, TServedObjectCreator& creator,
                                          IUnknown* outer=0);
   ~TServedObject();
    TServedObject* GetAppObject()        {return Creator.AppObject;}
    TServedObjectCreator& GetCreator()  {return Creator;}

    operator IDispatch*();

    void*                  Object;        ///< pointer to C++ object instance, 0 if deleted
    const void*            RootObject;    ///< pointer to object of most derived class
    TAutoClass*            Class;        ///< class of which object is an instance
    TServedObjectCreator& Creator;
    owl::TLangId               ReqLang;      ///< language requested by caller
    TObjectDescriptor::TDestruct
                          Destruct;      ///< what to do with C++ object

    // Object reference & lifetime managment
    // For internal OCF use only
    //
    owl::ulong   _IFUNC AddRef() {return GetOuter()->AddRef();}
    owl::ulong   _IFUNC Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}

  protected:
    // TUnknown overrides
    //
    HRESULT      QueryObject(const IID & iid, void** iface);

  private:
    IID                   iidEvent;
    TServedObject*        Next;
    TServedObject**        Prev;
    // need to access local LANGID for type info, either as member or as global

    // IDispatch implementation
    HRESULT _IFUNC GetTypeInfoCount(unsigned int* pctinfo);
    HRESULT _IFUNC GetTypeInfo(unsigned int itinfo, LCID lcid,
                               ITypeInfo** pptinfo);
    HRESULT _IFUNC GetIDsOfNames(const IID & riid, OLECHAR** rgszNames,
                                 owl::uint cNames, LCID lcid,
                                 DISPID* rgdispid);
    HRESULT _IFUNC Invoke(DISPID dispidMember, const IID & riid, LCID lcid,
                                 unsigned short wFlags,
                                 DISPPARAMS* pdispparams,
                                 VARIANT* pvarResult,
                                 EXCEPINFO* pexcepinfo,
                                 unsigned int* puArgErr);

    // ITypeInfo implementation
    //
    HRESULT _IFUNC GetTypeAttr(TYPEATTR** pptypeattr);
    HRESULT _IFUNC GetTypeComp(ITypeComp** pptcomp);
    HRESULT _IFUNC GetFuncDesc(unsigned int index, FUNCDESC** ppfuncdesc);
    HRESULT _IFUNC GetVarDesc(unsigned int index, VARDESC** ppvardesc);
    HRESULT _IFUNC GetNames(MEMBERID memid, BSTR* rgbstrNames,
                            unsigned int cMaxNames,
                            unsigned int* pcNames);
    HRESULT _IFUNC GetRefTypeOfImplType(unsigned int index, HREFTYPE* phreftype);
    HRESULT _IFUNC GetImplTypeFlags(unsigned int index, int* pimpltypeflags);
    HRESULT _IFUNC GetIDsOfNames(OLECHAR** rgszNames,
                                 unsigned int cNames,
                                 MEMBERID* rgmemid);
    HRESULT _IFUNC Invoke(void* pvInstance, MEMBERID memid,
                          unsigned short wFlags,
                          DISPPARAMS  *pdispparams,
                          VARIANT  *pvarResult,
                          EXCEPINFO  *pexcepinfo,
                          unsigned int  *puArgErr);
    HRESULT _IFUNC GetDocumentation(MEMBERID memid,
                                    BSTR* pbstrName,
                                    BSTR* pbstrDocString,
                                    owl::ulong* pdwHelpContext,
                                    BSTR* pbstrHelpFile);
    HRESULT _IFUNC GetDllEntry(MEMBERID memid, INVOKEKIND invkind,
                               BSTR* pbstrDllName,
                               BSTR* pbstrName,
                               unsigned short* pwOrdinal);
    HRESULT _IFUNC GetRefTypeInfo(HREFTYPE hreftype,
                                  ITypeInfo** pptinfo);
    HRESULT _IFUNC AddressOfMember(MEMBERID memid, INVOKEKIND invkind,
                                   void** ppv);
    HRESULT _IFUNC CreateInstance(IUnknown* punkOuter, const IID & riid,
                                  void** ppvObj);
    HRESULT _IFUNC GetMops(MEMBERID memid, BSTR* pbstrMops);
    HRESULT _IFUNC GetContainingTypeLib(ITypeLib** pptlib,
                                        owl::uint* pindex);
    void _IFUNC ReleaseTypeAttr(TYPEATTR* ptypeattr);
    void _IFUNC ReleaseFuncDesc(FUNCDESC* pfuncdesc);
    void _IFUNC ReleaseVarDesc(VARDESC* pvardesc);
    HRESULT _IFUNC GetFuncDocFromIndex(unsigned index,
                                       BSTR* retName, BSTR* retDoc,
                                       owl::ulong* retHelpContext,
                                       BSTR* retHelpFile);
    // GetVarDocFromIndex and GetDocFromSym are used to by WriteTypeLib to make
    // type libraries
    HRESULT _IFUNC GetVarDocFromIndex(unsigned index,
                                      BSTR* retName, BSTR* retDoc,
                                      owl::ulong* retHelpContext,
                                      BSTR* retHelpFile);
    HRESULT _IFUNC GetDocFromSym(TAutoSymbol* sym,
                                 BSTR* retName, BSTR* retDoc,
                                 owl::ulong* retHelpContext,
                                 BSTR* retHelpFile);
    friend class _ICLASS TAppDescriptor;   // access to Next,Prev
    friend class _ICLASS TOcxView;         // access to QueryObject
    friend class _ICLASS TOcControl;       // access to QueryObject
};

//----------------------------------------------------------------------------
/// \class TDispatch
/// lightweight IDispatch implementation for automation controllers
//

DECLARE_COMBASES1(TDispatchCOM, IDispatch)

class _ICLASS TDispatch : public TDispatchCOM {
  public:
    TDispatch(const TObjectDescriptor& obj, IUnknown* outer = 0);
    void InvalidateObject() {Object = 0;}
    operator IDispatch*();

    void*        Object;      ///< pointer to C++ object instance, 0 if deleted
    TAutoClass*  Class;       ///< class of which object is an instance
  private:
    // IDispatch implementation
    HRESULT _IFUNC GetTypeInfoCount(unsigned int* pctinfo);
    HRESULT _IFUNC GetTypeInfo(unsigned int itinfo, LCID lcid,
                               ITypeInfo** pptinfo);
    HRESULT _IFUNC GetIDsOfNames(const IID & riid, OLECHAR** rgszNames,
                                 unsigned int cNames, LCID lcid,
                                 DISPID* rgdispid);
    HRESULT _IFUNC Invoke(DISPID dispidMember, const IID & riid, LCID lcid,
                                 unsigned short wFlags,
                                 DISPPARAMS* pdispparams,
                                 VARIANT* pvarResult,
                                 EXCEPINFO* pexcepinfo,
                                 unsigned int* puArgErr);
};

//----------------------------------------------------------------------------
/// \class TAutoObject
/// holders for C++ object pointers for automation conversions
//

template<class T>
class TAutoObject {
  public:
    void operator=(IDispatch& dp);
    void operator=(T* p) {P=p;}
    void operator=(T& r) {P=&r;}
    T& operator*() {return *P;}
    operator TObjectDescriptor() {return TObjectDescriptor(P, T::ClassInfo);}
    TAutoObject() : P(0) {}
    TAutoObject(T* p) {P=p;}
    TAutoObject(T& r) {P=&r;}
    TAutoObject(IDispatch& dr);
    operator T*() {return P;}
    operator T&() {return *P;}
  protected:
    T* P;
    T F();// only purpose to remove const for typeid incase T is a const class
};

template<class T>
TAutoObject<T>::TAutoObject(IDispatch& ifc)
{
  TServedObject* obj;
  if (ifc.QueryInterface(IID_TServedObject, (void**)&obj) != 0)
    throw TXAuto(TXAuto::xForeignIDispatch);
  P = (T*)DynamicCast(obj->Object, obj->Class->GetTypeInfo(), typeid(T));
  if (!P)
    throw TXAuto(TXAuto::xTypeMismatch);
}

template<class T> void
TAutoObject<T>::operator=(IDispatch& ifc)
{
  TServedObject* obj;
  if (ifc.QueryInterface(IID_TServedObject, (void**)&obj) != 0)
    throw TXAuto(TXAuto::xForeignIDispatch);
  P = (T*)DynamicCast(obj->Object, obj->Class->GetTypeInfo(), typeid(T));
  if (!P)
    throw TXAuto(TXAuto::xTypeMismatch);
}

template<class T>      // for returning objects only, no IDispatch constructor
class TAutoObjectDelete : public TAutoObject<T>{
  public:
    void operator=(T* p) {this->P=p;}
    void operator=(T& r) {this->P=&r;}
    TAutoObjectDelete()     : TAutoObject<T>() {}
    TAutoObjectDelete(T* p) : TAutoObject<T>(p) {}
    TAutoObjectDelete(T& r) : TAutoObject<T>(r) {}
    operator TObjectDescriptor()
        {return TObjectDescriptor(this->P,T::ClassInfo, TObjectDescriptor::Delete);}
};

template<class T>      // for returning objects by value
class TAutoObjectByVal : public TAutoObjectDelete<T> {
  public:
    void operator=(T o) {this->P=new T(o);}
    TAutoObjectByVal() : TAutoObjectDelete<T>() {}
    TAutoObjectByVal(T o) : TAutoObjectDelete<T>(new T(o)) {}
};

//----------------------------------------------------------------------------
/// \class TAutoIterator
/// automation collection iterator
//


class _OCFCLASS TAutoIterator : public IEnumVARIANT {
  public:
    // IEnumVARIANT implementation
    HRESULT       _IFUNC QueryInterface(const GUID & iid, void** pif);
    unsigned long _IFUNC AddRef();
    unsigned long _IFUNC Release();
    HRESULT _IFUNC Next(unsigned long count, VARIANT* retvals,
                        unsigned long* retcount);
    HRESULT _IFUNC Skip(unsigned long count);
    HRESULT _IFUNC Reset();
    HRESULT _IFUNC Clone(IEnumVARIANT** retiter);

    // specific implementation required by derived classes
    virtual void           Init()=0;   // reset to first item
    virtual bool           Test()=0;   // test if item exists
    virtual void           Step()=0;   // advance to next item
    virtual void           Return(TAutoVal& v)=0;// convert item to variant
    virtual TAutoIterator* Copy()=0;   // return copy of iterator

    // inline data member accessor functions
    TAutoSymbol* GetSymbol();
    void SetSymbol(TAutoSymbol* sym);
    operator IUnknown*();     // essentially a quick QueryInterface

    virtual ~TAutoIterator();
    TAutoClass* Class;
  protected:
    TAutoSymbol*  Symbol;      // symbol entry generating this command
    TAutoCreator& Creator;    // object to create returned automation objects
    IUnknown*      Owner;          // Release() must be called on destruction
    unsigned      RefCnt;
    owl::TLangId        Lang;
    TAutoIterator(TAutoCreator& creator, IUnknown* owner, owl::TLangId lang); // called from derived
    TAutoIterator(TAutoIterator& copy);  // copy constructor for base class
  };

//____________________________________________________________________________
//
/// \class TAutoProxy
/// client C access proxy, base class
//____________________________________________________________________________

enum AutoCallFlag {
  acMethod         = 0x0001,  ///< method call, same as OLE2
  acPropGet        = 0x0002,  ///< returns property value, same as OLE2
  acPropSet        = 0x0004,  ///< set property value, same as OLE2
  acVoidRet        = 0x8000,  ///< pass NULL for return variant, not OLE2 flag
};

// TAutoProxyArgs is the base class of TAutoArgs.
//
class _OCFCLASS TAutoProxyArgs {
  protected:
    TAutoProxyArgs(int cnt) : Count(cnt) {}

    // Provides the address of the Args member which will be added in
    // the derived TAutoArgs template classes.
    //
    virtual TAutoVal* GetArgs() = 0;

  public:
    operator  TAutoVal&()    {return *GetArgs();}
    operator  VARIANT* ()    {return  (VARIANT* )GetArgs();}
    operator  unsigned int() {return Count;}
    TAutoVal& operator[](int index);

  private:
    int       Count;
};

/// \class TAutoArgs
// ~~~~~~~~~~~~~~~
/// The first TAutoVal item of TAutoArgs holds the return value. Items at
/// indices 1 through N+1 represent the first, second, third ... paramters.
/// Of course, these parameters are optional.
//
template <int N>
struct TAutoArgs : public TAutoProxyArgs {
  TAutoVal Args[N+1];
  TAutoArgs() : TAutoProxyArgs(N) {}

  protected:

    // Provides the address of Args.
    // Implemented as a (pure) virtual function to allow access to the Args
    // member also for TAutoArgs objects which are passed as TAutoProxyArgs.
    //
    virtual TAutoVal* GetArgs() {return Args;}
};


// class TAutoProxy
// ~~~~~ ~~~~~~~~~~
//
class _OCFCLASS TAutoProxy {
  public:
   ~TAutoProxy();

    // Attach to [Detatch from] a dispatch interface
    //
    void        Bind(IUnknown* obj);
    void        Bind(IUnknown& obj);
    void        Bind(const GUID& guid);
    void        Bind(LPCTSTR progid);
    void        Bind(TAutoVal& val);
    void        Bind(IDispatch* obj);
    void        Bind(IDispatch& obj);
    void        Unbind(bool release = true);

    // Check/Querry binding
    //
    void        MustBeBound();
    bool        IsBound()  {return owl::ToBool(That != 0);}

    // Check it there's a running copy of an object which supports IDispatch
    //
    static IDispatch*  GetObject(LPCTSTR progid);

    // Locale support
    //
    void        SetLang(owl::TLangId lang) {Lang = lang;}

    // Access to dispatch interface
    //
    operator    IDispatch*();
    operator    IDispatch&();

    // Retrieve dispId
    //
    long        Lookup(const long id) {return id;}
    long        Lookup(LPCTSTR name);
    void        Lookup(LPCTSTR names, long* ids, unsigned count);

  protected:
    TAutoProxy(owl::TLangId lang) : That(0), Lang(lang) {}

    // Invoke (IDispatch::Invoke)
    //
    TAutoVal&   Invoke(owl::uint16 attr, TAutoProxyArgs& args, long* ids, unsigned named=0);

    IDispatch*    That;
    owl::TLangId        Lang;
};

struct _OCFCLASS TAutoDispId {
  TAutoDispId(TAutoProxy* prx, LPCTSTR name) : Id(prx->Lookup(name)) {}
  TAutoDispId(TAutoProxy*, long id) : Id(id) {}
  operator long*()    {return &Id;}
  operator unsigned() {return   0;}

  long     Id;
};

template <int N> struct TAutoDispIds {
  TAutoDispIds(TAutoProxy* prx, LPCTSTR names) {prx->Lookup(names, Ids);}
  long Ids[N+1];
  operator long*()    {return Ids;}
  operator unsigned() {return N;}
};

class _OCFCLASS TAutoEnumeratorBase {
  public:
    void Bind(TAutoVal& val);
    void Unbind()   { if (Iterator) Iterator->Release(); Iterator=0; Clear();}
    bool Step();
    void Clear()      { Current.Clear(); }
    bool IsValid()    { return Current.GetDataType() != atVoid; }
    void Object(TAutoProxy& prx) { prx.Bind(Current); }
    void operator =(const TAutoEnumeratorBase& copy);
   ~TAutoEnumeratorBase() { Unbind(); }
  protected:
    TAutoEnumeratorBase() : Iterator(0) {Current = TAutoVoid();}
    TAutoEnumeratorBase(const TAutoEnumeratorBase& copy);
    IEnumVARIANT*     Iterator;
    TAutoVal          Current;
};

template <class T>
class TAutoEnumerator : public TAutoEnumeratorBase {
  public:
    TAutoEnumerator() : TAutoEnumeratorBase() {}
    TAutoEnumerator(const TAutoEnumerator<T>& cpy) : TAutoEnumeratorBase(cpy) {}
    void Value(T& v);
    operator T();
};

//____________________________________________________________________________
//
// Inline implementations for automation controller
//____________________________________________________________________________

//
//
inline TAutoVal& TAutoProxyArgs::operator[](int index)
{
  // NOTE: It's OK to use Count as subscript since there's always one
  //       addition TAutoVal/VARIANT for the return value.
  //
  PRECONDITION(index <= Count);

  // NOTE: The logic below takes care of the difference in the C and
  //       Automation calling conventions.
  //
  return *(GetArgs()+(index ? Count+1-index : 0));
}

//
//
inline TAutoProxy::operator IDispatch&()
{
  // Following conditional avoid function call overhead - specially since
  // method is inline
  //
  if (!That)
    MustBeBound();
  return *That;
}

//
//
inline TAutoProxy::operator IDispatch*()
{
  if (That)
    That->AddRef();
  return That;
}

//
//
inline void TAutoProxy::Unbind(bool release)
{
  if (release && That)
    That->Release();
  That = 0;
}

//
//
inline TAutoProxy::~TAutoProxy()
{
  Unbind();
}

template<>
inline void TAutoEnumerator<short>::Value(short& v) { v = Current; }

template<>
inline void TAutoEnumerator<long>::Value(long& v)   { v = Current; }

template<>
inline void TAutoEnumerator<bool>::Value(bool& v)   { v = Current; }

template<>
inline void TAutoEnumerator<TAutoString>::Value(TAutoString& v)
                                                    { v = Current; }
template<>
inline TAutoEnumerator<short>::operator short() { return (short)Current; }

template<>
inline TAutoEnumerator<long>::operator long()   { return (long)Current; }

template<>
inline TAutoEnumerator<bool>::operator bool()   { return (bool)Current; }

template<>
inline TAutoEnumerator<TAutoString>::operator TAutoString()
                                                { return (TAutoString)Current; }
//____________________________________________________________________________
//
// Inline implementations for automation server - part 1
//____________________________________________________________________________

// TAutoClass inlines

inline TAutoSymbol* TAutoClass::GetTable() const {
  return Table;
}
inline TAutoSymbol* TAutoClass::GetClassSymbol() const {
  return ClassSymbol;
}
inline TAutoCommandBuildDtr TAutoClass::GetDestructor() const {
  return ClassSymbol->BuildDtr;
}
inline const std::type_info&  TAutoClass::GetTypeInfo() const {
  return TypeInfo;
}
inline LPCTSTR TAutoClass::GetName(owl::TLangId id) const {
  return ClassSymbol->Name.Translate(id);
}
inline LPCTSTR TAutoClass::GetDoc (owl::TLangId id) const {
  return ClassSymbol->Doc.Translate(id);
}
inline unsigned long TAutoClass::GetHelpId() const {
  return ClassSymbol->HelpId;
}
inline unsigned short TAutoClass::GetTypeFlags() const {
  return ClassSymbol->TypeFlags & tfAutoClassMask;
}
inline int TAutoClass::GetImplTypeFlags() const {
  return ClassSymbol->TypeFlags>>12;
}
inline unsigned short TAutoClass::GetCoClassFlags() const {
  return ClassSymbol->TypeFlags & tfCoClassXfer;
}
inline TAggregator TAutoClass::GetAggregator() const {
  return Aggregator;
}
inline IUnknown& TAutoClass::Aggregate(ObjectPtr obj, TUnknown& inner) {
  return obj && Aggregator ? Aggregator(obj, inner) : (IUnknown&)inner;
}

//____________________________________________________________________________

// TAutoSymbol inlines

inline void TAutoSymbol::SetFlag(owl::uint16 mask) {
 Attr |= mask;
}
inline void TAutoSymbol::ClearFlag(owl::uint16 mask) {
  Attr &= owl::uint16(~mask);
}
inline bool TAutoSymbol::TestFlag(owl::uint16 mask) const {
  return (Attr&mask)? true : false;
}
inline owl::uint16 TAutoSymbol::GetFlags() const {
  return (owl::uint16)Attr;
}
inline bool TAutoSymbol::IsEnum() const {
  return (Type->GetType() & atEnum)? true : false;
}
inline bool TAutoSymbol::IsByRef() const {
  return (Type->GetType() & atByRef)? true:false;
}
inline bool TAutoSymbol::IsArray() const {
  return (Type->GetType() & atSafeArray)? true : false;
}
inline bool TAutoSymbol::IsIterator() const {
  return ((Attr&asIterator)==asIterator) ? true : false;
}
inline bool TAutoSymbol::IsTerminator() const {
  return ((Attr & asNotTerminator)== 0) ? true : false;
}
inline owl::uint16 TAutoSymbol::GetDataType() const {
  return owl::uint16(Type->GetType() & atTypeMask);
}
inline TAutoClass* TAutoSymbol::GetClass() const {
  return (Type->GetType() & atAutoClass)? (TAutoClass*)Type : 0;
}
inline TAutoEnum* TAutoSymbol::GetEnum() const {
  return (Type->GetType() & atEnum) ? (TAutoEnum*)Type : 0;
}
inline TAutoCommandBuild TAutoSymbol::InitTypeConvert(TAutoSymTypeConvert f)
                                         { return (TAutoCommandBuild)f;}
inline TAutoCommandBuild TAutoSymbol::InitAutoIterator(TAutoIteratorBuild f)
                                         { return (TAutoCommandBuild)f;}
inline TAutoCommandBuild TAutoSymbol::InitAutoDestructor(TAutoCommandBuildDtr f)
                                         { return (TAutoCommandBuild)f;}
//____________________________________________________________________________
// TAutoString
inline TAutoString::TAutoString(TAutoVal& val)
:
  owl::TString(val.operator owl::TString())
//#if defined(BI_COMP_MSC)
//  TString((TString)val.operator TString())
//#else
//  TString((TString)val)
//#endif
{
}
inline TAutoString& TAutoString::operator =(TAutoVal& val){
  --*S; S = val; return *this;
}

//____________________________________________________________________________
// TAutoVal inlines

//
//
inline TAutoVal::TAutoVal()
{
  Init();
}

//
//
inline TAutoVal::~TAutoVal()
{
  Clear();
}

//
//
inline TAutoVal::TAutoVal(const TAutoVal& src)
{
  Init();
  ::VariantCopy((VARIANT*)this, (VARIANT*)&src);
}

//
//
inline const TAutoVal& TAutoVal::operator=(const TAutoVal& src)
{
  // NOTE: 'VariantCopy' takes care of 'freeing' the destination
  //
  ::VariantCopy((VARIANT*)this, (VARIANT*)&src);
  return *this;
}

inline TAutoVal::operator unsigned long(){return (unsigned long)operator long();}
//#if (MAXINT==MAXSHORT)
//inline TAutoVal::operator int()          {return operator short();}
//inline TAutoVal::operator unsigned int() {return operator unsigned short();}
//inline TAutoVal::operator int*()         {return (int*)operator short*();}
//#else
inline TAutoVal::operator int()          {return operator long(); }
inline TAutoVal::operator unsigned int() {return (unsigned long)(long)*this;}
inline TAutoVal::operator int*()         {return (int*)operator long*();}
//#endif

inline TAutoVal::operator signed char()  {return operator unsigned char();}
inline TAutoVal::operator char()         {return operator unsigned char();}
inline TAutoVal::operator char*()        {return (char*)operator unsigned char*();}
inline TAutoVal::operator signed char*() {return (signed char*)operator unsigned char*();}

inline void TAutoVal::operator =(IUnknown*     ifc){vt=atUnknown; punkVal=ifc;}
inline void TAutoVal::operator =(IDispatch*    ifc){vt=atObject; pdispVal=ifc;}

inline void TAutoVal::operator =(IPictureDisp* ifc){vt=atObject; pdispVal=ifc;}
inline void TAutoVal::operator =(IFontDisp*    ifc){vt=atObject; pdispVal=ifc;}

inline void TAutoVal::operator =(VARIANT&        v){vt=atVariant; pvarVal= &v;}
inline void TAutoVal::operator =(unsigned char   v){vt=atByte;   bVal  = v;}
inline void TAutoVal::operator =(signed   char   v){vt=atByte;   bVal  = v;}
inline void TAutoVal::operator =(char            v){vt=atByte;   bVal  = v;}
inline void TAutoVal::operator =(short           v){vt=atShort;  iVal  = v;}
inline void TAutoVal::operator =(long            v){vt=atLong;   lVal  = v;}
inline void TAutoVal::operator =(float           v){vt=atFloat;  fltVal= v;}
inline void TAutoVal::operator =(double          v){vt=atDouble; dblVal= v;}
inline void TAutoVal::operator =(owl::TBool           v){vt=atBool;   boolVal = short(v ? -1 : 0);}
inline void TAutoVal::operator =(TAutoDate     v){vt=atDatetime; date = v;}
inline void TAutoVal::operator =(TAutoCurrency v){vt=atCurrency; cyVal = v;}
inline void TAutoVal::operator =(TAutoVoid      ){vt=atVoid;}
inline void TAutoVal::operator =(const TNoArg&  ){vt=atError; scode=DISP_E_PARAMNOTFOUND;}

inline void TAutoVal::operator =(unsigned char* p){vt=atByte+atByRef; pbVal=p;}
inline void TAutoVal::operator =(signed char*   p){vt=atByte+atByRef; pbVal=(unsigned char*) p;}
inline void TAutoVal::operator =(char*          p){vt=atByte+atByRef; pbVal=(unsigned char*) p;}
inline void TAutoVal::operator =(short*         p){vt=atShort+atByRef; piVal=p;}
inline void TAutoVal::operator =(long*          p){vt=atLong+atByRef;  plVal=p;}
inline void TAutoVal::operator =(owl::TBool*         p){vt=atBool+atByRef;  pbool=(VARIANT_BOOL*)p;}
inline void TAutoVal::operator =(float*         p){vt=atFloat+atByRef; pfltVal=p;}
inline void TAutoVal::operator =(double*        p){vt=atDouble+atByRef;pdblVal=p;}
inline void TAutoVal::operator =(void*          p){vt=atVoid+atByRef;byref=p;}
inline void TAutoVal::operator =(TAutoDate*     p){vt=atDatetime+atByRef;pdate=(double*)p;}
inline void TAutoVal::operator =(TAutoCurrency* p){vt=atCurrency+atByRef;pcyVal=p;}

inline void TAutoVal::operator =(unsigned short  v){vt=atLong; lVal = (long)v;}
inline void TAutoVal::operator =(unsigned short* p){vt=atShort+atByRef; piVal = (short*)p;}
inline void TAutoVal::operator =(unsigned long   v){vt=atLong; lVal = (long)v;}
inline void TAutoVal::operator =(unsigned long*  p){vt=atLong+atByRef; plVal = (long*)p;}
//#if (MAXINT==MAXSHORT)
//inline void TAutoVal::operator =(int           v){vt=atShort; iVal=v;}
//inline void TAutoVal::operator =(int*          p){vt=atShort+atByRef; piVal=(short*)p;}
//inline void TAutoVal::operator =(unsigned int  v){operator =((unsigned short)v);}
//inline void TAutoVal::operator =(unsigned int* p){operator =((unsigned short*)p);}
//#else
inline void TAutoVal::operator =(int           v){vt=atLong; lVal=v;}
inline void TAutoVal::operator =(int*          p){vt=atLong+atByRef; plVal = (long*)p;}
inline void TAutoVal::operator =(unsigned int  v){operator =((unsigned long)v);}
inline void TAutoVal::operator =(unsigned int* p){operator =((unsigned long*)p);}
//#endif

// The following operators are added to complement the functionality of TAutoVal
inline void TAutoVal::operator =(BSTR*          p){vt=atString+atByRef; pbstrVal=p;}
inline void TAutoVal::operator =(TBSTR*         p){vt=atString+atByRef; pbstrVal=(BSTR*)p;}
inline void TAutoVal::operator =(IUnknown**     p){vt=atUnknown+atByRef; ppunkVal=p;}
inline void TAutoVal::operator =(IDispatch**    p){vt=atObject+atByRef; ppdispVal=p;}
inline void TAutoVal::operator =(TAutoVal*      p){vt=atVariant+atByRef; pvarVal=(VARIANT*)p;}
inline void TAutoVal::operator =(LARGE_INTEGER  v){vt=VT_I8; hVal=v;}
inline void TAutoVal::operator =(ULARGE_INTEGER v){vt=VT_UI8; uhVal=v;}
inline void TAutoVal::operator =(SAFEARRAY* p) { SafeArrayGetVartype(p, &vt); vt |= VT_ARRAY; parray = p; }
#if defined(_WIN64)
inline void TAutoVal::operator =(ULONG_PTR      v){vt=VT_UI8; uptrVal=v;}
#endif
//
//

inline void TAutoVal::operator =(TObjectDescriptor od)
{
  vt = atObjectDesc;
  ObjDesc.Object= od.Object;
  ObjDesc.Class = od.Class;
  if (od.Destruct == TObjectDescriptor::Delete)
    vt |= atByRef;
}

//
//
inline int TAutoVal::GetDataType() const {
  return vt;
}

//
//
inline void TAutoVal::Init()
{
  memset(this, 0, sizeof(*this));
}

//
//
inline void TAutoVal::Clear()
{
  if ((vt & ~atByRef) == atLoanedBSTR) {
    s.Holder->ReleaseBstr((vt & atByRef) ? *pbstrVal : bstrVal);
    vt = owl::uint16(atString | (vt & atByRef));
  }
  ::VariantClear((VARIANT*)this);
}

//
//
inline void TAutoVal::Restore()
{
  if ((vt & ~atByRef) == atLoanedBSTR) {
    s.Holder->RevokeBstr((vt & atByRef) ? *pbstrVal : bstrVal);
    vt = owl::uint16(atString | (vt & atByRef));
  } else if (vt == owl::uint16(atBool | atByRef) && *pbool != 0) {
    *pbool = -1;
  }

  // Need to check if atObjectDesc? Should never happen as it is temporary
}

//
//
inline void TAutoVal::Copy(const TAutoVal& copy)
{
  vt = copy.vt;
  p  = copy.p;
  if (vt == atUnknown || vt == atObject)
    punkVal->AddRef();
  if (vt == atString)
    bstrVal = ::SysAllocString(bstrVal);
}

/// Converts TAutoVal/VARIANT to another type
/// \note Converts in-place if source is not specified
//
inline HRESULT TAutoVal::ChangeType(VARTYPE varType, TAutoVal* src) {
   return ::VariantChangeType((VARIANT*)this, src ? (VARIANT*)src : (VARIANT*)this, 0, varType);
}

/// Is TAutoVal Object (i.e. VARIANT) passing data by reference
//
inline bool TAutoVal::IsRef() const {
  return (vt & atByRef) ? true : false;
}

//
//
inline TAutoVal* TAutoVal::DereferenceVariant() {
  return vt==atVariant+atByRef ? (TAutoVal*)pvarVal : this;
}

//
//
inline owl::TString TAutoVal::StrVal() {
  return vt==atString ? bstrVal : 0;
}

//
//
inline bool TAutoVal::GetObjDesc(TObjectDescriptor& od)
{
  if ((char)vt != atObjectDesc)
    return false;
  od.Object = ObjDesc.Object;
  od.Class  = ObjDesc.Class;
  od.Destruct = IsRef() ? TObjectDescriptor::Delete : TObjectDescriptor::Quiet;
  return true;
}

//
//
inline SAFEARRAY* TAutoVal::GetArray()
{
  if ((vt & atSafeArray) == 0)
  {
    return 0;
  }

  return (vt & atByRef ? *pparray : parray);

/*
  if (vt == atSafeArray)
    return parray;
  if (vt == (atSafeArray|atByRef))
    return *pparray;
  return 0;
*/
}

//
//
inline void* TAutoVal::SetByRef(AutoDataType datatype)
{
  if (vt == atVariant)
    return this;
  vt = owl::uint16(datatype);
  return &byref;
}

//____________________________________________________________________________
// TServedObject inlines

inline TServedObject::operator IDispatch*()
{
  AddRef();
  return (IDispatch*)this;  // essentially a quick QueryInterface on the obj
}

// TDispatch inlines

inline TDispatch::operator IDispatch*()
{
  AddRef();
  return (IDispatch*)this;  // essentially a quick QueryInterface on the obj
}

//____________________________________________________________________________

// TAutoCommand inlines

inline TAutoCommand::TAutoCommand(int attr) : Attr(attr), Symbol(0) {}
inline void TAutoCommand::SetFlag(int mask) {Attr |= mask;}
inline void TAutoCommand::ClearFlag(int mask) {Attr&=(~mask);}
inline bool TAutoCommand::TestFlag(int mask) {return owl::ToBool((Attr&mask)==mask);}
inline bool TAutoCommand::IsPropSet() {return TestFlag(asSet);}
inline TAutoSymbol* TAutoCommand::GetSymbol() {return Symbol;}
inline void TAutoCommand::SetSymbol(TAutoSymbol* sym) {Symbol = sym;}
inline LPCTSTR TAutoCommand::LookupError(long errCode)
    {return TAutoCommand_ErrorLookup ? TAutoCommand_ErrorLookup(errCode) : 0;}
//____________________________________________________________________________

// TAutoEnum inlines and template function definitions

inline TAutoEnum::TAutoEnum(int count, int type)
                        : Count(count) { Type = short(type + atEnum); }

template <class T> inline
TAutoEnumT<T>::TAutoEnumT(TAutoEnumVal<T>* table, int symcount, int type)
: TAutoEnum(symcount, type), Table(table) {}

// incoming enumeration string localized lookup
//
template <class T> bool
TAutoEnumT<T>::Convert(TAutoVal& txtVal, TAutoVal& numVal)
{
  const owl::TString& stringRef = txtVal.StrVal();  // 0 if not string type
  const _TCHAR* str = stringRef;
  if (str) {
    owl::TLangId langId = txtVal.GetLanguage();
    for (int i = 0; i < Count; i++) {
      if (Table[i].Name.Compare(str, langId) == 0) {
        numVal = Table[i].Val;
        return true;
      }
    }
  }
  return false;
}

// outgoing enumeration translation to localized string
//
template <class T> bool
TAutoEnumT<T>::Convert(TAutoVal& numVal, owl::TLangId langId)
{
  T val;
  val = numVal;  // perhaps we should put a try/catch here!!
  for (int i = 0; i < Count; i++) {
    if (Table[i].Val == val) {
      numVal = Table[i].Name.Translate(langId);
      numVal.SetLocale(langId);
      return true;
    }
  }
  return false;
}

// special case for outgoing string enumerations
//
/// BGM: PRON 1325 (compiler bug) prevents this \#pragma for working
/// with some templatized functions.  Deferred for Ebony.
#pragma warn -inl
template<>
inline bool
TAutoEnumT<LPCTSTR>::Convert(TAutoVal& numVal, owl::TLangId langId)
{
  LPCTSTR str = numVal.StrVal();  // 0 if not string type
  if (str)
    for (int i = 0; i < Count; i++) {
      if (lstrcmp(Table[i].Val, str) == 0) {
        numVal = Table[i].Name.Translate(langId);
        numVal.SetLocale(langId);
        return true;
      }
    }
  return false;
}
#pragma warn .inl

//____________________________________________________________________________

// TAutoIterator inlines

inline TAutoIterator::operator IUnknown*() {RefCnt++; return this;}
inline TAutoSymbol* TAutoIterator::GetSymbol() {return Symbol;}
inline void TAutoIterator::SetSymbol(TAutoSymbol* sym) {Symbol = sym;}

//____________________________________________________________________________
//
// Inline implementations for automation server - part 2
//   These are inline only for the purpose of allowing _AUTOCLASS
//   to be specified at compile time without rebuilding OCF library
//   This applies to classes: TAutoBase, TAutoCommand, TAutoDetach
//____________________________________________________________________________

inline TAutoBase::~TAutoBase()
{
  ::ocf::SendObituary(this, typeid(TAutoBase));
}

inline void TAutoDetach::Notify(int offset, const std::type_info& typeInfo)
{
  ::ocf::SendObituary((char*)this-offset, typeInfo);
}
//____________________________________________________________________________

// TAutoCommand implementation specified inline to avoid _AUTOCLASS link errors

inline void TAutoCommand::Fail(TXAuto::TError err) { throw TXAuto(err); }

inline bool TAutoCommand::Validate() { return true; }

inline TAutoCommand& TAutoCommand::Invoke()
{
  if (!TAutoCommand_InvokeHook || TAutoCommand_InvokeHook(*this))
    Execute();
  return *this;
}

inline void TAutoCommand::Execute()
{
}

inline long TAutoCommand::Report()
{
  return 0;
}

inline void TAutoCommand::Return(TAutoVal& v)
{
  v = TAutoVoid();
}

inline void TAutoCommand::Transfer(TAutoTransfer& x)
{
}

inline TAutoCommand* TAutoCommand::Undo() {return 0;}

inline int TAutoCommand::Record(TAutoStack&) {return 0;}

inline TAutoCommand::TErrorMsgHook
TAutoCommand::SetErrorMsgHook(TAutoCommand::TErrorMsgHook callback){
  TErrorMsgHook hook = TAutoCommand_ErrorLookup;
  TAutoCommand_ErrorLookup = callback;
  return hook;
}

inline TAutoCommand::TCommandHook
TAutoCommand::SetCommandHook(TAutoCommand::TCommandHook callback){
  TCommandHook hook = TAutoCommand_InvokeHook;
  TAutoCommand_InvokeHook = callback;
  return hook;
}

#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif


} // OCF namespace


#endif  // OCF_AUTODEFS_H



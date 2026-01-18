//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Low level OLE Utility class definitions
//----------------------------------------------------------------------------

#if !defined(OCF_OLEUTIL_H)
#define OCF_OLEUTIL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <ocf/except.h>

#include <owl/pointer.h>

#include <owl/string.h>


#if !defined(_ICLASS)
# define _ICLASS _OCFCLASS
#endif

#define _IFUNC STDMETHODCALLTYPE


namespace owl {class _OWLCLASS TRegList;};

namespace ocf {

#if 1

//
/// BASIC global string encapsulation
//
typedef owl::TSysStr TBSTR;

#else

//
/// \class TBSTR
// ~~~~~ ~~~~~
/// BASIC global string encapsulation
//
class TBSTR {
  public:
    TBSTR() : P(0) {}
    TBSTR(BSTR p) : P(p) {}
    // NOTE: OK to pass 0 to 'SysFreeString'
    //
   ~TBSTR(){ ::SysFreeString(P); }


     // Operators
    //
    BSTR      operator =(BSTR p)        { ::SysFreeString(P);  return P = p; }
    TBSTR&    operator =(const TBSTR& p){::SysFreeString(P); P = p.P; return *this;}
              operator  BSTR()          { return P;}
              operator  BSTR*()          { if(P) ::SysFreeString(P); return &P;}
    int       operator !() const        { return P==0;}
    void      operator ~()              { ::SysFreeString(P);  P = 0; }

  protected:
    BSTR      P;

  private:

    // Prohibit use of new/delete operators
    //
    void* operator new(size_t) {return 0;}
    void  operator delete(void*) {}
};
#endif



class _OCFCLASS  TOleAutoDll
{
  public:
    static HRESULT SafeArrayGetVartype(SAFEARRAY * psa, VARTYPE * pvt);
};

inline HRESULT
TOleAutoDll::SafeArrayGetVartype(SAFEARRAY * psa, VARTYPE * pvt)
{
  return ::SafeArrayGetVartype(psa,pvt);
}




// Conversion routines
//
void OcHiMetricToPixel(const SIZEL& sizeHM, SIZEL& sizePix);
void OcPixelToHiMetric(const SIZEL& sizePix, SIZEL& sizeHM);

//
/// \class TOleAllocator
// ~~~~~ ~~~~~~~~~~~~~
/// Object to initialize OLE and access memory manager
//
class TOleAllocator {
  public:
    TOleAllocator();             // Gets allocator, does not initialize OLE
    TOleAllocator(IMalloc* mem); // Use 0 to initialize with OLE task allocator
   ~TOleAllocator();
    void*     Alloc(unsigned long size);
    void      Free(void * pv);
    IMalloc*  Mem;
  private:
    bool      Initialized;
};

//
//
//
#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
inline
TOleAllocator::TOleAllocator(IMalloc* mem) : Initialized(true)
{
  OLECALL(OleInitialize(mem), _T("OleInitialize"));
  OLECALL(CoGetMalloc(MEMCTX_TASK, &Mem), _T("CoGetMalloc"));
}

//
//
inline
TOleAllocator::TOleAllocator() : Mem(0), Initialized(false)
{
  OLECALL(CoGetMalloc(MEMCTX_TASK, &Mem), _T("CoGetMalloc"));
}

//
//
//
inline
TOleAllocator::~TOleAllocator()
{
  if (Mem)
    Mem->Release();
  Mem = 0;
  if (Initialized)
    ::OleUninitialize();
}

//
/// \class TOleInit
// ~~~~~ ~~~~~~~~
/// Simple wrapper of TOleAllocator used exclusively for initializing OLE
//
class TOleInit : public TOleAllocator {
  public:
    TOleInit() : TOleAllocator(0){};
};

// To maintain compatibility with initial release of v5.0 of OCF
//
typedef TOleInit  TOleStart;

//
/// \class TClassId
// ~~~~~ ~~~~~~~~
/// GUID/IID/CLSID management
//
class  TClassId {
  public:
    enum TGenerate {
      Generate
    };
    TClassId(const GUID& guid);
    TClassId(LPCTSTR idtxt);
    TClassId(const TClassId& copy);
    TClassId(TGenerate);
    TClassId& operator =(const TClassId& copy);
    TClassId(const GUID & guid, int offset);
   ~TClassId();
    operator GUID&();
    operator LPCTSTR();

  protected:
    TClassId();
    GUID  Guid;
    LPCTSTR Text;
};

//
//
inline
TClassId::TClassId(const GUID& guid) : Text(0) {
  Guid = guid;
}

//
//
inline
TClassId::TClassId(const TClassId& copy) : Guid(copy.Guid), Text(0)
{
}

/// Default constructor - Derived class must set GUID
//
inline
TClassId::TClassId() : Text(0)
{
}

//
//
inline
TClassId::operator GUID&()
{
  return Guid;
}

//
//
inline TClassId&
TClassId::operator =(const TClassId& copy)
{
  Guid = copy.Guid;
  Text=0;
  return *this;
}

//
/// \class TBaseClassId
// ~~~~~ ~~~~~~~~~~~~
/// Base GUID for a components sequence of GUIDs
//
class TBaseClassId : public TClassId {
  public:
    TBaseClassId(owl::TRegList& regInfo, int maxAlloc = 256);
    TBaseClassId(const GUID& guid, int maxAlloc = 256);
    TBaseClassId(LPCTSTR idtxt, int maxAlloc = 256);
    TBaseClassId(TGenerate, int maxAlloc = 256);
    TClassId operator [](int offset);
    int   AllocId();
    int   GetOffset(const GUID & guid); // returns -1 if not within range

  private:
    int   Offset;  // Last offset given out by AllocId(), creator reserves 0
    int   MaxAlloc;
};

//
// Fast GUID compare functions, assume that low word has already matched OK
//
void CmpGuid12 (IUnknown* This, REFIID req, REFIID ref, void** pif);
void CmpGuidOle(IUnknown* This, REFIID req, void** pif);

//
/// \class TUnknown
// ~~~~~ ~~~~~~~~
/// Standard implementation of a controlling IUnknown for an object, to be
/// inherited with other COM interfaces into an implementation class whose
/// IUnknown implementation delegates to TUnknown::Outer.
//
class _ICLASS TUnknown {
  public:
    operator      IUnknown&() {return *Outer;}
    operator      IUnknown*() {Outer->AddRef(); return Outer;}
    IUnknown*     SetOuter(IUnknown* outer = 0);
    IUnknown*     GetOuter() {return Outer;}
    unsigned long GetRefCount();
    unsigned long AdjustRefCount(int i);
    IUnknown&     Aggregate(TUnknown& inner);  // returns outermost IUnknown

  protected:
    TUnknown();
    virtual ~TUnknown();

    IUnknown&       ThisUnknown() {return I;}
    IUnknown*       Outer;
    virtual HRESULT QueryObject(const GUID& iid, void** pif);

  private:
    class _ICLASS TUnknownI : public IUnknown {
      public:
        HRESULT       _IFUNC QueryInterface(const GUID & iid, void** pif);
        unsigned long _IFUNC AddRef();
        unsigned long _IFUNC Release();
        TUnknownI() : RefCnt(0), Inner(0) {}
       ~TUnknownI();
        unsigned  RefCnt;  ///< Object's reference count, remains 1 if aggregated
        TUnknown* Inner;   ///< Chain of TUnknown-aggregated objects
      private:
        TUnknown& Host();
    } I;
  friend class TUnknownI;
};

inline IUnknown* TUnknown::SetOuter(IUnknown* outer)
{
  if (outer) {
    Outer = outer;
    if (I.Inner)
      I.Inner->SetOuter(outer);
  }
  return &I;
}

// is this portable?????????
inline TUnknown& TUnknown::TUnknownI::Host()
{
  return *(TUnknown*)((char*)this-(int)(&((TUnknown*)0)->I));
}

inline unsigned long TUnknown::AdjustRefCount(int i)
{
  return (unsigned long)(I.RefCnt += i);
}
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif


//------------------------------------------------------------------------
//
// Macros to generate COM mixin classes for use with TUnknown
//      Each base class must have an inline QueryInterface wrapper defined.
//
// DECLARE_COMBASES1(classname, base)    // declares support for 1 interface
// DECLARE_COMBASES2(classname, base, base)      // mixes in for 2 interfaces
//  .......
// DEFINE_COMBASES1(classname, base)  // implements IUnknown for 1 interface
// DEFINE_COMBASES2(classname, base, base)   // implements for 2 interfaces
//  ......
//
// Macros to generate inline QueryInterface wrappers for interfaces/delegates
//    (low GUID word may specify an explicit integer or an IID_whatever.Data1)
//    For efficiency they are void functions; check result pointer for success
//
// DEFINE_QI_BASE(cls, low)      // Inherited COM interface with IUnknown base
// DEFINE_QI_OLEBASE(cls, low)   // Inherited OLE interface using an OLE GUID
// DEFINE_QI_DERIVED(cls, base, low)     // COM interface derived from another
// DEFINE_QI_OLEDERIVED(cls, base, low)  // OLE interface derived from another
// DEFINE_QI_DELEGATE(cls, member)       // Inherited object with COM member
// DEFINE_QI_DELEGATE_TEST(cls, member)  // Object whose COM member may be 0
// DEFINE_QI_MEMBER(name, member)        // Used inside object with COM member
// DEFINE_QI_MEMBER_TEST(name, member)   // Where object's COM member may be 0
//
// Macro to invoke inline QueryInterface wrappers and return if successful
//    A list of these is generated by the DEFINE_COMBASESn(...) macro
//
// COMBASEQUERY(name) // Calls name_QueryInterface(), returns OK if success
//
// Macro to provide standard IUnknown implementation which forwards to Outer
//    This macro is automatically invoked by the DEFINE_COMBASESn(...) macro
//
// IUNKNOWN_FORWARD(cls, outer) // cls IUnknown implementation forwards to outer
//
// Example of user-written COM class inheriting from macro-generated mixin
//
//  class TSample : public classname {
//   public:
//    TSample(IUnknown* outer, IUnknown** pif) { *pif = SetOuter(outer); }
//  };
//
//------------------------------------------------------------------------

#define COMBASES_D(cls, bases)     \
  class cls : public TUnknown, bases { \
  protected:                        \
    virtual HRESULT _IFUNC QueryInterface(const GUID & iid, void ** pif); \
    virtual unsigned long _IFUNC AddRef();  \
    virtual unsigned long _IFUNC Release(); \
    virtual HRESULT QueryObject(const GUID & iid, void ** pif); \
            HRESULT QueryBases(const GUID & iid, void ** pif) \
                               {return cls::QueryObject(iid, pif);} \
  }; // QueryBases() is an inline wrapper to this QueryObject() implementation

#define COMQRY1BASES(i1)             public i1
#define COMQRY2BASES(i1,i2)          public i1, COMQRY1BASES(i2)
#define COMQRY3BASES(i1,i2,i3)       public i1, COMQRY2BASES(i2,i3)
#define COMQRY4BASES(i1,i2,i3,i4)    public i1, COMQRY3BASES(i2,i3,i4)
#define COMQRY5BASES(i1,i2,i3,i4,i5) public i1, COMQRY4BASES(i2,i3,i4,i5)

#define DECLARE_COMBASES1(cls,i1) \
  COMBASES_D(cls,COMQRY1BASES(i1))
#define DECLARE_COMBASES2(cls,i1,i2) \
  COMBASES_D(cls,COMQRY2BASES(i1,i2))
#define DECLARE_COMBASES3(cls,i1,i2,i3) \
  COMBASES_D(cls,COMQRY3BASES(i1,i2,i3))
#define DECLARE_COMBASES4(cls,i1,i2,i3,i4) \
  COMBASES_D(cls,COMQRY4BASES(i1,i2,i3,i4))
#define DECLARE_COMBASES5(cls,i1,i2,i3,i4,i5) \
  COMBASES_D(cls,COMQRY5BASES(i1,i2,i3,i4,i5))

// Macro which can be used by classes which inherit from TUnknown for
// IUnknown methods support - This macro defines the methods inline
//
#define TUNKNOWN_STANDARD_IUNKNOWN_METHODS \
    owl::ulong   _IFUNC   AddRef()       {return GetOuter()->AddRef();}    \
    owl::ulong   _IFUNC   Release()      {return GetOuter()->Release();}   \
    HRESULT _IFUNC   QueryInterface(const GUID& iid, void** iface)    \
                     {return GetOuter()->QueryInterface(iid, iface);}


// Macro which can be used by classes which inherit from TUnknown for
// IUnknown methods support - This macros defines the methods out-of-line
//
#define IUNKNOWN_FORWARD(cls, outer) \
  HRESULT _IFUNC cls::QueryInterface(const GUID & iid, void * * pif)\
                                 { return outer->QueryInterface(iid, pif); } \
  owl::ulong _IFUNC cls::AddRef()  { return outer->AddRef(); } \
  owl::ulong _IFUNC cls::Release() { return outer->Release(); }

#define COMBASES_I(cls, tests) \
  IUNKNOWN_FORWARD(cls, Outer) \
  HRESULT cls::QueryObject(const GUID & iid, void ** pif) \
         { *pif = 0; tests return ResultFromScode(E_NOINTERFACE); }

#define COMBASEQUERY(cls) \
  cls##_QueryInterface(this,iid,pif); if(*pif) return NOERROR;

#define COMQRY1TEST(i1)             COMBASEQUERY(i1)
#define COMQRY2TEST(i1,i2)          COMBASEQUERY(i1) COMQRY1TEST(i2)
#define COMQRY3TEST(i1,i2,i3)       COMBASEQUERY(i1) COMQRY2TEST(i2,i3)
#define COMQRY4TEST(i1,i2,i3,i4)    COMBASEQUERY(i1) COMQRY3TEST(i2,i3,i4)
#define COMQRY5TEST(i1,i2,i3,i4,i5) COMBASEQUERY(i1) COMQRY3TEST(i2,i3,i4,i5)

#define DEFINE_COMBASES1(cls,i1) \
  COMBASES_I(cls,COMQRY1TEST(i1))
#define DEFINE_COMBASES2(cls,i1,i2) \
  COMBASES_I(cls,COMQRY2TEST(i1,i2))
#define DEFINE_COMBASES3(cls,i1,i2,i3) \
  COMBASES_I(cls,COMQRY3TEST(i1,i2,i3))
#define DEFINE_COMBASES4(cls,i1,i2,i3,i4) \
  COMBASES_I(cls,COMQRY4TEST(i1,i2,i3,i4))
#define DEFINE_COMBASES5(cls,i1,i2,i3,i4,i5) \
  COMBASES_I(cls,COMQRY5TEST(i1,i2,i3,i4,i5))

#define DEFINE_QI_BASE(cls, low) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {if (iid.Data1==low) CmpGuid12(obj, iid, IID_##cls, pif);}

#define DEFINE_QI_OLEBASE(cls, low) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {if (iid.Data1==low) CmpGuidOle(obj, iid, pif);}

#define DEFINE_QI_DERIVED(cls, base, low) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {if (iid.Data1==low) CmpGuid12(obj, iid, IID_##cls, pif); \
  else base##_QueryInterface(obj, iid, pif);}

#define DEFINE_QI_OLEDERIVED(cls, base, low) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {if (iid.Data1==low) CmpGuidOle(obj, iid, pif); \
  else base##_QueryInterface(obj, iid, pif);}

#define DEFINE_QI_DELEGATE(cls, member) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {obj->member->QueryInterface(iid, pif);}

#define DEFINE_QI_DELEGATE_TEST(cls, member) \
  inline void cls##_QueryInterface(cls* obj, REFIID iid, void * * pif) \
  {if (obj->member) obj->member->QueryInterface(iid, pif);}

#define DEFINE_QI_MEMBER(name, member) \
  void name##_QueryInterface(void *, REFIID iid, void * * pif) \
  {member->QueryInterface(iid, pif);}

#define DEFINE_QI_MEMBER_TEST(name, member) \
  void name##_QueryInterface(void *, REFIID iid, void * * pif) \
  {if (member) member->QueryInterface(iid, pif);}

#define DEFINE_COMBASE_DELEGATE(cls, ifc, member) \
  struct _ICLASS cls { \
    cls() : member(0) {} \
    ifc* member; \
    DEFINE_QI_MEMBER_TEST(cls, member) \
  };

} // OCF namespace

#endif  //  OCF_OLEUTIL_H

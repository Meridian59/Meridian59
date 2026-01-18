//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TGdiObject, abstract class for GDI objects that can be
/// selected into TDCs
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/gdiobjec.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <owl/template.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlGDI, 1, 0);
// General GDI diagnostic group
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlGDIOrphan, 1, 0);
// Orphan control tracing group


//
// TGdiObject's internal orphan control object, container and member functions
//

struct TObjInfo {
  HANDLE            Handle;
  TGdiObject::TType Type;
  int RefCount;

  TObjInfo() : Handle(nullptr), Type(TGdiObject::None), RefCount(0)
  {}
  TObjInfo(HANDLE handle) : Handle(handle)
  {}
  TObjInfo(HANDLE handle, TGdiObject::TType type, int ref)
    : Handle(handle), Type(type), RefCount(ref)
  {}
  ~TObjInfo()
  {}
  bool operator ==(const TObjInfo& other) const {
    return other.Handle == Handle;
  }
  bool operator <(const TObjInfo& other) const {
    return other.Handle < Handle;
  }
};

typedef TSortedObjectArray<TObjInfo> TObjInfoBag;

static TObjInfoBag*    __ObjInfoBag      = 0;
#if defined(BI_MULTI_THREAD_RTL)
static TMRSWSection*  __ObjInfoBagLock  = 0;
#endif

#if defined(BI_MULTI_THREAD_RTL) //ObjInfoBag().Lock
#define LOCKOBJECTBAG(s) TMRSWSection::TLock __lock(*__ObjInfoBagLock,s)
#else
#define LOCKOBJECTBAG(s)
#endif

//static TObjInfoBag* ObjInfoBag;

#if defined(__TRACE)
static const LPCTSTR ObjTypeStr[] = {
  _T("?"), _T("Pen"), _T("Brush"), _T("Font"), _T("Palette"),
  _T("Bitmap"), _T("TextBrush"), 0
};
#endif

//
static void createObjInfoBag()
{
  static LONG createObjInfo = -1;
  if(__ObjInfoBag)
    return;

  if(::InterlockedIncrement(&createObjInfo)==0){
    ::InterlockedIncrement(&createObjInfo);
    __ObjInfoBag      = new TObjInfoBag;
#if defined(BI_MULTI_THREAD_RTL)
    __ObjInfoBagLock  = new TMRSWSection;
#endif
    createObjInfo = -1;
  }
}
//
static void destroyObjInfoBag()
{
  static LONG destroyObjInfo = -1;
  if(__ObjInfoBag==nullptr || __ObjInfoBag->Size()>0)
    return;

  if(::InterlockedIncrement(&destroyObjInfo)==0){
    ::InterlockedIncrement(&destroyObjInfo);
    delete __ObjInfoBag;
#if defined(BI_MULTI_THREAD_RTL)
    delete __ObjInfoBagLock;
#endif
    __ObjInfoBag  =  nullptr;
    destroyObjInfo = -1;
  }
}

//
/// Searches for the given object handle.
/// \returns If found, the object's type and reference count are returned in the specified TObjInfo
/// object. RefFind returns 0 if no match is found.
//
TObjInfo*
TGdiObject::RefFind(HANDLE handle)
{
  if (handle && __ObjInfoBag)
  {
    LOCKOBJECTBAG(true);
    const auto pos = __ObjInfoBag->Find(TObjInfo(handle));
    if (pos != __ObjInfoBag->NPOS)
      return &((*__ObjInfoBag)[pos]);
  }
  return nullptr;
}

//
/// RefAdd adds a reference entry for the object with the given handle and type.
/// If the table already has a matching entry, no action is taken, otherwise the reference count is
/// set to 1.
//
void
TGdiObject::RefAdd(HANDLE handle, TGdiObject::TType type)
{
  createObjInfoBag();
#if !defined(NO_GDI_SHARE_HANDLES)
  if (handle) {
    TObjInfo* member = RefFind(handle);
    if (member)
      member->RefCount++;
    else {
      LOCKOBJECTBAG(false);
      __ObjInfoBag->Add(TObjInfo(handle, type, 1));
    }
  }
#else
  if (handle && !RefFind(handle)){
    LOCKOBJECTBAG(false);
    __ObjInfoBag->Add(TObjInfo(handle, type, 1));
  }
#endif
}

//
/// Removes the reference entry to the object with the given handle.
/// If the given handle is not found, no action is taken.
//
void
TGdiObject::RefRemove(HANDLE handle)
{
  if(!__ObjInfoBag)
    return;
  {
    LOCKOBJECTBAG(false);
    __ObjInfoBag->DestroyItem(TObjInfo(handle));
  }
  destroyObjInfoBag();
}

//
/// Increments by 1 the reference count of the object associated with the given handle.
//
void
TGdiObject::RefInc(HANDLE handle)
{
  createObjInfoBag();
  TObjInfo* member = RefFind(handle);
  if (member)
  {
    member->RefCount++;
  }
}


//
/// Decrements this object's reference count by 1 and deletes the object when the reference count
/// reaches zero.
/// 
/// A warning is issued if the deletion was supposed to happen but didn't. Likewise, a warning is
/// issued if the deletion wasn't supposed to happen but did. The deleted object is also detached
/// from the ObjInfoBag table.
///
/// You can vary the normal deletion strategy by setting wantDelete to true or false.
//
void
TGdiObject::RefDec(HANDLE handle, bool wantDelete)
{
  InUse(wantDelete);
  if(!__ObjInfoBag)
    return;
  TObjInfo* member = RefFind(handle);
  if (member) {
    bool needDelete = --(member->RefCount) == 0;
#if defined(__TRACE)
    if (needDelete != wantDelete) {
      if (needDelete)
        TRACEX(OwlGDIOrphan, 1, "Orphan" << ObjTypeStr[member->Type] <<
        static_cast<void*>(member->Handle) << "Deleted");

      else
        TRACEX(OwlGDIOrphan, 1, ObjTypeStr[member->Type] <<
        static_cast<void*>(member->Handle) << "Orphan");

    }
#endif
    if (needDelete) {

      if (!::DeleteObject(member->Handle))
      {
        TXGdi::Raise(IDS_GDIDELETEFAIL, member->Handle);
      }


      {
        LOCKOBJECTBAG(false);
        __ObjInfoBag->DestroyItem(*member);
      }

      destroyObjInfoBag();
    }
  }
}

//
/// Returns this object's current reference count or -1 if the object is not found.
//
int
TGdiObject::RefCount(HANDLE handle)
{
  TObjInfo* member = RefFind(handle);
  if (member)
    return member->RefCount;
  return -1;
}



//----------------------------------------------------------------------------

//
//
//
#if defined(__TRACE) || defined(__WARN)
ostream& operator <<(ostream& os, const TGdiObject& gdiObject)
{
  os << "@" << (void*)&gdiObject << " ";
  os << '(';
  os << typeid(gdiObject).name() << ',';
  os << ')';
  return os;
}
#endif

//
/// This default constructor sets Handle to 0 and ShouldDelete to true. This
/// constructor is intended for use by derived classes that must set the Handle
/// member.
//
TGdiObject::TGdiObject()
{
  // Handle must be set and validated by the derived class.
  TRACEX(OwlGDI, 2, "TGdiObject constructed " << *this);
}

//
/// Create a wrapper for a given GDI object.
//
/// This constructor is intended for use by derived classes only. The Handle data
/// member is "borrowed" from an existing handle given by the argument handle The
/// ShouldDelete data member defaults to false ensuring that the borrowed handle
/// will not be deleted when the object is destroyed.
//
TGdiObject::TGdiObject(HANDLE handle, TAutoDelete autoDelete)
:
TGdiBase(handle, autoDelete)
{
  // Derived class must call RefAdd(Handle, type) if ShouldDelete
  TRACEX(OwlGDI, 2, "TGdiObject constructed " << *this);
}

//
/// Decrement the reference count on this object.
//
/// If ShouldDelete is false no action is taken. Otherwise, the reference count for the object's
/// handle is decremented, and if it reaches zero, the handle is is deleted.
//
TGdiObject::~TGdiObject()
{
  if (ShouldDelete)
    RefDec(Handle, true);
  TRACEX(OwlGDI, 2, "TGdiObject destructed " << *this);
}

} // OWL namespace

/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// OLE Utility class implementations
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/defs.h>
#include <ocf/oleutil.h>
#include <ocf/ocbocole.h>
#include <owl/string.h>
#include <owl/private/memory.h>

namespace ocf {

using namespace owl;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OcfRefCount, 1, 0);


//
//
//
void*
TOleAllocator::Alloc(unsigned long size)
{
  void* blk = Mem->Alloc(size);
  OLECALL(blk ? 0 : HR_OUTOFMEMORY, _T("IMalloc::Alloc"));
  return blk;
}

//
//
//
void
TOleAllocator::Free(void* pv)
{
  Mem->Free(pv);
}

//----------------------------------------------------------------------------
// TClassId - GUID/IID/CLSID management implementation
//

//
//
//
TClassId::TClassId(LPCTSTR idtxt)
         :Text(0)
{
  PRECONDITION(idtxt);
  OLECALL(::CLSIDFromString(OleStr(idtxt), &Guid), _T("Invalid GUID string"));
}

//
//
//
TClassId::TClassId(const GUID & guid, int offset)
:
  Text(0), Guid(guid)
{
  Guid.Data1 += (long)offset;
}

//
//
//
TClassId::TClassId(TGenerate)
:
  Text(0)
{
  OLECALL(::CoCreateGuid(&Guid), _T("CoCreateGuid"));
}

//
//
//
TClassId::~TClassId()
{
  delete[] const_cast<LPTSTR>(Text);
}

//
//
//
TClassId::operator LPCTSTR()
{
  if (!Text) {
    Text = new TCHAR[38+1];
    wsprintf(const_cast<LPTSTR>(Text),
             _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
             Guid.Data1, Guid.Data2, Guid.Data3,
             Guid.Data4[0], Guid.Data4[1], Guid.Data4[2], Guid.Data4[3],
             Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7]);
  }
  return Text;
}

//
//
//
TBaseClassId::TBaseClassId(const GUID & guid, int maxAlloc)
:
  TClassId(guid), MaxAlloc(maxAlloc), Offset(0)
{
}

//
//
//
TBaseClassId::TBaseClassId(LPCTSTR idtxt, int maxAlloc)
:
  TClassId(idtxt), MaxAlloc(maxAlloc), Offset(0)
{
}

//
//
//
TBaseClassId::TBaseClassId(TGenerate e, int maxAlloc)
:
  TClassId(e), MaxAlloc(maxAlloc), Offset(0)
{
}

//
//
//
TBaseClassId::TBaseClassId(TRegList& regInfo, int maxAlloc)
:
  TClassId(), MaxAlloc(maxAlloc), Offset(0)
{
  LPCTSTR str = regInfo["clsid"];
  if (!str)
    OLECALL(HR_CLASS_CLASSNOTAVAILABLE, _T("Missing REGDATA clsid"));
  OLECALL(::CLSIDFromString(OleStr(str), &Guid), _T("Invalid GUID string"));
}

//
//
//
TClassId
TBaseClassId::operator [](int offset)
{
  if(offset > Offset)
    OLECALL(HR_CLASS_CLASSNOTAVAILABLE, _T("Unassigned GUID"));
  return TClassId(Guid, offset);
}

//
//
//
int
TBaseClassId::AllocId()
{
  ++Offset;
  if(Offset >= MaxAlloc)
    OLECALL(HR_CLASS_CLASSNOTAVAILABLE, _T("GUID allocation exhausted"));
  return Offset;
}

//
//
//
int
TBaseClassId::GetOffset(const GUID& guid)
{
  unsigned long offset = guid.Data1 - Guid.Data1;
  if ( offset >= (unsigned long)Offset &&
       ((long*)&guid)[1] == ((long*)&Guid)[1] &&
       ((long*)&guid)[2] == ((long*)&Guid)[2] &&
       ((long*)&guid)[3] == ((long*)&Guid)[3] )
    return int(offset);
  else
    return -1;
}

//----------------------------------------------------------------------------
// GUID compare routines use by implementations of IUnknown::QueryInterface
//       the low-order long word is verifed before calling these routines
//       success is indicated by virtue of assigment of the returned interface
//

//
//
//
void
CmpGuid12(IUnknown* obj, REFIID req, REFIID ref, void ** pif)
{
  if ( ((long *)&req)[1] == ((long *)&ref)[1] &&
       ((long *)&req)[2] == ((long *)&ref)[2] &&
       ((long *)&req)[3] == ((long *)&ref)[3] ) {
    *pif = obj;
    obj->AddRef();
  }
}

//
//
//
void
CmpGuidOle(IUnknown* obj, REFIID req, void ** pif)
{
  if ( ((long*)&req)[1] == 0x00000000L &&
       ((long*)&req)[2] == 0x000000C0L &&
       ((long*)&req)[3] == 0x46000000L ) {
    *pif = obj;
    obj->AddRef();
  }
}

//----------------------------------------------------------------------------
// TUnknown - standard implementation of IUnknown
//

//
// Handy container to help watch COM object lifetime when debugging
//
#if (__DEBUG >= 2)
  typedef TPtrArray<TUnknown*> TUnknownBag;
  TUnknownBag* UnknownBag;
#endif

//
//
//
TUnknown::TUnknown()
         :Outer(&I)
{
#if (__DEBUG >= 2)
  if (!UnknownBag)
    UnknownBag = new TUnknownBag;
  UnknownBag->Add(this);
#endif
}

//
//
//
IUnknown&
TUnknown::Aggregate(TUnknown& inner)
{
  if (I.Inner)
    return I.Inner->Aggregate(inner);
  ((IUnknown&)inner).AddRef();
  I.Inner = &inner;
  return *Outer;
}

//
//
//
TUnknown::~TUnknown()
{
#if (__DEBUG >= 2)
  UnknownBag->DetachItem(this);
  if (UnknownBag->GetItemsInContainer()==0) {
    delete UnknownBag;
    UnknownBag=nullptr;
  }
#endif
  TRACEX(OcfRefCount, 1, "~TUnknown() @" << (void*)this
         << _T('(') << _OBJ_FULLTYPENAME(this) << _T(')')
         << ", RefCnt:" << I.RefCnt);
  WARNX(OcfRefCount, I.RefCnt != 0, 0, "~TUnknown() @ "
         << (void*)this
         << _T('(') << _OBJ_FULLTYPENAME(this) << _T(')')
         << ", RefCnt:" << I.RefCnt);

  // deleted by TUnknownI member
}

//
/// QueryObject used when TUnknown is in constructor/destructor
//
HRESULT
TUnknown::QueryObject(const GUID & /*iid*/, void * * /*pif*/)
{
  return ResultFromScode(E_NOINTERFACE);
}

//
//
//
HRESULT _IFUNC
TUnknown::TUnknownI::QueryInterface(const GUID & iid, void * * pif)
{
  *pif = 0;
  if (iid.Data1 == 0) { // IID_IUnknown.Data1
    CmpGuidOle(this, iid, pif);
    if (*pif)
      return NOERROR;
  }
  if (Host().QueryObject(iid, pif) == NOERROR)
    return NOERROR;
  if (Inner)
    return Inner->ThisUnknown().QueryInterface(iid, pif);
  else
    return ResultFromScode(E_NOINTERFACE);
}

//
//
//
unsigned long _IFUNC
TUnknown::TUnknownI::AddRef()
{
  TRACEX(OcfRefCount, 1,
         "AddRef on " << _OBJ_FULLTYPENAME(&Host()) << ' ' << (void*)&Host() << " to " << (RefCnt+1));
  return ++RefCnt;
}

//
//
//
unsigned long _IFUNC
TUnknown::TUnknownI::Release()
{
  TRACEX(OcfRefCount, 1,
         "Release on " << _OBJ_FULLTYPENAME(&Host()) << ' ' << (void*)&Host() << " to " << (RefCnt-1));
  if (--RefCnt != 0)
    return RefCnt;
  if (Inner) {
    Inner->ThisUnknown().Release();
    Inner = 0;
  }
  delete &Host();
  return 0;
}

//
//
//
unsigned long
TUnknown::GetRefCount()
{
  if (Outer == &I)
    return I.RefCnt;
  Outer->AddRef();
  return Outer->Release();
}

//
//
//
TUnknown::TUnknownI::~TUnknownI()
{
  // In case destructor called before RefCnt goes to 0
  //
  if (RefCnt)
    ::CoDisconnectObject(this,0);// OLE will try to remove external references
  Inner = 0;
}

//----------------------------------------------------------------------------
// Pixel <-> HiMetric conversion routines
//
static int logPixX = 0;
static int logPixY = 0;

//
//
static void
GetScreenMetrics(int& logPixX, int& logPixY)
{
  HDC hDCScreen = GetDC(0);
  logPixX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
  logPixY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
  ReleaseDC(0, hDCScreen);
}

//
//
//
void
OcHiMetricToPixel(const SIZEL& sizeHM, SIZEL& sizePix)
{
  if (!logPixX || !logPixY)
    GetScreenMetrics(logPixX, logPixY);

  sizePix.cx = MAP_LOGHIM_TO_PIX(sizeHM.cx, logPixX);
  sizePix.cy = MAP_LOGHIM_TO_PIX(sizeHM.cy, logPixY);
}

//
//
//
void
OcPixelToHiMetric(const SIZEL& sizePix, SIZEL& sizeHM)
{
  if (!logPixX || !logPixY)
    GetScreenMetrics(logPixX, logPixY);

  sizeHM.cx = MAP_PIX_TO_LOGHIM(sizePix.cx, logPixX);
  sizeHM.cy = MAP_PIX_TO_LOGHIM(sizePix.cy, logPixY);
}


} // OCF namespace

//==============================================================================


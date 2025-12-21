//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TMemoryDC and TDibDC encapsulation classes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>
#include <owl/gdiobjec.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group

//
// class TMemDCCache
// ~~~~~ ~~~~~~~~~~~
// Cache of screen-compatible memory HDCs used automatically by TMemoryDC when
// the default ctor is called. Reduces the amount of HDC creations & allows
// centralized sharing of these memory HDCs. The cache grows on demand.
//
class TMemDCCache
#if defined(BI_MULTI_THREAD_RTL)
                : public TLocalObject
#endif
{
  public:
    enum {MaxEntries = 16};          // Max # of mem DCs allowed in the cache

    struct TEntry {
      HDC   Handle;
      bool  Busy;
    };

    TMemDCCache();
   ~TMemDCCache();
    HDC  Get();
    void Release(HDC hDC);

#if defined(BI_MULTI_THREAD_RTL)
//    TMRSWSection  Lock;
#endif

  private:
    TEntry  Entries[MaxEntries];
};

//
// Static instance of the TMemoryDC cache used by TMemoryDC
//
static TMemDCCache& GetMemDCCache()
{
#if defined(BI_MULTI_THREAD_RTL)
  static TTlsContainer<TMemDCCache> memDCCache;
  return memDCCache.Get();
#else
  static TMemDCCache memDCCache;
  return memDCCache;
#endif
};

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TMemDCCache& InitMemDCCache = GetMemDCCache();
}

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKCACHE //TMRSWSection::TLock Lock(GetMemDCCache().Lock);
#else
#define LOCKCACHE
#endif

//
//
//
TMemDCCache::TMemDCCache()
{
  LOCKCACHE
  for (int i = 0; i < MaxEntries; i++) {
    Entries[i].Handle = nullptr;
    Entries[i].Busy = false;
  }
}

//
//
//
TMemDCCache::~TMemDCCache()
{
  LOCKCACHE
  for (int i = 0; i < MaxEntries && Entries[i].Handle; i++) {
    WARNX(OwlGDI, Entries[i].Busy, 0, _T("Unreleased DC ") <<
      static_cast<void*>(Entries[i].Handle) << _T(" in MemDCCache at destruction"));
    ::DeleteDC(Entries[i].Handle);
  }
}

//
// Get a screen compatible memory HDC from the cache, creating one if needed
// and there is room.
//
HDC
TMemDCCache::Get()
{
  LOCKCACHE
  for (int i = 0; i < MaxEntries; i++) {
    if (!Entries[i].Handle) {
      // Use a screen dc as reference when constructing the compatible DC
      // since in some rare situations CreateCompatibleDC(0) fails.
      //
      HDC sdc = ::GetWindowDC(nullptr);
      Entries[i].Handle = ::CreateCompatibleDC(sdc);
      ::ReleaseDC(nullptr, sdc);
    }
    if (!Entries[i].Busy) {
      Entries[i].Busy = true;
      return Entries[i].Handle;
    }
  }
  return nullptr;  // Cache is full
}

//
//
//
void
TMemDCCache::Release(HDC hDC)
{
  LOCKCACHE
  for (int i = 0; i < MaxEntries && Entries[i].Handle; i++) {
    if (Entries[i].Handle == hDC) {
      WARNX(OwlGDI, !Entries[i].Busy, 0, "Releasing non-busy DC " <<
        static_cast<void*>(Entries[i].Handle) << " in MemDCCache");
      Entries[i].Busy = false;
      return;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//
/// Constructs a screen-compatible memory DC.
//
TMemoryDC::TMemoryDC()
:
  TCreatedDC()
{
  Init();
}

//
/// Constructs a memory DC compatible with a non-screen DC.
//
TMemoryDC::TMemoryDC(const TDC& dc)
:
  TCreatedDC()
{
  Handle = ::CreateCompatibleDC(dc);
  CheckValid();
}

//
/// Constructs a TDC using an existing HDC.
/// If `autoDelete` is `true`, the handle is owned and deleted on destruction.
//
TMemoryDC::TMemoryDC(HDC handle, TAutoDelete autoDelete)
:
  TCreatedDC(handle, autoDelete)
{
}

//
/// Constructs a screen-compatible memory DC, with the given bitmap selected into it.
//
TMemoryDC::TMemoryDC(TBitmap& bitmap)
:
  TCreatedDC()
{
  Init();
  SelectObject(bitmap);
}

//
/// A protected pass-thru constructor for use by derived classes.
//
TMemoryDC::TMemoryDC(LPCTSTR driver, LPCTSTR device, const DEVMODE * initData)
  : TCreatedDC{driver, device, initData}
{}

//
/// String-aware overload
//
TMemoryDC::TMemoryDC(const tstring& driver, const tstring& device, const DEVMODE* initData)
  : TCreatedDC{driver, device, initData}
{}

#if defined(OWL5_COMPAT)

//
/// A protected pass-thru constructor for use by derived classes.
//
TMemoryDC::TMemoryDC(LPCTSTR driver, LPCTSTR device,
                     LPCTSTR output, const DEVMODE * initData)
:
  TCreatedDC(driver, device, output, initData)
{
}

//
/// A protected pass-thru constructor for use by derived classes.
//
TMemoryDC::TMemoryDC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData)
:
  TCreatedDC(driver, device, output, initData)
{
}

#endif

//
// Initialization for screen compatible DC construction.
// Tries to allocate a handle in the internal memory DC cache. If that fails (cache full), then
// we create our own.
//
void
TMemoryDC::Init()
{
  Handle = GetMemDCCache().Get();
  if (Handle) {
    ShouldDelete = false;              // Found, let the cache own the handle
  }
  else {
    Handle = ::CreateCompatibleDC(nullptr);  // Cache is full, we own the handle
    CheckValid();
  }
}

//
// Releases our handle if it has been allocated from the internal memory DC cache.
//
TMemoryDC::~TMemoryDC()
{
  if (!ShouldDelete)  // The HDC is never from the cache if ShouldDelete
    GetMemDCCache().Release(GetHDC());
}

} // OWL namespace
/* ========================================================================== */


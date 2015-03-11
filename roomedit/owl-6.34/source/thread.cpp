//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TThread class implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/thread.h>
#include <owl/pointer.h>
#include <owl/except.h>
#include <owl/module.h>

#if !defined(WINELIB)
#include <process.h>
#endif // !WINELIB

#include <stdlib.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-inl // Disable "Functions containing 'statement' is not expanded inline".
#endif

namespace owl {

OWL_DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlThread, 1, 0);

//----------------------------------------------------------------------------
// Semaphores Win32
//

//----------------------------------------
// TMutex WIN32

void TMutex::Release()
{
  ::ReleaseMutex(*this);
}

//----------------------------------------
// TCountedSemaphore WIN32

void TCountedSemaphore::Release()
{
  ::ReleaseSemaphore(*this, 1, 0);  
}

//----------------------------------------
// TEventSemaphore WIN32

void TEventSemaphore::Release()
{
  // Events don't need to be released
}

//----------------------------------------
// TWaitableTimer

//
//
//
TWaitableTimer::TWaitableTimer(bool manualReset, LPCTSTR name, LPSECURITY_ATTRIBUTES sa)
{
  Handle = CreateWaitableTimer(sa, manualReset, name);
  if (!Handle) throw TXOwl(_T("CreateWaitableTimer failed."));
}

//
// String-aware overload
//
TWaitableTimer::TWaitableTimer(bool manualReset, const tstring& name, LPSECURITY_ATTRIBUTES sa)
{
  Handle = CreateWaitableTimer(sa, manualReset, name.c_str());
  if (!Handle) throw TXOwl(_T("CreateWaitableTimer failed."));
}

//
TWaitableTimer::TWaitableTimer(LPCTSTR name, bool inherit,  uint32 access)
{
  Handle = OpenWaitableTimer(access, inherit, name);
  if (!Handle) throw TXOwl(_T("OpenWaitableTimer failed."));
}

//
// String-aware overload
//
TWaitableTimer::TWaitableTimer(const tstring& name, bool inherit,  uint32 access)
{
  Handle = OpenWaitableTimer(access, inherit, name.c_str());
  if (!Handle) throw TXOwl(_T("OpenWaitableTimer failed."));
}

//
bool  
TWaitableTimer::Cancel()
{
  return CancelWaitableTimer(Handle) ? true : false;  // handle to a timer object
}

//
bool  
TWaitableTimer::Set(const TFileTime& duetime,int32 period, PTIMERAPCROUTINE compFunc, void* param, bool resume)
{
  return SetWaitableTimer(Handle, (const LARGE_INTEGER*)&duetime,period,compFunc,param, resume);
}
//
void TWaitableTimer::Release()
{
}

//----------------------------------------
// TSemaphoreSet & its TLock for Win32

//
TSemaphoreSet::TSemaphoreSet(const TSemaphore* sems[], int size)
:
  Sems(0)
{
  int count = 0;
  if (sems)
    while (sems[count])
      count++;
  Count = count;
  Size = size >= 0 ? size : count;
  if (Size) {
    Sems = CONST_CAST(const TSemaphore**, new TSemaphorePtr[Size]);
    int i = 0;
    if (sems)
      for (; i < Count; i++)
        Sems[i] = sems[i];
    for (; i < Size; i++)
      Sems[i] = 0;
  }
}

//
TSemaphoreSet::~TSemaphoreSet()
{
  delete[] Sems;
}

//
void TSemaphoreSet::Add(const TSemaphore& sem)
{
  if (Count < Size)
    Sems[Count++] = &sem;
}

//
void TSemaphoreSet::Remove(const TSemaphore& sem)
{
  CHECK(Count <= Size);
  for (int i = 0; i < Count; i++)
    if (Sems[i] == &sem) {
      for (int j = i; j < Count-1; j++)  // Shift rest down to keep packed
        Sems[j] = Sems[j+1];
      Sems[Count-1] = 0;
      return;
    }
}

//
void TSemaphoreSet::Release(int index)
{
  if (index >= 0)
    CONST_CAST(TSemaphore*,Sems[index])->Release();
  else
    for (int i = 0; i < Count; i++)
      CONST_CAST(TSemaphore*,Sems[i])->Release();
}

//
static HANDLE* newHandles(const TSemaphoreSet& set)
{
  HANDLE* handles = new HANDLE[set.GetCount()];
  for (int i = 0; i < set.GetCount(); i++) {
    CHECK(set[i]);
    handles[i] = *set[i];  // Assumes non-0 since i is in set range
  }
  return handles;
}

//
TSemaphoreSet::TLock::TLock(const TSemaphoreSet& set, TWaitWhat wait,
                            ulong timeOut, bool alertable)
:
  Set(0)
{
  TAPointer<THandle> handles(newHandles(set));

  if (InitLock(set.Count, wait,
               ::WaitForMultipleObjectsEx(set.Count, handles, wait, timeOut, alertable)))
    Set = &set;
}

//
TSemaphoreSet::TLock::TLock(ulong msgMask, const TSemaphoreSet& set,
                            TWaitWhat wait, ulong timeOut)
{
  TAPointer<THandle> handles(newHandles(set));

  if (InitLock(set.Count, wait,
               ::MsgWaitForMultipleObjects(set.Count, handles, wait, timeOut, msgMask)))
    Set = &set;
}

//
// Init the Set and Locked members after a system wait call
//
bool TSemaphoreSet::TLock::InitLock(int count, TWaitWhat wait, int index)
{
  if (index >= (int)WAIT_OBJECT_0 && index < int(WAIT_OBJECT_0+count) ||
      index >= (int)WAIT_ABANDONED_0 && index < int(WAIT_ABANDONED_0+count))
  {
    if (wait == WaitAny) {
      if (index >= (int)WAIT_ABANDONED_0)
        index -= WAIT_ABANDONED_0;
      Locked = index;      // Just this one is locked
    }
    else
      Locked = AllAquired;         // They are all locked
    return true;
  }
  else if (index == int(WAIT_OBJECT_0+count))
    Locked = MsgWaiting;
  else if (index == WAIT_TIMEOUT)
    Locked = TimedOut;
  else if (index == (int)WAIT_IO_COMPLETION)
    Locked = IoComplete;
  return false;
}

//
TSemaphoreSet::TLock::~TLock()
{
  Release();
}

//
void TSemaphoreSet::TLock::Release(bool relinquish)
{
  if (Set) {
    CONST_CAST(TSemaphoreSet*,Set)->Release(Locked);
    if (relinquish)
      Set = 0;
  }
}

//----------------------------------------------------------------------------
// class TMRSWSection
//

namespace
{

  //
  // This class encapsulates the undocumented fat read/write lock functions in "ntdll.dll".
  // See http://undoc.airesoft.co.uk/ntdll.dll/RtlInitializeResource.php
  // These functions are used to implement TMRSWSection.
  //
  // TODO: Investigate alternatives that do not rely on this undocumented feature, such as the 
  // documented slim read/write (SRW) functions in the Windows API since Vista.
  //
  class TNtDll_ : public TModule
  {
  public:

    //
    // Returns the module instance.
    // Exceptions are thrown if the module cannot be loaded.
    //
    static TNtDll_& GetInstance()
    {
      //
      // Note that while this lazy initialization avoids problems with global initialization order,
      // the initial call of this function is not thread-safe (pre C++11). As a work-around, we 
      // ensure this function is called during program start-up (single-thread, safe). 
      // See InitNtDllInstance below.
      //
      // The work-around can be removed when C++11 compliant compilers are mandated.
      //
      // Also note that the singleton construction may throw an exception. Since there is no way to 
      // continue without this singleton, we make no attempt to handle it here. We assume that 
      // the exception will terminate the program, or if it is handled, that subsequent calls will
      // be retried still within a single thread.
      //
      static TNtDll_ instance; // initial call (construction) not thread-safe pre-C++11
      return instance;
    }

    //
    // Undocumented structure representing the critical section.
    //
    struct RTL_RESOURCE
    {
      CRITICAL_SECTION CSection;
      HANDLE hSharedSemaphore;
      ULONG sharedExclusive;
      HANDLE hExclusiveSemaphore;
      ULONG exclusiveWaiters;
      ULONG numberOfWaiters;
      HANDLE hOwnerThread;
      ULONG flags;
      PRTL_RESOURCE_DEBUG DebugInfo;
    };

    typedef RTL_RESOURCE* PRTL_RESOURCE;

    TModuleProcV1<PRTL_RESOURCE> RtlInitializeResource;
    TModuleProcV1<PRTL_RESOURCE> RtlDeleteResource;
    TModuleProc2<BOOLEAN, PRTL_RESOURCE, BOOLEAN> RtlAcquireResourceExclusive;
    TModuleProc2<BOOLEAN, PRTL_RESOURCE, BOOLEAN> RtlAcquireResourceShared;
    TModuleProcV1<PRTL_RESOURCE> RtlReleaseResource;
    TModuleProcV1<PRTL_RESOURCE> RtlDumpResource;

  private:

    TNtDll_()
      : TModule(_T("ntdll.dll"), true, true, false), // shouldLoad, mustLoad and !addToList
      RtlInitializeResource(*this, "RtlInitializeResource"),
      RtlDeleteResource(*this, "RtlDeleteResource"),
      RtlAcquireResourceExclusive(*this, "RtlAcquireResourceExclusive"),
      RtlAcquireResourceShared(*this, "RtlAcquireResourceShared"),
      RtlReleaseResource(*this, "RtlReleaseResource"),
      RtlDumpResource(*this, "RtlDumpResource")
    {
      TRACEX(OwlThread, 1, "Initializing " << TraceId(this));
    }

    TNtDll_(const TNtDll_&); // prohibited
    TNtDll_& operator =(const TNtDll_&); // prohibited
  };

  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TNtDll_& InitNtDllInstance = TNtDll_::GetInstance();

} // namespace

struct TMRSWSection::TPimpl
{
  TNtDll_& Dll;
  TNtDll_::RTL_RESOURCE RtlResource;

  TPimpl(TNtDll_& dll) : Dll(dll) {}
};

TMRSWSection::TMRSWSection()
  : Pimpl(new TPimpl(TNtDll_::GetInstance()))
{
  Pimpl->Dll.RtlInitializeResource(&Pimpl->RtlResource);
}

TMRSWSection::~TMRSWSection()
{
  Pimpl->Dll.RtlDeleteResource(&Pimpl->RtlResource);
  delete Pimpl;
}

void TMRSWSection::Dump()
{
  Pimpl->Dll.RtlDumpResource(&Pimpl->RtlResource);
}

TMRSWSection::TLock::TLock(TMRSWSection& s, bool shared, bool wait)
  : Section(s)
{
  TNtDll_& dll = Section.Pimpl->Dll;
  TNtDll_::PRTL_RESOURCE r = &Section.Pimpl->RtlResource;
  bool ok = shared ?
    dll.RtlAcquireResourceShared(r, wait) :
    dll.RtlAcquireResourceExclusive(r, wait);
  if (!ok)
    throw TXLockFailure(_T("TMRSWSection::TLock failed to acquire lock"));
}

TMRSWSection::TLock::~TLock()
{
  TNtDll_& dll = Section.Pimpl->Dll;
  TNtDll_::PRTL_RESOURCE r = &Section.Pimpl->RtlResource;
  dll.RtlReleaseResource(r);
}

TMRSWSection::TXLockFailure::TXLockFailure(const tstring& msg)
  : TXBase(msg)
{}

//----------------------------------------------------------------------------
// TThread Win32 

//
// TThread constructors
//
/// Creates an object of type TThread.
TThread::TThread()
:
  ThreadId(0),
  Handle(0),
  Stat(Created),
  TerminationRequested(0),
  Attached(false)
{
}

//
/// Attach to a running thread
//
TThread::TThread(TCurrent)
:
  ThreadId(::GetCurrentThreadId()),
  Handle(0),
  Stat(Running),
  TerminationRequested(0),
  Attached(true)
{
  ::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(),
                    ::GetCurrentProcess(), &Handle,
                    0, false, DUPLICATE_SAME_ACCESS);
}

//
/// Puts the object into the Created
/// state, just like the default constructor.
/// Does not copy any of the internal details of the thread being copied.
//
TThread::TThread(const TThread&)
:
  ThreadId(0),
  Handle(0),
  Stat(Created),
  TerminationRequested(0),
  Attached(false)
{
}

//
/// TThread assignment operator
//
/// Used when assigning derived objects. Attempting to assign from a running
/// object is an error, since the data fields in the running object can be
/// changing asynchronously.
///
/// The target object must be in either the
/// Created state or the Finished state. If
/// so, puts the object into the Created
/// state. If the object is not in either the
/// Created state or the Finished state it
/// is an error and an exception will be
/// thrown.

const TThread& TThread::operator =(const TThread& thread)
{
  switch (GetStatus()) {
    case Created:
    case Suspended:
    case Finished: {
      if (this != &thread) {
        Handle = 0;
        ThreadId = 0;
        Stat = Created;
        TerminationRequested = false;
        Attached = false;
      }
      return *this;
    }
    default:
      throw TThreadError(TThreadError::AssignError);
  }
#if defined(BI_COMP_MSC)
  return *this;     // Bogus return to make MSVC happy
#endif
}

//
/// TThread destructor
//
/// If the thread hasn't finished, destroying its control object is an error.
//
TThread::~TThread()
{
  if (!Attached && (GetStatus() == Running || GetStatus() == Suspended))
    throw TThreadError(TThreadError::DestroyBeforeExit);
//
// The RTL calls CloseHandle in _endthread, so we shouldn't if the thread
// was started with _beginthreadNT(...).
#  if !defined(BI_MULTI_THREAD_RTL) || !defined(BI_COMP_BORLANDC)
  ::CloseHandle(Handle);
#  endif
}

//
/// Starts the thread executing. The actual call depends on the operating system.
/// Returns the handle of the thread.
/// After the system call we check status. 
//
TThread::THandle TThread::Start()
{
  // If Start() has already been called for this thread, release the
  // previously created system thread object before launching a new one.
  //
  if ((GetStatus() != Created) && Handle) {
    ::CloseHandle(Handle);
  }

# if defined(BI_MULTI_THREAD_RTL)
#    if defined(BI_COMP_MSC) || defined(BI_COMP_GNUC)
  Handle = (HANDLE)::_beginthreadex(0, 4096, &TThread::Execute, this, 0, (uint*)&ThreadId);
#    else
  Handle = (HANDLE)::_beginthreadNT(&TThread::Execute, 4096, this, 0, 0, &ThreadId);
#    endif
# else
  Handle = ::CreateThread(0, 0, &TThread::Execute, this, 0, &ThreadId);
# endif


  if (Handle) {
    TRACEX(OwlThread, 1, _T("Thread started [id:") << Handle << _T(']'));
    Stat = Running;
  }
  else {
    TRACEX(OwlThread, 2, _T("Thread failed to start"));
    Stat = Invalid;
    throw TThreadError(TThreadError::CreationFailure);
  }

  return Handle;
}

/// Suspends execution of the thread.
//
/// It's an error to try to suspend a thread that hasn't been started or that
/// has already terminated.
//
ulong TThread::Suspend()
{
  switch (GetStatus()) {
    case Created:
      TRACEX(OwlThread, 2, _T("Illegal Created thread suspension [id:") << Handle << _T(']'));
      throw TThreadError(TThreadError::SuspendBeforeRun);
    case Finished:
      TRACEX(OwlThread, 2, _T("Illegal Finished thread suspension [id:") << Handle << _T(']'));
      throw TThreadError(TThreadError::SuspendAfterExit);
    default:
      ulong res = ::SuspendThread(Handle);
      if (res < MAXIMUM_SUSPEND_COUNT)  // Else a problem
        Stat = Suspended;

      TRACEX(OwlThread, 0, _T("Thread suspended [id:") << Handle << _T(", Count:") << res << _T(']'));
      return res;
  }
}

/// Resumes execution of a suspended thread.
//
/// It's an error to try to resume a thread that isn't suspended.
//
ulong TThread::Resume()
{
  switch (GetStatus()) {
    case Created:
      TRACEX(OwlThread, 2, _T("Illegal Created thread resumption [id:") << Handle << _T(']'));
      throw TThreadError(TThreadError::ResumeBeforeRun);
    case Running:
      TRACEX(OwlThread, 2, _T("Illegal Running thread resumption [id:") << Handle << _T(']'));
      throw TThreadError(TThreadError::ResumeDuringRun);
    case Finished:
      TRACEX(OwlThread, 2, _T("Illegal Finished thread resumption [id:") << Handle << _T(']'));
      throw TThreadError(TThreadError::ResumeAfterExit);
    default:
      ulong res = ::ResumeThread(Handle);
      TRACEX(OwlThread, 0, _T("Thread resumed [id:") << Handle << _T(", Count:") << res << _T(']'));
      if (res == 1) // Y.B. suggested by Richard.D.Crossley and Fendy Riyanto
        Stat = Running;
      return res;
    }
}

//
/// Mark the thread for termination.
//
/// Sets an internal flag that indicates that the 
/// thread should exit. The derived class can check
/// the state of this flag by calling ShouldTerminate().
//
void TThread::Terminate()
{
  TRACEX(OwlThread, 1, _T("Thread termination requested [handle:") << Handle << _T(']'));
  TerminationRequested = true;
}

//
/// Blocks the calling thread until the internal
/// thread exits or until the time specified by
/// timeout, in milliseconds,expires. A timeout of
/// NoLimit says to wait indefinitely.
//
ulong TThread::WaitForExit(ulong timeout)
{
  TRACEX(OwlThread, 1, _T("Waiting for thread exit [id:") << Handle << _T(']'));
  if (Stat == Running)
    return ::WaitForSingleObject(Handle, timeout);
  else
    return (ulong)-1;
}

//
/// Combines the behavior of Terminate() and
/// WaitForExit(). Sets an internal flag that
/// indicates that the thread should exit and blocks
/// the calling thread until the internal thread
/// exits or until the time specified by timeout, in
/// milliseconds, expires. A timeout of NoLimit says
/// to wait indefinitely.
//
ulong TThread::TerminateAndWait(ulong timeout)
{
  Terminate();
  return WaitForExit(timeout);
}

//
/// Sets the priority of the thread.
//
int TThread::SetPriority(int pri)
{
  TRACEX(OwlThread, 1, _T("Thread priority changed to ") << pri <<
                     _T(" [id:") << Handle << _T(']'));
  return ::SetThreadPriority(Handle, pri);
}

//
/// This function does the actual thread execution. Calling Start creates a thread
/// that begins executing Run with the this pointer pointing to the TThread-based
/// object. Every derived class should define its own version of Run. See the
/// example provided.
//
int
TThread::Run()
{
  TRACEX(OwlThread, 1, _T("Illegal Run() on base TThread [id:") << Handle << _T(']'));
  return -1;
}

//
// Run the thread. This static function is given as the thread start address,
// with 'this' thread object passed as the param. Invoke the Run() method on
// the thread
//
#if defined(BI_MULTI_THREAD_RTL)
#  if  defined(BI_COMP_BORLANDC)
void _USERENTRY TThread::Execute(void* thread)
{
  STATIC_CAST(TThread*,thread)->Run();
  STATIC_CAST(TThread*,thread)->Stat = Finished;
}
#  else
uint _stdcall TThread::Execute(void* thread)
{
  int code = STATIC_CAST(TThread*,thread)->Run();
  STATIC_CAST(TThread*,thread)->Stat = Finished;
  return code;
}
#  endif
#else //#if defined(BI_MULTI_THREAD_RTL)
DWORD WINAPI TThread::Execute(void* thread)
{
  int code = STATIC_CAST(TThread*,thread)->Run();
  STATIC_CAST(TThread*,thread)->Stat = Finished;
  return code;
}
#endif //#if defined(BI_MULTI_THREAD_RTL)

//
/// Exit provides an alternative to returning from Run.
//
/// Called from within the thread that wants to exit early.
//
void
TThread::Exit(ulong code)
{
  Stat = Finished;
#  if (defined(BI_COMP_MSC)||defined(BI_COMP_GNUC)) && defined(BI_MULTI_THREAD_RTL)
    _endthreadex(code);
#  else
    ::ExitThread(code);
# endif
}

//
// Call only when Stat claims that the thread is Running.
//
TThread::TStatus
TThread::CheckStatus() const
{
  DWORD code;
  ::GetExitCodeThread(Handle, &code);
  if (code == STILL_ACTIVE)
    return Running;
  else
    return Finished;
}

//----------------------------------------------------------------------------

//
// TThread::TThreadError constructor
//
TThread::TThreadError::TThreadError(TErrorType type)
:
  TXBase(MakeString(type)),
  Type(type)
{
}

//
// TThread::TThreadError::MakeString()
//
// Translates an error code into a string.
//
tstring TThread::TThreadError::MakeString(TErrorType type)
{
  static tchar* Names[] = {
    _T("Suspend() before Run()"),
    _T("Resume() before Run()"),
    _T("Resume() during Run()"),
    _T("Suspend() after Exit()"),
    _T("Resume() after Exit()"),
    _T("creation failure"),
    _T("destroyed before Exit()"),
    _T("illegal assignment"),
    _T("Multithreaded Runtime not selected"),
  };
  tstring Msg;
  Msg.reserve(80);
  Msg = _T("Error[thread]: ");
  Msg += Names[type];
  return Msg;
}
} // OWL namespace
//

#if defined(BI_MULTI_THREAD_RTL) && defined(OWL5_COMPAT)

namespace owl {

void TLocalData::SetData(int index, TLocalObject* obj) 
{ 
  while(index >= (int)Data.Size())
    Data.Add(0);
  Data[index] = obj;
}

} // OWL namespace

#endif

//
// These are just dummies left over from the old implementation of thread-local storage.
//

static long _owlTlsRef = 0;

long TlsAddRefs()
{  
  return _owlTlsRef++;
}

long TlsRelease()
{  
  return --_owlTlsRef;
}

/* ========================================================================== */

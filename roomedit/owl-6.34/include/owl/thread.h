//----------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// - Defines the class TSemaphore and its nested class TLock.
/// - Defines the derived semaphore class TMutex
/// - Defines the derived semaphore class TCountedSemaphore
/// - Defines the derived semaphore class TEventSemaphore
/// - Defines the derived semaphore class TWaitableTimer
/// - Defined the semaphore set class TSemaphoreSet and its nested class TLock.
/// - Defines the class TCriticalSection and its nested class TLock.
/// - Defines the class TMRSWSection and its nested class TLock.
/// - Defines the class TSync and its nested class TLock.
/// - Defines the template TStaticSync and its nested class TLock.
/// - Defines the class TThread.
/// - Defines the class TTlsAllocator
/// - Defines the class TLocalObject
/// - Defines the class TLocalData
/// - Defines the class TTlsDataManager
/// - Defines the class TLocalObject
//----------------------------------------------------------------------------

#if !defined(OWL_THREAD_H)
#define OWL_THREAD_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

#if !defined(BI_MULTI_THREAD)
#error Thread classes require multi-threaded operating system.
#endif

#include <owl/private/wsysinc.h>
#include <owl/wsyscls.h>
#include <owl/exbase.h>

#if defined(BI_MULTI_THREAD_RTL)
# include <owl/template.h>
# include <vector>
# include <algorithm>
#endif

namespace owl {

DIAG_DECLARE_GROUP(OwlThread);

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup utility
/// @{

//
/// \class TSemaphore
// ~~~~~ ~~~~~~~~~~
/// Base class for handle-based thread synchronization classes, TMutex,
/// TCountedSemaphore and TEventSemaphore. Defines some types & constants, as
/// well as holding the system object handle for Win32.
//
class _OWLCLASS TSemaphore{
  public:
    virtual ~TSemaphore();

    enum { NoWait = 0, NoLimit = INFINITE };
    typedef HANDLE THandle;
    operator THandle() const;


    class TLock {
      public:
        TLock(const TSemaphore&, ulong timeOut = NoLimit, bool alertable = false);
       ~TLock();

        bool WasAquired() const;  ///< See if really aquired, or just timed-out

        /// Release lock early & relinquish, i.e. before destructor. Or, just
        /// drop the lock count on an exiting semaphore & keep locked 'til dtor
        //
        void Release(bool relinquish = false);

      private:
        const TSemaphore* Sem;
    };
    friend class TLock;
    friend class _OWLCLASS TSemaphoreSet;

  protected:
    virtual void Release() = 0;  ///< Derived class must provide release

    THandle Handle;              ///< Derived class must initialize
};

//
/// \class TMutex
// ~~~~~ ~~~~~~
/// Mutual-exclusive semaphore
//
/// TMutex provides a system-independent interface to critical sections in
/// threads. With suitable underlying implementations the same code can be used
/// under OS/2 and Windows NT.
//
/// An object of type TMutex can be used in conjunction with objects of type
/// TMutex::TLock (inherited from TSemaphore) to guarantee that only one thread
/// can be executing any of the code sections protected by the lock at any
/// given time.
//
/// The differences between the classes TCriticalSection and TMutex are that a
/// timeout can be specified when creating a Lock on a TMutex object, and that
/// a TMutex object has a HANDLE which can be used outside the class. This
/// mirrors the distinction made in Windows NT between a CRITICALSECTION and a
/// Mutex. Under NT a TCriticalSection object is much faster than a TMutex
/// object. Under operating systems that don't make this distinction a
/// TCriticalSection object can use the same underlying implementation as a
/// TMutex, losing the speed advantage that it has under NT.
//
class _OWLCLASS TMutex : public TSemaphore{
  public:
    TMutex(LPCTSTR name = 0, LPSECURITY_ATTRIBUTES sa = 0);
    TMutex(const tstring& name, LPSECURITY_ATTRIBUTES sa = 0);
    TMutex(LPCTSTR name, bool inherit, uint32 access = MUTEX_ALL_ACCESS);
    TMutex(const tstring& name, bool inherit, uint32 access = MUTEX_ALL_ACCESS);
    TMutex(THandle handle);

    typedef TLock Lock;  ///< For compatibility with old T-less typename

    /// If another mutex with the same name existed when this mutex
    /// was created, then another handle to the object exists and
    /// someone else may be using it too.
    //
    bool IsShared();

  private:
    TMutex(const TMutex&);
    const TMutex& operator =(const TMutex&);

    virtual void Release();  ///< Release this mutex semaphore

    bool Shared;

};

//
/// \class TCountedSemaphore
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// Counted semaphore. Currently Win32 only
//
class _OWLCLASS TCountedSemaphore : public TSemaphore{
  public:
    TCountedSemaphore(int initialCount, int maxCount, LPCTSTR name = 0,
                      LPSECURITY_ATTRIBUTES sa = 0);
    TCountedSemaphore(int initialCount, int maxCount, const tstring& name, LPSECURITY_ATTRIBUTES sa = 0);
    TCountedSemaphore(LPCTSTR name, bool inherit,
                      uint32 access = SEMAPHORE_ALL_ACCESS);
    TCountedSemaphore(const tstring& name, bool inherit, uint32 access = SEMAPHORE_ALL_ACCESS);
    TCountedSemaphore(THandle handle);

  private:
    TCountedSemaphore(const TCountedSemaphore&);
    const TCountedSemaphore& operator =(const TCountedSemaphore&);

    virtual void Release();  ///< Release this counted semaphore
};

//
/// \class TEventSemaphore
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _OWLCLASS TEventSemaphore : public TSemaphore{
  public:
    TEventSemaphore(bool manualReset=false, bool initialState=false,
                    LPCTSTR name = 0, LPSECURITY_ATTRIBUTES sa = 0);
    TEventSemaphore(bool manualReset, bool initialState, const tstring& name, LPSECURITY_ATTRIBUTES sa = 0);
    TEventSemaphore(LPCTSTR name, bool inherit,
                    uint32 access = SEMAPHORE_ALL_ACCESS);
    TEventSemaphore(const tstring& name, bool inherit, uint32 access = SEMAPHORE_ALL_ACCESS);
    TEventSemaphore(THandle handle);

    void Set();
    void Reset();
    void Pulse();


  private:
    TEventSemaphore(const TMutex&);
    const TEventSemaphore& operator =(const TEventSemaphore&);

    virtual void Release();  ///< Release this event semaphore

};

//
/// \class TWaitableTimer
// ~~~~~ ~~~~~~~~~~~~~~
///  Encapsulation of Waitable Timer  over Borland Classlib TSemaphore hierarchy
//
//  Created by Marco Savegnago (msave) email: msave@tin.it
//
// additional typedef for old headers
#if !defined(CreateWaitableTimer)
typedef VOID (APIENTRY *PTIMERAPCROUTINE)(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue );
#endif


class _OWLCLASS TWaitableTimer : public TSemaphore {
  public:
    TWaitableTimer(bool manualReset=false, LPCTSTR name = 0, LPSECURITY_ATTRIBUTES sa = 0);
    TWaitableTimer(bool manualReset, const tstring& name, LPSECURITY_ATTRIBUTES sa = 0);
    TWaitableTimer(LPCTSTR name, bool inherit = false,  uint32  dwDesiredAccess = FILE_ALL_ACCESS);
    TWaitableTimer(const tstring& name, bool inherit = false,  uint32  dwDesiredAccess = FILE_ALL_ACCESS);
    TWaitableTimer(THandle handle);
    virtual ~TWaitableTimer();

    bool Set(const TFileTime& duetime,  int32 period=0,PTIMERAPCROUTINE compFunc=0,
             void* param=0,  bool resume=false);

    bool Cancel();
    virtual void Release();

  private:
    TWaitableTimer(const TWaitableTimer&);
    const TWaitableTimer& operator =(const TWaitableTimer&);
};


//
/// \class TSemaphoreSet
// ~~~~~ ~~~~~~~~~~~~~
/// Semaphore object aggregator. Used to combine a set of semaphore objects so
/// that they can be waited upon (locked) as a group. The lock can wait for any
/// one, or all of them. The semaphore objects to be aggregated MUST live at
/// least as long as this TSemaphoreSet, as it maintains pointers to them.
//
class _OWLCLASS TSemaphoreSet{
  public:
    /// sems is initial array of sem ptrs, may be 0 to add sems later,
    /// size is maximum sems to hold, -1 means count the 0-terminated array
    /// Passing (0,-1) is not valid
    //
    TSemaphoreSet(const TSemaphore* sems[], int size = -1);
   ~TSemaphoreSet();

    void Add(const TSemaphore& sem);
    void Remove(const TSemaphore& sem);
    int  GetCount() const;
    const TSemaphore* operator [](int index) const;

    enum TWaitWhat { WaitAny = false, WaitAll = true };
    enum { NoWait = 0, NoLimit = INFINITE };

    class _OWLCLASS TLock {
      public:
        /// Assumes that set is not modified while locked
        //
        TLock(const TSemaphoreSet& set, TWaitWhat wait,
              ulong timeOut = NoLimit, bool alertable = false);
        TLock(ulong msgMask, const TSemaphoreSet& set, TWaitWhat wait,
              ulong timeOut = NoLimit);
       ~TLock();

        bool WasAquired() const;    ///< See if one or more was aquired
        enum {
          AllAquired = -1,   ///< All semaphores were aquired
          TimedOut   = -2,   ///< None aquired: timed out
          IoComplete = -3,   ///<               IO complate alert
          MsgWaiting = -4,   ///<               Msg waiting
        };
        int  WhichAquired() const;  ///< See which was aquired >=0, or enum
       
        void Release(bool relinquish = false);

      protected:
        bool InitLock(int count, TWaitWhat wait, int index);

      private:
        const TSemaphoreSet* Set;
        int   Locked;    ///< Which one got locked, or wait code
    };
    friend class TLock;

  private:
    void Release(int index = -1);

    typedef TSemaphore* TSemaphorePtr;
    typedef HANDLE THandle;

    const TSemaphore** Sems;    ///< Array of ptrs to semaphores, packed at head

    int   Size;   ///< Size of array, i.e. maximum object count
    int   Count;  ///< Count of objects currently in array
};

//
/// \class TCriticalSection
// ~~~~~ ~~~~~~~~~~~~~~~~
/// Lightweight intra-process thread synchronization. Can only be used with
/// other critical sections, and only within the same process.
//
/// TCriticalSection provides a system-independent interface to critical sections in
/// threads. TCriticalSection objects can be used in conjunction with
/// TCriticalSection::Lock objects to guarantee that only one thread can be
/// executing any of the code sections protected by the lock at any given time.
//
class TCriticalSection{
  public:
    TCriticalSection();
   ~TCriticalSection();

/// This nested class handles locking and unlocking critical sections.
///
/// Only one thread of execution will be allowed to execute the critical code inside
/// function f at any one time.
/// \code
/// TCriticalSection LockF;
/// void f()
/// { 
///    TCriticalSection::Lock(LockF);
/// 
///    // critical processing here
/// }
/// \endcode
    class TLock {
      public:
        TLock(const TCriticalSection&);
       ~TLock();
      private:
        const TCriticalSection& CritSecObj;
    };
    friend class TLock;
    typedef TLock Lock;  ///< For compatibility with old T-less typename

  private:
    CRITICAL_SECTION CritSec;

    TCriticalSection(const TCriticalSection&);
    const TCriticalSection& operator =(const TCriticalSection&);
};

//
/// \class TMRSWSection
/// Multiple Read, Single Write section
//
class _OWLCLASS TMRSWSection
{
public:

  TMRSWSection();
  ~TMRSWSection();

  class _OWLCLASS TLock 
  {
  public:

    //
    /// shared == true; allows multiple (read) access to the section.
    /// shared == false; allows only exclusive (write) access to the section.
    /// wait == false; will throw an exception if the lock can not be acquired immediately.
    ///
    /// TXLockFailure is thrown on failure.
    //
    TLock(TMRSWSection&, bool shared, bool wait = true);
    ~TLock();

  private:

    TMRSWSection& Section;

    TLock(const TLock&); ///< prohibited
    const TLock& operator =(const TLock&); ///< prohibited
  };

  friend class TMRSWSection::TLock;

  class _OWLCLASS TXLockFailure
    : public TXBase
  {
  public:

    explicit TXLockFailure(const tstring& msg);
  };

  //
  /// Dumps the current state of the section to the debugger output.
  //
  void Dump();

private:

  struct TPimpl;
  TPimpl* Pimpl;

  TMRSWSection(const TMRSWSection&); ///< prohibited
  const TMRSWSection& operator =(const TMRSWSection&); ///< prohibited
};

//
// class TSync::TLock
// ~~~~~ ~~~~~~~~~~~
/// \class TSync
/// TSync provides a system-independent interface to build classes that act like
/// monitors, i.e., classes in which only one member can execute on a particular
/// instance at any one time. TSync uses TCriticalSection, so it is portable to
/// all platforms that TCriticalSection has been ported to.
///
/// Example
/// \code
///     class ThreadSafe : private TSync
///     {
///       public:
///         void f();
///         void g();
///       private:
///         int i;
///     };
///
///     void ThreadSafe::f()
///     {
///       TLock lock(this);
///       if (i == 2)
///         i = 3;
///     }
///
///     void ThreadSafe::g()
///     {
///       TLock lock(this);
///       if (i == 3)
///         i = 2;
///     }
/// \endcode
//
class TSync
{
  protected:
    TSync();
    TSync(const TSync&);
    const TSync& operator =(const TSync&);

/// This nested class handles locking and unlocking critical sections.
    class TLock : private TCriticalSection::TLock  
    {
      public:
        TLock(const TSync*);
      private:
        static const TCriticalSection& GetRef(const TSync*);
    };
    friend class TLock;
    typedef TLock Lock;  ///< For compatibility with old T-less typename

  private:
    TCriticalSection CritSec;
};

//
/// \class TStaticSync
// template <class T> class TStaticSync
// template <class T> class TStaticSync::TLock
// ~~~~~~~~ ~~~~~~~~~ ~~~~~ ~~~~~~~~~~~~~~~~~
/// TStaticSync provides a system-independent interface to build sets of classes
/// that act somewhat like monitors, i.e., classes in which only one member
/// function can execute at any one time regardless of which instance it is
/// being called on. TStaticSync uses TCriticalSection, so it is portable to all
/// platforms that TCriticalSection has been ported to.
//
// TStaticSync Public Interface
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// None. TStaticSync can only be a base class.
//
// TStaticSync Protected Interface
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     TStaticSync<T>(const TStaticSync<T>&);
//                             Copy constructor. Does not copy the
//                             TCriticalSection object.
//
//     const TStaticSync<T>& operator =(const TStaticSync<T>&);
//                             Assignment operator. Does not copy the
//                             TCriticalSection object.
//
//     class TLock;            Handles locking and unlocking of member
//                             functions.
//
/// Example
// ~~~~~~~
/// \code
///     class ThreadSafe : private TStaticSync<ThreadSafe>
///     {
///       public:
///         void f();
///         void g();
///       private:
///         static int i;
///     };
///
///     void ThreadSafe::f()
///     {
///       TLock lock;
///       if (i == 2)
///         i = 3;
///     }
///
///     void ThreadSafe::g()
///     {
///       TLock lock;
///       if (i == 3)
///         i = 2;
///     }
/// \endcode
//
template <class T> class TStaticSync{
  protected:
    TStaticSync();
    TStaticSync(const TStaticSync<T>&);
    const TStaticSync<T>& operator =(const TStaticSync<T>&);
   ~TStaticSync();

    class TLock : private TCriticalSection::TLock
    {
      public:
        TLock() : TCriticalSection::TLock(*TStaticSync<T>::CritSec) {}
    };
    friend class TLock;
    typedef TLock Lock;  ///< For compatibility with old T-less typename

  private:
    static TCriticalSection* CritSec;
    static ulong Count;
};

//
/// \class TThread
// ~~~~~ ~~~~~~~
/// TThread provides a system-independent interface to threads. With
/// suitable underlying implementations the same code can be used under
/// OS/2 and Win32.
///
/// Example
/// \code
///     class TimerThread : public TThread
///     {
///       public:
///         TimerThread() : Count(0) {}
///       private:
///         int Run();
///         int Count;
///     };
///
///     int TimerThread::Run()
///     {
///       // loop 10 times
///       while (Count++ < 10) {
///         Sleep(1000);    // delay 1 second
///         cout << "Iteration " << Count << endl;
///       }
///       return 0;
///     }
///
///     int main()
///     {
///       TimerThread timer;
///       timer.Start();
///       Sleep(20000);     // delay 20 seconds
///       return 0;
///     }
/// \endcode
///
/// Internal States
/// - Created:    the object has been created but its thread has not been
///             started. The only valid transition from this state is
///             to Running, which happens on a call to Start(). In
///             particular, a call to Suspend() or Resume() when the
///             object is in this state is an error and will throw an
///             exception.
///
/// - Running:    the thread has been started successfully. There are two
///             transitions from this state:
///
///                 When the user calls Suspend() the object moves into
///                 the Suspended state.
///
///                 When the thread exits the object moves into the
///                 Finished state.
///
///             Calling Resume() on an object that is in the Running
///             state is an error and will throw an exception.
///
/// -  Suspended:  the thread has been suspended by the user. Subsequent
///             calls to Suspend() nest, so there must be as many calls
///             to Resume() as there were to Suspend() before the thread
///             actually resumes execution.
///
/// - Finished:   the thread has finished executing. There are no valid
///             transitions out of this state. This is the only state
///             from which it is legal to invoke the destructor for the
///             object. Invoking the destructor when the object is in
///             any other state is an error and will throw an exception.
//
class _OWLCLASS TThread
{
  public:
    enum { NoLimit = INFINITE };
    typedef HANDLE THandle;

    // Attach to the current running (often primary) thread
    //
    enum TCurrent {Current};
    TThread(TCurrent);

    /// Identifies the states that the class can be in.
    enum TStatus 
    { 
    	Created, 		///< The class has been created but the thread has not been started.
    	Running, 		///< The thread is running.
    	Suspended,	///< The thread has been suspended. 
    	Finished,		///< The thread has finished execution. 
    	Invalid 		///< The object is invalid. Currently this happens only when the operating system is unable to start the thread.
    };

    THandle Start();
    ulong   Suspend();
    ulong   Resume();
    bool    Sleep(long timeMS, bool alertable = false);

    virtual void    Terminate();
    ulong   WaitForExit(ulong timeout = NoLimit);
    ulong   TerminateAndWait(ulong timeout = NoLimit);

    TStatus GetStatus() const;
    uint32  GetExitCode() const;

    enum TPriority {
      Idle         = THREAD_PRIORITY_IDLE,          ///< -15
      Lowest       = THREAD_PRIORITY_LOWEST,        ///<  -2
      BelowNormal  = THREAD_PRIORITY_BELOW_NORMAL,  ///<  -1
      Normal       = THREAD_PRIORITY_NORMAL,        ///<   0
      AboveNormal  = THREAD_PRIORITY_ABOVE_NORMAL,  ///<   1
      Highest      = THREAD_PRIORITY_HIGHEST,       ///<   2
      TimeCritical = THREAD_PRIORITY_TIME_CRITICAL, ///<  15
    };
      
    int GetPriority() const;
    int SetPriority(int);   ///< Can pass a TPriority for simplicity

		/// The error class that defines the objects that are thrown when an error occurs.
    class TThreadError : public TXBase  {
      public:
      	/// Identifies the type of error that occurred.
        enum TErrorType {
          SuspendBeforeRun,		///< The user called Suspend() on an object before calling Start().
          ResumeBeforeRun,		///< The user called Resume() on an object before calling Start().
          ResumeDuringRun,		///< The user called Resume() on a thread that was not Suspended.
          SuspendAfterExit,		///< The user called Suspend() on an object whose thread had already exited.
          ResumeAfterExit,		///< The user called Resume() on an object whose thread had already exited.
          CreationFailure,		///< The operating system was unable to create the thread.
          DestroyBeforeExit,	///< The object's destructor was invoked its thread had exited.
          AssignError,				///< An attempt was made to assign to an object that was not in either the Created or the Finished state.
          NoMTRuntime,				///< This usually results when you attempt to execute a multithread
															///< application which includes a module that has not been properly 
															///< compiled with the appropriate multithread options.

        };
        typedef TErrorType ErrorType;
        TErrorType GetErrorType() const;

      private:
        TThreadError(TErrorType type);
        static tstring MakeString(ErrorType type);
        TErrorType Type;
      friend class _OWLCLASS TThread;
    };

    typedef TStatus Status;     ///< For compatibility with old T-less typenames
    typedef TThreadError ThreadError;

  protected:
    TThread();          ///< Create a thread. Derived class overrides Run()

    // Copying a thread puts the target into the Created state
    //
    TThread(const TThread&);
    const TThread& operator =(const TThread&);

    virtual ~TThread();

    bool    ShouldTerminate() const;
    void    Exit(ulong code);  ///< Alternative to returning from Run()

  private:
    virtual int Run();

    TStatus CheckStatus() const;

#if defined(BI_MULTI_THREAD_RTL)
#if  defined(BI_COMP_BORLANDC)
    static void _USERENTRY Execute(void* thread);
#else
    static uint __stdcall Execute(void* thread);
#endif
#else
    static DWORD WINAPI Execute(void* thread);
#endif

    DWORD ThreadId;
    THandle Handle;
    mutable TStatus Stat;
    bool TerminationRequested;
    bool Attached;
};

#if defined(BI_MULTI_THREAD_RTL)

//
/// All classes used in local/tls process data have to be derived from here.
//
class TLocalObject : public TStandardAllocator {
  public:
    TLocalObject(){}
    virtual ~TLocalObject(){}
};

//
/// Exception class for TTlsContainer
//
class TXTlsContainer : public TXBase
{
public:

  TXTlsContainer(const tstring& msg)
    : TXBase(msg)
  {}
};

//
/// Thread-local storage container
/// Wraps a used-defined type, and provides separate instances of this type to each client thread.
/// The wrapped type must be default-constructible.
///
/// This class is normally used as a singleton.
/// Note that the construction of a singleton of this class must be synchronized (serialized execution).
/// A simple way to ensure this is to initialize it at program startup (single-threaded execution).
///
/// Usage:
///
/// \code
/// TFoo& GetFoo() // Meyers Singleton ensures global-initialization-order independency.
/// {
///   static TTlsContainer<TFoo> c;
///   return c.GetData(); // OWL5_COMPAT: return c; // implicit conversion
/// }
///
/// TFoo& init = GetFoo(); // Ensures initialization during start-up (i.e. single-threaded and safe).
/// \endcode
//
template <class T>
class TTlsContainer
{
public:

  TTlsContainer() 
    : Index(::TlsAlloc())
  {
    TRACEX(OwlThread, 1, "Initializing " << TraceId(this));
    if (Index == TLS_OUT_OF_INDEXES) 
      throw TXTlsContainer(_T("TLS_OUT_OF_INDEXES"));
  }

  //
  /// Deallocates all the thread-level data created by the container.
  //
  ~TTlsContainer()
  {
    TRACEX(OwlThread, 1, "Destructing " << TraceId(this));
    ::TlsFree(Index);
    TCriticalSection::TLock lock(GarbageSection);
    std::for_each(Garbage.begin(), Garbage.end(), &Delete);
  }

  //
  /// Returns 0 if data for the calling thread has not yet been allocated, 
  /// otherwise a pointer to previously initialized data is returned.
  /// If the function returns 0, you can call Get to create an instance.
  /// Can safely be called by many threads simultaneously without synchronization.
  //
  T* Query()
  {return static_cast<T*>(::TlsGetValue(Index));}

  //
  /// Returns the thread-local instance of the data.
  /// Can safely be called by many threads simultaneously without synchronization.
  //
  T& Get()
  {
    T* p = Query();
    return p ? *p : Create();
  }

  //
  /// Const overload
  //
  const T* Query() const
  {return const_cast<TTlsContainer*>(this)->Query();}

  //
  /// Const overload
  //
  const T& Get() const
  {return const_cast<TTlsContainer*>(this)->Get();}

  // Old interface for backwards compatibility:

#if defined(OWL5_COMPAT) 

  T* QueryData()
  {return Query();}

  T* GetData()
  {return &Get();}

  operator T*() 
  {return &Get();}

  operator T&()
  {return Get();}

  T* operator->()
  {return &Get();}

#endif

protected:

  const DWORD Index;
  TCriticalSection GarbageSection;
  std::vector<T*> Garbage;

  T& Create()
  {
    T* p = new T(); CHECK(p);
    bool r = ::TlsSetValue(Index, p); CHECK(r); InUse(r);
    TRACEX(OwlThread, 1, TraceId(this) << " created " << TraceId(p) << " for thread " << GetCurrentThreadId());

    // Store the pointer for garbage collection later.
    //
    TCriticalSection::TLock lock(GarbageSection);
    Garbage.push_back(p);
    return *p;
  }

  static void Delete(T* p)
  {
    delete p;
    TRACEX(OwlThread, 1, TraceType<TTlsContainer>() << " deleted " << TraceId(p));
  }

private:

  TTlsContainer(const TTlsContainer&); // prohibit
  TTlsContainer& operator =(const TTlsContainer&); // prohibit

};

//
// The following TLS support classes are only available in OWL 5 compatibility mode.
//

#if defined(OWL5_COMPAT)

//
// Created by Yura Bidus
// Multithread OWL internal support
//

class TLocalData: public TLocalObject {
  typedef TMIPtrArray<TLocalObject*,TLocalObject*,TLocalObject>  TLocalObjectArray;
  public:
    TLocalData(){}
    virtual ~TLocalData(){}

    TLocalObject* GetData(int index)  
      { 
        return index < (int)Data.Size() ? Data[index] : 0;
      }

    void SetData(int index, TLocalObject* obj);

    int  AddData(TLocalObject* data) { return Data.Add(data); }

  protected:
    TLocalObjectArray Data;
};

class TTlsAllocator {
  public:
    TTlsAllocator()              { index = ::TlsAlloc();}
    ~TTlsAllocator()            { ::TlsFree(index);    }

    bool  IsOk()                { return index != 0xFFFFFFFF; }
    bool  SetValue(void* value)  { return ::TlsSetValue(index, value); }
    void* GetValue()            { return ::TlsGetValue(index);        }

  private:
    uint32 index;
};

class TTlsDataManager : public TLocalObject {
  typedef TMIPtrArray<TLocalData*,TLocalData*,TLocalObject>  TLocalDataArray;
  public:
    TTlsDataManager()
      { 
        CHECK(Allocator.IsOk()); 
      }
    virtual ~TTlsDataManager()
      {
        TMRSWSection::TLock __lock(Lock,false,true);
        Data.Flush(true);
      }

    TLocalObject* GetData(int index)
      { 
        TLocalData* data = (TLocalData*)Allocator.GetValue();
        return (data ? data : CreateData())->GetData(index);
      }
    void SetData(int index, TLocalObject* obj)
      {
        TLocalData* data = (TLocalData*)Allocator.GetValue();
        (data ? data : CreateData())->SetData(index, obj);
      }
    int AddData(TLocalObject* obj)
      {
        TLocalData* data = (TLocalData*)Allocator.GetValue();
        return (data ? data : CreateData())->AddData(obj);
      }
  protected:
    TLocalData* CreateData()
      {
        TMRSWSection::TLock __lock(Lock,false,true);
        TLocalData* data = new TLocalData();
        Data.Add(data);
        Allocator.SetValue(data); 
        return data;
      }

  protected: // global static data 
    TTlsAllocator    Allocator;
    TLocalDataArray  Data;
    TMRSWSection    Lock;
};

template <class T> class TProcessContainer: public TLocalObject{
  public:
    TProcessContainer():Object(0){}
    virtual ~TProcessContainer(){ delete Object;}

    inline T* GetData()
      {
        if(!Object) // throw bad_alloc if error ??
          Object = new T;
        // check that T derived from TLocalObject
        CHECK(dynamic_cast<TLocalObject*>(Object));
        return Object;
      }
    inline T* QueryData()  {  return Object;    }
    inline operator T*()  { return GetData(); }
    inline operator T&()  { return *GetData();}
    inline T* operator->(){ return GetData(); }

  protected:
    T* Object;
};

#endif

#endif

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementation
//


//----------------------------------------
// TSemaphore Win32

//
inline TSemaphore::~TSemaphore()
{
  ::CloseHandle(Handle);
}

//
inline TSemaphore::operator TSemaphore::THandle() const
{
  return Handle;
}

//
inline TSemaphore::TLock::TLock(const TSemaphore& sem, ulong timeOut, bool alertable)
:
  Sem(0)
{
  if (::WaitForSingleObjectEx(sem, timeOut, alertable) == 0)
    Sem = &sem;
}

//
inline TSemaphore::TLock::~TLock()
{
  Release();
}

//
inline bool TSemaphore::TLock::WasAquired() const
{
  return Sem != 0;
}

//
// Release but hang on to the semaphore
//
inline void TSemaphore::TLock::Release(bool relinquish)
{
  if (Sem) {
    CONST_CAST(TSemaphore*,Sem)->Release();
    if (relinquish)
      Sem = 0;
  }
}

//----------------------------------------
// TMutex Win32

//

/// Constructs a TMutex instance encapsulating a newly created named or unnamed
/// mutex object. The name parameter points to a null-terminated string specifying
/// the name of the mutex object. The sa parameter points to a SECURITY_ATTRIBUTES
/// structure that specifies the security attributes of the mutex object.
/// \note The mutex object is not owned.
inline TMutex::TMutex(LPCTSTR name, LPSECURITY_ATTRIBUTES sa)
 : Shared(false)
{
  Handle = ::CreateMutex(sa, false, name);  // don't aquire now
  if (Handle && (GetLastError() == ERROR_ALREADY_EXISTS))
    Shared = true;
}

//
/// String-aware overload
//
inline TMutex::TMutex(const tstring& name, LPSECURITY_ATTRIBUTES sa)
 : Shared(false)
{
  Handle = ::CreateMutex(sa, false, name.c_str());  // don't aquire now
  if (Handle && (GetLastError() == ERROR_ALREADY_EXISTS))
    Shared = true;
}

//
/// Open an existing mutex. Fails if not there.
//
/// This form of the constructor instantiates a TMutex object that encapsulates an
/// existing named mutex object. The name parameter points to a null-terminated
/// string that names the mutex to be opened.
inline TMutex::TMutex(LPCTSTR name, bool inherit, uint32 access)
  : Shared(false)
{
  Handle = ::OpenMutex(access, inherit, name);
  if (Handle)
    Shared = true;
}

//
/// String-aware overlaod
//
inline TMutex::TMutex(const tstring& name, bool inherit, uint32 access)
  : Shared(false)
{
  Handle = ::OpenMutex(access, inherit, name.c_str());
  if (Handle)
    Shared = true;
}

//
/// Constructs a Tmutex instance encapsulating a handle obtained by duplicating the
/// specified handle. The handle parameter is the source mutex object to be
/// duplicated.
/// \note The duplicated handle has the same access attributes as the source handle.
//
inline TMutex::TMutex(THandle handle)
  : Shared(false)
{
  if (::DuplicateHandle(::GetCurrentProcess(), handle,
                        ::GetCurrentProcess(), &Handle,
                        0, false, DUPLICATE_SAME_ACCESS))
  {
    Shared = true;
  }
}

//
//
//
inline bool TMutex::IsShared()
{
  return Shared;
}

//----------------------------------------
// TCountedSemaphore Win32

//
inline TCountedSemaphore::TCountedSemaphore(int initialCount, int maxCount,
                                            LPCTSTR name,
                                            LPSECURITY_ATTRIBUTES sa)
{
  Handle = ::CreateSemaphore(sa, initialCount, maxCount, name);
}

//
// String-aware overload
//
inline TCountedSemaphore::TCountedSemaphore(int initialCount, int maxCount, const tstring& name, LPSECURITY_ATTRIBUTES sa)
{
  Handle = ::CreateSemaphore(sa, initialCount, maxCount, name.c_str());
}

//
inline TCountedSemaphore::TCountedSemaphore(LPCTSTR name, bool inherit,
                                            uint32 access)
{
  Handle = ::OpenSemaphore(access, inherit, name);
}

//
// String-aware overload
//
inline TCountedSemaphore::TCountedSemaphore(const tstring& name, bool inherit, uint32 access)
{
  Handle = ::OpenSemaphore(access, inherit, name.c_str());
}

//
inline TCountedSemaphore::TCountedSemaphore(THandle handle)
{
  ::DuplicateHandle(::GetCurrentProcess(), handle,
                    ::GetCurrentProcess(), &Handle,
                    0, false, DUPLICATE_SAME_ACCESS);
}

//----------------------------------------
// TEventSemaphore Win32

inline TEventSemaphore::TEventSemaphore(bool manualReset, bool initialState,
                                        LPCTSTR name,
                                        LPSECURITY_ATTRIBUTES sa)
{
  Handle = ::CreateEvent(sa, manualReset, initialState, name);
}

//
// String-aware overload
//
inline TEventSemaphore::TEventSemaphore(bool manualReset, bool initialState, const tstring& name, LPSECURITY_ATTRIBUTES sa)
{
  Handle = ::CreateEvent(sa, manualReset, initialState, name.c_str());
}

//
inline TEventSemaphore::TEventSemaphore(LPCTSTR name, bool inherit,
                                        uint32 access)
{
  Handle = ::OpenEvent(access, inherit, name);
}

//
// String-aware overload
//
inline TEventSemaphore::TEventSemaphore(const tstring& name, bool inherit, uint32 access)
{
  Handle = ::OpenEvent(access, inherit, name.c_str());
}

//
inline TEventSemaphore::TEventSemaphore(THandle handle)
{
  ::DuplicateHandle(::GetCurrentProcess(), handle,
                    ::GetCurrentProcess(), &Handle,
                    0, false, DUPLICATE_SAME_ACCESS);
}

inline void TEventSemaphore::Set()
{
  ::SetEvent(*this);
}

inline void TEventSemaphore::Reset()
{
  ::ResetEvent(*this);
}

inline void TEventSemaphore::Pulse()
{
  ::PulseEvent(*this);
}

//----------------------------------------
// TWaitableTimer Win NT or Win98
inline TWaitableTimer::TWaitableTimer(THandle handle)
{
  ::DuplicateHandle(::GetCurrentProcess(), handle, ::GetCurrentProcess(), 
                    &Handle, 0, false, DUPLICATE_SAME_ACCESS);
}

inline  TWaitableTimer::~TWaitableTimer()
{
  ::CloseHandle(Handle);
}
//----------------------------------------
// TSemaphoreSet Win32

inline int TSemaphoreSet::GetCount() const
{
  return Count;
}

inline const TSemaphore* TSemaphoreSet::operator [](int index) const
{
  return (index >= 0 && index < Count) ? Sems[index] : 0;
}

//
inline bool TSemaphoreSet::TLock::WasAquired() const
{
  return Set != 0;
}

//
/// Which one was locked, all locked code, or lock fail code
//
inline int TSemaphoreSet::TLock::WhichAquired() const
{
  return Locked;
}
       

//----------------------------------------------------------------------------
// TCriticalSection Win32
//

//
/// Use system call to initialize the CRITICAL_SECTION object.
//
inline TCriticalSection::TCriticalSection()
{
  ::InitializeCriticalSection(CONST_CAST(CRITICAL_SECTION*,&CritSec));
}

//
/// Use system call to destroy the CRITICAL_SECTION object.
//
inline TCriticalSection::~TCriticalSection()
{
  ::DeleteCriticalSection(CONST_CAST(CRITICAL_SECTION*,&CritSec));
}

//
/// Use system call to lock the CRITICAL_SECTION object.
//
/// Requests a lock on the TCriticalSection object. If no other Lock object holds a
/// lock on that TCriticalSection object, the lock is allowed and execution
/// continues. If another Lock object holds a lock on that object, the requesting
/// thread is blocked until the lock is released.
//
inline TCriticalSection::TLock::TLock(const TCriticalSection& sec)
:
  CritSecObj(sec)
{
  ::EnterCriticalSection(CONST_CAST(CRITICAL_SECTION*,&CritSecObj.CritSec));
}

//
/// Use system call to unlock the CRITICAL_SECTION object.
//
inline TCriticalSection::TLock::~TLock()
{
  ::LeaveCriticalSection(CONST_CAST(CRITICAL_SECTION*,&CritSecObj.CritSec));
}

//
// TCriticalSection OS2
//


//----------------------------------------------------------------------------

//
/// Default constructor.
inline TSync::TSync()
{
}

//
/// Copy constructor does not copy the TCriticalSection object,
/// since the new object is not being used in any of its own
/// member functions. This means that the new object must start
/// in an unlocked state.
//
inline TSync::TSync(const TSync&)
{
}

//
/// Does not copy the TCriticalSection object, since the new  object is not
/// being used in any of its own member functions.
/// This means that the new object must start in an unlocked state.
//
inline const TSync& TSync::operator =(const TSync&)
{
  return *this;
}

//
/// Locks the TCriticalSection object in the TSync object.
//
/// Requests a lock on the critical section of the TSync object pointed to by s. If
/// no other Lock object holds a lock on that TCriticalSection object, the lock is
/// allowed and execution continues. If another Lock object holds a lock on that
/// object, the requesting thread is blocked until the lock is released.
inline TSync::TLock::TLock(const TSync* sync)
:
  TCriticalSection::TLock(GetRef(sync))
{
}

//
// Returns a reference to the TCriticalSection object contained in the TSync
// object.
//
// In the diagnostic version, checks for a null pointer.
//
inline const TCriticalSection& TSync::TLock::GetRef(const TSync* sync)
{
  CHECK(sync != 0);
  return sync->CritSec;
}

//----------------------------------------------------------------------------

//
// Instantiate the data members.
//
template <class T> TCriticalSection* TStaticSync<T>::CritSec;
template <class T> ulong TStaticSync<T>::Count;

//
// If this is the first TStaticSync<T> object to be constructed, create the
// semaphore.
//
// The copy constructor only has to increment the count, since there will
// already be at least one TStaticSync<T> object, namely, the one being copied.
//
template <class T> inline TStaticSync<T>::TStaticSync()
{
  if (Count++ == 0)
    CritSec = new TCriticalSection;
}

template <class T> inline TStaticSync<T>::TStaticSync(const TStaticSync<T>&)
{
  Count++;
}

//
// TStaticSync<T> assignment operator
//
template <class T>
inline const TStaticSync<T>& TStaticSync<T>::operator =(const TStaticSync<T>&)
{
  return *this;
}

//
// If this is the only remaining TStaticSync<T> object, destroy the semaphore.
//
template <class T> inline TStaticSync<T>::~TStaticSync()
{
  if (--Count == 0)
    delete CritSec;
}

//----------------------------------------------------------------------------

//
/// Suspends execution of the current thread for the number of milliseconds
/// specified by the timeMS parameter. If the alertable parameter is set to true,
/// the function resumes execution upon I/O completion.
///
/// The function returns true if its wakeup is due to I/O completion.
//
inline bool TThread::Sleep(long timeMS, bool alertable)
{
  return ::SleepEx(timeMS, alertable) == WAIT_IO_COMPLETION;
}

//
/// Gets the current status of the thread.
//
/// If the thread is marked as Running it may have terminated without our
/// knowing it, so we have to check.
//
inline TThread::TStatus TThread::GetStatus() const
{
  if (Stat == Running)
    Stat = CheckStatus();
  return Stat;
}

//
inline uint32  TThread::GetExitCode() const
{
  uint32 code;
  ::GetExitCodeThread(Handle, (DWORD*)&code);
  return code;
}

//
/// Gets the thread priority. Under Win32, this is a direct call to the operating
/// system. See the description of TPriority for possible values.
//
inline int TThread::GetPriority() const
{
  return ::GetThreadPriority(Handle);
}

//
/// Returns a bool value to indicate that Terminate() or TerminateAndWait()
/// has been called. If this capability is being used, the thread should call
/// ShouldTerminate() regularly, and if it returns a non-zero value the thread
/// finish its processing and exit.
//
inline bool TThread::ShouldTerminate() const
{
  return TerminationRequested;
}

/// Returns a code indicating the type of error that occurred.
inline TThread::TThreadError::TErrorType TThread::TThreadError::GetErrorType() const
{
  return Type;
}

} // OWL namespace

#endif  // OWL_THREAD_H

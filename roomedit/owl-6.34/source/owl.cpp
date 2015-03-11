//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Internal window object instance proc creation & maintenance.
//----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/window.h>
#include <owl/private/memory.h>
#include <stddef.h>

#if !defined(__CYGWIN__) && !defined(WINELIB)
# include <dos.h>
#endif

#if defined(BI_MULTI_THREAD_RTL)
# include <owl/thread.h>
#endif

#if defined(__BORLANDC__)
# pragma option -w-amp // Disable "Superfluous & with function"
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-inl // Disable "Function containing 'statment' is not expanded inline"
#endif

namespace owl {

//------------------------------------------------------------------------------

//
// Diagnostics
//
OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlWin);
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlThunk, 0, 2);

//
// Returns the internal version number. See "owl/version.h".
//
_OWLFUNC(uint32)
OWLGetVersion()
{
  return OWLInternalVersion;
}

//
// Returns a pointer to the global OWL module object.
//
_OWLFUNC(TModule*)
OWLGetModule()
{
  return &GetGlobalModule();
}

//----------------------------------------------------------------------------

//
// This internal singleton class encapsulates the global "creation window" pointer.
// The creation window is the window currently being constructed/initialized.
// A thread-safe implementation is used for multi-threaded builds.
//
class TCreationWindow
{
  TCreationWindow() {} // Private to prohibit instantiation.

#if defined(BI_MULTI_THREAD_RTL)

  //
  // Thread-safe encapsulation of a creation window pointer.
  //
  struct TInstance : public TLocalObject
  {
    TInstance() : Window(0) {}
    TWindow* Window;
  };

#endif

public:
  //
  // Get a reference to the pointer to the creation window.
  // Returns the pointer to the current window being created in this thread.
  //
  static TWindow*& GetInstance();
};

TWindow*& TCreationWindow::GetInstance() 
{
#if defined(BI_MULTI_THREAD_RTL)

  //
  // Note that while this lazy initialization avoids problems with global initialization order,
  // the initial call of this function is not thread-safe (pre C++11). As a work-around, we 
  // ensure this function is called during program start-up (single-thread, safe). 
  // See InitCreationWindowInstance below.
  //
  // The work-around can be removed when C++11 compliant compilers are mandated.
  //
  // Also note that the singleton construction may throw an exception. Since there is no way to 
  // continue without this singleton, we make no attempt to handle it here. We assume that 
  // the exception will terminate the program, or if it is handled, that subsequent calls will
  // be retried still within a single thread.
  //
  static TTlsContainer<TInstance> container;
  return container.Get().Window;

#else
  static TWindow* creationWindow = 0;
  return creationWindow;
#endif
}
  
namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TWindow*& InitCreationWindowInstance = TCreationWindow::GetInstance(); 
}

//
// Accessor of the "creation window", the global pointer to the current window being created.
// Internal function.
//
_OWLFUNC(TWindow*)
GetCreationWindow()
{
  return TCreationWindow::GetInstance();
}

//
// Set the "creation window", the global pointer to the current window being created.
// Internal function.
//
_OWLFUNC(void)
SetCreationWindow(TWindow* w)
{
  // Theoretically, there should always be only one non-zero
  // "creation window" pointer. i.e. we never want to have a case where
  // we're holding on to a pointer waiting to be thunked, and this function
  // is invoked with yet another valid pointer (Otherwise, we'll fail to
  // thunk a HWND<->TWindow* pair.
  //
  PRECONDITION(GetCreationWindow() == 0 || w == 0);
  TRACEX(OwlWin, 1, _T("SetCreationWindow: Old(") << (void*)GetCreationWindow() <<\
                    _T("), New(") << (void*)w << _T(')'));
  TCreationWindow::GetInstance() = w;
}

//
// Global GetWindowPtr() message ID used for registered message
//
_OWLDATA(uint) GetWindowPtrMsgId = 0;


//----------------------------------------------------------------------------

//
// Returns the correct default window proc, depending on build options.
// Internal function.
//
WNDPROC 
GetDefWindowProc()
{
  // For Win32, find the default window proc in the right DLL for this build.

# if defined(UNICODE)
#   define DEFWINDOWPROC "DefWindowProcW"
# else
#   define DEFWINDOWPROC "DefWindowProcA"
# endif

#define DEFWINDOWPROC_MODULE _T("USER32")

  HMODULE module = GetModuleHandle(DEFWINDOWPROC_MODULE);
  return reinterpret_cast<WNDPROC>(GetProcAddress(module, DEFWINDOWPROC));
}

//
/// Callback process for hooking TWindow to native window.
///
/// Initial WndProc called when an OWL window is first created.
/// Subclasses the window function by installing the window proc then calls the
/// window proc to get this first message to the window.
//
LRESULT CALLBACK
TWindow::InitWndProc(HWND hWnd, UINT msg, WPARAM param1, LPARAM param2)
{
  // Get the creation window (i.e. the current TWindow being initialized at this point).
  // If there's no creation window, i.e. we'll be aliasing a resource, then we
  // can't do anything now.  Create() will take care of it later.
  //
  TWindow* w = GetCreationWindow();
  if (!w)
    return ::DefWindowProc(hWnd, msg, param1, param2);

  // Reset the creation window so that it is not inadvertently used again.
  //
  SetCreationWindow(0);

  // Initialize the  window.
  // Assign a default window proc before we subclass,
  // otherwise we would subclass InitWndProc which would be bad.
  //
  w->SetHandle(hWnd);
  w->SetWindowProc(GetDefWindowProc());
  w->SubclassWindowFunction(); // Install the instance window proc.

  // Call the instance window proc.
  //
  WNDPROC wndproc = w->GetWindowProc();
  return (*wndproc)(hWnd, msg, param1, param2);
}

//----------------------------------------------------------------------------

//
// Implementation of the instance window proc interface using lookup
//
#if defined(BI_NOTHUNK)

//
/// The name of the Window Property used to associate a HWND and a TWindow object.
//
const char OwlObjectProperty [] = "OWL_TWINDOW"; // NB! Should be an ANSI string, not UNICODE.

//
/// Returns a common instance proc for all windows. 
//
WNDPROC
TWindow::CreateInstanceProc()
{
  struct TLocal
  {
    //
    // This common window procedure is used instead of thunks. It forwards the message to the
    // owning TWindow instance using lookup.
    //
    static LRESULT CALLBACK CommonWndProc(HWND hWnd, UINT msg, WPARAM p1, LPARAM p2)
    {
      // Lookup the window pointer.
      //
      HANDLE hObj = ::GetPropA(hWnd, OwlObjectProperty);
      WARNX(OwlThunk, !hObj, 0, _T("CommonWndProc: Handle lookup failed! GetLastError: ") << ::GetLastError());
      TWindow* w = reinterpret_cast<TWindow*>(hObj);
      CHECK(w);

      // Dispatch the message to the window.
      //
      return w->ReceiveMessage(hWnd, msg, p1, p2);
    }
  };
  return &TLocal::CommonWndProc;
}

//
/// Creates an association between the window and its handle.
//
void
TWindow::InitInstanceProc()
{
  PRECONDITION(GetHandle());
  BOOL r = ::SetPropA(GetHandle(), OwlObjectProperty, reinterpret_cast<HANDLE>(this));
  WARNX(OwlThunk, !r, 0, _T("InitInstanceProc: Handle registration failed! GetLastError: ") << ::GetLastError());
  CHECK(r);
}

// 
/// Removes the association between the window and its handle.
//
void
TWindow::FreeInstanceProc()
{
  HANDLE h = ::RemovePropA(GetHandle(), OwlObjectProperty);
  WARNX(OwlThunk, !h, 0, _T("FreeInstanceProc: Handle removal failed! GetLastError: ") << ::GetLastError());
}

#else // nothunk/thunk

//----------------------------------------------------------------------------

//
// Diagnostics macros for the allocator
// We keep track of the current and maximum number of thunks allocated.
//
#if defined(__WARN)

#define THUNK_ALLOCATOR_DIAGNOSTICS_VARIABLES\
  unsigned Count;\
  unsigned Maximum;

#define THUNK_ALLOCATOR_DIAGNOSTICS_INITIALIZATION\
  , Count(0), Maximum(0)

#define THUNK_ALLOCATOR_DIAGNOSTICS_UPDATE(i)\
  {\
    if ((Count += i) > Maximum)\
    {\
      Maximum = Count;\
      WARNX(OwlThunk, true, 2, _T("New thunk count record: ") << Maximum);\
    }\
    CHECKX(Count >= 0, _T("Negative thunk count!"));\
  };

#else // no diagnostics

#define THUNK_ALLOCATOR_DIAGNOSTICS_VARIABLES
#define THUNK_ALLOCATOR_DIAGNOSTICS_INITIALIZATION
#define THUNK_ALLOCATOR_DIAGNOSTICS_UPDATE(i)

#endif


//
// This singelton class provides a memory heap for thunks. 
// The implementation is compatible with DEP (Data Execution Prevention).
//
class TThunkAllocator 
{
public:
  //
  // Provides access to the singleton allocator.
  //
  static TThunkAllocator& GetInstance()
  {
    //
    // Note that while this lazy initialization avoids problems with global initialization order,
    // the initial call of this function is not thread-safe (pre C++11). As a work-around, we 
    // ensure this function is called during program start-up (single-thread, safe). 
    // See InitThunkAllocatorInstance below.
    //
    // The work-around can be removed when C++11 compliant compilers are mandated.
    //
    // Also note that allocator construction may throw an exception. Since there is no way to 
    // continue without a thunk allocator, we make no attempt to handle it here. We assume that 
    // the exception will terminate the program, or if it is handled, that subsequent calls will
    // be retried still within a single thread.
    //
    static TThunkAllocator TheAllocator; // initial call (construction) not thread-safe pre-C++11
    return TheAllocator;
  }

  //
  // Destructor - destroys the heap.
  //
  ~TThunkAllocator()
  {
    PRECONDITION(Handle);
    BOOL r = ::HeapDestroy(Handle);
    WARNX(OwlThunk, !r, 0, _T("HeapDestroy failed! GetLastError: ") << ::GetLastError());
    TRACEX(OwlThunk, 1, _T("TThunkAllocator destructed @") << Handle);
  }

  //
  // Allocates memory from the private heap.
  //
  LPVOID Allocate(size_t size)
  {
    PRECONDITION(Handle);
    LPVOID p = ::HeapAlloc(Handle, 0, size);
    WARNX(OwlThunk, !p, 0, _T("HeapAlloc failed!"));
    if (!p) TXOutOfMemory().Throw();
    THUNK_ALLOCATOR_DIAGNOSTICS_UPDATE(1);
    return p;
  }

  //
  // Frees memory previously allocated from the private heap.
  //
  void Free(LPVOID p)
  {
    PRECONDITION(Handle);
    BOOL r = ::HeapFree(Handle, 0, p);
    WARNX(OwlThunk, !r, 0, _T("HeapFree failed! GetLastError: ") << ::GetLastError());
    THUNK_ALLOCATOR_DIAGNOSTICS_UPDATE(-1);
  }

private:
  HANDLE Handle; // Handle for the private heap (returned by HeapCreate)
  THUNK_ALLOCATOR_DIAGNOSTICS_VARIABLES

  //
  // Heap creation parameters
  //
  enum
  {
    Flags = 0x00040000, // = HEAP_CREATE_ENABLE_EXECUTE, // Allow thunks to run.
    InitialSize = 32768, // Room for 1024 32-byte thunks.
    MaxSize = 0, // Set no limits, i.e. dynamic growth. (Any benefits to limiting it?)
  };

  //
  // Private constructor - creates the heap.
  //
  TThunkAllocator()
    : Handle(::HeapCreate(Flags, InitialSize, MaxSize))
      THUNK_ALLOCATOR_DIAGNOSTICS_INITIALIZATION
  {
    WARNX(OwlThunk, !Handle, 0, _T("HeapCreate failed! GetLastError: ") << ::GetLastError());
    if (!Handle) TXOutOfMemory().Throw();
    TRACEX(OwlThunk, 1, _T("TThunkAllocator constructed @") << Handle);
  }

  //
  // Prohibit copying (i.e. not implemented)
  //
  TThunkAllocator(const TThunkAllocator&);
  TThunkAllocator& operator=(const TThunkAllocator&);
};

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TThunkAllocator& InitThunkAllocatorInstance = TThunkAllocator::GetInstance();
}

//
// This class template defines a thunk that contains an embedded TWindow pointer
// and dispatch function address.
//
template 
<
  const uint8* Initializer, size_t Size, 
  void* (*GetMessageReceiverAddress)(), int DispatchAddressIndex, int InstanceAddressIndex
>
struct TThunkT
{
  struct TFactory
  {
    struct TCode {uint8 bytes[Size];};
    TCode CodeTemplate;

    // 
    // Links the code template to the dispatch function.
    //
    TFactory() 
      : CodeTemplate(*reinterpret_cast<TCode*>(reinterpret_cast<void*>(const_cast<uint8*>(Initializer))))
    {*reinterpret_cast<void**>(&CodeTemplate.bytes[DispatchAddressIndex]) = (*GetMessageReceiverAddress)();}
  };

  typename TFactory::TCode Code;
  static const TFactory Factory;

  //
  // Permanently links this thunk to the given TWindow instance.
  //
  TThunkT(TWindow* w) 
    : Code(Factory.CodeTemplate)
  {*reinterpret_cast<void**>(reinterpret_cast<void*>(&Code.bytes[InstanceAddressIndex])) = w;}

  //
  // Class-local new operator - allocates virtual memory with DEP support.
  //
  void* operator new(size_t n)
  {
    LPVOID p = TThunkAllocator::GetInstance().Allocate (n);
    CHECK(p);
    TRACEX(OwlThunk, 1, _T("TThunk allocated @") << p);
    return p;
  }

  //
  // Class-local delete operator. Frees virtual memory.
  //
  void operator delete(void* p, size_t)
  {
    WARNX(OwlThunk, !p, 0, _T("TThunk::delete called with null pointer."));
    if (!p) return;
    TThunkAllocator::GetInstance().Free(p);
    TRACEX(OwlThunk, 1, _T("TThunk deallocated @") << p);
  }
};

//
// Static thunk factory
//
template <const uint8* i, size_t s, void* (*gpa)(), int dai, int iai>
const typename TThunkT<i, s, gpa, dai, iai>::TFactory 
TThunkT<i, s, gpa, dai, iai>::Factory;

//
// Thunk equivalent to the following function:
//
// LRESULT CALLBACK
// WndProcThunk(HWND, UINT msg, WPARAM param1, LPARAM param2)
// {
//   TWindow* w = WindowInstance; // hard-coded instance
//   return DispatchWindowMessage(w, msg, param1, param2);
// }
//
extern const uint8 ThunkInitializerForBorland32Bit[] = 
{
  0xB8, 0x00, 0x00, 0x00, 0x00,  // mov eax, 00000000h; instance address
  0x89, 0x44, 0x24, 0x04,  // mov [esp+4], eax; replace first argument
  0x68, 0x00, 0x00, 0x00, 0x00,  // push 00000000h; dispatch address
  0xC3  // ret
};

TResult CALLBACK DispatchWindowMessage(TWindow* w, TMsgId msg, TParam1 param1, TParam2 param2)
{return w->ReceiveMessage(w->GetHandle(), msg, param1, param2);}

void* GetDispatchHelper() 
{return &DispatchWindowMessage;}

typedef TThunkT
<
  ThunkInitializerForBorland32Bit, sizeof ThunkInitializerForBorland32Bit, 
  &GetDispatchHelper, 10, 1
> 
TThunkForBorland32Bit;

//
// Thunk equivalent to the following function:
//
// LRESULT CALLBACK
// WndProcThunk(HWND, UINT msg, WPARAM param1, LPARAM param2)
// {
//   TWindow* w = WindowInstance; // hard-coded instance
//   return w->ReceiveMessage(hwnd, msg, param1, param2);
// }
//
extern const uint8 ThunkInitializerForMicrosoft32Bit[] = 
{
  0xB9, 0x00, 0x00, 0x00, 0x00,  // mov ecx, 00000000h; instance address
  0x68, 0x00, 0x00, 0x00, 0x00,  // push 00000000h; dispatch address
  0xC3  // ret
};

//
// Provides access to the private member function in TWindow,
// to which we want to dispatch messages.
// Note: We assume that a member function pointer consists of an ordinary
// function pointer followed by extra stuff. We also assume that the
// member function is non-virtual.
//
void* GetMessageReceiverMemberFunctionAddress() 
{
  typedef TResult (TWindow::*TSignature)(HWND, TMsgId, TParam1, TParam2);
  TSignature f = &TWindow::ReceiveMessage;
  return *reinterpret_cast<void**>(&f);
}

typedef TThunkT
<
  ThunkInitializerForMicrosoft32Bit, sizeof ThunkInitializerForMicrosoft32Bit, 
  &GetMessageReceiverMemberFunctionAddress, 6, 1
> 
TThunkForMicrosoft32Bit;

//
// Thunk equivalent to the following function:
//
// LRESULT CALLBACK
// WndProcThunk(HWND, UINT msg, WPARAM param1, LPARAM param2)
// {
//   TWindow* w = WindowInstance; // hard-coded instance
//   return DispatchWindowMessage(w, msg, param1, param2);
// }
//
extern const uint8 ThunkInitializerForMicrosoft64Bit[] = 
{
  0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rcx, 0000000000000000h; dispatch address
  0x51,  // push rcx
  0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rcx, 0000000000000000h; instance address
  0xC3  // ret
};

typedef TThunkT
<
  ThunkInitializerForMicrosoft64Bit, sizeof ThunkInitializerForMicrosoft64Bit, 
  &GetDispatchHelper, 2, 13
> 
TThunkForMicrosoft64Bit;

//
// The following section selects the thunk types for the active platform.
//
#if defined(_M_AMD64)
typedef TThunkForMicrosoft64Bit TThunkForMicrosoft;
typedef TThunkForMicrosoft64Bit TThunkForBorland;     //To be tested
#elif defined(_M_IX86)
typedef TThunkForBorland32Bit TThunkForBorland;
typedef TThunkForMicrosoft32Bit TThunkForMicrosoft;
typedef TThunkForBorland32Bit TThunkForGnu;
#else
#error OWLNext: Unable to generate thunks for this platform.
#endif

//
// The following section selects the thunk type for the compiler.
//
#if defined(__BORLANDC__)
typedef TThunkForBorland TThunk;
#elif defined(_MSC_VER)
typedef TThunkForMicrosoft TThunk;
#elif defined(__GNUC__)
typedef TThunkForGnu TThunk;
#else
#error OWLNext: Unable to generate thunks for this compiler.
#endif

//
// Creates a thunk for this TWindow instance. 
//
WNDPROC
TWindow::CreateInstanceProc()
{
  return reinterpret_cast<WNDPROC>(new TThunk(this));
}

//
// Nothing to do.
//
void
TWindow::InitInstanceProc()
{
}

// 
// Releases the thunk for this TWindow instance.
//
void
TWindow::FreeInstanceProc()
{
  delete reinterpret_cast<TThunk*>(GetInstanceProc ());
}

#endif // nothunk/thunk

} // OWL namespace


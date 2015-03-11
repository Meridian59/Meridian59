//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (C) 1993, 1996 by Borland International, All Rights Reserved
// Copyright (C) 1998  by Yura Bidus
//----------------------------------------------------------------------------

#if defined(BI_COMP_BORLANDC) 
# pragma hdrstop
#endif
 
#define CHECKS_CPP

//
// Make sure __TRACE is defined so that we can provide
// run-time support in non-debug versions of the library.
//
#if defined(__TRACE)
#  define __TRACE_PREVIOUSLY_DEFINED
#endif

#undef  __TRACE
#define __TRACE

#if !defined(__TRACE_PREVIOUSLY_DEFINED)
#define _DONT_DECLARE_DEF_DIAG_GROUP // We need this define so checks.h won't try to declare Def diag group.
#endif

#include <owl/defs.h>

#if defined(BI_COMP_BORLANDC) 
# pragma option -w-inl // Disable warning "Functions containing 'statement' is not expanded inline".
#endif

#include <owl/private/checks.h>

#if defined(UNICODE)
# if defined(BI_COMP_GNUC)
#    include <stdlib.h>
# else
#    include <malloc.h> 
# endif
#endif

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <owl/module.h>

using namespace std;
namespace owl {

//
// Definition of the default diagnostic group "Def" (expands to TDiagGroupDef)
//
#if defined(_BUILDOWLDLL)
# if defined(__DEBUG) && defined(__TRACE_PREVIOUSLY_DEFINED)
    OWL_DIAG_DEFINE_EXPORTGROUP(Def, 1, 0); // JJH
# else
    DIAG_DEFINE_EXPORTGROUP(Def, 1, 0);
# endif
#else
# if defined(__DEBUG) && defined(__TRACE_PREVIOUSLY_DEFINED)
    OWL_DIAG_DEFINE_GROUP(Def, 1, 0); // DLN
# else
    DIAG_DEFINE_GROUP(Def, 1, 0);
# endif
#endif

struct TDiagProcessData 
{  
  TDiagBaseHook* GlobalHook;
  TDiagBase* DiagGroupStaticHead;

#if defined(BI_MULTI_THREAD_RTL)

  template <class TData, bool Shared>
  class TLockT
    : public TMRSWSection::TLock
  {
  public:

    TData& Data;

    explicit TLockT(TDiagProcessData& data = TDiagProcessData::GetInstance()) 
    : TMRSWSection::TLock(data.Section, Shared, true), // wait
      Data(data)
    {}
  };

  typedef TLockT<TDiagProcessData, false> TLock;
  typedef TLockT<const TDiagProcessData, true> TConstLock;

  template <class TData, bool Shared>
  friend class TLockT;

private:

  TMRSWSection Section;

#else

  template <class TData>
  class TLockT
  {
  public:

    TData& Data;

    explicit TLockT(TDiagProcessData& data = TDiagProcessData::GetInstance()) 
    : Data(data)
    {}
  };

  typedef TLockT<TDiagProcessData> TLock;
  typedef TLockT<const TDiagProcessData> TConstLock;

  template <class TData>
  friend class TLockT;

#endif

private:

  TDiagProcessData() {}
  TDiagProcessData(const TDiagProcessData&); // forbidden
  TDiagProcessData& operator =(const TDiagProcessData&); // forbidden

  static TDiagProcessData& GetInstance()
  {
    //
    // Note that while this lazy initialization avoids problems with global initialization order,
    // the initial call of this function is not thread-safe (pre C++11). This should not be a 
    // problem here because diagnostics groups are initialized during program start-up, at which
    // time only the main thread should be running. But, as a safe-guard, we ensure this function 
    // is called during program start-up using InitDiagProcessDataInstance below.
    //
    // This safe-guard can be removed when C++11 compliant compilers are mandated.
    //
    // Also note that the singleton construction may throw an exception. Since there is no way to 
    // continue without this singleton, we make no attempt to handle it here. We assume that 
    // the exception will terminate the program, or if it is handled, that subsequent calls will
    // be retried still within a single thread.
    //
    static TDiagProcessData d; // initial call (construction) not thread-safe pre-C++11
    return d;
  }

  static TDiagProcessData& InitDiagProcessDataInstance;

};

//
// Ensure singleton initialization at start-up (single-threaded, safe).
//
TDiagProcessData& TDiagProcessData::InitDiagProcessDataInstance = TDiagProcessData::GetInstance();

void 
TDiagBase::Trace(const tstring& msg, int level, LPCSTR fname, int line)
{
  Message("Trace", msg, level, fname, line);
}

void 
TDiagBase::Warn(const tstring& msg, int level, LPCSTR fname, int line)
{
  Message("Warning", msg, level, fname, line);
}

TDiagBaseHook*  
TDiagBase::SetGlobalHook(TDiagBaseHook* hook)
{
  TDiagProcessData::TLock lock;
  TDiagBaseHook* oldHook = lock.Data.GlobalHook;
  lock.Data.GlobalHook = hook;
  return oldHook;
}

TDiagBase::TDiagBase(LPCSTR name, bool enable, int level)
:
  Name(name), 
  Enabled(enable),
  Level(level),
  LocalHook(0)
{
  AddDiagGroup(this);
}

TDiagBase::~TDiagBase()
{
  RemoveDiagGroup(this);
}

void 
TDiagBase::AddDiagGroup(TDiagBase* group)
{
  TDiagProcessData::TLock lock;
  if (lock.Data.DiagGroupStaticHead == 0)
    lock.Data.DiagGroupStaticHead = group;
  else
  {
    TDiagBase* last = lock.Data.DiagGroupStaticHead;
    while (last->NextGroup)
      last = last->NextGroup;
    last->NextGroup = group;
  }
}

void 
TDiagBase::RemoveDiagGroup(TDiagBase* group)
{
  TDiagProcessData::TLock lock;
  if (lock.Data.DiagGroupStaticHead == group)
    lock.Data.DiagGroupStaticHead = group->NextGroup;
  else
  {
    TDiagBase* last = lock.Data.DiagGroupStaticHead;
    while (last->NextGroup != group)
      last = last->NextGroup;
    last->NextGroup = group->NextGroup;
  }
}

//
// NOTE: To access the linked list of diagnostics groups in a thread-safe manner, the caller of
// this function should ideally lock the structure for the duration of the access, since in theory
// other threads may otherwise mutate the linked list. In the current design, this is not possible
// since access to TDiagBase::NextGroup is not synchronized. In practice, the mutation of the 
// linked list only happens at startup and shutdown, so this problem should not be an issue.
//
TDiagBase* 
TDiagBase::GetDiagGroup(TDiagBase* curr)
{
  if(curr)
    return curr->NextGroup;

  TDiagProcessData::TConstLock lock;
  return lock.Data.DiagGroupStaticHead;
}

//
// Sends the specified message to the debug output device. 
// If a global or local hook is registered, then it is used to output the string.
// Otherwise, the system debug output function is used.
//
void 
TDiagBase::Output(const tstring& msg)
{
  {
    TDiagProcessData::TConstLock lock;
    if (lock.Data.GlobalHook)
    {
      lock.Data.GlobalHook->Output(this, msg.c_str());
      return;
    }
  }
  if (LocalHook)
    LocalHook->Output(this, msg.c_str());
  else
  {
    OWL_OUTPUT_DEBUG_STRING(msg.c_str());
  }
}

void 
TDiagBase::Message(LPCSTR type, const tstring& msg, int level, LPCSTR file, int line)
{
  _USES_CONVERSION;
  tostringstream out;
#if defined(BI_COMP_MSC) 
  out << _A2W(file) << _T("(") << line << _T(") : ") << _A2W(type)
#else 
  out << _A2W(type) << _T(" ") << _A2W(file) << _T(" ") << line
#endif
    << _T(": [") << Name << _T(':') << level << _T("] ") 
    << msg 
    << _T("\r\n");

  Output(out.str());
}

static LONG TraceIndent = 0;

TDiagFunction::TDiagFunction(TDiagBase& group, int level, LPCTSTR function, LPCSTR file, int line)
: 
  Group(group), Level(level), Function(function), File(file), Line(line),
  Enabled(group.IsEnabled() && level <= group.GetLevel())
{
  if (Enabled)
  {
    Trace(_T(">> "), 0);
    InterlockedIncrement(&TraceIndent);
  }
}

TDiagFunction::TDiagFunction(TDiagBase& group, int level, LPCTSTR function, LPCTSTR params, LPCSTR file, int line)
: 
  Group(group), Level(level), Function(function), File(file), Line(line),
  Enabled(group.IsEnabled() && level <= group.GetLevel())
{
  if (Enabled)
  {
    Trace(_T(">> "), params);
    InterlockedIncrement(&TraceIndent);
  }
}

TDiagFunction::~TDiagFunction()
{
  if (Enabled)
  {
    InterlockedDecrement(&TraceIndent);
    Trace(_T("<< "), 0);
  }
}

void 
TDiagFunction::Trace(LPCTSTR prefix, LPCTSTR params)
{
  tostringstream out;
  for (int indent = TraceIndent; indent--;)
    out << _T(" ");
  out << prefix << Function;
  if (params)
    out << _T(" (") << params << _T(")");
  Group.Trace(out.str().c_str(), Level, File, Line);
}

namespace
{
  tstring
  MakeString(LPCSTR type, const tstring& msg, LPCSTR file, int line)
  {
    _USES_CONVERSION; 
    tostringstream s;
    s << _A2W(type) << _T(" failed in \"") << _A2W(file) << _T("\" at line ") << line << _T(": ") << msg;
    return s.str();
  }
}

TDiagException::TDiagException(LPCSTR type, const tstring& msg, LPCSTR file, int line) 
: 
  std::exception(), 
  delegate(MakeString(type, msg, file, line))
{}

const char* 
TDiagException::what() const throw() 
{
  return delegate.what();
}

tstring 
TDiagException::why() const 
{
  return delegate.why();
}

tstring 
TDiagException::ToString(LPCSTR s)
{
  _USES_CONVERSION;
  return tstring(_A2W(s));
}

int
TDiagException::BreakMessage(LPCSTR type, const tstring& msg, LPCSTR file, int line)
{
  _USES_CONVERSION;

  static LONG entranceCount = 0;
  struct TReentranceGuard
  {
    TReentranceGuard() {InterlockedIncrement(&entranceCount);}
    ~TReentranceGuard() {InterlockedDecrement(&entranceCount);}
  } 
  guard;

  tstring error = MakeString(type, msg, file, line);
  if (entranceCount > 1)
  {
    // Reentrance; send error message to debugger or auxiliary port instead.
    // Then assert within assert (examine call stack to determine first one).
    //
    OutputDebugString(error.c_str());
    OWL_BREAK
    return 3;
  }

  // Get the active popup window for the current thread.
  //
  HWND hParent = ::GetActiveWindow();
  if (hParent)
  {
    HWND hPopup = ::GetLastActivePopup(hParent);
    if (hPopup)
      hParent = hPopup;
  }

  // Temporarily remove any WM_QUIT message in the queue, 
  // otherwise the message box will not display.
  //
  struct TQuitMsgHider
  {
    MSG QuitMsg;
    bool DidRemoveQuitMsg;
    TQuitMsgHider() : DidRemoveQuitMsg(PeekMessage(&QuitMsg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {}
    ~TQuitMsgHider() {if (DidRemoveQuitMsg) PostQuitMessage(static_cast<int>(QuitMsg.wParam));}
  } 
  qmh;

  error += _T("\n\nSelect Abort to terminate, Retry to debug, or Ignore to continue.");
  int ret = ::MessageBox(hParent, error.c_str(), _A2W(type), 
    MB_ABORTRETRYIGNORE | MB_DEFBUTTON3 | MB_TASKMODAL | MB_ICONHAND | MB_SETFOREGROUND);
  return ret;
}

TPreconditionFailure::TPreconditionFailure(const tstring& msg, LPCSTR file, int line) 
: TDiagException("Precondition", msg, file, line)
{}

TCheckFailure::TCheckFailure(const tstring& msg, LPCSTR file, int line) 
: TDiagException("Check", msg, file, line)
{}

} // OWL namespace

//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (C) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Diagnostic macros for assertions and tracing
/// \note
/// The macros are controlled by the following switches:
///
///   __DEBUG = 0   PRECONDITION and CHECK are nops
///   __DEBUG = 1   PRECONDITION is active
///   __DEBUG = 2   PRECONDITION and CHECK are active
///   __TRACE       When defined enables TRACE
///   __WARN        When defined enables WARN
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_CHECKS_H)
#define OWL_PRIVATE_CHECKS_H
#define SERVICES_CHECKS_H  //this file replaces old <services/checks.h>

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if defined(__CHECKS_H)
# error <checks.h> included before OWL one.
#endif

#define __CHECKS_H

#include <owl/exbase.h>
#include <owl/private/except.h>
#include <owl/private/strmdefs.h>

namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TDiagBase;
class _OWLCLASS TModule;
class _OWLCLASS TDiagFunction;

typedef tostringstream diag_stream; // deprecated - not in use
typedef tstring diag_string; // deprecated - not in use

//
// Static Global Redirector
//
struct TDiagBaseHook 
{
  virtual void Output(TDiagBase* group, LPCTSTR str) = 0;
};

//
// TDiagBase - this class forms the base for TDiagGroup classes and handles basic message output.
//
class _OWLCLASS TDiagBase
{
  public:

    virtual ~TDiagBase();

    void Trace(const tstring& msg, int level, LPCSTR file, int line);
    void Warn(const tstring& msg, int level, LPCSTR file, int line);
    LPCSTR GetName() const {return Name;}
    void Enable(bool enable = true) {Enabled = enable;}
    bool IsEnabled() const {return Enabled;}
    void SetLevel(int level) {Level = level;}
    int GetLevel() const {return Level;}
    void SetLocalHook(TDiagBaseHook* hook) {LocalHook = hook;}

    virtual TModule* GetDiagModule() = 0;
    static TDiagBase*  GetDiagGroup(TDiagBase* curr = 0);
    static TDiagBaseHook*  SetGlobalHook(TDiagBaseHook* hook = 0);

  protected:
  
    //
    // Name have to be static string (i.e. we store the pointer, not a string copy).
    //
     TDiagBase(LPCSTR name, bool enable, int level);

  protected:

    LPCSTR Name;
    bool Enabled;
    int Level;
    TDiagBaseHook* LocalHook; // second redirector  

  private:

    TDiagBase* NextGroup;

    void Message(LPCSTR type, const tstring& msg, int level, LPCSTR file, int line);
    void Output(const tstring& msg);
    
    static void AddDiagGroup(TDiagBase* group);
    static void RemoveDiagGroup(TDiagBase* group);

    //
    // Prohibit copying
    //
    TDiagBase(const TDiagBase& base);
    TDiagBase& operator=(const TDiagBase& base);
    
    friend class TDiagFunction;
};

//
// TraceFunction - idea from Don Griffin
//
class _OWLCLASS TDiagFunction
{
  public:

    TDiagFunction(TDiagBase& group, int level, LPCTSTR function, LPCSTR file, int line);
    TDiagFunction(TDiagBase& group, int level, LPCTSTR function, LPCTSTR params, LPCSTR file, int line);
    ~TDiagFunction();

  private:

    TDiagBase& Group;
    int Level;
    LPCTSTR Function;
    LPCSTR File;
    int Line;
    bool Enabled;

    void Trace(LPCTSTR prefix, LPCTSTR param);

    //
    // Prohibit copying
    //
    TDiagFunction(const TDiagFunction& f); // prohibite
    TDiagFunction& operator=(const TDiagFunction& f); // prohibite
};

//
// Base class for assertion failures
// We need UTF8 conversion functionality here, same as in TXBase. 
// But we don't want to be a derivative of TXBase, since we want diagnostics 
// exceptions to be separate from the other framework exceptions.
// While we could factor out the shared functionality in TXBase to a new
// base, instead we use TXBase as a delegate here.
//
class _OWLCLASS TDiagException 
  : public std::exception 
{
  public:

    TDiagException(LPCSTR type, const tstring& msg, LPCSTR file, int line);

    static tstring ToString(LPCSTR s);
    static int BreakMessage(LPCSTR type, const tstring& msg, LPCSTR file, int line);

    virtual const char* what() const throw();
    virtual tstring why() const;

  protected:

    TXBase delegate; // Holds UTF8 string and implements 'what' and 'why' for us.

};

//
// Signals a failure of a PRECONDITION macro.
//
class _OWLCLASS TPreconditionFailure
  : public TDiagException 
{
  public:

    TPreconditionFailure(const tstring& msg, LPCSTR file, int line);
};

//
// Signals a failure of a CHECK macro.
//
class _OWLCLASS TCheckFailure
  : public TDiagException 
{
  public:

    TCheckFailure(const tstring& msg, LPCSTR file, int line);
};

#include <owl/posclass.h>

//
/// Retrieves a diagnostic group's enabled flag from a private ini-file. 
/// The master enable switch 'Enabled' overrides individual group settings. 
/// The ini-file is searched for in standard profile places.
//
_OWLFUNC(bool) GetDiagEnabled(LPCSTR filename, LPCSTR diagGroup, bool defEnabled);

//
/// Retrieves a diagnostic group's level setting from a private ini-file.
/// The ini-file is searched for in standard profile places.
//
_OWLFUNC(int) GetDiagLevel(LPCSTR filename, LPCSTR diagGroup, int defLevel);

} // OWL namespace

//
// Macro definitions section
//

#if !defined(__DEBUG)
# define __DEBUG 0
#endif

#if __DEBUG < 1
# define IFDBG(a)
# define IFNDBG(a) a
# ifndef NDEBUG
#   define NDEBUG 
# endif
# define VALIDPTR(p) ((p) != 0) // This is faster than IsBadReadPtr.
#else
# define IFDBG(a) a
# define IFNDBG(a)
# define VALIDPTR(p) (!::IsBadReadPtr((p),1))
#endif

#define VALIDORNULLPTR(p) (!(p) || VALIDPTR(p))

//
// This guy generates a compiler error, such as 'Array must have at least one element',
// if the boolean condition argument is false.
//
#define COMPILE_ASSERT(B) typedef char _Assert##__LINE__ [2*(!!(B)) - 1]

//
// Definitions of precondition macros
//
#undef PRECONDITION
#undef PRECONDITIONX
#define PRECONDITION(condition) PRECONDITIONX(condition, ::owl::TDiagException::ToString(#condition))
#if __DEBUG >= 1
# define PRECONDITIONX(condition, message) DIAG_ASSERT_IMPL(Precondition, condition, message)
#else
# define PRECONDITIONX(condition, message) ((void)0)
#endif

//
// Definitions of check macros
//
#undef CHECK
#undef CHECKX
#define CHECK(condition) CHECKX(condition, ::owl::TDiagException::ToString(#condition))
#if __DEBUG >= 2
# define CHECKX(condition, message) DIAG_ASSERT_IMPL(Check, condition, message)
# define VERIFY(condition) CHECK(condition)
# define VERIFYX(condition, message) CHECKX(condition, message)
#else
# define CHECKX(condition, message) ((void)0)
# define VERIFY(condition) ((void)(condition))
# define VERIFYX(condition, message) ((void)(condition))
#endif

//
// Definition of trace macros
//
#undef TRACE
#undef TRACEX
#define TRACE(message) TRACEX(Def, 0, message)
#define TRACE_FUNCTION(function) TRACE_FUNCTIONX(Def, 0, function)
#define TRACE_FUNCTIONP(function, condition) TRACE_FUNCTIONXP(Def, 0, function, condition)
#if defined(__TRACE)
# define TRACEX(group, level, message) DIAG_TRACE_IMPL(Trace, group, true, level, message)
# define TRACE_FUNCTIONX(group, level, function) ::owl::TDiagFunction __traceFunc((::owl::TDiagBase&)__OwlDiagGroup##group, level, function, __FILE__, __LINE__)
# define TRACE_FUNCTIONXP(group, level, function, condition) ::owl::TDiagFunction __traceFunc((::owl::TDiagBase&)__OwlDiagGroup##group, level, function, condition, __FILE__, __LINE__)
#else
# define TRACEX(group, level, message) ((void)0)
# define TRACE_FUNCTIONX(group, level, function) ((void)0)
# define TRACE_FUNCTIONXP(group, level, function, condition) ((void)0)
#endif

//
// Definition of warning macros
//
#undef WARN
#undef WARNX
#define WARN(condition, message) WARNX(Def, condition, 0, message)
#if defined(__WARN)
# define WARNX(group, condition, level, message) DIAG_TRACE_IMPL(Warn, group, condition, level, message)
#else
# define WARNX(group, condition, level, message) ((void)0)
#endif

//
// Alternative names
//
#define TRACEX_IF(condition, group, level, message) WARNX(group, condition, level, message)
#define TRACE_IF(condition, message) WARNX(Def, condition, 0, message)

//
// Implmentation macro for PRECONDITION and CHECK
//
#define DIAG_ASSERT_IMPL(assertion, condition, message)\
  if (!(condition)) {\
    int retval = ::owl::TDiagException::BreakMessage(#assertion, message, __FILE__, __LINE__);\
    if (retval == IDRETRY)\
      {OWL_BREAK}\
    else if (retval == IDABORT)\
      {throw ::owl::T##assertion##Failure(message, __FILE__, __LINE__);}\
  }

//
// Implmentation macro for WARN and TRACE
//
#define DIAG_TRACE_IMPL(operation, group, condition, level, message)\
  do {\
    using namespace ::owl;\
    if (__OwlDiagGroup##group.IsEnabled() && level <= __OwlDiagGroup##group.GetLevel() && (condition)) {\
      tostringstream out; out << message;\
      __OwlDiagGroup##group.operation(out.str(), level, __FILE__, __LINE__);\
    }\
  } while (false)

// 
// Definitions of diagnostics group macros
//
#if defined(__TRACE) || defined(__WARN)

#if defined(_OWLDLL) || defined(_BUILDOWLDLL)
# if !defined(DIAG_IMPORT)
#   define DIAG_IMPORT __declspec(dllimport)
# endif
# if !defined(DIAG_EXPORT)
#   define DIAG_EXPORT __declspec(dllexport)
# endif
#else
# define DIAG_IMPORT /**/
# define DIAG_EXPORT /**/
#endif

#define DECLARE_DIAG_GROUP_COMMON(group)\
  public:\
    TDiagGroup##group(LPCSTR name, bool enable, int level);\
    virtual ::owl::TModule* GetDiagModule();\
  }

#define DECLARE_DIAG_GROUP(group, qual)\
  class qual TDiagGroup##group : public ::owl::TDiagBase {\
  DECLARE_DIAG_GROUP_COMMON(group)

#define DECLARE_DIAG_STATIC_GROUP(group)\
  class TDiagGroup##group : public ::owl::TDiagBase {\
  DECLARE_DIAG_GROUP_COMMON(group)

#define DIAG_DEFINE_GROUP_COMMON(group)\
  TDiagGroup##group::TDiagGroup##group(LPCSTR name,bool enable, int level) : ::owl::TDiagBase(name, enable, level) {}\
  ::owl::TModule* TDiagGroup##group::GetDiagModule() {return &::owl::GetGlobalModule();}

#define _DIAG_DEFINE_GROUP_(group, enable, level, t)\
  DECLARE_DIAG_GROUP(group, t);\
  DIAG_DEFINE_GROUP_COMMON(group)

#define _DIAG_DEFINE_GROUP_STATIC(group, enable, level)\
  DECLARE_DIAG_STATIC_GROUP(group);\
  DIAG_DEFINE_GROUP_COMMON(group);\
  TDiagGroup##group __OwlDiagGroup##group(#group, enable, level)

//
// DLN: Define OwlDiagGroupDef without redefining TDiagGroupDef.
//
#define OWL_DIAG_DEFINE_GROUP(group, enable, level) \
  DIAG_DEFINE_GROUP_COMMON(group); \
  TDiagGroup##group __OwlDiagGroup##group(#group, enable, level)

#define DIAG_DECLARE_GROUP(group)\
  DECLARE_DIAG_STATIC_GROUP(group);\
  extern TDiagGroup##group __OwlDiagGroup##group

#define DIAG_DEFINE_GROUP(group, enable, level)\
  _DIAG_DEFINE_GROUP_STATIC(group, enable, level);

#define DIAG_DECLARE_EXPORTGROUP(group)\
  DECLARE_DIAG_GROUP(group, DIAG_EXPORT);\
  extern _OWLEXPORTDATA(TDiagGroup##group) __OwlDiagGroup##group

#define DIAG_DECLARE_IMPORTTGROUP(group)\
  DECLARE_DIAG_GROUP(group, DIAG_IMPORT);\
  extern _OWLIMPORTDATA(TDiagGroup##group) __OwlDiagGroup##group

//
// JJH: Define group for winelib export macro without redefining TDiagGroupDef
//
#define OWL_DIAG_DEFINE_EXPORTGROUP(group, enable, level)\
  DIAG_DEFINE_GROUP_COMMON(group)\
  _OWLEXPORTDATA(TDiagGroup##group) __OwlDiagGroup##group(#group, enable, level)

#define DIAG_DEFINE_EXPORTGROUP(group, enable, level)\
  _DIAG_DEFINE_GROUP_(group, enable, level, DIAG_EXPORT);\
  _OWLEXPORTDATA(TDiagGroup##group) __OwlDiagGroup##group(#group, enable, level)

//
// Definitions of group accessors and mutators macros
//
#define DIAG_ENABLE(group, s) __OwlDiagGroup##group.Enable(s)
#define DIAG_ISENABLED(group) __OwlDiagGroup##group.IsEnabled()
#define DIAG_SETLEVEL(group, level) __OwlDiagGroup##group.SetLevel(level)
#define DIAG_GETLEVEL(group) __OwlDiagGroup##group.GetLevel()
#define DIAG_REDIRECT(group, s) __OwlDiagGroup##group.SetLocalHook(s)

//
// Definitions of utility macros
//
#define IFDIAG(a) a
#define IFNDIAG(a)

#else // #if defined(__TRACE) | defined(__WARN)

#define DIAG_DECLARE_GROUP(group)
#define DIAG_DECLARE_EXPORTGROUP(group)
#define DIAG_DECLARE_IMPORTTGROUP(group)

#define DIAG_DEFINE_GROUP(group,enable,level)
#define OWL_DIAG_DEFINE_GROUP(group,enable,level)
#define DIAG_DEFINE_EXPORTGROUP(group,enable,level)

#define DIAG_ENABLE(group,s) ((void)0)
#define DIAG_ISENABLED(group) ((void)0)
#define DIAG_SETLEVEL(group,level) ((void)0)
#define DIAG_GETLEVEL(group) ((void)0)
#define DIAG_REDIRECT(group,s) ((void)0)

#define IFDIAG(a)
#define IFNDIAG(a) a

#endif // #if defined(__TRACE) | defined(__WARN)

//
// Define without redefining TDiagGroupDef.
//
#define OWL_DIAG_DEFINE_GROUP_INIT(f,g,e,l)\
  OWL_DIAG_DEFINE_GROUP(g, ::owl::GetDiagEnabled(f,#g,e), ::owl::GetDiagLevel(f,#g,l));

#define DIAG_DEFINE_GROUP_INIT(f,g,e,l)\
  DIAG_DEFINE_GROUP(g, ::owl::GetDiagEnabled(f,#g,e), ::owl::GetDiagLevel(f,#g,l));

#define DIAG_DEFINE_EXPORTGROUP_INIT(f,g,e,l)\
  DIAG_DEFINE_EXPORTGROUP(g, ::owl::GetDiagEnabled(f,#g,e), ::owl::GetDiagLevel(f,#g,l));
// 
// Declaration of the Def default group
//
#if (defined(__TRACE) || defined(__WARN)) && !defined(_DONT_DECLARE_DEF_DIAG_GROUP) && !defined(_DEF_DECLARED)
namespace owl {
# define _DEF_DECLARED
# if defined(_BUILDOWLDLL)
    DIAG_DECLARE_EXPORTGROUP(Def);
# elif defined(_OWLDLL)
    DIAG_DECLARE_IMPORTTGROUP(Def);
# else
    DIAG_DECLARE_GROUP(Def);
# endif
} // OWL namespace
#endif

//
// Diagnostics helpers
//

#if defined(__TRACE) || defined(__WARN)

namespace owl {

//
/// An object of this class, when streamed out, logs the contained type name.
//
template <class T>
struct TTraceType
{
  friend tostream& operator <<(tostream& os, const TTraceType& t)
  {return os << typeid(T).name();}
};

//
/// Returns an object that, when streamed out, logs the pointee static type.
///
/// Usage:
/// \code
/// template <class T> void Foo() {TRACE("T type: " << TraceType<T>());}
/// \endcode
//
template <class T>
static TTraceType<T> TraceType(T* p = 0)
{
  InUse(p);
  return TTraceType<T>();
}

//
/// An object of this class, when streamed out, logs the contained pointee type and pointer value.
//
template <class T>
struct TTraceId
{
  T* p;
  TTraceId(T* p_) : p(p_) {}

  friend tostream& operator <<(tostream& os, const TTraceId& id)
  {return os << TraceType(id.p) << " [" << static_cast<void*>(id.p) << "]";}
};

//
/// Returns an object that, when streamed out, logs the pointee type and pointer value.
///
/// Usage:
/// \code
/// void Foo(Bar* p) {TRACE("Foo received pointer to " << TraceId(p));}
/// \endcode
//
template <class T>
static TTraceId<T> TraceId(T* p)
{return TTraceId<T>(p);}

} // OWL namespace

#endif

#endif  // OWL_PRIVATE_CHECKS_H

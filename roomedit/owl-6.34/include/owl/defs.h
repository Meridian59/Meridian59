//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// General definitions used by all ObjectWindows programs. Included directly
/// by ObjectWindows source modules.
//------------------------------------------------------------------------------

#if !defined(OWL_DEFS_H)
#define OWL_DEFS_H
#define __OWL_OWLDEFS_H   // Old-style define for VbxGen compatibility

//------------------------------------------------------------------------------


#if !defined(OWL_PRIVATE_DEFS_H)
# include <owl/private/defs.h>      // Supporting defines for app classes
#endif

#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

//------------------------------------------------------------------------------
// Get Winsys & Services headers common to all of Owl
//

#if !defined(OWL_PRIVATE_MEMORY_H)
# include <owl/private/memory.h>    // Common memory manipulation functions
#endif

#if !defined(OWL_PRIVATE_EXCEPT_H)
# include <owl/private/except.h>
#endif

#include <owl/private/number.h>

#include <tchar.h>
#include <string>
#include <limits>

namespace owl {

#if defined(_UNICODE)
typedef wchar_t tchar;
typedef wchar_t utchar;
typedef std::wstring tstring;
#else
typedef char tchar;
typedef unsigned char utchar;
typedef std::string tstring;
#endif    
  
const size_t NPOS = static_cast<size_t>(-1);

} // OWL namespace

#if !defined(_ttof)
# if defined(UNICODE)
#  define _ttof       _wtof
# else
#  define _ttof       atof
# endif
#endif

#include <owl/private/checks.h>    // Runtime diagnostic macros
#include <owl/wsysinc.h>     // Windowing system headers
#include <owl/system.h>
#include <owl/version.h>    // OWL version # constants

//------------------------------------------------------------------------------
// Additional windows.h related defines & undefs for Owl compatibility
//
//extern HINSTANCE _hInstance;

# undef GetNextWindow
# undef GetWindowTask
# define WM_SYSTEMERROR      0x0017
# define WM_CTLCOLOR         0x0019
# define HTASK HANDLE    // allow users to use the same type in Win16 & Win32
#undef  SetWindowFont
#undef  GetWindowFont

#if defined(_M_CEE)   // this is defined when /clr flag is used

#if !defined(GetClassName)
#ifdef UNICODE
#define GetClassName  GetClassNameW
#else
#define GetClassName  GetClassNameA
#endif // !UNICODE
#endif

#if !defined(SendMessage)
#ifdef UNICODE
#define SendMessage  SendMessageW
#else
#define SendMessage  SendMessageA
#endif // !UNICODE
#endif

#if !defined(GetTextMetrics)
#ifdef UNICODE
#define GetTextMetrics  GetTextMetricsW
#else
#define GetTextMetrics  GetTextMetricsA
#endif // !UNICODE
#endif

#if !defined(DrawText)
#ifdef UNICODE
#define DrawText  DrawTextW
#else
#define DrawText  DrawTextA
#endif // !UNICODE
#endif

#endif

//------------------------------------------------------------------------------
// Owl settings for runtime diagnostics in owl.defs.h & classlib/checks.h
//
#define   OWL_INI     "OWL.INI"
#define   OWL_CDLEVEL  2
#if !defined(OWL_DIAGINFO)
# define OWL_DIAGINFO
#endif


//------------------------------------------------------------------------------
// Strict data makes all data members private. Accessors must then be used
// to access the data members. Public data makes some data public, some data
// protected, and some private. 
//
// Note that here may be exceptions to these rules where data members are
// are explicitly declared public or protected.
//
// Unfortunately, the OWLNext source does not itself compile with any other
// option than OWL_PUBLIC_DATA. So, for now, keep OWLNext members as they were 
// in OWL 5.
//
#if defined(OWL_STRICT_DATA) || defined(OWL_PROTECTED_DATA)
# error "OWLNext: OWL_STRICT_DATA and OWL_PROTECTED_DATA are not yet supported.
#elif !defined(OWL_PUBLIC_DATA)
# define OWL_PUBLIC_DATA
#endif

#if defined(OWL_STRICT_DATA)

# define public_data    private
# define protected_data private

#elif defined(OWL_PROTECTED_DATA)

# define public_data    protected
# define protected_data protected

#elif defined(OWL_PUBLIC_DATA)

# define public_data    public
# define protected_data protected

#else 
# error "OWLNext: No data member access option is defined."
#endif

//
// Define a boolean flag to indicate if we compile in compatility mode or not.
//
#ifdef OWL5_COMPAT
# define OWL_STRICT 0
#else
# define OWL_STRICT 1
#endif

//
// This is a work-around for non-compliant compilers, unable to return void.
// When all supported compilers support the return of void, then use of this 
// macro can be replaced by TWindow::SetOrReturnHandle as follows:
//
// return SetOrReturnHandle(h);
//
#if OWL_STRICT
#define OWL_SET_OR_RETURN_HANDLE(h) return h
#else
#define OWL_SET_OR_RETURN_HANDLE(h) SetHandle(h)
#endif

//------------------------------------------------------------------------------
// Common external Owl functions & data
//

namespace owl {

/// Get version of OWL at runtime
///
/// Returns the version number of the ObjectWindows library. The version number is
/// represented as an unsigned short.
_OWLFUNC(uint32)  OWLGetVersion();   ///< Get version of OWL at runtime

extern void InitGlobalModule(HINSTANCE hInstance);
class _OWLCLASS TModule;
extern TModule& GetGlobalModule();

} // OWL namespace

//------------------------------------------------------------------------------
// Common utility functions
// TODO: Move to a more suitable home.

namespace owl {

//
// Integer byte, word and long word manipulation
//
inline uint16 MkUint16(uint8 lo, uint8 hi) {
  return uint16(lo | (uint16(hi) << 8));
}
inline uint32 MkUint32(uint16 lo, uint16 hi) {
  return lo | (uint32(hi) << 16);
}
inline uint16 LoUint16(LRESULT r) {
  return LOWORD(r);
}
inline int16 LoInt16(LRESULT r) {
  return static_cast<int16>(LOWORD(r));
}
inline uint16 HiUint16(LRESULT r) {
  return HIWORD(r);
}
inline int16 HiInt16(LRESULT r) {
  return static_cast<int16>(HIWORD(r));
}
inline uint8 LoUint8(LRESULT r) {
  return LOBYTE(r);
}
inline int8 LoInt8(LRESULT r) {
  return static_cast<int8>(LOBYTE(r));
}
inline uint8 HiUint8(LRESULT r) {
  return HIBYTE(r);
}
inline int8 HiInt8(LRESULT r) {
  return static_cast<int8>(HIBYTE(r));
}
inline uint16 SwapUint16(uint16 u16){
  return uint16((u16 >> 8) | (u16 << 8));
}
inline uint32 SwapUint32(uint32 u32){
  return MkUint32(SwapUint16(HiUint16(u32)), SwapUint16(LoUint16(u32)));
}

//
/// Handy utility to avoid compiler warnings about unused parameters
//
template <class T>
void InUse(const T& arg) 
{static_cast<void>(arg);}

//
/// Copies text from a C-string (null-terminated character array) into a string object,
/// using a function or functor (function object) to do the actual copying.
/// To be used by string-aware functions for copying text from C APIs to string objects.
/// Usually a functor will be provided that wraps a C API function that provides the data.
///
/// \code
/// int 
/// TGetText::operator()(LPTSTR buf, int buf_size)
/// \endcode
///
/// should copy max (buf_size - 1) characters of text into buf and add a null-terminator.
/// It should return the number of characters copied, excluding the null-terminator.
//
template<class TGetText>
tstring CopyText(int size, TGetText get_text)
{
  // This method makes no assumptions about the string implementation, other than what is
  // required by C++98; in particular that &s[0] returns a pointer to contiguous data.
  // Since there is no  guarantee that the data is null-terminated, we must add room for the 
  // null-terminator and remove it afterwards.

  tstring s;
  s.resize(size + 1); // Add space for explicit null-terminator.
  int n = get_text(&s[0], size + 1);
  s.erase(n); // Trim the excess space, including the null-terminator.
  return s;
}

namespace detail
{
  template <class T, class U, bool is_integer>
  struct TIsRepresentable;

  template <class T, class U>
  struct TIsRepresentable<T, U, true>
  {

#if defined(BI_COMP_BORLANDC)
# pragma warn -ccc // Disable warning "Condition is always true/false".
#endif

    static bool Call(U v)
    {
    typedef std::numeric_limits<T> Range;
    return v >= 0 ? (static_cast<ULONG_PTR>(v) <= static_cast<ULONG_PTR>(Range::max())) :
      static_cast<LONG_PTR>(v) >= static_cast<LONG_PTR>(Range::min());
    }

#if defined(BI_COMP_BORLANDC)
# pragma warn .ccc // Retore warning "Condition is always true/false".
#endif

  };
}

//
/// Helper function for testing for integer truncations.
//
template <class T, class U>
bool IsRepresentable(U v)
{
  // Restrict implementation to integer arguments.
  typedef detail::TIsRepresentable<T, U, std::numeric_limits<T>::is_integer> TImpl;
  return TImpl::Call(v);
}

} // OWL namespace

//
// Provide alias for old all-uppercase namespace name.
//
#if defined(OWL5_COMPAT)
#define OWL owl
#endif

#endif  // OWL_DEFS_H
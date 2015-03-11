//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Reliable platform independent header for common memory and string functions
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_MEMORY_H)
#define OWL_PRIVATE_MEMORY_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <tchar.h>

#if defined(BI_COMP_BORLANDC)
# include <mem.h>
#else
# include <memory.h>
#endif
#if !defined(__STRING_H) && !defined(_INC_STRING)
# include <string.h>
#endif
#if !defined(BI_COMP_GNUC)
#  include <stdlib.h>
#endif
#include <malloc.h>

#if !defined(BI_COMP_BORLANDC) && !defined(BI_COMP_WATCOM)  && !defined(BI_COMP_FORTE) && !defined(BI_COMP_ACC) && !(defined(__GNUC__) && (__GNUC__ >= 3))
#  define alloca _alloca
#endif

#if !defined(OWL_PRIVATE_WSYSINC_H)
# include <owl/private/wsysinc.h>
#endif


#if defined(BI_NEED_TCHAR)
#  if defined(BI_COMP_BORLANDC)
inline int  _tcscmp(const _TCHAR* s1, const _TCHAR* s2)
                   {return lstrcmp(s1, s2);}
inline int  _tcsicmp(const _TCHAR* s1, const _TCHAR* s2)
                   {return lstrcmpi(s1, s2);}
#  endif
#  if !defined(strcmpi)
inline int strcmpi(const _TCHAR* s1, const _TCHAR* s2)
                   {return lstrcmpi(s1, s2);}
# endif //strcmpi
#endif

/// Return the number of bytes of the first character of the passed string
inline UINT CharSize(const _TCHAR* s) {return static_cast<UINT>(sizeof(_TCHAR) * (::CharNext(s) - s));}

inline _TCHAR CharUpper(_TCHAR c) {::CharUpperBuff(&c,1); return c;}
inline _TCHAR CharLower(_TCHAR c) {::CharLowerBuff(&c,1); return c;}

#  if defined(_CONVERSION_USES_THREAD_LOCALE)
#    define BI_CONVERSION_USES_THREAD_LOCALE
#  endif

#  if !defined(USES_CONVERSION)
#    ifdef BI_CONVERSION_USES_THREAD_LOCALE
#      ifndef _DEBUG
#        define USES_CONVERSION int _convert = 0; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
#      else
#        define USES_CONVERSION int _convert = 0; _convert; UINT _acp = GetACP(); _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
#      endif
#    else
#      ifndef _DEBUG
#        define USES_CONVERSION int _convert = 0; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
#      else
#        define USES_CONVERSION int _convert = 0; _convert; UINT _acp = CP_ACP; _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
#      endif
#    endif
#  endif

// Global UNICODE<>ANSI translation helpers
inline LPWSTR OwlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, unsigned int acp)
{
  // verify that no illegal character present
  // since lpw was allocated based on the size of lpa
  // don't worry about the number of chars
  lpw[0] = '\0';
  MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars);
  return lpw;
}

inline LPSTR OwlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars, unsigned int acp)
{
  // verify that no illegal character present
  // since lpa was allocated based on the size of lpw
  // don't worry about the number of chars
  lpa[0] = '\0';
  WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL);
  return lpa;
}
inline LPWSTR OwlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
  return OwlA2WHelper(lpw, lpa, nChars, CP_ACP);
}

inline LPSTR OwlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
  return OwlW2AHelper(lpa, lpw, nChars, CP_ACP);
}

#  ifdef BI_CONVERSION_USES_THREAD_LOCALE
#    ifdef OWLA2WHELPER
#      undef OWLA2WHELPER
#      undef OWLW2AHELPER
#    endif
#    define OWLA2WHELPER OwlA2WHelper
#    define OWLW2AHELPER OwlW2AHelper
#  else
#    ifndef OWLA2WHELPER
#      define OWLA2WHELPER OwlA2WHelper
#      define OWLW2AHELPER OwlW2AHelper
#    endif
#  endif


#  if defined(BI_CONVERSION_USES_THREAD_LOCALE)
#    if !defined(A2W)
#      define A2W(lpa) (\
        ((_lpa = lpa) == NULL) ? NULL : (\
          _convert = (lstrlenA(_lpa)+1),\
          OWLA2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert, _acp)))
#    endif
#    if !defined(A2WB)
#      define A2WB(lpa,buf) (\
        ((_lpa = lpa) == NULL) ? NULL : (\
          _convert = (lstrlenA(_lpa)+1),\
          OWLA2WHELPER((LPWSTR) buf, _lpa, _convert, _acp)))
#    endif
#  else
#    if !defined(A2W)
#      define A2W(lpa) (\
        ((_lpa = lpa) == NULL) ? NULL : (\
          _convert = (lstrlenA(_lpa)+1),\
          OWLA2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert)))
#    endif
#    if !defined(A2WB)
#      define A2WB(lpa,buf) (\
        ((_lpa = lpa) == NULL) ? NULL : (\
          _convert = (lstrlenA(_lpa)+1),\
          OWLA2WHELPER((LPWSTR) buf, _lpa, _convert)))
#    endif
#  endif

#  ifdef BI_CONVERSION_USES_THREAD_LOCALE
#    if !defined(W2A)
#      define W2A(lpw) (\
        ((_lpw = lpw) == NULL) ? NULL : (\
          _convert = (lstrlenW(_lpw)+1)*2,\
          OWLW2AHELPER((LPSTR) alloca(_convert), _lpw, _convert, _acp)))
#    endif
#    if !defined(W2AB)
#    define W2AB(lpw,buf) (\
      ((_lpw = lpw) == NULL) ? NULL : (\
        _convert = (lstrlenW(_lpw)+1)*2,\
        OWLW2AHELPER((LPSTR) buf, _lpw, _convert, _acp)))
#    endif
#  else
#    if !defined(W2A)
#      define W2A(lpw) (\
        ((_lpw = lpw) == NULL) ? NULL : (\
          _convert = (lstrlenW(_lpw)+1)*2,\
          OWLW2AHELPER((LPSTR) alloca(_convert), _lpw, _convert)))
#    endif
#    if !defined(W2AB)
#      define W2AB(lpw,buf) (\
        ((_lpw = lpw) == NULL) ? NULL : (\
          _convert = (lstrlenW(_lpw)+1)*2,\
          OWLW2AHELPER((LPSTR) buf, _lpw, _convert)))
#    endif
#  endif

#  if !defined(A2CW)
#    define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#  endif
#  if !defined(W2CA)
#    define W2CA(lpw) ((LPCSTR)W2A(lpw))
#  endif
#  if !defined(A2CWB)
#    define A2CWB(lpa,buf) ((LPCWSTR)A2WB(lpa,buf))
#  endif
#  if !defined(W2CAB)
#    define W2CAB(lpw,bif) ((LPCSTR)W2AB(lpw,buf))
#  endif


//////////////////////////////
// Usefull macros
// _USES_CONVERSION    -> set variables if UNICODE
// _USES_CONVERSION_A  -> set variables if not UNICODE
// _W2A(lpw)          -> W to A convert if UNICODE
// _A2W(lpw)          -> A to W convert if UNICODE
// _W2AB(lpw,buf)      -> W to A convert if UNICODE using buffer
// _A2WB(lpw,buf)     -> A to W convert if UNICODE using buffer
// _W2A_A(lpw)          -> W to A convert if not UNICODE
// _A2W_A(lpw)          -> A to W convert if not UNICODE
// _W2A_AB(lpw,buf)      -> W to A convert if not UNICODE using buffer
// _A2W_AB(lpw,buf)     -> A to W convert if not UNICODE using buffer
#if defined(UNICODE)
#  define _USES_CONVERSION USES_CONVERSION
#  define _USES_CONVERSION_A 
#  define _W2A(lpw) W2A(lpw)
#  define _A2W(lpw) A2W(lpw)
#  define _W2AB(lpw,buf) W2AB(lpw,buf)
#  define _A2WB(lpw,buf) A2WB(lpw,buf)
#  define _W2A_A(lpw) lpw
#  define _A2W_A(lpw) lpw
#  define _W2A_AB(lpw,buf) lpw
#  define _A2W_AB(lpw,buf) lpw
#else
#  define _USES_CONVERSION 
#  define _USES_CONVERSION_A USES_CONVERSION
#  define _W2A(lpw) lpw
#  define _A2W(lpw) lpw
#  define _W2AB(lpw,buf) lpw
#  define _A2WB(lpw,buf) lpw
#  define _W2A_A(lpw) W2A(lpw)
#  define _A2W_A(lpw) A2W(lpw)
#  define _W2A_AB(lpw,buf) W2AB(lpw,buf)
#  define _A2W_AB(lpw,buf) A2WB(lpw,buf)
#endif

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup utility
/// @{

//
/// \class TCharIterator
// ~~~~~ ~~~~~~~~~~~~~
//
template<class T> class TCharIterator {
  public:
    TCharIterator(T* p);

    T* Next() const;
    T* Current() const;

    T* operator ++();    // prefix
    T* operator ++(int); // postfix

    operator T*() const;
  
  protected:
    T* P;
};

//
/// \class TBidirCharIterator
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
template<class T> class TBidirCharIterator : public TCharIterator<T> {
  public:
    TBidirCharIterator(T* begin, T* p);

    T* Prev() const;

    T* operator --();    // prefix
    T* operator --(int); // postfix

  private:
    T* Begin;
};


//
/// \class TTmpBufferBase
// ~~~~~ ~~~~~~~~~~~~~~
/// Static temporary fixed buffer, provides fast temporary buffer, for use in functions
// 
/// For example:
/// \code
///  TTmpBuffer<_TCHAR> buffer(MAX_PATH);
///  GetWindowDirectory(buffer,MAX_PATH);
/// \endcode
//
class _OWLCLASS TTmpBufferBase {
  protected:
    TTmpBufferBase(int size);
    ~TTmpBufferBase();

  protected:
    void* Buffer;
    int    Index;

  private:
    TTmpBufferBase(const TTmpBufferBase& buff);
    TTmpBufferBase& operator=(const TTmpBufferBase& buff);
#if defined(__GNUC__) //JJH
    void* operator new(size_t) throw();      // prohibit use of new
#else //vc7
    void* operator new(size_t);      // prohibit use of new
#endif
};
//
template<class T> class TTmpBuffer: public TTmpBufferBase {
  public:
    TTmpBuffer(int size) : TTmpBufferBase(size*sizeof(T)){}

    T&   operator  *()              {return *(T*)Buffer;}
         operator T*()              {return (T*)Buffer;}
    const bool operator  !() const  {return Buffer==0;}
    T& operator[](int i)            {return ((T*)Buffer)[i];}  
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
template<class T> inline TCharIterator<T>::TCharIterator(T* p)
: P(p)
{
}


//
template<class T> inline T* TCharIterator<T>::Next() const
{
#if defined(BI_DBCS_SUPPORT)
  return *P ? ::AnsiNext(P) : P + 1;
#else
  return P + 1;
#endif
}


//
template<class T> inline T* TCharIterator<T>::Current() const
{
  return P;
}

//
template<class T> inline T* TCharIterator<T>::operator ++()
{
  return P = Next();
}

//
template<class T> inline T* TCharIterator<T>::operator ++(int)
{
  T* p = P;
  P = Next();
  return p;
}

//
template<class T> inline TCharIterator<T>::operator T*() const
{
  return P;
}
//
template<class T> inline TBidirCharIterator<T>::TBidirCharIterator(T* begin, T* p)
:
  TCharIterator<T>(p), Begin(begin)
{
}

//
template<class T> inline T* TBidirCharIterator<T>::Prev() const
{
#if defined(BI_DBCS_SUPPORT)
  return ::AnsiPrev(Begin, this->P);
#else
  return this->P > Begin ? this->P - 1 : this->P;
#endif
}

//
template<class T> inline T* TBidirCharIterator<T>::operator --()    // prefix
{
  return this->P = Prev();
}

//
template<class T> inline T* TBidirCharIterator<T>::operator --(int) // postfix
{
  T* p = this->P;
  this->P = Prev();
  return p;
}

} // OWL namespace


//
// Type overloaded version of Window's huge mem copy (hmemcpy) for flat memory
// models.
//
//  inline void      hmemcpy(void* d, const void* s, long n) {memcpy(d, s, n);}

//
// A C++ version of strdup(), strnewdup(). Uses new char[], to allow duplicated
// strings to be deleted[].
// Also a string version of atol for near data models
//
_OWLFUNC(char*) strnewdup(const char* s, size_t minAllocSize = 0);
# define nstrnewdup strnewdup

# if !defined(_WCHAR_T_DEFINED)
    typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
# endif

_OWLFUNC(wchar_t*) strnewdup(const wchar_t* s, size_t minAllocSize = 0);

#endif  // OWL_PRIVATE_MEMORY_H

//----------------------------------------------------------------------------
// ObjectWindow - OWL NExt
// Copyright 1999. Yura Bidus.
// All Rights reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//  OVERVIEW
//  ~~~~~~~~
//  Syntax coloring text editor, grid.
//----------------------------------------------------------------------------
#if !defined(COOLPRJ_DEFS_H)
#define COOLPRJ_DEFS_H

#include <owl/wsyscls.h>
#include <owl/color.h>
#include <owl/contain.h>

//-------------------------------------------------------------------
// Handle the COOLPRJCLASS macro magic
#if defined (_BUILDCOOLPRJDLL)
// COOLPRJ being built as a DLL
# define _COOLCLASS _OWLEXPORTCLASS
#  define _COOLFUNC  _OWLEXPORTFUNC
// We must use all libraries in DLLs if we are using COOLPRJ in a DLL
# if !defined (_OWLDLL)
#  define _OWLDLL
# endif

#elif (defined (BUILD_COOLPRJLIB) || defined (USE_COOLPRJLIB))
// COOLPRJ being built or used as a static library
# define _COOLCLASS
# define _COOLFUNC(p) p

#elif defined (USE_COOLPRJDLL)
// COOLPRJ being used as a DLL
# define _COOLCLASS    _OWLIMPORTCLASS
# define _COOLFUNC     _OWLIMPORTFUNC
#else
// User forgot to tell us which version they're using
# error You must define either USE_COOLPRJLIB or USE_COOLPRJDLL.

#endif

#if defined(USE_COOLPRJLIB) && defined(USE_COOLPRJDLL)
# error "Both USE_COOLPRJLIB and USE_COOLPRJDLL are defined."
#endif

/*
#if !defined(_COOLCLASS)
#  if !defined(_COOLDLL)
#    define _COOLCLASS
#    define _COOLFUNC(r) r
#  else
#    define _COOLCLASS    _OWLIMPORTCLASS
#    define _COOLFUNC    _OWLIMPORTFUNC
#  endif
#endif
*/
// include libraries
#if !defined(_BUILDCOOLPRJDLL)
#include <coolprj/coolprjlink.h>
#endif


#if !defined(_COOLEDCLASS)
#define _COOLEDCLASS _COOLCLASS
#endif
#if !defined(_COOLEDFUNC)
#define _COOLEDFUNC  _COOLFUNC
#endif
#if !defined(_COOLGRIDCLASS)
#define _COOLGRIDCLASS _COOLCLASS
#endif
#if !defined(_DRAGDROPCLASS)
#define _DRAGDROPCLASS _COOLCLASS
#endif
#if !defined(_PATTERNFUNC)
#define _PATTERNFUNC _COOLFUNC
#endif

#include <vector>

//TODO: use TPtrArray or move class to standard OWL arrays
template <class T> class TCoolPtrArray: public std::vector<T> {
  public:
    TCoolPtrArray():std::vector<T>() {}
  ~TCoolPtrArray()    { Flush(true); }
  size_t Size() const   { return this->size(); }
  size_t Add(T t)
    {
    this->push_back(t);
    return this->size()-1;
    }
  size_t AddAt(T t, size_t index)
    {
    this->insert(this->begin()+index, t);
    return this->size()-1;
    }
  void Flush(bool erase)
    {
    if(erase){
      for(typename std::vector<T>::iterator itr = this->begin(); itr < this->end(); itr++)
      delete *itr;
    }
    this->clear();
    }
  void Destroy(size_t index)
    {
    delete *(this->begin()+index);
    this->erase(this->begin()+index);
    }
};

//
//
//
#define MEMBER_METHOD_PROLOGUE(theClass, localClass, resultName) \
  theClass* resultName = \
    ((theClass*)((BYTE*)this - offsetof(theClass, localClass))); \
  static_cast<void>(resultName); // avoid warning from compiler

//-----------------------------------------------------------------------------
//
//
//
inline int xisspecial(int c){
  return (unsigned) c > (unsigned) _T ('\x7f') || c == _T ('_');
}
inline int xisalpha (int c){
  return _istalpha (c) || xisspecial (c);
}
inline int xisalnum (int c){
  return _istalnum (c) || xisspecial (c);
}
//-----------------------------------------------------------------------------
_COOLFUNC(void) ExtractStrings(owl::TStringArray& array, const owl::TResId& resId,
                               _TCHAR separator=_T('|'), owl::TModule* module=0);
_COOLFUNC(owl::TColor) GetDefColor16(int index);
_COOLFUNC(int) FindColorIndex16(const owl::TColor& color);


/*============================================================================*/
#endif // COOLPRJ_DEFS_H

//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TString, a flexible universal string envelope class.
/// Facilitates efficient construction and assignment of many string types
//----------------------------------------------------------------------------

#if !defined(OWL_STRING_H)
#define OWL_STRING_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/private/ole2inc.h>
#include <owl/private/memory.h>
#include <owl/module.h>
#include <owl/lclstrng.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TSysStr
// ~~~~~ ~~~~~~~
/// System string (BSTR) encapsulation. Also gives BSTRs a unique type
/// Always assumes ownership of the BSTR, use Relinquish to take BSTR away.
//
class _OWLCLASS TSysStr {
  public:
    TSysStr() : P(0) {}
    TSysStr(const BSTR p) : P(p) {}
    TSysStr(const TSysStr& src) : P(TOleAuto::SysAllocString(src.P)) {}

   ~TSysStr() {if (P) TOleAuto::SysFreeString(P);}

    int    operator !() const {return P == 0;}
    operator const BSTR() const {return P;}
    operator BSTR() {return P;}

    TSysStr& operator =(BSTR p) {if (P) TOleAuto::SysFreeString(P); P = p; return *this;}
    operator BSTR *() {if (P) {TOleAuto::SysFreeString(P); P = 0;} return &P;}

    BSTR   Relinquish() {BSTR p = P; P = 0; return p;}

  protected:
    BSTR P;

  private:
#if defined(BI_COMP_GNUC)
    void* operator new(size_t) {throw bad_alloc();} // prohibit use of new, delete, etc
#else
    void* operator new(size_t) {return 0;} // prohibit use of new, delete, etc
#endif
    void  operator delete(void*) {}
};

class _OWLCLASS TUString;

//
/// \class TString
// ~~~~~ ~~~~~~~
/// Reference to reference counted string object TUString
/// Lightweight reference object consisting of a pointer to actual object
/// Facilitates copying and assignment with minimal string reallocations
//
class _OWLCLASS TString {
  public:
    /// Construct a TString from any type of string
    // !CQ the assumption that a const char* is non-volatile is bogus!
    //
    TString(const char * s = 0);
    TString(const wchar_t* s);
    TString(BSTR s, bool loan);
    TString(TSysStr& s, bool loan);
    TString(const tstring& s);
    TString(TUString* s);
    TString(const TString& src);

   ~TString();

    // Information
    //
    int  Length() const;        ///< The length in characters of this string
    bool IsNull() const;        ///< Is the string NULL?
    bool IsWide() const;        ///< Are the string contents any kind of wide?

    // Assign any type of string into this TString
    //
    TString& operator =(const TString& s);
    TString& operator =(const tstring& s);
    TString& operator =(const char * s);
    TString& operator =(char* s);
    TString& operator =(const wchar_t* s);
    TString& operator =(wchar_t* s);

    // Convert this TString into the desired string type & return pointer into
    // this TString
    //
    operator const char *() const;
    operator char*();
    operator const wchar_t*() const;
    operator wchar_t*();
    BSTR AsBSTR() const;

    // Relinquish ownership and return contents of this TString. Caller then
    // owns the string & must delete or free it.
    // !CQ currently returns a copy & then frees its copy on destruction
    //
    BSTR     RelinquishSysStr() const;
    wchar_t* RelinquishWide() const;
    char*    RelinquishNarrow() const;
    tchar*    Relinquish() const;

    // Language related
    //
    TLangId GetLangId();
    void    SetLangId(TLangId id);

  protected:
    TUString* S;
};

//
// Provide ANSI to Wide conversion when OLE requires wide chars
// Allocate a unicode BSTR from an ANSI char*
//
# define OleStr(s) ::owl::TString(s)
# define OleText(s) L##s
  inline BSTR SysAllocString(const char * str) {
    return TOleAuto::SysAllocString((wchar_t*)(const wchar_t*)TString(str));
  }

//----------------------------------------------------------------------------

//
/// \class TUString
// ~~~~~ ~~~~~~~~
/// Privately used by TString to manage string pointers
/// This is a reference counted union of various string representatons
/// Constructors/destructors are private to enforce reference count model
/// Create functions are used to facilitate rapid allocation schemes
/// Null pointers are never stored; instead a static null object is ref'd
//
class _OWLCLASS TUString {
  public:
    static TUString* Create(const char * str);
    static TUString* Create(char* str);
    static TUString* Create(const wchar_t* str);
    static TUString* Create(wchar_t* str);
    static TUString* Create(TSysStr& str, bool loan, TLangId lang = 0);
    static TUString* Create(BSTR str, bool loan, TLangId lang = 0);
    static TUString* Create(const tstring& str);

    TUString* Assign(const TUString& s);
    TUString* Assign(const tstring& s);
    TUString* Assign(const char * s);
    TUString* Assign(char* s);
    TUString* Assign(const wchar_t* s);
    TUString* Assign(wchar_t* s);
    TUString* Assign(BSTR str, TLangId lang);
    operator const char *() const;
    operator char*();
    operator const wchar_t*() const;
    operator wchar_t*();

    TUString& operator ++();    ///< Preincrement operator only
    TUString& operator --();    ///< Predecrement operator only

    int  Length() const;        ///< Return appropriate string length
    bool IsNull() const;        ///< Is the string a null string?
    bool IsWide() const;        ///< Are the string contents any kind of wide?

    TLangId Lang;
    void RevokeBstr(BSTR s);    ///< Used to restore if Created with loan==true
    void ReleaseBstr(BSTR s);   ///< Used to unhook  if Created with loan==true

    static wchar_t* ConvertAtoW(const char* src, size_t len = (size_t)-1);
    static char* ConvertWtoA(const wchar_t* src, size_t len = (size_t)-1);
    static BSTR ConvertAtoBSTR(const char* src);
    BSTR ConvertToBSTR();

#if defined(BI_COMP_MSC)    // MSC can't handle the dtor being private
  public:
#else
  private:
#endif
   ~TUString() {Free();}

  private:
    // !CQ the assumption that a const char* is non-volatile is bogus!
    TUString(const char & str);
    TUString(char& str);
    TUString(const wchar_t& str);
    TUString(wchar_t& str);
    TUString(TSysStr& str, bool loan, TLangId lang);
    TUString(BSTR str, bool loan, TLangId lang);
    TUString(const tstring& str);

    void Free();
    tstring& GetOWLString();
    const tstring& GetOWLString() const;
    void AllocOWLString(const tstring& as);

    char*    ChangeToCopy();
    wchar_t* ChangeToWCopy();

    enum TKind {
      isNull,
      isConst, isCopy,
      isWConst, isWCopy,
      isBstr, isExtBstr,
      isString,
#if 0  // if alighn greater then 2 
    } Kind : 16;
    int16 RefCnt;
#else
    } Kind;
    int RefCnt;
#endif
    union {
      const char * Const;  ///< Passed-in string, NOT owned here, read-only
      char*            Copy;   ///< Local copy, must be deleted, read-write
      const wchar_t*  WConst; ///< Unicode version of Const (Win32)
      wchar_t*        WCopy;  ///< Unicode version of Copy (Win32)
      BSTR            Bstr;   ///< Copy of pointer, owned here
      char            StringMem[sizeof(tstring)]; ///< Placeholder for string:: object
    };

    static TUString Null;     // Null TString references this
    TUString() : Lang(0),Kind(isNull),RefCnt(1),Const(0) {} // for Null object

  friend class TString;       // Envelope string class
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inlines
//

//
/// Construct a TString from a character array
//
inline TString::TString(const char * s)
:
  S(TUString::Create(s))
{
}

//
/// Construct a TString from a wide character array
//
inline TString::TString(const wchar_t* s)
:
  S(TUString::Create(s))
{
}

//
/// Construct a TString from a BSTR (OLE String)
//
inline TString::TString(BSTR s, bool loan)
:
  S(TUString::Create(s, loan))
{
}

//
/// Construct a TString from a System string (BSTR)
//
inline TString::TString(TSysStr& s, bool loan)
:
  S(TUString::Create(s, loan))
{
}

//
/// Construct a TString from a string
//
inline TString::TString(const tstring& s)
:
  S(TUString::Create(s))
{
}

//
/// Construct a TString from a TUString
//
inline TString::TString(TUString* s)
:
  S(s)
{
}

//
/// Construct a TString from a TString (Copy Constructor)
//
inline TString::TString(const TString& src)
:
  S(src.S)
{
  ++*S;
}

//
// Destruct a TString (actually decrements a reference counter)
//
inline TString::~TString()
{
  --*S;
}

//
// Return the length of the string
//
inline int TString::Length() const
{
  return S->Length();
}

//
/// Return true if string is empty
//
inline bool TString::IsNull() const
{
  return S->IsNull();
}

//
/// Return true if string uses wide character set
//
inline bool TString::IsWide() const
{
  return S->IsWide();
}

//
/// Copy contents of TString s into this string
//
inline TString& TString::operator =(const TString& s) 
{
  S = S->Assign(*s.S); return *this;
}

//
/// Copy contents of string s into this string
//
inline TString& TString::operator =(const tstring& s)  
{
  S = S->Assign(s); return *this;
}

//
/// Copy contents of const char* s into this string
//
inline TString& TString::operator =(const char * s)
{
  S = S->Assign(s); return *this;
}

//
/// Copy contents of char* s into this string
//
inline TString& TString::operator =(char* s)
{
  S = S->Assign(s); return *this;
}

//
/// Copy contents of const wchar_t* s into this string
//
inline TString& TString::operator =(const wchar_t* s)
{
  S = S->Assign(s); return *this;
}

//
/// Copy contents of wchar_t* s into this string
//
inline TString& TString::operator =(wchar_t* s)       
{
  S = S->Assign(s); return *this;
}

//
/// Return string as a const char *
//
inline TString::operator const char *() const
{
  return S->operator const char *();
}

//
/// Return string as a char*
//
inline TString::operator char*()          
{
  return S->operator char*();
}

//
/// Return string as a const wchar_t*
//
inline TString::operator const wchar_t*() const 
{
  return S->operator const wchar_t*();
}

//
/// Return string as a wchar_t*
//
inline TString::operator wchar_t*()       
{
  return S->operator wchar_t*();
}

//
/// Return a BSTR object owned by 'this' (not by the caller)
//
inline BSTR TString::AsBSTR() const 
{
  return S->ConvertToBSTR();
}

//
/// Return a pointer (BSTR) to a copy of the string
//
inline BSTR TString::RelinquishSysStr() const
{
  return TOleAuto::SysAllocString((wchar_t*)(const wchar_t*)*S);
}

//
/// Return a pointer (wchar_t*) to a copy of the string
//
inline wchar_t* TString::RelinquishWide() const
{
  return strnewdup((const wchar_t*)*S);
}

//
/// Return a pointer (char*) to a copy of the string
//
inline char* TString::RelinquishNarrow() const
{
  return strnewdup((const char*)*S);
}

//
/// Return a pointer (tchar*) to a copy of the string
//
inline tchar* TString::Relinquish() const
{
#if !defined(UNICODE)
  return RelinquishNarrow();
#else
  return RelinquishWide();
#endif
}

//
/// Get Language Id of this string
//
inline TLangId TString::GetLangId()
{
  return S->Lang;
}


//
/// Set Language Id of this string
//
inline void TString::SetLangId(TLangId id)
{
  S->Lang = id;
}

//----------------------------------------------------------------------------

//
/// Increment reference counter for this string
//
inline TUString& TUString::operator ++()
{
  ++RefCnt;
  return *this;
}

//
/// Decrement reference counter for this string
//
inline TUString& TUString::operator --()
{
  if (--RefCnt != 0)
    return *this;
  delete this;
  return Null;
}

//
/// Return true if string is empty
//
inline bool TUString::IsNull() const
{
  return Kind == isNull;
}

//
/// Return true if string uses wide character set
//
inline bool TUString::IsWide() const
{
  return Kind == isWConst || Kind == isWCopy || Kind == isBstr || Kind == isExtBstr;
}

inline tstring& TUString::GetOWLString()
{
  CHECK(Kind == isString);
  return *reinterpret_cast<tstring*>(StringMem);
}

inline const tstring& TUString::GetOWLString() const
{
  CHECK(Kind == isString);
  return *reinterpret_cast<const tstring*>(StringMem);
}

inline void TUString::AllocOWLString(const tstring& as)
{
  new(StringMem) tstring(as);
}

} // OWL namespace


#endif  // OWL_STRING_H

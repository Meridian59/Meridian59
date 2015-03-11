//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TLocaleString implementation - localized name support
/// TRegList implementation - runtime component and object registration list
/// TRegItem implementation - runtime component and object registration item
///
/// \note This code must reside in the same module that the strings are defined
///       The cache, NativeLangId, HINSTANCE are managed on a per-module basis
///       TLocaleString::NativeLangId may be user-implemented to symbol langid
///       TLocaleString::Module may be reset from this to another resource DLL
//----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/lclstrng.h>
//#include <stdio.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <owl/appdict.h>
#include <owl/applicat.h>


namespace owl {

//----------------------------------------------------------------------------
// Module global default values - except for TLocaleString::NativeLangId
//
TLangId TLocaleString::SystemDefaultLangId = TLocaleString::GetSystemLangId();
TLangId TLocaleString::UserDefaultLangId   = TLocaleString::GetUserLangId();
HINSTANCE TLocaleString::Module = 0;
static tchar* null_string = _T("");
TLocaleString TLocaleString::Null = {null_string};

//----------------------------------------------------------------------------
// _TLocaleCache definitions, private for implementation
//

#define AUTOLANG_CACHEDNEUT 0x02 // prefix indicates cache entry with neutral
#define AUTOLANG_CACHEDLOAD 0x01 // prefix indicates Neutral is not a string
const TLangId InvalidLangId = 0xFFFF;

struct _TLocaleCache;
struct _TLocaleCacheBase;

//
// Static object to hold destructable pointer
//
/// \cond
struct _TLocaleCacheList 
#if defined(BI_MULTI_THREAD_RTL)
                : public TLocalObject
#endif
{
  _TLocaleCacheList() : Next(0) 
    {
    }
 ~_TLocaleCacheList();                        // releases cache entries

  _TLocaleCache* Lookup(const tchar* name); // returns cache entry, 0 if failed
  void AddLink(_TLocaleCacheBase* next);     // Add link

  _TLocaleCacheBase* Next;                    // linked list of cached translations

#if defined(BI_MULTI_THREAD_RTL)
//    TMRSWSection  Lock;
#endif
};
/// \endcond

//
// Static instance of the colors 
//
static _TLocaleCacheList& GetLocaleCacheList() 
{
#if defined(BI_MULTI_THREAD_RTL)
  static TTlsContainer<_TLocaleCacheList> localeCacheList;
  return localeCacheList.Get();
#else
  static _TLocaleCacheList localeCacheList;
  return localeCacheList;
#endif
};

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  _TLocaleCacheList& InitLocalCacheList = GetLocaleCacheList(); 
}

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKCASHE //TMRSWSection::TLock Lock(GetLocaleCacheList().Lock);
#else
#define LOCKCASHE
#endif



//
// This base struct is used to cache failure to find language resource
//
/// \cond
struct _TLocaleCacheBase {
  long  Hash;            // hashed original string, for duplicate elimination
  const tchar* Neutral;   // original string, +1 if resource found and loaded
  _TLocaleCacheBase* Next;// linked list of cached strings, for search, cleanup

  _TLocaleCacheBase(const tchar* name, long hash);
};

//
// Buffer follows this header, sized for maximum string + null term.
//
struct _TLocaleCache : public _TLocaleCacheBase {
  void* operator new(size_t size, unsigned buflen);
#if defined(BI_COMP_MSC) && _MSC_VER >= 1200
  void operator delete(void* p, unsigned buflen);
#endif
  _TLocaleCache(const tchar* name, long hash, HRSRC rscHdl, HGLOBAL resData);
 ~_TLocaleCache() {}
  const tchar* Translate(TLangId lang);   // (re)translate string

  TLangId ActLangId;   // actual language ID of cached string
  TLangId ReqLangId;   // requested language ID of cached string
  HRSRC   ResHdl;      // handle returned from ::FindResource()
  tchar  Buf[1];      // first character is string type
};
/// \endcond


//----------------------------------------------------------------------------
// _TLocaleCache implementation
//

_TLocaleCacheBase::_TLocaleCacheBase(const tchar* name, long hash)
:
  Hash(hash),
  Neutral(name),
  Next(0)
{
  GetLocaleCacheList().AddLink(this);
}

void* _TLocaleCache::operator new(size_t size, unsigned buflen)
{
  return ::operator new(size+buflen);
}
#if defined(BI_COMP_MSC) && _MSC_VER >= 1200
void _TLocaleCache::operator delete(void* p, unsigned )
{
  ::operator delete(p);
}
#endif

_TLocaleCache::_TLocaleCache(const tchar* name, long hash,
                           HRSRC resHdl, HGLOBAL resData)
:
  _TLocaleCacheBase(name, hash),
  ResHdl(resHdl)
{
  ReqLangId = ActLangId = InvalidLangId;     // indicate initializing state
  *(HGLOBAL*)(Buf+1) = resData;   // store resource pointer temp in buffer
}
//
//
//
_TLocaleCache* _TLocaleCacheList::Lookup(const tchar* name)
{
  LOCKCASHE
  const tchar* neut = name + 1;      // skip over prefix flag tchar
  long hash = 0;
  const tchar* pc = name;
  while (*pc)
    hash = hash*2 ^ *pc++;
  for (_TLocaleCacheBase* entry = Next; entry; entry = entry->Next) {
    if (hash == entry->Hash) {
      const tchar* pc = entry->Neutral;
      if (*pc != *neut)       // Neutral points to prefix if lookup failed
        pc++;
      if (TLocaleString::CompareLang(pc,neut,TLocaleString::NativeLangId) != 0)
        return  pc == entry->Neutral ? (_TLocaleCache*)entry : 0;
    }
  }
  pc = name;
  if (*name != AUTOLANG_RCID)
    pc++;                    // '#' part of Id
  HRSRC resHdl = ::FindResource(TLocaleString::Module ? TLocaleString::Module : 
                     (HINSTANCE)*OWLGetAppDictionary().GetApplication(0),
                      pc, RT_LOCALIZATION);
  if (!resHdl) {
    new _TLocaleCacheBase(name, hash);    // add cache entry for failed lookup
    return 0;
  }
  HGLOBAL resData = ::LoadResource(TLocaleString::Module ? TLocaleString::Module : 
                      (HINSTANCE)*OWLGetAppDictionary().GetApplication(0), 
                        resHdl);
  if (!resData) {
    return 0;     // should throw exception on failure?!!
  }
  _TUCHAR * pr = (_TUCHAR *)::LockResource(resData);
  int maxLen = sizeof(HGLOBAL);  // scan for longest string, including null
  _TUCHAR c = *pr;         // check first byte of langid or neutral text
  if (c == 0) {                  // check for empty resource string
    ::FreeResource(resData);
    new _TLocaleCacheBase(name, hash); // add failed cache entry if null or err
    return 0;
  }
  if (c >= _T(' '))                  // check for unprefixed neutral string first
    pr--;                        // cancel ++ in for loop initialization
  else
    pr++;                        // start to skip over 2-byte language id
  do {                           // loop to check for maximum string length
    _TUCHAR * p = ++pr; // skip over id to start of translation
    while ((c=*pr++) >= _T(' ')) ;   // skip over translation string
    if ((int)(pr-p) > maxLen)    // update maximum, including terminator
      maxLen = (int)(pr-p);
  } while(c);
  _TLocaleCache* cache = new(maxLen)_TLocaleCache(neut, hash, resHdl, resData);
  cache->Buf[0] = tchar(*name == AUTOLANG_XLAT ? AUTOLANG_CACHEDNEUT
                                              : AUTOLANG_CACHEDLOAD);
  return cache;
}
//
const tchar* 
_TLocaleCache::Translate(TLangId reqLang)
{
  HGLOBAL resData;
  if (ReqLangId == InvalidLangId) { // if first time called after construction
    resData = *(HGLOBAL*)(Buf+1);
    ReqLangId = reqLang;
  }
  else {
    if (Buf[0]==AUTOLANG_CACHEDNEUT && TLocaleString::IsNativeLangId(reqLang))
      return Neutral;
    if (reqLang == ActLangId)
      return Buf+1;
    if (reqLang == ReqLangId) {
      if (ActLangId != InvalidLangId)
        return Buf+1;
      else if (Buf[0] == AUTOLANG_CACHEDNEUT)
        return Neutral;
      else
        return 0;
    }
    if ((resData = ::LoadResource(TLocaleString::Module ? TLocaleString::Module : 
                         (HINSTANCE)*OWLGetAppDictionary().GetApplication(0), 
                           ResHdl)) == 0)
      return Neutral;   // should throw exception on failure?!!
  }

  _TUCHAR  * resBuf = (_TUCHAR *)::LockResource(resData);
  _TUCHAR * translation = 0;
  _TUCHAR * pr = resBuf;
  TLangId actLang = InvalidLangId;
  TLangId resLang;
  _TUCHAR c;

  while ((c = *pr) != 0) {
    if (c > _T(' ')) { // check for initial neutral string, used with CACHEDLOAD
      actLang = resLang = TLocaleString::NativeLangId;
      translation = pr;    // lowest preference match
    }
    else {
      resLang = TLangId(((c - 1)<<10) | *++pr);
      pr++;
    }
    if (resLang == reqLang) {     // exact match
      translation = pr;
      actLang = resLang;
      break;
    }
    if ((tchar)resLang == (tchar)reqLang) {   // base language match
      if ((tchar)actLang != (tchar)reqLang || resLang == (reqLang & 0x00FF)) {
        translation = pr;
        actLang = resLang;
      }
    }
    for ( ; *pr >= _T(' '); ++pr)   // skip over translation string till next Id
      ;
  }
  const tchar* retVal;
  if (translation) {
    while (*translation < _T(' '))    // skip over multiple language IDs
      translation += 2;
    if (actLang != ActLangId) {   // if same as in buffer, leave alone
      tchar* pc;
      for (pr = translation, pc = Buf + 1; *pr >= _T(' '); )
        *pc++ = *pr++;
      *pc = 0;
      ActLangId = actLang;
      if (reqLang != ActLangId)
        ReqLangId = reqLang;
    }
    retVal = Buf+1;
  }
  else if (Buf[0] == AUTOLANG_CACHEDNEUT) {
    retVal = Neutral;
  }
  else {
    retVal = 0;
  }
  ::FreeResource(resData);
  return retVal;
}
//
_TLocaleCacheList::~_TLocaleCacheList()
{
  LOCKCASHE
  while (Next) {
    _TLocaleCacheBase* p = Next;
    Next = Next->Next;
    delete p;
  }
}
//
void _TLocaleCacheList::AddLink(_TLocaleCacheBase* link)
{
  LOCKCASHE
  link->Next = Next;
  Next = link;
}


//----------------------------------------------------------------------------
// TLocaleString implementation, except for static int CompareLang(s1,s2,lang)
//

/// Translates the string to the given language. Translate follows this order of
/// preference in order to choose a language for translation:
/// - 1.	Base language and dialect.
/// - 2.	Base language and no dialect.
/// - 3.	Base language and another dialect.
/// - 4.	The native language of the resource itself.
/// - 5.	Returns 0 if unable to translate the string. (This can happen only if an @
/// or # prefix is used; otherwise, the ! prefix indicates that the string following
/// is the native language itself.)
const tchar* TLocaleString::Translate(TLangId reqLang)
{
  if (!Private)                   // check for null string pointer
    return Private;

  if (reqLang == LangNeutral)
    reqLang = NativeLangId;
  else if (reqLang == LangSysDefault)
    reqLang = SystemDefaultLangId;
  else if (reqLang == LangUserDefault)
    reqLang = UserDefaultLangId;

  _TLocaleCache* cache;
  switch (Private[0])
  {
    default:                      // untranslatable string, no prefix
      return Private;

    case AUTOLANG_XLAT:           // not yet translated
      if (IsNativeLangId(reqLang))
        return Private+1;         // resource name IS neutral or default name
      if ((cache =   GetLocaleCacheList().Lookup(Private)) == 0)
        return ++Private;         // permanently bump pointer to make constant
      Private = cache->Buf;       // point to buffer in cache
      return cache->Translate(reqLang);

    case AUTOLANG_LOAD:           // named resource not accessed yet
    case AUTOLANG_RCID:           // numeric resource not accessed yet
      if ((cache = GetLocaleCacheList().Lookup(Private)) == 0)
        return (Private = 0);     // permanently set pointer to null
      Private = cache->Buf;       // point to buffer in cache
      return cache->Translate(reqLang);

    case AUTOLANG_CACHEDNEUT:     // string in cache with neutral pointer
    case AUTOLANG_CACHEDLOAD:     // string in cache with no neutral pointer
      cache = (_TLocaleCache*)(Private+1) - 1;   // backup to point to header
      return cache->Translate(reqLang);
  }
}

TLocaleString::operator const tchar*()
{
  if (Private == 0)
    return 0;

  switch (Private[0]) {
  case AUTOLANG_XLAT:       // not yet translated
  case AUTOLANG_CACHEDNEUT: // translated string in cache
  case AUTOLANG_CACHEDLOAD: // translated or neutral name in cache
    return Private+1;

  case AUTOLANG_RCID:       // resource not accessed yet
  case AUTOLANG_LOAD:       // resource not accessed yet
    return 0;

  default:                  // untranslatable string, no prefix
    return Private;
  }
}

/// Using the specified language (lang), Compare compares TLocaleString with another
/// string. It uses the standard string compare and the language-specific collation
/// scheme. It returns one of the following values:
/// - \c \b  	0	There is no match between the two strings.
/// - \c \b  	1	This string is greater than the other string.
/// - \c \b  	-1	This string is less than the other string.
int TLocaleString::Compare(const tchar * str, TLangId lang)
{
  return CompareLang(this->Translate(lang), str, lang);
}

/// Returns true if lang equals the native system language.
int TLocaleString::IsNativeLangId(TLangId lang)
{
  return lang == NativeLangId || lang == (NativeLangId & 0x00FF);
}


} // OWL namespace


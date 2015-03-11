//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TLocaleString - localized name support
//----------------------------------------------------------------------------

#if !defined(OWL_LCLSTRNG_H)
#define OWL_LCLSTRNG_H

#include <owl/defs.h>
#include <owl/private/defs.h>
#include <owl/private/number.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

namespace owl {

#define DEFAULT_SYSTEM_LANGID 0x409   // in case 16-bit with no NLS support

/// Holds a language ID, a predefined number that represents a base language and
/// dialect. For example, the number 409 represents American English. TLocaleString
/// uses the language ID to find the correct translation for strings.
typedef owl::uint16 TLangId;               // language ID - same as NT LANGID

typedef int (*TstricmpLang)(const tchar *, const char *, TLangId);

const TLangId LangSysDefault  = 0x0800;
const TLangId LangUserDefault = 0x0400;
const TLangId LangNeutral     = 0x0000;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>



//----------------------------------------------------------------------------
/// \struct TLocaleString
/// localizable substitute for char*
//
/// Designed to provide support for localized registration parameters, the
/// TLocaleString Struct defines a localizable substitute for char* strings. These
/// strings, which describe both OLE and non-OLE enabled objects to the user, are
/// available in whatever language the user needs. This Struct supports
/// ObjectWindows' Doc/View as well as ObjectComponents' OLE-enabled applications.
/// The public member functions, which supply information about the user's language,
/// the native language, and a description of the string marked for localization,
/// simplify the process of translating and comparing strings in a given language.
/// 
/// To localize the string resource, TLocaleString uses several user-entered
/// prefixes to determine what kind of string to translate. Each prefix must be
/// followed by a valid resource identifier (a standard C identifier).The following
/// table lists the prefixes TLocaleString  uses to localize strings. Each prefix is
/// followed by a sample entry.
/// 
/// - \c \b  @ TXY	The string is a series of characters interpreted as a resource ID and is
/// accessed only from a resource file. It is never used directly.
/// - \c \b  # 1045	The string is a series of digits interpreted as a resource ID and is
/// accessed from a resource file. It is never used directly.
/// - \c \b  ! MyWindow	The string is translated if it is not in the native language;
/// otherwise, this string is used directly.
/// 
/// 
/// See the section on localizing symbol names in the ObjectWindows Programmer's
/// Guide for more information about localizing strings.
//
struct _OWLCLASS TLocaleString {
  //TLocaleString():Private(0){}
  //TLocaleString(const TLocaleString& lstr):Private(lstr.Private){}
  //TLocaleString(const tchar* str):Private(str){}
  const tchar * Translate(TLangId lang);                      ///< translate string
  operator const tchar *();      ///< return current string
  void operator =(const tchar * str) { Private = str; }
  int   Compare(const tchar* str, TLangId lang);   ///< insensitive compare

  const tchar* Private;         ///< string pointer used for initialization

  static TLangId GetSystemLangId();  ///< platform-dependent implementation
  static TLangId GetUserLangId();    ///< platform-dependent implementation
  static int     IsNativeLangId(TLangId lang);  ///< returns bool true if native
  
/// The default language identifier.
  static TLangId SystemDefaultLangId;///< must define and set to system language
  
/// The user-defined default language identifier.
  static TLangId UserDefaultLangId;  ///< same as system language if single user
  
/// The base language ID of non-localized strings.
  static TLangId NativeLangId;   ///< must define and set to lang. of literals
  
/// The handle of the file containing the resource.
  static HINSTANCE Module;       ///< must define and set to resource module
  static TLocaleString Null;     ///< reference a null string
  static int CompareLang(const tchar * s1, const tchar * s2, TLangId);
          // CompareLang() may be implemented in another module for NLS support
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//
// Prefix characters for locale strings
//
#define AUTOLANG_RCID  '#' ///< indicates name specified by numeric ID
#define AUTOLANG_XLAT  '!' ///< indicates name to be localized (binary)
#define AUTOLANG_LOAD  '@' ///< indicates resource name to load (binary)

//
// Custom resource for translations
//
#define RT_LOCALIZATION MAKEINTRESOURCE(201)




#endif  // OWL_LCLSTRNG_H

//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TLocaleString default NLS compare function - used only if non-OLE2
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/lclstrng.h>

namespace owl {


/// Returns the system language ID, which can be the same as the UserLangId.
TLangId
TLocaleString::GetSystemLangId()
{
  return ::GetSystemDefaultLangID();
}

/// Returns the user language ID. For single user systems, this is the same as
/// LangSysDefault. The language ID is a predefined number that represents a base
/// language and dialect.
TLangId
TLocaleString::GetUserLangId()
{
  return ::GetUserDefaultLangID();
}

/// This function may be re-implemented with enhanced NLS support in another module. 
/// \note That module must be linked in before the library, to override this default
/// implementation.
int
TLocaleString::CompareLang(const tchar * s1, const tchar * s2, TLangId lang)
{
  return ::CompareString(lang, NORM_IGNORECASE | NORM_IGNORENONSPACE, s1, -1, s2, -1) - 2;
}


} // OWL namespace


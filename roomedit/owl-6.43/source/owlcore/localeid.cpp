//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TLocaleString default native language for symbols, only if not user-defined
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/lclstrng.h>

namespace owl {

TLangId TLocaleString::NativeLangId = TLocaleString::GetSystemLangId();

} // OWL namespace


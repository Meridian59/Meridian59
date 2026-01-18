//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TFilterValidator, a validator that filters out keys that
/// are not in the valid character set.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/validate.rh>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/framewin.h>

namespace owl {

OWL_DIAGINFO;

/** Let the compiler know that the following template instances will be defined elsewhere. **/
//#pragma option -Jgx

//
/// Constructs a filter validator object by first calling the constructor inherited
/// from TValidator, then setting ValidChars to validChars.
//
TFilterValidator::TFilterValidator(const TCharSet& validChars)
:
  TValidator()
{
  ValidChars = validChars;
}

//
/// IsValidInput overrides TValidator's virtual function and checks each character
/// in the string str to ensure it is in the set of allowed characters, ValidChar.
/// IsValidInput returns true if all characters in str are valid; otherwise, it
/// returns false.
//
bool
TFilterValidator::IsValidInput(LPTSTR str, bool /*suppressFill*/)
{
  for (LPTSTR p = str; *p; ) {
    uint n = CharSize(p) / sizeof(tchar);
    if (n > 1 || !ValidChars.Has(static_cast<tchar>(*p)))
      return false;
    p += n;
  }
  return true;
}

//
/// IsValid overrides TValidator's virtuals and returns true if all characters in
/// str are in the set of allowed characters, ValidChar; otherwise, it returns
/// false.
//
bool
TFilterValidator::IsValid(LPCTSTR str)
{
  for (LPCTSTR p = str; *p;) {
    uint n = CharSize(p) / sizeof(tchar);
    if (n > 1 || !ValidChars.Has(static_cast<tchar>(*p)))
      return false;
    p += n;
  }
  return true;
}

//
/// Error overrides TValidator's virtual function and displays a message box
/// indicating that the text string contains an invalid character.
//
void
TFilterValidator::Error(TWindow* owner)
{
  PRECONDITION(owner);
  owner->MessageBox(owner->LoadString(IDS_VALINVALIDCHAR), owner->LoadString(IDS_VALCAPTION), MB_ICONERROR | MB_OK);
}


IMPLEMENT_STREAMABLE1(TFilterValidator, TValidator);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TFilterValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TValidator*)GetObject(), is);
  is >> GetObject()->ValidChars;
  return GetObject();
}

//
//
//
void
TFilterValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TValidator*)GetObject(), os);
  os << GetObject()->ValidChars;
}

#endif


} // OWL namespace


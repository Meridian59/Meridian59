//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/framewin.h>

namespace owl {

OWL_DIAGINFO;

// Let the compiler know that the following template instances will be defined elsewhere. 
//#pragma option -Jgx


//
/// Constructs a TLookupValidator object.
//
TLookupValidator::TLookupValidator()
{
}

//
/// IsValid overrides TValidator's virtual function and calls Lookup to find the
/// string str in the list of valid input items. IsValid returns true if Lookup
/// returns true, meaning Lookup found str in its list; otherwise, it returns false.
//
bool
TLookupValidator::IsValid(LPCTSTR str)
{
  return Lookup(str);
}

//
/// Searches for the string str in the list of valid entries and returns true if it
/// finds str; otherwise, returns false. TLookupValidator's Lookup is an abstract
/// method that always returns false. Descendant lookup validator types must
/// override Lookup to perform a search based on the actual list of acceptable
/// items.
//
bool
TLookupValidator::Lookup(LPCTSTR /*str*/)
{
  return true;
}

//----------------------------------------------------------------------------
//
// TStringLookupValidator
//

/// Constructs a string-lookup object by first calling the constructor inherited
/// from TLookupValidator and then setting Strings to strings.
TStringLookupValidator::TStringLookupValidator(TSortedStringArray* strings)
:
  TLookupValidator()
{
  Strings = strings ? strings : new TSortedStringArray;
}

//
/// This destructor disposes of a list of valid strings by calling NewStringList and
/// then disposes of the string-lookup validator object by calling the Destructor
/// inherited from TLookupValidator.
//
TStringLookupValidator::~TStringLookupValidator()
{
  delete Strings;
}

//
/// Overrides TValidator's virtual function and displays a message box indicating
/// that the typed string does not match an entry in the string list.
//
void
TStringLookupValidator::Error(TWindow* owner)
{
  PRECONDITION(owner);
  owner->MessageBox(IDS_VALNOTINLIST, 0, MB_ICONEXCLAMATION|MB_OK);
}

//
/// Overrides TLookupValidator's virtual function. Returns true if the string passed
/// in str matches any of the strings. Uses the search method of the string
/// collection to determine if str is present.
//
bool
TStringLookupValidator::Lookup(LPCTSTR str)
{
  if (Strings)
    return Strings->Find(tstring(str)) != (int)NPOS;
  return false;
}

//
/// Sets the list of valid input string for the string-lookup validator. Disposes of
/// any existing string list and then sets Strings to strings.
//
void
TStringLookupValidator::NewStringList(TSortedStringArray* strings)
{
  delete Strings;
  Strings = strings;
}

/// Adjusts the 'value' of the text, given a cursor position and an amount. Returns
/// the actual amount adjusted.
//
int
TStringLookupValidator::Adjust(tstring& text, int& /*begPos*/, int& /*endPos*/, int amount)
{
  if (!Strings)
    return 0;

  int count = Strings->Size();
  int index = Strings->Find(text);
  if(index == (int)NPOS)
    return 0;
  int newIndex = index + amount;
  if (newIndex < 0)
    newIndex = 0;
  else if (newIndex >= count)
    newIndex = count-1;

  text = (*Strings)[newIndex];
  return newIndex - index;
}


IMPLEMENT_STREAMABLE1(TLookupValidator, TValidator);
IMPLEMENT_STREAMABLE1(TStringLookupValidator, TLookupValidator);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TLookupValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TValidator*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TLookupValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TValidator*)GetObject(), os);
}

//
//
//
void*
TStringLookupValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TLookupValidator*)GetObject(), is);
  unsigned count;
  is >> count;
  GetObject()->Strings = new TSortedStringArray;
  for (unsigned i = 0; i < count; i++ ) {
    tstring temp;
    is >> temp;
    GetObject()->Strings->Add(temp);
  }
  return GetObject();
}

//
//
//
void
TStringLookupValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TLookupValidator*)GetObject(), os);
  unsigned count = GetObject()->Strings->Size();
  os << count;
  for (unsigned i = 0; i < count; i++)
    os << (*GetObject()->Strings)[i];
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


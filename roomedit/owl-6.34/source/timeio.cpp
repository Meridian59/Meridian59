//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TTime class IO and conversion implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/time.h>

#include <owl/file.h>

#include <owl/private/strmdefs.h>
#include <iomanip>

using namespace std;

namespace owl {

//
// Static variable intialization:
//
int TTime::PrintDateFlag = 1;

//
/// Returns a string object containing the time.
//
tstring TTime::AsString() const
{
  tostringstream strtemp;
  strtemp << (*this);
  return strtemp.str();
}

/* -------------------------------------------------------------------------- */

//
/// Inserts time t into output stream os.
//
_OWLCFUNC(tostream &) operator << (tostream & s, const TTime & t)
{
  // We use an ostrstream to format into buf so that
  // we don't affect the ostream's width setting.
  //
  tostringstream out;

  // First print the date if requested:
  //
  if(TTime::PrintDateFlag)
      out << TDate(t) << _T(" ");

  unsigned hh = t.Hour();
  out << (hh <= 12 ? hh : hh-12) << _T(':');
  out << setfill(_T('0'));
  out << setw(2);
  out << t.Minute() << _T(':');
  out << setw(2);
  out << t.Second() << _T(' ');
  out << setfill(_T(' '));
  out << ( hh<12 ? _T("am") : _T("pm"));

  s << out.str();
  return s;
}

/// Set flag to 1 to print the date along with the time; set to 0 to not print the
/// date. Returns the old setting.
int TTime::PrintDate( int f )
{
  int temp = PrintDateFlag;
  PrintDateFlag = f;
  return temp;
}

/// Inserts time t into persistent stream s.
_OWLCFUNC(tostream  &) operator << ( tostream  & os, const TFileStatus  & status )
{
  os << _T("File Status: ") << status.fullName << endl;
  os << _T("    created: ") << status.createTime << endl;
  os << _T("   modified: ") << status.modifyTime << endl;
  os << _T("   accessed: ") << status.accessTime << endl;
  os << _T("       size: ") << status.size << endl;
  os << _T(" attributes: ") << (int)status.attribute << endl;
  return os;
}

} // OWL namespace
/* ========================================================================== */

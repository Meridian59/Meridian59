//----------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TDate class IO and conversion implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/date.h>
#include <stdio.h>
#include <ctype.h>

#include <owl/private/strmdefs.h>
#include <owl/profile.h>

using namespace std;

namespace owl {

//
//
//
TDate::HowToPrint TDate::PrintOption = TDate::Normal;
//TDate::HowToPrint TDate::PrintOption = TDate::WinIntSection;

//
/// Converts the TDate object to a string object.
//
tstring TDate::AsString() const
{
  tostringstream strtemp;
  strtemp << (*this);
  return strtemp.str();
}

//
/// Sets the print option for all TDate objects and returns the old setting.
//
TDate::HowToPrint TDate::SetPrintOption( HowToPrint h )
{
  HowToPrint oldoption = PrintOption;
  PrintOption = h;
  return oldoption;
}

//
// Skip any characters except alphanumeric characters
//
_OWLSTATICFUNC(static void) SkipDelim(tistream& strm )
{
  tchar c;
  if( !strm.good() )
    return;

  do  {
    strm >> c;
  } while (strm.good() && !(_istdigit(c)||_istalpha(c))) ; //_istalnum is not yet implemented in WineLib

  if (strm.good())
    strm.putback(c);
}

//
// Parse the name of a month from input stream.
//
_OWLSTATICFUNC(static tchar*)
ParseMonth(tistream& s, tchar* month)
{
//  static tchar month[12];
  tchar* p = month;
  tchar c;
  SkipDelim(s);
  s.get(c);
  while (s.good() && _istalpha(c) && (p != &month[10])){
    *p++ = c;
    s.get(c);
  }
  if( s.good() )
    s.putback(c);
  *p = _T('\0');
  return month;
}

//
//  Parse a date from the specified input stream.
//    The date must be in one of the following forms:
//                dd-mmm-yy, mm/dd/yy, or mmm dd,yy
//        e.g.: 10-MAR-86,  3/10/86, or March 10, 1986.
//  Any non-alphanumeric character may be used as a delimiter.
//
void TDate::ParseFrom(tistream& s )
{
  unsigned d,m,y;
  JulTy julnum = 0; // Assume failure.
  tchar month[20];

  if (s.good()){
    SkipDelim(s);
    s >> m;                 // try to parse day or month number
    SkipDelim(s);
    if (s.eof())
      return;
    if( s.fail()){          // parse <monthName><day><year>
      s.clear();
      m = IndexOfMonth(ParseMonth(s,month)); // parse month name
      SkipDelim(s);
      s >> d;                 // parse day
    }
    else{                        // try to parse day number
      s >> d;
      if (s.eof())
        return;
      if (s.fail()){          // parse <day><monthName><year>
        d = m;
        s.clear();
        m = IndexOfMonth(ParseMonth(s,month)); // parse month name
      }
    }
    SkipDelim(s);
    s >> y;
  }
  julnum = !s.fail() ? Jday(m, d, y) : 0;
  if (julnum != 0)
    Julnum = julnum;
  else
    s.clear(tistream::failbit);
}

namespace
{

  void FormatDate(tostream& out, LPCTSTR picture, const TDate& d)
  {
    out.fill(_T('0'));
    while (*picture) 
    {
      switch (*picture) 
      {
      case _T('d'):  case _T('D'): 
        {
          int length = 0;
          while (*picture == _T('d') || *picture == _T('D')) 
          {
            picture++;
            length++;
          }
          out.width(length <  2 ? 2 : length);
          out << d.DayOfMonth();
          break;
        }
      case _T('m'):  case _T('M'): 
        {
          int length = 0;
          while (*picture == _T('m') || *picture == _T('M')) 
          {
            picture++;
            length++;
          }
          out.width(length <  2 ? 2 : length);
          out << d.Month();
          break;
        }
      case _T('y'):  case _T('Y'): 
        {
          while (*picture == _T('y') || *picture == _T('Y'))
            picture++;
          out << d.Year();
          break;                
        }
      default: 
        {
          out << *picture;
          picture++;
          break;
        }
      }
    }
  }

}

//
/// Inserts date into output stream os.
//
_OWLCFUNC( tostream&) operator << (tostream& s, const TDate& d)
{
  // We use a string stream to format the date so that
  // we don't affect the ostream's width setting.
  //
  tostringstream out;
  switch (TDate::PrintOption)
  {
  case TDate::Normal:
    out.width(2);
    out.fill(_T('0'));
    out << d.NameOfMonth() << _T(" ") << d.DayOfMonth()  << _T(", ") << d.Year();
    break;

  case TDate::Terse:
    {
      tchar buf[80]; // NB! Arbitrary size!
      _stprintf(buf,_T("%02u-%.3s-%.4u"), d.DayOfMonth(), d.NameOfMonth(), d.Year());
      out << buf;
      break;
    }

  case TDate::Numbers:
    out.width(2);
    out.fill('0');
    out << d.Month() << _T("/") << d.DayOfMonth() << _T("/") << d.Year();
    break;

  case TDate::EuropeanNumbers:
    out.width(2);
    out.fill('0');
    out << d.DayOfMonth() << _T("/") << d.Month() << _T("/") << d.Year();
    break;

  case TDate::European:
    out.width(2);
    out.fill('0');
    out << d.DayOfMonth() << _T(" ") << d.NameOfMonth() << _T(" ") << d.Year();
    break;

  case TDate::WinIntSection: 
    {
    TProfile p(_T("intl"));
    tstring shortFmt = p.GetString(_T("sShortDate"), _T("mm/dd/yyyy"));
    FormatDate(out, shortFmt.c_str(), d);
    break;
    }
  }

  s << out.str();
  return s;
}

//
/// Extracts date from input stream is.
//
_OWLCFUNC(tistream&) operator >> ( tistream& s, TDate& d )
{
  d.ParseFrom(s);
  return s;
}

} // OWL namespace
/* ========================================================================== */


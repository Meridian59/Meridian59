//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of persistent streaming for window system geometry classes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/geometry.h>

namespace owl {

//----------------------------------------------------------------------------
// TRect streaming
//

//
/// Extracts the rectangle from a regular input stream.
//
_OWLCFUNC(tistream&)
operator >>(tistream& is, TRect& r)
{
  tchar ch;
  return is >> ch >> r.left >> ch >> r.top >> ch
            >> r.right >> ch >> r.bottom >> ch;
}

//
/// Formats and inserts the given TRect object, r, into the ostream, os. The format
/// is (r.left, r.top)(r.right, r.bottom). Returns a reference to the resulting
/// stream and allows the usual chaining of << operations.
//
_OWLCFUNC(tostream&)
operator <<(tostream& os, const TRect& r)
{
  return os << _T('(') << r.left << _T(',') << r.top << _T('-')
            << r.right << _T(',') << r.bottom << _T(')');
}

//----------------------------------------------------------------------------
// TPointL streaming
//

//
// Extract the point from a regular input stream.
//
_OWLCFUNC(tistream&)
operator >>(tistream& is, TPointL& p)
{
  tchar c;

  is >> c;

  long x;
  is >> x;
  p.x = x;

  is >> c;

  long y;
  is >> y;
  p.y = y;

  is >> c;

  return is;
}

//
// Insert the point into an output stream.
//
_OWLCFUNC(tostream&)
operator <<(tostream& os, const TPointL& p)
{
  return os << _T('(') << p.x << _T(',') << p.y << _T(')');
}

//----------------------------------------------------------------------------
// TPointF streaming
//

//
// Extract a point from a regular input stream.
//
_OWLCFUNC(tistream&)
operator >>(tistream& is, TPointF& p)
{
  tchar c;
  return is >> c >> p.x >> c >> p.y >> c;
}

//
// Insert a point into the output stream.
//
_OWLCFUNC(tostream&)
operator <<(tostream& os, const TPointF& p)
{
  return os << _T('(') << p.x << _T(',') << p.y << _T(')');
}


//----------------------------------------------------------------------------
// TPoint streaming
//

//
/// Extracts a TPoint object from stream is, and copies it to p. Returns a reference
/// to the resulting stream, allowing the usual chaining of >> operations.
//
_OWLCFUNC(tistream&)
operator >>(tistream& is, TPoint& p)
{
  tchar c;
  return is >> c >> p.x >> c >> p.y >> c;
}

//
/// Formats and inserts the given TPoint object p into the ostream os. The format is
/// "(x,y)". Returns a reference to the resulting stream, allowing the usual
/// chaining of << operations.
//
_OWLCFUNC(tostream&)
operator <<(tostream& os, const TPoint& p)
{
  return os << _T('(') << p.x << _T(',') << p.y << _T(')');
}

//----------------------------------------------------------------------------
// TSize streaming
//

//
/// Formats and inserts the given TSize object s into the ostream os. The format is
/// "(cx x cy)". Returns a reference to the resulting stream, allowing the usual
/// chaining of << operations.
//
_OWLCFUNC(tistream&)
operator >>(tistream& os, TSize& s)
{
  tchar c;
  return os >> c >> s.cx >> c >> s.cy >> c;
}

//
/// Insert a size into an output stream.
//
_OWLCFUNC(tostream&)
operator <<(tostream& os, const TSize& s)
{
  return os << _T('(') << s.cx << _T('x') << s.cy << _T(')');
}

} // OWL namespace


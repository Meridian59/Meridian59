//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of persistent streaming for window system structure and type
/// encapsulation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/geometry.h>

namespace owl {

//----------------------------------------------------------------------------
// TRect streaming
//

//
/// Extracts a TRect object from is, the given input stream, and copies it to r.
/// Returns a reference to the resulting stream, allowing the usual chaining of >>
/// operations.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TRect& r)
{
  return is >> r.left >> r.top >> r.right >> r.bottom;
}

//
/// Inserts the given TRect object, r, into the opstream, os. Returns a reference to
/// the resulting stream, allowing the usual chaining of << operations.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TRect& r)
{
  return os << r.left << r.top << r.right << r.bottom;
}

//----------------------------------------------------------------------------
// TPointL streaming
//

//
// Extract the point from the input persistent stream.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TPointL& p)
{
  long x;
  is >> x;
  p.x = x;

  long y;
  is >> y;
  p.y = y;

  return is;
}

//
// Insert the point into an output persistent stream.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TPointL& p)
{
  return os << p.x << p.y;
}

//----------------------------------------------------------------------------
// TPointF streaming
//

//
// Extract a point from the persistent input stream.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TPointF& p)
{
  return is >> p.x >> p.y;
}

//
// Insert a point into the output persistent stream.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TPointF& p)
{
  return os << p.x << p.y;
}

//----------------------------------------------------------------------------
// TPoint streaming
//

//
/// Extracts a TPoint object from persistent stream is, and copies it to p. Returns
/// a reference to the resulting stream, allowing the usual chaining of >>
/// operations.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TPoint& p)
{
  return is >> p.x >> p.y;
}

//
/// Inserts the given TPoint object p into persistent stream os. Returns a reference
/// to the resulting stream, allowing the usual chaining of << operations.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TPoint& p)
{
  return os << p.x << p.y;
}

//----------------------------------------------------------------------------
// TSize streaming
//

//
/// Extracts a TSize object from is, the given input stream, and copies it to s.
/// Returns a reference to the resulting stream, allowing the usual chaining of >>
/// operations.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TSize& s)
{
  return is >> s.cx >> s.cy;
}

//
/// Inserts the given TSize object s into the opstream os. Returns a reference to
/// the resulting stream, allowing the usual chaining of << operations.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TSize& s)
{
  return os << s.cx << s.cy;
}


} // OWL namespace


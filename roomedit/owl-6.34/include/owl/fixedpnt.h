//----------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Simple fixed point class that maintains numbers as 16.16
/// Fixed by Bidus Yura to work with Micro$oft VC--.
//----------------------------------------------------------------------------

#if !defined(OWL_FIXEDPNT_H)
#define OWL_FIXEDPNT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif



namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TFixedPoint
// ~~~~~ ~~~~~~~~~~~
class TFixedPoint {
  public:
    TFixedPoint(int s = 0) {Value = long(s) << 16;}
    TFixedPoint(int num, int denom) {Value = long(num) * 65536L / denom;}

    // Unary negation operator
    //
    TFixedPoint         operator -() {return -Value;}

    // Postfix increment/decrement operators
    //
    void           operator ++(int) {Value += 1L << 16;}
    void           operator --(int) {Value -= 1L << 16;}

    // Bitwise logical operators
    //
    TFixedPoint    operator <<(unsigned n) {return long(Value << tchar(n));}
    TFixedPoint    operator >>(unsigned n) {return long(Value >> tchar(n));}

    // Assignment operators
    //
    TFixedPoint&   operator <<=(unsigned n) {Value <<= n; return *this;}
    TFixedPoint&   operator >>=(unsigned n) {Value >>= n; return *this;}

    TFixedPoint&   operator +=(const TFixedPoint& f) {Value += f.Value; return *this;}

    TFixedPoint&   operator -=(const TFixedPoint& f) {Value -= f.Value; return *this;}

    TFixedPoint&   operator *=(int s) {Value *= s; return *this;}
    TFixedPoint&   operator *=(const TFixedPoint& f) {Value = (Value >> 8) * (f.Value >> 8);
                                                 return *this;}

    TFixedPoint&   operator /=(int s) {Value /= s; return *this;}
    TFixedPoint&   operator /=(const TFixedPoint& f) {Value /= f.Value >> 8; Value <<= 8;
                                                 return *this;}

    // Binary arithmetic operators
    //
    friend TFixedPoint operator +(const TFixedPoint& l,
                             const TFixedPoint& r) {return l.Value + r.Value;}
    friend TFixedPoint operator +(int           l,
                             const TFixedPoint& r) {return TFixedPoint(l) += r.Value;}
    friend TFixedPoint operator +(const TFixedPoint& l,
                             int           r) {return r + l;}

    friend TFixedPoint operator -(const TFixedPoint& l,
                             const TFixedPoint& r) {return l.Value - r.Value;}
    friend TFixedPoint operator -(int           l,
                             const TFixedPoint& r) {return TFixedPoint(l) -= r.Value;}
    friend TFixedPoint operator -(const TFixedPoint& l,
                             int           r) {return l - TFixedPoint(r);}

    friend TFixedPoint operator *(const TFixedPoint& l,
                             const TFixedPoint& r) {return (l.Value >> 8) * (r.Value >> 8);}
    friend TFixedPoint operator *(int           l,
                             const TFixedPoint& r) {return long(l * r.Value);}
    friend TFixedPoint operator *(const TFixedPoint& l,
                             int           r) {return long(l.Value * r);}

    friend TFixedPoint operator /(const TFixedPoint& l,
                             const TFixedPoint& r) {return (l.Value /(r.Value >> 8)) << 8;}
    friend TFixedPoint operator /(int           l,
                             const TFixedPoint& r) {return (long(l) << 16) / r.Value;}
    friend TFixedPoint operator /(const TFixedPoint& l,
                             int           r) {return long(l.Value / r);}

    // Equality operators
    //
    friend bool   operator ==(const TFixedPoint& l,
                              const TFixedPoint& r) {return l.Value == r.Value;}
    friend bool   operator !=(const TFixedPoint& l,
                              const TFixedPoint& r) {return l.Value != r.Value;}
    friend bool   operator <(const TFixedPoint& l,
                              const TFixedPoint& r) {return l.Value < r.Value;}

    // Conversion operator to int
    //
    operator int() {return int(Value >> 16);}

  private:
    TFixedPoint(long v) {Value = v;}

    long  Value;
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace


#endif  // CLASSLIB_FIXEDPNT_H

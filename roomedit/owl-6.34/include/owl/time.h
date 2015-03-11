//------------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
//
//------------------------------------------------------------------------------

#if !defined(OWL_TIME_H)
#define OWL_TIME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/date.h>
#include <time.h>
#include <owl/private/strmdefs.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS ipstream;
class _OWLCLASS opstream;

class _OWLCLASS TFileTime;

typedef unsigned HourTy;
typedef unsigned MinuteTy;
typedef unsigned SecondTy;
typedef unsigned long ClockTy;

static const unsigned long secFrom_Jan_1_1901_to_Jan_1_1970 = 2177452800UL;

/// The TTime class encapsulates time functions and characteristics.
class _OWLCLASS TTime{
  public:

    friend TDate::TDate( const TTime  & );

    TTime();                  // Current time
    TTime(const TSystemTime  & t); ///< from SYSTEMTIME
    TTime(const TFileTime  & t); ///< from FILETIME
    TTime( ClockTy s );       // Seconds since Jan 1, 1901.
    TTime( HourTy h, MinuteTy m, SecondTy s = 0 );
                                // Specified time and today's date
    TTime( const TDate &, HourTy h=0, MinuteTy m=0, SecondTy s=0 );
                                // Given date and time

    TSystemTime GetSystemTime() const;
    TFileTime   GetFileTime() const;

    tstring AsString() const;
    int CompareTo( const TTime  & ) const;
    unsigned Hash() const;
    HourTy Hour() const;        // hour: local time
    HourTy HourGMT() const;     // hour: GMT
    int IsDST() const;
    int IsValid() const;
    TTime Max( const TTime  & t ) const;
    TTime Min( const TTime  & t ) const;
    MinuteTy Minute() const;    // minute: local time
    MinuteTy MinuteGMT() const; // minute: GMT
    SecondTy Second() const;    // second: local time or GMT
    ClockTy Seconds() const;

    // Write times:
    friend _OWLCFUNC(tostream &) operator << ( tostream  &, const TTime  & );

    // Read or write times on persistent streams
    friend _OWLCFUNC(opstream &) operator << ( opstream  & s, const TTime  & d );
    friend _OWLCFUNC(ipstream &) operator >> ( ipstream  & s, TTime  & d );

    // Boolean operators.
    int operator <  ( const TTime  & t ) const;
    int operator <= ( const TTime  & t ) const;
    int operator >  ( const TTime  & t ) const;
    int operator >= ( const TTime  & t ) const;
    int operator == ( const TTime  & t ) const;
    int operator != ( const TTime  & t ) const;
    int Between( const TTime  & a, const TTime  & b ) const;

    // Add or subtract seconds.
    friend _OWLFUNC(TTime) operator + ( const TTime  & t, long s );
    friend _OWLFUNC(TTime) operator + ( long s, const TTime  & t );
    friend _OWLFUNC(TTime) operator - ( const TTime  & t, long s );
    friend _OWLFUNC(TTime) operator - ( long s, const TTime  & t );
    void operator++();
    void operator--();
    void operator+=(long s);
    void operator-=(long s);

    // Static member functions:
    static TTime BeginDST( unsigned year ); // Start of DST for given year.
    static TTime EndDST( unsigned year );   // End of DST for given year.
    static int PrintDate( int );    // Whether to include date when printing time

protected:

    static int AssertDate( const TDate  & );
    static const TDate RefDate; ///< The minimum valid date for TTime objects: January 1, 1901.
    static const TDate MaxDate; ///< The maximum valid date for TTime objects.

private:

    ClockTy Sec;        ///< Seconds since 1/1/1901.
    static int PrintDateFlag;  ///< True to print date along with time.

    ClockTy LocalSecs() const;
    static TTime BuildLocal( const TDate  &, HourTy );

};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

/// Constructs a TTime object with the given s (seconds since January 1, 1901).
inline TTime::TTime( ClockTy s )
{
  Sec = s;
}

/// Returns 1 if this TTime object contains a valid time, 0 otherwise.
/// \todo Why not return bool?
inline int TTime::IsValid() const
{
  return Sec > 0;
}

/// Returns seconds since January 1, 1901.
inline ClockTy TTime::Seconds() const
{
  return Sec;
}

/// Returns 1 if the target time is less than time t, 0 otherwise.
inline int TTime::operator <  ( const TTime& t ) const
{
  return Sec < t.Sec;
}

/// Returns 1 if the target time is less than or equal to time t, 0 otherwise.
inline int TTime::operator <= ( const TTime& t ) const
{
  return Sec <= t.Sec;
}

/// Returns 1 if the target time is greater than time t, 0 otherwise.
inline int TTime::operator >  ( const TTime& t ) const
{
  return Sec > t.Sec;
}

/// Returns 1 if the target time is greater than or equal to time t, 0 otherwise.
inline int TTime::operator >= ( const TTime& t ) const
{
  return Sec >= t.Sec;
}

/// Returns 1 if the target time is equal to time t, 0 otherwise.
inline int TTime::operator == ( const TTime& t ) const
{
  return Sec == t.Sec;
}

/// Returns 1 if the target time is not equal to time t, 0 otherwise.
inline int TTime::operator != ( const TTime& t ) const
{
  return Sec != t.Sec;
}

/// Returns 1 if the target date is between TTime a and TTime b, 0 otherwise.
inline int TTime::Between( const TTime& a, const TTime& b ) const
{
  return *this >= a && *this <= b;
}

/// Adds s seconds to time t.
inline TTime operator + ( const TTime& t, long s )
{
  return TTime(t.Sec+s);
}

/// Adds s seconds to time t.
inline TTime operator + ( long s, const TTime& t )
{
  return TTime(t.Sec+s);
}

/// Performs subtraction, in seconds, between s and t.
inline TTime operator - ( const TTime& t, long s )
{
  return TTime(t.Sec-s);
}

/// Performs subtraction, in seconds, between s and t.
inline TTime operator - ( long s, const TTime& t )
{
  return TTime(t.Sec-s);
}

/// Increments the time by 1 second.
inline void TTime::operator++()
{
  Sec += 1;
}

/// Decrements the time by 1 second.
inline void TTime::operator--()
{
  Sec -= 1;
}

/// Adds s seconds to the time.
inline void TTime::operator+=(long s)
{
  Sec += s;
}

/// Subtracts s seconds from the time.
inline void TTime::operator-=(long s)
{
  Sec -= s;
}

inline unsigned HashValue( TTime  & t )
{
  return t.Hash();
}

} // OWL namespace


#endif  // OWL_TIME_H

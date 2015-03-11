//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
// 1998 by Bidus Yura
//
/// \file
/// TTime class implementation
///
/// \note Be sure that you have set your environment variable TZ.
///       For example, for Pacific coast time, set TZ=PDT8PST.
///       For other time zones, see your manuals.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/uimetric.h>
#include <owl/time.h>
#include <owl/wsyscls.h>
#include <time.h>
#include <stdio.h>

#if defined(__GNUC__) /*JJH*/
//Jogy 2012-06-21 - is this needed anymore?
extern "C" int _RTLENTRY _daylight __declspec(dllimport);
extern "C" long _RTLENTRY _timezone __declspec(dllimport);
#else
//extern int _RTLENTRY _EXPDATA _daylight;
//extern long _RTLENTRY _EXPDATA _timezone;
#endif

//JJH I moved following line here, it was before definition 
//    of daylight and timezone originally
namespace owl {

enum TimeZone { CarolineIslands=-11,    MarianaIslands,         Japan,
                China,                  minusSeven,             minusSix,
                Pakistan,               CaspianSea,             Turkey,
                Finland,                Europe,                 Greenwich,
                Azores,                 two,                    Greenland,
                Atlantic,               USEastern,              USCentral,
                USMountain,             USPacific,              Alaska,
                Hawaii,                 Bearing};

static const unsigned long SECONDS_IN_DAY  = 86400L;
static const unsigned long SECONDS_IN_HOUR = 3600L;
static const unsigned      SECONDS_IN_MIN  = 60;

struct TInitTime
{
    TInitTime()
        { _tzset(); }
};

static const TInitTime cludgeTime;      // To force the call to tzset()


const TDate TTime::RefDate( (DayTy)0, (YearTy)0 );
const TDate TTime::MaxDate( (DayTy)49709L, (YearTy)0 ); // ((2**32)-1)/SECONDS_IN_DAY -1
static const int SUNDAY = 7;

/// Returns 1 if d is between the earliest valid date (RefDate) and the latest valid
/// date (MaxDate).
int TTime::AssertDate( const TDate  & date )
{
    return date.Between(RefDate,MaxDate);
}

//----------------------------------------------------------------------------
//                     private member functions

//
/// Adjust for local time zone and Daylight Savings Time.
//
ClockTy TTime::LocalSecs() const
{
    TTime local_time( Sec - _timezone );
    if (local_time.IsDST())
        local_time.Sec += SECONDS_IN_HOUR;
    return local_time.Sec;
}

//
/// Builds the time from a local time, adjusting to GMT.  Does *not* adjust for DST.
//
TTime TTime::BuildLocal( const TDate  & date, HourTy h )
{
    return TTime( SECONDS_IN_DAY * (date-RefDate) +
                  SECONDS_IN_HOUR * h +
                  _timezone);
}

//----------------------------------------------------------------------------
//                  public static member functions

//
/// Return the time at which DST starts for the given year.
///
/// Note that the time returned is the time at which DST starts locally,
/// but it is returned in GMT.
//
TTime TTime::BeginDST( unsigned year )
{
  if( year > 1986 ){
    TDate endMarch(31, 3, year);
    return BuildLocal( endMarch.Previous(SUNDAY)+7, 2 );
  }

  // Ah, remember those energy conscious years...???
  if( year==1974 )
    return BuildLocal( TDate(6,1,1974), 2 );
  if( year==1975 )
    return BuildLocal( TDate(23,2,1975), 2 );

  TDate endApril( 30, 4, year );
  return BuildLocal( endApril.Previous(SUNDAY), 2 );
}


//
/// Return the time at which DST ends for the given year.
///
/// Note that the time returned is the time at which DST ends locally,
/// but it is returned in GMT.
//
TTime TTime::EndDST( unsigned year )
{
  TDate endOctober( 31, 10, year );
  return BuildLocal( endOctober.Previous(SUNDAY), 1 );
}


//----------------------------------------------------------------------------
//                            constructors

//
/// Construct TTime with current time (seconds since Jan 1, 1901).
//
TTime::TTime()
{
#if 0
  time_t ltime;
  time(&ltime);
  struct tm *t = localtime(&ltime);

  // Construct the date.  The time struct returns int, so casts are used.
  //
  TDate today( (DayTy)t->tm_mday,
               (MonthTy)(t->tm_mon + 1),
               (YearTy)t->tm_year );// +1900 //????????

  *this = TTime( today,
                 (HourTy)t->tm_hour,
                 (MinuteTy)t->tm_min,
                 (SecondTy)t->tm_sec );
#else
  TSystemTime _clk(TSystemTime::LocalTime());
  *this = TTime(TDate(_clk), 
                _clk.GetHour(), 
                _clk.GetMinute(), 
                _clk.GetSecond());
#endif
}

//
/// Specified time and today's date:
//
TTime::TTime( HourTy h, MinuteTy m, SecondTy s )
{
  Sec = TTime( TDate(),h,m,s ).Sec;
}

//
/// Construct a Time for the specified (local) Date, hour, minute, and second.
/// \note this algorithm will fail if DST correction is something other
/// than an hour.
/// It is complicated by the DST boundary problem:
/// - 1) Times in the phantom zone between 2AM and 3AM when DST is invoked are invalid.
/// - 2) Times in the hour after 1AM when DST ends, are redundant.
///
/// Checking for these situations necessitates a lot of jumping back
/// and forth by an hour to check for the boundary.
//
TTime::TTime( const TDate  & date, HourTy h, MinuteTy m, SecondTy s )
{
  if( date.IsValid() ){
    Sec =   SECONDS_IN_DAY  * (date-RefDate) +
            SECONDS_IN_HOUR * (h-1L) +    /* Note the adjustment by one hour */
            SECONDS_IN_MIN  * m + s;
     if( Sec )
      Sec += _timezone;           // Adjust to GMT.

    if( IsDST() ){
      Sec += SECONDS_IN_HOUR;
      if( IsDST() )
        Sec -= SECONDS_IN_HOUR;
    }
    else{
      Sec += SECONDS_IN_HOUR;
      if( IsDST() )
        Sec = 0;            // Invalid "phantom" time.
    }
  }
  else
    Sec = 0;    // Invalid date
}

//
//
//
TTime::TTime(const TSystemTime  & t)
{
  Sec = TTime(TDate(t), t.GetHour(), t.GetMinute(), t.GetSecond()).Sec;
}

//
//
//
TSystemTime
TTime::GetSystemTime() const
{
  TDate dt(*this);
  return TSystemTime(dt.Year(), dt.Month(), dt.DayOfMonth(), 
                     Hour(),Minute(), Second(), dt.WeekDay());
}

//
//
//
TTime::TTime(const TFileTime  & tim)
{
  TSystemTime t(tim);
  Sec = TTime(TDate(t), t.GetHour(), t.GetMinute(), t.GetSecond()).Sec;
}

//
//
//
TFileTime
TTime::GetFileTime() const
{
  return TFileTime(GetSystemTime());
}

//----------------------------------------------------------------------------
//                     public member functions

/// Compares t to this TTime object and returns 0 if the times are equal, 1 if t is
/// earlier, and -1 if t is later.
int TTime::CompareTo( const TTime  &t ) const
{
  long diff = long(Sec - t.Sec);
  return diff==0 ? 0 : diff > 0 ? 1 : -1;
}

//
/// Hash function:
/// Returns seconds since January 1, 1901.
//
unsigned TTime::Hash() const
{
  return (unsigned)Sec;
}

//
/// Returns the hour in local time.
//
HourTy TTime::Hour() const
{
  return HourTy((LocalSecs() % SECONDS_IN_DAY) / SECONDS_IN_HOUR);
}

//
/// Returns the hour in Greenwich mean time.
//
HourTy TTime::HourGMT() const
{
  return HourTy((Sec % SECONDS_IN_DAY) / SECONDS_IN_HOUR);
}

//
/// Returns 1 if the time is in daylight saving time; 0 otherwise.
//
int TTime::IsDST() const
{
  if( !_daylight )
    return 0;

  DayTy daycount = (unsigned)(Sec/SECONDS_IN_DAY);
  YearTy year = TDate( (DayTy)daycount, (YearTy)0 ).Year();

  // Check to see if the time falls between the starting & stopping DST times.
  //
  return *this >= BeginDST( year ) && *this < EndDST( year );
}

/// Returns either this TTime object or t, whichever is greater.
TTime TTime::Max( const TTime  & t ) const
{
  if( *this > t )
    return *this;
  else
    return t;
}

/// Returns either this TTime object or t, whichever is less.
TTime TTime::Min( const TTime  & t ) const
{
  if( *this < t )
    return *this;
  else
    return t;
}

//
/// Returns the minute in local time.
//
MinuteTy TTime::Minute() const
{
  return MinuteTy(((LocalSecs()%SECONDS_IN_DAY)%SECONDS_IN_HOUR)/SECONDS_IN_MIN);
}

//
/// Returns the minute in Greenwich Mean Time.
//
MinuteTy TTime::MinuteGMT() const
{
  return MinuteTy(((Sec%SECONDS_IN_DAY)%SECONDS_IN_HOUR)/SECONDS_IN_MIN);
}

//
/// Returns seconds.
//
SecondTy TTime::Second() const
{
  return SecondTy(((Sec%SECONDS_IN_DAY)%SECONDS_IN_HOUR)%SECONDS_IN_MIN);
}

} // OWL namespace
//==============================================================================

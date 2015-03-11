//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#if !defined(OWL_TIMEGADG_H)
# include <owl/timegadg.h>
#endif
#include <owl/time.h>
#include <owl/pointer.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructor for TTimeGadget.
//
TTimeGadget::TTimeGadget(TGetTimeFunc timeFunc, int id, TBorderStyle border,
                         TAlign align, uint numChars, LPCTSTR text,
                         TFont* font)
:
  TTextGadget(id, border, align, numChars, text, font),
  TimeFunc(timeFunc)
{
  SetShrinkWrap(false, true);
}

//
/// String-aware overload
//
TTimeGadget::TTimeGadget(
  TGetTimeFunc timeFunc, 
  int id, 
  TBorderStyle border,
  TAlign align, 
  uint numChars, 
  const tstring& text,
  TFont* font
  )
  : TTextGadget(id, border, align, numChars, text, font),
  TimeFunc(timeFunc)
{
  SetShrinkWrap(false, true);
}

//
/// Overriden from TGadget to inform gadget window to setup a timer
//
void
TTimeGadget::Created()
{
  TGadget::Created();
  GetGadgetWindow()->EnableTimer();
}

//
/// Overridden from TGadget to display the current time.
//
bool
TTimeGadget::IdleAction(long count)
{
  TGadget::IdleAction(count);

  tstring newTime;
  TimeFunc(newTime);
  SetText(newTime.c_str());

  // NOTE: Don't return true to drain system. Let GadgetWindow Timer
  //       message indirectly trigger IdleAction.
  //
  return false;
}

//
/// Retrieves the current time.
//
void
TTimeGadget::GetTTime(tstring& newTime)
{
  TTime time;
  newTime = time.AsString();
}

//
// Win32 specific
//

//
/// Retrieves the system time using the Win32 API.
//
void
TTimeGadget::GetSystemTime(tstring& newTime)
{
  TAPointer<tchar> dateBuffer(new tchar[100]);
  TAPointer<tchar> timeBuffer(new tchar[100]);
  LCID lcid = ::GetUserDefaultLCID();
  SYSTEMTIME systemTime;
  ::GetSystemTime(&systemTime);

  if (::GetTimeFormat(lcid, LOCALE_NOUSEROVERRIDE, &systemTime, 0, timeBuffer, 100)) {
    if (::GetDateFormat(lcid, LOCALE_NOUSEROVERRIDE, &systemTime, 0, dateBuffer, 100)) {
      newTime += dateBuffer;
      newTime += _T(" ");
      newTime += timeBuffer;
    }
  }
}

//
/// Retrieves the local time using the Win32 API
//
void
TTimeGadget::GetLocalTime(tstring& newTime)
{
  TAPointer<tchar> dateBuffer(new tchar[100]);
  TAPointer<tchar> timeBuffer(new tchar[100]);
  LCID lcid = ::GetUserDefaultLCID();
  SYSTEMTIME systemTime;
  ::GetLocalTime(&systemTime);

  if (::GetTimeFormat(lcid, LOCALE_NOUSEROVERRIDE, &systemTime, 0, timeBuffer, 100)) {
    if (::GetDateFormat(lcid, LOCALE_NOUSEROVERRIDE, &systemTime, 0, dateBuffer, 100)) {
      newTime += dateBuffer;
      newTime += _T(" ");
      newTime += timeBuffer;
    }
  }
}


} // OWL namespace
/* ========================================================================== */



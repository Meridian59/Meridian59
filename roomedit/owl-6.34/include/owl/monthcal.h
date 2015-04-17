//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TMonthCalendar.
//----------------------------------------------------------------------------

#if !defined(OWL_MONTHCAL_H)
#define OWL_MONTHCAL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{

//
/// \struct TMonthCalendarData
//
struct TMonthCalendarData
{
  TMonthCalendarData() {}
  TMonthCalendarData(const TSystemTime& cursel) 
    : MultiSel(false), Date1(cursel) {}
  TMonthCalendarData(const TSystemTime& minsel, const TSystemTime& maxsel) 
    : MultiSel(true), Date1(minsel), Date2(maxsel) {}

  bool MultiSel;
  TSystemTime Date1;
  TSystemTime Date2;
};

//
/// \class TMonthCalendar
//
class _OWLCLASS TMonthCalendar 
  : public TControl 
{
  public:

    enum TColors
    {
      mcBackColor=MCSC_BACKGROUND,
      mcTextColor=MCSC_TEXT,
      mcTitleBackColor=MCSC_TITLEBK,
      mcTitleTextColor=MCSC_TITLETEXT,
      mcMonthBackColor=MCSC_MONTHBK,
      mcTrailingTextColor=MCSC_TRAILINGTEXT,
    };

    enum TDaysOfWeek
    {
      dwMonday = 0,
      dwTuesday,
      dwWednesday,
      dwThursday,
      dwFriday,
      dwSaturday,
      dwSunday,
    };

    TMonthCalendar(TWindow* parent, int id, int x, int y, int w, int h, TModule* = 0);
    TMonthCalendar(TWindow* parent, int resourceId, TModule* = 0);
    TMonthCalendar(THandle hWnd, TModule* module = 0);
    ~TMonthCalendar();

    void SetColor(TColors index, const TColor& clr);
    TColor GetColor(TColors index) const;

    bool SetCurSel(const TSystemTime& tm);
    bool GetCurSel(TSystemTime& tm) const;

    bool SetFirstDayOfWeek(TDaysOfWeek day);
    TDaysOfWeek GetFirstDayOfWeek(bool* local = 0) const;

    bool SetMaxSelCount(int count);
    int GetMaxSelCount() const;

    int GetMaxTodayWidth() const;

    bool GetMinReqRect(TRect& rc) const;

    int SetMonthDelta(int delta);
    int GetMonthDelta() const;

    int GetMonthRange(TSystemTime& minm, TSystemTime& maxm, uint32 flags=GMR_DAYSTATE) const;

    uint32 GetRange(TSystemTime& minm, TSystemTime& maxm) const;
    bool SetRange(const TSystemTime& minm, const TSystemTime& maxm);
    bool SetMinDate(const TSystemTime& minm);
    bool SetMaxDate(const TSystemTime& minm);

    bool GetSelRange(TSystemTime& minm, TSystemTime& maxm) const;
    bool SetSelRange(const TSystemTime& minm, const TSystemTime& maxm);

    bool GetToday(TSystemTime& today) const;
    void SetToday(const TSystemTime& today);

    bool IsUnicode() const;
    void SetUnicode(bool unicode = true);

    uint32 HitTest(TMCHitTestInfo& hitinfo);

    bool SetDayState(int months, LPMONTHDAYSTATE states);

    /// Safe overload
    //
    uint Transfer(TMonthCalendarData& data, TTransferDirection direction) {return Transfer(&data, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    uint Transfer(void* buffer, TTransferDirection direction);
    virtual TGetClassNameReturnType GetClassName();

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TMonthCalendar(const TMonthCalendar&);
    TMonthCalendar& operator =(const TMonthCalendar&);
};

/// @}

#include <owl/posclass.h>

//
// inlines
//

//
inline void TMonthCalendar::SetColor(TColors index, const TColor& clr){
  PRECONDITION(GetHandle());
  SendMessage(MCM_SETCOLOR, TParam1(index), TParam2(clr.GetValue()));
}

//
inline TColor TMonthCalendar::GetColor(TColors index) const{
  PRECONDITION(GetHandle());
   return static_cast<COLORREF>(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETCOLOR,TParam1(index),0));
}

//
inline bool TMonthCalendar::SetCurSel(const TSystemTime& tm){
  PRECONDITION(GetHandle());
  PRECONDITION(!(GetStyle() & MCS_MULTISELECT));
   return ToBool(SendMessage(MCM_SETCURSEL,0,TParam2(&tm)));
}

//
inline bool TMonthCalendar::GetCurSel(TSystemTime& tm) const{
  PRECONDITION(GetHandle());
  PRECONDITION(!(GetStyle() & MCS_MULTISELECT));
   return ToBool(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETCURSEL,0,TParam2(&tm)));
}

//
inline bool TMonthCalendar::SetFirstDayOfWeek(TDaysOfWeek day){
  PRECONDITION(GetHandle());
   return ToBool(HiUint16(SendMessage(MCM_SETFIRSTDAYOFWEEK,0,TParam2((int)day))));
}

//
inline TMonthCalendar::TDaysOfWeek TMonthCalendar::GetFirstDayOfWeek(bool* local) const
{
  PRECONDITION(GetHandle());
  TResult retval = CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETFIRSTDAYOFWEEK,0,0);
  if (local)
    *local = HiUint16(retval);
   return TDaysOfWeek(LoUint16(retval));
}

//
inline int TMonthCalendar::GetMaxSelCount() const{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETMAXSELCOUNT, 0, 0);
}

//
inline bool TMonthCalendar::SetMaxSelCount(int count){
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(MCM_SETMAXSELCOUNT, TParam1(count), 0));
}

//
inline int TMonthCalendar::GetMaxTodayWidth() const{
  PRECONDITION(GetHandle());
  return static_cast<int>(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETMAXTODAYWIDTH, 0, 0));
}

//
inline bool TMonthCalendar::GetMinReqRect(TRect& rc) const{
  PRECONDITION(GetHandle());
  return ToBool(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETMINREQRECT, 0, TParam2(&rc)));
}

//
inline int TMonthCalendar::SetMonthDelta(int delta){
  PRECONDITION(GetHandle());
  return (int)SendMessage(MCM_SETMONTHDELTA, TParam1(delta), 0);
}

//
inline int TMonthCalendar::GetMonthDelta() const{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETMONTHDELTA, 0, 0);
}

//
inline bool TMonthCalendar::GetToday(TSystemTime& today) const{
  PRECONDITION(GetHandle());
  return ToBool(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETTODAY, 0, TParam2(&today)));
}

//
inline void TMonthCalendar::SetToday(const TSystemTime& today){
  PRECONDITION(GetHandle());
  SendMessage(MCM_SETTODAY, 0, TParam2(&today));
}

//
inline bool TMonthCalendar::IsUnicode() const{
  PRECONDITION(GetHandle());
  return ToBool(CONST_CAST(TMonthCalendar*,this)->SendMessage(MCM_GETUNICODEFORMAT, 0, 0));
}

//
inline void TMonthCalendar::SetUnicode(bool unicode){
  PRECONDITION(GetHandle());
  SendMessage(MCM_SETUNICODEFORMAT, TParam1((BOOL)unicode), 0);
}

//
inline uint32 TMonthCalendar::HitTest(TMCHitTestInfo& hitinfo){
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(MCM_HITTEST, 0, TParam2(&hitinfo));
}

//
inline bool TMonthCalendar::SetDayState(int months, LPMONTHDAYSTATE states){
  return ToBool(SendMessage(MCM_SETDAYSTATE, TParam1(months), TParam2(states)));
}

} // OWL namespace

#endif  // OWL_MONTHCAL_H

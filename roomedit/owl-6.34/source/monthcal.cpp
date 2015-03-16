//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TMonthCalendar class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/monthcal.h>


namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

//
//
//
TMonthCalendar::TMonthCalendar(TWindow* parent, int id, int x, int y, int w,
                               int h, TModule* module)
:
  TControl(parent,id,_T(""),x,y,w,h,module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}


//
//
//
TMonthCalendar::TMonthCalendar(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}

//
/// Constructs a month calendar object to encapsulate (alias) an existing control.
//
TMonthCalendar::TMonthCalendar(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}

//
//
//
TMonthCalendar::~TMonthCalendar()
{
}


// Return the proper class name.
// Windows class: MONTHCAL_CLASS is defined in commctrl.h
TWindow::TGetClassNameReturnType
TMonthCalendar::GetClassName()
{
  return MONTHCAL_CLASS;
}

//
//
//
int
TMonthCalendar::GetMonthRange(TSystemTime& minm, TSystemTime& maxm, uint32 flags) const
{
  PRECONDITION(GetHandle());

  SYSTEMTIME sysTimes[2];
  memset(sysTimes, 0, sizeof(sysTimes));

  int count = (int)((TMonthCalendar*)this)->SendMessage(MCM_GETMONTHRANGE,
                                    TParam1(flags), TParam2(sysTimes));

  minm = sysTimes[0];
  maxm = sysTimes[1];
  return count;
}

//
//
//
uint32
TMonthCalendar::GetRange(TSystemTime& minm, TSystemTime& maxm) const
{
  PRECONDITION(GetHandle());

  SYSTEMTIME sysTimes[2];
  memset(sysTimes, 0, sizeof(sysTimes));

  uint32 ranges = (uint32)((TMonthCalendar*)this)->SendMessage(MCM_GETRANGE, 0, TParam2(sysTimes));
  minm  = TSystemTime(0,0,0);
  maxm  = TSystemTime(0,0,0);

  if (ranges & GDTR_MIN)
    minm = sysTimes[0];
  if (ranges & GDTR_MAX)
    maxm = sysTimes[1];

  return ranges;
}

//
//
//
bool
TMonthCalendar::SetRange(const TSystemTime& minm, const TSystemTime& maxm)
{
  PRECONDITION(GetHandle());
  SYSTEMTIME sysTimes[2];
  sysTimes[0] = minm;
  sysTimes[1] = maxm;
  return ToBool(SendMessage(MCM_SETRANGE, GDTR_MAX|GDTR_MIN, TParam2(sysTimes)));
}

//
//
//
bool
TMonthCalendar::SetMinDate(const TSystemTime& minm)
{
  PRECONDITION(GetHandle());
  SYSTEMTIME sysTimes[2];
  sysTimes[0] = minm;
  return ToBool(SendMessage(MCM_SETRANGE, GDTR_MIN, TParam2(sysTimes)));
}

//
//
//
bool
TMonthCalendar::SetMaxDate(const TSystemTime& maxm)
{
  PRECONDITION(GetHandle());
  SYSTEMTIME sysTimes[2];
  sysTimes[1] = maxm;
  return ToBool(SendMessage(MCM_SETRANGE, GDTR_MAX, TParam2(sysTimes)));
}

//
//
//
bool
TMonthCalendar::GetSelRange(TSystemTime& minm, TSystemTime& maxm) const
{
  PRECONDITION(GetHandle());
  PRECONDITION(GetStyle() & MCS_MULTISELECT);

  SYSTEMTIME sysTimes[2];
  bool retval = ToBool(((TMonthCalendar*)this)->SendMessage(MCM_GETSELRANGE, 0, TParam2(sysTimes)));
  if (retval){
    minm = sysTimes[0];
    maxm = sysTimes[1];
  }
  return retval;
}

bool
TMonthCalendar::SetSelRange(const TSystemTime& minm, const TSystemTime& maxm)
{
  PRECONDITION(GetHandle());
  PRECONDITION(GetStyle() & MCS_MULTISELECT);

  SYSTEMTIME sysTimes[2];
  sysTimes[0] = minm;
  sysTimes[1] = maxm;

  return ToBool(SendMessage(MCM_SETSELRANGE,0, TParam2(sysTimes)));
}

//
//
//
uint
TMonthCalendar::Transfer(void* buffer, TTransferDirection direction)
{
   TMonthCalendarData* mcData = (TMonthCalendarData*)buffer;
  if (direction == tdGetData) {
    if(mcData->MultiSel)
      GetSelRange(mcData->Date1, mcData->Date2);
    else
      GetCurSel(mcData->Date1);
   }
  else if (direction == tdSetData) {
    if(mcData->MultiSel)
      SetSelRange(mcData->Date1, mcData->Date2);
    else
      SetCurSel(mcData->Date1);
   }
  return sizeof(TMonthCalendarData);
}
} // OWL namespace
/* ========================================================================== */

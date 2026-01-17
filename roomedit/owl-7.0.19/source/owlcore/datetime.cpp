//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TDateTimePicker class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <stdio.h>
#ifndef _MAX_PATH
//JJH
#  if !defined(WINELIB)
#  include <dos.h>
#  endif // WINELIB
#endif

#include <owl/imagelst.h>
#include <stdio.h>

#include <owl/datetime.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // Common Controls diagnostic group

//
// class TDateTimePicker
// ~~~~~ ~~~~~~~~~~~~~~~
//
TDateTimePicker::TDateTimePicker(TWindow* parent, int id,int x, int y, int w,
                                 int h, TModule* module)
:
  TControl(parent, id, _T(""), x, y, w, h, module)
{
  InitializeCommonControls(ICC_DATE_CLASSES);
}

//
//
//
TDateTimePicker::TDateTimePicker(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  InitializeCommonControls(ICC_DATE_CLASSES);
}

//
/// Constructs a date time picker object to encapsulate (alias) an existing control.
//
TDateTimePicker::TDateTimePicker(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  InitializeCommonControls(ICC_DATE_CLASSES);
}

//
//
//
TDateTimePicker::~TDateTimePicker()
{
}

// Return the proper class name.
// Windows class: DATETIMEPICK_CLASS is defined in commctrl.h
auto TDateTimePicker::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{DATETIMEPICK_CLASS};
}

//
bool
TDateTimePicker::SetRange(const TSystemTime& mint, const TSystemTime& maxt)
{
  SYSTEMTIME st[2];
  TParam1 param = 0;

  if(mint != TSystemTime(0,0,0)){
    st[0] = mint;
    param |= GDTR_MIN;
  }

  if(maxt != TSystemTime(0,0,0)){
    st[1] = maxt;
    param |= GDTR_MAX;
  }
  return ToBool(SendMessage(DTM_SETRANGE, param, TParam2(&st)));
}

//
void
TDateTimePicker::GetRange(TSystemTime& mint, TSystemTime& maxt) const
{
  PRECONDITION(GetHandle());
  SYSTEMTIME st[2];
  uint flags = static_cast<uint>(::SendMessage(GetHandle(), DTM_GETRANGE, 0, TParam2(&st)));

  if(flags & GDTR_MIN)
    mint = st[0];
  else
    mint = TSystemTime(0,0,0);

  if(flags & GDTR_MAX)
    maxt = st[1];
  else
    maxt = TSystemTime(0,0,0);
}

//
//
//
uint
TDateTimePicker::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer && direction != tdSizeData) return 0;
  TDateTimePickerData* dtData = reinterpret_cast<TDateTimePickerData*>(buffer);
  if (direction == tdGetData) {
    GetTime(dtData->DateTime);
    SetRange(dtData->MinDate, dtData->MaxDate);
   }
  else if (direction == tdSetData) {
     SetTime(dtData->DateTime);
    SetRange(dtData->MinDate, dtData->MaxDate);
   }
  return sizeof(TDateTimePickerData);
}


} // OWL namespace
/* ========================================================================== */

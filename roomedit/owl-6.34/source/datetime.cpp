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
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

//
// Public global function to format a file date+time string
//
_OWLFUNC(int)
FormatFileTime(FILETIME* pft, void * dest, int textlen)
{
  tchar buf[40];
  int len;

  if (!textlen) {
    *(FILETIME *)dest = *pft;
    return sizeof(FILETIME);
  }
  SYSTEMTIME dt;
  FileTimeToSystemTime(pft, &dt);
  len = _stprintf(buf,_T("%d/%d/%d %02d:%02d:%02d.%02d"),
                dt.wMonth, dt.wDay, dt.wYear,
                dt.wHour, dt.wMinute, dt.wSecond, dt.wMilliseconds/10);
  if (textlen > len)
    textlen = len;
  memcpy(dest, buf, textlen);
  *((LPTSTR)dest + textlen) = 0;
  return len;
}


//
// class TDateTimePicker
// ~~~~~ ~~~~~~~~~~~~~~~
//
TDateTimePicker::TDateTimePicker(TWindow* parent, int id,int x, int y, int w,
                                 int h, TModule* module)
:
  TControl(parent, id, _T(""), x, y, w, h, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}

//
//
//
TDateTimePicker::TDateTimePicker(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}

//
/// Constructs a date time picker object to encapsulate (alias) an existing control.
//
TDateTimePicker::TDateTimePicker(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_DATE_CLASSES);
}

//
//
//
TDateTimePicker::~TDateTimePicker()
{
}

// Return the proper class name.
// Windows class: DATETIMEPICK_CLASS is defined in commctrl.h
TWindow::TGetClassNameReturnType
TDateTimePicker::GetClassName()
{
  return DATETIMEPICK_CLASS;
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
  uint flags = (uint)::SendMessage(GetHandle(), DTM_GETRANGE, 0, TParam2(&st));

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
   TDateTimePickerData* dtData = (TDateTimePickerData*)buffer;
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

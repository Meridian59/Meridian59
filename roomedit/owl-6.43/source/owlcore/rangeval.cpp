//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TRangeValidator, integer numeric range input validator
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/framewin.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

// Let the compiler know that the following template instances will be defined elsewhere.
//#pragma option -Jgx


//
/// Constructs a range validator object by first calling the constructor inherited
/// from TFilterValidator, passing a set of characters containing the digits
/// '0'..'9' and the characters '+' and '-'. Sets Min to minValue and Max to maxValue,
/// establishing the range of acceptable long integer values.
//
TRangeValidator::TRangeValidator(long minValue, long maxValue)
:
  TFilterValidator(_T("0-9+-"))
{
  if (minValue >= 0)
    ValidChars -= _T('-');
  Min = minValue;
  Max = maxValue;
}

//
/// Overrides TValidator's virtual function and displays a message box indicating
/// that the entered value does not fall within the specified range.
//
void
TRangeValidator::Error(TWindow* owner)
{
  PRECONDITION(owner);
  owner->FormatMessageBox(IDS_VALNOTINRANGE, 0, MB_ICONEXCLAMATION|MB_OK, Min, Max);
}

//
/// Converts the string str into an integer number and returns true if the result
/// meets all three of these conditions:
/// - It is a valid integer number.
/// - Its value is greater than or equal to Min.
/// - Its value is less than or equal to Max.
/// If any of those tests fails, IsValid returns false.
//
bool
TRangeValidator::IsValid(LPCTSTR s)
{
  if (TFilterValidator::IsValid(s)) {
    long value = _ttol(s);
    if (value >= Min && value <= Max)
      return true;
  }
  return false;
}

//
/// Incorporates the three types, tdSizeData, tdGetData, and tdSetData, that a range
/// validator can handle for its associated edit control. The parameter str is the
/// edit control's string value, and buffer is the data passed to the edit control.
/// Depending on the value of direction, Transfer either sets str from the number in
/// buffer or sets the number at buffer to the value of the string str. If direction
/// is tdSetData, Transfer sets str from buffer. If direction is tdGetData, Transfer
/// sets buffer from str. If direction is tdSizeData, Transfer neither sets nor
/// reads data.
/// Transfer always returns the size of the data transferred.
//
uint
TRangeValidator::Transfer(tchar * s, void* buffer, TTransferDirection direction)
{
  if (Options & voTransfer) {
    if (direction == tdGetData) {
      *(long*)buffer = _ttol(s);
    }
    else if (direction == tdSetData) {
      wsprintf(s, _T("%ld"), *(long*)buffer);  // need wsprintf for char *
    }
    return sizeof(long);
  }
  else
    return 0;
}

//
/// Adjusts the 'value' of the text, given a cursor position and an amount. Returns
/// the actual amount adjusted.
//
int
TRangeValidator::Adjust(tstring& text, int& /*begPos*/, int& /*endPos*/, int amount)
{
  long value = _ttol(text.c_str());
  long newValue = value + amount;
  if (newValue < Min)
    newValue = Min;
  else if (newValue > Max)
    newValue = Max;

  tchar buffer[15];
  _stprintf(buffer, _T("%ld"), newValue);
  text = buffer;

  return int(newValue - value);
}



IMPLEMENT_STREAMABLE1(TRangeValidator, TFilterValidator);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TRangeValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TFilterValidator*)GetObject(), is);
  is >> GetObject()->Min >> GetObject()->Max;
  return GetObject();
}

//
//
//
void
TRangeValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TFilterValidator*)GetObject(), os);
  os << GetObject()->Min << GetObject()->Max;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


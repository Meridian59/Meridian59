//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TRangeValidator, integer numeric range input validator
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/validate.rh>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/framewin.h>
#include <algorithm>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a range validator object by first calling the constructor inherited
/// from TFilterValidator, passing a set of characters containing the digits
/// '0'..'9' and the characters '+' and '-'. Sets Range establishing the range of
/// acceptable long integer values.
//
TRangeValidator::TRangeValidator(const TRange& range)
:
  TFilterValidator(_T("0-9+-"))
{
  SetRange(range);
}

//
/// Overrides TValidator's virtual function and displays a message box indicating
/// that the entered value does not fall within the specified range.
//
void
TRangeValidator::Error(TWindow* owner)
{
  PRECONDITION(owner);
  owner->FormatMessageBox(owner->LoadString(IDS_VALNOTINRANGE), owner->LoadString(IDS_VALCAPTION), MB_ICONERROR | MB_OK, Range.Min, Range.Max);
}

//
// Hide the internal helper functions so they are excluded from public view by
// using an unnamed namespace.
//
namespace
{

  //
  // Parses the given stream, expecting it to contain only a single valid integer value.
  // On success, returns the converted integer value. On failure, sets the `failbit` state of the
  // given stream and returns 0, unless exceptions are enabled for the stream, in which case an
  // exception will be thrown (see std::ios_base::exceptions).
  //
  auto ParseInteger_(tistream& is) -> TRangeValidator::TExtent
  {
    auto value = TRangeValidator::TExtent{};
    is >> value;

    // Allow trailing whitespace, but no other trailing content.
    //
    if (!is.fail() && !is.eof())
    {
      is >> std::ws;
      if (!is.eof())
      {
        value = 0;
        is.setstate(is.failbit);
      }
    }
    return value;
  }

}

namespace
{

  //
  // Ensures that proper magnitude signs are allowed/disallowed based on the range.
  //
  void ValidateMagnitudeFilter_(const TRangeValidator::TRange& range, TCharSet& validChars)
  {
    auto minus = _T('-');
    auto plus = _T('+');
    auto hasMinus = validChars.Has(minus);
    auto hasPlus = validChars.Has(plus);

    // Add or remove minus sign as needed based on range.Min value.
    //
    if ((range.Min < 0) != hasMinus)
    {
      if (hasMinus)
        validChars -= minus;
      else
        validChars += minus;
    }

    // Add or remove plus sign as needed based on range.Max value.
    //
    if ((range.Max > 0) != hasPlus)
    {
      if (hasPlus)
        validChars -= plus;
      else
        validChars += plus;
    }
  }

  //
  // Ensures that `range.Min` is less than `range.Max`; if not, swaps the values.
  // Calls ValidateMagnitudeFilter_ to ensure necessary magnitude signs are
  // available characters.
  //
  void ValidateRange_(TRangeValidator::TRange& range, TCharSet& validChars)
  {
    WARN(range.Min > range.Max, _T("TRangeValidator has a minimum value of ") << range.Min
      << _T(" that is greater than the maximum value of ") << range.Max
      << _T("; swapped values will be used."));
    std::sort(&range.Min, &range.Max + 1);
    ValidateMagnitudeFilter_(range, validChars);
  }

} // namespace

//
/// Sets the minimum number the validator can accept.
//
/// We can't validate the range here, because of the asynchronous manner in
/// which SetMin and SetMax could be called might have Range.Min greater
/// than Range.Max.
//
void TRangeValidator::SetMin(TExtent minValue)
{
  Range.Min = minValue;
  ValidateMagnitudeFilter_(Range, ValidChars);
}

//
/// Sets the maximum number the validator can accept.
//
/// We can't validate the range here, because of the asynchronous manner in
/// which SetMin and SetMax could be called might have Range.Min greater
/// than Range.Max.
//
void TRangeValidator::SetMax(TExtent maxValue)
{
  Range.Max = maxValue;
  ValidateMagnitudeFilter_(Range, ValidChars);
}

//
/// Sets the minimum and maximum number the validator can accept.
//
void TRangeValidator::SetRange(const TRange& range)
{
  Range = range;
  ValidateRange_(Range, ValidChars);
}

//
/// We first validate the range, because of the asynchronous manner in which SetMin
/// and SetMax could be called might have Min greater than Max.
//
/// If the given string is empty or blank (all white-space), returns true.
/// Otherwise, converts the string str into an integer number and returns true if the result
/// meets all three of these conditions:
/// - It is a valid integer number.
/// - Its value is greater than or equal to Min.
/// - Its value is less than or equal to Max.
/// If any of those tests fails, IsValid returns false.
//
bool
TRangeValidator::IsValid(LPCTSTR s)
{
  PRECONDITION(s);
  if (!TFilterValidator::IsValid(s))
    return false;

  ValidateRange_(Range, ValidChars);
  auto is = tistringstream{s};
  is >> std::ws;
  if (is.eof())
    return true; // Empty/blank input is allowed.

  const auto value = ParseInteger_(is);
  return !is.fail() && value >= Range.Min && value <= Range.Max;
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
    if (!buffer && direction != tdSizeData) return 0;
    if (direction == tdGetData) {
      auto is = tistringstream{s};
      const auto value = ParseInteger_(is);
      WARN(is.fail(), _T("ParseInteger_ failed"));
      *(TExtent*)buffer = value;
    }
    else if (direction == tdSetData) {
      wsprintf(s, _T("%ld"), *(TExtent*)buffer);  // need wsprintf for char *
    }
    return sizeof(TExtent);
  }
  else
    return 0;
}

//
/// We first validate the range, because of the asynchronous manner in which SetMin
/// and SetMax could be called might have Min greater than Max.
//
/// Adjusts the 'value' of the text, given a cursor position and an amount. Returns
/// the actual amount adjusted.
//
int
TRangeValidator::Adjust(tstring& text, int& /*begPos*/, int& /*endPos*/, int amount)
{
  auto clamp = [](TExtent e, const TRange& r)
  { return e < r.Min ? r.Min : e > r.Max ? r.Max : e; };

  ValidateRange_(Range, ValidChars);
  auto is = tistringstream{text};
  const auto value = ParseInteger_(is);
  WARN(is.fail(), _T("ParseInteger_ failed"));
  const auto newValue = clamp(value + amount, Range);
  text = to_tstring(newValue);
  return newValue - value;
}

IMPLEMENT_STREAMABLE1(TRangeValidator, TFilterValidator);

#if OWL_PERSISTENT_STREAMS

//
/// Reads an instance of TRangeValidator from the given ipstream.
//
void*
TRangeValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TFilterValidator*)GetObject(), is);
  is >> GetObject()->Range.Min >> GetObject()->Range.Max;
  return GetObject();
}

//
/// Writes the TRangeValidator to the given opstream.
//
void
TRangeValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TFilterValidator*)GetObject(), os);
  os << GetObject()->Range.Min << GetObject()->Range.Max;
}

#endif

} // OWL namespace

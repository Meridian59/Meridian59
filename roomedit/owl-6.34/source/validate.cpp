//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TValidator, user input validator abstract base class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/module.h>
#include <stdlib.h>
#include <ctype.h>

namespace owl {

OWL_DIAGINFO;


//
/// Constructs an abstract validator object and sets Options fields to 0.
//
TValidator::TValidator()
{
  Options = 0;
}

//
/// Destroys an abstract validator object.
//
TValidator::~TValidator()
{
}

//
/// Error is an abstract function called by Valid when it detects that the user has
/// entered invalid information. By default, TValidator::Error does nothing, but
/// derived classes can override Error to provide feedback to the user.
//
void
TValidator::Error(TWindow* /*owner*/)
{
}

//
/// Checks current input against validator. May adjust input if suppressFill
/// isn't set & validator has the voFill option set.
//
/// If an edit control has an associated validator object, it calls IsValidInput
/// after processing each keyboard event, thus giving validators such as filter
/// validators an opportunity to catch errors before the user fills the entire item
/// or screen.
/// By default, IsValidInput returns true. Derived data validators can override
/// IsValidInput to validate data as the user types it, returning true if str holds
/// valid data and false otherwise.
/// str is the current input string. suppressFill determines whether the validator
/// should automatically format the string before validating it. If suppressFill is
/// true, validation takes place on the unmodified string str. If suppressFill is
/// false, the validator should apply any filling or padding before validating data.
/// Of the standard validator objects, only TPXPictureValidator checks suppressFill.
/// IsValidInput can modify the contents of the input string; for example, it can
/// force characters to uppercase or insert literal characters from a format
/// picture. IsValidInput should not, however, delete invalid characters from the
/// string. By returning false, IsValidInput indicates that the edit control should
/// erase the incorrect characters.
//
bool
TValidator::IsValidInput(tchar *, bool /*suppressFill*/)
{
  return true;
}

//
/// Checks input against validator for completeness. Never modifies input.
//
/// By default, returns true. Derived validator classes can override IsValid to
/// validate data for a completed edit control.
/// If an edit control has an associated validator object, and the edit control's
/// IsValid method is called with its reportErr parameter set to true, then the
/// validator object's Valid method, which in turn calls IsValid, is called to
/// determine whether the contents of the edit control are valid. If the edit
/// control's IsValid method is called with the reportErr parameter set to false,
/// then the validator's IsValid method is called directly.
//
bool
TValidator::IsValid(LPCTSTR)
{
  return true;
}

//
/// Allows a validator to set and read the values of its associated edit control.
/// This is primarily useful for validators that check non-string data, such as
/// numeric values. For example, TRangeValidator uses Transfer to read and write
/// values instead of transferring an entire string.
/// By default, edit controls with validators give the validator the first chance to
/// respond to DataSize, GetData, and SetData by calling the validator's Transfer
/// method. If Transfer returns anything other than 0, it indicates to the edit
/// control that it has handled the appropriate transfer. The default action of
/// TValidator::Transfer is always to return 0. If you want the validator to
/// transfer data, you must override its Transfer method.
/// Transfer's first two parameters are the associated edit control's text string
/// and the tdGetData or tdSetData data record. Depending on the value of direction,
/// Transfer can set str from buffer or read the data from str into buffer. The
/// return value is always the number of bytes transferred.
/// If direction is tdSizeData, Transfer doesn't change either str or buffer; it
/// just returns the data size. If direction is tdSetData, Transfer reads the
/// appropriate number of bytes from buffer, converts them into the proper string
/// form, and sets them into str, returning the number of bytes read. If direction
/// is tdGetData, Transfer converts str into the appropriate data type and writes
/// the value into buffer, returning the number of bytes written.
//
uint
TValidator::Transfer(tchar *, void*, TTransferDirection)
{
  return 0;
}

//
/// Adjusts the 'value' of the text, given a cursor position and an amount.  Returns
/// the actual amount adjusted.
//
int
TValidator::Adjust(tstring& /*text*/, int& /*begPos*/, int& /*endPos*/, int /*amount*/)
{
  return 0;
}

//----------------------------------------------------------------------------

//
/// Constructs a TXValidator object, setting the resource ID to IDS_VALIDATORSYNTAX
/// string resource.
//
TXValidator::TXValidator(uint resId)
:
  TXOwl(resId)
{
}

//
/// Copies the exception so it can be rethrown at a safer time.
//
TXValidator*
TXValidator::Clone() const
{
  return new TXValidator(*this);
}


//
/// Creates an instance of TXValidator and throws it.
//
void
TXValidator::Throw()
{
  throw *this;
}

//
/// Creates an instance of TXValidator and throws it.
//
void
TXValidator::Raise()
{
  TXValidator().Throw();
}


#if !defined(BI_NO_OBJ_STREAMING)

IMPLEMENT_STREAMABLE(TValidator);

//
//
//
void*
TValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  is >> GetObject()->Options;
  return GetObject();
}

//
//
//
void
TValidator::Streamer::Write(opstream& os) const
{
  os << GetObject()->Options;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPXPictureValidator, Paradox-picture input validator
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
#include <owl/validate.rh>
#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/framewin.h>
#include <ctype.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;


//
/// Constructs a picture validator object by first calling the constructor inherited
/// from TValidator and setting pic to point to it. Then sets the voFill bit in
/// Options if AutoFill is true and sets Options to voOnAppend. Throws a TXValidator
/// exception if the picture is invalid.
//
TPXPictureValidator::TPXPictureValidator(LPCTSTR pic, bool autoFill)
:
  TValidator(),
  Pic(pic)
{
  Init(autoFill);
}

//
/// String-aware overload
//
TPXPictureValidator::TPXPictureValidator(const tstring& pic, bool autoFill)
:
  TValidator(),
  Pic(pic)
{
  Init(autoFill);
}

//
/// Shared initialization
//
void TPXPictureValidator::Init(bool autoFill)
{
  Options = voOnAppend;
  if (autoFill)
    Options |= voFill;
  if (!SyntaxCheck())
    TXValidator::Raise();
}

//
/// Overrides TValidator's virtual function and displays a message box that indicates
/// an error in the picture format and displays the string pointed to by Pic.
//
void
TPXPictureValidator::Error(TWindow* owner)
{
  PRECONDITION(owner);
  owner->FormatMessageBox(owner->LoadString(IDS_VALPXPCONFORM), owner->LoadString(IDS_VALCAPTION), MB_ICONERROR | MB_OK, Pic.c_str());
}

//
/// IsValidInput overrides TValidator's virtual function and checks the string
/// passed in str against the format picture specified in Pic. IsValid returns true
/// if Pic is NULL or Picture does not return Error for str; otherwise, it returns
/// false. The suppressFill parameter overrides the value in voFill for the duration
/// of the call to IsValidInput.
/// If suppressFill is false and voFill is set, the call to Picture returns a filled
/// string based on str, so the image in the edit control automatically reflects the
/// format specified in Pic.
//
bool
TPXPictureValidator::IsValidInput(LPTSTR input, bool suppressFill)
{
  bool autoFill = (Options&voFill) && !suppressFill;
  return Pic.empty()
    || Picture(input, autoFill) != prError;
}

//
/// IsValid overrides TValidator's virtual function and compares the string passed
/// in str with the format picture specified in Pic. IsValid returns true if Pic is
/// NULL or if Picture returns Complete for str, indicating that str needs no
/// further input to meet the specified format; otherwise, it returns false.
//
bool
TPXPictureValidator::IsValid(LPCTSTR input)
{
  if (Pic.empty())
    return true;

  TPicResult rslt = Picture(CONST_CAST(LPTSTR,input), false);
  return rslt == prComplete || rslt == prEmpty;
}

//
/// Adjusts the 'value' of the text, given a cursor position and an amount.  Returns
/// the actual amount adjusted.
//
int
TPXPictureValidator::Adjust(tstring& /*text*/, int& /*begPos*/, int& /*endPos*/, int /*amount*/)
{
  return 0;
}

//
//
//
inline bool
TPXPictureValidator::IsComplete(TPicResult rslt)
{
  return rslt == prComplete || rslt == prAmbiguous;
}

//
//
//
inline bool
TPXPictureValidator::IsIncomplete(TPicResult rslt)
{
  return rslt == prIncomplete || rslt == prIncompNoFill;
}

//
// Skip a character or a picture group
//
void
TPXPictureValidator::ToGroupEnd(uint termCh, uint& i)
{
  int brkLevel = 0;
  int brcLevel = 0;

  do {
    if (i == termCh)
      return;
    switch (Pic[i]) {
      case _T('['): brkLevel++; break;
      case _T(']'): brkLevel--; break;
      case _T('{'): brcLevel++; break;
      case _T('}'): brcLevel--; break;
      case _T(';'): i++; break;
      case _T('*'):
        i++;
        while (_istdigit((tchar)Pic[i]))
          i++;
        ToGroupEnd(termCh, i);
        continue;
    }
    i += CharSize((LPCTSTR)Pic.c_str() + i) / sizeof(tchar);
  } while (brkLevel || brcLevel);
}

//
/// Find the next comma separator
//
bool
TPXPictureValidator::SkipToComma(uint termCh, uint& i)
{
  for (;;) {
    ToGroupEnd(termCh, i);
    if (i == termCh)
      return false;
    if (Pic[i] == _T(',')) {
      i++;
      return i < termCh;
    }
  }
}

//
/// Calculate the end of a group (does not modify i)
//
uint
TPXPictureValidator::CalcTerm(uint termCh, uint i)
{
  ToGroupEnd(termCh, i);
  return i;
}

//
/// The next group is repeated X times
//
TPicResult
TPXPictureValidator::Iteration(LPTSTR input, uint termCh, uint& i, uint& j)
{
  TPicResult rslt;
  uint newTermCh;

  i++;  // Skip '*'

  // Retrieve number

  uint itr = 0;
  for (; _istdigit((tchar)Pic[i]); i++)
    itr = itr * 10 + Pic[i] - _T('0');

  if (i >= termCh)
    return prSyntax;

  newTermCh = CalcTerm(termCh, i);

  //
  // if itr is 0 allow any number, otherwise enforce the number
  //
  uint k = i;
  if (itr) {
    for (uint m = 0; m < itr; m++) {
      i = k;
      rslt = Process(input, newTermCh, i, j);
      if (!IsComplete(rslt)) {
        if (rslt == prEmpty)  // Empty means incomplete since all are required
          rslt = prIncomplete;
        return rslt;
      }
    }
  }
  else {
    do {
      i = k;
      rslt = Process(input, newTermCh, i, j);
    } while (IsComplete(rslt));
    if (rslt == prEmpty || rslt == prError) {
      i++;
      rslt = prAmbiguous;
    }
  }
  i = newTermCh;
  return rslt;
}

//
/// Process a picture group
//
TPicResult
TPXPictureValidator::Group(LPTSTR input, uint termCh, uint& i, uint& j)
{
  uint groupTermCh = CalcTerm(termCh, i);
  i++;
  TPicResult rslt = Process(input, groupTermCh - 1, i, j);
  if (!IsIncomplete(rslt))
    i = groupTermCh;
  return rslt;
}

//
//
//
TPicResult
TPXPictureValidator::CheckComplete(uint termCh, uint& i, TPicResult rslt)
{
  uint j = i;
  if (IsIncomplete(rslt)) {
    // Skip optional pieces
    for (;;) {
      if (Pic[j] == _T('['))
        ToGroupEnd(termCh, j);

      else if (Pic[j] == _T('*')) {
        if (!isdigit((tchar)Pic[j+1])) {
          j++;
          ToGroupEnd(termCh, j);
        }
        else
          break;
      }
      else
        break;

      if (j == termCh)
        return prAmbiguous;  // end of the string, don't know if complete
    }
  }
  return rslt;
}

//
// Check for UNICODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
#if !defined(__GNUC__)
#pragma warn -aus
#endif

TPicResult
TPXPictureValidator::Scan(LPTSTR input, uint termCh, uint& i, uint& j)
{
  tchar ch;
  TPicResult rslt = prEmpty;

  uint len = static_cast<uint>(::_tcslen(input));
  while (i != termCh && Pic[i] != _T(',')) {
    if (j >= len)
      return CheckComplete(termCh, i, rslt);

    ch = input[j];
    switch (Pic[i]) {
      case _T('#'):
        if (!_istdigit((tchar)ch))
          return prError;
        else {
          input[j++] = ch;
          i++;
        }
        break;
      case _T('?'):
        if (!_istalpha((tchar)ch))
          return prError;
        else {
          input[j++] = ch;
          i++;
        }
        break;
      case _T('&'):
        if (!_istalpha((tchar)ch))
          return prError;
        else {
          input[j++] = (tchar)_totupper(ch);
          i++;
        }
        break;
      case _T('!'): {
#if defined(BI_DBCS_SUPPORT)
        uint n = CharSize(&input[j]) / sizeof(tchar);
        if (j + n >= len)
          j = len;
        else{
          if (n == 1)
            input[j++] = (tchar)_totupper((tchar)ch);
          else
            j += n;
        }
#else
        input[j++] = (tchar)_totupper(ch);
#endif
        i++;
        break;
      }
      case _T('@'): {
#if defined(BI_DBCS_SUPPORT)
        uint n = CharSize(&input[j]) / sizeof(tchar);
        if (j + n >= len)
          j = len;
        else
          j += n;
#else
        input[j++] = ch;
#endif
        i++;
        break;
      }
      case _T('*'):
        rslt = Iteration(input, termCh, i, j);
        if (!IsComplete(rslt))
          return rslt;
        if (rslt == prError)
          rslt = prAmbiguous;
        break;
      case _T('{'):
        rslt = Group(input, termCh, i, j);
        if (!IsComplete(rslt))
          return rslt;
        break;
      case _T('['):
        rslt = Group(input, termCh, i, j);
        if (IsIncomplete(rslt))
          return rslt;
        if (rslt == prError)
          rslt = prAmbiguous;
        break;
      default: {
#if defined(BI_DBCS_SUPPORT)
        if (Pic[i] == _T(';'))
          i++;
        uint n  = CharSize(&input[j]) / sizeof(tchar);
        uint n2 = CharSize((LPCTSTR)Pic.c_str() + i) / sizeof(tchar);
        if (j + n >= len)
          n = len - j;
        if (n == 1) {
          if (ch == _T(' ')) {
#if defined(BI_AUTO_COMPLETION_DBCS_BY_SPACE)
// But, couldn't expand input buffer TValidator classes.
//
            if (n < n2) {
              memmove(input+n2, input+n, len-n+1);
              len += n2 - n;
              n = n2;
            }
            while (n-- > 0)
              input[j++] = Pic[i++];
#else
            if (n != n2)
              return prError;
            input[j++] = Pic[i++];
#endif
          }
          else {
            if (n != n2)
              return prError;
            if (_totupper((tchar)Pic[i]) != _totupper((tchar)ch))
              return prError;
            input[j++] = Pic[i++];
          }
        }
        else {
          if (n > n2)
            return prError;
          for (uint i1 = 0; i1 < n; i1++)
            if (input[j+i1] != Pic[i+i1])
              return prError;
          while (n-- > 0)
            input[j++] = Pic[i++];
        }
#else
        if (Pic[i] == _T(';'))
          i++;
        if (_totupper(Pic[i]) != _totupper(ch))
        {
          if (ch == _T(' '))
            ch = Pic[i];
          else
            return prError;
        }
        input[j++] = Pic[i];
        i++;
#endif
      }
    }
    if (rslt == prAmbiguous)
      rslt = prIncompNoFill;
    else
      rslt = prIncomplete;
  }

  return (rslt == prIncompNoFill) ? prAmbiguous : prComplete;
}
#if !defined(__GNUC__)
#pragma warn .aus
#endif

//
//
//
TPicResult
TPXPictureValidator::Process(LPTSTR input, uint termCh, uint& i, uint& j)
{
  TPicResult rslt;
  uint incompJ, incompI;
  incompJ = incompI = 0;

  bool incomp = false;
  uint oldI = i;
  uint oldJ = j;
  do {
    rslt = Scan(input, termCh, i, j);

    //
    // Only accept completes if they make it farther in the input
    // stream from the last incomplete
    //
    if ((rslt==prComplete || rslt==prAmbiguous) && incomp && j < incompJ) {
      rslt = prIncomplete;
      j = incompJ;
    }

    if (rslt == prError || rslt == prIncomplete) {
      if (!incomp && rslt == prIncomplete) {
        incomp = true;
        incompI = i;
        incompJ = j;
      }
      i = oldI;
      j = oldJ;
      if (!SkipToComma(termCh, i)) {
        if (incomp) {
          i = incompI;
          j = incompJ;
          return prIncomplete;
        }
        return rslt;
      }
      oldI = i;
    }
  } while (rslt == prError || rslt == prIncomplete);

  return (rslt == prComplete && incomp) ? prAmbiguous : rslt;
}

//
//
//
bool
TPXPictureValidator::SyntaxCheck()
{
  const auto n = static_cast<int>(Pic.length());
  if (Pic.empty() ||
    (Pic.back() == _T(';')) ||
    (Pic.back() == _T('*') && Pic[n - 2] != _T(';')))
  {
    return false;
  }

  auto brkLevel = 0;
  auto brcLevel = 0;
  auto next = [&](int& i) { i += CharSize(&Pic[i]) / sizeof(tchar); };
  for (auto i = 0; i < n; next(i))
  {
    switch (Pic[i])
    {
      case _T('['): ++brkLevel; break;
      case _T(']'): --brkLevel; break;
      case _T('{'): ++brcLevel; break;
      case _T('}'): --brcLevel; break;
      case _T(';'): next(i); break;
      case _T('*'):
        next(i);
        if (!_istdigit(Pic[i]))
          return false;
        break;
    }
  }
  return brkLevel == 0 && brcLevel == 0;
}

//
/// Checks the validity of the input according to the format specified by the
/// picture string, possibly adding fill characters to the end of the input.
//
/// Formats the string passed in input according to the format specified by the
/// picture string pointed to by Pic. Picture returns prError if there is an error
/// in the picture string or if input contains data that cannot fit the specified
/// picture. Returns prComplete if input can fully satisfy the specified picture.
/// Returns prIncomplete if input contains data that incompletely fits the specified
/// picture.
//
/// The following characters are used in creating format pictures:
///
/// Special
/// - \b #      Accept only a digit
/// - \b ?      Accept only a letter (case_insensitive)
/// - \b &      Accept only a letter, force to uppercase
/// - \b @      Accept any character
/// - \b !      Accept any character, force to uppercase
/// Match
/// - \b ;      Take next character literally
/// - \b *      Repetition count
/// - \b []     Option
/// - \b {}     Grouping operators
/// - \b ,      Set of alternatives
/// - All others        Taken literally
//
TPicResult
TPXPictureValidator::Picture(LPTSTR input, bool autoFill)
{
  // Check if any input data.
  //
  if (!input || !*input)
    return prEmpty;

  // Indexes for Pic[] and input[].
  //
  uint iPic = 0;
  uint iInput = 0;

  // Process the given input to check for an error.
  //
  TPicResult rslt = Process(input, static_cast<uint>(Pic.length()), iPic, iInput);

  if ((rslt != prError) && (rslt != prSyntax) && (iInput < ::_tcslen(input)))
    rslt = prError;

  // If the result is incomplete and autofill is requested, then copy literal
  // characters from the picture over to the input.
  //
  if ((rslt == prIncomplete) && autoFill) {
    bool reprocess = false;
    while (iPic < Pic.length() && !_tcschr(_T("#?&!@*{}[],"), Pic[iPic]))
    {
      if (Pic[iPic] == _T(';'))
        iPic++;
#if defined(BI_DBCS_SUPPORT)
      uint k = static_cast<uint>(::_tcslen(input));
      uint n = CharSize((LPCTSTR)Pic.c_str() + iPic) / sizeof(tchar);
      memmove(input + k, Pic.c_str() + iPic, n);
      input[k + n] = _T('\0');
      iPic += n;
      iInput += n;
#else
      input[iInput++] = Pic[iPic++];
#endif
      reprocess = true;
    }
    if (reprocess)
    {
      input[iInput] = _T('\0'); // Terminate the copy, since we are probably appending
      iInput = iPic = 0;
      rslt = Process(input, static_cast<uint>(Pic.length()), iPic, iInput);
    }
  }

  // We perform an additional check here because SyntaxCheck may not catch them
  // all with it using different logic.
  //
  if (rslt == prSyntax)
    TXValidator::Raise();

  return (rslt == prAmbiguous) ? prComplete : (rslt == prIncompNoFill) ? prIncomplete : rslt;

}


IMPLEMENT_STREAMABLE1(TPXPictureValidator, TValidator);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TPXPictureValidator::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TValidator*)GetObject(), is);
  is >> GetObject()->Pic;
  return GetObject();
}

//
//
//
void
TPXPictureValidator::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TValidator*)GetObject(), os);
  os << GetObject()->Pic;
}

#endif

} // OWL namespace

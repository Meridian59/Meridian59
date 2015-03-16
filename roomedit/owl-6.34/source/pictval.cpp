//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPXPictureValidator, Paradox-picture input validator
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/validate.h>
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
  if (Picture(0, false) != prEmpty)
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
  owner->FormatMessageBox(IDS_VALPXPCONFORM, 0, MB_ICONEXCLAMATION|MB_OK, (LPCTSTR)Pic.c_str());
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

  uint len = ::_tcslen(input);
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
#if defined(BI_PDOXWINJ_SUPPORT)
        // Paradox for Windows/J database program has two special picture to
        // support Japanese characters in CodePage 932
        //
        // '“' 0x81+0x93 - (2 byte '%' symbol)
        //      1 byte KATAKANA and KATAKANA symbols (0xA1 - 0xDF)
        // '—' 0x81+0x97 - (2 byte '@' symbol)
        //      any 2 byte characters except 2 byte space (0x81+0x40)
        //
        // This is hard coded, because we don't know how to get current
        // code page in Windows 3.1
        //
        uint n  = CharSize(&input[j]) / sizeof(tchar);
        uint n2 = CharSize(((const char *))Pic.c_str() + i) / sizeof(tchar);
        if (n2 == 2) {
          utchar uc1, uc2;
          uc1 = (utchar)Pic[i];
          uc2 = (utchar)Pic[i+1];
          if (uc1 == 0x81 && uc2 == 0x93) {
            if ((utchar)ch >= 0xA1 && (utchar)ch <= 0xDF){
              i += n2;
              j += n;
              break;
            }
            else
              return prError;
          } 
          else if (uc1 == 0x81 && uc2 == 0x97){
            if (n == 2 && j + n < len &&
                ((utchar)ch != 0x81 || (utchar)input[j+1] != 0x40)) {
              i += n2;
              j += n;
              break;
            } else
              return prError;
          }
        }
        if (n2 == 1 && Pic[i] == ';'){
          i++;
          n2 = CharSize((const char *)Pic.c_str() + i) / sizeof(tchar);
        }
#else
        if (Pic[i] == _T(';'))
          i++;
        uint n  = CharSize(&input[j]) / sizeof(tchar);
        uint n2 = CharSize((LPCTSTR)Pic.c_str() + i) / sizeof(tchar);
#endif
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
          if (ch == _T(' '))
            ch = Pic[i];
          else
            return prError;
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
  if (Pic.empty())
    return false;
  if (Pic[Pic.length()-1] == _T(';'))
    return false;
  if (Pic[Pic.length()-1] == _T('*') && Pic[Pic.length()-2] != _T(';'))
    return false;

  int brkLevel = 0;
  int brcLevel = 0;
  for (uint i = 0; i < Pic.length(); ) {
    switch (Pic[i]) {
      case _T('['): brkLevel++; break;
      case _T(']'): brkLevel--; break;
      case _T('{'): brcLevel++; break;
      case _T('}'): brcLevel--; break;
      case _T(';'): i++;
    }
    i += CharSize((LPCTSTR)Pic.c_str() + i) / sizeof(tchar);
  }
  return !(brkLevel || brcLevel);
}


#if defined(BI_DBCS_SUPPORT)
#if defined(BI_PDOXWINJ_SUPPORT)
static LPSTR lstrchrp(LPCSTR s1, LPCSTR s2)
{
  uint n2 = CharSize(s2) / sizeof(tchar);
  do  {
    uint n1 = CharSize(s1) / sizeof(tchar);
    if (n1 == n2 && memcmp(s1, s2, n1) == 0)
      return (char *)s1;
    for ( ; n1-- > 0; s1++)
      if( !*s1 )
        break;
  } while (*s1) ;
  return (char *)NULL;
}
#endif
#endif

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
/// The following characters are used in creating format pictures:
/// 
/// Special	
/// - \b #	Accept only a digit
/// - \b ?	Accept only a letter (case_insensitive)
/// - \b &	Accept only a letter, force to uppercase
/// - \b @	Accept any character
/// - \b !	Accept any character, force to uppercase
/// Match
/// - \b ;	Take next character literally
/// - \b *	Repetition count
/// - \b []	Option
/// - \b {}	Grouping operators
/// - \b ,	Set of alternatives
/// - All others	Taken literally
//
TPicResult
TPXPictureValidator::Picture(LPTSTR input, bool autoFill)
{
  if (!SyntaxCheck())
    return prSyntax;
  if (!input || !*input)
    return prEmpty;

  uint j = 0;  // index for input[]
  uint i = 0;  // index for Pic[]

  TPicResult rslt = Process(input, Pic.length(), i, j);
  if (rslt != prError && rslt != prSyntax && j < ::_tcslen(input))
    rslt = prError;

  // If the result is incomplete & autofill is requested, then copy literal
  // characters from the picture over to the input.
  //
  if (rslt == prIncomplete && autoFill) {
    bool  reprocess = false;
#if defined(BI_PDOXWINJ_SUPPORT)
    // "“—"
    while (i < Pic.length() && !lstrchrp("#?&!@*{}[],\x81\x93\x81\x97", (LPCSTR)Pic.c_str()+i)) {
#else
    while (i < Pic.length() && !_tcschr(_T("#?&!@*{}[],"), Pic[i])) {
#endif
      if (Pic[i] == _T(';'))
        i++;
#if defined(BI_DBCS_SUPPORT)
      uint k = ::_tcslen(input);
      uint n = CharSize((LPCTSTR)Pic.c_str() + i) / sizeof(tchar);
      memmove(input + k, Pic.c_str() + i, n);
      input[k + n] = _T('\0');
      i += n;
      j += n;
#else
      input[j++] = Pic[i++];
#endif
      reprocess = true;
    }
    if (reprocess) {
      input[j] = 0;   // terminate the copy, since we are probably appending
      j = i = 0;
      rslt = Process(input, Pic.length(), i, j);
    }
  }

  return (rslt == prAmbiguous) ? prComplete
                               : (rslt == prIncompNoFill) ? prIncomplete : rslt;
}


IMPLEMENT_STREAMABLE1(TPXPictureValidator, TValidator);

#if !defined(BI_NO_OBJ_STREAMING)

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

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


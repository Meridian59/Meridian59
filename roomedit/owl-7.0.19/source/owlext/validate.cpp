//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//
//Original code by David Foringer
//
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/validate.h>
#include <stdio.h>

using namespace owl;

namespace OwlExt {

//ctor:
TDoubleValidator::TDoubleValidator(double minValue, double maxValue)
:
TFilterValidator(_T("0-9+-."))
{
  if (minValue >= 0.)
    ValidChars -= _T('-');
  Min = minValue;
  Max = maxValue;
}


//called on error:
void TDoubleValidator::Error(TWindow* owner)
{
  owner->FormatMessageBox(_T("Value not in range %.3f , %.3f"), _T(""), MB_ICONEXCLAMATION|MB_OK, Min, Max);
}


//called on focus lost, str is * to edit line string:
bool TDoubleValidator::IsValid(LPCTSTR str)
{
  //see IsValidInput for description !
  if (!TFilterValidator::IsValid(str))
    return false;
  LPCTSTR t = _tcschr(str,_T('-'));
  if(t!=NULL){
    if(t!=str)
      return false;
    if(_tcschr(t+1,_T('-')))
      return false;
  }
  if((t=_tcschr(str,_T('+')))!=NULL){
    if(t!=str)
      return false;
    if(_tcschr(t+1,_T('+')))
      return false;
  }
  if((t=_tcschr(str,_T('.')))!=NULL){
    if(_tcschr(t+1,_T('.')))
      return false;
  }
  double value = _ttof(str);
  if (value >= Min && value <= Max)
    return true;
  return false;
}

//called on each entry, str is * to edit line string:
bool
TDoubleValidator::IsValidInput(LPTSTR str, bool /*suppressFill*/)
{
  for (LPCTSTR p = str; *p; ) {
    uint n = CharSize(p) / sizeof(TCHAR);
    if (n > 1 || !ValidChars.Has((utchar)*p))
      return false;
    p += n;
  }
  LPCTSTR t;
  //find '-'
  if((t=_tcschr(str,_T('-')))!=NULL){
    //if found, must be first char:
    if(t!=str)
      return false;
    //if found, disallow additional occurences:
    if(_tcschr(t+1,_T('-')))
      return false;
  }
  //find '+'
  if((t=_tcschr(str,_T('+')))!=NULL){
    //if found, must be first char:
    if(t!=str)
      return false;
    //if found, disallow additional occurences:
    if(_tcschr(t+1,_T('+')))
      return false;
  }
  //even if set so in locale, don't allow comma for decimal point
  if((t=_tcschr(str,_T(',')))!=NULL)
    return false;

  //find decimal point
  if((t=_tcschr(str,_T('.')))!=NULL){
    //if found, disallow additional occurences:
    if(_tcschr(t+1,_T('.')))
      return false;
  }
  //if here, succeded
  return true;
}

//
//
//
uint
TDoubleValidator::Transfer(LPTSTR  s, void* buffer, TTransferDirection direction)
{
  if (Options & voTransfer)
  {
    if (!buffer && direction != tdSizeData) return 0;
    if (direction == tdGetData)
      *(double *)buffer = _ttof(s);
    else if (direction == tdSetData)
      _stprintf(s, _T("%.3f"), *(double *)buffer);  // need wsprintf for char *
    return sizeof(double);
  }
  else
    return 0;
}

//
// Adjust the 'value' of the text, given a cursor position & an amount
// Return the actual amount adjusted.
//
int
TDoubleValidator::Adjust(owl::tstring& text, int& /*begPos*/, int& /*endPos*/, int amount)
{
  double value = _ttof(text.c_str());
  double newValue = value + (double)amount;
  if (newValue < Min)
    newValue = Min;
  else if (newValue > Max)
    newValue = Max;

  _TCHAR buffer[15];
  _stprintf(buffer, _T("%.3f"), newValue);
  text = buffer;

  return int(newValue - value);
}


} // OwlExt namespace
//==============================================================================

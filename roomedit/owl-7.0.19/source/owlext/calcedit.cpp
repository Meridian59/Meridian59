//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// CALCEDIT.CPP
//
// TCalcEdit Class
//
// Original code by Steve Carr (Compuserve: 100251,1571)
// Written: 13th November 1995
//
//------------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/validate.h>
#include <owl/dialog.h>

#include <owlext/calcedit.h>
#include <owlext/calcedit.rh>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace OwlExt {

using namespace owl;
using namespace std;

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);


class TCalcValidator : public TFilterValidator {
protected:
  double minValue;
  double maxValue;
  TCalcEdit* owner;

public:
  TCalcValidator( double aMin, double aMax, TCalcEdit* anOwner ) :
    TFilterValidator(_T("0-9+-."))
    {
      minValue = aMin;
      maxValue = aMax;
      owner = anOwner;
    }

    virtual void Error(TWindow*p=NULL) ;

    virtual bool IsValid( LPCTSTR s) ;

    void SetMin( double aMin )
    {
      minValue = aMin ;
      if( maxValue < minValue )
        maxValue = minValue ;
    }
    void SetMax( double aMax )
    {
      maxValue = aMax ;
      if( minValue > maxValue )
        minValue = maxValue ;
    }
    double GetMin() const { return minValue ; }
    double GetMax() const { return maxValue ; }
};


void TCalcValidator::Error(TWindow*)
{
  owner->FormatMessageBox(_T("Value must be between %.*f and %.*f"), _T(""), MB_ICONEXCLAMATION|MB_OK|MB_TASKMODAL,
    owner->GetDecimals(), minValue, owner->GetDecimals(), maxValue);
}


bool TCalcValidator::IsValid( LPCTSTR s )
{
  if(TFilterValidator::IsValid(s)){
    double val = _ttof(s) ;
    if(minValue == maxValue)
      return true;
    if( val >= minValue && val <= maxValue )
      return true;
  };
  return false;
}


#define CALC_DIGITS 15

class TCalc : public TDialog{
public:
  auto Execute() -> int override;
  void Destroy(int retValue = IDCANCEL) override;
  void CloseWindow(int retValue = IDCANCEL) override;

public:
  enum TCalcState { CS_FIRST, CS_VALID, CS_ERROR } ; // calculator state
  TCalcState CalcStatus;
  _TCHAR     Num[CALC_DIGITS + 1];
  _TCHAR     Operator;
  bool       Negative;
  double     Operand;
  _TCHAR     LastKey;
  TStatic*    display;

public:
  TCalc( TWindow* aParent);
  void FlashKey(_TCHAR key);
  void Error()
  {
    CalcStatus = CS_ERROR;
    _tcscpy(Num, _T("Error"));
    Negative = FALSE;
  }

  void SetDisplay(double r);
  double GetDisplay() ;
  virtual void  UpdateDisplay();

  void CheckFirst()
  {
    if (CalcStatus == CS_FIRST){
      CalcStatus = CS_VALID;
      _tcscpy(Num, _T("0"));
      Negative = false;
    }
  }
  void InsertKey(_TCHAR key);
  void CalcKey(_TCHAR key);
  void Clear()
  {
    CalcStatus = CS_FIRST;
    _tcscpy(Num, _T("0"));
    Negative = false;
    Operator = _T('=');
    LastKey = _T(' ');
  }

protected:
  void SetupWindow() override;

  //
  // override EvCommand() defined by class TWindow
  //
  auto EvCommand(UINT, HWND, UINT) -> LRESULT override;

  // Message response functions
  //
  HBRUSH        EvCtlColor(HDC, HWND hWndChild, uint ctlType);

  DECLARE_RESPONSE_TABLE(TCalc);
};

DEFINE_RESPONSE_TABLE1(TCalc, TDialog)
  EV_WM_CTLCOLORSTATIC(EvCtlColor),
END_RESPONSE_TABLE;

TCalc::TCalc( TWindow* parent )
:
TDialog(parent, IDD_CALCDIALOG,
    FindResourceModule(parent,0,IDD_CALCDIALOG,RT_DIALOG))
{
  Clear();
  TWindow::Attr.AccelTable = IDA_CALCULATOR;
  display = new TStatic( this, ID_DISPLAY);
}

void TCalc::SetupWindow()
{
  TDialog::SetupWindow();

  TRect rc, rcThis = GetWindowRect(), rcParent = Parent->GetWindowRect();

  rcThis.right = rcParent.left + rcThis.Width();
  rcThis.bottom = rcParent.bottom + rcThis.Height();
  rcThis.left = rcParent.left;
  rcThis.top = rcParent.bottom;

  ::GetWindowRect(::GetDesktopWindow(), &rc);
  if (rcThis.left < rc.left)
    rcThis.Offset(rc.left-rcThis.left, 0);
  if (rcThis.bottom > rc.bottom)
    rcThis.Offset(0, rc.bottom-rcThis.bottom);
  if (rcThis.right > rc.right)
    rcThis.Offset(rc.right-rcThis.right, 0);
  MoveWindow(rcThis);
}

int TCalc::Execute()
{
  if (Create()){
    if (!(TWindow::Attr.Style & WS_VISIBLE))
      ShowWindow(SW_SHOW);
    UpdateDisplay();
    return GetApplication()->BeginModal(this,MB_TASKMODAL);
  }
  return -1;
}

void TCalc::Destroy(int retval)
{
  GetApplication()->EndModal(retval);
  TWindow::Destroy(retval);
}

void TCalc::CloseWindow(int retval)
{
  if (CanClose()){
    TransferData(tdGetData);
    Destroy(retval);
  }
}

LRESULT TCalc::EvCommand(UINT id, HWND hWndCtl, UINT notifyCode)
{
  if (hWndCtl != 0 && notifyCode == BN_CLICKED)
    CalcKey( _TCHAR(id) );
  else if ( hWndCtl == 0 && notifyCode == 1 ){
    FlashKey(_TCHAR(id));
    CalcKey(_TCHAR(id));
  }
  return TDialog::EvCommand(id, hWndCtl, notifyCode) ;
}

//
// Colorize the calculator. Allows background to show through corners of
// buttons, uses yellow text on black background in the display, and sets
// the dialog background to blue.
//
HBRUSH
TCalc::EvCtlColor(HDC hDC, HWND /*hWndChild*/, uint ctlType)
{
  PRECONDITION(ctlType == CTLCOLOR_STATIC); InUse(ctlType);
  ::SetTextColor(hDC, TColor::LtYellow);
  SetBkColor(hDC, TColor::Black);
  return reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
}

void
TCalc::UpdateDisplay()
{
  _TCHAR  str[CALC_DIGITS + 2] ;

  if (Negative)
    _tcscpy(str, _T("-"));
  else
    str[0] = _T('\0');
  _tcscat( str, Num ) ;

  display->SetText( str ) ;
}

void TCalc::SetDisplay(double r)
{
  _TCHAR* first;
  _TCHAR* last;
  int      charsToCopy;
  _TCHAR   str[64];

  r = (floor(r * 10000000L + .5)) / 10000000L;

  _stprintf(str, _T("%0.10f"), r);
  first = str;
  Negative = FALSE;

  if(str[0] == '-'){
    first++;
    Negative = true;
  }

  if (_tcslen(first) > CALC_DIGITS + 1 + 10 )
    Error();
  else{
    last = _tcschr(first, 0);

    while (last[-1] == _T('0'))
      --last;

    if (last[-1] == _T('.'))
      --last;

    charsToCopy = std::min(CALC_DIGITS + 1, int(last - first));
    _tcsncpy(Num, first, charsToCopy);
    Num[charsToCopy] = 0;
  }
}


void TCalc::FlashKey(_TCHAR key)
{
  if (key == _T('\n'))
    key = _T('=') ;

  HWND button = GetDlgItem( toupper(key) ) ;
  if (button){
    ::SendMessage(button, BM_SETSTATE, 1, 0) ;
    Sleep(100);
    ::SendMessage(button, BM_SETSTATE, 0, 0) ;
  }
}

double
TCalc::GetDisplay()
{
  double r = _ttof(Num);

  if (Negative)
    r = -r;

  return r ;
}

void
TCalc::InsertKey(_TCHAR key)
{
  int l = static_cast<int>(_tcslen(Num));

  if (l < CALC_DIGITS){
    Num[l++] = key;
    Num[l] = _T('\0');
  }
}

void
TCalc::CalcKey(_TCHAR key)
{
  key = (_TCHAR)toupper(key);

  if (CalcStatus == CS_ERROR && key != _T('C'))
    key = _T(' ');

  if (key >= _T('0') && key <= _T('9')){
    CheckFirst();

    if (!_tcscmp(Num, _T("0")))
      Num[0] = _T('\0');

    InsertKey(key);
  }
  else if (key == _T('+') || key == _T('-') || key == _T('*') ||
    key == _T('/') || key == _T('=') || key == _T('%') || key == 0x0D)
  {
    if( ( key == 0x0D || key == _T('=') ) && key == LastKey )
      CloseWindow( IDOK ) ;

    if (CalcStatus == CS_VALID){
      CalcStatus = CS_FIRST;
      double  r = GetDisplay() ;

      if (key == _T('%')){
        switch(Operator){
case _T('+'):
case _T('-'):
  r = Operand * r / 100;
  break;

case _T('*'):
case _T('/'):
  r /= 100;
  break;
        }
      }

      switch(Operator){
case _T('+'):
  SetDisplay(Operand + r);
  break;

case _T('-'):
  SetDisplay(Operand - r);
  break;

case _T('*'):
  SetDisplay(Operand * r);
  break;

case _T('/'):
  if (r == 0)
    Error();
  else
    SetDisplay(Operand / r);
  break;
      }
    }

    Operator = key;
    Operand = GetDisplay();

  }
  else{
    switch(key){
case _T('.'):
  CheckFirst();
  if (!_tcschr(Num, _T('.')))
    InsertKey(key);
  break;

case 0x8:
  CheckFirst();
  if (_tcslen(Num) == 1)
    _tcscpy(Num, _T("0"));
  else
    Num[_tcslen(Num) - 1] = _T('\0');
  break;

case _T('_'):
  Negative = !Negative;
  break;

case _T('C'):
  Clear();
  break;
    }
  }
  LastKey = key ;

  UpdateDisplay();
}


TCalcEdit::TCalcEdit(TWindow* parent, int resourceId,
           uint aWidth, uint aDecimals, double aMin, double aMax,
           TModule* module)
           :
TPopupEdit(parent, resourceId, IDB_CALCBTN, aWidth+1, module)
{
  //  SetText("0"); //?????????????????
  SetCaption(_T("0"));
  aCalc = NULL;
  decimals = aDecimals ;
  Validator = new TCalcValidator( aMin, aMax, this ) ;
}

TCalcEdit::TCalcEdit(TWindow* parent, int id, double aVal, int x, int y,
           int w, int h, uint aWidth, uint aDecimals, double aMin, double aMax,
           TModule* module )
           :
TPopupEdit( parent, id, _T(""),  x, y, w, h, IDB_CALCBTN, aWidth+1, module )
{
  aCalc = NULL ;
  decimals = aDecimals ;
  Validator = new TCalcValidator( aMin, aMax, this ) ;
  operator =( aVal ) ;
}


void TCalcEdit::Clicked()
{
  TCalc* pCalc = new TCalc(this) ;
  ((TWindow*)pCalc)->GetWindowAttr().AccelTable = IDA_CALCULATOR;

  GetText( pCalc->Num, CALC_DIGITS );
  double r = _ttof( pCalc->Num );
  pCalc->CalcStatus = TCalc::CS_VALID;
  pCalc->SetDisplay( r );

  if( pCalc->Execute() == IDOK )
    operator =(pCalc->GetDisplay());

  delete pCalc;
}

void TCalcEdit::SetMin( double d )
{
  TCalcValidator *cv = (TCalcValidator*)Validator ;
  if( cv )
    cv->SetMin( d ) ;
}

void TCalcEdit::SetMax( double d )
{
  TCalcValidator *cv = (TCalcValidator*)Validator ;
  if( cv )
    cv->SetMax( d ) ;
}

void TCalcEdit::operator = ( double d )
{
  _TCHAR buf[64];
  _stprintf( buf, _T("%.*f"), decimals, d );
  SetText(buf);
}

TCalcEdit::operator double()
{
  _TCHAR buf[64] ;
  GetText( buf, sizeof(buf) ) ;
  return _ttof(buf) ;
}

} // OwlExt namespace
//==============================================================================

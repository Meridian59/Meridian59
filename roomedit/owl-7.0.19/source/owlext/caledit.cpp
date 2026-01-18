//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// CALEDIT.CPP
//
// TCalendarEdit Class
// TDateValidator Class
// TPopupCalWin Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
// Written: 19th November 1995
//
//------------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/appdict.h>

#include <owl/celarray.h>
#include <owl/gadgetwi.h>

#include <owlext/caledit.h>
#include <owlext/caledit.rh>

#include <algorithm>

namespace OwlExt {

using namespace owl;


// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

class TPopupCalWin : public TWindow {
public:

  TFont*   Font;
  int     FontWidth;
  int     FontHeight;

  TDate   Date;

  int MonthRow;
  int WeekDaysRow;
  int DaysRow;
  int StartCol;

  bool   Dragging;
  TRect CurRect;

  TCelArray* BmpArray;
  TRect BtnPrevRect;
  TRect BtnNextRect;

  TCalendarEdit* CalEdit;

  enum {Prev, Next};

  TPopupCalWin(TWindow* parent, TCalendarEdit* edit, TModule* module = 0);
  ~TPopupCalWin();

  void SetupWindow() override;

  void BuildBmpArray();
  void DrawFocusRect(TDC& dc, int x, int y);
  void Paint(TDC&, bool erase, TRect&) override;
  void PaintBorders(TDC& dc, TRect& rect);

  void EvKeyDown(uint key, uint repeatCount, uint flags);
  void EvKillFocus(HWND hwndGetFocus);
  void EvLButtonDown(uint modKeys, const TPoint& point);
  void EvLButtonUp(uint modKeys, const TPoint& point);
  void EvMouseMove(uint modKeys, const TPoint& point);

  DECLARE_RESPONSE_TABLE(TPopupCalWin);
};


//
//-- TPopupCalWin --------------------------------------------------------------
//

DEFINE_RESPONSE_TABLE1(TPopupCalWin, TWindow)
EV_WM_KEYDOWN,
EV_WM_KILLFOCUS,
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONUP,
EV_WM_MOUSEMOVE,
END_RESPONSE_TABLE;

TPopupCalWin::TPopupCalWin(TWindow* parent, TCalendarEdit* edit, TModule* module)
:
TWindow(parent, 0, module)
{
  ModifyStyle(0,WS_POPUP|WS_BORDER|WS_VISIBLE);

  CalEdit = edit;

  Font = new TFont(_T("MS Shell Dlg"), -8);
  {
    TRect rc;
    TScreenDC dc;
    dc.SelectObject(*Font);
    dc.DrawText(_T("Abc"), -1, rc, DT_CENTER | DT_CALCRECT);
    FontWidth = rc.Width();
    FontHeight = rc.Height();
  }

  MonthRow = 0;
  WeekDaysRow = MonthRow + FontHeight;
  DaysRow = WeekDaysRow + FontHeight + 4;
  StartCol = 4;

  Attr.W = (7 * FontWidth) + ( 2 * StartCol);
  Attr.H = DaysRow + (6 * FontHeight) + 4;

  Dragging = false;

  BmpArray = 0;
  BuildBmpArray();

  BtnPrevRect = TRect(2, 2, BmpArray->CelSize().cx + 2, BmpArray->CelSize().cy + 2);

  BtnNextRect.left = Attr.W - BmpArray->CelSize().cx - 4;
  BtnNextRect.top = 2;
  BtnNextRect.right = BtnNextRect.left + BmpArray->CelSize().cx;
  BtnNextRect.bottom = BtnNextRect.top + BmpArray->CelSize().cy;
}

TPopupCalWin::~TPopupCalWin()
{
  delete Font;
  delete BmpArray;
}

void TPopupCalWin::BuildBmpArray()
{
  TBitmap bmp(*FindResourceModule(Parent,GetModule(),IDB_CALPREVNEXT,RT_BITMAP),
    IDB_CALPREVNEXT);
  int bmpWidth = bmp.Width();
  int bmpHeight = bmp.Height() / 2;

  if (!BmpArray){
    TBitmap* tmp = new TBitmap(TScreenDC(), bmpWidth, bmpHeight);
    BmpArray = new TCelArray(tmp, 2);
  }

  TMemoryDC bmpDC;
  {
    TBitmap tmp(TScreenDC(), bmpWidth, bmpHeight);
    bmpDC.SelectObject(tmp);
    bmpDC.TextRect(0, 0, bmpWidth, bmpHeight, TColor::Sys3dFace);
    DrawMaskedBmp(bmpDC, 0, 0, bmp, 0, bmpWidth);
  }

  TMemoryDC memDC;
  memDC.SelectObject(*BmpArray);
  memDC.BitBlt(0, 0, bmpWidth, bmpHeight, bmpDC, 0, 0);
}

//
//
//
void
TPopupCalWin::DrawFocusRect(TDC& dc, int x, int y)
{
  TRect rect = TRect(StartCol + x * FontWidth,
    DaysRow + y * FontHeight,
    StartCol + x * FontWidth + FontWidth,
    DaysRow + y * FontHeight + FontHeight);

  dc.SaveDC();

  if (!CurRect.IsEmpty()){
    dc.SelectObject(TBrush(TColor::Sys3dFace));
    dc.PatBlt(CurRect.left, CurRect.top, CurRect.Width() - 1, 1, PATCOPY);
    dc.PatBlt(CurRect.left, CurRect.top, 1, CurRect.Height() - 1, PATCOPY);
    dc.PatBlt(CurRect.right - 1, CurRect.top, 1, CurRect.Height(), PATCOPY);
    dc.PatBlt(CurRect.left, CurRect.bottom - 1, CurRect.Width(), 1, PATCOPY);
  }

  dc.SelectObject(TBrush(TColor::Sys3dShadow));
  dc.PatBlt(rect.left, rect.top, rect.Width() - 1, 1, PATCOPY);
  dc.PatBlt(rect.left, rect.top, 1, rect.Height() - 1, PATCOPY);
  dc.SelectObject(TBrush(TColor::Sys3dHilight));
  dc.PatBlt(rect.right - 1, rect.top, 1, rect.Height(), PATCOPY);
  dc.PatBlt(rect.left, rect.bottom - 1, rect.Width(), 1, PATCOPY);

  CurRect = rect;

  dc.RestoreDC();
}

void
TPopupCalWin::EvKeyDown(uint key, uint repeatCount, uint flags)
{
  TDate curDate = Date;

  switch (key){
case VK_DOWN:
  Date += 7;
  break;
case VK_END:
  if (GetKeyState(VK_CONTROL) < 0)
    if (Date < TDate(31, 12, Date.Year()))
      Date = TDate(31, 12, Date.Year());
    else
      Date = TDate(31, 12, Date.Year() + 1);
  else
    if ((int)Date.DayOfMonth() < Date.DaysInMonth())
      Date = TDate(Date.DaysInMonth(), Date.Month(), Date.Year());
  break;
case VK_ESCAPE:
  CalEdit->SetFocus();
  CloseWindow();
  return;
case VK_HOME:
  if (GetKeyState(VK_CONTROL) < 0)
    if (Date > TDate(1, 1, Date.Year()))
      Date = TDate(1, 1, Date.Year());
    else
      Date = TDate(1, 1, Date.Year() - 1);
  else
    if (Date.DayOfMonth() > 1)
      Date = TDate(1, Date.Month(), Date.Year());
  break;
case VK_LEFT:
  --Date;
  break;
case VK_MULTIPLY:
  Date = TDate();
  break;
case VK_NEXT:
  if (GetKeyState(VK_CONTROL) < 0){
    if (Date.Month() == 2){
      TDate date(1, 2, Date.Year() + 1);
      if ((int)Date.DayOfMonth() > date.DaysInMonth()){
        Date = TDate(date.DaysInMonth(), 2, date.Year());
        break;
      }
    }
    Date = TDate(Date.DayOfMonth(), Date.Month(), Date.Year() + 1);
  }
  else
  {
    int days = Date.DaysInMonth(Date.Month()+1 > 12 ? 1 : Date.Month()+1);
    if ((int)Date.DayOfMonth() <= days)
      Date += Date.DaysInMonth();
    else
      if (Date.Month() == 1)
        Date = TDate(Date.DaysInMonth(2), 2, Date.Year());
      else
        Date += std::min(days, Date.DaysInMonth());
  }
  break;
case VK_PRIOR:
  if (GetKeyState(VK_CONTROL) < 0){
    if (Date.Month() == 2){
      TDate date(1, 2, Date.Year() - 1);
      if ((int)Date.DayOfMonth() > date.DaysInMonth()){
        Date = TDate(date.DaysInMonth(), 2, date.Year());
        break;
      }
    }
    Date = TDate(Date.DayOfMonth(), Date.Month(), Date.Year() - 1);
  }
  else{
    int days = Date.DaysInMonth(Date.Month()-1 < 1 ? 12 : Date.Month()-1);
    if (days < (int)Date.DayOfMonth())
      Date -= Date.DayOfMonth();
    else
      Date -= days;
  }
  break;
case VK_RETURN:
case VK_SPACE:
  CalEdit->SetDate(Date);
  CalEdit->SetFocus();
  CloseWindow();
  return;
case VK_RIGHT:
  ++Date;
  break;
case VK_UP:
  Date -= 7;
  break;
default:
  TWindow::EvKeyDown(key, repeatCount, flags);
  return;
  };

  if (Date.Month() != curDate.Month() || Date.Year() != curDate.Year())
    Invalidate();
  else{
    TClientDC dc(*this);
    DrawFocusRect(dc, Date.WeekDay() - 1, (Date.DayOfMonth() + Date.WeekDay(1) - 2 ) / 7);
  }
}

void
TPopupCalWin::EvKillFocus(HWND /*hwndGetFocus*/)
{
  CloseWindow();
}

void
TPopupCalWin::EvLButtonDown(uint modKeys, const TPoint& point)
{
  SetCapture();

  if (BtnPrevRect.Contains(point) || BtnNextRect.Contains(point))
    return;

  Dragging = true;
  EvMouseMove(modKeys, point);
}

void
TPopupCalWin::EvLButtonUp(uint /*modKeys*/, const TPoint& point)
{
  ReleaseCapture();

  if (Dragging){
    Dragging = false;

    int x = ((point.x - StartCol) / FontWidth) + 1;
    int y = ((point.y - DaysRow) / FontHeight) + 1;
    TDate date((y*7) - (7-x) - (Date.WeekDay(1)-1), Date.Month(), Date.Year());
    if (!date.IsValid())
      return;

    CalEdit->SetDate(date);
    CalEdit->SetFocus();

    CloseWindow();
  }
  else if (BtnPrevRect.Contains(point)){
    int days = Date.DaysInMonth(Date.Month()-1 < 1 ? 12 : Date.Month()-1);
    if (days < (int)Date.DayOfMonth())
      Date -= Date.DayOfMonth();
    else
      Date -= days;
    Invalidate();
  }
  else if (BtnNextRect.Contains(point)){
    int days = Date.DaysInMonth(Date.Month()+1 > 12 ? 1 : Date.Month()+1);
    if ((int)Date.DayOfMonth() <= days)
      Date += Date.DaysInMonth();
    else
      if (Date.Month() == 1)
        Date = TDate(Date.DaysInMonth(2), 2, Date.Year());
      else
        Date += std::min(days, Date.DaysInMonth());
    Invalidate();
  }
}

void
TPopupCalWin::EvMouseMove(uint modKeys, const TPoint& point)
{
  if (Dragging){
    int x = ((point.x - StartCol) / FontWidth);
    int y = ((point.y - DaysRow) / FontHeight);

    if (TRect(StartCol, DaysRow, StartCol + 7 * FontWidth,
      DaysRow + 6 * FontHeight).Contains(point) &&
      TDate::DayWithinMonth(((y+1)*7) - (7-(x+1)) - (Date.WeekDay(1)-1),
      Date.Month(), Date.Year()))
    {
      TClientDC dc(*this);
      DrawFocusRect(dc, x,y);
    }
  }
  TWindow::EvMouseMove(modKeys, point);
}

void
TPopupCalWin::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
  int i;
  TRect r;
  TDate today;
  int weekday = Date.WeekDay(1);
  TRect clientRect = GetClientRect();
  TFont boldFont(_T("MS Shell Dlg"), -8, 0, 0, 0, FW_BOLD);

  PaintBorders(dc, clientRect);

  dc.SetBkMode(TRANSPARENT);

  TMemoryDC memDC(dc);
  memDC.SelectObject(*BmpArray);
  dc.BitBlt(BtnPrevRect, memDC, BmpArray->CelOffset(Prev));
  dc.BitBlt(BtnNextRect, memDC, BmpArray->CelOffset(Next));

  dc.SetTextColor(TColor::SysBtnText);

  _TCHAR buf[80];
  wsprintf(buf, _T("%s %d"), Date.NameOfMonth(), Date.Year());
  r = TRect(0, MonthRow, clientRect.right, MonthRow + FontHeight);
  dc.SelectObject(boldFont);
  dc.DrawText(buf, -1, r, DT_CENTER | DT_VCENTER);
  dc.SelectObject(*Font);

  r = TRect(StartCol, WeekDaysRow, StartCol + FontWidth, WeekDaysRow + FontHeight);
  for (i = 0; i < 7; i++){
    dc.DrawText(TDate::GetIntlName(TDate::dtWeekDayNames,i),2,r,DT_CENTER|DT_VCENTER);
    r.left += FontWidth;
    r.right += FontWidth;
  }

  dc.SaveDC();
  clientRect.top = WeekDaysRow + FontHeight;
  clientRect.bottom = clientRect.top + 2;
  dc.SelectObject(TBrush(TColor::Sys3dShadow));
  dc.PatBlt(clientRect.left, clientRect.top, clientRect.Width() - 1, 1, PATCOPY);
  dc.SelectObject(TBrush(TColor::Sys3dHilight));
  dc.PatBlt(clientRect.left, clientRect.bottom - 1, clientRect.Width(), 1, PATCOPY);
  dc.RestoreDC();

  r = TRect(StartCol + ((weekday -1) * FontWidth),
    DaysRow,
    StartCol + ((weekday -1) * FontWidth) + FontWidth,
    DaysRow + FontHeight);
  for (i = 1; i <= Date.DaysInMonth(); i++, weekday++){
    _TCHAR buf[3];
    if (today == TDate(i, Date.Month(), Date.Year()))
      dc.SelectObject(boldFont);
    dc.DrawText(_itot(i, buf, 10), -1, r, DT_CENTER | DT_VCENTER);
    if (today == TDate(i, Date.Month(), Date.Year()))
      dc.SelectObject(*Font);
    r.left += FontWidth;
    r.right += FontWidth;
    if (weekday % 7 == 0){
      r.left = StartCol;
      r.right = StartCol + FontWidth;
      r.top += FontHeight;
      r.bottom += FontHeight;
    }
  }

  DrawFocusRect(dc, Date.WeekDay()-1, (Date.DayOfMonth()+Date.WeekDay(1)-2)/7);
}

void TPopupCalWin::PaintBorders(TDC& dc, TRect& rect)
{
  dc.SaveDC();
  dc.SelectObject(TBrush(TColor::Sys3dHilight));
  dc.PatBlt(rect.left, rect.top, rect.Width() - 1, 1, PATCOPY);
  dc.PatBlt(rect.left, rect.top, 1, rect.Height() - 1, PATCOPY);
  dc.SelectObject(TBrush(TColor::Sys3dShadow));
  dc.PatBlt(rect.right - 1, rect.top, 1, rect.Height(), PATCOPY);
  dc.PatBlt(rect.left, rect.bottom - 1, rect.Width(), 1, PATCOPY);
  dc.RestoreDC();
}

void TPopupCalWin::SetupWindow()
{
  TWindow::SetupWindow();

  _TCHAR str[80];
  CalEdit->GetText(str, sizeof(str) / sizeof(_TCHAR));

  Date = TDate(str);
  if (!Date.IsValid())
    Date = TDate();

  TRect rc, rcThis = GetWindowRect();
  ::GetWindowRect(GetDesktopWindow(), &rc);
  if (rcThis.left < rc.left)
    rcThis.Offset(rc.left-rcThis.left, 0);
  if (rcThis.bottom > rc.bottom)
    rcThis.Offset(0, rc.bottom-rcThis.bottom);
  if (rcThis.right > rc.right)
    rcThis.Offset(rc.right-rcThis.right, 0);
  MoveWindow(rcThis);

  BuildBmpArray();
  SetBkgndColor(TColor::Sys3dFace);
}


//
//-- TCalendarEdit -------------------------------------------------------------
//

DEFINE_RESPONSE_TABLE1(TCalendarEdit, TPopupEdit)
EV_WM_CHAR,
EV_WM_ENABLE,
EV_WM_KEYDOWN,
EV_WM_KILLFOCUS,
EV_WM_SIZE,
END_RESPONSE_TABLE;

TCalendarEdit::TCalendarEdit(TWindow* parent, int id, LPCTSTR text,
               int x, int y, int w, int h, uint textLen,
               TModule* module)
               :
TPopupEdit(parent, id, text, x, y, w, h, IDB_CALBTN, textLen, module)
{
  Init();
}

TCalendarEdit::TCalendarEdit(TWindow* parent, int resourceId, uint textLen,
               TModule* module)
               :
TPopupEdit(parent, resourceId, IDB_CALBTN, textLen, module)
{
  Init();
}

TCalendarEdit::~TCalendarEdit()
{
}

void
TCalendarEdit::Clicked()
{
  if (*CalWin)
    return;

  TRect r = GetWindowRect();
  CalWin->GetWindowAttr().X = r.left;
  CalWin->GetWindowAttr().Y = r.bottom;
  CalWin->Create();
}

void TCalendarEdit::EvChar(uint key, uint repeatCount, uint flags)
{
  if (key != '+' && key != '-')
    TPopupEdit::EvChar(key, repeatCount, flags);
}

void TCalendarEdit::EvKeyDown(uint key, uint repeatCount, uint flags)
{
  switch (key){
case VK_ADD:
case VK_SUBTRACT:  {
  _TCHAR str[80];
  GetText(str, sizeof(str) / sizeof(_TCHAR));
  TDate date(str);
  if (!date.IsValid())
    date = TDate();
  key == VK_ADD ? ++date : --date;
  SetDate(date);
  break;
          }
default:
  TPopupEdit::EvKeyDown(key, repeatCount, flags);
  }
}

void TCalendarEdit::EvKillFocus(HWND hWndGetFocus)
{
  _TCHAR str[80];
  GetText(str, sizeof(str) / sizeof(_TCHAR));
  SetDate(str);
  TPopupEdit::EvKillFocus(hWndGetFocus);
}

const TDate& TCalendarEdit::GetDate()
{
  _TCHAR str[80];
  GetText(str, sizeof(str) / sizeof(_TCHAR));
  CalWin->Date = TDate(str);
  return CalWin->Date;
}

void TCalendarEdit::Init()
{
  CalWin = new TPopupCalWin(Parent, this, GetModule());
  CalWin->DisableAutoCreate();
}

void TCalendarEdit::SetDate(const TDate& date)
{
  if (date.IsValid()){
    CalWin->Date = date;

    _TCHAR str[80];
    GetText(str, sizeof(str) / sizeof(_TCHAR));
    if (_tcscmp(str, date.AsString().c_str()))
      TPopupEdit::SetText(date.AsString().c_str());

  }
  else
    Clear();
}

void TCalendarEdit::SetDate(LPCTSTR str)
{
  SetDate(TDate(str));
}


//
//-- TCalendarEditGadget -------------------------------------------------------
//

TCalendarEditGadget::TCalendarEditGadget(TWindow* gadgetWnd, int id, int width,
                     LPCTSTR text)
                     :
TPopupEditGadget(*new TCalendarEdit(gadgetWnd, id, text, 0, 0, width, 0))
{
}


//
//-- TDateValidator ------------------------------------------------------------
//

TDateValidator::TDateValidator()
:
TValidator()
{
}

void TDateValidator::Error(TWindow* wnd)
{
  TApplication* app;

  if(wnd)
    app  = wnd->GetApplication();
  else{
    app  = GetApplicationObject();
    wnd = GetWindowPtr(((TWindow*)app->GetMainWindow())->GetLastActivePopup());
  }
  owl::tstring msg;
  TModule* module = FindResourceModule(wnd,app,IDS_CAL_ERRMSG,RT_STRING);
  if(module)
    msg = module->LoadString(IDS_CAL_ERRMSG);

  if (wnd)
    wnd->MessageBox(msg.c_str(), app->GetName(), MB_ICONEXCLAMATION | MB_OK);
  else
    ::MessageBox(0, msg.c_str(), app->GetName(), MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);
}

bool TDateValidator::IsValid(LPCTSTR str)
{
  if (!*str)
    return true;

  TDate date(str);
  bool isValid = date.IsValid();

  return isValid;
}

} // OwlExt namespace
//==============================================================================

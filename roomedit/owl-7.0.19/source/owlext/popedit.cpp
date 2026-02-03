//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// POPEDIT.CPP
//
// TPopupButton Class
// TPopupEdit Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
// Written: 15th November 1995
//
//------------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/button.h>
#include <owl/celarray.h>

#include <owlext/popedit.h>

using namespace owl;

namespace OwlExt {

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

class TPopupButton : public TButton {
public:
  enum {Up, Down, Disabled};

  TPopupButton(TWindow *parent, TPopupEdit* edit, TResId bmpId, TModule* module);
  ~TPopupButton();

  void      BuildBmpArray();
  auto GetWindowClassName() -> owl::TWindowClassName override;
  void      Layout(int bSizeEditBox = true);
  void SetupWindow() override;

  void      EvClicked();
  void      EvDrawItem(uint ctrlId, const DRAWITEMSTRUCT& drawInfo);
  void      EvMouseMove(uint modKeys, const TPoint& point);
  void      EvSysColorChange();

protected:
  TResId       BmpId;
  TCelArray*   BmpArray;
  TPopupEdit* PopupEdit;

  DECLARE_RESPONSE_TABLE(TPopupButton);
};


//
//-- TPopupButton --------------------------------------------------------------
//

DEFINE_RESPONSE_TABLE1(TPopupButton, TButton)
EV_WM_DRAWITEM,
EV_WM_MOUSEMOVE,
EV_WM_SYSCOLORCHANGE,
EV_NOTIFY_AT_CHILD(BN_CLICKED, EvClicked),
END_RESPONSE_TABLE;

TPopupButton::TPopupButton(TWindow *parent, TPopupEdit* edit, TResId bmpId,
               TModule* module)
               :
TButton(parent, 0, _T(""), 0, 0, 0, 0, false, module)
{
  ModifyStyle(WS_TABSTOP,BS_OWNERDRAW);

  BmpId = bmpId;
  BmpArray = 0;
  PopupEdit = edit;

  BuildBmpArray();

  if (TYPESAFE_DOWNCAST(Parent, TGadgetWindow))
    PopupEdit->GetWindowAttr().H = BmpArray->CelSize().cy;
}

TPopupButton::~TPopupButton()
{
  delete BmpArray;
}

void TPopupButton::BuildBmpArray()
{
  TBitmap bmp(*FindResourceModule(Parent, GetModule(),BmpId,RT_BITMAP), BmpId);
  int bmpWidth = bmp.Width();
  int bmpHeight = bmp.Height() / 2;

  TBrush faceBrush(TColor::Sys3dFace);
  TBrush hiliteBrush(TColor::Sys3dHilight);
  TBrush shadowBrush(TColor::Sys3dShadow);
  TBrush frameBrush(TColor::SysWindowFrame);

  if (!BmpArray){
    int width = (bmpWidth + 4) * 3;
    int height = bmpHeight + 4;
    TBitmap* tmp = new TBitmap(TScreenDC(), width, height);
    BmpArray = new TCelArray(tmp, 3);
  }

  TMemoryDC bmpDC;
  {
    TBitmap tmp(TScreenDC(), bmpWidth, bmpHeight);
    bmpDC.SelectObject(tmp);
    bmpDC.FillRect(0, 0, bmpWidth, bmpHeight, faceBrush);
    DrawMaskedBmp(bmpDC, 0, 0, bmp, 0, bmpWidth);
  }

  TMemoryDC memDC;
  memDC.SelectObject(*BmpArray);

  TRect bmpRect = BmpArray->CelRect(Up);
  memDC.FrameRect( bmpRect, frameBrush ) ;
  bmpRect.Inflate(-1, -1);
  memDC.SelectObject(hiliteBrush);
  memDC.PatBlt(bmpRect.left, 1, bmpRect.Width(), 1, PATCOPY);
  memDC.PatBlt(bmpRect.left, bmpRect.top, 1, bmpRect.Height(), PATCOPY);
  memDC.SelectObject(shadowBrush);
  memDC.PatBlt(bmpRect.right - 1, 1, 1, bmpRect.Height(), PATCOPY);
  memDC.PatBlt(bmpRect.left, bmpRect.Height(), bmpRect.Width(), 1, PATCOPY);
  memDC.BitBlt(BmpArray->CelRect(Up).OffsetBy(2, 2), bmpDC, TPoint(0, 0));

  bmpRect = BmpArray->CelRect(Down);
  memDC.FrameRect( bmpRect, frameBrush ) ;
  bmpRect.Inflate(-1, -1);
  memDC.FillRect(bmpRect, faceBrush);
  memDC.SelectObject(shadowBrush);
  memDC.PatBlt(bmpRect.left, 1, bmpRect.Width(), 1, PATCOPY);
  memDC.PatBlt(bmpRect.left, bmpRect.top, 1, bmpRect.Height(), PATCOPY);
  memDC.BitBlt(BmpArray->CelRect(Down).OffsetBy(3, 3), bmpDC, TPoint(0, 0));

  bmpRect = BmpArray->CelRect(Disabled);
  memDC.FrameRect( bmpRect, frameBrush ) ;
  bmpRect.Inflate(-1, -1);
  memDC.FillRect(bmpRect, faceBrush);
  memDC.BitBlt(BmpArray->CelRect(Disabled).OffsetBy(2, 2), bmpDC, TPoint(0, 0));
  memDC.BitBlt(BmpArray->CelRect(Disabled).OffsetBy(2, 2), bmpDC, TPoint(0, 0), 0x00B8074AL);
  memDC.SelectObject(hiliteBrush);
  memDC.PatBlt(bmpRect.left, 1, bmpRect.Width(), 1, PATCOPY);
  memDC.PatBlt(bmpRect.left, bmpRect.top, 1, bmpRect.Height(), PATCOPY);
  memDC.SelectObject(shadowBrush);
  memDC.PatBlt(bmpRect.right - 1, 1, 1, bmpRect.Height(), PATCOPY);
  memDC.PatBlt(bmpRect.left, bmpRect.Height(), bmpRect.Width(), 1, PATCOPY);
}

void TPopupButton::EvClicked()
{
  PopupEdit->Clicked();
}

void TPopupButton::EvDrawItem(uint /*ctrlId*/, const DRAWITEMSTRUCT& drawInfo)
{
  TRect bmpRect;
  if (drawInfo.itemState & ODS_SELECTED)
    bmpRect = BmpArray->CelRect(Down);
  else if (drawInfo.itemState & ODS_DISABLED)
    bmpRect = BmpArray->CelRect(Disabled);
  else
    bmpRect = BmpArray->CelRect(Up);

  TDC dc(drawInfo.hDC);
  TMemoryDC memDC(dc);
  memDC.SelectObject(*BmpArray);
  dc.BitBlt(0, 0, Attr.W, Attr.H, memDC, bmpRect.left, bmpRect.top);
}

void TPopupButton::EvMouseMove(uint modKeys, const TPoint& point)
{
  if (TYPESAFE_DOWNCAST(Parent, TGadgetWindow) && !(modKeys & MK_LBUTTON))
    PopupEdit->SendMessage(WM_MOUSEMOVE, (WPARAM)modKeys, MAKELPARAM(0, 0));
  TButton::EvMouseMove(modKeys, point);
}

void TPopupButton::EvSysColorChange()
{
  BuildBmpArray();
  TButton::EvSysColorChange();
}

auto TPopupButton::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{_T("BUTTON")};
}

void TPopupButton::Layout(int bSizeEditBox)
{

  TRect  rcEdit, rcParent, rcThis;

  PopupEdit->GetWindowRect(rcEdit);
  Parent->GetClientRect(rcParent);
  Parent->MapWindowPoints(NULL, rcParent, 2);

  if (bSizeEditBox)
    rcEdit.right -= BmpArray->CelSize().cx + 1;

  PopupEdit->SetWindowPos(NULL, rcEdit, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

  rcThis = rcEdit ;
  rcThis -= rcParent.TopLeft();
  rcThis.left = rcThis.right + 1;
  rcThis.right += BmpArray->CelSize().cx + 1;

  SetWindowPos(*PopupEdit, rcThis, SWP_NOACTIVATE);

}

void TPopupButton::SetupWindow()
{
  TButton::SetupWindow();
  EnableWindow(PopupEdit->IsWindowEnabled());
  Layout();
}


//
//-- TPopupEdit ----------------------------------------------------------------
//

DEFINE_RESPONSE_TABLE1(TPopupEdit, TEdit)
EV_WM_ENABLE,
EV_WM_KEYDOWN,
EV_WM_MOUSEMOVE,
EV_WM_SHOWWINDOW,
EV_WM_SIZE,
EV_WM_SYSKEYDOWN,
END_RESPONSE_TABLE;

TPopupEdit::TPopupEdit(TWindow* parent, int id, LPCTSTR text,
             int x, int y, int w, int h, TResId bmpId, uint textLen,
             TModule* module)
             :
TEdit(parent, id, text, x, y, w, h, textLen, false, module)
{
  Init(bmpId);
}

TPopupEdit::TPopupEdit(TWindow* parent, int resourceId, TResId bmpId,
             uint textLen, TModule* module)
             :
TEdit(parent, resourceId, textLen, module)
{
  Init(bmpId);
}

TPopupEdit::~TPopupEdit()
{
  delete Font;
}

void TPopupEdit::EvEnable(bool enabled)
{
  PopupBtn->EnableWindow(enabled);
}

void TPopupEdit::EvKeyDown(uint key, uint repeatCount, uint flags)
{
  switch (key){
case VK_DOWN:
  Clicked();
  break;
default:
  TEdit::EvKeyDown(key, repeatCount, flags);
  }
}

void TPopupEdit::EvMouseMove(uint modKeys, const TPoint& point)
{
  if (TYPESAFE_DOWNCAST(Parent, TGadgetWindow)){
    TPoint p(point);
    MapWindowPoints(*Parent, &p, 1);
    Parent->SendMessage(WM_MOUSEMOVE, (WPARAM)modKeys, MAKELPARAM(p.x, p.y));
  }
  else
    TEdit::EvMouseMove(modKeys, point);
}

void TPopupEdit::EvShowWindow(bool show, uint status)
{
  TEdit::EvShowWindow(show, status);
  if (*PopupBtn)
    PopupBtn->ShowWindow(show ? SW_SHOW : SW_HIDE);
}

void TPopupEdit::EvSize(uint /*sizeType*/, const TSize& /*size*/)
{
  static int bEvSize = false;

  if (!bEvSize){
    bEvSize = true;
    if (TYPESAFE_DOWNCAST(Parent, TGadgetWindow))
      PopupBtn->Layout(false);
    DefaultProcessing();
    bEvSize = false;
  }
}

void TPopupEdit::EvSysKeyDown(uint key, uint repeatCount, uint flags)
{
  switch (key){
case VK_DOWN:
  if (flags & KF_ALTDOWN){
    Clicked();
    break;
  }
default:
  TEdit::EvSysKeyDown(key, repeatCount, flags);
  }
}

void TPopupEdit::Init(TResId bmpId)
{
  Font = 0;
  PopupBtn = new TPopupButton(Parent, this, bmpId, GetModule());
}

void TPopupEdit::SetupWindow()
{
  TEdit::SetupWindow();

  if (TYPESAFE_DOWNCAST(Parent, TGadgetWindow)){
    Font = new TFont(_T("MS Shell Dlg"), -8);
    SetWindowFont(*Font, false);
  }
}

void TPopupEdit::SysColorChange()
{
  HandleMessage(WM_SYSCOLORCHANGE);
  PopupBtn->HandleMessage(WM_SYSCOLORCHANGE);
}


//
//-- TToolBar ------------------------------------------------------------------
//

TToolBar::TToolBar(TWindow* parent, TTileDirection direction, TFont* font,
           TModule* module)
           :
TControlBar(parent, direction, font, module)
{
}

bool TToolBar::IdleAction(long idleCount)
{
  if (idleCount == 0){
    if (AtMouse){
      TPoint crsPoint;
      GetCursorPos(crsPoint);
      ScreenToClient(crsPoint);
      if (!GadgetFromPoint(crsPoint) && !ChildWindowFromPoint(crsPoint))
        HandleMessage(WM_MOUSEMOVE, 0, MAKELPARAM(-1, -1));
    }

    for (TGadget* g = Gadgets; g; g = g->NextGadget())
      g->CommandEnable();
  }
  return TWindow::IdleAction(idleCount);
}

//
//-- TPopupEditGadget ----------------------------------------------------------
//

TPopupEditGadget::TPopupEditGadget(TWindow& control, TBorderStyle border)
:
TControlGadget(control, border)
{
}

void TPopupEditGadget::MouseEnter(uint modKeys, const TPoint& pt)
{
  TControlGadget::MouseEnter(modKeys, pt);
  if (Window->GetHintMode() == TGadgetWindow::EnterHints)
    Window->SetHintCommand(GetId());
}

void TPopupEditGadget::MouseLeave(uint modKeys, const TPoint& pt)
{
  TControlGadget::MouseLeave(modKeys, pt);
  if (Window->GetHintMode() == TGadgetWindow::EnterHints)
    Window->SetHintCommand(-1);
}

void TPopupEditGadget::SysColorChange()
{
  TPopupEdit* p = TYPESAFE_DOWNCAST( Control, TPopupEdit ) ;
  if( p )
    p->SysColorChange();
}


//
//-- Miscellaneous functions ---------------------------------------------------
//

// DrawMaskedBitmap was developed by Steve Saxon (CIS: 100321,2355)
// and Russell Morris (CIS: 75627,3615).
bool DrawMaskedBmp(HDC hDC, int x, int y, HBITMAP hbm, int XOrigin, int nWidth)
{
  HDC hdcShadow, hdcSource;
  HBITMAP hbmpOld, hbmpOld2, hbmDraw;
  BITMAP bm;
  int nHeight;
  bool bReturn;

  if (hbm == NULL)
    return FALSE;

  GetObject(hbm, sizeof(bm), &bm);

  if (nWidth == 0)
    nWidth = bm.bmWidth - XOrigin;

  nHeight = bm.bmHeight >> 1;

  hbmDraw = CreateCompatibleBitmap(hDC, nWidth, nHeight);

  if (hbmDraw == NULL)
    return FALSE;

  hdcShadow = CreateCompatibleDC(hDC);
  hdcSource = CreateCompatibleDC(hDC);

  hbmpOld = (HBITMAP)SelectObject(hdcShadow, hbmDraw);
  hbmpOld2 = (HBITMAP)SelectObject(hdcSource, hbm);

  bReturn = BitBlt(hdcShadow, 0, 0, nWidth, nHeight, hDC, x, y, SRCCOPY);

  if (bReturn)
    bReturn = BitBlt(hdcShadow, 0, 0, nWidth, nHeight, hdcSource, XOrigin, nHeight, SRCAND);

  if (bReturn)
    bReturn = BitBlt(hdcShadow, 0, 0, nWidth, nHeight, hdcSource, XOrigin, 0, SRCPAINT);

  if (bReturn)
    bReturn = BitBlt(hDC, x, y, nWidth, nHeight, hdcShadow, 0, 0, SRCCOPY);

  SelectObject(hdcShadow, hbmpOld);
  SelectObject(hdcSource, hbmpOld2);

  DeleteDC(hdcShadow);
  DeleteDC(hdcSource);

  DeleteObject(hbmDraw);

  return bReturn;
}

} // OwlExt namespace
//==============================================================================

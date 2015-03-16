//------------------------------------------------------------------------------
// ObjectWindows 1998 by Yura Bidus
//
// Another implementation of Flat controls.
// It use ideas from Joseph Parrello, Dieter Windau,and Kirk Stowell
//
//
/// \file
/// Implementation of classes TFlatComboBox,TFlatEdit, TFlatListBox
/// Implementation of classes TGadgetComboBox,TGadgetEdit, TGadgetListBox
//------------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/uihelper.h>
#include <owl/uimetric.h>

#include <owl/flatctrl.h>

namespace owl {

OWL_DIAGINFO;


////////////////////////////////////////////////////////////////////////////////
//#define TEST
//
void
TFlatPainter::SetFlatStyle(bool flat)
{
  if(flat && !IsSet(fpFlatLook)){
    Set(fpFlatLook);
    TWindow* wnd = TYPESAFE_DOWNCAST(this, TWindow);
    CHECK(wnd);
    if(wnd)
      wnd->Invalidate(false);
  }
  else if(!flat && IsSet(fpFlatLook)){
    Clear(fpFlatLook);
    TWindow* wnd = TYPESAFE_DOWNCAST(this, TWindow);
    CHECK(wnd);
    if(wnd)
      wnd->Invalidate(false);
  }
}

//
bool
TFlatPainter::IdleAction(long idleCount)
{
  TWindow* wnd = TYPESAFE_DOWNCAST(this, TWindow);
  if (idleCount == 0 && wnd && IsFlat() /*&& IsActiveApplicatino()*/){ ///!!!!!!!!!!!!!!!!!!!!!!!!!
    TPoint point;
    wnd->GetCursorPos(point);
    wnd->ScreenToClient(point);

    TRect rect = wnd->GetWindowRect();
    ::MapWindowPoints(HWND_DESKTOP, *wnd, LPPOINT(&rect), 2);

    if (rect.Contains(point)){
      if(!IsSet(fpMouseIn)){
        Set(fpMouseIn);
        wnd->Invalidate(false);
      }
    }
    else{
      if(IsSet(fpMouseIn)) {
        Clear(fpMouseIn);
        wnd->Invalidate(false);
      }
    }
  }
  return false;
}

//
void
TFlatPainter::Paint(TDC& dc, TRect& rect)
{
  TWindow* wnd = TYPESAFE_DOWNCAST(this, TWindow);
  if(!wnd)
    return;

  if(IsSet(fpMouseIn) && wnd->IsWindowEnabled()){
    // We draw the "tracked" situation
#ifdef TEST
    TBrush brush (TColor::LtRed);
#else
    TBrush brush (TColor::Sys3dFace);
#endif
    dc.FrameRect(rect, brush);
    rect.Inflate(-1, -1);
    dc.FrameRect(rect, brush);
    rect.Inflate(-1, -1);
    dc.FrameRect(rect, brush);
    rect.Inflate(1, 1);
    TUIBorder::DrawEdge(dc, rect, TUIBorder::SunkenOuter, TUIBorder::Rect);
  }
  else{
    // We draw the "untracked" situation
#ifdef TEST
    TBrush brush(TColor::LtBlue);
    TBrush brush2(TColor::LtGreen);
#else
    TBrush brush(TColor::Sys3dFace);
    TBrush brush2(TColor::SysWindow);
#endif

    dc.FrameRect(rect,brush);
    rect.Inflate(-1, -1);
    dc.FrameRect(rect,brush);
    rect.Inflate(-1, -1);
    dc.FrameRect(rect, brush2);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// class TFlatComboBox
// ~~~~~ ~~~~~~~~~~~~~
//
DEFINE_RESPONSE_TABLE1(TFlatComboBox, TComboBox)
  EV_WM_PAINT,
END_RESPONSE_TABLE;

//
void
TFlatComboBox::SetupWindow()
{
  TComboBox::SetupWindow();
  SetExtendedUI(true);
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);
}

//
// Intercept WM_PAINT to redirect from TWindow to the underlying control iff
// this Owl object is just a wrapper for a predefined class.
//
void
TFlatComboBox::EvPaint()
{
  TComboBox::EvPaint();

  if (IsFlat()){
    TWindowDC dc(GetHandle());
    TRect rect = GetClientRect();
    TFlatPainter::Paint(dc, rect);

    if(!IsSet(fpMouseIn) || !IsWindowEnabled()){
      // We draw the "untracked" situation
      rect.left = rect.right - TUIMetric::CxHThumb;// + 1;
#ifdef TEST
       dc.FrameRect(rect, TBrush(TColor::LtGreen));
#else
       dc.FrameRect(rect, TBrush(TColor::SysWindow));
#endif
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// class TFlatEdit
// ~~~~~ ~~~~~~~~~
//

DEFINE_RESPONSE_TABLE1(TFlatEdit, TEdit)
  EV_WM_PAINT,
END_RESPONSE_TABLE;

void
TFlatEdit::SetupWindow()
{
  TEdit::SetupWindow();
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);
}

//
// Intercept WM_PAINT to redirect from TWindow to the underlying control iff
// this Owl object is just a wrapper for a predefined class.
//
void
TFlatEdit::EvPaint()
{
  TEdit::EvPaint();

  if (IsFlat())
  {
    TWindowDC dc(GetHandle());
    TRect rect = GetClientRect();

    //28.11.2007 Jogy - painting of the was wrong, added some corrections  
	rect.top -= 4;
	rect.left -= 4;
	rect.right += 6;
    rect.bottom += 6;
    TFlatPainter::Paint(dc, rect);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// class TFlatListBox
// ~~~~~ ~~~~~~~~~~~~
//

DEFINE_RESPONSE_TABLE1(TFlatListBox, TListBox)
  EV_WM_PAINT,
END_RESPONSE_TABLE;

void
TFlatListBox::SetupWindow()
{
  TListBox::SetupWindow();
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);
}

//
// Intercept WM_PAINT to redirect from TWindow to the underlying control iff
// this Owl object is just a wrapper for a predefined class.
//
void
TFlatListBox::EvPaint()
{
  TListBox::EvPaint();

  if (IsFlat()){

    TWindowDC dc(GetHandle());

    TRect rect  = GetClientRect();
    TRect wrect = GetWindowRect();
    ::MapWindowPoints(HWND_DESKTOP, *this, LPPOINT(&wrect), 2);

    rect.bottom += 6;
    rect.right  += 6;
    wrect.Offset(3,3);

    bool haveScroll = wrect != rect;
    if(haveScroll){
       wrect.bottom -= 1;
       wrect.right  -= 1;
    }

    TFlatPainter::Paint(dc, wrect);

    if((!IsSet(fpMouseIn) || !IsWindowEnabled()) && haveScroll){
      // We draw the "untracked" situation
      wrect.left = wrect.right - TUIMetric::CxHThumb;

#ifdef TEST
       dc.FrameRect(wrect, TBrush(TColor::LtGreen));
#else
       dc.FrameRect(wrect, TBrush(TColor::SysWindow));
#endif
    }
  }
}
//------------------------------------------------------------------------------
//
bool
TGadgetComboBox::IsFlat()
{
  TGadgetWindow* wnd = TYPESAFE_DOWNCAST(GetParentO(),TGadgetWindow);
  if(wnd)
    return ToBool(wnd->GetFlatStyle()&TGadgetWindow::FlatStandard);
  return false;
}
//------------------------------------------------------------------------------
//
bool
TGadgetEdit::IsFlat()
{
  TGadgetWindow* wnd = TYPESAFE_DOWNCAST(GetParentO(),TGadgetWindow);
  if(wnd)
    return ToBool(wnd->GetFlatStyle()&TGadgetWindow::FlatStandard);
  return false;
}
//------------------------------------------------------------------------------
//
bool
TGadgetListBox::IsFlat()
{
  TGadgetWindow* wnd = TYPESAFE_DOWNCAST(GetParentO(),TGadgetWindow);
  if(wnd)
    return ToBool(wnd->GetFlatStyle()&TGadgetWindow::FlatStandard);
  return false;
}

} // OWL namespace

//==============================================================================


//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TTextGadget.
/// Implementation of TDynamicTextGadget.
//------------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/textgadg.h>
#include <owl/gadgetwi.h>
#include <owl/uimetric.h>
#include <owl/uihelper.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a TTextGadget object with the specified ID, border style, and
/// alignment. Sets Margins.Left and Margins.Right to 2. Sets Text and TextLen to 0.
//
TTextGadget::TTextGadget(int          id,
                         TBorderStyle borderStyle,
                         TAlign       align,
                         uint         numChars,
                         LPCTSTR      text,
                         TFont*       font)
:
  TGadget(id, borderStyle),
  Font(font)
{
  Margins.Left = Margins.Right = TUIMetric::CxFixedFrame;
  Align = align;
  NumChars = numChars;
  Text = 0;
  TextLen = 0;
  // init colors to same as reset methods
  ResetBkgndColor();
  ResetTextColor(); 

  SetShrinkWrap(false, true);
  SetText(text);
}

//
/// String-aware overload
//
TTextGadget::TTextGadget(
  int id,
  TBorderStyle borderStyle,
  TAlign align,
  uint numChars,
  const tstring& text,
  TFont* font
  )
  : TGadget(id, borderStyle),
  Font(font)
{
  Margins.Left = Margins.Right = TUIMetric::CxFixedFrame;
  Align = align;
  NumChars = numChars;
  Text = 0;
  TextLen = 0;
  // init colors to same as reset methods
  ResetBkgndColor();
  ResetTextColor(); 

  SetShrinkWrap(false, true);
  SetText(text);
}

//
/// Destruct this text gadget and delete the text it is holding
//
TTextGadget::~TTextGadget()
{
  delete[] Text;
  delete Font;
}

//
/// Invalidate the working portion of this gadget--in this case just the
/// InnerRect
///
/// Calls TGadget::GetInnerRect to compute the area of the text for the gadget and
/// then TGadget::InvalidateRect to invalidate the rectangle in the parent window.
void
TTextGadget::Invalidate()
{
  TRect  innerRect;

  GetInnerRect(innerRect);
  bool transparent = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatXPTheme;
  InvalidateRect(innerRect, transparent); // Erase (redraw background) if transparent.
}

//
/// Set the text for this gadget
//
/// If the text stored in Text is not the same as the new text, SetText deletes the
/// text stored in Text. Then, it sets TextLen to the length of the new string. If
/// no text exists, it sets both Text and TextLen to 0 and then calls Invalidate to
/// invalidate the rectangle.
//
void
TTextGadget::SetText(LPCTSTR text)
{
  // Skip processing if new text is the same as current
  //
  if (text && Text && _tcscmp(text, Text) == 0)
    return;

  delete[] Text;

  if (text) {
    Text = strnewdup(text);
    TextLen = ::_tcslen(Text);
  }
  else {
    Text = 0;
    TextLen = 0;
  }

  if (GetGadgetWindow())
    GetGadgetWindow()->GadgetChangedSize(*this);

  Invalidate();
}

//
/// Respond to the virtual call to let this gadget's Window know how big this
/// text gadget wants to be based on the text size.
//
/// If shrink-wrapping is requested, GetDesiredSize returns the size needed to
/// accommodate the borders, margins, and text; otherwise, if shrink-wrapping is not
/// requested, it returns the gadget's current width and height.
//
void
TTextGadget::GetDesiredSize(TSize& size)
{
  TGadget::GetDesiredSize(size);
  TFont* font = Font;
  if (font == 0)
    font = &(GetGadgetWindow()->GetFont());

  if (font == 0)
    return;

  if (ShrinkWrapWidth)
    size.cx += font->GetTextExtent(Text).cx;
  else {
    int  left, right, top, bottom;
    GetOuterSizes(left, right, top, bottom);

    int newW = font->GetMaxWidth() * NumChars;
    size.cx += newW + left + right - Bounds.Width();  // Old bounds already considered
  }

  if (ShrinkWrapHeight)
    size.cy += font->GetHeight() + 2;
}

//
/// Paint the text gadget by painting gadget borders, & then painting text in
/// the InnerRect. Empty or 0 text blanks the gadget.
//
/// Calls TGadget::PaintBorder to paint the border. Calls TGadget::GetInnerRect to
/// calculate the area of the text gadget's rectangle. If the text is left-aligned,
/// Paint calls dc.GetTextExtent to compute the width and height of a line of the
/// text. To set the background color, Paint calls dc.GetSysColor and sets the
/// default background color to face shading (COLOR_BTNFACE). To set the button text
/// color, Paint calls dc.SetTextColor and sets the default button text color to
/// COLOR_BTNTEXT. To draw the text, Paint calls dc.ExtTextOut and passes the
/// parameters ETO_CLIPPED (so the text is clipped to fit the rectangle) and
/// ETO_OPAQUE (so the rectangle is filled with the current background color).
//
void
TTextGadget::Paint(TDC& dc)
{
  PaintBorder(dc);

  TRect  innerRect;
  GetInnerRect(innerRect);

  if (!Font)
    dc.SelectObject(GetGadgetWindow()->GetFont());
  else
    dc.SelectObject(*Font);

  TColor textColor = GetEnabledColor();
  if(!GetEnabled())
    textColor = TColor::Sys3dHilight;

  bool transparent = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatXPTheme;
  if(!Text){
    if (!transparent)
    {
    TColor color = dc.SetBkColor(TColor::Sys3dFace);
    dc.ExtTextOut(0,0, ETO_OPAQUE, &innerRect, _T(""), 0);
    dc.SetBkColor(color);
  }
  }
  else
  {
    // Create a UI Face object for this button & let it paint the button face
    //
    uint align[] = {DT_LEFT, DT_CENTER, DT_RIGHT};
    uint format =  DT_SINGLELINE | DT_VCENTER | align[Align];
            TUIFace face(innerRect, Text, BkgndColor, format);

     TPoint  dstPt(innerRect.TopLeft());

      dc.SetBkColor(BkgndColor);

     TColor oldTxColor  = dc.SetTextColor(textColor);
    if (!GetEnabled())
      face.Paint(dc, dstPt, TUIFace::Disabled, false, !transparent);
    else
      face.Paint(dc, dstPt, TUIFace::Normal, false, !transparent);
    dc.SetTextColor(oldTxColor);
  }
  dc.RestoreFont();
}
void TTextGadget::SetBkgndColor(TColor& color)
{
  BkgndColor = color;
}

void TTextGadget::SetTextColor(TColor& color)
{
  TextColor = color;
}

void TTextGadget::ResetBkgndColor()
{
  BkgndColor = TColor::Sys3dFace;
}

void TTextGadget::ResetTextColor()
{
  TextColor = TColor::SysBtnText;
}
//------------------------------------------------------------------------------
// class TDynamicTextGadgetEnabler 
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~
//
class TDynamicTextGadgetEnabler : public TCommandEnabler {
  public:
    TDynamicTextGadgetEnabler(TWindow::THandle hReceiver, 
                              TDynamicTextGadget* g)
    :
      TCommandEnabler(g->GetId(), hReceiver),
      Gadget(g)
    {
    }

    // Override TCommandEnabler virtuals
    //
    void  Enable(bool);
    void  SetText(LPCTSTR);
    void  SetCheck(int){}

  protected:
    TDynamicTextGadget*  Gadget;
};
//
void TDynamicTextGadgetEnabler::Enable(bool enable)
{
  TCommandEnabler::Enable(enable);
  if(Gadget->GetEnabled() != enable)
    Gadget->SetEnabled(enable);
}
//
void TDynamicTextGadgetEnabler::SetText(LPCTSTR text)
{
  if(!Gadget->GetText() || _tcscmp(Gadget->GetText(), text) != 0)
    Gadget->SetText(text);
}
//
//--------------------------------------------------------
// TDynamicTextGadget
//
TDynamicTextGadget::TDynamicTextGadget(int id, TBorderStyle style, TAlign alighn,uint numChars, LPCTSTR text,TFont* font) 
: 
  TTextGadget(id, style, alighn, numChars, text,font)
{
}

//
// String-aware overload
//
TDynamicTextGadget::TDynamicTextGadget(
  int id, 
  TBorderStyle style, 
  TAlign align,
  uint numChars, 
  const tstring& text,
  TFont* font
  ) 
  : TTextGadget(id, style, align, numChars, text, font)
{}

//
void TDynamicTextGadget::CommandEnable()
{ 
  PRECONDITION(GetGadgetWindow());

  // Must send, not post here, since a ptr to a temp is passed
  //
  // This might be called during idle processing before the
  // HWND has created.  Therefore, confirm handle exists.
  //
  if (GetGadgetWindow()->GetHandle()){
    TDynamicTextGadgetEnabler ge(*GetGadgetWindow(), this);
    GetGadgetWindow()->HandleMessage(WM_COMMAND_ENABLE,0,TParam2(&ge));
  }
}
//
} // OWL namespace
/* ========================================================================== */


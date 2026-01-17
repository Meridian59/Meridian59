//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMessageBar.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/messageb.h>
#include <owl/textgadg.h>
#include <owl/uimetric.h>

namespace owl {

OWL_DIAGINFO;

static TFont* createMsgBarFont()
{
  return new TDefaultGuiFont(TDefaultGuiFont::sfiStatus);
}
//
/// Constructs a TMessageBar object with the gadget window font. Sets IDW_STATUSBAR,
/// HighlightLine to true, and TTextGadget's member WideAsPossible to true, making
/// the text gadget as wide as the window.
//
TMessageBar::TMessageBar(TWindow* parent, TFont* font, TModule* module)
:
  TGadgetWindow(parent, Horizontal, font ? font : createMsgBarFont(), module),
  HighlightLine{false},
  HintText{},
  GadgetVisibilityCache{}
{
  Attr.Id = IDW_STATUSBAR;
  TTextGadget* textGadget = new TTextGadget(IDG_MESSAGE, TTextGadget::Recessed, TTextGadget::Left, 10, 0, 0);
  textGadget->SetWideAsPossible();
  Insert(*textGadget);
}

//
/// Adjusts the message bar and paints a highlight line. Then, PaintGadgets either
/// paints the hint text if any is set or calls TGadgetWindow::PaintGadgets to
/// repaint each gadget.
//
void
TMessageBar::PaintGadgets(TDC& dc, bool erase, TRect& rect)
{
  if (HighlightLine && rect.top == 0)
    dc.TextRect(0, 0, rect.right, TUIMetric::CyBorder, TColor::Sys3dHilight);

  if (!HintText.empty())
  {
    TRect clientRect = GetClientRect();
    int y = (clientRect.bottom - GetFontHeight()) / 2;
    if (HighlightLine)
      clientRect.top += TUIMetric::CyBorder;
    dc.SelectObject(GetFont());
    dc.SetBkColor(TColor::Sys3dFace);
    dc.ExtTextOut(5, y, ETO_OPAQUE, &clientRect, HintText, static_cast<int>(HintText.length()));
  }
  else
  {
    TGadgetWindow::PaintGadgets(dc, erase, rect);
  }
}

//
/// GetInnerRect computes the rectangle inside the borders and margins of the
/// message bar. Adjust for the top highlight line if one is set.
//
void
TMessageBar::GetInnerRect(TRect& innerRect)
{
  TGadgetWindow::GetInnerRect(innerRect);

  if (HighlightLine)
    innerRect.top += TUIMetric::CyBorder;
}

//
/// Calls TGadgetWindow's GetDesiredSize to get the size of the message bar. Then,
/// if a highlighting line is drawn, adjusts the size of the message bar.
//
void
TMessageBar::GetDesiredSize(TSize& size)
{
  TGadgetWindow::GetDesiredSize(size);

  if (HighlightLine)
    size.cy++;
}

//
/// Forwards the message in the message bar to the text gadget for formatting.
//
void
TMessageBar::SetText(const tstring& text)
{
  SetMessageText(IDG_MESSAGE, text);
}

//
/// Sets the text for the default text message gadget by specifying the id.
//
void
TMessageBar::SetMessageText(int id, const tstring& text)
{
  TTextGadget* tg = TYPESAFE_DOWNCAST(GadgetWithId(id), TTextGadget);
  if (tg)
    tg->SetText(text.c_str());
}

void
TMessageBar::SetHintText(LPCTSTR text)
{
  if (text && *text) // !empty
  {
    if (text == HintText) return;
    HintText = text;
  }
  else
  {
    if (HintText.empty()) return;
    HintText.clear();
  }

  if (HintText.empty())
  {
    // Restore the visibility state of the gadgets.
    //
    // NOTE: We here ignore any changes in gadget visibility between the time the hint text was set
    // and the time it was cleared (now). To improve this, a notification mechanism is needed to
    // detect calls to TGadget::SetVisible while the hint text is shown, and update the visibility
    // state cache accordingly.
    //
    for (auto g = FirstGadget(); g; g = NextGadget(*g))
    {
      const auto i = GadgetVisibilityCache.find(g);
      g->SetVisible(i != GadgetVisibilityCache.cend() ? i->second : true);
    }
    GadgetVisibilityCache.clear();
  }
  else if (GadgetVisibilityCache.empty())
  {
    // Store the visibility state of the gadgets and hide them.
    //
    for (auto g = FirstGadget(); g; g = NextGadget(*g))
    {
      GadgetVisibilityCache.insert({g, g->IsVisible()});
      g->SetVisible(false);
    }
  }

  if (IsWindow())
    Invalidate();
}

void TMessageBar::ClearHintText()
{
  SetHintText(0);
}

} // OWL namespace
/* ========================================================================== */


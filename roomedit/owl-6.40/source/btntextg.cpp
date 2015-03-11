//----------------------------------------------------------------------------//
// ObjectWindows 1998 Copyright by Yura Bidus                                 //
//                                                                            //
// Used code and ideas from Dieter Windau and Joseph Parrello                 //
//                                                                            //
// THIS CLASS AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF         //
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO        //
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             //
// PARTICULAR PURPOSE.                                                        //
/// \file                                                                           //
/// Implementation of class TButtonTextGadget.                                 //
//----------------------------------------------------------------------------//
#include <owl/pch.h>

#include <owl/celarray.h>
#include <owl/gadgetwi.h>
#include <owl/uihelper.h>
#include <owl/btntextg.h>
#include <owl/decframe.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGadget);

//------------------------------------------------------------------------------
//
/// \class TButtonTextGadgetEnabler
/// Command enabler for button text gadget
//

#if defined(BI_COMP_BORLANDC)
# pragma warn -inl
#endif

class TButtonTextGadgetEnabler 
  : public TCommandEnabler 
{
public:
  TButtonTextGadgetEnabler(TWindow::THandle hReceiver, TButtonTextGadget* g)
    : TCommandEnabler(g->GetId(), hReceiver), Gadget(g)
  {}

  //
  // Override TCommandEnabler virtuals.
  //
  virtual void Enable(bool);
  virtual void SetText(LPCTSTR text);
  void SetText(const tstring& s) {SetText(s.c_str());}
  virtual void SetCheck(int);


protected:
  TButtonTextGadget*  Gadget;
};

#if defined(BI_COMP_BORLANDC)
# pragma warn .inl
#endif

//
/// Enable or disable the button gadget
//
void
TButtonTextGadgetEnabler::Enable(bool enable)
{
  TCommandEnabler::Enable(enable);
  Gadget->SetEnabled(enable);
}

//
/// Handle the SetText request for a button-text gadget.
//
void
TButtonTextGadgetEnabler::SetText(LPCTSTR text)
{
  if(Gadget->GetStyle()& TButtonTextGadget::sText)
    Gadget->SetText(text);
}

//
/// Set the check-state for the button gadget
//
void
TButtonTextGadgetEnabler::SetCheck(int state)
{
  Gadget->SetButtonState(TButtonTextGadget::TState(state));
}

//------------------------------------------------------------------------------

const int TextSpaceV = 0;
const int TextSpaceH = 0;

//
/// Constructs a TButtonTextGadget object using the specified bitmap ID, button
/// gadget ID, style, and type, with enabled set to false, in a button-up state, and
/// reserved number chars in text = 4. The button isn't enabled - its initial state
/// before command enabling occurs.
//
TButtonTextGadget::TButtonTextGadget(
  int id, TResId glyphResIdOrIndex,
  TStyle style, TType type,
  bool enabled, TState state,
  bool sharedGlyph, uint numChars)
:
  TButtonGadget(glyphResIdOrIndex, id, type, enabled, state, sharedGlyph),
  Text(),
  NumChars(numChars),
  Align(aCenter),
  Style(style),
  LayoutStyle(lTextBottom),
  Font(0)
{}

//
/// Destroys a TButtonTextGadget object.
//
TButtonTextGadget::~TButtonTextGadget()
{
  delete Font;
}

//
/// Initiates a command enable for this button gadget.
/// Enables the button-text gadget to capture messages. Calls SendMessage to send a
/// WM_COMMAND_ENABLE message to the gadget window's parent, passing a
/// TCommandEnable:EvCommandEnable message for this button.
//
void
TButtonTextGadget::CommandEnable()
{
  PRECONDITION(Window);

  // Must send, not post here, since a ptr to a temp is passed
  //
  // This might be called during idle processing before the
  // HWND has created.  Therefore, confirm handle exists.
  //
  if (GetGadgetWindow()->GetHandle()) {
    TButtonTextGadgetEnabler ge(*GetGadgetWindow(), this);
    GetGadgetWindow()->HandleMessage(WM_COMMAND_ENABLE, 0,TParam2(&ge));
  }
}

//
/// Sets the text of the gadget. If the given text is blank, then we attempt to
/// load the text from the menu or tooltip.
//
void
TButtonTextGadget::SetText(const tstring& text, bool repaint)
{
  if (text == Text) return;
  Text = text;

  if (Text.length() == 0 && (Style & sText) && GetGadgetWindow())
  {
    TWindow* parent = GetGadgetWindow()->GetParentO();
    TDecoratedFrame* frame= parent ? dynamic_cast<TDecoratedFrame*>(parent) : 0;
    while (parent && !frame){
      parent = parent->GetParentO();
      if (parent)
        frame = dynamic_cast<TDecoratedFrame*>(parent);
    }
    CHECK(frame);
    Text = frame->GetHintText(GetResId().GetInt(), htTooltip);
  }

  if (GetGadgetWindow() && repaint)
    GetGadgetWindow()->GadgetChangedSize(*this);
}

//
/// Set Font to be used by the Gadget. If repaint is true calls
/// Window->GadgetChangedSize(*this) to recalculate size of gadget.
//
void
TButtonTextGadget::SetFont(TFont* font, bool repaint)
{
  delete Font;
  Font = font;
  if (GetGadgetWindow() && repaint)
    GetGadgetWindow()->GadgetChangedSize(*this);
}


//
/// If the style stored in Style is not the same as the new style, SetStyle sets
/// Style to the new style, and then if repaint is true calls
/// Window->GadgetChangedSize(*this) to recalculate size of gadget.
//
void
TButtonTextGadget::SetStyle(TStyle style, bool repaint)
{
  if(Style != style){
    Style = style;
    if (GetGadgetWindow() && repaint)
      GetGadgetWindow()->GadgetChangedSize(*this);
  }
}

//
/// If the align stored in Style is not the same as the new align, SetAlign sets
/// Align to the new align, and then if repaint is true calls Invalidate to
/// invalidate the rectangle.
//
void
TButtonTextGadget::SetAlign(const TAlign align, bool repaint)
{
  if(Align != align){
    Align = align;
    if(repaint)
      Invalidate();
  }
}

//
/// If the style stored in LayoutStyle is not the same as the new style,
/// SetLayoutStyle sets LayoutStyle to the new style, and then if repaint is true
/// calls Window->GadgetChangedSize(*this) to recalculate size of gadget. 
//
void
TButtonTextGadget::SetLayoutStyle(const TLayoutStyle style, bool repaint)
{
  if(LayoutStyle != style){
    LayoutStyle = style;
    if (GetGadgetWindow() && repaint)
      GetGadgetWindow()->GadgetChangedSize(*this);
  }
}


//
/// Calls TButtonGadget::Created and if Text == 0 and (Style & sText) retrieves text
/// from menu or resource. 
///
/// Virtual called after the window holding a gadget has been created
//
void
TButtonTextGadget::Created()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  TButtonGadget::Created();

  // If text style and text isn't set, then get it now from the menu/resource.
  //
  if ((Style & sText) && Text.length() == 0)
  {
    SetText(_T(""));
    Invalidate();
  }
}

//
/// Respond to a WM_SYSCOLORCHANGE, in this case to rebuild the CelArray with
/// possibly new 3d colors
///
/// SysColorChange responds to an EvSysColorChange message forwarded by the owning
/// TGadgetWindow by setting the dither brush to zero. If a style is Bitmap it
/// forwards the message to TButtonGadget, otherwise to TGadget.
//
void
TButtonTextGadget::SysColorChange()
{
  // Baypass TButtonGadget handler if CellArray = 0
  if(Style&sBitmap)
    TButtonGadget::SysColorChange();
  else
    TGadget::SysColorChange();
}

//
void
TButtonTextGadget::GetTextSize(TSize& size)
{
  TFont* font = Font;
  if (font == 0)
    font = &(GetGadgetWindow()->GetFont());

  if (font == 0)
    return;

  TEXTMETRIC tm;
  font->GetTextMetrics(tm);

  size.cx += tm.tmAveCharWidth * NumChars;
  size.cy += tm.tmHeight + 2;
}

//
/// Calls TButtonGadget::GetDesiredSize  if (Style & sBitmap); calls
/// TGadget::GetDesiredSize and adds the size of the text region.
//
void
TButtonTextGadget::GetDesiredSize(TSize& size)
{
  PRECONDITION(Window);
  TRACEX(OwlGadget, 1, _T("TButtonTextGadget::GetDesiredSize() enter @") << this <<
    _T(" size ") << size);

  if(Style&sBitmap)
    TButtonGadget::GetDesiredSize(size);
  else
    TGadget::GetDesiredSize(size);

  // if paint text -> add text size
  if(Style&sText){
    TSize textSize;
    GetTextSize(textSize);

    TSize gsize;
    TGadget::GetDesiredSize(gsize);
    switch(LayoutStyle){
       case lTextLeft:
       case lTextRight:
         size.cx += textSize.cx + TextSpaceV;
         size.cy =  std::max(size.cy,textSize.cy+gsize.cy);
         break;
       case lTextTop:
       case lTextBottom:
         size.cx += std::max(size.cx,textSize.cx+gsize.cx);
         size.cy += textSize.cy + TextSpaceH;
         break;
    }
  }
  TRACEX(OwlGadget, 1, _T("TButtonTextGadget::GetDesiredSize() leave @") << this <<
    _T(" size ") << size);
}

//
/// If (Style & sBitmap) calls TButtonGadget::SetBounds; otherwise calls
/// TGadget::SetBounds to set the boundary of the rectangle, and it (Style &
/// sBitmap) calculates new BitmapOrigin. 
//
void
TButtonTextGadget::SetBounds(const TRect& rect)
{
  PRECONDITION(Window);

  if(Style&sBitmap)
    TButtonGadget::SetBounds(rect);
  else
    TGadget::SetBounds(rect);

  if(Style&sBitmap && Style&sText){
    TRect  faceRect, textRect, btnRect;
    GetInnerRect(faceRect);

    Layout(faceRect, textRect, btnRect);

    TSize  bitmapSize = GetCelArray() ? GetCelArray()->CelSize() : GetGadgetWindow()->GetCelArray().CelSize();
    TPoint pt;
    pt.x = btnRect.left + (btnRect.Width() - bitmapSize.cx) / 2;
    pt.y = btnRect.top + (btnRect.Height() - bitmapSize.cy) / 2;
    SetBitmapOrigin(pt);
  }
}

//
/// Calls TGadget::PaintBorder to perform the actual painting of the border of the
/// control. Get Inner rectangle of control, layout Text and Bitmap and call
/// PaintText() and PaintFace().
//
void
TButtonTextGadget::Paint(TDC& dc)
{
  PRECONDITION(Window);

  PaintBorder(dc);

  TRect  faceRect, textRect, btnRect;
  GetInnerRect(faceRect);

  Layout(faceRect, textRect, btnRect);

  if(Style&sText)
    PaintText(dc, textRect, Text);
  if(Style&sBitmap)
    PaintFace(dc, btnRect);
}

//
/// Layout button and text in the control area. 
//
void
TButtonTextGadget::Layout(TRect& faceRect, TRect& textRect, TRect& btnRect)
{
  if(Style == sBitmap)
    btnRect = faceRect;
  else if(Style == sText)
    textRect = faceRect;
  else{
    TSize textSize;
    if(Style&sText)
      GetTextSize(textSize);

    TSize btnSize;
    if(Style&sBitmap)
      TButtonGadget::GetDesiredSize(btnSize);

    switch(LayoutStyle){

       case lTextLeft:
        if(Style&sText)
          textRect = TRect(faceRect.TopLeft(), TSize(textSize.cx,faceRect.Height()));
        if(Style&sBitmap)
          btnRect = TRect(TPoint(faceRect.left+textSize.cx+TextSpaceV,faceRect.top),
                          faceRect.BottomRight());
         break;

       case lTextRight:
        if(Style&sBitmap)
          btnRect = TRect(faceRect.left, faceRect.top,
                          faceRect.left+btnSize.cx, faceRect.bottom);
        if(Style&sText)
          textRect = TRect(btnRect.right+TextSpaceV,faceRect.top,
                           faceRect.right, faceRect.bottom);

         break;

       case lTextTop:
        if(Style&sText)
          textRect = TRect(faceRect.TopLeft(), TSize(faceRect.Width(),textSize.cy));
        if(Style&sBitmap)
          btnRect = TRect(TPoint(faceRect.left,faceRect.top+textSize.cy+TextSpaceH),
                          TSize(faceRect.Width(), faceRect.Height()-textSize.cy));
         break;

       case lTextBottom:
        if(Style&sText)
          textRect = TRect(faceRect.left,faceRect.bottom-textSize.cy,
                           faceRect.right,faceRect.bottom);
        if(Style&sBitmap)
          btnRect = TRect(faceRect.TopLeft(),
                          TSize(faceRect.Width(), faceRect.Height()-textSize.cy));
         break;
    }
  }
}

//
/// Paint Text
//
void
TButtonTextGadget::PaintText(TDC& dc, TRect& rect, const tstring& text)
{
  if (!Font)
    dc.SelectObject(GetGadgetWindow()->GetFont());
  else
    dc.SelectObject(*Font);

  TColor textColor = TColor::SysBtnText;
  if(!GetEnabled())
    textColor = TColor::Sys3dHilight;
  else if((GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatHotText) && IsHaveMouse())
     textColor = TColor::LtBlue;

   TColor oldTxColor  = dc.SetTextColor(textColor);

  uint format =  DT_SINGLELINE|DT_NOCLIP|DT_END_ELLIPSIS;
  switch(Align){
    case aLeft:
      format |= DT_LEFT;
      break;
    case aRight:
      format |= DT_RIGHT;
      break;
    case aCenter:
      format |= DT_CENTER;
      break;
  }
  switch(LayoutStyle){
    case lTextLeft:
    case lTextRight:
      format |= DT_VCENTER;
      break;
    case lTextTop:
      format |= DT_VCENTER;//DT_BOTTOM;
      break;
    case lTextBottom:
      format |= DT_VCENTER;//DT_TOP;
      break;
  }

  // Create a UI Face object for this button & let it paint the button face
  //
  TPoint  dstPt(rect.TopLeft());

  if (GetButtonState() == Down && GetEnabled() &&
      GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard)
  {
    if(IsHaveMouse()) 
    {
      if(IsPressed()) 
      {
        const int dx = (format & DT_CENTER) ? 2 : 1;
        const int dy = (format & DT_VCENTER) ? 2 : 1;
        rect.Offset(dx, dy);
      }
      TUIFace(rect, text, TColor::Sys3dFace,format).Paint(dc, dstPt,
                TUIFace::Normal, true, true);
    }
    else
    {
      TUIFace face(rect, text, TColor::Sys3dFace,format);
      if(GetGadgetWindow()->GadgetGetCaptured()==this)
        face.Paint(dc, dstPt, TUIFace::Normal, true);
      else
        face.Paint(dc, dstPt, TUIFace::Down, IsPressed(), false);
    }
  }
  else
  {
    TUIFace face(rect, text, TColor::Sys3dFace,format);
    if (!GetEnabled())
      face.Paint(dc, dstPt, TUIFace::Disabled, false, false);
    else if (GetButtonState() == Indeterminate)
      face.Paint(dc, dstPt, TUIFace::Indeterm, IsPressed(), false);
    else if (GetButtonState() == Down) // Down and not flat
      face.Paint(dc, dstPt, TUIFace::Down, IsPressed(), false);
    else
      face.Paint(dc, dstPt, TUIFace::Normal, IsPressed(), false);
  }

  dc.SetTextColor(oldTxColor);

  dc.RestoreFont();
}

} // OWL namespace

//==============================================================================

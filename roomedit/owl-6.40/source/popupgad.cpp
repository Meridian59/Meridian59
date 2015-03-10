//----------------------------------------------------------------------------//
// ObjectWindows 1998 Copyright by Yura Bidus                                 //
//                                                                            //
// Used code and ideas from Dieter Windau and Joseph Parrello                 //
//                                                                            //
// EMail: dieter.windau@usa.net                                               //
// Web:   http://members.aol.com/softengage/index.htm                         //
// E-Mail : joparrello@geocities.com, joparrel@tin.it                         //
// Web:   http://space.tin.it/computer/giparrel                               //
//                                                                            //
// Revision 1.5                                                               //
// Edited by Bidus Yura                                                       //
// Date 11/19/98                                                              //
//                                                                            //
// THIS CLASS AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF         //
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO        //
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             //
// PARTICULAR PURPOSE.                                                        //
//                                                                            //
// Implementation of class TPopupButtonGadget, TPopupMenuGadget.              //
//----------------------------------------------------------------------------//
#include <owl/pch.h>

#include <owl/celarray.h>
#include <owl/gadgetwi.h>
#include <owl/uihelper.h>
#include <owl/popupgad.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGadget);

//------------------------------------------------------------------------------
//

bool TPopupButtonGadget::PopupGuard = false;

inline int F2Arr(int arrow, int width)
{
  return arrow == -1 ? width*1/4 : arrow;
}

inline int Arr2F(int arrow, int width)
{
  return arrow == -1 ? width*1/3 : arrow;
}

//
// class TPopupButtonGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
TPopupButtonGadget::TPopupButtonGadget(int id, TResId glyphResIdOrIndex,
  TStyle style, TPopupType poptype, TType type, bool enabled, TState state,
  bool sharedGlyph, uint numChars)
:
  TButtonTextGadget(id, glyphResIdOrIndex, style, type, enabled, state,
                    sharedGlyph, numChars),
  PopupType(poptype),
  PopFlags(0),
  ArrowWidth((uint)-1)
{
}

//
// GetDesiredSize
//
void
TPopupButtonGadget::GetDesiredSize(TSize& size)
{
  TButtonTextGadget::GetDesiredSize(size);

  if (PopupType != Normal)
    size.cx += Arr2F(ArrowWidth, size.cx);
}

//
void
TPopupButtonGadget::SetBounds(const TRect& rect)
{
  TButtonTextGadget::SetBounds(rect);

  BitmapOrigin.x = BitmapOrigin.x - F2Arr(ArrowWidth, rect.Width())/2;
}

//
//
//
void
TPopupButtonGadget::Paint(TDC& dc)
{
  PRECONDITION(Window);

  int arrowwidth = F2Arr(ArrowWidth, Bounds.Width());
  bool xpstyle = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatXPTheme;

  if (PopupType == DownArrowAction && !xpstyle)
    Bounds.right -= arrowwidth;

  PaintBorder(dc);

  if (PopupType == DownArrowAction && !xpstyle)
    Bounds.right += arrowwidth;

  TRect  faceRect, textRect, btnRect;
  GetInnerRect(faceRect);

  if (PopupType != Normal)
    faceRect.right -= arrowwidth;

  Layout(faceRect, textRect, btnRect);

  if(Style&sText)
    PaintText(dc, textRect, Text);
  if(Style&sBitmap)
    PaintFace(dc, btnRect);

  if (PopupType != Normal)
    faceRect.right += arrowwidth;

  if (PopupType == DownArrowAction)
    PaintArrowButton(dc);

   if (PopupType != Normal)
    PaintArrow(dc, faceRect);
}


// Paint the separator between button and down arrow
//
void
TPopupButtonGadget::PaintArrowButton(TDC& dc)
{
  PRECONDITION (GetGadgetWindow());

  TRect boundsRect = TRect(TPoint(0,0),Bounds.Size());
  boundsRect.left = boundsRect.right - F2Arr(ArrowWidth, Bounds.Width());

  bool xpstyle = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatXPTheme;
  if (xpstyle)
  {
    bool ok = GetEnabled();
    bool down = ok && PopFlags & ArrowPressed;
    bool hover = ok && (IsHaveMouse() || GetGadgetWindow()->GadgetGetCaptured() == this);
    uint flags = TUIBorder::Left | TUIBorder::Flat;
    if (down || hover) 
      TUIBorder(boundsRect, TUIBorder::RaisedInner, flags).Paint(dc);
  }
  else // !xpstyle
  {
  bool flat = GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard;

  // Paint pressed/raised border around down arrow
  //
    if (flat)
    {
      if (GetEnabled())
      {
        TUIBorder::TEdge edgeStyle = (TUIBorder::TEdge) 0;
    if(PopFlags&ArrowPressed)
          edgeStyle = TUIBorder::SunkenOuter;
        else if (IsHaveMouse() || GetGadgetWindow()->GadgetGetCaptured() == this)
          edgeStyle = TUIBorder::RaisedInner;
        TUIBorder(boundsRect, edgeStyle).Paint(dc);
      }
    }
    else // !flat
    {
      TUIBorder::TEdge edgeStyle = (PopFlags & ArrowPressed) ? 
        TUIBorder::EdgeSunken : TUIBorder::EdgeRaised;
      TUIBorder(boundsRect, edgeStyle, TUIBorder::Soft).Paint(dc);
    }
  }
}

// Paint the down arrow
//
void
TPopupButtonGadget::PaintArrow(TDC& dc, const TRect& innerRect)
{
  PRECONDITION (Window);

  // Paint pressed border around down arrow
  //
  bool flat = GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard;

  TPoint dstPt(innerRect.right - F2Arr(ArrowWidth,Bounds.Width())/2 - (flat?1:2),
               innerRect.top + (innerRect.Height()-4)/2);

  if(PopFlags&ArrowPressed)// || Pressed)
    dstPt.x++,dstPt.y++;

  //Y.B!!! Have to be rewritten
  // Paint down arrow
  TColor col = TColor::SysBtnText;
  if (!GetEnabled())
    col = TColor::Sys3dShadow;
  TPen pen3(col);
  dc.SelectObject(pen3);
  dc.MoveTo(dstPt.x, dstPt.y);
  dc.LineTo(dstPt.x+5, dstPt.y);
  dc.MoveTo(dstPt.x+1, dstPt.y+1);
  dc.LineTo(dstPt.x+4, dstPt.y+1);
  dc.SetPixel(dstPt.x+2, dstPt.y+2, col);
  if (!GetEnabled() && !flat) {
    dc.SetPixel(dstPt.x+2, dstPt.y+3, TColor::Sys3dHilight);
    dc.SetPixel(dstPt.x+3, dstPt.y+2, TColor::Sys3dHilight);
    dc.SetPixel(dstPt.x+4, dstPt.y+1, TColor::Sys3dHilight);
  }
  dc.RestorePen();
}


// Invoked by mouse-down & mouse enter events. sets member data "Pressed"
// to true and highlights the button
//
void
TPopupButtonGadget::BeginPressed(const TPoint& p)
{
  PRECONDITION (GetGadgetWindow());
  if(PopupGuard)
    return;

  if (PopupType != DownArrowAction) {
    TButtonTextGadget::BeginPressed(p);
    if (TrackMouse)
      GetGadgetWindow()->GadgetReleaseCapture(*this);
    PopupGuard = true;
    PopupAction();
    PopupGuard = false;
  }
  else {
    if ((PopFlags&HasCapture) == 0 &&
        (PopFlags&ArrowPressed) == 0 &&
        TRect(Bounds.right-Bounds.left-F2Arr(ArrowWidth, Bounds.Width()), 0,
              Bounds.right-Bounds.left,Bounds.bottom-Bounds.top).Contains(p)){

      Pressed = false;
      SetInMouse(true);
      PopFlags |= ArrowPressed;
      Invalidate();
      Update();
      if(TrackMouse)
        GetGadgetWindow()->GadgetReleaseCapture(*this);
      PopupGuard = true;
      PopupAction();
      PopupGuard = false;
    }
    else
      TButtonTextGadget::BeginPressed(p);
  }
}

// Invoked by mouse exit events. sets member data "Pressed" to false and
// paints the button in its current state
//
void
TPopupButtonGadget::CancelPressed(const TPoint& p, bool inmouse)
{
  if(PopupGuard)
    return;

  if (Pressed)
    TButtonTextGadget::CancelPressed(p, inmouse);
}

// Captures the mouse if "TrackMouse" is set.
//
void
TPopupButtonGadget::LButtonDown(uint modKeys, const TPoint& point)
{
  if(PopupGuard)
    return;

  TButtonTextGadget::LButtonDown(modKeys, point);
  if (Pressed)
    PopFlags |= HasCapture;
}

// Releases the mouse capture if "TrackMouse" is set.
//
void
TPopupButtonGadget::LButtonUp(uint modKeys, const TPoint& point)
{
  if(PopupGuard)
    return;
  TButtonTextGadget::LButtonUp(modKeys, point);
  PopFlags &= ~HasCapture;
}

// Start the PopupAction if the user click
// 1) into the button if PopupType != DownArrowAction
// 2) into the arrow  if PopupType == DownArrowAction
// This function only restore button state, overwrite them with functionality,
// but don't forget to call inherited one
//
void
TPopupButtonGadget::PopupAction()
{
  if(PopFlags&ArrowPressed){
    PopFlags &= ~ArrowPressed;
    Invalidate();
    Update();
  }
  else{
     TPoint p;
     CancelPressed(p, true);
   }
}


//
// class TPopupMenuGadget
// ~~~~~ ~~~~~~~~~~~~~~~~
//

uint TPopupMenuGadget::Flags = TPM_LEFTALIGN | TPM_LEFTBUTTON;

//
//
//
TPopupMenuGadget::TPopupMenuGadget(TMenu& menu, TWindow* window, int id,
  TResId glyphResIdOrIndex, TStyle style, TPopupType poptype, TType type,
  bool enabled, TState state, bool sharedGlyph, uint numChars)
:
  TPopupButtonGadget(id, glyphResIdOrIndex,style, poptype, type, enabled, state,
                     sharedGlyph, numChars),
  CmdTarget(window)
{
  PopupMenu = new TPopupMenu(menu);
}

//
// Delete the allocated popup menu.
//
TPopupMenuGadget::~TPopupMenuGadget()
{
  delete PopupMenu;
}

//
//
//
void
TPopupMenuGadget::PopupAction()
{
  TRect rect = GetBounds();

  TPoint p1(rect.TopLeft());
  Window->ClientToScreen(p1);

  TPoint p2(rect.BottomRight());
  Window->ClientToScreen(p2);
  TPMPARAMS tpm;
  tpm.cbSize             = sizeof(TPMPARAMS);
  tpm.rcExclude.top     = p1.y;
  tpm.rcExclude.left     = 0;
  tpm.rcExclude.bottom   = p2.y;
  tpm.rcExclude.right   = 32000;
  ::TrackPopupMenuEx(*PopupMenu, Flags, p1.x, p1.y, *CmdTarget, &tpm);

  if(!IsWindow(Window->GetHandle()))
    return;

  TPopupButtonGadget::PopupAction();
}

} // OWL namespace
//==============================================================================

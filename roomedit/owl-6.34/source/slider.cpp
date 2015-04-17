//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TSlider, slider UI widget abstract base class.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/slider.h>
#include <owl/dc.h>
#include <owl/commctrl.h>

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TSlider, TScrollBar)
  EV_WM_HSCROLL,
  EV_WM_VSCROLL,
END_RESPONSE_TABLE;

//
/// Constructs a slider object setting Pos and ThumbRgn to 0, TicGap to Range
/// divided by 10, SlotThick to 17, Snap to true, and Sliding to false. Sets Attr.W
/// and Attr.H to the values in X and Y. ThumbResId is set to thumbResId.
//
TSlider::TSlider(TWindow* parent, int id, int x, int y, int w, int h, TResId thumbResId, TModule* module)
:
  TScrollBar(parent, id, x, y, w, h, true, module),
  ThumbResId(thumbResId),
  ThumbRect(0, 0, 0, 0)  // This will get setup when bitmap is loaded.
{  
  SetRange(0, 100);
  Pos = 0;
  ThumbRgn = 0;
  TicGap = Range/10;  // Setup 10 evenly spaced tics by default, no array
  Tics = 0;
  TicCount = 0;
  SlotThick = 4;      // Default for all sliders
  Snap = true;
  SelStart = SelEnd = 0;

  Sliding = false;
}

//
/// Constructor for a TSlider object created from resource
//
TSlider::TSlider(TWindow* parent, int resId, TResId thumbResId, TModule* module)
:
  TScrollBar(parent, resId, module),
  ThumbResId(thumbResId),
  ThumbRect(0, 0, 0, 0)  // This will get setup when bitmap is loaded.
{
  SetRange(0, 100);
  Pos = 0;
  ThumbRgn = 0;
  TicGap = Range/10;  // Setup 10 evenly spaced tics by default, no array.
  Tics = 0;
  TicCount = 0;
  SlotThick = 4;      // Default for all sliders
  Snap = true;
  SelStart = SelEnd = 0;

  Sliding = false;
}

//
/// Constructs a slider object to encapsulate (alias) an existing control.
//
TSlider::TSlider(THandle hWnd, TModule* module)
:
  TScrollBar(hWnd, module),
  ThumbResId(0),
  ThumbRect(0, 0, 0, 0)  // This will get setup when bitmap is loaded.
{
  SetRange(SendMessage(TBM_GETRANGEMIN), SendMessage(TBM_GETRANGEMAX)); 
  Pos = SendMessage(TBM_GETPOS);
  GetClassName(); 

  ThumbRgn = 0;
  TicGap = Range/10;  // Setup 10 evenly spaced tics by default, no array.
  Tics = 0;
  TicCount = 0;
  SlotThick = 4;      // Default for all sliders
  Snap = TicGap > 0;
  SelStart = SelEnd = 0;

  Sliding = false;
}

//
/// Destructs a TSlider object and deletes ThumbRgn.
//
TSlider::~TSlider()
{
  delete[] Tics;
  delete ThumbRgn;
}

//
/// Checks and sets the slider range.
/// Sets the slider to the range between minValue and maxValue. Overloads TScrollBar's virtual
/// function.
//
void
TSlider::SetRange(int minValue, int maxValue, bool redraw)
{
  Min = minValue;
  Max = maxValue;
  if (Max > Min)
    Range = Max - Min;
  else if (Min > Max)
    Range = Min - Max;
  else
    Range = 1;

  if (GetHandle()) {
//    TParam2 p2 = (Attr.Style & TBS_VERT) ? MkParam2(maxValue, minValue) : MkParam2(minValue, maxValue);
    TParam2 p2 = MkParam2(minValue, maxValue);
    SendMessage(TBM_SETRANGE, redraw, p2);  // Swapped for vertical
  }
}

//
/// Sets the position of the thumb and always redraws.
/// Moves the thumb to the position specified in thumbPos. If thumbPos is outside
/// the present range of the slider, the thumb is moved to the closest position
/// within the specified range. Overloads TScrollBar's virtual function.
///
/// Always redraws.
//
void
TSlider::SetPosition(int pos)
{
  SetPosition(pos, true);
}

//
/// Sets the position of the thumb and always redraws.
/// Moves the thumb to the position specified in thumbPos. If thumbPos is outside
/// the present range of the slider, the thumb is moved to the closest position
/// within the specified range. Overloads TScrollBar's virtual function.
///
/// Redraw is optional.
//
void
TSlider::SetPosition(int pos, bool redraw)
{
  // Constrain pos to be in the range "Min .. Max" & snap to tics if enabled
  //
  pos = SnapPos(pos);

  // Slide thumb to new position, converting pos to pixels
  //
  if (GetHandle()) {
    SendMessage(TBM_SETPOS, redraw, pos);
  }
  Pos = pos;
}

//
/// Sets the slider's ruler. Each slider has a built-in ruler that is drawn with the
/// slider. The ruler, which can be blank or have tick marks on it, can be created
/// so that it forces the thumb to snap to the tick positions automatically.
/// \note Snapping is not supported in native currently
//
void
TSlider::SetRuler(int ticGap, bool snap)
{
  TicGap = ticGap;
  Snap = snap;
  delete[] Tics;
  Tics = 0;

  if (GetHandle()) {
    SendMessage(TBM_SETTICFREQ, ticGap, 0);
  }
}

//
/// Sets the ruler's custom tics and snap. Snapping is not currently supported in
/// native.
//\ todo need to add multple tics support
//
void
TSlider::SetRuler(int tics[], int ticCount, bool snap)
{
  PRECONDITION(tics || ticCount == 0);  // A 0 tics array is only OK if no tics

  // Alloc the array if the size is different, or we dont have one. Then copy
  // the tic positions
  //
  if (ticCount > TicCount || !Tics) {
    delete[] Tics;
    Tics = ticCount ? new int[ticCount] : 0;
  }
  for (int i = 0; i < ticCount; i++)
    if (tics[i] >= Min && tics[i] <= Max)  // Ignore out of range tics
      Tics[i] = tics[i];
  TicCount = ticCount;

  Snap = snap;

  if (GetHandle()) {
    SendMessage(TBM_CLEARTICS, false);
    for (int i = 0; i < TicCount; i++)
      SendMessage(TBM_SETTIC, i, tics[i]);
  }
}

//
/// Set a selection range for the slider. Requires that TBS_ENABLESELRANGE style
/// attribute be set.
//
void
TSlider::SetSel(int start, int end, bool redraw)
{
  // Save selection state
  //
  SelStart = start;
  SelEnd = end;

  if (GetHandle()) {
    SendMessage(TBM_SETSEL, redraw, MkUint32(uint16(start), uint16(end)));
  }
}

//
// Get the selection range from the slider.
// Requires that TBS_ENABLESELRANGE style attribute be set.
//
void
TSlider::GetSel(int& start, int& end)
{
  if (GetHandle()) {
    start = (int)SendMessage(TBM_GETSELSTART);
    end = (int)SendMessage(TBM_GETSELEND);
  }
  // Resync selection state
  //
  SelStart = start;
  SelEnd = end;
}

//----------------------------------------------------------------------------
// Protected implementation

//
/// Returns the windows system class name that this slider is basing itself on.
//
TWindow::TGetClassNameReturnType
TSlider::GetClassName()
{
  PRECONDITION(TCommCtrl::IsAvailable());
  return TRACKBAR_CLASS;

}

//
/// Calls TScrollBar::SetupWindow and SetupThumbRgn to set up the slider window.
//
void
TSlider::SetupWindow()
{
  TScrollBar::SetupWindow();


    SetRange(Min, Max);
    if (Tics)
      SetRuler(Tics, TicCount, Snap);
    else
      SetRuler(TicGap, Snap);
    SetSel(SelStart, SelEnd, false);
    SetPosition(Pos, true);

}

//
/// Constrains pos so it is in the range from Min to Max and (if snapping is
/// enabled) performs snapping by rounding pos to the nearest TicGap.
//
int
TSlider::SnapPos(int pos)
{
  if (pos > Max)
    pos = Max;

  else if (pos < Min)
    pos = Min;

  return Snap && TicGap > 0 ? (((pos-Min)+TicGap/2)/TicGap)*TicGap + Min : pos;
}

//
/// Overrides the handling in TScrollBar.
/// In particular, TSlider does not support the code in TScrollBar to retrieve 32-bit positions, 
/// so we here simply resort to the 16-bit position value passed with the TB_THUMBPOSITION and
/// TB_THUMBTRACK notifications.
//
// TODO: For Windows Vista and later, handle the TRBN_THUMBPOSCHANGING notification instead. 
// This new notification carries 32-bit data in the accompanying structure NMTRBTHUMBPOSCHANGING.
//
void
TSlider::EvHScroll(uint scrollCode, uint thumbPos, THandle hCtl)
{
  PRECONDITION(hCtl == GetHandle()); // Only handle messages for ourselves.
  switch (scrollCode) 
  {
    case TB_THUMBPOSITION: 
      SBThumbPosition(thumbPos); 
      break;

    case TB_THUMBTRACK:
      SBThumbTrack(thumbPos); 
      break;

    default: 
      TScrollBar::EvHScroll(scrollCode, thumbPos, hCtl);
  }
}

//
/// Overrides the handling in TScrollBar.
/// See EvHScroll for details.
//
void
TSlider::EvVScroll(uint scrollCode, uint thumbPos, THandle hCtl)
{
  // Simply forward to EvHScroll, since the code is identical.
  //
  EvHScroll(scrollCode, thumbPos, hCtl);
}


IMPLEMENT_ABSTRACT_STREAMABLE1(TSlider, TScrollBar);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TSlider::Streamer::Read(ipstream& is, uint32 version) const
{
  TSlider* o = GetObject();
  ReadBaseObject((TScrollBar*)o, is);
  is >> o->Min
     >> o->Max
     >> o->Pos
     >> o->ThumbResId
     >> o->ThumbRect;
  if (version == 1) {
    TRect tmpRect;
    is >> tmpRect;     // dummy CaretRect for compatibilty with stream v1
  }
  is >> o->TicGap
     >> o->Snap;

  // In stream version 2 and above, a tic array with count is written, as well
  // as a selection range
  //
  if (version >= 2) {
    is >> o->TicCount;
    o->Tics = o->TicCount ? new int[o->TicCount] : 0;
    for (int i = 0; i < o->TicCount; i++)
      is >> o->Tics[i];

    is >> o->SelStart
       >> o->SelEnd;
  }

  o->SetRange(o->Min, o->Max);  // let it calculate Range
  return o;
}

//
//
//
void
TSlider::Streamer::Write(opstream& os) const
{
  TSlider* o = GetObject();
  WriteBaseObject((TScrollBar*)o, os);
  os << o->Min
     << o->Max
     << o->Pos
     << o->ThumbResId
     << o->ThumbRect
     << o->TicGap
     << o->Snap
     << o->TicCount;

  // In stream version 2 and above, a tic array with count is written, as well
  // as a selection range
  //
  os << o->TicCount;
  for (int i = 0; i < o->TicCount; i++)
    os << o->Tics[i];
  int d1,d2;
  o->GetSel(d1,d2);     // Force retrieval of current selection before writing
  os << o->SelStart
     << o->SelEnd;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


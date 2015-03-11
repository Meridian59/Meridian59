//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Implementation of class TFlatScroller.
//------------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/fscroll.h>
#include <owl/window.h>

namespace owl {

OWL_DIAGINFO;

namespace
{

  SCROLLINFO FlatSB_GetScrollInfo_(const TWindow* w, int bar, uint mask = SIF_ALL)
  {
    PRECONDITION(w && w->GetHandle());
    SCROLLINFO i = {sizeof(SCROLLINFO), mask};
    bool r = TCommCtrl::Dll()->FlatSB_GetScrollInfo(w->GetHandle(), bar, &i);
    CHECK(r); InUse(r);
    return i;
  }

  int FlatSB_SetScrollInfo_(const TWindow* w, int bar, SCROLLINFO& i, bool redraw)
  {
    PRECONDITION(w && w->GetHandle());
    return TCommCtrl::Dll()->FlatSB_SetScrollInfo(w->GetHandle(), bar, &i, redraw);
  }

} // namespace

//
// constructs a TScroller object
//
TFlatScroller::TFlatScroller(TWindow* window,
                             int      xUnit,
                             int      yUnit,
                             long     xRange,
                             long     yRange)
:
  TScroller(window,xUnit,yUnit,xRange,yRange)
{
}

//
// destructs a TFlatScroller object
//
TFlatScroller::~TFlatScroller()
{
}


void
TFlatScroller::SetScrollPage(int bar, int page, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_PAGE | SIF_DISABLENOSCROLL, 0, 0, page};
  FlatSB_SetScrollInfo_(Window, bar, i, redraw);
}


int
TFlatScroller::GetScrollPage(int bar) const
{
  return FlatSB_GetScrollInfo_(Window, bar, SIF_PAGE).nPage;
}


//
void
TFlatScroller::GetScrollRange(int bar, int& minPos, int& maxPos) const
{
  SCROLLINFO i = FlatSB_GetScrollInfo_(Window, bar, SIF_RANGE);
  minPos = i.nMin;
  maxPos = i.nMax;
}


//
void
TFlatScroller::SetScrollRange(int bar, int minPos, int maxPos, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_RANGE | SIF_DISABLENOSCROLL, minPos, maxPos};
  FlatSB_SetScrollInfo_(Window, bar, i, redraw);
}


//
int
TFlatScroller::GetScrollPos(int bar) const
{
  return FlatSB_GetScrollInfo_(Window, bar, SIF_POS).nPos;
}


//
int
TFlatScroller::SetScrollPos(int bar, int pos, bool redraw)
{
  SCROLLINFO i = {sizeof(SCROLLINFO), SIF_POS | SIF_DISABLENOSCROLL, 0, 0, 0, pos};
  return FlatSB_SetScrollInfo_(Window, bar, i, redraw);
}

//
int
TFlatScroller::GetScrollTrackPos(int bar) const
{
  return FlatSB_GetScrollInfo_(Window, bar, SIF_TRACKPOS).nTrackPos;
}


IMPLEMENT_STREAMABLE1(TFlatScroller, TScroller);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TFlatScroller from the passed ipstream
//
void*
TFlatScroller::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TScroller*)GetObject(), is);

  return GetObject();
}

//
// writes the TFlatScroller to the passed opstream
//
void
TFlatScroller::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TScroller*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */


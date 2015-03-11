//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1997 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of Pane Splitter classes
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/contain.h>
#include <owl/layoutwi.h>
#include <owl/panespli.h>
#include <owl/uihelper.h>
#include <owl/dc.h>
#include <owl/uimetric.h>
#include <owl/template.h>

#include <math.h>

namespace owl {

OWL_DIAGINFO;


//
// Defines to make typesafe downcasting simpler.
//
#define SPLITTER(x) TYPESAFE_DOWNCAST(x,TSplitter)
#define LAYOUTWINDOW(x) TYPESAFE_DOWNCAST(x,TLayoutWindow)
//#define USE_CUSTOM_CURSORS


#if !defined(BI_COMP_GNUC)
#  pragma warn -inl
#endif
void TSplitterIndicatorList::Flush(bool del)
{
  if(del){
    for(TSplitterIndicatorListIterator iter(*this); iter; iter++)
      delete *iter;
  }
  TBaseList<TSplitterIndicator*>::Flush();
}
#if !defined(BI_COMP_GNUC)
#  pragma warn .inl
#endif
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// TSplitter class.
//

DEFINE_RESPONSE_TABLE1(TSplitter, TLayoutWindow)
  EV_WM_LBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONUP,
  EV_WM_SETCURSOR,
  EV_WM_SIZE,
END_RESPONSE_TABLE;

//
// Constructor for abstract base class - don't set a border.
//
TSplitter::TSplitter(TWindow* parent, TPaneSplitter* ps, float percent)
:
  TLayoutWindow(parent, 0, 0),
  PercentOf(percent),
  PaneSplitter(ps)
{
  Attr.Style &= ~WS_BORDER;
  Attr.Style |= WS_CLIPCHILDREN;
}

//
// Notify TPaneSplitter object that it should begin the splitter move process.
//
void
TSplitter::EvLButtonDown(uint /*modKeys*/, const TPoint& point)
{
  TPoint  p = point;
  MapWindowPoints(0, &p, 1);  // map to screen
  PaneSplitter->StartSplitterMove(this, p);
//  TLayoutWindow::EvLButtonDown(modKeys, point);
}

//
// Notify TPaneSplitter object that splitter has moved.
//
void
TSplitter::EvMouseMove(uint /*modKeys*/, const TPoint& point)
{
  TPoint  p = point;
  MapWindowPoints(0, &p, 1);  // map to screen
  PaneSplitter->MouseMoved(p);
//  TLayoutWindow::EvMouseMove(modKeys, point);
}

//
// Notify TPaneSplitter object that splitters have finished being moved.
//
void
TSplitter::EvLButtonUp(uint /*modKeys*/, const TPoint& point)
{
  TPoint  p = point;
  MapWindowPoints(0, &p, 1);  // map to screen
  PaneSplitter->EndSplitterMove(p);
//JRSTSplitter::EvLButtonUp(uint /*modKeys*/, TPoint& /*point*/)
//JRS{
//JRS  PaneSplitter->EndSplitterMove();
//  TLayoutWindow::EvLButtonUp(modKeys, point); // !CQ we may be deleted!
}

//
// Notify TPaneSplitter object that it should set the appropriate cursor.
//
bool
TSplitter::EvSetCursor(THandle hWndCursor, uint hitTest, uint mouseMsg)
{
  TPoint p;
  if (hWndCursor == GetHandle() && hitTest == HTCLIENT) {
    GetCursorPos(p);
    PaneSplitter->SetSplitterMoveCursor(this, p);
  }
  else
    TLayoutWindow::EvSetCursor(hWndCursor, hitTest, mouseMsg);
  return true;
}

//
// Notify TPaneSplitter object that splitter needs to be repainted.
// This allows the user to draw the splitter.
//
void
TSplitter::Paint(TDC& dc, bool, TRect& rect)
{
  TLayoutWindow::Paint(dc, true, rect);
  PaneSplitter->DrawSplitter(dc, GetRect());
}

//
// Resize the splitter.  In certain situations the splitters that are
// nested in this splitter don't want to resize, in which case their
// sizes are adjusted (retained).
//
void
TSplitter::EvSize(uint sizeType, const TSize& size)
{
  if (sizeType != SIZE_MINIMIZED && PaneSplitter->PaneSplitterResizing)
    AdjForResize(size);
  TLayoutWindow::EvSize(sizeType, size);
}

//
// Common code for T*Splitter::Setup().  Sets pane's parent to this and
// sets their layout metrics.
//
void
TSplitter::SetupEpilog(TSplitter* s, TWindow* targetPane, TWindow* newPane,
                       TLayoutMetrics& lmOfTargetPane,
                       TLayoutMetrics& lmOfNewPane)
{
  targetPane->SetParent(s);
  newPane->SetParent(s);
  newPane->Create();
  SetChildLayoutMetrics(*targetPane, lmOfTargetPane);
  SetChildLayoutMetrics(*newPane, lmOfNewPane);
}

//
// Remove a pane from this splitter.
//
TLayoutWindow*
TSplitter::RemovePane(TWindow* pane, TShouldDelete::TDelete dt)
{
  // 'parentSplitter' represents the splitter 'this' splitter is contained in.
  // 'retval' indicates weather their is a parent splitter, if not then 'pane'
  // is only one.
  // 'otherPane' is other window contained in this splitter.
  //
  TSplitter*        parentSplitter = SPLITTER(GetParentO());
  TLayoutWindow*    retval = parentSplitter;
  TWindow*          otherPane = Pane1() == pane ? Pane2() : Pane1();

  TLayoutMetrics    lm1;
  TLayoutMetrics    lm2;

  pane->ShowWindow(SW_HIDE);
  if (parentSplitter) {
    TWindow*          p1 = parentSplitter->Pane1();
    TWindow*          p2 = parentSplitter->Pane2();

    parentSplitter->GetChildLayoutMetrics(*p1, lm1);
    parentSplitter->GetChildLayoutMetrics(*p2, lm2);
    pane->SetParent(0);               // remove pane.
    otherPane->SetParent(parentSplitter);   // move up one level.

    if (this == p1) {                 // if top or left pane is being removed.
      if (lm2.X.RelWin == this)       // adjust other panes layout metrics.
        lm2.X.RelWin = otherPane;
      else if (lm2.Y.RelWin == this)
        lm2.Y.RelWin = otherPane;
      parentSplitter->SetChildLayoutMetrics(*p2, lm2);
      parentSplitter->SetChildLayoutMetrics(*otherPane, lm1);
    }
    else {  // other pane's layout is same as one being removed.
            //
      parentSplitter->SetChildLayoutMetrics(*otherPane, lm2);
    }
  }
  else {                          // remove last pane.
    retval = LAYOUTWINDOW(GetParentO());
    TLayoutMetrics  lmOfOtherPane;

    PaneSplitter->GetDefLM(lmOfOtherPane);
    otherPane->SetParent(retval);
    pane->SetParent(0);
    retval->SetChildLayoutMetrics(*otherPane, lmOfOtherPane);
  }
  // Destroy pane.
  //
  RemoveChildLayoutMetrics(*pane);
  PaneSplitter->DestroyPane(pane, dt);
  return retval;
}

//----------------------------------------------------------------------------
// TVSplitter class.
//

//
// Constructor for a vertical splitter
//
TVSplitter::TVSplitter(TWindow* parent, TPaneSplitter* ps, float percent)
:
  TSplitter(parent, ps, percent)
{
  SetCaption(_T("VSplitter"));
}

//
// Return TRect for the visible area (in client coordinates) of splitter.
//
TRect
TVSplitter::GetRect()
{
  TRect     rect;
  TWindow*  p1 = Pane1();

  rect.left = p1->GetWindowAttr().X + p1->GetWindowAttr().W;
  rect.top = p1->GetWindowAttr().Y;
  rect.right = rect.left + PaneSplitter->GetSplitterWidth();
  rect.bottom = p1->GetWindowAttr().Y + p1->GetWindowAttr().H;

  return rect;
}

//
// Split given pane into 2 parts: itself and a new pane.
//
void
TVSplitter::Split(TWindow* targetPane, TWindow* newPane, TSplitDirection sd, float percent)
{
  // Panes contained in this splitter.
  //
  TWindow*        p1 = Pane1();
  TWindow*        p2 = Pane2();
  TLayoutMetrics  lm;

  GetChildLayoutMetrics(*p2, lm);

  // Create new splitter (will be a child of this splitter).
  //
  TSplitter* splitter;
  if (sd == psHorizontal)
    splitter = new THSplitter(this, PaneSplitter, percent);
  else
    splitter = new TVSplitter(this, PaneSplitter, percent);

  // Initialize new splitter by putting target and new pane into it.
  //
  splitter->Create();
  TLayoutMetrics lmOfSplitter = splitter->Setup(targetPane, newPane, percent);

  if (p1 == targetPane) {   // target pane == left pane.  adjust new splitter's
                            // width and other windows left edge to be
                            // adjacent to new splitter.
                            //
    lmOfSplitter.Width.Absolute(targetPane->GetWindowAttr().W);

    lm.X.RelWin = splitter;
    SetChildLayoutMetrics(*p2, lm);
  }
  else {                    // set new splitter to the right of left pane.
                            //
    lmOfSplitter.X.RightOf(p1, PaneSplitter->GetSplitterWidth());
  }
  SetChildLayoutMetrics(*splitter, lmOfSplitter);
}

//
// Setup layout metrics and children for new splitter. Return that
// layout metrics.
//
TLayoutMetrics
TVSplitter::Setup(TWindow* targetPane, TWindow* newPane, float percent)
{
  TLayoutMetrics  lmOfSplitter;
  TLayoutMetrics  lmOfPane1;
  TLayoutMetrics  lmOfPane2;

  PaneSplitter->GetDefLM(lmOfSplitter);
  lmOfPane1 = lmOfPane2 = lmOfSplitter;

  // Target pane's width is 1/2 of what it was.  New pane is to the right
  // of target pane.
  //
  lmOfPane1.Width.Absolute(static_cast<int>(targetPane->GetWindowAttr().W * percent));
  lmOfPane2.X.RightOf(targetPane, PaneSplitter->GetSplitterWidth());
  SetupEpilog(this, targetPane, newPane, lmOfPane1, lmOfPane2);

  return lmOfSplitter;
}

//
// Create splitter indicator for this splitter.
//
TSplitterIndicator*
TVSplitter::CreateSplitterIndicator()
{
  return new TVSplitterIndicator(this, GetScreenRect());
}

//
// Return left pane, or 0 if none exist.
//
TWindow*
TVSplitter::Pane1()
{
  TWindow*  p1 = GetFirstChild();
  if (p1) {
    TWindow*  p2 = p1->Next();

    return p1->GetWindowAttr().X <= p2->GetWindowAttr().X ? p1 : p2;
  }
  return 0;
}

//
// Return right pane, or 0 if none exist.
//
TWindow*
TVSplitter::Pane2()
{
  if (NumChildren() != 2)
    return 0;

  TWindow*  p1 = GetFirstChild();
  TWindow*  p2 = p1->Next();

  return p1->GetWindowAttr().X > p2->GetWindowAttr().X ? p1 : p2;
}

//
// Called as a result of parent being resized. Adjust width of pane 1 to make
// it appear this splitter didn't move.
//
void
TVSplitter::AdjForResize(const TSize& sz)
{
  TLayoutMetrics     lm;
  TWindow*           pane = Pane1();

  if (pane) {
    GetChildLayoutMetrics(*pane, lm);
    lm.Width.Absolute(static_cast<int>(GetPercent() * sz.cx)); //JJH added static cast
    SetChildLayoutMetrics(*pane, lm);
  }
}

//
// Move splitter given distance.  Accomplished by resizing pane 1.
//
void
TVSplitter::Move(int dist)
{
  TLayoutMetrics  lm;
  TWindow*        pane = Pane1();

  if (pane) {
    GetChildLayoutMetrics(*pane, lm);
    lm.Width.Absolute(lm.Width.Value + dist);
    SetChildLayoutMetrics(*pane, lm);

    // Calc new percent.
    //
    PercentOf = (float)lm.Width.Value / (float)Attr.W;

    // Update nested splitters to retain their positions.
    //
    TSplitter* splitter = SPLITTER(Pane2());
    if (splitter) {
      while (1) {
        if (splitter->SplitDirection() == SplitDirection()) {
          TLayoutMetrics lm;
          splitter->GetChildLayoutMetrics(*splitter->Pane1(), lm);
          lm.Width.Absolute(lm.Width.Value - dist);
          splitter->SetPercent((float)lm.Width.Value /
                               (float)(splitter->GetWindowAttr().W - dist));
          splitter->SetChildLayoutMetrics(*splitter->Pane1(), lm);
        }
        if ((splitter = SPLITTER(splitter->Pane1())) == 0)
          break;
      }
    }
  }
}

//
// Change the width of the splitter and then do a Layout() to see changes.
//
void
TVSplitter::AdjSplitterWidth(int w)
{
  TLayoutMetrics  lm;

  GetChildLayoutMetrics(*Pane2(), lm);
  lm.X.Value += w;
  SetChildLayoutMetrics(*Pane2(), lm);
  Layout();
}

//----------------------------------------------------------------------------
// THSplitter class.
//

//
// Constructor for a horizontal splitter
//
THSplitter::THSplitter(TWindow* parent, TPaneSplitter* ps, float percent)
:
  TSplitter(parent, ps, percent)
{
  SetCaption(_T("HSplitter"));
}

//
// Return TRect for the visible area (in client coordinates) of splitter.
//
TRect
THSplitter::GetRect()
{
  TRect     rect;
  TWindow*  p1 = Pane1();

  rect.left = p1->GetWindowAttr().X;
  rect.top = p1->GetWindowAttr().Y + p1->GetWindowAttr().H;
  rect.right = p1->GetWindowAttr().X + p1->GetWindowAttr().W;
  rect.bottom = rect.top + PaneSplitter->GetSplitterWidth();

  return rect;
}

//
// Split given pane into 2 parts: itself and a new pane.
//
void
THSplitter::Split(TWindow* targetPane, TWindow* newPane, TSplitDirection sd, float percent)
{
  // Panes contained in this splitter.
  //
  TWindow*       p1 = Pane1();
  TWindow*       p2 = Pane2();
  TLayoutMetrics lm;

  GetChildLayoutMetrics(*p2, lm);

  // Create new splitter (will be a child of this splitter).
  //
  TSplitter* splitter;
  if (sd == psHorizontal)
    splitter = new THSplitter(this, PaneSplitter, percent);
  else
    splitter = new TVSplitter(this, PaneSplitter, percent);

  // Initialize new splitter by putting target and new pane into it.
  //
  splitter->Create();
  TLayoutMetrics lmOfSplitter = splitter->Setup(targetPane, newPane, percent);

  if (p1 == targetPane) {   // target pane == top pane.  adjust new splitter's
                            // height and other windows top edge to be
                            // adjacent to new splitter.
                            //
    lmOfSplitter.Height.Absolute(targetPane->GetWindowAttr().H);

    lm.Y.RelWin = splitter;
    SetChildLayoutMetrics(*p2, lm);
  }
  else {                    // set new splitter to below top pane.
                            //
    lmOfSplitter.Y.Below(p1, PaneSplitter->GetSplitterWidth());
  }
  SetChildLayoutMetrics(*splitter, lmOfSplitter);
}

//
// Setup layout metrics and children for new splitter. Return that
// layout metrics.
//
TLayoutMetrics
THSplitter::Setup(TWindow* targetPane, TWindow* newPane, float percent)
{
  TLayoutMetrics  lmOfSplitter;
  TLayoutMetrics  lmOfPane1;
  TLayoutMetrics  lmOfPane2;

  PaneSplitter->GetDefLM(lmOfSplitter);
  lmOfPane1 = lmOfPane2 = lmOfSplitter;

  // Target pane's height is 1/2 of what it was.  New pane is below
  // target pane.
  //
  lmOfPane1.Height.Absolute(static_cast<int>(targetPane->GetWindowAttr().H * percent)); //JJH added static cast
  lmOfPane2.Y.Below(targetPane, PaneSplitter->GetSplitterWidth());
  SetupEpilog(this, targetPane, newPane, lmOfPane1, lmOfPane2);

  return lmOfSplitter;
}

//
// Create splitter indicator for this splitter.
//
TSplitterIndicator*
THSplitter::CreateSplitterIndicator()
{
  return new THSplitterIndicator(this, GetScreenRect());
}

//
// Return top pane, or 0 if none exist.
//
TWindow*
THSplitter::Pane1()
{
  TWindow* p1 = GetFirstChild();

  if (p1) {
    TWindow*  p2 = p1->Next();

    return p1->GetWindowAttr().Y <= p2->GetWindowAttr().Y ? p1 : p2;
  }
  return 0;
}

//
// Return bottom pane, or 0 if none exist.
//
TWindow*
THSplitter::Pane2()
{
  if (NumChildren() != 2)
    return 0;

  TWindow* p1 = GetFirstChild();
  TWindow* p2 = p1->Next();

  return p1->GetWindowAttr().Y > p2->GetWindowAttr().Y ? p1 : p2;
}

//
// Called as a result of parent being resized. Adjust height of pane 1 to make
// it appear this splitter didn't move.
//
void
THSplitter::AdjForResize(const TSize& sz)
{
  TLayoutMetrics lm;
  TWindow*       pane = Pane1();

  if (pane) {
    GetChildLayoutMetrics(*pane, lm);
    lm.Height.Absolute(static_cast<int>(GetPercent() * sz.cy)); //JJH added static cast
    SetChildLayoutMetrics(*pane, lm);
  }
}

//
// Move splitter given distance.  Accomplished by resizing pane 1.
//
void
THSplitter::Move(int dist)
{
  TLayoutMetrics  lm;
  TWindow*        pane = Pane1();

  if (pane) {
    GetChildLayoutMetrics(*pane, lm);
    lm.Height.Absolute(lm.Height.Value + dist);
    SetChildLayoutMetrics(*pane, lm);

    // Calc new percent.
    //
    PercentOf = (float)lm.Height.Value / (float)Attr.H;

    // Update nested splitters to retain there relative positions.
    //
    TSplitter* splitter = SPLITTER(Pane2());
    if (splitter) {
      while (1) {
        if (splitter->SplitDirection() == SplitDirection()) {
          TLayoutMetrics lm;
          splitter->GetChildLayoutMetrics(*splitter->Pane1(), lm);
          lm.Height.Absolute(lm.Height.Value - dist);
          splitter->SetPercent((float)lm.Height.Value /
                               (float)(splitter->GetWindowAttr().H - dist));
          splitter->SetChildLayoutMetrics(*splitter->Pane1(), lm);
        }
        if ((splitter = SPLITTER(splitter->Pane1())) == 0)
          break;
      }
    }
  }
}

//
// Change the width of the splitter and then do a Layout() to see changes.
//
void
THSplitter::AdjSplitterWidth(int w)
{
  TLayoutMetrics  lm;

  GetChildLayoutMetrics(*Pane2(), lm);
  lm.Y.Value += w;
  SetChildLayoutMetrics(*Pane2(), lm);
  Layout();
}

//----------------------------------------------------------------------------
// TSplitterIndicator class (Base class).
//

//
// Draw splitter indicator.  Use InvertRect() to create the effect.
//
void
TSplitterIndicator::Draw()
{
  if (!Showing) {
    Showing = true;
    TScreenDC screenDC;
    screenDC.InvertRect(*this);
  }
}

//----------------------------------------------------------------------------
// TVSplitterIndicator class.
//

//
// Connect either top or bottom side of indicator to either top or bottom side
// of given TRect. If top of given TRect is below indicator then connect to
// TRect's top, else if TRect is above indicator then connect to TRect's bottom.
// Assumes that given TRect represents a horizontal indicator.
//
void
TVSplitterIndicator::ConnectToRect(const TRect& rect)
{
  if (rect.top > bottom)
    bottom = rect.top;
  else if (rect.top < top)
    top = rect.bottom;
}

//
// Move splitter indicator (do not draw it though) given dist, which
// could be + (move right) or - (move left).  If the move would put the
// indicator out of the move area then move to edge of move area.
//
void
TVSplitterIndicator::Move(int dist)
{
  TRect moveArea = Splitter->GetMoveArea();
  TRect orig = Splitter->GetScreenRect();
  int   splitterWidth = right - left;

  left = orig.left + dist;
  right = left + splitterWidth;
  top = orig.top;
  bottom = orig.bottom;

  //
  // Don't allow the indicator to get closer to the edge of the moveArea
  // than the cushion.
  //
  if( moveArea.Width() > (int)Cushion )  {
    moveArea.left += Cushion;
    moveArea.right -= Cushion;
  }
  else {
    DistMoved = 0;
    return;
  }


  if (!moveArea.Contains(*this)) {
    // Indicator out of move area to put it adjacent to either top or bottom
    // of move area.
    //
    if (left < moveArea.left) {
      left = moveArea.left;
      right = left + splitterWidth;
    }
    else {
      right = moveArea.right;
      left = right - splitterWidth;
    }
  }
  else
    DistMoved = dist;
}

//
// Return distance moved (left or right) given starting and current point.
//
int
TVSplitterIndicator::CalcDistMoved(const TPoint& start, const TPoint& cur)
{
  return cur.x - start.x;
}

//
// A check to see if given point could be in indicator if indicator were
// streched.  This function is used when determining if multiple splitters
// intersect at a point (for dragging).
//
bool
TVSplitterIndicator::CouldContain(const TPoint& point)
{
  TRect r = *this;
  int   splitterWidth = right - left;

  r.top -= splitterWidth;
  r.bottom += splitterWidth;
  return r.Contains(point);
}

//
// Calculate the area in which the indicator moved.  Use original splitter
// position and current indicator position to calculate area. Used to
// determine which panes need to be removed.
//
TRect
TVSplitterIndicator::CalcAreaOfSplitterMove()
{
  TRect r1 = *this;
  TRect r2 = Splitter->GetScreenRect();
  TRect area = r2;

  if (r1.left < r2.left) {
    area.left = r1.left;
    area.right = r2.right;
  }
  else {
    area.left = r2.left;
    area.right = r1.right;
  }
  return area;
}

//----------------------------------------------------------------------------
// THSplitterIndicator class.
//

//
// Connect either left or right side of indicator to either left or right side
// of given TRect. If left side of given TRect is to the right of indicator
// then connect to TRect's left side, else if TRect is to the left of indicator
// then connect to TRect's right side. Assumes that given TRect represents a
// vertical indicator.
//
void
THSplitterIndicator::ConnectToRect(const TRect& rect)
{
  if (rect.left > right)
    right = rect.left;
  else if (rect.left < left)
    left = rect.right;
}

//
// Move splitter indicator (do not draw it though) given dist, which
// could be + (move down) or - (move up).  If the move would put the
// indicator out of the move area then move to edge of move area.
//
void
THSplitterIndicator::Move(int dist)
{
  TRect moveArea = Splitter->GetMoveArea();
  TRect orig = Splitter->GetScreenRect();
  int   splitterWidth = bottom - top;

  top = orig.top + dist;
  bottom = top + splitterWidth;
  left = orig.left;
  right = orig.right;

  //
  // Don't allow the indicator to get closer to the edge of the moveArea
  // than the cushion.
  //
  if( moveArea.Height() > (int)Cushion )  {
    moveArea.top += Cushion;
    moveArea.bottom -= Cushion;
  }
  else {
    DistMoved = 0;
    return;
  }

  if (!moveArea.Contains(*this)) {
    // Indicator out of move area to put it adjacent to either top or bottom
    // of move area.
    //
    if (top < moveArea.top) {
      top = moveArea.top;
      bottom = top + splitterWidth;
    }
    else {
      bottom = moveArea.bottom;
      top = bottom - splitterWidth;
    }
  }
  else
    DistMoved = dist;
}

//
// Return distance moved (up or down) given starting and current point.
//
int
THSplitterIndicator::CalcDistMoved(const TPoint& start, const TPoint& cur)
{
  return cur.y - start.y;
}

//
// A check to see if given point could be in indicator if indicator were
// streched.  This function is used when determining if multiple splitters
// intersect at a point (for dragging).
//
bool
THSplitterIndicator::CouldContain(const TPoint& point)
{
  TRect r = *this;
  int   splitterWidth = bottom - top;

  r.left -= splitterWidth;
  r.right += splitterWidth;

  return r.Contains(point);
}

//
// Calculate the area in which the indicator moved.  Use original splitter
// position and current indicator position to calculate area. Used to
// determine which panes need to be removed.
//
TRect
THSplitterIndicator::CalcAreaOfSplitterMove()
{
  TRect r1 = *this;
  TRect r2 = Splitter->GetScreenRect();
  TRect area = r2;

  if (r1.top < r2.top) {
    area.top = r1.top;
    area.bottom = r2.bottom;
  }
  else {
    area.top = r2.top;
    area.bottom = r1.bottom;
  }
  return area;
}

//----------------------------------------------------------------------------
// TSplitterIndicatorList class.

//
// Search for indicator that was created from given splitter.
//
TSplitterIndicator*
TSplitterIndicatorList::FindIndicatorWithSplitter(TSplitter* splitter)
{
  for(TSplitterIndicatorListIterator  iter(*this); iter; iter++){
    TSplitterIndicator* i = (TSplitterIndicator*)*iter;
    if (i->GetSplitter() == splitter)
      return i;
  }
  return 0;
}

//----------------------------------------------------------------------------
// TSplitterIndicatorMgr class.
//

//
// Save indicator list for later manipulation.  Record starting drag point.
// Draw splitter indicators.
//
void
TSplitterIndicatorMgr::StartMove(TSplitterIndicatorList& sil,
                                 const TPoint& point)
{
  SplitterIndicatorList = &sil;
  StartDragPoint = point;
  DrawIndicators();
}

//
// Clear indicators from screen.
//
void
TSplitterIndicatorMgr::EndMove()
{
  ClearIndicators();
}

//
// Move all indicators.  Distance moved is based on starting drag point and
// given point.
//
void
TSplitterIndicatorMgr::MoveIndicators(const TPoint& point)
{
  ClearIndicators();

  TSplitterIndicatorListIterator  iter(*SplitterIndicatorList);

  if (iter){
    TSplitterIndicator* main = (TSplitterIndicator*)*iter;
    while(iter) {
      TSplitterIndicator* si = (TSplitterIndicator*)*iter;
      int dist = si->CalcDistMoved(StartDragPoint, point);
      si->Move(dist);
      if (si != main)
        si->ConnectToRect(*main);
      iter++;
    }
  }
  DrawIndicators();
}

//
// Draw indicators on screen.
//
void
TSplitterIndicatorMgr::DrawIndicators()
{
  if( !SplitterIndicatorList )
    return;
  for(TSplitterIndicatorListIterator iter(*SplitterIndicatorList);iter;iter++)
    ((TSplitterIndicator*)*iter)->Draw();
}

//
// Clear all indicators from screen.
//
void
TSplitterIndicatorMgr::ClearIndicators()
{
  if( !SplitterIndicatorList )
    return;
  for(TSplitterIndicatorListIterator iter(*SplitterIndicatorList);iter;iter++)
    ((TSplitterIndicator*)*iter)->Clear();
}

//----------------------------------------------------------------------------
// TPaneSplitter class
//

DEFINE_RESPONSE_TABLE1(TPaneSplitter, TLayoutWindow)
  EV_WM_SIZE,
END_RESPONSE_TABLE;


//
/// Initializes data members.
//
TPaneSplitter::TPaneSplitter(TWindow*        parent,
                             LPCTSTR         title,
                             int             splitterWidth,
                             TModule*        module)
:
  TLayoutWindow(parent, title, module),
  Dragging(false),
  SplitterWidth(splitterWidth),
  SplitterCushion(0),
  PaneSplitterResizing(false),
  ShouldDelete(TShouldDelete::NoDelete)
{
  Attr.Style |= WS_CLIPCHILDREN;
  if (!SplitterWidth)
    SplitterWidth = TUIMetric::CxFixedFrame;
  SetCaption(_T("PaneSplitter"));
   SplitterIndicatorList = new TSplitterIndicatorList;

}

//
/// String-aware overload
//
TPaneSplitter::TPaneSplitter(
  TWindow* parent,
  const tstring& title,
  int splitterWidth,
  TModule* module
  )
  : TLayoutWindow(parent, title, module),
  Dragging(false),
  SplitterWidth(splitterWidth),
  SplitterCushion(0),
  PaneSplitterResizing(false),
  ShouldDelete(TShouldDelete::NoDelete)
{
  Attr.Style |= WS_CLIPCHILDREN;
  if (!SplitterWidth)
    SplitterWidth = TUIMetric::CxFixedFrame;
  SetCaption(_T("PaneSplitter"));
  SplitterIndicatorList = new TSplitterIndicatorList;
}

//
// Empty destructor
//
TPaneSplitter::~TPaneSplitter()
{
  SplitterIndicatorList->Flush(true);
  delete SplitterIndicatorList;
}

//
/// Loads cursors and sets TSplitter's pane splitter object.
//
void
TPaneSplitter::SetupWindow()
{
  TLayoutWindow::SetupWindow();

  // Load cursors.
  //
#if defined(USE_CUSTOM_CURSORS)
  ResizeCursorH = LoadCursor(IDC_RESIZE_H);
  ResizeCursorV = LoadCursor(IDC_RESIZE_V);
  ResizeCursorHV = LoadCursor(IDC_RESIZE_HV);
#else
  ResizeCursorH = ::LoadCursor(0, IDC_SIZENS);
  ResizeCursorV = ::LoadCursor(0, IDC_SIZEWE);
  ResizeCursorHV = ::LoadCursor(0, IDC_SIZEALL);
#endif
}

//
/// Removes all panes (and related splitters). Destroys cursors.
//
void
TPaneSplitter::CleanupWindow()
{
  RemoveAllPanes();

#if defined(USE_CUSTOM_CURSORS)
  ::DestroyCursor(ResizeCursorH);
  ::DestroyCursor(ResizeCursorV);
  ::DestroyCursor(ResizeCursorHV);
#endif

  TLayoutWindow::CleanupWindow();
}

//
/// Sets data member 'PaneSplitterResizing' to indicate that contained splitters
/// should adjust themselves according to their percentage.
//
void
TPaneSplitter::EvSize(uint sizeType, const TSize& size)
{
  PaneSplitterResizing = true;
  TLayoutWindow::EvSize(sizeType, size);
  PaneSplitterResizing = false;
}

//
/// Called by TSplitter when it needs to draw itself. The default behavior provided
/// in this base class is to draw a 3dface splitter.
//
void
TPaneSplitter::DrawSplitter(TDC& dc, const TRect& splitter)
{
  dc.TextRect(splitter, TColor::Sys3dFace);
}

//
/// Splits given pane, 'target', with 'newPane', in either the vertical or
/// horizontal direction. Creates a new splitter.
//
bool
TPaneSplitter::SplitPane(TWindow* target, TWindow* newPane,
                         TSplitDirection splitDir, float percent)
{
  PRECONDITION(target);
  PRECONDITION(target != newPane);

  if (GetFirstChild() == 0) {     // if no panes.
    TLayoutMetrics  lmOfTarget;

    GetDefLM(lmOfTarget);
    target->SetParent(this);
    target->Create();
    SetChildLayoutMetrics(*target, lmOfTarget);
    Layout();
  }

  if (newPane) {
    if (!HasPane(target))
      return false;

    TSplitter*      splitter;
    TLayoutWindow*  parentSplitter = LAYOUTWINDOW(target->GetParentO());

    if (parentSplitter == this) {   // if first split.
      if (splitDir == psHorizontal)
        splitter = new THSplitter(target->GetParentO(), this, percent);
      else
        splitter = new TVSplitter(target->GetParentO(), this, percent);
      splitter->Create();
      splitter->SetPercent(percent);

      TLayoutMetrics lm = splitter->Setup(target, newPane, percent);
      parentSplitter->SetChildLayoutMetrics(*splitter, lm);
    }
    else {
      // Ask splitter to split itself.
      //
      SPLITTER(parentSplitter)->Split(target, newPane, splitDir, percent);
    }
    parentSplitter->Layout();
  }
  return true;
}

//
/// Removes given pane from TPaneSplitter, deleting it if requested.
/// This operation has the side effect of removing the splitter it was
/// contained in.
//
bool
TPaneSplitter::RemovePane(TWindow* pane, TPaneSplitter::TDelete dt)
{
  TLayoutWindow* layoutWin = DoRemovePane(pane, dt);
  if (layoutWin) {
    PaneSplitterResizing = true;
    layoutWin->Layout();
    PaneSplitterResizing = false;
    return true;
  }
  return false;
}

//
/// Replaces 'target' pane (must exist) with 'newPane' (does not exist). 'target'
/// may be deleted; it depends on 'dt'.
//
bool
TPaneSplitter::ReplacePane(TWindow* target, TWindow* newPane,
                           TPaneSplitter::TDelete dt)
{
  if (!HasPane(target) || HasPane(newPane)) // 'newPane' should not exist.
    return false;

  if (target->CanClose()) {     // 'newPane' takes on target's layout metrics.
    TLayoutMetrics  lmOfTarget;
    TLayoutMetrics  lmOfOther;
    TLayoutWindow*  splitter = LAYOUTWINDOW(target->GetParentO());
    TWindow*        pane1 = splitter->GetFirstChild();
    TWindow*        pane2 = 0;

    if (SPLITTER(splitter)) {
      pane1 = SPLITTER(splitter)->Pane1();
      pane2 = SPLITTER(splitter)->Pane2();
    }
    splitter->GetChildLayoutMetrics(*target, lmOfTarget);

    if (pane1 == target && pane2) {
      splitter->GetChildLayoutMetrics(*pane2, lmOfOther);
      if (lmOfOther.X.RelWin == target)
        lmOfOther.X.RelWin = newPane;
      else
        lmOfOther.Y.RelWin = newPane;
      splitter->SetChildLayoutMetrics(*pane2, lmOfOther);
    }
    newPane->SetParent(splitter);
    newPane->Create();
    splitter->SetChildLayoutMetrics(*newPane, lmOfTarget);
    splitter->RemoveChildLayoutMetrics(*target);
    DestroyPane(target, dt);
    splitter->Layout();
    return true;
  }
  return false;
}

//
/// Swaps given panes (must exist). Panes take on each others' layout metrics.
//
bool
TPaneSplitter::SwapPanes(TWindow* pane1, TWindow* pane2)
{
  if (!HasPane(pane1) || !HasPane(pane2))
    return false;

  TSplitter* splitter1 = SPLITTER(pane1->GetParentO());
  TSplitter* splitter2 = SPLITTER(pane2->GetParentO());

  if (splitter1 != splitter2) {
    TLayoutMetrics  lmOfPane1;
    TLayoutMetrics  lmOfPane2;
    TLayoutMetrics  lmOfOther;
    TWindow*        paneA = splitter1->Pane1();
    TWindow*        paneB = splitter1->Pane2();

    splitter1->GetChildLayoutMetrics(*pane1, lmOfPane1);
    splitter2->GetChildLayoutMetrics(*pane2, lmOfPane2);

    if (paneA == pane1) {   // if top or left pane.
      splitter1->GetChildLayoutMetrics(*paneB, lmOfOther);
      pane1->SetParent(0);
      if (lmOfOther.X.RelWin == pane1)
        lmOfOther.X.RelWin = pane2;
      else
        lmOfOther.Y.RelWin = pane2;
      splitter1->SetChildLayoutMetrics(*paneB, lmOfOther);
    }

    paneA = splitter2->Pane1();
    paneB = splitter2->Pane2();
    if (paneA == pane2) {   // if top or left pane.
      splitter2->GetChildLayoutMetrics(*paneB, lmOfOther);
      pane2->SetParent(0);
      if (lmOfOther.X.RelWin == pane2)
        lmOfOther.X.RelWin = pane1;
      else
        lmOfOther.Y.RelWin = pane1;
      splitter2->SetChildLayoutMetrics(*paneB, lmOfOther);
    }
    pane1->SetParent(splitter2);
    pane2->SetParent(splitter1);
    splitter1->SetChildLayoutMetrics(*pane2, lmOfPane1);
    splitter2->SetChildLayoutMetrics(*pane1, lmOfPane2);
    splitter1->Layout();
    splitter2->Layout();
  }
  else {
    TLayoutWindow*  lw = LAYOUTWINDOW(pane1->GetParentO());
    TLayoutMetrics  lmOfPane1;
    TLayoutMetrics  lmOfPane2;

    lw->GetChildLayoutMetrics(*pane1, lmOfPane1);
    lw->GetChildLayoutMetrics(*pane2, lmOfPane2);

    if (lmOfPane1.X.RelWin == pane2)
      lmOfPane1.X.RelWin = pane1;
    else if (lmOfPane2.X.RelWin == pane1)
      lmOfPane2.X.RelWin = pane2;
    else if (lmOfPane1.Y.RelWin == pane2)
      lmOfPane1.Y.RelWin = pane1;
    else
      lmOfPane2.Y.RelWin = pane2;

    lw->SetChildLayoutMetrics(*pane1, lmOfPane2);
    lw->SetChildLayoutMetrics(*pane2, lmOfPane1);
    lw->Layout();
  }
  return true;
}

//
/// Iterates over each pane. If 'callback' returns 0, then iteration stops.
//
void
TPaneSplitter::ForEachPane(TPaneSplitter::TForEachPaneCallback callback,
                           void* p)
{
  ForEachObject(GetFirstChild(), &TPaneSplitter::DoForEachPane,
                (void*)callback, p);  // !CQ hack cast
}

//
/// Returns the number of panes in TPaneSplitter.
//
int
TPaneSplitter::PaneCount()
{
  int nPanes = 0;

  ForEachObject(GetFirstChild(), &TPaneSplitter::DoPaneCount, &nPanes, 0);
  return nPanes;
}

//
/// Removes all the panes (and their splitters). If any pane can't close, the
/// operation is aborted.
//
void
TPaneSplitter::RemoveAllPanes(TDelete dt)
{
  TBaseList<TWindow*>    wList;

  // Remove and delete (depending on 'dt') all panes.
  //
  ForEachObject(GetFirstChild(), &TPaneSplitter::GetPanes, &wList, 0);
  TBaseList<TWindow*>::Iterator wListIter(wList);

  // check to see if a pane can't close, if so abort operation.
  //
  while(wListIter){
    if (!(*wListIter)->CanClose())
      return;
    wListIter++;
  }
  wListIter.Restart();
  while (wListIter){    // remove all the panes.
    RemovePane(*wListIter, dt);
    wListIter++;
  }
}

//
/// Sets the splitter width and adjusts all splitters. Takes effect immediately.
//
int
TPaneSplitter::SetSplitterWidth(int newWidth)
{
  int oldWidth = SplitterWidth;
  int widthDiff = newWidth - SplitterWidth;

  SplitterWidth = newWidth;
  ForEachObject(GetFirstChild(), &TPaneSplitter::AdjSplitterWidth, &widthDiff,
                0);
  return oldWidth;
}

//
/// Moves 'pane' a given distance. If 'pane' does not exist, or there are no
/// splitters, or removal of a pane has failed, then false is returned. Otherwise,
/// true is returned.
//
bool
TPaneSplitter::MoveSplitter(TWindow* pane, int dist)
{
  if (!HasPane(pane) || pane->GetParentO() == this )
    return false;

  TSplitter*    splitter = SPLITTER(pane->GetParentO());
  TSplitterIndicator* si = splitter->CreateSplitterIndicator();
  si->SetCushion( SplitterCushion );

  SplitterIndicatorList->Flush(true);
  SplitterIndicatorList->PushBack(si);
  si->Move(dist);
  if (RemovePanes() == 0) {
    MoveSplitters();
    SplitterIndicatorList->Flush(true);
    return true;
  }
  return false;
}

//
// Private Member Functions...
//

//
// Notify indicator mgr to move the indicators.
//
void
TPaneSplitter::MouseMoved(const TPoint& point)
{
  if (Dragging)
    SplitterIndicatorMgr.MoveIndicators(point);
}

//
// Set capture (for mouse dragging) and notify indicator mgr that
// we are starting to move the indicators.
//
void
TPaneSplitter::StartSplitterMove(TSplitter* splitter, const TPoint& point)
{
  Dragging = true;
  splitter->SetCapture();
  SplitterIndicatorMgr.StartMove(*SplitterIndicatorList, point);
}

//
// Release the capture and notify indicator mgr that we have stopped moving
// the indicators.  Remove any panes that were 'covered' by the move.
// Finally, move the splitters.
//
void
TPaneSplitter::EndSplitterMove(const TPoint& /*point*/)
{
  if( !Dragging )
    return;
  Dragging = false;
  ReleaseCapture();
  SplitterIndicatorMgr.EndMove();
  if (RemovePanes() == 0) {
    MoveSplitters();
    SplitterIndicatorList->Flush(true);
  }
}

//
// Set the appropriate cursor depending on the number of splitters
// that will be moved.
//
void
TPaneSplitter::SetSplitterMoveCursor(TSplitter* splitter, const TPoint& point)
{
  if (!Dragging)
    FindIntersectingSplitters(point);

  if (SplitterIndicatorList->NSplitterIndicators() > 1)
    ::SetCursor(ResizeCursorHV);
  else if (splitter->SplitDirection() == psHorizontal)
    ::SetCursor(ResizeCursorH);
  else
    ::SetCursor(ResizeCursorV);
}

//
// Private structure for splitter and pane traversal.
//
struct TTraversalRec {
  TTraversalRec() : Cnt(0), Object(0) {}
  TTraversalRec(TWindow* o) : Cnt(0), Object(o) {}
  TTraversalRec(const TTraversalRec& r) : Cnt(r.Cnt), Object(r.Object) {}

  int operator == (const TTraversalRec& r) const { return Object == r.Object; }
  int operator < (const TTraversalRec& r) const { return Object < r.Object; }
  TTraversalRec& operator = (const TTraversalRec& r);

  char              Cnt;
  TWindow*          Object;
};

TTraversalRec&
TTraversalRec::operator = (const TTraversalRec& r)
{
  Cnt = r.Cnt; Object = r.Object;
  return *this;
}

//
// Iterate over the splitters and panes, calling 'callback' on each.
// allows for 2 parameters to be passed.  Traverses the objects in
// one of three order: InOrder, PreOrder and PostOrder.
//
void
TPaneSplitter::ForEachObject(TWindow* o, TForEachObjectCallback callback,
                             void* p1, void* p2, TTraversalOrder order)
{
  if (!o)
    return;

  TBaseList<TTraversalRec> tStack;

  TTraversalRec         trec;
  bool                  done = false;

  tStack.Push(TTraversalRec(o));
  while (!done) {
    trec = tStack.Pop();
    //trec = tStack.Top();
    //delete tStack.Pop();
    trec.Cnt++;
    if ((order == psPreOrder && trec.Cnt == 1) ||
        (order == psInOrder && trec.Cnt == 2)  ||
        (order == psPostOrder && trec.Cnt == 3))
      done = !(this->*callback)(trec.Object, p1, p2);

    if (!done) {
      if (trec.Cnt != 3) {
        tStack.Push(TTraversalRec(trec));
        if (trec.Cnt == 1 && SPLITTER(trec.Object))
          tStack.Push(TTraversalRec(SPLITTER(trec.Object)->Pane1()));
        else if (trec.Cnt == 2 && SPLITTER(trec.Object) &&
                 SPLITTER(trec.Object)->Pane2())
          tStack.Push(TTraversalRec(SPLITTER(trec.Object)->Pane2()));
      }
      done = tStack.Empty();
    }
  }
}

//
// Set default TLayoutMetrics.  Everything relative to parent.
//
void
TPaneSplitter::GetDefLM(TLayoutMetrics& lm)
{
  lm.SetMeasurementUnits(lmPixels);
  lm.X.SameAs(lmParent, lmLeft);
  lm.Y.SameAs(lmParent, lmTop);
  lm.Width.Set(lmRight, lmSameAs, lmParent, lmRight);
  lm.Height.Set(lmBottom, lmSameAs, lmParent, lmBottom);
}

//
// Return true if this TPaneSplitter contains 'p' (pane or splitter).
//
bool
TPaneSplitter::HasPane(TWindow* p)
{
  TWindow* parent = p->GetParentO();
  while (parent) {
    if (parent == this)
      return true;
    parent = parent->GetParentO();
  }
  return false;
}

//
// Do the work of removing given pane.
//
TLayoutWindow*
TPaneSplitter::DoRemovePane(TWindow* pane, TPaneSplitter::TDelete dt)
{
  if (!HasPane(pane) || !pane->CanClose())
    return 0;

  TSplitter*        splitter = SPLITTER(pane->GetParentO());
  TLayoutWindow*    retval = this;

  if (splitter) {
    retval = splitter->RemovePane(pane, dt);
    splitter->Destroy();
    delete splitter;
  }
  else {
    RemoveChildLayoutMetrics(*pane);
    DestroyPane(pane, dt);
  }
  return retval;
}

//
// Find intersecting splitters.  Called when splitters will be moved.
//
void
TPaneSplitter::FindIntersectingSplitters(const TPoint& point)
{
  TPoint p = point;

  SplitterIndicatorList->Flush(true);
  ForEachObject(GetFirstChild(), &TPaneSplitter::DoFindIntersectingSplitters,
                SplitterIndicatorList, &p);
}

//
// Remove any panes 'covered' as a result of a splitter move.
//
int
TPaneSplitter::RemovePanes()
{
  TSplitterIndicatorList          tempList;   // don't process indicators.
  TSplitterIndicatorListIterator  iter(*SplitterIndicatorList);
  TSplitterIndicator*             si;
  int                             nPanesNotRemoved = 0;

  while(iter){
    si = iter++;

    if(!tempList.Find(si)){   // if indicator not in list then process it.
      TRect               area = si->CalcAreaOfSplitterMove();
      TBaseList<TWindow*> wList; // list of panes to remove.
      ForEachObject(GetFirstChild(), &TPaneSplitter::GetListOfPanesToRemove,
                    &wList, &area);
      TBaseList<TWindow*>::Iterator listIter(wList);
      while(listIter){
        TWindow* pane = listIter++;
        TSplitterIndicator* i;
        if((i = SplitterIndicatorList->FindIndicatorWithSplitter(
              SPLITTER(pane->GetParentO()))) != 0)
          tempList.PushBack(i);
        if (!RemovePane(pane)) {
          tempList.PushBack(i);
          nPanesNotRemoved++;
          break;
        }
      }
    }
  }
  // remove panes not to be moved from list.
  for(TSplitterIndicatorListIterator iter1(tempList);iter1;iter1++){
    delete *iter1;
    SplitterIndicatorList->DetachItem(iter1.Current());
  }

  return nPanesNotRemoved;
}

//
// Move all splitters.
//
void
TPaneSplitter::MoveSplitters()
{
  for(TSplitterIndicatorListIterator iter(*SplitterIndicatorList);iter;iter++){
    TSplitterIndicator* si = iter.Current();
    si->GetSplitter()->Move(si->GetDistMoved());
    si->GetSplitter()->Layout();
  }
}

//
// Destroy a pane, deleting the OWL object if requested
//
void
TPaneSplitter::DestroyPane(TWindow* pane, TDelete dt)
{
  pane->ShowWindow(SW_HIDE);    // so user can't see.
  pane->SetParent(0);
  pane->Destroy();
  if (DelObj(dt))
    delete pane;
}

//
// ForEachObject callbacks...
//

//
// Iterate over all panes.
//
int
TPaneSplitter::DoForEachPane(TWindow* o, void *p1, void*p2)
{
  if (!SPLITTER(o))
    return ((TForEachPaneCallback)p1)(*o, p2);
  return 1;
}

//
// Find intersecting splitters. Determine which splitters to move.
//
int
TPaneSplitter::DoFindIntersectingSplitters(TWindow* o, void *p1, void*p2)
{
  TSplitter* splitter = SPLITTER(o);
  if (splitter) {
    TSplitterIndicator* i = splitter->CreateSplitterIndicator();
    i->SetCushion( SplitterCushion );
    if (i->CouldContain(*(TPoint*)p2))
      ((TSplitterIndicatorList*)p1)->PushBack(i);
    else
      delete i;
  }
  return 1;
}

//
// Get a list of panes that will be removed as a result of a splitter move.
//
int
TPaneSplitter::GetListOfPanesToRemove(TWindow* o, void *p1, void*p2)
{
  if (!SPLITTER(o)) {
    TRect r = o->GetWindowRect();
    if ((*(TRect*)p2).Contains(r))
      ((TBaseList<TWindow*>*)p1)->PushBack(o);
  }
  return 1;
}

//
// Change the width of every splitter.
//
int
TPaneSplitter::AdjSplitterWidth(TWindow* o, void *p1, void*)
{
  TSplitter* splitter = SPLITTER(o);

  if (splitter)
    splitter->AdjSplitterWidth(*(int *)p1);
  return 1;
}

//
// Calculate number of panes.
//
int
TPaneSplitter::DoPaneCount(TWindow* o, void *p1, void*)
{
  if (!SPLITTER(o))
    (*(int*)p1)++;
  return 1;
}

//
// Get a list of all the panes.
//
int
TPaneSplitter::GetPanes(TWindow* o, void *p1, void*)
{
  if (!SPLITTER(o))
    ((TBaseList<TWindow*>*)p1)->PushBack(o);
  return 1;
}

//
// Get a list of all splitters.
//
int
TPaneSplitter::GetSplitters(TWindow* o, void* p1, void*)
{
  if (SPLITTER(o))
    ((TBaseList<TWindow*>*)p1)->PushBack(o);
  return 1;
}

} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1995 by Borland International, All Rights Reserved
//    notebook.cpp - defines TNotebook class
//
//   Special explanation for non-client painting in TNotebook:
//
//   normal windows sequence of events for window resize:
//        WM_NCCALCSIZE
//        WM_NCPAINT
//        WM_SIZE
//
//   This causes multiple ncpaints since subclassers must hook WM_SIZE and
//   show/hide scrollbars based on new client size; these multiple ncpaints
//   are especially noticable in TNotebook since the painting of the tabs
//   takes a relative long amount of time.
//
//   Solution taken to this problem in TNotebook class: created a variable
//   called "defer_ncpaint" to turn nc painting on/off.  It is used as
//   follows:
//
//      1. Constructor: sets defer_ncpaint = 0
//      2. EvNCPaint(): if defer_ncpaint > 0, decrements it and returns
//         without painting
//      3. EvNCCalcSize(): if defer_ncpaint >= 0, increments it
//      4. EvSize(): if defer_ncpaint = 0, calls ForceResize(TRUE) to force
//         ncpaint
//      5. ForceResize(repaint): if repaint=TRUE, defer_ncpaint=-1,
//         resize window, defer_ncpaint=0
//
//   Subclassers of TNotebook::Evsize() should:
//      1. set defer_ncpaint=1
//      2. call TNotebook::Evsize(...)
//      3. call SetScrollRange(...) as needed to hide/show scrollbars
//      4. call ForceResize(TRUE) to force ncpaint
//
//   If subclassers just ignore "defer_ncpaint", TNotebook should paint
//   normally but with multiple nc paints.
//
//----------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/mdichild.h>
#include <owl/dc.h>
#include <owl/uimetric.h>

#include <owlext/notebook.h>

using namespace owl;
using namespace std;

namespace OwlExt {

//----------------------------------------------------------------------------
DEFINE_RESPONSE_TABLE1(TNotebook, TWindow)
EV_WM_GETMINMAXINFO,
EV_WM_SIZE,
EV_WM_NCCALCSIZE,
EV_WM_NCHITTEST,
EV_WM_NCPAINT,
EV_WM_NCCREATE,
EV_WM_NCLBUTTONDOWN,
EV_WM_NCLBUTTONDBLCLK,
EV_WM_LBUTTONUP,                      // while mouse captured, we get these
EV_WM_MOUSEMOVE,                      // while mouse captured, we get these
EV_WM_SYSCOLORCHANGE,
EV_MESSAGE(NW_HSCROLL, NWHscroll),
EV_MESSAGE(NW_VSCROLL, NWHscroll),
END_RESPONSE_TABLE;
//----------------------------------------------------------------------------
TNotebook::TNotebook(int TabLoc)
{
  tabloc     = TabLoc;
  tabhorz    = ((tabloc == LOC_TOP) || (tabloc == LOC_BOTTOM));
  tabcnt     = 1;
  firsttab   = 0;
  activetab  = 0;
  erasetabs  = false;
  tabhscroll = NULL;            // until created
  tabvscroll = NULL;            // until created
  winhscroll = NULL;            // until created
  winvscroll = NULL;            // until created
  hscrollon  = false;
  vscrollon  = false;
  defer_ncpaint = 0;

  ///YB nbcheight = 9 + GetSystemMetrics(SM_CYHSCROLL);
  nbcheight = 6 + TUIMetric::CyHScroll;

  rgncnt = 0;
  fillmode = ALTERNATE;
  clientprev = TRect(0, 0, 0, 0);

  BuildPaintObjects();
}
//----------------------------------------------------------------------------
void TNotebook::BuildPaintObjects()
{
  BuildBgBrush();

  framebrush   = new TBrush(GetSysColor(COLOR_WINDOWFRAME));
  hilitebrush = new TBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
  shadowbrush = new TBrush(GetSysColor(COLOR_BTNSHADOW));
  facebrush   = new TBrush(GetSysColor(COLOR_BTNFACE));
  trackbrush   = new TBrush(GetSysColor(COLOR_SCROLLBAR));
  tabbrush     = new TBrush(GetSysColor(COLOR_WINDOW));
  default_tabbrush = new TBrush(GetSysColor(COLOR_WINDOW));

  shadowpen   = new TPen(GetSysColor(COLOR_BTNSHADOW));
  framepen    = new TPen(GetSysColor(COLOR_WINDOWFRAME));
#if 0
  if (tabhorz)
    tabfont = new TFont(_T("ms sans serif"), 10);
  else
    tabfont = new TFont(_T("arial"), 10, 0, 2700, 2700);
#else
  if (tabhorz)
    tabfont = new TFont(_T("ms sans serif"), 8);
  else
    tabfont = new TFont(_T("arial"), 10, 0, 2700, 2700);
#endif
}
//----------------------------------------------------------------------------
void TNotebook::DeletePaintObjects()
{
  delete bgbrush;
  delete framebrush;
  delete hilitebrush;
  delete shadowbrush;
  delete facebrush;
  delete trackbrush;
  delete tabbrush;
  delete default_tabbrush;

  delete shadowpen;
  delete framepen;
  delete tabfont;
}
//----------------------------------------------------------------------------
TNotebook::~TNotebook()
{
  delete tabhscroll;
  delete tabvscroll;
  delete winhscroll;
  delete winvscroll;

  DeletePaintObjects();
}
//----------------------------------------------------------------------------
UINT TNotebook::EvNCHitTest(const TPoint& screenPt)
{
  TPoint winpt = screenPt - GetWindowRect().TopLeft();  // win coords

  if (nbcrect.Contains(winpt))
    return(HTBORDER);
  if ((hscrollon) && (hsbrect.Contains(winpt)))
    return(HTBORDER);
  if ((vscrollon) && (vsbrect.Contains(winpt)))
    return(HTBORDER);
  return(TWindow::EvNCHitTest(screenPt));
}
//----------------------------------------------------------------------------
void TNotebook::PaintNBC(int paintcontrol)
{
  bool goth, gotv;

  if ((!GetHandle()) || IsIconic())
    return;

  TWindowDC mydc(*this);

  DrawBackground(mydc, nbcrect, paintcontrol);
  DrawTabLines(mydc, nbcrect, TRUE);
  DrawTabs(mydc);

  if (paintcontrol == NBC_PAINTALL){
    if (tabhscroll)
      tabhscroll->ForcePaint();
    if (tabvscroll)
      tabvscroll->ForcePaint();
    if (hscrollon)
      winhscroll->ForcePaint();
    if (vscrollon)
      winvscroll->ForcePaint();
  }

  goth = (hscrollon || tabloc == LOC_BOTTOM);
  gotv = (vscrollon || tabloc == LOC_RIGHT);

  if (goth && gotv){                        // paint corner
    int mywidth, myheight;
    if(tabloc == LOC_RIGHT)
      mywidth = nbcrect.Width();
    else
      mywidth = GetSystemMetrics(SM_CXVSCROLL);
    if(tabloc == LOC_BOTTOM)
      myheight = nbcrect.Height() + 1;
    else
      myheight = GetSystemMetrics(SM_CYHSCROLL);

    if((tabloc == LOC_TOP)||(tabloc == LOC_LEFT))
      mydc.SelectObject(*trackbrush);
    else
      mydc.SelectObject(*bgbrush);

    mydc.SelectObject(*framepen);
    Rectangle(mydc, clientrect.right, clientrect.bottom,
      clientrect.right+mywidth, clientrect.bottom+myheight);
  }
}
//----------------------------------------------------------------------------
void
TNotebook::EvNCPaint(HRGN rgn)
{
  if (defer_ncpaint > 0) {
    defer_ncpaint--;
    return;
  }

  TRACE("ncpaint being done");
  TWindow::EvNCPaint(rgn); // let windows paint everything else in NC

  ComputeSizes();               // we are called before WM_SIZE is sent...
  PaintNBC(NBC_PAINTALL);
}
//----------------------------------------------------------------------------
void TNotebook::GetTabName(int tabnum, LPTSTR tabname)
{

  if (tabnum < 26){
    tabname[0] = (_TCHAR)(_T('A')+tabnum); //Cast to char to avoid 32 bit conv warnings
    tabname[1] = 0;
  }
  else if (tabnum < (702)) {
    tabnum -= 26;
    tabname[0] = (_TCHAR)(_T('A')+(tabnum/26));
    tabname[1] = (_TCHAR)(_T('A')+(tabnum%26));
    tabname[2] = 0;
  }
  else {
    tabnum -= 702;
    tabname[0] = (_TCHAR)(_T('A')+tabnum/676);
    tabnum = tabnum % 676;
    tabname[1] = (_TCHAR)(_T('A')+(tabnum/26));
    tabname[2] = (_TCHAR)(_T('A')+(tabnum%26));
    tabname[3] = 0;
  }
}
//----------------------------------------------------------------------------
void TNotebook::DrawDownFrame(TDC &mydc, TRect &myrect)
{
  int width = myrect.right - myrect.left;
  int height = myrect.bottom - myrect.top;

  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.top, width);
  FASTVLINE(mydc, myrect.left, myrect.top, height);

  mydc.SelectObject(*facebrush);
  FASTHLINE(mydc, myrect.left, myrect.bottom-1, width);
  FASTVLINE(mydc, myrect.right-1, myrect.top, height);
}
//----------------------------------------------------------------------------
void TNotebook::DrawShadeFrame(TDC &mydc, TRect &myrect)
{
  int width = myrect.right - myrect.left;
  int height = myrect.bottom - myrect.top;

  mydc.SelectObject(*hilitebrush);
  FASTHLINE(mydc, myrect.left, myrect.top, width);
  FASTVLINE(mydc, myrect.left, myrect.top, height);

  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.bottom-1, width);
  FASTVLINE(mydc, myrect.right-1, myrect.top, height);
}
//---------------------------------------------------------------------------
int TNotebook::DrawHorzTab(TDC& mydc, int tabnum, int rgnnum, int toff)
{
  _TCHAR tabname[MAXTABNAMELEN+1];
  SIZE textsize;
  int tabsize, bp;
  TRect textrect;
  POINT *points;
  int tabsheight = tabsrect.Height();
  int halfheight = tabsheight/2;
  int textextra = 10;           // extra spacing around text
  int taboff = 4;
  int ydir, row0, row1, row2, row3;

  points = tabpoints[rgnnum];
  mydc.SelectObject(*tabfont);
  SetBkMode(mydc, TRANSPARENT);

  GetTabName(tabnum, tabname);
  GetTextExtentPoint(mydc, tabname, static_cast<int>(_tcslen(tabname)),&textsize);
  tabsize = textsize.cx + textextra + taboff + taboff;
  if (toff+tabsize+2*taboff > tabsrect.right){     // no room for this one
    tabsize = 0;
    goto exit;
  }

  if (tabloc == LOC_BOTTOM) {
    row0 = tabsrect.top - 5;
    row1 = tabsrect.top;
    row2 = row1+halfheight;
    row3 = row1+tabsheight;
    ydir = 1;
    textrect = TRect(toff+taboff, row1+2, toff+taboff+textsize.cx+10, row3);
  }
  else {              // LOC_TOP
    row0 = tabsrect.bottom + 4;
    row1 = tabsrect.bottom - 1;
    row2 = row1-halfheight;
    row3 = row1-tabsheight;
    ydir = -1;
    textrect = TRect(toff+taboff, row3+2, toff+taboff+textsize.cx+10, row1);
  }

  if (erasetabs){               // erase as we draw
    mydc.SelectObject(*bgbrush);
    FASTRECT(mydc, toff+1, row2, tabsize+1, ydir*(halfheight+3));
  }

  mydc.SelectObject(*framepen);
  mydc.SelectObject(*facebrush);

  // draw tab "tabnum"
  if ((tabnum == activetab) || (! rgnnum)){
    TBrush *endbrush = shadowbrush;
    int delta = 0;
    int myrow = row1;
    if (tabnum == activetab){
      mydc.SelectObject(*tabbrush);
      endbrush = tabbrush;
      delta = 2;
      myrow = row0;
    }
    points[0] = TPoint(toff-taboff-delta, myrow);
    points[1] = TPoint(toff+taboff, row3);
    points[2] = TPoint(points[1].x+textsize.cx+textextra, row3);
    points[3] = TPoint(points[2].x+taboff+taboff+delta, myrow);
    points[4] = points[0];
    bp = 1;
    Polygon(mydc, points, 5);
    // draw top shadow line (or erase any such line for active tab)
    mydc.SelectObject(*endbrush);
    FASTHLINE(mydc, points[0].x, points[0].y, points[3].x-points[0].x-1);
  }
  else {
    points[0] = TPoint(toff+taboff, row1);
    points[1] = TPoint(toff, row2);
    points[2] = TPoint(toff+taboff, row3);
    points[3] = TPoint(points[2].x+textsize.cx+textextra, row3);
    points[4] = TPoint(points[3].x+taboff+taboff, row1);
    bp = 2;
    Polygon(mydc, points, 5);
    // draw top shadow line
    mydc.SelectObject(*shadowbrush);
    FASTHLINE(mydc, points[0].x, points[0].y, points[4].x-points[0].x-1);

    // redraw tab shadow for previous tab (Polygon erased part of it)
    mydc.SelectObject(*shadowpen);
    mydc.MoveTo(toff-taboff+1, row3+ydir);
    mydc.LineTo(toff+taboff+1, row1-ydir);
  }

  // draw tab shadow for this tab
  mydc.SelectObject(*shadowpen);
  mydc.MoveTo(points[bp].x+1, points[bp].y+ydir);
  mydc.LineTo(points[bp+1].x+1, points[bp+1].y+ydir);
  mydc.LineTo(points[bp+2].x+1, points[bp+2].y-ydir);

  // draw text inside tab
  mydc.SelectObject(*framepen);
  mydc.DrawText(tabname, -1, textrect, DT_CENTER);

exit:
  mydc.RestoreFont();
  return(tabsize);
}
//---------------------------------------------------------------------------
int TNotebook::DrawVertTab(TDC &mydc, int tabnum, int rgnnum, int toff)
{
  _TCHAR tabname[MAXTABNAMELEN+1];
  SIZE textsize;
  int tabsize, bp, xval, yval;
  TRect textrect;
  POINT *points;
  int tabswidth = tabsrect.Width();
  int halfwidth = tabswidth/2;
  int textextra = 10;           // extra spacing around text
  int taboff = 4;
  int xdir, col0, col1, col2, col3;

  points = tabpoints[rgnnum];
  mydc.SelectObject(*tabfont);
  SetBkMode(mydc, TRANSPARENT);

  GetTabName(tabnum, tabname);
  GetTextExtentPoint(mydc, tabname, static_cast<int>(_tcslen(tabname)), &textsize);
  tabsize = textsize.cx + textextra + taboff + taboff;
  if (toff+tabsize+2*taboff > tabsrect.bottom){     // no room for this one
    tabsize = 0;
    goto exit;
  }

  if (tabloc == LOC_LEFT){
    col0 = tabsrect.right + 4;
    col1 = col0-5;
    col2 = col1-halfwidth;
    col3 = col1-tabswidth;
    xdir = -1;
    textrect = TRect(col3, toff+taboff, col1-2, toff+taboff+textsize.cx+10);
  }
  else {              // LOC_RIGHT
    col0 = tabsrect.left - 5;
    col1 = col0+5;
    col2 = col1+halfwidth;
    col3 = col1+tabswidth;
    xdir = 1;
    textrect = TRect(col1, toff+taboff+2, col3-2, toff+taboff+textsize.cx+10);
  }

  if (erasetabs){               // erase as we draw
    mydc.SelectObject(*bgbrush);
    FASTRECT(mydc, col2, toff+1, xdir*(halfwidth+3), tabsize+1);
  }

  mydc.SelectObject(*framepen);
  mydc.SelectObject(*facebrush);

  // draw tab "tabnum"
  if ((tabnum == activetab) || (! rgnnum)) {
    TBrush *endbrush = shadowbrush;
    int delta = 0;
    int mycol = col1;
    if (tabnum == activetab) {
      mydc.SelectObject(*tabbrush);
      endbrush = tabbrush;
      delta = 2;
      mycol = col0;
    }
    points[0] = TPoint(mycol, toff-taboff-delta);
    points[1] = TPoint(col3, toff+taboff);
    points[2] = TPoint(col3, points[1].y+textsize.cx+textextra);
    points[3] = TPoint(mycol, points[2].y+taboff+taboff+delta);
    points[4] = points[0];
    Polygon(mydc, points, 5);
    mydc.SelectObject(*endbrush);
    // draw top shadow line (or erase any such line for active tab)
    FASTVLINE(mydc, points[0].x, points[0].y, points[3].y-points[0].y);
    bp = 1;
  }
  else {
    points[0] = TPoint(col1, toff+taboff);
    points[1] = TPoint(col2, toff);
    points[2] = TPoint(col3, toff+taboff);
    points[3] = TPoint(col3, points[2].y+textsize.cx+textextra);
    points[4] = TPoint(col1, points[3].y+taboff+taboff);
    bp = 2;
    Polygon(mydc, points, 5);
    mydc.SelectObject(*shadowbrush);
    // draw top shadow line
    FASTVLINE(mydc, points[0].x, points[0].y, points[4].y-points[0].y);
    // draw part of tab shadow for previous tab
    mydc.SelectObject(*shadowpen);
    mydc.MoveTo(points[0].x+1, points[0].y);
    mydc.LineTo(points[1].x, points[1].y+2);
  }

  // draw tab shadow for this tab
  mydc.SelectObject(*shadowpen);
  mydc.MoveTo(points[bp].x+xdir, points[bp].y);
  mydc.LineTo(points[bp+1].x+xdir, points[bp+1].y);
  mydc.LineTo(points[bp+2].x-xdir, points[bp+2].y);

  // draw text
  mydc.SelectObject(*framepen);
  xval = textrect.right;                // vert text is wierd...
  yval = textrect.top + (textrect.Height() - textsize.cx)/2;
  mydc.TextOut(xval, yval, tabname, static_cast<int>(_tcslen(tabname)));

exit:
  mydc.RestoreFont();
  return(tabsize);
}
//----------------------------------------------------------------------------
void TNotebook::DrawTabs(TDC& mydc)
{
  int tabnum = firsttab;
  int tsize, toff;

  if (tabhorz)
    toff = tabsrect.left + 22;
  else
    toff = tabsrect.top + 22;

  rgncnt = 0;

  while (1) {
    if (tabhorz)
      tsize = DrawHorzTab(mydc, tabnum, rgncnt, toff);
    else
      tsize = DrawVertTab(mydc, tabnum, rgncnt, toff);
    if (! tsize)
      break;
    toffs[rgncnt++] = toff;
    toff += tsize;
    if (rgncnt >= MAXTABSSHOWN)
      break;
    if (++tabnum >= tabcnt)
      break;
  }

  if (erasetabs) {                      // erase as we draw
    if (tabhorz)
      EraseLastHorz(mydc, toff);
    else
      EraseLastVert(mydc, toff);
  }
}
//----------------------------------------------------------------------------
void TNotebook::EraseLastVert(TDC &mydc, int toff)
{
  int tabswidth = tabsrect.Width();
  POINT points[5];
  int halfwidth = tabswidth/2;
  int col0, col1, col2, col3;
  int taboff = 4;

  if (tabloc == LOC_RIGHT) {
    col0 = tabsrect.left;
    col1 = col0 + 1;
    col2 = col0 + halfwidth;
    col3 = col0 + tabswidth + 2;
  }
  else {
    col0 = tabsrect.right;
    col1 = col0 - 1;
    col2 = col0 - halfwidth;
    col3 = col0 - (tabswidth + 2);
  }

  mydc.SelectStockObject(NULL_PEN);
  mydc.SelectObject(*bgbrush);
  toff += 2;

  points[0] = TPoint(col1, toff+taboff);
  points[1] = TPoint(col2, toff);
  points[2] = TPoint(col3, toff);
  points[3] = TPoint(col3, tabsrect.bottom+2);
  points[4] = TPoint(col1, tabsrect.bottom+2);

  Polygon(mydc, points, 5);
  erasetabs = FALSE;
}
//----------------------------------------------------------------------------
void TNotebook::EraseLastHorz(TDC &mydc, int toff)
{
  int tabsheight = tabsrect.Height();
  POINT points[5];
  int halfheight = tabsheight/2;
  int row0, row1, row2, row3;
  int taboff = 4;

  if (tabloc == LOC_BOTTOM) {
    row0 = tabsrect.top;
    row1 = row0 + 1;
    row2 = row0 + halfheight;
    row3 = row0 + tabsheight + 2;
  }
  else {
    row0 = tabsrect.bottom;
    row1 = row0 - 1;
    row2 = row0 - halfheight;
    row3 = row0 - (tabsheight + 2);
  }

  mydc.SelectStockObject(NULL_PEN);
  mydc.SelectObject(*bgbrush);
  toff += 2;

  points[0] = TPoint(toff+taboff, row1);
  points[1] = TPoint(toff, row2);
  points[2] = TPoint(toff, row3);
  points[3] = TPoint(tabsrect.right+2, row3);
  points[4] = TPoint(tabsrect.right+2, row1);

  Polygon(mydc, points, 5);
  erasetabs = FALSE;
}
//----------------------------------------------------------------------------
void TNotebook::DrawBackground(TDC &mydc, TRect &myrect, int paintcontrol)
{
  if (paintcontrol == NBC_PAINTALL)
    mydc.FillRect(myrect, *bgbrush);
  else
    erasetabs = TRUE;
}
//----------------------------------------------------------------------------
void
TNotebook::DrawTabLines(TDC& mydc, TRect& myrect, bool shadowline)
{
  int width = myrect.right - myrect.left;
  int height = myrect.bottom - myrect.top;
  int tabstart, row, col, dir;

  if (tabhorz){
    tabstart = tabsrect.left;
    if (tabloc == LOC_BOTTOM) {
      dir = 1;
      row = myrect.top;
    }
    else {
      row = myrect.bottom-1;
      dir = -1;
    }
    mydc.SelectObject(*framebrush);
    FASTHLINE(mydc, myrect.left, row, width);
    FASTVLINE(mydc, tabstart, row, dir*2);
    FASTHLINE(mydc, tabstart, row+dir*2, myrect.right-tabstart);
    FASTVLINE(mydc, tabstart+3, row+dir*2, dir*2);
    FASTHLINE(mydc, tabstart+3, row+dir*4, myrect.right-tabstart-3);
    mydc.SelectObject(*default_tabbrush);
    FASTHLINE(mydc, tabstart+1, row+dir*1, myrect.right-tabstart-1);
    FASTHLINE(mydc, tabstart+4, row+dir*3, myrect.right-tabstart-4);
    if (shadowline) {
      mydc.SelectObject(*shadowbrush);
      FASTHLINE(mydc, tabstart+5, row+dir*5, myrect.right-tabstart-5);
    }
  }
  else {              // left or right
    tabstart = tabsrect.top;

    if (tabloc == LOC_LEFT){
      col = myrect.right-1;
      dir = -1;
    }
    else {
      col = myrect.left;
      dir = 1;
    }
    mydc.SelectObject(*framebrush);
    FASTVLINE(mydc, col, myrect.top, height);
    FASTHLINE(mydc, col, tabstart, dir*2);
    FASTVLINE(mydc, col+dir*2, tabstart, myrect.bottom-tabstart);
    FASTHLINE(mydc, col+dir*2, tabstart+3, dir*2);
    FASTVLINE(mydc, col+dir*4, tabstart+3, myrect.bottom-tabstart-3);
    mydc.SelectObject(*default_tabbrush);
    FASTVLINE(mydc, col+dir, tabstart+1, myrect.bottom-tabstart-1);
    FASTVLINE(mydc, col+dir*3, tabstart+4, myrect.bottom-tabstart-4);
    if (shadowline) {
      mydc.SelectObject(*shadowbrush);
      FASTVLINE(mydc, col+dir*5, tabstart+5, myrect.bottom-tabstart-5);
    }
  }
}
//----------------------------------------------------------------------------
void
TNotebook::CalcTabRects()
{
  int nleft, ntop, nright, nbottom, nwidth, nheight;
  int hwidth, hheight, vwidth, vheight, tsblong, tsbshort;

  nleft   = nbcrect.left;
  ntop     = nbcrect.top;
  nright   = nbcrect.right;
  nbottom = nbcrect.bottom;
  nwidth   = nbcrect.right - nbcrect.left;
  nheight = nbcrect.bottom - nbcrect.top;

  // tab scrollbar's size is fixed to match font (doesn't shrink with window)
  if (tabhorz)
    tsbshort = 8 + nheight/3;
  else
    tsbshort = 8 + nwidth/3;
  tsblong = 1 + 5*(tsbshort-5);

  if(hscrollon){
    hwidth   = hsbrect.Width();
    hheight = hsbrect.Height();
  }
  else
    hheight = hwidth = 0;

  if (vscrollon){
    vwidth   = vsbrect.Width();
    vheight = vsbrect.Height();
  }
  else
    vwidth = vheight = 0;

  switch (tabloc) {
  case LOC_LEFT:
    tsbrect = TRect(nleft+2, ntop+2, nleft+2+tsbshort, ntop+2+tsblong);
    ///YB tabsrect = TRect(nleft+5,tsbrect.bottom-16,nright-5,nbottom-(5+hheight));
    tabsrect = TRect(nleft+4,tsbrect.bottom-18,nright-4,nbottom-(4+hheight));
    break;

  case LOC_RIGHT:
    tsbrect = TRect(nleft+3, ntop+3, nleft+3+tsbshort, ntop+3+tsblong);
    tabsrect = TRect(nleft+5, tsbrect.bottom-18, nright-5,nbottom-(vheight+5));
    break;

  case LOC_TOP:
    tsbrect = TRect(nleft+2, ntop+1, nleft+2+tsblong, ntop+1+tsbshort);
    tabsrect = TRect(tsbrect.right-18, ntop+3, nright-(5+vwidth),nbottom-4);
    break;

  case LOC_BOTTOM:
    ///YB tsbrect = TRect(nleft+4, ntop+9, nleft+4+tsblong,ntop+9+tsbshort);
    ///YB tabsrect = TRect(tsbrect.right-16, ntop+5, nright-(hwidth+5),nbottom-6);
    tsbrect   = TRect(nleft+2, ntop+6, nleft+2+tsblong,ntop+6+tsbshort);
    tabsrect  = TRect(tsbrect.right-18, ntop+2, nright-(hwidth+5),nbottom-4);
    break;
  }
}
//----------------------------------------------------------------------------
void TNotebook::ComputeSizes()
{
  int cleft, ctop, cright, cbottom, cwidth, cheight;
  int vswidth, hsheight;

  if ((!GetHandle()) || (clientrect == clientprev))        // no size change
    return;
  clientprev = clientrect;

  if (vscrollon)
    vswidth = GetSystemMetrics(SM_CXVSCROLL);
  else
    vswidth = 0;
  if (hscrollon)
    hsheight = GetSystemMetrics(SM_CYHSCROLL);
  else
    hsheight = 0;

  cleft   = clientrect.left;
  ctop     = clientrect.top;
  cright   = clientrect.right;
  cbottom = clientrect.bottom;
  cwidth   = clientrect.right - clientrect.left;
  cheight = clientrect.bottom - clientrect.top;

  // calculate rects: nbcrect, hsbrect, vsbrect, tsbrect, tabsrect
  // remember: overlap standalone scrollbars with surround win borders
  switch (tabloc){
  case LOC_LEFT:
    nbcrect = TRect(cleft-nbcheight, ctop, cleft, cbottom+hsheight);
    if (hscrollon)
      hsbrect = TRect(cleft-1, cbottom, cright+1, cbottom+hsheight);
    if (vscrollon)
      vsbrect = TRect(cright, ctop-1, cright+vswidth, cbottom+1);
    CalcTabRects();
    break;

  case LOC_TOP:
    nbcrect = TRect(cleft, ctop-nbcheight, cright+vswidth, ctop);
    if (hscrollon)
      hsbrect = TRect(cleft-1, cbottom, cright+1, cbottom+hsheight);
    if (vscrollon)
      vsbrect = TRect(cright, ctop-1, cright+vswidth, cbottom+1);
    CalcTabRects();
    break;

  case LOC_RIGHT:
    nbcrect = TRect(cright, ctop, cright+nbcheight, cbottom);
    if (hscrollon)
      hsbrect = TRect(cleft-1, cbottom, cright+1, cbottom+hsheight);
    if (vscrollon)
      vsbrect = TRect(cright+3, ctop+3*cheight/4, cright+3+vswidth,cbottom-2);
    CalcTabRects();
    break;

  case LOC_BOTTOM:
    ///YB nbcrect = TRect(cleft, cbottom, cright, cbottom+nbcheight);
    nbcrect = TRect(cleft, cbottom, cright, cbottom+nbcheight);
    if (hscrollon)
      hsbrect = TRect(cleft+3*cwidth/4, cbottom+7, cright-2,cbottom+7+hsheight);
    if (vscrollon)
      vsbrect = TRect(cright, ctop-1, cright+vswidth, cbottom+1);
    CalcTabRects();
    break;
  }

  if (winhscroll)
    winhscroll->ChangeRect(hsbrect, false);
  if (winvscroll)
    winvscroll->ChangeRect(vsbrect, false);
  if (tabhscroll)
    tabhscroll->ChangeRect(tsbrect, false);
  if (tabvscroll)
    tabvscroll->ChangeRect(tsbrect, false);
}
//----------------------------------------------------------------------------
uint TNotebook::EvNCCalcSize(bool calcValidRects,
               NCCALCSIZE_PARAMS & calcSize)
{
  UINT status;
  RECT  *rp;
  int vswidth, hsheight;
  TPoint wtopleft;

  // get upperleft corner of window in screen
  rp = &calcSize.rgrc[0];       // window rect in screen coords
  wtopleft = TPoint(rp->left, rp->top);

  status = TWindow::EvNCCalcSize(calcValidRects, calcSize);
  rp = &calcSize.rgrc[0];       // client rect in screen coords

  if (!IsIconic()){
    // change size of client window to allow for our area & scrollbars
    // subtrack "1" from scrollbar sizes (overlap last line with win border)
    vswidth  = GetSystemMetrics(SM_CXVSCROLL) - 1;
    hsheight = GetSystemMetrics(SM_CYHSCROLL) - 1;

    switch (tabloc) {
    case LOC_LEFT:
      rp->left += nbcheight;
      if (vscrollon)
        rp->right -= vswidth;
      if (hscrollon)
        rp->bottom -= hsheight;
      break;

    case LOC_TOP:
      rp->top += nbcheight;
      if (vscrollon)
        rp->right -= vswidth;
      if (hscrollon)
        rp->bottom -= hsheight;
      break;

    case LOC_RIGHT:
      rp->right -= nbcheight;
      if (hscrollon)
        rp->bottom -= hsheight;
      break;

    case LOC_BOTTOM:
      rp->bottom -= nbcheight;
      if (vscrollon)
        rp->right -= vswidth;
      break;
    }
    if (rp->bottom < rp->top)
      rp->bottom = rp->top;
    if (rp->right < rp->left)
      rp->right = rp->left;

    if (defer_ncpaint >= 0)
      defer_ncpaint++;    // skip next ncpaint
  }

  clientrect = TRect(*rp);
  clientrect -= wtopleft;               // win-relative client rect

  return(status);
}
//----------------------------------------------------------------------------
void TNotebook::ActivateTab(int tabnum)
{
  TRect myrect;
  int oldactive, r;

  if (activetab != tabnum){
    oldactive = activetab;
    activetab = tabnum;

    Parent->SendMessage(NB_TABCHANGED, activetab, NULL);
    if (IsIconic())
      return;

    // repaint lines
    TWindowDC mydc(*this);
    DrawTabLines(mydc, nbcrect, FALSE);

    // repaint old tab and tab before it as nonactive
    r = oldactive - firsttab - 1;
    if ((r >= 0) && (r < rgncnt)) {
      if (tabhorz)
        DrawHorzTab(mydc, oldactive-1, r, toffs[r]);
      else
        DrawVertTab(mydc, oldactive-1, r, toffs[r]);
    }
    if ((r+1 >= 0) && (r+1 < rgncnt)) {
      if (tabhorz)
        DrawHorzTab(mydc, oldactive, r+1, toffs[r+1]);
      else
        DrawVertTab(mydc, oldactive, r+1, toffs[r+1]);
    }

    // paint new active tab
    r = activetab - firsttab;
    if ((r >= 0) && (r < rgncnt)) {
      if (tabhorz)
        DrawHorzTab(mydc, activetab, r, toffs[r]);
      else
        DrawVertTab(mydc, activetab, r, toffs[r]);
    }
  }
}
//----------------------------------------------------------------------------
void TNotebook::ScrollTabs(int delta)
{
  int oldfirst;

  oldfirst = firsttab;
  firsttab += delta;

  if (firsttab < 0)
    firsttab = 0;
  else if (firsttab > tabcnt-1)
    firsttab = tabcnt-1;

  if (firsttab != oldfirst) {
    if (tabhscroll)
      tabhscroll->SetScrollPos(firsttab, TRUE);
    if (tabvscroll)
      tabvscroll->SetScrollPos(firsttab, TRUE);
    PaintNBC(NBC_PAINTTABS);
  }
}
//----------------------------------------------------------------------------
bool
TNotebook::MyLButtonDown(uint hitTest, TPoint& screenPt)
{
  int i;
  bool match;
  HRGN myrgn;

  TPoint winpt = screenPt - GetWindowRect().TopLeft();  // win coords

  if (nbcrect.Contains(winpt)) {
    // see if mouse is on tab hscrollbar
    if ((tabhscroll) && (tabhscroll->MyLButtonDown(hitTest, winpt)))
      return true;
    // see if mouse is on tab vscrollbar
    if ((tabvscroll) && (tabvscroll->MyLButtonDown(hitTest, winpt)))
      return true;

    // see if mouse down is in tabs
    for (i=rgncnt-1; i >= 0; i--) {
      myrgn = CreatePolygonRgn(tabpoints[i], 5, fillmode);
      match = PtInRegion(myrgn, winpt.x, winpt.y);
      DeleteObject(myrgn);
      if (match) {
        ActivateTab(firsttab+i);
        return true;
      }
    }
  }

  // see if mouse is on window hscrollbar
  if ((hscrollon) && (winhscroll->MyLButtonDown(hitTest, winpt)))
    return true;

  // see if mouse is on window vscrollbar
  if ((vscrollon) && (winvscroll->MyLButtonDown(hitTest, winpt)))
    return true;

  return false;
}
//----------------------------------------------------------------------------
void
TNotebook::EvNCLButtonDown(uint hitTest, const TPoint& point)
{
  TPoint p(point);
  if (MyLButtonDown(hitTest, p))
    return;
  TWindow::EvNCLButtonDown(hitTest, point);
}
//----------------------------------------------------------------------------
void
TNotebook::EvNCLButtonDblClk(uint hitTest, const TPoint& point)
{
  TPoint p(point);
  if (MyLButtonDown(hitTest, p))
    return;
  TWindow::EvNCLButtonDblClk(hitTest, point);
}
//----------------------------------------------------------------------------
void
TNotebook::EvMouseMove(uint hitTest, const TPoint& point)
{
  // convert from client to windows coords
  TPoint screenpt = MapClientToScreen(point);
  TPoint winpt = screenpt - GetWindowRect().TopLeft();  // win coords

  // see if mouse has been captured by tab hscrollbar
  if ((tabhscroll) && (tabhscroll->MyMouseMove(hitTest, winpt)))
    return;
  // see if mouse has been captured by tab vscrollbar
  if ((tabvscroll) && (tabvscroll->MyMouseMove(hitTest, winpt)))
    return;

  // see if mouse has been captured by window hscrollbar
  if ((hscrollon) && (winhscroll->MyMouseMove(hitTest, winpt)))
    return;
  // see if mouse has been captured by window vscrollbar
  if ((vscrollon) && (winvscroll->MyMouseMove(hitTest, winpt)))
    return;

  TWindow::EvNCMouseMove(hitTest, point);
}
//----------------------------------------------------------------------------
void
TNotebook::EvLButtonUp(uint hitTest, const TPoint& point)
{
  // convert from client to windows coords
  TPoint screenpt = MapClientToScreen(point);
  TPoint winpt = screenpt - GetWindowRect().TopLeft();  // win coords

  // see if mouse has been captured by tab hscrollbar
  if ((tabhscroll) && (tabhscroll->MyLButtonUp(hitTest, winpt)))
    return;
  // see if mouse has been captured by tab vscrollbar
  if ((tabvscroll) && (tabvscroll->MyLButtonUp(hitTest, winpt)))
    return;

  // see if mouse has been captured by window hscrollbar
  if ((hscrollon) && (winhscroll->MyLButtonUp(hitTest, winpt)))
    return;

  // see if mouse has been captured by window vscrollbar
  if ((vscrollon) && (winvscroll->MyLButtonUp(hitTest, winpt)))
    return;

  TWindow::EvNCLButtonUp(hitTest, point);
}
//----------------------------------------------------------------------------
void
TNotebook::EvGetMinMaxInfo(MINMAXINFO & minmaxinfo)
{
  TWindow::EvGetMinMaxInfo(minmaxinfo);
  minmaxinfo.ptMinTrackSize.x += (nbcheight+2);
  minmaxinfo.ptMinTrackSize.y += (nbcheight+2);
}
//----------------------------------------------------------------------------
void
TNotebook::SetTabCnt(int Tabcnt, int Firsttab, int Activetab)
{
  tabcnt = Tabcnt;
  firsttab = Firsttab;
  ActivateTab(Activetab);
  if (tabhscroll){
    tabhscroll->SetScrollRange(0, tabcnt, false);
    tabhscroll->SetScrollPos(activetab, true);
  }
  if (tabvscroll){
    tabvscroll->SetScrollRange(0, tabcnt, false);
    tabvscroll->SetScrollPos(activetab, true);
  }
  PaintNBC(NBC_PAINTTABS);
}
//----------------------------------------------------------------------------
bool
TNotebook::EvNCCreate(CREATESTRUCT & createStruct)
{
  long wstyle;
  bool status;

  // we will draw hscroll and vscroll if needed; tell windows not to bother
  wanthscroll = ((createStruct.style & WS_HSCROLL) != 0);
  wantvscroll = ((createStruct.style & WS_VSCROLL) != 0);

  // reset WS_HSCROLL and WH_VSCROLL in both style locations "just in case..."
  createStruct.style &= ~(WS_HSCROLL | WS_VSCROLL);
  wstyle = GetWindowLong(GWL_STYLE);
  wstyle &= ~(WS_HSCROLL | WS_VSCROLL);
  SetWindowLong(GWL_STYLE, wstyle);

  status = TWindow::EvNCCreate(createStruct);
  CreateScrollBars();
  return(status);
}
//----------------------------------------------------------------------------
void
TNotebook::EvSize(uint sizeType, const TSize& newsize)
{
  TWindow::EvSize(sizeType, newsize);
  ComputeSizes();
  if (! defer_ncpaint) {
    defer_ncpaint = 1;
    ForceResize(TRUE);
  }
}
//----------------------------------------------------------------------------
LRESULT TNotebook::NWHscroll(TParam1 wparam, TParam2 lparam)
{
  int thumbpos = LOWORD(lparam);
  int ctrlid = HIWORD(lparam);

  if (ctrlid != ID_TABSB)
    return(0);

  switch (wparam) {
  case SB_LINELEFT:
    ScrollTabs(-1);
    break;

  case SB_LINERIGHT:
    ScrollTabs(1);
    break;

  case SB_PAGELEFT:
    if (rgncnt < 2)
      ScrollTabs(-1);
    else
      ScrollTabs(-(rgncnt-1));
    break;

  case SB_PAGERIGHT:
    if (rgncnt < 2)
      ScrollTabs(1);
    else
      ScrollTabs(rgncnt-1);
    break;

  case SB_THUMBPOSITION:
    ScrollTabs(thumbpos-firsttab);
    break;
  }

  return(0);
}
//----------------------------------------------------------------------------
void TNotebook::CreateScrollBars()
{
  if (tabhorz){
    tabhscroll = new THScroll(this, 0, 0, 1, 1, ID_TABSB, 0, tabcnt-1, 0,
      true, false, true);
  }
  else{
    tabvscroll = new TVScroll(this, 0, 0, 1, 1, ID_TABSB, 0, tabcnt-1, 0,
      true, false, true);
  }

  if (wanthscroll) {
    winhscroll = new THScroll(this, 0, 0, 1, 1, ID_WINHSB, 0, 100, 0,
      false, true, false);
    hscrollon = true;
  }
  if (wantvscroll) {
    winvscroll = new TVScroll(this, 0, 0, 1, 1, ID_WINVSB, 0, 100, 0,
      false, true, false);
    vscrollon = true;
  }
}
//----------------------------------------------------------------------------
void
TNotebook::EvSysColorChange()
{
  DeletePaintObjects();
  BuildPaintObjects();

  if (tabhscroll)
    tabhscroll->SysColorChange();
  if (tabvscroll)
    tabvscroll->SysColorChange();
  if (winhscroll)
    winhscroll->SysColorChange();
  if (winvscroll)
    winvscroll->SysColorChange();

  TWindow::EvSysColorChange();
}
//----------------------------------------------------------------------------
int
TNotebook::GetScrollPos(int bar)
{
  if ((bar == SB_HORZ) && (winhscroll))
    return(winhscroll->GetScrollPos());
  else if ((bar == SB_VERT) && (winvscroll))
    return(winvscroll->GetScrollPos());

  return(0);
}
//----------------------------------------------------------------------------
void TNotebook::GetScrollRange(int bar, int& low, int& high)
{
  if ((bar == SB_HORZ) && (winhscroll))
    winhscroll->GetScrollRange(low, high);
  else if ((bar == SB_VERT) && (winvscroll))
    winvscroll->GetScrollRange(low, high);
}
//----------------------------------------------------------------------------
int TNotebook::SetScrollPos(int bar, int pos, bool redraw)
{
  if ((bar == SB_HORZ) && (winhscroll))
    return(winhscroll->SetScrollPos(pos, redraw));
  else if ((bar == SB_VERT) && (winvscroll))
    return(winvscroll->SetScrollPos(pos, redraw));

  return 0;
}
//----------------------------------------------------------------------------
void TNotebook::SetScrollRange(int bar, int  minPos, int  maxPos, bool redraw)
{
  bool showit = (minPos != maxPos);

  if ((bar == SB_HORZ) && (winhscroll)){
    if (hscrollon != showit) {
      hscrollon = showit;
      winhscroll->ShowScrollBar(showit);
      ForceResize(FALSE);
    }
    winhscroll->SetScrollRange(minPos, maxPos, redraw);
  } else if ((bar == SB_VERT) && (winvscroll)) {
    if (vscrollon != showit){
      vscrollon = showit;
      winvscroll->ShowScrollBar(showit);
      ForceResize(FALSE);
    }
    winvscroll->SetScrollRange(minPos, maxPos, redraw);
  }
}
//----------------------------------------------------------------------------
void TNotebook::ForceResize(bool repaint)
{
  // force a RECALC SIZE and possibly REPAINT of non-client area thru
  // following wierd code...

  TRect wrect;
  int width, height;

  wrect = GetWindowRect();
  TPoint tl = wrect.TopLeft();
  TPoint br = wrect.BottomRight();
  Parent->ScreenToClient(tl);
  Parent->ScreenToClient(br);

  width = br.x - tl.x;
  height = br.y - tl.y;

  MoveWindow(tl.x, tl.y, width, height+1, TRUE);
  if (repaint)
    defer_ncpaint = -1;         // do ncpaint but don't call myself
  MoveWindow(tl.x, tl.y, width, height, TRUE);
  if (repaint)
    defer_ncpaint = 0;
}
//----------------------------------------------------------------------------
void
TNotebook::BuildBgBrush()
{
  struct {
    BITMAPINFOHEADER hdr;
    RGBQUAD colors[2];
  } bmi;

  HBITMAP mybitmap;
  TWindowDC mydc(*this);
  unsigned char bits[8*sizeof(DWORD)];
  COLORREF color;
  HBRUSH hbrush;

  bmi.hdr.biSize = sizeof(BITMAPINFOHEADER);
  bmi.hdr.biWidth = 8;
  bmi.hdr.biHeight = 8;
  bmi.hdr.biPlanes = 1;
  bmi.hdr.biBitCount = 1;
  bmi.hdr.biSizeImage = 0;
  bmi.hdr.biCompression = BI_RGB;
  bmi.hdr.biClrUsed = 0;
  bmi.hdr.biClrImportant = 0;

  memset(bits, 0, sizeof(bits));
  bits[0 * sizeof(DWORD)] = 0xAA;
  bits[2 * sizeof(DWORD)] = 0xAA;
  bits[4 * sizeof(DWORD)] = 0xAA;
  bits[6 * sizeof(DWORD)] = 0xAA;
  bits[1 * sizeof(DWORD)] = 0x55;
  bits[3 * sizeof(DWORD)] = 0x55;
  bits[5 * sizeof(DWORD)] = 0x55;
  bits[7 * sizeof(DWORD)] = 0x55;

  color = GetSysColor(COLOR_BTNFACE);
  bmi.colors[0].rgbRed = GetRValue(color);
  bmi.colors[0].rgbGreen = GetGValue(color);
  bmi.colors[0].rgbBlue = GetBValue(color);
  bmi.colors[0].rgbReserved = 0;

  color = GetSysColor(COLOR_BTNHIGHLIGHT);
  bmi.colors[1].rgbRed = GetRValue(color);
  bmi.colors[1].rgbGreen = GetGValue(color);
  bmi.colors[1].rgbBlue = GetBValue(color);
  bmi.colors[1].rgbReserved = 0;

  mybitmap = ::CreateDIBitmap(mydc, &bmi.hdr, CBM_INIT, bits,
    (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
  hbrush = ::CreatePatternBrush(mybitmap);
  bgbrush = new TBrush(hbrush);
  ::DeleteObject(mybitmap);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TCustomScroll::TCustomScroll(TWindow* parent, int, int, int, int,
               int /*idnum*/, bool Frame3d, bool Arrowline, bool Thumbmove, int Repeatdelay)
{
  Parent = parent;
  frame3d = Frame3d;
  arrowline = Arrowline;
  thumbmove = Thumbmove;
  repeatdelay = Repeatdelay;
  showit = true;
  if (arrowline)
    barsize = 1;
  else
    barsize = 2;

  BuildBrushes();
}
//----------------------------------------------------------------------------
TCustomScroll::~TCustomScroll()
{
  DeleteBrushes();
}
//----------------------------------------------------------------------------
void
TCustomScroll::DeleteBrushes()
{
  delete framebrush;
  delete trackbrush;
  delete hilitebrush;
  delete shadowbrush;
  delete facebrush;
}
//----------------------------------------------------------------------------
void
TCustomScroll::BuildBrushes()
{
  framebrush  = new TBrush(GetSysColor(COLOR_WINDOWFRAME));
  trackbrush  = new TBrush(GetSysColor(COLOR_SCROLLBAR));
  hilitebrush = new TBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
  shadowbrush = new TBrush(GetSysColor(COLOR_BTNSHADOW));
  facebrush   = new TBrush(GetSysColor(COLOR_BTNFACE));
}
//----------------------------------------------------------------------------
void
TCustomScroll::ShowScrollBar(bool show)
{
  showit = show;
}
//----------------------------------------------------------------------------
void
TCustomScroll::SysColorChange()
{
  DeleteBrushes();
  BuildBrushes();
}
//----------------------------------------------------------------------------
int
TCustomScroll::GetScrollPos()
{
  return curval;
}
//----------------------------------------------------------------------------
void
TCustomScroll::GetScrollRange(int& low, int& high)
{
  low = lowval;
  high = highval;
}
//----------------------------------------------------------------------------
void
TCustomScroll::AutoScrollMouse()
{
  MSG  loopMsg;
  DWORD delayticks;

  loopMsg.message = 0;
  Parent->SetCapture();

  delayticks = repeatdelay + GetTickCount();

  while (loopMsg.message != WM_LBUTTONUP &&
    (loopMsg.message != WM_MOUSEMOVE ||(loopMsg.wParam&MK_LBUTTON))) {
      if (::PeekMessage(&loopMsg, 0, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&loopMsg);
        ::DispatchMessage(&loopMsg);
      }
      if (GetTickCount() > delayticks)
        AutoScrollAction();
  }

  Parent->ReleaseCapture();
}
//----------------------------------------------------------------------------
void
TCustomScroll::ForcePaint()
{
  MyPaint(CSB_FORCEALL);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
THScroll::THScroll(TWindow* Parent, int x, int y, int width, int height,
           int idnum, int Lowval, int Highval, int initval, bool Frame3d,
           bool Arrowline, bool Thumbmove, int Repeatdelay)
           :
TCustomScroll(Parent, x, y, width, height,idnum,Frame3d, Arrowline,
        Thumbmove, Repeatdelay)
{
  SetScrollRange(Lowval, Highval, false);
  SetScrollPos(initval, false);
  mousedown = CSB_NOTDOWN;
  displaystate = CSB_NOTDOWN;
  mylongid = MkUint32(0, (uint16)idnum);
  if (!idnum)
    scrollmsg = WM_HSCROLL;
  else
    scrollmsg = NW_HSCROLL;

  ChangeRect(TRect(x, y, x+width, y+height), false);
}
//----------------------------------------------------------------------------
THScroll::~THScroll()
{
}
//----------------------------------------------------------------------------
void
THScroll::ChangeRect(const TRect& newrect, bool repaint)
{
  int height = newrect.Height();
  int width = newrect.Width();

  // set minimum sizes
  if (height < 4)
    height = 4;

  if (width < 14)
    width = 14;

  framerect = TRect(newrect.left, newrect.top, newrect.left+width,
    newrect.top+height);

  if (frame3d)    // will add 3d frame on outside
    sbrect = framerect.InflatedBy(-1, -1);
  else
    sbrect = framerect;

  // compute a few measurements here to make our life easier
  sbheight = sbrect.bottom - sbrect.top;
  sbwidth = sbrect.right - sbrect.left;
  sqsize = sbheight - 2;
  trackwidth = sbwidth - 2*(sqsize + barsize);  // overlaps arrow lines
  trange = trackwidth - (sqsize+2);
  trackxoff = sbrect.left + sqsize + barsize;  // start on outer left arrow bar

  SetScrollPos(curval, SV_NOREPAINT);

  if (repaint)
    MyPaint(CSB_FORCEALL);
}
//----------------------------------------------------------------------------
void
THScroll::DrawFrames(TDC& mydc)
{
  if (frame3d)      // outer 3d frame
    DrawShadeFrame(mydc, framerect, false);

  // draw inner frame (avoid flicker by not painting interrior)
  mydc.FrameRect(sbrect, *framebrush);

  // draw double black lines for left and right buttons
  mydc.SelectObject(*framebrush);
  FASTRECT(mydc, sbrect.left+sqsize+1, sbrect.top, barsize, sbheight);
  FASTRECT(mydc, sbrect.right-(sqsize+1+barsize), sbrect.top, barsize,
    sbheight);
}
//----------------------------------------------------------------------------
void
THScroll::DrawLeftArrow(TDC& mydc, bool isdown)
{
  TRect myrect(sbrect.left+1, sbrect.top+1, sbrect.left+sqsize+1,
    sbrect.bottom-1);
  DrawArrowButton(mydc, myrect, isdown, true);
}
//----------------------------------------------------------------------------
void
THScroll::DrawRightArrow(TDC& mydc, bool isdown)
{
  TRect myrect(sbrect.right-(sqsize+1), sbrect.top+1, sbrect.right-1,
    sbrect.bottom-1);
  DrawArrowButton(mydc, myrect, isdown, false);
}

//----------------------------------------------------------------------------
void
THScroll::DrawLeftTrack(TDC& mydc, bool isdown)
{
  if (isdown)
    mydc.SelectObject(*shadowbrush);
  else
    mydc.SelectObject(*trackbrush);
  FASTRECT(mydc, trackxoff, sbrect.top+barsize, thumbxoff-trackxoff,
    sbheight-2);
}
//----------------------------------------------------------------------------
void
THScroll::DrawRightTrack(TDC& mydc, bool isdown)
{
  int xoff = thumbxoff+sqsize+2;
  int lastx = trackxoff + trackwidth - 1;

  if (isdown)
    mydc.SelectObject(*shadowbrush);
  else
    mydc.SelectObject(*trackbrush);

  FASTRECT(mydc, xoff, sbrect.top+1, lastx-xoff+1, sbheight-2);
}
//----------------------------------------------------------------------------
void
THScroll::DrawThumb(TDC& mydc, bool drawfocus, bool erasefocus)
{
  if ((erasefocus) && (lastfocusxoff >= 0)){
    mydc.DrawFocusRect(lastfocusxoff, sbrect.top, lastfocusxoff+sqsize+2,
      sbrect.top+sbheight);
  }

  if (drawfocus){
    mydc.DrawFocusRect(thumbxoff, sbrect.top, thumbxoff+sqsize+2,
      sbrect.top+sbheight);
    lastfocusxoff = thumbxoff;
    return;
  }

  // draw thumb track (but don't cover two outside black lines)
  mydc.SelectObject(*trackbrush);
  FASTRECT(mydc, trackxoff+1, sbrect.top+1, trackwidth-2, sbheight-2);

  // draw black lines on each side of solid thumb
  mydc.SelectObject(*framebrush);
  FASTVLINE(mydc, thumbxoff, sbrect.top, sbheight);
  FASTVLINE(mydc, thumbxoff+sqsize+1, sbrect.top, sbheight);

  // draw solid thumb
  mydc.SelectObject(*facebrush);
  FASTRECT(mydc, thumbxoff+1, sbrect.top+1, sqsize, sqsize);

  // draw shaded frame and extra "_|" design for solid thumb
  TRect myrect(thumbxoff+1, sbrect.top+1, thumbxoff+sqsize+1,
    sbrect.bottom-1);

  DrawShadeFrame(mydc, myrect, true);

  lastfocusxoff = -1;
}

//----------------------------------------------------------------------------
void
THScroll::DrawDownFrame(TDC& mydc, TRect& myrect)
{
  // always a single width shadow line
  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.top, myrect.right-myrect.left);
  FASTVLINE(mydc, myrect.left, myrect.top, myrect.bottom-myrect.top);
}
//----------------------------------------------------------------------------
void
THScroll::DrawShadeFrame(TDC& mydc, TRect& myrect, bool forthumb)
{
  int width = myrect.right - myrect.left;
  int height = myrect.bottom - myrect.top;

  mydc.SelectObject(*hilitebrush);
  FASTHLINE(mydc, myrect.left, myrect.top, width);
  FASTVLINE(mydc, myrect.left, myrect.top, height);
  if (sqsize > 15) {    // double highlight
    FASTHLINE(mydc, myrect.left, myrect.top+1, width);
    FASTVLINE(mydc, myrect.left+1, myrect.top, height);
  }

  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.bottom-1, width);
  FASTVLINE(mydc, myrect.right-1, myrect.top, height);
  if ((forthumb) || (sqsize >= 15)) {    // double shadow
    FASTHLINE(mydc, myrect.left+1, myrect.bottom-2, width-1);
    FASTVLINE(mydc, myrect.right-2, myrect.top+1, height-1);
  }
}
//---------------------------------------------------------------------------
void
THScroll::DrawArrowButton(TDC& mydc, TRect& brect, bool isdown, bool left)
{
  int xoff, yoff, width, height, downoff, aheight, awidth;
  int lheight, lwidth, mywidth, delta;

  // draw button background
  mydc.FillRect(brect, *facebrush);

  // draw frame around button
  if (isdown)
    DrawDownFrame(mydc, brect);
  else
    DrawShadeFrame(mydc, brect, FALSE);

  // calculations for drawing arrow line and head
  width = brect.right - brect.left;
  height = brect.bottom - brect.top;

  if (isdown)
    downoff = 1;
  else
    downoff = 0;

  mydc.SelectObject(*framebrush);  // for drawing arrows

  aheight = 2 + (height+1)/3;
  if (! (aheight%2))    // need an odd numbered arrow height
    aheight--;

  awidth = (aheight+1)/2;
  if (arrowline)
    lwidth = awidth-1;
  else
    lwidth = 0;

  mywidth = awidth + lwidth;
  delta = width - mywidth;
  xoff = brect.left + delta/2;  // where whole object should start
  if (delta%2)  // odd number - put extra space on left
    xoff++;

  if (arrowline) {    // draw arrow line
    if (left)
      xoff += awidth;    // xoff is now where line starts
    lheight = (aheight+2)/3;
    yoff = brect.top + (height/2 - lheight/2);
    FASTRECT(mydc, xoff+downoff, yoff+downoff, lwidth, lheight);
    if (left)
      xoff--;
    else
      xoff += lwidth;
  }else if (left)
    xoff += (awidth-1);

  // xoff is now where back line of arrow should be drawn
  yoff = brect.top + (height/2 - aheight/2);

  // draw arrow head
  while (aheight > 0) {
    FASTVLINE(mydc, xoff+downoff, yoff+downoff, aheight);
    aheight -= 2;
    if (left)
      xoff--;
    else
      xoff++;
    yoff++;
  }
}
//---------------------------------------------------------------------------
void
THScroll::MyPaint(int newstate)
{
  if ((!Parent->GetHandle())||(Parent->IsIconic())||(!showit))
    return;

  bool forceall = (newstate == CSB_FORCEALL);

  if ((! forceall) && (newstate == displaystate) &&
    (newstate <= CSB_RIGHTTHUMB))
    return;      // already drawn

  bool doleft = (displaystate==CSB_LEFTARROW) || (newstate==CSB_LEFTARROW);
  bool doright = (displaystate==CSB_RIGHTARROW) || (newstate==CSB_RIGHTARROW);
  bool dothumb = (displaystate>=CSB_LEFTTHUMB) || (newstate>=CSB_LEFTTHUMB);

  if (newstate <= CSB_RIGHTTHUMB)
    displaystate = newstate;

  TWindowDC mydc(*Parent);

  if (forceall)
    DrawFrames(mydc);

  if ((forceall) || (doleft))
    DrawLeftArrow(mydc, (displaystate==CSB_LEFTARROW));

  if ((forceall) || (doright))
    DrawRightArrow(mydc, (displaystate==CSB_RIGHTARROW));

  if ((forceall) || (dothumb)) {
    DrawThumb(mydc, (displaystate==CSB_THUMBFOCUS),
      (displaystate==CSB_THUMBFOCUS));
    if (displaystate == CSB_LEFTTHUMB)      // gray to left of thumb
      DrawLeftTrack(mydc, TRUE);
    else if (displaystate == CSB_RIGHTTHUMB)    // gray to right of thumb
      DrawRightTrack(mydc, TRUE);
  }
}
//----------------------------------------------------------------------------
int
THScroll::SetScrollPos(int newval, int repaint)
{
  int oldval = curval;
  curval = std::min(std::max(newval, lowval), highval);

  // reposition thumb to match new value
  thumbxoff = (int)(trackxoff + ((LONG)trange*(curval-lowval))/vrange);

  if (repaint == SV_REPAINT)
    MyPaint(CSB_THUMB);
  else if (repaint == SV_THUMBTRACK)
    MyPaint(thumbmove ? CSB_THUMB : CSB_THUMBFOCUS);

  return(oldval);
}
//----------------------------------------------------------------------------
void
THScroll::SetScrollRange(int low, int hi, bool redraw)
{
  lowval = low;
  highval = hi;
  if (highval <= lowval)
    highval = lowval + 1;

  vrange = highval - lowval;

  if (redraw)
    MyPaint(CSB_FORCEALL);
}
//----------------------------------------------------------------------------
int
THScroll::GetMouseLoc(TPoint& winpt)
{
  if ((Parent->IsIconic()) || (! sbrect.Contains(winpt)) || (! showit))
    return CSB_NOTDOWN;

  int myx = winpt.x;
  int left = sbrect.left;

  if (myx < left+sqsize+2)
    return CSB_LEFTARROW;
  if (myx > (left+sqsize+2+trackwidth))
    return CSB_RIGHTARROW;
  if (myx < thumbxoff)
    return CSB_LEFTTHUMB;
  if (myx > thumbxoff+sqsize+2)
    return CSB_RIGHTTHUMB;
  return CSB_THUMB;
}
//----------------------------------------------------------------------------
bool
THScroll::MyLButtonDown(uint /*hitTest*/, TPoint& winpt)
{
  mousedown = GetMouseLoc(winpt);
  if (mousedown == CSB_NOTDOWN)  // mousedown not in scrollbar
    return false;

  origval = curval;
  newloc = mousedown;

  switch (mousedown) {
  case CSB_LEFTARROW:
  case CSB_RIGHTARROW:
    MyPaint(mousedown);
    break;

  case CSB_LEFTTHUMB:
    MyPaint(mousedown);
    Parent->SendMessage(scrollmsg, SB_PAGELEFT, mylongid);
    break;

  case CSB_RIGHTTHUMB:
    MyPaint(mousedown);
    Parent->SendMessage(scrollmsg, SB_PAGERIGHT, mylongid);
    break;

  case CSB_THUMB:
    thumbgrabxoff = winpt.x - thumbxoff;
    SetScrollPos(curval, SV_THUMBTRACK);
  }

  AutoScrollMouse();

  return true;
}
//----------------------------------------------------------------------------
bool
THScroll::MyMouseMove(uint /*hitTest*/, TPoint& winpt)
{
  if (mousedown == CSB_NOTDOWN)  // mousedown was not in our scrollbar
    return false;

  newloc = GetMouseLoc(winpt);

  switch (mousedown) {
  case CSB_LEFTARROW:
  case CSB_RIGHTARROW:
  case CSB_LEFTTHUMB:
  case CSB_RIGHTTHUMB:
    if (newloc == mousedown)
      MyPaint(mousedown);
    else
      MyPaint(CSB_NOTDOWN);
    break;

  case CSB_THUMB:
    if (newloc != CSB_NOTDOWN){    // anywhere inside sb
      int newpos = (winpt.x - thumbgrabxoff) - trackxoff;
      if (newpos < 0)
        newpos = 0;
      else if (newpos > trange)
        newpos = trange;
      int newval = (int)(lowval + ((LONG)vrange*newpos)/trange);
      SetScrollPos(newval, SV_THUMBTRACK);
      Parent->SendMessage(scrollmsg, SB_THUMBTRACK,
        mylongid | newval);
    } else {
      SetScrollPos(origval, SV_THUMBTRACK);
      Parent->SendMessage(scrollmsg, SB_THUMBTRACK,
        mylongid | origval);
    }
    break;
  }
  return true;
}
//----------------------------------------------------------------------------
bool
THScroll::MyLButtonUp(uint /*hitTest*/, TPoint& winpt)
{
  int sendval;

  if (mousedown == CSB_NOTDOWN)  // mousedown was not in our scrollbar
    return false;

  newloc = GetMouseLoc(winpt);

  switch (mousedown){
  case CSB_LEFTARROW:
    MyPaint(CSB_NOTDOWN);
    if (newloc == mousedown)
      Parent->SendMessage(scrollmsg, SB_LINELEFT, mylongid);
    break;

  case CSB_RIGHTARROW:
    MyPaint(CSB_NOTDOWN);
    if (newloc == mousedown)
      Parent->SendMessage(scrollmsg, SB_LINERIGHT, mylongid);
    break;

  case CSB_THUMB:
    sendval = curval;    // save off before overwritting
    SetScrollPos(origval, SV_REPAINT);
    Parent->SendMessage(scrollmsg, SB_THUMBPOSITION,
      mylongid | sendval);
    break;

  default:
    MyPaint(CSB_NOTDOWN);
    break;
  }

  Parent->SendMessage(scrollmsg, SB_ENDSCROLL, mylongid);
  mousedown = CSB_NOTDOWN;
  return true;
}
//----------------------------------------------------------------------------
void
THScroll::AutoScrollAction()
{
  if (newloc != mousedown)
    return;

  switch (mousedown) {
  case CSB_LEFTARROW:
    Parent->SendMessage(scrollmsg, SB_LINELEFT, mylongid);
    break;

  case CSB_RIGHTARROW:
    Parent->SendMessage(scrollmsg, SB_LINERIGHT, mylongid);
    break;

  case CSB_LEFTTHUMB:
    Parent->SendMessage(scrollmsg, SB_PAGELEFT, mylongid);
    break;

  case CSB_RIGHTTHUMB:
    Parent->SendMessage(scrollmsg, SB_PAGERIGHT, mylongid);
    break;
  }
}
////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TVScroll::TVScroll(TWindow* Parent, int x, int y, int width, int height,
           int idnum, int Lowval, int Highval, int initval, bool Frame3d,
           bool Arrowline, bool Thumbmove, int Repeatdelay)
           :
TCustomScroll(Parent, x, y, width, height,idnum,Frame3d, Arrowline,
        Thumbmove, Repeatdelay)
{
  SetScrollRange(Lowval, Highval, false);
  SetScrollPos(initval, false);

  mousedown = CSB_NOTDOWN;
  displaystate = CSB_NOTDOWN;
  mylongid = MAKELONG(0, idnum);
  if (!idnum)
    scrollmsg = WM_VSCROLL;
  else
    scrollmsg = NW_VSCROLL;

  ChangeRect(TRect(x, y, x+width, y+height), FALSE);
}
//----------------------------------------------------------------------------
TVScroll::~TVScroll()
{
}
//----------------------------------------------------------------------------
void
TVScroll::ChangeRect(const TRect& newrect, bool repaint)
{
  int width = newrect.Width();
  int height = newrect.Height();

  // set minimum sizes
  if (height < 14)
    height = 14;
  if (width < 4)
    width = 4;
  framerect = TRect(newrect.left, newrect.top, newrect.left+width,
    newrect.top+height);

  if (frame3d)    // will add 3d frame on outside
    sbrect = framerect.InflatedBy(-1, -1);
  else
    sbrect = framerect;

  // compute a few measurements here to make our life easier
  sbheight = sbrect.bottom - sbrect.top;
  sbwidth = sbrect.right - sbrect.left;
  sqsize = sbwidth - 2;
  trackheight = sbheight - 2*(sqsize+barsize);    // overlaps arrow lines
  trange = trackheight - (sqsize+2);
  trackyoff = sbrect.top + sqsize + barsize;  // start on 2nd left arrow bar

  SetScrollPos(curval, SV_NOREPAINT);

  if (repaint)
    MyPaint(CSB_FORCEALL);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawFrames(TDC& mydc)
{
  if (frame3d)      // outer 3d frame
    DrawShadeFrame(mydc, framerect, FALSE);

  // draw inner frame (avoid flicker by not painting interrior)
  mydc.FrameRect(sbrect, *framebrush);

  // draw single/double black lines for top and bottom buttons
  mydc.SelectObject(*framebrush);
  FASTRECT(mydc, sbrect.left, sbrect.top+sqsize+1, sbwidth, barsize);
  FASTRECT(mydc, sbrect.left, sbrect.bottom-(sqsize+barsize+1), sbwidth,
    barsize);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawTopArrow(TDC& mydc, bool isdown)
{
  TRect myrect(sbrect.left+1, sbrect.top+1, sbrect.left+sqsize+1,
    sbrect.top+sqsize+1);
  DrawArrowButton(mydc, myrect, isdown, TRUE);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawBottomArrow(TDC& mydc, bool isdown)
{
  TRect myrect(sbrect.left+1, sbrect.bottom-(sqsize+1),
    sbrect.left+sqsize+1, sbrect.bottom-1);
  DrawArrowButton(mydc, myrect, isdown, FALSE);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawTopTrack(TDC &mydc, bool isdown)
{
  if (isdown)
    mydc.SelectObject(*shadowbrush);
  else
    mydc.SelectObject(*trackbrush);
  FASTRECT(mydc, sbrect.left+1, trackyoff, sbwidth-2, thumbyoff-trackyoff);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawBottomTrack(TDC& mydc, bool isdown)
{
  int yoff = thumbyoff+sqsize+2;
  int lasty = trackyoff + trackheight - 1;

  if (isdown)
    mydc.SelectObject(*shadowbrush);
  else
    mydc.SelectObject(*trackbrush);
  FASTRECT(mydc, sbrect.left+1, yoff, sbwidth-2, lasty-yoff+1);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawThumb(TDC& mydc, bool drawfocus, bool erasefocus)
{
  if ((erasefocus) && (lastfocusyoff >= 0)) {
    mydc.DrawFocusRect(sbrect.left, lastfocusyoff, sbrect.right,
      lastfocusyoff+sqsize+2);
  }

  if (drawfocus) {
    mydc.DrawFocusRect(sbrect.left, thumbyoff, sbrect.left+sbwidth,
      thumbyoff+sqsize+2);
    lastfocusyoff = thumbyoff;
    return;
  }

  // draw thumb track (but don't cover two outside black lines)
  mydc.SelectObject(*trackbrush);
  FASTRECT(mydc, sbrect.left+1, trackyoff+1, sbwidth-2, trackheight-2);

  // draw black lines on each side of solid thumb
  mydc.SelectObject(*framebrush);
  FASTHLINE(mydc, sbrect.left, thumbyoff, sbwidth);
  FASTHLINE(mydc, sbrect.left, thumbyoff+sqsize+1, sbwidth);

  // draw solid thumb
  mydc.SelectObject(*facebrush);
  FASTRECT(mydc, sbrect.left+1, thumbyoff+1, sqsize-2, sqsize-2);

  // draw shaded frame and extra "_|" design for solid thumb
  TRect myrect(sbrect.left+1, thumbyoff+1, sbrect.right-1,
    thumbyoff+sqsize+1);
  DrawShadeFrame(mydc, myrect, TRUE);

  lastfocusyoff = -1;
}
//----------------------------------------------------------------------------
void
TVScroll::DrawDownFrame(TDC& mydc, TRect& myrect)
{
  // always a single width shadow line
  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.top, myrect.right-myrect.left);
  FASTVLINE(mydc, myrect.left, myrect.top, myrect.bottom-myrect.top);
}
//----------------------------------------------------------------------------
void
TVScroll::DrawShadeFrame(TDC& mydc, TRect& myrect, bool forthumb)
{
  int width = myrect.right - myrect.left;
  int height = myrect.bottom - myrect.top;

  mydc.SelectObject(*hilitebrush);
  FASTHLINE(mydc, myrect.left, myrect.top, width);
  FASTVLINE(mydc, myrect.left, myrect.top, height);
  if (sqsize > 15) {    // double highlight
    FASTHLINE(mydc, myrect.left, myrect.top+1, width);
    FASTVLINE(mydc, myrect.left+1, myrect.top, height);
  }

  mydc.SelectObject(*shadowbrush);
  FASTHLINE(mydc, myrect.left, myrect.bottom-1, width);
  FASTVLINE(mydc, myrect.right-1, myrect.top, height);
  if ((forthumb) || (sqsize >= 15)) {    // double shadow
    FASTHLINE(mydc, myrect.left+1, myrect.bottom-2, width-1);
    FASTVLINE(mydc, myrect.right-2, myrect.top+1, height-1);
  }
}
//---------------------------------------------------------------------------
void TVScroll::DrawArrowButton(TDC &mydc, TRect& brect, bool isdown, bool top)
{
  int xoff, yoff, width, height, downoff, aheight, awidth;
  int lheight, lwidth, myheight, delta;

  // draw button background
  mydc.FillRect(brect, *facebrush);

  // draw frame around button
  if (isdown)
    DrawDownFrame(mydc, brect);
  else
    DrawShadeFrame(mydc, brect, FALSE);

  // calculations for drawing arrow line and head
  width = brect.right - brect.left;
  height = brect.bottom - brect.top;

  if (isdown)
    downoff = 1;
  else
    downoff = 0;

  mydc.SelectObject(*framebrush);  // for drawing arrows

  awidth = 2 + (width+1)/3;
  if (! (awidth%2))    // need an odd numbered arrow width
    awidth--;
  aheight = (awidth+1)/2;
  if (arrowline)
    lheight = aheight-1;
  else
    lheight = 0;

  myheight = aheight + lheight;
  delta = height - myheight;
  yoff = brect.top + delta/2;    // where whole object should start
  if (delta%2)  // odd number - put extra space on left
    yoff++;

  if (arrowline) {    // draw arrow line
    if (top)
      yoff += aheight;    // yoff is now where line starts
    lwidth = (awidth+2)/3;
    xoff = brect.left + (width/2 - lwidth/2);
    FASTRECT(mydc, xoff+downoff, yoff+downoff, lwidth, lheight);
    if (top)
      yoff--;
    else
      yoff += lheight;
  } else if (top)
    yoff += (aheight-1);

  // yoff is now where back line of arrow should be drawn
  xoff = brect.left + (width/2 - awidth/2);

  // draw arrow head
  while (awidth > 0) {
    FASTHLINE(mydc, xoff+downoff, yoff+downoff, awidth);
    awidth -= 2;
    if (top)
      yoff--;
    else
      yoff++;
    xoff++;
  }
}
//---------------------------------------------------------------------------
void
TVScroll::MyPaint(int newstate)
{
  if ((! Parent->GetHandle()) || (Parent->IsIconic()) || (! showit))
    return;

  bool forceall = (newstate == CSB_FORCEALL);

  if ((! forceall) && (newstate == displaystate) &&
    (newstate <= CSB_BOTTOMTHUMB))
    return;      // already drawn

  bool dotop = (displaystate==CSB_TOPARROW) || (newstate==CSB_TOPARROW);
  bool dobottom = (displaystate==CSB_BOTTOMARROW) ||
    (newstate==CSB_BOTTOMARROW);
  bool dothumb = (displaystate>=CSB_TOPTHUMB) || (newstate>=CSB_TOPTHUMB);

  if (newstate <= CSB_BOTTOMTHUMB)
    displaystate = newstate;

  TWindowDC mydc(*Parent);

  if (forceall)
    DrawFrames(mydc);

  if ((forceall) || (dotop))
    DrawTopArrow(mydc, (displaystate==CSB_TOPARROW));

  if ((forceall) || (dobottom))
    DrawBottomArrow(mydc, (displaystate==CSB_BOTTOMARROW));

  if ((forceall) || (dothumb)) {
    DrawThumb(mydc, (displaystate==CSB_THUMBFOCUS),
      (displaystate==CSB_THUMBFOCUS));
    if (displaystate == CSB_TOPTHUMB)      // gray to top of thumb
      DrawTopTrack(mydc, TRUE);
    else if (displaystate == CSB_BOTTOMTHUMB)    // gray to right of thumb
      DrawBottomTrack(mydc, TRUE);
  }
}
//----------------------------------------------------------------------------
int
TVScroll::SetScrollPos(int newval, int repaint)
{
  int oldval = curval;
  curval = std::min(std::max(newval, lowval), highval);

  // reposition thumb to match new value
  thumbyoff = (int)(trackyoff + ((LONG)trange*(curval-lowval))/vrange);

  if (repaint == SV_REPAINT)
    MyPaint(CSB_THUMB);
  else if (repaint == SV_THUMBTRACK)
    MyPaint(thumbmove ? CSB_THUMB : CSB_THUMBFOCUS);

  return(oldval);
}
//----------------------------------------------------------------------------
void
TVScroll::SetScrollRange(int Lowval, int Highval, bool redraw)
{
  lowval = Lowval;
  highval = Highval;
  if (highval <= lowval)
    highval = lowval + 1;
  vrange = highval - lowval;

  if (redraw)
    MyPaint(CSB_FORCEALL);
}
//----------------------------------------------------------------------------
int
TVScroll::GetMouseLoc(TPoint& winpt)
{
  if ((Parent->IsIconic()) || (! sbrect.Contains(winpt)) || (! showit))
    return CSB_NOTDOWN;

  int myy = winpt.y;
  int top = sbrect.top;

  if (myy < top+sqsize+2)
    return CSB_TOPARROW;
  if (myy > (top+sqsize+2+trackheight))
    return CSB_BOTTOMARROW;
  if (myy < thumbyoff)
    return CSB_TOPTHUMB;
  if (myy > thumbyoff+sqsize+2)
    return CSB_BOTTOMTHUMB;
  return CSB_THUMB;
}
//----------------------------------------------------------------------------
bool
TVScroll::MyLButtonDown(uint /*hitTest*/, TPoint& winpt)
{
  mousedown = GetMouseLoc(winpt);
  if (mousedown == CSB_NOTDOWN)  // mousedown not in scrollbar
    return false;

  origval = curval;
  newloc = mousedown;

  switch (mousedown) {
  case CSB_TOPARROW:
  case CSB_BOTTOMARROW:
    MyPaint(mousedown);
    break;

  case CSB_TOPTHUMB:
    MyPaint(mousedown);
    Parent->SendMessage(scrollmsg, SB_PAGEUP, mylongid);
    break;

  case CSB_BOTTOMTHUMB:
    MyPaint(mousedown);
    Parent->SendMessage(scrollmsg, SB_PAGEDOWN, mylongid);
    break;

  case CSB_THUMB:
    thumbgrabyoff = winpt.y - thumbyoff;
    SetScrollPos(curval, SV_THUMBTRACK);
  }

  AutoScrollMouse();

  return true;
}
//----------------------------------------------------------------------------
bool
TVScroll::MyMouseMove(uint /*hitTest*/, TPoint& winpt)
{
  if (mousedown == CSB_NOTDOWN)  // mousedown was not in our scrollbar
    return false;

  newloc = GetMouseLoc(winpt);

  switch (mousedown){
  case CSB_TOPARROW:
  case CSB_BOTTOMARROW:
  case CSB_TOPTHUMB:
  case CSB_BOTTOMTHUMB:
    if (newloc == mousedown)
      MyPaint(mousedown);
    else
      MyPaint(CSB_NOTDOWN);
    break;

  case CSB_THUMB:
    if (newloc != CSB_NOTDOWN){    // anywhere inside sb
      int newpos = (winpt.y - thumbgrabyoff) - trackyoff;
      if (newpos < 0)
        newpos = 0;
      else if (newpos > trange)
        newpos = trange;
      int newval = (int)(lowval + ((LONG)vrange*newpos)/trange);
      SetScrollPos(newval, SV_THUMBTRACK);
      Parent->SendMessage(scrollmsg, SB_THUMBTRACK,
        mylongid | newval);
    } else {
      SetScrollPos(origval, SV_THUMBTRACK);
      Parent->SendMessage(scrollmsg, SB_THUMBTRACK,
        mylongid | origval);
    }
    break;
  }
  return true;
}
//----------------------------------------------------------------------------
bool
TVScroll::MyLButtonUp(uint /*hitTest*/, TPoint& winpt)
{
  int sendval;

  if (mousedown == CSB_NOTDOWN)  // mousedown was not in our scrollbar
    return false;

  newloc = GetMouseLoc(winpt);

  switch (mousedown) {
  case CSB_TOPARROW:
    MyPaint(CSB_NOTDOWN);
    if (newloc == mousedown)
      Parent->SendMessage(scrollmsg, SB_LINEUP, mylongid);
    break;

  case CSB_BOTTOMARROW:
    MyPaint(CSB_NOTDOWN);
    if (newloc == mousedown)
      Parent->SendMessage(scrollmsg, SB_LINEDOWN, mylongid);
    break;

  case CSB_THUMB:
    sendval = curval;    // save off before overwritting
    SetScrollPos(origval, SV_REPAINT);
    Parent->SendMessage(scrollmsg, SB_THUMBPOSITION,
      mylongid | sendval);
    break;

  default:
    MyPaint(CSB_NOTDOWN);
    break;
  }

  Parent->SendMessage(scrollmsg, SB_ENDSCROLL, mylongid);
  mousedown = CSB_NOTDOWN;
  return true;
}
//----------------------------------------------------------------------------
void
TVScroll::AutoScrollAction()
{
  if (newloc != mousedown)
    return;

  switch (mousedown){
    case CSB_TOPARROW:
      Parent->SendMessage(scrollmsg, SB_LINEUP, mylongid);
      break;

    case CSB_BOTTOMARROW:
      Parent->SendMessage(scrollmsg, SB_LINEDOWN, mylongid);
      break;

    case CSB_TOPTHUMB:
      Parent->SendMessage(scrollmsg, SB_PAGEUP, mylongid);
      break;

    case CSB_BOTTOMTHUMB:
      Parent->SendMessage(scrollmsg, SB_PAGEDOWN, mylongid);
      break;
  }
}

} // OwlExt namespace

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//\\\\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//\\\\\\\\\\\\\\\\\\//\\\\\\\\\\\\\\\\\//
////////////////////////////////////////////////////////////////////////////////

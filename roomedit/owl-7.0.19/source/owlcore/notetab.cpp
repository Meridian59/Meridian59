//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TNoteTab
/// Added Scroll handling. Contributed by Jogy.
/// Added support for set font and set image. Contributed by Jogy.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/contain.h>
#include <owl/control.h>
#include <owl/notetab.h>
#include <owl/uihelper.h>
#include <owl/commctrl.h>
#include <owl/uimetric.h>
#include <owl/updown.h>
#include <owl/uimetric.h>
#include <owl/celarray.h>
#include <owl/theme.h>
#include <owl/gdiplus.h>
#include <algorithm>

using namespace std;

namespace owl {

OWL_DIAGINFO;

#define OWL_CLIP_TAB_BOUNDING_RECT_ 0 // debugging
#define OWL_DRAW_TAB_BOUNDING_RECT_ 0 // debugging

#if OWL_CLIP_TAB_BOUNDING_RECT_

bool ShouldClipTabBoundingRect_ = false;

#endif

#if OWL_DRAW_TAB_BOUNDING_RECT_

bool ShouldDrawTabBoundingRect_ = false;

#endif

//----------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TNoteTab, TControl)
  EV_WM_SIZE,
  EV_WM_LBUTTONDOWN,
  EV_WM_GETDLGCODE,
  EV_WM_KEYDOWN,
  EV_WM_SETFOCUS,
  EV_WM_KILLFOCUS,
  EV_WM_HSCROLL,
  EV_WM_PAINT,
  EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

//
/// Constructor of NoteTab object. Use this constructor when creating a notetab
/// control from scratch.
//
TNoteTab::TNoteTab(TWindow*   parent,
                   int        id,
                   int x, int y, int w, int h,
                   TWindow*   buddy,
                   bool       dialogBuddy,
                   TModule*   module)
:
  TControl(parent, id, nullptr, x, y, w, h, module),
  WindowFace(!dialogBuddy),
  Buddy(buddy)
{
  InitCtrl();
}

//
/// Constructor of NoteTab object. Use this constructor when aliasing a control
/// defined within a dialog template.
//
TNoteTab::TNoteTab(TWindow*   parent,
                   int        resourceId,
                   TWindow*   buddy,
                   bool       dialogBuddy,
                   TModule*   module)
:
  TControl(parent, resourceId, module),
  WindowFace(!dialogBuddy),
  Buddy(buddy)
{
  InitCtrl();
}

//
/// Returns "OWL_Notetab" - the Window class name of the notetab control object
//
auto TNoteTab::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{OWL_NOTETAB};
}

//
/// Overriden virtual of TWindow - Initializes font used by control and resize
/// accordingly.
//
void
TNoteTab::SetupWindow()
{
  TControl::SetupWindow();

  // Initialize BuddyHandle if we have a Buddy.
  //
  if (Buddy)
    BuddyHandle = Buddy->GetHandle();
}

//
/// Adds a new tabitem to the notetab control
//
int
TNoteTab::Add(
  LPCTSTR label,
  INT_PTR clientData,
  int imageIdx,
  TAbsLocation imageLoc,
  bool shouldSelect)
{
  return Insert(label, GetCount(), clientData, imageIdx, imageLoc, shouldSelect);
}

//
/// Inserts a new TTabItem at the specified index.
//
int
TNoteTab::Insert(
  LPCTSTR label,
  int index,
  INT_PTR clientData,
  int imageIdx,
  TAbsLocation imageLoc,
  bool shouldSelect)
{
  PRECONDITION(index >= 0 && index <= GetCount());

  TabList.insert(TabList.begin() + index,
    TNoteTabItem(label, clientData, imageIdx, imageLoc));

  SetTabSize(index);
  SetTabRects(FirstVisibleTab);
  if (shouldSelect)
    SetSel(index);

  return index;
}

//
/// Remove the tabitem at the specified 'index'. Returns true on success; false
/// otherwise.
//
bool
TNoteTab::Delete(int index)
{
  if (index < 0 || index >= GetCount()) return false; // invalid index

  // Mark rectangles that need to be redrawn.
  //
  if (IsVisible(index))
  {
    for (int i = index; i < GetCount(); i++)
      InvalidateTab(i);
  }

  // Remove the tab.
  //
  TabList.erase(TabList.begin() + index);

  // Adjust SelectedTab and FirstVisibleTab.
  // If a tab before SelectedTab is deleted, then correct the index so that it
  // points to the same item. If the SelectedTab was deleted, then do nothing,
  // thus moving the selection to the following tab, unless it was the last
  // tab, in which case we need to move the selection left. Then do the same
  // for FirstVisibleTab.
  //
  int n = GetCount(); // new count
  if (index < SelectedTab || SelectedTab == n)
    --SelectedTab;
  CHECK(SelectedTab >= -1 && SelectedTab < n);
  if (index < FirstVisibleTab || FirstVisibleTab == n)
    --FirstVisibleTab;
  CHECK(FirstVisibleTab >= -1 && FirstVisibleTab < n);

  // Force SelectedTab to have focus.
  //
  if ((SelectedTab >= 0) && (SelectedTab < n))
    InvalidateTab(SelectedTab);

  // Calculate new tab rectangles, if any left.
  //
  if (n > 0)
    SetTabRects(FirstVisibleTab);
  return true;
}

//
/// Removes all tab items in the notetab control. Always returns true.
//
bool
TNoteTab::DeleteAll()
{
  TabList.clear();
  FirstVisibleTab = 0;
  SelectedTab = -1;

  return true;
}

//
/// Return the number of tab items in the notetab control.
//
int
TNoteTab::GetCount() const
{
  PRECONDITION(TabList.size() <= INT_MAX);
  return static_cast<int>(TabList.size());
}

//
/// Returns the index of the selected tabitem.
///
/// \note  Returns a zero-based index or -1 if there are no tab items in the notetab
/// control.
int
TNoteTab::GetSel() const
{
  return SelectedTab;
}

//
/// Selects the tabitem at the specified index.
/// \note  SetSel does not send any
/// notifications to the parent and/or buddy. Returns the 0 based index of the tab
/// item selected or -1 on failure.
//
int
TNoteTab::SetSel(int index)
{
  if (index < 0 || index >= GetCount()) return -1;
  if (index == SelectedTab) return index;

  // Change the selection.
  // First, invalidate area occupied by previously selected item.
  //
  if (SelectedTab >= 0 && SelectedTab < GetCount())
    InvalidateTab(SelectedTab);

  // Update selected index and tab rectangles (selected tab may differ from the rest).
  //
  SelectedTab = index;
  SetTabRects(FirstVisibleTab);

  // Invalidate area occupied by new selection.
  //
  if (SelectedTab >= 0 && SelectedTab < GetCount())
    InvalidateTab(SelectedTab);

  return index;
}

int
TNoteTab::GetMinimalHeight()
{
  int labelHeight = 0;
  for (int i = 0; i != GetCount(); ++i)
  {
    SetTabSize(i);
    labelHeight = std::max(labelHeight, static_cast<int>(TabList[i].LabelSize.cy));
  }
  return Margin.cy +
    LabelMargin.cy +
    labelHeight +
    LabelMargin.cy +
    (Style3d ? TUIMetric::CyEdge.Get() : 0);
}

//
/// Sets the amount of space to the left of the tabs and above the tabs.
/// Note that the horizontal margin is additional to the space required for scroll buttons, if the
/// latter are aligned to the left and visible. In this case, the horizontal margin is the space
/// between the scroll buttons and the first visible tab. Otherwise, the horizontal margin is the
/// space between the left edge of the control and the first visible tab.
//
void
TNoteTab::SetMargin(const TSize& v)
{
  Margin = v;
  Update();
}

//
/// Sets the amount of padding around the tab label.
//
void
TNoteTab::SetLabelMargin(const TSize& v)
{
  LabelMargin = v;
  Update();
}

//
/// Sets the horizontal spacing between image and text in the label.
//
void
TNoteTab::SetLabelImageMargin(int v)
{
  LabelImageMargin = v;
  Update();
}

//
/// Sets the margin around the focus rectangle for the selected tab.
//
void
TNoteTab::SetFocusMargin(const TSize& v)
{
  FocusMargin = v;
  Update();
}

//
/// Sets the horizontal distance between two tabs.
//
void
TNoteTab::SetTabSpacing(int v)
{
  TabSpacing = v;
  Update();
}

//
/// Sets the amount of narrowing on each side of the tab towards the bottom.
//
void
TNoteTab::SetTabTapering(int v)
{
  TabTapering = v;
  Update();
}

//
/// Sets the amount of extra height of the selected tab.
//
void
TNoteTab::SetSelectedTabProtrusion(int v)
{
  SelectedTabProtrusion = v;
  Update();
}

//
/// Returns the font used to render the text part of the tab labels.
//
const TFont&
TNoteTab::GetTabFont() const
{
  PRECONDITION(TabFont);
  return *TabFont;
}

//
/// Sets the font used to render the text part of the tab labels.
//
void
TNoteTab::SetTabFont(const TFont& font)
{
  TabFont.reset(new TFont(font.GetObject()));
  Update();
}

//
/// Returns the font used to render the text part of the selected tab label.
//
const TFont&
TNoteTab::GetSelectedTabFont() const
{
  PRECONDITION(TabFont);
  return SelectedTabFont ? *SelectedTabFont : *TabFont;
}

//
/// Sets the font used to render the text part of the selected tab label.
//
void
TNoteTab::SetSelectedTabFont(const TFont& font)
{
  SelectedTabFont.reset(new TFont(font.GetObject()));
  Update();
}

//
/// Sets the fill color used to paint the tabs.
//
void
TNoteTab::SetTabColor(const TColor& v)
{
  TabColor = v;
  Update();
}

//
/// Sets the fill color used to paint the selected tab.
/// This color is only used when WindowFace mode is selected.
//
void
TNoteTab::SetSelectedTabColor(const TColor& v)
{
  SelectedTabColor = v;
  Update();
}

//
/// Sets the pen color used to draw the edges of the tabs.
//
void
TNoteTab::SetEdgeColor(const TColor& v)
{
  EdgeColor = v;
  Update();
}

//
/// Retrieve information about the tab item at the specified index. Always returns
/// true.
//
bool
TNoteTab::GetItem(int index, TNoteTabItem& tabItem) const
{
  PRECONDITION(index >= 0);
  PRECONDITION(index < GetCount());

  tabItem = TabList[index];
  return true;
}

//
/// Functional style overload
//
TNoteTabItem
TNoteTab::GetItem(int index) const
{
  TNoteTabItem n;
  bool r = GetItem(index, n); CHECK(r); InUse(r);
  return n;
}

//
/// Updates information about the tab item at the specified index.
//
bool
TNoteTab::SetItem(int index, const TNoteTabItem& tabItem)
{
  if (index < GetCount() && index >= 0) {

    TabList[index] = tabItem;

    // !BB Need to recalc/invalidate etc...
    SetTabSize(index);
    SetTabRects(FirstVisibleTab);
    InvalidateTab(index);
    return true;
  }
  return false;
}

//
/// Return handle of buddy window associated with the notetab control.
//
HWND
TNoteTab::GetBuddy() const
{
  return BuddyHandle;
}

//
/// Sets handle of the buddy window associated with this notetab control.
//
void
TNoteTab::SetBuddy(HWND hwnd)
{
  BuddyHandle = hwnd;
  Buddy = GetWindowPtr( BuddyHandle );
  if( !Buddy )
    Buddy = new TWindow( hwnd );
}

//
/// Overrides TWindow virtual member function to handle transfers. There are no
/// transfers for NoteTab controls.
//
uint
TNoteTab::Transfer(void* /*buffer*/, TTransferDirection /*direction*/)
{
  return 0;
}

//----------------------------------------------------------------------------

//
/// Initialize internal variables used by NoteTab.
//
void
TNoteTab::InitCtrl()
{
  Style3d = true;
  ShouldUseThemes = false;
  Margin = TSize(5, TUIMetric::CySizeFrame);
  LabelMargin = TSize(5, 3);
  SelectedTabProtrusion = 0;
  LabelImageMargin = 3;
  FocusMargin = TSize(2, 2);
  TabSpacing = 4;
  TabTapering = 4;
  TabColor = TColor::Sys3dFace;
  SelectedTabColor = TColor::SysWindow;
  EdgeColor = TColor::SysWindowFrame;
  SetBkgndColor(TColor::Sys3dFace);

  TabList.clear();
  SetTabFont(TDefaultGuiFont());
  SelectedTabFont.reset(); // Causes TabFont to be used for selected tabs.
  SelectedTab = -1;
  FirstVisibleTab = 0;
  CelArray = nullptr;
  OwnedCelArray.reset();
  TransparentColor = TColor::Sys3dFace;
  ScrollLoc = alRight;

  ModifyStyle(0, WS_CLIPCHILDREN); // Clipping eliminates scrollbar flicker.
  ScrollButtons = new TUpDown{this, -1, 0, 0, 0, 0};
  ScrollButtons->ModifyStyle(WS_TABSTOP, UDS_HORZ); // No focus, please.
}

//
/// Returns the bounding rectangle of a tab given its hit rectangle.
/// The bounding rectangle is the smallest rectangle that encapsulates the
/// whole tab, and outside which the tab will not draw.
/// The bounding rectangle may be larger than the tab's hit rectangle.
//
TRect
TNoteTab::GetBoundingRect(const TRect& tabRect) const
{
  // Adjust for the widening of the tab towards the page edge (top).
  //
  return tabRect.IsEmpty() ? TRect() : tabRect.InflatedBy(TabTapering, 0);
}

//
/// Invalidates the rectangle occupied by the tab at the specified index.
//
void
TNoteTab::InvalidateTab(int index)
{
  PRECONDITION(index >= 0 && index < GetCount());
  if (!GetHandle()) return;

  const TNoteTabItem& tab = TabList[index];
  InvalidateRect(GetBoundingRect(tab.Rect));
}

namespace
{

  //
  // Returns `true` if themes are enabled for this application and
  // themed Common Controls (version 6 or later) are in use.
  //
  // Important: This function must be called after the creation of the main
  // window, otherwise it may always return `false`.
  //
  // Note that IsAppThemed will return `false` if either (a) themes have been
  // disabled for the application by selecting "Disable visual themes" in the
  // Compatibility tab in the Properties dialog for the executable, or (b)
  // themes have been deactivated by selecting the Windows Classic style in
  // the Windows XP/7/Vista Control Panel (not available in Windows 8).
  // Note that (b) may change at run-time.
  //
  // Note we do not have to use IsThemeActive here. This function only reports
  // the state of the Control Panel setting (Classic vs themed).
  //
  bool IsThemed_()
  {
    static const auto v = GetCommCtrlVersion();
    return TThemeModule::GetInstance().IsAppThemed() && v >= 0x60000;
  }

  //
  // Defines a base class with common implementation and utilities for derived
  // UI part renderers.
  //
  struct TRenderer_
  {
    TWindow& Window;
    TDC& Dc;
    const TFont& TabFont;
    const TFont& SelectedTabFont;
    TColor TabTextColor;

    TRenderer_(
      TWindow& w,
      TDC& dc,
      const TRect& paintRect,
      const TFont& tabFont,
      const TFont& selectedTabFont,
      TColor tabTextColor,
      TColor tabColor,
      TColor selectedTabColor,
      TColor edgeColor
      )
      : Window(w), Dc(dc), TabFont(tabFont), SelectedTabFont(selectedTabFont), TabTextColor(tabTextColor)
    {InUse(paintRect); InUse(tabColor); InUse(selectedTabColor); InUse(edgeColor);}

    void PaintPageEdge(const TSize& margin)
    {CHECKX(false, _T("Not implemented")); InUse(margin);}

    void PaintTabFace(const TRect& tabBoundingRect, TPoint (&pt)[4], bool isSelectedTab)
    {CHECKX(false, _T("Not implemented")); InUse(tabBoundingRect); InUse(pt); InUse(isSelectedTab);}

    void DrawTabContour(const TPoint (&pt)[4], bool isSelectedTab)
    {CHECKX(false, _T("Not implemented")); InUse(pt); InUse(isSelectedTab);}

    void PaintTabIcon(const TNoteTabItem& tab, const TCelArray& celArray, TColor transparentColor, const TSize& labelMargin, bool isSelectedTab, int selectedTabProtrusion)
    {
      CHECK(!tab.Rect.IsEmpty());
      int dy = isSelectedTab ? selectedTabProtrusion : 0;
      TRect imageRect(tab.Rect.TopLeft() + TSize(0, dy), tab.Rect.BottomRight());
      TUIFace face(imageRect, celArray, transparentColor);
      TSize size = celArray.CelSize();
      TRect srcRect(celArray.CelRect(tab.ImageIdx));
      TPoint dstPt(
        (tab.ImageLoc == alRight) ? imageRect.Width() - (size.cx + labelMargin.cx) : labelMargin.cx,
        (imageRect.Height() - size.cy) / 2);
      face.Paint(Dc, srcRect, dstPt, TUIFace::Normal, false, false);
    }

    void DrawTabText(const TNoteTabItem& tab, const TCelArray* celArray, const TSize& labelMargin, int labelImageMargin, bool isSelectedTab, int selectedTabProtrusion)
    {
      bool hasImage = celArray && tab.ImageIdx >= 0;
      int leftMargin = tab.Rect.left + labelMargin.cx;
      int rightMargin = tab.Rect.right - labelMargin.cx;
      int topMargin = labelMargin.cy + (isSelectedTab ? selectedTabProtrusion : 0);
      TRect labelRect(
        (hasImage && tab.ImageLoc == alLeft) ? (leftMargin + celArray->CelSize().cx + labelImageMargin) : leftMargin,
        tab.Rect.top + topMargin,
        (hasImage && tab.ImageLoc == alRight) ? (rightMargin - celArray->CelSize().cx - labelImageMargin) : rightMargin,
        tab.Rect.bottom - labelMargin.cy);
      uint16 labelAlign = static_cast<uint16>(!hasImage ? DT_CENTER : (tab.ImageLoc == alRight ? DT_RIGHT : DT_LEFT));
      uint16 labelFormat = static_cast<uint16>(DT_SINGLELINE | DT_VCENTER | labelAlign);
      int oldBkMode = Dc.SetBkMode(TRANSPARENT);
      Dc.SetTextColor(TabTextColor);
      Dc.SelectObject(isSelectedTab ? SelectedTabFont : TabFont);
      Dc.DrawText(tab.Label, -1, labelRect, labelFormat);
      Dc.SetBkMode(oldBkMode);
    }

    void DrawTabFocusRect(const TNoteTabItem& tab, const TSize& focusMargin, int selectedTabProtrusion)
    {
      DrawTabFocusRect(Dc, tab, focusMargin, selectedTabProtrusion);
    }

    static void DrawTabFocusRect(TDC& dc, const TNoteTabItem& tab, const TSize& focusMargin, int selectedTabProtrusion)
    {
      int oldBkMode = dc.SetBkMode(TRANSPARENT);
      TRect selectedRect = TRect(
        tab.Rect.TopLeft().OffsetBy(0, selectedTabProtrusion),
        tab.Rect.BottomRight())
        .InflatedBy(-focusMargin);
      dc.DrawFocusRect(selectedRect);
      dc.SetBkMode(oldBkMode);
    }

  protected:

    //
    // Utilities for concrete renderers.
    //

    void FillPageEdge(const TSize& margin, const TBrush& brush)
    {
      TRect r = Window.GetClientRect();
      TRect m(r.TopLeft(), TSize(r.Width(), margin.cy));
      Dc.FillRect(m, brush);
    }

    void DrawPageEdge(const TSize& margin, const TPen& pen)
    {
      TRect r = Window.GetClientRect();
      TRect m(r.TopLeft(), TSize(r.Width(), margin.cy));
      Dc.SelectObject(pen);
      Dc.MoveTo(m.BottomLeft());
      Dc.LineTo(m.BottomRight());
    }

    void FillPolygon(const TPoint (&pt)[4], const TBrush& brush)
    {
      Dc.SelectStockObject(NULL_PEN);
      Dc.SelectObject(brush);
      Dc.Polygon(pt, COUNTOF(pt));
    }

    void DrawContour(const TPoint (&pt)[4], const TPen& pen)
    {

#if defined(OWL_GDIPLUS_H)

      // Draw anti-aliased lines using GDI+.
      //
      Gdiplus::Graphics g(Dc);
      Gdiplus::Pen gdiPlusPen(gdiplus_cast<Gdiplus::Color>(TColor(pen.GetObject().lopnColor)));
      Gdiplus::Point points[COUNTOF(pt)];
      std::transform(pt, pt + COUNTOF(pt), points, &gdiplus_cast<Gdiplus::Point, TPoint>);
      g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
      g.DrawLines(&gdiPlusPen, points, COUNTOF(points));

#else

      Dc.SelectObject(pen);
      Dc.Polyline(pt, COUNTOF(pt));
      Dc.SetPixel(pt[3], pen.GetObject().lopnColor); // We want that last pixel as well!

#endif

    }

  };

  struct TFlatRenderer_
    : TRenderer_
  {
    TBrush TabBrush;
    TPen EdgePen;

    TFlatRenderer_(
      TWindow& w,
      TDC& dc,
      const TRect& paintRect,
      const TFont& tabFont,
      const TFont& selectedTabFont,
      TColor tabTextColor,
      TColor tabColor,
      TColor selectedTabColor,
      TColor edgeColor
      )
      : TRenderer_(w, dc, paintRect, tabFont, selectedTabFont, tabTextColor, tabColor, selectedTabColor, edgeColor),
      TabBrush(tabColor),
      EdgePen(edgeColor)
    {}

    void PaintPageEdge(const TSize& margin) // non-virtual override
    {
      FillPageEdge(margin, TabBrush);
      DrawPageEdge(margin, EdgePen);
    }

    void PaintTabFace(const TRect& tabBoundingRect, TPoint (&pt)[4], bool isSelectedTab)
    {
      InUse(tabBoundingRect); InUse(isSelectedTab); // non-virtual override
      FillPolygon(pt, TabBrush);
    }

    void DrawTabContour(const TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      InUse(isSelectedTab);
      DrawContour(pt, EdgePen);
    }

  };

  struct TWindowFaceRenderer_
    : TFlatRenderer_
  {
    TBrush SelectedTabBrush;

    TWindowFaceRenderer_(
      TWindow& w,
      TDC& dc,
      const TRect& paintRect,
      const TFont& tabFont,
      const TFont& selectedTabFont,
      TColor tabTextColor,
      TColor tabColor,
      TColor selectedTabColor,
      TColor edgeColor
      )
      : TFlatRenderer_(w, dc, paintRect, tabFont, selectedTabFont, tabTextColor, tabColor, selectedTabColor, edgeColor),
      SelectedTabBrush(selectedTabColor)
    {}

    void PaintPageEdge(const TSize& margin) // non-virtual override
    {
      FillPageEdge(margin, SelectedTabBrush);
      DrawPageEdge(margin, EdgePen);
    }

    void PaintTabFace(const TRect& tabBoundingRect, TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      InUse(tabBoundingRect);
      FillPolygon(pt, isSelectedTab ? SelectedTabBrush : TabBrush);
    }

  };

  struct TStyle3dRenderer_
    : TRenderer_
  {
    TBrush FaceBrush;
    TPen LightPen;
    TPen HilightPen;
    TPen ShadowPen;
    TPen DkShadowPen;

    TStyle3dRenderer_(
      TWindow& w,
      TDC& dc,
      const TRect& paintRect,
      const TFont& tabFont,
      const TFont& selectedTabFont,
      TColor tabTextColor,
      TColor tabColor,
      TColor selectedTabColor,
      TColor edgeColor
      )
      : TRenderer_(w, dc, paintRect, tabFont, selectedTabFont, tabTextColor, tabColor, selectedTabColor, edgeColor),
      FaceBrush(tabColor),
      LightPen(TColor::Sys3dLight),
      HilightPen(TColor::Sys3dHilight),
      ShadowPen(TColor::Sys3dShadow),
      DkShadowPen(TColor::Sys3dDkShadow)
    {}

    void PaintPageEdge(const TSize& margin) // non-virtual override
    {
      FillPageEdge(margin, FaceBrush);

      // Draw a recessed frame around the tabs.
      //
      TRect r = Window.GetClientRect();
      TRect f(
        r.TopLeft().OffsetBy(0, margin.cy),
        r.BottomRight());
      TUIBorder(f, TUIBorder::WndRecessed).Paint(Dc);
    }

    void PaintTabFace(const TRect& tabBoundingRect, TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      InUse(tabBoundingRect); InUse(isSelectedTab);
      FillPolygon(pt, FaceBrush);
    }

    void DrawTabContour(const TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      InUse(isSelectedTab);

      Dc.SelectObject(LightPen); // inside left
      Dc.MoveTo(pt[0].x + 1, pt[0].y);
      Dc.LineTo(pt[1].x + 1, pt[1].y - 1);

      Dc.SelectObject(HilightPen); // outside left
      Dc.MoveTo(pt[0].x, pt[0].y);
      Dc.LineTo(pt[1].x, pt[1].y);

      Dc.SelectObject(ShadowPen); // inside bottom & right
      Dc.MoveTo(pt[1].x + 1, pt[1].y - 1);
      Dc.LineTo(pt[2].x - 1, pt[2].y - 1);
      Dc.MoveTo(pt[2].x - 1, pt[2].y);
      Dc.LineTo(pt[3].x - 1, pt[3].y);

      Dc.SelectObject(DkShadowPen); // outside bottom & right
      Dc.MoveTo(pt[1].x + 1, pt[1].y);
      Dc.LineTo(pt[2].x, pt[2].y);
      Dc.LineTo(pt[3].x, pt[3].y);
    }

  };

  struct TThemeRenderer_
    : TRenderer_
  {
    TPen EdgePen;

    TThemeRenderer_(
      TWindow& w,
      TDC& dc,
      const TRect& paintRect,
      const TFont& tabFont,
      const TFont& selectedTabFont,
      TColor tabTextColor,
      TColor tabColor,
      TColor selectedTabColor,
      TColor edgeColor
      )
      : TRenderer_(w, dc, paintRect, tabFont, selectedTabFont, tabTextColor, tabColor, selectedTabColor, edgeColor),
      EdgePen(edgeColor)
    {}

    void PaintPageEdge(const TSize& margin) // non-virtual override
    {
      TRect r = Window.GetClientRect();
      TRect m(r.TopLeft(), TSize(r.Width(), margin.cy));
      TThemePart p(Window, L"TAB", TABP_BODY, 0);
      p.DrawBackground(Dc, m);
      DrawPageEdge(margin, EdgePen);
    }

    void PaintTabFace(const TRect& tabBoundingRect, TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      int item = isSelectedTab ? TABP_TOPTABITEM : TABP_TABITEM;
      int state = isSelectedTab ? TIS_SELECTED : TIS_NORMAL;
      TThemePart p(Window, L"TAB", item, state);
      TRegion savedClipRegion;
      int r = Dc.GetClipRgn(savedClipRegion); CHECK(r != -1);
      TRegion clipRegion(pt, COUNTOF(pt), WINDING);
      Dc.SelectClipRgn(clipRegion);
      p.DrawBackground(Dc, tabBoundingRect.InflatedBy(0, 1)); // Inflate to not paint the edge.
      Dc.SelectClipRgn(r == 1 ? savedClipRegion.GetHandle() : NULL);
    }

    void DrawTabContour(const TPoint (&pt)[4], bool isSelectedTab) // non-virtual override
    {
      InUse(isSelectedTab);
      DrawContour(pt, EdgePen);
    }

  };

} // namespace

//
/// Implements the rendering of the window, using the given part renderer.
//
template <class TPartRenderer>
void TNoteTab::PaintTabs(TDC& dc, const TRect& paintRect)
{
  if (GetCount() == 0) return;

  TPartRenderer renderer(
    *this,
    dc,
    paintRect,
    GetTabFont(),
    GetSelectedTabFont(),
    TColor::SysWindowText,
    TabColor,
    SelectedTabColor,
    EdgeColor);

  // Now, go through the tab list in reverse order and paint each tab, except for
  // the selected one, which has its painting deferred to the end (since it overlaps
  // both its neighbours). We iterate in reverse order so that the tabs overlap
  // correctly from left to right.
  //
  for (int i = GetCount(); --i >= -1;)
  {
    // Skip the selected tab; it's drawn last. Otherwise get the working index.
    //
    if (i == GetSel()) continue;
    const bool isSelectedTab = (i == -1);

    // If this is the final item (selected tab), then draw the page edge.
    //
    if (isSelectedTab)
      renderer.PaintPageEdge(Margin);

    // Retrieve tab item information.
    //
    const TNoteTabItem& tab = TabList[isSelectedTab ? GetSel() : i];
    TRect tabBoundingRect = GetBoundingRect(tab.Rect);
    if (tabBoundingRect.IsNull()) continue;
    CHECK(!tabBoundingRect.IsEmpty());

    // If the tab is completely outside the painting area, then skip it.
    // Note that we inflate the paint area by 1 here to avoid edge conditions,
    // i.e. whether or not a shared edge constitutes touching.
    //
    if (!(tabBoundingRect.InflatedBy(1, 1).Touches(paintRect)))
      continue;

#if OWL_CLIP_TAB_BOUNDING_RECT_

    struct TTabBoundingRectClipper
    {
      TDC& Dc;
      TRegion SavedClipRegion;
      bool DidSaveClipRegion;

      TTabBoundingRectClipper(TDC& d, const TRect& b)
        : Dc(d), SavedClipRegion(), DidSaveClipRegion(false)
      {
        if (!ShouldClipTabBoundingRect_) return;
        int rc = Dc.GetClipRgn(SavedClipRegion);
        CHECK(rc != -1);
        DidSaveClipRegion = rc == 1;
        int ri = Dc.IntersectClipRect(b);
        CHECK(ri != ERROR);
      }

      ~TTabBoundingRectClipper()
      {
        if (!ShouldClipTabBoundingRect_) return;
        int r = DidSaveClipRegion ?
          Dc.SelectClipRgn(SavedClipRegion) :
          Dc.RemoveClipRgn();
        CHECK(r != ERROR);
      }
    }
    tbrc_(dc, tabBoundingRect);

#endif

    // Define the contour of the tab.
    //
    // Note that we exclude the edge on the right and bottom of our bounding
    // rectangle, as per Windows drawing conventions. See the documentation
    // for GDI, e.g. for the Rectangle function:
    //
    // "The rectangle that is drawn excludes the bottom and right edges."
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd162898.aspx
    //
    // By doing so we draw only within the clipping rectangle that would be
    // visible if we had intersected the clipping region with the tab's
    // bounding rectangle. This ensures that invalidation of our bounding
    // rectangle covers all pixels drawn.
    //
    int dx = TabTapering; // amount of narrowing on each side
    TPoint pt[4] =
    {
      tabBoundingRect.TopLeft(),
      tabBoundingRect.BottomLeft().OffsetBy(dx, -1),
      tabBoundingRect.BottomRight().OffsetBy(-dx - 1, -1),
      tabBoundingRect.TopRight().OffsetBy(-1, 0)
    };

    // Fill the face of the tab, draw the contour and paint the label.
    // If the note tab has input focus, also draw the focus rectangle.
    //
    renderer.PaintTabFace(tabBoundingRect, pt, isSelectedTab);
    renderer.DrawTabContour(pt, isSelectedTab);
    if (isSelectedTab && GetFocus() == GetHandle())
      renderer.DrawTabFocusRect(tab, FocusMargin, SelectedTabProtrusion);
    bool hasImage = CelArray && tab.ImageIdx >= 0;
    if (hasImage)
      renderer.PaintTabIcon(tab, *CelArray, TransparentColor, LabelMargin, isSelectedTab, SelectedTabProtrusion);
    renderer.DrawTabText(tab, CelArray, LabelMargin, LabelImageMargin, isSelectedTab, SelectedTabProtrusion);
  }

#if OWL_DRAW_TAB_BOUNDING_RECT_

  if (ShouldDrawTabBoundingRect_)
    for (int i = 0; i != GetCount(); ++i)
    {
      const TNoteTabItem& tab = TabList[i];
      if (tab.Rect.IsNull()) continue;

      // Use Polyline for accuracy; Rectangle excludes right and bottom edge.
      //
      TPen tabRectPen(TColor::LtRed, 0, PS_DOT);
      dc.SelectObject(tabRectPen);
      TRect b = GetBoundingRect(tab.Rect);
      TPoint bp[] = {b.TopLeft(), b.BottomLeft(), b.BottomRight(), b.TopRight(), b.TopLeft()};
      dc.Polyline(bp, COUNTOF(bp));
    }

#endif

  dc.RestorePen();
  dc.RestoreBrush();
  dc.RestoreFont();
}

//
/// TWindow::Paint override
//
void
TNoteTab::Paint(TDC& dc, bool erase, TRect& paintRect)
{
  const auto bkgndColor = GetBkgndColor();
  if (erase && bkgndColor != TColor::Transparent)
  {
    const auto color = (bkgndColor != TColor::None) ? bkgndColor : TColor::Sys3dFace;
    if (GetCount() == 0)
      dc.FillSolidRect(paintRect, color);
    else
    {
      // First create a region consisting of the invalidated parts of the tabs
      // area, i.e. excluding the top margin, which will be painted later.
      // Then clip this against the tabs, since these will be painted anyway.
      // This leaves only the invalidated parts around the tabs. Note that we
      // do not exclude the scroller, since we rely on WS_CLIPCHILDREN.
      //
      TRect c = GetClientRect();
      TRect tabsArea(c.TopLeft().OffsetBy(0, Margin.cy), c.BottomRight());
      TRegion r(paintRect & tabsArea);
      for (TNoteTabItemArray::const_iterator i = TabList.begin(); i != TabList.end(); ++i)
      {
        const TNoteTabItem& t = *i;
        CHECK(t.Rect.IsNull() || !t.Rect.IsEmpty());
        r -= t.Rect;
      }
      dc.FillRgn(r, TBrush{color}); // May be NULLREGION at this point.
    }
  }

  if (GetCount() == 0) return;

  if (ShouldUseThemes && IsThemed_())
    PaintTabs<TThemeRenderer_>(dc, paintRect);
  else if (Style3d)
    PaintTabs<TStyle3dRenderer_>(dc, paintRect);
  else if (WindowFace)
    PaintTabs<TWindowFaceRenderer_>(dc, paintRect);
  else
    PaintTabs<TFlatRenderer_>(dc, paintRect);

  if (ScrollButtons->IsWindowVisible())
  {
    ScrollButtons->Invalidate(false);
    ScrollButtons->UpdateWindow();
  }

  LastClientRectPainted = GetClientRect(); // See EvSize.
}

//
/// WM_SIZE handler - Relay tab items
//
void
TNoteTab::EvSize(uint sizeType, const TSize& size)
{
  TControl::EvSize(sizeType, size);

  // Unless the layout of the tabs changes in response to the size change,
  // the only stale part of the old painting is possibly the right and bottom
  // edges, which are only drawn for 3D style. These need to be invalidated if
  // the width or height of the window increases.
  //
  if (Style3d)
  {
    const auto getRightEdge = [](const TRect& r) { return TRect{r.TopRight().OffsetBy(-TUIMetric::CyEdge, 0), r.BottomRight()}; };
    const auto getBottomEdge = [](const TRect& r) { return TRect{r.BottomLeft().OffsetBy(0, -TUIMetric::CxEdge), r.BottomRight()}; };

    const auto n = GetClientRect();
    const auto& p = LastClientRectPainted;
    const auto hasPaintedBefore = !p.IsNull();

    if (hasPaintedBefore && n.Width() > p.Width()) // Increasing width; invalidate old right edge.
      InvalidateRect(getRightEdge(p));
    else if (!hasPaintedBefore || n.Width() < p.Width()) // New or decreasing width; invalidate new right edge.
      InvalidateRect(getRightEdge(n));

    if (hasPaintedBefore && n.Height() > p.Height()) // Inreasing height; invalidate old bottom edge.
      InvalidateRect(getBottomEdge(p));
    else if (!hasPaintedBefore || n.Height() < p.Height()) // New or decreasing height; invalidate new bottom edge.
      InvalidateRect(getBottomEdge(n));
  }

  // Layout tab items.
  //
  int s = GetSel();
  bool hasVisibleSel = GetCount() > 0 && s >= 0 && IsVisible(s);
  SetTabRects(FirstVisibleTab);
  if (hasVisibleSel)
    EnsureVisible(s);
}

//
/// WM_LBUTTONDOWN handler - Checks whether the mouse was clicked on a tab item and
/// selects it.
///
/// \note  A notification is sent to the parent before and after selecting the tab.
/// The parent may choose to veto the selection after receiving the first
/// notification.
//
void
TNoteTab::EvLButtonDown(uint /*modKeys*/, const TPoint& point)
{
  PRECONDITION(ScrollButtons);
  if (ScrollButtons->IsWindowVisible() && GetScrollerArea().Contains(point)) return;

  SetFocus();

  const auto hitIndex = TabFromPoint(point);
  if (hitIndex != -1)
    NotifyAndSelect(hitIndex);
}

//
/// Handle WM_SETFOCUS: Draw focus to identify selected tab.
//
void
TNoteTab::EvSetFocus(THandle hWndLostFocus)
{
  PRECONDITION(SelectedTab >= 0 && SelectedTab < GetCount());
  TControl::EvSetFocus(hWndLostFocus);
  InvalidateRect(TabList[SelectedTab].Rect);
  UpdateWindow();
}

//
/// Handle WM_KillFOCUS: Remove dotted focus rectangle from selected tab.
//
void
TNoteTab::EvKillFocus(THandle hWndGetFocus)
{
  PRECONDITION(SelectedTab >= 0 && SelectedTab < GetCount());
  TControl::EvKillFocus(hWndGetFocus);
  InvalidateRect(TabList[SelectedTab].Rect);
  UpdateWindow();
}

//
/// WM_GETDLGCODE handler - Informs dialog manager that arrow keys are to be used.
//
uint
TNoteTab::EvGetDlgCode(const MSG* msg)
{
  return TControl::EvGetDlgCode(msg) | DLGC_WANTARROWS;
}

//
/// WM_KEYDOWN handler - handles arrow keys to allow user to navigate through tab
/// items.
//
void
TNoteTab::EvKeyDown(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  int tab = GetSel();
  if (tab < 0) return;
  CHECK(tab < GetCount());

  if (GetKeyState(VK_CONTROL) & 0x8000) switch (key)
  {
    case VK_RIGHT:
      EvHScroll(SB_LINERIGHT, 0, nullptr);
      break;

    case VK_LEFT:
      EvHScroll(SB_LINELEFT, 0, nullptr);
      break;
  }
  else switch (key)
  {
    case VK_RIGHT:
    {
      int last = GetCount() - 1;
      if (tab < last)
      {
        if (NotifyAndSelect(++tab))
        {
          EnsureVisible(tab);
          while (!IsFullyVisible(tab) && GetFirstVisibleTab() < last)
            SetFirstVisibleTab(GetFirstVisibleTab() + 1);
        }
      }
      break;
    }
    case VK_LEFT:
    {
      if (tab > 0)
      {
        if (NotifyAndSelect(--tab))
          EnsureVisible(tab);
      }
      break;
    }

#if OWL_CLIP_TAB_BOUNDING_RECT_

    case VK_F2:
      ShouldClipTabBoundingRect_ = !ShouldClipTabBoundingRect_;
      Invalidate();
      break;

#endif

#if OWL_DRAW_TAB_BOUNDING_RECT_

    case VK_F3:
      ShouldDrawTabBoundingRect_ = !ShouldDrawTabBoundingRect_;
      Invalidate();
      break;

#endif

  }
}

//
/// Updates the internal information stored about the label of a particular tab
/// item.
//
void
TNoteTab::SetTabSize(int index)
{
  PRECONDITION(TabFont);

  const TFont& selectedTabFont = GetSelectedTabFont();

  // Compute size of label.
  // Accommodate the largest font in both dimensions.
  // Use screen DC (may be called before 'HWND').
  //
  TScreenDC dc;
  TNoteTabItem& tab = TabList[index];
  static const tchar overhangSet[] = _T("dfkmVWY"); // TODO: Add complete set, or query font.
  bool hasOverhang = !tab.Label.empty() &&
    (TabFont->GetObject().lfItalic || selectedTabFont.GetObject().lfItalic) &&
    find(begin(overhangSet), end(overhangSet), tab.Label.back()) != end(overhangSet);
  tstring s = tab.Label + (hasOverhang ? _T(" ") : _T(""));
  TRect regularExtent = TRect(TPoint(), TabFont->GetTextExtent(dc, s));
  TRect selectedExtent = TRect(TPoint(), selectedTabFont.GetTextExtent(dc, s));
  tab.LabelSize = (regularExtent | selectedExtent).Size();

  // Add image dimensions to LabelSize.
  //
  if (tab.ImageIdx >= 0 && CelArray)
  {
    TSize size = CelArray->CelSize();
    tab.LabelSize.cx += size.cx + LabelImageMargin;
    tab.LabelSize.cy = std::max(tab.LabelSize.cy, size.cy);
  }
}

//
/// Calculates the tab rectangle for the given tab and position.
//
TRect
TNoteTab::CalculateTabRect(const TNoteTabItem& tab, const TPoint& p, bool isSelected) const
{
  TSize padding(2 * LabelMargin.cx, 2 * LabelMargin.cy + (isSelected ? SelectedTabProtrusion : 0));
  return TRect(p.OffsetBy(TabTapering, 0), tab.LabelSize + padding);
};


//
/// Calculates and returns a list of updated tab hit rectangles based on the
/// given start index and the given area for the tabs.
//
TNoteTab::TRects
TNoteTab::CalculateTabRects(int firstTab, const TRect& area) const
{
  TRects rects;
  rects.reserve(GetCount());

  // Reset all the tab rectangles preceding the first visible tab.
  // Value-initialize these by calling `resize` (i.e. insert empty rectangles).
  //
  rects.resize(std::min(firstTab, GetCount()));

  // Assign rectangles to all visible tabs.
  //
  TPoint p = area.TopLeft();
  for (int i = firstTab; i != GetCount(); ++i)
  {
    const TNoteTabItem& tab = TabList[i];
    const TRect r = CalculateTabRect(tab, p, i == SelectedTab);

    // If the tab is completely outside the tabs area, then break. In other
    // words; include partially visible tabs.
    //
    const TRect b = GetBoundingRect(r);
    bool isVisible = (b.left < area.right);
    if (!isVisible)
    {
      // Reset the rectangles for the remaining tabs, then exit.
      //
      rects.resize(GetCount());
      break;
    }
    rects.push_back(r);
    p.Offset(r.Width() + TabSpacing, 0); // Move to next tab position.
  }
  return rects;
}


//
/// Assigns the given new rectangle to the given tab item.
/// Both old and new rectangle is invalidated in the client area.
//
void
TNoteTab::AssignTabRect(TNoteTabItem& tab, const TRect& newRect)
{
  TRect& r = tab.Rect;
  if (r == newRect) return;
  if (GetHandle() && !r.IsNull())
    InvalidateRect(GetBoundingRect(r));
  r = newRect;
  if (GetHandle() && !r.IsNull())
    InvalidateRect(GetBoundingRect(r));
}

namespace
{

  //
  // Returns true if the horizontal projection of the given rect `r`
  // lies within the horizontal projection of the given rect `area`.
  //
  bool ContainsHorizontalExtents_(const TRect& area, const TRect& r)
  {
    if (area.IsEmpty() || r.IsEmpty()) return false;
    return r.left >= area.left && r.right <= area.right;
  }

} // namespace

//
/// Lays out tab items (and scroll buttons) with the specified index at the leftmost.
/// Scroll buttons are enabled when needed (unless the scroll location is set to alNone).
//
void TNoteTab::SetTabRects(int firstTab)
{
  const auto tabCount = GetCount();
  PRECONDITION((tabCount == 0 && firstTab == 0) || (firstTab >= 0 && firstTab < tabCount));
  if (!GetHandle()) return;

  // First, calculate tab layout with no scroll buttons present. If we have multiple tabs, and all
  // tabs did not fit, then we need to reserve space for the scroll buttons (unless ScrollLoc ==
  // alNone), so calculate the layout anew within the dedicated tabs area. Finally, store the
  // effective tabs area, and assign tab rectangles for the new layout.
  //
  const auto c = GetClientRect();
  const auto allArea = TRect{c.TopLeft() + Margin, c.BottomRight()};
  const auto scrollingTabsArea = GetScrollingTabsArea();
  const auto scrollerArea = GetScrollerArea();
  auto newRects = CalculateTabRects(firstTab, allArea);
  const auto needScrolling = ScrollLoc != alNone && tabCount > 1 &&
    (
    firstTab != 0 || // Invisible tab to the left.
    newRects.back().IsNull() || // Invisible tab to the right.
    !ContainsHorizontalExtents_(allArea, GetBoundingRect(newRects.back())) // Tab does not fit.
    );
  if (needScrolling)
    newRects = CalculateTabRects(firstTab, scrollingTabsArea);
  EffectiveTabsArea = needScrolling ? scrollingTabsArea : allArea;
  CHECK(static_cast<int>(newRects.size()) == tabCount);
  for (auto i = 0; i != tabCount; ++i)
    AssignTabRect(TabList[i], newRects[i]);

  // If we need scrolling, then calculate the upper index of the scroll range. Then update and show
  // the scroll buttons. Otherwise, hide the scroll buttons.
  //
  CHECK(ScrollButtons && ScrollButtons->GetHandle());
  if (needScrolling)
  {
    const auto countNotFullyVisibleTabs = [&]
    {
      PRECONDITION(tabCount > 0);
      PRECONDITION(firstTab >= 0 && firstTab < tabCount);
      auto n = firstTab; // Invisible tabs to the left.
      for (auto i = tabCount - 1; i >= firstTab && !IsFullyVisible(i); --i)
        ++n; // Invisible (or partially visible) tab to the right.
      CHECK(n >= 0 && n <= tabCount);
      return n;
    };
    const auto upper = min(countNotFullyVisibleTabs(), tabCount - 1);
    CHECK(upper > 0 && upper < tabCount);
    ScrollButtons->SetRange(0, upper);
    ScrollButtons->SetPos(firstTab);
    ScrollButtons->MoveWindow(scrollerArea, true);
    ScrollButtons->ShowWindow(SW_SHOW);
    ScrollButtons->UpdateWindow();
  }
  else
    ScrollButtons->ShowWindow(SW_HIDE);
}

//
/// Returns the index of the tab item at the specified window coordinate. Returns -1
/// on failure.
//
int
TNoteTab::TabFromPoint(const TPoint& pt) const
{
  for (int i = 0; i < GetCount(); i++)
  {
    const TNoteTabItem& tab = TabList[i];
    if (tab.Rect.Contains(pt))
      return i;
  }
  return -1;
}

//
/// Selects the tab at the given index and sends the appropriate notifications.
/// Returns false if the change was vetoed by the buddy/parent, true otherwise.
/// Does nothing and returns false if the tab at the given index is already selected.
//
bool
TNoteTab::NotifyAndSelect(int index)
{
  PRECONDITION(index >= 0);
  PRECONDITION(index < GetCount());

  if (index == SelectedTab) return false;

  HWND receiver = Buddy ? BuddyHandle : ::GetParent(GetHandle());

  // First notify that we're about to change selection.
  //
  TNotify nSelChanging(*this, Attr.Id, TCN_SELCHANGING);
  bool wasVetoed = SendNotification(receiver, Attr.Id, nSelChanging);
  if (wasVetoed) return false;

  // Set new selection and notify that it has changed.
  //
  SetSel(index);
  TNotify nSelChange(*this, Attr.Id, TCN_SELCHANGE);
  SendNotification(receiver, Attr.Id, nSelChange);
  return true;
}

//
// Refreshes the drawing of the control.
// If the control has not been created yet, does nothing.
//
void
TNoteTab::Update()
{
  if (!GetHandle()) return;
  for (int i = 0; i != GetCount(); ++i)
    SetTabSize(i);
  SetTabRects(GetFirstVisibleTab());
  Invalidate();
}

//
/// Returns the desired location of the scrollers within the tab.
//
TRect
TNoteTab::GetScrollerArea() const
{
  TRect rect = GetClientRect();
  rect.bottom -= TUIMetric::CyFixedFrame;
  rect.top = std::max(
    rect.top + Margin.cy + TUIMetric::CyFixedFrame,
    rect.bottom - TUIMetric::CyHScroll);
  switch (ScrollLoc)
  {
    default:
    case alNone:
      rect.left = rect.right = 0;
      break;

    case alLeft:
      rect.left += TUIMetric::CxFixedFrame;
      rect.right = rect.left + 2 * TUIMetric::CxHScroll;
      break;

    case alRight:
      rect.left = rect.right - 2 * TUIMetric::CxHScroll;
      rect.right -= TUIMetric::CxFixedFrame;
      break;
  }
  return rect;
}

//
/// Returns the rectangle of the area reserved for tabs when scrolling is active.
//
TRect
TNoteTab::GetScrollingTabsArea() const
{
  // First retrieve left, top and right borders
  //
  TRect rect = GetClientRect();
  rect.left += Margin.cx;
  rect.top += Margin.cy;
  TRect scrollArea = GetScrollerArea();
  switch (ScrollLoc)
  {
    default:
    case alRight:
      rect.right -= scrollArea.Width();
      break;
    case alLeft:
      rect.left += scrollArea.Width();
      break;
  }
  return rect;
}

//
/// Returns true if the tab item at the specified index is visible.
/// Note that true is returned even if the tab is only partially visible.
/// Use IsFullyVisible to check for full visibility.
//
bool
TNoteTab::IsVisible(int index) const
{
  PRECONDITION(index < GetCount() && index >= 0);
  return TabList[index].Rect.IsNull() ? false : true;
}

//
/// Returns true if the horizontal projection of the tab item at the specified
/// index is contained within the horizontal projection of the current
/// effective tabs area. Vertical visibility is ignored.
//
bool
TNoteTab::IsFullyVisible(int index) const
{
  PRECONDITION(index < GetCount() && index >= 0);
  if (EffectiveTabsArea.IsEmpty()) return false;
  if (!IsVisible(index)) return false;
  TRect b = GetBoundingRect(TabList[index].Rect);
  CHECK(!b.IsEmpty());
  return ContainsHorizontalExtents_(EffectiveTabsArea, b);
}

//
/// Sets FirstVisibleTab to index if index is valid.
//
void
TNoteTab::SetFirstVisibleTab(int index)
{
  if (index == GetFirstVisibleTab() || index < 0 || index >= GetCount()) return;
  FirstVisibleTab = index;
  SetTabRects(FirstVisibleTab);
}

//
/// If the tab specified by index is not visible, it is scrolled into view.
/// If the given index is -1, the index of the selected tab is used instead.
/// Returns true if successful, false on failure.
//
bool
TNoteTab::EnsureVisible(int index)
{
  PRECONDITION(GetCount() > 0);
  PRECONDITION(index >= -1 && index < GetCount());
  int i = (index == -1) ? GetSel() : index; CHECK(i >= 0);
  while (i != GetFirstVisibleTab() && !IsFullyVisible(i))
  {
    int v = GetFirstVisibleTab(); CHECK(v != i);
    SetFirstVisibleTab(v + (i < v ? -1 : +1));
  }
  return true;
}

//
/// Sets the first visible tab to the given thumb position.
//
void TNoteTab::EvHScroll(uint scrollCode, uint thumbPos, [[maybe_unused]] HWND hWndCtl)
{
  if (scrollCode == SB_THUMBPOSITION)
    SetFirstVisibleTab(thumbPos);
}

//
/// EV_WM_PAINT handler.
//
void
TNoteTab::EvPaint()
{
  TControl::EvPaint();
}

//
/// Disables automatic background erasure by returning `false`.
//
bool
TNoteTab::EvEraseBkgnd(HDC)
{
  return false;
}

//
/// Set the scroller location.
//
void
TNoteTab::SetScrollLocation(TAbsLocation loc)
{
  if (ScrollLoc == loc)
    return;

  ScrollLoc = loc;

  SetTabRects(FirstVisibleTab);
}

//
/// Sets the bitmap array to be used for the tabs.
/// Use SetCelArrayTransparentColor to set the bitmap pixel color that should
/// not be drawn. The default color is TColor::Sys3dFace.
//
void
TNoteTab::SetCelArray(TCelArray* array, TAutoDelete del)
{
  CelArray = array;
  OwnedCelArray.reset(del == AutoDelete ? array : nullptr);
  Update();
}

//
/// Sets the color assigned to denote transparency in the bitmaps used for the
/// tabs (see SetCelArray). All the bitmap pixels of this color will be fully
/// transparent (i.e. not drawn).
//
void
TNoteTab::SetCelArrayTransparentColor(const TColor& c)
{
  TransparentColor = c;
  Update();
}

} // OWL namespace
/* ========================================================================== */

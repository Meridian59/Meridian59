//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1997 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TGadgetWindow and TGadgetWindowFont.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/gadget.h>
#include <owl/gadgetwi.h>
#include <owl/framewin.h>
#include <owl/decframe.h>
#include <owl/celarray.h>
#include <owl/tooltip.h>
#include <owl/uimetric.h>
#include <owl/commctrl.h>
#include <owl/theme.h>

#if defined(__TRACE) || defined(__WARN)
#  if !defined(OWL_PROFILE_H)
#    include <owl/profile.h>
#  endif
#endif

#include <stdio.h>

#include <algorithm>

using namespace std;

namespace owl {

//
// Diagnostic group for gadgets & gadget windows
//
OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlGadget, 1, 0);
DIAG_DECLARE_GROUP(OwlCmd);

const uint GadgetWindowTimerID = 0xBACA;
const uint GadgetWindowTimeOut = 1000;

DEFINE_RESPONSE_TABLE1(TGadgetWindow, TWindow)
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONUP,
  EV_WM_LBUTTONDBLCLK,
  EV_WM_RBUTTONDOWN,
  EV_WM_RBUTTONUP,
  EV_WM_MOUSEMOVE,
  EV_WM_WINDOWPOSCHANGING,
  EV_WM_SYSCOLORCHANGE,
  EV_WM_CREATETOOLTIP,
END_RESPONSE_TABLE;

uint TGadgetWindow::FlatStyle = NonFlatNormal;

//
// Calculate & return the height in pixels of a given number of points on
// the screen.
//
static inline int heightInPixels(int pointSize)
{
  TScreenDC  dc;
  return MulDiv(-pointSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
}

//
//
/// Constructs a TGadgetWindowFont interface object with a default point size of 10
/// picas without bold or italic typeface. By default, the constructor creates the
/// system font: a variable-width, sans-serif Helvetica.
//
TGadgetWindowFont::TGadgetWindowFont(int pointSize, bool bold, bool italic)
:
  TFont(TDefaultGUIFont().GetFaceName().c_str(),
        heightInPixels(pointSize),
        0, 0, 0,
        bold ? FW_BOLD : FW_NORMAL,
        TDefaultGUIFont().GetPitchAndFamily(),
        uint8(italic))
/*
  TFont("MS Sans Serif", heightInPixels(pointSize), 0, 0, 0,
        bold ? FW_BOLD : FW_NORMAL,
        uint8(VARIABLE_PITCH | FF_SWISS), uint8(italic))
*/
{
}

//
/// Construct a specialized gadget window font. Base it on an existing font
/// object.
//
TGadgetWindowFont::TGadgetWindowFont(const TFont& font)
:
  TFont(font)
{
}

//----------------------------------------------------------------------------

//
/// Creates a TGadgetWindow interface object with the default tile direction and
/// font and passes module with a default value of 0.
//
TGadgetWindow::TGadgetWindow(TWindow*       parent,
                             TTileDirection direction,
                             TFont*         font,
                             TModule*       module)
:
  WantTimer(false),TimerID(0), ThemeBackgroundMode(false)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, 0, module);

  // Make sure we don't paint or erase over any child windows
  //
  ModifyStyle(0,WS_CLIPCHILDREN);

  Capture = 0;
  AtMouse = 0;
#if !defined(BI_DBCS_SUPPORT)
  Font = font ? font : new TGadgetWindowFont(6); 
#else
  Font = font ? font : new TDefaultGUIFont(); 
#endif

  Direction = direction;

  // Shrink wrap the narrow axis
  //
#if defined(BI_COMP_GNUC)  
  TTileDirection dir = Direction;
  ShrinkWrapWidth  = dir != Horizontal;// As narrow as possible
  ShrinkWrapHeight = dir != Vertical;  // As short as possible
#else
  ShrinkWrapWidth  = ((int)Direction) != Horizontal;// As narrow as possible
  ShrinkWrapHeight = ((int)Direction) != Vertical;  // As short as possible
#endif
  HintMode   = PressHints;

  WideAsPossible = 0;  // Number of gadgets with "WideAsPossible" set
  DirtyLayout = true;

  // Compute the font height
  //
  TEXTMETRIC  metrics;
  Font->GetTextMetrics(metrics);
  FontHeight = metrics.tmHeight + metrics.tmExternalLeading;

  // Choose a default height based on the height of the font plus room
  // for a top and bottom border.
  // !CQ Really only useful for Horizontal layout if at all...
  //
  Attr.H = FontHeight;

  if (Attr.Style & WS_BORDER)
    Attr.H += 2 * TUIMetric::CyBorder;

  SetBkgndColor(TColor::Sys3dFace);

  SharedCels = 0;
  Tooltip = 0;
  WantTooltip = false;

  // Rectangular layout members
  // !CQ Use Attr.W now for desired width?
  //
  RowWidth  = 0;  // Don't know yet? Set externally. Never let get smaller than widest gadget
  RowMargin = 4;
}

//
/// Destructs this gadget window. Deletes the gadgets, the font and the shared
/// cel array
//
TGadgetWindow::~TGadgetWindow()
{
  delete Font;
  delete SharedCels;
}

//
/// Responds to WM_SYSCOLORCHANGE to let the gadgets update their UI colors, and to
/// let this gadget window update its background color.
/// 
/// \note This is an obsolete function retained for compatibility. New TWindow,
/// TColor, and other UI support makes this unecessary.
//
void
TGadgetWindow::EvSysColorChange()
{
  for (TGadget* g = Gadgets; g; g = g->NextGadget())
    g->SysColorChange();
}

//
/// During idle time, iterates over the Gadgets invoking their CommandEnable()
/// member function. Also detects lost mouse up for fly-over hint mode.
//
bool
TGadgetWindow::IdleAction(long idleCount)
{
  if (idleCount == 0) {

    // See if we missed a mouse move & still need to send a MouseLeave to a
    // gadget
    //
    if (AtMouse) {
      TPoint crsPoint;
      GetCursorPos(crsPoint);
      if (WindowFromPoint(crsPoint) != GetHandle())
        HandleMessage(WM_MOUSEMOVE, 0, MkParam2(-1,-1));  // nowhere
    }
  }

  // Let the gadgets do command enabling if they need to
  //
  for (TGadget* g = Gadgets; g; g = g->NextGadget())
    g->IdleAction(idleCount);

  // Chain to base implementation
  //
  return TWindow::IdleAction(idleCount);
}

//
/// Inform GadgetWindow to start a Timer notification.
///
/// Set timer - useful for gadgets that need regular update [Time/Date].
///
/// \note TGadgetWindow does not catch the WM_TIMER event. The purpose of 
///       the timer message it to cause message dispatching which eventually
///       leads to an 'IdleAction' call.
//
bool          
TGadgetWindow::EnableTimer()
{
  // Set flag and let 'SetupWindow' create timer
  //
  if (!GetHandle()) {
    WantTimer = true;
    return true;
  }

  // Set a timer if one's not enabled already
  //
  if (!TimerID) 
    TimerID = SetTimer(GadgetWindowTimerID, GadgetWindowTimeOut);

  return TimerID != 0;
}

//
/// Sets the width and height of the data members. By default, if the tile direction
/// is horizontal, ShrinkWrapWidth is false and ShrinkWrapHeight is true. Also by
/// default, if the direction is vertical, ShrinkWrapWidth is true and
/// ShrinkWrapHeight is false.
//
void
TGadgetWindow::SetShrinkWrap(bool shrinkWrapWidth, bool shrinkWrapHeight)
{
  ShrinkWrapWidth = shrinkWrapWidth;
  ShrinkWrapHeight = shrinkWrapHeight;
}

//
/// Called by a gadget that wants to capture the mouse
///
/// GadgetSetCapture reserves all mouse messages for the gadget window until the
/// capture is released. Although gadgets are always notified if a left button-down
/// event occurs within the rectangle, the derived gadget class must call
/// GadgetSetCapture if you want the gadget to be notified when a mouse drag and a
/// mouse button-up event occurs.
///
/// Fails if already captured
//
bool
TGadgetWindow::GadgetSetCapture(TGadget& gadget)
{
  if (Capture)
    return false;

  else {
    Capture = &gadget;
    SetCapture();
    return true;
  }
}

//
/// Releases the capture so that other windows can receive mouse messages.
///
/// Ignores other gadgets
//
void
TGadgetWindow::GadgetReleaseCapture(TGadget& gadget)
{
  if (&gadget == Capture) {
    Capture = 0;
    ReleaseCapture();
  }
}

//
/// Simulates menu selection messages so that ObjectWindows command processing can
/// display command hints for the given command id (CM_xxxx).
//
void
TGadgetWindow::SetHintCommand(int id)
{
  // Find our ancestor decorated frame [which potentially has a statusbar
  // to show hint messages]
  //
  TWindow* parent= GetParentO();
  TDecoratedFrame* frame= parent ? 
                          TYPESAFE_DOWNCAST(parent, TDecoratedFrame) : 0;
  while (parent && !frame) {
    parent = parent->GetParentO();
    if (parent)
      frame = TYPESAFE_DOWNCAST(parent, TDecoratedFrame);
  }

  // Forward command id to frame via fake WM_MENUSELECT messages
  //
  if (frame) {
    if (id > 0) {
      frame->HandleMessage(WM_MENUSELECT, id, 0);
    }
    else {
      // Send a menuselect w/ flags==0xFFFF and hMenu==0 to indicate end
      //
      frame->HandleMessage(WM_MENUSELECT, 0xFFFF0000, 0); // flags+item, hmenu
    }
    frame->HandleMessage(WM_ENTERIDLE, MSGF_MENU);
  }
}

//
/// Sets or changes the margins for the gadget window and calls LayoutSession().
//
void
TGadgetWindow::SetMargins(TMargins& margins)
{
  Margins = margins;
  LayoutSession();
}

//
/// Converts layout units to pixels. A layout unit is determined by dividing the
/// window font height by eight.
//
int
TGadgetWindow::LayoutUnitsToPixels(int units)
{
  const long  unitsPerEM = 8;

  return int((long(units) * FontHeight + unitsPerEM / 2) / unitsPerEM);
}

//
/// LayoutSession is typically called when a change occurs in the size of the
/// margins or gadgets, or when gadgets are added or deleted. LayoutSession calls
/// TileGadgets() to tile the gadgets in the specified direction and Invalidate() to
/// mark the area as invalid (needs repainting).
//
void
TGadgetWindow::LayoutSession()
{
  if (GetHandle())
    InvalidateRect(TileGadgets());
}

//
/// Get the desired size for this gadget window by performing a trial layout of
/// the gadgets without touching them.
///
/// If shrink-wrapping was requested, GetDesiredSize returns the size needed to
/// accommodate the borders and the margins of the widest and highest gadget;
/// otherwise, it returns the width and height in the window's Attr structure.
///
/// If you want to leave extra room for a specific look (for example, a separator
/// line between gadgets, a raised line, and so on), you can override this function.
/// However, if you override GetDesiredSize(), you will probably also need to override
/// GetInnerRect() to calculate your custom inner rectangle.
///
/// For Rectangular layout it is assumed that the EndOfRow gadgets are already
/// Set.
//
void
TGadgetWindow::GetDesiredSize(TSize& size)
{
  TLayoutInfo layout(NumGadgets);
  if (ShrinkWrapWidth || ShrinkWrapHeight)
    LayoutGadgets(Direction, layout);

  // Calculate the X dimension for both shrinkWrap and externally set
  //
  if (ShrinkWrapWidth)
    size.cx = layout.DesiredSize.cx;
  else
    size.cx = Attr.W;  // Not shrink wrapped, just use set size

  // Calculate the Y dimension for both shrinkWrap and externally set
  // Always shrinkwrap rectangular layout vertically
  //
  if (ShrinkWrapHeight) {
    if (NumGadgets == 0)
      size.cy = 0;
    else
      size.cy = layout.DesiredSize.cy;
  }
  else
    size.cy = Attr.H;  // Not shrink wrapped, just use set size
}

//
/// Updates the Size in Attr.W and Attr.H to match that obtained using
/// GetDesiredSize() for each dimension that is shrink-wrapped.
//
void
TGadgetWindow::UseDesiredSize()
{
  if (ShrinkWrapWidth || ShrinkWrapHeight) {
    TSize  size;
    GetDesiredSize(size);

    if (ShrinkWrapWidth)
      Attr.W = size.cx;
    if (ShrinkWrapHeight)
      Attr.H = size.cy;
  }
}

//
/// Overrides TWindow member function and chooses the initial size of the gadget if
/// shrink-wrapping was requested.
//
bool
TGadgetWindow::Create()
{
  UseDesiredSize();
  return TWindow::Create();
}

//
/// Sets the horizontal or vertical orientation of the gadgets. If the gadget window
/// is already created, SetDirection readjusts the dimensions of the gadget window
/// to fit around the gadgets.
/// 
/// The setting of the direction parameter is also related to the setting of the
/// second parameter (TLocation) in TDecoratedFrame's Insert function,. which
/// specifies where the decoration is added in relation to the frame window's client
/// window. If the second parameter in TDecoratedFrame::Insert() is set to top or
/// bottom, the direction parameter in SetDirection must be horizontal. If the
/// second parameter in TDecoratedFrame::Insert() is set to left or right, the
/// direction parameter in SetDirection must be vertical.
//
void
TGadgetWindow::SetDirection(TTileDirection direction)
{
#if defined(BI_COMP_GNUC)  
  TTileDirection dir = Direction;
  if (dir != direction || DirtyLayout) {
    if (dir != direction) {
#else
  if (((int)Direction) != direction || DirtyLayout) {

    // Swap margin's and ShrinkWrap's X & Y axis
    //
    if (((int)Direction) != direction) {
#endif
      // Margin swapping seems to not be used in the latest UIs
      //
#if 0
      int t = Margins.Left; Margins.Left = Margins.Top; Margins.Top = t;
      t = Margins.Right; Margins.Right = Margins.Bottom; Margins.Bottom = t;
#endif
      // NOTE: a limitation. Passing thru Rectangular will mess this swap up
      //
      bool sww = ShrinkWrapWidth;
      ShrinkWrapWidth = ShrinkWrapHeight;
      ShrinkWrapHeight = sww;

      Direction = direction;
      DirtyLayout = true;
    }

    // Get & use the new size & relayout if created
    //
    UseDesiredSize();
    LayoutSession();
  }
}

//
/// Set flat style options, or disable it.
//
void
TGadgetWindow::EnableFlatStyle(uint style)
{
  if(style == FlatDefault)
  {
    style = FlatStandard;
    uint32 v = TCommCtrl::Dll()->GetCtrlVersion();
    if (v > ComCtlVersionIE3)
      style |= FlatSingleDiv|FlatGrayButtons;
    if (v > ComCtlVersionIE4)
      style |= FlatHotText;
    if (v >= 0x60000)
      style |= FlatXPTheme;
  }
  FlatStyle = style;
}

//
// Returns true if the application is themed (Windows XP Visual Style).
//
bool 
TGadgetWindow::IsThemed() const 
{
  return (FlatStyle & FlatXPTheme) && TThemeModule::GetInstance().IsAppThemed();
}

//
// Enables or disables themed background.
//
void
TGadgetWindow::EnableThemeBackground(bool enable) 
{
	ThemeBackgroundMode = enable;
}

//
// Returns true if themed background is enabled.
//
bool
TGadgetWindow::IsThemeBackgroundEnabled() const
{
  return ThemeBackgroundMode;
}

//
// Returns true if themed background is enabled and themes are active.
//
bool 
TGadgetWindow::IsBackgroundThemed() const 
{
  return IsThemeBackgroundEnabled() && IsThemed();
}

//
/// Sets the maximum width for each row used for rectangular layout.
/// LayoutSession() or SetDirection() must be called for changes to take effect
//
void
TGadgetWindow::SetRectangularDimensions(int width, int /*height*/, int rowMargin)
{
  if (width != RowWidth || rowMargin >= 0 && rowMargin != RowMargin) {
    RowWidth = width;
    if (rowMargin >= 0)
      RowMargin = rowMargin;
    if (Direction == Rectangular)
      DirtyLayout = true;
  }
}

//
/// GetInnerRect computes the rectangle inside of the borders and margins of the
/// gadget.
/// 
/// If you want to leave extra room for a specific look (for example, a separator
/// line between gadgets, a raised line, and so on), you can override this function.
/// If you override GetInnerRect, you will probably also need to override
/// GetDesiredSize() to calculate your custom total size.
//
void
TGadgetWindow::GetInnerRect(TRect& rect)
{
  int left, right, top, bottom;
  GetMargins(Margins, left, right, top, bottom);

  if (Attr.W != 0) {
    rect.left = left;
    rect.right = Attr.W - right;
    if (Attr.Style & WS_BORDER)
      rect.right -= 2 * TUIMetric::CyBorder;
  }
  else {
    rect.left = 0;
    rect.right = 0;
  }
  if (Attr.H != 0) {
    rect.top = top;
    rect.bottom = Attr.H - bottom;
    if (Attr.Style & WS_BORDER)
      rect.bottom -= 2 * TUIMetric::CxBorder;
  }
  else {
    rect.top = 0;
    rect.bottom = 0;
  }
}

//
/// Virtual called by TileGadgets() for each gadget in order to give derived
/// classes a chance to modify the gadget positioning. Default is to overlap
/// adjacent plain-bordered gadgets
//
void
TGadgetWindow::PositionGadget(TGadget* previous, TGadget* next, TPoint& origin)
{
  // Overlap the button borders
  // !CQ make sure this is right for win95. Also in regular controlbar
  //
  if (previous->GetBorderStyle() == TGadget::Plain &&
      next->GetBorderStyle() == TGadget::Plain)
    if (Direction == Horizontal)
      origin.x -= TUIMetric::CxBorder;
    else
      origin.y -= TUIMetric::CyBorder;
}

//
/// Tiles the gadgets in the direction requested (horizontal or vertical). Calls
/// PositionGadget() to give derived classes an opportunity to adjust the spacing
/// between gadgets in their windows.
//
TRect
TGadgetWindow::TileGadgets()
{
  TLayoutInfo layout(NumGadgets);
  LayoutGadgets(Direction, layout);

  TRect invalidRect(0,0,0,0);
  int   i = 0;
  for (TGadget* gadget = Gadgets; gadget; gadget = gadget->Next, i++) {
    TRect  originalBounds = gadget->GetBounds();
    TRect  bounds = layout.GadgetBounds[i];
    if (originalBounds != bounds) {
      gadget->SetBounds(bounds);
      if (originalBounds.TopLeft() != TPoint(0, 0))
        invalidRect |= originalBounds;
      invalidRect |= bounds;
    }
  }
  DirtyLayout = false;
  return invalidRect;
}

//
/// Helper for LayoutGadgets() to calculate horizontal tiling
//
void
TGadgetWindow::LayoutHorizontally(TLayoutInfo& layout)
{
  TRect  innerRect;
  GetInnerRect(innerRect);

  int leftM, rightM, topM, bottomM;
  GetMargins(Margins, leftM, rightM, topM, bottomM);

  layout.DesiredSize = TSize(0,0);
  layout.GadgetBounds = new TRect[NumGadgets];

  // First pass tally the width of all gadgets that don't have "WideAsPossible"
  // set if any have it seet so that we can divide up the extra width
  //
  // NOTE: we must also take into account any adjustments to the gadget spacing
  //
  int  wideAsPossibleWidth;
  if (WideAsPossible) {
    int  width = 0;
    for (TGadget* gadget = Gadgets; gadget; gadget = gadget->Next) {
      if (!gadget->WideAsPossible) {
        TSize desiredSize(0, 0);

        gadget->GetDesiredSize(desiredSize);
        width += desiredSize.cx;
      }

      if (gadget->Next) {
        TPoint spacing(0, 0);

        PositionGadget(gadget, gadget->Next, spacing);
        width += spacing.x;
      }
    }
    wideAsPossibleWidth = std::max(( (innerRect.Width() - width) / (int)WideAsPossible ), 0);
  }
  else
    wideAsPossibleWidth = 0;

  // Now tile all the gadgets
  //
  int x = leftM;
  int y = topM;
  int h = 0;
  int i = 0;


  // Pass 1: calculate the gadget sizes and row height
  //
  TGadget* gadget;
  for (gadget = Gadgets; gadget; gadget = gadget->Next, i++) {
    TSize  desiredSize(0, 0);
    gadget->GetDesiredSize(desiredSize);

    // Stash away the desiredSize for the next pass
    //
    layout.GadgetBounds[i].left = desiredSize.cx;
    layout.GadgetBounds[i].top = desiredSize.cy;

    h = std::max(h, (int)desiredSize.cy);
  }

  // Pass 2: place the gadget in the row, centered vertically
  //
  i = 0;
  for (gadget = Gadgets; gadget; gadget = gadget->Next, i++) {
    TRect  bounds = gadget->GetBounds();

    // Recover desired size from where we stashed it
    //
    TSize desiredSize(layout.GadgetBounds[i].left, layout.GadgetBounds[i].top);

    bounds.left = x;
    bounds.top = y + (h - desiredSize.cy) / 2;   // Center vertically
    if (gadget->WideAsPossible)
      bounds.right = bounds.left + std::max(wideAsPossibleWidth, (int)desiredSize.cx);
    else
      bounds.right = bounds.left + desiredSize.cx;
    bounds.bottom = bounds.top + desiredSize.cy;

    layout.GadgetBounds[i] = bounds;
    x += bounds.Width();

    if (gadget->Next) {
      TPoint  origin(x, 0);

      PositionGadget(gadget, gadget->Next, origin);
      x = origin.x;
    }

    // Update gadget window's cumulative desired size
    //
    layout.DesiredSize.cx = std::max(int(bounds.right+rightM), (int)layout.DesiredSize.cx);
    layout.DesiredSize.cy = std::max(int(bounds.bottom+bottomM), (int)layout.DesiredSize.cy);
  }
}

//
/// Helper for LayoutGadgets() to calculate vertical tiling
//
void
TGadgetWindow::LayoutVertically(TLayoutInfo& layout)
{
  TRect  innerRect;
  GetInnerRect(innerRect);

  int leftM, rightM, topM, bottomM;
  GetMargins(Margins, leftM, rightM, topM, bottomM);

  layout.DesiredSize = TSize(0,0);
  layout.GadgetBounds = new TRect[NumGadgets];

  // Now tile all the gadgets
  //
  int y = topM;
  int x = leftM;
  int w = 0;
  int i = 0;

  // Pass 1: calculate gadget sizes and column width
  //
  TGadget* gadget;
  for (gadget = Gadgets; gadget; gadget = gadget->Next, i++) {
    TSize  desiredSize(0, 0);
    gadget->GetDesiredSize(desiredSize);

    // Stash away the DesiredSize for the next pass
    //
    layout.GadgetBounds[i].left = desiredSize.cx;
    layout.GadgetBounds[i].top = desiredSize.cy;

    w = std::max(w, (int)desiredSize.cx);
  }

  // Pass 2: place the gadget in the column, centered horizontally
  //
  i = 0;
  for (gadget = Gadgets; gadget; gadget = gadget->Next, i++) {
    TRect  bounds = gadget->GetBounds();

    // Recover desired size from where we stashed it
    //
    TSize desiredSize(layout.GadgetBounds[i].left, layout.GadgetBounds[i].top);

    bounds.top = y;
    bounds.bottom = bounds.top + desiredSize.cy;
    bounds.left = x + (w - desiredSize.cx) / 2;   // Center horizontally
    bounds.right = bounds.left + desiredSize.cx;

    layout.GadgetBounds[i] = bounds;

    y += bounds.Height();

    if (gadget->Next) {
      TPoint  origin(0, y);

      PositionGadget(gadget, gadget->Next, origin);
      y = origin.y;
    }

    // Update gadget window's desired size
    //
    layout.DesiredSize.cx = (int)std::max(int(bounds.right+rightM), (int)layout.DesiredSize.cx);
    layout.DesiredSize.cy = (int)std::max(int(bounds.bottom+bottomM), (int)layout.DesiredSize.cy);
  }
}

//
/// Assign top & bottoms of all controls on this row (i.e. from rowStart to
/// lastBreak). If !lastBreak, go to the end of the list.
//
void TGadgetWindow::FinishRow(int istart, TGadget* rowStart, TGadget* lastBreak,
                              int rowTop, TLayoutInfo& layout, int& rowHeight)
{
   int j;
   TGadget* g;
   rowHeight = 0;

   for (j = istart, g = rowStart; g; g = g->Next, j++) {
     rowHeight = std::max(rowHeight, (int)layout.GadgetBounds[j].bottom);
     if (g == lastBreak)
       break;
   }
   for (j = istart, g = rowStart; g; g = g->Next, j++) {
     // bounds.bottom has DesiredSize.cy
     layout.GadgetBounds[j].top = rowTop + (rowHeight - layout.GadgetBounds[j].bottom) / 2;
     layout.GadgetBounds[j].bottom = layout.GadgetBounds[j].top + layout.GadgetBounds[j].bottom;
//JJH - max can compare only equal types
#if defined(__GNUC__)
     layout.DesiredSize.cy = max((LONG)layout.GadgetBounds[j].bottom, layout.DesiredSize.cy);
#else
     layout.DesiredSize.cy = max(layout.GadgetBounds[j].bottom, layout.DesiredSize.cy);
#endif
     if (g == lastBreak)
       break;
   }
}

//
/// Helper for LayoutGadgets() to calculate rectangular 2D tiling
//
void
TGadgetWindow::LayoutRectangularly(TLayoutInfo& layout)
{
  TRect  innerRect;
  GetInnerRect(innerRect);

  layout.DesiredSize = TSize(0,0);
  layout.GadgetBounds = new TRect[NumGadgets];
// !CQ  memset((void*)layout.GadgetBounds, 0, sizeof(TRect) * NumGadgets);// Debugging code

  int       leftM, rightM, topM, bottomM;
  GetMargins(Margins, leftM, rightM, topM, bottomM);

  // Now tile all the gadgets. Assume no wide-as-possibles.
  //
  int x = leftM;
  int y = topM;
  int right = RowWidth - rightM;  // Base right margin limit on RowWidth

  // If any controls are wider than right margin, push out the right margin.
  //
  TGadget* gadget;
  for (gadget = Gadgets; gadget; gadget = gadget->Next) {
    TSize  desiredSize;
    gadget->GetDesiredSize(desiredSize);

    // SIR June 20th 2007 max instead of ::max
    right = max(right, (int)(x + desiredSize.cx + rightM));
  }

  // Scan gadgets, positioning & placing all of the EndOfRow flags
  //
  TGadget* rowStart;        // Tracks the first gadget in the row
  TGadget* lastBreak;       // Tracks the last gadget in the row
  bool     contRow = false; // Working on a group continuation row
  bool     contBreak = false; // Finished group on continuation row
  int i = 0;
  int istart = 0;
  int iend = 0;             // Tracks the last visible gadget in the row
  int ibreak = 0;
  int rowHeight;

  rowStart = Gadgets;
  lastBreak = Gadgets;
  for (gadget =  Gadgets; gadget; gadget = gadget->Next, i++) {
    gadget->SetEndOfRow(false);

    // !CQ ignore wide-as-possible gadget requests
    //
    TSize  desiredSize;
    gadget->GetDesiredSize(desiredSize);

    TRect  bounds = gadget->GetBounds();

    // Do the horizontal layout of this control
    //
    bounds.left = x;
    bounds.right = bounds.left + desiredSize.cx;

    // Store gadget's height in bottom, so we can calculate the row height
    // later
    //
    bounds.top = 0;
    bounds.bottom = desiredSize.cy;

    // If too big or a new group on a group continue row, (& is not the first
    // & is visible) then back up to iend & reset to lastBreak+1
    //
    if ((bounds.right > right || contBreak) &&
        gadget != rowStart && gadget->IsVisible()) {

      lastBreak->SetEndOfRow(true);

      // Update gadget window's desired size
      //
      layout.DesiredSize.cx =
        std::max((layout.GadgetBounds[iend].right+rightM), layout.DesiredSize.cx);

      // Do the vertical layout of this row
      //
      FinishRow(istart, rowStart, lastBreak, y, layout, rowHeight);

      contRow = lastBreak->IsVisible();  // Next row is a group continuation
      x = leftM;
      y += rowHeight;
      if (!contRow)
        y += RowMargin;

      gadget = lastBreak;       // will get bumped to Next by for incr
      i = ibreak;               // so will this

      rowStart = lastBreak = gadget->Next;  // Begin next row
      istart = i+1;
      contBreak = false;
      continue;
    }

    layout.GadgetBounds[i] = bounds;

    // Advance the break and end cursors.
    //
    if (gadget->IsVisible()) {
      if (lastBreak->IsVisible()) {  // advance both if in the first group
        iend = i;
        lastBreak = gadget;
        ibreak = i;
      }
      else if (!gadget->Next || !gadget->Next->IsVisible()) {
        // advance end if next is a break.
        iend = i;
      }
    }
    else { // advance last break if this gadget is a break
      lastBreak = gadget;
      ibreak = i;
      if (contRow)          // This invisible gadget signifies end of group
        contBreak = true;
    }

    x += bounds.Width();

    if (gadget->Next) {
      TPoint  origin(x, 0);

      PositionGadget(gadget, gadget->Next, origin);
      x = origin.x;
    }
  }

  // Update gadget window's desired size
  //
  layout.DesiredSize.cx =
    (int)std::max(int(layout.GadgetBounds[iend].right+rightM), (int)layout.DesiredSize.cx);

  // Do the vertical layout of the last row & add in bottom margin
  //
  FinishRow(istart, rowStart, 0, y, layout, rowHeight);
  layout.DesiredSize.cy += bottomM;
}

//
/// Calculate the layout of the Gadgets in a given direction
//
void
TGadgetWindow::LayoutGadgets(TTileDirection dir, TLayoutInfo& layout)
{
  switch (dir) {
    case Horizontal:
      LayoutHorizontally(layout);
      break;
    case Vertical:
      LayoutVertically(layout);
      break;
    default:
    //case Rectangular:
      CHECK(dir == Rectangular);
      LayoutRectangularly(layout);
      break;
  }
}

//
/// Used to notify the gadget window that a gadget has changed its size,
/// GadgetChangedSize calls LayoutSession() to re-layout all gadgets.
//
void
TGadgetWindow::GadgetChangedSize(TGadget&)
{
  LayoutSession();
}

//
/// Inserts a gadget before or after a sibling gadget (TPlacement). If sibling is 0,
/// then the new gadget is inserted at either the beginning or the end of the gadget
/// list. If this window has already been created, LayoutSession() needs to be called
/// after inserting gadgets.
//
void
TGadgetWindow::Insert(TGadget& gadget, TPlacement placement, TGadget* sibling)
{
  TGadgetList::Insert(gadget, placement, sibling);

  // Compute the maxWidth and maxHeight of the gadgetwindow. Needed early when
  // not created if gadget window is being positioned within another window.
  //
  // !CQ is this really needed? Seems very time consuming after every insert
  // !CQ wait until all done inserting. Maybe only do if gadgetwindow is created
  // !CQ have docking slip ask (via GetDesiredSize) when it needs initial dimensions
  //
  UseDesiredSize();
}

//
/// Inserts a list of Gadgets.
///
/// The caller needs to also call LayoutSession() after inserting gadgets if
/// this window has already been created.
//
void
TGadgetWindow::InsertFrom(TGadgetList& _list, TPlacement placement, TGadget* sibling)
{
  TGadgetList::InsertFrom(_list, placement, sibling);

  // Compute the maxWidth and maxHeight of the gadgetwindow. Needed early when
  // not created if gadget window is being positioned within another window.
  //
  // !CQ is this really needed? Seems very time consuming after every insert
  // !CQ wait until all done inserting. Maybe only do if gadgetwindow is created
  // !CQ have docking slip ask (via GetDesiredSize) when it needs initial dimensions
  //
  UseDesiredSize();
}

//
/// A gadget has been inserted into this gadget window
//
void
TGadgetWindow::Inserted(TGadget& gadget)
{
  // Let gadget know that it is now in this window
  //
  gadget.Window = this;
  gadget.Inserted();

  // If the gadgetwindow was already created, inform gadget
  // so it may perform initialization that requires an HWND
  //
  if (GetHandle())
    gadget.Created();

  if (gadget.WideAsPossible)
    WideAsPossible++;
}

//
/// Removes a gadget from the gadget window. The gadget is returned but not
/// destroyed. Remove returns 0 if the gadget is not in the window.
///
/// If this window has already been created, the calling application must call
/// LayoutSession after any gadgets have been removed.
//
TGadget*
TGadgetWindow::Remove(TGadget& gadget)
{
  if (gadget.Window != this)
    return 0;

  // Perform actual removal from list
  //
  return TGadgetList::Remove(gadget);
}

//
/// This indicates that a gadget has been removed from this gadget window.
//
void
TGadgetWindow::Removed(TGadget& gadget)
{
  // Re-adjust gadget now that it doesn't live in a window
  //
  if (gadget.WideAsPossible)
    WideAsPossible--;

  // Clear last know gadget as mouse location
  //
  if (&gadget == AtMouse)
    AtMouse = 0;

  // Release caption if it has/had it
  //
  GadgetReleaseCapture(gadget);
  
  // Notify gadget and reset/adjust variables to reflect new state of gadget
  //
  gadget.Removed();
  gadget.Window = 0;
  gadget.Next = 0;
  gadget.GetBounds() -= gadget.GetBounds().TopLeft();
}

//
/// Sets a new Shared CelArray for this gadget window. Allows a predefined array of
/// images to be shared by the gadgets. This GadgetWindow assumes ownership of the
/// passed CelArray pointer.
//
void
TGadgetWindow::SetCelArray(TCelArray* sharedCels)
{
  delete SharedCels;
  SharedCels = sharedCels;
}

//
/// Gets the Shared CelArray for this gadget window. Makes an empty one "on the fly"
/// if needed.
//
TCelArray&
TGadgetWindow::GetCelArray(int minX, int minY)
{
  if (!SharedCels) {
    // !CQ default??? BOGUS values here for now on purpose
    if (!minX)
      minX = 10;
    if (!minY)
      minY = 10;
    SharedCels = new TCelArray(TSize(minX,minY), ILC_MASK, 10, 5);
  }

  return *SharedCels;
}

//
/// Overrides TWindow::SetupWindow to create tooltips for the gadget window.
//
void
TGadgetWindow::SetupWindow()
{
  TWindow::SetupWindow();

  // if 'WantTimer' is enabled, start a timer
  //
  if (WantTimer)
    EnableTimer();

  if (DirtyLayout)    // !CQ latest OWL 5 was here...
    LayoutSession();

  // Now that this window is created, see if any of the gadgets have changed
  // sizes (like control gadgets). If so, remember the size & relayout the
  // gadgets. Also adjust the size of this gadget window to match what we want.
  // !CQ should do or is done earlier? Like EvWindowPosChanging??
  //
  TSize  size;
  GetDesiredSize(size);

//  if (DirtyLayout)  // !CQ was here in old OWL 5, & Conrad's is here...
//    LayoutSession();

  if (ShrinkWrapWidth  && Attr.W != size.cx ||
      ShrinkWrapHeight && Attr.H != size.cy)
  {
    if (ShrinkWrapWidth)
      Attr.W = size.cx;
    if (ShrinkWrapHeight)
      Attr.H = size.cy;

    LayoutSession();
    SetWindowPos(0, 0, 0, size.X(), size.Y(),
                 SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
  }

  // Create toolips after all other windows have been created.
  ///JGD This fixes problem with tooltips and dialog initially showing
  ///JGD up behind the main window (BTS 43768)
  //
  PostMessage( WM_OWLCREATETTIP );
}

//
// Override TWindow's virtual to cleanup potential Timer
//
void
TGadgetWindow::CleanupWindow()
{
  // Cleanup pending timer
  //
  if (TimerID && KillTimer(TimerID)) {
    TimerID = 0;
  }

  if (AtMouse)
  {
    AtMouse->MouseLeave(0, TPoint(-1, -1));

    AtMouse = 0;  //the window is destroyed => emulating 'mouse move at nowhere'
  }

  // Chain to base class' version
  //
  TWindow::CleanupWindow();
}

//
/// Relays 'interesting' messages to the tooltip window.
//
bool
TGadgetWindow::PreProcessMsg(MSG& msg)
{
  // Check if this message might be worth relaying to the tooltip window
  //
  if (Tooltip && Tooltip->IsWindow()) {
    if (msg.hwnd == *this || IsChild(msg.hwnd)) {
      if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
        Tooltip->RelayEvent(msg);
      }
    }
  }

  // Always let event go through.
  //
  return TWindow::PreProcessMsg(msg);
}

//
// Create Tooltips for GadgetWindow
//
void
TGadgetWindow::EvCreateTooltips()
{
  // If 'WantTooltip' is enabled, created the control
  //
  if (WantTooltip)
    EnableTooltip(true);

  // Inform each gadget that the gadgetwindow is now created. This allows
  // gadgets to perform initialization [eg. registering with the tooltip] which
  // requires the 'HWND' to be valid
  //
  for (TGadget* g = Gadgets; g; g = g->NextGadget())
    g->Created();

}

//
/// Intercepts window size changes to make sure that this gadget window follows its
/// own sizing rules. Also gives it a chance to layout wide-as-possible gadgets.
//
void
TGadgetWindow::EvWindowPosChanging(WINDOWPOS & windowPos)
{
  TWindow::EvWindowPosChanging(windowPos);

  // Only process if it is a size change
  //
  if (!(windowPos.flags&SWP_NOSIZE))
  {
    // Knowing how big we might be, tile the gadgets to let them get positioned
    // the way they want.
    //
    Attr.W = windowPos.cx;
    Attr.H = windowPos.cy;

    // Only tile if either we are dirty or have some wide-as-possible gadgets
    // to adjust. Dont call LayoutSession() in order to avoid a loop. Derived
    // versions might trigger a resize in there.
    //
    if (DirtyLayout || WideAsPossible)
      InvalidateRect(TileGadgets());

    // Find out how big we really want to be if any shrink wrap, and enforce it
    //
    UseDesiredSize();
    if (ShrinkWrapWidth)
      windowPos.cx = Attr.W;
    if (ShrinkWrapHeight)
      windowPos.cy = Attr.H;

    // Redraw all if we're using themes (could have background gradients).
    //
    if (IsBackgroundThemed())
      Invalidate(true); 
  }
}

//
/// Called by Paint to repaint all of the gadgets, PaintGadgets iterates through the
/// list of gadgets, determines the gadget's area, and repaints each gadget.
/// 
/// You can override this function to implement a specific look (for example,
/// separator line, raised, and so on).
//
void
TGadgetWindow::PaintGadgets(TDC& dc, bool, TRect& rect)
{
  TPoint viewport = dc.GetViewportOrg();
  for (TGadget* gadget = Gadgets; gadget; gadget = gadget->Next) {
    if (gadget->GetBounds().Touches(rect)) {
      dc.SetViewportOrg((TPoint&)gadget->GetBounds());

      // If the gadget is a sloppy painter & needs clipping support, set the
      // clip rect before painting, otherwise just paint. Most gadgets don't
      // need this help
      //
      if (gadget->Clip) {
        TRegion savedClipRgn;
        dc.GetClipRgn(savedClipRgn);
        dc.IntersectClipRect(gadget->GetBounds());

        gadget->Paint(dc);

        dc.SelectClipRgn(savedClipRgn);
      }
      else
        gadget->Paint(dc);
    }
  }
  dc.SetViewportOrg(viewport);
}

//
/// Puts the font into the device context and calls PaintGadgets.
///
/// Respond to virtual TWindow Paint call. Call our own virtual PaintGadgets
/// to actually perform the painting of the gadgets
//
void
TGadgetWindow::Paint(TDC& dc, bool erase, TRect& rect)
{
  if (IsBackgroundThemed())
  {
    TThemePart part(GetHandle(), (LPCWSTR) L"REBAR", 0, 0);
    part.DrawBackground(dc, GetClientRect(), rect);
    erase = false;
  }

  dc.SelectObject(*Font);
  PaintGadgets(dc, erase, rect);

#if defined(__TRACE) || defined(__WARN)
  // Highlight the tools of this gadgetwindow for debugging purposes.
  // Especially useful for docking windows which undergo internal state
  // changes
  //
  TProfile iniFile(_T("Diagnostics"), _T(OWL_INI));

  bool useDiagColor = iniFile.GetInt(_T("DiagToolbarBorders")) != 0;

  if (useDiagColor && Tooltip && Tooltip->IsWindow()) {
    uint count = Tooltip->GetToolCount();
    if (count) {
      TPen redPen(TColor(0xff, 0, 0));
      dc.SelectObject(redPen);

      TToolInfo ti;
      while (count) {
        if (Tooltip->EnumTools(--count, ti)) {
          dc.MoveTo(ti.rect.left, ti.rect.top);
          dc.LineTo(ti.rect.right,ti.rect.top);
          dc.LineTo(ti.rect.right,ti.rect.bottom);
          dc.LineTo(ti.rect.left, ti.rect.bottom);
          dc.LineTo(ti.rect.left, ti.rect.top);
        }
      }
      dc.RestorePen();
    }
  }
#endif
}

//
/// Responds to a left button-down mouse message by forwarding the event to the
/// gadget (if it is enabled) positioned under the mouse.
//
void
TGadgetWindow::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TGadget* gadget = Capture ? Capture : GadgetFromPoint(point);

  if (gadget && (gadget->GetEnabled() || Capture)) {
    TPoint gadgetPoint = point - *(TSize*)&gadget->GetBounds().TopLeft();
    gadget->LButtonDown(modKeys, gadgetPoint);
  }

  TWindow::EvLButtonDown(modKeys, point);
}

//
/// Responds to a left button-up mouse message by forwarding the event to the gadget
/// that has the capture or to the gadget positioned under the mouse
//
void
TGadgetWindow::EvLButtonUp(uint modKeys, const TPoint& point)
{
  TGadget* gadget = Capture ? Capture : GadgetFromPoint(point);

  if (gadget && (gadget->GetEnabled() || Capture)){
    TPoint p(point.x - gadget->GetBounds().left, point.y - gadget->GetBounds().top);
    gadget->LButtonUp(modKeys, p);
  }
  TWindow::EvLButtonUp(modKeys, point);
}

//
/// Passes double clicks through as if they were just a second click; finishes the
/// first click, and begins the second: Dn + Dbl + Up -> Dn + Up+Dn + Up.
//
void
TGadgetWindow::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  EvLButtonUp(modKeys, point);
  EvLButtonDown(modKeys, point);
}

//
/// Pass RButton messages to the gadget at the mouse location or the one
/// with Capture
///
/// \note The default right-mouse down handler of TGadget does
///       *NOT* set capture
//
void
TGadgetWindow::EvRButtonDown(uint modKeys, const TPoint& point)
{
  TGadget* gadget = Capture ? Capture : GadgetFromPoint(point);

  if (gadget && (gadget->GetEnabled() || Capture)) {
    TPoint gadgetPoint = point - *(TSize*)&gadget->GetBounds().TopLeft();
    gadget->RButtonDown(modKeys, gadgetPoint);
  }

  TWindow::EvRButtonDown(modKeys, point);
}

//
/// Pass RButton messages to the gadget at the mouse location or the one with
/// capture.
///
/// \note The default right-mouse down handler of TGadget does
///       *NOT* set capture
//
void
TGadgetWindow::EvRButtonUp(uint modKeys, const TPoint& point)
{
  TGadget* gadget = Capture ? Capture : GadgetFromPoint(point);

  if (gadget && (gadget->GetEnabled() || Capture)){
    TPoint p(point.x - gadget->GetBounds().left, point.y - gadget->GetBounds().top);
    gadget->RButtonUp(modKeys, p);
  }
  TWindow::EvRButtonUp(modKeys, point);
}

//
/// Forward mouse moves to the gadget that has captured the mouse, if any. Otherwise
/// checks for mouse entering and leaving gadgets. This could be enhanced by
/// delaying mouse enter messages until the mouse has been in the same area for a
/// while, or by looking ahead in the queue for mouse messages.
//
void
TGadgetWindow::EvMouseMove(uint modKeys, const TPoint& point)
{
  struct Local 
  { 
    typedef void (TGadget::*MouseFun)(uint, const TPoint&);
    static void Notify(TGadget* g, MouseFun f, uint modKeys, TPoint p)
    {
      if (!g) return;
      p -= g->GetBounds().TopLeft(); // Convert to local coo.
      (g->*f)(modKeys, p);
        }
  };

  if (Capture)
    Local::Notify(Capture, &TGadget::MouseMove, modKeys, point);
  else
  {
    TGadget* gadget = GadgetFromPoint(point);
    if (gadget != AtMouse)
    {
      Local::Notify(AtMouse, &TGadget::MouseLeave, modKeys, point);
      AtMouse = gadget;
      Local::Notify(AtMouse, &TGadget::MouseEnter, modKeys, point);
    }
  }
  TWindow::EvMouseMove(modKeys, point);
}

//
//
//
void
TGadgetWindow::EnableTooltip(bool enable)
{
  if (!Tooltip) {

    // Find a parent for the tooltip: It's attractive to make the
    // gadgetwindow the owner of the tooltip, a popup window. However, this
    // will typically fail since the gadget window is invariably a child
    // window. Windows seems to accomodate this situation by simply making
    // the tooltip's owner the gadgetwindow's owner. This works fine until
    // the owner of the gadgetwindow is destroyed before the gadgetwindow
    // is destroyed, such as in the case of a gadgetwindow initally created
    // as a floating docking toolbar; When it's docked, the floating slip,
    // it's original owner, is destroyed and the gadget window is reparented
    // to an edge slip. In this scenario, the tooltip is silently destroyed
    // along with the floating slip [it's real owner!] and the docked
    // gadgetwindow no longer provide tool tips!
    //

    // To circumvent this scenario, we'll look for a window which is fairly
    // stable/rooted as owner of the tooltip. Ideally, we'll get the
    // application's main window.
    //
    TWindow* tipParent = this;
    while (tipParent->GetParentO()) {
      tipParent = tipParent->GetParentO();
      if (tipParent->IsFlagSet(wfMainWindow))
        break;
    }

    // Create and initialize tooltip
    //
    SetTooltip(new TTooltip(tipParent));
  }
  else {
    if (Tooltip->GetHandle())
      Tooltip->Activate(enable);
  }
}

//
/// Sets the Tooltip data member equal to tooltip. If the tooltip is invalid (if
/// Tooltip->GetHandle() fails), a new tooltip is created by the TTooltip::Create()
/// function.
//
void
TGadgetWindow::SetTooltip(TTooltip* tooltip)
{
  // Cleanup; via Condemned list if tooltip was created
  //
  if (Tooltip) {
    if (Tooltip->GetHandle())
      Tooltip->SendMessage(WM_CLOSE);
    else
      delete Tooltip;
  }

  // Store new tooltip and create if necessary
  //
  Tooltip = tooltip;
  if (Tooltip) {
    if(GetHandle() && !Tooltip->GetHandle()) {
      // Make sure tooltip is disabled so it does not take input focus
      Tooltip->ModifyStyle(0,WS_DISABLED);
      Tooltip->Create();
    }
  }
}

//
/// When the gadget window receives a WM_COMMAND message, it is likely from a gadget
/// or control within a TControlGadget. This reroutes it to the command target.
//
TResult
TGadgetWindow::EvCommand(uint id, HWND hWndCtl, uint notifyCode) 
{
  TRACEX(OwlCmd, 1, "TGadgetWindow::EvCommand - id(" << id << "), ctl(" <<\
                     hex << uint(hWndCtl) << "), code(" << notifyCode  << ")");

  // First allow any derived class that wants to handle the command
  // NOTE: This search only caters for menu-style WM_COMMANDs (not those
  //       sent by controls)
  //
  TEventInfo  eventInfo(0, id);
  if (Find(eventInfo)) {
    Dispatch(eventInfo, id);
    return 0;
  }


#if 0
  // Prior versions of TGadgetWindow relied on TWindow's EvCommand for
  // dispatching WM_COMMAND events. This required that one derives from
  // a decoration class (eg. TControlbar, TToolbox) to handle control
  // notifications. The current version uses a more generalized logic
  // involving the CommandTarget and a frame ancestor class. This allows
  // a client window to handle notifications of a control in a toolbar
  // without using a TControlbar-derived class.
  // However, if you need to previous behaviour, simply invoke TWindow's
  // EvCommand from this handler.

  return TWindow::EvCommand(id, hWndCtl, notifyCode);
#endif

  TWindow* target;
  TFrameWindow* frame;

  // Find the frame who is our latest ancestor and make it our command target
  //
  for (target = GetParentO(); target; target = target->GetParentO()) {
    frame = TYPESAFE_DOWNCAST(target, TFrameWindow);
    if (frame || !target->GetParentO())
      break;
  }

  // Make sure the frame doesn't think we are its command target, or a BAD
  // loop will happen
  //
  if (target && (!frame || frame->GetCommandTarget() != GetHandle())) {
    CHECK(target->IsWindow());
    return target->EvCommand(id, hWndCtl, notifyCode);
  }

  // If all command routing fails, go back to basic dispatching of TWindow
  //
  return TWindow::EvCommand(id, hWndCtl, notifyCode);
}

//
/// When the gadget window receives a WM_COMMAND_ENABLE message, it is likely from a
/// gadget or control within a TControlGadget. This reroutes it to the command
/// target.
//
void
TGadgetWindow::EvCommandEnable(TCommandEnabler& ce)
{
  // If someone derived from TGadgetWindow and handles the command there,
  // give these handlers the first crack.
  //
  TEventInfo eventInfo(WM_COMMAND_ENABLE, ce.GetId());
  if (Find(eventInfo)) {
    Dispatch(eventInfo, 0, TParam2(&ce));
    return;
  }

  TWindow* target = GetParentO();

  // Forward to command target if the enabler was really destined for us, and
  // not a routing from the frame.
  //
  if (target && ce.IsReceiver(*this)) {
    CHECK(target->IsWindow());
    ce.SetReceiver(*target);
    target->EvCommandEnable(ce);
    if( ce.GetHandled() )
      return;
  }

  // Default to TWindow's implementation if the above routing fails
  //
  TWindow::EvCommandEnable(ce);
}

//----------------------------------------------------------------------------
//
//
/// Constructs a control out of a gadget.
//
TGadgetControl::TGadgetControl(TWindow*  parent,
                               TGadget*  soleGadget,
                               TFont*    font,
                               TModule*  module)
:
  TGadgetWindow(parent, Horizontal, font, module)
{
  // Let Attr rect override
  //
  SetShrinkWrap(false, false);

  // Let inner gadget paint everything
  //
  TMargins mar(TMargins::Pixels, 0, 0, 0, 0);
  SetMargins(mar);

  if (soleGadget)
    Insert(*soleGadget);
}

} // OWL namespace
/* ========================================================================== */


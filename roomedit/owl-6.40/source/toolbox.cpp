//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TToolBox, a 2-d arrangement of TButtonGadgets.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/toolbox.h>
#include <owl/buttonga.h>
#include <owl/uimetric.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a TToolBox object with the specified number of columns and rows and
/// tiling direction. Overlaps the borders of the toolbox with those of the gadget
/// and sets ShrinkWrapWidth to true.
//
TToolBox::TToolBox(TWindow*        parent,
                   int             numColumns,
                   int             numRows,
                   TTileDirection  direction,
                   TModule*        module)
:
  TGadgetWindow(parent, direction, new TGadgetWindowFont, module)
{
  NumRows = numRows;
  NumColumns = numColumns;

  // Make the gadget borders (if any) overlap the tool box's borders
  //
  Margins.Units = TMargins::BorderUnits;
  Margins.Left = Margins.Right = 0;
  Margins.Top = Margins.Bottom = 0;

  ShrinkWrapWidth = true;
}

//
/// Overrides TGadget's Insert function and tells the button not to notch its corners.
///
/// Only TButtonGadgets or derived gadgets are supported.
//
void
TToolBox::Insert(TGadget& g, TPlacement placement, TGadget* sibling)
{
  TGadgetWindow::Insert(g, placement, sibling);

  // Notch the corners if it's a buttonGadget
  //
  TButtonGadget* bg = TYPESAFE_DOWNCAST(&g,TButtonGadget);
  if (bg)
    bg->SetNotchCorners(false);
}

//
/// Sets the direction of the tiling--either horizontal or vertical.
///
/// Swap the rows & columns count, and let our base class do the rest
//
void
TToolBox::SetDirection(TTileDirection direction)
{
  TTileDirection dir = Direction;
  if (dir != direction) {
    int t = NumRows;
    NumRows = NumColumns;
    NumColumns = t;
  }

  TGadgetWindow::SetDirection(direction);
}

//
// Compute the numer of rows & columns, filling in rows OR columns if left
// unspecified using AS_MANY_AS_NEEDED (but not both).
//
void
TToolBox::ComputeNumRowsColumns(int& numRows, int& numColumns)
{
  CHECK(NumRows != AS_MANY_AS_NEEDED || NumColumns != AS_MANY_AS_NEEDED);
  numRows = NumRows == AS_MANY_AS_NEEDED ?
              (NumGadgets + NumColumns - 1) / NumColumns :
              NumRows;
  numColumns = NumColumns == AS_MANY_AS_NEEDED ?
                 (NumGadgets + NumRows - 1) / NumRows :
                 NumColumns;
}

//
// Compute the cell size which is determined by the widest and the highest
// gadget
//
void
TToolBox::ComputeCellSize(TSize& cellSize)
{
  cellSize.cx = cellSize.cy = 0;

  for (TGadget* g = Gadgets; g; g = g->NextGadget()) {
    TSize  desiredSize(0, 0);

    g->GetDesiredSize(desiredSize);

    if (desiredSize.cx > cellSize.cx)
      cellSize.cx = desiredSize.cx;

    if (desiredSize.cy > cellSize.cy)
      cellSize.cy = desiredSize.cy;
  }
}

//
/// Overrides TGadget's GetDesiredSize function and computes the size of the cell by
/// calling GetMargins to get the margins.
//
void
TToolBox::GetDesiredSize(TSize& size)
{
  // Get border sizes
  //
  int    cxBorder = 0;
  int    cyBorder = 0;

  int    left, right, top, bottom;
  GetMargins(Margins, left, right, top, bottom);
  size.cx = left + right;
  size.cy = top + bottom;

  // Add in this window's border size if used
  //
  if (Attr.Style & WS_BORDER) {
    size.cx += 2 * TUIMetric::CxBorder;
    size.cy += 2 * TUIMetric::CyBorder;
  }

  TSize  cellSize;
  ComputeCellSize(cellSize);

  int    numRows, numColumns;
  ComputeNumRowsColumns(numRows, numColumns);

  size.cx += numColumns * cellSize.cx;
  size.cy += numRows * cellSize.cy;

  // Compensate for the gadgets overlapping if UI style does that
  //
  size.cx -= (numColumns - 1) * cxBorder;
  size.cy -= (numRows - 1) * cyBorder;
}

//
/// Tiles the gadgets in the direction requested (horizontal or vertical). Derived
/// classes can adjust the spacing between gadgets.
///
/// Horizontal direction results in a row-major layout, 
/// and vertical direction results in column-major layout
//
TRect
TToolBox::TileGadgets()
{
  TSize     cellSize;
  ComputeCellSize(cellSize);

  int       numRows, numColumns;
  ComputeNumRowsColumns(numRows, numColumns);

  TRect     innerRect;
  GetInnerRect(innerRect);

  TRect     invalidRect;
  invalidRect.SetEmpty();

  if (Direction == Horizontal) {
    // Row Major
    //
    int y = innerRect.top;
    TGadget* g = Gadgets;

    for (int r = 0; r < numRows; r++) {
      int x = innerRect.left;

      for (int c = 0; c < numColumns && g; c++) {
        TRect bounds(TPoint(x, y), cellSize);
        TRect originalBounds(g->GetBounds());

        if (bounds != g->GetBounds()) {
          g->SetBounds(bounds);

          if (invalidRect.IsNull())
            invalidRect = bounds;
          else
            invalidRect |= bounds;

          if (originalBounds.TopLeft() != TPoint(0, 0))
            invalidRect |= originalBounds;
        }

        x += cellSize.cx;
        g = g->NextGadget();
      }

      y += cellSize.cy;
    }
  }
  else {
    // Column Major
    //
    int x = innerRect.left;
    TGadget* g = Gadgets;

    for (int c = 0; c < numColumns; c++) {
      int y = innerRect.top;

      for (int r = 0; r < numRows && g; r++) {
        TRect bounds(TPoint(x, y), cellSize);
        TRect originalBounds(g->GetBounds());

        if (bounds != originalBounds) {
          g->SetBounds(bounds);

          if (invalidRect.IsNull())
            invalidRect = bounds;
          else
            invalidRect |= bounds;

          if (originalBounds.TopLeft() != TPoint(0, 0))
            invalidRect |= originalBounds;
        }

        y += cellSize.cy;
        g = g->NextGadget();
      }

      x += cellSize.cx;
    }
  }
  return invalidRect;
}

//
/// Called when a change occurs in the size of the margins of the tool box or size
/// of the gadgets, LayoutSession gets the desired size and moves the window to
/// adjust to the desired change in size.
///
/// Assumes it is used as a client in a frame.
//
void
TToolBox::LayoutSession()
{
  TGadgetWindow::LayoutSession();

  TSize sz;
  GetDesiredSize(sz);

  SetWindowPos(0, 0,0, sz.cx, sz.cy, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
}

} // OWL namespace
/* ========================================================================== */



//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
// MoveWin.cpp -- An implementation of a floating border when
//                moving/resizing a window
//----------------------------------------------------------------------------
#ifndef MOVEWIN_H
#include "movewin.h"
#endif

#include <mem.h>

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//
//
//  Purpose:
//
//  Parameters:
//
//  Return value:
//
//
GrayGhostRect::GrayGhostRect(RECT& WinSize, UINT wHitTestCode, UINT wLineSize) :
                                    wHitCode(wHitTestCode),
                                    wThickness(wLineSize),
                                    hBrush(0), hBmp(0)
{
  // initialize data members
  CopyRect ((LPRECT)&GrayRect, (LPRECT)&WinSize);

  // initialize pattern information
  WORD pattern[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
  FramePattern (pattern, 16);
}

GrayGhostRect::~GrayGhostRect()
{
  DeleteObject (hBrush);
  DeleteObject (hBmp);
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::FramePattern
//
//  Purpose:
//    create brush and bitmap to be used in drawing to the screen
//
//  Parameters:
//    const WORD*              pattern array
//    int                      array size
//
//  Return value:
//    HBRUSH                  created brush or null if the function fails
//
HBRUSH GrayGhostRect::FramePattern (const WORD* newPattern, int nSize)
{
  // create bitmap for the brush. It must be 8x8. We use monochrome bitmap.
  memcpy (wGrayBmp, newPattern, nSize);
  if (hBmp)
    DeleteObject (hBmp);
  hBmp = CreateBitmap (8, 8, 1, 1, wGrayBmp);

  if (hBmp)  {
    // create and store our brush
    if (hBrush)
      DeleteObject (hBrush);
    hBrush = CreatePatternBrush (hBmp);
  }

  return hBrush;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::MoveFrame
//
//  Purpose:
//    main function for simulating moving or resizing
//
//  Parameters:
//    POINT                    starting point
//    BOOL                    indicates move or resize
//
void GrayGhostRect::MoveFrame(POINT &MousePos, BOOL bOnMove)
{
    HBRUSH  hbrOld;
    HDC hDC;

  if (hBrush)  {
    hDC = ::GetDC (NULL); // get screen DC
    hbrOld = SelectObject(hDC, hBrush); // change brush
  }
  else
    return; // no brush, abort operation. Here an exception would be much better

  if (bOnMove) {
    RECT newRectFrame;

    switch (wHitCode) {
      // resizing check window size; we don't want resizing
      // to invert the window or nuke the window
      case HTTOPLEFT:
        if (MousePos.y > GrayRect.bottom - MINIMUM_SIZE)
          MousePos.y = GrayRect.bottom - MINIMUM_SIZE;
        if (MousePos.x > GrayRect.right - MINIMUM_SIZE)
          MousePos.x = GrayRect.right - MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame,
                              GrayRect.left + MousePos.x - StartMove.x,
                              GrayRect.top + MousePos.y - StartMove.y,
                              GrayRect.right, GrayRect.bottom);
      break;
      case HTLEFT:
        if (MousePos.x > GrayRect.right - MINIMUM_SIZE)
          MousePos.x = GrayRect.right - MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, MousePos.x, GrayRect.top,
                              GrayRect.right, GrayRect.bottom);
      break;
      case HTBOTTOMLEFT:
        if (MousePos.y < GrayRect.top + MINIMUM_SIZE)
          MousePos.y = GrayRect.top + MINIMUM_SIZE;
        if (MousePos.x > GrayRect.right - MINIMUM_SIZE)
          MousePos.x = GrayRect.right - MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame,
                              GrayRect.left + MousePos.x - StartMove.x,
                              GrayRect.top,  GrayRect.right,
                              GrayRect.bottom + MousePos.y - StartMove.y);
      break;
      case HTTOP:
        if (MousePos.y > GrayRect.bottom - MINIMUM_SIZE)
          MousePos.y = GrayRect.bottom - MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, GrayRect.left, MousePos.y,
                              GrayRect.right, GrayRect.bottom);
      break;
      case HTBOTTOM:
        if (MousePos.y < GrayRect.top + MINIMUM_SIZE)
          MousePos.y = GrayRect.top + MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, GrayRect.left, GrayRect.top,
                              GrayRect.right, MousePos.y);
      break;
      case HTTOPRIGHT:
        if (MousePos.y > GrayRect.bottom - MINIMUM_SIZE)
          MousePos.y = GrayRect.bottom - MINIMUM_SIZE;
        if (MousePos.x < GrayRect.left + MINIMUM_SIZE)
          MousePos.x = GrayRect.left + MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, GrayRect.left,
                              GrayRect.top + MousePos.y - StartMove.y,
                              GrayRect.right + MousePos.x - StartMove.x,
                              GrayRect.bottom);
      break;
      case HTRIGHT:
        if (MousePos.x < GrayRect.left + MINIMUM_SIZE)
          MousePos.x = GrayRect.left + MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, GrayRect.left, GrayRect.top,
                              MousePos.x, GrayRect.bottom);
      break;
      case HTBOTTOMRIGHT:
        if (MousePos.y < GrayRect.top + MINIMUM_SIZE)
          MousePos.y = GrayRect.top + MINIMUM_SIZE;
        if (MousePos.x < GrayRect.left + MINIMUM_SIZE)
          MousePos.x = GrayRect.left + MINIMUM_SIZE;
        SetRect ((LPRECT)&newRectFrame, GrayRect.left, GrayRect.top,
                                GrayRect.right - StartMove.x + MousePos.x,
                                GrayRect.bottom - StartMove.y + MousePos.y);
      break;
      // just move, no resizing involved
      default:
        // adjust offset since moving is upon top/left position
        CopyRect ((LPRECT)&newRectFrame, (LPRECT)&GrayRect);
        OffsetRect ((LPRECT)&newRectFrame, MousePos.x - StartMove.x,
                              MousePos.y - StartMove.y);
        //StartMove.x = MousePos.x;
        //StartMove.y = MousePos.y;
    }

    StartMove.x = MousePos.x;
    StartMove.y = MousePos.y;

    RECT TempFrameSubRect1, TempFrameSubRect2;
    //\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
    // get left frame sub rectangle of old position
    TempFrameSubRect1.left = GrayRect.left;
    TempFrameSubRect1.top = GrayRect.top;
    TempFrameSubRect1.right = GrayRect.left + wThickness;
    TempFrameSubRect1.bottom = GrayRect.bottom;
    // get left frame sub rectangle of new position
    TempFrameSubRect2.left = newRectFrame.left;
    TempFrameSubRect2.top = newRectFrame.top;
    TempFrameSubRect2.right = newRectFrame.left + wThickness;
    TempFrameSubRect2.bottom = newRectFrame.bottom;
    // do drawing
    UpdateRect (hDC, TempFrameSubRect1, TempFrameSubRect2);

    //\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
    // get right frame sub rectangle of old position
    TempFrameSubRect1.left = GrayRect.right - wThickness;
    TempFrameSubRect1.right = GrayRect.right;
    // get right frame sub rectangle of new position
    TempFrameSubRect2.left = newRectFrame.right - wThickness;
    TempFrameSubRect2.right = newRectFrame.right;
    // do drawing
    UpdateRect (hDC, TempFrameSubRect1, TempFrameSubRect2);

    //\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
    // get top frame sub rectangle of old position
    TempFrameSubRect1.left = GrayRect.left + wThickness;
    TempFrameSubRect1.right -= wThickness;
    TempFrameSubRect1.bottom = GrayRect.top + wThickness;
    // get top frame sub rectangle of new position
    TempFrameSubRect2.left = newRectFrame.left + wThickness;
    TempFrameSubRect2.right -= wThickness;
    TempFrameSubRect2.bottom = newRectFrame.top + wThickness;
    // do drawing
    UpdateRect (hDC, TempFrameSubRect1, TempFrameSubRect2);

    //\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
    // get bottom frame sub rectangle of old position
    TempFrameSubRect1.top = GrayRect.bottom - wThickness;
    TempFrameSubRect1.bottom = GrayRect.bottom;
    // get bottom frame sub rectangle of new position
    TempFrameSubRect2.top = newRectFrame.bottom - wThickness;
    TempFrameSubRect2.bottom = newRectFrame.bottom;
    // do drawing
    UpdateRect (hDC, TempFrameSubRect1, TempFrameSubRect2);


    //XORFrame(hDC); //old solution
    CopyRect ((LPRECT)&GrayRect, (LPRECT)&newRectFrame);
    //XORFrame(hDC); // old solution
  } // end if (bOnMove)
  else
    // this is the first or the last drawing
    XORFrame(hDC);

  SelectObject(hDC, hbrOld); // select back old brush
  ReleaseDC (NULL, hDC); // release DC
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::XORFrame
//
//  Purpose:
//    XOR the current rect with the screen
//
//  Parameters:
//    HDC                      DC to xor our rect
//
void GrayGhostRect::XORFrame (HDC hDC)
{
  PatBlt(hDC, GrayRect.left, GrayRect.top,
                        GrayRect.right - GrayRect.left,
                        wThickness, PATINVERT);
  PatBlt(hDC, GrayRect.left, GrayRect.top + wThickness, wThickness,
                        GrayRect.bottom - GrayRect.top  - wThickness,
                        PATINVERT);
  PatBlt(hDC, GrayRect.right - wThickness, GrayRect.top + wThickness,
                        wThickness,
                        GrayRect.bottom - GrayRect.top  - wThickness,
                        PATINVERT);
  PatBlt(hDC, GrayRect.left + wThickness, GrayRect.bottom - wThickness,
                        GrayRect.right - wThickness - GrayRect.left - wThickness,
                        wThickness, PATINVERT);
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::SetStartMove
//
//  Purpose:
//    set data member StartMove
//
void GrayGhostRect::SetStartMove (const POINT& mousePoint)
{
  StartMove.x = mousePoint.x;
  StartMove.y = mousePoint.y;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::UpperCorner
//
//  Purpose:
//    set data mousePoint to the upper left corner of the window rect
//
void GrayGhostRect::UpperCorner (POINT& TopLeft) const
{
  TopLeft.x = GrayRect.left;
  TopLeft.y = GrayRect.top;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::Size
//
//  Purpose:
//    return window size
//
void GrayGhostRect::Size (POINT& WidthEight) const
{
  WidthEight.x = GrayRect.right - GrayRect.left;
  WidthEight.y = GrayRect.bottom - GrayRect.top;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// GrayGhostRect::UpdateRect
//
//  Purpose:
//    this function draws the rect on the screen. If there are overlapped
//    areas only draws non overlapped parts
//
//  Parameters:
//    HDC                      Device context to draw on
//    RECT&                    old rectangle position
//    RECT&                    new rectangle position
//
//
void GrayGhostRect::UpdateRect (HDC hDC, RECT& oldRect, RECT& newRect)
{
  RECT Intersection;
  if (IntersectRect ((LPRECT)&Intersection, (LPRECT)&oldRect, (LPRECT)&newRect)) {
    if (oldRect.top >= newRect.top || oldRect.bottom >= newRect.bottom) {
      if (oldRect.left <= newRect.left || oldRect.right <= newRect.right) {
        PatBlt(hDC, newRect.left, newRect.top, oldRect.right - newRect.left,
                        oldRect.top - newRect.top, PATINVERT);
        PatBlt(hDC, newRect.left, newRect.bottom, oldRect.right - newRect.left,
                        oldRect.bottom - newRect.bottom, PATINVERT);
        PatBlt(hDC, oldRect.right, newRect.top, newRect.right - oldRect.right,
                        newRect.bottom - newRect.top, PATINVERT);
        PatBlt(hDC, oldRect.left, oldRect.top, newRect.left - oldRect.left,
                        oldRect.bottom - oldRect.top, PATINVERT);
      }
      else if (oldRect.left > newRect.left || oldRect.right > newRect.right) {
        PatBlt(hDC, oldRect.left, newRect.top, newRect.right - oldRect.left,
                        oldRect.top - newRect.top, PATINVERT);
        PatBlt(hDC, oldRect.left, newRect.bottom, newRect.right - oldRect.left,
                        oldRect.bottom - newRect.bottom, PATINVERT);
        PatBlt(hDC, newRect.right, oldRect.top, oldRect.right - newRect.right,
                        oldRect.bottom - oldRect.top, PATINVERT);
        PatBlt(hDC, newRect.left, newRect.top, oldRect.left - newRect.left,
                        newRect.bottom - newRect.top, PATINVERT);
      }
    }
    else if (oldRect.top < newRect.top || oldRect.bottom < newRect.bottom) {
      if (oldRect.left <= newRect.left || oldRect.right <= newRect.right) {
        PatBlt(hDC, newRect.left, oldRect.top, oldRect.right - newRect.left,
                        newRect.top - oldRect.top, PATINVERT);
        PatBlt(hDC, newRect.left, oldRect.bottom, oldRect.right - newRect.left,
                        newRect.bottom - oldRect.bottom, PATINVERT);
        PatBlt(hDC, oldRect.right, newRect.top, newRect.right - oldRect.right,
                        newRect.bottom - newRect.top, PATINVERT);
        PatBlt(hDC, oldRect.left, oldRect.top, newRect.left - oldRect.left,
                        oldRect.bottom - oldRect.top, PATINVERT);
      }
      else if (oldRect.left > newRect.left || oldRect.right > newRect.right) {
        PatBlt(hDC, oldRect.left, oldRect.top, newRect.right - oldRect.left,
                        newRect.top - oldRect.top, PATINVERT);
        PatBlt(hDC, oldRect.left, oldRect.bottom, newRect.right - oldRect.left,
                        newRect.bottom - oldRect.bottom, PATINVERT);
        PatBlt(hDC, newRect.right, oldRect.top, oldRect.right - newRect.right,
                        oldRect.bottom - oldRect.top, PATINVERT);
        PatBlt(hDC, newRect.left, newRect.top, oldRect.left - newRect.left,
                        newRect.bottom - newRect.top, PATINVERT);
      }
    }
  }
  else {
    PatBlt(hDC, oldRect.left, oldRect.top, oldRect.right - oldRect.left,
                        oldRect.bottom - oldRect.top, PATINVERT);
    PatBlt(hDC, newRect.left, newRect.top, newRect.right - newRect.left,
                        newRect.bottom - newRect.top, PATINVERT);
  }
}



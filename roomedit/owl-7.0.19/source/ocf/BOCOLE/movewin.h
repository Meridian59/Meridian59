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
#define MOVEWIN_H

#include <windows.h>

#define STANDARD_THICKNESS     3
// minimum size is three times standard thickness
#define MINIMUM_SIZE          21

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//
// GrayGhostRect
//
//  Purpose:
//    simulate non full drag operation even when full drag is on.
//    Draw a resizable and movable rectangle on the screen accordingly
//    to the window to be moved
//
class GrayGhostRect {

  public:

    GrayGhostRect (RECT&, UINT, UINT = STANDARD_THICKNESS);
    ~GrayGhostRect ();

    HBRUSH FramePattern (const WORD*, int); // change the frame pattern
    void MoveFrame (POINT&, BOOL);

    void SetStartMove (const POINT&);
    void UpperCorner (POINT&) const;
    void Size (POINT&) const;

  private:

    void XORFrame (HDC);
    void UpdateRect (HDC, RECT&, RECT&);

  private:

    // frame related data
    RECT GrayRect; // store rect dimension each MOUSEMOVE
    UINT wThickness; // frame line thickness
    POINT StartMove; // mouse initial point when moving (no size)

    // stuff used to paint the frame. Initialized at construction
    // time and relesed when the destructor is called
    WORD wGrayBmp[8]; // bitmap pattern
    HBITMAP hBmp; // bitmap handle
    HBRUSH  hBrush; // brush handle

    UINT wHitCode; // code describing operation type (move, size,..)
};

#endif

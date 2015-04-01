//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
///  Implements TPictureWindow
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/window.h>
#include <owl/gdiobjec.h>
#include <owl/pictwind.h>
#include <owl/pointer.h>
#include <owl/system.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructor for this class. Assumes ownership of the DIB passed in.
//
TPictureWindow::TPictureWindow(TWindow* parent, TDib* dib, TDisplayHow how,
                               LPCTSTR title, TModule* module)
:
  TWindow(parent, title, module),
  Dib(dib)
{
  SetHowToDisplay(how);
  if (GetHowToDisplay() == Stretch) {
    if (dib) {
      Attr.W = dib->Width();
      Attr.H = dib->Height();
    }
  }
}

//
/// String-aware overload
//
TPictureWindow::TPictureWindow(
  TWindow* parent, 
  TDib* dib, 
  TDisplayHow how,
  const tstring& title, 
  TModule* module
  )
  : TWindow(parent, title, module),
  Dib(dib)
{
  SetHowToDisplay(how);
  if (GetHowToDisplay() == Stretch) 
  {
    if (dib) 
    {
      Attr.W = dib->Width();
      Attr.H = dib->Height();
    }
  }
}

//
/// Destructor for this class. Deletes the owned DIB.
//
TPictureWindow::~TPictureWindow()
{
  delete Dib;
}


//
/// Allows changing of the DIB. Returns the old DIB.
//
TDib*
TPictureWindow::SetDib(TDib* newDib)
{
  TDib* retDib = Dib;
  Dib = newDib;
  return retDib;
}


//
/// Paints the DIB onto the window.
//
void
TPictureWindow::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
  TPointer<TPalette> palette(0);
  bool hasPalette = ToBool(dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE);

  TDib* dib = GetDib();
  if (dib) {
    if (hasPalette) {
      palette = new TPalette(*GetDib());
      dc.SelectObject(*palette);
      dc.RealizePalette();
    }

    // figure out upper left corner of the client area
    //
    TRect clientRect(GetClientRect());
    TPoint sourcePoint(0, 0);

    // adjust the upper left corner for centering picture
    //
    if (HowToDisplay == Center) {
      // determine offsets
      //
      int offsetX = abs(dib->Width() - clientRect.Width()) / 2;
      if (dib->Width() > clientRect.Width())
        sourcePoint.x += offsetX;
      else
        clientRect.Offset(offsetX, 0);

      int offsetY = abs(dib->Height() - clientRect.Height()) / 2;
      if (dib->Height() > clientRect.Height())
        sourcePoint.y += offsetY;
      else
        clientRect.Offset(0, offsetY);
    }

    // adjust the lower right corner
    //
    if (HowToDisplay != Stretch) {
      clientRect.bottom = clientRect.top + dib->Height();
      clientRect.right  = clientRect.left + dib->Width();

      // if the picture is larger than screen dimensions,
      // adjust the upper left corner.
      //
      clientRect.top   -= sourcePoint.y;
      clientRect.left  -= sourcePoint.x;
    }

    // display the dib
    //
    switch (HowToDisplay) {
      case UpperLeft:
      case Center:
        dc.SetDIBitsToDevice(clientRect, sourcePoint, *dib);
//        if(!dc.SetDIBitsToDevice(clientRect, sourcePoint, *dib)){
//          TSystemMessage().MessageBox();
//        }
        break;
      case Stretch: {
        TRect sourceRect(0, 0, dib->Width(), dib->Height());
        dc.StretchDIBits(clientRect, sourceRect, *dib);
        break;
      }
    } // switch HowToDisplay

    dc.RestoreObjects();
  }
}


//
/// Changes the formatting of the DIB.
//
void
TPictureWindow::SetHowToDisplay(TDisplayHow how)
{
  HowToDisplay = how;
  if (IsWindow())
    Invalidate(true);
}


//
/// Overridden from TWindow. Returns a unique name to force GetWindowClass to be
/// called.
//
TWindow::TGetClassNameReturnType
TPictureWindow::GetClassName()
{
  return _T("OWL_PictureWindow");
}


//
/// Overridden from TWindow.
//
void
TPictureWindow::GetWindowClass(WNDCLASS& wndClass)
{
  TWindow::GetWindowClass(wndClass);
  wndClass.style |= CS_HREDRAW | CS_VREDRAW;
}


} // OWL namespace
/* ========================================================================== */


//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// An implementation of the hatched border around inplace active windows.
//----------------------------------------------------------------------------
#ifndef _BHATCH_H
#define _BHATCH_H

#include "bolesite.h"

#ifndef MOVEWIN_H
#include "movewin.h"
#endif

#define CLIPPED              0x0001
#define CLIPPED_LEFT         0x0002
#define CLIPPED_TOP          0x0004
#define CLIPPED_TOPLEFT      CLIPPED_LEFT|CLIPPED_TOP

void SetRectExt (RECT FAR *,UINT,UINT,UINT);

class BOleHatchWindow {

public:

  BOleHatchWindow (HWND,HINSTANCE,BOleSite*);
  ~BOleHatchWindow();

  static           Register (HINSTANCE hInst); // register window
  static void     InitWidth ();

  // window messages related member functions
  LRESULT         wmCreate (LPCREATESTRUCT);
  LRESULT         wmPaint ();
  UINT             wmNCHitTest (LPPOINT);
  LRESULT         wmSetCursor (HWND, UINT, UINT);
  UINT             wmMouseActivate (HWND, UINT, UINT);
  LRESULT         wmNCLButtonDown (UINT, LPPOINT);
  LRESULT         wmMouseMove (UINT, LPPOINT);
  LRESULT         wmLButtonUp (UINT, LPPOINT);
  LRESULT         wmGetMinMaxInfo (MINMAXINFO*);
  LRESULT         wmWindowPosChanging (LPWINDOWPOS);

  void             Show (BOOL, LPOLEINPLACESITE = NULL);
  void             SetSize (LPRECT,LPRECT,LPPOINT);
  void             NegotiateSize (WINDOWPOS*);
  UINT             GetWidth ();

  operator         HWND () {return hWndHatch;}

protected:

  static UINT       hatchWidth; // hatch border size

  HWND             hWndHatch; // hatch window handle
  RECT             hatchRect; // keep server window rectangle in client coordinates

  int             nClippedFlag; // clipped status flag
  RECT             MoveRect; // temp rect used only inside WM_WINDOWPOSCHANGING (orrible!)
  BOOL             fInMouseDown; // used to signal when mouse left button is kept down

  GrayGhostRect*  GrayFrame; // gray frame showing window movement

  LPOLEINPLACESITE  pSite;
  BOleSite *      pBack;

};

#endif

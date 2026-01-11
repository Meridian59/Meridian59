//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// An implementation of the hatched border around inplace active windows.
//
//  Modifications to OLE2UI version:
//    - C++ version
//    - removed restriction that parent must be non-null in CreateHatchWindow
//    - changed window from a child to a popup to allow null parent
//    - "extra bytes" removed in favor of member data
//    - added Paint() and InitWidth() to unclutter wnd proc
//    - added hit testing and correct cursors for handles
//    - added resizing for handles
//    - added NegotiateSize() to mediate inplace size negotiations between
//      the container and the server
//----------------------------------------------------------------------------
#include "bole.h"

#define STRICT
#include "ole2ui.h"
#include "bhatch.h"

// class name of hatch window
  #define CLASS_HATCH TEXT("BOle Hatch Window")

// local function prototypes
LRESULT FAR PASCAL EXPORT BOleHatchWndProc (HWND, UINT, WPARAM, LPARAM);

// static initializer
UINT BOleHatchWindow::hatchWidth = 0;

// global functions
void SetRectExt (LPRECT r, UINT x, UINT y, UINT extent)
{
  r->left = x;
  r->top = y;
  r->right = x + extent;
  r->bottom = y + extent;
}

BOleHatchWindow::BOleHatchWindow(HWND hWndParent, HINSTANCE hInst, BOleSite *p)
{
  pBack = p; // assign BOleSite
  hWndHatch = ::CreateWindowEx(
    0,
    CLASS_HATCH,    // Class name
    TEXT ("Hatch Window"), // Window name
    WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    GetDesktopWindow(),
    (HMENU)NULL,
    hInst,
    this
  );

  hatchRect.left = hatchRect.top = hatchRect.bottom = hatchRect.right = 0;
  MoveRect.left = MoveRect.top = MoveRect.bottom = MoveRect.right = 0;
  nClippedFlag = 0;
  pSite = NULL;
  fInMouseDown = FALSE;
  GrayFrame = NULL;
}

//***************************************************************************
// Destructor: ~BOleHatchWindow
//
// Purpose:
//              Public destructor to destroy the hWnd
//
//***************************************************************************
BOleHatchWindow::~BOleHatchWindow ()
{
  if (hWndHatch)
    ::DestroyWindow (hWndHatch);
}

//***************************************************************************
// Static member function: Register
//
// Purpose:
//      Register the hatch window
//
// Parameters:
//      hInst                   Process instance
//
// Return Value:
//      TRUE                    if successful
//      FALSE                   if failed
//
//***************************************************************************
BOOL BOleHatchWindow::Register (HINSTANCE hInst)
{
  WNDCLASS wc;

  // Register Hatch Window Class
  wc.style = CS_BYTEALIGNWINDOW;
  wc.lpfnWndProc = BOleHatchWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = sizeof(BOleHatchWindow*);    // 'this' in extra bytes
  wc.hInstance = hInst;
  wc.hIcon = NULL;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = CLASS_HATCH;

  if (!::RegisterClass(&wc))
    return FALSE;
  else
    return TRUE;
}

//***************************************************************************
// Static member function: InitWidth
//
// Purpose:
//              Initialize hatch border with from WIN.INI
//
//***************************************************************************
void BOleHatchWindow::InitWidth ()
{
  hatchWidth = GetProfileInt (TEXT ("windows"), TEXT ("oleinplaceborderwidth"),
           DEFAULT_HATCHBORDER_WIDTH);
}

//***************************************************************************
// Member function: GetWidth
//
// Purpose:
//      Get width of hatch border
//
// Parameters:          None
//
// Return Value:
//    UINT                                      width of the hatch border
//***************************************************************************
UINT BOleHatchWindow::GetWidth ()
{
  if (!::IsWindow(hWndHatch))
    return 0;

  return hatchWidth;
}

//****************************************************************
// window messages related member functions
//
// Purpose:
//              Window Procedure dispatches all messages here.
//      Each member functions got the message name.
//              Do messages processing here.
//              Refer to windows help to get information about messages
//
//****************************************************************

//****************************************************************
// Member function: wmCreate
//
// Parameters:
//                      LPCREATESTRUCT  struct containing information about the window
//                                                                      to be created. Not used
//
// Return Value:
//                      LRESULT                         0L to continue creation process
//
//****************************************************************
LRESULT BOleHatchWindow::wmCreate (LPCREATESTRUCT)
{
  BOleHatchWindow::InitWidth ();
  return 0L;
}

//***************************************************************************
// Member function: wmPaint
//
// Purpose:
//              Draw hatched border and handles
//
// Parameters:                          None
//
// Return Value:
//                      LRESULT                         0L
//
//***************************************************************************
LRESULT BOleHatchWindow::wmPaint ()
{
  HDC hDC;
  PAINTSTRUCT ps;
  RECT rcHatchRect;

  hDC = ::BeginPaint(hWndHatch, &ps);
  // get hatchRect to paint hatch border
  //rcHatchRect = hatchRect;
  ::CopyRect (&rcHatchRect, &hatchRect);
  OleUIDrawShading(&rcHatchRect, hDC, OLEUI_SHADE_BORDERIN, hatchWidth);

  // get the real visible rect to draw handles
  GetWindowRect(hWndHatch, (LPRECT)&rcHatchRect);
  SetRect((LPRECT)&rcHatchRect,0, 0, rcHatchRect.right-rcHatchRect.left,
                  rcHatchRect.bottom-rcHatchRect.top);
  ::InflateRect ((LPRECT)&rcHatchRect, -hatchWidth, -hatchWidth);
  OleUIDrawHandles (&rcHatchRect, hDC, OLEUI_HANDLES_OUTSIDE,
    hatchWidth+1, TRUE);
  ::EndPaint (hWndHatch, &ps);
  return 0L;
}

//****************************************************************
// Member function: wmNCHitTest
//
// Purpose:
//                      Find out which handle the cursor is over
//
// Parameters:
//       LPPOINT                mouse position
//
// Return Value:
//       UINT                   a value indicating the position of the cursor
//
//****************************************************************
UINT BOleHatchWindow::wmNCHitTest (LPPOINT lpPoint)
{
  RECT handleRect;
  RECT rc; // contain hatch window rect

  // we want lpPoint to be in client coordinates
  ::ScreenToClient (hWndHatch, lpPoint);

  GetWindowRect(hWndHatch, (LPRECT)&rc);
  SetRect((LPRECT)&rc,0,0,rc.right-rc.left,rc.bottom-rc.top);

  SetRectExt (&handleRect, rc.left, rc.top, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTTOPLEFT;

  SetRectExt (&handleRect, rc.left, (rc.top+rc.bottom-hatchWidth + 1)/2, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTLEFT;

  SetRectExt (&handleRect, rc.left, rc.bottom - hatchWidth + 1, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTBOTTOMLEFT;

  SetRectExt (&handleRect, (rc.left+rc.right-hatchWidth + 1)/2, rc.top, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTTOP;

  SetRectExt (&handleRect, (rc.left+rc.right-hatchWidth + 1)/2, rc.bottom - hatchWidth + 1, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTBOTTOM;

  SetRectExt (&handleRect, rc.right-hatchWidth + 1, rc.top, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTTOPRIGHT;

  SetRectExt (&handleRect, rc.right-hatchWidth + 1, (rc.top+rc.bottom-hatchWidth + 1)/2, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTRIGHT;

  SetRectExt (&handleRect, rc.right-hatchWidth + 1, rc.bottom-hatchWidth + 1, hatchWidth + 1);
  if (::PtInRect(&handleRect, *lpPoint))  return HTBOTTOMRIGHT;

  // If we've gotten this far, the cursor is not over a handle, but it
  // is over the hatch window (because we got the message). This means
  // the cursor is over the hatched border. It can't be over our client
  // area because the server object takes up the client area.
  //
  // So, by telling Windows that the cursor is over the caption bar, we
  // can piggy-back on Windows' code for dragging windows around.
  //
  return HTCAPTION;
}

//****************************************************************
// Member function: wmSetCursor
//
//      Purpose:
//                      Set the cursor bitmap to match the handle it's over.
//
// Parameters:
//                      HWND                    handle of the window with the cursor
//       UINT                   only used. Contains the value returned from wmNCHitTest
//                      UINT        Mouse message number
//
// Return Value:
//                      LRESULT         TRUE, to stop further processing
//
//****************************************************************
LRESULT BOleHatchWindow::wmSetCursor (HWND, UINT hitTestCode, UINT)
{

  switch (hitTestCode) {
    case HTTOPLEFT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENWSE)));
      break;
    case HTLEFT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZEWE)));
      break;
    case HTBOTTOMLEFT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENESW)));
      break;
    case HTTOP:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENS)));
      break;
    case HTBOTTOM:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENS)));
      break;
    case HTTOPRIGHT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENESW)));
      break;
    case HTRIGHT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZEWE)));
      break;
    case HTBOTTOMRIGHT:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_SIZENWSE)));
      break;
    default:
      ::SetCursor (::LoadCursor (NULL, TResId(IDC_ARROW)));
  }

  return (LRESULT)TRUE;
}

//****************************************************************
// Member function: wmMouseActivate
//
// Parameters:
//                      HWND                    handle of the top level parent. Not used
//       UINT                   hit test code. Not used
//                      UINT        Mouse message identifier. Not used
//
// Return Value:
//                      UINT                    MA_NOACTIVATE does not activate the window
//
//****************************************************************
UINT BOleHatchWindow::wmMouseActivate (HWND, UINT, UINT)
{
  return MA_NOACTIVATE;
}

//****************************************************************
// Member function: wmNCLButtonDown
//
// Parameters:
//                      UINT            hit test code as returned from WM_NCHITTEST
//                      LPOINT                  cursor coordinates
//
// Return Value:
//                      LRESULT         let it process by DefWindowProc
//
//****************************************************************
LRESULT BOleHatchWindow::wmNCLButtonDown (UINT nTestCode, LPPOINT lpPoint)
{
  RECT WinRect;

  GetWindowRect( hWndHatch, (LPRECT)&WinRect);

  // first resize window if needed
  if (nClippedFlag & CLIPPED) {
    // if clipped left or top hatchRect adjust top and/or left
    if (nClippedFlag & CLIPPED_LEFT)
      WinRect.left += hatchRect.left;
    if (nClippedFlag & CLIPPED_TOP)
      WinRect.top += hatchRect.top;

    WinRect.right = WinRect.left + hatchRect.right - hatchRect.left;
    WinRect.bottom = WinRect.top + hatchRect.bottom - hatchRect.top;
  }

  // set flag indicating the user is moving the window and initiating
  // all space negotiation procedure
  fInMouseDown=TRUE;

  //\\//\\//\\/\\//\\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
  ////\\//\\//\\/\\//\\/\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
  GrayFrame = new GrayGhostRect (WinRect, nTestCode);
  GrayFrame->SetStartMove (*lpPoint);
  SetCapture (hWndHatch);
  GrayFrame->MoveFrame (*lpPoint, FALSE);

  return 0L;
  //return ::DefWindowProc (hWndHatch, WM_NCLBUTTONDOWN, nTestCode, *((long *)lpPoint));
}

//****************************************************************
// Member function: wmMouseMove
//
// Purpose:
//   Move around the fake window rect in XOR with the screen
//
// Parameters:
//   UINT            hit test code as returned from WM_NCHITTEST
//  LPOINT          cursor coordinates
//
// Return Value:
//  LRESULT         0L
//
//****************************************************************
LRESULT BOleHatchWindow::wmMouseMove (UINT nTestCode, LPPOINT lpPoint)
{
  if (fInMouseDown) {
    // transform point in screen coordinates
    ClientToScreen (hWndHatch, lpPoint);
    GrayFrame->MoveFrame (*lpPoint, TRUE);

    return 0L;
  }
  else {
    DWORD lParam = MAKELONG((WORD)(lpPoint->x),(WORD)(lpPoint->y));
    return ::DefWindowProc (hWndHatch, WM_MOUSEMOVE, nTestCode, lParam);
  }

}

//****************************************************************
// Member function: wmLButtonUp
//
// Parameters:
//  UINT            hit test code as returned from WM_NCHITTEST
//  LPOINT          cursor coordinates
//
// Return Value:
//  LRESULT         0L
//
//****************************************************************
LRESULT BOleHatchWindow::wmLButtonUp (UINT nTestCode, LPPOINT lpPoint)
{
  if (fInMouseDown) {
    POINT TopLeft;
    POINT WidthHeight;

    ClientToScreen (hWndHatch, lpPoint);
    GrayFrame->MoveFrame (*lpPoint, FALSE);

    GrayFrame->UpperCorner (TopLeft);
    GrayFrame->Size (WidthHeight);
    ScreenToClient (GetParent (hWndHatch), (LPPOINT)&TopLeft);
    // move the window now
    MoveWindow (hWndHatch, TopLeft.x, TopLeft.y, WidthHeight.x, WidthHeight.y, TRUE);
    InvalidateRect (hWndHatch, NULL, TRUE);

    delete GrayFrame;

    // release mouse
    ReleaseCapture();

    return 0L;
  }
  else {
    DWORD lParam = MAKELONG((WORD)(lpPoint->x),(WORD)(lpPoint->y));
    return ::DefWindowProc (hWndHatch, WM_LBUTTONUP, nTestCode, lParam);
  }

}

//****************************************************************
// Member function: wmGetMinMaxInfo
//
// Parameters:
//                      LPMINMAXINFO    struct containing resizing information
//
// Return Value:
//                      LRESULT                 0L when processing this message
//
//****************************************************************
LRESULT BOleHatchWindow::wmGetMinMaxInfo (MINMAXINFO* pMMI)
{
  int minWidth = (GetWidth() * 2) + 15;
  pMMI->ptMinTrackSize.x = minWidth;
  pMMI->ptMinTrackSize.y = minWidth;
  return 0L;
}

//****************************************************************
// Member function: wmWindowPosChanging
//
// Purpose:
//       Call negotiate size to activate the space negotiation
//                      procedure and, in case, changes the WINDOWPOS values
//                      so to accomplish the new size
//
// Parameters:
//       LPWINDOWPOS    structure containing information about
//                                                      resizing values
//
// Return Value:
//                      LRESULT
//
//****************************************************************
LRESULT BOleHatchWindow::wmWindowPosChanging (LPWINDOWPOS pWP)
{
      if (fInMouseDown) {
           int minWidth = (GetWidth() * 2) + 15;
           if (pWP->cx < minWidth) {
                pWP->cx = minWidth;
           }
           if (pWP->cy < minWidth) {
                pWP->cy = minWidth;
           }
           NegotiateSize (pWP);
           if (nClippedFlag & CLIPPED) {
                pWP->x = MoveRect.left;
                pWP->y = MoveRect.top;
                pWP->cx = MoveRect.right;
                pWP->cy = MoveRect.bottom;
                pWP->flags = 0;
           }
           ::InvalidateRect(hWndHatch, NULL, TRUE);
           fInMouseDown = FALSE;
           return 0L;
      }

      return ::DefWindowProc (hWndHatch, WM_WINDOWPOSCHANGING, 0, (DWORD)(LPVOID)pWP);
}

//***************************************************************************
// Member function: Show
//
// Purpose:
//              Show or hide the hatch window; provide access to the site
//
// Parameters:
//              BOOL                                    TRUE to show the hatch window, FALSE to hide it
//              LPOLEINPLACESITE        Pointer to the in place site, which is used to
//                                              negotiate new size when a handle is dragged
//
//***************************************************************************
void BOleHatchWindow::Show (BOOL fShow, LPOLEINPLACESITE pS)
{
  pSite = pS;
  ::ShowWindow (hWndHatch, fShow ? SW_SHOW : SW_HIDE);
}

//***************************************************************************
//      Member function: SetSize
//
// Purpose:
//              Move/size the HatchWindow correctly given the rect required by the
//              in-place server object window and the lprcClipRect imposed by the
//              in-place container. both rect's are expressed in the client coord.
//              of the in-place container's window (which is the parent of the
//              HatchWindow). When fInMouseDown is TRUE must not use MoveWindow
//                      but change the values in the WINDOWPOS struct inside the
//                      WM_WINDOWPOSCHANGING message (it seems Windows code is not re-entrant
//       there)
//
//              OLE2NOTE: the in-place server must honor the lprcClipRect specified
//              by its in-place container. it must NOT draw outside of the ClipRect.
//              in order to achieve this, the hatch window is sized to be
//              exactly the size that should be visible (rcVisRect). the
//              rcVisRect is defined as the intersection of the full size of
//              the HatchRect window and the lprcClipRect.
//              the ClipRect could infact clip the HatchRect on the
//              right/bottom and/or on the top/left. if it is clipped on the
//              right/bottom then it is sufficient to simply resize the hatch
//              window. but if the HatchRect is clipped on the top/left then
//              in-place server document window (child of HatchWindow) must be moved
//              by the delta that was clipped. the window origin of the
//              in-place server window will then have negative coordinates relative
//              to its parent HatchWindow.
//
// Parameters:
//              LPRECT                  full size of in-place server object window
//              LPRECT                  clipping rect imposed by in-place container
//              LPPOINT         offset required to position in-place server object
//                      window properly. caller should call:
//                                                              OffsetRect(&rcObjRect,lpptOffset->x,lpptOffset->y)
//
//      Return Value:                   None
//
//***************************************************************************
void BOleHatchWindow::SetSize(
    LPRECT      lprcIPObjRect,
    LPRECT      lprcClipRect,
    LPPOINT     lpptOffset)
{

  RECT        rcHatchRect;
  RECT        rcVisRect;
  UINT        uHatchWidth;
  POINT       ptOffset;

  if (!::IsWindow(hWndHatch))
    return;

  rcHatchRect = *lprcIPObjRect;
  uHatchWidth = GetWidth();
  ::InflateRect((LPRECT)&rcHatchRect, uHatchWidth + 1, uHatchWidth + 1);

  // find the intersection between hatch (server) window and
  // clipping area (area where the object is allowed to paint)
  ::IntersectRect (&rcVisRect, &rcHatchRect, lprcClipRect);

  // move the hatch window at the intersection rectangle if you are not
  // dragging the window around
  if (!fInMouseDown) {
    ::MoveWindow(
        hWndHatch,
        rcVisRect.left,
        rcVisRect.top,
        rcVisRect.right-rcVisRect.left,
        rcVisRect.bottom-rcVisRect.top,
        TRUE    /* fRepaint */
    );
    ::InvalidateRect(hWndHatch, NULL, TRUE);
  }
  else {
    // this information will be used in the WM_WINDOWPOSCHANGING
    // message to change WINDOWPOS structure values
     ::SetRect(&MoveRect, rcVisRect.left, rcVisRect.top,
                          rcVisRect.right-rcVisRect.left,
                          rcVisRect.bottom-rcVisRect.top);
  }


  nClippedFlag = 0; // clear clipped flag
  // store clipping information
  if (!(::EqualRect (&rcVisRect, &rcHatchRect))) {
    nClippedFlag |= CLIPPED;
    if (rcVisRect.left != rcHatchRect.left) {
      nClippedFlag |= CLIPPED_LEFT;
    }
    if (rcVisRect.top != rcHatchRect.top) {
      nClippedFlag |= CLIPPED_TOP;
    }
  }

  // convert the rcHatchRect into the client coordinate system of the
  // HatchWindow itself
  ptOffset.x = - rcVisRect.left;
  ptOffset.y = - rcVisRect.top;
  ::OffsetRect(&rcHatchRect, ptOffset.x, ptOffset.y);
  ::CopyRect (&hatchRect, &rcHatchRect);
  //hatchRect = rcHatchRect;

  // calculate offset required to position in-place
  // server doc window (output value)
  lpptOffset->x = ptOffset.x;
  lpptOffset->y = ptOffset.y;

  return;
}

//***************************************************************************
// Member function: NegotiateSize
//
// Purpose:
//              When the user drags a handle of the hatch window, Windows attempts
//              to resize the window. This routine catches that attempt and validates
//              it with the client. We don't need to validate it with the server
//              because the client will call the server back from OnPosRectChange to
//              BOleSite::SetObjectRects.
//
//      Parameters:
//              WINDOWPOS*                              Pointer to Windows' WINDOWPOS structure
//
//***************************************************************************
void BOleHatchWindow::NegotiateSize (WINDOWPOS *pwp)
{
  RECT rcWnd; // The pre-resize size of the hatch window

  // As an optimization, get the current size of the hatch window
  // and bail out if it's the same as the new size. This saves the
  // potentially expensive renegotiation.
  //
  ::GetWindowRect (hWndHatch, &rcWnd);
  HWND parent = ::GetParent (hWndHatch);
  ::ScreenToClient (parent, (LPPOINT) &rcWnd.left);
  ::ScreenToClient (parent, (LPPOINT) &rcWnd.right);
      //if ( rcWnd.left == pwp->x && rcWnd.top == pwp->y &&
      //        (rcWnd.bottom - rcWnd.top == pwp->cy) &&
      //        (rcWnd.right - rcWnd.left == pwp->cx))
  //        return;

  // Ask the client to resize to the new size of the window
  //
  RECT rcNew;

  // The new size of the server's inplace window
  ::SetRect (&rcNew, pwp->x, pwp->y, pwp->x + pwp->cx, pwp->y + pwp->cy);
  ::InflateRect (&rcNew, -(hatchWidth + 1), -(hatchWidth + 1));
  if (pSite) {
    // Ask the server to show more/less of object
    //
    if (!SUCCEEDED(pBack->SetSiteRect(&rcNew))) {
      SIZE size;
      // undo resize of the server
      size.cx = rcWnd.right - rcWnd.left;
      size.cy = rcWnd.bottom - rcWnd.top;
      pBack->pPart->SetPartSize(&size);

      // If the client refuses the new size, give the pre-resize
      // size of the hatch window, back to Windows through the WINDOWPOS
      //
      pwp->x = rcWnd.left;
      pwp->y = rcWnd.top;
      pwp->cx = rcWnd.right - rcWnd.left;
      pwp->cy = rcWnd.bottom - rcWnd.top;
    }
  }
}

//***************************************************************************
//      BOleHatchWndProc
//
// Purpose:
//                      WndProc for hatch window
//
// Parameters:
//                      hWnd
//                      Message
//                      wParam
//                      lParam
//
// Return Value:
//                      message dependent
//***************************************************************************
LRESULT FAR PASCAL EXPORT BOleHatchWndProc (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  BOleHatchWindow *pHW  = (BOleHatchWindow*) ::GetWindowLong(hWnd, 0);

  // if pHW is NULL, assign it if WM_NCCREATE otherwise
  // return DefWindowProc
  if (!pHW) {
    if (Message==WM_NCCREATE) {

      CREATESTRUCT FAR* lpcs = (CREATESTRUCT FAR*)lParam;
      ::SetWindowLong(hWnd, 0, (DWORD)(LPVOID)(lpcs->lpCreateParams));
    }
    return ::DefWindowProc (hWnd, Message, wParam, lParam);
  }

  switch (Message) {
    case WM_CREATE:
      return pHW->wmCreate((LPCREATESTRUCT)lParam);

    case WM_PAINT:
      return pHW->wmPaint ();

    case WM_GETMINMAXINFO:
      return pHW->wmGetMinMaxInfo((MINMAXINFO*)lParam);

    // The cursor setting strategy here is to catch the WM_NCHITTEST
    // first to figure out which hit-test area the cursor is over.
    // Then Windows calls us back with WM_SETCURSOR so we can make
    // the cursor the correct shape for that hit-test area.
    //
    case WM_NCHITTEST:
      if (pHW) {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        return pHW->wmNCHitTest ((LPPOINT) &pt);
      }

    // Any window that is used during in-place activation
    // must handle the WM_SETCURSOR message or else the cursor
    // of the in-place parent will be used. if WM_SETCURSOR is
    // not handled, then DefWindowProc sends the message to the
    // window's parent.
    //
    case WM_SETCURSOR:
      return pHW->wmSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_MOUSEACTIVATE:
      return pHW->wmMouseActivate((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    // The window move/resize strategy here is:
    // - Keep control so we can allow the OLE2 client app to refuse
    //   the new dimensions. Don't wait until the window has already
    //   been moved and then move it back.
    // - In order to keep control, we catch the WM_WINDOWPOSCHANGING
    //   and set appropriate flags in the WINDOWPOS struct.
    // - Since WM_WINDOWPOSCHANGING is sent for both user-interface
    //   initiated actions and for ::MoveWindow-initiated changes, we
    //   keep a flag which prevents changes during mousedowns
    //
    //case WM_NCLBUTTONDOWN:
    //  return pHW->wmNCLButtonDown(wParam, (LPPOINT)&lParam);

    case WM_NCLBUTTONDOWN: {
      short xPos, yPos;
      POINT pt;
      xPos = LOWORD(lParam);
      yPos = HIWORD(lParam);
      pt.x = xPos;
      pt.y = yPos;
      return pHW->wmNCLButtonDown(wParam, (LPPOINT)&pt);
      //return ::DefWindowProc(hWnd, Message, wParam, lParam);
    }

    case WM_MOUSEMOVE: {
      short xPos, yPos;
      POINT pt;
      xPos = LOWORD(lParam);
      yPos = HIWORD(lParam);
      pt.x = xPos;
      pt.y = yPos;
      return pHW->wmMouseMove(wParam, (LPPOINT)&pt);
      //return ::DefWindowProc(hWnd, Message, wParam, lParam);
    }

    case WM_LBUTTONUP: {
      short xPos, yPos;
      POINT pt;
      xPos = LOWORD(lParam);
      yPos = HIWORD(lParam);
      pt.x = xPos;
      pt.y = yPos;
      return pHW->wmLButtonUp(wParam, (LPPOINT)&pt);
      //return ::DefWindowProc(hWnd, Message, wParam, lParam);
    }

    case WM_WINDOWPOSCHANGING:
      return pHW->wmWindowPosChanging((WINDOWPOS*) lParam);

    default:
      return ::DefWindowProc (hWnd, Message, wParam, lParam);
  }
}




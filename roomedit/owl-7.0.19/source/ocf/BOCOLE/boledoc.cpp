//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Implements the Bolero half of the document-level object. Provides plumbing
// for activation of OLE2 objects as document windows become active.
// Also the main drag/drop and clipboard implementation.
//----------------------------------------------------------------------------
#include "boledoc.h"
#include "bolesvc.h"
#include "bolepart.h"
#include "bolectrl.h"


//**************************************************************************
//
// Initialize static/global data
//
//**************************************************************************

// IOleUILinkContainer is only used here

#include "initguid.h"
DEFINE_GUID(IID_IOleUILinkContainer, 0x000004FF, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
PREDECLARE_INTERFACE(IOleUILinkContainer)
DEFINE_INLINE_QI(IOleUILinkContainer, IUnknown)

// CLIPFORMAT is a WORD while RegisterClipboardFormat returns a UINT

CLIPFORMAT BOleDocument::oleEmbdObjClipFmt   = RegisterClipboardFormat (TEXT ("Embedded Object"));
CLIPFORMAT BOleDocument::oleEmbSrcClipFmt    = RegisterClipboardFormat (TEXT ("Embed Source"));
CLIPFORMAT BOleDocument::oleLinkSrcClipFmt   = RegisterClipboardFormat (TEXT ("Link Source"));
CLIPFORMAT BOleDocument::oleLinkSrcDescFmt   = RegisterClipboardFormat (TEXT ("Link Source Descriptor"));

CLIPFORMAT BOleDocument::oleLinkClipFmt      = RegisterClipboardFormat (TEXT ("ObjectLink"));
CLIPFORMAT BOleDocument::oleNativeClipFmt    = RegisterClipboardFormat (TEXT ("Native"));
CLIPFORMAT BOleDocument::oleOwnerLinkClipFmt = RegisterClipboardFormat (TEXT ("OwnerLink"));
CLIPFORMAT BOleDocument::oleObjectDescFmt    = RegisterClipboardFormat (TEXT ("Object Descriptor"));

UINT BOleDocument::dragScrollInset    = GetProfileInt (TEXT ("windows"), TEXT ("DragScrollInset"), DD_DEFSCROLLINSET);
UINT BOleDocument::dragScrollDelay    = GetProfileInt (TEXT ("windows"), TEXT ("DragScrollDelay"), DD_DEFSCROLLDELAY);
UINT BOleDocument::dragScrollInterval = GetProfileInt (TEXT ("windows"), TEXT ("DragScrollInterval"), DD_DEFSCROLLINTERVAL);

#ifdef OLEDBG
OLEDBGDATA;
#endif

//**************************************************************************
//
// BOleDocument implementation
//
//**************************************************************************

HRESULT _IFUNC BOleDocument::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // self

  if (iid == IID_BOleDocument) {
  (BOleDocument*) *ppv = this;
  AddRef ();
  return NOERROR;
  }

  // interfaces

   SUCCEEDED(hr = IOleInPlaceUIWindow_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IDropTarget_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IBDocument_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IOleUILinkContainer_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IOleInPlaceFrame_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IBOverrideBrowseLinks_QueryInterface(this, iid, ppv))

  // base classes

  || SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))

  // helpers
  ;

  return hr;
}

BOleDocument::BOleDocument (BOleClassManager *pF, IBUnknownMain * pObj, BOleService * pOS) :
  BOleComponent(pF, pObj), pService (pOS), fInBrowseLinks(FALSE),
  pActivePart (NULLP), pActiveCtrl (NULLP), pFirstPart(NULLP),
  pFirstCtrl(NULLP), fRegistered(FALSE)
{
  pContainer = NULLP;
  pApplication = NULLP;

#ifdef OLEDBG
  DebugMode = 0;
#endif

  WIN::SetRect (&rcScrollTop,0,0,0,0);
  WIN::SetRect (&rcScrollBottom,0,0,0,0);
  WIN::SetRect (&rcScrollLeft,0,0,0,0);
  WIN::SetRect (&rcScrollRight,0,0,0,0);
  scrollEnterTime = 0;
  scrollLastTime = 0;
  fScrollDelay = FALSE;

  dndObjPos.left = dndObjPos.top = dndObjPos.right = dndObjPos.bottom = 0;
  dndLastPos.x = dndLastPos.y = -1;
  dndSize.cx = dndSize.cy = 0;
  dndOffset.x = dndOffset.y = 0;
  dndInfoAvailable = FALSE;

  dndFormatCount = 0;
  dndFormatList = NULL;
  fDrawn = FALSE;
}

BOleDocument::~BOleDocument ()
{
  if (pService->GetActiveDoc() == this)
    pService->SetActiveDoc (NULL);

  if (pActivePart)  {
    pActivePart->Release ();
    pActivePart = NULL;
  }
  if (pActiveCtrl)  {
    pActiveCtrl->Release ();
    pActiveCtrl = NULL;
  }
}

//**************************************************************************
//
// IBDocument implementation
//
//**************************************************************************

HRESULT _IFUNC BOleDocument::Init( IBContainer *pBack)
{
  HRESULT hRes = NOERROR;

  pContainer = pBack;
  pApplication = pService->GetApplication();

  HWND hwnd = pContainer->GetWindow();

  if (!fRegistered) {

    // Register the document window as a Drag & Drop target. This
    // means that we can't drag onto the frame window, but that
    // doesn't seem like an oppressive restriction
    //
    if (hwnd) {

      CoLockObjectExternal (AsPIUnknown(pObjOuter), TRUE, TRUE);
      hRes = OLE::RegisterDragDrop(hwnd, this);
      fRegistered = SUCCEEDED(hRes);
    }
  }

  return hRes;
}

// Call when the container window in the client app was resized
//
void _IFUNC BOleDocument::OnResize ()
{
  if (pContainer) {
    RECT r;
    pContainer->GetWindowRect (&r);
    if (pActivePart) {

      // You would think that telling the object that its container has
      // resized would renegotiate the clip rect, but it doesn't seem to
      //
      pActivePart->ResizeBorder (&r, this, FALSE);

      // We renegotiate the clipping rect for them here
      //
      BOlePart *pWalk = pFirstPart;
      while (pWalk) {
        if (pWalk->IsInPlace()) {
          pWalk->ResetObjectRects();
          pWalk = NULL;
        }
        else
          pWalk = pWalk->GetNextPart();
      }
    }
  }
}

void _IFUNC BOleDocument::OnActivate (BOOL fActivate)
{
  // Remember the active document state
  //
  if (fActivate)
    pService->SetActiveDoc (this);
  else
    if (pService->GetActiveDoc() == this)
      pService->SetActiveDoc (NULL);

  // Make the OLE2 call to remove the inplace active object's
  // user interface
  //
  BOOL fMDI = (GetScode(pService->GetApplication()->IsMDI()) == S_OK);

  if (fMDI && pActivePart) {
    pActivePart->OnDocWindowActivate (fActivate);

    // Strangely, no OLE2 traffic is generated by deactivating the
    // document window of an inplace active object. I would expect
    // them to call SetBorderSpace(NULL), but the docs specifically
    // instruct *not* to do that. I suppose straight OLE2 apps can
    // just put back their UI in addition to calling OnDocWindowActivate,
    // but Bolero will notify clients to do it.
    //
    if (!fActivate)
      pService->GetApplication()->RestoreUI ();
  }
}

// Call just before the container window is destroyed
//
void _IFUNC BOleDocument::OnClose ()
{
  if (fRegistered) {
    HWND hwnd = pContainer->GetWindow();
    HRESULT hRes = OLE::RevokeDragDrop(hwnd);

    // If we can't unregister the window, it isn't registered and can't
    // be unlocked. In particular, CoLockObjectExternal can recursively
    // call OnClose, and calling CoLockObjectExternal recursively will
    // be a big mess
    //
    if (hRes == NOERROR) {
      CoLockObjectExternal (AsPIUnknown(pObjOuter), FALSE, TRUE);
      fRegistered = FALSE;
    }
    else
      fRegistered = TRUE;
  }
}

HRESULT _IFUNC BOleDocument::OnSetFocus(BOOL bSet)
{
  // The policy is that inplace active object always gets first shot at
  // keystrokes, but the practicality is that the user can set focus to
  // any old window by clicking on it. So, if some other window does
  // succeed in getting focus, it should give it immediately back to
  // the inplace server object.
  //
  if(bSet && pActivePart) {
    HWND w = NULL;
    HRESULT h = pActivePart->GetWindow(&w);
    if (SUCCEEDED(h)) {
      ::SetFocus (w);
      return ResultFromScode (S_FALSE);
    }
  }

  // The Bolero container app gets to keep focus since there was no
  // a inplace active server object
  //
  return ResultFromScode (S_OK);
}

HRESULT _IFUNC BOleDocument::EnumLinks (PIBLinkInfo FAR*)
{
  return ResultFromScode (E_NOTIMPL);
}

// UpdateLinks --  This function packages up some functionality of IBLinkInfo
//                and IBDocument::EnumLinks. The goal is to provide a
//                one-stop way for client apps to update the links in their
//                document. They are responsible for the dialog box, if any.
//
HRESULT _IFUNC BOleDocument::UpdateLinks ()
{
  BOlePart FAR *pWalk = pFirstPart->GetNextLink (TRUE);
  HRESULT hr = NOERROR;

  while (pWalk) {
    hr = pWalk->UpdateNow();
    pWalk = pWalk->GetNextLink (FALSE);
  }

  return hr;
}

HRESULT _IFUNC BOleDocument::BrowseLinks ()
{
  if( fInBrowseLinks )
    // Cannot nest link browser for the same document
    return ResultFromScode (E_FAIL);
  fInBrowseLinks++;

  UINT r = 0;
  OLEUIEDITLINKS      el;
  memset((LPOLEUIEDITLINKS)&el,0,sizeof(el));
  el.cbStruct = sizeof (OLEUIEDITLINKS);
  el.hWndOwner = ::GetActiveWindow(); // pContainer->GetWindow ();
  el.lpOleUILinkContainer = this;
  if (pService->ShowHelpButton(BOLE_HELP_BROWSELINKS))
    el.dwFlags = ELF_SHOWHELP;

  pService->EnterBOleDialog (el.hWndOwner, NULL, NULL);

  r = OleUIEditLinks (&el);
  pService->ExitBOleDialog ();

  fInBrowseLinks--;

  if (r == 1)
    return ResultFromScode (S_OK);
  return ResultFromScode (S_FALSE);
}

// This guy is private implementation called by BOleService because the
// document knows if any parts are links
//
BOOL _IFUNC BOleDocument::EnableBrowseLinks ()
{
  return pFirstPart && pFirstPart->GetNextLink (TRUE) != NULLP;
}

//**************************************************************************
//
// IOleWindow implementation
//
//**************************************************************************

HRESULT _IFUNC BOleDocument::GetWindow (HWND FAR *phwnd)
{
  *phwnd = pContainer->GetWindow();
  OLERET(S_OK);
}

HRESULT _IFUNC BOleDocument::ContextSensitiveHelp (BOOL fEnterMode)
{
  pService->SetHelpMode (fEnterMode);
  OLERET(S_OK);
}

//**************************************************************************
//
// IOleInPlaceUIWindow implementation
//
//**************************************************************************

HRESULT _IFUNC BOleDocument::GetBorder (LPRECT prectBorder)
{
  OLERET (SUCCEEDED(pContainer->GetWindowRect(prectBorder))
    ? S_OK : INPLACE_E_NOTOOLSPACE);
}

HRESULT _IFUNC BOleDocument::RequestBorderSpace (LPCRECT prectWidths)
{
  OLERET (SUCCEEDED(pContainer->RequestBorderSpace(prectWidths))
    ? S_OK : INPLACE_E_NOTOOLSPACE);
}

HRESULT _IFUNC BOleDocument::SetBorderSpace (LPCBORDERWIDTHS prectWidths)
{



  OLERET(SUCCEEDED(pContainer->SetBorderSpace(prectWidths))
    ? S_OK : OLE_E_INVALIDRECT);
}

HRESULT _IFUNC BOleDocument::SetActiveObject (IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
  // Decrement the ref count of the pointer we're giving up, and
  // bump the ref count of the new pointer we're going to hold
  //
  if (pActivePart)
    pActivePart->Release ();
  if (pActiveCtrl) {
    pActiveCtrl->Release();
    pActiveCtrl = NULL;
  }

  pActivePart = pActiveObject;
  if (pActivePart) {
    pActivePart->AddRef ();
    pActivePart->QueryInterface (IID_IOleControl, &(LPVOID)pActiveCtrl);
  }

  // Although it seems a little strange, we're supposed to put the
  // name of the object in our caption bar in the client.
  //
  if (pszObjName)
    pContainer->AppendWindowTitle (pszObjName);
  else
    // Null name means object is deactivating.
    pContainer->RestoreUI();

  OLERET (S_OK);
}

//**************************************************************************
//
// IDropTarget implementation (and some helper functions)
//
//**************************************************************************

// Helper function called by IDropTarget functions to make sure the object the
// user is trying to drop is acceptable to the application
//
short _IFUNC BOleDocument::ValidateDrag(DWORD keyState, POINTL *where, DWORD *effect, IDataObject *dataObj)
{
  // No container formats -- no legal drop available
  //
  if (!dndFormatCount) {
    *effect = DROPEFFECT_NONE;
    dndFormat = 0;
    return -1;
  }

  IBApplication*pIA = pService->GetApplication();

  // Find out what the user wants to do by looking at the modifier keys
  if (keyState & MK_CONTROL)
    *effect = (keyState & MK_SHIFT ? DROPEFFECT_LINK : DROPEFFECT_COPY);
  else
    // !(MK_SHIFT) is the "default" case for no modifier keys. In this case
    // the default has the same effect as shift, but it isn't required to
    //
    *effect = (keyState & MK_SHIFT ? DROPEFFECT_MOVE : DROPEFFECT_MOVE);

  // if the key state doesn't match with the server capabilities, fail
  // ie. don't allow user to press control shift if server doesn't allow link
  if (*effect == DROPEFFECT_LINK) {
    if (fCantLink) {
      *effect = DROPEFFECT_NONE;
      return -1;
    }
  }
  // dndFormat explanation: The reason dndFormat is in member data is that
  // for DragOver messages, we don't have a data object to negotiate with.
  // But we still want to give the format to the container window, so
  // dndFormat remembers what the last agreed format was and we can still
  // pass it back.
  //
  // Also, dndFormat is an actual clipboard format and agreedFormat is a
  // list index.

  // Now make sure the application supports what the user wants to do
  //
  BOOL canLink = (GetScode (pIA->CanLink()) == S_OK);
  BOOL canEmbed = (GetScode(pIA->CanEmbed()) == S_OK);
  if ((*effect & DROPEFFECT_LINK && !canLink) ||
    ((*effect & DROPEFFECT_COPY || *effect & DROPEFFECT_MOVE) && !canEmbed)) {
    *effect = DROPEFFECT_NONE;
    dndFormat = 0;
    return -1;
  }

  // Find the best data format the server and client can agree upon and
  // see if that format can be dropped at the current mouse location
  //
  // If we didn't get an IDataObject (it isn't passed to DragOver) we can't
  // validate the clipboard format, which is fine with us because it's pretty slow
  //
  if (dataObj) {
    short agreedFormat = -1;
    agreedFormat = pService->MatchPriorityClipFormat(dataObj,
      dndFormatList, dndFormatCount);
    if (agreedFormat == -1) {
      *effect = DROPEFFECT_NONE;
      dndFormat = 0;    //  to 0 to know we don't agree during DragOver
      return -1;
    }
    else {
      dndFormat = dndFormatList[agreedFormat].fmtetc.cfFormat;
      return agreedFormat;
    }
  }
  else
    // No data object -- no format to negotiate (don't touch dndFormat)
    //
    if (dndFormat == 0)      // in case we couldn't agree in DragEnter
      return -1;
    else
      return 1;

}

// In OLE2, the container (IDropTarget) may decide to scroll if certain
// conditions are met:
//    1. The mouse must be within 11 pixels of the edge of the target
//    2. The mouse must remain within 11 pixels for 50 millisecs ("delay")
//      before any scrolling takes place
//    3. The rate of scrolling ("interval") must be no more than once
//      every 50 millisecs.
//  These values are pulled from WIN.INI when the Bolero DLL is loaded.
//
// Bolero clients will receive an IContainer::Scroll message whenever
// it's appropriate to scroll. The BOleScroll bitfield may specify
// two-directional scrolling (e.g. up and to the left). The client may
// scroll or not, and should return TRUE if it does in order to tell
// the server to change the cursor shape to the scrolling version.
//
BOleScroll _IFUNC BOleDocument::DragScrollReqd (POINTL *where)
{
  // Check to see if the point is in any of our cached scrolling rects
  //
  BOleScroll ret = (BOleScroll) 0;
  POINT shortWhere = {where->x, where->y};
  HWND w = pContainer->GetWindow();
  WIN::ScreenToClient (w, &shortWhere);
  if (::PtInRect (&rcScrollLeft, shortWhere))
    ret = ret | BOLE_SCROLL_RIGHT;
  if (::PtInRect (&rcScrollRight, shortWhere))
    ret = ret | BOLE_SCROLL_LEFT;
  if (::PtInRect (&rcScrollTop, shortWhere))
    ret = ret| BOLE_SCROLL_DOWN;
  if (::PtInRect (&rcScrollBottom, shortWhere))
    ret = ret | BOLE_SCROLL_UP;

  // If not in any scroll rects, reset the times and leave
  //
  if (!ret) {
    scrollEnterTime = 0;
    scrollLastTime = 0;
    return (BOleScroll) ret;
  }

  DWORD now = WIN::GetCurrentTime ();

  // If this is the first time we've noticed the mouse in the scroll
  // area, make a note of the time and start waiting for the
    // initial delay latency
  //
  if (!scrollEnterTime) {
    scrollEnterTime = now;
    fScrollDelay = TRUE;
    return (BOleScroll) 0;
  }

  // If we're waiting for the initial delay to pass, check 'now' against
  // the initial delay latency instead of the interval latency. These
  // are probably the same, but we're good citizens. If the delay has
  // expired, we can return the scrolling direction to indicate the
  // beginning of a scroll operation.
  //
  if (fScrollDelay)
    if (now > scrollEnterTime + dragScrollDelay) {
      fScrollDelay = FALSE;
      scrollLastTime = now;
      return (BOleScroll) ret;
    }
    else
      return (BOleScroll) 0;

  // If we've gotten this far, we must be in the middle of a scrolling
  // operation. Check 'now' against the interval latency and return
  // the scrolling direction, or 0 if the latency has not expired.
  //
  if (now > scrollLastTime + dragScrollInterval) {
    scrollLastTime = now;
    return (BOleScroll) ret;
  }
  else
    return (BOleScroll) 0;
}

// CalculateDndObjPos is a helper function which sets up a rectangle which
// represents the position of the object being dragged RELATIVE TO ITS
// ORIGINAL HIT POINT.
//
// I know I'm not passing any parameters, but this stuff is all in member
// data anyway, so it seems silly to pass it on the stack too.
//
void _IFUNC BOleDocument::CalculateDndObjPos ()
{
  ::SetRect (&dndObjPos,
    dndLastPos.x - dndOffset.x,
    dndLastPos.y - dndOffset.y,
    dndLastPos.x - dndOffset.x + dndSize.cx,
    dndLastPos.y - dndOffset.y + dndSize.cy);
}

HRESULT _IFUNC BOleDocument::DragEnter (IDataObject *dataObj,
                                DWORD keyState, POINTL where, LPDWORD effect)
{
  POINT shortPoint;
  shortPoint.x = LOWORD(where.x);
  shortPoint.y = LOWORD(where.y);

  HRESULT hr = pContainer->FindDropDest (&shortPoint, &pDropDest);
  if (!SUCCEEDED(hr)) {
    *effect = DROPEFFECT_NONE;
    return ResultFromScode (E_FAIL);
  }

  // cache this for use in ValidateDrag during drag-over
  fCantLink = S_OK != GetScode(OleQueryLinkFromData (dataObj));

  // same dropdest/document pair
  //
  RECT r;
  if (SUCCEEDED(pDropDest->GetScrollRect (&r)))
    CacheDragRects (&r);

  // Find out how many formats the client can accept. If they can't
  // accept any, return an error so no drop can take place
  //
  dndFormatCount = pDropDest->CountFormats ();
  if (!dndFormatCount) {
    *effect = DROPEFFECT_NONE;
    return ResultFromScode (E_FAIL);
  }

  // Hold the container's legal data formats in an array
  //
  dndFormatList = new OLEUIPASTEENTRY [dndFormatCount];
  BOleFormat f;
  for (short i = 0; i < dndFormatCount; i++) {
    pDropDest->GetFormat (i, &f);

    dndFormatList[i].fmtetc.cfFormat = f.fmtId;
    dndFormatList[i].fmtetc.ptd = NULL;
    dndFormatList[i].fmtetc.dwAspect = DVASPECT_CONTENT;
    dndFormatList[i].fmtetc.tymed = f.fmtMedium & ~BOLE_MED_STATIC;
    dndFormatList[i].fmtetc.lindex = -1;

    dndFormatList[i].lpstrFormatName = TEXT ("%s");  // unused for drag&drop
//      (f.fmtName[0] ? f.fmtName : OLESTR ("%s"));
    dndFormatList[i].lpstrResultText = TEXT ("%s"); // unused for drag&drop
//      (f.fmtResultName[0] ? f.fmtResultName : OLESTR ("%s"));

    if (f.fmtId == BOleDocument::oleEmbdObjClipFmt ||
      f.fmtId == BOleDocument::oleLinkSrcClipFmt ||
      f.fmtId == BOleDocument::oleEmbSrcClipFmt)

      //  PASTEONLY and ENABLEICON are mutually exclusive
      //
      dndFormatList[i].dwFlags = OLEUIPASTE_PASTE | OLEUIPASTE_ENABLEICON;
    else
      dndFormatList[i].dwFlags = OLEUIPASTE_PASTEONLY;

  }

  dndFormat = -1; // prime with "no agreed format"


  // Ask the data object for its object descriptor
  //
  FORMATETC fmtetc;
  STGMEDIUM tymed;
  fmtetc.cfFormat = BOleDocument::oleObjectDescFmt;
  fmtetc.ptd = 0;
  fmtetc.dwAspect = DVASPECT_CONTENT;
  fmtetc.lindex = -1;
  fmtetc.tymed = TYMED_HGLOBAL;

  hr = dataObj->GetData (&fmtetc, &tymed);

  if (SUCCEEDED(hr)) {

    // Keep this around since not all servers give out object
    // descriptor data, and we wouldn't want to send a bogus
    // size back to the client
    //
    dndInfoAvailable = TRUE;

    //  Ask the object descriptor how much screen size it needs
    //
    OBJECTDESCRIPTOR *pOD = (OBJECTDESCRIPTOR*) ::GlobalLock(tymed.hGlobal);

    // MSGraph5 fix: object descriptor is garbage
    //
    if (!IsBadReadPtr (pOD, sizeof (OBJECTDESCRIPTOR))) {

      // These coordinate transformations produce a rectangle, in
      // client pixel coordinates, which represents the position of
      // the dragged object RELATIVE TO THE HIT POSITION in the
      // original server object.
      //
      dndSize.cx = MAP_LOGHIM_TO_PIX (pOD->sizel.cx, BOleService::pixPerIn.x);
      dndSize.cy = MAP_LOGHIM_TO_PIX (pOD->sizel.cy, BOleService::pixPerIn.y);

      dndOffset.x = MAP_LOGHIM_TO_PIX (pOD->pointl.x, BOleService::pixPerIn.x);
      dndOffset.y = MAP_LOGHIM_TO_PIX (pOD->pointl.y, BOleService::pixPerIn.y);
    }
    else
      SetRect (&dndObjPos, -1, -1, -1, -1);

    WIN::GlobalUnlock (tymed.hGlobal);
    OLE::ReleaseStgMedium (&tymed);
    pOD = NULL;
    }
  else {
    SetRect (&dndObjPos, -1, -1, -1, -1);
  }

  // Make sure the action implied by 'keyState' is supported
  //
  if (ValidateDrag(keyState, &where, effect, dataObj) >= 0) {

    // Find out if the user has dragged over the scroll zone
    //
    BOleScroll scrollDirection = (BOleScroll) 0;
    scrollEnterTime = 0;
    scrollLastTime = 0;
    scrollDirection = DragScrollReqd(&where);
    if (scrollDirection) {
      if (GetScode (pDropDest->Scroll (scrollDirection)) == S_OK)

        // If the app did scroll, tell the server so it can
        // make the cursor into the drag-scroll bitmap
        //
        *effect |= DROPEFFECT_SCROLL;

    }

    // Get the drag position in container window's client coords
    //
    dndLastPos.x = where.x;
    dndLastPos.y = where.y;
    HWND w = pContainer->GetWindow();
    ::ScreenToClient (w, &dndLastPos);

    // Ask the container to give appropriate feedback. This first time
    // through draws the feedback so we're ready for the erase-redraw
    // sequence in DragOver
    //
    CalculateDndObjPos ();

    HRESULT hr = NOERROR;
    pDropDest->DragFeedback (&dndLastPos, &dndObjPos, BOLE_MOUSEENTER, dndFormat, hr);
    fDrawn = TRUE;
    if (hr != NOERROR) {
      dndLastFeedbackFailed = TRUE;
      *effect = DROPEFFECT_NONE;
      return hr;
    }
    else
      dndLastFeedbackFailed = FALSE;

    // We can return 'ok' as long as ValidateDrag succeeded.
    // It isn't important whether the IContainer accepted Scroll
    //
    OLERET(S_OK);
  }
  OLERET(E_FAIL);
}

HRESULT _IFUNC BOleDocument::DragOver (DWORD keyState, POINTL where, LPDWORD effect)
{
//  *effect = NULL;
//  return ResultFromScode (E_FAIL);

  // Make sure the action implied by 'keyState' is supported
  //
  BOOL fValid = ValidateDrag(keyState, &where, effect) >= 0;

  if (fValid) {
    // Find out if the user has dragged over the scroll zone
    //
    BOleScroll scrollDirection = (BOleScroll) 0;
    scrollDirection = DragScrollReqd(&where);
    if (scrollDirection) {
      if (GetScode (pDropDest->Scroll (scrollDirection)) == S_OK)

        // If the app did scroll, tell the server so it can
        // make the cursor into the drag-scroll bitmap
        //
        *effect |= DROPEFFECT_SCROLL;

    }
  }

  // Do drop-target feedback
  //

  HWND w = pContainer->GetWindow();
  POINT newPos;
  newPos.x = where.x;
  newPos.y = where.y;
  ::ScreenToClient (w, &newPos);

  if ((dndLastPos.x != newPos.x) || (dndLastPos.y != newPos.y)) {
    if (fDrawn) {
      // Once to erase in the old position
      //
      if (dndInfoAvailable)
        CalculateDndObjPos ();
      else
        SetRect (&dndObjPos, -1, -1, -1, -1);

      HRESULT hr = NOERROR;
      pDropDest->DragFeedback (&dndLastPos, &dndObjPos, BOLE_MOUSECONTAINED, dndFormat, hr);
      fDrawn = FALSE;
      if (hr != NOERROR) {
        dndLastFeedbackFailed = TRUE;
        *effect = DROPEFFECT_NONE;
        return hr;
      }
      else
        dndLastFeedbackFailed = FALSE;

    }

    if (fValid) { // Once to draw in the new position
      //
      dndLastPos = newPos;
      if (dndInfoAvailable)
        CalculateDndObjPos ();
      else
        SetRect (&dndObjPos, -1, -1, -1, -1);

      HRESULT hr = NOERROR;
      pDropDest->DragFeedback (&dndLastPos, &dndObjPos, BOLE_MOUSECONTAINED, dndFormat, hr);
      fDrawn = TRUE;
      if (hr != NOERROR) {
        dndLastFeedbackFailed = TRUE;
        *effect = DROPEFFECT_NONE;
        return hr;
      }
      else
        dndLastFeedbackFailed = FALSE;
    }
  }
  else {
    if (dndLastFeedbackFailed) {
      *effect = DROPEFFECT_NONE;
      return ResultFromScode (E_FAIL);
    }
  }
  if (fValid) {
    // We can return 'ok' as long as ValidateDrag succeeded.
    // It isn't important whether the IContainer accepted Scroll
    //
    OLERET (S_OK);
  }
  else {
    *effect = DROPEFFECT_NONE;
    OLERET (E_FAIL);
  }

}

HRESULT _IFUNC BOleDocument::DragLeave ()
{
  HRESULT hr = NOERROR;

  // Erase feedback at last position
  //
  if (!dndInfoAvailable)
    SetRect (&dndObjPos, -1, -1, -1, -1);

  if (fDrawn) {
    pDropDest->DragFeedback (&dndLastPos, &dndObjPos, BOLE_MOUSEEXIT, dndFormat, hr);
    fDrawn = FALSE;
    // dndLastFeedbackFailed shouldn't matter here
  }

  // This info is invalid after the DnD transaction
  //
  dndSize.cx = dndSize.cy = 0;
  dndLastPos.x = dndLastPos.y = -1;
  dndInfoAvailable = FALSE;
  if (dndFormatList)
    delete [] dndFormatList;
  dndFormatList = NULL;

  return hr;
}

HRESULT _IFUNC BOleDocument::Drop (IDataObject *dataObj, DWORD keyState,
                                   POINTL where, LPDWORD effect)
{
  short agreedFormat = ValidateDrag(keyState, &where, effect, dataObj);

  if (agreedFormat >= 0) {

    // Find out if the format we agreed on can support static objects.
    // This would be a little more convenient if dndFormatList contained
    // BOleFormats rather than FORMATETCs, but this works out.
    //
    BOOL fStaticOK = FALSE;
    BOleFormat f;
    if (SUCCEEDED(pDropDest->GetFormat(agreedFormat, &f)))
      fStaticOK = f.fmtMedium & BOLE_MED_STATIC;

    //  Fill in BOleInitInfo fields dependent on the format
    //
     BOleInitInfo bii;
    bii.pContainer = NULL;
    UINT cf = dndFormatList[agreedFormat].fmtetc.cfFormat;
    if (cf == oleEmbdObjClipFmt || cf == oleEmbSrcClipFmt) {
      bii.Where = BOLE_DATAOBJECT;
      bii.pStorage = NULLP;
      bii.whereData.pData = dataObj;
    }
    else if (cf == oleLinkSrcClipFmt) {
      bii.Where = BOLE_DATAOBJECT;
      bii.pStorage = NULLP;
      bii.whereData.pData = dataObj;
    }
    else if (fStaticOK) {
      bii.How = (cf == CF_METAFILEPICT) ?
        BOLE_EMBED_ASMETAFILE : BOLE_EMBED_ASBITMAP;
      bii.Where = BOLE_DATAOBJECT;
      bii.whereData.pData = dataObj;
    }
    else {
      STGMEDIUM medium;
      bii.Where = BOLE_HANDLE;
      bii.whereHandle.dataFormat = cf;
      bii.whereHandle.data = pService->GetDataFromDataObject (
        dataObj, cf, NULL, DVASPECT_CONTENT, &medium);

      bii.pStorage = NULL;
    }

    // Fields the same for all dropped objects
    //
    bii.hIcon = NULL;
    if (*effect & DROPEFFECT_COPY || *effect & DROPEFFECT_MOVE)
      bii.How = BOLE_EMBED;
    else
      if (*effect & DROPEFFECT_LINK)
        bii.How = BOLE_LINK;

    // Erase feedback at last position
    //
    if (dndInfoAvailable)
      CalculateDndObjPos ();
    else
      SetRect (&dndObjPos, -1, -1, -1, -1);

    HRESULT hr = NOERROR;
    if (fDrawn) {
      pDropDest->DragFeedback (&dndLastPos, &dndObjPos, BOLE_MOUSECONTAINED, dndFormat, hr);
      fDrawn = FALSE;
      if (hr != NOERROR) {
        // dndLastFeedbackFailed shouldn't matter here
        *effect = DROPEFFECT_NONE;
      }
    }

    if (hr == NOERROR)
      hr = pDropDest->Drop (&bii, &dndLastPos, &dndObjPos);

    // This info is invalid after the DnD transaction
    //
    dndInfoAvailable = FALSE;
    dndSize.cx = dndSize.cy = 0;
    dndLastPos.x = dndLastPos.y = -1;
    if (dndFormatList)
      delete [] dndFormatList;
    dndFormatList = NULL;

    SCODE sc = GetScode (hr);
    if (sc == S_OK || sc == DRAGDROP_S_DROP)
      pContainer->BringToFront();
    else
      *effect = DROPEFFECT_NONE;

    return hr;
  }

  *effect = DROPEFFECT_NONE;
  OLERET (E_FAIL);
}

/**************************************************************************

  IOleUILinkContainer implementation

  This interface isn't part of OLE2 proper -- it's part of OLE2UI, and
  is implemented here to allow clients to use the default Links dialog
  box without building their own. If all the clients do build their own,
  we could take this implementation out and also remove the BrowseLinks
  member function of IDocument.

  The DWORD dwLink parameter to these functions seems to be a magic
  cookie that the OLE2UI Links dialog passes around to refer to an
  individual link object. We use a BOlePart* and downcast as necessary.

  The role of these functions is to translate the OLE2UI IOleUILinkContainer
  calls into Bolero ILinkInfo calls and delegate them to BOlePart.

*/

DWORD _IFUNC BOleDocument::GetNextLink (DWORD dwLink)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  if (!pPart)

    // Start looking at the beginning, rather than relative to another part
    //
    if (pFirstPart)

      // Find the next linked part *including* pFirstPart
      //
      return (DWORD) pFirstPart->GetNextLink (TRUE);
    else

      // We don't have any parts, linked or otherwise
      //
      return 0;

  // Find the next linked part *after* pPart
  //
   return (DWORD) pPart->GetNextLink (FALSE);
}

HRESULT _IFUNC BOleDocument::SetLinkUpdateOptions
(
  DWORD dwLink,
  DWORD dwUpdateOpt
)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  return pPart->UpdateSet ((BOleLinkUpdate)dwUpdateOpt);
}

HRESULT _IFUNC BOleDocument::GetLinkUpdateOptions
(
  DWORD dwLink,
  DWORD FAR* lpdwUpdateOpt
)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  BOleLinkUpdate blu;
  HRESULT hr = pPart->UpdateGet (&blu);
  *lpdwUpdateOpt = blu;
  return hr;
}

HRESULT _IFUNC BOleDocument::SetLinkSource
(
  DWORD       dwLink,
  TCHAR      *lpszDisplayName,
  ULONG       lenFileName,
  ULONG FAR*  pchEaten,
  BOOL        fValidateSource
)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  HRESULT   hr = ResultFromScode(E_FAIL);

#if !defined(UNICODE)   // MS_OLEUI_DEF UNICODE conversion
  OLECHAR *pName = new OLECHAR [strlen (lpszDisplayName) + 1];
  if (pName) {
    MultiByteToWideChar (CP_ACP, 0,
                      lpszDisplayName, strlen (lpszDisplayName) + 1,
                      pName, strlen (lpszDisplayName) + 1);
    hr = pPart->SourceSet (pName);
    delete [] pName;
  }
#else
  hr = pPart->SourceSet (lpszDisplayName);
#endif
  return hr;
}

HRESULT _IFUNC BOleDocument::GetLinkSource
(
  DWORD       dwLink,
  TCHAR   FAR*  FAR* lplpszDisplayName,
  ULONG   FAR*  lplenFileName,
  TCHAR   FAR*  FAR* lplpszFullLinkType,
  TCHAR   FAR*  FAR* lplpszShortLinkType,
  BOOL FAR*   lpfSourceAvailable,
  BOOL FAR*   lpfIsSelected
)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  HRESULT hr;

  *lplpszDisplayName = NULL;
  *lplpszFullLinkType = NULL;
  *lplpszShortLinkType = NULL;
  *lplenFileName = NULL;
  *lpfSourceAvailable = pPart->GetLinkAvailability();
  if (lpfIsSelected)
    *lpfIsSelected = pPart == pFirstPart ? TRUE : FALSE;

#if !defined(UNICODE)   // MS_OLEUI_DEF UNICODE conversion
  LPOLESTR pDisplayName;
  LPOLESTR pFullLinkType;
  LPOLESTR pShortLinkType;
  LPMALLOC pMalloc = NULL;
  hr = pPart->SourceGet (&pDisplayName, &pFullLinkType,
        &pShortLinkType, lplenFileName);
  if (SUCCEEDED (hr)) {
    IMalloc *pMalloc;
    if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pMalloc))) {

      *lplpszDisplayName   = (PTCHAR) pMalloc->Alloc (lstrlen (pDisplayName) + 1);
      *lplpszFullLinkType  = (PTCHAR) pMalloc->Alloc (lstrlen (pFullLinkType) + 1);
      *lplpszShortLinkType = (PTCHAR) pMalloc->Alloc (lstrlen (pShortLinkType) + 1);

      WideCharToMultiByte (CP_ACP, 0, pDisplayName, lstrlen (pDisplayName) + 1,
                      *lplpszDisplayName, lstrlen (pDisplayName) + 1,
                      0, 0);
      WideCharToMultiByte (CP_ACP, 0, pFullLinkType, lstrlen (pFullLinkType) + 1,
                      *lplpszFullLinkType, lstrlen (pFullLinkType) + 1,
                      0, 0);
      WideCharToMultiByte (CP_ACP, 0, pShortLinkType, lstrlen (pShortLinkType) + 1,
                      *lplpszShortLinkType, lstrlen (pShortLinkType) + 1,
                      0, 0);

      pMalloc->Free (pDisplayName);
      pMalloc->Free (pFullLinkType);
      pMalloc->Free (pShortLinkType);
      pMalloc->Release ();
    }
  }
#else
  hr = pPart->SourceGet (lplpszDisplayName, lplpszFullLinkType,
        lplpszShortLinkType, lplenFileName);
#endif
  return hr;
}

HRESULT _IFUNC BOleDocument::OpenLinkSource (DWORD dwLink)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  return pPart->Activate(TRUE);
}

HRESULT _IFUNC BOleDocument::UpdateLink
(
  DWORD dwLink,
  BOOL fErrorMessage,
  BOOL fErrorAction
)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  return pPart->UpdateNow ();
}

HRESULT _IFUNC BOleDocument::CancelLink (DWORD dwLink)
{
  BOlePart *pPart = (BOlePart*) dwLink;
  return pPart->SourceBreak();
}

//**************************************************************************
//
// Support functions for other Bolero objects
//
//**************************************************************************

// This OnModalDialog is not part of any Bolero interface.
// It's called by BOleService because the service doesn't know
// who the active object is, so the service just routes the call
// to the active doc to let us figure it out
//
HRESULT BOleDocument::OnModalDialog (BOOL fDialogComingActive)
{
  if (pActivePart)
    return pActivePart->EnableModeless (!fDialogComingActive);
  return NOERROR;
}

// This TranslateAccel is not part of any Bolero interface.
// It's called by BOleService because the service doesn't know
// who the active object is, so the service just routes the call
// to the active doc to let us figure it out
//
HRESULT BOleDocument::TranslateAccel (LPMSG pMsg)
{
  HRESULT hRes = ResultFromScode( S_FALSE );
  if (pActiveCtrl && pActivePart)
    hRes = pActivePart->TranslateAccelerator(pMsg);
  else {
    if (pActivePart)
      hRes = pActivePart->TranslateAccelerator(pMsg);
    if (hRes != S_OK)
      hRes = TranslateControlAccel (pMsg);
  }
  return hRes;
}

// This TranslateCtrlAccel spins through all controls to look for
// control mnemonics that need to be serviced
//
HRESULT BOleDocument::TranslateControlAccel (LPMSG pMsg)
{
  HRESULT hRes = ResultFromScode( S_FALSE );
  BOleControl *pWalk = pFirstCtrl;
  CONTROLINFO  info;
  while (pWalk) {
    pWalk->GetControlInfo (&info);
    if ((info.hAccel)  && ::IsAccelerator (info.hAccel, info.cAccel,
                                          pMsg, NULL)) {
      pWalk->GetOleControl()->OnMnemonic(pMsg);
      hRes = ResultFromScode(S_OK);
      break;
    }
    pWalk = pWalk->GetNextControl();
  }
  return hRes;
}

// CacheDragRects is called to precompute the 11-pixel zones
// around the perimeter of the drop target. Normally, this
// wouldn't be a big deal to compute on the fly, but the traffic
// already generated during a drag-and-drop is so time consuming
// it seems worthwhile to give up a few bytes of instance data
//
void _IFUNC BOleDocument::CacheDragRects (LPRECT pR)
{
  RECT r = *pR;
  WIN::SetRect (&rcScrollTop, r.left, r.top, r.right, r.top + 11);
  WIN::SetRect (&rcScrollTop, r.left, r.top, r.right, r.top + 11);
   WIN::SetRect (&rcScrollLeft, r.left, r.top, r.left + 11, r.bottom);
  WIN::SetRect (&rcScrollRight, r.right - 11, r.top, r.right, r.bottom);
  WIN::SetRect (&rcScrollBottom, r.left, r.bottom - 11, r.right, r.bottom);
}

// Accessor function used by BOlePart::RemoveFromList to make sure
// the head of the list (maintained by BOleDocument) is up-to-date
//
void BOleDocument::OnRemovePart (BOlePart *pPart)
{
  if (pFirstPart == pPart)
    pFirstPart = pFirstPart->GetNextPart();
}


//*************************************************************************
//
// IOleInPlaceFrame implementation
//
//*************************************************************************

HRESULT _IFUNC BOleDocument::InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
  OLERET (SUCCEEDED(pContainer->InsertContainerMenus (hmenuShared, (BOleMenuWidths*) lpMenuWidths)) ?
      S_OK : E_FAIL);
}

HRESULT _IFUNC BOleDocument::SetMenu (HMENU hmenuShared, HOLEMENU holeMenu, HWND hwndActiveObject)
{
  HRESULT hr = NOERROR;

  // hmenuShared will be null when the call to BOleDocument::SetMenu is
  // initiated from BOlePart::OnUIDeactivate. hmenuShared will be valid
  // when the SetMenu call comes from the real server object
  //
  if (hmenuShared)
    hr = pContainer->SetFrameMenu (hmenuShared);

  // holemenu will be null when the call to BOleDocument::SetMenu is
  // initiated from BOlePart::OnUIDeactivate. holeMenu will be valid
  // when the SetMenu call comes from the real server object
  //
  if (SUCCEEDED(hr))
    hr = OleSetMenuDescriptor (holeMenu, pContainer->GetWindow(),
      hwndActiveObject, NULL, NULL);

  return hr;
}

HRESULT _IFUNC BOleDocument::RemoveMenus (HMENU hmenuShared)
{
  BOOL fNoError = TRUE;

  // Remove container group menus
  while (GetMenuItemCount(hmenuShared))
    fNoError &= RemoveMenu(hmenuShared, 0, MF_BYPOSITION);

  return fNoError ? NOERROR : ResultFromScode(E_FAIL);
}

HRESULT _IFUNC BOleDocument::SetStatusText (LPCOLESTR statusText)
{
  // Servers use this function to put text in the container's status bar.
  // The server is not supposed to negotiate tool space to put their own
  // status bar at the bottom of the frame window.
  //
  pContainer->SetStatusText(statusText);
  OLERET(S_OK);
}

HRESULT _IFUNC BOleDocument::EnableModeless (BOOL fEnable)
{
  return pApplication->OnModalDialog (!fEnable);
}

HRESULT _IFUNC BOleDocument::TranslateAccelerator (MSG FAR* msg, WORD wID)
{
  HWND oldhwnd = msg->hwnd;
  msg->hwnd = pContainer->GetWindow ();
  HRESULT hr = pContainer->Accelerator(msg);
  msg->hwnd = oldhwnd;
  return hr;
}


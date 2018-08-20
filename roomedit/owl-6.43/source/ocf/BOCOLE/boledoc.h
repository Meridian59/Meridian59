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
#ifndef _BOLEDOC_H
#define _BOLEDOC_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif

#ifndef _BOLEPART_H
#include "bolepart.h"
#endif

class _ICLASS BOleService;
class _ICLASS BOleControl;


class _ICLASS BOleDocument : public BOleComponent, public IBDocument,
  public IDropTarget, public IOleUILinkContainer,  public IOleInPlaceFrame,
  public IBOverrideBrowseLinks
{
public:
  static CLIPFORMAT oleEmbdObjClipFmt;
  static CLIPFORMAT oleEmbSrcClipFmt;
  static CLIPFORMAT oleLinkSrcClipFmt;

  static CLIPFORMAT oleLinkClipFmt;
  static CLIPFORMAT oleNativeClipFmt;
  static CLIPFORMAT oleOwnerLinkClipFmt;
  static CLIPFORMAT  oleObjectDescFmt;
  static CLIPFORMAT  oleLinkSrcDescFmt;

  static UINT dragScrollInset;
  static UINT dragScrollDelay;
  static UINT dragScrollInterval;

protected:

  friend BOlePart;
  friend BOleService;

  IBContainer    * pContainer;
  IBApplication  * pApplication;
  BOleService    * pService;

  IOleInPlaceActiveObject
                 * pActivePart;  // For keeping object's active state in sync with window's
  IOleControl    * pActiveCtrl;

  BOlePart       * pFirstPart;  // For enumerating objects
  BOleControl    * pFirstCtrl;  // For enumerating objects

  // For drawing XOR'ed outline rect during drag/drop
  //
  RECT     dndObjPos;
  SIZE     dndSize;
  POINT    dndLastPos;
  BOOL     dndInfoAvailable;
  POINT    dndOffset;
  PIBDropDest pDropDest;
  BOOL fDrawn;

  BOOL dndLastFeedbackFailed; // so we know what to do when mouse doesn't move

  // For scrolling during drag/drop
  //
  RECT  rcScrollTop, rcScrollLeft, rcScrollRight, rcScrollBottom;
  DWORD scrollEnterTime;
  DWORD scrollLastTime;
  BOOL  fScrollDelay;

  // For format negotiation during drag/drop
  //
  OLEUIPASTEENTRY *dndFormatList;
  UINT dndFormatCount;
  UINT dndFormat;
  BOOL fCantLink;

  // For registering drag/drop
  BOOL fRegistered;

  // To prevent BrowseLinks from being re-entrant
  BOOL fInBrowseLinks;

  // Private implementation functions
  //
  short _IFUNC ValidateDrag(DWORD keyState, POINTL *where, DWORD *effect, IDataObject *dataObj = NULL);
  BOleScroll _IFUNC DragScrollReqd (POINTL *where);
  void _IFUNC CacheDragRects (LPRECT);
  void _IFUNC CalculateDndObjPos ();

public:
  BOleDocument (BOleClassManager *, IBUnknownMain *, BOleService *);
  ~BOleDocument ();

  IBContainer *GetContainer () { return pContainer; }

  BOlePart    *GetFirstPart ()    { return pFirstPart; }
  BOleControl *GetFirstControl () { return pFirstCtrl; }
  void         SetFirstPart    (BOlePart*pPart) { pFirstPart = pPart; }
  void         SetFirstControl (BOleControl*pCtrl) { pFirstCtrl = pCtrl; }
  void         OnRemovePart (BOlePart *pPart);
  BOOL _IFUNC  EnableBrowseLinks();

  HRESULT      OnModalDialog (BOOL);
  HRESULT      TranslateAccel (LPMSG);
  HRESULT      TranslateControlAccel (LPMSG pMsg);

  // IDocument functions
  //
  virtual HRESULT _IFUNC Init(PIBContainer);
  virtual void    _IFUNC OnResize ();
  virtual void    _IFUNC OnActivate (BOOL fActivate);
  virtual void    _IFUNC OnClose ();
  virtual HRESULT _IFUNC OnSetFocus(BOOL);
  virtual HRESULT _IFUNC EnumLinks (PIBLinkInfo FAR*);
  virtual HRESULT _IFUNC BrowseLinks ();
  virtual HRESULT _IFUNC UpdateLinks ();

public:

  // IUnknown Methods
  //                delegate aggregator's IUnknownMain
  DEFINE_IUNKNOWN(pObjOuter)

  // IUnknownMain methods
  //
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

private:

  // IOleWindow methods, base of IOleInPlaceUIWindow and IOleInPlaceSite
  //
  HRESULT _IFUNC GetWindow(HWND FAR* lphwnd);
  HRESULT _IFUNC ContextSensitiveHelp(BOOL fEnterMode);

  // IOleInPlaceUIWindow methods
  //
  HRESULT _IFUNC GetBorder(LPRECT lprectBorder);
  HRESULT _IFUNC RequestBorderSpace(LPCRECT lprectWidths);
  HRESULT _IFUNC SetBorderSpace(LPCBORDERWIDTHS lpborderwidths);
  HRESULT _IFUNC SetActiveObject(IOleInPlaceActiveObject* lpActiveObject,
                      LPCOLESTR lpszObjName);

  // IDropTarget functions
  //
  HRESULT _IFUNC DragEnter(IDataObject *dataObj, DWORD keyState, POINTL where, LPDWORD effect);
  HRESULT _IFUNC DragOver(DWORD keyState, POINTL where, LPDWORD effect);
  HRESULT _IFUNC DragLeave();
  HRESULT _IFUNC Drop(IDataObject *dataObj, DWORD keyState, POINTL where, LPDWORD effect);

  // IOleUILinkContainer functions
  //
  DWORD   _IFUNC GetNextLink (DWORD dwLink);
  HRESULT _IFUNC SetLinkUpdateOptions (DWORD dwLink, DWORD dwUpdateOpt);
  HRESULT _IFUNC GetLinkUpdateOptions (DWORD dwLink, DWORD FAR* lpdwUpdateOpt);
  HRESULT _IFUNC SetLinkSource (DWORD dwLink,
                    TCHAR      *lpszDisplayName,
                    ULONG       lenFileName,
                    ULONG FAR*  pchEaten,
                    BOOL        fValidateSource);
  HRESULT _IFUNC GetLinkSource (DWORD dwLink,
                    TCHAR    FAR* FAR*  lplpszDisplayName,
                    ULONG    FAR*       lplenFileName,
                    TCHAR    FAR* FAR*  lplpszFullLinkType,
                    TCHAR    FAR* FAR*  lplpszShortLinkType,
                    BOOL     FAR*       lpfSourceAvailable,
                    BOOL     FAR*       lpfIsSelected);
  HRESULT _IFUNC OpenLinkSource (DWORD dwLink);
  HRESULT _IFUNC UpdateLink(DWORD dwLink, BOOL fErrorMessage,BOOL fErrorAction);
  HRESULT _IFUNC CancelLink (DWORD dwLink);

  // IOleInPlaceFrame methods
  //
  HRESULT _IFUNC InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidts);
  HRESULT _IFUNC SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
  HRESULT _IFUNC RemoveMenus(HMENU hmenuShared);
  HRESULT _IFUNC SetStatusText(LPCOLESTR lpszStatusText);
  HRESULT _IFUNC EnableModeless(BOOL fEnable);
  HRESULT _IFUNC TranslateAccelerator(LPMSG lpmsg, WORD wID);
};

#endif _BOLEDOC_H


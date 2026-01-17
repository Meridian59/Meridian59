//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 client site. BOlePart objects
//  impersonate the server object (IPart) from the point of view of the
//  Bolero customer who's writing a client site (ISite)
//----------------------------------------------------------------------------
#ifndef _BOLEPART_H
#define _BOLEPART_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif


extern "C"{
#include "ole2ui.h"  // includes oledlg.h
}

class _ICLASS BOleDocument;

class _ICLASS BOlePart : public BOleComponent,   public IBPart2,
                         public IOleClientSite,  public IAdviseSink,
                         public IOleInPlaceSite, public IBLinkInfo,
                         public IBLinkable,      public IDropSource
{
protected:

  IBSite            * pSite;
  IBContainer       * pContainer;
  IBLinkable        * pILinkCont;
  BOleDocument      * pDoc;
  IViewObject       * pViewObject;
  IEnumFORMATETC    * pEnumFE;
  IOleInPlaceObject * pInPlaceObject;
  IEnumOLEVERB      * pVerbEnumerator;
  IOleLink          * pLinkObject;
  IStorage          * pStgFromInit;

  BOOL IsOpen          : 1;
  BOOL IsOpenInPlace   : 1;
  BOOL IsOpenInsideOut : 1;
  BOOL HasMoniker      : 1;
  BOOL IsLink          : 1;
  BOOL MonikerAssigned : 1;
  BOOL fLinkSrcAvail   : 1;

  LPOLESTR        pShortName;
  LPOLESTR        pLongName;
  LPOLESTR        pAppName;
  WORD            verbIndex;
  OLEVERB         oleVerb;
  DWORD           dwDrawAspect, dwExtAspect;
  HGLOBAL         mfpIcon;
  BOlePart      * pNextPart, *pPrevPart;
  LPOLESTR        pszInstName;
  BOleScaleFactor scaleSite;
  SIZEL           CachedExtent;

public:

  IOleObject   * pOleObject;
  IDataObject  * pDataObject;
  LPUNKNOWN      pDocument;
private:
  void AddToList ();
  void RemoveFromList ();

public:
  BOlePart (BOleClassManager *pF, IBUnknownMain * pO, BOleDocument * pOD);
  ~BOlePart ();

  virtual HRESULT   _IFUNC GetParentIPS(LPOLEINPLACESITE *pIPSite);
  virtual IMoniker *_IFUNC GetObjectMoniker (DWORD dwAssign);
  virtual BOOL      _IFUNC GetLinkAvailability() {return fLinkSrcAvail;}
  virtual BOlePart *_IFUNC GetNextLink (BOOL);
  virtual HRESULT   _IFUNC SourceGet(LPOLESTR FAR *,LPOLESTR FAR *,
                                      LPOLESTR FAR *,DWORD FAR *);
  virtual BOOL      _IFUNC CacheIconicAspect (HGLOBAL);
  virtual BOOL      _IFUNC InitAdvises(LPOLEOBJECT, DWORD, LPOLESTR, LPOLESTR,
                                   LPADVISESINK, BOOL);
  HRESULT CreateFromBOleInitInfo(BOleInitInfo *pBI, BOOL &fShowAfterCreate);
  void    UpdateSiteScaleFactor();
  void    DeleteTypeNames();
  void    SiteShow(BOOL show);

  virtual HRESULT   _IFUNC CopyFromOriginal (LPFORMATETC, LPDATAOBJECT FAR*);
  virtual HRESULT   _IFUNC AddCachedData (IDataObject *pTarget);
  virtual HRESULT   _IFUNC AddLinkSourceData (IDataObject *pTarget);
  virtual HRESULT   _IFUNC AddSiteData (IDataObject *pTarget);

  virtual HRESULT   _IFUNC ConvertUI (PIBApplication, BOOL,
                                                      BOleConvertInfo FAR*);
  virtual HRESULT   _IFUNC ConvertGuts (BOOL, BOleConvertInfo FAR*);
  virtual HRESULT   _IFUNC ConvertHelper (LPCLSID, LPOLESTR FAR *, WORD FAR*);

  virtual HRESULT   _IFUNC BOleCreate (BOleInitInfo *pBI, DWORD dwRenderOpt,
                                                      LPVOID FAR *ppv);
  virtual BOOL      _IFUNC IsInPlace () {return IsOpenInPlace;}
  virtual void      _IFUNC ResetObjectRects ();
  virtual BOlePart* _IFUNC GetNextPart () {return pNextPart;}

  HRESULT _IFUNC SwitchDisplayAspect (LPOLEOBJECT lpOleObj,
                              LPDWORD lpdwCurAspect, DWORD dwNewAspect,
                              HGLOBAL hMetaPict, BOOL fDeleteOldAspect,
                              BOOL fSetupViewAdvise,
                              LPADVISESINK  lpAdviseSink,
                              BOOL FAR* lpfMustUpdate, BOOL fRunIfNecessary);
  void    _IFUNC HandleLinkError();

public:

// IPart methods
  virtual HRESULT _IFUNC Init(PIBSite, BOleInitInfo FAR*);
  virtual HRESULT _IFUNC Save (PIStorage, BOOL, BOOL);

// Information
  virtual HRESULT  _IFUNC CanOpenInPlace ();
  virtual HRESULT  _IFUNC GetPalette (LPLOGPALETTE FAR*);
  virtual HRESULT  _IFUNC SetHost (IBContainer FAR *pObjContainer);
  virtual LPOLESTR _IFUNC GetName (BOlePartName);

// Server part state changes
  virtual HRESULT _IFUNC Open (BOOL );
  virtual HRESULT _IFUNC Show (BOOL );
  virtual HRESULT _IFUNC Close ();
  virtual HRESULT _IFUNC Activate (BOOL fActivate);
  virtual HWND    _IFUNC OpenInPlace (HWND);

// Part size and update methods
  virtual HRESULT _IFUNC GetPartSize (LPSIZE);
  virtual HRESULT _IFUNC SetPartSize (LPSIZE);
  virtual HRESULT _IFUNC SetPartPos (LPRECT);
  virtual HRESULT _IFUNC Draw (HDC, LPCRECTL lpExtent, LPCRECTL lpClip, BOleAspect, BOleDraw);

// Tool handling
  virtual HRESULT _IFUNC InsertMenus (HMENU, BOleMenuWidths*);
  virtual void    _IFUNC FrameResized (const RECT *, BOOL);
  virtual HRESULT _IFUNC ShowTools (BOOL);

  virtual HRESULT _IFUNC EnumVerbs (BOleVerb*);
  virtual HRESULT _IFUNC DragFeedback(POINT*,BOOL);
  virtual HRESULT _IFUNC DoVerb (UINT);

//IBLinkInfo methods
  virtual HRESULT _IFUNC SourceBreak();
  virtual HRESULT _IFUNC SourceGet(LPOLESTR, UINT);
  virtual HRESULT _IFUNC SourceSet(LPOLESTR);
  virtual HRESULT _IFUNC SourceOpen();
  virtual HRESULT _IFUNC UpdateGet(BOleLinkUpdate*);
  virtual HRESULT _IFUNC UpdateSet(BOleLinkUpdate);
  virtual HRESULT _IFUNC UpdateNow();

//Clipboard methods inherited from IBDataProvider
  virtual UINT    _IFUNC CountFormats ();
  virtual HRESULT _IFUNC GetFormat (UINT, BOleFormat*);
  virtual HANDLE  _IFUNC GetFormatData (BOleFormat*);
  virtual HRESULT _IFUNC SetFormatData(BOleFormat FAR* fmt, HANDLE data, BOOL release);

  virtual HRESULT _IFUNC DoQueryInterface(REFIID iid, void FAR* FAR* pif);

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

protected:
  // IOleClientSite Methods
  //
  HRESULT _IFUNC SaveObject();
  HRESULT _IFUNC GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
                            IMoniker* FAR* ppmk);
  HRESULT _IFUNC GetContainer(IOleContainer* FAR* ppContainer);
  HRESULT _IFUNC ShowObject();
  HRESULT _IFUNC OnShowWindow(BOOL fShow);
  HRESULT _IFUNC RequestNewObjectLayout();

  // IAdviseSink Methods
  //
  void _IFUNC OnDataChange(FORMATETC FAR* pFormatetc,
                           STGMEDIUM FAR* pStgmed);
  void _IFUNC OnViewChange(DWORD aspects, LONG lindex);
  void _IFUNC OnRename(IMoniker* pmk);
  void _IFUNC OnSave();
  void _IFUNC OnClose();

  // IOleWindow methods, base of IOleInPlaceUIWindow and IOleInPlaceSite
  //
  HRESULT _IFUNC GetWindow(HWND FAR* lphwnd);
  HRESULT _IFUNC ContextSensitiveHelp(BOOL fEnterMode);

  // IOleInPlaceSite methods
  //
  HRESULT _IFUNC CanInPlaceActivate();
  HRESULT _IFUNC OnInPlaceActivate();
  HRESULT _IFUNC OnUIActivate();
  HRESULT _IFUNC GetWindowContext(IOleInPlaceFrame* FAR* lplpFrame,
                                  IOleInPlaceUIWindow* FAR* lplpDoc,
                                  LPRECT lprcPosRect,
                                  LPRECT lprcClipRect,
                                  LPOLEINPLACEFRAMEINFO lpFrameInfo);
  HRESULT _IFUNC Scroll(SIZE scrollExtent);
  HRESULT _IFUNC OnUIDeactivate(BOOL fUndoable);
  HRESULT _IFUNC OnInPlaceDeactivate();
  HRESULT _IFUNC DiscardUndoState();
  HRESULT _IFUNC DeactivateAndUndo();
  HRESULT _IFUNC OnPosRectChange(LPCRECT lprcPosRect);

  // IBLinkable methods
  //
  virtual HRESULT _IFUNC GetMoniker(IMoniker FAR* FAR* ppMon);
  virtual HRESULT _IFUNC OnRename(PIBLinkable pContainer, LPCOLESTR szName);

  // IDropSource methods
  //
  HRESULT _IFUNC QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
  HRESULT _IFUNC GiveFeedback(DWORD dwEffect);
};

#endif

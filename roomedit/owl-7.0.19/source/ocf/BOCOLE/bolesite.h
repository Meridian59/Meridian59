//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Interface of the Bolero half of the OLE2 server object. BOleSite objects
//  impersonate the client application from the point of view of the Bolero
//  customer who's writing a server object (IPart)
//
//  NOTE: Since the server object only has to implement IPart, it's
//        convenient to have only one helper object (BOleSite) which
//        inherits from all the required Bolero container interfaces
//----------------------------------------------------------------------------
#ifndef _BOLESITE_H
#define _BOLESITE_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif

#ifndef _BOLEFACT_H
#include "bolefact.h"
#endif

#ifndef _BHATCH_H
#include "bhatch.h"
#endif

class _ICLASS BOleService;

class _ICLASS BOleSite :   public BOleComponent, public IBSite,
                           public IBContainer,   public IBLinkable,

                           public IPersistStorage, public IDataObject,
                  public IOleObject,      public IOleInPlaceObject,
                           public IDropSource,     public IEnumFORMATETC,
                           public IOleInPlaceActiveObject,
                           public IBDataState
{
  // to have separate IBWindow implementation
  class _ICLASS IBApplicationImpl : public IBApplication {
  public:
    IBUnknownMain *pOuter;
    BOleSite *pThis;

    IBApplicationImpl::IBApplicationImpl(BOleSite *pT, IBUnknownMain *pO)
    : pThis (pT), pOuter (pO) {}
    DEFINE_IUNKNOWN(pOuter);

    // IBApplication methods
    //
    virtual HWND     _IFUNC GetWindow ();
    virtual HRESULT  _IFUNC GetWindowRect (LPRECT);
    virtual LPCOLESTR _IFUNC GetWindowTitle ();
    virtual void     _IFUNC AppendWindowTitle (LPCOLESTR);
    virtual HRESULT  _IFUNC SetStatusText (LPCOLESTR);

    virtual HRESULT  _IFUNC RequestBorderSpace (LPCRECT);
    virtual HRESULT  _IFUNC SetBorderSpace (LPCRECT);

    virtual HRESULT  _IFUNC InsertContainerMenus (HMENU,BOleMenuWidths FAR*);
    virtual HRESULT  _IFUNC SetFrameMenu (HMENU);
    virtual void     _IFUNC RestoreUI ();
    virtual HRESULT  _IFUNC Accelerator (LPMSG);
    virtual HRESULT  _IFUNC GetAccelerators (HACCEL FAR*, int FAR*);

    virtual LPCOLESTR _IFUNC GetAppName ();
    virtual BOleHelp _IFUNC HelpMode (BOleHelp);
    virtual HRESULT  _IFUNC CanLink ();
    virtual HRESULT  _IFUNC CanEmbed ();
    virtual HRESULT  _IFUNC IsMDI ();
    virtual HRESULT  _IFUNC OnModalDialog (BOOL);
    virtual void     _IFUNC DialogHelpNotify (BOleDialogHelp);
    virtual void     _IFUNC ShutdownMaybe ();
  } iApplication;

  friend IBApplicationImpl;
  friend class BOleHatchWindow;


public:
  IDataAdviseHolder * pDAdvHolder;
  IOleAdviseHolder  * pAdvHolder;
  LPUNKNOWN          pDocument;

protected:
  BOleService         * pService;
  PIBPart               pPart;
  PIBDataProvider       pProvider;
  IMoniker            * pmkDoc;    // moniker of server doc
  IOleClientSite      * pClientSite;
  IOleInPlaceSite     * pIPSite;
  IOleInPlaceFrame    * pIPFrame;
  IOleInPlaceUIWindow * pIPDoc;
  IStorage            * pStg;

  BOOL fShown          : 1;
  BOOL fInPlaceActive  : 1;
  BOOL fInPlaceVisible : 1;
  BOOL fInsideOut      : 1;
  BOOL fUIActive       : 1;
  BOOL fInHelpMode     : 1;
  BOOL fIsDirty        : 1;
  BOOL fHandsOff       : 1;
  BOOL fInClose        : 1;

  CLSID cid;
  DWORD regLink;               // to register full moniker

  LPOLESTR pszProgID;
  LPOLESTR pszInstName;
  OLEINPLACEFRAMEINFO frameInfo;
  RECT rcPosRect;
  RECT rcClipRect;

  HWND hWndParent;
  HWND hWndInPlace;

  OLEMENUGROUPWIDTHS  menuGroupWidths;
  HOLEMENU            hOleMenu;
  HMENU               hMenuShared;

  BOleHatchWindow *pHatchWnd;
  POINT hatchOffset;
  ULONG enumFmtIndex;

  LPOLESTR pAppName;    // Container app's name (not used in current OLE UI spec)
  LPOLESTR pWindowTitle;  // Container app's document (used in open editing)
  LPOLESTR pInPlaceName;  // Server object name for SetActiveObject

  LPFORMATETC formatList;
  UINT       formatCount;
  BOOL formatLink;
  BOOL formatEmbed;

  BOleScaleFactor scale;

public:

  BOleSite(BOleClassManager * pFact, IBUnknownMain *pOuter, BOleService*);
  ~BOleSite ();

  // IUnknown methods
  //
  DEFINE_IUNKNOWN(pObjOuter)

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IDataObject methods
  //
  HRESULT _IFUNC GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
  HRESULT _IFUNC GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
  HRESULT _IFUNC QueryGetData(LPFORMATETC pformatetc);
  HRESULT _IFUNC GetCanonicalFormatEtc(LPFORMATETC pformatetc,
                     LPFORMATETC pformatetcOut);
  HRESULT _IFUNC SetData(LPFORMATETC pformatetc, STGMEDIUM FAR* pmedium,
               BOOL fRelease);
  HRESULT _IFUNC EnumFormatEtc(DWORD dwDirection,
                   LPENUMFORMATETC FAR* ppenumFormatEtc);
  HRESULT _IFUNC DAdvise(FORMATETC FAR* pFormatetc, DWORD advf,
               IAdviseSink* pAdvSink, DWORD FAR* pdwConnection);
  HRESULT _IFUNC DUnadvise(DWORD dwConnection);
  HRESULT _IFUNC EnumDAdvise(IEnumSTATDATA* FAR* ppenumAdvise);

  // IEnumFORMATETC methods
  //
  HRESULT _IFUNC Next (ULONG celt, FORMATETC FAR * rgelt, ULONG FAR* pceltFetched);
  HRESULT _IFUNC Skip (ULONG celt);
  HRESULT _IFUNC Reset ();
  HRESULT _IFUNC Clone (IEnumFORMATETC FAR* FAR* ppenum);

  // IOleObject methods
  //
  HRESULT _IFUNC SetClientSite(IOleClientSite* pClientSite);
  HRESULT _IFUNC GetClientSite(IOleClientSite* FAR* ppClientSite);
  HRESULT _IFUNC SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
  HRESULT _IFUNC Close(DWORD dwSaveOption);
  HRESULT _IFUNC SetMoniker(DWORD dwWhichMoniker, IMoniker* pmk);
  HRESULT _IFUNC GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
                IMoniker* FAR* ppmk);
  HRESULT _IFUNC InitFromData(IDataObject* pDataObject, BOOL fCreation,
                DWORD dwReserved);
  HRESULT _IFUNC GetClipboardData(DWORD dwReserved,
                                  IDataObject* FAR* ppDataObject);
  HRESULT _IFUNC DoVerb(LONG iVerb, LPMSG lpmsg,
                         IOleClientSite* pActiveSite, LONG lindex,
                         HWND hwndParent, LPCRECT lprcPosRect);
  HRESULT _IFUNC EnumVerbs(IEnumOLEVERB* FAR* ppenumOleVerb);
  HRESULT _IFUNC Update();
  HRESULT _IFUNC IsUpToDate();
  HRESULT _IFUNC GetUserClassID(CLSID FAR* pClsid);
  HRESULT _IFUNC GetUserType(DWORD dwFormOfType, LPOLESTR FAR* pszUserType);
  HRESULT _IFUNC SetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);
  HRESULT _IFUNC GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);
  HRESULT _IFUNC Advise(IAdviseSink* pAdvSink, DWORD FAR* pdwConnection);
  HRESULT _IFUNC Unadvise(DWORD dwConnection);
  HRESULT _IFUNC EnumAdvise(IEnumSTATDATA* FAR* ppenumAdvise);
  HRESULT _IFUNC GetMiscStatus(DWORD dwAspect, DWORD FAR* pdwStatus);
  HRESULT _IFUNC SetColorScheme(LPLOGPALETTE lpLogpal);

  // IPersistStorage and IPersistFile Common methods
  //
  HRESULT _IFUNC GetClassID(LPCLSID lpClassID);
  HRESULT _IFUNC IsDirty();

  // IPersistStorage methods
  //
  HRESULT _IFUNC InitNew(IStorage* pStg);
  HRESULT _IFUNC Load(IStorage* pStg);
  HRESULT _IFUNC Save(IStorage* pStgSave,BOOL fSameAsLoad);
  HRESULT _IFUNC SaveCompleted(IStorage* pStgSaved);
  HRESULT _IFUNC HandsOffStorage();

  // IOleWindow methods
  //
  HRESULT _IFUNC GetWindow (HWND FAR* lphwnd);
  HRESULT _IFUNC ContextSensitiveHelp (BOOL fEnterMode);

  // IOleInPlaceObject methods
  //
  HRESULT _IFUNC InPlaceDeactivate ();
  HRESULT _IFUNC UIDeactivate ();
  HRESULT _IFUNC SetObjectRects (LPCRECT pPos,  LPCRECT pClip);
  HRESULT _IFUNC ReactivateAndUndo ();

  // IOleInPlaceActiveObject methods
  //
  HRESULT _IFUNC TranslateAccelerator (LPMSG lpmsg);
  HRESULT _IFUNC OnFrameWindowActivate (BOOL fActivate);
  HRESULT _IFUNC OnDocWindowActivate (BOOL fActivate);
  HRESULT _IFUNC ResizeBorder (LPCRECT lprectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL fFrameWindow);
  HRESULT _IFUNC EnableModeless (BOOL fEnable);

  // IDropSource methods
  //
  HRESULT _IFUNC QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
  HRESULT _IFUNC GiveFeedback(DWORD dwEffect);

  // IBWindow methods
  //
  virtual HWND      _IFUNC GetWindow ();
  virtual HRESULT   _IFUNC GetWindowRect (LPRECT);
  virtual LPCOLESTR _IFUNC GetWindowTitle ();
  virtual void      _IFUNC AppendWindowTitle (LPCOLESTR);
  virtual HRESULT   _IFUNC SetStatusText (LPCOLESTR);

  virtual HRESULT   _IFUNC RequestBorderSpace (LPCRECT);
  virtual HRESULT   _IFUNC SetBorderSpace (LPCRECT);

  virtual HRESULT   _IFUNC InsertContainerMenus (HMENU,BOleMenuWidths FAR*);
  virtual HRESULT   _IFUNC SetFrameMenu (HMENU);
  virtual void      _IFUNC RestoreUI ();
  virtual HRESULT   _IFUNC Accelerator (LPMSG);
  virtual HRESULT   _IFUNC GetAccelerators (HACCEL FAR*, int FAR*);

  // IBContainer methods
  //
  virtual HRESULT   _IFUNC AllowInPlace ();
  virtual HRESULT   _IFUNC BringToFront ();
  virtual HRESULT   _IFUNC FindDropDest (LPPOINT, PIBDropDest FAR*);

  // ISite methods
  //
  virtual HRESULT _IFUNC SiteShow(BOOL);
  virtual HRESULT _IFUNC DiscardUndo();
  virtual HRESULT _IFUNC GetSiteRect(LPRECT,LPRECT);
  virtual HRESULT _IFUNC SetSiteRect(LPCRECT);
  virtual HRESULT _IFUNC SetSiteExtent(LPCSIZE);
  virtual void    _IFUNC Invalidate(BOleInvalidate);
  virtual void    _IFUNC OnSetFocus(BOOL);
  virtual HRESULT _IFUNC Init(PIBDataProvider pP, PIBPart pPart, LPCOLESTR psz, BOOL fHatchWnd);
  virtual void    _IFUNC Disconnect ();
  virtual HRESULT _IFUNC GetZoom (BOleScaleFactor *pScale);
  virtual HRESULT _IFUNC CalcZoom (LPCRECT rcposRect);

  // ILinkable methods
  //
  virtual HRESULT _IFUNC GetMoniker(IMoniker FAR* FAR* ppMon);
  virtual HRESULT _IFUNC OnRename(PIBLinkable pILContainer, LPCOLESTR szName);

  // IBDataState
  // IsDirty is implemented as part of IPersitStorage
  virtual void _IFUNC SetDirty (BOOL fDirty);

protected:

  // Implementation methods
  //
  HRESULT    _IFUNC GetMetaFileData (HGLOBAL&);
  HGLOBAL   _IFUNC GetObjDescData ();
  HRESULT   _IFUNC GetEmbeddedObjectData (LPFORMATETC, LPSTGMEDIUM);
  HRESULT   _IFUNC GetLinkSourceData(LPMONIKER, REFCLSID, LPFORMATETC, LPSTGMEDIUM);
  void  _IFUNC GetFormatFlags(BOOL *fEmbed, BOOL *fLinkable);
  void  _IFUNC GetFormatList(LPFORMATETC *ppList, UINT *pCount);
  void  _IFUNC GetFormatCount(UINT *pCount);
  void  _IFUNC GetFormatInfo(BOOL *fEmbed, BOOL *fLinkable, LPFORMATETC *ppList, UINT *pCount);

  LPSTORAGE _IFUNC CreateStorageInMemory (DWORD);

  HRESULT _IFUNC Lock (BOOL fLock, BOOL fLastUnlockReleases);
  HRESULT _IFUNC ShowWindow ();
  HRESULT _IFUNC HideWindow (BOOL fShutdown);
  HRESULT _IFUNC AssembleMenus ();
  void    _IFUNC DisassembleMenus ();
  HRESULT _IFUNC DoInPlaceActivate (BOOL fActivateOnly);
  HRESULT _IFUNC DoInPlaceShow (BOOL fActivateOnly);
  HRESULT _IFUNC DoUIActivate ();
  HRESULT _IFUNC DoInPlaceDeactivate ();
  HRESULT _IFUNC DoUIDeactivate ();
  HRESULT _IFUNC DoInPlaceHide ();
  HRESULT _IFUNC DoClose ();
};

#endif


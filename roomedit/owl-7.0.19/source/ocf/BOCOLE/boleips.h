//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//   Implements the Bolero half of the OLE2 in-process server object.
//    BOleInProcSite  overrides some of the default handler implementation
//    for IViewObject and IBSite implementation on the BOleInProcServer
//    taking advantage of the fact that we're in a DLL.
//----------------------------------------------------------------------------
#ifndef _BOLEIPS_H
#define _BOLEIPS_H

#include <bolesite.h>

class _ICLASS BOleService;

//
//  class BOleInProcServer
//  ----- --------------
//

class _ICLASS BOleInProcServer : public BOleSite,
                               public IViewObject2,
                 public IExternalConnection,
                 public IRunnableObject
{
protected:
  BOleService *pService;
  IUnknown *pDefHandler;

  IAdviseSink * pAdviseView;
  DWORD dwAdviseAspects;
  DWORD dwAdviseFlags;
  DWORD exCon;
  BOOL fRunningMode;
public:

  BOleInProcServer(BOleClassManager *pFact, IBUnknownMain *pOuter, BOleService*);
  ~BOleInProcServer();

  // IUnknown methods
  //
  DEFINE_IUNKNOWN(pObjOuter);

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IViewObject methods
  //
  virtual HRESULT _IFUNC Draw(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
    HDC hicTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL(CALLBACK * pfnContinue)(DWORD),
    DWORD dwContinue);

  virtual HRESULT _IFUNC GetColorSet(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
    HDC hicTargetDev,
    LPLOGPALETTE FAR* ppColorSet);

  virtual HRESULT _IFUNC Freeze(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect,
    DWORD FAR* pdwFreeze);
  virtual HRESULT _IFUNC Unfreeze(DWORD dwFreeze);
  virtual HRESULT _IFUNC SetAdvise(DWORD aspects, DWORD advf,
    LPADVISESINK pAdvSink);
  virtual HRESULT _IFUNC GetAdvise(DWORD FAR* pAspects, DWORD FAR* pAdvf,
    LPADVISESINK FAR* ppAdvSink);

  virtual HRESULT _IFUNC GetExtent(DWORD dwDrawAspect,
    LONG lindex, DVTARGETDEVICE FAR*ptd, LPSIZEL lpsizel);

  // BOleSite overrides
  //
  virtual HRESULT _IFUNC GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);
  virtual HRESULT _IFUNC TranslateAccelerator (LPMSG lpmsg);
  virtual HRESULT _IFUNC EnumVerbs(IEnumOLEVERB* FAR* ppenumOleVerb);
  virtual HRESULT _IFUNC GetUserType(DWORD dwFormOfType, LPOLESTR FAR* pszUserType);
  virtual HRESULT _IFUNC GetMiscStatus(DWORD dwAspect, DWORD FAR* pdwStatus);
  virtual HRESULT _IFUNC SetExtent (DWORD dwDrawAspect, LPSIZEL lpsizel);
  virtual HRESULT _IFUNC Close(DWORD dwSaveOption);

  // ISite methods
  //
  virtual void    _IFUNC Invalidate(BOleInvalidate);
  virtual HRESULT _IFUNC Init(PIBDataProvider pP, PIBPart pPart,
    LPCOLESTR psz, BOOL fHatchWnd);

  // IExternalConnection
  //
  virtual DWORD _IFUNC AddConnection(DWORD, DWORD);
  virtual DWORD _IFUNC ReleaseConnection(DWORD, DWORD, BOOL);

  // IRunnableObject
  //
  virtual HRESULT _IFUNC GetRunningClass(LPCLSID lpClsid);
  virtual HRESULT _IFUNC Run(LPBINDCTX pbc);
  virtual BOOL _IFUNC IsRunning();
  virtual HRESULT _IFUNC LockRunning(BOOL fLock, BOOL fLastCloses);
  virtual HRESULT _IFUNC SetContainedObject(BOOL fContained);

  // IPersistStorage overrides
  virtual HRESULT _IFUNC InitNew(IStorage* pStg);
  virtual HRESULT _IFUNC Load(IStorage* pStg);
  virtual HRESULT _IFUNC Save(IStorage* pStgSave,BOOL fSameAsLoad);
  virtual HRESULT _IFUNC SaveCompleted(IStorage* pStgSaved);
  virtual HRESULT _IFUNC HandsOffStorage();
};

class _ICLASS BOleInProcHandler : public BOleComponent,
                               public IViewObject2,
                               public IOleInPlaceActiveObject,
                               public IBSite
{
protected:
  BOleService *pService;
  IUnknown *pDefHandler;
  IBSite *pDefSite;
  IOleInPlaceActiveObject *pDefIPAO;
  PIBPart pPart;
  PIBDataProvider pProvider;

  IAdviseSink * pAdviseView;
  DWORD dwAdviseAspects;
  DWORD dwAdviseFlags;

public:

  BOleInProcHandler(BOleClassManager *pFact, IBUnknownMain *pOuter, BOleService*);
  ~BOleInProcHandler();

  // IUnknown methods
  //
  DEFINE_IUNKNOWN(pObjOuter);

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IViewObject methods
  //
  virtual HRESULT _IFUNC Draw(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
    HDC hicTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL(CALLBACK * pfnContinue)(DWORD),
    DWORD dwContinue);

  virtual HRESULT _IFUNC GetColorSet(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
    HDC hicTargetDev,
    LPLOGPALETTE FAR* ppColorSet);

  virtual HRESULT _IFUNC Freeze(DWORD dwDrawAspect, LONG lindex,
    void FAR* pvAspect,
    DWORD FAR* pdwFreeze);
  virtual HRESULT _IFUNC Unfreeze(DWORD dwFreeze);
  virtual HRESULT _IFUNC SetAdvise(DWORD aspects, DWORD advf,
    LPADVISESINK pAdvSink);
  virtual HRESULT _IFUNC GetAdvise(DWORD FAR* pAspects, DWORD FAR* pAdvf,
    LPADVISESINK FAR* ppAdvSink);

  virtual HRESULT _IFUNC GetExtent(DWORD dwDrawAspect,
    LONG lindex, DVTARGETDEVICE FAR*ptd, LPSIZEL lpsizel);

  // IOleInPlaceActiveObject methods
  //
  virtual HRESULT _IFUNC GetWindow(HWND FAR *phwnd);
  virtual HRESULT _IFUNC ContextSensitiveHelp(BOOL fEnterMode);
  virtual HRESULT _IFUNC TranslateAccelerator (LPMSG lpmsg);
  virtual HRESULT _IFUNC OnFrameWindowActivate (BOOL fActivate);
  virtual HRESULT _IFUNC OnDocWindowActivate (BOOL fActivate);
  virtual HRESULT _IFUNC ResizeBorder (LPCRECT lprectBorder,
    LPOLEINPLACEUIWINDOW lpUIWindow, BOOL fFrameWindow);
  virtual HRESULT _IFUNC EnableModeless (BOOL fEnable);

  // ISite methods
  //
  virtual HRESULT _IFUNC SiteShow(BOOL);
  virtual HRESULT _IFUNC DiscardUndo();
  virtual HRESULT _IFUNC GetSiteRect(LPRECT,LPRECT);
  virtual HRESULT _IFUNC SetSiteRect(LPCRECT);
  virtual HRESULT _IFUNC SetSiteExtent(LPCSIZE);
  virtual void    _IFUNC Invalidate(BOleInvalidate);
  virtual void    _IFUNC OnSetFocus(BOOL);
  virtual HRESULT _IFUNC Init(PIBDataProvider pP, PIBPart pPart,
    LPCOLESTR psz, BOOL fHatchWnd);
  virtual void _IFUNC Disconnect ();
  virtual HRESULT _IFUNC GetZoom( BOleScaleFactor *pScale);

  // implementation methods
  //
  IOleInPlaceActiveObject * DefaultIPAO();
};

#endif

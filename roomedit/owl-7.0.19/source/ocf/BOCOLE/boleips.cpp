//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//   Implements the Bolero half of the OLE2 in-process server object.
//    BOleIps objects impersonate the client application from the point of
//    view of the Bolero customer who's writing a server object (IPart)
//----------------------------------------------------------------------------
#include "boleips.h"
#include "bolecman.h"


//**************************************************************************
//
// BOleInProcHandler implementation
//
//**************************************************************************

BOleInProcHandler::BOleInProcHandler(BOleClassManager *pFact,
  IBUnknownMain * pOuter, BOleService *pSvc ) : BOleComponent(pFact, pOuter)
{
  pService = pSvc;
  pDefHandler = NULL;
  pDefIPAO = NULL;
  pDefSite = NULL;
  pAdviseView = NULL;
  dwAdviseFlags = 0L;
  dwAdviseAspects = 0L;
}

BOleInProcHandler::~BOleInProcHandler ()
{
  if (pDefHandler)
    pDefHandler->Release();

  if (pAdviseView)
    pAdviseView->Release();
}

HRESULT _IFUNC BOleInProcHandler::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces
  if (SUCCEEDED(hr = IViewObject2_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IOleInPlaceActiveObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IBSite_QueryInterface(this, iid, ppv))) {
  }
  // base classes
  else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))) {
  }
  // helpers
  else if (pDefHandler && SUCCEEDED(hr = pDefHandler->QueryInterface(iid, ppv))) {
  }
  return hr;
}


//**************************************************************************
//
//  IViewObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleInProcHandler::Draw (DWORD dwDrawAspect, LONG lindex,
  void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
  HDC hicTargetDev,
  HDC hdcDraw,
  LPCRECTL lprcBounds,
  LPCRECTL lprcWBounds,
  BOOL(CALLBACK * pfnContinue)(DWORD),
  DWORD dwContinue)
{
  HRESULT hr = ResultFromScode(VIEW_E_DRAW);
  // Let provider Draw
  // unless its iconic which the handler does for us
  //
  if (dwDrawAspect != DVASPECT_ICON) {
    if (SUCCEEDED(pProvider->Draw(hdcDraw, lprcBounds,
      // lprcWBounds is NULL unless this is a metafile
      //
      lprcWBounds ? lprcWBounds : lprcBounds,
      (BOleAspect) dwDrawAspect))) {
      hr = NOERROR;
    }
  }

  // If this failed let the default handler render the Metafile
  //
  if (!SUCCEEDED(hr)) {
    IViewObject *pVO;
    if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
      hr = pVO->Draw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev, hdcDraw, lprcBounds, lprcWBounds, pfnContinue, dwContinue);
      pVO->Release();
    }
  }
  return hr;
}

HRESULT _IFUNC BOleInProcHandler::GetColorSet (
  DWORD dwDrawAspect,
  LONG lindex,
  void FAR* pvAspect,
  DVTARGETDEVICE FAR * ptd,
  HDC hicTargetDev,
  LPLOGPALETTE FAR* ppColorSet)
{
  // what color pallete will the object use when rendering
  return pPart->GetPalette (ppColorSet);
}

HRESULT _IFUNC BOleInProcHandler::Freeze (
  DWORD dwDrawAspect,
  LONG lindex,
  void FAR* pvAspect,
  DWORD FAR* pdwFreeze)
{
  HRESULT hr;
  IViewObject *pVO;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
    hr = pVO->Freeze(dwDrawAspect, lindex, pvAspect, pdwFreeze);
    pVO->Release();
    return hr;
  }
  // the object shouldn't change the rendering
  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleInProcHandler::Unfreeze (DWORD dwFreeze)
{
  HRESULT hr;
  IViewObject *pVO;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
    hr = pVO->Unfreeze(dwFreeze);
    pVO->Release();
    return hr;
  }
  // ok it can change again
  return NOERROR;
}

HRESULT _IFUNC BOleInProcHandler::SetAdvise (DWORD aspects, DWORD advf, LPADVISESINK pAdvSink)
{
  dwAdviseAspects = aspects;
  dwAdviseFlags = advf;
  if (pAdviseView)
    pAdviseView->Release();
  pAdviseView = pAdvSink;

  if (pAdviseView)
    pAdviseView->AddRef ();


  if (advf | ADVF_PRIMEFIRST)
    // Cause advises now
    //
    Invalidate(BOLE_INVAL_DATA | BOLE_INVAL_VIEW);

  return NOERROR;
}

HRESULT _IFUNC BOleInProcHandler::GetAdvise (DWORD FAR* pAspects,
  DWORD FAR* pAdvf, LPADVISESINK FAR* ppAdvSink)
{
  if (ppAdvSink)  {
    pAdviseView->AddRef();

    *ppAdvSink = pAdviseView;
    *pAspects = dwAdviseAspects;
    *pAdvf = dwAdviseFlags;
    return NOERROR;
  }
  else {
    return ResultFromScode (E_INVALIDARG);
  }
}

HRESULT _IFUNC BOleInProcHandler::GetExtent(DWORD dwDrawAspect,
  LONG lindex, DVTARGETDEVICE FAR *ptd, LPSIZEL lpsizel)
{
  if (dwDrawAspect != DVASPECT_CONTENT)
    return ResultFromScode (E_FAIL);

  if (!lpsizel)
    return ResultFromScode (E_INVALIDARG);

  SIZE s;
  HRESULT hr = pProvider->GetPartSize (&s);

  lpsizel->cx = MAP_PIX_TO_LOGHIM (s.cx, BOleService::pixPerIn.x);
  lpsizel->cy = MAP_PIX_TO_LOGHIM (s.cy, BOleService::pixPerIn.y);

  return hr;
}

//**************************************************************************
//
// IBSite methods (override this interface on aggregated BOleIPSServer
// to implement Invalidate).
//
//**************************************************************************

HRESULT _IFUNC BOleInProcHandler::Init(PIBDataProvider pDP,
  PIBPart pP, LPCOLESTR szProgId, BOOL fHatchWnd)
{
  HRESULT hr;
  // Initialize back pointers
  //
  pProvider = pDP;
  pPart = pP;

  CLSID cid;
  if (SUCCEEDED(hr = CLSIDFromProgID(szProgId, &cid))) {

    // create the default handler, which will launch the exe server
    //
    OleCreateEmbeddingHelper(cid, AsPIUnknown(pObjOuter),
      EMBDHLP_INPROC_HANDLER | EMBDHLP_DELAYCREATE, NULL, IID_IUnknown,
      &(LPVOID)pDefHandler);

    // initialize default IPS
    //
    hr = pDefHandler->QueryInterface(IID_IBSite, &(LPVOID)pDefSite);
    if (SUCCEEDED(hr)) {
      pDefSite->Release();     // prevent aggregation deadlock
      hr = pDefSite->Init(pDP, pP, szProgId, fHatchWnd);
    }
  }
  return hr;
}

HRESULT _IFUNC BOleInProcHandler::SiteShow(BOOL b)
{
  return pDefSite->SiteShow(b);
}



HRESULT _IFUNC BOleInProcHandler::DiscardUndo()
{
  return pDefSite->DiscardUndo();
}

HRESULT _IFUNC BOleInProcHandler::GetSiteRect(LPRECT r1, LPRECT r2)
{
  return pDefSite->GetSiteRect(r1,r2);
}

HRESULT _IFUNC BOleInProcHandler::SetSiteRect(LPCRECT r)
{
  return pDefSite->SetSiteRect(r);
}

HRESULT _IFUNC BOleInProcHandler::SetSiteExtent(LPCSIZE s)
{
  return pDefSite->SetSiteExtent(s);
}

void _IFUNC BOleInProcHandler::OnSetFocus(BOOL b)
{
  pDefSite->OnSetFocus(b);
}

void _IFUNC BOleInProcHandler::Disconnect ()
{
  pDefSite->Disconnect ();
}

HRESULT _IFUNC BOleInProcHandler::GetZoom( BOleScaleFactor *pScale)
{
  return pDefSite->GetZoom(pScale);
}

void _IFUNC BOleInProcHandler::Invalidate( BOleInvalidate b)
{
  if ((b & BOLE_INVAL_VIEW) && pAdviseView) {
    pAdviseView->OnViewChange(dwAdviseAspects, -1);
    if (dwAdviseFlags == ADVF_ONLYONCE) {
      pAdviseView->Release();
      pAdviseView = NULL;
      dwAdviseAspects = 0L;
      dwAdviseFlags = 0L;
    }
  }
  if (b)  // VIEW or DATA or PERSIST remains invalid
    pDefSite->Invalidate(b);
}

//**************************************************************************
//
// IOleInPlaceActiveObject
//
// This interface is overridden to pre-process translator and other messages
// from the container's message loop.
//
//**************************************************************************

IOleInPlaceActiveObject * BOleInProcHandler::DefaultIPAO()
{
  if (!pDefIPAO) {
    // Initialize pointer to default in place active object implementation
    //
    if (SUCCEEDED(pDefHandler->QueryInterface(IID_IOleInPlaceActiveObject,
      &(LPVOID) pDefIPAO))) {
      pDefIPAO->Release();    // prevent aggregation deadlock
    }
  }
  return pDefIPAO;
}

HRESULT _IFUNC BOleInProcHandler::GetWindow(HWND FAR *phwnd)
{
  return DefaultIPAO()->GetWindow (phwnd);
}

HRESULT _IFUNC BOleInProcHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
  return DefaultIPAO()->ContextSensitiveHelp (fEnterMode);
}

//  Allow DLL Server to translate accelerators
//
HRESULT _IFUNC BOleInProcHandler::TranslateAccelerator (LPMSG lpmsg)
{
  return pService->GetApplication()->Accelerator(lpmsg);
}

HRESULT _IFUNC BOleInProcHandler::OnFrameWindowActivate (BOOL fActivate)
{
  return DefaultIPAO()->OnFrameWindowActivate (fActivate);
}

HRESULT _IFUNC BOleInProcHandler::OnDocWindowActivate (BOOL fActivate)
{
  return DefaultIPAO()->OnDocWindowActivate (fActivate);
}

HRESULT _IFUNC BOleInProcHandler::ResizeBorder (LPCRECT lprectBorder,
    LPOLEINPLACEUIWINDOW lpUIWindow, BOOL fFrameWindow)
{
  return DefaultIPAO()->ResizeBorder (lprectBorder, lpUIWindow, fFrameWindow);
}

HRESULT _IFUNC BOleInProcHandler::EnableModeless (BOOL fEnable)
{
  return DefaultIPAO()->EnableModeless (fEnable);
}



//**************************************************************************
//
// BOleInProcServer implementation
//
//**************************************************************************

BOleInProcServer::BOleInProcServer(BOleClassManager *pFact,
  IBUnknownMain * pOuter, BOleService *pSvc ) : BOleSite(pFact, pOuter, pSvc)
{
  pService = pSvc;
  pDefHandler = NULL;
  pAdviseView = NULL;
  dwAdviseFlags = 0L;
  dwAdviseAspects = 0L;
  exCon = 0L;
  fRunningMode = FALSE;
}

BOleInProcServer::~BOleInProcServer ()
{
  if (pDefHandler)
    pDefHandler->Release();

  if (pAdviseView)
    pAdviseView->Release();
}

HRESULT _IFUNC BOleInProcServer::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces
  if (SUCCEEDED(hr = IViewObject2_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IRunnableObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IExternalConnection_QueryInterface(this, iid, ppv))) {
  }
  // base classes
  else if (SUCCEEDED(hr = BOleSite::QueryInterfaceMain(iid, ppv))) {
  }
  // helpers
  else if (pDefHandler && SUCCEEDED(hr = pDefHandler->QueryInterface(iid, ppv))) {
  }
  return hr;
}


//**************************************************************************
//
//  IViewObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleInProcServer::Draw (DWORD dwDrawAspect, LONG lindex,
  void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
  HDC hicTargetDev,
  HDC hdcDraw,
  LPCRECTL lprcBounds,
  LPCRECTL lprcWBounds,
  BOOL(CALLBACK * pfnContinue)(DWORD),
  DWORD dwContinue)
{
  HRESULT hr = ResultFromScode(VIEW_E_DRAW);
  // Let provider Draw unless its iconic which the handler does for us
  //
  if (dwDrawAspect == DVASPECT_CONTENT) {
    if (SUCCEEDED(pProvider->Draw(hdcDraw, lprcBounds,
      // lprcWBounds is NULL unless this is a metafile
      //
      lprcWBounds ? lprcWBounds : lprcBounds,
      (BOleAspect) dwDrawAspect))) {
      hr = NOERROR;
    }
  }

  // If this failed let the default handler render the Metafile
  //
  if (!SUCCEEDED(hr)) {
    IViewObject *pVO;
    if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
      hr = pVO->Draw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev,
               hdcDraw, lprcBounds, lprcWBounds, pfnContinue,
               dwContinue);
      pVO->Release();
    }
  }
  return hr;
}

HRESULT _IFUNC BOleInProcServer::GetColorSet (
  DWORD dwDrawAspect,
  LONG lindex,
  void FAR* pvAspect,
  DVTARGETDEVICE FAR * ptd,
  HDC hicTargetDev,
  LPLOGPALETTE FAR* ppColorSet)
{
  // what color pallete will the object use when rendering
  return pPart->GetPalette (ppColorSet);
}

HRESULT _IFUNC BOleInProcServer::Freeze (
  DWORD dwDrawAspect,
  LONG lindex,
  void FAR* pvAspect,
  DWORD FAR* pdwFreeze)
{
  HRESULT hr;
  IViewObject *pVO;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
    hr = pVO->Freeze(dwDrawAspect, lindex, pvAspect, pdwFreeze);
    pVO->Release();
    return hr;
  }
  // the object shouldn't change the rendering
  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleInProcServer::Unfreeze (DWORD dwFreeze)
{
  HRESULT hr;
  IViewObject *pVO;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject, &(LPVOID)pVO))) {
    hr = pVO->Unfreeze(dwFreeze);
    pVO->Release();
    return hr;
  }
  // ok it can change again
  return NOERROR;
}

HRESULT _IFUNC BOleInProcServer::SetAdvise (DWORD aspects, DWORD advf, LPADVISESINK pAdvSink)
{
  dwAdviseAspects = aspects;
  dwAdviseFlags = advf;
  if (pAdviseView)
    pAdviseView->Release();
  pAdviseView = pAdvSink;

  if (pAdviseView)
    pAdviseView->AddRef ();


  if (advf & ADVF_PRIMEFIRST)
    // Cause advises now
    //
    Invalidate(BOLE_INVAL_DATA | BOLE_INVAL_VIEW);

  return NOERROR;
}

HRESULT _IFUNC BOleInProcServer::GetAdvise (DWORD FAR* pAspects,
  DWORD FAR* pAdvf, LPADVISESINK FAR* ppAdvSink)
{
  if (ppAdvSink)  {
    pAdviseView->AddRef();

    *ppAdvSink = pAdviseView;
    *pAspects = dwAdviseAspects;
    *pAdvf = dwAdviseFlags;
    return NOERROR;
  }
  else {
    return ResultFromScode (E_INVALIDARG);
  }
}

HRESULT _IFUNC BOleInProcServer::GetExtent(DWORD dwDrawAspect,
  LONG lindex, DVTARGETDEVICE FAR *ptd, LPSIZEL lpsizel)
{
  HRESULT hr;
  if (dwDrawAspect != DVASPECT_CONTENT) {
    // Get extent from the cache.
    //
    IViewObject2 *pVO;
    if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject2, &(LPVOID)pVO))) {
      hr = pVO->GetExtent(dwDrawAspect, lindex, ptd, lpsizel);
      pVO->Release();
    }
  }
  else {
    hr = BOleSite::GetExtent(dwDrawAspect, lpsizel);
  }
  return hr;
}

HRESULT _IFUNC BOleInProcServer::GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel)
{
  HRESULT hr;
  if (dwDrawAspect != DVASPECT_CONTENT) {
    // Get extent from the cache.
    //
    IViewObject2 *pVO;
    if (SUCCEEDED(pDefHandler->QueryInterface(IID_IViewObject2, &(LPVOID)pVO))) {
      hr = pVO->GetExtent(dwDrawAspect, -1, NULL, lpsizel);
      pVO->Release();
    }
  }
  else {
    hr = BOleSite::GetExtent(dwDrawAspect, lpsizel);
  }
  return hr;
}

//**************************************************************************
//  Override IPersistStorage to allow OleCache to be persistent.
//

HRESULT _IFUNC BOleInProcServer::InitNew(IStorage* pStg)
{
  LPPERSISTSTORAGE pPersistCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IPersistStorage, &(LPVOID) pPersistCache))) {
    pPersistCache->InitNew(pStg);
    pPersistCache->Release();
  }
  return BOleSite::InitNew(pStg);
}

HRESULT _IFUNC BOleInProcServer::Load(IStorage* pStg)
{
  LPPERSISTSTORAGE pPersistCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IPersistStorage, &(LPVOID) pPersistCache))) {
    pPersistCache->Load(pStg);
    pPersistCache->Release();
  }
  return BOleSite::Load(pStg);
}

HRESULT _IFUNC BOleInProcServer::Save(IStorage* pStgSave,BOOL fSameAsLoad)
{
  LPOLECACHE2 pCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IOleCache2, &(LPVOID)pCache))) {
    pCache->UpdateCache(this, ADVFCACHE_ONSAVE, NULL);
    pCache->Release();
  }
  LPPERSISTSTORAGE pPersistCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IPersistStorage, &(LPVOID)pPersistCache))) {
    pPersistCache->Save(pStgSave, fSameAsLoad);
    pPersistCache->Release();
  }
  return BOleSite::Save(pStgSave, fSameAsLoad);
}

HRESULT _IFUNC BOleInProcServer::SaveCompleted(IStorage* pStgSaved)
{
  LPPERSISTSTORAGE pPersistCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IPersistStorage, &(LPVOID) pPersistCache))) {
    pPersistCache->SaveCompleted(pStgSaved);
    pPersistCache->Release();
  }
  return BOleSite::SaveCompleted(pStgSaved);
}

HRESULT _IFUNC BOleInProcServer::HandsOffStorage()
{
  LPPERSISTSTORAGE pPersistCache;
  if (SUCCEEDED(pDefHandler->QueryInterface(IID_IPersistStorage, &(LPVOID) pPersistCache))) {
    pPersistCache->HandsOffStorage();
    pPersistCache->Release();
  }
  return BOleSite::HandsOffStorage();
}


//**************************************************************************
//
// IBSite methods (override this interface on aggregated BOleIPSServer
// to implement Invalidate).
//
//**************************************************************************

HRESULT _IFUNC BOleInProcServer::Init(PIBDataProvider pDP,
  PIBPart pP, LPCOLESTR szProgId, BOOL fHatchWnd)
{
  // Initialize back pointers
  //
  pProvider = pDP;
  pPart = pP;

  CLSID cid = CLSID_NULL;

  if (SUCCEEDED(CLSIDFromProgID(szProgId, &cid))) {
    // create the default handler
    //
    CreateDataCache(AsPIUnknown(pObjOuter),cid, IID_IUnknown,
      &(LPVOID)pDefHandler);
  }

  // initialize default IPS
  //
  return BOleSite::Init(pDP, pP, szProgId, fHatchWnd);
}

void _IFUNC BOleInProcServer::Invalidate( BOleInvalidate b)
{
  if ((b & BOLE_INVAL_VIEW) && pAdviseView) {
    pAdviseView->OnViewChange(dwAdviseAspects, -1);
    if (dwAdviseFlags == ADVF_ONLYONCE) {
      pAdviseView->Release();
      pAdviseView = NULL;
      dwAdviseAspects = 0L;
      dwAdviseFlags = 0L;
    }
  }
  if (b)  // VIEW or DATA or PERSIST remains invalid
    BOleSite::Invalidate(b);
}

//**************************************************************************
//
// IOleInPlaceActiveObject
//
// This interface is overridden to pre-process translator and other messages
// from the container's message loop.
//
//**************************************************************************


//  Allow DLL Server to translate accelerators
//
HRESULT _IFUNC BOleInProcServer::TranslateAccelerator (LPMSG lpmsg)
{
  return pService->GetApplication()->Accelerator(lpmsg);
}

HRESULT _IFUNC BOleInProcServer::EnumVerbs(IEnumOLEVERB* FAR* ppenumOleVerb)
{
  return OleRegEnumVerbs(cid, ppenumOleVerb);
}

HRESULT _IFUNC BOleInProcServer::GetUserType(DWORD dwFormOfType, LPOLESTR FAR* pszUserType)
{
  return OleRegGetUserType(cid, dwFormOfType, pszUserType);
}

HRESULT _IFUNC BOleInProcServer::GetMiscStatus(DWORD dwAspect, DWORD FAR* pdwStatus)
{
  return OleRegGetMiscStatus(cid, dwAspect, pdwStatus);
}

HRESULT _IFUNC BOleInProcServer::SetExtent (DWORD dwDrawAspect, LPSIZEL lpsizel)
{
  HRESULT hr = BOleSite::SetExtent(dwDrawAspect, lpsizel);
  return hr;
}

// IRunnableObject methods
//
HRESULT _IFUNC BOleInProcServer::GetRunningClass(LPCLSID lpClassID)
{
  *lpClassID = cid;
  return NOERROR;
}

HRESULT _IFUNC BOleInProcServer::Run(LPBINDCTX pbc)
{
  // Enter running mode by setting a flag and locking the container
  //
  if (!fRunningMode) {
    fRunningMode = TRUE;
    // Should register the moniker here.
    // Don't register if this is a clone (pPart is NULL)
    // otherwise we would link to what's on the clipboard...
    //
    if (pPart)
      SetMoniker(OLEWHICHMK_OBJFULL, NULLP);    // register
  }

  return NOERROR;
}

BOOL _IFUNC BOleInProcServer::IsRunning()
{
  return fRunningMode;
}

HRESULT _IFUNC BOleInProcServer::LockRunning(BOOL fLock, BOOL fLastCloses)
{
  return Lock(fLock, fLastCloses);
}

HRESULT _IFUNC BOleInProcServer::SetContainedObject(BOOL fContained)
{
  return NOERROR;
}

// IExternalConnection implementation
//
DWORD _IFUNC BOleInProcServer::AddConnection (DWORD flag, DWORD reserved)
{
   return (flag & EXTCONN_STRONG) ? ++exCon : 0;
}

DWORD _IFUNC BOleInProcServer::ReleaseConnection (DWORD flag, DWORD reserved, BOOL fLastReleaseCloses)
{
   if (flag & EXTCONN_STRONG) {
     --exCon;
     if( exCon == 0 && fLastReleaseCloses){
      Close(OLECLOSE_SAVEIFDIRTY);
     }
     return exCon;
   }
  return 0;
}

HRESULT _IFUNC BOleInProcServer::Close (DWORD dwSaveOption)
{
  fRunningMode = FALSE;
  return BOleSite::Close(dwSaveOption);
}

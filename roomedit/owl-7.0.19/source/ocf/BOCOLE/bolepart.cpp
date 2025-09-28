//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 client site. BOlePart objects
//  impersonate the server object (IPart) from the point of view of the
//  Bolero customer who's writing a client site (ISite)
//----------------------------------------------------------------------------
#include "bolesvc.h"
#include "boledoc.h"
#include "bolepart.h"
#include "boledata.h"
#include "bolecman.h"

#include "ole2ui.h"
#include "utils.h"


// MulDiv uses int; won't work with HIMETRIC values
//
inline int Scale(double src, long mult, long div)
{
  src *= mult;
  src += (div/2);
  src /= div;
  return (int) src;
}

const OLECHAR szcBOlePart[] = OLESTR ("\3BOlePart");

// Since we don't know why Excel returns RPC_E_CALL_REJECTED the
// first N times and then finally works, we have this kludge.
//
HRESULT ForceOleRun(IOleObject *pOO)
{
  int timeout = 500;
  while (timeout-- > 0) {
    if (ResultFromScode(RPC_E_CALL_REJECTED) != OleRun(pOO))
      break;
  }
  return (timeout == 0) ? ResultFromScode(RPC_E_CALL_REJECTED) : NOERROR;
}


HRESULT _IFUNC BOlePart::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // Self
  //
  if (iid == IID_BOlePart) {
    (BOlePart*) *ppv = this;
    AddRef ();
    return NOERROR;
  }

  // interfaces

     SUCCEEDED(hr = IOleInPlaceSite_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IOleClientSite_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IAdviseSink_QueryInterface(this, iid, ppv))
 //       || SUCCEEDED(hr = IBPart_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IBPart2_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IBLinkable_QueryInterface(this, iid, ppv))
#ifdef PART_DATA
  || SUCCEEDED(hr = IPersist_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IDataObject_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IEnumFORMATETC_QueryInterface(this, iid, ppv))
#endif
  || SUCCEEDED(hr = IDropSource_QueryInterface(this, iid, ppv))
  || (IsLink && SUCCEEDED(hr = IBLinkInfo_QueryInterface(this, iid, ppv)))

  // base classes

  || SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))

  ;

  return hr;
};

//**************************************************************************
//
// BOlePart implementation -- functions which aren't part of any inherited
//                            interface, but are just part of the private
//                            implementation
//
//**************************************************************************

BOlePart::BOlePart (BOleClassManager *pF, IBUnknownMain * pO,
                                                BOleDocument * pOD) :
  BOleComponent(pF, pO),
  pDoc (pOD),             pOleObject(NULLP),   pViewObject(NULLP),
  pInPlaceObject(NULLP),  pSite (NULLP),       pVerbEnumerator(NULLP),
  verbIndex(0),           dwDrawAspect (0),    IsOpen(FALSE),
  HasMoniker(FALSE),      IsOpenInPlace(FALSE),IsOpenInsideOut(FALSE),
  IsLink (FALSE),         pNextPart(NULLP),    pPrevPart (NULLP),
  pLinkObject (NULLP),    dwExtAspect(0),      pStgFromInit(NULLP),
  pDataObject (NULLP),    pEnumFE (NULLP),     MonikerAssigned(FALSE),
  pILinkCont (0),         pszInstName (NULLP), pShortName (NULL),
  pLongName (NULL),       pAppName (NULL),     fLinkSrcAvail(TRUE),
  pDocument (NULL)
{
  if (pDoc)
    pDoc->AddRefMain();

  oleVerb.lpszVerbName = NULLP;
  mfpIcon = NULL;
  CachedExtent.cx = CachedExtent.cy = 0;
  scaleSite.xN = 1L;
  scaleSite.xD = 1L;
  scaleSite.yN = 1L;
  scaleSite.yD = 1L;
  OLEPRINTF5("Initializing scaleSite:\t {%ld, %ld, %ld, %ld}",
                scaleSite.xN, scaleSite.xD, scaleSite.yN, scaleSite.yD);
}


BOlePart::~BOlePart()
{
  Close();  // just to be sure

  if (pszInstName) {
    delete [] pszInstName;
    pszInstName = NULLP;
  }

  if (pDocument) {        // set in BOleContainer::GetObject
    CoLockObjectExternal(pDocument, FALSE, FALSE);
    pDocument = NULL;
  }
  DeleteTypeNames();
}

void BOlePart::DeleteTypeNames ()
{
    // Delete any type names which were allocated in IOleObject::GetUserType
  //
  if (pLongName || pShortName || pAppName) {
    LPMALLOC pMalloc = NULL;
    HRESULT hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
    if (SUCCEEDED(hr)) {
      if (pShortName) {
        if (pMalloc->DidAlloc(pShortName))
          pMalloc->Free (pShortName);
        pShortName = NULLP;
      }
      if (pAppName) {
        if (pMalloc->DidAlloc(pAppName))
          pMalloc->Free (pAppName);
        pAppName = NULLP;
      }
      if (pLongName) {
        if (pMalloc->DidAlloc(pLongName))
          pMalloc->Free (pLongName);
        pLongName = NULLP;
      }
      pMalloc->Release ();
    }
  }
}

// Close -- Let go of all our connections to the server object
//
HRESULT _IFUNC BOlePart::Close()
{
  // We should let go of all server resources to allow the server to quit.
  //
  HRESULT hRes = NOERROR;

  RemoveFromList ();

  // Ask the server to close its document and exit its program
  if (pOleObject) {
    hRes = pOleObject->Close(OLECLOSE_SAVEIFDIRTY);
    IsOpen = FALSE;
  }

  if (pOleObject) {
    pOleObject->Release();
    pOleObject = NULL;
  }
  if (pViewObject) {
    pViewObject->Release();
    pViewObject = NULL;
  }
  if (pDataObject) {
    pDataObject->Release();
    pDataObject = NULL;
  }
  if (pEnumFE) {
    pEnumFE = NULL;
  }
  if (pDoc) {
    pDoc->ReleaseMain();  // dec count on helper
    pDoc = NULL;
  }
  if (pLinkObject) {
    pLinkObject->Release();
    pLinkObject = NULL;
  }
  if (pStgFromInit) {
    pStgFromInit->Release ();
    pStgFromInit = NULL;
  }

  // Required for AVI media stuff to release properly.
  // And DLL servers
  //
  CoFreeUnusedLibraries();

  return hRes;
}

BOOL _IFUNC BOlePart::InitAdvises(LPOLEOBJECT lpOleObject,
          DWORD dwDrawAspect,
          LPOLESTR lpszContainerApp,
          LPOLESTR lpszContainerObj,
          LPADVISESINK lpAdviseSink,
          BOOL fCreate)
{
  LPVIEWOBJECT lpViewObject;
  HRESULT hrErr;
  BOOL fStatus = TRUE;
#if defined( SPECIAL_CONTAINER )
  DWORD dwTemp;
#endif

  hrErr = lpOleObject->QueryInterface (IID_IViewObject,
    (LPVOID FAR*)&lpViewObject);

  // Setup View advise
  //
  if (hrErr == NOERROR) {

    lpViewObject->SetAdvise(dwDrawAspect, (fCreate ? ADVF_PRIMEFIRST : 0),
      lpAdviseSink);

    lpViewObject->Release();
  } else
    fStatus = FALSE;

#if defined( SPECIAL_CONTAINER )
  /* Setup OLE advise.
  **    OLE2NOTE: normally containers do NOT need to setup an OLE
  **    advise. this advise connection is only useful for the OLE's
  **    DefHandler and the OleLink object implementation. some
  **    special container's might need to setup this advise for
  **    programatic reasons.
  **
  **    NOTE: this advise will be torn down automatically by the
  **    server when we release the object, therefore we do not need
  **    to store the connection id.
  */
  hrErr = lpOleObject->Advise(lpAdviseSink, (DWORD FAR*)&dwTemp);
  if (hrErr != NOERROR)
    fStatus = FALSE;
#endif

  // Setup the host names for the OLE object.
  //
  hrErr = lpOleObject->SetHostNames(lpszContainerApp,lpszContainerObj);
  if (hrErr != NOERROR)
    fStatus = FALSE;

  // Inform the loadded object's handler/inproc-server that it is in
  //    its embedding container's process.
  //
  OleSetContainedObject(lpOleObject, TRUE);

  return fStatus;
}

#ifdef __BORLANDC__
#pragma option -Od
#endif

HRESULT BOlePart::CreateFromBOleInitInfo(BOleInitInfo *pBI, BOOL &fShowAfterCreate)
{
  HRESULT hRes = ResultFromScode(E_INVALIDARG);
  // another result code which can fail without causing Init to fail
  HRESULT hrOkToFail;

  BOOL fIcon = pBI->hIcon != NULL;
  DWORD dwRenderOpt = fIcon ? OLERENDER_NONE : OLERENDER_DRAW;

  dwDrawAspect = fIcon ? DVASPECT_ICON : DVASPECT_CONTENT;

  switch (pBI->Where) {
    case BOLE_STORAGE: {

      // Load the OLE object's display aspect out of the stream we
      // dumped it in. This is all we need to do in order to make
      // drawing aspects persistent
      //
      LPSTREAM pStream;
      if (SUCCEEDED (pStgFromInit->OpenStream (szcBOlePart, NULL,
        STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream)))  {
        ULONG cb;
        pStream->Read (&dwDrawAspect, sizeof(dwDrawAspect), &cb);
        if (dwDrawAspect == DVASPECT_ICON) {
          fIcon = TRUE;
          dwRenderOpt = OLERENDER_NONE;
        }
        if (!fIcon) {
          pStream->Read (&scaleSite, sizeof(scaleSite), &cb);
          OLEPRINTF5("Reading scaleSite:\t {%ld, %ld, %ld, %ld}",
            scaleSite.xN, scaleSite.xD, scaleSite.yN, scaleSite.yD);
          SIZE size;
          size.cx = scaleSite.xD;     // server's extent's in Denom
          size.cy = scaleSite.yD;

          if (!(size.cx == 1 && size.cy == 1))
            // if scale factor exists,
            // force the server to restore the extent from last time.

            // because Word doesn't put height back to what
            // it was before. (they only load the width??)
            SetPartSize(&size);
        }
        pStream->Release ();
      }
      // Load OLE object from container's file
      //
      hRes = OLE::OleLoad (pStgFromInit, IID_IOleObject, this, &(LPVOID)pOleObject);
      if (hRes != NOERROR)
        break;

      // Don't activate OLE objects when loading container's file. If there
      // are a bunch of objects, or if the server is a big app, performance
      // would be severely degraded.
      //

      DWORD status = 0;
      fShowAfterCreate = FALSE;
      IsOpenInsideOut = FALSE;
      hrOkToFail = pOleObject->GetMiscStatus (dwDrawAspect, &status);
      if (hrOkToFail == NOERROR) {
        IsOpenInsideOut = (status & (OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE)) ?
            TRUE : FALSE;
        fShowAfterCreate = (status & (OLEMISC_ACTIVATEWHENVISIBLE)) ?
            TRUE : FALSE;
      }
      break;
    }

    case BOLE_NEW_OCX:
    case BOLE_NEW:
      // Create the object and set its parent
      hRes = BOleCreate (pBI, dwRenderOpt, &(LPVOID) pOleObject);
      if (hRes != NOERROR)
        break;

      // Find out whether the object is inside out.
      if (dwDrawAspect == DVASPECT_CONTENT) {
        DWORD status = 0;

        // Cl2test is passing 0 and ignoring the
        // error from Shapes. If I use the real aspect, outline returns
        // that it's an inside-out object. Crazy.
        //
        hrOkToFail = pOleObject->GetMiscStatus (dwDrawAspect, &status);
        if (hrOkToFail != NOERROR)
          IsOpenInsideOut = FALSE;
        else
          IsOpenInsideOut = (status & (OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE)) ?
            TRUE : FALSE;
      }

      break;

    case BOLE_FILE:

      if (pBI->How == BOLE_EMBED) {
        hRes = OLE::OleCreateFromFile(CLSID_NULL, pBI->whereFile.pPath, IID_IOleObject,
        dwRenderOpt, NULL, this, pStgFromInit, &(VOID FAR*)pOleObject);

        fShowAfterCreate = FALSE;
      }
      else {
        hRes = OLE::OleCreateLinkToFile (pBI->whereFile.pPath, IID_IOleObject,
          dwRenderOpt, NULL, this, pStgFromInit, &(VOID FAR*)pOleObject);

        // User should double-click to open link source. Showing the
        // object here would bring up the server out-of-place, which
        // would obscure the link we just created.
        //
        fShowAfterCreate = FALSE;
      }
      break;

    case BOLE_DATAOBJECT:

      if (pBI->How == BOLE_EMBED) {
        hRes = OLE::OleCreateFromData (pBI->whereData.pData, IID_IOleObject,
          OLERENDER_DRAW, NULL, this, pStgFromInit, &(VOID FAR*)pOleObject);

        // This is the result of a drop or a paste special. The UI
        // convention is not to inplace activate the server
        //
        fShowAfterCreate = FALSE;
      }
      else if (pBI->How == BOLE_LINK) {

        hRes = OLE::OleCreateLinkFromData (pBI->whereData.pData, IID_IOleObject,
          OLERENDER_DRAW, NULL, this, pStgFromInit, &(VOID FAR*)pOleObject);

        // User should double-click to open link source. Showing the
        // object here would bring up the server out-of-place, which
        // would obscure the link we just created.
        //
        fShowAfterCreate = FALSE;

      }
      else {
        hRes = OleCreateStaticFromData (pBI->whereData.pData, IID_IOleObject,
          OLERENDER_DRAW, NULL, this, pStgFromInit, &(LPVOID) pOleObject);
        fShowAfterCreate = FALSE;
      }


      // Get the drawing aspect for the object.
      //
      STGMEDIUM stgmed;
      dwDrawAspect = fIcon ? DVASPECT_ICON : DVASPECT_CONTENT;
      HGLOBAL memhandle = pDoc->pService->GetDataFromDataObject (
        pBI->whereData.pData, BOleDocument::oleObjectDescFmt, NULL,
        DVASPECT_CONTENT, &stgmed);
      if (memhandle) {
        LPOBJECTDESCRIPTOR objDesc = (LPOBJECTDESCRIPTOR) WIN::GlobalLock (memhandle);
        if (objDesc->dwDrawAspect == DVASPECT_ICON)
          dwDrawAspect = DVASPECT_ICON;
        WIN::GlobalUnlock (memhandle);
        OLE::ReleaseStgMedium (&stgmed);
      }

      // Find out whether the object is inside out.
      //
      if (hRes == NOERROR)
        if (dwDrawAspect == DVASPECT_CONTENT) {
          DWORD status;
          pOleObject->GetMiscStatus (dwDrawAspect, &status);
          IsOpenInsideOut = (status & (OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE)) ?
            TRUE : FALSE;
        }
      break;
  }
  if (SUCCEEDED(hRes) && !pOleObject)
    hRes = ResultFromScode(E_NOINTERFACE);

  return hRes;
}

HRESULT _IFUNC BOlePart::BOleCreate (BOleInitInfo *pBI, DWORD dwRenderOpt,
                                                      LPVOID FAR *ppv)
{
  return OLE::OleCreate (*(LPIID)pBI->whereNew.cid, IID_IOleObject,
        dwRenderOpt, NULL, this, pStgFromInit, &(LPVOID) pOleObject);
}


#ifdef __BORLANDC__
#pragma option -O.
#endif

// Init -- Create a new OLE2 server object. Probably the BOlePart helper
//   has just been created, and it's time to initiate OLE2 transactions
//
HRESULT _IFUNC BOlePart::Init(IBSite * pIS,BOleInitInfo *pBI)
{

#ifdef _DEBUG

  // Test to make sure client's QueryInterface is implemented correctly
  //
  IBSite *dbgSite = NULL;
  LPOLECLIENTSITE dbgCSite = NULL;
  if (  (!SUCCEEDED(pObjOuter->QueryInterfaceMain(IID_IBSite, &(LPVOID)dbgSite)))
    || (!SUCCEEDED(pObjOuter->QueryInterfaceMain(IID_IOleClientSite,&(LPVOID)dbgCSite)))) {
    MessageBox (NULL,
      TEXT ("QueryInterface Aggregation isn\'t working right"),
      TEXT ("BOlePart::Init"), MB_OK);
  }
  else {
    dbgSite->Release();
    dbgCSite->Release();
    dbgSite = NULL;
    dbgCSite = NULL;
  }

#endif


  pSite = pIS;

  if (pBI->pContainer) {
    BOleDocument *oldDoc = pDoc;
    if (!SUCCEEDED(pBI->pContainer->QueryInterface(IID_BOleDocument, &(LPVOID) pDoc)))
      return ResultFromScode (E_NOINTERFACE);
    pDoc->Release();  // PREVENT CYCLE

    // Since passing the container in the init info was added later, this
    // makes the reference counting semantic on our BOleDocument pointer
    // the same as is done in BOlePart::BOlePart and BOlePart::Close
    //
    pDoc->AddRefMain();       // AddRef the new doc from the init info
    if (oldDoc)
      oldDoc->ReleaseMain();  // Release the doc from the constructor

    pContainer = pBI->pContainer;
  }
  else
    //  Only work if the caller is not a container/server
    pContainer = pDoc->GetContainer();

  AddToList ();

  pStgFromInit = pBI->pStorage;
  pStgFromInit->AddRef();

  HRESULT hRes = ResultFromScode (E_INVALIDARG);

  BOOL fShowAfterCreate = TRUE;

  for (int idx=0; idx < 42; idx++) {    // knock 42 times if you want Excel
    if (ResultFromScode(RPC_E_CALL_REJECTED) !=
      (hRes = CreateFromBOleInitInfo(pBI, fShowAfterCreate)))
      break;
  }

  if (!SUCCEEDED(hRes))
    return hRes;

  // Do standard initialization on the freshly created object
  // where pOleObject is valid


  // If we have to draw as an icon, make sure the IOleCache in
  // handler for this OLE object has an iconic representation
  //
  // If we're loading from storage, just run the usual advises
  //
  if (dwDrawAspect == DVASPECT_ICON && pBI->Where != BOLE_STORAGE) {
#if 0
    if (pBI->Where == BOLE_FILE)
      mfpIcon = OleGetIconOfFile ((LPSTR)(pBI->whereFile.pPath), TRUE);
    else {
      OLECHAR buf[128] = {0,};
      mfpIcon = OleGetIconOfClass (*(LPIID)pBI->whereNew.cid, buf, TRUE);
    }
#endif
    mfpIcon = pBI->hIcon;
    CacheIconicAspect (mfpIcon);
    OnViewChange(dwDrawAspect, -1);
  }
  else {
    IBApplication * pApplication = pDoc->pApplication;
    InitAdvises (pOleObject, dwDrawAspect,
      (LPOLESTR)pApplication->GetAppName(),
      (LPOLESTR)pContainer->GetWindowTitle(), this, (pBI->Where != BOLE_STORAGE));
  // <eho> This not getting called causes our part size to be {0,0}!!
    OnViewChange(dwDrawAspect, -1);
  }

  if (fShowAfterCreate && pContainer->GetWindow() && hRes == NOERROR)
    hRes = DoVerb(OLEIVERB_SHOW);

  // Cache the view object interface for drawing later on
  //
  pOleObject->QueryInterface (IID_IViewObject, &(VOID FAR*)pViewObject);

  // Cache the data object interface for "embed from embedding"
  //
  pOleObject->QueryInterface (IID_IDataObject, &(VOID FAR*)pDataObject);

  // Cache the "user type name" for enumerating verbs later on
  //
  pOleObject->GetUserType (USERCLASSTYPE_SHORT, &pShortName);

  //      Ask the OLE object if it's a link, and cache the pointer if so
  //
  IsLink = SUCCEEDED(pOleObject->QueryInterface (IID_IOleLink, &(VOID FAR*) pLinkObject));

  DWORD status = 0;
  pOleObject->GetMiscStatus (dwDrawAspect, &status);
  if (status & (OLEMISC_ALWAYSRUN))
    OleRun (pOleObject);

  return hRes;
}

// Copies CF_EMBEDDEDOBJECT into a BOleData cache so we can do embed from
// embedding with a snapshot of the embedded object. This allows the user to
// change the original embedding and still paste the snapshot later.
//
HRESULT _IFUNC BOlePart::CopyFromOriginal (LPFORMATETC pFE, LPDATAOBJECT FAR*ppCopy)
{

  *ppCopy = NULL;

  // Get the original object's IDataObject and IPersistStorage interfaces
  // so we can write it into our cache. The server object must support
  // these interfaces.
  //
#if 1
  // We have to make sure that we DO need to know that this guy implements
  // IDataObject, otherwise, just use the other branch
  LPDATAOBJECT pOrig;
  HRESULT hr = pOleObject->QueryInterface (IID_IDataObject, &(LPVOID) pOrig);
  if (!SUCCEEDED(hr))
    return hr;
  LPPERSISTSTORAGE pPersistStg = NULL;
  hr = pOrig->QueryInterface (IID_IPersistStorage, &(LPVOID) pPersistStg);
  // JP: Now no longer use pOrig. Lets release it
  pOrig->Release();
  if (!SUCCEEDED(hr))
    return hr;
#else
  HRESULT hr = pOleObject->QueryInterface( IID_IPersistStorage, &(LPVOID) pPersistStg);
  if (!SUCCEEDED(hr))
    return hr;
#endif
  // Create the data cache object which will hold the snapshots in as many
  // formats as we need
  //
  LPUNKNOWN pObjDataCache = NULL;
  hr = pFactory->ComponentCreate ((LPUNKNOWN FAR*)&pObjDataCache, NULL, cidBOleData);
  if (!SUCCEEDED(hr))
    return hr;
  LPDATAOBJECT pIDataCache = NULL;
  hr = pObjDataCache->QueryInterface (IID_IDataObject, &(LPVOID)pIDataCache);
  pObjDataCache->Release(); //JP
  pObjDataCache = NULL;

  if (!SUCCEEDED(hr))
    return hr;

  // Create a temporary docfile which will hold the data
  //
  STGMEDIUM stm;
  memset (&stm, 0, sizeof(STGMEDIUM));
  stm.tymed = TYMED_ISTORAGE;
  hr = ::StgCreateDocfile (NULL, STGM_TRANSACTED | STGM_READWRITE |
    STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE, 0, &stm.pstg);
  if (!SUCCEEDED(hr))
    return hr;

  // Save the original OLE server object into our temporary docfile
  //
  if (pPersistStg->IsDirty () == NOERROR) {
    ::OleSave (pPersistStg, stm.pstg, FALSE);
    pPersistStg->SaveCompleted (NULL);
  }
  else
    pStgFromInit->CopyTo (0, NULL, NULL, stm.pstg);
  pPersistStg->Release ();

  FORMATETC fe;
  memset (&fe, 0, sizeof(FORMATETC));
  fe.cfFormat = BOleDocument::oleEmbdObjClipFmt;
  fe.ptd = NULL;
  fe.dwAspect = dwDrawAspect;
  fe.lindex = -1;
  fe.tymed = TYMED_ISTORAGE;

  hr = pIDataCache->SetData (&fe, &stm, TRUE);
  if (!SUCCEEDED(hr))
    return hr;

  stm.tymed = TYMED_HGLOBAL;
  fe.cfFormat = BOleDocument::oleObjectDescFmt;
  fe.tymed = TYMED_HGLOBAL;
  POINTL unused = {0,0};
  SIZEL unused2 = {0,0};
  stm.hGlobal = OleStdGetObjectDescriptorDataFromOleObject (pOleObject,
    NULL, dwDrawAspect, unused, &unused2);
  hr = pIDataCache->SetData (&fe, &stm, TRUE);
  if (!SUCCEEDED(hr))
    return hr;

  AddSiteData (pIDataCache);
  AddCachedData( pIDataCache );

  // Setup linking formats for linking to embedding (if possible)
  //
  DWORD dwStat=0L;
  pOleObject->GetMiscStatus (dwDrawAspect, &dwStat);
  if (!(OLEMISC_CANTLINKINSIDE & dwStat)) // (says can't do in registry)
    AddLinkSourceData (pIDataCache);

  *ppCopy = pIDataCache;
  return NOERROR;
}

HRESULT _IFUNC BOlePart::SwitchDisplayAspect(
    LPOLEOBJECT   lpOleObj,
    LPDWORD       lpdwCurAspect,
    DWORD         dwNewAspect,
    HGLOBAL       hMetaPict,
    BOOL          fDeleteOldAspect,
    BOOL          fSetupViewAdvise,
    LPADVISESINK  lpAdviseSink,
    BOOL FAR*     lpfMustUpdate,
    BOOL          fRunIfNecessary
)
{
  LPOLECACHE      lpOleCache = NULL;
  LPVIEWOBJECT    lpViewObj = NULL;
  LPENUMSTATDATA  lpEnumStatData = NULL;
  STATDATA        StatData;
  FORMATETC       FmtEtc;
  STGMEDIUM       Medium;
  DWORD           dwAdvf;
  DWORD           dwNewConnection;
  DWORD           dwOldAspect = *lpdwCurAspect;
  HRESULT         hrErr;

  // This fixes a bug in the sample code: insert a shape as an icon,
  // close sr2test, then convert the icon to content. They fail because
  // the object isn't running. We run it if we need to. The fRunIfNecessary
  // flag may be false if we're in SwitchDisplayAspect from Insert Object.
  // Obviously, then the object doesn't need to be run.
  //
  BOOL fMustClose = FALSE;
  if (fRunIfNecessary && !OLE::OleIsRunning(pOleObject)) {
    OleRun (pOleObject);
    fMustClose = TRUE;
  }

  if (lpfMustUpdate)
    *lpfMustUpdate = FALSE;

  lpOleObj->QueryInterface (IID_IOleCache, &(VOID FAR*) lpOleCache);

  // if IOleCache* is NOT available, do nothing
  //
  if (!lpOleCache)
    return ResultFromScode(E_INVALIDARG);

  // Setup new cache with the new aspect
  FmtEtc.cfFormat = NULL;     // whatever is needed to draw
  FmtEtc.ptd      = NULL;
  FmtEtc.dwAspect = dwNewAspect;
  FmtEtc.lindex   = -1;
  FmtEtc.tymed    = TYMED_NULL;

  // Icons shouldn't receive data advise connections. If they did, the
  // metafile representation of the icon in the OLE cache would be lost.
  //
  if (dwNewAspect == DVASPECT_ICON && hMetaPict)
    dwAdvf = ADVF_NODATA;
  else
    dwAdvf = ADVF_PRIMEFIRST;

  hrErr = lpOleCache->Cache(&FmtEtc, dwAdvf, &dwNewConnection);

  if (!SUCCEEDED(hrErr)) {
    lpOleCache->Release();
    return hrErr;
  }

  *lpdwCurAspect = dwNewAspect;

  // Stuff the icon into the cache
  //
  if (dwNewAspect == DVASPECT_ICON && hMetaPict) {

    FmtEtc.cfFormat = CF_METAFILEPICT;
    FmtEtc.ptd      = NULL;
    FmtEtc.dwAspect = DVASPECT_ICON;
    FmtEtc.lindex   = -1;
    FmtEtc.tymed    = TYMED_MFPICT;

    Medium.tymed      = TYMED_MFPICT;
    Medium.hGlobal    = hMetaPict;
    Medium.pUnkForRelease   = NULL;

    hrErr = lpOleCache->SetData (&FmtEtc, &Medium, FALSE /* fRelease */);
  } else {
    if (lpfMustUpdate)
      *lpfMustUpdate = TRUE;
  }

  if (fSetupViewAdvise && lpAdviseSink) {
    // re-establish the ViewAdvise connection
    //
    lpOleObj->QueryInterface (IID_IViewObject, &(VOID FAR*) lpViewObj);

    if (lpViewObj) {
      lpViewObj->SetAdvise(dwNewAspect, 0, lpAdviseSink);
      lpViewObj->Release();
    }
  }

  // Remove cached presentations for the ole display aspect
  //
  if (fDeleteOldAspect) {
    hrErr = lpOleCache->EnumCache(&lpEnumStatData);

    while(hrErr == NOERROR) {
      hrErr = lpEnumStatData->Next(1, &StatData, NULL);
      if (hrErr != NOERROR)
        break;        // DONE! no more caches.

      if (StatData.formatetc.dwAspect == dwOldAspect) {

        // Remove previous cache with old aspect
        //
        lpOleCache->Uncache(StatData.dwConnection);
      }
    }

    if (lpEnumStatData)
      lpEnumStatData->Release();
  }

  if (lpOleCache)
    lpOleCache->Release();

  if (fMustClose)
    pOleObject->Close (OLECLOSE_SAVEIFDIRTY);

  return NOERROR;
}

BOOL _IFUNC BOlePart::CacheIconicAspect (HGLOBAL mfpIcon)
{
  BOOL fMustUpdate;
  HRESULT hrErr = SwitchDisplayAspect (pOleObject, &dwDrawAspect,
    DVASPECT_ICON, mfpIcon, FALSE, TRUE, this, &fMustUpdate, FALSE);

  //if (hrErr == NOERROR)
    //hrErr = pOleObject->Update ();

  OleUIMetafilePictIconFree(mfpIcon);    // clean up metafile
  return (hrErr == NOERROR);
}

void BOlePart::AddToList ()
{
  // Insert "this" onto the list of part instances in this document.
  //
  // Avoid overhead by inserting the new part at the front of the list
  // rather than searching for a null pNextPart. So the list is in
  // first-in-last-out order
  //
  BOlePart *pOldFirstPart = pDoc->GetFirstPart ();
  pDoc->SetFirstPart (this);
  pNextPart = pOldFirstPart;
  if (pOldFirstPart)
    pOldFirstPart->pPrevPart = this;
}

void BOlePart::RemoveFromList ()
{
  // Remove "this" from the list of part instances in this document.
  //
  // Standard list removal by joining the previous part with the next one
  //
  if (pPrevPart)
    pPrevPart->pNextPart = pNextPart;
  if (pNextPart)
    pNextPart->pPrevPart = pPrevPart;

  if (pDoc)
    pDoc->OnRemovePart (this);

  pNextPart = pPrevPart = NULLP;
}

void BOlePart::SiteShow(BOOL show)
{
  BOOL doShow = show;
  DWORD status = 0;
  HRESULT hrOkToFail = pOleObject->GetMiscStatus (dwDrawAspect, &status);
  if (hrOkToFail == NOERROR) {
    if ((status & (OLEMISC_ACTIVATEWHENVISIBLE))
                        && IsOpenInPlace) // !DR try to avoid crash when 2 controls inserted
      doShow = FALSE;
  }
  pSite->SiteShow (doShow);
}

// ConvertHelper -- Helper function which is used in ConvertUI and
//      ConvertGuts. The reason to have this function is
//      because the user type and clsid must be known in
//      both UI and guts, but it wouldn't be nice to put
//      them in the BOleConvertInfo struct, which would
//      otherwise be the best way to get dynamic scope.
//
HRESULT _IFUNC BOlePart::ConvertHelper (LPCLSID pCid,
          LPOLESTR FAR* ppType,
          WORD FAR* pwFormat)
{
  BOOL fHaveClsid = FALSE;
  BOOL fHaveUserType = FALSE;
  HRESULT hr = NOERROR;

  //
  LPPERSISTSTORAGE pps = NULL;
  pOleObject->QueryInterface (IID_IPersistStorage, &(LPVOID) pps);
  pps->Save (pStgFromInit, TRUE);
  pps->Release();

  hr = ReadClassStg (pStgFromInit, pCid);
  if (SUCCEEDED(hr))
    fHaveClsid = TRUE;

  hr = ReadFmtUserTypeStg (pStgFromInit, pwFormat, ppType);
  if (SUCCEEDED(hr))
    fHaveUserType = TRUE;

  if (!fHaveClsid) {
    hr = pOleObject->GetUserClassID (pCid);
    if (!SUCCEEDED(hr))
      *pCid = CLSID_NULL;
  }

  if (!fHaveUserType) {
    OLECHAR szUserType[128];
    *pwFormat = 0;
  if (OleStdGetUserTypeOfClass(*pCid, szUserType, sizeof(szUserType), NULL))
  {
    //LPMALLOC pMalloc;
    //CoGetMalloc (MEMCTX_TASK, pMalloc); // get allocator
    //*ppType = pMalloc->Alloc(128 * sizeof (OLECHAR));
    //pMalloc->Release ();
    *ppType = new OLECHAR[128];
#if !defined UNICODE
    lstrcpyW2 (*ppType, szUserType);
#else
    lstrcpy(*ppType, szUserType);
#endif //UNICODE
    //*ppType = OleStdCopyString(szUserType, NULL);
  }
  else
  {
    *ppType = NULL;
    }
  }
  return hr;
}


// Bring up the Convert dialog box to give the user a chance to convert
// or emulate the OLE object associated with this part.
//
// Convert is actually implemented here because most of the data needed
// to do the conversion lives in the BOlePart not the BOleService
//
//
HRESULT _IFUNC BOlePart::ConvertUI (PIBApplication pHelpApp, BOOL f, BOleConvertInfo FAR *pInfo)
{
  HRESULT hr = NOERROR;

  memset (pInfo, 0, sizeof(BOleConvertInfo));

  OLEUICONVERT ouc;
  LPOLESTR DefLabel = NULL;
  LPOLESTR UserType = NULL;
  TCHAR    temp [255];

  memset (&ouc, 0, sizeof (ouc));
  ouc.cbStruct = sizeof (ouc);
  if (pDoc->pService->ShowHelpButton (BOLE_HELP_CONVERT))
    ouc.dwFlags = CF_SHOWHELPBUTTON;
  ouc.hWndOwner = ::GetActiveWindow();    // pHelpApp->GetWindow();
  ouc.fIsLinkedObject = IsLink;
  ouc.dvAspect = dwDrawAspect;

  if (IsLink) {
    hr = pLinkObject->GetSourceDisplayName (&DefLabel);
#if !defined(UNICODE) // MS_OLEUI_DEF
  WideCharToMultiByte (CP_ACP, 0, DefLabel, lstrlen(DefLabel),
            temp, 255, 0, 0);
  ouc.lpszDefLabel = temp;
#else
  ouc.lpszDefLabel = DefLabel;
#endif
  hr = pOleObject->GetUserClassID (&ouc.clsid);
  }
  else {
  ConvertHelper (&ouc.clsid, &UserType, &ouc.wFormat);
#if !defined(UNICODE)   // MS_OLEUI_DEF
    WideCharToMultiByte (CP_ACP, 0, UserType, -1 /*lstrlen(UserType)*/,
                      temp, 255, 0, 0);
    ouc.lpszUserType = temp;
#else
    ouc.lpszUserType = UserType;
#endif
  }

  STGMEDIUM medium;
  if (dwDrawAspect == DVASPECT_ICON)
    ouc.hMetaPict = OleStdGetData (pDataObject, CF_METAFILEPICT, NULL,
      DVASPECT_ICON, &medium);
  else
    ouc.hMetaPict = NULL;

  pDoc->pService->EnterBOleDialog (ouc.hWndOwner, NULL, NULL);

  UINT uRet = OleUIConvert (&ouc);
  pDoc->pService->ExitBOleDialog ();

  // If the user clicked OK, look at the dialog settings to see what
  // we should set in the output BOleConvertInfo.
  //
  if (uRet == OLEUI_OK) {

    if (ouc.dwFlags & CF_SELECTCONVERTTO && !IsEqualCLSID (ouc.clsid, ouc.clsidNew)) {
      (int) pInfo->action |= BOLE_CONVERT_TO_CLSID;
      pInfo->clsidNew = ouc.clsidNew;
    }

    if (ouc.dwFlags & CF_SELECTACTIVATEAS) {
      (int) pInfo->action |= BOLE_ACTIVATE_AS_CLSID;
      pInfo->clsidNew = ouc.clsidNew;
    }

    if (ouc.dvAspect != dwDrawAspect || ouc.fObjectsIconChanged) {
      if (ouc.dvAspect == DVASPECT_ICON) {
        (int) pInfo->action |= BOLE_ASPECT_TO_ICON;
        pInfo->hIcon = ouc.hMetaPict;
      }
      else
        (int) pInfo->action |= BOLE_ASPECT_TO_CONTENT;
    }
  }

  if (DefLabel) {
    LPMALLOC pMalloc;
    CoGetMalloc( MEMCTX_TASK, &pMalloc );
    pMalloc->Free (DefLabel);
    pMalloc->Release();
  }

  // If the user clicked Cancel or clicked OK, but didn't actually do
  // anything, we would get here with the action bitfield cleared.
  //
  // Return S_FALSE to indicate that the BOleConvertInfo is not valid,
  // and we shouldn't call ConvertGuts
  //
  if (pInfo->action == (BOleConvertAction) 0)
    hr = ResultFromScode (S_FALSE);
  else
    hr = ResultFromScode (S_OK);
  return hr;

}

HRESULT _IFUNC BOlePart::ConvertGuts (BOOL b, BOleConvertInfo FAR *pInfo)
{
  BOOL fMustActivate = FALSE;
  BOOL fMustRun = FALSE;
  BOOL fObjConverted = TRUE;

  HRESULT hr = NOERROR;

  // This could be a while
  //
  HCURSOR hPrevCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

  CLSID cid = CLSID_NULL;
  LPOLESTR pType = NULL;
  WORD wFormat = 0;
  ConvertHelper (&cid, &pType, &wFormat);

  // User selected Convert To
  //
  if (pInfo->action & BOLE_CONVERT_TO_CLSID) {
    LPSTORAGE pTmpStg = pStgFromInit; pTmpStg->AddRef();// will be released in Close
    BOleDocument *pTmpDoc = pDoc; pTmpDoc->AddRefMain();
    Close ();
    pDoc = pTmpDoc;
    pStgFromInit = pTmpStg;
    DeleteTypeNames();
    hr = OleStdDoConvert (pStgFromInit, pInfo->clsidNew);
    if (!SUCCEEDED (hr))
      goto error;
    hr = OLE::OleLoad (pStgFromInit, IID_IOleObject, this, &(LPVOID)pOleObject);
    fMustRun = TRUE;
    fObjConverted = TRUE;

    InitAdvises (pOleObject, dwDrawAspect,
      (LPOLESTR)pDoc->pApplication->GetAppName(),
      (LPOLESTR)pContainer->GetWindowTitle(), this, FALSE);

  }

  // User selected Activate As
  //
  if (pInfo->action & BOLE_ACTIVATE_AS_CLSID) {
    LPSTORAGE pTmpStg = pStgFromInit; pTmpStg->AddRef();// will be released in Close
    BOleDocument *pTmpDoc = pDoc; pTmpDoc->AddRefMain();
    Close ();
    pDoc = pTmpDoc;
    pStgFromInit = pTmpStg;
    DeleteTypeNames();

    pDoc->pService->UnloadObjects (cid);
    hr = OleStdDoTreatAsClass (pType, cid, pInfo->clsidNew);
    hr = OLE::OleLoad (pStgFromInit, IID_IOleObject, this, &(LPVOID)pOleObject);
    fMustActivate = TRUE;
  }

  // We'll want to draw this object later on
  //
  if (pOleObject) {
    if (pViewObject)
      pViewObject->Release();
    pOleObject->QueryInterface (IID_IViewObject, &(LPVOID) pViewObject);
    if (pDataObject)
      pDataObject->Release();
    pOleObject->QueryInterface (IID_IDataObject, &(VOID FAR*)pDataObject);
  }

  // If we're switching to icon, don't check the curr aspect because
  // we could be switching between icons.
  //
  if (pInfo->action & BOLE_ASPECT_TO_ICON) {

    // Special thing for scripting: if you ask for an object to be converted
    // to an icon from script/macro, you probably don't have an HMETAPICT
    // for the icon, so we let them pass 0 as a sentinel, and we use a
    // default icon for the CLSID
    //
    if (pInfo->hIcon == 0) {
      pInfo->hIcon = OleGetIconOfClass (cid, NULL, TRUE);
    }

    // Don't delete the old aspect if we're switching from one icon
    // to another. Otherwise, deleting the old aspect depends on whether
    // we're activating the object
    //
    BOOL fDeleteOld;
    if (dwDrawAspect == DVASPECT_ICON)
      fDeleteOld = FALSE;
    else
      fDeleteOld = !b;

    hr = SwitchDisplayAspect (pOleObject, &dwDrawAspect, DVASPECT_ICON,
      pInfo->hIcon, fDeleteOld, TRUE, this, &fMustRun, TRUE);
    if (SUCCEEDED(hr)) {
      OnViewChange(DVASPECT_ICON, -1);
    }
  }

  // Switching to content aspect
  //
  if (dwDrawAspect == DVASPECT_ICON && pInfo->action & BOLE_ASPECT_TO_CONTENT) {
    hr = SwitchDisplayAspect (pOleObject, &dwDrawAspect, DVASPECT_CONTENT,
      NULL, !b, TRUE, this, &fMustRun, TRUE);
    if (SUCCEEDED(hr)) {
      OnViewChange(DVASPECT_CONTENT, -1);
    }
  }

  if (fMustRun) {
    if (!fMustActivate && !OLE::OleIsRunning (pOleObject)) {
      hr = OleRun (pOleObject);
      if (!SUCCEEDED(hr))
        return hr;

      if (fObjConverted) {

        // Of course, static objects can't be run. This isn't an
        // error to be worried about
        //
        if (hr == ResultFromScode (OLE_E_STATIC)) {
          Close();
          pStgFromInit->Revert ();
          return hr;
        }
        else {
          // The object probably already has an OLERENDER_DRAW cache,
          // but this is just to make sure.
          //
          LPOLECACHE pCache = NULL;
          hr = pOleObject->QueryInterface (IID_IOleCache, &(LPVOID) pCache);
          if (SUCCEEDED(hr)) {
            FORMATETC fe;
            DWORD newCnxn;
            fe.cfFormat = NULL;
            fe.ptd = NULL;
            fe.dwAspect = DVASPECT_CONTENT;
            fe.lindex = -1;
            fe.tymed = TYMED_NULL;
            pCache->Cache (&fe, ADVF_PRIMEFIRST, &newCnxn);
            pCache->Release();
          }
        }
        // Force the storage to be committed
        //
        pOleObject->Close (OLECLOSE_SAVEIFDIRTY);

      }
    }
  }

  pSite->Invalidate(BOLE_INVAL_VIEW);
  if (fMustActivate)
    DoVerb (OLEIVERB_PRIMARY);
error:


  ::SetCursor (hPrevCursor);
  return hr;
}

void _IFUNC BOlePart::ResetObjectRects ()
{
  RECT pos, clip;
  OLEPRINTF1("Entering ResetObjectRects");
  if (NOERROR == pSite->GetSiteRect (&pos, &clip)) {
    OLEPRINTF3("GettingSiteRect:\t {%d, %d}",
           pos.right - pos.left, pos.bottom - pos.top);
    pInPlaceObject->SetObjectRects (&pos, &clip);
  }
}

//**************************************************************************
//
// IPart implementation
//
//**************************************************************************

HRESULT _IFUNC BOlePart::Save (IStorage * pStg, BOOL fSameAsLoad, BOOL fRemember)
{
  HRESULT ret = NOERROR;
  // Get a pointer to the Ole object's IPersistStorage
  if (pOleObject) {
    IPersistStorage* pPersistStg;
    HRESULT hr = pOleObject->QueryInterface(IID_IPersistStorage,
      &(VOID FAR *)pPersistStg);
    if (SUCCEEDED(hr) && !pPersistStg)
      ret = ResultFromScode (E_FAIL);
    else {

      // Write the object out to storage.
      //
      ret = OLE::OleSave(pPersistStg, pStg, fSameAsLoad);

      // Write the object's drawing aspect out to storage so we know
      // which presentation data to use when we reload it. OleSave
      // already knows how to save the iconic presentation data to storage
      //
      // Also write out the additional scale factor for the size
      // imposed when the user resized the size.
      //

      LPSTREAM pStream = NULLP;

      if (SUCCEEDED(pStg->CreateStream(szcBOlePart,
        STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
        NULL,NULL,&pStream)))  {
        ULONG cb;
        pStream->Write(&dwDrawAspect, sizeof(dwDrawAspect), &cb);
        if (!(dwDrawAspect == DVASPECT_ICON)) {
          OLEPRINTF5("Saving scaleSite:\t {%ld, %ld, %ld, %ld}",
                 scaleSite.xN, scaleSite.xD, scaleSite.yN, scaleSite.yD);
          pStream->Write(&scaleSite, sizeof(scaleSite), &cb);
        }
        pStream->Release();
      }

      if (fRemember) {
#if 0     // not used in outline example or mfc source
        // for SaveAs case release initial storage and remember new one
        pPersistStg->HandsOffStorage();  // server releases initial storage
#endif
        pPersistStg->SaveCompleted ( pStg );
        if (pStgFromInit)
          pStgFromInit->Release();
        pStgFromInit = pStg;
        pStgFromInit->AddRef();
      }
      else {
        pPersistStg->SaveCompleted ( NULL );
      }
      pPersistStg->Release();
    }

  }
  return ret;
}

HRESULT _IFUNC BOlePart::CanOpenInPlace ()
{
  // this is only used in our server code
  return ResultFromScode(E_FAIL);
}

HRESULT _IFUNC BOlePart::GetPalette (LPLOGPALETTE FAR *ppPal)
{
  if (pViewObject) {
    HRESULT hrErr = pViewObject->GetColorSet (
      dwDrawAspect, -1, NULL, NULL, NULL, ppPal);
    if (hrErr == NOERROR)
      return hrErr;
  }

  return ResultFromScode (E_FAIL);

}

HRESULT _IFUNC BOlePart::SetHost (IBContainer FAR *pNewContainer)
{
  if (!pNewContainer)
    return ResultFromScode (E_FAIL);

  BOleDocument *pNewDoc = NULL;
  if (!SUCCEEDED(pNewContainer->QueryInterface(IID_BOleDocument, &(LPVOID) pNewDoc)))
    return ResultFromScode (E_NOINTERFACE);
  pNewDoc->Release();    // PREVENT CYCLE

  if (IsOpenInPlace)
    return ResultFromScode (E_FAIL);

  if (pNewDoc->pActivePart)
    // Need to change pActivePart to a BOlePart* to
    // we can call Activate (FALSE) here.
    //
    return ResultFromScode (E_FAIL);


  RemoveFromList ();
  if (pDoc) {
    pDoc->ReleaseMain();
    pDoc = NULLP;
  }
  if (pNewDoc) {
    pDoc = pNewDoc;
    pDoc->AddRefMain();     // directly on helper
    pContainer = pNewContainer;
  }
  AddToList ();
  return NOERROR;
}

HRESULT _IFUNC BOlePart::Open (BOOL bOpen)
{
  if (bOpen) {
    HRESULT hRes = DoVerb(OLEIVERB_OPEN);
    pSite->Invalidate(BOLE_INVAL_VIEW); // ? do we need this (ICS)
    return hRes;
  }
  else {
    return pOleObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0,
      pContainer->GetWindow(), (LPRECT) 0);
  }
}

HRESULT _IFUNC BOlePart::Show (BOOL bShow)
{
  if (bShow) {
    HRESULT hRes = DoVerb(OLEIVERB_SHOW);
    pSite->Invalidate(BOLE_INVAL_VIEW); // ? do we need this (ICS)
    return hRes;
  }
  else {
    return pOleObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0,
      pContainer->GetWindow(), (LPRECT) 0);
  }

}


HRESULT _IFUNC BOlePart::Activate (BOOL fActivate)
{
  HRESULT hr = NOERROR;

  if (fActivate) {
    hr = DoVerb (OLEIVERB_PRIMARY);
  }
  else
    if (IsOpenInPlace) {
      hr = pInPlaceObject->UIDeactivate ();

#if 1
      if (hr == NOERROR)
        if (IsOpenInsideOut) {
          hr = pOleObject->DoVerb (OLEIVERB_DISCARDUNDOSTATE, NULL,
            this, 0, pContainer->GetWindow(), (LPRECT) 0);
          // if DISCARDUNDOSTATE is not implemented who cares!
          if (hr == ResultFromScode(E_NOTIMPL))
            hr = NOERROR;
        }
        else {
          if (pInPlaceObject)
            hr = pInPlaceObject->InPlaceDeactivate ();
        }
#endif
    }

  return hr;
}

HWND _IFUNC BOlePart::OpenInPlace (HWND hWndParent)
{
  // this is meaningful only in our server code
  return 0;
}

HRESULT _IFUNC BOlePart::SetPartSize (LPSIZE pSize)
{
  SIZEL sl;
  sl.cx = MAP_PIX_TO_LOGHIM (pSize->cx, BOleService::pixPerIn.x);
  sl.cy = MAP_PIX_TO_LOGHIM (pSize->cy, BOleService::pixPerIn.y);

  // Size passed is in scaled units but we need to tell the server
  // its "actual size"
  //
  BOleScaleFactor scale;
  pSite->GetZoom(&scale);
  sl.cx = Scale(sl.cx, scale.xD, scale.xN);    // unscale
  sl.cy = Scale(sl.cy, scale.yD, scale.yN);

  // IOleObject::SetExtent only works if the object is running.
  // If not, we need to cache the extent locally and pass it on to the
  // object when it opens.
  //
  if (!pOleObject || !OLE::OleIsRunning(pOleObject)) {

    // If the object's MISCSTATUS specifies that it wants to be run
    // whenever we resize it, do so here
    //
    DWORD miscStatus = 0;
    if (pOleObject &&
      (NOERROR == pOleObject->GetMiscStatus(dwDrawAspect, &miscStatus)))
      if (miscStatus & OLEMISC_RECOMPOSEONRESIZE)
        if (NOERROR == ForceOleRun (pOleObject))
          return pOleObject->SetExtent (dwDrawAspect, &sl);

    // Cache the size of the nonrunning object
    //
    CachedExtent = sl;
    return NOERROR;

  }

  // Set the size of the running object
  //
  HRESULT hr = pOleObject->SetExtent (dwDrawAspect, &sl);
  return hr;
}

HRESULT _IFUNC BOlePart::GetPartSize (LPSIZE pSize)
{
  if (!pSize)
    return ResultFromScode (E_INVALIDARG);

  SIZEL sl;

  // get himetric size and convert it to pixels
  // also adjust for scaling
  // we can give it to SetPartPos and Extent
  //
  HRESULT hr = pOleObject->GetExtent(dwDrawAspect, &sl);
  if (SUCCEEDED(hr)) {

    // If we've cached a modified extent for the object, inform the object
    // now that it's running.
    //
    if (CachedExtent.cx) {
      if ((sl.cx == CachedExtent.cx) && (sl.cy == CachedExtent.cy)) {
        CachedExtent.cx = 0L;
      }
      else {
        if (SUCCEEDED(pOleObject->SetExtent (dwDrawAspect, &CachedExtent))) {
          hr = pOleObject->GetExtent(dwDrawAspect, &sl);
          CachedExtent.cx = 0L;
        }
      }
    }


    pSize->cx = MAP_LOGHIM_TO_PIX (sl.cx, BOleService::pixPerIn.x);
    pSize->cy = MAP_LOGHIM_TO_PIX (sl.cy, BOleService::pixPerIn.y);

    BOleScaleFactor scale;
    pSite->GetZoom(&scale);
    pSize->cx = Scale(pSize->cx, scale.xN, scale.xD);
    pSize->cy = Scale(pSize->cy, scale.yN, scale.yD);
  }
  else {
    OLEPRINTF1("Entering GetPartSize failure case");
    // if the server isn't ready to return the size we should use the sites
    RECT rPos, rClip;
    pSite->GetSiteRect(&rPos, &rClip);
    OLEPRINTF3("GettingSiteRect:\t {%d, %d}",
           rPos.right - rPos.left, rPos.bottom - rPos.top);
    pSize->cx = rPos.right - rPos.left;
    pSize->cy = rPos.bottom - rPos.top;
  }
  return hr;
}

HRESULT _IFUNC BOlePart::SetPartPos (LPRECT pNewSize)
{
  HRESULT hr = ResultFromScode (E_FAIL);

  OLEPRINTF1("Entering SetPartPos");
  // If we're open in place, we can actually change the location of the
  // object's in-place window. Used for scrolling
  //
  if (IsOpenInPlace) {
    RECT pos, clip;
    if (NOERROR == pSite->GetSiteRect (&pos, &clip))  {
    hr = pInPlaceObject->SetObjectRects ((LPCRECT)pNewSize, &clip);
    OLEPRINTF3("SettingObjectRects:\t {%d, %d}",
       pNewSize->right - pNewSize->left,
       pNewSize->bottom - pNewSize->top);
    }
  }

  // If we're not open in place, we can only change the extent of the
  // object, and the rectangles should be right in the next IPart::Draw
  //
  else {
    SIZE s;
    s.cx = pNewSize->right - pNewSize->left;
    s.cy = pNewSize->bottom - pNewSize->top;
    SetPartSize(&s);
  }

  return hr;
}

inline BOOL IsMetafile(HDC hdc)
{
  return (DT_METAFILE == GetDeviceCaps(hdc,TECHNOLOGY));
}

HRESULT _IFUNC BOlePart::Draw (HDC hdc, LPCRECTL pPosRect, LPCRECTL pClipRect,
             BOleAspect reqAspect, BOleDraw bdFlags)
{
  int oldDCInfo = WIN::SaveDC (hdc);


  HRESULT hRes = ResultFromScode (E_FAIL);

  // If you're drawing objects, but not using the metafile to get it,
  // you'd get the high-res graphics data yourself, but you still need to
  // draw the open-editing shading. Hence this flag.
  //
  if (bdFlags & BOLE_DRAW_SHADINGONLY) {
    hRes = NOERROR;
    goto shadingOnly;
  }

  // If this object is a link, make sure we're bound to it just in case
  // someone else has opened the link source and makes changes to it. This
  // will force those changes to be propogated to us
  //
  if (IsLink) {
    DWORD linkopts = 0;
    if (SUCCEEDED(pLinkObject->GetUpdateOptions (&linkopts)))
      if (linkopts & OLEUPDATE_ALWAYS)
        pLinkObject->BindIfRunning();
  }

  // Remove the special Bolero aspect values from the bitfield
  // so those values don't show through when we send the aspect
  // to OLE2. OLE2 will return OLE_E_BLANK for non-standard values
  //
  dwExtAspect = 0; // Clear this out so caller can use different settings each time through
  if (reqAspect & BOLE_DEFAULT)
    dwExtAspect = dwExtAspect | BOLE_DEFAULT;
  else
    dwDrawAspect = reqAspect;
  if (reqAspect & BOLE_ASPECT_MAINTAIN)
    dwExtAspect = dwExtAspect | BOLE_ASPECT_MAINTAIN;
  if (reqAspect & BOLE_CLIP)
    dwExtAspect = dwExtAspect | BOLE_CLIP;

  reqAspect = (reqAspect & ~(BOLE_ASPECT_MAINTAIN | BOLE_DEFAULT | BOLE_CLIP));

  if (pViewObject) {
    RECTL hmRect = *pPosRect;

    // Get the part's extents in himetric
    SIZEL srvExt;
    HRESULT hr = pOleObject->GetExtent(dwDrawAspect, &srvExt);
    if (SUCCEEDED(hr)) {
      // scale part's extents by client's scale factor
      BOleScaleFactor scale;
      pSite->GetZoom(&scale);
      srvExt.cx = Scale(srvExt.cx, scale.xN, scale.xD);
      srvExt.cy = Scale(srvExt.cy, scale.yN, scale.yD);
    }
    else {
      OLEPRINTF1("Couldn't GetExtent during draw");
      // if server's extent isn't available, use what's passed in
      srvExt.cx = hmRect.right - hmRect.left;
      srvExt.cy = hmRect.bottom - hmRect.top;
    }

#if 1
    if (dwExtAspect & BOLE_ASPECT_MAINTAIN) {
      // Scale the rect we draw into if either the x extent or the y extent
      // overflows. The resulting rect is in the upper left corner of the client.
      SIZEL cliExt;
      cliExt.cx = hmRect.right - hmRect.left;
      cliExt.cy = hmRect.bottom - hmRect.top;

      //
      long dx, dy;
      dx = cliExt.cx - srvExt.cx;
      dy = cliExt.cy - srvExt.cy;
      if (dx >= 0 && dy >= 0) {
        // We have enough room, both x and y, to display the object
        hmRect.right = hmRect.left + srvExt.cx;
        hmRect.bottom = hmRect.top + srvExt.cy;
      }
      else {
        // Either the x or y extent is too large. Decide which axis overflows
        // more, and leave it the same size as the client, which is its
        // maximum value. Then scale the axis which overflows less (maybe not
        // at all) to maintain the server's aspect ratio
        //
        if (dx < dy)
          hmRect.bottom = hmRect.top + Scale(srvExt.cy, cliExt.cx, srvExt.cx);
        else
          hmRect.right = hmRect.left + Scale(srvExt.cx, cliExt.cy, srvExt.cy);
      }
    }
#endif

    if (dwExtAspect & BOLE_CLIP) {
      hmRect.right = hmRect.left + srvExt.cx;
      hmRect.bottom = hmRect.top + srvExt.cy;
    }
    // Ask the server object to draw itself.
    //
    hRes = pViewObject->Draw(dwDrawAspect, -1, NULL,  NULL,
      NULL, hdc, &hmRect, IsMetafile(hdc) ? pClipRect : NULL,
      NULL, 0);
  }
  else {
    WIN::RestoreDC (hdc, oldDCInfo);
    return ResultFromScode (E_FAIL);
  }

shadingOnly:

  RECT r;
  r.top = pPosRect->top;
  r.left = pPosRect->left;
  r.bottom = pPosRect->bottom;
  r.right = pPosRect->right;

  // If the server app has the embedded object open in its own window,
  // as opposed to inplace in our window, draw special highlighting
  //
  if (IsOpen) {
    OUI::OleUIDrawShading(&r, hdc, OLEUI_SHADE_FULLRECT, 0);
  }

  // If the app wants us to draw the borders around OLE2 objects as
  // described in the OLE2 UI guidelines, do it here.
  //
  if (bdFlags & BOLE_DRAW_SHOWOBJECTS) {
    int penStyle = IsLink ? PS_DASH : PS_SOLID;
    HPEN p = CreatePen (penStyle, 1, RGB(0,0,0));
    HBRUSH b = (HBRUSH) GetStockObject (HOLLOW_BRUSH);
    HPEN oldP = (HPEN) SelectObject (hdc,p);
    HBRUSH oldB = (HBRUSH) SelectObject (hdc,b);
    Rectangle (hdc, ( int )r.left, ( int )r.top,
      ( int )( r.right - r.left ),
      ( int )( r.bottom - r.top ) );
    SelectObject (hdc, oldP);
    SelectObject (hdc, oldB);
    DeleteObject (p);
    DeleteObject (b);
  }

  WIN::RestoreDC (hdc, oldDCInfo);
  return hRes;
}

HRESULT _IFUNC BOlePart::InsertMenus (HMENU, BOleMenuWidths*)
{
  // this is meaningful only in our server code
  return ResultFromScode (E_NOTIMPL);
}

void _IFUNC BOlePart::FrameResized (const RECT *, BOOL)
{
  // this is meaningful only in our server code
}

HRESULT _IFUNC BOlePart::ShowTools (BOOL)
{
  // this is meaningful only in our server code
   return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOlePart::EnumVerbs (BOleVerb *pBV)
{
  // The purpose of verbIndex is to keep track of the enumerator in
  // here rather than forcing the Bolero user to do it. It is assumed
  // that the Bolero user will call EnumVerbs until it fails, otherwise
  // the enumerator will be left in an incoherent state

  // The strategy for pVerbEnumerator is different than other interfaces.
  // Usually, we just get a pointer to it during IPart::Init and hang on
  // to the pointer until the Part is destroyed. In this case, the verb
  // enumerator could be made invalid by closing the server app. Then
  // the Next function returns RPC_E_SERVERDIED. It seems that OLE2, probably
  // in the default handler, provides another enumerator which can get
  // the verbs, probably from the registration database, when the
  // server isn't running. So we'll obtain and release the pointer
  // dynamically.

  // The lpszVerbName field in the OLEVERB is dynamically allocated during
  // IEnumVerb::Next. In order to prevent a memory leak, deallocate it here.
  // The DidAlloc is probably unnecessary, but it's a little protection
  // against the user passing in a pBV->verbName which is just a random
  // value off the stack.
  //
  if (pBV->verbName) {
    LPMALLOC pMalloc = NULL;
    HRESULT hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
    if (!SUCCEEDED(hr))
      return hr;
    if (!IsBadReadPtr ((LPVOID) pBV->verbName, 4) &&
      pMalloc->DidAlloc ((LPVOID) pBV->verbName)) {
      pMalloc->Free ((LPVOID) pBV->verbName);
      pBV->verbName = NULL;
    }
    pMalloc->Release ();
  }

  if (verbIndex == 0) {
    HRESULT hr = pOleObject->EnumVerbs(&pVerbEnumerator);

    if (hr != NOERROR)
      return hr;

    pVerbEnumerator->Reset ();
  }

  // Loop over verbs looking for non-negative verb numbers. The negative
  // ones are "aliases" for other verbs, and shouldn't be displayed
  //
  // The loop stops when Next returns an error, but loops over alias
  // verbs. EnumVerbs should be called in a loop by the user
  //
  SCODE sc;
  ULONG  temp = 0L;
  while (TRUE) {
    sc = GetScode(pVerbEnumerator->Next (1, &oleVerb, &temp));
    if (sc == S_OK) {
      if (oleVerb.lVerb > -1) {
        pBV->verbName = oleVerb.lpszVerbName;
        pBV->typeName = GetName (BOLE_NAME_SHORT);
        pBV->verbIndex = oleVerb.lVerb;
        pBV->fCanDirty = !(oleVerb.grfAttribs & OLEVERBATTRIB_NEVERDIRTIES);
        verbIndex++;
        return NOERROR;
      }
      else {
        // free the string coming from Next, if any
        if (oleVerb.lpszVerbName) {
          LPMALLOC pMalloc = NULL;
          HRESULT hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
          if (!SUCCEEDED(hr))
            return hr;
          if (!IsBadReadPtr ((LPVOID) oleVerb.lpszVerbName, 4) &&
            pMalloc->DidAlloc ((LPVOID) oleVerb.lpszVerbName)) {
            pMalloc->Free ((LPVOID) oleVerb.lpszVerbName);
          }
          pMalloc->Release ();
        }
      }
    }
    else {
      verbIndex = 0;
      pVerbEnumerator->Release ();
      return ResultFromScode (sc);
    }
  }
}

HRESULT _IFUNC BOlePart::DoVerb (UINT whichVerb)
{
  ForceOleRun(pOleObject);    // this line only added for Excel problems

  // Ask the container/app again for their names, in case something
  // is different from the last time we were open editing.
  //
  pOleObject->SetHostNames (pDoc->pApplication->GetAppName(),
    pDoc->pContainer->GetWindowTitle());

  RECT rect;
  pSite->GetSiteRect (&rect, 0);
  HRESULT hRes = pOleObject->DoVerb((LONG) (int)whichVerb, NULL, this, 0,
    pContainer->GetWindow(), &rect);

  if( !SUCCEEDED( hRes ) ){
    // This following (potentially big) block will deserve a helper function.
    // So far, only ONE error is implemented. Need to be completed.
    // Reference: outline/cntrline.c: ContainerLine_ProcessOleRunError()
    //
    SCODE error = GetScode( hRes );
    if ((error >= MK_E_FIRST && error <= MK_E_LAST) ||
       (error == OLE_E_CANT_BINDTOSOURCE)) {
      HandleLinkError();
      fLinkSrcAvail = FALSE;
    }
    if( error == REGDB_E_CLASSNOTREG ){
      // Server not registered (unexpected here?)
    }
    if( error == OLE_E_STATIC ){
      // A static object with no server registered
    }
    if( error == OLE_E_CLASSDIFF ){
      // Type of link has changed
    }
    if( ( error == CO_E_APPDIDNTREG )||( error == CO_E_APPNOTFOUND ) ){
      // Server not found
    }
    if( error == E_OUTOFMEMORY ){
      // Out of memory
    }
    return hRes;
  }

  HWND hWnd, hWndPrevParented;
  if (IsOpenInPlace && pInPlaceObject &&
    SUCCEEDED(pInPlaceObject->GetWindow(&hWnd)) &&
    SUCCEEDED(GetWindow(&hWndPrevParented))) {
    while (hWnd && GetParent(hWnd) != hWndPrevParented) {
      hWnd = GetParent(hWnd);  // make sure its the window they parented
    }
    if (hWnd && hWnd != GetTopWindow(GetParent(hWnd))) {
      SetWindowPos(hWnd, HWND_TOP, 0,0,0,0,   // MSWORD z-order workaround
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
      LONG l = GetWindowLong(hWnd, GWL_STYLE);  // visio paint problems
      SetWindowLong(hWnd, GWL_STYLE ,l | WS_CLIPSIBLINGS);  // visio paint problems
    }
  }

  return hRes;
}

HRESULT _IFUNC BOlePart::DragFeedback(POINT*,BOOL)
{
  return ResultFromScode (E_NOTIMPL);
}

UINT _IFUNC BOlePart::CountFormats ()
{
#if 0
  LPENUMFORMATETC enumFormatEtc;
  UINT n = 0;

  if( pDataObject && SUCCEEDED( pDataObject->EnumFormatEtc( DATADIR_GET, &enumFormatEtc ) ) ){
    FORMATETC fmt;
    while( enumFormatEtc->Next( 1, &fmt, 0 ) == NOERROR )
      n++;
    enumFormatEtc->Release();
  }
  return n;
#else
  return 0;
#endif
}

HRESULT _IFUNC BOlePart::GetFormat ( UINT i, BOleFormat *BOleFmt )
{
#if 0
  LPENUMFORMATETC enumFormatEtc;

  if( pDataObject && SUCCEEDED( pDataObject->EnumFormatEtc( DATADIR_GET, &enumFormatEtc ) ) ){
    FORMATETC fmt;
    while( enumFormatEtc->Next( 1, &fmt, 0 ) != NOERROR && i-- );
    memset( BOleFmt, 0, sizeof( *BOleFmt ) );
    BOleFmt->fmtId = fmt.cfFormat;
    BOleFmt->fmtMedium = ( BOleMedium )fmt.tymed;
    enumFormatEtc->Release();
  }
  return NOERROR;
#else
  return ResultFromScode (E_NOTIMPL);
#endif
}

HANDLE  _IFUNC BOlePart::GetFormatData (BOleFormat*pF)
{
  LPDATAOBJECT pDataObj = NULL;
  if (!pOleObject)
    assert (FALSE);

  HRESULT hr = pOleObject->QueryInterface (IID_IDataObject, &(void FAR*) pDataObj);

  if (!pDataObj || !SUCCEEDED(hr))
    return NULL;

  STGMEDIUM medium;

  // Just change it to WORD in boledefs.h please
  HANDLE h = pDoc->pService->GetDataFromDataObject (pDataObj,
    (CLIPFORMAT)pF->fmtId, NULL, dwDrawAspect, &medium);

  pDataObj->Release ();

  return h;
}

// Set data into the server
//
HRESULT  _IFUNC BOlePart::SetFormatData(BOleFormat* fmt, HANDLE hData, BOOL release)
{
  if (!fmt || !hData)
    return ResultFromScode(DATA_E_FORMATETC);

  LPDATAOBJECT pIData;
  HRESULT hr = pOleObject->QueryInterface (IID_IDataObject, &(LPVOID) pIData);
  if (!SUCCEEDED(hr))
    return hr;

  STGMEDIUM stm;
  memset (&stm, 0, sizeof(STGMEDIUM));
  stm.tymed = TYMED_ISTORAGE;

  FORMATETC fe;
  memset (&fe, 0, sizeof(FORMATETC));
  fe.cfFormat = fmt->fmtId;
  fe.ptd = NULL;
  fe.dwAspect = DVASPECT_CONTENT;
  fe.lindex = -1;
  fe.tymed = TYMED_HGLOBAL;

  return pIData->SetData (&fe, &stm, release);
}


LPOLESTR _IFUNC BOlePart::GetName (BOlePartName nameCode)
{
  // First see if we've already retrieved this name, and return
  // it without asking again
  //
  LPOLESTR FAR *ppName = NULL;
  switch (nameCode) {
    case BOLE_NAME_LONG  : ppName = &pLongName; break;
    case BOLE_NAME_SHORT : ppName = &pShortName; break;
    case BOLE_NAME_APP   : ppName = &pAppName; break;
    default        : return NULL;
  }

  if (*ppName)
    return *ppName;

  if (!pOleObject)
    return NULL;

  // Ask the OLE object for the name. GetUserType will allocate space
  // in our IMalloc, so we don't have to allocate it again. Of course,
  // we will have to free it later.
  //
  HRESULT hr = pOleObject->GetUserType (nameCode, ppName);

  if (!SUCCEEDED(hr) || !(*ppName))
    return NULL;

  return *ppName;
}

HRESULT _IFUNC BOlePart::DoQueryInterface(REFIID iid, void FAR* FAR* pif)
{       return (pOleObject) ?
    pOleObject->QueryInterface(iid, pif)
    : ResultFromScode(E_NOINTERFACE);
}

#undef  OLE_INTERFACE
/**************************************************************************/
#define OLE_INTERFACE "IOleClientSite"

HRESULT _IFUNC BOlePart::SaveObject ()
{
  if (!pStgFromInit)
    return ResultFromScode(E_NOTIMPL);

  return Save(pStgFromInit, TRUE, FALSE);
}

IMoniker * _IFUNC BOlePart::GetObjectMoniker(DWORD dwAssign)
{
  IMoniker* pMoniker = NULL;
  if (pszInstName) {

    switch (dwAssign) {
      case OLEGETMONIKER_FORCEASSIGN:
        OLE::CreateItemMoniker (OLESTR("!"), pszInstName, &pMoniker);
        if (pOleObject && !MonikerAssigned) {
          pOleObject->SetMoniker(OLEWHICHMK_OBJREL, pMoniker);
          MonikerAssigned = TRUE;
        }
        break;
      case OLEGETMONIKER_TEMPFORUSER:
        OLE::CreateItemMoniker (OLESTR("!"), pszInstName, &pMoniker);
        break;
      case OLEGETMONIKER_ONLYIFTHERE:
        if (MonikerAssigned) {
          OLE::CreateItemMoniker (OLESTR("!"), pszInstName, &pMoniker);
        }
        break;
      case OLEGETMONIKER_UNASSIGN:
        MonikerAssigned = FALSE;
        break;
    }
  }
  return pMoniker;
}

HRESULT _IFUNC BOlePart::GetMoniker (DWORD dwAssign, DWORD dwWhichMoniker, IMoniker* FAR* ppmk)
{
  *ppmk = NULL;
  if (pILinkCont) {
    switch (dwWhichMoniker) {

      // Identifies the document within the filesystem
      case OLEWHICHMK_CONTAINER:
        pILinkCont->GetMoniker(ppmk);
        break;

      // Identifies the object relative to the document
      case OLEWHICHMK_OBJREL:
        *ppmk = GetObjectMoniker(dwAssign);
        break;

      // Identifies the doc and object absolutely
      case OLEWHICHMK_OBJFULL:
        LPMONIKER pmkDoc;
        LPMONIKER pmkObj;
        if (SUCCEEDED(pILinkCont->GetMoniker(&pmkDoc))) {
          pmkObj = GetObjectMoniker(dwAssign);
          if (pmkObj) {
            CreateGenericComposite(pmkDoc, pmkObj, ppmk);
            pmkDoc->Release();
            pmkObj->Release();
          }
        }
        break;
    }
  }
  return ResultFromScode((*ppmk) ? S_OK : E_FAIL);
}

HRESULT _IFUNC BOlePart::GetContainer (IOleContainer* FAR* ppContainer)
{
  return pDoc->QueryInterface(IID_IOleContainer, (LPVOID*)ppContainer);
}

HRESULT _IFUNC BOlePart::ShowObject ()
{
  if (!IsOpenInPlace)
    SiteShow (TRUE);
  return NOERROR;
}

HRESULT _IFUNC BOlePart::OnShowWindow (BOOL fShow)
{
  // The server app calls OnShowWindow when it believes we should draw
  // or redraw the open-editing shading over the object. To cut down on
  // the noise, only Invalidate if the new state is different.
  //
  if (fShow != IsOpen) {
    IsOpen = fShow;
    pSite->Invalidate(BOLE_INVAL_VIEW);
  }
  return NOERROR;
}

HRESULT _IFUNC BOlePart::RequestNewObjectLayout ()
{
  // The OUTLINE test app says OLE doesn't use this function yet.
  // The OLE docs say it can be called if the server needs more or less
  //   room in the client, but that the client can ignore the request if
  //       the client wants to determine the amount of available real estate
  //
  return ResultFromScode (E_NOTIMPL);
}

//*************************************************************************
//
// ILinkInfo implementation
//
//*************************************************************************

HRESULT _IFUNC BOlePart::SourceBreak()
{
  if (!pLinkObject)
    return ResultFromScode (E_FAIL);

  // Break the link by setting its moniker to NULL
  //
  HRESULT hrErr = pLinkObject->SetSourceMoniker ((LPMONIKER)NULL, CLSID_NULL);
  if (SUCCEEDED(hrErr)) {
    IsLink = FALSE;
    pLinkObject->Release();
    pLinkObject = NULL;
  }

  // Release all the pointers we have on this OLE object in preparation
  // for creating a new object. We might also delete the part, but it seems
  // rude to do that while the user still has a refcount on us.
  //
  //Close ();

  // Create a static object in place of the current link object
  //
  IDataObject *dataObj = NULL;
  hrErr = pOleObject->QueryInterface (IID_IDataObject, &(VOID FAR*)dataObj);
  if (hrErr != NOERROR)
    return hrErr;

  FORMATETC fe;
  LPFORMATETC pFE;
  DWORD render;
  if (dwDrawAspect == DVASPECT_ICON) {
    // If the link we're breaking is displayed as an icon, we'll have to
    // display the static picture as content
    //
    fe.tymed = TYMED_NULL;
    fe.ptd = NULL;
    fe.lindex = -1;
    fe.dwAspect = DVASPECT_ICON;
    fe.cfFormat = 0;
    pFE = &fe;
//        dwDrawAspect = DVASPECT_CONTENT;
  }
  else {
    pFE = NULL;
  }

  render = OLERENDER_DRAW;

  pOleObject->Release();

  hrErr = OLE::OleCreateStaticFromData (dataObj, IID_IOleObject, render,
    pFE, this, pStgFromInit, &(LPVOID)pOleObject);

  if (dataObj) {
    dataObj->Release ();
    dataObj = NULL;
  }

  return NOERROR;
}

HRESULT _IFUNC BOlePart::SourceGet(LPOLESTR FAR szOut, UINT count)
{
  if (!pOleObject || !pLinkObject)
    return ResultFromScode (E_FAIL);

  LPOLESTR pDisplayName;
  DWORD len;


  HRESULT hr = pLinkObject->GetSourceDisplayName (&pDisplayName);
  if (SUCCEEDED (hr)) {
    lstrcpyn( szOut, pDisplayName, count );
    IMalloc *pMalloc;
    if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pMalloc))) {
      pMalloc->Free(pDisplayName);
      pMalloc->Release();
    }
  }
  return hr;
}

HRESULT _IFUNC BOlePart::SourceSet (LPOLESTR pNewSrc)
{
  if (!pLinkObject)
    return ResultFromScode (E_FAIL);

  HRESULT hrErr = NOERROR;
  LPMONIKER pMk = NULLP;
  CLSID clsid   = CLSID_NULL;
  LPBC bindCtx  = NULLP;


  // Create a new bind context to use for these monikers
  //
  hrErr = OLE::CreateBindCtx (0 /*unused*/, &bindCtx);
  if (hrErr != NOERROR)
    return hrErr;

  // Recursively resolve this textual representation into a binary moniker
  //
  DWORD chEaten = 0;
  hrErr = OLE::MkParseDisplayName (bindCtx, pNewSrc, &chEaten, &pMk);
  if (hrErr != NOERROR) {
    bindCtx->Release ();
    return hrErr;
  }

  // Obtain the CLSID of the object the moniker points to by using
  // a pointer to the IOleObject as a middleman
  //
  LPOLEOBJECT pObj = NULLP;
  hrErr = pMk->BindToObject (bindCtx, NULL, IID_IOleObject, &(VOID FAR*)pObj);
  if (hrErr == NOERROR) {
    hrErr = pObj->GetUserClassID (&clsid);
    pObj->Release();

    OnViewChange (dwDrawAspect, -1); // get new size if any
    fLinkSrcAvail = TRUE;
  }
  else {
    HandleLinkError();
    fLinkSrcAvail = FALSE;
  }

  // Make changes to the link object: if we were successful in getting a
  // moniker, use that, but fall back to the textual name if no moniker
  // was found.
  //
  if (pMk)
    hrErr = pLinkObject->SetSourceMoniker (pMk, clsid);
  else
    hrErr = pLinkObject->SetSourceDisplayName (pNewSrc);

  // Release scratch pointers (in reverse order)
  //
  if (pMk)
    pMk->Release ();
  if (bindCtx)
    bindCtx->Release ();

  // Now that the site is relinked to a new link source, we don't know
  // anything about the type of the new source. So delete the type
  // information we keep around. It will get allocated next time someone
  // asks for it.
  //
  DeleteTypeNames();

  if (hrErr == NOERROR)
    UpdateNow();

  if (dwDrawAspect == DVASPECT_ICON) {
    CacheIconicAspect(OleGetIconOfFile (pNewSrc, TRUE));
  }

  return hrErr;
}

HRESULT _IFUNC BOlePart::SourceOpen()
{
  if (!pOleObject)
    return ResultFromScode (E_FAIL);

  return DoVerb (OLEIVERB_SHOW);
}

HRESULT _IFUNC BOlePart::UpdateGet(BOleLinkUpdate *pOptions)
{
  if (!pLinkObject)
    return ResultFromScode (E_FAIL);

  // Use a temporary to avoid problems since BOleLinkUpdate is an enum
  // of some size (we don't want to rely on) and the update options
  // are a long
  //
  DWORD o = 0;
  HRESULT hrErr = pLinkObject->GetUpdateOptions (&o);
  *pOptions = (BOleLinkUpdate)o;

  return hrErr;
}

HRESULT _IFUNC BOlePart::UpdateSet(BOleLinkUpdate options)
{
  if (!pLinkObject)
    return ResultFromScode (E_FAIL);

  HRESULT hrErr = pLinkObject->SetUpdateOptions (options);
  return hrErr;
}

// Since we don't know why Excel returns RPC_E_CALL_REJECTED the
// first N times and then finally works, we have this kludge.
//
HRESULT ForceOleUpdate(IOleObject *pOO)
{
  int timeout = 500;
  while (timeout-- > 0) {
    if (ResultFromScode(RPC_E_CALL_REJECTED) != pOO->Update())
      break;
  }
  return (timeout == 0) ? ResultFromScode(RPC_E_CALL_REJECTED) : NOERROR;
}

HRESULT _IFUNC BOlePart::UpdateNow()
{
  if (!pOleObject)
    return ResultFromScode (E_FAIL);

  HRESULT hrErr = pOleObject->IsUpToDate ();
  if (hrErr != NOERROR)
    hrErr = ForceOleUpdate(pOleObject);

  return hrErr;
}

// This version of SourceGet is not part of the public bolero
// interface for now. It only exists to support IOleUILinkContainer::GetLinkSource
// Maybe we'll end up putting more stuff in the Bolero version ???
//
HRESULT _IFUNC BOlePart::SourceGet
(
  LPOLESTR FAR *lplpszDisplayName,
  LPOLESTR FAR *lplpszFullLinkType,
  LPOLESTR FAR *lplpszShortLinkType,
  DWORD FAR *lplenFileName
)
{
  if (!pOleObject || !pLinkObject)
    return ResultFromScode (E_FAIL);

  LPMONIKER pmk;
  HRESULT hrErr = pLinkObject->GetSourceMoniker (&pmk);
  if (hrErr == NOERROR) {
    pOleObject->GetUserType (USERCLASSTYPE_FULL, lplpszFullLinkType);
    pOleObject->GetUserType (USERCLASSTYPE_SHORT, lplpszShortLinkType);
    *lplenFileName = OleStdGetLenFilePrefixOfMoniker (pmk);
    pmk->Release ();
    hrErr = pLinkObject->GetSourceDisplayName (lplpszDisplayName);
  }
  return hrErr;
}

BOlePart * _IFUNC BOlePart::GetNextLink (BOOL fAllowThis)
{
  BOlePart *ret = fAllowThis ? this : this->pNextPart;
  while (ret) {
    if (ret->IsLink)
      return ret;
    ret = ret->pNextPart;
  }
  return NULLP;
}

//*************************************************************************
//
// IAdviseSink implementation
//
//*************************************************************************

void _IFUNC BOlePart::OnDataChange (FORMATETC FAR* pFormatetc,STGMEDIUM FAR* pStgmed)
{
  // I'm not sure how to respond to this, vs how to respond to OnViewChange.
  //  It seems like if our visible view onto the data hasn't changed, then
  //  we're not interested in OnDataChange. Maybe there's some significance
  //  for non-visible server objects...
  //
#ifdef _DEBUG
  MessageBox (NULL, TEXT ("BOLE.DLL"), TEXT ("Unexpected call to OnDataChange"), MB_OK);
#endif
  return;
}

void _IFUNC BOlePart::OnViewChange (DWORD changedAspect, LONG lindex)
{
  // The server may generate an OnViewChange when its extent changes,
  // so try to make the site the same size.
  //
  // This is really important for Paintbrush (all OLE1 servers?) whose
  // initial size is zero, and don't send OnPosRectChanged (because
  // they're never inplace). They only send OnViewChange, and expect
  // us to get the new extent.
  //
  // The changedAspect parameter tells us which aspect's view has
  // changed. It seems like we only care if the aspect we're currently
  // viewing is the same as the one which has changed.
  //

  if (dwDrawAspect == changedAspect) {

    // this can be called when we've resized the site, but not set
    // the part extent. In such a case the part extent is not equal to
    // the site extent.  The part is scaling by the ratio, scaleSite (set in
    // OnUIActivate).  So, we need to scale the part extent
    // and set our site size to that. If we were to just use the part's
    // extent, the symptom would be that upon deactivation the site
    // would return to the size it had before the (non-active) site
    // was resized, (or worse if an in-place active resize happened too).

    SIZE shortSize;

    GetPartSize(&shortSize);

    // adjust by additional scaling caused when resizing site while
    // not in place active (and not setting server extent).

    OLEPRINTF5("Adjusting by scaleSite:\t {%ld, %ld, %ld, %ld}",
      scaleSite.xN, scaleSite.xD, scaleSite.yN, scaleSite.yD);

    shortSize.cx = Scale(shortSize.cx, scaleSite.xN, scaleSite.xD);
    shortSize.cy = Scale(shortSize.cy, scaleSite.yN, scaleSite.yD);

    if (!SUCCEEDED(pSite->SetSiteExtent(&shortSize))) {

      // if client doesn't accept server's extent maybe the
      // server will accept the client's...
      //
      RECT rSite, rClip;
      HRESULT hr = pSite->GetSiteRect(&rSite, &rClip);
      if (SUCCEEDED(hr)) {
        SIZE sizeSite = {rSite.right - rSite.left, rSite.bottom - rSite.top};
        SetPartSize(&sizeSite);
      }
    } else {
      RECT posR, clipR;
      pSite->GetSiteRect (&posR, &clipR);
      if (pInPlaceObject) {
        posR.right  = posR.left + shortSize.cx;
        posR.bottom = posR.top  + shortSize.cy;
        // adjust posR because the site may not want to really
        // change the extent.  They could succeed to allow
        // the in-place window to be free, but when the user
        // deactivates go back to the same old site.
        pInPlaceObject->SetObjectRects (&posR, &clipR);
      }
    }

    // Generate a paint message for the client app's window, which will
    // eventually call back to our pViewObject->Draw
    //
    pSite->Invalidate(BOLE_INVAL_VIEW | BOLE_INVAL_DATA);
    UpdateSiteScaleFactor();
  }

  return;
}

void _IFUNC BOlePart::OnRename (IMoniker* pmk)
{
  // Significant for IOleLink objects which must track the link source
  //
  return;
}

void _IFUNC BOlePart::OnSave ()
{
  // Significant if the client implements IOleCache with ADVF_ONSAVE
  //
  return;
}

void _IFUNC BOlePart::OnClose ()
{
  // Significant for IOleLink objects when server is shutting down
  //
  return;
}


//**************************************************************************
//
// IOleWindow implementation
//
//**************************************************************************

HRESULT _IFUNC BOlePart::GetWindow (HWND FAR *phwnd)
{
  HRESULT hr = NOERROR;
  PIBSite2 pSite2 = NULL;
  *phwnd = NULL;

  hr = pObjOuter->QueryInterfaceMain (IID_IBSite2, &(LPVOID)pSite2);
  if (SUCCEEDED (hr)) {
    hr = pSite2->GetParentWindow (phwnd);
    pSite2->Release();
  }
  else {
    *phwnd = pContainer->GetWindow();
    hr = NOERROR;
  }

  return hr;
}

HRESULT _IFUNC BOlePart::ContextSensitiveHelp (BOOL fEnterMode)
{
  // The BOleService object is responsible for keeping track of Shift-F1
  //
  pDoc->pService->SetHelpMode (fEnterMode);

  return NOERROR;
}

//**************************************************************************
//
// IOleInPlaceSite Implementation
//
//**************************************************************************

HRESULT _IFUNC BOlePart::CanInPlaceActivate (void)
{
  HRESULT hr;
  // Objects rendered as an icon can only be edited "out of place"
  //
  if (dwDrawAspect == DVASPECT_ICON)
    OLERET(S_FALSE);

  return pContainer->AllowInPlace();
}

HRESULT _IFUNC BOlePart::OnInPlaceActivate (void)
{
  // In case we don't have balanced calls...
  if (pInPlaceObject) {
    pInPlaceObject->Release();
    pInPlaceObject = NULL;
  }
  // This is our opportunity to get a pointer to the object's
  // IOleInPlaceObject interface
  //
  HRESULT hr = pOleObject->QueryInterface(IID_IOleInPlaceObject, &(VOID FAR*)pInPlaceObject);

  if (hr == NOERROR) {
    IsOpenInPlace = TRUE;
    // If the object has opened in place site should hide themselves to
    // avoid strange drawing behavior
    //
    SiteShow (FALSE);
  }
  else {
    IsOpenInPlace = FALSE;
    pInPlaceObject = NULL;
  }

  return hr;
}

HRESULT _IFUNC BOlePart::OnInPlaceDeactivate (void)
{
  if (pInPlaceObject) {
    pInPlaceObject->Release();
    pInPlaceObject = NULL;
  }
  if (IsOpenInPlace) {
    IsOpenInPlace = FALSE;
    SiteShow (TRUE);    // put the site window back again
  }
  return NOERROR;
}

// Remember the additional scale factor for this site.
// If we've resized the site the scale factor is different than normal.
// We need to know it in OnViewChange, in case the server is resized
// when in-place active.
//
void BOlePart::UpdateSiteScaleFactor()
{
  RECT rPos, rClip;
  if (SUCCEEDED(pSite->GetSiteRect (&rPos, &rClip))) {

    SIZE sizePart;
    GetPartSize(&sizePart);

    if ((sizePart.cx == 0) || (sizePart.cy == 0)) {
      scaleSite.xN = scaleSite.xD = scaleSite.yN = scaleSite.yD = 1L;
    }
    else {
      scaleSite.xN = rPos.right - rPos.left;
      scaleSite.xD = sizePart.cx;
      scaleSite.yN = rPos.bottom - rPos.top;
      scaleSite.yD = sizePart.cy;
    }
    OLEPRINTF5("Updating scaleSite:\t {%ld, %ld, %ld, %ld}",
      scaleSite.xN, scaleSite.xD, scaleSite.yN, scaleSite.yD);
  }
}

HRESULT _IFUNC BOlePart::OnUIActivate (void)
{
  OLEPRINTF1("Entering OnUIActivate");

  // (Used to UpdateSiteScaleFactor in GetWindowContext.
  // But, MS Excel Chart doesn't give accurate extents at that point
  // In OnInPlaceActivate Excel Chart gives inaccurate extents.
  // In OnUIActivate Excel Sheet gives inaccurate extents.
  //
  UpdateSiteScaleFactor();

  // The inplace server is about to put up its own tool/status
  // bars, so ask the client app to remove its adorners
  //
  // Although it isn't in the official docs, the Outline example
  // (in frametls.c) uses the convention where SetBorderSpace({0,})
  // means hide adornments

  // hide their adornments
  //
  HRESULT hr = NOERROR;

  LPOLEINPLACESITE pIPS;
  if (SUCCEEDED(GetParentIPS(&pIPS))) {
    IBPart *pServerSide;
    if (SUCCEEDED(pDoc->QueryInterface(IID_IBPart, &(LPVOID)pServerSide))) {
      pServerSide->ShowTools(FALSE);
      pServerSide->Release();
    }
    pIPS->Release();
  }
  else {
    RECT r = {0,0,0,0};

    pContainer->BringToFront();

    IBWindow *pWnd = getNegotiator(pDoc->pApplication, pContainer);

//    hr = pWnd->SetBorderSpace (&r);
  }

  pSite->OnSetFocus(TRUE);

  // Hitting ESC from some servers only does a UIDeactivate so
  // next time you activate it, it only calls OnUIActivate rather
  // than OnInPlaceActivate. So be sure to keep the flags coherent with
  // the real state.
  //
  IsOpenInPlace = TRUE;
  SiteShow (FALSE); // assumes hiding an already hidden site is a nop

  return hr;
}

HRESULT _IFUNC BOlePart::OnUIDeactivate (BOOL fUndoable)
{
  // The inplace server is about to remove its own tool/status
  // bars, so ask the client app to restore its adorners
  //
  // Although it isn't in the official docs, the Outline example
  // (in frametls.c) uses the convention where SetBorderSpace(NULL)
  // means show adornments
  //

  HRESULT hr = NOERROR;

  if (IsOpenInPlace && !IsOpenInsideOut) {
    // when ESC key is used to deactivate excel and word
    // OnInPlaceDeactivate is not called.
    SiteShow (TRUE);    // put the site window back again
    IsOpenInPlace = FALSE;
  }

  // This SetFocus call is to take the focus back from the OLE object's
  // inplace window, since that window has gone away
  //
  pSite->OnSetFocus(TRUE);

  LPOLEINPLACESITE pIPS;
  if (SUCCEEDED(GetParentIPS(&pIPS))) {
    IBPart *pServerSide;
    if (SUCCEEDED(pDoc->QueryInterface(IID_IBPart, &(LPVOID)pServerSide))) {
      pServerSide->ShowTools(TRUE);
      pServerSide->Release();
    }
    pIPS->Release();
  }
  else {
    // show their adornments
    //
    IBWindow *pWnd = getNegotiator(pDoc->pApplication, pContainer);

    hr = pWnd->SetBorderSpace (NULL);

    // We're using this call to our own IOleInPlaceFrame implementation
    // to get to OleSetMenuDescriptor(NULL), which is supposed to unhook
    // OLE's message dispatching stuff. We could just call
    // OleSetMenuDescriptor from here, but it's not clear if the server
    // app might call SetMenu with NULL for the same purpose?
    //
    // The pointer-juggling here is intended to make sure we disconnect
    // the OLE message filter from the right window (frame vs. container).
    //
    IOleInPlaceFrame FAR *ipframe = NULL;
    if (S_OK == (GetScode(pDoc->pApplication->IsMDI())))
      ipframe = pDoc->pService;
    else
      ipframe = pDoc;
    PIBWindow ipNegWnd = getNegotiator (pDoc->pApplication, pDoc->pContainer);
    hr = ipframe->SetMenu (NULL, NULL, ipNegWnd->GetWindow());
  }

  return hr;
}

HRESULT _IFUNC BOlePart::GetParentIPS(LPOLEINPLACESITE *ppIPS)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  LPOLEOBJECT pSelfAsServer;
  if (SUCCEEDED(pDoc->QueryInterface(IID_IOleObject, &(LPVOID)pSelfAsServer))) {
    LPOLECLIENTSITE pCS;
    if (SUCCEEDED(pSelfAsServer->GetClientSite(&pCS)) && pCS) {
      hr = pCS->QueryInterface(IID_IOleInPlaceSite, (LPVOID*)ppIPS);
      pCS->Release();
    }
    pSelfAsServer->Release();
  }
  return hr;
}

HRESULT _IFUNC BOlePart::GetWindowContext (IOleInPlaceFrame* FAR* ppFrame, IOleInPlaceUIWindow* FAR* ppDoc, LPRECT prcPosRect, LPRECT prcVisRect, LPOLEINPLACEFRAMEINFO pFrameInfo)
{
  LPOLEINPLACESITE pIPSite;
  if (SUCCEEDED(GetParentIPS(&pIPSite)) && (S_OK == pIPSite->CanInPlaceActivate ()))
  {
    // if nested in place active,
    // get parent's window interfaces, but our site size
    //
    HRESULT hr = pIPSite->GetWindowContext(ppFrame, ppDoc, prcPosRect,
                 prcVisRect, pFrameInfo);
    pIPSite->Release();
    pSite->GetSiteRect (prcPosRect, prcVisRect);
    return hr;
  }

  // The isMdi flag tells the server whether it should negotiate for
  // space on the "frame" window or the "document" window
  //
  IBApplication * pApp = pDoc->pApplication;
  pFrameInfo->fMDIApp = (GetScode(pApp->IsMDI()) == S_OK) ? TRUE : FALSE;

  // Decide which window should do inplace negotiation
  //
  PIBWindow ipNegWnd = NULL;
  if (pFrameInfo->fMDIApp)
    ipNegWnd = pDoc->pApplication;
  else
    ipNegWnd = pContainer;

  // Decide which HWND to give out so toolbar parenting etc is correct
  //
  pFrameInfo->hwndFrame = ipNegWnd->GetWindow();

  // Allow the server to call back for menu negotiation at the document level
  //
  ipNegWnd->QueryInterface (IID_IOleInPlaceFrame, (void**) ppFrame);

  // Allow the server to call back for toolbar negotiation at the document level
  //
  if (pFrameInfo->fMDIApp)
    pDoc->QueryInterface (IID_IOleInPlaceUIWindow, (void**)ppDoc);
  else
    *ppDoc = NULL;

  // Get the accelerator table from the window
  //
  if (!SUCCEEDED(ipNegWnd->GetAccelerators (&pFrameInfo->haccel,
    (int FAR *) (&pFrameInfo->cAccelEntries)))) {
    pFrameInfo->haccel = NULL;
    pFrameInfo->cAccelEntries = 0;
  }

  pSite->GetSiteRect (prcPosRect, prcVisRect);

  return NOERROR;
}

HRESULT _IFUNC BOlePart::DiscardUndoState ()
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOlePart::DeactivateAndUndo ()
{
  // This is called by the server when the user picks Undo immediately
  // after activating the server inplace. We should remove the server's
  // adornments, set focus to ourselves, and pop the undo stack
  //
  HRESULT hr = pInPlaceObject->UIDeactivate();
  if (hr != NOERROR)
    return hr;

  pSite->OnSetFocus(TRUE);
  return NOERROR;
}

HRESULT _IFUNC BOlePart::Scroll (SIZE scrollExtent)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOlePart::OnPosRectChange (LPCRECT lprcPosRect)
{
  HRESULT hr;
  // The server size is changing, let's try to set it in the container..

  OLEPRINTF1("Entering OnPosRectChange");
  if (SUCCEEDED(pSite->SetSiteRect (lprcPosRect))) {
    RECT posR, clipR;
    pSite->GetSiteRect (&posR, &clipR);
    if (pInPlaceObject) {
      // use lprcPosRect because the site may not want to really
      // change the extent.  They could succeed to allow
      // the in-place window to be free, but when the user
      // deactivates go back to the same old site.
      hr = pInPlaceObject->SetObjectRects (lprcPosRect, &clipR);
    }
  }
  else {
    // try to back out of hatch window resize with the fewest possible
    // side effects.  This is best all around solution.  Although visio
    // scales inappropriately, one can live with it.

    RECT rSite, rClip;
    pSite->GetSiteRect(&rSite, &rClip);
    SIZE sizeSite = {rSite.right - rSite.left, rSite.bottom - rSite.top};

    SetPartSize(&sizeSite);
    pInPlaceObject->SetObjectRects (&rSite, &rClip);
    hr = ResultFromScode(E_FAIL);

#if 0
    // Here are the other trials:

    // this approach causes visio to scale inappropriately
    // and excel's extents to be wrong
    SetPartSize(&sizeSite);
    hr = pInPlaceObject->SetObjectRects (&rSite, &rClip);

    // w/o SetPartSize also excel sheet and chart scale inappropriately
    hr = pInPlaceObject->SetObjectRects (&rSite, &rClip);

    // same but chart in MS excel is a bit better
    pInPlaceObject->SetObjectRects (&rSite, &rClip);
    hr = ResultFromScode(E_FAIL);

    // this approach causes servers (chart, visio) to leave hatch window
    SetPartSize(&sizeSite);
    hr = NOERROR;

    // same but chart doesn't leave window, only visio
    SetPartSize(&sizeSite);
    hr = ResultFromScode(E_FAIL);

    // just failing causes both scale factor and window problems
    hr = ResultFromScode(E_FAIL);

    // doing the set part size after plays havoc with scaling
    pInPlaceObject->SetObjectRects (&rSite, &rClip);
    SetPartSize(&sizeSite);
    hr = ResultFromScode(E_FAIL);

#endif
  }
  return hr;
}

HRESULT _IFUNC BOlePart::GetMoniker(IMoniker FAR* FAR* ppMon)
{
  HRESULT hrErr = GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
    OLEWHICHMK_OBJFULL, ppMon);
  return hrErr;
}

HRESULT _IFUNC BOlePart::OnRename(PIBLinkable pCont, LPCOLESTR szName)
{
  HRESULT hrErr = NOERROR;

  pILinkCont = pCont;
  if (pszInstName) {
    delete pszInstName;
    pszInstName = NULL;
  }
  if (szName) {
    pszInstName = new OLECHAR [lstrlen (szName) +1];
    lstrcpy(pszInstName, szName);
  }

  if (pOleObject) {
    LPMONIKER pMon;
    // Server registers as GetObjectMoniker calls IOleObject::SetMoniker
    GetObjectMoniker(OLEGETMONIKER_UNASSIGN);    // really force!
    pMon = GetObjectMoniker(OLEGETMONIKER_FORCEASSIGN);
    if (pMon)
      pMon->Release();

    // update window title
    if (pszInstName) {
      pOleObject->SetHostNames (pDoc->pApplication->GetAppName(),
      pDoc->pContainer->GetWindowTitle());
    }
  }
  return hrErr;
}

//**************************************************************************
//
// IDropSource Implementation -- Although this is normally implemented by
//             server objects, we need it on our client
//             side helper to do "embed from embedding"
//             which is where you drag/drop or copy/paste
//             from an existing embedded object.
//
//**************************************************************************

HRESULT _IFUNC BOlePart::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
  SCODE sc;

  if (fEscapePressed)
    sc = DRAGDROP_S_CANCEL;
  else
    if (!(grfKeyState & MK_LBUTTON))
      sc = DRAGDROP_S_DROP;
    else
      sc = S_OK;

  return ResultFromScode (sc);

}

HRESULT _IFUNC BOlePart::GiveFeedback(DWORD dwEffect)
{
  return ResultFromScode (DRAGDROP_S_USEDEFAULTCURSORS);
}

//**************************************************************************
//
// CopyFromOriginal() helpers
//
//**************************************************************************

HRESULT _IFUNC BOlePart::AddCachedData (IDataObject *pTarget)
{
  BOOL fAddedMetafile = FALSE;
  LPOLECACHE pOleCache;

  if (SUCCEEDED (pOleObject && pDataObject &&
    pOleObject->QueryInterface (IID_IOleCache, &(LPVOID) pOleCache))) {

    // Get IEnumSTATDATA interface for IOleCache
    //
    LPENUMSTATDATA lpEnumSTATDATA;
    if (pOleCache->EnumCache (&lpEnumSTATDATA) == NOERROR && lpEnumSTATDATA) {

      // Enumerate all of the cached formats
      //
      STATDATA statData;
      while (lpEnumSTATDATA->Next (1, &statData, NULL) == NOERROR) {
        TCHAR name[32];
        GetClipboardFormatName (statData.formatetc.cfFormat, name, sizeof(name));

        // For each "cachable" format, try to get a copy of the data
        //
        STGMEDIUM stgMedium;
        if (pDataObject->GetData (&statData.formatetc, &stgMedium) != NOERROR) {
          // Data is not available
        }
        else
          if (stgMedium.pUnkForRelease != NULL) {

            // Don't cache data with pUnkForRelease != NULL
            //
            ::ReleaseStgMedium (&stgMedium);
            ::OleStdDeleteTargetDevice (statData.formatetc.ptd);
          }
          else {

            // Transfer to the target
            //
            pTarget->SetData (&statData.formatetc, &stgMedium, TRUE /*give ownership*/);
            if (statData.formatetc.cfFormat == CF_METAFILEPICT)
              fAddedMetafile = TRUE;
          }
      }
      lpEnumSTATDATA->Release();
    }
    pOleCache->Release();
  }

  // Just about everyone has a metafile in the cache. Paintbrush does not,
  // so we'll add it forcibly here.
  //
  if (!fAddedMetafile) {
    FORMATETC fe;
    fe.cfFormat = CF_METAFILEPICT;
    fe.tymed = TYMED_MFPICT;
    fe.lindex = -1;
    fe.ptd = NULL;

    SIZEL sizel;
    if (NOERROR != pOleObject->GetExtent (dwDrawAspect, &sizel)) {
      RECT rcpos, rcclip;
      pSite->GetSiteRect (&rcpos, &rcclip);
      sizel.cx = MAP_PIX_TO_LOGHIM (rcpos.right - rcpos.left, BOleService::pixPerIn.x);
      sizel.cy = MAP_PIX_TO_LOGHIM (rcpos.bottom - rcpos.top, BOleService::pixPerIn.y);
    }

    STGMEDIUM sm;
    sm.tymed = TYMED_MFPICT;
    sm.pUnkForRelease = NULL;
    sm.hGlobal = OleStdGetMetafilePictFromOleObject (pOleObject,
      dwDrawAspect, &sizel, NULL);
    if (sm.hGlobal)
      pTarget->SetData (&fe, &sm, TRUE);

  }
  return NOERROR;
}

HRESULT _IFUNC BOlePart::AddLinkSourceData (IDataObject *pTarget)
{
  FORMATETC fe;
  STGMEDIUM stgMedium;
  LPMONIKER lpMoniker;

  if (!pLinkObject) {

    // Get moniker from client site
    //
    LPOLECLIENTSITE cSite;
    if (SUCCEEDED (pObjOuter->QueryInterfaceMain (IID_IOleClientSite, &(LPVOID)cSite))) {
      cSite->GetMoniker (OLEGETMONIKER_TEMPFORUSER, OLEWHICHMK_OBJFULL, &lpMoniker);
      cSite->Release();
      if (!lpMoniker)
        return ResultFromScode (E_FAIL);
    } else
      return ResultFromScode (E_FAIL);
  }
  else {
    // Get moniker from the link object itself
    //
    if (SUCCEEDED (pOleObject->GetMoniker(OLEGETMONIKER_TEMPFORUSER,
      OLEWHICHMK_OBJFULL, &lpMoniker))) {
    }
    else
      return ResultFromScode (E_FAIL);
  }

  // Create a memory based stream to write the moniker to
  //
  LPSTREAM lpStream;
  if (!SUCCEEDED (::CreateStreamOnHGlobal (NULL, TRUE, &lpStream))) {
    lpMoniker->Release();
    return ResultFromScode (E_FAIL);
  }

  // Write the moniker to the stream
  //
  if (!SUCCEEDED (OleSaveToStream (lpMoniker, lpStream))) {
    lpStream->Release();
    return ResultFromScode (E_FAIL);
  }

  // Write the class ID of the document to the stream as well
  //
  CLSID clsid;
  pOleObject->GetUserClassID (&clsid);
  WriteClassStm (lpStream, clsid);

  lpMoniker->Release();

  // Give this stream to the data source (which then owns it)
  //
  memset (&fe, 0, sizeof (fe));
  fe.cfFormat = BOleDocument::oleLinkSrcClipFmt;
  fe.dwAspect = dwDrawAspect;
  fe.lindex = -1;
  fe.tymed = TYMED_ISTREAM;
  stgMedium.tymed = TYMED_ISTREAM;
  stgMedium.pstm = lpStream;
  stgMedium.pUnkForRelease = NULL;
  pTarget->SetData (&fe, &stgMedium, TRUE);

  // Set the object descriptor
  //
  stgMedium.tymed = TYMED_HGLOBAL;
  fe.cfFormat = BOleDocument::oleLinkSrcDescFmt;
  fe.tymed = TYMED_HGLOBAL;
  POINTL unused = {0,0};
  SIZEL unused2 = {0,0};
  stgMedium.hGlobal = OleStdGetObjectDescriptorDataFromOleObject(pOleObject,
    NULL, dwDrawAspect, unused, &unused2);
  stgMedium.pUnkForRelease = NULL;
  pTarget->SetData (&fe, &stgMedium, TRUE);

  return NOERROR;
}

HRESULT _IFUNC BOlePart::AddSiteData (LPDATAOBJECT pTarget)
{
  // Ask the site if they have any data to add.
  //
  // Implementation note: this is a slightly quirky way to do this, but
  // we're not set up to change it at this point. Other ways it could work:
  //
  // 1. Use aggregation so the user calls IBService::Clip with the data
  //    provider supported on their site object. Then they would get to
  //    provide their formats first, and then call back to the data provier
  //    on BOlePart to add the cached formats. Although this seems OLE-like,
  //    abandoned this idea because the aggregation is tricky and our
  //    data format negotiation stuff is different enough from OLE2's to
  //    make it difficult to bottleneck the OLE2 cache.
  //
  // 2. Put a flag in BOlePart which would be set when we're in a CopyFrom-
  //    Original. Then our BOlePart's data provider implementation would
  //    know when to get the data of the real OLE object, and when to get
  //    it from the cache. Abandoned this idea because the idea of more
  //    weird state information isn't attractive.
  //
  // 3. Add a SetFormatData to IBPart or IBDataProvider which could do an
  //    IBDataObject::SetData on the cache directly. Abandoned this idea
  //    because it would be another stub method on the server side.
  //

  HRESULT hr = NOERROR;
  PIBDataProvider pProvider = NULL;
  hr = pSite->QueryInterface (IID_IBDataProvider, &(LPVOID) pProvider);

  // Should never happen because aggregation will always return the
  // data provider on us if the site doesn't support it.
  //
  if (hr != NOERROR)
    return hr;

  if (pProvider != (PIBDataProvider)(IBPart*)this) {
    UINT count = pProvider->CountFormats ();

    HANDLE hData;
    BOleFormat bf;
    FORMATETC fe;
    STGMEDIUM stm;

    for (UINT i = 0; i < count; i++) {

      // Get the formats the site wants to add
      //
      hr = pProvider->GetFormat (i, &bf);
      if (hr == NOERROR) {
        hData = pProvider->GetFormatData (&bf);

        // Convert from BOleFormat to FORMATETC
        //
        if (hData) {
          fe.cfFormat = bf.fmtId;
          fe.ptd = NULL;
          fe.dwAspect = dwDrawAspect;
          fe.lindex = -1;
          switch (bf.fmtId) {
            case CF_METAFILEPICT:
              fe.tymed = TYMED_MFPICT;
              break;
            case CF_BITMAP:
              fe.tymed = TYMED_GDI;
              break;
            default:
              fe.tymed = TYMED_HGLOBAL;
          }

          // Only formats described by HGLOBALs are allowed here.
          // If we had this to do again, we'd just use FORMATETC
          //
          stm.tymed = fe.tymed;
          stm.hGlobal = hData;
          stm.pUnkForRelease = NULL;
        }

        // Set the FORMATETC and STGMEDIUM into the data cache
        //
        hr = pTarget->SetData (&fe, &stm, TRUE);
      }
    }
  }

  pProvider->Release();
  return hr;
}

// Link source not available
//
void _IFUNC BOlePart::HandleLinkError()
{
  HWND hWnd = pContainer->GetWindow();
  if (hWnd) {

    // Need to retrieve the descriptive name for the container to make the caption
    //
    HWND hTopWnd = GetTopWindow( hWnd );
    LPOLESTR pCaption = OLESTR( "" );
#ifndef MS_OLEUI_DEF
    if (OleUIPromptUser (IDD_LINKSOURCEUNAVAILABLE, hTopWnd, pCaption) == ID_PU_LINKS ){
// #endif

      // Since a link is unavailable, we ask the user whether they
      // want to go to the links dialog box. If they do, we invoke it
      // through the IBOverrideBrowseLinks interface. The reason we
      // don't call it directly on the BOleDocument is so that a
      // container app (WPWin) can use their own dialog in this case.
      //
      // In general, it isn't nice for Bolero to call our own dialogs
      // because users are supposed to be able to use their own.
      //
      PIBOverrideBrowseLinks pOBL = NULL;
      HRESULT qiHr = pDoc->QueryInterface (IID_IBOverrideBrowseLinks, &(LPVOID)pOBL);
      if (SUCCEEDED(qiHr)) { // should never fail since OBL is supported on the doc
        pOBL->BrowseLinks();
        pOBL->Release();
      }
// #ifndef MS_OLEUI_DEF
    }
#endif
  }
}



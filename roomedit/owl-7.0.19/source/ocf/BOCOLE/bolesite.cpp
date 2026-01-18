//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 server object. BOleSite objects
//  impersonate the client application from the point of view of the Bolero
//  customer who's writing a server object (IPart)
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolesite.h"
#include "boledoc.h"
#include "bolecman.h"

extern "C" {
  #include "ole2ui.h"
}

// System/RTL headers
//



#define NOT_IMPLEMENTED OLERET(E_NOTIMPL);


HRESULT _IFUNC BOleSite::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // Self
  //
  if (IID_BOleSite == iid) {
    (BOleSite *)*ppv = this;
    AddRef();
    return NOERROR;
  }
  // nested interface class

  if (IID_IBApplication == iid) {
    ((IBApplication *)*ppv) = &iApplication;
    AddRef();
    return NOERROR;
  }

  // interfaces
  if (SUCCEEDED(hr = IBContainer_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IBLinkable_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IPersistStorage_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IDataObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IDropSource_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IOleObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IOleInPlaceObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IOleInPlaceActiveObject_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IBSite_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IEnumFORMATETC_QueryInterface(this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IBDataState_QueryInterface(this, iid, ppv))) {
  }

  // base classes
  else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))) {
  }

  return hr;
};


//
// BOleSite implementation --    These functions aren't part of an inherited
//             interface, but are part of the private
//             implementation of BOleSite
//

BOleSite::BOleSite(BOleClassManager * pFact, IBUnknownMain *pOuter,
       BOleService *pSvc)
  : BOleComponent (pFact, pOuter) , iApplication(this, pOuter),
    cid (CLSID_NULL)
{
  pService = pSvc;

#ifdef OLEDBG
  DebugMode = 0;
#endif

  pClientSite = NULL;
  pStg = NULL;
  pDAdvHolder = NULL;
  pAdvHolder = NULL;
  pPart = NULL;  //    (IPart *) pOuter->InterfaceGet(IID(IPart));
  pProvider = NULL;
  pIPSite = 0;
  pIPFrame = 0;
  pIPDoc = 0;
  fShown = FALSE;
  fInPlaceActive = FALSE;
  fInPlaceVisible = FALSE;
  fInsideOut = FALSE;
  fUIActive = FALSE;
  fInHelpMode = FALSE;
  fInClose = FALSE;
  hWndParent = 0;
  pHatchWnd = NULLP;
  hWndInPlace = 0;
  hOleMenu = 0;
  hMenuShared = 0;
  pszProgID = NULLP;
  pmkDoc = NULLP;
  hatchOffset.x = hatchOffset.y = 0;
  pFactory->ServerCount(+1L);
  regLink = 0L;
  pszInstName = NULL;
  pWindowTitle = NULL;
  pAppName = NULL;
  fHandsOff = FALSE;
  fIsDirty = TRUE;
  formatList = NULL;
  formatCount = 0;
  formatLink = TRUE;     // default pending moniker existence
  formatEmbed = TRUE;
  pInPlaceName = NULL;
  pDocument = NULL;
}

HRESULT _IFUNC BOleSite::Init (PIBDataProvider pProv, PIBPart ppart, LPCOLESTR psz, BOOL fHatchWnd)
{
  // If the provider also supports IBPart, then it's a full-fledged
  // server object capable of inplace activation. If it doesn't, this
  // BOleSite will only be used for data transfer.
  //
  pProvider = pProv;
  pPart = ppart;

  // Bind to BOleFact so we can get the CLSID
  //
  if (pszProgID != NULLP)
    delete pszProgID;
  pszProgID = new OLECHAR [lstrlen(psz) + 1];
  if (!pszProgID)
    return ResultFromScode (E_OUTOFMEMORY);
  lstrcpy (pszProgID, psz);

  CLSIDFromProgID(pszProgID, &cid);

  // Cache the available formats because OLE2 will ask for formats
  // during link/embed from file, even though no Clip or Drag
  // happened.
  //
  pService->FormatHelper (&formatList, &formatCount, formatLink,
          formatEmbed, pProvider);

  // Create a hatched window which is visible during visual editing
  //
  if (pPart && fHatchWnd) {
    //pHatchWnd = BOleHatchWindow::Create (NULL, boleInst, this);
    pHatchWnd = new BOleHatchWindow(NULL, boleInst, this);
    if (!pHatchWnd || !(HANDLE)pHatchWnd)
      return ResultFromScode (E_OUTOFMEMORY);
  }

  // Catenate the APPNAME and the SHORTUSERTYPENAME together. This seems
  // to be the convention for SetActiveObject
  //
  LPOLESTR appname = NULL, shortname = NULL;
  int c = 0;
  OleRegGetUserType (cid, USERCLASSTYPE_APPNAME, &appname);
  if (appname)
    c += lstrlen (appname) + lstrlen (OLESTR (" - "));
  OleRegGetUserType (cid, USERCLASSTYPE_SHORT, &shortname);
  if (shortname)
    c += lstrlen (shortname);

  pInPlaceName = new OLECHAR [c + 1]; // +1 for null term
  if (!pInPlaceName)
    return ResultFromScode (E_OUTOFMEMORY);
  pInPlaceName[0] = 0;

  if (appname) {
    lstrcat (pInPlaceName, appname);
    lstrcat (pInPlaceName, OLESTR(" - ") );
  }
  if (shortname)
    lstrcat (pInPlaceName, shortname);

  IMalloc *pMalloc;
  if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pMalloc))) {
    pMalloc->Free(appname);
    pMalloc->Free(shortname);
    pMalloc->Release();
  }

  return NOERROR;
}

BOleSite::~BOleSite ()
{
  // Don't translate any more accelerators through this helper
  //
  if (pService->pFocusedSite == this)
    OnSetFocus (NULL);

  DoClose();

  // Release the client site here (not in DoClose) so that we can in place
  // activate after an open-edit session  (Dll server case).
  //
  if (pClientSite) {
    pClientSite->Release ();
    pClientSite = NULL;
  }
  if (pWindowTitle) {
    delete [] pWindowTitle;
    pWindowTitle = NULL;
  }
  if (pAppName) {
    delete [] pAppName;
    pAppName = NULL;
  }
  if (pszProgID) {
    delete pszProgID;
    pszProgID = NULL;
  }
  if (pHatchWnd) {
    delete pHatchWnd;
    pHatchWnd = NULL;
  }
  if (pDocument) {  // Set in BOleContainer::GetObject
    CoLockObjectExternal(pDocument, FALSE, FALSE);
    pDocument = NULL;
    }
  if (pmkDoc) {
    pmkDoc->Release();
    pmkDoc = NULL;
  }
  if (pszInstName) {
    delete [] pszInstName;
    pszInstName = NULL;
  }
  if (pInPlaceName) {
    delete [] pInPlaceName;
    pInPlaceName = NULL;
  }
  if (formatList) {
    delete [] formatList;
  }
  pFactory->ServerCount(-1L);

}

//
// Accessor functions for format lists
//
void _IFUNC BOleSite::GetFormatFlags(BOOL *pfEmbed, BOOL *pfLink)
{
  if (pService->dropCount || (pService->clipCount && pService->IsOnClipboard(this))) {
    *pfLink  = pService->fUseDropList ? pService->dropOkToLink  : pService->clipOkToLink;
    *pfEmbed = pService->fUseDropList ? pService->dropOkToEmbed : pService->clipOkToEmbed;
  }
  else {
    if (!formatList) {
      // Cache the available formats because OLE2 will ask for formats
      // during link/embed from file, even though no Clip or Drag
      // happened.
      pService->FormatHelper (&formatList, &formatCount, formatLink,
            formatEmbed, pProvider);
    }
    *pfLink = formatLink;
    *pfEmbed = formatEmbed;
  }

  if (fInClose)
    *pfLink = FALSE; // no link format available after shutdown

}

void _IFUNC BOleSite::GetFormatList(LPFORMATETC *ppList, UINT *pCount)
{
  if (pService->dropCount || (pService->clipCount && pService->IsOnClipboard(this))) {
    *ppList = pService->fUseDropList ? pService->dropList : pService->clipList;
    *pCount = pService->fUseDropList ? pService->dropCount : pService->clipCount;
  }
  else {
    if (!formatList) {
      // Cache the available formats because OLE2 will ask for formats
      // during link/embed from file, even though no Clip or Drag
      // happened.
      pService->FormatHelper (&formatList, &formatCount, formatLink,
            formatEmbed, pProvider);
    }
    *ppList = formatList;
    *pCount = formatCount;
  }
}

void _IFUNC BOleSite::GetFormatCount(UINT *pCount)
{
  if (pService->dropCount || (pService->clipCount && pService->IsOnClipboard(this))) {
    *pCount = pService->fUseDropList ? pService->dropCount : pService->clipCount;
  }
  else {
    if (!formatList) {
      // Cache the available formats because OLE2 will ask for formats
      // during link/embed from file, even though no Clip or Drag
      // happened.
      pService->FormatHelper (&formatList, &formatCount, formatLink,
            formatEmbed, pProvider);
    }
    *pCount = formatCount;
  }
}

void _IFUNC BOleSite::GetFormatInfo(BOOL *pfEmbed, BOOL *pfLink, LPFORMATETC *ppList, UINT *pCount)
{
  if (pService->dropCount || (pService->clipCount && pService->IsOnClipboard(this))) {
    *pfLink = pService->fUseDropList ? pService->dropOkToLink : pService->clipOkToLink;
    *pfEmbed = pService->fUseDropList ? pService->dropOkToEmbed : pService->clipOkToEmbed;
    *ppList = pService->fUseDropList ? pService->dropList : pService->clipList;
    *pCount = pService->fUseDropList ? pService->dropCount : pService->clipCount;
  }
  else {
    if (!formatList) {
      // Cache the available formats because OLE2 will ask for formats
      // during link/embed from file, even though no Clip or Drag
      // happened.
      pService->FormatHelper (&formatList, &formatCount, formatLink,
                  formatEmbed, pProvider);
    }
    *pfLink = formatLink;
    *pfEmbed = formatEmbed;
    *ppList = formatList;
    *pCount = formatCount;
  }
}

// Render this server object onto a storage medium.
//
HRESULT _IFUNC BOleSite::GetEmbeddedObjectData (LPFORMATETC pFmt, LPSTGMEDIUM pMed)
{
  LPSTORAGE   lpstg = NULL;
  SCODE       sc = S_OK;
  HRESULT     hrErr;

  pMed->pUnkForRelease = NULL;

  if (pMed->tymed == TYMED_NULL) {

    // If the tymed passed in is NULL, we will create a temporary storage
    // in memory in order to render the object
    //
    if (pFmt->tymed & TYMED_ISTORAGE) {
      lpstg = CreateStorageInMemory (STGM_READWRITE
        | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE??*/);
      if (!lpstg)
        return ResultFromScode(E_OUTOFMEMORY);

      pMed->pstg = lpstg;
      pMed->tymed = TYMED_ISTORAGE;
      pMed->pUnkForRelease = NULL;
    }
    else
      return ResultFromScode(DATA_E_FORMATETC);
  }

  // If the tymed passed in is non-NULL, we will use the storage they
  // gave us. If it's not a storage, return failure.
  //
  else if (pMed->tymed & TYMED_ISTORAGE) {
    pMed->tymed = TYMED_ISTORAGE;

    // The online dox say that OleSave does this, but there's no
    // CompObj stream without WriteFmtUseTypeStg and no CLSID in the
    // stream without WriteClassStg
    //
    CLSID clsid = CLSID_NULL;
    CLSIDFromProgID (pszProgID, &clsid);
    if ((hrErr = WriteClassStg (pMed->pstg, clsid)) != NOERROR)
      return hrErr;
    LPOLESTR lpLongType = NULL;
    OleRegGetUserType (clsid, USERCLASSTYPE_FULL, &lpLongType);
    hrErr = WriteFmtUserTypeStg (pMed->pstg, pFmt->cfFormat, lpLongType);
  }
  else
    return ResultFromScode(DATA_E_FORMATETC);

  // We don't really expect anyone to respond to this call, but WPWin
  // is going to try to decide whether a new object is linked or embedded
  // based on this notification.
  //
  BOleFormat f;
  f.fmtId = BOleDocument::oleEmbSrcClipFmt;
  f.fmtMedium = BOLE_MED_STORAGE;
  f.fmtName[0] = 0;
  f.fmtResultName[0] = 0;
  f.fmtIsLinkable = FALSE;
  pProvider->GetFormatData (&f);

  hrErr = OleSave(this, pMed->pstg, FALSE /* fSameAsLoad */);
  if (hrErr != NOERROR)
    sc = GetScode(hrErr);

  // Always call SaveCompleted, but don't stomp the error code if
  // OleSave failed
  //
  hrErr = SaveCompleted (NULL);
  if (hrErr != NOERROR && sc == S_OK)
    sc = GetScode(hrErr);

  return ResultFromScode(sc);
}

// Render the data for a link source into a stream medium
//
HRESULT _IFUNC BOleSite::GetLinkSourceData(LPMONIKER lpmk, REFCLSID cid,
    LPFORMATETC lpformatetc, LPSTGMEDIUM lpMedium)
{
  LPSTREAM    lpstm = NULL;
  HRESULT     hrErr;

  // We don't really expect anyone to respond to this call, but WPWin
  // is going to try to decide whether a new object is linked or embedded
  // based on this notification.
  //
  BOleFormat f;
  f.fmtId = BOleDocument::oleLinkSrcClipFmt;
  f.fmtMedium = BOLE_MED_STREAM;
  f.fmtName[0] = 0;
  f.fmtResultName[0] = 0;
  f.fmtIsLinkable = TRUE;
  pProvider->GetFormatData (&f);

  if (lpMedium->tymed == TYMED_NULL) {
    if (lpformatetc->tymed & TYMED_ISTREAM) {
      hrErr = CreateStreamOnHGlobal(
          NULL, // auto allocate
          TRUE, // delete on release
          (LPSTREAM FAR*)&lpstm
      );
      if (hrErr != NOERROR) {
        lpMedium->pUnkForRelease = NULL;
        return ResultFromScode (E_OUTOFMEMORY);
      }
      lpMedium->pstm = lpstm;
      lpMedium->tymed = TYMED_ISTREAM;
      lpMedium->pUnkForRelease = NULL;
    }
    else {
      lpMedium->pUnkForRelease = NULL;
      return ResultFromScode (DATA_E_FORMATETC);
    }
  }
  else {
    if (lpMedium->tymed & TYMED_ISTREAM) {
      lpMedium->tymed = TYMED_ISTREAM;
      lpMedium->pstm = lpMedium->pstm;
      lpMedium->pUnkForRelease = NULL;
    }
    else {
      lpMedium->pUnkForRelease = NULL;
      return ResultFromScode (DATA_E_FORMATETC);
    }
  }

  hrErr = OleSaveToStream((LPPERSISTSTREAM)lpmk, lpMedium->pstm);
  if (hrErr != NOERROR)
    return hrErr;
  return WriteClassStm (lpMedium->pstm, cid);
}

// Creates an LPSTORAGE with the specified flags on a memory lockbytes
//
LPSTORAGE _IFUNC BOleSite::CreateStorageInMemory (DWORD grfMode)
{
  DWORD grfCreateMode =grfMode | STGM_CREATE;
  HRESULT hrErr;
  LPLOCKBYTES lpLockBytes = NULL;
  DWORD reserved = 0;
  LPSTORAGE lpStg = NULL;

  hrErr = CreateILockBytesOnHGlobal(NULL, TRUE /*delete on release*/,
    &lpLockBytes);
  if (hrErr != NOERROR)
    return NULL;

  hrErr = StgCreateDocfileOnILockBytes(lpLockBytes, grfCreateMode,
    reserved, &lpStg);
  if (hrErr != NOERROR) {
    lpLockBytes->Release();
    return NULL;
  }
  lpLockBytes->Release();
  return lpStg;
}

//**************************************************************************
//
// IDataObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::GetData (LPFORMATETC pFormatEtcIn, LPSTGMEDIUM pMedium)
{
  TCHAR name[32];
  GetClipboardFormatName (pFormatEtcIn->cfFormat, name, sizeof(name));

  HRESULT hrErr = ResultFromScode (DATA_E_FORMATETC);

  BOOL fLink = TRUE;
  BOOL fEmbed = TRUE;

  GetFormatFlags(&fEmbed, &fLink);

  if (pFormatEtcIn->dwAspect & (DVASPECT_CONTENT | DVASPECT_DOCPRINT)) {

    pMedium->tymed    = 0L;
    pMedium->pUnkForRelease = NULL;
    pMedium->hGlobal  = NULL;

    if (pFormatEtcIn->cfFormat == BOleDocument::oleEmbSrcClipFmt) {
      if( fEmbed )
        hrErr = GetEmbeddedObjectData (pFormatEtcIn, pMedium);
    }
    else if (pFormatEtcIn->cfFormat == BOleDocument::oleObjectDescFmt) {
      if( fEmbed )
        goto _getobjdesc;
    }
    else if (pFormatEtcIn->cfFormat == BOleDocument::oleLinkSrcClipFmt) {
      if( fLink ) {
        LPMONIKER pMon;
        hrErr = GetMoniker(OLEGETMONIKER_ONLYIFTHERE,OLEWHICHMK_OBJFULL, &pMon);
        if (SUCCEEDED(hrErr)) {
          CLSID id;
          hrErr = GetUserClassID(&id);
          pMedium->tymed = TYMED_NULL;    // for GetLinkSourceData
          pFormatEtcIn->cfFormat = BOleDocument::oleLinkSrcClipFmt; // per Brockshmit's suggestion
          pFormatEtcIn->tymed = TYMED_ISTREAM;    // pg. 773
          if (SUCCEEDED(hrErr)) {
            hrErr = GetLinkSourceData(pMon, id, pFormatEtcIn, pMedium);
          }
        pMon->Release();
        }
      }
    }
    else if (pFormatEtcIn->cfFormat == BOleDocument::oleLinkSrcDescFmt) {
      if( fLink ) {
_getobjdesc:;
        // Object descriptors *must* be returned in a memory handle
        //
        if (pFormatEtcIn->tymed & TYMED_HGLOBAL) {
          pMedium->hGlobal = GetObjDescData ();
          if (pMedium->hGlobal) {
            pMedium->tymed = TYMED_HGLOBAL;
            hrErr = ResultFromScode (S_OK);
          }
          else
            hrErr = ResultFromScode (E_OUTOFMEMORY);
        }
      }
    }
    else if (pFormatEtcIn->cfFormat == CF_METAFILEPICT && (pFormatEtcIn->tymed & TYMED_MFPICT)) {
      hrErr = GetMetaFileData (pMedium->hGlobal);
      if (hrErr == NOERROR)
        pMedium->tymed = TYMED_MFPICT;
    }
    // It's not an OLE2 format, maybe it's a regular clipboard format
    // the client would know.
    //
    else {
      BOleFormat boleFmt;
      memset(&boleFmt, 0, sizeof(boleFmt));
      boleFmt.fmtId = pFormatEtcIn->cfFormat;
      boleFmt.fmtMedium = (BOleMedium) pFormatEtcIn->tymed;

      //PHP It would be more efficient to use our list of formats
      //from enumFormatEtc to check before calling GetFormatData

      if (pMedium) {
        pMedium->hGlobal = pProvider->GetFormatData (&boleFmt);
        if (pMedium->hGlobal) {
          pMedium->tymed = boleFmt.fmtMedium & ~BOLE_MED_STATIC;
        }
      }

      if (pMedium->hGlobal)
        hrErr = ResultFromScode (S_OK);
    }
  }

  return hrErr;
}

#ifdef TESTMETAFILE
extern HINSTANCE boleInst;

void RenderMetaFile(HGLOBAL hMFPict)
{
  LPMETAFILEPICT pMFPict = (LPMETAFILEPICT)    GlobalLock (hMFPict);
  int xPix = MAP_LOGHIM_TO_PIX (pMFPict->xExt, BOleService::pixPerIn.x);
  int yPix = MAP_LOGHIM_TO_PIX (pMFPict->yExt, BOleService::pixPerIn.y);

  HWND w = GetDesktopWindow();

  if (w) {
    HDC dc = GetDC (w);
    UINT oldMapMode = SetMapMode(dc, MM_ANISOTROPIC);
    SetWindowExt (dc, 1000, 1000);
    SetViewportExt (dc, xPix, yPix);
    PlayMetaFile (dc, pMFPict->hMF);
    SetMapMode (dc, oldMapMode);
    ReleaseDC(w, dc);
    GlobalUnlock (hMFPict);
    }
}

#endif

HRESULT _IFUNC BOleSite::GetMetaFileData (HGLOBAL& hMem)
{
  HRESULT hrErr = NOERROR;
  HDC hDC = CreateMetaFile(NULL);
  if (hDC) {
    // Get server size in pixels
    SIZE s;
    pProvider->GetPartSize (&s);
    // Convert server size to himetric, and set up dc for drawing
    //
    SIZEL sl;
    sl.cx = MAP_PIX_TO_LOGHIM (s.cx, BOleService::pixPerIn.x);
    sl.cy = MAP_PIX_TO_LOGHIM (s.cy, BOleService::pixPerIn.y);

    SetMapMode(hDC, MM_ANISOTROPIC);
    SetWindowOrg(hDC, 0, 0);
    SetWindowExt(hDC, sl.cx, sl.cy);

    RECTL rPos = {0L, 0L, sl.cx, sl.cy}, rExt = {0L, 0L, sl.cx, sl.cy};
    pProvider->Draw (hDC, &rPos, &rExt, BOLE_CONTENT);
    HMETAFILE hMF = CloseMetaFile (hDC);

    if (hMF) {
      HGLOBAL hMFPict = ::GlobalAlloc (GMEM_SHARE|GMEM_ZEROINIT, sizeof (METAFILEPICT));
      if (hMFPict) {
  LPMETAFILEPICT pPict = (LPMETAFILEPICT) ::GlobalLock (hMFPict);
  if (pPict) {
    pPict->mm   =  MM_ANISOTROPIC;
    pPict->hMF  =  hMF;
    pPict->xExt =  sl.cx;
    pPict->yExt =  sl.cy;  // add minus sign to make it +ve
    GlobalUnlock (hMFPict);
    hMem = hMFPict;
  }
  else {
    GlobalFree (hMFPict);
    DeleteMetaFile (hMF);
    hrErr = ResultFromScode (E_OUTOFMEMORY);
  }
      }
      else {
  DeleteMetaFile (hMF);
  hrErr = ResultFromScode (E_OUTOFMEMORY);
      }
    }
    else
      hrErr = ResultFromScode (E_OUTOFMEMORY);
  }
#ifdef TESTMETAFILE
  if (SUCCEEDED(hrErr))
    RenderMetaFile(hMem);
#endif

  return hrErr;
}

HRESULT _IFUNC BOleSite::GetDataHere (LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  TCHAR name[32];
  GetClipboardFormatName(pFormatEtc->cfFormat, name, sizeof(name));

  BOOL fLink = TRUE;
  BOOL fEmbed = TRUE;

  GetFormatFlags(&fEmbed, &fLink);

  HRESULT hr = ResultFromScode(DATA_E_FORMATETC);

  if (pFormatEtc->cfFormat == BOleDocument::oleEmbSrcClipFmt)  {
    if( fEmbed )
      hr = GetEmbeddedObjectData (pFormatEtc, pMedium);
  }
  else
    if (pFormatEtc->cfFormat == BOleDocument::oleLinkSrcClipFmt) {
      if (fLink) {
        LPMONIKER pMon;
        hr = GetMoniker(OLEGETMONIKER_ONLYIFTHERE,OLEWHICHMK_OBJFULL, &pMon);
        if (SUCCEEDED(hr)) {
          CLSID id;
          hr = GetUserClassID (&id);
          pMedium->tymed = TYMED_NULL;    // for GetLinkSourceData
          pFormatEtc->cfFormat = BOleDocument::oleLinkSrcClipFmt; // per Brockshmit's suggestion
          pFormatEtc->tymed = TYMED_ISTREAM;    // pg. 773
          if (SUCCEEDED(hr))
            hr = GetLinkSourceData(pMon, id, pFormatEtc, pMedium);
          pMon->Release();
        }
      }
    }

  return hr;
}

HRESULT _IFUNC BOleSite::QueryGetData (LPFORMATETC pFormatEtc)
{
  TCHAR name[32];
  GetClipboardFormatName(pFormatEtc->cfFormat, name, sizeof(name));

  BOOL fLink = TRUE;
  BOOL fEmbed = TRUE;
  LPFORMATETC pList;
  UINT count;

  GetFormatInfo(&fEmbed, &fLink, &pList, &count);

  HRESULT hrErr = ResultFromScode (DV_E_FORMATETC);

  if (pFormatEtc->cfFormat == BOleDocument::oleObjectDescFmt) {
    if (fEmbed) {
      // Object Descriptors must be offered in global handles
      //
      if (pFormatEtc->tymed & TYMED_HGLOBAL)
        hrErr = NOERROR;
      else
        hrErr = ResultFromScode (DV_E_FORMATETC);
    }
  }

  else if (pFormatEtc->cfFormat == BOleDocument::oleLinkSrcDescFmt) {
    if (fLink) {
      // Link Src Desc, like Obj Desc, must be offered in a global handle
      //
      if (pFormatEtc->tymed & TYMED_HGLOBAL)
        hrErr = NOERROR;
      else
        hrErr = ResultFromScode (DV_E_FORMATETC);
    }
  }

  for (UINT i = 0; i < count; i++) {
    if ((pList[i].cfFormat == pFormatEtc->cfFormat) &&
      (pList[i].tymed & pFormatEtc->tymed)) {
      return ResultFromScode(S_OK);
    }
  }

  return hrErr;
}

HRESULT _IFUNC BOleSite::GetCanonicalFormatEtc (LPFORMATETC pFormatetc, LPFORMATETC pFormatetcOut)
{
  if (!pFormatetcOut)
    return ResultFromScode( E_INVALIDARG );

  pFormatetcOut->ptd = NULL;

  if (!pFormatetc)
    return ResultFromScode (E_INVALIDARG);

  HRESULT hrErr;

  hrErr = QueryGetData (pFormatetc);
  if (hrErr != NOERROR)
    return hrErr;

  *pFormatetcOut = *pFormatetc;
  if (!pFormatetc->ptd)
    return ResultFromScode (DATA_S_SAMEFORMATETC);

  pFormatetcOut->ptd = NULL;
  return NOERROR;
}

// Set data into the server
//
HRESULT _IFUNC BOleSite::SetData (LPFORMATETC pFormatEtc, STGMEDIUM FAR* pMedium, BOOL fRelease)
{
  PIBDataProvider2 pProvider2 = NULL;

  // Does client implement IBDataProvider2?
  //
  if (!SUCCEEDED(pObjOuter->QueryInterfaceMain (IID_IBDataProvider2,&(LPVOID)pProvider2)))
    NOT_IMPLEMENTED

  HRESULT hr = ResultFromScode (DATA_E_FORMATETC);

  if ((BOleMedium) pFormatEtc->tymed == BOLE_MED_HGLOBAL) {
    BOleFormat boleFmt;
    memset(&boleFmt, 0, sizeof(boleFmt));
    boleFmt.fmtId = pFormatEtc->cfFormat;
    boleFmt.fmtMedium = (BOleMedium) pFormatEtc->tymed;

    if (pMedium) {
      hr = pProvider2->SetFormatData(&boleFmt, pMedium->hGlobal, FALSE);

      // Always do release here instead of in SetFormatData
      //
      if (fRelease)
        ReleaseStgMedium(pMedium);
    }
  }

  pProvider2->Release();
  return hr;
}

HRESULT _IFUNC BOleSite::EnumFormatEtc (DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{
  if (dwDirection == DATADIR_GET) {
    *ppenumFormatEtc = this;
    enumFmtIndex = 0L;
    AddRef();    // caller must release
    return ResultFromScode(S_OK);
  }

  // I'm not sure what it would mean to call IDataObject::SetData
  // on the BOleSite so it's blank until I can do more research
  //
  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleSite::DAdvise (FORMATETC FAR* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD FAR* pdwConnection)
{
  HRESULT hrErr;

  *pdwConnection = 0;
  if (pDAdvHolder == 0)
    hrErr = CreateDataAdviseHolder (&pDAdvHolder);
  if (pDAdvHolder)
    hrErr = pDAdvHolder->Advise (this, pFormatetc, advf, pAdvSink, pdwConnection);
  else
    hrErr = ResultFromScode (E_OUTOFMEMORY);

  return hrErr;
}

HRESULT _IFUNC BOleSite::DUnadvise (DWORD dwConnection)
{
  HRESULT hrErr = ResultFromScode(E_FAIL);
  if (pDAdvHolder)
    hrErr = pDAdvHolder->Unadvise (dwConnection);
  return hrErr;
}

HRESULT _IFUNC BOleSite::EnumDAdvise (IEnumSTATDATA* FAR* ppenumAdvise)
{
  HRESULT hrErr = ResultFromScode(E_FAIL);
  if (pDAdvHolder)
    hrErr = pDAdvHolder->EnumAdvise (ppenumAdvise);
  return hrErr;
}

//**************************************************************************
//
// IEnumFORMATETC implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::Next (ULONG celt, FORMATETC FAR * pFE, ULONG FAR* pceltFetched)
{
  LPFORMATETC pList;
  UINT count;

  GetFormatList(&pList, &count);

  if (pceltFetched)
    *pceltFetched = 0;

  if( !celt )
    return ResultFromScode (E_INVALIDARG);

  short nCopied = 0;
  while ( (enumFmtIndex < count) && (nCopied < celt) ) {
    pFE[nCopied] = pList[enumFmtIndex];
    enumFmtIndex++;
    nCopied++;
    if (pceltFetched)
      (*pceltFetched)++;
  }
  return ResultFromScode( nCopied != celt ? S_FALSE : S_OK);
}

HRESULT _IFUNC BOleSite::Skip (ULONG celt)
{
  UINT count;
  GetFormatCount(&count);

  if( celt > count )
    return ResultFromScode (E_INVALIDARG);

  if( enumFmtIndex < count ) {
    int diff = count - enumFmtIndex;
    enumFmtIndex += celt;
    return ResultFromScode (diff < celt ? S_OK : S_FALSE);
  }
  return ResultFromScode (S_FALSE);
}

HRESULT _IFUNC BOleSite::Reset ()
{
  enumFmtIndex = 0L;
  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::Clone (IEnumFORMATETC FAR* FAR* ppenum)
{
  return ResultFromScode (E_NOTIMPL);
}


//**************************************************************************
//
// IOleObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::SetClientSite (IOleClientSite* pCS)
{
  if (pClientSite)
    pClientSite->Release ();
  pClientSite = pCS;
  if (pClientSite)
    pClientSite->AddRef ();
  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::GetClientSite (IOleClientSite* FAR* ppClientSite)
{
  if (pClientSite) {
    pClientSite->AddRef ();
    *ppClientSite = pClientSite;
    return ResultFromScode(S_OK);
  }
  else {
    *ppClientSite = NULL;
    return ResultFromScode(E_FAIL);
  }
}

HRESULT _IFUNC BOleSite::SetHostNames (LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
  // Delete the old ones if any
  //
  if (pAppName) {
    delete [] pAppName;
    pAppName = NULL;
  }
  if (pWindowTitle) {
    delete [] pWindowTitle;
    pWindowTitle = NULL;
  }

  // Copy the new ones into member data so they're around in case
  // the part object asks for them (through IBContainer or IBApplication)
  //
  short c = lstrlen (szContainerApp) + 1; //+1 for \0 terminator
  pAppName = new OLECHAR[c];
  if (!pAppName)
    return ResultFromScode (E_OUTOFMEMORY);
  lstrcpyn( pAppName, szContainerApp, c );
  if (szContainerObj) {       // docs say this could be NULL
    c = lstrlen (szContainerObj) + 1;    //+1 for \0 terminator
    pWindowTitle = new OLECHAR[c];
    if (!pWindowTitle)
      return ResultFromScode (E_OUTOFMEMORY);
    lstrcpyn ( pWindowTitle, szContainerObj, c);
  }
  return NOERROR;
}


HRESULT _IFUNC BOleSite::DoClose ()
{
  // If closed while open in place, exit in-place mode
  //
  if (fInPlaceActive) {
    DoInPlaceDeactivate ();
  }

  // If closed while open editing, exit open edit mode
  //
  if (fShown) {
    // pass FALSE here to not allow Lock(FALSE) to cause ::Close to be
    // called recursively in in-proc server case.
    // Unlocking is done at the end of ::Close (see comments below).
    //
    HideWindow(FALSE);
  }

  //    If this object is on the clipboard, render all formats
  //
  if (pService->IsOnClipboard(this))
    pService->FlushClipboardData(FALSE);

  // Unregister for linking
  //
  if (regLink != 0L) {
    LPRUNNINGOBJECTTABLE pROT = NULL;
    HRESULT hr = OLE::GetRunningObjectTable (0, &pROT);
    if (SUCCEEDED(hr)) {
      pROT->Revoke(regLink);
      regLink = 0L;
      pROT->Release();
    }
  }

  // Notify containers of exit from "running state"
  //
  if (pDAdvHolder) {
    pDAdvHolder->SendOnDataChange (this, 0, ADVF_DATAONSTOP);
    pDAdvHolder->Release ();
    pDAdvHolder = NULL;
  }
  if (pAdvHolder) {
    BOOL tmp = fInClose;
    fInClose = TRUE;
    pAdvHolder->SendOnClose ();   // SendOnClose recursively calls Close!
    pAdvHolder->Release ();
    fInClose = tmp;
    pAdvHolder = NULL;
  }

  // Release some remaining references to others
  //
  if (pIPSite) {
    pIPSite->Release();
    pIPSite = NULL;
  }
  if (pIPFrame) {
    pIPFrame->Release();
    pIPFrame = NULL;
  }
  if (pIPDoc) {
    pIPDoc->Release();
    pIPDoc = NULL;
  }
  if (pStg) {
    pStg->Release ();
    pStg = NULL;
  }
  return NOERROR;
}

HRESULT _IFUNC BOleSite::Close (DWORD dwSaveOption)
{
  HRESULT hrErr = NOERROR;
  if (fInClose)
    return NOERROR;
  AddRef();
  fInClose = TRUE;

  if (fIsDirty) {
    if (dwSaveOption == OLECLOSE_PROMPTSAVE) {
      if (pAppName) {
        TCHAR text[64];
        ::LoadString (boleInst, IDS_SAVEOBJ, text, sizeof text);
        UINT nResponse = MessageBox(NULL, text,
            NULL /* pAppName */, MB_ICONQUESTION | MB_YESNOCANCEL );  // rayk - fix title pAppName for win95
        switch (nResponse) {
          case IDNO:
            dwSaveOption = OLECLOSE_NOSAVE;
            break;
          case IDCANCEL:
            hrErr = ResultFromScode(OLE_E_PROMPTSAVECANCELLED); // don't close
            fInClose = FALSE;
            return hrErr;
          case IDYES:
            dwSaveOption = OLECLOSE_SAVEIFDIRTY;
            break;
        }
      }
    }
    switch (dwSaveOption) {
      case OLECLOSE_SAVEIFDIRTY:
        if (pClientSite) {
          pClientSite->SaveObject();
          fIsDirty = FALSE;
        }
        break;
      case OLECLOSE_NOSAVE:
        break;
    }
  }

  DoClose();

  // Might not have a pPart if this helper object was only used by a
  // pProvider for data transfer
  //
  if (pPart) {
    pPart->Close();
  }

  // This allows self embedded apps to in place activate and still shutdown
  // properly.  In self embed in place activation, default handler aggregator
  // seems to have interfaces that don't go away until a last unlock releases.
  //
  // Unfortunately, the default handler also calls IOleObject::Close
  // recursively.  Wish it would just drop those references without
  // calling Close!  or Release those references before calling close...
  //
  CoDisconnectObject(AsPIUnknown(pObjOuter), 0);
  fInClose = FALSE;
  Release();

  return hrErr;
}

HRESULT _IFUNC BOleSite::SetMoniker (DWORD dwWhichMoniker, IMoniker* pmk)
{
  HRESULT hr= ResultFromScode(E_FAIL);
  LPMONIKER pmkFull;

  // To play "link to embedding", we need to register this object
  // under the full moniker given by the client site.

  // get the running object table
  LPRUNNINGOBJECTTABLE pROT = NULL;
  hr = OLE::GetRunningObjectTable (0, &pROT);
  if (SUCCEEDED(hr)) {
    // Register the new moniker BEFORE revoking the old moniker.
    // Otherwise the object's "StubManager" gets hosed.
    //
    DWORD oldRegLink = regLink;

    hr = GetMoniker(OLEGETMONIKER_ONLYIFTHERE, OLEWHICHMK_OBJFULL, &pmkFull);
    if (SUCCEEDED(hr)) {
      hr = pROT->Register (0, //ROTFLAGS_REGISTRATIONKEEPSALIVE,
                 AsPIUnknown(pObjOuter), pmkFull, &regLink);
      if (pAdvHolder) {
        pAdvHolder->SendOnRename (pmkFull);
      }
      pmkFull->Release();
    }
    else
      regLink = 0L;

    // release any old running object table registration
    if (oldRegLink != 0L) {
      pROT->Revoke(oldRegLink);
      oldRegLink = 0L;
    }
    pROT->Release();
  }
  return hr;
}


HRESULT _IFUNC BOleSite::GetMoniker (DWORD dwAssign, DWORD dwWhichMoniker, IMoniker* FAR* ppmk)
{
  // If we're open in a server doc its relative to the one in pmkDoc
  // If we're an embedding, its relative to the container's doc
  HRESULT hr = ResultFromScode(E_FAIL);
  *ppmk = NULL;
  if (pClientSite) {
    hr = pClientSite->GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
      OLEWHICHMK_OBJFULL, ppmk);
  }
  else if (pmkDoc && pszInstName) {
    LPMONIKER pmkObj = NULL;

    if (SUCCEEDED(CreateItemMoniker (OLESTR("!"), pszInstName, &pmkObj))) {
      hr = CreateGenericComposite(pmkDoc, pmkObj, ppmk);
      pmkObj->Release();
    }
  }
  else if (pmkDoc && !pszInstName) {
    *ppmk = pmkDoc;
    pmkDoc->AddRef();
    hr = NOERROR;
  }
  return hr;
}

HRESULT _IFUNC BOleSite::InitFromData (IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::GetClipboardData (DWORD dwReserved, IDataObject* FAR* ppDataObject)
{
  return ResultFromScode (E_NOTIMPL);
}

// Put external lock on document
HRESULT _IFUNC BOleSite::Lock (BOOL fLock, BOOL fLastUnlockReleases)
{
  return CoLockObjectExternal (AsPIUnknown(pObjOuter), fLock, fLastUnlockReleases);
}

HRESULT _IFUNC BOleSite::ShowWindow ()
{
  HRESULT hrErr = NOERROR;
  if (fShown)
    hrErr = pPart->Open(TRUE);    // show window and set focus anyway
  else {
    // If there's no pClientSite, this is a linked object
    //
    if (!pClientSite) {
      hrErr = pPart->Open(TRUE);
      fShown = SUCCEEDED(hrErr);
    }
    else {
      // This is an embedded object. ask container to show itself and object
      //
      pClientSite->ShowObject();
      if (!fInPlaceActive) {
        hrErr = pPart->Open(TRUE);    // show app main window
        fShown = SUCCEEDED(hrErr);
        if (fShown)       // show open edit hatching
          pClientSite->OnShowWindow (TRUE);
      }
      else
        fShown = TRUE;
    }
    if (fShown)
      Lock (TRUE, FALSE);    // lock document till window is hidden
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::HideWindow (BOOL fShutdown)
{
  HRESULT hrErr = NOERROR;
  if (fShown) {
    fShown = FALSE;

    // If there's no pClientSite, this is a linked object
    //
    if (!pClientSite)
      hrErr = pPart->Open(FALSE);
    else {
      // This is an embedded object.
      // inform container that our window is hiding
      //
      if (!fInPlaceVisible) {
        pClientSite->OnShowWindow (FALSE);    // hide open edit hatching
        hrErr = pPart->Open(FALSE);
      }
    }

    // Must unlock at the bottom in case object gets deleted
    //
    Lock (FALSE, fShutdown);
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::AssembleMenus ()
{
  HRESULT hrErr = NOERROR;

  hMenuShared = CreateMenu ();
  if (hMenuShared) {
    //\\// zero's menu descriptor
    for (int i = 0; i < 6; i++) {  menuGroupWidths.width[i] = 0; }
    hrErr = pIPFrame->InsertMenus (hMenuShared, &menuGroupWidths);
    if (hrErr == NOERROR) {
      hrErr = pPart->InsertMenus (hMenuShared, (BOleMenuWidths *)&menuGroupWidths);
    }
    else {
      // Even when the in-place container does not allow menu merging
      // the server must still call OleCreateMenuDescriptor(NULL)
      //
      DestroyMenu (hMenuShared);
      hMenuShared = NULL;
    }
  }
  hOleMenu = OleCreateMenuDescriptor (hMenuShared, &menuGroupWidths);
  if (!hOleMenu)
    hrErr = ResultFromScode (E_OUTOFMEMORY);
  return hrErr;
}

void _IFUNC BOleSite::DisassembleMenus ()
{
  if (hOleMenu) {
    OleDestroyMenuDescriptor (hOleMenu);
    hOleMenu = 0;
  }
  if (hMenuShared) {
    UINT uDeleteAt = 0;
    for (UINT uGroup = 0; uGroup < 6; uGroup++) {
      uDeleteAt += menuGroupWidths.width [uGroup++];
      for (UINT uCount = 0; uCount < menuGroupWidths.width [uGroup]; uCount++)
      DeleteMenu (hMenuShared, uDeleteAt, MF_BYPOSITION);
    }
    if (pIPFrame)
      pIPFrame->RemoveMenus (hMenuShared);
    DestroyMenu (hMenuShared);
    hMenuShared = 0;
  }
}

HRESULT _IFUNC BOleSite::DoInPlaceActivate (BOOL fActivateOnly)
{
  HRESULT hrErr = NOERROR;
  if (pClientSite && !fInPlaceActive /*&& !fShown*/) {
    hrErr = pClientSite->QueryInterface(IID_IOleInPlaceSite,&(void *)pIPSite);
    if (SUCCEEDED(hrErr)) {
      hrErr = pIPSite->CanInPlaceActivate ();
      if (GetScode(hrErr) == S_OK) { //returns S_FALSE instead of E_FAIL
        hrErr = pIPSite->OnInPlaceActivate ();
        if (SUCCEEDED(hrErr)) {
          fInPlaceActive = TRUE;
          hrErr = DoInPlaceShow (fActivateOnly);
          if (SUCCEEDED(hrErr))
            hrErr = DoUIActivate ();
        }
        else {
          pIPSite->Release();
          pIPSite = 0;
        }
      }
      else {
        pIPSite->Release();
        pIPSite = 0;
      }
    }
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::CalcZoom( LPCRECT rcPosRect )
{
  SIZE sCurrent;
  HRESULT hr = pProvider->GetPartSize (&sCurrent);
  if (SUCCEEDED(hr)) {
    if (sCurrent.cx == 0) {
      scale.xN = 1;
      scale.xD = 1;
    } else {
      scale.xN = rcPosRect->right - rcPosRect->left;
      scale.xD = sCurrent.cx;
    }
    if (sCurrent.cy == 0) {
      scale.yN = 1;
      scale.yD = 1;
    } else {
      scale.yN = rcPosRect->bottom - rcPosRect->top;
      scale.yD = sCurrent.cy;
    }
  }
  return hr;
}

HRESULT _IFUNC BOleSite::DoInPlaceShow (BOOL fActivateOnly)
{
  HRESULT hrErr = NOERROR;
  if (!fInPlaceVisible) {
    fInPlaceVisible = TRUE;
    hrErr = pIPSite->GetWindow (&hWndParent);
    if (SUCCEEDED(hrErr)) {
      frameInfo.cb = sizeof (frameInfo);
      hrErr = pIPSite->GetWindowContext (
        &pIPFrame, &pIPDoc, &rcPosRect, &rcClipRect, &frameInfo);

      CalcZoom( &rcPosRect );

      if (SUCCEEDED(hrErr) && pIPFrame) {
        // set message filter (ICS)

        hWndInPlace = pPart->OpenInPlace (hWndParent);

        AssembleMenus ();
        if (fInsideOut && fActivateOnly) {
          // show object without adornments and border
          pPart->SetPartPos (&rcPosRect);
        }
      }
      else {
        DoInPlaceDeactivate ();
      }
    }
    else {
      DoInPlaceDeactivate ();
    }
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::DoUIActivate ()
{
  HRESULT hrErr = NOERROR;
  if (!fUIActive) {
    fUIActive = TRUE;
    hrErr = pIPSite->OnUIActivate ();
    if (SUCCEEDED(hrErr)) {

      if (pHatchWnd) {

        // Put the hatch window around the inplace window. The offset
        // is necessary because we're reparenting the inplace window
        // behind the scenes, so the border of the hatch window would
        // throw rcPosRect off.
        //
        ::SetParent (hWndInPlace, *pHatchWnd);
        ::SetParent (*pHatchWnd, hWndParent);
        pHatchWnd->SetSize (&rcPosRect, &rcClipRect, &hatchOffset);
        pHatchWnd->Show (TRUE, pIPSite);
        ::OffsetRect (&rcPosRect, hatchOffset.x, hatchOffset.y);
      }

      pPart->SetPartPos (&rcPosRect);
      pPart->Show(TRUE); // Show after SetPos to prevent flashing

      // window position of part may need to move but the extent
      // should be the same
      pPart->Activate (TRUE);
      pIPFrame->SetActiveObject (this, pInPlaceName);
      if (pIPDoc)
        pIPDoc->SetActiveObject (this, pInPlaceName);
      pIPFrame->SetMenu (hMenuShared, hOleMenu, hWndInPlace);
      pPart->ShowTools (TRUE);

    }
    else {
      fUIActive = FALSE;
    }
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::DoInPlaceDeactivate ()
{
  HRESULT hrErr = NOERROR;
  if (fInPlaceActive) {

    hrErr = DoUIDeactivate ();
    if (SUCCEEDED(hrErr) && fInPlaceActive) {
      fInPlaceActive = FALSE;

      pIPSite->OnInPlaceDeactivate ();
      pIPSite->Release();
      pIPSite = 0;
    }
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::DoUIDeactivate ()
{
  if (!SUCCEEDED(pPart->Show (FALSE)))
    return ResultFromScode (E_FAIL); // Hide here to match Show which prevents flashing

  HRESULT hrErr = NOERROR;
  if (fUIActive) {
    fUIActive = FALSE;
    // remove shading and adornments
    if (pIPDoc)
      pIPDoc->SetActiveObject (NULL, NULL);
    if (pIPFrame)
      pIPFrame->SetActiveObject (NULL, NULL);
    pPart->ShowTools (FALSE);
    pIPSite->OnUIDeactivate (FALSE);
    // deactivate contained objects if any
    if (!fInsideOut) {
      DoInPlaceHide ();
      if (pIPFrame) {
        pIPFrame->Release();
        pIPFrame = 0;
      }
      if (pIPDoc) {
        pIPDoc->Release();
        pIPDoc = 0;
      }
    }
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::DoInPlaceHide ()
{
  HRESULT hrErr = NOERROR;
  if (fInPlaceVisible) {
    // deactivate contained objects (ICS)
    // remove shading and adornments (ICS)
    hrErr = pPart->OpenInPlace (0) == NULL ? ResultFromScode (E_FAIL) : NOERROR;
    if (pHatchWnd)
      // Hide the hatch window
      //
      pHatchWnd->Show (FALSE);

    // set filter ? (ICS)
    DisassembleMenus ();
    HideWindow (FALSE);
    fInPlaceVisible = FALSE;
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::DoVerb (LONG lVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
{
  HRESULT hrErr = NOERROR;

  switch (lVerb) {

  case OLEIVERB_PRIMARY:
  default:
   // First ask the server app if it knows this verb
   //
   hrErr = pPart->DoVerb ((UINT)lVerb);
   if (SUCCEEDED(hrErr))
    return hrErr;

   // If the server app doesn't know it, and it's a predefined (<0)
   // verb, return an error
   //
   hrErr = ResultFromScode (OLEOBJ_S_INVALIDVERB);
   if (lVerb < 0)
    OLERES(hrErr);

   // If it's not a predefined verb and we don't know it, deliberately
   // fall through to Primary Verb

  case OLEIVERB_SHOW:

   // If the window is already open (primary verb has already been
   // run, just bring the window to the front
   //
   if (pClientSite && !fShown && !fInPlaceActive &&
      (S_OK == GetScode(pPart->CanOpenInPlace())))
      hrErr = DoInPlaceActivate (FALSE);
    else if (fInPlaceActive && !fInPlaceVisible) {
      if (SUCCEEDED(DoInPlaceShow (FALSE))) {
        hrErr = DoUIActivate ();
      }
    }
    hrErr = ShowWindow ();
    break;
  case 1:
  case OLEIVERB_OPEN:
    if (pClientSite && fInPlaceActive)
      hrErr = DoInPlaceDeactivate ();
    hrErr = ShowWindow ();
    break;
  case OLEIVERB_HIDE:
    if (pClientSite && fInPlaceActive)
      hrErr = DoInPlaceHide ();
    else
      hrErr = HideWindow (TRUE /*fShutdown*/);
    break;
  case OLEIVERB_UIACTIVATE:
  case OLEIVERB_INPLACEACTIVATE:

    // If our window is already open-editing we can't activate in-place.
    //
    if (pClientSite && !fShown && !fInPlaceActive &&
      (S_OK == GetScode(pPart->CanOpenInPlace())))
      hrErr = DoInPlaceActivate (lVerb == OLEIVERB_INPLACEACTIVATE);
    else
      hrErr = ResultFromScode (OLE_E_NOT_INPLACEACTIVE);
    break;
  }
  return hrErr;
}

HRESULT _IFUNC BOleSite::EnumVerbs (IEnumOLEVERB* FAR* ppenumOleVerb)
{
  return OleRegEnumVerbs(cid, ppenumOleVerb);
}

HRESULT _IFUNC BOleSite::Update ()
{
  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::IsUpToDate ()
{
  return ResultFromScode(MK_E_UNAVAILABLE);
}

HRESULT _IFUNC BOleSite::GetUserClassID (CLSID FAR* pClsid)
{
  *pClsid = cid;
  return NOERROR;
}

HRESULT _IFUNC BOleSite::GetUserType (DWORD dwFormOfType, LPOLESTR FAR* pszUserType)
{
  return ResultFromScode (OLE_S_USEREG);
}

HRESULT _IFUNC BOleSite::SetExtent (DWORD dwDrawAspect, LPSIZEL lpsizel)
{
  RECT pos;
  pos.left = pos.top = 0;
  pos.right = MAP_LOGHIM_TO_PIX (lpsizel->cx, BOleService::pixPerIn.x);
  pos.bottom = MAP_LOGHIM_TO_PIX (lpsizel->cy, BOleService::pixPerIn.y);
  SIZE size;
  size.cx = pos.right;
  size.cy = pos.bottom;

  HRESULT hr = pPart->SetPartSize(&size);

  if (SUCCEEDED(hr)) {
    Invalidate(BOLE_INVAL_VIEW);
    pPart->SetPartPos (&pos);
  }

  return hr;
}

HRESULT _IFUNC BOleSite::GetExtent (DWORD dwDrawAspect, LPSIZEL lpsizel)
{
  if (dwDrawAspect != DVASPECT_CONTENT) //PHP for now.
    return ResultFromScode (E_FAIL);

  if (!lpsizel)
    return ResultFromScode (E_INVALIDARG);

  SIZE s;
  HRESULT hr = pProvider->GetPartSize (&s); //PHP do we need aspect here?

  lpsizel->cx = MAP_PIX_TO_LOGHIM (s.cx, BOleService::pixPerIn.x);
  lpsizel->cy = MAP_PIX_TO_LOGHIM (s.cy, BOleService::pixPerIn.y);

  return hr;
}

HRESULT _IFUNC BOleSite::Advise (IAdviseSink* pAdvSink, DWORD FAR* pdwConnection)
{
  HRESULT hrErr;

  *pdwConnection = 0;
  if (pAdvHolder == 0)
    hrErr = CreateOleAdviseHolder (&pAdvHolder);
  if (pAdvHolder)
    hrErr = pAdvHolder->Advise (pAdvSink, pdwConnection);
  else
    hrErr = ResultFromScode (E_OUTOFMEMORY);
  return hrErr;
}

HRESULT _IFUNC BOleSite::Unadvise (DWORD dwConnection)
{
  HRESULT hrErr = ResultFromScode (E_FAIL);
  if (pAdvHolder)
    hrErr = pAdvHolder->Unadvise (dwConnection);

  return hrErr;
}

HRESULT _IFUNC BOleSite::EnumAdvise (IEnumSTATDATA* FAR* ppenumAdvise)
{
  HRESULT hrErr = ResultFromScode (E_FAIL);
  if (pAdvHolder)
    hrErr = pAdvHolder->EnumAdvise (ppenumAdvise);

  return hrErr;
}

HRESULT _IFUNC BOleSite::GetMiscStatus (DWORD dwAspect, DWORD FAR* pdwStatus)
{
  return ResultFromScode (OLE_S_USEREG);
}

HRESULT _IFUNC BOleSite::SetColorScheme (LPLOGPALETTE lpLogpal)
{
  return ResultFromScode (E_NOTIMPL);
}

//**************************************************************************
//
// IPersist implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::GetClassID (LPCLSID lpClassID)
{
  *lpClassID = cid;
  return NOERROR;
}

HRESULT _IFUNC BOleSite::IsDirty ()
{
  return ResultFromScode((fIsDirty) ? S_OK : S_FALSE);
}

//**************************************************************************
//
// IPersistStorage implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::InitNew (IStorage* pStgNew)
{
  if (pStg) {
    pStg->Release ();
    pStg = NULL;
  }
  pStg = pStgNew;
  if (pStg) {
    pStg->AddRef();
  }
  BOleInitInfo bi;
  bi.pContainer = NULL;
  bi.Where = BOLE_NEW;
  bi.How = BOLE_EMBED;
  bi.pStorage = pStgNew;

  HRESULT hr = pPart->Init(this, &bi);
  if (!SUCCEEDED(hr))
    return hr;

  Invalidate(BOLE_INVAL_DATA);

  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::Load (IStorage* pStgFrom)
{
  HRESULT hresult = NOERROR;

  fIsDirty = FALSE;       // data will be the same as what was saved

  if (pStg) {
    pStg->Release ();
    pStg= NULL;
  }
  pStg = pStgFrom;
  pStg->AddRef();

  BOleInitInfo bi;
  bi.pContainer = NULL;
  bi.Where = BOLE_STORAGE;
  bi.How = BOLE_EMBED;
  bi.pStorage = pStg;
  hresult = pPart->Init (this, &bi);

  return hresult;
}

HRESULT _IFUNC BOleSite::Save (IStorage* pStgSave, BOOL fSameAsLoad)
{
//      if (fSameAsLoad && !fIsDirty)  // don't bother, its up to date
//        return NOERROR;

  HRESULT hresult = ResultFromScode(S_OK);

  if (!pStgSave && fSameAsLoad)
    pStgSave = pStg;

  // The online dox say that OleSave does this, but there's no
  // CompObj stream without WriteFmtUseTypeStg and no CLSID in the
  // stream without WriteClassStg
  //
  CLSID clsid = CLSID_NULL;
  CLSIDFromProgID (pszProgID, &clsid);
  if ((hresult = WriteClassStg (pStgSave, clsid)) != NOERROR)
    return hresult;
  LPOLESTR lpLongType = NULL;
  OleRegGetUserType (clsid, USERCLASSTYPE_FULL, &lpLongType);
  hresult = WriteFmtUserTypeStg (pStgSave, BOleDocument::oleEmbSrcClipFmt, lpLongType);

  // We pass along the fHandsOff flag here to "fRemember"
  // If HandsOffStorage was called before it means SaveAs is happening
  // and objects need to remember their new substorages as they create them.
  // We use the fRemember flag to do what what HandsOffStorage and
  // SaveCompleted do (we can't scribble to storages between saves anyway)

  if (SUCCEEDED(pProvider->Save( pStgSave, fSameAsLoad, fHandsOff))) {

    // since we don't scribble, do the SaveCompleted stuff here
    // where we know fSameAsLoad

    if (fHandsOff || fSameAsLoad) {    // ! save copy as situation
      fIsDirty = FALSE;
      if (pAdvHolder)
        hresult = pAdvHolder->SendOnSave();
      if (fHandsOff) {
        fHandsOff = FALSE;
        pStg = pStgSave;  // pStg was Released in HandsOffStorage
        if (pStg)
          pStg->AddRef();
      }
    }
  }
  else
    hresult = ResultFromScode(STG_E_CANTSAVE);

  return hresult;
}

HRESULT _IFUNC BOleSite::SaveCompleted (IStorage* pStgNew)
{
  if (fHandsOff) {    // HandsOffStorage, SaveCompleted without Save
    Save(pStgNew, FALSE);
  }
  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::HandsOffStorage ()
{
  fHandsOff = TRUE;
  if (pStg) {
    pStg->Release();
    pStg = NULL;
  }
  return ResultFromScode(S_OK);
}


//**************************************************************************
//
// IDropSource implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::QueryContinueDrag (BOOL fEscapePressed, DWORD grfKeyState)
{
  if (fEscapePressed) {
    OLERET (DRAGDROP_S_CANCEL);
  }

  if (!(grfKeyState & (MK_LBUTTON | MK_RBUTTON))) {
    OLERET (DRAGDROP_S_DROP);
  }

  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::GiveFeedback (DWORD dwEffect)
{
  POINT p = {0,0};
  BOOL fNearScroll = FALSE;

  // of IBDataProvider so we can do drag/drop with only a provider
  //
  if (!pPart)
    return ResultFromScode (DRAGDROP_S_USEDEFAULTCURSORS);

  if (S_OK == GetScode(pPart->DragFeedback (&p, fNearScroll))) {
    return ResultFromScode (DRAGDROP_S_USEDEFAULTCURSORS);
  }
  else {
    return ResultFromScode (S_OK);
  }
}

//**************************************************************************
//
// IOleWindow implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::GetWindow (HWND FAR* lphwnd)
{
  if (lphwnd) {
    *lphwnd = hWndInPlace;
    return NOERROR;
  }
  return ResultFromScode (E_INVALIDARG);
}

HRESULT _IFUNC BOleSite::ContextSensitiveHelp (BOOL fEnterMode)
{
  return pService->ContextSensitiveHelp (fEnterMode);
}

//**************************************************************************
//
// IOleInPlaceObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::InPlaceDeactivate ()
{
  return DoInPlaceDeactivate ();
}

HRESULT _IFUNC BOleSite::UIDeactivate ()
{
  return DoUIDeactivate ();
}

HRESULT _IFUNC BOleSite::SetObjectRects (LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
  rcPosRect = *lprcPosRect;
  rcClipRect = *lprcClipRect;
  RECT r;

  if (pHatchWnd) {
//        if (! fUIActive) // hatch and adornments should not be drawn
//          lprcClipRect = lprcPosRect;

    pHatchWnd->SetSize (&rcPosRect, &rcClipRect, &hatchOffset);
    r = rcPosRect;

    // offset hatch border

    ::OffsetRect (&r, hatchOffset.x, hatchOffset.y);

  }
  else
    r = rcPosRect;


  CalcZoom (lprcPosRect);

  pPart->SetPartPos (&r);

  return NOERROR;
}

HRESULT _IFUNC BOleSite::GetZoom( BOleScaleFactor *pScale)
{
  if (fInPlaceActive) {
    *pScale = scale;
  }
  else {
    pScale->xN = 1;
    pScale->yN = 1;
    pScale->xD = 1;
    pScale->yD = 1;
  }
  return NOERROR;
}

HRESULT _IFUNC BOleSite::ReactivateAndUndo ()
{
  HRESULT hrErr;
  if (pClientSite && !fShown && !fInPlaceActive &&
    (S_OK == GetScode(pPart->CanOpenInPlace())))
    hrErr = DoInPlaceActivate (OLEIVERB_INPLACEACTIVATE);
  else
    hrErr = ResultFromScode (OLE_E_NOT_INPLACEACTIVE);

  // do undo?

  return hrErr;
}

//**************************************************************************
//
// IOleInPlaceActiveObject implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::TranslateAccelerator (LPMSG lpmsg)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::OnFrameWindowActivate (BOOL fActivate)
{
  // post message to server app ? (ICS)
  return NOERROR;
}

HRESULT _IFUNC BOleSite::OnDocWindowActivate (BOOL fActivate)
{
  if (fActivate) {
    pIPFrame->SetActiveObject ((IOleInPlaceActiveObject *)this, pInPlaceName);
    pIPFrame->SetMenu (hMenuShared, hOleMenu, hWndInPlace);
    pPart->ShowTools (TRUE);
  }
  else {
    pIPFrame->SetActiveObject (NULL, NULL);

    pPart->ShowTools (FALSE);
    // deactivate contained objects if any (ICS)
  }
  return NOERROR;
}

HRESULT _IFUNC BOleSite::ResizeBorder (LPCRECT lprectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL fFrameWindow)
{
  pPart->FrameResized(lprectBorder, fFrameWindow);
  return ResultFromScode(S_OK);
}

HRESULT _IFUNC BOleSite::EnableModeless (BOOL fEnable)
{
  return pService->GetApplication()->OnModalDialog (!fEnable);
}

//**************************************************************************
//
//    IContainer implementation
//
//**************************************************************************

HRESULT _IFUNC BOleSite::AllowInPlace()
{
  // anything we can use this for on server side??
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::BringToFront()
{
  // anything we can use this for on server side??
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::FindDropDest (LPPOINT, PIBDropDest FAR*)
{
  // anything we can use this for on server side??
  return ResultFromScode (E_NOTIMPL);
}

HWND _IFUNC BOleSite::GetWindow()
{
  HWND hWnd;
  return (pIPSite && pIPSite->GetWindow (&hWnd) == NOERROR) ? hWnd : 0;
}

HRESULT _IFUNC BOleSite::GetWindowRect(LPRECT pR)
{
  if (pIPDoc)
    return pIPDoc->GetBorder (pR);

  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleSite::RequestBorderSpace(LPCRECT pR)
{
  BORDERWIDTHS r;
  if (pR) {
    r = *pR;  // Word bums modify our LPCRECT contents
    pR = &r;
  }
  if (pIPDoc)
    return pIPDoc->RequestBorderSpace (pR);

  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleSite::SetBorderSpace(LPCRECT pR)
{
  BORDERWIDTHS r;
  if (pR) {
    r = *pR;  // Word bums modify our LPCRECT contents
    pR = &r;
  }

  if (pIPDoc)
    return pIPDoc->SetBorderSpace (pR);

  return ResultFromScode (E_FAIL);
}

LPCOLESTR _IFUNC BOleSite::GetWindowTitle()
{
  return pWindowTitle;
}

void _IFUNC BOleSite::AppendWindowTitle(LPCOLESTR)
{
  //PHP Do we need to call SetActiveObject???
}

//**************************************************************************
//
// ISite implementation
//
//**************************************************************************


HRESULT _IFUNC BOleSite::SiteShow(BOOL)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::DiscardUndo()
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::GetSiteRect(LPRECT prcPos,LPRECT prcClip)
{
  if( prcPos )
    *prcPos = rcPosRect;
  if( prcClip )
    *prcClip = rcClipRect;

  return NOERROR;
}

HRESULT _IFUNC BOleSite::SetSiteRect(LPCRECT pR)
{
  // Can be useful for hatch window implementations or resizing
  // the server when in-place active
  HRESULT hr;
  SIZE s;
  s.cx = pR->right - pR->left;
  s.cy = pR->bottom - pR->top;

  if (SUCCEEDED(hr = SetSiteExtent(&s))) {
    // Ask the client to resize to the new size of the window
    if (pIPSite) {
      hr = pIPSite->OnPosRectChange (pR);
    }
  }
  return hr;
}

HRESULT _IFUNC BOleSite::SetSiteExtent (LPCSIZE pS)
{
  SIZE s = *pS;
  // Servers call this to update their own scaled size
  // and notify the container of the change

  BOleScaleFactor scale;
  GetZoom(&scale);
  s.cx = s.cx * scale.xD / scale.xN; // unscale
  s.cy = s.cy * scale.yD / scale.yN;
  HRESULT hr = pPart->SetPartSize(&s);
  if (SUCCEEDED(hr)) {
    // Invalidate the metafile since it's size has changed
    Invalidate(BOLE_INVAL_VIEW);
  }
  return hr;
}

void _IFUNC BOleSite::Invalidate(BOleInvalidate inval)
{
  // The PERSISTENT flag is used for servers "Update" menu.
  //
  if (inval & BOLE_INVAL_PERSISTENT && pClientSite) {
    pClientSite->SaveObject ();
    fIsDirty = FALSE;
  }

  // We're not distinguishing between the other two flags on the
  // server side
  //
  if (inval & BOLE_INVAL_VIEW || inval & BOLE_INVAL_DATA) {
    fIsDirty = TRUE;
    if (pDAdvHolder)
      pDAdvHolder->SendOnDataChange (this, 0, 0);
  }
}

void _IFUNC BOleSite::OnSetFocus(BOOL bSet)
{
  pService->SetFocusedSite( bSet ? this : NULL );
}

void _IFUNC BOleSite::Disconnect ()
{

  Close(OLECLOSE_SAVEIFDIRTY);
}

//**************************************************************************
//
// IApplication implementation
//
//**************************************************************************

LPCOLESTR _IFUNC BOleSite::IBApplicationImpl::GetAppName()
{
  return pThis->pAppName;
}

// This is called when an inplace active object receives a Shift-F1
// keystroke. We need to call our container so that other objects can
// be prepared to give context-sensitive help if clicked.
//
BOleHelp _IFUNC BOleSite::IBApplicationImpl::HelpMode (BOleHelp newMode)
{
  BOleHelp oldMode = (BOleHelp) pThis->fInHelpMode;
  if (newMode != BOLE_HELP_GET)
    if (pThis->pIPSite) {
      pThis->fInHelpMode = newMode == BOLE_HELP_ENTER;
      pThis->pIPSite->ContextSensitiveHelp (pThis->fInHelpMode);
    }

  return oldMode;
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::InsertContainerMenus(HMENU hMenu,BOleMenuWidths FAR* pMW)
{
  return pThis->InsertContainerMenus(hMenu, pMW);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::SetFrameMenu (HMENU h)
{
  return pThis->SetFrameMenu(h);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::Accelerator(MSG FAR*pMsg)
{
  return pThis->Accelerator(pMsg);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::GetAccelerators (HACCEL FAR*, int FAR*)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::CanLink()
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::CanEmbed()
{
  return ResultFromScode (E_NOTIMPL);
}


HWND  _IFUNC BOleSite::IBApplicationImpl::GetWindow ()
{
  HWND hWnd;
  return (pThis->pIPFrame && pThis->pIPFrame->GetWindow (&hWnd) == NOERROR) ? hWnd : 0;
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::RequestBorderSpace (LPCRECT pR)
{
  BORDERWIDTHS r;
  if (pR) {
    r = *pR;      // Word modifies our LPCRECT contents
    pR = &r;
  }

  if (pThis->pIPFrame)
    return pThis->pIPFrame->RequestBorderSpace (pR);

  return ResultFromScode (E_FAIL);
}

HRESULT  _IFUNC BOleSite::IBApplicationImpl::SetBorderSpace (LPCRECT pR)
{
  BORDERWIDTHS r;
  if (pR) {
    r = *pR;  // Word bums modify our LPCRECT contents
    pR = &r;
  }

  if (pThis->pIPFrame)
    return pThis->pIPFrame->SetBorderSpace (pR);

  return ResultFromScode (E_FAIL);
}


void  _IFUNC BOleSite::IBApplicationImpl::AppendWindowTitle (LPCOLESTR s)
{
  if (pThis->pIPFrame)
    pThis->pIPFrame->SetActiveObject(pThis, s);
}

HRESULT  _IFUNC BOleSite::IBApplicationImpl::SetStatusText (LPCOLESTR s)
{
  if (pThis->pIPFrame)
    return pThis->pIPFrame->SetStatusText(s);
  return ResultFromScode(E_FAIL);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::IsMDI ()
{
  if (pThis->pIPDoc)
    return ResultFromScode (S_OK);
  else
    return ResultFromScode (S_FALSE);
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::OnModalDialog (BOOL fDialogComingActive)
{
  return (pThis->pIPFrame) ?
    pThis->pIPFrame->EnableModeless (!fDialogComingActive) :
    NOERROR;
}

void _IFUNC BOleSite::IBApplicationImpl::RestoreUI ()
{
  //PHP Server object should use this instead of SetBorderSpace(NULL)
  //    and AppendWindowTitle(NULL). Semantics are much cleaner.
}

void _IFUNC BOleSite::IBApplicationImpl::DialogHelpNotify (BOleDialogHelp helpCode)
{
  // unimplemented in server side helper
}

void _IFUNC BOleSite::IBApplicationImpl::ShutdownMaybe ()
{
  // unimplemented in server side helper
}

//
// ILinkable
//

HRESULT _IFUNC BOleSite::GetMoniker(IMoniker FAR* FAR* ppMon)
{
  return GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
    OLEWHICHMK_OBJFULL, ppMon);
}

HRESULT _IFUNC BOleSite::OnRename(PIBLinkable pCont, LPCOLESTR szName)
{
  HRESULT ret = NOERROR;

  if (pCont)
    pCont->GetMoniker(&pmkDoc);
  else if (pmkDoc) {
    // Reset cached format list to cause it to be updated next time
    // (now that link format is enabled/disabled).
    if (formatList) {
      delete [] formatList;
      formatLink = FALSE;
      formatEmbed = FALSE;
      formatList = NULL;
      formatCount = 0;
    }

    if (pService->IsOnClipboard(this)) {
      // remove the linking capability from this object if one calls
      // OnRename(NULL, NULL) before Close rather during Close
      //
      AddRef();
      pService->Clip(NULL, FALSE, FALSE, FALSE);
      pService->Clip(pProvider, FALSE, pService->clipOkToEmbed, FALSE);
      Release();
    }

    if (pmkDoc) {
      pmkDoc->Release();
      pmkDoc = NULL;
    }
  }

  if (pszInstName) {
    delete [] pszInstName;
    pszInstName = NULL;
  }
  if (szName) {
    pszInstName = new OLECHAR [lstrlen (szName) +1];
    lstrcpy(pszInstName, szName);
  }

  // Don't register if this is a clone (pPart is NULL)
  // otherwise we would link to what's on the clipboard...
  //
  // Also don't bother if it is only a document (no item name)
  // (pszInstName may be NULL and pmkDoc valid, in that case
  // we shouldn't register here. BOleContainer already registered it.
  //
  if (pPart && pmkDoc && pszInstName)
    SetMoniker(OLEWHICHMK_OBJFULL, NULLP);  // register

  if (!pmkDoc && (regLink != 0L)) {
    // Unregister for linking if pmkDoc is NULL
    LPRUNNINGOBJECTTABLE pROT = NULL;
    HRESULT hr = OLE::GetRunningObjectTable (0, &pROT);
    if (SUCCEEDED(hr)) {
      pROT->Revoke(regLink);
      regLink = 0L;
      pROT->Release();
    }
  }

  return ret;
}


HRESULT     _IFUNC BOleSite::SetStatusText (LPCOLESTR s)
{
  if (pIPFrame)
    return pIPFrame->SetStatusText(s);
  return ResultFromScode(E_FAIL);
}



LPCOLESTR _IFUNC BOleSite::IBApplicationImpl::GetWindowTitle()
{
  return pThis->pWindowTitle;
}

HRESULT _IFUNC BOleSite::IBApplicationImpl::GetWindowRect(LPRECT pR)
{
  if (pThis->pIPFrame)
    return pThis->pIPFrame->GetBorder (pR);
  return ResultFromScode(CO_E_OBJNOTCONNECTED);
}

HRESULT _IFUNC BOleSite::GetAccelerators( HACCEL FAR *,int *)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleSite::Accelerator(MSG FAR*pMsg)
{
  // Give the container a chance to handle its accelerators.
  // Returns S_OK if processed or S_FALSE if no accelerator was met.
  if (pIPFrame)
    return OleTranslateAccelerator( pIPFrame, &frameInfo, pMsg );
  return ResultFromScode(S_FALSE);
}

void _IFUNC BOleSite::RestoreUI()
{
}

// This little trick allows Bolero servers to dynamically renegotiate
// the menu bar in case some state changes while they're inplace active
//
HRESULT _IFUNC BOleSite::SetFrameMenu(HMENU)
{
  // First get the client to let go of the menus so we can put new ones up
  //
  if (pIPFrame)
    pIPFrame->SetMenu (NULL, hOleMenu, hWndInPlace);

  DisassembleMenus ();       // Tear down the old shared menu
  HRESULT hr = AssembleMenus ();   // Put together a new shared menu
  if (!SUCCEEDED(hr))
    return hr;
                  // Tell the client to install it
  return pIPFrame->SetMenu (hMenuShared, hOleMenu, hWndInPlace);
}

HRESULT _IFUNC BOleSite::InsertContainerMenus(HMENU hMenu,BOleMenuWidths *)
{
  if (pIPFrame)
    return pIPFrame->InsertMenus (hMenu, &menuGroupWidths);
  return ResultFromScode (CO_E_OBJNOTCONNECTED);
}

void _IFUNC BOleSite::SetDirty (BOOL fDirty)
{
  fIsDirty = fDirty;
}

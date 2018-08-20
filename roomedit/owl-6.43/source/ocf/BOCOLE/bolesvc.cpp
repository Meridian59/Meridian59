//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the application-level object. Provides some
//  system services such as default dialogs.
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolesvc.h"
#include "boleips.h"
#include "bolepart.h"
#include "boledoc.h"
#include "bhatch.h"
#include "bolecman.h"
#include "bolefact.h"
#include "boledata.h"

#include "ole2ui.h"

struct BOleFactNode {
  IBClass *factory;
  BOleFactNode *pNext;
  BOleFactNode( IBClass *f ){ factory = f; f->AddRef(); pNext = NULL; }
  ~BOleFactNode(){ factory->Release(); }
};

BOOL IsControlClass (REFCLSID rclsid, HKEY hKey);

HRESULT _IFUNC BOleService::QueryInterfaceMain(REFIID iid, LPVOID *p)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *p = NULL;

  // interfaces

  if (SUCCEEDED(hr = IOleInPlaceFrame_QueryInterface(this, iid, p))) {
  }
  else if SUCCEEDED(hr = IBService_QueryInterface(this, iid, p)) {
  }
  else if SUCCEEDED(hr = IBService2_QueryInterface(this, iid, p)) {
  }
  // base classes
  else if SUCCEEDED(hr = BOleComponent::  QueryInterfaceMain(iid, p)) {
  }

  // helpers

  return hr;
}

POINT BOleService::pixPerIn = {0,};

BOleService::BOleService (BOleClassManager *pF, IBUnknownMain *pO) : pApp(NULL),
  pActiveDoc(NULL), pFocusedSite(NULL), BOleComponent(pF, pO),
  helpMode (BOLE_HELP_GET), pClipboardData (NULL), pSubclassedWindow(NULL),
  lpfnHelpWndProc (NULL)
{
  clipList = NULL;
  clipCount = 0;
  clipOkToLink = FALSE;
  clipOkToEmbed = FALSE;

  dropList = NULL;
  dropCount = 0;
  dropOkToLink = FALSE;
  dropOkToEmbed = FALSE;

  fUseDropList = FALSE;
  pFirstFactNode = NULL;

}

BOleService::~BOleService ()
{
  FlushClipboardData();

  if (clipList) {
    delete [] clipList;
    clipList = NULL;
  }
  if (dropList) {
    delete [] dropList;
    dropList = NULL;
  }
}

//
//        Called in BOleSite::OnClose to check if the bolesite object
//        is on the clipboard (BOleSite doesn't do delayed rendering.)
//
BOOL BOleService::IsOnClipboard(IDataObject *pPat)
{
  BOOL ret = FALSE;
  if ( pClipboardData && (S_OK == OleIsCurrentClipboard (pClipboardData))) {
    if (pPat == pClipboardData) {
      ret = TRUE;
    }
    else {
      // if the shadow object is on the clipboard we need to check
      // if the real data object pointer it holds is the same as
      // the one passed
      BOleShadowData * pShadow;
      if (SUCCEEDED(pClipboardData->QueryInterface(
          IID_BOleShadowData, &(LPVOID)pShadow))) {
        ret = (pShadow->GetRealData() == pPat);
        pShadow->Release();
      }
    }
  }
  return ret;
}

void BOleService::FlushClipboardData(BOOL bCheck)
{
  if (pClipboardData) { // one of ours may've been on the clipboard
    if (bCheck) {
      IDataObject *pD;
      if (S_OK == GetScode(OleGetClipboard(&pD))) {
  if (pD == pClipboardData)
    OleFlushClipboard();
  pD->Release();
      }
    }
    else
      OleFlushClipboard();

    pClipboardData = NULL;
  }
}

void BOleService::NotifyClipboardEmptied()
{
  pClipboardData = NULL;
}

HRESULT _IFUNC BOleService::Init( IBApplication *pA )
{

#ifdef _DEBUG
  //  to make sure client's QueryInterface is implemented correctly
  //
  IBApplication *dbgApp = NULL;
  LPOLEINPLACEFRAME dbgIPFrame = NULL;
  if ((!SUCCEEDED(pObjOuter->QueryInterfaceMain(IID_IBApplication, &(LPVOID)dbgApp)))
       || (!SUCCEEDED(pObjOuter->QueryInterfaceMain(IID_IOleInPlaceFrame,&(LPVOID)dbgIPFrame)))) {
    MessageBox (NULL, TEXT("QueryInterface Aggregation isn\'t working right"),
          TEXT("BOleService::Init"), MB_OK);
  }
  else {
    dbgApp->Release();
    dbgIPFrame->Release();
    dbgApp = NULL;
    dbgIPFrame = NULL;
  }
#endif

  if (!pA)
    return ResultFromScode(E_INVALIDARG);

  pApp = pA;
  BOleHatchWindow::Register (boleInst);

  // The screen is the same resolution for all windows and all processes
  // so we'll just squirrel this away so we don't have to retrieve it
  // every time we need to translate between himetric and pixels
  //
  HWND w = ::GetDesktopWindow ();
  HDC dc = ::GetDC (w);
  if (!w || !dc)
    return ResultFromScode (E_FAIL);
  pixPerIn.x = WIN::GetDeviceCaps(dc, LOGPIXELSX);
  pixPerIn.y = WIN::GetDeviceCaps(dc, LOGPIXELSY);
  ::ReleaseDC (w, dc);

  return NOERROR;
}

void _IFUNC BOleService::SetActiveDoc (BOleDocument FAR *pD)
{
  pActiveDoc = pD;
}

BOleDocument FAR *_IFUNC BOleService::GetActiveDoc ()
{
  return pActiveDoc;
}

// SetHelpMode is used by the other Bolero helper objects when
// one of them gets a broadcast help msg. It can come through
// any of them because help is in IOleWindow, from which they inherit.
//
void _IFUNC BOleService::SetHelpMode (BOOL fEnterMode)
{
  helpMode = fEnterMode ? BOLE_HELP_ENTER : BOLE_HELP_EXIT;
}

// Maybe useful for Activate As
//
void _IFUNC BOleService::UnloadObjects (REFCLSID c)
{
}

UINT _IFUNC BOleService::ExcludeOurselves (LPCLSID FAR *ppClsid)
{
  LPCLSID pClsid = *ppClsid;
  UINT rCnt = 128;
  UINT exclusionCount = 0;

  // Allocate an array of CLSIDs to pass back to the caller (who must delete it)
  //
  pClsid = (LPCLSID) new CLSID [rCnt];
  if (!pClsid) {
    *ppClsid = NULL;
    return 0;
  }

  BOleFactNode *pNode;
  for (pNode = pFirstFactNode; pNode; pNode = pNode->pNext ){

    // Ask the class factory if it's willing to embed in its own container
    // If it isn't willing to do this, we will exclude the CLSID for that
    // factory from the list.
    //
    if( !pNode->factory->AllowEmbedFromSelf() ){

      pNode->factory->GetClassID(&pClsid[exclusionCount]);
      exclusionCount++;
    }
  }

  *ppClsid = pClsid;
  return exclusionCount;
}

// ShowHelpButton -- BOleService helper function used by Bolero to find
//       out if the container app wants us to show the help
//       button for the dialog boxes. If the container is
//       generated by Delphi, they might not.
//
BOOL _IFUNC BOleService::ShowHelpButton (BOleDialogHelp whichDialog)
{
  BOOL ret = TRUE;
  PIBOverrideHelpButton pOHB = NULL;

  if (SUCCEEDED(pApp->QueryInterface(IID_IBOverrideHelpButton,
      &(LPVOID) pOHB))) {
    if (S_OK != GetScode (pOHB->ShowHelpButton (whichDialog)))
      ret = FALSE;
    pOHB->Release();
  }

  return ret;
}

//*************************************************************************
//
// IService implementation
//
//*************************************************************************

// Convert is supported on the service because (1) the other OLE2 dialogs
// are supported on the IBService and (2) I didn't want to make another
// function on IBPart which doesn't match up on the real server object
//
HRESULT _IFUNC BOleService::ConvertUI (PIBPart pPart, BOOL b, BOleConvertInfo FAR *pInfo)
{
  BOlePart FAR *pPartImpl = NULL;
  pPart->QueryInterface (IID_BOlePart, &(LPVOID) pPartImpl);
  HRESULT hr = pPartImpl->ConvertUI (pApp, b, pInfo);
  pPartImpl->Release();
  return hr;
}

HRESULT _IFUNC BOleService::ConvertGuts (PIBPart pPart, BOOL b, BOleConvertInfo FAR *pInfo)
{
  BOlePart FAR *pPartImpl = NULL;
  pPart->QueryInterface (IID_BOlePart, &(LPVOID) pPartImpl);
  HRESULT hr = pPartImpl->ConvertGuts (b, pInfo);
  pPartImpl->Release();
  return hr;
}

// CreateStorageOnFile is a near-trivial helper function which creates
// an IStorage on a DOS file path. Bolero users can do this for themselves
// if they like, but this is a little protection from the OLE2 headers
//
LPSTORAGE _IFUNC BOleService::CreateStorageOnFile (LPCOLESTR pFileName, BOOL bCreate)
{
  IStorage * pStorage;
  HRESULT hr;
  if (bCreate) {
    hr = StgCreateDocfile (
      pFileName,
      STGM_READWRITE|STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_CREATE,
      0,
      &pStorage
      );
  }
  else {
    hr = StgOpenStorage (
      pFileName, NULL,
      STGM_READWRITE|STGM_DIRECT|STGM_SHARE_EXCLUSIVE,
      NULL,
      0,
      &pStorage
      );
  }
  return pStorage;
}

void _IFUNC BOleService::OnResize ()
{
  if (pActiveDoc && pActiveDoc->pActivePart) {

    // Ask the active IOleInPlaceActiveObject to recalculate the
    // dimensions of its border adornments in the window, based on
    // the rectangle available in our frame
    //
    RECT r;
    pApp->GetWindowRect (&r);
    pActiveDoc->pActivePart->ResizeBorder (&r, this, TRUE);
  }
}

void _IFUNC BOleService::OnActivate (BOOL fActivate)
{
  if (pActiveDoc && pActiveDoc->pActivePart)
    pActiveDoc->pActivePart->OnFrameWindowActivate (fActivate);
}

HRESULT _IFUNC BOleService::OnSetFocus(BOOL bSet)
{
  // Just send it on to the active doc
  //
  if (pActiveDoc)
    return pActiveDoc->OnSetFocus(bSet);

  // If there's no active doc, there can't be an active object, so
  // the Bolero container app gets to keep focus
  //
  return ResultFromScode (S_OK);
}

HRESULT _IFUNC BOleService::Browse(BOleInitInfo FAR * pInfo)
{
  HRESULT ret = ResultFromScode (S_FALSE);

  // These guys are in member data so we don't have to create big objects
  // on the stack, but we can still have their lifetime extend from
  // IBService::Browse until IBPart::Init, like they couldn't if we
  // just used a stack variable for OLEUIINSERTOBJECT
  //
  dynamicScopePath[0] = 0;
  dynamicScopeClsid = CLSID_NULL;

  OLEUIINSERTOBJECT iod;
  memset(&iod, 0, sizeof(iod));
  iod.cbStruct = sizeof(iod);
  iod.dwFlags  = IOF_SELECTCREATENEW;
  iod.hWndOwner= ::GetActiveWindow();
  iod.lpszFile = dynamicScopePath;
  iod.cchFile  = _MAX_PATH;
  pInfo->pStorage = NULLP;  // User must fill this in after...

  iod.cClsidExclude = ExcludeOurselves (&iod.lpClsidExclude);

#ifdef NO_CONTROLS

  // BEGIN:  - exclude controls from insert object dlg
  HKEY hkey;
  if (RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hkey) == ERROR_SUCCESS)
  {
    DWORD dwNumSubKeys;
    RegQueryInfoKey(hkey, NULL, 0, 0, &dwNumSubKeys, NULL, NULL, NULL, NULL,
               NULL, NULL, NULL);

    for (int i = 0; i < dwNumSubKeys; i++)
    {
      char keyBuf[255];
      if (RegEnumKey(hkey, i, keyBuf, sizeof(keyBuf)-1) == ERROR_SUCCESS)
      {
        HKEY subkey;
        if (RegOpenKey(hkey, keyBuf, &subkey) == ERROR_SUCCESS)
        {
          HKEY tempkey;
          if (RegOpenKey(subkey, "Control", &tempkey) == ERROR_SUCCESS)
          {
            OLECHAR keyBufW[255];
            CLSID clsid;
            // move keyBuf (clsid) into the exclude list
            MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,keyBuf,-1,keyBufW, sizeof(keyBufW) / sizeof(OLECHAR));
            CLSIDFromString(keyBufW, &clsid);
            if (iod.cClsidExclude < 128)    // rCnt in ::ExcludeOurselves
              iod.lpClsidExclude[iod.cClsidExclude++] = clsid;
            else
            {
              RegCloseKey(tempkey);
              RegCloseKey(subkey);
              break;    // leave for loop
            }
            RegCloseKey(tempkey);
          }
          RegCloseKey(subkey);
        }
      }
    }
    RegCloseKey(hkey);
  }
  // End:
#endif

  if (ShowHelpButton(BOLE_HELP_BROWSE))
    iod.dwFlags |= IOF_SHOWHELP;

  EnterBOleDialog (iod.hWndOwner, NULL, NULL);

  UINT stat = OUI::OleUIInsertObject (&iod);
  ExitBOleDialog ();

  if (stat == OLEUI_OK) {
    DWORD dwFlags = iod.dwFlags;
    pInfo->hIcon = (dwFlags & IOF_CHECKDISPLAYASICON) ? (HICON) iod.hMetaPict : NULL;
    if (dwFlags & IOF_SELECTCREATENEW) {
      pInfo->Where = BOLE_NEW;
      pInfo->How = BOLE_EMBED;
      dynamicScopeClsid = iod.clsid;
      pInfo->whereNew.cid = (BCID) &dynamicScopeClsid;
      if (IsControlClass (dynamicScopeClsid, NULL))
        pInfo->Where = BOLE_NEW_OCX;

    }
    else if (dwFlags & IOF_SELECTCREATEFROMFILE) {
      pInfo->Where = BOLE_FILE;
      pInfo->How = (dwFlags & IOF_CHECKLINK) ? BOLE_LINK : BOLE_EMBED;
      int len = strlen (iod.lpszFile) + 1;
      pInfo->whereFile.pPath = new OLECHAR [len];
#if !defined(UNICODE)    // MS_OLEUI_DEF
      MultiByteToWideChar (CP_ACP, 0, iod.lpszFile, len,
                      (LPOLESTR)pInfo->whereFile.pPath, len);
#else
      lstrcpy ((LPOLESTR)pInfo->whereFile.pPath, iod.lpszFile);
#endif
    }
    ret = ResultFromScode (S_OK);
  }

  if (iod.lpClsidExclude)
    delete [] iod.lpClsidExclude;

  if (iod.hMetaPict) {
    ;//       OleUIMetafilePictIconFree(iod.hMetaPict);    // clean up metafile
  }
  return ret;
}

HRESULT _IFUNC BOleService::BrowseControls(BOleInitInfo FAR * pInfo)
{
  HRESULT ret = ResultFromScode (S_FALSE);

  // These guys are in member data so we don't have to create big objects
  // on the stack, but we can still have their lifetime extend from
  // IBService::Browse until IBPart::Init, like they couldn't if we
  // just used a stack variable for OLEUIINSERTOBJECT
  //
  dynamicScopePath[0] = 0;
  dynamicScopeClsid = CLSID_NULL;

  OLEUIINSERTOBJECT iod;
  memset(&iod, 0, sizeof(iod));
  iod.cbStruct = sizeof(iod);
  iod.dwFlags  = IOF_SELECTCREATENEW;
  iod.hWndOwner= ::GetActiveWindow();
  iod.lpszFile = dynamicScopePath;
  iod.cchFile  = _MAX_PATH;
  pInfo->pStorage = NULLP;  // User must fill this in after...

  iod.cClsidExclude = ExcludeOurselves (&iod.lpClsidExclude);

  if (ShowHelpButton(BOLE_HELP_BROWSE))
    iod.dwFlags |= IOF_SHOWHELP;

  EnterBOleDialog (iod.hWndOwner, NULL, NULL);

  UINT stat = OleUIInsertControl (&iod);
  ExitBOleDialog ();

  if (stat == OLEUI_OK) {
    DWORD dwFlags = iod.dwFlags;
    pInfo->hIcon = NULL;
    if (dwFlags & IOF_SELECTCREATENEW) {
      pInfo->Where = BOLE_NEW_OCX;
      pInfo->How = BOLE_EMBED;
      dynamicScopeClsid = iod.clsid;
      pInfo->whereNew.cid = (BCID) &dynamicScopeClsid;
    }
    ret = ResultFromScode (S_OK);
  }

  if (iod.lpClsidExclude)
    delete [] iod.lpClsidExclude;

  return ret;
}

// Dialog enabling strategy:
//
// BOleService::OnModalDialog is how Bolero containers and servers
// send dialog state to the outside world
//
// IBApplication::OnModalDialog is how Bolero containers and servers
// receive dialog state from the outside world
//
HRESULT _IFUNC BOleService::OnModalDialog (BOOL fEnable)
{
  HRESULT hr = NOERROR;

  // Propogate this state "down" to the active object
  //
  if (pActiveDoc)
    hr = pActiveDoc->OnModalDialog (fEnable);

  // Propogate this state "up" to the inplace frame
  //
  if (pFocusedSite) {
    PIBApplication tmpapp = NULL;
    if (SUCCEEDED(pFocusedSite->QueryInterface(
      IID_IBApplication, &(LPVOID) tmpapp))) {
      hr = tmpapp->OnModalDialog (fEnable);
      tmpapp->Release();
    }
  }

  // Propogate this state "back" to the application. Since Bolero
  // calls OnModalDialog itself for the OLE2 dialog boxes, the app
  // wouldn't otherwise know we're in a modal dialog
  //
  hr = pApp->OnModalDialog (fEnable);

  return hr;
}

HRESULT _IFUNC BOleService::TranslateAccel (LPMSG pMsg)
{
  HRESULT hR = ResultFromScode( S_FALSE );
  if( pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST ){
    if( pFocusedSite )
      hR = pFocusedSite->Accelerator( pMsg );
    else if( pActiveDoc )
      hR = pActiveDoc->TranslateAccel( pMsg );
  }
  return hR;
}

BOleHelp _IFUNC BOleService::HelpMode (BOleHelp newMode)
{
  BOleHelp oldMode = helpMode;
  if (newMode != BOLE_HELP_GET)
    // broadcast to all documents
    if (pActiveDoc && pActiveDoc->pActivePart)
      pActiveDoc->pActivePart->ContextSensitiveHelp (newMode == BOLE_HELP_ENTER);

  return oldMode;
}

HRESULT _IFUNC BOleService::CanClose()
{
  if (pFactory->ServerCount() == 0)
    return ResultFromScode (S_OK);
  else
    return ResultFromScode (S_FALSE);
}

HRESULT _IFUNC BOleService::BrowseClipboard (BOleInitInfo FAR * pInfo)
{
  if (TRUE == PasteHelper (pInfo, TRUE))
    return ResultFromScode (S_OK);
  else
    return ResultFromScode (S_FALSE);
}

HRESULT _IFUNC BOleService::Paste (BOleInitInfo *pInfo)
{
  if (TRUE == PasteHelper (pInfo, FALSE))
    return ResultFromScode (S_OK);
  else
    return ResultFromScode (S_FALSE);
}

// This function is called when a moniker becomes invalid for an object
// on the clipboard
//
void _IFUNC BOleService::RemoveLinkFromClipList()
{
  UINT i=0, from=0;
  for (; i < clipCount; i++, from++) {
    if ((clipList[i].cfFormat == BOleDocument::oleLinkSrcDescFmt) ||
      (clipList[i].cfFormat == BOleDocument::oleLinkSrcClipFmt)) {
      // skip link format
      //
      ++from;
    }
    // and shift
    //
    if (i != from && from < clipCount)
      clipList[i] = clipList[from];
  }
  clipCount -= (from - i);
  clipOkToLink = FALSE;
}

// This is a helper function for IService::Drag and IService::Clip. Its
// purpose is to make a format list for each operation. The lists are
// independent since drag/drop shouldn't disturb the clipboard. So the
// lists are parameterized in this function.
//
BOOL _IFUNC BOleService::FormatHelper (LPFORMATETC FAR* ppList, UINT *pCount,
  BOOL &fLink, BOOL &fEmbed, PIBDataProvider pProvider)
{
  UINT nOffered = 0;

  if (!pProvider) {
    *ppList = NULL;
    *pCount = 0;
    fLink =FALSE;
    fEmbed = FALSE;
    return FALSE;
  }

  nOffered = pProvider->CountFormats();

  if (nOffered == 0) {
    *ppList = NULL;
    *pCount = 0;
    fLink =FALSE;
    fEmbed = FALSE;
    return FALSE;
  }

  // This will disable linking when GetMoniker fails
  // eg. When the container has not been saved.
  if (fLink) {
    IOleObject *pOleObj;
    if (SUCCEEDED(pProvider->QueryInterface(IID_IOleObject,
            &(LPVOID) pOleObj))) {
      IMoniker *pMon;
      if (SUCCEEDED(pOleObj->GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
                OLEWHICHMK_OBJFULL, &pMon))) {
        pMon->Release();
      }
      else {
        fLink = FALSE;
      }
      pOleObj->Release();
    }
    else
      fLink = FALSE;    // forbid linking if IOleObject fails
  }

  if (*ppList)
    delete [] *ppList;

  // The part is responsible for offering all clipboard formats except
  // link source descriptor and object descriptor.
  //
  // Note that the part won't get called in GetFormatData for embeddings
  // (because getting an embedded object means calling IBDataProv::Save) or
  // for CF_METAFILEPICT (because we get that in IBDataProv::Draw)
  //
  *ppList = new FORMATETC [nOffered + 1 + (fLink ? 1 : 0)];
            // 1 for object descriptor
            // 1 for link source descriptor if linkable
  if (!*ppList)
    return FALSE;

  LPFORMATETC pNext = *ppList;


  // All objects get an object descriptor
  //

    pNext->cfFormat = BOleDocument::oleObjectDescFmt;
    pNext->ptd = NULL;
    pNext->dwAspect = DVASPECT_CONTENT;
    pNext->lindex = -1;
    pNext->tymed = TYMED_HGLOBAL;
    pNext++;


  if (fLink) {
    pNext->cfFormat = BOleDocument::oleLinkSrcDescFmt;
    pNext->ptd = NULL;
    pNext->dwAspect = DVASPECT_CONTENT;
    pNext->lindex = -1;
    pNext->tymed = TYMED_HGLOBAL;
    pNext++;
  }

  // Enumerate the server's formats for this object into our list
  //
  BOleFormat boleFmt;
  fEmbed = FALSE;
  UINT i;
  for (i = 0; i < nOffered; i++) {
    if (NOERROR == pProvider->GetFormat (i, &boleFmt)) {
      if ((boleFmt.fmtId == BOleDocument::oleLinkSrcDescFmt) && !fLink) {
        // This will disable linking when GetMoniker fails
        // eg. When the container has not been saved.
        continue;
      }
      if ((boleFmt.fmtId == BOleDocument::oleLinkSrcClipFmt) && !fLink) {
        // This will disable linking when GetMoniker fails
        // eg. When the container has not been saved.
        continue;
      }
      if (boleFmt.fmtId == BOleDocument::oleEmbdObjClipFmt ||
        boleFmt.fmtId == BOleDocument::oleEmbSrcClipFmt) {
        // This will enable embedding if the source of the
        // drag offers an embedding format
        //
        fEmbed = TRUE;
      }
      pNext->cfFormat = boleFmt.fmtId;
      pNext->ptd = NULL;
      pNext->dwAspect = DVASPECT_CONTENT;
      pNext->lindex = -1;
      pNext->tymed = boleFmt.fmtMedium & ~BOLE_MED_STATIC;
      pNext++;
    }
  }
  *pCount = pNext - *ppList;
  return TRUE;
}

HRESULT _IFUNC BOleService::Clip (PIBDataProvider pClonePart, BOOL fLink,
  BOOL fEmbed, BOOL fUseDataCache)
{
  HRESULT hr;

  // If the clone object is null, we're invalidating everything on the
  // clipboard, for example if the app is shutting down or if the app
  // is doing pseudo-delayed-rendering, and the clipboard doesn't reflect
  // the data any more.
  //
  if (!pClonePart) {

    OleSetClipboard (NULL);
    if (clipList) {
      delete [] clipList;
      clipList = NULL;
    }
    clipCount = 0;
    clipOkToLink = FALSE;
    clipOkToEmbed = FALSE;
    return ResultFromScode (S_OK);
  }

  // Get the IPart from the part object. Need this so we can enumerate
  // the formats which should be available to the data object.
  //
  PIBDataProvider pProvider = pClonePart;

  FlushClipboardData();

  // Here we have a choice of ways to get a data object to put on the
  // clipboard. The first is a "snapshot" object which can be used for
  // delayed rendering. The second is the original object which won't
  // be protected from delayed rendering.
  //

  if (fUseDataCache) {
    // Get the data object from the snapshot data cache implemented
    // in BOleData
    //
    BOlePart *pBOlePart = NULL;
    hr = pProvider->QueryInterface (IID_BOlePart, &(LPVOID) pBOlePart);
    if (!SUCCEEDED(hr))
      return hr;

    hr = pBOlePart->CopyFromOriginal (NULL, &pClipboardData);
    pBOlePart->Release();
    if (!SUCCEEDED(hr))
      return hr;
  }
  else {
    // Get the data object from the part. Because of aggregation this
    // is actually the IDataObject of the BOleSite which the part is
    // associated with.
    //
    IDataObject *pData;
    hr = pClonePart->QueryInterface(IID_IDataObject, &(LPVOID) pData);
    if (!SUCCEEDED(hr))
      return hr;
    pClipboardData = new BOleShadowData(pFactory, pData);
    pData->Release();

    // Find out how many formats we need. This allows us to malloc a list
    // of FORMATETCs for matching later. The user doesn't have to know
    // about the OLE2 formats, so we need to add those to the count.
    //
    // This list is only used in BOleSite.
    //
    fUseDropList= FALSE;
    clipOkToLink = fLink; // will be cleared by FormatHelper if format not offered
    clipOkToEmbed = fEmbed;// will be cleared by FormatHelper if format not offered

    if (!FormatHelper (&clipList, &clipCount, clipOkToLink, clipOkToEmbed, pProvider))
      // clipOkToLink is passed by reference
      hr = ResultFromScode (DV_E_FORMATETC);
  }

  // Use the OLE2 API to put the data object on the clipboard.
  //
  if (SUCCEEDED(hr)) {

    hr = OleSetClipboard( pClipboardData);
    if (!SUCCEEDED(hr))
      Clip(NULL, FALSE, FALSE, FALSE);

  }
  if (pClipboardData)
    pClipboardData->Release();

  // pClipboardData is a non ref counted copy of the pointer
  // that is referenced by the clipboard.
  // We release our ref count now to allow BOleShadowData or BOleData
  // to go away as soon as some other app copies to the clipboard. We'll set
  // pClipboardData to NULL when BOleShadowData or BoleData goes away
  // to avoid leaving it dangling.

  return hr;
}

HRESULT _IFUNC BOleService::Drag (PIBDataProvider pPart, BOleDropAction inEffects, BOleDropAction *outEffect)
{
  HRESULT hr = ResultFromScode(S_FALSE);
  DWORD dwOutEffect;
  IDataObject *pDataObj = NULL;
  IDropSource *pDropSrc = NULL;
  if (outEffect)
    *outEffect = BOLE_DROP_NONE;
  BOOL fUnlockRunning = FALSE;

  // Get IDataObject of embedding or server being dragged

  BOlePart *pBOlePart = NULL;

  if (SUCCEEDED(pPart->QueryInterface(IID_IDataObject,&(LPVOID)pDataObj))) {

    // Cache the data formats associated with the server object so
    // when we get called in the IDataObject, we'll have something to
    // compare against in BOleSite
    //
    fUseDropList = TRUE;
    dropOkToLink = (inEffects & BOLE_DROP_LINK) != 0;
    FormatHelper (&dropList, &dropCount, dropOkToLink, dropOkToEmbed, pPart);
    if (!dropOkToLink)        // in case link not possible
      (int)inEffects &= ~BOLE_DROP_LINK;
  }
  else {
    hr = pPart->QueryInterface (IID_BOlePart, &(LPVOID) pBOlePart);
    if (!SUCCEEDED(hr))
      return hr;















      // This can be a little slow, but I think it should be better
      // than running the app
      //
      pBOlePart->CopyFromOriginal (NULL, &pDataObj);

  }

  if (!pDataObj)
    return hr;

  if (!pDropSrc)
    pPart->QueryInterface(IID_IDropSource,&(LPVOID)pDropSrc);

  if (!pDropSrc) {
    pDataObj->Release();
    return hr;
  }

  // Don't lock if the objects are in another process space
  //
  if (!pBOlePart) {
    CoLockObjectExternal (pDataObj, TRUE, FALSE);
    CoLockObjectExternal (pDropSrc, TRUE, FALSE);
  }

  hr = OLE::DoDragDrop (pDataObj, pDropSrc, (DWORD)inEffects, &dwOutEffect);

  // Fill in the out param if we got one
  //
  if (outEffect)
    *outEffect = (BOleDropAction)dwOutEffect;

  // Make up a return code. The Bolero convention here is NOERROR for
  // success or an error if not. Strangely, the container can fail from
  // IDropTarget::Drop and we still get DRAGDROP_S_DROP. Fortunately we
  // do get the DROPEFFECT_NONE if the container set it, so use it here
  // to produce an error.
  //
  if (GetScode(hr) == DRAGDROP_S_DROP)
    if (dwOutEffect == DROPEFFECT_NONE)
      hr = ResultFromScode (E_FAIL);
    else
      hr = ResultFromScode(S_OK);

  if (!pBOlePart) {
    // TRUE releases ref counts held by OLE2 if this is last unlock
    // this is important if the dragging object is a clone
    CoLockObjectExternal (pDataObj, FALSE, TRUE);
    CoLockObjectExternal (pDropSrc, FALSE, TRUE);
  }

  // When we're dragging an embedding, we need to keep the object running.
  // Unlock it after the drag/drop has completed.
  //
  if (pBOlePart) {
    if (fUnlockRunning)
      OleLockRunning (pBOlePart->pOleObject, FALSE, FALSE);
    pBOlePart->Release();
  }

  pDropSrc->Release();
  pDataObj->Release(); // should delete BOleData if dragging from embedding

  return hr;
}

BOleMenuEnable _IFUNC BOleService::EnableEditMenu (BOleMenuEnable bmeIn,
              PIBDataConsumer pConsumer)
{
  BOleMenuEnable bmeOut = BOleMenuEnable( 0 );
  HRESULT hr = NOERROR;
  LPDATAOBJECT data = NULL;

  if (bmeIn & BOLE_ENABLE_PASTELINK) {

    hr = OleGetClipboard (&data);
    if (S_OK == GetScode(hr)) {

      hr = OleQueryLinkFromData (data);
      if (data)
        data->Release ();

      // I didn't use SUCCEEDED here because someone returns 0x00030000
      // which doesn't look like an scode at all. Go figure.
      //
      if (GetScode(hr) == S_OK)
        ( int )bmeOut |= BOLE_ENABLE_PASTELINK;
    }
  }

  // We're borrowing the logic from the dialog which enumerates the links in
  // the document. In this case, we don't so much need to enumerate them;
  // it's enough to know that one exists.
  //
  if (bmeIn & BOLE_ENABLE_BROWSELINKS) {
    if (pActiveDoc && pActiveDoc->EnableBrowseLinks())
      ( int )bmeOut |= BOLE_ENABLE_BROWSELINKS;
  }

  // The test for whether to enable these last two is really the same
  // criteria: a match between what the container can accept and what the
  // server offers.
  //
  if ((bmeIn & BOLE_ENABLE_PASTE) || (bmeIn & BOLE_ENABLE_BROWSECLIPBOARD)) {





    LPDATAOBJECT pDataObj = NULL;
    OLEUIPASTEENTRY *pOle2UIEntries = NULL;
    BOleFormat *pBOleEntries = NULL;
    UINT nAcceptableFormats = 0;
    CLIPFORMAT i;

    hr = OLE::OleGetClipboard (&pDataObj);
    if (S_OK == GetScode(hr)) {
      nAcceptableFormats = pConsumer->CountFormats ();
      if (!nAcceptableFormats)
        goto cleanup;
      pOle2UIEntries = new OLEUIPASTEENTRY [nAcceptableFormats];
      if (!pOle2UIEntries)
        goto cleanup;
      pBOleEntries = new BOleFormat[nAcceptableFormats];
      if (!pBOleEntries)
        goto cleanup;

      for (i = 0; i < nAcceptableFormats; i++) {
        //_fmemset (&pOle2UIEntries[i], 0, sizeof (OLEUIPASTEENTRY));
        pConsumer->GetFormat (i, &pBOleEntries[i]);

        pOle2UIEntries[i].fmtetc.cfFormat = pBOleEntries[i].fmtId;
        pOle2UIEntries[i].fmtetc.ptd = NULL;
        pOle2UIEntries[i].fmtetc.dwAspect = DVASPECT_CONTENT;
        pOle2UIEntries[i].fmtetc.tymed = pBOleEntries[i].fmtMedium & ~BOLE_MED_STATIC;
        pOle2UIEntries[i].fmtetc.lindex = -1;

        pOle2UIEntries[i].lpstrFormatName = TEXT ("%s");  // unused for menu enabling
//          (pBOleEntries[i].fmtName[0] ? pBOleEntries[i].fmtName : OLESTR ("%s") );
        pOle2UIEntries[i].lpstrResultText = TEXT ("%s");  // unused for menu enabling
//          (pBOleEntries[i].fmtResultName[0] ? pBOleEntries[i].fmtResultName : OLESTR ("%s") );

        if (pBOleEntries[i].fmtId == BOleDocument::oleEmbdObjClipFmt ||
          pBOleEntries[i].fmtId == BOleDocument::oleLinkSrcClipFmt  ||
          pBOleEntries[i].fmtId == BOleDocument::oleEmbSrcClipFmt)

          //      PASTEONLY and ENABLEICON are mutually exclusive
          //
          pOle2UIEntries[i].dwFlags =
            OLEUIPASTE_PASTE | OLEUIPASTE_ENABLEICON;
        else
          pOle2UIEntries[i].dwFlags = OLEUIPASTE_PASTEONLY;

      }

      i = MatchPriorityClipFormat (pDataObj, pOle2UIEntries, nAcceptableFormats);
      if (i != 0xFFFF) {
        ( int )bmeOut |= ( BOLE_ENABLE_PASTE|BOLE_ENABLE_BROWSECLIPBOARD );
      }

cleanup:
      // Clean up our various scratch buffers etc
      //
      if (pBOleEntries)
        delete [] pBOleEntries;
      if (pOle2UIEntries)
        delete [] pOle2UIEntries;

      pDataObj->Release();

    }
  }
  return bmeOut;

}

//*************************************************************************
//
// IOleWindow implementation -- base class of OLE2 windows
//
//*************************************************************************

HRESULT _IFUNC BOleService::GetWindow (HWND FAR *phwnd)
{
  *phwnd = pApp->GetWindow();
  return *phwnd != NULL ? NOERROR : ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleService::ContextSensitiveHelp (BOOL fEnterMode)
{
  // The BOleService object is responsible for keeping track of Shift-F1
  //
  SetHelpMode (fEnterMode);
  return NOERROR;
}

//*************************************************************************
//
// IOleInPlaceUIWindow implementation
//
//*************************************************************************

HRESULT _IFUNC BOleService::GetBorder (LPRECT prectBorder)
{
  if (SUCCEEDED(pApp->GetWindowRect(prectBorder)))
    return NOERROR;

  return ResultFromScode (INPLACE_E_NOTOOLSPACE);
}

HRESULT _IFUNC BOleService::RequestBorderSpace (LPCRECT prectWidths)
{
  // Ask the application if it can accept the amount of space the
  // server wants on the left, top, right, and bottom of the window
  //
  if (SUCCEEDED(pApp->RequestBorderSpace (prectWidths)))
    return NOERROR;

  return ResultFromScode (INPLACE_E_NOTOOLSPACE);
}

HRESULT _IFUNC BOleService::SetBorderSpace (LPCBORDERWIDTHS prectWidths)
{
  // This seems like it does the same thing as RequestBorderSpace,
  // except that this time the server is really taking the space, so
  // we should get our stuff out of the way.
  //

  if (SUCCEEDED(pApp->SetBorderSpace(prectWidths)))
    return NOERROR;

  return ResultFromScode (OLE_E_INVALIDRECT);
}

HRESULT _IFUNC BOleService::SetActiveObject (IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
  // Although it seems a little strange, we're supposed to put the
  // name of the object in our caption bar in the client.
  //
  IBWindow *pWnd;
  if (pActiveDoc)
    pWnd = getNegotiator(pApp, pActiveDoc->pContainer);
  else
    pWnd = pApp;      // just in case

  if (pszObjName)
     pWnd->AppendWindowTitle (pszObjName);
  else if (!pszObjName && !pActiveObject)
    // Calling SetActiveObject with a null name means the object
    // is deactivating.


    //
    pWnd->RestoreUI();


  // Decrement the ref count of the pointer we're giving up, and
  // bump the ref count of the new pointer we're going to hold
  //
  if (pActiveDoc) {
    if (pActiveDoc->pActivePart)
      pActiveDoc->pActivePart->Release();
    if (pActiveDoc->pActiveCtrl) {
      pActiveDoc->pActiveCtrl->Release();
      pActiveDoc->pActiveCtrl = NULL;
    }
    pActiveDoc->pActivePart = pActiveObject;
    if (pActiveObject) {
      pActiveObject->AddRef();
      pActiveObject->QueryInterface(
          IID_IOleControl, &(LPVOID)pActiveDoc->pActiveCtrl);
    }
  }

  return NOERROR;
}

//*************************************************************************
//
// IOleInPlaceFrame implementation
//
//*************************************************************************

HRESULT _IFUNC BOleService::InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
  if (SUCCEEDED(pApp->InsertContainerMenus (hmenuShared,
    (BOleMenuWidths*) lpMenuWidths)))
    return NOERROR;

  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleService::SetMenu (HMENU hmenuShared, HOLEMENU holeMenu, HWND hwndActiveObject)
{
  HRESULT hr = NOERROR;

  // hmenuShared will be null when the call to BOleService::SetMenu is
  // initiated from BOlePart::OnUIDeactivate. hmenuShared will be valid
  // when the SetMenu call comes from the real server object
  //
  // Even when hmenuShared is NULL, we have to pass the call on to the
  // container because the container has to remove the menu from its
  // frame window.
  //
  hr = pApp->SetFrameMenu (hmenuShared);

  // holemenu will be null when the call to BOleService::SetMenu is
  // initiated from BOlePart::OnUIDeactivate. holeMenu will be valid
  // when the SetMenu call comes from the real server object
  //
  if (SUCCEEDED(hr))
    hr = OleSetMenuDescriptor (holeMenu, pApp->GetWindow(),
      hwndActiveObject, NULL, NULL);

  return hr;
}

HRESULT _IFUNC BOleService::RemoveMenus (HMENU hmenuShared)
{




  BOOL fNoError = TRUE;

  // Remove container group menus
  //
  while (GetMenuItemCount(hmenuShared))
    fNoError &= RemoveMenu(hmenuShared, 0, MF_BYPOSITION);

  return fNoError ? NOERROR : ResultFromScode(E_FAIL);
}

HRESULT _IFUNC BOleService::SetStatusText (LPCOLESTR statusText)
{
  // Servers use this function to put text in the container's status bar.
  // The server is not supposed to negotiate tool space to put their own
  // status bar at the bottom of the frame window.
  //
  pApp->SetStatusText(statusText);
  return NOERROR;
}

HRESULT _IFUNC BOleService::EnableModeless (BOOL fEnable)
{
  return pApp->OnModalDialog (!fEnable);
}

HRESULT _IFUNC BOleService::TranslateAccelerator (MSG FAR* msg, WORD wID)
{
  HWND oldhwnd = msg->hwnd;
  msg->hwnd = pApp->GetWindow ();
  HRESULT hr = pApp->Accelerator(msg);
  msg->hwnd = oldhwnd;
  return hr;
}

/*
 *        Registration related.
 */

HRESULT _IFUNC BOleService::RegisterClass(LPCOLESTR szProgId,
  IBClassMgr * pCM, BCID rId,
  BOOL fInProcExe, BOOL fSingleUse)
{
  HRESULT hErr;
  LPUNKNOWN pObjF;
  IBClass *pClass;
  pFactory->ComponentCreate(&pObjF, NULL, cidBOleFactory);
  if (SUCCEEDED(hErr = pObjF->QueryInterface(IID_IBClass, &(LPVOID)pClass))) {
    if (SUCCEEDED(hErr = pClass->Init(fInProcExe, szProgId, pCM, rId))) {
      if (SUCCEEDED(hErr = pClass->Register(fSingleUse))) {
        AddClassFactory( pClass );
      }
    }
    pClass->Release();
  }
  pObjF->Release();
  return hErr;
}

HRESULT _IFUNC BOleService::UnregisterClass (LPCOLESTR szProgId)
{
  return RemoveClassFactory(szProgId);
}

HRESULT _IFUNC BOleService::RegisterControlClass (LPCOLESTR szProgId,
  IBClassMgr * pCM, BCID rId,
  BOOL fInProcExe, BOOL fSingleUse)
{
  // rayk - this is a place holder for registering a control class
  //        this will create the appropriate class factory that will
  //        properly handle the licensing information for controls that we
  //        create

  HRESULT hErr;
  LPUNKNOWN pObjF;
  IBClass *pClass;
  pFactory->ComponentCreate(&pObjF, NULL, cidBOleControlFactory);
  if (SUCCEEDED(hErr = pObjF->QueryInterface(IID_IBClass, &(LPVOID)pClass))) {
    if (SUCCEEDED(hErr = pClass->Init(fInProcExe, szProgId, pCM, rId))) {
      if (SUCCEEDED(hErr = pClass->Register(fSingleUse))) {
        AddClassFactory( pClass );
      }
    }
    pClass->Release();
  }
  pObjF->Release();
  return hErr;
}

HRESULT _IFUNC BOleService::UnregisterControlClass (LPCOLESTR szProgId)
{
  return RemoveClassFactory (szProgId);
}

HRESULT BOleService::AddClassFactory (IBClass *pF)
{
  HRESULT hErr = NOERROR;
  BOleFactNode *pNewNode = new BOleFactNode( pF );
  if( !pFirstFactNode )
    pFirstFactNode = pNewNode;
  else {
    BOleFactNode *pNode = pFirstFactNode;
    for(; pNode->pNext; pNode = pNode->pNext )
      ;
    pNode->pNext = pNewNode;
  }
  OLEHRES("Registered classid", hErr);
  return hErr;
}

HRESULT BOleService::RemoveClassFactory (LPCOLESTR szProgId)
{
  CLSID cid;
  if (SUCCEEDED( CLSIDFromProgID (szProgId, &cid) ) ){
    return RemoveClassFactory (cid);
  }
  return ResultFromScode (E_FAIL);
}

HRESULT BOleService::RemoveClassFactory (REFCLSID cid)
{
  BOleFactNode *pNode = pFirstFactNode, *pPrevNode = NULL;
  while( pNode ){
    IBClass *pF = pNode->factory;
    if (pF->IsA(cid)) {
      pF->Revoke();
      // Unchain the node
      if( pPrevNode )
        pPrevNode->pNext = pNode->pNext;
      else
        pFirstFactNode = pNode->pNext;
      BOleFactNode *pDel = pNode;
      pNode = pNode->pNext;
      delete pDel;        // releases the factory
    } else {
      pPrevNode = pNode;
      pNode = pNode->pNext;
    }
  }
  return NOERROR;
}


HRESULT BOleService::FindClassFactory(LPCOLESTR szProgId, IBClass **pF)
{
  CLSID cid;
  if( SUCCEEDED( CLSIDFromProgID(szProgId, &cid ) ) ){
    return FindClassFactory( cid, pF );
  }
  return ResultFromScode (E_FAIL);
}


HRESULT BOleService::FindClassFactory(REFCLSID cid, IBClass **ppF)
{
  *ppF = NULL;
  BOleFactNode *pNode;
  for (pNode = pFirstFactNode; pNode; pNode = pNode->pNext ){
    if( pNode->factory->IsA(cid) ) {
      *ppF = pNode->factory;
      (*ppF)->AddRef();
      break;
    }
  }
  return (*ppF) ? NOERROR : ResultFromScode(E_FAIL);
}


BOOL IsControlClass (REFCLSID rclsid, HKEY hKey)
{
  DWORD    dw;
  LONG     lRet;
  LPOLESTR lpszCLSID;
  char     lpszAnsiCLSID[256];
  char     szKey[64];
  char     szMiscStatus[_MAX_PATH];
  BOOL     bCloseRegDB = FALSE;

  if (hKey == NULL)
  {

   //Open up the root key.
   lRet=RegOpenKey(HKEY_CLASSES_ROOT, (LPCTSTR)"CLSID", &hKey);

   if ((LONG)ERROR_SUCCESS!=lRet)
     return FALSE;

   bCloseRegDB = TRUE;
  }

  // Get a string containing the class name
  StringFromCLSID(rclsid, &lpszCLSID);
#if !defined(UNICODE)    // MS_OLEUI_DEF
  WideCharToMultiByte (CP_ACP, 0, lpszCLSID, -1, lpszAnsiCLSID, 256,
      NULL, NULL);
  strcpyA ((LPTSTR)szKey, (LPCTSTR)lpszAnsiCLSID);
  strcatA ((LPTSTR)szKey, (LPCTSTR)"\\Control");
#else
  lstrcpy((LPTSTR)szKey, (LPCTSTR)lpszCLSID);
  lstrcat((LPTSTR)szKey, (LPCTSTR)"\\Control");
#endif

  dw=_MAX_PATH;
  lRet = RegQueryValue(hKey, (LPCTSTR)szKey, (LPTSTR)szMiscStatus, (PLONG)&dw);

  OleStdFreeString(lpszCLSID, NULL);
  if (bCloseRegDB)
    RegCloseKey(hKey);

  if ((LONG)ERROR_SUCCESS!=lRet)
     return FALSE;

  return TRUE;

}

//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolesite.h"
#include "ole2.h"
#include "bolesvc.h"
#include "boledoc.h"

extern "C" {
  #include "OLE2UI.h"
}


//
// Take a wide char string & return an ANSI string in a new'd char[] buffer
//
char* ConvertWtoA(const wchar_t* src, size_t len)
{
  size_t size = WideCharToMultiByte(CP_ACP, 0, src, len, 0, 0, 0, 0);
  char* dst = new char[size + (len != -1)];  // room for null if fixed size
  size = WideCharToMultiByte(CP_ACP, 0, src, len, dst, size, 0, 0);
  if (len != -1)
  dst[size] = 0;
  return dst;
}

// GetObjDescData -- Fabricate the "Object Descriptor" clipboard format
//                   structure for the server-side helper.
//
HGLOBAL _IFUNC BOleSite::GetObjDescData ()
{
  HGLOBAL  hMem = NULL;

  CLSID cid;
  HRESULT hr = CLSIDFromProgID (pszProgID, &cid);
  if (!SUCCEEDED(hr))
    return NULL;

  DWORD miscStatus = 0;
  hr = ::OleRegGetMiscStatus (cid, DVASPECT_CONTENT, &miscStatus);
  if (!SUCCEEDED(hr))
    return NULL;
  miscStatus |= OLEMISC_CANLINKBYOLE1;

  SIZEL sizel;
  SIZE size;
  pProvider->GetPartSize (&size);
  sizel.cx = MAP_PIX_TO_LOGHIM (size.cx, BOleService::pixPerIn.x);
  sizel.cy = MAP_PIX_TO_LOGHIM (size.cy, BOleService::pixPerIn.y);

  POINTL pointl;
  pointl.x = pointl.y = 0;

  //      If we have a moniker we can use that for the instance name

  LPOLESTR pszDisplay = NULL;  // must be freed later
  {
    LPMONIKER pMoniker;
    hr = GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
      OLEWHICHMK_OBJFULL, &pMoniker);
    if (SUCCEEDED(hr)) {
      LPBC pbc;
      CreateBindCtx(0, &pbc);
      pMoniker->GetDisplayName(pbc, NULL, &pszDisplay);
      pbc->Release();
      pMoniker->Release();
    }
  }

  LPOLESTR pszType = NULL;     // must be freed later

  hr = ::OleRegGetUserType (cid, USERCLASSTYPE_FULL, &pszType);

  if (SUCCEEDED(hr)) {

    DWORD dwObjectDescSize, dwFullUserTypeNameLen, dwSrcOfCopyLen;
    LPOLESTR pszInstance = pszDisplay;

    // Get the length of Full User Type Name; Add 1 for the null terminator
    dwFullUserTypeNameLen = (pszType ? lstrlen(pszType)+1 : 0) * sizeof(OLECHAR);

    // Get the Source of Copy string and it's length; Add 1 for the null terminator
    if (pszInstance)
      dwSrcOfCopyLen = (lstrlen(pszInstance)+1) * sizeof(OLECHAR);
    else {
      // No src moniker so use user type name as source string.
      pszInstance = pszType;
      dwSrcOfCopyLen = dwFullUserTypeNameLen;
    }

    // Allocate space for OBJECTDESCRIPTOR and the additional string data
    //
    dwObjectDescSize = sizeof(OBJECTDESCRIPTOR);
    hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
          dwObjectDescSize + dwFullUserTypeNameLen + dwSrcOfCopyLen);
    if (hMem) {
      // Fill in the object descriptor
      //
      LPOBJECTDESCRIPTOR lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);

      // Set the FullUserTypeName offset and copy the string
      if (pszType)    {
        lpOD->dwFullUserTypeName = dwObjectDescSize;
        LPSTR pDest = (LPSTR)lpOD + dwObjectDescSize; // dwObjectDescSize is BYTE offset
        lstrcpy((LPOLESTR)pDest, pszType);
      }
      else lpOD->dwFullUserTypeName = 0;  // zero offset indicates that string is not present

      // Set the SrcOfCopy offset and copy the string
      if (pszInstance) {
        LPSTR pDest = (LPSTR)lpOD + dwObjectDescSize + dwFullUserTypeNameLen; // dwObjectDescSize is BYTE offset
        lpOD->dwSrcOfCopy = dwObjectDescSize + dwFullUserTypeNameLen;
        lstrcpy((LPOLESTR)pDest, pszInstance);
      }
      else lpOD->dwSrcOfCopy = 0;  // zero offset indicates that string is not present

      // Initialize the rest of the OBJECTDESCRIPTOR
      lpOD->cbSize       = dwObjectDescSize + dwFullUserTypeNameLen + dwSrcOfCopyLen;
      lpOD->clsid  = cid;
      lpOD->dwDrawAspect = DVASPECT_CONTENT;
      lpOD->sizel  = sizel;
      lpOD->pointl       = pointl;
      lpOD->dwStatus     = miscStatus;

      GlobalUnlock(hMem);
    }
  }
  // Free pszType and pszDisplay
  //
  LPMALLOC pMalloc;
  hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
  if (SUCCEEDED(hr)) {
    if (pszType && pMalloc->DidAlloc(pszType)) {
      pMalloc->Free (pszType);
      pszType = NULLP;
    }
    if (pszDisplay && pMalloc->DidAlloc(pszDisplay)) {
      pMalloc->Free (pszDisplay);
      pszDisplay = NULLP;
    }
    pMalloc->Release ();
  }

  return hMem;

}

// GetPasteIcon -- Retrieve the iconic representation of the data object
//     from the clipboard if necessary. OLE2UI does this for us
//     for Paste Special, but we need to do it ourselves for
//     Paste.
//
HMETAFILE _IFUNC BOleService::GetPasteIcon (LPDATAOBJECT pData)
{
  HMETAFILE hIconMetafile = NULL;
  HGLOBAL hObjDesc = NULL;

  STGMEDIUM medObjDesc;
  STGMEDIUM medIconMetafile;

  hObjDesc = GetDataFromDataObject (pData, BOleDocument::oleObjectDescFmt,
    NULL, DVASPECT_CONTENT, &medObjDesc);

  if (hObjDesc) {

    LPOBJECTDESCRIPTOR pObjDesc = (LPOBJECTDESCRIPTOR) WIN::GlobalLock (hObjDesc);

    if (pObjDesc && pObjDesc->dwDrawAspect == DVASPECT_ICON)
      hIconMetafile = (HMETAFILE) GetDataFromDataObject (pData,
        CF_METAFILEPICT, NULL, DVASPECT_ICON, &medIconMetafile);

    WIN::GlobalUnlock (hObjDesc);
    OLE::ReleaseStgMedium (&medObjDesc);
  }

  return hIconMetafile; // should be freed in IBPart::Init
}

// PasteHelper -- Does both paste and paste special.
//
//         The Paste Special  dialog is borrowed from OLE2UI, but
//         it isn't mandatory to use it.  You can always fill in
//         the BOleInitInfo yourself and send it to IPart::Init
//
//         Similarly, you can also get the available clipboard
//         formats using the normal Windows API, and only if you
//         decide to create an OLE2 object (link or embed), call
//         IPart::Init
//
BOOL _IFUNC BOleService::PasteHelper (BOleInitInfo FAR *pInfo, BOOL fPasteSpecial)
{
  UINT ret = OLEUI_FALSE;

  // Initialize hIcon to NULL so as to not free the icon in BOlePart::Init
  pInfo->hIcon = NULL;

  // Find the container we should paste into
  //
  PIBContainer pCont = NULL;
  if (pInfo->pContainer) {
    pCont = pInfo->pContainer;
  }
  else {
    if (!pActiveDoc || !pActiveDoc->GetContainer())
      return FALSE;
    pCont = pActiveDoc->GetContainer();
  }

  // From the container, get the data consumer
  //
  PIBDataConsumer pConsumer = NULL;
  HRESULT hr = pCont->QueryInterface (IID_IBDataConsumer, &(LPVOID) pConsumer);
  if (!SUCCEEDED(hr))
    return FALSE;
  else
    pConsumer->Release();

  OLEUIPASTESPECIAL ops;
  memset (&ops, 0, sizeof (ops));

  LPDATAOBJECT pDataObj = NULL;
  OLEUIPASTEENTRY *pOle2UIEntries = NULL;
  BOleFormat *pBOleEntries = NULL;
  UINT *pBOleLinkEntries = NULL;
  UINT nAcceptableFormats = 0, nLinkableFormats = 0;
  UINT i;
  CLIPFORMAT cf;
  char text[4];

  // If this fails, the clipboard may be empty or otherwise unavailable
  //
  text[0] = '%';
  text[1] = 's';
  text[2] = '\0';
  hr = OLE::OleGetClipboard (&pDataObj);
  if (S_OK != GetScode (hr))
    return FALSE;

  // The idea here is to insulate Bolero users from Ole2UI. BOleFormat
  // is only marginally simpler than OLEUIPASTEENTRY, so I'm not sure
  // there's a big complexity win.
  //
  nAcceptableFormats = pConsumer->CountFormats ();
  if (!nAcceptableFormats)
    goto cleanup;
  pOle2UIEntries = new OLEUIPASTEENTRY [nAcceptableFormats];
  if (!pOle2UIEntries)
    goto cleanup;
  pBOleEntries = new BOleFormat[nAcceptableFormats];
  if (!pBOleEntries)
    goto cleanup;
  pBOleLinkEntries = new UINT[nAcceptableFormats];
  if (!pBOleLinkEntries)
    goto cleanup;

  for (i = 0; i < nAcceptableFormats; i++) {
    //memset (&pOle2UIEntries[i], 0, sizeof (OLEUIPASTEENTRY));
    pConsumer->GetFormat (i, &pBOleEntries[i]);

    pOle2UIEntries[i].fmtetc.cfFormat = pBOleEntries[i].fmtId;
    pOle2UIEntries[i].fmtetc.ptd = NULL;
    pOle2UIEntries[i].fmtetc.dwAspect = DVASPECT_CONTENT;
    pOle2UIEntries[i].fmtetc.tymed = pBOleEntries[i].fmtMedium & ~BOLE_MED_STATIC;
    pOle2UIEntries[i].fmtetc.lindex = -1;

    // The paste special dialog accepts names in Ascii only
    //
  // OleUI always prefers ANSI, even in UNICODE compile
  pOle2UIEntries[i].lpstrFormatName =
    (TCHAR *)(pBOleEntries[i].fmtName[0] ?
    ConvertWtoA(pBOleEntries[i].fmtName, -1) : text );
  pOle2UIEntries[i].lpstrResultText =
    (TCHAR *)(pBOleEntries[i].fmtResultName[0] ?
    ConvertWtoA(pBOleEntries[i].fmtResultName, -1) : text );

    pOle2UIEntries[i].dwFlags =
      pBOleEntries[i].fmtIsLinkable ? OLEUIPASTE_PASTE : OLEUIPASTE_PASTEONLY;

    if (pBOleEntries[i].fmtId == BOleDocument::oleEmbdObjClipFmt ||
      pBOleEntries[i].fmtId == BOleDocument::oleLinkSrcClipFmt  ||
      pBOleEntries[i].fmtId == BOleDocument::oleEmbSrcClipFmt)

      pOle2UIEntries[i].dwFlags |= (OLEUIPASTE_PASTE | OLEUIPASTE_ENABLEICON);

    if (pBOleEntries[i].fmtIsLinkable) {
      pBOleLinkEntries[nLinkableFormats] = pBOleEntries[i].fmtId;
      DWORD f = 0;
      switch (nLinkableFormats) {
        case 0 : f = OLEUIPASTE_LINKTYPE1; break;
        case 1 : f = OLEUIPASTE_LINKTYPE2; break;
        case 2 : f = OLEUIPASTE_LINKTYPE3; break;
        case 3 : f = OLEUIPASTE_LINKTYPE4; break;
        case 4 : f = OLEUIPASTE_LINKTYPE5; break;
        case 5 : f = OLEUIPASTE_LINKTYPE6; break;
        case 6 : f = OLEUIPASTE_LINKTYPE7; break;
        case 7 : f = OLEUIPASTE_LINKTYPE8; break;
      }
      pOle2UIEntries[i].dwFlags |= f;
      nLinkableFormats++;
    }
  }


  if (fPasteSpecial) {

    // Fill in the parameter block to the OLE2UI Paste Special dialog box
    //
    ops.cbStruct = sizeof (ops);
    ops.dwFlags = PSF_SELECTPASTE;
    if (ShowHelpButton (BOLE_HELP_BROWSECLIPBOARD))
      ops.dwFlags |= PSF_SHOWHELP;
    ops.hWndOwner = ::GetActiveWindow(); // pApp->GetWindow ();
    ops.lpszCaption = NULL; //TEXT("Paste Special");
    ops.lpfnHook = NULL;
    ops.lCustData = NULL;
    ops.hInstance = NULL;
    ops.lpszTemplate = NULL;
    ops.hResource = NULL;

    ops.arrPasteEntries = pOle2UIEntries;
    ops.cPasteEntries = nAcceptableFormats;
    ops.lpSrcDataObj = pDataObj;
    ops.arrLinkTypes = pBOleLinkEntries;
  ops.cLinkTypes = nLinkableFormats;
  ops.cClsidExclude = 0;

  EnterBOleDialog (ops.hWndOwner, NULL, NULL);

  ret = OleUIPasteSpecial (&ops);
    ExitBOleDialog ();

    if (ret == OLEUI_OK) {
      if (ops.dwFlags & PSF_CHECKDISPLAYASICON)
        pInfo->hIcon = (HICON) ops.hMetaPict;
      i = ops.nSelectedIndex;
    }
    else
      goto cleanup;
  }
  else {
    i = MatchPriorityClipFormat (pDataObj, pOle2UIEntries, nAcceptableFormats);
    if (i == 0xFFFF)
      goto cleanup;

    (HMETAFILE) pInfo->hIcon = GetPasteIcon (pDataObj);
    ret = OLEUI_OK;
  }

  cf = pBOleEntries[i].fmtId;

  if (cf == BOleDocument::oleEmbdObjClipFmt) {
    if (ops.fLink) {
      pInfo->How = BOLE_LINK;
      pInfo->Where = BOLE_DATAOBJECT;
      pInfo->whereData.pData = pDataObj;
    }
    else {
      pInfo->How = BOLE_EMBED;
      pInfo->Where = BOLE_DATAOBJECT;
      pInfo->whereData.pData = pDataObj;
    }
  }
  else if (cf == BOleDocument::oleEmbSrcClipFmt) {
    if (ops.fLink) {
      pInfo->How = BOLE_LINK;
      pInfo->Where = BOLE_DATAOBJECT;
      pInfo->whereData.pData = pDataObj;
    }
    else {
      pInfo->How = BOLE_EMBED;
      pInfo->Where = BOLE_DATAOBJECT;
      pInfo->whereData.pData = pDataObj;
      pInfo->pStorage = NULL;
    }
  }
  else if (cf == BOleDocument::oleLinkSrcClipFmt) {
    pInfo->How = BOLE_LINK;
    pInfo->Where = BOLE_DATAOBJECT;
    pInfo->whereData.pData = pDataObj;
  }
  else if (pBOleEntries[i].fmtMedium & BOLE_MED_STATIC) {
    pInfo->How = (cf == CF_METAFILEPICT) ?
      BOLE_EMBED_ASMETAFILE : BOLE_EMBED_ASBITMAP;
    pInfo->Where = BOLE_DATAOBJECT;
    pInfo->whereData.pData = pDataObj;
  }
  else {
    STGMEDIUM medium;
    pInfo->How = ops.fLink ? BOLE_LINK : BOLE_EMBED;
    pInfo->Where = BOLE_HANDLE;
    pInfo->whereHandle.dataFormat = cf;
    if (!ops.fLink)
      pInfo->whereHandle.data = GetDataFromDataObject (pDataObj, cf, NULL,
                        DVASPECT_CONTENT, &medium);

    // Normally, the user is expected to call ReleaseDataObject on the
    // BOleInitInfo, but in this case we aren't passing back the data
    // object, only a handle to real data, so we have to release the
    // data object here
    //
    pDataObj->Release();

    // We're not going to draw as an icon if we're not creating
    // an OLE object (i.e. can't draw CF_TEXT as an icon)
    //
    OleUIMetafilePictIconFree (pInfo->hIcon);    // clean up metafile
    pInfo->hIcon = NULL;

  }

cleanup:

  // Clean up our various scratch buffers etc
  //
  if (pBOleEntries)
    delete [] pBOleEntries;
  if (pOle2UIEntries) {
  for (i = 0; i < nAcceptableFormats; i++) {
    if (pOle2UIEntries[i].lpstrFormatName != text)
    delete [] pOle2UIEntries[i].lpstrFormatName;
    if (pOle2UIEntries[i].lpstrResultText != text)
    delete [] pOle2UIEntries[i].lpstrResultText;
  }
    delete [] pOle2UIEntries;
  }
  if (pBOleLinkEntries)
    delete [] pBOleLinkEntries;

  // Since the BOleInitInfo is considered invalid if we fail out of
  // IBService::Paste (e.g. user cancels the dialog), they can't call
  // ReleaseDataObject so we should do it instead.
  //
  if (ret != OLEUI_OK && pDataObj)
    pDataObj->Release();


  return ((ret == OLEUI_OK) ? TRUE : FALSE);
}

// Get data in a specified format from a data object. This doesn't really
// have to be on the BOleService, but I couldn't think of anywhere more
// appropriate.
//
HGLOBAL _IFUNC BOleService::GetDataFromDataObject (
  LPDATAOBJECT  lpDataObj,
  CLIPFORMAT    cfFormat,
  DVTARGETDEVICE FAR* lpTargetDevice,
  DWORD         dwAspect,
  LPSTGMEDIUM   lpMedium
)
{
    HRESULT hrErr;
    FORMATETC formatetc;
    HGLOBAL hGlobal = NULL;
    HGLOBAL hCopy;
    LPVOID  lp;

    formatetc.cfFormat = cfFormat;
    formatetc.ptd = lpTargetDevice;
    formatetc.dwAspect = dwAspect;
    formatetc.lindex = -1;

    switch (cfFormat) {
  case CF_METAFILEPICT:
      formatetc.tymed = TYMED_MFPICT;
      break;

  case CF_BITMAP:
      formatetc.tymed = TYMED_GDI;
      break;

  default:
      formatetc.tymed = TYMED_HGLOBAL;
      break;
    }

    hrErr = lpDataObj->GetData(
      (LPFORMATETC)&formatetc,
      lpMedium
    );

    if (hrErr != NOERROR)
  return NULL;

    if ((hGlobal = lpMedium->hGlobal) == NULL)
  return NULL;

    // Check if hGlobal really points to valid memory
    if ((lp = GlobalLock(hGlobal)) != NULL) {
  if (IsBadReadPtr(lp, 1)) {
      GlobalUnlock(hGlobal);
      return NULL;    // ERROR: memory is NOT valid
  }
  GlobalUnlock(hGlobal);
    }

    if (hGlobal != NULL && lpMedium->pUnkForRelease != NULL) {
  /* OLE2NOTE: the callee wants to retain ownership of the data.
  **    this is indicated by passing a non-NULL pUnkForRelease.
  **    thus, we will make a copy of the data and release the
  **    callee's copy.
  */

  hCopy = OleDuplicateData(hGlobal, cfFormat, GHND|GMEM_SHARE);
  ReleaseStgMedium(lpMedium); // release callee's copy of data

  hGlobal = hCopy;
  lpMedium->hGlobal = hCopy;
  lpMedium->pUnkForRelease = NULL;
    }
    return hGlobal;
}

//  Mark each entry in the PasteEntryList if its format is available from
//  the source IDataObject*. the dwScratchSpace field of each PasteEntry
//  is set to TRUE if available, else FALSE.
//
void _IFUNC BOleService::MarkPasteEntries(LPDATAOBJECT dataObj,
    LPOLEUIPASTEENTRY priorityList, int cEntries)
{
  LPENUMFORMATETC     pEnumFmtEtc = NULL;
  #define FORMATETC_MAX 64
  FORMATETC     *rgfmtetc = new FORMATETC[FORMATETC_MAX];
  int     i;
  HRESULT       hrErr;
  DWORD         cFetched;

  // Clear all marks
  //
  for (i = 0; i < cEntries; i++) {
    priorityList[i].dwScratchSpace = FALSE;

    if (!priorityList[i].fmtetc.cfFormat) {

      // Caller wants this item always considered available
      // (by specifying a NULL format)
      //
      priorityList[i].dwScratchSpace = TRUE;
    }
    else if (priorityList[i].fmtetc.cfFormat == BOleDocument::oleEmbdObjClipFmt
        || priorityList[i].fmtetc.cfFormat == BOleDocument::oleEmbSrcClipFmt) {

      // If there is an OLE object format, then handle it
      // specially by calling OleQueryCreateFromData. the caller
      // need only specify one object type format.
      //
#if 0
      // This returns NOERROR when QPW drags a graph object which doesn't
      // offer any embedding formats. It also DOESN"T CALL BOLESITE!!
      // Maybe an OLE2 bug? If we use this function, we'll try to drop
      // anyway because we think embedding is available. It's not, so
      // the user gets an error.
      //
      // Workaround: use IDataObject::QueryGetData directly which
      // seems to call BOleSite as expected
      //
      hrErr = OleQueryCreateFromData(dataObj);
#else
      FORMATETC fe;
      fe.cfFormat = BOleDocument::oleEmbdObjClipFmt;
      fe.lindex = -1;
      fe.ptd = NULL;
      fe.tymed = TYMED_ISTORAGE;
      fe.dwAspect = DVASPECT_CONTENT;
      hrErr = dataObj->QueryGetData (&fe);
      if (hrErr != NOERROR) {
        fe.cfFormat = BOleDocument::oleEmbSrcClipFmt;
        fe.lindex = -1;
        fe.ptd = NULL;
        fe.tymed = TYMED_ISTORAGE;
        fe.dwAspect = DVASPECT_CONTENT;
        hrErr = dataObj->QueryGetData (&fe);
      }
#endif
    if (NOERROR == hrErr)
        priorityList[i].dwScratchSpace = TRUE;
    }
    else if (priorityList[i].fmtetc.cfFormat == BOleDocument::oleLinkSrcClipFmt) {

      // If there is OLE 2.0 LinkSource format, then handle it
      // specially by calling OleQueryLinkFromData.
      //
      hrErr = OleQueryLinkFromData(dataObj);
      if(NOERROR == hrErr)
        priorityList[i].dwScratchSpace = TRUE;
    }
  }

  hrErr = dataObj->EnumFormatEtc(DATADIR_GET, &pEnumFmtEtc);
  if (hrErr != NOERROR)
    return;    // unable to get format enumerator

  // Enumerate the formats offered by the source
  //
  cFetched = 0;
  memset (rgfmtetc,0,sizeof(FORMATETC)*FORMATETC_MAX);
  if (pEnumFmtEtc->Next (FORMATETC_MAX, rgfmtetc, &cFetched) == NOERROR ||
    (cFetched > 0 && cFetched <= FORMATETC_MAX))
  {

    for (DWORD j = 0; j < cFetched; j++)
      for (i = 0; i < cEntries; i++)
        if (!priorityList[i].dwScratchSpace &&
        (OleStdCompareFormatEtc (&priorityList[i].fmtetc, &rgfmtetc[j]) != 1))
       // IsEqualFORMATETC(rgfmtetc[j], priorityList[i].fmtetc)) !DR
       priorityList[i].dwScratchSpace = TRUE;
  }

  // Clean up
  //
  if (pEnumFmtEtc)
    pEnumFmtEtc->Release();
  delete [] rgfmtetc;
}

// Retrieve the first clipboard format in a list for which data
// exists in the source IDataObject*.
//
// Returns -1 if no acceptable match is found.
//   index of first acceptable match in the priority list.
//
//
CLIPFORMAT _IFUNC BOleService::MatchPriorityClipFormat(
    LPDATAOBJECT dataObj, LPOLEUIPASTEENTRY priorityList, int cEntries)
{
  int i;
  CLIPFORMAT nFmtEtc = 0xFFFF;

  // Mark all entries that the Source provides
  //
  MarkPasteEntries (dataObj, priorityList, cEntries);

  // Loop over the target's priority list of formats
  //
  for (i = 0; i < cEntries; i++)
  {
    if ((priorityList[i].dwFlags != OLEUIPASTE_PASTEONLY) &&
       !(priorityList[i].dwFlags & OLEUIPASTE_PASTE))
      continue;

    // get first marked entry
    //
    if (priorityList[i].dwScratchSpace) {
      nFmtEtc = i;
      break;    // Found priority format; DONE
    }
  }
  return nFmtEtc;
}


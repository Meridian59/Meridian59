//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Implements a data cache object which is used for delayed rendering.
//
//  Bolero customers aren't required to use this object (or delayed rendering
//  at all) but when we cut/copy an embedded object (i.e. embed from
//  embedding) we need the data cache to avoid getting changes to the
//  original object when the user expects a snapshot (the usual delayed
//  rendering problem).
//----------------------------------------------------------------------------
#include "boledata.h"
#include "bolecman.h"
#include "boledoc.h"

#ifdef  _DEBUG

//**************************************************************************
//
// StgWatcher
//
// StgWatcher is a debugging tool used to watch when temporary storages are
// created and deleted, if they ever are.
//
//**************************************************************************

class StgWatcher : public IUnknown, public STATSTG {
 public:
  StgWatcher( IUnknown *Data, IStorage *Stg );
  ~StgWatcher();
  // IUnknown
  virtual HRESULT _IFUNC QueryInterface(REFIID iid, void FAR* FAR* pif);
  virtual ULONG _IFUNC AddRef();
  virtual ULONG _IFUNC Release();
 protected:
  IStorage *Stg;
  IUnknown *Data;
  ULONG  nRef;
};

StgWatcher::StgWatcher( IUnknown *data, IStorage *stg ) : nRef( 1 ), Data( data ), Stg( stg )
{
  stg->Stat (this, 0 );
  TCHAR tmp[80];
  wsprintf (tmp, TEXT("Stg \"%s\" created\n\r" ), pwcsName );
  OutputDebugString (tmp);
}

StgWatcher::~StgWatcher (){
  LPMALLOC pMalloc;
  CoGetMalloc (MEMCTX_TASK, &pMalloc );
  pMalloc->Free (pwcsName );
  pMalloc->Release ();
}

HRESULT _IFUNC StgWatcher::QueryInterface(REFIID iid, void FAR* FAR* pif){
  *pif = ( iid == IID_IUnknown ) ? ( void FAR * )( IUnknown * )this : 0;
  return ( iid == IID_IUnknown ) ? ResultFromScode(S_OK) : ResultFromScode(E_NOINTERFACE);
}
ULONG _IFUNC StgWatcher::AddRef(){
  nRef++;
  TCHAR tmp[80];
  wsprintf (tmp, TEXT("Stg \"%s\" AddRef to %d\n\r" ), pwcsName, nRef );
  OutputDebugString (tmp);
  return nRef;
}
ULONG _IFUNC StgWatcher::Release(){
  nRef--;
  TCHAR tmp[80];
  wsprintf( tmp, TEXT("Stg \"%s\" Release to %d\n\r" ), pwcsName, nRef );
  OutputDebugString( tmp );
  if( nRef == 0 ){
    wsprintf( tmp, TEXT("Stg \"%s\" deleted at %d\n\r" ), pwcsName, nRef );
    OutputDebugString( tmp );
    delete this;
    return 0;
  }
  return nRef;
}
#endif

//**************************************************************************
//
// BOleData Implementation
//
//**************************************************************************

BOleData::BOleData (BOleClassManager *pCM, PIBUnknownMain pOuter) :
  BOleComponent (pCM, pOuter), pFirstItem (NULL), pLastItem (NULL)
{
  #ifdef _DEBUG
  {
    TCHAR b[ 80 ];
    wsprintf( b, TEXT("BOleData %04x:%04x created\n\r"),
      HIWORD( ( DWORD )this ), LOWORD( ( DWORD )this ) );
    OutputDebugString( b );
  }
  #endif
}

BOleData::~BOleData ()
{
  #ifdef _DEBUG
  {
    TCHAR b[ 80 ];
    wsprintf( b, TEXT("BOleData %04x:%04x deleted\n\r"),
      HIWORD( ( DWORD )this ), LOWORD( ( DWORD )this ) );
    OutputDebugString( b );
  }
  #endif
  FreeItems();

  if (NOERROR == OleIsCurrentClipboard (this))
    pFactory->GetService()->NotifyClipboardEmptied();
}

void BOleData::FreeItems()
{
  // Release the mediums we might have created for deferred rendering
  //
  LPMALLOC pMalloc;
  CoGetMalloc( MEMCTX_TASK, &pMalloc );

  BOleDataItem FAR *pWalk = pFirstItem;
  while (pWalk) {
    BOleDataItem *pTmp = pWalk->pNext;
    ReleaseStgMedium (&(pWalk->tymed));
    pMalloc->Free (pWalk);
    pWalk = pTmp;
  }
  pFirstItem = NULL;
  pMalloc->Release();
}

HRESULT _IFUNC BOleData::QueryInterfaceMain (REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces

  SUCCEEDED(hr = IDataObject_QueryInterface(this, iid, ppv))

  // base classes

  || SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))

  // helpers

  ;

  return hr;
};

//**************************************************************************
//
// IDataObject Implementation
//
//**************************************************************************

HRESULT _IFUNC BOleData::GetData (LPFORMATETC pformatetcIn,
                       LPSTGMEDIUM pmedium)
{
  pmedium->tymed = TYMED_NULL;
  pmedium->pstg = NULL;

  if (!pFirstItem)
    return ResultFromScode (E_FAIL);

#ifdef _DEBUG
  TCHAR name[32];
  GetClipboardFormatName (pformatetcIn->cfFormat, name, sizeof(name));
#endif

  BOleDataItem *pWalk = pFirstItem;
  while (pWalk) {
    if (pformatetcIn->cfFormat == pWalk->fmtEtc.cfFormat &&
       pformatetcIn->tymed == pWalk->fmtEtc.tymed &&
       pformatetcIn->dwAspect == pWalk->fmtEtc.dwAspect) {
      *pmedium = pWalk->tymed;
      if( pWalk->fmtEtc.tymed == TYMED_ISTREAM )
        pWalk->tymed.pstm->AddRef();
      if( pWalk->fmtEtc.tymed == TYMED_ISTORAGE )
        pWalk->tymed.pstg->AddRef();
      // so we go away when they release the storage
      pmedium->pUnkForRelease = this;
      pmedium->pUnkForRelease->AddRef();
      // old pUnkForRelease is still in the table
      return NOERROR;
    }
    pWalk = pWalk->pNext;
  }

  return ResultFromScode (DATA_E_FORMATETC);
}

HRESULT _IFUNC BOleData::GetDataHere (LPFORMATETC pformatetc,
                                      LPSTGMEDIUM pmedium)
{
  pmedium->tymed = TYMED_NULL;

  if (!pFirstItem)
    return ResultFromScode (E_FAIL);
  if (!pformatetc || !(pformatetc->tymed & TYMED_ISTORAGE))
    return ResultFromScode (DV_E_FORMATETC);
  if (!pmedium)
    return ResultFromScode (DV_E_STGMEDIUM);

  BOleDataItem *pWalk = pFirstItem;
  while (pWalk) {
    if (pformatetc->cfFormat == pWalk->fmtEtc.cfFormat &&
      pformatetc->tymed == pWalk->fmtEtc.tymed &&
       pformatetc->dwAspect == pWalk->fmtEtc.dwAspect) {
      pmedium->tymed = TYMED_ISTORAGE;
      return pWalk->tymed.pstg->CopyTo (NULL, NULL, NULL, pmedium->pstg);
    }
    pWalk = pWalk->pNext;
  }

  return ResultFromScode (DV_E_FORMATETC);
}

HRESULT _IFUNC BOleData::QueryGetData (LPFORMATETC pformatetc)
{
  if (!pFirstItem)
    return ResultFromScode (E_FAIL);
  if (!pformatetc)
    return ResultFromScode (DV_E_FORMATETC);

  BOleDataItem *pWalk = pFirstItem;
  while (pWalk) {
    if (pformatetc->cfFormat == pWalk->fmtEtc.cfFormat &&
        pformatetc->tymed == pWalk->fmtEtc.tymed &&
       pformatetc->dwAspect == pWalk->fmtEtc.dwAspect) {
      return NOERROR;
    }
    pWalk = pWalk->pNext;
  }

  return ResultFromScode (DV_E_FORMATETC);
}

HRESULT _IFUNC BOleData::GetCanonicalFormatEtc (LPFORMATETC pformatetc,
                        LPFORMATETC pformatetcOut)
{
  return ResultFromScode (DATA_S_SAMEFORMATETC);
}

HRESULT _IFUNC BOleData::SetData (LPFORMATETC pformatetc,
                  LPSTGMEDIUM pmedium,
                  BOOL fRelease)
{
  // We own the data
  //
#ifdef _DEBUG
  assert(fRelease);
#endif
  if (!fRelease)
    return ResultFromScode (E_FAIL);

  // If the input pointers are NULL, we're supposed to delete the list
  //
  if (!pformatetc || !pmedium) {
    FreeItems();
    return NOERROR;
  }

  // Allocate a new item to hold data
  //
  LPMALLOC pMalloc = NULL;
  HRESULT hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
  if (!SUCCEEDED(hr))
    return hr;

  BOleDataItem FAR *pTmp = NULL;
  pTmp = (BOleDataItem FAR*) pMalloc->Alloc (sizeof(BOleDataItem));
  pMalloc->Release ();
  if (!pTmp)
    return ResultFromScode (E_OUTOFMEMORY);
  else
    memset (pTmp, 0, sizeof(BOleDataItem));


  // Insert formats onto the back of the list. Minimizes list traversal and
  // keeps the priority order of the formats intact
  //
  if (pLastItem) {
    pLastItem->pNext = pTmp;
    pLastItem = pTmp;
  }
  else {
    pLastItem = pFirstItem = pTmp;
  }

  pTmp->fmtEtc = *pformatetc;
  pTmp->tymed = *pmedium;

  // When copying an embedding displayed as icon, it seems that the only
  // format which can have DVASPECT_ICON is the metafile. It doesn't work
  // if all formats have DVASPECT_ICON. Weird.
  //
  if (pTmp->fmtEtc.dwAspect == DVASPECT_ICON)
    if (pTmp->fmtEtc.cfFormat != CF_METAFILEPICT)
      pTmp->fmtEtc.dwAspect = DVASPECT_CONTENT;

  return NOERROR;
}

HRESULT _IFUNC BOleData::EnumFormatEtc (DWORD dwDirection,
                    LPENUMFORMATETC FAR* ppenumFormatEtc)
{
  *ppenumFormatEtc = NULL;

  if (dwDirection == DATADIR_SET) {
    *ppenumFormatEtc = NULL;
    return ResultFromScode (E_FAIL);
  }
  *ppenumFormatEtc = new BOleEnumFormatEtc (this);
  return *ppenumFormatEtc != NULL ? NOERROR : ResultFromScode (E_OUTOFMEMORY);
}

HRESULT _IFUNC BOleData::DAdvise (FORMATETC FAR* pFormatetc,
                                  DWORD advf,
                                  LPADVISESINK pAdvSink,
                                  DWORD FAR* pdwConnection)
{
  *pdwConnection = 0;
  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleData::DUnadvise (DWORD dwConnection)
{
  return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleData::EnumDAdvise (LPENUMSTATDATA FAR* ppenumAdvise)
{
  *ppenumAdvise = NULL;
  return ResultFromScode (E_FAIL);
}


//**************************************************************************
//
// IEnumFORMATETC Implementation
//
//**************************************************************************

BOleEnumFormatEtc::BOleEnumFormatEtc (BOleData *data) : pData (data), nRef (1)
{
#ifdef _DEBUG
  OutputDebugString( TEXT("EnumFormatEtc Created\n\r") );
#endif

  pCurItem = pFirstItem = pLastItem = NULL;
  Reset();
  // Hold the data object
  pData->AddRef();
}

HRESULT _IFUNC BOleEnumFormatEtc::QueryInterface(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces
  hr = IEnumFORMATETC_QueryInterface(this, iid, ppv);

  return hr;
}

ULONG _IFUNC BOleEnumFormatEtc::AddRef()
{
  return ++nRef;
}

ULONG _IFUNC BOleEnumFormatEtc::Release()
{
  if (--nRef == 0) {
#ifdef _DEBUG
    OutputDebugString( TEXT("EnumFormatEtc Deleted\n\r") );
#endif
    pData->Release();
    delete this;
    return 0;
  }
  else
    return nRef;
}

HRESULT _IFUNC BOleEnumFormatEtc::Next (ULONG celt,
                 FORMATETC FAR * rgelt,
                 ULONG FAR* pceltFetched)
{
  // In case the format list has changed since the last call, resynchronize
  Resync();

  // Initialize output parameters
  //
  if (pceltFetched)
    *pceltFetched = 0;
  if (rgelt)
    memset (rgelt, 0, sizeof(FORMATETC) * celt);

  // Error on output parameter
  //
  if (!rgelt)
    return ResultFromScode (DV_E_FORMATETC);

  // Either no data in the cache, or we've already enumerated them all
  //
  if (!pFirstItem || !pCurItem)
    return ResultFromScode (S_FALSE);

  // Loop over formats, copying as many as we have, or as many as they
  // asked for, whichever comes first
  //
  short nCopied = 0;
  while (pCurItem && (celt > 0)) {

    *rgelt = pCurItem->fmtEtc;    // Copy formats

    rgelt++;                // Increment pointers
    pCurItem = pCurItem->pNext;

    nCopied++;              // Increment counters
    celt--;
  }

  if (pceltFetched)
    *pceltFetched = nCopied;

  return NOERROR;
}

HRESULT _IFUNC BOleEnumFormatEtc::Skip (ULONG celt)
{
  // In case the format list has changed since the last call, resynchronize
  Resync();

  // Save off the current item pointer in case Skip fails
  //
  BOleDataItem FAR *pTmp = pCurItem;

  // Skip as many items as we can
  //
  ULONG count = 0;
  while (pCurItem && (count < celt)) {
    pCurItem = pCurItem->pNext;
    count++;
  }

  // Ran off the edge of the list before skipping the requested number
  // of items. Restore current item pointer
  //
  if (count < celt) {
    pCurItem = pTmp;
    return ResultFromScode (S_FALSE);
  }

  return NOERROR;
}

HRESULT _IFUNC BOleEnumFormatEtc::Reset ()
{
  // In case the format list has changed since the last call, resynchronize
  Resync();

  // Reset the cursor to the head of the list.
  //
  pCurItem = pFirstItem;
  return NOERROR;
}

HRESULT _IFUNC BOleEnumFormatEtc::Clone (LPENUMFORMATETC FAR* ppEnum)
{
  // In case the format list has changed since the last call, resynchronize
  Resync();

  *ppEnum = NULL;

  // Get a memory allocator from COM
  //
  LPMALLOC pMalloc = NULL;
  HRESULT hr = ::CoGetMalloc (MEMCTX_TASK, &pMalloc);
  if (!SUCCEEDED(hr))
    return hr;

  // Allocate a new BOleDataObject
  //
  BOleEnumFormatEtc FAR *pClone = (BOleEnumFormatEtc FAR*) pMalloc->Alloc (sizeof(BOleEnumFormatEtc));
  pMalloc->Release ();
  if (!pClone)
    return ResultFromScode (E_OUTOFMEMORY);

  // Copy the pointers from this BOleData object into the new one.
  // Note that the data cache is not copied, but I think that's ok since
  // cloning the enumerator is supposed to provide a snapshot of the state,
  // not necessarily the whole data cache
  //

  memcpy (pClone, this, sizeof(BOleEnumFormatEtc));
  *ppEnum = pClone;

  return NOERROR;
}


void BOleEnumFormatEtc::Resync(){
  // Now that the enumerator is seperated from the data object, we have
  // to make sure the format list is still the same. The cheap way is just
  // to check that the two ends of the list are still the same, otherwise
  // we decide to restart
  //
  if( pFirstItem != pData->pFirstItem || pLastItem != pData->pLastItem ){
    pFirstItem = pData->pFirstItem;
    pLastItem = pData->pLastItem;
    pCurItem = pData->pFirstItem;
  }
}

//**************************************************************************
//
// BOleShadowData --  Prevent EmptyClipboard from deleting our real object
//                   when it calls CoDisconnectObject. (see class defn)
//
//**************************************************************************

BOleShadowData::BOleShadowData (BOleClassManager *pCM,
                                LPDATAOBJECT pDeleg)   :
                                pDelegate (pDeleg),
                                BOleComponent(pCM, NULL)
{
  pDelegate->AddRef();
}

BOleShadowData::~BOleShadowData ()
{
  if (NOERROR == OleIsCurrentClipboard (this))
    pFactory->GetService()->NotifyClipboardEmptied();

  pDelegate->Release();
}

HRESULT _IFUNC BOleShadowData::QueryInterfaceMain(REFIID iid, void FAR* FAR* pif)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *pif = NULL;

  // interfaces

  if (iid == IID_BOleShadowData) {
    (BOleShadowData*) *pif = this;
    AddRef ();
    hr = NOERROR;
  }
  else if (SUCCEEDED(hr = IDataObject_QueryInterface(this, iid, pif))) {
  }
  else if SUCCEEDED(hr = BOleComponent::  QueryInterfaceMain(iid, pif)) {
  }
  return hr;
}

HRESULT _IFUNC BOleShadowData::GetData (LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
  return pDelegate->GetData (pformatetcIn, pmedium);
}

HRESULT _IFUNC BOleShadowData::GetDataHere (LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
  return pDelegate->GetDataHere (pformatetc, pmedium);
}

HRESULT _IFUNC BOleShadowData::QueryGetData (LPFORMATETC pformatetc)
{
  return pDelegate->QueryGetData (pformatetc);
}

HRESULT _IFUNC BOleShadowData::GetCanonicalFormatEtc (LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{
  return pDelegate->GetCanonicalFormatEtc (pformatetc, pformatetcOut);
}

HRESULT _IFUNC BOleShadowData::SetData (LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease)
{
  return pDelegate->SetData (pformatetc, pmedium, fRelease);
}

HRESULT _IFUNC BOleShadowData::EnumFormatEtc (DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{
  return pDelegate->EnumFormatEtc (dwDirection, ppenumFormatEtc);
}

HRESULT _IFUNC BOleShadowData::DAdvise (FORMATETC FAR* pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
  return pDelegate->DAdvise (pFormatetc, advf, pAdvSink, pdwConnection);
}

HRESULT _IFUNC BOleShadowData::DUnadvise (DWORD dwConnection)
{
  return pDelegate->DUnadvise (dwConnection);
}

HRESULT _IFUNC BOleShadowData::EnumDAdvise (LPENUMSTATDATA FAR* ppenumAdvise)
{
  return pDelegate->EnumDAdvise (ppenumAdvise);
}


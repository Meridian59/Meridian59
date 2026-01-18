//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the server helper for documents by masquerading as the
//  container object. Provides plumbing for server linking to documents
//  and items
//----------------------------------------------------------------------------
#include "bolecont.h"
#include "bolefact.h"
#include "bolecman.h"
#include "bolepart.h"  // for link or embed to embedding

HRESULT _IFUNC BOleContainer::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces

     SUCCEEDED(hr = IBRootLinkable_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IPersistFile_QueryInterface(this, iid, ppv))
  || SUCCEEDED(hr = IOleItemContainer_QueryInterface(this, iid, ppv))
  // base classes

  || SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))

  // helpers
  ;
  return hr;
};

BOleContainer::BOleContainer (BOleClassManager *pF, IBUnknownMain * pObj) :
  BOleComponent(pF, pObj)
{
  pDoc = NULL;
  regId = 0L;
  pMoniker = NULL;
  pClassFact = NULLP;
}

BOleContainer::~BOleContainer ()
{
  if (pClassFact) {
    pClassFact->Release ();
    pClassFact = NULL;
  }
}

/**************************************************************************/
// IOleItemContainer
/**************************************************************************/

HRESULT _IFUNC BOleContainer::GetClassID (LPCLSID lpClassID)
{
  HRESULT hrErr = ResultFromScode (E_FAIL);
  // outer could have called IRootLinkable::Init.
  if (lpClassID && pClassFact) {
    hrErr = pClassFact->GetClassID(lpClassID);
  }
  else {
    // this is sometimes called for documents not attached to servers!
    *lpClassID = CLSID_NULL;
  }
  return hrErr;
}

HRESULT _IFUNC BOleContainer::IsDirty ()
{
  HRESULT hrErr = ResultFromScode (S_OK);


  return hrErr;
}

#define OLE_MAXNAMESIZE     (256)

HRESULT _IFUNC BOleContainer::ParseDisplayName (IBindCtx *pbc, LPOLESTR pszDisplayName,
                                                ULONG FAR* pchEaten, IMoniker* FAR* ppmk)
{
  *ppmk = NULL;

  OLECHAR   szItemName[OLE_MAXNAMESIZE];
  OLECHAR  *pszDest = szItemName;
  LPCOLESTR pszSrc = pszDisplayName;

  // skip leading delimiters
  int cEaten = 0;
#if defined(DBCS)
  while (*pszSrc != '\0' && (*pszSrc == '\\' || *pszSrc == '/' ||
    *pszSrc == '!' || *pszSrc == '['))
  {
    if(IsDBCSLeadByte(*pszSrc))
    {
      ++pszSrc;
      ++cEaten;
    }

    ++pszSrc;
    ++cEaten;
  }
#else
  while (*pszSrc != '\0' && (*pszSrc == '\\' || *pszSrc == '/' ||
    *pszSrc == '!' || *pszSrc == '['))
  {
    ++pszSrc;
    ++cEaten;
  }
#endif

#if defined(DBCS)
  // parse next token into szItemName
  while (*pszSrc != '\0' && *pszSrc != '\\' && *pszSrc != '/' &&
    *pszSrc != '!' && *pszSrc != '[' &&  cEaten < OLE_MAXNAMESIZE-1)
  {
    if(IsDBCSLeadByte(*pszSrc))
    {
      *pszDest++ = *pszSrc++;
      ++cEaten;
    }
    *pszDest++ = *pszSrc++;
    ++cEaten;
  }
#else
  // parse next token into szItemName
  while (*pszSrc != '\0' && *pszSrc != '\\' && *pszSrc != '/' &&
    *pszSrc != '!' && *pszSrc != '[' &&  cEaten < OLE_MAXNAMESIZE-1)
  {
    *pszDest++ = *pszSrc++;
    ++cEaten;
  }
#endif
  *pchEaten = cEaten;
  *pszDest = 0;

  // attempt to get the object
  LPUNKNOWN pUnknown;
  HRESULT hr = GetObject(szItemName, BINDSPEED_INDEFINITE, pbc,
    IID_IUnknown, &(LPVOID)pUnknown);
  if (SUCCEEDED(hr))  {
    pUnknown->Release();
    // item name found -- create item moniker for it
    return ::CreateItemMoniker (OLESTDDELIMW, szItemName, ppmk);
  }
  else {
    return hr;
  }
}


HRESULT _IFUNC BOleContainer::EnumObjects (DWORD, IEnumUnknown* FAR* ppenumUnknown)
{
  HRESULT hrErr = ResultFromScode(E_NOTIMPL);
  *ppenumUnknown = NULLP;
  return hrErr;
}

HRESULT _IFUNC BOleContainer::LockContainer (BOOL fLock)
{
  //  Keep app from closing
  //
  pFactory->ServerCount(fLock ? +1 : -1);
  //  Lock external, but pass FALSE as last arg, so in-proc "self" linking
  //  work (otherwise, Close gets called).
  //

  if (fLock)
    AddRef();
  else
    Release();

  return NOERROR;

}

HRESULT _IFUNC BOleContainer::GetObject (LPOLESTR lpszItem,
  DWORD  dwSpeedNeeded, IBindCtx* pbc, REFIID riid, LPVOID FAR* ppvObject)
{
  HRESULT hrErr = ResultFromScode(MK_E_NOOBJECT);
  *ppvObject = NULL;
  PIBPart pPart;
  if (SUCCEEDED(pDoc->GetPart(& pPart, lpszItem))) {
    // bind speed always INDEFINITE? p.803
    IUnknown *pUnk = pPart;
    BOleSite *pBOleSite;
    BOlePart *pBOlePart;
    if (SUCCEEDED(pUnk->QueryInterface(IID_BOlePart, &(LPVOID)pBOlePart))) {
      // Link to embedding case give the real IOleObject
      //
      if (pBOlePart->pOleObject) {
        pUnk = pBOlePart->pOleObject;
      }
      // Links to items need to maintain locks on their document
      // or the ProxyManager gets hosed.
      // When the BOlePart goes away it unlocks this.
      //
      if (!pBOlePart->pDocument)  // only ONCE
        pBOlePart->pDocument = AsPIUnknown(pObjOuter);
      CoLockObjectExternal(AsPIUnknown(pObjOuter), TRUE, TRUE);
      pBOlePart->Release();
    }
    else if (SUCCEEDED(pUnk->QueryInterface(IID_BOleSite, &(LPVOID)pBOleSite))) {
      // Links to items need maintain locks on their documents
      // or the ProxyManager gets hosed.
      // When the BOleSite goes away it unlocks this.
      //
      if (!pBOleSite->pDocument)  // only ONCE
        pBOleSite->pDocument = AsPIUnknown(pObjOuter);
      pBOleSite->pDocument = AsPIUnknown(pObjOuter);
      CoLockObjectExternal(AsPIUnknown(pObjOuter), TRUE, TRUE);
      pBOleSite->Release();
    }
    if (SUCCEEDED(hrErr=OleRun(pUnk))) {
      hrErr = pUnk->QueryInterface( riid, ppvObject);
    }
    pPart->Release();
    pPart = NULL;
    }
  return hrErr;
}

HRESULT _IFUNC BOleContainer::GetObjectStorage (LPOLESTR lpszItem, IBindCtx* pbc, REFIID riid, LPVOID FAR* ppvStorage)
{
  *ppvStorage = NULL;

  if (riid != IID_IStorage)
    return ResultFromScode(E_UNEXPECTED);

  // could get part and ask for BOlePart or BOleSite
  // and get the pStorage out of there
  // noone calls this

  return ResultFromScode(MK_E_NOSTORAGE);
}

HRESULT _IFUNC BOleContainer::IsRunning (LPOLESTR lpszItem)
{
  HRESULT hrErr = ResultFromScode(MK_E_NOOBJECT);
  IOleObject *pOleObj = NULL;
  PIBPart pPart;
  if (SUCCEEDED(pDoc->GetPart(&pPart, lpszItem))) {

    IUnknown *pUnk = pPart;
    // check for link to embedding case!
    // if so, get the IOleObject out of the Bolero helper and use that!
    BOlePart *pBOlePart;
    if (SUCCEEDED(pUnk->QueryInterface(IID_BOlePart, &(LPVOID)pBOlePart))) {
      if (pBOlePart->pOleObject) {
        pUnk = pBOlePart->pOleObject;
      }
      pBOlePart->Release();
    }

    if (SUCCEEDED(pUnk->QueryInterface(IID_IOleObject,&(LPVOID)pOleObj))) {
      hrErr = OleIsRunning(pOleObj)
        ? NOERROR
        : ResultFromScode(S_FALSE);
      pOleObj->Release();
      pOleObj = NULL;
    }
    pPart->Release();
  }
  return hrErr;
}

//**************************************************************************
//
// IPersistFile implementation
//
//**************************************************************************

HRESULT _IFUNC BOleContainer::Load (LPCOLESTR lpszFileName, DWORD grfMode)
{
  HRESULT hr = NOERROR;

  hr = pDoc->Init(lpszFileName);

  return hr;
}

HRESULT _IFUNC BOleContainer::Save (LPCOLESTR lpszFileName, BOOL fRemember)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleContainer::SaveCompleted (LPCOLESTR lpszFileName)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleContainer::GetCurFile (LPOLESTR FAR* lplpszFileName)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT  _IFUNC BOleContainer::OnRename(PIBLinkable pContainer, LPCOLESTR pszName)
{
  HRESULT hr;
  LPMONIKER pMon;

  if (pszName)  {
    LPMONIKER pmkDoc = NULL;
    if (pContainer && SUCCEEDED(pContainer->GetMoniker(&pmkDoc))) {
      // intermediate moniker
      LPMONIKER pmkObj = NULL;

      if (SUCCEEDED(CreateItemMoniker (OLESTR("!"), (LPOLESTR)pszName, &pmkObj))) {
        hr = CreateGenericComposite(pmkDoc, pmkObj, &pMon);
        pmkObj->Release();
      }
      pmkDoc->Release();
    }
    else {
      // root moniker
      hr = OLE::CreateFileMoniker ((LPOLESTR)pszName, &pMon);
    }
  }
  else
    pMon = NULL;

  hr = SetMoniker (pMon);
  if (pMon)
    pMon->Release();

  return hr;
}

//**************************************************************************
//
// IBLinkable implementation
//
//**************************************************************************

HRESULT _IFUNC BOleContainer::Init( PIBContains pC, LPCOLESTR pszProgId)
{
  pDoc = pC;
  pFactory->GetService()->FindClassFactory( pszProgId, &pClassFact);
  return pClassFact ? NOERROR : ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleContainer::SetMoniker(LPMONIKER pMon)
{
  HRESULT hr;
  // get the running object table
  LPRUNNINGOBJECTTABLE pROT = NULL;
  hr = OLE::GetRunningObjectTable (0, &pROT);
  if (SUCCEEDED(hr)) {
    // Register the new moniker BEFORE revoking the old moniker.
    // Otherwise the object's "StubManager" gets hosed.
    //
    DWORD oldRegId = regId;

    if (pMoniker) {
      pMoniker->Release();
      pMoniker = NULLP;
    }
    // Store the new moniker if any
    if (pMon) {
      pMoniker = pMon;
      pMoniker->AddRef();

      // Register in the running object table
      //
      hr = pROT->Register (0, //ROTFLAGS_REGISTRATIONKEEPSALIVE,
        AsPIUnknown(pObjOuter), pMoniker, &regId);
    }
    else
      regId = 0L;

    // release any old monikers and running object table registration
    if (oldRegId != 0L) {
      pROT->Revoke(oldRegId);
      oldRegId = 0L;
    }
    pROT->Release();
  }
  return hr;
}

HRESULT _IFUNC BOleContainer::GetMoniker(LPMONIKER *ppMon)
{
  HRESULT hrErr = ResultFromScode (E_FAIL);
  if (pMoniker) {
    pMoniker->AddRef();
    *ppMon = pMoniker;
    hrErr = NOERROR;
  }
  else {
    // Must be a nested embedding. Ask the container for one.

    IOleObject *pOleObj;
    if (SUCCEEDED(QueryInterface(IID_IOleObject, &(LPVOID)pOleObj))) {
      hrErr = pOleObj->GetMoniker(OLEGETMONIKER_ONLYIFTHERE, OLEWHICHMK_OBJFULL, ppMon);
      pOleObj->Release();
    }
  }
  return hrErr;
}





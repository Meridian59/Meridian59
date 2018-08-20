//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the server-side helper for allowing OLE2 to create objects
//  made available by Bolero server apps.
//----------------------------------------------------------------------------
#include "bolefact.h"
#include "bolesvc.h"
#include "boleips.h"
#include "bolecman.h"


//
//  DllGetClassObject
//

STDAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
{
  HRESULT hRes =  ResultFromScode(E_OUTOFMEMORY);
  return hRes;
}


STDAPI  DllCanUnloadNow()
{
  return ResultFromScode( S_OK );
}


BOleFact::BOleFact(BOleClassManager * pFact, IBUnknownMain *pOuter):
    RefCnt (1),
    pObjCM(NULL),
    hReg (0L),
    regId (0),
    dwRegisterActiveObject (0),
    id (CLSID_NULL),
    BOleComponent(pFact, (IBUnknownMain *) this),
    pIPEFact (NULL)
{
}

BOleFact::~BOleFact()
{
  if (pIPEFact) {
    pIPEFact->Release();
    pIPEFact = NULL;
  }
}

HRESULT _IFUNC BOleFact::QueryInterfaceMain(REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces

  SUCCEEDED(hr = IBClass_QueryInterface(this, iid, ppv))

  // base classes

  || SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))

  // helpers
  ;

  return hr;
}

HRESULT _IFUNC BOleFact::Init (BOOL fInProc, LPCOLESTR szProgId,
                                            IBClassMgr *pCM, BCID rid)
{
  pObjCM = pCM;
  regId = rid;

  // If the user's object is capable of being instantiated in its own
  // container, create a special BOleInProcFactory
  //
  if (fInProc) {
    pIPEFact = new BOleInProcFactory (pFactory, this, id);
    if (!pIPEFact)
      return ResultFromScode(E_OUTOFMEMORY);
  }

  return CLSIDFromProgID(szProgId, &id);
}

HRESULT _IFUNC BOleFact::Register (BOOL fSingleUse)
{
  HRESULT hErr;

  // register the class factory for this id
  //
  if (!SUCCEEDED(hErr = CoRegisterClassObject(
      id,
      (IClassFactory *)this,
      CLSCTX_LOCAL_SERVER,
      fSingleUse ? REGCLS_SINGLEUSE : REGCLS_MULTI_SEPARATE,
      &hReg)))
  {
    OLEHRES("Failed Registering", hErr);
  }

  // Register the embedding helper factory if we embed in ourselves
  if (pIPEFact) {
    if (!SUCCEEDED(hErr = pIPEFact->Register (fSingleUse)))
      return hErr;
  }

  // The online dox for CoLockObjectExternal say we should do this here
  //
  if (SUCCEEDED(hErr)) {
    LPCLASSFACTORY pWorkAround = this;
    CoLockObjectExternal (pWorkAround, TRUE, TRUE);
  }

  // if the class manager supports automation let's register it as "Active"
  //
  IDispatch *pDispatch;
  if (SUCCEEDED(pObjCM->QueryInterface(IID_IDispatch, &(LPVOID)pDispatch))) {
    RegisterActiveObject (pDispatch, id, NULL, &dwRegisterActiveObject);
    pDispatch->Release();
  }
  return hErr;
}

HRESULT _IFUNC BOleFact::Revoke ()
{
  if (hReg) {
    LPCLASSFACTORY pWorkAround = this;
    CoLockObjectExternal (pWorkAround, FALSE, TRUE);
    CoRevokeClassObject( hReg);
    hReg = 0;
  }

  if (dwRegisterActiveObject) {
    RevokeActiveObject (dwRegisterActiveObject, NULL);
    dwRegisterActiveObject = 0;
  }

  if (pIPEFact) {
    pIPEFact->Revoke ();
    pIPEFact->Release ();
    pIPEFact = NULL;
  }
  return NOERROR;
}

HRESULT _IFUNC BOleFact::GetClassID (LPCLSID pClass)
{
  *pClass = id;
  return NOERROR;
}

BOOL _IFUNC BOleFact::IsA (REFCLSID cid)
{
  return IsEqualCLSID(cid, id);
}

HRESULT _IFUNC BOleFact::CreateInstance(IUnknown FAR* pUnk, REFIID iid,
                                              LPVOID  FAR* ppv)
{
  HRESULT hr = ResultFromScode(E_FAIL);
  *ppv = 0L;

  PIUnknown pObj;
  IBClassMgr * pICM;

  if (SUCCEEDED(pObjCM->QueryInterface(IID_IBClassMgr, &(LPVOID)pICM))) {
    if (SUCCEEDED(pICM->ComponentCreate(&pObj, pUnk, regId))) {
      if (pUnk && iid == IID_IUnknown) {
        hr = NOERROR;
        *ppv = pObj;  // always give out main IUnknown
      }
      else {
        hr = pObj->QueryInterface(iid, ppv);
        pObj->Release();  // QI bumps
      }
    }
    pICM->Release();
  }

  return hr;
}


HRESULT _IFUNC BOleFact::LockServer( BOOL fLock )
{
  LPCLASSFACTORY pICF = this;
  CoLockObjectExternal(pICF, fLock, TRUE);
  pFactory->ServerCount( fLock ? 1L : -1L);

  return NOERROR;
}

// Used by DllGetClassObject to return the interface to the factory which
// creates the default handler which will create the in-proc server object.
//
HRESULT _IFUNC BOleFact::GetEmbeddingHelperFactory (LPUNKNOWN *ppF)
{
  if (pIPEFact) {
    *ppF = pIPEFact;
    (*ppF)->AddRef();
    return NOERROR;
  }
  else {
    *ppF = NULL;
    return ResultFromScode(E_FAIL);
  }
}

// When Bolero container-side dialog boxes need to know whether to put the
// CLSID for our own factories in the dialog box, this is how they find out
//
BOOL _IFUNC BOleFact::AllowEmbedFromSelf ()
{
  return pIPEFact ? TRUE : FALSE;
}

//**************************************************************************
//
// BOleInProcFactory -- The role of this factory is to load the default
//                      handler on the server side when the container
//                      side of an app needs to instantiate an object it
//                      serves.
//
//                      If we don't do this, the RPC/handler system
//                      creates the server object as if it were an inproc
//                      server with no handler, but our EXE-based objects
//                      must use the default handler.
//
//**************************************************************************

//***********************************
//
// Implementation methods
//
//***********************************

BOleInProcFactory::BOleInProcFactory (BOleClassManager *pCM,
                                      BOleFact FAR *pF, REFIID iid) :
  BOleComponent(pCM, NULL),
  iidServer (iid),
  nRef (1),
  pRealFactory (pF),
  regCookie (0)
{
  pF->AddRef();
}

BOleInProcFactory::~BOleInProcFactory ()
{
  if (pRealFactory) {
    pRealFactory->Release();
    pRealFactory = NULL;
  }
}

HRESULT _IFUNC BOleInProcFactory::Register (BOOL fSingleUse)
{
  HRESULT hr =  ::CoRegisterClassObject (
    iidServer, this, CLSCTX_INPROC_SERVER,
    fSingleUse ? REGCLS_SINGLEUSE : REGCLS_MULTI_SEPARATE, &regCookie);

  if (SUCCEEDED(hr)) {
    LPCLASSFACTORY pWorkAround = this;
    CoLockObjectExternal (pWorkAround, TRUE, TRUE);
  }

  return hr;
}

HRESULT _IFUNC BOleInProcFactory::Revoke ()
{
  if (regCookie) {
    LPCLASSFACTORY pWorkAround = this;
    CoLockObjectExternal (pWorkAround, FALSE, TRUE);
    return ::CoRevokeClassObject (regCookie);
  }

  if (pRealFactory) {               // this breaks a cycle when we're
    pRealFactory->Release();  // in the embed from file "registered case"
    pRealFactory = NULL;
  }
  // if we don't register we expect releasing BOleInProcFactory
  // to release BOleFact (since BOleInProcFactory is returned
  // from DllGetClassObject)
  //

  return ResultFromScode (E_FAIL);
}

//***********************************
//
// IUnknown methods
//
//***********************************

HRESULT _IFUNC BOleInProcFactory::QueryInterfaceMain(REFIID riid,
                                            LPVOID FAR *ppObj)
{
  HRESULT hr;
  *ppObj = NULL;
  if (SUCCEEDED(hr = IClassFactory_QueryInterface (this, riid, ppObj))) {
  }
  else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(riid, ppObj))) {
  }
  return hr;
}

//***********************************
//
// IClassFactory methods
//
//***********************************

HRESULT _IFUNC BOleInProcFactory::CreateInstance (LPUNKNOWN pUnkOuter,
                                  REFIID iidContainer, LPVOID FAR* ppv)
{
  // We ask OLE2 to create an instance
  // of the default handler. The default handler uses pRealFactory to
  // call back to us when it's time to create the real server object.
  //
  return ::OleCreateEmbeddingHelper (iidServer, pUnkOuter,
    EMBDHLP_INPROC_SERVER | EMBDHLP_DELAYCREATE, pRealFactory,
    iidContainer, ppv);
}

HRESULT _IFUNC BOleInProcFactory::LockServer (BOOL fLock)
{
  ::CoLockObjectExternal (this, fLock, TRUE);
  pFactory->ServerCount (fLock ? 1L : -1L);
  return NOERROR;
}



//***********************************
//
// BOleControlFactory methods
//
//***********************************

  // rayk - this is a place holder for the real control class factory
  //        this will properly handle the licensing information for
  //        controls that we create

BOleControlFact::BOleControlFact(BOleClassManager * pFact,
                                        IBUnknownMain *pOuter):
  BOleFact (pFact, pOuter)
{
}

BOleControlFact::~BOleControlFact()
{
}

HRESULT _IFUNC BOleControlFact::QueryInterfaceMain(REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // interfaces

  SUCCEEDED(hr = IClassFactory2_QueryInterface(this, iid, ppv))

  // base classes

  || SUCCEEDED(hr = BOleFact::QueryInterfaceMain(iid, ppv))

  // helpers
  ;

  return hr;
}


HRESULT _IFUNC BOleControlFact::CreateInstanceLic (LPUNKNOWN pUnkOuter,
                 LPUNKNOWN pUnkReserved, REFIID riid, BSTR bstrKey,
                  LPVOID FAR* ppvObject)
{
  return ResultFromScode(E_FAIL);
}


HRESULT _IFUNC BOleControlFact::CreateInstance(IUnknown FAR* pUnk,
                 REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = ResultFromScode(E_FAIL);
  *ppv = 0L;

  PIUnknown pObj;
  IBClassMgr * pICM;

  // rayk - need a check here to see if we think the license is valid
  if (SUCCEEDED(pObjCM->QueryInterface(IID_IBClassMgr, &(LPVOID)pICM))) {
    if (SUCCEEDED(pICM->ComponentCreate(&pObj, pUnk, regId))) {
      if (pUnk && iid == IID_IUnknown) {
        hr = NOERROR;
        *ppv = pObj;  // always give out main IUnknown
      }
      else {
        hr = pObj->QueryInterface(iid, ppv);
        pObj->Release();  // QI bumps
      }
    }
    pICM->Release();
  }

  return hr;
}



HRESULT _IFUNC BOleControlFact::GetLicInfo (LPLICINFO pLicInfo)
{
  return ResultFromScode(E_FAIL);
}

HRESULT _IFUNC BOleControlFact::RequestLicKey (DWORD dwResrved, BSTR FAR* pbstrKey)
{
  return ResultFromScode(E_FAIL);
}

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
#ifndef _BOLEFACT_H
#define _BOLEFACT_H

#include "bolecomp.h"

class _ICLASS BOleSite;
class _ICLASS BOleInProcFactory;

// class BOleFact -- OLE 2.0 Factory helper
//
class _ICLASS BOleFact : public BOleComponent, public IBClass
{
protected:
  unsigned RefCnt;
  BCID regId;
  IBClassMgr * pObjCM;
  DWORD hReg;
  DWORD dwRegisterActiveObject;
  CLSID id;
  BOleInProcFactory FAR* pIPEFact;

public:

  BOleFact(BOleClassManager * pFact, IBUnknownMain *pOuter);
  ~BOleFact();

  virtual HRESULT _IFUNC Init(BOOL fSelfEmbed, LPCOLESTR pszProgId,
                                   IBClassMgr * pCM, BCID rid);
  virtual HRESULT _IFUNC Register(BOOL fSingleUse);
  virtual HRESULT _IFUNC Revoke();
  virtual HRESULT _IFUNC GetClassID(LPCLSID pClass);
  virtual BOOL    _IFUNC IsA(REFCLSID);
  virtual BOOL    _IFUNC AllowEmbedFromSelf ();
  virtual HRESULT _IFUNC GetEmbeddingHelperFactory (LPUNKNOWN *ppF);

  IBClassMgr * ClassMgrGet() { return pObjCM; }


  operator const CLSID &() const {return id;}

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IClassFactory methods
  //
  HRESULT _IFUNC CreateInstance(IUnknown FAR* pUnk, REFIID iid, LPVOID FAR* ppv);
  HRESULT _IFUNC LockServer(BOOL fLock);
};


// class BOleControlFact -- OLE 2.0 Factory helper
//
class _ICLASS BOleControlFact : public BOleFact, public IClassFactory2
{
public:

  BOleControlFact(BOleClassManager * pFact, IBUnknownMain *pOuter);
  ~BOleControlFact();

  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);

  // IClassFactory methods
  //
  HRESULT _IFUNC CreateInstance(IUnknown FAR* pUnk, REFIID iid, LPVOID FAR* ppv);
  HRESULT _IFUNC LockServer(BOOL fLock)      { return BOleFact::LockServer (fLock);};

  //  IClassFactory2 methods
  HRESULT _IFUNC GetLicInfo (LPLICINFO pLicInfo);
  HRESULT _IFUNC RequestLicKey (DWORD dwResrved, BSTR FAR* pbstrKey);
  HRESULT _IFUNC CreateInstanceLic (LPUNKNOWN pUnkOuter,
                   LPUNKNOWN pUnkReserved, REFIID riid, BSTR bstrKey,
                   LPVOID FAR* ppvObject);
};

//
// BOleInProcFactory -- Private factory used when a server object is
//                      registered to be instantiated in its own container
//
class _ICLASS BOleInProcFactory : public BOleComponent, public IClassFactory
{
  public:

    BOleInProcFactory (BOleClassManager *,BOleFact FAR*, REFIID);
    ~BOleInProcFactory ();

    HRESULT _IFUNC Register (BOOL);
    HRESULT _IFUNC Revoke ();

    // IUnknown methods
    //
    virtual HRESULT _IFUNC QueryInterfaceMain (REFIID iid, LPVOID FAR* pif);
    DEFINE_IUNKNOWN(pObjOuter);

  protected:

    ULONG nRef;
    DWORD regCookie;
    BOleFact *pRealFactory;
    REFIID iidServer;

    // IClassFactory methods
    //
    virtual HRESULT _IFUNC CreateInstance (LPUNKNOWN pUnk, REFIID iid,
                                                      LPVOID FAR* ppv);
    virtual HRESULT _IFUNC LockServer (BOOL fLock);

};

#endif

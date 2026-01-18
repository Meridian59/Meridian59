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
#ifndef _BOLECONT_H
#define _BOLECONT_H 1

#include "bolecomp.h"

class _ICLASS BOleFact;

class _ICLASS BOleContainer :
  public BOleComponent,
  public IPersistFile,
  public IOleItemContainer,
#if 0
  public IExternalConnection,
#endif
  public IBRootLinkable
{
protected:
  DWORD regId;          // magic cookie for linkable document
  LPMONIKER pMoniker;   // file moniker
  PIBContains pDoc;     // back pointer
  IBClass *pClassFact;  // pointer to class object

public:
  BOleContainer (BOleClassManager *, IBUnknownMain *);
  ~BOleContainer ();

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter)

  // IUnknownMain methods
  //
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IPersistFile methods
  //
  HRESULT _IFUNC GetClassID(LPCLSID lpClassID);
  HRESULT _IFUNC IsDirty();
  HRESULT _IFUNC Load(LPCOLESTR lpszFileName, DWORD grfMode);
  HRESULT _IFUNC Save(LPCOLESTR lpszFileName, BOOL fRemember);
  HRESULT _IFUNC SaveCompleted(LPCOLESTR lpszFileName);
  HRESULT _IFUNC GetCurFile(LPOLESTR FAR* lplpszFileName);

  // IOleItemContainer methods
  //
  HRESULT _IFUNC ParseDisplayName(IBindCtx* pbc, LPOLESTR lpszDisplayName,
    ULONG FAR* pchEaten, IMoniker* FAR* ppmkOut);
  HRESULT _IFUNC EnumObjects(DWORD grfFlags,IEnumUnknown* FAR* ppenumUnknown);
  HRESULT _IFUNC LockContainer(BOOL fLock);
  HRESULT _IFUNC GetObject(LPOLESTR lpszItem, DWORD dwSpeedNeeded, IBindCtx* pbc,
                             REFIID riid, LPVOID FAR* ppvObject);
  HRESULT _IFUNC GetObjectStorage(LPOLESTR lpszItem, IBindCtx* pbc, REFIID riid,
                                    LPVOID FAR* ppvStorage);
  HRESULT _IFUNC IsRunning(LPOLESTR lpszItem);

  // IBLinkable

  // pass in class name (if reg'd in .reg)
  //
  virtual HRESULT _IFUNC Init(PIBContains, LPCOLESTR pszProgId);
  virtual HRESULT _IFUNC SetMoniker(LPMONIKER pMon);

  // pass NULL to OnRename when Doc closes
  //
  virtual HRESULT _IFUNC OnRename(PIBLinkable, LPCOLESTR) ;

  virtual HRESULT _IFUNC GetMoniker(LPMONIKER *ppMon);

#if 0
  virtual DWORD _IFUNC AddConnection(DWORD, DWORD);
  virtual DWORD _IFUNC ReleaseConnection(DWORD, DWORD, BOOL);
#endif
};

#endif _BOLECONT_H


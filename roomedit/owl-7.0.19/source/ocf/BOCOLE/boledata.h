//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
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
#ifndef _BOLEDATA_H
#define _BOLEDATA_H

#include "bole.h"
#include "bolecomp.h"

// This is where the cached data actually lives.
//
typedef struct FAR tagBOleDataItem {
  struct FAR tagBOleDataItem FAR *pNext;
  FORMATETC    fmtEtc;
  STGMEDIUM    tymed;
} BOleDataItem;

// BOleData -- Provides a data cache object for use when copying from
//             an embedding.
//
class _ICLASS BOleData : public BOleComponent,
                         public IDataObject
{
  public:

    BOleData (BOleClassManager *, PIBUnknownMain);
    ~BOleData ();

    // IUnknown Methods
    //
    DEFINE_IUNKNOWN (pObjOuter);
    HRESULT _IFUNC QueryInterfaceMain (REFIID iid, LPVOID FAR* pif);

  protected:

    BOleDataItem FAR *pFirstItem;
    BOleDataItem FAR *pLastItem;
    void FreeItems();

    friend  class _ICLASS  BOleEnumFormatEtc;

    // IDataObject methods
    //
    virtual HRESULT _IFUNC GetData (LPFORMATETC, LPSTGMEDIUM);
    virtual HRESULT _IFUNC GetDataHere (LPFORMATETC, LPSTGMEDIUM);
    virtual HRESULT _IFUNC QueryGetData (LPFORMATETC);
    virtual HRESULT _IFUNC GetCanonicalFormatEtc (LPFORMATETC, LPFORMATETC);
    virtual HRESULT _IFUNC SetData (LPFORMATETC, STGMEDIUM FAR *, BOOL);
    virtual HRESULT _IFUNC EnumFormatEtc (DWORD, LPENUMFORMATETC FAR*);
    virtual HRESULT _IFUNC DAdvise (FORMATETC FAR*, DWORD, LPADVISESINK, DWORD FAR*);
    virtual HRESULT _IFUNC DUnadvise (DWORD);
    virtual HRESULT _IFUNC EnumDAdvise (LPENUMSTATDATA FAR*);

};

// BOleEnumFormatEtc -- Provides a "cursor" on the list of formats
//                      offered by BOleData
//
class _ICLASS BOleEnumFormatEtc : public IEnumFORMATETC
{
  public:

  BOleEnumFormatEtc( BOleData *Data );

  // IUnknown methods
  //
  HRESULT _IFUNC QueryInterface(REFIID iid, void FAR* FAR* pif);
  ULONG _IFUNC AddRef();
  ULONG _IFUNC Release();

  // IEnumFORMATETC methods
  //
  HRESULT _IFUNC Next (ULONG celt, FORMATETC FAR * rgelt,
                ULONG FAR* pceltFetched);
  HRESULT _IFUNC Skip (ULONG celt);
  HRESULT _IFUNC Reset ();
  HRESULT _IFUNC Clone (IEnumFORMATETC FAR* FAR* ppenum);

  // Private methods
  //
  void Resync();

  BOleDataItem FAR *pCurItem;
  BOleDataItem FAR *pFirstItem;
  BOleDataItem FAR *pLastItem;
  BOleData *pData;
  LONG nRef;
};

// BOleShadowData -- The execution of EmptyClipboard() causes OLE2 to call
//                   CoDisconnectObject on the data object which has been
//                   posted to the clipboard.
//
//                   The consequence is that if we post the original objects
//                   (as opposed to clones of them), OLE2 might FORCE the
//                   refcount of such objects to zero on our behalf.
//
//                   To solve this we post a small shadow object which
//                   delegates IDataObject to the real object, but implements
//                   IUnknown here (stopping CoDisconnectObject from
//                   decrementing the real object until it goes to zero).
//
class _ICLASS BOleShadowData : public BOleComponent, public IDataObject
{
  public:

    BOleShadowData (BOleClassManager *pCM, LPDATAOBJECT pDeleg);
    virtual ~BOleShadowData();
    LPDATAOBJECT GetRealData() {return pDelegate;}

    DEFINE_IUNKNOWN(pObjOuter);

    // IDataObject
    //
    virtual HRESULT _IFUNC GetData (LPFORMATETC, LPSTGMEDIUM);
    virtual HRESULT _IFUNC GetDataHere (LPFORMATETC, LPSTGMEDIUM);
    virtual HRESULT _IFUNC QueryGetData (LPFORMATETC);
    virtual HRESULT _IFUNC GetCanonicalFormatEtc (LPFORMATETC, LPFORMATETC);
    virtual HRESULT _IFUNC SetData (LPFORMATETC, STGMEDIUM FAR *, BOOL);
    virtual HRESULT _IFUNC EnumFormatEtc (DWORD, LPENUMFORMATETC FAR*);
    virtual HRESULT _IFUNC DAdvise (FORMATETC FAR*, DWORD, LPADVISESINK, DWORD FAR*);
    virtual HRESULT _IFUNC DUnadvise (DWORD);
    virtual HRESULT _IFUNC EnumDAdvise (LPENUMSTATDATA FAR*);

    // IUnknown
    //
    virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, void FAR* FAR* pif);

  protected:

    LPDATAOBJECT pDelegate;
};

#endif


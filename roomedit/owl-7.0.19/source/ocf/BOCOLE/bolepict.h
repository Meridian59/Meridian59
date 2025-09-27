//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero versoin of the OLE2 picture object.
//----------------------------------------------------------------------------
#ifndef _BOLEPICT_H
#define _BOLEPICT_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif

class _ICLASS BEventList;

class _ICLASS BOlePicture : public BOleComponent,   public IBPicture,
                            public IPicture,        public IPictureDisp,
                            public IConnectionPointContainer
// rayk - should also support IPersistStream and IDataObject
{
protected:
  PICTDESC    Pict;
  long        Width;
  long        Height;
  bool        KeepOriginalFormat;
  long        Attrib;
  HDC         hdc;
  ITypeLib   *pTL;
  ITypeInfo  *pTI;
  BEventList *pEventList; // ConnectionPoints list

public:
   BOlePicture (BOleClassManager *pF, IBUnknownMain * pO);
   ~BOlePicture ();

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IPicture methods
  //
  HRESULT _IFUNC get_Handle (OLE_HANDLE FAR* phandle);
  HRESULT _IFUNC get_hPal (OLE_HANDLE FAR* phpal);
  HRESULT _IFUNC get_Type (short FAR* ptype);
  HRESULT _IFUNC get_Width (OLE_XSIZE_HIMETRIC FAR* pwidth);
  HRESULT _IFUNC get_Height (OLE_YSIZE_HIMETRIC FAR* pheight);
  HRESULT _IFUNC Render (HDC hdc, long x, long y, long cx, long cy,
                  OLE_XPOS_HIMETRIC xSrc, OLE_YPOS_HIMETRIC ySrc,
                  OLE_XSIZE_HIMETRIC cxSrc, OLE_YSIZE_HIMETRIC cySrc,
                  LPCRECT lprcWBounds);
  HRESULT _IFUNC set_hPal (OLE_HANDLE hpal);
  HRESULT _IFUNC get_CurDC (HDC FAR * phdcOut);
  HRESULT _IFUNC SelectPicture (HDC hdcIn, HDC FAR * phdcOut,
                  OLE_HANDLE FAR * phbmpOut);
  HRESULT _IFUNC get_KeepOriginalFormat (BOOL * pfkeep);
  HRESULT _IFUNC put_KeepOriginalFormat (BOOL fkeep);
  HRESULT _IFUNC PictureChanged ();
  HRESULT _IFUNC SaveAsFile (LPSTREAM lpstream, BOOL fSaveMemCopy,
                  LONG FAR * lpcbSize);
  HRESULT _IFUNC get_Attributes (DWORD FAR * lpdwAttr);


  // IDispatch methods
  //
  HRESULT _IFUNC GetTypeInfoCount (UINT FAR* pctinfo);
  HRESULT _IFUNC GetTypeInfo (UINT itinfo, LCID lcid,
                  ITypeInfo FAR* FAR* pptinfo);
  HRESULT _IFUNC GetIDsOfNames (REFIID riid, LPOLESTR FAR* rgszNames,
                  UINT cNames, LCID lcid, DISPID FAR* rgdispid);
  HRESULT _IFUNC Invoke (DISPID dispidMember, REFIID riid, LCID lcid,
                  WORD wFlags, DISPPARAMS FAR* pdispparams,
                  VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,
                  UINT FAR* puArgErr);

  // IConnectionPointContainer methods
  //
  HRESULT _IFUNC EnumConnectionPoints (LPENUMCONNECTIONPOINTS FAR*);
  HRESULT _IFUNC FindConnectionPoint (REFIID, LPCONNECTIONPOINT FAR*);

  // IBPicture method
  //
  HRESULT _IFUNC GetHandle (HANDLE FAR* phandle);
  HRESULT _IFUNC Draw (HDC hdc, long x, long y, long cx, long cy,
                  long xSrc, long ySrc, long cxSrc, long cySrc,
                  LPCRECT lprcWBounds);
  HRESULT _IFUNC PictChanged ();
  HRESULT _IFUNC SetPictureDesc (LPVOID pPD);  // LPVOID = PICTDESC *

protected:
  HRESULT      PropChanged (DISPID dispid);
  HRESULT      PropRequestEdit (DISPID dispid);
};


#endif

//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero versoin of the OLE2 picture object.
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolecman.h"
#include "bolepict.h"
#include "bocxctrl.h"
#include "bconnpnt.h"
#include <olectl.h>


//-----------
//  BOlePict implementation
//

// rayk - need to ensure that this object supports IPersistStream
BOlePicture::BOlePicture (BOleClassManager *pF, IBUnknownMain * pO) :
        BOleComponent (pF, pO),
        Width (0),                     Height (0),
        pTI (NULLP),                   pTL (NULLP),
        KeepOriginalFormat (FALSE),    hdc (0),
        Attrib (0),                    pEventList (NULLP)
{
  HRESULT err = LoadTypeLib (OLESTR ("bole.tlb"), &pTL);
  if (!err) {
    ITypeInfo *pTI;
    err = pTL->GetTypeInfoOfGuid (IID_BOlePicture, &pTI);
  }
  memset (&Pict, 0, sizeof (PICTDESC));
  Pict.cbSizeofstruct = sizeof (PICTDESC);

  // EventHandler deal with ConnecctionPoints
  pEventList = new BEventList (1);
  if (pEventList) {
    pEventList->AddRef();
    pEventList->AddEventsSet (IID_IPropertyNotifySink, AsPIUnknown(this), 5);
  }
}


BOlePicture::~BOlePicture ()
{
  if (pTI)
    pTI->Release ();
  if (pTL)
    pTL->Release ();
  if (pEventList)
    pEventList->Release();
}


HRESULT _IFUNC BOlePicture::QueryInterfaceMain (REFIID iid, LPVOID FAR *ppv)
{
   HRESULT hr = ResultFromScode(E_NOINTERFACE);
   *ppv = NULL;

   // Self
   //
   if (iid == IID_BOlePicture) {
      (BOlePicture *)*ppv = this;
      AddRef();
      return NOERROR;
   }

   // interfaces
   if (SUCCEEDED(hr = IPicture_QueryInterface (this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IPictureDisp_QueryInterface (this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IConnectionPointContainer_QueryInterface(this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IDispatch_QueryInterface(this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IBPicture_QueryInterface(this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))) {
   }
   return hr;
}

HRESULT _IFUNC BOlePicture::SetPictureDesc (LPVOID pPD)
{
  PICTDESC *pPicDes = (PICTDESC *)pPD;

  if (pPicDes->cbSizeofstruct == sizeof (PICTDESC)) {
    Width = 0;   Height = 0;   hdc = 0;
    memcpy (&Pict, pPD, sizeof (PICTDESC));
    if (Pict.picType == PICTYPE_BITMAP) {
      BITMAP bmp;
      GetObject (Pict.bmp.hbitmap, sizeof (BITMAP), &bmp);
      Width = bmp.bmWidth;
      Height = bmp.bmHeight;
    }
    return NOERROR;
  }
  return ResultFromScode (E_INVALIDARG);
}

HRESULT _IFUNC BOlePicture::GetHandle (HANDLE FAR* phandle)
{
  *phandle = (HANDLE) Pict.bmp.hbitmap;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::Draw (HDC hdc, long x, long y, long cx, long cy,
                 long xSrc, long ySrc, long cxSrc, long cySrc,
                 LPCRECT lprcWBounds)
{
  return Render (hdc, x, y, cx, cy, xSrc, ySrc, cxSrc, cySrc,
                lprcWBounds);
}

HRESULT _IFUNC BOlePicture::PictChanged ()
{
  return PictureChanged ();
}

HRESULT _IFUNC BOlePicture::Render (HDC hdcOut,
                long x, long y, long cx, long cy,
                OLE_XPOS_HIMETRIC xSrc, OLE_YPOS_HIMETRIC ySrc,
                OLE_XSIZE_HIMETRIC cxSrc, OLE_YSIZE_HIMETRIC cySrc,
                LPCRECT lprcWBounds)
{
  HRESULT   hr = NOERROR;
  // rayk - if Attrib == PICTURE_TRANSPARENT???
  // rayk - Do we need to check GetDeviceCaps for not support of StretchBlt?
  // rayk - hpal - SelectPalette last param should be true or false? (i.e. bForceBackground

  if ((!Pict.bmp.hbitmap) || (cx == 0) || (cy == 0))
    return hr;

  if (Pict.picType == PICTYPE_BITMAP) {

    HPALETTE  hOldPal = 0;
    HBITMAP   hOldBmp = 0;
    if (!hdc) {
      HDC   hScreenDC = GetDC (NULL);
      hdc = CreateCompatibleDC (hScreenDC);
      ReleaseDC (NULL, hScreenDC);
      hOldBmp = (HBITMAP) SelectObject (hdc, Pict.bmp.hbitmap);
    }

    if (Pict.bmp.hpal) {
      hOldPal = SelectPalette (hdcOut, Pict.bmp.hpal, FALSE);
      RealizePalette (hdcOut);
    }
    if (Attrib & PICTURE_SCALABLE) {
      StretchBlt (hdcOut, x, y, cx, cy, hdc, xSrc, ySrc, cxSrc, cySrc,
                             SRCCOPY);
    } else {
      BitBlt (hdcOut, x, y, cx, cy, hdc, xSrc, ySrc, SRCCOPY);
    }
    if (hOldBmp) {
      SelectObject (hdc, hOldBmp);
      ReleaseDC (NULL, hdc);
      hdc = 0;
    }
    if (Pict.bmp.hpal) {
      SelectPalette (hdcOut, hOldPal, TRUE);
      RealizePalette (hdcOut);
    }

  } else if (Pict.picType == PICTYPE_ICON) {

    DrawIcon (hdcOut, x, y, Pict.icon.hicon);

  } else if (Pict.picType == PICTYPE_METAFILE) {

    int   SavedDC;
    POINT ViewOrg;

    SavedDC = SaveDC (hdcOut);
    //  Set mapping mode to MM_ANISOTROPIC since we want the coordinate space
    //  to allow a non-1 by 1 mapping. Note the order of these calls is
    //  essential.  The mapping mode must be set to MM_ANISOTROPIC or
    //  Windows will ignore the WindowExt change which must preceed a
    //  ViewportExt change.
    SetMapMode (hdcOut, MM_ANISOTROPIC);
    //  Set the logical size of the DC to be the maximum X and maximum Y
    //  used in the metafile's drawing
    SetWindowExtEx (hdcOut, Pict.wmf.xExt, Pict.wmf.yExt, NULL);
    //  Set the DC 'window' (logical coordinate extent) to map on to the
    //  entire rectangle provided.
    SetViewportExtEx (hdcOut, cx, cy, NULL);
    //  Get the original viewport origin since we are relative to that
    GetViewportOrgEx (hdcOut, &ViewOrg);
    //  Move the viewport origin to match the top left of the rectangle
    SetViewportOrgEx (hdcOut, ViewOrg.x + x, ViewOrg.y + y, NULL);
    //  Now that the metafile's coordinate space has been setup, play the
    //  metafile
    PlayMetaFile (hdcOut, Pict.wmf.hmeta);
    //  Reset everything back the way it came.
    RestoreDC (hdcOut, SavedDC);
  }
  return hr;
}

HRESULT _IFUNC BOlePicture::get_Handle (OLE_HANDLE FAR* phandle)
{
  *phandle = (OLE_HANDLE)Pict.bmp.hbitmap;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_hPal (OLE_HANDLE FAR* phpal)
{
  *phpal = 0;
  if (Pict.picType == PICTYPE_BITMAP)
    *phpal = (OLE_HANDLE)Pict.bmp.hpal;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_Type (short FAR* ptype)
{
  *ptype = Pict.picType;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_Width (OLE_XSIZE_HIMETRIC FAR* pwidth)
{
  *pwidth = Width;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_Height (OLE_YSIZE_HIMETRIC FAR* pheight)
{
  *pheight = Height;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_KeepOriginalFormat (BOOL * pfkeep)
{
  *pfkeep = KeepOriginalFormat;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::get_Attributes (DWORD FAR * lpdwAttr)
{
  *lpdwAttr = Attrib;  // rayk - where does this get set?
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::put_KeepOriginalFormat (BOOL fkeep)
{
  // rayk - still need to implement KeepOriginalFormat
  // e.g. if bitmap is loaded in 24bit format, keep it around, or convert to 8-bit?
  KeepOriginalFormat = fkeep;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::set_hPal (OLE_HANDLE hpal)
{
  if (Pict.picType == PICTYPE_BITMAP) {
    if (PropRequestEdit (DISPID_PICT_HPAL) == S_OK) {
      (OLE_HANDLE)Pict.bmp.hpal = hpal;
      PropChanged (DISPID_PICT_HPAL);
    }
  }
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::SelectPicture (HDC hdcIn, HDC FAR * phdcOut,
                OLE_HANDLE FAR * phbmpOut)
{
  if (phdcOut)
    *phdcOut = 0;
  if (phbmpOut)
    *phbmpOut = 0;
  if ((Pict.picType == PICTYPE_BITMAP) && Pict.bmp.hbitmap) {
    HBITMAP hBmpOld;
    if (phdcOut)
      *phdcOut = hdc;

    hBmpOld = (HBITMAP) SelectObject (hdcIn, Pict.bmp.hbitmap);
    hdc = hdcIn;
    if (phbmpOut)
      *phbmpOut = (OLE_HANDLE)hBmpOld;
    return NOERROR;
  }
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOlePicture::get_CurDC (HDC FAR * phdcOut)
{
  *phdcOut = hdc;
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::PictureChanged ()
{
  PropChanged (DISPID_PICT_HANDLE);
  return NOERROR;
}

HRESULT _IFUNC BOlePicture::SaveAsFile (LPSTREAM lpstream, BOOL fSaveMemCopy,
                LONG FAR * lpcbSize)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOlePicture::GetTypeInfoCount (UINT FAR* pctinfo)
{
  *pctinfo = 1;
  if (!pTI)
    *pctinfo = 0;
  return NOERROR;
}


HRESULT _IFUNC BOlePicture::GetTypeInfo (UINT itinfo, LCID lcid,
   ITypeInfo FAR* FAR* pptinfo)
{
  if (!pTI)
    return ResultFromScode (E_NOTIMPL);
  *pptinfo = pTI;
  return NOERROR;
}


HRESULT _IFUNC BOlePicture::GetIDsOfNames (REFIID riid, LPOLESTR FAR* rgszNames,
   UINT cNames, LCID lcid, DISPID FAR* rgdispid)
{
  return DispGetIDsOfNames (pTI, rgszNames, cNames, rgdispid);
}


HRESULT _IFUNC BOlePicture::Invoke (DISPID dispidMember, REFIID riid, LCID lcid,
   WORD wFlags, DISPPARAMS FAR* pargs, VARIANT FAR* pVarResult,
   EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr)
{
  HRESULT    hr = NOERROR;

  if (pVarResult)
     VariantInit (pVarResult);

  if ( !(wFlags & DISPATCH_PROPERTYGET|DISPATCH_PROPERTYPUT))
    return ResultFromScode(E_INVALIDARG);

  if (wFlags & DISPATCH_PROPERTYGET) {

    if ( !((IID_NULL == riid) && (pVarResult)))
      return ResultFromScode(E_INVALIDARG);

    switch (dispidMember)
    {
      case DISPID_PICT_HEIGHT:
         V_I4 (pVarResult) = Height;
         V_VT (pVarResult) = VT_I4;
         break;

      case DISPID_PICT_WIDTH:
         V_I4 (pVarResult) = Width;
         V_VT (pVarResult) = VT_I4;
         break;

      case DISPID_PICT_TYPE:
         V_I2 (pVarResult) = Pict.picType;
         V_VT (pVarResult) = VT_I2;
         break;

      case DISPID_PICT_HANDLE:
         V_I4 (pVarResult) = (OLE_HANDLE)Pict.bmp.hbitmap;
         V_VT (pVarResult) = VT_I4;
         break;

      case DISPID_PICT_HPAL:
         V_I4 (pVarResult) = 0;
         if (Pict.picType == PICTYPE_BITMAP)
           V_I4 (pVarResult) = (OLE_HANDLE)Pict.bmp.hpal;
         V_VT (pVarResult) = VT_I4;
         break;

    }
  } else if (wFlags & DISPATCH_PROPERTYPUT) {

     if ((pargs->cArgs != 1) || (pargs->cNamedArgs != 1))
       return ResultFromScode(E_INVALIDARG);
     if (pargs->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
       return ResultFromScode(E_INVALIDARG);

    switch (dispidMember)
    {
      case DISPID_PICT_HPAL:
         hr = set_hPal (V_I4 (pargs->rgvarg));
         break;
    }
  }
  return hr;
}

// IConnectionPointContainer methods
//
HRESULT _IFUNC BOlePicture::EnumConnectionPoints
                                (LPENUMCONNECTIONPOINTS FAR* ppEnum)
{
  // if at least one connection point is here, return the collection!
  HRESULT hr = ResultFromScode (E_NOINTERFACE);
  if (pEventList && (*pEventList)[0]) {
    hr = pEventList->QueryInterface (IID_IEnumConnectionPoints,
                                                   (LPVOID *)ppEnum);
  }
  return hr;
}

HRESULT _IFUNC BOlePicture::FindConnectionPoint (REFIID iid, LPCONNECTIONPOINT FAR* ppCP)
{
  // only one connection point for BOlePicture (i.e. IPropertyNotifySink)
  //
  HRESULT hr = ResultFromScode (E_NOINTERFACE);
  IID     TempIID;

  (*pEventList)[0]->GetConnectionInterface (&TempIID); // get iid
  if (TempIID == iid) {
    IConnectionPoint *pCP = (*pEventList)[0];
    pCP->AddRef ();
    *ppCP = pCP;
    hr = NOERROR;
  }
  return hr;
}

// Service prop notify sink connection point
//
HRESULT BOlePicture::PropChanged (DISPID dispid)
{
  IBSinkList    *pSinkList;
  IBEventClass  *pEC;
  IPropertyNotifySink *pSink;


  (*pEventList)[0]->QueryInterface (IID_IBEventClass, &(LPVOID) pEC);
  pEC->GetSinkList (&pSinkList);
  pSinkList->Reset();
  while (pSinkList->NextSink (&(LPVOID)pSink) == S_OK){
    pSink->OnChanged (dispid);
    pSink->Release ();
  }
  pEC->Release ();
  return NOERROR;
}


HRESULT BOlePicture::PropRequestEdit (DISPID dispid)
{
  HRESULT result = NOERROR;
  IBSinkList    *pSinkList;
  IBEventClass  *pEC;
  IPropertyNotifySink *pSink;

  (*pEventList)[0]->QueryInterface (IID_IBEventClass, &(LPVOID) pEC);
  pEC->GetSinkList (&pSinkList);
  pSinkList->Reset ();
  while (pSinkList->NextSink (&(LPVOID)pSink) == S_OK){
    result = pSink->OnRequestEdit (dispid);
    pSink->Release ();
    if (FAILED (result))
      break;
  }
  pEC->Release ();
  return result;
}





//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Implements the Bolero versoin of the OLE2 font object.
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolecman.h"
#include "bolefont.h"
#include "bocxctrl.h"
#include <olectl.h>


//-----------
//  BOleFont implementation
//

// rayk - need to ensure that this object supports IPersistStream
BOleFont::BOleFont (BOleClassManager *pCF, IBUnknownMain * pO) :
        BOleComponent (pCF, pO),
        weight (FW_NORMAL),            modified (FALSE),
        charSet (DEFAULT_CHARSET),     style (fsBold),
        pTI (NULLP),                   pTL (NULLP),
        pF (NULLP),                    pEventList (NULLP),
        cyLogical (1),                 cyHimetric (1),
        bstrName (SysAllocString (OLESTR ("Arial")))
//        bstrName (SysAllocString (OLESTR ("MS Sans Serif")))
{
  SETFONTSIZE(height, 12);
  HRESULT err = LoadTypeLib (OLESTR ("bole.tlb"), &pTL);
  if (!err) {
    ITypeInfo *pTI;
    err = pTL->GetTypeInfoOfGuid (IID_BOleFont, &pTI);
  }
  // EventHandler deal with ConnecctionPoints
  pEventList = new BEventList (1);
  if (pEventList) {
    pEventList->AddRef();
    pEventList->AddEventsSet (IID_IPropertyNotifySink, AsPIUnknown(this), 5);
  }
}


BOleFont::~BOleFont ()
{
  SysFreeString (bstrName);
  if (pF)
    pF->Release ();
  if (pTI)
    pTI->Release ();
  if (pTL)
    pTL->Release ();
  if (pEventList)
    pEventList->Release();

}


HRESULT _IFUNC BOleFont::QueryInterfaceMain (REFIID iid, LPVOID FAR *ppv)
{
   HRESULT hr = ResultFromScode(E_NOINTERFACE);
   *ppv = NULL;

   // Self
   //
   if (iid == IID_BOleFont) {
      (BOleFont *)*ppv = this;
      AddRef();
      return NOERROR;
   }

   // interfaces
   if (SUCCEEDED(hr = IFont_QueryInterface (this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IFontDisp_QueryInterface (this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IConnectionPointContainer_QueryInterface(this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = IDispatch_QueryInterface(this, iid, ppv))) {
   }
   else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppv))) {
   }
   return hr;
}


HRESULT _IFUNC BOleFont::get_Name (BSTR FAR* pname)
{
  *pname = SysAllocString (bstrName);
  if ( !(*pname))
    return ResultFromScode (E_OUTOFMEMORY);
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Size (CY FAR* psize)
{
  XFERFONTSIZE ( (*psize), height);
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Weight (short FAR* pweight)
{
  *pweight = weight;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Charset (short FAR* pcharset)
{
  *pcharset = charSet;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Bold (BOOL FAR* pbold)
{
  *pbold = (style & fsBold) != 0;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Italic (BOOL FAR* pitalic)
{
  *pitalic = (style & fsItalic) != 0;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Underline (BOOL FAR* punderline)
{
  *punderline = (style & fsUnderline) != 0;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::get_Strikethrough (BOOL FAR* pstrikethrough)
{
  *pstrikethrough = (style & fsStrikeThrough) != 0;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Name (BSTR name)
{
  if (lstrcmp (name, bstrName) != 0) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      SysReAllocString (&bstrName, name);
      modified = TRUE;
      PropChanged (DISPID_FONT_NAME);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Size (CY value)
{
  if ( !ISEQUALFONTSIZE (value, height)) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      XFERFONTSIZE (height, value);
      modified = TRUE;
      PropChanged (DISPID_FONT_SIZE);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Bold (BOOL bold)
{
  if (bold != ((style & fsBold) != 0)) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      if (bold) {
    weight = FW_BOLD;
        style |= fsBold;
      } else {
        weight = FW_NORMAL;
        style &= ~fsBold;
      }
      modified = TRUE;
      PropChanged (DISPID_FONT_BOLD);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Italic (BOOL italic)
{
  if (italic != ((style & fsItalic) != 0)) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      if (italic)
        style |= fsItalic;
      else
        style &= ~fsItalic;
      modified = TRUE;
      PropChanged (DISPID_FONT_ITALIC);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Underline (BOOL underline)
{
  if (underline != ((style & fsUnderline) != 0)) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      if (underline)
        style |= fsUnderline;
      else
        style &= ~fsUnderline;
      modified = TRUE;
      PropChanged (DISPID_FONT_UNDER);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Strikethrough (BOOL strikethrough)
{
  if (strikethrough != ((style & fsStrikeThrough) != 0)) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      if (strikethrough)
        style |= fsStrikeThrough;
      else
        style &= ~fsStrikeThrough;
      modified = TRUE;
      PropChanged (DISPID_FONT_STRIKE);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Weight (short value)
{
  if (value != weight) {
    if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      weight = value;
      if (weight > (FW_BOLD + FW_NORMAL)/2 )
        style |= fsBold;
      else
        style &= ~fsBold;
      modified = TRUE;
      PropChanged (DISPID_FONT_WEIGHT);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::put_Charset (short value)
{
  if (value != charSet) {
  if (PropRequestEdit (DISPID_FONT_NAME) == S_OK) {
      charSet = value;
      modified = TRUE;
      PropChanged (DISPID_FONT_CHARSET);
    }
  }
  return NOERROR;
}


HRESULT _IFUNC BOleFont::GetTypeInfoCount (UINT FAR* pctinfo)
{
  *pctinfo = 1;
  if (!pTI)
    *pctinfo = 0;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::GetTypeInfo (UINT itinfo, LCID lcid,
   ITypeInfo FAR* FAR* pptinfo)
{
  if (!pTI)
    return ResultFromScode (E_NOTIMPL);
  *pptinfo = pTI;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::GetIDsOfNames (REFIID riid, LPOLESTR FAR* rgszNames,
   UINT cNames, LCID lcid, DISPID FAR* rgdispid)
{
  return DispGetIDsOfNames (pTI, rgszNames, cNames, rgdispid);
}


HRESULT _IFUNC BOleFont::Invoke (DISPID dispidMember, REFIID riid, LCID lcid,
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
      case DISPID_FONT_NAME:
         V_BSTR (pVarResult) = SysAllocString (bstrName);
         if ( !(V_BSTR (pVarResult)))
           return ResultFromScode (E_OUTOFMEMORY);
         V_VT (pVarResult) = VT_BSTR;
         break;

      case DISPID_FONT_SIZE:
         XFERFONTSIZE ( V_CY (pVarResult), height);
         V_VT (pVarResult) = VT_CY;
         break;

      case DISPID_FONT_CHARSET:
         V_I2 (pVarResult) = charSet;
         V_VT (pVarResult) = VT_I2;
         break;

      case DISPID_FONT_BOLD:
         V_BOOL (pVarResult) = (style & fsBold) != 0;
         V_VT (pVarResult) = VT_BOOL;
         break;

      case DISPID_FONT_ITALIC:
         V_BOOL (pVarResult) = (style & fsItalic) != 0;
         V_VT (pVarResult) = VT_BOOL;
         break;

    case DISPID_FONT_UNDER:
         V_BOOL (pVarResult) = (style & fsUnderline) != 0;
         V_VT (pVarResult) = VT_BOOL;
         break;

      case DISPID_FONT_STRIKE:
         V_BOOL (pVarResult) = (style & fsStrikeThrough) != 0;
         V_VT (pVarResult) = VT_BOOL;
         break;

      case DISPID_FONT_WEIGHT:
         V_I2 (pVarResult) = weight;
         V_VT (pVarResult) = VT_I2;
         break;
    }
  } else if (wFlags & DISPATCH_PROPERTYPUT) {
     if ((pargs->cArgs != 1) || (pargs->cNamedArgs != 1))
       return ResultFromScode(E_INVALIDARG);
     if (pargs->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
       return ResultFromScode(E_INVALIDARG);

    switch (dispidMember)
    {
      case DISPID_FONT_NAME:
         hr = put_Name (V_BSTR (pargs->rgvarg));
         break;

      case DISPID_FONT_SIZE:
         hr = put_Size (V_CY (pargs->rgvarg));
         break;

      case DISPID_FONT_CHARSET:
         hr = put_Charset (V_I2 (pargs->rgvarg));
         break;

      case DISPID_FONT_BOLD:
         hr = put_Bold (V_BOOL (pargs->rgvarg));
         break;

      case DISPID_FONT_ITALIC:
         hr = put_Italic (V_BOOL (pargs->rgvarg));
     break;

      case DISPID_FONT_UNDER:
         hr = put_Underline (V_BOOL (pargs->rgvarg));
         break;

      case DISPID_FONT_STRIKE:
         hr = put_Strikethrough (V_BOOL (pargs->rgvarg));
         break;

      case DISPID_FONT_WEIGHT:
         hr = put_Weight (V_I2 (pargs->rgvarg));
         break;

    }
  }
  return hr;
}


HRESULT _IFUNC BOleFont::get_hFont (HFONT FAR* phfont)
{
  HRESULT hr;
  BOleFontObj *pFontObj = NULL;

  if (!IsEqual (pF)) {
    pFontObj = FindFontObj ();
    if (pFontObj) {
      pFontObj->AddRef ();
    } else if ((!pF) || (!(pF->SingleRef()) && !IsEqual (pF))) {
      pFontObj = new BOleFontObj (pFactory, NULL);
      if (!pFontObj)
        return ResultFromScode (E_OUTOFMEMORY);
      pFactory->SetFontCache (pFontObj);
    }
    if (pF && pFontObj)  // if replace fontobj, release old one
      pF->Release ();
    if (pFontObj)        // now replace it
      pF = pFontObj;
    pF->CreateHFont (this);
  } else if (pF->hFont == 0)
  pF->CreateHFont (this);

  *phfont = NULL;
  if (pF->hFont)
    *phfont = pF->hFont;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::Clone (IFont FAR* FAR* lplpfont)
{
  IFont *pIF;

  pIF = new BOleFont (pFactory, NULL);
  if (!pIF)
    return ResultFromScode (E_OUTOFMEMORY);

  pIF->put_Name (bstrName);
  pIF->put_Size (height);
  pIF->put_Italic (style & fsItalic);
  pIF->put_Underline (style & fsUnderline);
  pIF->put_Strikethrough (style & fsStrikeThrough);
  pIF->put_Weight (weight);   // weight also covers fsBold
  pIF->put_Charset (charSet);
  *lplpfont = pIF;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::IsEqual (IFont FAR * lpFontOther)
{
  BSTR  name = NULL;
  BOOL  bValue;
  SHORT sValue;
  CY    hValue;
  HRESULT hr = S_OK;

  lpFontOther->get_Bold (&bValue);
  if (bValue != ((style & fsBold) != 0))
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Italic (&bValue);
  if (bValue != ((style & fsItalic) != 0))
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Underline (&bValue);
  if (bValue != ((style & fsUnderline) != 0))
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Strikethrough (&bValue);
  if (bValue != ((style & fsStrikeThrough) != 0))
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Weight (&sValue);   // weight also covers fsBold
  if (sValue != weight)
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Charset (&sValue);
  if (sValue != charSet)
    return ResultFromScode (S_FALSE);
  lpFontOther->get_Size (&hValue);
  if ( !ISEQUALFONTSIZE (hValue, height))
    return ResultFromScode (S_FALSE);
  hr = lpFontOther->get_Name (&name);
  if (name) {
     if (lstrcmp (bstrName, name) != 0)
       hr = ResultFromScode (S_FALSE);
     SysFreeString (name);
  }
  return hr;
}


HRESULT _IFUNC BOleFont::SetRatio (long cyLog, long cyHi)
{
  cyLogical  = cyLog;
  cyHimetric = cyHi;
  modified   = TRUE;
  return NOERROR;
}


HRESULT _IFUNC BOleFont::QueryTextMetrics (LPTEXTMETRICOLE lptm)
{
  HDC     hdc;
  HRESULT hr;
  BOOL    ret;
  HFONT   hF, SaveFont;

  hr = get_hFont (&hF);
  if (hr)
    return hr;

  hdc = GetDC (0);
  if (!hdc)
    return ResultFromScode (E_NOTIMPL);

  SaveFont = (HFONT) SelectObject (hdc, hF);
#if !defined(OLE2ANSI)
  ret = GetTextMetricsW (hdc, lptm);
#else
  ret = GetTextMetricsA (hdc, lptm);
#endif
  SelectObject (hdc, SaveFont);
  ReleaseDC (0, hdc);
  if (!ret)
    return ResultFromScode (E_NOTIMPL);
  return NOERROR;
}


HRESULT _IFUNC BOleFont::AddRefHfont (HFONT hfont)
{
  BOleFontObj *pF = FindFontObj (hfont);
  if (pF) {
    pF->AddRef ();
    return NOERROR;
  }
  else
    return ResultFromScode (CTL_E_PROPERTYNOTFOUND);
}


HRESULT _IFUNC BOleFont::ReleaseHfont (HFONT hfont)
{
  BOleFontObj *pF = FindFontObj (hfont);
  if (pF) {
    pF->Release ();
  return NOERROR;
  }
  else
    return ResultFromScode (CTL_E_PROPERTYNOTFOUND);
}


BOOL BOleFont::IsEqual (BOleFontObj FAR * pObj)
{
  if (pObj && ISEQUALFONTSIZE (height, pObj->height) &&
      (weight == pObj->weight) && (style == pObj->style) &&
      (charSet == pObj->charSet) &&
      (cyLogical == pObj->cyLogical) &&
      (cyHimetric == pObj->cyHimetric) &&
      ((pObj->bstrName) && (lstrcmp (bstrName, pObj->bstrName) == 0)))
    return TRUE;
  return FALSE;
#ifdef MANUAL_WAY
  BOOL res = TRUE;
  if (pObj != NULLP) {
    if ( !ISEQUALFONTSIZE (height, pObj->height))
      res = FALSE;
    if ( !(weight == pObj->weight))
      res = FALSE;
    if ( !(style == pObj->style))
      res = FALSE;
    if ( !(charSet == pObj->charSet))
      res = FALSE;
    BSTR  name = pObj->bstrName;
    if ( (!name && (strcmp (bstrName, name) != 0)))
      res = FALSE;
    if ( (!(pObj->bstrName) && (strcmp (bstrName, pObj->bstrName) != 0)))
      res = FALSE;
    return res;
  }
  return FALSE;
#endif
}


BOleFontObj *BOleFont::FindFontObj (HFONT hFont)
{
  BOleFontObj *pF = pFactory->GetFontCache ();

  while (pF) {
    if (pF->hFont == hFont)
      return pF;
    pF = pF->pNext;
  }
  return NULLP;
}


BOleFontObj *BOleFont::FindFontObj ()
{
  BOleFontObj *pF = pFactory->GetFontCache ();

  while (pF) {
    if (IsEqual (pF))
      return pF;
    pF = pF->pNext;
  }
  return NULLP;
}

// IConnectionPointContainer methods
//
HRESULT _IFUNC BOleFont::EnumConnectionPoints
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

HRESULT _IFUNC BOleFont::FindConnectionPoint (REFIID iid, LPCONNECTIONPOINT FAR* ppCP)
{
  // only one connection point for BOleFont (i.e. IPropertyNotifySink)
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
HRESULT BOleFont::PropChanged (DISPID dispid)
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


HRESULT BOleFont::PropRequestEdit (DISPID dispid)
{
  HRESULT        result;
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




// --- Font Cache implementation


BOleFontObj::BOleFontObj (BOleClassManager *pF, IBUnknownMain * pO) :
        BOleComponent (pF, pO),        weight (FW_NORMAL),
        charSet (DEFAULT_CHARSET),     style (fsBold),
        hFont (0),                     bstrName (NULL),
        pNext (NULL),                  pPrev (NULL),
        nRefs (0)

{
  SETFONTSIZE(height, 10);
  nRefs = 1;
}


BOleFontObj::~BOleFontObj ()
{
  SysFreeString (bstrName);
  if (hFont)
    DeleteObject (hFont);
  if (this == pFactory->GetFontCache ())
    pFactory->SetFontCache (NULL);
  else if (pPrev)
  pPrev->pNext = pNext;

}


HRESULT BOleFontObj::CreateHFont (BOleFont *pBF)
{
  HRESULT hr;
  LOGFONT LogFont;
  long    ht;

  if (hFont)
    DeleteObject (hFont);

  pBF->get_Size (&height);
  pBF->get_Weight (&weight);
  pBF->get_Charset (&charSet);
  style = pBF->getStyle ();
  cyHimetric = pBF->getCyHimetric ();
  cyLogical  = pBF->getCyLogical ();
  if (!bstrName || (lstrcmp (pBF->getName (), bstrName) != 0))
  {
    if (!bstrName)
      bstrName = SysAllocString (pBF->getName());
    else
      SysReAllocString (&bstrName, pBF->getName());
    lstrcpy (bstrName, pBF->getName());
  }

  ht = 0;
  HDC hdc = GetDC (0);
  if (hdc) {
  ht = height.s.Lo / 10000;
    long temp = GetDeviceCaps (hdc, LOGPIXELSY);
    ht = - MulDiv (ht, GetDeviceCaps (hdc, LOGPIXELSY), 72);  // translate to pixel
//    ht = - (cyHimetric * ht) / cyLogical;
    ReleaseDC (0, hdc);
  }
  LogFont.lfHeight      = ht;
  LogFont.lfWidth       = 0;  // have font mapper choose
  LogFont.lfEscapement  = 0;  // only straight fonts
  LogFont.lfOrientation = 0;  // no rotation
  LogFont.lfWeight      = weight;
  LogFont.lfItalic      = (style & fsItalic) != 0;
  LogFont.lfUnderline   = (style & fsUnderline) != 0;
  LogFont.lfStrikeOut   = (style & fsStrikeThrough) != 0;
  LogFont.lfCharSet     = charSet;
  LogFont.lfQuality     = DEFAULT_QUALITY;
  LogFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  LogFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  LogFont.lfPitchAndFamily = DEFAULT_PITCH;
#if !defined(UNICODE)    // UNICODE conversion needed
  WideCharToMultiByte (CP_ACP, 0, bstrName, lstrlen(bstrName),
                      LogFont.lfFaceName, 32, 0, 0);
#else
  lstrcpy (LogFont.lfFaceName, bstrName);
#endif
  hFont = CreateFontIndirect (&LogFont);
  if (hFont)
    GetObject (hFont, sizeof(LogFont), &LogFont);
  return NOERROR;
}


ULONG _IFUNC BOleFontObj::AddRefMain()
{
  return ++nRefs;
}

ULONG _IFUNC BOleFontObj::ReleaseMain()
{
  return --nRefs ? nRefs : (delete this, 0);
}




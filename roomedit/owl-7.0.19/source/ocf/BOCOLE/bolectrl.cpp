//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 control object.
//    BOleControl objects impersonate the OCX from the point of view of the
//    Bolero customer who's writing a OCX container.
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolecman.h"
#include "bolectrl.h"
#include "bolefont.h"
#include "boledoc.h"


#define NOT_IMPLEMENTED OLERET(E_NOTIMPL);

BOOL InterfaceDisconnect(LPUNKNOWN pObj, REFIID riid, LPDWORD pdwConn);
BOOL InterfaceConnect(LPUNKNOWN pObj, REFIID riid,
                      LPUNKNOWN pIUnknownSink, LPDWORD pdwConn);

//**************************************************************************
//
// BOleControl implementation -- These functions aren't part of an inherited
//                              interface, but are part of the private
//                              implementation of BOleControl
//
//**************************************************************************

HRESULT _IFUNC BOleControl::QueryInterfaceMain(REFIID iid, LPVOID FAR *ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  // Self
  //
  if (iid == IID_BOleControl) {
     (BOleControl *)*ppv = this;
     BOlePart::AddRef();
     return NOERROR;
  }

  // interfaces
  if (SUCCEEDED(hr = IOleControlSite_QueryInterface (this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IPropertyNotifySink_QueryInterface (this, iid, ppv))) {
  }
  else if (SUCCEEDED(hr = IBControl_QueryInterface(this, iid, ppv))) {
  }
  else if ((iid == IID_IDispatch) && (pIAmbients)){
    ((IDispatch *)*ppv) = pIAmbients;
    pIAmbients->AddRef ();
    return NOERROR;
  }
  else if ((pIEvents) && (iid == iidEvents)) {
    ((IDispatch *)*ppv) = pIEvents;
    pIEvents->AddRef ();
    return NOERROR;
  }
  // base classes
  else if (SUCCEEDED(hr = BOlePart::QueryInterfaceMain(iid, ppv))) {
  }
  return hr;
};


BOleControl::BOleControl (BOleClassManager *pF, IBUnknownMain * pO,
                           BOleDocument * pOD) :
  BOlePart (pF, pO, pOD),
  pIEvents (NULL),         pIAmbients (NULL),
  pCtrlDispatch (NULL),    pOleCtrl (NULL),
  pCSite (NULL),           pNextCtrl (NULL),
  pPrevCtrl (NULL),        iidEvents (IID_NULL),
  ConnEvents (0),          ConnProp (0)
{
  CtrlInfo.cb = sizeof(CONTROLINFO);
  CtrlInfo.cAccel = 0;
  CtrlInfo.hAccel = 0;
}

BOleControl::~BOleControl ()
{
  RemoveFromCtrlList ();
  if (ConnEvents)
    InterfaceDisconnect (pOleCtrl, iidEvents, &ConnEvents);
  if (ConnProp)
    InterfaceDisconnect (pOleCtrl, IID_IPropertyNotifySink, &ConnProp);

  if (pCtrlDispatch) {
    pCtrlDispatch->Release();
    pCtrlDispatch = NULL;
  }
  if (pIEvents) {
    pIEvents->Release();
    pIEvents = NULL;
  }
  if (pIAmbients) {
    pIAmbients->Release();
    pIAmbients = NULL;
  }
  if (pOleCtrl) {
    pOleCtrl->Release();
    pOleCtrl = NULL;
  }
}



HRESULT _IFUNC BOleControl::Init(IBSite * pIS,BOleInitInfo *pBI)
{
  HRESULT hr;

  hr = pIS->QueryInterface (IID_IBControlSite, &(VOID FAR*)pCSite);
  if (!pCSite)
     return hr;
  pCSite->Release ();

  hr = BOlePart::Init (pIS, pBI);
  if (FAILED (hr))
     return hr;

  hr = pOleObject->QueryInterface (IID_IOleControl, &(VOID FAR*)pOleCtrl);
  if (!pOleCtrl)
     return hr;
  AddToCtrlList ();

  /*
   * Call IOleControl::GetControlInfo to retrieve the keyboard
   * information for this control.  We have to reload this
   * information in IOleControlSite::OnControlInfoChanged.
   */
  pOleCtrl->GetControlInfo (&CtrlInfo);

  /*
   * Get the properties and methods IDispatch
   */
  pOleCtrl->QueryInterface (IID_IDispatch, &(VOID FAR*)pCtrlDispatch);


  /*
   * Connect our events IDispatch to the object.  The function
   * EventIIDFromObject retrieves the events IID for us.  If
   * InterfaceConnect fails, then we'll just do without events.  If
   * we do connect to the events, then we need to initialize event
   * information.
   */
  ObjectEventsIID (&iidEvents);

  /*
   * Connect our property notification sink to the object.
   * If "new" fails then InterfaceConnect will fail and we'll
   * just do without notifications.
   */
  IUnknown *pObj;
  QueryInterface (IID_IPropertyNotifySink, &(VOID FAR*)pObj);
  if (pObj)
  {
    InterfaceConnect (pOleCtrl, IID_IPropertyNotifySink, pObj, &ConnProp);
    pObj->Release ();
  }
  return hr;
}


HRESULT _IFUNC BOleControl::BOleCreate (BOleInitInfo *pBI, DWORD dwRenderOpt,
                                                      LPVOID FAR *ppv)
{
  if (pBI->Where == BOLE_NEW)
    return BOlePart::BOleCreate (pBI, dwRenderOpt, ppv);

  // else if (pBI->Where == BOLE_NEW_OCX)

  HRESULT     hr;
  FORMATETC   fmtetc;
  CLSID       clsidNew;
  IOleObject *poleobj = NULL;
  IUnknown   *punk;

  *ppv = NULL;

  // Check if this class auto-converts to another class
  hr = OleGetAutoConvert (*(LPIID)pBI->whereNew.cid, &clsidNew);
  if (FAILED (hr))
    clsidNew = *(LPIID)pBI->whereNew.cid;

  // Create handler or inproc server for class
  hr = CoCreateInstance (clsidNew, NULL, CLSCTX_INPROC, IID_IOleObject,
                                                      &(LPVOID)punk);
  if (FAILED (hr))
    goto error;

  *ppv = punk;
  DWORD dwMiscStatus;
  // Determine if IOleObject::SetClientSite should be called before
  // IPersistStorage::InitNew
  hr = punk->QueryInterface (IID_IOleObject, (LPVOID FAR*)&poleobj);
  if (FAILED (hr))
    goto error;
  hr = poleobj->GetMiscStatus (DVASPECT_CONTENT, &dwMiscStatus);
  if (FAILED (hr))
    goto error;

  // Set client site before InitNew if the control requires it.
  if (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
  {
    hr = poleobj->SetClientSite (this);
    if (FAILED (hr))
       goto error;
  }

 /*
 * BOleCreate do not use the IPersistStreamInit
 * interfaces. A control container that wants to use this interface
 * would either not cache a presentation or would cache the presentation
 * itself after obtaining it using IDataObject::GetData. It will not be
 * able to use the default handler to cache the presentation because
 * the default handler can only save/load presentations into/from storages.
 * If the controls are always shipped with the container, caching is not
 * required.
 */
  LPPERSISTSTORAGE pPS;
  // Write class into storage and call InitNew
  hr = punk->QueryInterface (IID_IPersistStorage, (LPVOID FAR*)&pPS);
  if (FAILED (hr))
    goto error;
  WriteClassStg (pStgFromInit, clsidNew);
  hr = pPS->InitNew (pStgFromInit);
  pPS->Release ();
  if (FAILED (hr))
    goto error;

  if (!(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
  {
    hr = poleobj->SetClientSite (this);
    if (FAILED (hr))
       goto error;
  }
  poleobj->Release ();
  poleobj = NULL;


  // No need to run the object or cache presentations if no
  // caches are requested.
  if ((dwRenderOpt == OLERENDER_NONE) || (dwRenderOpt == OLERENDER_ASIS))
    return NOERROR;

  // If the render option is OLERENDER_DRAW, pfmtetc can be NULL. If
  // pfmtetc is non-NULL, cfFormat & tymed must be 0.
  if (dwRenderOpt == OLERENDER_DRAW) {
    fmtetc.cfFormat = NULL;
    fmtetc.tymed = TYMED_NULL;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;
    fmtetc.dwAspect = DVASPECT_CONTENT;
  }

#if 0
  OleCreate will run the object at this point using OleRun. But OleRun
  calls IPersistStorage::InitNew before IOleObject::SetClientSite using
  the client site and storage pointers saved in the handler. For this
  reason, ControlCreate () doesn't support LocalServer controls.
  (LocalServer controls currently cannot be implemented because the new
   OLE control interfaces do not have marshalling code).

  // Run object
  hr = OleRun (punk);
  if (FAILED (hr))
     goto error;
#endif

  LPOLECACHE polecache;
  DWORD dwConnId;
  // Cache presentation
  hr = punk->QueryInterface (IID_IOleCache, (LPVOID FAR*)&polecache);
  if (FAILED (hr))
    return NOERROR;
    //goto error;
  hr =  polecache->Cache (&fmtetc, ADVF_PRIMEFIRST, &dwConnId);
  polecache->Release ();
  if (FAILED (hr))
    goto error;

  return NOERROR;

error:
  if (punk) punk->Release ();
  *ppv = NULL;
  if (poleobj) poleobj->Release ();
  return hr;

}


void BOleControl::AddToCtrlList ()
{
  // Insert "this" onto the list of control instances in this document.
  // Similar to part list on doc
  BOleControl *pOldFirstCtrl = pDoc->GetFirstControl ();
  pDoc->SetFirstControl (this);
  pNextCtrl = pOldFirstCtrl;
  if (pOldFirstCtrl)
    pOldFirstCtrl->pPrevCtrl = this;
}

void BOleControl::RemoveFromCtrlList ()
{
  // Remove "this" from the list of part instances in this document.
  //
  if (pPrevCtrl)
    pPrevCtrl->pNextCtrl = pNextCtrl;
  if (pNextCtrl)
    pNextCtrl->pPrevCtrl = pPrevCtrl;

  if (pDoc) {
    if (this == pDoc->GetFirstControl ())
      pDoc->SetFirstControl (pNextCtrl);
  }
  pNextCtrl = pPrevCtrl = NULLP;
}

// standard control methods
HRESULT _IFUNC BOleControl::Refresh ()
{
  return InvokeStdMethod (DISPID_REFRESH);
}


HRESULT _IFUNC BOleControl::DoClick ()
{
  return InvokeStdMethod (DISPID_DOCLICK);
}


HRESULT _IFUNC BOleControl::AboutBox ()
{
  return InvokeStdMethod (DISPID_ABOUTBOX);
}


HRESULT BOleControl::InvokeStdMethod (DISPID id)
{
  VARIANTARG   result;
  DISPPARAMS   args;
  unsigned int argError;

  VariantInit (&result);
  args.rgvarg = NULLP;
  args.cArgs = 0;
  args.cNamedArgs = 0;
  args.rgdispidNamedArgs = NULLP;
  return pCtrlDispatch->Invoke (id, IID_NULL,
      LOCALE_SYSTEM_DEFAULT,
      DISPATCH_METHOD, &args, &result, NULLP, &argError);
}


HRESULT _IFUNC BOleControl::GetEventIID (IID *iidEv)
{
  *iidEv = iidEvents;
  return NOERROR;
}


HRESULT _IFUNC BOleControl::GetEventTypeInfo (LPTYPEINFO *ppITypeInfo)
{
  HRESULT             hr;
  LPTYPEINFO          pITypeInfoAll;
  LPPROVIDECLASSINFO  pIProvideClassInfo;
  LPTYPEATTR          pTA;

  *ppITypeInfo = NULL;  //Use this to determine success
  if (!(pOleCtrl) || !(ppITypeInfo))
     return ResultFromScode (S_FALSE);

  /*
   * Get the object's IProvideClassInfo and call the GetClassInfo
   * method therein.  This will give us back the ITypeInfo for
   * the entire object.
   */

  hr = pOleCtrl->QueryInterface(IID_IProvideClassInfo,
                                   &(VOID FAR*)pIProvideClassInfo);
  if (FAILED (hr))
     return hr;

  hr = pIProvideClassInfo->GetClassInfo (&pITypeInfoAll);
  pIProvideClassInfo->Release ();
  if (FAILED (hr))
     return hr;


  /*
   * We have the object's overall ITypeInfo in pITypeInfoAll.
   * Now get the type attributes which will tell us the number of
   * individual interfaces in this type library.  We then loop
   * through the "implementation types" for all those interfaces
   * calling GetImplTypeFlags, looking for the default source.
   */
  if (SUCCEEDED (pITypeInfoAll->GetTypeAttr (&pTA))) {
    UINT        i;
    int         iFlags;

    for (i=0; i < pTA->cImplTypes; i++) {
      //Get the implementation type for this interface
      if (FAILED (pITypeInfoAll->GetImplTypeFlags (i, &iFlags)))
         continue;

      if ((iFlags & IMPLTYPEFLAG_FDEFAULT)
                           && (iFlags & IMPLTYPEFLAG_FSOURCE))
      {
        HREFTYPE    hRefType=NULL;

        /*
         * This is the interface we want.  Get a handle to
         * the type description from which we can then get
         * the ITypeInfo.
         */
        pITypeInfoAll->GetRefTypeOfImplType (i, &hRefType);
        hr = pITypeInfoAll->GetRefTypeInfo (hRefType, ppITypeInfo);
        break;
      }
    }
    pITypeInfoAll->ReleaseTypeAttr(pTA);
  }
  pITypeInfoAll->Release();
  return (*ppITypeInfo != NULL)? ResultFromScode (S_OK) : ResultFromScode (S_FALSE);
}


HRESULT _IFUNC BOleControl::GetCtrlDispatch (IDispatch FAR * FAR *ppDispatch)
{
  *ppDispatch = pCtrlDispatch;
  pCtrlDispatch->AddRef ();
  return S_OK;
}


/*
 * ObjectEventsIID
 *
 * Purpose:
 *  Determines the IID of the object's events interface so we
 *  can ask for the right IConnectionPoint for events.
 */

BOOL BOleControl::ObjectEventsIID (IID *piid)
{
  HRESULT     hr;
  LPTYPEINFO  pITypeInfo;
  LPTYPEATTR  pTA;

  *piid = IID_IDispatch;
  *piid = CLSID_NULL;
  if (GetEventTypeInfo (&pITypeInfo) != S_OK)
    return FALSE;

  hr = pITypeInfo->GetTypeAttr (&pTA);
  if (SUCCEEDED (hr)) {
    *piid = pTA->guid;
    pITypeInfo->ReleaseTypeAttr (pTA);
  }
  pITypeInfo->Release ();
  return SUCCEEDED (hr);
}



HRESULT _IFUNC BOleControl::SetEventDispatch (IDispatch *pNew)
{
  if (pIEvents) {
    if (ConnEvents)
      InterfaceDisconnect (pOleCtrl, iidEvents, &ConnEvents);
    pIEvents->Release ();
  }

  pIEvents = pNew;
  if (pIEvents) {
    pIEvents->AddRef ();
    if (pOleCtrl)
      InterfaceConnect (pOleCtrl, iidEvents, pIEvents, &ConnEvents);
  }
  return NOERROR;
}


HRESULT _IFUNC BOleControl::SetAmbientDispatch (IDispatch *pNew)
{
  if (pIAmbients)
    pIAmbients->Release ();

  pIAmbients = pNew;
  if (pIAmbients)
    pIAmbients->AddRef ();
  return NOERROR;
}



HRESULT _IFUNC BOleControl::AmbientChanged (DISPID dispid)
{
  if (pOleCtrl)
    return pOleCtrl->OnAmbientPropertyChange (dispid);
  else return NOERROR;
}

// function passed down by BOleControlSite
HRESULT _IFUNC BOleControl::GetControlInfo (LPCONTROLINFO pCI)
{
  *pCI = CtrlInfo;
  return S_OK;
}

HRESULT _IFUNC BOleControl::OnMnemonic (LPMSG pMsg)
{
  return ResultFromScode (E_NOTIMPL);
}


#if 0
HRESULT _IFUNC BOleControl::OnAmbientPropertyChange (DISPID dispid)
{
  return ResultFromScode (E_NOTIMPL);
}
#endif

HRESULT _IFUNC BOleControl::GetSizeMax (ULARGE_INTEGER FAR*)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControl::Save (LPSTREAM)
{
  return ResultFromScode (E_NOTIMPL);
}



HRESULT _IFUNC BOleControl::FreezeEvents (BOOL bFreeze)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControl::GetClassInfo (LPTYPEINFO FAR* ppTI)
{
  return ResultFromScode (E_NOTIMPL);
}

// IOleControlSite methods
HRESULT _IFUNC BOleControl::OnControlInfoChanged (void)
{
  if (pOleCtrl)
  pOleCtrl->GetControlInfo (&CtrlInfo);
  return NOERROR;
}


HRESULT _IFUNC BOleControl::LockInPlaceActive (BOOL fLock)
{
  NOT_IMPLEMENTED
}


HRESULT _IFUNC BOleControl::GetExtendedControl (LPDISPATCH FAR* ppDisp)
{
  NOT_IMPLEMENTED
}



HRESULT _IFUNC BOleControl::TransformCoords (POINTL FAR* lpptlHimetric,
                               POINTF FAR* lpptfContainer, DWORD flags)
{
  return pCSite->TransformCoords (lpptlHimetric,
                        (TPOINTF FAR*)lpptfContainer, flags);
}



HRESULT _IFUNC BOleControl::TranslateAccelerator (LPMSG lpMsg, DWORD )
{
  return pDoc->TranslateControlAccel (lpMsg);
}



HRESULT _IFUNC BOleControl::OnFocus (BOOL fGotFocus)
{
  HRESULT hr = NOERROR;

  pCSite->OnControlFocus (fGotFocus);
  if (fGotFocus && pOleObject)
    hr = DoVerb (OLEIVERB_UIACTIVATE);
  return hr;
}



HRESULT _IFUNC BOleControl::ShowPropertyFrame (void)
{
  NOT_IMPLEMENTED
}




// IPropertyNotifySink methods
HRESULT _IFUNC BOleControl::OnChanged (DISPID dispid)
{
  pCSite->OnPropertyChanged (dispid);
  return NOERROR;
}



HRESULT _IFUNC BOleControl::OnRequestEdit (DISPID dispid)
{
  return pCSite->OnPropertyRequestEdit (dispid);
  // OnPropertyRequestEdit to return S_OK or S_FALSE
}



/////////// utility routines


/*
 * InterfaceConnect
 *
 * Purpose:
 *  Connects some sink interface of a given IID to an object.
 *
 * Parameters:
 *  pObj            LPUNKNOWN of the object to which we connect
 *  riid            REFIID of the interface of the sink
 *  pIUnknownSink   LPUNKNOWN of the caller's sink interface that
 *                  is actually of the type matching riid
 *  pdwConn         LPDWORD in which to return the connection key
 *
 * Return Value:
 *  BOOL            TRUE if the function succeeded, FALSE otherwise.
 */

BOOL InterfaceConnect(LPUNKNOWN pObj, REFIID riid,
                      LPUNKNOWN pIUnknownSink, LPDWORD pdwConn)
{
  HRESULT                     hr;
  LPCONNECTIONPOINTCONTAINER  pCPC;
  LPCONNECTIONPOINT           pCP;

  if (!pObj || !pIUnknownSink || !pdwConn)
    return FALSE;

  if (FAILED (pObj->QueryInterface(IID_IConnectionPointContainer,
                     &(VOID FAR*)pCPC)))
    return FALSE;

  hr = pCPC->FindConnectionPoint(riid, &pCP);
  if (SUCCEEDED (hr)) {
    hr = pCP->Advise (pIUnknownSink, pdwConn);
    pCP->Release();
  }
  pCPC->Release();
  return SUCCEEDED(hr);
}



/*
 * InterfaceDisconnect
 *
 * Purpose:
 *  Disconnects a prior connection to an object.
 *
 * Parameters:
 *  pObj            LPUNKNOWN of the object from which to disconnect
 *  riid            REFIID of the interface of the sink
 *  pdwConn         LPDWORD containing the key returned by
 *                  InterfaceConnect.  This function will zero the
 *                  key on diconnect.
 *
 * Return Value:
 *  BOOL            TRUE if the function succeeded, FALSE otherwise.
 */

BOOL InterfaceDisconnect(LPUNKNOWN pObj, REFIID riid, LPDWORD pdwConn)
{
  HRESULT                     hr;
  LPCONNECTIONPOINTCONTAINER  pCPC;
  LPCONNECTIONPOINT           pCP;

  if (!pObj || !pdwConn)
    return FALSE;

  if (*pdwConn == 0)
    return FALSE;

  if (FAILED (pObj->QueryInterface (IID_IConnectionPointContainer,
                     &(VOID FAR*)pCPC)))
    return FALSE;

  hr = pCPC->FindConnectionPoint (riid, &pCP);
  if (SUCCEEDED (hr)) {
    hr = pCP->Unadvise (*pdwConn);
    if (SUCCEEDED (hr))
      *pdwConn = 0L;
    pCP->Release ();
  }
  pCPC->Release();
  return SUCCEEDED (hr);
}



// BOleTranslateColor - poor man's replacement function for OleTranslateColor
//     If we get an official license for OCX'es and the associated dll's,
//     then we can remove this and use the real thing
//
//     The only thing this appears to be missing is error checking if a
//     Palette index is specified and is out of range for the spec'ed palette
extern "C"

HRESULT PASCAL FAR _export _loadds BOleTranslateColor (OLE_COLOR color,
  HPALETTE hpal, COLORREF *pcolorref)
{
  long flag = 0xff000000 & color;
  if (flag == 0x80000000)
    *pcolorref = GetSysColor ((int) (0xff & color));
  else if ((flag == 0) && (hpal))
    *pcolorref = 0x02000000 | color;
  else
    *pcolorref = color;
  return NOERROR;
}

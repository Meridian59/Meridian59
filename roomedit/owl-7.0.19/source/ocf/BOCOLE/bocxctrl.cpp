//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 Control
//
//  BOleControlSite objects impersonate the client application (Container)
//    from the point of view of the Bolero customer who's
//    writing a control (IBPart and IBControl)
//    BOleControlSite aggregate BOleSite or BOleInProcServer and provide
//    implementation for control specific interfaces.
//    BOleControlSite uses IBControl to talk to the control whereas
//    the aggregated BOleInProcServer (BOleSite) uses IBpart.
//    In consequence a control must implement both IBPart and IBControl.
//----------------------------------------------------------------------------
#ifndef _BOCXCTRL_H
#include <bocxctrl.h>
#endif

#ifndef _BCONNPNT_H
#include "bconnpnt.h"
#endif

#ifndef BOLECMAN_H
#include <bolecman.h>
#endif

BOleControlSite::BOleControlSite (BOleClassManager *pClassManager,
                              IBUnknownMain *pOuter):
                          BOleComponent (pClassManager, pOuter),
                          // initialize data members
                          pObjInner(NULL), pEventList (NULL),
                          pControlSite (NULL), pControl (NULL),
                          fRegConnPoint (false), pDataDirty (NULL)
{

}

BOleControlSite::~BOleControlSite()
{
  // release aggregate helper
  if (pObjInner)
    pObjInner->ReleaseMain();

  // release EventHandler
  // Even if EventHandler is not an interface but a real object
  // we call Realese() on it. EventHandler implements the enumeration
  // of ConnectionPoints and so has got its own lifetime.
  // Anyway we do not use that iterface enumeration to talk with the list
  // but we go through standard C++
  if (pEventList)
    pEventList->Release();

  //** release interfaces **
  // Container interfaces
  if (pControlSite)
    pControlSite->Release();

  // Control interface
  //if (pControl)
  //  pControl->Release();
}

//**** IUnknown methods ****
HRESULT _IFUNC BOleControlSite::QueryInterfaceMain(REFIID iid,
                                      LPVOID FAR* ppVObj)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppVObj = NULL;

  // answer for self (who can ask that???)
  //if (IID_BOleControlSite == iid) {
  //  (BOleControlSite *)*ppVObj = this;
  //  AddRef();
  //  return NOERROR;
  //}

  // implemented interfaces
  if (SUCCEEDED(hr = IBControlSite_QueryInterface(this, iid, ppVObj))) {
  }
  else if (SUCCEEDED(hr = IProvideClassInfo_QueryInterface(this, iid, ppVObj))) {
  }
  else if (SUCCEEDED(hr = IOleControl_QueryInterface(this, iid, ppVObj))) {
  }
  else if (SUCCEEDED(hr = IPersistStreamInit_QueryInterface(this, iid, ppVObj))) {
  }
  else if (SUCCEEDED(hr = IConnectionPointContainer_QueryInterface(this, iid, ppVObj))) {
  }
  else if (SUCCEEDED(hr = IBEventsHandler_QueryInterface(this, iid, ppVObj))) {
  }

  // base classes
  else if (SUCCEEDED(hr = BOleComponent::QueryInterfaceMain(iid, ppVObj))) {
  }

  // ask aggregate if he knows iid
  else if (pObjInner) {
    hr = pObjInner->QueryInterfaceMain (iid, ppVObj);
  }

  return hr;
}

//**** IBControlSite methods ****
HRESULT _IFUNC BOleControlSite::Init (UINT nHelperChoice, IBControl* pCnt, UINT CPCSize)
{
  //
  // create aggregated object as required
  //
  if ( nHelperChoice != BOLE_USE_LIGHT ) { // use light control (no linking and embedding)
    PIUnknown pInner = AsPIUnknown (pObjInner);
    if (nHelperChoice == BOLE_USE_EXE )
      pFactory->ComponentCreate (&pInner, AsPIUnknown(pObjOuter), cidBOleSite); // aggregate BOleSite
    else
      pFactory->ComponentCreate (&pInner, AsPIUnknown(pObjOuter), cidBOleInProcSite); // aggregate BOleInProcServer
    pObjInner = AsPIUnknownMain (pInner);
  }

  // keep IBControl interface
  pControl = pCnt;
  //pControl->AddRef();

  if (SUCCEEDED (pObjInner->QueryInterfaceMain (IID_IBDataState, (LPVOID FAR*)&pDataDirty)))
    pDataDirty->Release ();

  // EventHandler deal with ConnecctionPoints
  pEventList = new BEventList (CPCSize);
  if (pEventList) pEventList->AddRef();

  return NOERROR;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// Container impementation
//   The following 4 functions are implemented on the container side and
//   do not need any implememtation here
//
//
HRESULT _IFUNC BOleControlSite::OnPropertyChanged (DISPID)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::OnPropertyRequestEdit (DISPID)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::OnControlFocus (BOOL)
{
  return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::TransformCoords (POINTL FAR* lpptlHimetric,
                            TPOINTF FAR* lpptfContainer, DWORD flags)
{
  return ResultFromScode (E_NOTIMPL);
}

//\\// IOleControl methods \\//\\
HRESULT _IFUNC BOleControlSite::GetControlInfo (LPCONTROLINFO pCI)
{
  if (pControl)
    return pControl->GetControlInfo (pCI);
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::OnMnemonic (LPMSG pMsg)
{
  if (pControl)
    return pControl->OnMnemonic (pMsg);
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::OnAmbientPropertyChange (DISPID PropertyID)
{
  if (pControl)
    return pControl->AmbientChanged (PropertyID);
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::FreezeEvents (BOOL bFreeze)
{
  if (pControl)  {
    // here code to freeze events internally (no call to the user)
    pEventList->FreezeAll (bFreeze);

    if (fRegConnPoint)  {
      //\\// user must freeze events on his own
      //\\// registered connection point
      return pControl->FreezeEvents (bFreeze);
    }
    else {
      return NOERROR;
    }
  }
  else
    return ResultFromScode (E_NOTIMPL);
}

//**** IProvideClassInfo methods ****
HRESULT _IFUNC BOleControlSite::GetClassInfo (LPTYPEINFO FAR* ppTypeInfo)
{
  if (pControl)
    return pControl->GetClassInfo (ppTypeInfo);
  else
    return ResultFromScode (E_NOTIMPL);
}

//**** IPersistStreamInit methods ****
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//
// We need to duplicate IsDirty flag here since we need to acces it.
// What will happen is that we have the dirty state in both BOleSite and
// BOleControlSite and we will use one or the other depending if we are
// using IPersistStreamInit or IPersistStorage. I don't like this but the
// only other chance is to create a specific interface to set isDirty flag
// into the other side.
// By the way, there is another solution, too: derive from BOleInProcServer
// and forget about exe controls!!
//

HRESULT _IFUNC BOleControlSite::GetClassID (LPCLSID pCId)
{
  if (pDataDirty)
    return pDataDirty->GetClassID (pCId);
  else
    return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleControlSite::IsDirty ()
{
  if (pDataDirty)
    return pDataDirty->IsDirty ();
  else
    return ResultFromScode (E_FAIL);
}

HRESULT _IFUNC BOleControlSite::Load (LPSTREAM pStream)
{
  HRESULT hr;
  BOleInitInfo bi;

  // use this stream to embed
  bi.Where = BOLE_STREAM;
  bi.How = BOLE_EMBED;

  // zero unused data
  bi.pContainer = NULL;
  bi.hIcon = NULL;
  bi.pStorage = NULL;

  // load stream and pass down
  bi.whereStream.pStream = pStream;
  hr = pControl->Init (NULL, &bi);
  pDataDirty->SetDirty (false);

  return hr;
}

HRESULT _IFUNC BOleControlSite::Save (LPSTREAM pStream, BOOL fCleanIsDirty)
{
  HRESULT hr;
  if (pControl)  {
    if (SUCCEEDED (hr = pControl->Save (pStream)))
      if (fCleanIsDirty)
        pDataDirty->SetDirty (false);

    return hr;
  }
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::GetSizeMax (ULARGE_INTEGER FAR* maxSize)
{
  if (pControl)
    return pControl->GetSizeMax (maxSize);
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::InitNew ()
{
  BOleInitInfo bi;

  // tell control to initialize any state it needs to
  bi.Where = BOLE_NEW_OCX;

  // zero unused data
  bi.How = NULL;
  bi.pContainer = NULL;
  bi.hIcon = NULL;
  bi.pStorage = NULL;

  pDataDirty->SetDirty (false);
  // call init
  return pControl->Init (NULL, &bi);
}

//**** IBEventsHandler methods ****
HRESULT _IFUNC BOleControlSite::RegisterEventsSet (REFIID iid, UINT nSize)
{
  if (pEventList)
    return pEventList->AddEventsSet (iid, AsPIUnknown(this), nSize);
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::RegisterConnectionPoint (LPCONNECTIONPOINT pCP)
{
  if (pEventList)  {
    fRegConnPoint = true;
    return pEventList->Add (pCP);
  }
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::GetSinkListForIID (REFIID iid, IBSinkList** ppSinkList)
{
  HRESULT hr;
  LPCONNECTIONPOINT tempCP;
  if (SUCCEEDED (hr = FindConnectionPoint (iid, (LPCONNECTIONPOINT FAR*)&tempCP))) { // find connectionpoint
    IBEventClass* pEvClss; // ask for our implementation in order to get the list
    if (SUCCEEDED(hr = tempCP->QueryInterface (IID_IBEventClass, (LPVOID FAR*)&pEvClss))) {
      if (ppSinkList)
        hr = pEvClss->GetSinkList (ppSinkList); // get list
      else
        hr = ResultFromScode (E_INVALIDARG);
      pEvClss->Release(); // release event class (QueryInterface)
    }
    tempCP->Release(); // release connection point (FindConnectionPoint)
  }

  return hr;
}

//**** IConnectionPointContainer methods ****
HRESULT _IFUNC BOleControlSite::EnumConnectionPoints (LPENUMCONNECTIONPOINTS FAR* ppEnum)
{
  // if at least one connection point is here, return the collection!
  if (pEventList && (*pEventList)[0]) {
    *ppEnum = (LPENUMCONNECTIONPOINTS)pEventList;
    (*ppEnum)->AddRef();
    return NOERROR;
  }
  else
    return ResultFromScode (E_NOTIMPL);
}

HRESULT _IFUNC BOleControlSite::FindConnectionPoint (REFIID iid, LPCONNECTIONPOINT FAR* ppCP)
{
  HRESULT hr = ResultFromScode (E_NOINTERFACE);
  IID  TempIID;

  for (int i=0; (*pEventList)[i]; i++) {
    (*pEventList)[i]->GetConnectionInterface(&TempIID); // get iid
    if (TempIID == iid) {

      // according to specs if the same iid is supported more than once report an error
      if (hr == NOERROR) {
        *ppCP = NULL;
        hr = ResultFromScode (S_FALSE);
        break;
      }
      else { // GOT IT!
        *ppCP = (*pEventList)[i];
        hr = NOERROR; // it's ok but keep searching (multiple exposed iid)
      }
    }
  } // end for

  if (hr == NOERROR)
    (*ppCP)->AddRef();

  return hr;
}

//************************************************************************
//  class BEventList
//
//************************************************************************
BEventList::BEventList (UINT nElemToAlloc):
                        nCurrPos(0),
                        nSize(nElemToAlloc),
                        cRef (0)
{
  pConnectionPoints = (IConnectionPoint **) new char[sizeof(IConnectionPoint*)*nSize];

  if (pConnectionPoints)
    memset (pConnectionPoints, 0, sizeof(IConnectionPoint *) * nSize);
}

BEventList::~BEventList ()
{
  // release stored connection points
  for (int i = 0; i < nSize && pConnectionPoints[i]; i++)
    pConnectionPoints[i]->Release();

  delete [] pConnectionPoints;
}

//**** IUnknown methods ****
ULONG _IFUNC BEventList::AddRef ()
{
  return ++cRef;
}

ULONG _IFUNC BEventList::Release ()
{
  DWORD tempRef = --cRef;

  if (!cRef)
    delete this;

  return tempRef;
}

HRESULT _IFUNC BEventList::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = ResultFromScode(E_NOINTERFACE);
  *ppv = NULL;

  if (SUCCEEDED (hr = IEnumConnectionPoints_QueryInterface (this, iid, ppv))) {
  }
  return hr;
}

//**** IEnumConnectionPoints methods ****
HRESULT _IFUNC BEventList::Next (ULONG cConnections, LPCONNECTIONPOINT FAR* rgpcn,
                        ULONG FAR* lpcFetched)
{
  HRESULT hr = S_OK;

  // check rgpcn (for fun);
  // lpcFetched can only be NULL if one element is request (cConnections = 1)
  if (rgpcn == NULL || (lpcFetched == NULL && cConnections != 1))
    hr = ResultFromScode (E_INVALIDARG);
  else {
    int nElems;

    // copy elements
    for (nElems = 0; cConnections && nElems < (nSize - nCurrPos) &&
                                    pConnectionPoints[nElems]; nElems++) {
      rgpcn[nElems] = pConnectionPoints[nCurrPos++];
      rgpcn[nElems]->AddRef();
      cConnections--;
    }

    if (lpcFetched != NULL) *lpcFetched = nElems;
    if (cConnections) hr = ResultFromScode (S_FALSE);
  }

  return hr;
}

HRESULT _IFUNC BEventList::Skip (ULONG cConnections)
{
  HRESULT hr = S_OK;
  int nSkip;

  // find out how many elements to skip
  for ( nSkip = 0; cConnections-- && nSkip < (nSize - nCurrPos) &&
                                      pConnectionPoints[nSkip]; nSkip++);

  if (cConnections) hr = ResultFromScode (S_FALSE);

  nCurrPos += nSkip;  // update currPos

  return hr;
}

HRESULT _IFUNC BEventList::Reset ()
{
  nCurrPos = 0;
  return S_OK;
}

HRESULT _IFUNC BEventList::Clone (LPENUMCONNECTIONPOINTS FAR* ppEnum)
{
  HRESULT hr = S_OK;

  // check ppEnum
  if (ppEnum == NULL)  hr = ResultFromScode (E_INVALIDARG);

  *ppEnum = (LPENUMCONNECTIONPOINTS) new BEventList (this);

  // check ppEnum for out of memory
  if (ppEnum == NULL)  hr = ResultFromScode (E_OUTOFMEMORY);

  (*ppEnum)->AddRef(); // user must release it

  return hr;
}

HRESULT BEventList::AddEventsSet (REFIID iid, LPUNKNOWN pCPC, UINT nSize)
{
  HRESULT hr = NOERROR;
  BEventHandler* newEventSet;
  newEventSet = new BEventHandler (iid, pCPC, nSize);

  if (newEventSet) {
    int nInsPos;

    for (nInsPos = 0; nInsPos < nSize && pConnectionPoints[nInsPos]; nInsPos++); // get insert position

    if (nInsPos < nSize || Expand()) {
      IConnectionPoint *pCP;
      newEventSet->QueryInterface (IID_IConnectionPoint, &(LPVOID)pCP);
      pConnectionPoints[nInsPos] = pCP;
    }
    else
      hr = ResultFromScode (E_OUTOFMEMORY);
  }
  else
    hr = ResultFromScode (E_OUTOFMEMORY);

  return hr;
}

HRESULT BEventList::Add (LPCONNECTIONPOINT pCP)
{
  HRESULT hr = NOERROR;
  int nInsPos;

  for (nInsPos = 0; nInsPos < nSize && pConnectionPoints[nInsPos]; nInsPos++); // get insert position

  if (nInsPos < nSize || Expand()) {
    pConnectionPoints[nInsPos] = pCP;
    pCP->AddRef();
  }
  else
    hr = ResultFromScode (E_OUTOFMEMORY);

  return hr;
}

void BEventList::FreezeAll (bool fFreeze)
{

  for (int nInsPos = 0; nInsPos < nSize && pConnectionPoints[nInsPos]; nInsPos++)  {
    IBEventClass* eventCls;
    if (SUCCEEDED (pConnectionPoints[nInsPos]->QueryInterface (IID_IBEventClass, (LPVOID FAR*)&eventCls)))  {
      eventCls->FreezeEvents (fFreeze);
      eventCls->Release ();
    }
  }

  return;
}

BEventList::BEventList (BEventList* copyList)
{
  // copy constructor for ::Clone method
  nCurrPos = copyList->nCurrPos;
  nSize = copyList->nSize;
  pConnectionPoints = (IConnectionPoint **) new
                                  char[sizeof (IConnectionPoint*) * nSize];
  for (int i = 0; i < nSize; i++) {
    pConnectionPoints[i] = (*copyList)[i];
    if (pConnectionPoints[i]) pConnectionPoints[i]->AddRef();
  }
}

int BEventList::Expand ()
{
  int newSize = nSize + nSize/2; // new dimension
  // allocate
  IConnectionPoint ** pNewList = (IConnectionPoint **) new char[sizeof(IConnectionPoint*)*newSize];
  if (pNewList) {
    // copy existing values
    memcpy (pNewList, pConnectionPoints, sizeof(IConnectionPoint*)*nSize);
    delete [] pConnectionPoints; // delete old array
    pConnectionPoints = pNewList;
    nSize = newSize;
    return 1;
  }
  else
    return 0;
}




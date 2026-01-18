//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Bolero helper for Connection Points
//
//    BEventClass is the base implementation for Connection Point.
//    BEventClass deals with sinks collection and with Advise and
//    Unadvise member functions.
//    BEventClass keeps also a collection of void pointers which are
//    those resulted from QueryInterface on sinks. Control implementors
//    can ask for this list.
//----------------------------------------------------------------------------
#ifndef _BCONNPNT_H
#include "bconnpnt.h"
#endif


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
// //\\//\\//\\//\\//\\//\\//\\//\\\  \\
// // //\\//\\//\\//\\//\\//\\\\  \\  \\
// // //  BEventHandler class \\  \\  \\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//      BEventHandler::BEventHandler costructor
//
//      Parameters:
//        REFIID        interface IID for the specific Connection Point,
//                      used by QueryInterface on Advise()
//        LPUNKNOWN     pointer back to the ConnectionPointContainer.
//                      IUnknown is used here becouse of possible aggregation
//        UINT          default size for the two arrays of sinks
//
BEventHandler::BEventHandler (REFIID iid, LPUNKNOWN pCPC, UINT size) :
                                EventIID (iid),   cRef (0)
{
  // allocate sink list
  pSinkList = new BSinkList (size);
  if (pSinkList) {
    pSinkList->AddRef();
    // save ConnectionPointContainer
    // don't AddRef since this guy owns us
    pBackToCPC = pCPC;
  }
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::~BEventHandler destructor
//
BEventHandler::~BEventHandler()
{
  if (pSinkList)
    pSinkList->Release();

  //if (pBackToCPC)
  //  pBackToCPC->Release();
}

//\\// IUnknown methods \\//\\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::AddRef
//
//      Purpose:
//        Increment the reference count of "this"
//
//      Return Value:
//        ULONG         new RefCount value
//
ULONG _IFUNC BEventHandler::AddRef ()
{
  return ++cRef;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::Release
//
//      Purpose:
//        Decrement the reference count of "this". Delete the object if
//        RefCount is 0
//
//      Return Value:
//        ULONG         new RefCount value
//
ULONG _IFUNC BEventHandler::Release ()
{
  DWORD tempRef = --cRef;

  if (!cRef)
    delete this;

  return tempRef;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::QueryInterface
//
//      Purpose:
//        Return all visible interfaces from this object
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = NOERROR;
  *ppv = NULL;

  // interfaces
  if (SUCCEEDED(hr = IBEventClass_QueryInterface (this, iid, ppv))) {
  }
  else if (SUCCEEDED (hr = IConnectionPoint_QueryInterface (this, iid, ppv))) {
  }
  else if (SUCCEEDED (hr = pSinkList->QueryInterface (iid, ppv))) {
  }
  else if (iid == IID_IUnknown) {
     (BEventHandler *)*ppv = this;
     AddRef();
     return NOERROR;
  }

  return hr;
}

//\\// IBEventClass methods \\//\\


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::GetSinkList
//
//      Purpose:
//        Return the list of sinks which contains the cached interface
//        for event firing. Called from the main Control object to
//        satisfy a request on GetSinkListForIID()
//
//      Parameters:
//        IBSinkList**      a pointer where to return the list
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::GetSinkList (IBSinkList** ppSinkList)
{
  HRESULT hr = NOERROR;

  if (ppSinkList) {
    *ppSinkList = pSinkList;
    pSinkList->AddRef(); //\\//\\
  }
  else
    hr = ResultFromScode (E_INVALIDARG);

  return hr;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
// BEventHandler::FreezeEvents
//
//  Purpose:
//    Called in response to FreezeEvents (IOleControl) this function sets
//    a flag so that NextSink or GetSinkAt will fail and no events can
//     be fired
//
HRESULT _IFUNC BEventHandler::FreezeEvents (bool fFreeze)
{
  if (pSinkList)  {
    pSinkList->SetFreeze (fFreeze);
    return NOERROR;
  }
  else
    return ResultFromScode (E_UNEXPECTED);
}


//\\// IConnectionPoint methods \\//\\


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::GetConnectionInterface
//
//      Purpose:
//        Answer what IID is supported from this specific IConnectionPoint
//
//      Parameters:
//        IID FAR*      point to where the IID is returned
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::GetConnectionInterface (IID FAR* iid)
{
  if (iid) {
    *iid = EventIID;
    return NOERROR;
  }
  else
    return ResultFromScode (E_INVALIDARG);
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::GetConnectionPointContainer
//
//      Purpose:
//        Here we have a pointer back to the ConnectionPointContainer, in case
//        user does not want to keep a couple of pointer but want to navigate
//        through interfaces
//
//      Parameters:
//        LPCONNECTIONPOINTCONTAINER FAR*   point to where CPC is stored (passed back)
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::GetConnectionPointContainer (LPCONNECTIONPOINTCONTAINER FAR* ppCPC)
{
  if (ppCPC)
    // this is because of aggregation! The object which is aggregating us could
    // have a different connection point container
    return pBackToCPC->QueryInterface (IID_IConnectionPointContainer, (LPVOID FAR*)ppCPC);
  else
    return ResultFromScode (E_INVALIDARG);
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::Advise
//
//      Purpose:
//        This is the main function for ConnectionPoint. With this function
//        a client of this control can connect event handler (sinks)
//
//      Parameters:
//        LPUNKNOWN           the sink passed in. We QueryInterface this
//                  object to see if it really implemente the interface
//                  we want to talk with. If so we cache that interface
//                  pointer
//        DWORD FAR*          user refers to this specific location in the ConnectionPoint
//                  with a DWORD we return here. This "cookie" will be used
//                  later to disconnect the sink
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::Advise (LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie)
{
  HRESULT hr = NOERROR;

  if (pUnkSink && pdwCookie) { // check if values passed in are valid
    int nInsPos;
    for (nInsPos=0; nInsPos < pSinkList->Size() && (*pSinkList)[nInsPos]; nInsPos++); // get insert position

    *pdwCookie = 0;

    if (nInsPos < pSinkList->Size() || pSinkList->Expand()) {
      LPUNKNOWN pObj; // passed to QueryInterface

      if (SUCCEEDED(hr = pUnkSink->QueryInterface(EventIID, (LPVOID FAR*)&pObj))) {
        pSinkList->SetSinkAt (pUnkSink, pObj, nInsPos); // pass interfaces pointer to the list object
        *pdwCookie = nInsPos + 1; // cookie will be position into the array + 1 (0 is error)
      }
    }
    else
      hr = ResultFromScode(E_OUTOFMEMORY);
  }
  else
    hr = ResultFromScode(E_INVALIDARG);

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::Unadvise
//
//      Purpose:
//        Disconnect a previous connected (Advise) sink
//
//      Parameters:
//        DWORD         this is the cookie value to get the sink (sort of sink ID)
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::Unadvise (DWORD dwCookie)
{
  HRESULT hr = NOERROR;

  if (--dwCookie > pSinkList->Size() || !(*pSinkList)[dwCookie])
    hr = ResultFromScode (E_INVALIDARG);
  else
    pSinkList->DeleteSinkAt (dwCookie); // inform the list to release interfaces

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::EnumConnections
//
//      Purpose:
//        return an Enumeration of connections
//
//      Parameters:
//        LPENUMCONNECTIONS FAR*    point to where the list of connection is stored
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::EnumConnections (LPENUMCONNECTIONS FAR* ppEnum)
{
  *ppEnum = (LPENUMCONNECTIONS)pSinkList;
  (*ppEnum)->AddRef(); //\\//\\
  return NOERROR;
}

//\\//\\//\\//\\//\\//\\//\\//\\\\
// //\\//\\//\\//\\//\\//\\//\\ \\
// // //\\//\\//\\//\\//\\\\ \\ \\
// // //  BSinkList class \\ \\ \\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//
//      BSinkList::BSinkList constructor
//
//      Parameters:
//        UINT        default size for arrays in this object
//
BEventHandler::BSinkList::BSinkList (UINT size):
                        nCurrPos (0),
                        nSize (size),
                        cRef (0),
                        fFreeze (false)
{
  pUnkList = (LPUNKNOWN FAR*) new char[sizeof(LPUNKNOWN) * nSize];
  pDirectList = (LPVOID FAR*) new char[sizeof(LPVOID) * nSize];

  memset (pUnkList, 0, sizeof(LPUNKNOWN) * nSize);
  memset (pDirectList, 0, sizeof(LPVOID) * nSize);
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::~BSinkList destructor
//
BEventHandler::BSinkList::~BSinkList()
{
  for (int i = 0; i < nSize; i++) {
    if (pUnkList[i]) {
      pUnkList[i]->Release();
      ((IUnknown*)(pDirectList[i]))->Release();
    }
  }

  delete [] pUnkList;
  delete [] pDirectList;
}

//\\// IUnknown \\//\\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::AddRef
//
//      Purpose:
//        Increment the reference count of "this"
//
//      Return Value:
//        ULONG         new RefCount value
//
ULONG _IFUNC BEventHandler::BSinkList::AddRef ()
{
  return ++cRef;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::Release
//
//      Purpose:
//        Decrement the reference count of "this". Delete the object if
//        RefCount is 0
//
//      Return Value:
//        ULONG         new RefCount value
//
ULONG _IFUNC BEventHandler::BSinkList::Release ()
{
  DWORD tempRef = --cRef;

  if (!cRef)
    delete this;

  return tempRef;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BEventHandler::QueryInterface
//
//      Purpose:
//        Return all visible interfaces from this object
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
  HRESULT hr = NOERROR;
  *ppv = NULL;

  if (SUCCEEDED(hr = IBSinkList_QueryInterface (this, iid, ppv))) {
  }
  else if (SUCCEEDED (hr = IEnumConnections_QueryInterface (this, iid, ppv))) {
  }
  else if (iid == IID_IUnknown) {
     (BSinkList *)*ppv = this;
     AddRef();
     return NOERROR;
  }
  return hr;
}

//\\// IBSinkList methods \\//\\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::NextSink
//
//      Purpose:
//        Answer the next sink object. Gives you the illusion to work
//        on a linked list.
//
//      Parameters:
//        LPVOID FAR*           point to where the next sink will be returned
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::NextSink (LPVOID FAR* ppObj)
{
  HRESULT hr = ResultFromScode (E_FAIL);

  if (!fFreeze)  {
    if (ppObj) {
      // loop until you get an element
      *ppObj = NULL;
      while (nCurrPos < nSize) {
        if (pDirectList[nCurrPos]) {
          ((LPUNKNOWN)(pDirectList[nCurrPos]))->AddRef(); //\\//\\
          *ppObj = pDirectList[nCurrPos];
          hr = S_OK;
          nCurrPos++;
          break;
        }
        nCurrPos++;
      }
    }
    else
      hr = ResultFromScode (E_INVALIDARG);
  }

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::GetSinkAt
//
//      Purpose:
//        Answer the sink object at a specific position.
//
//      Parameters:
//        int               psition into the array
//        LPVOID FAR*           point to where the next sink will be returned
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::GetSinkAt (int nPos, LPVOID FAR* ppObj)
{
  HRESULT hr = ResultFromScode (E_FAIL);

  if (!fFreeze)  {
    if (ppObj) {
      *ppObj = NULL;
      if (nPos < nSize && pDirectList[nPos]) {
        ((LPUNKNOWN)(pDirectList[nPos]))->AddRef(); //\\//\\
        *ppObj = pDirectList[nPos];
        hr = S_OK;
      }
    }
  }

  return hr;
}

//\\// IEnumConnections methods \\//\\

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::Next
//
//      Purpose:
//        return as many elements as required by the first parameter
//
//      Parameters:
//        ULONG           number of elements required
//        LPCONNECTDATA       array of pointers where to store those elements
//        ULONG FAR*            number of element really copied
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::Next (ULONG cConnection,
                            LPCONNECTDATA rgcd,
                            ULONG FAR* lpcFetched)
{
  HRESULT hr = S_OK;

  // check rgpcd (for fun);
  // lpcFetched can only be NULL if one element is request (cConnections = 1)
  if (rgcd == NULL || (lpcFetched == NULL && cConnection != 1))
    hr = ResultFromScode (E_INVALIDARG);
  else {
    int nNumEl = 0;

    // copy elements
    while (cConnection && nCurrPos < nSize) {
      if (pUnkList[nCurrPos]) { // see if a connection point is there
        pUnkList[nCurrPos]->AddRef(); //\\//\\
        rgcd[nNumEl].pUnk = pUnkList[nCurrPos]; // copy it
        rgcd[nNumEl].dwCookie = nCurrPos; // copy it

        cConnection--; nNumEl++;
      }
      nCurrPos++;
    }

    // save number of elements copied
    if (lpcFetched != NULL) *lpcFetched = nNumEl;
    // if copied less elements than required return FALSE
    if (cConnection) hr = ResultFromScode (S_FALSE);
  }

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::Skip
//
//      Purpose:
//        skip a given number of elements
//
//      Parameters:
//        ULONG       number of elements to skip
//
//      Return Value:
//        HRESULT     S_FLASE if we could not skip so many elements
//                S_OK otherwise
//
HRESULT _IFUNC BEventHandler::BSinkList::Skip (ULONG cConnections)
{
  HRESULT hr = S_OK;

  // find out how many elements to skip
  for ( ; cConnections && nCurrPos < nSize; nCurrPos++)
    if (pUnkList[nCurrPos]) // see if a connection point is there
      cConnections--;

  if (cConnections) hr = ResultFromScode (S_FALSE);

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::Reset
//
//      Purpose:
//        reset the internal position of this list
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::Reset ()
{
  nCurrPos = 0;
  return S_OK;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::Clone
//
//      Purpose:
//        give back a copy of this enumeration
//
//      Parameters:
//        LPENUMCONNECTIONS FAR*    points to where to copy the enumeration
//
//      Return Value:
//        HRESULT
//
HRESULT _IFUNC BEventHandler::BSinkList::Clone (LPENUMCONNECTIONS FAR* ppecn)
{
  HRESULT hr = S_OK;

  // check ppecn
  if (ppecn == NULL)      hr = ResultFromScode (E_INVALIDARG);

  *ppecn = (LPENUMCONNECTIONS) new BSinkList (this);

  // check ppecn for out of memory
  if (ppecn == NULL)      hr = ResultFromScode (E_OUTOFMEMORY);

  (*ppecn)->AddRef(); // user must release it

  return hr;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::SetSinkAt
//
//      Purpose:
//        save the two interface pointers. AddRef the IUnknown one since we keep it
//
//      Parameters:
//        LPUNKNOWN           sink passed by clients
//        LPVOID        interface obtained by QueryInterface on the previous IUnknown
//        int             position into the array
//
//      Return Value:
//        HRESULT
//
void BEventHandler::BSinkList::SetSinkAt (IUnknown* pU, LPVOID pObj, int nPos)
{
  pU->AddRef(); //\\//\\
  pUnkList[nPos] = pU;
  pDirectList[nPos] = pObj; // already AddRef'd by QueryInterface
  return;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::DeleteSinkAt
//
//      Purpose:
//        release pointers to interfaces at the specific position
//
//      Parameters:
//        int           position to delete
//
//      Return Value:
//        HRESULT
//
void BEventHandler::BSinkList::DeleteSinkAt (int nPos)
{
  pUnkList[nPos]->Release();
  pUnkList[nPos] = NULL;

  ((IUnknown*)(pDirectList[nPos]))->Release();
  pDirectList[nPos] = NULL;
  return;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::Expand
//
//      Purpose:
//        makes more room for new elements
//
//      Parameters:
//        none
//
//      Return Value:
//        int           1 if succesfull, 0 otherwise
//
int BEventHandler::BSinkList::Expand ()
{
  int newSize = nSize + nSize/2; // new dimension
  // allocate
  LPUNKNOWN FAR* pNewUnkList = (LPUNKNOWN FAR*) new char[sizeof(LPUNKNOWN) * newSize];
  LPVOID FAR* pNewIntList = (LPVOID FAR*) new char[sizeof(LPVOID) * newSize];

  if (pNewUnkList && pNewIntList) {
    // copy existing values
    memcpy (pNewUnkList, pUnkList, sizeof(LPUNKNOWN) * nSize);
    delete [] pUnkList; // delete old array
    pUnkList = pNewUnkList;
    memset (pUnkList + nSize, 0, sizeof(LPUNKNOWN) * nSize);

    memcpy (pNewIntList, pDirectList, sizeof(LPVOID) * nSize);
    delete [] pDirectList; // delete old array
    pDirectList = pNewIntList;
    memset (pDirectList + nSize, 0, sizeof(LPVOID) * nSize);

    nSize = newSize;
    return 1;
  }
  else
    return 0;
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
//      BSinkList::BSinkList copy constructor
//
//
BEventHandler::BSinkList::BSinkList (BSinkList* pList) :
                        nCurrPos (pList->nCurrPos),
                        nSize (pList->nSize),
                        cRef(0)
{
  // allocate
  LPUNKNOWN FAR* pUnkList = (LPUNKNOWN FAR*) new char[sizeof(LPUNKNOWN) * nSize];
  LPVOID FAR* pDirectList = (LPVOID FAR*) new char[sizeof(LPVOID) * nSize];

  for (int i = 0; i < nSize; i++) {
    // must copy and AddRef all elements
    pUnkList[i] = pList->pUnkList[i];
    if (pUnkList[i]) pUnkList[i]->AddRef();

    pDirectList[i] = pList->pDirectList[i];
    if (pDirectList[i]) ((LPUNKNOWN)(pDirectList[i]))->AddRef();
  }
}



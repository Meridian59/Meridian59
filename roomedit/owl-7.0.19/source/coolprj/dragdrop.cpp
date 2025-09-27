//----------------------------------------------------------------------------
// ObjectWindow - OWL NExt
// Copyright 1999. Yura Bidus. All Rights reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//  OVERVIEW
//  ~~~~~~~~
//  DragDrop support.
//----------------------------------------------------------------------------

#include <coolprj/pch.h>
#pragma hdrstop

#include <owl/scroller.h>
#include <coolprj/dragdrop.h>

using namespace owl;
using namespace std;

inline void __NoTrace(LPCTSTR /*lpszFormat*/, ...){}
extern void __Trace(LPCTSTR lpszFormat, ...);
//------------------------------------------------------------------------------

#if 0
#define _TRACE __Trace
#else
#define _TRACE __NoTrace
#endif

//-------------------------------------------------------------
// class TOleInitializer
// ~~~~~ ~~~~~~~~~~~~~~~
//
void TOleInitializer::Initialize()
{
  static TOleInitializer init;
}
//
TOleInitializer::TOleInitializer()
{
  // OLE DragDrop
  ::OleInitialize(NULL);
  //_TRACE("TOleInitializer()\n");
}
//
TOleInitializer::~TOleInitializer()
{
  ::OleFlushClipboard();
  // OLE DragDrop
  ::OleUninitialize();
  //_TRACE("~TOleInitializer()\n");
}
//-------------------------------------------------------------
// class TFormatEtc
// ~~~~~ ~~~~~~~~~~
//
TFormatEtc::TFormatEtc()
{
  memset(this, 0, sizeof(TFormatEtc));
  dwAspect = DVASPECT_CONTENT;
  lindex   = -1;
}
//
TFormatEtc::TFormatEtc(const TFormatEtc& etc)
{
  memcpy(this, &etc, sizeof(FORMATETC));
}
//
TFormatEtc::TFormatEtc(const FORMATETC& etc)
{
  memcpy(this, &etc, sizeof(FORMATETC));
}
//
TFormatEtc::TFormatEtc(CLIPFORMAT cfmt, DWORD tmd)
{
  memset(this, 0, sizeof(FORMATETC));
  cfFormat  = cfmt;
  tymed      = tmd;
  dwAspect  = DVASPECT_CONTENT;
  lindex    = -1;
}
//
bool TFormatEtc::operator ==(const TFormatEtc& other) const
{
  return cfFormat == other.cfFormat && (tymed & other.tymed) &&
         dwAspect == other.dwAspect;
}
//
bool TFormatEtc::operator !=(const TFormatEtc& other) const
{
  return !operator==(other);
}
//-------------------------------------------------------------
// class TStorageMedium
// ~~~~~ ~~~~~~~~~~~~~~
//
TStorageMedium::TStorageMedium()
{
  memset(this, 0, sizeof(TStorageMedium));
}
//-------------------------------------------------------------
// class TDragDropProxy
// ~~~~~ ~~~~~~~~~~~~~~
//
uint TDragDropProxy::ScrollInset = 0;
uint TDragDropProxy::ScrollDelay = 0;
TDragDropProxy::TDragDropProxy(TWindow*  parent)
:
  DataArray(0),
  DataSize(0),
  ItemsReserved(0),
  Parent(parent),
  DataObject(0)
{
  if(!ScrollInset)
    ScrollInset = GetProfileInt(_T("windows"), _T("DragScrollInset"), DD_DEFSCROLLINSET);
  if(!ScrollDelay)
    ScrollDelay = GetProfileInt(_T("windows"), _T("DragScrollDelay"), DD_DEFSCROLLDELAY);
  //_TRACE("TDragDropProxy::TDragDropProxy()\n");
}
//
TDragDropProxy::~TDragDropProxy()
{
  //_TRACE("TDragDropProxy::~TDragDropProxy()\n");
  ResetDragData();
}
//
void TDragDropProxy::ResetDragData()
{
  //_TRACE("TDragDropProxy::ResetDragData() START\n");
  for(int i = 0; i < (int)DataSize; i++){
    // cleanup current entry and return it
    CoTaskMemFree(DataArray[i].FormatEtc.ptd);
    ::ReleaseStgMedium(&DataArray[i].StgMedium);
  }
  delete[] DataArray;
  DataArray      = 0;
  DataSize      = 0;
  ItemsReserved  = 0;
  //_TRACE("TDragDropProxy::ResetDragData() END\n");
}
//
TDragDropProxy::TDataEntry*
TDragDropProxy::CreateDataEntry(TFormatEtc& formatEtc)
{
  //_TRACE("TDragDropProxy::CreateDataEntry()\n");
  if(!DataArray){
    ItemsReserved = 5;
    DataArray = new TDataEntry[ItemsReserved];
    memset(DataArray, 0, sizeof(TDataEntry)*ItemsReserved);
  }
  DataSize++;
  if(DataSize >= ItemsReserved){
    TDataEntry* tmp = new TDataEntry[ItemsReserved+5];
    memcpy(tmp,DataArray, ItemsReserved*sizeof(TDataEntry));
    ItemsReserved += 5;
    delete DataArray;
    DataArray = tmp;
  }
  DataArray[DataSize-1].FormatEtc = formatEtc;
  return &DataArray[DataSize-1];
}
//
TDragDropProxy::TDataEntry*
TDragDropProxy::GetDataEntry(TFormatEtc& formatEtc)
{
  if(!DataArray){
    //_TRACE("TDragDropProxy::CreateDataEntry() No DataArray!!!!!!!!!\n");
    return 0;
  }
  for(int i = 0; i < (int)DataSize;i++){
    TFormatEtc& fc = DataArray[i].FormatEtc;
    if (fc == formatEtc && (DataArray[i].StgMedium.tymed == TYMED_NULL ||
        fc.lindex == formatEtc.lindex)){
      CoTaskMemFree(fc.ptd);
      fc.ptd = 0;
      ::ReleaseStgMedium(&DataArray[i].StgMedium);
      //_TRACE("TDragDropProxy::CreateDataEntry() Found\n");
      return &DataArray[i];
    }
  }
  //_TRACE("TDragDropProxy::CreateDataEntry() NotFound !!!!!!!!!\n");
  return 0;
}
//
void TDragDropProxy::SetDragData(CLIPFORMAT cfFormat, HGLOBAL hGlobal)
{
  CHECK(hGlobal);
  //_TRACE("TDragDropProxy::SetDragData()\n");

  // fill in FORMATETC struct
  FORMATETC formatEtc;
  formatEtc.cfFormat  = cfFormat;
  formatEtc.ptd        = 0;
  formatEtc.dwAspect  = DVASPECT_CONTENT;
  formatEtc.lindex    = -1;
  formatEtc.tymed      = TYMED_HGLOBAL;

  // add it to the cache
  TFormatEtc entry(formatEtc);
  TDataEntry* pEntry = GetDataEntry(entry);
  if(!pEntry)
    pEntry = CreateDataEntry(entry);
  pEntry->StgMedium.tymed            = TYMED_HGLOBAL;
  pEntry->StgMedium.hGlobal          = hGlobal;
  pEntry->StgMedium.pUnkForRelease  = 0;
}
//
uint
TDragDropProxy::IsDroppable(const TPoint& point, const TRect& clientRect)
{
  //Check for at least a client area hit.
  if(!clientRect.Contains(point))
    return UDROP_NONE;

  uint retVal = UDROP_CLIENT;

  //Check horizontal inset
  if(point.x <= clientRect.left+(int)ScrollInset)
    retVal |= UDROP_INSETLEFT;
  else if(point.x >= clientRect.right-(int)ScrollInset)
    retVal |= UDROP_INSETRIGHT;

  //Check vertical inset
  if(point.y <= clientRect.top+(int)ScrollInset)
    retVal |= UDROP_INSETTOP;
  else if (point.y >= clientRect.bottom-(int)ScrollInset)
    retVal |= UDROP_INSETBOTTOM;

  return retVal;
}
//
bool
TDragDropProxy::PerformDrop(IDataObject*, const TPoint&, DROPEFFECT)
{
  return true;
}
//
bool
TDragDropProxy::StartDragDrop()
{
  // peek for next input message
  MSG msg;
  if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
    PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)){
    // check for button cancellation (any button down will cancel)
    if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP ||
      msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
      return false;

    // check for keyboard cancellation
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
      return false;
  }
  return true;
}
//
void
TDragDropProxy::EndDragDrop(DROPEFFECT)
{
  // end AutoMode loop in Scroller
  if(Parent && Parent->GetScroller())
  {
    MSG msg;
    PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);
    Parent->PostMessage(WM_MOUSEMOVE);
    Parent->PostMessage(WM_LBUTTONUP);
  }
}
/*
 * TDropSource::QueryDragContinue
 *
 * Purpose:
 *  Determines whether to continue a drag operation or cancel it.
 *
 * Parameters:
 *  fEsc            BOOL indicating that the ESC key was pressed.
 *  grfKeyState     DWORD providing states of keys and mouse buttons
 *
 * Return Value:
 *  HRESULT         DRAGDROP_S_CANCEL to stop the drag,
 *                  DRAGDROP_S_DROP to drop the data where it is,
 *                  or NOERROR to continue.
 */
STDMETHODIMP TDragDropProxy::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
{
  //_TRACE("TDragDropProxy::QueryContinueDrag()\n");
  if (fEsc)
    return DRAGDROP_S_CANCEL;

  if(!(grfKeyState & MK_LBUTTON))
    return DRAGDROP_S_DROP;

  return NOERROR;
}

/*
 * TDropSource::GiveFeedback
 *
 * Purpose:
 *  Provides cursor feedback to the user since the source task
 *  always has the mouse capture.  We can also provide any other
 *  type of feedback above cursors if we so desire.
 *
 * Parameters:
 *  dwEffect        DWORD effect flags returned from the last target
 *
 * Return Value:
 *  HRESULT         NOERROR if you set a cursor yourself or
 *                  DRAGDROP_S_USEDEFAULTCURSORS to let OLE do
 *                  the work.
 */

STDMETHODIMP TDragDropProxy::GiveFeedback(DWORD)
{
  //_TRACE("TDragDropProxy::GiveFeedback()\n");
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

/*
 * TDragDropProxy::DragEnter
 *
 * Purpose:
 *  Indicates that data in a drag operation has been dragged over
 *  our window that's a potential target.  We are to decide if it's
 *  something in which we're interested.
 *
 * Parameters:
 *  pIDataSource    LPDATAOBJECT providing the source data.
 *  grfKeyState     DWORD flags: states of keys and mouse buttons.
 *  pt              POINTL coordinates in the client space of
 *                  the document.
 *  pdwEffect       LPDWORD into which we'll place the appropriate
 *                  effect flag for this point.
 */

STDMETHODIMP TDragDropProxy::DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
  //_TRACE("TDragDropProxy::DragEnter()\n");
  *pdwEffect=DROPEFFECT_NONE;

  if(!Parent)
    return NOERROR;

  DataObject = pIDataSource;
  DataObject->AddRef();

  TPoint point(pt.x,pt.y);
  Parent->ScreenToClient(point);

  TRect clientRect;
  Parent->GetClientRect(clientRect);
  uint dropRes = IsDroppable(point, clientRect);

  LastTest = dropRes;
  if(dropRes == UDROP_NONE)
    *pdwEffect=DROPEFFECT_NONE;
  else{
    *pdwEffect = DROPEFFECT_MOVE;
    if (grfKeyState & MK_CONTROL)
      *pdwEffect = DROPEFFECT_COPY;
  }

  //Bring the document window up front, show what a drop will do.
  Parent->BringWindowToTop();
  Parent->UpdateWindow();
  SavedPos = point;
  VScrollCode = NOVALUE;
  HScrollCode = NOVALUE;
  PendingRepaint = false;
  ShowIndicator = false;
  if(dropRes != UDROP_NONE){
    ShowIndicator = true;
    ShowDropIndicator(point);
  }
  return NOERROR;
}

/* TDropTarget::DragOver                                               */
/*                                                                     */
/* Purpose:                                                            */
/*  Indicates that the mouse was moved inside the window represented   */
/*  by this drop target.  This happens on every WM_MOUSEMOVE, so       */
/*  this function should be very efficient.                            */
/* Parameters:                                                         */
/*  grfKeyState     DWORD providing the current keyboard and           */
/*                  mouse states                                       */
/*  pt              POINTL where the mouse currently is.               */
/*  pdwEffect       LPDWORD in which to store the effect flag          */
/*                  for this point.                                    */
/* Return Value:                                                       */
/*  HRESULT         NOERROR                                            */
STDMETHODIMP TDragDropProxy::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
  *pdwEffect=DROPEFFECT_NONE;

  if (!DataObject || !Parent)
    return NOERROR;

  TPoint point(pt.x,pt.y);
  Parent->ScreenToClient(point);
  TRect clientRect;
  Parent->GetClientRect(clientRect);
  uint testVal = IsDroppable(point, clientRect);

  if(testVal != UDROP_NONE){
    //Store these before possibly ORing in DROPEFFECT_SCROLL
    *pdwEffect=DROPEFFECT_MOVE;
    if (grfKeyState & MK_CONTROL)
      *pdwEffect=DROPEFFECT_COPY;
  }

  if(SavedPos == point && (LastTest&(UDROP_INSETHORZ|UDROP_INSETVERT))==0){
    return NOERROR;
  }
  /* Scrolling is a little tricky:  We get a DragOver pulse even   */
  /* if we didn't move.  First we have to delay scrolling for      */
  /* ppg->m_uScrollDelay clock ticks which we can determine using  */
  /* GetTickCount.  Timers do not work here since we may not be    */
  /* yielding to our message loop.                                 */
  /*                                                               */
  /* Once we know we are scrolling then we determine if we         */
  /* scroll again or if we reset the scrolling state.              */
  uint uLast = LastTest;
  LastTest   = testVal;
  if(Parent->GetScroller()){ // only if Window have Scroll
    if (UDROP_NONE==testVal){
      //If we're now an invalid point, better repaint as necessary
      if(PendingRepaint){
        Parent->UpdateWindow();
        PendingRepaint=false;
      }

      VScrollCode=NOVALUE;
      HScrollCode=NOVALUE;
      if (ShowIndicator)
        HideDropIndicator();
      ShowIndicator = false;
      return NOERROR;
    }

    if((UDROP_INSETHORZ&uLast) && !(UDROP_INSETHORZ&testVal))
      HScrollCode=NOVALUE;

    if (!(UDROP_INSETHORZ&uLast) && (UDROP_INSETHORZ&testVal)){
      TimeLast = GetTickCount();
      HScrollCode= (0!=(UDROP_INSETLEFT & testVal))
                    ? SB_LINELEFT : SB_LINERIGHT; //Same as UP & DOWN codes.
    }

    if ((UDROP_INSETVERT&uLast) && !(UDROP_INSETVERT&testVal))
      VScrollCode = NOVALUE;

    if (!(UDROP_INSETVERT&uLast) && (UDROP_INSETVERT&testVal)){
      TimeLast = GetTickCount();
      VScrollCode = (0!=(UDROP_INSETTOP&testVal))
                     ? SB_LINEUP : SB_LINEDOWN;
    }
    //Only change the last time if ALL scrolling stops.
    if (NOVALUE==HScrollCode && NOVALUE==VScrollCode)
      TimeLast=0L;

    //Set the scroll effect on any inset hit.
    if((UDROP_INSETHORZ | UDROP_INSETVERT)&testVal)
      *pdwEffect |= DROPEFFECT_SCROLL;
    int xPos = Parent->GetScroller()->XPos;
    int yPos = Parent->GetScroller()->YPos;

    //Has the delay elapsed?  We can scroll if so
    if(TimeLast != 0 && (GetTickCount() - TimeLast) > (uint32)ScrollDelay){
      if(NOVALUE != HScrollCode){
        PendingRepaint = true;
        Parent->HandleMessage(WM_HSCROLL, HScrollCode, 0L);
      }
      if(NOVALUE != VScrollCode){
        PendingRepaint=true;
        Parent->HandleMessage(WM_VSCROLL, VScrollCode, 0L);
      }
    }

    //If we didn't scroll but have a pending repaint, do it now.
    if(xPos==Parent->GetScroller()->XPos && yPos==Parent->GetScroller()->YPos && PendingRepaint){
      Parent->UpdateWindow();
      PendingRepaint=false;
    }
  }
  else if(PendingRepaint){
    Parent->UpdateWindow();
    PendingRepaint=false;
  }

  SavedPos = point;
  ShowIndicator = true;
  ShowDropIndicator(point);
  return NOERROR;
}

/*
 * TDropTarget::DragLeave
 *
 * Purpose:
 *  Informs the drop target that the operation has left its window.
 *
 * Parameters:
 *  None
 */
STDMETHODIMP TDragDropProxy::DragLeave()
{
  //_TRACE("TDragDropProxy::DragLeave()\n");
  if (!DataObject || !Parent)
    return NOERROR;

  //Stop scrolling
  HScrollCode = NOVALUE;
  VScrollCode = NOVALUE;
  DataObject->Release();
  DataObject = 0;

  if (PendingRepaint)
    Parent->UpdateWindow();
  if(ShowIndicator)
    HideDropIndicator();
  ShowIndicator = false;
  return NOERROR;
}

/*
 * TDropTarget::Drop
 *
 * Purpose:
 *  Instructs the drop target to paste the data that was just now
 *  dropped on it.
 *
 * Parameters:
 *  pIDataSource    LPDATAOBJECT from which we'll paste.
 *  grfKeyState     DWORD providing current keyboard/mouse state.
 *  pt              POINTL at which the drop occurred.
 *  pdwEffect       LPDWORD in which to store what you did.
 */
STDMETHODIMP TDragDropProxy::Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
  //_TRACE("TDragDropProxy::Drop()\n");
  *pdwEffect=DROPEFFECT_NONE;

  if(!DataObject || !Parent)
    return E_FAIL;

  TPoint point(pt.x,pt.y);
  Parent->ScreenToClient(point);
  TRect clientRect;
  Parent->GetClientRect(clientRect);
  if(UDROP_NONE == IsDroppable(point,clientRect))
    return E_FAIL;

  //Stop scrolling
  HScrollCode=NOVALUE;
  VScrollCode=NOVALUE;

  if(PendingRepaint)
    Parent->UpdateWindow();

  //2.  Remove the UI feedback
  if(ShowIndicator)
    HideDropIndicator();
  ShowIndicator = false;

  DataObject->Release();
  DataObject = 0;

  /* Check if we can do the paste, and if so, tell our pasting */
  /* mechanism exactly where to place us.                      */

  *pdwEffect = DROPEFFECT_MOVE;
  if (grfKeyState & MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;

  // if Parent is source and target both it will return true
  if(PerformDrop(pIDataSource, point, *pdwEffect))
    *pdwEffect = DROPEFFECT_NONE;
  return NOERROR;
}
//
/*
 * TDragDropProxy::GetData
 *
 * Purpose:
 *  Retrieves data described by a specific FormatEtc into a StgMedium
 *  allocated by this function.  Used like GetClipboardData.
 *
 * Parameters:
 *  pFE             LPFORMATETC describing the desired data.
 *  pSTM            LPSTGMEDIUM in which to return the data.
 */
STDMETHODIMP TDragDropProxy::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  //_TRACE("TDragDropProxy::GetData() Start\n");
  if (NULL==DataArray || NULL==pFE || NULL==pSTM){
    //_TRACE("TDragDropProxy::GetData() == DATA_E_FORMATETC 1\n");
    return DATA_E_FORMATETC;
  }

  for(int i=0; i < (int)DataSize; i++){
    TDataEntry& de = DataArray[i];
    /* Check if the requested FORMATETC is the same as one   */
    /* that we already have. If so, then copy that STGMEDIUM */
    /* to pSTM and AddRef ourselves for pUnkForRelease.      */
    if (de.FormatEtc== *pFE){
      if(de.FormatEtc.tymed == TYMED_HGLOBAL){
        *pSTM = de.StgMedium;
        PRECONDITION(de.StgMedium.hGlobal);
        SIZE_T nSize = ::GlobalSize(de.StgMedium.hGlobal);
        pSTM->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, nSize);
        LPVOID lpSource = ::GlobalLock(de.StgMedium.hGlobal);
        LPVOID lpDest = ::GlobalLock(pSTM->hGlobal);
        CHECK(lpSource);
        CHECK(lpDest);
        memcpy(lpDest, lpSource, nSize);
        ::GlobalUnlock(pSTM->hGlobal);
        ::GlobalUnlock(de.StgMedium.hGlobal);
        return NOERROR;
      }
      /* ReleaseStgMedium will Release both storage  */
      /* and stream elements regardless of the value */
      /* of pUnkForRelease, so we have to AddRef the */
      /* element and bump our own ref count here.    */
      if (de.FormatEtc.tymed == TYMED_ISTORAGE)
        de.StgMedium.pstg->AddRef();
      else if (de.FormatEtc.tymed == TYMED_ISTREAM)
        de.StgMedium.pstm->AddRef();
      *pSTM = de.StgMedium;
      //_TRACE("TDragDropProxy::GetData() type(%d)== NOERROR\n",(int)de.FormatEtc.tymed);
      return NOERROR;
    }
  }
  //_TRACE("TDragDropProxy::GetData() == DATA_E_FORMATETC\n");
  return DATA_E_FORMATETC;
}
/*
 * TDragDropProxy::GetDataHere
 *
 * Purpose:
 *  Renders the specific FormatEtc into caller-allocated medium
 *  provided in pSTM.
 *
 * Parameters:
 *  pFE             LPFORMATETC describing the desired data.
 *  pSTM            LPSTGMEDIUM providing the medium into which
 *                  wer render the data.
 */
STDMETHODIMP TDragDropProxy::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  //_TRACE("TDragDropProxy::GetDataHere() Start\n");
  if (NULL==DataArray || NULL==pFE || NULL==pSTM){
    //_TRACE("TDragDropProxy::GetDataHere() DATA_E_FORMATETC 1\n");
    return DATA_E_FORMATETC;
  }

  for (int i = 0; i < (int)DataSize; i++){
    TDataEntry& de = DataArray[i];
    /* When we find a matching FORMATETC, we know we're    */
    /* only looking for IStorage or IStream (we checked    */
    /* above), so use IStorage::CopyTo or IStream::CopyTo  */
    /* to make the copy.                                   */
    if (de.FormatEtc == *pFE){
      if(TYMED_ISTORAGE==pFE->tymed){
        pSTM->tymed=TYMED_ISTORAGE;
        return de.StgMedium.pstg->CopyTo(NULL, NULL, NULL, pSTM->pstg);
      }
      else if(TYMED_ISTREAM==pFE->tymed){
        STATSTG  st;
        de.StgMedium.pstm->Stat(&st, STATFLAG_NONAME);
        pSTM->tymed=TYMED_ISTREAM;
        return de.StgMedium.pstm->CopyTo(pSTM->pstm, st.cbSize, NULL, NULL);
      }
      *pSTM = de.StgMedium;
      //_TRACE("TDragDropProxy::GetDataHere() NOERROR\n");
      return NOERROR;
    }
  }
  //_TRACE("TDragDropProxy::GetDataHere() DATA_E_FORMATETC\n");
  return DATA_E_FORMATETC;
}
//
/*
 * TDragDropProxy::SetData
 *
 * Purpose:
 *  Places data described by a FormatEtc and living in a StgMedium
 *  into the object.  The object may be responsible to clean up the
 *  StgMedium before exiting.
 *
 * Parameters:
 *  pFE             LPFORMATETC describing the data to set.
 *  pSTM            LPSTGMEDIUM containing the data.
 *  fRelease        BOOL indicating if this function is responsible
 *                  for freeing the data.
 */
STDMETHODIMP TDragDropProxy::SetData(LPFORMATETC, LPSTGMEDIUM, BOOL)
{
  //_TRACE("TDragDropProxy::SetData()\n");
  return DATA_E_FORMATETC;
}
/*
 * TDragDropProxy::QueryGetData
 *
 * Purpose:
 *  Tests if a call to GetData with this FormatEtc will provide
 *  any rendering; used like IsClipboardFormatAvailable.
 *
 * Parameters:
 *  pFE             LPFORMATETC describing the desired data.
 */
STDMETHODIMP TDragDropProxy::QueryGetData(LPFORMATETC pFE)
{
  //_TRACE("TDragDropProxy::QueryGetData()\n");
  if (NULL==DataArray || NULL==pFE){
    //_TRACE("TDragDropProxy::QueryGetData() S_FALSE 1\n");
    return S_FALSE;
  }

  for (int i=0; i < (int)DataSize; i++){
    TDataEntry& de = DataArray[i];
    /* Check if the requested FORMATETC is the same as one  */
    /* that we already have.                                */
    if (de.FormatEtc== *pFE){
      //_TRACE("TDragDropProxy::QueryGetData() NOERROR\n");
      return NOERROR;
    }
  }
  //_TRACE("TDragDropProxy::QueryGetData() S_FALSE\n");
  return S_FALSE;
}
/*
 * TDragDropProxy::GetCanonicalFormatEtc
 *
 * Purpose:
 *  Provides the caller with an equivalent FormatEtc to the one
 *  provided when different FormatEtcs will produce exactly the
 *  same renderings.
 *
 * Parameters:
 *  pFEIn            LPFORMATETC of the first description.
 *  pFEOut           LPFORMATETC of the equal description.
 */
STDMETHODIMP TDragDropProxy::GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC)
{
  //_TRACE("TDragDropProxy::GetCanonicalFormatEtc()\n");
  return DATA_S_SAMEFORMATETC;
}
/*
 * TDragDropProxy::EnumFormatEtc
 *
 * Purpose:
 *  Returns an IEnumFORMATETC object through which the caller can
 *  iterate to learn about all the data formats this object can
 *  provide through either GetData[Here] or SetData.
 *
 * Parameters:
 *  dwDir           DWORD describing a data direction, either
 *                  DATADIR_SET or DATADIR_GET.
 *  ppEnum          LPENUMFORMATETC * in which to return the
 *                  pointer to the enumerator.
 */
STDMETHODIMP TDragDropProxy::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
  //_TRACE("TDragDropProxy::EnumFormatEtc()\n");
  TEnumFormatEtc* pEnum;
  *ppEnum=0;

  /* From an external point of view there are no SET formats,   */
  /* because we want to allow the user of this component object */
  /* to be able to stuff ANY format in via Set.  Only external  */
  /* users will call EnumFormatEtc and they can only Get.       */
  switch (dwDir){
    case DATADIR_GET:
      pEnum = new TEnumFormatEtc;
      break;

    case DATADIR_SET:
    default:
      pEnum=0;
      break;
  }

  if(pEnum){
    //Let the enumerator copy our format list.
    if(!pEnum->Init(DataArray, DataSize)){
      delete pEnum;
      return E_FAIL;
    }
    pEnum->AddRef();
    *ppEnum=pEnum;
    //_TRACE("TDragDropProxy::EnumFormatEtc() NOERROR\n");
    return NOERROR;
  }
  //_TRACE("TDragDropProxy::EnumFormatEtc() E_FAIL\n");
  return E_FAIL;
}
/*
 * TDragDropProxy::DAdvise
 *
 * Purpose:
 *  Provides the data object with an IAdviseSink object that we are
 *  responsible to notify when the data changes.
 *
 * Parameters:
 *  ppFE            LPFORMATETC
 *  dwFlags         DWORD carrying flags indicating how the advise
 *                  sink wants to be treated.
 *  pIAdviseSink    LPADVISESINK to the object to notify
 *  pdwConn         LPDWORD into which we store a DWORD key
 *                  identifying the advise connection.
 */
STDMETHODIMP TDragDropProxy::DAdvise(LPFORMATETC, DWORD /*dwFlags*/, LPADVISESINK, LPDWORD /*pdwConn*/)
{
  //_TRACE("TDragDropProxy::DAdvise()\n");
  return E_OUTOFMEMORY;
}
/*
 * TDragDropProxy::DUnadvise
 *
 * Purpose:
 *  Turns off advising previously set up with Advise.
 *
 * Parameters:
 *  dwConn          DWORD connection key returned from Advise.
 */
STDMETHODIMP TDragDropProxy::DUnadvise(DWORD /*dwConn*/)
{
  //_TRACE("TDragDropProxy::DUnadvise()\n");
  return E_FAIL;
}
/*
 * TDragDropProxy::EnumDAdvise
 *
 * Purpose:
 *  Returns an enumerator object through which the caller can find
 *  all the agents currently receiving advises on this data object.
 *
 * Parameters:
 *  ppEnum          LPENUMSTATDATA * in which to return the
 *                  enumerator.
 */
STDMETHODIMP TDragDropProxy::EnumDAdvise(LPENUMSTATDATA*)
{
  //_TRACE("TDragDropProxy::EnumDAdvise()\n");
  return E_FAIL;
}
///////////////////////////////////////////////////////////
//
// TDragDropSupport
//
//
//
uint TDragDropSupport::DragDelay   = 0;
uint TDragDropSupport::DragMinDist = 0;
//
TDragDropSupport::TDragDropSupport(TDragDropProxy* proxy)
:
  RefCnt(0),
  Proxy(proxy)
{
  //_TRACE("TDragDropSupport::TDragDropSupport()\n");
  TOleInitializer::Initialize();
  AddRef();
  if(!DragDelay)
    DragDelay    = GetProfileInt(_T("windows"), _T("DragDelay"), DD_DEFDRAGDELAY);
  if(!DragMinDist)
    DragMinDist  = GetProfileInt(_T("windows"), _T("DragMinDist"), DD_DEFDRAGMINDIST);
}
//
TDragDropSupport::~TDragDropSupport()
{
  //_TRACE("TDragDropSupport::~TDragDropSupport()\n");
}
//
TDragDropProxy*
TDragDropSupport::SetProxy(TDragDropProxy* proxy)
{
  //_TRACE("TDragDropSupport::SetProxy()\n");
  TDragDropProxy* tmp = Proxy;
  Proxy = proxy;
  return tmp;
}
//
bool TDragDropSupport::Register(TWindow& parent)
{
  //_TRACE("TDragDropSupport::Register()\n");
  PRECONDITION(parent.GetHandle());
  HRESULT hr = ::RegisterDragDrop(parent.GetHandle(), &DropTarget);
  CoLockObjectExternal(&DropTarget, TRUE, FALSE);
  return hr == S_OK || hr == DRAGDROP_E_ALREADYREGISTERED;
}
//
bool TDragDropSupport::UnRegister(TWindow& parent)
{
  //_TRACE("TDragDropSupport::UnRegister()\n");
  PRECONDITION(parent.GetHandle());
  CoLockObjectExternal(&DropTarget, FALSE, TRUE);
  return ::RevokeDragDrop(parent.GetHandle()) == S_OK;
}
//
STDMETHODIMP TDragDropSupport::QueryInterface(REFIID riid, LPVOID* ppv)
{
  *ppv = 0;

  if (IID_IUnknown==riid || IID_IDropSource==riid)
    *ppv=(LPVOID)&DropSource;

  if (IID_IDropTarget == riid)
    *ppv=(LPVOID)&DropTarget;

  if (IID_IDataObject == riid)
    *ppv=(LPVOID)&DataObject;

  if(*ppv != 0){
    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
  }
  return E_NOINTERFACE;
}
//
STDMETHODIMP_(ULONG)
TDragDropSupport::AddRef(void)
{
  return ++RefCnt;
}
//
STDMETHODIMP_(ULONG)
TDragDropSupport::Release()
{
  if(0L!=--RefCnt)
    return RefCnt;
  delete this;
  return 0;
}
//
bool
TDragDropSupport::CanDragDrop(const TPoint& startPoint, const TPoint& endPoint)
{
  TPoint delta = startPoint - endPoint;
  delta.x = delta.x > 0 ? delta.x : -delta.x;
  delta.y = delta.y > 0 ? delta.y : -delta.y;
  if(delta.x > (LONG)DragMinDist || delta.y > (LONG)DragMinDist)
    return true;
  return false;
}
//
DROPEFFECT
TDragDropSupport::DoDragDrop(DWORD dwEffects)
{
  DWORD dwEffect = DROPEFFECT_NONE;
  if(Proxy->StartDragDrop()){
    HRESULT hr = ::DoDragDrop(&DataObject, &DropSource, dwEffects, &dwEffect);
    if(hr != DRAGDROP_S_DROP)
      dwEffect = DROPEFFECT_NONE;
    Proxy->EndDragDrop(dwEffect);
  }
  return dwEffect;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

//////////////////////////////////////////////////////////////////////////
//
STDMETHODIMP TDragDropSupport::TDropSource::QueryInterface(REFIID riid, LPVOID* ppv)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropSource, theParent);
  return theParent->QueryInterface(riid, ppv);
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDropSource::AddRef()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropSource, theParent);
  return theParent->AddRef();
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDropSource::Release()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropSource, theParent);
  return theParent->Release();
}
//
STDMETHODIMP TDragDropSupport::TDropSource::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropSource, theParent);
  return theParent->Proxy->QueryContinueDrag(fEsc, grfKeyState);
}
//
STDMETHODIMP TDragDropSupport::TDropSource::GiveFeedback(DWORD dwEffect)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropSource, theParent);
  return theParent->Proxy->GiveFeedback(dwEffect);
}
//
STDMETHODIMP TDragDropSupport::TDropTarget::QueryInterface(REFIID riid, LPVOID *ppv)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->QueryInterface(riid, ppv);
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDropTarget::AddRef()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->AddRef();
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDropTarget::Release()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->Release();
}
//
STDMETHODIMP TDragDropSupport::TDropTarget::DragEnter(LPDATAOBJECT pIDataSource,
         DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->Proxy->DragEnter(pIDataSource, grfKeyState, pt, pdwEffect);
}
//
STDMETHODIMP TDragDropSupport::TDropTarget::DragOver(DWORD grfKeyState,
         POINTL pt, LPDWORD pdwEffect)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->Proxy->DragOver(grfKeyState, pt, pdwEffect);
}
//
STDMETHODIMP TDragDropSupport::TDropTarget::DragLeave()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->Proxy->DragLeave();
}
//
STDMETHODIMP TDragDropSupport::TDropTarget::Drop(LPDATAOBJECT pIDataSource,
          DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DropTarget, theParent);
  return theParent->Proxy->Drop(pIDataSource, grfKeyState, pt, pdwEffect);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->QueryInterface(riid, ppv);
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDataObject::AddRef()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->AddRef();
}
//
STDMETHODIMP_(ULONG) TDragDropSupport::TDataObject::Release()
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Release();
}
//
STDMETHODIMP TDragDropSupport::TDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  HRESULT hr = theParent->Proxy->GetData(pFE, pSTM);
  if(hr==NOERROR)
    AddRef();
  return hr;
}
//
STDMETHODIMP TDragDropSupport::TDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);

  HRESULT hr = theParent->Proxy->GetDataHere(pFE, pSTM);
  if(hr==NOERROR)
    AddRef();
  return hr;
}
//
STDMETHODIMP TDragDropSupport::TDataObject::QueryGetData(LPFORMATETC pFE)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->QueryGetData(pFE);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->GetCanonicalFormatEtc(pFEIn, pFEOut);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->SetData(pFE, pSTM, fRelease);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->EnumFormatEtc(dwDir, ppEnum);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::DAdvise(LPFORMATETC pFE, DWORD dwFlags, LPADVISESINK pIAdviseSink, LPDWORD pdwConn)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->DAdvise(pFE, dwFlags, pIAdviseSink, pdwConn);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::DUnadvise(DWORD dwConn)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->DUnadvise(dwConn);
}
//
STDMETHODIMP TDragDropSupport::TDataObject::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
  MEMBER_METHOD_PROLOGUE(TDragDropSupport, DataObject, theParent);
  return theParent->Proxy->EnumDAdvise(ppEnum);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

//
TEnumFormatEtc::TEnumFormatEtc()
:
  RefCnt(0),
  DataSize(0),
  DataArray(0)
{
  //_TRACE("TEnumFormatEtc::TEnumFormatEtc()\n");
  m_iCur = 0;
}
//
TEnumFormatEtc::~TEnumFormatEtc()
{
  //_TRACE("TEnumFormatEtc::~TEnumFormatEtc()\n");
  delete[] DataArray;
}
//
bool TEnumFormatEtc::Init(TDragDropProxy::TDataEntry* array, uint size)
{
  //_TRACE("TEnumFormatEtc::Init()\n");
  DataSize  = size;
  try{
    DataArray =  new TFormatEtc[DataSize];
  }
  catch(...){
    return false;
  }
  for(int i=0; i < (int)DataSize; i++)//Copy just the FORMATETC
    DataArray[i] = array[i].FormatEtc;
  return true;
}
/*
 * TEnumFormatEtc::QueryInterface
 * TEnumFormatEtc::AddRef
 * TEnumFormatEtc::Release
 *
 * Purpose:
 *  IUnknown members for TEnumFormatEtc object.  For QueryInterface
 *  we only return out own interfaces and not those of the data
 *  object.  However, since enumerating formats only makes sense
 *  when the data object is around, we insure that it stays as
 *  long as we stay by calling an outer IUnknown for AddRef and
 *  Release.  But since we are not controlled by the lifetime of
 *  the outer object, we still keep our own reference count in
 *  order to free ourselves.
 */
STDMETHODIMP TEnumFormatEtc::QueryInterface(REFIID riid, void** ppv)
{
  *ppv = 0;

  /*
   * Enumerators are separate objects, not the data object, so
   * we only need to support out IUnknown and IEnumFORMATETC
   * interfaces here with no concern for aggregation.
   */
  if (IID_IUnknown==riid || IID_IEnumFORMATETC==riid)
    *ppv=this;

  if(*ppv){
    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
  }
  return E_NOINTERFACE;
}
//
STDMETHODIMP_(ULONG) TEnumFormatEtc::AddRef()
{
  return ++RefCnt;
}
//
STDMETHODIMP_(ULONG) TEnumFormatEtc::Release()
{
  if (0!=--RefCnt)
    return RefCnt;

  delete this;
  return 0;
}
/*
 * CEnumFormatEtc::Next
 * CEnumFormatEtc::Skip
 * CEnumFormatEtc::Reset
 * CEnumFormatEtc::Clone
 *
 * Standard enumerator members for IEnumFORMATETC
 */
STDMETHODIMP TEnumFormatEtc::Next(ULONG cFE, LPFORMATETC pFE, ULONG *pulFE)
{
  //_TRACE("TEnumFormatEtc::Next()\n");
  ULONG cReturn = 0L;

  if(!DataArray)
    return S_FALSE;

  if(!pulFE){
    if(1L != cFE)
      return E_POINTER;
  }
  else
    *pulFE = 0L;
  if(NULL==pFE || m_iCur >= DataSize)
    return S_FALSE;

  while (m_iCur < DataSize && cFE > 0){
    *pFE++ = DataArray[m_iCur++];
    cReturn++;
    cFE--;
  }

  if(NULL!=pulFE)
    *pulFE = cReturn;

  return NOERROR;
}
//
STDMETHODIMP TEnumFormatEtc::Skip(ULONG cSkip)
{
  //_TRACE("TEnumFormatEtc::Skip() START\n");
  if ((m_iCur+cSkip) >= DataSize){
    //_TRACE("TEnumFormatEtc::Skip() FALSE\n");
    return S_FALSE;
  }

  m_iCur += cSkip;
  //_TRACE("TEnumFormatEtc::Skip() NOERROR\n");
  return NOERROR;
}
//
STDMETHODIMP TEnumFormatEtc::Reset()
{
  //_TRACE("TEnumFormatEtc::Reset()\n");
  m_iCur=0;
  return NOERROR;
}
//
STDMETHODIMP TEnumFormatEtc::Clone(IEnumFORMATETC** ppEnum)
{
  //_TRACE("TEnumFormatEtc::Clone()\n");
  TEnumFormatEtc*  pNew = new TEnumFormatEtc;

  pNew->DataSize  = DataSize;
  try{
    pNew->DataArray =  new TFormatEtc[DataSize];
  }
  catch(...){
    return E_OUTOFMEMORY;
  }
  for(int i=0; i < (int)DataSize; i++)
    pNew->DataArray[i] = DataArray[i];

  pNew->m_iCur = m_iCur;
  pNew->AddRef();

  *ppEnum=pNew;

  return NOERROR;
}
/////////////////////////////////////////////////////////////////////////////////

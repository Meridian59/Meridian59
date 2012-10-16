//------------------------------------------------------------------------------
// File: Crossbar.cpp
//
// Desc: A class for controlling video crossbars. 
//
//       This class creates a single object which encapsulates all connected
//       crossbars, enumerates all unique inputs which can be reached from
//       a given starting pin, and automatically routes audio when a video
//       source is selected.
//
//       The class supports an arbitrarily complex graph of crossbars, 
//       which can be cascaded and disjoint, that is not all inputs need 
//       to traverse the same set of crossbars.
//
//       Given a starting input pin (typically the analog video input to
//       the capture filter), the class recursively traces upstream 
//       searching for all viable inputs.  An input is considered viable if
//       it is a video pin and is either:
//
//           - unconnected 
//           - connects to a filter which does not support IAMCrossbar 
//
//       Methods:
//
//       CCrossbar (IPin *pPin);             
//       ~CCrossbar();
//
//       HRESULT GetInputCount (LONG *pCount);
//       HRESULT GetInputType  (LONG Index, LONG * PhysicalType);
//       HRESULT GetInputName  (LONG Index, TCHAR * pName, LONG NameSize);
//       HRESULT SetInputIndex (LONG Index);
//       HRESULT GetInputIndex (LONG *Index);
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include "crossbar.h"




//------------------------------------------------------------------------------
// Name: CCrossbar::CCrossbar()
// Desc: Constructor for the CCrossbar class
//------------------------------------------------------------------------------
CCrossbar::CCrossbar(
        IPin *pStartingInputPin
    ) 
    : m_pStartingPin (pStartingInputPin)
    , m_CurrentRoutingIndex (0)
    , m_RoutingList (NULL)

{
    HRESULT hr;

    DbgLog((LOG_TRACE,3,TEXT("CCrossbar Constructor")));

    ASSERT (pStartingInputPin != NULL);

    // Init everything to zero
    ZeroMemory (&m_RoutingRoot, sizeof (m_RoutingRoot));

    m_RoutingList = new CRoutingList (TEXT("RoutingList"), 5);
    if (m_RoutingList) {

        hr = BuildRoutingList(
                pStartingInputPin,
                &m_RoutingRoot, 
                0 /* Depth */);
    }
}


//------------------------------------------------------------------------------
// Name: CCrossbar::CCrossbar()
// Desc: Destructor for the CCrossbar class
//------------------------------------------------------------------------------
CCrossbar::~CCrossbar()
{
    HRESULT hr;

    DbgLog((LOG_TRACE,3,TEXT("CCrossbar Destructor")));

    hr = DestroyRoutingList ();

    delete m_RoutingList;
}


//
// This function is called recursively, every time a new crossbar is
// entered as we search upstream.
//
// Return values:
//
//  S_OK -    Returned on final exit after recursive search if at least
//            one routing is possible
//  S_FALSE - Normal return indicating we've reached the end of a 
//            recursive search, so save the current path
//  E_FAIL -  Unable to route anything

HRESULT
CCrossbar::BuildRoutingList (
   IPin     *pStartingInputPin,
   CRouting *pRouting,
   int       Depth
   )
{
    HRESULT  hr;
    LONG     InputIndexRelated;
    LONG     InputPhysicalType;
    LONG     OutputIndexRelated;
    LONG     OutputPhysicalType;
    IPin    *pPin;
    IPin    *pStartingOutputPin;
    CRouting RoutingNext;

    LONG     Inputs;
    LONG     Outputs;
    LONG     InputIndex;
    LONG     OutputIndex;
    PIN_INFO pinInfo;
    IAMCrossbar *pXbar;

    ASSERT (pStartingInputPin != NULL);
    ASSERT (pRouting != NULL);

    //
    // If the pin isn't connected, then it's a terminal pin
    //

    hr = pStartingInputPin->ConnectedTo (&pStartingOutputPin);
    if (hr != S_OK) {
        return (Depth == 0) ? E_FAIL : S_FALSE;
    }

    //
    // It is connected, so now find out if the filter supports 
    // IAMCrossbar
    //

    if (S_OK == pStartingOutputPin->QueryPinInfo(&pinInfo)) {
        ASSERT (pinInfo.dir == PINDIR_OUTPUT);

        hr = pinInfo.pFilter->QueryInterface(IID_IAMCrossbar, 
                            (void **)&pXbar);
        if (hr == S_OK) {
            EXECUTE_ASSERT (S_OK == pXbar->get_PinCounts(&Outputs, &Inputs));

            EXECUTE_ASSERT (S_OK == GetCrossbarIndexFromIPin (
                                    pXbar,
                                    &OutputIndex,
                                    FALSE,   // Input ?
                                    pStartingOutputPin));

            EXECUTE_ASSERT (S_OK == pXbar->get_CrossbarPinInfo(
                                    FALSE, // Input ?
                                    OutputIndex,
                                    &OutputIndexRelated,
                                    &OutputPhysicalType));

            //
            // for all input pins
            //

            for (InputIndex = 0; InputIndex < Inputs; InputIndex++) {
                EXECUTE_ASSERT (S_OK == pXbar->get_CrossbarPinInfo(
                                        TRUE, // Input?
                                        InputIndex,
                                        &InputIndexRelated,
                                        &InputPhysicalType));

                //
                // Is the pin a video pin?
                //
                if (InputPhysicalType < PhysConn_Audio_Tuner) {
                    //
                    // Can we route it?
                    //
                    if (S_OK == pXbar->CanRoute(OutputIndex, InputIndex)) {

                        EXECUTE_ASSERT (S_OK == GetCrossbarIPinAtIndex (
                                        pXbar,
                                        InputIndex,
                                        TRUE,   // Input
                                        &pPin));

                        //
                        // We've found a route through this crossbar
                        // so save our state before recusively searching
                        // again.
                        //
                        ZeroMemory (&RoutingNext, sizeof (RoutingNext));

                        // doubly linked list
                        RoutingNext.pRightRouting = pRouting;
                        pRouting->pLeftRouting = &RoutingNext;

                        pRouting->pXbar = pXbar;
                        pRouting->VideoInputIndex = InputIndex;
                        pRouting->VideoOutputIndex = OutputIndex;
                        pRouting->AudioInputIndex = InputIndexRelated;
                        pRouting->AudioOutputIndex = OutputIndexRelated;
                        pRouting->InputPhysicalType = InputPhysicalType;
                        pRouting->OutputPhysicalType = OutputPhysicalType;
                        pRouting->Depth = Depth;

                        hr = BuildRoutingList (
                                    pPin,
                                    &RoutingNext,
                                    Depth + 1);
                        
                        if (hr == S_OK) {
                            ; // Nothing to do?  
                        }
                        else if (hr == S_FALSE) {
                            pRouting->pLeftRouting = NULL;
                            SaveRouting (pRouting);
                        }
                        else if (hr == E_FAIL) {
                            ;  // Nothing to do?
                        }
                    } // if we can route
                } // if its a video pin
            } // for all input pins
            pXbar->Release();
        }
        else {
            // The filter doesn't support IAMCrossbar, so this
            // is a terminal pin
            pinInfo.pFilter->Release();
            pStartingOutputPin->Release ();

            return (Depth == 0) ? E_FAIL : S_FALSE;
        }

        pinInfo.pFilter->Release();
    }

    pStartingOutputPin->Release ();

    return S_OK;
}

//
// Make a copy of the current routing, and AddRef the IAMCrossbar
// interfaces.
//

HRESULT
CCrossbar::SaveRouting (CRouting *pRoutingNew)
{
    int j;
    int Depth= pRoutingNew->Depth + 1;
    CRouting *pr;
    CRouting *pCurrent = pRoutingNew;

    DbgLog((LOG_TRACE,3,TEXT("CCrossbar::SaveRouting, Depth=%d, NumberOfRoutings=%d"), 
            Depth, m_RoutingList->GetCount() + 1));

    pr = new CRouting[Depth];

    if (pr == NULL) {
        return E_FAIL;
    }

    m_RoutingList->AddTail (pr);

    for (j = 0; j < Depth; j++, pr++) {
        *pr = *pCurrent;
        ASSERT (pCurrent->pXbar != NULL);
        //
        // We're holding onto this interface, so AddRef
        //
        pCurrent->pXbar->AddRef();

        pCurrent = pCurrent->pRightRouting;

        //
        // Pointers were stack based during recursive search, so update them
        // in the allocated array
        //
        pr->pLeftRouting = &(*(pr-1));
        pr->pRightRouting = pCurrent;

        if (j == 0) {                   // first element
            pr->pLeftRouting = NULL;
        } 
        if (j == (Depth - 1)) {  // last element
            pr->pRightRouting = NULL;
        }
    }

    return S_OK;
}


//
//
HRESULT
CCrossbar::DestroyRoutingList()
{
    int k;
    int Depth;
    CRouting * pCurrent;

    DbgLog((LOG_TRACE,3,TEXT("DestroyRoutingList")));

    while (m_RoutingList->GetCount()) {
        pCurrent = m_RoutingList->RemoveHead();
        Depth = pCurrent->Depth + 1;

        for (k = 0; k < Depth; k++) {
            ASSERT (pCurrent->pXbar != NULL);
            pCurrent->pXbar->Release();

            pCurrent = pCurrent->pRightRouting;    
        }
    }

    return S_OK;
}


//
// Does not AddRef the returned *Pin 
//
HRESULT
CCrossbar::GetCrossbarIPinAtIndex(
   IAMCrossbar *pXbar,
   LONG PinIndex,
   BOOL IsInputPin,
   IPin ** ppPin)
{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins;
    ULONG        n;
    HRESULT      hr;

    *ppPin = 0;

    if(S_OK != pXbar->get_PinCounts(&cntOutPins, &cntInPins)) {
        return E_FAIL;
    }

    LONG TrueIndex = IsInputPin ? PinIndex : PinIndex + cntInPins;

    hr = pXbar->QueryInterface(IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) {
        if(SUCCEEDED(pFilter->EnumPins(&pins))) {            
            LONG i=0;
            while(pins->Next(1, &pP, &n) == S_OK) {
                pP->Release();
                if (i == TrueIndex) {
                    *ppPin = pP;
                    break;
                }
                i++;
            }
            pins->Release();
        }
        pFilter->Release();
    }
    
    return *ppPin ? S_OK : E_FAIL; 
}



//
// Find corresponding index of an IPin on a crossbar
//
HRESULT
CCrossbar::GetCrossbarIndexFromIPin (
    IAMCrossbar * pXbar,
    LONG * PinIndex,
    BOOL IsInputPin,
    IPin * pPin)

{
    LONG         cntInPins, cntOutPins;
    IPin        *pP = 0;
    IBaseFilter *pFilter = NULL;
    IEnumPins   *pins;
    ULONG        n;
    BOOL         fOK = FALSE;
    HRESULT      hr;

    if(S_OK != pXbar->get_PinCounts(&cntOutPins, &cntInPins)) {
        return E_FAIL;
    }

    hr = pXbar->QueryInterface(IID_IBaseFilter, (void **)&pFilter);

    if (hr == S_OK) {
        if(SUCCEEDED(pFilter->EnumPins(&pins))) {            
            LONG i=0;
            while(pins->Next(1, &pP, &n) == S_OK) {
                pP->Release();
                if (pPin == pP) {
                    *PinIndex = IsInputPin ? i : i - cntInPins;
                    fOK = TRUE;
                    break;
                }
                i++;
            }
            pins->Release();
        }
        pFilter->Release();
    }
    
    return fOK ? S_OK : E_FAIL; 
}


//
// How many unique video inputs can be selected?
//
HRESULT 
CCrossbar::GetInputCount (
    LONG *pCount)
{
    *pCount = m_RoutingList->GetCount();
    return S_OK;
}


//
// What is the physical type of a given input?
//
HRESULT 
CCrossbar::GetInputType (
    LONG Index, 
    LONG * PhysicalType)
{
    CRouting *pCurrent = m_RoutingList->GetHead();

    if (Index >= m_RoutingList->GetCount()) {
        return E_FAIL;
    }

    POSITION pos = m_RoutingList->GetHeadPosition();
    for (int j = 0; j <= Index; j++) {  
       pCurrent = m_RoutingList->GetNext(pos);
    }
    ASSERT (pCurrent != NULL);

    *PhysicalType = pCurrent->InputPhysicalType;

    return S_OK;
}


//
// Converts a PinType into a String
//
BOOL 
CCrossbar::StringFromPinType (TCHAR *pc, int nSize, long lType)
{
    TCHAR *pcT;
	BOOL bSuccess;

    switch (lType) {
    
    case PhysConn_Video_Tuner:              pcT = TEXT("Video Tuner");          break;
    case PhysConn_Video_Composite:          pcT = TEXT("Video Composite");      break;
    case PhysConn_Video_SVideo:             pcT = TEXT("Video SVideo");         break;
    case PhysConn_Video_RGB:                pcT = TEXT("Video RGB");            break;
    case PhysConn_Video_YRYBY:              pcT = TEXT("Video YRYBY");          break;
    case PhysConn_Video_SerialDigital:      pcT = TEXT("Video SerialDigital");  break;
    case PhysConn_Video_ParallelDigital:    pcT = TEXT("Video ParallelDigital");break;
    case PhysConn_Video_SCSI:               pcT = TEXT("Video SCSI");           break;
    case PhysConn_Video_AUX:                pcT = TEXT("Video AUX");            break;
    case PhysConn_Video_1394:               pcT = TEXT("Video 1394");           break;
    case PhysConn_Video_USB:                pcT = TEXT("Video USB");            break;
    case PhysConn_Video_VideoDecoder:       pcT = TEXT("Video Decoder");        break;
    case PhysConn_Video_VideoEncoder:       pcT = TEXT("Video Encoder");        break;
    
    case PhysConn_Audio_Tuner:              pcT = TEXT("Audio Tuner");          break;
    case PhysConn_Audio_Line:               pcT = TEXT("Audio Line");           break;
    case PhysConn_Audio_Mic:                pcT = TEXT("Audio Mic");            break;
    case PhysConn_Audio_AESDigital:         pcT = TEXT("Audio AESDigital");     break;
    case PhysConn_Audio_SPDIFDigital:       pcT = TEXT("Audio SPDIFDigital");   break;
    case PhysConn_Audio_SCSI:               pcT = TEXT("Audio SCSI");           break;
    case PhysConn_Audio_AUX:                pcT = TEXT("Audio AUX");            break;
    case PhysConn_Audio_1394:               pcT = TEXT("Audio 1394");           break;
    case PhysConn_Audio_USB:                pcT = TEXT("Audio USB");            break;
    case PhysConn_Audio_AudioDecoder:       pcT = TEXT("Audio Decoder");        break;
    
    default:
        pcT = TEXT("Unknown");
        break;
    }
    
    // return TRUE on sucessful copy
    if (lstrcpyn (pc, pcT, nSize) != NULL) {
		bSuccess = TRUE;
	}
	else {
		bSuccess = FALSE;
	}
	
	return (bSuccess);
};


//
// Get a text version of an input
//
// Return S_OK if the buffer is large enough to copy the string name
//
HRESULT 
CCrossbar::GetInputName (
    LONG   Index, 
    TCHAR *pName, 
    LONG   Size)
{
    CRouting *pCurrent = m_RoutingList->GetHead();

    if ((Index >= m_RoutingList->GetCount()) || (pName == NULL)) {
        return E_FAIL;
    }

    POSITION pos = m_RoutingList->GetHeadPosition();
    for (int j = 0; j <= Index; j++) { 
       pCurrent = m_RoutingList->GetNext(pos);
    }
    ASSERT (pCurrent != NULL);

    return (StringFromPinType (pName, Size, pCurrent->InputPhysicalType) ?
            S_OK : E_FAIL);
}


//
// Select an input 
//
HRESULT 
CCrossbar::SetInputIndex (
    LONG Index)
{
    HRESULT hr = E_FAIL;
    CRouting *pCurrent = m_RoutingList->GetHead();
    int j;

    if (Index >= m_RoutingList->GetCount()) {
        return hr;
    }

    POSITION pos = m_RoutingList->GetHeadPosition();
    for (j = 0; j <= Index; j++) { 
       pCurrent = m_RoutingList->GetNext(pos);
    }

    ASSERT (pCurrent != NULL);

    int Depth= pCurrent->Depth + 1;

    for (j = 0; j < Depth; j++) {
        hr = pCurrent->pXbar->Route (pCurrent->VideoOutputIndex, pCurrent->VideoInputIndex);
        ASSERT (S_OK == hr);

        if ((pCurrent->AudioOutputIndex != -1) && (pCurrent->AudioInputIndex != -1)) {
            EXECUTE_ASSERT (S_OK == pCurrent->pXbar->Route (pCurrent->AudioOutputIndex, pCurrent->AudioInputIndex));
        }

        DbgLog((LOG_TRACE,3,TEXT("CCrossbar::Routing, VideoOutIndex=%d VideoInIndex=%d"), 
                pCurrent->VideoOutputIndex, pCurrent->VideoInputIndex));

        pCurrent++;
    }

    m_CurrentRoutingIndex = Index;

    return hr;
}


//
// What input is currently selected?
//
HRESULT 
CCrossbar::GetInputIndex (
    LONG *Index)
{
    *Index = m_CurrentRoutingIndex;
    return S_OK;
}


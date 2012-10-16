//------------------------------------------------------------------------------
// File: Graph.h
//
// Desc: Sample code for BDA graph building.
//
// Copyright (c) 2000-2001, Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#ifndef GRAPH_H_INCLUDED_
#define GRAPH_H_INCLUDED_

#include "bdasampl.h"

class CBDAFilterGraph
{
private:
    CComPtr <ITuningSpace>   m_pITuningSpace;

    CComPtr <IScanningTuner> m_pITuner;

    CComPtr <IGraphBuilder>  m_pFilterGraph;         // for current graph
    CComPtr <IMediaControl>  m_pIMediaControl;       // for controlling graph state
    CComPtr <ICreateDevEnum> m_pICreateDevEnum;      // for enumerating system devices

    CComPtr <IBaseFilter>    m_pNetworkProvider;     // for network provider filter
    CComPtr <IBaseFilter>    m_pTunerDevice;         // for tuner device filter
    CComPtr <IBaseFilter>    m_pDemodulatorDevice;   // for tuner device filter
    CComPtr <IBaseFilter>    m_pCaptureDevice;       // for capture device filter
    CComPtr <IBaseFilter>    m_pDemux;               // for demux filter
    CComPtr <IBaseFilter>    m_pVideoDecoder;        // for mpeg video decoder filter
    CComPtr <IBaseFilter>    m_pAudioDecoder;        // for mpeg audio decoder filter
    CComPtr <IBaseFilter>    m_pTIF;                 // for transport information filter
    CComPtr <IBaseFilter>    m_pMPE;                 // for multiple protocol encapsulator
    CComPtr <IBaseFilter>    m_pIPSink;              // for ip sink filter
    CComPtr <IBaseFilter>    m_pOVMixer;             // for overlay mixer filter
    CComPtr <IBaseFilter>    m_pVRenderer;           // for video renderer filter
    CComPtr <IBaseFilter>    m_pDDSRenderer;         // for sound renderer filter

    //required for an ATSC network when creating a tune request
    LONG            m_lMajorChannel;
    LONG            m_lMinorChannel;
    LONG            m_lPhysicalChannel;

    //registration number for the RunningObjectTable
    DWORD           m_dwGraphRegister;

    NETWORK_TYPE    m_NetworkType;

    HRESULT InitializeGraphBuilder();
    HRESULT LoadTuningSpace();
    HRESULT LoadNetworkProvider();
    HRESULT LoadDemux();
    HRESULT RenderDemux();
    HRESULT LoadFilter(
        REFCLSID clsid, 
        IBaseFilter** ppFilter,
        IBaseFilter* pConnectFilter, 
        BOOL fIsUpstream
        );
    HRESULT ConnectFilters(
        IBaseFilter* pFilterUpstream, 
        IBaseFilter* pFilterDownstream
        );
    HRESULT CreateATSCTuneRequest(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel,
        IATSCChannelTuneRequest**   pTuneRequest
        );

public:
    bool            m_fGraphBuilt;
    bool            m_fGraphRunning;
    bool            m_fGraphFailure;

    CBDAFilterGraph();
    
    ~CBDAFilterGraph();

    HRESULT     
    BuildGraph(
        NETWORK_TYPE NetworkType
        );

    HRESULT 
    RunGraph();

    HRESULT 
    StopGraph();

    HRESULT 
    TearDownGraph();

    HRESULT 
    SetVideoWindow(
        HWND hwndMain
        );
    
    HRESULT
    ChangeChannel(
        LONG lMajorChannel, 
        LONG lMinorChannel
        );
    

    // Adds/removes a DirectShow filter graph from the Running Object Table,
    // allowing GraphEdit to "spy" on a remote filter graph if enabled.
    HRESULT 
    AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        );

    void 
    RemoveGraphFromRot(
        DWORD pdwRegister
        );

    LONG
    GetMajorChannel () {return m_lMajorChannel;};

    LONG
    GetMinorChannel () {return m_lMinorChannel;};
 };
#endif // GRAPH_H_INCLUDED_

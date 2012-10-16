//------------------------------------------------------------------------------
// File: Graph.cpp
//
// Desc: Sample code for BDA graph building.
//
// Copyright (c) 2000-2001, Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include "graph.h"

// Constructor, initializes member variables
// and calls InitializeGraphBuilder
CBDAFilterGraph::CBDAFilterGraph() :
    m_fGraphBuilt(FALSE),
    m_fGraphRunning(FALSE),
    m_NetworkType(ATSC),
    m_lMajorChannel(-1), 
    m_lMinorChannel(-1),
    m_lPhysicalChannel(46L),// 46 is an in house test channel - go ahead and change it
    m_dwGraphRegister (0)
{
    if(FAILED(InitializeGraphBuilder()))
        m_fGraphFailure = TRUE;
    else
        m_fGraphFailure = FALSE;
}

// Destructor
CBDAFilterGraph::~CBDAFilterGraph()
{
    if(m_fGraphRunning)
    {
        StopGraph();
    }

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        TearDownGraph();
    }
}


// Instantiate graph object for filter graph building
HRESULT
CBDAFilterGraph::InitializeGraphBuilder()
{
    HRESULT hr = S_OK;
    
    // we have a graph already
    if (m_pFilterGraph)
        return S_OK;

    // create the filter graph
    if (FAILED (hr = m_pFilterGraph.CoCreateInstance (CLSID_FilterGraph)))
    {
        ErrorMessageBox(TEXT("Couldn't CoCreate IGraphBuilder\n"));
        m_fGraphFailure = TRUE;
        return hr;
    }
    
    return hr;
}

// BuildGraph sets up devices, adds and connects filters
HRESULT
CBDAFilterGraph::BuildGraph(NETWORK_TYPE NetType)
{
    HRESULT hr = S_OK;
    m_NetworkType = NetType;

    // if we have already have a filter graph, tear it down
    if(m_fGraphBuilt)
    {
        if(m_fGraphRunning)
        {
            hr = StopGraph ();
        }

        hr = TearDownGraph ();
    }

    // STEP 1: load network provider first so that it can configure other
    // filters, such as configuring the demux to sprout output pins.
    // We also need to submit a tune request to the Network Provider so it will
    // tune to a channel
    if(FAILED (hr = LoadNetworkProvider()))
    {
        ErrorMessageBox(TEXT("Cannot load network provider\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    hr = m_pNetworkProvider->QueryInterface(__uuidof (ITuner), reinterpret_cast <void**> (&m_pITuner));
    if(FAILED (hr))
    {
        ErrorMessageBox(TEXT("pNetworkProvider->QI: Can't QI for ITuner.\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // create a tune request to initialize the network provider
    // before connecting other filters
    CComPtr <IATSCChannelTuneRequest>  pATSCTuneRequest;
    if(FAILED (hr = CreateATSCTuneRequest(
        m_lPhysicalChannel,
        m_lMajorChannel, 
        m_lMinorChannel,
        &pATSCTuneRequest
        )))
    {
        ErrorMessageBox(TEXT("Cannot create tune request\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    //submit the tune request to the network provider
    hr = m_pITuner->put_TuneRequest(pATSCTuneRequest);
    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("Cannot submit the tune request\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }


    // STEP2: Load tuner device and connect to network provider
    if(FAILED (hr = LoadFilter (
        KSCATEGORY_BDA_NETWORK_TUNER, 
        &m_pTunerDevice,
        m_pNetworkProvider, 
        TRUE
        )))
    {
        ErrorMessageBox(TEXT("Cannot load tuner device and connect network provider\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // STEP3: Load tuner device and connect to demodulator device
    if(FAILED (hr = LoadFilter (
        KSCATEGORY_BDA_RECEIVER_COMPONENT, 
        &m_pDemodulatorDevice,
        m_pTunerDevice, 
        TRUE
        )))
    {
        ErrorMessageBox(TEXT("Cannot load capture device and connect tuner\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // Step4: Load capture device and connect to tuner device
    if(FAILED (hr = LoadFilter (
        KSCATEGORY_BDA_RECEIVER_COMPONENT, 
        &m_pCaptureDevice,
        m_pDemodulatorDevice, 
        TRUE
        )))
    {
        ErrorMessageBox(TEXT("Cannot load capture device and connect tuner\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    // Step5: Load demux
    if(FAILED (hr = LoadDemux()))
    {
        ErrorMessageBox(TEXT("Cannot load demux\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    //
    // this next call loads and connects filters associated with
    // the demultiplexor. if you want to manually load individual
    // filters such as audio and video decoders, use the code at
    // the bottom of this file
    //
#ifdef DEBUG
    hr = AddGraphToRot (m_pFilterGraph, &m_dwGraphRegister);
    if (FAILED(hr))
    {
        ///ErrorMessageBox(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        m_dwGraphRegister = 0;
    }
#endif
    //MessageBox (NULL, _T(""), _T(""), MB_OK);
    // Step6: Render demux pins
    if(FAILED (hr = RenderDemux()))
    {
        ErrorMessageBox(TEXT("Cannot load demux\n"));
        TearDownGraph();
        m_fGraphFailure = true;
        return hr;
    }

    m_fGraphBuilt = true;

    return S_OK;
}


// Loads the correct tuning space based on NETWORK_TYPE that got
// passed into BuildGraph()
HRESULT
CBDAFilterGraph::LoadTuningSpace()
{   
    CComPtr <ITuningSpaceContainer>  pITuningSpaceContainer;

    // get the tuningspace container for all the tuning spaces from SYSTEM_TUNING_SPACES
    HRESULT hr = pITuningSpaceContainer.CoCreateInstance(CLSID_SystemTuningSpaces);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Could not CoCreate SystemTuningSpaces\n"));
        return hr;
    }

    CComVariant var (m_NetworkType);

    hr = pITuningSpaceContainer->get_Item(var, &m_pITuningSpace);

    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("Unable to retrieve Tuning Space\n"));
    }

    return hr;
}

// Creates an ATSC Tune Request
HRESULT
CBDAFilterGraph::CreateATSCTuneRequest(
        LONG lPhysicalChannel,
        LONG lMajorChannel, 
        LONG lMinorChannel,
        IATSCChannelTuneRequest**   pTuneRequest
    )
{
    HRESULT hr = S_OK;

    if (pTuneRequest == NULL)
    {
        ErrorMessageBox (TEXT("Invalid pointer\n"));
        return E_POINTER;
    }

    // Making sure we have a valid tuning space
    if (m_pITuningSpace == NULL)
    {
        ErrorMessageBox(TEXT("Tuning Space is NULL\n"));
        return E_FAIL;
    }

    //  Create an instance of the ATSC tuning space
    CComQIPtr <IATSCTuningSpace> pATSCTuningSpace (m_pITuningSpace);
    if (!pATSCTuningSpace)
    {
        ErrorMessageBox(TEXT("Cannot QI for an IATSCTuningSpace\n"));
        return E_FAIL;
    }

    //  Create an empty tune request.
    CComPtr <ITuneRequest> pNewTuneRequest;
    hr = pATSCTuningSpace->CreateTuneRequest(&pNewTuneRequest);

    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("CreateTuneRequest: Can't create tune request.\n"));
        return hr;
    }

    //query for an IATSCChannelTuneRequest interface pointer
    CComQIPtr <IATSCChannelTuneRequest> pATSCTuneRequest (pNewTuneRequest);
    if (!pATSCTuneRequest)
    {
        ErrorMessageBox(TEXT("CreateATSCTuneRequest: Can't QI for IATSCChannelTuneRequest.\n"));
        return E_FAIL;
    }

    //  Set the initial major and minor channels
    hr = pATSCTuneRequest->put_Channel(lMajorChannel);
    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("put_Channel failed\n"));
        return hr;
    }

    hr = pATSCTuneRequest->put_MinorChannel(lMinorChannel);
    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("put_MinorChannel failed\n"));
        return hr;
    }

    CComPtr <IATSCLocator> pATSCLocator;
    hr = pATSCLocator.CoCreateInstance (CLSID_ATSCLocator);
    if (FAILED( hr))
    {
        ErrorMessageBox(TEXT("Cannot create the ATSC locator failed\n"));
        return hr;
    }

    //  Set the initial physical channel.
    //
    hr = pATSCLocator->put_PhysicalChannel (lPhysicalChannel);
    if (FAILED( hr))
    {
        ErrorMessageBox(TEXT("Cannot put the physical channel\n"));
        return hr;
    }

    hr = pATSCTuneRequest->put_Locator (pATSCLocator);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot put the locator\n"));
        return hr;
    }

    hr = pATSCTuneRequest.QueryInterface (pTuneRequest);

    return hr;
}


// LoadNetworkProvider loads network provider
HRESULT
CBDAFilterGraph::LoadNetworkProvider()
{
    HRESULT     hr = S_OK;
    CComBSTR    bstrNetworkType;
    CLSID       CLSIDNetworkType;

    // obtain tuning space then load network provider
    if(m_pITuningSpace == NULL)
    {
        hr = LoadTuningSpace();
        if(FAILED(hr))
        {
            ErrorMessageBox(TEXT("Cannot load TuningSpace\n"));
            return hr;
        }
    }

    // Get the current Network Type clsid
    hr = m_pITuningSpace->get_NetworkType(&bstrNetworkType);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("ITuningSpace::Get Network Type failed\n"));
        return hr;
    }

    hr = CLSIDFromString(bstrNetworkType, &CLSIDNetworkType);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Couldn't get CLSIDFromString\n"));
        return hr;
    }

    // create the network provider based on the clsid obtained from the tuning space
    hr = CoCreateInstance(CLSIDNetworkType, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pNetworkProvider));
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Couldn't CoCreate Network Provider\n"));
        return hr;
    }

    //add the Network Provider filter to the graph
    hr = m_pFilterGraph->AddFilter(m_pNetworkProvider, L"Network Provider");

    return hr;
}


// enumerates through registered filters
// instantiates the the filter object and adds it to the graph
// it checks to see if it connects to upstream filter
// if not,  on to the next enumerated filter
// used for tuner, capture, MPE Data Filters and decoders that
// could have more than one filter object
// if pUpstreamFilter is NULL don't bother connecting
HRESULT
CBDAFilterGraph::LoadFilter(
    REFCLSID clsid, 
    IBaseFilter** ppFilter,
    IBaseFilter* pConnectFilter, 
    BOOL fIsUpstream
    )
{
    HRESULT                 hr = S_OK;
    BOOL                    fFoundFilter = FALSE;
    CComPtr <IMoniker>      pIMoniker;
    CComPtr <IEnumMoniker>  pIEnumMoniker;

    if (!m_pICreateDevEnum)
    {
        hr = m_pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
        if (FAILED (hr))
        {
            ErrorMessageBox(TEXT("LoadFilter(): Cannot CoCreate ICreateDevEnum"));
            return hr;
        }
    }

    // obtain the enumerator
    hr = m_pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("LoadFilter(): Cannot CreateClassEnumerator"));
        return hr;
    }
    if (S_OK != hr)  // Class not found
    {
        ErrorMessageBox(TEXT("LoadFilter(): Class not found, CreateClassEnumerator returned S_FALSE"));
        return E_UNEXPECTED;
    }

    // next filter
    while(pIEnumMoniker->Next(1, &pIMoniker, 0) == S_OK)
    {
        // obtain filter's friendly name
        CComPtr <IPropertyBag>  pBag;
        hr = pIMoniker->BindToStorage(
                NULL, 
                NULL, 
                IID_IPropertyBag,
                reinterpret_cast<void**>(&pBag)
                );

        if(FAILED(hr))
        {
            OutputDebugString (TEXT("LoadFilter(): Cannot BindToStorage"));
            return hr;
        }

        CComVariant varBSTR;
        hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
        if(FAILED(hr))
        {
            OutputDebugString (TEXT("LoadFilter(): IPropertyBag->Read method failed"));
            pIMoniker = NULL;
            continue;
        }

        // bind the filter
        CComPtr <IBaseFilter>   pFilter;
        hr = pIMoniker->BindToObject(
                NULL, 
                NULL, 
                IID_IBaseFilter,
                reinterpret_cast<void**>(&pFilter)
                );

        if (FAILED(hr))
        {
            pIMoniker = NULL;
            pFilter = NULL;
            continue;
        }


        hr = m_pFilterGraph->AddFilter (pFilter, varBSTR.bstrVal);

        if (FAILED(hr))
        {
            OutputDebugString (TEXT("Cannot add filter\n"));
            return hr;
        }

        //MessageBox (NULL, _T(""), _T(""), MB_OK);
        // test connections
        // to upstream filter
        if (pConnectFilter)
        {
            if(fIsUpstream)
            {
                hr = ConnectFilters (pConnectFilter, pFilter);
            }
            else
            {
                hr = ConnectFilters (pFilter, pConnectFilter);
            }

            if(SUCCEEDED(hr))
            {
                //that's the filter we want
                fFoundFilter = TRUE;
                pFilter.QueryInterface (ppFilter);
                break;
            }
            else
            {
                fFoundFilter = FALSE;
                // that wasn't the the filter we wanted
                // so unload and try the next one
                hr = m_pFilterGraph->RemoveFilter(pFilter);

                if(FAILED(hr))
                {
                    OutputDebugString(TEXT("Failed unloading Filter\n"));
                    return hr;
                }
            }
        }
        else
        {
            fFoundFilter = TRUE;
            pFilter.QueryInterface (ppFilter);
            break;
        }

        pIMoniker = NULL;
        pFilter = NULL;
    } // while
    return S_OK;
}

// loads the demux into the FilterGraph
HRESULT
CBDAFilterGraph::LoadDemux()
{
    HRESULT hr = S_OK;
    
    hr = CoCreateInstance(
            CLSID_MPEG2Demultiplexer, 
            NULL, 
            CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, 
            reinterpret_cast<void**>(&m_pDemux)
            );
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Could not CoCreateInstance CLSID_MPEG2Demultiplexer\n"));
        return hr;
    }

    hr = m_pFilterGraph->AddFilter(m_pDemux, L"Demux");
    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("Unable to add demux filter to graph\n"));
        return hr;
    }

    return hr;
}


// renders demux output pins
HRESULT
CBDAFilterGraph::RenderDemux()
{
    HRESULT             hr = S_OK;
    CComPtr <IPin>      pIPin;
    CComPtr <IPin>      pDownstreamPin;
    CComPtr <IEnumPins> pIEnumPins;
    PIN_DIRECTION       direction;

    if (!m_pDemux)
    {
        return E_FAIL;
    }

    // connect the demux to the capture device
    hr = ConnectFilters (m_pCaptureDevice, m_pDemux);

    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot connect demux to capture filter\n"));
        return hr;
    }

    // load transform information filter and connect it to the demux
    hr = LoadFilter (
            KSCATEGORY_BDA_TRANSPORT_INFORMATION, 
            &m_pTIF, 
            m_pDemux, 
            TRUE
            );
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot load TIF\n"));
        return hr;
    }

    // load multi protocol encapsulator
    hr = LoadFilter (
            KSCATEGORY_BDA_RECEIVER_COMPONENT, 
            &m_pMPE, 
            m_pDemux, 
            TRUE
            );
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot load MPE\n"));
        return hr;
    }

    // load IP Sink
    hr = LoadFilter (
            KSCATEGORY_IP_SINK, 
            &m_pIPSink, 
            m_pMPE, 
            TRUE
            );
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot load IP Sink\n"));
        return hr;
    }

    // render/connect the rest of the demux pins
    hr = m_pDemux->EnumPins (&pIEnumPins);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Cannot get the enumpins\n"));
        return hr;
    }

    while(pIEnumPins->Next(1, &pIPin, 0) == S_OK)
    {
        hr = pIPin->QueryDirection(&direction);

        if(direction == PINDIR_OUTPUT)
        {
            pIPin->ConnectedTo (&pDownstreamPin);

            if(pDownstreamPin == NULL)
            {
                m_pFilterGraph->Render (pIPin);
            }

            pDownstreamPin = NULL;
        }
        pIPin = NULL;
    }

    return hr;
}


// removes each filter from the graph
HRESULT
CBDAFilterGraph::TearDownGraph()
{
    HRESULT hr = S_OK;
    CComPtr <IBaseFilter> pFilter;
    CComPtr <IEnumFilters> pIFilterEnum;

    m_pITuningSpace = NULL;

    if(m_fGraphBuilt || m_fGraphFailure)
    {
        // unload manually added filters
        m_pFilterGraph->RemoveFilter(m_pIPSink);
        m_pFilterGraph->RemoveFilter(m_pMPE);
        m_pFilterGraph->RemoveFilter(m_pTIF);
        m_pFilterGraph->RemoveFilter(m_pDemux);
        m_pFilterGraph->RemoveFilter(m_pNetworkProvider);
        m_pFilterGraph->RemoveFilter(m_pTunerDevice);
        m_pFilterGraph->RemoveFilter(m_pCaptureDevice);

        m_pIPSink = NULL;
        m_pMPE = NULL;
        m_pTIF = NULL;
        m_pDemux = NULL;
        m_pNetworkProvider = NULL;
        m_pTunerDevice = NULL;
        m_pDemodulatorDevice = NULL;
        m_pCaptureDevice = NULL;

        // now go unload rendered filters
        hr = m_pFilterGraph->EnumFilters(&pIFilterEnum);

        if(FAILED(hr))
        {
            ErrorMessageBox(TEXT("TearDownGraph: cannot EnumFilters\n"));
            return E_FAIL;
        }

        pIFilterEnum->Reset();

        while(pIFilterEnum->Next(1, &pFilter, 0) == S_OK) // addrefs filter
        {
            hr = m_pFilterGraph->RemoveFilter(pFilter);

            if (FAILED (hr))
                return hr;

            pIFilterEnum->Reset();
            pFilter.Release ();
        }
    }

#ifdef DEBUG
    if (m_dwGraphRegister)
    {
        RemoveGraphFromRot(m_dwGraphRegister);
        m_dwGraphRegister = 0;
    }
#endif

    m_fGraphBuilt = FALSE;
    return S_OK;
}


// ConnectFilters is called from BuildGraph
// to enumerate and connect pins
HRESULT
CBDAFilterGraph::ConnectFilters(
    IBaseFilter* pFilterUpstream, 
    IBaseFilter* pFilterDownstream
    )
{
    HRESULT         hr = E_FAIL;

    CComPtr <IPin>  pIPinUpstream;


    PIN_INFO        PinInfoUpstream;
    PIN_INFO        PinInfoDownstream;

    // validate passed in filters
    ASSERT (pFilterUpstream);
    ASSERT (pFilterDownstream);

    // grab upstream filter's enumerator
    CComPtr <IEnumPins> pIEnumPinsUpstream;
    hr = pFilterUpstream->EnumPins(&pIEnumPinsUpstream);

    if(FAILED(hr))
    {
        ErrorMessageBox(TEXT("Cannot Enumerate Upstream Filter's Pins\n"));
        return hr;
    }

    // iterate through upstream filter's pins
    while (pIEnumPinsUpstream->Next (1, &pIPinUpstream, 0) == S_OK)
    {
        hr = pIPinUpstream->QueryPinInfo (&PinInfoUpstream);
        if(FAILED(hr))
        {
            ErrorMessageBox(TEXT("Cannot Obtain Upstream Filter's PIN_INFO\n"));
            return hr;
        }

        CComPtr <IPin>  pPinDown;
        pIPinUpstream->ConnectedTo (&pPinDown);

        // bail if pins are connected
        // otherwise check direction and connect
        if ((PINDIR_OUTPUT == PinInfoUpstream.dir) && (pPinDown == NULL))
        {
            // grab downstream filter's enumerator
            CComPtr <IEnumPins> pIEnumPinsDownstream;
            hr = pFilterDownstream->EnumPins (&pIEnumPinsDownstream);

            // iterate through downstream filter's pins
            CComPtr <IPin>  pIPinDownstream;
            while (pIEnumPinsDownstream->Next (1, &pIPinDownstream, 0) == S_OK)
            {
                // make sure it is an input pin
                hr = pIPinDownstream->QueryPinInfo(&PinInfoDownstream);
                if(SUCCEEDED(hr))
                {
                    CComPtr <IPin>  pPinUp;
                    pIPinDownstream->ConnectedTo (&pPinUp);

                    if ((PINDIR_INPUT == PinInfoDownstream.dir) && (pPinUp == NULL))
                    {
                        if (SUCCEEDED (m_pFilterGraph->Connect(
                                        pIPinUpstream,
                                        pIPinDownstream))
                                        )
                        {
                            PinInfoDownstream.pFilter->Release();
                            PinInfoUpstream.pFilter->Release();
                            return S_OK;
                        }
                    }
                }

                PinInfoDownstream.pFilter->Release();
                pIPinDownstream = NULL;
            } // while next downstream filter pin

            //We are now back into the upstream pin loop
        } // if output pin

        pIPinUpstream = NULL;
        PinInfoUpstream.pFilter->Release();
    } // while next upstream filter pin

    return E_FAIL;
}


// RunGraph checks to see if a graph has been built
// if not it calls BuildGraph
// RunGraph then calls MediaCtrl-Run
HRESULT
CBDAFilterGraph::RunGraph()
{
    // check to see if the graph is already running
    if(m_fGraphRunning)
        return S_OK;

    HRESULT hr = S_OK;
    if (m_pIMediaControl == NULL)
        hr = m_pFilterGraph.QueryInterface (&m_pIMediaControl);

    if (SUCCEEDED (hr))
    {
        // run the graph
        hr = m_pIMediaControl->Run();
        if(SUCCEEDED(hr))
        {
            m_fGraphRunning = true;
        }
        else
        {
            // stop parts of the graph that ran
            m_pIMediaControl->Stop();
            ErrorMessageBox(TEXT("Cannot run graph\n"));
        }
    }

    return hr;
}


// StopGraph calls MediaCtrl - Stop
HRESULT
CBDAFilterGraph::StopGraph()
{
    // check to see if the graph is already stopped
    if(m_fGraphRunning == false)
        return S_OK;

    HRESULT hr = S_OK;

    ASSERT (m_pIMediaControl);
    // pause before stopping
    hr = m_pIMediaControl->Pause();

    // stop the graph
    hr = m_pIMediaControl->Stop();

    m_fGraphRunning = (FAILED (hr))?true:false;
    return hr;
}

// Set our client area for viewing
//
// Note, what you're not seeing here is a call to
// IAMSreamCconfig's GetFormat to obtain the video
// format properties that would enable us to set
// the viewing window's size
HRESULT
CBDAFilterGraph::SetVideoWindow(
        HWND hwndMain
        )
{
    CComPtr <IVideoWindow>  pVideoWindow;
    RECT                    rc;
    INT                     cyBorder;
    INT                     cy;
    HRESULT                 hr = S_OK;

    // get IVideoWindow interface
    hr = m_pFilterGraph->QueryInterface (&pVideoWindow);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("QueryInterface IVideoWindow Failed\n"));
        return hr;
    }

    hr = pVideoWindow->put_Owner (reinterpret_cast <LONG> (hwndMain));
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Unable to set video window\n"));
        return hr;
    }

    hr = pVideoWindow->put_WindowStyle (WS_CHILD);
    if (FAILED (hr))
    {
        ErrorMessageBox(TEXT("Unable to set the style for the video window\n"));
        return hr;
    }

    GetClientRect(hwndMain, &rc);
    cyBorder = GetSystemMetrics(SM_CYBORDER);
    cy = cyBorder;
    rc.bottom -= cy;
    hr = pVideoWindow->SetWindowPosition(
                            0, 
                            0, 
                            rc.right, 
                            rc.bottom
                            );
    hr = pVideoWindow->put_Visible (OATRUE);

    return hr;
}

HRESULT
CBDAFilterGraph::ChangeChannel(
        LONG lMajorChannel, 
        LONG lMinorChannel
        )
{
    HRESULT hr = S_OK;
    m_lMajorChannel = lMajorChannel;
    m_lMinorChannel = lMinorChannel;

    CComPtr <IScanningTuner> pIScanningTuner;

    if (!m_pNetworkProvider)
    {
        ErrorMessageBox(TEXT("The FilterGraph is not yet built.\n"));
        return E_FAIL;
    }

    hr = m_pNetworkProvider.QueryInterface (&pIScanningTuner);
    if (FAILED(hr))
    {
        ErrorMessageBox(TEXT("Cannot QI for IScanningTuner\n"));
        return hr;
    }


    // create tune request
    CComPtr <IATSCChannelTuneRequest> pTuneRequest;
    hr = CreateATSCTuneRequest(
        m_lPhysicalChannel, 
        lMajorChannel, 
        lMinorChannel,
        &pTuneRequest
        );

    if(SUCCEEDED(hr))
    {
        hr = m_pITuner->put_TuneRequest (pTuneRequest);
        if (FAILED (hr))
            ErrorMessageBox(TEXT("Cannot submit tune request\n"));
    }
    else
    {
        ErrorMessageBox(TEXT("Cannot Change Channels\n"));
    }

    return hr;
}


//
// USE THE CODE BELOW IF YOU WANT TO MANUALLY LOAD AND
// CONNECT A/V DECODERS TO THE DEMUX OUTPUT PINS
//

/*
To use this code:
1) in LoadAudioDecoder() and LoadVideoDecoder(), fill in decoder specific information (clsid)
2) goto BuildGraph() and replace RenderDemux() with BuildAVSegment()
*/

/*
// Builds the Audio, Video segment of the digital TV graph.
// Demux -> AV Decoder -> OVMixer -> Video Renderer
HRESULT
CBDAFilterGraph::BuildAVSegment()
{
    HRESULT hr = E_FAIL;

    // connect the demux to the capture device
    hr = ConnectFilters(m_pCaptureDevice, m_pDemux);

    hr = LoadVideoDecoder();

    if(SUCCEEDED(hr) && m_pVideoDecoder)
    {
        // Connect the demux & video decoder
        hr = ConnectFilters(m_pDemux, m_pVideoDecoder);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting Demux & Video Decoder Failed\n");
            goto err;
        }
    }
    else
    {
        //ErrorMessageBox("Unable to load Video Decoder\n");
        goto err;
    }

    //Audio
    hr = LoadAudioDecoder();

    if(SUCCEEDED(hr) && m_pAudioDecoder)
    {
        hr = ConnectFilters(m_pDemux, m_pAudioDecoder);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting Deumx & Audio Decoder Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Unable to load Audio Decoder\n");
        goto err;
    }

    // Create the OVMixer & Video Renderer for the video segment
    hr = CoCreateInstance(CLSID_OverlayMixer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pOVMixer));

    if(SUCCEEDED(hr) && m_pOVMixer)
    {
        hr = m_pFilterGraph->AddFilter(m_pOVMixer, L"OVMixer");

        if(FAILED(hr))
        {
            ErrorMessageBox("Adding OVMixer to the FilterGraph Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Loading OVMixer Failed\n");
        goto err;
    }

    hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER,
            IID_IBaseFilter, reinterpret_cast<void**>(&m_pVRenderer));

    if(SUCCEEDED(hr) && m_pVRenderer)
    {
        hr = m_pFilterGraph->AddFilter(m_pVRenderer, L"Video Renderer");

        if(FAILED(hr))
        {
            ErrorMessageBox("Adding Video Renderer to the FilterGraph Failed\n");
            goto err;
        }
    }
    else
    {
        ErrorMessageBox("Loading Video Renderer Failed\n");
        goto err;
    }

    // Split AV Decoder? Then add Default DirectSound Renderer to the filtergraph
    if(m_pVideoDecoder != m_pAudioDecoder)
    {
        hr = CoCreateInstance(CLSID_DSoundRender, NULL,
                        CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                        reinterpret_cast<void**>(&m_pDDSRenderer));

        if(SUCCEEDED(hr) && m_pDDSRenderer)
        {
            hr = m_pFilterGraph->AddFilter(m_pDDSRenderer, L"Sound Renderer");

            if(FAILED(hr))
            {
                ErrorMessageBox("Adding DirectSound Device to the FilterGraph Failed\n");
                goto err;
            }
        }
        else
        {
            ErrorMessageBox("Loading DirectSound Device Failed\n");
            goto err;
        }
    }

    hr = ConnectFilters(m_pVideoDecoder, m_pOVMixer);

    if(FAILED(hr))
    {
        ErrorMessageBox("Connecting Capture & OVMixer Failed\n");
        goto err;
    }

    hr = ConnectFilters(m_pOVMixer, m_pVRenderer);

    if(FAILED(hr))
    {
        ErrorMessageBox("Connecting OVMixer & Video Renderer Failed\n");
        goto err;
    }

    // Split AV Decoder & if you need audio too ?? then connect Audio decoder to Sound Renderer
    if(m_pVideoDecoder != m_pAudioDecoder)
    {
        hr = ConnectFilters(m_pAudioDecoder, m_pDDSRenderer);

        if(FAILED(hr))
        {
            ErrorMessageBox("Connecting AudioDecoder & DirectSound Device Failed\n");
            goto err;
        }
    }

err:
    return hr;
}

// placeholders for real decoders
DEFINE_GUID(CLSID_FILL_IN_NAME_AUDIO_DECODER, 0xFEEDFEED, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00);
DEFINE_GUID(CLSID_FILL_IN_NAME_VIDEO_DECODER, 0xFEEDFEED, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00);

HRESULT
CBDAFilterGraph::LoadVideoDecoder()
{
    HRESULT hr = E_FAIL;

    hr = CoCreateInstance(CLSID_FILL_IN_NAME_VIDEO_DECODER, NULL,
            CLSCTX_INPROC_SERVER, IID_IBaseFilter,
            reinterpret_cast<void**>(&m_pVideoDecoder));

    if(SUCCEEDED(hr) && m_pVideoDecoder)
    {
        hr = m_pFilterGraph->AddFilter(m_pVideoDecoder, L"Video Decoder");

        if(FAILED(hr))
        {
            ErrorMessageBox("Unable to add Video Decoder filter to graph\n");
        }
    }

    return hr;
}


HRESULT
CBDAFilterGraph::LoadAudioDecoder()
{
    HRESULT hr = E_FAIL;

    hr = CoCreateInstance(CLSID_FILL_IN_NAME_AUDIO_DECODER, NULL,
            CLSCTX_INPROC_SERVER, IID_IBaseFilter,
            reinterpret_cast<void**>(&m_pAudioDecoder));

    if(SUCCEEDED(hr) && m_pAudioDecoder)
    {
        hr = m_pFilterGraph->AddFilter(m_pAudioDecoder, L"Audio Decoder");

        if(FAILED(hr))
        {
            ErrorMessageBox("Unable to add Audio filter to graph\n");
        }
    }

    return hr;
}

*/

// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT CBDAFilterGraph::AddGraphToRot(
        IUnknown *pUnkGraph, 
        DWORD *pdwRegister
        ) 
{
    CComPtr <IMoniker>              pMoniker;
    CComPtr <IRunningObjectTable>   pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
    return hr;
}

// Removes a filter graph from the Running Object Table
void CBDAFilterGraph::RemoveGraphFromRot(
        DWORD pdwRegister
        )
{
    CComPtr <IRunningObjectTable> pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
        pROT->Revoke(pdwRegister);

}


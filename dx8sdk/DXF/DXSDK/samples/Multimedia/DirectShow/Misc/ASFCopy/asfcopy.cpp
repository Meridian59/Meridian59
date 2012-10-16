//------------------------------------------------------------------------------
// File: ASFCopy.cpp
//
// Desc: DirectShow sample code - ASF copy.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>

// Disable warning C4268, which is generated within <wmsdk.h>
#pragma warning(disable:4268)
#include <wmsdk.h>
#pragma warning(default:4268)

#include <atlbase.h>
#include <atlimpl.cpp>
#include <stdio.h>

#include <dshowasf.h>

//
// Build warning to remind developers of the dependency on the 
// Windows Media Format SDK libraries, which do not ship with
// the DirectX SDK.
//
#pragma message("NOTE: To link and run this sample, you must install the Windows Media Format SDK.")
#pragma message("After signing a license agreement with Microsoft, you will receive a")
#pragma message("unique version of WMStub.LIB, which should be added to this VC++ project.")
#pragma message("Without this library, you will receive linker errors for the following:")
#pragma message("       WMCreateCertificate")
#pragma message("You must also add WMVCore.LIB to the linker settings to resolve the following:")
#pragma message("       WMCreateProfileManager")

// Global data
BOOL fVerbose = FALSE;

// Function prototypes
HRESULT MapProfileIdToProfile(int iProfile, IWMProfile **ppProfile);



class CKeyProvider : public IServiceProvider
{
    public:
        //
        // IUnknown interface
        //
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

        CKeyProvider();

        // IServiceProvider
        STDMETHODIMP QueryService(REFIID siid, REFIID riid, void **ppv);

    private:
        ULONG m_cRef;
};

CKeyProvider::CKeyProvider() : m_cRef(0)
{
}

//////////////////////////////////////////////////////////////////////////
//
// IUnknown methods
//
//////////////////////////////////////////////////////////////////////////

ULONG CKeyProvider::AddRef()
{
    return ++m_cRef;
}

ULONG CKeyProvider::Release()
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if(m_cRef == 0)
    {
        delete this;

        // don't return m_cRef, because the object doesn't exist anymore
        return((ULONG) 0);
    }

    return(m_cRef);
}

//
// QueryInterface
//
// We only support IUnknown and IServiceProvider
//
HRESULT CKeyProvider::QueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IServiceProvider || riid == IID_IUnknown)
    {
        *ppv = (void *) static_cast<IServiceProvider *>(this);
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP CKeyProvider::QueryService(REFIID siid, REFIID riid, void **ppv)
{
    if(siid == __uuidof(IWMReader) && riid == IID_IUnknown)
    {
        IUnknown *punkCert;

        HRESULT hr = WMCreateCertificate(&punkCert);

        if(SUCCEEDED(hr))
            *ppv = (void *) punkCert;
        else
            printf("CKeyProvider::QueryService failed to create certificate!  hr=0x%x\n", hr);
    
        return hr;
    }

    return E_NOINTERFACE;
}



HRESULT FindPinOnFilter( IBaseFilter * pFilter, PIN_DIRECTION PinDir,
						DWORD dwPin, BOOL fConnected, IPin ** ppPin )
{
	HRESULT			hr = S_OK;
	IEnumPins *		pEnumPin = NULL;
	IPin *			pConnectedPin = NULL;
	PIN_DIRECTION	PinDirection;
	ULONG			ulFetched;
	DWORD			nFound = 0;

	ASSERT( pFilter != NULL );
	*ppPin = NULL;

	hr = pFilter->EnumPins( &pEnumPin );
	if(SUCCEEDED(hr))
	{
		while ( S_OK == ( hr = pEnumPin->Next( 1L, ppPin, &ulFetched ) ) )
		{
			hr = (*ppPin)->ConnectedTo( &pConnectedPin );
			if (pConnectedPin)
			{
				pConnectedPin->Release();
				pConnectedPin = NULL;
			}
			if ( ( ( VFW_E_NOT_CONNECTED == hr ) && !fConnected ) ||
				 ( ( S_OK                == hr ) &&  fConnected ) )
			{
				hr = (*ppPin)->QueryDirection( &PinDirection );
				if ( ( S_OK == hr ) && ( PinDirection == PinDir ) )
				{
					if ( nFound == dwPin ) break;
					nFound++;
				}
			}
			(*ppPin)->Release();
		}
	}
	pEnumPin->Release();
	return hr;
} // FindPinOnFilter


HRESULT GetPin(IBaseFilter *pFilter, DWORD dwPin, IPin **ppPin)
{
    IEnumPins *pins;

    *ppPin = NULL;
    HRESULT hr = pFilter->EnumPins(&pins);
    if(FAILED(hr))
    {
        DbgLog((LOG_ERROR,1,TEXT("EnumPins failed!  (%x)\n"), hr));
        return hr;
    }

    if(dwPin > 0)
    {
        hr = pins->Skip(dwPin);
        if(FAILED(hr))
        {
            DbgLog((LOG_ERROR,1,TEXT("Skip(%d) failed!  (%x)\n"), dwPin, hr));
            pins->Release();
            return hr;
        }

        if(hr == S_FALSE)
        {
            DbgLog((LOG_ERROR,1,TEXT("Skip(%d) ran out of pins!\n"), dwPin));
            pins->Release();
            return hr;
        }
    }

    DWORD n;
    hr = pins->Next(1, ppPin, &n);

    if(FAILED(hr))
    {
        DbgLog((LOG_ERROR,1,TEXT("Next() failed!  (%x)\n"), hr));
    }

    if(hr == S_FALSE)
    {
        DbgLog((LOG_ERROR,1,TEXT("Next() ran out of pins!  \n")));
        pins->Release();
        return hr;
    }

    pins->Release();
    return hr;
}


void ListProfiles()
{
    USES_CONVERSION;

    int wextent = 0, Loop = 0;
    DWORD cProfiles = 0;
    DWORD cchName, cchDescription;
    CComPtr <IWMProfileManager> pIWMProfileManager;

    printf("Standard system profiles:\n");

    HRESULT hr = WMCreateProfileManager(&pIWMProfileManager);
    if(FAILED(hr))
    {
        printf("ListProfiles: Failed to create profile manager!  hr=0x%x\n", hr);
        return; // error
    }

    CComQIPtr<IWMProfileManager2, &IID_IWMProfileManager2> pIPM2(pIWMProfileManager);
    if(!pIPM2) 
    {
        printf("ListProfiles: Failed to QI IWMProfileManager2!  hr=0x%x\n", hr);
        return;
    }

    // we only use 7_0 profiles
    hr = pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
    if(FAILED(hr)) 
    {
        printf("ListProfiles: Failed to set system profile version!  hr=0x%x\n", hr);
        return;
    }

    hr = pIWMProfileManager->GetSystemProfileCount(&cProfiles);
    if(FAILED(hr))
    {
        printf("ListProfiles: Failed to get system profile count!  hr=0x%x\n", hr);
        return;
    }

    // Load the profile strings
    for(int i = 0; i < (int)cProfiles; ++i)
    {
        CComPtr <IWMProfile> pIWMProfile;

        hr = pIWMProfileManager->LoadSystemProfile(i, &pIWMProfile);
        if(FAILED(hr))
        {
            printf("ListProfiles: Failed to load system profile!  hr=0x%x\n", hr);
            return;
        }

        // How large is the profile name?
        hr = pIWMProfile->GetName(NULL, &cchName);
        if(FAILED(hr))
        {
            printf("ListProfiles: Failed to read profile name size!  hr=0x%x\n", hr);
            return;
        }

        WCHAR *wszProfile = new WCHAR[ cchName + 1 ];
        if(NULL == wszProfile)
            return;

        hr = pIWMProfile->GetName(wszProfile, &cchName);
        if(FAILED(hr))
        {
            printf("ListProfiles: Failed to read profile name!  hr=0x%x\n", hr);
            return;
        }

        // How large is the description?
        hr = pIWMProfile->GetDescription(NULL, &cchDescription);
        if(FAILED(hr))
        {
            printf("ListProfiles: Failed to read profile description size!  hr=0x%x\n", hr);
            return;
        }

        WCHAR *wszDescription = new WCHAR[ cchDescription + 1 ];
        if(NULL == wszDescription)
            return;

        hr = pIWMProfile->GetDescription(wszDescription, &cchDescription);
        if(FAILED(hr))
        {
            printf("ListProfiles: Failed to read profile description!  hr=0x%x\n", hr);
            return;
        }

        // Display the profile name and description
        if (fVerbose)
            printf("  %3d:  %ls \n[%ls]\n\n", i, wszProfile, wszDescription);
        else
            printf("  %3d:  %ls\n", i, wszProfile);

        delete[] wszProfile;
        delete[] wszDescription;
    }
}


//=======================
// CreateFilterGraph
//=======================

HRESULT CreateFilterGraph(IGraphBuilder **pGraph)
{
    HRESULT hr;

    hr = CoCreateInstance(CLSID_FilterGraph, // get the graph object
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void **) pGraph);

    if(FAILED(hr))
    {
        printf("CreateFilterGraph: Failed to create graph!  hr=0x%x\n", hr);
        *pGraph = NULL;
        return hr;
    }

    return S_OK;
}


HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

    hr = CoCreateInstance(clsid,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void **) ppFilter);

    if(FAILED(hr))
    {
        printf("CreateFilter: Failed to create filter!  hr=0x%x\n", hr);
        *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}


HRESULT SetNoClock(IFilterGraph *graph)
{
    // Keep a useless clock from being instantiated....
    IMediaFilter *graphF;
    HRESULT hr = graph->QueryInterface(IID_IMediaFilter, (void **) &graphF);

    if(SUCCEEDED(hr))
    {
        hr = graphF->SetSyncSource(NULL);
        if (FAILED(hr))
            printf("SetNoClock: Failed to set sync source!  hr=0x%x\n", hr);

        graphF->Release();
    }
    else
    {
        printf("SetNoClock: Failed to QI for media filter!  hr=0x%x\n", hr);
    }

    return hr;
}


HRESULT MapProfileIdToProfile(int iProfile, IWMProfile **ppProfile)
{
    DWORD cProfiles;

    *ppProfile = 0;
    
    CComPtr <IWMProfileManager> pIWMProfileManager;
    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if(FAILED(hr)) 
    {
        printf("MapProfile: Failed to create profile manager!  hr=0x%x\n", hr);
        return hr;
    }

    // We only use 7_0 profiles
    CComQIPtr<IWMProfileManager2, &IID_IWMProfileManager2> pIPM2(pIWMProfileManager);
    if(!pIPM2) 
    {
        printf("MapProfile: Failed to QI IWMProfileManager2!\n");
        return E_UNEXPECTED;
    }

    hr = pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
    if(FAILED(hr))
    {
        printf("MapProfile: Failed to set system profile version!  hr=0x%x\n", hr);
        return hr;
    }

    hr = pIWMProfileManager->GetSystemProfileCount( &cProfiles );
    if(FAILED(hr))
    {
        printf("MapProfile: Failed to get system profile count!  hr=0x%x\n", hr);
        return hr;
    }

    // Invalid profile requested?
    if( (DWORD)iProfile >= cProfiles ) 
    {
        printf("Invalid profile: %d\n", iProfile);
        return E_INVALIDARG;
    }

    return (pIWMProfileManager->LoadSystemProfile( iProfile, ppProfile ));
}


void WaitForCompletion( IGraphBuilder *pGraph )
{
    HRESULT hr;
    LONG lEvCode = 0;
    IMediaEvent *pEvent;

    pGraph->QueryInterface(IID_IMediaEvent, (void **) &pEvent);

    printf("Waiting for completion...\n  This could take several minutes, "
           "depending on file size and selected profile.\n");
    do
    {
        MSG Message;

        while(PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        hr = pEvent->WaitForCompletion(10, &lEvCode);

    } while(lEvCode == 0);

    pEvent->Release();
}


HRESULT CopyASF(int argc, char *argv[])
{
    HRESULT hr;    
    WCHAR SourceFile[256], TargetFile[256];
    BOOL fListProfiles = TRUE;
    DWORD dwProfile=0;
    int i = 1;

    // Parse command line options
    while(i < argc && (argv[i][0] == '-' || argv[i][0] == '/'))
    {
        // options
        if(lstrcmpiA(argv[i] + 1, "v") == 0)
        {
            fVerbose = TRUE;
            printf("Verbose mode enabled.\n");
        }
        else if((i+1 < argc) && lstrcmpiA(argv[i] + 1, "p") == 0)
        {
            fListProfiles = FALSE;
            dwProfile = atoiA(argv[i+1]);
            i++;  // skip two args here
        } 

        i++;
    }

    // List profiles only?
    if(fListProfiles)
    {
        printf("Usage: asfcopy [/v] /p profnum file1 [ file2 ...] target\n\n");

        HRESULT hr = CoInitialize(NULL);
        ListProfiles();
        CoUninitialize();
        return -1;
    }

    // Fail with usage information if improper number of arguments
    if(argc < i+2)
    {
        printf("Usage: asfcopy [/v] /p profnum file1 [ file2 ...] target\n");
        return -1;
    }


    CComPtr <IGraphBuilder> pGraph;
    CComPtr <IObjectWithSite> pObjectWithSite;
    CComPtr <IBaseFilter> pMux;
    CComPtr <IBaseFilter> pWriter;
    CComPtr <IFileSinkFilter> pFS;
    CComPtr <IConfigInterleaving> pConfigInterleaving;
    CComPtr <IConfigAsfWriter> pConfigAsfWriter;
    CComPtr <IMediaControl> pGraphC;

    // Convert target filename
    MultiByteToWideChar(CP_ACP, 0, argv[argc - 1], -1, TargetFile, 256);

    hr = CreateFilterGraph(&pGraph);
    if(FAILED(hr))
    {
        printf("Couldn't create filter graph! hr=0x%x", hr);
        return hr;
    }

    CKeyProvider prov;
    prov.AddRef();  // Don't let COM try to free our static object

    // Give the graph a pointer to us for callbacks & QueryService
    hr = pGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
    if(SUCCEEDED(hr))
    {
        hr = pObjectWithSite->SetSite((IUnknown *) (IServiceProvider *) &prov);
		if(FAILED(hr))
		{
			printf("Failed to set service provider!  hr=0x%x\n", hr);
			return hr;
		}
    }

    hr = CreateFilter(CLSID_WMAsfWriter, &pMux);
    if(FAILED(hr))
	{
        printf("Failed to create WMAsfWriter filter!  hr=0x%x\n", hr);
		return hr;
	}

    hr = pMux->QueryInterface(IID_IFileSinkFilter, (void **) &pFS);
    if(FAILED(hr))
    {
        // We need a writer also
        hr = CreateFilter(CLSID_FileWriter, &pWriter);
        if(FAILED(hr))
        {
            printf("Failed to create FileWriter filter!  hr=0x%x\n", hr);
			return hr;
        }
        else
        {
            hr = pWriter->QueryInterface(IID_IFileSinkFilter, (void **) &pFS);
            if(FAILED(hr))
            {
                printf("Failed to create QI IFileSinkFilter!  hr=0x%x\n", hr);
				return hr;
            }
        }
    }

    hr = pFS->SetFileName(TargetFile, NULL);
    if(FAILED(hr))
	{
        printf("Failed to set target filename!  hr=0x%x\n", hr);
        return hr;
    }

    hr = pGraph->AddFilter(pMux, L"Mux");
    if(FAILED(hr))
	{
        printf("Failed to add Mux filter to graph!  hr=0x%x\n", hr);
		return hr;
	}

    // Set interleaving mode to FULL
    // !!! ASF won't support this, but that's okay
    hr = pMux->QueryInterface(IID_IConfigInterleaving, (void **) &pConfigInterleaving);
    if(SUCCEEDED(hr))
    {
        printf("Setting interleaving mode to INTERLEAVE_FULL\r\n");
        hr = pConfigInterleaving->put_Mode(INTERLEAVE_FULL);
    }

    // !!! We should only require a profile if we're using a filter which needs it
    hr = pMux->QueryInterface(IID_IConfigAsfWriter, (void **) &pConfigAsfWriter);
    if(SUCCEEDED(hr))
    {
        if (fVerbose)
            printf("Setting profile to %d\r\n", dwProfile);

        CComPtr<IWMProfile> pProfile;

        hr = MapProfileIdToProfile(dwProfile, &pProfile);
        if(FAILED(hr)) {
            printf("Failed to map profile ID!  hr=0x%x\n", hr);
            return hr;
        }

        // Note that the ASF writer will not run if the number of streams
        // does not match the profile.
        hr = pConfigAsfWriter->ConfigureFilterUsingProfile(pProfile);
        if(FAILED(hr)) {
            printf("Failed to configure filter to use profile!  hr=0x%x\n", hr);
            return hr;
        }       
    }
    else
	{
        printf("Failed to QI for IConfigAsfWriter!  hr=0x%x\n", hr);
		return hr;
	}

    // Connect writer filter if needed
    if(pWriter)
    {
        IPin *pMuxOut, *pWriterIn;
        hr = pGraph->AddFilter(pWriter, L"Writer");
	    if(FAILED(hr))
		{
			printf("Failed to add FileWriter filter to graph!  hr=0x%x\n", hr);
			return hr;
		}

		// Look for the first unconnected output pin
        hr = FindPinOnFilter(pMux, PINDIR_OUTPUT, 0, FALSE, &pMuxOut);
		if(FAILED(hr))
		{
			printf("Failed to find output pin on Mux!  hr=0x%x\n", hr);
			return hr;
		}
		// Find the first connected pin
        hr = FindPinOnFilter(pWriter, PINDIR_INPUT, 0, FALSE, &pWriterIn);
		if(FAILED(hr))
		{
			printf("Failed to find input pin on FileWriter!  hr=0x%x\n", hr);
			pMuxOut->Release();
			return hr;
		}

        hr = pGraph->ConnectDirect(pMuxOut, pWriterIn, NULL);
        pMuxOut->Release(); 
        pWriterIn->Release();
		if(FAILED(hr))
		{
			printf("Failed to connect Mux to FileWriter!  hr=0x%x\n", hr);
			return hr;
		}

        if(fVerbose)
            printf("Connected Mux and writer, hr = 0x%x\n", hr);
    }

    // Set sync source to NULL to speed processing
    SetNoClock(pGraph);

    // Render all source files listed on the command line
    while(i < argc - 1)
    {
        MultiByteToWideChar(CP_ACP, 0, argv[i], -1,
            SourceFile, 256);

        printf("Copying %ls to %ls\n", SourceFile, TargetFile);

        hr = pGraph->RenderFile(SourceFile, NULL);
        if(FAILED(hr))
            printf("Failed to render source file %s!  hr=0x%x\n", argv[i], hr);
        else if (fVerbose)
            printf("RenderFile('%ls') returned hr=0x%x\n", SourceFile, hr);

        ++i;
    }

    // Run the graph
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pGraphC);
    if(FAILED(hr))
	{
        printf("Failed to QI for IMediaControl!  hr=0x%x\n", hr);
		return hr;
	}

    hr = pGraphC->Run();
    if(FAILED(hr))
    {
        printf("Failed to run the graph!  hr=0x%x\nCopy aborted.\n\n", hr);
        printf("Please check that you have selected the correct profile for copying.\n"
               "Note that if your source ASF file is audio-only, then selecting a\n"
               "video profile will cause a failure when running the graph.\n\n");
        ListProfiles();
    }
    else
    {
        WaitForCompletion(pGraph);
        printf("Copy complete.\n");

        // Stop the graph
        hr = pGraphC->Stop();
    }

    return hr;
}


int __cdecl
main(
    int argc,
    char *argv[]
    )
{
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);

    // Since COM smart pointers are used, the main functionality is wrapped
    // in CopyASF().  When the function returns, the smart pointers will clean
    // up properly, and then we'll uninitialize COM.
    hr = CopyASF(argc, argv);

    CoUninitialize();
    return hr;
}


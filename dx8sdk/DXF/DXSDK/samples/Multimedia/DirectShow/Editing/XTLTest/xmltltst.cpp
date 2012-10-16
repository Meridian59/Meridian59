//------------------------------------------------------------------------------
// File: XMLTlTst.cpp
//
// Desc: DirectShow sample code - test utility for building timelines from
//       .XTL files.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
// This sample demonstrates the following:
// 
// - Using IXml2Dex to load timelines from and save to .XTL files 
// - Adding Windows Media (ASF) support to Editing applications
// - Using IAMErrorLog to display error messages
// - Preview of DES timelines (IAMTimeLine, IRenderEngine)
// - Traversing the timeline nodes
//

#include <windows.h>
#include <streams.h>
#include <stdio.h>
#include <atlbase.h>
#include <qedit.h>

extern CComModule _Module;
#include <atlimpl.cpp>

//
// NOTE:
//
// In order to write ASF files using this program, you need to obtain
// a Windows Media Format SDK Certificate (WMStub.lib), link to it, 
// and define USE_WMF_CERT below. 
//
// See the Windows Media Format SDK documentation for more information.
//
// The SDK download page is located at 
// http://msdn.microsoft.com/workshop/imedia/windowsmedia/sdk/wmsdk.asp, 
// with links to the SDK itself and information for obtaining a certificate.
//
// #define USE_WMF_CERT
// 

#ifdef USE_WMF_CERT

#include <wmsdk.h>
#include <dshowasf.h>


// Note: this object is a SEMI-COM object, and can only be created statically.
class CKeyProvider : public IServiceProvider {
public:
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    // IServiceProvider
    STDMETHODIMP QueryService(REFIID siid, REFIID riid, void **ppv);
    
};

CKeyProvider g_prov; // note, must stay in scope while graph is alive

#endif // USE_WMF_CERT


class CErrorReporter : public IAMErrorLog
{
    // IAMErrorLog
    STDMETHODIMP LogError( long Severity, BSTR ErrorString, LONG ErrorCode, 
                           HRESULT hresult, VARIANT * pExtraInfo );

public:

    // SEMI COM
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == IID_IAMErrorLog || riid == IID_IUnknown) {
            *ppv = (void *) static_cast<IAMErrorLog *>(this);
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
};


//
// Declarations for helper functions defined below
//
IPin * GetPin( IBaseFilter * pFilter, int PinNum, PIN_DIRECTION pd );

void TurnOffPreviewMode( IAMTimeline * pTimeline );

HRESULT GetFirstSourceOnTimeline(
    IAMTimeline *pTimeline, GUID MajorType,
    IAMTimelineGroup ** ppGroup, IAMTimelineSrc ** ppSource );
    
HRESULT GetSourceVideoType(WCHAR *wszFilename, AM_MEDIA_TYPE *pmt);

HRESULT GetDestinationASFFormat(
    AM_MEDIA_TYPE **ppmt,
    int iProfile );

#ifdef USE_WMF_CERT
HRESULT MapProfileIdToProfile(
    int iProfile, IWMProfile **ppProfile);
#endif

void ListWMSDKProfiles();
BOOL IsAsfExtension( WCHAR * Filename );

HRESULT ConnectOutputFile(
    IRenderEngine * pEngine, WCHAR * Filename
#ifdef USE_WMF_CERT
    , int iProfile
#endif
    );


//
// Main program code
//
int __cdecl main(int argc, char *argv[])
{
    CErrorReporter pLog; // note, must stay in scope while graph is alive
    HRESULT hr = 0;

    ++argv; // skip the app name
    --argc;

    int Ret = 0;

    char **&ppArg = argv;

    BOOL fNoRender = FALSE;
    BOOL fDynamic = FALSE;
    BOOL fRecomp = FALSE;
    BOOL fWriteIt = FALSE;
    int nWriteArg = -1;
    BOOL fWriteGrf = FALSE;
    int nWriteGrfArg = -1;
    int nInputFileArg = -1;
    BOOL fWriteXtl = FALSE;
    int nWriteXtlArg = -1;
    double RenderStart = -1.0;
    double RenderStop = -1.0;
    BOOL fNoClock = FALSE;
#ifdef USE_WMF_CERT
    int iASFProfile = -1;
#endif

    // find which switches we've set. Every time we process a switch,
    // we pull it off the stack
    //
    for( int arg = 0 ; arg < argc ; arg++ )
    {
        if( !ppArg[arg] ) continue;

        // found the input .xtl file specified
        //
        if( ppArg[arg][0] != '/' )
        {
            nInputFileArg = arg;
            continue;
        }

        if (ppArg[arg][1] == 'N' || ppArg[arg][1] == 'n') {
            fNoRender = TRUE;
            continue;
        }

        if (ppArg[arg][1] == 'D' || ppArg[arg][1] == 'd') {
            fDynamic = TRUE;
            continue;
        }

        if (ppArg[arg][1] == 'C' || ppArg[arg][1] == 'c') {
            fNoClock = TRUE;
            continue;
        }

        if (ppArg[arg][1] == 'R' || ppArg[arg][1] == 'r') {
            fRecomp = TRUE;
            continue;
        }
        if (ppArg[arg][1] == 'W' || ppArg[arg][1] == 'w') {
            fWriteIt = TRUE;
            nWriteArg = arg + 1;
            arg++; // advance beyond the extra arg
            continue;
        }
        if (ppArg[arg][1] == 'G' || ppArg[arg][1] == 'g') {
            fWriteGrf = TRUE;
            nWriteGrfArg = arg + 1;
            arg++; // advance beyond the extra arg
            continue;
        }
        if (ppArg[arg][1] == 'X' || ppArg[arg][1] == 'x') {
            fWriteXtl = TRUE;
            nWriteXtlArg = arg + 1;
            arg++; // advance beyond the extra arg
            continue;
        }

#ifdef USE_WMF_CERT        
        if (ppArg[arg][1] == 'P' || ppArg[arg][1] == 'p') {
            arg++;
            if (arg >= argc || (ppArg[arg][0] < '0' || ppArg[arg][0] > '9')) {
                ListWMSDKProfiles();
                return -1;
            }

            iASFProfile = atoiA(ppArg[arg]);
            printf("Using WMSDK profile #%d\r\n", iASFProfile);
            continue;
        }
#endif // USE_WMF_CERT
        
        if( ppArg[arg][1] == '[' ) {
            // need to pull doubles out of the string
            //
            char * p = &ppArg[arg][2];
            for( unsigned long k = 0 ; k < strlen( p ) ; k++ )
            {
                if( p[k] == '-' )
                {
                    break;
                }
            }
            if( p[k] != '-' )
            {
                printf( "forgot the '-' between doubles\r\n" );
                return -1;
            }
            for( unsigned long j = 0 ; j < strlen( p ) ; j++ )
            {
                if( p[j] == ']' )
                {
                    break;
                }
            }
            if( p[j] != ']' )
            {
                printf( "forgot the ']' \r\n" );
                return -1;
            }
            
            // set it to zero so atof will work
            //
            p[k] = 0;
            p[j] = 0;

            // get the float
            //
            RenderStart = atof( p );

            // get the next float
            //
            p = &p[k+1];

            RenderStop = atof( p );
            continue;
        }


        printf("unrecognized switch: %s\n\n", ppArg[arg]);
        nInputFileArg = -1;
        break;
    }
    
    if( argc < 1 || nInputFileArg == -1 ) {
        printf("Usage:     [various switches] input.xtl\r\n");
        printf("           /N  - No preview, just connect it up\r\n");
        printf("           /G output.grf - Output a GRF file\r\n");
        printf("           /X output.xtl - Output an XTL file\r\n");
        printf("           /R  - Connect the graph with smart recompression turned on\r\n");
        printf("           /W <filename> - Render the clip to file. (It may take a while)\r\n" );
        printf("           /D  - Dynamic connections on\r\n");
        printf("           /[double-double] - set the render range start/stop times\r\n" );
        printf("           /C  - No clock. Run as fast as you can\r\n" );
#ifdef USE_WMF_CERT        
        printf("           /P [number] - Choose an ASF compression profile\r\n" );
        printf("           /P without a number - list available profiles\r\n" );
#endif // USE_WMF_CERT
        return -1;
    }

    USES_CONVERSION;
    WCHAR *wszFileInput = A2W(ppArg[nInputFileArg]);

    CComPtr< IRenderEngine > pRenderEngine;
    CComPtr< IAMTimeline > pTimeline;
    CComPtr< IXml2Dex > pXml;


    do
    {
        hr = CoInitialize( NULL );

        hr = CoCreateInstance( 
            __uuidof(AMTimeline), 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(IAMTimeline), 
            (void**) &pTimeline
            );

        if (FAILED(hr)) {
            printf("Failed to create timeline, error = %x\r\n", hr);
            Ret = -1;
            break;
        }

        CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pTimelineLog( pTimeline );
        if( pTimelineLog )
        {
            pTimelineLog->put_ErrorLog( &pLog );
        }

        hr = CoCreateInstance( 
            __uuidof(Xml2Dex), 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(IXml2Dex), 
            (void**) &pXml
            );
    
        if (FAILED(hr)) {
            printf("QEDIT not registered properly\r\n");
            Ret = -1;
            break;
        }

        
        hr = pXml->ReadXMLFile(pTimeline, wszFileInput);
        printf("ReadXMLFile('%ls') returned %x\r\n", wszFileInput, hr);
        if (FAILED(hr)) {
            Ret = -1;
            break;
        }


        // validate sources
        //
        HANDLE WaitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

        long Flags =
            SFN_VALIDATEF_CHECK |
            SFN_VALIDATEF_POPUP | 
            // SFN_VALIDATEF_TELLME |
            SFN_VALIDATEF_REPLACE |
            SFN_VALIDATEF_USELOCAL |
            0;
        
        hr = pTimeline->ValidateSourceNames( Flags, NULL, (LONG_PTR) WaitEvent );
        if( hr == NOERROR )
        {
            WaitForSingleObject( WaitEvent, INFINITE );
        }
        CloseHandle( WaitEvent );

        // create a render engine
        //
        if( fRecomp )
        {
            hr = CoCreateInstance( 
                __uuidof(SmartRenderEngine),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IRenderEngine),
                (void**) &pRenderEngine );

            TurnOffPreviewMode( pTimeline );
        
#ifdef USE_WMF_CERT
            // if we're doing smart recompression then the AMTimeline object will 
            // need a way to get our key, so we SetSite on the AMTimeline object here.
            // Note that the pRenderEngine gets the key a little later than this.
            if( SUCCEEDED( hr ) )
            {
                CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pTimeline );
                ASSERT( pOWS );
                if( pOWS )
                {            
                    pOWS->SetSite((IUnknown *) (IServiceProvider *) &g_prov);
                }                
            }

#endif // USE_WMF_CERT

            SCompFmt0 scompFmt;
            ZeroMemory(&scompFmt, sizeof(scompFmt));
            // scompFmt.nFormatId initialized to 0;
            
            // use the compression type of the first input video in
            // the project (note this can change the height/width of
            // the group)

            CComPtr<IAMTimelineGroup> pGroup;
            CComPtr<IAMTimelineSrc> pSource;
            hr = GetFirstSourceOnTimeline( pTimeline, MEDIATYPE_Video, &pGroup, &pSource);


#ifdef USE_WMF_CERT

            // smart recompression works differently for ASF than it
            // does for AVI. ASF compression needs to be configured by
            // profile id rather than compressor and format block. We
            // cannot recover the profile used to author an ASF file,
            // so the caller is required to provide us the right
            // profile id. We ask the WMSDK for the media type
            // corresponding to the profile.
            
            WCHAR *wszFileTmp = A2W(ppArg[nWriteArg]);
            if( IsAsfExtension( wszFileTmp ) )
            {
                AM_MEDIA_TYPE *pmt;
                hr = GetDestinationASFFormat(
                    &pmt, 
                    iASFProfile);
                if(SUCCEEDED(hr)) {
                    CopyMediaType(&scompFmt.MediaType, pmt);
                    DeleteMediaType(pmt);
                }
            }
            else
#endif // USE_WMF_CERT
            {
                CComBSTR Filename;
                if(SUCCEEDED(hr)) {
                    hr = pSource->GetMediaName(&Filename);
                }
                if(SUCCEEDED(hr)) {
                    hr = GetSourceVideoType(Filename, &scompFmt.MediaType);
                }
            }
            
            if(SUCCEEDED(hr)) {
                hr = pGroup->SetSmartRecompressFormat( (long*) &scompFmt );
                FreeMediaType(scompFmt.MediaType);
            }
        }
        else
        {
            hr = CoCreateInstance( 
                __uuidof(RenderEngine),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IRenderEngine),
                (void**) &pRenderEngine );

        }
        if (pRenderEngine == NULL) {
            printf("Failed to create render engine, error = %x\r\n", hr);
            Ret = -1;
            break;
        }

#ifdef USE_WMF_CERT

        //
        // give the RenderEngine a key to support ASF
        //
        {
            CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pRenderEngine );
            ASSERT( pOWS );
            if( pOWS )
            {        
                pOWS->SetSite((IUnknown *) (IServiceProvider *) &g_prov);
            }            
        }
#endif // USE_WMF_CERT

        long gRenderMedLocFlags = 0 |
            //        SFN_VALIDATEF_POPUP | 
            //        SFN_VALIDATEF_TELLME |
            //        SFN_VALIDATEF_VERBOSE | 
            //        SFN_VALIDATEF_USELOCAL |
            0;
        
        pRenderEngine->SetSourceNameValidation( NULL, NULL, gRenderMedLocFlags );

        // set it's timeline
        //
        hr = pRenderEngine->SetTimelineObject( pTimeline );
        if (FAILED(hr)) {
            printf("Failed to set timeline object, error = %x\r\n", hr);
            Ret = -1;
            break;
        }

        // set dynamicness
        //
        if (fDynamic) {
            pRenderEngine->SetDynamicReconnectLevel(CONNECTF_DYNAMIC_SOURCES);
            printf("DYNAMIC ON\r\n");
        } else {
            pRenderEngine->SetDynamicReconnectLevel(CONNECTF_DYNAMIC_NONE);
            printf("DYNAMIC OFF\r\n");
        }

        // set the renderrange, if there is one
        //
        if( RenderStart != -1.0 && RenderStop != -1.0 )
        {
            pRenderEngine->SetRenderRange2( RenderStart, RenderStop );
        }

        
       
        // connect up first half
        //
        hr = pRenderEngine->ConnectFrontEnd( );

        if( FAILED( hr ) )
        {
            printf("ConnectFrontEnd returned bomb code of %x\r\n", hr);
            Ret = -1;
            break;
        }



        // get this now, because other calls will need it
        //
        CComPtr< IGraphBuilder > pGraph;
        hr = pRenderEngine->GetFilterGraph( &pGraph );

        if (FAILED(hr)) {
            printf("Couldn't get graph!  hr = %x", hr);
            break;
        }

        // if we didn't want to write the file to disk, may as well render it
        //
        if( !fWriteIt )
        {
            hr = pRenderEngine->RenderOutputPins( );


            if (FAILED(hr)) 
            {
                printf("RenderOutputPins returned bomb code of %x\r\n", hr);
                Ret = -1;
                break;
            }

            if( fNoClock )
            {
                CComQIPtr< IMediaFilter, &IID_IMediaFilter > pMFGraph( pGraph );
                if( pMFGraph )
                {
                    pMFGraph->SetSyncSource( NULL );
                }
            }
        }
        else
        {
            TurnOffPreviewMode( pTimeline );
            WCHAR *wszFileTmp = A2W(ppArg[nWriteArg]);

            hr = ConnectOutputFile(pRenderEngine, wszFileTmp
#ifdef USE_WMF_CERT
                                   , iASFProfile
#endif
                                   );
                                   
            printf( "ConnectOutputFile returned %x\r\n", hr );
            if( FAILED( hr ) )
            {
                fNoRender = TRUE;
            }
        }

        // write out GRF files
        //
        if( fWriteGrf )
        {
            WCHAR *wszFileTmp = A2W(ppArg[nWriteGrfArg]);
            hr = pXml->WriteGrfFile(pGraph, wszFileTmp);
            printf("WriteGrfFile('%ls') returned %x\r\n", wszFileTmp, hr);
        }
        if( fWriteXtl )
        {
            WCHAR *wszFileTmp = A2W(ppArg[nWriteXtlArg]);
            hr = pXml->WriteXMLFile(pTimeline, wszFileTmp);
            printf("WriteXtlFile('%ls') returned %x\r\n", wszFileTmp, hr);
        }

        if (!fNoRender) {
            CComQIPtr< IMediaEvent, &IID_IMediaEvent > pEvent( pGraph );
            CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( pGraph );
            hr = pControl->Run( );
            if (FAILED(hr)) {
                printf("Failed to run the graph, error = %x\r\n", hr);
                Ret = -1;
                break;
            }


            HANDLE hEvent;

            if( !pEvent ) {
                // unexpected error
                Ret = -1;
            }

            hr = pEvent->GetEventHandle((OAEVENT *)&hEvent);
            if(FAILED(hr)) {
                // unexpected error
                Ret = -1;
            }

            // wait for completion and dispatch messages for any windows
            // created on our thread.
            for(;;)
            {
                while(MsgWaitForMultipleObjects(
                    1,
                    &hEvent,
                    FALSE,
                    INFINITE,
                    QS_ALLINPUT) != WAIT_OBJECT_0)
                {
                    MSG Message;

                    while (PeekMessage(&Message, NULL, 0, 0, TRUE))
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }

                // event signaled. see if we're done.

                LONG levCode;
                LONG_PTR lp1, lp2;

                if(pEvent->GetEvent(&levCode, &lp1, &lp2, 0) == S_OK)
                {
                    EXECUTE_ASSERT(SUCCEEDED(
                        pEvent->FreeEventParams(levCode, lp1, lp2)));
                
                    if(levCode == EC_COMPLETE || levCode == EC_ERRORABORT ||
                       levCode == EC_USERABORT)
                    {
                        break;
                    }
                }
            }
            
            hr = pControl->Stop( );

            
        }
    } while(0); 

    pRenderEngine.Release( );
    pXml.Release( );
    if (pTimeline)
    {
        hr = pTimeline->ClearAllGroups();
        pTimeline.Release( );
    }

    CoUninitialize( );

    
    return Ret;
}

//
// helper function to traverse a timline and return the first source
// 

HRESULT GetFirstSourceOnTimeline(
    IAMTimeline *pTimeline, GUID MajorType,
    IAMTimelineGroup ** ppGroup, IAMTimelineSrc ** ppSource )
{
    *ppGroup = 0;
    *ppSource = 0;
    
    for( long g = 0 ;; g++ )
    {
        // locate group of right media type (audio / video)
        CComPtr< IAMTimelineObj > pGroupObj;
        HRESULT hr = pTimeline->GetGroup( &pGroupObj, g );
        if(FAILED(hr)) {
            break;
        }
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );

        CMediaType GroupType;
        pGroup->GetMediaType( &GroupType );
        if( GroupType.majortype != MajorType ) {
            continue;
        }

        // found the right group, go through each track in it

        long TrackCount = 0;
        long Layers = 0;
        pTimeline->GetCountOfType( g, &TrackCount, &Layers, TIMELINE_MAJOR_TYPE_TRACK );
    
        if( TrackCount < 1 )
        {
            continue;
        }

        CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pGroupComp( pGroupObj );

        for( int CurrentLayer = 0 ; CurrentLayer < Layers ; CurrentLayer++ )
        {
            // get the layer itself
            //
            CComPtr< IAMTimelineObj > pLayer;
            hr = pGroupComp->GetRecursiveLayerOfType( &pLayer, CurrentLayer, TIMELINE_MAJOR_TYPE_TRACK );

            // ask if the layer is muted
            //
            BOOL LayerMuted = FALSE;
            pLayer->GetMuted( &LayerMuted );
            if( LayerMuted )
            {
                // don't look at this layer
                //
                continue; // skip this layer, don't worry about grid
            }

            CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack( pLayer );
            if( !pTrack )
            {
                continue;
            }

            REFERENCE_TIME InOut = 0;
            while( 1 )
            {
                // get the next source on this layer, given a time.
                //
                CComPtr< IAMTimelineObj > pSourceObj;
                hr = pTrack->GetNextSrc( &pSourceObj, &InOut );
                ASSERT( !FAILED( hr ) );
                if( hr != NOERROR )
                {
                    // all done with sources
                    //
                    break;
                }

                CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource( pSourceObj );
                ASSERT( pSource );
                if( !pSource )
                {
                    // this one failed, so look at the next
                    //
                    continue; // sources
                }

                // ask if the source is muted
                //
                BOOL SourceMuted = FALSE;
                pSourceObj->GetMuted( &SourceMuted );
                if( SourceMuted )
                {
                    // don't look at this source
                    //
                    continue; // sources
                }

                *ppSource = pSource;
                pSource.p->AddRef();
                *ppGroup = pGroup;
                pGroup.p->AddRef();

                return S_OK;

            } // while sources


        } // while currentlayer

    } // for all groups

    return VFW_E_NOT_FOUND;
}

void TurnOffPreviewMode( IAMTimeline * pTimeline )
{
    long Groups = 0;
    pTimeline->GetGroupCount( &Groups );
    for( int i = 0 ; i < Groups ; i++ )
    {
        CComPtr< IAMTimelineObj > pGroupObj;
        pTimeline->GetGroup( &pGroupObj, i );
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
        pGroup->SetPreviewMode( FALSE );
    }
}

//
// uses the media detector to retrieve mediatype of a clip
// 
HRESULT GetSourceVideoType(WCHAR *wszFilename, AM_MEDIA_TYPE *pmt)
{
    CComPtr< IMediaDet > pDet;
    HRESULT hr = CoCreateInstance( CLSID_MediaDet,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IMediaDet,
                           (void**) &pDet );
    if( FAILED( hr ) ) {
        return hr;
    }

#ifdef USE_WMF_CERT

    //
    // set the site provider on the MediaDet object to allowed keyed apps
    // to use ASF decoder
    //
    CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pDet );
    if( pOWS )
    {
        pOWS->SetSite( (IServiceProvider *) &g_prov );
    }

#endif // USE_WMF_CERT

    hr = pDet->put_Filename( wszFilename );
    if( FAILED( hr ) )
    {
        return hr;
    }

    // go through and find the video stream type
    //
    long Streams = 0;
    long VideoStream = -1;
    hr = pDet->get_OutputStreams( &Streams );
    for( int i = 0 ; i < Streams ; i++ )
    {
        pDet->put_CurrentStream( i );
        GUID Major = GUID_NULL;
        pDet->get_StreamType( &Major );
        if( Major == MEDIATYPE_Video )
        {
            VideoStream = i;
            break;
        }
    }
    if( VideoStream == -1 )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    hr = pDet->get_StreamMediaType( pmt );
    return hr;
}


#ifdef USE_WMF_CERT

// ------------------------------------------------------------------------
// CKeyProvider methods
// 

HRESULT CKeyProvider::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IServiceProvider || riid == IID_IUnknown) {
        *ppv = (void *) static_cast<IServiceProvider *>(this);
        return NOERROR;
    }    
    return E_NOINTERFACE;
}


STDMETHODIMP CKeyProvider::QueryService(REFIID siid, REFIID riid, void **ppv)
{
    if (siid == __uuidof(IWMReader) && riid == IID_IUnknown) {
    
        IUnknown *punkCert;

        HRESULT hr = WMCreateCertificate( &punkCert );
        if (SUCCEEDED(hr)) {
            *ppv = (void *) punkCert;
        }
        return hr;
    }
    return E_NOINTERFACE;
}

#endif // USE_WMF_CERT

// ------------------------------------------------------------------------
// CErrorReporter methods
// 

STDMETHODIMP CErrorReporter::LogError( long Severity, BSTR ErrorString, LONG ErrorCode, HRESULT hresult, VARIANT * pExtraInfo )
{
    USES_CONVERSION;
    printf( "Error %d (HRESULT %x)\r\n", ErrorCode, hresult);
    char * t = W2A( ErrorString );
    printf( t );
    printf( "\r\n" );

#ifdef DEBUG
    OutputDebugStringA( t );
    OutputDebugString( TEXT("\r\n") );
#endif // DEBUG

    // look at variant
    //
    if( pExtraInfo )
    {
        if( pExtraInfo->vt == VT_BSTR )
        {
            printf( "Extra Info:%ls\r\n", pExtraInfo->bstrVal);
        } else if( pExtraInfo->vt == VT_I4 ) {
            printf( "Extra Info: %d\r\n", (int)pExtraInfo->lVal );
        } else if( pExtraInfo->vt == VT_R8 ) {
            printf( "Extra Info: %d/100\r\n",
                    (int)(V_R8(pExtraInfo) * 100));
        }
    }
    return hresult;
}

#ifdef USE_WMF_CERT

// list formats Windows Media Format SDK supports.
// 
void ListWMSDKProfiles()
{
    USES_CONVERSION;
    
    int wextent = 0 ;
    int Loop = 0 ;
    SIZE extent ;
    DWORD cProfiles = 0 ;

    printf("Standard system profiles:\n");
    
    CComPtr <IWMProfileManager> pIWMProfileManager;

    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if( FAILED( hr ) ) {   
        return; // error
    }        

    IWMProfileManager2* pIPM2;
    hr = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2,
                                             ( void ** )&pIPM2 );
    if(FAILED(hr)) {
        return;
    }
    // we only use 7_0 profiles
    hr = pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
    pIPM2->Release();
    
    if(FAILED(hr)) {
        return;
    }
        
    hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
    if( FAILED( hr ) )
    {
        return;
    }
        
    //    
    // now load the profile strings
    //    
    LRESULT ix;
    DWORD cchName, cchDescription;
    for (int i = 0; i < (int)cProfiles; ++i) {
        CComPtr <IWMProfile> pIWMProfile;
        
        hr = pIWMProfileManager->LoadSystemProfile( i, &pIWMProfile );
        if( FAILED( hr ) )
            return;
            
        hr = pIWMProfile->GetName( NULL, &cchName );
        if( FAILED( hr ) )
            return;
            
        WCHAR *wszProfile = new WCHAR[ cchName ];
        if( NULL == wszProfile )
            return;
            
        hr = pIWMProfile->GetName( wszProfile, &cchName );
        if( FAILED( hr ) )
            return;
        
        hr = pIWMProfile->GetDescription( NULL, &cchDescription );
        if( FAILED( hr ) )
            return;
            
        WCHAR *wszDescription = new WCHAR[ cchDescription ];
        if( NULL == wszDescription )
            return;
            
        
        hr = pIWMProfile->GetDescription( wszDescription, &cchDescription );
        if( FAILED( hr ) )
            return;
        

        printf("  %3d:  %ls\n", i, wszProfile);
        
        delete[] wszProfile;
        delete[] wszDescription;
    }

    printf("\r\n Use /p [#] to choose the profile you want\r\n"); 
}
#endif // USE_WMF_CERT

// 
// helper function to return Nth input pin or output pin on a
// filter. pin is addref'd
// 
IPin * GetPin( IBaseFilter * pFilter, int PinNum, PIN_DIRECTION pd )
{
    CComPtr<IEnumPins> pEnum;
    HRESULT hr = pFilter->EnumPins( &pEnum );
    if(SUCCEEDED(hr))
    {
        ULONG cFetched;
        IPin * pPin;
        while(pEnum->Next( 1, &pPin, &cFetched) == S_OK)
        {
            PIN_DIRECTION pd2;
            pPin->QueryDirection( &pd2 );
            if( pd2 == pd )
            {
                if( PinNum == 0 )
                {
                    // return addref'd pin
                    return pPin;
                }
                PinNum--;
            }

            pPin->Release();
        }
    }

    return NULL;
}

//
// Configure the graph to write to an AVI file (or ASF or .WAV)
//

HRESULT ConnectOutputFile(
    IRenderEngine * pEngine, WCHAR * Filename
#ifdef USE_WMF_CERT
    , int iProfile
#endif
    )
{
    CComPtr< ICaptureGraphBuilder2 > pBuilder;
    HRESULT hr = CoCreateInstance(
        CLSID_CaptureGraphBuilder2,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICaptureGraphBuilder2,
        (void**) &pBuilder );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComPtr< IAMTimeline > pTimeline;
    hr = pEngine->GetTimelineObject( &pTimeline );
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComPtr< IGraphBuilder > pGraph;
    hr = pEngine->GetFilterGraph( &pGraph );
    if( FAILED( hr ) )
    {
        return hr;
    }

    long Groups = 0;
    pTimeline->GetGroupCount( &Groups );
    if( !Groups )
    {
        return E_INVALIDARG;
    }

    CComPtr< IBaseFilter > pMux;
    CComPtr< IFileSinkFilter > pWriter;

    hr = pBuilder->SetFiltergraph( pGraph );
    if( FAILED( hr ) )
    {
        return hr;
    }


    bool fConnectManually = false;
    GUID guid = MEDIASUBTYPE_Avi;

    // determine which writer to use based on file extension
    if (lstrlenW(Filename) > 4)
    {
        WCHAR *pExt = Filename + lstrlenW(Filename) - 3;

#ifdef USE_WMF_CERT
        if(IsAsfExtension(pExt))
        {
            guid =  CLSID_WMAsfWriter;
        }
#endif // USE_WMF_CERT
        if (lstrcmpiW(pExt, L"wav") == 0)
        {
            fConnectManually = true;
            
            // creation of the wav writer filter SDK sample
            //
            // .wav files need to be special-cased here because the
            // capture graph builder doesn't know about the wav writer
            // filter.
            // 
            // {3C78B8E2-6C4D-11d1-ADE2-0000F8754B99}
            const CLSID CLSID_WavDest = {
                0x3C78B8E2,0x6C4D,0x11D1,{0xAD,0xE2,0x00,0x00,0xF8,0x75,0x4B,0x99}
            };

            hr = CoCreateInstance(
                CLSID_WavDest,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IBaseFilter,
                (void**) &pMux );
            if( SUCCEEDED( hr ) )
            {
                hr = pGraph->AddFilter( pMux, L"Wave Mux" );
            }

            if( SUCCEEDED( hr ) )
            {
                hr = CoCreateInstance(
                    CLSID_FileWriter,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IFileSinkFilter,
                    (void**) &pWriter );
            }
            if( SUCCEEDED( hr ) )
            {
                hr = pWriter->SetFileName( Filename, NULL );
            }
            if( SUCCEEDED( hr ) )
            {
                CComQIPtr< IBaseFilter, &IID_IBaseFilter > pWriterBase( pWriter );
                hr = pGraph->AddFilter( pWriterBase, L"Writer" );
            }
        }
    }

    if(FAILED(hr)) {
        return hr;
    }

    if(!fConnectManually)
    {
        hr = pBuilder->SetOutputFileName(
            &guid, Filename, &pMux, &pWriter );
    }
    if( FAILED( hr ) )
    {
        return hr;
    }

    CComQIPtr<IConfigInterleaving, &IID_IConfigInterleaving> pConfigInterleaving(pMux);
    if(pConfigInterleaving) {
        pConfigInterleaving->put_Mode(INTERLEAVE_FULL);
    }
    CComQIPtr<IFileSinkFilter2, &IID_IFileSinkFilter2> pCfgFw(pWriter);
    if(pCfgFw) {
        pCfgFw->SetMode(AM_FILE_OVERWRITE);
    }
     
#ifdef USE_WMF_CERT
    CComQIPtr<IConfigAsfWriter, &IID_IConfigAsfWriter> pConfigAsfWriter(pMux);
    if(pConfigAsfWriter && iProfile >= 0)
    {
        CComPtr<IWMProfile> pProfile;
        hr = MapProfileIdToProfile(iProfile, &pProfile);
        if(FAILED(hr)) {
            return hr;
        }

        // note that the ASF writer will not run if the number of streams
        // does not match the profile.
        hr = pConfigAsfWriter->ConfigureFilterUsingProfile(pProfile);
        if(FAILED(hr)) {
            return hr;
        }
    }
#endif

    for( int g = 0 ; g < Groups ; g++ )
    {
        CComPtr< IPin > pPin;
        hr = pEngine->GetGroupOutputPin( g, &pPin );
        if( FAILED( hr ) )
        {
            return hr;
        }

        // connect pin to the mux
        //
        hr = pBuilder->RenderStream( NULL, NULL, pPin, NULL, pMux );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    if( fConnectManually )
    {
        // connect the mux to the writer. the wav mux refuses to
        // connect its output pin until its input pin is
        // connected. Otherwise this could be done earlier.
        //
        CComQIPtr< IBaseFilter, &IID_IBaseFilter > pWriterBase( pWriter );
        CComPtr<IPin> pMuxOut, pWriterIn;
        pMuxOut.p = GetPin( pMux, 0, PINDIR_OUTPUT );
        pWriterIn.p = GetPin( pWriterBase, 0, PINDIR_INPUT );
        if(!pMuxOut || !pWriterIn) {
            return E_UNEXPECTED;
        }
        hr = pGraph->Connect( pMuxOut, pWriterIn );
    }

    return hr;
}

BOOL IsAsfExtension( WCHAR *Filename )
{
    if (lstrlenW(Filename) >= 3)
    {
        WCHAR *pExt = Filename + lstrlenW(Filename) - 3;

        if(lstrcmpiW(pExt, L"asf") == 0 ||
           lstrcmpiW(pExt, L"wma") == 0 ||
           lstrcmpiW(pExt, L"wmv") == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//
// Get the mediatype for a particular ASF profile. it does this using
// the DirectShow components that use the WMSDK rather than through
// the WMSDK directly.
//

#ifdef USE_WMF_CERT

HRESULT GetDestinationASFFormat(
    AM_MEDIA_TYPE **ppmt,
    int iProfile
    )
{
    *ppmt = 0;
    
    CComPtr< IConfigAsfWriter > pWriter;
    HRESULT hr = CoCreateInstance(
        CLSID_WMAsfWriter,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IConfigAsfWriter,
        (void **)&pWriter);
    if(FAILED(hr)) {
        return hr;
    }

    CComQIPtr<IBaseFilter,&IID_IBaseFilter> pwf(pWriter);
    if(!pWriter) {
        return E_UNEXPECTED;
    }

    // we have to add the ASF writer to a dummy graph in order for the
    // calls to work.
    CComPtr< IGraphBuilder > pBuilder;
    hr = CoCreateInstance(
        CLSID_FilterGraph,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**) &pBuilder );
    if( FAILED( hr ) )
    {
        return hr;
    }

    //
    // give the graph a key to support ASF
    //
    {
        CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pBuilder );
        ASSERT( pOWS );
        if( pOWS )
        {        
            pOWS->SetSite((IUnknown *) (IServiceProvider *) &g_prov);
        }            
    }

    hr = pBuilder->AddFilter(pwf, 0);
    if(FAILED(hr)) {
        return hr;
    }

    // we use profile id -1 to indicate no profile was set.
    if (iProfile >= 0)
    {
        CComPtr<IWMProfile> pProfile;
        hr = MapProfileIdToProfile(iProfile, &pProfile);
        if(FAILED(hr)) {
            return hr;
        }
        hr = pWriter->ConfigureFilterUsingProfile(pProfile);
        if(FAILED(hr)) {
            return hr;
        }
    }

    CComPtr<IEnumPins> pEnumPins;
    hr = pwf->EnumPins( &pEnumPins );
    if(FAILED(hr)) {
        return hr;
    }
    
    // get destination video format from ASF writer using IAMStreamConfig::GetFormat on
    // its input pins
    int iPins = 0;
    ULONG ul = 0;
    CComPtr < IPin > pWriterInputPin; // we only expect to find input pins
    while( S_OK == pEnumPins->Next( 1, &pWriterInputPin, &ul ) )
    {
#ifdef DEBUG
        {
            PIN_DIRECTION pd;
            pWriterInputPin->QueryDirection( &pd );
            ASSERT( PINDIR_OUTPUT != pd );
        }
#endif
        CComQIPtr <IAMStreamConfig, &IID_IAMStreamConfig> pStreamConfig( pWriterInputPin);
        if( !pStreamConfig ) {
            return E_UNEXPECTED;
        }
        pWriterInputPin.Release();

        AM_MEDIA_TYPE *pmt2;
        hr = pStreamConfig->GetFormat( &pmt2 );
        if( SUCCEEDED( hr ) )
        {
            if(pmt2->majortype != MEDIATYPE_Video) {
                DeleteMediaType(pmt2);
                continue;
            }
            *ppmt = pmt2; // caller needs to delete
            return S_OK;
        }
    }

    return E_FAIL;
}


HRESULT MapProfileIdToProfile(
    int iProfile, IWMProfile **ppProfile)
{
    *ppProfile = 0;
    
    CComPtr <IWMProfileManager> pIWMProfileManager;
    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if(FAILED(hr)) {
        return hr;
    }

    // we only use 7_0 profiles
    CComQIPtr<IWMProfileManager2, &IID_IWMProfileManager2> pIPM2(pIWMProfileManager);
    if(!pIWMProfileManager) {
        return E_UNEXPECTED;
    }
    hr = pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
    if(FAILED(hr)) {
        return hr;
    }

    DWORD cProfiles;
    hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
    if(FAILED(hr)) {
        return hr;
    }
    if( (DWORD)iProfile >= cProfiles ) {
        printf("Invalid profile: %d\n", iProfile);
        return E_INVALIDARG;
    }

    return pIWMProfileManager->LoadSystemProfile( iProfile, ppProfile );
}

#endif // USE_WMF_CERT

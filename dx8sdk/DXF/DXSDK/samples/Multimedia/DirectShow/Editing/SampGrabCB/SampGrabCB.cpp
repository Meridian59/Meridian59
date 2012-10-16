//------------------------------------------------------------------------------
// File: SampGrabCB.cpp
//
// Desc: DirectShow sample code - C++ console application demonstrating
//       use of the IMediaDet interface to create a graph that contains a
//       sample grabber filter.  It shows how to use the sample grabber 
//       and a COM object callback to display information about media
//       samples in a running video file.  
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"

// Function prototypes
//
int GrabSamples( char * pFilename ); 

#ifdef UNICODE
#error This sample will not compile UNICODE.
#endif

// This semi-COM object is a callback COM object for the sample grabber
//
class CFakeCallback : public ISampleGrabberCB 
{
public:
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) 
        {
            *ppv = (void *) static_cast<ISampleGrabberCB *>(this);
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }

    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
    {
        static long counter = 0;
        printf( "Sample received = %05ld  Clock = %ld\r\n", 
                 counter++, timeGetTime( ) );
        return 0;
    }

    STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferLen )
    {
        return 0;
    }

};


int main(int argc, char* argv[])
{
    if( argc != 2 )
    {
        printf( "Usage: SampGrabCB <filename>\r\n\r\n" );
        printf( "This application reads a media file and receives callbacks for every\r\n"
                "media sample processed.  You must provide a valid filename.\r\n");
        return -1;
    }

    // Initialize COM
    CoInitialize( NULL );

    // Run the test on the filename specified on the command line
    GrabSamples( argv[1] );

    // COM cleanup
    CoUninitialize( );

    return 0;
}


int GrabSamples( char * pFilename )
{
    USES_CONVERSION;

    CFakeCallback pCallback;
    HRESULT hr;

    printf("Grabbing samples from %s.\r\n", pFilename);

    // create a media detector
    //
    CComPtr< IMediaDet > pDet;
    hr = CoCreateInstance( CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, 
                           IID_IMediaDet, (void**) &pDet );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in CoCreateInstance!  hr=0x%x\r\n", hr );
        return hr;
    }

    // set filename
    //
    hr = pDet->put_Filename( T2W( pFilename ) );
    if( FAILED( hr ) ) 
    {
        printf( "couldn't load the file!  hr=0x%x\r\n", hr );
        return hr;
    }

    // look for a video stream
    //
    long Streams = 0;
    hr = pDet->get_OutputStreams( &Streams );
    if( FAILED( hr ) ) 
    {
        printf( "couldn't get the output streams!  hr=0x%x\r\n", hr );
        return hr;
    }

    BOOL bFoundVideo = FALSE;

    for( int i = 0 ; i < Streams ; i++ )
    {
        BOOL bIsVideo = FALSE;

        AM_MEDIA_TYPE Type;
        memset( &Type, 0, sizeof( Type ) );

        // Select a media stream
        hr = pDet->put_CurrentStream( i );
        if( FAILED( hr ) ) 
        {
            printf( "couldn't put stream %d  hr=0x%x\r\n", i, hr );
            return hr;
        }

        // Read the media type of the selected stream
        hr = pDet->get_StreamMediaType( &Type );
        if( FAILED( hr ) ) 
        {
            printf( "couldn't get stream media type for stream %d  hr=0x%x\r\n",
                    i, hr );
            return hr;
        }

        // Does this stream contain video?
        if( Type.majortype == MEDIATYPE_Video )
            bIsVideo = TRUE;

        FreeMediaType( Type );

        if( !bIsVideo ) 
            continue;

        // Found a video stream
        bFoundVideo = TRUE;
        break;
    }

    if( !bFoundVideo )
    {
        printf( "Couldn't find a video stream\r\n" );
        return 0;
    }

    // this method will change the MediaDet to go into 
    // "sample grabbing mode" at time 0.
    //
    hr = pDet->EnterBitmapGrabMode( 0.0 );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in EnterBitmapGrabMode!  hr=0x%x\r\n", hr );
        return hr;
    }

    // ask for the sample grabber filter that we know lives inside the
    // graph made by the MediaDet
    //
    CComPtr< ISampleGrabber > pGrabber;
    hr = pDet->GetSampleGrabber( &pGrabber );
    if( FAILED(hr) || !pGrabber)
    {
        printf( "couldn't find the sample grabber filter!  hr=0x%x\r\n", hr );
        return hr;
    }

    // set the callback (our COM object callback)
    //
    CComQIPtr< ISampleGrabberCB, &IID_ISampleGrabberCB > pCB( &pCallback );
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pFilter( pGrabber );
    hr = pGrabber->SetCallback( pCB, 0 );
    hr = pGrabber->SetOneShot( FALSE );       // don't do one-shot mode
    hr = pGrabber->SetBufferSamples( FALSE ); // don't buffer samples

    // find the filter graph interface from the sample grabber filter
    //
    FILTER_INFO fi;
    memset( &fi, 0, sizeof( fi ) );
    hr = pFilter->QueryFilterInfo( &fi );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in QueryFilterInfo!  hr=0x%x\r\n", hr );
        return hr;
    }

    // Release the filter's graph reference
    if( fi.pGraph ) 
        fi.pGraph->Release( );
    IFilterGraph * pGraph = fi.pGraph;

    // The graph will have been paused by entering bitmap grab mode.
    // We'll need to seek back to 0 to get it to deliver correctly.
    //
    CComQIPtr< IMediaSeeking, &IID_IMediaSeeking > pSeeking( pGraph );
    REFERENCE_TIME Start = 0;
    REFERENCE_TIME Duration = 0;

    hr = pSeeking->GetDuration( &Duration );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in GetDuration!  hr=0x%x\r\n", hr );
        return hr;
    }

    hr = pSeeking->SetPositions( &Start,    AM_SEEKING_AbsolutePositioning, 
                                 &Duration, AM_SEEKING_AbsolutePositioning );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in SetPositions!  hr=0x%x\r\n", hr );
        return hr;
    }

    // run the graph
    //
    CComQIPtr< IMediaEvent, &IID_IMediaEvent > pEvent( pGraph );
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( pGraph );
    hr = pControl->Run( );
    if( FAILED( hr ) ) 
    {
        printf( "Failed to run the graph!  hr=0x%x\r\n", hr );
        return hr;
    }

    // wait 
    //
    long EventCode = 0;
    hr = pEvent->WaitForCompletion( INFINITE, &EventCode );
    if( FAILED( hr ) ) 
    {
        printf( "Failed in WaitForCompletion!  hr=0x%x\r\n", hr );
        return hr;
    }

    printf("Sample grabbing complete.\r\n");
    return 0;
}


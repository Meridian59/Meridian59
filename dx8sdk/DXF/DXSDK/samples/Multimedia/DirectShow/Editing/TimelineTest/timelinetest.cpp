//------------------------------------------------------------------------------
// File: TimelineTest.cpp
//
// Desc: DirectShow sample code - creates two video tracks, using a 
//       transition from the A track to the B track, and then back again.  
//       It demonstrates how to add a transition to the timeline using a 
//       CLSID.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
// This example creates 2 (video) tracks with a transition from the A track to 
// the B track and then from B back to A. It also crossfades two audio tracks 
// from A to B.  This sample also demonstrates how to add a transition by CLSID 
// to the timeline and how to vary volume envelope properties by the 
// property setter.
//
// Note: Error checking is handled mostly with ASSERTs in order to keep
// the sample as uncluttered as possible.  In some cases where several
// related function calls are made together, the HRESULT value returned from
// the functions will be bitwise OR'ed with other HRESULT values.  This value
// will eventually be tested for failure using the FAILED macro.
//----------------------------------------------------------------------------

#include "stdafx.h"
#include <streams.h>
#include <atlbase.h>
#include <qedit.h>
#include <dxutil.h>

#ifdef STRICT
#undef STRICT
#endif
#include <dxutil.cpp>

//
// Conditional compilation flags to enable/disable effects
//
//#define CUTS_ONLY

// define this to show an example of calling ConnectFrontEnd( ) 
// a second time on the render engine and having it rerender
// the graph
#define DO_RECONNECT

// define this to do an audio crossfade
#define DO_CROSSFADE

// define this to only render a portion of the full timeline
#define DO_RENDERRANGE

// define this to enable transitions
#define DO_TRANSITION

//
// Global data
//

// Use bitmaps and audio files known to install with the DirectX 8 SDK
WCHAR * wszVideo1Name = L"dx5_logo.bmp";
WCHAR * wszVideo2Name = L"env3.bmp";
WCHAR * wszAudio1Name = L"track2.mp3";
WCHAR * wszAudio2Name = L"track3.mp3";
WCHAR * wszTitle = L"Timeline Test Sample";

//
// Function prototypes
//
int TimelineTest( );
void Err(TCHAR *szErr);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    // init the ole32 libraries
    //
    CoInitialize( NULL );

    //
    // NOTE: There is no user interaction with this sample.
    // It will run to completion and exit.
    //
    TimelineTest();

    CoUninitialize( );

    return 0;
}


void Err(TCHAR *szErr)
{
    MessageBox(NULL, szErr, TEXT("Error"), MB_ICONEXCLAMATION);
}


int TimelineTest( )
{
    USES_CONVERSION;
    HRESULT hr;

    // use the ATL libraries to do automatic reference counting on pointers
    //
    CComPtr< IRenderEngine > pRenderEngine;
    CComPtr< IGraphBuilder > pGraph;
    CComPtr< IVideoWindow > pVidWindow;
    CComPtr< IMediaEvent > pEvent;
    CComPtr< IAMTimeline > pTimeline;
    CComPtr< IAMTimelineObj > pVideoGroupObj;
    CComPtr< IAMTimelineObj > pAudioGroupObj;
    CComPtr< IMediaControl > pControl;
    CComPtr< IMediaSeeking > pSeeking;

    //--------------------------------------------
    // make the timeline
    //--------------------------------------------

    hr = CoCreateInstance(
                         CLSID_AMTimeline, 
                         NULL, 
                         CLSCTX_INPROC_SERVER, 
                         IID_IAMTimeline, 
                         (void**) &pTimeline 
                         );

    if(FAILED( hr )) {
        Err(_T("Could not create timeline"));
        return hr;
    }

    //--------------------------------------------
    // make the root group/composition
    //--------------------------------------------

    hr = pTimeline->CreateEmptyNode( &pVideoGroupObj, TIMELINE_MAJOR_TYPE_GROUP );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create empty node"));
        return hr;
    }

    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pVideoGroup( pVideoGroupObj );
    CMediaType VideoGroupType;

    // all we set is the major type. The group will automatically use other defaults
    VideoGroupType.SetType( &MEDIATYPE_Video );
    hr = pVideoGroup->SetMediaType( &VideoGroupType );
    if(FAILED( hr )) 
    {
        Err(_T("Could not set media type"));
        return hr;
    }

    //--------------------------------------------
    // add the group to the timeline
    //--------------------------------------------

    hr = pTimeline->AddGroup( pVideoGroupObj );
    if(FAILED( hr )) 
    {
        Err(_T("Could not add video group"));
        return hr;
    }

    //--------------------------------------------
    // make a track
    //--------------------------------------------

    CComPtr< IAMTimelineObj > pTrack1Obj;
    hr = pTimeline->CreateEmptyNode( &pTrack1Obj, TIMELINE_MAJOR_TYPE_TRACK );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create empty track"));
        return hr;
    }

    //--------------------------------------------
    // tell the composition about the track
    //--------------------------------------------

    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pRootComp( pVideoGroupObj );
    hr = pRootComp->VTrackInsBefore( pTrack1Obj, -1 );
    if(FAILED( hr )) 
    {
        Err(_T("Could not insert track"));
        return hr;
    }

    //--------------------------------------------
    // create a source from 0 to 8 seconds
    //--------------------------------------------

    REFERENCE_TIME TLStart = 0 * UNITS;
    REFERENCE_TIME TLStop = 8 * UNITS;

    // you can set these if you want to other numbers, and the video will 
    // speed up or slow down if the duration isn't the same as the timeline's.
    REFERENCE_TIME MediaStart = 0 * UNITS;
    REFERENCE_TIME MediaStop = 8 * UNITS;
    WCHAR pClipname[256];
    TCHAR * tBasePath = (TCHAR *) DXUtil_GetDXSDKMediaPath( );
    wcscpy( pClipname, T2W( tBasePath ) );
    wcscat( pClipname, L"\\" );
    wcscat( pClipname, wszVideo1Name );

    // create the timeline source
    //
    CComPtr<IAMTimelineObj> pSource1Obj;
    hr = pTimeline->CreateEmptyNode( &pSource1Obj, TIMELINE_MAJOR_TYPE_SOURCE );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create the timeline source"));
        return hr;
    }

    // set up source right
    //
    hr = pSource1Obj->SetStartStop( TLStart, TLStop );
    CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource1Src( pSource1Obj );
    hr |= pSource1Src->SetMediaTimes( MediaStart, MediaStop );
    hr |= pSource1Src->SetMediaName( pClipname );
    if(FAILED( hr )) 
    {
        Err(_T("Could not configure media source"));
        return E_FAIL;
    }

    //--------------------------------------------
    // tell the track about the source
    //--------------------------------------------

    CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack1( pTrack1Obj );
    hr = pTrack1->SrcAdd( pSource1Obj );
    if(FAILED( hr )) 
    {
        Err(_T("Could not add source to track"));
        return hr;
    }

    //--------------------------------------------
    // make another track 
    //--------------------------------------------

    CComPtr< IAMTimelineObj > pTrack2Obj;
    hr = pTimeline->CreateEmptyNode( &pTrack2Obj, TIMELINE_MAJOR_TYPE_TRACK );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create second track"));
        return hr;
    }

    //--------------------------------------------
    // tell the composition about the track
    //--------------------------------------------

    hr = pRootComp->VTrackInsBefore( pTrack2Obj, -1 );
    if(FAILED( hr )) 
    {
        Err(_T("Could not insert second track"));
        return hr;
    }

    //--------------------------------------------
    // create a source for the 2nd track
    //--------------------------------------------

    TLStart = 0 * UNITS;
    TLStop = 8 * UNITS;
    MediaStart = 0 * UNITS;
    MediaStop = 8 * UNITS;
    wcscpy( pClipname, T2W( tBasePath ) );
    wcscat( pClipname, L"\\" );
    wcscat( pClipname, wszVideo2Name );

    // create the timeline source
    //
    CComPtr<IAMTimelineObj> pSource2Obj;
    hr = pTimeline->CreateEmptyNode( &pSource2Obj, TIMELINE_MAJOR_TYPE_SOURCE );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create the second timeline source"));
        return hr;
    }

    // set up source right
    //
    hr = pSource2Obj->SetStartStop( TLStart, TLStop );
    CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource2Src( pSource2Obj );
    hr |= pSource2Src->SetMediaTimes( MediaStart, MediaStop );
    hr |= pSource2Src->SetMediaName( pClipname );
    if(FAILED( hr )) 
    {
        Err(_T("Could not configure second media source"));
        return E_FAIL;
    }

    //--------------------------------------------
    // tell the track about the source
    //--------------------------------------------

    CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack2( pTrack2Obj );
    hr = pTrack2->SrcAdd( pSource2Obj );
    if(FAILED( hr )) 
    {
        Err(_T("Could not add second track"));
        return hr;
    }

    CComQIPtr< IAMTimelineTransable, &IID_IAMTimelineTransable > pTransable( pTrack2 );
    

#ifdef DO_TRANSITION

    //---------------------------------------------
    // create an transition on the track from A 2 B
    //---------------------------------------------

    REFERENCE_TIME TransStart = 0 * UNITS;
    REFERENCE_TIME TransStop = 4 * UNITS;

    // create the timeline effect
    //
    CComPtr<IAMTimelineObj> pTrackTransObj;
    hr = pTimeline->CreateEmptyNode(
                                   &pTrackTransObj,
                                   TIMELINE_MAJOR_TYPE_TRANSITION );
    if(FAILED( hr )) 
    {
        Err(_T("Could not create transition effect"));
        return hr;
    }

    //--------------------------------------------
    // set up filter right
    //--------------------------------------------

    // we set the CLSID of the DXT to use instead of a pointer to the
    // actual object. We let the DXT have it's default properties.
    //
    hr = pTrackTransObj->SetSubObjectGUID( CLSID_DxtJpeg );
    hr |= pTrackTransObj->SetStartStop( TransStart, TransStop );
    CComQIPtr< IAMTimelineTrans, &IID_IAMTimelineTrans > pTrackTrans( pTrackTransObj );
    hr |= pTransable->TransAdd( pTrackTransObj );
    if(FAILED( hr )) 
    {
        Err(_T("Could not configure transition object"));
        return E_FAIL;
    }

#ifdef CUTS_ONLY 
    //---------------------------------------------
    // turn the transition into a cut by doing this
    //---------------------------------------------

    hr = pTrackTrans->SetCutsOnly( TRUE );
    if(FAILED( hr )) 
    {
        Err(_T("Could not SetCutsOnly to TRUE"));
        return hr;
    }

#endif  // CUTS_ONLY

    //---------------------------------------------
    // create an transition on the track from B 2 A
    //---------------------------------------------

    TransStart = 4 * UNITS;
    TransStop = 8 * UNITS;

    // create the timeline effect
    //
    pTrackTransObj.Release( );
    hr = pTimeline->CreateEmptyNode(
                                   &pTrackTransObj,
                                   TIMELINE_MAJOR_TYPE_TRANSITION );
    ASSERT( !FAILED( hr ) );

    // set up filter right
    //
    hr = pTrackTransObj->SetSubObjectGUID( CLSID_DxtJpeg );            
    hr |= pTrackTransObj->SetStartStop( TransStart, TransStop );
    pTrackTrans = pTrackTransObj;
    hr |= pTrackTrans->SetSwapInputs( TRUE );
    hr |= pTransable->TransAdd( pTrackTransObj );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // set a property on the transition
    //--------------------------------------------

    CComPtr< IPropertySetter > pTransSetter;
    pTransSetter.CoCreateInstance( CLSID_PropertySetter );
    DEXTER_PARAM Param;
    CComBSTR ParamName( "MaskNum" ); // the property name
    Param.Name = ParamName;
    Param.nValues = 1; // how many values we want to set
    DEXTER_VALUE Value;
    memset( &Value, 0, sizeof( Value ) );
    VariantClear( &Value.v );
    V_I4( &Value.v ) = 128; // mask number 128
    V_VT( &Value.v ) = VT_I4; // integer
    hr = pTransSetter->AddProp( Param, &Value );
    hr |= pTrackTransObj->SetPropertySetter( pTransSetter );
    ASSERT( !FAILED( hr ) );
    // pTransSetter will be auto-freed by COM

#endif  // DO_TRANSITION

#ifdef CUTS_ONLY 
    //---------------------------------------------
    // turn the transition into a cut by doing this
    //---------------------------------------------

    hr = pTrackTrans->SetCutsOnly( TRUE );
    ASSERT( !FAILED( hr ) );
#endif

    //--------------------------------------------
    // make the root audio group/composition
    //--------------------------------------------

    hr = pTimeline->CreateEmptyNode( &pAudioGroupObj, TIMELINE_MAJOR_TYPE_GROUP );
    ASSERT( !FAILED( hr ) );

    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pAudioGroup( pAudioGroupObj );
    CMediaType AudioGroupType;
    // all we set is the major type. The group will automatically use other defaults
    AudioGroupType.SetType( &MEDIATYPE_Audio );
    hr = pAudioGroup->SetMediaType( &AudioGroupType );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // add the group to the timeline
    //--------------------------------------------

    hr = pTimeline->AddGroup( pAudioGroupObj );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // make a track
    //--------------------------------------------

    CComPtr< IAMTimelineObj > pTrack3Obj;
    hr = pTimeline->CreateEmptyNode( &pTrack3Obj, TIMELINE_MAJOR_TYPE_TRACK );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // tell the composition about the track
    //--------------------------------------------

    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pAudioComp( pAudioGroupObj );
    hr = pAudioComp->VTrackInsBefore( pTrack3Obj, -1 );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // create a source 
    //--------------------------------------------

    TLStart = 0 * UNITS;
    TLStop = 6 * UNITS;
    // you can set these if you want to other numbers, the video will speed up
    // or slow down if the duration isn't the same at the timeline's.
    MediaStart = 0 * UNITS;
    MediaStop = 6 * UNITS;
    wcscpy( pClipname, T2W( tBasePath ) );
    wcscat( pClipname, L"\\" );
    wcscat( pClipname, wszAudio1Name );

    // create the timeline source
    //
    CComPtr<IAMTimelineObj> pSource3Obj;
    hr = pTimeline->CreateEmptyNode( &pSource3Obj, TIMELINE_MAJOR_TYPE_SOURCE );
    ASSERT( !FAILED( hr ) );

    // set up source right
    //
    hr = pSource3Obj->SetStartStop( TLStart, TLStop );
    CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource3Src( pSource3Obj );
    hr |= pSource3Src->SetMediaTimes( MediaStart, MediaStop );
    hr |= pSource3Src->SetMediaName( pClipname );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // tell the track about the source
    //--------------------------------------------

    CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack3( pTrack3Obj );
    hr = pTrack3->SrcAdd( pSource3Obj );
    ASSERT( !FAILED( hr ) );

#ifdef DO_CROSSFADE
    //--------------------------------------------
    // put a volume effect on the source
    //--------------------------------------------

    TLStart = 4 * UNITS;
    TLStop = 6 * UNITS;

    CComPtr< IAMTimelineObj > pTrack3FxObj;
    hr = pTimeline->CreateEmptyNode( &pTrack3FxObj, TIMELINE_MAJOR_TYPE_EFFECT );
    ASSERT( !FAILED( hr ) );

    // set up effect right
    //
    hr = pTrack3FxObj->SetStartStop( TLStart, TLStop );
    hr |= pTrack3FxObj->SetSubObjectGUID( CLSID_AudMixer );
    ASSERT( !FAILED( hr ) );

    // add the effect
    //
    CComQIPtr< IAMTimelineEffectable , &IID_IAMTimelineEffectable > pTrack3Fable( pTrack3 );
    hr = pTrack3Fable->EffectInsBefore( pTrack3FxObj, -1 );
    ASSERT( !FAILED( hr ) );

    //------------------------------------------------
    // set the the volume envelope on the audio source
    //------------------------------------------------

    CComPtr< IPropertySetter > pVolSetter;
    pVolSetter.CoCreateInstance( CLSID_PropertySetter );
    CComBSTR VolParamName( "Vol" ); // the property name
    Param.Name = VolParamName;
    Param.nValues = 2; // how many values we want to set

    DEXTER_VALUE AudioValue[2];
    memset( &AudioValue[0], 0, sizeof( Value ) );
    VariantClear( &AudioValue[0].v );
    V_R8( &AudioValue[0].v ) = 1.0;
    V_VT( &AudioValue[0].v ) = VT_R8;
    AudioValue[0].rt = 0 * UNITS;
    memset( &AudioValue[1], 0, sizeof( Value ) );
    VariantClear( &AudioValue[1].v );
    V_R8( &AudioValue[1].v ) = 0.0;
    V_VT( &AudioValue[1].v ) = VT_R8;
    AudioValue[1].rt = 2 * UNITS;
    AudioValue[1].dwInterp = DEXTERF_INTERPOLATE;

    hr = pVolSetter->AddProp( Param, AudioValue );
    hr |= pTrack3FxObj->SetPropertySetter( pVolSetter );
    ASSERT( !FAILED( hr ) );
    pVolSetter.Release( );
#endif  // DO_CROSSFADE

    //--------------------------------------------
    // make another track 
    //--------------------------------------------

    CComPtr< IAMTimelineObj > pTrack4Obj;
    hr = pTimeline->CreateEmptyNode( &pTrack4Obj, TIMELINE_MAJOR_TYPE_TRACK );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // tell the composition about the track
    //--------------------------------------------

    hr = pAudioComp->VTrackInsBefore( pTrack4Obj, -1 );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // create a source for the 2nd track
    //--------------------------------------------

    TLStart = 4 * UNITS;
    TLStop = 8 * UNITS;
    MediaStart = 0 * UNITS;
    MediaStop = 4 * UNITS;
    wcscpy( pClipname, T2W( tBasePath ) );
    wcscat( pClipname, L"\\" );
    wcscat( pClipname, wszAudio2Name );

    // create the timeline source
    //
    CComPtr<IAMTimelineObj> pSource4Obj;
    hr = pTimeline->CreateEmptyNode( &pSource4Obj, TIMELINE_MAJOR_TYPE_SOURCE );
    ASSERT( !FAILED( hr ) );

    // set up source right
    //
    hr = pSource4Obj->SetStartStop( TLStart, TLStop );
    CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource4Src( pSource4Obj );
    hr |= pSource4Src->SetMediaTimes( MediaStart, MediaStop );
    hr |= pSource4Src->SetMediaName( pClipname );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // tell the track about the source
    //--------------------------------------------

    CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack4( pTrack4Obj );
    hr = pTrack4->SrcAdd( pSource4Obj );
    ASSERT( !FAILED( hr ) );

#ifdef DO_CROSSFADE
    //--------------------------------------------
    // put a volume effect on the source
    //--------------------------------------------

    TLStart = 4 * UNITS;
    TLStop = 6 * UNITS;

    CComPtr< IAMTimelineObj > pTrack4FxObj;
    hr = pTimeline->CreateEmptyNode( &pTrack4FxObj, TIMELINE_MAJOR_TYPE_EFFECT );
    ASSERT( !FAILED( hr ) );

    // set up effect riht
    //
    hr = pTrack4FxObj->SetStartStop( TLStart, TLStop );
    hr |= pTrack4FxObj->SetSubObjectGUID( CLSID_AudMixer );
    ASSERT( !FAILED( hr ) );

    // add the effect
    //
    CComQIPtr< IAMTimelineEffectable , &IID_IAMTimelineEffectable > pTrack4Fable( pTrack4 );
    hr = pTrack4Fable->EffectInsBefore( pTrack4FxObj, -1 );
    ASSERT( !FAILED( hr ) );

    //------------------------------------------------
    // set the the volume envelope on the audio source
    //------------------------------------------------

    pVolSetter.CoCreateInstance( CLSID_PropertySetter );
    memset( &AudioValue[0], 0, sizeof( Value ) );
    VariantClear( &AudioValue[0].v );
    V_R8( &AudioValue[0].v ) = 0.0;
    V_VT( &AudioValue[0].v ) = VT_R8;
    AudioValue[0].rt = 0 * UNITS;
    memset( &AudioValue[1], 0, sizeof( Value ) );
    VariantClear( &AudioValue[1].v );
    V_R8( &AudioValue[1].v ) = 1.0;
    V_VT( &AudioValue[1].v ) = VT_R8;
    AudioValue[1].rt = 2 * UNITS;
    AudioValue[1].dwInterp = DEXTERF_INTERPOLATE;

    hr = pVolSetter->AddProp( Param, AudioValue );
    hr |= pTrack4FxObj->SetPropertySetter( pVolSetter );
    ASSERT( !FAILED( hr ) );
    // pVolSetter will be auto-freed by COM
#endif  // DO_CROSSFADE
    
    //----------------------------------------------
    // make sure files are in their correct location
    //----------------------------------------------
    
    hr = pTimeline->ValidateSourceNames( 
        SFN_VALIDATEF_CHECK | SFN_VALIDATEF_POPUP | SFN_VALIDATEF_REPLACE, 
        NULL, 
        0 );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // create the render engine
    //--------------------------------------------

    hr = CoCreateInstance(
                         CLSID_RenderEngine,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_IRenderEngine,
                         (void**) &pRenderEngine );
    ASSERT( !FAILED( hr ) );

    // tell the render engine about the timeline it should look at
    //
    hr = pRenderEngine->SetTimelineObject( pTimeline );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // connect up the front end, then the back end
    //--------------------------------------------

    hr = pRenderEngine->ConnectFrontEnd( );
    hr |= pRenderEngine->RenderOutputPins( );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // get a bunch of pointers, then run the graph
    //--------------------------------------------

    hr = pRenderEngine->GetFilterGraph( &pGraph );
    hr |= pGraph->QueryInterface( IID_IMediaEvent, (void**) &pEvent );
    hr |= pGraph->QueryInterface( IID_IMediaControl, (void**) &pControl );
    hr |= pGraph->QueryInterface( IID_IMediaSeeking, (void**) &pSeeking );
    hr |= pGraph->QueryInterface( IID_IVideoWindow, (void**) &pVidWindow );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // give the main window a meaningful title
    //--------------------------------------------

    BSTR bstrCaption;
    WriteBSTR(&bstrCaption, wszTitle);
    hr = pVidWindow->put_Caption(bstrCaption);
    FreeBSTR(&bstrCaption);

    //--------------------------------------------
    // since no user interaction is allowed, remove
    // system menu and maximize/minimize buttons
    //--------------------------------------------
    long lStyle=0;
    hr = pVidWindow->get_WindowStyle(&lStyle);
    lStyle &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    hr = pVidWindow->put_WindowStyle(lStyle);

    //--------------------------------------------
    // run it
    //--------------------------------------------

    hr = pControl->Run( );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // wait for it
    //--------------------------------------------

    long EventCode = 0;
    hr = pEvent->WaitForCompletion( -1, &EventCode );
    ASSERT( !FAILED( hr ) );

    REFERENCE_TIME Start = 0;

#ifdef DO_RENDERRANGE

    // seek the timeline back to 0
    //
    hr = pSeeking->SetPositions( &Start, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
    ASSERT( !FAILED( hr ) );

    hr = pRenderEngine->SetRenderRange2( 2.0, 6.0 );
    ASSERT( !FAILED( hr ) );

    //------------------------------------------------------
    // connect up the front end, then the back end if needed
    //------------------------------------------------------

    hr = pRenderEngine->ConnectFrontEnd( );
    ASSERT( !FAILED( hr ) );
    if(hr == S_WARN_OUTPUTRESET) {
        hr |= pRenderEngine->RenderOutputPins( );
    }
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // run it
    //--------------------------------------------

    hr = pControl->Run( );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // wait for it
    //--------------------------------------------

    hr = pEvent->WaitForCompletion( -1, &EventCode );
    ASSERT( !FAILED( hr ) );
#endif  // DO_RENDERRANGE

#ifdef DO_RECONNECT 

    //---------------------------------------------
    // make a change to the timeline, however small
    //---------------------------------------------

    CComPtr< IAMTimelineObj > pTransObj;
    REFERENCE_TIME InOut = -1;
    pTransable->GetNextTrans( &pTransObj, &InOut );
    CComQIPtr< IAMTimelineTrans, &IID_IAMTimelineTrans > pTrans( pTransObj );
    pTrans->SetCutsOnly( TRUE );
    pTransObj.Release( );
    pTrans.Release( );
    hr = pTransable->GetNextTrans( &pTransObj, &InOut );
    pTrans = pTransObj;
    hr |= pTrans->SetCutsOnly( TRUE );
    ASSERT( !FAILED( hr ) );

    // seek the timeline back to 0
    //
    hr = pSeeking->SetPositions( &Start, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
    ASSERT( !FAILED( hr ) );

    //------------------------------------------------------
    // connect up the front end, then the back end if needed
    //------------------------------------------------------

    hr = pRenderEngine->ConnectFrontEnd( );
    if(hr == S_WARN_OUTPUTRESET) {
        hr |= pRenderEngine->RenderOutputPins( );
    }
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // run it
    //--------------------------------------------

    hr = pControl->Run( );
    ASSERT( !FAILED( hr ) );

    //--------------------------------------------
    // wait for it
    //--------------------------------------------

    hr = pEvent->WaitForCompletion( -1, &EventCode );
    ASSERT( !FAILED( hr ) );
#endif  // DO_RECONNECT

    return 0;
}




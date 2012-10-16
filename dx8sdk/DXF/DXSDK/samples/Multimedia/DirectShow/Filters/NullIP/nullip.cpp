//------------------------------------------------------------------------------
// File: NullIP.h
//
// Desc: DirectShow sample code - illustrates: 
//             An in-place transform filter
//             Media types and the connection process
//             Performance logging using Measure.dll
//             Use of CAutoLock class to automatically release critical sections             
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//     Summary
//
// A simple in-place transform with one input pin and one output pin.
// The filter operates in-place which means that it has no media type
// enumerators of its own.  It obtains enumerators from the filters either
// on either side and passes types so obtained across for approval by the
// one on the other side.  On the way through it sees if it likes the
// type and rejects any that it doesn't.  The property sheet displays the
// list of types obtained from the upstream filter and allows one or
// all to be selected.  Any type not selected will be rejected by the
// connection process.  Almost all the work is done by the base classes.
//
//
//     Demonstration instructions
//
// First build the sample and get it registered
// Start GraphEdit (available in the SDK DXUtils folder).
// Select Graph then InsertFilters and insert FileSource (async)
// Select Angry.AVI as the source file.
// Insert Null-In-Place.  (If it is not on the list then you
// failed to register it properly.  If it fails to load then you either
// didn't build it properly or the registration does not correctly point to
// the path where nullip.ax is now found.)
// Right click on the output pin of the source filter. Select Render.
// In the resulting graph you will see the Null-In-Place filter inserted between
// the AVI decompressor and the video renderer.
// Right click on the null-in-place filter to bring up its properties page.
// Select Major Type: MEDIATYPE_Video - Sub Type:MEDIASUBTYPE_RGB24 <etc>
// Click on Apply.  It wil tell you that it can't do it.
// Delete the output connection (Null-In-Place to Video Renderer)
// Try to Apply the media type again.  It succeeds.
// Connect the Mull-In-Place transform to the Video Renderer again.
// It will (probably) insert a Colour Space Transform to make the connection.
//
//
//     Implementation
//
// This filter has one input pin, one output pin and
// does its "transform" in-place (i.e. without copying the data)
// on the push thread (i.e. it is called with a buffer, which it
// "transforms" and gives to the next filter downstream.  It is
// then blocked until that filter returns.  It then returns
// to its own caller.)
//
// In this case the transform does nothing.  It just passes the data on.
//
// It has a properties page which allows control of what media types
// are allowed on connection.

// Note about Receive:
//
// NOTE: Please note that we shoud NOT override the Receive method to Deliver
// the input sample directly. We may have a case where we could not negotiate
// the same allocator for the input and output pins. (For example, if the
// upstream filter insists on its own allocator and provides a read-only
// allocator, that will be the case). In this case samples need to be
// copied from input to output allocator and then transformed. The base
// class Receive method does this. Transform () will get called on the copied
// over buffer and in our case, that function is a NOP and does nothing.
//
//
//      Files
//
// nullip.cpp    This file - main implementation
// nullprop.cpp  Implementation of the properties sheet
// nullprop.h    Class definition of properties class (used in nullip.cpp)
// nullprop.rc   Defines the property page dialog
// inull.h       Interface between nullip and nullprop
// nulluids.h    The public class ids (only referred to in nullip)
// resource.h    constants shared between nullip.rc and nullip.cpp
// texttype.cpp  implementation of class used to display media types as text
// texttype.h    definition of class used to display media types as text
// nullip.def    Imports and exports
// makefile      How to build it
//
//
//     Base classes used (refer to docs for diagram of what they inherit):
//
// CTransInPlaceFilter
// CBasePropertyPage
// CMediaType
// CTransInPlaceInputPin
// CTransInPlaceOutputPin


#include <streams.h>          // quartz, includes windows

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <measure.h>          // performance measurement (MSR_)

#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#include "nullUids.h"         // our own public guids

#include "inull.h"            // interface between filter and property sheet
#include "nullprop.h"         // property sheet implementatino class



// ----------------------------------------------------------------------------
// Class definitions of input pin, output pin and filter
// ----------------------------------------------------------------------------


class CNullInPlaceInputPin : public CTransInPlaceInputPin
{
    public:
        CNullInPlaceInputPin( TCHAR *pObjectName
                            , CTransInPlaceFilter *pTransInPlaceFilter
                            , HRESULT * phr
                            , LPCWSTR pName
                            )
                              : CTransInPlaceInputPin( pObjectName
                                                     , pTransInPlaceFilter
                                                     , phr
                                                     , pName
                                                     )
        {
        }

        HRESULT CheckMediaType(const CMediaType* pmt);
};


class CNullInPlaceOutputPin : public CTransInPlaceOutputPin
{
    public:
        CNullInPlaceOutputPin( TCHAR *pObjectName
                             , CTransInPlaceFilter *pTransInPlaceFilter
                             , HRESULT * phr
                             , LPCWSTR pName
                             )
                               : CTransInPlaceOutputPin( pObjectName
                                                       , pTransInPlaceFilter
                                                       , phr
                                                       , pName
                                                       )
        {
        }

        HRESULT CheckMediaType(const CMediaType* pmt);
};



// CNullInPlace
//
class CNullInPlace : public CTransInPlaceFilter
                   , public INullIPP
                   , public ISpecifyPropertyPages
{

        friend class CNullInPlaceInputPin;
        friend class CNullInPlaceOutputPin;

    public:

        static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

        DECLARE_IUNKNOWN;

        //
        // --- CTransInPlaceFilter Overrides --
        //

        virtual CBasePin *GetPin( int n );

        HRESULT CheckInputType(const CMediaType* mtIn)
            { UNREFERENCED_PARAMETER(mtIn);  return S_OK; }

        // Basic COM - used here to reveal our property interface.
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

        STDMETHODIMP get_IPin (IPin **ppPin) ;
        STDMETHODIMP put_MediaType(CMediaType *pmt);
        STDMETHODIMP get_MediaType(CMediaType **pmt);
        STDMETHODIMP get_State(FILTER_STATE *state);


        //
        // --- ISpecifyPropertyPages ---
        //

        STDMETHODIMP GetPages(CAUUID *pPages);

    private:

        CNullInPlace(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

        //
        // Overrides the PURE virtual Transform of CTransInPlaceFilter base class
        // This is where the "real work" is done.
        //
        HRESULT Transform(IMediaSample *pSample)
            { UNREFERENCED_PARAMETER(pSample); return NOERROR; }


        //
        // If there are multiple instances of this filter active, it's
        // useful for debug messages etc. to know which one this is.
        //
        static m_nInstanceCount;   // total instances
        int m_nThisInstance;

        CMediaType m_mtPreferred;  // Media type chosen from property sheet

        CCritSec m_NullIPLock;     // To serialise access.
};


// ----------------------------------------------------------------------------
// Implementation of pins and filter
// ----------------------------------------------------------------------------


//
// DbgFunc
//
// Put out the name of the function and instance on the debugger.
// Call this at the start of interesting functions to help debug
//
#define DbgFunc(a) DbgLog(( LOG_TRACE                        \
                          , 2                                \
                          , TEXT("CNullInPlace(Instance %d)::%s") \
                          , m_nThisInstance                  \
                          , TEXT(a)                          \
                         ));

//
// setup data
//

const AMOVIESETUP_MEDIATYPE
sudPinTypes =   { &MEDIATYPE_NULL                // clsMajorType
                , &MEDIASUBTYPE_NULL }  ;       // clsMinorType

const AMOVIESETUP_PIN
psudPins[] = { { L"Input"            // strName
               , FALSE               // bRendered
               , FALSE               // bOutput
               , FALSE               // bZero
               , FALSE               // bMany
               , &CLSID_NULL         // clsConnectsToFilter
               , L"Output"           // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes }      // lpTypes
             , { L"Output"           // strName
               , FALSE               // bRendered
               , TRUE                // bOutput
               , FALSE               // bZero
               , FALSE               // bMany
               , &CLSID_NULL         // clsConnectsToFilter
               , L"Input"            // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes } };   // lpTypes


const AMOVIESETUP_FILTER
sudNullIP = { &CLSID_NullInPlace                 // clsID
            , L"Null In Place"                // strName
            , MERIT_DO_NOT_USE                // dwMerit
            , 2                               // nPins
            , psudPins };                     // lpPin

//
// Needed for the CreateInstance mechanism
//
CFactoryTemplate g_Templates[]=
    {   {L"Null-In-Place"
        , &CLSID_NullInPlace
        ,   CNullInPlace::CreateInstance
        , NULL
        , &sudNullIP }
    ,
        { L"Null IP Property Page"
        , &CLSID_NullIPPropertyPage
        , NullIPProperties::CreateInstance }

    };
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//
// initialise the static instance count.
//
int CNullInPlace::m_nInstanceCount = 0;


// ----------------------------------------------------------------------------
//            Input pin implementation
// ----------------------------------------------------------------------------


// CheckMediaType
//
// Override CTransInPlaceInputPin method.
// If we have been given a preferred media type from the property sheet
// then only accept a type that is exactly that.
// else if there is nothing downstream, then accept anything
// else if there is a downstream connection then first check to see if
// the subtype (and implicitly the major type) are different from the downstream
// connection and if they are different, fail them
// else ask the downstream input pin if the type (i.e. all details of it)
// are acceptable and take that as our answer.
//
HRESULT CNullInPlaceInputPin::CheckMediaType( const CMediaType *pmt )
{   CNullInPlace *pNull = (CNullInPlace *) m_pTIPFilter;

#ifdef DEBUG
    DisplayType(TEXT("Input type proposed"),pmt);
#endif

    if (pNull->m_mtPreferred.IsValid() == FALSE)
    {
        if( pNull->m_pOutput->IsConnected() ) {

            //  We used to check here if the subtype of the proposed type
            //  matched the subtype of the type on the output pin
            //  but this broke as follows:
            //
            //  Renderering the output pin of a CODEC we picked up
            //  2 NULLIPs already in the graph:
            //
            //  Subtypes      Y41P       Y41P       RGB565
            //  Filters  CODEC---->NULLIP---->NULLIP------>RENDERER
            //
            //  Each NULLIP has scheduled a reconnect at this point
            //  and the reconnect on the first connection happens
            //  first:
            //
            //  Subtypes                 Y41P       RGB565
            //  Filters  CODEC     NULLIP---->NULLIP------>RENDERER
            //
            //  In trying to (re)connect the CODEC to the first NULLIP
            //  we first propose (say) Y41P and the first NULLIP
            //  checks that Y41P is the same as its output type
            //  so the call gets passed to the QueryAccept of
            //  the second NULLIP.  The second NULLIP rejected the
            //  call because the subtype on its output pin is not
            //  RGB565.  In a similar way the first NULLIP
            //  rejected Y41P.
            //
            //  By removing this optimization (checking the
            //  subtype before passing the call on) we avoided
            //  the problem.

            return pNull->m_pOutput->GetConnected()->QueryAccept( pmt );
        }
        return S_OK;
    }
    else
        if (*pmt == pNull->m_mtPreferred)
            return S_OK  ;
        else
            return VFW_E_TYPE_NOT_ACCEPTED;

}



// ----------------------------------------------------------------------------
//            Input pin implementation
// ----------------------------------------------------------------------------


// CheckMediaType
//
// Override CTransInPlaceOutputPin method.
// If we have ben given a media type from the property sheet, then insist on
// exactly that, else pass the request up to the base class implementation.
//
HRESULT CNullInPlaceOutputPin::CheckMediaType( const CMediaType *pmt )
{   CNullInPlace *pNull = (CNullInPlace *) m_pTIPFilter;

    if (pNull->m_mtPreferred.IsValid() == FALSE)
    {
        return CTransInPlaceOutputPin::CheckMediaType (pmt) ;
    }
    else
        if (*pmt == pNull->m_mtPreferred)
            return S_OK  ;
        else
            return VFW_E_TYPE_NOT_ACCEPTED;

}



// ----------------------------------------------------------------------------
//            Filter implementation
// ----------------------------------------------------------------------------


//
// CNullInPlace::Constructor
//
CNullInPlace::CNullInPlace(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransInPlaceFilter (tszName, punk, CLSID_NullInPlace, phr)
{
    m_nThisInstance = ++m_nInstanceCount;
    m_mtPreferred.InitMediaType () ;
    DbgFunc("CNullInPlace");


} // (CNullInPlace constructor)


//
// CreateInstance
//
// Override CClassFactory method.
// Provide the way for COM to create a CNullInPlace object
//
CUnknown * WINAPI CNullInPlace::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CNullInPlace *pNewObject = new CNullInPlace(NAME("Null-In-Place Filter"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;

} // CreateInstance


//
// GetPin
//
// Override CBaseFilter method.
// return a non-addrefed CBasePin * for the user to addref if he holds onto it
// for longer than his pointer to us.  This is part of the implementation of
// EnumMediaTypes.  All attempts to refer to our pins from the outside have
// to come through here, so it's a valid place to create them.
//
CBasePin *CNullInPlace::GetPin(int n)
{
    // Create the single input pin and the single output pin
    // If anything fails, fail the whole lot and clean up.

    if (m_pInput == NULL || m_pOutput == NULL) {

        HRESULT hr = S_OK;

        m_pInput = new CNullInPlaceInputPin(NAME("Null input pin"),
                                          this,              // Owner filter
                                          &hr,               // Result code
                                          L"Input");         // Pin name

        // a failed return code should delete the object

        if (FAILED(hr) || m_pInput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
            return NULL;
        }

        m_pOutput = new CNullInPlaceOutputPin(NAME("Null output pin"),
                                            this,            // Owner filter
                                            &hr,             // Result code
                                            L"Output");      // Pin name

        // failed return codes cause both objects to be deleted

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
            delete m_pOutput;
            m_pOutput = NULL;
            return NULL;
        }
    }

    /* Find which pin is required */

    switch(n) {
        case 0:
            return m_pInput;
        case 1:
           return m_pOutput;
    }
    return NULL;

} // GetPin


//
// NonDelegatingQueryInterface
//
// Override CUnknown method.
// Part of the basic COM (Compound Object Model) mechanism.
// This is how we expose our interfaces.
//
STDMETHODIMP CNullInPlace::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_INullIPP) {
        return GetInterface((INullIPP *) this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    }
    else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface

// get_IPin
//
// INull method.
// Set *ppPin to the upstream output pin which supplies us
// or to NULL if there is no upstream pin connected to us.
//
STDMETHODIMP CNullInPlace::get_IPin (IPin **ppPin)
{
    CAutoLock l(&m_NullIPLock);
    if (!m_pInput)
    {
        *ppPin = NULL ;
        return NOERROR ;
    }
    if( !m_pInput->IsConnected() )
        *ppPin = NULL ;
    else
    {
        *ppPin = m_pInput->GetConnected () ;
        (*ppPin)->AddRef() ;
    }
    return NOERROR ;

} // get_IPin


//
// put_MediaType
//
// INull method.
//
STDMETHODIMP CNullInPlace::put_MediaType(CMediaType *pmt)
{
    CAutoLock l(&m_NullIPLock);

    //
    // if the state of the graph is running, fail the call.
    //
    if (m_State == State_Running)
        return E_UNEXPECTED ;

    //
    // check the source and sink filters like this media type
    //
    if (pmt == NULL)
        m_mtPreferred.InitMediaType () ;
    else {
        IPin *pPin= m_pInput->GetConnected();
        if (pPin) {
            if (pPin->QueryAccept(pmt) != NOERROR) {
                MessageBox(NULL,TEXT("Upstream filter cannot provide this type"),
                           TEXT("Format Selection"),
                           MB_OK | MB_ICONEXCLAMATION);
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
        pPin= m_pOutput->GetConnected();
        if (pPin) {
            if (pPin->QueryAccept(pmt) != NOERROR) {
                MessageBox(NULL,TEXT("Downstream filter cannot accept this type"),
                           TEXT("Format Selection"),
                           MB_OK | MB_ICONEXCLAMATION);
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
        m_mtPreferred = *pmt ;
    }

    //
    // force reconnect of input if the media type of connection does not match.
    //
    if( m_pInput->IsConnected() )
    {
        if (m_pInput->CurrentMediaType()!= m_mtPreferred)
            m_pGraph->Reconnect(m_pInput);
    }
    return NOERROR ;

} // put_MediaType



//
// get_MediaType
//
// INull method.
// Set *pmt to the current preferred media type.
//
STDMETHODIMP CNullInPlace::get_MediaType(CMediaType **pmt)
{
    CAutoLock l(&m_NullIPLock);

    *pmt = &m_mtPreferred ;
    return NOERROR ;

} // get_MediaType


//
// get_State
//
// INull method
// Set *state to the current state of the filter (State_Stopped etc)
//
STDMETHODIMP CNullInPlace::get_State(FILTER_STATE *state)
{
    CAutoLock l(&m_NullIPLock);
    *state = m_State ;
    return NOERROR;

} // get_State



//-----------------------------------------------------------------------------
//                  ISpecifyPropertyPages implementation
//-----------------------------------------------------------------------------


//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CNullInPlace::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_NullIPPropertyPage;

    return NOERROR;

} // GetPages



/******************************Public*Routine******************************\
* exported entry points for registration and
* unregistration (in this case they only call
* through to default implmentations).
*
*
*
* History:
*
\**************************************************************************/
STDAPI
DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI
DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}


// Microsoft C Compiler will give hundreds of warnings about
// unused inline functions in header files.  Try to disable them.
#pragma warning( disable:4514)

//------------------------------------------------------------------------------
// File: NullNull.cpp
//
// Desc: DirectShow sample code - illustrates a minimum filter implementation.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//     Summary
//
// A filter which does nothing except pass the data through.
//
//     Demonstration instructions
//
// First build the sample and get it registered
// Start GraphEdit, which is available in the SDK DXUtils folder. 
// Drag and drop onto the tool any WAV file or any MPEG, AVI or MOV file.
// A graph will be built to render the file.  From the Graph menu select
// Insert filters and insert NullNull.  (If it is not on the list then you
// failed to register it properly.  If it fails to load then you either
// didn't build it properly or the registration does not correctly point to
// the path where nullnull.ax is now found.)
// In the graph displayed, pick on a connection.  Disconnect it (click on
// the incoming arrow and then press the Delete key).
// Create a connection between the same two filters.  (Drag the mouse along
// the same path as the arrow follwed from tail to head).
// If the nullnull filter is not now included, try it with a different
// connection.  (Connections between source filters and parsers normally
// do not work, others normally do.  Why is another story).
//
//
//     Implementation
//
// Does everything possible with the base classes.  That which cannot be so
// done (e.g. creating this filter) is done here.
// The filter has one input pin, one output pin and does its transform
// in-place (i.e. without copying the data) on the push thread (i.e. it
// is called with a buffer, which it "transforms" and gives to the next
// filter downstream.  It is then blocked until that filter returns.
// It then returns to its own caller).  This filter in fact does the null
// transform it merely passes the data on unchanged.
//
//
//    Known problems ("features NOT illustrated by this sample"):
//
// Almost everything!  This is a truly minimal filter.
//
//
//      Files
//
// nullnull.cpp  The entire implementation
// nullnull.def  Lists exports and imports (part of build process)
// nullnull.rc   Contains some "boiler-plate"
//
//     Base classes used (refer to docs for diagram of what they inherit):
//
// CTransInPlaceFilter



#include <streams.h>     // DirectShow (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.

// The CLSID used by the minimal, in-place, null filter
// DO NOT REUSE IT.  Run uuidgen.exe to create a new one.
// {08af6540-4f21-11cf-aacb-0020af0b99a3}
DEFINE_GUID(CLSID_NullNull,
0x08af6540, 0x4f21, 0x11cf, 0xaa, 0xcb, 0x00, 0x20, 0xaf, 0x0b, 0x99, 0xa3);


// setup data - allows the self-registration to work.

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ &MEDIATYPE_NULL        // clsMajorType
, &MEDIASUBTYPE_NULL };  // clsMinorType

const AMOVIESETUP_PIN psudPins[] =
{ { L"Input"            // strName
  , FALSE               // bRendered
  , FALSE               // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
, { L"Output"           // strName
  , FALSE               // bRendered
  , TRUE                // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 1                   // nTypes
  , &sudPinTypes        // lpTypes
  }
};


const AMOVIESETUP_FILTER sudNullNull =
{ &CLSID_NullNull                  // clsID
, L"Minimal Null"                 // strName
, MERIT_DO_NOT_USE                // dwMerit
, 2                               // nPins
, psudPins };                     // lpPin

// CNullNull
//
class CNullNull
    : public CTransInPlaceFilter
{

public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

private:

    // Constructor - just calls the base class constructor
    CNullNull(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
        : CTransInPlaceFilter (tszName, punk, CLSID_NullNull, phr)
    { }

    // Overrides the PURE virtual Transform of CTransInPlaceFilter base class
    // This is where the "real work" is done by altering *pSample.
    // We do the Null transform by leaving it alone.
    HRESULT Transform(IMediaSample *pSample){ return NOERROR; }

    // We accept any input type.  We'd return S_FALSE for any we didn't like.
    HRESULT CheckInputType(const CMediaType* mtIn) { return S_OK; }
};



// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
    {   { L"Minimal Null"
        , &CLSID_NullNull
        , CNullNull::CreateInstance
        , NULL
        , &sudNullNull }
    };
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//
// CreateInstance
//
// Provide the way for COM to create a CNullNull object
CUnknown * WINAPI CNullNull::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CNullNull *pNewObject = new CNullNull(NAME("Minimal, in-place, null filter"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
} // CreateInstance


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

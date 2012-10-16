//------------------------------------------------------------------------------
// File: Contrast.cpp
//
// Desc: DirectShow sample code - simple contrast-adjusting transform
//       filter.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//
// What this sample illustrates
//
// A simple transform filter that adjusts contrast on a video stream. We use
// eight bit palettised images exclusively and can increase or decrease the
// contrast. The contrast is adjusted through a custom interface we support.
// We have a property page that uses this interface to respond to user action
//
// Summary
//
// This is a sample transform filter - we have a single input pin and also a
// single output pin. The input pin accepts only palettised eight bit video
// formats and our output pin also provides an eight bit palettised format.
// We will normally be put between a video decoder (AVI or MPEG) and a video
// renderer. We adjust the contrast of the video images sent through us, by
// increasing contrast the colours become more suturated and likewise less
// contrast reduces the colour into some grey shades and finally to all grey
//
//
// Implementation
//
// We adjust contrast using a neat trick with palettes, the colour palette of
// an image effectively determines how the image is interpreted. So in short
// how the value 23 (for example) is turned into an RGB triplet for display.
// By changing the palette we can reduce and increase contrast without doing
// anything to the actual image pixels themselves (which makes us go faster).
// So by adjusting the palette in band we can adjust the pixel interpretion.
//
// The runtime video renderer supports palette changes in band, when we want
// to change palettes we pick the new colours and attach a VIDEOINFO to the
// next image we send to the renderer. It will then take that palette out of
// the sample and realise a new palette in the window for us. The trigger to
// change the palette is through a custom interface this filter provides.
//
// The custom interface we implement is called IContrast, this allows someone
// to increase and decrease contrast levels for a given video. The normal way
// for the interface to be used is to right click on the contrast filter in
// GraphEdit and bring up our property page. This has a slider control on it
// that can be used to adjust the contrast level. Each time the slider moves
// it will ask for a contrast change by calling IContrast. To expose property
// pages a filter implements the ISpecifyPropertyPages interface (it has only
// one method called GetPages which returns the CLSIDs for the property pages)
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// an MPEG, AVI or MOV file into the tool and it will be rendered. Then go to
// the filters in the graph and find the filter (box) titled "Video Renderer"
// Disconnect the renderer from its source filter and insert a contrast in
// between them (the contrast is added to the graph using the Graph Menu and
// selecting Insert Filters, the filter is in the dialog box list). Once the
// contrast filter is inserted run the graph. Right click on the contrast
// box and select properties which brings up a property page that contains a
// slider. The slider can be dragged up and down to adjust the contrast level
//
//
// Files
//
// contprop.cpp         Property page implementation with a slider control
// contprop.h           Class definition for the property page
// contprop.rc          Dialog box template for the property page
// contrast.cpp         Main contrast filter class implementation
// contrast.def         What APIs we import and export from this DLL
// contrast.h           Class definition for the contrast filter
// contuids.h           The transform filter CLSIDs
// icontrst.h           Defines the custom contrast filter interface
// makefile             How to build it...
// resource.h           Microsoft Visual C++ generated resource file
//
//
// Base classes we use
//
// CTransformFilter     A generic transform filter that has a single input
//                      and a single output pin. It has some PURE virtual
//                      methods we must override (such as the Transform
//                      method that does the real work). We also use it's
//                      CUnknown base class to implement IUnknown for the
//                      IContrast and ISpecifyPropertyPages we also support
//
//


#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include <olectl.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#endif
#include "contuids.h"
#include "icontrst.h"
#include "contprop.h"
#include "contrast.h"

#pragma warning(disable:4238)  // nonstd extension used: class rvalue used as lvalue

// setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
    {
        L"Input",           // String pin name
        FALSE,              // Is it rendered
        FALSE,              // Is it an output
        FALSE,              // Allowed none
        FALSE,              // Allowed many
        &CLSID_NULL,        // Connects to filter
        L"Output",          // Connects to pin
        1,                  // Number of types
        &sudPinTypes },     // The pin details
      { L"Output",          // String pin name
        FALSE,              // Is it rendered
        TRUE,               // Is it an output
        FALSE,              // Allowed none
        FALSE,              // Allowed many
        &CLSID_NULL,        // Connects to filter
        L"Input",           // Connects to pin
        1,                  // Number of types
        &sudPinTypes        // The pin details
    }
};


const AMOVIESETUP_FILTER sudContrast =
{
    &CLSID_Contrast,        // Filter CLSID
    L"Video Contrast",      // Filter name
    MERIT_DO_NOT_USE,       // Its merit
    2,                      // Number of pins
    psudPins                // Pin details
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[2] = {

    { L"Video Contrast"
    , &CLSID_Contrast
    , CContrast::CreateInstance
    , NULL
    , &sudContrast }
  ,
    { L"Video Contrast Property Page"
    , &CLSID_ContrastPropertyPage
    , CContrastProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// Constructor
//
CContrast::CContrast(TCHAR *tszName,LPUNKNOWN punk,HRESULT *phr) :
    CTransformFilter(tszName, punk, CLSID_Contrast),
    m_DefaultContrastLevel(0),
    m_ContrastLevel(m_DefaultContrastLevel),
    m_PrevLevel(m_ContrastLevel),
    m_lBufferRequest(1)
{
    ASSERT(tszName);
    ASSERT(phr);

} // Contrast


//
// CreateInstance
//
// Provide the way for COM to create a CContrast object
//
CUnknown * WINAPI CContrast::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CContrast *pNewObject = new CContrast(NAME("Contrast"), punk, phr);
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Reveals IContrast and ISpecifyPropertyPages
//
STDMETHODIMP CContrast::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IContrast) {
        return GetInterface((IContrast *) this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface


//
// Transform
//
// Copy the input sample into the output sample
// Then transform the output sample 'in place'
//
HRESULT CContrast::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    HRESULT hr = Copy(pIn, pOut);
    if (FAILED(hr)) {
        return hr;
    }
    return Transform(pOut);

} // Transform


//
// Copy
//
// Make destination an identical copy of source
//
HRESULT CContrast::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef DEBUG
    long lDestSize	= pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory( (PVOID) pDestBuffer,(PVOID) pSourceBuffer,lSourceSize);

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the Sync point property

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type

    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
	pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the actual data length

    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);
    return NOERROR;

} // Copy


//
// Transform
//
// 'In place' adjust the contrast of this sample
//
HRESULT CContrast::Transform(IMediaSample *pMediaSample)
{
    signed char ContrastLevel;

    // Take a copy, so we dont hold the lock for the whole transform. 
    // Also ensures that we use the same level throughout this!
    {
        CAutoLock cAutoLock(&m_ContrastLock);
        ContrastLevel = m_ContrastLevel;	
    }

    AM_MEDIA_TYPE *pAdjustedType = NULL;

    pMediaSample->GetMediaType(&pAdjustedType);
    if (pAdjustedType != NULL) {
        if (CheckInputType(&CMediaType(*pAdjustedType)) == S_OK) {
            m_pInput->CurrentMediaType() = *pAdjustedType;
	    CoTaskMemFree(pAdjustedType);
	} else {
	    CoTaskMemFree(pAdjustedType);
	    return E_FAIL;
	}
    }

    // Pass on format changes to downstream filters

    if ((pAdjustedType != NULL) || (m_PrevLevel != ContrastLevel)) {
	CMediaType AdjustedType((AM_MEDIA_TYPE) m_pInput->CurrentMediaType());
        HRESULT hr = Transform(&AdjustedType, ContrastLevel);
        if (hr == S_OK) {
            pMediaSample->SetMediaType(&AdjustedType);
	    m_PrevLevel = ContrastLevel;
        } else {
            return hr;
        }
    }
    return NOERROR;

} // Transform


//
// CheckInputType
//
// Check the input type is OK, return an error otherwise
//
HRESULT CContrast::CheckInputType(const CMediaType *mtIn)
{
    // Check this is a VIDEOINFO type

    if (*mtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    // Is this a palettised format

    if (CanChangeContrastLevel(mtIn)) {
    	return NOERROR;
    }
    return E_FAIL;

} // CheckInputType


//
// CheckTransform
//
// To be able to transform the formats must be identical
//
HRESULT CContrast::CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut)
{
    HRESULT hr;
    if (FAILED(hr = CheckInputType(mtIn))) {
	return hr;
    }

    // format must be a VIDEOINFOHEADER
    if (*mtOut->FormatType() != FORMAT_VideoInfo) {
	return E_INVALIDARG;
    }
    
    // formats must be big enough 
    if (mtIn->FormatLength() < sizeof(VIDEOINFOHEADER) ||
	mtOut->FormatLength() < sizeof(VIDEOINFOHEADER))
	return E_INVALIDARG;
    
    VIDEOINFO *pInput = (VIDEOINFO *) mtIn->Format();
    VIDEOINFO *pOutput = (VIDEOINFO *) mtOut->Format();
    if (memcmp(&pInput->bmiHeader,&pOutput->bmiHeader,sizeof(BITMAPINFOHEADER)) == 0) {
	return NOERROR;
    }

    return E_INVALIDARG;
} // CheckTransform


//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CContrast::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();

    ASSERT(pProperties->cbBuffer);

    // If we don't have fixed sized samples we must guess some size

    if (!m_pInput->CurrentMediaType().bFixedSizeSamples) {
        if (pProperties->cbBuffer < 100000) {
            // nothing more than a guess!!
            pProperties->cbBuffer = 100000;
        }
    }

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT( Actual.cBuffers == 1 );

    if (pProperties->cBuffers > Actual.cBuffers ||
            pProperties->cbBuffer > Actual.cbBuffer) {
                return E_FAIL;
    }
    return NOERROR;

} // DecideBufferSize


//
// GetMediaType
//
// I support one type, namely the type of the input pin
// We must be connected to support the single output type
//
HRESULT CContrast::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    // This should never happen

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    // Do we have more items to offer

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = m_pInput->CurrentMediaType();
    return NOERROR;

} // GetMediaType


//
// get_ContrastLevel
//
// Return the current contrast level
//
STDMETHODIMP CContrast::get_ContrastLevel(signed char *ContrastLevel)
{
    CAutoLock cAutoLock(&m_ContrastLock);
    *ContrastLevel = m_ContrastLevel;
    return NOERROR;

} // get_ContrastLevel


//
// put_ContrastLevel
//
// Sets the contrast level for the filter
//
STDMETHODIMP CContrast::put_ContrastLevel(signed char ContrastLevel, unsigned long ChangeTime)
{
    CAutoLock cAutoLock(&m_ContrastLock);
    m_ContrastLevel = ContrastLevel;
    return NOERROR;

} // put_ContrastLevel


//
// put_DefaultContrastLevel
//
// Sets the default contrast level for the filter
//
STDMETHODIMP CContrast::put_DefaultContrastLevel()
{
    CAutoLock cAutoLock(&m_ContrastLock);
    m_ContrastLevel = m_DefaultContrastLevel;
    return NOERROR;

} // put_DefaultContrastLevel


//
// GetPages
//
// This is the sole member of ISpecifyPropertyPages
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CContrast::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_ContrastPropertyPage;
    return NOERROR;

} // GetPages


//
// CanChangeContrastLevel
//
// Check if this is a paletised format
//
BOOL CContrast::CanChangeContrastLevel(const CMediaType *pMediaType) const
{
    if ((IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video))
         && (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB8))) {

        // I think I can process this format (8 bit palettised)
	// So do a quick sanity check on the palette information

	VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();
	return (pvi->bmiHeader.biBitCount == 8);

    } else {
        return FALSE;
    }

} // CanChangeContrastLevel


//
// Transform
//
// Adjust the palette entries of pType to reflect the specified contrast level
//
HRESULT CContrast::Transform(AM_MEDIA_TYPE *pType, const signed char ContrastLevel) const
{
    VIDEOINFO *pvi = (VIDEOINFO *) pType->pbFormat;

    if (ContrastLevel >= 0) {
        int Low	= 0 + m_ContrastLevel;
        int High	= 255 - m_ContrastLevel;
	float Grad	= ((float)(High - Low)) / 255;

        for (UINT i = 0; i < pvi->bmiHeader.biClrUsed; i++) {

            IncreaseContrast(&pvi->bmiColors[i], Low, High, Grad);
        }
    } else {
        float Grad = 255 / (255 + (float) ContrastLevel + (float) ContrastLevel);

        for (UINT i = 0; i < pvi->bmiHeader.biClrUsed; i++) {
            DecreaseContrast(&pvi->bmiColors[i], ContrastLevel, Grad);
        }
    }
    return NOERROR;

} // Transform


//
// IncreaseContrast
//
// Adjust the contrast of this palette element
//
inline
void CContrast::IncreaseContrast(RGBQUAD *pElem, const int Low, const int High, const float Grad) const
{
    IncreaseContrast(&pElem->rgbRed, Low, High, Grad);
    IncreaseContrast(&pElem->rgbGreen, Low, High, Grad);
    IncreaseContrast(&pElem->rgbBlue, Low, High, Grad);

} // IncreaseContrast


//
// IncreaseContrast
//
// Change this byte's contrast
//
inline
void CContrast::IncreaseContrast(BYTE *pByte, const int Low, const int High, const float Grad) const
{
    if (*pByte <= Low) {
        *pByte = 0;
    }
    else if ((Low < *pByte) && (*pByte <  High)) {
	*pByte = (BYTE)( (*pByte - Low) / Grad);
    }
    else {	// pElem->rgbGreen >= High
        *pByte = 255;
    }

} // IncreaseContrast


//
// DecreaseContrast
//
// Adjust the contrast of this palette element
//
inline
void CContrast::DecreaseContrast(RGBQUAD *pElem, const int Level, const float Grad) const
{
    DecreaseContrast(&pElem->rgbRed, Level, Grad);
    DecreaseContrast(&pElem->rgbGreen, Level, Grad);
    DecreaseContrast(&pElem->rgbBlue, Level, Grad);

} // DecreaseContrast


//
// DecreaseContrast
//
// Use different maths to calculate the 'decreasing contrast' line
//
inline
void CContrast::DecreaseContrast(BYTE *pByte, const int Level, const float Grad) const
{
    ASSERT(pByte);
    ASSERT(Grad != 0.0);
    *pByte = (BYTE) ( ((int) (*pByte / Grad)) - Level);

} // DecreaseContrast


//
// DllRegisterServer
//
// Handle registration of this filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


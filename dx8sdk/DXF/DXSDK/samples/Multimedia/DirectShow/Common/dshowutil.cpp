//------------------------------------------------------------------------------
// File: DShowUtil.cpp
//
// Desc: DirectShow sample code - utility functions.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <atlbase.h>
#include <dshow.h>
#include <mtype.h>
#include <wxdebug.h>
#include <reftime.h>

#include "dshowutil.h"


HRESULT FindRenderer(IGraphBuilder *pGB, const GUID *mediatype, IBaseFilter **ppFilter)
{
    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    IPin *pPin;
    ULONG ulFetched, ulInPins, ulOutPins;
    BOOL bFound=FALSE;

    // Verify graph builder interface
    if (!pGB)
        return E_NOINTERFACE;

    // Verify that a media type was passed
    if (!mediatype)
        return E_POINTER;

    // Clear the filter pointer in case there is no match
    if (ppFilter)
        *ppFilter = NULL;

    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
        return hr;

    pEnum->Reset();

    // Enumerate all filters in the graph
    while(!bFound && (pEnum->Next(1, &pFilter, &ulFetched) == S_OK))
    {
#ifdef DEBUG
        // Read filter name for debugging purposes
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
    
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (SUCCEEDED(hr))
        {
            // Show filter name in debugger
#ifdef UNICODE
            lstrcpy(szName, FilterInfo.achName);
#else
            WideCharToMultiByte(CP_ACP, 0, FilterInfo.achName, -1, szName, 256, 0, 0);
#endif
            FilterInfo.pGraph->Release();
        }       
#endif

        // Find a filter with one input and no output pins
        hr = CountFilterPins(pFilter, &ulInPins, &ulOutPins);
        if (FAILED(hr))
            break;

        if ((ulInPins == 1) && (ulOutPins == 0))
        {
            // Get the first pin on the filter
            pPin=0;
            pPin = GetInPin(pFilter, 0);

            // Read this pin's major media type
            AM_MEDIA_TYPE type={0};
            hr = pPin->ConnectionMediaType(&type);
            if (FAILED(hr))
                break;

            // Is this pin's media type the requested type?
            // If so, then this is the renderer for which we are searching.
            // Copy the interface pointer and return.
            if (type.majortype == *mediatype)
            {
                // Found our filter
                *ppFilter = pFilter;
                bFound = TRUE;;
            }
            // This is not the renderer, so release the interface.
            else
                pFilter->Release();

            // Delete memory allocated by ConnectionMediaType()
            FreeMediaType(type);
        }
        else
        {
            // No match, so release the interface
            pFilter->Release();
        }
    }

    pEnum->Release();
    return hr;
}

HRESULT FindAudioRenderer(IGraphBuilder *pGB, IBaseFilter **ppFilter)
{
    return FindRenderer(pGB, &MEDIATYPE_Audio, ppFilter);
}

HRESULT FindVideoRenderer(IGraphBuilder *pGB, IBaseFilter **ppFilter)
{
    return FindRenderer(pGB, &MEDIATYPE_Video, ppFilter);
}

HRESULT CountFilterPins(IBaseFilter *pFilter, ULONG *pulInPins, ULONG *pulOutPins)
{
    HRESULT hr=S_OK;
    IEnumPins *pEnum=0;
    ULONG ulFound;
    IPin *pPin;

    // Verify input
    if (!pFilter || !pulInPins || !pulOutPins)
        return E_POINTER;

    // Clear number of pins found
    *pulInPins = 0;
    *pulOutPins = 0;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    pEnum->Reset();

    // Count every pin on the filter
    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        hr = pPin->QueryDirection(&pindir);

        if(pindir == PINDIR_INPUT)
            (*pulInPins)++;
        else
            (*pulOutPins)++;

        pPin->Release();
    } 

    pEnum->Release();
    return hr;
}


HRESULT CountTotalFilterPins(IBaseFilter *pFilter, ULONG *pulPins)
{
    HRESULT hr;
    IEnumPins *pEnum=0;
    ULONG ulFound;
    IPin *pPin;

    // Verify input
    if (!pFilter || !pulPins)
        return E_POINTER;

    // Clear number of pins found
    *pulPins = 0;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    // Count every pin on the filter, ignoring direction
    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        (*pulPins)++;
        pPin->Release();
    } 

    pEnum->Release();
    return hr;
}


HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;
        pPin->QueryDirection(&pindir);
        if(pindir == dirrequired)
        {
            if(iNum == 0)
            {
                *ppPin = pPin;
                // Found requested pin, so clear error
                hr = S_OK;
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    return hr;
}


IPin * GetInPin( IBaseFilter * pFilter, int Num )
{
    CComPtr< IPin > pComPin;
    GetPin(pFilter, PINDIR_INPUT, Num, &pComPin);
    return pComPin;
}


IPin * GetOutPin( IBaseFilter * pFilter, int Num )
{
    CComPtr< IPin > pComPin;
    GetPin(pFilter, PINDIR_OUTPUT, Num, &pComPin);
    return pComPin;
}


HRESULT FindOtherSplitterPin(IPin *pPinIn, GUID guid, int nStream, IPin **ppSplitPin)
{
    if (!ppSplitPin)
        return E_POINTER;

    CComPtr< IPin > pPinOut;
    pPinOut = pPinIn;

    while(pPinOut)
    {
        PIN_INFO ThisPinInfo;
        pPinOut->QueryPinInfo(&ThisPinInfo);
        if(ThisPinInfo.pFilter) ThisPinInfo.pFilter->Release();

        pPinOut = NULL;
        CComPtr< IEnumPins > pEnumPins;
        ThisPinInfo.pFilter->EnumPins(&pEnumPins);
        if(!pEnumPins)
        {
            return NULL;
        }

        // look at every pin on the current filter...
        //
        ULONG Fetched = 0;
        while(1)
        {
            CComPtr< IPin > pPin;
            Fetched = 0;
            ASSERT(!pPin); // is it out of scope?
            pEnumPins->Next(1, &pPin, &Fetched);
            if(!Fetched)
            {
                break;
            }

            PIN_INFO pi;
            pPin->QueryPinInfo(&pi);
            if(pi.pFilter) pi.pFilter->Release();

            // if it's an input pin...
            //
            if(pi.dir == PINDIR_INPUT)
            {
                // continue searching upstream from this pin
                //
                pPin->ConnectedTo(&pPinOut);

                // a pin that supports the required media type is the
                // splitter pin we are looking for!  We are done
                //
            }
            else
            {
                CComPtr< IEnumMediaTypes > pMediaEnum;
                pPin->EnumMediaTypes(&pMediaEnum);
                if(pMediaEnum)
                {
                    Fetched = 0;
                    AM_MEDIA_TYPE *pMediaType;
                    pMediaEnum->Next(1, &pMediaType, &Fetched);
                    if(Fetched)
                    {
                        if(pMediaType->majortype == guid)
                        {
                            if(nStream-- == 0)
                            {
                                DeleteMediaType(pMediaType);
                                *ppSplitPin = pPin;
                                (*ppSplitPin)->AddRef();
                                return S_OK;
                            }
                        }
                        DeleteMediaType(pMediaType);
                    }
                }
            }

            // go try the next pin

        } // while
    }
    ASSERT(FALSE);
    return E_FAIL;
}


HRESULT SeekNextFrame( IMediaSeeking * pSeeking, double FPS, long Frame )
{
    // try seeking by frames first
    //
    HRESULT hr = pSeeking->SetTimeFormat(&TIME_FORMAT_FRAME);
    REFERENCE_TIME Pos = 0;
    if(!FAILED(hr))
    {
        pSeeking->GetCurrentPosition(&Pos);
        Pos++;
    }
    else
    {
        // couldn't seek by frames, use Frame and FPS to calculate time
        //
        Pos = REFERENCE_TIME(double( Frame * UNITS ) / FPS);

        // add a half-frame to seek to middle of the frame
        //
        Pos += REFERENCE_TIME(double( UNITS ) * 0.5 / FPS);
    }

    hr = pSeeking->SetPositions(&Pos, AM_SEEKING_AbsolutePositioning, 
                                NULL, AM_SEEKING_NoPositioning);
    return hr;

}

#ifdef DEBUG
    // for debugging purposes
    const INT iMAXLEVELS = 5;                // Maximum debug categories
    extern DWORD m_Levels[iMAXLEVELS];       // Debug level per category
#endif


void TurnOnDebugDllDebugging( )
{
#ifdef DEBUG
    for(int i = 0 ; i < iMAXLEVELS ; i++)
    {
        m_Levels[i] = 1;
    }
#endif
}

void DbgPrint( char * pText )
{
    DbgLog(( LOG_TRACE, 1, "%s", pText ));
}

void ErrPrint( char * pText )
{
    printf(pText);
    return;
}


// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}

// Removes a filter graph from the Running Object Table
void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}


void ShowFilenameByCLSID(REFCLSID clsid, TCHAR *szFilename)
{
    HRESULT hr;
    LPOLESTR strCLSID;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(clsid, &strCLSID);
    if(SUCCEEDED(hr))
    {
        TCHAR szKey[512];
        CString strQuery(strCLSID);

        // Create key name for reading filename registry
        wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                 strQuery);

        // Free memory associated with strCLSID (allocated in StringFromCLSID)
        CoTaskMemFree(strCLSID);

        HKEY hkeyFilter=0;
        DWORD dwSize=MAX_PATH;
        BYTE szFile[MAX_PATH];
        int rc=0;

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, szFile, &dwSize);

            if (rc == ERROR_SUCCESS)
                wsprintf(szFilename, TEXT("%s"), szFile);
            else
                wsprintf(szFilename, TEXT("<Unknown>\0"));

            rc = RegCloseKey(hkeyFilter);
        }
    }
}


HRESULT GetFileDurationString(IMediaSeeking *pMS, TCHAR *szDuration)
{
    HRESULT hr;

    if (!pMS)
        return E_NOINTERFACE;
    if (!szDuration)
        return E_POINTER;

    // Initialize the display in case we can't read the duration
    wsprintf(szDuration, TEXT("<00:00.000>"));

    // Is media time supported for this file?
    if (S_OK != pMS->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME))
        return E_NOINTERFACE;

    // Read the time format to restore later
    GUID guidOriginalFormat;
    hr = pMS->GetTimeFormat(&guidOriginalFormat);
    if (FAILED(hr))
        return hr;

    // Ensure media time format for easy display
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
    if (FAILED(hr))
        return hr;

    // Read the file's duration
    LONGLONG llDuration;
    hr = pMS->GetDuration(&llDuration);
    if (FAILED(hr))
        return hr;

    // Return to the original format
    if (guidOriginalFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = pMS->SetTimeFormat(&guidOriginalFormat);
        if (FAILED(hr))
            return hr;
    }

    // Convert the LONGLONG duration into human-readable format
    unsigned long nTotalMS = (unsigned long) llDuration / 10000; // 100ns -> ms
    int nMS = nTotalMS % 1000;
    int nSeconds = nTotalMS / 1000;
    int nMinutes = nSeconds / 60;
    nSeconds %= 60;

    // Update the string
    wsprintf(szDuration, _T("%02dm:%02d.%03ds\0"), nMinutes, nSeconds, nMS);

    return hr;
}


BOOL SupportsPropertyPage(IBaseFilter *pFilter) 
{
    HRESULT hr;
    ISpecifyPropertyPages *pSpecify;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        pSpecify->Release();
        return TRUE;
    }
    else
        return FALSE;
}


HRESULT ShowFilterPropertyPage(IBaseFilter *pFilter, HWND hwndParent)
{
    HRESULT hr;
    ISpecifyPropertyPages *pSpecify=0;

    if (!pFilter)
        return E_NOINTERFACE;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        do 
        {
            FILTER_INFO FilterInfo;
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            if (FAILED(hr))
                break;

            CAUUID caGUID;
            hr = pSpecify->GetPages(&caGUID);
            if (FAILED(hr))
                break;

            pSpecify->Release();
        
            // Display the filter's property page
            OleCreatePropertyFrame(
                hwndParent,             // Parent window
                0,                      // x (Reserved)
                0,                      // y (Reserved)
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of filters
                (IUnknown **)&pFilter,  // Pointer to the filter 
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Pointer to property page CLSIDs
                0,                      // Locale identifier
                0,                      // Reserved
                NULL                    // Reserved
            );
            CoTaskMemFree(caGUID.pElems);
            FilterInfo.pGraph->Release(); 

        } while(0);
    }

    pFilter->Release();
    return hr;
}


//
// Some hardware decoders and video renderers support stepping media
// frame by frame with the IVideoFrameStep interface.  See the interface
// documentation for more details on frame stepping.
//
BOOL CanFrameStep(IGraphBuilder *pGB)
{
    HRESULT hr;
    IVideoFrameStep* pFS;

    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFS);
    if (FAILED(hr))
        return FALSE;

    // Check if this decoder can step
    hr = pFS->CanStep(0L, NULL); 

    pFS->Release();

    if (hr == S_OK)
        return TRUE;
    else
        return FALSE;
}



//------------------------------------------------------------------------------
// File: MFCUtil.cpp
//
// Desc: DirectShow sample code - Utility functions used by MFC applications.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <dshow.h>
#include <atlbase.h>
#include "mfcutil.h"


HRESULT AddGraphFiltersToList (IGraphBuilder *pGB, CListBox& m_ListFilters) 
{
    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;

    // Clear filters list box
    m_ListFilters.ResetContent();
    
    // Verify graph builder interface
    if (!pGB)
        return E_NOINTERFACE;

    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
    {
        m_ListFilters.AddString(TEXT("<ERROR>"));
        return hr;
    }

    // Enumerate all filters in the graph
    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            m_ListFilters.AddString(TEXT("<ERROR>"));
        }
        else
        {
            USES_CONVERSION;

            // Add the filter name to the filters listbox
            lstrcpy(szName, W2T(FilterInfo.achName));
            m_ListFilters.AddString(szName);

            // Must release filter graph reference
            FilterInfo.pGraph->Release();
        }       
        pFilter->Release();
    }
    pEnum->Release();

    return hr;
}


//
// The GraphBuilder interface provides a FindFilterByName() method,
// which provides similar functionality to the method below.
// This local method is provided for educational purposes.
//
IBaseFilter *FindFilterFromName(IGraphBuilder *pGB, LPTSTR szNameToFind)
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;
    BOOL bFound = FALSE;

    // Verify graph builder interface
    if (!pGB)
        return NULL;

    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
        return NULL;

    // Enumerate all filters in the graph
    while((pEnum->Next(1, &pFilter, &cFetched) == S_OK) && (!bFound))
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            pFilter->Release();
            pEnum->Release();
            return NULL;
        }

        // Compare this filter's name with the one we want
        lstrcpy(szName, W2T(FilterInfo.achName));
        if (! lstrcmp(szName, szNameToFind))
        {
            bFound = TRUE;
        }

        FilterInfo.pGraph->Release();

        // If we found the right filter, don't release its interface.
        // The caller will use it and release it later.
        if (!bFound)
            pFilter->Release();
        else
            break;
    }
    pEnum->Release();

    return (bFound ? pFilter : NULL);
}


HRESULT EnumFiltersWithCLSIDToList(ICreateDevEnum *pSysDevEnum, 
                                   const GUID *clsid, CListBox& List)
{
    HRESULT hr;
    IEnumMoniker *pEnumCat = NULL;

    // Instantiate the system device enumerator if it doesn't exist
    if (pSysDevEnum == NULL)
    {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                              CLSCTX_INPROC, IID_ICreateDevEnum, 
                              (void **)&pSysDevEnum);
        if FAILED(hr)
            return hr;
    }

    // Enumerate all filters of the selected category  
    hr = pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    if (SUCCEEDED(hr))
    {
        // Enumerate all filters using the category enumerator
        hr = EnumFiltersAndCLSIDsToList(pEnumCat, List);

        SAFE_RELEASE(pEnumCat);
    }

    pSysDevEnum->Release();
    return hr;
}


HRESULT EnumFiltersWithMonikerToList(ICreateDevEnum *pSysDevEnum, 
                                     const GUID *clsid, CListBox& List)
{
    HRESULT hr;
    IEnumMoniker *pEnumCat = NULL;

    // Instantiate the system device enumerator if it doesn't exist
    if (pSysDevEnum == NULL)
    {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                              CLSCTX_INPROC, IID_ICreateDevEnum, 
                              (void **)&pSysDevEnum);
        if FAILED(hr)
            return hr;
    }

    // Enumerate all filters of the selected category  
    hr = pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    if (SUCCEEDED(hr))
    {
        // Enumerate all filters using the category enumerator
        hr = EnumFiltersAndMonikersToList(pEnumCat, List);

        SAFE_RELEASE(pEnumCat);
    }

    pSysDevEnum->Release();
    return hr;
}


HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CListBox& ListFilters)
{
    HRESULT hr=S_OK;
    IMoniker *pMoniker;
    ULONG cFetched;
    VARIANT varName={0};
    int nFilters=0;

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        ListFilters.AddString(TEXT("<< No entries >>"));
        return S_FALSE;
    }

    // Enumerate all items associated with the moniker
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // Associate moniker with a file
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);
        ASSERT(SUCCEEDED(hr));
        ASSERT(pPropBag);
        if (FAILED(hr))
            continue;

        // Read filter name from property bag
        varName.vt = VT_BSTR;
        hr = pPropBag->Read(L"FriendlyName", &varName, 0);
        if (FAILED(hr))
            continue;

        // Get filter name (converting BSTR name to a CString)
        CString str(varName.bstrVal);
        SysFreeString(varName.bstrVal);
        nFilters++;

        // Add the filter name and moniker to the listbox
        AddFilterToListWithMoniker(str, pMoniker, ListFilters);
       
        // Cleanup interfaces
        SAFE_RELEASE(pPropBag);

        // Intentionally DO NOT release the pMoniker, since it is
        // stored in a listbox for later use
    }

    return hr;
}


HRESULT EnumFiltersAndCLSIDsToList(IEnumMoniker *pEnumCat, CListBox& ListFilters)
{
    HRESULT hr=S_OK;
    IMoniker *pMoniker;
    ULONG cFetched;
    VARIANT varName={0};
    int nFilters=0;

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        ListFilters.AddString(TEXT("<< No entries >>"));
        return S_FALSE;
    }

    // Enumerate all items associated with the moniker
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // Associate moniker with a file
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);
        ASSERT(SUCCEEDED(hr));
        ASSERT(pPropBag);
        if (FAILED(hr))
            continue;

        // Read filter name from property bag
        varName.vt = VT_BSTR;
        hr = pPropBag->Read(L"FriendlyName", &varName, 0);
        if (FAILED(hr))
            continue;

        // Get filter name (converting BSTR name to a CString)
        CString str(varName.bstrVal);
        SysFreeString(varName.bstrVal);
        nFilters++;

        // Read filter's CLSID from property bag.  This CLSID string will be
        // converted to a binary CLSID and passed to AddFilter(), which will
        // add the filter's name to the listbox and its CLSID to the listbox
        // item's DataPtr item.  When the user clicks on a filter name in
        // the listbox, we'll read the stored CLSID, convert it to a string,
        // and use it to find the filter's filename in the registry.
        VARIANT varFilterClsid;
        varFilterClsid.vt = VT_BSTR;

        // Read CLSID string from property bag
        hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
        if(SUCCEEDED(hr))
        {
            CLSID clsidFilter;

            // Add filter name and CLSID to listbox
            if(CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter) == S_OK)
            {
                AddFilterToListWithCLSID(str, &clsidFilter, ListFilters);
            }

            SysFreeString(varFilterClsid.bstrVal);
        }
       
        // Cleanup interfaces
        SAFE_RELEASE(pPropBag);
        SAFE_RELEASE(pMoniker);
    }

    return hr;
}


HRESULT EnumPinsOnFilter (IBaseFilter *pFilter, PIN_DIRECTION PinDir,
                          CListBox& Listbox)
{
    HRESULT hr;
    IEnumPins  *pEnum = NULL;
    IPin *pPin = NULL;

    // Clear the specified listbox (input or output)
    Listbox.ResetContent();

    // Verify filter interface
    if (!pFilter)
        return E_NOINTERFACE;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        Listbox.AddString(TEXT("<ERROR>"));
        return hr;
    }

    pEnum->Reset();

    // Enumerate all pins on this filter
    while((hr = pEnum->Next(1, &pPin, 0)) == S_OK)
    {
        PIN_DIRECTION PinDirThis;

        hr = pPin->QueryDirection(&PinDirThis);
        if (FAILED(hr))
        {
            Listbox.AddString(TEXT("<ERROR>"));
            pPin->Release();
            continue;
        }

        // Does the pin's direction match the requested direction?
        if (PinDir == PinDirThis)
        {
            PIN_INFO pininfo={0};

            // Direction matches, so add pin name to listbox
            hr = pPin->QueryPinInfo(&pininfo);
            if (SUCCEEDED(hr))
            {
                CString str(pininfo.achName);
                Listbox.AddString(str);
                //RetailOutput(_T("pPin=0x%x\n"), pPin);
            }

            // The pininfo structure contains a reference to an IBaseFilter,
            // so you must release its reference to prevent resource a leak.
            pininfo.pFilter->Release();
        }
        pPin->Release();
    }
    pEnum->Release();

    return hr;
}


HRESULT AddFilterPinsToLists (IGraphBuilder *pGB, CListBox& m_ListFilters,
                             CListBox& m_ListPinsInput, CListBox& m_ListPinsOutput) 
{
    HRESULT hr;
    IBaseFilter *pFilter = NULL;
    TCHAR szNameToFind[128];

    // Read the current filter name from the list box
    int nCurSel = m_ListFilters.GetCurSel();
    m_ListFilters.GetText(nCurSel, szNameToFind);

    // Read the current list box name and find it in the graph
    pFilter = FindFilterFromName(pGB, szNameToFind);
    if (!pFilter)
        return E_UNEXPECTED;

    // Now that we have filter information, enumerate pins by direction
    // and add their names to the appropriate listboxes
    //RetailOutput(_T("Listing input pins on filter %s\n"), szNameToFind);
    hr = EnumPinsOnFilter(pFilter, PINDIR_INPUT, m_ListPinsInput);

    if (SUCCEEDED(hr))
    {
        //RetailOutput(_T("Listing output pins on filter %s\n"), szNameToFind);
        hr = EnumPinsOnFilter(pFilter, PINDIR_OUTPUT, m_ListPinsOutput);
    }
    
    // Must release the filter interface returned from FindFilterByName()
    pFilter->Release();

    return hr;
}


BOOL GetFileInfoStrings(LPTSTR szFile, TCHAR *szSize, TCHAR *szDate)
{
    HANDLE hFile;
    LONGLONG llSize=0;
    DWORD dwSizeLow=0, dwSizeHigh=0;

    // Open the specified file to read size and date information
    hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, (DWORD) 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        RetailOutput(TEXT("*** Failed(0x%x) to open file (to read size)!\r\n"),
                     GetLastError());
        return FALSE;
    }

    if (szSize)
    {
        dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
        if ((dwSizeLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        {
            RetailOutput(TEXT("*** Error(0x%x) reading file size!\r\n"),
                         GetLastError());
            CloseHandle(hFile);
            return FALSE;
        }

        // Large files will also use the upper DWORD to report size.
        // Add them together for the true size if necessary.
        if (dwSizeHigh)
            llSize = (dwSizeHigh << 16) + dwSizeLow;
        else
            llSize = dwSizeLow;

        // Update size string
        wsprintf(szSize, TEXT("Size: %d bytes\0"), dwSizeLow);
    }

    if (szDate)
    {
        // Read date information
        BY_HANDLE_FILE_INFORMATION fi;
        if (GetFileInformationByHandle(hFile, &fi))
        {
            CTime time(fi.ftLastWriteTime);

            // Update date string
            wsprintf(szDate, TEXT("File date: %02d/%02d/%d\0"), 
                     time.GetMonth(), time.GetDay(), time.GetYear());
        }
    }

    CloseHandle(hFile);
    return TRUE;
}


void AddEventToList(CListBox& ListEvents, long lEventCode)
{
    static TCHAR szMsg[256];
    BOOL bMatch = TRUE;

#define HANDLE_EC(c)                              \
    case c:                                       \
        wsprintf(szMsg, TEXT("%s\0"), TEXT(#c));  \
        break;

    switch (lEventCode)
    {
        HANDLE_EC(EC_ACTIVATE);
        HANDLE_EC(EC_BUFFERING_DATA);
        HANDLE_EC(EC_CLOCK_CHANGED);
        HANDLE_EC(EC_COMPLETE);
        HANDLE_EC(EC_DEVICE_LOST);
        HANDLE_EC(EC_DISPLAY_CHANGED);
        HANDLE_EC(EC_END_OF_SEGMENT);
        HANDLE_EC(EC_ERROR_STILLPLAYING);
        HANDLE_EC(EC_ERRORABORT);
        HANDLE_EC(EC_EXTDEVICE_MODE_CHANGE);
        HANDLE_EC(EC_FULLSCREEN_LOST);
        HANDLE_EC(EC_GRAPH_CHANGED);
        HANDLE_EC(EC_LENGTH_CHANGED);
        HANDLE_EC(EC_NEED_RESTART);
        HANDLE_EC(EC_NOTIFY_WINDOW);
        HANDLE_EC(EC_OLE_EVENT);
        HANDLE_EC(EC_OPENING_FILE);
        HANDLE_EC(EC_PALETTE_CHANGED);
        HANDLE_EC(EC_PAUSED);
        HANDLE_EC(EC_QUALITY_CHANGE);
        HANDLE_EC(EC_REPAINT);
        HANDLE_EC(EC_SEGMENT_STARTED);
        HANDLE_EC(EC_SHUTTING_DOWN);
        HANDLE_EC(EC_SNDDEV_IN_ERROR);
        HANDLE_EC(EC_SNDDEV_OUT_ERROR);
        HANDLE_EC(EC_STARVATION);
        HANDLE_EC(EC_STEP_COMPLETE);
        HANDLE_EC(EC_STREAM_CONTROL_STARTED);
        HANDLE_EC(EC_STREAM_CONTROL_STOPPED);
        HANDLE_EC(EC_STREAM_ERROR_STILLPLAYING);
        HANDLE_EC(EC_STREAM_ERROR_STOPPED);
        HANDLE_EC(EC_TIMECODE_AVAILABLE);
        HANDLE_EC(EC_USERABORT);
        HANDLE_EC(EC_VIDEO_SIZE_CHANGED);
        HANDLE_EC(EC_WINDOW_DESTROYED);

    default:
        bMatch = FALSE;
        RetailOutput(TEXT("  Received unknown event code (0x%x)\r\n"), lEventCode);
        break;
    }

    // If a recognized event was found, add its name to the events list box
    if (bMatch)
        ListEvents.AddString(szMsg);
}


void RetailOutput(TCHAR *tszErr, ...)
{
    TCHAR tszErrOut[MAX_PATH + 256];

    va_list valist;

    va_start(valist,tszErr);
    wvsprintf(tszErrOut, tszErr, valist);
    OutputDebugString(tszErrOut);
    va_end (valist);
}


void AddFilterCategoriesToList(CListBox& ListFilters)
{
    HRESULT hr;
    IEnumMoniker *pEmCat = 0;
    ICreateDevEnum *pCreateDevEnum = NULL;
    int nClasses=0;

    // Create an enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return;

    // Use the meta-category that contains a list of all categories.
    // This emulates the behavior of GraphEdit.
    hr = pCreateDevEnum->CreateClassEnumerator(
                         CLSID_ActiveMovieCategories, &pEmCat, 0);
    ASSERT(SUCCEEDED(hr));

    if(hr == S_OK)
    {
        IMoniker *pMCat;
        ULONG cFetched;

        // Enumerate over every category
        while(hr = pEmCat->Next(1, &pMCat, &cFetched),
              hr == S_OK)
        {
            IPropertyBag *pPropBag;

            // Associate moniker with a file
            hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                VARIANT varCatClsid;
                varCatClsid.vt = VT_BSTR;

                // Read CLSID string from property bag
                hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
                if(SUCCEEDED(hr))
                {
                    CLSID clsidCat;

                    if(CLSIDFromString(varCatClsid.bstrVal, &clsidCat) == S_OK)
                    {
                        USES_CONVERSION;

                        // Use the guid if we can't get the name
                        WCHAR *wszCatName;
                        TCHAR szCatDesc[MAX_PATH];

                        VARIANT varCatName;
                        varCatName.vt = VT_BSTR;

                        // Read filter name
                        hr = pPropBag->Read(L"FriendlyName", &varCatName, 0);
                        if(SUCCEEDED(hr))
                            wszCatName = varCatName.bstrVal;
                        else
                            wszCatName = varCatClsid.bstrVal;

                        lstrcpy(szCatDesc, W2T(wszCatName));

                        if(SUCCEEDED(hr))
                            SysFreeString(varCatName.bstrVal);

                        // Add category name and CLSID to list box
                        AddFilterToListWithCLSID(szCatDesc, &clsidCat, ListFilters);
                        nClasses++;
                    }

                    SysFreeString(varCatClsid.bstrVal);
                }

                pPropBag->Release();
            }
            else
            {
                break;
            }

            pMCat->Release();
        } // for loop

        pEmCat->Release();
    }

    pCreateDevEnum->Release();
}


void AddFilterToListWithCLSID(const TCHAR *szFilterName, const GUID *pCatGuid, CListBox& ListFilters)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;

    // Add the category name and a pointer to its CLSID to the list box
    int nSuccess  = ListFilters.AddString(szFilterName);
    int nIndexNew = ListFilters.FindStringExact(-1, szFilterName);

    if (pclsid)
    {
        *pclsid = *pCatGuid;
        nSuccess = ListFilters.SetItemDataPtr(nIndexNew, pclsid);
    }
}


void ClearFilterListWithCLSID(CListBox& ListFilters)
{
    CLSID *pStoredId = NULL;
    
    int nCount = ListFilters.GetCount();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (int i=0; i < nCount; i++)
    {
        pStoredId = (CLSID *) ListFilters.GetItemDataPtr(i);
        if (pStoredId != 0)
        {
            delete pStoredId;
            pStoredId = NULL;
        }
    }

    // Clean up
    ListFilters.ResetContent();
}


void AddFilterToListWithMoniker(const TCHAR *szFilterName, 
                                IMoniker *pMoniker, CListBox& ListFilters)
{
    // Add the category name and a pointer to its CLSID to the list box
    int nSuccess  = ListFilters.AddString(szFilterName);
    int nIndexNew = ListFilters.FindStringExact(-1, szFilterName);
    nSuccess = ListFilters.SetItemDataPtr(nIndexNew, pMoniker);
}


void ClearFilterListWithMoniker(CListBox& ListFilters)
{
    IMoniker *pMoniker = NULL;
    
    int nCount = ListFilters.GetCount();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (int i=0; i < nCount; i++)
    {
        pMoniker = (IMoniker *) ListFilters.GetItemDataPtr(i);
        if (pMoniker != 0)
        {
            pMoniker->Release();
            pMoniker = NULL;
        }
    }

    // Clean up
    ListFilters.ResetContent();
}


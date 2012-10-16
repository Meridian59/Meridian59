//------------------------------------------------------------------------------
// File: MFCDMOUtil.cpp
//
// Desc: DirectShow sample code - DMO utility functions used by MFC applications.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <dshow.h>
#include <dmo.h>

#include "namedguid.h"
#include "mfcdmoutil.h"
#include "mfcutil.h"


HRESULT AddDMOsToList(const GUID *clsid, CListBox& ListFilters, BOOL bIncludeKeyed) 
{
    HRESULT hr;    
    IEnumDMO *pEnum = NULL;
    int nFilters=0;
    DWORD dwFlags = bIncludeKeyed ? DMO_ENUMF_INCLUDE_KEYED : 0;

    // Enumerate all DMOs of the selected category  
    hr = DMOEnum(*clsid, dwFlags, 0, NULL, 0, NULL, &pEnum);
    if (FAILED(hr))
        return hr;

    // Enumerate all filters using the category enumerator
    hr = EnumDMOsToList(pEnum, ListFilters, nFilters);

    // Now that the DMOs (if any) are enumerated and added 
    // to the list, go ahead and select the first one.
    ListFilters.SetCurSel(0);
    pEnum->Release();
    return hr;
}


HRESULT EnumDMOsToList(IEnumDMO *pEnumCat, CListBox& ListFilters, int& nFilters)
{
    HRESULT hr=S_OK;
    ULONG cFetched;
    WCHAR *wszName;
    CLSID clsid;

    // Clear the current filter list
    ClearFilterListWithCLSID(ListFilters);
    nFilters = 0;

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        ListFilters.AddString(TEXT("<< No entries >>"));
        return S_FALSE;
    }

    // Enumerate all items associated with the moniker
    while(pEnumCat->Next(1, &clsid, &wszName, &cFetched) == S_OK)
    {
        nFilters++;
        CString str(wszName);

        // Add filter's name and CLSID to the list box
        AddFilterToListWithCLSID(str, &clsid, ListFilters);
        CoTaskMemFree(wszName);
    }

    // If no DMOs matched the query, show a default item
    if (nFilters == 0)
        ListFilters.AddString(TEXT("<< No entries >>"));

    return hr;
}


void DisplayDMOTypeInfo(const GUID *pCLSID, 
                        ULONG& ulNumInputsSupplied,  CListBox& ListInputTypes,
                        ULONG& ulNumOutputsSupplied, CListBox& ListOutputTypes)
{
    const int NUM_PAIRS=20;
    HRESULT hr;
    DMO_PARTIAL_MEDIATYPE aInputTypes[NUM_PAIRS]={0}, 
                          aOutputTypes[NUM_PAIRS]={0};
    ULONG ulNumInputTypes=NUM_PAIRS, ulNumOutputTypes=NUM_PAIRS, i;
    TCHAR szCLSID[128];

    // Read type/subtype information
    hr = DMOGetTypes(
        *pCLSID,
        ulNumInputTypes,  &ulNumInputsSupplied,  aInputTypes,
        ulNumOutputTypes, &ulNumOutputsSupplied, aOutputTypes);

    if (FAILED(hr))
        return;

    // Show input type/subtype pairs
    for (i=0; i<ulNumInputsSupplied; i++)
    {
        GetTypeSubtypeString(szCLSID, aInputTypes[i]);
        ListInputTypes.AddString(szCLSID);
    }

    // Show output type/subtype pairs
    for (i=0; i<ulNumOutputsSupplied; i++)
    {
        GetTypeSubtypeString(szCLSID, aOutputTypes[i]);
        ListOutputTypes.AddString(szCLSID);
    }
}


//------------------------------------------------------------------------------
// File: MFCUtil.h
//
// Desc: DirectShow sample code - prototypes for utility functions
//       used by MFC applications.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
// Application-defined messages
//
#define WM_GRAPHNOTIFY  WM_APP + 1

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)  if(x) {x->Release(); x=0;}
#endif

#define JIF(x) if (FAILED(hr=(x))) \
    {RetailOutput(TEXT("FAILED(0x%x) ") TEXT(#x) TEXT("\n"), hr); goto CLEANUP;}

//
//  Function prototypes
//
HRESULT AddGraphFiltersToList (IGraphBuilder *pGB, CListBox& m_ListFilters);
HRESULT AddFilterPinsToLists  (IGraphBuilder *pGB, CListBox& m_ListFilters, 
                               CListBox& m_ListPinsInput, CListBox& m_ListPinsOutput);

HRESULT EnumPinsOnFilter (IBaseFilter *pFilter, PIN_DIRECTION PinDir, CListBox& Listbox);

// Enumeration functions that add extra data to the listbox
HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CListBox& ListFilters);
HRESULT EnumFiltersAndCLSIDsToList(IEnumMoniker *pEnumCat, CListBox& ListFilters);

HRESULT EnumFiltersWithCLSIDToList(ICreateDevEnum *pSysDevEnum, 
                                   const GUID *clsid, CListBox& List);
HRESULT EnumFiltersWithMonikerToList(ICreateDevEnum *pSysDevEnum, 
                                   const GUID *clsid, CListBox& List);

// Similar to IGraphBuilder::FindFilterByName()
IBaseFilter *FindFilterFromName(IGraphBuilder *pGB, LPTSTR szNameToFind);

// Listbox add/clear functions that store extra data
void AddFilterToListWithCLSID  (const TCHAR *szFilterName, 
                                const GUID *pCatGuid, CListBox& ListFilters);
void AddFilterToListWithMoniker(const TCHAR *szFilterName, 
                                IMoniker *pMoniker, CListBox& ListFilters);
void ClearFilterListWithCLSID  (CListBox& ListFilters);
void ClearFilterListWithMoniker(CListBox& ListFilters);

// Listbox manipulation functions
void AddEventToList(CListBox& ListEvents, long lEventCode);
void AddFilterCategoriesToList(CListBox& ListFilters);

// Debug output
void RetailOutput(TCHAR *tszErr, ...);

BOOL GetFileInfoStrings(LPTSTR szFile, TCHAR *szSize, TCHAR *szDate);

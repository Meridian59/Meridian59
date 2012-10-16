//------------------------------------------------------------------------------
// File: Dump.cpp
//
// Desc: DirectShow sample code - implementation of a renderer that dumps
//       the samples it receives into a text file.
//
// Copyright (c) 1992-2001  Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// Summary
//
// We are a generic renderer that can be attached to any data stream that
// uses IMemInputPin data transport. For each sample we receive we write
// its contents including its properties into a dump file. The file we
// will write into is specified when the dump filter is created. GraphEdit
// creates a file open dialog automatically when it sees a filter being
// created that supports the IFileSinkFilter interface.
//
//
// Implementation
//
// Pretty straightforward really, we have our own input pin class so that
// we can override Receive, all that does is to write the properties and
// data into a raw data file (using the Write function). We don't keep
// the file open when we are stopped so the flags to the open function
// ensure that we open a file if already there otherwise we create it.
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// an MPEG, AVI or MOV file into the tool and it will be rendered. Then go to
// the filters in the graph and find the filter (box) titled "Video Renderer"
// This is the filter we will be replacing with the dump renderer. Then click
// on the box and hit DELETE. After that go to the Graph menu and select the
// "Insert Filters", from the dialog box find and select the "Dump Filter".
//
// You will be asked to supply a filename where you would like to have the
// data dumped, the data we receive in this filter is dumped in text form.
// Then dismiss the dialog. Back in the graph layout find the output pin of
// the filter that used to be connected to the input of the video renderer
// you just deleted, right click and do "Render". You should see it being
// connected to the input pin of the dump filter you just inserted.
//
// Click Pause and Run and then a little later stop on the GraphEdit frame and
// the data being passed to the renderer will be dumped into a file. Stop the
// graph and dump the filename that you entered when inserting the filter into
// the graph, the data supplied to the renderer will be displayed as raw data
//
//
// Files
//
// dump.cpp             Main implementation of the dump renderer
// dump.def             What APIs the DLL will import and export
// dump.h               Class definition of the derived renderer
// dump.rc              Version information for the sample DLL
// dumpuids.h           CLSID for the dump filter
// makefile             How to build it...
//
//
// Base classes used
//
// CBaseFilter          Base filter class supporting IMediaFilter
// CRenderedInputPin    An input pin attached to a renderer
// CUnknown             Handle IUnknown for our IFileSinkFilter
// CPosPassThru         Passes seeking interfaces upstream
// CCritSec             Helper class that wraps a critical section
//
//

#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include "dumpuids.h"
#include "dump.h"


// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,            // Major type
    &MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
    L"Input",                   // Pin string name
    FALSE,                      // Is it rendered
    FALSE,                      // Is it an output
    FALSE,                      // Allowed none
    FALSE,                      // Likewise many
    &CLSID_NULL,                // Connects to filter
    L"Output",                  // Connects to pin
    1,                          // Number of types
    &sudPinTypes                // Pin information
};

const AMOVIESETUP_FILTER sudDump =
{
    &CLSID_Dump,                // Filter CLSID
    L"Dump",                    // String name
    MERIT_DO_NOT_USE,           // Filter merit
    1,                          // Number pins
    &sudPins                    // Pin details
};


//
//  Object creation stuff
//
CFactoryTemplate g_Templates[]= {
    L"Dump", &CLSID_Dump, CDump::CreateInstance, NULL, &sudDump
};
int g_cTemplates = 1;


// Constructor

CDumpFilter::CDumpFilter(CDump *pDump,
                         LPUNKNOWN pUnk,
                         CCritSec *pLock,
                         HRESULT *phr) :
    CBaseFilter(NAME("CDumpFilter"), pUnk, pLock, CLSID_Dump),
    m_pDump(pDump)
{
}


//
// GetPin
//
CBasePin * CDumpFilter::GetPin(int n)
{
    if (n == 0) {
        return m_pDump->m_pPin;
    } else {
        return NULL;
    }
}


//
// GetPinCount
//
int CDumpFilter::GetPinCount()
{
    return 1;
}


//
// Stop
//
// Overriden to close the dump file
//
STDMETHODIMP CDumpFilter::Stop()
{
    CAutoLock cObjectLock(m_pLock);
    m_pDump->CloseFile();
    return CBaseFilter::Stop();
}


//
// Pause
//
// Overriden to open the dump file
//
STDMETHODIMP CDumpFilter::Pause()
{
    CAutoLock cObjectLock(m_pLock);
    m_pDump->OpenFile();
    return CBaseFilter::Pause();
}


//
// Run
//
// Overriden to open the dump file
//
STDMETHODIMP CDumpFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);
    m_pDump->OpenFile();
    return CBaseFilter::Run(tStart);
}


//
//  Definition of CDumpInputPin
//
CDumpInputPin::CDumpInputPin(CDump *pDump,
                             LPUNKNOWN pUnk,
                             CBaseFilter *pFilter,
                             CCritSec *pLock,
                             CCritSec *pReceiveLock,
                             HRESULT *phr) :

    CRenderedInputPin(NAME("CDumpInputPin"),
                  pFilter,                   // Filter
                  pLock,                     // Locking
                  phr,                       // Return code
                  L"Input"),                 // Pin name
    m_pReceiveLock(pReceiveLock),
    m_pDump(pDump),
    m_tLast(0)
{
}


//
// CheckMediaType
//
// Check if the pin can support this specific proposed type and format
//
HRESULT CDumpInputPin::CheckMediaType(const CMediaType *)
{
    return S_OK;
}


//
// BreakConnect
//
// Break a connection
//
HRESULT CDumpInputPin::BreakConnect()
{
    if (m_pDump->m_pPosition != NULL) {
        m_pDump->m_pPosition->ForceRefresh();
    }
    return CRenderedInputPin::BreakConnect();
}


//
// ReceiveCanBlock
//
// We don't hold up source threads on Receive
//
STDMETHODIMP CDumpInputPin::ReceiveCanBlock()
{
    return S_FALSE;
}


//
// Receive
//
// Do something with this media sample
//
STDMETHODIMP CDumpInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock lock(m_pReceiveLock);
    PBYTE pbData;

    // Has the filter been stopped yet
    if (m_pDump->m_hFile == INVALID_HANDLE_VALUE) {
        return NOERROR;
    }

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime(&tStart, &tStop);
    DbgLog((LOG_TRACE, 1, TEXT("tStart(%s), tStop(%s), Diff(%d ms), Bytes(%d)"),
           (LPCTSTR) CDisp(tStart),
           (LPCTSTR) CDisp(tStop),
           (LONG)((tStart - m_tLast) / 10000),
           pSample->GetActualDataLength()));

    m_tLast = tStart;

    // Copy the data to the file

    HRESULT hr = pSample->GetPointer(&pbData);
    if (FAILED(hr)) {
        return hr;
    }
    return m_pDump->Write(pbData,pSample->GetActualDataLength());
}


//
// DumpStringInfo
//
// Write to the file as text form
//
HRESULT CDumpInputPin::WriteStringInfo(IMediaSample *pSample)
{
    TCHAR TempString[256],FileString[256];
    PBYTE pbData;

    // Retrieve the time stamps from this sample

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime(&tStart, &tStop);
    m_tLast = tStart;

    // Write the sample time stamps out

    wsprintf(FileString,TEXT("\r\nRenderer received sample (%dms)"),timeGetTime());
    m_pDump->WriteString(FileString);
    wsprintf(FileString,TEXT("   Start time (%s)"),CDisp(tStart));
    m_pDump->WriteString(FileString);
    wsprintf(FileString,TEXT("   End time (%s)"),CDisp(tStop));
    m_pDump->WriteString(FileString);

    // Display the media times for this sample

    HRESULT hr = pSample->GetMediaTime(&tStart, &tStop);
    if (hr == NOERROR) {
        wsprintf(FileString,TEXT("   Start media time (%s)"),CDisp(tStart));
        m_pDump->WriteString(FileString);
        wsprintf(FileString,TEXT("   End media time (%s)"),CDisp(tStop));
        m_pDump->WriteString(FileString);
    }

    // Is this a sync point sample

    hr = pSample->IsSyncPoint();
    wsprintf(FileString,TEXT("   Sync point (%d)"),(hr == S_OK));
    m_pDump->WriteString(FileString);

    // Is this a preroll sample

    hr = pSample->IsPreroll();
    wsprintf(FileString,TEXT("   Preroll (%d)"),(hr == S_OK));
    m_pDump->WriteString(FileString);

    // Is this a discontinuity sample

    hr = pSample->IsDiscontinuity();
    wsprintf(FileString,TEXT("   Discontinuity (%d)"),(hr == S_OK));
    m_pDump->WriteString(FileString);

    // Write the actual data length

    LONG DataLength = pSample->GetActualDataLength();
    wsprintf(FileString,TEXT("   Actual data length (%d)"),DataLength);
    m_pDump->WriteString(FileString);

    // Does the sample have a type change aboard

    AM_MEDIA_TYPE *pMediaType;
    pSample->GetMediaType(&pMediaType);
    wsprintf(FileString,TEXT("   Type changed (%d)"),
        (pMediaType ? TRUE : FALSE));
    m_pDump->WriteString(FileString);
    DeleteMediaType(pMediaType);

    // Copy the data to the file

    hr = pSample->GetPointer(&pbData);
    if (FAILED(hr)) {
        return hr;
    }

    // Write each complete line out in BYTES_PER_LINES groups

    for (int Loop = 0;Loop < (DataLength / BYTES_PER_LINE);Loop++) {
        wsprintf(FileString,FIRST_HALF_LINE,pbData[0],pbData[1],pbData[2],
                 pbData[3],pbData[4],pbData[5],pbData[6],
                    pbData[7],pbData[8],pbData[9]);
        wsprintf(TempString,SECOND_HALF_LINE,pbData[10],pbData[11],pbData[12],
                 pbData[13],pbData[14],pbData[15],pbData[16],
                    pbData[17],pbData[18],pbData[19]);
        lstrcat(FileString,TempString);
        m_pDump->WriteString(FileString);
        pbData += BYTES_PER_LINE;
    }

    // Write the last few bytes out afterwards

    wsprintf(FileString,TEXT("   "));
    for (Loop = 0;Loop < (DataLength % BYTES_PER_LINE);Loop++) {
        wsprintf(TempString,TEXT("%x "),pbData[Loop]);
        lstrcat(FileString,TempString);
    }
    m_pDump->WriteString(FileString);
    return NOERROR;
}


//
// EndOfStream
//
STDMETHODIMP CDumpInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();

} // EndOfStream


//
// NewSegment
//
// Called when we are seeked
//
STDMETHODIMP CDumpInputPin::NewSegment(REFERENCE_TIME tStart,
                                       REFERENCE_TIME tStop,
                                       double dRate)
{
    m_tLast = 0;
    return S_OK;

} // NewSegment


//
//  CDump class
//
CDump::CDump(LPUNKNOWN pUnk, HRESULT *phr) :
    CUnknown(NAME("CDump"), pUnk),
    m_pFilter(NULL),
    m_pPin(NULL),
    m_pPosition(NULL),
    m_hFile(INVALID_HANDLE_VALUE),
    m_pFileName(0)
{
    m_pFilter = new CDumpFilter(this, GetOwner(), &m_Lock, phr);
    if (m_pFilter == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_pPin = new CDumpInputPin(this,GetOwner(),
                               m_pFilter,
                               &m_Lock,
                               &m_ReceiveLock,
                               phr);
    if (m_pPin == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }
}


//
// SetFileName
//
// Implemented for IFileSinkFilter support
//
STDMETHODIMP CDump::SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
    // Is this a valid filename supplied

    CheckPointer(pszFileName,E_POINTER);
    if(wcslen(pszFileName) > MAX_PATH)
        return ERROR_FILENAME_EXCED_RANGE;

    // Take a copy of the filename

    m_pFileName = new WCHAR[1+lstrlenW(pszFileName)];
    if (m_pFileName == 0)
        return E_OUTOFMEMORY;
    lstrcpyW(m_pFileName,pszFileName);

    // Create the file then close it

    HRESULT hr = OpenFile();
    CloseFile();
    return hr;

} // SetFileName


//
// GetCurFile
//
// Implemented for IFileSinkFilter support
//
STDMETHODIMP CDump::GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt)
{
    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName != NULL) {
        *ppszFileName = (LPOLESTR)
        QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(m_pFileName)));
        if (*ppszFileName != NULL) {
            lstrcpyW(*ppszFileName, m_pFileName);
        }
    }

    if(pmt) {
        ZeroMemory(pmt, sizeof(*pmt));
        pmt->majortype = MEDIATYPE_NULL;
        pmt->subtype = MEDIASUBTYPE_NULL;
    }
    return S_OK;

} // GetCurFile


// Destructor

CDump::~CDump()
{
    CloseFile();
    delete m_pPin;
    delete m_pFilter;
    delete m_pPosition;
    delete m_pFileName;
}


//
// CreateInstance
//
// Provide the way for COM to create a dump filter
//
CUnknown * WINAPI CDump::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CDump *pNewObject = new CDump(punk, phr);
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Override this to say what interfaces we support where
//
STDMETHODIMP CDump::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    CAutoLock lock(&m_Lock);

    // Do we have this interface

    if (riid == IID_IFileSinkFilter) {
        return GetInterface((IFileSinkFilter *) this, ppv);
    } 
    else if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) {
	    return m_pFilter->NonDelegatingQueryInterface(riid, ppv);
    } 
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        if (m_pPosition == NULL) {

            HRESULT hr = S_OK;
            m_pPosition = new CPosPassThru(NAME("Dump Pass Through"),
                                           (IUnknown *) GetOwner(),
                                           (HRESULT *) &hr, m_pPin);
            if (m_pPosition == NULL) {
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr)) {
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }

        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } 

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);

} // NonDelegatingQueryInterface


//
// OpenFile
//
// Opens the file ready for dumping
//
HRESULT CDump::OpenFile()
{
    TCHAR *pFileName = NULL;

    // Is the file already opened
    if (m_hFile != INVALID_HANDLE_VALUE) {
        return NOERROR;
    }

    // Has a filename been set yet
    if (m_pFileName == NULL) {
        return ERROR_INVALID_NAME;
    }

    // Convert the UNICODE filename if necessary

#if defined(WIN32) && !defined(UNICODE)
    char convert[MAX_PATH];
    if(!WideCharToMultiByte(CP_ACP,0,m_pFileName,-1,convert,MAX_PATH,0,0))
        return ERROR_INVALID_NAME;
    pFileName = convert;
#else
    pFileName = m_pFileName;
#endif

    // Try to open the file

    m_hFile = CreateFile((LPCTSTR) pFileName,   // The filename
                         GENERIC_WRITE,         // File access
                         (DWORD) 0,             // Share access
                         NULL,                  // Security
                         CREATE_ALWAYS,         // Open flags
                         (DWORD) 0,             // More flags
                         NULL);                 // Template

    if (m_hFile == INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();
        return HRESULT_FROM_WIN32(dwErr);
    }
    return S_OK;

} // Open


//
// CloseFile
//
// Closes any dump file we have opened
//
HRESULT CDump::CloseFile()
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return NOERROR;
    }

    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    return NOERROR;

} // Open


//
// Write
//
// Write stuff to the file
//
HRESULT CDump::Write(PBYTE pbData,LONG lData)
{
    DWORD dwWritten;

    if (!WriteFile(m_hFile,(PVOID)pbData,(DWORD)lData,&dwWritten,NULL)) {
        DWORD dwErr = GetLastError();
        return HRESULT_FROM_WIN32(dwErr);
    }
    return S_OK;
}


//
// WriteString
//
// Writes the given string into the file
//
void CDump::WriteString(TCHAR *pString)
{
    DWORD dwWritten = lstrlen(pString);
    const TCHAR *pEndOfLine = TEXT("\r\n");

    WriteFile((HANDLE) m_hFile,
              (PVOID) pString,
              (DWORD) dwWritten,
              &dwWritten, NULL);

    dwWritten = lstrlen(pEndOfLine);
    WriteFile((HANDLE) m_hFile,
              (PVOID) pEndOfLine,
              (DWORD) dwWritten,
              &dwWritten, NULL);

} // WriteString


//
// DllRegisterSever
//
// Handle the registration of this filter
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

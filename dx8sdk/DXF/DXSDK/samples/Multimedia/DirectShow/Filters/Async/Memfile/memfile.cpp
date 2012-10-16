//------------------------------------------------------------------------------
// File: MemFile.cpp
//
// Desc: DirectShow sample code - application using async filter.
//
// Copyright (c) 1996-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <stdio.h>
#include <asyncio.h>
#include <asyncrdr.h>
#include "memfile.h"

/* Fail gracefully if UNICODE build is enabled */
#ifdef UNICODE
#error This application does not build for UNICODE.
#endif

/*  Function prototypes */
HRESULT SelectAndRender(CMemReader *pReader, IFilterGraph **pFG);
HRESULT PlayFileWait(IFilterGraph *pFG);

/*  Read a file into memory, play it (or part of it), then exit */

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        printf("Usage : memfile foo.bar <Kbytes per sec>\n");
        return 0;
    }
    DWORD dwKBPerSec = argc == 2 ? INFINITE : atoi(argv[2]);
    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
    LPTSTR lpType;
    {
        /*  Find the extension */
        int len = lstrlen(argv[1]);
        if (len >= 4 && argv[1][len - 4] == TEXT('.')) {
            lpType = argv[1] + len - 3;
        } else {
            printf("Invalid file extension\n");
            return 1;
        }
    }
    if (lstrcmpi(lpType, TEXT("mpg")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1System;
    } else
    if (lstrcmpi(lpType, TEXT("mpa")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1Audio;
    } else
    if (lstrcmpi(lpType, TEXT("mpv")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1Video;
    } else
    if (lstrcmpi(lpType, TEXT("dat")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1VideoCD;
    } else
    if (lstrcmpi(lpType, TEXT("avi")) == 0) {
        mt.subtype = MEDIASUBTYPE_Avi;
    } else
    if (lstrcmpi(lpType, TEXT("mov")) == 0) {
        mt.subtype = MEDIASUBTYPE_QTMovie;
    } else
    if (lstrcmpi(lpType, TEXT("wav")) == 0) {
        mt.subtype = MEDIASUBTYPE_WAVE;
    } else
    {
        printf("Unknown file type %s\n", lpType);
        return 1;
    }

    /*  Read the file */
    HANDLE hFile = CreateFile(argv[1],
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Could not open %s\n", argv[1]);
        return 1;
    }

    ULARGE_INTEGER uliSize;
    uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

    PBYTE pbMem = new BYTE[uliSize.LowPart];
    if (pbMem == NULL) {
        printf("Could not allocate %d bytes\n", uliSize.LowPart);
        return 1;
    }

    DWORD dwBytesRead;
    if (!ReadFile(hFile,
                  (LPVOID)pbMem,
                  uliSize.LowPart,
                  &dwBytesRead,
                  NULL) ||
        dwBytesRead != uliSize.LowPart) {
        printf("Could not read file\n");
        CloseHandle(hFile);
        return 1;
    }
    CloseHandle(hFile);
    HRESULT hr = S_OK;

    CoInitialize(NULL);

    CMemStream Stream(pbMem, (LONGLONG)uliSize.QuadPart, dwKBPerSec);
    CMemReader *rdr = new CMemReader(&Stream, &mt, &hr);
    if (FAILED(hr) || rdr == NULL) {
        delete rdr;
        printf("Could not create filter HRESULT 0x%8.8X\n", hr);
        CoUninitialize();
        return 1;
    }

    //  Make sure we don't accidentally go away!
    rdr->AddRef();
    IFilterGraph *pFG = NULL;
    hr = SelectAndRender(rdr, &pFG);

    if (FAILED(hr)) {
        printf("Failed to create graph and render file HRESULT 0x%8.8X",
               hr);
    } else {
        //  Play the file
        HRESULT hr = PlayFileWait(pFG);
        if (FAILED(hr)) {
            printf("Failed to play graph HRESULT 0x%8.8X",
                   hr);
        }
    }
    rdr->Release();
    if (pFG) {
        ULONG ulRelease = pFG->Release();
        if (ulRelease != 0) {
            printf("Filter graph count not 0!  was %d", ulRelease);
        }
    }
    CoUninitialize();
    return 0;
}


//  Select a filter into a graph and render its output pin, returning
//  the graph

HRESULT SelectAndRender(CMemReader *pReader, IFilterGraph **ppFG)
{
    /*  Create filter graph */
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph,
                                  NULL,
                                  CLSCTX_INPROC,
                                  IID_IFilterGraph,
                                  (void**) ppFG);

    if (FAILED(hr)) {
        return hr;
    }

    /*  Get the various interfaces we need */

    /*  Add our filter */
    hr = (*ppFG)->AddFilter(pReader, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    /*  Render our output pin */
    IGraphBuilder *pBuilder;
    hr = (*ppFG)->QueryInterface(IID_IGraphBuilder, (void **)&pBuilder);
    if (FAILED(hr)) {
        return hr;
    }
    hr = pBuilder->Render(pReader->GetPin(0));
    pBuilder->Release();
    return hr;
}

HRESULT PlayFileWait(IFilterGraph *pFG)
{
    IMediaControl *pMC;
    IMediaEvent *pME;
    HRESULT hr = pFG->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pFG->QueryInterface(IID_IMediaEvent, (void **)&pME);
    if (FAILED(hr)) {
        pMC->Release();
        return hr;
    }

    OAEVENT oEvent;
    hr = pME->GetEventHandle(&oEvent);
    if (SUCCEEDED(hr)) {
        hr = pMC->Run();
    }
    if (SUCCEEDED(hr)) {
        LONG levCode;
        hr = pME->WaitForCompletion(INFINITE, &levCode);
    }
    pMC->Release();
    pME->Release();
    return hr;
}

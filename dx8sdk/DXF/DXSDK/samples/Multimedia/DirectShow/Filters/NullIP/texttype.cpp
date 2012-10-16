//------------------------------------------------------------------------------
// File: TextType.cpp
//
// Desc: DirectShow sample code - implementation of CTextMediaType class.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <windows.h>
#include <mmsystem.h>
#include <streams.h>

#include <string.h>
#include <tchar.h>

#include <stdio.h>
#include <wchar.h>

#include "resource.h"
#include "texttype.h"


//
// AsText
//
// Return the media type as a text string. Will place szAfterMajor after
// the text string for the major type and szAfterOthers after all other
// string apart from the last one.
//
void CTextMediaType::AsText(LPTSTR szType,
                            unsigned int iLen,
                            LPTSTR szAfterMajor,
                            LPTSTR szAfterOthers,
                            LPTSTR szAtEnd)
{
    ASSERT(szType);

    //
    // Convert Majortype to string
    //
    TCHAR szMajorType[100];
    UINT  iMajorType = 100;

    CLSID2String(szMajorType, iMajorType, &majortype);

    //
    // Convert Subtype to string
    //
    TCHAR szSubType[100];
    UINT  iSubType = 100;
    CLSID2String(szSubType, iSubType, &subtype);

    //
    // Convert Format to string
    //
    TCHAR szFormat[300];
    UINT  iFormat = 300;
    Format2String(szFormat, iFormat, FormatType(), Format(), FormatLength());

    //
    // Obtain the strings preceeding the Major Type, Sub Type and Format.
    //
    TCHAR szPreMajor[50];
    TCHAR szPreSub[50];
    TCHAR szPreFormat[50];

    LoadString(g_hInst, IDS_PREMAJOR, szPreMajor, 50);
    LoadString(g_hInst, IDS_PRESUB, szPreSub, 50);
    LoadString(g_hInst, IDS_PREFORMAT, szPreFormat, 50);

    _sntprintf(szType, iLen, TEXT("%s%s%s%s%s%s%s%s%s"),
                szPreMajor,  szMajorType, szAfterMajor,
                szPreSub,    szSubType, szAfterOthers,
                szPreFormat, szFormat, szAtEnd);
}


//
// CLSID2String
//
// Find a string description for a given GUID
//
void CTextMediaType::CLSID2String(LPTSTR szBuffer,
                                  UINT iLength,
                                  const GUID* pGuid)
{
#ifdef UNICODE
    TCHAR *pGuidName;
    StringFromCLSID(*pGuid, &pGuidName);
#else
    TCHAR *pGuidName = GuidNames[*pGuid];
#endif

    UINT strLength = lstrlen(pGuidName) + 1;
    lstrcpyn(szBuffer,pGuidName,min(strLength,iLength));
}


//
// Format2String
//
// Converts a format block to a string
//
void CTextMediaType::Format2String(LPTSTR szBuffer,
                                   UINT iLength,
                                   const GUID* pFormatType,
                                   BYTE* pFormat,
                                   ULONG lFormatLength)
{
    UNREFERENCED_PARAMETER(lFormatLength);

    //
    // Get the name of the format
    //
    TCHAR szName[50];
    UINT iName = 50;
    CLSID2String(szName, iName, pFormatType);

    //
    // Video Format
    //
    if (IsEqualGUID(*pFormatType, FORMAT_VideoInfo) ||
        IsEqualGUID(*pFormatType, FORMAT_MPEGVideo)) {

        VIDEOINFOHEADER * pVideoFormat = (VIDEOINFOHEADER *) pFormat;

        _sntprintf(szBuffer, iLength, TEXT("%4.4hs %dx%d, %d bits")
                   , (pVideoFormat->bmiHeader.biCompression == 0) ? TEXT("RGB") :
                         ((pVideoFormat->bmiHeader.biCompression == BI_BITFIELDS) ? TEXT("BITF") :
                         (LPTSTR) &pVideoFormat->bmiHeader.biCompression )
                   , pVideoFormat->bmiHeader.biWidth
                   , pVideoFormat->bmiHeader.biHeight
                   , pVideoFormat->bmiHeader.biBitCount);

         return;
    }

    //
    // Audio Format
    //
    if (IsEqualGUID(*pFormatType, FORMAT_WaveFormatEx)) {
        WAVEFORMATEX *pWaveFormat = (WAVEFORMATEX *) pFormat;

        // !!! use ACM to get format type name?
        _sntprintf(szBuffer, iLength, TEXT("%s: %.3f KHz %d bit %s ")
                   , szName
                   , (double) pWaveFormat->nSamplesPerSec / 1000.0
                   , pWaveFormat->wBitsPerSample
                   , pWaveFormat->nChannels == 1 ? TEXT("mono") : TEXT("stereo")
                  );

        return;
    }
    _sntprintf(szBuffer, iLength, TEXT("%s"), szName);
}

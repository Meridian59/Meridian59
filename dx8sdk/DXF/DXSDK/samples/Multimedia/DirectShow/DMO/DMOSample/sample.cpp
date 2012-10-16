//------------------------------------------------------------------------------
// File: Sample.cpp
//
// Desc: DirectShow sample code - implementation of CSample class.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"

#define FIX_LOCK_NAME
#include <dmo.h>

#include <limits.h>     //  _I64_MAX
#include <crtdbg.h>
#include <dmoimpl.h>
#include <uuids.h>      // DirectShow media type guids
#include <amvideo.h>    // VIDEOINFOHEADER definition
#include "resource.h"
#include "state.h"
#include "Sample.h"
#include "util.h"       // Helpers

#pragma warning(disable:4100)  // Disable C4100: unreferenced formal parameter


HRESULT CSample::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
    //  We can process data on any boundary
    *pdwFlags = 0;
    return S_OK;
}

HRESULT CSample::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
    //  We output single frames
    if (0 == dwOutputStreamIndex) {
        *pdwFlags = DMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                    DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                    DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE;
    } else {
        //  Stream 1
        //  Just text, no special buffering but 1 sample per sample
        *pdwFlags = DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                    DMO_OUTPUT_STREAMF_OPTIONAL;
    }
    return S_OK;
}

HRESULT CSample::InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
    //  Check if the type is already set and if so reject any type that's not identical
    if (InputTypeSet(dwInputStreamIndex)) {
        if (!TypesMatch(pmt, InputType(dwInputStreamIndex))) {
            return DMO_E_INVALIDTYPE;
        } else {
            return S_OK;
        }
    }
    
    //  We accept MEDIATYPE_Video, MEDIASUBTYPE_MPEG1Video
    //  Check the format is defined
    if (pmt->majortype == MEDIATYPE_Video &&
        pmt->subtype == MEDIASUBTYPE_MPEG1Payload &&
        pmt->formattype == FORMAT_MPEGVideo &&
        pmt->pbFormat != NULL) {
        return S_OK;
    } else {
        return DMO_E_INVALIDTYPE;
    }
}

HRESULT CSample::InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
    //  Check if the type is already set and if so reject any type that's not identical
    if (OutputTypeSet(dwOutputStreamIndex)) {
        if (!TypesMatch(pmt, OutputType(dwOutputStreamIndex))) {
            return DMO_E_INVALIDTYPE;
        } else {
            return S_OK;
        }
    }
    //  We output frames on stream 1
    if (dwOutputStreamIndex == 0) {
        if (!InputTypeSet(0)) {
            return DMO_E_INVALIDTYPE;
        }
        if (pmt->majortype == MEDIATYPE_Video &&
            pmt->subtype == MEDIASUBTYPE_RGB565 &&
            pmt->formattype == FORMAT_VideoInfo &&
            pmt->pbFormat != NULL) {
            const VIDEOINFOHEADER *pvihInput = (const VIDEOINFOHEADER *)InputType(0)->pbFormat;
            const VIDEOINFOHEADER *pvihOutput = (const VIDEOINFOHEADER *)pmt->pbFormat;

            LONG lWidth, lHeight;
            if (IsRectEmpty(&pvihOutput->rcTarget)) {
                lWidth = pvihOutput->bmiHeader.biWidth;
                lHeight = pvihOutput->bmiHeader.biHeight;
            } else {
                lWidth = pvihOutput->rcTarget.right - pvihOutput->rcTarget.left;
                lHeight = pvihOutput->rcTarget.bottom - pvihOutput->rcTarget.top;
            }
            if (pvihInput->bmiHeader.biWidth == lWidth &&
                pvihInput->bmiHeader.biHeight == lHeight) {
                return S_OK;
            }
        }
        return DMO_E_INVALIDTYPE;
    } else {
        //  Stream 1
        if (pmt->majortype == MEDIATYPE_Text && pmt->subtype == GUID_NULL) {
            return S_OK;
        } else {
            return DMO_E_INVALIDTYPE;
        }
    }
}


HRESULT CSample::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex,
                             DMO_MEDIA_TYPE *pmt)
{
    //  No types to all indices for dwTypeIndex are out of range.
    return DMO_E_NO_MORE_ITEMS;
}

HRESULT CSample::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex,
                             DMO_MEDIA_TYPE *pmt)
{
    if (!InputTypeSet(0)) {
        return DMO_E_TYPE_NOT_SET;
    }

    if (dwTypeIndex != 0) {
        return DMO_E_NO_MORE_ITEMS;
    }

    // If GetOutputType()'s pmt parameter is NULL, return S_OK if the type exists.
    // Return DMO_E_NO_MORE_ITEMS if the type does not exists.  See the 
    // documentation for IMediaObject::GetOutputType() for more information.
    if (NULL != pmt) {
        if (dwOutputStreamIndex == 0) {

            //  Create our media type
            HRESULT hr = MoInitMediaType(pmt, FIELD_OFFSET(VIDEOINFO, dwBitMasks[3]));
            if (FAILED(hr)) {
                return hr;
            }

            const VIDEOINFOHEADER *pvihInput = (const VIDEOINFOHEADER *)InputType(0)->pbFormat;
            LONG lWidth  = pvihInput->bmiHeader.biWidth;
            LONG lHeight = pvihInput->bmiHeader.biHeight;

            //  Initialize the media type structure (MoInitMediaType initalized cbFormat
            //  and pbFormat)
            pmt->majortype            = MEDIATYPE_Video;
            pmt->subtype              = MEDIASUBTYPE_RGB565;
            pmt->bFixedSizeSamples    = TRUE;
            pmt->bTemporalCompression = FALSE;
            pmt->lSampleSize          = lWidth * lHeight * 2;
            pmt->formattype           = FORMAT_VideoInfo;
            pmt->pUnk                 = NULL;
            
            //  Initialize the format
            VIDEOINFO *pviOutput = (VIDEOINFO *)pmt->pbFormat;
            ZeroMemory(pviOutput, FIELD_OFFSET(VIDEOINFO, dwBitMasks[3]));
            pviOutput->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            pviOutput->bmiHeader.biCompression = BI_BITFIELDS;
            pviOutput->bmiHeader.biBitCount = 16;
            pviOutput->bmiHeader.biPlanes = 1;
            pviOutput->bmiHeader.biWidth = lWidth;
            pviOutput->bmiHeader.biHeight = lHeight;
            pviOutput->bmiHeader.biSizeImage = pmt->lSampleSize;
            pviOutput->TrueColorInfo.dwBitMasks[0] = 0xF800;
            pviOutput->TrueColorInfo.dwBitMasks[1] = 0x07E0;
            pviOutput->TrueColorInfo.dwBitMasks[2] = 0x001F;
            pviOutput->AvgTimePerFrame = pvihInput->AvgTimePerFrame;
        } else {
            ZeroMemory(pmt, sizeof(*pmt));
            pmt->majortype            = MEDIATYPE_Text;
        }
    }

    return S_OK;
}
HRESULT CSample::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize,
                             DWORD *pcbMaxLookahead, DWORD *pcbAlignment)
{
    *pcbSize = 1;
    *pcbMaxLookahead = 0;
    *pcbAlignment = 1;
    return S_OK;
}
HRESULT CSample::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize,
                              DWORD *pcbAlignment)
{
    *pcbAlignment = 1;
    if (dwOutputStreamIndex == 0) {
        *pcbSize = OutputType(0)->lSampleSize;
        return S_OK;
    } else {
        *pcbSize = sizeof(L"hh:mm:ss:ff"); //  hh:mm:ss:ff
        return S_OK;
    }
}
HRESULT CSample::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
    return E_NOTIMPL;
}
HRESULT CSample::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
    return E_NOTIMPL;
}
HRESULT CSample::InternalFlush()
{
    InternalDiscontinuity(0);

    //  Release buffer
    m_pBuffer = NULL;

    return S_OK;
}
HRESULT CSample::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
    //  Zero our timestamp
    m_rtFrame = 0;

    //  No pictures yet
   m_bPicture   = false;

   //  Reset state machine
   m_StreamState.Reset();
    return S_OK;
}
HRESULT CSample::InternalAllocateStreamingResources()
{
    //  Reinitialize variables
    InternalDiscontinuity(0);

    //  Allocate our bitmap
    return S_OK;
}
HRESULT CSample::InternalFreeStreamingResources()
{
    return S_OK;
}
HRESULT CSample::InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,
                                DWORD dwFlags, REFERENCE_TIME rtTimestamp,
                                REFERENCE_TIME rtTimelength)
{
    //  Check parameters
    _ASSERTE(m_pBuffer == NULL);
    HRESULT hr = pBuffer->GetBufferAndLength(&m_pbData, &m_cbData);
    if (FAILED(hr)) {
        return hr;
    }
    m_pBuffer        = pBuffer;

    if (0 == (dwFlags & DMO_INPUT_DATA_BUFFERF_TIME)) {
        rtTimestamp = INVALID_TIME;
    }

    m_StreamState.TimeStamp(rtTimestamp);

    //  Process() returns S_FALSE if there is no output, S_OK otherwise
    hr = Process();
    return hr;
}

HRESULT CSample::InternalProcessOutput(DWORD dwFlags,
                                       DWORD cOutputBufferCount,
                                       DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                                       DWORD *pdwStatus)
{
    //  Check buffer
    PBYTE pbData;
    DWORD cbData;
    DWORD cbCurrent;

    //  Do we have any output?
    if (!m_bPicture) {
        return S_FALSE;
    }
    HRESULT hr = pOutputBuffers[0].pBuffer->GetBufferAndLength(
                       &pbData, &cbCurrent);
    if (FAILED(hr)) {
        return hr;
    }
    hr = pOutputBuffers[0].pBuffer->GetMaxLength(&cbData);
    if (FAILED(hr)) {
        return hr;
    }

    if (cbData < cbCurrent + (DWORD)OutputType(0)->lSampleSize) {
        return E_INVALIDARG;
    }

    //  Say we've filled the buffer
    hr = pOutputBuffers[0].pBuffer->SetLength(cbCurrent + (DWORD)OutputType(0)->lSampleSize);

    cbData -= cbCurrent;
    pbData += cbCurrent;

    //  Generate our data
    DWORD dwTimeCode;
    REFERENCE_TIME rt = m_StreamState.PictureTime(&dwTimeCode);
    TCHAR szBuffer[20];
    wsprintf(szBuffer, TEXT("%2.2d:%2.2d:%2.2d:%2.2d"),
             (dwTimeCode >> 19) & 0x1F,
             (dwTimeCode >> 13) & 0x3F,
             (dwTimeCode >> 6) & 0x3F,
             dwTimeCode & 0x3F);

    //  Update our bitmap with turquoise
    OurFillRect((const VIDEOINFOHEADER *)OutputType(0)->pbFormat, pbData, 0x03EF);

    //  Draw our text
    DrawOurText((const VIDEOINFOHEADER *)OutputType(0)->pbFormat, pbData, szBuffer);

    pOutputBuffers[0].dwStatus = DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT;

    //  Set the timestamp
    if (rt != INVALID_TIME) {
        pOutputBuffers[0].rtTimestamp = rt;

    } else {
        pOutputBuffers[0].rtTimestamp = m_rtFrame;
    }
    REFERENCE_TIME rtLength = ((const VIDEOINFOHEADER *)OutputType(0)->pbFormat)->AvgTimePerFrame;
    pOutputBuffers[0].rtTimelength = rtLength;
    m_rtFrame = pOutputBuffers[0].rtTimestamp + rtLength;

    //  Uncompressed video must always have a timestamp
    pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIME | DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH;

    //  Update our state
    m_bPicture = false;

    //  Is there any more data to output at this point?
    if (S_OK == Process()) {
        pOutputBuffers[0].dwStatus = DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
    }

    //  Is there an output buffer for stream 1?
    if (pOutputBuffers[1].pBuffer) {
        PBYTE pbData;
        DWORD cbLength;
        DWORD cbMax;            
        DWORD dwLen = lstrlen(szBuffer) * sizeof(WCHAR);
        if (S_OK == pOutputBuffers[1].pBuffer->GetBufferAndLength(&pbData, &cbLength) &&
            S_OK == pOutputBuffers[1].pBuffer->GetMaxLength(&cbMax) &&
            cbLength + dwLen <= cbMax) {

            //  Convert to UNICODE!
            USES_CONVERSION;
            LPWSTR lpsz = T2W(szBuffer);

            CopyMemory(pbData + cbLength, lpsz, dwLen);
            pOutputBuffers[1].pBuffer->SetLength(cbLength + dwLen);
            pOutputBuffers[1].dwStatus = pOutputBuffers[0].dwStatus;
            pOutputBuffers[1].rtTimestamp = pOutputBuffers[0].rtTimestamp;
            pOutputBuffers[1].rtTimelength = rtLength;
        }
    }
    return S_OK;
}
HRESULT CSample::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
    return m_pBuffer == NULL ? S_OK : S_FALSE;
}

//  Scan input data until either we're exhausted or we find
//  a picture start code
//  Note GOP time codes as we encounter them so we can
//  output time codes

HRESULT CSample::Process()
{
    //  Process bytes and update our state machine
    while (m_cbData && !m_bPicture) {
        m_bPicture = m_StreamState.NextByte(*m_pbData);
        m_cbData--;
        m_pbData++;
    }

    //  Release buffer if we're done with it
    if (m_cbData == 0) {
        m_pBuffer = NULL;
    }

    //  assert that if have no picture to output then we ate all the data
    _ASSERTE(m_bPicture || m_cbData == 0);
    return m_bPicture ? S_OK : S_FALSE;
}

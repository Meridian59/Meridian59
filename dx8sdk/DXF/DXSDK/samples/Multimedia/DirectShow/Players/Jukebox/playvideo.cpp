//------------------------------------------------------------------------------
// File: PlayVideo.cpp
//
// Desc: DirectShow sample code - media control functions.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <dshow.h>

#include "playvideo.h"


HRESULT RunMedia()
{
    HRESULT hr=S_OK;

    if (!pMC)
        return S_OK;

    // Start playback
    hr = pMC->Run();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Run()!\r\n"), hr);
        return hr;
    }

    // Remember play state
    g_psCurrent = State_Running;
    return hr;
}


HRESULT StopMedia()
{
    HRESULT hr=S_OK;

    if (!pMC)
        return S_OK;

    // If we're already stopped, don't check again
    if (g_psCurrent == State_Stopped)
        return hr;

    // Stop playback
    hr = pMC->Stop();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Stop()!\r\n"), hr);
        return hr;
    }

    // Remember play state
    g_psCurrent = State_Stopped;
    return hr;
}


HRESULT PauseMedia(void)
{
    HRESULT hr=S_OK;

    if (!pMC)
        return S_OK;

    // Play/pause
    if(g_psCurrent != State_Running)
        return S_OK;

    hr = pMC->Pause();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Pause()!\r\n"), hr);
        return hr;
    }
//  else
//      RetailOutput(TEXT("*** Media is PAUSED.\r\n"));

    // Remember play state
    g_psCurrent = State_Paused;
    return hr;
}


HRESULT MuteAudio(void)
{
    HRESULT hr=S_OK;
    IBasicAudio *pBA=NULL;
    long lVolume;

    if (!pGB)
        return S_OK;

    hr =  pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA);
    if (FAILED(hr))
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        pBA->Release();
        return hr;
    }
    else if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in pBA->get_Volume!  hr=0x%x\r\n"), hr);
        pBA->Release();
        return hr;
    }

    lVolume = VOLUME_SILENCE;
//  RetailOutput(TEXT("*** Media is MUTING.\r\n"));

    // Set new volume
    hr = pBA->put_Volume(lVolume);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in pBA->put_Volume!  hr=0x%x\r\n"), hr);
    }

    pBA->Release();
    return hr;
}


HRESULT ResumeAudio(void)
{
    HRESULT hr=S_OK;
    IBasicAudio *pBA=NULL;
    long lVolume;

    if (!pGB)
        return S_OK;

    hr =  pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA);
    if (FAILED(hr))
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        pBA->Release();
        return hr;
    }
    else if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in pBA->get_Volume!  hr=0x%x\r\n"), hr);
        pBA->Release();
        return hr;
    }

    lVolume = VOLUME_FULL;
//  RetailOutput(TEXT("*** Media is Resuming normal audio\r\n"));

    // Set new volume
    hr = pBA->put_Volume(lVolume);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in pBA->put_Volume!  hr=0x%x\r\n"), hr);
    }

    pBA->Release();
    return hr;
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



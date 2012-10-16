//-----------------------------------------------------------------------------
// File: EchoTool.cpp
//
// Desc: Implements an object based on IDirectMusicTool
//       that provides echoing effects.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <dmusici.h>
#include "EchoTool.h"




//-----------------------------------------------------------------------------
// Name: CEchoTool::CEchoTool()
// Desc: 
//-----------------------------------------------------------------------------
CEchoTool::CEchoTool()
{
    m_cRef = 1;                 // Set to 1 so one call to Release() will free this
    m_dwEchoNum = 3;            // Default to 3 echoes per note
    m_mtDelay = DMUS_PPQ / 2;   // Default to 8th note echoes
    InitializeCriticalSection(&m_CrSec);
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::~CEchoTool()
// Desc: 
//-----------------------------------------------------------------------------
CEchoTool::~CEchoTool()
{
    DeleteCriticalSection(&m_CrSec);
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::QueryInterface()
// Desc: 
//-----------------------------------------------------------------------------
STDMETHODIMP CEchoTool::QueryInterface(const IID &iid, void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicTool)
    {
        *ppv = static_cast<IDirectMusicTool*>(this);
    } 
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::AddRef()
// Desc: 
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CEchoTool::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::Release()
// Desc: 
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CEchoTool::Release()
{
    if( 0 == InterlockedDecrement(&m_cRef) )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::Init()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::Init( IDirectMusicGraph* pGraph )
{
    // This tool has no need to do any type of initialization.
    return E_NOTIMPL;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::GetMsgDeliveryType()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::GetMsgDeliveryType( DWORD* pdwDeliveryType )
{
    // This tool wants messages immediately.
    // This is the default, so returning E_NOTIMPL
    // would work. The other method is to specifically
    // set *pdwDeliveryType to the delivery type, DMUS_PMSGF_TOOL_IMMEDIATE,
    // DMUS_PMSGF_TOOL_QUEUE, or DMUS_PMSGF_TOOL_ATTIME.
    
    *pdwDeliveryType = DMUS_PMSGF_TOOL_IMMEDIATE;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::GetMediaTypeArraySize()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::GetMediaTypeArraySize( DWORD* pdwNumElements )
{
    // This tool only wants note messages, patch messages, sysex, and MIDI messages, so set
    // *pdwNumElements to 4.
    
    *pdwNumElements = 4;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::GetMediaTypes()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::GetMediaTypes( DWORD** padwMediaTypes, 
                                                    DWORD dwNumElements )
{
    // Fill in the array padwMediaTypes with the type of
    // messages this tool wants to process. In this case,
    // dwNumElements will be 3, since that is what this
    // tool returns from GetMediaTypeArraySize().
    
    if( dwNumElements == 4 )
    {
        // Set the elements in the array to DMUS_PMSGT_NOTE,
        // DMUS_PMSGT_MIDI, and DMUS_PMSGT_PATCH
        (*padwMediaTypes)[0] = DMUS_PMSGT_NOTE;
        (*padwMediaTypes)[1] = DMUS_PMSGT_MIDI;
        (*padwMediaTypes)[2] = DMUS_PMSGT_PATCH;
        (*padwMediaTypes)[3] = DMUS_PMSGT_SYSEX;
        return S_OK;
    }
    else
    {
        // This should never happen
        return E_FAIL;
    }
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::ProcessPMsg()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::ProcessPMsg( IDirectMusicPerformance* pPerf, 
                                                  DMUS_PMSG* pPMsg )
{
    DWORD dwCount;
    DWORD dwEchoNum;
    MUSIC_TIME mtDelay;
    
    // SetEchoNum() and SetDelay() use these member variables,
    // so use a critical section to make them thread-safe.
    EnterCriticalSection(&m_CrSec);
    dwEchoNum = m_dwEchoNum;
    mtDelay = m_mtDelay;
    LeaveCriticalSection(&m_CrSec);
    
    // Returning S_FREE frees the message. If StampPMsg()
    // fails, there is no destination for this message so
    // free it.
    if(( NULL == pPMsg->pGraph ) ||
        FAILED(pPMsg->pGraph->StampPMsg(pPMsg)))
    {
        return DMUS_S_FREE;
    }

  
    // The Tool is set up to only receive messages of types
    // DMUS_PMSGT_NOTE, DMUS_PMSGT_MIDI, DMUS_PMSGT_SYSEX, or DMUS_PMSGT_PATCH
    // We use the DX8 ClonePMsg method to make a copy of the pmsg and
    // send it to a pchannel in the next pchannel group. 
    // If it's a note, we also doctor the velocity.
    IDirectMusicPerformance8 *pPerf8;
    if (SUCCEEDED(pPerf->QueryInterface(IID_IDirectMusicPerformance8,(void **)&pPerf8)))
    {
        for( dwCount = 1; dwCount <= dwEchoNum; dwCount++ )
        {
            DMUS_PMSG *pClone;
            if( SUCCEEDED( pPerf8->ClonePMsg( pPMsg,&pClone)))
            {
                // Add to the time of the echoed note
                pClone->mtTime += (dwCount * mtDelay);
                if (pPMsg->dwType == DMUS_PMSGT_NOTE )
                {
                    DMUS_NOTE_PMSG *pNote = (DMUS_NOTE_PMSG*)pPMsg;
                    DMUS_NOTE_PMSG *pCloneNote = (DMUS_NOTE_PMSG*)pClone;
                    // Reduce the volume of the echoed note
                    // percentage of reduction in velocity increases with each echo
                    pCloneNote->bVelocity = (BYTE) (pNote->bVelocity - 
                        ((pNote->bVelocity * (dwCount * 15))/100));
                }
                // Set the note so only MUSIC_TIME is valid.
                // REFERENCE_TIME will be recomputed inside
                // SendPMsg()
                pClone->dwFlags = DMUS_PMSGF_MUSICTIME;
                pClone->dwPChannel = pPMsg->dwPChannel + 
                    (16*dwCount);
                // Queue the echoed PMsg
                pPerf->SendPMsg(pClone );
            }
        }
        pPerf8->Release();
    }


    // Return DMUS_S_REQUEUE so the original message is requeued
    return DMUS_S_REQUEUE;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::Flush()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CEchoTool::Flush( IDirectMusicPerformance* pPerf, 
                                            DMUS_PMSG* pDMUS_PMSG,
                                            REFERENCE_TIME rt)
{
    // This tool does not need to flush.
    return E_NOTIMPL;
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::SetEchoNum()
// Desc: 
//-----------------------------------------------------------------------------
void CEchoTool::SetEchoNum( DWORD dwEchoNum )
{
    // ProcessPMsg() uses m_dwEchoNum, so use a critical
    // section to make it thread-safe.
    if( dwEchoNum <= MAX_ECHOES )
    {
        EnterCriticalSection(&m_CrSec);
        m_dwEchoNum = dwEchoNum;
        LeaveCriticalSection(&m_CrSec);
    }
}




//-----------------------------------------------------------------------------
// Name: CEchoTool::SetDelay()
// Desc: 
//-----------------------------------------------------------------------------
void CEchoTool::SetDelay( MUSIC_TIME mtDelay )
{
    // ProcessPMsg() uses m_mtDelay, so use a critical
    // section to make it thread-safe.
    EnterCriticalSection(&m_CrSec);
    m_mtDelay = mtDelay;
    LeaveCriticalSection(&m_CrSec);
}

//----------------------------------------------------------------------------
// File: server.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _DPLAY8_SERVER_H
#define _DPLAY8_SERVER_H



#define MIN_SP_THREADS		1
#define MAX_SP_THREADS		128
#define MAX_SP_BUFFER       1024000

#define MAX_PACK_SIZE		2048
#define MAX_THREAD_WAIT     60000

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include "NetAbstract.h"

interface IDirectPlay8Server;




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CDPlay8Server : public IOutboundServer
{
public:
    CDPlay8Server();

    HRESULT Start(DWORD dwNumSPThreads);
    void    Shutdown();
    void    SetServer( INetServer* pServer ) { m_pServer = pServer; };

    DWORD   GetNumSPThreads();
    void    SetNumSPThreads(DWORD dwNumSPThreads);

    DWORD   GetSPBuffer();
    void    SetSPBuffer(DWORD dwSPBufferSize);

    // From IOutboundServer
    virtual HRESULT SendPacket( DWORD dwTo, void* pData, DWORD dwSize, BOOL bGuaranteed, DWORD dwTimeout );
    virtual HRESULT GetConnectionInfo( DWORD dwID, TCHAR* strConnectionInfo );
    virtual HRESULT RejectClient( DWORD dwID, HRESULT hrReason );


protected:
    IDirectPlay8Server*     m_pDPlay;
    INetServer*             m_pServer;

    static HRESULT WINAPI StaticReceiveHandler( void *pvContext, DWORD dwMessageType, void *pvMessage );
    HRESULT WINAPI ReceiveHandler( void *pvContext, DWORD dwMessageType, void *pvMessage );
};




#endif

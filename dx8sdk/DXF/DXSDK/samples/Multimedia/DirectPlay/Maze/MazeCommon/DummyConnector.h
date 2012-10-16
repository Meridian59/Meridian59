//----------------------------------------------------------------------------
// File: dummyconnector.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _DUMMY_CONNECTOR_H
#define _DUMMY_CONNECTOR_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include "NetAbstract.h"
#include <tchar.h>




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CDummyConnectorServer : public IOutboundServer
{
public:
    CDummyConnectorServer() : m_pTarget(NULL),m_dwID(0) {};

    void    SetTarget( INetClient* ptarget )
    {
        m_pTarget = ptarget;
    };

    // From IOutboundServer
    virtual HRESULT SendPacket( DWORD /*to*/ , void* data , DWORD size , BOOL /*guaranteed*/, DWORD /*dwTimeout*/ )
    {
        if ( m_pTarget )
            return m_pTarget->OnPacket( m_dwID , data , size );
        return S_OK;
    };

    virtual HRESULT GetConnectionInfo( DWORD dwID, TCHAR* strConnectionInfo )
    {
        _tcscpy( strConnectionInfo, TEXT("") );
        return S_OK;
    }

    virtual HRESULT RejectClient( DWORD dwID, HRESULT hrReason )
    {
        return S_OK;
    }

private:
    INetClient* m_pTarget;
    DWORD       m_dwID;
};








//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CDummyConnectorClient : public IOutboundClient
{
public:
    CDummyConnectorClient() : m_pTarget(NULL),m_dwID(0) {};

    void    SetTarget( INetServer* ptarget )
    {
        m_pTarget = ptarget;
    };

    // Connect (sends an "add connection" message to target)
    void    Connect( DWORD id )
    {
        m_dwID = id;
        if ( m_pTarget )
            m_pTarget->OnAddConnection( id );
    };

    // Disconnect (sends a "remove connection" message to target
    void    Disconnect( DWORD id )
    {
        if ( m_pTarget )
            m_pTarget->OnRemoveConnection( m_dwID );
    }

    // From IOutboundClient
    virtual HRESULT SendPacket( void* data , DWORD size , BOOL /*guaranteed*/, DWORD /*dwTimeout*/ )
    {
        if ( m_pTarget )
            return m_pTarget->OnPacket( m_dwID , data , size );
        return S_OK;
    };

    virtual DWORD GetThroughputBPS()
    {
        return 0;
    }

    virtual DWORD GetRoundTripLatencyMS()
    {
        return 0;
    }

    virtual BOOL    IsSessionLost() { return FALSE; };
    virtual DWORD   GetSessionLostReason() { return 0; };

    virtual HRESULT GetConnectionInfo( TCHAR* strConnectionInfo )
    {
        _tcscpy( strConnectionInfo, TEXT("") );
        return S_OK;
    }

private:
    INetServer* m_pTarget;
    DWORD       m_dwID;
};




#endif

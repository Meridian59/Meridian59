//----------------------------------------------------------------------------
// File: dplay8client.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _DPLAY8_CLIENT_H
#define _DPLAY8_CLIENT_H


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include "NetAbstract.h"

struct MazeConfig;

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class   CDPlay8Client : public IOutboundClient
{
public:
    CDPlay8Client();
    ~CDPlay8Client();

    HRESULT     Init(MazeConfig* pMazeConfig);
    void        Shutdown();
    HRESULT     StartSessionEnum( const TCHAR* ipaddress );
    HRESULT     StopSessionEnum();
    DWORD       GetNumSessions() const { return m_dwNumSessions; };
    const TCHAR* GetSessionName( DWORD num ) const { return m_szSessionNames[num]; };
    DWORD       GetSessionMaxPlayers( DWORD num ) const { return m_Sessions[num].dwMaxPlayers; };
    DWORD       GetSessionCurrentPlayers( DWORD num ) const { return m_Sessions[num].dwCurrentPlayers; };
    GUID        GetSessionGUID( DWORD num ) const { return m_Sessions[num].guidInstance; };
    HRESULT     JoinSession( DWORD num );
    void        SetClient( INetClient* pclient ) { m_pClient = pclient; };
    IOutboundClient* GetOutboundClient() const { return ((IOutboundClient*)this); };
    
    DWORD       GetNumSPThreads();
    void        SetNumSPThreads(DWORD dwNumSPThreads);

    DWORD       GetSPBuffer();
    void        SetSPBuffer(DWORD dwSPBufferSize);

    // IOutboundClient
    virtual HRESULT     SendPacket( void* pData, DWORD dwSize, BOOL bGuaranteed, DWORD dwTimeout );
    virtual DWORD       GetThroughputBPS();
    virtual DWORD       GetRoundTripLatencyMS();
    virtual BOOL        IsSessionLost() { return m_bSessionLost; };
    virtual DWORD       GetSessionLostReason() { return m_dwSessionLostReason; };
    virtual HRESULT     GetConnectionInfo( TCHAR* strConnectionInfo );

protected:
    IDirectPlay8Client* m_pDPlay;
    BOOL                m_bConnected;
    INetClient*         m_pClient;
    BOOL                m_bSessionLost;
    DWORD               m_dwSessionLostReason;
    FLOAT               m_fThroughputBPS;
    DWORD               m_dwThroughputBytes;
    
    CRITICAL_SECTION    m_csThreadCountLock;
    WORD                m_wActiveThreadCount;
    WORD                m_wMaxThreadCount;
    FLOAT               m_fAvgThreadCount;
    FLOAT               m_fAvgThreadTime;
    FLOAT               m_fMaxThreadTime;

    // Configuration info.
    MazeConfig*         m_MazeConfig;

    // Connection info 
    FLOAT               m_fLastUpdateConnectInfoTime;
    DPN_CONNECTION_INFO m_dpnConnectionInfo;
    DWORD               m_dwHighPriMessages, m_dwHighPriBytes;
    DWORD               m_dwNormalPriMessages, m_dwNormalPriBytes;
    DWORD               m_dwLowPriMessages, m_dwLowPriBytes;
    HRESULT             UpdateConnectionInfo();

    BOOL EnumSessionCallback( const DPN_APPLICATION_DESC *pdesc, IDirectPlay8Address* pDP8AddressHost, IDirectPlay8Address* pDP8AddressDevice );
    static HRESULT WINAPI StaticReceiveHandler( void *pvContext, DWORD dwMessageType, void *pvMessage );
    HRESULT WINAPI ReceiveHandler( void *pvContext, DWORD dwMessageType, void *pvMessage );

    enum                    {MAX_SESSIONS=256};
    enum                    {MAX_SESSION_NAME=64};
    DWORD                   m_dwNumSessions;
    DPN_APPLICATION_DESC    m_Sessions[MAX_SESSIONS];
    IDirectPlay8Address*    m_pHostAddresses[MAX_SESSIONS];
    IDirectPlay8Address*    m_pDeviceAddresses[MAX_SESSIONS];
    TCHAR                   m_szSessionNames[MAX_SESSIONS][MAX_SESSION_NAME];
    FLOAT                   m_fSessionLastSeenTime[MAX_SESSIONS];
    CRITICAL_SECTION        m_csLock;
    DPNHANDLE               m_dpnhEnum;
};



#endif

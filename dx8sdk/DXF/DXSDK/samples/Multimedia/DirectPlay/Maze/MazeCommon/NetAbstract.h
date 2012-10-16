//----------------------------------------------------------------------------
// File: netabstract.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _NETABSTRACT_H
#define _NETABSTRACT_H


#define SERVER_ID   1
enum { DISCONNNECT_REASON_UNKNOWN = 0, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE };



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
interface   INetServer
{
public:
    virtual ~INetServer() {} ;

    virtual HRESULT OnPacket( DWORD dwFrom, void* dwData, DWORD dwSize ) = 0;
    virtual void    OnAddConnection( DWORD dwID ) = 0;
    virtual void    OnRemoveConnection( DWORD dwID ) = 0;
    virtual void    OnSessionLost( DWORD dwReason ) = 0;
};





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
interface   INetClient
{
public:
    virtual ~INetClient() {} ;

    virtual HRESULT OnPacket( DWORD dwFrom, void* dwData, DWORD dwSize ) = 0;
    virtual void    OnSessionLost( DWORD dwReason ) = 0;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
interface   IOutboundServer
{
public:
    virtual HRESULT SendPacket( DWORD dwTo, void* dwData, DWORD dwSize, 
                                BOOL dwGuaranteed, DWORD dwTimeout ) = 0;
    virtual HRESULT GetConnectionInfo( DWORD dwID, TCHAR* strConnectionInfo ) = 0;
    virtual HRESULT RejectClient( DWORD dwID, HRESULT hrReason ) = 0;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
interface   IOutboundClient
{
public:
    virtual HRESULT SendPacket( void* dwData, DWORD dwSize, 
                                BOOL dwGuaranteed, DWORD dwTimeout ) = 0;
    virtual DWORD GetThroughputBPS() = 0;
    virtual DWORD GetRoundTripLatencyMS() = 0;
    virtual BOOL  IsSessionLost() = 0;
    virtual DWORD GetSessionLostReason() = 0;
    virtual HRESULT GetConnectionInfo( TCHAR* strConnectionInfo ) = 0;
};



#endif

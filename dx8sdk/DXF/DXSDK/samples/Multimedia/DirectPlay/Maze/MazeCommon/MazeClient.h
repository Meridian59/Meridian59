//----------------------------------------------------------------------------
// File: mazeclient.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MAZE_CLIENT_H
#define _MAZE_CLIENT_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include "DXUtil.h"
#include "Maze.h"
#include "NetAbstract.h"
#include "Packets.h"
#include "Trig.h"
#include "SimpleStack.h"
#include "Packets.h"
#include "MazeServer.h"
#include "IMazeGraphics.h"

class CMazeApp;

// The MAZE_CLIENT_VERSION should be rev'ed whenever the client exposes 
// new functionality that the server expects.  This number is sent to
// the server so the server can accept or reject the client based on its version.
#define MAZE_CLIENT_VERSION         107



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct PlayerObject
{
    DWORD           dwID;
    D3DXVECTOR3     vPos;
    ANGLE           aCameraYaw;
    WORD            wCellX;
    WORD            wCellY;
    FLOAT           fLastValidTime;
    PlayerObject*  pNext;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#define MAZE_WIDTH  128
#define MAZE_HEIGHT 128
#define MAZE_SIZE   (MAZE_WIDTH*MAZE_HEIGHT)




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CMazeClient : public INetClient
{
public:
    CMazeClient();
    ~CMazeClient();

    void SetApp( CMazeApp* pMazeApp ) { m_pMazeApp = pMazeApp; };

    // INetClient
    virtual HRESULT OnPacket( DWORD from , void* data , DWORD size );
    virtual void    OnSessionLost( DWORD dwReason );

    // Connect an outbound network provider
    void            SetOutboundClient( IOutboundClient* poutnet ) { m_pNet = poutnet; };

    HRESULT         Init( CMazeApp* pMazeApp, IMazeGraphics* pMazeGraphics );
    HRESULT         Reset();
    void            Shutdown();
    void            Update( FLOAT elapsed );

    // Lock and unlock the world database
    void            LockWorld() { m_WorldLock.Enter(); };
    void            UnlockWorld() { m_WorldLock.Leave(); };

    // Lock and unlock the world database
    void            SetMazeReady( BOOL bReady ) { if( bReady ) SetEvent( m_hReady ); else ResetEvent( m_hReady ); };
    BOOL            IsMazeReady() { if( WaitForSingleObject( m_hReady, 0 ) == WAIT_OBJECT_0 ) return TRUE; else return FALSE; };

    // Check to see if we have received first Connect Config Packet.
    void            SetFirstConfig( BOOL bReady ) { if( bReady ) SetEvent( m_hGotFirstConfig ); else ResetEvent( m_hGotFirstConfig ); };
    BOOL            GotFirstConfig() { if( WaitForSingleObject( m_hGotFirstConfig, 0 ) == WAIT_OBJECT_0 ) return TRUE; else return FALSE; };

    // Get data useful for the engine (current position, etc. etc.)
    D3DXVECTOR3     GetCameraPos() const { return m_vCameraPos; };
    ANGLE           GetCameraYaw() const { return m_aCameraYaw; };
    DWORD           GetNumPlayerObjects() const { return m_dwNumPlayerObjects; };

    // Get first engine PlayerObject is a cell
    // NOTE: The engine must lock the world DB before traversing the cells
    PlayerObject*   GetFirstPlayerObjectInCell( DWORD x, DWORD z );

    // The layout of the maze (engine needs this to draw)
    CMaze           m_Maze;

    // Get network stats
    DWORD           GetThroughputBPS();
    DWORD           GetRoundTripLatencyMS();
    void            GetPlayerStats( DWORD* pdwNumPlayers, DWORD* pdwNumNearbyPlayers ) { m_StatLock.Enter(); *pdwNumPlayers = m_dwNumPlayers; *pdwNumNearbyPlayers = m_dwNumNearbyPlayers; m_StatLock.Leave(); };
    void            SetPlayerStats( DWORD dwNumPlayers, DWORD dwNumNearbyPlayers )     { m_StatLock.Enter(); m_dwNumPlayers = dwNumPlayers;   m_dwNumNearbyPlayers = dwNumNearbyPlayers;   m_StatLock.Leave(); };
    DWORD           GetLocalClientID() const { return m_dwLocalClientID; };

    // Autopilot
    void    EngageAutopilot( BOOL engage );
    void    SetAutopilot(BOOL engage) { m_bEngageAutopilot = engage; };
    BOOL    IsAutopilot() const { return m_bAutopilot; };

    // Set whether or not we have input focus
    void    SetInputFocus( BOOL havefocus ) { m_bHaveInputFocus = havefocus; };

protected:
    CMazeApp*       m_pMazeApp;
    IMazeGraphics*  m_pMazeGraphics;

    D3DXVECTOR3     m_vCameraPos;
    ANGLE           m_aCameraYaw;

    DWORD           m_dwNumPlayerObjects;

    CRandom         m_Rand;

    IOutboundClient* m_pNet;

    FLOAT           m_fLastOutboundTime;

    DWORD           m_dwNumPlayers;
    DWORD           m_dwNumNearbyPlayers;
    DWORD           m_dwLocalClientID;

    BOOL            m_bHaveInputFocus;

    void            SendPacket( ClientPacket* packet , DWORD size , BOOL guaranteed, DWORD dwTimeout );

    BOOL            IsValidPackSize( DWORD dwSize );

    // Arrays of cells and PlayerObjects (this consitutes the world DB), with associated lock
    PlayerObject*       m_pctCells[SERVER_MAX_HEIGHT][SERVER_MAX_WIDTH];
    PlayerObject        m_PlayerObjects[MAX_PLAYER_OBJECTS];
    CCriticalSection    m_WorldLock;
    CCriticalSection    m_StatLock;
    HANDLE              m_hReady;
    

    // Autopilot stuff
    struct  AutopilotCell
    {
        AutopilotCell() {};
        AutopilotCell( BYTE X , BYTE Y ) : x(X),y(Y) {};
        BYTE   x,y;
    };
    SimpleStack<AutopilotCell,MAZE_SIZE>    m_AutopilotStack;
    BYTE                                   m_AutopilotVisited[MAZE_HEIGHT][MAZE_WIDTH];
    BOOL            m_bAutopilot;
    BOOL            m_bEngageAutopilot;
    D3DXVECTOR3     m_vAutopilotTarget;
    ANGLE           m_aAutopilotTargetAngle;

    void            DoAutopilot( FLOAT elapsed );
    void            DoManualPilot( FLOAT elapsed ); 
    void            PickAutopilotTarget();

    void    HandlePlayerObjectsInAckPacket( ServerAckPacket* ppacket );

    void    AddPlayerObjectToCells( PlayerObject* pPlayerObject );
    void    RemovePlayerObjectFromCells( PlayerObject* pPlayerObject );
    BOOL    IsPlayerObjectInCell( DWORD wCellX, DWORD wCellY, PlayerObject* pPlayerObject );

    HANDLE              m_hGotFirstConfig;

    ClientNetConfig     m_NetConfig;
    CRandom             m_NetRandom;
    BOOL                m_bDoneInit;

private:
    CMazeClient( const CMazeClient& );
    void operator=( const CMazeClient& );
};




#endif

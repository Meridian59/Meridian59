//----------------------------------------------------------------------------
// File: mazeserver.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MAZESERVER_H
#define _MAZESERVER_H





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include "NetAbstract.h"
#include "SyncObjects.h"
#include "Maze.h"
#include "Random.h"
#include "Packets.h"
#include "Trig.h"

class   CMaze;
struct  ClientPosPacket;

#define SERVER_MAX_WIDTH    128
#define SERVER_MAX_HEIGHT   128
#define DEFAULT_MAZE_WIDTH  16
#define DEFAULT_MAZE_HEIGHT 16
#define DEFAULT_SEED        314159
#define LOCK_GRID_SIZE      16




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct PlayerData
{
    DWORD           dwID;                   // Client ID
    DWORD           NetID;                  // NetID for owning player (0 if none)
    DWORD           dwVersion;              // Version of the owning player
    BOOL            bAllow;                 // If FALSE, then we should drop this player

    // Links for the various lists
    PlayerData*     pNext;                  // Free/active PlayerData list (double link)
    PlayerData*     pPrevious;
    PlayerData*     pNextInCell;            // Cell list (single link)
    PlayerData*     pNextInIDHashBucket;    // ID hash bucket (single link)

    FLOAT           fLastDisplayTime;
    FLOAT           fLastCITime;
    BOOL            bActive;

    float           fPosX;                  // Floating point position
    float           fPosY;
    WORD            wCellX;                 // Coordinates of the cell this player is in
    WORD            wCellY;                 // or (0xffff,0xffff) for off-map
    ANGLE           aCameraYaw;
    DWORD           dwNumNearbyPlayers;     // Number of nearby players

    DWORD           pad[4];
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct ServerCell
{
    PlayerData* pFirstPlayerData;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CMazeServer : public INetServer
{
public:
    CMazeServer();

    // From INetServer
    void    OnAddConnection( DWORD dwID );
    HRESULT OnPacket( DWORD from, void* pData, DWORD dwSize );
    void    OnRemoveConnection( DWORD dwID );
    void    OnSessionLost( DWORD dwReason );

    // Hook up to the network service
    void    SetOutboundServer( IOutboundServer* poutnet ) { m_pNet = poutnet; };

    // Initialisation - need to hook up a maze object
    HRESULT Init( BOOL bLocalLoopback, const CMaze* pmaze );
    void    Shutdown();

    // Chance of server sending packets via reliable transport
    void    SetServerReliableRate( DWORD percent ) { m_dwServerReliableRate = percent; };
    DWORD   GetServerReliableRate() const { return m_dwServerReliableRate; };

    // Timeout of server's packets
    void    SetServerTimeout( DWORD timeout ) { m_dwServerTimeout = timeout; };
    DWORD   GetServerTimeout() const { return m_dwServerTimeout; };

    // Change of client sending packets via reliable transport. Setting this causes the server
    // to propagate this setting to all currently connected clients
    void    SetClientReliableRate( DWORD percent );
    DWORD   GetClientReliableRate() const { return DWORD(m_ClientNetConfig.ubReliableRate); };

    // Change client update rate. Setting this causes the server to propagate this setting to all
    // currently connected clients
    void    SetClientUpdateRate( DWORD rate );
    DWORD   GetClientUpdateRate() const { return DWORD(m_ClientNetConfig.wUpdateRate); };

    // Change client timeout. Setting this causes the server to propagate this setting to all
    // currently connected clients
    void    SetClientTimeout( DWORD timeout );
    DWORD   GetClientTimeout() const { return DWORD(m_ClientNetConfig.wTimeout); };

    // Change Client position pack size. Setting this causes the server to propagate this setting to all
    // currently connected clients
    void    SetClientPackSize( DWORD size );
    DWORD   GetClientPackSize() const { return DWORD(m_ClientNetConfig.wClientPackSizeArray[m_ClientNetConfig.ubClientPackIndex]); };

    // Change Server position pack size.
    void    SetServerPackSize( DWORD size );
    DWORD   GetServerPackSize() const { return DWORD(m_ClientNetConfig.wServerPackSizeArray[m_ClientNetConfig.ubServerPackIndex]); };
    
    // How long the user wants to hold the Server's Dplay Threads
    void    SetServerThreadWait( DWORD threadwait ) { m_dwServerThreadWait = threadwait; };
    DWORD   GetServerThreadWait() const { return m_dwServerThreadWait; };

    // How long the user wants to hold the Server's Dplay Threads
    void    SetClientThreadWait( DWORD threadwait );
    DWORD   GetClientThreadWait() const { return DWORD(m_ClientNetConfig.dwThreadWait); };
    
    
    // Various commands
    void    DisplayConnectionInfo( DWORD dwID );
    void    DisplayNextConnectionInfo();
    void    PrintStats();

    void    SetLogLevel( DWORD dwLogLevel ) { m_dwLogLevel = dwLogLevel; }
    DWORD   GetLogLevel() { return m_dwLogLevel; }

    DWORD   GetNumPlayers() { return m_dwPlayerCount; }

protected:
    BOOL                m_bLocalLoopback;
    IOutboundServer*    m_pNet;
    const CMaze*        m_pMaze;
    DWORD               m_dwLogLevel;
    DWORD               m_dwWidth;
    DWORD               m_dwHeight;
    CCriticalSection    m_AddRemoveLock;

    // A fixed sized grid of locks which we lay over the maze to control access to it
    // We demand that the maze dimensions are a power-of-2 times the dimensions of this
    // grid, and pre-store that power to allow fast translation
    CLockArray<LOCK_GRID_SIZE,LOCK_GRID_SIZE>   m_LockGrid;
    DWORD               m_dwMazeXShift;
    DWORD               m_dwMazeYShift;
    void                LockCell( DWORD x , DWORD y );
    void                UnlockCell( DWORD x , DWORD y );
    void                LockRange( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 );
    void                UnlockRange( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 );
    void                LockCellPair( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 );
    void                UnlockCellPair( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 );
    CCriticalSection    m_OffMapLock;

    // The PlayerData lists
    PlayerData          m_PlayerDatas[MAX_PLAYER_OBJECTS];
    PlayerData*         m_pFirstActivePlayerData;
    PlayerData*         m_pFirstFreePlayerData;
    DWORD               m_dwActivePlayerDataCount;
    DWORD               m_dwPlayerDataUniqueValue;
    CCriticalSection    m_PlayerDataListLock;

    // The player object locks
    enum { NUM_PLAYER_OBJECT_LOCKS = 16 };
    CCriticalSection    m_PlayerDataLocks[NUM_PLAYER_OBJECT_LOCKS];
    void                LockPlayerData( PlayerData* pPlayerData ) { m_PlayerDataLocks[((pPlayerData-m_PlayerDatas) & (NUM_PLAYER_OBJECT_LOCKS-1))].Enter(); };
    void                UnlockPlayerData( PlayerData* pPlayerData ) { m_PlayerDataLocks[((pPlayerData-m_PlayerDatas) & (NUM_PLAYER_OBJECT_LOCKS-1))].Leave(); };

    PlayerData*        CreatePlayerData();
    void                DestroyPlayerData( PlayerData* pPlayerData );

    // The cell array and the "off-map" cell.
    ServerCell          m_OffMapCell;
    ServerCell          m_Cells[SERVER_MAX_WIDTH][SERVER_MAX_HEIGHT];

    // Remove playerdata from its cell
    void    RemovePlayerDataFromCell( PlayerData* pPlayerData );

    // Unsafe versions of add/remove. Must have playerdata and cell locked when you call this
    void    UnsafeRemovePlayerDataFromCell( PlayerData* pPlayerData );
    void    UnsafeAddPlayerDataToCell( PlayerData* pPlayerData );

    ServerCell* GetCell( PlayerData* pPlayerData )
    {
        if ( pPlayerData->wCellX == 0xffff )
            return &m_OffMapCell;
        else
            return &m_Cells[pPlayerData->wCellY][pPlayerData->wCellX];
    };

    void    HandleClientPosPacket( DWORD dwFrom, ClientPosPacket* pPacket );
    void    HandleClientVersionPacket( DWORD dwFrom, ClientVersionPacket* pClientVersionPack );
    void    HandleUnknownPacket( DWORD dwFrom, ClientPacket* pClientPack, DWORD size );

    BOOL    IsValidPackSize( DWORD wSize );
    BOOL    IsClientVersionSupported( DWORD dwClientVersion );

    DWORD   m_dwPlayerCount;
    
    CCriticalSection    m_csThreadCountLock;    
    WORD   m_wActiveThreadCount;
    WORD   m_wMaxThreadCount;
    FLOAT  m_fAvgThreadCount;
    FLOAT  m_fAvgThreadTime;
    FLOAT  m_fMaxThreadTime;

    DWORD  m_dwPeakPlayerCount;

    // Hashing DPIDs to PlayerData pointers
    void                SetPlayerDataForID( DWORD dwID, PlayerData* pPlayerData );
    PlayerData*        GetPlayerDataForID( DWORD dwID );
    void                RemovePlayerDataID( PlayerData* pPlayerData );
    DWORD               IDHash( DWORD dwID );
    enum { NUM_ID_HASH_BUCKETS = 1024 };
    enum { NUM_ID_HASH_BUCKET_LOCKS = 16 };
    PlayerData*        m_pstIDHashBucket[NUM_ID_HASH_BUCKETS];
    CCriticalSection    m_IDHashBucketLocks[NUM_ID_HASH_BUCKET_LOCKS];

    // Random number generator
    CRandom     m_Rand;

    // Send packet wrapper
    HRESULT SendPacket( DWORD dwTo, void* pData, DWORD dwSize, BOOL bReliable, DWORD dwTimeout );
    void SendConfigPacketToAll( ServerConfigPacket* pPacket );

    // Network configuration parameters
    DWORD               m_dwServerReliableRate;
    DWORD               m_dwServerTimeout;
    DWORD               m_dwServerThreadWait;

    ClientNetConfig     m_ClientNetConfig;
    CCriticalSection    m_ClientNetConfigLock;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: This function is called by the server to output informational text.
//       In the client it should probably just be a dummy function, in the server 
//       it should probably just spew out to the console
//-----------------------------------------------------------------------------
enum ServerBufferType { SLINE_PROMPT, SLINE_INPUT, SLINE_LOG, SLINE_CMD };
void ConsolePrintf( ServerBufferType enumLineType, const TCHAR* fmt , ... );



#endif

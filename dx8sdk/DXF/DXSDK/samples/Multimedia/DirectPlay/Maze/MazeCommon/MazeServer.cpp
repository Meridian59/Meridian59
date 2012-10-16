//----------------------------------------------------------------------------
// File: mazeserver.cpp
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS
#include <windows.h>
#include <d3dx.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dxerr8.h>
#include "DXUtil.h"
#include "MazeServer.h"
#include "Packets.h"
#include "Maze.h"
#include <malloc.h>
#include <tchar.h>



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMazeServer::CMazeServer()
{
    m_dwPlayerCount         = 0;
    
    m_wActiveThreadCount   = 0;
    m_wMaxThreadCount      = 0;
    m_fAvgThreadCount      = 0;
    m_fAvgThreadTime       = 0;
    m_fMaxThreadTime       = 0;

    m_dwServerReliableRate  = 15;
    m_dwServerTimeout       = 150;
    m_dwLogLevel            = 2;
    m_pMaze                 = NULL;

    m_ClientNetConfig.ubReliableRate = 15;
    m_ClientNetConfig.wUpdateRate    = 150;
    m_ClientNetConfig.wTimeout       = 150;

    m_ClientNetConfig.dwThreadWait = 0;

    m_ClientNetConfig.ubClientPackIndex = 0;
    m_ClientNetConfig.ubServerPackIndex = 0;
    for(WORD x = 0; x < PACK_ARRAY_SIZE; x++)
    {
        m_ClientNetConfig.wClientPackSizeArray[x] = 0;
        m_ClientNetConfig.wServerPackSizeArray[x] = 0;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeServer::Init( BOOL bLocalLoopback, const CMaze* pMaze )
{
    m_bLocalLoopback = bLocalLoopback;
    m_pMaze = pMaze;
    if( m_pMaze == NULL )
        return DXTRACE_ERR( TEXT("Param"), E_FAIL );

    // Grab height and width of maze
    m_dwWidth = m_pMaze->GetWidth();
    m_dwHeight = m_pMaze->GetHeight();

    m_ClientNetConfig.dwMazeWidth  = m_dwWidth;
    m_ClientNetConfig.dwMazeHeight = m_dwHeight;

    // Validate size. Must be a power-of-2 times LOCK_GRID_SIZE. Compute the shifts.
    if( m_dwWidth > SERVER_MAX_WIDTH || m_dwHeight > SERVER_MAX_HEIGHT )
        return DXTRACE_ERR( TEXT("Maze height and width need to be less than 128"), E_INVALIDARG );
    if( (m_dwWidth % LOCK_GRID_SIZE) != 0 || (m_dwHeight % LOCK_GRID_SIZE) != 0 )
        return DXTRACE_ERR( TEXT("Maze height and width need to be divisable by 16"), E_INVALIDARG );

    DWORD scale = m_dwWidth / LOCK_GRID_SIZE;
    m_dwMazeXShift = 0;
    while ( (scale >>= 1) )
        m_dwMazeXShift++;

    scale = m_dwHeight / LOCK_GRID_SIZE;
    m_dwMazeYShift = 0;
    while ( (scale >>= 1) )
        m_dwMazeYShift++;

    if( ((DWORD(LOCK_GRID_SIZE) << m_dwMazeXShift) != m_dwWidth) ||
        ((DWORD(LOCK_GRID_SIZE) << m_dwMazeYShift) != m_dwHeight) )
        return DXTRACE_ERR( TEXT("Maze height and width need to be power of 2"), E_INVALIDARG );

    // Initialise the player list
    ZeroMemory( m_PlayerDatas, sizeof(m_PlayerDatas) );
    m_pFirstActivePlayerData = NULL;
    m_pFirstFreePlayerData = m_PlayerDatas;
    for( DWORD i = 1; i < MAX_PLAYER_OBJECTS-1; i++ )
    {
        m_PlayerDatas[i].pNext = &m_PlayerDatas[i+1];
        m_PlayerDatas[i].pPrevious = &m_PlayerDatas[i-1];
    }

    m_PlayerDatas[0].pNext = &m_PlayerDatas[1];
    m_PlayerDatas[MAX_PLAYER_OBJECTS-1].pPrevious = &m_PlayerDatas[MAX_PLAYER_OBJECTS-2];
    m_dwActivePlayerDataCount = 0;
    m_dwPlayerDataUniqueValue = 0;

    // Initialise the cells
    ZeroMemory( m_Cells, sizeof(m_Cells) );
    ZeroMemory( &m_OffMapCell, sizeof(m_OffMapCell) );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::Shutdown()
{
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::LockRange( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
{
    m_LockGrid.LockRange( x1>>m_dwMazeXShift, y1>>m_dwMazeYShift ,
                          x2>>m_dwMazeXShift, y2>>m_dwMazeYShift );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::UnlockRange( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
{
    m_LockGrid.UnlockRange( x1>>m_dwMazeXShift, y1>>m_dwMazeYShift ,
                            x2>>m_dwMazeXShift, y2>>m_dwMazeYShift );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::LockCell( DWORD x, DWORD y )
{
    if( x == 0xffff )
        m_OffMapLock.Enter();
    else
        m_LockGrid.LockCell(x>>m_dwMazeXShift,y>>m_dwMazeYShift);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::UnlockCell( DWORD x, DWORD y )
{
    if( x == 0xffff )
        m_OffMapLock.Leave();
    else
        m_LockGrid.UnlockCell(x>>m_dwMazeXShift,y>>m_dwMazeYShift);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::LockCellPair( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
{
    if( x1 == x2 && y1 == y2 )
    {
        if( x1 != 0xffff && y2 != 0xffff )
            LockCell( x1, y1 );
        else
            m_OffMapLock.Enter();

        return;
    }

    DWORD x1shift = x1>>m_dwMazeXShift;
    DWORD x2shift = x2>>m_dwMazeXShift;
    DWORD y1shift = y1>>m_dwMazeYShift;
    DWORD y2shift = y2>>m_dwMazeYShift;

    if( x1 == 0xffff )
    {
        m_OffMapLock.Enter();
        m_LockGrid.LockCell(x2shift,y2shift);
    }
    else if( x2 == 0xffff )
    {
        m_OffMapLock.Enter();
        m_LockGrid.LockCell(x1shift,y1shift);
    }
    else 
    {
        m_LockGrid.LockCellPair(x1shift,y1shift,x2shift,y2shift);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::UnlockCellPair( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
{
    if( x1 == x2 && y1 == y2 )
    {
        if( x1 != 0xffff && y2 != 0xffff )
            UnlockCell( x1, y1 );
        else
            m_OffMapLock.Leave();

        return;
    }

    DWORD x1shift = x1>>m_dwMazeXShift;
    DWORD x2shift = x2>>m_dwMazeXShift;
    DWORD y1shift = y1>>m_dwMazeYShift;
    DWORD y2shift = y2>>m_dwMazeYShift;

    if( x1 == 0xffff )
    {
        m_LockGrid.UnlockCell(x2shift,y2shift);
        m_OffMapLock.Leave();
    }
    else if( x2 == 0xffff )
    {
        m_LockGrid.UnlockCell(x1shift,y1shift);
        m_OffMapLock.Leave();
    }
    else 
    {
        m_LockGrid.UnlockCellPair(x1shift,y1shift,x2shift,y2shift);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::OnAddConnection( DWORD id )
{
    m_AddRemoveLock.Enter();

    // Increment our count of players
    m_dwPlayerCount++;
    if( m_dwLogLevel > 0 )
    {
        ConsolePrintf( SLINE_LOG, TEXT("Adding player DPNID %0.8x"), id );
        ConsolePrintf( SLINE_LOG, TEXT("Players connected = %d"), m_dwPlayerCount );
    }

    if( m_dwPlayerCount > m_dwPeakPlayerCount )
        m_dwPeakPlayerCount = m_dwPlayerCount;

    // Create a player for this client
    PlayerData* pPlayerData = CreatePlayerData();
    if( pPlayerData == NULL )
    {
        ConsolePrintf( SLINE_LOG, TEXT("ERROR! Unable to create new PlayerData for client!") );
        DXTRACE_ERR( TEXT("CreatePlayerData"), E_FAIL );
        m_AddRemoveLock.Leave();
        return;
    }

    // Store that pointer as local player data
    SetPlayerDataForID( id, pPlayerData );

    // Grab net config into to send to client
    m_ClientNetConfigLock.Enter();
    ServerConfigPacket packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    // Send it
    SendPacket( id, &packet, sizeof(packet), TRUE, 0 );

    m_AddRemoveLock.Leave();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::OnRemoveConnection( DWORD id )
{
    m_AddRemoveLock.Enter();

    // Decrement count of players
    m_dwPlayerCount--;

    if( m_dwLogLevel > 0 )
    {
        ConsolePrintf( SLINE_LOG, TEXT("Removing player DPNID %0.8x"), id );
        ConsolePrintf( SLINE_LOG, TEXT("Players connected = %d"), m_dwPlayerCount );
    }

    // Find playerdata for this client
    PlayerData* pPlayerData = GetPlayerDataForID( id );
    if( pPlayerData != NULL )
    {
        // Destroy it
        RemovePlayerDataID( pPlayerData );
        DestroyPlayerData( pPlayerData );
    }

    m_AddRemoveLock.Leave();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeServer::OnPacket( DWORD dwFrom, void* pData, DWORD size )
{
    BOOL fFoundSize = FALSE;

    // Increment the number of thread we have in this process.
    m_csThreadCountLock.Enter();

    //Get the start time of when we entered the message handler.
    FLOAT   fStartTime = DXUtil_Timer( TIMER_GETAPPTIME );

    m_wActiveThreadCount++;
    if(m_wActiveThreadCount > m_wMaxThreadCount)
        m_wMaxThreadCount = m_wActiveThreadCount;
    
    // Calculate and average.
    FLOAT fdiff = m_wActiveThreadCount - m_fAvgThreadCount;
    m_fAvgThreadCount += fdiff/32;
    
    m_csThreadCountLock.Leave();


    ClientPacket* pClientPack = (ClientPacket*)pData;
    switch( pClientPack->wType )
    {
        case PACKETTYPE_CLIENT_POS:
            
            // Check to see if the packet has a valid size. Including 
            // the custom pack size.
            if( size < sizeof(ClientPosPacket))
                fFoundSize = FALSE;
            else if( ! IsValidPackSize(size - sizeof(ClientPosPacket)))
                fFoundSize = FALSE;
            else
                fFoundSize = TRUE;

            // If valid sized packet, handle the position.
            if(fFoundSize)
                HandleClientPosPacket( dwFrom, (ClientPosPacket*)pClientPack );
            else
                m_pNet->RejectClient( dwFrom, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE );

            break;

        case PACKETTYPE_CLIENT_VERSION:
            if( size == sizeof(ClientVersionPacket) )
                HandleClientVersionPacket( dwFrom, (ClientVersionPacket*)pClientPack );
            else
                m_pNet->RejectClient( dwFrom, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE );
            break;

        case PACKETTYPE_SERVER_CONFIG:

            // Server config packet sent to all users (including server). Just ignore and continue.

            break;
        default:
            HandleUnknownPacket( dwFrom, pClientPack, size );
            break;
    }

    //If the user wants to hold the thread, Sleep for given amount of time.
    if ( m_dwServerThreadWait > 0 )
    {
        Sleep( m_dwServerThreadWait );
    }
    
    // Retrieve thread data for this process.
    m_csThreadCountLock.Enter();

    m_wActiveThreadCount--;

    FLOAT fDiffTime = (DXUtil_Timer( TIMER_GETAPPTIME ) - fStartTime) - m_fAvgThreadTime;
    m_fAvgThreadTime += fDiffTime/32;

    //Get the Max time in the thread.
    if ( fDiffTime > m_fMaxThreadTime )
    {
        m_fMaxThreadTime = fDiffTime;
    }

    m_csThreadCountLock.Leave();

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMazeServer::IsValidPackSize( DWORD dwSize )
{
    BOOL fFoundSize = FALSE;
    BYTE ubPackLocation = m_ClientNetConfig.ubClientPackIndex;
    
    // Check through the array of valid pack sizes.
    if(dwSize != m_ClientNetConfig.wClientPackSizeArray[ubPackLocation])
    {
        for( --ubPackLocation; ubPackLocation != m_ClientNetConfig.ubClientPackIndex; ubPackLocation--)
        {
            if(dwSize == m_ClientNetConfig.wClientPackSizeArray[ubPackLocation])
            {
                // Found valid size in the array.
                fFoundSize = TRUE;
                break;
            }
            if(ubPackLocation >= PACK_ARRAY_SIZE)  ubPackLocation = PACK_ARRAY_SIZE;  //Wrap the array.
        }
    }
    else
    {
        fFoundSize = TRUE;
    }

    return fFoundSize;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::OnSessionLost( DWORD dwReason )
{
    ConsolePrintf( SLINE_LOG, TEXT("ERROR! Session was lost") );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
PlayerData* CMazeServer::CreatePlayerData()
{
    m_PlayerDataListLock.Enter();

    // Grab first free player in the list
    PlayerData* pPlayerData = m_pFirstFreePlayerData;

    if( pPlayerData )
    {
        LockPlayerData( pPlayerData );

        // Got one, so remove it from the free list
        if( pPlayerData->pPrevious )
            pPlayerData->pPrevious->pNext = pPlayerData->pNext;
        if( pPlayerData->pNext )
            pPlayerData->pNext->pPrevious = pPlayerData->pPrevious;
        m_pFirstFreePlayerData = pPlayerData->pNext;

        // Add it to the active list
        if( m_pFirstActivePlayerData )
            m_pFirstActivePlayerData->pPrevious = pPlayerData;
        pPlayerData->pNext = m_pFirstActivePlayerData;
        pPlayerData->pPrevious = NULL;
        m_pFirstActivePlayerData = pPlayerData;

        // Update count of players
        m_dwActivePlayerDataCount++;

        // Generate the ID for this player
        m_dwPlayerDataUniqueValue++;
        pPlayerData->dwID = (DWORD) ((pPlayerData-m_PlayerDatas)|(m_dwPlayerDataUniqueValue<<PLAYER_OBJECT_SLOT_BITS));

        pPlayerData->pNextInIDHashBucket = NULL;
        pPlayerData->NetID = 0;
        pPlayerData->dwNumNearbyPlayers = 0;

        // Insert into the "off-map" cell
        pPlayerData->fPosX = pPlayerData->fPosY = -1;
        pPlayerData->wCellX = pPlayerData->wCellY = 0xffff;
        m_OffMapLock.Enter();
        pPlayerData->pNextInCell = m_OffMapCell.pFirstPlayerData;
        m_OffMapCell.pFirstPlayerData = pPlayerData;
        m_OffMapLock.Leave();

        // Mark as active
        pPlayerData->bActive = TRUE;

        UnlockPlayerData( pPlayerData );
    }

    m_PlayerDataListLock.Leave();

    return pPlayerData;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::DestroyPlayerData( PlayerData* pPlayerData )
{
    m_PlayerDataListLock.Enter();
    LockPlayerData( pPlayerData );

    // Remove the player from its cell
    RemovePlayerDataFromCell( pPlayerData );

    // Mark as inactive
    pPlayerData->bActive = FALSE;

    // Remove player from active list
    if( pPlayerData->pPrevious )
        pPlayerData->pPrevious->pNext = pPlayerData->pNext;
    if( pPlayerData->pNext )
        pPlayerData->pNext->pPrevious = pPlayerData->pPrevious;

    if( m_pFirstActivePlayerData == pPlayerData )
        m_pFirstActivePlayerData = pPlayerData->pNext;

    // Add it to the free list
    if( m_pFirstFreePlayerData )
        m_pFirstFreePlayerData->pPrevious = pPlayerData;
    pPlayerData->pNext = m_pFirstFreePlayerData;
    pPlayerData->pPrevious = NULL;
    m_pFirstFreePlayerData = pPlayerData;

    // Update count of players
    m_dwActivePlayerDataCount--;

    UnlockPlayerData( pPlayerData );
    m_PlayerDataListLock.Leave();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::RemovePlayerDataFromCell( PlayerData* pPlayerData )
{
    // Lock the player
    LockPlayerData( pPlayerData );

    // Lock the cell the player is in
    ServerCell* pCell;
    if( pPlayerData->wCellX == 0xffff )
    {
        m_OffMapLock.Enter();
        pCell = &m_OffMapCell;
    }
    else
    {
        LockCell( pPlayerData->wCellX, pPlayerData->wCellY );
        pCell = &m_Cells[pPlayerData->wCellY][pPlayerData->wCellX];
    }

    // Remove it from the cell
    PlayerData* pPt = pCell->pFirstPlayerData;
    PlayerData* pPrev = NULL;
    while ( pPt )
    {
        if( pPt == pPlayerData )
        {
            if( pPrev )
                pPrev->pNextInCell = pPlayerData->pNextInCell;
            else
                pCell->pFirstPlayerData = pPlayerData->pNextInCell;

            pPlayerData->pNextInCell = NULL;
            break;
        }
        pPrev = pPt;
        pPt = pPt->pNextInCell;
    }

    // Unlock the cell
    if( pPlayerData->wCellX == 0xffff )
        m_OffMapLock.Leave();
    else
        UnlockCell( pPlayerData->wCellX, pPlayerData->wCellY );

    // Unlock the player
    UnlockPlayerData( pPlayerData );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::UnsafeRemovePlayerDataFromCell( PlayerData* pPlayerData )
{
    ServerCell* pCell = GetCell( pPlayerData );
    PlayerData* pPt  = pCell->pFirstPlayerData;
    PlayerData* pPrev = NULL;
    while ( pPt )
    {
        if( pPt == pPlayerData )
        {
            if( pPrev )
                pPrev->pNextInCell = pPlayerData->pNextInCell;
            else
                pCell->pFirstPlayerData = pPlayerData->pNextInCell;
            pPlayerData->pNextInCell = NULL;
            break;
        }
        pPrev = pPt;
        pPt = pPt->pNextInCell;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::UnsafeAddPlayerDataToCell( PlayerData* pPlayerData )
{
    ServerCell* pCell   = GetCell( pPlayerData );
    pPlayerData->pNextInCell = pCell->pFirstPlayerData;
    pCell->pFirstPlayerData = pPlayerData;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::HandleClientPosPacket( DWORD dwFrom, ClientPosPacket* pClientPosPack )
{
    // Grab player for this client and lock it
    PlayerData* pFromPlayer = GetPlayerDataForID( dwFrom );
    if( pFromPlayer == NULL )
    {
        if( m_dwLogLevel > 1 )
            ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Could not find data structure for this player"), pFromPlayer->NetID );
        return;
    }

    LockPlayerData( pFromPlayer );

    if( FALSE == pFromPlayer->bAllow )
    {
        if( m_dwLogLevel > 0 )
            ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Got position packet from bad client.  Rejecting client"), pFromPlayer->NetID );

        m_pNet->RejectClient( dwFrom, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE );
        UnlockPlayerData( pFromPlayer );
        return;
    }

    // Compute the cell the player should be in now
    DWORD newcellx = int(pClientPosPack->fX);
    DWORD newcelly = int(pClientPosPack->fY);
    DWORD oldcellx = pFromPlayer->wCellX;
    DWORD oldcelly = pFromPlayer->wCellY;

    // Have we moved cell?
    if( newcellx != oldcellx || newcelly != oldcelly )
    {
        // Yes, so lock the pair of cells in question
        LockCellPair( oldcellx, oldcelly, newcellx, newcelly );

        // Remove from old cell and add to new cell
        UnsafeRemovePlayerDataFromCell( pFromPlayer );
        pFromPlayer->wCellX = WORD(newcellx); pFromPlayer->wCellY = WORD(newcelly);
        UnsafeAddPlayerDataToCell( pFromPlayer );

        // Unlock cells
        UnlockCellPair( oldcellx, oldcelly, newcellx, newcelly );
    }

    // Update player position
    pFromPlayer->fPosX      = pClientPosPack->fX;
    pFromPlayer->fPosY      = pClientPosPack->fY;
    pFromPlayer->aCameraYaw = pClientPosPack->aCameraYaw;

    // Allocate space to build the reply packet, and fill in header 
    DWORD dwAllocSize;
    ServerAckPacket* pSvrAckPack = NULL;

    // Begin by allocating a buffer sized according to
    // the current number of nearby players + 4.  This will give 
    // a little room for more players to come 'near' without resize
    // the buffer.
    DWORD dwMaxPlayerStatePackets = pFromPlayer->dwNumNearbyPlayers + 4;

    dwAllocSize = sizeof(ServerAckPacket) + dwMaxPlayerStatePackets*sizeof(PlayerStatePacket);
    pSvrAckPack = (ServerAckPacket*) realloc( pSvrAckPack, dwAllocSize );
    if( NULL == pSvrAckPack )
    {
        // Out of mem.  Cleanup and return
        UnlockPlayerData( pFromPlayer );
        return;       
    }
    ZeroMemory( pSvrAckPack, dwAllocSize );

    *pSvrAckPack = ServerAckPacket(m_dwPlayerCount);
    pSvrAckPack->wPlayerStatePacketCount = 0;
    PlayerStatePacket* pChunk = (PlayerStatePacket*)(pSvrAckPack+1);

    // Compute range of cells we're going to scan for players to send
    DWORD minx = (newcellx > 7) ? (newcellx - 7) : 0;
    DWORD miny = (newcelly > 7) ? (newcelly - 7) : 0;
    DWORD maxx = (newcellx+7 >= m_dwWidth) ? m_dwWidth-1 : newcellx+7;
    DWORD maxy = (newcelly+7 >= m_dwHeight) ? m_dwHeight-1 : newcelly+7;

    // Lock that range of cells
    LockRange( minx, miny, maxx, maxy );

    // Scan through the cells, tagging player data onto the end of
    // our pSvrAckPacket until we run out of room
    for( DWORD y = miny; y <= maxy; y++ )
    {
        for( DWORD x = minx; x <= maxx; x++ )
        {
            PlayerData* pCurPlayerData = m_Cells[y][x].pFirstPlayerData;
            while ( pCurPlayerData )
            {
                if( pCurPlayerData != pFromPlayer )
                {
                    if( pSvrAckPack->wPlayerStatePacketCount >= dwMaxPlayerStatePackets )
                    {
                        // Make sure pChunk is where we think it is
                        assert( (BYTE*) pChunk == (BYTE*) ((BYTE*)pSvrAckPack + sizeof(ServerAckPacket) + pSvrAckPack->wPlayerStatePacketCount*sizeof(PlayerStatePacket)) );

                        // There are more than just 4 new nearby players, so resize the 
                        // buffer pSvrAckPack to allow 16 more PlayerStatePacket's.
                        dwMaxPlayerStatePackets += 16;
                        dwAllocSize = sizeof(ServerAckPacket) + dwMaxPlayerStatePackets*sizeof(PlayerStatePacket);
                        ServerAckPacket* pNewSvrAckPack = NULL;
                        pNewSvrAckPack = (ServerAckPacket*) realloc( pSvrAckPack, dwAllocSize );
                        if( NULL == pNewSvrAckPack )
                        {
                            // Out of mem.  Cleanup and return
                            free( pSvrAckPack );
                            UnlockRange( minx, miny, maxx, maxy );
                            UnlockPlayerData( pFromPlayer );
                            return;       
                        }

                        pSvrAckPack = pNewSvrAckPack;
                        pChunk = (PlayerStatePacket*) ((BYTE*) ((BYTE*)pSvrAckPack + sizeof(ServerAckPacket) + pSvrAckPack->wPlayerStatePacketCount*sizeof(PlayerStatePacket) ) );

                        // Make sure pChunk is still where its supposed to be
                        assert( (BYTE*) pChunk == (BYTE*) ((BYTE*)pSvrAckPack + sizeof(ServerAckPacket) + pSvrAckPack->wPlayerStatePacketCount*sizeof(PlayerStatePacket)) );
                    }

                    pChunk->dwID       = pCurPlayerData->dwID;
                    pChunk->fX         = pCurPlayerData->fPosX;
                    pChunk->fY         = pCurPlayerData->fPosY;
                    pChunk->aCameraYaw = pCurPlayerData->aCameraYaw;
                    pChunk++;
                    pSvrAckPack->wPlayerStatePacketCount++;
                }
                pCurPlayerData = pCurPlayerData->pNextInCell;
            }
        }
    }

    // Update the dwNumNearbyPlayers for this player
    pFromPlayer->dwNumNearbyPlayers = pSvrAckPack->wPlayerStatePacketCount;

    // Unlock range of cells
    UnlockRange( minx, miny, maxx, maxy );

    if( m_dwLogLevel > 2 )
    {
        ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Position is (%0.2f,%0.2f)"), pFromPlayer->NetID, pFromPlayer->fPosX, pFromPlayer->fPosY );
    }
    else if( m_dwLogLevel == 2 )
    {
        FLOAT fTime = DXUtil_Timer( TIMER_GETAPPTIME );
        if( fTime - pFromPlayer->fLastDisplayTime > 60.0f )
        {
            ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Position is (%0.2f,%0.2f)"), pFromPlayer->NetID, pFromPlayer->fPosX, pFromPlayer->fPosY );
            pFromPlayer->fLastDisplayTime = fTime;
        }
    }

    // Unlock the playerdata
    UnlockPlayerData( pFromPlayer );

    // Send acknowledgement back to client, including list of nearby players 
    DWORD acksize = sizeof(ServerAckPacket) + (pSvrAckPack->wPlayerStatePacketCount * sizeof(PlayerStatePacket));
 
    // Pack the buffer with dummy data.
    if(m_ClientNetConfig.wServerPackSizeArray[m_ClientNetConfig.ubServerPackIndex] > 0)
    {
        DWORD   dwBufferSize = acksize + m_ClientNetConfig.wServerPackSizeArray[m_ClientNetConfig.ubServerPackIndex];
        VOID*   pTempBuffer = 0;

        pTempBuffer = malloc(dwBufferSize);
        if( NULL == pTempBuffer )
        {
            //Out of memory
            DXTRACE_ERR_NOMSGBOX( TEXT("System out of Memory!"), E_OUTOFMEMORY );
            free( pSvrAckPack );
            return;
        }

        FillMemory(pTempBuffer, dwBufferSize, 'Z');
        memcpy(pTempBuffer, pSvrAckPack, acksize);

        SendPacket( dwFrom, pTempBuffer, dwBufferSize, FALSE, m_dwServerTimeout );
    
        free(pTempBuffer);
    }   
    else
    {
        SendPacket( dwFrom, pSvrAckPack, acksize, FALSE, m_dwServerTimeout );
    }

    free( pSvrAckPack );

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::HandleClientVersionPacket( DWORD dwFrom, ClientVersionPacket* pClientVersionPack )
{
    // Grab playerdata for this client and lock it
    PlayerData* pPlayerData = GetPlayerDataForID( dwFrom );
    if( pPlayerData == NULL )
        return;
    LockPlayerData( pPlayerData );

    // Record the version number 
    pPlayerData->dwVersion = pClientVersionPack->dwVersion;

    if( m_bLocalLoopback )
        pPlayerData->bAllow = TRUE;
    else
        pPlayerData->bAllow = IsClientVersionSupported( pClientVersionPack->dwVersion );

    if( m_dwLogLevel > 0 )
        ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Client version=%d (%s)"), pPlayerData->NetID, pPlayerData->dwVersion, pPlayerData->bAllow ? TEXT("Accepted") : TEXT("Rejected") );

    if( FALSE == pPlayerData->bAllow )
    {
        if( m_dwLogLevel > 0 )
            ConsolePrintf( SLINE_LOG, TEXT("DPNID %0.8x: Rejecting client"), pPlayerData->NetID );

        m_pNet->RejectClient( dwFrom, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE );
        UnlockPlayerData( pPlayerData );
        return;
    }

    // Unlock the playerdata
    UnlockPlayerData( pPlayerData );

    // Send acknowledgement to client that the client was either accepted or rejected
    ServerAckVersionPacket packet( pPlayerData->bAllow, dwFrom );
    SendPacket( dwFrom, &packet, sizeof(packet), TRUE, 0 );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMazeServer::IsClientVersionSupported( DWORD dwClientVersion )
{
    switch( dwClientVersion )
    {
        case 107: // only v107 is supported
            return TRUE;
        default:
            return FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::HandleUnknownPacket( DWORD dwFrom, ClientPacket* pClientPack, DWORD size )
{
    if( m_dwLogLevel > 1 )
        ConsolePrintf( SLINE_LOG, TEXT("ERROR! Unknown %d byte packet from player %0.8x"), size, dwFrom );

    m_pNet->RejectClient( dwFrom, DISCONNNECT_REASON_CLIENT_OUT_OF_DATE );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
DWORD   CMazeServer::IDHash( DWORD id )
{
    DWORD   hash = ((id) + (id>>8) + (id>>16) + (id>>24)) & (NUM_ID_HASH_BUCKETS-1);
    return hash;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::RemovePlayerDataID( PlayerData* pPlayerData )
{
    // Hash the ID to a bucket number
    DWORD   bucket = IDHash( pPlayerData->NetID );

    // Lock that hash bucket
    const   DWORD   buckets_per_lock = NUM_ID_HASH_BUCKETS / NUM_ID_HASH_BUCKET_LOCKS;
    m_IDHashBucketLocks[bucket/buckets_per_lock].Enter();

    // Loop though players in bucket until we find the right one
    PlayerData* pPt = m_pstIDHashBucket[bucket];
    PlayerData* pPrev = NULL;
    while( pPt )
    {
        if( pPt == pPlayerData )
            break;
        pPrev = pPt;
        pPt = pPt->pNextInIDHashBucket;
    }

    if( pPt )
    {
        if( pPrev )
            pPrev->pNextInIDHashBucket = pPt->pNextInIDHashBucket;
        else
            m_pstIDHashBucket[bucket] = pPt->pNextInIDHashBucket;
        pPt->pNextInIDHashBucket = NULL;
    }

    // Unlock the hash bucket
    m_IDHashBucketLocks[bucket/buckets_per_lock].Leave();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetPlayerDataForID( DWORD id, PlayerData* pPlayerData )
{
    // Make sure this player isn't added twice to the m_pstIDHashBucket[]
    // otherwise there will be a circular reference
    PlayerData* pSearch = GetPlayerDataForID( id );
    if( pSearch != NULL )
        return;

    // Hash the ID to a bucket number
    DWORD   bucket = IDHash( id );

    // Lock that hash bucket
    const   DWORD   buckets_per_lock = NUM_ID_HASH_BUCKETS / NUM_ID_HASH_BUCKET_LOCKS;
    m_IDHashBucketLocks[bucket/buckets_per_lock].Enter();

    // Add player onto hash bucket chain
    pPlayerData->pNextInIDHashBucket = m_pstIDHashBucket[bucket];
    m_pstIDHashBucket[bucket] = pPlayerData;

    // Store net id in player
    pPlayerData->NetID = id;

    // Unlock the hash bucket
    m_IDHashBucketLocks[bucket/buckets_per_lock].Leave();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
PlayerData* CMazeServer::GetPlayerDataForID( DWORD id )
{
    // Hash the ID to a bucket number
    DWORD   bucket = IDHash( id );

    // Lock that hash bucket
    const   DWORD   buckets_per_lock = NUM_ID_HASH_BUCKETS / NUM_ID_HASH_BUCKET_LOCKS;
    m_IDHashBucketLocks[bucket/buckets_per_lock].Enter();

    // Loop though players in bucket until we find the right one
    PlayerData* pPlayerData = m_pstIDHashBucket[bucket];
    while ( pPlayerData )
    {
        if( pPlayerData->NetID == id )
            break;
        pPlayerData = pPlayerData->pNextInIDHashBucket;
    }

    // Unlock the hash bucket
    m_IDHashBucketLocks[bucket/buckets_per_lock].Leave();

    // Return the player we found (will be NULL if we couldn't find it)
    return pPlayerData;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: calls DisplayConnectionInfo for each connection in a round-robin manner
//-----------------------------------------------------------------------------
void CMazeServer::DisplayNextConnectionInfo()
{
    if( m_pNet )
    {
        // Find the player that was displayed the longest time ago, and display it.
        FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
        PlayerData* pOldestPlayerData = NULL;
        FLOAT fOldestTime = 0.0f;

        m_PlayerDataListLock.Enter();

        PlayerData* pPlayerData = m_pFirstActivePlayerData;
        while ( pPlayerData )
        {
            if( fCurTime - pPlayerData->fLastCITime > fOldestTime )
            {
                fOldestTime  = fCurTime - pPlayerData->fLastCITime;
                pOldestPlayerData = pPlayerData;
            }

            pPlayerData = pPlayerData->pNext;
        }

        // Display the player with the oldest CI field, and update its CI field.
        if( pOldestPlayerData )
        {
            ConsolePrintf( SLINE_LOG, TEXT("Displaying connection info for next player") );
            DisplayConnectionInfo( pOldestPlayerData->NetID );
            pOldestPlayerData->fLastCITime = fCurTime;
        }
        else
        {
            ConsolePrintf( SLINE_LOG, TEXT("No players found") );
        }

        m_PlayerDataListLock.Leave();
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::PrintStats()
{
    ConsolePrintf( SLINE_LOG, TEXT("Thread count: Active=%d Avg=%.2f Max=%d"), 
                                    m_wActiveThreadCount, m_fAvgThreadCount, m_wMaxThreadCount );
    ConsolePrintf( SLINE_LOG, TEXT("Thread Time: Avg=%.4f Max=%.4f(s)"),
                                    m_fAvgThreadTime, m_fMaxThreadTime );
    ConsolePrintf( SLINE_LOG, TEXT("Players online (not including server player): %d"), m_dwPlayerCount );
    ConsolePrintf( SLINE_LOG, TEXT("Peak player count: %d"), m_dwPeakPlayerCount );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::DisplayConnectionInfo( DWORD dwID )
{
    TCHAR strInfo[5000];
    TCHAR* strEndOfLine;
    TCHAR* strStartOfLine;

    // Query the IOutboudNet for info about the connection to this user
    m_pNet->GetConnectionInfo( dwID, strInfo );

    ConsolePrintf( SLINE_LOG, TEXT("Displaying connection info for %0.8x"), dwID );
    ConsolePrintf( SLINE_LOG, TEXT("(Key: G=Guaranteed NG=Non-Guaranteed B=Bytes P=Packets)") );

    // Display each line seperately
    strStartOfLine = strInfo;
    while( TRUE )
    {
        strEndOfLine = _tcschr( strStartOfLine, '\n' );
        if( strEndOfLine == NULL )
            break;

        *strEndOfLine = 0;
        ConsolePrintf( SLINE_LOG, strStartOfLine );
        strStartOfLine = strEndOfLine + 1;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeServer::SendPacket( DWORD to, void* pData, 
                                 DWORD size, BOOL reliable, DWORD dwTimeout )
{
    // Chance of forcing any packet to be delivered reliably
    if( m_Rand.Get( 100 ) < m_dwServerReliableRate )
        reliable = TRUE;

    return m_pNet->SendPacket( to, pData, size, reliable, dwTimeout );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SendConfigPacketToAll( ServerConfigPacket* pPacket )
{
    // If we're up and running, then send this new information to all clients
    if( m_pNet )
    {
        //Use the AllPlayers ID
        SendPacket( DPNID_ALL_PLAYERS_GROUP, pPacket, sizeof(ServerConfigPacket), TRUE, 0 );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetClientReliableRate( DWORD percent )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    m_ClientNetConfig.ubReliableRate = BYTE(percent);
    ServerConfigPacket packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetClientUpdateRate( DWORD rate )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    m_ClientNetConfig.wUpdateRate = WORD(rate);
    ServerConfigPacket  packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetClientTimeout( DWORD timeout )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    m_ClientNetConfig.wTimeout = WORD(timeout);
    ServerConfigPacket  packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetClientPackSize( DWORD size )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    
    m_ClientNetConfig.ubClientPackIndex++; //Increase index and verify location in array.
    if(m_ClientNetConfig.ubClientPackIndex >= PACK_ARRAY_SIZE)   
        m_ClientNetConfig.ubClientPackIndex = 0;

    m_ClientNetConfig.wClientPackSizeArray[m_ClientNetConfig.ubClientPackIndex] = WORD(size);
    ServerConfigPacket packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetServerPackSize( DWORD size )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    
    m_ClientNetConfig.ubServerPackIndex++; //Increase index and verify location in array.
    if(m_ClientNetConfig.ubServerPackIndex >= PACK_ARRAY_SIZE)   
        m_ClientNetConfig.ubServerPackIndex = 0;

    m_ClientNetConfig.wServerPackSizeArray[m_ClientNetConfig.ubServerPackIndex] = WORD(size);
    ServerConfigPacket packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeServer::SetClientThreadWait( DWORD dwThreadWait )
{
    // Update client config, and build packet containing that data
    m_ClientNetConfigLock.Enter();
    
    m_ClientNetConfig.dwThreadWait = dwThreadWait;
    ServerConfigPacket packet( m_ClientNetConfig );
    m_ClientNetConfigLock.Leave();

    SendConfigPacketToAll( &packet );
}

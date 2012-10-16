//----------------------------------------------------------------------------
// File: mazecient.cpp
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
#include <dplay8.h>
#include <dpaddr.h>
#include <dxerr8.h>
#include "DXUtil.h"
#include "SyncObjects.h"
#include "IMazeGraphics.h"
#include "DummyConnector.h"
#include "MazeApp.h"
#include "MazeClient.h"
#include "Packets.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#define NORTH_ANGLE 0x8000
#define EAST_ANGLE  0xc000
#define SOUTH_ANGLE 0x0000
#define WEST_ANGLE  0x4000




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMazeClient::CMazeClient()
{
    ZeroMemory( m_pctCells, sizeof(m_pctCells) );
    m_hReady            = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hGotFirstConfig   = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_pNet              = NULL;
    m_bAutopilot        = FALSE;
    m_bEngageAutopilot  = TRUE;
    m_dwNumNearbyPlayers = 0;

    m_NetConfig.ubReliableRate = 0;
    m_NetConfig.wUpdateRate    = 150;
    m_NetConfig.wTimeout       = 150;
    m_NetConfig.dwMazeWidth    = 0;
    m_NetConfig.dwMazeHeight   = 0;

    m_NetConfig.dwThreadWait = 0;

    m_NetConfig.ubClientPackIndex = 0;
    m_NetConfig.ubServerPackIndex = 0;
    for(WORD x = 0; x < PACK_ARRAY_SIZE; x++)
    {
        m_NetConfig.wClientPackSizeArray[x]  = 0;
        m_NetConfig.wServerPackSizeArray[x]  = 0;
    }

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMazeClient::~CMazeClient()
{
    CloseHandle( m_hGotFirstConfig );
    CloseHandle( m_hReady );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeClient::Init( CMazeApp* pMazeApp, IMazeGraphics* pMazeGraphics )
{
    m_pMazeApp              = pMazeApp;
    m_pMazeGraphics         = pMazeGraphics;
    m_aCameraYaw            = 0;
    m_fLastOutboundTime     = DXUtil_Timer( TIMER_GETAPPTIME );
    m_bHaveInputFocus       = TRUE;

    Reset();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeClient::Reset()
{
    m_dwNumPlayers = 0;
    m_bAutopilot   = FALSE;

    SetFirstConfig( FALSE );
    SetMazeReady( FALSE ); 

    ZeroMemory( m_pctCells, sizeof(m_pctCells) );

    PlayerObject* pPlayerObject = m_PlayerObjects;
    for( DWORD i = 0; i < MAX_PLAYER_OBJECTS; i++, pPlayerObject++ )
    {
        pPlayerObject->dwID = 0;
        pPlayerObject->wCellX = pPlayerObject->wCellY = 0xffff;
        pPlayerObject->pNext = NULL;
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::Shutdown()
{
    // Destroy the maze

    Reset();
    m_Maze.Empty();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::Update( FLOAT fElapsed )
{
    // Don't do anyPlayerObject until we get a server config packet
    if( !IsMazeReady() ) 
        return;

    if( m_bAutopilot )
        DoAutopilot( fElapsed );
    else
        DoManualPilot( fElapsed );

    // See if it's time to send a packet to the server with our updated coordinates
    FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
    if( (fCurTime - m_fLastOutboundTime)*1000.0f > m_NetConfig.wUpdateRate )
    {

        ClientPosPacket packet( m_vCameraPos.x, m_vCameraPos.z, m_aCameraYaw );

        // Pack the buffer with dummy data for testing.
        if(m_NetConfig.wClientPackSizeArray[m_NetConfig.ubClientPackIndex] > 0)
        {
            WORD    wBufferSize = sizeof(packet) + m_NetConfig.wClientPackSizeArray[m_NetConfig.ubClientPackIndex];
            VOID*   pTempBuffer = 0;

            pTempBuffer = malloc(wBufferSize);
            if( NULL == pTempBuffer )
            {
                //Out of memory, just bail
                DXTRACE_ERR_NOMSGBOX( TEXT("System out of Memory!"), E_OUTOFMEMORY );
                return;
            }

            
            FillMemory(pTempBuffer, wBufferSize, 'Z');
            memcpy(pTempBuffer, &packet, sizeof(packet));

            SendPacket( (ClientPacket*)pTempBuffer, wBufferSize, FALSE, m_NetConfig.wTimeout );
        
            free(pTempBuffer);
        }   
        else
        {
            SendPacket( &packet, sizeof(packet), FALSE, m_NetConfig.wTimeout );
        }

        m_fLastOutboundTime = fCurTime;

    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::DoManualPilot( FLOAT fElapsed )
{
    // Check if we have the input focus
    if( !m_bHaveInputFocus )
        return;

    // Do rotations
    if( GetAsyncKeyState( VK_LEFT ) & 0x8000 )
        m_aCameraYaw += (DWORD) ((fElapsed*1000.0f) * 40.0f);
    if( GetAsyncKeyState( VK_RIGHT ) & 0x8000 )
        m_aCameraYaw -= (DWORD) ((fElapsed*1000.0f) * 40.0f);

    float e = fElapsed*1000.0f;

    // Compute new position based key input
    D3DXVECTOR3 pos = m_vCameraPos;
    if( GetAsyncKeyState( VK_UP ) & 0x8000 )
    {
        pos.x -= Sin(m_aCameraYaw) * 0.002f * e;
        pos.z += Cos(m_aCameraYaw) * 0.002f * e;
    }
        
    if( GetAsyncKeyState( VK_DOWN ) & 0x8000 )
    {
        pos.x += Sin(m_aCameraYaw) * 0.002f * e;
        pos.z -= Cos(m_aCameraYaw) * 0.002f * e;
    }

    // Ensure that we have stayed within the maze boundaries
    if( pos.x < 0 ) pos.x = 0.1f;
    if( pos.x >= m_Maze.GetWidth() ) pos.x = m_Maze.GetWidth() - 0.1f;
    if( pos.z < 0 ) pos.z = 0.1f;
    if( pos.z >= m_Maze.GetHeight() ) pos.z = m_Maze.GetHeight() - 0.1f;
            
    m_vCameraPos = pos;    
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::DoAutopilot( FLOAT fElapsed )
{
    // While there is still time to use up...
    while( fElapsed )
    {
        // See if we need to turn
        if( m_aAutopilotTargetAngle != m_aCameraYaw )
        {
            SHORT diff = SHORT((m_aAutopilotTargetAngle - m_aCameraYaw)&TRIG_ANGLE_MASK);
            FLOAT fNeeded = abs(diff)/40.0f;
            if( fNeeded/1000.0f <= fElapsed )
            {
                m_aCameraYaw = m_aAutopilotTargetAngle;
                fElapsed -= fNeeded/1000.0f;
            }
            else
            {
                if( diff < 0 )
                    m_aCameraYaw -= (DWORD) ((fElapsed*1000.0f) * 40.0f);
                else
                    m_aCameraYaw += (DWORD) ((fElapsed*1000.0f) * 40.0f);
                fElapsed = 0;
            }
        }
        else
        {
            // Ensure vAutopilotTarget is inside the maze boundry
            if( m_vAutopilotTarget.x < 0                  || 
                m_vAutopilotTarget.x >= m_Maze.GetWidth() ||
                m_vAutopilotTarget.z < 0                  ||
                m_vAutopilotTarget.z >= m_Maze.GetHeight() )
            {
                ZeroMemory( m_AutopilotVisited, sizeof(m_AutopilotVisited) );
                m_AutopilotStack.Empty();
                PickAutopilotTarget();
                return;
            }
            
            // Facing right way, so now compute distance to target
            D3DXVECTOR3 diff = m_vAutopilotTarget - m_vCameraPos;

            float fRange = float(sqrt((diff.x*diff.x)+(diff.z*diff.z)));

            // Are we there yet?
            if( fRange > 0 )
            {
                // No, so compute how long we'd need
                FLOAT fNeeded = fRange / 0.002f;

                //Ensure we never leave the boundary of the Maze.
                D3DXVECTOR3 pos = m_vCameraPos;
                
                // Do we have enough time this frame?
                if( fNeeded/1000.0f <= fElapsed )
                {

                    // Yes, so just snap us there
                    pos.x = m_vAutopilotTarget.x;
                    pos.z = m_vAutopilotTarget.z;
                    
                    fElapsed -= fNeeded/1000.0f;
                }
                else
                {
                    // No, so move us as far as we can
                    pos.x -= Sin(m_aCameraYaw) * 0.002f * fElapsed*1000.0f;
                    pos.z += Cos(m_aCameraYaw) * 0.002f * fElapsed*1000.0f;
                    
                    fElapsed = 0;
                }

                // Ensure that we have stayed within the maze boundaries
                if( pos.x < 0 ) pos.x = 0.1f;
                if( pos.x >= m_Maze.GetWidth() ) pos.x = m_Maze.GetWidth() - 0.1f;
                if( pos.z < 0 ) pos.z = 0.1f;
                if( pos.z >= m_Maze.GetHeight() ) pos.z = m_Maze.GetHeight() - 0.1f;
            
                // Assign our new values back to our globals.
                m_vCameraPos = pos;

            }
            else
            {
                // Reached target, so pick another
                PickAutopilotTarget();
            }
        }
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::EngageAutopilot( BOOL bEngage )
{
    m_bEngageAutopilot = bEngage;

    if( !IsMazeReady() ) 
        return;

    BOOL bPrevious = m_bAutopilot;
    m_bAutopilot = bEngage;

    // If we weren't on autopilot before and are are autopilot now then need to init autopilot
    if( m_bAutopilot && !bPrevious )
    {
        // First of all, snap us to the centre of the current cell
        int cellx = int(m_vCameraPos.x);
        int cellz = int(m_vCameraPos.z);
        m_vCameraPos.x = cellx + 0.5f;
        m_vCameraPos.z = cellz + 0.5f;

        // Ensure we're within the maze boundaries
        if( cellx < 0 ) m_vCameraPos.x = 0.5f;
        if( cellx >= int(m_Maze.GetWidth()) ) m_vCameraPos.x = m_Maze.GetWidth() - 0.5f;
        if( cellz < 0 ) m_vCameraPos.z = 0.5f;
        if( cellz >= int(m_Maze.GetHeight()) ) m_vCameraPos.z = m_Maze.GetHeight() - 0.5f;

        // Clear the visited array and stack
        ZeroMemory( m_AutopilotVisited, sizeof(m_AutopilotVisited) );
        m_AutopilotStack.Empty();

        // Pick the next target cell
        PickAutopilotTarget();
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::PickAutopilotTarget()
{
    // Get current cell and mark as visited
    DWORD currentx = DWORD(m_vCameraPos.x);
    DWORD currentz = DWORD(m_vCameraPos.z);
    m_AutopilotVisited[currentz][currentx] = 1;

    // Figure out which directions are allowed. We're allowed to go in any direction
    // where there isn't a wall in the way and that takes us to a cell we've visited before.
    BYTE cell = m_Maze.GetCell(currentx,currentz);
    ANGLE alloweddirs[5];
    DWORD dwAllowed = 0;

    if( !(cell & MAZE_WALL_NORTH) && !m_AutopilotVisited[currentz-1][currentx] )
        alloweddirs[dwAllowed++] = NORTH_ANGLE;
    if( !(cell & MAZE_WALL_WEST) && !m_AutopilotVisited[currentz][currentx-1] )
        alloweddirs[dwAllowed++] = WEST_ANGLE;
    if( !(cell & MAZE_WALL_EAST) && !m_AutopilotVisited[currentz][currentx+1] )
        alloweddirs[dwAllowed++] = EAST_ANGLE;
    if( !(cell & MAZE_WALL_SOUTH) && !m_AutopilotVisited[currentz+1][currentx] )
        alloweddirs[dwAllowed++] = SOUTH_ANGLE;
/*
    printf( "Walls: ") );
    if( (cell & MAZE_WALL_NORTH) )
        printf( "N ") );
    if( (cell & MAZE_WALL_WEST) )
        printf( "W ") );
    if( (cell & MAZE_WALL_EAST) )
        printf( "E ") );
    if( (cell & MAZE_WALL_SOUTH) )
        printf( "S ") );
    printf( "\n") );
*/

    // Is there anywhere to go?
    if( dwAllowed == 0 )
    {
        // Nope. Can we backtrack?
        if( m_AutopilotStack.GetCount() > 0 )
        {
            // Yes, so pop cell off the stack
            AutopilotCell   cell(m_AutopilotStack.Pop());
            m_vAutopilotTarget.x = float(cell.x) + 0.5f;
            m_vAutopilotTarget.z = float(cell.y) + 0.5f;

            if( cell.x < currentx )
                m_aAutopilotTargetAngle = WEST_ANGLE;
            else if( cell.x > currentx )
                m_aAutopilotTargetAngle = EAST_ANGLE;
            else if( cell.y > currentz )
                m_aAutopilotTargetAngle = SOUTH_ANGLE;
            else
                m_aAutopilotTargetAngle = NORTH_ANGLE;
        }
        else
        {
            // No, so we have explored entire maze and must start again
            ZeroMemory( m_AutopilotVisited, sizeof(m_AutopilotVisited) );
            m_AutopilotStack.Empty();
            PickAutopilotTarget();
        }
    }
    else
    {
        // See if we can continue in current direction
        BOOL bPossible = FALSE;
        for( DWORD i = 0; i < dwAllowed; i++ )
        {
            if( alloweddirs[i] == m_aCameraYaw )
            {
                bPossible = TRUE;
                break;
            }
        }

        // If it's allowed to go forward, then have 1 in 2 chance of doing that anyway, otherwise pick randomly from
        // available alternatives
        if( bPossible && (rand() & 0x1000) )
            m_aAutopilotTargetAngle = m_aCameraYaw;
        else
            m_aAutopilotTargetAngle = alloweddirs[ (rand() % (dwAllowed<<3) ) >>3 ];

        m_vAutopilotTarget.z = float(currentz) + 0.5f;
        m_vAutopilotTarget.x = float(currentx) + 0.5f;

        switch( m_aAutopilotTargetAngle )
        {
            case SOUTH_ANGLE:
                m_vAutopilotTarget.z += 1.0f;
                break;

            case WEST_ANGLE:
                m_vAutopilotTarget.x -= 1.0f;
                break;

            case EAST_ANGLE:
                m_vAutopilotTarget.x += 1.0f;
                break;

            case NORTH_ANGLE:
                m_vAutopilotTarget.z -= 1.0f;
                break;
        }

        // Push current cell onto stack
        m_AutopilotStack.Push( AutopilotCell(BYTE(currentx),BYTE(currentz)) );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeClient::OnPacket( DWORD dwFrom, void* dwData, DWORD dwSize )
{
    HRESULT hr  = DPN_OK;
    
    BOOL    fFoundSize = FALSE;
    DWORD   dwReqSize = 0;
    DWORD   dwSRand = 0;
    
    ServerPacket* pPacket = (ServerPacket*)dwData;
    switch( pPacket->wType )
    {
        case PACKETTYPE_SERVER_CONFIG:
        {
            if( dwSize != sizeof(ServerConfigPacket) )
            {
                m_pMazeApp->SetDisconnectNow( TRUE );
                m_pMazeApp->SetOutOfDateClient( TRUE );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Disconnected because MazeClient is out of date.") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Please get updated version") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("from http://msdn.microsoft.com/directx/") );
                break;
            }

            m_NetConfig = ((ServerConfigPacket*)pPacket)->Config;

            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Got MazeServer config settings") );
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Maze Size=(%d,%d) ReliableRate=%d%%"), 
                                       m_NetConfig.dwMazeWidth, m_NetConfig.dwMazeHeight, 
                                       DWORD(m_NetConfig.ubReliableRate) );
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("UpdateRate=%dms Timeout=%d"), 
                                       m_NetConfig.wUpdateRate, m_NetConfig.wTimeout );
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("ThreadWait=%dms"), 
                                       m_NetConfig.dwThreadWait );
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("ClientPackSize=%d ServerPackSize=%d (bytes)"), 
                                       m_NetConfig.wClientPackSizeArray[m_NetConfig.ubClientPackIndex],
                                       m_NetConfig.wServerPackSizeArray[m_NetConfig.ubServerPackIndex]);
            
            // See if we have gotten our fist config. If not, send version info.
            if(! GotFirstConfig()) //If first time in maze.
            {
                // The client expects the server to send a ServerConfigPacket packet first, 
                // then the client sends a ClientVersionPacket, and then the server sends a 
                // ServerAckVersionPacket packet and the game begins
                ClientVersionPacket packet( MAZE_CLIENT_VERSION );
                SendPacket( &packet, sizeof(packet), TRUE, 0 );

                SetFirstConfig( TRUE );
            }

            break;
        }

        case PACKETTYPE_SERVER_ACKVERSION:
        {
            if( dwSize != sizeof(ServerAckVersionPacket) )
            {
                m_pMazeApp->SetDisconnectNow( TRUE );
                m_pMazeApp->SetOutOfDateClient( TRUE );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Disconnected because MazeClient is out of date.") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Please get updated version") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("from http://msdn.microsoft.com/directx/") );
                break;
            }

            ServerAckVersionPacket* pAckVersionPacket = (ServerAckVersionPacket*)pPacket;

            // Record the dpnid that the server uses for to talk to us. 
            // This is just done so that we can record this number in the 
            // logs to help match server side logs with client side logs.
            m_dwLocalClientID = pAckVersionPacket->dwClientID;

            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Server assigned ID: 0x%0.8x"), m_dwLocalClientID );
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Server accepted client version") );


            hr = m_Maze.Init( m_NetConfig.dwMazeWidth, 
                              m_NetConfig.dwMazeHeight, DEFAULT_SEED );
            if( FAILED(hr) )
                DXTRACE_ERR( TEXT("Init"), hr );

            //Seed the random number generator.
            dwSRand = (DWORD) (DXUtil_Timer( TIMER_GETABSOLUTETIME ) * (DWORD)GetCurrentThreadId() );
            srand( dwSRand );

            // Set random start location
            m_vCameraPos = D3DXVECTOR3( rand() % m_Maze.GetWidth() + 0.5f, 0.5, 
                                        rand() % m_Maze.GetHeight() + 0.5f );

            SetMazeReady( TRUE );
            EngageAutopilot( TRUE );
            break;
        }

        case PACKETTYPE_SERVER_ACKPOS:
            
            //Make sure we at least have a ServerAckPacket.
            if( dwSize < sizeof(ServerAckPacket) )
            {
                fFoundSize = FALSE;
            }
            else 
            {   
                //Size of our required packet. Does not include custom pack data.
                dwReqSize = (sizeof(ServerAckPacket) + 
                             (sizeof(PlayerStatePacket) *
                             ((ServerAckPacket*)pPacket)->wPlayerStatePacketCount));
                
                //Check to see if we have a valid packet size.
                if (dwSize < dwReqSize)
                    fFoundSize = FALSE;
                else if ( !IsValidPackSize( dwSize - dwReqSize ))
                    fFoundSize = FALSE;
                else
                    fFoundSize = TRUE;
            }
            
            //If we did not find a correct packet size. Exit.
            if( !fFoundSize )
            {
                m_pMazeApp->SetDisconnectNow( TRUE );
                m_pMazeApp->SetOutOfDateClient( TRUE );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Disconnected because MazeClient is out of date.") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Please get updated version") );
                m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("from http://msdn.microsoft.com/directx/") );
                break;
            }

            //Found correct packet size. Update clients.
            SetPlayerStats( ((ServerAckPacket*)pPacket)->wPlayerCount,
                            ((ServerAckPacket*)pPacket)->wPlayerStatePacketCount );

            if( ((ServerAckPacket*)pPacket)->wPlayerStatePacketCount )
                HandlePlayerObjectsInAckPacket( (ServerAckPacket*)pPacket );
            break;

        default:
            m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Received unknown %d byte packet from server"), dwSize );
            break;
    };

    
    //If the server has given us a custom wait time, Let's sleep for that amount of time.
    if( m_NetConfig.dwThreadWait > 0 )
    {
        Sleep( m_NetConfig.dwThreadWait );
    }
          
        
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMazeClient::IsValidPackSize( DWORD dwSize )
{
    BOOL fFoundSize = FALSE;
    BYTE ubPackLocation = m_NetConfig.ubServerPackIndex;

    // Look for valid Client pack size in PackArray.
    // If found, return TRUE, otherwise FALSE.
    if(dwSize != m_NetConfig.wServerPackSizeArray[ubPackLocation])
    {
        for( --ubPackLocation; ubPackLocation != m_NetConfig.ubServerPackIndex; ubPackLocation--)
        {
            if(dwSize == m_NetConfig.wServerPackSizeArray[ubPackLocation])
            {
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
void CMazeClient::OnSessionLost( DWORD dwReason )
{
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::SendPacket( ClientPacket* pPacket, DWORD dwSize, 
                              BOOL bGuaranteed, DWORD dwTimeout )
{
    if( m_NetConfig.ubReliableRate > m_NetRandom.Get( 100 ) )
        bGuaranteed = TRUE;

    m_pNet->SendPacket( pPacket, dwSize, bGuaranteed, dwTimeout );
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
DWORD CMazeClient::GetRoundTripLatencyMS()
{
    return m_pNet->GetRoundTripLatencyMS();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
DWORD CMazeClient::GetThroughputBPS()
{
    return m_pNet->GetThroughputBPS();
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::HandlePlayerObjectsInAckPacket( ServerAckPacket* pPacket )
{
    PlayerStatePacket* pClientInfo = (PlayerStatePacket*)(pPacket+1);

    if( !IsMazeReady() ) 
        return;

    // Lock the world database
    LockWorld();

    // Loop though the PlayerObject chunks
    for( DWORD count = pPacket->wPlayerStatePacketCount; count; count--, pClientInfo++ )
    {
        // Get the PlayerObject we think this is
        PlayerObject* pPlayerObject = &m_PlayerObjects[pClientInfo->dwID & PLAYER_OBJECT_SLOT_MASK];

        // Does the ID match the one we have?
        if( pPlayerObject->dwID != pClientInfo->dwID )
        {
            // No, so the PlayerObject we have needs to be deleted (server reused the same slot
            // number, so old PlayerObject must be toast)
            RemovePlayerObjectFromCells( pPlayerObject );

            // Set the ID to the new ID
            pPlayerObject->dwID = pClientInfo->dwID;
            pPlayerObject->wCellX = WORD(pClientInfo->fX);
            pPlayerObject->wCellY = WORD(pClientInfo->fY);

            // Insert into the appropriate cell list
            AddPlayerObjectToCells( pPlayerObject );
        }
        else
        {
            // Yes, compute the new cell coordinates
            DWORD newcellx = DWORD(pClientInfo->fX);
            DWORD newcelly = DWORD(pClientInfo->fY);

            // Are they the same as the ones we already have?
            if( newcellx != pPlayerObject->wCellX || newcelly != pPlayerObject->wCellY )
            {
                // No, so need to remove from old cell and add to new one
                RemovePlayerObjectFromCells( pPlayerObject );
                pPlayerObject->wCellX = WORD(newcellx);
                pPlayerObject->wCellY = WORD(newcelly);
                AddPlayerObjectToCells( pPlayerObject );
            }
        }

        // Update timestamp and position
        pPlayerObject->vPos.x     = pClientInfo->fX;
        pPlayerObject->vPos.y     = 0.5f;
        pPlayerObject->vPos.z     = pClientInfo->fY;
        pPlayerObject->aCameraYaw = pClientInfo->aCameraYaw;
        pPlayerObject->fLastValidTime = DXUtil_Timer( TIMER_GETAPPTIME );
    }

    // Unlock world database
    UnlockWorld();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::AddPlayerObjectToCells( PlayerObject* pPlayerObject )
{
    if( pPlayerObject->wCellX == 0xffff )
        return;

    if( FALSE == IsPlayerObjectInCell( pPlayerObject->wCellX, pPlayerObject->wCellY, pPlayerObject ) )
    {
        PlayerObject** ppCell = &m_pctCells[pPlayerObject->wCellY][pPlayerObject->wCellX];
        pPlayerObject->pNext = *ppCell;
        *ppCell = pPlayerObject;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeClient::RemovePlayerObjectFromCells( PlayerObject* pPlayerObject )
{
    if( pPlayerObject->wCellX == 0xffff )
        return;

    PlayerObject** ppCell   = &m_pctCells[pPlayerObject->wCellY][pPlayerObject->wCellX];
    PlayerObject* pCur      = *ppCell;
    PlayerObject* pPrev     = NULL;
    while( pCur )
    {
        if( pCur == pPlayerObject )
        {
            pCur = pPlayerObject->pNext;

            // Found pPlayerObject, so remove pPlayerObject from the m_pctCells linked list
            if( pPrev )
                pPrev->pNext = pPlayerObject->pNext;
            else
                *ppCell = pPlayerObject->pNext;
            pPlayerObject->pNext = NULL;

            // Update pPlayerObject so that it is marked as removed
            pPlayerObject->wCellX = pPlayerObject->wCellY = 0xffff;

            // Continue searching, and remove any other instances of 
            // pPlayerObject from list (there shouldn't be, however)
        }
        else
        {
            pPrev = pCur;
            pCur  = pCur->pNext;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
PlayerObject* CMazeClient::GetFirstPlayerObjectInCell( DWORD x, DWORD z )
{
    if( !IsMazeReady() ) 
        return NULL;

    FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );

    // Remove any PlayerObjects which are out of date (since they're probably not really in this
    // cell any more, but the server has just stopped telling us about them)
    PlayerObject** ppCell           = &m_pctCells[z][x];
    PlayerObject* pCur              = m_pctCells[z][x];
    PlayerObject* pPrev             = NULL;
    PlayerObject* pPlayerObject     = NULL;
    while( pCur )
    {
        // Too old?
        if( (fCurTime - pCur->fLastValidTime) > 5.0f )
        {
            pPlayerObject = pCur;
            pCur = pCur->pNext;

            // pPlayerObject is too old, so remove pPlayerObject from the m_pctCells linked list
            if( pPrev )
                pPrev->pNext = pPlayerObject->pNext;
            else
                *ppCell = pPlayerObject->pNext;
            pPlayerObject->pNext = NULL;

            // Update pPlayerObject so that it is marked as removed
            pPlayerObject->wCellX = pPlayerObject->wCellY = 0xffff;

            // Continue searching, and remove any other old instances from list 
        }
        else
        {
            pPrev = pCur;
            pCur  = pCur->pNext;
        }
    }

    // Now return first remaining PlayerObject in the cell (if any)
    return m_pctCells[z][x];
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMazeClient::IsPlayerObjectInCell( DWORD wCellX, DWORD wCellY, PlayerObject* pPlayerObject )
{
    PlayerObject* pPlayerObjectTmp = m_pctCells[wCellY][wCellX];
    while( pPlayerObjectTmp )
    {
        if( pPlayerObjectTmp == pPlayerObject )
            return TRUE;

        pPlayerObjectTmp = pPlayerObjectTmp->pNext;
    }

    return FALSE;
}


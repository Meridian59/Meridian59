//----------------------------------------------------------------------------
// File: maze.cpp
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
#include <malloc.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dxerr8.h>
#include "DXUtil.h"
#include "Maze.h"
#include "MazeServer.h"



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMaze::CMaze()
{
    m_dwWidth = m_dwHeight = m_dwSeed = m_dwSize = 0;
    m_pMaze = NULL;
    m_dwMaxView = 15;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMaze::~CMaze()
{
    if( m_pMaze != NULL )
        DXTRACE( TEXT("Warning: Destructing CMaze object without calling Empty()\n") );

    Empty();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMaze::Init( DWORD dwWidth, DWORD dwHeight, DWORD dwSeed )
{
    HRESULT hr;
    CellNode* pCells = NULL;
    CellNode* pCellNode = NULL;
    DWORD dwNumWalls;
    WallNode* pWalls = NULL;
    WallNode* pWallNode = NULL;
    WallNode tempWall;
    DWORD    dwIndex;
    DWORD i;

    m_Random.Reset( dwSeed );

    // Empty out any old data
    Empty();

    // Store parameters and compute number of cells in the maze
    m_dwWidth  = dwWidth;
    m_dwHeight = dwHeight;
    m_dwSeed   = dwSeed;
    m_dwSize   = m_dwWidth * m_dwHeight;

    // Must be non-zero
    if( m_dwSize == 0 )
    {
        hr = E_INVALIDARG;
        DXTRACE_ERR( TEXT("Maze height and width need must be greater than 0"), E_INVALIDARG );
        goto LFail;
    }

    // Validate maze size
    if( m_dwWidth > SERVER_MAX_WIDTH || m_dwHeight > SERVER_MAX_HEIGHT )
    {
        hr = E_INVALIDARG;
        DXTRACE_ERR( TEXT("Maze height and width must be less than 128"), E_INVALIDARG );
        goto LFail;
    }

    if( (m_dwWidth % LOCK_GRID_SIZE) != 0 || (m_dwHeight % LOCK_GRID_SIZE) != 0 )
    {
        hr = E_INVALIDARG;
        DXTRACE_ERR( TEXT("Maze height and width need to be divisable by 16"), E_INVALIDARG );
        goto LFail;
    }

    // Allocate maze, and initially make all walls solid
    m_pMaze = new BYTE[m_dwSize];
    if( m_pMaze == NULL )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR( TEXT("new"), hr );
        goto LFail;
    }
    memset( m_pMaze, MAZE_WALL_ALL, m_dwSize );

    // Okay, now we're going to generate the maze. We use Kruskal's algorithm, which
    // works by walking through the list of walls in a random order, removing a wall
    // if it would connect two previously (path-)disconnected cells. This guarantees
    // a fully connected maze (i.e. you can reach any cell from any other).

    // Allocate and initialize temporary cell list
    pCells = new CellNode[m_dwSize];
    if( pCells == NULL )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR( TEXT("new"), hr );
        goto LFail;
    }

    pCellNode = pCells;
    for( i = 0; i < m_dwSize; i++ )
    {
        pCellNode->pNext = NULL;
        pCellNode->pPartition = pCellNode;
        pCellNode++;
    }

    // Create list of walls
    dwNumWalls = ((m_dwWidth-1)*m_dwHeight)+((m_dwHeight-1)*m_dwWidth);
    pWalls = new WallNode[dwNumWalls];
    if( pWalls == NULL )
    {
        hr = E_OUTOFMEMORY;
        DXTRACE_ERR( TEXT("new"), hr );
        goto LFail;
    }

    pWallNode = pWalls;
    for( i = 1; i < m_dwWidth; i++ )
    {
        for( DWORD j = 0; j < m_dwHeight; j++, pWallNode++ )
        {
            pWallNode->dwX = i;
            pWallNode->dwY = j;
            pWallNode->dwType = MAZE_WALL_WEST;
        }
    }
    for( i = 0; i < m_dwWidth; i++ )
    {
        for( DWORD j = 1; j < m_dwHeight; j++, pWallNode++ )
        {
            pWallNode->dwX = i;
            pWallNode->dwY = j;
            pWallNode->dwType = MAZE_WALL_NORTH;
        }
    }

    // Randomly permute the wall list
    for( i = dwNumWalls-1; i > 0; i-- )
    {
        dwIndex         = m_Random.Get(i);
        tempWall        = pWalls[dwIndex];
        pWalls[dwIndex] = pWalls[i];
        pWalls[i]       = tempWall;
    }

    // Walk through all the walls
    pWallNode = pWalls;
    for( i = 0; i < dwNumWalls; i++, pWallNode++ )
    {
        // Determine the cells either side of the wall
        DWORD dwCellA = pWallNode->dwX + (pWallNode->dwY * m_dwWidth);
        DWORD dwCellB = dwCellA;
        if( pWallNode->dwType == MAZE_WALL_NORTH )
            dwCellB -= m_dwWidth;
        else
            dwCellB--;

        // Are they already connected (partitions equal)?
        CellNode* pCellA = pCells+dwCellA;
        CellNode* pCellB = pCells+dwCellB;
        if( pCellA->pPartition != pCellB->pPartition )
        {
            // Nope, so let's take out that wall. First, connect the partition lists
            while ( pCellA->pNext )
                pCellA = pCellA->pNext;
            pCellB = pCellB->pPartition;
            pCellA->pNext = pCellB;
            while ( pCellB )
            {
                pCellB->pPartition = pCellA->pPartition;
                pCellB = pCellB->pNext;
            }

            // Now remove the walls in our maze array
            if( pWallNode->dwType == MAZE_WALL_NORTH )
            {
                m_pMaze[dwCellA] &= ~MAZE_WALL_NORTH;
                m_pMaze[dwCellB] &= ~MAZE_WALL_SOUTH;
            }
            else
            {
                m_pMaze[dwCellA] &= ~MAZE_WALL_WEST;
                m_pMaze[dwCellB] &= ~MAZE_WALL_EAST;
            }
        }
    }

    // Free temporary wall and cell lists
    delete[] pWalls;
    delete[] pCells;

    return S_OK;

LFail:
    SAFE_DELETE_ARRAY( pCells );
    SAFE_DELETE_ARRAY( pWalls );
    SAFE_DELETE_ARRAY( m_pMaze );
    return hr;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMaze::Empty()
{
    if( m_pMaze != NULL )
        SAFE_DELETE_ARRAY( m_pMaze );

    m_dwWidth = m_dwHeight = m_dwSeed = m_dwSize = 0;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
DWORD CMaze::GetVisibleCells( const D3DXVECTOR2& pos, const D3DXVECTOR2& dir ,
                              float fov, MazeCellRef* plist, DWORD maxlist )
{
    // Check we have a maze, and that we were passed reasonable parameters
    if( m_pMaze == NULL || plist == NULL || maxlist == 0 )
        return 0;

    // Check bounds of given viewpoint, must be inside maze
    if( pos.x < 0.0f || pos.y < 0.0f || 
        pos.x >= float(m_dwWidth) || pos.y >= float(m_dwHeight) )
        return 0;

    // State data for the algorithm
    VisState state;

    // Figure out which cell the viewpoint is in
    state.dwPosX = DWORD(pos.x);
    state.dwPosY = DWORD(pos.y);
    state.vPos   = pos;

    // Compute view boundaries
    float   c = float(cos(fov*0.5f));
    float   s = float(sin(fov*0.5f));
    D3DXVECTOR2 left,right;
    left.x  = (dir.x*c)+(dir.y*s);
    left.y  = (dir.y*c)-(dir.x*s);
    right.x = (dir.x*c)-(dir.y*s);
    right.y = (dir.y*c)+(dir.x*s);

    // Store view direction (for near plane clip)
    state.vDir = dir;

    // Figure out boundary of area we're prepared to look at (view cutoff)
    state.dwMinX = (state.dwPosX > m_dwMaxView) ? state.dwPosX - m_dwMaxView : 0;
    state.dwMaxX = ((state.dwPosX + m_dwMaxView) > m_dwWidth) ? m_dwWidth : state.dwPosX + m_dwMaxView;
    state.dwMinY = (state.dwPosY > m_dwMaxView) ? state.dwPosY - m_dwMaxView : 0;
    state.dwMaxY = ((state.dwPosY + m_dwMaxView) > m_dwHeight) ? m_dwHeight : state.dwPosY + m_dwMaxView;
    state.dwArrayPitch = state.dwMaxX-state.dwMinX+1;

    // Allocate a temporary buffer which we'll use to mark visited cells
    DWORD array_size = state.dwArrayPitch * (state.dwMaxY-state.dwMinY+1);
    state.pArray = (BYTE*)_alloca( array_size );
    ZeroMemory( state.pArray, array_size );

    state.ppVisList = &plist;
    state.dwMaxList = maxlist;
    state.dwListLen = 0;

    // Recurse through cells
    RecurseCheckCellVis( state, state.dwPosX, state.dwPosY, left, right );

    return state.dwListLen;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMaze::RecurseCheckCellVis( VisState& state, DWORD x, DWORD y, 
                                 D3DXVECTOR2 left, D3DXVECTOR2 right )
{
    // Fall out if we've overrun list length
    if( state.dwListLen >= state.dwMaxList )
        return;

    // If cell is outside the maximum view bounds, then it's not visible
    if( x < state.dwMinX || x > state.dwMaxX || 
        y < state.dwMinY || y > state.dwMaxY )
        return;

    // If cell is already marked, then we don't visit it either
    if( state.pArray[x-state.dwMinX+((y-state.dwMinY)*state.dwArrayPitch)] )
        return;

    // Mark cell as visited
    state.pArray[x-state.dwMinX+((y-state.dwMinY)*state.dwArrayPitch)] = 1;

    // Compute visibility flags
    D3DXVECTOR2 offset;
    offset.x = float(x)-state.vPos.x;
    offset.y = float(y)-state.vPos.y;
    BYTE   flags[4];
    flags[0] = ComputeVisFlags( state.vDir, left, right, offset );
    offset.x += 1.0f;
    flags[1] = ComputeVisFlags( state.vDir, left, right, offset );
    offset.y += 1.0f;
    flags[2] = ComputeVisFlags( state.vDir, left, right, offset );
    offset.x -= 1.0f;
    flags[3] = ComputeVisFlags( state.vDir, left, right, offset );
    offset.y -= 1.0f;

    // If there is an edge which clips all points, then the cell isn't in frustrum
    if( flags[0]&flags[1]&flags[2]&flags[3] )
        return;

    // Cell is visible, so add it to list
    (*state.ppVisList)->x = x;
    (*state.ppVisList)->y = y;
    (*state.ppVisList)++;
    state.dwListLen++;

    // Recurse into adjoining cells. Can move into an adjacent cell only if 
    // there is a 'portal' (i.e. hole in the wall) that is not clipped and 
    // that lies on the correct side of the viewport.
    BYTE   cell = GetCell(x,y);
    D3DXVECTOR2 se = offset + D3DXVECTOR2(1,1);

    if( !(cell & MAZE_WALL_NORTH) && offset.y < 0 && !(flags[0]&flags[1]) )
        RecurseCheckCellVis( state, x, y-1, left, right );

    if( !(cell & MAZE_WALL_SOUTH) && se.y > 0 && !(flags[2]&flags[3]) )
        RecurseCheckCellVis( state, x, y+1, left, right );

    if( !(cell & MAZE_WALL_WEST) && offset.x < 0 && !(flags[3]&flags[0]) )
        RecurseCheckCellVis( state, x-1, y, left, right );

    if( !(cell & MAZE_WALL_EAST) && se.x > 0 && !(flags[1]&flags[2]) )
        RecurseCheckCellVis( state, x+1, y, left, right );

    return;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BYTE CMaze::ComputeVisFlags( const D3DXVECTOR2& dir, const D3DXVECTOR2& left, 
                             const D3DXVECTOR2& right, const D3DXVECTOR2& offset )
{
    BYTE flag = (D3DXVec2Dot(&offset, &dir) >= 0) ? 0 : 1;

    if( D3DXVec2CCW(&offset,&left) > 0 )
        flag |= 2;

    if( D3DXVec2CCW(&offset,&right) < 0 )
        flag |= 4;

    return flag;
}

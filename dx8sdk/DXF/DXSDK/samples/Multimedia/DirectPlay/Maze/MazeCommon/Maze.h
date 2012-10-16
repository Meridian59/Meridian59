//----------------------------------------------------------------------------
// File: maze.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MAZE_H
#define _MAZE_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: Client IDs are 32-bit values that refer to a particular Client. They are 
//       broken up into two bitfields, one of which can be used into an index 
//       of a list of Client 'slots', the other bitfield is a "uniqueness" value 
//       that is incremented each time a new Client is created. Hence, although
//       the same slot may be reused by different Clients are different times, 
//       it's possible to distinguish between the two by comparing uniqueness 
//       values (you can just compare the whole 32-bit id).
//-----------------------------------------------------------------------------
typedef DWORD   ClientID;

#define PLAYER_OBJECT_SLOT_BITS 13
#define MAX_PLAYER_OBJECTS      (1<<PLAYER_OBJECT_SLOT_BITS)
#define PLAYER_OBJECT_SLOT_MASK (MAX_PLAYER_OBJECTS-1)



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
#include <assert.h>
#include "Random.h"

const   BYTE   MAZE_WALL_NORTH = (1<<0);
const   BYTE   MAZE_WALL_EAST = (1<<1);
const   BYTE   MAZE_WALL_SOUTH = (1<<2);
const   BYTE   MAZE_WALL_WEST = (1<<3);
const   BYTE   MAZE_WALL_ALL = MAZE_WALL_NORTH|MAZE_WALL_EAST|
                                MAZE_WALL_SOUTH|MAZE_WALL_WEST;




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct  MazeCellRef
{
    DWORD   x,y;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class   CMaze
{
public:
    CMaze();
    ~CMaze();

    HRESULT Init( DWORD width , DWORD height , DWORD seed );
    void    Empty();

    DWORD   GetWidth()  const { return m_dwWidth; };
    DWORD   GetHeight() const { return m_dwHeight; };
    DWORD   GetSize()   const { return m_dwWidth*m_dwHeight; };
    BYTE   GetCell( DWORD x , DWORD y )    const { assert(m_pMaze!=NULL); return *(m_pMaze+x+(y*m_dwWidth)); };
    BOOL    CanGoNorth( DWORD x , DWORD y ) const { return !(GetCell(x,y)&MAZE_WALL_NORTH); };
    BOOL    CanGoEast( DWORD x , DWORD y )  const { return !(GetCell(x,y)&MAZE_WALL_EAST); };
    BOOL    CanGoSouth( DWORD x , DWORD y ) const { return !(GetCell(x,y)&MAZE_WALL_SOUTH); };
    BOOL    CanGoWest( DWORD x , DWORD y )  const { return !(GetCell(x,y)&MAZE_WALL_WEST); };

    // Get list of visible cells from the given position. Fills out the list pointed to
    // be plist, and stops if it blows maxlist. Returns number of visible cells
    DWORD   GetVisibleCells( const D3DXVECTOR2& pos , const D3DXVECTOR2& dir ,
                             float fov , MazeCellRef* plist , DWORD maxlist );

    // Get/set max view distance (used by GetVisibleCells)
    DWORD   GetMaxView() const { return m_dwMaxView; };
    void    SetMaxView() { m_dwMaxView = m_dwMaxView; };

    BYTE*  m_pMaze;

protected:
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    DWORD   m_dwSize;
    DWORD   m_dwSeed;
    DWORD   m_dwMaxView;
    CRandom m_Random;

    // Local types for the maze generation algorithm
    struct  CellNode
    {
        CellNode*   pPartition;
        CellNode*   pNext;
    };
    struct  WallNode
    {
        DWORD   dwX,dwY;
        DWORD   dwType;
    };

    // Local type for visibilty alg state
    struct  VisState
    {
        DWORD           dwPosX,dwPosY;      // Cell containing view position
        D3DXVECTOR2         vPos;               // View position
        D3DXVECTOR2         vDir;               // View direction
        BYTE*          pArray;             // Array in which cell visits are marked
        DWORD           dwMinX,dwMaxX;      // Extents to consider (also array bounds)
        DWORD           dwMinY,dwMaxY;
        DWORD           dwArrayPitch;       // 'Pitch' of array (width)
        MazeCellRef**   ppVisList;          // Pointer to vis list pointer
        DWORD           dwMaxList;          // Maximum length of vis list
        DWORD           dwListLen;          // Current length of vis list
    };

    void    RecurseCheckCellVis( VisState& state , DWORD x , DWORD y , D3DXVECTOR2 left , D3DXVECTOR2 right );
    BYTE   ComputeVisFlags( const D3DXVECTOR2& dir , const D3DXVECTOR2& left , const D3DXVECTOR2& right , const D3DXVECTOR2& offset );

private:
    CMaze( const CMaze& );
    void operator=( const CMaze& );
};




#endif

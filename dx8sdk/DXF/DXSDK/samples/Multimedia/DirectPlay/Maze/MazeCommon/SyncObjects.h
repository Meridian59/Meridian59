//----------------------------------------------------------------------------
// File: syncobjects.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _SYNC_OBJECTS_H
#define _SYNC_OBJECTS_H


#include <assert.h>


//-----------------------------------------------------------------------------
// Name: 
// Desc: Simple wrapper for critical section
//-----------------------------------------------------------------------------
class CCriticalSection
{
public:
    CCriticalSection( DWORD spincount = 2000 )
    {
        InitializeCriticalSection( &m_CritSec );
    };

    ~CCriticalSection()
    {
        DeleteCriticalSection( &m_CritSec );
    };

    void    Enter()
    {
        EnterCriticalSection( &m_CritSec );
    };

    void    Leave()
    {
        LeaveCriticalSection( &m_CritSec );
    };

private:
    CRITICAL_SECTION    m_CritSec;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Array of critical section objects. The methods allow locking single 
//       elements, rectangular regions, or a combination. Using these methods 
//       ensures the cells are locked/unlocked in a consistent order
//       which prevents deadlocks.
//-----------------------------------------------------------------------------
template< DWORD width , DWORD height > class    CLockArray
{
public:
    #define CS_RESOLUTION 4

    // Lock/Unlock a single cell
    void    LockCell( DWORD x , DWORD y )
    {
        x /= CS_RESOLUTION;
        y /= CS_RESOLUTION;

        assert( x<width && y<height );

        m_Grid[y][x].Enter();
    };

    void    UnlockCell( DWORD x , DWORD y )
    {
        x /= CS_RESOLUTION;
        y /= CS_RESOLUTION;

        assert( x<width && y<height );

        m_Grid[y][x].Leave();
    };

    // Lock/Unlock a rectangular range of cells
    void    LockRange( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 )
    {
        x1 /= CS_RESOLUTION;
        y1 /= CS_RESOLUTION;
        x2 /= CS_RESOLUTION;
        y2 /= CS_RESOLUTION;

        if ( x1 > x2 ) { DWORD t = x1; x1 = x2; x2 = t; }; // x1 == min
        if ( y1 > y2 ) { DWORD t = y1; y1 = y2; y2 = t; }; // y1 == min

        assert( x1 <= x2 && y1 <= y2 );
        assert( x1<width && y1<height );
        assert( x2<width && y2<height );

        // Lock from xmin,ymin to xmax,ymax (from xmin,y to xmax,y first) 
        for ( INT y = y1 ; y <= (INT) y2 ; y++ )
            for ( INT x = x1 ; x <= (INT) x2 ; x++ )
                m_Grid[y][x].Enter();
    };

    void    UnlockRange( DWORD x1 , DWORD y1 , DWORD x2 , DWORD y2 )
    {
        x1 /= CS_RESOLUTION;
        y1 /= CS_RESOLUTION;
        x2 /= CS_RESOLUTION;
        y2 /= CS_RESOLUTION;

        if ( x1 > x2 ) { DWORD t = x1; x1 = x2; x2 = t; }; // x1 == min
        if ( y1 > y2 ) { DWORD t = y1; y1 = y2; y2 = t; }; // y1 == min

        assert( x1 <= x2 && y1 <= y2 );
        assert( x1<width && y1<height );
        assert( x2<width && y2<height );

        // Unlock from xmax,ymax to xmin,ymin 
        for ( INT y = y2 ; y >= (INT) y1 ; y-- )
            for ( INT x = x2 ; x >= (INT) x1 ; x-- )
                m_Grid[y][x].Leave();
    };

    void LockCellPair( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
    {
        x1 /= CS_RESOLUTION;
        y1 /= CS_RESOLUTION;
        x2 /= CS_RESOLUTION;
        y2 /= CS_RESOLUTION;

        assert( x1<width && y1<height );
        assert( x2<width && y2<height );

        if( x1 == x2 && y1 == y2 )
        {
            LockCell( x1, y1 );
            return;
        }

        if( (y1<y2) || ((y1==y2)&&(x1<=x2)) )
        {
            assert( ((y1<y2)) ||             // y1 < y2 case
                    ((y1==y2)&&(x1<=x2)) );  // y1 == y2 case

            // Lock from xmin,ymin to xmax,ymax (from xmin,y to xmax,y first)
            LockCell(x1,y1);
            LockCell(x2,y2);
        }
        else
        {
            assert( ((y1>=y2)) ||            // y1 < y2 case
                    ((y1==y2)&&(x1>x2)) );   // y1 == y2 case

            // Lock from xmin,ymin to xmax,ymax (from xmin,y to xmax,y first) 
            LockCell(x2,y2);
            LockCell(x1,y1);
        }
    }


    void UnlockCellPair( DWORD x1, DWORD y1, DWORD x2, DWORD y2 )
    {
        x1 /= CS_RESOLUTION;
        y1 /= CS_RESOLUTION;
        x2 /= CS_RESOLUTION;
        y2 /= CS_RESOLUTION;

        assert( x1<width && y1<height );
        assert( x2<width && y2<height );

        if( x1 == x2 && y1 == y2 )
        {
            UnlockCell( x1, y1 );
            return;
        }

        if( (y1<y2) || ((y1==y2)&&(x1<=x2)) )
        {
            assert( ((y1<y2)) ||             // y1 < y2 case
                    ((y1==y2)&&(x1<=x2)) );  // y1 == y2 case

            // Unlock from xmax,ymax to xmin,ymin (from xmax,y to xmin,y first)  
            UnlockCell(x2,y2);
            UnlockCell(x1,y1);
        }
        else
        {
            assert( ((y1>=y2)) ||            // y1 < y2 case
                    ((y1==y2)&&(x1>x2)) );   // y1 == y2 case

            // Unlock from xmax,ymax to xmin,ymin (from xmax,y to xmin,y first)  
            UnlockCell(x1,y1);
            UnlockCell(x2,y2);
        }
    }


private:
    CCriticalSection    m_Grid[height][width];
};




#endif

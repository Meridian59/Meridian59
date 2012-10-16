//----------------------------------------------------------------------------
// File: random.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _RANDOM_H
#define _RANDOM_H





//-----------------------------------------------------------------------------
// Name: 
// Desc: Random number generator class - a simple linear congruential generator.
// We use this instead of the CRT function because we want to be certain that
// we are using the exact same generator on both server and client side and so
// (a) don't want to be at the mercy of CRT version changes, and (b) may want
// multiple independent generators which we can rely on the sequencing of.
//-----------------------------------------------------------------------------
class   CRandom
{
public:
    // Constructor. The random formula is X(n+1) = (a*X(n) + b) mod m
    // The default values for a,b,m give a maximal period generator that does not
    // overflow with 32-bit interger arithmetic. 
    CRandom( DWORD seed = 31415, DWORD a = 8121, 
             DWORD b = 28411,    DWORD m = 134456 ) :
        m_dwSeed(seed),m_dwA(a),m_dwB(b),m_dwM(m) {};

    // Grab a random DWORD between 0 and m
    DWORD Get()
    {
        m_dwSeed = ((m_dwA*m_dwSeed)+m_dwB) % m_dwM;
        return m_dwSeed;
    };

    // Grab a random DWORD in the range [0,n-1]
    DWORD Get( DWORD n )
    {
        return (Get() % n);
    };

    // Grab a random float in the range [0,1]
    float GetFloat()
    {
        return Get() / float(m_dwM-1);
    };

    // Reset the seed
    void Reset( DWORD seed = 31415 )
    {
        m_dwSeed = seed;
    };

protected:
    DWORD m_dwA, m_dwB, m_dwM;
    DWORD m_dwSeed;
};




#endif

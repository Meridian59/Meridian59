//----------------------------------------------------------------------------
// File: simplestack.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _SIMPLE_STACK_H
#define _SIMPLE_STACK_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
template< class Item , DWORD size > class SimpleStack
{
public:
    SimpleStack()
    {
        m_dwCount = 0;
    };

    void    Push( const Item& item )
    {
        m_Stack[m_dwCount++] = item;
    };

    Item    Pop()
    {
        return m_Stack[--m_dwCount];
    };

    DWORD   GetCount() const
    {
        return m_dwCount;
    };

    void    Empty()
    {
        m_dwCount = 0;
    };

private:
    Item    m_Stack[size];
    DWORD   m_dwCount;
};




#endif

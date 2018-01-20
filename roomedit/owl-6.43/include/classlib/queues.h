//----------------------------------------------------------------------------
// Borland BIDS Container Library Compatibility header
// Copyright (c) 1998, Yura Bidus, All Rights Reserved
//
/// \file
/// Version 1.4 - Timothy Byrd
///     Implemented barebones TQueueAsDoubleList as a std::queue.
///     To implement other forms of Queues, see the code below
///     and the code in arrays.h and deques.h
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined( CLASSLIB_QUEUES_H )
#define CLASSLIB_QUEUES_H

#pragma message ("Included obsolete file: <classlib/queues.h> use STL instead")

#include <classlib/defs.h>

// if use <owl/template.h>
#if defined(BI_NO_EMULATESTL)
#  include <owl/template.h>
  // include compatibility classes
#  include <classlib/compat.h>
#else

#include <owl/shddel.h>

#include <list>

#include <queue>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

template<class T> class TQueueAsDoubleList : public std::queue<T, std::list<T> >
{
public:
    const T& Peek() const
    {
        return front();
    }

    T Get()
    {
        T t = front();
        pop();
        return t;
    }

    void Put( const T& t )
    {
        push( t );
    }

    int GetItemsInContainer() const
    {
        return size();
    }

    bool IsEmpty() const
    {
        return empty();
    }

    void Flush()
    {
        while (!empty()) {
            pop();
        }
    }
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

#endif  //BI_NO_EMULATESTL


#endif  // CLASSLIB_QUEUES_H

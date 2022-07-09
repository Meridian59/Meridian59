//----------------------------------------------------------------------------
// Borland BIDS Container Library Compatibility header
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Version 1.3 - Timothy Byrd
///     Implemented barebones TDequeAsVector, TDequeAsVectorIterator,
///     TIDequeAsVector and TIDequeAsVectorIterator using std::deque.
///     To implement other forms of Deques, see the code below
///     and the code in arrays.h and queues.h
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined( CLASSLIB_DEQUES_H )
#define CLASSLIB_DEQUES_H

#pragma message ("Included obsolete file: <classlib/deques.h>  use STL instead")

#include <classlib/defs.h>

// if use <owl/template.h>
#if defined(BI_NO_EMULATESTL)
#  include <owl/template.h>

  // include compatibility classes
#  include <classlib/compat.h>
#else

#include "owl/shddel.h"

#include <deque>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


template<class T> class TDequeAsVector : public std::deque<T> {
public:
    TDequeAsVector(unsigned int init_size = -1)
    {
    }

    void PutLeft(const T& t) {
//Show("In TDequeAsVector Add");
        push_front(t);
    }

    void PutRight(const T& t) {
//Show("In TDequeAsVector Add");
        push_back(t);
    }

    T GetLeft() {
        T rVal = front();
        pop_front();
        return rVal;
    }

    T GetRight() {
        T rVal = back();
        pop_back();
        return rVal;
    }

    const T& PeekLeft() const {
        return front();
    }

    const T& PeekRight() const {
        return right();
    }

    unsigned int GetItemsInContainer() const
    {
        return size();
    }

    bool IsEmpty() const
    {
        return empty();
    }
};


template<class T> class TDequeAsVectorIterator {
public:
    TDequeAsVectorIterator(const TDequeAsVector<T>& a)
        : d_a(a), d_it(a.begin())
    { };

    operator bool() const { return notdone(); }

    const T& Current() const {
        return *d_it;
    }

    const T& operator ++(int) {
        return *d_it++;
    }

    const T& operator ++() {
        return *++d_it;
    }

    void Restart() { d_it = d_a.begin(); };

protected:
    bool notdone() const { return d_it != d_a.end(); }
    const TDequeAsVector<T>& d_a;
    TDequeAsVector<T>::const_iterator d_it;
};


template<class T> class TIDequeAsVector : public TDequeAsVector<T*>, public TShouldDelete {
public:
    TIDequeAsVector(unsigned int  init_size)
        : TDequeAsVector<T*>(init_size)
    { };

    ~TIDequeAsVector() {
        Flush();
    }

    void Flush( DeleteType dt = DefDelete )
    {
        if (DelObj(dt)) {
            for (int i = size() - 1; i >= 0; --i) {
                delete at(i);
                at(i) = 0;
            }
        }
        clear();
    }
};


template<class T> class TIDequeAsVectorIterator {
public:
    TIDequeAsVectorIterator(const TIDequeAsVector<T>& a)
        : d_a(&a), d_it(a.begin())
    { };

    operator bool() const { return notdone(); }

    T* Current() const {
        return const_cast<T*>(*d_it);
    }

    T* operator ++(int) {
        return const_cast<T*>(*d_it++);
    }

    T* operator ++() {
        return const_cast<T*>(*++d_it);
    }

    void Restart() { d_it = d_a.begin(); };

protected:
    bool notdone() const { return d_it != d_a.end(); }
    const TIDequeAsVector<T>* d_a;
    TIDequeAsVector<T>::const_iterator d_it;
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace


#endif  // BI_NO_EMULATESTL

#endif  // CLASSLIB_DEQUES_H


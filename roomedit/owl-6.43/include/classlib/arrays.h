//----------------------------------------------------------------------------
// Borland BIDS Container Library Compatibility header
// Copyright (c) 1998, Yura Bidus, All Rights Reserved
//
/// Version 1.5 - Timothy Byrd
///     Implemented barebones TArrayAsVector, TSArrayAsVector,
///     TIArrayAsVector and TISArrayAsVector along with iterators.
///     To implement other forms of Arrays, see the code below
///     and the code in deques.h and queues.h
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined( CLASSLIB_ARRAYS_H )
#define CLASSLIB_ARRAYS_H

#pragma message ("Included obsolete file: <classlib/arrays.h> use STL or <owl/template.h> instead")

#include <classlib/defs.h>

// if use <owl/template.h>
#if defined(BI_NO_EMULATESTL)

#  include <owl/template.h>
  // include compatibility classes
#  include <classlib/compat.h>

#else

#define CLASSLIB_COMPAT_H

#include <owl/shddel.h>
#include <vector>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup utility
/// @{

template<class T> class TArrayAsVector : public std::vector<T> {
public:
    typedef void (*IterFunc)(T&, void*);
    typedef bool (*CondFunc)(const T&, void*);

    explicit TArrayAsVector(unsigned int init_size = 0)
        : d_lower(0), d_delta(0)
    {
        if (init_size > 0) {
            this->reserve(init_size);
        }
    }

    TArrayAsVector(unsigned int init_size, int lower_bound, unsigned int delta = 0)
        : d_lower(lower_bound), d_delta(delta)
    {
        if (init_size > 0) {
            this->reserve(init_size);
        }
    }

    bool Add(const T& t) {
        if ( d_delta > 0 && this->size() >= this->capacity()) {
            reserve(this->size() + d_delta);
        }
        push_back(t);
        return true;
    }

    bool AddAt(const T& t, uint index) {
        index = ZeroBase(index);
        PRECONDITION(index >= 0 && index <= this->size());
        if(index == this->size()){
            return Add(t);
        }
        if ( d_delta > 0 && this->size() >= this->capacity()) {
            reserve(this->size() + d_delta);
        }
        insert(this->begin()+index, t);
        return true;
    }

    T& operator []( int loc )
    {
        Grow( loc );
        return at(ZeroBase(loc));
    }

    const T& operator []( int loc ) const
    {
        PRECONDITION( loc >= d_lower && ZeroBase(loc) < this->size() );
        return at(ZeroBase(loc));
    }

    unsigned int ZeroBase( int loc ) const
    {
        return loc - d_lower;
    }

    int BoundBase( int loc ) const
    {
        return loc == -1 || loc == UINT_MAX ? INT_MAX : loc + d_lower;
    }

    int LowerBound() const
    {
        return d_lower;
    }

    int UpperBound() const
    {
        return d_lower + this->size() - 1;
    }

    bool IsEmpty() const
    {
        return this->empty();
    }

    unsigned int GetItemsInContainer() const
    {
        return this->size();
    }

    unsigned int ArraySize() const
    {
        return this->capacity();
    }
    
    bool HasMember(const T& t) const
    {
        return _Find(t) != -1;
    }

    int Find( const T& t ) const
    {
        return BoundBase(_Find(t));
    }

    void Flush()
    {
        //clear();
        erase(this->begin(), this->end());
    }

    int Detach(const T& t)
    {
        return Detach(Find(t));
    }

    int Detach( int loc )
    {           
        loc = ZeroBase(loc);
        if (loc >= this->size() || loc < 0)
            return 0;
        erase(this->begin()+loc);
        return 1;
    }

    int Destroy(const T& t)
    {
        return Detach(t);
    }

    int Destroy(int loc)
    {           
        return Detach(loc);
    }

    void ForEach(IterFunc iter, void* args)
    {
        for (int i = 0; i < this->size(); ++i)
            (iter)(this->at(i), args);
    }

    T* FirstThat(CondFunc cond, void* args) const
    {
        for (int i = 0; i < this->size(); ++i)
        {
            if ((cond)(this->at(i), args))
                return const_cast<T*>(& this->at(i));
        }
        return 0;
    }

    T* LastThat(CondFunc cond, void* args) const
    {
        for (int i = this->size() - 1; i >= 0; --i)
        {
            if ((cond)(this->at(i), args))
                return const_cast<T*>(& this->at(i));
        }
        return 0;
    }

    void RemoveEntry( int loc )//SL 29-4-2009
    {
        Detach( loc );//detach will 'ZeroBase' loc
    }


    bool Reallocate( unsigned sz, unsigned offset = 0 )
    {
        if (!d_delta) {
            return false;
        }
        --sz;
        sz = sz + d_delta - (sz % d_delta);
        if (sz > this->capacity()) {
            this->reserve(sz);
        }
        if (offset > 0) {
            insert(this->begin(), offset, T());
        }
        if (this->size() < sz) {
            this->resize(sz);
        }
        return true;
    }

    void Grow( int loc ) //increases the sizes, but only if needed, to ensure that loc is a valid position into the array
    {
      int new_size = ZeroBase(loc);
      //if ( new_size >= (int)size() ) { //bad, count of items stored in the array - it should be:
      if ( new_size >= (int)this->capacity() ) { //if new_size>max number of items, then resize

        PRECONDITION(d_delta>0); //will help while debugging

        //also to calculte the correct positions to increment:
        if (d_delta) {
          new_size = (((new_size-1) / d_delta) +1) * d_delta;
        }
        else {
          ++ new_size; //should we throw an exception? At least the PRECONDITION will help to debug
        }
        this->reserve(new_size); //hgx 2010-06-11
      }
      else if (new_size < 0) {
        PRECONDITION(d_delta>0); //will help while debugging
        insert(this->begin(), -new_size, T());
        d_lower += new_size;
      }
    }
    
    int IsFull() const
    {
        return d_delta == 0 && this->size() >= this->capacity();
    }    
    
protected:
    int _Find(const T& t) const
    {
        int i = this->size() - 1;
        while (i >= 0) {
            if (this->at(i) == t) break;
            --i;
        }
        return i;
    }

    int d_lower;
    unsigned int d_delta;
};


template<class T> class TArrayAsVectorIterator {
public:

    typedef TArrayAsVector<T> abase;
    typedef TArrayAsVectorIterator<T> base;
    
    TArrayAsVectorIterator(const abase& a)
        : d_a(&a), d_cur(0), d_lower(0), d_upper(a.size())
    { };

    TArrayAsVectorIterator(const abase& a, unsigned start, unsigned stop)
        : d_a(&a), d_cur(start), d_lower(start), d_upper(stop)
    {
        PRECONDITION(0 <= d_cur);
        PRECONDITION(d_cur <= d_upper);
        PRECONDITION(d_upper <= d_a->size());
    };

    operator bool() const { return notdone(); }

    const T& Current() const {
        PRECONDITION(d_cur < d_upper);
        return d_a->at(d_cur);
    }

    const T& operator ++(int) {
        d_cur++;
        PRECONDITION(d_cur <= d_upper);
        return d_a->at(d_cur-1);
    }

    const T& operator ++() {
        ++d_cur;
        PRECONDITION(d_cur < d_upper);
        return d_a->at(d_cur);
    }

#if 0
    base& operator =(const base& src) {
        d_cur++;
        PRECONDITION(d_cur <= d_upper);
        return *this;
    }
#endif

    void Restart() { Restart(d_lower, d_upper); };

    void Restart(unsigned start, unsigned stop) {
        PRECONDITION(0 <= d_cur);
        PRECONDITION(d_cur <= d_upper);
        PRECONDITION(d_upper <= d_a->size());
        d_cur = d_lower = start;
        d_upper = stop;
    }

protected:
    bool notdone() const { return d_cur < d_upper /* && d_cur < Vect.Size() */; }
    const abase* d_a;
    unsigned d_cur;
    unsigned d_upper;
    unsigned d_lower;
};


template<class T> class TSArrayAsVector : public TArrayAsVector<T> {
public:
    explicit TSArrayAsVector(unsigned int init_size = 0)
        : TArrayAsVector<T>(init_size)
    { };

    TSArrayAsVector(unsigned int init_size, int lower_bound, unsigned int delta = 0)
        : TArrayAsVector<T>(init_size, lower_bound, delta)
    { };

    bool Add(const T& t) {
      /* incorrect algorithm, found by Ruben P 
      int i = size() - 1;
      push_back(t);
      while (0 <= i && t < at(i)) {
        at(i+1) = at(i);
        --i;
      }
      if (i < (size() - 2)) {
        at(i+1) = t;
      }*/
      // fix by Ruben P 
      iterator i=this->begin();
      while(i!=this->end() && t>(*i))
        i++;

      if (i==this->end())
        push_back(t);
      else
        insert(i, t);

      return true;
    }

private:
    bool AddAt(const T&) { return false; };
};


template<class T> class TSArrayAsVectorIterator : public TArrayAsVectorIterator<T> {
public:
    TSArrayAsVectorIterator(const TSArrayAsVector<T>& a)
        : TArrayAsVectorIterator<T>(a)
    { };

    TSArrayAsVectorIterator(const TSArrayAsVector<T>& a, unsigned start, unsigned stop)
        : TArrayAsVectorIterator<T>(a, start, stop)
    { };
};


template<class T> class TIArrayAsVector : public TArrayAsVector<T*>, public TShouldDelete {
public:
    typedef void (*IterFunc)(T&, void*);
//    typedef bool (*CondFunc)(const T&, void*);

    explicit TIArrayAsVector(unsigned int init_size = 0)
        : TArrayAsVector<T*>(init_size)
    { };

    TIArrayAsVector(unsigned int init_size, int lower_bound, unsigned int delta = 0)
        : TArrayAsVector<T*>(init_size, lower_bound, delta)
    { };

    ~TIArrayAsVector() {
        Flush();
    }

    bool HasMember(const T* t) const
    {
        return _Find(t) != -1;
    }

    int Find(const T* t) const
    {
        return BoundBase(_Find(t));
    }

    void Flush( DeleteType dt = DefDelete )
    {
        if (DelObj(dt)) {
            for (int i = this->size() - 1; i >= 0; --i) {
                delete this->at(i);
                this->at(i) = 0;
            }
        }
        erase(this->begin(), this->end());
    }

//    int Detach(const T* t, TShouldDelete::DeleteType dt = TShouldDelete::NoDelete)
    int Detach(const T* t, DeleteType dt = NoDelete)
    {
        return Detach(Find(t), dt);
    }

//    int Detach(int loc, TShouldDelete::DeleteType dt = TShouldDelete::NoDelete)
    int Detach(int loc, DeleteType dt = NoDelete)
    {
        loc = this->ZeroBase(loc);
        if (loc >= this->size() || loc < 0)
            return 0;
        if (DelObj(dt)) {
            delete *(&* this->begin()+loc);
        }
        erase(this->begin()+loc);
        return 1;
    }

    int Destroy( T *t )
    {
        return Detach(t, Delete);
    }

    int Destroy( int loc )
    {
        return Detach(loc, Delete);
    }

    void ForEach(IterFunc iter, void* args)
    {
        for(int i = 0; i < this->size(); ++i)
            (iter)(* this->at(i), args);
    }

    bool Reallocate( unsigned sz, unsigned offset = 0 )
    {
        if (!this->d_delta) {
            return false;
        }
        --sz;
        sz = sz + this->d_delta - (sz % this->d_delta);
        if (sz > this->capacity()) {
            this->reserve(sz);
        }
        if (offset > 0) {
            insert(this->begin(), offset, 0);
        }
        if (this->size() < sz) {
            this->resize(sz, 0);
        }
        return true;
    }

protected:
    int _Find(const T* t) const
    {
        int i = this->size() - 1;
        while (i >= 0) {
            if (this->at(i) == t) break;
            --i;
        }
        return i;
    }

};


template<class T> class TIArrayAsVectorIterator {
public:

    typedef TIArrayAsVector<T> abase;
    typedef TIArrayAsVectorIterator<T> base;

    TIArrayAsVectorIterator(const abase& a)
        : d_a(&a), d_cur(0), d_lower(0), d_upper(a.size())
    { };

    TIArrayAsVectorIterator(const abase& a, unsigned start, unsigned stop)
        : d_a(&a), d_cur(start), d_lower(start), d_upper(stop)
    {
        PRECONDITION(0 <= d_cur);
        PRECONDITION(d_cur <= d_upper);
        PRECONDITION(d_upper <= d_a->size());
    };

    operator bool() const { return notdone(); }

    T* Current() const {
        PRECONDITION(d_cur < d_upper);
        return const_cast<T*>(d_a->at(d_cur));
    }

    T* operator ++(int) {
        d_cur++;
        PRECONDITION(d_cur <= d_upper);
        return const_cast<T*>(d_a->at(d_cur-1));
    }

    T* operator ++() {
        ++d_cur;
        PRECONDITION(d_cur < d_upper);
        return const_cast<T*>(d_a->at(d_cur));
    }

    void Restart() { Restart(d_lower, d_upper); };

    void Restart(unsigned start, unsigned stop) {
        PRECONDITION(0 <= d_cur);
        PRECONDITION(d_cur <= d_upper);
        PRECONDITION(d_upper <= d_a->size());
        d_cur = d_lower = start;
        d_upper = stop;
    }

protected:
    bool notdone() const { return d_cur < d_upper /* && d_cur < Vect.Size() */; }
    const abase* d_a;
    unsigned d_cur;
    unsigned d_upper;
    unsigned d_lower;
};


template<class T> class TISArrayAsVector : public TIArrayAsVector<T> {
public:
    explicit TISArrayAsVector(unsigned int init_size = 0)
        : TIArrayAsVector<T>(init_size)
    { };

    TISArrayAsVector(unsigned int init_size, int lower_bound, unsigned int delta = 0)
        : TIArrayAsVector<T>(init_size, lower_bound, delta)
    { };

    bool Add(T* t) {
      if (!t) return false;

      iterator i=this->begin();
      while(i!=this->end() && *t > **i)
        i++;

      if (i==this->end())
        push_back(t);
      else
        insert(i, t);

      return true;
    }

private:
    bool AddAt(const T&) { return false; };
};


template<class T> class TISArrayAsVectorIterator : public TIArrayAsVectorIterator<T> {
public:
    TISArrayAsVectorIterator(const TISArrayAsVector<T>& a)
        : TIArrayAsVectorIterator<T>(a)
        { };

    TISArrayAsVectorIterator(const TISArrayAsVector<T>& a, unsigned start, unsigned stop)
        : TIArrayAsVectorIterator<T>(a, start, stop)
        { };
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

#define TArray                  TArrayAsVector
#define TArrayIterator          TArrayAsVectorIterator
#define TSArray                 TSArrayAsVector
#define TSArrayIterator         TSArrayAsVectorIterator
#define TIArray                 TIArrayAsVector
#define TIArrayIterator         TIArrayAsVectorIterator
#define TISArray                TISArrayAsVector
#define TISArrayIterator        TISArrayAsVectorIterator

} // OWL namespace

#endif  // BI_USE_TEMPLATE


#endif  // CLASSLIB_ARRAYS_H

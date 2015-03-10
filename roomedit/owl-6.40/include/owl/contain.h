//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of container classes used and made available by OWL
//------------------------------------------------------------------------------

#if !defined(OWL_CONTAIN_H)
#define OWL_CONTAIN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/template.h>

//class TStandardAllocator;

namespace owl {

//------------------------------------------------------------------------------
//class _OWLCLASS TStandardAllocator;
//template <class T, class T1>          class TPtrArrayIterator;
//template <class T, class P, class A>  class TTypedArray;
//template <class T, class T1>          class TObjArrayIterator;
//template <class T>                    class TObjectArray;
//template <class T>                    class TSortedObjectArray;
//template <class T, class T1>          class TMapNode;

typedef TTypedArray<int,int,TStandardAllocator>        TIntArray;
typedef TPtrArrayIterator<int,TIntArray>               TIntArrayIterator;
typedef TTypedArray<uint32,uint32,TStandardAllocator>  TUint32Array;
typedef TPtrArrayIterator<uint32,TUint32Array>         TUint32ArrayIterator;
typedef TObjectArray<tstring>                       TStringArray;
typedef TObjArrayIterator<tstring,TStringArray>     TStringArrayIterator;
typedef TSortedObjectArray<tstring>                 TSortedStringArray;
typedef TObjArrayIterator<tstring, TSortedStringArray> TSortedStringArrayIterator;
typedef int                                            TInt; // for portability

typedef TTypedArray<LPARAM, LPARAM, TStandardAllocator> TLParamArray;
typedef TPtrArrayIterator<LPARAM, TLParamArray> TLParamArrayIterator;

#if defined(_OWLDLL) || defined(BI_APP_DLL)
//
// Export templates when building ObjectWindows DLL and provide import
// declaration of DLL instance for users of the class.
//
//template class _OWLCLASS TMapNode<string,string>;
//template class _OWLCLASS TObjectArray< TMapNode<string,string> >;
//template class _OWLCLASS TSortedObjectArray< TMapNode<string,string> >;
#endif //  _OWLDLL || BI_APP_DLL

//------------------------------------------------------------------------------

//
/// Maximum number of entries in each Vector
//
const uint MAXENTRIES = uint((uint32(UINT_MAX) - sizeof(uint))/
                                     sizeof(void*));

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>
//
/// \class TCollection
// ~~~~~ ~~~~~~~~~~~
/// Simple template holding pointers to Ts used internally by
/// ObjectWindows Controls
//
template <class T> class TCollection {
  public:
    TCollection(uint aLimit, bool shldDel = true);
   ~TCollection();

    T&          operator [](uint index) const;
    int         GetCount() const;

    int         Append(T* item);
    void        InsertAt(uint index, T* item);
    void        RemoveAt(uint index);
    void        RemoveAll();
    void        FreeAt(uint index);
    void        FreeAll();

  protected:
    void        SetLimit(uint aLimit);

  private:
    TCollection(const TCollection&);
    TCollection& operator =(const TCollection&);

    void        FreeItem(T* item);
    T**         items;              ///< Pointer to array of T*
    uint        count;              ///< Number of items
    uint        limit;              ///< Size array is allocated for
    bool        shouldDelete;       ///< Should free in destructor
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//------------------------------------------------------------------------------
// Inline implementations
//

//
template<class T>
TCollection<T>::TCollection(uint aLimit, bool shldDel)
: items(0), count(0), limit(0), shouldDelete(shldDel)
{
  SetLimit(aLimit);
}

//
template<class T>
TCollection<T>::~TCollection()
{
  if (shouldDelete)
    FreeAll();
  SetLimit(0);
}

//
template<class T>
int TCollection<T>::GetCount() const
{
  return count;
}

//
template<class T>
void TCollection<T>::RemoveAll()
{
  count = 0;
}

//
template<class T>
T& TCollection<T>::operator[](uint index) const
{
  PRECONDITION(index < count);
  return *items[index];
}

//
template<class T>
int TCollection<T>::Append(T* item)
{
  InsertAt(count, item);
  return count-1;
}

//
template<class T>
void TCollection<T>::InsertAt(uint index, T* item)
{
  PRECONDITION(index <= count);
  if (count == limit)
    SetLimit(count*2);
  if (count-index != 0)
    memmove((void*)&items[index+1], (void*)&items[index],
            (count-index)*sizeof(T*));
  items[index] = item;
  count++;
}

//
template<class T>
void TCollection<T>::RemoveAt(uint index)
{
  PRECONDITION(index < count);
  if (index != count-1)
    memmove((void*)&items[index], (void*)&items[index+1],
            (count-(index+1))*sizeof(T*));
  count--;
}

//
template<class T>
void TCollection<T>::FreeAt(uint index)
{
  T& item = operator[](index);
  RemoveAt(index);
  FreeItem(&item);
}

//
template<class T>
void TCollection<T>::FreeAll()
{
  for (uint i = 0; i < count; i++)
    FreeItem(&(operator[](i)));
  count = 0;
}

//
template<class T>
void TCollection<T>::FreeItem(T* item)
{
  delete item;
}

//
template<class T>
void TCollection<T>::SetLimit(uint aLimit)
{
  if (aLimit < count)
    aLimit = count;
  CHECK(aLimit <= MAXENTRIES);

  if (aLimit != limit) {
    T **aItems;

    if (aLimit == 0)
      aItems = 0;
    else {
      aItems = new T *[aLimit];
      if (count != 0)
        memcpy(aItems, items, count*sizeof(T*));
    }
    delete [] items;
    items = aItems;
    limit = aLimit;
  }
}


} // OWL namespace


#endif  // OWL_CONTAIN_H

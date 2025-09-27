//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//----------------------------------------------------------------------------
#ifndef _HASHTBL_H
#define _HASHTBL_H

#include <bole.h>

//------------------------------ IHashable -------------------------
// An ABC for objects which can be linked into a hash table

typedef unsigned int HashIndex;

template <class T> class _ICLASS IHashable : public IUnknown {
public:
  virtual HashIndex Hash(HashIndex modulo) const = 0;
  virtual BOOL operator==(const T *) const =0;
  virtual BOOL operator!=(const T *) const =0;
  virtual IHashable<T> *&Next() = 0;
};


template <class T, HashIndex SIZE> class  _ICLASS HashTable {
protected:
  IHashable<T> *  aBuckets[SIZE];
  IHashable<T> *&  Insert(IHashable<T> *&rpFound, IHashable<T> *pKey);
  IHashable<T> *&  Delete(IHashable<T> *&rpFound);
  IHashable<T> *&  Find(IHashable<T> *hashKey);

public:
  ~HashTable();
  // Add  returns existing instance or adds
  BOOL  Add(IHashable<T> **pPat);
  // Remove takes key out of HashTable
  void  Remove(IHashable<T> *pKey);
};


template <class T, HashIndex SIZE> IHashable<T> *& HashTable<T, SIZE>::
  Insert(IHashable<T> *&rpFound, IHashable<T> *pKey)
{
  pKey->Next()=rpFound,
  pKey->AddRef(),
  rpFound=pKey;
  return rpFound;
}

template <class T, HashIndex SIZE> IHashable<T> *& HashTable<T, SIZE>::
  Delete(IHashable<T> *&rpFound)
{
  assert(rpFound != 0L);
  IHashable<T> **ppFoundNext = &rpFound->Next();
  IHashable<T> *pFoundNext = *ppFoundNext;
  *ppFoundNext=0L;
  rpFound->Release();
  return rpFound = pFoundNext;
}

template <class T, HashIndex SIZE>
BOOL  HashTable<T, SIZE>::Add(IHashable<T> **ppPat)
{
  IHashable<T> *&pSrc = Find(*ppPat);
  if (pSrc) {
    (*ppPat)->Release();
    *ppPat = pSrc;
  }
  else
    Insert(pSrc, *ppPat) ;

  (*ppPat) ->AddRef();
  return TRUE;
}

//  (pSrc) ?
//    pPat->Release(),
//      pPat = pSrc :
//    Insert(pSrc, pPat) ;

template <class T, HashIndex SIZE>
void  HashTable<T, SIZE>::Remove(IHashable<T> *pKey)
{
  IHashable<T> *&pFound =Find(pKey);
  pFound ? Delete(pFound) : 0L;
}

#endif

//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//----------------------------------------------------------------------------
#include <hashtbl.h>
#include <oledebug.h>


template <class T, HashIndex SIZE>
HashTable<T, SIZE>::~HashTable()
{
  IHashable<T> *pH, *pTmp;
  for (HashIndex i=0; i < SIZE; i++) {
    pH = aBuckets[i];
    while (pH) {
      pTmp = pH->Next();
      pH -> Release();
      pH = pTmp;
    }
  }
}


template <class T, HashIndex SIZE>
IHashable<T> *&HashTable<T, SIZE>::Find(IHashable<T> *pKey)
{
  IHashable<T> **ppElem = &aBuckets[pKey->Hash(SIZE)];
  while ((*ppElem != NULL) && (**ppElem != (T*) pKey)) {
    ppElem = &(*ppElem)->Next();
  }
  return *ppElem;
}



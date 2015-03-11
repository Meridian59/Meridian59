//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TRegItem and TRegList members that need to be linked into user app and not
/// the WinSys DLL
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/registry.h>
#include <stdio.h>   // sprintf()

namespace owl {

//
/// Initialize the data members to 0.
//
TRegFormatHeap::TRegFormatHeap()
:
  Head(0)
{
}

//
/// Walk the heap chain & delete the memory blocks
//
TRegFormatHeap::~TRegFormatHeap()
{
  while (Head) {
    TBlock* next = Head->Next;
    delete[] (tchar*)Head;
    Head = next;
  }
}

//
/// Allocate a block of memory of a given size & link it into the heap chain
//
LPTSTR TRegFormatHeap::Alloc(int spaceNeeded)
{
  TBlock* newblock = (TBlock*) new tchar[sizeof(TBlock) + spaceNeeded];
  newblock->Next = Head;
  Head = newblock;

  return newblock->Data;
}

//
/// Performs the lookup of the TRegItems using a key (an item name such as progid)
/// and returns the value associated with the key (for example, "My Sample
/// Application"). The value is returned in the language specified in lang (for
/// example, French Canadian).
///
/// Returns an empty string if there is no associated value.
///
/// If the key does not exist, it returns a null pointer.
//
LPCTSTR TRegList::Lookup(LPCSTR key, TLangId lang)
{
  if (key) {
    for (TRegItem* regItem = Items; regItem->Key != 0; regItem++) {
      if (strcmp(regItem->Key, key) == 0)
        if (regItem->Value.Private)  // current can't test Value directly
          return regItem->Value.Translate(lang);
        else
          return _T("");
    }
  }
  return 0;
}

//
//
/// Looks up and returns a reference to a local string value associated with a
/// particular item name (key). You can then translate this string into the local
/// language as necessary.
//
TLocaleString& TRegList::LookupRef(LPCSTR key)
{
  for (TRegItem* regItem = Items; regItem->Key != 0; regItem++) {
    if (strcmp(regItem->Key, key) == 0)
       return regItem->Value;
  }
  return TLocaleString::Null;
}

//
// Maximum string length for REGFORMAT w/ string arg. String is clipped if too
// long.
//
const int MaxFormatLen = 40;

//
/// Registers data formats for the object.
//
//
LPTSTR TRegItem::RegFormat(int f, int a, int t, int d, TRegFormatHeap& heap)
{
  // sprintf into sized auto buffer
  // ints have a max of 11 digits: -2000000000. Add pad of 8 just in case
  //
  tchar temp[11+1+11+1+11+1+11+1+8];
  int len = _stprintf(temp, _T("%d,%d,%d,%d"), f, a, t, d);

  // Copy into real heap buffer & return it
  //
  return ::_tcscpy(heap.Alloc(len + 1), temp);
}

//
//
/// Registers data formats for the object.
//
LPTSTR TRegItem::RegFormat(LPCTSTR f, int a, int t, int d, TRegFormatHeap& heap)
{
  // _stprintf into sized auto buffer
  //
  tchar temp[MaxFormatLen+1+11+1+11+1+11+1+8];
  int len = _stprintf(temp, _T("%.*s,%d,%d,%d"), MaxFormatLen, (LPTSTR)f, a, t, d);

  // Copy into real heap buffer & return it
  //
  return ::_tcscpy(heap.Alloc(len + 1), temp);
}

//
/// Registers the flag.
//
//
LPTSTR TRegItem::RegFlags(long flags, TRegFormatHeap& heap)
{
  // _stprintf into sized auto buffer
  //
  tchar temp[11+1+8];
  int len = _stprintf(temp, _T("%ld"), flags);

  // Copy into real heap buffer & return it
  //
  return ::_tcscpy(heap.Alloc(len + 1), temp);
}

//
/// Registers the verb option.
//
//
LPTSTR TRegItem::RegVerbOpt(int mf, int sf, TRegFormatHeap& heap)
{
  // _stprintf into sized auto buffer
  //
  tchar temp[11+1+11+1+8];
  int len = _stprintf(temp, _T("%d,%d"), mf, sf);

  // Copy into real heap buffer & return it
  //
  return ::_tcscpy(heap.Alloc(len + 1), temp);
}

} // OWL namespace
/* ========================================================================== */

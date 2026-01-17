//----------------------------------------------------------------------------
// ObjectComponents
// Original Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// OleNew.cpp -- Remap global operator new to use the current Ole2 allocator
//----------------------------------------------------------------------------
#include "bole.h"
#include <windows.h>

#include <string.h>
#include <compobj.h>


/*inline*/ void _FAR * OleAlloc (unsigned size)
{
  void _FAR * p = NULL;
  IMalloc *pMalloc;

  if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pMalloc))) {
    p = pMalloc->Alloc(size);
    pMalloc->Release();
  }
  return p;
}

inline BOOL OleFree (void * p)
{
  IMalloc *pMalloc;

  if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pMalloc))) {
    pMalloc->Free(p);
    return TRUE;
  }
  return FALSE;
}

void _FAR * _cdecl _FARFUNC operator new (unsigned size)
{
  return OleAlloc(size);
}

void _FAR * _cdecl _FARFUNC operator new[] (unsigned size)
{
  return OleAlloc(size);
}

void _cdecl _FARFUNC operator delete (void *p)
{
  OleFree(p);
}

void _cdecl _FARFUNC operator delete[] (void *p)
{
  OleFree(p);
}


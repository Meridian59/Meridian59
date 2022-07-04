//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
// Contains some functions to make 32-bit bocole work under WIN95.
//----------------------------------------------------------------------------
#ifndef UNICODE

#include "bole.h"
#include "utils.h"

LPWSTR lstrcpyW2(LPWSTR lpString1, LPCWSTR lpString2)
{
  while (*lpString1++ = *lpString2++)
  ;

  return lpString1;
}

LPWSTR lstrcpynW2(LPWSTR lpString1, LPCWSTR lpString2, int count)
{
  for (int i = 0; i < count; i++)
    *(lpString1+i) = *(lpString2+i);

  return lpString1;
}

LPWSTR lstrcatW2(LPWSTR lpString1, LPCWSTR lpString2)
{
  lstrcpyW2(lpString1+lstrlenW2(lpString1), lpString2);

  return lpString1;
}

int lstrlenW2(LPCWSTR lpString)
{
  int count = 0;

  while (*lpString++)
    count++;

  return count;
}

int lstrcmpW2 (LPWSTR lpString1, LPCWSTR lpString2)
{
  while (*lpString1++) {
    if (*lpString1 != *lpString2)
      return -1;
  }
  if (*lpString2 != '\0')
    return -1;
  return 0;
}

#endif


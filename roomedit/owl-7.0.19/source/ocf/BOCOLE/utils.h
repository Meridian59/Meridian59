//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
// Utils.h -- Contains some functions to make 32-bit bocole work under WIN95.
//----------------------------------------------------------------------------
#ifndef _UTILS_H
#define _UTILS_H

#ifndef UNICODE

#undef lstrcpy
#undef lstrlen
#undef lstrcpyn
#undef lstrcat
#undef lstrcmp

#define lstrcpy(a,b)  lstrcpyW2(a,b)
#define lstrlen(a)    lstrlenW2(a)
#define lstrcpyn(a, b, c)   lstrcpynW2(a, b, c)
#define lstrcat(a, b)   lstrcatW2(a, b)
#define lstrcmp(a, b)   lstrcmpW2(a, b)

LPWSTR lstrcpyW2(LPWSTR lpString1, LPCWSTR lpString2);
int    lstrlenW2(LPCWSTR lpString);
LPWSTR lstrcpynW2(LPWSTR lpString1, LPCWSTR lpString2, int iMaxLength);
LPWSTR lstrcatW2(LPWSTR lpString1, LPCWSTR lpString2);
int    lstrcmpW2(LPWSTR lpString1, LPCWSTR lpString2);

#undef  MAKEINTRESOURCEW
#define MAKEINTRESOURCEW  MAKEINTRESOURCEA
#define LoadCursorW LoadCursorA
#define DefWindowProcW DefWindowProcA

#endif  // UNICODE

#endif  // _UTILS_H

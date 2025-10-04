//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// PortDefs.h -- Contains some portability macros which are used to compile
//               the same Bolero codebase for Win32 and Win64.
//----------------------------------------------------------------------------
#ifndef _portdefs_h
#define _portdefs_h

#ifdef WIN32

#  ifdef FAR
#    undef FAR
#  endif

#  ifdef _FAR
#    undef _FAR
#  endif

#  ifdef __FAR
#    undef __FAR
#  endif

#  ifdef PASCAL
#    undef PASCAL
#  endif

#  ifdef CDECL
#    undef CDECL
#  endif

#  ifdef _CDECL
#    undef _CDECL
#  endif

#  ifdef __CDECL
#    undef __CDECL
#  endif

#  ifdef HUGE
#    undef HUGE
#  endif

#  ifdef _HUGE
#    undef _HUGE
#  endif

#  ifdef __HUGE
#    undef __HUGE
#  endif

#  ifdef _LOADDS
#    undef _LOADDS
#  endif

#define PASCAL          __stdcall
#define __STDC__        TRUE

#ifdef far
#undef far
#define far
#endif

#define FAR      far
#define _FAR
#define __FAR
#define _far


#define CDECL
#define __CDECL
#define _CDECL
#define _cdecl

#define HUGE
#define _HUGE
#define __HUGE
#define huge
#define _huge

#define _LOADDS
#define _loadds

#define MAKEPOINT(l)       (*((POINT FAR *)&(l)))

#define SetViewportExt(a,b,c)   SetViewportExtEx(a,b,c,NULL)
#define SetWindowExt(a,b,c)     SetWindowExtEx(a,b,c,NULL)
#define SetWindowOrg(a,b,c)     SetWindowOrgEx(a,b,c,NULL)
#define MoveTo(a,b,c)           MoveToEx(a,b,c,NULL)
#define GetTextExtent(a,b,c)    GetTextExtentPoint(a,b,c,NULL)
#define OffsetWindowOrg(a,b,c)  OffsetWindowOrgEx(a,b,c,NULL)

#ifdef  strcmpi
#undef  strcmpi
#endif

// String operations on default character set (Unicode)
// #define strcpy(a,b)    lstrcpy(a,b)
// #define strcat(a,b)    lstrcat(a,b)
#define strcmp(a,b)    lstrcmp(a,b)
#define strcmpi(a,b)   lstrcmpi(a,b)
#define strupr(a)      CharUpper(a)
#define strlwr(a)      CharLower(a)
#ifdef UNICODE
#define strlen(a)      lstrlen(a)
#endif

// String operations on ANSI
#define strcpyA(a,b)    lstrcpyA(a,b)
#define strlenA(a)      lstrlenA(a)
#define strcatA(a,b)    lstrcatA(a,b)
#define strcmpA(a,b)    lstrcmpA(a,b)
#define strcmpiA(a,b)   lstrcmpiA(a,b)

// String operations on Unicode
#define strcpyW(a,b)    lstrcpyW(a,b)
#define strlenW(a)      lstrlenW(a)
#define strcatW(a,b)    lstrcatW(a,b)
#define strcmpW(a,b)    lstrcmpW(a,b)
#define strcmpiW(a,b)   lstrcmpW(a,b)

#else   /* For 64-bit, for the first version is the same */

#  ifdef FAR
#    undef FAR
#  endif

#  ifdef _FAR
#    undef _FAR
#  endif

#  ifdef __FAR
#    undef __FAR
#  endif

#  ifdef PASCAL
#    undef PASCAL
#  endif

#  ifdef CDECL
#    undef CDECL
#  endif

#  ifdef _CDECL
#    undef _CDECL
#  endif

#  ifdef __CDECL
#    undef __CDECL
#  endif

#  ifdef HUGE
#    undef HUGE
#  endif

#  ifdef _HUGE
#    undef _HUGE
#  endif

#  ifdef __HUGE
#    undef __HUGE
#  endif

#  ifdef _LOADDS
#    undef _LOADDS
#  endif

#define PASCAL          __stdcall
#define __STDC__        TRUE

#ifdef far
#undef far
#define far
#endif

#define FAR      far
#define _FAR
#define __FAR
#define _far


#define CDECL
#define __CDECL
#define _CDECL
#define _cdecl

#define HUGE
#define _HUGE
#define __HUGE
#define huge
#define _huge

#define _LOADDS
#define _loadds

#define MAKEPOINT(l)       (*((POINT FAR *)&(l)))

#define SetViewportExt(a,b,c)   SetViewportExtEx(a,b,c,NULL)
#define SetWindowExt(a,b,c)     SetWindowExtEx(a,b,c,NULL)
#define SetWindowOrg(a,b,c)     SetWindowOrgEx(a,b,c,NULL)
#define MoveTo(a,b,c)           MoveToEx(a,b,c,NULL)
#define GetTextExtent(a,b,c)    GetTextExtentPoint(a,b,c,NULL)
#define OffsetWindowOrg(a,b,c)  OffsetWindowOrgEx(a,b,c,NULL)

#ifdef  strcmpi
#undef  strcmpi
#endif

// String operations on default character set (Unicode)
// #define strcpy(a,b)    lstrcpy(a,b)
// #define strcat(a,b)    lstrcat(a,b)
#define strcmp(a,b)    lstrcmp(a,b)
#define strcmpi(a,b)   lstrcmpi(a,b)
#define strupr(a)      CharUpper(a)
#define strlwr(a)      CharLower(a)
#ifdef UNICODE
#define strlen(a)      lstrlen(a)
#endif

// String operations on ANSI
#define strcpyA(a,b)    lstrcpyA(a,b)
#define strlenA(a)      lstrlenA(a)
#define strcatA(a,b)    lstrcatA(a,b)
#define strcmpA(a,b)    lstrcmpA(a,b)
#define strcmpiA(a,b)   lstrcmpiA(a,b)

// String operations on Unicode
#define strcpyW(a,b)    lstrcpyW(a,b)
#define strlenW(a)      lstrlenW(a)
#define strcatW(a,b)    lstrcatW(a,b)
#define strcmpW(a,b)    lstrcmpW(a,b)
#define strcmpiW(a,b)   lstrcmpW(a,b)

#endif

#endif

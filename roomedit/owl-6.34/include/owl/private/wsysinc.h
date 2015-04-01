//----------------------------------------------------------------------------
// Object Windows Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
// Includes windowing system headers, with necessary macros defined
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_WSYSINC_H)
#define OWL_PRIVATE_WSYSINC_H
#define SERVICES_WSYSINC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

//
// Get "windows.h" and other standard windows headers.
//
#if !defined(_INC_WINDOWS)

//
// We're going to pull in "windows.h".
// Select API version, and set the header arguments we require.
// We target Windows API version 0x0501 (Windows XP).
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // _WIN32_WINNT_WINXP
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef STRICT
#define STRICT
#endif

#pragma pack(push,8)
#include <windows.h>
#pragma pack(pop)

#else

//
// Unfortunately, "windows.h" has already been included.
// Ensure that it was included with the required header arguments.
// In general, let OWLNext include "windows.h".
//
#if !defined(STRICT) || !defined(NOMINMAX)
#error If "windows.h" is included before OWLNext, then STRICT and NOMINMAX must be defined.
#endif

#endif

//
// Retrieve other Windows headers (before we proceed with BOOL-remapping)
//

// VER.H is currently mucking with definitions of UINT if LIB is not 
// defined.
//

// Grab shell extension header


// Avoid mapping GetWindowLongPtr to GetWindowLong[A/W]
#if defined(GetWindowLongPtr)
#undef GetWindowLongPtr
inline LONG_PTR GetWindowLongPtr(HWND hWnd, int nIndex)
#if defined(UNICODE)
  { return ::GetWindowLongPtrW(hWnd, nIndex); }
# else
  { return ::GetWindowLongPtrA(hWnd, nIndex); }
# endif
#endif

// Avoid mapping SetWindowLongPtr to SetWindowLong[A/W]
#if defined(SetWindowLongPtr)
#undef SetWindowLongPtr
inline LONG_PTR SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
#if defined(UNICODE)
  { return ::SetWindowLongPtrW(hWnd, nIndex, dwNewLong); }
# else
  { return ::SetWindowLongPtrA(hWnd, nIndex, dwNewLong); }
# endif
#endif



// Avoid mapping MessageBox to MessageBox[A/W]
#if defined(MessageBox)
#undef MessageBox
inline int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
#if defined(UNICODE)
  { return ::MessageBoxExW(hWnd, lpText, lpCaption, uType, 0); }
# else
  { return ::MessageBoxExA(hWnd, lpText, lpCaption, uType, 0); }
# endif
#endif

// Avoid mapping GetFileTitle to GetFileTitle[A/W]
#if defined(GetFileTitle)
#undef GetFileTitle
inline short GetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf)
#ifdef UNICODE
  { return ::GetFileTitleW(lpszFile, lpszTitle, cbBuf); }
#else
  { return ::GetFileTitleA(lpszFile, lpszTitle, cbBuf); }
#endif
#endif

// Avoid Win95 mapping DrawStatusText to DrawStatusText[A/W]
#if defined(DrawStatusText)
#undef DrawStatusText
inline void DrawStatusText(HDC hDC, 
                           LPRECT lprc, 
                           LPCTSTR szText,
                           UINT uFlags)
#ifdef UNICODE
  { ::DrawStatusTextW(hDC, lprc, szText, uFlags); }
#else
  { ::DrawStatusTextA(hDC, lprc, szText, uFlags); }
#endif
#endif

// Avoid Win95 mapping DrawState to DrawState[A/W]
#if defined(DrawState)
#undef DrawState
inline BOOL DrawState(HDC hdc, HBRUSH hbr, DRAWSTATEPROC lpOutputFunc,
  LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT fuFlags)
#ifdef UNICODE
  { return ::DrawStateW(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy, fuFlags); }
#else
  { return ::DrawStateA(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy, fuFlags); }
#endif
#endif

#if defined(SetProp)
#undef SetProp
inline BOOL SetProp(HWND hWnd, LPCTSTR lpString, HANDLE hData)
#if defined(UNICODE)
{ return ::SetPropW(hWnd, lpString, hData); }
# else
{ return ::SetPropA(hWnd, lpString, hData); }
# endif
#endif

#endif  // OWL_PRIVATE_WSYSINC_H


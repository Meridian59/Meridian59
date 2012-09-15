// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * winxblak.h:  Replacements for some of the windowsx.h message crackers.
 *   For some reason, some of the default crackers discard return values
 *   when the values are needed to determine if the message should be 
 *   passed on to Windows.  These replacements preserve the return values.
 */

#ifndef _WINXBLAK_H
#define _WINXBLAK_H

/* Bool Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), TRUE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)))
/* Bool Cls_OnSysKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_SYSKEYDOWN_BLAK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), TRUE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)))

/* void Cls_OnEnterSizeMove(HWND hwnd) */
#define HANDLE_WM_ENTERSIZEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)), 0L)
/* void Cls_OnExitSizeMove(HWND hwnd) */
#define HANDLE_WM_EXITSIZEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)), 0L)

// Macros for common controls

#define Trackbar_SetRange(hwnd, low, high, redraw) \
SendMessage(hwnd, TBM_SETRANGE, (WPARAM)(BOOL)redraw, (LPARAM)(MAKELONG(low, high)))

#define Trackbar_SetPos(hwnd, pos) \
SendMessage(hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM)(pos))

#define Trackbar_GetPos(hwnd) \
SendMessage(hwnd, TBM_GETPOS, 0, 0)

#endif

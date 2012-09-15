// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * winmsg.h
 *
 */

#ifndef _WINMSG_H
#define _WINMSG_H

/*
 * Main states of client:
 * STATE_INIT:       Program is starting up.
 * STATE_OFFLINE:    Not connected to server.
 * STATE_CONNECTING: Connecting socket to server.
 * STATE_STARTUP:    Synchronizing with server.
 * STATE_LOGIN:      Using login protocol with server (main menu, etc).
 * STATE_TERM:       Terminal mode (used for admin mode).
 * STATE_DOWNLOAD:   Performing file download.
 * STATE_GAME:       Playing the game.
 */
enum { STATE_OFFLINE, STATE_TERM, STATE_GAME, STATE_STARTUP, STATE_LOGIN, 
	  STATE_DOWNLOAD, STATE_INIT, STATE_CONNECTING, };

#ifdef __cplusplus
extern "C" {
#endif

void StartHelp(void);

BOOL MainInit(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void MainQuit(HWND hwnd);

void MainInitState(int new_state);
void MainExitState(void);
void MainSetState(int new_state);
void MainChangeColor(void);
void MainChangeFont(void);

void MainExpose(HWND hwnd);
Bool MainKey(HWND hwnd, UINT vk,BOOL fDown,int cRepeat,UINT flags);
void MainChar(HWND hwnd, char ch, int cRepeat);
void MainMouseLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void MainMouseMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void MainMouseRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void MainMouseLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void MainMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void MainVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
void MainTimer(HWND hwnd, UINT id);
void MainMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags);
void MainProcessBuffer(void);
void MainCommand(HWND hwnd,int id,HWND hwndCtl,UINT codeNotify);
void MainResize(HWND hwnd, UINT resize_flag, int xsize, int ysize);
void MainMinMaxInfo(HWND hwnd, MINMAXINFO FAR *lpmmi);
void MainMove(HWND hwnd, int x, int y);
void MainSetFocus(HWND hwnd, HWND hwndOldFocus);
void MainKillFocus(HWND hwnd, HWND hwndNewFocus);
Bool MainDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void MainPaletteChanged(HWND hwnd, HWND hwndPaletteChange);
Bool MainQueryNewPalette(HWND hwnd);
void MainIdle(void);
void MainEnterIdle(HWND hwnd, UINT source, HWND hwndSource);
int  MainNotify(HWND hwnd, int idCtrl, NMHDR *pnmh);
void MainActivateApp(HWND hwnd, BOOL fActivate, DWORD dwThreadId);

void MainReadSocket(HWND hwnd, int SelectType, SOCKET s, int error);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _WINMSG_H */

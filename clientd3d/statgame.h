// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * statgame.h
 *
 */

#ifndef _STATGAME_H
#define _STATGAME_H

/* Game states:
 *
 * GAME_NONE:     Main state is not STATE_GAME.
 * GAME_INIT:     Loading resources & initializing game.
 * GAME_PLAY:     "Normal" playing state.
 * GAME_SELECT:   Selecting an object as a target.
 * GAME_RESYNC:   Resynchronizing with server.
 * GAME_WAIT:     Server has told us to wait.
 * GAME_INVALID:  Our game data is invalid; we are waiting to receive new data from server.
 */
enum { GAME_NONE = 0, GAME_PLAY = 1, GAME_RESYNC = 2, GAME_INIT = 3,
    GAME_WAIT = 4, GAME_INVALID = 5, GAME_SELECT = 6, };

/* Return True iff user actions shouldn't be processed in given game state */
#define IsWaitingGameState(s) ((s) == GAME_WAIT || (s) == GAME_INVALID || (s) == GAME_NONE)

void GameInit(void);
void GameExit(void);
void GameExpose(HWND hwnd);
Bool GameKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void GameMouseButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void GameLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void GameMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void GameMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags);
void GameVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
void GameMove(HWND hwnd, int x, int y);
void GameResize(HWND hwnd, UINT resize_flag, int xsize, int ysize);
void GameSetFocus(HWND hwnd, HWND hwndOldFocus);
void GameKillFocus(HWND hwnd, HWND hwndNewFocus);
void GameCommand(HWND hwnd,int id,HWND hwndCtl,UINT codeNotify);
Bool GameDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void GameIdle(void);
void GameTimer(HWND hwnd, UINT id);
void GameEnterIdle(HWND hwnd, UINT source, HWND hwndSource);
int  GameNotify(HWND hwnd, int idCtrl, NMHDR *pnmh);

M59EXPORT void GameSetState(int new_state);
M59EXPORT int  GameGetState(void);

void PingSentMessage(void);

#endif

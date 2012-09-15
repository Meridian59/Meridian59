// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * chess.h:  Header file for chess.c
 */

#ifndef _CHESS_H
#define _CHESS_H

#include "chessrc.h"
#include "board.h"
#include "cmove.h"

#define MODULE_ID        110   // Unique module id #

extern client_message chess_msg_table[];

#define RequestGameMove(obj, s)    ToServer(BP_USERCOMMAND, user_msg_table, UC_MINIGAME_STATE, obj, s)
#define RequestGameResetPlayers(obj)  ToServer(BP_USERCOMMAND, user_msg_table, UC_MINIGAME_RESET_PLAYERS, obj)

extern HINSTANCE   hInst;          // Handle of this DLL
extern HWND        hChessDlg;
extern ClientInfo *c;              // Holds data passed from main client
extern Bool        exiting;        // True when module is exiting and should be unloaded

void ChessRedrawBoard(void);
BYTE ChessGetPromotionPiece(void);

#endif /* #ifndef _CHESS_H */


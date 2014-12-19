// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * game.h
 *
 */

#ifndef _GAME_H
#define _GAME_H

enum { GAME_NORMAL, GAME_BEACON, GAME_FINAL_SYNC };

typedef struct
{
   int object_id;
   int game_state;
   int game_sync_index;
   int game_last_message_time;
} game_data;

enum
{
   DM_CMD_GO_ROOM = 1,
   DM_CMD_GO_PLAYER = 2,
   DM_CMD_GET_PLAYER = 3
};

void UpdateSecurityRedbook(void);
const char* GetSecurityRedbook(void);
int GetSecurityRedbookID(void);

#endif

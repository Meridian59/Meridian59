// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * savegame.h
 *
 */

#ifndef _SAVEGAME_H
#define _SAVEGAME_H

enum
{
   SAVE_GAME_CLASS = 1,
   SAVE_GAME_RESOURCE = 2,
   SAVE_GAME_SYSTEM = 3,
   SAVE_GAME_OBJECT = 4,
   SAVE_GAME_LIST_NODES = 5,
   SAVE_GAME_TIMER = 6,
   SAVE_GAME_USER = 7
};

Bool SaveGame(char *filename);

#endif

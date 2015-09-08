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

// Version 0 save games don't have a SAVE_GAME_VERSION byte/version added.
// Version 1 are save games with the SAVE_GAME_BUILTINOBJ type that can load
// any number of built-in objects (i.e. System, Settings, RealTime, etc.)
#define SAVEGAME_VERS 1

// Buffer used to store info for writing save game in blocks.
#define SAVEGAME_BUFFER 1048576

enum
{
   SAVE_GAME_CLASS = 1,
   SAVE_GAME_RESOURCE = 2,
   SAVE_GAME_BUILTINOBJ = 3,
   SAVE_GAME_OBJECT = 4,
   SAVE_GAME_LIST_NODES = 5,
   SAVE_GAME_TIMER = 6,
   SAVE_GAME_USER = 7,
   SAVE_GAME_TABLES = 8,
   SAVE_GAME_VERSION = 9
};

Bool SaveGame(char *filename);

#endif

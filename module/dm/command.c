// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * command.c:  Handle typed user commands.
 */

#include "client.h"
#include "dm.h"

static ID FindPlayerByName(char *name);
/************************************************************************/
/*
 * CommandShowGChannel: "gchannel" command
 */
void CommandShowGChannel(char *args)
{
   ShowGChannelDlg();
}
/************************************************************************/
/*
 * CommandGoRoom: "goroom" command
 */
void CommandGoRoom(char *args)
{
   if (*args == 0)
      return;
   RequestDMCommand(DM_GO_ROOM, args);
}
/************************************************************************/
/*
 * CommandGoPlayer: "goplayer" command
 */
void CommandGoPlayer(char *args)
{
   ID player;
   char temp[MAXAMOUNT + 1];

   if (*args == 0)
      return;

   player = FindPlayerByName(args);
   if (player == 0)
   {
      GameMessage(GetString(hInst, IDS_NOPLAYERNAME));
      return;
   }

   if (player == INVALID_ID)
   {
      GameMessage(GetString(hInst, IDS_DUPLICATEPLAYERNAME));
      return;
   }

   sprintf(temp, "%d", player);
   RequestDMCommand(DM_GO_PLAYER, temp);
}
/************************************************************************/
/*
 * CommandGetPlayer: "getplayer" command
 */
void CommandGetPlayer(char *args)
{
   ID player;
   char temp[MAXAMOUNT + 1];

   if (*args == 0)
      return;

   player = FindPlayerByName(args);
   if (player == 0)
   {
      GameMessage(GetString(hInst, IDS_NOPLAYERNAME));
      return;
   }

   if (player == INVALID_ID)
   {
      GameMessage(GetString(hInst, IDS_DUPLICATEPLAYERNAME));
      return;
   }

   sprintf(temp, "%d", player);
   RequestDMCommand(DM_GET_PLAYER, temp);
}
/************************************************************************/
/*
 * CommandShowMap:  "showmap" command.
 */
void CommandShowMap(char *args)
{
  MapShowAllWalls(cinfo->current_room, True);
  RedrawAll();
}
/************************************************************************/
/*
 * CommandHideMap:  "hidemap" command.
 */
void CommandHideMap(char *args)
{
  MapShowAllWalls(cinfo->current_room, False);
  RedrawAll();
}
/************************************************************************/
/*
 * CommandResetData:  "reset" command.
 */
void CommandResetData(char *args)
{
  ResetUserData();
}
/************************************************************************/
/*
 * CommandDM: "DM" say command by Chris!  Yay!  Infiltrating the client, slowly...
 */
void CommandDM(char *args)
{
   if (*args == 0)
      return;
   SendSay(SAY_DM, args);
}
/************************************************************************/
/*
 * CommandEcho: Room-wide purple message without the name.
 */
void CommandEcho(char *args)
{
   if (*args == 0)
      return;
   SendSay(SAY_MESSAGE, args);
}
/************************************************************************/
/*
 * FindPlayerByName:  Return the object id of the logged-on player whose name
 *   best matches name.  Return 0 if no one matches, -1 if more than one
 *   player matches equally well.
 */
ID FindPlayerByName(char *name)
{
   list_type l;
   char *player, *ptr;
   int match, max_match;
   ID best_player;
   Bool tied;            // True if a different player matches as well as best_player

   max_match = 0;
   tied = False;
   for (l = *(cinfo->current_users); l != NULL; l = l->next)
   {
      object_node *obj = (object_node *) (l->data);

      player = LookupRsc(obj->name_res);
      if (player == NULL)
	 continue;

      ptr = name;
      match = 0;
      while (*ptr != 0 && *player != 0)
      {
	 if (toupper(*ptr) != toupper(*player))
	 {
	    match = 0;
	    break;
	 }
	 match++;
	 ptr++;
	 player++;
      }

      // Check for exact match, or extra characters in search string
      if (*player == 0)
      {
	 if (*ptr == 0)
	    return obj->id;
	 else
	    continue;
      }

      if (match > max_match)
      {
	 max_match = match;
	 best_player = obj->id;
	 tied = False;
      }
      else if (match == max_match)
	 tied = True;
   }
   
   if (max_match == 0)
      return 0;

   if (tied)
      return INVALID_ID;

   return best_player;
}

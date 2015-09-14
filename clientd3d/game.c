// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * game.c:  Handle main game events when server sends messages.
 */

#include "client.h"

player_info player;
room_type current_room;
BOOL dataValid = FALSE;

/* This flag is True before we get the first player info message from the server,
 * and when we're not in the game.  We use it to keep track of entering the game,
 * so that we can load stuff from the INI file only the first time we get a player
 * info message for a given player.
 */
static Bool first_entry;
static BOOL frameDrawn = FALSE;           

void SetGameDataValid(BOOL flag)
{
   dataValid = flag;
}

BOOL GetGameDataValid(void)
{
   return dataValid;
}

void SetFrameDrawn(BOOL flag)
{
   if (!frameDrawn && flag) // this is first visible flag and we need to send MOVE to server
   {
      InvalidateRect(hMain, NULL, FALSE);
      MoveUpdateServer();
      CacheReport();
   }
   frameDrawn = flag;
}

BOOL GetFrameDrawn(void)
{
   return frameDrawn;
}

/* local function prototypes */
static void EnterNewRoom(void);
/************************************************************************/
void InitializeGame(void)
{
   LoadResources();
      
   dataValid = FALSE;
   first_entry = True;

   current_room.contents    = NULL;
   current_room.projectiles = NULL;
   current_room.tree        = NULL;
   current_room.flags       = 0;
   memset(&player, 0, sizeof(player_info));

   PlayerResetHeight();  // Set player to be looking straight ahead

   UseListDelete();

   EffectsInit();

   MoveSetValidity(False);
   srand((unsigned) time(NULL));
}
/************************************************************************/
/*
 * ResetUserData:  Ask the server to refresh our knowledge of the game.
 *   Called at the start of the game, and anytime we need to resynchronize 
 *   with the server, or in response to a RESETDATA message.
 */
void ResetUserData(void)
{
   SetGameDataValid(FALSE);
   ModuleEvent(EVENT_RESETDATA);

   /* Kill off look dialog, if present; its data is stale */
   AbortLookList();

   if (OfferInProgress()) 
   {
      OfferCanceled();    /* Offer dialogs are also stale */
      SendCancelOffer();  /* Let server know that we are canceling offer */
   }

   MoveSetValidity(False);
   GameSetState(GAME_INVALID);  /* Our data is bad until we hear from server */

   current_room.contents    = RoomObjectListDestroy(current_room.contents);
   current_room.projectiles = list_destroy(current_room.projectiles);
   current_room.flags       = 0;
   player.inventory         = ObjectListDestroy(player.inventory);
   BackgroundOverlaysReset();

   PlayerOverlaysFree();

   UseListDelete();

   /* Server automatically sends us some info on first startup */
   if (!first_entry)
   {
      debug(("wait for player\n"));
      RequestPlayer();
      debug(("wait for room\n"));
      RequestRoom();
      debug(("ask for current players\n"));
      RequestPlayers();
   }

   debug(("ask for inventory\n"));
   RequestInventory();
}
/************************************************************************/
void CloseGame(void)
{
   SetGameDataValid(FALSE);
   MapExitRoom(&current_room);
   BackgroundOverlaysReset();

   current_room.contents    = RoomObjectListDestroy(current_room.contents);
   current_room.projectiles = list_destroy(current_room.projectiles);
   BSPRoomFree(&current_room);

   PlayerOverlaysFree();

   player.inventory = ObjectListDestroy(player.inventory);
   UseListDelete();
   EffectsExit();

   FreeCurrentUsers();
   FreeResources();

   first_entry = True;
}
/************************************************************************/
/*
 * GetPlayerId:  Return player's id #.
 */
ID GetPlayerId(void)
{
   return player.id;
}
/************************************************************************/
/*
 * GetRoomObjectById:  Return object node of object in current room contents,
 *   or NULL if it isn't there.
 */
room_contents_node *GetRoomObjectById(ID id)
{
   return (room_contents_node *)
      list_find_item(current_room.contents, (void *) id, CompareIdRoomObject);
}

/* called from messages from server */
/************************************************************************/
void GameMessage(char *message)
{
   DisplayServerMessage(message, GetColor(COLOR_SYSMSGFGD), 0);
}
/************************************************************************/
/*
 * GameMessagePrintf:  varargs version of GameMessage.
 *   Handles strings up to 1k in length.
 */
void _cdecl GameMessagePrintf(char *fmt, ...)
{
   char s[1024];
   va_list marker;

   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   GameMessage(s);
}
/************************************************************************/
void GameSysMessage(char *message)
{
   DisplayServerMessage(message, GetColor(COLOR_SYSMSGFGD), 0);
}
/************************************************************************/
void SetPlayerRemoteView(ID objID, int flags, int height, BYTE light)
{
   room_contents_node *viewObject = NULL;

   RedrawAll();
   if (objID)
      viewObject = GetRoomObjectById(objID);

   if ((objID == player.id) || (objID == 0) || (NULL == viewObject))
   {
      player.viewID = 0;
      player.viewFlags = REMOTE_VIEW_MOVE | REMOTE_VIEW_TURN | 
	 REMOTE_VIEW_TILT | REMOTE_VIEW_CAST | REMOTE_VIEW_LOOK;
      player.viewHeight = player.height;
      player.viewLight = player.light;
   }
   else
   {
      int objHeight = 0;
      int objWidth = 0;
      int floor = GetFloorBase(viewObject->motion.x,viewObject->motion.y);
      if (viewObject->obj.boundingHeight == 0)
      {
	 if(GetObjectSize(viewObject->obj.icon_res, viewObject->obj.animate->group, 0, *(viewObject->obj.overlays), 
			 &objWidth, &objHeight))
	 {
	    viewObject->obj.boundingHeight = objHeight;
	    viewObject->obj.boundingWidth = objWidth;
	 }
      }
      else
      {
	 objWidth = viewObject->obj.boundingWidth;
	 objHeight = viewObject->obj.boundingHeight;
      }
      player.viewID = objID;
      player.viewFlags = flags;
      if (flags & REMOTE_VIEW_VALID_HEIGHT)
	 player.viewHeight = floor + height;
      else if (flags & REMOTE_VIEW_TOP)
	 player.viewHeight = viewObject->motion.z + objHeight;
      else if (flags & REMOTE_VIEW_BOTTOM)
	 player.viewHeight = viewObject->motion.z;
      else if (flags & REMOTE_VIEW_MID)
	 player.viewHeight = viewObject->motion.z + objHeight/2;
      else 
	 player.viewHeight = player.height;
      if (flags & REMOTE_VIEW_VALID_LIGHT)
	 player.viewLight = light;
      else
	 player.viewLight = player.light;
   }
}
/************************************************************************/
void SetPlayerInfo(player_info *new_player, BYTE ambient_light, ID bkgnd_id)
{
   char *fname;

   player.id            = new_player->id;
   player.name_res      = new_player->name_res;
   player.icon_res      = new_player->icon_res;
   player.room_id       = new_player->room_id;
   player.room_name_res = new_player->room_name_res;
   player.room_res      = new_player->room_res;
   player.light         = new_player->light;
   player.room_security = new_player->room_security;

   // XXX should get player's width from kod
   player.width = 31 * KOD_FINENESS / 4; // FINENESS >> 1;
   player.height = 3 * FINENESS / 4;  /* XXX For now user's eye at constant height */
   ResetPlayerPosition();

   current_room.ambient_light = ambient_light;
   current_room.bkgnd = bkgnd_id; 

   /* Save walls and free old room, if any */
   if (current_room.tree != NULL)
   {
      MapExitRoom(&current_room);
      BSPRoomFree(&current_room);
   }

   /* Get id's filename */
   fname = LookupRsc(new_player->room_res);
   if (fname == NULL)
      return;
	
   debug(("Loading room file %s\n", fname));
   GameSetState(GAME_INVALID);  /* Wait for room contents */

   if (config.clearCache)
      CacheClearAll();

   /* Load room info */
   if (!LoadRoomFile(fname, &current_room))
   {
      AbortGameDialogs();
      effects.paralyzed = TRUE;
      effects.blind = TRUE;
      ClientError(hInst, hMain, IDS_NOROOMFILE, fname);
   }

   // Check room security with value from roo file (lower 28 bits only)
   if (config.security && GetObjId(player.room_security) != GetObjId(current_room.security))
   {
      debug(("Room security mismatch between server and client\n"));
      AbortGameDialogs();
      effects.paralyzed = TRUE;
      effects.blind = TRUE;
      ClientError(hInst, hMain, IDS_NOROOMFILE, fname);
   }

   first_entry = False;

   EnterNewRoom();
}
/************************************************************************/
void EnterNewRoom(void)
{
   SetFrameDrawn(FALSE);

   DrawGridBorder();   

   SoundStopAll(SF_LOOP);  // Turn off looping sounds

   //	Clear any user selected target.
	SetUserTargetID( INVALID_ID );

   // Set up graphics for drawing new room
   EnterNewRoom3D(&current_room);

   MapEnterRoom(&current_room);

   LightChanged3D(player.light, current_room.ambient_light);

   ServerMovedPlayer();

   /* Don't redraw here; wait until we get room's contents */
}

void SetRoomFlags(DWORD flags)
{
   current_room.flags = flags;
}

DWORD GetRoomFlags(void)
{
   return current_room.flags;
}

void SetOverrideRoomDepth(int level, DWORD depth)
{
   current_room.overrideDepth[level] = depth;
}

DWORD GetOverrideRoomDepth(int level)
{
   return current_room.overrideDepth[level];
}

/************************************************************************/
void SetRoomInfo(ID room_id, list_type new_room_contents)
{
   list_type l;
   room_contents_node *r;

   if (room_id != player.room_id)
   {
      debug(("Tried to set contents of room %d; player is in room %d\n", room_id, player.room_id));
      return;
   }

   /* Get rid of old room contents */
   RoomObjectListDestroy(current_room.contents);

   current_room.contents    = new_room_contents;
   current_room.projectiles = list_destroy(current_room.projectiles);

   // Set player's coords & angle to angle of his room object
   r = GetRoomObjectById(player.id);
   if (r == NULL)
   {
      debug(("SetRoomInfo failed to find player in room objects!\n"));
   }
   else 
   {
      player.x = r->motion.x;
      player.y = r->motion.y;
      r->motion.z = GetFloorBase(player.x,player.y);
      player.angle = r->angle;

      // set initial 3d sound listener position
      SoundSetListenerPosition(player.x, player.y, player.angle);
   }

   // Set z coordinates of all objects
   for (l = current_room.contents; l != NULL; l = l->next)
   {
      r = (room_contents_node *) (l->data);
      RoomObjectSetHeight(r);
   }

   ModuleEvent(EVENT_NEWROOM);

   /* Now it's ok for user to do actions */
   SetGameDataValid(TRUE);

   GameSetState(GAME_PLAY);

   MoveSetValidity(True);

   /* Redraw room */
   RedrawAll();

   ModuleEvent(EVENT_USERCHANGED);
}
/************************************************************************/
void TurnObject(ID object_id, WORD angle)
{
   room_contents_node *r;
   r = GetRoomObjectById(object_id);

   if (r == NULL)
   {
      debug(("Couldn't find object #%d to turn\n", object_id));
      return;
   }

   /* Convert angle to NUMDEGREES = 360 degrees */
   r->angle = ANGLE_STOC(angle);

   if (object_id == player.id)
      player.angle = r->angle;

   RedrawAll();
}
/************************************************************************/
void CreateObject(room_contents_node *r)
{
   /* Add object to list of room contents */
   current_room.contents = list_add_first(current_room.contents, r);

   RoomObjectSetHeight(r);
   
   RedrawAll();
}
/************************************************************************/
void RemoveObject(ID obj_id)
{
   room_contents_node *r;

   r = GetRoomObjectById(obj_id);

   if (r == NULL)
   {
      debug(("Couldn't find object #%d to delete\n", obj_id));
      return;
   }

	//	If deleted object was user's selected target, clear target.
	if( GetUserTargetID() == obj_id )
		SetUserTargetID( INVALID_ID );

   /* Remove from list of things in room */
   current_room.contents = 
      list_delete_item(current_room.contents, (void *) obj_id, CompareIdRoomObject);

   RedrawAll();
   RoomObjectDestroy(r);
   SafeFree(r);
}
/************************************************************************/
/*
 * ChangeObject:  a gives new motion animation, overlays gives new motion overlays.
 *   translation gives palette translation when moving.
 */
void ChangeObject(object_node *new_obj, BYTE translation, BYTE effect, Animate *a, list_type overlays)
{
   room_contents_node *r;
   object_node *obj;
   Bool in_room = False;

   /* If animation off, discard animations */
   if (!VerifyAnimation(new_obj->animate))
   {
      ObjectDestroyAndFree(new_obj);
      list_destroy(overlays);
      return;
   }

   /* First look for object in room */
   r = GetRoomObjectById(new_obj->id);
   if (r != NULL)
   {
      RoomObjectDestroy(r);
      // XXX This is bad--should call ObjectCopy and redo allocation in server.c/HandleChange

	  // I'd like to second the notion that this is bad, wtf... - mistery

      memcpy(&r->obj, new_obj, sizeof(object_node));
      memcpy(&r->motion.animate, a, sizeof(Animate));
      r->motion.overlays = overlays;
      r->motion.move_animating = False;
      if (255 != translation)
	 r->motion.translation = translation;
      if (255 != effect)
         r->motion.effect = effect;

      r->obj.animate  = &r->obj.normal_animate;
      r->obj.overlays = &r->obj.normal_overlays;

      // If object was user's selected target, and target became invisible, clear selection.
      if(GetUserTargetID() == r->obj.id
            && (r->obj.drawingtype == DRAWFX_INVISIBLE))
         SetUserTargetID(INVALID_ID);

      // Object may have changed its effects such as OF_HANGING.
      RoomObjectSetHeight(r);

      in_room = True;
      RedrawAll();
   }

   /* Now check inventory */
   obj = (object_node *)
      list_find_item(player.inventory, (void *) new_obj->id, CompareIdObject);
   if (obj != NULL)
   {
      ObjectDestroy(obj);
      memcpy(obj, new_obj, sizeof(object_node));
      obj->animate  = &obj->normal_animate;
      obj->overlays = &obj->normal_overlays;
      ModuleEvent(EVENT_INVENTORY, INVENTORY_CHANGE, obj);
      // Ignore motion animation
   }

   // If object is player, tell modules that player changed
   if (new_obj->id == player.id)
      ModuleEvent(EVENT_USERCHANGED);

   if (!in_room)
      OverlayListDestroy(overlays);
}
/************************************************************************/


/************************************************************************/
void SetInventory(list_type inventory)
{
   ObjectListDestroy(player.inventory);

   player.inventory = inventory;
   ModuleEvent(EVENT_INVENTORY, INVENTORY_SET, NULL);
}
/************************************************************************/
void AddToInventory(object_node *obj)
{
   player.inventory = list_add_item(player.inventory, obj);
   ModuleEvent(EVENT_INVENTORY, INVENTORY_ADD, obj);
}
/************************************************************************/
void RemoveFromInventory(ID obj_id)
{
   object_node *node = (object_node *)
      list_find_item(player.inventory, (void *) obj_id, CompareIdObject);

   if (node == NULL)
   {
      debug(("Tried to remove nonexistent object #%d from inventory\n", obj_id));
      return;
   }

   ModuleEvent(EVENT_INVENTORY, INVENTORY_REMOVE, obj_id);
   player.inventory = list_delete_item(player.inventory, (void *) obj_id, CompareIdObject);
   ObjectDestroyAndFree(node);

   /* If we're using object, unuse it */
   UnuseObject(obj_id);
}
/************************************************************************/
void GameWait(void)
{
   GameSetState(GAME_WAIT);
}
/************************************************************************/
void GameUnwait(void)
{
   /* If our data is valid, continue playing */
   if (GameGetState() != GAME_INVALID)
      GameSetState(GAME_PLAY);
}
/************************************************************************/
/*
 * GamePlaySound: Play sound given by sound_rsc as if originating from object
 *   source_obj.  If source_obj is 0, use row & col if either or both are non-zero.
 *	 Otherwise just play sound as if originating from player.
 *	 For looping sounds, radius and max_vol are used to attenuate sound with distance
 *		default radius is VOLUME_CUTOFF_DISTANCE, default max_vol is MAX_VOLUME
 */
void GamePlaySound(ID sound_rsc, ID source_obj, BYTE flags, int y, int x, int radius, int max_vol)
{
   // source coordinates from object
   if (source_obj != 0)
   {
      room_contents_node *obj = GetRoomObjectById(source_obj);

      if (obj)
      {
         // values are in needed scale
         x = obj->motion.x;
         y = obj->motion.y;       
      }
   }
   // source coordinates from server (in big row/col)
   else if((x > 0) || (y > 0))	
   {
      x = ((x - 1) * FINENESS) + (FINENESS/2);
      y = ((y - 1) * FINENESS) + (FINENESS/2);
   }
   // 2d playback (= at avatar)
   else
   {
      x = 0;
      y = 0;
   }

   SoundPlayResource(sound_rsc, flags, x, y);
}
/************************************************************************/
void GameQuit(void)
{
   MainSetState(STATE_LOGIN);
}
/************************************************************************/
void GameDisplayResync(void)
{
   DisplayServerMessage(GetString(hInst, IDS_TRANSMITERROR), GetColor(COLOR_MAINEDITFGD), 0);
}
/************************************************************************/
void SetAmbientLight(BYTE l)
{
   current_room.ambient_light = l;
   LightChanged3D(player.light, current_room.ambient_light);
   RedrawAll();
}
/************************************************************************/
void SetPlayerLight(BYTE l)
{
   /* Keep light in range, just in case */
   player.light = l;
   LightChanged3D(player.light, current_room.ambient_light);
   RedrawAll();
}
/************************************************************************/
void SetBackground(ID bkgnd)
{
   current_room.bkgnd = bkgnd;
   NewBackground3D(bkgnd);
   RedrawAll();
}
/************************************************************************/
/*
 * ComputeObjectDistance:  Return distance between two given objects,
 *   in FINENESS units.
 *   XXX Would be nice to get rid of square root.
 */
int ComputeObjectDistance(room_contents_node *r1, room_contents_node *r2)
{
   int dx = (r1->motion.x - r2->motion.x) >> 4;
   int dy = (r1->motion.y - r2->motion.y) >> 4;
   return Distance(dx,dy) << 4;
}

player_info *GetPlayerInfo(void)
{
   return &player;
}

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dm.c:  DM ("Dungeon Master") DLL.
 */

#include "client.h"
#include "dm.h"

HINSTANCE hInst;              // Handle of this DLL

ClientInfo *cinfo;         // Holds data passed from main client
Bool        exiting;

/* local function prototypes */
static Bool HandleAdmin(char *ptr, long len);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_ADMIN,             HandleAdmin },
{ 0, NULL},
};

// Client message table
client_message msg_table[] = {
{ BP_REQ_DM,               { PARAM_BYTE, PARAM_STRING, PARAM_END }, },
{ 0,                       { PARAM_END }, },
};

static TypedCommand commands[] = {
{ "goroom",      CommandGoRoom, },
{ "goplayer",    CommandGoPlayer, },
{ "getplayer",   CommandGetPlayer, },
{ "gchannel",    CommandShowGChannel, },
{ "showmap",     CommandShowMap, },
{ "hidemap",     CommandHideMap, },
{ "reset",       CommandResetData, },
{ "dm",          CommandDM, },
{ "echo",        CommandEcho, },
{ NULL,          NULL},    // Must end table this way
};

// Key translation table
#define A_QEDITOR (A_MODULE + 900)
#define A_GCHANNEL (A_MODULE + 901)
#define A_LOOKINVENTORY (A_MODULE + 202)    // Examine item in inventory (from merintr.dll)

static keymap dm_key_table[] = {
//{ '5',          KEY_SHIFT,   A_QEDITOR },
{ '6',          KEY_SHIFT,   A_GCHANNEL },
{ 0, 0, 0},   // Must end table this way
};

/****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
      hInst = hModule;
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   info->event_mask = EVENT_TEXTCOMMAND | EVENT_USERACTION | EVENT_SERVERMSG;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;
   cinfo = client_info;    // Save client info for our use later
   exiting = False;

   KeyAddTable(GAME_PLAY, dm_key_table);
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   exiting = True;
   PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
}
/****************************************************************************/
/*
 * EVENT_SERVERMSG
 */
/****************************************************************************/
Bool WINAPI EventServerMessage(char *message, long len)
{
   Bool retval;

   retval = LookupMessage(message, len, handler_table);

   // If we handle message, don't pass it on to anyone else
   if (retval == True)
     return False;

   return True;    // Allow other modules to get other messages
}
/********************************************************************/
Bool HandleAdmin(char *ptr, long len)
{
   char message[COMBUFSIZE];
   WORD length;

   memcpy(&length, ptr, SIZE_STRING_LEN);
   ptr += SIZE_STRING_LEN;

   memcpy(message, ptr, length);
   message[length] = 0;

//   GameMessage(message);
   return True;
}
/****************************************************************************/
/*
 * EVENT_TEXTCOMMAND
 */
/****************************************************************************/
Bool WINAPI EventTextCommand(char *str)
{
   // Parse command, and don't pass it on if we handle it
   if (ParseCommand(str, commands))
      return False;

   return True;
}
/****************************************************************************/
/*
 * EVENT_USERACTION
 */
/****************************************************************************/
Bool WINAPI EventUserAction(int action, void *action_data)
{
   int x, y;
   list_type objects;
   char buf[MAX_ADMIN];
   object_node *obj;
   ID id;

   switch (action)
   {
   case A_QEDITOR:
      debug(("A_QEDITOR\n"));
      if (GetQEditorDlg() || GetGChannelDlg())
	 break;
      ShowQEditorDlg();
      break;

   case A_GCHANNEL:
      debug(("A_GCHANNEL\n"));
      if (GetQEditorDlg() || GetGChannelDlg())
	 break;
      ShowGChannelDlg();
      break;

   case A_LOOKMOUSE: /* user 'looks' on main window */
      if (!GetQEditorDlg() && !GetGChannelDlg())
	 return True;

      if (!MouseToRoom(&x, &y))
	 return True;

      objects = GetObjects3D(x, y, 0, 0, 0, 0, 0);
      if (objects == NULL)
	 return True;

      /* Get details of object */
      obj = (object_node *) (objects->data);
      if (!obj)
      {
	 debug(("clicked, but nothing\n"));
	 return True;
      }

      strcpy(buf, LookupNameRsc(obj->name_res));
      if (GetGChannelDlg() && (obj->flags & OF_PLAYER))
      {
	 debug(("gchannel needs to hear we clicked on '%s' (flags = 0x08X)\n", buf, obj->flags));
	 SendMessage(GetGChannelDlg(), DMDLGM_CLICKEDUSER, 0, (LPARAM)buf);
	 return False;
      }

      if (GetQEditorDlg())
      {
	 debug(("qeditor needs to hear we clicked on '%s'\n", buf));
	 SendMessage(GetQEditorDlg(), DMDLGM_CLICKEDUSER, 0, (LPARAM)buf);
	 return False;
      }

      break;

   case A_LOOKINVENTORY: /* user 'looks' on inventory window */
      if (!GetQEditorDlg() /* && !GetGChannelDlg() */)
	 return True;

      id = (ID)action_data;
      if (id == INVALID_ID)
	 return True;

      break;

   default: 
      return True;
   }

   return False;
}

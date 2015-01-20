// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admin.c:  DLL that allows user to enter admin commands while in game mode.
 */

#include "client.h"
#include "admin.h"

HINSTANCE hInst;                 // Handle of this DLL
extern Bool   hidden;         // True when dialog exists but is hidden

ClientInfo *c;         // Holds data passed from main client

Bool exiting;

/* local function prototypes */
static Bool HandleAdmin(char *ptr, long len);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_ADMIN,             HandleAdmin },
{ 0, NULL},
};

// Client message table
client_message admin_msg_table[] = {
{ BP_REQ_ADMIN,            { PARAM_STRING, PARAM_END }, },
{ 0,                       { PARAM_END }, },
};

// Key translation table
#define A_ADMIN (A_MODULE + 1)
#define A_LOOKINVENTORY (A_MODULE + 202)    // Examine item in inventory (from merintr.dll)

static keymap admin_key_table[] = {
{ '4',          KEY_SHIFT,   A_ADMIN },
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
   info->event_mask = EVENT_SERVERMSG | EVENT_USERACTION | EVENT_FONTCHANGED | EVENT_RESETDATA;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;
   c = client_info;    // Save client info for our use later
   exiting = False;

   KeyAddTable(GAME_PLAY, admin_key_table);
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   exiting = True;

   KeyRemoveTable(GAME_PLAY, admin_key_table);

   if (hAdminDlg != NULL)
      DestroyWindow(hAdminDlg);
   else PostMessage(c->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
}

/* event handlers */
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

   if (hAdminDlg != NULL)
      SendMessage(hAdminDlg, BK_GOTTEXT, 0, (LPARAM) message);
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
   case A_ADMIN:
      if (hAdminDlg == NULL)
	 CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADMIN), c->hMain, AdminDialogProc);
      else 
      {
	 ShowWindow(hAdminDlg, SW_SHOWNORMAL);
	 SetFocus(hAdminDlg);
      }
      hidden = False;

      break;

   case A_LOOKMOUSE:
      /* We're looking for a user clicking on main window */
      if (hAdminDlg == NULL || hidden || !MouseToRoom(&x, &y))
	 return True;
      
      objects = GetObjects3D(x, y, 0, 0, 0, 0, 0);
      if (objects == NULL)
	 return True;

      obj = (object_node *) (objects->data);
      
      /* Get details of object */
      sprintf(buf, "show object %d", GetObjId(obj->id));
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) buf);
      break;

   case A_LOOKINVENTORY:
      // Examining inventory object
      id = (ID) action_data;
      if (hAdminDlg == NULL || hidden || id == INVALID_ID)
	 return True;

      /* Get details of object */
      sprintf(buf, "show object %d", GetObjId(id));
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) buf);      
      break;

   default: 
      return True;
   }
   return False;
}
/****************************************************************************/
/*
 * EVENT_FONTCHANGED
 */
/****************************************************************************/
Bool WINAPI EventFontChanged(WORD font_id, LOGFONT *font)
{
   if (hAdminDlg != NULL)
      SendMessage(hAdminDlg, BK_SETDLGFONTS, 0, 0);
   return True;
}
/****************************************************************************/
/*
 * EVENT_RESETDATA
 */
/****************************************************************************/
Bool WINAPI EventResetData(void)
{
   // Clear all object info from dialog, since these might have changed
   if (hAdminDlg != NULL)
      SendMessage(hAdminDlg, BK_RESETDATA, 0, 0);
   return True;
}


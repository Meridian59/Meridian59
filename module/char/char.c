// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charmain.c:  DLL that allows user to select a character and modify it.
 *   This DLL is invoked every time the user enters the game.
 */

#include "client.h"
#include "char.h"

HINSTANCE hInst;            // Handle of this DLL

ClientInfo *cinfo;         // Holds data passed from main client
Bool        exiting;

static Bool HandleCharacters(char *ptr,long len);
static Bool HandleCharInfo(char *ptr,long len);
static Bool HandleCharInfoOk(char *ptr, long len);
static Bool HandleCharInfoNotOk(char *ptr, long len);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_CHARACTERS,        HandleCharacters },
{ BP_CHARINFO,          HandleCharInfo },
{ BP_CHARINFO_OK,       HandleCharInfoOk },
{ BP_CHARINFO_NOT_OK,   HandleCharInfoNotOk },
{ 0, NULL},
};

// Client message table
client_message msg_table[] = {
{ BP_SEND_CHARACTERS,      { PARAM_END }, },
{ BP_USE_CHARACTER,        { PARAM_ID, PARAM_END }, },
{ BP_NEW_CHARINFO,         { PARAM_ID, PARAM_STRING, PARAM_STRING, PARAM_BYTE, PARAM_INT_ARRAY, 
			     PARAM_BYTE, PARAM_BYTE,
			     PARAM_INT_ARRAY, PARAM_ID_LIST, PARAM_ID_LIST, PARAM_END }, },
{ BP_SEND_CHARINFO,        { PARAM_END }, },
{ BP_AD_SELECTED,          { PARAM_BYTE, PARAM_END }, },
{ 0,                       { PARAM_END }, },
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
//      AbortCharDialogs();
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   info->event_mask = EVENT_SERVERMSG | EVENT_STATECHANGED;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;
   cinfo = client_info;    // Save client info for our use later

   exiting = False;
   // Ask server for characters to pick from
   RequestCharacters();
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
Bool HandleCharacters(char *ptr, long len)
{
   char *start = ptr;
   WORD num_characters, i, string_len;
   Character *characters;
   char message[MAXMESSAGE + 1];
   Advertisement ads[MAX_ADVERTISEMENTS];
   BYTE num_ads;

   Extract(&ptr, &num_characters, SIZE_LIST_LEN);

   characters = (Character *) SafeMalloc(num_characters * sizeof(Character));

   for (i=0; i < num_characters; i++)
   {
      Character *c = &characters[i];

      Extract(&ptr, &c->id, SIZE_ID);
      Extract(&ptr, &string_len, SIZE_STRING_LEN);
      string_len = min(string_len, MAXNAME);
      c->name = (char *) SafeMalloc(string_len + 1);
      Extract(&ptr, c->name, string_len);
      c->name[string_len] = 0;
      Extract(&ptr, &c->flags, 1);
   }

   ExtractString(&ptr, len, message, MAXMESSAGE);

   // Get info for advertisements
   Extract(&ptr, &num_ads, 1);
   if (num_ads > MAX_ADVERTISEMENTS)
   {
      CharactersDestroy(characters, num_characters);
      return False;
   }

   for (i=0; i < num_ads; i++)
   {
      ExtractString(&ptr, len, ads[i].filename, MAX_PATH);
      ExtractString(&ptr, len, ads[i].url, MAX_PATH);
   }

   len -= (ptr - start);
   if (len != 0)
   {
      CharactersDestroy(characters, num_characters);
      return False;
   }   

   ChooseCharacter(characters, num_characters, message, num_ads, ads, cinfo->config->quickstart);
   return True;
}
/********************************************************************/
Bool HandleCharInfo(char *ptr, long len)
{
   CharAppearance *ap;
   char *start = ptr;
   int i, num, gender;
   list_type spells, skills;
   BYTE byte;

   // *** Read face info
   ap = (CharAppearance *) SafeMalloc(sizeof(CharAppearance));

   // Palette translations
   Extract(&ptr, &byte, 1);
   ap->num_hair_translations = byte;
   ap->hair_translations = (BYTE *) SafeMalloc(byte * sizeof(BYTE));
   for (i=0; i < byte; i++)
      Extract(&ptr, &ap->hair_translations[i], 1);

   Extract(&ptr, &byte, 1);
   ap->num_face_translations = byte;
   ap->face_translations = (BYTE *) SafeMalloc(byte * sizeof(BYTE));
   for (i=0; i < byte; i++)
      Extract(&ptr, &ap->face_translations[i], 1);

   for (gender = 0; gender < NUM_GENDERS; gender++)
   {
      FaceInfo *info = &ap->parts[gender];
      // Hair
      Extract(&ptr, &num, 4);
      info->num_hair = num;
      info->hair = (ID *) SafeMalloc(num * sizeof(ID));
      for (i=0; i < num; i++)
	 Extract(&ptr, &info->hair[i], SIZE_ID);

      Extract(&ptr, &info->head, SIZE_ID);
	
      Extract(&ptr, &info->num_eyes, 4);
      info->eyes = (ID *) SafeMalloc(info->num_eyes * sizeof(ID));
      for (i=0; i < info->num_eyes; i++)
	 Extract(&ptr, &info->eyes[i], SIZE_ID);
      
      Extract(&ptr, &info->num_noses, 4);
      info->noses = (ID *) SafeMalloc(info->num_noses * sizeof(ID));
      for (i=0; i < info->num_noses; i++)
	 Extract(&ptr, &info->noses[i], SIZE_ID);
      
      Extract(&ptr, &info->num_mouths, 4);
      info->mouths = (ID *) SafeMalloc(info->num_mouths * sizeof(ID));
      for (i=0; i < info->num_mouths; i++)
	 Extract(&ptr, &info->mouths[i], SIZE_ID);
   }

   // *** Read spell info
   spells = NULL;
   Extract(&ptr, &num, 4);
   for (i=0; i < num; i++)
   {
      Spell *s = (Spell *) SafeMalloc(sizeof(Spell));
      
      Extract(&ptr, &s->id, 4);
      Extract(&ptr, &s->name_res, SIZE_ID);
      Extract(&ptr, &s->desc_res, SIZE_ID);
      Extract(&ptr, &s->cost, 4);
      Extract(&ptr, &s->school, 1);
      s->chosen = False;

      spells = list_add_item(spells, s);
   }

   // *** Read skill info
   skills = NULL;
   Extract(&ptr, &num, 4);
   for (i=0; i < num; i++)
   {
      Skill *s = (Skill *) SafeMalloc(sizeof(Skill));
      
      Extract(&ptr, &s->id, 4);
      Extract(&ptr, &s->name_res, SIZE_ID);
      Extract(&ptr, &s->desc_res, SIZE_ID);
      Extract(&ptr, &s->cost, 4);
      s->chosen = False;

      skills = list_add_item(skills, s);
   }
   
   len -= (ptr - start);
   if (len != 0)
   {
      CharAppearanceDestroy(ap);
      list_destroy(spells);
      list_destroy(skills);
      return False;
   }
   
   MakeChar(ap, spells, skills);
   return True;
}
/********************************************************************/
Bool HandleCharInfoOk(char *ptr, long len)
{
   extern ID char_to_use;
   ID char_returned = 0;

   Extract(&ptr, &char_returned, SIZE_ID);

   debug(("HandleCharInfoOk got len of %i and char_returned of %i\n",len,(char_returned & 0x0FFFFFFF)));

   len -= SIZE_ID;
   if (len != 0)
      return False;

   char_to_use = char_returned;
   CharInfoValid();
   return True;
}
/********************************************************************/
Bool HandleCharInfoNotOk(char *ptr, long len)
{
   if (len != 0)
      return False;
   CharInfoInvalid();
   return True;
}
/****************************************************************************/
/*
 * EVENT_STATECHANGED
 */
/****************************************************************************/
Bool WINAPI EventStateChanged(int old_state, int new_state)
{
   if (old_state == GAME_PICKCHAR && new_state != GAME_PICKCHAR)
   {
      // Force redraw of main window, to get rid of dialogs
      UpdateWindow(cinfo->hMain);
      AbortCharDialogs();
   }
   return True;
}

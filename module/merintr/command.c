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
#include "merintr.h"

// Constants for user actions (wave, etc.)

enum { 
   UA_NORMAL = 1,
   UA_HAPPY  = 2,
   UA_SAD    = 3,
   UA_WRY    = 4,
   UA_WAVE   = 8, 
   UA_POINT  = 9,
   UA_DANCE  = 10,
};

static HWND hSuicideDialog;

/************************************************************************/
/*
 * CommandSay: "say" command
 */
void CommandSay(char *args)
{
   if (*args == 0)
      return;
   if (!FilterSayMessage(args))
      return;
   SendSay(SAY_NORMAL, args);
}
/************************************************************************/
/*
 * CommandEmote: "emote" command
 */
void CommandEmote(char *args)
{
   if (*args == 0)
      return;
   if (!FilterSayMessage(args))
      return;
   SendSay(SAY_EMOTE, args);
}
/************************************************************************/
/*
 * CommandYell: "yell" command
 */
void CommandYell(char *args)
{
   if (*args == 0)
      return;
   if (!FilterSayMessage(args))
      return;
   SendSay(SAY_YELL, args);
}
/************************************************************************/
/*
 * CommandBroadcast: "broadcast" command
 */
void CommandBroadcast(char *args)
{
   if (*args == 0)
      return;
   if (!FilterSayMessage(args))
      return;
   SendSay(SAY_EVERYONE, args);
}
/************************************************************************/
/*
 * CommandQuit: "quit" command
 */
void CommandQuit(char *args)
{
   PerformAction(A_QUIT, NULL);
}
/************************************************************************/
/*
 * CommandHel: Used to prevent accidental activation of help page
 */
void CommandHel(char *args)
{
   GameMessage(GetString(hInst, IDS_HEL));
}
/************************************************************************/
/*
 * CommandHelp: "help" command
 */
void CommandHelp(char *args)
{
   PerformAction(A_HELP, NULL);
}
/************************************************************************/
/*
 * CommandTell: "tell" command
 */
void CommandTell(char *args)
{
   char *name;  // Name of person we're telling to
   ID player;   // Object id of person we're telling to
   char *message;
   IDList say_group;
   int index;

   name = GetPlayerName(args, &message);
   if (name == NULL || message == NULL)
   {
      debug(("Missing argument to tell\n"));
      return;
   }

   // First check for exact player match
   player = FindPlayerByNameExact(name);
   if (player == 0)
   {
      // If no exact player match, check for exact group match
      index = FindGroupByName(name);
      if (GroupNameMatches(name, index))
	 if (TellGroup(name, message))
	    return;
   }

   // Look for a player name
   player = FindPlayerByName(name);
   if (player != 0 && player != INVALID_ID)
   {
      if (!FilterSayMessage(message))
	 return;
      
      say_group = IDListAdd(NULL, player);
      SendSayGroup(say_group, message);
      IDListDelete(say_group);
      return;
   }

   // Finally, look for a group name
   if (TellGroup(name, message))
      return;

   if (player == 0)
   {
      GameMessage(GetString(hInst, IDS_NOTELLNAME));
      return;
   }

   if (player == INVALID_ID)
   {
      GameMessage(GetString(hInst, IDS_DUPLICATETELLNAME));
      return;
   }
}
/************************************************************************/
/*
 * CommandWho: "who" command
 */
void CommandWho(char *args)
{
   PerformAction(A_WHO, NULL);
}
/************************************************************************/
/*
 * CommandGroupAdd: "addgroup" command; adds names to user group
 */
void CommandGroupAdd(char *args)
{
   char *group_name, *players;
   int num_added;

   group_name = GetPlayerName(args, &players);
   if (group_name == NULL)
   {
      GroupsPrint();
      return;
   }

   if (players == NULL)
   {
      GroupPrint(group_name);
      return;
   }

   num_added = GroupAdd(group_name, players);
   if (num_added != 0)
      GameMessagePrintf(GetString(hInst, IDS_GROUPADDED), num_added);
}
/************************************************************************/
/*
 * CommandGroupDelete: "delgroup" command; remove names from user group
 */
void CommandGroupDelete(char *args)
{
   char *group_name, *players;
   int num_deleted;

   group_name = GetPlayerName(args, &players);
   if (group_name == NULL)
   {
      GroupsPrint();
      return;
   }

   if (players == NULL)
   {
      if (GroupDelete(group_name))
	 GameMessage(GetString(hInst, IDS_GROUPDELETED));
      return;
   }

   num_deleted = GroupDeleteNames(group_name, players);
   if (num_deleted == -1)
      GameMessage(GetString(hInst, IDS_GROUPDELETED));
   else GameMessagePrintf(GetString(hInst, IDS_NAMESDELETED), num_deleted);

}
/************************************************************************/
/*
 * CommandGroupNew: "newgroup" command; create a new group.
 */
void CommandGroupNew(char *args)
{
   char *group_name; 

   group_name = GetPlayerName(args, NULL);
   if (group_name == NULL)
   {
      GroupsPrint();
      return;
   }
   
   if (GroupNew(group_name))
      GameMessage(GetString(hInst, IDS_GROUPCREATED));
}
/************************************************************************/
/*
 * CommandBuy: "buy" command; try to buy items.
 */
void CommandBuy(char *args)
{
   PerformAction(A_BUY, NULL);
}
/************************************************************************/
/*
 * CommandDrop: "drop" command
 */
void CommandDrop(char *args)
{
   PerformAction(A_DROP, NULL);
}
/************************************************************************/
/*
 * CommandGet: "get" command; try to pick up visible items.
 */
void CommandGet(char *args)
{
   PerformAction(A_PICKUP, NULL);
}
/************************************************************************/
/*
 * CommandLook: "look" command; examine a visible item.
 */
void CommandLook(char *args)
{
   PerformAction(A_LOOK, NULL);
}
/************************************************************************/
/*
 * CommandOffer: "offer" command
 */
void CommandOffer(char *args)
{
   PerformAction(A_OFFER, NULL);
}
/************************************************************************/
/*
 * CommandCast: "cast" command; find spell name and cast it
 */
void CommandCast(char *args)
{
   spell *sp;
   char *spell_name;

   spell_name = GetSpellName(args, NULL);
   if (spell_name == NULL)
   {
      PerformAction(A_CAST, NULL);
      return;
   }

   sp = FindSpellByName(spell_name);
   if (sp == SPELL_NOMATCH)
   {
      GameMessage(GetString(hInst, IDS_NOSPELLNAME));
      return;
   }

   if (sp == SPELL_AMBIGUOUS)
   {
      GameMessage(GetString(hInst, IDS_DUPLICATESPELLNAME));
      return;
   }

   PerformAction(A_CASTSPELL, sp);
}

/************************************************************************/
/*
 * TellGroup:  Send message to a group of people.
 *   Return True iff the name was matched to a group name.
 */
Bool TellGroup(char *name, char *message)
{
   UserGroup g;
   int i;
   IDList say_group;
   ID id;

   switch (GroupLoad(name, &g))
   {
   case GROUP_NOMATCH:
      return False;

   case GROUP_AMBIGUOUS:
      GameMessage(GetString(hInst, IDS_DUPLICATEGROUPNAME));
      return True;
   }
   
   say_group = NULL;
   for (i=0; i < g.num_users; i++)
   {
      id = FindPlayerByNameExact(g.names[i]);
      if (id == 0)
	 continue;

      if (id == INVALID_ID)
      {
	 debug(("%s is ambiguous in group tell\n", g.names[i]));
	 continue;
      }
      
      // Add to list, removing duplicates
      if (!IDListMember(say_group, id))
	 say_group = IDListAdd(say_group, id);
   }

   if (say_group == NULL)
   {
      GameMessage(GetString(hInst, IDS_GROUPNOTON));
      return True;
   }
  
   if (!FilterSayMessage(message))
      return True;

   SendSayGroup(say_group, message);
   IDListDelete(say_group);
   return True;
}
/************************************************************************/
/*
 * CommandWave: "wave" command
 */
void CommandWave(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_WAVE);
}
/************************************************************************/
/*
 * CommandPoint: "point" command
 */
void CommandPoint(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_POINT);
}
/************************************************************************/
/*
 * CommandDance: "dance" command
 */
void CommandDance(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_DANCE);
}
/************************************************************************/
/*
 * CommandRest: "rest" command
 */
void CommandRest(char *args)
{
   if (pinfo.resting)
   {
      GameMessage(GetString(hInst, IDS_RESTING));
      return;
   }

   RequestRest();
   GameMessage(GetString(hInst, IDS_REST));
   pinfo.resting = True;

   ToolbarSetButtonState(default_buttons[REST_INDEX].action, default_buttons[REST_INDEX].data, True);
}
/************************************************************************/
/*
 * CommandStand: "stand" command
 */
void CommandStand(char *args)
{
   if (!pinfo.resting)
   {
      GameMessage(GetString(hInst, IDS_STANDING));
      return;
   }

   RequestStand();
   GameMessage(GetString(hInst, IDS_STAND));
   pinfo.resting = False;

   ToolbarSetButtonState(default_buttons[REST_INDEX].action, default_buttons[REST_INDEX].data, False);
}
/************************************************************************/
/*
 * CommandSuicid: "suicid" command (used to prevent accidental suicides).
 */
void CommandSuicid(char *args)
{
   GameMessage(GetString(hInst, IDS_SUICID));
}
/************************************************************************/
/*
 * CommandSuicide: "suicide" command
 */
BOOL CALLBACK SuicideVerifyDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   HWND hEdit;
   char achPassword[MAXPASSWORD+1];

   switch (message)
   {
   case WM_INITDIALOG:
      hEdit = GetDlgItem(hDlg, IDC_ANNOTATE);
      Edit_LimitText(hEdit, MAXPASSWORD + 1);

      CenterWindow(hDlg, GetParent(hDlg));

      hSuicideDialog = hDlg;
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 hEdit = GetDlgItem(hDlg, IDC_SUICIDE_PASSWORD);
	 Edit_GetText(hEdit, achPassword, MAXPASSWORD + 1);
	 if (cinfo && cinfo->config &&
	     0 == strcmp(cinfo->config->password, achPassword))
	 {
	    EndDialog(hDlg, IDOK);
	 }
	 else
	 {
	    EndDialog(hDlg, IDCANCEL);
	 }
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hSuicideDialog = NULL;
      return TRUE;
   }

   return FALSE;
}
void CommandSuicide(char *args)
{
   if (IDOK == DialogBox(hInst, MAKEINTRESOURCE(IDD_SUICIDE),
                         cinfo->hMain, SuicideVerifyDialogProc))
   {
      RequestSuicide();
   }
}
/************************************************************************/
/*
 * CommandNeutral: "neutral" command
 */
void CommandNeutral(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_NORMAL);
}
/************************************************************************/
/*
 * CommandHappy: "happy" command
 */
void CommandHappy(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_HAPPY);
}
/************************************************************************/
/*
 * CommandSad: "sad" command
 */
void CommandSad(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_SAD);
}
/************************************************************************/
/*
 * CommandWry: "wry" command
 */
void CommandWry(char *args)
{
   PerformAction(A_USERACTION, (void *) UA_WRY);
}
/************************************************************************/
/*
 * CommandMap: "map" command
 */
void CommandMap(char *args)
{
   PerformAction(A_MAP, NULL);
}
/************************************************************************/
/*
 * CommandPut: "put" command
 */
void CommandPut(char *args)
{
   PerformAction(A_PUT, NULL);
}
/************************************************************************/
/*
 * CommandActivate: "use" command
 */
void CommandActivate(char *args)
{
   PerformAction(A_ACTIVATE, NULL);
}
/************************************************************************/
/*
 * SendSafetyOn: "safetyOn" command
 */
void CommandSafetyOn(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_SAFETY_OFF);
}
/************************************************************************/
/*
 * SendSafetyOff: "safetyoff" command
 */
void CommandSafetyOff(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_SAFETY_OFF);
}
/************************************************************************/
/*
 * SendTempSafeOn: "tempSafeOn" command
 */
void CommandTempSafeOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_TEMPSAFE);
}
/************************************************************************/
/*
 * SendTempSafeOff: "tempSafeoff" command
 */
void CommandTempSafeOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_TEMPSAFE);
}
/************************************************************************/
/*
 * SendGroupingOn: "groupingon" command
 */
void CommandGroupingOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_GROUPING);
}
/************************************************************************/
/*
 * SendGroupingOff: "groupingoff" command
 */
void CommandGroupingOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_GROUPING);
}
/************************************************************************/
/*
 * SendAutoLootOn: "autoloot on" command
 */
void CommandAutoLootOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_AUTOLOOT);
}
/************************************************************************/
/*
 * SendAutoLootOff: "autoloot off" command
 */
void CommandAutoLootOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_AUTOLOOT);
}
/************************************************************************/
/*
 * SendAutoCombineOn: "autocombine on" command
 */
void CommandAutoCombineOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_AUTOCOMBINE);
}
/************************************************************************/
/*
 * SendAutoCombineOff: "autocombine off" command
 */
void CommandAutoCombineOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_AUTOCOMBINE);
}
/************************************************************************/
/*
 * SendReagentBagOn: "reagentbag on" command
 */
void CommandReagentBagOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_REAGENTBAG);
}
/************************************************************************/
/*
 * SendReagentBagOff: "reagentbag off" command
 */
void CommandReagentBagOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_REAGENTBAG);
}
/************************************************************************/
/*
 * SendReagentBagOn: "spellpower on" command
 */
void CommandSpellPowerOn(char *args)
{
   SendPreferences(cinfo->config->preferences |= CF_SPELLPOWER);
}
/************************************************************************/
/*
 * SendReagentBagOff: "spellpower off" command
 */
void CommandSpellPowerOff(char *args)
{
   SendPreferences(cinfo->config->preferences &= ~CF_SPELLPOWER);
}
/************************************************************************/
/*
 * CommandGuild: "guild" command
 */
void CommandGuild(char *args)
{
   RequestGuildInfo();
}
/************************************************************************/
/*
 * CommandGuild: "guild" command
 */
void CommandPassword(char *args)
{
   PerformAction(A_CHANGEPASSWORD, NULL);
}
/************************************************************************/
/*
 * CommandBalance: "balance" command
 */
void CommandBalance(char *args)
{
   RequestBalance();
}
/************************************************************************/
/*
 * CommandWithdraw: "withdraw" command
 */
void CommandWithdraw(char *args)
{
   int amount;

   amount = atoi(args);

   if (amount > 0)
   {
      RequestWithdrawMoney(amount);
   }
   else
   {
      PerformAction(A_WITHDRAW, NULL);
   }
}
/************************************************************************/
/*
 * CommandDeposit: "deposit" command
 */
void CommandDeposit(char *args)
{
   int amount;

   amount = atoi(args);
   if (amount > 0)
   {
      RequestDepositMoney(amount);
   }
   else
   {
      PerformAction(A_DEPOSIT, NULL);
   }
}
/************************************************************************/
/*
 * CommandGroup: "group" command
 */
void CommandGroup(char *args)
{
   GroupConfigure();
}
/************************************************************************/
/*
 * CommandAppeal: "appeal" command
 */
void CommandAppeal(char *args)
{
   if (!FilterSayMessage(args))
      return;
   RequestAppeal(args);
}
/************************************************************************/
/*
 * CommandTellGuild: "tellguild" command
 */
void CommandTellGuild(char *args)
{
   if (*args == 0)
      return;
   if (!FilterSayMessage(args))
      return;
   SendSay(SAY_GUILD, args);
}

/************************************************************************/
/*
 * CommandTime: "time" command
 */
void CommandTime(char *args)
{
   RequestTime();
}
/************************************************************************/
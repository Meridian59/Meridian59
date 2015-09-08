// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mermain.c:  Parts of Meridian interface on the main window
 */

#include "client.h"
#include "merintr.h"

// Buttons to add to toolbar by default
// If you change this table, you MUST check the indices defined in mermain.h
AddButton default_buttons[] = {
{ A_TEXTCOMMAND, "help", A_NOACTION,     NULL,   NULL, IDB_HELP,   IDS_TBHELP, },
{ A_TEXTCOMMAND, "drop", A_NOACTION,     NULL,   NULL, IDB_DROP,   IDS_TBDROP, },
{ A_TEXTCOMMAND, "get",  A_NOACTION,     NULL,   NULL, IDB_GET,    IDS_TBGET, },
{ A_TEXTCOMMAND, "rest", A_TEXTCOMMAND, "stand", NULL, IDB_REST,   IDS_TBREST, },
{ 0,             NULL,   0,              NULL,   NULL, 0,          0, },           // Separator
};

static int num_default_buttons = (sizeof(default_buttons) / sizeof(AddButton));

// Player/game data that's known only in module
PInfo pinfo;

// These booleans prevent multiple requests to change data being sent to server.
// True when we've told server to change safety flag
static Bool safety_flipped;
// True when we've told server to change temp safe flag
static Bool temp_safe_flipped;
// True when we've told server to change player's grouping flag
static Bool grouping_flipped;
// True when we've told server to change player's reagent bag setting
static Bool reagentbag_flipped;
// True when we've told server to change player's autoloot setting
static Bool autoloot_flipped;
// True when we've told server to change player's autocombine setting
static Bool autocombine_flipped;
// True when we've told server to change player's spellpower display setting
static Bool spellpower_flipped;

/****************************************************************************/
/*
 * InterfaceInit:  Called on startup.  Initialize interface.
 */
void InterfaceInit(void)
{
   int i;
   RECT r;
   AREA area;

   for (i = 0; i < num_default_buttons; i++)
   {
      default_buttons[i].hModule = hInst;
      ToolbarAddButton(&default_buttons[i]);
   }
   RequestStatGroups();
   RequestSkills();
   RequestSpells();

   SpellsInit();
   ActionsInit();
   LanguageInit();
   InventoryBoxCreate(cinfo->hMain);
   StatsCreate(cinfo->hMain);
   GroupsLoad();
   InterfaceDrawInit();

   UserAreaCreate();
   AliasInit();
   CmdAliasInit();
   EnchantmentsInit();

   GetClientRect(cinfo->hMain, &r);
   CopyCurrentView(&area);
   InterfaceResizeModule(r.right, r.bottom, &area);

   pinfo.resting = False;
   safety_flipped = False;
   temp_safe_flipped = False;
   grouping_flipped = False;
   autoloot_flipped = False;
   autocombine_flipped = False;
   reagentbag_flipped = False;
   spellpower_flipped = False;

   if (pinfo.resting)
      RequestRest();
   else
      RequestStand();
   if (cinfo->config->aggressive)
      SendSafety(0);
   else
      SendSafety(1);
   if (cinfo->config->tempsafe)
      SendTempSafe(1);
   else
      SendTempSafe(0);
   if (cinfo->config->grouping)
      SendGrouping(1);
   else
      SendGrouping(0);
   if (cinfo->config->autoloot)
      SendAutoLoot(1);
   else
      SendAutoLoot(0);
   if (cinfo->config->autocombine)
      SendAutoCombine(1);
   else
      SendAutoCombine(0);
   if (cinfo->config->reagentbag)
      SendReagentBag(1);
   else
      SendReagentBag(0);
   if (cinfo->config->spellpower)
      SendSpellPower(1);
   else
      SendSpellPower(0);
}
/****************************************************************************/
/*
 * InterfaceExit:  Called when leaving game; delete interface.
 */
void InterfaceExit(void)
{
   LanguageExit();
   AliasExit();
   ActionsExit();
   SpellsExit();
   InventoryBoxDestroy();
   UserAreaDestroy();
   StatsMainDestroy();  // must get rid of main stat windows before clearing all stats
   StatsDestroy();
   EnchantmentsDestroy();
   AbortGuildDialogs();
   AbortGroupDialog();
   InterfaceDrawExit();

   // Restore window title to normal
   SetWindowText(cinfo->hMain, cinfo->szAppName);
}
/****************************************************************************/
void InterfaceFontChanged(WORD font_id, LOGFONT *font)
{
   InventoryResetFont();
   StatsResetFont();
}
/****************************************************************************/
void InterfaceColorChanged(WORD color_id, COLORREF color)
{
   InventoryChangeColor();   
   StatsChangeColor();
   StatsMainChangeColor();
}
/****************************************************************************/
/*
 * InterfaceResizeModule:  Called when main window resized.
 */
void InterfaceResizeModule(int xsize, int ysize, AREA *view)
{
   InventoryBoxResize(xsize, ysize, view);
   UserAreaResize(xsize, ysize, view);   
   StatsResize(xsize, ysize, view);
   StatsMainResize(xsize, ysize, view);
   EnchantmentsResize(xsize, ysize, view);
   InterfaceDrawResize(xsize, ysize, view);
}
/****************************************************************************/
/*
 * InterfaceRedrawModule:  Called when main window needs to be redrawn.
 */
void InterfaceRedrawModule(HDC hdc)
{
	UserAreaRedraw();
  InterfaceDrawElements(hdc);
  StatsDrawBorder();
  StatsMainRedraw();
  StatsDraw();
  if( StatsGetCurrentGroup() == STATS_INVENTORY )
  {
    InvalidateRect( GetHwndInv(), NULL, FALSE );
    ShowInventory(TRUE);
    InventoryRedraw();
  }
}
/****************************************************************************/
Bool InterfaceDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   switch (lpdis->CtlID)
   {
   case IDC_USERAREA:
      UserAreaRedraw();
      return False;

   case IDC_STATS:
      StatsDrawNumItem(hwnd, lpdis);
      return False;

   case IDC_ENCHANTMENT:
      EnchantmentDrawItem(hwnd, lpdis);
      return False;
   }
   return True;
}
/****************************************************************************/
void InterfaceResetData(void)
{
   if (pinfo.resting)
      RequestRest();
   else
      RequestStand();
   if (cinfo->config->aggressive)
      SendSafety(0);
   else
      SendSafety(1);
   if (cinfo->config->tempsafe)
      SendTempSafe(1);
   else
      SendTempSafe(0);

   RequestStatGroups();
   RequestSpells();
   RequestSkills();

   InventoryResetData();
   EnchantmentsResetData();
   GuildResetData();
}
/****************************************************************************/
void InterfaceNewRoom(void)
{
   char buf[MAXRSCSTRING];

   EnchantmentsNewRoom();

   // Set main window title to show room name
   sprintf(buf, GetString(hInst, IDS_WINDOWTITLE), LookupNameRsc(cinfo->player->room_name_res));
   SetWindowText(cinfo->hMain, buf);
}
/****************************************************************************/
/*
 * InterfaceTab:  User is moving focus from given control in given direction.
 *   Return False iff event should not be sent to other modules (i.e. handled here)
 */
Bool InterfaceTab(int control, Bool forward)
{
   switch (control)
   {
   case IDC_MAIN:
      if (!forward)
	 break;

		if( StatsGetCurrentGroup() == STATS_INVENTORY )		//	ajw
			InventorySetFocus( forward );
		else
			StatsSetFocus( forward );
      return False;

   case IDC_MAINTEXT:
   case IDC_TEXTINPUT:
      if (forward)
	 break;
      
		if( StatsGetCurrentGroup() == STATS_INVENTORY )		//	ajw
			InventorySetFocus( forward );
		else
			StatsSetFocus( forward );

      return False;
      
   }
   return True;
}
/****************************************************************************/
/*
 * InterfaceAction:  User wants to perform given action.  Return True iff
 *   action should be passed along for further processing.
 */
Bool InterfaceAction(int action, void *action_data)
{
   POINT mouse;
   AREA a;
   room_contents_node *r;
   static int lastRestWarning = 0;

   if ((pinfo.resting || cinfo->effects->paralyzed) && 
       (IsMoveAction(action) || IsAttackAction(action)))
   {
      //      debug(("Can't do while resting\n"));
      return False;
   }

   switch (action)
   {
   case A_TARGETCLEAR:
   case A_TARGETSELF:
	case A_TARGETNEXT:
	case A_TARGETPREVIOUS:
	case A_ATTACK:
	   UserAreaRedraw();
	   return True;
   break;

   case A_FORWARDFAST:
      // If too tired, don't run
      if (pinfo.vigor < MIN_VIGOR)
      {
	 PerformAction(A_FORWARD, NULL);
	 return False;
      }
      break;

   case A_BACKWARDFAST:
      if (pinfo.vigor < MIN_VIGOR)
      {
	 PerformAction(A_BACKWARD, NULL);
	 return False;
      }
      break;

	case A_SLIDELEFTFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDELEFT, NULL);
			return False;
		}
	break;

	case A_SLIDERIGHTFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDERIGHT, NULL);
			return False;
		}
	break;

	case A_SLIDELEFTFORWARDFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDELEFTFORWARD, NULL);
			return False;
		}
	break;

	case A_SLIDELEFTBACKWARDFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDELEFTBACKWARD, NULL);
			return False;
		}
	break;

	case A_SLIDERIGHTFORWARDFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDERIGHTFORWARD, NULL);
			return False;
		}
	break;

	case A_SLIDERIGHTBACKWARDFAST:
		if (pinfo.vigor < MIN_VIGOR)
		{
			PerformAction(A_SLIDERIGHTBACKWARD, NULL);
			return False;
		}
	break;

   case A_CAST:
      UserCastSpell();
      return False;

   case A_CASTSPELL:  // action_data is pointer to spell
      if (CheckForAlwaysActiveSpells((spelltemp *) action_data))
      {
         SpellCast((spell *) action_data);
         return False;
      }

       if (GetPlayer()->viewID && (GetPlayer()->viewID != GetPlayer()->id))
      {
	 if (!(GetPlayer()->viewFlags & REMOTE_VIEW_CAST))
	 {
	    GameMessage(GetString(hInst, IDS_SPELLPARALYZED));
	    return False;
	 }
      }
      if (cinfo->effects->paralyzed)
      {
	 GameMessage(GetString(hInst, IDS_SPELLPARALYZED));
	 return False;
      }

      if (pinfo.resting)
      {
	 GameMessage(GetString(hInst, IDS_SPELLRESTING));
	 return False;
      }

      SpellCast((spell *) action_data);

      return False;

   case A_GO:
      if (pinfo.resting)
	 return False;
      break;

   case A_ENDDRAG:   // action_data is ID of object being dragged
      // See if user dragged object to inventory
      InventoryGetArea(&a);
      
      GetCursorPos(&mouse);
      ScreenToClient(cinfo->hMain, &mouse);
      
      if (!IsInArea(&a, mouse.x, mouse.y))
	 break;

      r = GetRoomObjectById((ID) action_data);
      if (r == NULL)
	 break;

      // If a non-gettable container, try to get contents
      if ((r->obj.flags & OF_CONTAINER) && !(r->obj.flags & OF_GETTABLE))
	 RequestObjectContents((ID) action_data);
      else RequestPickup((ID) action_data);
      
      break;

   case A_TABFWD:
      return InterfaceTab((int) action_data, True);
   case A_TABBACK:
      return InterfaceTab((int) action_data, False);
   }

   return True;
}

Bool CheckForAlwaysActiveSpells(spelltemp *sp)
{
   char *name;

   // Use the redbook rsc function, always returns English string.
   name = LookupRscRedbook(sp->obj.name_res);

   if (stricmp(name, "phase") == 0)
   {
      return True;
   }

   return False;
}

/****************************************************************************/
/*
 * InterfaceUserChanged:  Player object changed; update safety flag and redraw player.
 */
void InterfaceUserChanged(void)
{
   room_contents_node *r;

   UserAreaRedraw();
   AliasInit();

   r = GetRoomObjectById(cinfo->player->id);

   if (r == NULL)
      return;

   if (r->obj.flags & OF_SAFETY)
   {
      if (cinfo->config->aggressive)
      {
         if (!safety_flipped)
            SendSafety(0);
         safety_flipped = True;
      }
      else
      {
         pinfo.aggressive = cinfo->config->aggressive = False;
         safety_flipped = False;
      }
   }
   else 
   {
      if (!cinfo->config->aggressive)
      {
         if (!safety_flipped)
            SendSafety(1);
         safety_flipped = True;
      }
      else
      {
         pinfo.aggressive = cinfo->config->aggressive = True;
         safety_flipped = False;
      }

   }

   if (r->obj.flags & OF_TEMPSAFE)
   {
      if (cinfo->config->tempsafe)
      {
         // Flag on, setting on, don't change.
         pinfo.tempsafe = cinfo->config->tempsafe = True;
         temp_safe_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!temp_safe_flipped)
            SendTempSafe(0);
         temp_safe_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->tempsafe)
      {
         // Flag off, setting off, don't change.
         pinfo.tempsafe = cinfo->config->tempsafe = False;
         temp_safe_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!temp_safe_flipped)
            SendTempSafe(1);
         temp_safe_flipped = True;
      }
   }

   if (r->obj.flags & OF_GROUPING)
   {
      if (cinfo->config->grouping)
      {
         // Flag on, setting on, don't change.
         pinfo.grouping = cinfo->config->grouping = True;
         grouping_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!grouping_flipped)
            SendGrouping(0);
         grouping_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->grouping)
      {
         // Flag off, setting off, don't change.
         pinfo.grouping = cinfo->config->grouping = False;
         grouping_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!grouping_flipped)
            SendGrouping(1);
         grouping_flipped = True;
      }
   }

   if (r->obj.flags & OF_AUTOLOOT)
   {
      if (cinfo->config->autoloot)
      {
         // Flag on, setting on, don't change.
         pinfo.autoloot = cinfo->config->autoloot = True;
         autoloot_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!autoloot_flipped)
            SendAutoLoot(0);
         autoloot_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->autoloot)
      {
         // Flag off, setting off, don't change.
         pinfo.autoloot = cinfo->config->autoloot = False;
         autoloot_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!autoloot_flipped)
            SendAutoLoot(1);
         autoloot_flipped = True;
      }
   }

   if (r->obj.flags & OF_AUTOCOMBINE)
   {
      if (cinfo->config->autocombine)
      {
         // Flag on, setting on, don't change.
         pinfo.autocombine = cinfo->config->autocombine = True;
         autocombine_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!autocombine_flipped)
            SendAutoCombine(0);
         autocombine_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->autocombine)
      {
         // Flag off, setting off, don't change.
         pinfo.autocombine = cinfo->config->autocombine = False;
         autocombine_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!autocombine_flipped)
            SendAutoCombine(1);
         autocombine_flipped = True;
      }
   }

   if (r->obj.flags & OF_REAGENTBAG)
   {
      if (cinfo->config->reagentbag)
      {
         // Flag on, setting on, don't change.
         pinfo.reagentbag = cinfo->config->reagentbag = True;
         reagentbag_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!reagentbag_flipped)
            SendReagentBag(0);
         reagentbag_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->reagentbag)
      {
         // Flag off, setting off, don't change.
         pinfo.reagentbag = cinfo->config->reagentbag = False;
         reagentbag_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!reagentbag_flipped)
            SendReagentBag(1);
         reagentbag_flipped = True;
      }
   }

   if (r->obj.flags & OF_SPELLPOWER)
   {
      if (cinfo->config->spellpower)
      {
         // Flag on, setting on, don't change.
         pinfo.spellpower = cinfo->config->spellpower = True;
         spellpower_flipped = False;
      }
      else
      {
         // Flag on, setting off, turn flag off.
         if (!spellpower_flipped)
            SendSpellPower(0);
         spellpower_flipped = True;
      }
   }
   else
   {
      if (!cinfo->config->spellpower)
      {
         // Flag off, setting off, don't change.
         pinfo.spellpower = cinfo->config->spellpower = False;
         spellpower_flipped = False;
      }
      else
      {
         // Flag off, setting on, turn flag on.
         if (!spellpower_flipped)
            SendSpellPower(1);
         spellpower_flipped = True;
      }
   }
}
/****************************************************************************/
void InterfaceConfigChanged(void)
{
   // See if user changed safety flag
   if (pinfo.aggressive != cinfo->config->aggressive)
   {
      pinfo.aggressive = cinfo->config->aggressive;
      if (pinfo.aggressive)
         SendSafety(0);
      else SendSafety(1);
   }

   // See if user changed temp safe flag
   if (pinfo.tempsafe != cinfo->config->tempsafe)
   {
      pinfo.tempsafe = cinfo->config->tempsafe;
      if (pinfo.tempsafe)
         SendTempSafe(1);
      else SendTempSafe(0);
   }

   // See if user changed grouping flag
   if (pinfo.grouping != cinfo->config->grouping)
   {
      pinfo.grouping = cinfo->config->grouping;
      if (pinfo.grouping)
         SendGrouping(1);
      else SendGrouping(0);
   }

   // See if user changed autoloot flag
   if (pinfo.autoloot != cinfo->config->autoloot)
   {
      pinfo.autoloot = cinfo->config->autoloot;
      if (pinfo.autoloot)
         SendAutoLoot(1);
      else SendAutoLoot(0);
   }

   // See if user changed autocombine flag
   if (pinfo.autocombine != cinfo->config->autocombine)
   {
      pinfo.autocombine = cinfo->config->autocombine;
      if (pinfo.autocombine)
         SendAutoCombine(1);
      else SendAutoCombine(0);
   }

   // See if user changed reagentbag flag
   if (pinfo.reagentbag != cinfo->config->reagentbag)
   {
      pinfo.reagentbag = cinfo->config->reagentbag;
      if (pinfo.reagentbag)
         SendReagentBag(1);
      else SendReagentBag(0);
   }

   // See if user changed spellpower display flag
   if (pinfo.spellpower != cinfo->config->spellpower)
   {
      pinfo.spellpower = cinfo->config->spellpower;
      if (pinfo.spellpower)
         SendSpellPower(1);
      else SendSpellPower(0);
   }
}

/* Utility functions */
/****************************************************************************/


/************************************************************************/
/*
 * GetPlayerName:  Parse str, and return a pointer to the start of the first
 *   player name in it, or NULL if there is none in str.
 *   If next is non-NULL and we find a player name, fill next with 
 *   a pointer to the position in str just after the end of the first player name.
 *   str and *next can be identical.
 *   Modifies str by null terminating player name.
 */
char *GetPlayerName(char *str, char **next)
{
   char *ptr, *retval, *last_ptr;
   Bool quoted = False;
   Bool ambiguous = False;

   if (str == NULL)
      return NULL;

   // Skip separators
   while (*str == ' ' || *str == ',')
      str++;

   if (*str == 0)
      return NULL;

   // See if name is quoted
   if (*str == '\"')
   {
      quoted = True;
      retval = str + 1;
   }
   else retval = str;

   ptr = retval;
   last_ptr = NULL;

   // Skip to end of name and null terminate
   do
   {
      if (quoted)
      {
	 while (*ptr != '\"' && *ptr != 0)
	    ptr++;
      }
      else
      {
	 while (*ptr != ' ' && *ptr != ',' && *ptr != 0)
	    ptr++;
      }
      
      if (*ptr == '\0')
      {
	 if (next != NULL)
	    *next = NULL;
	 break;
      }
      else
      {
	 if (next != NULL)
   	    *next = ptr + 1;

	 // Null terminate player name
	 *ptr = '\0';
      }

      if (!quoted)
      {
	 // if we're not quoted
	 //
	 Bool wasambiguous = ambiguous;
	 ambiguous = (INVALID_ID == FindPlayerByName(retval));
	 if (ambiguous)
	 {
	    // restore player name and try again by appending next word
	    last_ptr = ptr;
	    *ptr++ = ' ';
	 }
	 else if (wasambiguous)
	 {
	    // String was ambiguous but now it's not.
	    break;
	 }
      }

   } while (ambiguous);

   // If we ran out of words before resolving ambiguity, remove the last word;
   // the caller may need it.
   if (ambiguous)
   {
      if (last_ptr == NULL)
	 debug(("GetPlayerName found ambiguous name in bad case\n"));
      else 
      {
	 *last_ptr = 0;
	 if (next != NULL)
	    *next = last_ptr + 1;
      }
   }

   return retval;
}
/************************************************************************/
/*
 * FindPlayerByName:  Return the object id of the logged-on player whose name
 *   best matches name.  Return 0 if no one matches, INVALID_ID if more than one
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
	 if (*ptr == 0)
	    return obj->id;
	 else continue;

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
/************************************************************************/
/*
 * FindPlayerByNameExact:  Return the object id of the logged-on player whose name
 *   exactly matches name.  The comparison is case insensitive.
 *   Return 0 if no one matches.
 */
ID FindPlayerByNameExact(char *name)
{
  list_type l;
  char *player;

  for (l = *(cinfo->current_users); l != NULL; l = l->next)
  {
    object_node *obj = (object_node *) (l->data);
    
    player = LookupRsc(obj->name_res);
    if (player == NULL)
      continue;

    if (!stricmp(name, player))
      return obj->id;
   }
   return 0;
}

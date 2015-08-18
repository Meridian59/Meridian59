// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * spells.c:  Deal with casting spells.
 * 
 * The "Spells" menu is added dynamically when the user enters the game.  When the server adds
 * or removes a spell from the player, we add or remove a menu item for it.  
 * The menu id of a spell is the index of the spell in the menu, plus a base constant.  
 * When the user chooses a spell from the menu, we take this index and find the spell.
 *
 * The spell list can become larger than the screen, so we subdivide it according to spell school.
 * Each of these corresponds to a submenu under the Spells menu.
 */

#include "client.h"
#include "merintr.h"

#define MAX_SCHOOLS 10

static list_type spells = NULL;     /* All spells available to user */

static list_type temp_list = NULL;  /* Temporary one-node list to save allocation every time */
static object_node temp_obj;        /* Temporary object node in temp_list */

static ID use_spell;  /* Remember spell player wants to use while he selects target */

static int num_schools;     // # of spell schools (sent from server)
static ID schoolID[MAX_SCHOOLS];

static HMENU spell_menu;    // Spells submenu
static HMENU *submenus;     // Array of school submenus

// Need to know numeric positions of main menu top level things, silly as this may be
#define MENU_POSITION_SPELLS 2

/* local function prototypes */
static void CastCallback(ID id);
static void MenuRenumberSpells(void);
static void MenuAddSpell(spell *sp);
static void MenuRemoveSpell(spell *sp);
/********************************************************************/
/*
 * SpellsInit:  Initialize spells when game entered.
 */
void SpellsInit(void)
{
   spell_menu = CreatePopupMenu();

   // Add "spells" menu item
   if (spell_menu != NULL)
   {
      InsertMenu(cinfo->main_menu, MENU_POSITION_SPELLS, MF_STRING | MF_POPUP | MF_BYPOSITION, 
		 (UINT) spell_menu, GetString(hInst, IDS_SPELLS));
      DrawMenuBar(cinfo->hMain);

   }
   
   num_schools = 0;
   submenus = NULL;
}
/********************************************************************/
/*
 * SpellsExit:  Free spells when game exited.
 */
void SpellsExit(void)
{
   int i;

   for (i=0; i < num_schools; i++)
   {
      if (submenus[i])
	 DestroyMenu(submenus[i]);
   }

   // Remove "Spells" menu
   if (spell_menu != NULL)
   {
      RemoveMenu(cinfo->main_menu, MENU_POSITION_SPELLS, MF_BYPOSITION);
      DrawMenuBar(cinfo->hMain);
      DestroyMenu(spell_menu);
   }

   spell_menu = NULL;

   SafeFree(submenus);
   submenus = NULL;
   FreeSpells();
}
/********************************************************************/
/*
* MenuRemoveAllSpells:  Remove all spells from menu.
*/
void MenuRemoveAllSpells(void)
{
   list_type l;

   for (l = spells; l != NULL; l = l->next)
   {
      MenuRemoveSpell((spell *)(l->data));
   }
   for (int i = 0; i < num_schools; i++)
   {
      if (submenus[i])
      {
         DestroyMenu(submenus[i]);
         submenus[i] = NULL;
      }
      RemoveMenu(spell_menu, 0, MF_BYPOSITION);
   }

}
/********************************************************************/
/*
* MenuReAddAllSpells:  Add all spells to menu.
*/
void MenuAddAllSpells(void)
{
   list_type l;

   for (l = spells; l != NULL; l = l->next)
   {
      MenuAddSpell((spell *)(l->data));
   }
}
/********************************************************************/
/*
 * SpellsGotSchools:  Display spell school names in menu.  This
 *   must be done before the spells themselves arrive, since the spells
 *   are added to the schools' submenus.
 */
void SpellsGotSchools(BYTE num, ID *schools)
{
   int i;

   num_schools = num;
   submenus = (HMENU *) SafeMalloc(num_schools * sizeof(HMENU));
    
   // Add spell schools to spell submenu
   for (i=0; i < num; i++)
   {
      schoolID[i] = schools[i];
      submenus[i] = NULL;
#if 0
     submenus[i] = CreatePopupMenu();
     
     InsertMenu(spell_menu, i, MF_STRING | MF_POPUP | MF_BYPOSITION, 
		(UINT) submenus[i], LookupNameRsc(schools[i]));
#endif
   }

   SafeFree(schools);
}
/********************************************************************/
void SetSpells(list_type new_spells)
{
   list_type l;

   FreeSpells();

   for (l = new_spells; l != NULL; l = l->next)
      AddSpell( (spell *) (l->data));

   list_delete(new_spells);

   /* Make temporary list for sending a spell's target to server */
   list_delete(temp_list);
   temp_list = list_add_item(temp_list, &temp_obj);
}
/********************************************************************/
void FreeSpells(void)
{
   list_type l;

   for (l = spells; l != NULL; l = l->next)
      MenuRemoveSpell( (spell *) (l->data));

   spells = ObjectListDestroy(spells);
   temp_list = list_delete(temp_list);
}
/********************************************************************/
void AddSpell(spell *sp)
{
   spells = list_add_item(spells, sp);
   MenuAddSpell(sp);
}
/********************************************************************/
void RemoveSpell(ID id)
{
   spell *old_spell = (spell *) list_find_item(spells, (void *) id, CompareIdObject);

   if (old_spell == NULL)
   {
      debug(("Tried to remove nonexistent spell #%ld\n", id));
      return;
   }

   MenuRemoveSpell(old_spell);

   spells = list_delete_item(spells, (void *) id, CompareIdObject);
   SafeFree(old_spell);
}
/************************************************************************/
/*
 * GetSpellName:  Parse str, and return a pointer to the start of the first
 *   spell name in it, or NULL if there is none in str.
 *   If next is non-NULL and we find a spell name, fill next with 
 *   a pointer to the position in str just after the end of the first spell name.
 *   str and *next can be identical.
 *   Modifies str by null terminating spell name.
 */
char *GetSpellName(char *str, char **next)
{

   // Shamefully stolen from GetPlayerName().
   // Copied so that we can do the new FindSpellByName()
   // checks on unquoted ambiguous words.

   char *ptr, *retval;
   Bool quoted = False;
   Bool ambiguous = False;

   if (str == NULL)
      return NULL;

   // Skip spaces
   while (*str == ' ')
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

   // Skip to end of name and null terminate
   ptr = retval;

   do
   {

      if (quoted)
      {
	 while (*ptr != '\"' && *ptr != 0)
	    ptr++;
      }
      else
      {
	 while (*ptr != ' ' && *ptr != 0)
	    ptr++;
      }

      if (*ptr == 0)
      {
	 if (next != NULL)
	    *next = NULL;
	 break;
      }
      else
      {
	 if (next != NULL)
	    *next = ptr + 1;

	 // Null terminate spell name
	 *ptr = 0;
      }

      if (!quoted)
	 ambiguous = (SPELL_AMBIGUOUS == FindSpellByName(retval));
      if (ambiguous)
      {
	 // restore spell name and try again by appending next word
	 *ptr++ = ' ';
      }

   } while (ambiguous);

   return retval;
}

spell *FindSpellByID(ID id)
{
   list_type l;

   for (l = spells; l != NULL; l = l->next)
   {
      spell *pSpell = (spell *) (l->data);
      if (pSpell->obj.id == id)
	 return pSpell;
   }
   return NULL;
}

/********************************************************************/
/*
 * FindSpellByName:  Return the spell whose name best matches name.
 *   Return SPELL_NOMATCH if no spell matches, or SPELL_AMBIGUOUS if more than one 
 *   spell matches equally well.
 */
spell *FindSpellByName(char *name)
{
   list_type l;
   spell *sp, *best_spell = NULL;
   int match, max_match;
   Bool tied;            // True if a different spell matches as well as best_spell
   char *ptr, *spell_name;

   max_match = 0;
   tied = False;
   for (l = spells; l != NULL; l = l->next)
   {
      sp = (spell *) (l->data);

      spell_name = LookupNameRsc(sp->obj.name_res);
      ptr = name;
      match = 0;
      while (*ptr != 0 && *spell_name != 0)
      {
	 if (toupper(*ptr) != toupper(*spell_name))
	 {
	    match = 0;
	    break;
	 }
	 match++;
	 ptr++;
	 spell_name++;
      }

      // Check for exact match, or extra characters in search string
      if (*spell_name == 0)
	 if (*ptr == 0)
	    return sp;
	 else continue;

      if (match > max_match)
      {
	 max_match = match;
	 best_spell = sp;
	 tied = False;
      }
      else if (match == max_match)
	 tied = True;
   }

   
   if (max_match == 0)
      return SPELL_NOMATCH;

   if (tied)
      return SPELL_AMBIGUOUS;

   return best_spell;
}

/********************************************************************/
void UserCastSpell(void)
{
   spell *sp = NULL, *temp;
   list_type sel_list, l;

   if (GetPlayer()->viewID && (GetPlayer()->viewID != GetPlayer()->id))
   {
     if (!(GetPlayer()->viewFlags & REMOTE_VIEW_CAST))
       return;
   }
   
   sel_list = DisplayLookList(cinfo->hMain, GetString(hInst, IDS_CAST), spells, LD_SORT);
   
   if (sel_list == NULL)
      return;

   /* Find spell in our private list */
   temp = (spell *) sel_list->data;
   for (l = spells; l != NULL; l = l->next)
   {
      sp = (spell *) l->data;
      if (sp->obj.id == temp->obj.id)
	 break;
   }

   if (sp == NULL)
   {
      debug(("Couldn't find selected spell in spell list!\n"));
      return;
   }
   
   list_delete(sel_list);
   
   PerformAction(A_CASTSPELL, sp);
}
/********************************************************************/
/*
 * SpellCast:  User wants to cast given spell.
 */
void SpellCast(spell *sp)
{
   if (GetPlayer()->viewID && (GetPlayer()->viewID != GetPlayer()->id))
   {
     if (!(GetPlayer()->viewFlags & REMOTE_VIEW_CAST))
       return;
   }

   /* If we don't need target, just cast spell */
   if (sp->num_targets == 0)
   {
      RequestCast(sp->obj.id, NULL);
      return;
   }

	if( GetUserTargetID() != INVALID_ID )
	{
    ID id = GetUserTargetID();
		//	User has target already selected.
		if (id == GetPlayer()->id || FindVisibleObjectById(id))
		{
			/* Make temporary list for sending to server */
			temp_obj.id = GetUserTargetID();
			temp_obj.temp_amount = 1;
			RequestCast( sp->obj.id, temp_list );
		}
		else	//	Target cannot be seen.
			GameMessage( GetString( hInst, IDS_TARGETNOTVISIBLEFORCAST ) );
		return;
	}

   /* Get target object from user */
   /* Save spell # */
   use_spell = sp->obj.id;
   
   /* Register callback & select item */
   GameSetState(GAME_SELECT);
   SetSelectCallback(CastCallback);

   if (UserMouselookIsEnabled())
   {
		while (ShowCursor(TRUE) < 0)
			ShowCursor(TRUE);
   }
}
/********************************************************************/
/*
 * CastCallback:  Called when user selects target of spell.
 */
void CastCallback(ID id)
{
   /* Make temporary list for sending to server */
   temp_obj.id = id;
   temp_obj.temp_amount = 1;

   if (pinfo.resting)
      GameMessage(GetString(hInst, IDS_SPELLRESTING));
   else RequestCast(use_spell, temp_list);

   if (UserMouselookIsEnabled())
   {
		while (ShowCursor(FALSE) >= 0)
			ShowCursor(FALSE);
   }
}

/* stuff for the "spells" menu on the main window
/********************************************************************/
/*
 * MenuAddSpell:  Add given spell to Spells submenu.
 */
void MenuAddSpell(spell *sp)
{
   int num, index, len;
   char item_name[MAXRSCSTRING + 1], *name;
   HMENU submenu;
   int school = sp->school;

   if ((school < 0) || (school >= num_schools))
   {
      debug(("MenuAddSpell got spell with bad school number %d\n", school));
      return;
   }

   submenu = submenus[school];

   if (NULL == submenu)
   {
      submenus[school] = CreatePopupMenu();
      AppendMenu(spell_menu, MF_STRING | MF_POPUP | MF_BYPOSITION, 
	 (UINT) submenus[school], LookupNameRsc(schoolID[school]));
      submenu = submenus[school];
   }
   num = GetMenuItemCount(submenu);

   name = LookupNameRsc(sp->obj.name_res);
   // Add in sorted order
   for (index = 0; index < num; index++)
   {
      len = GetMenuString(submenu, index, item_name, MAXRSCSTRING, MF_BYPOSITION);
      if (len == 0)
	 continue;
      if (stricmp(item_name, name) >= 0)
	 break;
   }

   // Check for adding to end
   if (index == num)
      index = -1;

   // Use ID_SPELL + (school * # per school)+ (# of spells) as menu id
   InsertMenu(submenu, index, MF_STRING | MF_BYPOSITION, 
	      ID_SPELL + num + sp->school * MAX_SPELLS_PER_SCHOOL, name);
}
/********************************************************************/
/*
 * MenuRemoveSpell:  Remove given spell from main menu.
 */
void MenuRemoveSpell(spell *sp)
{
   int index = -1, i;
   HMENU submenu = NULL;

   if (spell_menu == NULL)
      return;

   for (i=0; i < num_schools; i++)
   {
      submenu = submenus[i];
      if (submenu)
      {
	 index = MenuFindItemByName(submenu, LookupNameRsc(sp->obj.name_res));
	 if (index != -1)
	    break;
      }
   }

   if (index == -1)
   {
      debug(("MenuRemoveSpell failed to find spell %d in menu\n", sp->obj.id));
      return;
   }

   RemoveMenu(submenu, index, MF_BYPOSITION);

   MenuRenumberSpells();
}
/********************************************************************/
/*
 * MenuRenumberSpells:  Reset spell menu ids when a spell is removed.
 */
void MenuRenumberSpells(void)
{
   int num, i, j;
   char name[MAXRSCSTRING];
   HMENU submenu;

   if (spell_menu == NULL)
      return;

   for (i=0; i < num_schools; i++)
   {
      submenu = submenus[i];
      if (submenu)
      {
	 num = GetMenuItemCount(submenu);
	 for (j=0; j < num; j++)
	 {
	    GetMenuString(submenu, j, name, MAXRSCSTRING, MF_BYPOSITION);
	    ModifyMenu(submenu, j, MF_STRING | MF_BYPOSITION, 
	       ID_SPELL + j + i * MAX_SPELLS_PER_SCHOOL, name);
	 }
      }
   }
}
/********************************************************************/
/*
 * MenuSpellChosen:  The spell with the given command id in the spell menu was chosen.
 */
void MenuSpellChosen(int id)
{
   int len, num=0, index=0, i;
   char item_name[MAXRSCSTRING + 1];
   spell *sp;
   HMENU submenu = NULL;

   if (spell_menu == NULL)
      return;


   for (i=0; i < num_schools; i++)
   {
      submenu = submenus[i];
      if (submenu)
      {
	 num = GetMenuItemCount(submenu);

	 // Look for menu item matching command id
	 for (index = 0; index < num; index++)
	 {
	    if (GetMenuItemID(submenu, index) == (UINT) id)
	    break;
	 }
	 if (index != num)
	    break;
      }
   }

   if (index == num)
   {
      debug(("MenuSpellChosen couldn't find menu id #%d\n", id));
      return;
   }

   len = GetMenuString(submenu, index, item_name, MAXRSCSTRING, MF_BYPOSITION);
   if (len == 0)
   {
      debug(("MenuSpellChosen get spell #%d from menu\n", index));
      return;
   }
   sp = FindSpellByName(item_name);
   if (sp == SPELL_NOMATCH || sp == SPELL_AMBIGUOUS)
   {
      debug(("MenuSpellChosen couldn't find spell %s\n", item_name));
      return;
   }

   PerformAction(A_CASTSPELL, sp);
}

/********************************************************************/
/*
 * GetSpellsObject:  Added by ajw. Returns ptr to the spell object
 *					 specified by id. There is probably another way to do this, but I can't find the function.
 */
object_node* GetSpellObject( ID idFind )
{
   spell* sp = NULL;
   list_type listptr;
   for( listptr = spells; listptr != NULL; listptr = listptr->next )
   {
      sp = (spell*)listptr->data;
      if( sp->obj.id == idFind )
	 break;
   }
   if (sp == NULL)
   {
      debug(("Couldn't find selected spell in spell list!\n"));
      return NULL;
   }
   return &sp->obj;
}

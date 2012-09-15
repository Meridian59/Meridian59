// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * skills.c:  Deal with casting skills.
 * 
 * The "Skills" menu is added dynamically when the user enters the game.  When the server adds
 * or removes a skill from the player, we add or remove a menu item for it.  
 * The menu id of a skill is the index of the skill in the menu, plus a base constant.  
 * When the user chooses a skill from the menu, we take this index and find the skill.
 *
 * The skill list can become larger than the screen, so we subdivide it according to skill school.
 * Each of these corresponds to a submenu under the Skills menu.
 */

#include "client.h"
#include "merintr.h"
#include "skills.h"

static list_type skills = NULL;     /* All skills available to user */

static list_type temp_list = NULL;  /* Temporary one-node list to save allocation every time */
static object_node temp_obj;        /* Temporary object node in temp_list */

/********************************************************************/
/*
 * SkillsInit:  Initialize skills when game entered.
 */
void SkillsInit(void)
{
}
/********************************************************************/
/*
 * SkillsExit:  Free skills when game exited.
 */
void SkillsExit(void)
{
   FreeSkills();
}
/********************************************************************/
void SetSkills(list_type new_skills)
{
   list_type l;

   FreeSkills();

   for (l = new_skills; l != NULL; l = l->next)
      AddSkill( (skill *) (l->data));

   list_delete(new_skills);

   /* Make temporary list for sending a skill's target to server */
   list_delete(temp_list);
   temp_list = list_add_item(temp_list, &temp_obj);
}
/********************************************************************/
void FreeSkills(void)
{
   skills = ObjectListDestroy(skills);
   temp_list = list_delete(temp_list);
}
/********************************************************************/
void AddSkill(skill *sp)
{
   skills = list_add_item(skills, sp);
}
/********************************************************************/
void RemoveSkill(ID id)
{
   skill *old_skill = (skill *) list_find_item(skills, (void *) id, CompareIdObject);

   if (old_skill == NULL)
   {
      debug(("Tried to remove nonexistent skill #%ld\n", id));
      return;
   }

   skills = list_delete_item(skills, (void *) id, CompareIdObject);
   SafeFree(old_skill);
}
/********************************************************************/
/*
 * GetSkillsObject: Returns ptr to the skill object specified by id.
 */
object_node* GetSkillObject( ID idFind )
{
   skill *sp = FindSkillByID(idFind);
   if (sp)
      return &sp->obj;
   else
      return NULL;
}

skill *FindSkillByID(ID idFind)
{
   skill* sp = NULL;
   list_type listptr;

   for( listptr = skills; listptr != NULL; listptr = listptr->next )
   {
      sp = (skill*)listptr->data;
      if( sp->obj.id == idFind )
	 return sp;
   }
   return NULL;
}

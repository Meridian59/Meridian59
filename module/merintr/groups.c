// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * groups.c:  Deal with loading, saving, and searching user groups.  These groups are
 *   used as targets of "group tells" (text send to a group of users).
 */

#include "client.h"
#include "merintr.h"

#define MAX_GROUPLINE (MAX_GROUPSIZE * (MAX_CHARNAME + 1))
#define MAX_GROUPKEYS (MAX_NUMGROUPS * (MAX_GROUPNAME + 1))

static char group_section[] = "Groups";   // Section for groups in INI file
static char *name_separator = ",";

int num_groups;
char groups[MAX_NUMGROUPS][MAX_GROUPNAME + 1];       // Pointers to group names

/****************************************************************************/
/*
 * GroupsLoad:  Load all group names from INI file.
 */
void GroupsLoad(void)
{
   char buf[MAX_GROUPKEYS], *ptr;

   // Enumerate all group names
   GetPrivateProfileString(group_section, NULL, "", buf, MAX_GROUPKEYS, cinfo->ini_file);
   
   ptr = buf;
   num_groups = 0;
   while (*ptr != 0 && num_groups < MAX_NUMGROUPS)
   {
      strncpy(groups[num_groups], ptr, MAX_NUMGROUPS);
      groups[num_groups][MAX_NUMGROUPS] = 0;
      ptr += strlen(ptr) + 1;
      num_groups++;
   }
}

/****************************************************************************/
/*
 * GroupLoad:  Look up the group with the given name.  If a group exists, fill in
 *   g with information about the group and return GROUP_MATCH.
 *   If the group name is ambiguous, return GROUP_AMBIGUOUS.
 *   Otherwise, return GROUP_NOMATCH.
 */
int GroupLoad(char *name, UserGroup *g)
{
   int index;
   char buf[MAX_GROUPLINE], *ptr;

   index = FindGroupByName(name);
   switch(index)
   {
   case GROUP_NOMATCH:
      return GROUP_NOMATCH;

   case GROUP_AMBIGUOUS:
      return GROUP_AMBIGUOUS;

   default:
      // Load and copy group info
      GetPrivateProfileString(group_section, groups[index], "", 
			      buf, MAX_GROUPLINE, cinfo->ini_file);

      ptr = strtok(buf, name_separator);
      g->num_users = 0;
      while (ptr != NULL && g->num_users < MAX_GROUPSIZE)
      {
	 strncpy(g->names[g->num_users], ptr, MAX_CHARNAME);
	 g->names[g->num_users][MAX_CHARNAME] = 0;
	 ptr = strtok(NULL, name_separator);
	 g->num_users++;
      }
      return GROUP_MATCH;
   }
}
/****************************************************************************/
/*
 * GroupSave:  Save the given group to the INI file.  name gives the name of the group.
 */
void GroupSave(char *name, UserGroup *g)
{
   int i;
   char buf[MAX_GROUPLINE], *ptr;

   buf[0] = 0;
   ptr = buf;
   for (i=0; i < g->num_users; i++)
   {
      if (i != 0)
	  strcat(ptr, ",");
      strcat(ptr, g->names[i]);
   }
   WritePrivateProfileString(group_section, name, buf, cinfo->ini_file);
}
/****************************************************************************/
/*
 * GroupAdd:  Add the names in args to the given group name.  If the group doesn't
 *   exist, create it.
 *   Return number of names added to group (0 on failure).
 */
int GroupAdd(char *group_name, char *args)
{
   UserGroup g;
   int index, i, num_added;
   char *ptr, *name;
   Bool add_name;

   num_added = 0;
   index = FindGroupByName(group_name);
   switch(index)
   {
   case GROUP_NOMATCH:
      GameMessage(GetString(hInst, IDS_BADGROUPNAME));
      return 0;

   case GROUP_AMBIGUOUS:
      GameMessage(GetString(hInst, IDS_DUPLICATEGROUPNAME));
      return 0;

   default:
      group_name = groups[index];
      GroupLoad(group_name, &g);
      break;
   }

   name = GetPlayerName(args, &ptr);
   while (name != NULL)
   {
      if (g.num_users >= MAX_GROUPSIZE)
      {
	 GameMessagePrintf(GetString(hInst, IDS_GROUPFULL), MAX_GROUPNAME, group_name);
	 break;
      }

      // Check for duplicates
      add_name = True;
      for (i=0; i < g.num_users; i++)
	 if (!stricmp(g.names[i], name))
	 {
	    add_name = False;
	    break;
	 }
      
      if (add_name)
      {
	 strncpy(g.names[g.num_users], name, MAX_CHARNAME);
	 g.names[g.num_users][MAX_CHARNAME] = 0;
	 g.num_users++;
	 num_added++;
      }
      name = GetPlayerName(ptr, &ptr);
   }

   GroupSave(group_name, &g);
   return num_added;
}
/****************************************************************************/
/*
 * GroupDelete:  Remove the group with the given name.
 *   Return True on success.
 */
Bool GroupDelete(char *group_name)
{
   int index, i;

   index = FindGroupByName(group_name);
   switch(index)
   {
   case GROUP_NOMATCH:
      GameMessage(GetString(hInst, IDS_BADGROUPNAME));
      return False;

   case GROUP_AMBIGUOUS:
      GameMessage(GetString(hInst, IDS_DUPLICATEGROUPNAME));
      return False;
   }

   group_name = groups[index];

   // Remove INI entry
   WritePrivateProfileString(group_section, group_name, NULL, cinfo->ini_file);

   // Remove group from internal group structure
   for (i=index; i < num_groups - 1; i++) 
      strcpy(groups[i], groups[i + 1]);

   num_groups--;
   return True;
}
/****************************************************************************/
/*
 * GroupDeleteNames:  Remove the given names from the given group.
 *   Return number of names deleted, or -1 if the group itself was deleted.
 */
int GroupDeleteNames(char *group_name, char *args)
{
   int i, j, index, num_deleted;
   UserGroup g;
   char *name, *ptr;
   
   index = FindGroupByName(group_name);
   switch(index)
   {
   case GROUP_NOMATCH:
      GameMessage(GetString(hInst, IDS_BADGROUPNAME));
      return 0;
      
   case GROUP_AMBIGUOUS:
      GameMessage(GetString(hInst, IDS_DUPLICATEGROUPNAME));
      return 0;
      
   default:
      group_name = groups[index];
      break;
   }
   
   GroupLoad(group_name, &g);
   
   // Remove each name
   num_deleted = 0;
   name = GetPlayerName(args, &ptr);
   while (name != NULL)
   {
      for (i=0; i < g.num_users; i++)
	 if (!stricmp(g.names[i], name))
	 {
	    for (j=i; j < g.num_users - 1; j++)
	       strcpy(g.names[j], g.names[j + 1]);
	    g.num_users--;
	    num_deleted++;
	    break;
	 }
      name = GetPlayerName(ptr, &ptr);
   }

   // XXX Don't delete empty groups (???)
#if 0   
   if (g.num_users == 0)
   {
      GroupDelete(group_name);
      return -1;
   }
#endif
   
   GroupSave(group_name, &g);
   return num_deleted;
}
/****************************************************************************/
/*
 * GroupNew:  Create a new group with the given name.
 *   Return True if group created.
 */
Bool GroupNew(char *group_name)
{
   int i;
   UserGroup g;

   if (num_groups >= MAX_NUMGROUPS)
   {
      GameMessage(GetString(hInst, IDS_TOOMANYGROUPS));
      return FALSE;
   }

   // Search for exact duplicate
   for (i=0; i < num_groups; i++)
      if (!stricmp(group_name, groups[i]))
      {
	 GameMessage(GetString(hInst, IDS_ALREADYGROUP));
	 return False;
      }


   // Add group to list
   strncpy(groups[num_groups], group_name, MAX_GROUPNAME);
   groups[num_groups][MAX_GROUPNAME] = 0;
   num_groups++;

   g.num_users = 0;
   GroupSave(group_name, &g);
   return True;
}
/****************************************************************************/
/*
 * GroupsPrint:  Display a list of all defined groups.
 */
void GroupsPrint(void)
{
   int i;
   COLORREF color;
   BYTE style;

   if (num_groups == 0)
   {
      GameMessage(GetString(hInst, IDS_NOGROUPS));
      return;
   }

   GameMessage(GetString(hInst, IDS_GROUPLIST));
   color = RGB(0, 0, 0);
   style = STYLE_NORMAL;
   EditBoxStartAdd();
   for (i=0; i < num_groups; i++)
   {
      if (i != 0)
	 DisplayMessage(", ", color, style);
      DisplayMessage(groups[i], color, style);
   }
   EditBoxEndAdd();
}
/****************************************************************************/
/*
 * GroupPrint:  Display the members of the group with the given name.
 */
void GroupPrint(char *group_name)
{
   UserGroup g;
   int i, index;
   COLORREF color;
   BYTE style;
   ID id;
   char buf[MAX_CHARNAME + 10];

   index = FindGroupByName(group_name);
   switch (index)
   {
   case GROUP_NOMATCH:
      GameMessage(GetString(hInst, IDS_BADGROUPNAME));
      break;
      
   case GROUP_AMBIGUOUS:
      GameMessage(GetString(hInst, IDS_DUPLICATEGROUPNAME));
      break;

   default:
      group_name = groups[index];
      GroupLoad(group_name, &g);

      GameMessagePrintf(GetString(hInst, IDS_GROUPMEMBERS), MAX_GROUPNAME, group_name);

      color = RGB(0, 0, 0);
      style = STYLE_NORMAL;
      EditBoxStartAdd();
      for (i=0; i < g.num_users; i++)
      {
	 if (i != 0)
	    DisplayMessage(", ", color, style);
	 id = FindPlayerByNameExact(g.names[i]);
	 
	 // Show player in red if logged on
	 if (id == 0 || id == INVALID_ID)
	    DisplayMessage(g.names[i], color, style);
	 else
	 {
	    sprintf(buf, "~r%s~n", g.names[i]);
	    DisplayMessage(buf, color, style);
	 }
      }
      EditBoxEndAdd();
      break;
   }
}

/****************************************************************************/
/*
 * FindGroupByName:  Return the index into "groups" of the group whose name
 *   best matches name.  Return GROUP_NOMATCH if no group matches, GROUP_AMBIGUOUS 
 *   if more than one group matches equally well.
 */
int FindGroupByName(char *name)
{
   int index;
   char *group_name, *ptr;
   int match, max_match;
   int best_index;
   Bool tied;            // True if a different index matches as well as best_index

   max_match = 0;
   tied = False;
   for (index = 0; index < num_groups; index++)
   {
      group_name = groups[index];

      ptr = name;
      match = 0;
      while (*ptr != 0 && *group_name != 0)
      {
	 if (toupper(*ptr) != toupper(*group_name))
	 {
	    match = 0;
	    break;
	 }
	 match++;
	 ptr++;
	 group_name++;
      }

      // Check for exact match, or extra characters in search string
      if (*group_name == 0)
	 if (*ptr == 0)
	    return index;
	 else continue;
       
      if (match > max_match)
      {
	 max_match = match;
	 best_index = index;
	 tied = False;
      }
      else if (match == max_match)
	 tied = True;
   }
   
   if (max_match == 0)
      return GROUP_NOMATCH;

   if (tied)
      return GROUP_AMBIGUOUS;

   return best_index;
}
/****************************************************************************/
/*
 * GroupNameMatches:  Return True iff the given string exactly matches
 *   the name of the group with the given index.
 */
Bool GroupNameMatches(char *name, int index)
{
   if (index < 0 || index >= num_groups)
      return False;

   if (!stricmp(name, groups[index]))
      return True;
   return False;
}

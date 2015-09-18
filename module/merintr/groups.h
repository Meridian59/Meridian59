// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * groups.h:  Header file for groups.c
 */

#ifndef _GROUPS_H
#define _GROUPS_H

#define MAX_GROUPSIZE 100  // Maximum number of players in a user-defined group

#define MAX_NUMGROUPS 30  // Maximum number of user-defined groups
#define MAX_GROUPNAME 10  // Maximum length of group name

typedef struct {
   int  num_users;
   char names[MAX_GROUPSIZE][MAX_CHARNAME + 1];
} UserGroup;

// Return codes for group functions
enum {GROUP_MATCH = 0, GROUP_AMBIGUOUS = -1, GROUP_NOMATCH = -2};

void GroupsLoad(void);
int  GroupLoad(char *name, UserGroup *g);
void GroupSave(char *name, UserGroup *g);
int  GroupAdd(char *group_name, char *args);
Bool GroupNew(char *group_name);

Bool GroupDelete(char *group_name);
int  GroupDeleteNames(char *group_name, char *args);

void GroupPrint(char *group_name);
void GroupsPrint(void);

int FindGroupByName(char *name);
Bool GroupNameMatches(char *name, int index);

void AbortGroupDialog(void);

#endif /* #ifndef _GROUPS_H */

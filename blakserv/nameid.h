// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * nameid.h
 *
 */

#ifndef _NAMEID_H
#define _NAMEID_H

typedef struct nameid_struct
{
   char *name;
   int id;
   struct nameid_struct *next;
} nameid_node;

void InitNameID(void);
void ResetNameID(void);
void CreateNameID(char *parm_name,int parm_id);
int GetIDByName(const char *name);
const char * GetNameByID(int id);

#endif

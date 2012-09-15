// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dllist.h
 *
 */

#ifndef _DLLIST_H
#define _DLLIST_H

typedef struct dllist_struct
{
   char fname[MAX_PATH+FILENAME_MAX];
   int file_type; /* BFER_FILE_xxx, either plain or bar/gzip */
   int last_mod_time;
   int file_size;
   struct dllist_struct *next;
} dllist_node;

void InitDLlist(void);
void ResetDLlist(void);
dllist_node * GetDLNodeByFilename(char *filename);
int CountNewDLFile(session_node *s);
void ForEachNewDLFile(session_node *s,void (*callback_func)(char *str,int time,int type,int size));
void ForEachDLlist(void (*callback_func)(dllist_node *dl));
void AddBuiltInDLlist(void);

#endif

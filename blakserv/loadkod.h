// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadkod.h
 *
 */

#ifndef _LOADKOD_H
#define _LOADKOD_H

typedef struct loaded_bof_struct
{
   char fname[MAX_PATH+FILENAME_MAX];
   char *mem;
   int length;
   struct loaded_bof_struct *next;
} loaded_bof_node;

void InitLoadBof(void);
void ResetLoadBof(void);
void LoadBof(void);
void CloseAllFiles(void);

#endif

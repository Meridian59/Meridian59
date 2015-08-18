// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admincons.h
 *
 */

#ifndef _ADMINCONS_H
#define _ADMINCONS_H

typedef struct admin_constant_struct
{
   char *name;
   int value;
   struct admin_constant_struct *next;
} admin_constant_node;

void InitAdminConstants(void);
void ResetAdminConstants(void);
void LoadAdminConstants(void);
Bool LookupAdminConstant(const char *name,int *ret_ptr);


#endif

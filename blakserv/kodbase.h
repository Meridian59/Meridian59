// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * kodbase.h
 *
 */

#ifndef _KODBASE_H
#define _KODBASE_H

typedef struct property_name_struct
{
   int id;
   char *name;
   struct property_name_struct *next;
} property_name_node;

typedef struct classvar_name_struct
{
   int id;
   char *name;
   struct classvar_name_struct *next;
} classvar_name_node;

void LoadKodbase(void);

#endif

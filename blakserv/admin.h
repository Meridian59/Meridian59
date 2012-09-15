// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admin.h
 *
 */

#ifndef _ADMIN_H
#define _ADMIN_H

#define MAX_ADMIN_COMMAND 120

typedef struct
{
   char command[MAX_ADMIN_COMMAND];
} admin_data;

#endif

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * system.c
 *

 This module supports keeping track of the system object, which we
 have to send messages to sometimes (like garbage collection
 notifications and things like that).  It is not currently ever not
 object 0, but there's no permanent reason for that.

 */

#include "blakserv.h"

int system_obj_id;

int GetSystemObjectID()
{
   return system_obj_id;
}

void SetSystemObjectID(int new_id)
{
   system_obj_id = new_id;
}



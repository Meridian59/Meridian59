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

 This module supports keeping track of the built-in objects, which we
 have to send messages to sometimes (like garbage collection
 notifications for system and things like that). System iss not
 currently ever not object 0, but there's no permanent reason for that.

 */

#include "blakserv.h"

int system_obj_id;
int settings_obj_id;

int GetSystemObjectID()
{
   return system_obj_id;
}

void SetSystemObjectID(int new_id)
{
   system_obj_id = new_id;
}

int GetSettingsObjectID()
{
   return settings_obj_id;
}

void SetSettingsObjectID(int new_id)
{
   settings_obj_id = new_id;
}

// Creates all the built-in objects that aren't referenced by other objects
// in blakod, but can be called by constant (e.g. SYSTEM).
void CreateBuiltInObjects()
{
   SetSystemObjectID(CreateObject(SYSTEM_CLASS, 0, NULL));
   SetSettingsObjectID(CreateObject(SETTINGS_CLASS, 0, NULL));
}

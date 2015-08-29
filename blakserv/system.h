// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * system.h
 *
 */

#ifndef _SYSTEM_H
#define _SYSTEM_H

void CreateBuiltInObjects();
int GetBuiltInObjectID(int id);
void SetBuiltInObjectID(int ref_id, int obj_id);
void SetBuiltInObjectIDByClass(int ref_id, int obj_id);
int GetSystemObjectID(void);
void SetSystemObjectID(int new_id);
int GetSettingsObjectID();
void SetSettingsObjectID(int new_id);
int GetRealTimeObjectID();
void SetRealTimeObjectID(int new_id);
int GetEventEngineObjectID();
void SetEventEngineObjectID(int new_id);

#endif

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * say.h:  Header file for say.c
 */

#ifndef _SAY_H
#define _SAY_H

void SetCurrentUsers(list_type users);
void AddCurrentUser(object_node *user);
void RemoveCurrentUser(ID user_id);
void FreeCurrentUsers(void);
void UserYell(void);
void UserWho(void);

M59EXPORT BOOL FilterSayMessage(char *message);

#endif /* #ifndef _SAY_H */

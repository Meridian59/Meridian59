// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * user.h
 *
 */

#ifndef _USER_H
#define _USER_H

typedef struct user_struct
{
   int account_id;
   int object_id;
   struct user_struct *next;
} user_node;

void InitUser(void);
void ResetUser(void);
void ClearUser(void);
user_node * CreateNewUser(int account_id,int class_id);
void CreateUseronAccount(account_node *a);
Bool AssociateUser(int account_id,int object_id);
void LoadUser(int account_id,int object_id);
void ChangeUserObjectID(int new_id,int prev_id);
int DeleteUserByAccountID(int account_id);
int DeleteUserByObjectID(int object_id);
int CountUserByAccountID(int account_id);
user_node * GetUserByObjectID(int object_id);
void ForEachUser(void (*callback_func)(user_node *u));
void ForEachUserByAccountID(void (*callback_func)(user_node *u),int account_id);
user_node * GetFirstUserByAccountID(int account_id);
user_node * GetUserByName(char *username);
void ChangeUserAccountID(int account_id, int new_account_id);

#endif

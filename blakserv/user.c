// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * user.c
 *

 This module maintains a linked list of user nodes, which are
 associations between account numbers and object numbers in Blakod of
 their game player.  An account can have more than one of these.
 Since the objects can change number when garbage collected, there is
 a function the garbage collector calls to change the object number.

 */

#include "blakserv.h"

user_node *users;

void InitUser(void)
{
   users = NULL;
}

void ResetUser(void)
{
   ClearUser();
}

void ClearUser(void)
{
   user_node *u,*temp;

   u = users;
   while (u != NULL)
   {
      temp = u;
      u = u->next;
      FreeMemory(MALLOC_ID_USER,temp,sizeof(user_node));
   }
   users = NULL;
}

user_node * CreateNewUser(int account_id,int class_id)
{   
   user_node *u;

   val_type system_id_const,name_val;
   parm_node p[2];
   char buf[100];
   
   u = (user_node *)AllocateMemory(MALLOC_ID_USER,sizeof(user_node));

   u->account_id = account_id;

   system_id_const.v.tag = TAG_OBJECT;
   system_id_const.v.data = GetSystemObjectID();

   p[0].type = CONSTANT;
   p[0].value = system_id_const.int_val;
   p[0].name_id = SYSTEM_PARM;

   sprintf(buf,"User%i%i%I64i",account_id,GetTime()%100000,GetMilliCount()%1000);
   
   name_val.v.tag = TAG_RESOURCE;
   name_val.v.data = AddDynamicResource(buf);
	 
   p[1].type = CONSTANT;
   p[1].value = name_val.int_val;
   p[1].name_id = NAME_PARM;
   
   u->object_id = CreateObject(class_id,2,p);

   u->next = users;
   users = u;

   return u;
}

void CreateUseronAccount(account_node *a)
{
	CreateNewUser(a->account_id,USER_CLASS);
}

user_node * CreateNewUserByName(int account_id,int class_id,char*buf)
{   
   user_node *u;

   val_type system_id_const,name_val;
   parm_node p[2];
   
   u = (user_node *)AllocateMemory(MALLOC_ID_USER,sizeof(user_node));

   u->account_id = account_id;

   system_id_const.v.tag = TAG_OBJECT;
   system_id_const.v.data = GetSystemObjectID();

   p[0].type = CONSTANT;
   p[0].value = system_id_const.int_val;
   p[0].name_id = SYSTEM_PARM;

   
   name_val.v.tag = TAG_RESOURCE;
   name_val.v.data = AddDynamicResource(buf);
	 
   p[1].type = CONSTANT;
   p[1].value = name_val.int_val;
   p[1].name_id = NAME_PARM;
   
   u->object_id = CreateObject(class_id,2,p);

   u->next = users;
   users = u;

   return u;
}

Bool AssociateUser(int account_id,int object_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->object_id == object_id)
	 return False;
      u = u->next;
   }

   u = (user_node *)AllocateMemory(MALLOC_ID_USER,sizeof(user_node));
   u->account_id = account_id;
   u->object_id = object_id;

   u->next = users;
   users = u;

   return True;
}

void LoadUser(int account_id,int object_id)
{
   AssociateUser(account_id,object_id);
}

void ChangeUserObjectID(int new_id,int prev_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->object_id == prev_id)
	 u->object_id = new_id;
      u = u->next;
   }
}

int DeleteUserByAccountID(int account_id)
{
   user_node *u,*temp;
   int count;

   count = 0;
   u = users;

   if (u == NULL)
      return 0;

   /* delete from front of list */
   while (u->account_id == account_id)
   {
      count++;
      temp = u->next;
      FreeMemory(MALLOC_ID_USER,u,sizeof(user_node));
      u = temp;
      users = u;
   }
   
   while (u != NULL)
   {
      temp = u->next;
      if (temp != NULL && temp->account_id == account_id)
      {
	 count++;
	 u->next = temp->next;
	 FreeMemory(MALLOC_ID_USER,temp,sizeof(user_node));
      }
      else
	 u = u->next;
   }
   return count;
}

int DeleteUserByObjectID(int object_id)
{
   user_node *u,*temp;
   int count;

   count = 0;
   u = users;

   if (u == NULL)
      return 0;

   /* delete from front of list */
   if (u->object_id == object_id)
   {
      count++;
      temp = u->next;
      FreeMemory(MALLOC_ID_USER,u,sizeof(user_node));
      u = temp;
      users = u;
   }
   
   while (u != NULL)
   {
      temp = u->next;
      if (temp != NULL && temp->object_id == object_id)
      {
	 count++;
	 u->next = temp->next;
	 FreeMemory(MALLOC_ID_USER,temp,sizeof(user_node));
      }
      else
	 u = u->next;
   }
   return count;
}

int CountUserByAccountID(int account_id)
{
   user_node *u;
   int count;

   count = 0;
   u = users;
   while (u != NULL)
   {
      if (u->account_id == account_id)
	 count++;
      u = u->next;
   }
   return count;
}

user_node * GetUserByObjectID(int object_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->object_id == object_id)
	 return u;
      u = u->next;
   }
   return NULL;
}

void ForEachUser(void (*callback_func)(user_node *u))
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      callback_func(u);
      u = u->next;
   }
}

void ForEachUserByAccountID(void (*callback_func)(user_node *u),int account_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->account_id == account_id)
	 callback_func(u);
      u = u->next;
   }
}

user_node * GetFirstUserByAccountID(int account_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->account_id == account_id)
	 return u;
      u = u->next;
   }
   return NULL;
}

user_node * GetUserByName(char *username)
{
   val_type temp_str_val,ret_val;
   parm_node p[1];

   SetTempString(username,strlen(username));

   temp_str_val.v.tag = TAG_TEMP_STRING;
   temp_str_val.v.data = 0;

   p[0].type = CONSTANT;
   p[0].value = temp_str_val.int_val;
   p[0].name_id = STRING_PARM;

   ret_val.int_val = SendTopLevelBlakodMessage(GetSystemObjectID(),
					       FIND_USER_BY_STRING_MSG,1,p);
   if (ret_val.v.tag != TAG_OBJECT)
      return NULL;

   return GetUserByObjectID(ret_val.v.data);
}

void ChangeUserAccountID(int account_id, int new_account_id)
{
   user_node *u;

   u = users;
   while (u != NULL)
   {
      if (u->account_id == account_id)
         u->account_id = new_account_id;
      u = u->next;
   }
}

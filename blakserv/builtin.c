// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* builtin.c
*

  This module adds certain hard coded accounts if the accounts cannot
  be read in from the file.
  
*/

#include "blakserv.h"

typedef struct
{
	char *name;
	char *password;
	int type;
	char *game_name;
} bi_account;

bi_account bi_accounts[] =
{
	{ "Daenks",  "somethingnew",  ACCOUNT_ADMIN, "Daenks" },
};

enum
{
	NUM_BUILTIN = sizeof(bi_accounts)/sizeof(bi_account)
};

void CreateBuiltInAccounts(void)
{
	int i,account_id,object_id;
	val_type name_val,system_id_val;
	parm_node p[2];
	
	for (i=0;i<NUM_BUILTIN;i++)
	{
		account_node *a;
		
		a = GetAccountByName(bi_accounts[i].name);
		if (a != NULL)
			account_id = a->account_id;
		else
			account_id = CreateAccountSecurePassword(bi_accounts[i].name,bi_accounts[i].password,
			bi_accounts[i].type);
		
		if (bi_accounts[i].game_name != NULL)
		{
			/* make a character for this account */
			
			system_id_val.v.tag = TAG_OBJECT;
			system_id_val.v.data = GetSystemObjectID();
			
			p[0].type = CONSTANT;
			p[0].value = system_id_val.int_val;
			p[0].name_id = SYSTEM_PARM;
			
			name_val.v.tag = TAG_RESOURCE;
			name_val.v.data = AddDynamicResource(bi_accounts[i].game_name);
			
			p[1].type = CONSTANT;
			p[1].value = name_val.int_val;
			p[1].name_id = NAME_PARM;
			
			switch (bi_accounts[i].type)
			{
			case ACCOUNT_GUEST : 
				object_id = CreateObject(GUEST_CLASS,2,p);
				break;
			case ACCOUNT_NORMAL :
				object_id = CreateObject(USER_CLASS,2,p);
				break;
			case ACCOUNT_DM : 
				object_id = CreateObject(DM_CLASS,2,p);
				break;
			case ACCOUNT_ADMIN :
#if 1
				if (i < 2)
					object_id = CreateObject(CREATOR_CLASS,2,p);
				else
#endif
					object_id = CreateObject(ADMIN_CLASS,2,p);
				break;
			} 
			
			if (AssociateUser(account_id,object_id) == False)
				eprintf("CreateBuiltInAccounts had AssociateUser fail, on account %i object %i\n",
				account_id,object_id);
		}
	}
}


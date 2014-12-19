// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * account.h
 *
 */

#ifndef _ACCOUNT_H
#define _ACCOUNT_H

enum { ACCOUNT_NORMAL = 0, ACCOUNT_ADMIN = 1, ACCOUNT_DM = 2, ACCOUNT_GUEST = 3};

typedef struct account_node_struct
{
   int account_id;
   char *name;
   char *password;
   int type;
   int credits;			/* remember, stored as 1/100 of a credit */
   int last_login_time;
   int suspend_time;
   struct account_node_struct *next;
} account_node;

void InitAccount(void);
void ResetAccount(void);
account_node * GetConsoleAccount(void);
int GetNextAccountID(void);
int GetUsedGuestAccounts(void);
Bool CreateAccount(char *name,char *password,int type,int *account_id);
int CreateAccountSecurePassword(const char *name,const char *password,int type);
int RecreateAccountSecurePassword(int account_id,char *name,char *password,int type);
void LoadAccount(int account_id,char *name,char *password,int type,int last_login_time,
		 int suspend_time, int credits);
Bool DeleteAccount(int account_id);
void SetAccountName(account_node *a,char *name);
void SetAccountPassword(account_node *a,char *password);
void SetAccountPasswordAlreadyEncrypted(account_node *a,char *password);
void SetNextAccountID(int accountNum);
account_node * GetAccountByID(int account_id);
account_node * GetAccountByName(const char *name);
account_node * AccountLoginByName(char *name);
void AccountLogoff(account_node *a);
void DoneLoadAccounts(void);
void ForEachAccount(void (*callback_func)(account_node *a));
void DeleteAccountAndAssociatedUsersByID(int account_id);

Bool SuspendAccountAbsolute(account_node *a, int suspend_time);
Bool SuspendAccountRelative(account_node *a, int hours);
int GetActiveAccountCount();

#endif


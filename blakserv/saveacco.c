// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * saveacco.c
 *

 This module saves account information to a file, with the password in
 a cheesy pseudo-encoded format.  See loadacco.c for the format of the
 file.

 */

#include "blakserv.h"

FILE *accofile;

/* local function prototypes */
void SaveEachAccount(account_node *a);

Bool SaveAccounts(char *filename)
{
   if ((accofile = fopen(filename,"wt")) == NULL)
   {
      eprintf("SaveAccounts can't open %s to save accounts!\n",filename);
      return False;
   }

   ForEachAccount(SaveEachAccount);
   fprintf(accofile,"NEXT_ACCOUNT_ID %i\n",GetNextAccountID());
   fclose(accofile);

   return True;
}

void SaveEachAccount(account_node *a)
{
   unsigned char *ptr;
   
   fprintf(accofile,"ACCOUNT %i:%s:",a->account_id,a->name);

   ptr = (unsigned char *) a->password;
   while (*ptr != 0)
   {
      fprintf(accofile,"%02x",*ptr);
      ptr++;
   }

   if (a->password[0] == 0)
      fprintf(accofile,"None");

   fprintf(accofile,":%i:%i:%i:%i\n",a->type,a->last_login_time,
           a->credits,a->suspend_time);
}

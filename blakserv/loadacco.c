// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadacco.c
 *

 This module loads account information from a file.  The file is a
 text file, with comment lines started with a pound sign (#).  An
 account starts with the word "ACCOUNT", then has 7 fields, separated
 by colons (:).  The fields are 

 1) account number
 2) account name
 3) password (a list of comma separated numbers, with a cheesy encoding)
 4) type of account (a number, see account.h)
 5) last login time, in seconds since 1970.
 6) number of credits (in 100ths of a credit)
 7) last download time (of resources)

 Sample:
 ACCOUNT 2:Andrew Kirmse:97,105,70:1:0:1200:0

 */

#include "blakserv.h"

#define MAX_ACCOUNT_LINE 300

int lineno;
static int highestAccount = -1;

/* local function prototypes */
Bool LoadLineAccount(char *account_str,char *name_str,char *password_str,
		     char *type_str,char *last_login_str,char *suspend_str,
		     char *credits_str);

Bool LoadAccounts(char *filename)
{
   FILE *accofile;
   char line[MAX_ACCOUNT_LINE+1];
   char *type_str,*t1,*t2,*t3,*t4,*t5,*t6,*t7;
   int nextAccountID = -1;

   if ((accofile = fopen(filename,"rt")) == NULL)
   {
      eprintf("LoadAccounts can't open %s to load the accounts!\n",
	      filename);
      return False;
   }

   highestAccount = -1;
   lineno = 0;
   while (fgets(line,MAX_ACCOUNT_LINE,accofile))
   {
      lineno++;

      type_str = strtok(line,": \t\n");

      if (type_str == NULL)	/* ignore blank lines */
	 continue;

      t1 = strtok(NULL,":\n");
      t2 = strtok(NULL,":\n");
      t3 = strtok(NULL,":\n");
      t4 = strtok(NULL,":\n");
      t5 = strtok(NULL,":\n");
      t6 = strtok(NULL,":\n");
      t7 = strtok(NULL,":\n");
      /* t7 is suspend_str, note different order from LoadLineAccount args */

      if (*type_str == '#')
	 continue;

      if (!stricmp(type_str,"ACCOUNT"))
      {
	 if (!LoadLineAccount(t1,t2,t3,t4,t5,t7,t6))
	 {
	    fclose(accofile);
	    return False;
	 }
	 else
	    continue;
      }
      else if (0 == stricmp(type_str,"NEXT_ACCOUNT_ID"))
      {
	 nextAccountID = atoi(t1);
	 continue;
      }

      eprintf("LoadAccounts can't handle account file line type %s\n",type_str);
      return False;     
   }

   fclose(accofile);
   if (GetNextAccountID() > nextAccountID)
      nextAccountID = GetNextAccountID();
   SetNextAccountID(nextAccountID);
   /*
   dprintf("LoadAccounts successfully loaded accounts from %s\n",ACCOUNT_FILE);
   */

   return True;
}

Bool LoadLineAccount(char *account_str,char *name_str,char *password_str,
		     char *type_str,char *last_login_str,char *suspend_str,
		     char *credits_str)
{   
   int account_id,type,last_login_time,suspend_time,credits;
   int index;
   char decoded[100];
   char *ptr,*end_password;
   unsigned int ch;

   /* required fields of line */

   if (account_str == NULL || name_str == NULL || password_str == NULL ||
       type_str == NULL || last_login_str == NULL || credits_str == NULL ||
       sscanf(account_str,"%i",&account_id) != 1 ||
       sscanf(type_str,"%i",&type) != 1 ||
       sscanf(last_login_str,"%i",&last_login_time) != 1 ||
       sscanf(credits_str,"%i",&credits) != 1)
   {
      eprintf("LoadLineAccount (%i) found invalid account\n",lineno);
      return False;
   }

   /* optional fields of line */

   suspend_time = 0;
   if (suspend_str)
   {
      sscanf(suspend_str, "%i", &suspend_time);
   }

   /* now decode the password */
   index = 0;
   ptr = password_str;
   end_password = ptr + strlen(ptr);
   while (ptr < end_password && sscanf(ptr,"%02x",&ch) == 1)
   {
      decoded[index++] = ch;
      ptr += 2;
   }
   decoded[index] = 0;

   //dprintf("account %i password %s\n",account_id,decoded);

   if (account_id > highestAccount)
   {
      highestAccount = account_id;
   }

   LoadAccount(account_id,name_str,decoded,type,last_login_time,suspend_time,credits);
   return True;
}



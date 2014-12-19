// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * apndfile.c
 *

 This module appends text to a text file.  Each write has a header
 marking which account the text comes from.  This is only used for
 comments from users.

 */

#include "blakserv.h"

void AppendTextFile(session_node *s,const char *filename,
                    int len,const char *text)
{
   FILE *appendfile;
   char save_name[MAX_PATH+FILENAME_MAX];
   int i;

   sprintf(save_name,"%s%s",ConfigStr(PATH_FORMS),filename);
   if ((appendfile = fopen(save_name,"at")) == NULL)
   {
      eprintf("AppendTextFile can't open %s to write out new text!\n",save_name);
      dprintf("-------------------------------------------\n");
      dprintf("From account %i, %s\n",s->account->account_id,s->account->name);
      dprintf("%s\n\n",TimeStr(GetTime()));
      for (i=0;i<len;i++)
	 dprintf("%c",text[i]);
      dprintf("\n\n");

      return;
   }

   fprintf(appendfile,"-------------------------------------------\n");
   fprintf(appendfile,"From account %i, %s\n",s->account->account_id,s->account->name);
   fprintf(appendfile,"%s\n\n",TimeStr(GetTime()));
   for (i=0;i<len;i++)
      fprintf(appendfile,"%c",text[i]);
   fprintf(appendfile,"\n\n");
   fclose(appendfile);
}

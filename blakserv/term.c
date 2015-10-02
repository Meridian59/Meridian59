// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * term.c
 *

 This module has a couple functions useful when dealing with a user in
 admin mode.

 */

#include "blakserv.h"

void cprintf(int session_id,const char *fmt,...)
{
   char s[8000];
   va_list marker;

   va_start(marker,fmt);
   vsnprintf(s,sizeof(s),fmt,marker);
   va_end(marker);

   TermConvertBuffer(s,sizeof(s));

   SendClientStr(session_id,s);
}

void TermConvertBuffer(char *s,int len_s)
{
   int i;

   /* convert \n's to cr-lf pairs */

   i = 0;
   while (s[i])
   {
      if (s[i] == '\n')
      {
	 // newlines get a carriage return inserted
	 memmove(s+i+2,s+i+1,len_s-i-2);
	 s[i++] = '\r';
	 s[i++] = '\n';
      }
      else if (s[i] == '\r')
      {
         // carriage returns in the original are stripped to avoid making \r\r\n's
	 memmove(s+i,s+i+1,len_s-i-1);
      }
      else
      {
	 i++;
      }
      if (i >= len_s-2)
      {
	 s[len_s-1] = '\0';
	 //eprintf("TermConvertBuffer buffer too small\n");
	 break;
      }
   }
}


const char * GetTagName(val_type val)
{
   static char s[10];

   switch (val.v.tag)
   {
   case TAG_NIL : return "$";
   case TAG_INT : return "INT";
   case TAG_OBJECT : return "OBJECT";
   case TAG_LIST : return "LIST";
   case TAG_RESOURCE : return "RESOURCE";
   case TAG_TIMER : return "TIMER";
   case TAG_SESSION : return "SESSION";  
   case TAG_ROOM_DATA : return "ROOM_DATA";
   case TAG_TEMP_STRING : return "TEMP_STRING";
   case TAG_STRING : return "STRING";
   case TAG_CLASS : return "CLASS";
   case TAG_MESSAGE : return "MESSAGE";
   case TAG_DEBUGSTR : return "DEBUGSTR";
   case TAG_TABLE : return "HASHTABLE";
   case TAG_OVERRIDE : return "OVERRIDE";
   case TAG_EMPTY : return "EMPTY";
   default :
      eprintf("GetTagName warning, can't identify tag %i\n",val.v.tag);
      sprintf(s,"%i",val.v.tag);
      return s;
   }
}

const char * GetDataName(val_type val)
{
   resource_node *r;
   class_node *c;
   static char s[10];
   val_type int_val;

   switch (val.v.tag)
   {
   case TAG_RESOURCE :
      r = GetResourceByID(val.v.data);
      if (r == NULL)
      {
         sprintf(s,"%i",val.v.data);
         return s;
      }
      if (r->resource_name == NULL)
      {
         sprintf(s,"%i",r->resource_id);
         return s;
      }
      return r->resource_name;

   case TAG_CLASS :
      c = GetClassByID(val.v.data);
      if (c == NULL)
      {
         eprintf("GetTagData error, can't find class id %i\n",val.v.data);
         sprintf(s,"%i",val.v.data);
         return s;
      }
      return c->class_name;

   case TAG_MESSAGE :
      // Message tag saving *is* supported, however is not advised as message ID
      // numbers can change and any resulting call could have undesired effects.
      eprintf("GetTagData error, message tag saving not supported %i\n",val.v.data);

      /* fall through */
   default :
      /* write as positive int so no problem reading in */
      int_val.v.tag = val.v.tag;
      int_val.v.data = val.v.data;
      sprintf(s,"%i",int_val.v.data);
      return s;
   }
}


int GetTagNum(const char *tag_str)
{
   int retval;
   char ch;

   ch = toupper(tag_str[0]);

   if (0 == stricmp(tag_str,"$"))
      return TAG_NIL;
   if (ch == 'N')
      return TAG_NIL;
   if (ch == 'O')
      return TAG_OBJECT;
   if (ch == 'C')
      return TAG_CLASS;
   if (ch == 'M')
      return TAG_MESSAGE;
   if (ch == 'L')
      return TAG_LIST;
   if (ch == 'T')
      return TAG_TIMER;
   if (ch == 'Q')
      return TAG_TEMP_STRING;
   if (ch == 'H')
      return TAG_TABLE;
   if (0 == stricmp(tag_str,"EMPTY"))
      return TAG_EMPTY;
   if (ch == 'I')
      return TAG_INT;

   if (0 == stricmp(tag_str,"ROOM_DATA"))
      return TAG_ROOM_DATA;
   if (ch == 'R')
      return TAG_RESOURCE;

   if (0 == stricmp(tag_str,"SESSION"))
      return TAG_SESSION;
   if (ch == 'S')
      return TAG_STRING;

   if (sscanf(tag_str,"%i",&retval) != 1)
      return INVALID_TAG;

   return retval;
}

int GetDataNum(int tag_val,const char *data_str)
{
   resource_node *r;
   class_node *c;
   int retval;

   switch (tag_val)
   {
   case TAG_RESOURCE :
      r = GetResourceByName(data_str);
      if (r != NULL)
      {
         retval = r->resource_id;
         break;
      }

      if (sscanf(data_str,"%i",&retval) != 1)
         retval = INVALID_DATA;

      break;

   case TAG_CLASS :
      c = GetClassByName(data_str);
      if (c != NULL)
      {
         retval = c->class_id;
         break;
      }

      if (sscanf(data_str,"%i",&retval) != 1)
         retval = INVALID_DATA;

      break;

   case TAG_MESSAGE :
      retval = GetIDByName(data_str);
      if (retval == INVALID_ID)
         retval = INVALID_DATA;
      break;

   case TAG_TEMP_STRING :
      eprintf( "Recognized TAG_TEMP_STRING type, attempting to load..." );
      retval = INVALID_DATA;
      break;

   default :
      if (sscanf(data_str,"%i",&retval) != 1)
         retval = INVALID_DATA;
   }

   return retval;
}

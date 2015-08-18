// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * string.c
 *

 This module maintains a dynamically sized array with the string nodes
 for the Blakod.  It also has a temp string, for things from the
 client like say commands which are not stored by the Blakod.

 */

#include "blakserv.h"

string_node *strings;
int num_strings,max_strings;

/* this is for say commands, which are not saved */
string_node temp_str;

/* local function prototypes */
int AllocateString();

void InitString()
{
   num_strings = 0;
   max_strings = INIT_STRING_NODES;
   strings = (string_node *)AllocateMemory(MALLOC_ID_STRING,max_strings*sizeof(string_node));

   /* allocate max client bytes for temp string because max string len is < this */
   temp_str.data = (char *)AllocateMemory(MALLOC_ID_STRING,LEN_TEMP_STRING+1);
   temp_str.len_data = 0;
   temp_str.data[temp_str.len_data] = '\0';
}

void ResetString()
{
   int i,old_strings;

   for (i=0;i<num_strings;i++)
      if (strings[i].data)
         FreeMemory(MALLOC_ID_STRING,strings[i].data,strings[i].len_data+1);

   old_strings = max_strings;
   num_strings = 0;  
   max_strings = INIT_STRING_NODES;
   strings = (string_node *)
      ResizeMemory(MALLOC_ID_STRING,strings,old_strings*sizeof(string_node),
		   max_strings*sizeof(string_node));
}

int GetStringsUsed()
{
   return num_strings;
}

int AllocateString()
{
   int old_strings;

   if (num_strings == max_strings)
   {
      old_strings = max_strings;
      max_strings = max_strings * 2;
      strings = (string_node *)
	 ResizeMemory(MALLOC_ID_STRING,strings,old_strings*sizeof(string_node),
		      max_strings*sizeof(string_node));      
      lprintf("AllocateStringNode resized to %i string nodes\n",max_strings);
   }

   strings[num_strings].data = NULL;
   strings[num_strings].len_data = 0;
   
   return num_strings++;
}

string_node *GetStringByID(int string_id)
{
   if (string_id < 0 || string_id >= num_strings)
   {
      eprintf("GetStringByID can't retrieve invalid string %i\n",string_id);
      return NULL;
   }
   return &strings[string_id];
}

Bool IsStringByID(int string_id)
{
   if (string_id < 0 || string_id >= num_strings)
      return False;

   return True;
}

int CreateString(const char *new_str)
{
   return CreateStringWithLen(new_str,strlen(new_str));
}

int CreateStringWithLen(const char *buf,int len)
{
   int string_id;
   string_node *snod;
   
   /* note:  new_str is NOT null-terminated */
   string_id = AllocateString();
   snod = GetStringByID(string_id);

   snod->data = (char *)AllocateMemory(MALLOC_ID_STRING,len+1);
   memcpy(snod->data,buf,len);
   snod->len_data = len;
   snod->data[snod->len_data] = '\0';
   
   return string_id;
}

Bool LoadBlakodString(FILE *f,int len_str,int string_id)
{
   string_node *snod;

   /* note:  new_str is not a null-terminated string */
   if (AllocateString() != string_id)
   {
      eprintf("LoadString didn't make string id %i\n",string_id);
      return False;
   }
   snod = GetStringByID(string_id);

   if (len_str != 0)
   {
      snod->data = (char *)AllocateMemory(MALLOC_ID_STRING,len_str+1);
      if (!fread(snod->data, 1, len_str, f))
         return False;
      snod->data[len_str] = '\0';
   }
   else
   {
      snod->data = NULL;
   }

   snod->len_data = len_str;
   
   return True;
}

void ForEachString(void (*callback_func)(string_node *snod,int string_id))
{
   int i;

   for (i=0;i<num_strings;i++)
      callback_func(&strings[i],i);
}

void FreeString(int string_id) /* for garbage collection */
{
   string_node *snod;

   snod = GetStringByID(string_id);
   if (snod == NULL)
   {
      eprintf("FreeString can't find %i\n",string_id);
      return;
   }

   if (snod->data != NULL)
   {
      FreeMemory(MALLOC_ID_STRING,snod->data,snod->len_data+1);
   }
   else
   {
      /* happens when blank string saved, loaded, then garbage collected */
      /* eprintf("FreeString can't free NULL string %i\n",string_id); */
   }

   snod->data = NULL;
   snod->len_data = 0;
}

void MoveStringNode(int dest_id,int source_id) /* for garbage collection */
{
   string_node *source,*dest;
   
   source = GetStringByID(source_id);
   if (source == NULL)
   {
      eprintf("MoveString can't find source %i, total death end game\n",
	      source_id);
      FlushDefaultChannels();
      return;
   }

   dest = GetStringByID(dest_id);
   if (dest == NULL)
   {
      eprintf("MoveString can't find dest %i, total death end game\n",
	      dest_id);
      FlushDefaultChannels();
      return;
   }

   /* we're guaranteed dest_id <= source_id.  We don't need to do anything
    * if they're the same.
    */

   if (dest_id == source_id)
      return;

   if (dest->data != NULL)
      FreeMemory(MALLOC_ID_STRING,dest->data,dest->len_data+1);

   dest->data = source->data;
   dest->len_data = source->len_data;

   source->data = NULL;
   source->len_data = 0;
}

void SetNumStrings(int new_num_strings) /* for garbage collecting */
{
   num_strings = new_num_strings;
}

int GetNumStrings() /* for saving */
{
   return num_strings;
}

void SetString(string_node *snod,char *buf,int len)
{
   if (snod == &temp_str)
   {
      SetTempString(buf,len);
      return;
   }
   FreeMemory(MALLOC_ID_STRING,snod->data,snod->len_data+1);
   snod->data = (char *)AllocateMemory(MALLOC_ID_STRING,len+1);
   memcpy(snod->data,buf,len);
   snod->len_data = len;
   snod->data[snod->len_data] = '\0';
}

void SetTempString(char *buf,int len)
{
  len = std::min(len, LEN_TEMP_STRING);
   memcpy(temp_str.data,buf,len);
   temp_str.len_data = len;
   temp_str.data[temp_str.len_data] = '\0';
}

string_node * GetTempString()
{
   return &temp_str;
}

void ClearTempString(void)
{
   *temp_str.data = '\0';
   temp_str.len_data = 0;
}

void AppendTempString(const char *buf, int len)
{
  len = std::min(len, LEN_TEMP_STRING-temp_str.len_data);
   if (len <= 0)
      return;
   memcpy(temp_str.data + temp_str.len_data, buf, len);
   temp_str.len_data += len;
   temp_str.data[temp_str.len_data] = '\0';
}

void AppendNumToTempString(int iNum)
{
   char numbuf[20];

   sprintf(numbuf, "%d", iNum);
   AppendTempString(numbuf, strlen(numbuf));
}

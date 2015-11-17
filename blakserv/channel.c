// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * channel.c
 *

 This module has all the functions dealing with writing to channels.
 Based on the configuration, the channels may or not be written to
 files, but they are always shown on the interface (chanbuf.c).  

 */

#include "blakserv.h"

typedef struct
{
   int channel_id;
   int disk_config_id;
   const char *file_name;
} channel_table_type;

channel_table_type channel_table[] =
{
{ CHANNEL_D, CHANNEL_DEBUG_DISK, DEBUG_FILE, },
{ CHANNEL_E, CHANNEL_ERROR_DISK, ERROR_FILE, },
{ CHANNEL_L, CHANNEL_LOG_DISK,   LOG_FILE,   },
{ CHANNEL_G, CHANNEL_GOD_DISK,   GOD_FILE,   },
{ CHANNEL_A, CHANNEL_ADMIN_DISK, ADMIN_FILE, },
};

channel_node channel[NUM_CHANNELS];

/* local function prototypes */
void WriteStrChannel(int channel_id,char *s);
FILE *CreateFileChannel(int channel_id);

void OpenDefaultChannels()
{
   int i;
   for (i=0;i<NUM_CHANNELS;i++)
   {
      if (ConfigBool(channel_table[i].disk_config_id))
      {
         channel[i].file = CreateFileChannel(i);
         if (channel[i].file == NULL)
            StartupPrintf("OpenDefaultChannels couldn't open file %s\n",
                          channel_table[i].file_name);
      }
      else
         channel[i].file = NULL;
   }
}

void CloseDefaultChannels()
{
   int i;

   for (i=0;i<NUM_CHANNELS;i++)
   {
      if (channel[i].file != NULL)
      {
         fclose(channel[i].file);
         channel[i].file = NULL;
      }
   }
}

void FlushDefaultChannels()
{
   int i;

   for (i=0;i<NUM_CHANNELS;i++)
      if (channel[i].file != NULL)
         fflush(channel[i].file);
}

void dprintf(const char *fmt,...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   sprintf(s,"%s|",TimeStr(GetTime()));

   va_start(marker,fmt);
   vsprintf(s+strlen(s),fmt,marker);
   va_end(marker);

   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_D,s);
}

void eprintf(const char *fmt,...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   sprintf(s,"%s | ",TimeStr(GetTime()));

   va_start(marker,fmt);
   vsprintf(s+strlen(s),fmt,marker);
   va_end(marker);

   WriteStrChannel(CHANNEL_E,s);
}

void bprintf(const char *fmt,...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   sprintf(s,"%s | [%s] ",TimeStr(GetTime()),BlakodDebugInfo());

   va_start(marker,fmt);
   vsprintf(s+strlen(s),fmt,marker);
   va_end(marker);

   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_E,s);
}

void lprintf(const char *fmt,...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   sprintf(s,"%s | ",TimeStr(GetTime()));

   va_start(marker,fmt);
   vsprintf(s+strlen(s),fmt,marker);
   va_end(marker);

   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_L,s);
}

void gprintf(const char *fmt,...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   sprintf(s,"%s | ",TimeStr(GetTime()));

   va_start(marker,fmt);
   vsprintf(s+strlen(s),fmt,marker);
   va_end(marker);

   char *excludedword1 = "account";
   char *excludedword2 = "ACCOUNT";

   if (strstr(s, excludedword1) != NULL || strstr(s, excludedword2) != NULL)
   {
	   sprintf(s, "%s | Line excluded due to personal information.", TimeStr(GetTime()));
   }

   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_G,s);
}

void aprintf(const char *fmt, ...)
{
   char s[BUFFER_SIZE];
   va_list marker;

   va_start(marker, fmt);
   vsnprintf(s, sizeof(s), fmt, marker);
   va_end(marker);

   WriteStrChannel(CHANNEL_A, s);
   AdminBufferSend(s, strlen(s));
}

void WriteStrChannel(int channel_id,char *s)
{
   if (channel[channel_id].file != NULL)
   {
      fwrite(s, 1, strlen(s), channel[channel_id].file);
      if (ConfigBool(CHANNEL_FLUSH))
         fflush(channel[channel_id].file);
   }

   WriteChannelBuffer(channel_id,s);
}

FILE *CreateFileChannel(int channel_id)
{
   char channel_file[MAX_PATH+FILENAME_MAX];
   FILE *pFile;

   sprintf(channel_file,"%s%s",ConfigStr(PATH_CHANNEL),channel_table[channel_id].file_name);
   pFile = fopen(channel_file, "ab");

   return pFile;
}

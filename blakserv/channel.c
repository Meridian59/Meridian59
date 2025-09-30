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
{ CHANNEL_D, CHANNEL_DEBUG_DISK, DEBUG_FILE_BASE, },
{ CHANNEL_E, CHANNEL_ERROR_DISK, ERROR_FILE_BASE, },
{ CHANNEL_L, CHANNEL_LOG_DISK,   LOG_FILE_BASE,   },
};

channel_node channel[NUM_CHANNELS];

/* local function prototypes */
void WriteStrChannel(int channel_id,char *s);
FILE *CreateFileChannel(int channel_id);

std::string obj_to_string(int tag, INT64 data)
{
  return std::to_string(tag) + "," + std::to_string(data);
}

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

// Max length of a time string
static const int MAX_TIME_SIZE = 100;

void dprintf(const char *fmt,...)
{
   char s[2000];
   va_list marker;

   snprintf(s, sizeof(s), "%s|",TimeStr(GetTime()).c_str());

   va_start(marker,fmt);
   vsnprintf(s+strlen(s), sizeof(s) - MAX_TIME_SIZE, fmt,marker);
   va_end(marker);

   TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs */
   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_D,s);
}

void eprintf(const char *fmt,...)
{
   char s[2000];
   va_list marker;

   snprintf(s, sizeof(s), "%s | ",TimeStr(GetTime()).c_str());

   va_start(marker,fmt);
   vsnprintf(s+strlen(s), sizeof(s) - MAX_TIME_SIZE, fmt,marker);
   va_end(marker);

   TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs */

   WriteStrChannel(CHANNEL_E,s);
}

void bprintf(const char *fmt,...)
{
   char s[1000];
   va_list marker;

   snprintf(s, sizeof(s), "%s | [%s] ",TimeStr(GetTime()).c_str(),BlakodDebugInfo());

   va_start(marker,fmt);
   vsnprintf(s+strlen(s), sizeof(s) - MAX_TIME_SIZE, fmt,marker);
   va_end(marker);

   TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs */
   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_E,s);
}

void lprintf(const char *fmt,...)
{
   char s[1000];
   va_list marker;

   snprintf(s, sizeof(s), "%s | ",TimeStr(GetTime()).c_str());

   va_start(marker,fmt);
   vsnprintf(s+strlen(s), sizeof(s) - MAX_TIME_SIZE, fmt,marker);
   va_end(marker);

   TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs */
   if (s[strlen(s)-1] != '\n')
      strcat(s,"\r\n");

   WriteStrChannel(CHANNEL_L,s);
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

   char date_str[500];

   time_t t;
   struct tm *time_struct;

   t = time(NULL);
   time_struct = localtime(&t);
   if (time_struct == NULL) {
       strncpy(date_str, "unknown", sizeof(date_str));
   } else {
       strftime(date_str, sizeof(date_str), "%Y-%m-%d", time_struct);
   }
   snprintf(channel_file, sizeof(channel_file),
            "%s%s-%s.txt",ConfigStr(PATH_CHANNEL),channel_table[channel_id].file_name, date_str);
   pFile = fopen(channel_file, "ab");

   return pFile;
}

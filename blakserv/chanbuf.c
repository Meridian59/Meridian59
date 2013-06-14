// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * chanbuf.c
 *
 
 This module keeps a circular queue of buffers for data written to
 channels.  The interface reads these buffers pretty often to add
 newly written text to the interface.  We keep a lock on the buffers
 because the main thread writes data here and the interface thread
 reads it.

 */

#include "blakserv.h"

/* this is a circular queue of buffers */

#define MAX_BUFFERS 60

channel_buffer_node channel_buffers[MAX_BUFFERS];
int head_buffer;
int tail_buffer;

CRITICAL_SECTION csChannel_buffers; /* protects all the variables above */

void InitChannelBuffer()
{
   head_buffer = 0;
   tail_buffer = 0;
   InitializeCriticalSection(&csChannel_buffers);
}

void WriteChannelBuffer(int channel_id,char *s)
{
   const char *str = s;

   EnterCriticalSection(&csChannel_buffers);

   if (((head_buffer+1) % MAX_BUFFERS) != tail_buffer)
   {
      channel_buffers[head_buffer].channel_id = channel_id;
      if (strlen(str) > CHANBUF_SIZE)
			str = "<<WriteChannelBuffer got string too long to display>>";

      // Copy all but trailing \n's and \r's.
      strcpy(channel_buffers[head_buffer].buf,str);
      char *ptr = channel_buffers[head_buffer].buf;
      ptr += strlen(ptr);
      while (ptr > channel_buffers[head_buffer].buf &&
				 (*(ptr-1) == '\n' || *(ptr-1) == '\r'))
			*(--ptr) = '\0';

      head_buffer = (head_buffer+1) % MAX_BUFFERS;
   }
   LeaveCriticalSection(&csChannel_buffers);

   InterfaceUpdateChannel();
}

/* this is executed in the interface thread */
Bool IsNewChannelText()
{
   Bool is_new;

   EnterCriticalSection(&csChannel_buffers);
   is_new = (Bool)(head_buffer != tail_buffer);
   LeaveCriticalSection(&csChannel_buffers);
   
   return is_new;
}

/* these two functions must be paired!!!!  And you must call IsNewChannelText
 * first to make sure you can do this */

/* this is executed in the interface thread */
channel_buffer_node * GetChannelBuffer()
{
   EnterCriticalSection(&csChannel_buffers);

   return &channel_buffers[tail_buffer];
}

/* this is executed in the interface thread */
void DoneChannelBuffer()
{
   tail_buffer = (tail_buffer+1) % MAX_BUFFERS;

   LeaveCriticalSection(&csChannel_buffers);
}


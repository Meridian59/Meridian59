// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * channel.h
 *
 */

#ifndef _CHANNEL_H
#define _CHANNEL_H

typedef struct
{
   FILE *file;
} channel_node;

enum
{
   CHANNEL_D,		/* debug info */
   CHANNEL_E,		/* errors */
   CHANNEL_L,		/* system log */
   CHANNEL_G,		/* god log */
   CHANNEL_A,     /* admin log */
   NUM_CHANNELS
};

void OpenDefaultChannels(void);
void CloseDefaultChannels(void);
void FlushDefaultChannels(void);

void aprintf(const char *fmt,...); // Admin window output, not displayed in channels.
void dprintf(const char *fmt,...);
void eprintf(const char *fmt,...);
void bprintf(const char *fmt,...);  /* blakod errors, goes to channel e */
void lprintf(const char *fmt,...);
void gprintf(const char *fmt,...);

#endif

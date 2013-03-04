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
   NUM_CHANNELS
};

void OpenDefaultChannels(void);
void CloseDefaultChannels(void);
void FlushDefaultChannels(void);

void __cdecl dprintf(char *fmt,...);
void __cdecl eprintf(char *fmt,...);
void __cdecl bprintf(char *fmt,...);	/* blakod errors, goes to channel e */
void __cdecl lprintf(char *fmt,...);

#endif

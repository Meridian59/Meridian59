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

// Give warnings on these functions if arguments don't match format (gcc only)
#if defined(__GNUC__)
    #define PRINTF_FORMAT(string_index, first_to_check) \
        __attribute__((format(printf, string_index, first_to_check)))
#elif defined(_MSC_VER)
    #define PRINTF_FORMAT(string_index, first_to_check)
#else
    #define PRINTF_FORMAT(string_index, first_to_check)
#endif

void dprintf(const char *fmt,...) PRINTF_FORMAT(1,2);
void eprintf(const char *fmt,...) PRINTF_FORMAT(1,2);
void bprintf(const char *fmt,...) PRINTF_FORMAT(1,2);  /* blakod errors, goes to channel e */
void lprintf(const char *fmt,...) PRINTF_FORMAT(1,2);

#endif

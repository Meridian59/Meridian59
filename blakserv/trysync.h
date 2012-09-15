// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * trysync.h
 *
 */

#ifndef _TRYSYNC_H
#define _TRYSYNC_H

#define TRYSYNC_TIMEOUT_SECS 7

extern unsigned char tell_cli_str[];
#define LENGTH_TELL_CLI 9

extern unsigned char detect_str[];
#define LENGTH_DETECT 9

typedef struct
{
   int detect_index;
} trysync_data;

#endif

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * resync.h
 *
 */

#ifndef _RESYNC_H
#define _RESYNC_H


extern unsigned char beacon_str[];
enum
{
   LENGTH_BEACON = 9
};


#define RESYNC_NOSYNC_SECS 600

typedef struct
{
   int beacon_index;
} resync_data;

#endif

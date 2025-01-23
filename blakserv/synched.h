// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * synched.h
 *
 */

#ifndef _SYNCHED_H
#define _SYNCHED_H

typedef struct
{
   int failed_tries;
   int download_count; /* 0 if not downloading, otherwise count */
} synched_data;

#endif

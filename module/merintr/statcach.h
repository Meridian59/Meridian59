// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statcach.h:  Header file for statcach.c
 */

#ifndef _STATCACH_H
#define _STATCACH_H

void StatCacheCreate(void);
void StatCacheDestroy(void);
Bool StatCacheGetEntry(int group, list_type *stats);
void StatCacheSetEntry(int group, list_type stats);
void StatCacheSetSize(int size);
Statistic *StatCacheUpdate(BYTE group, Statistic *s);

#endif /* #ifndef _STATCACH_H */

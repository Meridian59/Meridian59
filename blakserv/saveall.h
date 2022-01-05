// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * saveall.h
 *
 */

#ifndef _SAVEALL_H
#define _SAVEALL_H

// Returns timestamp of save
INT64 SaveAll(void);
void SaveControlFile(INT64 save_time);

#endif

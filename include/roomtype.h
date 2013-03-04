// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomtype.h:  
 */

#ifndef _ROOMTYPE_H
#define _ROOMTYPE_H

/* Room contents to draw */
typedef struct
{
   short rows;             /* Size of room in grid squares */
   short cols;
   int  width, height;    /* Size of room in FINENESS units */

   unsigned char **grid;           /* Array that tells whether its legal to move between adjacent squares 
			   * (used only in server) */
   unsigned char **flags;          /* Array that gives per-square flags 
			   * (used only in server) */
   unsigned char **monster_grid;   /* Array that tells whether its legal for monsters to move between adjacent squares  (used only in server) */
   int bkgnd;           /* Resource ID of background bitmap; 0 if none */
   unsigned char ambient_light;    /* Intensity of ambient light, 0 = min, 15 = max */

   int security;          /* Security number, unique to each roo file, to ensure that client
                             loads the correct roo file */
} room_type;

#endif /* #ifndef _ROOMTYPE_H */

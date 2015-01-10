// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * project.h:  Header file for project.c
 */

#ifndef _PROJECT_H
#define _PROJECT_H

/* Flags bit values for Projectile structure */
#define PROJ_FLAG_FOLLOWGROUND	 0x0001

typedef struct {
   ID      icon_res;             // Icon resource of bitmap
   Animate animate;             
   Motion  motion;               // State of projectile's motion (includes position)
   int     angle;                // Angle projectile is traveling
   WORD	   flags;		 // Projectile flags
   WORD	   reserved;		 // Reserved for future use
   BYTE    translation;          // Palette translation
   BYTE	   effect;		 // Display effect (like translucency)
   d_lighting	dLighting;
} Projectile;

void ProjectileAdd(Projectile *p, ID source_obj, ID dest_obj, BYTE speed, WORD flags, WORD reserved);
Bool ProjectilesMove(int dt);
void RadiusProjectileAdd(Projectile *p, ID source_obj, BYTE speed, WORD flags, WORD reserved, BYTE range, BYTE number);
#endif /* #ifndef _PROJECT_H */

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * boverlay.h:  Header file for boverlay.c
 */

#ifndef _BOVERLAY_H
#define _BOVERLAY_H

typedef struct {
   object_node obj;               /* Basic info on overlay (ID, icon, animation) */
   int         x;                 /* Angle on background to display overlay */
   int         y;                 /* Vertical position of overlay (0 = horizon, positive = up) */
   BOOL	       drawn;		  /* Was this drawn on the last frame? */
   RECT	       rcScreen;	  /* Screen coordinates if drawn */
} BackgroundOverlay;

void BackgroundOverlaysReset(void);
void BackgroundOverlayDestroyAndFree(BackgroundOverlay *overlay);
void BackgroundOverlayAdd(BackgroundOverlay *overlay);
void BackgroundOverlayRemove(ID id);
void BackgroundOverlayChange(BackgroundOverlay *overlay);

#endif /* #ifndef _BOVERLAY_H */

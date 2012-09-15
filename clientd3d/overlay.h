// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * overlay.h:  Header file for overlay.c
 */

#ifndef _OVERLAY_H
#define _OVERLAY_H

void SetPlayerOverlay(char hotspot, object_node *poverlay);
void PlayerOverlaysFree(void);

void DrawPlayerOverlays(void);

#endif /* #ifndef _OVERLAY_H */

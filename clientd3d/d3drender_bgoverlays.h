// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// Responsible for rendering background overlays such as the Sun and Moon within the game's environment.
//
#ifndef _D3DRENDERBGOVERLAYS_H
#define _D3DRENDERBGOVERLAYS_H

void D3DRenderBackgroundOverlays(d3d_render_pool_new* pPool, int angleHeading, int anglePitch, room_type* room, Draw3DParams* params);

#endif	/* #ifndef _D3DRENDERBGOVERLAYS_H */

// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// Render area visual effects such as sandstorm, rain, etc as well as visual effects such 
// as being dazzled or blinded are provided here.
//
#ifndef _D3DRENDERFX_H
#define _D3DRENDERFX_H

void D3DFxInit();
void D3DRenderParticles();
void D3DPostOverlayEffects(d3d_render_pool_new *pPool);
void D3DFxBlurWaver();

#endif	/* #ifndef _D3DRENDERFX_H */

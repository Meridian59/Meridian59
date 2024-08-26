// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render skybox focusing on rendering a skybox and managing the background textures 
// associated with different in-game environments. A skybox is a large cube that surrounds 
// the player, with textures applied to its inner faces to create the illusion of a 
// distant 3D environment. 
//
#ifndef _D3DRENDERSKYBOX_H
#define _D3DRENDERSKYBOX_H

void D3DRenderSkyBoxBegin();
void D3DRenderSkyBox(room_type* room, Draw3DParams* params, room_contents_node* pRNode, 
	int& angleHeading, int& anglePitch);
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch);
void D3DRenderSkyBoxShutdown();

#endif	/* #ifndef _D3DRENDERSKYBOX_H */

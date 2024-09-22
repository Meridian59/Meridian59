// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render world interface is responsible for rendering the 3d game world - 
// it's responsible for drawing the elements of the BSP tree. The 3d world is always drawn 
// when the d3drenderer `draw_world` is true. It  also includes the
// functionality to build and maintain the 3d geometry for the world.
//
#ifndef _D3DRENDERWORLD_H
#define _D3DRENDERWORLD_H

void D3DRenderGeometry(room_type* room, Bool gD3DRedrawAll);
void D3DRenderWorld(room_type* room, Draw3DParams* params, room_contents_node* pRNode);

#endif	/* #ifndef _D3DRENDERWORLD_H */

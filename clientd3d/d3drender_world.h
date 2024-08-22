// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render world interface is responsible for rendering the 3d game world
// (including buildings, scenery, roads and so on). The 3d world is always drawn 
// when the d3drenderer `draw_world` is true. It  also includes the
// functionality to build and maintain the 3d geometry for the world.
//
#ifndef _D3DRENDERWORLD_H
#define _D3DRENDERWORLD_H

void D3DRenderWorld(room_type* room, Draw3DParams* params, room_contents_node* pRNode);
void D3DGeometryBuildNew(room_type* room, d3d_render_pool_new* pPool, bool transparent_pass);
void GeometryUpdate(d3d_render_pool_new* pPool, d3d_render_cache_system* pCacheSystem);

#endif	/* #ifndef _D3DRENDERWORLD_H */

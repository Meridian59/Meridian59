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

struct SkyboxRenderParams {
    LPDIRECT3DVERTEXDECLARATION9 decl1dc;
    d3d_driver_profile gD3DDriverProfile;
    d3d_render_pool_new gWorldPool;
    d3d_render_cache_system gWorldCacheSystem;
    room_type current_room;

    SkyboxRenderParams(
        LPDIRECT3DVERTEXDECLARATION9 decl1dc,
        d3d_driver_profile driver_profile,
        d3d_render_pool_new pool,
        d3d_render_cache_system cache_system,
        room_type& current_room)
        : decl1dc(decl1dc),
          gD3DDriverProfile(driver_profile),
          gWorldPool(pool),
          gWorldCacheSystem(cache_system),
          current_room(current_room)
    {}
};

bool D3DRenderSkyBoxBegin(SkyboxRenderParams& params);
void D3DRenderSkyBox(room_type* room, Draw3DParams* params, room_contents_node* pRNode,
    int angleHeading, int anglePitch, D3DMATRIX view);
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch);
void D3DRenderSkyBoxShutdown();

#endif	/* #ifndef _D3DRENDERSKYBOX_H */

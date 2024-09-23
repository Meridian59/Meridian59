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

struct skybox_render_object {
    LPDIRECT3DVERTEXDECLARATION9& decl1dc;
    room_type& current_room;
    d3d_driver_profile& gD3DDriverProfile;
    d3d_render_pool_new& gWorldPool;
    d3d_render_cache_system& gWorldCacheSystem;
    D3DMATRIX& view;
    Bool& gD3DRedrawAll;

    skybox_render_object(
        LPDIRECT3DVERTEXDECLARATION9& decl1dc_ref,
        room_type& current_room_ref,
        d3d_driver_profile& driver_profile_ref,
        d3d_render_pool_new& pool_ref,
        d3d_render_cache_system& cache_system_ref,
        D3DMATRIX& view_ref,
        Bool& redraw_all_ref)
        : decl1dc(decl1dc_ref),
          current_room(current_room_ref),
          gD3DDriverProfile(driver_profile_ref),
          gWorldPool(pool_ref),
          gWorldCacheSystem(cache_system_ref),
          view(view_ref),
          gD3DRedrawAll(redraw_all_ref)
    {}
};

void D3DRenderSkyBoxBegin(skybox_render_object* skybox_render_obj_param);
void D3DRenderSkyBox(room_type* room, Draw3DParams* params, room_contents_node* pRNode,
    int& angleHeading, int& anglePitch);
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch);
void D3DRenderSkyBoxShutdown();

#endif	/* #ifndef _D3DRENDERSKYBOX_H */

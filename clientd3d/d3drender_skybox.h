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
    LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
    d3d_driver_profile driverProfile;
    mutable d3d_render_pool_new renderPool;
    mutable d3d_render_cache_system cacheSystem;

    SkyboxRenderParams(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        d3d_driver_profile driverProfileParam,
        d3d_render_pool_new renderPoolParam,
        d3d_render_cache_system cacheSystemParam)
        : vertexDeclaration(vertexDeclarationParam),
          driverProfile(driverProfileParam),
          renderPool(renderPoolParam),
          cacheSystem(cacheSystemParam)
    {}
};

bool D3DRenderUpdateSkyBox(DWORD background);
void D3DRenderSkyBox(Draw3DParams* params, int angleHeading, int anglePitch, const D3DMATRIX& view, 
    const SkyboxRenderParams& skyboxRenderParams);
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch);
void D3DRenderSkyBoxShutdown();

#endif	/* #ifndef _D3DRENDERSKYBOX_H */

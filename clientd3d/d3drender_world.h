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

struct WorldCacheSystemParams {

    d3d_render_cache_system* worldCacheSystem;
    d3d_render_cache_system* worldCacheSystemStatic;

    d3d_render_cache_system* lMapCacheSystem;
    d3d_render_cache_system* lMapCacheSystemStatic;

    d3d_render_cache_system* wallMaskCacheSystem; // geometry

    WorldCacheSystemParams(
        d3d_render_cache_system* worldCacheSystemParam,
        d3d_render_cache_system* worldCacheSystemStaticParam,
        d3d_render_cache_system* lMapCacheSystemParam,
        d3d_render_cache_system* lMapCacheSystemStaticParam,
        d3d_render_cache_system* wallMaskCacheSystemParam
    )
        : worldCacheSystem(worldCacheSystemParam),
          worldCacheSystemStatic(worldCacheSystemStaticParam),
          lMapCacheSystem(lMapCacheSystemParam),
          lMapCacheSystemStatic(lMapCacheSystemStaticParam),
          wallMaskCacheSystem(wallMaskCacheSystemParam)
    {}
};

struct WorldPoolParams {

	d3d_render_pool_new* worldPool;
	d3d_render_pool_new* worldPoolStatic;

	d3d_render_pool_new* lMapPool;
	d3d_render_pool_new* lMapPoolStatic;

	d3d_render_pool_new* wallMaskPool; // geometry

    WorldPoolParams(
        d3d_render_pool_new* worldPoolParam,
        d3d_render_pool_new* worldPoolStaticParam,
        d3d_render_pool_new* lMapPoolParam,
        d3d_render_pool_new* lMapPoolStaticParam,
        d3d_render_pool_new* wallMaskPoolParam
    )
        : worldPool(worldPoolParam),
            worldPoolStatic(worldPoolStaticParam),
            lMapPool(lMapPoolParam),
            lMapPoolStatic(lMapPoolStaticParam),
            wallMaskPool(wallMaskPoolParam)
    {}
};

struct WorldPropertyParams {
    
    LPDIRECT3DTEXTURE9 noLookThroughTexture;
    LPDIRECT3DTEXTURE9 lightOrangeTexture;

	WorldPropertyParams(LPDIRECT3DTEXTURE9 noLookThroughTextureParam,
        LPDIRECT3DTEXTURE9 lightOrangeTextureParam,
        player_info* playerParam)
		: noLookThroughTexture(noLookThroughTextureParam),
		  lightOrangeTexture(lightOrangeTextureParam)
	{}
};

struct WorldRenderParams {

    LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
    LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationSecondary;
    const d3d_driver_profile& driverProfile;

    const WorldCacheSystemParams& cacheSystemParams;
    const WorldPoolParams& poolParams;

    const D3DMATRIX& view;
    const D3DMATRIX& proj;

    room_type* room;
    Draw3DParams* params;

    WorldRenderParams(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationSecondaryParam,
        d3d_driver_profile& driverProfileParam,
        const WorldCacheSystemParams& cacheSystemParamsParam,
        const WorldPoolParams& poolParamsParam,
        const D3DMATRIX& viewParam,
        const D3DMATRIX& projParam,
        room_type* roomParam
    )
        : vertexDeclaration(vertexDeclarationParam),
          vertexDeclarationSecondary(vertexDeclarationSecondaryParam),
          driverProfile(driverProfileParam),
          cacheSystemParams(cacheSystemParamsParam),
          poolParams(poolParamsParam),
          view(viewParam),
          proj(projParam),
          room(roomParam)
    {}
}; 

long D3DRenderWorld(
    const WorldRenderParams& worldRenderParams, 
    const WorldPropertyParams& worldPropertyParams, 
    const LightAndTextureParams& lightAndTextureParams);

void D3DGeometryBuildNew(
    const WorldRenderParams& worldRenderParams, 
    const WorldPropertyParams& worldPropertyParams, 
    const LightAndTextureParams& lightAndTextureParams, 
    bool transparent_pass);

void GeometryUpdate(d3d_render_pool_new* pPool, d3d_render_cache_system* pCacheSystem);

#endif	/* #ifndef _D3DRENDERWORLD_H */

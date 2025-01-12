// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render world object interface is responsible for rendering world objects 
// (such as monsters, players, ornaments and vegetation) in the game world.
// World objects are always drawn when the d3drenderer `draw_objects` is true.
//
#ifndef _D3DRENDEROBJECTS_H
#define _D3DRENDEROBJECTS_H

struct PlayerViewParams
{

    // Screen dimensions
    int screenWidth;
    int screenHeight;

    // Viewport dimensions
    int viewportWidth;
    int viewportHeight;

    // D3D viewport rectangle
    RECT d3dRect;

    PlayerViewParams(
        int screenWidthParam,
        int screenHeightParam,
        int viewportWidthParam,
        int viewportHeightParam,
        RECT d3dRectParam)
        : screenWidth(screenWidthParam),
          screenHeight(screenHeightParam),
          viewportWidth(viewportWidthParam),
          viewportHeight(viewportHeightParam),
          d3dRect(d3dRectParam)
    {}
};

struct FontTextureParams
{
    font_3d* font;
    int smallTextureSize;

    FontTextureParams(
        font_3d* fontParam,
        int smallTextureSizeParam)
        : font(fontParam),
          smallTextureSize(smallTextureSizeParam)
    {}
};

struct LightAndTextureParams
{
    // Light cache details
    d_light_cache* lightCache;
    d_light_cache* lightCacheDynamic;

    // Texture and sector settings
    int smallTextureSize;
    int* sectorDepths;

    LightAndTextureParams(
        d_light_cache* lightCacheParam,
        d_light_cache* lightCacheDynamicParam,
        int smallTextureSizeParam,
        int* sectorDepthsParam)
        : lightCache(lightCacheParam),
          lightCacheDynamic(lightCacheDynamicParam),
          smallTextureSize(smallTextureSizeParam),
          sectorDepths(sectorDepthsParam)
    {}
};

struct GameObjectDataParams
{
    // Information about draw items and object ranges
    long numItems;
    mutable int* numVisibleObjects;
    mutable int* numObjects;
    DrawItem* drawData;
    ObjectRange* visibleObjects;    // Where objects are on screen

    // Textures for the back buffer
    PDIRECT3DTEXTURE9 backBufferTexFull;
    LPDIRECT3DTEXTURE9 (&backBufferTex)[16];

    GameObjectDataParams(
        long numItemsParam,
        int* numVisibleObjectsParam,
        int* numObjectsParam,
        DrawItem* drawDataParam,
        ObjectRange* visibleObjectsParam,
        PDIRECT3DTEXTURE9 backBufferTexFullParam,
        LPDIRECT3DTEXTURE9 (&backBufferTexParam)[16])
        : numItems(numItemsParam),
          numVisibleObjects(numVisibleObjectsParam),
          numObjects(numObjectsParam),
          drawData(drawDataParam),
          visibleObjects(visibleObjectsParam),
          backBufferTexFull(backBufferTexFullParam),
          backBufferTex(backBufferTexParam)
    {}
};

struct ObjectsRenderParams {

	LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationInvisible;
	const d3d_driver_profile& driverProfile;

	d3d_render_pool_new* renderPool;
	d3d_render_cache_system* cacheSystem;

	D3DMATRIX view;
	D3DMATRIX proj;

    room_type* room;
    Draw3DParams* params;

    ObjectsRenderParams(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationInvisibleParam,
        const d3d_driver_profile& driverProfileParam,
        d3d_render_pool_new* renderPoolParam,
        d3d_render_cache_system* cacheSystemParam,
        D3DMATRIX viewParam,
        D3DMATRIX projParam,
        room_type* roomParam, 
        Draw3DParams* paramsParam
    )
        : vertexDeclaration(vertexDeclarationParam),
          vertexDeclarationInvisible(vertexDeclarationInvisibleParam),
          driverProfile(driverProfileParam),
          renderPool(renderPoolParam),
          cacheSystem(cacheSystemParam),
          view(viewParam),
          proj(projParam),
          room(roomParam),
          params(paramsParam)
    {}
};

long D3DRenderObjects(
    const ObjectsRenderParams& objectsRenderParams, 
    const GameObjectDataParams& gameObjectDataParams, 
    const LightAndTextureParams& lightAndTextureParams,
    const FontTextureParams& fontTextureParams,
    const PlayerViewParams& playerViewParams);

#endif	/* #ifndef _D3DRENDEROBJECTS_H */

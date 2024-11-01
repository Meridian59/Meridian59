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

struct BackgroundOverlaysRenderStateParams {
    LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
    const d3d_driver_profile& driverProfile;
    d3d_render_pool_new* worldPool;
    d3d_render_cache_system* worldCacheSystem;
    const D3DMATRIX& view;
    D3DMATRIX& mat;
    RECT& d3dRect;

    BackgroundOverlaysRenderStateParams(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        const d3d_driver_profile& driverProfileParam,
        d3d_render_pool_new* worldPoolParam,
        d3d_render_cache_system* worldCacheSystemParam,
        const D3DMATRIX& viewParam,
        D3DMATRIX& matParam,
        RECT d3dRectParam)
        : vertexDeclaration(vertexDeclarationParam),
          driverProfile(driverProfileParam),
          worldPool(worldPoolParam),
          worldCacheSystem(worldCacheSystemParam),
          view(viewParam),
          mat(matParam),
          d3dRect(d3dRectParam) 
    {}
};

struct BackgroundOverlaysSceneParams {
    mutable int* numVisibleObjects;
    ObjectRange* visibleObjects;
    int angleHeading;
    int anglePitch;
    room_type* room;
    Draw3DParams* params;

    BackgroundOverlaysSceneParams(
        int* numVisibleObjectsParam,
        ObjectRange* visibleObjectsParam,
        int angleHeadingParam,
        int anglePitchParam,
        room_type* roomParam,
        Draw3DParams* paramsParam)
        : numVisibleObjects(numVisibleObjectsParam),
          visibleObjects(visibleObjectsParam),
          angleHeading(angleHeadingParam),
          anglePitch(anglePitchParam),
          room(roomParam),
          params(paramsParam) 
    {}
};

void D3DRenderBackgroundOverlays(BackgroundOverlaysRenderStateParams backgroundOverlaysRenderStateParams, 
    BackgroundOverlaysSceneParams backgroundOverlaysSceneParams);

#endif	/* #ifndef _D3DRENDERBGOVERLAYS_H */

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
    D3DMATRIX& transformMatrix;
    const RECT& d3dRect;

    BackgroundOverlaysRenderStateParams(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        const d3d_driver_profile& driverProfileParam,
        d3d_render_pool_new* worldPoolParam,
        d3d_render_cache_system* worldCacheSystemParam,
        const D3DMATRIX& viewParam,
        D3DMATRIX& transformMatrixParam,
        const RECT& d3dRectParam)
        : vertexDeclaration(vertexDeclarationParam),
          driverProfile(driverProfileParam),
          worldPool(worldPoolParam),
          worldCacheSystem(worldCacheSystemParam),
          view(viewParam),
          transformMatrix(transformMatrixParam),
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

// Defines a projected screen-space region for background overlays.
// Used to determine if a BG overlay is within player view, and to map coordinates for drawing.
struct OverlayRegion {
	// Screen-space coordinates in Normalized Device Coordinates (NDC).
	// X/Y ranges from -1.0 to 1.0 (edges), while Z ranges from 0.0 to 1.0 (depth).
	custom_xyzw topLeft;
	custom_xyzw topRight;
	custom_xyzw bottomLeft;
	custom_xyzw bottomRight;
	custom_xyzw center;
	
	// Full viewport dimensions in pixels.
	// Used to map NDC coordinates back to screen pixels for drawing the BG overlay.
	int width;
	int height;
};

// Camera orientation and transformation matrices for background overlays.
// Used to project a background overlay onto a sky dome above the player.
struct BackgroundOverlayTransform {
	// The background overlay's own position.
	Vector3D pos;
	
	// The viewer's heading/pitch in game units.
	float angleHeading;
	float anglePitch;
	
	// Rotation matrix based on viewer's heading/pitch.
	D3DMATRIX rot;
	
	// Combined transformation matrix for world-space placement.
	D3DMATRIX mat;
};

void D3DRenderBackgroundOverlays(const BackgroundOverlaysRenderStateParams& backgroundOverlaysRenderStateParams, 
    const BackgroundOverlaysSceneParams& backgroundOverlaysSceneParams);

void D3DProcessBackgroundOverlay(const BackgroundOverlaysRenderStateParams& bgoRenderStateParams, 
    const BackgroundOverlaysSceneParams& bgoSceneParams, list_type list);

void D3DBuildBGOverlayMesh(d3d_render_chunk_new* pChunk, float object_width, float object_height);

OverlayRegion D3DSetupOverlayRegion(const auto& d3dRect, d3d_render_chunk_new* pChunk, 
	BackgroundOverlayTransform* transform, const auto& params);

bool D3DIsBGOverlayVisible(OverlayRegion* region);

void D3DFinalizeBGOverlay(BackgroundOverlay* overlay, OverlayRegion* region, BackgroundOverlayTransform* transform, ObjectRange* range,
	const BackgroundOverlaysSceneParams& bgoSceneParams);

#endif	/* #ifndef _D3DRENDERBGOVERLAYS_H */

// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render lights interface is responsible for managing dynamic lighting
// in the 3D game world. This includes light cache management, light map texture
// creation, flickering light effects, and debug visualization of light positions.
//
#ifndef _D3DRENDERLIGHTS_H
#define _D3DRENDERLIGHTS_H

// Parameters for updating light caches from room contents
struct LightCacheUpdateParams
{
   d_light_cache* lightCache;          // Static lights cache
   d_light_cache* lightCacheDynamic;   // Dynamic/flickering lights cache
   int* redrawFlags;                    // Pointer to gD3DRedrawAll flags

   LightCacheUpdateParams(
      d_light_cache* lightCacheParam,
      d_light_cache* lightCacheDynamicParam,
      int* redrawFlagsParam)
      : lightCache(lightCacheParam),
        lightCacheDynamic(lightCacheDynamicParam),
        redrawFlags(redrawFlagsParam)
   {}
};

// Parameters for rendering debug light visualization
struct LightDebugRenderParams
{
   d_light_cache* lightCache;
   d3d_render_pool_new* objectPool;
   d3d_render_cache_system* objectCacheSystem;

   LightDebugRenderParams(
      d_light_cache* lightCacheParam,
      d3d_render_pool_new* objectPoolParam,
      d3d_render_cache_system* objectCacheSystemParam)
      : lightCache(lightCacheParam),
        objectPool(objectPoolParam),
        objectCacheSystem(objectCacheSystemParam)
   {}
};

// Populate the light caches with lights from the room's objects and projectiles.
// Separates lights into static (lightCache) and dynamic/flickering (lightCacheDynamic).
void D3DLMapsStaticGet(room_type* room, const LightCacheUpdateParams& params);

// Check if a light's cached properties match the current object state.
// Used for structural change detection to trigger world rebuilds.
bool D3DLMapCheck(d_light* dLight, room_contents_node* pRNode);

// Get the ambient light level for an object based on its BSP tree position.
int D3DRenderObjectGetLight(BSPnode* tree, room_contents_node* pRNode);

// Debug visualization: renders yellow wireframe ellipsoids at static light positions.
// Enable by setting debugLightPositions = true in d3drender_lights.c.
void D3DRenderDebugLightPositions(Draw3DParams* params, const LightDebugRenderParams& debugParams);

// Check if debug light positions visualization is enabled.
bool D3DLightsDebugPositionsEnabled(void);

// Build light map textures (white glow, orange glow).
// Called during D3D initialization.
void D3DRenderLMapsBuild(void);

// Release light map textures during shutdown.
void D3DRenderLightsShutdown(void);

// Get the white light map texture.
LPDIRECT3DTEXTURE9 D3DRenderLightsGetWhite(void);

// Get the orange light map texture.
LPDIRECT3DTEXTURE9 D3DRenderLightsGetOrange(void);

// Set the global light scale multiplier (0.0 to 5.0).
// This controls the radius of all dynamic light sources.
// Returns true if the value changed and a world rebuild is needed.
bool SetGlobalLightScale(float scale, int* redrawFlags);

// Get the current global light scale multiplier.
float GetGlobalLightScale(void);

// Report flicker performance stats (called from D3DRenderBegin).
void D3DLightsReportFlickerPerf(void);

#endif /* #ifndef _D3DRENDERLIGHTS_H */

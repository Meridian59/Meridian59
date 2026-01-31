// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render lights module is responsible for managing dynamic lighting
// in the 3D game world. This includes light cache management, light map texture
// creation, flickering light effects, and debug visualization of light positions.
//
#include "client.h"
#include "d3drender_lights.h"

// Debug flag to enable/disable light position visualization.
// When true, renders yellow wireframe ellipsoids at static light positions.
// See D3DRenderDebugLightPositions() for details.
static const bool debugLightPositions = false;

// Performance profiling for flickering lights feature.
// When true, outputs timing and count stats every 256 frames.
// Use this to compare performance with flickering on vs off.
static bool flickerPerfProfile = false;

// Accumulated performance counters (reset every 256 frames)
static long flickerPerfTimeAccum = 0;      // Time spent in light processing (ms)
static int flickerPerfLightsAccum = 0;     // Total lights processed
static int flickerPerfFlickerAccum = 0;    // Flickering lights processed
static int flickerPerfFrameCount = 0;      // Frames since last report

// GPU metrics for flicker profiling
static int flickerRedrawsTriggered = 0;    // Full redraws triggered by light changes

// The maximum number of lights supported across all types (static, dynamic and projectiles)
static const int maximumLights = 50;

// Global light scale multiplier for all dynamic lights.
// Applied in D3DLightScale() to control the radius of all light sources.
// Lower values = smaller, more concentrated lights. Higher values = larger, more diffuse lights.
static float gLightScale = 0.45f;

// Minimum world light radius in FINENESS units (1024 = one grid square).
// With the bounding box check in D3DRenderLMapPostFloorAdd/Ceiling/Wall, we no longer
// need a large minimum radius to catch lights centered in polygons. The bounding box
// check handles that case. This minimum is just a safety floor for very small lights.
// Set to 0 to allow full control via gLightScale and object intensity.
static const float MIN_WORLD_LIGHT_RADIUS = 0.0f;

bool SetGlobalLightScale(float scale, int* redrawFlags)
{
   float newScale = std::clamp(scale, 0.0f, 5.0f);
   if (newScale == gLightScale)
      return false;

   gLightScale = newScale;

   // Force a full world rebuild so cached lightmaps / geometry that depend on radii update.
   if (redrawFlags)
      *redrawFlags |= D3DRENDER_REDRAW_ALL;

   return true;
}

float GetGlobalLightScale(void) { return gLightScale; }

/*
 * Scale light radius based on intensity (0-255) and global scale factor.
 * Applies the original D3D light radius curve, then scales by gLightScale.
 */
static float D3DLightScale(int intensity)
{
   // Keep the original curve but apply the global scale variable
   const float LIGHT_MULTIPLIER = 12000.0f;
   const float LIGHT_BASE_SIZE = 2000.0f;

   float baseRadius = (intensity * LIGHT_MULTIPLIER / 255.0f) + LIGHT_BASE_SIZE;
   float r = baseRadius * gLightScale;

   // enforce a minimum world radius so small gLightScale values don't collapse lights
   return std::max(r, MIN_WORLD_LIGHT_RADIUS);
}

bool D3DLMapCheck(d_light* dLight, room_contents_node* pRNode)
{
   if (dLight->objID != pRNode->obj.id)
      return false;
   if (dLight->baseIntensity != DLIGHT_SCALE(pRNode->obj.dLighting.intensity))
      return false;
   if (dLight->baseColor != pRNode->obj.dLighting.color)
      return false;

   return true;
}

/*
 * Helper structure to pass light data for initialization.
 */
struct LightSourceData
{
   int baseIntensity;
   int objFlags;
   int lightAdjust;
   WORD lightColor;  // 16-bit RGB color (5-5-5 format)
   ID objID;         // Object ID for debug output
   WORD lightFlags;  // Light flags for debug output
};

/*
 * Calculate flickered intensity and brightness for a light.
 * Returns the scaled intensity with flicker applied (if applicable).
 * Also outputs the flickerBrightness value for color calculations.
 */
static int CalculateFlickeredIntensity(const LightSourceData& lightData, float* outFlickerBrightness)
{
   float flickerBrightness = 1.0f;
   int flickeredIntensity;

   if (lightData.objFlags & (OF_FLICKERING | OF_FLASHING))
   {
      flickerBrightness = (float)lightData.lightAdjust / GetFlickerLevel();
      flickeredIntensity = (int)(D3DLightScale(lightData.baseIntensity) * flickerBrightness);
   }
   else
   {
      flickeredIntensity = D3DLightScale(lightData.baseIntensity);
   }

   if (outFlickerBrightness)
      *outFlickerBrightness = flickerBrightness;

   return flickeredIntensity;
}

/*
 * Initialize all light scale and color properties for a d_light structure.
 * This handles xyzScale, invXYZScale, invXYZScaleHalf, and color with flicker applied.
 * Optionally outputs debug information if debugLights is true.
 */
static void InitializeLightProperties(d_light* light,
                                       const LightSourceData& lightData,
                                       bool debugLights, const char* lightType,
                                       int& lightCount)
{
   float flickerBrightness;
   int flickeredIntensity = CalculateFlickeredIntensity(lightData, &flickerBrightness);

   lightCount++;

   // Store base values for cache validation (unflickered)
   light->baseIntensity = DLIGHT_SCALE(lightData.baseIntensity);
   light->baseColor = lightData.lightColor;

   // Debug output
   if (debugLights)
   {
      debug(("%s Light %d: objID=%ld, objFlags=0x%08X, lightFlags=0x%04X, color=0x%04X, intensity=%d, "
             "lightAdjust=%d, flickerBright=%.3f, flickeredInt=%d%s%s\n",
             lightType, lightCount, lightData.objID, lightData.objFlags, lightData.lightFlags, lightData.lightColor,
             lightData.baseIntensity, lightData.lightAdjust, flickerBrightness, flickeredIntensity,
             (lightData.objFlags & OF_FLICKERING) ? " [FLICKERING]" : "",
             (lightData.objFlags & OF_FLASHING) ? " [FLASHING]" : ""));
   }

   // Set xyz scales (all three axes use same value)
   light->xyzScale.x = flickeredIntensity;
   light->xyzScale.y = flickeredIntensity;
   light->xyzScale.z = flickeredIntensity;

   // Calculate inverse scales
   light->invXYZScale.x = 1.0f / light->xyzScale.x;
   light->invXYZScale.y = 1.0f / light->xyzScale.y;
   light->invXYZScale.z = 1.0f / light->xyzScale.z;

   // Calculate inverse half scales
   light->invXYZScaleHalf.x = 1.0f / (light->xyzScale.x / 2.0f);
   light->invXYZScaleHalf.y = 1.0f / (light->xyzScale.y / 2.0f);
   light->invXYZScaleHalf.z = 1.0f / (light->xyzScale.z / 2.0f);

   // Set color with flicker applied (convert from 16-bit 5-5-5 RGB to 8-bit RGBA)
   light->color.a = COLOR_MAX;
   light->color.r = (BYTE)(((lightData.lightColor >> 10) & 31) * COLOR_MAX / 31 * flickerBrightness);
   light->color.g = (BYTE)(((lightData.lightColor >> 5) & 31) * COLOR_MAX / 31 * flickerBrightness);
   light->color.b = (BYTE)((lightData.lightColor & 31) * COLOR_MAX / 31 * flickerBrightness);
}

/*
 * Calculate the Z position for a light based on object position, floor height, and sprite dimensions.
 * This ensures lights are positioned above the floor for proper illumination.
 */
static float CalculateLightZPosition(room_type* room, long motionX, long motionY, long motionZ, PDIB pDib)
{
   long top, bottom;
   int sector_flags;

   // Get floor height at object position
   GetRoomHeight(room->tree, &top, &bottom, &sector_flags, motionX, motionY);

   // Start at floor level
   float floorZ = (float)std::max(bottom, motionZ);

   // Calculate sprite height
   float spriteHeight = 0.0f;
   if (pDib)
      spriteHeight = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

   // Ensure the light is positioned above the floor for proper illumination
   const float MIN_LIGHT_HEIGHT_ABOVE_FLOOR = 64.0f;
   float heightAboveFloor = (spriteHeight > MIN_LIGHT_HEIGHT_ABOVE_FLOOR) ? spriteHeight : MIN_LIGHT_HEIGHT_ABOVE_FLOOR;

   return floorZ + heightAboveFloor;
}

/*
 * Check if a light should be processed based on cache limits and light properties.
 * Returns true if the light should be skipped.
 */
static bool ShouldSkipLight(int currentLightCount, WORD lightFlags, WORD lightColor, int lightIntensity)
{
   // Check cache limit
   if (currentLightCount >= maximumLights)
      return true;

   // Check if light has valid color and intensity
   if (lightColor == 0 || lightIntensity == 0)
      return true;

   return false;
}

void D3DLMapsStaticGet(room_type* room, const LightCacheUpdateParams& params)
{
   room_contents_node* pRNode;
   list_type list;
   PDIB pDib;

   bool projectileLightsEnable = true;
   bool dynamicLightsEnabled = true;
   bool staticLightsEnabled = true;

   // Debug flags to control light map processing debug output
   bool debugLights = false;

   // Performance profiling
   long flickerPerfStart = 0;
   int flickerCount = 0;
   int totalLightCount = 0;
   if (flickerPerfProfile)
      flickerPerfStart = timeGetTime();

   if (debugLights)
      debug(("=== PROCESSING LIGHTS IN ROOM ===\n"));

   d_light_cache* lightCache = params.lightCache;
   d_light_cache* lightCacheDynamic = params.lightCacheDynamic;

   if (projectileLightsEnable)
   {
      int projectileCount = 0;

      if (debugLights)
         debug(("=== PROJECTILE LIGHTS ===\n"));

      // projectiles
      for (list = room->projectiles; list != NULL; list = list->next)
      {
         Projectile* pProjectile = (Projectile*)list->data;

         if (ShouldSkipLight(lightCacheDynamic->numLights, 0, pProjectile->dLighting.color,
                             pProjectile->dLighting.intensity))
            continue;

         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.x = pProjectile->motion.x;
         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.y = pProjectile->motion.y;
         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.z = pProjectile->motion.z;

         pDib = GetObjectPdib(pProjectile->icon_res, 0, 0);
         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.z =
            CalculateLightZPosition(room, pProjectile->motion.x, pProjectile->motion.y, pProjectile->motion.z, pDib);

         if (debugLights)
            debug(("Projectile Light %d: color=0x%04X, intensity=%d\n", projectileCount, pProjectile->dLighting.color,
                   pProjectile->dLighting.intensity));

         LightSourceData lightData = {.baseIntensity = pProjectile->dLighting.intensity,
                                       .objFlags = pProjectile->flags,
                                       .lightAdjust = 0,
                                       .lightColor = pProjectile->dLighting.color,
                                       .objID = 0,
                                       .lightFlags = 0};
         InitializeLightProperties(&lightCacheDynamic->dLights[lightCacheDynamic->numLights], lightData, false,
                                   "Projectile", projectileCount);

         lightCacheDynamic->numLights++;

         // Performance profiling: count projectile lights
         if (flickerPerfProfile)
            totalLightCount++;
      }

      if (debugLights)
         debug(("Total Projectile Lights: %d\n\n", projectileCount));
   }

   if (dynamicLightsEnabled)
   {
      int dynamicCount = 0;

      if (debugLights)
         debug(("=== DYNAMIC LIGHTS ===\n"));

      // dynamic lights
      for (list = room->contents; list != NULL; list = list->next)
      {
         pRNode = (room_contents_node*)list->data;

         bool isDynamic = (pRNode->obj.dLighting.flags & LIGHT_FLAG_DYNAMIC) != 0;
         if (!isDynamic)
            continue;

         if (ShouldSkipLight(lightCacheDynamic->numLights, pRNode->obj.dLighting.flags, pRNode->obj.dLighting.color,
                             pRNode->obj.dLighting.intensity))
            continue;

         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.x = pRNode->motion.x;
         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.y = pRNode->motion.y;

         pDib = GetObjectPdib(pRNode->obj.icon_res, 0, 0);
         lightCacheDynamic->dLights[lightCacheDynamic->numLights].xyz.z =
            CalculateLightZPosition(room, pRNode->motion.x, pRNode->motion.y, pRNode->motion.z, pDib);

         LightSourceData lightData = {.baseIntensity = pRNode->obj.dLighting.intensity * 10,
                                       .objFlags = pRNode->obj.flags,
                                       .lightAdjust = pRNode->obj.lightAdjust,
                                       .lightColor = pRNode->obj.dLighting.color,
                                       .objID = pRNode->obj.id,
                                       .lightFlags = pRNode->obj.dLighting.flags};

         InitializeLightProperties(&lightCacheDynamic->dLights[lightCacheDynamic->numLights], lightData, debugLights,
                                   "Dynamic", dynamicCount);

         lightCacheDynamic->numLights++;

         // Performance profiling: count flickering lights
         if (flickerPerfProfile)
         {
            totalLightCount++;
            if (pRNode->obj.flags & (OF_FLICKERING | OF_FLASHING))
               flickerCount++;
         }
      }

      if (debugLights)
         debug(("Total Dynamic Lights: %d\n\n", dynamicCount));
   }

   if (staticLightsEnabled)
   {
      int staticCount = 0;

      if (debugLights)
         debug(("=== STATIC LIGHTS ===\n"));

      // static lights
      for (list = room->contents; list != NULL; list = list->next)
      {
         pRNode = (room_contents_node*)list->data;

         bool isDynamic = (pRNode->obj.dLighting.flags & LIGHT_FLAG_DYNAMIC) != 0;
         if (isDynamic)
            continue;

         if (ShouldSkipLight(lightCacheDynamic->numLights, pRNode->obj.dLighting.flags, pRNode->obj.dLighting.color,
                             pRNode->obj.dLighting.intensity))
            continue;

         bool isFlickering = (pRNode->obj.flags & (OF_FLICKERING | OF_FLASHING)) != 0;

         // Select target cache: flickering lights go to dynamic cache, static lights to main cache
         d_light_cache* targetCache = isFlickering ? lightCacheDynamic : lightCache;
         const char* debugLabel = isFlickering ? "Flickering" : "Static";

         // Non-flickering lights need structural change detection
         if (!isFlickering)
         {
            if (!D3DLMapCheck(&targetCache->dLights[targetCache->numLights], pRNode))
            {
               // Structural change (light added/removed/moved, base intensity/color changed)
               if (params.redrawFlags)
                  *params.redrawFlags |= D3DRENDER_REDRAW_ALL;
               if (flickerPerfProfile)
                  flickerRedrawsTriggered++;
            }
         }

         // Common light setup
         pDib = GetObjectPdib(pRNode->obj.icon_res, 0, 0);

         d_light* light = &targetCache->dLights[targetCache->numLights];

         // Static lights need objID for cache validation
         if (!isFlickering)
            light->objID = pRNode->obj.id;

         light->xyz.x = pRNode->motion.x;
         light->xyz.y = pRNode->motion.y;
         light->xyz.z = CalculateLightZPosition(room, pRNode->motion.x, pRNode->motion.y, pRNode->motion.z, pDib);

         LightSourceData lightData = {.baseIntensity = pRNode->obj.dLighting.intensity,
                                       .objFlags = pRNode->obj.flags,
                                       .lightAdjust = pRNode->obj.lightAdjust,
                                       .lightColor = pRNode->obj.dLighting.color,
                                       .objID = pRNode->obj.id,
                                       .lightFlags = pRNode->obj.dLighting.flags};
         InitializeLightProperties(light, lightData, debugLights, debugLabel, staticCount);

         targetCache->numLights++;

         // Performance profiling: count flickering lights
         if (flickerPerfProfile)
         {
            totalLightCount++;
            if (isFlickering)
               flickerCount++;
         }
      }

      if (debugLights)
         debug(("Total Static Lights: %d\n\n", staticCount));
   }

   // Accumulate performance stats
   if (flickerPerfProfile)
   {
      flickerPerfTimeAccum += (timeGetTime() - flickerPerfStart);
      flickerPerfLightsAccum += totalLightCount;
      flickerPerfFlickerAccum += flickerCount;
   }
}

int D3DRenderObjectGetLight(BSPnode* tree, room_contents_node* pRNode)
{
   long side0;
   BSPnode *pos, *neg;

   while (1)
   {
      if (tree == NULL)
      {
         return false;
      }

      switch (tree->type)
      {
      case BSPleaftype:
         return tree->u.leaf.sector->light;

      case BSPinternaltype:
         side0 = tree->u.internal.separator.a * pRNode->motion.x + tree->u.internal.separator.b * pRNode->motion.y +
                 tree->u.internal.separator.c;

         pos = tree->u.internal.pos_side;
         neg = tree->u.internal.neg_side;

         if (side0 == 0)
            tree = (pos != NULL) ? pos : neg;
         else if (side0 > 0)
            tree = pos;
         else if (side0 < 0)
            tree = neg;
         break;

      default:
         debug(("add_object error!\n"));
         return false;
      }
   }
}

/**
 * Debug visualization function that renders yellow wireframe ellipsoids at each static light position.
 * The ellipsoids are oriented according to the sun direction and sized to show the light's radius.
 */
void D3DRenderDebugLightPositions(Draw3DParams* params, const LightDebugRenderParams& debugParams)
{
   // Debug: Draw WIREFRAME ELLIPSOIDS oriented by sun direction at static light positions
   const int LAT_SEGMENTS = 8;
   const int LON_SEGMENTS = 12;

   d_light_cache* lightCache = debugParams.lightCache;
   d3d_render_pool_new* objectPool = debugParams.objectPool;
   d3d_render_cache_system* objectCacheSystem = debugParams.objectCacheSystem;

   // Get sun direction for orientation
   const Vector3D& sunVect = getSunVector();
   float sunAngle = atan2f(sunVect.y, sunVect.x);  // Angle in XY plane

   D3DRenderPoolReset(objectPool, &D3DMaterialObjectPool);
   D3DCacheSystemReset(objectCacheSystem);

   for (int i = 0; i < lightCache->numLights; i++)
   {
      d_light* light = &lightCache->dLights[i];

      // The lighting system uses invXYZScaleHalf (1.0 / (xyzScale / 2.0))
      float radiusX = light->xyzScale.x / 2.0f;
      float radiusY = light->xyzScale.y / 2.0f;
      float radiusZ = light->xyzScale.z / 2.0f;

      // Precompute rotation for sun direction
      float cosSun = cosf(sunAngle);
      float sinSun = sinf(sunAngle);

      // Draw latitude rings (horizontal circles)
      for (int lat = 0; lat <= LAT_SEGMENTS; lat++)
      {
         d3d_render_packet_new* pPacket = D3DRenderPacketFindMatch(objectPool, NULL, NULL, 0, 0, 0);
         if (pPacket == NULL)
            continue;

         d3d_render_chunk_new* pChunk = D3DRenderChunkNew(pPacket);
         if (pChunk == NULL)
            continue;

         pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
         pChunk->pMaterialFctn = &D3DMaterialNone;

         pChunk->numVertices = LON_SEGMENTS + 1;
         pChunk->numIndices = LON_SEGMENTS * 2;
         pChunk->numPrimitives = LON_SEGMENTS;

         MatrixIdentity(&pChunk->xForm);

         float theta = (float)lat / (float)LAT_SEGMENTS * PI;  // 0 to PI (north to south)
         float sinTheta = sinf(theta);
         float cosTheta = cosf(theta);

         // Generate ring of vertices at this latitude
         for (int lon = 0; lon <= LON_SEGMENTS; lon++)
         {
            float phi = (float)lon / (float)LON_SEGMENTS * 2.0f * PI;  // 0 to 2*PI (around)
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            // Ellipsoid formula with anisotropic scaling (using HALF radius)
            float localX = radiusX * sinTheta * cosPhi;
            float localY = radiusY * sinTheta * sinPhi;
            float localZ = radiusZ * cosTheta;

            // Rotate by sun angle (around Z-axis to align with sun direction in XY plane)
            float rotatedX = localX * cosSun - localY * sinSun;
            float rotatedY = localX * sinSun + localY * cosSun;
            float rotatedZ = localZ;

            // Translate to light position
            pChunk->xyz[lon].x = light->xyz.x + rotatedX;
            pChunk->xyz[lon].y = light->xyz.y + rotatedY;
            pChunk->xyz[lon].z = light->xyz.z + rotatedZ;

            // Yellow wireframe
            pChunk->bgra[lon].r = 255;
            pChunk->bgra[lon].g = 255;
            pChunk->bgra[lon].b = 0;
            pChunk->bgra[lon].a = 255;
         }

         // Build line indices (connect consecutive points in the ring)
         for (int j = 0; j < LON_SEGMENTS; j++)
         {
            pChunk->indices[j * 2 + 0] = j;
            pChunk->indices[j * 2 + 1] = j + 1;
         }
      }

      // Draw longitude lines (vertical meridians)
      for (int lon = 0; lon < LON_SEGMENTS; lon++)
      {
         d3d_render_packet_new* pPacket = D3DRenderPacketFindMatch(objectPool, NULL, NULL, 0, 0, 0);
         if (pPacket == NULL)
            continue;

         d3d_render_chunk_new* pChunk = D3DRenderChunkNew(pPacket);
         if (pChunk == NULL)
            continue;

         pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
         pChunk->pMaterialFctn = &D3DMaterialNone;

         pChunk->numVertices = LAT_SEGMENTS + 1;
         pChunk->numIndices = LAT_SEGMENTS * 2;
         pChunk->numPrimitives = LAT_SEGMENTS;

         MatrixIdentity(&pChunk->xForm);

         float phi = (float)lon / (float)LON_SEGMENTS * 2.0f * PI;
         float sinPhi = sinf(phi);
         float cosPhi = cosf(phi);

         // Generate line of vertices from north to south pole at this longitude
         for (int lat = 0; lat <= LAT_SEGMENTS; lat++)
         {
            float theta = (float)lat / (float)LAT_SEGMENTS * PI;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            // Ellipsoid formula with anisotropic scaling (using HALF radius)
            float localX = radiusX * sinTheta * cosPhi;
            float localY = radiusY * sinTheta * sinPhi;
            float localZ = radiusZ * cosTheta;

            // Rotate by sun angle
            float rotatedX = localX * cosSun - localY * sinSun;
            float rotatedY = localX * sinSun + localY * cosSun;
            float rotatedZ = localZ;

            // Translate to light position
            pChunk->xyz[lat].x = light->xyz.x + rotatedX;
            pChunk->xyz[lat].y = light->xyz.y + rotatedY;
            pChunk->xyz[lat].z = light->xyz.z + rotatedZ;

            // Yellow wireframe
            pChunk->bgra[lat].r = 255;
            pChunk->bgra[lat].g = 255;
            pChunk->bgra[lat].b = 0;
            pChunk->bgra[lat].a = 255;
         }

         // Build line indices (connect consecutive points along meridian)
         for (int j = 0; j < LAT_SEGMENTS; j++)
         {
            pChunk->indices[j * 2 + 0] = j;
            pChunk->indices[j * 2 + 1] = j + 1;
         }
      }
   }

   // Flush everything using the standard cache system with LINE LIST primitive
   D3DCacheFill(objectCacheSystem, objectPool, 1);
   D3DCacheFlush(objectCacheSystem, objectPool, 1, D3DPT_LINELIST);
}

bool D3DLightsDebugPositionsEnabled(void)
{
   return debugLightPositions;
}

void D3DRenderLMapsBuild(LPDIRECT3DTEXTURE9* outWhiteTex, LPDIRECT3DTEXTURE9* outOrangeTex)
{
   D3DLOCKED_RECT lockedRect;
   unsigned char* pBits = NULL;
   int width, height;

   // white glow
   IDirect3DDevice9_CreateTexture(gpD3DDevice, 32, 32, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, outWhiteTex, NULL);

   IDirect3DTexture9_LockRect(*outWhiteTex, 0, &lockedRect, NULL, 0);

   pBits = (unsigned char*)lockedRect.pBits;

   for (height = 0; height < 32; height++)
   {
      for (width = 0; width < 32; width++)
      {
         float scale = sqrtf((height - 16) * (height - 16) + (width - 16) * (width - 16));
         scale = 16.0f - scale;
         scale = std::max(scale, 0.0f);
         scale /= 16.0f;

         *(pBits++) = 255 * scale;
         *(pBits++) = 255 * scale;
         *(pBits++) = 255 * scale;
         *(pBits++) = 255;
      }
   }

   IDirect3DTexture9_UnlockRect(*outWhiteTex, 0);

   // orange glow
   IDirect3DDevice9_CreateTexture(gpD3DDevice, 32, 32, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, outOrangeTex, NULL);

   IDirect3DTexture9_LockRect(*outOrangeTex, 0, &lockedRect, NULL, 0);

   pBits = (unsigned char*)lockedRect.pBits;

   for (height = 0; height < 32; height++)
   {
      for (width = 0; width < 32; width++)
      {
         float scale = sqrtf((height - 16) * (height - 16) + (width - 16) * (width - 16));

         scale = 16.0f - scale;
         scale = std::max(scale, 0.0f);
         scale /= 16.0f;

         if ((height == 0) || (height == 31) || (width == 0) || (width == 31))
            scale = 0;

         *(pBits++) = LIGHTMAP_B * scale;
         *(pBits++) = LIGHTMAP_G * scale;
         *(pBits++) = LIGHTMAP_R * scale;
         *(pBits++) = COLOR_MAX * scale;
      }
   }

   IDirect3DTexture9_UnlockRect(*outOrangeTex, 0);
}

void D3DRenderLightsShutdown(LPDIRECT3DTEXTURE9 whiteTex, LPDIRECT3DTEXTURE9 orangeTex)
{
   if (whiteTex)
   {
      IDirect3DTexture9_Release(whiteTex);
   }
   if (orangeTex)
   {
      IDirect3DTexture9_Release(orangeTex);
   }
}

void D3DLightsReportFlickerPerf(void)
{
   if (flickerPerfProfile)
   {
      flickerPerfFrameCount++;
      if (flickerPerfFrameCount >= 256)
      {
         float avgTimePerFrame = (float)flickerPerfTimeAccum / flickerPerfFrameCount;
         float avgLightsPerFrame = (float)flickerPerfLightsAccum / flickerPerfFrameCount;
         float avgFlickerPerFrame = (float)flickerPerfFlickerAccum / flickerPerfFrameCount;

         debug(("=== FLICKER PERF (256 frames) === time=%ldms (avg %.2fms/frame) | lights=%.1f/frame | "
                "flickering=%.1f/frame | redraws=%d ===\n",
                flickerPerfTimeAccum, avgTimePerFrame, avgLightsPerFrame, avgFlickerPerFrame, flickerRedrawsTriggered));

         // Report memory usage for flicker feature
         int lightCacheMemory = 2 * (int)sizeof(d_light_cache);
         debug(("=== FLICKER MEMORY === lightCaches=%d bytes | flickerObjs=%.1f/frame (x8 bytes) ===\n",
                lightCacheMemory, avgFlickerPerFrame));

         // Reset accumulators
         flickerPerfTimeAccum = 0;
         flickerPerfLightsAccum = 0;
         flickerPerfFlickerAccum = 0;
         flickerPerfFrameCount = 0;
         flickerRedrawsTriggered = 0;
      }
   }
}

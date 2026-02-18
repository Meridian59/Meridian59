// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Using static constexpr to set up constants at compile time to reduce CPU overhead.
// Meridian 59's angles are in game units. (4096 units = 360-degree circle)
static constexpr float FULL_CIRCLE_UNITS = 4096.0f;
static constexpr float GAME_UNITS_TO_RADIANS = (2.0f * PI) / FULL_CIRCLE_UNITS;
static constexpr float FULL_CIRCLE_TO_DEGREES = 360.0f / FULL_CIRCLE_UNITS;
static constexpr float DEGREES_TO_RADIANS = PI / 180.0f;
static constexpr float ANGLE_RANGE_TO_DEGREES = 45.0f / 414.0f;

// Specify the maximum and minimum altitude of the background overlay.
// This will map from the -200 to 200 values return from the server to these values.
static constexpr int ALTITUDE_MAX = 200;
static constexpr int ALTITUDE_MIN = -200;
// Increase the size of the background overlay if necessary.
static constexpr float SIZE_SCALER = 1.2f;

// Rendering constants for a 2D quad.
static constexpr int NUM_VERTICES = 4;
static constexpr int NUM_INDICES = NUM_VERTICES;
static constexpr int NUM_PRIMITIVES = NUM_VERTICES - 2;

// Small value to adjust UV coordinates inward. Avoids texture bleeding.
static constexpr float EPSILON = 0.007f;  
// Pre-calculated UV arrays for a quad.
static constexpr float u[NUM_VERTICES] = { EPSILON, 1.0f - EPSILON, 1.0f - EPSILON, EPSILON };
static constexpr float v[NUM_VERTICES] = { EPSILON, EPSILON, 1.0f - EPSILON, 1.0f - EPSILON };


/**
* Render background overlays in the current room -- for example the Sun and Moon.
*/
void D3DRenderBackgroundOverlays(const BackgroundOverlaysRenderStateParams& bgoRenderStateParams, 
    const BackgroundOverlaysSceneParams& bgoSceneParams)
{
	// Reset world matrix and setup fixed-function pipeline for simple quad rendering.	
	MatrixIdentity(&bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, bgoRenderStateParams.vertexDeclaration);

	// Disable z-writing so BG overlays won't block world objects 
	// and enable z-testing so that world objects can hide them.
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);

	// Enable alpha blending and alpha testing for subsequent rendering.
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);

	D3DRenderPoolReset(bgoRenderStateParams.worldPool, &D3DMaterialWorldPool);
	D3DCacheSystemReset(bgoRenderStateParams.worldCacheSystem);

	// Process all background overlays into the render pool.
	for (list_type list = bgoSceneParams.room->bg_overlays; list != NULL; list = list->next)
	{
		D3DProcessBackgroundOverlay(bgoRenderStateParams, bgoSceneParams, list);
	}
	
	// Batch and dispatch the draw calls to the GPU using Triangle Strips.
	D3DCacheFill(bgoRenderStateParams.worldCacheSystem, bgoRenderStateParams.worldPool, 1);
	D3DCacheFlush(bgoRenderStateParams.worldCacheSystem, bgoRenderStateParams.worldPool, 1, D3DPT_TRIANGLESTRIP);

	// Restore standard render states to avoid breaking subsequent world-object rendering.
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);

	// Disable alpha blending and alpha testing.
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);

	// Restore the correct material and view matrices.
	MatrixIdentity(&bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &bgoRenderStateParams.view);
}

/**
* Determines if this background overlay should be rendered.
*/
void D3DProcessBackgroundOverlay(const BackgroundOverlaysRenderStateParams& bgoRenderStateParams, 
    const BackgroundOverlaysSceneParams& bgoSceneParams, list_type list)
{
	BackgroundOverlay* overlay = (BackgroundOverlay*)(list->data);

	PDIB pDib = GetObjectPdib(overlay->obj.icon_res, 0, overlay->obj.animate->group);
	if (NULL == pDib)
		return;

	BYTE* bkgnd_bmap = DibPtr(pDib);
	if (bkgnd_bmap == NULL)
		return;

	// The background overlay is not yet considered visible to the player.
	overlay->drawn = FALSE;

	// Check if the sky is cloudy. If so, don't render the sun/moon and don't let us right-click it.
	if (IsClearWeather() == false)
		return;

	long object_width = DibWidth(pDib) * SIZE_SCALER;
	long object_height = DibHeight(pDib) * SIZE_SCALER;

	// Attempt to batch this overlay into an existing render packet for this texture.
	d3d_render_packet_new* pPacket = D3DRenderPacketFindMatch(bgoRenderStateParams.worldPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;

	// Initialize a new chunk of geometry data for this background overlay.
	d3d_render_chunk_new* pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	// Define the quad's topology and rendering flags.
	pChunk->numIndices = NUM_INDICES;
	pChunk->numVertices = NUM_VERTICES;
	pChunk->numPrimitives = NUM_PRIMITIVES;
	pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
	pChunk->flags |= D3DRENDER_NOAMBIENT | D3DRENDER_WORLD_OBJ;
	pChunk->zBias = ZBIAS_BASE;

	// Map raw server height (-200 to 200) to engine altitude range.
	auto mapRange = [](double value, double in_min, double in_max, double out_min, double out_max) -> double {
		return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	};

	long mappedHeightValue = mapRange(overlay->y, -200, 200, ALTITUDE_MIN, ALTITUDE_MAX);
	double azimuthalAngle = overlay->x * GAME_UNITS_TO_RADIANS;
	
	// Calculate BG overlay's position on a sky dome surrounding the viewer.
	// A 5x multiplier ensures enough depth for the overlay to stay behind world geometry.
	long radius = ALTITUDE_MAX * 5;
	double horizontalRadius = sqrt(pow(radius, 2) - pow(mappedHeightValue, 2));		

	const auto& params = bgoSceneParams.params;

	// Center the quad and project it onto the sky dome relative to the viewer.
	overlay_transform transform;
	transform.pos.x = (float)(params->viewer_x - (object_width / 2) + horizontalRadius * cos(azimuthalAngle));
	transform.pos.y = (float)(params->viewer_y - (object_height / 2) + horizontalRadius * sin(azimuthalAngle));
	transform.pos.z = (float)(params->viewer_height + mappedHeightValue);
	
	MatrixIdentity(&transform.mat);
	MatrixIdentity(&transform.rot);
	// Apply camera rotation and tilt.
	transform.angleHeading = (float)bgoSceneParams.angleHeading;
	transform.anglePitch = (float)bgoSceneParams.anglePitch;
	
	// Build rotation matrices.
	MatrixRotateY(&transform.rot, transform.angleHeading * FULL_CIRCLE_TO_DEGREES * DEGREES_TO_RADIANS);
	MatrixRotateX(&transform.mat, transform.anglePitch * ANGLE_RANGE_TO_DEGREES * DEGREES_TO_RADIANS);
	MatrixTranspose(&transform.rot, &transform.rot);
	
	// Position overlay in the world relative to viewer's current location.
	MatrixTranslate(&transform.mat, transform.pos.x, transform.pos.z, transform.pos.y);
	MatrixMultiply(&pChunk->xForm, &transform.rot, &transform.mat);

	D3DBuildBGOverlayMesh(pChunk, &object_width, &object_height);

	// Determine if the background overlay is visible for click detection in client
	// (e.g. Blinded by the light of the Sun).
	ObjectRange* range = FindVisibleObjectById(overlay->obj.id);
	const auto& d3dRect = bgoRenderStateParams.d3dRect;
	overlay_region region = D3DSetupOverlayRegion(d3dRect,pChunk,&transform,params);
	if (D3DIsBGOverlayVisible(&region))
	{
		D3DFinalizeBGOverlay(overlay, &region, &transform, range, bgoSceneParams);
	}	
}

/**
* Defines the background overlay's shape, size, and texture mapping.
*/
void D3DBuildBGOverlayMesh(d3d_render_chunk_new* pChunk, long* object_width, long* object_height)
{
	// Defines four corners of the quad in local object space.
	// Bottom Right (origin)
	pChunk->xyz[0].x = 0;
	pChunk->xyz[0].z = 0;
	pChunk->xyz[0].y = 0;

	// Bottom Left
	pChunk->xyz[1].x = -(*object_width);
	pChunk->xyz[1].z = 0;
	pChunk->xyz[1].y = 0;

	// Top Left
	pChunk->xyz[2].x = -(*object_width);
	pChunk->xyz[2].z = *object_height;
	pChunk->xyz[2].y = 0;

	// Top Right
	pChunk->xyz[3].x = 0;
	pChunk->xyz[3].z = *object_height;
	pChunk->xyz[3].y = 0;

	// Sets each of the four vertices to solid white to display texture in original colors.
	for (int j = 0; j < NUM_VERTICES; j++)
	{
		pChunk->bgra[j].b = 255;
		pChunk->bgra[j].g = 255;
		pChunk->bgra[j].r = 255;
		pChunk->bgra[j].a = 255;
	}

	// Map texture coordinates to vertices.
	// Normally, you'd assign these in a predictable (e.g., top-left to bottom-right) order. 
	// Here, we intentionally swap certain indices to mirror the texture along the X-axis.
	pChunk->st0[0].s = u[1];
	pChunk->st0[0].t = v[0];
	pChunk->st0[1].s = u[0];
	pChunk->st0[1].t = v[1];
	pChunk->st0[2].s = u[3];
	pChunk->st0[2].t = v[2];
	pChunk->st0[3].s = u[2];
	pChunk->st0[3].t = v[3];

	// Define the index buffer for a Triangle Strip.
    // This order (1, 2, 0, 3) creates two triangles that share an edge to form the quad.
	pChunk->indices[0] = 1;
	pChunk->indices[1] = 2;
	pChunk->indices[2] = 0;
	pChunk->indices[3] = 3;
}

/**
* Maps 3D overlay coordiantes to 2D screen space.
*/
overlay_region D3DSetupOverlayRegion(const auto& d3dRect, d3d_render_chunk_new* pChunk, overlay_transform* transform, const auto& params)
{
	overlay_region region;
	
	// Initialize viewport dimensions and quad corner points.
	region.width = d3dRect.right - d3dRect.left;
	region.height = d3dRect.bottom - d3dRect.top;

	region.topLeft = {pChunk->xyz[3].x, pChunk->xyz[3].z, 0, 1.0f};
	region.topRight = {pChunk->xyz[3].x, pChunk->xyz[3].z, 0, 1.0f};
	region.bottomLeft = {pChunk->xyz[1].x, pChunk->xyz[1].z, 0, 1.0f};
	region.bottomRight = {pChunk->xyz[1].x, pChunk->xyz[1].z, 0, 1.0f};

	// Construct combined World-View-Projection matrix for screen-space mappping.
	D3DMATRIX localToScreen, trans;
	MatrixRotateY(&transform->rot, transform->angleHeading * FULL_CIRCLE_TO_DEGREES * DEGREES_TO_RADIANS);
	MatrixRotateX(&transform->mat, transform->anglePitch * 50.0f / 414.0f * DEGREES_TO_RADIANS);
	MatrixMultiply(&transform->rot, &transform->rot, &transform->mat);
	MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
	MatrixMultiply(&transform->mat, &trans, &transform->rot);
	
	XformMatrixPerspective(&localToScreen, FovHorizontal(d3dRect.right - d3dRect.left), FovVertical(d3dRect.bottom - d3dRect.top), 1.0f, 2000000.0f);
	
	MatrixMultiply(&transform->mat, &pChunk->xForm, &transform->mat);
	MatrixMultiply(&localToScreen, &transform->mat, &localToScreen);

	// Transform 3D vertices to clip space.
	MatrixMultiplyVector(&region.topLeft, &localToScreen, &region.topLeft);
	MatrixMultiplyVector(&region.topRight, &localToScreen, &region.topRight);
	MatrixMultiplyVector(&region.bottomLeft, &localToScreen, &region.bottomLeft);
	MatrixMultiplyVector(&region.bottomRight, &localToScreen, &region.bottomRight);

	// Perspective divide to reach Normalized Device Coordinates.
	region.topLeft.x /= region.topLeft.w;
	region.topLeft.y /= region.topLeft.w;
	region.topLeft.z /= region.topLeft.w;
	region.bottomRight.x /= region.bottomRight.w;
	region.bottomRight.y /= region.bottomRight.w;
	region.bottomRight.z /= region.bottomRight.w;

	// Remap depth to D3D range and align bounding box corners.
	region.topLeft.z = region.topLeft.z * 2.0f - 1.0f;
	region.bottomRight.z = region.bottomRight.z * 2.0f - 1.0f;

	region.topRight.x = region.bottomRight.x;
	region.topRight.y = region.topLeft.y;
	region.topRight.z = region.topLeft.z;
	region.bottomLeft.x = region.topLeft.x;
	region.bottomLeft.y = region.bottomRight.y;
	region.bottomLeft.z = region.topLeft.z;

	region.center.x = (region.topLeft.x + region.topRight.x) / 2.0f;
	region.center.y = (region.topLeft.y + region.bottomLeft.y) / 2.0f;
	region.center.z = region.topLeft.z;
	
	return region;
}

// Returns true if BG overlay quad is both on-screen and within viewable depth range.
bool D3DIsBGOverlayVisible(overlay_region* region)
{
	bool isAnyCornerVisible =  
		(D3DRENDER_CLIP(region->topLeft.x, 1.0f)		&& D3DRENDER_CLIP(region->topLeft.y, 1.0f)) ||
		(D3DRENDER_CLIP(region->bottomLeft.x, 1.0f)		&& D3DRENDER_CLIP(region->bottomLeft.y, 1.0f)) ||
		(D3DRENDER_CLIP(region->topRight.x, 1.0f)		&& D3DRENDER_CLIP(region->topRight.y, 1.0f)) ||
		(D3DRENDER_CLIP(region->bottomRight.x, 1.0f)	&& D3DRENDER_CLIP(region->bottomRight.y, 1.0f));

	return (isAnyCornerVisible || D3DRENDER_CLIP(region->center.x, 1.0f)) && D3DRENDER_CLIP(region->topLeft.z, 1.0f);
}

/**
* Projects BG overlay into screen space and registers it for rendering.
* Calculates pixel boundaries, handles visibility entries, and enables click detection.
*/
void D3DFinalizeBGOverlay(BackgroundOverlay* overlay, overlay_region* region, overlay_transform* transform, ObjectRange* range,
		const BackgroundOverlaysSceneParams& bgoSceneParams)
{
	int tempLeft = (region->topLeft.x * region->width / 2) + (region->width / 2);
	int tempRight = (region->bottomRight.x * region->width / 2) + (region->width / 2);
	int tempTop = (region->topLeft.y * -(region->height / 2)) + (region->height / 2);
	int tempBottom = (region->bottomRight.y * -(region->height) / 2) + (region->height / 2);

	const auto* player = GetPlayerInfo();
	int distX = transform->pos.x - player->x;
	int distY = transform->pos.y - player->y;

	int distance = DistanceGet(distX, distY);

	if (range == NULL)
	{
		// Set up new visible object.
		range = &bgoSceneParams.visibleObjects[*bgoSceneParams.numVisibleObjects];
		range->id = overlay->obj.id;
		range->distance = distance;
		range->left_col = tempLeft;
		range->right_col = tempRight;
		range->top_row = tempTop;
		range->bottom_row = tempBottom;

		*bgoSceneParams.numVisibleObjects = std::min(*bgoSceneParams.numVisibleObjects + 1, MAXOBJECTS);
	}

	overlay->rcScreen.left = tempLeft;
	overlay->rcScreen.right = tempRight;
	overlay->rcScreen.top = tempTop;
	overlay->rcScreen.bottom = tempBottom;

	// The background overlay is visible and eligible for click detection.
	overlay->drawn = TRUE;

	// Record boundaries of drawing area.
	range->left_col = std::min(range->left_col, (long)tempLeft);
	range->right_col = std::max(range->right_col, (long)tempRight);
	range->top_row = std::min(range->top_row, (long)tempTop);
	range->bottom_row = std::max(range->bottom_row, (long)tempBottom);
}
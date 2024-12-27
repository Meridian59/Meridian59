// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

/**
* Render background overlays in the current room -- for example the Sun and Moon.
*/
void D3DRenderBackgroundOverlays(const BackgroundOverlaysRenderStateParams& bgoRenderStateParams, 
    const BackgroundOverlaysSceneParams& bgoSceneParams)
{
	MatrixIdentity(&bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, bgoRenderStateParams.vertexDeclaration);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);

	// Enable alpha blending and alpha testing for subsequent rendering
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);

	D3DRenderPoolReset(bgoRenderStateParams.worldPool, &D3DMaterialWorldPool);
	D3DCacheSystemReset(bgoRenderStateParams.worldCacheSystem);

	for (list_type list = bgoSceneParams.room->bg_overlays; list != NULL; list = list->next)
	{
		BackgroundOverlay* overlay = (BackgroundOverlay*)(list->data);

		PDIB pDib = GetObjectPdib(overlay->obj.icon_res, 0, overlay->obj.animate->group);
		if (NULL == pDib)
			continue;

		BYTE* bkgnd_bmap = DibPtr(pDib);
		if (bkgnd_bmap == NULL)
			continue;

		// The background overlay is not yet considered visible to the player.
		overlay->drawn = FALSE;

		// Increase the size of the background overlay if necessary.
		float size_scaler = 1.2f;

		// Specify the maximum and minimum altitude of the background overlay.
		// This will map from the -200 to 200 values return from the server to these values.
		int height_max = 200;
		int height_min = -200;

		long object_width = DibWidth(pDib) * size_scaler;
		long object_height = DibHeight(pDib) * size_scaler;

		d3d_render_packet_new* pPacket = D3DRenderPacketFindMatch(bgoRenderStateParams.worldPool, NULL, pDib, 0, 0, 0);
		if (NULL == pPacket)
			return;

		d3d_render_chunk_new* pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
		pChunk->flags |= D3DRENDER_NOAMBIENT | D3DRENDER_WORLD_OBJ;
		pChunk->zBias = ZBIAS_BASE;

		int piAngle = overlay->x;
		int piHeight = overlay->y;

		auto mapRange = [](double value, double in_min, double in_max, double out_min, double out_max) -> double {
			return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		};

		long mappedHeightValue = mapRange(piHeight, -200, 200, height_min, height_max);
		float angleInRadians = piAngle / 4096.0f;
		double azimuthalAngle = angleInRadians * 2 * PI;
		long radius = height_max * 5;
		double horizontalRadius = sqrt(pow(radius, 2) - pow(mappedHeightValue, 2));		

		const auto& params = bgoSceneParams.params;

		long x = params->viewer_x - (object_width / 2) + horizontalRadius * cos(azimuthalAngle);
		long y = params->viewer_y - (object_height / 2) + horizontalRadius * sin(azimuthalAngle);
		long z = params->viewer_height + mappedHeightValue;

		Vector3D bg_overlay_pos;
		bg_overlay_pos.x = x;
		bg_overlay_pos.y = y;
		bg_overlay_pos.z = z;

		D3DMATRIX rot, mat;
		MatrixIdentity(&mat);
		MatrixIdentity(&rot);

		const float FULL_CIRCLE_TO_DEGREES = 360.0f / 4096.0f;
		const float DEGREES_TO_RADIANS = PI / 180.0f;
		const float ANGLE_RANGE_TO_DEGREES = 45.0f / 414.0f;

		const auto& angleHeading = bgoSceneParams.angleHeading;
		const auto& anglePitch = bgoSceneParams.anglePitch;

		MatrixRotateY(&rot, (float)angleHeading * FULL_CIRCLE_TO_DEGREES * DEGREES_TO_RADIANS);
		MatrixRotateX(&mat, (float)anglePitch * ANGLE_RANGE_TO_DEGREES * DEGREES_TO_RADIANS);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, bg_overlay_pos.x, bg_overlay_pos.z, bg_overlay_pos.y);
		MatrixMultiply(&pChunk->xForm, &rot, &mat);

		pChunk->xyz[0].x = 0;
		pChunk->xyz[0].z = 0;
		pChunk->xyz[0].y = 0;

		pChunk->xyz[1].x = -object_width;
		pChunk->xyz[1].z = 0;
		pChunk->xyz[1].y = 0;

		pChunk->xyz[2].x = -object_width;
		pChunk->xyz[2].z = object_height;
		pChunk->xyz[2].y = 0;

		pChunk->xyz[3].x = 0;
		pChunk->xyz[3].z = object_height;
		pChunk->xyz[3].y = 0;

		for (int j = 0; j < 4; j++) {
			pChunk->bgra[j].b = 255;
			pChunk->bgra[j].g = 255;
			pChunk->bgra[j].r = 255;
			pChunk->bgra[j].a = 255;
		}

		const float epsilon = 0.007f; // Small value to adjust UV coordinates inward
		static const float u[4] = { epsilon, 1.0f - epsilon, 1.0f - epsilon, epsilon };
		static const float v[4] = { epsilon, epsilon, 1.0f - epsilon, 1.0f - epsilon };

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

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// Determine if the background overlay is visible for click detection in client
		// (e.g. Blinded by the light of the Sun).
		ObjectRange* range = FindVisibleObjectById(overlay->obj.id);

		const auto& d3dRect = bgoRenderStateParams.d3dRect;

		int w = d3dRect.right - d3dRect.left;
		int h = d3dRect.bottom - d3dRect.top;

		custom_xyzw topLeft;
		topLeft.x = pChunk->xyz[3].x;
		topLeft.y = pChunk->xyz[3].z;
		topLeft.z = 0;
		topLeft.w = 1.0f;

		custom_xyzw topRight;
		topRight.x = pChunk->xyz[3].x;
		topRight.y = pChunk->xyz[3].z;
		topRight.z = 0;
		topRight.w = 1.0f;

		custom_xyzw bottomLeft;
		bottomLeft.x = pChunk->xyz[1].x;
		bottomLeft.y = pChunk->xyz[1].z;
		bottomLeft.z = 0;
		bottomLeft.w = 1.0f;

		custom_xyzw bottomRight;
		bottomRight.x = pChunk->xyz[1].x;
		bottomRight.y = pChunk->xyz[1].z;
		bottomRight.z = 0;
		bottomRight.w = 1.0f;

		D3DMATRIX localToScreen, trans;
		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixRotateX(&mat, (float)anglePitch * 50.0f / 414.0f * PI / 180.0f);
		MatrixMultiply(&rot, &rot, &mat);
		MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
		MatrixMultiply(&mat, &trans, &rot);
		XformMatrixPerspective(&localToScreen, FovHorizontal(d3dRect.right - d3dRect.left), FovVertical(d3dRect.bottom - d3dRect.top), 1.0f, 2000000.0f);
		MatrixMultiply(&mat, &pChunk->xForm, &mat);
		MatrixMultiply(&localToScreen, &mat, &localToScreen);

		MatrixMultiplyVector(&topLeft, &localToScreen, &topLeft);
		MatrixMultiplyVector(&topRight, &localToScreen, &topRight);
		MatrixMultiplyVector(&bottomLeft, &localToScreen, &bottomLeft);
		MatrixMultiplyVector(&bottomRight, &localToScreen, &bottomRight);

		topLeft.x /= topLeft.w;
		topLeft.y /= topLeft.w;
		topLeft.z /= topLeft.w;
		bottomRight.x /= bottomRight.w;
		bottomRight.y /= bottomRight.w;
		bottomRight.z /= bottomRight.w;

		topLeft.z = topLeft.z * 2.0f - 1.0f;
		bottomRight.z = bottomRight.z * 2.0f - 1.0f;

		topRight.x = bottomRight.x;
		topRight.y = topLeft.y;
		topRight.z = topLeft.z;
		bottomLeft.x = topLeft.x;
		bottomLeft.y = bottomRight.y;
		bottomLeft.z = topLeft.z;

		custom_xyzw center;
		center.x = (topLeft.x + topRight.x) / 2.0f;
		center.y = (topLeft.y + bottomLeft.y) / 2.0f;
		center.z = topLeft.z;

		if (
			(
				(D3DRENDER_CLIP(topLeft.x, 1.0f) &&
					D3DRENDER_CLIP(topLeft.y, 1.0f)) ||
				(D3DRENDER_CLIP(bottomLeft.x, 1.0f) &&
					D3DRENDER_CLIP(bottomLeft.y, 1.0f)) ||
				(D3DRENDER_CLIP(topRight.x, 1.0f) &&
					D3DRENDER_CLIP(topRight.y, 1.0f)) ||
				(D3DRENDER_CLIP(bottomRight.x, 1.0f) &&
					D3DRENDER_CLIP(bottomRight.y, 1.0f)) ||
				(D3DRENDER_CLIP(center.x, 1.0f))
				) &&
			D3DRENDER_CLIP(topLeft.z, 1.0f))
		{

			int tempLeft = (topLeft.x * w / 2) + (w / 2);
			int tempRight = (bottomRight.x * w / 2) + (w / 2);
			int tempTop = (topLeft.y * -h / 2) + (h / 2);
			int tempBottom = (bottomRight.y * -h / 2) + (h / 2);

			const auto* player = GetPlayerInfo();
			int distX = bg_overlay_pos.x - player->x;
			int distY = bg_overlay_pos.y - player->y;

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

				*bgoSceneParams.numVisibleObjects = min(*bgoSceneParams.numVisibleObjects + 1, MAXOBJECTS);
			}

			overlay->rcScreen.left = tempLeft;
			overlay->rcScreen.right = tempRight;
			overlay->rcScreen.top = tempTop;
			overlay->rcScreen.bottom = tempBottom;

			// The background overlay is visible and eligable for click detection.
			overlay->drawn = TRUE;

			// Record boundaries of drawing area.
			range->left_col = min(range->left_col, tempLeft);
			range->right_col = max(range->right_col, tempRight);
			range->top_row = min(range->top_row, tempTop);
			range->bottom_row = max(range->bottom_row, tempBottom);
		}

	}
	D3DCacheFill(bgoRenderStateParams.worldCacheSystem, bgoRenderStateParams.worldPool, 1);
	D3DCacheFlush(bgoRenderStateParams.worldCacheSystem, bgoRenderStateParams.worldPool, 1, D3DPT_TRIANGLESTRIP);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);

	// Disable alpha blending and alpha testing
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);

	// restore the correct material and view matrices.
	MatrixIdentity(&bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &bgoRenderStateParams.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &bgoRenderStateParams.view);
}

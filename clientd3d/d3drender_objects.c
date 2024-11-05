// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Constants

static const auto TRANSLUCENT_FLAGS = OF_TRANSLUCENT25 | OF_TRANSLUCENT50 | OF_TRANSLUCENT75 | OF_DITHERTRANS;

// Interfaces

static void D3DRenderNamesDraw3D(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const FontTextureParams& fontTextureParams, 
	const LightAndTextureParams& lightAndTextureParams);

static void D3DRenderObjectsDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const GameObjectDataParams& gameObjectDataParams, 
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams,
	int flags);

static void D3DRenderOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const GameObjectDataParams& gameObjectDataParams, 
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams,
	bool underlays, 
	int flags);

static int D3DRenderProjectilesDraw(const ObjectsRenderParams& objectsRenderParams);

static void D3DRenderPlayerOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams);

static void D3DRenderPlayerOverlayOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams,
	list_type overlays,
	PDIB pDib, 
	AREA* objArea, 
	bool underlays);

static bool D3DObjectLightingCalc(
	room_type* room, 
	room_contents_node* pRNode, 
	custom_bgra* bgra, 
	DWORD flags, 
	bool fogEnabled,
	const LightAndTextureParams& lightAndTextureParams);

static int getKerningAmount(font_3d* pFont, char* str, char* ptr);
static bool D3DComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA* obj_area, const PlayerViewParams& playerViewParams);

// Functions

static bool IsInvisibleEffect(int flags) {
	return (flags & (OF_INVISIBLE | OF_DITHERINVIS)) == OF_INVISIBLE;
}

// Update the pChunks animation values as a function of time.
static void updateRenderChunkAnimationIntensity(d3d_render_chunk_new* pChunk)
{
	auto duration_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
	int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
	float animationIntensity = (milliseconds & 3) / 256.0f;

	pChunk->st1[0].s -= animationIntensity;
	pChunk->st1[0].t -= animationIntensity;
	pChunk->st1[1].s -= animationIntensity;
	pChunk->st1[1].t += animationIntensity;
	pChunk->st1[2].s += animationIntensity;
	pChunk->st1[2].t += animationIntensity;
	pChunk->st1[3].s += animationIntensity;
	pChunk->st1[3].t -= animationIntensity;
}

// Implementations

/**
* The main entry point for rendering objects in the game world.
* Returns the total time taken to render all objects.
*/
long D3DRenderObjects(
	const ObjectsRenderParams& objectsRenderParams, 
    const GameObjectDataParams& gameObjectDataParams, 
    const LightAndTextureParams& lightAndTextureParams,
    const FontTextureParams& fontTextureParams,
    const PlayerViewParams& playerViewParams)
{
	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE); // Ensure Z-write is enabled
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);      // Ensure Z-buffer is enabled

	long timeObjects = timeGetTime();
	const auto room = objectsRenderParams.room;
	const auto params = objectsRenderParams.params;

	if (config.draw_names)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclaration);

		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		D3DRenderPoolReset(objectsRenderParams.renderPool, &D3DMaterialObjectPool);
		D3DCacheSystemReset(objectsRenderParams.cacheSystem);
		D3DRenderNamesDraw3D(objectsRenderParams, playerViewParams, fontTextureParams, lightAndTextureParams);
		D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1);
		D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1, D3DPT_TRIANGLESTRIP);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, objectsRenderParams.driverProfile.magFilter);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, objectsRenderParams.driverProfile.minFilter);
	}

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclaration);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	D3DRenderPoolReset(objectsRenderParams.renderPool, &D3DMaterialObjectPool);
	D3DCacheSystemReset(objectsRenderParams.cacheSystem);

	// Render opaque objects
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 1, false);
	D3DRenderObjectsDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, false);
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 0, false);

	D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1);
	D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1, D3DPT_TRIANGLESTRIP);

	// Render translucent objects
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 1, TRANSLUCENT_FLAGS);
	D3DRenderObjectsDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, TRANSLUCENT_FLAGS);
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 0, TRANSLUCENT_FLAGS);

	*gameObjectDataParams.numObjects += D3DRenderProjectilesDraw(objectsRenderParams);
	D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1);
	D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1, D3DPT_TRIANGLESTRIP);

	SetZBias(gpD3DDevice, ZBIAS_DEFAULT);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);

	D3DRenderFramebufferTextureCreate(gameObjectDataParams.backBufferTexFull, gameObjectDataParams.backBufferTex[0],
		fontTextureParams.smallTextureSize, fontTextureParams.smallTextureSize);

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &objectsRenderParams.view);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &objectsRenderParams.proj);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclarationInvisible);

	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	// Render invisible world objects
	D3DRenderPoolReset(objectsRenderParams.renderPool, &D3DMaterialObjectInvisiblePool);
	D3DCacheSystemReset(objectsRenderParams.cacheSystem);
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 1, OF_INVISIBLE);
	D3DRenderObjectsDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, OF_INVISIBLE);
	D3DRenderOverlaysDraw(objectsRenderParams, gameObjectDataParams, playerViewParams, lightAndTextureParams, 0, OF_INVISIBLE);
	D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 2);
	D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 2, D3DPT_TRIANGLESTRIP);

	const auto* player = GetPlayerInfo();
	room_contents_node* pRNode = GetRoomObjectById(player->id);
	if (pRNode != nullptr)
	{
		// Rendering of Personal Equipment (Shields, weapons etc)
		if (IsInvisibleEffect(pRNode->obj.flags))
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclarationInvisible);

			D3DRenderPoolReset(objectsRenderParams.renderPool, &D3DMaterialObjectInvisiblePool);
			D3DCacheSystemReset(objectsRenderParams.cacheSystem);
			D3DRenderPlayerOverlaysDraw(objectsRenderParams, playerViewParams, lightAndTextureParams);
			D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 2);
			D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 2, D3DPT_TRIANGLESTRIP);
		}
		else
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclaration);

			D3DRenderPoolReset(objectsRenderParams.renderPool, &D3DMaterialObjectPool);
			D3DCacheSystemReset(objectsRenderParams.cacheSystem);
			D3DRenderPlayerOverlaysDraw(objectsRenderParams, playerViewParams, lightAndTextureParams);
			D3DCacheFill(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1);
			D3DCacheFlush(objectsRenderParams.cacheSystem, objectsRenderParams.renderPool, 1, D3DPT_TRIANGLESTRIP);
		}
	}
	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, objectsRenderParams.vertexDeclaration);

	return timeGetTime() - timeObjects;
}

/**
* Rendering names above objects, such as player names.
*/
void D3DRenderNamesDraw3D(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const FontTextureParams& fontTextureParams, 
	const LightAndTextureParams& lightAndTextureParams)
{
	D3DMATRIX			mat, rot, xForm, trans;
	int					sector_flags, offset;
	long				dx, dy, angle, top, bottom;
	custom_xyz			vector;
	custom_st			st[4];
	custom_bgra			bgra;
	TCHAR				c;
	Color				color;
	BYTE* palette;
	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new * pChunk;
	auto pFont = fontTextureParams.font;

	int angleHeading = objectsRenderParams.params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	int anglePitch = PlayerGetHeightOffset();

	// base objects
	for (list_type list = objectsRenderParams.room->contents; list != NULL; list = list->next)
	{
		float glyph_scale = 255;

		room_contents_node* pRNode = (room_contents_node*)list->data;

		const auto* player = GetPlayerInfo();
		if (pRNode->obj.id == player->id)
			continue;

		if (!(pRNode->obj.flags & OF_PLAYER) || IsInvisibleEffect(pRNode->obj.flags))
			continue;

		vector.x = pRNode->motion.x - player->x;
		vector.y = pRNode->motion.y - player->y;

		float distance = sqrtf((vector.x * vector.x) + (vector.y * vector.y));
		if (distance <= 0)
			distance = 1;

		if (distance >= MAX_NAME_DISTANCE)
			continue;

		PDIB pDib = GetObjectPdib(pRNode->obj.icon_res, 0, pRNode->obj.animate->group);

		if (NULL == pDib)
			continue;

		dx = pRNode->motion.x - objectsRenderParams.params->viewer_x;
		dy = pRNode->motion.y - objectsRenderParams.params->viewer_y;

		angle = (pRNode->angle - intATan2(-dy, -dx)) & NUMDEGREES_MASK;

		char* pName = LookupNameRsc(pRNode->obj.name_res);

		angle = pRNode->angle - (objectsRenderParams.params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		/* Make sure that object is above the floor. */
		if (!GetRoomHeight(objectsRenderParams.room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
		{
			continue;
		}

		// Set object depth based on "depth" sector flags
		float depth = lightAndTextureParams.sectorDepths[SectorDepth(sector_flags)];

		if (ROOM_OVERRIDE_MASK & GetRoomFlags()) // if depth flags are normal (no overrides)
		{
			switch (SectorDepth(sector_flags))
			{
			case SF_DEPTH1:
				if (ROOM_OVERRIDE_DEPTH1 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH1);
				}
				break;
			case SF_DEPTH2:
				if (ROOM_OVERRIDE_DEPTH2 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH2);
				}
				break;
			case SF_DEPTH3:
				if (ROOM_OVERRIDE_DEPTH3 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH3);
				}
				break;
			}
		}

		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, (float)pRNode->motion.x, (float)max(bottom,
			pRNode->motion.z) - depth +
			(((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f) +
			((float)pRNode->boundingHeightAdjust * 4.0f), (float)pRNode->motion.y);
		MatrixMultiply(&xForm, &rot, &mat);

		COLORREF fg_color = GetPlayerNameColor(pRNode->obj.flags, pName);

		// Some names never grow darker, they use PALETTEINDEX().
		if (HIBYTE(HIWORD(fg_color)) == HIBYTE(HIWORD(PALETTEINDEX(0))))
		{
			//     normally, SetTextColor() works with PALETTEINDEX() types fine,
			//     but not here for unknown reason
			//     so we convert to our base_palette[] PALETTERGB() type.
			//
			color = fontTextureParams.basePalette[LOBYTE(LOWORD(fg_color))];
		}
		else
		{
			// Draw name with color that fades with distance, just like object
			if (pRNode->obj.flags & (OF_FLICKERING | OF_FLASHING))
			{
				palette = GetLightPalette(D3DRENDER_LIGHT_DISTANCE, 63, FINENESS, 0);
			}
			else
			{
				palette = GetLightPalette(D3DRENDER_LIGHT_DISTANCE, 63, FINENESS, 0);
			}
			color = fontTextureParams.basePalette[palette[GetClosestPaletteIndex(fg_color)]];
			D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams);

			glyph_scale = max(bgra.b, bgra.g);
			glyph_scale = max(glyph_scale, bgra.r);
		}

		bgra.r = color.red * glyph_scale / 255;
		bgra.g = color.green * glyph_scale / 255;
		bgra.b = color.blue * glyph_scale / 255;
		bgra.a = COLOR_MAX;

		float lastDistance = 0;

		for (offset = 0; offset <= 1; offset++)
		{
			float x = 0.0f;
			float z = 0;
			char* ptr = pName;

			while (c = *ptr++)
			{
				int index = c - 32;
				float charWidth = (pFont->texST[index][1].s - pFont->texST[index][0].s) *
					pFont->texWidth;

				// Take out space for kerning
				int kerningAmount = getKerningAmount(pFont, pName, ptr - 1);
				float leading = min(0, pFont->abc[index].abcA + kerningAmount);
				float trailing = min(0, pFont->abc[index].abcC);
				charWidth += (leading + trailing);
				x += charWidth;
			}
			x *= (distance / FINENESS) / pFont->texScale;

			ptr = pName;

			while (c = *ptr++)
			{
				int	i;
				int index = c - 32;

				// flip t values since bmps are upside down
				st[0].s = pFont->texST[index][0].s;
				st[0].t = pFont->texST[index][1].t;
				st[1].s = pFont->texST[index][0].s;
				st[1].t = pFont->texST[index][0].t;
				st[2].s = pFont->texST[index][1].s;
				st[2].t = pFont->texST[index][0].t;
				st[3].s = pFont->texST[index][1].s;
				st[3].t = pFont->texST[index][1].t;

				float width = (st[2].s - st[0].s) * pFont->texWidth * 2.0f / pFont->texScale *
					(distance / FINENESS);
				float height = (st[0].t - st[2].t) * pFont->texHeight * 2.0f / pFont->texScale *
					(distance / FINENESS);

				pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, pFont->pTexture, NULL, 0, 0, 0);
				if (NULL == pPacket)
					return;
				pChunk = D3DRenderChunkNew(pPacket);
				assert(pChunk);

				pChunk->flags = pRNode->obj.flags;
				pChunk->numIndices = 4;
				pChunk->numVertices = 4;
				pChunk->numPrimitives = pChunk->numVertices - 2;
				if (offset == 0)
					pChunk->zBias = 255;
				else
					pChunk->zBias = 254;

				if (offset)
				{
					MatrixTranslate(&trans, -30.0f * distance / MAX_NAME_DISTANCE,
						-30.0f * distance / MAX_NAME_DISTANCE, 0);
					MatrixMultiply(&pChunk->xForm, &trans, &xForm);
				}
				else
					MatrixCopy(&pChunk->xForm, &xForm);

				pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
				pChunk->pMaterialFctn = &D3DMaterialObjectChunk;

				float leftx = x;
				// Kerning: add in leading for character if necessary
				int kerningAmount = getKerningAmount(pFont, pName, ptr - 1);
				float leading = pFont->abc[index].abcA + kerningAmount;
				float trailing = pFont->abc[index].abcC;
				leftx -= 2.0 * leading * (distance / FINENESS) / pFont->texScale;
				float rightx = leftx - width;

				pChunk->xyz[0].x = leftx;
				pChunk->xyz[0].y = 0;
				pChunk->xyz[0].z = z;

				pChunk->st0[0].s = st[0].s;
				pChunk->st0[0].t = st[0].t;

				pChunk->xyz[1].x = leftx;
				pChunk->xyz[1].y = 0;
				pChunk->xyz[1].z = z + height;

				pChunk->st0[1].s = st[1].s;
				pChunk->st0[1].t = st[1].t;

				pChunk->xyz[2].x = rightx;
				pChunk->xyz[2].y = 0;
				pChunk->xyz[2].z = z + height;

				pChunk->st0[2].s = st[2].s;
				pChunk->st0[2].t = st[2].t;

				pChunk->xyz[3].x = rightx;
				pChunk->xyz[3].y = 0;
				pChunk->xyz[3].z = z;

				pChunk->st0[3].s = st[3].s;
				pChunk->st0[3].t = st[3].t;

				if (offset != 0)
				{
					bgra.r = 0;
					bgra.g = 0;
					bgra.b = 0;
				}

				for (i = 0; i < 4; i++)
				{
					pChunk->bgra[i].r = bgra.r;
					pChunk->bgra[i].g = bgra.g;
					pChunk->bgra[i].b = bgra.b;
					pChunk->bgra[i].a = bgra.a;
				}

				pChunk->indices[0] = 1;
				pChunk->indices[1] = 2;
				pChunk->indices[2] = 0;
				pChunk->indices[3] = 3;

				if (pRNode->obj.flags & OF_SECONDTRANS)
				{
					pChunk->xLat0 = 0;
					pChunk->xLat1 = pRNode->obj.secondtranslation;
				}
				else if (pRNode->obj.flags & OF_DOUBLETRANS)
				{
					pChunk->xLat0 = pRNode->obj.translation;
					pChunk->xLat1 = pRNode->obj.secondtranslation;
				}
				else
				{
					pChunk->xLat0 = pRNode->obj.translation;
					pChunk->xLat1 = 0;
				}

				// Deal with kerning when moving to next character: character width
				// doesn't include overhangs
				x -= width;
				x -= 2.0 * (leading + trailing) * (distance / FINENESS) / pFont->texScale;
			}
		}
	}
}

/**
* Overlays are draw on top of objects (these include player limbs, animations over ornaments).
* They can be drawn under or over objects.
*/
void D3DRenderOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const GameObjectDataParams& gameObjectDataParams, 
	const PlayerViewParams& playerViewParams, 
	const LightAndTextureParams& lightAndTextureParams,
	bool underlays, 
	int flags)
{
	D3DMATRIX			mat, rot, trans;
	int					angleHeading, anglePitch, i, curObject;
	room_contents_node* pRNode;
	long				dx, dy, angle, top, bottom;
	PDIB				pDib, pDibOv, pDibOv2;
	Overlay* pOverlay;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	float				lastDistance, invShrink, invOvShrink, invOv2Shrink, depthf;
	BYTE				xLat0, xLat1, zBias;
	int					sector_flags;
	bool				bHotspot;

	d3d_render_packet_new* pPacket = NULL;
	d3d_render_chunk_new* pChunk = NULL;

	const auto* player = GetPlayerInfo();;

	angleHeading = objectsRenderParams.params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	// drawdata[] may contain more than one entry with the same id.
	// Rendering translucent objects multiple times can cause them to appear less translucent (more opaque).
	// This happens because each rendering pass compounds the alpha blending, making the object appear more solid than intended.
	// To prevent this, we need to keep track of those we've already processed to avoid rendering duplicates.
	std::unordered_set<int> processedIds;
	auto drawdata = gameObjectDataParams.drawData;
	for (curObject = 0; curObject < gameObjectDataParams.numItems; curObject++)
	{
		list_type	list2;
		int			pass, depth;

		if (drawdata[curObject].type != DrawObjectType)
			continue;

		pRNode = drawdata[curObject].u.object.object->draw.obj;

		if (pRNode == NULL)
			continue;

		if (processedIds.find(pRNode->obj.id) != processedIds.end())
			continue;
		processedIds.insert(pRNode->obj.id);

		if (pRNode->obj.id == player->id)
			continue;

		bool objInvisible = IsInvisibleEffect(pRNode->obj.flags);
		if ((flags & OF_INVISIBLE) == OF_INVISIBLE)
		{
			if (!objInvisible)
				continue;
		}
		else
		{
			if (objInvisible)
				continue;
		}

		// Check for translucent objects
		bool objTranslucent = (pRNode->obj.flags & TRANSLUCENT_FLAGS) != 0;
		if ((flags & TRANSLUCENT_FLAGS) == 0) {
			if (objTranslucent)
				continue;
		}
		else
		{
			if (!objTranslucent)
				continue;
		}

		if (NULL == *pRNode->obj.overlays)
			continue;

		// three passes each
		for (pass = 0; pass < 3; pass++)
		{
			// flush cache between passes
			// unlock all buffers

			if (underlays)
				switch (pass)
				{
				case 0:
					depth = HOTSPOT_UNDERUNDER;
					zBias = ZBIAS_UNDERUNDER;
					break;

				case 1:
					depth = HOTSPOT_UNDER;
					zBias = ZBIAS_UNDER;
					break;

				case 2:
					depth = HOTSPOT_UNDEROVER;
					zBias = ZBIAS_UNDEROVER;
					break;
				}
			else
				switch (pass)
				{
				case 0:
					depth = HOTSPOT_OVERUNDER;
					zBias = ZBIAS_OVERUNDER;
					break;

				case 1:
					depth = HOTSPOT_OVER;
					zBias = ZBIAS_OVER;
					break;

				case 2:
					depth = HOTSPOT_OVEROVER;
					zBias = ZBIAS_OVEROVER;
					break;
				}

			for (list2 = *pRNode->obj.overlays; list2 != NULL; list2 = list2->next)
			{
				bHotspot = true;

				pOverlay = (Overlay*)list2->data;

				if (NULL == pOverlay)
					continue;

				dx = pRNode->motion.x - objectsRenderParams.params->viewer_x;
				dy = pRNode->motion.y - objectsRenderParams.params->viewer_y;

				angle = (pRNode->angle - intATan2(-dy, -dx)) & NUMDEGREES_MASK;

				pDib = GetObjectPdib(pRNode->obj.icon_res, angle, pRNode->obj.animate->group);
				pDibOv = GetObjectPdib(pOverlay->icon_res, angle, pOverlay->animate.group);

				if ((NULL == pDib) || (NULL == pDibOv))
					continue;

				invShrink = 1.0f / pDib->shrink;
				invOvShrink = 1.0f / pDibOv->shrink;

				if (pOverlay->hotspot)
				{
					POINT	point;
					int		retVal, retVal2;

					retVal = FindHotspotPdib(pDib, pOverlay->hotspot, &point);

					if (retVal != HOTSPOT_NONE)
					{
						if (retVal != depth)
							continue;

						// set top right corner at top right corner of base bitmap
						xyz[3].x = (float)pDib->width * invShrink * 8.0f;
						xyz[3].z = ((float)pDib->height * invShrink * 16.0f);

						xyz[2].x = xyz[3].x;
						xyz[2].z = xyz[3].z -
							((float)pDibOv->height * invOvShrink * 16.0f);

						xyz[1].x = xyz[3].x -
							(float)pDibOv->width * invOvShrink * 16.0f;;
						xyz[1].z = xyz[3].z -
							((float)pDibOv->height * invOvShrink * 16.0f);

						xyz[0].x = xyz[3].x -
							(float)pDibOv->width * invOvShrink * 16.0f;
						xyz[0].z = xyz[3].z;

						// add base object's offsets
						for (i = 0; i < 4; i++)
						{
							xyz[i].x += pDib->xoffset;
							xyz[i].z -= pDib->yoffset * 4.0f;
						}

						for (i = 0; i < 4; i++)
						{
							// add hotspot
							xyz[i].x -= point.x * 16.000f * invShrink;
							xyz[i].z -= (float)point.y * 16.000f * invShrink;
							// add overlay offsets
							xyz[i].x -= pDibOv->xoffset * 16.000f * invShrink;
							xyz[i].z -= pDibOv->yoffset * 16.000f * invShrink;
						}

						bHotspot = true;
					}
					else
					{
						// the hotspot wasn't in the base object pdib
						// must be an overlay on an overlay, so find base overlay
						list_type	tempList;

						for (tempList = *pRNode->obj.overlays; tempList != NULL; tempList = tempList->next)
						{
							POINT	point2;
							Overlay* pTempOverlay = (Overlay*)tempList->data;

							if (pTempOverlay == NULL)
								continue;

							pDibOv2 = GetObjectPdib(pTempOverlay->icon_res, angle, pTempOverlay->animate.group);

							if (NULL == pDibOv2)
								continue;

							invOv2Shrink = 1.0f / pDibOv2->shrink;

							if ((retVal = FindHotspotPdib(pDibOv2, pOverlay->hotspot, &point2)) != HOTSPOT_NONE)
							{
								int	tempRetVal;

								// pDib = the base object
								// pDibOv2 = the base overlay
								// pDibOv = the current overlay we are drawing
								retVal2 = FindHotspotPdib(pDib, pTempOverlay->hotspot, &point);

								if (retVal2 != HOTSPOT_NONE)
								{
									if (retVal == HOTSPOT_OVER)
									{
										if (retVal2 == HOTSPOT_OVER)
											tempRetVal = HOTSPOT_OVEROVER;
										else
											tempRetVal = HOTSPOT_UNDEROVER;
									}
									else
									{
										if (retVal2 == HOTSPOT_OVER)
											tempRetVal = HOTSPOT_OVERUNDER;
										else
											tempRetVal = HOTSPOT_UNDERUNDER;
									}

									if (tempRetVal != depth)
										goto TEMP_END2;

									// set top right corner at top right corner of base bitmap
									xyz[3].x = (float)pDib->width * invShrink * 8.0f;
									xyz[3].z = ((float)pDib->height * invShrink * 16.0f);

									xyz[2].x = xyz[3].x;
									xyz[2].z = xyz[3].z -
										((float)pDibOv->height * invOvShrink * 16.0f);

									xyz[1].x = xyz[3].x -
										(float)pDibOv->width * invOvShrink * 16.0f;;
									xyz[1].z = xyz[3].z -
										((float)pDibOv->height * invOvShrink * 16.0f);

									xyz[0].x = xyz[3].x -
										(float)pDibOv->width * invOvShrink * 16.0f;
									xyz[0].z = xyz[3].z;

									// add base object's offsets
									for (i = 0; i < 4; i++)
									{
										xyz[i].x += pDib->xoffset;
										xyz[i].z -= pDib->yoffset * 4.0f;
									}

									for (i = 0; i < 4; i++)
									{
										// add hotspot
										xyz[i].x -= point.x * 16.000f * invShrink;
										xyz[i].z -= (float)point.y * 16.000f * invShrink;
										// add overlay offsets
										xyz[i].x -= pDibOv2->xoffset * 16.000f * invShrink;
										xyz[i].z -= pDibOv2->yoffset * 16.000f * invShrink;
										// add hotspot
										xyz[i].x -= point2.x * 16.000f * invOv2Shrink;
										xyz[i].z -= (float)point2.y * 16.000f * invOv2Shrink;
										// add overlay offsets
										xyz[i].x -= pDibOv->xoffset * 16.000f * invOv2Shrink;
										xyz[i].z -= pDibOv->yoffset * 16.000f * invOv2Shrink;
									}

									bHotspot = true;
								}
								else
									continue;
							}
							else
								continue;
						}
					}
				}
				else
					continue;

				if (bHotspot)
				{
					if (pRNode->obj.flags & OF_SECONDTRANS)
					{
						xLat0 = 0;
						xLat1 = pRNode->obj.secondtranslation;
					}
					else if (pRNode->obj.flags & OF_DOUBLETRANS)
					{
						xLat0 = pOverlay->translation;
						xLat1 = pRNode->obj.secondtranslation;
					}
					else
					{
						xLat0 = pOverlay->translation;
						xLat1 = 0;
					}

					pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDibOv, xLat0, xLat1,
						GetDrawingEffect(pRNode->obj.flags));

					if (NULL == pPacket)
						continue;

					pChunk = D3DRenderChunkNew(pPacket);
					assert(pChunk);

					pChunk->flags = pRNode->obj.flags;
					pChunk->numIndices = 4;
					pChunk->numVertices = 4;
					pChunk->numPrimitives = pChunk->numVertices - 2;
					pChunk->xLat0 = xLat0;
					pChunk->xLat1 = xLat1;
					pChunk->zBias = zBias;

					zBias++;

					if (IsInvisibleEffect(pRNode->obj.flags))
					{
						pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
						pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
					}
					else
					{
						pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
						pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
					}

					for (i = 0; i < 4; i++)
					{
						pChunk->xyz[i].x = xyz[i].x;
						pChunk->xyz[i].y = 0;//zHack;
						pChunk->xyz[i].z = xyz[i].z;
					}

					angle = pRNode->angle - (objectsRenderParams.params->viewer_angle + 3072);

					if (angle < -4096)
						angle += 4096;

					/* Make sure that object is above the floor. */
					if (!GetRoomHeight(objectsRenderParams.room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
					{
						continue;
					}

					// Set object depth based on "depth" sector flags
					depthf = lightAndTextureParams.sectorDepths[SectorDepth(sector_flags)];

					if (ROOM_OVERRIDE_MASK & GetRoomFlags()) // if depth flags are normal (no overrides)
					{
						switch (SectorDepth(sector_flags))
						{
						case SF_DEPTH1:
							if (ROOM_OVERRIDE_DEPTH1 & GetRoomFlags())
							{
								depthf = GetOverrideRoomDepth(SF_DEPTH1);
							}
							break;
						case SF_DEPTH2:
							if (ROOM_OVERRIDE_DEPTH2 & GetRoomFlags())
							{
								depthf = GetOverrideRoomDepth(SF_DEPTH2);
							}
							break;
						case SF_DEPTH3:
							if (ROOM_OVERRIDE_DEPTH3 & GetRoomFlags())
							{
								depthf = GetOverrideRoomDepth(SF_DEPTH3);
							}
							break;
						}
					}

					MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
					MatrixTranspose(&rot, &rot);
					MatrixTranslate(&mat, (float)pRNode->motion.x, (float)max(bottom,
						pRNode->motion.z) - depthf, (float)pRNode->motion.y);
					MatrixMultiply(&pChunk->xForm, &rot, &mat);

					lastDistance = 0;

					if (GetDrawingEffect(pRNode->obj.flags) == OF_BLACK)
					{
						bgra.b = bgra.g = bgra.r = 0;
						bgra.a = 255;
					}
					else
					{
						if (D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, 
							objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams))
							pChunk->flags |= D3DRENDER_NOAMBIENT;
					}

					if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT25 >> 20))
						bgra.a = D3DRENDER_TRANS25;
					if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT50 >> 20))
						bgra.a = D3DRENDER_TRANS50;
					if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT75 >> 20))
						bgra.a = D3DRENDER_TRANS75;
					if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERTRANS >> 20))
						bgra.a = D3DRENDER_TRANS50;
					if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERINVIS >> 20))
						bgra.a = D3DRENDER_TRANS50;

					if (bgra.a != 255)
						D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA,
							D3DBLEND_INVSRCALPHA);

					for (i = 0; i < 4; i++)
					{
						pChunk->bgra[i].b = bgra.b;
						pChunk->bgra[i].g = bgra.g;
						pChunk->bgra[i].r = bgra.r;
						pChunk->bgra[i].a = bgra.a;
					}

					{
						float oneOverW, oneOverH;

						oneOverW = (1.0f / pDibOv->width);
						oneOverH = (1.0f / pDibOv->height);

						pChunk->st0[0].s = 1.0f - oneOverW;
						pChunk->st0[0].t = oneOverH;
						pChunk->st0[1].s = 1.0f - oneOverW;
						pChunk->st0[1].t = 1.0f - oneOverH;
						pChunk->st0[2].s = oneOverW;
						pChunk->st0[2].t = 1.0f - oneOverH;
						pChunk->st0[3].s = oneOverW;
						pChunk->st0[3].t = oneOverH;
					}

					pChunk->indices[0] = 1;
					pChunk->indices[1] = 2;
					pChunk->indices[2] = 0;
					pChunk->indices[3] = 3;

					// now add object to visible object list
					if ((pRNode->obj.id != INVALID_ID) && (pRNode->obj.id != player->id))
					{
						D3DMATRIX	localToScreen, rot, mat;
						custom_xyzw	topLeft, topRight, bottomLeft, bottomRight, center;
						ObjectRange* range = FindVisibleObjectById(pRNode->obj.id);
						int			w, h;
						int			tempLeft, tempRight, tempTop, tempBottom;
						int			distX, distY, distance;

						w = playerViewParams.d3dRect.right - playerViewParams.d3dRect.left;
						h = playerViewParams.d3dRect.bottom - playerViewParams.d3dRect.top;

						topLeft.x = pChunk->xyz[3].x;
						topLeft.y = pChunk->xyz[3].z;
						topLeft.z = 0;
						topLeft.w = 1.0f;

						topRight.x = pChunk->xyz[3].x;
						topRight.y = pChunk->xyz[3].z;
						topRight.z = 0;
						topRight.w = 1.0f;

						bottomLeft.x = pChunk->xyz[1].x;
						bottomLeft.y = pChunk->xyz[1].z;
						bottomLeft.z = 0;
						bottomLeft.w = 1.0f;

						bottomRight.x = pChunk->xyz[1].x;
						bottomRight.y = pChunk->xyz[1].z;
						bottomRight.z = 0;
						bottomRight.w = 1.0f;

						MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
						MatrixRotateX(&mat, (float)anglePitch * 50.0f / 414.0f * PI / 180.0f);
						MatrixMultiply(&rot, &rot, &mat);
						MatrixTranslate(&trans, -(float)objectsRenderParams.params->viewer_x, 
							-(float)objectsRenderParams.params->viewer_height, -(float)objectsRenderParams.params->viewer_y);
						MatrixMultiply(&mat, &trans, &rot);
						XformMatrixPerspective(&localToScreen, FovHorizontal(w), FovVertical(h), 1.0f, Z_RANGE);
						MatrixMultiply(&mat, &pChunk->xForm,
							&mat);
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

						center.x = (topLeft.x + topRight.x) / 2.0f;
						center.y = (topLeft.y + bottomLeft.y) / 2.0f;
						center.z = topLeft.z;

						if (IsInvisibleEffect(flags))
						{
							pChunk->st1[0].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[0].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[1].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[1].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[2].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[2].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[3].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[3].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;

							updateRenderChunkAnimationIntensity(pChunk);
						}

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
								(D3DRENDER_CLIP(center.x, 1.0f) &&
									D3DRENDER_CLIP(center.y, 1.0f))
								) &&
							D3DRENDER_CLIP(topLeft.z, 1.0f))
						{
							tempLeft = (topLeft.x * w / 2) + (w / 2);
							tempRight = (bottomRight.x * w / 2) + (w / 2);
							tempTop = (topLeft.y * -h / 2) + (h / 2);
							tempBottom = (bottomRight.y * -h / 2) + (h / 2);

							distX = pRNode->motion.x - player->x;
							distY = pRNode->motion.y - player->y;

							distance = DistanceGet(distX, distY);

							if (range == NULL)
							{
								// Set up new visible object
								range = &gameObjectDataParams.visibleObjects[*gameObjectDataParams.numVisibleObjects];
								range->id = pRNode->obj.id;
								range->distance = distance;
								range->left_col = tempLeft;
								range->right_col = tempRight;
								range->top_row = tempTop;
								range->bottom_row = tempBottom;

								*gameObjectDataParams.numVisibleObjects = min(*gameObjectDataParams.numVisibleObjects + 1, MAXOBJECTS);
							}

							/* Record boundaries of drawing area */
							range->left_col = min(range->left_col, tempLeft);
							range->right_col = max(range->right_col, tempRight);

							if (tempTop <= range->top_row)
							{
								int	i;
								pRNode->boundingHeightAdjust =
									((pDib->height / pDib->shrink) - pDib->yoffset) / 4.0f;
								i = 0;
							}

							range->top_row = min(range->top_row, tempTop);
							range->bottom_row = max(range->bottom_row, tempBottom);
						}
					}

					if (pRNode->obj.id != INVALID_ID 
						&& pRNode->obj.id == GetUserTargetID() 
						&& !IsInvisibleEffect(pRNode->obj.flags))
					{
						pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDibOv, xLat0, xLat1,
							GetDrawingEffect(pRNode->obj.flags));
						if (NULL == pPacket)
							continue;
						pPacket->pMaterialFctn = &D3DMaterialObjectPacket;

						pChunk = D3DRenderChunkNew(pPacket);
						assert(pChunk);

						pChunk->flags = pRNode->obj.flags | OF_TRANSLUCENT50;
						pChunk->numIndices = 4;
						pChunk->numVertices = 4;
						pChunk->numPrimitives = pChunk->numVertices - 2;
						pChunk->xLat0 = xLat0;
						pChunk->xLat1 = xLat1;
						pChunk->zBias = ZBIAS_TARGETED;
						pChunk->isTargeted = TRUE;

						MatrixMultiply(&pChunk->xForm, &rot, &mat);

						if (IsInvisibleEffect(pRNode->obj.flags))
						{
							pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
						}
						else
						{
							pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
						}

						D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, 
							objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams);

						for (i = 0; i < 4; i++)
						{
							pChunk->xyz[i].x = xyz[i].x;
							pChunk->xyz[i].y = 0;
							pChunk->xyz[i].z = xyz[i].z;

							switch (config.halocolor)
							{
							case 0:
								pChunk->bgra[i].b = 0;
								pChunk->bgra[i].g = 0;
								pChunk->bgra[i].r = min(bgra.r * 2.0f, 255);
								pChunk->bgra[i].a = 255;
								break;

							case 1:
								pChunk->bgra[i].b = min(bgra.r * 2.0f, 255);
								pChunk->bgra[i].g = 0;
								pChunk->bgra[i].r = 0;
								pChunk->bgra[i].a = 255;
								break;

							default:
								pChunk->bgra[i].b = 0;
								pChunk->bgra[i].g = min(bgra.r * 2.0f, 255);
								pChunk->bgra[i].r = 0;
								pChunk->bgra[i].a = 255;
								break;
							}
						}

						pChunk->xyz[0].x -= 32.0f * invOvShrink;
						pChunk->xyz[0].z += 32.0f * invOvShrink;
						pChunk->xyz[1].x -= 32.0f * invOvShrink;
						pChunk->xyz[1].z -= 32.0f * invOvShrink;
						pChunk->xyz[2].x += 32.0f * invOvShrink;
						pChunk->xyz[2].z -= 32.0f * invOvShrink;
						pChunk->xyz[3].x += 32.0f * invOvShrink;
						pChunk->xyz[3].z += 32.0f * invOvShrink;

						pChunk->st0[0].s = 1.0f;
						pChunk->st0[0].t = 0.0f;
						pChunk->st0[1].s = 1.0f;
						pChunk->st0[1].t = 1.0f;
						pChunk->st0[2].s = 0.0f;
						pChunk->st0[2].t = 1.0f;
						pChunk->st0[3].s = 0.0f;
						pChunk->st0[3].t = 0.0f;

						pChunk->indices[0] = 1;
						pChunk->indices[1] = 2;
						pChunk->indices[2] = 0;
						pChunk->indices[3] = 3;
					}

					*gameObjectDataParams.numObjects++;
				TEMP_END2: // ewww we have a goto.. todo: lets remove
					{
						int i = 0;
					}
				}
			}
		}
	}
}

/**
* Draw all objects in the room.
* They may be opaque, invivisble or translucent. We draw these based on the flags passed in.
*/
void D3DRenderObjectsDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const GameObjectDataParams& gameObjectDataParams, 
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams,
	int flags)
{
	D3DMATRIX			mat, rot, trans;
	int					angleHeading, anglePitch, i, curObject;
	room_contents_node* pRNode;
	long				dx, dy, angle;
	PDIB				pDib;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	float				lastDistance, depth;
	BYTE				xLat0, xLat1;
	int					sector_flags;
	long				top, bottom;

	d3d_render_packet_new* pPacket = NULL;
	d3d_render_chunk_new* pChunk = NULL;

	angleHeading = objectsRenderParams.params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	auto drawdata = gameObjectDataParams.drawData;

	// We track all objects that are in similar positions in the 3D world.
	// This is to mitigate z-fighting by incrementing z-depths for each unique object in the same position.
	// The key is composed of the x and y coordinates of the object and the value is the current
	// count of objects found at that location.
	std::unordered_map<int64, int> depth_adjustment_map;

	// As we receive objects in different orders from the BSP walk this can cause inconsistent z-depth ordering.
	// We now attempt to maintain a consistent view by sorting draw data by their ids.
	std::sort(drawdata, drawdata + gameObjectDataParams.numItems, [](const DrawItem& a, const DrawItem& b) {

		if (a.type != DrawObjectType && b.type == DrawObjectType) {
			return false;
		}
		if (a.type == DrawObjectType && b.type != DrawObjectType) {
			return true;
		}

		// If both items are of the DrawObjectType, compare their draw.id
		if (a.type == DrawObjectType && b.type == DrawObjectType) {
			// Compare draw.id from the object union member
			// drawdata[] may contain more than one entry with the same id so this is not a stable sort.
			return a.u.object.object->draw.id < b.u.object.object->draw.id;
		}

		// If the types are the same and not DrawObjectType, keep the original order.
		return false;
	});

	// drawdata[] may contain more than one entry with the same id.
	// We need to keep track of those we've already processed to avoid duplicates.
	std::unordered_set<int> processedIds;

	for (curObject = 0; curObject < gameObjectDataParams.numItems; curObject++)
	{
		if (drawdata[curObject].type != DrawObjectType)
			continue;

		pRNode = drawdata[curObject].u.object.object->draw.obj;

		if (pRNode == NULL)
			continue;

		const auto* player = GetPlayerInfo();

		if (pRNode->obj.id == player->id)
			continue;

		if (processedIds.find(pRNode->obj.id) != processedIds.end())
			continue;
		processedIds.insert(pRNode->obj.id);

		// Check for invisible objects
		bool objInvisible = IsInvisibleEffect(pRNode->obj.flags);
		if ((flags & OF_INVISIBLE) == OF_INVISIBLE)
		{
			if (!objInvisible)
				continue;
		}
		else
		{
			if (objInvisible)
				continue;
		}

		// Check for translucent objects
		bool objTranslucent = (pRNode->obj.flags & TRANSLUCENT_FLAGS) != 0;
		if ((flags & TRANSLUCENT_FLAGS) != 0) {
			if (!objTranslucent)
				continue;
		}
		else
		{
			if (objTranslucent)
				continue;
		}

		dx = pRNode->motion.x - objectsRenderParams.params->viewer_x;
		dy = pRNode->motion.y - objectsRenderParams.params->viewer_y;

		angle = (pRNode->angle - intATan2(-dy, -dx)) & NUMDEGREES_MASK;

		pDib = GetObjectPdib(pRNode->obj.icon_res, angle, pRNode->obj.animate->group);

		if (NULL == pDib)
			continue;

		if (pRNode->obj.flags & OF_SECONDTRANS)
		{
			xLat0 = 0;
			xLat1 = pRNode->obj.secondtranslation;
		}
		else if (pRNode->obj.flags & OF_DOUBLETRANS)
		{
			xLat0 = pRNode->obj.translation;
			xLat1 = pRNode->obj.secondtranslation;
		}
		else
		{
			xLat0 = pRNode->obj.translation;
			xLat1 = 0;
		}

		pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDib, xLat0, xLat1,
			GetDrawingEffect(pRNode->obj.flags));
		if (NULL == pPacket)
			return;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->flags = pRNode->obj.flags;

		if (IsInvisibleEffect(pRNode->obj.flags))
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
		}
		else
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
		}

		angle = pRNode->angle - (objectsRenderParams.params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		/* Make sure that object is above the floor. */
		if (!GetRoomHeight(objectsRenderParams.room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
		{
			continue;
		}

		// Set object depth based on "depth" sector flags
		depth = lightAndTextureParams.sectorDepths[SectorDepth(sector_flags)];

		if (ROOM_OVERRIDE_MASK & GetRoomFlags()) // if depth flags are normal (no overrides)
		{
			switch (SectorDepth(sector_flags))
			{
			case SF_DEPTH1:
				if (ROOM_OVERRIDE_DEPTH1 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH1);
				}
				break;
			case SF_DEPTH2:
				if (ROOM_OVERRIDE_DEPTH2 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH2);
				}
				break;
			case SF_DEPTH3:
				if (ROOM_OVERRIDE_DEPTH3 & GetRoomFlags())
				{
					depth = GetOverrideRoomDepth(SF_DEPTH3);
				}
				break;
			}
		}

		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, (float)pRNode->motion.x, max(bottom, pRNode->motion.z) - depth,
			(float)pRNode->motion.y);
		MatrixMultiply(&pChunk->xForm, &rot, &mat);

		xyz[0].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		xyz[0].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;
		xyz[0].y = 0.0f;

		xyz[1].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		xyz[1].z = -(float)pDib->yoffset * 4.0f;
		xyz[1].y = 0.0f;

		xyz[2].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		xyz[2].z = -(float)pDib->yoffset * 4.0f;
		xyz[2].y = 0.0f;

		xyz[3].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		xyz[3].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;
		xyz[3].y = 0.0f;

		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = xLat0;
		pChunk->xLat1 = xLat1;

		// For players and objects with a bounding height adjustment (such as Tos Fountain), we draw them at the base depth.
		// This is because they have multiple layers positioned relative to base depth (e.g. behind, in front of and so on).
		// For everything else we start with the deault z bias which positions them behind these more complex arrangements.
		pChunk->zBias = (pRNode->obj.flags & OF_PLAYER) || (pRNode->boundingHeightAdjust != 0) ? ZBIAS_BASE : ZBIAS_DEFAULT;

		if (pRNode->obj.flags & OF_GETTABLE)
		{
			// Typical items such as reagents, keys, etc. are drawn at the default depth
			// offset by the number of items already drawn at this location.

			// Combine objects x and y position into a single int64 for the map key.
			int64 key = ((int64)pRNode->motion.x << 32) | (int)(pRNode->motion.y & 0xFFFFFFFF);

			// Increment the counter at the appropriate bin and assign the appropriate zBias.
			pChunk->zBias += (BYTE)depth_adjustment_map[key]++;
		}

		lastDistance = 0;

		if (GetDrawingEffect(pRNode->obj.flags) == OF_BLACK)
		{
			bgra.b = bgra.g = bgra.r = 0;
			bgra.a = 255;
		}
		else
		{
			if (D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, 
				objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams))
				pChunk->flags |= D3DRENDER_NOAMBIENT;
		}

		if (GetDrawingEffect(pRNode->obj.flags) == OF_TRANSLUCENT25)
			bgra.a = D3DRENDER_TRANS25;
		if (GetDrawingEffect(pRNode->obj.flags) == OF_TRANSLUCENT50)
			bgra.a = D3DRENDER_TRANS50;
		if (GetDrawingEffect(pRNode->obj.flags) == OF_TRANSLUCENT75)
			bgra.a = D3DRENDER_TRANS75;
		if (GetDrawingEffect(pRNode->obj.flags) == OF_DITHERTRANS)
			bgra.a = D3DRENDER_TRANS50;
		if (GetDrawingEffect(pRNode->obj.flags) == OF_DITHERINVIS)
			bgra.a = D3DRENDER_TRANS50;

		if (bgra.a != 255)
			D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		for (i = 0; i < 4; i++)
		{
			pChunk->xyz[i].x = xyz[i].x;
			pChunk->xyz[i].y = xyz[i].y;
			pChunk->xyz[i].z = xyz[i].z;

			pChunk->bgra[i].b = bgra.b;
			pChunk->bgra[i].g = bgra.g;
			pChunk->bgra[i].r = bgra.r;
			pChunk->bgra[i].a = bgra.a;
		}

		{
			float oneOverW, oneOverH;

			oneOverW = (1.0f / pDib->width);
			oneOverH = (1.0f / pDib->height);

			pChunk->st0[0].s = 1.0f - oneOverW;
			pChunk->st0[0].t = oneOverH;
			pChunk->st0[1].s = 1.0f - oneOverW;
			pChunk->st0[1].t = 1.0f - oneOverH;
			pChunk->st0[2].s = oneOverW;
			pChunk->st0[2].t = 1.0f - oneOverH;
			pChunk->st0[3].s = oneOverW;
			pChunk->st0[3].t = oneOverH;
		}

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// now add object to visible object list
		if ((pRNode->obj.id != INVALID_ID) && (pRNode->obj.id != player->id))
		{
			D3DMATRIX	localToScreen, rot, mat;
			custom_xyzw	topLeft, topRight, bottomLeft, bottomRight, center;
			ObjectRange* range = FindVisibleObjectById(pRNode->obj.id);
			int			w, h;
			int			tempLeft, tempRight, tempTop, tempBottom;
			int			distX, distY, distance;
			if (pRNode->obj.id == player->id)
				break;

			w = playerViewParams.d3dRect.right - playerViewParams.d3dRect.left;
			h = playerViewParams.d3dRect.bottom - playerViewParams.d3dRect.top;

			topLeft.x = pChunk->xyz[3].x;
			topLeft.y = pChunk->xyz[3].z;
			topLeft.z = 0;
			topLeft.w = 1.0f;

			topRight.x = pChunk->xyz[3].x;
			topRight.y = pChunk->xyz[3].z;
			topRight.z = 0;
			topRight.w = 1.0f;

			bottomLeft.x = pChunk->xyz[1].x;
			bottomLeft.y = pChunk->xyz[1].z;
			bottomLeft.z = 0;
			bottomLeft.w = 1.0f;

			bottomRight.x = pChunk->xyz[1].x;
			bottomRight.y = pChunk->xyz[1].z;
			bottomRight.z = 0;
			bottomRight.w = 1.0f;

			MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
			MatrixRotateX(&mat, (float)anglePitch * 50.0f / 414.0f * PI / 180.0f);
			MatrixMultiply(&rot, &rot, &mat);
			MatrixTranslate(&trans, -(float)objectsRenderParams.params->viewer_x, 
				-(float)objectsRenderParams.params->viewer_height, -(float)objectsRenderParams.params->viewer_y);
			MatrixMultiply(&mat, &trans, &rot);
			XformMatrixPerspective(&localToScreen, FovHorizontal(w), FovVertical(h), 1.0f, 2000000.0f);
			MatrixMultiply(&mat, &pChunk->xForm,
				&mat);
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

			center.x = (topLeft.x + topRight.x) / 2.0f;
			center.y = (topLeft.y + bottomLeft.y) / 2.0f;
			center.z = topLeft.z;

			if (IsInvisibleEffect(pRNode->obj.flags))
			{
				pChunk->st1[0].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[0].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[1].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[1].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[2].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[2].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[3].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[3].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;

				updateRenderChunkAnimationIntensity(pChunk);
			}

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
				tempLeft = (topLeft.x * w / 2) + (w / 2);
				tempRight = (bottomRight.x * w / 2) + (w / 2);
				tempTop = (topLeft.y * -h / 2) + (h / 2);
				tempBottom = (bottomRight.y * -h / 2) + (h / 2);

				distX = pRNode->motion.x - player->x;
				distY = pRNode->motion.y - player->y;

				distance = DistanceGet(distX, distY);

				if (range == NULL)
				{
					// Set up new visible object
					range = &gameObjectDataParams.visibleObjects[*gameObjectDataParams.numVisibleObjects];
					range->id = pRNode->obj.id;
					range->distance = distance;
					range->left_col = tempLeft;
					range->right_col = tempRight;
					range->top_row = tempTop;
					range->bottom_row = tempBottom;

					*gameObjectDataParams.numVisibleObjects = min(*gameObjectDataParams.numVisibleObjects + 1, MAXOBJECTS);
				}

				// Record boundaries of drawing area
				range->left_col = min(range->left_col, tempLeft);
				range->right_col = max(range->right_col, tempRight);
				range->top_row = min(range->top_row, tempTop);
				range->bottom_row = max(range->bottom_row, tempBottom);
			}
		}

		if (pRNode->obj.id != INVALID_ID 
			&& pRNode->obj.id == GetUserTargetID()
			&& !IsInvisibleEffect(pRNode->obj.flags))
		{
			pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDib, xLat0, xLat1,
				GetDrawingEffect(pRNode->obj.flags));
			if (NULL == pPacket)
				return;
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->flags = pRNode->obj.flags | OF_TRANSLUCENT50;
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->xLat0 = xLat0;
			pChunk->xLat1 = xLat1;
			pChunk->zBias = ZBIAS_TARGETED;
			pChunk->isTargeted = TRUE;

			MatrixMultiply(&pChunk->xForm, &rot, &mat);

			if (IsInvisibleEffect(pRNode->obj.flags))
			{
				pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
			}
			else
			{
				pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
			}

			D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams);

			for (i = 0; i < 4; i++)
			{
				pChunk->xyz[i].x = xyz[i].x;
				pChunk->xyz[i].y = 0;
				pChunk->xyz[i].z = xyz[i].z;

				switch (config.halocolor)
				{
				case 0:
					pChunk->bgra[i].b = 0;
					pChunk->bgra[i].g = 0;
					pChunk->bgra[i].r = min(bgra.r * 2.0f, 255);
					pChunk->bgra[i].a = 255;
					break;

				case 1:
					pChunk->bgra[i].b = min(bgra.r * 2.0f, 255);
					pChunk->bgra[i].g = 0;
					pChunk->bgra[i].r = 0;
					pChunk->bgra[i].a = 255;
					break;

				default:
					pChunk->bgra[i].b = 0;
					pChunk->bgra[i].g = min(bgra.r * 2.0f, 255);
					pChunk->bgra[i].r = 0;
					pChunk->bgra[i].a = 255;
					break;
				}
			}

			pChunk->xyz[0].x -= 32.0f / (float)pDib->shrink;
			pChunk->xyz[0].z += 32.0f / (float)pDib->shrink;
			pChunk->xyz[1].x -= 32.0f / (float)pDib->shrink;
			pChunk->xyz[1].z -= 32.0f / (float)pDib->shrink;
			pChunk->xyz[2].x += 32.0f / (float)pDib->shrink;
			pChunk->xyz[2].z -= 32.0f / (float)pDib->shrink;
			pChunk->xyz[3].x += 32.0f / (float)pDib->shrink;
			pChunk->xyz[3].z += 32.0f / (float)pDib->shrink;

			pChunk->st0[0].s = 1.0f;
			pChunk->st0[0].t = 0.0f;
			pChunk->st0[1].s = 1.0f;
			pChunk->st0[1].t = 1.0f;
			pChunk->st0[2].s = 0.0f;
			pChunk->st0[2].t = 1.0f;
			pChunk->st0[3].s = 0.0f;
			pChunk->st0[3].t = 0.0f;

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}
}

/**
* Drawing of projectiles such as arrows and ranged spells such as fireball/lightening bolt.
* Return the number of projectiles drawn.
*/
int D3DRenderProjectilesDraw(const ObjectsRenderParams& objectsRenderParams)
{
	D3DMATRIX			mat, rot;
	int					angleHeading, anglePitch;
	int					i;
	Projectile* pProjectile;
	list_type			list;
	long				dx, dy, angle;
	PDIB				pDib;
	int projectileCount = 0;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	angleHeading = objectsRenderParams.params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	// base objects
	for (list = objectsRenderParams.room->projectiles; list != NULL; list = list->next)
	{
		BYTE	xLat0, xLat1;

		pProjectile = (Projectile*)list->data;

		dx = pProjectile->motion.x - objectsRenderParams.params->viewer_x;
		dy = pProjectile->motion.y - objectsRenderParams.params->viewer_y;

		angle = (pProjectile->angle - intATan2(-dy, -dx)) & NUMDEGREES_MASK;

		pDib = GetObjectPdib(pProjectile->icon_res, angle, pProjectile->animate.group);

		if (NULL == pDib)
			continue;

		xLat0 = pProjectile->translation;
		xLat1 = 0;

		pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDib, xLat0, xLat1, 0);
		if (NULL == pPacket)
			return projectileCount;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->flags = pProjectile->flags;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = pProjectile->translation;
		pChunk->xLat1 = 0;
		pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = &D3DMaterialObjectChunk;

		angle = pProjectile->angle - (objectsRenderParams.params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, (float)pProjectile->motion.x, (float)pProjectile->motion.z,
			(float)pProjectile->motion.y);
		MatrixMultiply(&pChunk->xForm, &rot, &mat);

		// Projectile pDib `y offset` values are perfectly tuned for the software renderer.
		// To spawn in the correct center location we require 3 times the offset.
		float yOffsetScaler = 3.0f;

		pChunk->xyz[0].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		pChunk->xyz[0].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * yOffsetScaler;

		pChunk->xyz[1].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		pChunk->xyz[1].z = -(float)pDib->yoffset * yOffsetScaler;

		pChunk->xyz[2].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		pChunk->xyz[2].z = -(float)pDib->yoffset * yOffsetScaler;

		pChunk->xyz[3].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		pChunk->xyz[3].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * yOffsetScaler;

		{
			float	oneOverW, oneOverH;

			oneOverW = 1.0f / pDib->width;
			oneOverH = 1.0f / pDib->height;

			pChunk->st0[0].s = 1.0f - oneOverW;
			pChunk->st0[0].t = oneOverH;
			pChunk->st0[1].s = 1.0f - oneOverW;
			pChunk->st0[1].t = 1.0f - oneOverH;
			pChunk->st0[2].s = oneOverW;
			pChunk->st0[2].t = 1.0f - oneOverH;
			pChunk->st0[3].s = oneOverW;
			pChunk->st0[3].t = oneOverH;
		}

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = COLOR_MAX;
			pChunk->bgra[i].g = COLOR_MAX;
			pChunk->bgra[i].r = COLOR_MAX;
			pChunk->bgra[i].a = COLOR_MAX;
		}

		projectileCount++;
	}

	return projectileCount;
}

/**
* Rendering of player's overlay objects such as scimitars, shields, etc.
*/
void D3DRenderPlayerOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams)
{
	// Renders UI elements (like Scimitars, shields etc)
	D3DMATRIX			mat;
	room_contents_node* pRNode;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	PDIB				pDib;
	custom_bgra			bgra;
	AREA				objArea;
	float				screenW, screenH;

	int i, count;
	object_node* obj;
	list_type overlays;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	screenW = (float)(playerViewParams.viewportWidth);
	screenH = (float)(playerViewParams.viewportHeight);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	// Get player's object flags for special drawing effects
	const auto* player = GetPlayerInfo();
	pRNode = GetRoomObjectById(player->id);

	for (i = 0; i < NUM_PLAYER_OVERLAYS; i++)
	{
		BYTE	xLat0, xLat1;

		const PlayerOverlay* pOverlay = &player->poverlays[i];

		if (pOverlay->obj == NULL || pOverlay->hotspot == 0)
			continue;

		obj = pOverlay->obj;

		pDib = GetObjectPdib(obj->icon_res, 0, obj->animate->group);
		if (pDib == NULL)
			continue;

		D3DComputePlayerOverlayArea(pDib, pOverlay->hotspot, &objArea, playerViewParams);

		overlays = *(obj->overlays);

		if (overlays != NULL)
			D3DRenderPlayerOverlayOverlaysDraw(objectsRenderParams, playerViewParams, lightAndTextureParams, 
				overlays, pDib, &objArea, true);

		if (obj->flags & OF_SECONDTRANS)
		{
			xLat0 = 0;
			xLat1 = obj->secondtranslation;
		}
		else if (obj->flags & OF_DOUBLETRANS)
		{
			xLat0 = obj->translation;
			xLat1 = obj->secondtranslation;
		}
		else
		{
			xLat0 = obj->translation;
			xLat1 = 0;
		}

		pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDib, xLat0, xLat1,
			GetDrawingEffect(pRNode->obj.flags));
		if (NULL == pPacket)
			return;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = xLat0;
		pChunk->xLat1 = xLat1;

		if (IsInvisibleEffect(pRNode->obj.flags))
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
		}
		else
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
		}

		pChunk->flags = pRNode->obj.flags;

		MatrixIdentity(&pChunk->xForm);

		if (GetDrawingEffect(pRNode->obj.flags) == OF_BLACK)
		{
			bgra.b = bgra.g = bgra.r = 0;
			bgra.a = 255;
		}
		else
		{
			D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, 
				objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams);
		}

		if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT25 >> 20))
			bgra.a = D3DRENDER_TRANS25;
		if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT50 >> 20))
			bgra.a = D3DRENDER_TRANS50;
		if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT75 >> 20))
			bgra.a = D3DRENDER_TRANS75;
		if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERTRANS >> 20))
			bgra.a = D3DRENDER_TRANS50;
		if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERINVIS >> 20))
			bgra.a = D3DRENDER_TRANS50;

		pChunk->xyz[0].x = pChunk->xyz[1].x = objArea.x;
		pChunk->xyz[0].z = pChunk->xyz[3].z = objArea.y;
		pChunk->xyz[2].x = pChunk->xyz[3].x = pChunk->xyz[0].x + objArea.cx;
		pChunk->xyz[2].z = pChunk->xyz[1].z = pChunk->xyz[0].z + objArea.cy;

		for (count = 0; count < 4; count++)
		{
			if (IsInvisibleEffect(pRNode->obj.flags))
			{
				pChunk->st1[count].s = pChunk->xyz[count].x / gScreenWidth;
				pChunk->st1[count].t = pChunk->xyz[count].z / gScreenHeight;
			}

			pChunk->xyz[count].x = D3DRENDER_SCREEN_TO_CLIP_X(pChunk->xyz[count].x,
				gScreenWidth);
			pChunk->xyz[count].z = D3DRENDER_SCREEN_TO_CLIP_Y(pChunk->xyz[count].z,
				gScreenHeight);
			pChunk->xyz[count].y = PLAYER_OVERLAY_Z;

			pChunk->bgra[count].b = bgra.b;
			pChunk->bgra[count].g = bgra.g;
			pChunk->bgra[count].r = bgra.r;
			pChunk->bgra[count].a = bgra.a;
		}

		{
			float	oneOverW, oneOverH;

			oneOverW = (1.0f / pDib->width);
			oneOverH = (1.0f / pDib->height);

			pChunk->st0[0].s = oneOverW;
			pChunk->st0[0].t = oneOverH;
			pChunk->st0[1].s = oneOverW;
			pChunk->st0[1].t = 1.0f - oneOverH;
			pChunk->st0[2].s = 1.0f - oneOverW;
			pChunk->st0[2].t = 1.0f - oneOverH;
			pChunk->st0[3].s = 1.0f - oneOverW;
			pChunk->st0[3].t = oneOverH;
		}

		updateRenderChunkAnimationIntensity(pChunk);

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		if (overlays != NULL)
			D3DRenderPlayerOverlayOverlaysDraw(objectsRenderParams, playerViewParams, lightAndTextureParams,
				overlays, pDib, &objArea, false);
	}
}

/**
* Rendering of player's overlay overlays as overlays can have overlays too.
*/
void D3DRenderPlayerOverlayOverlaysDraw(
	const ObjectsRenderParams& objectsRenderParams,
	const PlayerViewParams& playerViewParams,
	const LightAndTextureParams& lightAndTextureParams,
	list_type overlays,
	PDIB pDib, 
	AREA* objArea, 
	bool underlays)
{
	int					pass, depth;
	room_contents_node* pRNode;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	PDIB				pDibOv;
	Overlay* pOverlay;
	int					flags, i, zBias;
	float				lastDistance, screenW, screenH;
	BYTE				xLat0, xLat1;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	screenW = (float)(playerViewParams.d3dRect.right - playerViewParams.d3dRect.left) / (float)playerViewParams.viewportWidth;
	screenH = (float)(playerViewParams.d3dRect.bottom - playerViewParams.d3dRect.top) / (float)playerViewParams.viewportHeight;

	// Get player's object flags for special drawing effects
	const auto* player = GetPlayerInfo();
	pRNode = GetRoomObjectById(player->id);

	if (pRNode == NULL)
		flags = 0;
	else
		flags = pRNode->obj.flags;

	pass = 0;

	if (overlays != NULL)
	{
		for (; pass < 3; pass++)
		{
			if (underlays)
				switch (pass)
				{
				case 0: depth = HOTSPOT_UNDERUNDER;
					zBias = ZBIAS_UNDERUNDER;
					break;

				case 1: depth = HOTSPOT_UNDER;
					zBias = ZBIAS_UNDER;
					break;

				case 2: depth = HOTSPOT_UNDEROVER;
					zBias = ZBIAS_UNDEROVER;
					break;
				}
			else
				switch (pass)
				{
				case 0: depth = HOTSPOT_OVERUNDER;
					zBias = ZBIAS_OVERUNDER;
					break;

				case 1: depth = HOTSPOT_OVER;
					zBias = ZBIAS_OVER;
					break;

				case 2: depth = HOTSPOT_OVEROVER;
					zBias = ZBIAS_OVEROVER;
					break;
				}

			pOverlay = (Overlay*)overlays->data;

			pDibOv = GetObjectPdib(pOverlay->icon_res, 0, pOverlay->animate.group);

			if ((NULL == pDib) || (NULL == pDibOv))
				continue;

			if (pOverlay->hotspot)
			{
				POINT	point;
				int		retVal;

				retVal = FindHotspotPdib(pDib, pOverlay->hotspot, &point);

				if (retVal != HOTSPOT_NONE)
				{
					if (retVal != depth)
						continue;

					for (i = 0; i < 4; i++)
					{
						// add hotspot
						xyz[i].x = (float)point.x / pDib->shrink / (float)screenW;
						xyz[i].z = (float)point.y / pDib->shrink / (float)screenH;
						// add overlay offsets
						xyz[i].x += (float)pDibOv->xoffset / pDibOv->shrink / (float)screenW;
						xyz[i].z += (float)pDibOv->yoffset / pDibOv->shrink / (float)screenH;
					}

					xyz[1].z += (float)pDibOv->height / pDibOv->shrink / (float)screenH;
					xyz[2].x += (float)pDibOv->width / pDibOv->shrink / (float)screenW;
					xyz[2].z += (float)pDibOv->height / pDibOv->shrink / (float)screenH;
					xyz[3].x += (float)pDibOv->width / pDibOv->shrink / (float)screenW;
				}
				else
				{
					assert(0);
					// the hotspot wasn't in the base object pdib
					// must be an overlay on an overlay, so find base overlay
				}
			}

			for (i = 0; i < 4; i++)
			{
				xyz[i].x += objArea->x;
				xyz[i].z += objArea->y;
			}

			if (pRNode->obj.flags & OF_SECONDTRANS)
			{
				xLat0 = 0;
				xLat1 = pRNode->obj.secondtranslation;
			}
			else if (pRNode->obj.flags & OF_DOUBLETRANS)
			{
				xLat0 = pOverlay->translation;
				xLat1 = pRNode->obj.secondtranslation;
			}
			else
			{
				xLat0 = pOverlay->translation;
				xLat1 = 0;
			}

			pPacket = D3DRenderPacketFindMatch(objectsRenderParams.renderPool, NULL, pDibOv, xLat0, xLat1,
				GetDrawingEffect(pRNode->obj.flags));
			if (NULL == pPacket)
				return;

			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->flags = pRNode->obj.flags;
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->xLat0 = xLat0;
			pChunk->xLat1 = xLat1;

			if (IsInvisibleEffect(pRNode->obj.flags))
			{
				pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
				pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
			}
			else
			{
				pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
				pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
			}

			MatrixIdentity(&pChunk->xForm);

			lastDistance = 0;

			if (GetDrawingEffect(pRNode->obj.flags) == OF_BLACK)
			{
				bgra.b = bgra.g = bgra.r = 0;
				bgra.a = 255;
			}
			else
			{
				D3DObjectLightingCalc(objectsRenderParams.room, pRNode, &bgra, 0, objectsRenderParams.driverProfile.bFogEnable, lightAndTextureParams);
			}

			if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT25 >> 20))
				bgra.a = D3DRENDER_TRANS25;
			if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT50 >> 20))
				bgra.a = D3DRENDER_TRANS50;
			if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_TRANSLUCENT75 >> 20))
				bgra.a = D3DRENDER_TRANS75;
			if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERTRANS >> 20))
				bgra.a = D3DRENDER_TRANS50;
			if (GetDrawingEffectIndex(pRNode->obj.flags) == (OF_DITHERINVIS >> 20))
				bgra.a = D3DRENDER_TRANS50;

			for (i = 0; i < 4; i++)
			{
				if (IsInvisibleEffect(pRNode->obj.flags))
				{
					pChunk->st1[i].s = xyz[i].x / gScreenWidth;
					pChunk->st1[i].t = xyz[i].z / gScreenHeight;
				}

				pChunk->xyz[i].x = D3DRENDER_SCREEN_TO_CLIP_X(xyz[i].x, gScreenWidth);
				pChunk->xyz[i].z = D3DRENDER_SCREEN_TO_CLIP_Y(xyz[i].z, gScreenHeight);
				pChunk->xyz[i].y = PLAYER_OVERLAY_Z;

				pChunk->bgra[i].b = bgra.b;
				pChunk->bgra[i].g = bgra.g;
				pChunk->bgra[i].r = bgra.r;
				pChunk->bgra[i].a = bgra.a;
			}

			{
				float	oneOverW, oneOverH;

				oneOverW = (1.0f / pDib->width);
				oneOverH = (1.0f / pDib->height);

				pChunk->st0[0].s = oneOverW;
				pChunk->st0[0].t = oneOverH;
				pChunk->st0[1].s = oneOverW;
				pChunk->st0[1].t = 1.0f - oneOverH;
				pChunk->st0[2].s = 1.0f - oneOverW;
				pChunk->st0[2].t = 1.0f - oneOverH;
				pChunk->st0[3].s = 1.0f - oneOverW;
				pChunk->st0[3].t = oneOverH;
			}

			updateRenderChunkAnimationIntensity(pChunk);

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}
}

/**
* Lighting calculations for world objects.
*/
bool D3DObjectLightingCalc(
	room_type* room, 
	room_contents_node* pRNode, 
	custom_bgra* bgra, 
	DWORD flags, 
	bool fogEnabled,
	const LightAndTextureParams& lightAndTextureParams)
{
	int			light, intDistance, numLights;
	d_light* pDLight = NULL;
	float		distX, distY;
	float		lastDistance, distance;
	bool		bFogDisable = false;

	lastDistance = DLIGHT_SCALE(255);

	for (numLights = 0; numLights < lightAndTextureParams.lightCache->numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - lightAndTextureParams.lightCache->dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - lightAndTextureParams.lightCache->dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - lightAndTextureParams.lightCache->dLights[numLights].xyz.z);

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (lightAndTextureParams.lightCache->dLights[numLights].xyzScale.x / 2.0f);

		if (distance < lastDistance)
		{
			lastDistance = distance;
			pDLight = &lightAndTextureParams.lightCache->dLights[numLights];
		}
	}

	for (numLights = 0; numLights < lightAndTextureParams.lightCacheDynamic->numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - lightAndTextureParams.lightCacheDynamic->dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - lightAndTextureParams.lightCacheDynamic->dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - lightAndTextureParams.lightCacheDynamic->dLights[numLights].xyz.z);

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (lightAndTextureParams.lightCacheDynamic->dLights[numLights].xyzScale.x / 2.0f);

		if (distance < lastDistance)
		{
			lastDistance = distance;
			pDLight = &lightAndTextureParams.lightCacheDynamic->dLights[numLights];
		}
	}

	lastDistance = 1.0f - lastDistance;
	lastDistance = max(0, lastDistance);
	lastDistance = COLOR_AMBIENT * lastDistance;

	light = D3DRenderObjectGetLight(room->tree, pRNode);

	if (light <= 127)
		bFogDisable = true;

	const auto* player = GetPlayerInfo();

	distX = pRNode->motion.x - player->x;
	distY = pRNode->motion.y - player->y;

	intDistance = DistanceGet(distX, distY);

	if (fogEnabled && ((flags & D3DRENDER_NOAMBIENT) == 0))
		intDistance = FINENESS;

	if (pRNode->obj.flags & OF_FLASHING)
		light = GetLightPaletteIndex(intDistance, light, FINENESS,
			pRNode->obj.lightAdjust);
	else
		light = GetLightPaletteIndex(intDistance, light, FINENESS,
			0);

	light = light * COLOR_AMBIENT / 64;

	if (pDLight)
	{
		bgra->b = min(COLOR_AMBIENT, light);
		bgra->g = min(COLOR_AMBIENT, light);
		bgra->r = min(COLOR_AMBIENT, light);
		bgra->a = 255;

		bgra->b = min(COLOR_AMBIENT, bgra->b + (lastDistance * pDLight->color.b / COLOR_AMBIENT));
		bgra->g = min(COLOR_AMBIENT, bgra->g + (lastDistance * pDLight->color.g / COLOR_AMBIENT));
		bgra->r = min(COLOR_AMBIENT, bgra->r + (lastDistance * pDLight->color.r / COLOR_AMBIENT));
	}
	else
	{
		bgra->b = min(COLOR_AMBIENT, light + lastDistance);
		bgra->g = min(COLOR_AMBIENT, light + lastDistance);
		bgra->r = min(COLOR_AMBIENT, light + lastDistance);
		bgra->a = 255;
	}

	return bFogDisable;
}

// Given a string and a pointer into the string, return the amount of kerning
// between the pointer and the previous character (if any).
int getKerningAmount(font_3d* pFont, char* str, char* ptr) {
	int kerningAmount = 0;
	for (int i = 0; i < pFont->numKerningPairs; ++i) {
		KERNINGPAIR* pair = &pFont->kerningPairs[i];
		if (ptr > str && pair->wFirst == *(ptr - 1) && pair->wSecond == *ptr) {
			return pair->iKernAmount;
		}
	}
	return 0;
}

/**
* Calculate scaling factors for UI elements (Scimtar/shield etc).
*/
bool D3DComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA * obj_area, const PlayerViewParams& playerViewParams)
{
	// Reference resolution and scaling factors for classic 800x600 resolution.
	const float REFERENCE_WIDTH = 800.0f;
	const float REFERENCE_HEIGHT = 600.0f;
	const float REFERENCE_SCALE_W = 1.75f;
	const float REFERENCE_SCALE_H = 2.25f;

	// Calculate the scaling factors based on the current resolution.
	float scaleFactorWidth = playerViewParams.screenWidth / REFERENCE_WIDTH;
	float scaleFactorHeight = playerViewParams.screenHeight / REFERENCE_HEIGHT;

	// Apply these scaling factors to the original reference scaling factors.
	float scaleW = REFERENCE_SCALE_W * scaleFactorWidth;
	float scaleH = REFERENCE_SCALE_H * scaleFactorHeight;

	float screenW = (float)(playerViewParams.d3dRect.right - playerViewParams.d3dRect.left) / (float)(playerViewParams.viewportWidth * scaleW);
	float screenH = (float)(playerViewParams.d3dRect.bottom - playerViewParams.d3dRect.top) / (float)(playerViewParams.viewportHeight * scaleH);

	if (hotspot < 1 || hotspot > HOTSPOT_PLAYER_MAX)
	{
		debug(("ComputePlayerOverlayArea found hotspot out of range (%d).\n", (int)hotspot));
		return false;
	}

	// Find x position
	switch (hotspot)
	{
	case HOTSPOT_NW:
	case HOTSPOT_W:
	case HOTSPOT_SW:
		obj_area->x = 0;
		break;

	case HOTSPOT_SE:
	case HOTSPOT_E:
	case HOTSPOT_NE:
		obj_area->x = playerViewParams.screenWidth - DibWidth(pdib) / (float)screenW;
		break;

	case HOTSPOT_N:
	case HOTSPOT_S:
	case HOTSPOT_CENTER:
		obj_area->x = (playerViewParams.screenWidth - DibWidth(pdib) / (float)screenW) / 2;
		break;
	}

	// Find y position
	switch (hotspot)
	{
	case HOTSPOT_NW:
	case HOTSPOT_N:
	case HOTSPOT_NE:
		obj_area->y = 0;
		break;

	case HOTSPOT_SW:
	case HOTSPOT_S:
	case HOTSPOT_SE:
		obj_area->y = playerViewParams.screenHeight - DibHeight(pdib) / (float)screenH;
		break;

	case HOTSPOT_W:
	case HOTSPOT_E:
	case HOTSPOT_CENTER:
		obj_area->y = (playerViewParams.screenHeight - DibHeight(pdib) / (float)screenH) / 2;
		break;
	}

	obj_area->x += DibXOffset(pdib) / (float)screenW;
	obj_area->y += DibYOffset(pdib) / (float)screenH;
	obj_area->cx = DibWidth(pdib) / (float)screenW;
	obj_area->cy = DibHeight(pdib) / (float)screenH;
	return true;
}

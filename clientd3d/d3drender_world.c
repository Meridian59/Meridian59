// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Constants

static const auto alpha_test_threshold = 128; // Threshold for alpha testing to determine transparency

// Interfaces

void D3DRenderWorldDraw(const WorldRenderParams& worldRenderParams, bool transparent_pass);
void D3DRenderWorldLighting(
	const WorldRenderParams& worldRenderParams, 
	const LightAndTextureParams& lightAndTextureParams);

void D3DRenderLMapsPostDraw(
	const WorldRenderParams& worldRenderParams, 
	const LightAndTextureParams& lightAndTextureParams, 
	BSPnode* tree, 
	bool transparent_pass);

void D3DRenderLMapsDynamicPostDraw(
	const WorldRenderParams& worldRenderParams,
	const LightAndTextureParams& lightAndTextureParams,
	BSPnode* tree,
	bool transparent_pass);

void D3DRenderPacketWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side, bool dynamic);
void D3DRenderPacketFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, bool dynamic);
void D3DRenderPacketCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, bool dynamic);

void D3DRenderPacketWallMaskAdd(WallData* pWall, d3d_render_pool_new* pPool, LPDIRECT3DTEXTURE9 noLookThrough, 
	unsigned int type, int side, bool dynamic);
void D3DRenderFloorMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, LPDIRECT3DTEXTURE9 noLookThrough, bool bDynamic);
void D3DRenderCeilingMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, room_type* current_room, 
	LPDIRECT3DTEXTURE9 noLookThrough, LPDIRECT3DTEXTURE9 lightOrange, bool bDynamic);

void D3DRenderLMapPostFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache, bool bDynamic);
void D3DRenderLMapPostCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache, bool bDynamic);
void D3DRenderLMapPostWallAdd(WallData* pWall, d3d_render_pool_new* pPool, 
	unsigned int type, int side, d_light_cache* pDLightCache, bool bDynamic);

void D3DRenderFloorExtract(BSPnode* pNode, PDIB pDib, custom_xyz* pXYZ, custom_st* pST, custom_bgra* pBGRA);
void D3DRenderCeilingExtract(BSPnode* pNode, PDIB pDib, custom_xyz* pXYZ, custom_st* pST, custom_bgra* pBGRA);
int D3DRenderWallExtract(WallData* pWall, PDIB pDib, unsigned int* flags, 
	custom_xyz* pXYZ, custom_st* pST, custom_bgra* pBGRA, unsigned int type, int side);

// Implementations

/**
* The main entry point for rendering the 3d game world.
* Returns the total time taken to render the world.
*/
long D3DRenderWorld(
	const WorldRenderParams& worldRenderParams, 
	const WorldPropertyParams& worldPropertyParams, 
	const LightAndTextureParams& lightAndTextureParams)
{
	// We implement a separate transparent pass to ensure that transparent objects
	// are rendered correctly. This approach avoids issues with depth sorting by first rendering
	// all opaque objects and then rendering all transparent objects in a separate pass.
	// This technique is also applied to draw_world, draw_objects, and light maps.

	SetZBias(gpD3DDevice, ZBIAS_WORLD);
	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, worldRenderParams.vertexDeclaration);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	auto timeWorld = timeGetTime();

	// Adjusted Alpha Testing and Blending
	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, alpha_test_threshold, D3DCMP_GREATEREQUAL);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, alpha_test_threshold);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	// Set up texture filtering
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// Ensure texture addressing mode is clamp
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Enable depth testing
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	auto& cacheSystem = worldRenderParams.cacheSystemParams;
	auto& pools = worldRenderParams.poolParams;

	D3DRenderPoolReset(pools.worldPool, &D3DMaterialWorldPool);
	D3DCacheSystemReset(cacheSystem.worldCacheSystem);
	D3DRenderWorldDraw(worldRenderParams, false); // Non-transparent objects pass
	D3DRenderWorldDraw(worldRenderParams, true);  // Transparent objects pass
	D3DCacheFill(cacheSystem.worldCacheSystem, pools.worldPool, 1);

	// Render the wireframe pass to cover cracks in geometry
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	setWireframeMode(TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	D3DCacheFlush(cacheSystem.worldCacheSystemStatic, pools.worldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
	D3DCacheFlush(cacheSystem.worldCacheSystem, pools.worldPool, 1, D3DPT_TRIANGLESTRIP);
	setWireframeMode(FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ANTIALIASEDLINEENABLE, FALSE);

	// Finally, draw the solid world
	D3DCacheFlush(cacheSystem.worldCacheSystemStatic, pools.worldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
	D3DCacheFlush(cacheSystem.worldCacheSystem, pools.worldPool, 1, D3DPT_TRIANGLESTRIP);

	timeWorld = timeGetTime() - timeWorld;

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &worldRenderParams.view);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	D3DRenderWorldLighting(worldRenderParams, lightAndTextureParams);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	SetZBias(gpD3DDevice, 1);

	return timeWorld;
}

/**
* Applying static and dynamic lighting to the 3d world
*/
void D3DRenderWorldLighting(const WorldRenderParams& worldRenderParams, const LightAndTextureParams& lightAndTextureParams)
{
	// draw post lightmaps
	if (config.bDynamicLighting)
	{
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MAGFILTER, worldRenderParams.driverProfile.magFilter);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MINFILTER, worldRenderParams.driverProfile.minFilter);

		SetZBias(gpD3DDevice, ZBIAS_WORLD);
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, worldRenderParams.vertexDeclarationSecondary);

		auto& cacheSystem = worldRenderParams.cacheSystemParams;
		auto& pools = worldRenderParams.poolParams;
		auto& room = worldRenderParams.room;
		auto& params = worldRenderParams.params;

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, alpha_test_threshold, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_ONE, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
		D3DCacheFlush(cacheSystem.lMapCacheSystemStatic, pools.lMapPoolStatic, 2, D3DPT_TRIANGLESTRIP);

		D3DRenderPoolReset(pools.lMapPool, &D3DMaterialLMapDynamicPool);

		D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, room->tree, false);
		D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, room->tree, false);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE); // Disable depth writing
		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, alpha_test_threshold, D3DCMP_GREATEREQUAL);

		D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, room->tree, true);
		D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, room->tree, true);

		D3DCacheFill(cacheSystem.lMapCacheSystem, pools.lMapPool, 2);
		D3DCacheFlush(cacheSystem.lMapCacheSystem, pools.lMapPool, 2, D3DPT_TRIANGLESTRIP);

		// Restore states for subsequent rendering
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE); // Restore depth writing
		if (isFogEnabled())
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE); // Restore fog state
	}
}

/**
* Drawing of the world composed of floors, ceilings and walls
*/
void D3DRenderWorldDraw(const WorldRenderParams& worldRenderParams, bool transparent_pass)
{
	int count;
	BSPnode* pNode = NULL;
	WallData* pWall;
	Bool bDynamic;

	auto& cacheSystem = worldRenderParams.cacheSystemParams;
	auto& pools = worldRenderParams.poolParams;
	auto& room = worldRenderParams.room;

	for (count = 0; count < room->num_nodes; count++)
	{
		pNode = &room->nodes[count];

		switch (pNode->type)
		{
		case BSPinternaltype:
			for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
			{
				// Determine if the wall is transparent
				bool isTransparent = (pWall->pos_sidedef && pWall->pos_sidedef->flags & WF_TRANSPARENT) ||
					(pWall->neg_sidedef && pWall->neg_sidedef->flags & WF_TRANSPARENT);

				if (!ShouldRenderInCurrentPass(transparent_pass, isTransparent))
					continue;

				int	flags, wallFlags;

				flags = 0;
				wallFlags = 0;
				bDynamic = FALSE;

				if (pWall->pos_sidedef)
				{
					if (pWall->pos_sidedef->flags & WF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->neg_sidedef)
				{
					if (pWall->neg_sidedef->flags & WF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->pos_sector)
				{
					if (pWall->pos_sector->flags & SF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->neg_sector)
				{
					if (pWall->neg_sector->flags & SF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (FALSE == bDynamic)
					continue;

				if (pWall->pos_sidedef)
				{
					wallFlags |= pWall->pos_sidedef->flags;

					if (pWall->pos_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->pos_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->pos_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				if (pWall->neg_sidedef)
				{
					wallFlags |= pWall->neg_sidedef->flags;

					if (pWall->neg_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->neg_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->neg_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				pWall->separator.a = pNode->u.internal.separator.a;
				pWall->separator.b = pNode->u.internal.separator.b;
				pWall->separator.c = pNode->u.internal.separator.c;

				if ((flags & D3DRENDER_WALL_NORMAL) && (((short)pWall->z2 != (short)pWall->z1)
					|| ((short)pWall->zz2 != (short)pWall->zz1)))
				{
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_NORMAL, 1, true);
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_NORMAL, -1, true);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && (((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0)))
				{
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_BELOW, 1, true);
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_BELOW, -1, true);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && (((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2)))
				{
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_ABOVE, 1, true);
					D3DRenderPacketWallAdd(pWall, pools.worldPool, D3DRENDER_WALL_ABOVE, -1, true);
				}
			}

			break;

		case BSPleaftype:
			if (pNode->u.leaf.sector->flags & SF_HAS_ANIMATED)
			{
				D3DRenderPacketFloorAdd(pNode, pools.worldPool, true);
				D3DRenderPacketCeilingAdd(pNode, pools.worldPool, true);
			}
			break;

		default:
			break;
		}
	}
}

/*
* Rendering of post lightmaps for the 3d world.
*/
void D3DRenderLMapsPostDraw(
	const WorldRenderParams& worldRenderParams, 
	const LightAndTextureParams& lightAndTextureParams, 
	BSPnode* tree, 
	bool transparent_pass)
{
	long side;
	float a, b;

	if (!tree)
		return;

	auto& pools = worldRenderParams.poolParams;
	auto& cache = worldRenderParams.cacheSystemParams;
	auto& params = worldRenderParams.params;

	switch (tree->type)
	{
	case BSPleaftype:
		if (tree->u.leaf.sector->flags & SF_HAS_ANIMATED)
		{
			if (transparent_pass)
			{
				D3DRenderLMapPostFloorAdd(tree, pools.lMapPool, lightAndTextureParams.lightCache, TRUE);
				D3DRenderLMapPostCeilingAdd(tree, pools.lMapPool, lightAndTextureParams.lightCache, TRUE);
			}
		}
		return;
	case BSPinternaltype:
		side = (a = tree->u.internal.separator.a) * params->viewer_x +
			(b = tree->u.internal.separator.b) * params->viewer_y +
			tree->u.internal.separator.c;

		if (side < 0)
		{
			a = -a;
			b = -b;
		}

		/* first, traverse closer side */
		if (side > 0)
			D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, tree->u.internal.pos_side, transparent_pass);
		else
			D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, tree->u.internal.neg_side, transparent_pass);

		/* then do walls on the separator */
		if (side != 0)
		{
			WallData* pWall;
			int			flags, wallFlags;

			for (pWall = tree->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
			{
				Bool	bDynamic = FALSE;

				// Determine if the wall is transparent
				bool isTransparent = (pWall->pos_sidedef && pWall->pos_sidedef->flags &
					WF_TRANSPARENT) || (pWall->neg_sidedef && pWall->neg_sidedef->flags &
						WF_TRANSPARENT);

				if (!ShouldRenderInCurrentPass(transparent_pass, isTransparent))
					continue;

				flags = 0;
				wallFlags = 0;

				if (pWall->pos_sidedef)
				{
					if (pWall->pos_sidedef->flags & WF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->neg_sidedef)
				{
					if (pWall->neg_sidedef->flags & WF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->pos_sector)
				{
					if (pWall->pos_sector->flags & SF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (pWall->neg_sector)
				{
					if (pWall->neg_sector->flags & SF_HAS_ANIMATED)
						bDynamic = TRUE;
				}

				if (FALSE == bDynamic)
					continue;

				if (pWall->pos_sidedef)
				{
					wallFlags |= pWall->pos_sidedef->flags;

					if (pWall->pos_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->pos_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->pos_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				if (pWall->neg_sidedef)
				{
					wallFlags |= pWall->neg_sidedef->flags;

					if (pWall->neg_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->neg_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->neg_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				pWall->separator.a = tree->u.internal.separator.a;
				pWall->separator.b = tree->u.internal.separator.b;
				pWall->separator.c = tree->u.internal.separator.c;

				if ((flags & D3DRENDER_WALL_NORMAL) && ((short)pWall->z2 != (short)pWall->z1)
					|| (pWall->zz2 != pWall->zz1))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_NORMAL, side, 
						lightAndTextureParams.lightCache, TRUE);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_BELOW, side, 
						lightAndTextureParams.lightCache, TRUE);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_ABOVE, side, 
						lightAndTextureParams.lightCache, TRUE);
				}
			}
		}

		/* lastly, traverse farther side */
		if (side > 0)
			D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, tree->u.internal.neg_side, transparent_pass);
		else
			D3DRenderLMapsPostDraw(worldRenderParams, lightAndTextureParams, tree->u.internal.pos_side, transparent_pass);

		return;

	default:
		debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

/*
* Rendering of post lightmaps for the dynamic 3d world.
*/
void D3DRenderLMapsDynamicPostDraw(
	const WorldRenderParams& worldRenderParams,
	const LightAndTextureParams& lightAndTextureParams, 
	BSPnode* tree, 
	bool transparent_pass)
{
	long side;
	float a, b;

	if (!tree)
		return;

	auto& pools = worldRenderParams.poolParams;
	auto& cache = worldRenderParams.cacheSystemParams;
	auto& params = worldRenderParams.params;

	switch (tree->type)
	{
	case BSPleaftype:
		D3DRenderLMapPostFloorAdd(tree, pools.lMapPool, lightAndTextureParams.lightCacheDynamic, TRUE);
		D3DRenderLMapPostCeilingAdd(tree, pools.lMapPool, lightAndTextureParams.lightCacheDynamic, TRUE);

		return;

	case BSPinternaltype:
		side = (a = tree->u.internal.separator.a) * params->viewer_x +
			(b = tree->u.internal.separator.b) * params->viewer_y +
			tree->u.internal.separator.c;

		if (side < 0)
		{
			a = -a;
			b = -b;
		}

		/* first, traverse closer side */
		if (side > 0)
			D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, 
				tree->u.internal.pos_side, transparent_pass);
		else
			D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, 
				tree->u.internal.neg_side, transparent_pass);

		/* then do walls on the separator */
		if (side != 0)
		{
			WallData* pWall;
			int			flags, wallFlags;

			for (pWall = tree->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
			{
				// Determine if the wall is transparent
				bool isTransparent = (pWall->pos_sidedef && pWall->pos_sidedef->flags & WF_TRANSPARENT) ||
					(pWall->neg_sidedef && pWall->neg_sidedef->flags & WF_TRANSPARENT);

				if (!ShouldRenderInCurrentPass(transparent_pass, isTransparent))
					continue;

				flags = 0;
				wallFlags = 0;

				if (pWall->pos_sidedef)
				{
					wallFlags |= pWall->pos_sidedef->flags;

					if (pWall->pos_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->pos_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->pos_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				if (pWall->neg_sidedef)
				{
					wallFlags |= pWall->neg_sidedef->flags;

					if (pWall->neg_sidedef->normal_bmap)
						flags |= D3DRENDER_WALL_NORMAL;

					if (pWall->neg_sidedef->below_bmap)
						flags |= D3DRENDER_WALL_BELOW;

					if (pWall->neg_sidedef->above_bmap)
						flags |= D3DRENDER_WALL_ABOVE;
				}

				pWall->separator.a = tree->u.internal.separator.a;
				pWall->separator.b = tree->u.internal.separator.b;
				pWall->separator.c = tree->u.internal.separator.c;

				if ((flags & D3DRENDER_WALL_NORMAL) && ((short)pWall->z2 != (short)pWall->z1)
					|| ((short)pWall->zz2 != (short)pWall->zz1))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_NORMAL, side, lightAndTextureParams.lightCacheDynamic, TRUE);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_BELOW, side, lightAndTextureParams.lightCacheDynamic, TRUE);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2))
				{
					D3DRenderLMapPostWallAdd(pWall, pools.lMapPool, D3DRENDER_WALL_ABOVE, side, lightAndTextureParams.lightCacheDynamic, TRUE);
				}
			}
		}

		/* lastly, traverse farther side */
		if (side > 0)
			D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, 
				tree->u.internal.neg_side, transparent_pass);
		else
			D3DRenderLMapsDynamicPostDraw(worldRenderParams, lightAndTextureParams, 
				tree->u.internal.pos_side, transparent_pass);

		return;

	default:
		debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

/*
* Add a floor to the render pool
*/
void D3DRenderPacketFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_st	st[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	PDIB		pDib;
	int			vertex;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	if (pSector->floor)
	{
		pDib = pSector->floor;
	}
	else
		return;

	D3DRenderFloorExtract(pNode, pDib, xyz, st, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->side = 0;

	if (pSector->light <= 127)
		pChunk->flags |= D3DRENDER_NOAMBIENT;

	if (pSector->sloped_floor)
	{
		pChunk->plane.a = pSector->sloped_floor->plane.a;
		pChunk->plane.b = pSector->sloped_floor->plane.b;
		pChunk->plane.c = pSector->sloped_floor->plane.c;
	}
	else
	{
		pChunk->plane.a = FINENESS;
		pChunk->plane.b = FINENESS;
		pChunk->plane.c = FINENESS;
	}

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	if (bDynamic)
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
	else
		pChunk->pMaterialFctn = &D3DMaterialWorldStaticChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->st0[vertex].s = st[vertex].s;
		pChunk->st0[vertex].t = st[vertex].t;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	{
		u_int	index;
		int	first, last;

		first = 1;
		last = pChunk->numVertices - 1;

		pChunk->indices[0] = 0;
		pChunk->indices[1] = last--;
		pChunk->indices[2] = first++;

		for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
		{
			pChunk->indices[index] = last;
			pChunk->indices[index + 1] = first;
		}
	}
}

/*
* Add a ceiling to the render pool
*/
void D3DRenderPacketCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_st	st[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	PDIB		pDib;
	int			vertex;
	int			left, top;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	left = top = 0;

	if (pSector->ceiling)
	{
		pDib = pSector->ceiling;
	}
	else
		return;

	D3DRenderCeilingExtract(pNode, pDib, xyz, st, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->side = 0;

	if (pSector->light <= 127)
		pChunk->flags |= D3DRENDER_NOAMBIENT;

	if (pSector->sloped_ceiling)
	{
		pChunk->plane.a = pSector->sloped_ceiling->plane.a;
		pChunk->plane.b = pSector->sloped_ceiling->plane.b;
		pChunk->plane.c = pSector->sloped_ceiling->plane.c;
	}
	else
	{
		pChunk->plane.a = FINENESS;
		pChunk->plane.b = FINENESS;
		pChunk->plane.c = FINENESS;
	}

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	if (bDynamic)
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
	else
		pChunk->pMaterialFctn = &D3DMaterialWorldStaticChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->st0[vertex].s = st[vertex].s;
		pChunk->st0[vertex].t = st[vertex].t;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	{
		u_int	index;
		int	first, last;

		first = 1;
		last = pChunk->numVertices - 1;

		pChunk->indices[0] = 0;
		pChunk->indices[1] = first++;
		pChunk->indices[2] = last--;

		for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
		{
			pChunk->indices[index] = first;
			pChunk->indices[index + 1] = last;
		}
	}
}

/*
* Add a wall to the render pool
*/
void D3DRenderPacketWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side, bool bDynamic)
{
	Sidedef* pSideDef;
	custom_xyz		xyz[4];
	custom_st		st[4];
	custom_bgra		bgra[MAX_NPTS];
	unsigned int	flags = 0;
	PDIB			pDib;
	int				vertex;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	// pos and neg sidedefs have their x and y coords reversed
	if (side > 0)
	{
		pSideDef = pWall->pos_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pSideDef->normal_bmap)
				pDib = pSideDef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pSideDef->below_bmap)
				pDib = pSideDef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pSideDef->above_bmap)
				pDib = pSideDef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}
	else if (side < 0)
	{
		pSideDef = pWall->neg_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pSideDef->normal_bmap)
				pDib = pSideDef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pSideDef->below_bmap)
				pDib = pSideDef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pSideDef->above_bmap)
				pDib = pSideDef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}

	if (NULL == pDib)
		return;

	D3DRenderWallExtract(pWall, pDib, &flags, xyz, st, bgra, type, side);

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pChunk->numIndices = 4;
	pChunk->numVertices = 4;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->flags = flags;
	pChunk->pSideDef = pSideDef;
	pChunk->pSectorPos = pWall->pos_sector;
	pChunk->pSectorNeg = pWall->neg_sector;
	pChunk->side = side;
	pChunk->plane.a = pWall->separator.a;
	pChunk->plane.b = pWall->separator.b;
	pChunk->plane.c = pWall->separator.c;

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	if (bDynamic)
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
	else
		pChunk->pMaterialFctn = &D3DMaterialWorldStaticChunk;

	for (vertex = 0; vertex < 4; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->st0[vertex].s = st[vertex].s;
		pChunk->st0[vertex].t = st[vertex].t;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	pChunk->indices[0] = 1;
	pChunk->indices[1] = 2;
	pChunk->indices[2] = 0;
	pChunk->indices[3] = 3;
}

/*
* Add a wall mask to the render pool
*/
void D3DRenderPacketWallMaskAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side, 
	LPDIRECT3DTEXTURE9 noLookThrough, bool bDynamic)
{
	Sidedef*		pSideDef;
	custom_xyz		xyz[4];
	custom_st		st[4];
	custom_bgra		bgra[MAX_NPTS];
	unsigned int	flags;
	PDIB			pDib;
	int				vertex;
	bool			bNoVTile = false;
	bool			bNoLookThrough = false;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	// pos and neg sidedefs have their x and y coords reversed
	if (side > 0)
	{
		pSideDef = pWall->pos_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pWall->pos_sidedef->normal_bmap)
				pDib = pWall->pos_sidedef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pWall->pos_sidedef->below_bmap)
				pDib = pWall->pos_sidedef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pWall->pos_sidedef->above_bmap)
				pDib = pWall->pos_sidedef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}
	else if (side < 0)
	{
		pSideDef = pWall->neg_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pWall->neg_sidedef->normal_bmap)
				pDib = pWall->neg_sidedef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pWall->neg_sidedef->below_bmap)
				pDib = pWall->neg_sidedef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pWall->neg_sidedef->above_bmap)
				pDib = pWall->neg_sidedef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}

	if (pWall->pos_sidedef)
	{
		if (pWall->pos_sidedef->flags & WF_NO_VTILE)
			bNoVTile = true;
		if (pWall->pos_sidedef->flags & WF_NOLOOKTHROUGH)
			bNoLookThrough = true;
	}

	if (pWall->neg_sidedef)
	{
		if (pWall->neg_sidedef->flags & WF_NO_VTILE)
			bNoVTile = true;
		if (pWall->neg_sidedef->flags & WF_NOLOOKTHROUGH)
			bNoLookThrough = true;
	}

	if (NULL == pDib)
		return;

	if ((pSideDef->flags & WF_NOLOOKTHROUGH) == 0)
	{
		if (!bNoLookThrough || !bNoVTile)
			return;
	}

	D3DRenderWallExtract(pWall, pDib, &flags, xyz, st, bgra, type, side);

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	flags = 0;
	pChunk->numIndices = 4;
	pChunk->numVertices = 4;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->flags = flags;
	pChunk->pSideDef = pSideDef;
	pChunk->pSectorPos = pWall->pos_sector;
	pChunk->pSectorNeg = pWall->neg_sector;
	pChunk->side = side;
	pChunk->zBias = ZBIAS_MASK;

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < 4; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->st0[vertex].s = st[vertex].s;
		pChunk->st0[vertex].t = st[vertex].t;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	pChunk->indices[0] = 1;
	pChunk->indices[1] = 2;
	pChunk->indices[2] = 0;
	pChunk->indices[3] = 3;

	if (bNoVTile)
	{
		if (side > 0)
		{
			pSideDef = pWall->pos_sidedef;

			xyz[0].x = pWall->x0;
			xyz[3].x = pWall->x1;
			xyz[1].x = pWall->x0;
			xyz[2].x = pWall->x1;

			xyz[0].y = pWall->y0;
			xyz[3].y = pWall->y1;
			xyz[1].y = pWall->y0;
			xyz[2].y = pWall->y1;

			xyz[1].z = xyz[0].z;
			xyz[2].z = xyz[3].z;

			switch (type)
			{
			case D3DRENDER_WALL_NORMAL:
			{
				xyz[0].z = pWall->z2;
				xyz[3].z = pWall->zz2;
			}
			break;

			case D3DRENDER_WALL_BELOW:
			{
				xyz[0].z = pWall->z1;
				xyz[3].z = pWall->zz1;
			}
			break;

			case D3DRENDER_WALL_ABOVE:
			{
				xyz[0].z = pWall->z3;
				xyz[3].z = pWall->zz3;
			}
			break;

			default:
				break;
			}
		}
		else if (side < 0)
		{
			pSideDef = pWall->neg_sidedef;

			xyz[0].x = pWall->x1;
			xyz[3].x = pWall->x0;
			xyz[1].x = pWall->x1;
			xyz[2].x = pWall->x0;

			xyz[0].y = pWall->y1;
			xyz[3].y = pWall->y0;
			xyz[1].y = pWall->y1;
			xyz[2].y = pWall->y0;

			xyz[1].z = xyz[0].z;
			xyz[2].z = xyz[3].z;

			switch (type)
			{
			case D3DRENDER_WALL_NORMAL:
			{
				xyz[0].z = pWall->zz2;
				xyz[3].z = pWall->z2;
			}
			break;

			case D3DRENDER_WALL_BELOW:
			{
				xyz[0].z = pWall->zz1;
				xyz[3].z = pWall->z1;
			}
			break;

			case D3DRENDER_WALL_ABOVE:
			{
				xyz[0].z = pWall->zz3;
				xyz[3].z = pWall->z3;
			}
			break;

			default:
				break;
			}
		}
		else
			assert(0);

		pPacket = D3DRenderPacketFindMatch(pPool, noLookThrough, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->flags = flags;
		pChunk->pSideDef = pSideDef;
		pChunk->pSectorPos = pWall->pos_sector;
		pChunk->pSectorNeg = pWall->neg_sector;
		pChunk->side = side;
		pChunk->zBias = ZBIAS_WORLD + 1;

		if (bNoVTile)
			pChunk->flags |= D3DRENDER_NOCULL;

		pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

		pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

		for (vertex = 0; vertex < 4; vertex++)
		{
			pChunk->xyz[vertex].x = xyz[vertex].x;
			pChunk->xyz[vertex].y = xyz[vertex].y;
			pChunk->xyz[vertex].z = xyz[vertex].z;

			pChunk->st0[vertex].s = st[vertex].s;
			pChunk->st0[vertex].t = st[vertex].t;

			pChunk->bgra[vertex].b = bgra[vertex].b;
			pChunk->bgra[vertex].g = bgra[vertex].g;
			pChunk->bgra[vertex].r = bgra[vertex].r;
			pChunk->bgra[vertex].a = bgra[vertex].a;
		}

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}
}

/*
* Add a floor mask to the render pool
*/
void D3DRenderFloorMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, LPDIRECT3DTEXTURE9 noLookThrough, bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	int			vertex;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	D3DRenderFloorExtract(pNode, NULL, xyz, NULL, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, noLookThrough, NULL, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->zBias = ZBIAS_MASK;

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	{
		u_int	index;
		int	first, last;

		first = 1;
		last = pChunk->numVertices - 1;

		pChunk->indices[0] = 0;
		pChunk->indices[1] = last--;
		pChunk->indices[2] = first++;

		for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
		{
			pChunk->indices[index] = last;
			pChunk->indices[index + 1] = first;
		}
	}
}

/*
* Add a ceiling mask to the render pool
*/
void D3DRenderCeilingMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, room_type* current_room, 
	LPDIRECT3DTEXTURE9 noLookThrough, LPDIRECT3DTEXTURE9 lightOrange, bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	int			vertex;
	int			left, top;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	left = top = 0;

	D3DRenderCeilingExtract(pNode, NULL, xyz, NULL, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, noLookThrough, NULL, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->zBias = ZBIAS_MASK;

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->bgra[vertex].b = bgra[vertex].b;
		pChunk->bgra[vertex].g = bgra[vertex].g;
		pChunk->bgra[vertex].r = bgra[vertex].r;
		pChunk->bgra[vertex].a = bgra[vertex].a;
	}

	{
		u_int	index;
		int	first, last;

		first = 1;
		last = pChunk->numVertices - 1;

		pChunk->indices[0] = 0;
		pChunk->indices[1] = first++;
		pChunk->indices[2] = last--;

		for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
		{
			pChunk->indices[index] = first;
			pChunk->indices[index + 1] = last;
		}
	}

	if ((pSector->sloped_ceiling == NULL) && (pSector->ceiling_height !=
		current_room->sectors[0].ceiling_height))
	{
		int	vertex, i;

		pPacket = D3DRenderPacketFindMatch(pPool, lightOrange, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

		for (i = 0, vertex = 0; i < pNode->u.leaf.poly.npts; i++)
		{
			if (vertex >= MAX_NPTS)
			{
				pChunk = D3DRenderChunkNew(pPacket);
				vertex = 0;
				pChunk->numVertices = 20;
				pChunk->numIndices = pChunk->numVertices;
				pChunk->numPrimitives = pChunk->numVertices - 2;
				pChunk->pSector = pSector;
				pChunk->zBias = 0;
				pChunk->flags = D3DRENDER_NOCULL;

				pChunk->pMaterialFctn = &D3DMaterialMaskChunk;
			}

			pChunk->xyz[vertex].x = xyz[i].x;
			pChunk->xyz[vertex].y = xyz[i].y;
			pChunk->xyz[vertex].z = xyz[i].z;

			pChunk->bgra[vertex].b = bgra[i].b;
			pChunk->bgra[vertex].g = bgra[i].g;
			pChunk->bgra[vertex].r = bgra[i].r;
			pChunk->bgra[vertex].a = bgra[i].a;

			pChunk->indices[vertex] = vertex;

			vertex++;

			pChunk->xyz[vertex].x = xyz[i].x;
			pChunk->xyz[vertex].y = xyz[i].y;
			pChunk->xyz[vertex].z = 65535;

			pChunk->bgra[vertex].b = bgra[i].b;
			pChunk->bgra[vertex].g = bgra[i].g;
			pChunk->bgra[vertex].r = bgra[i].r;
			pChunk->bgra[vertex].a = bgra[i].a;

			pChunk->indices[vertex] = vertex;

			vertex++;
		}

		pChunk->numVertices = vertex - 1;
		pChunk->numIndices = pChunk->numVertices;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->pSector = pSector;
		pChunk->zBias = 0;
		pChunk->flags = D3DRENDER_NOCULL;

		pChunk->pMaterialFctn = &D3DMaterialMaskChunk;
	}
}

/*
* Add light map for floor to the render pool.
*/
void D3DRenderLMapPostFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache,
	bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_st	st[MAX_NPTS];
	custom_st	stBase[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	PDIB		pDib;
	int			count;
	int			numLights;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	if (pSector->floor)
	{
		pDib = pSector->floor;
	}
	else
		return;

	D3DRenderFloorExtract(pNode, pDib, xyz, stBase, bgra);

	// dynamic lightmaps
	for (numLights = 0; numLights < pDLightCache->numLights; numLights++)
	{
		custom_xyz	vector;
		float		distance, lightRange;
		int			unlit;
		float		falloff, invXScale, invYScale, invZScale,
			invXScaleHalf, invYScaleHalf, invZScaleHalf;

		invXScale = pDLightCache->dLights[numLights].invXYZScale.x;
		invYScale = pDLightCache->dLights[numLights].invXYZScale.y;
		invZScale = pDLightCache->dLights[numLights].invXYZScale.z;

		invXScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.x;
		invYScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.y;
		invZScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.z;

		unlit = 0;
		lightRange = (pDLightCache->dLights[numLights].xyzScale.x / 1.5f) *
			(pDLightCache->dLights[numLights].xyzScale.x / 1.5f);// * 2.0f;

		// if dlight is too far away, skip it
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			vector.x = xyz[count].x - pDLightCache->dLights[numLights].xyz.x;
			vector.y = xyz[count].y - pDLightCache->dLights[numLights].xyz.y;

			distance = ((vector.x * vector.x) + (vector.y * vector.y));


			if (distance > lightRange)
				unlit++;
		}

		if (unlit < pNode->u.leaf.poly.npts)
		{
			custom_xyz	lightVec, normal;
			float		cosAngle;

			lightVec.x = pDLightCache->dLights[numLights].xyz.x - xyz[0].x;
			lightVec.y = pDLightCache->dLights[numLights].xyz.y - xyz[0].y;
			lightVec.z = pDLightCache->dLights[numLights].xyz.z - xyz[0].z;

			normal.x = 0;
			normal.y = 0;
			normal.z = 1.0f;

			cosAngle = lightVec.x * normal.x +
				lightVec.y * normal.y +
				lightVec.z * normal.z;

			if (cosAngle <= 0)
				continue;

			pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->numVertices = pNode->u.leaf.poly.npts;
			pChunk->numIndices = pChunk->numVertices;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->pSector = pSector;
			pPacket->pMaterialFctn = &D3DMaterialLMapDynamicPacket;
			pChunk->side = 0;

			if (pSector->light <= 127)
				pChunk->flags |= D3DRENDER_NOAMBIENT;

			if (bDynamic)
				pChunk->pMaterialFctn = &D3DMaterialLMapDynamicChunk;
			else
				pChunk->pMaterialFctn = &D3DMaterialLMapStaticChunk;

			for (count = 0; count < pNode->u.leaf.poly.npts; count++)
			{
				falloff = (xyz[count].z - pDLightCache->dLights[numLights].xyz.z) *
					invZScaleHalf;

				if (falloff < 0)
					falloff = -falloff;

				falloff = min(1.0f, falloff);
				falloff = 1.0f - falloff;

				bgra[count].b = falloff * pDLightCache->dLights[numLights].color.b;
				bgra[count].g = falloff * pDLightCache->dLights[numLights].color.g;
				bgra[count].r = falloff * pDLightCache->dLights[numLights].color.r;
				bgra[count].a = falloff * pDLightCache->dLights[numLights].color.a;

				st[count].s = xyz[count].x - pDLightCache->dLights[numLights].xyz.x;
				st[count].t = xyz[count].y - pDLightCache->dLights[numLights].xyz.y;

				st[count].s *= invXScale;
				st[count].t *= invXScale;

				st[count].s += 0.5f;
				st[count].t += 0.5f;

				pChunk->xyz[count].x = xyz[count].x;
				pChunk->xyz[count].y = xyz[count].y;
				pChunk->xyz[count].z = xyz[count].z;

				pChunk->bgra[count].b = bgra[count].b;
				pChunk->bgra[count].g = bgra[count].g;
				pChunk->bgra[count].r = bgra[count].r;
				pChunk->bgra[count].a = bgra[count].a;

				pChunk->st0[count].s = st[count].s;
				pChunk->st0[count].t = st[count].t;

				pChunk->st1[count].s = stBase[count].s;
				pChunk->st1[count].t = stBase[count].t;
			}

			{
				unsigned int	index;
				int	first, last;

				first = 1;
				last = pChunk->numVertices - 1;

				pChunk->indices[0] = 0;
				pChunk->indices[1] = last--;
				pChunk->indices[2] = first++;

				for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
				{
					pChunk->indices[index] = last;
					pChunk->indices[index + 1] = first;
				}
			}
		}
	}
}

/*
* Add light map for a ceiling to the render pool.
*/
void D3DRenderLMapPostCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache,
	bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_st	st[MAX_NPTS];
	custom_st	stBase[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	PDIB		pDib;
	int			count;
	int			numLights;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	if (pSector->ceiling)
	{
		pDib = pSector->ceiling;
	}
	else
		return;

	D3DRenderCeilingExtract(pNode, pDib, xyz, stBase, NULL);

	// dynamic lightmaps
	for (numLights = 0; numLights < pDLightCache->numLights; numLights++)
	{
		custom_xyz	vector;
		float		distance, lightRange;
		int			unlit;
		float		falloff, invXScale, invYScale, invZScale,
			invXScaleHalf, invYScaleHalf, invZScaleHalf;

		invXScale = pDLightCache->dLights[numLights].invXYZScale.x;
		invYScale = pDLightCache->dLights[numLights].invXYZScale.y;
		invZScale = pDLightCache->dLights[numLights].invXYZScale.z;

		invXScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.x;
		invYScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.y;
		invZScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.z;

		unlit = 0;

		lightRange = (pDLightCache->dLights[numLights].xyzScale.x / 1.5f) *
			(pDLightCache->dLights[numLights].xyzScale.x / 1.5f);


		// if dlight is too far away, skip it
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			vector.x = xyz[count].x - pDLightCache->dLights[numLights].xyz.x;
			vector.y = xyz[count].y - pDLightCache->dLights[numLights].xyz.y;

			distance = ((vector.x * vector.x) + (vector.y * vector.y));

			if (distance > lightRange)
				unlit++;
		}

		if (unlit < pNode->u.leaf.poly.npts)
		{
			custom_xyz	lightVec, normal;
			float		cosAngle;

			lightVec.x = pDLightCache->dLights[numLights].xyz.x - xyz[0].x;
			lightVec.y = pDLightCache->dLights[numLights].xyz.y - xyz[0].y;
			lightVec.z = pDLightCache->dLights[numLights].xyz.z - xyz[0].z;

			normal.x = 0;
			normal.y = 0;
			normal.z = -1.0f;

			cosAngle = lightVec.x * normal.x +
				lightVec.y * normal.y +
				lightVec.z * normal.z;

			if (cosAngle <= 0)
				continue;

			pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->numVertices = pNode->u.leaf.poly.npts;
			pChunk->numIndices = pChunk->numVertices;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->pSector = pSector;
			pPacket->pMaterialFctn = &D3DMaterialLMapDynamicPacket;
			pChunk->side = 0;

			if (pSector->light <= 127)
				pChunk->flags |= D3DRENDER_NOAMBIENT;

			if (bDynamic)
				pChunk->pMaterialFctn = &D3DMaterialLMapDynamicChunk;
			else
				pChunk->pMaterialFctn = &D3DMaterialLMapStaticChunk;

			for (count = 0; count < pNode->u.leaf.poly.npts; count++)
			{
				falloff = (xyz[count].z - pDLightCache->dLights[numLights].xyz.z) *
					invZScaleHalf;

				if (falloff < 0)
					falloff = -falloff;

				falloff = min(1.0f, falloff);
				falloff = 1.0f - falloff;

				bgra[count].b = falloff * pDLightCache->dLights[numLights].color.b;
				bgra[count].g = falloff * pDLightCache->dLights[numLights].color.g;
				bgra[count].r = falloff * pDLightCache->dLights[numLights].color.r;
				bgra[count].a = falloff * pDLightCache->dLights[numLights].color.a;

				st[count].s = xyz[count].x - pDLightCache->dLights[numLights].xyz.x;
				st[count].t = xyz[count].y - pDLightCache->dLights[numLights].xyz.y;

				st[count].s *= invXScale;
				st[count].t *= invYScale;

				st[count].s += 0.5f;
				st[count].t += 0.5f;

				pChunk->xyz[count].x = xyz[count].x;
				pChunk->xyz[count].y = xyz[count].y;
				pChunk->xyz[count].z = xyz[count].z;

				pChunk->bgra[count].b = bgra[count].b;
				pChunk->bgra[count].g = bgra[count].g;
				pChunk->bgra[count].r = bgra[count].r;
				pChunk->bgra[count].a = bgra[count].a;

				pChunk->st0[count].s = st[count].s;
				pChunk->st0[count].t = st[count].t;

				pChunk->st1[count].s = stBase[count].s;
				pChunk->st1[count].t = stBase[count].t;
			}

			{
				unsigned int	index;
				int	first, last;

				first = 1;
				last = pChunk->numVertices - 1;

				pChunk->indices[0] = 0;
				pChunk->indices[1] = first++;
				pChunk->indices[2] = last--;

				for (index = 3; index < pChunk->numIndices; first++, last--, index += 2)
				{
					pChunk->indices[index] = first;
					pChunk->indices[index + 1] = last;
				}
			}
		}
	}
}

/*
* Add light map for a wall to the render pool
*/
void D3DRenderLMapPostWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side, d_light_cache* pDLightCache, bool bDynamic)
{
	custom_xyz		xyz[4];
	custom_st		st[4];
	custom_st		stBase[4];
	custom_bgra		bgra[4];
	unsigned int	flags;
	PDIB			pDib;
	Sidedef* pSideDef;
	int				numLights;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	// pos and neg sidedefs have their x and y coords reversed
	if (side > 0)
	{
		pSideDef = pWall->pos_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pWall->pos_sidedef->normal_bmap)
				pDib = pWall->pos_sidedef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pWall->pos_sidedef->below_bmap)
				pDib = pWall->pos_sidedef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pWall->pos_sidedef->above_bmap)
				pDib = pWall->pos_sidedef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}
	else if (side < 0)
	{
		pSideDef = pWall->neg_sidedef;

		if (NULL == pSideDef)
			return;

		switch (type)
		{
		case D3DRENDER_WALL_NORMAL:
			if (pWall->neg_sidedef->normal_bmap)
				pDib = pWall->neg_sidedef->normal_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_BELOW:
			if (pWall->neg_sidedef->below_bmap)
				pDib = pWall->neg_sidedef->below_bmap;
			else
				pDib = NULL;
			break;

		case D3DRENDER_WALL_ABOVE:
			if (pWall->neg_sidedef->above_bmap)
				pDib = pWall->neg_sidedef->above_bmap;
			else
				pDib = NULL;
			break;

		default:
			break;
		}
	}

	if (NULL == pDib)
		return;

	D3DRenderWallExtract(pWall, pDib, &flags, xyz, stBase, bgra, type, side);

	// add dynamic lightmaps
	for (numLights = 0; numLights < pDLightCache->numLights; numLights++)
	{
		custom_xyz	vector;
		float		distance, lightRange;
		int			unlit;
		unsigned int	i;
		float		falloff, invXScale, invYScale, invZScale,
			invXScaleHalf, invYScaleHalf, invZScaleHalf;

		invXScale = pDLightCache->dLights[numLights].invXYZScale.x;
		invYScale = pDLightCache->dLights[numLights].invXYZScale.y;
		invZScale = pDLightCache->dLights[numLights].invXYZScale.z;

		invXScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.x;
		invYScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.y;
		invZScaleHalf = pDLightCache->dLights[numLights].invXYZScaleHalf.z;

		unlit = 0;
		lightRange = (pDLightCache->dLights[numLights].xyzScale.x / 1.5f) *
			(pDLightCache->dLights[numLights].xyzScale.x / 1.5f);

		// if dlight is too far away, skip it
		for (i = 0; i < 4; i++)
		{
			vector.x = xyz[i].x - pDLightCache->dLights[numLights].xyz.x;
			vector.y = xyz[i].y - pDLightCache->dLights[numLights].xyz.y;

			distance = ((vector.x * vector.x) + (vector.y * vector.y));

			if (distance > lightRange)
				unlit++;
		}

		if (unlit < 4)
		{
			custom_xyz	normal, vec0, vec1;

			// calc cross product, get normal and determine major axis
			vec0.x = xyz[1].x - xyz[0].x;
			vec0.y = xyz[1].y - xyz[0].y;
			vec0.z = xyz[1].z - xyz[0].z;

			vec1.x = xyz[3].x - xyz[0].x;
			vec1.y = xyz[3].y - xyz[0].y;
			vec1.z = xyz[3].z - xyz[0].z;

			normal.x = vec0.z * vec1.y - vec0.y * vec1.z;
			normal.z = vec0.y * vec1.x - vec0.x * vec1.y;
			normal.y = vec0.x * vec1.z - vec0.z * vec1.x;

			custom_xyz	lightVec;
			float		cosAngle;

			lightVec.x = pDLightCache->dLights[numLights].xyz.x - xyz[0].x;
			lightVec.y = pDLightCache->dLights[numLights].xyz.y - xyz[0].y;
			lightVec.z = pDLightCache->dLights[numLights].xyz.z - xyz[0].z;

			cosAngle = lightVec.x * normal.x +
				lightVec.y * normal.y +
				lightVec.z * normal.z;

			if (cosAngle <= 0)
				continue;

			pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->flags = flags;
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->pSideDef = pSideDef;
			pChunk->pSectorPos = pWall->pos_sector;
			pChunk->pSectorNeg = pWall->neg_sector;
			pChunk->side = side;
			pPacket->pMaterialFctn = &D3DMaterialLMapDynamicPacket;

			if (bDynamic)
				pChunk->pMaterialFctn = &D3DMaterialLMapDynamicChunk;
			else
				pChunk->pMaterialFctn = &D3DMaterialLMapStaticChunk;

			if (normal.x < 0)
				normal.x = -normal.x;

			if (normal.y < 0)
				normal.y = -normal.y;

			if (normal.x > normal.y)
			{
				for (i = 0; i < 4; i++)
				{
					falloff = (xyz[i].x - pDLightCache->dLights[numLights].xyz.x) *
						invXScaleHalf;

					if (falloff < 0)
						falloff = -falloff;

					falloff = min(1.0f, falloff);
					falloff = 1.0f - falloff;

					st[i].s = (xyz[i].y - pDLightCache->dLights[numLights].xyz.y) *
						invYScale + 0.5f;
					st[i].t = (xyz[i].z - pDLightCache->dLights[numLights].xyz.z) *
						invZScale + 0.5f;
					bgra[i].b = falloff * pDLightCache->dLights[numLights].color.b;
					bgra[i].g = falloff * pDLightCache->dLights[numLights].color.g;
					bgra[i].r = falloff * pDLightCache->dLights[numLights].color.r;
					bgra[i].a = falloff * pDLightCache->dLights[numLights].color.a;
				}
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					falloff = (xyz[i].y - pDLightCache->dLights[numLights].xyz.y) *
						invYScaleHalf;

					if (falloff < 0)
						falloff = -falloff;

					falloff = min(1.0f, falloff);
					falloff = 1.0f - falloff;

					st[i].s = (xyz[i].x - pDLightCache->dLights[numLights].xyz.x) *
						invXScale + 0.5f;
					st[i].t = (xyz[i].z - pDLightCache->dLights[numLights].xyz.z) *
						invZScale + 0.5f;
					bgra[i].b = falloff * pDLightCache->dLights[numLights].color.b;
					bgra[i].g = falloff * pDLightCache->dLights[numLights].color.g;
					bgra[i].r = falloff * pDLightCache->dLights[numLights].color.r;
					bgra[i].a = falloff * pDLightCache->dLights[numLights].color.a;
				}
			}

			for (i = 0; i < pChunk->numVertices; i++)
			{
				pChunk->xyz[i].x = xyz[i].x;
				pChunk->xyz[i].y = xyz[i].y;
				pChunk->xyz[i].z = xyz[i].z;

				pChunk->bgra[i].b = bgra[i].b;
				pChunk->bgra[i].g = bgra[i].g;
				pChunk->bgra[i].r = bgra[i].r;
				pChunk->bgra[i].a = bgra[i].a;

				pChunk->st0[i].s = st[i].s;
				pChunk->st0[i].t = st[i].t;

				pChunk->st1[i].s = stBase[i].s;
				pChunk->st1[i].t = stBase[i].t;
			}

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}
}

/**
* Build up the gemetry for a given room, adding all render packets to the provided pool.
*/
void D3DGeometryBuildNew(
	const WorldRenderParams& worldRenderParams,
	const WorldPropertyParams& worldPropertyParams,
	const LightAndTextureParams& lightAndTextureParams, 
	bool transparent_pass)
{
	int			count;
	BSPnode		*pNode = NULL;
	WallData	*pWall;

	auto& cacheSystem = worldRenderParams.cacheSystemParams;
	auto& pools = worldRenderParams.poolParams;
	auto& room = worldRenderParams.room;

	for (count = 0; count < room->num_nodes; count++)
	{
		pNode = &room->nodes[count];

		switch (pNode->type)
		{
			case BSPinternaltype:
				for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
				{

					// Determine if the wall is transparent
					bool isTransparent = (pWall->pos_sidedef && pWall->pos_sidedef->flags & WF_TRANSPARENT) ||
						(pWall->neg_sidedef && pWall->neg_sidedef->flags & WF_TRANSPARENT);

					if (!ShouldRenderInCurrentPass(transparent_pass, isTransparent))
						continue;

					int	flags, wallFlags;

					flags = 0;
					wallFlags = 0;

					if (pWall->pos_sidedef)
					{
						wallFlags |= pWall->pos_sidedef->flags;

						if (pWall->pos_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->pos_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->pos_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if (pWall->neg_sidedef)
					{
						wallFlags |= pWall->neg_sidedef->flags;

						if (pWall->neg_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->neg_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->neg_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					pWall->separator.a = pNode->u.internal.separator.a;
					pWall->separator.b = pNode->u.internal.separator.b;
					pWall->separator.c = pNode->u.internal.separator.c;

					if ((flags & D3DRENDER_WALL_NORMAL) && ((short)pWall->z2 != (short)pWall->z1)
						|| ((short)pWall->zz2 != (short)pWall->zz1))
					{
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_NORMAL, 1, false);
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_NORMAL, -1, false);
					}

					if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0))
					{
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_BELOW, 1, false);
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_BELOW, -1, false);
					}

					if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2))
					{
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_ABOVE, 1, false);
						D3DRenderPacketWallAdd(pWall, pools.worldPoolStatic, D3DRENDER_WALL_ABOVE, -1, false);
					}
				}

			break;

			case BSPleaftype: // floors and ceilings
				D3DRenderPacketFloorAdd(pNode, pools.worldPoolStatic, false);
				D3DRenderPacketCeilingAdd(pNode, pools.worldPoolStatic, false);
        break;

			default:
        break;
		}
	}

	if (config.bDynamicLighting)
	{
		D3DCacheSystemReset(cacheSystem.worldCacheSystemStatic);
		D3DRenderPoolReset(pools.lMapPoolStatic, &D3DMaterialLMapDynamicPool);

		for (count = 0; count < room->num_nodes; count++)
		{
			pNode = &room->nodes[count];

			switch (pNode->type)
			{
				case BSPinternaltype:
					for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
					{
						int	flags, wallFlags;

						flags = 0;
						wallFlags = 0;

						if (pWall->pos_sidedef)
						{
							wallFlags |= pWall->pos_sidedef->flags;

							if (pWall->pos_sidedef->normal_bmap)
								flags |= D3DRENDER_WALL_NORMAL;

							if (pWall->pos_sidedef->below_bmap)
								flags |= D3DRENDER_WALL_BELOW;

							if (pWall->pos_sidedef->above_bmap)
								flags |= D3DRENDER_WALL_ABOVE;
						}

						if (pWall->neg_sidedef)
						{
							wallFlags |= pWall->neg_sidedef->flags;

							if (pWall->neg_sidedef->normal_bmap)
								flags |= D3DRENDER_WALL_NORMAL;

							if (pWall->neg_sidedef->below_bmap)
								flags |= D3DRENDER_WALL_BELOW;

							if (pWall->neg_sidedef->above_bmap)
								flags |= D3DRENDER_WALL_ABOVE;
						}

						pWall->separator.a = pNode->u.internal.separator.a;
						pWall->separator.b = pNode->u.internal.separator.b;
						pWall->separator.c = pNode->u.internal.separator.c;

						if ((flags & D3DRENDER_WALL_NORMAL) && ((short)pWall->z2 != (short)pWall->z1)
							|| ((short)pWall->zz2 != (short)pWall->zz1))
						{
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_NORMAL, 1, lightAndTextureParams.lightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_NORMAL, -1, lightAndTextureParams.lightCache, FALSE);
						}

						if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
							|| ((short)pWall->zz1 != (short)pWall->zz0))
						{
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_BELOW, 1, lightAndTextureParams.lightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_BELOW, -1, lightAndTextureParams.lightCache, FALSE);
						}

						if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
							|| ((short)pWall->zz3 != (short)pWall->zz2))
						{
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_ABOVE, 1, lightAndTextureParams.lightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, pools.lMapPoolStatic, D3DRENDER_WALL_ABOVE, -1, lightAndTextureParams.lightCache, FALSE);
						}
					}

				break;

				case BSPleaftype:
					D3DRenderLMapPostFloorAdd(pNode, pools.lMapPoolStatic, lightAndTextureParams.lightCache, FALSE);
					D3DRenderLMapPostCeilingAdd(pNode, pools.lMapPoolStatic, lightAndTextureParams.lightCache, FALSE);
				break;

				default:
				break;
			}
		}

		D3DCacheFill(cacheSystem.lMapCacheSystemStatic, pools.lMapPoolStatic, 2);
	}

	for (count = 0; count < room->num_nodes; count++)
	{
		pNode = &room->nodes[count];

		switch (pNode->type)
		{
			case BSPinternaltype:
				for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
				{
					int	flags, wallFlags;

					flags = 0;
					wallFlags = 0;

					if (pWall->pos_sidedef)
					{
						wallFlags |= pWall->pos_sidedef->flags;

						if (pWall->pos_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->pos_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->pos_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if (pWall->neg_sidedef)
					{
						wallFlags |= pWall->neg_sidedef->flags;

						if (pWall->neg_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->neg_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->neg_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if ((flags & D3DRENDER_WALL_NORMAL) && ((short)pWall->z2 != (short)pWall->z1)
						|| ((short)pWall->zz2 != (short)pWall->zz1))
					{
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_NORMAL, 1, worldPropertyParams.noLookThrough, false);
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_NORMAL, -1, worldPropertyParams.noLookThrough, false);
					}

					if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0))
					{
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_BELOW, 1, worldPropertyParams.noLookThrough, false);
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_BELOW, -1, worldPropertyParams.noLookThrough, false);
					}

					if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2))
					{
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_ABOVE, 1, worldPropertyParams.noLookThrough, false);
						D3DRenderPacketWallMaskAdd(pWall, pools.wallMaskPool, D3DRENDER_WALL_ABOVE, -1, worldPropertyParams.noLookThrough, false);
					}
				}

			break;

			case BSPleaftype:
				if ((pNode->u.leaf.sector->ceiling == NULL) &&
					(pNode->u.leaf.sector->sloped_floor == NULL))
					D3DRenderCeilingMaskAdd(pNode, pools.wallMaskPool, room, worldPropertyParams.noLookThrough, 
						worldPropertyParams.lightOrange, false);
			break;

			default:
			break;
		}
	}

	{
		D3DCacheFill(cacheSystem.worldCacheSystemStatic, pools.worldPoolStatic	, 1);
		D3DCacheFill(cacheSystem.wallMaskCacheSystem, pools.wallMaskPool, 1);
	}
}

/*
* Update the 3d world gemoetry for the 3d world.
*/
void GeometryUpdate(d3d_render_pool_new *pPool, d3d_render_cache_system *pCacheSystem)
{
	auto shadeAmount = getShadeAmount();
	auto& sunVect = getSunVector();

	u_int				curPacket, curChunk;
	u_int				i, numPackets;
	d3d_render_cache	*pRenderCache = NULL;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	Sector					*pSector;
	int						distX, distY, distance, paletteIndex;
	list_type				list;
	long					lightScale;
	long					lo_end = FINENESS-shadeAmount;

	for (list = pPool->renderPacketList; list != pPool->curPacketList->next; list = list->next)
	{
		pPacket = (d3d_render_packet_new *)list->data;

		if (list == pPool->curPacketList)
			numPackets = pPool->curPacket;
		else
			numPackets = pPool->size;

		for (curPacket = 0; curPacket < numPackets; curPacket++, pPacket++)
		{
			for (curChunk = 0; curChunk < pPacket->curChunk; curChunk++)
			{
				pChunk = &pPacket->renderChunks[curChunk];

				if (pChunk->pSector)
					pSector = pChunk->pSector;
				else
				{
					// is a wall
					if (pChunk->side > 0)
						pSector = pChunk->pSectorPos;
					else
						pSector = pChunk->pSectorNeg;
				}

				if (NULL == pSector)
					continue;

				for (i = 0; i < pChunk->numVertices; i++)
				{
					const auto playerPosition = getPlayerPosition();
					distX = pChunk->xyz[i].x - playerPosition.first;
					distY = pChunk->xyz[i].y - playerPosition.second;

					distance = DistanceGet(distX, distY);

					if (shadeAmount != 0)
					{
						long	a, b;

						a = pChunk->plane.a;
						b = pChunk->plane.b;

						if (pChunk->side < 0)
						{
							a = -a;
							b = -b;
						}

						lightScale = (long) (a * sunVect.x +
										b * sunVect.y) >> LOG_FINENESS;

						lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range

						lightScale = lo_end + ((lightScale * shadeAmount)>>LOG_FINENESS);
						
						if (lightScale > FINENESS)
							lightScale = FINENESS;
						else if ( lightScale < 0)
							lightScale = 0;
					}
					else
						lightScale = FINENESS;

					if (isFogEnabled())
						paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightScale, 0);
					else
						paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightScale, 0);

					pChunk->bgra[i].r = pChunk->bgra[i].g = pChunk->bgra[i].b =
						paletteIndex * COLOR_AMBIENT / 64;
					pChunk->bgra[i].a = 255;
				}

				if (pRenderCache != pChunk->pRenderCache)
				{
					if (pRenderCache)
						CACHE_BGRA_UNLOCK(pRenderCache);

					pRenderCache = pChunk->pRenderCache;
					CACHE_BGRA_LOCK(pRenderCache);
				}

				for (i = 0; i < pChunk->numVertices; i++)
				{
					D3DCacheBGRASet(pChunk->pRenderCache, pChunk->startIndex + i, pChunk->bgra[i].b,
						pChunk->bgra[i].g, pChunk->bgra[i].r, pChunk->bgra[i].a);
				}
			}
		}
	}

	if (pRenderCache)
		CACHE_BGRA_UNLOCK(pRenderCache);
}

/**
 * Extracts and prepares wall data for rendering by calculating texture coordinates,
 * vertex positions, and applying lighting and animation effects based on wall type and orientation.
 */
int D3DRenderWallExtract(WallData *pWall, PDIB pDib, unsigned int *flags, 
	custom_xyz *pXYZ, custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side)
{
	int				top, bottom;
	int				xOffset, yOffset;
	Sidedef			*pSideDef;
	int				drawTopDown;
	int				paletteIndex;
	BYTE			light;

	if (pXYZ)
	{
		// pos and neg sidedefs have their x and y coords reversed
		if (side > 0)
		{
			pSideDef = pWall->pos_sidedef;

			if (NULL == pWall->pos_sector)
				light = 0;
			else
				light = pWall->pos_sector->light;

			xOffset = pWall->pos_xoffset;
			yOffset = pWall->pos_yoffset;

			pXYZ[0].x = pWall->x0;
			pXYZ[3].x = pWall->x1;
			pXYZ[1].x = pWall->x0;
			pXYZ[2].x = pWall->x1;

			pXYZ[0].y = pWall->y0;
			pXYZ[3].y = pWall->y1;
			pXYZ[1].y = pWall->y0;
			pXYZ[2].y = pWall->y1;

			switch (type)
			{
				case D3DRENDER_WALL_NORMAL:
				{
					pXYZ[0].z = pWall->z2;
					pXYZ[3].z = pWall->zz2;
					pXYZ[1].z = pWall->z1;
					pXYZ[2].z = pWall->zz1;

					if (pSideDef->flags & WF_NORMAL_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_BELOW:
				{
					if ((pWall->bowtie_bits & BT_BELOW_POS) ||
						(pWall->bowtie_bits & BT_BELOW_NEG))
					{
						pXYZ[0].z = pWall->z1Neg;
						pXYZ[3].z = pWall->zz1Neg;
					}
					else
					{
						pXYZ[0].z = pWall->z1;
						pXYZ[3].z = pWall->zz1;
					}

					pXYZ[1].z = pWall->z0;
					pXYZ[2].z = pWall->zz0;

					if (pSideDef->flags & WF_BELOW_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
					pXYZ[0].z = pWall->z3;
					pXYZ[3].z = pWall->zz3;
					pXYZ[1].z = pWall->z2;
					pXYZ[2].z = pWall->zz2;

					if (pSideDef->flags & WF_ABOVE_BOTTOMUP)
						drawTopDown = 0;
					else
						drawTopDown = 1;
				}
				break;

				default:
				break;
			}
		}
		else if (side < 0)
		{
			pSideDef = pWall->neg_sidedef;

			if (NULL == pWall->neg_sector)
				light = 0;
			else
				light = pWall->neg_sector->light;

			xOffset = pWall->neg_xoffset;
			yOffset = pWall->neg_yoffset;

			pXYZ[0].x = (float)pWall->x1;
			pXYZ[3].x = (float)pWall->x0;
			pXYZ[1].x = (float)pWall->x1;
			pXYZ[2].x = (float)pWall->x0;

			pXYZ[0].y = (float)pWall->y1;
			pXYZ[3].y = (float)pWall->y0;
			pXYZ[1].y = (float)pWall->y1;
			pXYZ[2].y = (float)pWall->y0;

			switch (type)
			{
				case D3DRENDER_WALL_NORMAL:
				{
					pXYZ[0].z = (long)pWall->zz2;
					pXYZ[3].z = (long)pWall->z2;
					pXYZ[1].z = (long)pWall->zz1;
					pXYZ[2].z = (long)pWall->z1;

					if (pSideDef->flags & WF_NORMAL_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_BELOW:
				{
					pXYZ[0].z = (long)pWall->zz1;
					pXYZ[3].z = (long)pWall->z1;
					pXYZ[1].z = (long)pWall->zz0;
					pXYZ[2].z = (long)pWall->z0;

					if (pSideDef->flags & WF_BELOW_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
					pXYZ[0].z = (long)pWall->zz3;
					pXYZ[3].z = (long)pWall->z3;
					pXYZ[1].z = (long)pWall->zz2;
					pXYZ[2].z = (long)pWall->z2;

					if (pSideDef->flags & WF_ABOVE_BOTTOMUP)
						drawTopDown = 0;
					else
						drawTopDown = 1;
				}
				break;

				default:
				break;
			}
		}
		else
			assert(0);
	}

	*flags = 0;

	if (pSideDef->flags & WF_TRANSPARENT)
		*flags |= D3DRENDER_TRANSPARENT;

	switch (type)
	{
		case D3DRENDER_WALL_NORMAL:
			if (pSideDef->flags & WF_NO_VTILE)
				*flags |= D3DRENDER_NO_VTILE;
			if (pSideDef->flags & WF_NO_HTILE)
				*flags |= D3DRENDER_NO_HTILE;
		break;

		default:
      break;
	}

	if ((pXYZ) && (pST))
	{
		float	invWidth, invHeight, invWidthFudge, invHeightFudge;

		// force a wraparound because many textures in the old client do this, grr
		yOffset = yOffset << 16;
		yOffset = yOffset >> 16;

		invWidth = 1.0f / (float)pDib->width;
		invHeight = 1.0f / (float)pDib->height;
		invWidthFudge = 1.0f / ((float)pDib->width * PETER_FUDGE);
		invHeightFudge = 1.0f / ((float)pDib->height * PETER_FUDGE);

		pST[0].s = (float)xOffset * (float)(pDib->shrink) * invHeight;
		pST[1].s = (float)xOffset * (float)(pDib->shrink) * invHeight;
		pST[3].s = (float)(pST[0].s + ((float)pWall->length * (float)pDib->shrink) * invHeight);
		pST[2].s = (float)(pST[1].s + ((float)pWall->length * (float)pDib->shrink) * invHeight);

		if (!drawTopDown)
		{
			if (pXYZ[1].z == pXYZ[2].z)
				bottom = pXYZ[1].z;
			else
			{
				bottom = min(pXYZ[1].z, pXYZ[2].z);
				bottom = bottom & ~(FINENESS - 1);
			}

			if (pXYZ[0].z == pXYZ[3].z)
				top = pXYZ[0].z;
			else
			{
				top = max(pXYZ[0].z, pXYZ[3].z);
				top = (top + FINENESS - 1) & ~(FINENESS - 1);
			}

			if (pXYZ[1].z == pXYZ[2].z)
			{
				pST[1].t = 1.0f - (((float)yOffset * (float)pDib->shrink)
					* invWidth);
				pST[2].t = 1.0f - (((float)yOffset * (float)pDib->shrink)
					* invWidth);
			}
			else
			{
				pST[1].t = 1.0f - (((float)yOffset * (float)pDib->shrink)
					*invWidth);
				pST[2].t = 1.0f - (((float)yOffset * (float)pDib->shrink)
					* invWidth);
				pST[1].t -= ((float)pXYZ[1].z - bottom) * (float)pDib->shrink
						* invWidthFudge;
				pST[2].t -= ((float)pXYZ[2].z - bottom) * (float)pDib->shrink
						* invWidthFudge;
			}

			pST[0].t = pST[1].t -
				((float)(pXYZ[0].z - pXYZ[1].z) * (float)pDib->shrink * invWidthFudge);
			pST[3].t = pST[2].t -
				((float)(pXYZ[3].z - pXYZ[2].z) * (float)pDib->shrink * invWidthFudge);
		}
		else	// else, need to place tex origin at top left
		{
			if (pXYZ[0].z == pXYZ[3].z)
				top = pXYZ[0].z;
			else
			{
				top = max(pXYZ[0].z, pXYZ[3].z);
				top = (top + FINENESS - 1) & ~(FINENESS - 1);
			}

			if (pXYZ[1].z == pXYZ[2].z)
				bottom = pXYZ[1].z;
			else
			{
				bottom = min(pXYZ[1].z, pXYZ[2].z);
				bottom = bottom & ~(FINENESS - 1);
			}

			if (pXYZ[0].z == pXYZ[3].z)
			{
				pST[0].t = 0.0f;
				pST[3].t = 0.0f;
			}
			else
			{
				pST[0].t = ((float)top - pXYZ[0].z) * (float)pDib->shrink
						* invWidthFudge;
				pST[3].t = ((float)top - pXYZ[3].z) * (float)pDib->shrink
						* invWidthFudge;
			}

			pST[0].t -= ((float)(yOffset * pDib->shrink) * invWidth);
			pST[3].t -= ((float)(yOffset * pDib->shrink) * invWidth);

			pST[1].t = pST[0].t + ((pXYZ[0].z - pXYZ[1].z) * (float)pDib->shrink
				* invWidthFudge);
			pST[2].t = pST[3].t + ((pXYZ[3].z - pXYZ[2].z) * (float)pDib->shrink
				* invWidthFudge);
		}

		if (pSideDef->animate != NULL && pSideDef->animate->animation == ANIMATE_SCROLL)
		{
			int	i;
			if (pSideDef->flags & WF_BACKWARDS)
			{
				for (i = 0; i < 4; i++)
				{
					pST[i].s -= pSideDef->animate->u.scroll.xoffset * pDib->shrink *
						invHeight;
					pST[i].t += pSideDef->animate->u.scroll.yoffset * pDib->shrink *
						invWidth;
				}
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					pST[i].s += pSideDef->animate->u.scroll.xoffset * pDib->shrink *
						invHeight;
					pST[i].t -= pSideDef->animate->u.scroll.yoffset * pDib->shrink *
						invWidth;
				}
			}
		}

		if (pSideDef->flags & WF_BACKWARDS)
		{
			float	temp;

			temp = pST[3].s;
			pST[3].s = pST[0].s;
			pST[0].s = temp;

			temp = pST[2].s;
			pST[2].s = pST[1].s;
			pST[1].s = temp;
		}

		if (*flags & D3DRENDER_NO_VTILE)
		{
			if (pST[0].t < 0.0f)
			{
				float	tex, wall, ratio, temp;

				tex = pST[1].t - pST[0].t;
				if (tex == 0)
					tex = 1.0f;
				temp = -pST[0].t;
				ratio = temp / tex;

				wall = pXYZ[0].z - pXYZ[1].z;
				temp = wall * ratio;
				pXYZ[0].z -= temp;
				pST[0].t = 0.0f;
			}
			if (pST[3].t < 0.0f)
			{
				float	tex, wall, ratio, temp;

				tex = pST[2].t - pST[3].t;
				if (tex == 0)
					tex = 1.0f;
				temp = -pST[3].t;
				ratio = temp / tex;

				wall = pXYZ[3].z - pXYZ[2].z;
				temp = wall * ratio;
				pXYZ[3].z -= temp;
				pST[3].t = 0.0f;
			}

			pXYZ[1].z -= 16.0f;
			pXYZ[2].z -= 16.0f;
		}

		pST[0].t += 1.0f / pDib->width;
		pST[3].t += 1.0f / pDib->width;
		pST[1].t -= 1.0f / pDib->width;
		pST[2].t -= 1.0f / pDib->width;
	}

	if (pBGRA)
	{
		auto shadeAmount = getShadeAmount();
		auto& sunVect = getSunVector();
		int	i;
		float a, b;
		int	distX, distY, distance;
		long	lightScale;
		long lo_end = FINENESS-shadeAmount;

		for (i = 0; i < 4; i++)
		{
			const auto playerPosition = getPlayerPosition();
			distX = pXYZ[i].x - playerPosition.first;
			distY = pXYZ[i].y - playerPosition.second;

			distance = DistanceGet(distX, distY);

			if (shadeAmount != 0)
			{
				a = pWall->separator.a;
				b = pWall->separator.b;

				if (side < 0)
				{
					a = -a;
					b = -b;
				}

				lightScale = (long)(a * sunVect.x +
								b * sunVect.y) >> LOG_FINENESS;

				lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range

				lightScale = lo_end + ((lightScale * shadeAmount)>>LOG_FINENESS);
				
				if (lightScale > FINENESS)
					lightScale = FINENESS;
				else if ( lightScale < 0)
					lightScale = 0;
			}
			else
				lightScale = FINENESS;

			pWall->lightscale = lightScale;

			if (isFogEnabled())
				paletteIndex = GetLightPaletteIndex(FINENESS, light, lightScale, 0);
			else
				paletteIndex = GetLightPaletteIndex(distance, light, lightScale, 0);

			if (light <= 127)
				*flags |= D3DRENDER_NOAMBIENT;

			pBGRA[i].r = pBGRA[i].g = pBGRA[i].b = paletteIndex * COLOR_AMBIENT / 64;
			pBGRA[i].a = 255;
		}
	}

	return 1;
}

/**
 * Extracts and processes floor data for rendering, generating vertex positions,
 * texture coordinates, and applying lighting effects.
 */
void D3DRenderFloorExtract(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
							custom_bgra *pBGRA)
{
	Sector	*pSector = pNode->u.leaf.sector;
	int		count;
	int		left, top;
	int		paletteIndex;
	float	oneOverC, inv128, inv64;
	custom_xyz	intersectTop, intersectLeft;
	long		lightscale;

	left = top = 0;

	inv128 = 1.0f / (128.0f * PETER_FUDGE);
	inv64 = 1.0f / (64.0f * PETER_FUDGE);

	// generate texture coordinates
	if (pSector->sloped_floor)
	{
		left = pSector->sloped_floor->p0.x;
		top = pSector->sloped_floor->p0.y;
	}
	else
	{
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			if (pNode->u.leaf.poly.p[count].x < left)
				left = pNode->u.leaf.poly.p[count].x;
		}
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			if (pNode->u.leaf.poly.p[count].y < top)
				top = pNode->u.leaf.poly.p[count].y;
		}
	}

	if (pSector->sloped_floor)
	{
		oneOverC = 1.0f / pSector->sloped_floor->plane.c;
	}

	if (pXYZ)
	{
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			if (pSector->sloped_floor)
			{
				pXYZ[count].x = (float)pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = (float)pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (-pSector->sloped_floor->plane.a * pXYZ[count].x -
					pSector->sloped_floor->plane.b * pXYZ[count].y -
					pSector->sloped_floor->plane.d) * oneOverC;
			}
			else
			{
				pXYZ[count].x = (float)pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = (float)pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (float)pSector->floor_height;
			}

			if (pST)
			{
				custom_xyz	vectorU, vectorV, vector;
				float		U, temp;

				if (pSector->sloped_floor)
				{
					float	distance;

					// calc distance from top line (vector u)
					U = ((pXYZ[count].x - pSector->sloped_floor->p0.x) *
						(pSector->sloped_floor->p1.x - pSector->sloped_floor->p0.x)) +
						((pXYZ[count].z - pSector->sloped_floor->p0.z) *
						(pSector->sloped_floor->p1.z - pSector->sloped_floor->p0.z)) +
						((pXYZ[count].y - pSector->sloped_floor->p0.y) *
						(pSector->sloped_floor->p1.y - pSector->sloped_floor->p0.y));
					temp = ((pSector->sloped_floor->p1.x - pSector->sloped_floor->p0.x) *
						(pSector->sloped_floor->p1.x - pSector->sloped_floor->p0.x)) +
						((pSector->sloped_floor->p1.z - pSector->sloped_floor->p0.z) *
						(pSector->sloped_floor->p1.z - pSector->sloped_floor->p0.z)) +
						((pSector->sloped_floor->p1.y - pSector->sloped_floor->p0.y) *
						(pSector->sloped_floor->p1.y - pSector->sloped_floor->p0.y));

					if (temp == 0)
						temp = 1.0f;

					U /= temp;

					intersectTop.x = pSector->sloped_floor->p0.x +
						U * (pSector->sloped_floor->p1.x - pSector->sloped_floor->p0.x);
					intersectTop.z = pSector->sloped_floor->p0.z +
						U * (pSector->sloped_floor->p1.z - pSector->sloped_floor->p0.z);
					intersectTop.y = pSector->sloped_floor->p0.y +
						U * (pSector->sloped_floor->p1.y - pSector->sloped_floor->p0.y);

					pST[count].s = (float)sqrt((pXYZ[count].x - intersectTop.x) *
									(pXYZ[count].x - intersectTop.x) +
									(pXYZ[count].z - intersectTop.z) *
									(pXYZ[count].z - intersectTop.z) +
									(pXYZ[count].y - intersectTop.y) *
									(pXYZ[count].y - intersectTop.y));

					// calc distance from left line (vector v)
					U = ((pXYZ[count].x - pSector->sloped_floor->p0.x) *
						(pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x)) +
						((pXYZ[count].z - pSector->sloped_floor->p0.z) *
						(pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z)) +
						((pXYZ[count].y - pSector->sloped_floor->p0.y) *
						(pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y));
					temp = ((pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x) *
						(pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x)) +
						((pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z) *
						(pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z)) +
						((pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y) *
						(pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y));

					if (temp == 0)
						temp = 1.0f;

					U /= temp;

					intersectLeft.x = pSector->sloped_floor->p0.x +
						U * (pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x);
					intersectLeft.z = pSector->sloped_floor->p0.z +
						U * (pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z);
					intersectLeft.y = pSector->sloped_floor->p0.y +
						U * (pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y);

					pST[count].t = (float)sqrt((pXYZ[count].x - intersectLeft.x) *
									(pXYZ[count].x - intersectLeft.x) +
									(pXYZ[count].z - intersectLeft.z) *
									(pXYZ[count].z - intersectLeft.z) +
									(pXYZ[count].y - intersectLeft.y) *
									(pXYZ[count].y - intersectLeft.y));

					pST[count].s += pSector->ty / 2.0f;
					pST[count].t += pSector->tx / 2.0f;

					vectorU.x = pSector->sloped_floor->p1.x - pSector->sloped_floor->p0.x;
					vectorU.z = pSector->sloped_floor->p1.z - pSector->sloped_floor->p0.z;
					vectorU.y = pSector->sloped_floor->p1.y - pSector->sloped_floor->p0.y;

					distance = (float)sqrt((vectorU.x * vectorU.x) +
						(vectorU.y * vectorU.y));

					if (distance == 0)
						distance = 1.0f;

					vectorU.x /= distance;
					vectorU.z /= distance;
					vectorU.y /= distance;

					vectorV.x = pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x;
					vectorV.z = pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z;
					vectorV.y = pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y;

					distance = (float)sqrt((vectorV.x * vectorV.x) +
						(vectorV.y * vectorV.y));

					if (distance == 0)
						distance = 1.0f;

					vectorV.x /= distance;
					vectorV.z /= distance;
					vectorV.y /= distance;

					vector.x = pXYZ[count].x - pSector->sloped_floor->p0.x;
					vector.y = pXYZ[count].y - pSector->sloped_floor->p0.y;

					distance = (float)sqrt((vector.x * vector.x) +
						(vector.y * vector.y));

					if (distance == 0)
						distance = 1.0f;

					vector.x /= distance;
					vector.y /= distance;

					if (((vector.x * vectorU.x) +
						(vector.y * vectorU.y)) <= 0)
						pST[count].t = -pST[count].t;

					if (((vector.x * vectorV.x) +
						(vector.y * vectorV.y)) > 0)
						pST[count].s = -pST[count].s;
				}
				else
				{
					pST[count].s = abs(pNode->u.leaf.poly.p[count].y - top) - pSector->ty;
					pST[count].t = abs(pNode->u.leaf.poly.p[count].x - left) - pSector->tx;
				}

				if (pSector->animate != NULL && pSector->animate->animation == ANIMATE_SCROLL)
				{
					if (pSector->flags & SF_SCROLL_FLOOR)
					{
						pST[count].s -= pSector->animate->u.scroll.yoffset;
						pST[count].t += pSector->animate->u.scroll.xoffset;
					}
				}

        pST[count].s *= inv64;
        pST[count].t *= inv64;
			}
		}
	}

	if (pBGRA)
	{
		int	distX, distY, distance;

		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			const auto playerPosition = getPlayerPosition();
			distX = pXYZ[count].x - playerPosition.first;
			distY = pXYZ[count].y - playerPosition.second;

			lightscale = FINENESS;

			auto shadeAmount = getShadeAmount();
			auto& sunVect = getSunVector();

			if (shadeAmount != 0)
			{
				// floor is sloped, and it's marked as being steep enough to be eligible for
				// directional lighting
				if ((pNode->u.leaf.sector->sloped_floor != NULL) &&
					(pNode->u.leaf.sector->sloped_floor->flags & SLF_DIRECTIONAL))
				{
					long lo_end = FINENESS-shadeAmount;

					// light scale is based on dot product of surface normal and sun vector
					lightscale = (long)(pNode->u.leaf.sector->sloped_floor->plane.a * sunVect.x +
						pNode->u.leaf.sector->sloped_floor->plane.b * sunVect.y +
						pNode->u.leaf.sector->sloped_floor->plane.c * sunVect.z)>>LOG_FINENESS;

					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range

					lightscale = lo_end + ((lightscale * shadeAmount)>>LOG_FINENESS);

					if (lightscale > FINENESS)
						lightscale = FINENESS;
					else if ( lightscale < 0)
						lightscale = 0;

					pNode->u.leaf.sector->sloped_floor->lightscale = lightscale;
				}
			}
			else
			{ // normal light intensity
				if (pNode->u.leaf.sector->sloped_floor != NULL)
					pNode->u.leaf.sector->sloped_floor->lightscale = FINENESS;
			}

			distance = DistanceGet(distX, distY);

			if (isFogEnabled())
				paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightscale, 0);
			else
				paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightscale, 0);

			pBGRA[count].r = pBGRA[count].g = pBGRA[count].b = paletteIndex * COLOR_AMBIENT / 64;
			pBGRA[count].a = 255;
		}
	}
}

/**
 * Extracts and processes ceiling data for rendering by calculating vertex positions,
 * texture coordinates, and applying lighting and animation effects.
 */
void D3DRenderCeilingExtract(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
							custom_bgra *pBGRA)
{
	Sector		*pSector = pNode->u.leaf.sector;
	int			count;
	int			left, top;
	int			paletteIndex;
	float		oneOverC, inv128, inv64;
	custom_xyz	intersectTop, intersectLeft;
	long		lightscale;

	left = top = 0;

	inv128 = 1.0f / (128.0f * PETER_FUDGE);
	inv64 = 1.0f / (64.0f * PETER_FUDGE);

	// generate texture coordinates
	for (count = 0; count < pNode->u.leaf.poly.npts; count++)
	{
		if (pNode->u.leaf.poly.p[count].x < left)
			left = pNode->u.leaf.poly.p[count].x;
	}
	for (count = 0; count < pNode->u.leaf.poly.npts; count++)
	{
		if (pNode->u.leaf.poly.p[count].y < top)
			top = pNode->u.leaf.poly.p[count].y;
	}

	if (pSector->sloped_ceiling)
	{
		// extract plane normal
		oneOverC = 1.0f / pSector->sloped_ceiling->plane.c;
	}

	if (pXYZ)
	{
		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			if (pSector->sloped_ceiling)
			{
				pXYZ[count].x = (float)pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = (float)pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (-pSector->sloped_ceiling->plane.a * pXYZ[count].x -
					pSector->sloped_ceiling->plane.b * pXYZ[count].y -
					pSector->sloped_ceiling->plane.d) * oneOverC;
			}
			else
			{
				pXYZ[count].x = (float)pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = (float)pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (float)pSector->ceiling_height;
			}

			if (pST)
			{
				custom_xyz	vectorU, vectorV, vector;
				float		U, temp;

				if (pSector->sloped_ceiling)
				{
					float	distance;

					// calc distance from top line (vector u)
					U = ((pXYZ[count].x - pSector->sloped_ceiling->p0.x) *
						(pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x)) +
						((pXYZ[count].z - pSector->sloped_ceiling->p0.z) *
						(pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z)) +
						((pXYZ[count].y - pSector->sloped_ceiling->p0.y) *
						(pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y));
					temp = ((pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x) *
						(pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x)) +
						((pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z) *
						(pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z)) +
						((pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y) *
						(pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y));

					if (temp == 0)
						temp = 1.0f;

					U /= temp;

					intersectTop.x = pSector->sloped_ceiling->p0.x +
						U * (pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x);
					intersectTop.z = pSector->sloped_ceiling->p0.z +
						U * (pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z);
					intersectTop.y = pSector->sloped_ceiling->p0.y +
						U * (pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y);

					pST[count].s = (float)sqrt((pXYZ[count].x - intersectTop.x) *
									(pXYZ[count].x - intersectTop.x) +
									(pXYZ[count].z - intersectTop.z) *
									(pXYZ[count].z - intersectTop.z) +
									(pXYZ[count].y - intersectTop.y) *
									(pXYZ[count].y - intersectTop.y));

					// calc distance from left line (vector v)
					U = ((pXYZ[count].x - pSector->sloped_ceiling->p0.x) *
						(pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x)) +
						((pXYZ[count].z - pSector->sloped_ceiling->p0.z) *
						(pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z)) +
						((pXYZ[count].y - pSector->sloped_ceiling->p0.y) *
						(pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y));
					temp = ((pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x) *
						(pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x)) +
						((pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z) *
						(pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z)) +
						((pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y) *
						(pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y));

					if (temp == 0)
						temp = 1.0f;

					U /= temp;

					intersectLeft.x = pSector->sloped_ceiling->p0.x +
						U * (pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x);
					intersectLeft.z = pSector->sloped_ceiling->p0.z +
						U * (pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z);
					intersectLeft.y = pSector->sloped_ceiling->p0.y +
						U * (pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y);

					pST[count].t = (float)sqrt((pXYZ[count].x - intersectLeft.x) *
									(pXYZ[count].x - intersectLeft.x) +
									(pXYZ[count].z - intersectLeft.z) *
									(pXYZ[count].z - intersectLeft.z) +
									(pXYZ[count].y - intersectLeft.y) *
									(pXYZ[count].y - intersectLeft.y));

					vectorU.x = pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x;
					vectorU.z = pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z;
					vectorU.y = pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y;

					distance = (float)sqrt((vectorU.x * vectorU.x) +
						(vectorU.y * vectorU.y));

					if (distance == 0)
						distance = 1.0f;

					vectorU.x /= distance;
					vectorU.z /= distance;
					vectorU.y /= distance;

					vectorV.x = pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x;
					vectorV.z = pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z;
					vectorV.y = pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y;

					distance = (float)sqrt((vectorV.x * vectorV.x) +
						(vectorV.y * vectorV.y));

					if (distance == 0)
						distance = 1.0f;

					vectorV.x /= distance;
					vectorV.z /= distance;
					vectorV.y /= distance;

					vector.x = pXYZ[count].x - pSector->sloped_ceiling->p0.x;
					vector.y = pXYZ[count].y - pSector->sloped_ceiling->p0.y;

					distance = (float)sqrt((vector.x * vector.x) +
						(vector.y * vector.y));

					if (distance == 0)
						distance = 1.0f;

					vector.x /= distance;
					vector.y /= distance;

					if (((vector.x * vectorU.x) +
						(vector.y * vectorU.y)) < 0)
						pST[count].t = -pST[count].t;

					if (((vector.x * vectorV.x) +
						(vector.y * vectorV.y)) > 0)
						pST[count].s = -pST[count].s;

					pST[count].s -= pSector->ty / 2.0f;
					pST[count].t -= pSector->tx / 2.0f;
				}
				else
				{
					pST[count].s = abs(pNode->u.leaf.poly.p[count].y - top) - pSector->ty;
					pST[count].t = abs(pNode->u.leaf.poly.p[count].x - left) - pSector->tx;
				}

				if (pSector->animate != NULL && pSector->animate->animation == ANIMATE_SCROLL)
				{
					if (pSector->flags & SF_SCROLL_CEILING)
					{
						pST[count].s -= pSector->animate->u.scroll.yoffset;
						pST[count].t += pSector->animate->u.scroll.xoffset;
					}
				}

            pST[count].s *= inv64;
            pST[count].t *= inv64;
			}
		}
	}

	if (pBGRA)
	{
		int	distX, distY, distance;

		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			const auto playerPosition = getPlayerPosition();
			distX = pXYZ[count].x - playerPosition.first;
			distY = pXYZ[count].y - playerPosition.second;

			lightscale = FINENESS;

			auto shadeAmount = getShadeAmount();
			auto& sunVect = getSunVector();

			if (shadeAmount != 0)
			{
				if ((pNode->u.leaf.sector->sloped_ceiling != NULL) &&
					(pNode->u.leaf.sector->sloped_ceiling->flags & SLF_DIRECTIONAL))
				{
					long lo_end = FINENESS-shadeAmount;

					// light scale is based on dot product of surface normal and sun vector
					lightscale = (long)(pNode->u.leaf.sector->sloped_ceiling->plane.a * sunVect.x +
						pNode->u.leaf.sector->sloped_ceiling->plane.b * sunVect.y +
						pNode->u.leaf.sector->sloped_ceiling->plane.a * sunVect.z)>>LOG_FINENESS;

					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range

					lightscale = lo_end + ((lightscale * shadeAmount)>>LOG_FINENESS);

					if (lightscale > FINENESS)
						lightscale = FINENESS;
					else if ( lightscale < 0)
						lightscale = 0;

					pNode->u.leaf.sector->sloped_ceiling->lightscale = lightscale;
				}
			}
			else
			{
				if (pNode->u.leaf.sector->sloped_ceiling != NULL)
					pNode->u.leaf.sector->sloped_ceiling->lightscale = FINENESS;
			}

			distance = DistanceGet(distX, distY);

			if (isFogEnabled())
				paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightscale, 0);
			else
				paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightscale, 0);

			pBGRA[count].r = pBGRA[count].g = pBGRA[count].b = paletteIndex * COLOR_AMBIENT / 64;
			pBGRA[count].a = 255;
		}
	}
}
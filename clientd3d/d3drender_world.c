// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables

extern LPDIRECT3DVERTEXDECLARATION9 decl1dc;
extern LPDIRECT3DVERTEXDECLARATION9 decl2dc;

extern d3d_driver_profile gD3DDriverProfile;
extern d3d_render_pool_new gWorldPool;
extern d3d_render_pool_new gLMapPool;
extern d3d_render_pool_new gWorldPoolStatic;
extern d3d_render_pool_new gLMapPoolStatic;
extern d3d_render_pool_new gWallMaskPool;

extern d3d_render_cache_system gLMapCacheSystem;
extern d3d_render_cache_system gLMapCacheSystemStatic;
extern d3d_render_cache_system gWorldCacheSystem;
extern d3d_render_cache_system gWorldCacheSystemStatic;
extern d3d_render_cache_system gWallMaskCacheSystem;

extern d_light_cache gDLightCache;
extern d_light_cache gDLightCacheDynamic;

extern LPDIRECT3DTEXTURE9 gpNoLookThrough;
extern LPDIRECT3DTEXTURE9 gpDLightOrange;

extern long shade_amount;

extern int gNumCalls;
extern long timeWorld;
extern room_type current_room;
extern int gNumObjects;
extern Vector3D sun_vect;
extern D3DMATRIX view;

static const auto alpha_test_threshold = 128; // Threshold for alpha testing to determine transparency
extern Bool gWireframe;

extern player_info player;

// Interfaces

void D3DRenderWorldDraw(d3d_render_pool_new* pPool, room_type* room, Draw3DParams* params, 
	bool transparent_pass);
void D3DRenderWorldLighting(room_type* room, Draw3DParams* params, room_contents_node* pRNode);
void D3DRenderLMapsPostDraw(BSPnode* tree, Draw3DParams* params, bool transparent_pass);
void D3DRenderLMapsDynamicPostDraw(BSPnode* tree, Draw3DParams* params, bool transparent_pass);

void D3DRenderPacketWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side,
	Bool dynamic);
void D3DRenderPacketFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool dynamic);
void D3DRenderPacketCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool dynamic);

void D3DRenderPacketWallMaskAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side,
	Bool dynamic);
void D3DRenderFloorMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool,
	Bool bDynamic);
void D3DRenderCeilingMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool,
	Bool bDynamic);

void D3DRenderLMapPostFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache, 
	Bool bDynamic);
void D3DRenderLMapPostCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache, 
	Bool bDynamic);
void D3DRenderLMapPostWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side, 
	d_light_cache* pDLightCache, Bool bDynamic);

void D3DRenderFloorExtract(BSPnode* pNode, PDIB pDib, custom_xyz* pXYZ, custom_st* pST,
	custom_bgra* pBGRA);
void D3DRenderCeilingExtract(BSPnode* pNode, PDIB pDib, custom_xyz* pXYZ, custom_st* pST,
	custom_bgra* pBGRA);
int D3DRenderWallExtract(WallData* pWall, PDIB pDib, unsigned int* flags, custom_xyz* pXYZ,
	custom_st* pST, custom_bgra* pBGRA, unsigned int type, int side);

extern bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent);

// Implementations

/**
* The main entry point for rendering the 3d game world.
*/
void D3DRenderWorld(room_type* room, Draw3DParams* params, room_contents_node* pRNode)
{
	// We implement a separate transparent pass to ensure that transparent objects
	// are rendered correctly. This approach avoids issues with depth sorting by first rendering
	// all opaque objects and then rendering all transparent objects in a separate pass.
	// This technique is also applied to draw_world, draw_objects, and light maps.

	SetZBias(gpD3DDevice, ZBIAS_WORLD);
	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	timeWorld = timeGetTime();
	gNumCalls = 0;

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

	D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
	D3DCacheSystemReset(&gWorldCacheSystem);
	D3DRenderWorldDraw(&gWorldPool, room, params, false); // Non-transparent objects pass
	D3DRenderWorldDraw(&gWorldPool, room, params, true);  // Transparent objects pass
	D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);

	// Render the wireframe pass to cover cracks in geometry
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	gWireframe = TRUE;
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	D3DCacheFlush(&gWorldCacheSystemStatic, &gWorldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
	D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);
	gWireframe = FALSE;
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ANTIALIASEDLINEENABLE, FALSE);

	// Finally, draw the solid world
	D3DCacheFlush(&gWorldCacheSystemStatic, &gWorldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
	D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

	timeWorld = timeGetTime() - timeWorld;

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	D3DRenderWorldLighting(room, params, pRNode);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	SetZBias(gpD3DDevice, 1);
}

/**
* Applying static and dynamic lighting to the 3d world
*/
void D3DRenderWorldLighting(room_type* room, Draw3DParams* params, room_contents_node* pRNode)
{
	// draw post lightmaps
	if (config.bDynamicLighting)
	{
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);

		SetZBias(gpD3DDevice, ZBIAS_WORLD);
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl2dc);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, alpha_test_threshold, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_ONE, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
		D3DCacheFlush(&gLMapCacheSystemStatic, &gLMapPoolStatic, 2, D3DPT_TRIANGLESTRIP);

		D3DRenderPoolReset(&gLMapPool, &D3DMaterialLMapDynamicPool);

		D3DRenderLMapsPostDraw(room->tree, params, false);
		D3DRenderLMapsDynamicPostDraw(room->tree, params, false);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE); // Disable depth writing
		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, alpha_test_threshold, D3DCMP_GREATEREQUAL);

		D3DRenderLMapsPostDraw(room->tree, params, true);
		D3DRenderLMapsDynamicPostDraw(room->tree, params, true);

		D3DCacheFill(&gLMapCacheSystem, &gLMapPool, 2);
		D3DCacheFlush(&gLMapCacheSystem, &gLMapPool, 2, D3DPT_TRIANGLESTRIP);

		// Restore states for subsequent rendering
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE); // Restore depth writing
		if (gD3DDriverProfile.bFogEnable)
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE); // Restore fog state
	}
}

/**
* Drawing of the world composed of floors, ceilings and walls
*/
void D3DRenderWorldDraw(d3d_render_pool_new* pPool, room_type* room, Draw3DParams* params, bool transparent_pass)
{
	int count;
	BSPnode* pNode = NULL;
	WallData* pWall;
	Bool bDynamic;

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
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_NORMAL, 1, TRUE);
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_NORMAL, -1, TRUE);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && (((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0)))
				{
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_BELOW, 1, TRUE);
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_BELOW, -1, TRUE);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && (((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2)))
				{
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_ABOVE, 1, TRUE);
					D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_ABOVE, -1, TRUE);
				}
			}

			break;

		case BSPleaftype:
			if (pNode->u.leaf.sector->flags & SF_HAS_ANIMATED)
			{
				D3DRenderPacketFloorAdd(pNode, &gWorldPool, TRUE);
				D3DRenderPacketCeilingAdd(pNode, &gWorldPool, TRUE);
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
void D3DRenderLMapsPostDraw(BSPnode* tree, Draw3DParams* params, bool transparent_pass)
{
	long side;
	float a, b;

	if (!tree)
		return;

	switch (tree->type)
	{
	case BSPleaftype:
		if (tree->u.leaf.sector->flags & SF_HAS_ANIMATED)
		{
			if (transparent_pass)
			{
				D3DRenderLMapPostFloorAdd(tree, &gLMapPool, &gDLightCache, TRUE);
				D3DRenderLMapPostCeilingAdd(tree, &gLMapPool, &gDLightCache, TRUE);
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
			D3DRenderLMapsPostDraw(tree->u.internal.pos_side, params, transparent_pass);
		else
			D3DRenderLMapsPostDraw(tree->u.internal.neg_side, params, transparent_pass);

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
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_NORMAL, side, &gDLightCache, TRUE);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0))
				{
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_BELOW, side, &gDLightCache, TRUE);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2))
				{
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_ABOVE, side, &gDLightCache, TRUE);
				}
			}
		}

		/* lastly, traverse farther side */
		if (side > 0)
			D3DRenderLMapsPostDraw(tree->u.internal.neg_side, params, transparent_pass);
		else
			D3DRenderLMapsPostDraw(tree->u.internal.pos_side, params, transparent_pass);

		return;

	default:
		debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

/*
* Rendering of post lightmaps for the dynamic 3d world.
*/
void D3DRenderLMapsDynamicPostDraw(BSPnode* tree, Draw3DParams* params, bool transparent_pass)
{
	long side;
	float a, b;

	if (!tree)
		return;

	switch (tree->type)
	{
	case BSPleaftype:
		D3DRenderLMapPostFloorAdd(tree, &gLMapPool, &gDLightCacheDynamic, TRUE);
		D3DRenderLMapPostCeilingAdd(tree, &gLMapPool, &gDLightCacheDynamic, TRUE);

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
			D3DRenderLMapsDynamicPostDraw(tree->u.internal.pos_side, params, transparent_pass);
		else
			D3DRenderLMapsDynamicPostDraw(tree->u.internal.neg_side, params, transparent_pass);

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
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_NORMAL, side, &gDLightCacheDynamic, TRUE);
				}

				if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
					|| ((short)pWall->zz1 != (short)pWall->zz0))
				{
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_BELOW, side, &gDLightCacheDynamic, TRUE);
				}

				if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
					|| ((short)pWall->zz3 != (short)pWall->zz2))
				{
					D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_ABOVE, side, &gDLightCacheDynamic, TRUE);
				}
			}
		}

		/* lastly, traverse farther side */
		if (side > 0)
			D3DRenderLMapsDynamicPostDraw(tree->u.internal.neg_side, params, transparent_pass);
		else
			D3DRenderLMapsDynamicPostDraw(tree->u.internal.pos_side, params, transparent_pass);

		return;

	default:
		debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

/*
* Add a floor to the render pool
*/
void D3DRenderPacketFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool bDynamic)
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
void D3DRenderPacketCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool bDynamic)
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
void D3DRenderPacketWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side,
	Bool bDynamic)
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
* Add a wal mask to the render pool
*/
void D3DRenderPacketWallMaskAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type,
	int side, Bool bDynamic)
{
	Sidedef*		pSideDef;
	custom_xyz		xyz[4];
	custom_st		st[4];
	custom_bgra		bgra[MAX_NPTS];
	unsigned int	flags;
	PDIB			pDib;
	int				vertex;
	Bool			bNoVTile = FALSE;
	Bool			bNoLookThrough = FALSE;

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
			bNoVTile = TRUE;
		if (pWall->pos_sidedef->flags & WF_NOLOOKTHROUGH)
			bNoLookThrough = TRUE;
	}

	if (pWall->neg_sidedef)
	{
		if (pWall->neg_sidedef->flags & WF_NO_VTILE)
			bNoVTile = TRUE;
		if (pWall->neg_sidedef->flags & WF_NOLOOKTHROUGH)
			bNoLookThrough = TRUE;
	}

	if (NULL == pDib)
		return;

	if ((pSideDef->flags & WF_NOLOOKTHROUGH) == 0)
	{
		if ((bNoLookThrough == 0) || (bNoVTile == 0))
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

		pPacket = D3DRenderPacketFindMatch(pPool, gpNoLookThrough, NULL, 0, 0, 0);
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
void D3DRenderFloorMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool bDynamic)
{
	Sector* pSector = pNode->u.leaf.sector;
	custom_xyz	xyz[MAX_NPTS];
	custom_bgra	bgra[MAX_NPTS];
	int			vertex;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

	D3DRenderFloorExtract(pNode, NULL, xyz, NULL, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, gpNoLookThrough, NULL, 0, 0, 0);
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
void D3DRenderCeilingMaskAdd(BSPnode* pNode, d3d_render_pool_new* pPool, Bool bDynamic)
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

	pPacket = D3DRenderPacketFindMatch(pPool, gpNoLookThrough, NULL, 0, 0, 0);
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
		current_room.sectors[0].ceiling_height))
	{
		int	vertex, i;

		pPacket = D3DRenderPacketFindMatch(pPool, gpDLightOrange, NULL, 0, 0, 0);
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
* Add light map for floor to the render pool
*/
void D3DRenderLMapPostFloorAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache,
	Bool bDynamic)
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

			gNumObjects++;
		}
	}
}

/*
* Add light map for a ceiling to the render pool.
*/
void D3DRenderLMapPostCeilingAdd(BSPnode* pNode, d3d_render_pool_new* pPool, d_light_cache* pDLightCache,
	Bool bDynamic)
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

			gNumObjects++;
		}
	}
}

/*
* Add light map for a wall to the render pool
*/
void D3DRenderLMapPostWallAdd(WallData* pWall, d3d_render_pool_new* pPool, unsigned int type, int side,
	d_light_cache* pDLightCache, Bool bDynamic)
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
void D3DGeometryBuildNew(room_type *room, d3d_render_pool_new *pPool, bool transparent_pass)
{
	int			count;
	BSPnode		*pNode = NULL;
	WallData	*pWall;

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
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_NORMAL, 1, FALSE);
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_NORMAL, -1, FALSE);
					}

					if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0))
					{
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_BELOW, 1, FALSE);
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_BELOW, -1, FALSE);
					}

					if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2))
					{
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_ABOVE, 1, FALSE);
						D3DRenderPacketWallAdd(pWall, &gWorldPoolStatic, D3DRENDER_WALL_ABOVE, -1, FALSE);
					}
				}

			break;

			case BSPleaftype: // floors and ceilings
				D3DRenderPacketFloorAdd(pNode, &gWorldPoolStatic, FALSE);
				D3DRenderPacketCeilingAdd(pNode, &gWorldPoolStatic, FALSE);
        break;

			default:
        break;
		}
	}

	if (config.bDynamicLighting)
	{
		D3DCacheSystemReset(&gLMapCacheSystemStatic);
		D3DRenderPoolReset(&gLMapPoolStatic, &D3DMaterialLMapDynamicPool);

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
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_NORMAL, 1, &gDLightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_NORMAL, -1, &gDLightCache, FALSE);
						}

						if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
							|| ((short)pWall->zz1 != (short)pWall->zz0))
						{
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_BELOW, 1, &gDLightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_BELOW, -1, &gDLightCache, FALSE);
						}

						if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
							|| ((short)pWall->zz3 != (short)pWall->zz2))
						{
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_ABOVE, 1, &gDLightCache, FALSE);
							D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_ABOVE, -1, &gDLightCache, FALSE);
						}
					}

				break;

				case BSPleaftype:
					D3DRenderLMapPostFloorAdd(pNode, &gLMapPoolStatic, &gDLightCache, FALSE);
					D3DRenderLMapPostCeilingAdd(pNode, &gLMapPoolStatic, &gDLightCache, FALSE);
				break;

				default:
				break;
			}
		}

		D3DCacheFill(&gLMapCacheSystemStatic, &gLMapPoolStatic, 2);
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
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_NORMAL, 1, FALSE);
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_NORMAL, -1, FALSE);
					}

					if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0))
					{
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_BELOW, 1, FALSE);
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_BELOW, -1, FALSE);
					}

					if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2))
					{
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_ABOVE, 1, FALSE);
						D3DRenderPacketWallMaskAdd(pWall, &gWallMaskPool, D3DRENDER_WALL_ABOVE, -1, FALSE);
					}
				}

			break;

			case BSPleaftype:
				if ((pNode->u.leaf.sector->ceiling == NULL) &&
					(pNode->u.leaf.sector->sloped_floor == NULL))
					D3DRenderCeilingMaskAdd(pNode, &gWallMaskPool, FALSE);
			break;

			default:
			break;
		}
	}

	{
		D3DCacheFill(&gWorldCacheSystemStatic, &gWorldPoolStatic, 1);
		D3DCacheFill(&gWallMaskCacheSystem, &gWallMaskPool, 1);
	}
}

/*
* Update the 3d world gemoetry for the 3d world.
*/
void GeometryUpdate(d3d_render_pool_new *pPool, d3d_render_cache_system *pCacheSystem)
{
	u_int				curPacket, curChunk;
	u_int				i, numPackets;
	d3d_render_cache	*pRenderCache = NULL;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	Sector					*pSector;
	int						distX, distY, distance, paletteIndex;
	list_type				list;
	long					lightScale;
	long					lo_end = FINENESS-shade_amount;

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
					distX = pChunk->xyz[i].x - player.x;
					distY = pChunk->xyz[i].y - player.y;

					distance = DistanceGet(distX, distY);

					if (shade_amount != 0)
					{
						long	a, b;

						a = pChunk->plane.a;
						b = pChunk->plane.b;

						if (pChunk->side < 0)
						{
							a = -a;
							b = -b;
						}

						lightScale = (long) (a * sun_vect.x +
										b * sun_vect.y) >> LOG_FINENESS;

						lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range

						lightScale = lo_end + ((lightScale * shade_amount)>>LOG_FINENESS);
						
						if (lightScale > FINENESS)
							lightScale = FINENESS;
						else if ( lightScale < 0)
							lightScale = 0;
					}
					else
						lightScale = FINENESS;

					if (gD3DDriverProfile.bFogEnable)
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
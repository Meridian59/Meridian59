// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables

extern d3d_driver_profile gD3DDriverProfile;

extern LPDIRECT3DTEXTURE9 gpDLightWhite;
extern LPDIRECT3DTEXTURE9 gpBackBufferTex[16];

extern font_3d gFont;
extern Bool gWireframe;

extern room_type current_room;

extern Draw3DParams* p;

// Interfaces

float D3DRenderFogEndCalc(d3d_render_chunk_new* pChunk);

// Implementations

/**
 * Configures the rendering device to use modulated color and alpha stages for world material rendering.
 */
Bool D3DMaterialWorldPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for a world packet and sets it for rendering.
 */
Bool D3DMaterialWorldPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

  // The else clause is handy for testing; it draws the font texture everywhere
  if (1)
  {
    if (pPacket->pTexture)
      pTexture = pPacket->pTexture;
    else if (pPacket->pDib)
      pTexture = D3DCacheTextureLookupSwizzled(&pCacheSystem->textureCache, pPacket, 0);
  }
  else 
    pTexture = gFont.pTexture;

	if (pTexture)
		IDirect3DDevice9_SetTexture(
         gpD3DDevice, 0, (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}

/**
 * Configures rendering states for a dynamic chunk in the world, including transformations and fog.
 */
Bool D3DMaterialWorldDynamicChunk(d3d_render_chunk_new *pChunk)
{
	if ((pChunk->flags & D3DRENDER_WORLD_OBJ))
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	if (gWireframe)
	{
		if (pChunk->pSector == &current_room.sectors[0])
			return FALSE;
	}

	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
				SetZBias(gpD3DDevice, 0);
			else
            SetZBias(gpD3DDevice, ZBIAS_WORLD);
		}
	}
	else
	{
		SetZBias(gpD3DDevice, ZBIAS_WORLD);
	}

	// Clamp texture V axis if vertical tiling is disabled 
	auto state = (pChunk->flags & D3DRENDER_NO_VTILE) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSV, state);

	// Clamp texture U axis if horizontal tiling is disabled
	state = (pChunk->flags & D3DRENDER_NO_HTILE) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSU, state);

	if (gD3DDriverProfile.bFogEnable)
	{
		float	end = D3DRenderFogEndCalc(pChunk);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	return TRUE;
}

/**
 * Configures rendering states for a static chunk in the world, excluding certain animated sectors.
 */
Bool D3DMaterialWorldStaticChunk(d3d_render_chunk_new *pChunk)
{
	if (gWireframe)
	{
		if (pChunk->pSector == &current_room.sectors[0])
		{
			if ((pChunk->pSector->ceiling == current_room.sectors[0].ceiling) &&
				(pChunk->pSector->ceiling != NULL))
				return FALSE;
		}
	}

	if (pChunk->pSector)
		if (pChunk->pSector->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSectorPos)
		if (pChunk->pSectorPos->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSectorNeg)
		if (pChunk->pSectorNeg->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSideDef)
		if (pChunk->pSideDef->flags & WF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
				SetZBias(gpD3DDevice, 0);
			else
				SetZBias(gpD3DDevice, ZBIAS_WORLD);
		}
	}
	else
	{
		SetZBias(gpD3DDevice, ZBIAS_WORLD);
	}

	// Clamp texture V axis if vertical tiling is disabled 
	auto state = (pChunk->flags & D3DRENDER_NO_VTILE) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSV, state);

	// Clamp texture U axis if horizontal tiling is disabled
	state = (pChunk->flags & D3DRENDER_NO_HTILE) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSU, state);

	if (gD3DDriverProfile.bFogEnable)
	{
		float	end = D3DRenderFogEndCalc(pChunk);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	return TRUE;
}

/**
 * Configures the rendering device for wall mask materials, similar to world materials.
 */
Bool D3DMaterialWallMaskPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	return TRUE;
}

/**
 * Configures rendering states for mask chunks, including culling mode and Z-bias.
 */
Bool D3DMaterialMaskChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->flags & D3DRENDER_NOCULL)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	else
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	SetZBias(gpD3DDevice, pChunk->zBias);

	return TRUE;
}

/**
 * A placeholder function for rendering chunks with no specific material.
 */
Bool D3DMaterialNone(d3d_render_chunk_new *pPool)
{
	return TRUE;
}

/**
 * Configures the rendering device for dynamic light map materials, setting up multiple texture stages.
 */
Bool D3DMaterialLMapDynamicPool(d3d_render_pool_new *pPool)
{
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	IDirect3DDevice9_SetTexture(gpD3DDevice, 0, (IDirect3DBaseTexture9 *) gpDLightWhite);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, 0);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_TEXTURE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_SELECTARG2, D3DTA_CURRENT, D3DTA_TEXTURE);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for a dynamic light map packet and sets it for rendering.
 */
Bool D3DMaterialLMapDynamicPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

	if (pPacket->pTexture)
		pTexture = pPacket->pTexture;
	else if (pPacket->pDib)
		pTexture = D3DCacheTextureLookupSwizzled(&pCacheSystem->textureCache, pPacket, 0);

	if (pTexture)
		IDirect3DDevice9_SetTexture(gpD3DDevice, 1, (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}
/**
 * Configures rendering states for a dynamic light map chunk, including Z-bias adjustments.
 */
Bool D3DMaterialLMapDynamicChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
				SetZBias(gpD3DDevice, 0);
			else
				SetZBias(gpD3DDevice, ZBIAS_WORLD);
		}
	}
	else
		SetZBias(gpD3DDevice, ZBIAS_WORLD);

	return TRUE;
}


/**
 * Configures rendering states for a static light map chunk, including Z-bias adjustments and exclusions for animated sectors.
 */
Bool D3DMaterialLMapStaticChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
				SetZBias(gpD3DDevice, 0);
			else
				SetZBias(gpD3DDevice, ZBIAS_WORLD);
		}
	}
	else
      SetZBias(gpD3DDevice, ZBIAS_WORLD);

	if (pChunk->pSector)
		if (pChunk->pSector->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSectorPos)
		if (pChunk->pSectorPos->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSectorNeg)
		if (pChunk->pSectorNeg->flags & SF_HAS_ANIMATED)
			return FALSE;

	if (pChunk->pSideDef)
		if (pChunk->pSideDef->flags & WF_HAS_ANIMATED)
			return FALSE;

	return TRUE;
}

/**
 * Configures the rendering device to use modulated color and alpha stages for object material rendering.
 */
Bool D3DMaterialObjectPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for an object packet and sets it for rendering.
 */
Bool D3DMaterialObjectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

	if (pPacket->pTexture)
		pTexture = pPacket->pTexture;
	else if (pPacket->pDib)
		pTexture = D3DCacheTextureLookup(&pCacheSystem->textureCache, pPacket, pPacket->effect);

	if (pTexture)
		IDirect3DDevice9_SetTexture(gpD3DDevice, 0, (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}

/**
 * Configures rendering states for an object chunk, including transformations, Z-bias, and alpha settings.
 */
Bool D3DMaterialObjectChunk(d3d_render_chunk_new *pChunk)
{

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	SetZBias(gpD3DDevice, pChunk->zBias);

	if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT25)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS25 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT50)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT75)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS75 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_DITHERTRANS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_DITHERINVIS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);

	if (pChunk->isTargeted)
	{
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_DIFFUSE, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	}
	else
	{
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	}

	if (gD3DDriverProfile.bFogEnable)
	{
		float	end;

		end = D3DRenderFogEndCalc(pChunk);
      IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	return TRUE;
}

/**
 * Configures the rendering device for invisible object materials, including texture sampling and filter settings.
 */
Bool D3DMaterialObjectInvisiblePool(d3d_render_pool_new *pPool)
{
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_SELECTARG2, 0, D3DTA_TEXTURE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_SELECTARG1, D3DTA_CURRENT, 0);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_MINFILTER, D3DTEXF_POINT);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1,
                                         D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	IDirect3DDevice9_SetTexture(
      gpD3DDevice, 1, (IDirect3DBaseTexture9 *) gpBackBufferTex[0]);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for an invisible object packet and sets it for rendering.
 */
Bool D3DMaterialObjectInvisiblePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

	if (pPacket->pTexture)
		pTexture = pPacket->pTexture;
	else if (pPacket->pDib)
		pTexture = D3DCacheTextureLookup(&pCacheSystem->textureCache, pPacket, pPacket->effect);

	if (pTexture)
		IDirect3DDevice9_SetTexture(gpD3DDevice, 0, (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}

/**
 * Configures rendering states for an invisible object chunk, including transformations, Z-bias, and alpha settings.
 */
Bool D3DMaterialObjectInvisibleChunk(d3d_render_chunk_new *pChunk)
{
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	SetZBias(gpD3DDevice, pChunk->zBias);

	if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT25)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS25 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT50)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_TRANSLUCENT75)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS75 - 1);
	else if (GetDrawingEffect(pChunk->flags) == OF_DITHERTRANS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      1);
	else if (GetDrawingEffect(pChunk->flags) == OF_DITHERINVIS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);

	return TRUE;
}

/**
 * Configures the rendering device for effect materials, including alpha blending and color stages.
 */
Bool D3DMaterialEffectPool(d3d_render_pool_new *pPool)
{
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_DIFFUSE, 0);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_DIFFUSE, 0);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for an effect packet and sets it for rendering.
 */
Bool D3DMaterialEffectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

	if (pPacket->pTexture)
		pTexture = pPacket->pTexture;

	if (pTexture)
		IDirect3DDevice9_SetTexture(
         gpD3DDevice, 0, (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}

/**
 * Configures the rendering device for blur materials, setting up color and alpha stages.
 */
Bool D3DMaterialBlurPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	return TRUE;
}

/**
 * Binds the appropriate texture to the device for a blur packet and sets it for rendering.
 */
Bool D3DMaterialBlurPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	LPDIRECT3DTEXTURE9	pTexture = NULL;

	if (pPacket->pTexture)
		pTexture = pPacket->pTexture;

	if (pTexture)
		IDirect3DDevice9_SetTexture(gpD3DDevice, 0,
                                  (IDirect3DBaseTexture9 *) pTexture);

	return TRUE;
}

/**
 * A placeholder function for rendering blur chunks with no specific additional material setup required.
 */
Bool D3DMaterialBlurChunk(d3d_render_chunk_new *pChunk)
{
	return TRUE;
}

/**
 * Configures the rendering device for particle materials, including alpha blending and color stages.
 */
Bool D3DMaterialParticlePool(d3d_render_pool_new *pPool)
{
   IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, 0, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, 0, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	return TRUE;
}

/**
 * A placeholder function for rendering particle packets with no specific material setup required.
 */
Bool D3DMaterialParticlePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	return TRUE;
}

/**
 * Configures rendering states for a particle chunk, including transformations.
 */
Bool D3DMaterialParticleChunk(d3d_render_chunk_new *pChunk)
{
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	return TRUE;
}

/**
 * A placeholder function for rendering effect chunks with no specific additional material setup required.
 */
Bool D3DMaterialEffectChunk(d3d_render_chunk_new *pChunk)
{
	return TRUE;
}

/**
 * Calculates the end distance of the fog effect for a given render chunk, based on the lighting conditions
 * within the sector or side of the chunk.
 */
float D3DRenderFogEndCalc(d3d_render_chunk_new *pChunk)
{
	float	end, light;

	end = LIGHT_NEUTRAL;

	if (pChunk->pSector == NULL)
	{
		if (pChunk->side > 0)
		{
			if (pChunk->pSectorPos)
				light = pChunk->pSectorPos->light;
			else
				light = LIGHT_NEUTRAL;
		}
		else
		{
			if (pChunk->pSectorNeg)
				light = pChunk->pSectorNeg->light;
			else
				light = LIGHT_NEUTRAL;
		}
	}
	else
		light = pChunk->pSector->light;

	// these numbers appear to be pulled out of thin air, but they aren't.  see
	// GetLightPalette(), LightChanged3D(), and LIGHT_INDEX() for more info
	// note: sectors with the no ambient flag attenuate twice as fast in the old client.
	// bug or not, it needs to be emulated here...
	if (pChunk->flags & D3DRENDER_NOAMBIENT)
		end = (16384 + (light * FINENESS) + (p->viewer_light * 64));
	else
		end = (32768 + (max(0, light - LIGHT_NEUTRAL) * FINENESS) + (p->viewer_light * 64) +
		(current_room.ambient_light * FINENESS));

	return end;
}

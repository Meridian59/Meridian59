// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

int	gNumCalls;
extern int						gNumDPCalls;
extern d3d_render_cache			gTempCache;

extern LPDIRECT3DTEXTURE9		gpDLightAmbient;
extern LPDIRECT3DTEXTURE9		gpDLightWhite;
extern LPDIRECT3DTEXTURE9		gpDLightOrange;
extern d3d_driver_profile		gD3DDriverProfile;

void	D3DCacheLock(d3d_render_cache *pCache);

static int getTextureSizeBytes(const D3DSURFACE_DESC &surface)
{
   int size = surface.Width * surface.Height;

   switch(surface.Format)
   {
   case D3DFMT_A8R8G8B8:
		size *= 4;
	case D3DFMT_A4R4G4B4:
		size *= 2;
	}
   return size;
}

void D3DCacheInit(d3d_render_cache *pCache, int size, int numStages, DWORD flags)
{
	IDirect3DDevice9_CreateIndexBuffer(gpD3DDevice, size * sizeof(custom_index),
                                      flags, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
                                      &pCache->indexBuffer.pIBuffer, NULL);

	IDirect3DDevice9_CreateVertexBuffer(gpD3DDevice, size *
                                       sizeof(custom_xyz), flags, D3DFVF_XYZ,
                                       D3DPOOL_DEFAULT, &pCache->xyzBuffer.pVBuffer, NULL);
   
	IDirect3DDevice9_CreateVertexBuffer(gpD3DDevice, size *
                                       sizeof(custom_st), flags, D3DFVF_TEX0,
                                       D3DPOOL_DEFAULT, &pCache->stBuffer[0].pVBuffer, NULL);
   
	IDirect3DDevice9_CreateVertexBuffer(gpD3DDevice, size *
                                       sizeof(custom_st), flags, D3DFVF_TEX1,
                                       D3DPOOL_DEFAULT, &pCache->stBuffer[1].pVBuffer, NULL);
   
	IDirect3DDevice9_CreateVertexBuffer(gpD3DDevice, size *
                                       sizeof(custom_bgra), flags, D3DFVF_DIFFUSE,
                                       D3DPOOL_DEFAULT, &pCache->bgraBuffer.pVBuffer, NULL);
   
	pCache->numPackets = 0;
	pCache->bgraBuffer.curIndex = 0;
	pCache->xyzBuffer.curIndex = 0;
	pCache->indexBuffer.curIndex = 0;
	pCache->size = size;
}

void D3DCacheShutdown(d3d_render_cache *pCache)
{
	if (pCache->indexBuffer.pIBuffer)
		IDirect3DIndexBuffer9_Release(pCache->indexBuffer.pIBuffer);

	if (pCache->xyzBuffer.pVBuffer)
		IDirect3DVertexBuffer9_Release(pCache->xyzBuffer.pVBuffer);

	if (pCache->stBuffer[0].pVBuffer)
		IDirect3DVertexBuffer9_Release(pCache->stBuffer[0].pVBuffer);

	if (pCache->stBuffer[1].pVBuffer)
		IDirect3DVertexBuffer9_Release(pCache->stBuffer[1].pVBuffer);

	if (pCache->bgraBuffer.pVBuffer)
		IDirect3DVertexBuffer9_Release(pCache->bgraBuffer.pVBuffer);

	pCache->numPackets = 0;
	pCache->bgraBuffer.curIndex = 0;
	pCache->xyzBuffer.curIndex = 0;
	pCache->indexBuffer.curIndex = 0;
}

LPDIRECT3DTEXTURE9 D3DCacheTextureLookupSwizzled(d3d_texture_cache *pTextureCache, d3d_render_packet_new *pPacket,
												 int effect)
{
	LPDIRECT3DTEXTURE9		pTexture = NULL;
	D3DSURFACE_DESC			surfDesc;
	d3d_texture_cache_entry	*pTexEntry;
	list_type				list;
	int	curTex = 0;

	for (list = pTextureCache->textureList; list != NULL; list = list->next)
	{
		pTexEntry = (d3d_texture_cache_entry *)list->data;

		if ((pPacket->pDib->uniqueID == pTexEntry->pDibID) &&
			(pPacket->pDib->uniqueID2 == pTexEntry->pDibID2) &&
			(pPacket->pDib->frame == pTexEntry->frame))
		{
			if ((pPacket->xLat0 == pTexEntry->xLat0) &&
				(pPacket->xLat0 == pTexEntry->xLat0) &&
				(effect == pTexEntry->effects))
			{
				return pTexEntry->pTexture;
			}
		}
	}

	while (pTextureCache->size > pTextureCache->max)
	{
		pTexEntry = (d3d_texture_cache_entry *)list_first_item(pTextureCache->textureList);

		if (pTexEntry)
		{
			if (pTexEntry->pTexture)
				IDirect3DTexture9_Release(pTexEntry->pTexture);

			pTexEntry->pTexture = NULL;
			pTextureCache->size -= pTexEntry->size;
			free(pTextureCache->textureList->data);
			pTextureCache->textureList = list_delete_first(pTextureCache->textureList);
		}
	}

	pTexture = D3DRenderTextureCreateFromBGFSwizzled(pPacket->pDib, pPacket->xLat0,
			pPacket->xLat1, effect);

	if (NULL == pTexture)
		return NULL;

	pTexEntry = (d3d_texture_cache_entry *)D3DRenderMalloc(sizeof(d3d_texture_cache_entry));
	assert(pTexEntry);

	IDirect3DTexture9_GetLevelDesc(pTexture, 0, &surfDesc);

	pTexEntry->effects = effect;
	pTexEntry->pDibID = pPacket->pDib->uniqueID;
	pTexEntry->pDibID2 = pPacket->pDib->uniqueID2;
	pTexEntry->frame = pPacket->pDib->frame;
	pTexEntry->pTexture = pTexture;
	pTexEntry->xLat0 = pPacket->xLat0;
	pTexEntry->xLat1 = pPacket->xLat1;
	pTexEntry->size = getTextureSizeBytes(surfDesc);

	pTextureCache->textureList = list_add_item(pTextureCache->textureList, pTexEntry);
	pTextureCache->size += pTexEntry->size;

	return pTexEntry->pTexture;
}

LPDIRECT3DTEXTURE9 D3DCacheTextureLookup(d3d_texture_cache *pTextureCache, d3d_render_packet_new *pPacket,
										 int effect)
{
	LPDIRECT3DTEXTURE9		pTexture = NULL;
	D3DSURFACE_DESC			surfDesc;
	d3d_texture_cache_entry	*pTexEntry;
	list_type				list;
	int	curTex = 0;

	for (list = pTextureCache->textureList; list != NULL; list = list->next)
	{
		pTexEntry = (d3d_texture_cache_entry *)list->data;

		if ((pPacket->pDib->uniqueID == pTexEntry->pDibID) &&
			(pPacket->pDib->uniqueID2 == pTexEntry->pDibID2) &&
			(pPacket->pDib->frame == pTexEntry->frame))
		{
			if ((pPacket->xLat0 == pTexEntry->xLat0) &&
				(pPacket->xLat0 == pTexEntry->xLat0) &&
				(effect == pTexEntry->effects))
			{
				return pTexEntry->pTexture;
			}
		}
	}

	while (pTextureCache->size > pTextureCache->max)
	{
		pTexEntry = (d3d_texture_cache_entry *)list_first_item(pTextureCache->textureList);

		if (pTexEntry)
		{
			if (pTexEntry->pTexture)
				IDirect3DTexture9_Release(pTexEntry->pTexture);

			pTexEntry->pTexture = NULL;
			pTextureCache->size -= pTexEntry->size;
			free(pTextureCache->textureList->data);
			pTextureCache->textureList = list_delete_first(pTextureCache->textureList);
		}
	}

	pTexture = D3DRenderTextureCreateFromBGF(pPacket->pDib, pPacket->xLat0,
                                            pPacket->xLat1, effect);

	if (NULL == pTexture)
		return NULL;

	pTexEntry = (d3d_texture_cache_entry *)D3DRenderMalloc(sizeof(d3d_texture_cache_entry));
	assert(pTexEntry);

	IDirect3DTexture9_GetLevelDesc(pTexture, 0, &surfDesc);

	pTexEntry->effects = effect;
	pTexEntry->pDibID = pPacket->pDib->uniqueID;
	pTexEntry->pDibID2 = pPacket->pDib->uniqueID2;
	pTexEntry->frame = pPacket->pDib->frame;
	pTexEntry->pTexture = pTexture;
	pTexEntry->xLat0 = pPacket->xLat0;
	pTexEntry->xLat1 = pPacket->xLat1;
	pTexEntry->size = getTextureSizeBytes(surfDesc);

	pTextureCache->textureList = list_add_item(pTextureCache->textureList, pTexEntry);
	pTextureCache->size += pTexEntry->size;

	return pTexEntry->pTexture;
}

void D3DCacheReset(d3d_render_cache *pRenderCache)
{
	int	i;

	pRenderCache->xyzBuffer.curIndex = 0;

	for (i = 0; i < TEMP_NUM_STAGES; i++)
		pRenderCache->stBuffer[i].curIndex = 0;

	pRenderCache->bgraBuffer.curIndex = 0;
	pRenderCache->indexBuffer.curIndex = 0;
	pRenderCache->curPacket = 0;
	pRenderCache->numPackets = 0;
}

void D3DCacheSystemInit(d3d_render_cache_system *pCacheSystem, int texCacheSize)
{
	d3d_render_cache *pRenderCache = NULL;

	memset(pCacheSystem, 0, sizeof(pCacheSystem));
	
	pRenderCache = (d3d_render_cache *)D3DRenderMalloc(sizeof(d3d_render_cache));

	if (pRenderCache)
	{
		pCacheSystem->renderCacheList = list_create(pRenderCache);
		pCacheSystem->numCaches = 1;
		pCacheSystem->pCurCache = pRenderCache;
		pCacheSystem->curCache = pCacheSystem->renderCacheList;
		//pCacheSystem->textureCache.curIndex = 0;
		pCacheSystem->textureCache.textureList = NULL;
		pCacheSystem->textureCache.max = texCacheSize;
		D3DCacheInit((d3d_render_cache *)pRenderCache, TEMP_CACHE_MAX,
			TEMP_NUM_STAGES, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY |
			gD3DDriverProfile.vertexProcessFlag);
	}
}

void D3DCacheSystemShutdown(d3d_render_cache_system *pCacheSystem)
{
	d3d_texture_cache_entry	*pTexEntry;
	list_type	list;
	d3d_render_cache *pRenderCache = NULL;

	// first free all textures
	for (list = pCacheSystem->textureCache.textureList; list != NULL; list = list->next)
	{
		pTexEntry = (d3d_texture_cache_entry *)list->data;

		if (pTexEntry->pTexture)
			IDirect3DTexture9_Release(pTexEntry->pTexture);
	}

	list_destroy(pCacheSystem->textureCache.textureList);

	// now destroy the vertex buffers
	for (list = pCacheSystem->renderCacheList; list != NULL; list = list->next)
	{
		pRenderCache = (d3d_render_cache *)list->data;

		if (pRenderCache)
			D3DCacheShutdown(pRenderCache);
	}

	list_destroy(pCacheSystem->renderCacheList);

	memset(pCacheSystem, 0, sizeof(d3d_render_cache_system));
}

void D3DCacheSystemReset(d3d_render_cache_system *pCacheSystem)
{
	list_type	list;

	for (list = pCacheSystem->renderCacheList; list != NULL; list = list->next)
	{
		CACHE_RESET((d3d_render_cache *) list->data);
	}
	pCacheSystem->pCurCache =
      (d3d_render_cache *) list_first_item(pCacheSystem->renderCacheList);
	pCacheSystem->curCache = pCacheSystem->renderCacheList;
}

d3d_render_cache *D3DCacheSystemSwap(d3d_render_cache_system *pCacheSystem)
{
	list_type			list = pCacheSystem->renderCacheList;
	d3d_render_cache	*pRenderCache = NULL;

	if (pCacheSystem->curCache->next)
	{
		pCacheSystem->curCache = pCacheSystem->curCache->next;

		return (d3d_render_cache *)pCacheSystem->curCache->data;
	}
	else
	{
		pRenderCache = (d3d_render_cache *)D3DRenderMalloc(sizeof(d3d_render_cache));

		if (pRenderCache)
		{
			D3DCacheInit(pRenderCache, TEMP_CACHE_MAX, TEMP_NUM_STAGES, D3DUSAGE_DYNAMIC |
				D3DUSAGE_WRITEONLY | gD3DDriverProfile.vertexProcessFlag);
			list_add_item(pCacheSystem->renderCacheList, pRenderCache);
			pCacheSystem->curCache = pCacheSystem->curCache->next;
			CACHE_RESET(pRenderCache);
			pCacheSystem->pCurCache = pRenderCache;
			pCacheSystem->numCaches++;

			return pRenderCache;
		}
		else
			return NULL;
	}
}

void D3DCacheXYZAdd(d3d_render_cache *pRenderCache, float x, float y, float z)
{
	if (pRenderCache->xyzBuffer.curIndex < pRenderCache->size)
	{
		pRenderCache->xyzBuffer.u.pXYZ->x = x;
		pRenderCache->xyzBuffer.u.pXYZ->y = y;
		pRenderCache->xyzBuffer.u.pXYZ->z = z;
		pRenderCache->xyzBuffer.u.pXYZ++;
		pRenderCache->xyzBuffer.curIndex++;
	}
}

void D3DCacheSTAdd(d3d_render_cache *pRenderCache, int stage, float s, float t)
{
	if (pRenderCache->stBuffer[stage].curIndex < pRenderCache->size)
	{
		pRenderCache->stBuffer[stage].u.pST->s = s;
		pRenderCache->stBuffer[stage].u.pST->t = t;
		pRenderCache->stBuffer[stage].u.pST++;
		pRenderCache->stBuffer[stage].curIndex++;
	}
}

void D3DCacheBGRAAdd(d3d_render_cache *pRenderCache, int b, int g, int r, int a)
{
	if (pRenderCache->bgraBuffer.curIndex < pRenderCache->size)
	{
		pRenderCache->bgraBuffer.u.pBGRA->b = b;
		pRenderCache->bgraBuffer.u.pBGRA->g = g;
		pRenderCache->bgraBuffer.u.pBGRA->r = r;
		pRenderCache->bgraBuffer.u.pBGRA->a = a;
		pRenderCache->bgraBuffer.u.pBGRA++;
		pRenderCache->bgraBuffer.curIndex++;
	}
}

void D3DCacheIndexAdd(d3d_render_cache *pRenderCache, int index)
{
	if (pRenderCache->indexBuffer.curIndex < pRenderCache->size)
	{
		*(pRenderCache->indexBuffer.pIndex) = index;
		pRenderCache->indexBuffer.pIndex++;
		pRenderCache->indexBuffer.curIndex++;
	}
}

void D3DCacheBGRASet(d3d_render_cache *pRenderCache, int index, int b, int g, int r, int a)
{
	if (index < (int) pRenderCache->size)
	{
		pRenderCache->bgraBuffer.u.pBGRA[index].b = b;
		pRenderCache->bgraBuffer.u.pBGRA[index].g = g;
		pRenderCache->bgraBuffer.u.pBGRA[index].r = r;
		pRenderCache->bgraBuffer.u.pBGRA[index].a = a;
	}
}

void D3DCacheLock(d3d_render_cache *pCache)
{
	int	i;

	IDirect3DVertexBuffer9_Lock((pCache)->xyzBuffer.pVBuffer,
                               0, TEMP_CACHE_MAX * sizeof(custom_xyz),
                               (void **)&(pCache)->xyzBuffer.u.pXYZ,
                               D3DLOCK_DISCARD);
	for (i = 0; i < TEMP_NUM_STAGES; i++)
		IDirect3DVertexBuffer9_Lock((pCache)->stBuffer[i].pVBuffer,
                                  0, TEMP_CACHE_MAX * sizeof(custom_st),
                                  (void **)&(pCache)->stBuffer[i].u.pST,
                                  D3DLOCK_DISCARD);
	IDirect3DVertexBuffer9_Lock((pCache)->bgraBuffer.pVBuffer,
                               0, TEMP_CACHE_MAX * sizeof(custom_bgra),
                               (void **)&(pCache)->bgraBuffer.u.pBGRA,
                               D3DLOCK_DISCARD);
	IDirect3DVertexBuffer9_Lock((pCache)->indexBuffer.pIBuffer,
                               0, TEMP_CACHE_MAX * sizeof(custom_index),
                               (void **)&(pCache)->indexBuffer.pIndex,
                               D3DLOCK_DISCARD);
}

void D3DCacheFill(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool, int numStages)
{
	u_int				curPacket, curChunk, count, indexOffset, numPackets;
	d3d_render_cache	*pRenderCache = (d3d_render_cache *)pCacheSystem->curCache->data;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	list_type				list;

	CACHE_RESET(pRenderCache);
	CACHE_LOCK(pRenderCache);

	for (list = pPool->renderPacketList; list != pPool->curPacketList->next; list = list->next)
	{
		if (list == pPool->curPacketList)
			numPackets = pPool->curPacket;
		else
			numPackets = pPool->size;

		pPacket = (d3d_render_packet_new *)list->data;

		for (curPacket = 0; curPacket < numPackets; curPacket++, pPacket++)
		{
			for (curChunk = 0; curChunk < pPacket->curChunk; curChunk++)
			{
				pChunk = &pPacket->renderChunks[curChunk];

				if ((pRenderCache->indexBuffer.curIndex + pChunk->numIndices) >= pRenderCache->size)
				{
					CACHE_UNLOCK(pRenderCache);

					pRenderCache = D3DCacheSystemSwap(pCacheSystem);
					if (NULL == pRenderCache)
						return;

					CACHE_LOCK(pRenderCache);
				}

				pChunk->pRenderCache = pRenderCache;
				pChunk->startIndex = pRenderCache->indexBuffer.curIndex;
				indexOffset = pRenderCache->xyzBuffer.curIndex;

				for (count = 0; count < pChunk->numVertices; count++)
				{
					CACHE_XYZ_ADD(pRenderCache, pChunk->xyz[count].x, pChunk->xyz[count].z, pChunk->xyz[count].y);

					switch (numStages)
					{
						case 1:
							CACHE_ST_ADD(pRenderCache, 0, pChunk->st0[count].s, pChunk->st0[count].t);
						break;

						case 2:
							CACHE_ST_ADD(pRenderCache, 0, pChunk->st0[count].s, pChunk->st0[count].t);
							CACHE_ST_ADD(pRenderCache, 1, pChunk->st1[count].s, pChunk->st1[count].t);
						break;
					}

					CACHE_BGRA_ADD(pRenderCache, pChunk->bgra[count].b, pChunk->bgra[count].g, pChunk->bgra[count].r,
						pChunk->bgra[count].a);
				}

				for (count = 0; count < pChunk->numIndices; count++)
				{
					CACHE_INDEX_ADD(pRenderCache, pChunk->indices[count] + indexOffset);
				}
			}
		}
	}

	if (pRenderCache)
		CACHE_UNLOCK(pRenderCache);
}

void D3DCacheFlush(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool, int numStages,
				   int type)
{
	u_int				curPacket, curChunk, numPackets;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	d3d_render_cache		*pRenderCache = NULL;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	list_type				list;
	int						i;

	// call material function for this pool
	if (FALSE == pPool->pMaterialFctn(pPool))
		return;

	for (list = pPool->renderPacketList; list != pPool->curPacketList->next; list = list->next)
	{
		pPacket = (d3d_render_packet_new *)list->data;

		if (list == pPool->curPacketList)
			numPackets = pPool->curPacket;
		else
			numPackets = pPool->size;

		for (curPacket = 0; curPacket < numPackets; curPacket++, pPacket++)
		{
			// call material function for this packet
			if (FALSE == pPacket->pMaterialFctn(pPacket, pCacheSystem))
				continue;

			for (curChunk = 0; curChunk < pPacket->curChunk; curChunk++)
			{
				pChunk = &pPacket->renderChunks[curChunk];

				// call material function for this chunk
				if (FALSE == pChunk->pMaterialFctn(pChunk))
					continue;

				if (pRenderCache != pChunk->pRenderCache)
				{
					pRenderCache = pChunk->pRenderCache;
					D3DRENDER_SET_STREAMS(gpD3DDevice, pRenderCache, numStages);
				}

				IDirect3DDevice9_DrawIndexedPrimitive(gpD3DDevice,
                                                  (D3DPRIMITIVETYPE) type,
                                                  0,
                                                  pChunk->startIndex,
                                                  pChunk->numIndices,
                                                  pChunk->startIndex,
                                                  pChunk->numPrimitives);

				gNumVertices += pChunk->numIndices;
				gNumDPCalls++;
			}
		}
	}

	// now decrement reference count for these textures
	for (i = 0; i < numStages; i++)
	{
		IDirect3DDevice9_SetTexture(gpD3DDevice, i, NULL);
	}

	D3DRENDER_CLEAR_STREAMS(gpD3DDevice, numStages);
}

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DCACHE_H__
#define __D3DCACHE_H__

#define TEMP_CACHE_MAX			(50000)
#define TEX_CACHE_MAX			(3000)
#define TEMP_NUM_STAGES			(2)
#define POOL_SIZE				(64)
#define PACKET_SIZE				(32)

#if 0
#define CACHE_RESET(_pCache)	\
do	\
{	\
	int	i;	\
	(_pCache)->xyzBuffer.curIndex = 0;	\
	for (i = 0; i < TEMP_NUM_STAGES; i++)	\
		(_pCache)->stBuffer[i].curIndex = 0;	\
	(_pCache)->bgraBuffer.curIndex = 0;	\
	(_pCache)->indexBuffer.curIndex = 0;	\
	(_pCache)->curPacket = 0;	\
	(_pCache)->numPackets = 0;	\
	(_pCache)->textureCache.curIndex = 0;	\
} while (0)

#define CACHE_XYZ_ADD(_pCache, _x, _y, _z)	\
do	\
{	\
	if ((_pCache)->xyzBuffer.curIndex < _pCache->size)	\
	{	\
		(_pCache)->xyzBuffer.u.pXYZ->x = _x;	\
		(_pCache)->xyzBuffer.u.pXYZ->y = _y;	\
		(_pCache)->xyzBuffer.u.pXYZ->z = _z;	\
		(_pCache)->xyzBuffer.u.pXYZ++;	\
		(_pCache)->xyzBuffer.curIndex++;	\
	}	\
} while(0)

#define CACHE_BGRA_ADD(_pCache, _b, _g, _r, _a)	\
do	\
{	\
	if ((_pCache)->bgraBuffer.curIndex < _pCache->size)	\
	{	\
		(_pCache)->bgraBuffer.u.pBGRA->b = _b;	\
		(_pCache)->bgraBuffer.u.pBGRA->g = _g;	\
		(_pCache)->bgraBuffer.u.pBGRA->r = _r;	\
		(_pCache)->bgraBuffer.u.pBGRA->a = _a;	\
		(_pCache)->bgraBuffer.u.pBGRA++;	\
		(_pCache)->bgraBuffer.curIndex++;	\
	}	\
} while(0)

#define CACHE_ST_ADD(_pCache, _stage, _s, _t)	\
do	\
{	\
	if ((_pCache)->stBuffer[_stage].curIndex < _pCache->size)	\
	{	\
		(_pCache)->stBuffer[_stage].u.pST->s = _s;	\
		(_pCache)->stBuffer[_stage].u.pST->t = _t;	\
		(_pCache)->stBuffer[_stage].u.pST++;	\
		(_pCache)->stBuffer[_stage].curIndex++;	\
	}	\
} while(0)

#define CACHE_INDEX_ADD(_pCache, _index)	\
do	\
{	\
	if ((_pCache)->indexBuffer.curIndex < _pCache->size)	\
	{	\
		*((_pCache)->indexBuffer.pIndex) = _index;	\
		(_pCache)->indexBuffer.pIndex++;	\
		(_pCache)->indexBuffer.curIndex++;	\
	}	\
} while(0)
#else
#define CACHE_RESET(_pCache) D3DCacheReset(_pCache)
#define CACHE_LOCK(_pCache)	D3DCacheLock(_pCache)

#define CACHE_XYZ_ADD(_pCache, _x, _y, _z) D3DCacheXYZAdd(_pCache, (_x), (_y), (_z))

#define CACHE_BGRA_ADD(_pCache, _b, _g, _r, _a) D3DCacheBGRAAdd(_pCache, _b, _g, _r, _a)

#define CACHE_ST_ADD(_pCache, _stage, _s, _t) D3DCacheSTAdd(_pCache, _stage, _s, _t)

#define CACHE_INDEX_ADD(_pCache, _index) D3DCacheIndexAdd(_pCache, _index)
#endif

#define CHUNK_XYZ_SET(_pChunk, _index, _x, _y, _z)	\
do	\
{	\
	_pChunk->xyz[_index].x = _x;	\
	_pChunk->xyz[_index].y = _y;	\
	_pChunk->xyz[_index].z = _z;	\
} while (0)

#define CHUNK_BGRA_SET(_pChunk, _index, _b, _g, _r, _a)	\
do	\
{	\
	_pChunk->bgra[_index].b = _b;	\
	_pChunk->bgra[_index].g = _g;	\
	_pChunk->bgra[_index].r = _r;	\
	_pChunk->bgra[_index].a = _a;	\
} while (0)

#define CHUNK_ST0_SET(_pChunk, _index, _s, _t)	\
do	\
{	\
	_pChunk->st0[_index].s = _s;	\
	_pChunk->st0[_index].t = _t;	\
} while (0)

#define CHUNK_ST1_SET(_pChunk, _index, _s, _t)	\
do	\
{	\
	_pChunk->st1[_index].s = _s;	\
	_pChunk->st1[_index].t = _t;	\
} while (0)

#define CHUNK_INDEX_SET(_pChunk, _index, _value)	\
do	\
{	\
	_pChunk->indices[_index] = _value;	\
} while (0)

#define CACHE_UNLOCK(_pCache)	\
do	\
{	\
	int	i;	\
	IDirect3DVertexBuffer9_Unlock((_pCache)->xyzBuffer.pVBuffer);	\
	for (i = 0; i < TEMP_NUM_STAGES; i++)	\
		IDirect3DVertexBuffer9_Unlock((_pCache)->stBuffer[i].pVBuffer);	\
	IDirect3DVertexBuffer9_Unlock((_pCache)->bgraBuffer.pVBuffer);	\
	IDirect3DIndexBuffer9_Unlock((_pCache)->indexBuffer.pIBuffer);	\
} while (0)

#define CACHE_BGRA_LOCK(_pCache)	\
do	\
{	\
	IDirect3DVertexBuffer9_Lock((_pCache)->bgraBuffer.pVBuffer,	\
                              0, 0, (void **)&(_pCache)->bgraBuffer.u.pBGRA, \
                              D3DLOCK_DISCARD);             \
} while (0)

#define CACHE_BGRA_UNLOCK(_pCache)	\
do	\
{	\
	IDirect3DVertexBuffer9_Unlock((_pCache)->bgraBuffer.pVBuffer);	\
} while (0)

typedef struct d3d_texture_cache_entry
{
	LPDIRECT3DTEXTURE9	pTexture;
	unsigned int		pDibID;
	unsigned int		pDibID2;
	int					size;
	int					effects;
	BYTE				xLat0;
	BYTE				xLat1;
	char				frame;
} d3d_texture_cache_entry;

typedef struct d3d_texture_cache
{
	int					curIndex;
	int					size;
	int					max;
	list_type			textureList;
} d3d_texture_cache;

typedef struct d3d_render_cache
{
	// packet list
	unsigned int	curPacket;
	unsigned int	numPackets;
	unsigned int	size;
	unsigned int	curPage;
	render_packet	rPackets[TEMP_CACHE_MAX];

	// dedicated vertex buffers
	custom_vbuffer	stBuffer[TEMP_NUM_STAGES];
	custom_vbuffer	bgraBuffer;
	custom_vbuffer	xyzBuffer;
	custom_ibuffer	indexBuffer;
} d3d_render_cache;

typedef struct d3d_render_cache_system
{
	int					numCaches;
//	int					curCache;
	list_type			renderCacheList;
	list_type			curCache;
//	d3d_render_cache	renderCaches[5];
	d3d_render_cache	*pCurCache;

	// dedicated texture cache
	d3d_texture_cache	textureCache;
} d3d_render_cache_system;

// raw chunk of geometry data, has it's own transform
typedef struct d3d_render_chunk_new
{
	D3DMATRIX			xForm;
	Bool				(*pMaterialFctn)(struct d3d_render_chunk_new *pChunk);
	d3d_render_cache	*pRenderCache;
	u_int				flags;
	u_int				curIndex;
	u_int				startIndex;
	u_int				numIndices;
	u_int				numVertices;
	u_int				numPrimitives;
	BOOL				drawn;
	int					side;

	Sector				*pSector;
	Sidedef				*pSideDef;
	Sector				*pSectorPos;
	Sector				*pSectorNeg;

	BYTE				xLat0;
	BYTE				xLat1;
	BYTE				zBias;
	Bool				isTargeted;
	Plane				plane;
	custom_index		indices[(MAX_NPTS - 2) * 3];
	custom_st			st0[MAX_NPTS];
	custom_st			st1[MAX_NPTS];
	custom_bgra			bgra[MAX_NPTS];
	custom_xyz			xyz[MAX_NPTS];
} d3d_render_chunk_new;

// chunks of geometry all using same texture and material
typedef struct d3d_render_packet_new
{
	Bool				(*pMaterialFctn)(struct d3d_render_packet_new *pPacket,
										d3d_render_cache_system *pCacheSystem);
	LPDIRECT3DTEXTURE9	pTexture;
	PDIB				pDib;
	BYTE				xLat0;
	BYTE				xLat1;
	int					effect;
	int					numStages;
	u_int				flags;
	u_int				curChunk;
	u_int				size;
	d3d_render_chunk_new	renderChunks[PACKET_SIZE];
} d3d_render_packet_new;

// render packet pool
typedef Bool (*MaterialFctn)(struct d3d_render_pool_new *pPool);
typedef struct d3d_render_pool_new
{
	MaterialFctn		pMaterialFctn;
	u_int					curPacket;
	u_int					size;
	u_int					packetSize;
	u_int					numLists;
	list_type				curPacketList;
	list_type				renderPacketList;
//	d3d_render_packet_new	renderPackets[POOL_SIZE];
} d3d_render_pool_new;

void				D3DCacheInit(d3d_render_cache *pCache, int size,
									  int numStages, DWORD flags);
void				D3DCacheFill(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool,
								 int numStages);
void				D3DCacheFlush(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool,
								  int numStages, int type);
LPDIRECT3DTEXTURE9	D3DCacheTextureLookupSwizzled(d3d_texture_cache *pTextureCache, d3d_render_packet_new *pPacket,
												 int effect);
LPDIRECT3DTEXTURE9	D3DCacheTextureLookup(d3d_texture_cache *pTextureCache, d3d_render_packet_new *pPacket,
												 int effect);
void				D3DCacheReset(d3d_render_cache *pRenderCache);
void				D3DCacheXYZAdd(d3d_render_cache *pRenderCache, float x, float y, float z);
void				D3DCacheSTAdd(d3d_render_cache *pRenderCache, int stage, float s, float t);
void				D3DCacheBGRAAdd(d3d_render_cache *pRenderCache, int b, int g, int r, int a);
void				D3DCacheIndexAdd(d3d_render_cache *pRenderCache, int index);
void				D3DCacheBGRASet(d3d_render_cache *pRenderCache, int index, int b, int g,
									int r, int a);

// cache system stuff
void				D3DCacheSystemInit(d3d_render_cache_system *pCacheSystem, int texCacheSize);
d3d_render_cache	*D3DCacheSystemSwap(d3d_render_cache_system *pCacheSystem);
void				D3DCacheSystemShutdown(d3d_render_cache_system *pCacheSystem);
void				D3DCacheSystemReset(d3d_render_cache_system *pCacheSystem);

#endif // __D3DCACHE_H__

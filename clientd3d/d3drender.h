// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

#define DEGREES_TO_RADIANS(_x)	((float)_x * PITWICE / 360.0f)
#define RADIANS_TO_DEGREES(_x)	((float)_x * 360.0f / PITWICE)

#define DLIGHT_SCALE(_x)		((_x * 14000 / 255) + 4000)

#define D3DRENDER_CLIP(_x, _w)	((_x > -fabs(_w)) && (_x < fabs(_w)) ? 1 : 0)

#define ZBIAS_UNDERUNDER		1
#define ZBIAS_UNDER				3
#define ZBIAS_UNDEROVER			6
#define ZBIAS_BASE				10
#define ZBIAS_OVERUNDER			11
#define ZBIAS_OVER				13
#define ZBIAS_OVEROVER			15
#define ZBIAS_TARGETED			0
#define ZBIAS_DEFAULT			1


#define VIEW_ELEMENT_Z			0.01f
#define PLAYER_OVERLAY_Z		0.02f

#define ZBIAS_WORLD				2
#define ZBIAS_MASK				1

#define D3DRENDER_REDRAW_UPDATE	0x00000001
#define D3DRENDER_REDRAW_ALL	0x00000002

#define D3DRENDER_SET_ALPHATEST_STATE(_pDevice, _enable, _refValue, _compareFunc)	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, _refValue);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAFUNC, _compareFunc);

#define D3DRENDER_SET_ALPHABLEND_STATE(_pDevice, _enable, _srcBlend, _dstBlend)	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, _srcBlend);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, _dstBlend);

#define D3DRENDER_SET_STENCIL_STATE(_pDevice, _enable, _stencilFunc, _refValue, _pass, _fail, _zfail)	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILFUNC, _stencilFunc);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILREF, _refValue);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILPASS, _pass);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILFAIL, _fail);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILZFAIL, _zfail);

#define D3DRENDER_SET_COLOR_STAGE(_pDevice, _stage, _opValue, _arg0Value, _arg1Value)	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLOROP,	_opValue);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLORARG1, _arg0Value);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLORARG2, _arg1Value);

#define D3DRENDER_SET_ALPHA_STAGE(_pDevice, _stage, _opValue, _arg0Value, _arg1Value)	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAOP,	_opValue);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAARG1, _arg0Value);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAARG2, _arg1Value);

#define D3DRENDER_SET_STREAMS(_pDevice, _pCache, _numStages)	\
	int	_i = 0;	\
	int	_j;	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
		(_pCache)->xyzBuffer.pVBuffer, 0, sizeof(custom_xyz));	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
		(_pCache)->bgraBuffer.pVBuffer, 0, sizeof(custom_bgra));	\
	for (_j = 0; _j < _numStages; _j++)	\
		IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
			(_pCache)->stBuffer[_j].pVBuffer, 0, sizeof(custom_st));	\
	IDirect3DDevice9_SetIndices(_pDevice, (_pCache)->indexBuffer.pIBuffer);

#define D3DRENDER_CLEAR_STREAMS(_pDevice, _numStages)	\
	int	_i = 0;	\
	int	_j;	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	for (_j = 0; _j < _numStages; _j++)	\
		IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	IDirect3DDevice9_SetIndices(_pDevice, NULL);

typedef struct d_light
{
	custom_xyz	xyz;
   // Max and min x,y values for how far
   // the light reaches.
   float maxX, maxY, minX, minY;
	custom_xyz	xyzScale;
	custom_xyz	invXYZScale;
	custom_xyz	invXYZScaleHalf;
	custom_bgra	color;
	ID			objID;
} d_light;

typedef struct d_light_cache
{
	int		numLights;
	d_light	dLights[MAX_DLIGHTS];
} d_light_cache;

typedef struct font_3d
{
	TCHAR				strFontName[80];
	long				fontHeight;
	long				flags;

	LPDIRECT3DTEXTURE9	pTexture;
	long				texWidth;
	long				texHeight;
	float				texScale;
	custom_st			texST[128 - 32][2];
  // Deal with underhanging and overhanging characters
  ABC         abc[128 - 32];
} font_3d;

extern LPDIRECT3D9				gpD3D;
extern LPDIRECT3DDEVICE9		gpD3DDevice;

extern int		gNumVertices;
extern BOOL		gbAlwaysRun;
extern int		gD3DEnabled;
extern int		gScreenWidth;
extern int		gScreenHeight;

HRESULT				D3DRenderInit(HWND hWnd);
void				D3DRenderShutDown(void);
void				D3DRenderBegin(room_type *room, Draw3DParams *params);
void				D3DGeometryBuild(room_type *room);
void				D3DGeometryUpdate(room_type *room);
void				D3DRenderResizeDisplay(int left, int top, int right, int bottom);
void				D3DRenderEnableToggle(void);
int					D3DRenderIsEnabled(void);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect);
void				D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, BYTE flags);
int					D3DRenderObjectGetLight(BSPnode *tree, room_contents_node *pRNode);
void				D3DRenderBackgroundSet(ID background);
void				D3DRenderBackgroundSet2(ID background);
d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, BYTE effect);
d3d_render_packet_new *D3DRenderPacketNew(d3d_render_pool_new *pPool);
d3d_render_chunk_new *D3DRenderChunkNew(d3d_render_packet_new *pPacket);
void				D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc);
void				*D3DRenderMalloc(unsigned int bytes);
void				D3DRenderFontInit(font_3d *pFont, HFONT hFont);

// material functions
Bool D3DMaterialNone(d3d_render_chunk_new *pPool);

// world
Bool D3DMaterialWorldPool(d3d_render_pool_new *pPool);
Bool D3DMaterialWorldPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialWorldDynamicChunk(d3d_render_chunk_new *pChunk);
Bool D3DMaterialWorldStaticChunk(d3d_render_chunk_new *pChunk);
Bool D3DMaterialWallMaskPool(d3d_render_pool_new *pPool);
Bool D3DMaterialMaskChunk(d3d_render_chunk_new *pChunk);

// lmaps
Bool D3DMaterialLMapDynamicPool(d3d_render_pool_new *pPool);
Bool D3DMaterialLMapDynamicPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialLMapDynamicChunk(d3d_render_chunk_new *pChunk);
Bool D3DMaterialLMapStaticChunk(d3d_render_chunk_new *pChunk);

// objects
Bool D3DMaterialObjectPool(d3d_render_pool_new *pPool);
Bool D3DMaterialObjectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialObjectChunk(d3d_render_chunk_new *pChunk);

// invisible objects
Bool D3DMaterialObjectInvisiblePool(d3d_render_pool_new *pPool);
Bool D3DMaterialObjectInvisiblePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialObjectInvisibleChunk(d3d_render_chunk_new *pChunk);

// effects
Bool D3DMaterialEffectPool(d3d_render_pool_new *pPool);
Bool D3DMaterialEffectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialEffectChunk(d3d_render_chunk_new *pChunk);

// blur
Bool D3DMaterialBlurPool(d3d_render_pool_new *pPool);
Bool D3DMaterialBlurPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialBlurChunk(d3d_render_chunk_new *pChunk);

// particles
Bool D3DMaterialParticlePool(d3d_render_pool_new *pPool);
Bool D3DMaterialParticlePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
Bool D3DMaterialParticleChunk(d3d_render_chunk_new *pChunk);

// Use this function to determine if the bounding box is out of the player's
//view. Useful for not adding stuff to draw that the player can't see.
Bool IsHidden(Draw3DParams *params, long x0, long y0, long x1, long y1);

#endif	// __D3DRENDER_H__

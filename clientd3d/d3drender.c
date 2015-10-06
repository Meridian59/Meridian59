// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

#define	TEX_CACHE_MAX_OBJECT	8000000
#define	TEX_CACHE_MAX_WORLD		8000000
#define	TEX_CACHE_MAX_LMAP		8000000
#define	TEX_CACHE_MAX_WALLMASK	2000000
#define	TEX_CACHE_MAX_EFFECT	2000000
#define	TEX_CACHE_MAX_PARTICLE	1000000
#define FOV_H					((gD3DRect.right - gD3DRect.left) / (float)(MAXX * stretchfactor) * (-PI / 3.6f))
#define FOV_V					((gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * stretchfactor) * (PI / 6.0f))
#define Z_RANGE					(400000.0f)

d3d_render_packet_new	*gpPacket;

LPDIRECT3D9				gpD3D = NULL;
LPDIRECT3DDEVICE9		gpD3DDevice = NULL;

// temp dynamic lightmaps
LPDIRECT3DTEXTURE9		gpDLightAmbient = NULL;
LPDIRECT3DTEXTURE9		gpDLightWhite = NULL;
LPDIRECT3DTEXTURE9		gpDLightOrange = NULL;
LPDIRECT3DTEXTURE9		gpBloom = NULL;
LPDIRECT3DTEXTURE9		gpNoLookThrough = NULL;
LPDIRECT3DTEXTURE9		gpBackBufferTex[16];
LPDIRECT3DTEXTURE9		gpBackBufferTexFull;
LPDIRECT3DTEXTURE9		gpViewElements[NUM_VIEW_ELEMENTS];
LPDIRECT3DTEXTURE9		gpSunTex;

LPDIRECT3DTEXTURE9		gpSkyboxTextures[5][6];

D3DVIEWPORT9			gViewport;
D3DCAPS9				gD3DCaps;

d3d_render_cache		gObjectCache;
d3d_render_cache		gWorldCache;
d3d_render_cache		gWorldCacheStatic;
d3d_render_cache		gLMapCacheStatic;

d3d_render_cache_system	gObjectCacheSystem;
d3d_render_cache_system	gWorldCacheSystem;
d3d_render_cache_system	gLMapCacheSystem;
d3d_render_cache_system	gWorldCacheSystemStatic;
d3d_render_cache_system	gLMapCacheSystemStatic;
d3d_render_cache_system	gWallMaskCacheSystem;
d3d_render_cache_system	gEffectCacheSystem;
d3d_render_cache_system	gParticleCacheSystem;

d_light_cache			gDLightCache;
d_light_cache			gDLightCacheDynamic;

d3d_render_pool_new		gObjectPool;
d3d_render_pool_new		gWorldPool;
d3d_render_pool_new		gLMapPool;
d3d_render_pool_new		gWorldPoolStatic;
d3d_render_pool_new		gLMapPoolStatic;
d3d_render_pool_new		gWallMaskPool;
d3d_render_pool_new		gEffectPool;
d3d_render_pool_new		gParticlePool;

extern particle_system			gParticleSystemSand;
extern particle_system			gParticleSystemRain;
extern particle_system			gParticleSystemSnow;
extern particle_system			gParticleSystemFireworks;

custom_xyz				playerOldPos;
custom_xyz				playerDeltaPos;

font_3d					gFont;

RECT					gD3DRect;
int						gD3DEnabled;
BYTE					gViewerLight = 0;
int						gNumObjects;
int						gNumVertices;
int						gNumDPCalls;
PALETTEENTRY			gPalette[256];
unsigned int			gFrame = 0;
int						gScreenWidth;
int						gScreenHeight;
int						gCurBackground;

D3DVERTEXELEMENT9		decl0[] = {
	{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	D3DDECL_END()
	};

D3DVERTEXELEMENT9		decl1[] = {
	{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{2, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
	};

D3DVERTEXELEMENT9		decl2[] = {
	{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{2, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{3, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	D3DDECL_END()
	};

LPDIRECT3DVERTEXDECLARATION9 decl0dc;
LPDIRECT3DVERTEXDECLARATION9 decl1dc;
LPDIRECT3DVERTEXDECLARATION9 decl2dc;

AREA					gD3DView;
Bool					gD3DRedrawAll = FALSE;
int						gTemp = 0;
Bool					gWireframe;		// this is really bad, I'm sorry

extern player_info		player;
extern long				viewer_height;
extern Color			base_palette[];
extern PDIB				background;         /* Pointer to background bitmap */
extern ObjectRange		visible_objects[];    /* Where objects are on screen */
extern int				num_visible_objects;
extern Draw3DParams		*p;
extern int				gNumCalls;
extern room_type		current_room;
extern long				shade_amount;
extern DrawItem			drawdata[];
extern long				nitems;
extern int				sector_depths[];
extern d3d_driver_profile	gD3DDriverProfile;
extern BYTE				*gBits;
extern BYTE				*gBufferBits;
extern D3DPRESENT_PARAMETERS	gPresentParam;
extern Vector3D			sun_vect;
extern long				shade_amount;
extern long				stretchfactor;
extern BYTE				light_rows[MAXY/2+1];      // Strength of light as function of screen row
extern ViewElement		ViewElements[];
extern HDC				gBitsDC;

#define SKYBOX_DIMENSIONS	(75000.0f)

float gSkyboxXYZ[] =
{
	// back
	SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,

	// bottom
	-SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,

	// front
	-SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,

	// left
	-SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,

	// right
	SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -37000.0f, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,

	// top
	-SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, 37000.0f, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, 37000.0f, -SKYBOX_DIMENSIONS,
};

float gSkyboxST[] =
{
	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,

	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,

	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,

	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,

	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,

	0.001f, 0.001f,
	0.001f, 0.999f,
	0.999f, 0.999f,
	0.999f, 0.001f,
};

unsigned char gSkyboxBGRA[] =
{
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,

	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,

	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,

	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,

	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,

	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
	192, 192, 192, 255,
};

void				D3DRenderBackgroundsLoad(char *pFilename, int index);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromResource(BYTE *ptr, int width, int height);
void				D3DRenderWorldDraw(d3d_render_pool_new *pPool, room_type *room,
										  Draw3DParams *params);
void				D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, BYTE flags);
void				D3DRenderPaletteSetNew(UINT xlatID0, UINT xlatID1, BYTE flags);
void				D3DRenderNamesDraw3D(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool,
						room_type *room, Draw3DParams *params, font_3d *pFont);
void				D3DRenderLMapsBuild(void);
void				D3DLMapsStaticGet(room_type *room);
void				D3DRenderFontInit(font_3d *pFont, HFONT hFont);
void				D3DRenderSkyboxDraw(d3d_render_pool_new *pPool, int angleHeading, int anglePitch);
void				D3DRenderSunDraw(int angleHeading, int anglePitch);
Bool				D3DComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area);

// Functions to calculate and extract data from walls/nodes.
void				D3DRenderFloorExtract(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
											custom_bgra *pBGRA);
void				D3DRenderCeilingExtract(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
											custom_bgra *pBGRA);
int				D3DRenderWallExtract(WallData *pWall, PDIB pDib, unsigned int *flags, custom_xyz *pXYZ,
										   custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side);

// new render stuff
void					GeometryUpdate(d3d_render_pool_new *pPool, d3d_render_cache_system *pCacheSystem);
void					D3DRenderPoolInit(d3d_render_pool_new *pPool, int size, int packetSize);
void					D3DRenderPoolShutdown(d3d_render_pool_new *pPool);
void					D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc);
d3d_render_packet_new	*D3DRenderPacketNew(d3d_render_pool_new *pPool);
void					D3DRenderPacketInit(d3d_render_packet_new *pPacket);
d3d_render_chunk_new	*D3DRenderChunkNew(d3d_render_packet_new *pPacket);
void					D3DRenderChunkInit(d3d_render_chunk_new *pChunk);
d3d_render_packet_new	*D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, BYTE effect);
float					D3DRenderObjectLightGetNearest(room_contents_node *pRNode);
void					D3DRenderObjectsDraw(d3d_render_pool_new *pPool, room_type *room,
							Draw3DParams *params, BYTE flags, Bool drawTransparent);
void					D3DRenderOverlaysDraw(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params,
							BOOL underlays, BYTE flags, Bool drawTransparent);
void					D3DRenderProjectilesDrawNew(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params);
void					D3DRenderPlayerOverlaysDraw(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params);
void					D3DRenderPlayerOverlayOverlaysDraw(d3d_render_pool_new *pPool, list_type overlays,
							PDIB pDib, room_type *room, Draw3DParams *params, AREA *objArea, BOOL underlays);
void					D3DRenderViewElementsDraw(d3d_render_pool_new *pPool);

void					D3DRenderPacketFloorAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool dynamic);
void					D3DRenderPacketCeilingAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool dynamic);
void					D3DRenderPacketWallAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type, int side,
												  Bool dynamic);
void					D3DRenderPacketWallMaskAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type, int side,
												  Bool dynamic);
void					D3DRenderFloorMaskAdd(BSPnode *pNode, d3d_render_pool_new *pPool,
											  Bool bDynamic);
void					D3DRenderCeilingMaskAdd(BSPnode *pNode, d3d_render_pool_new *pPool,
												Bool bDynamic);

// Light map functions.
void					D3DRenderLMapsPostDraw(BSPnode *tree, Draw3DParams *params, d_light *light);
void					D3DRenderLMapsDynamicPostDraw(BSPnode *tree, Draw3DParams *params, d_light *light);
void					D3DRenderLMapPostFloorAdd(BSPnode *pNode, d3d_render_pool_new *pPool, d_light *light, Bool bDynamic);
void					D3DRenderLMapPostCeilingAdd(BSPnode *pNode, d3d_render_pool_new *pPool, d_light *light, Bool bDynamic);
void					D3DRenderLMapPostWallAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type, int side, d_light *light, Bool bDynamic);

void					D3DGeometryBuildNew(room_type *room, d3d_render_pool_new *pPool, Draw3DParams *params);

void					D3DPostOverlayEffects(d3d_render_pool_new *pPool);
LPDIRECT3DTEXTURE9		D3DRenderFramebufferTextureCreate(LPDIRECT3DTEXTURE9	pTex0,
													 LPDIRECT3DTEXTURE9	pTex1,
													 float width, float height);
void					*D3DRenderMalloc(unsigned int bytes);

float					D3DRenderFogEndCalc(d3d_render_chunk_new *pChunk);

// Functions for filling out tree data and retrieving it.
void              D3DFillTreeData(room_type *room, Draw3DParams *params, Bool full);
void              D3DFillTreeDataTraverse(BSPnode *tree, Draw3DParams *params, Bool full);
void           D3DExtractWallFromTree(WallData *pWall, PDIB pDib, unsigned int *flags, custom_xyz *pXYZ,
   custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side);
void           D3DExtractFloorFromTree(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
   custom_bgra *pBGRA);
void           D3DExtractCeilingFromTree(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
   custom_bgra *pBGRA);

// externed stuff
extern int			FindHotspotPdib(PDIB pdib, char hotspot, POINT *point);
extern void			DrawItemsD3D();
extern Bool			ComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area);
extern void			UpdateRoom3D(room_type *room, Draw3DParams *params);

inline static int DistanceGet(int x, int y)
{
   return (int)sqrt((double)(x * x) + (double)(y * y));;
}

inline void *D3DRenderMalloc(unsigned int bytes)
{
   return malloc(bytes);
}

/************************************************************************************
*
*  Initializes Direct3d
*  - Creates D3D object, D3DDevice object, sets up buffers
*
************************************************************************************/
HRESULT D3DRenderInit(HWND hWnd)
{
	D3DDISPLAYMODE	displayMode;		
	HRESULT			hr;
	int				i;
	
	// try to initialize Direct3D9
	if (NULL == (gpD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// create D3D9 device and other preparations
	gD3DEnabled = D3DDriverProfileInit();

	/*modeCount = gpD3D->lpVtbl->GetAdapterModeCount(gpD3D, D3DADAPTER_DEFAULT);
	for (i = 0; i < modeCount; i++)
	{
		if (FAILED(gpD3D->lpVtbl->EnumAdapterModes(gpD3D, D3DADAPTER_DEFAULT, i,
			&displayMode2[i])))
		{
		error = E_FAIL;
		assert(0);
		}
	}*/

	if (!gD3DEnabled)
		return FALSE;

	hr = IDirect3D9_GetAdapterDisplayMode(gpD3D, D3DADAPTER_DEFAULT, &displayMode);
	hr = IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &gD3DCaps);

	/***************************************************************************/
	/*                              VIEWPORT                                   */
	/***************************************************************************/
	
	// define viewport
	gViewport.X			= 0;
	gViewport.Y			= 0;
	gViewport.Width		= gScreenWidth;
	gViewport.Height	= gScreenHeight;
	gViewport.MinZ		= 0.0f;
	gViewport.MaxZ		= 1.0f;

	// set viewport
	hr = IDirect3DDevice9_SetViewport(gpD3DDevice, &gViewport);

	/***************************************************************************/
	/*                       INITIAL RENDERSTATE                               */
	/***************************************************************************/
	
	// Keep AA disabled for now, until fixed
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_LIGHTING, FALSE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CLIPPING, FALSE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_TRUE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DITHERENABLE, FALSE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, FALSE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_LASTPIXEL, TRUE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	//	DX_FUNC_CALL(gpD3DDevice->lpVtbl->SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME), gD3DError);

    D3DCacheSystemInit(&gLMapCacheSystem, gD3DDriverProfile.texMemLMapDynamic);
    D3DCacheSystemInit(&gLMapCacheSystemStatic, gD3DDriverProfile.texMemLMapStatic);
    D3DRenderPoolInit(&gLMapPool, POOL_SIZE, PACKET_SIZE);
    D3DRenderPoolInit(&gLMapPoolStatic, POOL_SIZE, PACKET_SIZE);

	D3DCacheSystemInit(&gObjectCacheSystem, gD3DDriverProfile.texMemObjects);
	D3DCacheSystemInit(&gWorldCacheSystem, gD3DDriverProfile.texMemWorldDynamic);
	D3DCacheSystemInit(&gWorldCacheSystemStatic, gD3DDriverProfile.texMemWorldStatic);
	D3DCacheSystemInit(&gWallMaskCacheSystem, 2000000);
	D3DCacheSystemInit(&gEffectCacheSystem, 1000000);
	D3DCacheSystemInit(&gParticleCacheSystem, 1000000);

	D3DRenderPoolInit(&gObjectPool, POOL_SIZE, PACKET_SIZE);
	D3DRenderPoolInit(&gWorldPool, POOL_SIZE, PACKET_SIZE);
	D3DRenderPoolInit(&gWorldPoolStatic, POOL_SIZE, PACKET_SIZE);
	D3DRenderPoolInit(&gWallMaskPool, POOL_SIZE / 2, PACKET_SIZE);
	D3DRenderPoolInit(&gEffectPool, POOL_SIZE / 8, PACKET_SIZE);
	D3DRenderPoolInit(&gParticlePool, POOL_SIZE, PACKET_SIZE);

	gWorldPool.pMaterialFctn = &D3DMaterialWorldPool;
	gWorldPoolStatic.pMaterialFctn = &D3DMaterialWorldPool;
	gLMapPool.pMaterialFctn = &D3DMaterialLMapDynamicPool;
	gObjectPool.pMaterialFctn = &D3DMaterialObjectPool;
	gLMapPoolStatic.pMaterialFctn = &D3DMaterialLMapDynamicPool;
	gWallMaskPool.pMaterialFctn = &D3DMaterialWallMaskPool;
	gEffectPool.pMaterialFctn = &D3DMaterialEffectPool;
	gParticlePool.pMaterialFctn = &D3DMaterialParticlePool;
	
	
	D3DRenderPaletteSet(0, 0, 0);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAXANISOTROPY, gD3DDriverProfile.maxAnisotropy);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);		
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MAXANISOTROPY, gD3DDriverProfile.maxAnisotropy);
		
	/***************************************************************************/
	/*                    VERTEX DECLARATIONS                                  */
	/***************************************************************************/

	IDirect3DDevice9_CreateVertexDeclaration(gpD3DDevice, decl0, &decl0dc);
	IDirect3DDevice9_CreateVertexDeclaration(gpD3DDevice, decl1, &decl1dc);
	IDirect3DDevice9_CreateVertexDeclaration(gpD3DDevice, decl2, &decl2dc);
	
	/***************************************************************************/
	/*                    DEFAULT Z-BIAS/DEPTHBIAS                             */
	/***************************************************************************/

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW(0.0f));

	/***************************************************************************/
	/*                    VERTEX DECLARATIONS                                  */
	/***************************************************************************/

	D3DRenderLMapsBuild();
	ReleaseCapture();

	if (gD3DDriverProfile.bFogEnable)
	{
		float	start = 0.0f;
		float	end = 50000.8f;
		DWORD	mode = D3DFOG_LINEAR;

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGCOLOR, 0);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGTABLEMODE, mode);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGSTART, *(DWORD *)(&start));
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	/***************************************************************************/
	/*                       FRAMEBUFFER TEXTURES                              */
	/***************************************************************************/

	for (i = 0; i <= 15; i++)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, 256, 256, 1, D3DUSAGE_RENDERTARGET, 
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gpBackBufferTex[i], NULL);
	}

	IDirect3DDevice9_CreateTexture(gpD3DDevice, 1024, 1024, 1, D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gpBackBufferTexFull, NULL);
	
	/***************************************************************************/
	/*                                FONT                                     */
	/***************************************************************************/

	// This will call D3DRenderFontInit to make sure the font texture is created
	GraphicsResetFont();

	/***************************************************************************/
	/*                                MISC                                     */
	/***************************************************************************/

	playerOldPos.x = 0;
	playerOldPos.y = 0;
	playerOldPos.z = 0;

	return S_OK;
}

/************************************************************************************
*
*  Shutdown Direct3d
*  - ...
*
************************************************************************************/
void D3DRenderShutDown(void)
{
	int	i, j;
	HRESULT hr;

	if (!gD3DDriverProfile.bSoftwareRenderer)
	{
		if (config.bDynamicLighting)
		{
			D3DCacheSystemShutdown(&gLMapCacheSystem);
			D3DCacheSystemShutdown(&gLMapCacheSystemStatic);		
			D3DRenderPoolShutdown(&gLMapPool);
			D3DRenderPoolShutdown(&gLMapPoolStatic);
		}
		
		D3DCacheSystemShutdown(&gObjectCacheSystem);
		D3DCacheSystemShutdown(&gWorldCacheSystem);
		D3DCacheSystemShutdown(&gWorldCacheSystemStatic);
		D3DCacheSystemShutdown(&gWallMaskCacheSystem);
		D3DCacheSystemShutdown(&gEffectCacheSystem);
		D3DCacheSystemShutdown(&gParticleCacheSystem);

		D3DRenderPoolShutdown(&gObjectPool);
		D3DRenderPoolShutdown(&gWorldPool);
		D3DRenderPoolShutdown(&gWorldPoolStatic);
		D3DRenderPoolShutdown(&gWallMaskPool);
		D3DRenderPoolShutdown(&gEffectPool);
		D3DRenderPoolShutdown(&gParticlePool);

		/***************************************************************************/
		/*                          CUSTOM TEXTURES                                */
		/***************************************************************************/
		
		if (gpDLightWhite)			hr = IDirect3DDevice9_Release(gpDLightWhite);
		if (gpDLightOrange)			hr = IDirect3DDevice9_Release(gpDLightOrange);
		if (gpBloom)				hr = IDirect3DDevice9_Release(gpBloom);
		if (gpNoLookThrough)		hr = IDirect3DDevice9_Release(gpNoLookThrough);
		if (gpBackBufferTexFull)	hr = IDirect3DDevice9_Release(gpBackBufferTexFull);
		if (gpSunTex)				hr = IDirect3DDevice9_Release(gpSunTex);
		if (gFont.pTexture)			hr = IDirect3DDevice9_Release(gFont.pTexture);

		gpDLightWhite		= NULL;   
		gpDLightOrange		= NULL;     
		gpBloom				= NULL;
		gpNoLookThrough		= NULL;
		gpBackBufferTexFull	= NULL;
		gpSunTex			= NULL;
		gFont.pTexture		= NULL;
		
		for (i = 0; i < 16; i++)
		{
			if (gpBackBufferTex[i])
			{
				hr = IDirect3DDevice9_Release(gpBackBufferTex[i]);
				gpBackBufferTex[i] = NULL;
			}
		}

		for (j = 0; j < 5; j++)
		{
			for (i = 0; i < 6; i++)
			{
				if (gpSkyboxTextures[j][i])
				{
					hr = IDirect3DDevice9_Release(gpSkyboxTextures[j][i]);
					gpSkyboxTextures[j][i] = NULL;
				}
			}
		}

		for (i = 0; i < 8; i++)
		{
			if (gpViewElements[i])
			{
				hr = IDirect3DDevice9_Release(gpViewElements[i]);
				gpViewElements[i] = NULL;
			}
		}
		
		/***************************************************************************/
		/*                       VERTEX DECLARATIONS                               */
		/***************************************************************************/
		
		if (decl0dc) hr = IDirect3DDevice9_Release(decl0dc);
		if (decl1dc) hr = IDirect3DDevice9_Release(decl1dc);
		if (decl2dc) hr = IDirect3DDevice9_Release(decl2dc);

		decl0dc = NULL;
		decl1dc = NULL;
		decl2dc = NULL;

		/***************************************************************************/
		/*                               DEVICE                                    */
		/***************************************************************************/

		// clean D3D device
		if (gpD3DDevice)
		{
			// check cooperative level
			hr = IDirect3DDevice9_TestCooperativeLevel(gpD3DDevice);
			
			// must reset devices in state DEVICENOTRESET
			// or the release below will leave a mem-leak
			if (hr == D3DERR_DEVICENOTRESET)		
				hr = IDirect3DDevice9_Reset(gpD3DDevice, &gPresentParam);	

			// release device
			hr = IDirect3DDevice9_Release(gpD3DDevice);
		}

		// clean Direct3D
		if (gpD3D)
			hr = IDirect3D9_Release(gpD3D);

		gpD3DDevice	= NULL;
		gpD3D		= NULL;
	}
}

/************************************************************************************
*
*  Main render function
*  - This creates a frame on your monitor
*
************************************************************************************/
void D3DRenderBegin(room_type *room, Draw3DParams *params)
{
	D3DMATRIX	mat, rot, trans, view, proj, identity;
	int			angleHeading, anglePitch;
   long		timeOverall, timeWorld, timeObjects, timeLMaps, timeSkybox;
   long     timeSkybox2, timeInit, timeParticles;
	static ID	tempBkgnd = 0;
	HRESULT		hr;
	Bool		draw_sky = TRUE;
	Bool		draw_world = TRUE;
	Bool		draw_objects = TRUE;
	Bool		draw_particles = TRUE;
	room_contents_node	*pRNode;
	
	/***************************************************************************/
	/*                             PREPARATIONS                                */
	/***************************************************************************/

	// If blind, don't draw anything
	if (effects.blind)
	{
		draw_sky = FALSE;
		draw_world = FALSE;
		draw_objects = FALSE;
		draw_particles = FALSE;
	}

	// Load skybox textures if not yet loaded
	if (gpSkyboxTextures[0][0] == NULL)
	{
		D3DRenderBackgroundsLoad("./resource/skya.bsf", 0);
		D3DRenderBackgroundsLoad("./resource/skyb.bsf", 1);
		D3DRenderBackgroundsLoad("./resource/skyc.bsf", 2);
		D3DRenderBackgroundsLoad("./resource/skyd.bsf", 3);
		D3DRenderBackgroundsLoad("./resource/redsky.bsf", 4);
	}

	if (tempBkgnd != current_room.bkgnd)
	{
		D3DRenderBackgroundSet2(current_room.bkgnd);
		tempBkgnd = current_room.bkgnd;
	}

	// view element textures
	if (gFrame == 0)
	{
		int	i;

		for (i = 0; i < NUM_VIEW_ELEMENTS; i++)
		{
			gpViewElements[i] = D3DRenderTextureCreateFromResource(ViewElements[i].bits,
				ViewElements[i].width, ViewElements[i].height);
		}
	}

	gFrame++;

   timeOverall = timeWorld = timeObjects = timeLMaps = timeSkybox = timeSkybox2 = timeInit = timeParticles = 0;

	timeOverall = timeInit = timeGetTime();

	gNumObjects = 0;
	gNumVertices = 0;
	gNumDPCalls = 0;

	p = params;

	gDLightCache.numLights = 0;
	gDLightCacheDynamic.numLights = 0;
	D3DLMapsStaticGet(room);

	if (gD3DRedrawAll & D3DRENDER_REDRAW_ALL)
	{
      D3DGeometryBuildNew(room, &gWorldPoolStatic, params);
		gD3DRedrawAll = FALSE;
	}
	else if (gD3DRedrawAll & D3DRENDER_REDRAW_UPDATE)
	{
		GeometryUpdate(&gWorldPoolStatic, &gWorldCacheSystemStatic);
		gD3DRedrawAll = FALSE;
	}
	else if (gD3DDriverProfile.bFogEnable == FALSE)
	{
		GeometryUpdate(&gWorldPoolStatic, &gWorldCacheSystemStatic);
	}

	/***************************************************************************/
	/*                      BEGIN SCENE RENDERING                              */
	/***************************************************************************/
		
	IDirect3DDevice9_Clear(gpD3DDevice, 0, NULL, D3DCLEAR_TARGET |
                          D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);

	D3DRenderPaletteSet(0, 0, 0);

 	IDirect3DDevice9_BeginScene(gpD3DDevice);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= MAX_ANGLE)
		angleHeading -= MAX_ANGLE;

	anglePitch = PlayerGetHeightOffset();

	MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
	MatrixRotateX(&mat, (float)anglePitch * 45.0f / 414.0f * PI / 180.0f);
	MatrixMultiply(&rot, &rot, &mat);
	MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
	MatrixMultiply(&view, &trans, &rot);

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);

	XformMatrixPerspective(&proj, FOV_H, FOV_V, 100.0f, Z_RANGE);
//	aspectRatio = (float)(gD3DRect.right - gD3DRect.left) / (float)(gD3DRect.bottom - gD3DRect.top);
//	XformMatrixPerspective(&proj, -PI / 4.0f * 0.64f * aspectRatio, PI / 6.0f * 1.55f * (1.0f / aspectRatio), 100.0f, 150000.0f);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &proj);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_COLORWRITEENABLE,
                                   D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	D3DCacheSystemReset(&gObjectCacheSystem);
	D3DCacheSystemReset(&gLMapCacheSystem);
	D3DCacheSystemReset(&gWorldCacheSystem);
	
	UpdateRoom3D(room, params);

   // These two functions do the same thing, D3DFillTreeData uses the room's
   // node list and D3DFillTreeDataTraverse traverses the tree to fill it.
   // D3DFillTreeDataTraverse is slightly faster for drawing a partial room.
   // This function is added here so the data for walls, ceilings and floors
   // can be added to their structs (i.e. the data from the Extract functions)
   // so we don't have to calculate them repeatedly each time we draw something.
   // Data is valid for the entire frame. D3DGeometryBuildNew also calls this
   // to build the static light maps (with the third parameter set to TRUE to
   // fill the entire tree).
   D3DFillTreeDataTraverse(room->tree, params, FALSE);
   //D3DFillTreeData(room, params, FALSE);

	playerDeltaPos.x = params->viewer_x - playerOldPos.x;
	playerDeltaPos.y = params->viewer_y - playerOldPos.y;
	playerDeltaPos.z = params->viewer_height - playerOldPos.z;

	playerOldPos.x = params->viewer_x;
	playerOldPos.y = params->viewer_y;
	playerOldPos.z = params->viewer_height;

	timeInit = timeGetTime() - timeInit;
	// skybox
	if (draw_sky)
	{
		timeSkybox = timeGetTime();
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);

		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
		D3DCacheSystemReset(&gWorldCacheSystem);
		D3DRenderSkyboxDraw(&gWorldPool, angleHeading, anglePitch);
		D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);
		D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);
		timeSkybox = timeGetTime() - timeSkybox;
	}

	// restore the correct view matrix
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);

	/***************************************************************************/
	/*                              WORLD                                      */
	/***************************************************************************/
	
	if (draw_world)
	{
		timeWorld = timeGetTime();
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

		gNumCalls = 0;

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_ONE, D3DBLEND_ONE);

		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		IDirect3DDevice9_SetTextureStageState(gpD3DDevice, 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// This is disabled as it seems to draw sky texture in-between wall
		// textures on some maps. No errant effects noted from disabling this.
		// no look through/sky texture walls
		if (0)
		{
			// for no look through and sky texture, we take all map and procedurally generated
			// polys and render them, drawing only where there is no alpha.  for no look
			// through walls, this means transparent pixels will be drawn, and the "sky" texture
			// we use in the new client is just a 1x1 black texture with no alpha.  all pixels
			// rendered in this fashion also set stencil to one.  afterwards, normal geometry
			// is drawn and any world geometry that passes z test reverts stencil back to zero.
			// finally, skybox is drawn again only where stencil = 1, with z test set to ALWAYS
			// zbias is used to try and cover up as much zfighting as possible.
			D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, 254, D3DCMP_LESSEQUAL);
			D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_ONE, D3DBLEND_ONE);
			D3DRENDER_SET_STENCIL_STATE(gpD3DDevice, TRUE, D3DCMP_ALWAYS, 1, D3DSTENCILOP_REPLACE,
				D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP);

			// like below, we need to render a wireframe of each poly to cover holes
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			gWireframe = TRUE;
//			D3DCacheFlush(&gWallMaskCacheSystem, &gWallMaskPool, 1, D3DPT_TRIANGLESTRIP);
			gWireframe = FALSE;
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
			D3DCacheFlush(&gWallMaskCacheSystem, &gWallMaskPool, 1, D3DPT_TRIANGLESTRIP);
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, FALSE);
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);
			
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, FALSE);
		}

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_STENCIL_STATE(gpD3DDevice, TRUE, D3DCMP_ALWAYS, 1, D3DSTENCILOP_ZERO,
			D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP);

		D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
		D3DCacheSystemReset(&gWorldCacheSystem);
		D3DRenderWorldDraw(&gWorldPool, room, params);
		D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);

		// this pass is a gigantic hack used to cover up the cracks
		// caused by all the t-junctions in the old geometry.  the entire world is drawn
		// in wireframe, with zwrite disabled.  welcome to my hell
		if (1)
		{
			gWireframe = TRUE;
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			D3DCacheFlush(&gWorldCacheSystemStatic, &gWorldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
			D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);
			gWireframe = FALSE;
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);		
		}

		// finally, we actually get around to just drawing the goddam world
		D3DCacheFlush(&gWorldCacheSystemStatic, &gWorldPoolStatic, 1, D3DPT_TRIANGLESTRIP);
		D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

		timeWorld = timeGetTime() - timeWorld;
	}

	// skybox again, drawing only where stencil = 1
	if (draw_sky)
	{
		timeSkybox2 = timeGetTime();
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
      
		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_ONE, D3DBLEND_ONE);
		D3DRENDER_SET_STENCIL_STATE(gpD3DDevice, TRUE, D3DCMP_LESS, 0, D3DSTENCILOP_KEEP,
			D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_FALSE);

		if (gD3DDriverProfile.bFogEnable)
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);
    
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
		D3DCacheSystemReset(&gWorldCacheSystem);
		D3DRenderSkyboxDraw(&gWorldPool, angleHeading, anglePitch);
		D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);
		D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_TRUE);

		if (gD3DDriverProfile.bFogEnable)
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);
		timeSkybox2 = timeGetTime() - timeSkybox2;
	}

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	/***************************************************************************/
	/*                          POST LIGHTMAPS                                 */
	/***************************************************************************/
	
	if (draw_world && config.bDynamicLighting)
	{
		timeLMaps = timeGetTime();
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 1, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl2dc);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_ONE, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
      D3DCacheFlush(&gLMapCacheSystemStatic, &gLMapPoolStatic, 2, D3DPT_TRIANGLESTRIP);
		D3DRenderPoolReset(&gLMapPool, &D3DMaterialLMapDynamicPool);

      // We now loop through each light and traverse the BSP tree to determine
      // which nodes get lit up. Any parts of the tree that are behind the player
      // or whose bounding box does not contain the light are not checked, thus
      // speeding up light drawing significantly.
      //long timeStatic = timeGetTime();
      for (int numLights = 0; numLights < gDLightCache.numLights; numLights++)
      {
         D3DRenderLMapsPostDraw(room->tree, params, &gDLightCache.dLights[numLights]);
      }
      //timeStatic = timeGetTime() - timeStatic;
      //long timeDynamic = timeGetTime();
      for (int numLights = 0; numLights < gDLightCacheDynamic.numLights; numLights++)
      {
         D3DRenderLMapsDynamicPostDraw(room->tree, params, &gDLightCacheDynamic.dLights[numLights]);
      }
      //timeDynamic = timeGetTime() - timeDynamic;
      //debug(("Dynamic light maps = %d, static = %d\n", timeDynamic, timeStatic));

      D3DCacheFill(&gLMapCacheSystem, &gLMapPool, 2);
      D3DCacheFlush(&gLMapCacheSystem, &gLMapPool, 2, D3DPT_TRIANGLESTRIP);

		if (gD3DDriverProfile.bFogEnable)
			IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);

		timeLMaps = timeGetTime() - timeLMaps;
	}

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	
	/***************************************************************************/
	/*                             PARTICLES                                   */
	/***************************************************************************/
	
	if (draw_particles)
	{
      timeParticles = timeGetTime();

      // Updates emitter positions for all particle systems.
      D3DParticleSystemSetPlayerPos(playerDeltaPos.x, playerDeltaPos.y, playerDeltaPos.z);

		if (effects.sand)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

			D3DParticleSystemUpdateFluid(&gParticleSystemSand, &gParticlePool, &gParticleCacheSystem, params);
		}
		if (effects.raining && config.weather)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

			D3DParticleSystemUpdateFluid(&gParticleSystemRain, &gParticlePool, &gParticleCacheSystem, params);
		}
		if (effects.snowing && config.weather)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

			D3DParticleSystemUpdateFluid(&gParticleSystemSnow, &gParticlePool, &gParticleCacheSystem, params);
		}
		if (effects.fireworks)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

			D3DParticleSystemUpdateBurst(&gParticleSystemFireworks, &gParticlePool, &gParticleCacheSystem, params);
		}
		timeParticles = timeGetTime() - timeParticles;
	}

	/***************************************************************************/
	/*                   NAMES / OBJECTS / PLAYEROVERLAYS                      */
	/***************************************************************************/
	
	if (draw_objects)
	{
		timeObjects = timeGetTime();
		
		/************************** NAMES *********************************/
		
		if (config.draw_names)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

			IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					
			D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
			D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		
			MatrixIdentity(&identity);

			D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
			D3DCacheSystemReset(&gObjectCacheSystem);
			D3DRenderNamesDraw3D(&gObjectCacheSystem, &gObjectPool, room, params, &gFont);
			D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
			D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);

			IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
			IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);
		}

		/********************** NORMAL OBJECTS *****************************/
		
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
		D3DCacheSystemReset(&gObjectCacheSystem);
		D3DRenderOverlaysDraw(&gObjectPool, room, params, 1, FALSE, FALSE);
		D3DRenderObjectsDraw(&gObjectPool, room, params, FALSE, FALSE);
		D3DRenderOverlaysDraw(&gObjectPool, room, params, 0, FALSE, FALSE);
		D3DRenderProjectilesDrawNew(&gObjectPool, room, params);
		D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
		D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);

      /********************** TRANSPARENT OBJECTS ****************************/

      D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
      D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

      D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
      D3DCacheSystemReset(&gObjectCacheSystem);
      D3DRenderOverlaysDraw(&gObjectPool, room, params, 1, FALSE, TRUE);
      D3DRenderObjectsDraw(&gObjectPool, room, params, FALSE, TRUE);
      D3DRenderOverlaysDraw(&gObjectPool, room, params, 0, FALSE, TRUE);
      D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
      D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);

		/******************** INVISIBLE OBJECTS *****************************/

		D3DRenderFramebufferTextureCreate(gpBackBufferTexFull, gpBackBufferTex[0], 256, 256);

		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &proj);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl2dc);

		D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectInvisiblePool);
		D3DCacheSystemReset(&gObjectCacheSystem);
		D3DRenderOverlaysDraw(&gObjectPool, room, params, 1, DRAWFX_INVISIBLE, FALSE);
		D3DRenderObjectsDraw(&gObjectPool, room, params, DRAWFX_INVISIBLE, FALSE);
		D3DRenderOverlaysDraw(&gObjectPool, room, params, 0, DRAWFX_INVISIBLE, FALSE);
		D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 2);
		D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 2, D3DPT_TRIANGLESTRIP);

		/********************** PLAYER-OVERLAYS ****************************/

		// get player node
		pRNode = GetRoomObjectById(player.id);

		// player is invisible
		if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl2dc);

			D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectInvisiblePool);
			D3DCacheSystemReset(&gObjectCacheSystem);
			D3DRenderPlayerOverlaysDraw(&gObjectPool, room, params);
			D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 2);
			D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 2, D3DPT_TRIANGLESTRIP);
		}
		else
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

			D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
			D3DCacheSystemReset(&gObjectCacheSystem);
			D3DRenderPlayerOverlaysDraw(&gObjectPool, room, params);
			D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
			D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);
		}

      IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
      IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);
		timeObjects = timeGetTime() - timeObjects;
	}

	/***************************************************************************/
	/*                        POST OVERLAYS                                    */
	/***************************************************************************/
	
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

	// Set up orthographic projection for drawing overlays
	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	// post overlay effects
	if (draw_objects)
	{
		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, 1, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_FALSE);

		D3DCacheSystemReset(&gObjectCacheSystem);
		D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
		D3DPostOverlayEffects(&gObjectPool);
		D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
		D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, D3DZB_TRUE);
	}

	/***************************************************************************/
	/*                         EFFECTS: BLUR/WAVER                             */
	/***************************************************************************/
	
	// test blur
	if (effects.blur || effects.waver)
//	if (1)
	{
		d3d_render_packet_new	*pPacket;
		d3d_render_chunk_new	*pChunk;
		int						i, t;
		static int				offset = 0;
		static int				offsetDir = 1;

		t = gFrame & 7;

		if (gFrame & 63)
		{
			offset += offsetDir;

			if ((offset > 31) || (offset < 0))
			{
				offsetDir = -offsetDir;
				offset += offsetDir;
			}
		}

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		D3DRenderFramebufferTextureCreate(gpBackBufferTexFull, gpBackBufferTex[t], 256, 256);
		D3DCacheSystemReset(&gEffectCacheSystem);
		D3DRenderPoolReset(&gEffectPool, &D3DMaterialBlurPool);

		for (i = 0; i <= 7; i++)
		{
			pPacket = D3DRenderPacketNew(&gEffectPool);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			pPacket->pMaterialFctn = D3DMaterialBlurPacket;
			pChunk->pMaterialFctn = D3DMaterialBlurChunk;
			pPacket->pTexture = gpBackBufferTex[i];
			pChunk->numIndices = pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			MatrixIdentity(&pChunk->xForm);

			CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(-offset, gScreenWidth),
				0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(-offset, gScreenWidth),
				0, D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight + offset, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth + offset, gScreenWidth),
				0, D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight + offset, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth + offset, gScreenWidth),
				0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, gScreenHeight));

			CHUNK_ST0_SET(pChunk, 0, 0.0f, 0.0f);
			CHUNK_ST0_SET(pChunk, 1, 0.0f, 1.0f);
			CHUNK_ST0_SET(pChunk, 2, 1.0f, 1.0f);
			CHUNK_ST0_SET(pChunk, 3, 1.0f, 0.0f);

			CHUNK_BGRA_SET(pChunk, 0, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
			CHUNK_BGRA_SET(pChunk, 1, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
			CHUNK_BGRA_SET(pChunk, 2, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
			CHUNK_BGRA_SET(pChunk, 3, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);

			CHUNK_INDEX_SET(pChunk, 0, 1);
			CHUNK_INDEX_SET(pChunk, 1, 2);
			CHUNK_INDEX_SET(pChunk, 2, 0);
			CHUNK_INDEX_SET(pChunk, 3, 3);
		}

		MatrixIdentity(&mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		D3DCacheFill(&gEffectCacheSystem, &gEffectPool, 1);
		D3DCacheFlush(&gEffectCacheSystem, &gEffectPool, 1, D3DPT_TRIANGLESTRIP);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
	}

	/***************************************************************************/
	/*                             VIEW ELEMENTS                               */
	/***************************************************************************/
	
	if (1)
	{
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
		D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
		D3DCacheSystemReset(&gObjectCacheSystem);
		D3DRenderViewElementsDraw(&gObjectPool);
		D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
		D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);

		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);
		IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);
	}

	/***************************************************************************/
	/*                        END SCENE RENDERING                              */
	/***************************************************************************/
		
	IDirect3DDevice9_EndScene(gpD3DDevice);

	RECT	rect;
	rect.top = 0;
	rect.bottom = gScreenHeight;
	rect.left = 0;
	rect.right = gScreenWidth;

	hr = IDirect3DDevice9_Present(gpD3DDevice, &rect, &gD3DRect, NULL, NULL);

	if (hr == D3DERR_DEVICELOST)
	{
		// Keep checking until we can reset.
		while (hr == D3DERR_DEVICELOST)
			hr = IDirect3DDevice9_TestCooperativeLevel(gpD3DDevice);

		// completely shutdown & restart D3D9
		if (hr == D3DERR_DEVICENOTRESET)
		{		
			D3DRenderShutDown();
			gFrame = 0;
         gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;
			D3DRenderInit(hMain);
			D3DParticlesInit(false);
		}
	}
	
	timeOverall = timeGetTime() - timeOverall;
//	debug(("number of objects = %d\n", gNumObjects));
	if ((gFrame & 255) == 255)
		debug(("number of vertices = %d\nnumber of dp calls = %d\n", gNumVertices,
		gNumDPCalls));

   //debug(("all = %d lmaps = %d wrld = %d obj = %d  particles = %d sky = %d init = %d\n",
		//timeOverall, timeLMaps, timeWorld, timeObjects, timeParticles, timeSkybox+timeSkybox2, timeInit));
}

void D3DRenderWorldDraw(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params)
{
	int			count;
	BSPnode		*pNode = NULL;
	WallData	*pWall;
	Bool		bDynamic;

	for (count = 0; count < room->num_nodes; count++)
	{
		pNode = &room->nodes[count];

      // If the player can't see the bounding box (i.e.behind camera view) skip this node.
      // Since the extract packet functions use pre-extracted data, we shouldn't draw data
      // not visible to the player (it may not be extracted).
      if (IsHidden(params, (long)pNode->bbox.x0, (long)pNode->bbox.y0, (long)pNode->bbox.x1, (long)pNode->bbox.y1))
         continue;

		switch (pNode->type)
		{
			case BSPinternaltype:
				for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
				{
					int	flags;
					flags = 0;

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

					if ((pWall->pos_sidedef) && (pWall->neg_sidedef))
					{
						if ((pWall->pos_sidedef->flags & WF_TRANSPARENT) &&
							(pWall->neg_sidedef->flags & WF_TRANSPARENT))
							{
			//					continue;
							}
					}

					if (pWall->pos_sidedef)
					{
						if (pWall->pos_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->pos_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->pos_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if (pWall->neg_sidedef)
					{
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

               // TODO: this feature doesn't currently work properly, due to walls not being tagged
               // as drawable correctly in drawbsp.c. Code left commented out, as it is a work in progress.

               // D3DRenderWorldDraw now relies on checks done while constructing the list
               // of viewable objects, walls etc. in DrawBSP() in drawbsp.c. Walls that can be seen
               // have the respective boolean value set to TRUE, so all we need to do here is check
               // that value to decide whether to render the wall.  Old code left commented out as 
               // opposed to deleting in case any modification to drawbsp.c is performed which
               // invalidates this method. NOTE: rendering the world doesn't actually take a lot of
               // time, however the items drawn here need to match what was preloaded into the tree
               // by the D3DFillTreeData/D3DFillTreeDataTraversal functions, and they won't add data
               // if we don't need to draw it.
               //if (pWall->drawnormal)
               if ((flags & D3DRENDER_WALL_NORMAL) && (((short)pWall->z2 != (short)pWall->z1)
						|| ((short)pWall->zz2 != (short)pWall->zz1)))
					{
						D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_NORMAL, 1, TRUE);
						D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_NORMAL, -1, TRUE);
					}

					//if (pWall->drawbelow)
               if ((flags & D3DRENDER_WALL_BELOW) && (((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0)))
					{
						D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_BELOW, 1, TRUE);
						D3DRenderPacketWallAdd(pWall, &gWorldPool, D3DRENDER_WALL_BELOW, -1, TRUE);
					}

					//if (pWall->drawabove)
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
               //if (pNode->drawfloor)
               D3DRenderPacketFloorAdd(pNode, &gWorldPool, TRUE);
               //if (pNode->drawceiling)
               D3DRenderPacketCeilingAdd(pNode, &gWorldPool, TRUE);
				}
			break;

			default:
			break;
		}
	}
}

/*
 * D3DFillTreeData: Iterates through the list of BSP nodes in a room, and extracts
 *                  the data to the BSPnode struct for walls and ceilings, and to
 *                  the WallData struct for walls. This is done to precalculate the
 *                  data once prior to drawing a frame, and removes the need to 
 *                  calculate it multiple times. NOTE that without the full parameter
 *                  set, this ONLY adds data for what the player can actually see.
 */
void D3DFillTreeData(room_type *room, Draw3DParams *params, Bool full)
{
   BSPnode		*pNode = NULL;
   WallData	*pWall;
   Sector		*pSector;
   debug(("number of nodes is %i\n", room->num_nodes));
   for (int count = 0; count < room->num_nodes; count++)
   {
      pNode = &room->nodes[count];

      // If full is True, we draw the entire tree (e.g. for static lightmaps).
      // Otherwise, check if the bounding box of the tree is outside the player's
      // view and skip this node if so.
      if (!full && IsHidden(params, (long)pNode->bbox.x0, (long)pNode->bbox.y0, (long)pNode->bbox.x1, (long)pNode->bbox.y1))
         continue;

      switch (pNode->type)
      {
      case BSPinternaltype:
         for (pWall = pNode->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
         {
            pWall->separator.a = pNode->u.internal.separator.a;
            pWall->separator.b = pNode->u.internal.separator.b;
            pWall->separator.c = pNode->u.internal.separator.c;

            int	flags;

            flags = 0;

            if (pWall->pos_sidedef)
            {
               if (pWall->pos_sidedef->normal_bmap && (((short)pWall->z2 != (short)pWall->z1)
                  || ((short)pWall->zz2 != (short)pWall->zz1)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->normal_bmap, &pWall->pos_normal_d3dFlags,
                     pWall->pos_normal_xyz, pWall->pos_normal_stBase, pWall->pos_normal_bgra, D3DRENDER_WALL_NORMAL, 1);
               }

               if (pWall->pos_sidedef->below_bmap && (((short)pWall->z1 != (short)pWall->z0)
                  || ((short)pWall->zz1 != (short)pWall->zz0)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->below_bmap, &pWall->pos_below_d3dFlags,
                     pWall->pos_below_xyz, pWall->pos_below_stBase, pWall->pos_below_bgra, D3DRENDER_WALL_BELOW, 1);
               }

               if (pWall->pos_sidedef->above_bmap && (((short)pWall->z3 != (short)pWall->z2)
                  || ((short)pWall->zz3 != (short)pWall->zz2)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->above_bmap, &pWall->pos_above_d3dFlags,
                     pWall->pos_above_xyz, pWall->pos_above_stBase, pWall->pos_above_bgra, D3DRENDER_WALL_ABOVE, 1);
               }
            }

            if (pWall->neg_sidedef)
            {
               if (pWall->neg_sidedef->normal_bmap && (((short)pWall->z2 != (short)pWall->z1)
                  || ((short)pWall->zz2 != (short)pWall->zz1)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->normal_bmap, &pWall->neg_normal_d3dFlags,
                     pWall->neg_normal_xyz, pWall->neg_normal_stBase, pWall->neg_normal_bgra, D3DRENDER_WALL_NORMAL, -1);
               }

               if (pWall->neg_sidedef->below_bmap && (((short)pWall->z1 != (short)pWall->z0)
                  || ((short)pWall->zz1 != (short)pWall->zz0)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->below_bmap, &pWall->neg_below_d3dFlags,
                     pWall->neg_below_xyz, pWall->neg_below_stBase, pWall->neg_below_bgra, D3DRENDER_WALL_BELOW, -1);
               }
               if (pWall->neg_sidedef->above_bmap && (((short)pWall->z3 != (short)pWall->z2)
                  || ((short)pWall->zz3 != (short)pWall->zz2)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->above_bmap, &pWall->neg_above_d3dFlags,
                     pWall->neg_above_xyz, pWall->neg_above_stBase, pWall->neg_above_bgra, D3DRENDER_WALL_ABOVE, -1);
               }
            }
         }
         break;

      case BSPleaftype:
         pSector = pNode->u.leaf.sector;
         if (pSector->floor)
         {
            pNode->floor_pDib = pSector->floor;
            D3DRenderFloorExtract(pNode, pNode->floor_pDib, pNode->floor_xyz, pNode->floor_stBase, pNode->floor_bgra);
         }
         if (pSector->ceiling)
         {
            pNode->ceiling_pDib = pSector->ceiling;
            D3DRenderCeilingExtract(pNode, pNode->ceiling_pDib, pNode->ceiling_xyz, pNode->ceiling_stBase, pNode->ceiling_bgra);
         }
         break;

      default:
         break;
      }
   }
}

/*
 * D3DFillTreeDataTraverse: Traverses the BSP tree for a room, extracts the data
 *                          for the floors, ceilings and walls and stores it in
 *                          the appropriate structs (BSPnode and WallData).
 *                          Differs from D3DFillTreeData in that this is a tree
 *                          traversal via recursion versus iterating through the
 *                          nodes stored in the room_type struct. NOTE that without
 *                          the full parameter set, this ONLY adds data for what
 *                          the player can actually see.
 */
void D3DFillTreeDataTraverse(BSPnode *tree, Draw3DParams *params, Bool full)
{
   long		side;
   Sector		*pSector;

   if (!tree)
      return;

   // If full is True, we draw the entire tree (e.g. for static lightmaps).
   // Otherwise, check if the bounding box of the tree is outside the player's
   // view and skip this node if so.
   if (!full && IsHidden(params, (long)tree->bbox.x0, (long)tree->bbox.y0, (long)tree->bbox.x1, (long)tree->bbox.y1))
      return;

   switch (tree->type)
   {
   case BSPleaftype:
      pSector = tree->u.leaf.sector;
      if (pSector->floor)
      {
         tree->floor_pDib = pSector->floor;
         D3DRenderFloorExtract(tree, tree->floor_pDib, tree->floor_xyz, tree->floor_stBase, tree->floor_bgra);
      }
      if (pSector->ceiling)
      {
         tree->ceiling_pDib = pSector->ceiling;
         D3DRenderCeilingExtract(tree, tree->ceiling_pDib, tree->ceiling_xyz, tree->ceiling_stBase, tree->ceiling_bgra);
      }
      return;

   case BSPinternaltype:
      side = tree->u.internal.separator.a * params->viewer_x +
         tree->u.internal.separator.b * params->viewer_y +
         tree->u.internal.separator.c;

      //first, traverse closer side 
      if (side > 0)
         D3DFillTreeDataTraverse(tree->u.internal.pos_side, params, full);
      else
         D3DFillTreeDataTraverse(tree->u.internal.neg_side, params, full);

      // then do walls on the separator
      if (side != 0)
      {
         //WallList list;
         WallData	*pWall;
         int			flags;

         for (pWall = tree->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
         {
            flags = 0;

            pWall->separator.a = tree->u.internal.separator.a;
            pWall->separator.b = tree->u.internal.separator.b;
            pWall->separator.c = tree->u.internal.separator.c;

            if (pWall->pos_sidedef)
            {
               if (pWall->pos_sidedef->normal_bmap && (((short)pWall->z2 != (short)pWall->z1)
                  || ((short)pWall->zz2 != (short)pWall->zz1)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->normal_bmap, &pWall->pos_normal_d3dFlags,
                     pWall->pos_normal_xyz, pWall->pos_normal_stBase, pWall->pos_normal_bgra, D3DRENDER_WALL_NORMAL, 1);
               }

               if (pWall->pos_sidedef->below_bmap && (((short)pWall->z1 != (short)pWall->z0)
                  || ((short)pWall->zz1 != (short)pWall->zz0)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->below_bmap, &pWall->pos_below_d3dFlags,
                     pWall->pos_below_xyz, pWall->pos_below_stBase, pWall->pos_below_bgra, D3DRENDER_WALL_BELOW, 1);
               }

               if (pWall->pos_sidedef->above_bmap && (((short)pWall->z3 != (short)pWall->z2)
                  || ((short)pWall->zz3 != (short)pWall->zz2)))
               {
                  D3DRenderWallExtract(pWall, pWall->pos_sidedef->above_bmap, &pWall->pos_above_d3dFlags,
                     pWall->pos_above_xyz, pWall->pos_above_stBase, pWall->pos_above_bgra, D3DRENDER_WALL_ABOVE, 1);
               }
            }

            if (pWall->neg_sidedef)
            {
               if (pWall->neg_sidedef->normal_bmap && (((short)pWall->z2 != (short)pWall->z1)
                  || ((short)pWall->zz2 != (short)pWall->zz1)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->normal_bmap, &pWall->neg_normal_d3dFlags,
                     pWall->neg_normal_xyz, pWall->neg_normal_stBase, pWall->neg_normal_bgra, D3DRENDER_WALL_NORMAL, -1);
               }

               if (pWall->neg_sidedef->below_bmap && (((short)pWall->z1 != (short)pWall->z0)
                  || ((short)pWall->zz1 != (short)pWall->zz0)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->below_bmap, &pWall->neg_below_d3dFlags,
                     pWall->neg_below_xyz, pWall->neg_below_stBase, pWall->neg_below_bgra, D3DRENDER_WALL_BELOW, -1);
               }
               if (pWall->neg_sidedef->above_bmap && (((short)pWall->z3 != (short)pWall->z2)
                  || ((short)pWall->zz3 != (short)pWall->zz2)))
               {
                  D3DRenderWallExtract(pWall, pWall->neg_sidedef->above_bmap, &pWall->neg_above_d3dFlags,
                     pWall->neg_above_xyz, pWall->neg_above_stBase, pWall->neg_above_bgra, D3DRENDER_WALL_ABOVE, -1);
               }
            }
         }
      }

      // lastly, traverse farther side
      if (side > 0)
         D3DFillTreeDataTraverse(tree->u.internal.neg_side, params, full);
      else
         D3DFillTreeDataTraverse(tree->u.internal.pos_side, params, full);

      return;

   default:
      debug(("WalkBSPtree fill tree data error!\n"));
      return;
   }
}

void D3DRenderLMapsPostDraw(BSPnode *tree, Draw3DParams *params, d_light *light)
{
	long		side;

	if (!tree)
		return;

   // If the light isn't in this tree's bounding box, return.
   if (tree->bbox.x0 > light->maxX
      || tree->bbox.x1 < light->minX
      || tree->bbox.y0 > light->maxY
      || tree->bbox.y1 < light->minY)
   return;

   // If the player can't see the bounding box (behind camera view), return.
   if (IsHidden(params, (long)tree->bbox.x0, (long)tree->bbox.y0, (long)tree->bbox.x1, (long)tree->bbox.y1))
      return;

	switch(tree->type)
	{
		case BSPleaftype:
			if (tree->u.leaf.sector->flags & SF_HAS_ANIMATED)
			{
            //if (tree->drawfloor)
               D3DRenderLMapPostFloorAdd(tree, &gLMapPool, light, TRUE);
            //if (tree->drawceiling)
				   D3DRenderLMapPostCeilingAdd(tree, &gLMapPool, light, TRUE);
			}
			return;
	      
		case BSPinternaltype:
			side = tree->u.internal.separator.a * params->viewer_x + 
			tree->u.internal.separator.b * params->viewer_y +
			tree->u.internal.separator.c;

			/* first, traverse closer side */
			if (side > 0)
				D3DRenderLMapsPostDraw(tree->u.internal.pos_side, params, light);
			else
				D3DRenderLMapsPostDraw(tree->u.internal.neg_side, params, light);
	      
			/* then do walls on the separator */
			if (side != 0)
			{
				//WallList list;
				WallData	*pWall;
				int			flags;

				for (pWall = tree->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
				{
					Bool	bDynamic = FALSE;

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

               flags = 0;
					if (pWall->pos_sidedef)
					{
						if (pWall->pos_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->pos_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->pos_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if (pWall->neg_sidedef)
					{
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

               // D3DRenderLMapsPostDraw now relies on checks done while constructing the list
               // of viewable objects, walls etc. in DrawBSP() in drawbsp.c. Walls that can be seen
               // have the respective boolean value set to TRUE, so all we need to do here is check
               // that value to decide whether to render light on the wall. Significantly speeds up
               // rendering large amounts of lights. Old code left commented out as opposed to deleting
               // in case any modification to drawbsp.c is performed which invalidates this method.
					//if (pWall->drawnormal) 
               if ((flags & D3DRENDER_WALL_NORMAL) && (((short)pWall->z2 != (short)pWall->z1)
						|| (pWall->zz2 != pWall->zz1)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_NORMAL, side, light, TRUE);
					}

					//if (pWall->drawbelow)
               if ((flags & D3DRENDER_WALL_BELOW) && (((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_BELOW, side, light, TRUE);
					}

					//if (pWall->drawabove)
               if ((flags & D3DRENDER_WALL_ABOVE) && (((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_ABOVE, side, light, TRUE);
					}
				}
			}
	      
			/* lastly, traverse farther side */
			if (side > 0)
				D3DRenderLMapsPostDraw(tree->u.internal.neg_side, params, light);
			else
				D3DRenderLMapsPostDraw(tree->u.internal.pos_side, params, light);
	      
			return;

		default:
			debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

void D3DRenderLMapsDynamicPostDraw(BSPnode *tree, Draw3DParams *params, d_light *light)
{
	long		side;

	if (!tree)
		return;

   // If the light isn't in this tree's bounding box, return.
   if (tree->bbox.x0 > light->maxX
      || tree->bbox.x1 < light->minX
      || tree->bbox.y0 > light->maxY
      || tree->bbox.y1 < light->minY)
   return;

   // If the player can't see the bounding box (behind camera view), return.
   if (IsHidden(params,(long)tree->bbox.x0, (long)tree->bbox.y0, (long)tree->bbox.x1, (long)tree->bbox.y1))
      return;

	switch(tree->type)
	{
      case BSPleaftype:
         //if (tree->drawfloor)
			   D3DRenderLMapPostFloorAdd(tree, &gLMapPool, light, TRUE);
         //if (tree->drawceiling)
			   D3DRenderLMapPostCeilingAdd(tree, &gLMapPool, light, TRUE);

			return;
	      
		case BSPinternaltype:
			side = tree->u.internal.separator.a * params->viewer_x + 
			tree->u.internal.separator.b * params->viewer_y +
			tree->u.internal.separator.c;

			/* first, traverse closer side */
			if (side > 0)
				D3DRenderLMapsDynamicPostDraw(tree->u.internal.pos_side, params, light);
			else
				D3DRenderLMapsDynamicPostDraw(tree->u.internal.neg_side, params, light);
	      
			/* then do walls on the separator */
			if (side != 0)
			{
				//WallList list;
				WallData	*pWall;
				int			flags;

				for (pWall = tree->u.internal.walls_in_plane; pWall != NULL; pWall = pWall->next)
				{
					flags = 0;

					if (pWall->pos_sidedef)
					{

						if (pWall->pos_sidedef->normal_bmap)
							flags |= D3DRENDER_WALL_NORMAL;

						if (pWall->pos_sidedef->below_bmap)
							flags |= D3DRENDER_WALL_BELOW;

						if (pWall->pos_sidedef->above_bmap)
							flags |= D3DRENDER_WALL_ABOVE;
					}

					if (pWall->neg_sidedef)
					{
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

               // D3DRenderLMapsDynamicPostDraw now relies on checks done while constructing the list
               // of viewable objects, walls etc. in DrawBSP() in drawbsp.c. Walls that can be seen
               // have the respective boolean value set to TRUE, so all we need to do here is check
               // that value to decide whether to render light on the wall. Significantly speeds up
               // rendering large amounts of lights. Old code left commented out as opposed to deleting
               // in case any modification to drawbsp.c is performed which invalidates this method.
					//if (pWall->drawnormal)
               if ((flags & D3DRENDER_WALL_NORMAL) && (((short)pWall->z2 != (short)pWall->z1)
						|| ((short)pWall->zz2 != (short)pWall->zz1)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_NORMAL, side, light, TRUE);
					}

               //if (pWall->drawbelow) 
               if ((flags & D3DRENDER_WALL_BELOW) && (((short)pWall->z1 != (short)pWall->z0)
						|| ((short)pWall->zz1 != (short)pWall->zz0)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_BELOW, side, light, TRUE);
					}

               //if (pWall->drawabove)
               if ((flags & D3DRENDER_WALL_ABOVE) && (((short)pWall->z3 != (short)pWall->z2)
						|| ((short)pWall->zz3 != (short)pWall->zz2)))
					{
						D3DRenderLMapPostWallAdd(pWall, &gLMapPool, D3DRENDER_WALL_ABOVE, side, light, TRUE);
					}
				}
			}
	      
			/* lastly, traverse farther side */
			if (side > 0)
				D3DRenderLMapsDynamicPostDraw(tree->u.internal.neg_side, params, light);
			else
				D3DRenderLMapsDynamicPostDraw(tree->u.internal.pos_side, params, light);
	      
			return;

		default:
			debug(("WalkBSPtree lightmaps error!\n"));
		return;
	}
}

Bool D3DLMapCheck(d_light *dLight, room_contents_node *pRNode)
{
	if (dLight->objID != pRNode->obj.id)
		return FALSE;
	if (dLight->xyzScale.x != DLIGHT_SCALE(pRNode->obj.dLighting.intensity))
		return FALSE;
	if (dLight->color.b != (pRNode->obj.dLighting.color & 31) * 255 / 31)
		return FALSE;
	if (dLight->color.g != ((pRNode->obj.dLighting.color >> 5) & 31) * 255 / 31)
		return FALSE;
	if (dLight->color.r != ((pRNode->obj.dLighting.color >> 10) & 31) * 255 / 31)
		return FALSE;

	return TRUE;
}

void D3DLMapsStaticGet(room_type *room)
{
	room_contents_node	*pRNode;
	list_type			list;
	long				top, bottom;
	int					sector_flags;
	PDIB				pDib;

	for (list = room->projectiles; list != NULL; list = list->next)
	{
		Projectile	*pProjectile = (Projectile *)list->data;

		if (gDLightCacheDynamic.numLights >= 150)
			continue;

		if ((pProjectile->dLighting.color == 0) || (pProjectile->dLighting.intensity == 0))
			continue;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x = pProjectile->motion.x;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y = pProjectile->motion.y;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z = pProjectile->motion.z;

		pDib = GetObjectPdib(pProjectile->icon_res, 0, 0);

		GetRoomHeight(current_room.tree, &top, &bottom, &sector_flags, pProjectile->motion.x, pProjectile->motion.y);

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z =
			max(bottom, pProjectile->motion.z);

		if (pDib)
			gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z +=
				((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x =
			DLIGHT_SCALE(pProjectile->dLighting.intensity);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y =
			DLIGHT_SCALE(pProjectile->dLighting.intensity);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z =
			DLIGHT_SCALE(pProjectile->dLighting.intensity);

      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x +
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x -
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y +
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y -
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.x =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.y =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.z =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.x =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.y =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.z =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z / 2.0f);

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.a = COLOR_MAX;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.r =
			((pProjectile->dLighting.color >> 10) & 31) * COLOR_MAX / 31;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.g =
			((pProjectile->dLighting.color >> 5) & 31) * COLOR_MAX / 31;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.b =
			(pProjectile->dLighting.color & 31) * COLOR_MAX / 31;

		gDLightCacheDynamic.numLights++;
	}

	// dynamic lights
	for (list = room->contents; list != NULL; list = list->next)
	{
		pRNode = (room_contents_node *)list->data;

		if (gDLightCacheDynamic.numLights >= 150)
			continue;

		if ((pRNode->obj.dLighting.flags & LIGHT_FLAG_DYNAMIC) == 0)
			continue;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x = pRNode->motion.x;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y = pRNode->motion.y;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z = pRNode->motion.z;

		pDib = GetObjectPdib(pRNode->obj.icon_res, 0, 0);

		GetRoomHeight(current_room.tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y);

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z =
			max(bottom, pRNode->motion.z);

		if (pDib)
			gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z +=
				((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

		if ((pRNode->obj.dLighting.color == 0) || (pRNode->obj.dLighting.intensity == 0))
			continue;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);

      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x +
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x -
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y +
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;
      gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y -
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.x =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.y =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.z =
			1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z;

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.x =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.y =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f);
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.z =
			1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z / 2.0f);

		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.a = COLOR_MAX;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.r =
			((pRNode->obj.dLighting.color >> 10) & 31) * COLOR_MAX / 31;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.g =
			((pRNode->obj.dLighting.color >> 5) & 31) * COLOR_MAX / 31;
		gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.b =
			(pRNode->obj.dLighting.color & 31) * COLOR_MAX / 31;

		gDLightCacheDynamic.numLights++;
	}

   // fireworks
   emitter *pEmitter;
   particle *pParticle;
   for (list = gParticleSystemFireworks.emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;
      pParticle = &pEmitter->particles[0];
      if (pParticle && pParticle->energy > 0)
      {
         if (gDLightCacheDynamic.numLights >= 150)
            continue;

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x = pParticle->pos.x;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y = pParticle->pos.y;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.z = pParticle->pos.z;

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x =
            DLIGHT_SCALE(pParticle->energy);
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y =
            DLIGHT_SCALE(pParticle->energy);
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z =
            DLIGHT_SCALE(pParticle->energy);

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x +
            gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minX = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.x -
            gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].maxY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y +
            gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].minY = gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyz.y -
            gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f;

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.x =
            1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.y =
            1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScale.z =
            1.0f / gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z;

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.x =
            1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.x / 2.0f);
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.y =
            1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.y / 2.0f);
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].invXYZScaleHalf.z =
            1.0f / (gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].xyzScale.z / 2.0f);

         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.a = COLOR_MAX;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.r = pParticle->bgra.r;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.g = pParticle->bgra.g;
         gDLightCacheDynamic.dLights[gDLightCacheDynamic.numLights].color.b = pParticle->bgra.b;

         gDLightCacheDynamic.numLights++;
      }
   }

	// static lights
	for (list = room->contents; list != NULL; list = list->next)
	{
		pRNode = (room_contents_node *)list->data;

		if (gDLightCache.numLights >= 150)
			continue;

		if (pRNode->obj.dLighting.flags & LIGHT_FLAG_DYNAMIC)
			continue;

		if ((pRNode->obj.dLighting.color == 0) || (pRNode->obj.dLighting.intensity == 0))
			continue;

		if (!D3DLMapCheck(&gDLightCache.dLights[gDLightCache.numLights], pRNode))
			gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;

		pDib = GetObjectPdib(pRNode->obj.icon_res, 0, 0);

		gDLightCache.dLights[gDLightCache.numLights].objID = pRNode->obj.id;

		gDLightCache.dLights[gDLightCache.numLights].xyz.x = pRNode->motion.x;
		gDLightCache.dLights[gDLightCache.numLights].xyz.y = pRNode->motion.y;
		gDLightCache.dLights[gDLightCache.numLights].xyz.z = pRNode->motion.z;

		GetRoomHeight(current_room.tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y);

		gDLightCache.dLights[gDLightCache.numLights].xyz.z =
			max(bottom, pRNode->motion.z);

		if (pDib)
			gDLightCache.dLights[gDLightCache.numLights].xyz.z +=
				((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

		gDLightCache.dLights[gDLightCache.numLights].xyzScale.x =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);
		gDLightCache.dLights[gDLightCache.numLights].xyzScale.y =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);
		gDLightCache.dLights[gDLightCache.numLights].xyzScale.z =
			DLIGHT_SCALE(pRNode->obj.dLighting.intensity);

      gDLightCache.dLights[gDLightCache.numLights].maxX = gDLightCache.dLights[gDLightCache.numLights].xyz.x +
         gDLightCache.dLights[gDLightCache.numLights].xyzScale.x / 2.0f;
      gDLightCache.dLights[gDLightCache.numLights].minX = gDLightCache.dLights[gDLightCache.numLights].xyz.x -
         gDLightCache.dLights[gDLightCache.numLights].xyzScale.x / 2.0f;
      gDLightCache.dLights[gDLightCache.numLights].maxY = gDLightCache.dLights[gDLightCache.numLights].xyz.y +
         gDLightCache.dLights[gDLightCache.numLights].xyzScale.y / 2.0f;
      gDLightCache.dLights[gDLightCache.numLights].minY = gDLightCache.dLights[gDLightCache.numLights].xyz.y -
         gDLightCache.dLights[gDLightCache.numLights].xyzScale.y / 2.0f;

		if (pRNode->obj.dLighting.intensity == 0)
			pRNode->obj.dLighting.intensity = 1;

		gDLightCache.dLights[gDLightCache.numLights].invXYZScale.x =
			1.0f / gDLightCache.dLights[gDLightCache.numLights].xyzScale.x;
		gDLightCache.dLights[gDLightCache.numLights].invXYZScale.y =
			1.0f / gDLightCache.dLights[gDLightCache.numLights].xyzScale.y;
		gDLightCache.dLights[gDLightCache.numLights].invXYZScale.z =
			1.0f / gDLightCache.dLights[gDLightCache.numLights].xyzScale.z;

		gDLightCache.dLights[gDLightCache.numLights].invXYZScaleHalf.x =
			1.0f / (gDLightCache.dLights[gDLightCache.numLights].xyzScale.x / 2.0f);
		gDLightCache.dLights[gDLightCache.numLights].invXYZScaleHalf.y =
			1.0f / (gDLightCache.dLights[gDLightCache.numLights].xyzScale.y / 2.0f);
		gDLightCache.dLights[gDLightCache.numLights].invXYZScaleHalf.z =
			1.0f / (gDLightCache.dLights[gDLightCache.numLights].xyzScale.z / 2.0f);

		gDLightCache.dLights[gDLightCache.numLights].color.a = COLOR_MAX;
		gDLightCache.dLights[gDLightCache.numLights].color.r =
			((pRNode->obj.dLighting.color >> 10) & 31) * COLOR_MAX / 31;
		gDLightCache.dLights[gDLightCache.numLights].color.g =
			((pRNode->obj.dLighting.color >> 5) & 31) * COLOR_MAX / 31;
		gDLightCache.dLights[gDLightCache.numLights].color.b =
			(pRNode->obj.dLighting.color & 31) * COLOR_MAX / 31;

		gDLightCache.numLights++;
	}
}

void D3DGeometryBuildNew(room_type *room, d3d_render_pool_new *pPool, Draw3DParams *params)
{
	int			count;
	BSPnode		*pNode = NULL;
	WallData	*pWall;

	D3DCacheSystemReset(&gWorldCacheSystemStatic);
	D3DCacheSystemReset(&gWallMaskCacheSystem);

	D3DRenderPoolReset(&gWorldPoolStatic, &D3DMaterialWorldPool);
	D3DRenderPoolReset(&gWallMaskPool, &D3DMaterialWallMaskPool);

   // We now precalculate all the wall, ceiling and floor data and save it
   // in the structs. This means D3DFillTreeData *must* be called here with
   // full (3rd parameter) set to TRUE so we fill the entire tree before
   // attempting to use it.
   //long timeFill = timeGetTime();
   D3DFillTreeData(room, params, TRUE);
   //debug(("Time to calculate and fill BSP data is %d\n", timeGetTime() - timeFill));

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

			case BSPleaftype:
				if (pNode->u.leaf.sector == &room->sectors[0])
				{
/*					if ((pNode->bbox.x0 < 0) ||
						(pNode->bbox.x1 > room->width))
						break;*/
//					if (pNode->u.leaf.sector->server_id == 0)
//						break;
/*						if (bDone)
							break;
						else
							bDone = TRUE;*/
				}

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
         for (int numLights = 0; numLights < gDLightCache.numLights; numLights++)
         {
            d_light *light = &gDLightCache.dLights[numLights];
            
            // If the light doesn't reach this bounding box, skip to the next one.
            pNode = &room->nodes[count];
            if (pNode->bbox.x0 >= light->maxX
               || pNode->bbox.x1 <= light->minX
               || pNode->bbox.y0 >= light->maxY
               || pNode->bbox.y1 <= light->minY)
             continue;
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
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_NORMAL, 1, light, FALSE);
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_NORMAL, -1, light, FALSE);
                  }

                  if ((flags & D3DRENDER_WALL_BELOW) && ((short)pWall->z1 != (short)pWall->z0)
                     || ((short)pWall->zz1 != (short)pWall->zz0))
                  {
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_BELOW, 1, light, FALSE);
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_BELOW, -1, light, FALSE);
                  }

                  if ((flags & D3DRENDER_WALL_ABOVE) && ((short)pWall->z3 != (short)pWall->z2)
                     || ((short)pWall->zz3 != (short)pWall->zz2))
                  {
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_ABOVE, 1, light, FALSE);
                       D3DRenderLMapPostWallAdd(pWall, &gLMapPoolStatic, D3DRENDER_WALL_ABOVE, -1, light, FALSE);
                  }
               }

               break;

            case BSPleaftype:
               D3DRenderLMapPostFloorAdd(pNode, &gLMapPoolStatic, light, FALSE);
               D3DRenderLMapPostCeilingAdd(pNode, &gLMapPoolStatic, light, FALSE);
               break;

            default:
               break;
            }
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
				// this causes a lot of z fighting, and I don't think it's necessary anywhere

				//if (pNode->u.leaf.sector->floor == NULL)
				//	D3DRenderFloorMaskAdd(pNode, &gWallMaskPool, FALSE);
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

						lightScale = (long)(a * sun_vect.x +
										b * sun_vect.y) >> LOG_FINENESS;

#if PERPENDICULAR_DARK
						lightScale = ABS(lightScale);
#else
						lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range
#endif

						lightScale = lo_end + ((lightScale * shade_amount)>>LOG_FINENESS);
						
						if (lightScale > FINENESS)
							lightScale = FINENESS;
						else if ( lightScale < 0)
							lightScale = 0;
					}
					else
						lightScale = FINENESS;

//					if (distance <= 0)
//						distance = MAX_DISTANCE;

					if (gD3DDriverProfile.bFogEnable)
						paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightScale, 0);
					else
						paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightScale, 0);
	//				paletteIndex = 64;

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

void D3DRenderPacketFloorAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool bDynamic)
{
	Sector		*pSector = pNode->u.leaf.sector;
	PDIB		pDib;
	int			vertex;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	if (pSector->floor)
	{
		pDib = pSector->floor;
	}
	else
		return;

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

   // We now use the previously extracted data.
   //custom_xyz	xyz[MAX_NPTS];
   //custom_st	st[MAX_NPTS];
   //custom_bgra	bgra[MAX_NPTS];
   //D3DRenderFloorExtract(pNode, pDib, xyz, st, bgra);

//	pChunk->numIndices = (pNode->u.leaf.poly.npts - 2) * 3;
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
		pChunk->xyz[vertex].x = pNode->floor_xyz[vertex].x;
		pChunk->xyz[vertex].y = pNode->floor_xyz[vertex].y;
		pChunk->xyz[vertex].z = pNode->floor_xyz[vertex].z;

		pChunk->st0[vertex].s = pNode->floor_stBase[vertex].s;
      pChunk->st0[vertex].t = pNode->floor_stBase[vertex].t;

		pChunk->bgra[vertex].b = pNode->floor_bgra[vertex].b;
		pChunk->bgra[vertex].g = pNode->floor_bgra[vertex].g;
		pChunk->bgra[vertex].r = pNode->floor_bgra[vertex].r;
      pChunk->bgra[vertex].a = pNode->floor_bgra[vertex].a;
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

void D3DRenderPacketCeilingAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool bDynamic)
{
	Sector		*pSector = pNode->u.leaf.sector;
	PDIB		pDib;
	int			vertex;
	int			left, top;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	left = top = 0;

	if (pSector->ceiling)
	{
		pDib = pSector->ceiling;
	}
	else
		return;

   // We now use the previously extracted data.
   //custom_xyz	xyz[MAX_NPTS];
   //custom_st	st[MAX_NPTS];
   //custom_bgra	bgra[MAX_NPTS];
   //D3DRenderCeilingExtract(pNode, pDib, xyz, st, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

//	pChunk->numIndices = (pNode->u.leaf.poly.npts - 2) * 3;
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
		pChunk->xyz[vertex].x = pNode->ceiling_xyz[vertex].x;
      pChunk->xyz[vertex].y = pNode->ceiling_xyz[vertex].y;
      pChunk->xyz[vertex].z = pNode->ceiling_xyz[vertex].z;

      pChunk->st0[vertex].s = pNode->ceiling_stBase[vertex].s;
      pChunk->st0[vertex].t = pNode->ceiling_stBase[vertex].t;

      pChunk->bgra[vertex].b = pNode->ceiling_bgra[vertex].b;
      pChunk->bgra[vertex].g = pNode->ceiling_bgra[vertex].g;
      pChunk->bgra[vertex].r = pNode->ceiling_bgra[vertex].r;
      pChunk->bgra[vertex].a = pNode->ceiling_bgra[vertex].a;
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

void D3DRenderPacketWallAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type, int side,
							   Bool bDynamic)
{
	Sidedef			*pSideDef = NULL;
	unsigned int	flags = 0;
	PDIB			pDib = NULL;
	int				vertex;
   custom_xyz		xyz[4];
   custom_st		stBase[4];
   custom_bgra		bgra[MAX_NPTS];
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

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
         {
            pDib = pWall->pos_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_normal_xyz[i];
               bgra[i] = pWall->pos_normal_bgra[i];
               stBase[i] = pWall->pos_normal_stBase[i];
               flags = pWall->pos_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->pos_sidedef->below_bmap)
         {
            pDib = pWall->pos_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_below_xyz[i];
               bgra[i] = pWall->pos_below_bgra[i];
               stBase[i] = pWall->pos_below_stBase[i];
               flags = pWall->pos_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->pos_sidedef->above_bmap)
         {
            pDib = pWall->pos_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_above_xyz[i];
               bgra[i] = pWall->pos_above_bgra[i];
               stBase[i] = pWall->pos_above_stBase[i];
               flags = pWall->pos_above_d3dFlags;
            }
         }
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
         {
            pDib = pWall->neg_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_normal_xyz[i];
               bgra[i] = pWall->neg_normal_bgra[i];
               stBase[i] = pWall->neg_normal_stBase[i];
               flags = pWall->neg_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->neg_sidedef->below_bmap)
         {
            pDib = pWall->neg_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_below_xyz[i];
               bgra[i] = pWall->neg_below_bgra[i];
               stBase[i] = pWall->neg_below_stBase[i];
               flags = pWall->neg_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->neg_sidedef->above_bmap)
         {
            pDib = pWall->neg_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_above_xyz[i];
               bgra[i] = pWall->neg_above_bgra[i];
               stBase[i] = pWall->neg_above_stBase[i];
               flags = pWall->neg_above_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      default:
         break;
      }
   }

	if (NULL == pDib)
		return;
//		pDib = current_room.sectors[0].floor;

   // We now use the previously extracted data.
	//D3DRenderWallExtract(pWall, pDib, &flags, xyz, st, bgra, type, side);

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

		pChunk->st0[vertex].s = stBase[vertex].s;
		pChunk->st0[vertex].t = stBase[vertex].t;

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

void D3DRenderPacketWallMaskAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type,
								int side, Bool bDynamic)
{
	Sidedef			*pSideDef = NULL;
	custom_xyz		xyz[4];
	custom_st		stBase[4];
	custom_bgra		bgra[MAX_NPTS];
	unsigned int	flags;
	PDIB			pDib = NULL;
	int				vertex;
	Bool			bNoVTile = FALSE;
	Bool			bNoLookThrough = FALSE;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

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
         {
            pDib = pWall->pos_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_normal_xyz[i];
               bgra[i] = pWall->pos_normal_bgra[i];
               stBase[i] = pWall->pos_normal_stBase[i];
               flags = pWall->pos_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->pos_sidedef->below_bmap)
         {
            pDib = pWall->pos_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_below_xyz[i];
               bgra[i] = pWall->pos_below_bgra[i];
               stBase[i] = pWall->pos_below_stBase[i];
               flags = pWall->pos_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->pos_sidedef->above_bmap)
         {
            pDib = pWall->pos_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_above_xyz[i];
               bgra[i] = pWall->pos_above_bgra[i];
               stBase[i] = pWall->pos_above_stBase[i];
               flags = pWall->pos_above_d3dFlags;
            }
         }
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
         {
            pDib = pWall->neg_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_normal_xyz[i];
               bgra[i] = pWall->neg_normal_bgra[i];
               stBase[i] = pWall->neg_normal_stBase[i];
               flags = pWall->neg_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->neg_sidedef->below_bmap)
         {
            pDib = pWall->neg_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_below_xyz[i];
               bgra[i] = pWall->neg_below_bgra[i];
               stBase[i] = pWall->neg_below_stBase[i];
               flags = pWall->neg_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->neg_sidedef->above_bmap)
         {
            pDib = pWall->neg_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_above_xyz[i];
               bgra[i] = pWall->neg_above_bgra[i];
               stBase[i] = pWall->neg_above_stBase[i];
               flags = pWall->neg_above_d3dFlags;
            }
         }
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

   // We now use the previously extracted data.
	//D3DRenderWallExtract(pWall, pDib, &flags, xyz, st, bgra, type, side);

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

//	if (bNoVTile)
//		pChunk->flags |= D3DRENDER_NOCULL;

	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < 4; vertex++)
	{
		pChunk->xyz[vertex].x = xyz[vertex].x;
		pChunk->xyz[vertex].y = xyz[vertex].y;
		pChunk->xyz[vertex].z = xyz[vertex].z;

		pChunk->st0[vertex].s = stBase[vertex].s;
		pChunk->st0[vertex].t = stBase[vertex].t;

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
					xyz[0].z = (float)pWall->z2;
               xyz[3].z = (float)pWall->zz2;
				}
				break;

				case D3DRENDER_WALL_BELOW:
				{
               xyz[0].z = (float)pWall->z1;
               xyz[3].z = (float)pWall->zz1;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
               xyz[0].z = (float)pWall->z3;
               xyz[3].z = (float)pWall->zz3;
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
               xyz[0].z = (float)pWall->zz2;
               xyz[3].z = (float)pWall->z2;
				}
				break;

				case D3DRENDER_WALL_BELOW:
				{
               xyz[0].z = (float)pWall->zz1;
               xyz[3].z = (float)pWall->z1;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
               xyz[0].z = (float)pWall->zz3;
               xyz[3].z = (float)pWall->z3;
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

			pChunk->st0[vertex].s = stBase[vertex].s;
			pChunk->st0[vertex].t = stBase[vertex].t;

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

void D3DRenderFloorMaskAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool bDynamic)
{
	Sector		*pSector = pNode->u.leaf.sector;
	int			vertex;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

   // We now use the previously extracted data.
	//D3DRenderFloorExtract(pNode, NULL, xyz, NULL, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, gpNoLookThrough, NULL, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

//	pChunk->numIndices = (pNode->u.leaf.poly.npts - 2) * 3;
	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->zBias = ZBIAS_MASK;
	
	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = pNode->floor_xyz[vertex].x;
		pChunk->xyz[vertex].y = pNode->floor_xyz[vertex].y;
		pChunk->xyz[vertex].z = pNode->floor_xyz[vertex].z;

		pChunk->bgra[vertex].b = pNode->floor_bgra[vertex].b;
      pChunk->bgra[vertex].g = pNode->floor_bgra[vertex].g;
      pChunk->bgra[vertex].r = pNode->floor_bgra[vertex].r;
      pChunk->bgra[vertex].a = pNode->floor_bgra[vertex].a;
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

void D3DRenderCeilingMaskAdd(BSPnode *pNode, d3d_render_pool_new *pPool, Bool bDynamic)
{
	Sector		*pSector = pNode->u.leaf.sector;
	int			vertex;
	int			left, top;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	left = top = 0;

   // We now use the previously extracted data.
	//D3DRenderCeilingExtract(pNode, NULL, xyz, NULL, bgra);

	pPacket = D3DRenderPacketFindMatch(pPool, gpNoLookThrough, NULL, 0, 0, 0);
	if (NULL == pPacket)
		return;
	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);

//	pChunk->numIndices = (pNode->u.leaf.poly.npts - 2) * 3;
	pChunk->numVertices = pNode->u.leaf.poly.npts;
	pChunk->numIndices = pChunk->numVertices;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	pChunk->pSector = pSector;
	pChunk->zBias = ZBIAS_MASK;
	
	pPacket->pMaterialFctn = &D3DMaterialWorldPacket;

	pChunk->pMaterialFctn = &D3DMaterialMaskChunk;

	for (vertex = 0; vertex < pNode->u.leaf.poly.npts; vertex++)
	{
		pChunk->xyz[vertex].x = pNode->ceiling_xyz[vertex].x;
      pChunk->xyz[vertex].y = pNode->ceiling_xyz[vertex].y;
      pChunk->xyz[vertex].z = pNode->ceiling_xyz[vertex].z;

      pChunk->bgra[vertex].b = pNode->ceiling_bgra[vertex].b;
      pChunk->bgra[vertex].g = pNode->ceiling_bgra[vertex].g;
      pChunk->bgra[vertex].r = pNode->ceiling_bgra[vertex].r;
      pChunk->bgra[vertex].a = pNode->ceiling_bgra[vertex].a;
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

         pChunk->xyz[vertex].x = pNode->ceiling_xyz[i].x;
         pChunk->xyz[vertex].y = pNode->ceiling_xyz[i].y;
         pChunk->xyz[vertex].z = pNode->ceiling_xyz[i].z;

         pChunk->bgra[vertex].b = pNode->ceiling_bgra[i].b;
         pChunk->bgra[vertex].g = pNode->ceiling_bgra[i].g;
         pChunk->bgra[vertex].r = pNode->ceiling_bgra[i].r;
         pChunk->bgra[vertex].a = pNode->ceiling_bgra[i].a;

			pChunk->indices[vertex] = vertex;

			vertex++;

         pChunk->xyz[vertex].x = pNode->ceiling_xyz[i].x;
         pChunk->xyz[vertex].y = pNode->ceiling_xyz[i].y;
			pChunk->xyz[vertex].z = 65535;

         pChunk->bgra[vertex].b = pNode->ceiling_bgra[i].b;
         pChunk->bgra[vertex].g = pNode->ceiling_bgra[i].g;
         pChunk->bgra[vertex].r = pNode->ceiling_bgra[i].r;
         pChunk->bgra[vertex].a = pNode->ceiling_bgra[i].a;

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

int D3DRenderObjectGetLight(BSPnode *tree, room_contents_node *pRNode)
{
	long side0;
	BSPnode *pos, *neg;
   
	while (1)
	{
		if (tree == NULL)
		{
			return False;
		}

		switch(tree->type)
		{
			case BSPleaftype:
        return tree->u.leaf.sector->light;

			case BSPinternaltype:
				side0 = tree->u.internal.separator.a * pRNode->motion.x +
				tree->u.internal.separator.b * pRNode->motion.y +
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
			return False;
		}
	}
}

float D3DRenderObjectLightGetNearest(room_contents_node *pRNode)
{
	int		numLights;
	float	lastDistance, distance;

	lastDistance = 0;

	for (numLights = 0; numLights < gDLightCache.numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - gDLightCache.dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - gDLightCache.dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - gDLightCache.dLights[numLights].xyz.z);// * 4.0f;

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (gDLightCache.dLights[numLights].xyzScale.x / 2.0f);

		if (0 == numLights)
			lastDistance = distance;
		else if (distance < lastDistance)
			lastDistance = distance;
	}

	for (numLights = 0; numLights < gDLightCacheDynamic.numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - gDLightCacheDynamic.dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - gDLightCacheDynamic.dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - gDLightCacheDynamic.dLights[numLights].xyz.z);// * 4.0f;

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (gDLightCacheDynamic.dLights[numLights].xyzScale.x / 2.0f);

		if (distance < lastDistance)
			lastDistance = distance;
	}

	if (gDLightCache.numLights || gDLightCacheDynamic.numLights)
	{
		lastDistance = 1.0f - lastDistance;
		lastDistance = max(0, lastDistance);
//		lastDistance = 255 * lastDistance;
	}

	return lastDistance;
}

Bool D3DObjectLightingCalc(room_type *room, room_contents_node *pRNode, custom_bgra *bgra, DWORD flags)
{
	int			light, intDistance, numLights;
	d_light		*pDLight = NULL;
	float		distX, distY;
	float		lastDistance, distance;
	Bool		bFogDisable = FALSE;

	lastDistance = DLIGHT_SCALE(255);

	for (numLights = 0; numLights < gDLightCache.numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - gDLightCache.dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - gDLightCache.dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - gDLightCache.dLights[numLights].xyz.z);// * 4.0f;

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (gDLightCache.dLights[numLights].xyzScale.x / 2.0f);

		if (distance < lastDistance)
		{
			lastDistance = distance;
			pDLight = &gDLightCache.dLights[numLights];
		}
	}

	for (numLights = 0; numLights < gDLightCacheDynamic.numLights; numLights++)
	{
		custom_xyz	vector;

		vector.x = pRNode->motion.x - gDLightCacheDynamic.dLights[numLights].xyz.x;
		vector.y = pRNode->motion.y - gDLightCacheDynamic.dLights[numLights].xyz.y;
		vector.z = (pRNode->motion.z - gDLightCacheDynamic.dLights[numLights].xyz.z);// * 4.0f;

		distance = (vector.x * vector.x) + (vector.y * vector.y) +
			(vector.z * vector.z);
		distance = (float)sqrt((double)distance);

		distance /= (gDLightCacheDynamic.dLights[numLights].xyzScale.x / 2.0f);

		if (distance < lastDistance)
		{
			lastDistance = distance;
			pDLight = &gDLightCacheDynamic.dLights[numLights];
		}
	}

	lastDistance = 1.0f - lastDistance;
	lastDistance = max(0, lastDistance);
	lastDistance = COLOR_AMBIENT * lastDistance;

	light = D3DRenderObjectGetLight(room->tree, pRNode);

	if (light <= 127)
		bFogDisable = TRUE;

	distX = pRNode->motion.x - player.x;
	distY = pRNode->motion.y - player.y;

	intDistance = DistanceGet(distX, distY);

	if (gD3DDriverProfile.bFogEnable && ((flags & D3DRENDER_NOAMBIENT) == 0))
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

Bool D3DComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area)
{
	float	screenW, screenH;

	if (stretchfactor == 2)
	{
		screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)(MAXX * (stretchfactor * 1.75f));
		screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * (stretchfactor * 2.25f));
	}
	else
	{
		screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)(MAXX * (stretchfactor * 1.25f));
		screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * (stretchfactor * 1.5f));
	}

   if (hotspot < 1 || hotspot > HOTSPOT_PLAYER_MAX)
   {
      debug(("ComputePlayerOverlayArea found hotspot out of range (%d).\n", (int) hotspot));
      return False;
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
      obj_area->x = gScreenWidth - DibWidth(pdib) / (float)screenW;
      break;

   case HOTSPOT_N:
   case HOTSPOT_S:
   case HOTSPOT_CENTER:
      obj_area->x = (gScreenWidth - DibWidth(pdib) / (float)screenW) / 2;
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
      obj_area->y = gScreenHeight - DibHeight(pdib) / (float)screenH;
      break;

   case HOTSPOT_W:
   case HOTSPOT_E:
   case HOTSPOT_CENTER:
      obj_area->y = (gScreenHeight - DibHeight(pdib) / (float)screenH) / 2;
      break;
   }

   obj_area->x += DibXOffset(pdib) / (float)screenW;
   obj_area->y += DibYOffset(pdib) / (float)screenH;
   obj_area->cx = DibWidth(pdib) / (float)screenW;
   obj_area->cy = DibHeight(pdib) / (float)screenH;
   return True;
}

void D3DRenderNamesDraw3D(d3d_render_cache_system *pCacheSystem, d3d_render_pool_new *pPool,
						room_type *room, Draw3DParams *params, font_3d *pFont)
{
	D3DMATRIX			mat, rot, xForm, trans;
	int					angleHeading, anglePitch, strLen, sector_flags, offset;
	room_contents_node	*pRNode;
	list_type			list;
	long				dx, dy, angle, top, bottom;
	PDIB				pDib;
	custom_xyz			vector;
	custom_st			st[4];
	custom_bgra			bgra;
	float				lastDistance, width, height, x, z, depth, distance;
	char				*pName, *ptr;
	TCHAR				c;
	COLORREF			fg_color;
	Color				color;
	BYTE				*palette;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	// base objects
	for (list = room->contents; list != NULL; list = list->next)
	{
      float glyph_scale = 255;
      
		pRNode = (room_contents_node *)list->data;

		if (pRNode->obj.id == player.id)
			continue;

		if (!(pRNode->obj.flags & OF_PLAYER) || (pRNode->obj.drawingtype == DRAWFX_INVISIBLE))
			continue;

		vector.x = pRNode->motion.x - player.x;
		vector.y = pRNode->motion.y - player.y;

		distance = (vector.x * vector.x) + (vector.y * vector.y);
		distance = (float)sqrt((double)distance);
		if (distance <= 0)
			distance = 1;

		if (distance >= MAX_NAME_DISTANCE)
			continue;

		pDib = GetObjectPdib(pRNode->obj.icon_res, 0, pRNode->obj.animate->group);

		if (NULL == pDib)
			continue;

		dx = pRNode->motion.x - params->viewer_x;
		dy = pRNode->motion.y - params->viewer_y;

		angle = (pRNode->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

		pName = LookupNameRsc(pRNode->obj.name_res);
		strLen = strlen(pName);

		angle = pRNode->angle - (params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		/* Make sure that object is above the floor. */
		if (!GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
		{
			continue;
		}

		// Set object depth based on "depth" sector flags
		depth = sector_depths[SectorDepth(sector_flags)];

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

		//			z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

//			z += ((float)pRNode->boundingHeightAdjust * 4.0f);// / 1.66f;

		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, (float)pRNode->motion.x, (float)max(bottom,
			pRNode->motion.z) - depth +
			(((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f) +
			((float)pRNode->boundingHeightAdjust * 4.0f), (float)pRNode->motion.y);
		MatrixMultiply(&xForm, &rot, &mat);

		fg_color = GetPlayerNameColor(&pRNode->obj, pName);

		// Some names never grow darker, they use PALETTEINDEX().
		if (HIBYTE(HIWORD(fg_color)) == HIBYTE(HIWORD(PALETTEINDEX(0))))
		{
			//XXX: normally, SetTextColor() works with PALETTEINDEX() types fine,
			//     but not here for unknown reason
			//     so we convert to our base_palette[] PALETTERGB() type.
			//
			color = base_palette[LOBYTE(LOWORD(fg_color))];
		}
		else
		{
			// Draw name with color that fades with distance, just like object
			if (pRNode->obj.flags & (OF_FLICKERING | OF_FLASHING))
			{
				//palette = GetLightPalette(object->distance, object->light, FINENESS,GetFlicker(r));
				palette = GetLightPalette(D3DRENDER_LIGHT_DISTANCE, 63, FINENESS,0);
			}
			else
			{
				//palette = GetLightPalette(object->distance, object->light, FINENESS,0);
				palette = GetLightPalette(D3DRENDER_LIGHT_DISTANCE, 63, FINENESS,0);
			}
			color = base_palette[palette[GetClosestPaletteIndex(fg_color)]];
			D3DObjectLightingCalc(room, pRNode, &bgra, 0);

			glyph_scale = max(bgra.b, bgra.g);
			glyph_scale = max(glyph_scale, bgra.r);
		}

      bgra.r = color.red * glyph_scale / 255;
      bgra.g = color.green * glyph_scale / 255;
      bgra.b = color.blue * glyph_scale / 255;
      bgra.a = COLOR_MAX;
      
		lastDistance = 0;

		for (offset = 0; offset <= 1; offset++)
		{
			x = 0.0f;
			z = 0;
//			z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

//			z += ((float)pRNode->boundingHeightAdjust * 4.0f);// / 1.66f;

			ptr = pName;

			while (c = *ptr++)
			{
				x += (pFont->texST[c - 32][1].s - pFont->texST[c - 32][0].s) *
					pFont->texWidth / pFont->texScale * (distance / FINENESS);
			}

			ptr = pName;

			while (c = *ptr++)
			{
				int	i;

				// flip t values since bmps are upside down
				st[0].s = pFont->texST[c - 32][0].s;
				st[0].t = pFont->texST[c - 32][1].t;
				st[1].s = pFont->texST[c - 32][0].s;
				st[1].t = pFont->texST[c - 32][0].t;
				st[2].s = pFont->texST[c - 32][1].s;
				st[2].t = pFont->texST[c - 32][0].t;
				st[3].s = pFont->texST[c - 32][1].s;
				st[3].t = pFont->texST[c - 32][1].t;

				width = (st[2].s - st[0].s) * pFont->texWidth * 2.0f / pFont->texScale *
					(distance / FINENESS);
				height = (st[0].t - st[2].t) * pFont->texHeight * 2.0f / pFont->texScale *
					(distance / FINENESS);

				pPacket = D3DRenderPacketFindMatch(pPool, pFont->pTexture, NULL, 0, 0, 0);
				if (NULL == pPacket)
					return;
				pChunk = D3DRenderChunkNew(pPacket);
				assert(pChunk);

				pChunk->flags = pRNode->obj.flags;
				pChunk->numIndices = 4;
				pChunk->numVertices = 4;
				pChunk->numPrimitives = pChunk->numVertices - 2;

				// zBias is scaled way differently in Dx9
				// try to put names as much in the front as possible
				if (offset == 0)
					pChunk->zBias = 255;
				else
					pChunk->zBias = 254;

				if (offset)
				{
					//MatrixScale(&scale, 1.01f);
					MatrixTranslate(&trans, -30.0f * distance / MAX_NAME_DISTANCE,
						-30.0f * distance / MAX_NAME_DISTANCE, 0);
					MatrixMultiply(&pChunk->xForm, &trans, &xForm);
				}
				else
					MatrixCopy(&pChunk->xForm, &xForm);

				pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
				pChunk->pMaterialFctn = &D3DMaterialObjectChunk;

				pChunk->xyz[0].x = x;
				pChunk->xyz[0].y = 0;
				pChunk->xyz[0].z = z;

				pChunk->st0[0].s = st[0].s;
				pChunk->st0[0].t = st[0].t;

				pChunk->xyz[1].x = x;
				pChunk->xyz[1].y = 0;
				pChunk->xyz[1].z = z + height;

				pChunk->st0[1].s = st[1].s;
				pChunk->st0[1].t = st[1].t;

				pChunk->xyz[2].x = x - width;
				pChunk->xyz[2].y = 0;
				pChunk->xyz[2].z = z + height;

				pChunk->st0[2].s = st[2].s;
				pChunk->st0[2].t = st[2].t;

				pChunk->xyz[3].x = x - width;
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

				if (pRNode->obj.drawingtype == DRAWFX_SECONDTRANS)
				{
					pChunk->xLat0 = 0;
					pChunk->xLat1 = pRNode->obj.secondtranslation;
				}
				else if (pRNode->obj.drawingtype == DRAWFX_DOUBLETRANS)
				{
					pChunk->xLat0 = pRNode->obj.translation;
					pChunk->xLat1 = pRNode->obj.secondtranslation;
				}
				else
				{
					pChunk->xLat0 = pRNode->obj.translation;
					pChunk->xLat1 = 0;
				}

				x -= width;
			}
		}
	}
}

// straight texture loader for objects
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char		*pBits = NULL;
	unsigned int		w, h;
	unsigned short		*pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth,	newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;
	Color				lastColor;

	lastColor.red = 128;
	lastColor.green = 128;
	lastColor.blue = 128;

	D3DRenderPaletteSetNew(xLat0, xLat1, effect);
	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & pDib->width))
		w = w >> 1;

	while (!(h & pDib->height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (pDib->width < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (w != pDib->width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - pDib->width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = pDib->width - newWidth;
		skipValW = 1;
	}

	if (pDib->height < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (h != pDib->height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - pDib->height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = pDib->height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = DibPtr(pDib);

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);

	if (pTexture == NULL)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short *)lockedRect.pBits;

	for (si = 0, di = 0; di < newHeight; si++, di++)
	{
		if (diffHeight)
			if ((l += diffHeight) >= 0)
			{
				si += skipValH;
				l -= newHeight;
			}

		for (dj = 0, sj = 0; dj < newWidth; dj++, sj++)
		{
			if (diffWidth)
				if ((k += diffWidth) >= 0)
				{
					sj += skipValW;
					k -= newWidth;
				}

			// 16bit 1555 textures
			if (gPalette[pBits[si * pDib->width + sj]].peFlags != 0)
			{
				pPixels16[di * pitchHalf + dj] =
					(gPalette[pBits[si * pDib->width + sj]].peBlue >> 3) |
					((gPalette[pBits[si * pDib->width + sj]].peGreen >> 3) << 5) |
					((gPalette[pBits[si * pDib->width + sj]].peRed >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[si * pDib->width + sj]].peFlags ? (1 << 15) : 0;

				lastColor.red = gPalette[pBits[si * pDib->width + sj]].peRed;
				lastColor.green = gPalette[pBits[si * pDib->width + sj]].peGreen;
				lastColor.blue = gPalette[pBits[si * pDib->width + sj]].peBlue;
			}
			else
			{
				pPixels16[di * pitchHalf + dj] =
					(lastColor.blue >> 3) |
					((lastColor.green >> 3) << 5) |
					((lastColor.red >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[si * pDib->width + sj]].peFlags ? (1 << 15) : 0;
			}
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
            gpD3DDevice, (IDirect3DBaseTexture9 *) pTexture,
            (IDirect3DBaseTexture9 *) pTextureFinal);
		}
		IDirect3DDevice9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

// texture loader that rotates for walls and such
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  BYTE effect)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char		*pBits = NULL;
	unsigned int		w, h;
	unsigned short		*pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth,	newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;
	Color				lastColor;

	lastColor.red = 128;
	lastColor.green = 128;
	lastColor.blue = 128;

	D3DRenderPaletteSetNew(xLat0, xLat1, effect);

	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & pDib->width))
		w = w >> 1;

	while (!(h & pDib->height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (pDib->width < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (w != pDib->width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - pDib->width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = pDib->width - newWidth;
		skipValW = 1;
	}

	if (pDib->height < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (h != pDib->height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - pDib->height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = pDib->height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = DibPtr(pDib);

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);
   
	if (NULL == pTexture)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short *)lockedRect.pBits;

	for (si = 0, di = 0; di < newWidth; si++, di++)
	{
		if (diffWidth)
			if ((k += diffWidth) >= 0)
			{
				si += skipValW;
				k -= newWidth;
			}

		for (dj = 0, sj = 0; dj < newHeight; dj++, sj++)
		{
			if (diffHeight)
				if ((l += diffHeight) >= 0)
				{
					sj += skipValH;
					l -= newHeight;
				}

			// 16bit 1555 textures
			if (gPalette[pBits[(sj * pDib->width) + si]].peFlags != 0)
			{
				pPixels16[di * pitchHalf + dj] =
					(gPalette[pBits[(sj * pDib->width) + si]].peBlue >> 3) |
					((gPalette[pBits[(sj * pDib->width) + si]].peGreen >> 3) << 5) |
					((gPalette[pBits[(sj * pDib->width) + si]].peRed >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[(sj * pDib->width) + si]].peFlags ? (1 << 15) : 0;

				lastColor.red = gPalette[pBits[(sj * pDib->width) + si]].peRed;
				lastColor.green = gPalette[pBits[(sj * pDib->width) + si]].peGreen;
				lastColor.blue = gPalette[pBits[(sj * pDib->width) + si]].peBlue;
			}
			else
			{
				pPixels16[di * pitchHalf + dj] =
					(lastColor.blue >> 3) |
					((lastColor.green >> 3) << 5) |
					((lastColor.red >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[(sj * pDib->width) + si]].peFlags ? (1 << 15) : 0;
			}
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
            gpD3DDevice, (IDirect3DBaseTexture9 *) pTexture,
            (IDirect3DBaseTexture9 *) pTextureFinal);
		}
		IDirect3DDevice9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromResource(BYTE *ptr, int width, int height)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char		*pBits = NULL;
	unsigned int		w, h;
	unsigned short		*pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth,	newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;

	D3DRenderPaletteSetNew(0, 0, 0);

	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & width))
		w = w >> 1;

	while (!(h & height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (width < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (w != width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = width - newWidth;
		skipValW = 1;
	}

	if (height < D3DRENDER_TEXTURE_THRESHOLD)
	{
		if (h != height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = ptr;

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);
   
	if (NULL == pTexture)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short *)lockedRect.pBits;

//	for (dj = 0, sj = 0; dj < newHeight; dj++, sj++)
	for (dj = newHeight - 1, sj = 0; dj >= 0; dj--, sj++)
	{
		if (diffHeight)
			if ((l += diffHeight) >= 0)
			{
				sj += skipValH;
				l -= newHeight;
			}

		for (si = 0, di = 0; di < newWidth; si++, di++)
		{
			if (diffWidth)
				if ((k += diffWidth) >= 0)
				{
					si += skipValW;
					k -= newWidth;
				}

			// 16bit 1555 textures
			pPixels16[dj * pitchHalf + di] =
				(gPalette[pBits[(sj * width) + si]].peBlue >> 3) |
				((gPalette[pBits[(sj * width) + si]].peGreen >> 3) << 5) |
				((gPalette[pBits[(sj * width) + si]].peRed >> 3) << 10);
			pPixels16[dj * pitchHalf + di] |=
				gPalette[pBits[(sj * width) + si]].peFlags ? (1 << 15) : 0;
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
                                     D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
                     gpD3DDevice, (IDirect3DBaseTexture9 *) pTexture,
                     (IDirect3DBaseTexture9 *) pTextureFinal);
		}
		IDirect3DDevice9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromPNG(char *pFilename)
{
	FILE	*pFile;
	png_structp	pPng = NULL;
	png_infop	pInfo = NULL;
	png_infop	pInfoEnd = NULL;
	png_bytepp   rows;

	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	unsigned char		*pBits = NULL;
	unsigned int		w, h, b;
	int					pitchHalf, bytePP, stride;

	pFile = fopen(pFilename, "rb");
	if (pFile == NULL)
		return NULL;

	pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == pPng)
	{
		fclose(pFile);
		return NULL;
	}

	pInfo = png_create_info_struct(pPng);
	if (NULL == pInfo)
	{
		png_destroy_read_struct(&pPng, NULL, NULL);
		fclose(pFile);
		return NULL;
	}

	pInfoEnd = png_create_info_struct(pPng);
	if (NULL == pInfoEnd)
	{
		png_destroy_read_struct(&pPng, &pInfo, NULL);
		fclose(pFile);
		return NULL;
	}

	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
		fclose(pFile);
		return NULL;
	}

	png_init_io(pPng, pFile);
	png_read_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
	rows = png_get_rows(pPng, pInfo);

	bytePP = pPng->pixel_depth / 8;
	stride = pPng->width * bytePP - bytePP;

	{
		int	i;
		png_bytep	curRow;

		for (i = 0; i < 6; i++)
		{
			IDirect3DDevice9_CreateTexture(gpD3DDevice, pPng->width, pPng->height, 1, 0,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL);

			IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

			pitchHalf = lockedRect.Pitch / 2;

			pBits = (unsigned char *)lockedRect.pBits;

			for (h = 0; h < pPng->height; h++)
			{
				curRow = rows[h];

				for (w = 0; w < pPng->width; w++)
				{
					for (b = 0; b < 4; b++)
					{
						if (b == 3)
							pBits[h * lockedRect.Pitch + w * 4 + b] = 255;
						else
							pBits[h * lockedRect.Pitch + w * 4 + (3 - b)] =
								curRow[(w * bytePP) + b];
//							pBits[h * lockedRect.Pitch + w * 4 + b] =
//								curRow[(stride) - (w * bytePP) + b];
					}
				}
			}

			IDirect3DTexture9_UnlockRect(pTexture, 0);
		}
	}

	png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
	fclose(pFile);

	return pTexture;
}

void D3DRenderResizeDisplay(int left, int top, int right, int bottom)
{
	gD3DRect.left = left;
	gD3DRect.right = left + right;
	gD3DRect.top = top;
	gD3DRect.bottom = top + bottom;
}

void D3DRenderEnableToggle(void)
{
	gD3DEnabled = 1 - gD3DEnabled;

	if (gD3DEnabled)
	{
		memset(gBits, 0, MAXX * MAXY);
		memset(gBufferBits, 0, MAXX * 2 * MAXY * 2);
	}
}

int D3DRenderIsEnabled(void)
{
	return gD3DEnabled;
}

void D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, BYTE flags)
{
	xlat	*pXLat0, *pXLat1, *pXLatGrey;
	Color	*pPalette;
	int		i;

	return;

	pXLat0 = FindStandardXlat(xlatID0);
	pXLat1 = FindStandardXlat(xlatID1);

	pPalette = base_palette;

	switch (flags)
	{
		case DRAWFX_DRAW_PLAIN:
		case DRAWFX_DOUBLETRANS:
		case DRAWFX_BLACK:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_TRANSLUCENT25:
		case DRAWFX_TRANSLUCENT50:
		case DRAWFX_TRANSLUCENT75:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_INVISIBLE:
		break;

		case DRAWFX_TRANSLATE:
		break;

		case DRAWFX_DITHERINVIS:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_DITHERGREY:
			pXLatGrey = FindStandardXlat(XLAT_FILTERWHITE90);
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLatGrey)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLatGrey)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLatGrey)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_DITHERTRANS:
			if ((0 == xlatID1) || (xlatID0 == xlatID1))
			{
				for (i = 0; i < 256; i++)
				{
					gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
					gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
					gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

					if (i == 254)
						gPalette[i].peFlags = 0;
					else
						gPalette[i].peFlags = 255;
				}
			}
			else
			{
				for (i = 0; i < 256; i++)
				{
					gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
					gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
					gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

					if (i == 254)
						gPalette[i].peFlags = 0;
					else
						gPalette[i].peFlags = 255;
				}
			}
		break;

		case DRAWFX_SECONDTRANS:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		default:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;
	}

	IDirect3DDevice9_SetPaletteEntries(gpD3DDevice, 0, gPalette);

	IDirect3DDevice9_SetCurrentTexturePalette(gpD3DDevice, 0);
}

void D3DRenderPaletteSetNew(UINT xlatID0, UINT xlatID1, BYTE flags)
{
	xlat	*pXLat0, *pXLat1, *pXLatGrey;
	Color	*pPalette;
	int		i;
	BYTE	effect;

	pXLat0 = FindStandardXlat(xlatID0);
	pXLat1 = FindStandardXlat(xlatID1);

	pPalette = base_palette;
	effect = flags;

	switch (effect)
	{
		case DRAWFX_DRAW_PLAIN:
		case DRAWFX_DOUBLETRANS:
		case DRAWFX_BLACK:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_TRANSLUCENT25:
		case DRAWFX_TRANSLUCENT50:
		case DRAWFX_TRANSLUCENT75:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_INVISIBLE:
		break;

		case DRAWFX_TRANSLATE:
		break;

		case DRAWFX_DITHERINVIS:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_DITHERGREY:
			pXLatGrey = FindStandardXlat(XLAT_FILTERWHITE90);
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLatGrey)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLatGrey)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLatGrey)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case DRAWFX_DITHERTRANS:
			if ((0 == xlatID1) || (xlatID0 == xlatID1))
			{
				for (i = 0; i < 256; i++)
				{
					gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
					gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
					gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

					if (i == 254)
						gPalette[i].peFlags = 0;
					else
						gPalette[i].peFlags = 255;
				}
			}
			else
			{
				for (i = 0; i < 256; i++)
				{
					gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
					gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
					gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

					if (i == 254)
						gPalette[i].peFlags = 0;
					else
						gPalette[i].peFlags = 255;
				}
			}
		break;

		case DRAWFX_SECONDTRANS:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		default:
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;
	}
}

void D3DRenderLMapsBuild(void)
{
	D3DLOCKED_RECT	lockedRect;
	unsigned char	*pBits = NULL;
	int				width, height;

	// white glow
	IDirect3DDevice9_CreateTexture(gpD3DDevice, 32, 32, 1, 0,
		D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpDLightWhite, NULL);

	IDirect3DTexture9_LockRect(gpDLightWhite, 0, &lockedRect, NULL, 0);

	pBits = (unsigned char *)lockedRect.pBits;

	for (height = 0; height < 32; height++)
	{
		for (width = 0; width < 32; width++)
		{
			float	scale = (float)sqrt((double)((height - 16) * (height - 16) +
											(width - 16) * (width - 16)));
			scale = 16.0f - scale;
			scale = max(scale, 0);
			scale /= 16.0f;

			if ((height == 0) || (height == 31) ||
				(width == 0) || (width == 31))
				scale = 0;

			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255;
		}
	}

	IDirect3DTexture9_UnlockRect(gpDLightWhite, 0);

	// orange glow
	IDirect3DDevice9_CreateTexture(gpD3DDevice, 32, 32, 1, 0,
		D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpDLightOrange, NULL);

	IDirect3DTexture9_LockRect(gpDLightOrange, 0, &lockedRect, NULL, 0);

	pBits = (unsigned char *)lockedRect.pBits;

	for (height = 0; height < 32; height++)
	{
		for (width = 0; width < 32; width++)
		{
			float	scale = (float)sqrt((double)((height - 16) * (height - 16) +
											(width - 16) * (width - 16)));

			scale = 16.0f - scale;
			scale = max(scale, 0);
			scale /= 16.0f;

			if ((height == 0) || (height == 31) ||
				(width == 0) || (width == 31))
				scale = 0;

			*(pBits++) = LIGHTMAP_B * scale;
			*(pBits++) = LIGHTMAP_G * scale;
			*(pBits++) = LIGHTMAP_R * scale;
			*(pBits++) = COLOR_MAX * scale;
		}
	}

	IDirect3DTexture9_UnlockRect(gpDLightOrange, 0);

	// sun texture
	IDirect3DDevice9_CreateTexture(gpD3DDevice, 128, 128, 1, 0,
                                  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpSunTex, NULL);

	IDirect3DTexture9_LockRect(gpSunTex, 0, &lockedRect, NULL, 0);

	pBits = (unsigned char *)lockedRect.pBits;

	for (height = 0; height < 128; height++)
	{
		for (width = 0; width < 128; width++)
		{
			float	scale = (float)sqrt((double)((height - 64) * (height - 64) +
											(width - 64) * (width - 64)));

			scale = 64.0f - scale;
			scale = max(scale, 0);
			scale /= 64.0f;

			if (scale > 0)
				scale = 1.0f;

//			if ((height == 0) || (height == 63) ||
//				(width == 0) || (width == 63))
//				scale = 0;

			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
		}
	}

	IDirect3DTexture9_UnlockRect(gpSunTex, 0);

	// sun bloom texture
	IDirect3DDevice9_CreateTexture(gpD3DDevice, 32, 32, 1, 0,
                                  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpBloom, NULL);

	IDirect3DTexture9_LockRect(gpBloom, 0, &lockedRect, NULL, 0);

	pBits = (unsigned char *)lockedRect.pBits;

	for (height = 0; height < 32; height++)
	{
		for (width = 0; width < 32; width++)
		{
			float	scale = (float)sqrt((double)((height - 16) * (height - 16) +
											(width - 16) * (width - 16)));
			float	scaleAlpha;

			scale = 16.0f - scale;
			scale = max(scale, 0);
			scale /= 16.0f;

			if ((height == 0) || (height == 31) ||
				(width == 0) || (width == 31))
				scale = 0;

			scaleAlpha = scale;
			scale = max(0.33f, scale);

			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scale;
			*(pBits++) = 255 * scaleAlpha;
		}
	}

	IDirect3DTexture9_UnlockRect(gpBloom, 0);

	// no look through texture
	IDirect3DDevice9_CreateTexture(gpD3DDevice, 1, 1, 1, 0,
                                  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpNoLookThrough, NULL);

	IDirect3DTexture9_LockRect(gpNoLookThrough, 0, &lockedRect, NULL, 0);

	pBits = (unsigned char *)lockedRect.pBits;

	*(pBits++) = 0;
	*(pBits++) = 0;
	*(pBits++) = 0;
	*(pBits++) = 0;

	IDirect3DTexture9_UnlockRect(gpNoLookThrough, 0);
}

void D3DRenderLMapPostFloorAdd(BSPnode *pNode, d3d_render_pool_new *pPool, d_light *light,
   Bool bDynamic)
{
   Sector		*pSector = pNode->u.leaf.sector;

   PDIB		pDib;

   if (pSector->floor)
   {
      pDib = pSector->floor;
   }
   else
      return;

   int count;
   float falloff;

   custom_st st[MAX_NPTS];
   custom_bgra bgra[MAX_NPTS];

   d3d_render_packet_new	*pPacket;
   d3d_render_chunk_new	*pChunk;

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
      bgra[count] = pNode->floor_bgra[count];

      falloff = (pNode->floor_xyz[count].z - light->xyz.z) * light->invXYZScaleHalf.z;

      if (falloff < 0)
         falloff = -falloff;

      falloff = min(1.0f, falloff);
      falloff = 1.0f - falloff;
      //				falloff = 255.0f * falloff;

      bgra[count].b = falloff * light->color.b;
      bgra[count].g = falloff * light->color.g;
      bgra[count].r = falloff * light->color.r;
      bgra[count].a = falloff * light->color.a;

      st[count].s = pNode->floor_xyz[count].x - light->xyz.x;
      st[count].t = pNode->floor_xyz[count].y - light->xyz.y;

      st[count].s *= light->invXYZScale.x;
      st[count].t *= light->invXYZScale.y;

      st[count].s += 0.5f;
      st[count].t += 0.5f;

      pChunk->xyz[count].x = pNode->floor_xyz[count].x;
      pChunk->xyz[count].y = pNode->floor_xyz[count].y;
      pChunk->xyz[count].z = pNode->floor_xyz[count].z;

      pChunk->bgra[count].b = bgra[count].b;
      pChunk->bgra[count].g = bgra[count].g;
      pChunk->bgra[count].r = bgra[count].r;
      pChunk->bgra[count].a = bgra[count].a;

      pChunk->st0[count].s = st[count].s;
      pChunk->st0[count].t = st[count].t;

      pChunk->st1[count].s = pNode->floor_stBase[count].s;
      pChunk->st1[count].t = pNode->floor_stBase[count].t;
   }

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
   gNumObjects++;
}

void D3DRenderLMapPostCeilingAdd(BSPnode *pNode, d3d_render_pool_new *pPool, d_light *light,
   Bool bDynamic)
{
   Sector		*pSector = pNode->u.leaf.sector;
   PDIB		pDib;

   if (pSector->ceiling)
   {
      pDib = pSector->ceiling;
   }
   else
      return;

   custom_bgra bgra[MAX_NPTS];
   custom_st	st[MAX_NPTS];
   float falloff;
   int count;

   d3d_render_packet_new	*pPacket;
   d3d_render_chunk_new	*pChunk;

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
      bgra[count] = pNode->ceiling_bgra[count];

      falloff = (pNode->ceiling_xyz[count].z - light->xyz.z) * light->invXYZScaleHalf.z;

      if (falloff < 0)
         falloff = -falloff;

      falloff = min(1.0f, falloff);
      falloff = 1.0f - falloff;
      //				falloff = 255.0f * falloff;

      bgra[count].b = falloff * light->color.b;
      bgra[count].g = falloff * light->color.g;
      bgra[count].r = falloff * light->color.r;
      bgra[count].a = falloff * light->color.a;

      st[count].s = pNode->ceiling_xyz[count].x - light->xyz.x;
      st[count].t = pNode->ceiling_xyz[count].y - light->xyz.y;

      st[count].s *= light->invXYZScale.x;
      st[count].t *= light->invXYZScale.y;

      st[count].s += 0.5f;
      st[count].t += 0.5f;

      pChunk->xyz[count].x = pNode->ceiling_xyz[count].x;
      pChunk->xyz[count].y = pNode->ceiling_xyz[count].y;
      pChunk->xyz[count].z = pNode->ceiling_xyz[count].z;

      pChunk->bgra[count].b = bgra[count].b;
      pChunk->bgra[count].g = bgra[count].g;
      pChunk->bgra[count].r = bgra[count].r;
      pChunk->bgra[count].a = bgra[count].a;

      pChunk->st0[count].s = st[count].s;
      pChunk->st0[count].t = st[count].t;

      pChunk->st1[count].s = pNode->ceiling_stBase[count].s;
      pChunk->st1[count].t = pNode->ceiling_stBase[count].t;

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

void D3DRenderLMapPostWallAdd(WallData *pWall, d3d_render_pool_new *pPool, unsigned int type, int side,
   d_light *light, Bool bDynamic)
{
   custom_xyz		xyz[4];
   custom_st		st[4];
   custom_st		stBase[4];
   custom_bgra		bgra[4];
   unsigned int	flags;
   PDIB			pDib = NULL;
   Sidedef			*pSideDef = NULL;

   d3d_render_packet_new	*pPacket;
   d3d_render_chunk_new	*pChunk;

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
         {
            pDib = pWall->pos_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_normal_xyz[i];
               bgra[i] = pWall->pos_normal_bgra[i];
               stBase[i] = pWall->pos_normal_stBase[i];
               flags = pWall->pos_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->pos_sidedef->below_bmap)
         {
            pDib = pWall->pos_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_below_xyz[i];
               bgra[i] = pWall->pos_below_bgra[i];
               stBase[i] = pWall->pos_below_stBase[i];
               flags = pWall->pos_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->pos_sidedef->above_bmap)
         {
            pDib = pWall->pos_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->pos_above_xyz[i];
               bgra[i] = pWall->pos_above_bgra[i];
               stBase[i] = pWall->pos_above_stBase[i];
               flags = pWall->pos_above_d3dFlags;
            }
         }
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
         {
            pDib = pWall->neg_sidedef->normal_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_normal_xyz[i];
               bgra[i] = pWall->neg_normal_bgra[i];
               stBase[i] = pWall->neg_normal_stBase[i];
               flags = pWall->neg_normal_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_BELOW:
         if (pWall->neg_sidedef->below_bmap)
         {
            pDib = pWall->neg_sidedef->below_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_below_xyz[i];
               bgra[i] = pWall->neg_below_bgra[i];
               stBase[i] = pWall->neg_below_stBase[i];
               flags = pWall->neg_below_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      case D3DRENDER_WALL_ABOVE:
         if (pWall->neg_sidedef->above_bmap)
         {
            pDib = pWall->neg_sidedef->above_bmap;
            for (int i = 0; i < 4; i++)
            {
               xyz[i] = pWall->neg_above_xyz[i];
               bgra[i] = pWall->neg_above_bgra[i];
               stBase[i] = pWall->neg_above_stBase[i];
               flags = pWall->neg_above_d3dFlags;
            }
         }
         else
            pDib = NULL;
         break;

      default:
         break;
      }
   }

   if (NULL == pDib)
      return;
     
   unsigned int	i;
   float falloff;
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

   // check to see if dot product is necessary
   //			if ((light->xyz.z < xyz[0].z) &&
   //				(light->xyz.z > xyz[1].z))

   // Disabled, causes some polys not to be lit when they should be.
   /*if (0)
   {
   custom_xyz	lightVec;
   float		cosAngle;

   lightVec.x = light->xyz.x - xyz[0].x;
   lightVec.y = light->xyz.y - xyz[0].y;
   lightVec.z = light->xyz.z - xyz[0].z;

   cosAngle = lightVec.x * normal.x +
   lightVec.y * normal.y +
   lightVec.z * normal.z;

   if (cosAngle <= 0)
   continue;
   }*/

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
         falloff = (xyz[i].x - light->xyz.x) * light->invXYZScaleHalf.x;

         if (falloff < 0)
            falloff = -falloff;

         falloff = min(1.0f, falloff);
         falloff = 1.0f - falloff;

         st[i].s = (xyz[i].y - light->xyz.y) * light->invXYZScale.y + 0.5f;
         st[i].t = (xyz[i].z - light->xyz.z) * light->invXYZScale.z + 0.5f;
         bgra[i].b = falloff * light->color.b;
         bgra[i].g = falloff * light->color.g;
         bgra[i].r = falloff * light->color.r;
         bgra[i].a = falloff * light->color.a;
      }
   }
   else
   {
      for (i = 0; i < 4; i++)
      {
         falloff = (xyz[i].y - light->xyz.y) * light->invXYZScaleHalf.y;

         if (falloff < 0)
            falloff = -falloff;

         falloff = min(1.0f, falloff);
         falloff = 1.0f - falloff;

         st[i].s = (xyz[i].x - light->xyz.x) * light->invXYZScale.x + 0.5f;
         st[i].t = (xyz[i].z - light->xyz.z) * light->invXYZScale.z + 0.5f;
         bgra[i].b = falloff * light->color.b;
         bgra[i].g = falloff * light->color.g;
         bgra[i].r = falloff * light->color.r;
         bgra[i].a = falloff * light->color.a;
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

   //			for (i = 0; i < 4; i++)
   //				CACHE_ST_ADD(pRenderCache, 1, stBase[i].s, stBase[i].t);
}

void D3DRenderFontInit(font_3d *pFont, HFONT hFont)
{
	D3DCAPS9		d3dCaps;
	HDC				hDC;
	HBITMAP			hbmBitmap;
	DWORD			*pBitmapBits;
	BITMAPINFO		bmi;
	long			x = 0;
	long			y = 0;
	TCHAR			str[2] = _T("x");
	TCHAR			c;
	SIZE			size;
	D3DLOCKED_RECT	d3dlr;
	BYTE			*pDstRow;
	WORD			*pDst16;
	BYTE			bAlpha;
	HRESULT			hr;

	pFont->fontHeight = GetFontHeight(hFont);
//	pFont->flags = flags;
	pFont->flags = 0;
	pFont->texScale = 1.0f;
   
	if (pFont->fontHeight > 40)
		pFont->texWidth = pFont->texHeight = 1024;
	else if (pFont->fontHeight > 20)
		pFont->texWidth = pFont->texHeight = 512;
	else
		pFont->texWidth = pFont->texHeight = 256;
   
	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &d3dCaps);
  
	if (pFont->texWidth > (long) d3dCaps.MaxTextureWidth)
	{
		pFont->texScale = (float)pFont->texWidth / (float)d3dCaps.MaxTextureWidth;
		pFont->texHeight = pFont->texWidth = d3dCaps.MaxTextureWidth;
	}
  
	if (pFont->pTexture)
		IDirect3DDevice9_Release(pFont->pTexture);
   
	hr = IDirect3DDevice9_CreateTexture(
		gpD3DDevice, pFont->texWidth,
		pFont->texHeight, 1, 0, D3DFMT_A4R4G4B4,
		D3DPOOL_MANAGED, &pFont->pTexture, NULL);

	memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = (int)pFont->texWidth;
	bmi.bmiHeader.biHeight = -(int)pFont->texHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;
   
	hDC = CreateCompatibleDC(gBitsDC);
	hbmBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (VOID**)&pBitmapBits, NULL, 0 );
	SetMapMode(hDC, MM_TEXT);
  
/*	nHeight = -MulDiv(pFont->fontHeight, (int)(GetDeviceCaps(hDC, LOGPIXELSY)
 * pFont->texScale), 72);
 dwBold = (pFont->flags & D3DFONT_BOLD) ? FW_BOLD : FW_NORMAL;
 dwItalic = (pFont->flags & D3DFONT_ITALIC) ? TRUE : FALSE;
 hFont = CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic,
 FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
 VARIABLE_PITCH, pFont->strFontName);*/
  
	assert(hFont);
   
	SelectObject(hDC, hbmBitmap);
	SelectObject(hDC, hFont);
   
	// Set text properties
	SetTextColor(hDC, RGB(255,255,255));
	SetBkColor(hDC, 0);
	SetTextAlign(hDC, TA_TOP);
   
	for(c = 32; c < 127; c++ )
	{
		BOOL	temp;
		int left_offset, right_offset;
      
		str[0] = c;
		GetTextExtentPoint32(hDC, str, 1, &size);
      
		if (!GetCharABCWidths(hDC, c, c, &pFont->abc[c-32])) 
		{
			pFont->abc[c-32].abcA = 0;
			pFont->abc[c-32].abcB = size.cx;
			pFont->abc[c-32].abcC = 0;
		}
      
		left_offset = abs(pFont->abc[c-32].abcA);
		right_offset = abs(pFont->abc[c-32].abcC);
		size.cx = abs(pFont->abc[c-32].abcA) + pFont->abc[c-32].abcB + abs(pFont->abc[c-32].abcC);
      
		if (x + size.cx + 1 > pFont->texWidth)
		{
			x  = 0;
			y += size.cy + 1;
		}
      
		temp = ExtTextOut(hDC, x + left_offset, y+0, ETO_OPAQUE, NULL, str, 1, NULL);
      
		pFont->texST[c-32][0].s = ((FLOAT)(x+0)) / pFont->texWidth;
		pFont->texST[c-32][0].t = ((FLOAT)(y+0)) / pFont->texHeight;
		pFont->texST[c-32][1].s = ((FLOAT)(x+0 + size.cx)) / pFont->texWidth;
		pFont->texST[c-32][1].t = ((FLOAT)(y+0 + size.cy)) / pFont->texHeight;
      
		x += size.cx+1;
	}
   
	hr = IDirect3DTexture9_LockRect(pFont->pTexture, 0, &d3dlr, 0, 0);

	if (SUCCEEDED(hr))
	{
		pDstRow = (BYTE*)d3dlr.pBits;
   
		for(y = 0; y < pFont->texHeight; y++)
		{
			pDst16 = (WORD *)pDstRow;
			for(x = 0; x < pFont->texWidth; x++)
			{
				bAlpha = (BYTE)((pBitmapBits[pFont->texWidth * y + x] & 0xff) >> 4);
				if (bAlpha > 0)
				{
					*pDst16++ = (bAlpha << 12) | 0x0fff;
				}
				else
				{
					*pDst16++ = 0x0000;
				}
			}
			pDstRow += d3dlr.Pitch;
		}
   
		IDirect3DTexture9_UnlockRect(pFont->pTexture, 0);
	}
	
	DeleteObject(hbmBitmap);
	DeleteDC(hDC);
}

void D3DRenderSkyboxDraw(d3d_render_pool_new *pPool, int angleHeading, int anglePitch)
{
	int			i, j;
	D3DMATRIX	rot, mat;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);

	MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
	MatrixRotateX(&mat, (float)anglePitch * 45.0f / 414.0f * PI / 180.0f);
	MatrixMultiply(&mat, &rot, &mat);

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);

//	XformMatrixPerspective(&mat, -PI / 4.0f, PI / 6.0f, 100.0f, 150000.0f);
//	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
                D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
				D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	for (i = 0; i < 6; i++)
	{
		pPacket = D3DRenderPacketFindMatch(pPool, gpSkyboxTextures[gCurBackground][i], NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialWorldPacket;
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
		pChunk->flags |= D3DRENDER_NOAMBIENT;

		// add xyz, st, and bgra data
		for (j = 0; j < 4; j++)
		{
			pChunk->xyz[j].x = gSkyboxXYZ[(i * 4 * 3) + (j * 3)];
			pChunk->xyz[j].z = gSkyboxXYZ[(i * 4 * 3) + (j * 3) + 1];
			pChunk->xyz[j].y = gSkyboxXYZ[(i * 4 * 3) + (j * 3) + 2];

			pChunk->st0[j].s = gSkyboxST[(i * 4 * 2) + (j * 2)];
			pChunk->st0[j].t = gSkyboxST[(i * 4 * 2) + (j * 2) + 1];

			pChunk->bgra[j].b = gSkyboxBGRA[(i * 4 * 4) + (j * 4)];// / 2.0f;
			pChunk->bgra[j].g = gSkyboxBGRA[(i * 4 * 4) + (j * 4) + 1];// / 2.0f;
			pChunk->bgra[j].r = gSkyboxBGRA[(i * 4 * 4) + (j * 4) + 2];// / 2.0f;
			pChunk->bgra[j].a = gSkyboxBGRA[(i * 4 * 4) + (j * 4) + 3];

		}
		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
				D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0,
				D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

/*
 * D3DExtractWallFromTree: Fills pointers with previously extracted data
 *                         contained in the WallData struct.
 */
void D3DExtractWallFromTree(WallData *pWall, PDIB pDib, unsigned int *flags, custom_xyz *pXYZ,
   custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side)
{
   if (side > 0)
   {
      switch (type)
      {
      case D3DRENDER_WALL_NORMAL:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->pos_normal_xyz[i];
            pBGRA[i] = pWall->pos_normal_bgra[i];
            pST[i] = pWall->pos_normal_stBase[i];
            *flags = pWall->pos_normal_d3dFlags;
         }
         return;
      case D3DRENDER_WALL_BELOW:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->pos_below_xyz[i];
            pBGRA[i] = pWall->pos_below_bgra[i];
            pST[i] = pWall->pos_below_stBase[i];
            *flags = pWall->pos_below_d3dFlags;
         }
         return;
      case D3DRENDER_WALL_ABOVE:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->pos_above_xyz[i];
            pBGRA[i] = pWall->pos_above_bgra[i];
            pST[i] = pWall->pos_above_stBase[i];
            *flags = pWall->pos_above_d3dFlags;
         }
         return;
      }
   }
   else if (side < 0)
   {
      switch (type)
      {
      case D3DRENDER_WALL_NORMAL:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->neg_normal_xyz[i];
            pBGRA[i] = pWall->neg_normal_bgra[i];
            pST[i] = pWall->neg_normal_stBase[i];
            *flags = pWall->neg_normal_d3dFlags;
         }
         return;
      case D3DRENDER_WALL_BELOW:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->neg_below_xyz[i];
            pBGRA[i] = pWall->neg_below_bgra[i];
            pST[i] = pWall->neg_below_stBase[i];
            *flags = pWall->neg_below_d3dFlags;
         }
         return;
      case D3DRENDER_WALL_ABOVE:
         for (int i = 0; i < 4; i++)
         {
            pXYZ[i] = pWall->neg_above_xyz[i];
            pBGRA[i] = pWall->neg_above_bgra[i];
            pST[i] = pWall->neg_above_stBase[i];
            *flags = pWall->neg_above_d3dFlags;
         }
         return;
      }
   }
   else
      assert(0);
}

/*
* D3DExtractFloorFromTree: Fills pointers with previously extracted data
*                          contained in the BSPnode struct.
*/
void D3DExtractFloorFromTree(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
   custom_bgra *pBGRA)
{
   for (int i = 0; i < MAX_NPTS; i++)
   {
      pXYZ[i] = pNode->floor_xyz[i];
      pBGRA[i] = pNode->floor_bgra[i];
      pST[i] = pNode->floor_stBase[i];
   }
}

/*
* D3DExtractCeilingFromTree: Fills pointers with previously extracted data
*                          contained in the BSPnode struct.
*/
void D3DExtractCeilingFromTree(BSPnode *pNode, PDIB pDib, custom_xyz *pXYZ, custom_st *pST,
   custom_bgra *pBGRA)
{
   for (int i = 0; i < MAX_NPTS; i++)
   {
      pXYZ[i] = pNode->ceiling_xyz[i];
      pBGRA[i] = pNode->ceiling_bgra[i];
      pST[i] = pNode->ceiling_stBase[i];
   }
}

int D3DRenderWallExtract(WallData *pWall, PDIB pDib, unsigned int *flags, custom_xyz *pXYZ,
						  custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side)
{
	int				top, bottom;
	int				xOffset, yOffset;
	Sidedef			*pSideDef = NULL;
	int				drawTopDown;
	int				paletteIndex;
	BYTE			light;

   if (pDib == NULL)
      return 0;
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
         //debug(("y0 is %6.4f, pXYZ[0].y is %6.4f\n", pWall->y0, pXYZ[0].y));
			switch (type)
			{
				case D3DRENDER_WALL_NORMAL:
				{
					pXYZ[0].z = (float)pWall->z2;
					pXYZ[3].z = (float)pWall->zz2;
					pXYZ[1].z = (float)pWall->z1;
					pXYZ[2].z = (float)pWall->zz1;

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
						pXYZ[0].z = (float)pWall->z1Neg;
						pXYZ[3].z = (float)pWall->zz1Neg;
					}
					else
					{
						pXYZ[0].z = (float)pWall->z1;
						pXYZ[3].z = (float)pWall->zz1;
					}

					pXYZ[1].z = (float)pWall->z0;
					pXYZ[2].z = (float)pWall->zz0;

					if (pSideDef->flags & WF_BELOW_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
               pXYZ[0].z = (float)pWall->z3;
               pXYZ[3].z = (float)pWall->zz3;
               pXYZ[1].z = (float)pWall->z2;
               pXYZ[2].z = (float)pWall->zz2;

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

			pXYZ[0].x = pWall->x1;
			pXYZ[3].x = pWall->x0;
			pXYZ[1].x = pWall->x1;
			pXYZ[2].x = pWall->x0;

			pXYZ[0].y = pWall->y1;
			pXYZ[3].y = pWall->y0;
			pXYZ[1].y = pWall->y1;
			pXYZ[2].y = pWall->y0;

			switch (type)
			{
				case D3DRENDER_WALL_NORMAL:
				{
               pXYZ[0].z = (float)pWall->zz2;
               pXYZ[3].z = (float)pWall->z2;
               pXYZ[1].z = (float)pWall->zz1;
               pXYZ[2].z = (float)pWall->z1;

					if (pSideDef->flags & WF_NORMAL_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_BELOW:
				{
               pXYZ[0].z = (float)pWall->zz1;
               pXYZ[3].z = (float)pWall->z1;
               pXYZ[1].z = (float)pWall->zz0;
               pXYZ[2].z = (float)pWall->z0;

					if (pSideDef->flags & WF_BELOW_TOPDOWN)
						drawTopDown = 1;
					else
						drawTopDown = 0;
				}
				break;

				case D3DRENDER_WALL_ABOVE:
				{
               pXYZ[0].z = (float)pWall->zz3;
               pXYZ[3].z = (float)pWall->z3;
               pXYZ[1].z = (float)pWall->zz2;
               pXYZ[2].z = (float)pWall->z2;

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
		break;

/*		case D3DRENDER_WALL_BELOW:
			if (pSideDef->flags & WF_NO_VTILE)
				if (NULL == pSideDef->normal_bmap)
					*flags |= D3DRENDER_NO_VTILE;
		break;

		case D3DRENDER_WALL_ABOVE:
			if (pSideDef->flags & WF_NO_VTILE)
				*flags |= D3DRENDER_NO_VTILE;
		break;*/

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
				pST[1].t -= (pXYZ[1].z - bottom) * (float)pDib->shrink
						* invWidthFudge;
				pST[2].t -= (pXYZ[2].z - bottom) * (float)pDib->shrink
						* invWidthFudge;
			}

			pST[0].t = pST[1].t -
				((pXYZ[0].z - pXYZ[1].z) * (float)pDib->shrink * invWidthFudge);
			pST[3].t = pST[2].t -
				((pXYZ[3].z - pXYZ[2].z) * (float)pDib->shrink * invWidthFudge);
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

/*		if (pSideDef->flags & WF_NO_VTILE)
		{
			if (pST[1].t > 0.99f)
				pST[1].t = 0.99f;

			if (pST[2].t > 0.99f)
				pST[2].t = 0.99f;
		}*/

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
		int	i;
		float a, b;
		int	distX, distY, distance;
		long	lightScale;
		long lo_end = FINENESS-shade_amount;

		for (i = 0; i < 4; i++)
		{
			distX = pXYZ[i].x - player.x;
			distY = pXYZ[i].y - player.y;

			distance = DistanceGet(distX, distY);

			if (shade_amount != 0)
			{
				a = pWall->separator.a;
				b = pWall->separator.b;

				if (side < 0)
				{
					a = -a;
					b = -b;
				}

				lightScale = (long)(a * sun_vect.x +
								b * sun_vect.y) >> LOG_FINENESS;

#if PERPENDICULAR_DARK
				lightScale = ABS(lightScale);
#else
				lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range
#endif

				lightScale = lo_end + ((lightScale * shade_amount)>>LOG_FINENESS);
				
				// Dx9.0 applies too much shading, so set a lower limit of 860 here for now.
				if (lightScale > FINENESS)
					lightScale = FINENESS;
				else if ( lightScale < 0)
					lightScale = 0;
			}
			else
				lightScale = FINENESS;

			pWall->lightscale = lightScale;

			if (gD3DDriverProfile.bFogEnable)
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

   //inv128 = 1.0f / (128.0f * PETER_FUDGE);// / pDib->shrink);
   inv128 = 0.00048828125f;
   //inv64 = 1.0f / (64.0f * PETER_FUDGE);// / pDib->shrink);
   inv64 = 0.0009765625f;

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
				pXYZ[count].x = pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (-pSector->sloped_floor->plane.a * pXYZ[count].x -
					pSector->sloped_floor->plane.b * pXYZ[count].y -
					pSector->sloped_floor->plane.d) * oneOverC;
			}
			else
			{
				pXYZ[count].x = pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = pNode->u.leaf.poly.p[count].y;
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

					pST[count].s = sqrt((pXYZ[count].x - intersectTop.x) *
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

					pST[count].t = sqrt((pXYZ[count].x - intersectLeft.x) *
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

					distance = sqrt((vectorU.x * vectorU.x) +
						(vectorU.y * vectorU.y));

					if (distance == 0)
						distance = 1.0f;

					vectorU.x /= distance;
					vectorU.z /= distance;
					vectorU.y /= distance;

					vectorV.x = pSector->sloped_floor->p2.x - pSector->sloped_floor->p0.x;
					vectorV.z = pSector->sloped_floor->p2.z - pSector->sloped_floor->p0.z;
					vectorV.y = pSector->sloped_floor->p2.y - pSector->sloped_floor->p0.y;

					distance = sqrt((vectorV.x * vectorV.x) +
						(vectorV.y * vectorV.y));

					if (distance == 0)
						distance = 1.0f;

					vectorV.x /= distance;
					vectorV.z /= distance;
					vectorV.y /= distance;

					vector.x = pXYZ[count].x - pSector->sloped_floor->p0.x;
					vector.y = pXYZ[count].y - pSector->sloped_floor->p0.y;

					distance = sqrt((vector.x * vector.x) +
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
					pST[count].s = fabs(pNode->u.leaf.poly.p[count].y - top) - pSector->ty;// / pDib->shrink;
					pST[count].t = fabs(pNode->u.leaf.poly.p[count].x - left) - pSector->tx;// / pDib->shrink;
				}

				if (pSector->animate != NULL && pSector->animate->animation == ANIMATE_SCROLL)
				{
					if (pSector->flags & SF_SCROLL_FLOOR)
					{
						pST[count].s -= pSector->animate->u.scroll.yoffset;// * pDib->shrink;
						pST[count].t += pSector->animate->u.scroll.xoffset;// * pDib->shrink;
					}
				}

				//if (pDib->width == BITMAP_WIDTH * 2)
				if(0)
				{
					pST[count].s *= inv128;
					pST[count].t *= inv128;
				}
				else
				{
					pST[count].s *= inv64;
					pST[count].t *= inv64;
				}
			}
		}
	}

	if (pBGRA)
	{
		int	distX, distY, distance;

		for (count = 0; count < pNode->u.leaf.poly.npts; count++)
		{
			distX = pXYZ[count].x - player.x;
			distY = pXYZ[count].y - player.y;

			lightscale = FINENESS;

			if (shade_amount != 0)
			{
				// floor is sloped, and it's marked as being steep enough to be eligible for
				// directional lighting
				if ((pNode->u.leaf.sector->sloped_floor != NULL) &&
					(pNode->u.leaf.sector->sloped_floor->flags & SLF_DIRECTIONAL))
				{
					long lo_end = FINENESS-shade_amount;

					// light scale is based on dot product of surface normal and sun vector
					lightscale = (long)(pNode->u.leaf.sector->sloped_floor->plane.a * sun_vect.x +
						pNode->u.leaf.sector->sloped_floor->plane.b * sun_vect.y +
						pNode->u.leaf.sector->sloped_floor->plane.c * sun_vect.z)>>LOG_FINENESS;

#if PERPENDICULAR_DARK
					lightscale = ABS(lightscale);
#else
					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range
#endif

					lightscale = lo_end + ((lightscale * shade_amount)>>LOG_FINENESS);

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

			if (gD3DDriverProfile.bFogEnable)
				paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightscale, 0);
			else
				paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightscale, 0);

			pBGRA[count].r = pBGRA[count].g = pBGRA[count].b = paletteIndex * COLOR_AMBIENT / 64;
			pBGRA[count].a = 255;
		}
	}
}

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

   //inv128 = 1.0f / (128.0f * PETER_FUDGE);// / pDib->shrink);
   inv128 = 0.00048828125f;
   //inv64 = 1.0f / (64.0f * PETER_FUDGE);// / pDib->shrink);
   inv64 = 0.0009765625f;

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
				pXYZ[count].x = pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = pNode->u.leaf.poly.p[count].y;
				pXYZ[count].z = (-pSector->sloped_ceiling->plane.a * pXYZ[count].x -
					pSector->sloped_ceiling->plane.b * pXYZ[count].y -
					pSector->sloped_ceiling->plane.d) * oneOverC;
			}
			else
			{
				pXYZ[count].x = pNode->u.leaf.poly.p[count].x;
				pXYZ[count].y = pNode->u.leaf.poly.p[count].y;
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

					pST[count].s = sqrt((pXYZ[count].x - intersectTop.x) *
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

					pST[count].t = sqrt((pXYZ[count].x - intersectLeft.x) *
									(pXYZ[count].x - intersectLeft.x) +
									(pXYZ[count].z - intersectLeft.z) *
									(pXYZ[count].z - intersectLeft.z) +
									(pXYZ[count].y - intersectLeft.y) *
									(pXYZ[count].y - intersectLeft.y));

//					pST[count].s = -pST[count].s;
//					pST[count].t = -pST[count].t;

//					pST[count].s -= pSector->ty / 2.0f;
//					pST[count].t -= pSector->tx / 2.0f;

					vectorU.x = pSector->sloped_ceiling->p1.x - pSector->sloped_ceiling->p0.x;
					vectorU.z = pSector->sloped_ceiling->p1.z - pSector->sloped_ceiling->p0.z;
					vectorU.y = pSector->sloped_ceiling->p1.y - pSector->sloped_ceiling->p0.y;

					distance = sqrt((vectorU.x * vectorU.x) +
						(vectorU.y * vectorU.y));

					if (distance == 0)
						distance = 1.0f;

					vectorU.x /= distance;
					vectorU.z /= distance;
					vectorU.y /= distance;

					vectorV.x = pSector->sloped_ceiling->p2.x - pSector->sloped_ceiling->p0.x;
					vectorV.z = pSector->sloped_ceiling->p2.z - pSector->sloped_ceiling->p0.z;
					vectorV.y = pSector->sloped_ceiling->p2.y - pSector->sloped_ceiling->p0.y;

					distance = sqrt((vectorV.x * vectorV.x) +
						(vectorV.y * vectorV.y));

					if (distance == 0)
						distance = 1.0f;

					vectorV.x /= distance;
					vectorV.z /= distance;
					vectorV.y /= distance;

					vector.x = pXYZ[count].x - pSector->sloped_ceiling->p0.x;
					vector.y = pXYZ[count].y - pSector->sloped_ceiling->p0.y;

					distance = sqrt((vector.x * vector.x) +
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
					pST[count].s = fabs(pNode->u.leaf.poly.p[count].y - top) - pSector->ty;
					pST[count].t = fabs(pNode->u.leaf.poly.p[count].x - left) - pSector->tx;
				}

				if (pSector->animate != NULL && pSector->animate->animation == ANIMATE_SCROLL)
				{
					if (pSector->flags & SF_SCROLL_CEILING)
					{
						pST[count].s -= pSector->animate->u.scroll.yoffset;// * pDib->shrink;
						pST[count].t += pSector->animate->u.scroll.xoffset;// * pDib->shrink;
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
			distX = pXYZ[count].x - player.x;
			distY = pXYZ[count].y - player.y;

			lightscale = FINENESS;

			if (shade_amount != 0)
			{
				if ((pNode->u.leaf.sector->sloped_ceiling != NULL) &&
					(pNode->u.leaf.sector->sloped_ceiling->flags & SLF_DIRECTIONAL))
				{
					long lo_end = FINENESS-shade_amount;

					// light scale is based on dot product of surface normal and sun vector
					lightscale = (long)(pNode->u.leaf.sector->sloped_ceiling->plane.a * sun_vect.x +
						pNode->u.leaf.sector->sloped_ceiling->plane.b * sun_vect.y +
						pNode->u.leaf.sector->sloped_ceiling->plane.a * sun_vect.z)>>LOG_FINENESS;

#if PERPENDICULAR_DARK
					lightscale = ABS(lightscale);
#else
					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range
#endif

					lightscale = lo_end + ((lightscale * shade_amount)>>LOG_FINENESS);

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

			if (gD3DDriverProfile.bFogEnable)
				paletteIndex = GetLightPaletteIndex(FINENESS, pSector->light, lightscale, 0);
			else
				paletteIndex = GetLightPaletteIndex(distance, pSector->light, lightscale, 0);

			pBGRA[count].r = pBGRA[count].g = pBGRA[count].b = paletteIndex * COLOR_AMBIENT / 64;
			pBGRA[count].a = 255;
		}
	}
}

void D3DRenderBackgroundsLoad(char *pFilename, int index)
{
	FILE	*pFile;
	png_structp	pPng = NULL;
	png_infop	pInfo = NULL;
	png_infop	pInfoEnd = NULL;
	png_bytepp   rows;

	D3DLOCKED_RECT		lockedRect;
	unsigned char		*pBits = NULL;
	unsigned int		w, h, b;
	int					pitchHalf, bytePP;
	fpos_t				pos;

	pFile = fopen(pFilename, "rb");
	if (pFile == NULL)
		return;

	pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == pPng)
	{
		fclose(pFile);
		return;
	}

	pInfo = png_create_info_struct(pPng);
	if (NULL == pInfo)
	{
		png_destroy_read_struct(&pPng, NULL, NULL);
		fclose(pFile);
		return;
	}

	pInfoEnd = png_create_info_struct(pPng);
	if (NULL == pInfoEnd)
	{
		png_destroy_read_struct(&pPng, &pInfo, NULL);
		fclose(pFile);
		return;
	}

	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
		fclose(pFile);
		return;
	}

	png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);

	pos = 0;

	{
		int	i;
		png_bytep	curRow;

		for (i = 0; i < 6; i++)
		{
			pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			pInfo = png_create_info_struct(pPng);
			pInfoEnd = png_create_info_struct(pPng);
			setjmp(png_jmpbuf(pPng));

			fseek(pFile, pos, SEEK_SET);

			png_init_io(pPng, pFile);
			png_read_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
			rows = png_get_rows(pPng, pInfo);

			bytePP = pPng->pixel_depth / 8;

			IDirect3DDevice9_CreateTexture(gpD3DDevice, pPng->width, pPng->height, 1, 0,
                                        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpSkyboxTextures[index][i], NULL);

			IDirect3DTexture9_LockRect(gpSkyboxTextures[index][i], 0, &lockedRect, NULL, 0);

			pitchHalf = lockedRect.Pitch / 2;

			pBits = (unsigned char *)lockedRect.pBits;

			for (h = 0; h < pPng->height; h++)
			{
				curRow = rows[h];

				for (w = 0; w < pPng->width; w++)
				{
					for (b = 0; b < 4; b++)
					{
						if (b == 3)
							pBits[h * lockedRect.Pitch + w * 4 + b] = 255;
						else
              // Apparently PNGs are BGR, while DirectX wants RGB
							pBits[h * lockedRect.Pitch + w * 4 + (2 - b)] =
								curRow[(w * 3) + b];
					}
				}
			}

			IDirect3DTexture9_UnlockRect(gpSkyboxTextures[index][i], 0);

			fgetpos(pFile, &pos);
			png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
		}
	}

//	png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
	fclose(pFile);
}

void D3DRenderBackgroundSet2(ID background)
{
	char	*filename = NULL;
	filename = LookupRsc(background);

	if (filename)
	{
		if (strstr(filename, "skya.bgf"))
			gCurBackground = 0;
		else if (strstr(filename, "skyb.bgf"))
			gCurBackground = 1;
		else if (strstr(filename, "skyc.bgf"))
			gCurBackground = 2;
		else if (strstr(filename, "skyd.bgf"))
			gCurBackground = 3;
		else if (strstr(filename, "redsky.bgf"))
			gCurBackground = 4;
	}

	// force a rebuild since static lightmaps might have changed
	gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;
}

// new render stuff
void D3DRenderPoolInit(d3d_render_pool_new *pPool, int size, int packetSize)
{
	d3d_render_packet_new	*pPacket = NULL;
	u_int	i;

	pPool->size = size;
	pPool->curPacket = 0;
	pPacket = (d3d_render_packet_new *)D3DRenderMalloc(sizeof(d3d_render_packet_new) * size);
	assert(pPacket);
	pPool->renderPacketList = list_create(pPacket);
	pPool->packetSize = packetSize;

	D3DRenderPoolReset(pPool, NULL);

	for (i = 0; i < pPool->size; i++)
	{
		pPacket->size = packetSize;
//		pPool->renderPackets[i].size = 128;
	}
}

void D3DRenderPoolShutdown(d3d_render_pool_new *pPool)
{
	list_destroy(pPool->renderPacketList);
	memset(pPool, 0, sizeof(d3d_render_pool_new));
}

void D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc)
{
	pPool->curPacket = 0;
	pPool->numLists = 0;
	pPool->curPacketList = pPool->renderPacketList;
	pPool->pMaterialFctn = (MaterialFctn) pMaterialFunc;
}

d3d_render_packet_new *D3DRenderPacketNew(d3d_render_pool_new *pPool)
{
	d3d_render_packet_new	*pPacket;

	if (pPool->curPacket >= pPool->size)
	{
		if (pPool->curPacketList->next == NULL)
		{
			pPacket = (d3d_render_packet_new *)D3DRenderMalloc(sizeof(d3d_render_packet_new) * pPool->size);
			assert(pPacket);
			list_add_item(pPool->renderPacketList, pPacket);
		}
		else
			pPacket = (d3d_render_packet_new *)pPool->curPacketList->next->data;

		pPool->curPacketList = pPool->curPacketList->next;
		pPool->curPacket = 1;
		pPool->numLists++;

		pPacket = (d3d_render_packet_new *)pPool->curPacketList->data;
	}
	else
	{
		pPacket = (d3d_render_packet_new *)pPool->curPacketList->data;
		pPacket += pPool->curPacket;

		if (pPool->curPacket == 12)
			gpPacket = pPacket;

		pPool->curPacket++;
	}

	D3DRenderPacketInit(pPacket);
	return pPacket;
}

void D3DRenderPacketInit(d3d_render_packet_new *pPacket)
{
	pPacket->curChunk = 0;
	pPacket->effect = 0;
	pPacket->flags = 0;
	pPacket->pDib = NULL;
	pPacket->pMaterialFctn = NULL;
	pPacket->pTexture = NULL;
	pPacket->xLat0 = 0;
	pPacket->xLat1 = 0;
}

d3d_render_chunk_new *D3DRenderChunkNew(d3d_render_packet_new *pPacket)
{
	if (pPacket->curChunk >= (pPacket->size - 1))
		return NULL;
	else
	{
		pPacket->curChunk++;
		D3DRenderChunkInit(&pPacket->renderChunks[pPacket->curChunk - 1]);
		return &pPacket->renderChunks[pPacket->curChunk - 1];
	}
}

void D3DRenderChunkInit(d3d_render_chunk_new *pChunk)
{
	pChunk->curIndex = 0;
	pChunk->drawn = 0;
	pChunk->flags = 0;
	pChunk->zBias = 0;
	pChunk->isTargeted = FALSE;
	pChunk->numIndices = 0;
	pChunk->xLat0 = 0;
	pChunk->xLat1 = 0;
	pChunk->pSector = NULL;
	pChunk->pSectorNeg = NULL;
	pChunk->pSectorPos = NULL;
	pChunk->pSideDef = NULL;
	pChunk->pMaterialFctn = NULL;
	pChunk->pRenderCache = NULL;
}

d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, BYTE effect)
{
	u_int						count, numPackets;
	d3d_render_packet_new	*pPacket;
	list_type				list;

	for (list = pPool->renderPacketList; list != pPool->curPacketList->next; list = list->next)
	{
		pPacket = (d3d_render_packet_new *)list->data;

		if (list == pPool->curPacketList)
			numPackets = pPool->curPacket;
		else
			numPackets = pPool->size;

		// for each packet
		for (count = 0; count < numPackets; count++, pPacket++)
		{
			// if we find a match that isn't full already, return it
			if ((pPacket->pDib == pDib) && (pPacket->pTexture == pTexture) &&
				(pPacket->xLat0 == xLat0) && (pPacket->xLat1 == xLat1) &&
				(pPacket->effect == effect))
			{
				if (pPacket->curChunk < (pPacket->size - 1))
					return pPacket;
			}
		}
	}

	// otherwise, return a new one (or NULL if no more remain)
	pPacket = D3DRenderPacketNew(pPool);

	if (pPacket)
	{
		pPacket->pDib = pDib;
		pPacket->pTexture = pTexture;
		pPacket->xLat0 = xLat0;
		pPacket->xLat1 = xLat1;
		pPacket->effect = effect;
		pPacket->size = pPool->packetSize;
	}

	return pPacket;
}

void D3DRenderObjectsDraw(d3d_render_pool_new *pPool, room_type *room,
							 Draw3DParams *params, BYTE flags, Bool drawTransparent)
{
	D3DMATRIX			mat, rot, trans;
	int					angleHeading, anglePitch, i, curObject;
	room_contents_node	*pRNode;
	long				dx, dy, angle;
	PDIB				pDib;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	float				lastDistance, depth;
	BYTE				xLat0, xLat1;
	int					sector_flags;
	long				top, bottom;

	d3d_render_packet_new	*pPacket = NULL;
	d3d_render_chunk_new	*pChunk = NULL;

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	// base objects
	//for (list = room->contents; list != NULL; list = list->next)
	for (curObject = 0; curObject < nitems; curObject++)
	{
//		pRNode = (room_contents_node *)list->data;
		if (drawdata[curObject].type != DrawObjectType)
			continue;

		pRNode = drawdata[curObject].u.object.object->draw.obj;

		if (pRNode == NULL)
			continue;

		if (pRNode->obj.id == player.id)
			continue;

		if (flags == DRAWFX_INVISIBLE)
		{
			if (pRNode->obj.drawingtype != DRAWFX_INVISIBLE)
				continue;
		}
		else
		{
			if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
				continue;

         if (drawTransparent != ((pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
            || (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
            || (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)))
            continue;
      }

		dx = pRNode->motion.x - params->viewer_x;
		dy = pRNode->motion.y - params->viewer_y;

		angle = (pRNode->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

		pDib = GetObjectPdib(pRNode->obj.icon_res, angle, pRNode->obj.animate->group);

		if (NULL == pDib)
			continue;

		if (pRNode->obj.drawingtype == DRAWFX_SECONDTRANS)
		{
			xLat0 = 0;
			xLat1 = pRNode->obj.secondtranslation;
		}
		else if (pRNode->obj.drawingtype == DRAWFX_DOUBLETRANS)
		{
			xLat0 = pRNode->obj.translation;
			xLat1 = pRNode->obj.secondtranslation;
		}
		else
		{
			xLat0 = pRNode->obj.translation;
			xLat1 = 0;
		}

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, xLat0, xLat1,
			pRNode->obj.drawingtype);
		if (NULL == pPacket)
			return;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->drawingtype = pRNode->obj.drawingtype;

		if (flags == DRAWFX_INVISIBLE)
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
		}
		else
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
		}

		angle = pRNode->angle - (params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		/* Make sure that object is above the floor. */
		if (!GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
		{
			continue;
		}

		// Set object depth based on "depth" sector flags
		depth = sector_depths[SectorDepth(sector_flags)];

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
		pChunk->zBias = ZBIAS_BASE;

		lastDistance = 0;

		if (pRNode->obj.drawingtype == DRAWFX_BLACK)
		{
			bgra.b = bgra.g = bgra.r = 0;
			bgra.a = 255;
		}
		else
		{
			if (D3DObjectLightingCalc(room, pRNode, &bgra, 0))
				pChunk->flags |= D3DRENDER_NOAMBIENT;
		}

/*		if(pRNode->obj.id != INVALID_ID && pRNode->obj.id == GetUserTargetID())
		{
			bgra.b = 0.0f;
			bgra.g = 0.0f;
			bgra.r = 255.0f;
		}*/

		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
			bgra.a = D3DRENDER_TRANS25;
		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
			bgra.a = D3DRENDER_TRANS75;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)
			bgra.a = D3DRENDER_TRANS50;

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
		if ((pRNode->obj.id != INVALID_ID) && (pRNode->obj.id != player.id))
		{
			D3DMATRIX	localToScreen, rot, mat;
			custom_xyzw	topLeft, topRight, bottomLeft, bottomRight, center;
			ObjectRange *range = FindVisibleObjectById(pRNode->obj.id);
			int			w, h;
			int			tempLeft, tempRight, tempTop, tempBottom;
			int			distX, distY, distance;

			if (pRNode->obj.id == player.id)
				break;

			w = gD3DRect.right - gD3DRect.left;
			h = gD3DRect.bottom - gD3DRect.top;

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
			MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
			MatrixMultiply(&mat, &trans, &rot);
			XformMatrixPerspective(&localToScreen, FOV_H, FOV_V, 1.0f, 2000000.0f);
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

			if (flags == DRAWFX_INVISIBLE)
			{
				pChunk->st1[0].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[0].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[1].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[1].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[2].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[2].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
				pChunk->st1[3].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
				pChunk->st1[3].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;

				pChunk->st1[0].s -= (gFrame & 3) / 256.0f;
				pChunk->st1[0].t -= (gFrame & 3) / 256.0f;
				pChunk->st1[1].s -= (gFrame & 3) / 256.0f;
				pChunk->st1[1].t += (gFrame & 3) / 256.0f;
				pChunk->st1[2].s += (gFrame & 3) / 256.0f;
				pChunk->st1[2].t += (gFrame & 3) / 256.0f;
				pChunk->st1[3].s += (gFrame & 3) / 256.0f;
				pChunk->st1[3].t -= (gFrame & 3) / 256.0f;
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
            tempLeft = (topLeft.x * w / 2) + (w >> 1);
            tempRight = (bottomRight.x * w / 2) + (w >> 1);
            tempTop = (topLeft.y * -h / 2) + (h >> 1);
            tempBottom = (bottomRight.y * -h / 2) + (h >> 1);

            if (config.large_area)
            {
               tempLeft >>= 1;
               tempRight >>= 1;
               tempTop >>= 1;
               tempBottom >>= 1;
            }

				distX = pRNode->motion.x - player.x;
				distY = pRNode->motion.y - player.y;

				distance = DistanceGet(distX, distY);

				if (range == NULL)
				{
					// Set up new visible object
					range = &visible_objects[num_visible_objects];
					range->id       = pRNode->obj.id;
					range->distance = distance;
					range->left_col   = tempLeft;
					range->right_col  = tempRight;
					range->top_row    = tempTop;
					range->bottom_row = tempBottom;

					num_visible_objects = ++num_visible_objects > MAXOBJECTS ? MAXOBJECTS : num_visible_objects;
				}
				
				// Record boundaries of drawing area
				range->left_col   = min(range->left_col, tempLeft);
				range->right_col  = max(range->right_col, tempRight);
				range->top_row    = min(range->top_row, tempTop);
				range->bottom_row = max(range->bottom_row, tempBottom);
			}
		}

		if (pRNode->obj.id != INVALID_ID && pRNode->obj.id == GetUserTargetID() &&
			(pRNode->obj.drawingtype != DRAWFX_INVISIBLE))
		{
			pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, xLat0, xLat1,
				pRNode->obj.drawingtype);
			if (NULL == pPacket)
				return;
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);
			pChunk->drawingtype = DRAWFX_TRANSLUCENT50;
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->xLat0 = xLat0;
			pChunk->xLat1 = xLat1;
			pChunk->zBias = ZBIAS_TARGETED;
			pChunk->isTargeted = TRUE;

			MatrixMultiply(&pChunk->xForm, &rot, &mat);

			if (flags == DRAWFX_INVISIBLE)
			{
				pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
			}
			else
			{
				pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
			}

			D3DObjectLightingCalc(room, pRNode, &bgra, 0);

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
						pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
					break;

					case 1:
						pChunk->bgra[i].b = min(bgra.r * 2.0f, 255);
						pChunk->bgra[i].g = 0;
						pChunk->bgra[i].r = 0;
						pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
					break;

					default:
						pChunk->bgra[i].b = 0;
						pChunk->bgra[i].g = min(bgra.r * 2.0f, 255);
						pChunk->bgra[i].r = 0;
						pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
					break;
				}
			}

			pChunk->xyz[0].x -= 96.0f / (float)pDib->shrink;
			pChunk->xyz[0].z += 96.0f / (float)pDib->shrink;
			pChunk->xyz[1].x -= 96.0f / (float)pDib->shrink;
			pChunk->xyz[1].z -= 96.0f / (float)pDib->shrink;
			pChunk->xyz[2].x += 96.0f / (float)pDib->shrink;
			pChunk->xyz[2].z -= 96.0f / (float)pDib->shrink;
			pChunk->xyz[3].x += 96.0f / (float)pDib->shrink;
			pChunk->xyz[3].z += 96.0f / (float)pDib->shrink;

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

void D3DRenderOverlaysDraw(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params,
						   BOOL underlays, BYTE flags, Bool drawTransparent)
{
	D3DMATRIX			mat, rot, trans;
	int					angleHeading, anglePitch, i, curObject;
	room_contents_node	*pRNode;
	long				dx, dy, angle, top, bottom;
	PDIB				pDib, pDibOv, pDibOv2;
	Overlay				*pOverlay;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	float				lastDistance, invShrink, invOvShrink, invOv2Shrink, depthf;
	BYTE				xLat0, xLat1, zBias;
	int					sector_flags;
	Bool				bHotspot;

	d3d_render_packet_new	*pPacket = NULL;
	d3d_render_chunk_new	*pChunk = NULL;

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	for (curObject = 0; curObject < nitems; curObject++)
//	for (list = room->contents; list != NULL; list = list->next)
	{
		list_type	list2;
		int			pass, depth;

		if (drawdata[curObject].type != DrawObjectType)
			continue;

//		pRNode = (room_contents_node *)list->data;
		pRNode = drawdata[curObject].u.object.object->draw.obj;

		if (pRNode == NULL)
			continue;

		if (pRNode->obj.id == player.id)
			continue;

		if (flags == DRAWFX_INVISIBLE)
		{
			if (pRNode->obj.drawingtype != DRAWFX_INVISIBLE)
				continue;
		}
		else
		{
			if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
				continue;

         if (drawTransparent != ((pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
            || (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
            || (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
            || (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)))
            continue;
      }

		if (NULL == *pRNode->obj.overlays)
			continue;

//		continue;

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
				bHotspot = FALSE;

				pOverlay = (Overlay *)list2->data;

				if (NULL == pOverlay)
					continue;

				dx = pRNode->motion.x - params->viewer_x;
				dy = pRNode->motion.y - params->viewer_y;

				angle = (pRNode->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

				pDib = GetObjectPdib(pRNode->obj.icon_res, angle, pRNode->obj.animate->group);
				pDibOv = GetObjectPdib(pOverlay->icon_res, angle, pOverlay->animate.group);

				if ((NULL == pDib) || (NULL == pDibOv))
					continue;

				invShrink = 1.0f / pDib->shrink;
				invOvShrink = 1.0f / pDibOv->shrink;

//				if (pDibOv->shrink > 7)
//					continue;

				if (pOverlay->hotspot)
				{
					POINT	point;
					int		retVal, retVal2;

					retVal = FindHotspotPdib(pDib, pOverlay->hotspot, &point);

					if (retVal != HOTSPOT_NONE)
					{
						if (retVal != depth)
						//if (1)
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

						bHotspot = TRUE;
					}
					else
					{
						// the hotspot wasn't in the base object pdib
						// must be an overlay on an overlay, so find base overlay
						list_type	tempList;

						for (tempList = *pRNode->obj.overlays; tempList != NULL; tempList = tempList->next)
						{
							POINT	point2;
							Overlay	*pTempOverlay = (Overlay *)tempList->data;

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
//									if (1)
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

									bHotspot = TRUE;
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
					if (pRNode->obj.drawingtype == DRAWFX_SECONDTRANS)
					{
						xLat0 = 0;
						xLat1 = pRNode->obj.secondtranslation;
					}
					else if (pRNode->obj.drawingtype == DRAWFX_DOUBLETRANS)
					{
						xLat0 = pOverlay->translation;
						xLat1 = pRNode->obj.secondtranslation;
					}
					else
					{
						xLat0 = pOverlay->translation;
						xLat1 = 0;
					}

					pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDibOv, xLat0, xLat1,
						pRNode->obj.drawingtype);
//					pPacket = D3DRenderPacketNew(pPool);
	//				pPacket = D3DRenderPacketNew(pPool);
	//				pPacket = NULL;
	//				pPool->curPacket--;

					if (NULL == pPacket)
						continue;

					pChunk = D3DRenderChunkNew(pPacket);
					assert(pChunk);

					pChunk->drawingtype = DRAWFX_TRANSLUCENT50;;
					pChunk->numIndices = 4;
					pChunk->numVertices = 4;
					pChunk->numPrimitives = pChunk->numVertices - 2;
					pChunk->xLat0 = xLat0;
					pChunk->xLat1 = xLat1;
					pChunk->zBias = zBias;

					zBias++;

					if (flags == DRAWFX_INVISIBLE)
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

					angle = pRNode->angle - (params->viewer_angle + 3072);

					if (angle < -4096)
						angle += 4096;

					/* Make sure that object is above the floor. */
					if (!GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y))
					{
						continue;
					}

					// Set object depth based on "depth" sector flags
					depthf = sector_depths[SectorDepth(sector_flags)];

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

					if (pRNode->obj.drawingtype == DRAWFX_BLACK)
					{
						bgra.b = bgra.g = bgra.r = 0;
						bgra.a = 255;
					}
					else
					{
						if (D3DObjectLightingCalc(room, pRNode, &bgra, 0))
							pChunk->flags |= D3DRENDER_NOAMBIENT;
					}

/*					if (pRNode->obj.id != INVALID_ID && pRNode->obj.id == GetUserTargetID())
					{
						bgra.b = 0.0f;
						bgra.g = 0.0f;
						bgra.r = 255.0f;
					}*/

					if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
						bgra.a = D3DRENDER_TRANS25;
					if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
						bgra.a = D3DRENDER_TRANS50;
					if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
						bgra.a = D3DRENDER_TRANS75;
					if (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
						bgra.a = D3DRENDER_TRANS50;
					if (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
						bgra.a = D3DRENDER_TRANS50;
					if (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)
						bgra.a = D3DRENDER_TRANS50;

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
					if ((pRNode->obj.id != INVALID_ID) && (pRNode->obj.id != player.id))
					{
						D3DMATRIX	localToScreen, rot, mat;
						custom_xyzw	topLeft, topRight, bottomLeft, bottomRight, center;
						ObjectRange *range = FindVisibleObjectById(pRNode->obj.id);
						int			w, h;
						int			tempLeft, tempRight, tempTop, tempBottom;
						int			distX, distY, distance;

						w = gD3DRect.right - gD3DRect.left;
						h = gD3DRect.bottom - gD3DRect.top;

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
						MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
						MatrixMultiply(&mat, &trans, &rot);
						XformMatrixPerspective(&localToScreen, FOV_H, FOV_V, 1.0f, Z_RANGE);
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

						if (flags == DRAWFX_INVISIBLE)
						{
							pChunk->st1[0].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[0].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[1].s = D3DRENDER_CLIP_TO_SCREEN_X(bottomRight.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[1].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[2].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[2].t = D3DRENDER_CLIP_TO_SCREEN_Y(bottomRight.y, gScreenHeight) / gScreenHeight;
							pChunk->st1[3].s = D3DRENDER_CLIP_TO_SCREEN_X(topLeft.x, gScreenWidth) / gScreenWidth;
							pChunk->st1[3].t = D3DRENDER_CLIP_TO_SCREEN_Y(topLeft.y, gScreenHeight) / gScreenHeight;

							//if ((gFrame & 15) < 15)
							{
				/*				pChunk->st1[0].s -= ((int)rand() % 3) / 256.0f;
								pChunk->st1[0].t -= ((int)rand() % 3) / 256.0f;
								pChunk->st1[1].s -= ((int)rand() % 3) / 256.0f;
								pChunk->st1[1].t += ((int)rand() % 3) / 256.0f;
								pChunk->st1[2].s += ((int)rand() % 3) / 256.0f;
								pChunk->st1[2].t += ((int)rand() % 3) / 256.0f;
								pChunk->st1[3].s += ((int)rand() % 3) / 256.0f;
								pChunk->st1[3].t -= ((int)rand() % 3) / 256.0f;*/
								pChunk->st1[0].s -= (gFrame & 3) / 256.0f;
								pChunk->st1[0].t -= (gFrame & 3) / 256.0f;
								pChunk->st1[1].s -= (gFrame & 3) / 256.0f;
								pChunk->st1[1].t += (gFrame & 3) / 256.0f;
								pChunk->st1[2].s += (gFrame & 3) / 256.0f;
								pChunk->st1[2].t += (gFrame & 3) / 256.0f;
								pChunk->st1[3].s += (gFrame & 3) / 256.0f;
								pChunk->st1[3].t -= (gFrame & 3) / 256.0f;
								/*pChunk->st1[0].s += (5) / 256.0f;
								pChunk->st1[0].t -= (5) / 256.0f;
								pChunk->st1[1].s += (5) / 256.0f;
								pChunk->st1[1].t += (5) / 256.0f;
								pChunk->st1[2].s -= (5) / 256.0f;
								pChunk->st1[2].t += (5) / 256.0f;
								pChunk->st1[3].s -= (5) / 256.0f;
								pChunk->st1[3].t -= (5) / 256.0f;*/
							}
						}

/*						if (((D3DRENDER_CLIP(topLeft.x, 1.0f) ||
							D3DRENDER_CLIP(topLeft.y, 1.0f)) &&
							(D3DRENDER_CLIP(bottomRight.x, 1.0f) ||
							D3DRENDER_CLIP(bottomRight.y, 1.0f))) &&
							D3DRENDER_CLIP(topLeft.z, 1.0f))*/
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
							tempLeft   = (topLeft.x * w / 2) + (w >> 1);
							tempRight  = (bottomRight.x * w / 2) + (w >> 1);
							tempTop    = (topLeft.y * -h / 2) + (h >> 1);
							tempBottom = (bottomRight.y * -h / 2) + (h >> 1);

							if (config.large_area)
							{
								tempLeft >>= 1;
								tempRight >>= 1;
								tempTop >>= 1;
								tempBottom >>= 1;
							}

							distX = pRNode->motion.x - player.x;
							distY = pRNode->motion.y - player.y;

							distance = DistanceGet(distX, distY);

							if (range == NULL)
							{
								// Set up new visible object
								range = &visible_objects[num_visible_objects];
								range->id       = pRNode->obj.id;
								range->distance = distance;
								range->left_col   = tempLeft;
								range->right_col  = tempRight;
								range->top_row    = tempTop;
								range->bottom_row = tempBottom;

								num_visible_objects = ++num_visible_objects > MAXOBJECTS ? MAXOBJECTS : num_visible_objects;
							}
							
							/* Record boundaries of drawing area */
							range->left_col   = min(range->left_col, tempLeft);
							range->right_col  = max(range->right_col, tempRight);

							if (tempTop <= range->top_row)
							{
								int	i;
								pRNode->boundingHeightAdjust =
									((pDib->height / pDib->shrink) - pDib->yoffset) / 4.0f;
								i = 0;
							}

							range->top_row    = min(range->top_row, tempTop);
							range->bottom_row = max(range->bottom_row, tempBottom);
						}
					}

					if (pRNode->obj.id != INVALID_ID && pRNode->obj.id == GetUserTargetID() &&
						(pRNode->obj.drawingtype != DRAWFX_INVISIBLE))
					{
						pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDibOv, xLat0, xLat1,
															pRNode->obj.drawingtype);
						if (NULL == pPacket)
							continue;
						pPacket->pMaterialFctn = &D3DMaterialObjectPacket;

						pChunk = D3DRenderChunkNew(pPacket);
						assert(pChunk);

						pChunk->drawingtype = DRAWFX_TRANSLUCENT50;
						pChunk->numIndices = 4;
						pChunk->numVertices = 4;
						pChunk->numPrimitives = pChunk->numVertices - 2;
						pChunk->xLat0 = xLat0;
						pChunk->xLat1 = xLat1;
						pChunk->zBias = ZBIAS_TARGETED;
						pChunk->isTargeted = TRUE;

						MatrixMultiply(&pChunk->xForm, &rot, &mat);

						if (flags == DRAWFX_INVISIBLE)
						{
							pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
						}
						else
						{
							pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
						}

						D3DObjectLightingCalc(room, pRNode, &bgra, 0);

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
									pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
								break;

								case 1:
									pChunk->bgra[i].b = min(bgra.r * 2.0f, 255);
									pChunk->bgra[i].g = 0;
									pChunk->bgra[i].r = 0;
									pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
								break;

								default:
									pChunk->bgra[i].b = 0;
									pChunk->bgra[i].g = min(bgra.r * 2.0f, 255);
									pChunk->bgra[i].r = 0;
									pChunk->bgra[i].a = 255;//D3DRENDER_TRANS50;
								break;
							}
						}

						pChunk->xyz[0].x -= 96.0f  * invOvShrink;
						pChunk->xyz[0].z += 96.0f  * invOvShrink;
						pChunk->xyz[1].x -= 96.0f  * invOvShrink;
						pChunk->xyz[1].z -= 96.0f  * invOvShrink;
						pChunk->xyz[2].x += 96.0f  * invOvShrink;
						pChunk->xyz[2].z -= 96.0f  * invOvShrink;
						pChunk->xyz[3].x += 96.0f  * invOvShrink;
						pChunk->xyz[3].z += 96.0f  * invOvShrink;

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

					gNumObjects++;
TEMP_END2:
					{
						int i = 0;
					}
				}
			}
		}
	}
}

void D3DRenderProjectilesDrawNew(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params)
{
	D3DMATRIX			mat, rot;
	int					angleHeading, anglePitch;
	int					i;
	Projectile			*pProjectile;
	list_type			list;
	long				dx, dy, angle;
	PDIB				pDib;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	// base objects
	for (list = room->projectiles; list != NULL; list = list->next)
	{
		BYTE	xLat0, xLat1;

		pProjectile = (Projectile *)list->data;

		dx = pProjectile->motion.x - params->viewer_x;
		dy = pProjectile->motion.y - params->viewer_y;

		angle = (pProjectile->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

		pDib = GetObjectPdib(pProjectile->icon_res, angle, pProjectile->animate.group);

		if (NULL == pDib)
			continue;

		xLat0 = pProjectile->translation;
		xLat1 = 0;

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, xLat0, xLat1, 0);
		if (NULL == pPacket)
			return;

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

		angle = pProjectile->angle - (params->viewer_angle + 3072);

		if (angle < -4096)
			angle += 4096;

		MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
		MatrixTranspose(&rot, &rot);
		MatrixTranslate(&mat, (float)pProjectile->motion.x, (float)pProjectile->motion.z,
			(float)pProjectile->motion.y);
		MatrixMultiply(&pChunk->xForm, &rot, &mat);

		pChunk->xyz[0].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		pChunk->xyz[0].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

		pChunk->xyz[1].x = (float)pDib->width / (float)pDib->shrink * -8.0f + (float)pDib->xoffset;
		pChunk->xyz[1].z = -(float)pDib->yoffset * 4.0f;

		pChunk->xyz[2].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		pChunk->xyz[2].z = -(float)pDib->yoffset * 4.0f;

		pChunk->xyz[3].x = (float)pDib->width / (float)pDib->shrink * 8.0f + (float)pDib->xoffset;
		pChunk->xyz[3].z = ((float)pDib->height / (float)pDib->shrink * 16.0f) - (float)pDib->yoffset * 4.0f;

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

		gNumObjects++;
	}
}

void D3DRenderPlayerOverlaysDraw(d3d_render_pool_new *pPool, room_type *room, Draw3DParams *params)
{
	D3DMATRIX			mat;
	room_contents_node	*pRNode;
	PDIB				pDib;
	custom_bgra			bgra;
	AREA				objArea;
	float				screenW, screenH;

	int i, count;
	object_node *obj;
	list_type overlays;
	BYTE drawingtype;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)(MAXX * stretchfactor);
	screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * stretchfactor);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	// Get player's object drawing flags for special drawing effects
	pRNode = GetRoomObjectById(player.id);
	if (pRNode == NULL)
		drawingtype = 0;
	else
		drawingtype = pRNode->obj.drawingtype;

	for (i=0; i < NUM_PLAYER_OVERLAYS; i++)
	{
		BYTE	xLat0, xLat1;

		PlayerOverlay *pOverlay = &player.poverlays[i];

		if (pOverlay->obj == NULL || pOverlay->hotspot == 0)
			continue;

		obj = pOverlay->obj;

		pDib = GetObjectPdib(obj->icon_res, 0, obj->animate->group);
		if (pDib == NULL)
			continue;

		D3DComputePlayerOverlayArea(pDib, pOverlay->hotspot, &objArea);
//		objArea.x *= screenW;
//		objArea.y *= screenH;
//		objArea.cx /= screenW;
//		objArea.cy /= screenH;

		overlays = *(obj->overlays);

		if (overlays != NULL)
			D3DRenderPlayerOverlayOverlaysDraw(pPool, overlays, pDib, room, params,
				&objArea, TRUE);

		if (obj->drawingtype == DRAWFX_SECONDTRANS)
		{
			xLat0 = 0;
			xLat1 = obj->secondtranslation;
		}
		else if (obj->drawingtype == DRAWFX_DOUBLETRANS)
		{
			xLat0 = obj->translation;
			xLat1 = obj->secondtranslation;
		}
		else
		{
			xLat0 = obj->translation;
			xLat1 = 0;
		}

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, xLat0, xLat1,
			pRNode->obj.drawingtype);
		if (NULL == pPacket)
			return;

		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = xLat0;
		pChunk->xLat1 = xLat1;

		if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectInvisiblePacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectInvisibleChunk;
		}
		else
		{
			pPacket->pMaterialFctn = &D3DMaterialObjectPacket;
			pChunk->pMaterialFctn = &D3DMaterialObjectChunk;
		}

		pChunk->drawingtype = pRNode->obj.drawingtype;

		MatrixIdentity(&pChunk->xForm);

		if (pRNode->obj.drawingtype == DRAWFX_BLACK)
		{
			bgra.b = bgra.g = bgra.r = 0;
			bgra.a = 255;
		}
		else
		{
			D3DObjectLightingCalc(room, pRNode, &bgra, 0);
/*				lastDistance = D3DRenderObjectLightGetNearest(pRNode);

			light = D3DRenderObjectGetLight(room->tree, pRNode);

			if ((pRNode->obj.flags & OF_FLICKERING) || (pRNode->obj.flags & OF_FLASHING))
				light = GetLightPaletteIndex(D3DRENDER_LIGHT_DISTANCE, light, FINENESS,
							-pRNode->obj.lightAdjust);
			else
				light = GetLightPaletteIndex(D3DRENDER_LIGHT_DISTANCE, light, FINENESS,
							0);

			light = light * COLOR_AMBIENT / 64;

			bgra.r = bgra.g = bgra.b = min(255, light + lastDistance);
			bgra.a = 255;*/
		}

		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
			bgra.a = D3DRENDER_TRANS25;
		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
			bgra.a = D3DRENDER_TRANS75;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
			bgra.a = D3DRENDER_TRANS50;
		if (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)
			bgra.a = D3DRENDER_TRANS50;

		pChunk->xyz[0].x = pChunk->xyz[1].x = objArea.x;
		pChunk->xyz[0].z = pChunk->xyz[3].z = objArea.y;
		pChunk->xyz[2].x = pChunk->xyz[3].x = pChunk->xyz[0].x + objArea.cx;
		pChunk->xyz[2].z = pChunk->xyz[1].z = pChunk->xyz[0].z + objArea.cy;

//		D3DObjectLightingCalc(room, pRNode, &bgra);

		for (count = 0; count < 4; count++)
		{
			if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
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

		pChunk->st1[0].t -= (gFrame & 3) / 256.0f;
		pChunk->st1[1].s -= (gFrame & 3) / 256.0f;
		pChunk->st1[1].t += (gFrame & 3) / 256.0f;
		pChunk->st1[2].s += (gFrame & 3) / 256.0f;
		pChunk->st1[2].t += (gFrame & 3) / 256.0f;
		pChunk->st1[3].s += (gFrame & 3) / 256.0f;
		pChunk->st1[3].t -= (gFrame & 3) / 256.0f;

/*		pChunk->st1[0].s = 0.0f;
		pChunk->st1[0].t = 0.0f;
		pChunk->st1[1].s = 0.0f;
		pChunk->st1[1].t = 1.0f;
		pChunk->st1[2].s = 1.0f;
		pChunk->st1[2].t = 1.0f;
		pChunk->st1[3].s = 1.0f;
		pChunk->st1[3].t = 0.0f;*/

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		if (overlays != NULL)
			D3DRenderPlayerOverlayOverlaysDraw(pPool, overlays, pDib, room, params,
				&objArea, FALSE);
	}
}

void D3DRenderPlayerOverlayOverlaysDraw(d3d_render_pool_new *pPool, list_type overlays,
		PDIB pDib, room_type *room, Draw3DParams *params, AREA *objArea, BOOL underlays)
{
	int					pass, depth;
	room_contents_node	*pRNode;
	custom_xyz			xyz[4];
	custom_bgra			bgra;
	PDIB				pDibOv;
	Overlay				*pOverlay;
	int					i, zBias;
	float				lastDistance, screenW, screenH;
	BYTE				xLat0, xLat1, drawingtype;

	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)(MAXX * (stretchfactor * 1.5f));
	screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * (stretchfactor * 1.5f));

   // Get player's object drawing flags for special drawing effects
	pRNode = GetRoomObjectById(player.id);

	if (pRNode == NULL)
		drawingtype = 0;
	else
		drawingtype = pRNode->obj.drawingtype;

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

			pOverlay = (Overlay *)overlays->data;

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

			if (pRNode->obj.drawingtype == DRAWFX_SECONDTRANS)
			{
				xLat0 = 0;
				xLat1 = pRNode->obj.secondtranslation;
			}
			else if (pRNode->obj.drawingtype == DRAWFX_DOUBLETRANS)
			{
				xLat0 = pOverlay->translation;
				xLat1 = pRNode->obj.secondtranslation;
			}
			else
			{
				xLat0 = pOverlay->translation;
				xLat1 = 0;
			}

			pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDibOv, xLat0, xLat1,
				pRNode->obj.drawingtype);
			if (NULL == pPacket)
				return;

			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);

			pChunk->drawingtype = pRNode->obj.drawingtype;
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pChunk->xLat0 = xLat0;
			pChunk->xLat1 = xLat1;
			
			if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
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

			if (pRNode->obj.drawingtype == DRAWFX_BLACK)
			{
				bgra.b = bgra.g = bgra.r = 0;
				bgra.a = 255;
			}
			else
			{
				D3DObjectLightingCalc(room, pRNode, &bgra, 0);
			}

			if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT25)
				bgra.a = D3DRENDER_TRANS25;
			if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT50)
				bgra.a = D3DRENDER_TRANS50;
			if (pRNode->obj.drawingtype == DRAWFX_TRANSLUCENT75)
				bgra.a = D3DRENDER_TRANS75;
			if (pRNode->obj.drawingtype == DRAWFX_DITHERTRANS)
				bgra.a = D3DRENDER_TRANS50;
			if (pRNode->obj.drawingtype == DRAWFX_DITHERINVIS)
				bgra.a = D3DRENDER_TRANS50;
			if (pRNode->obj.drawingtype == DRAWFX_DITHERGREY)
				bgra.a = D3DRENDER_TRANS50;

			for (i = 0; i < 4; i++)
			{
				if (pRNode->obj.drawingtype == DRAWFX_INVISIBLE)
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

			pChunk->st1[0].t -= (gFrame & 3) / 256.0f;
			pChunk->st1[1].s -= (gFrame & 3) / 256.0f;
			pChunk->st1[1].t += (gFrame & 3) / 256.0f;
			pChunk->st1[2].s += (gFrame & 3) / 256.0f;
			pChunk->st1[2].t += (gFrame & 3) / 256.0f;
			pChunk->st1[3].s += (gFrame & 3) / 256.0f;
			pChunk->st1[3].t -= (gFrame & 3) / 256.0f;

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}
}

void D3DRenderViewElementsDraw(d3d_render_pool_new *pPool)
{
	int						i;
	float					screenW, screenH, foffset;
	int						offset = 0;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)(MAXX * stretchfactor);
	screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)(MAXY * stretchfactor);

	if (GetFocus() == hMain)
		offset = 4;

	foffset = 1.0f / 64.0f;

	if (stretchfactor == 1)
	{
		// top left
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[0 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(72 / (float)screenH, gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(58 / (float)screenW, gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(72 / (float)screenH, gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(58 / (float)screenW, gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// top right
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[1 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(72 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(72 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// bottom left
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[2 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 73 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 73 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// bottom right
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[3 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 73 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 58 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight,
			gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 73 / (float)screenH, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}
	else
	{
		// top left
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[0 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(56 / (float)screenH, gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(46 / (float)screenW, gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(56 / (float)screenH, gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(46 / (float)screenW, gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// top right
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[1 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 47 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 47 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(56 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(56 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// bottom left
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[2 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 56 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 0.5f, gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(46 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 0.5f,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(46 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 56 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// bottom right
		pPacket = D3DRenderPacketNew(pPool);
		pPacket->pDib = NULL;
		pPacket->pTexture = gpViewElements[3 + offset];
		pPacket->xLat0 = 0;
		pPacket->xLat1 = 0;
		pPacket->effect = 0;
		pPacket->size = pPool->packetSize;

		pChunk = D3DRenderChunkNew(pPacket);
		pChunk->flags = 0;
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pChunk->xLat0 = 0;
		pChunk->xLat1 = 0;

		pPacket->pMaterialFctn = D3DMaterialObjectPacket;
		pChunk->pMaterialFctn = D3DMaterialNone;

		pChunk->xyz[0].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 47 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[0].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 56 / (float)screenH,
			gScreenHeight);
		pChunk->xyz[0].y = VIEW_ELEMENT_Z;
		pChunk->xyz[1].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - 47 / (float)screenW,
			gScreenWidth);
		pChunk->xyz[1].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 0.5f,
			gScreenHeight);
		pChunk->xyz[1].y = VIEW_ELEMENT_Z;
		pChunk->xyz[2].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[2].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 0.5f,
			gScreenHeight);
		pChunk->xyz[2].y = VIEW_ELEMENT_Z;
		pChunk->xyz[3].x = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth,
			gScreenWidth);
		pChunk->xyz[3].z = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - 56 / (float)screenH, gScreenHeight);
		pChunk->xyz[3].y = VIEW_ELEMENT_Z;

		for (i = 0; i < 4; i++)
		{
			pChunk->bgra[i].b = 255;
			pChunk->bgra[i].g = 255;
			pChunk->bgra[i].r = 255;
			pChunk->bgra[i].a = 255;
		}

		pChunk->st0[0].s = foffset;
		pChunk->st0[0].t = foffset;
		pChunk->st0[1].s = foffset;
		pChunk->st0[1].t = 1.0f - foffset;
		pChunk->st0[2].s = 1.0f - foffset;
		pChunk->st0[2].t = 1.0f - foffset;
		pChunk->st0[3].s = 1.0f - foffset;
		pChunk->st0[3].t = foffset;

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}
}

void D3DPostOverlayEffects(d3d_render_pool_new *pPool)
{
	static DWORD			timeLastFrame = 0;
	DWORD					timeCurrent, timeDelta;
	int						i;
	d3d_render_chunk_new	*pChunk;
	d3d_render_packet_new	*pPacket;

	timeCurrent = timeGetTime();
	timeDelta = timeCurrent - timeLastFrame;
	timeLastFrame = timeCurrent;

	// Flash of XLAT.  Could be color, blindness, whatever.
	if (effects.flashxlat != XLAT_IDENTITY)
	{
		custom_bgra	bgra;

		effects.duration -= (int)timeDelta;
		switch (effects.flashxlat)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		if (effects.duration <= 0)
		{
			effects.flashxlat = XLAT_IDENTITY;
			effects.duration = 0;
		}

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	if (effects.xlatOverride > 0)
	{
		custom_bgra	bgra;

		switch (effects.xlatOverride)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// May be drawn over room or map.

	// whiteout
	if (effects.whiteout)
	{
		int	whiteout = min(effects.whiteout, 500);

		whiteout = whiteout * COLOR_MAX / 500;
		whiteout = max(whiteout, 200);

		pPacket = D3DRenderPacketFindMatch(pPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, COLOR_MAX, COLOR_MAX, COLOR_MAX, whiteout);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// Pain (always drawn last).
	if (config.pain)
	{
		if (effects.pain)
		{
			int	pain = min(effects.pain, 2000);

			pain = pain * 204 / 2000;

			pPacket = D3DRenderPacketFindMatch(pPool, NULL, NULL, 0, 0, 0);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
			pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
			MatrixIdentity(&pChunk->xForm);

			for (i = 0; i < 4; i++)
			{
				CHUNK_BGRA_SET(pChunk, i, 0, 0, COLOR_MAX, pain);
			}

			CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight));
			CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight));

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}
}

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

Bool D3DMaterialWorldDynamicChunk(d3d_render_chunk_new *pChunk)
{
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
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)0));
			}
			else
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
			}
		}
	}
	else
	{
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
	}

	if (gD3DDriverProfile.bFogEnable)
	{
		float	end;

		end = D3DRenderFogEndCalc(pChunk);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	return TRUE;
}

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
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)0));
			}
			else
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
			}
		}
	}
	else
	{
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
	}

	if (gD3DDriverProfile.bFogEnable)
	{
		float	end;

		end = D3DRenderFogEndCalc(pChunk);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	return TRUE;
}

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

Bool D3DMaterialMaskChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->flags & D3DRENDER_NOCULL)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	else
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)pChunk->zBias * -0.00001f));

	return TRUE;
}

Bool D3DMaterialNone(d3d_render_chunk_new *pPool)
{
	return TRUE;
}

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

Bool D3DMaterialLMapDynamicChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)0));
			}
			else
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
			}
		}
	}
	else
	{
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
	}

	return TRUE;
}

Bool D3DMaterialLMapStaticChunk(d3d_render_chunk_new *pChunk)
{
	if (pChunk->pSideDef == NULL)
	{
		if (pChunk->pSector)
		{
			if (pChunk->pSector->ceiling == current_room.sectors[0].ceiling)
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)0));
			}
			else
			{
				IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
			}
		}
	}
	else
	{
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)ZBIAS_WORLD * -0.00001f));
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

	return TRUE;
}

Bool D3DMaterialObjectPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	return TRUE;
}

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

Bool D3DMaterialObjectChunk(d3d_render_chunk_new *pChunk)
{
	static BYTE	lastXLat0, lastXLat1;

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	if ((pChunk->xLat0) || (pChunk->xLat1))
	{
		D3DRenderPaletteSet(pChunk->xLat0, pChunk->xLat1, pChunk->drawingtype);
		lastXLat0 = pChunk->xLat0;
		lastXLat1 = pChunk->xLat1;
	}
	else if ((lastXLat0) || (lastXLat1))
	{
		D3DRenderPaletteSet(0, 0, 0);
		lastXLat0 = lastXLat1 = 0;
	}

	// apply Z-BIAS here to make sure coplanar object-layers are rendered correctly
	// layer-ordering is saved in pChunk->zBias
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)pChunk->zBias * -0.00001f));

	if (pChunk->drawingtype == DRAWFX_TRANSLUCENT25)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS25 - 1);
	else if (pChunk->drawingtype == DRAWFX_TRANSLUCENT50)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (pChunk->drawingtype == DRAWFX_TRANSLUCENT75)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS75 - 1);
	else if (pChunk->drawingtype == DRAWFX_DITHERTRANS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, 1);
	else if (pChunk->drawingtype == DRAWFX_DITHERINVIS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (pChunk->drawingtype == DRAWFX_DITHERGREY)
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

Bool D3DMaterialObjectInvisibleChunk(d3d_render_chunk_new *pChunk)
{
	static BYTE	lastXLat0, lastXLat1;

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	if ((pChunk->xLat0) || (pChunk->xLat1))
	{
		D3DRenderPaletteSet(pChunk->xLat0, pChunk->xLat1, pChunk->drawingtype);
		lastXLat0 = pChunk->xLat0;
		lastXLat1 = pChunk->xLat1;
	}
	else if ((lastXLat0) || (lastXLat1))
	{
		D3DRenderPaletteSet(0, 0, 0);
		lastXLat0 = lastXLat1 = 0;
	}

	// apply Z-BIAS here to make sure coplanar object-layers are rendered correctly
	// layer-ordering is saved in pChunk->zBias
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DEPTHBIAS, F2DW((float)pChunk->zBias * -0.00001f));

	if (pChunk->drawingtype == DRAWFX_TRANSLUCENT25)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS25 - 1);
	else if (pChunk->drawingtype == DRAWFX_TRANSLUCENT50)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (pChunk->drawingtype == DRAWFX_TRANSLUCENT75)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS75 - 1);
	else if (pChunk->drawingtype == DRAWFX_DITHERTRANS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      1);
	else if (pChunk->drawingtype == DRAWFX_DITHERINVIS)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	else if (pChunk->drawingtype == DRAWFX_DITHERGREY)
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF,
                                      D3DRENDER_TRANS50 - 1);
	return TRUE;
}

Bool D3DMaterialEffectPool(d3d_render_pool_new *pPool)
{
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
   IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
   IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
   IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_DIFFUSE, 0);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_DIFFUSE, 0);

	return TRUE;
}

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

Bool D3DMaterialBlurPool(d3d_render_pool_new *pPool)
{
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG2, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

	return TRUE;
}

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

Bool D3DMaterialBlurChunk(d3d_render_chunk_new *pChunk)
{
	return TRUE;
}

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

Bool D3DMaterialParticlePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem)
{
	return TRUE;
}

Bool D3DMaterialParticleChunk(d3d_render_chunk_new *pChunk)
{
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &pChunk->xForm);

	return TRUE;
}

Bool D3DMaterialEffectChunk(d3d_render_chunk_new *pChunk)
{
	return TRUE;
}

LPDIRECT3DTEXTURE9 D3DRenderFramebufferTextureCreate(LPDIRECT3DTEXTURE9	pTex0,
													 LPDIRECT3DTEXTURE9	pTex1,
													 float width, float height)
{
	LPDIRECT3DSURFACE9	pSrc, pDest[2], pZBuf;
	RECT				rect;
	D3DMATRIX			mat;
	HRESULT				hr;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	D3DCacheSystemReset(&gEffectCacheSystem);
	D3DRenderPoolReset(&gEffectPool, &D3DMaterialEffectPool);

	// get pointer to backbuffer surface and z/stencil surface
	hr = IDirect3DDevice9_GetRenderTarget(gpD3DDevice, 0, &pSrc);
	hr = IDirect3DDevice9_GetDepthStencilSurface(gpD3DDevice, &pZBuf);

	// get pointer to texture surface for rendering
	hr = IDirect3DTexture9_GetSurfaceLevel(pTex0, 0, &pDest[0]);
	hr = IDirect3DTexture9_GetSurfaceLevel(pTex1, 0, &pDest[1]);

	rect.left = rect.top = 0;
	rect.right = gScreenWidth;
	rect.bottom = gScreenHeight;

	// copy framebuffer to texture
	hr = IDirect3DDevice9_StretchRect(gpD3DDevice, pSrc, &rect, pDest[0], &rect, D3DTEXF_NONE);
	
	// clear local->screen transforms
	MatrixIdentity(&mat);
	hr = IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	hr = IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	hr = IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);

	hr = IDirect3DDevice9_SetRenderTarget(gpD3DDevice, 0, pDest[1]);

	pPacket = D3DRenderPacketNew(&gEffectPool);
	if (NULL == pPacket)
		return NULL;
	pChunk = D3DRenderChunkNew(pPacket);
	pPacket->pMaterialFctn = D3DMaterialEffectPacket;
	pChunk->pMaterialFctn = D3DMaterialEffectChunk;
	pPacket->pTexture = pTex0;
	pChunk->numIndices = pChunk->numVertices = 4;
	pChunk->numPrimitives = pChunk->numVertices - 2;
	MatrixIdentity(&pChunk->xForm);

	CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, 256),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(0, 256));
	CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, 256),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(256, 256));
	CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(256, 256),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(256, 256));
	CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(256, 256),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(0, 256));

	CHUNK_ST0_SET(pChunk, 0, 0.0f, 0.0f);
	CHUNK_ST0_SET(pChunk, 1, 0.0f, gScreenHeight / 1024.0f);
	CHUNK_ST0_SET(pChunk, 2, gScreenWidth / 1024.0f, gScreenHeight / 1024.0f);
	CHUNK_ST0_SET(pChunk, 3, gScreenWidth / 1024.0f, 0.0f);

	CHUNK_BGRA_SET(pChunk, 0, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX);
	CHUNK_BGRA_SET(pChunk, 1, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX);
	CHUNK_BGRA_SET(pChunk, 2, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX);
	CHUNK_BGRA_SET(pChunk, 3, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX);

	CHUNK_INDEX_SET(pChunk, 0, 1);
	CHUNK_INDEX_SET(pChunk, 1, 2);
	CHUNK_INDEX_SET(pChunk, 2, 0);
	CHUNK_INDEX_SET(pChunk, 3, 3);

	D3DCacheFill(&gEffectCacheSystem, &gEffectPool, 1);
	D3DCacheFlush(&gEffectCacheSystem, &gEffectPool, 1, D3DPT_TRIANGLESTRIP);

	// restore render target to backbuffer
	hr = IDirect3DDevice9_SetRenderTarget(gpD3DDevice, 0, pSrc);
	hr = IDirect3DDevice9_SetDepthStencilSurface(gpD3DDevice, pZBuf);
	
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
   
	hr = IDirect3DSurface9_Release(pSrc);
	hr = IDirect3DSurface9_Release(pZBuf);
	hr = IDirect3DSurface9_Release(pDest[0]);
	hr = IDirect3DSurface9_Release(pDest[1]);

	return pTex1;
}

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
		end = (16384 + (light * FINENESS) + (p->viewer_light << 6));
	else
		end = (32768 + (max(0, light - LIGHT_NEUTRAL) * FINENESS) + (p->viewer_light << 6) +
		(current_room.ambient_light << LOG_FINENESS));

	return end;
}

/*
* IsHidden: Checks the coordinates (bounding box) against the player's frustum. If
*           the box doesn't fall inside the player's view, returns TRUE. Doesn't
*           account for height or occlusion. Modified from Bbox_shadowed in drawbsp.c.
*/
Bool IsHidden(Draw3DParams *params, long x0, long y0, long x1, long y1)
{
   long center_a, center_b, left_a, left_b, right_a, right_b;

   center_a = COS(params->viewer_angle) >> 6;
   center_b = SIN(params->viewer_angle) >> 6;

   left_a = -center_b + ((center_a * MAXX) >> LOG_VIEWER_DISTANCE);
   left_b = center_a + ((center_b * MAXX) >> LOG_VIEWER_DISTANCE);

   right_a = center_b + ((center_a * MAXX) >> LOG_VIEWER_DISTANCE);
   right_b = -center_a + ((center_b * MAXX) >> LOG_VIEWER_DISTANCE);

   x0 -= params->viewer_x;
   y0 -= params->viewer_y;
   x1 -= params->viewer_x;
   y1 -= params->viewer_y;

   if (center_a * x0 + center_b * y0 < 0)
   {
      if (center_a * x0 + center_b * y1 < 0)
      {
         if (center_a * x1 + center_b * y0 < 0)
         {
            if (center_a * x1 + center_b * y1 < 0)
            {
               return True;                /* all behind viewer */
            }
         }
      }
   }

   long l0, l1, l2, l3, r0, r1, r2, r3;

   l0 = left_a * x0 + left_b * y0;
   l1 = left_a * x0 + left_b * y1;
   l2 = left_a * x1 + left_b * y1;
   l3 = left_a * x1 + left_b * y0;

   if (l0 < 0 && l1 < 0 && l2 < 0 && l3 < 0)
   {
      return True;                /* all in quadrants 1&3 */
   }

   r0 = right_a * x0 + right_b * y0;
   r1 = right_a * x0 + right_b * y1;
   r2 = right_a * x1 + right_b * y1;
   r3 = right_a * x1 + right_b * y0;

   if (r0 < 0 && r1 < 0 && r2 < 0 && r3 < 0)
   {
      return True;                /* all in quadrants 2&3 */
   }

   return False;
}

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#define	TEX_CACHE_MAX_OBJECT	8000000
#define	TEX_CACHE_MAX_WORLD		8000000
#define	TEX_CACHE_MAX_LMAP		8000000
#define	TEX_CACHE_MAX_WALLMASK	2000000
#define	TEX_CACHE_MAX_EFFECT	2000000
#define	TEX_CACHE_MAX_PARTICLE	1000000

// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;

// Define field of views with magic numbers for tuning
inline float FovHorizontal(long width)
{
	return width / (float)(main_viewport_width) * (-PI / 3.78f);
}

inline float FovVertical(long height)
{
	return height / (float)(main_viewport_height) * (PI / 5.88f);
}

// Helper function to determine if an object should be rendered in the current pass based on transparency.
bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent)
{
	return transparent_pass == isTransparent;
}

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

particle_system			gParticleSystem;

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

// The size of the main full size render buffer and also a smaller buffer for effects.
// The smaller buffer is used for effects that don't need full resolution.
// As per the original specification, the smaller buffer is 1/4 the size of the full buffer.
int						gFullTextureSize;
int						gSmallTextureSize;

int						d3dRenderTextureThreshold;

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
												  unsigned int effect);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  unsigned int effect);
LPDIRECT3DTEXTURE9	D3DRenderTextureCreateFromResource(BYTE *ptr, int width, int height);

void				D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, unsigned int flags);
void				D3DRenderPaletteSetNew(UINT xlatID0, UINT xlatID1, unsigned int flags);

void				D3DRenderLMapsBuild(void);
void				D3DLMapsStaticGet(room_type *room);
void				D3DRenderFontInit(font_3d *pFont, HFONT hFont);
void				D3DRenderSkyboxDraw(d3d_render_pool_new *pPool, int angleHeading, int anglePitch);
void				D3DRenderBackgroundOverlays(d3d_render_pool_new* pPool, int angleHeading, int anglePitch, room_type* room, Draw3DParams* params);
void				D3DRenderSunDraw(int angleHeading, int anglePitch);

// new render stuff
void					D3DRenderPoolInit(d3d_render_pool_new *pPool, int size, int packetSize);
void					D3DRenderPoolShutdown(d3d_render_pool_new *pPool);
void					D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc);
d3d_render_packet_new	*D3DRenderPacketNew(d3d_render_pool_new *pPool);
void					D3DRenderPacketInit(d3d_render_packet_new *pPacket);
d3d_render_chunk_new	*D3DRenderChunkNew(d3d_render_packet_new *pPacket);
void					D3DRenderChunkInit(d3d_render_chunk_new *pChunk);
d3d_render_packet_new	*D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, int effect);

void					D3DRenderViewElementsDraw(d3d_render_pool_new *pPool);

void					D3DPostOverlayEffects(d3d_render_pool_new *pPool);

void					*D3DRenderMalloc(unsigned int bytes);

float					D3DRenderFogEndCalc(d3d_render_chunk_new *pChunk);

void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias) {
   float bias = z_bias * -0.00001f;
   IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS,
                                   *((DWORD *) &bias));
}

// externed stuff
extern void			DrawItemsD3D();
extern Bool			ComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area);
extern void			UpdateRoom3D(room_type *room, Draw3DParams *params);

int DistanceGet(int x, int y)
{
	int	distance;
	float	xf, yf;

	xf = (float)x;
	yf = (float)y;

	distance = sqrt((double)(xf * xf) + (double)(yf * yf));

	return (int)distance;
}

void *D3DRenderMalloc(unsigned int bytes)
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
	D3DDISPLAYMODE			displayMode;

	if (NULL == (gpD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	gD3DEnabled = D3DDriverProfileInit();
	if (!gD3DEnabled)
		return E_FAIL;

	IDirect3D9_GetAdapterDisplayMode(gpD3D, D3DADAPTER_DEFAULT, &displayMode);

	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &gD3DCaps);

   gFrame = 0;
   
	gViewport.X = 0;
	gViewport.Y = 0;
	gViewport.Width = gScreenWidth;
	gViewport.Height = gScreenHeight;
	gViewport.MinZ = 0.0f;
	gViewport.MaxZ = 1.0f;

	IDirect3DDevice9_SetViewport(gpD3DDevice, &gViewport);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_LIGHTING, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CLIPPING, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DITHERENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, TEMP_ALPHA_REF);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_LASTPIXEL, TRUE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
		D3DCOLORWRITEENABLE_BLUE);

	{

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
	}

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

   SetZBias(gpD3DDevice, 0);

	D3DRenderLMapsBuild();

	ReleaseCapture();

	if (gD3DDriverProfile.bFogEnable)
	{
		float	start = 0.0f;
		float	end = 50000.8f;
		DWORD	mode = D3DFOG_LINEAR;

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGCOLOR, 0);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGTABLEMODE,
                                      mode);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGSTART,
                                      *(DWORD *)(&start));
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND,
                                      *(DWORD *)(&end));
	}

	// create framebuffer textures
   for (int i = 0; i <= 15; i++)
      IDirect3DDevice9_CreateTexture(gpD3DDevice, gSmallTextureSize, gSmallTextureSize, 1,
                                     D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                     &gpBackBufferTex[i], NULL);
   
   IDirect3DDevice9_CreateTexture(gpD3DDevice, gFullTextureSize, gFullTextureSize, 1,
                                  D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                  &gpBackBufferTexFull, NULL);

	/***************************************************************************/
	/*                                FONT                                     */
	/***************************************************************************/

	// This will call D3DRenderFontInit to make sure the font texture is created
	GraphicsResetFont();

   playerOldPos.x = 0;
	playerOldPos.y = 0;
	playerOldPos.z = 0;

	return S_OK;
}

void D3DRenderShutDown(void)
{
	int	i, j;

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

      IDirect3DTexture9_Release(gpDLightWhite);
		gpDLightWhite = NULL;
      IDirect3DTexture9_Release(gpDLightOrange);
		gpDLightOrange = NULL;
      IDirect3DTexture9_Release(gpBloom);
		gpBloom = NULL;
      IDirect3DTexture9_Release(gpNoLookThrough);
		gpNoLookThrough = NULL;
      IDirect3DTexture9_Release(gpBackBufferTexFull);
		gpBackBufferTexFull = NULL;
      IDirect3DTexture9_Release(gpSunTex);
		gpSunTex = NULL;

		if (gFont.pTexture)
		{
         IDirect3DTexture9_Release(gFont.pTexture);
         delete [] gFont.kerningPairs;
         gFont.pTexture = NULL;
		}

		for (i = 0; i < 16; i++)
		{
         IDirect3DTexture9_Release(gpBackBufferTex[i]);
			gpBackBufferTex[i] = NULL;
		}

		for (j = 0; j < 5; j++)
			for (i = 0; i < 6; i++)
			{
				if (gpSkyboxTextures[j][i])
				{
               IDirect3DTexture9_Release(gpSkyboxTextures[j][i]);
					gpSkyboxTextures[j][i] = NULL;
				}
			}

		for (i = 0; i < NUM_VIEW_ELEMENTS; i++)
		{
			if (gpViewElements[i])
			{
				IDirect3DDevice9_Release(gpViewElements[i]);
				gpViewElements[i] = NULL;
			}
		}
      
		/***************************************************************************/
		/*                       VERTEX DECLARATIONS                               */
		/***************************************************************************/
		
		if (decl0dc) IDirect3DDevice9_Release(decl0dc);
		if (decl1dc) IDirect3DDevice9_Release(decl1dc);
		if (decl2dc) IDirect3DDevice9_Release(decl2dc);

		decl0dc = NULL;
		decl1dc = NULL;
		decl2dc = NULL;
      
      IDirect3DDevice9_Release(gpD3DDevice);
		gpD3DDevice = NULL;
      IDirect3D9_Release(gpD3D);
		gpD3D = NULL;
	}
}
D3DMATRIX	mat, rot, trans, view, proj, identity;
long		timeOverall, timeWorld, timeObjects, timeLMaps, timeSkybox, timeSetup, timeComplete;
void D3DRenderBegin(room_type *room, Draw3DParams *params)
{
	int			angleHeading, anglePitch;
	int			curPacket = 0;
	int			curIndex = 0;
	static ID	tempBkgnd = 0;
	room_contents_node *pRNode = nullptr;

	timeOverall = timeGetTime();
	timeSetup = timeGetTime();

	// If blind, don't draw anything
	Bool can_see = !effects.blind;
	Bool draw_sky = can_see;
	Bool draw_world = can_see;
	Bool draw_objects = can_see;
	Bool draw_particles = can_see;
	Bool draw_background_overlays = can_see;
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
		D3DRenderBackgroundSet(current_room.bkgnd);
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

	timeWorld = timeObjects = timeLMaps = timeSkybox = timeComplete = 0;

	gNumObjects = 0;
	gNumVertices = 0;
	gNumDPCalls = 0;

	p = params;

	gDLightCache.numLights = 0;
	gDLightCacheDynamic.numLights = 0;
	D3DLMapsStaticGet(room);

	IDirect3DDevice9_Clear(gpD3DDevice, 0, NULL, D3DCLEAR_TARGET |
		D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
		D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);

	D3DRenderPaletteSet(0, 0, 0);

	IDirect3DDevice9_BeginScene(gpD3DDevice);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);

	angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	anglePitch = PlayerGetHeightOffset();

	MatrixRotateY(&rot, (float)angleHeading * 360.0f / 4096.0f * PI / 180.0f);
	MatrixRotateX(&mat, (float)anglePitch * 45.0f / 414.0f * PI / 180.0f);
	MatrixMultiply(&rot, &rot, &mat);
	MatrixTranslate(&trans, -(float)params->viewer_x, -(float)params->viewer_height, -(float)params->viewer_y);
	MatrixMultiply(&view, &trans, &rot);

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);

	XformMatrixPerspective(&proj, FovHorizontal(gD3DRect.right - gD3DRect.left), FovVertical(gD3DRect.bottom - gD3DRect.top), 100.0f, Z_RANGE);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &proj);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_CW);

	D3DCacheSystemReset(&gObjectCacheSystem);
	D3DCacheSystemReset(&gLMapCacheSystem);
	D3DCacheSystemReset(&gWorldCacheSystem);

	SetZBias(gpD3DDevice, ZBIAS_DEFAULT);

	UpdateRoom3D(room, params);

	playerDeltaPos.x = params->viewer_x - playerOldPos.x;
	playerDeltaPos.y = params->viewer_y - playerOldPos.y;
	playerDeltaPos.z = params->viewer_height - playerOldPos.z;

	playerOldPos.x = params->viewer_x;
	playerOldPos.y = params->viewer_y;
	playerOldPos.z = params->viewer_height;

	timeSetup = timeGetTime() - timeSetup;

	if (draw_sky) // Render the skybox first
	{
		// Set render states for skybox
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
		SetZBias(gpD3DDevice, 0);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

		// Disable alpha blending and alpha testing for the skybox
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);

		// Disable fog for the skybox
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, FALSE);

		// Set texture stages for the skybox
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
		D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);
		D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, 0, 0);

		// Set vertex shader and declaration for the skybox
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		// Render the skybox
		D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
		D3DRenderSkyboxDraw(&gWorldPool, angleHeading, anglePitch);
		D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);
		D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

		// Restore render states after skybox rendering
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);

		// Restore alpha blending and alpha testing for subsequent rendering
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);

		// restore the correct view matrix
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);
	}

	if (gD3DRedrawAll & D3DRENDER_REDRAW_ALL)
	{
		D3DCacheSystemReset(&gWorldCacheSystemStatic);
		D3DCacheSystemReset(&gWallMaskCacheSystem);

		D3DRenderPoolReset(&gWorldPoolStatic, &D3DMaterialWorldPool);
		D3DRenderPoolReset(&gWallMaskPool, &D3DMaterialWallMaskPool);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
		D3DGeometryBuildNew(room, &gWorldPoolStatic, false);
		
		// Second pass: render transparent objects
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);  // Disable depth writing

		D3DGeometryBuildNew(room, &gWorldPoolStatic, true);

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

	// background overlays (e.g. the Sun & Moon)
	if (draw_background_overlays)
	{
		MatrixIdentity(&mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);

		D3DRenderPoolReset(&gWorldPool, &D3DMaterialWorldPool);
		D3DCacheSystemReset(&gWorldCacheSystem);
		D3DRenderBackgroundOverlays(&gWorldPool, angleHeading, anglePitch, room, params);
		D3DCacheFill(&gWorldCacheSystem, &gWorldPool, 1);
		D3DCacheFlush(&gWorldCacheSystem, &gWorldPool, 1, D3DPT_TRIANGLESTRIP);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGENABLE, TRUE);

		// restore the correct material and view matrices.
		MatrixIdentity(&mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
		IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &view);
	}

	if (draw_world)
	{
		D3DRenderWorld(room, params, pRNode);
	}

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	SetZBias(gpD3DDevice, 1);

	// draw particles
	if (draw_particles)
	{
		list_type	list;
		emitter		*pEmitter;

		for (list = gParticleSystem.emitterList; list != NULL; list = list->next)
		{
			pEmitter = (emitter *)list->data;

			if (pEmitter)
				D3DParticleEmitterUpdate(pEmitter, playerDeltaPos.x, playerDeltaPos.y, playerDeltaPos.z);
		}

		if (effects.sand)
		{
			IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
			IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl0dc);

			D3DParticleSystemUpdate(&gParticleSystem, &gParticlePool, &gParticleCacheSystem);
		}
	}

	if (draw_objects)
	{
		D3DRenderObjects(room, params, pRNode);
	}

	D3DRENDER_SET_COLOR_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);
	D3DRENDER_SET_ALPHA_STAGE(gpD3DDevice, 1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);

	SetZBias(gpD3DDevice, ZBIAS_DEFAULT);

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
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

		D3DCacheSystemReset(&gObjectCacheSystem);
		D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
		D3DPostOverlayEffects(&gObjectPool);
		D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
		D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
	}

	// test blur
	if (effects.blur || effects.waver)
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

		D3DRenderFramebufferTextureCreate(gpBackBufferTexFull, gpBackBufferTex[t],
			gSmallTextureSize, gSmallTextureSize);

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

	timeComplete = timeGetTime();
	// view elements (e.g. viewport corners)
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

	IDirect3DDevice9_EndScene(gpD3DDevice);
	RECT rect;

	rect.top = 0;
	rect.bottom = gScreenHeight;
	rect.left = 0;
	rect.right = gScreenWidth;

	HRESULT hr = IDirect3DDevice9_Present(gpD3DDevice, &rect, &gD3DRect, NULL, NULL);

	if (hr == D3DERR_DEVICELOST)
	{
		while (hr == D3DERR_DEVICELOST)
			hr = IDirect3DDevice9_TestCooperativeLevel(gpD3DDevice);

		if (hr == D3DERR_DEVICENOTRESET)
		{
			D3DRenderShutDown();
			D3DRenderInit(hMain);
			D3DGeometryBuildNew(room, &gWorldPoolStatic, false);
			D3DGeometryBuildNew(room, &gWorldPoolStatic, true);
		}
	}
	if ((gFrame & 255) == 255)
		debug(("number of vertices = %d\nnumber of dp calls = %d\n", gNumVertices, gNumDPCalls));

	timeComplete = timeGetTime() - timeComplete;
	timeOverall = timeGetTime() - timeOverall;

	//debug(("overall = %d lightmaps = %d world = %d objects = %d skybox = %d num vertices = %d setup = %d completion = %d (%d, %d, %d)\n"
	//, timeOverall, timeLMaps, timeWorld, timeObjects, timeSkybox, gNumVertices, timeComplete));

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

		if (gDLightCacheDynamic.numLights >= 50)
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

		if (gDLightCacheDynamic.numLights >= 50)
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

	// static lights
	for (list = room->contents; list != NULL; list = list->next)
	{
		pRNode = (room_contents_node *)list->data;

		if (gDLightCache.numLights >= 50)
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

// straight texture loader for objects
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  unsigned int effect)
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
	if (pDib->width < d3dRenderTextureThreshold)
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

	if (pDib->height < d3dRenderTextureThreshold)
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
		IDirect3DTexture9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

// texture loader that rotates for walls and such
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
												  unsigned int effect)
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
	if (pDib->width < d3dRenderTextureThreshold)
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

	if (pDib->height < d3dRenderTextureThreshold)
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
		IDirect3DTexture9_Release(pTexture);

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
	if (width < d3dRenderTextureThreshold)
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

	if (height < d3dRenderTextureThreshold)
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
		IDirect3DTexture9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromPNG(char *pFilename)
{
	FILE	*pFile;
	char	*filename = NULL;
	png_structp	pPng = NULL;
	png_infop	pInfo = NULL;
	png_infop	pInfoEnd = NULL;
	png_bytepp   rows;

	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	PALETTEENTRY		*pTemp = NULL;
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

  unsigned int image_width = png_get_image_width(pPng, pInfo);
  unsigned int image_height = png_get_image_height(pPng, pInfo);
	bytePP = png_get_bit_depth(pPng, pInfo) / 8;
	stride = image_width * bytePP - bytePP;
  
	{
		int	i;
		png_bytep	curRow;

		for (i = 0; i < 6; i++)
		{
			IDirect3DDevice9_CreateTexture(gpD3DDevice, image_width, image_height, 1, 0,
                                     D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL);

			IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

			pitchHalf = lockedRect.Pitch / 2;

			pBits = (unsigned char *)lockedRect.pBits;

			for (h = 0; h < image_height; h++)
			{
				curRow = rows[h];

				for (w = 0; w < image_width; w++)
				{
					for (b = 0; b < 4; b++)
					{
						if (b == 3)
							pBits[h * lockedRect.Pitch + w * 4 + b] = 255;
						else
							pBits[h * lockedRect.Pitch + w * 4 + (3 - b)] =
								curRow[(w * bytePP) + b];
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

void D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, unsigned int flags)
{
	xlat	*pXLat0, *pXLat1;
	Color	*pPalette;
	int		i;
	unsigned int	effect;

	return;

	pXLat0 = FindStandardXlat(xlatID0);
	pXLat1 = FindStandardXlat(xlatID1);

	pPalette = base_palette;
	effect = GetDrawingEffect(flags);

	switch (effect)
	{
		case OF_DRAW_PLAIN:
		case OF_DOUBLETRANS:
		case OF_BLACK:
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

		case OF_TRANSLUCENT25:
		case OF_TRANSLUCENT50:
		case OF_TRANSLUCENT75:
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

		case OF_INVISIBLE:
		break;

		case OF_TRANSLATE:
		break;

		case OF_DITHERINVIS:
			for (i = 0; i < 256; i++)
			{
				pXLat0 = FindStandardXlat(xlatID0);
				pXLat1 = FindStandardXlat(xlatID1);
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case OF_DITHERTRANS:
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

		case OF_SECONDTRANS:
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

void D3DRenderPaletteSetNew(UINT xlatID0, UINT xlatID1, unsigned int flags)
{
	xlat	*pXLat0, *pXLat1;
	Color	*pPalette;
	int		i;
	unsigned int	effect;

	pXLat0 = FindStandardXlat(xlatID0);
	pXLat1 = FindStandardXlat(xlatID1);

	pPalette = base_palette;
	effect = GetDrawingEffect(flags);

	switch (effect)
	{
		case OF_DRAW_PLAIN:
		case OF_DOUBLETRANS:
		case OF_BLACK:
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

		case OF_TRANSLUCENT25:
		case OF_TRANSLUCENT50:
		case OF_TRANSLUCENT75:
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

		case OF_INVISIBLE:
		break;

		case OF_TRANSLATE:
		break;

		case OF_DITHERINVIS:
			for (i = 0; i < 256; i++)
			{
				pXLat0 = FindStandardXlat(xlatID0);
				pXLat1 = FindStandardXlat(xlatID1);
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		break;

		case OF_DITHERTRANS:
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

		case OF_SECONDTRANS:
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
			float	scale = sqrtf((height - 16) * (height - 16) +
											(width - 16) * (width - 16));
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
			float	scale = sqrtf((height - 16) * (height - 16) +
											(width - 16) * (width - 16));

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
			float	scale = sqrtf((height - 64) * (height - 64) +
											(width - 64) * (width - 64));

			scale = 64.0f - scale;
			scale = max(scale, 0);
			scale /= 64.0f;

			if (scale > 0)
				scale = 1.0f;

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
			float	scale = sqrtf((height - 16) * (height - 16) +
											(width - 16) * (width - 16));
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

void D3DRenderFontInit(font_3d *pFont, HFONT hFont)
{
	D3DCAPS9		d3dCaps;
	HDC				hDC;
   HBITMAP			hbmBitmap;
	DWORD			*pBitmapBits;
   BITMAPINFO		bmi;
   long x = 0;
   long y = 0;
   TCHAR			str[2] = _T("x");
	TCHAR			c;
   SIZE			size;
	D3DLOCKED_RECT	d3dlr;
   WORD			*pDst16;
   BYTE			bAlpha;

   // Ask for a bigger font to reduce aliasing, then scale the texture
   // down by the same amount.
   float fontScale = 3.0;
   HFONT hScaledFont = FontsGetScaledFont(hFont, fontScale);
   assert(hScaledFont);
      
   pFont->fontHeight = GetFontHeight(hScaledFont);
   pFont->texScale = fontScale;
   
	if (pFont->fontHeight > 40)
		pFont->texWidth = pFont->texHeight = 1024;
	else if (pFont->fontHeight > 20)
		pFont->texWidth = pFont->texHeight = 512;
	else
		pFont->texWidth = pFont->texHeight = 256;

	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &d3dCaps);
  
	if (pFont->texWidth > (long) d3dCaps.MaxTextureWidth)
	{
		pFont->texScale *= (float)pFont->texWidth / (float)d3dCaps.MaxTextureWidth;
		pFont->texHeight = pFont->texWidth = d3dCaps.MaxTextureWidth;
	}
  
	if (pFont->pTexture)
		IDirect3DTexture9_Release(pFont->pTexture);
   
   IDirect3DDevice9_CreateTexture(
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
  
   SelectObject(hDC, hbmBitmap);
   SelectObject(hDC, hScaledFont);
   
   // Set text properties
   SetTextColor(hDC, RGB(255,255,255));
   SetBkColor(hDC, 0);
   SetBkMode(hDC, TRANSPARENT);
   SetTextAlign(hDC, TA_TOP);
   
   for(c = 32; c < 127; c++ )
   {
      int index = c-32;
      
      str[0] = c;
      GetTextExtentPoint32(hDC, str, 1, &size);
      
      if (!GetCharABCWidths(hDC, c, c, &pFont->abc[index])) {
         pFont->abc[index].abcA = 0;
         pFont->abc[index].abcB = size.cx;
         pFont->abc[index].abcC = 0;
      }

      size.cx = pFont->abc[index].abcB;

      // Is this row of the texture filled up?
      if (x + size.cx >= pFont->texWidth)
      {
         x = 0;
         y += size.cy + 1;
      }
      
      int left_offset = pFont->abc[index].abcA;
      ExtTextOut(hDC, x - left_offset, y+0, 0, NULL, str, 1, NULL);
      
      pFont->texST[index][0].s = ((FLOAT)(x+0)) / pFont->texWidth;
      pFont->texST[index][0].t = ((FLOAT)(y+0)) / pFont->texHeight;
      pFont->texST[index][1].s = ((FLOAT)(x+0 + size.cx)) / pFont->texWidth;
      pFont->texST[index][1].t = ((FLOAT)(y+0 + size.cy)) / pFont->texHeight;

      // Leave +1 space so bilinear filtering doesn't pick up neighboring character
      x += size.cx+1;  
   }
   
   IDirect3DTexture9_LockRect(pFont->pTexture, 0, &d3dlr, 0, 0);
   
   BYTE *pDstRow = (BYTE*)d3dlr.pBits;
   
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

   // Get kerning pairs for font
   pFont->numKerningPairs = GetKerningPairs(hDC, 0, NULL);
   pFont->kerningPairs = new KERNINGPAIR[pFont->numKerningPairs];
   GetKerningPairs(hDC, pFont->numKerningPairs, pFont->kerningPairs);
   
   DeleteObject(hbmBitmap);
   DeleteObject(hScaledFont);
   DeleteDC(hDC);
}

void D3DRenderBackgroundOverlays(d3d_render_pool_new* pPool, int angleHeading, int anglePitch, room_type* room, Draw3DParams* params)
{
	for (list_type list = room->bg_overlays; list != NULL; list = list->next)
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

		d3d_render_packet_new* pPacket = D3DRenderPacketFindMatch(pPool, NULL, pDib, 0, 0, 0);
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

		static bool initialized = false;
		static float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		static float v[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
		const float epsilon = 0.007f; // Small value to adjust UV coordinates inward
		if (!initialized)
		{
			// Set the texture coordinate template with epsilon adjustment once.
			u[0] = epsilon;
			v[0] = epsilon;
			u[1] = 1.0f - epsilon;
			v[1] = epsilon;
			u[2] = 1.0f - epsilon;
			v[2] = 1.0f - epsilon;
			u[3] = epsilon;
			v[3] = 1.0f - epsilon;
			initialized = true;
		}

		pChunk->st0[0].s = u[0];
		pChunk->st0[0].t = v[0];
		pChunk->st0[1].s = u[1];
		pChunk->st0[1].t = v[1];
		pChunk->st0[2].s = u[2];
		pChunk->st0[2].t = v[2];
		pChunk->st0[3].s = u[3];
		pChunk->st0[3].t = v[3];

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;

		// Determine if the background overlay is visible for click detection in client
		// (e.g. Blinded by the light of the Sun).
		ObjectRange* range = FindVisibleObjectById(overlay->obj.id);

		int w = gD3DRect.right - gD3DRect.left;
		int h = gD3DRect.bottom - gD3DRect.top;

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
		XformMatrixPerspective(&localToScreen, FovHorizontal(gD3DRect.right - gD3DRect.left), FovVertical(gD3DRect.bottom - gD3DRect.top), 1.0f, 2000000.0f);
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

			int distX = bg_overlay_pos.x - player.x;
			int distY = bg_overlay_pos.y - player.y;

			int distance = DistanceGet(distX, distY);

			if (range == NULL)
			{
				// Set up new visible object.
				range = &visible_objects[num_visible_objects];
				range->id = overlay->obj.id;
				range->distance = distance;
				range->left_col = tempLeft;
				range->right_col = tempRight;
				range->top_row = tempTop;
				range->bottom_row = tempBottom;

				num_visible_objects = min(num_visible_objects + 1, MAXOBJECTS);
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

			pChunk->bgra[j].b = gSkyboxBGRA[(i * 4 * 4) + (j * 4)];
			pChunk->bgra[j].g = gSkyboxBGRA[(i * 4 * 4) + (j * 4) + 1];
			pChunk->bgra[j].r = gSkyboxBGRA[(i * 4 * 4) + (j * 4) + 2];
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

int D3DRenderWallExtract(WallData *pWall, PDIB pDib, unsigned int *flags, custom_xyz *pXYZ,
						  custom_st *pST, custom_bgra *pBGRA, unsigned int type, int side)
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

				lightScale = (lightScale + FINENESS)>>1; // map to 0 to 1 range

				lightScale = lo_end + ((lightScale * shade_amount)>>LOG_FINENESS);
				
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

					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range

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

					lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range

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

      unsigned int image_width = png_get_image_width(pPng, pInfo);
      unsigned int image_height = png_get_image_height(pPng, pInfo);
      bytePP = png_get_bit_depth(pPng, pInfo) / 8;

			IDirect3DDevice9_CreateTexture(gpD3DDevice, image_width, image_height, 1, 0,
                                     D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpSkyboxTextures[index][i], NULL);

			IDirect3DTexture9_LockRect(gpSkyboxTextures[index][i], 0, &lockedRect, NULL, 0);

			pitchHalf = lockedRect.Pitch / 2;

			pBits = (unsigned char *)lockedRect.pBits;

			for (h = 0; h < image_height; h++)
			{
				curRow = rows[h];

				for (w = 0; w < image_width; w++)
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

	fclose(pFile);
}

void D3DRenderBackgroundSet(ID background)
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
												PDIB pDib, BYTE xLat0, BYTE xLat1, int effect)
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

void D3DRenderViewElementsDraw(d3d_render_pool_new *pPool)
{
   // Render view elements (such as the main viewport yellow ui corners)
	int						i;
	float					screenW, screenH, foffset;
	int						offset = 0;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;

	screenW = (float)(gD3DRect.right - gD3DRect.left) / (float)main_viewport_width;
	screenH = (float)(gD3DRect.bottom - gD3DRect.top) / (float)main_viewport_height;

	if (GetFocus() == hMain)
		offset = 4;

	foffset = 1.0f / 64.0f;

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

	SetZBias(gpD3DDevice, pChunk->zBias);

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
				SetZBias(gpD3DDevice, 0);
			else
				SetZBias(gpD3DDevice, ZBIAS_WORLD);
		}
	}
	else
		SetZBias(gpD3DDevice, ZBIAS_WORLD);

	return TRUE;
}

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
		D3DRenderPaletteSet(pChunk->xLat0, pChunk->xLat1, pChunk->flags);
		lastXLat0 = pChunk->xLat0;
		lastXLat1 = pChunk->xLat1;
	}
	else if ((lastXLat0) || (lastXLat1))
	{
		D3DRenderPaletteSet(0, 0, 0);
		lastXLat0 = lastXLat1 = 0;
	}

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
		D3DRenderPaletteSet(pChunk->xLat0, pChunk->xLat1, pChunk->flags);
		lastXLat0 = pChunk->xLat0;
		lastXLat1 = pChunk->xLat1;
	}
	else if ((lastXLat0) || (lastXLat1))
	{
		D3DRenderPaletteSet(0, 0, 0);
		lastXLat0 = lastXLat1 = 0;
	}

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
	POINT				pnt;
	D3DMATRIX			mat;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
   HRESULT hr;

	D3DCacheSystemReset(&gEffectCacheSystem);
	D3DRenderPoolReset(&gEffectPool, &D3DMaterialEffectPool);

	// get pointer to backbuffer surface and z/stencil surface
	IDirect3DDevice9_GetRenderTarget(gpD3DDevice, 0, &pSrc);
	IDirect3DDevice9_GetDepthStencilSurface(gpD3DDevice, &pZBuf);

	// get pointer to texture surface for rendering
	IDirect3DTexture9_GetSurfaceLevel(pTex0, 0, &pDest[0]);
	IDirect3DTexture9_GetSurfaceLevel(pTex1, 0, &pDest[1]);

	pnt.x = 0;
	pnt.y = 0;
	rect.left = rect.top = 0;
	rect.right = gScreenWidth;
	rect.bottom = gScreenHeight;

	// copy framebuffer to texture
	IDirect3DDevice9_StretchRect(gpD3DDevice, pSrc, &rect, pDest[0], &rect, D3DTEXF_NONE);
   
	// clear local->screen transforms
	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);

	IDirect3DDevice9_SetRenderTarget(gpD3DDevice, 0, pDest[1]);

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

	CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, gSmallTextureSize),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(0, gSmallTextureSize));
	CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, gSmallTextureSize),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(gSmallTextureSize, gSmallTextureSize));
	CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(gSmallTextureSize, gSmallTextureSize),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(gSmallTextureSize, gSmallTextureSize));
	CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(gSmallTextureSize, gSmallTextureSize),
		0, D3DRENDER_SCREEN_TO_CLIP_Y(0, gSmallTextureSize));

	CHUNK_ST0_SET(pChunk, 0, 0.0f, 0.0f);
	CHUNK_ST0_SET(pChunk, 1, 0.0f, gScreenHeight / (float)gFullTextureSize);
	CHUNK_ST0_SET(pChunk, 2, gScreenWidth / (float)gFullTextureSize, gScreenHeight / (float)gFullTextureSize);
	CHUNK_ST0_SET(pChunk, 3, gScreenWidth / (float)gFullTextureSize, 0.0f);

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
   
	IDirect3DSurface9_Release(pSrc);
	IDirect3DSurface9_Release(pZBuf);
	IDirect3DSurface9_Release(pDest[0]);
	IDirect3DSurface9_Release(pDest[1]);

	return pTex1;
}

void SandstormInit(void)
{
#define EMITTER_RADIUS	(12)
#define EMITTER_ENERGY	(40)
#define EMITTER_HEIGHT	(0)

	D3DParticleSystemReset(&gParticleSystem);
	// four corners, blowing around the perimeter
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		0, 500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		500.0f, 0, 0,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		0, -500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		-500.0f, 0, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);

	// four corners, blowing towards player
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		353.55f, 353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		353.55f, -353.55f, 0,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		-353.55f, -353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		-353.55f, 353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);

	// forward, left, right, and back, blowing towards player
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
		500.0f, 0.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
		-500.0f, 0, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
		0, -500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
		0, 500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);

	// four corners, blowing around the perimeter
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		0, 500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		500.0f, 0, 0,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		0, -500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		-500.0f, 0, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);

	// four corners, blowing towards player
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		353.55f, 353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		353.55f, -353.55f, 0,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
		-353.55f, -353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
		-353.55f, 353.55f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);

	// forward, left, right, and back, blowing towards player
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
		500.0f, 0.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
		-500.0f, 0, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
		0, -500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
	D3DParticleEmitterInit(&gParticleSystem,
		0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
		0, 500.0f, 0.0f,
		SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
		EMITTER_ENERGY, 1,
		0, -PI / 500.0f, -PI / 500.0f,
		1, 1024, 2);
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
		end = (16384 + (light * FINENESS) + (p->viewer_light * 64));
	else
		end = (32768 + (max(0, light - LIGHT_NEUTRAL) * FINENESS) + (p->viewer_light * 64) +
		(current_room.ambient_light * FINENESS));

	return end;
}

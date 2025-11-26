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

// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;

// Define field of views with magic numbers for tuning
float FovHorizontal(long width)
{
	return width / (float)(main_viewport_width) * (-PI / 3.78f);
}

float FovVertical(long height)
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

custom_xyz				playerOldPos;
custom_xyz				playerDeltaPos;

font_3d					gFont;

RECT					gD3DRect;
int						gD3DEnabled;
BYTE					gViewerLight = 0;
int						gNumObjects;
int						gNumVertices;
int						gNumDPCalls;
static PALETTEENTRY		gPalette[256];
int						gScreenWidth;
int						gScreenHeight;

static unsigned int		gFrame = 0;

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
int           gD3DRedrawAll = 0;
int						gTemp = 0;
bool					gWireframe;		// this is really bad, I'm sorry

extern long				viewer_height;
extern Color			base_palette[NUM_COLORS];
extern PDIB				background;         /* Pointer to background bitmap */
extern ObjectRange		visible_objects[];    /* Where objects are on screen */
extern int				num_visible_objects;
extern DrawItem			drawdata[];
extern long				nitems;
extern int				sector_depths[];
extern d3d_driver_profile	gD3DDriverProfile;
extern BYTE				*gBits;
extern BYTE				*gBufferBits;
extern D3DPRESENT_PARAMETERS	gPresentParam;
extern long				stretchfactor;
extern BYTE				light_rows[MAXY/2+1];      // Strength of light as function of screen row
extern ViewElement		ViewElements[];
extern HDC				gBitsDC;

D3DMATRIX view, mat, rot, trans, proj;

void				D3DRenderLMapsBuild(void);
void				D3DLMapsStaticGet(room_type *room);

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

void					*D3DRenderMalloc(unsigned int bytes);

void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias) {
   float bias = z_bias * -0.00001f;
   IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS,
                                   *((DWORD *) &bias));
}

int getD3dRenderThreshold()
{
	return d3dRenderTextureThreshold;
}

bool isManagedTexturesEnabled() {
    return gD3DDriverProfile.bManagedTextures;
}

bool isFogEnabled()
{
	return gD3DDriverProfile.bFogEnable;
}

void setWireframeMode(bool isEnabled)
{
	gWireframe = isEnabled;
}

PALETTEENTRY* getPalette()
{
    return gPalette;
}

const Color(&getBasePalette())[NUM_COLORS]
{
	return base_palette;
}

bool isWireframeMode()
{
	return gWireframe;
}

const font_3d& getFont3d()
{
	return gFont;
}

const LPDIRECT3DTEXTURE9 getWhiteLightTexture()
{
	return gpDLightWhite;
}

const LPDIRECT3DTEXTURE9 getBackBufferTextureZero()
{
	return gpBackBufferTex[0];
}

// externed stuff
extern void			DrawItemsD3D();
extern bool			ComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area);
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
	int	i;

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

		D3DRenderSkyBoxShutdown();

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

void D3DRenderBegin(room_type *room, Draw3DParams *params)
{
	int			angleHeading, anglePitch;
	int			curPacket = 0;
	int			curIndex = 0;
	room_contents_node *pRNode = nullptr;

	long		timeOverall, timeWorld, timeObjects, timeLMaps, timeSkybox, timeSetup, timeComplete;

	timeOverall = timeGetTime();
	timeSetup = timeGetTime();

	// Static variable to track the player's previous ability to see. Initialize it only once.
	static bool can_see = !effects.blind;

	// Determine the current ability to see.
	bool can_see_now = !effects.blind;

	// Trigger a redraw only if the player was blind, but now can see.
	if (!can_see && can_see_now) {
		gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;
	}

	// Update the can_see state for the next frame.
	can_see = can_see_now;

	// If blind (!can_see), don't draw anything
	bool draw_sky = can_see;
	bool draw_world = can_see;
	bool draw_objects = can_see;
	bool draw_particles = can_see;
	bool draw_background_overlays = can_see;

	if (D3DRenderUpdateSkyBox(room->bkgnd))
	{
		gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;
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

	setDrawParams(params);

	gDLightCache.numLights = 0;
	gDLightCacheDynamic.numLights = 0;
	D3DLMapsStaticGet(room);

	IDirect3DDevice9_Clear(gpD3DDevice, 0, NULL, D3DCLEAR_TARGET |
		D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
		D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);

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
		SkyboxRenderParams skyboxRenderParams(decl1dc, gD3DDriverProfile, gWorldPool, gWorldCacheSystem);
		D3DRenderSkyBox(params, angleHeading, anglePitch, view, skyboxRenderParams);
	}

	// Prepare our rendering parameters

	WorldCacheSystemParams worldCacheSystemParams(&gWorldCacheSystem, &gWorldCacheSystemStatic,
		&gLMapCacheSystem, &gLMapCacheSystemStatic, &gWallMaskCacheSystem);

	WorldPoolParams worldPoolParams(&gWorldPool, &gWorldPoolStatic, &gLMapPool, &gLMapPoolStatic, &gWallMaskPool);
		
	WorldRenderParams worldRenderParams(decl1dc, decl2dc, gD3DDriverProfile, worldCacheSystemParams, worldPoolParams, 
		view, proj);

	LightAndTextureParams lightAndTextureParams(&gDLightCache, &gDLightCacheDynamic, gSmallTextureSize, sector_depths);

	WorldPropertyParams worldPropertyParams(gpNoLookThrough, gpDLightOrange);

	if (gD3DRedrawAll & D3DRENDER_REDRAW_ALL)
	{
		D3DCacheSystemReset(&gWorldCacheSystemStatic);
		D3DCacheSystemReset(&gWallMaskCacheSystem);

		D3DRenderPoolReset(&gWorldPoolStatic, &D3DMaterialWorldPool);
		D3DRenderPoolReset(&gWallMaskPool, &D3DMaterialWallMaskPool);

		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
		D3DGeometryBuildNew(worldRenderParams, worldPropertyParams, lightAndTextureParams, false);
		
		// Second pass: render transparent objects
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);  // Disable depth writing

		D3DGeometryBuildNew(worldRenderParams, worldPropertyParams, lightAndTextureParams, true);

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
		BackgroundOverlaysRenderStateParams bgoRenderStateParams(decl1dc, gD3DDriverProfile, &gWorldPool, &gWorldCacheSystem, 
			view, mat, gD3DRect);
		BackgroundOverlaysSceneParams bgoSceneParams(&num_visible_objects, visible_objects, angleHeading, anglePitch, room, params);
		D3DRenderBackgroundOverlays(bgoRenderStateParams, bgoSceneParams);
	}

	if (draw_world)
	{
		timeWorld = D3DRenderWorld(worldRenderParams, worldPropertyParams, lightAndTextureParams);
	}

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_CULLMODE, D3DCULL_NONE);
	SetZBias(gpD3DDevice, 1);

	if (draw_particles)
	{
		ParticleSystemStructure particleSystemStructure(decl1dc, playerDeltaPos, &gParticlePool, &gParticleCacheSystem);
		D3DRenderParticles(particleSystemStructure);
	}

	if (draw_objects)
	{
		ObjectsRenderParams objectsRenderParams(decl1dc, decl2dc, gD3DDriverProfile, &gObjectPool, &gObjectCacheSystem, view, proj, room, params);

		GameObjectDataParams gameObjectDataParams(nitems, &num_visible_objects, &gNumObjects, drawdata, visible_objects, 
			gpBackBufferTexFull, gpBackBufferTex);

		FontTextureParams fontTextureParams(&gFont, gSmallTextureSize);

		PlayerViewParams playerViewParams(gScreenWidth, gScreenHeight, main_viewport_width, main_viewport_height, gD3DRect);

		timeObjects = D3DRenderObjects(objectsRenderParams, gameObjectDataParams, lightAndTextureParams, fontTextureParams, playerViewParams);
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

	FxRenderSystemStructure fxRenderSystemStructure(decl1dc, &gObjectPool, &gObjectCacheSystem, 
		&gEffectPool, &gEffectCacheSystem, gpBackBufferTex, gpBackBufferTexFull, 
		gFullTextureSize, gSmallTextureSize, mat, gFrame, gScreenWidth, gScreenHeight);

	// post overlay effects
	if (draw_objects)
	{
		D3DPostOverlayEffects(fxRenderSystemStructure);
	}

	// apply blur and wave distortion effects
	if (effects.blur || effects.waver)
	{
		MatrixIdentity(&mat);
		D3DFxBlurWaver(fxRenderSystemStructure);
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
			D3DGeometryBuildNew(worldRenderParams, worldPropertyParams, lightAndTextureParams, false);
			D3DGeometryBuildNew(worldRenderParams, worldPropertyParams, lightAndTextureParams, true);
		}
	}
	if ((gFrame & 255) == 255)
		debug(("number of vertices = %d\nnumber of dp calls = %d\n", gNumVertices, gNumDPCalls));

	timeComplete = timeGetTime() - timeComplete;
	timeOverall = timeGetTime() - timeOverall;

	//debug(("overall = %d lightmaps = %d world = %d objects = %d skybox = %d num vertices = %d setup = %d completion = %d (%d, %d, %d)\n"
	//, timeOverall, timeLMaps, timeWorld, timeObjects, timeSkybox, gNumVertices, timeComplete));

}

bool D3DLMapCheck(d_light *dLight, room_contents_node *pRNode)
{
	if (dLight->objID != pRNode->obj.id)
		return false;
	if (dLight->xyzScale.x != DLIGHT_SCALE(pRNode->obj.dLighting.intensity))
		return false;
	if (dLight->color.b != (pRNode->obj.dLighting.color & 31) * 255 / 31)
		return false;
	if (dLight->color.g != ((pRNode->obj.dLighting.color >> 5) & 31) * 255 / 31)
		return false;
	if (dLight->color.r != ((pRNode->obj.dLighting.color >> 10) & 31) * 255 / 31)
		return false;

	return true;
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

		GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pProjectile->motion.x, pProjectile->motion.y);

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

		GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y);

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

		GetRoomHeight(room->tree, &top, &bottom, &sector_flags, pRNode->motion.x, pRNode->motion.y);

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
			return false;
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
			return false;
		}
	}
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

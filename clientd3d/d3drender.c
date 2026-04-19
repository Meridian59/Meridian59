// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;

// Defines field of views
float FovHorizontal(long width)
{
	static constexpr float HORIZONTAL_TUNING_FACTOR  = (-PI / 3.78f);
	return width / static_cast<float>(main_viewport_width) * HORIZONTAL_TUNING_FACTOR ;
}

float FovVertical(long height)
{
	static constexpr float VERTICAL_TUNING_FACTOR = (PI / 5.88f);
	return height / static_cast<float>(main_viewport_height) * VERTICAL_TUNING_FACTOR;
}

// Helper function to determine if an object should be rendered in the current pass based on transparency.
bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent)
{
	return transparent_pass == isTransparent;
}

// Pool of textures used for off-screen rendering (drawing to memory instead of on screen).
// These are used for intermediate passes for things like dynamic lighting.
static constexpr int MAX_RENDER_TARGET_POOL = 16;

d3d_render_packet_new	*gpPacket;

LPDIRECT3DTEXTURE9		gpNoLookThrough = nullptr;
LPDIRECT3DTEXTURE9		gpBackBufferTex[MAX_RENDER_TARGET_POOL];
LPDIRECT3DTEXTURE9		gpBackBufferTexFull;
LPDIRECT3DTEXTURE9		gpViewElements[NUM_VIEW_ELEMENTS];

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
BYTE					gViewerLight = 0;
int						gNumObjects;
int						gNumDPCalls;
static PALETTEENTRY		gPalette[NUM_COLORS];

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
int						gD3DRedrawAll = 0;
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
extern ViewElement		ViewElements[];
extern HDC				gBitsDC;

D3DMATRIX view, mat, rot, trans, proj;

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

void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias)
{
   float bias = z_bias * -0.00001f;
   IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, *((DWORD *) &bias));
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
	return D3DRenderLightsGetWhite();
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
	float xf = static_cast<float>(x);
	float yf = static_cast<float>(y);
	int distance = sqrt((xf * xf) + (yf * yf));

	return static_cast<int>(distance);
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
	if (nullptr == (gpD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	gD3DEnabled = D3DDriverProfileInit();
	if (!gD3DEnabled)
		return E_FAIL;

	D3DDISPLAYMODE displayMode;
	IDirect3D9_GetAdapterDisplayMode(gpD3D, D3DADAPTER_DEFAULT, &displayMode);

	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &gD3DCaps);

	gFrame = 0;
	
	// Initializes D3D viewport to match current screen dimensions.
	// Defines screen dimensions (X, Y, Width, Height) and depth range (MinZ, MaxZ).
	gViewport = { 0, 0, static_cast<DWORD>(gScreenWidth), static_cast<DWORD>(gScreenHeight), 0.0f, 1.0f};

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
   for (int i = 0; i < MAX_RENDER_TARGET_POOL; i++)
      IDirect3DDevice9_CreateTexture(gpD3DDevice, gSmallTextureSize, gSmallTextureSize, 1,
                                     D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                     &gpBackBufferTex[i], nullptr);
   
   IDirect3DDevice9_CreateTexture(gpD3DDevice, gFullTextureSize, gFullTextureSize, 1,
                                  D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                  &gpBackBufferTexFull, nullptr);

	/***************************************************************************/
	/*                                FONT                                     */
	/***************************************************************************/

	// This will call D3DRenderFontInit to make sure the font texture is created
	GraphicsResetFont();

	playerOldPos = { 0, 0, 0 };

	return S_OK;
}

void D3DRenderShutDown(void)
{
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

		D3DRenderLightsShutdown();

		if (gpNoLookThrough)
		{
			IDirect3DTexture9_Release(gpNoLookThrough);
			gpNoLookThrough = nullptr;
		}
		if (gpBackBufferTexFull)
		{
			IDirect3DTexture9_Release(gpBackBufferTexFull);
			gpBackBufferTexFull = nullptr;
		}

		if (gFont.pTexture)
		{
         IDirect3DTexture9_Release(gFont.pTexture);
         delete [] gFont.kerningPairs;
         gFont.pTexture = nullptr;
		}

		for (int i = 0; i < MAX_RENDER_TARGET_POOL; i++)
		{
         IDirect3DTexture9_Release(gpBackBufferTex[i]);
			gpBackBufferTex[i] = nullptr;
		}

		D3DRenderSkyBoxShutdown();

		for (int i = 0; i < NUM_VIEW_ELEMENTS; i++)
		{
			if (gpViewElements[i])
			{
				IDirect3DDevice9_Release(gpViewElements[i]);
				gpViewElements[i] = nullptr;
			}
		}
      
		/***************************************************************************/
		/*                       VERTEX DECLARATIONS                               */
		/***************************************************************************/
		
		if (decl0dc) IDirect3DDevice9_Release(decl0dc);
		if (decl1dc) IDirect3DDevice9_Release(decl1dc);
		if (decl2dc) IDirect3DDevice9_Release(decl2dc);

		decl0dc = nullptr;
		decl1dc = nullptr;
		decl2dc = nullptr;
      
      IDirect3DDevice9_Release(gpD3DDevice);
		gpD3DDevice = nullptr;
      IDirect3D9_Release(gpD3D);
		gpD3D = nullptr;
	}
}

void D3DRenderBegin(room_type *room, Draw3DParams *params)
{
	room_contents_node *pRNode = nullptr;

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
		for (int i = 0; i < NUM_VIEW_ELEMENTS; i++)
		{
			gpViewElements[i] = D3DRenderTextureCreateFromResource(ViewElements[i].bits,
				ViewElements[i].width, ViewElements[i].height);
		}
	}

	gFrame++;

	gNumObjects = 0;
	gNumVertices = 0;
	gNumDPCalls = 0;

	setDrawParams(params);

	gDLightCache.numLights = 0;
	gDLightCacheDynamic.numLights = 0;
	LightCacheUpdateParams lightCacheParams{&gDLightCache, &gDLightCacheDynamic, gD3DRedrawAll};
	D3DLMapsStaticGet(room, lightCacheParams);

	IDirect3DDevice9_Clear(gpD3DDevice, 0, nullptr, D3DCLEAR_TARGET |
		D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
		D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);

	IDirect3DDevice9_BeginScene(gpD3DDevice);

	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);

	// A full 360-degree circle is 4096 game units. Player camera rotation is offset
	// by 270 degrees (3072 game units) to align it with the legacy engine orientation.
	int angleHeading = params->viewer_angle + 3072;
	if (angleHeading >= 4096)
		angleHeading -= 4096;

	int anglePitch = PlayerGetHeightOffset();

	MatrixRotateY(&rot, static_cast<float>(angleHeading) * 360.0f / 4096.0f * PI / 180.0f);
	MatrixRotateX(&mat, static_cast<float>(anglePitch) * 45.0f / 414.0f * PI / 180.0f);
	MatrixMultiply(&rot, &rot, &mat);
	MatrixTranslate(&trans, -static_cast<float>(params->viewer_x), -static_cast<float>(params->viewer_height), -static_cast<float>(params->viewer_y));
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

	playerDeltaPos = { 
		static_cast<float>(params->viewer_x) - playerOldPos.x, 
		static_cast<float>(params->viewer_y) - playerOldPos.y, 
		static_cast<float>(params->viewer_height) - playerOldPos.z 
	};
	
	playerOldPos = { 
		static_cast<float>(params->viewer_x),
		static_cast<float>(params->viewer_y),
		static_cast<float>(params->viewer_height)
	};

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

	WorldPropertyParams worldPropertyParams(gpNoLookThrough, D3DRenderLightsGetOrange());

	if (gD3DRedrawAll & D3DRENDER_REDRAW_ALL)
	{
		// Defer static cache rebuild while invert effect is active.
		// GetLightPaletteIndex returns PALETTE_INVERT during the flash effect, which would
		// cause incorrect lighting values to be baked into the static geometry cache.
		// Keep gD3DRedrawAll set so rebuild happens after the invert effect ends.
		if (effects.invert > 0)
		{
			// Skip rebuild this frame - will be processed when invert effect ends
		}
		else
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
		D3DRenderWorld(worldRenderParams, worldPropertyParams, lightAndTextureParams);

		// DEBUG: Draw circles at static light positions
		if (D3DLightsDebugPositionsEnabled() && config.bDynamicLighting)
		{
			LightDebugRenderParams debugParams{&gDLightCache, &gObjectPool, &gObjectCacheSystem};
			D3DRenderDebugLightPositions(params, debugParams);
		}
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

		D3DRenderObjects(objectsRenderParams, gameObjectDataParams, lightAndTextureParams, fontTextureParams, playerViewParams);
	}

	// Transparent walls are drawn LAST so that sprites/monsters behind them show
	// through correctly. Depth write is off during this pass; depth test remains on
	// so walls behind opaque geometry are still occluded.
	if (draw_world)
	{
		D3DRenderTransparentWallsPass(worldRenderParams);
	}

	D3DRender_SetColorStage(1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);
	D3DRender_SetAlphaStage(1, D3DTOP_DISABLE, D3DTA_CURRENT, D3DTA_TEXTURE);

	SetZBias(gpD3DDevice, ZBIAS_DEFAULT);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
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

	// view elements (e.g. viewport corners)
	D3DRender_SetColorStage(1, D3DTOP_DISABLE, 0, 0);
	D3DRender_SetAlphaStage(1, D3DTOP_DISABLE, 0, 0);

	D3DRender_SetAlphaTestState(TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
	D3DRender_SetAlphaBlendState(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

   IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
   IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, decl1dc);

	D3DRenderPoolReset(&gObjectPool, &D3DMaterialObjectPool);
	D3DCacheSystemReset(&gObjectCacheSystem);
	D3DRenderViewElementsDraw(&gObjectPool);
	D3DCacheFill(&gObjectCacheSystem, &gObjectPool, 1);
	D3DCacheFlush(&gObjectCacheSystem, &gObjectPool, 1, D3DPT_TRIANGLESTRIP);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, gD3DDriverProfile.magFilter);	
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, gD3DDriverProfile.minFilter);

	IDirect3DDevice9_EndScene(gpD3DDevice);
	
	RECT rect = GetScreenRect();

	HRESULT hr = IDirect3DDevice9_Present(gpD3DDevice, &rect, &gD3DRect, nullptr, nullptr);

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

void D3DRenderFontInit(font_3d *pFont, HFONT hFont)
{
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

	D3DCAPS9 d3dCaps;
	IDirect3DDevice9_GetDeviceCaps(gpD3DDevice, &d3dCaps);
  
	if ( pFont->texWidth > static_cast<long>(d3dCaps.MaxTextureWidth) )
	{
		pFont->texScale *= static_cast<float>(pFont->texWidth) / static_cast<float>(d3dCaps.MaxTextureWidth);
		pFont->texHeight = pFont->texWidth = d3dCaps.MaxTextureWidth;
	}
  
	if (pFont->pTexture)
		IDirect3DTexture9_Release(pFont->pTexture);
   
   IDirect3DDevice9_CreateTexture(
      gpD3DDevice, pFont->texWidth,
      pFont->texHeight, 1, 0, D3DFMT_A4R4G4B4,
      D3DPOOL_MANAGED, &pFont->pTexture, nullptr);
   
   BITMAPINFO bmi;
   memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = static_cast<int>(pFont->texWidth);
   bmi.bmiHeader.biHeight = -static_cast<int>(pFont->texHeight);
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biBitCount = 32;
   
   HDC hDC = CreateCompatibleDC(gBitsDC);
   DWORD *pBitmapBits;
   HBITMAP hbmBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (VOID**)&pBitmapBits, nullptr, 0 );
   SetMapMode(hDC, MM_TEXT);
  
   SelectObject(hDC, hbmBitmap);
   SelectObject(hDC, hScaledFont);
   
   // Set text properties
   SetTextColor(hDC, RGB(255,255,255));
   SetBkColor(hDC, 0);
   SetBkMode(hDC, TRANSPARENT);
   SetTextAlign(hDC, TA_TOP);
   
   TCHAR str[2] = _T("x");
   long x = 0;
   long y = 0;
   for(TCHAR c = 32; c < 127; c++ )
   {
      int index = c - 32;
      
      str[0] = c;
	  
	  SIZE size;
      GetTextExtentPoint32(hDC, str, 1, &size);
      
      if (!GetCharABCWidths(hDC, c, c, &pFont->abc[index]))
	  {
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
      ExtTextOut(hDC, x - left_offset, y+0, 0, nullptr, str, 1, nullptr);
      
      pFont->texST[index][0].s = ((FLOAT)(x+0)) / pFont->texWidth;
      pFont->texST[index][0].t = ((FLOAT)(y+0)) / pFont->texHeight;
      pFont->texST[index][1].s = ((FLOAT)(x+0 + size.cx)) / pFont->texWidth;
      pFont->texST[index][1].t = ((FLOAT)(y+0 + size.cy)) / pFont->texHeight;

      // Leave +1 space so bilinear filtering doesn't pick up neighboring character
      x += size.cx+1;  
   }
   
   D3DLOCKED_RECT d3dlr;
   IDirect3DTexture9_LockRect(pFont->pTexture, 0, &d3dlr, 0, 0);
   
   BYTE *pDstRow = (BYTE*)d3dlr.pBits;
   
   for(y = 0; y < pFont->texHeight; y++)
   {
      WORD *pDst16 = (WORD *)pDstRow;
      for(x = 0; x < pFont->texWidth; x++)
      {
         BYTE bAlpha = static_cast<BYTE>( (pBitmapBits[pFont->texWidth * y + x] & 0xff) >> 4 );
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
   pFont->numKerningPairs = GetKerningPairs(hDC, 0, nullptr);
   pFont->kerningPairs = new KERNINGPAIR[pFont->numKerningPairs];
   GetKerningPairs(hDC, pFont->numKerningPairs, pFont->kerningPairs);
   
   DeleteObject(hbmBitmap);
   DeleteObject(hScaledFont);
   DeleteDC(hDC);
}

// new render stuff
void D3DRenderPoolInit(d3d_render_pool_new *pPool, int size, int packetSize)
{
	pPool->size = size;
	pPool->curPacket = 0;
	d3d_render_packet_new *pPacket = (d3d_render_packet_new *)D3DRenderMalloc(sizeof(d3d_render_packet_new) * size);
	assert(pPacket);
	pPool->renderPacketList = list_create(pPacket);
	pPool->packetSize = packetSize;

	D3DRenderPoolReset(pPool, nullptr);

	for (u_int i = 0; i < pPool->size; i++)
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
	d3d_render_packet_new *pPacket;

	if (pPool->curPacket >= pPool->size)
	{
		if (pPool->curPacketList->next == nullptr)
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
	pPacket->pDib = nullptr;
	pPacket->pMaterialFctn = nullptr;
	pPacket->pTexture = nullptr;
	pPacket->xLat0 = 0;
	pPacket->xLat1 = 0;
}

d3d_render_chunk_new *D3DRenderChunkNew(d3d_render_packet_new *pPacket)
{
	if (pPacket->curChunk >= (pPacket->size - 1))
		return nullptr;
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
	pChunk->pSector = nullptr;
	pChunk->pSectorNeg = nullptr;
	pChunk->pSectorPos = nullptr;
	pChunk->pSideDef = nullptr;
	pChunk->pMaterialFctn = nullptr;
	pChunk->pRenderCache = nullptr;
}

d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, int effect)
{
	d3d_render_packet_new *pPacket;
	
	for (list_type list = pPool->renderPacketList; list != pPool->curPacketList->next; list = list->next)
	{
		pPacket = (d3d_render_packet_new *)list->data;

		u_int numPackets;
		if (list == pPool->curPacketList)
			numPackets = pPool->curPacket;
		else
			numPackets = pPool->size;

		// for each packet
		for (u_int count = 0; count < numPackets; count++, pPacket++)
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
   float screenW = static_cast<float>(gD3DRect.right - gD3DRect.left) / static_cast<float>(gScreenWidth);
   float screenH = static_cast<float>(gD3DRect.bottom - gD3DRect.top) / static_cast<float>(gScreenHeight);

   // Render view elements (such as the main viewport yellow ui corners).
   int offset = (GetFocus() == hMain) ? 4 : 0;
   
   // 0 = top-left
   // 1 = top-right
   // 2 = bottom-left
   // 3 = bottom-right
   static constexpr int NUM_CORNERS = 4;
   for (int i = 0; i < NUM_CORNERS; ++i)
   {
      float width = static_cast<float>(ViewElements[i + offset].width) / screenW;
      float height = static_cast<float>(ViewElements[i + offset].height) / screenH;

      float left, right, top, bottom;
	  
      if (i % 2 == 0)  // left side
      {
         left = D3DRENDER_SCREEN_TO_CLIP_X(0, gScreenWidth);
         right = D3DRENDER_SCREEN_TO_CLIP_X(width, gScreenWidth);
      }
      else  // right side
      {
         left = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth - width, gScreenWidth);
         right = D3DRENDER_SCREEN_TO_CLIP_X(gScreenWidth, gScreenWidth);
      }

      if (i < 2)  // top side
      {
         top = D3DRENDER_SCREEN_TO_CLIP_Y(0, gScreenHeight);
         bottom = D3DRENDER_SCREEN_TO_CLIP_Y(height, gScreenHeight);
      }
      else  // bottom side
      {
         top = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight - height, gScreenHeight);
         bottom = D3DRENDER_SCREEN_TO_CLIP_Y(gScreenHeight, gScreenHeight);
      }

      d3d_render_packet_new *pPacket = D3DRenderPacketNew(pPool);
      pPacket->pDib = nullptr;
      pPacket->pTexture = gpViewElements[i + offset];
      pPacket->xLat0 = 0;
      pPacket->xLat1 = 0;
      pPacket->effect = 0;
      pPacket->size = pPool->packetSize;

      d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
      pChunk->flags = 0;
      pChunk->numIndices = 4;
      pChunk->numVertices = 4;
      pChunk->numPrimitives = pChunk->numVertices - 2;
      pChunk->xLat0 = 0;
      pChunk->xLat1 = 0;

      pPacket->pMaterialFctn = D3DMaterialObjectPacket;
      pChunk->pMaterialFctn = D3DMaterialNone;

      pChunk->xyz[0] = { left, VIEW_ELEMENT_Z, top };
      pChunk->xyz[1] = { left, VIEW_ELEMENT_Z, bottom };	  
      pChunk->xyz[2] = { right, VIEW_ELEMENT_Z, bottom };
      pChunk->xyz[3] = { right, VIEW_ELEMENT_Z, top };	  

      for (auto& color : pChunk->bgra)
      {
         color = {255, 255, 255, 255}; // Solid white (no tinting)
      }

      // Half-pixel offset to prevent texture bleeding.
      static constexpr float foffset = 1.0f / 64.0f;
      pChunk->st0[0] = { foffset, foffset };
      pChunk->st0[1] = { foffset, (1.0f - foffset) };
      pChunk->st0[2] = { (1.0f - foffset), (1.0f - foffset) };
      pChunk->st0[3] = { (1.0f - foffset), foffset };

      pChunk->indices[0] = 1;
      pChunk->indices[1] = 2;
      pChunk->indices[2] = 0;
      pChunk->indices[3] = 3;
   }
}

LPDIRECT3DTEXTURE9 D3DRenderFramebufferTextureCreate(LPDIRECT3DTEXTURE9	pTex0,
													 LPDIRECT3DTEXTURE9	pTex1,
													 float width, float height)
{
	LPDIRECT3DSURFACE9	pSrc, pDest[2], pZBuf;

	D3DCacheSystemReset(&gEffectCacheSystem);
	D3DRenderPoolReset(&gEffectPool, &D3DMaterialEffectPool);

	// get pointer to backbuffer surface and z/stencil surface
	IDirect3DDevice9_GetRenderTarget(gpD3DDevice, 0, &pSrc);
	IDirect3DDevice9_GetDepthStencilSurface(gpD3DDevice, &pZBuf);

	// get pointer to texture surface for rendering
	IDirect3DTexture9_GetSurfaceLevel(pTex0, 0, &pDest[0]);
	IDirect3DTexture9_GetSurfaceLevel(pTex1, 0, &pDest[1]);

	POINT pnt = { 0, 0 };
	
	RECT rect = GetScreenRect();

	// copy framebuffer to texture
	IDirect3DDevice9_StretchRect(gpD3DDevice, pSrc, &rect, pDest[0], &rect, D3DTEXF_NONE);
   
	// clear local->screen transforms
	D3DMATRIX mat;
	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, FALSE);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, FALSE);

	IDirect3DDevice9_SetRenderTarget(gpD3DDevice, 0, pDest[1]);

	d3d_render_packet_new *pPacket = D3DRenderPacketNew(&gEffectPool);
	
	if (nullptr == pPacket)
		return nullptr;
	
	d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
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
	CHUNK_ST0_SET(pChunk, 1, 0.0f, gScreenHeight / static_cast<float>(gFullTextureSize));
	CHUNK_ST0_SET(pChunk, 2, gScreenWidth / static_cast<float>(gFullTextureSize), gScreenHeight / static_cast<float>(gFullTextureSize));
	CHUNK_ST0_SET(pChunk, 3, gScreenWidth / static_cast<float>(gFullTextureSize), 0.0f);

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
	HRESULT hr = IDirect3DDevice9_SetRenderTarget(gpD3DDevice, 0, pSrc);
	hr = IDirect3DDevice9_SetDepthStencilSurface(gpD3DDevice, pZBuf);
	hr = IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZWRITEENABLE, TRUE);
   
	IDirect3DSurface9_Release(pSrc);
	IDirect3DSurface9_Release(pZBuf);
	IDirect3DSurface9_Release(pDest[0]);
	IDirect3DSurface9_Release(pDest[1]);

	return pTex1;
}

// Controls alpha testing, which is a 'pass/fail' check for pixels based on their transparency.
void D3DRender_SetAlphaTestState(BOOL enable, DWORD alphaRef, D3DCMPFUNC comparisonFunc)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, enable);
	gpD3DDevice->SetRenderState(D3DRS_ALPHAREF, alphaRef);
	gpD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, comparisonFunc);
}

// Controls alpha blending, which mixes source color with destination color.
void D3DRender_SetAlphaBlendState(BOOL enable, D3DBLEND srcBlend, D3DBLEND dstBlend)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enable);
	gpD3DDevice->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	gpD3DDevice->SetRenderState(D3DRS_DESTBLEND, dstBlend);
}

// Enables stencil writing, and marks surfaces with provided reference.
void D3DRender_SetStencilMark(DWORD refValue)
{
	if (!gpD3DDevice) return;
    gpD3DDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    gpD3DDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    gpD3DDevice->SetRenderState(D3DRS_STENCILREF, refValue);
	// Only mark pixels that pass the depth test. And don't mark areas that are occluded.
    gpD3DDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
    gpD3DDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    gpD3DDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
}

// Filters drawing based on stencil buffer comparison.  Doesn't modify the buffer.
void D3DRender_SetStencilTest(D3DCMPFUNC comparisonFunc, DWORD refValue)
{
    if (!gpD3DDevice) return;
    gpD3DDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    gpD3DDevice->SetRenderState(D3DRS_STENCILFUNC, comparisonFunc);
    gpD3DDevice->SetRenderState(D3DRS_STENCILREF, refValue);
	// Don't change the stencil buffer during the test.
    gpD3DDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    gpD3DDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    gpD3DDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
}

// Disable stencil testing for subsequent rendering.
void D3DRender_DisableStencil()
{
    if (!gpD3DDevice) return;
    gpD3DDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

// Controls how textures and colors are mathematically combined, whether it's 2D sprites or 3D surfaces.
void D3DRender_SetColorStage(DWORD stage, D3DTEXTUREOP colorOp, DWORD arg1, DWORD arg2)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLOROP, colorOp);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, arg1);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, arg2);
}

// Controls how alpha transparency is mathematically combined for any rendered surface.
void D3DRender_SetAlphaStage(DWORD stage, D3DTEXTUREOP alphaOp, DWORD arg1, DWORD arg2)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, alphaOp);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, arg1);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, arg2);
}

// Binds vertex buffers (positions, colors, and texture coordinates) to GPU input streams.
void D3DRender_SetStreams(d3d_render_cache* pCache, int numStages)
{
	if (!gpD3DDevice || !pCache) return;
	
	// Tracks current stream index to ensure buffers are bound in order.
	int i = 0;
	
	gpD3DDevice->SetStreamSource(i++, pCache->xyzBuffer.pVBuffer, 0, sizeof(custom_xyz));
	gpD3DDevice->SetStreamSource(i++, pCache->bgraBuffer.pVBuffer, 0, sizeof(custom_bgra));
	
	for (int j = 0; j < numStages; j++)
	{
		gpD3DDevice->SetStreamSource(i++, pCache->stBuffer[j].pVBuffer, 0, sizeof(custom_st));
	}
	
	gpD3DDevice->SetIndices(pCache->indexBuffer.pIBuffer);
}

// Disconnects vertex buffers from the GPU input streams.
void D3DRender_ClearStreams(int numStages)
{
	if (!gpD3DDevice) return;
	
	// Tracks current stream index to ensure buffers are cleared in order.
	int i = 0;
	
	gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	
	for (int j = 0; j < numStages; j++)
	{
		gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	}
	
	gpD3DDevice->SetIndices(nullptr);
}

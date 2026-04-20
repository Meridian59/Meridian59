// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__

///////////////
// Constants //
///////////////
static constexpr int ZBIAS_UNDERUNDER = 1;
static constexpr int ZBIAS_UNDER = 3;
static constexpr int ZBIAS_UNDEROVER = 6;
static constexpr int ZBIAS_BASE = 10;
static constexpr int ZBIAS_OVERUNDER = 11;
static constexpr int ZBIAS_OVER = 13;
static constexpr int ZBIAS_OVEROVER = 15;
static constexpr int ZBIAS_TARGETED = 0;
static constexpr int ZBIAS_DEFAULT = 1;

static constexpr float VIEW_ELEMENT_Z = 0.01f;
static constexpr float PLAYER_OVERLAY_Z = 0.02f;

static constexpr int ZBIAS_WORLD = 2;
static constexpr int ZBIAS_MASK = 1;

static constexpr int D3DRENDER_REDRAW_UPDATE = 0x00000001;
static constexpr int D3DRENDER_REDRAW_ALL = 0x00000002;

// the far clipping plane distance, which determines the maximum depth of the visible scene.
static constexpr float Z_RANGE = 200000.0f;

// Standard ASCII table, minus the first 32 non-printable control characters.
static constexpr int NUM_CHARS = 128 - 32;

// Pool of textures used for off-screen rendering (drawing to memory instead of on screen).
// These are used for intermediate passes for things like dynamic lighting.
static constexpr int MAX_RENDER_TARGET_POOL = 16;

////////////////
// Structures //
////////////////
struct d_light
{
	custom_xyz	xyz;
	custom_xyz	xyzScale;
	custom_xyz	invXYZScale;
	custom_xyz	invXYZScaleHalf;
	custom_bgra	color;
	ID			objID;
    int			baseIntensity;  // Unflickered base intensity for cache validation
    WORD		baseColor;      // Raw 16-bit color for cache validation
};

struct d_light_cache
{
	int		numLights;
	d_light	dLights[MAX_DLIGHTS];
};

struct font_3d
{
	TCHAR        strFontName[80];
	long         fontHeight;

	LPDIRECT3DTEXTURE9	pTexture;
	long				 texWidth;
	long				 texHeight;
	float				 texScale;
	custom_st	   texST[NUM_CHARS][2];
	// Deal with underhanging and overhanging characters
	ABC          abc[NUM_CHARS];
	int          numKerningPairs;
	KERNINGPAIR *kerningPairs;
};

/////////////
// Globals //
/////////////
inline IDirect3D9* gpD3D = nullptr;
inline IDirect3DDevice9* gpD3DDevice = nullptr;

inline int gNumVertices = 0;
inline int gD3DEnabled = 0;
inline int gScreenWidth = 0;
inline int gScreenHeight = 0;

inline int d3dRenderTextureThreshold;

inline bool gWireframe = false;
inline font_3d gFont;

inline LPDIRECT3DTEXTURE9 gpBackBufferTex[MAX_RENDER_TARGET_POOL];
inline LPDIRECT3DTEXTURE9 gpBackBufferTexFull;

inline static PALETTEENTRY gPalette[NUM_COLORS];

// Main client windows current viewport area
// Derived from graphics.c
extern int main_viewport_width;
extern int main_viewport_height;

// Defined in d3ddriver.c
extern d3d_driver_profile gD3DDriverProfile;

// Derived from palette.c
extern Color base_palette[NUM_COLORS];

//////////////////////
// Helper Functions //
//////////////////////
constexpr float deg_to_rad(float degrees)
{
	constexpr float DEG_TO_RAD_FACTOR = PITWICE / 360.0f;
	return degrees * DEG_TO_RAD_FACTOR;
}
constexpr float rad_to_deg(float radians)
{	
	constexpr float RAD_TO_DEG_FACTOR = 360.0f / PITWICE;
	return radians * RAD_TO_DEG_FACTOR;
}

// Calculates light range in world units.
constexpr float dlight_scale(float intensity)
{
	// Conversion factor from light intensity to world units.
	constexpr float DLIGHT_SCALE_FACTOR = 14000.0f / 255.0f;
	// Minimum radius to ensure lights don't cull/pop too early.
	constexpr float DLIGHT_MIN_RADIUS = 4000.0f;
	
	return (intensity * DLIGHT_SCALE_FACTOR) + DLIGHT_MIN_RADIUS;
}

// Returns true if a coordinate is within symmetric range (-range, range). Assumes range is positive.
inline bool D3DRender_InBounds(float coordinate, float range)
{
	return fabs(coordinate) < range;
}

inline RECT GetScreenRect()
{
	// RECT struct is set in this order: Left -> Top -> Right -> Bottom
	return { 0, 0, gScreenWidth, gScreenHeight };
}

inline int D3DRenderIsEnabled()
{
	return gD3DEnabled;
}

inline void *D3DRenderMalloc(unsigned int bytes)
{
	return malloc(bytes);
}

inline void SetZBias(int z_bias)
{
   float bias = z_bias * -0.00001f;
   gpD3DDevice->SetRenderState(D3DRS_DEPTHBIAS, std::bit_cast<DWORD>(bias));
}

inline int DistanceGet(int x, int y)
{
	return static_cast<int>(sqrtf(static_cast<float>(x * x + y * y)));
}

// Helper function to determine if an object should be rendered in the current pass based on transparency.
inline bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent)
{
	return transparent_pass == isTransparent;
}

// Defines field of views
inline float FovHorizontal(long width)
{
	static constexpr float HORIZONTAL_TUNING_FACTOR  = (-PI / 3.78f);
	return width / static_cast<float>(main_viewport_width) * HORIZONTAL_TUNING_FACTOR ;
}

inline float FovVertical(long height)
{
	static constexpr float VERTICAL_TUNING_FACTOR = (PI / 5.88f);
	return height / static_cast<float>(main_viewport_height) * VERTICAL_TUNING_FACTOR;
}

// Retrieve the threshold value for determining whether to round up the dimensions of a texture.
inline int getD3dRenderThreshold()
{
	return d3dRenderTextureThreshold;
}

inline bool isManagedTexturesEnabled()
{
    return gD3DDriverProfile.bManagedTextures;
}

inline bool isFogEnabled()
{
	return gD3DDriverProfile.bFogEnable;
}

inline void setWireframeMode(bool isEnabled)
{
	gWireframe = isEnabled;
}

inline bool isWireframeMode()
{
	return gWireframe;
}

inline const font_3d& getFont3d()
{
	return gFont;
}

inline const LPDIRECT3DTEXTURE9 getBackBufferTextureZero()
{
	return gpBackBufferTex[0];
}

// Global palette array containing 256 color entries used for rendering textures in the current frame.
// This palette is dynamically updated based on the current rendering context.
inline PALETTEENTRY* getPalette()
{
    return gPalette;
}

// Base palette array containing predefined colors used as a reference for rendering effects.
// This palette remains constant and is used for color lookups and transformations.
inline const Color(&getBasePalette())[NUM_COLORS]
{
	return base_palette;
}

/////////////////////////
// Function Prototypes //
/////////////////////////
HRESULT				D3DRenderInit(HWND hWnd);
void				D3DRenderShutDown(void);
void				D3DRenderBegin(room_type *room, Draw3DParams *params);
void				D3DRenderResizeDisplay(int left, int top, int right, int bottom);
void				D3DRenderEnableToggle(void);
int					D3DRenderObjectGetLight(BSPnode *tree, room_contents_node *pRNode);
d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, int effect);												
d3d_render_packet_new *D3DRenderPacketNew(d3d_render_pool_new *pPool);
d3d_render_chunk_new *D3DRenderChunkNew(d3d_render_packet_new *pPacket);
void				D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc);
void				D3DRenderFontInit(font_3d *pFont, HFONT hFont);
LPDIRECT3DTEXTURE9  D3DRender_CaptureEffect(LPDIRECT3DTEXTURE9 pTex0, LPDIRECT3DTEXTURE9 pTex1);

// D3D State Functions
void D3DRender_SetAlphaTestState(BOOL enable, DWORD alphaRef, D3DCMPFUNC comparisonFunc);
void D3DRender_SetAlphaBlendState(BOOL enable, D3DBLEND srcBlend, D3DBLEND dstBlend);

void D3DRender_SetStencilMark(DWORD refValue);
void D3DRender_SetStencilTest(D3DCMPFUNC comparisonFunc, DWORD refValue);
void D3DRender_DisableStencil();

void D3DRender_SetColorStage(DWORD stage, D3DTEXTUREOP colorOp, DWORD arg1, DWORD arg2);
void D3DRender_SetAlphaStage(DWORD stage, D3DTEXTUREOP alphaOp, DWORD arg1, DWORD arg2);

void D3DRender_SetStreams(d3d_render_cache* pCache, int numStages);
void D3DRender_ClearStreams(int numStages);

////////////////////////
// External Functions //
////////////////////////

// Defined in object3d.c
int FindHotspotPdib(PDIB pdib, char hotspot, POINT* point);
const Vector3D& getSunVector();

// Defined in graphics.c
// Returns the max shading range (FINENESS-shade_amount) to FINENESS.
long getShadeAmount();

// Defined in d3drender_lights.c
LPDIRECT3DTEXTURE9 D3DRenderLightsGetWhite();

#endif	// __D3DRENDER_H__

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

// Geometry constants for rendering a quad as a triangular strip.
static constexpr int TRI_STRIP_INDICES  = 4;
static constexpr int TRI_STRIP_VERTICES = 4;
static constexpr int TRI_STRIP_PRIMITIVES = TRI_STRIP_VERTICES - 2;
static constexpr int TRI_STRIP_INDICES_PATTERN[] = { 1, 2, 0, 3 };

/////////////
// Globals //
/////////////
inline IDirect3D9* gpD3D = nullptr;
inline IDirect3DDevice9* gpD3DDevice = nullptr;

inline int gNumVertices = 0;
inline int gD3DEnabled = 0;
inline int gScreenWidth = 0;
inline int gScreenHeight = 0;

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

////////////////
// Prototypes //
////////////////

// Helper Function Prototypes //
int D3DRenderIsEnabled(void);
void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias);
int DistanceGet(int x, int y);
bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent);
float FovHorizontal(long width);
float FovVertical(long height);
int getD3dRenderThreshold();
bool isManagedTexturesEnabled();
bool isFogEnabled();
void setWireframeMode(bool isEnabled);
bool isWireframeMode();
const font_3d& getFont3d();
const LPDIRECT3DTEXTURE9 getBackBufferTextureZero();

// Global palette array containing 256 color entries used for rendering textures in the current frame.
// This palette is dynamically updated based on the current rendering context.
PALETTEENTRY* getPalette();

// Base palette array containing predefined colors used as a reference for rendering effects.
// This palette remains constant and is used for color lookups and transformations.
const Color(&getBasePalette())[NUM_COLORS];


// Main Function Prototypes //
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
IDirect3DTexture9*  D3DRender_CaptureEffect(IDirect3DTexture9* pTex0, IDirect3DTexture9* pTex1);

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

#endif	// __D3DRENDER_H__

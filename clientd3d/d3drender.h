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
// Inline Functions //
//////////////////////
inline constexpr float DEGREES_TO_RADIANS(float degrees)
{
	constexpr float DEG_TO_RAD_FACTOR = PITWICE / 360.0f;
	return degrees * DEG_TO_RAD_FACTOR;
}
inline constexpr float RADIANS_TO_DEGREES(float radians)
{	
	constexpr float RAD_TO_DEG_FACTOR = 360.0f / PITWICE;
	return radians * RAD_TO_DEG_FACTOR;
}

// Calculates light range in world units.
inline constexpr float DLIGHT_SCALE(float intensity)
{
	// Conversion factor from light intensity to world units.
	constexpr float DLIGHT_SCALE_FACTOR = 14000.0f / 255.0f;
	// Minimum radius to ensure lights don't cull/pop too early.
	constexpr float DLIGHT_MIN_RADIUS = 4000.0f;
	
	return (intensity * DLIGHT_SCALE_FACTOR) + DLIGHT_MIN_RADIUS;
}

// Checks if a coordinate is within the bounds of range.
inline constexpr bool D3DRENDER_CLIP(float coordinate, float range)
{
	const float absRange = (range < 0.0f) ? -range : range;
	return (coordinate > -absRange) && (coordinate < absRange);
}

// Controls alpha testing, which is a 'pass/fail' chck for pixels based on their transparency.
inline void D3DRender_SetAlphaTestState(BOOL enable, DWORD alphaRef, D3DCMPFUNC comparisonFunc)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, enable);
	gpD3DDevice->SetRenderState(D3DRS_ALPHAREF, alphaRef);
	gpD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, comparisonFunc);
}

// Controls alpha blending, which mixes source color with destination color.
inline void D3DRender_SetAlphaBlendState(BOOL enable, D3DBLEND srcBlend, D3DBLEND dstBlend)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enable);
	gpD3DDevice->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	gpD3DDevice->SetRenderState(D3DRS_DESTBLEND, dstBlend);
}

// Enables stencil writing, and marks surfaces with provided reference.
inline void D3DRender_SetStencilMark(DWORD refValue)
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
inline void D3DRender_SetStencilTest(D3DCMPFUNC comparisonFunc, DWORD refValue)
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
inline void D3DRender_DisableStencil()
{
    if (!gpD3DDevice) return;
    gpD3DDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

// Controls how textures and colors are mathematically combined, whether it's 2D sprites or 3D surfaces.
inline void D3DRender_SetColorStage(DWORD stage, D3DTEXTUREOP colorOp, DWORD arg1, DWORD arg2)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLOROP, colorOp);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, arg1);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, arg2);
}

// Controls how alpha transparency is mathematically combined for any rendered surface.
inline void D3DRender_SetAlphaStage(DWORD stage, D3DTEXTUREOP alphaOp, DWORD arg1, DWORD arg2)
{
	if (!gpD3DDevice) return;
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, alphaOp);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, arg1);
	gpD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, arg2);
}

// Controls the source of vertex data (positions, colors, and textures) for the rendering pipeline.
inline void D3DRender_SetStreams(d3d_render_cache* pCache, int numStages)
{
	if (!gpD3DDevice || !pCache) return;
	
	int i = 0;
	
	gpD3DDevice->SetStreamSource(i++, pCache->xyzBuffer.pVBuffer, 0, sizeof(custom_xyz));
	gpD3DDevice->SetStreamSource(i++, pCache->bgraBuffer.pVBuffer, 0, sizeof(custom_bgra));
	
	for (int j = 0; j < numStages; j++)
	{
		gpD3DDevice->SetStreamSource(i++, pCache->stBuffer[j].pVBuffer, 0, sizeof(custom_st));
	}
	
	gpD3DDevice->SetIndices(pCache->indexBuffer.pIBuffer);
}

// Disconnects vertext data sources from the rendering pipeline.
inline void D3DRender_ClearStreams(int numStages)
{
	if (!gpD3DDevice) return;
	
	int i = 0;
	
	gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	
	for (int j = 0; j < numStages; j++)
	{
		gpD3DDevice->SetStreamSource(i++, nullptr, 0, 0);
	}
	
	gpD3DDevice->SetIndices(nullptr);
}

/////////////////////////
// Function Prototypes //
/////////////////////////
HRESULT				D3DRenderInit(HWND hWnd);
void				D3DRenderShutDown(void);
void				D3DRenderBegin(room_type *room, Draw3DParams *params);
void				D3DRenderResizeDisplay(int left, int top, int right, int bottom);
void				D3DRenderEnableToggle(void);
int					D3DRenderIsEnabled(void);
int					D3DRenderObjectGetLight(BSPnode *tree, room_contents_node *pRNode);
d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, LPDIRECT3DTEXTURE9 pTexture,
												PDIB pDib, BYTE xLat0, BYTE xLat1, int effect);
d3d_render_packet_new *D3DRenderPacketNew(d3d_render_pool_new *pPool);
d3d_render_chunk_new *D3DRenderChunkNew(d3d_render_packet_new *pPacket);
void				D3DRenderPoolReset(d3d_render_pool_new *pPool, void *pMaterialFunc);
void				*D3DRenderMalloc(unsigned int bytes);
void				D3DRenderFontInit(font_3d *pFont, HFONT hFont);

LPDIRECT3DTEXTURE9  D3DRenderFramebufferTextureCreate(LPDIRECT3DTEXTURE9 pTex0, LPDIRECT3DTEXTURE9 pTex1, 
	float width, float height);

void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias);
int DistanceGet(int x, int y);

int FindHotspotPdib(PDIB pdib, char hotspot, POINT* point);

bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent);

float FovHorizontal(long width);
float FovVertical(long height);

// Retrieve the threshold value for determining whether to round up the dimensions of a texture.
int getD3dRenderThreshold();

// Returns the max shading range (FINENESS-shade_amount) to FINENESS
long getShadeAmount();

bool isManagedTexturesEnabled();
bool isFogEnabled();

const Vector3D& getSunVector();

void setWireframeMode(bool isEnabled);
bool isWireframeMode();

const font_3d& getFont3d();

const LPDIRECT3DTEXTURE9 getWhiteLightTexture();

const LPDIRECT3DTEXTURE9 getBackBufferTextureZero();

// Global palette array containing 256 color entries used for rendering textures in the current frame.
// This palette is dynamically updated based on the current rendering context.
PALETTEENTRY* getPalette();

// Base palette array containing predefined colors used as a reference for rendering effects.
// This palette remains constant and is used for color lookups and transformations.
const Color(&getBasePalette())[NUM_COLORS];

#endif	// __D3DRENDER_H__

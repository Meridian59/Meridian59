// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DRENDER_H__
#define __D3DRENDER_H__

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

// the far clipping plane distance, which determines the maximum depth of the visible scene.
#define Z_RANGE					(200000.0f)

#define D3DRENDER_SET_ALPHATEST_STATE(_pDevice, _enable, _refValue, _compareFunc)	\
do	\
{	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHATESTENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAREF, _refValue);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHAFUNC, _compareFunc);	\
} while (0)

#define D3DRENDER_SET_ALPHABLEND_STATE(_pDevice, _enable, _srcBlend, _dstBlend)	\
do	\
{	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ALPHABLENDENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_SRCBLEND, _srcBlend);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_DESTBLEND, _dstBlend);	\
} while (0)

#define D3DRENDER_SET_STENCIL_STATE(_pDevice, _enable, _stencilFunc, _refValue, _pass, _fail, _zfail)	\
do	\
{	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILENABLE, _enable);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILFUNC, _stencilFunc);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILREF, _refValue);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILPASS, _pass);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILFAIL, _fail);	\
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_STENCILZFAIL, _zfail);	\
} while (0)

#define D3DRENDER_SET_COLOR_STAGE(_pDevice, _stage, _opValue, _arg0Value, _arg1Value)	\
do	\
{	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLOROP,	_opValue);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLORARG1, _arg0Value);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_COLORARG2, _arg1Value);	\
} while (0)

#define D3DRENDER_SET_ALPHA_STAGE(_pDevice, _stage, _opValue, _arg0Value, _arg1Value)	\
do	\
{	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAOP,	_opValue);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAARG1, _arg0Value);	\
	IDirect3DDevice9_SetTextureStageState(_pDevice, _stage, D3DTSS_ALPHAARG2, _arg1Value);	\
} while (0)

#define D3DRENDER_SET_STREAMS(_pDevice, _pCache, _numStages)	\
do	\
{	\
	int	_i = 0;	\
	int	_j;	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
      (_pCache)->xyzBuffer.pVBuffer, 0, sizeof(custom_xyz)); \
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
      (_pCache)->bgraBuffer.pVBuffer, 0, sizeof(custom_bgra)); \
	for (_j = 0; _j < _numStages; _j++)	\
		IDirect3DDevice9_SetStreamSource(_pDevice, _i++,	\
         (_pCache)->stBuffer[_j].pVBuffer, 0, sizeof(custom_st)); \
	IDirect3DDevice9_SetIndices(_pDevice, (_pCache)->indexBuffer.pIBuffer);	\
} while (0)

#define D3DRENDER_CLEAR_STREAMS(_pDevice, _numStages)	\
do	\
{	\
	int	_i = 0;	\
	int	_j;	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	for (_j = 0; _j < _numStages; _j++)	\
		IDirect3DDevice9_SetStreamSource(_pDevice, _i++, NULL, 0, 0);	\
	IDirect3DDevice9_SetIndices(_pDevice,	NULL);	\
} while (0)

typedef struct d_light
{
	custom_xyz	xyz;
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

static const int numChars = 128 - 32;
typedef struct font_3d
{
	TCHAR        strFontName[80];
	long         fontHeight;

	LPDIRECT3DTEXTURE9	pTexture;
	long				 texWidth;
	long				 texHeight;
	float				 texScale;
	custom_st	   texST[numChars][2];
  // Deal with underhanging and overhanging characters
  ABC          abc[numChars];
  int          numKerningPairs;
  KERNINGPAIR *kerningPairs;
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

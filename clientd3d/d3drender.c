// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

static constexpr int TEX_CACHE_MAX_WALLMASK = 2000000;
static constexpr int TEX_CACHE_MAX_EFFECT = 1000000;
static constexpr int TEX_CACHE_MAX_PARTICLE = 1000000;

// Pool of textures used for off-screen rendering (drawing to memory instead of on screen).
// These are used for intermediate passes for things like dynamic lighting.
static constexpr int MAX_RENDER_TARGET_POOL = 16;

///////////////
// Variables //
///////////////
d3d_render_packet_new	*gpPacket;

IDirect3DTexture9*		gpNoLookThrough = nullptr;
IDirect3DTexture9*		gpBackBufferTex[MAX_RENDER_TARGET_POOL];
IDirect3DTexture9*		gpBackBufferTexFull;
IDirect3DTexture9*		gpViewElements[NUM_VIEW_ELEMENTS];

D3DVIEWPORT9			gViewport;
D3DCAPS9				gD3DCaps;

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

custom_xyz				playerOldPos = {0.0f, 0.0f, 0.0f};
custom_xyz				playerDeltaPos = {0.0f, 0.0f, 0.0f};

font_3d					gFont;

RECT					gD3DRect = {0, 0, 0, 0};
int						gNumObjects = 0;

// Incremented in d3dcache.c to track total DrawPrimitive calls per frame.
int						gNumDPCalls = 0;

static PALETTEENTRY		gPalette[NUM_COLORS];

static unsigned int		gFrame = 0;

// The size of the main full size render buffer and also a smaller buffer for effects.
// The smaller buffer is used for effects that don't need full resolution.
// As per the original specification, the smaller buffer is 1/4 the size of the full buffer.
int						gFullTextureSize = 0;
int						gSmallTextureSize = 0;

int 					d3dRenderTextureThreshold;

// Basic layout: Position and diffuse color.  Used for simple geometry and UI.
IDirect3DVertexDeclaration9* g_pVertexDecl_PosColor;
// Standard layout: Position, color, and one UV. Used for most world textures, sprites, and particles.
IDirect3DVertexDeclaration9* g_pVertexDecl_PosColorTex1;
// Multi-textured layout: Position, color, and two UVs. Used for lightmapped surfaces.
IDirect3DVertexDeclaration9* g_pVertexDecl_PosColorTex2;

int						gD3DRedrawAll = 0;
bool 					gWireframe = false;

// Transformation matrices for the current frame's pipeline.
static D3DMATRIX view, mat, rot, trans, proj;

///////////////////////////
// External Dependencies //
///////////////////////////

// Defined in graphics.c
// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;

// Defined in d3ddriver.c
extern d3d_driver_profile gD3DDriverProfile;

// Defined in palette.c
extern Color base_palette[NUM_COLORS];

extern long				viewer_height;
extern ObjectRange		visible_objects[];    /* Where objects are on screen */
extern int				num_visible_objects;
extern DrawItem			drawdata[];
extern long				nitems;
extern int				sector_depths[];
extern BYTE				*gBits;
extern BYTE				*gBufferBits;
extern ViewElement		ViewElements[];
extern HDC				gBitsDC;

extern void UpdateRoom3D(room_type *room, Draw3DParams *params);

/////////////////////////
// Internal Prototypes //
/////////////////////////
void D3DRenderPacketInit(d3d_render_packet_new *pPacket);
void D3DRenderChunkInit(d3d_render_chunk_new *pChunk);
void D3DRenderViewElementsDraw(d3d_render_pool_new *pPool);

/////////////////////////////
// Internal Implementation //
/////////////////////////////
RECT GetScreenRect()
{
	// RECT struct is set in this order: Left -> Top -> Right -> Bottom
	return { 0, 0, gScreenWidth, gScreenHeight };
}

void D3DRenderFontInit(font_3d *pFont, HFONT hFont)
{
	// Ask for a bigger font to reduce aliasing, then scale the texture
	// down by the same amount.
	static constexpr float FONT_SCALE = 3.0;
	HFONT hScaledFont = FontsGetScaledFont(hFont, FONT_SCALE);
	assert(hScaledFont);

	pFont->fontHeight = GetFontHeight(hScaledFont);
	pFont->texScale = FONT_SCALE;

	static constexpr int LARGE_FONT_THRESHOLD = 40;
	static constexpr int MEDIUM_FONT_THRESHOLD = 20;

	static constexpr int FONT_TEXTURE_SIZE_LARGE = 1024;
	static constexpr int FONT_TEXTURE_SIZE_MEDIUM = 512;
	static constexpr int FONT_TEXTURE_SIZE_SMALL = 256;

	if (pFont->fontHeight > LARGE_FONT_THRESHOLD)
		pFont->texWidth = pFont->texHeight = FONT_TEXTURE_SIZE_LARGE;
	else if (pFont->fontHeight > MEDIUM_FONT_THRESHOLD)
		pFont->texWidth = pFont->texHeight = FONT_TEXTURE_SIZE_MEDIUM;
	else
		pFont->texWidth = pFont->texHeight = FONT_TEXTURE_SIZE_SMALL;

	D3DCAPS9 d3dCaps = {};
	gpD3DDevice->GetDeviceCaps(&d3dCaps);

	if ( pFont->texWidth > static_cast<long>(d3dCaps.MaxTextureWidth) )
	{
		pFont->texScale *= static_cast<float>(pFont->texWidth) / static_cast<float>(d3dCaps.MaxTextureWidth);
		pFont->texHeight = pFont->texWidth = d3dCaps.MaxTextureWidth;
	}

	if (pFont->pTexture)
		pFont->pTexture->Release();

	gpD3DDevice->CreateTexture(pFont->texWidth, pFont->texHeight, 1, 0, D3DFMT_A4R4G4B4,
									D3DPOOL_MANAGED, &pFont->pTexture, nullptr);

	static constexpr int BITMAP_PLANES = 1;
	static constexpr int BITMAP_BIT_DEPTH = 32;

	// Initialize bitmap info. Top-down DIBs (negative height) match D3D's coordinate system.
	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = static_cast<int>(pFont->texWidth);
	bitmapInfo.bmiHeader.biHeight = -static_cast<int>(pFont->texHeight);
	bitmapInfo.bmiHeader.biPlanes = BITMAP_PLANES;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biBitCount = BITMAP_BIT_DEPTH;

	// Handle to Device Context for fonts.
	HDC fontDC = CreateCompatibleDC(gBitsDC);
	DWORD *pBitmapBits = nullptr;
	HBITMAP fontBitmap = CreateDIBSection(fontDC, &bitmapInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBitmapBits), nullptr, 0);
	SetMapMode(fontDC, MM_TEXT);

	SelectObject(fontDC, fontBitmap);
	SelectObject(fontDC, hScaledFont);

	// Set text properties
	SetTextColor(fontDC, RGB(255,255,255));
	SetBkColor(fontDC, 0);
	SetBkMode(fontDC, TRANSPARENT);
	SetTextAlign(fontDC, TA_TOP);

	// Temporary string that holds both a character and a null terminator.
	TCHAR charBuffer[2] = _T("x");

	// Tracks next available pixel position in the texture atlas.
	long atlasX = 0;
	long atlasY = 0;

	// Iterate through printable ASCII characters.
	for (int i = 0; i < NUM_CHARS; i++)
	{
		// Skip the first 32 non-printable characters.
		TCHAR currentChar = static_cast<TCHAR>(i + 32);

		charBuffer[0] = currentChar;

		SIZE size = {};
		GetTextExtentPoint32(fontDC, charBuffer, 1, &size);

		if (!GetCharABCWidths(fontDC, currentChar, currentChar, &pFont->abc[i]))
		{
			// If font isn't TrueType, fallback to basic width (abcB).
			pFont->abc[i] = { 0, static_cast<UINT>(size.cx), 0 };
		}

		// Use the 'B' width (actual character body) for layout.
		size.cx = pFont->abc[i].abcB;

		// Is this row of the texture filled up?
		if (atlasX + size.cx >= pFont->texWidth)
		{
			atlasX = 0;
			atlasY += (size.cy + 1);
		}

		int left_offset = pFont->abc[i].abcA;
		ExtTextOut(fontDC, (atlasX - left_offset), atlasY, 0, nullptr, charBuffer, 1, nullptr);

		pFont->texST[i][0] = {(static_cast<float>(atlasX) / pFont->texWidth), 
									(static_cast<float>(atlasY) / pFont->texHeight)};

		pFont->texST[i][1] = {(static_cast<float>(atlasX + size.cx) / pFont->texWidth),
									(static_cast<float>(atlasY + size.cy) / pFont->texHeight)};

		// Leave +1 space so bilinear filtering doesn't pick up neighboring character
		atlasX += (size.cx + 1);  
	}

	D3DLOCKED_RECT d3dlr = {};
	pFont->pTexture->LockRect(0, &d3dlr, 0, 0);

	BYTE *pDstRow = reinterpret_cast<BYTE*>(d3dlr.pBits);

	// Convert a texture bitmask into a 16-bit pixel format.
	for (int y = 0; y < pFont->texHeight; y++)
	{
		WORD *pDst16 = reinterpret_cast<WORD*>(pDstRow);
		for (int x = 0; x < pFont->texWidth; x++)
		{
			// Extract 4-bit alpha from 8-bit source.
			BYTE bAlpha = static_cast<BYTE>( (pBitmapBits[pFont->texWidth * y + x] & 0xff) >> 4 );

			// If there's any alpha, set color to white with alpha. Otherwise it's transparent.
			*pDst16++ = (bAlpha > 0) ? (static_cast<WORD>(bAlpha << 12) | 0x0FFF) : 0x0000;
		}
		pDstRow += d3dlr.Pitch;
	}

	pFont->pTexture->UnlockRect(0);

	// Get kerning pairs for font
	pFont->numKerningPairs = GetKerningPairs(fontDC, 0, nullptr);
	pFont->kerningPairs = new KERNINGPAIR[pFont->numKerningPairs];
	GetKerningPairs(fontDC, pFont->numKerningPairs, pFont->kerningPairs);

	DeleteObject(fontBitmap);
	DeleteObject(hScaledFont);
	DeleteDC(fontDC);
}

// new render stuff
void D3DRenderPoolInit(d3d_render_pool_new *pPool, int size, int packetSize)
{
	pPool->size = size;
	pPool->curPacket = 0;

	d3d_render_packet_new *pPacket = reinterpret_cast<d3d_render_packet_new*>( malloc(sizeof(d3d_render_packet_new) * size) );
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
	pPool->pMaterialFctn = reinterpret_cast<MaterialFctn>(pMaterialFunc);
}

d3d_render_packet_new *D3DRenderPacketNew(d3d_render_pool_new *pPool)
{
	d3d_render_packet_new *pPacket;

	if (pPool->curPacket >= pPool->size)
	{
		if (pPool->curPacketList->next == nullptr)
		{
			pPacket = reinterpret_cast<d3d_render_packet_new*>( malloc(sizeof(d3d_render_packet_new) * pPool->size) );
			assert(pPacket);
			list_add_item(pPool->renderPacketList, pPacket);
		}
		else
		{
			reinterpret_cast<d3d_render_packet_new*>(pPool->curPacketList->next->data);
		}

		pPool->curPacketList = pPool->curPacketList->next;
		pPool->curPacket = 1;
		pPool->numLists++;

		pPacket = reinterpret_cast<d3d_render_packet_new*>(pPool->curPacketList->data);
	}
	else
	{
		pPacket = reinterpret_cast<d3d_render_packet_new*>(pPool->curPacketList->data);
		pPacket += pPool->curPacket;
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
	pPacket->curChunk++;
	D3DRenderChunkInit(&pPacket->renderChunks[pPacket->curChunk - 1]);
	return &pPacket->renderChunks[pPacket->curChunk - 1];
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

d3d_render_packet_new *D3DRenderPacketFindMatch(d3d_render_pool_new *pPool, IDirect3DTexture9* pTexture,
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
		pChunk->numIndices = TRI_STRIP_INDICES;
		pChunk->numVertices = TRI_STRIP_VERTICES;
		pChunk->numPrimitives = TRI_STRIP_PRIMITIVES;
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
		
		for (int i = 0; i < TRI_STRIP_INDICES; i++)
		{
			pChunk->indices[i] = TRI_STRIP_INDICES_PATTERN[i];
		}
	}
}

// Captures current rendered frame as a texture for post-processing effects.
IDirect3DTexture9* D3DRender_CaptureEffect(IDirect3DTexture9* pTex0, IDirect3DTexture9* pTex1)
{
	D3DCacheSystemReset(&gEffectCacheSystem);
	D3DRenderPoolReset(&gEffectPool, &D3DMaterialEffectPool);

	// get pointer to backbuffer surface and z/stencil surface
	IDirect3DSurface9* pSrc = nullptr;
	IDirect3DSurface9* pZBuf = nullptr;
	gpD3DDevice->GetRenderTarget(0, &pSrc);
	gpD3DDevice->GetDepthStencilSurface(&pZBuf);

	// get pointer to texture surface for rendering
	IDirect3DSurface9* pDest[2]{};
	pTex0->GetSurfaceLevel(0, &pDest[0]);
	pTex1->GetSurfaceLevel(0, &pDest[1]);

	// copy framebuffer to texture
	RECT rect = GetScreenRect();
	gpD3DDevice->StretchRect(pSrc, &rect, pDest[0], &rect, D3DTEXF_NONE);

	// clear local->screen transforms
	D3DMATRIX tempMat;
	MatrixIdentity(&tempMat);
	gpD3DDevice->SetTransform(D3DTS_WORLD, &tempMat);
	gpD3DDevice->SetTransform(D3DTS_VIEW, &tempMat);
	gpD3DDevice->SetTransform(D3DTS_PROJECTION, &tempMat);

	gpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	gpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	gpD3DDevice->SetRenderTarget(0, pDest[1]);

	d3d_render_packet_new *pPacket = D3DRenderPacketNew(&gEffectPool);
	if (pPacket == nullptr)
		return nullptr;

	d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
	pPacket->pMaterialFctn = D3DMaterialEffectPacket;
	pChunk->pMaterialFctn = D3DMaterialEffectChunk;
	pPacket->pTexture = pTex0;

	pChunk->numIndices = TRI_STRIP_INDICES;
	pChunk->numVertices = TRI_STRIP_VERTICES;
	pChunk->numPrimitives = TRI_STRIP_PRIMITIVES;

	MatrixIdentity(&pChunk->xForm);

	pChunk->xyz[0] = { D3DRENDER_SCREEN_TO_CLIP_X(0, gSmallTextureSize),
							0, D3DRENDER_SCREEN_TO_CLIP_Y(0, gSmallTextureSize) };
						
	pChunk->xyz[1] = { D3DRENDER_SCREEN_TO_CLIP_X(0, gSmallTextureSize),
							0, D3DRENDER_SCREEN_TO_CLIP_Y(gSmallTextureSize, gSmallTextureSize) };
					
	pChunk->xyz[2] = { D3DRENDER_SCREEN_TO_CLIP_X(gSmallTextureSize, gSmallTextureSize),
							0, D3DRENDER_SCREEN_TO_CLIP_Y(gSmallTextureSize, gSmallTextureSize) };

	pChunk->xyz[3] = { D3DRENDER_SCREEN_TO_CLIP_X(gSmallTextureSize, gSmallTextureSize),
							0, D3DRENDER_SCREEN_TO_CLIP_Y(0, gSmallTextureSize) };

	const float texSize = static_cast<float>(gFullTextureSize);
	pChunk->st0[0] = { 0.0f, 0.0f };
	pChunk->st0[1] = { 0.0f, (gScreenHeight / texSize) };
	pChunk->st0[2] = { (gScreenWidth / texSize), (gScreenHeight / texSize) };
	pChunk->st0[3] = { (gScreenWidth / texSize), 0.0f };

	for (auto& color : pChunk->bgra)
	{
		color = {COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX};
	}

	for (int i = 0; i < TRI_STRIP_INDICES; i++)
	{
		pChunk->indices[i] = TRI_STRIP_INDICES_PATTERN[i];
	}

	D3DCacheFill(&gEffectCacheSystem, &gEffectPool, 1);
	D3DCacheFlush(&gEffectCacheSystem, &gEffectPool, 1, D3DPT_TRIANGLESTRIP);

	// restore render target to backbuffer
	gpD3DDevice->SetRenderTarget(0, pSrc);
	gpD3DDevice->SetDepthStencilSurface(pZBuf);
	gpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	pSrc->Release();
	pZBuf->Release();
	pDest[0]->Release();
	pDest[1]->Release();

	return pTex1;
}

//////////////////////
// Public Functions //
//////////////////////
int D3DRenderIsEnabled(void)
{
	return gD3DEnabled;
}

void SetZBias(LPDIRECT3DDEVICE9 device, int z_bias) {
   float bias = z_bias * -0.00001f;
   IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS,
                                   *((DWORD *) &bias));
}

int DistanceGet(int x, int y)
{
	int	distance;
	float	xf, yf;

	xf = (float)x;
	yf = (float)y;

	distance = sqrt((double)(xf * xf) + (double)(yf * yf));

	return (int)distance;
}

// Helper function to determine if an object should be rendered in the current pass based on transparency.
bool ShouldRenderInCurrentPass(bool transparent_pass, bool isTransparent)
{
	return transparent_pass == isTransparent;
}

// Define field of views with magic numbers for tuning
float FovHorizontal(long width)
{
	return width / (float)(main_viewport_width) * (-PI / 3.78f);
}

float FovVertical(long height)
{
	return height / (float)(main_viewport_height) * (PI / 5.88f);
}

// Retrieve the threshold value for determining whether to round up the dimensions of a texture.
int getD3dRenderThreshold()
{
	return d3dRenderTextureThreshold;
}

bool isManagedTexturesEnabled()
{
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

bool isWireframeMode()
{
	return gWireframe;
}

const font_3d& getFont3d()
{
	return gFont;
}

const LPDIRECT3DTEXTURE9 getBackBufferTextureZero()
{
	return gpBackBufferTex[0];
}

PALETTEENTRY* getPalette()
{
    return gPalette;
}

const Color(&getBasePalette())[NUM_COLORS]
{
	return base_palette;
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

	if ( (gpD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr )
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
		D3DCacheSystemInit(&gWallMaskCacheSystem, TEX_CACHE_MAX_WALLMASK);
		D3DCacheSystemInit(&gEffectCacheSystem, TEX_CACHE_MAX_EFFECT);
		D3DCacheSystemInit(&gParticleCacheSystem, TEX_CACHE_MAX_PARTICLE );

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

	// Vertex layout for non-textured geometry.
	static constexpr D3DVERTEXELEMENT9 VERTEX_LAYOUT_POS_COLOR[] = {
		{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
		};

	// Vertex layout for standard texturing.
	static constexpr D3DVERTEXELEMENT9 VERTEX_LAYOUT_POS_COLOR_TEX1[] = {
		{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{2, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
		};

	// Vertex layout for multi-texturing (lightmaps).
	static constexpr D3DVERTEXELEMENT9 VERTEX_LAYOUT_POS_COLOR_TEX2[] = {
		{0, 0, D3DDECLTYPE_FLOAT3,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{2, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{3, 0, D3DDECLTYPE_FLOAT2,	 D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		D3DDECL_END()
		};

	gpD3DDevice->CreateVertexDeclaration(VERTEX_LAYOUT_POS_COLOR, &g_pVertexDecl_PosColor);
	gpD3DDevice->CreateVertexDeclaration(VERTEX_LAYOUT_POS_COLOR_TEX1, &g_pVertexDecl_PosColorTex1);
	gpD3DDevice->CreateVertexDeclaration(VERTEX_LAYOUT_POS_COLOR_TEX2, &g_pVertexDecl_PosColorTex2);

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
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGTABLEMODE, mode);
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGSTART, *(DWORD *)(&start));
		IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_FOGEND, *(DWORD *)(&end));
	}

	// create framebuffer textures
	for (int i = 0; i <= 15; i++)
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

		for (i = 0; i < MAX_RENDER_TARGET_POOL; i++)
		{
         IDirect3DTexture9_Release(gpBackBufferTex[i]);
			gpBackBufferTex[i] = nullptr;
		}

		D3DRenderSkyBoxShutdown();

		for (i = 0; i < NUM_VIEW_ELEMENTS; i++)
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

		if (g_pVertexDecl_PosColor) IDirect3DDevice9_Release(g_pVertexDecl_PosColor);
		if (g_pVertexDecl_PosColorTex1) IDirect3DDevice9_Release(g_pVertexDecl_PosColorTex1);
		if (g_pVertexDecl_PosColorTex2) IDirect3DDevice9_Release(g_pVertexDecl_PosColorTex2);

		g_pVertexDecl_PosColor = nullptr;
		g_pVertexDecl_PosColorTex1 = nullptr;
		g_pVertexDecl_PosColorTex2 = nullptr;

		IDirect3DDevice9_Release(gpD3DDevice);
		gpD3DDevice = nullptr;
		IDirect3D9_Release(gpD3D);
		gpD3D = nullptr;
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
	LightCacheUpdateParams lightCacheParams{&gDLightCache, &gDLightCacheDynamic, gD3DRedrawAll};
	D3DLMapsStaticGet(room, lightCacheParams);

	IDirect3DDevice9_Clear(gpD3DDevice, 0, nullptr, D3DCLEAR_TARGET |
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
		SkyboxRenderParams skyboxRenderParams(g_pVertexDecl_PosColorTex1, gD3DDriverProfile, gWorldPool, gWorldCacheSystem);
		D3DRenderSkyBox(params, angleHeading, anglePitch, view, skyboxRenderParams);
	}

	// Prepare our rendering parameters

	WorldCacheSystemParams worldCacheSystemParams(&gWorldCacheSystem, &gWorldCacheSystemStatic,
		&gLMapCacheSystem, &gLMapCacheSystemStatic, &gWallMaskCacheSystem);

	WorldPoolParams worldPoolParams(&gWorldPool, &gWorldPoolStatic, &gLMapPool, &gLMapPoolStatic, &gWallMaskPool);

	WorldRenderParams worldRenderParams(g_pVertexDecl_PosColorTex1, g_pVertexDecl_PosColorTex2, gD3DDriverProfile, worldCacheSystemParams, worldPoolParams,
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
		BackgroundOverlaysRenderStateParams bgoRenderStateParams(g_pVertexDecl_PosColorTex1, gD3DDriverProfile, &gWorldPool, &gWorldCacheSystem,
			view, mat, gD3DRect);
		BackgroundOverlaysSceneParams bgoSceneParams(&num_visible_objects, visible_objects, angleHeading, anglePitch, room, params);
		D3DRenderBackgroundOverlays(bgoRenderStateParams, bgoSceneParams);
	}

	if (draw_world)
	{
		timeWorld = D3DRenderWorld(worldRenderParams, worldPropertyParams, lightAndTextureParams);

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
		ParticleSystemStructure particleSystemStructure(g_pVertexDecl_PosColorTex1, playerDeltaPos, &gParticlePool, &gParticleCacheSystem);
		D3DRenderParticles(particleSystemStructure);
	}

	if (draw_objects)
	{
		ObjectsRenderParams objectsRenderParams(g_pVertexDecl_PosColorTex1, g_pVertexDecl_PosColorTex2, gD3DDriverProfile, &gObjectPool, &gObjectCacheSystem, view, proj, room, params);

		GameObjectDataParams gameObjectDataParams(nitems, &num_visible_objects, &gNumObjects, drawdata, visible_objects,
			gpBackBufferTexFull, gpBackBufferTex);

		FontTextureParams fontTextureParams(&gFont, gSmallTextureSize);

		PlayerViewParams playerViewParams(gScreenWidth, gScreenHeight, main_viewport_width, main_viewport_height, gD3DRect);

		timeObjects = D3DRenderObjects(objectsRenderParams, gameObjectDataParams, lightAndTextureParams, fontTextureParams, playerViewParams);
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
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, g_pVertexDecl_PosColor);

	// Set up orthographic projection for drawing overlays
	MatrixIdentity(&mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &mat);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &mat);

	FxRenderSystemStructure fxRenderSystemStructure(g_pVertexDecl_PosColorTex1, &gObjectPool, &gObjectCacheSystem,
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
	D3DRender_SetColorStage(1, D3DTOP_DISABLE, 0, 0);
	D3DRender_SetAlphaStage(1, D3DTOP_DISABLE, 0, 0);

	D3DRender_SetAlphaTestState(TRUE, TEMP_ALPHA_REF, D3DCMP_GREATEREQUAL);
	D3DRender_SetAlphaBlendState(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice9_SetSamplerState(gpD3DDevice, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, g_pVertexDecl_PosColorTex1);

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

	timeComplete = timeGetTime() - timeComplete;
	timeOverall = timeGetTime() - timeOverall;

	//debug(("overall = %d lightmaps = %d world = %d objects = %d skybox = %d num vertices = %d setup = %d completion = %d (%d, %d, %d)\n"
	//, timeOverall, timeLMaps, timeWorld, timeObjects, timeSkybox, gNumVertices, timeComplete));
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

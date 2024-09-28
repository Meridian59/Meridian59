// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <unordered_map>

// Variables

static const float SKYBOX_DIMENSIONS = 75000.0f;
static const float SKYBOX_Y = 37000.0f;

static LPDIRECT3DTEXTURE9 gpSkyboxTextures[5][6];
static int gCurBackground;
static ID tempBkgnd = 0;

static float gSkyboxXYZ[] =
{
	// back
	SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,

	// bottom
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,

	// front
	-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,

	// left
	-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,

	// right
	SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,

	// top
	-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,
	-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS,
	SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS,
};

static float gSkyboxST[] =
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

static unsigned char gSkyboxBGRA[] =
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


// Interfaces

static void D3DRenderBackgroundsLoad(char* pFilename, int index);
static bool D3DRenderBackgroundSet(ID background);

// Implementations

/**
* Initialize the skybox with the current room
* Return true the background has changed and false otherwise.
* If the background has changed, the caller should trigger a redraw all.
*/
bool D3DRenderSkyBoxBegin(room_type* current_room)
{
	if (gpSkyboxTextures[0][0] == NULL)
	{
		D3DRenderBackgroundsLoad("./resource/skya.bsf", 0);
		D3DRenderBackgroundsLoad("./resource/skyb.bsf", 1);
		D3DRenderBackgroundsLoad("./resource/skyc.bsf", 2);
		D3DRenderBackgroundsLoad("./resource/skyd.bsf", 3);
		D3DRenderBackgroundsLoad("./resource/redsky.bsf", 4);
	}
	if (tempBkgnd != current_room->bkgnd)
	{
		tempBkgnd = current_room->bkgnd;
		return D3DRenderBackgroundSet(tempBkgnd);
	}
	return false;
}

/**
* Function to render the skybox with the current background.
*/
void D3DRenderSkyBox(room_type* room, Draw3DParams* params, room_contents_node* pRNode, 
	int angleHeading, int anglePitch, D3DMATRIX view, const SkyboxRenderParams& skyboxRenderParams)
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
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, skyboxRenderParams.decl1dc);

	// Render the skybox
	D3DRenderPoolReset(&skyboxRenderParams.gWorldPool, &D3DMaterialWorldPool);
	D3DRenderSkyboxDraw(&skyboxRenderParams.gWorldPool, angleHeading, anglePitch);
	D3DCacheFill(&skyboxRenderParams.gWorldCacheSystem, &skyboxRenderParams.gWorldPool, 1);
	D3DCacheFlush(&skyboxRenderParams.gWorldCacheSystem, &skyboxRenderParams.gWorldPool, 1, 
		D3DPT_TRIANGLESTRIP);

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

/**
* Function to draw the individual faces of the skybox.
*/
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch)
{
	int			i, j;
	D3DMATRIX	rot, mat;

	d3d_render_packet_new* pPacket;
	d3d_render_chunk_new* pChunk;

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

/**
* Set the current background texture for the skybox by background ID.
* Returns true if the background was set successfully and false otherwise.
*/
bool D3DRenderBackgroundSet(ID background)
{
	char* filename = LookupRsc(background);

	if (!filename) return false;

	static const std::unordered_map<std::string, int> backgroundMap = {
		{"1skya.bgf", 0},
		{"2skya.bgf", 0},
		{"1skyb.bgf", 1},
		{"2skyb.bgf", 1},
		{"1skyc.bgf", 2},
		{"2skyc.bgf", 2},
		{"1skyd.bgf", 3},
		{"2skyd.bgf", 3},
		{"redsky.bgf", 4}
	};

	auto it = backgroundMap.find(filename);
	if (it != backgroundMap.end())
	{
		gCurBackground = it->second;
	}

	return true;
}

/**
* Shutdown the skybox by releasing all textures.
*/
void D3DRenderSkyBoxShutdown()
{
	for (int j = 0; j < 5; j++)
	{
		for (int i = 0; i < 6; i++)
		{
			if (gpSkyboxTextures[j][i])
			{
				IDirect3DTexture9_Release(gpSkyboxTextures[j][i]);
				gpSkyboxTextures[j][i] = NULL;
			}
		}
	}
}


/**
* Loads a series of PNG images from a specified file and creates textures for the skybox.
*/
void D3DRenderBackgroundsLoad(char* pFilename, int index)
{
	FILE* pFile;
	png_structp	pPng = NULL;
	png_infop	pInfo = NULL;
	png_infop	pInfoEnd = NULL;
	png_bytepp   rows;

	D3DLOCKED_RECT		lockedRect;
	unsigned char* pBits = NULL;
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
		png_bytep curRow;

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

			pBits = (unsigned char*)lockedRect.pBits;

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

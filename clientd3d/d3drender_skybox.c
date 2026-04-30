// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <filesystem>
#include <iterator>

// Variables
static constexpr float SKYBOX_DIMENSIONS = 75000.0f;
static constexpr float SKYBOX_Y = 37000.0f;
static constexpr int SKYBOX_SIDES = 6;

static constexpr int SKYBOX_QUAD_VERTICES = 4;
static constexpr int SKYBOX_QUAD_INDICES = 4;
static constexpr int SKYBOX_QUAD_PRIMITIVES = SKYBOX_QUAD_VERTICES - 2;

static int gCurBackground;
static ID tempBkgnd = 0;

// Defines vertices of the skybox.
static constexpr custom_xyz SKYBOX_XYZ[] =
{
	// Back
	{SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS},
	{-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS},

	// Bottom
	{-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS},
	{SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS},

	// Front
	{-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS},
	{SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS},

	// Left
	{-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS},
	{-SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS},

	// Right
	{SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, -SKYBOX_Y, SKYBOX_DIMENSIONS},
	{SKYBOX_DIMENSIONS, -SKYBOX_Y, -SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS},

	// Top
	{-SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS}, {-SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS},
	{SKYBOX_DIMENSIONS, SKYBOX_Y, SKYBOX_DIMENSIONS}, {SKYBOX_DIMENSIONS, SKYBOX_Y, -SKYBOX_DIMENSIONS}
};

// Defines the four corners of a skybox texture.
static constexpr custom_st SKYBOX_ST[] =
{
	{ 0.001f, 0.001f },		// Top-Left
	{ 0.001f, 0.999f },		// Bottom-Left
	{ 0.999f, 0.999f },		// Bottom-Right
	{ 0.999f, 0.001f }		// Top-Right
};

static constexpr custom_bgra SKYBOX_BGRA = {192, 192, 192, 255};

// Defines which .bgf resource gets paired with its respective hardware rendered skybox.
struct SkyboxDefinition
{
	const char* resourceName;	// The .bgf file used by the software renderer.
	const char* fileName;		// The .bsf/.png file used in the hardware renderer.
};

// Lookup table that pairs software-rendered skyboxes to hardware-rendered skyboxes.
// Note: In hardware rendering, Ko'catan uses the same skybox textures as the mainland.
static constexpr SkyboxDefinition SKYBOX_TABLE[] = 
{
	// Clear skies
    {"1skya.bgf", "skya.bsf"}, 		// Index 0
    {"2skya.bgf", "skya.bsf"}, 		
    {"1skyb.bgf", "skyb.bsf"}, 		// Index 1
    {"2skyb.bgf", "skyb.bsf"}, 		
    {"1skyc.bgf", "skyc.bsf"}, 		// Index 2
	{"2skyc.bgf", "skyc.bsf"}, 
    {"1skyd.bgf", "skyd.bsf"}, 		// Index 3
	{"2skyd.bgf", "skyd.bsf"}, 
	// Frenzy
    {"redsky.bgf", "redsky.bsf"}, 	// Index 4
};

static constexpr int NUM_SKYBOXES = static_cast<int>(std::size(SKYBOX_TABLE));
static IDirect3DTexture9* gpSkyboxTextures[NUM_SKYBOXES][SKYBOX_SIDES];

// Interfaces
static void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch);
static void D3DRenderBackgroundsLoad(const char* pFilename, int index);
static bool D3DRenderBackgroundSet(ID background);


// Implementations

/**
* Update the skybox with the current background.
* Return true the background has changed and false otherwise.
* If the background has changed, the caller should trigger a redraw all.
*/
bool D3DRenderUpdateSkyBox(DWORD background)
{
	if (background == 0)
		return false;

	if (gpSkyboxTextures[0][0] == nullptr)
	{	
		for (int i = 0; i < NUM_SKYBOXES; i++)
		{
			auto fullPath = std::filesystem::path("./resource") / SKYBOX_TABLE[i].fileName;
			
			// Skip any skybox that is missing.
			if (!std::filesystem::exists(fullPath))
				continue;
			
			D3DRenderBackgroundsLoad(fullPath.string().c_str(), i);
		}
	}
	if (tempBkgnd != background)
	{
		tempBkgnd = background;
		return D3DRenderBackgroundSet(tempBkgnd);
	}
	return false;
}

/**
* Function to render the skybox with the current background.
*/
void D3DRenderSkyBox(Draw3DParams* params, int angleHeading, int anglePitch, const D3DMATRIX& view, 
	const SkyboxRenderParams& skyboxRenderParams)
{
	// Set render states for skybox
	gpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	SetZBias(0);
	gpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	gpD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// Disable alpha blending and alpha testing for the skybox
	gpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	gpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	// Disable fog for the skybox
	gpD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);

	// Set texture stages for the skybox
	D3DRender_SetColorStage(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRender_SetAlphaStage(0, D3DTOP_SELECTARG1, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	D3DRender_SetColorStage(1, D3DTOP_DISABLE, 0, 0);
	D3DRender_SetAlphaStage(1, D3DTOP_DISABLE, 0, 0);

	// Set vertex shader and declaration for the skybox
	gpD3DDevice->SetVertexShader(nullptr);
	gpD3DDevice->SetVertexDeclaration(skyboxRenderParams.vertexDeclaration);

	// Render the skybox
	D3DRenderPoolReset(&skyboxRenderParams.renderPool, &D3DMaterialWorldPool);
	D3DRenderSkyboxDraw(&skyboxRenderParams.renderPool, angleHeading, anglePitch);
	D3DCacheFill(&skyboxRenderParams.cacheSystem, &skyboxRenderParams.renderPool, 1);
	D3DCacheFlush(&skyboxRenderParams.cacheSystem, &skyboxRenderParams.renderPool, 1, D3DPT_TRIANGLESTRIP);

	// Restore render states after skybox rendering
	gpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	gpD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	gpD3DDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);

	// restore the correct view matrix
	gpD3DDevice->SetTransform(D3DTS_VIEW, &view);
}

/**
* Function to draw the individual faces of the skybox.
*/
void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch)
{
	gpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	D3DMATRIX rot, mat;
	MatrixIdentity(&mat);
	gpD3DDevice->SetTransform(D3DTS_WORLD, &mat);

	MatrixRotateY(&rot, static_cast<float>(angleHeading) * 360.0f / 4096.0f * PI / 180.0f);
	MatrixRotateX(&mat, static_cast<float>(anglePitch) * 45.0f / 414.0f * PI / 180.0f);
	MatrixMultiply(&mat, &rot, &mat);

	gpD3DDevice->SetTransform(D3DTS_VIEW, &mat);

	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	for (int i = 0; i < SKYBOX_SIDES; i++)
	{
		auto *pPacket = D3DRenderPacketFindMatch(pPool, gpSkyboxTextures[gCurBackground][i], nullptr, 0, 0, 0);
		if (pPacket == nullptr)
			return;

		auto *pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);

		pChunk->numIndices = SKYBOX_QUAD_INDICES;
		pChunk->numVertices = SKYBOX_QUAD_VERTICES;
		pChunk->numPrimitives = SKYBOX_QUAD_PRIMITIVES;
		pPacket->pMaterialFctn = &D3DMaterialWorldPacket;
		pChunk->pMaterialFctn = &D3DMaterialWorldDynamicChunk;
		pChunk->flags |= D3DRENDER_NOAMBIENT;

		// add xyz, st, and bgra data
		for (int j = 0; j < SKYBOX_QUAD_VERTICES; j++)
		{
			// Note that the coordinate system is z-up, so y-z are flipped here.
			pChunk->xyz[j].x = SKYBOX_XYZ[(i * 4) + j].x;
			pChunk->xyz[j].y = SKYBOX_XYZ[(i * 4) + j].z;
			pChunk->xyz[j].z = SKYBOX_XYZ[(i * 4) + j].y;

			pChunk->st0[j] = SKYBOX_ST[j];

			pChunk->bgra[j] = SKYBOX_BGRA;
		}
		
		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

/**
* Set the current background texture for the skybox by background ID.
* Returns true if the background was set successfully and false otherwise.
*/
bool D3DRenderBackgroundSet(ID background)
{
	char* filename = LookupRsc(background);

	if (!filename)
		return false;

	for (int i = 0; i < NUM_SKYBOXES; i++)
	{
        // If the string names match, then set the index.
		if (_stricmp(filename, SKYBOX_TABLE[i].resourceName) == 0)
        {
            gCurBackground = i;
            return true;
        }
	}

	return true;
}

/**
* Shutdown the skybox by releasing all textures.
*/
void D3DRenderSkyBoxShutdown()
{
	for (int j = 0; j < NUM_SKYBOXES; j++)
	{
		for (int i = 0; i < SKYBOX_SIDES; i++)
		{
			if (gpSkyboxTextures[j][i])
			{
				gpSkyboxTextures[j][i]->Release();
				gpSkyboxTextures[j][i] = nullptr;
			}
		}
	}
}

/**
* Loads a series of PNG images from a specified file and creates textures for the skybox.
*/
void D3DRenderBackgroundsLoad(const char* pFilename, int index)
{
	// The .bsf/.png files for skyboxes are actually six .png files stored in a single 
	// file by appending each picture's binary data into one file.
	//
	// The pictures for each cubemap face are stored in this order, similarly in SKYBOX_XYZ[]:
	// Back -> Bottom -> Front -> Left -> Right -> Top
	//
	// Color depth is also 24-bit since the alpha channel (transparency) isn't used.
	
	auto *pFile = fopen(pFilename, "rb");
	if (pFile == nullptr)
		return;

	fpos_t pos = 0;	

	for (int i = 0; i < SKYBOX_SIDES; i++)
	{
		png_structp pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (pPng == nullptr)
		{
			fclose(pFile);
			return;
		}

		png_infop pInfo = png_create_info_struct(pPng);
		if (pInfo == nullptr)
		{
			png_destroy_read_struct(&pPng, nullptr, nullptr);
			fclose(pFile);
			return;
		}

		png_infop pInfoEnd = png_create_info_struct(pPng);
		if (pInfoEnd == nullptr)
		{
			png_destroy_read_struct(&pPng, &pInfo, nullptr);
			fclose(pFile);
			return;
		}

		if (setjmp(png_jmpbuf(pPng)))
		{
			png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
			fclose(pFile);
			return;
		}

		fseek(pFile, pos, SEEK_SET);

		png_init_io(pPng, pFile);
		png_read_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, nullptr);
		png_bytepp rows = png_get_rows(pPng, pInfo);

		uint32_t image_width = png_get_image_width(pPng, pInfo);
		uint32_t image_height = png_get_image_height(pPng, pInfo);

		gpD3DDevice->CreateTexture(image_width, image_height, 1, 0,
			D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &gpSkyboxTextures[index][i], nullptr);

		D3DLOCKED_RECT lockedRect = {};
		gpSkyboxTextures[index][i]->LockRect(0, &lockedRect, nullptr, 0);

		auto *pBits = reinterpret_cast<uint8_t*>(lockedRect.pBits);

		for (uint32_t h = 0; h < image_height; h++)
		{
			png_bytep curRow = rows[h];

			for (uint32_t w = 0; w < image_width; w++)
			{
				for (uint32_t b = 0; b < 4; b++)
				{
					if (b == 3)
						pBits[h * lockedRect.Pitch + w * 4 + b] = 255;
					else
						// PNGs are in RGB, while DirectX wants BGRA.
						pBits[h * lockedRect.Pitch + w * 4 + (2 - b)] = curRow[(w * 3) + b];
				}
			}
		}

		gpSkyboxTextures[index][i]->UnlockRect(0);

		fgetpos(pFile, &pos);
		png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
	}

	fclose(pFile);
}

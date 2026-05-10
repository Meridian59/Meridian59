// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include "skybox_load.h"
#include <filesystem>
#include <iterator>

///////////////
// Constants //
///////////////

// Geometry constants for rendering a quad as a triangular strip.
static constexpr int SKYBOX_QUAD_VERTICES = 4;
static constexpr int SKYBOX_QUAD_INDICES = 4;
static constexpr int SKYBOX_QUAD_PRIMITIVES = SKYBOX_QUAD_VERTICES - 2;
static constexpr int SKYBOX_QUAD_INDICES_PATTERN[] = {1, 2, 0, 3};

static constexpr custom_bgra SKYBOX_BGRA = {192, 192, 192, 255};

static constexpr float SKYBOX_DIMENSIONS = 75000.0f;
static constexpr float SKYBOX_Y = 37000.0f;

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

/////////////////////
// Local Variables //
/////////////////////
static int gCurBackground;
static ID tempBkgnd = 0;
static IDirect3DTexture9* gpSkyboxTextures[NUM_SKYBOXES][SKYBOX_SIDES];

////////////////////////
// Internal Functions //
////////////////////////

/**
* Set the current background texture for the skybox by background ID.
* Returns true if the background was set successfully and false otherwise.
*/
static bool D3DRenderBackgroundSet(ID background)
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

	debug(("Skybox Error: '%s' is not mapped in SKYBOX_TABLE.\n", filename));
	return false;
}

/**
* Function to draw the individual faces of the skybox.
*/
static void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch)
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

		for (int i = 0; i < SKYBOX_QUAD_INDICES; i++)
		{
			pChunk->indices[i] = SKYBOX_QUAD_INDICES_PATTERN[i];
		}
	}

	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

// Checks if a skybox is already set up, and copies the pointers over if a match is found..
// Used for hardware rendered skyboxes that is shared with both the mainland and Ko'catan island.
static bool CheckExistingSkyboxTextures(int currentIndex)
{
    for (int i = 0; i < currentIndex; i++)
    {
        if (_stricmp(SKYBOX_TABLE[currentIndex].fileName, SKYBOX_TABLE[i].fileName) == 0)
        {
            // Copy the pointers from the existing skybox over to this one.
            memcpy(gpSkyboxTextures[currentIndex], gpSkyboxTextures[i], sizeof(gpSkyboxTextures[currentIndex]));
            return true;
        }
    }
    return false;	
}

//////////////////////
// Public Functions //
//////////////////////

/*
* Update the skybox with the current background.
* Return true the background has changed and false otherwise.
* If the background has changed, the caller should trigger a redraw all.
*/
bool D3DRenderUpdateSkyBox(DWORD background)
{
	// Skip if there's no background yet or if it's already set.
	if (background == 0 || tempBkgnd == background)
		return false;

	static bool bSkyboxesInitialized = false;
	if (!bSkyboxesInitialized)
	{
		for (int i = 0; i < NUM_SKYBOXES; i++)
		{
			// If the skybox was already set up, just copy the pointers over and skip.
			if (CheckExistingSkyboxTextures(i))
				continue;
			
			// Otherwise, load the PNG as normal.
			auto fullPath = std::filesystem::path("./resource") / SKYBOX_TABLE[i].fileName;
			if (std::filesystem::exists(fullPath))
			{
				LoadSkyboxFaces(fullPath.string().c_str(), gpSkyboxTextures[i]);
			}
			else
			{
				debug(("Skybox Error: File not found at %s\n", fullPath.string().c_str()));
			}
		}
		bSkyboxesInitialized = true;
	}

	tempBkgnd = background;
	return D3DRenderBackgroundSet(tempBkgnd);
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

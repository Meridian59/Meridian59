// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include "skybox_load.h"
#include <unordered_map>
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

// Lookup table that pairs software-rendered skyboxes to hardware-rendered skyboxes.
static const std::unordered_map<std::string, std::string> SKYBOX_ASSET_MAPPING =
{
	// Clear skies
	{"1skya.bgf", "skya.bsf"},
	{"2skya.bgf", "skya.bsf"},
	{"1skyb.bgf", "skyb.bsf"},
	{"2skyb.bgf", "skyb.bsf"},
	{"1skyc.bgf", "skyc.bsf"},
	{"2skyc.bgf", "skyc.bsf"},
	{"1skyd.bgf", "skyd.bsf"},
	{"2skyd.bgf", "skyd.bsf"},
	// Frenzy
	{"redsky.bgf", "redsky.bsf"},
};

/////////////////////
// Local Variables //
/////////////////////
static ID gCachedBackgroundID = 0;
// Dynamic texture cache that maps filenames of hardware-rendered skyboxes to their loaded texture pointers.
static std::unordered_map<std::string, std::vector<IDirect3DTexture9*>> gSkyboxCache;
// Holds the six texture pointers for the skybox actively being rendered on screen this frame.
static std::vector<IDirect3DTexture9*> gpActiveSkyboxTextures;

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

	// Check if the .bgf file is in the lookup table.
	auto assetIt = SKYBOX_ASSET_MAPPING.find(filename);
	if (assetIt == SKYBOX_ASSET_MAPPING.end())
	{
		debug(("Skybox Error: '%s' is not mapped in SKYBOX_ASSET_MAPPING.\n", filename));
		return false;
	}
	// If found, get the filename of the hardware-rendered counterpart.
	const std::string& hwFilename = assetIt->second;

	// Now check if we can reuse skybox textures that are already cached.
	auto cacheIt = gSkyboxCache.find(hwFilename);
	if (cacheIt != gSkyboxCache.end())
	{
		gpActiveSkyboxTextures = cacheIt->second;
		return true;
	}

	// Otherwise, check if the file exists before attempting to load the PNG.
	auto fullPath = std::filesystem::path("./resource") / hwFilename;
	if (!std::filesystem::exists(fullPath))
	{
		debug(("Skybox Error: File missing at %s\n", fullPath.string().c_str()));
		return false;
	}

	// Allocate space for the six dynamic texture pointers.
	std::vector<IDirect3DTexture9*> newTextures(SKYBOX_SIDES, nullptr);

	// Load the PNG file data directly into the pre-allocated data.
	LoadSkyboxFaces(fullPath.string().c_str(), newTextures.data());

	// Save into the dynamic cache map and mark as active for rendering.
	gSkyboxCache[hwFilename] = newTextures;
	gpActiveSkyboxTextures = newTextures;

	return true;
}

/**
* Function to draw the individual faces of the skybox.
*/
static void D3DRenderSkyboxDraw(d3d_render_pool_new* pPool, int angleHeading, int anglePitch)
{
	if (gpActiveSkyboxTextures.empty())
		return;
	
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

	for (size_t i = 0; i < SKYBOX_SIDES; i++)
	{
		// Ensure the pointer slot is valid before fetching.
		if (i >= gpActiveSkyboxTextures.size() || gpActiveSkyboxTextures[i] == nullptr)
			continue;
		
		auto *pPacket = D3DRenderPacketFindMatch(pPool, gpActiveSkyboxTextures[i], nullptr, 0, 0, 0);
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

		for (int k = 0; k < SKYBOX_QUAD_INDICES; k++)
		{
			pChunk->indices[k] = SKYBOX_QUAD_INDICES_PATTERN[k];
		}
	}

	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	gpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
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
	if (background == 0 || gCachedBackgroundID == background)
		return false;

	gCachedBackgroundID = background;
	return D3DRenderBackgroundSet(gCachedBackgroundID);
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
	// Release each skybox texture that was stored in the skybox cache.
	for (auto& [hwFilename, textures] : gSkyboxCache)
	{
		for (auto* pTexture : textures)
		{
			if (pTexture != nullptr)
			{
				pTexture->Release();
			}
		}
	}
	gSkyboxCache.clear();
	gpActiveSkyboxTextures.clear();
	gCachedBackgroundID = 0;
}

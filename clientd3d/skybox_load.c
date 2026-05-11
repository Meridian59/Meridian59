// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <filesystem>
#include <iterator>

static bool LoadPngChunkToTexture(FILE* pFile, IDirect3DTexture9*& pTexture)
{
	png_structp pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	png_infop pInfo = png_create_info_struct(pPng);

	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_read_struct(&pPng, &pInfo, nullptr);
		return false;
	}

	png_init_io(pPng, pFile);
	png_read_info(pPng, pInfo);

	// Remove alpha in case a 32-bit PNG is used. Then fill in the 4th byte with 0xFF (opaque).
	png_set_strip_alpha(pPng);
	png_set_filler(pPng, 0xFF, PNG_FILLER_AFTER);

	png_set_bgr(pPng); // Swap red and blue to match D3D's BGRA format.
	png_read_update_info(pPng, pInfo);

	uint32_t image_width = png_get_image_width(pPng, pInfo);
	uint32_t image_height = png_get_image_height(pPng, pInfo);

	// Creates the D3D texture in a 32-bit BGRA format, even if the source PNG is 24-bit.
	HRESULT hrTex = gpD3DDevice->CreateTexture(image_width, image_height, 1, 0,
						D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, nullptr);

    if (FAILED(hrTex))
	{
		debug(("Skybox Error: Failed to create %dx%d texture (HRESULT: 0x%08X). Possible VRAM limit.\n",
				image_width, image_height, hrTex));
		png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

	D3DLOCKED_RECT lockedRect = {};
	HRESULT hrLock = pTexture->LockRect(0, &lockedRect, nullptr, 0);
	if (FAILED(hrLock))
	{
		debug(("Skybox Error: LockRect failed (HRESULT: 0x%08X). Skybox face will be empty.\n", hrLock));
		png_destroy_read_struct(&pPng, &pInfo, nullptr);
		return false;
	}

	// Read directly into the texture buffer, row-by-row.
	for (uint32_t h = 0; h < image_height; h++)
	{
		uint8_t* pRowDest = static_cast<uint8_t*>(lockedRect.pBits) + (h * lockedRect.Pitch);
		png_read_row(pPng, pRowDest, nullptr);
	}
	pTexture->UnlockRect(0);

	png_read_end(pPng, pInfo);
	png_destroy_read_struct(&pPng, &pInfo, nullptr);
	return true;
}

/**
* Loads a series of PNG images from a specified file and creates textures for the skybox.
*/
void LoadSkyboxFaces(const char* pFilePath, IDirect3DTexture9** ppSkyboxFace)
{
	// The .bsf/.png files for skyboxes are actually six PNGs stored in a single
	// file by appending each picture's binary data into one file.
	//
	// The pictures for each cubemap face are stored in this order, similarly in SKYBOX_XYZ[]:
	// Back -> Bottom -> Front -> Left -> Right -> Top
	//
	// The pixel-packing loop expects a 24-bit PNG file since the alpha channel isn't used.
	// However, a 32-bit PNG can still be used even if it's larger in file size.

	// The calling function should verify that the file exists.
	auto *pFile = fopen(pFilePath, "rb");

	if (pFile == nullptr)
	{
		// If 'fopen' fails here, the file exists and is verified by the caller, but cannot be opened.
		// It's possible that it's locked by another process, or has restricted permissions.
		debug(("Skybox Error: File is inaccessible at: %s\n", pFilePath));
		return;
	}

	for (int i = 0; i < SKYBOX_SIDES; i++)
	{
		if(!LoadPngChunkToTexture(pFile, ppSkyboxFace[i]))
		{
            debug(("Skybox Error: Failed to load face %d in %s\n", i, pFilePath));
            break;
		}
	}

	fclose(pFile);
}

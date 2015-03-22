// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DDRIVER_H__
#define __D3DDRIVER_H__

enum lmap_modes
{
	LMAP_MODE_NONE = 0,
	LMAP_MODE_STATIC_ONLY,
	LMAP_MODE_DYNAMIC_ONLY,
	LMAP_MODE_ALL,
	LMAP_MODE_END,
};

enum texture_modes
{
	TEXTURE_MODE_NONE = 0,
	TEXTURE_MODE_POINT,
	TEXTURE_MODE_BILINEAR,
	TEXTURE_MODE_ANISOTROPIC,
	TEXTURE_MODE_END,
};

typedef struct d3d_driver_profile
{
	D3DCAPS9				d3dCaps;
	D3DADAPTER_IDENTIFIER9	adapterID;
	unsigned int	texMemTotal;
	unsigned int  texMemWorldStatic;
  unsigned int  texMemWorldDynamic;
  unsigned int  texMemLMapStatic;
  unsigned int  texMemLMapDynamic;
  unsigned int  texMemObjects;
	DWORD					minFilter;
	DWORD					magFilter;
	DWORD					vertexProcessFlag;
	DWORD					maxAnisotropy;
	DWORD					vertexProcessingMode;
	DWORD					lmapMode;
	Bool					bZBias;
	Bool					bCull;
	Bool					bManagedTextures;
	Bool					bSoftwareRenderer;
	Bool					bFogEnable;
} d3d_driver_profile;

Bool D3DDriverProfileInit(void);

#endif

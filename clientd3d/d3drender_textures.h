// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render textures code to create from resource (png, bgf - regular or swizzled).
//
#ifndef _D3DRENDERTEXTURES_H
#define _D3DRENDERTEXTURES_H

// ORed into a render packet's effect field to ask for the solid variant of a bitmap, in
// which palette index 254 is uploaded as its own colour instead of as a cutout hole.
// Sits outside OF_EFFECT_MASK so it can never be mistaken for an object drawing effect.
static const int D3DRENDER_TEXTURE_SOLID = 0x10000000;

LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromResource(BYTE* ptr, int width, int height);
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
	unsigned int effect);
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
	unsigned int effect);

#endif	/* #ifndef _D3DRENDERTEXTURES_H */

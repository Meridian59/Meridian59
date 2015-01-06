// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DTYPES_H__
#define __D3DTYPES_H__

#define PETER_FUDGE			(16.0f)
#define MAX_DISTANCE		(4 * FINENESS * FINENESS)
#define COLOR_AMBIENT		239
#define COLOR_MAX			255
#define TEMP_LIGHT_SCALE	(32 * 150)
// XXX
//#define TEMP_ALPHA_REF		(127)
#define TEMP_ALPHA_REF		(1)

#define LIGHTMAP_B			(165)
#define LIGHTMAP_G			(240)
#define LIGHTMAP_R			(255)
#define LIGHTMAP_A			(255)
#define TEMP_LIGHT_AMB		(255)

#define D3DRENDER_TRANS25	(64)
#define D3DRENDER_TRANS50	(128)
#define D3DRENDER_TRANS75	(192)

// chunk or packet-wide flags
#define D3DRENDER_WALL_NORMAL	(0x01000000)
#define D3DRENDER_WALL_ABOVE	(0x02000000)
#define D3DRENDER_WALL_BELOW	(0x04000000)
#define D3DRENDER_NO_VTILE		(0x08000000)
#define D3DRENDER_TRANSPARENT	(0x10000000)
#define D3DRENDER_DIRTY			(0x20000000)
#define D3DRENDER_NOCULL		(0x40000000)
#define D3DRENDER_NOAMBIENT		(0x80000000)

// cache system-wide flags
#define D3DRENDER_WIREFRAME		(0x01)

#define	D3DRENDER_MAXTEXTURES	(1024)
#define	D3DRENDER_MAXPACKETS	(1024)
#define	D3DRENDER_MAXINDICES	(10000)

#define D3DRENDER_TEXTURE_THRESHOLD	(256)
#define	D3DRENDER_LIGHT_DISTANCE	(FINENESS)

#define MAX_DLIGHTS				(256)

// font crap
// Font creation flags
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002
#define D3DFONT_ZENABLE     0x0004

// Font rendering flags
#define D3DFONT_CENTERED    0x0001
#define D3DFONT_TWOSIDED    0x0002
#define D3DFONT_FILTERED    0x0004

#define D3DRENDER_CLIP_TO_SCREEN_X(_x, _scale)	(((_x) * (_scale) / 2.0f) + ((_scale) / 2.0f))
#define D3DRENDER_CLIP_TO_SCREEN_Y(_y, _scale)	(((_y) * -(_scale) / 2.0f) + ((_scale) / 2.0f))

#define D3DRENDER_SCREEN_TO_CLIP_X(_x, _scale)	(((_x) - (_scale) / 2.0f) / ((_scale) / 2.0f))
#define D3DRENDER_SCREEN_TO_CLIP_Y(_y, _scale)	(((_y) - (_scale) / 2.0f) / (-(_scale) / 2.0f))

typedef struct custom_xyz
{
	float	x, y, z;
} custom_xyz;

typedef struct custom_xyzw
{
	float	x, y, z, w;
} custom_xyzw;

typedef struct custom_st
{
	float	s, t;
} custom_st;

typedef struct custom_bgra
{
	unsigned char	b, g, r, a;
} custom_bgra;

typedef short custom_index;

typedef struct custom_vbuffer
{
	u_int					curIndex;
	LPDIRECT3DVERTEXBUFFER9	pVBuffer;
	union
	{
		custom_xyz		*pXYZ;
		custom_st		*pST;
		custom_bgra		*pBGRA;
	} u;
} custom_vbuffer;

typedef struct custom_ibuffer
{
	u_int					curIndex;
	LPDIRECT3DINDEXBUFFER9	pIBuffer;
	custom_index			*pIndex;
} custom_ibuffer;

typedef struct render_packet
{
	D3DMATRIX			xForm;
	u_int				flags;
	LPDIRECT3DTEXTURE9	pTexture;
	PDIB				pDib;
	int					numStages;
	u_int				startIndex;
	u_int				numIndices;
	BOOL				drawn;
	BYTE				xLat0;
	BYTE				xLat1;
} render_packet;

extern HRESULT			gD3DError;

#endif // __D3DTYPES_H__

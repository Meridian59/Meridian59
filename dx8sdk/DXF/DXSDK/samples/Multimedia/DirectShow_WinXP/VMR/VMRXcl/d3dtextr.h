//-----------------------------------------------------------------------------
// File: D3DTextr.h
//
// Desc: Functions to manage textures, including creating (loading from a
//       file), restoring lost surfaces, invalidating, and destroying.
//
//       Note: the implementation of these functions maintains an internal list
//       of loaded textures. After creation, individual textures are referenced
//       via their ASCII names.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#ifndef D3DTEXTR_H
#define D3DTEXTR_H
#include <ddraw.h>
#include <d3d.h>


//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE7 D3DTextr_GetSurface( TCHAR* strName );


//-----------------------------------------------------------------------------
// Texture invalidation and restoration functions
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Invalidate( TCHAR* strName );
HRESULT D3DTextr_Restore( TCHAR* strName, LPDIRECT3DDEVICE7 pd3dDevice );
HRESULT D3DTextr_InvalidateAllTextures();
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE7 pd3dDevice );


//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------
#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002
#define D3DTEXTR_32BITSPERPIXEL   0x00000004
#define D3DTEXTR_16BITSPERPIXEL   0x00000008
#define D3DTEXTR_CREATEWITHALPHA  0x00000010


HRESULT D3DTextr_CreateTextureFromFile( TCHAR* strName, DWORD dwStage=0L,
                                        DWORD dwFlags=0L );
HRESULT D3DTextr_CreateEmptyTexture( TCHAR* strName, DWORD dwWidth,
                                     DWORD dwHeight, DWORD dwStage,
                                     DWORD dwFlags );
HRESULT D3DTextr_DestroyTexture( TCHAR* strName );
VOID    D3DTextr_SetTexturePath( TCHAR* strTexturePath );

//-----------------------------------------------------------------------------
// Name: TextureContainer
// Desc: Linked list structure to hold info per texture
//-----------------------------------------------------------------------------
struct TextureContainer
{
    TextureContainer* m_pNext;   // Linked list ptr

    TCHAR   m_strName[MAX_PATH]; // Name of texture (doubles as image filename)
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    DWORD   m_dwStage;           // Texture stage (for multitexture devices)
    DWORD   m_dwBPP;
    DWORD   m_dwFlags;
    BOOL    m_bHasAlpha;
    UINT    m_nRes;

    LPDIRECTDRAWSURFACE7 m_pddsSurface; // Surface of the texture
    HBITMAP m_hbmBitmap;         // Bitmap containing texture image
    DWORD*  m_pRGBAData;

public:
    HRESULT LoadImageData();
    HRESULT LoadBitmapFile( TCHAR* strPathname );
    HRESULT Restore( LPDIRECT3DDEVICE7 pd3dDevice );
    HRESULT CopyBitmapToSurface();
    HRESULT CopyRGBADataToSurface();

    TextureContainer( TCHAR* strName, DWORD dwStage, DWORD dwFlags, UINT nRes = NULL );
    ~TextureContainer();
};



#endif // D3DTEXTR_H

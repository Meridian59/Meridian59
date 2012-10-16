//-----------------------------------------------------------------------------
// File: D3DFont.h
//
// Desc: Texture-based font class
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3DFONT_H
#define D3DFONT_H
#include <tchar.h>
#include <ddraw.h>
#include <d3dxmath.h>
#define D3D_OVERLOADS
#include <d3d.h>


// Font creation flags
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002

// Font rendering flags
#define D3DFONT_CENTERED    0x0001
#define D3DFONT_TWOSIDED    0x0002
#define D3DFONT_FILTERED    0x0004


#ifndef __INITDDSTRUCT_DEFINED
#define __INITDDSTRUCT_DEFINED
template <typename T>
__inline void INITDDSTRUCT(T& dd)
{
    ZeroMemory(&dd, sizeof(dd));
    dd.dwSize = sizeof(dd);
}
#endif

#ifndef __RELEASE_DEFINED
#define __RELEASE_DEFINED
template<typename T>
__inline void RELEASE( T* &p )
{
    if( p ) {
        p->Release();
        p = NULL;
    }
}
#endif


#define MAX_NUM_VERTICES 50*6

//-----------------------------------------------------------------------------
// Custom vertex types for rendering text
//-----------------------------------------------------------------------------
struct FONT2DVERTEX {
    D3DXVECTOR4 p;
    DWORD color;
    FLOAT tu, tv;
};


//-----------------------------------------------------------------------------
// Name: class CD3DFont
// Desc: Texture-based font class for doing text in a 3D scene.
//-----------------------------------------------------------------------------
class CD3DFont
{
    TCHAR   m_strFontName[80];            // Font properties
    DWORD   m_dwFontHeight;
    DWORD   m_dwFontFlags;

    LPDIRECT3DDEVICE7       m_pd3dDevice; // A D3DDevice used for rendering
    LPDIRECTDRAWSURFACE7    m_pTexture;   // The d3d texture for this font
    FONT2DVERTEX            m_VB[MAX_NUM_VERTICES];  // VertexBuffer for rendering text

    DWORD   m_dwTexWidth;                 // Texture dimensions
    DWORD   m_dwTexHeight;
    FLOAT   m_fTextScale;
    FLOAT   m_fTexCoords[128-32][4];

    // Stateblocks for setting and restoring render states
    DWORD   m_dwSavedStateBlock;
    DWORD   m_dwDrawTextStateBlock;

public:
    // 2D and 3D text drawing functions
    HRESULT DrawText( FLOAT x, FLOAT y, DWORD dwColor,
                      TCHAR* strText, DWORD dwFlags=0L );

    // Function to get extent of text
    HRESULT GetTextExtent( TCHAR* strText, SIZE* pSize );

    // Initializing and destroying device-dependent objects
    HRESULT InitDeviceObjects(LPDIRECTDRAW7 pDD, LPDIRECT3DDEVICE7 pd3dDevice);
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();

    // Constructor / destructor
    CD3DFont( TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags=0L );
    ~CD3DFont();
};

#endif



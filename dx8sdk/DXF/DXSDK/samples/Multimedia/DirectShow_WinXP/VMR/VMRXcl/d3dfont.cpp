//-----------------------------------------------------------------------------
// File: D3DFont.cpp
//
// Desc: Texture-based font class
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <streams.h>
#include <stdio.h>
#include <tchar.h>

#include <ddraw.h>
#define D3D_OVERLOADS
#include <d3d.h>
#include "D3DFont.h"


#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

inline FONT2DVERTEX InitFont2DVertex( const D3DXVECTOR4& p, D3DCOLOR color,
                                      FLOAT tu, FLOAT tv )
{
    FONT2DVERTEX v;

    v.p.x = p.x;
    v.p.y = p.y;
    v.p.z = p.z;
    v.p.w = p.w;
    v.color = color;
    v.tu = tu;
    v.tv = tv;

    return v;
}

//-----------------------------------------------------------------------------
// Name: CD3DFont()
// Desc: Font class constructor
//-----------------------------------------------------------------------------
CD3DFont::CD3DFont( TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags )
{
    _tcscpy( m_strFontName, strFontName );
    m_dwFontHeight         = dwHeight;
    m_dwFontFlags          = dwFlags;

    m_pd3dDevice           = NULL;
    m_pTexture             = NULL;
    ZeroMemory(&m_VB, sizeof(m_VB));

    m_dwSavedStateBlock    = 0L;
    m_dwDrawTextStateBlock = 0L;
}

//-----------------------------------------------------------------------------
// Name: ~CD3DFont()
// Desc: Font class destructor
//-----------------------------------------------------------------------------
CD3DFont::~CD3DFont()
{
    InvalidateDeviceObjects();
    DeleteDeviceObjects();
}

//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initializes device-dependent objects, including the vertex buffer used
//       for rendering text and the texture map which stores the font image.
//-----------------------------------------------------------------------------
HRESULT CD3DFont::InitDeviceObjects(
    LPDIRECTDRAW7 pDD,
    LPDIRECT3DDEVICE7 pd3dDevice
    )
{
    HRESULT hr;

    // Keep a local copy of the device
    m_pd3dDevice = pd3dDevice;

    // Establish the font and texture size
    m_fTextScale  = 1.0f; // Draw fonts into texture without scaling

    // Large fonts need larger textures
    if( m_dwFontHeight > 40 )
        m_dwTexWidth = m_dwTexHeight = 1024;
    else if( m_dwFontHeight > 20 )
        m_dwTexWidth = m_dwTexHeight = 512;
    else
        m_dwTexWidth  = m_dwTexHeight = 256;

    // If requested texture is too big, use a smaller texture and smaller font,
    // and scale up when rendering.
    D3DDEVICEDESC7 d3dCaps;
    m_pd3dDevice->GetCaps( &d3dCaps );

    if( m_dwTexWidth > d3dCaps.dwMaxTextureWidth )
    {
        m_fTextScale = (FLOAT)d3dCaps.dwMaxTextureWidth / (FLOAT)m_dwTexWidth;
        m_dwTexWidth = m_dwTexHeight = d3dCaps.dwMaxTextureWidth;
    }

    DDSURFACEDESC2 ddsd;
    INITDDSTRUCT(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTSTATIC;

    ddsd.dwWidth  = m_dwTexWidth;
    ddsd.dwHeight = m_dwTexHeight;
    ddsd.ddpfPixelFormat.dwFourCC = BI_RGB;
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xF000;
    ddsd.ddpfPixelFormat.dwRBitMask = 0x0F00;
    ddsd.ddpfPixelFormat.dwGBitMask = 0x00F0;
    ddsd.ddpfPixelFormat.dwBBitMask = 0x000F;

    // Create a new texture for the font
    hr = pDD->CreateSurface(&ddsd, &m_pTexture, NULL);

    if( FAILED(hr) )
        return hr;

    // Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)m_dwTexWidth;
    bmi.bmiHeader.biHeight      = -(int)m_dwTexHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // Create a DC and a bitmap for the font
    HDC     hDC       = CreateCompatibleDC( NULL );
    HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );
    SetMapMode( hDC, MM_TEXT );

    // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
    // antialiased font, but this is not guaranteed.
    INT nHeight    = -MulDiv( m_dwFontHeight,
        (INT)(GetDeviceCaps(hDC, LOGPIXELSY) * m_fTextScale), 72 );
    DWORD dwBold   = (m_dwFontFlags&D3DFONT_BOLD)   ? FW_BOLD : FW_NORMAL;
    DWORD dwItalic = (m_dwFontFlags&D3DFONT_ITALIC) ? TRUE    : FALSE;
    HFONT hFont    = CreateFont( nHeight, 0, 0, 0, dwBold, dwItalic,
                          FALSE, FALSE, DEFAULT_CHARSET, OUT_STRING_PRECIS,
                          CLIP_STROKE_PRECIS, ANTIALIASED_QUALITY,
                          VARIABLE_PITCH, m_strFontName );
    if( NULL==hFont )
        return E_FAIL;

    SelectObject( hDC, hbmBitmap );
    SelectObject( hDC, hFont );

    // Set text properties
    SetTextColor( hDC, RGB(255,255,255) );
    SetBkColor(   hDC, 0x00000000 );
    SetTextAlign( hDC, TA_TOP );

    // Loop through all printable character and output them to the bitmap..
    // Meanwhile, keep track of the corresponding tex coords for each character.
    DWORD x = 0;
    DWORD y = 0;
    TCHAR str[2] = _T("x");
    SIZE size;

    for( TCHAR c=32; c<127; c++ )
    {
        str[0] = c;
        GetTextExtentPoint32( hDC, str, 1, &size );

        if( (DWORD)(x+size.cx+1) > m_dwTexWidth )
        {
            x  = 0;
            y += size.cy+1;
        }

        ExtTextOut( hDC, x+0, y+0, ETO_OPAQUE, NULL, str, 1, NULL );

        m_fTexCoords[c-32][0] = ((FLOAT)(x+0))/m_dwTexWidth;
        m_fTexCoords[c-32][1] = ((FLOAT)(y+0))/m_dwTexHeight;
        m_fTexCoords[c-32][2] = ((FLOAT)(x+0+size.cx))/m_dwTexWidth;
        m_fTexCoords[c-32][3] = ((FLOAT)(y+0+size.cy))/m_dwTexHeight;

        x += size.cx+1;
    }

    // Lock the surface and write the alpha values for the set pixels
    m_pTexture->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);

    WORD* pDst16 = (WORD*)ddsd.lpSurface;
    BYTE bAlpha; // 4-bit measure of pixel intensity

    for( y=0; y < m_dwTexHeight; y++ )
    {
        for( x=0; x < m_dwTexWidth; x++ )
        {
            bAlpha = (BYTE)((pBitmapBits[m_dwTexWidth*y + x] & 0xff) >> 4);
            if (bAlpha > 0)
            {
                *pDst16++ = (WORD) ((bAlpha << 12) | 0x0fff);
            }
            else
            {
                *pDst16++ = 0x0000;
            }
        }
    }

    // Done updating texture, so clean up used objects
    m_pTexture->Unlock(NULL);
    DeleteObject( hbmBitmap );
    DeleteDC( hDC );
    DeleteObject( hFont );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DFont::RestoreDeviceObjects()
{
    // Create vertex buffer for the letters

    ZeroMemory(&m_VB, sizeof(m_VB));

    // Create the state blocks for rendering text
    m_pd3dDevice->SetTexture(0, m_pTexture);

    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x08);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,  D3DCMP_GREATEREQUAL);

    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);


    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);

    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

    m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT CD3DFont::InvalidateDeviceObjects()
{
    ZeroMemory(&m_VB, sizeof(m_VB));

    // Delete the state blocks
    if( m_pd3dDevice )
    {
        if (m_dwSavedStateBlock)
            m_pd3dDevice->DeleteStateBlock(m_dwSavedStateBlock);

        if (m_dwDrawTextStateBlock)
            m_pd3dDevice->DeleteStateBlock(m_dwDrawTextStateBlock);
    }

    m_dwSavedStateBlock    = 0L;
    m_dwDrawTextStateBlock = 0L;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DeleteDeviceObjects()
{
    RELEASE(m_pTexture);
    m_pd3dDevice = NULL;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: Get the dimensions of a text string
//-----------------------------------------------------------------------------
HRESULT CD3DFont::GetTextExtent( TCHAR* strText, SIZE* pSize )
{
    if( NULL==strText || NULL==pSize )
        return E_FAIL;

    FLOAT fRowWidth  = 0.0f;
    FLOAT fRowHeight = (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight;
    FLOAT fWidth     = 0.0f;
    FLOAT fHeight    = fRowHeight;

    while( *strText )
    {
        TCHAR c = *strText++;

        if( c == _T('\n') )
        {
            fRowWidth = 0.0f;
            fHeight  += fRowHeight;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT tx2 = m_fTexCoords[c-32][2];

        fRowWidth += (tx2-tx1)*m_dwTexWidth;

        if( fRowWidth > fWidth )
            fWidth = fRowWidth;
    }

    pSize->cx = (int)fWidth;
    pSize->cy = (int)fHeight;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws 2D text
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DrawText( FLOAT sx, FLOAT sy, DWORD dwColor,
                            TCHAR* strText, DWORD dwFlags )
{
    if( m_pd3dDevice == NULL )
        return E_FAIL;

    // Setup renderstate
    RestoreDeviceObjects();

    // Set filter states
    if( dwFlags & D3DFONT_FILTERED )
    {
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
    }

    FLOAT fStartX = sx;

    // Fill vertex buffer
    FONT2DVERTEX* pVertices = NULL;
    DWORD         dwNumTriangles = 0;
    pVertices = &m_VB[0];

    while( *strText )
    {
        TCHAR c = *strText++;

        if( c == _T('\n') )
        {
            sx = fStartX;
            sy += (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT ty1 = m_fTexCoords[c-32][1];
        FLOAT tx2 = m_fTexCoords[c-32][2];
        FLOAT ty2 = m_fTexCoords[c-32][3];

        FLOAT w = (tx2-tx1) * m_dwTexWidth  / m_fTextScale;
        FLOAT h = (ty2-ty1) * m_dwTexHeight / m_fTextScale;

        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,0.5f,2.0f), dwColor, tx1, ty2 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.5f,2.0f), dwColor, tx2, ty2 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,0.5f,2.0f), dwColor, tx1, ty1 );
        *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+0-0.5f,0.5f,2.0f), dwColor, tx2, ty1 );

        dwNumTriangles += 2;

        m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                                    &m_VB[0], dwNumTriangles*2, D3DDP_WAIT);
        pVertices = &m_VB[0];
        dwNumTriangles = 0L;

        sx += w;
    }
    return S_OK;
}


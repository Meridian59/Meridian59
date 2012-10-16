//-----------------------------------------------------------------------------
// File: mtexture.cpp
//
// Desc: Example code showing how to do multitexturing in D3D
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include "stdafx.h"
#include <tchar.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "MFCTex.h"




//-----------------------------------------------------------------------------
// Define a custom vertex that uses multiple sets of tex coords
//-----------------------------------------------------------------------------
struct WALLVERTEX
{
    D3DXVECTOR3 p;
    DWORD dwColor;
    FLOAT tu1, tv1;
    FLOAT tu2, tv2;
    FLOAT tu3, tv3;
};

#define D3DFVF_WALLVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3 )

#define FILL_WALLVERTEX( v, ax, ay, az, acolor, atu1, atv1, atu2, atv2, atu3, atv3 )  \
{   v.p.x = ax; v.p.y = ay; v.p.z = az; v.dwColor  = acolor; \
    v.tu1 = atu1; v.tv1 = atv1; v.tu2 = atu2; v.tv2 = atv2;\
    v.tu3 = atu3; v.tv3 = atv3;\
}

#define WALL_VERT_NUM 10


struct FLOORCEILINGVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT tu, tv;
};

#define D3DFVF_FLOORCEILINGVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )

#define FILL_FLOORCEILINGVERTEX( v, ax, ay, az, nx, ny, nz, atu, atv )  \
{   v.p.x = ax; v.p.y = ay; v.p.z = az; v.n.x = nx; v.n.y = ny; v.n.z = nz; \
    v.tu = atu; v.tv = atv; \
}

#define FLOORCEILING_VERT_NUM 12



//-----------------------------------------------------------------------------
// Storage of the texture stage states
//-----------------------------------------------------------------------------
WORD  g_wT0COp,   g_wT1COp,   g_wT2COp;
WORD  g_wT0CArg1, g_wT1CArg1, g_wT2CArg1;
WORD  g_wT0CArg2, g_wT1CArg2, g_wT2CArg2;
WORD  g_wT0AOp,   g_wT1AOp,   g_wT2AOp;
WORD  g_wT0AArg1, g_wT1AArg1, g_wT2AArg1;
WORD  g_wT0AArg2, g_wT1AArg2, g_wT2AArg2;
DWORD g_dwTextureFactor         = 0xffffffff;
DWORD g_dwDiffuseColor          = 0xffffffff;
DWORD g_dwMaxTextureBlendStages = 0;





//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 g_pFloorTexture = NULL;
LPDIRECT3DTEXTURE8 g_pTexture0     = NULL;
LPDIRECT3DTEXTURE8 g_pTexture1     = NULL;
LPDIRECT3DTEXTURE8 g_pTexture2     = NULL;

// Filenames for the textures (one for each texture stage)
BOOL  g_bTexturesChanged = FALSE;
TCHAR g_strTexture0[256];
TCHAR g_strTexture1[256];
TCHAR g_strTexture2[256];




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CAppForm::OneTimeSceneInit()
{
    m_pFont = new CD3DFont( _T("Arial"), 9, D3DFONT_BOLD );

    // Create some textures
    _tcscpy( g_strTexture0, _T("env2.bmp") );
    _tcscpy( g_strTexture1, _T("spotlite.bmp") );
    _tcscpy( g_strTexture2, _T("env3.bmp") );

    g_pFloorTexture = NULL;
    g_pTexture0     = NULL;
    g_pTexture1     = NULL;
    g_pTexture2     = NULL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FrameMove()
{
    // Setup the world spin matrix
    D3DXMATRIX matWorldSpin;
    D3DXMatrixRotationY( &matWorldSpin, -m_fTime/9 );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorldSpin );

    // keep texture coordinates in a reasonable range
    static FLOAT fTimeKeySub = 0.0f;
    FLOAT fTexTimeKey = m_fTime + fTimeKeySub;
    if( fTexTimeKey > 160.0f )
    {
        fTimeKeySub -= 160.0f;
    }

    WALLVERTEX* pVertices;
    m_pVBWalls->Lock( 0, 0, (BYTE**)&pVertices, 0 );
    // Rotate the light map around the walls each frame
    for( int i=0; i<(WALL_VERT_NUM/2); i++ )
    {
        pVertices[2*i+0].tu2 = fTexTimeKey/(WALL_VERT_NUM/2)+i;
        pVertices[2*i+1].tu2 = fTexTimeKey/(WALL_VERT_NUM/2)+i;
    }
    for( i=0; i<WALL_VERT_NUM; i++ )
        pVertices[i].dwColor = g_dwDiffuseColor;
    m_pVBWalls->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetTextureStageStatesForRendering()
// Desc: Sets up the texture stage, as per the global variables defining each
//       stage.
//-----------------------------------------------------------------------------
HRESULT SetTextureStageStatesForRendering( LPDIRECT3DDEVICE8 pd3dDevice )
{
    // If new textures were selected, restore them now
    if( g_bTexturesChanged )
    {
        SAFE_RELEASE( g_pFloorTexture );
        SAFE_RELEASE( g_pTexture0 );
        SAFE_RELEASE( g_pTexture1 );
        SAFE_RELEASE( g_pTexture2 );
        if( FAILED( D3DUtil_CreateTexture( pd3dDevice, _T("floor.bmp"), &g_pFloorTexture ) ) )
            g_pFloorTexture = NULL;
        if( FAILED( D3DUtil_CreateTexture( pd3dDevice, g_strTexture0, &g_pTexture0 ) ) )
            g_pTexture0 = NULL;
        if( FAILED( D3DUtil_CreateTexture( pd3dDevice, g_strTexture1, &g_pTexture1 ) ) )
            g_pTexture1 = NULL;
        if( FAILED( D3DUtil_CreateTexture( pd3dDevice, g_strTexture2, &g_pTexture2 ) ) )
            g_pTexture2 = NULL;
        g_bTexturesChanged = FALSE;
    }

    pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, g_dwTextureFactor );

    pd3dDevice->SetTexture( 0, g_pTexture0 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, g_wT0CArg1 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   g_wT0COp   );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, g_wT0CArg2 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, g_wT0AArg1 );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   g_wT0AOp   );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, g_wT0AArg2 );

    if( g_dwMaxTextureBlendStages > 1 )
    {
        pd3dDevice->SetTexture( 1, g_pTexture1 );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, g_wT1CArg1 );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   g_wT1COp   );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, g_wT1CArg2 );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, g_wT1AArg1 );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   g_wT1AOp   );
        pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, g_wT1AArg2 );
    }

    if( g_dwMaxTextureBlendStages > 2)
    {
        pd3dDevice->SetTexture( 2, g_pTexture2 );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0 );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, g_wT2CArg1 );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   g_wT2COp   );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, g_wT2CArg2 );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG1, g_wT2AArg1 );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   g_wT2AOp   );
        pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG2, g_wT2AArg2 );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::Render()
{
    HRESULT hr;

    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                       0x000000ff, 1.0f, 0L );

    // Begin the scene
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return E_FAIL;

    // Render the floor and ceiling (using single-textured vertices)
    m_pd3dDevice->SetTexture( 0, g_pFloorTexture );
    m_pd3dDevice->SetTexture( 1, NULL );
    m_pd3dDevice->SetTexture( 2, NULL );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetVertexShader( D3DFVF_FLOORCEILINGVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pVBFloorCeiling, sizeof(FLOORCEILINGVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 4 );

    // Setup the texture stages' state
    SetTextureStageStatesForRendering( m_pd3dDevice );

    // Validate the device. This checks to see if the texture stage states we
    // set up are valid for the device. If so, render the object.
    DWORD dwNumPasses;
    if( SUCCEEDED( hr = m_pd3dDevice->ValidateDevice( &dwNumPasses ) ) )
    {
        // Render the multi-textured object
        m_pd3dDevice->SetVertexShader( D3DFVF_WALLVERTEX );
        m_pd3dDevice->SetStreamSource( 0, m_pVBWalls, sizeof(WALLVERTEX) );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 8);
    }

    TCHAR* pstr;
    switch( hr )
    {
    case D3DERR_UNSUPPORTEDCOLOROPERATION:
        pstr = _T("Unsupported color op");
        break;
    case D3DERR_UNSUPPORTEDCOLORARG:
        pstr = _T("Unsupported color arg");
        break;
    case D3DERR_UNSUPPORTEDALPHAOPERATION:
        pstr = _T("Unsupported alpha op");
        break;
    case D3DERR_UNSUPPORTEDALPHAARG:
        pstr = _T("Unsupported alpha arg");
        break;
    case D3DERR_TOOMANYOPERATIONS:
        pstr = _T("Too many texture ops");
        break;
    case D3DERR_WRONGTEXTUREFORMAT:
        pstr = _T("Incompatible texture formats");
        break;
    case D3DERR_CONFLICTINGRENDERSTATE:
        pstr = _T("Conflicting render state");
        break;
    case S_OK:
        pstr = _T("Device validated OK");
        break;
    default:
        pstr = _T("Using DX5 driver");
        break;
    }
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), pstr );

    // End the scene.
    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InitDeviceObjects()
{
    UpdateUIForDeviceCapabilites();

    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Create the walls vertex buffer
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( WALL_VERT_NUM * sizeof(WALLVERTEX),
        0, D3DFVF_WALLVERTEX, D3DPOOL_MANAGED, &m_pVBWalls ) ) )
    {
        return E_FAIL;
    }

    WALLVERTEX* pVertices;
    m_pVBWalls->Lock( 0, 0, (BYTE**)&pVertices, 0 );

    FILL_WALLVERTEX( pVertices[ 0], -5.0f,-5.0f, 5.0f, g_dwDiffuseColor, 0.00f, 1.0f, 1.00, 1.0f, 1.00, 1.0f );
    FILL_WALLVERTEX( pVertices[ 1], -5.0f, 5.0f, 5.0f, g_dwDiffuseColor, 0.00f, 0.0f, 1.00, 0.0f, 1.00, 0.0f );
    FILL_WALLVERTEX( pVertices[ 2],  5.0f,-5.0f, 5.0f, g_dwDiffuseColor, 1.00f, 1.0f, 0.00, 1.0f, 0.00, 1.0f );
    FILL_WALLVERTEX( pVertices[ 3],  5.0f, 5.0f, 5.0f, g_dwDiffuseColor, 1.00f, 0.0f, 0.00, 0.0f, 0.00, 0.0f );
    FILL_WALLVERTEX( pVertices[ 4],  5.0f,-5.0f,-5.0f, g_dwDiffuseColor, 2.00f, 1.0f, 1.00, 1.0f, 1.00, 1.0f );
    FILL_WALLVERTEX( pVertices[ 5],  5.0f, 5.0f,-5.0f, g_dwDiffuseColor, 2.00f, 0.0f, 1.00, 0.0f, 1.00, 0.0f );
    FILL_WALLVERTEX( pVertices[ 6], -5.0f,-5.0f,-5.0f, g_dwDiffuseColor, 3.00f, 1.0f, 1.00, 1.0f, 1.00, 1.0f );
    FILL_WALLVERTEX( pVertices[ 7], -5.0f, 5.0f,-5.0f, g_dwDiffuseColor, 3.00f, 0.0f, 1.00, 0.0f, 1.00, 0.0f );
    FILL_WALLVERTEX( pVertices[ 8], -5.0f,-5.0f, 5.0f, g_dwDiffuseColor, 4.00f, 1.0f, 0.00, 1.0f, 0.00, 1.0f );
    FILL_WALLVERTEX( pVertices[ 9], -5.0f, 5.0f, 5.0f, g_dwDiffuseColor, 4.00f, 0.0f, 0.00, 0.0f, 0.00, 0.0f );

    m_pVBWalls->Unlock();

    // Create the floor/ceiling vertex buffer
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( FLOORCEILING_VERT_NUM * sizeof(FLOORCEILINGVERTEX),
        0, D3DFVF_FLOORCEILINGVERTEX, D3DPOOL_MANAGED, &m_pVBFloorCeiling ) ) )
    {
        return E_FAIL;
    }

    FLOORCEILINGVERTEX* pFloorVertices;
    m_pVBFloorCeiling->Lock( 0, 0, (BYTE**)&pFloorVertices, 0 );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[0],  -5,-5, 5, 0, 1, 0, 0.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[1],   5,-5, 5, 0, 1, 0, 0.0f, 1.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[2],   5,-5,-5, 0, 1, 0, 1.0f, 1.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[3],   5,-5,-5, 0, 1, 0, 1.0f, 1.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[4],  -5,-5,-5, 0, 1, 0, 1.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[5],  -5,-5, 5, 0, 1, 0, 0.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[6],   5, 5,-5, 0,-1, 0, 1.0f, 1.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[7],   5, 5, 5, 0,-1, 0, 0.0f, 1.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[8],  -5, 5, 5, 0,-1, 0, 0.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[9],  -5, 5, 5, 0,-1, 0, 0.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[10], -5, 5,-5, 0,-1, 0, 1.0f, 0.0f );
    FILL_FLOORCEILINGVERTEX( pFloorVertices[11],  5, 5,-5, 0,-1, 0, 1.0f, 1.0f );
    m_pVBFloorCeiling->Unlock();

    g_bTexturesChanged = TRUE; // Force texture reload

    g_dwMaxTextureBlendStages = m_d3dCaps.MaxTextureBlendStages;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Create and set up the shine materials w/ textures
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set the transform matrices
    D3DXMATRIX matWorld, matView, matProj;
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -4.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/2, 1.0f, 1.0f, 1000.0f );

    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set any appropiate state
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::DeleteDeviceObjects()
{
    m_pFont->DeleteDeviceObjects();
    SAFE_RELEASE( m_pVBWalls );
    SAFE_RELEASE( m_pVBFloorCeiling );
    SAFE_RELEASE( g_pFloorTexture );
    SAFE_RELEASE( g_pTexture0 );
    SAFE_RELEASE( g_pTexture1 );
    SAFE_RELEASE( g_pTexture2 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FinalCleanup()
{
    SAFE_DELETE( m_pFont );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CAppForm::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, 
                                 D3DFORMAT Format )
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetTextureMaps()
// Desc: Changes the texture maps used during rendering of the room.
//-----------------------------------------------------------------------------
VOID CAppForm::SetTextureMaps( const TCHAR* strTexture0, const TCHAR* strTexture1,
                         const TCHAR* strTexture2 )
{
    _tcscpy( g_strTexture0, strTexture0 );
    _tcscpy( g_strTexture1, strTexture1 );
    _tcscpy( g_strTexture2, strTexture2 );

    g_bTexturesChanged = TRUE;
}





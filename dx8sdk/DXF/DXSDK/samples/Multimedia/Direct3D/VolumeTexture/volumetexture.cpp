//-----------------------------------------------------------------------------
// File: VolumeTexture.cpp
//
// Desc: Example code showing how to do volume textures in D3D.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <stdio.h>
#include <math.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
struct VOLUMEVERTEX
{
    FLOAT      x, y, z;
    DWORD      color;
    FLOAT      tu, tv, tw;
};

#define D3DFVF_VOLUMEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0))

VOLUMEVERTEX g_vVertices[4] =
{
    { 1.0f, 1.0f, 0.0f, 0xffffffff, 1.0f, 1.0f, 0.0f },
    {-1.0f, 1.0f, 0.0f, 0xffffffff, 0.0f, 1.0f, 0.0f },
    { 1.0f,-1.0f, 0.0f, 0xffffffff, 1.0f, 0.0f, 0.0f },
    {-1.0f,-1.0f, 0.0f, 0xffffffff, 0.0f, 0.0f, 0.0f }
};




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont* m_pFont;
    LPDIRECT3DVOLUMETEXTURE8 m_pVolumeTexture;
    LPDIRECT3DVERTEXBUFFER8  m_pVB;

    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT FinalCleanup();
    HRESULT Render();
    HRESULT FrameMove();

public:
    CMyD3DApplication();
};




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;

    return d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("VolumeTexture");
    m_bUseDepthBuffer   = TRUE;

    m_pFont          = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pVolumeTexture = NULL;
    m_pVB            = NULL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    static FLOAT fAngle = 0.0f;
    fAngle += 0.1f;

    // Play with the volume texture coordinate
    VOLUMEVERTEX* pVertices = NULL;
    m_pVB->Lock( 0, 4*sizeof(VOLUMEVERTEX), (BYTE**)&pVertices, 0 );
    for( int i=0; i<4; i++ )
        pVertices[i].tw = sinf(fAngle)*sinf(fAngle);
    m_pVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x00000000, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Draw the quad, with the volume texture
        m_pd3dDevice->SetTexture( 0, m_pVolumeTexture );
        m_pd3dDevice->SetVertexShader( D3DFVF_VOLUMEVERTEX );
        m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(VOLUMEVERTEX) );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);

        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    HRESULT hr;

    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Create a 64x64x4 volume texture
    hr = m_pd3dDevice->CreateVolumeTexture( 16, 16, 16, 1, 0,
                                            D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
                                            &m_pVolumeTexture );
    if( FAILED(hr) )
        return hr;

    // Fill the volume texture
    {
        D3DLOCKED_BOX LockedBox;
        hr = m_pVolumeTexture->LockBox( 0, &LockedBox, 0, 0 );
        if( FAILED(hr) )
            return hr;

        for( UINT w=0; w<16; w++ )
        {
            BYTE* pSliceStart = (BYTE*)LockedBox.pBits;

            for( UINT v=0; v<16; v++ )
            {
                for( UINT u=0; u<16; u++ )
                {
                    FLOAT du = (u-7.5f)/7.5f;
                    FLOAT dv = (v-7.5f)/7.5f;
                    FLOAT dw = (w-7.5f)/7.5f;
                    FLOAT fScale = sqrtf( du*du + dv*dv + dw*dw ) / sqrtf(1.0f);

                    if( fScale > 1.0f ) fScale = 0.0f;
                    else                fScale = 1.0f - fScale;

                                        DWORD r = (DWORD)((w<<4)*fScale);
                                        DWORD g = (DWORD)((v<<4)*fScale);
                                        DWORD b = (DWORD)((u<<4)*fScale);

                    ((DWORD*)LockedBox.pBits)[u] = 0xff000000 + (r<<16) + (g<<8) + (b);
                }
                LockedBox.pBits = (BYTE*)LockedBox.pBits + LockedBox.RowPitch;
            }
            LockedBox.pBits = pSliceStart + LockedBox.SlicePitch;
        }

        m_pVolumeTexture->UnlockBox( 0 );
    }

    // Create a vertex buffer
    {
        if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( 4*sizeof(VOLUMEVERTEX),
                                           D3DUSAGE_WRITEONLY,
                                           D3DFVF_VOLUMEVERTEX,
                                           D3DPOOL_MANAGED, &m_pVB ) ) )
            return hr;

        VOLUMEVERTEX* pVertices;
        m_pVB->Lock( 0, 4*sizeof(VOLUMEVERTEX), (BYTE**)&pVertices, 0 );
        memcpy( pVertices, g_vVertices, sizeof(VOLUMEVERTEX)*4 );
        m_pVB->Unlock();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Set the matrices
    D3DXVECTOR3 vEye( 0.0f, 0.0f,-3.0f );
    D3DXVECTOR3 vAt(  0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUp(  0.0f, 1.0f, 0.0f );
    D3DXMATRIX matWorld, matView, matProj;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEye,&vAt, &vUp );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set state
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPING,     FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPING,     FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pFont->DeleteDeviceObjects();
    SAFE_RELEASE( m_pVolumeTexture );
    SAFE_RELEASE( m_pVB );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE( m_pFont );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( 0 == ( pCaps->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP ) )
        return E_FAIL;

    return S_OK;
}





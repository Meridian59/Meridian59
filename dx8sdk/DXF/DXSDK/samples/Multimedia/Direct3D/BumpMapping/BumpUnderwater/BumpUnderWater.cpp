//-----------------------------------------------------------------------------
// File: Underwater.cpp
//
// Desc: Code to simulate underwater distortion.
//       Games could easily make use of this technique to achieve an underwater
//       effect without affecting geometry by rendering the scene to a texture
//       and applying the bump effect on a rectangle mapped with it.
//
// Note: From the Matrox web site demos
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include "D3DX8.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
inline FLOAT rnd() { return (((FLOAT)rand()-(FLOAT)rand())/(2L*RAND_MAX)); }
inline FLOAT RND() { return (((FLOAT)rand())/RAND_MAX); }
inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

struct BUMPVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu1, tv1;
    FLOAT       tu2, tv2;
};

#define D3DFVF_BUMPVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*               m_pFont;
    LPDIRECT3DVERTEXBUFFER8 m_pWaterVB;
    LPDIRECT3DTEXTURE8      m_pBumpMap;
    LPDIRECT3DTEXTURE8      m_pBackgroundTexture;
    BOOL                    m_bDeviceValidationFailed;

    HRESULT CreateBumpMap();
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();

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
    m_strWindowTitle     = _T("BumpUnderWater: Effect Using BumpMapping");
    m_bUseDepthBuffer    = TRUE;

    m_pFont              = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pWaterVB           = NULL;
    m_pBumpMap           = NULL;
    m_pBackgroundTexture = NULL;
    m_bDeviceValidationFailed = FALSE;
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
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT00, F2DW(0.01f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT01, F2DW(0.00f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT10, F2DW(0.00f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT11, F2DW(0.01f) );

    BUMPVERTEX* vWaterVertices;
    m_pWaterVB->Lock( 0, 0, (BYTE**)&vWaterVertices, 0 );
    vWaterVertices[0].tu1 =  0.000f; vWaterVertices[0].tv1 = 0.5f*m_fTime + 2.0f;
    vWaterVertices[1].tu1 =  0.000f; vWaterVertices[1].tv1 = 0.5f*m_fTime;
    vWaterVertices[2].tu1 =  1.000f; vWaterVertices[2].tv1 = 0.5f*m_fTime;
    vWaterVertices[3].tu1 =  1.000f; vWaterVertices[3].tv1 = 0.5f*m_fTime + 2.0f;
    m_pWaterVB->Unlock();

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
    // Clear the scene
    m_pd3dDevice->Clear( 0, NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0xff000000, 1.0f, 0L );

    // Begin the scene
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK;

    // Render the waves
    m_pd3dDevice->SetTexture( 0, m_pBumpMap );
    m_pd3dDevice->SetTexture( 1, m_pBackgroundTexture );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BUMPENVMAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetVertexShader( D3DFVF_BUMPVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pWaterVB, sizeof(BUMPVERTEX) );

    // Verify that the texture operations are possible on the device
    DWORD dwNumPasses;
    if( FAILED( m_pd3dDevice->ValidateDevice( &dwNumPasses ) ) )
    {
        // The right thing to do when device validation fails is to try
        // a different rendering technique.  This sample just warns the user.
        m_bDeviceValidationFailed = TRUE;
    }

    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

    if( m_bDeviceValidationFailed )
    {
        m_pFont->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,0,0), 
            _T("Warning: Device validation failed.  Rendering may not look right.") );
    }

    // End the scene.
    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateBumpMapFromSurface()
// Desc: Creates a bumpmap from a surface
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::CreateBumpMap()
{
    UINT iWidth  = 256;
    UINT iHeight = 256;

    // Create the bumpmap's surface and texture objects
    if( FAILED( m_pd3dDevice->CreateTexture( iWidth, iHeight, 1, 0, 
        D3DFMT_V8U8, D3DPOOL_MANAGED, &m_pBumpMap ) ) )
    {
        return E_FAIL;
    }

    // Fill the bumpmap texels to simulate a lens
    D3DLOCKED_RECT lrDst;
    m_pBumpMap->LockRect( 0, &lrDst, 0, 0 );
    DWORD dwDstPitch = (DWORD)lrDst.Pitch;
    BYTE* pDst       = (BYTE*)lrDst.pBits;

    for( DWORD y=0; y<iHeight; y++ )
    {
        for( DWORD x=0; x<iWidth; x++ )
        {
            FLOAT fx = x/(FLOAT)iWidth  - 0.5f;
            FLOAT fy = y/(FLOAT)iHeight - 0.5f;
            FLOAT r  = sqrtf( fx*fx + fy*fy );

            CHAR iDu = (CHAR)(64*cosf(4.0f*(fx+fy)*D3DX_PI));
            CHAR iDv = (CHAR)(64*sinf(4.0f*(fx+fy)*D3DX_PI));

            pDst[2*x+0] = iDu;
            pDst[2*x+1] = iDv;
        }
        pDst += lrDst.Pitch;
    }

    m_pBumpMap->UnlockRect(0);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("LobbyXPos.bmp"),
                                       &m_pBackgroundTexture, D3DFMT_A8R8G8B8 ) ) )
        return E_FAIL;

    // create a bumpmap from info in source surface
    if( FAILED( CreateBumpMap() ) )
        return E_FAIL;

    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(BUMPVERTEX),
                                                  D3DUSAGE_WRITEONLY,
                                                  D3DFVF_BUMPVERTEX,
                                                  D3DPOOL_MANAGED, &m_pWaterVB ) ) )
        return E_FAIL;

    BUMPVERTEX* v;
    m_pWaterVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p = D3DXVECTOR3(-60.0f,-60.0f, 0.0f ); v[0].n = D3DXVECTOR3( 0, 1, 0 );
    v[1].p = D3DXVECTOR3(-60.0f, 60.0f, 0.0f ); v[1].n = D3DXVECTOR3( 0, 1, 0 );
    v[2].p = D3DXVECTOR3( 60.0f,-60.0f, 0.0f ); v[2].n = D3DXVECTOR3( 0, 1, 0 );
    v[3].p = D3DXVECTOR3( 60.0f, 60.0f, 0.0f ); v[3].n = D3DXVECTOR3( 0, 1, 0 );
    v[0].tu2 = 0.000f; v[0].tv2 = 1.0f;
    v[1].tu2 = 0.000f; v[1].tv2 = 0.0f;
    v[2].tu2 = 1.000f; v[2].tv2 = 1.0f;
    v[3].tu2 = 1.000f; v[3].tv2 = 0.0f;
    m_pWaterVB->Unlock();

    m_bDeviceValidationFailed = FALSE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Set the transform matrices
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -100.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,    0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,    0.0f );
    D3DXMATRIX matWorld, matView, matProj;

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    D3DXMatrixPerspectiveFovLH( &matProj, 1.00f, 1.0f, 1.0f, 3000.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set any appropiate state
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

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
    SAFE_RELEASE( m_pWaterVB );
    SAFE_RELEASE( m_pBumpMap );
    SAFE_RELEASE( m_pBackgroundTexture );

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
    // Device must be able to do bumpmapping
    if( 0 == ( pCaps->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP ) )
        return E_FAIL;

    // Accept devices that can create D3DFMT_V8U8 textures
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal,
                                              pCaps->DeviceType, Format,
                                              0, D3DRTYPE_TEXTURE,
                                              D3DFMT_V8U8 ) ) )
        return S_OK;

    return E_FAIL;
}





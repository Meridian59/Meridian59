//-----------------------------------------------------------------------------
// File: BumpLens.cpp
//
// Desc: Code to simulate a magnifying glass using bumpmapping.
//
// Note: Based on a sample from the Matrox web site
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include "D3DX8.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

struct BUMPVERTEX          // Vertex type used for bumpmap lens effect
{
    D3DXVECTOR3 p;
    FLOAT       tu1, tv1;
    FLOAT       tu2, tv2;
};

struct BACKGROUNDVERTEX    // Vertex type used for rendering background
{
    D3DXVECTOR4 p;
    DWORD       color;
    FLOAT       tu, tv;
};

#define D3DFVF_BUMPVERTEX       (D3DFVF_XYZ|D3DFVF_TEX2)
#define D3DFVF_BACKGROUNDVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*               m_pFont;

    LPDIRECT3DVERTEXBUFFER8 m_pBackgroundVB;
    LPDIRECT3DVERTEXBUFFER8 m_pLensVB;

    FLOAT                   m_fLensX;
    FLOAT                   m_fLensY;

    LPDIRECT3DTEXTURE8      m_pBumpMapTexture;
    LPDIRECT3DTEXTURE8      m_pBackgroundTexture;

    BOOL                    m_bDeviceValidationFailed;

    HRESULT CreateBumpMap( UINT iWidth, UINT iHeight );
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
    m_strWindowTitle     = _T("BumpLens: Lens Effect Using BumpMapping");
    m_bUseDepthBuffer    = FALSE;

    m_pFont              = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );

    m_pBumpMapTexture    = NULL;
    m_pBackgroundTexture = NULL;

    m_pBackgroundVB      = NULL;
    m_pLensVB            = NULL;
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
    // Get a triangle wave between -1 and 1
    m_fLensX = 2 * fabsf( 2 * ( (m_fTime/2) - floorf(m_fTime/2) ) - 1 ) - 1;

    // Get a regulated sine wave between -1 and 1
    m_fLensY = 2 * fabsf( sinf( m_fTime ) ) - 1;

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
    // Begin the scene
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK;

    // Render the background
    m_pd3dDevice->SetTexture( 0, m_pBackgroundTexture );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetVertexShader( D3DFVF_BACKGROUNDVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pBackgroundVB, sizeof(BACKGROUNDVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Render the lens
    m_pd3dDevice->SetTexture( 0, m_pBumpMapTexture );
    m_pd3dDevice->SetTexture( 1, m_pBackgroundTexture );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BUMPENVMAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT00,   F2DW(0.2f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT01,   F2DW(0.0f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT10,   F2DW(0.0f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT11,   F2DW(0.2f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVLSCALE,  F2DW(1.0f) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f) );

    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // Generate texture coords depending on objects camera space position
    D3DXMATRIX mat;
    mat._11 = 0.5f; mat._12 = 0.0f;
    mat._21 = 0.0f; mat._22 =-0.5f;
    mat._31 = 0.0f; mat._32 = 0.0f;
    mat._41 = 0.5f; mat._42 = 0.5f;

    // Scale-by-z here
    D3DXMATRIX matView, matProj;
    m_pd3dDevice->GetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
    D3DXVECTOR3 vEyePt( matView._41, matView._42, matView._43 );
    FLOAT       z = D3DXVec3Length( &vEyePt );
    mat._11 *= ( matProj._11 / ( matProj._33 * z + matProj._34 ) );
    mat._22 *= ( matProj._22 / ( matProj._33 * z + matProj._34 ) );

    m_pd3dDevice->SetTransform( D3DTS_TEXTURE1, &mat );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1);

    // Position the lens
    D3DXMATRIX matWorld;
    D3DXMatrixTranslation( &matWorld, 0.7f * (1000.0f-256.0f)*m_fLensX,
                                      0.7f * (1000.0f-256.0f)*m_fLensY,
                                      0.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    m_pd3dDevice->SetVertexShader( D3DFVF_BUMPVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pLensVB, sizeof(BUMPVERTEX) );

    // Verify that the texture operations are possible on the device
    DWORD dwNumPasses;
    if( FAILED( m_pd3dDevice->ValidateDevice( &dwNumPasses ) ) )
    {
        // The right thing to do when device validation fails is to try
        // a different rendering technique.  This sample just warns the user.
        m_bDeviceValidationFailed = TRUE;
    }

    // Render the lens
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
// Name: CreateBumpmap()
// Desc: Create a bump map texture and fill its content to BUMPDUDV format
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::CreateBumpMap( UINT iWidth, UINT iHeight )
{
    // Create the bumpmap's surface and texture objects
    if( FAILED( m_pd3dDevice->CreateTexture( iWidth, iHeight, 1, 0, 
        D3DFMT_V8U8, D3DPOOL_MANAGED, &m_pBumpMapTexture ) ) )
    {
        return E_FAIL;
    }

    // Fill the bumpmap texels to simulate a lens
    D3DLOCKED_RECT d3dlr;
    m_pBumpMapTexture->LockRect( 0, &d3dlr, 0, 0 );
    DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
    BYTE* pDst       = (BYTE*)d3dlr.pBits;
    UINT  mid        = iWidth/2;

    for( DWORD y0 = 0; y0 < iHeight; y0++ )
    {
        CHAR* pDst = (CHAR*)d3dlr.pBits + y0*d3dlr.Pitch;

        for( DWORD x0 = 0; x0 < iWidth; x0++ )
        {
            DWORD x1 = ( (x0==iWidth-1)  ? x0 : x0+1 );
            DWORD y1 = ( (x0==iHeight-1) ? y0 : y0+1 );

            FLOAT fDistSq00 = (FLOAT)( (x0-mid)*(x0-mid) + (y0-mid)*(y0-mid) );
            FLOAT fDistSq01 = (FLOAT)( (x1-mid)*(x1-mid) + (y0-mid)*(y0-mid) );
            FLOAT fDistSq10 = (FLOAT)( (x0-mid)*(x0-mid) + (y1-mid)*(y1-mid) );

            FLOAT v00 = ( fDistSq00 > (mid*mid) ) ? 0.0f : sqrtf( (mid*mid) - fDistSq00 );
            FLOAT v01 = ( fDistSq01 > (mid*mid) ) ? 0.0f : sqrtf( (mid*mid) - fDistSq01 );
            FLOAT v10 = ( fDistSq10 > (mid*mid) ) ? 0.0f : sqrtf( (mid*mid) - fDistSq10 );

            FLOAT iDu = (128/D3DX_PI)*atanf(v00-v01); // The delta-u bump value
            FLOAT iDv = (128/D3DX_PI)*atanf(v00-v10); // The delta-v bump value

            *pDst++ = (CHAR)(iDu);
            *pDst++ = (CHAR)(iDv);
        }
    }

    m_pBumpMapTexture->UnlockRect(0);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Load the texture for the background image
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Lake.bmp"),
                                       &m_pBackgroundTexture ) ) )
        return E_FAIL;

    // Create the bump map texture
    if( FAILED( CreateBumpMap( 256, 256 ) ) )
       return E_FAIL;

    // Create a square for rendering the background
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(BACKGROUNDVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_BACKGROUNDVERTEX,
                                                  D3DPOOL_MANAGED, &m_pBackgroundVB ) ) )
        return E_FAIL;

    // Create a square for rendering the lens
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(BUMPVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_BUMPVERTEX,
                                                  D3DPOOL_MANAGED, &m_pLensVB ) ) )
        return E_FAIL;

    BUMPVERTEX* vLens;
    m_pLensVB->Lock( 0, 0, (BYTE**)&vLens, 0 );
    vLens[0].p = D3DXVECTOR3(-256.0f,-256.0f, 0.0f );
    vLens[1].p = D3DXVECTOR3(-256.0f, 256.0f, 0.0f );
    vLens[2].p = D3DXVECTOR3( 256.0f,-256.0f, 0.0f );
    vLens[3].p = D3DXVECTOR3( 256.0f, 256.0f, 0.0f );
    vLens[0].tu1 = 0.0f; vLens[0].tv1 = 1.0f;
    vLens[1].tu1 = 0.0f; vLens[1].tv1 = 0.0f;
    vLens[2].tu1 = 1.0f; vLens[2].tv1 = 1.0f;
    vLens[3].tu1 = 1.0f; vLens[3].tv1 = 0.0f;
    m_pLensVB->Unlock();

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
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -2001.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,     0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,     0.0f );
    D3DXMATRIX matWorld, matView, matProj;

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 3000.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set any appropiate state
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ADDRESSU,   D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ADDRESSV,   D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE );

    // Size the background image
    BACKGROUNDVERTEX* vBackground;
    m_pBackgroundVB->Lock( 0, 0, (BYTE**)&vBackground, 0 );
    for( UINT i=0; i<4; i ++ )
    {
        vBackground[i].p = D3DXVECTOR4( 0.0f, 0.0f, 0.9f, 1.0f );
        vBackground[i].color = 0xffffffff;
    }
    vBackground[0].p.y = (FLOAT)m_d3dsdBackBuffer.Height;
    vBackground[2].p.y = (FLOAT)m_d3dsdBackBuffer.Height;
    vBackground[2].p.x = (FLOAT)m_d3dsdBackBuffer.Width;
    vBackground[3].p.x = (FLOAT)m_d3dsdBackBuffer.Width;
    vBackground[0].tu = 0.0f; vBackground[0].tv = 1.0f;
    vBackground[1].tu = 0.0f; vBackground[1].tv = 0.0f;
    vBackground[2].tu = 1.0f; vBackground[2].tv = 1.0f;
    vBackground[3].tu = 1.0f; vBackground[3].tv = 0.0f;
    m_pBackgroundVB->Unlock();

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
    SAFE_RELEASE( m_pBackgroundTexture );
    SAFE_RELEASE( m_pBumpMapTexture );
    SAFE_RELEASE( m_pBackgroundVB );
    SAFE_RELEASE( m_pLensVB );

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
    if( dwBehavior & D3DCREATE_PUREDEVICE )
        return E_FAIL; // GetTransform doesn't work on PUREDEVICE

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




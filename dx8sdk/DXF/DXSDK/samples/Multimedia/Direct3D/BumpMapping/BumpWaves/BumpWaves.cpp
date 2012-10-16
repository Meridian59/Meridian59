//-----------------------------------------------------------------------------
// File: BumpWaves.cpp
//
// Desc: Code to simulate reflections off waves using bumpmapping.
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

struct VERTEX
{
    D3DXVECTOR3 p;
    FLOAT       tu, tv;
};

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)


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
    LPDIRECT3DTEXTURE8      m_pBackgroundTexture;

    LPDIRECT3DVERTEXBUFFER8 m_pWaterVB;
    LPDIRECT3DTEXTURE8      m_psBumpMap;
    D3DXMATRIX              m_matBumpMat;

    LPDIRECT3DTEXTURE8 CreateBumpMap( DWORD, DWORD, D3DFORMAT );
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
    HRESULT SetEMBMStates();

    UINT m_n;               // Number of vertices in the ground grid along X
    UINT m_m;               // Number of vertices in the ground grid along Z
    UINT m_nTriangles;      // Number of triangles in the ground grid

    DWORD m_dwVertexShader;   // Vertex shader handle for the bump waves
    BOOL  m_bUseVertexShader; // Whether to use vertex shader or FF pipeline

public:
    CMyD3DApplication();
};


//-----------------------------------------------------------------------------
//
// Vertex shader code for bump waves -- this is used when the fixed-function
// approach is not supported by the device
//
// const[0 - 2] Transformation matrix to the camera space, combined with texture
//              transformation matrix, because position in the camera space is 
//              used as texture coordinates. Texture transfoX and Y row are 
//              multiplied by 0.8 to make texture transformation.
//
// const[3-7]   Transformation matrix to the projection space
// const[8]     [0.5, -0.5, 0, 0]
//
// v[0]   - position
// v[1]   - texture coordinates for stage 0
//
char g_strVertexShader[] = 
    "vs.1.1\n"
    "m4x4 oPos, v0, c3 ; transform position to the projection space\n"
    "; Compute vertex position in the camera space - this is our texture coordinates\n"
    "dp4 r0.x, v0, c0 \n"
    "dp4 r0.y, v0, c1 \n"
    "dp4 r0.z, v0, c2 \n"
    "; Do the rest of texture transform (first part was combined with the camera matrix) \n"
    "rcp r0.z, r0.z \n"
    "mad oT1.x, r0.x, r0.z, c8.x \n"
    "mad oT1.y, r0.y, r0.z, c8.y \n"
    "mov oT0.xy, v1     ; Copy input texture coordinates for the stage 0\n";




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
    m_strWindowTitle    = _T("BumpWaves: Using BumpMapping For Waves");
    m_bUseDepthBuffer   = FALSE;

    m_pFont              = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_psBumpMap          = NULL;
    m_pBackgroundTexture = NULL;
    m_pBackgroundVB      = NULL;
    m_pWaterVB           = NULL;

    // The following are set in InitDeviceObjects
    m_n = 0;
    m_m = 0;
    m_nTriangles = 0;

    m_dwVertexShader = 0;
    m_bUseVertexShader = FALSE;
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
    // Setup the bump matrix
    // Min r is 0.04 if amplitude is 32 to miss temporal aliasing
    // Max r is 0.16 for amplitude is 8 to miss spatial aliasing
    FLOAT r = 0.04f;
    m_matBumpMat._11 =  r * cosf( m_fTime * 9.0f );
    m_matBumpMat._12 = -r * sinf( m_fTime * 9.0f );
    m_matBumpMat._21 =  r * sinf( m_fTime * 9.0f );
    m_matBumpMat._22 =  r * cosf( m_fTime * 9.0f );

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
    // Clear the render target
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0L );
    
    // Begin the scene
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK;

    // Set up texture stage states for the background
    m_pd3dDevice->SetTexture( 0, m_pBackgroundTexture );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

    // Render the background
    m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pBackgroundVB, sizeof(VERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Render the water
    SetEMBMStates();
    if( m_bUseVertexShader )
        m_pd3dDevice->SetVertexShader( m_dwVertexShader );
    else
        m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX );

    m_pd3dDevice->SetStreamSource( 0, m_pWaterVB, sizeof(VERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_nTriangles );

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
    TCHAR strInfo[100];
    if( m_bUseVertexShader )
        lstrcpy( strInfo, TEXT("Using Vertex Shader") );
    else
        lstrcpy( strInfo, TEXT("Using Fixed-Function Vertex Pipeline") );
    m_pFont->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,255,255), strInfo );

    // End the scene.
    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetEMBMStates()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::SetEMBMStates()
{
    // Set up texture stage 0's states for the bumpmap
    m_pd3dDevice->SetTexture( 0, m_psBumpMap );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,   D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,   D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT00,   F2DW( m_matBumpMat._11 ) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT01,   F2DW( m_matBumpMat._12 ) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT10,   F2DW( m_matBumpMat._21 ) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_BUMPENVMAT11,   F2DW( m_matBumpMat._22 ) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BUMPENVMAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

    // Set up texture stage 1's states for the environment map
    m_pd3dDevice->SetTexture( 1, m_pBackgroundTexture );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

    if( m_bUseVertexShader )
    {
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
    }
    else
    {
        // Set up projected texture coordinates
        // tu = (0.8x + 0.5z) / z
        // tv = (0.8y - 0.5z) / z
        D3DXMATRIX mat;
        mat._11 = 0.8f; mat._12 = 0.0f; mat._13 = 0.0f;
        mat._21 = 0.0f; mat._22 = 0.8f; mat._23 = 0.0f;
        mat._31 = 0.5f; mat._32 =-0.5f; mat._33 = 1.0f;
        mat._41 = 0.0f; mat._42 = 0.0f; mat._43 = 0.0f;

        m_pd3dDevice->SetTransform( D3DTS_TEXTURE1, &mat );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3|D3DTTFF_PROJECTED );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1 );

    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateBumpMap()
// Desc: Creates a bumpmap
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 CMyD3DApplication::CreateBumpMap( DWORD dwWidth, DWORD dwHeight,
                                                     D3DFORMAT d3dBumpFormat )
{
    LPDIRECT3DTEXTURE8 psBumpMap;

    // Check if the device can create the format
    if( FAILED( m_pD3D->CheckDeviceFormat( m_d3dCaps.AdapterOrdinal,
                                           m_d3dCaps.DeviceType,
                                           m_d3dsdBackBuffer.Format,
                                           0, D3DRTYPE_TEXTURE, d3dBumpFormat ) ) )
        return NULL;

    // Create the bump map texture
    if( FAILED( m_pd3dDevice->CreateTexture( dwWidth, dwHeight, 1, 0 /* Usage */,
                                             d3dBumpFormat, D3DPOOL_MANAGED,
                                             &psBumpMap ) ) )
        return NULL;

    // Lock the surface and write in some bumps for the waves
    D3DLOCKED_RECT d3dlr;
    psBumpMap->LockRect( 0, &d3dlr, 0, 0 );
    CHAR* pDst = (CHAR*)d3dlr.pBits;
    CHAR  iDu, iDv;

    for( DWORD y=0; y<dwHeight; y++ )
    {
        CHAR* pPixel = pDst;

        for( DWORD x=0; x<dwWidth; x++ )
        {
            FLOAT fx = x/(FLOAT)dwWidth - 0.5f;
            FLOAT fy = y/(FLOAT)dwHeight - 0.5f;

            FLOAT r = sqrtf( fx*fx + fy*fy );

            iDu  = (CHAR)( 64 * cosf( 300.0f * r ) * expf( -r * 5.0f ) );
            iDu += (CHAR)( 32 * cosf( 150.0f * ( fx + fy ) ) );
            iDu += (CHAR)( 16 * cosf( 140.0f * ( fx * 0.85f - fy ) ) );

            iDv  = (CHAR)( 64 * sinf( 300.0f * r ) * expf( -r * 5.0f ) );
            iDv += (CHAR)( 32 * sinf( 150.0f * ( fx + fy ) ) );
            iDv += (CHAR)( 16 * sinf( 140.0f * ( fx * 0.85f - fy ) ) );

            *pPixel++ = iDu;
            *pPixel++ = iDv;
        }
        pDst += d3dlr.Pitch;
    }
    psBumpMap->UnlockRect(0);

    return psBumpMap;
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
                                       &m_pBackgroundTexture, D3DFMT_R5G6B5 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Create the bumpmap. 
    m_psBumpMap = CreateBumpMap( 256, 256, D3DFMT_V8U8 );
    if( NULL == m_psBumpMap )
        return E_FAIL;

    // Create a square for rendering the background
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(VERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pBackgroundVB ) ) )
        return E_FAIL;
    VERTEX* v;
    m_pBackgroundVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p  = D3DXVECTOR3(-1000.0f,    0.0f, 0.0f );
    v[1].p  = D3DXVECTOR3(-1000.0f, 1000.0f, 0.0f );
    v[2].p  = D3DXVECTOR3( 1000.0f,    0.0f, 0.0f );
    v[3].p  = D3DXVECTOR3( 1000.0f, 1000.0f, 0.0f );
    v[0].tu = 0.0f; v[0].tv = 147/256.0f;
    v[1].tu = 0.0f; v[1].tv = 0.0f;
    v[2].tu = 1.0f; v[2].tv = 147/256.0f;
    v[3].tu = 1.0f; v[3].tv = 0.0f;
    m_pBackgroundVB->Unlock();

    // See if EMBM and projected vertices are supported at the same time
    // in the fixed-function shader.  If not, switch to using a vertex shader.
    m_bUseVertexShader = FALSE;
    SetEMBMStates();
    DWORD dwPasses;
    if( FAILED( m_pd3dDevice->ValidateDevice( &dwPasses ) ) )
        m_bUseVertexShader = TRUE;

    // If D3DPTEXTURECAPS_PROJECTED is set, projected textures are computed
    // per pixel, so this sample will work fine with just a quad for the water
    // model.  If it's not set, textures are projected per vertex rather than 
    // per pixel, so distortion will be visible unless we use more vertices.
    if( m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED && !m_bUseVertexShader)
    {
        m_n = 2;               // Number of vertices in the ground grid along X
        m_m = 2;               // Number of vertices in the ground grid along Z
    }
    else
    {
        m_n = 8;               // Number of vertices in the ground grid along X
        m_m = 8;               // Number of vertices in the ground grid along Z
    }
    m_nTriangles = (m_n-1)*(m_m-1)*2;   // Number of triangles in the ground

    // Create a square grid m_n*m_m for rendering the water
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_nTriangles*3*sizeof(VERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pWaterVB ) ) )
        return E_FAIL;
    m_pWaterVB->Lock( 0, 0, (BYTE**)&v, 0 );
    float dX = 2000.0f/(m_n-1);
    float dZ = 1250.0f/(m_m-1);
    float x0 = -1000;
    float z0 = -1250;
    float dU = 1.0f/(m_n-1);
    float dV = 0.7f/(m_m-1);
    UINT k = 0;
    for (UINT z=0; z < (m_m-1); z++)
    {
        for (UINT x=0; x < (m_n-1); x++)
        {
            v[k].p  = D3DXVECTOR3(x0 + x*dX, 0.0f, z0 + z*dZ );
            v[k].tu = x*dU; 
            v[k].tv = z*dV;
            k++;
            v[k].p  = D3DXVECTOR3(x0 + x*dX, 0.0f, z0 + (z+1)*dZ );
            v[k].tu = x*dU; 
            v[k].tv = (z+1)*dV;
            k++;
            v[k].p  = D3DXVECTOR3(x0 + (x+1)*dX, 0.0f, z0 + (z+1)*dZ );
            v[k].tu = (x+1)*dU; 
            v[k].tv = (z+1)*dV;
            k++;
            v[k].p  = D3DXVECTOR3(x0 + x*dX, 0.0f, z0 + z*dZ );
            v[k].tu = x*dU; 
            v[k].tv = z*dV;
            k++;
            v[k].p  = D3DXVECTOR3(x0 + (x+1)*dX, 0.0f, z0 + (z+1)*dZ );
            v[k].tu = (x+1)*dU; 
            v[k].tv = (z+1)*dV;
            k++;
            v[k].p  = D3DXVECTOR3(x0 + (x+1)*dX, 0.0f, z0 + z*dZ );
            v[k].tu = (x+1)*dU; 
            v[k].tv = z*dV;
            k++;
        }
    }
    m_pWaterVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    HRESULT hr;

    m_pFont->RestoreDeviceObjects();

    // Set the transform matrices
    D3DXVECTOR3 vEyePt(    0.0f, 400.0f, -1650.0f );
    D3DXVECTOR3 vLookatPt( 0.0f,   0.0f,     0.0f );
    D3DXVECTOR3 vUpVec(    0.0f,   1.0f,     0.0f );
    D3DXMATRIX matWorld, matView, matProj;

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    D3DXMatrixPerspectiveFovLH( &matProj, 1.00f, 1.0f, 1.0f, 10000.0f );
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

    if ( m_bUseVertexShader )
    {
        DWORD dwWaveDecl[] =
        {
            D3DVSD_STREAM( 0 ),
            D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position
            D3DVSD_REG( 1, D3DVSDT_FLOAT2 ), // Tex coords
            D3DVSD_END()
        };

        LPD3DXBUFFER pCode;
        if( FAILED( hr = D3DXAssembleShader( g_strVertexShader, 
                                             strlen(g_strVertexShader), 
                                             0, NULL, &pCode, NULL ) ) )
        {
            return hr;
        }
        hr = m_pd3dDevice->CreateVertexShader( dwWaveDecl,
                                               (DWORD*)pCode->GetBufferPointer(),
                                               &m_dwVertexShader, 0);
        if( FAILED( hr ) )
        {
            return hr;
        }
        pCode->Release();

        D3DXMATRIX matCamera, matFinal;
        D3DXMatrixMultiply( &matCamera, &matWorld, &matView );
        D3DXMatrixMultiply( &matFinal, &matCamera, &matProj );
        D3DXMatrixTranspose(&matCamera, &matCamera);
        D3DXMatrixTranspose(&matFinal, &matFinal);
        matCamera(0, 0) *= 0.8f;
        matCamera(0, 1) *= 0.8f;
        matCamera(0, 2) *= 0.8f;
        matCamera(0, 3) *= 0.8f;
        matCamera(1, 0) *= 0.8f;
        matCamera(1, 1) *= 0.8f;
        matCamera(1, 2) *= 0.8f;
        matCamera(1, 3) *= 0.8f;
        if (FAILED( hr = m_pd3dDevice->SetVertexShaderConstant(0, &matCamera, 3) ) )
        {
            return hr;
        }
        if (FAILED( hr = m_pd3dDevice->SetVertexShaderConstant(3, &matFinal, 4) ) )
        {
            return hr;
        }
        FLOAT data[4] = {0.5f, -0.5f, 0, 0};
        if (FAILED( hr = m_pd3dDevice->SetVertexShaderConstant(8, &data, 1) ) )
        {
            return hr;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    if( m_dwVertexShader != 0 )
    {
        m_pd3dDevice->DeleteVertexShader( m_dwVertexShader );
        m_dwVertexShader = 0;
    }

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
    SAFE_RELEASE( m_psBumpMap );
    SAFE_RELEASE( m_pBackgroundVB );
    SAFE_RELEASE( m_pWaterVB );

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

    // Else, reject the device
    return E_FAIL;
}



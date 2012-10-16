//-----------------------------------------------------------------------------
// File: FishEye.cpp
//
// Desc: Example code showing how to do a fisheye lens effect with cubemapping.
//       The scene is rendering into a cubemap each frame, and then a
//       funky-shaped object is rendered using the cubemap and an environment
//       map.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
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
// Name: struct ENVMAPPEDVERTEX
// Desc: D3D vertex type for environment-mapped objects
//-----------------------------------------------------------------------------
struct ENVMAPPEDVERTEX
{
    D3DXVECTOR3 p; // Position
    D3DXVECTOR3 n; // Normal
};

#define D3DFVF_ENVMAPVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    // Font for drawing text
    CD3DFont* m_pFont;

    CD3DMesh*               m_pSkyBoxObject;
    LPDIRECT3DCUBETEXTURE8  m_pCubeMap;

    LPDIRECT3DVERTEXBUFFER8 m_pFishEyeLensVB;
    LPDIRECT3DINDEXBUFFER8  m_pFishEyeLensIB;
    DWORD                   m_dwNumFishEyeLensVertices;
    DWORD                   m_dwNumFishEyeLensFaces;

    HRESULT RenderScene();
    HRESULT RenderSceneIntoCubeMap();
    HRESULT GenerateFishEyeLens( DWORD, DWORD, FLOAT );

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

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
    m_strWindowTitle    = _T("FishEye: Environment mapping");
    m_bUseDepthBuffer   = TRUE;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pSkyBoxObject     = new CD3DMesh();
    m_pCubeMap          = NULL;
    m_pFishEyeLensVB    = NULL;
    m_pFishEyeLensIB    = NULL;
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
    // When the window has focus, let the mouse adjust the scene
    if( GetFocus() )
    {
        D3DXQUATERNION quat = D3DUtil_GetRotationFromCursor( m_hWnd );
        D3DXMATRIX matTrackBall;
        D3DXMatrixRotationQuaternion( &matTrackBall, &quat );
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTrackBall );

        // Render the scene into the surfaces of the cubemap
        if( FAILED( RenderSceneIntoCubeMap() ) )
            return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Renders all visual elements in the scene. This is called by the main
//       Render() function, and also by the RenderIntoCubeMap() function.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderScene()
{
    // Render the skybox
    {
        // Save current state
        D3DXMATRIX matViewSave, matProjSave;
        m_pd3dDevice->GetTransform( D3DTS_VIEW,       &matViewSave );
        m_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProjSave );

        // Disable zbuffer, center view matrix, and set FOV to 90 degrees
        D3DXMATRIX matView = matViewSave;
        D3DXMATRIX matProj = matViewSave;
        matView._41 = matView._42 = matView._43 = 0.0f;
        D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/2, 1.0f, 0.5f, 10000.0f );
        m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
        m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
        m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  FALSE );

        // Render the skybox
        m_pSkyBoxObject->Render( m_pd3dDevice );

        // Restore the render states
        m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matViewSave );
        m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProjSave );
        m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
    }

    // Render any other elements of the scene here. In this sample, only a
    // skybox is render, but a much more interesting scene could be rendered
    // instead.

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderSceneIntoCubeMap()
// Desc: Renders the scene to each of the 6 faces of the cube map
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderSceneIntoCubeMap()
{
    // Save transformation matrices of the device
    D3DXMATRIX   matProjSave, matViewSave;
    m_pd3dDevice->GetTransform( D3DTS_VIEW,       &matViewSave );
    m_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProjSave );

    // Set the projection matrix for a field of view of 90 degrees
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/2, 1.0f, 0.5f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Get the current view matrix, to concat it with the cubemap view vectors
    D3DXMATRIX matViewDir;
    m_pd3dDevice->GetTransform( D3DTS_VIEW, &matViewDir );
    matViewDir._41 = 0.0f; matViewDir._42 = 0.0f; matViewDir._43 = 0.0f;

    // Store the current backbuffer and zbuffer
    LPDIRECT3DSURFACE8 pBackBuffer, pZBuffer;
    m_pd3dDevice->GetRenderTarget( &pBackBuffer );
    m_pd3dDevice->GetDepthStencilSurface( &pZBuffer );

    // Render to the six faces of the cube map
    for( DWORD i=0; i<6; i++ )
    {
        // Set the view transform for this cubemap surface
        D3DXMATRIX matView;
        matView = D3DUtil_GetCubeMapViewMatrix( (D3DCUBEMAP_FACES)i );
        D3DXMatrixMultiply( &matView, &matViewDir, &matView );
        m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

        // Set the rendertarget to the i'th cubemap surface
        LPDIRECT3DSURFACE8 pCubeMapFace;
        m_pCubeMap->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &pCubeMapFace );
        m_pd3dDevice->SetRenderTarget( pCubeMapFace, NULL );
        pCubeMapFace->Release();

        // Render the scene
        m_pd3dDevice->BeginScene();
        RenderScene();
        m_pd3dDevice->EndScene();
    }

    // Change the rendertarget back to the main backbuffer
    m_pd3dDevice->SetRenderTarget( pBackBuffer, pZBuffer );
    pBackBuffer->Release();
    pZBuffer->Release();

    // Restore the original transformation matrices
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matViewSave );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProjSave );

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

    // Set the states we want: identity matrix, no z-buffer, and cubemap texture
    // coordinate generation
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 );

    // Render the fisheye lens object with the environment-mapped body.
    m_pd3dDevice->SetTexture( 0, m_pCubeMap );
    m_pd3dDevice->SetVertexShader( D3DFVF_ENVMAPVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pFishEyeLensVB, sizeof(ENVMAPPEDVERTEX) );
    m_pd3dDevice->SetIndices( m_pFishEyeLensIB, 0 );
    m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                        0, m_dwNumFishEyeLensVertices,
                                        0, m_dwNumFishEyeLensFaces );

    // Restore the render states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

    // End the scene.
    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    HRESULT hr;

    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Load the file objects
    if( FAILED( hr = m_pSkyBoxObject->Create( m_pd3dDevice, _T("Lobby_skybox.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Create the fisheye lens
    if( FAILED( hr = GenerateFishEyeLens( 20, 20, 1.0f ) ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restore device-memory objects and state after a device is created or
//       resized.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    HRESULT hr;

    m_pFont->RestoreDeviceObjects();

    // Restore device objects for the skybox
    m_pSkyBoxObject->RestoreDeviceObjects( m_pd3dDevice );

    // Create the cubemap
    if( FAILED( hr = m_pd3dDevice->CreateCubeTexture( 256, 1, D3DUSAGE_RENDERTARGET,
                                                      m_d3dsdBackBuffer.Format,
                                                      D3DPOOL_DEFAULT, &m_pCubeMap ) ) )
        return hr;

    // Set default render states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_MIRROR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_MIRROR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

    // Set the transforms
    D3DXVECTOR3 m_vEyePt    = D3DXVECTOR3( 0.0f, 0.0f,-5.0f );
    D3DXVECTOR3 m_vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 m_vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    D3DXMATRIX matWorld, matView, matProj;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &m_vEyePt, &m_vLookatPt, &m_vUpVec );
    D3DXMatrixOrthoLH( &matProj, 2.0f, 2.0f, 0.5f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pSkyBoxObject->InvalidateDeviceObjects();

    SAFE_RELEASE( m_pCubeMap );

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
    m_pSkyBoxObject->Destroy();

    SAFE_RELEASE( m_pFishEyeLensVB );
    SAFE_RELEASE( m_pFishEyeLensIB );

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
    SAFE_DELETE( m_pSkyBoxObject );

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

    // Check for cubemapping devices
    if( 0 == ( pCaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP ) )
        return E_FAIL;

    // Check that we can create a cube texture that we can render into
    if( FAILED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, 
        pCaps->DeviceType, Format, D3DUSAGE_RENDERTARGET,
        D3DRTYPE_CUBETEXTURE, Format ) ) )
    {
       return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GenerateFishEyeLens()
// Desc: Makes vertex and index data for a fish eye lens
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::GenerateFishEyeLens( DWORD dwNumRings, DWORD dwNumSections,
                                                FLOAT fScale )
{
    ENVMAPPEDVERTEX* pVertices;
    WORD*            pIndices;
    DWORD            dwNumTriangles = (dwNumRings+1) * dwNumSections * 2;
    DWORD            dwNumVertices  = (dwNumRings+1) * dwNumSections + 2;
    DWORD            i, j, m;

    // Generate space for the required triangles and vertices.
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(ENVMAPPEDVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_ENVMAPVERTEX,
                                                  D3DPOOL_MANAGED, &m_pFishEyeLensVB ) ) )
        return E_FAIL;

    if( FAILED( m_pd3dDevice->CreateIndexBuffer( dwNumTriangles*3*sizeof(WORD),
                                                 D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                                 D3DPOOL_MANAGED, &m_pFishEyeLensIB ) ) )
        return E_FAIL;

    m_pFishEyeLensVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );
    m_pFishEyeLensIB->Lock( 0, 0, (BYTE**)&pIndices, 0 );

    // Generate vertices at the end points.
    pVertices->p = D3DXVECTOR3( 0.0f, 0.0f, fScale );
    pVertices->n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    pVertices++;

    // Generate vertex points for rings
    FLOAT r = 0.0f;
    for( i = 0; i < (dwNumRings+1); i++ )
    {
        FLOAT phi = 0.0f;

        for( j = 0; j < dwNumSections; j++ )
        {
            FLOAT x  =  r * sinf(phi);
            FLOAT y  =  r * cosf(phi);
            FLOAT z  = 0.5f - 0.5f * ( x*x + y*y );

            FLOAT nx = -x;
            FLOAT ny = -y;
            FLOAT nz = 1.0f;

            pVertices->p = D3DXVECTOR3( x, y, z );
            pVertices->n = D3DXVECTOR3( nx, ny, nz );
            pVertices++;

            phi += (FLOAT)(2*D3DX_PI / dwNumSections);
        }

        r += 1.5f/dwNumRings;
    }

    // Generate triangles for the centerpiece
    for( i = 0; i < 2*dwNumSections; i++ )
    {
        *pIndices++ = (WORD)(0);
        *pIndices++ = (WORD)(i + 1);
        *pIndices++ = (WORD)(1 + ((i + 1) % dwNumSections));
    }

    // Generate triangles for the rings
    m = 1;  // 1st vertex begins at 1 to skip top point

    for( i = 0; i < dwNumRings; i++ )
    {
        for( j = 0; j < dwNumSections; j++ )
        {
            *pIndices++ = (WORD)(m + j);
            *pIndices++ = (WORD)(m + dwNumSections + j);
            *pIndices++ = (WORD)(m + dwNumSections + ((j + 1) % dwNumSections));

            *pIndices++ = (WORD)(m + j);
            *pIndices++ = (WORD)(m + dwNumSections + ((j + 1) % dwNumSections));
            *pIndices++ = (WORD)(m + ((j + 1) % dwNumSections));
        }
        m += dwNumSections;
    }

    m_pFishEyeLensVB->Unlock();
    m_pFishEyeLensIB->Unlock();
    m_dwNumFishEyeLensVertices = dwNumVertices;
    m_dwNumFishEyeLensFaces    = dwNumTriangles;

    return S_OK;
}





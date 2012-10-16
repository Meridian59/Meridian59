//-----------------------------------------------------------------------------
// File: StencilMirror.cpp
//
// Desc: Example code showing how to use stencil buffers to implement planar
//       mirrors.
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <math.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Custom vertex types
//-----------------------------------------------------------------------------
struct MESHVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;
};

struct MIRRORVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
};

#define D3DFVF_MESHVERTEX   (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define D3DFVF_MIRRORVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

#define FOG_COLOR 0x00000080

inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

inline FLOAT HeightField( FLOAT x, FLOAT z )
{
    FLOAT y = 0.0f;
    y += 7.0f * cosf( 0.051f*x + 0.0f ) * sinf( 0.055f*x + 0.0f );
    y += 7.0f * cosf( 0.053f*z + 0.0f ) * sinf( 0.057f*z + 0.0f );
    y += 1.0f * cosf( 0.101f*x + 0.0f ) * sinf( 0.105f*x + 0.0f );
    y += 1.0f * cosf( 0.103f*z + 0.0f ) * sinf( 0.107f*z + 0.0f );
    y += 1.0f * cosf( 0.251f*x + 0.0f ) * sinf( 0.255f*x + 0.0f );
    y += 1.0f * cosf( 0.253f*z + 0.0f ) * sinf( 0.257f*z + 0.0f );
    return y;
}




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont* m_pFont;

    LPDIRECT3DVERTEXBUFFER8 m_pMirrorVB;
    D3DMATERIAL8 m_mtrlMirrorMaterial;      // Material of the mirror
    D3DXMATRIX   m_matMirrorMatrix;         // Matrix to position mirror

    CD3DFile*    m_pTerrain;                // X file of terrain
    D3DXMATRIX   m_matTerrainMatrix;        // Matrix to position terrain

    CD3DFile*    m_pHelicopter;             // X file object to render
    D3DXMATRIX   m_matHelicopterMatrix;     // Matrix to animate X file object

    HRESULT RenderScene();
    HRESULT RenderMirror();
    HRESULT CreateStencilBuffer();
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
    m_strWindowTitle    = _T("StencilMirror: Doing Reflections with Stencils");
    m_bUseDepthBuffer   = TRUE;
    m_dwMinDepthBits    = 16;
    m_dwMinStencilBits  = 4;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pTerrain          = new CD3DFile();
    m_pHelicopter       = new CD3DFile();
    m_pMirrorVB         = NULL;
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
    // Position the terrain
    D3DXMatrixTranslation( &m_matTerrainMatrix, 0.0f, 8.0f, 0.0f );

    // Position the mirror (water polygon intersecting with terrain)
    D3DXMatrixTranslation( &m_matMirrorMatrix, 0.0f, 0.0f, 0.0f );

    // Position and animate the main object
    FLOAT fObjectPosX = 50.0f*sinf(m_fTime/2);
    FLOAT fObjectPosY = 6;
    FLOAT fObjectPosZ = 10.0f*cosf(m_fTime/2);
    D3DXMATRIX matRoll, matPitch, matRotate, matScale, matTranslate;
    D3DXMatrixRotationZ( &matRoll, 0.2f*sinf(m_fTime/2)  );
    D3DXMatrixRotationY( &matRotate, m_fTime/2-D3DX_PI/2  );
    D3DXMatrixRotationX( &matPitch, -0.1f * (1+cosf(m_fTime))  );
    D3DXMatrixScaling( &matScale, 0.5f, 0.5f, 0.5f );
    D3DXMatrixTranslation( &matTranslate, fObjectPosX, fObjectPosY, fObjectPosZ );
    D3DXMatrixMultiply( &m_matHelicopterMatrix, &matScale, &matTranslate );
    D3DXMatrixMultiply( &m_matHelicopterMatrix, &matRoll, &m_matHelicopterMatrix );
    D3DXMatrixMultiply( &m_matHelicopterMatrix, &matRotate, &m_matHelicopterMatrix );
    D3DXMatrixMultiply( &m_matHelicopterMatrix, &matPitch, &m_matHelicopterMatrix );

    // Move the camera around
    FLOAT fEyeX = 10.0f * sinf( m_fTime/2.0f );
    FLOAT fEyeY =  3.0f * sinf( m_fTime/25.0f ) + 13.0f;
    FLOAT fEyeZ =  5.0f * cosf( m_fTime/2.0f );
    D3DXMATRIX matView;
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( fEyeX, fEyeY, fEyeZ );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( fObjectPosX, fObjectPosY, fObjectPosZ );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderScene()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderScene()
{
    // Render terrain
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTerrainMatrix );
    m_pTerrain->Render( m_pd3dDevice );

    // Draw the mirror
    m_pd3dDevice->SetTexture( 0, NULL );
    m_pd3dDevice->SetMaterial( &m_mtrlMirrorMaterial );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matMirrorMatrix );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,     D3DBLEND_DESTCOLOR );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,    D3DBLEND_ZERO );
    m_pd3dDevice->SetVertexShader( D3DFVF_MIRRORVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pMirrorVB, sizeof(MIRRORVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    // Draw the object. Note: do this last, in case the object has alpha
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matHelicopterMatrix );
    m_pHelicopter->Render( m_pd3dDevice );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderMirror()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderMirror()
{
    // Turn depth buffer off, and stencil buffer on
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,     D3DCMP_ALWAYS );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,      0x1 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,     0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK,0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_REPLACE );

    // Make sure no pixels are written to the z-buffer or frame buffer
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    // Draw the reflecting surface into the stencil buffer
    m_pd3dDevice->SetTexture( 0, NULL);
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matMirrorMatrix );
    m_pd3dDevice->SetVertexShader( D3DFVF_MIRRORVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pMirrorVB, sizeof(MIRRORVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Save the view matrix
    D3DXMATRIX matViewSaved;
    m_pd3dDevice->GetTransform( D3DTS_VIEW, &matViewSaved );

    // Reflect camera in X-Z plane mirror
    D3DXMATRIX matView, matReflect;
    D3DXPLANE plane;
    D3DXPlaneFromPointNormal( &plane, &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0) );
    D3DXMatrixReflect( &matReflect, &plane );
    D3DXMatrixMultiply( &matView, &matReflect, &matViewSaved );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Set a clip plane, so that only objects above the water are reflected
    m_pd3dDevice->SetClipPlane( 0, plane );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0x01 );

    // Setup render states to a blended render scene against mask in stencil
    // buffer. An important step here is to reverse the cull-order of the
    // polygons, since the view matrix is being relected.
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_EQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,     D3DBLEND_DESTCOLOR );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,    D3DBLEND_ZERO );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CW );

    // Clear the zbuffer (leave frame- and stencil-buffer intact)
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_ZBUFFER, 0L, 1.0f, 0L );

    // Render the scene
    RenderScene();

    // Restore render states
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  0x00 );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matViewSaved );

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
    // Clear the viewport, zbuffer, and stencil buffer
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
                         FOG_COLOR, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Render the scene
        RenderScene();

        // Render the reflection in the mirror
        RenderMirror();

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
    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Load the main file object
    if( FAILED( m_pHelicopter->Create( m_pd3dDevice, _T("Heli.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Load the terrain
    if( FAILED( m_pTerrain->Create( m_pd3dDevice, _T("SeaFloor.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Tweak the terrain vertices to add some bumpy terrain
    CD3DMesh* pMesh = m_pTerrain->FindMesh( _T("SeaFloor") );
    if( pMesh )
    {
        // Set FVF to VertexFVF
        pMesh->SetFVF( m_pd3dDevice, D3DFVF_MESHVERTEX );

        // Get access to the mesh vertices
        LPDIRECT3DVERTEXBUFFER8 pVB;
        MESHVERTEX* pVertices;
        DWORD dwNumVertices = pMesh->GetSysMemMesh()->GetNumVertices();
        pMesh->GetSysMemMesh()->GetVertexBuffer( &pVB );
        pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );

        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            D3DXVECTOR3 v00( pVertices[i].p.x + 0.0f, 0.0f, pVertices[i].p.z + 0.0f );
            D3DXVECTOR3 v10( pVertices[i].p.x + 0.1f, 0.0f, pVertices[i].p.z + 0.0f );
            D3DXVECTOR3 v01( pVertices[i].p.x + 0.0f, 0.0f, pVertices[i].p.z + 0.1f );
            v00.y = HeightField( 1*v00.x, 1*v00.z );
            v10.y = HeightField( 1*v10.x, 1*v10.z );
            v01.y = HeightField( 1*v01.x, 1*v01.z );

            D3DXVECTOR3 n;
            D3DXVec3Cross( &n, &(v01-v00), &(v10-v00) );
            D3DXVec3Normalize( &n, &n );

            pVertices[i].p.y  = v00.y;
            pVertices[i].n.x  = n.x;
            pVertices[i].n.y  = n.y;
            pVertices[i].n.z  = n.z;
            pVertices[i].tu  *= 10;
            pVertices[i].tv  *= 10;
        }

        pVB->Unlock();
        pVB->Release();
    }

    // Create a big square for rendering the mirror
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(MIRRORVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_MIRRORVERTEX,
                                       D3DPOOL_MANAGED, &m_pMirrorVB ) ) )
        return E_FAIL;

    MIRRORVERTEX* v;
    m_pMirrorVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p = D3DXVECTOR3(-80.0f, 0.0f,-80.0f );
    v[0].n = D3DXVECTOR3( 0.0f,  1.0f,  0.0f );
    v[1].p = D3DXVECTOR3(-80.0f, 0.0f, 80.0f );
    v[1].n = D3DXVECTOR3( 0.0f,  1.0f,  0.0f );
    v[2].p = D3DXVECTOR3( 80.0f, 0.0f,-80.0f );
    v[2].n = D3DXVECTOR3( 0.0f,  1.0f,  0.0f );
    v[3].p = D3DXVECTOR3( 80.0f, 0.0f, 80.0f );
    v[3].n = D3DXVECTOR3( 0.0f,  1.0f,  0.0f );
    m_pMirrorVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restore device-memory objects and state after a device is created or
//       resized.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Build the device objects for the file-based objecs
    m_pHelicopter->RestoreDeviceObjects( m_pd3dDevice );
    m_pTerrain->RestoreDeviceObjects( m_pd3dDevice );

    // Set up textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    // Set up misc render states
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x00555555 );

    // Set the transform matrices
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 5.5f, -15.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 1.5f,   0.0f  );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,   0.0f  );
    D3DXMATRIX matWorld, matView, matProj;

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 1000.0f );

    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set up a material
    D3DUtil_InitMaterial( m_mtrlMirrorMaterial, 0.5f, 0.8f, 1.0f );

    // Set up the light
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, -1.0f, 1.0f );
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );

    // Turn on fog
    FLOAT fFogStart =  80.0f;
    FLOAT fFogEnd   = 100.0f;
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR,       FOG_COLOR );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_RANGEFOGENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGSTART,       FtoDW(fFogStart) );
    m_pd3dDevice->SetRenderState( D3DRS_FOGEND,         FtoDW(fFogEnd) );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pTerrain->InvalidateDeviceObjects();
    m_pHelicopter->InvalidateDeviceObjects();

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
    m_pTerrain->Destroy();
    m_pHelicopter->Destroy();

    SAFE_RELEASE( m_pMirrorVB );

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
    SAFE_DELETE( m_pTerrain );
    SAFE_DELETE( m_pHelicopter );

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

    // Make sure device supports directional lights
    if( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( 0 == ( pCaps->VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS ) )
            return E_FAIL;
    }

    return S_OK;
}





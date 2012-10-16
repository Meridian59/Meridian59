//-----------------------------------------------------------------------------
// File: ClipMirror.cpp
//
// Desc: This sample shows how to use clip planes to implement a planar mirror.
//       The scene is reflected in a mirror and rendered in a 2nd pass. The
//       corners of the mirrors, together with the camera eye point, are used
//       to define a custom set of clip planes so that the reflected geometry
//       appears only within the mirror's boundaries.
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
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
// Name: struct MIRRORVERTEX
// Desc: Custom mirror vertex type
//-----------------------------------------------------------------------------
struct MIRRORVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    DWORD       color;
};

#define D3DFVF_MIRRORVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*    m_pFont;       // Font for drawing text
    CD3DMesh*    m_pTeapot;     // The teapot object
    D3DXMATRIX   m_matTeapot;   // Teapot's local matrix

    LPDIRECT3DVERTEXBUFFER8 m_pMirrorVB;

    D3DXVECTOR3  m_vEyePt;      // Vectors defining the camera
    D3DXVECTOR3  m_vLookatPt;
    D3DXVECTOR3  m_vUpVec;

    HRESULT RenderMirror();
    HRESULT RenderScene();

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format );
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
    m_strWindowTitle    = _T("ClipMirror: Using D3D Clip Planes");
    m_bUseDepthBuffer   = TRUE;
    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pTeapot           = new CD3DMesh;
    m_pMirrorVB         = NULL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Initialize the camera's orientation
    m_vEyePt    = D3DXVECTOR3( 0.0f, 2.0f, -6.5f );
    m_vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
    m_vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Set the teapot's local matrix (rotating about the y-axis)
    D3DXMatrixRotationY( &m_matTeapot, m_fTime );

    // When the window has focus, let the mouse adjust the camera view
    if( GetFocus() )
    {
        D3DXQUATERNION quat = D3DUtil_GetRotationFromCursor( m_hWnd );
        m_vEyePt.x = 5*quat.y;
        m_vEyePt.y = 5*quat.x;
        m_vEyePt.z = -sqrtf( 50.0f - 25*quat.x*quat.x - 25*quat.y*quat.y );

        D3DXMATRIX matView;
        D3DXMatrixLookAtLH( &matView, &m_vEyePt, &m_vLookatPt, &m_vUpVec );
        m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Renders all objects in the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderScene()
{
    D3DXMATRIX matLocal, matWorldSaved;
    m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorldSaved );

    // Build the local matrix
    D3DXMatrixMultiply( &matLocal, &m_matTeapot, &matWorldSaved );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matLocal );

    // Render the object
    m_pTeapot->Render( m_pd3dDevice );

    // Restore the modified render states
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorldSaved );

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderMirror()
// Desc: Renders the scene as reflected in a mirror. The corners of the mirror
//       define a plane, which is used to build the reflection matrix. The
//       scene is rendered with the cull-mode reversed, since all normals in
//       the scene are likewise reflected.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderMirror()
{
    D3DXMATRIX matWorldSaved;
    D3DXMATRIX matReflectInMirror;
    D3DXPLANE  plane;

    // Save the world matrix so it can be restored
    m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorldSaved );

    // Get the four corners of the mirror. (This should be dynamic rather than
    // hardcoded.)
    D3DXVECTOR3 a(-1.5f, 1.5f, 3.0f );
    D3DXVECTOR3 b( 1.5f, 1.5f, 3.0f );
    D3DXVECTOR3 c( -1.5f,-1.5f, 3.0f );
    D3DXVECTOR3 d( 1.5f,-1.5f, 3.0f );

    // Construct the reflection matrix
    D3DXPlaneFromPoints( &plane, &a, &b, &c );
    D3DXMatrixReflect( &matReflectInMirror, &plane );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matReflectInMirror );

    // Reverse the cull mode (since normals will be reflected)
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

    // Set the custom clip planes (so geometry is clipped by mirror edges).
    // This is the heart of this sample. The mirror has 4 edges, so there are
    // 4 clip planes, each defined by two mirror vertices and the eye point.
    m_pd3dDevice->SetClipPlane( 0, *D3DXPlaneFromPoints( &plane, &b, &a, &m_vEyePt ) );
    m_pd3dDevice->SetClipPlane( 1, *D3DXPlaneFromPoints( &plane, &d, &b, &m_vEyePt ) );
    m_pd3dDevice->SetClipPlane( 2, *D3DXPlaneFromPoints( &plane, &c, &d, &m_vEyePt ) );
    m_pd3dDevice->SetClipPlane( 3, *D3DXPlaneFromPoints( &plane, &a, &c, &m_vEyePt ) );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 
        D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2 | D3DCLIPPLANE3 );

    // Render the scene
    RenderScene();

    // Restore the modified render states
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorldSaved );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0x00 );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

    // Finally, render the mirror itself (as an alpha-blended quad)
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    m_pd3dDevice->SetStreamSource( 0, m_pMirrorVB, sizeof(MIRRORVERTEX) );
    m_pd3dDevice->SetVertexShader( D3DFVF_MIRRORVERTEX );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );

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
                         0x000000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Render the scene
        RenderScene();

        // Render the scene in the mirror
        RenderMirror();

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

    // Set up the geometry objects
    if( FAILED( m_pTeapot->Create( m_pd3dDevice, _T("Teapot.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Create a square for rendering the mirror
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(MIRRORVERTEX),
                                                  D3DUSAGE_WRITEONLY,
                                                  D3DFVF_MIRRORVERTEX,
                                                  D3DPOOL_MANAGED, &m_pMirrorVB ) ) )
        return E_FAIL;

    // Initialize the mirror's vertices
    MIRRORVERTEX* v;
    m_pMirrorVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p = D3DXVECTOR3(-1.5f, 1.5f, 3.0f );
    v[2].p = D3DXVECTOR3(-1.5f,-1.5f, 3.0f );
    v[1].p = D3DXVECTOR3( 1.5f, 1.5f, 3.0f );
    v[3].p = D3DXVECTOR3( 1.5f,-1.5f, 3.0f );
    v[0].n     = v[1].n     = v[2].n     = v[3].n     = D3DXVECTOR3(0.0f,0.0f,-1.0f);
    v[0].color = v[1].color = v[2].color = v[3].color = 0x80ffffff;
    m_pMirrorVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Set up the geometry objects
    m_pTeapot->RestoreDeviceObjects( m_pd3dDevice );

    // Set up the textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );

    // Set up the matrices
    D3DXMATRIX matWorld, matView, matProj;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &m_vEyePt, &m_vLookatPt, &m_vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set up a light
    if( ( m_d3dCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS ) ||
       !( m_dwCreateFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) )
    {
        D3DLIGHT8 light;
        D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.2f, -1.0f, -0.2f );
        m_pd3dDevice->SetLight( 0, &light );
        m_pd3dDevice->LightEnable( 0, TRUE );
    }
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xff555555 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pTeapot->InvalidateDeviceObjects();

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
    m_pTeapot->Destroy();

    SAFE_RELEASE( m_pMirrorVB );

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

    if( ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->MaxUserClipPlanes < 4 )
            return E_FAIL;
    }

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
    SAFE_DELETE( m_pTeapot );

    return S_OK;
}





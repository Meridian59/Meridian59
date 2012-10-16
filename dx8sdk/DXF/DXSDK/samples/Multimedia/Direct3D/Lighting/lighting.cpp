//-----------------------------------------------------------------------------
// File: Lighting.cpp
//
// Desc: Example code showing how to use D3D lights.
//
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include <tchar.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------

// Custom D3D vertex format used by the vertex buffer
struct MYVERTEX
{
    D3DXVECTOR3 p;       // vertex position
    D3DXVECTOR3 n;       // vertex normal
};

#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)



//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    LPDIRECT3DVERTEXBUFFER8 m_pWallVB; // Vertex buffer for the walls and floor
    LPD3DXMESH m_pSphereMesh;          // Representation of point light
    LPD3DXMESH m_pConeMesh;            // Representation of dir/spot light
    CD3DFont* m_pFont;                 // Font for drawing text
    D3DLIGHT8 m_light;                 // Description of the D3D light
    UINT m_n;                          // Number of vertices in the ground grid along X
    UINT m_m;                          // Number of vertices in the ground grid along Z
    UINT m_nTriangles;                 // Number of triangles in the ground grid

protected:
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT FrameMove();
    HRESULT Render();
    HRESULT FinalCleanup();

public:
    CMyD3DApplication();
};


CMyD3DApplication g_d3dApp;


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    if( FAILED( g_d3dApp.Create( hInst ) ) )
        return 0;

    return g_d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle            = TEXT( "Lighting" );
    m_bUseDepthBuffer           = FALSE;

    m_pFont                     = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pWallVB                   = NULL;
    m_pSphereMesh               = NULL;
    m_pConeMesh                 = NULL;

    // Set up wall/floor mesh resolution.  Try changing m_n and m_m to see
    // how the lighting is affected.
    m_n = 32;
    m_m = 32;
    m_nTriangles = (m_n-1)*(m_m-1)*2;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    HRESULT hr;

    if( FAILED( hr = m_pFont->InitDeviceObjects( m_pd3dDevice ) ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restores scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    MYVERTEX* v;

    // Create a square grid m_n*m_m for rendering the wall
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_nTriangles*3*sizeof(MYVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX,
                                                  D3DPOOL_MANAGED, &m_pWallVB ) ) )
    {
        return E_FAIL;
    }

    // Fill in the grid vertex data
    m_pWallVB->Lock( 0, 0, (BYTE**)&v, 0 );
    float dX = 1.0f/(m_n-1);
    float dZ = 1.0f/(m_m-1);
    float dU = 1.0f/(m_n-1);
    float dV = 1.0f/(m_m-1);
    UINT k = 0;
    for (UINT z=0; z < (m_m-1); z++)
    {
        for (UINT x=0; x < (m_n-1); x++)
        {
            v[k].p  = D3DXVECTOR3(10 * x*dX, 0.0f, 10 * z*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
            v[k].p  = D3DXVECTOR3(10 * x*dX, 0.0f, 10 * (z+1)*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
            v[k].p  = D3DXVECTOR3(10 * (x+1)*dX, 0.0f, 10 * (z+1)*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
            v[k].p  = D3DXVECTOR3(10 * x*dX, 0.0f, 10 * z*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
            v[k].p  = D3DXVECTOR3(10 * (x+1)*dX, 0.0f, 10 * (z+1)*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
            v[k].p  = D3DXVECTOR3(10 * (x+1)*dX, 0.0f, 10 * z*dZ );
            v[k].n  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            k++;
        }
    }
    m_pWallVB->Unlock();

    // Create sphere and cone meshes to represent the lights
    if (FAILED( D3DXCreateSphere(m_pd3dDevice, 0.25f, 20, 20, &m_pSphereMesh, NULL) ) )
        return E_FAIL;

    if (FAILED( D3DXCreateCylinder(m_pd3dDevice, 0.0f, 0.25f, 0.5f, 20, 20, &m_pConeMesh, NULL) ) )
        return E_FAIL;

    // Set up a material
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

    // Set the world matrix
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity( &matIdentity );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,  &matIdentity );

    // Set the view matrix.
    D3DXMATRIX matView;
    D3DXVECTOR3 vFromPt( -10, 10, -10);
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Set the projection matrix
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Turn on lighting.
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Enable ambient lighting to a dim, grey light, so objects that
    // are not lit by the other lights are not completely black
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 
        D3DCOLOR_COLORVALUE( 0.25, 0.25, 0.25, 1.0 ) );

    // Set light #0 to be a simple, faint grey directional light so 
    // the walls and floor are slightly different shades of grey
    D3DLIGHT8 light;                 // Description of the D3D light
    ZeroMemory( &light, sizeof(light) );
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Direction = D3DXVECTOR3( 0.3f, -0.5f, 0.2f );
    light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = 0.25f;
    m_pd3dDevice->SetLight( 0, &light );

    // Set light #1 to be a simple, bright directional light to use 
    // on the mesh representing light #2
    ZeroMemory( &light, sizeof(light) );
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Direction = D3DXVECTOR3( 0.5f, -0.5f, 0.5f );
    light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = 1.0f;
    m_pd3dDevice->SetLight( 1, &light );

    // Light #2 will be the light used to light the floor and walls.  It will
    // be set up in FrameMove() since it changes every frame.

    // Restore the font
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    ZeroMemory( &m_light, sizeof(m_light) );

    // Rotate through the various light types
    m_light.Type = (D3DLIGHTTYPE)(1+(((DWORD)m_fTime)/5)%3);

    // Make sure the light type is supported by the device.  If 
    // D3DVTXPCAPS_POSITIONALLIGHTS is not set, the device does not support 
    // point or spot lights, so change light #2's type to a directional light.
    DWORD dwCaps = m_d3dCaps.VertexProcessingCaps;
    if( 0 == ( dwCaps & D3DVTXPCAPS_POSITIONALLIGHTS ) )
    {
        if( m_light.Type == D3DLIGHT_POINT || m_light.Type == D3DLIGHT_SPOT )
            m_light.Type = D3DLIGHT_DIRECTIONAL;
    }

    // Values for the light position, direction, and color
    FLOAT x = sinf( m_fTime*2.000f );
    FLOAT y = sinf( m_fTime*2.246f );
    FLOAT z = sinf( m_fTime*2.640f );

    m_light.Diffuse.r  = 0.5f + 0.5f * x;
    m_light.Diffuse.g  = 0.5f + 0.5f * y;
    m_light.Diffuse.b  = 0.5f + 0.5f * z;
    m_light.Range      = 100.0f;
    
    switch( m_light.Type )
    {
        case D3DLIGHT_POINT:
            m_light.Position     = 4.5f * D3DXVECTOR3( x, y, z );
            m_light.Attenuation1 = 0.4f;
            break;
        case D3DLIGHT_DIRECTIONAL:
            m_light.Direction    = D3DXVECTOR3( x, y, z );
            break;
        case D3DLIGHT_SPOT:
            m_light.Position     = 2.0f * D3DXVECTOR3( x, y, z );
            m_light.Direction    = D3DXVECTOR3( x, y, z );
            m_light.Theta        = 0.5f;
            m_light.Phi          = 1.0f;
            m_light.Falloff      = 1.0f;
            m_light.Attenuation0 = 1.0f;
    }
    m_pd3dDevice->SetLight( 2, &m_light );

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
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0x000000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        m_pd3dDevice->SetStreamSource( 0, m_pWallVB, sizeof(MYVERTEX) );
        m_pd3dDevice->SetVertexShader( D3DFVF_MYVERTEX );

        D3DXMATRIX matWorld;
        D3DXMATRIX matTrans;
        D3DXMATRIX matRotate;

        // Turn on light #0 and #2, and turn off light #1
        m_pd3dDevice->LightEnable( 0, TRUE );
        m_pd3dDevice->LightEnable( 1, FALSE );
        m_pd3dDevice->LightEnable( 2, TRUE );

        // Draw the floor
        D3DXMatrixTranslation( &matTrans, -5.0f, -5.0f, -5.0f );
        D3DXMatrixRotationZ( &matRotate, 0.0f );
        matWorld = matRotate * matTrans;
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_nTriangles );

        // Draw the back wall
        D3DXMatrixTranslation( &matTrans, 5.0f,-5.0f, -5.0f );
        D3DXMatrixRotationZ( &matRotate, D3DX_PI/2 );
        matWorld = matRotate * matTrans;
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_nTriangles );

        // Draw the side wall
        D3DXMatrixTranslation( &matTrans, -5.0f, -5.0f, 5.0f );
        D3DXMatrixRotationX( &matRotate,  -D3DX_PI/2 );
        matWorld = matRotate * matTrans;
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_nTriangles );

        // Turn on light #1, and turn off light #0 and #2
        m_pd3dDevice->LightEnable( 0, FALSE );
        m_pd3dDevice->LightEnable( 1, TRUE );
        m_pd3dDevice->LightEnable( 2, FALSE );

        // Draw the mesh representing the light
        if( m_light.Type == D3DLIGHT_POINT )
        {
            // Just position the point light -- no need to orient it
            D3DXMatrixTranslation( &matWorld, m_light.Position.x, 
                m_light.Position.y, m_light.Position.z );
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
            m_pSphereMesh->DrawSubset(0);
        }
        else
        {
            // Position the light and point it in the light's direction
            D3DXVECTOR3 vecFrom( m_light.Position.x, m_light.Position.y, m_light.Position.z );
            D3DXVECTOR3 vecAt( m_light.Position.x + m_light.Direction.x, 
                               m_light.Position.y + m_light.Direction.y,
                               m_light.Position.z + m_light.Direction.z );
            D3DXVECTOR3 vecUp( 0, 1, 0);
            D3DXMATRIX matWorldInv;
            D3DXMatrixLookAtLH( &matWorldInv, &vecFrom, &vecAt, &vecUp);
            D3DXMatrixInverse( &matWorld, NULL, &matWorldInv);
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
            m_pConeMesh->DrawSubset(0);
        }

        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
        TCHAR* strLight = (m_light.Type == D3DLIGHT_POINT ? TEXT("Point Light") : 
            m_light.Type == D3DLIGHT_SPOT ? TEXT("Spot Light") : TEXT("Directional Light"));
        m_pFont->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,255,255), strLight);

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    SAFE_RELEASE( m_pWallVB );
    SAFE_RELEASE( m_pSphereMesh );
    SAFE_RELEASE( m_pConeMesh );
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
    SAFE_RELEASE( m_pWallVB );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // Cleanup D3D font
    SAFE_DELETE( m_pFont );

    return S_OK;
}

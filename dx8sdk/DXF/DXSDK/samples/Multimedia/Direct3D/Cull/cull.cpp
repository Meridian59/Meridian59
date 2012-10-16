//-----------------------------------------------------------------------------
// File: Cull.cpp
//
// Desc: Shows a technique for culling objects whose bounding boxes are 
//       outside the view frustum.  This technique is described at:
//       http://www.cs.unc.edu/~hoff/research/vfculler/viewcull.html
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <math.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"


//-----------------------------------------------------------------------------
// Name: enum CULLSTATE
// Desc: Represents the result of the culling calculation on an object.
//-----------------------------------------------------------------------------
enum CULLSTATE
{
    CS_UNKNOWN,      // cull state not yet computed
    CS_INSIDE,       // object bounding box is at least partly inside the frustum
    CS_OUTSIDE,      // object bounding box is outside the frustum
    CS_INSIDE_SLOW,  // OBB is inside frustum, but it took extensive testing to determine this
    CS_OUTSIDE_SLOW, // OBB is outside frustum, but it took extensive testing to determine this
};


//-----------------------------------------------------------------------------
// Name: struct CULLINFO
// Desc: Stores information that will be used when culling objects.  It needs
//       to be recomputed whenever the view matrix or projection matrix changes.
//-----------------------------------------------------------------------------
struct CULLINFO
{
    D3DXVECTOR3 vecFrustum[8];    // corners of the view frustum
    D3DXPLANE planeFrustum[6];    // planes of the view frustum
};


// Prototypes for the culling functions
VOID UpdateCullInfo( CULLINFO* pCullInfo, D3DXMATRIX* pMatView, D3DXMATRIX* pMatProj );
CULLSTATE CullObject( CULLINFO* pCullInfo, D3DXVECTOR3* pVecBounds, D3DXPLANE* pPlaneBounds );
BOOL EdgeIntersectsFace( D3DXVECTOR3* pEdges, D3DXVECTOR3* pFaces, D3DXPLANE* pPlane );


//-----------------------------------------------------------------------------
// Name: struct PLANEVERTEX
// Desc: Custom vertex type used for drawing the frustum planes
//-----------------------------------------------------------------------------
struct PLANEVERTEX
{
    D3DXVECTOR3 p;
    DWORD       color;
};

#define D3DFVF_PLANEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)


//-----------------------------------------------------------------------------
// Name: class CCullableThing
// Desc: A cullable object
//-----------------------------------------------------------------------------
class CCullableThing
{
public:
    D3DXVECTOR3  m_pos;       // origin of object
    FLOAT        m_fRotX;     // rotation of object around X axis
    FLOAT        m_fRotY;     // rotation of object around Y axis
    D3DXMATRIX   m_mat;       // object's local-to-world transformation
    D3DXVECTOR3  m_vecBoundsLocal[8];   // bounding box coordinates (in local coord space)
    D3DXVECTOR3  m_vecBoundsWorld[8];   // bounding box coordinates (in world coord space)
    D3DXPLANE    m_planeBoundsWorld[6]; // bounding box planes (in world coord space)
    CULLSTATE    m_cullstate; // whether object is in the view frustum

public:
    VOID Init(VOID)
    {
        // Pick a random position and orientation
        m_pos = D3DXVECTOR3( (FLOAT)(rand() % 50 - 25), // X is in (-25.0, 25.0)
                             (FLOAT)(rand() % 50 - 25), // Y is in (-25.0, 25.0)
                             (FLOAT)(rand() % 25) );    // Z is in (  0.0, 25.0)
        m_fRotX = D3DXToRadian(rand() % 360);
        m_fRotY = D3DXToRadian(rand() % 360);
        UpdateMatrix();
        m_cullstate = CS_UNKNOWN;
    }
    VOID UpdateMatrix(VOID)
    {
        // Recompute m_mat, m_vecBoundsWorld, and m_planeBoundsWorld 
        // when the thing's position, orientation, or bounding box has changed
        D3DXMATRIX matRotX, matRotY, matTrans;
        D3DXMatrixRotationX( &matRotX, m_fRotX );
        D3DXMatrixRotationY( &matRotY, m_fRotY );
        D3DXMatrixTranslation( &matTrans, m_pos.x, m_pos.y, m_pos.z );
        m_mat = matRotX * matRotY * matTrans;

        // Transform bounding box coords from local space to world space
        for( int i = 0; i < 8; i++ )
            D3DXVec3TransformCoord( &m_vecBoundsWorld[i], &m_vecBoundsLocal[i], &m_mat );

        // Determine planes of the bounding box
        D3DXPlaneFromPoints( &m_planeBoundsWorld[0], &m_vecBoundsWorld[0], 
            &m_vecBoundsWorld[1], &m_vecBoundsWorld[2] ); // Near
        D3DXPlaneFromPoints( &m_planeBoundsWorld[1], &m_vecBoundsWorld[6], 
            &m_vecBoundsWorld[7], &m_vecBoundsWorld[5] ); // Far
        D3DXPlaneFromPoints( &m_planeBoundsWorld[2], &m_vecBoundsWorld[2], 
            &m_vecBoundsWorld[6], &m_vecBoundsWorld[4] ); // Left
        D3DXPlaneFromPoints( &m_planeBoundsWorld[3], &m_vecBoundsWorld[7], 
            &m_vecBoundsWorld[3], &m_vecBoundsWorld[5] ); // Right
        D3DXPlaneFromPoints( &m_planeBoundsWorld[4], &m_vecBoundsWorld[2], 
            &m_vecBoundsWorld[3], &m_vecBoundsWorld[6] ); // Top
        D3DXPlaneFromPoints( &m_planeBoundsWorld[5], &m_vecBoundsWorld[1], 
            &m_vecBoundsWorld[0], &m_vecBoundsWorld[4] ); // Bottom
    }
};


//-----------------------------------------------------------------------------
// Name: struct Camera
// Desc: 
//-----------------------------------------------------------------------------
struct Camera
{
    D3DXVECTOR3        m_vPosition;
    D3DXVECTOR3        m_vVelocity;
    FLOAT              m_fYaw;
    FLOAT              m_fYawVelocity;
    FLOAT              m_fPitch;
    FLOAT              m_fPitchVelocity;
    D3DXMATRIX         m_matView;
    D3DXMATRIX         m_matOrientation;
};


//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*      m_pFont;       // Font for drawing text
    CD3DFont*      m_pFontSmall;

    LPD3DXMESH     m_pMeshTeapot; // Mesh of thing to be cull-tested
    LPD3DXMESH     m_pMeshBox;    // Mesh to visualize bounding box
    D3DXMATRIX     m_matBox;      // Matrix that places bounding box correctly on teapot

    LPDIRECT3DVERTEXBUFFER8 m_pPlaneVB[6]; // VBs to visualize the view frustum

    // The things to render and cull
    CCullableThing m_CullableThingArray[50];
    DWORD          m_dwNumCullableThings;

    // Variables for determining the view
    BYTE           m_bKey[256];
    BOOL           m_bLeftActive; // left vs right view currently active
    Camera         m_CameraLeft;
    Camera         m_CameraRight;
    D3DXMATRIX     m_matProjLeft;
    D3DXMATRIX     m_matProjRight;

    D3DMATERIAL8   m_mtrlOutside;
    D3DMATERIAL8   m_mtrlInside;
    D3DMATERIAL8   m_mtrlOutsideSlow;
    D3DMATERIAL8   m_mtrlInsideSlow;
    D3DMATERIAL8   m_mtrlWhite;

    CULLINFO       m_cullinfo;
    BOOL           m_bShowHelp;

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT RenderScene( BOOL bRenderPlanes );
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    VOID    UpdateCamera(Camera* pCamera);
    HRESULT UpdatePlaneVBs(VOID);
    VOID    CullObjects(VOID);

public:
    CMyD3DApplication();
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
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
    m_strWindowTitle    = _T("Cull: Culling nonvisible objects");
    m_bUseDepthBuffer   = TRUE;
    m_dwCreationWidth   = 600;
    m_dwCreationHeight  = 300;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pFontSmall        = new CD3DFont( _T("Arial"),  9, D3DFONT_BOLD );
    m_pMeshTeapot       = NULL;
    m_pMeshBox          = NULL;
    m_dwNumCullableThings = 0;

    ZeroMemory( m_pPlaneVB, sizeof(m_pPlaneVB) );
    ZeroMemory( m_bKey, 256 );
    ZeroMemory( &m_CameraLeft, sizeof(m_CameraLeft) );
    ZeroMemory( &m_CameraRight, sizeof(m_CameraRight) );

    m_bLeftActive = TRUE;
    m_bShowHelp = FALSE;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Initialize cullable things
    for( int i = 0; i < 50; i++ )
    {
        m_CullableThingArray[m_dwNumCullableThings].Init();
        m_dwNumCullableThings++;
    }

    UpdateCamera( &m_CameraLeft );
    UpdateCamera( &m_CameraRight );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    BOOL bNeedToCull = FALSE;

    // Handle object rotations
    if( m_bKey['Y'] || m_bKey['U'] || m_bKey['H'] || m_bKey['J'])
    {
        CCullableThing* pCullableThing;
        for( DWORD iThing = 0; iThing < m_dwNumCullableThings; iThing++ )
        {
            pCullableThing = &m_CullableThingArray[iThing];
            if( m_bKey['Y'] )
                pCullableThing->m_fRotY += m_fElapsedTime;
            else if( m_bKey['U'] )
                pCullableThing->m_fRotY -= m_fElapsedTime;
            if( m_bKey['H'] )
                pCullableThing->m_fRotX += m_fElapsedTime;
            else if( m_bKey['J'] )
                pCullableThing->m_fRotX -= m_fElapsedTime;
            pCullableThing->UpdateMatrix();
        }
        bNeedToCull = TRUE;
    }

    // Handle camera motion
    UpdateCamera(m_bLeftActive ? &m_CameraLeft : &m_CameraRight);
    if( !m_bLeftActive )
    {
        UpdateCullInfo( &m_cullinfo, &m_CameraRight.m_matView, &m_matProjRight );
        bNeedToCull = TRUE;
        UpdatePlaneVBs();
    }

    // Re-determine cull state of all objects if necessary
    if( bNeedToCull )
        CullObjects();

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
    // The "hot" clear color is used to indicate the viewport
    // whose camera is currently being controlled by the keyboard
    const DWORD dwHotClearColor = 0xff0000ff;
    const DWORD dwColdClearColor = 0xff000080;

    //
    // Draw left viewport
    //
    D3DVIEWPORT8 vp;
    vp.X = 0;
    vp.Y = 0;
    vp.Width = m_d3dsdBackBuffer.Width / 2;
    vp.Height = m_d3dsdBackBuffer.Height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    m_pd3dDevice->SetViewport( &vp );

    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
        m_bLeftActive ? dwHotClearColor : dwColdClearColor, 1.0f, 0L );

    m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_CameraLeft.m_matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProjLeft );

    // Draw contents of left viewport
    RenderScene( TRUE ); // TRUE means render frustum planes

    //
    // Draw right viewport
    //
    vp.X = m_d3dsdBackBuffer.Width / 2;
    vp.Width = m_d3dsdBackBuffer.Width / 2;
    m_pd3dDevice->SetViewport( &vp );

    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
        m_bLeftActive ? dwColdClearColor : dwHotClearColor, 1.0f, 0L );

    m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_CameraRight.m_matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProjRight );

    // Draw contents of right viewport
    RenderScene( FALSE ); // FALSE means don't render frustum planes

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderScene()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderScene( BOOL bRenderPlanes )
{
    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Render each object
        CCullableThing* pCullableThing;
        for( DWORD iThing = 0; iThing < m_dwNumCullableThings; iThing++ )
        {
            pCullableThing = &m_CullableThingArray[iThing];

            // Normally, if the cullstate is CS_OUTSIDE or CS_OUTSIDE_SLOW,
            // there is no need to ask D3D to render the object since
            // it's outside the view frustum.  Since this app is meant to
            // visualize the culling process, all objects are passed to D3D,
            // and different colors are used to show their cullstates.

            switch( pCullableThing->m_cullstate )
            {
            case CS_UNKNOWN:
                m_pd3dDevice->SetMaterial( &m_mtrlWhite );
                break;
            case CS_OUTSIDE:
                m_pd3dDevice->SetMaterial( &m_mtrlOutside );
                break;
            case CS_INSIDE:
                m_pd3dDevice->SetMaterial( &m_mtrlInside );
                break;
            case CS_OUTSIDE_SLOW:
                m_pd3dDevice->SetMaterial( &m_mtrlOutsideSlow );
                break;
            case CS_INSIDE_SLOW:
                m_pd3dDevice->SetMaterial( &m_mtrlInsideSlow );
                break;
            }
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &pCullableThing->m_mat );
            m_pMeshTeapot->DrawSubset( 0 );
            D3DXMATRIX mat = m_matBox * pCullableThing->m_mat;
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat );
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
            m_pMeshBox->DrawSubset( 0 );
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
        }

        if( bRenderPlanes )
        {
            // Render frustum planes
            m_pd3dDevice->SetMaterial( &m_mtrlWhite );
            D3DXMATRIX mat;
            D3DXMatrixIdentity( &mat );
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat );
            m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
            m_pd3dDevice->SetVertexShader( D3DFVF_PLANEVERTEX );
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
            for( int iPlane = 0; iPlane < 6; iPlane++ )
            {
                m_pd3dDevice->SetStreamSource( 0, m_pPlaneVB[iPlane], sizeof(PLANEVERTEX) );
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
            }
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
            m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        }

        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
        
        // Show help
        if( m_bShowHelp )
        {
            m_pFontSmall->DrawText(  2, 40, D3DCOLOR_ARGB(255,200,100,100),
                                    _T("Press F1 to hide help\n")
                                    _T("The right viewport is the frustum being culled against.\n")
                                    _T("The left viewport has its own camera that lets you view\n")
                                    _T("the scene and frustum from different angles.\n")
                                    _T("Click the viewport whose camera you want to control.\n")
                                    _T("Keyboard controls:") );
            m_pFontSmall->DrawText( 20, 140, D3DCOLOR_ARGB(255,200,100,100),
                                    _T("Move\n")
                                    _T("Turn\n")
                                    _T("Spin objects\n")
                                    _T("Snap left view to viewport\n")
                                    _T("Snap right view to origin\n") );
            m_pFontSmall->DrawText( 210, 140, D3DCOLOR_ARGB(255,200,100,100),
                                    _T("W, S, Arrow keys\n")
                                    _T("Q, E, A, Z\n")
                                    _T("Y, U, H, J\n")
                                    _T("N\n")
                                    _T("M") );
        }
        else
        {
            m_pFontSmall->DrawText(  2, 40, D3DCOLOR_ARGB(255,200,100,100), 
                               _T("Press F1 for help") );
        }

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateCamera()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::UpdateCamera(Camera* pCamera)
{
    FLOAT fElapsedTime;

    if( m_fElapsedTime > 0.0f )
        fElapsedTime = m_fElapsedTime;
    else
        fElapsedTime = 0.05f;

    FLOAT fSpeed        = 5.0f*fElapsedTime;
    FLOAT fAngularSpeed = 2.0f*fElapsedTime;

    // De-accelerate the camera movement (for smooth motion)
    pCamera->m_vVelocity      *= 0.75f;
    pCamera->m_fYawVelocity   *= 0.75f;
    pCamera->m_fPitchVelocity *= 0.75f;

    // Process keyboard input
    if( m_bKey[VK_RIGHT] )    pCamera->m_vVelocity.x    += fSpeed; // Slide Right
    if( m_bKey[VK_LEFT] )     pCamera->m_vVelocity.x    -= fSpeed; // Slide Left
    if( m_bKey[VK_UP] )       pCamera->m_vVelocity.y    += fSpeed; // Slide Up
    if( m_bKey[VK_DOWN] )     pCamera->m_vVelocity.y    -= fSpeed; // Slide Down
    if( m_bKey['W'] )         pCamera->m_vVelocity.z    += fSpeed; // Move Forward
    if( m_bKey['S'] )         pCamera->m_vVelocity.z    -= fSpeed; // Move Backward
    if( m_bKey['E'] )         pCamera->m_fYawVelocity   += fSpeed; // Turn Right
    if( m_bKey['Q'] )         pCamera->m_fYawVelocity   -= fSpeed; // Turn Left
    if( m_bKey['Z'] )         pCamera->m_fPitchVelocity += fSpeed; // Turn Down
    if( m_bKey['A'] )         pCamera->m_fPitchVelocity -= fSpeed; // Turn Up

    // Update the position vector
    D3DXVECTOR3 vT = pCamera->m_vVelocity * fSpeed;
    D3DXVec3TransformNormal( &vT, &vT, &pCamera->m_matOrientation );
    pCamera->m_vPosition += vT;

    // Update the yaw-pitch-rotation vector
    pCamera->m_fYaw   += fAngularSpeed * pCamera->m_fYawVelocity;
    pCamera->m_fPitch += fAngularSpeed * pCamera->m_fPitchVelocity;
    if( pCamera->m_fPitch < -D3DX_PI/2 ) 
        pCamera->m_fPitch = -D3DX_PI/2;
    if( pCamera->m_fPitch > D3DX_PI/2 ) 
        pCamera->m_fPitch = D3DX_PI/2;

    // Set the view matrix
    D3DXQUATERNION qR;
    D3DXQuaternionRotationYawPitchRoll( &qR, pCamera->m_fYaw, pCamera->m_fPitch, 0.0f );
    D3DXMatrixAffineTransformation( &pCamera->m_matOrientation, 1.25f, NULL, &qR, &pCamera->m_vPosition );
    D3DXMatrixInverse( &pCamera->m_matView, NULL, &pCamera->m_matOrientation );
}




//-----------------------------------------------------------------------------
// Name: UpdatePlaneVBs()
// Desc: Update the vertex buffers to match the view frustum.  Slightly 
//       different colors are used for each plane to make them easier to
//       distinguish from each other.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::UpdatePlaneVBs()
{
    HRESULT hr;
    for( int iPlane = 0; iPlane < 6; iPlane++ )
    {
        PLANEVERTEX* v;
        if( FAILED(hr = m_pPlaneVB[iPlane]->Lock( 0, 0, (BYTE**)&v, 0 ) ) )
            return hr;
        switch( iPlane )
        {
        case 0: // near
            v[0].p = m_cullinfo.vecFrustum[0];
            v[1].p = m_cullinfo.vecFrustum[1];
            v[2].p = m_cullinfo.vecFrustum[2];
            v[3].p = m_cullinfo.vecFrustum[3];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80505050;
            break;
        case 1: // far
            v[0].p = m_cullinfo.vecFrustum[4];
            v[1].p = m_cullinfo.vecFrustum[6];
            v[2].p = m_cullinfo.vecFrustum[5];
            v[3].p = m_cullinfo.vecFrustum[7];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80404040;
            break;
        case 2: // left
            v[0].p = m_cullinfo.vecFrustum[0];
            v[1].p = m_cullinfo.vecFrustum[2];
            v[2].p = m_cullinfo.vecFrustum[4];
            v[3].p = m_cullinfo.vecFrustum[6];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80404040;
            break;
        case 3:
            v[0].p = m_cullinfo.vecFrustum[1];
            v[1].p = m_cullinfo.vecFrustum[3];
            v[2].p = m_cullinfo.vecFrustum[5];
            v[3].p = m_cullinfo.vecFrustum[7];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80606060;
            break;
        case 4:
            v[0].p = m_cullinfo.vecFrustum[2];
            v[1].p = m_cullinfo.vecFrustum[3];
            v[2].p = m_cullinfo.vecFrustum[6];
            v[3].p = m_cullinfo.vecFrustum[7];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80505050;
            break;
        case 5:
            v[0].p = m_cullinfo.vecFrustum[0];
            v[1].p = m_cullinfo.vecFrustum[1];
            v[2].p = m_cullinfo.vecFrustum[4];
            v[3].p = m_cullinfo.vecFrustum[5];
            v[0].color = v[1].color = v[2].color = v[3].color = 0x80505050;
            break;
        }
        m_pPlaneVB[iPlane]->Unlock();
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

    // Initialize the font's internal textures
    if( FAILED( hr = m_pFont->InitDeviceObjects( m_pd3dDevice ) ) )
        return hr;
    if( FAILED( hr = m_pFontSmall->InitDeviceObjects( m_pd3dDevice ) ) )
        return hr;

    // Create a teapot mesh
    if( FAILED( D3DXCreateTeapot( m_pd3dDevice, &m_pMeshTeapot, NULL ) ) )
        return hr;
    D3DXComputeNormals( m_pMeshTeapot, NULL );

    // Determine bounding box of the teapot
    D3DXVECTOR3 vecMin;
    D3DXVECTOR3 vecMax;
    BYTE* pVertices;
    if( FAILED( hr = m_pMeshTeapot->LockVertexBuffer(D3DLOCK_READONLY, &pVertices) ) )
        return hr;
    hr = D3DXComputeBoundingBox( pVertices, m_pMeshTeapot->GetNumVertices(), 
        m_pMeshTeapot->GetFVF(), &vecMin, &vecMax );
    m_pMeshTeapot->UnlockVertexBuffer();

    // Note that the m_vecBoundsLocal are identical for every CullableThing
    // because the cullable things are all the same teapot object.  In a real 
    // app, of course, they would all be potentially different.
    CCullableThing* pCullableThing;
    for( DWORD iThing = 0; iThing < m_dwNumCullableThings; iThing++ )
    {
        pCullableThing = &m_CullableThingArray[iThing];
        pCullableThing->m_vecBoundsLocal[0] = D3DXVECTOR3( vecMin.x, vecMin.y, vecMin.z ); // xyz
        pCullableThing->m_vecBoundsLocal[1] = D3DXVECTOR3( vecMax.x, vecMin.y, vecMin.z ); // Xyz
        pCullableThing->m_vecBoundsLocal[2] = D3DXVECTOR3( vecMin.x, vecMax.y, vecMin.z ); // xYz
        pCullableThing->m_vecBoundsLocal[3] = D3DXVECTOR3( vecMax.x, vecMax.y, vecMin.z ); // XYz
        pCullableThing->m_vecBoundsLocal[4] = D3DXVECTOR3( vecMin.x, vecMin.y, vecMax.z ); // xyZ
        pCullableThing->m_vecBoundsLocal[5] = D3DXVECTOR3( vecMax.x, vecMin.y, vecMax.z ); // XyZ
        pCullableThing->m_vecBoundsLocal[6] = D3DXVECTOR3( vecMin.x, vecMax.y, vecMax.z ); // xYZ
        pCullableThing->m_vecBoundsLocal[7] = D3DXVECTOR3( vecMax.x, vecMax.y, vecMax.z ); // XYZ
        pCullableThing->UpdateMatrix();
    }

    // Set up m_matBox to be able to render the bounding box properly
    D3DXMATRIX matTrans;
    D3DXMATRIX matScale;
    D3DXMatrixTranslation( &matTrans, (vecMin.x + vecMax.x) / 2, 
        (vecMin.y + vecMax.y) / 2, (vecMin.z + vecMax.z) / 2 );
    D3DXMatrixScaling( &matScale, vecMax.x - vecMin.x, vecMax.y - vecMin.y, vecMax.z - vecMin.z );
    m_matBox = matScale * matTrans;

    if( FAILED( hr = D3DXCreateBox( m_pd3dDevice, 1.0f, 1.0f, 1.0f, &m_pMeshBox, NULL ) ) )
        return hr;
    D3DXComputeNormals( m_pMeshBox, NULL );

    // Create VBs for frustum planes
    for( int iPlane = 0; iPlane < 6; iPlane++ )
    {
        if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( 4*sizeof(PLANEVERTEX),
            D3DUSAGE_WRITEONLY, D3DFVF_PLANEVERTEX, D3DPOOL_MANAGED, &m_pPlaneVB[iPlane] ) ) )
        {
            return hr;
        }
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
    m_pFontSmall->RestoreDeviceObjects();

    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );

    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,       D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,      D3DBLEND_INVSRCALPHA );

    // Set up the matrices
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &m_matProjRight, D3DX_PI/4, fAspect, 0.2f, 10.0f );
    D3DXMatrixPerspectiveFovLH( &m_matProjLeft, D3DX_PI/4, fAspect, 0.2f, 200.0f );

    // Set up a light
    if( ( m_d3dCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS ) ||
       !( m_dwCreateFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) )
    {
        D3DLIGHT8 light;
        D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.2f, -0.4f, 0.2f );
        m_pd3dDevice->SetLight( 0, &light );
        m_pd3dDevice->LightEnable( 0, TRUE );
    }
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xff555555 );

    // Set up materials
    D3DUtil_InitMaterial(m_mtrlInside,      0.2f, 0.6f, 0.2f, 0.5f); // dark green
    D3DUtil_InitMaterial(m_mtrlOutside,     0.6f, 0.2f, 0.2f, 0.5f); // dark red
    D3DUtil_InitMaterial(m_mtrlInsideSlow,  0.7f, 1.0f, 0.7f, 0.5f); // pastel green
    D3DUtil_InitMaterial(m_mtrlOutsideSlow, 1.0f, 0.7f, 0.7f, 0.5f); // pastel red
    D3DUtil_InitMaterial(m_mtrlWhite,       1.0f, 1.0f, 1.0f, 0.5f); // white

    UpdateCullInfo( &m_cullinfo, &m_CameraRight.m_matView, &m_matProjRight );
    CullObjects();
    UpdatePlaneVBs();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pFontSmall->InvalidateDeviceObjects();

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
    m_pFontSmall->DeleteDeviceObjects();
    SAFE_RELEASE( m_pMeshTeapot );
    SAFE_RELEASE( m_pMeshBox );

    for( int iPlane = 0; iPlane < 6; iPlane++ )
        SAFE_RELEASE( m_pPlaneVB[iPlane] );

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
    SAFE_DELETE( m_pFontSmall );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Record key presses
    if( WM_KEYDOWN == uMsg )
    {
        m_bKey[wParam] = 1;
    }
    // Perform commands when keys are released
    if( WM_KEYUP == uMsg )
    {
        m_bKey[wParam] = 0;
        switch( wParam )
        {
        case 'N': // Set left camera to match right camera
            m_CameraLeft = m_CameraRight;
            break;
        case 'M': // Reset right camera to original orientation
            ZeroMemory( &m_CameraRight, sizeof(m_CameraRight) );
            UpdateCamera( &m_CameraRight );
            UpdateCullInfo( &m_cullinfo, &m_CameraRight.m_matView, &m_matProjRight );
            CullObjects();
            UpdatePlaneVBs();
            break;
        case VK_F1:
            m_bShowHelp = !m_bShowHelp;
            break;
        }
    }
    if( WM_LBUTTONDOWN == uMsg )
    {
        INT x = LOWORD(lParam);
        if( x < (INT)m_d3dsdBackBuffer.Width / 2 )
            m_bLeftActive = TRUE;
        else
            m_bLeftActive = FALSE;
    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: CullObjects()
// Desc: Cull each object in the CCullableThing array
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::CullObjects(VOID)
{
    CCullableThing* pCullableThing;

    for( DWORD iThing = 0; iThing < m_dwNumCullableThings; iThing++ )
    {
        pCullableThing = &m_CullableThingArray[iThing];
        pCullableThing->m_cullstate = CullObject( &m_cullinfo, 
            pCullableThing->m_vecBoundsWorld, pCullableThing->m_planeBoundsWorld );
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateCullInfo()
// Desc: Sets up the frustum planes, endpoints, and center for the frustum
//       defined by a given view matrix and projection matrix.  This info will 
//       be used when culling each object in CullObject().
//-----------------------------------------------------------------------------
VOID UpdateCullInfo( CULLINFO* pCullInfo, D3DXMATRIX* pMatView, D3DXMATRIX* pMatProj )
{
    D3DXMATRIX mat;

    D3DXMatrixMultiply( &mat, pMatView, pMatProj );
    D3DXMatrixInverse( &mat, NULL, &mat );

    pCullInfo->vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  0.0f); // xyz
    pCullInfo->vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  0.0f); // Xyz
    pCullInfo->vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  0.0f); // xYz
    pCullInfo->vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  0.0f); // XYz
    pCullInfo->vecFrustum[4] = D3DXVECTOR3(-1.0f, -1.0f,  1.0f); // xyZ
    pCullInfo->vecFrustum[5] = D3DXVECTOR3( 1.0f, -1.0f,  1.0f); // XyZ
    pCullInfo->vecFrustum[6] = D3DXVECTOR3(-1.0f,  1.0f,  1.0f); // xYZ
    pCullInfo->vecFrustum[7] = D3DXVECTOR3( 1.0f,  1.0f,  1.0f); // XYZ

    for( INT i = 0; i < 8; i++ )
        D3DXVec3TransformCoord( &pCullInfo->vecFrustum[i], &pCullInfo->vecFrustum[i], &mat );

    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[0], &pCullInfo->vecFrustum[0], 
        &pCullInfo->vecFrustum[1], &pCullInfo->vecFrustum[2] ); // Near
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[1], &pCullInfo->vecFrustum[6], 
        &pCullInfo->vecFrustum[7], &pCullInfo->vecFrustum[5] ); // Far
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[2], &pCullInfo->vecFrustum[2], 
        &pCullInfo->vecFrustum[6], &pCullInfo->vecFrustum[4] ); // Left
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[3], &pCullInfo->vecFrustum[7], 
        &pCullInfo->vecFrustum[3], &pCullInfo->vecFrustum[5] ); // Right
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[4], &pCullInfo->vecFrustum[2], 
        &pCullInfo->vecFrustum[3], &pCullInfo->vecFrustum[6] ); // Top
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[5], &pCullInfo->vecFrustum[1], 
        &pCullInfo->vecFrustum[0], &pCullInfo->vecFrustum[4] ); // Bottom
}




//-----------------------------------------------------------------------------
// Name: CullObject()
// Desc: Determine the cullstate for an object bounding box (OBB).  
//       The algorithm is:
//       1) If any OBB corner pt is inside the frustum, return CS_INSIDE
//       2) Else if all OBB corner pts are outside a single frustum plane, 
//          return CS_OUTSIDE
//       3) Else if any frustum edge penetrates a face of the OBB, return 
//          CS_INSIDE_SLOW
//       4) Else if any OBB edge penetrates a face of the frustum, return
//          CS_INSIDE_SLOW
//       5) Else if any point in the frustum is outside any plane of the 
//          OBB, return CS_OUTSIDE_SLOW
//       6) Else return CS_INSIDE_SLOW
//-----------------------------------------------------------------------------
CULLSTATE CullObject( CULLINFO* pCullInfo, D3DXVECTOR3* pVecBounds, 
                      D3DXPLANE* pPlaneBounds )
{
    BYTE bOutside[8];
    ZeroMemory( &bOutside, sizeof(bOutside) );

    // Check boundary vertices against all 6 frustum planes, 
    // and store result (1 if outside) in a bitfield
    for( int iPoint = 0; iPoint < 8; iPoint++ )
    {
        for( int iPlane = 0; iPlane < 6; iPlane++ )
        {
            if( pCullInfo->planeFrustum[iPlane].a * pVecBounds[iPoint].x +
                pCullInfo->planeFrustum[iPlane].b * pVecBounds[iPoint].y +
                pCullInfo->planeFrustum[iPlane].c * pVecBounds[iPoint].z +
                pCullInfo->planeFrustum[iPlane].d < 0)
            {
                bOutside[iPoint] |= (1 << iPlane);
            }
        }
        // If any point is inside all 6 frustum planes, it is inside
        // the frustum, so the object must be rendered.
        if( bOutside[iPoint] == 0 )
            return CS_INSIDE;
    }

    // If all points are outside any single frustum plane, the object is
    // outside the frustum
    if( (bOutside[0] & bOutside[1] & bOutside[2] & bOutside[3] & 
        bOutside[4] & bOutside[5] & bOutside[6] & bOutside[7]) != 0 )
    {
        return CS_OUTSIDE;
    }

    // Now see if any of the frustum edges penetrate any of the faces of
    // the bounding box
    D3DXVECTOR3 edge[12][2] = 
    {
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[1], // front bottom
        pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[3], // front top
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[2], // front left
        pCullInfo->vecFrustum[1], pCullInfo->vecFrustum[3], // front right
        pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[5], // back bottom
        pCullInfo->vecFrustum[6], pCullInfo->vecFrustum[7], // back top
        pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[6], // back left
        pCullInfo->vecFrustum[5], pCullInfo->vecFrustum[7], // back right
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[4], // left bottom
        pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[6], // left top
        pCullInfo->vecFrustum[1], pCullInfo->vecFrustum[5], // right bottom
        pCullInfo->vecFrustum[3], pCullInfo->vecFrustum[7], // right top
    };
    D3DXVECTOR3 face[6][4] =
    {
        pVecBounds[0], pVecBounds[2], pVecBounds[3], pVecBounds[1], // front
        pVecBounds[4], pVecBounds[5], pVecBounds[7], pVecBounds[6], // back
        pVecBounds[0], pVecBounds[4], pVecBounds[6], pVecBounds[2], // left
        pVecBounds[1], pVecBounds[3], pVecBounds[7], pVecBounds[5], // right
        pVecBounds[2], pVecBounds[6], pVecBounds[7], pVecBounds[3], // top
        pVecBounds[0], pVecBounds[4], pVecBounds[5], pVecBounds[1], // bottom
    };
    D3DXVECTOR3* pEdge;
    D3DXVECTOR3* pFace;
    pEdge = &edge[0][0];
    for( INT iEdge = 0; iEdge < 12; iEdge++ )
    {
        pFace = &face[0][0];
        for( INT iFace = 0; iFace < 6; iFace++ )
        {
            if( EdgeIntersectsFace( pEdge, pFace, &pPlaneBounds[iFace] ) )
            {
                return CS_INSIDE_SLOW;
            }
            pFace += 4;
        }
        pEdge += 2;
    }

    // Now see if any of the bounding box edges penetrate any of the faces of
    // the frustum
    D3DXVECTOR3 edge2[12][2] = 
    {
        pVecBounds[0], pVecBounds[1], // front bottom
        pVecBounds[2], pVecBounds[3], // front top
        pVecBounds[0], pVecBounds[2], // front left
        pVecBounds[1], pVecBounds[3], // front right
        pVecBounds[4], pVecBounds[5], // back bottom
        pVecBounds[6], pVecBounds[7], // back top
        pVecBounds[4], pVecBounds[6], // back left
        pVecBounds[5], pVecBounds[7], // back right
        pVecBounds[0], pVecBounds[4], // left bottom
        pVecBounds[2], pVecBounds[6], // left top
        pVecBounds[1], pVecBounds[5], // right bottom
        pVecBounds[3], pVecBounds[7], // right top
    };
    D3DXVECTOR3 face2[6][4] =
    {
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[3], pCullInfo->vecFrustum[1], // front
        pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[5], pCullInfo->vecFrustum[7], pCullInfo->vecFrustum[6], // back
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[6], pCullInfo->vecFrustum[2], // left
        pCullInfo->vecFrustum[1], pCullInfo->vecFrustum[3], pCullInfo->vecFrustum[7], pCullInfo->vecFrustum[5], // right
        pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[6], pCullInfo->vecFrustum[7], pCullInfo->vecFrustum[3], // top
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[5], pCullInfo->vecFrustum[1], // bottom
    };
    pEdge = &edge2[0][0];
    for( iEdge = 0; iEdge < 12; iEdge++ )
    {
        pFace = &face2[0][0];
        for( INT iFace = 0; iFace < 6; iFace++ )
        {
            if( EdgeIntersectsFace( pEdge, pFace, &pCullInfo->planeFrustum[iFace] ) )
            {
                return CS_INSIDE_SLOW;
            }
            pFace += 4;
        }
        pEdge += 2;
    }

    // Now see if frustum is contained in bounding box
    // If any frustum corner point is outside any plane of the bounding box,
    // the frustum is not contained in the bounding box, so the object
    // is outside the frustum
    for( INT iPlane = 0; iPlane < 6; iPlane++ )
    {
        if( pPlaneBounds[iPlane].a * pCullInfo->vecFrustum[0].x +
            pPlaneBounds[iPlane].b * pCullInfo->vecFrustum[0].y +
            pPlaneBounds[iPlane].c * pCullInfo->vecFrustum[0].z +
            pPlaneBounds[iPlane].d  < 0 )
        {
            return CS_OUTSIDE_SLOW;
        }
    }

    // Bounding box must contain the frustum, so render the object
    return CS_INSIDE_SLOW;
}




//-----------------------------------------------------------------------------
// Name: EdgeIntersectsFace()
// Desc: Determine if the edge bounded by the two vectors in pEdges intersects
//       the quadrilateral described by the four vectors in pFacePoints.  
//       Note: pPlane could be derived from pFacePoints using 
//       D3DXPlaneFromPoints, but it is precomputed in advance for greater
//       speed.
//-----------------------------------------------------------------------------
BOOL EdgeIntersectsFace( D3DXVECTOR3* pEdges, D3DXVECTOR3* pFacePoints, 
                         D3DXPLANE* pPlane )
{
    // If both edge points are on the same side of the plane, the edge does
    // not intersect the face
    FLOAT fDist1;
    FLOAT fDist2;
    fDist1 = pPlane->a * pEdges[0].x + pPlane->b * pEdges[0].y +
             pPlane->c * pEdges[0].z + pPlane->d;
    fDist2 = pPlane->a * pEdges[1].x + pPlane->b * pEdges[1].y +
             pPlane->c * pEdges[1].z + pPlane->d;
    if( fDist1 > 0 && fDist2 > 0 ||
        fDist1 < 0 && fDist2 < 0 )
    {
        return FALSE;
    }

    // Find point of intersection between edge and face plane (if they're
    // parallel, edge does not intersect face and D3DXPlaneIntersectLine 
    // returns NULL)
    D3DXVECTOR3 ptIntersection;
    if( NULL == D3DXPlaneIntersectLine( &ptIntersection, pPlane, &pEdges[0], &pEdges[1] ) )
        return FALSE;

    // Project onto a 2D plane to make the pt-in-poly test easier
    FLOAT fAbsA = (pPlane->a > 0 ? pPlane->a : -pPlane->a);
    FLOAT fAbsB = (pPlane->b > 0 ? pPlane->b : -pPlane->b);
    FLOAT fAbsC = (pPlane->c > 0 ? pPlane->c : -pPlane->c);
    D3DXVECTOR2 facePoints[4];
    D3DXVECTOR2 point;
    if( fAbsA > fAbsB && fAbsA > fAbsC )
    {
        // Plane is mainly pointing along X axis, so use Y and Z
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].y;
            facePoints[i].y = pFacePoints[i].z;
        }
        point.x = ptIntersection.y;
        point.y = ptIntersection.z;
    }
    else if( fAbsB > fAbsA && fAbsB > fAbsC )
    {
        // Plane is mainly pointing along Y axis, so use X and Z
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].x;
            facePoints[i].y = pFacePoints[i].z;
        }
        point.x = ptIntersection.x;
        point.y = ptIntersection.z;
    }
    else
    {
        // Plane is mainly pointing along Z axis, so use X and Y
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].x;
            facePoints[i].y = pFacePoints[i].y;
        }
        point.x = ptIntersection.x;
        point.y = ptIntersection.y;
    }

    // If point is on the outside of any of the face edges, it is
    // outside the face.  
    // We can do this by taking the determinant of the following matrix:
    // | x0 y0 1 |
    // | x1 y1 1 |
    // | x2 y2 1 |
    // where (x0,y0) and (x1,y1) are points on the face edge and (x2,y2) 
    // is our test point.  If this value is positive, the test point is
    // "to the left" of the line.  To determine whether a point needs to
    // be "to the right" or "to the left" of the four lines to qualify as
    // inside the face, we need to see if the faces are specified in 
    // clockwise or counter-clockwise order (it could be either, since the
    // edge could be penetrating from either side).  To determine this, we
    // do the same test to see if the third point is "to the right" or 
    // "to the left" of the line formed by the first two points.
    // See http://forum.swarthmore.edu/dr.math/problems/scott5.31.96.html
    FLOAT x0, x1, x2, y0, y1, y2;
    x0 = facePoints[0].x;
    y0 = facePoints[0].y;
    x1 = facePoints[1].x;
    y1 = facePoints[1].y;
    x2 = facePoints[2].x;
    y2 = facePoints[2].y;
    BOOL bClockwise = FALSE;
    if( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 < 0 )
        bClockwise = TRUE;
    x2 = point.x;
    y2 = point.y;
    for( INT i = 0; i < 4; i++ )
    {
        x0 = facePoints[i].x;
        y0 = facePoints[i].y;
        if( i < 3 )
        {
            x1 = facePoints[i+1].x;
            y1 = facePoints[i+1].y;
        }
        else
        {
            x1 = facePoints[0].x;
            y1 = facePoints[0].y;
        }
        if( ( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 > 0 ) == bClockwise )
            return FALSE;
    }

    // If we get here, the point is inside all four face edges, 
    // so it's inside the face.
    return TRUE;
}

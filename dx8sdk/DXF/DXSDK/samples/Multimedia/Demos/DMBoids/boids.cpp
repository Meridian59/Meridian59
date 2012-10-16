//-----------------------------------------------------------------------------
// File: Boids.cpp
//
// Desc:
//
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <stdio.h>
#include <math.h>
#include <d3dx8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "boids.h"
#include "music.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
BoidMusic g_Music;

struct BOIDVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
};

struct GRIDVERTEX
{
    D3DXVECTOR3 pos;
    D3DCOLOR    color;
};

#define D3DFVF_BOIDVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)
#define D3DFVF_GRIDVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

#define NUM_BOIDS       40

inline FLOAT rnd() { return (((FLOAT)rand())/RAND_MAX); }

inline FLOAT Min( D3DXVECTOR3 v )
{
    if( v.x < v.y ) return (v.x < v.z ) ? v.x : v.z;
    else            return (v.y < v.z ) ? v.y : v.z;
}

inline FLOAT Max( D3DXVECTOR3 v )
{
    if( v.x > v.y ) return (v.x > v.z ) ? v.x : v.z;
    else            return (v.y > v.z ) ? v.y : v.z;
}

BOOL      g_bSeparation = FALSE;
BOOL      g_bAlignment  = FALSE;
BOOL      g_bCohesion   = FALSE;
BOOL      g_bMigratory  = FALSE;
BOOL      g_bObstacle   = FALSE;




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*    m_pFont;              // Font for drawing text

    D3DXMATRIX   m_matWorld;           // Transform matrices
    D3DXMATRIX   m_matView;
    D3DXMATRIX   m_matProj;

    D3DLIGHT8    m_Light1;             // Lights and materials
    D3DLIGHT8    m_Light2;
    D3DMATERIAL8 m_mtrlBackground;
    D3DMATERIAL8 m_mtrlGrid;
    D3DMATERIAL8 m_mtrlBoid;

    CD3DMesh*    m_pSphere;            // Spheres
    FLOAT        m_fSphereSpin;

    LPDIRECT3DVERTEXBUFFER8 m_pBoidVB; // Boid mesh
    LPDIRECT3DINDEXBUFFER8  m_pBoidIB;
    BOIDVERTEX   m_vBoidVertices[16];
    WORD         m_wBoidIndices[30];
    DWORD        m_dwNumBoidVertices;
    DWORD        m_dwNumBoidIndices;

    CFlock       m_Flock;             // The flock structure

    CD3DMesh*    m_pSeaGull;          // Seagull mesh

    GRIDVERTEX   m_vGridPattern1[25]; // Grid mesh
    GRIDVERTEX   m_vGridPattern2[9];

    // Internal functions
    HRESULT RenderFlock();

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
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

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
                :CD3DApplication()
{
    m_strWindowTitle    = _T("DMBoids: DMusic Flocking Boids Sample");
    m_bUseDepthBuffer   = TRUE;

    m_fSphereSpin       = 0.0f;
    m_pBoidVB           = NULL;
    m_pBoidIB           = NULL;
    m_pSphere           = new CD3DMesh();
    m_pSeaGull          = new CD3DMesh();
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    D3DXVECTOR3 vNorm;

    // generate the boid data
    m_dwNumBoidVertices = 16;
    m_dwNumBoidIndices  = 30;

    // top
    m_vBoidVertices[ 0].p = D3DXVECTOR3(  0.0f, 0.0f, 10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 0].n, &D3DXVECTOR3( 0.2f, 1.0f, 0.0f) );
    m_vBoidVertices[ 1].p = D3DXVECTOR3( 10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 1].n, &D3DXVECTOR3( 0.1f, 1.0f, 0.0f) );
    m_vBoidVertices[ 2].p = D3DXVECTOR3(  3.0f, 3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[ 2].n, &D3DXVECTOR3( 0.0f, 1.0f, 0.0f) );
    m_vBoidVertices[ 3].p = D3DXVECTOR3( -3.0f, 3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[ 3].n, &D3DXVECTOR3(-0.1f, 1.0f, 0.0f) );
    m_vBoidVertices[ 4].p = D3DXVECTOR3(-10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 4].n, &D3DXVECTOR3(-0.2f, 1.0f, 0.0f) );

    // bottom
    m_vBoidVertices[ 5].p = D3DXVECTOR3(  0.0f, 0.0f, 10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 5].n, &D3DXVECTOR3( 0.2f, -1.0f, 0.0f) );
    m_vBoidVertices[ 6].p = D3DXVECTOR3( 10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 6].n, &D3DXVECTOR3( 0.1f, -1.0f, 0.0f) );
    m_vBoidVertices[ 7].p = D3DXVECTOR3(  3.0f,-3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[ 7].n, &D3DXVECTOR3( 0.0f, -1.0f, 0.0f) );
    m_vBoidVertices[ 8].p = D3DXVECTOR3( -3.0f,-3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[ 8].n, &D3DXVECTOR3(-0.1f, -1.0f, 0.0f) );
    m_vBoidVertices[ 9].p = D3DXVECTOR3(-10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[ 9].n, &D3DXVECTOR3(-0.2f, -1.0f, 0.0f) );

    // rear
    m_vBoidVertices[10].p = D3DXVECTOR3( 10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[10].n, &D3DXVECTOR3(-0.4f, 0.0f, -1.0f) );
    m_vBoidVertices[11].p = D3DXVECTOR3(  3.0f, 3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[11].n, &D3DXVECTOR3(-0.2f, 0.0f, -1.0f) );
    m_vBoidVertices[12].p = D3DXVECTOR3( -3.0f, 3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[12].n, &D3DXVECTOR3( 0.2f, 0.0f, -1.0f) );
    m_vBoidVertices[13].p = D3DXVECTOR3(-10.0f, 0.0f,-10.0f); D3DXVec3Normalize( &m_vBoidVertices[13].n, &D3DXVECTOR3( 0.4f, 0.0f, -1.0f) );
    m_vBoidVertices[14].p = D3DXVECTOR3( -3.0f,-3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[14].n, &D3DXVECTOR3( 0.2f, 0.0f, -1.0f) );
    m_vBoidVertices[15].p = D3DXVECTOR3(  3.0f,-3.0f, -7.0f); D3DXVec3Normalize( &m_vBoidVertices[15].n, &D3DXVECTOR3(-0.2f, 0.0f, -1.0f) );

    // top
    m_wBoidIndices[ 0] = 0; m_wBoidIndices[ 1] = 1; m_wBoidIndices[ 2] = 2;
    m_wBoidIndices[ 3] = 0; m_wBoidIndices[ 4] = 2; m_wBoidIndices[ 5] = 3;
    m_wBoidIndices[ 6] = 0; m_wBoidIndices[ 7] = 3; m_wBoidIndices[ 8] = 4;

    // bottom
    m_wBoidIndices[ 9] = 5; m_wBoidIndices[10] = 7; m_wBoidIndices[11] = 6;
    m_wBoidIndices[12] = 5; m_wBoidIndices[13] = 8; m_wBoidIndices[14] = 7;
    m_wBoidIndices[15] = 5; m_wBoidIndices[16] = 9; m_wBoidIndices[17] = 8;

    // rear
    m_wBoidIndices[18] = 10; m_wBoidIndices[19] = 15; m_wBoidIndices[20] = 11;
    m_wBoidIndices[21] = 11; m_wBoidIndices[22] = 15; m_wBoidIndices[23] = 12;
    m_wBoidIndices[24] = 12; m_wBoidIndices[25] = 15; m_wBoidIndices[26] = 14;
    m_wBoidIndices[27] = 12; m_wBoidIndices[28] = 14; m_wBoidIndices[29] = 13;

    // scale the boid to be unit length
    for( DWORD i=0; i<16; i++ )
    {
        m_vBoidVertices[i].p.x /= 20.0f;
        m_vBoidVertices[i].p.y /= 20.0f;
        m_vBoidVertices[i].p.z /= 20.0f;
    }

    // seed the random number generator
    srand( timeGetTime() );

    // allocate the flock
    m_Flock.m_Boids      = new Boid[NUM_BOIDS];
    m_Flock.m_dwNumBoids = NUM_BOIDS;
    m_Flock.m_afDist     = (FLOAT**)new LPVOID[NUM_BOIDS];
    m_Flock.m_vGoal      = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

    for( i=0; i<m_Flock.m_dwNumBoids; i++ )
    {
        D3DXMatrixIdentity( &m_Flock.m_Boids[i].matWorld );
        m_Flock.m_Boids[i].vPos   = D3DXVECTOR3( 200.0f*(rnd()-rnd()), 100.0f*rnd(), 200.0f*(rnd()-rnd()) );
        D3DXVec3Normalize( &m_Flock.m_Boids[i].vDir, &D3DXVECTOR3(rnd()-rnd(), rnd()-rnd(), rnd()-rnd()));
        m_Flock.m_Boids[i].yaw    = 0.0f;
        m_Flock.m_Boids[i].pitch  = 0.0f;
        m_Flock.m_Boids[i].roll   = 0.0f;
        m_Flock.m_Boids[i].dyaw   = 0.0f;
        m_Flock.m_Boids[i].speed  = 0.1f;
        m_Flock.m_Boids[i].color  = D3DXVECTOR3( rnd(), rnd(), rnd() );
        m_Flock.m_Boids[i].color -= D3DXVECTOR3( Min(m_Flock.m_Boids[i].color), Min(m_Flock.m_Boids[i].color), Min(m_Flock.m_Boids[i].color) );
        m_Flock.m_Boids[i].color /= Max( m_Flock.m_Boids[i].color );
        m_Flock.m_afDist[i]       = new FLOAT[NUM_BOIDS];
    }

    m_Flock.m_dwNumObstacles = 4;
    m_Flock.m_Obstacles = new Obstacle[m_Flock.m_dwNumObstacles];
    m_Flock.m_Obstacles[0].vPos    = D3DXVECTOR3( 100.0f, 10.0f,   0.0f );
    m_Flock.m_Obstacles[1].vPos    = D3DXVECTOR3(   0.0f, 10.0f, 100.0f );
    m_Flock.m_Obstacles[2].vPos    = D3DXVECTOR3(-100.0f, 10.0f,   0.0f );
    m_Flock.m_Obstacles[3].vPos    = D3DXVECTOR3(   0.0f, 10.0f,-100.0f );
    m_Flock.m_Obstacles[0].fRadius = 0.2f;
    m_Flock.m_Obstacles[1].fRadius = 0.2f;
    m_Flock.m_Obstacles[2].fRadius = 0.2f;
    m_Flock.m_Obstacles[3].fRadius = 0.2f;

    D3DCOLOR diffuse  = D3DCOLOR_RGBA( 0, 0, 30, 128 );
    D3DCOLOR specular = D3DCOLOR_RGBA( 0, 0, 0, 0 );

    for( i=0; i<=24; i++ )
        m_vGridPattern1[i].color = diffuse;

    m_vGridPattern1[ 0].pos = D3DXVECTOR3(-25.0f, 0.0f, 35.0f );
    m_vGridPattern1[ 1].pos = D3DXVECTOR3(-15.0f, 0.0f, 35.0f );
    m_vGridPattern1[ 2].pos = D3DXVECTOR3( -5.0f, 0.0f, 25.0f );
    m_vGridPattern1[ 3].pos = D3DXVECTOR3(  5.0f, 0.0f, 25.0f );
    m_vGridPattern1[ 4].pos = D3DXVECTOR3( 15.0f, 0.0f, 35.0f );
    m_vGridPattern1[ 5].pos = D3DXVECTOR3( 25.0f, 0.0f, 35.0f );
    m_vGridPattern1[ 6].pos = D3DXVECTOR3( 35.0f, 0.0f, 25.0f );
    m_vGridPattern1[ 7].pos = D3DXVECTOR3( 35.0f, 0.0f, 15.0f );
    m_vGridPattern1[ 8].pos = D3DXVECTOR3( 25.0f, 0.0f,  5.0f );
    m_vGridPattern1[ 9].pos = D3DXVECTOR3( 25.0f, 0.0f, -5.0f );
    m_vGridPattern1[10].pos = D3DXVECTOR3( 35.0f, 0.0f,-15.0f );
    m_vGridPattern1[11].pos = D3DXVECTOR3( 35.0f, 0.0f,-25.0f );
    m_vGridPattern1[12].pos = D3DXVECTOR3( 25.0f, 0.0f,-35.0f );
    m_vGridPattern1[13].pos = D3DXVECTOR3( 15.0f, 0.0f,-35.0f );
    m_vGridPattern1[14].pos = D3DXVECTOR3(  5.0f, 0.0f,-25.0f );
    m_vGridPattern1[15].pos = D3DXVECTOR3( -5.0f, 0.0f,-25.0f );
    m_vGridPattern1[16].pos = D3DXVECTOR3(-15.0f, 0.0f,-35.0f );
    m_vGridPattern1[17].pos = D3DXVECTOR3(-25.0f, 0.0f,-35.0f );
    m_vGridPattern1[18].pos = D3DXVECTOR3(-35.0f, 0.0f,-25.0f );
    m_vGridPattern1[19].pos = D3DXVECTOR3(-35.0f, 0.0f,-15.0f );
    m_vGridPattern1[20].pos = D3DXVECTOR3(-25.0f, 0.0f, -5.0f );
    m_vGridPattern1[21].pos = D3DXVECTOR3(-25.0f, 0.0f,  5.0f );
    m_vGridPattern1[22].pos = D3DXVECTOR3(-35.0f, 0.0f, 15.0f );
    m_vGridPattern1[23].pos = D3DXVECTOR3(-35.0f, 0.0f, 25.0f );
    m_vGridPattern1[24].pos = D3DXVECTOR3(-25.0f, 0.0f, 35.0f );

    for( i=0; i<=8; i++ )
        m_vGridPattern2[i].color = diffuse;

    m_vGridPattern2[0].pos = D3DXVECTOR3( -5.0f, 0.0f, 15.0f );
    m_vGridPattern2[1].pos = D3DXVECTOR3(  5.0f, 0.0f, 15.0f );
    m_vGridPattern2[2].pos = D3DXVECTOR3( 15.0f, 0.0f,  5.0f );
    m_vGridPattern2[3].pos = D3DXVECTOR3( 15.0f, 0.0f, -5.0f );
    m_vGridPattern2[4].pos = D3DXVECTOR3(  5.0f, 0.0f,-15.0f );
    m_vGridPattern2[5].pos = D3DXVECTOR3( -5.0f, 0.0f,-15.0f );
    m_vGridPattern2[6].pos = D3DXVECTOR3(-15.0f, 0.0f, -5.0f );
    m_vGridPattern2[7].pos = D3DXVECTOR3(-15.0f, 0.0f,  5.0f );
    m_vGridPattern2[8].pos = D3DXVECTOR3( -5.0f, 0.0f, 15.0f );

    if( FAILED( g_Music.LoadMusic( m_hWnd ) ) )
    {
        OutputDebugString("Failed to initialize DirectMusic.\n");
        return E_FAIL;
    }

    g_Music.StartMusic();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    D3DXVECTOR3 vEyePt;
    D3DXVECTOR3 vLookAtPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUp( 0.0f, 1.0f, 0.0f );

    FLOAT fTime;
    FLOAT fElapsedTime;

    // The REF device is slow enough to throw off the animation
    // in this sample, so if using the REF device, simulate
    // running at 20fps regardless of actual rendering speed.
    if( m_d3dCaps.DeviceType == D3DDEVTYPE_REF )
    {
        static FLOAT fTimePrev = 0.0f;
        fElapsedTime = 0.05f;
        fTime = fTimePrev + fElapsedTime;
        fTimePrev = fTime;
    }
    else
    {
        fElapsedTime = m_fElapsedTime;
        fTime = m_fTime;
    }

    // Move each boids to its new location
    m_Flock.Update( fElapsedTime );

    // LookAt point is the center of the flock
    for( DWORD i=0; i<m_Flock.m_dwNumBoids; i++ )
        vLookAtPt += m_Flock.m_Boids[i].vPos;
    vLookAtPt /= (FLOAT)m_Flock.m_dwNumBoids;

    vEyePt = vLookAtPt + 40 * D3DXVECTOR3( sinf(fTime*0.111f),
                                           0.70f+0.75f*sinf(fTime*0.163f),
                                           cosf(fTime*0.155f) );

    D3DXMatrixLookAtLH( &m_matView, &vEyePt, &vLookAtPt, &vUp );

    g_Music.SetDistance( D3DXVec3Length( &( vEyePt - vLookAtPt ) ) );

    // Update the flock's goal
    m_Flock.m_vGoal = 100.0f * D3DXVECTOR3( sinf(fTime*0.1f),  0.1f, cosf(fTime*0.1f) );

    m_fSphereSpin = fTime;

    g_Music.HandleNotifies();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderFlock()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderFlock()
{
    // Set the view and projection matrices
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &m_matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProj );

    // Draw ground grid
    m_pd3dDevice->SetMaterial( &m_mtrlGrid );

    for (int dx= -2; dx<3; dx++)
    {
        for (int dz= -2; dz<3; dz++)
        {
            D3DXMatrixTranslation( &m_matWorld, dx*80.0f, 0.0f, dz*80.0f );
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matWorld );

            m_pd3dDevice->SetVertexShader( D3DFVF_GRIDVERTEX );
            m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINESTRIP, 24, m_vGridPattern1,
                                           sizeof(GRIDVERTEX) );
            m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINESTRIP,  8, m_vGridPattern2,
                                           sizeof(GRIDVERTEX) );
        }
    }

    // Draw the boids
    for( DWORD i=0; i<m_Flock.m_dwNumBoids; i++ )
    {
        // Most of the time display the boid
        if( i%13 || m_d3dCaps.DeviceType == D3DDEVTYPE_REF )
        {
            // Set the boid's world matrix
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_Flock.m_Boids[i].matWorld );

            // Set the boid's color
            m_mtrlBoid.Diffuse.r = m_Flock.m_Boids[i].color.x;
            m_mtrlBoid.Diffuse.g = m_Flock.m_Boids[i].color.y;
            m_mtrlBoid.Diffuse.b = m_Flock.m_Boids[i].color.z;
            m_pd3dDevice->SetMaterial( &m_mtrlBoid );

            // Render the boid
            m_pd3dDevice->SetVertexShader( D3DFVF_BOIDVERTEX );
            m_pd3dDevice->SetStreamSource( 0, m_pBoidVB, sizeof(BOIDVERTEX) );
            m_pd3dDevice->SetIndices( m_pBoidIB, 0L );
            m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                                0, m_dwNumBoidVertices,
                                                0, m_dwNumBoidIndices/3 );
        }
        else
        {
            // Set the matrix
            D3DXMATRIX matWorld, matRotateY;
            D3DXMatrixRotationY( &matRotateY, D3DX_PI );
            D3DXMatrixMultiply( &matWorld, &matRotateY, &m_Flock.m_Boids[i].matWorld );
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

            // Display a seagull
            m_pSeaGull->Render( m_pd3dDevice );
        }
    }

    for( i=0; i<m_Flock.m_dwNumObstacles; i++ )
    {
        D3DXMATRIX matRotate, matScale;
        FLOAT      fScale = m_Flock.m_Obstacles[i].fRadius;
        D3DXMatrixRotationY( &matRotate, m_fSphereSpin );
        D3DXMatrixScaling( &matScale, fScale, fScale, fScale );
        D3DXMatrixMultiply( &m_matWorld, &matScale,   &matRotate );
        m_matWorld._41 = m_Flock.m_Obstacles[i].vPos.x;
        m_matWorld._42 = m_Flock.m_Obstacles[i].vPos.y;
        m_matWorld._43 = m_Flock.m_Obstacles[i].vPos.z;

        m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matWorld );

        m_pSphere->Render( m_pd3dDevice );
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // Clear the scene to black
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x00000000, 1.0f, 0L );

    // Begin Scene
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK;

    RenderFlock();

    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    if( FAILED( m_pSphere->Create( m_pd3dDevice, _T("orbiter.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( m_pSeaGull->Create( m_pd3dDevice, _T("Shusui.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Create a VB for the boids
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_dwNumBoidVertices*sizeof(BOIDVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_BOIDVERTEX,
                                                  D3DPOOL_MANAGED, &m_pBoidVB ) ) )
        return E_FAIL;

    BOIDVERTEX* v;
    m_pBoidVB->Lock( 0, 0, (BYTE**)&v, 0 );
    memcpy( v, m_vBoidVertices, m_dwNumBoidVertices*sizeof(BOIDVERTEX) );
    m_pBoidVB->Unlock();

    // Create an IB for the boids
    if( FAILED( m_pd3dDevice->CreateIndexBuffer( m_dwNumBoidIndices*sizeof(WORD),
                                                 D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                                 D3DPOOL_MANAGED, &m_pBoidIB ) ) )
        return E_FAIL;

    WORD* i;
    m_pBoidIB->Lock( 0, 0, (BYTE**)&i, 0 );
    memcpy( i, m_wBoidIndices, m_dwNumBoidIndices*sizeof(WORD) );
    m_pBoidIB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pSphere->RestoreDeviceObjects( m_pd3dDevice );
    m_pSeaGull->RestoreDeviceObjects( m_pd3dDevice );

    // Set up transform matrices
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -100.0f );
    D3DXVECTOR3 vLookAtPt = D3DXVECTOR3( 0.0f, 0.0f,    0.0f );
    D3DXVECTOR3 vUp       = D3DXVECTOR3( 0.0f, 1.0f,    0.0f );
    D3DXMatrixIdentity( &m_matWorld );
    D3DXMatrixLookAtLH( &m_matView, &vEyePt, &vLookAtPt, &vUp );
    D3DXMatrixPerspectiveFovLH( &m_matProj, D3DX_PI/4, 1.0f, 5.0f, 400.0f );

    // Setup materials
    D3DUtil_InitMaterial( m_mtrlBackground, 0.0f, 0.0f, 0.0f );
    D3DUtil_InitMaterial( m_mtrlGrid, 0.0f, 0.0f, 0.0f );
    m_mtrlGrid.Emissive.r = 0.0f;
    m_mtrlGrid.Emissive.g = 0.3f;
    m_mtrlGrid.Emissive.b = 0.5f;
    D3DUtil_InitMaterial( m_mtrlBoid, 1.0f, 1.0f, 1.0f );

    // Create 2 lights
    D3DUtil_InitLight( m_Light1, D3DLIGHT_DIRECTIONAL, -0.5f, -1.0f, -0.3f );
    D3DUtil_InitLight( m_Light2, D3DLIGHT_DIRECTIONAL,  0.5f,  1.0f,  0.3f );
    m_Light2.Diffuse.r = 0.5f;
    m_Light2.Diffuse.g = 0.5f;
    m_Light2.Diffuse.b = 0.5f;
    m_pd3dDevice->SetLight( 0, &m_Light1 );
    m_pd3dDevice->SetLight( 1, &m_Light2 );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->LightEnable( 1, TRUE );

    // Set render state
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x11111111 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pSphere->InvalidateDeviceObjects();
    m_pSeaGull->InvalidateDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pSphere->Destroy();
    m_pSeaGull->Destroy();

    SAFE_RELEASE( m_pBoidVB );
    SAFE_RELEASE( m_pBoidIB );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    for( int i=0; i<NUM_BOIDS; i++ )
    {
        SAFE_DELETE_ARRAY( m_Flock.m_afDist[i] );
    }

    SAFE_DELETE_ARRAY( m_Flock.m_Boids );
    SAFE_DELETE_ARRAY( m_Flock.m_afDist );
    SAFE_DELETE_ARRAY( m_Flock.m_Obstacles );

    SAFE_DELETE( m_pSphere );
    SAFE_DELETE( m_pSeaGull );

    g_Music.EndMusic();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                   LPARAM lParam )
{
    // Handle menu commands
    if( WM_KEYDOWN==uMsg || WM_KEYUP==uMsg )
    {
        switch( wParam )
        {
        case 'A':
            g_bAlignment = (WM_KEYDOWN==uMsg);
            break;

        case 'C':
            g_bCohesion = (WM_KEYDOWN==uMsg);
            break;

        case 'O':
            g_bObstacle = (WM_KEYDOWN==uMsg);
            break;

        case 'M':
            g_bMigratory = (WM_KEYDOWN==uMsg);

            if( g_bMigratory )
                g_Music.Migrate();
            break;

        case 'S':
            g_bSeparation = (WM_KEYDOWN==uMsg);

            if( g_bSeparation )
                g_Music.Collapse();
            else
                g_Music.Expand();

            break;
        }
    }
    else if( WM_ACTIVATE==uMsg )
    {
        g_Music.Activate( LOWORD( wParam ) != WA_INACTIVE );
    }

    // Pass remaining messages to default handler
    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




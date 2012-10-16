//-----------------------------------------------------------------------------
// File: ShadowVolume.cpp
//
// Desc: Example code showing how to use stencil buffers to implement shadow
//       volumes.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DUtil.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// External definitions and prototypes
//-----------------------------------------------------------------------------
inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

struct VERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;
};

struct SHADOWVERTEX
{
    D3DXVECTOR4 p;
    D3DCOLOR    color;
};

#define D3DFVF_VERTEX       (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define D3DFVF_SHADOWVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)




//-----------------------------------------------------------------------------
// Name: struct ShadowVolume
// Desc: A shadow volume object
//-----------------------------------------------------------------------------
class ShadowVolume
{
    D3DXVECTOR3 m_pVertices[32000]; // Vertex data for rendering shadow volume
    DWORD       m_dwNumVertices;

public:
    VOID    Reset() { m_dwNumVertices = 0L; }
    HRESULT BuildFromMesh( LPD3DXMESH pObject, D3DXVECTOR3 vLight );
    HRESULT Render( LPDIRECT3DDEVICE8 pd3dDevice );
};




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*     m_pFont;
    CD3DArcBall   m_ArcBall;

    CD3DMesh*     m_pAirplane;
    CD3DMesh*     m_pTerrainObject;
    ShadowVolume* m_pShadowVolume;

    D3DXMATRIX    m_matObjectMatrix;
    D3DXMATRIX    m_matTerrainMatrix;

    LPDIRECT3DVERTEXBUFFER8 m_pBigSquareVB;
    HRESULT DrawShadow();
    HRESULT RenderShadow();

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
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::Render( LPDIRECT3DDEVICE8 pd3dDevice )
{
    pd3dDevice->SetVertexShader( D3DFVF_XYZ );

    return pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_dwNumVertices/3,
                                        m_pVertices, sizeof(D3DXVECTOR3) );
}




//-----------------------------------------------------------------------------
// Name: AddEdge()
// Desc: Adds an edge to a list of silohuette edges of a shadow volume.
//-----------------------------------------------------------------------------
VOID AddEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
    // Remove interior edges (which appear in the list twice)
    for( DWORD i=0; i < dwNumEdges; i++ )
    {
        if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
            ( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
        {
            if( dwNumEdges > 1 )
            {
                pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
                pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
            }
            dwNumEdges--;
            return;
        }
    }

    pEdges[2*dwNumEdges+0] = v0;
    pEdges[2*dwNumEdges+1] = v1;
    dwNumEdges++;
}




//-----------------------------------------------------------------------------
// Name: BuildFromMesh()
// Desc: Takes a mesh as input, and uses it to build a shadowvolume. The
//       technique used considers each triangle of the mesh, and adds it's
//       edges to a temporary list. The edge list is maintained, such that
//       only silohuette edges are kept. Finally, the silohuette edges are
//       extruded to make the shadow volume vertex list.
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::BuildFromMesh( LPD3DXMESH pMesh, D3DXVECTOR3 vLight )
{
    // Note: the MESHVERTEX format depends on the FVF of the mesh
    struct MESHVERTEX { D3DXVECTOR3 p, n; FLOAT tu, tv; };
    DWORD dwFVF = pMesh->GetFVF();

    MESHVERTEX* pVertices;
    WORD*       pIndices;

    // Lock the geometry buffers
    pMesh->LockVertexBuffer( 0L, (BYTE**)&pVertices );
    pMesh->LockIndexBuffer( 0L, (BYTE**)&pIndices );
    DWORD dwNumVertices = pMesh->GetNumVertices();
    DWORD dwNumFaces    = pMesh->GetNumFaces();

    // Allocate a temporary edge list
    WORD* pEdges = new WORD[dwNumFaces*6];
    DWORD dwNumEdges = 0;

    // For each face
    for( DWORD i=0; i<dwNumFaces; i++ )
    {
        WORD wFace0 = pIndices[3*i+0];
        WORD wFace1 = pIndices[3*i+1];
        WORD wFace2 = pIndices[3*i+2];

        D3DXVECTOR3 v0 = pVertices[wFace0].p;
        D3DXVECTOR3 v1 = pVertices[wFace1].p;
        D3DXVECTOR3 v2 = pVertices[wFace2].p;

        // Transform vertices or transform light?
        D3DXVECTOR3 vNormal;
        D3DXVec3Cross( &vNormal, &(v2-v1), &(v1-v0) );

        if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f )
        {
            AddEdge( pEdges, dwNumEdges, wFace0, wFace1 );
            AddEdge( pEdges, dwNumEdges, wFace1, wFace2 );
            AddEdge( pEdges, dwNumEdges, wFace2, wFace0 );
        }
    }

    for( i=0; i<dwNumEdges; i++ )
    {
        D3DXVECTOR3 v1 = pVertices[pEdges[2*i+0]].p;
        D3DXVECTOR3 v2 = pVertices[pEdges[2*i+1]].p;
        D3DXVECTOR3 v3 = v1 - vLight*10;
        D3DXVECTOR3 v4 = v2 - vLight*10;

        // Add a quad (two triangles) to the vertex list
        m_pVertices[m_dwNumVertices++] = v1;
        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v3;

        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v4;
        m_pVertices[m_dwNumVertices++] = v3;
    }
    // Delete the temporary edge list
    delete[] pEdges;

    // Unlock the geometry buffers
    pMesh->UnlockVertexBuffer();
    pMesh->UnlockIndexBuffer();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("ShadowVolume: RealTime Shadows Using The StencilBuffer");
    m_bUseDepthBuffer   = TRUE;
    m_dwMinDepthBits    = 16;
    m_dwMinStencilBits  = 4;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pFont           = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pAirplane       = new CD3DMesh();
    m_pTerrainObject  = new CD3DMesh();
    m_pShadowVolume   = NULL;
    m_pBigSquareVB    = NULL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Construct a shadow volume object;
    m_pShadowVolume = new ShadowVolume();

    // Set cursor to indicate that user can move the object with the mouse
#ifdef _WIN64
    SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor( NULL, IDC_SIZEALL ) );
#else
    SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)LoadCursor( NULL, IDC_SIZEALL ) );
#endif
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Setup the world spin matrix
    D3DXMatrixTranslation( &m_matTerrainMatrix, 0.0f, 0.0f, 0.0f );

    // Setup viewing postion from ArcBall
    m_matObjectMatrix = *m_ArcBall.GetRotationMatrix();
    D3DXMatrixMultiply( &m_matObjectMatrix, &m_matObjectMatrix, m_ArcBall.GetTranslationMatrix() );

    // Move the light
    FLOAT Lx =   5;
    FLOAT Ly =   5;
    FLOAT Lz =  -5;
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_POINT, Lx, Ly, Lz );
    light.Attenuation0 = 0.9f;
    light.Attenuation1 = 0.0f;
    m_pd3dDevice->SetLight( 0, &light );

    // Transform the light vector to be in object space
    D3DXVECTOR3 vLight;
    D3DXMATRIX m;
    D3DXMatrixInverse( &m, NULL, &m_matObjectMatrix );
    vLight.x = Lx*m._11 + Ly*m._21 + Lz*m._31 + m._41;
    vLight.y = Lx*m._12 + Ly*m._22 + Lz*m._32 + m._42;
    vLight.z = Lx*m._13 + Ly*m._23 + Lz*m._33 + m._43;

    // Build the shadow volume
    m_pShadowVolume->Reset();
    m_pShadowVolume->BuildFromMesh( m_pAirplane->GetSysMemMesh(), vLight );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderShadow()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderShadow()
{
    // Disable z-buffer writes (note: z-testing still occurs), and enable the
    // stencil-buffer
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );

    // Dont bother with interpolating color
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE,     D3DSHADE_FLAT );

    // Set up stencil compare fuction, reference value, and masks.
    // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
    // Note: since we set up the stencil-test to always pass, the STENCILFAIL
    // renderstate is really not needed.
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );

    // If ztest passes, inc/decrement stencil buffer value
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );

    // Make sure that no pixels get drawn to the frame buffer
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    // Draw front-side of shadow volume in stencil/z only
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matObjectMatrix );
    m_pShadowVolume->Render( m_pd3dDevice );

    // Now reverse cull order so back sides of shadow volume are written.
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );

    // Decrement stencil buffer value
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );

    // Draw back-side of shadow volume in stencil/z only
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matObjectMatrix );
    m_pShadowVolume->Render( m_pd3dDevice );

    // Restore render states
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawShadow()
// Desc: Draws a big gray polygon over scene according to the mask in the
//       stencil buffer. (Any pixel with stencil==1 is in the shadow.)
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DrawShadow()
{
    // Set renderstates (disable z-buffering, enable stencil, disable fog, and
    // turn on alphablending)
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

    // Only write where stencil val >= 1 (count indicates # of shadows that
    // overlap that pixel)
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );

    // Draw a big, gray square
    m_pd3dDevice->SetVertexShader( D3DFVF_SHADOWVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pBigSquareVB, sizeof(SHADOWVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Restore render states
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

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
                         0xff0000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matTerrainMatrix );
        m_pTerrainObject->Render( m_pd3dDevice );

        m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matObjectMatrix );
        m_pAirplane->Render( m_pd3dDevice );

/*
        // Draw shadow volume
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &m_matObjectMatrix );
        m_pShadowVolume->Render( m_pd3dDevice );
*/

        // Render the shadow volume into the stenicl buffer, then add it into
        // the scene
        RenderShadow();
        DrawShadow();

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

    // Load an object to cast the shadow
    if( FAILED( m_pAirplane->Create( m_pd3dDevice, _T("airplane 2.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Load some terrain
    if( FAILED( m_pTerrainObject->Create( m_pd3dDevice, _T("SeaFloor.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Set a reasonable vertex type
    m_pAirplane->SetFVF( m_pd3dDevice, D3DFVF_VERTEX );
    m_pTerrainObject->SetFVF( m_pd3dDevice, D3DFVF_VERTEX );

    // Tweak the terrain vertices
    {
        LPDIRECT3DVERTEXBUFFER8 pVB;
        VERTEX* pVertices;
        DWORD   dwNumVertices = m_pTerrainObject->GetSysMemMesh()->GetNumVertices();

        // Lock the vertex buffer to access the terrain geometry
        m_pTerrainObject->GetSysMemMesh()->GetVertexBuffer( &pVB );
        pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );

        // Add some more bumpiness to the terrain object
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            pVertices[i].p.y  += 1*(rand()/(FLOAT)RAND_MAX);
            pVertices[i].p.y  += 2*(rand()/(FLOAT)RAND_MAX);
            pVertices[i].p.y  += 1*(rand()/(FLOAT)RAND_MAX);
        }

        // Release the vertex buffer
        pVB->Unlock();
        pVB->Release();
    }

    // Create a big square for rendering the stencilbuffer contents
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(SHADOWVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_SHADOWVERTEX,
                                       D3DPOOL_MANAGED, &m_pBigSquareVB ) ) )
        return E_FAIL;

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

    // Initialize the vertex buffers for the file-based objects
    m_pAirplane->RestoreDeviceObjects( m_pd3dDevice );
    m_pTerrainObject->RestoreDeviceObjects( m_pd3dDevice );

    // Create and set up the shine materials w/ textures
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

    // Set the transform matrices
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 10.0f, -20.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f,  0.0f,   0.0f  );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f,  1.0f,   0.0f  );
    D3DXMATRIX matWorld, matView, matProj;

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );

    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Turn on fog
    FLOAT fFogStart =  30.0f;
    FLOAT fFogEnd   =  80.0f;
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR,       0xff0000ff );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_RANGEFOGENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGSTART,       FtoDW(fFogStart) );
    m_pd3dDevice->SetRenderState( D3DRS_FOGEND,         FtoDW(fFogEnd) );

    // Set the ArcBall parameters
    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height, 2.0f );
    m_ArcBall.SetRadius( 5.0f );

    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,  0x00303030 );

    // Set the size of the big square shadow
    SHADOWVERTEX* v;
    FLOAT sx = (FLOAT)m_d3dsdBackBuffer.Width;
    FLOAT sy = (FLOAT)m_d3dsdBackBuffer.Height;
    m_pBigSquareVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
    v[1].p = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
    v[2].p = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
    v[3].p = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
    v[0].color = 0x7f000000;
    v[1].color = 0x7f000000;
    v[2].color = 0x7f000000;
    v[3].color = 0x7f000000;
    m_pBigSquareVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pAirplane->InvalidateDeviceObjects();
    m_pTerrainObject->InvalidateDeviceObjects();
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
    m_pAirplane->Destroy();
    m_pTerrainObject->Destroy();

    SAFE_RELEASE( m_pBigSquareVB );

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
    SAFE_DELETE( m_pAirplane );
    SAFE_DELETE( m_pTerrainObject );
    SAFE_DELETE( m_pShadowVolume );

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
    // Make sure device supports point lights
    if( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( 0 == ( pCaps->VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS ) )
            return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Pass mouse messages to the ArcBall so it can build internal matrices
    m_ArcBall.HandleMouseMessages( hWnd, uMsg, wParam, lParam );

    // Trap the context menu
    if( WM_CONTEXTMENU == uMsg )
        return 0;

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// File: Dolphin.cpp
//
// Desc: Sample of swimming dolphin
//
//       Note: This code uses the D3D Framework helper library.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Globals variables and definitions
//-----------------------------------------------------------------------------
#define WATER_COLOR         0x00004080

struct D3DVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;
};

#define D3DFVF_D3DVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Main class to run this application. Most functionality is inherited
//       from the CD3DApplication base class.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    // Font for drawing text
    CD3DFont* m_pFont;

    // Transform matrices
    D3DXMATRIX              m_matWorld;
    D3DXMATRIX              m_matView;
    D3DXMATRIX              m_matProj;

    // Dolphin object
    LPDIRECT3DTEXTURE8      m_pDolphinTexture;
    LPDIRECT3DVERTEXBUFFER8 m_pDolphinVB1;
    LPDIRECT3DVERTEXBUFFER8 m_pDolphinVB2;
    LPDIRECT3DVERTEXBUFFER8 m_pDolphinVB3;
    LPDIRECT3DINDEXBUFFER8  m_pDolphinIB;
    DWORD                   m_dwNumDolphinVertices;
    DWORD                   m_dwNumDolphinFaces;
    DWORD                   m_dwDolphinVertexShader;
    DWORD                   m_dwDolphinVertexShader2;

    // Seafloor object
    LPDIRECT3DTEXTURE8      m_pSeaFloorTexture;
    LPDIRECT3DVERTEXBUFFER8 m_pSeaFloorVB;
    LPDIRECT3DINDEXBUFFER8  m_pSeaFloorIB;
    DWORD                   m_dwNumSeaFloorVertices;
    DWORD                   m_dwNumSeaFloorFaces;
    DWORD                   m_dwSeaFloorVertexShader;
    DWORD                   m_dwSeaFloorVertexShader2;

    // Water caustics
    LPDIRECT3DTEXTURE8      m_pCausticTextures[32];
    LPDIRECT3DTEXTURE8      m_pCurrentCausticTexture;

public:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    HRESULT ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format );

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
// Desc: Constructor
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    // Override base class members
    m_strWindowTitle         = _T("DolphinVS: Tweening Vertex Shader");
    m_bUseDepthBuffer        = TRUE;

    m_pFont                  = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );

    // Dolphin object
    m_pDolphinTexture        = NULL;
    m_pDolphinVB1            = NULL;
    m_pDolphinVB2            = NULL;
    m_pDolphinVB3            = NULL;
    m_pDolphinIB             = NULL;
    m_dwDolphinVertexShader  = 0L;
    m_dwDolphinVertexShader2 = 0L;

    // SeaFloor object
    m_pSeaFloorTexture       = NULL;
    m_pSeaFloorVB            = NULL;
    m_pSeaFloorIB            = NULL;
    m_dwSeaFloorVertexShader = 0L;
    m_dwSeaFloorVertexShader2= 0L;

    // Water caustics
    for( DWORD t=0; t<32; t++ )
        m_pCausticTextures[t] = NULL;
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
    // Animation attributes for the dolphin
    FLOAT fKickFreq    = 2*m_fTime;
    FLOAT fPhase       = m_fTime/3;
    FLOAT fBlendWeight = sinf( fKickFreq );

    // Move the dolphin in a circle
    D3DXMATRIX matDolphin, matTrans, matRotate1, matRotate2;
    D3DXMatrixScaling( &matDolphin, 0.01f, 0.01f, 0.01f );
    D3DXMatrixRotationZ( &matRotate1, -cosf(fKickFreq)/6 );
    D3DXMatrixMultiply( &matDolphin, &matDolphin, &matRotate1 );
    D3DXMatrixRotationY( &matRotate2, fPhase );
    D3DXMatrixMultiply( &matDolphin, &matDolphin, &matRotate2 );
    D3DXMatrixTranslation( &matTrans, -5*sinf(fPhase), sinf(fKickFreq)/2, 10-10*cosf(fPhase) );
    D3DXMatrixMultiply( &matDolphin, &matDolphin, &matTrans );

    // Animate the caustic textures
    DWORD tex = ((DWORD)(m_fTime*32))%32;
    m_pCurrentCausticTexture = m_pCausticTextures[tex];

    // Set the vertex shader constants. Note: outside of the blend matrices,
    // most of these values don't change, so don't need to really be set every
    // frame. It's just done here for clarity
    {
        // Some basic constants
        D3DXVECTOR4 vZero( 0.0f, 0.0f, 0.0f, 0.0f );
        D3DXVECTOR4 vOne( 1.0f, 0.5f, 0.2f, 0.05f );

        FLOAT fWeight1;
        FLOAT fWeight2;
        FLOAT fWeight3;

        if( fBlendWeight > 0.0f )
        {
            fWeight1 = fabsf(fBlendWeight);
            fWeight2 = 1.0f - fabsf(fBlendWeight);
            fWeight3 = 0.0f;
        }
        else
        {
            fWeight1 = 0.0f;
            fWeight2 = 1.0f - fabsf(fBlendWeight);
            fWeight3 = fabsf(fBlendWeight);
        }
        D3DXVECTOR4 vWeight( fWeight1, fWeight2, fWeight3, 0.0f );

        // Lighting vectors (in world space and in dolphin model space)
        // and other constants
        FLOAT fLight[]    = { 0.0f,  1.0f, 0.0f, 0.0f };
        FLOAT fLightDolphinSpace[]    = { 0.0f,  1.0f, 0.0f, 0.0f };
        FLOAT fDiffuse[]  = { 1.00f, 1.00f, 1.00f, 1.00f };
        FLOAT fAmbient[]  = { 0.25f, 0.25f, 0.25f, 0.25f };
        FLOAT fFog[]      = { 0.5f, 50.0f, 1.0f/(50.0f-1.0f), 0.0f };
        FLOAT fCaustics[] = { 0.05f, 0.05f, sinf(m_fTime)/8, cosf(m_fTime)/10 };

        D3DXMATRIX matDolphinInv;
        D3DXMatrixInverse(&matDolphinInv, NULL, &matDolphin);
        D3DXVec4Transform((D3DXVECTOR4*)fLightDolphinSpace, (D3DXVECTOR4*)fLight, &matDolphinInv);
        D3DXVec4Normalize((D3DXVECTOR4*)fLightDolphinSpace, (D3DXVECTOR4*)fLightDolphinSpace);

        // Vertex shader operations use transposed matrices
        D3DXMATRIX mat, matCamera, matTranspose, matCameraTranspose;
        D3DXMATRIX matViewTranspose, matProjTranspose;
        D3DXMatrixMultiply(&matCamera, &matDolphin, &m_matView);
        D3DXMatrixMultiply(&mat, &matCamera, &m_matProj);
        D3DXMatrixTranspose(&matTranspose, &mat);
        D3DXMatrixTranspose(&matCameraTranspose, &matCamera);
        D3DXMatrixTranspose(&matViewTranspose, &m_matView);
        D3DXMatrixTranspose(&matProjTranspose, &m_matProj);

        // Set the vertex shader constants
        m_pd3dDevice->SetVertexShaderConstant(  0, &vZero,     1 );
        m_pd3dDevice->SetVertexShaderConstant(  1, &vOne,      1 );
        m_pd3dDevice->SetVertexShaderConstant(  2, &vWeight,   1 );
        m_pd3dDevice->SetVertexShaderConstant(  4, &matTranspose, 4 );
        m_pd3dDevice->SetVertexShaderConstant(  8, &matCameraTranspose,  4 );
        m_pd3dDevice->SetVertexShaderConstant( 12, &matViewTranspose,  4 );
        m_pd3dDevice->SetVertexShaderConstant( 19, &fLightDolphinSpace,   1 );
        m_pd3dDevice->SetVertexShaderConstant( 20, &fLight,    1 );
        m_pd3dDevice->SetVertexShaderConstant( 21, &fDiffuse,  1 );
        m_pd3dDevice->SetVertexShaderConstant( 22, &fAmbient,  1 );
        m_pd3dDevice->SetVertexShaderConstant( 23, &fFog,      1 );
        m_pd3dDevice->SetVertexShaderConstant( 24, &fCaustics, 1 );
        m_pd3dDevice->SetVertexShaderConstant( 28, &matProjTranspose,  4 );
    }

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
                         WATER_COLOR, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        FLOAT fAmbientLight[]  = { 0.25f, 0.25f, 0.25f, 0.25f };
        m_pd3dDevice->SetVertexShaderConstant( 22, &fAmbientLight, 1 );

        // Render the seafloor
        m_pd3dDevice->SetTexture( 0, m_pSeaFloorTexture );
        m_pd3dDevice->SetVertexShader( m_dwSeaFloorVertexShader );
        m_pd3dDevice->SetStreamSource( 0, m_pSeaFloorVB, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetIndices( m_pSeaFloorIB, 0 );
        m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                            0, m_dwNumSeaFloorVertices,
                                            0, m_dwNumSeaFloorFaces );

        // Render the dolphin
        m_pd3dDevice->SetTexture( 0, m_pDolphinTexture );
        m_pd3dDevice->SetVertexShader( m_dwDolphinVertexShader );
        m_pd3dDevice->SetStreamSource( 0, m_pDolphinVB1, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetStreamSource( 1, m_pDolphinVB2, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetStreamSource( 2, m_pDolphinVB3, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetIndices( m_pDolphinIB, 0 );
        m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                            0, m_dwNumDolphinVertices,
                                            0, m_dwNumDolphinFaces );

        // Now, we are going to do a 2nd pass, to alpha-blend in the caustics.
        // The caustics use a 2nd set of texture coords that are generated
        // by the vertex shaders. Lighting from the light above is used, but
        // ambient is turned off to avoid lighting objects from below (for
        // instance, we don't want caustics appearing on the dolphin's
        // underbelly). Finally, fog color is set to black, so that caustics
        // fade in distance.

        // Turn on alpha blending
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        
        m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
        m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

        // Setup the caustic texture
        m_pd3dDevice->SetTexture( 0, m_pCurrentCausticTexture );

        // Set ambient and fog colors to black
        FLOAT fAmbientDark[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pd3dDevice->SetVertexShaderConstant( 22, &fAmbientDark, 1 );
        m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, 0x00000000 );

        // Render the caustic effects for the seafloor
        m_pd3dDevice->SetVertexShader( m_dwSeaFloorVertexShader2 );
        m_pd3dDevice->SetStreamSource( 0, m_pSeaFloorVB, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetIndices( m_pSeaFloorIB, 0 );
        m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                            0, m_dwNumSeaFloorVertices,
                                            0, m_dwNumSeaFloorFaces );

        // Finally, render the caustic effects for the dolphin
        m_pd3dDevice->SetVertexShader( m_dwDolphinVertexShader2 );
        m_pd3dDevice->SetStreamSource( 0, m_pDolphinVB1, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetStreamSource( 1, m_pDolphinVB2, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetStreamSource( 2, m_pDolphinVB3, sizeof(D3DVERTEX) );
        m_pd3dDevice->SetIndices( m_pDolphinIB, 0 );
        m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
                                            0, m_dwNumDolphinVertices,
                                            0, m_dwNumDolphinFaces );

        // Restore modified render states
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, WATER_COLOR );

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
// Desc: Initialize device-dependent objects. This is the place to create mesh
//       and texture objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    LPDIRECT3DVERTEXBUFFER8 pMeshSourceVB;
    LPDIRECT3DINDEXBUFFER8  pMeshSourceIB;
    D3DVERTEX*              pSrc;
    D3DVERTEX*              pDst;
    CD3DMesh                DolphinMesh01;
    CD3DMesh                DolphinMesh02;
    CD3DMesh                DolphinMesh03;
    CD3DMesh                SeaFloorMesh;

    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Create texture for the dolphin
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Dolphin.bmp"),
                                       &m_pDolphinTexture ) ) )
    {
        return D3DAPPERR_MEDIANOTFOUND;
    }

    // Create textures for the seafloor
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("SeaFloor.bmp"),
                                       &m_pSeaFloorTexture ) ) )
    {
        return D3DAPPERR_MEDIANOTFOUND;
    }

    // Create textures for the water caustics
    for( DWORD t=0; t<32; t++ )
    {
        TCHAR strName[80];
        sprintf( strName, _T("Caust%02ld.tga"), t );
        if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, strName,
                                           &m_pCausticTextures[t] ) ) )
        {
            return D3DAPPERR_MEDIANOTFOUND;
        }
    }

    // Load the file-based mesh objects
    if( FAILED( DolphinMesh01.Create( m_pd3dDevice, _T("dolphin1.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( DolphinMesh02.Create( m_pd3dDevice, _T("dolphin2.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( DolphinMesh03.Create( m_pd3dDevice, _T("dolphin3.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( SeaFloorMesh.Create( m_pd3dDevice, _T("SeaFloor.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Set the FVF type to match the vertex format we want
    DolphinMesh01.SetFVF( m_pd3dDevice, D3DFVF_D3DVERTEX );
    DolphinMesh02.SetFVF( m_pd3dDevice, D3DFVF_D3DVERTEX );
    DolphinMesh03.SetFVF( m_pd3dDevice, D3DFVF_D3DVERTEX );
    SeaFloorMesh.SetFVF(  m_pd3dDevice, D3DFVF_D3DVERTEX );

    // Get the number of vertices and faces for the meshes
    m_dwNumDolphinVertices  = DolphinMesh01.GetSysMemMesh()->GetNumVertices();
    m_dwNumDolphinFaces     = DolphinMesh01.GetSysMemMesh()->GetNumFaces();
    m_dwNumSeaFloorVertices = SeaFloorMesh.GetSysMemMesh()->GetNumVertices();
    m_dwNumSeaFloorFaces    = SeaFloorMesh.GetSysMemMesh()->GetNumFaces();

    // Create the dolphin and seafloor vertex and index buffers
    m_pd3dDevice->CreateVertexBuffer( m_dwNumDolphinVertices * sizeof(D3DVERTEX),
                                      D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
                                      &m_pDolphinVB1 );
    m_pd3dDevice->CreateVertexBuffer( m_dwNumDolphinVertices * sizeof(D3DVERTEX),
                                      D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
                                      &m_pDolphinVB2 );
    m_pd3dDevice->CreateVertexBuffer( m_dwNumDolphinVertices * sizeof(D3DVERTEX),
                                      D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
                                      &m_pDolphinVB3 );
    m_pd3dDevice->CreateVertexBuffer( m_dwNumSeaFloorVertices * sizeof(D3DVERTEX),
                                      D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
                                      &m_pSeaFloorVB );
    m_pd3dDevice->CreateIndexBuffer( m_dwNumDolphinFaces * 3 * sizeof(WORD),
                                      D3DUSAGE_WRITEONLY,
                                      D3DFMT_INDEX16, D3DPOOL_MANAGED,
                                      &m_pDolphinIB );
    m_pd3dDevice->CreateIndexBuffer( m_dwNumSeaFloorFaces * 3 * sizeof(WORD),
                                      D3DUSAGE_WRITEONLY,
                                      D3DFMT_INDEX16, D3DPOOL_MANAGED,
                                      &m_pSeaFloorIB );

    // Copy vertices for mesh 01
    DolphinMesh01.GetSysMemMesh()->GetVertexBuffer( &pMeshSourceVB );
    m_pDolphinVB1->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceVB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, m_dwNumDolphinVertices * sizeof(D3DVERTEX) );
    m_pDolphinVB1->Unlock();
    pMeshSourceVB->Unlock();
    pMeshSourceVB->Release();

    // Copy vertices for mesh 2
    DolphinMesh02.GetSysMemMesh()->GetVertexBuffer( &pMeshSourceVB );
    m_pDolphinVB2->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceVB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, m_dwNumDolphinVertices * sizeof(D3DVERTEX) );
    m_pDolphinVB2->Unlock();
    pMeshSourceVB->Unlock();
    pMeshSourceVB->Release();

    // Copy vertices for mesh 3
    DolphinMesh03.GetSysMemMesh()->GetVertexBuffer( &pMeshSourceVB );
    m_pDolphinVB3->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceVB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, m_dwNumDolphinVertices * sizeof(D3DVERTEX) );
    m_pDolphinVB3->Unlock();
    pMeshSourceVB->Unlock();
    pMeshSourceVB->Release();

    // Copy vertices for the seafloor mesh, and add some bumpiness
    SeaFloorMesh.GetSysMemMesh()->GetVertexBuffer( &pMeshSourceVB );
    m_pSeaFloorVB->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceVB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, m_dwNumSeaFloorVertices * sizeof(D3DVERTEX) );
    srand(5);
    for( DWORD i=0; i<m_dwNumSeaFloorVertices; i++ )
    {
        ((D3DVERTEX*)pDst)[i].p.y += (rand()/(FLOAT)RAND_MAX);
        ((D3DVERTEX*)pDst)[i].p.y += (rand()/(FLOAT)RAND_MAX);
        ((D3DVERTEX*)pDst)[i].p.y += (rand()/(FLOAT)RAND_MAX);
        ((D3DVERTEX*)pDst)[i].tu  *= 10;
        ((D3DVERTEX*)pDst)[i].tv  *= 10;
    }
    m_pSeaFloorVB->Unlock();
    pMeshSourceVB->Unlock();
    pMeshSourceVB->Release();

    // Copy indices for the dolphin mesh
    DolphinMesh01.GetSysMemMesh()->GetIndexBuffer( &pMeshSourceIB );
    m_pDolphinIB->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceIB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, 3 * m_dwNumDolphinFaces * sizeof(WORD) );
    m_pDolphinIB->Unlock();
    pMeshSourceIB->Unlock();
    pMeshSourceIB->Release();

    // Copy indices for the seafloor mesh
    SeaFloorMesh.GetSysMemMesh()->GetIndexBuffer( &pMeshSourceIB );
    m_pSeaFloorIB->Lock( 0, 0, (BYTE**)&pDst, 0 );
    pMeshSourceIB->Lock( 0, 0, (BYTE**)&pSrc, 0 );
    memcpy( pDst, pSrc, 3 * m_dwNumSeaFloorFaces * sizeof(WORD) );
    m_pSeaFloorIB->Unlock();
    pMeshSourceIB->Unlock();
    pMeshSourceIB->Release();

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

    // Set the transform matrices
    D3DXVECTOR3 vEyePt      = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 vLookatPt   = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
    D3DXVECTOR3 vUpVec      = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
    FLOAT       fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixIdentity( &m_matWorld );
    D3DXMatrixLookAtLH( &m_matView, &vEyePt, &vLookatPt, &vUpVec );
    D3DXMatrixPerspectiveFovLH( &m_matProj, D3DX_PI/3, fAspect, 1.0f, 10000.0f );

    // Set default render states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR,       WATER_COLOR );

    // Create vertex shader for the dolphin
    DWORD dwDolphinVertexDecl[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 6, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_STREAM( 1 ),
        D3DVSD_REG( 1, D3DVSDT_FLOAT3 ), // Position of second mesh
        D3DVSD_REG( 4, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 7, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_STREAM( 2 ),
        D3DVSD_REG( 2, D3DVSDT_FLOAT3 ), // Position of second mesh
        D3DVSD_REG( 5, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 8, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    if( FAILED( hr = D3DUtil_CreateVertexShader( m_pd3dDevice, _T("DolphinTween.vsh"),
                                                 dwDolphinVertexDecl,
                                                 &m_dwDolphinVertexShader ) ) )
    {
        return hr;
    }

    if( FAILED( hr = D3DUtil_CreateVertexShader( m_pd3dDevice, _T("DolphinTween2.vsh"),
                                                 dwDolphinVertexDecl,
                                                 &m_dwDolphinVertexShader2 ) ) )
    {
        return hr;
    }

    // Create vertex shader for the seafloor
    DWORD dwSeaFloorVertexDecl[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 6, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    if( FAILED( hr = D3DUtil_CreateVertexShader( m_pd3dDevice, _T("SeaFloor.vsh"),
                                                 dwSeaFloorVertexDecl,
                                                 &m_dwSeaFloorVertexShader ) ) )
    {
        return hr;
    }

    if( FAILED( hr = D3DUtil_CreateVertexShader( m_pd3dDevice, _T("SeaFloor2.vsh"),
                                                 dwSeaFloorVertexDecl,
                                                 &m_dwSeaFloorVertexShader2 ) ) )
    {
        return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();

    // Clean up vertex shaders
    if( m_dwDolphinVertexShader != 0 )
    {
        m_pd3dDevice->DeleteVertexShader( m_dwDolphinVertexShader );
        m_dwDolphinVertexShader = 0;
    }

    if( m_dwDolphinVertexShader2 != 0 )
    {
        m_pd3dDevice->DeleteVertexShader( m_dwDolphinVertexShader2 );
        m_dwDolphinVertexShader2 = 0;
    }
    
    if( m_dwSeaFloorVertexShader != 0 )
    {
        m_pd3dDevice->DeleteVertexShader( m_dwSeaFloorVertexShader );
        m_dwSeaFloorVertexShader = 0;
    }
    
    if( m_dwSeaFloorVertexShader2 != 0 )
    {
        m_pd3dDevice->DeleteVertexShader( m_dwSeaFloorVertexShader2 );
        m_dwSeaFloorVertexShader2 = 0;
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

    // Clean up dolphin objects
    SAFE_RELEASE( m_pDolphinTexture );
    SAFE_RELEASE( m_pDolphinVB1 );
    SAFE_RELEASE( m_pDolphinVB2 );
    SAFE_RELEASE( m_pDolphinVB3 );
    SAFE_RELEASE( m_pDolphinIB );

    // Clean up seafoor objects
    SAFE_RELEASE( m_pSeaFloorTexture );
    SAFE_RELEASE( m_pSeaFloorVB );
    SAFE_RELEASE( m_pSeaFloorIB );

    // Clean up textures for water caustics
    for( DWORD i=0; i<32; i++ )
        SAFE_RELEASE( m_pCausticTextures[i] );

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
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->VertexShaderVersion < D3DVS_VERSION(1,0) )
            return E_FAIL;
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// File: SphereMap.cpp
//
// Desc: Example code showing how to use sphere-mapping in D3D, using generated 
//       texture coordinates.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"


//-----------------------------------------------------------------------------
// Name: g_szEffect
// Desc: String containing effect used to render shiny teapot.
//-----------------------------------------------------------------------------

const char g_szEffect[] = 

    "texture texSphereMap;\n"
    "matrix matWorld;\n"
    "matrix matViewProject;\n"
    "vector vecPosition;\n"


    "technique Sphere\n"
    "{\n"
        "pass P0\n"
        "{\n"

            // Vertex state
            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  // position
                    "float v1[3];\n"  // normal
                "}\n"
                "asm\n"
                "{\n"
                    "vs.1.0\n"
                    "def c64, 0.25f, 0.5f, 1.0f, -1.0f\n"
        
                    // r0: camera-space position
                    // r1: camera-space normal
                    // r2: camera-space vertex-eye vector
                    // r3: camera-space reflection vector
                    // r4: texture coordinates

                    // Transform position and normal into camera-space
                    "m4x4 r0, v0, c0\n"
                    "m3x3 r1, v1, c0\n"

                    // Compute normalized view vector
                    "add r2, c8, -r0\n"
                    "dp3 r3, r2, r2\n"
                    "rsq r3, r3\n"
                    "mul r2, r2, r3\n"

                    // Compute camera-space reflection vector
                    "dp3 r3, r1, r2\n"
                    "mul r1, r1, r3\n"
                    "add r1, r1, r1\n"
                    "add r3, r1, -r2\n"

                    // Compute sphere-map texture coords
                    "mad r4.w, -r3.z, c64.y, c64.y\n"
                    "rsq r4, r4\n"
                    "mul r4, r3, r4\n"
                    "mad r4, r4, c64.x, c64.y\n"

                    // Project position
                    "m4x4 oPos, r0, c4\n"
                    "mul oT0.xy, r4.xy, c64.zw\n"
                    "mov oT0.zw, c64.z\n"
                "};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProject>;\n"
            "VertexShaderConstant1[8] = <vecPosition>;\n"

            // Pixel state
            "Texture[0] = <texSphereMap>;\n"
            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
            "ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
        "}\n"
    "}\n";

const UINT g_cchEffect = sizeof(g_szEffect) - 1;




//-----------------------------------------------------------------------------
// Name: struct ENVMAPPEDVERTEX
// Desc: D3D vertex type for environment-mapped objects
//-----------------------------------------------------------------------------
struct ENVMAPPEDVERTEX
{
    D3DXVECTOR3 p; // Position
    D3DXVECTOR3 n; // Normal
};

#define D3DFVF_ENVMAPVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL )



//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    BOOL m_bCapture;

    D3DXMATRIX m_matProject;
    D3DXMATRIX m_matView;
    D3DXMATRIX m_matWorld;
    D3DXMATRIX m_matTrackBall;

    CD3DFont* m_pFont;
    CD3DMesh* m_pShinyTeapot;
    CD3DMesh* m_pSkyBox;

    ID3DXEffect* m_pEffect;
    IDirect3DTexture8* m_pSphereMap;

protected:
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );
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
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("SphereMap: Environment Mapping Technique");
    m_bUseDepthBuffer   = TRUE;
    m_bCapture          = FALSE;

    m_pFont             = NULL;
    m_pShinyTeapot      = NULL;
    m_pSkyBox           = NULL;
    
    m_pEffect           = NULL;
    m_pSphereMap        = NULL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    D3DXMatrixIdentity( &m_matWorld );

    m_pFont        = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pShinyTeapot = new CD3DMesh();
    m_pSkyBox      = new CD3DMesh();
    
    if( !m_pFont || !m_pShinyTeapot || !m_pSkyBox )
        return E_OUTOFMEMORY;

    D3DXMatrixIdentity( &m_matTrackBall );
    D3DXMatrixTranslation( &m_matView, 0.0f, 0.0f, 3.0f );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // When the window has focus, let the mouse adjust the camera view
    if( m_bCapture )
    {
        D3DXMATRIX matCursor;
        D3DXQUATERNION qCursor = D3DUtil_GetRotationFromCursor( m_hWnd );
        D3DXMatrixRotationQuaternion( &matCursor, &qCursor );
        D3DXMatrixMultiply( &m_matView, &m_matTrackBall, &matCursor );

        D3DXMATRIX matTrans;
        D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 3.0f );
        D3DXMatrixMultiply( &m_matView, &m_matView, &matTrans );
    }
    else
    {
        D3DXMATRIX matRotation;
        D3DXMatrixRotationY( &matRotation, -m_fElapsedTime );
        D3DXMatrixMultiply( &m_matWorld, &m_matWorld, &matRotation );
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
    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Render the Skybox
        {
            D3DXMATRIX matWorld;
            D3DXMatrixScaling( &matWorld, 10.0f, 10.0f, 10.0f );

            D3DXMATRIX matView(m_matView);
            matView._41 = matView._42 = matView._43 = 0.0f;

            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
            m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
            m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProject );

            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_MIRROR );
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_MIRROR );

            // Always pass Z-test, so we can avoid clearing color and depth buffers
            m_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
            m_pSkyBox->Render( m_pd3dDevice );
            m_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
        }


        // Render the environment-mapped ShinyTeapot
        {
            // Set transform state
            D3DXMATRIX matViewProject;
            D3DXMatrixMultiply( &matViewProject, &m_matView, &m_matProject );

            D3DXMATRIX matViewInv;
            D3DXMatrixInverse( &matViewInv, NULL, &m_matView );
            D3DXVECTOR4 vecPosition( matViewInv._41, matViewInv._42, matViewInv._43, 1.0f );


            m_pEffect->SetMatrix( "matWorld", &m_matWorld );
            m_pEffect->SetMatrix( "matViewProject", &matViewProject );
            m_pEffect->SetVector( "vecPosition", &vecPosition );


            // Draw teapot
            LPDIRECT3DVERTEXBUFFER8 pVB;
            LPDIRECT3DINDEXBUFFER8 pIB;

            m_pShinyTeapot->m_pLocalMesh->GetVertexBuffer( &pVB );
            m_pShinyTeapot->m_pLocalMesh->GetIndexBuffer( &pIB );

            m_pd3dDevice->SetStreamSource( 0, pVB, sizeof(ENVMAPPEDVERTEX) );
            m_pd3dDevice->SetIndices( pIB, 0 );

            UINT uPasses;
            m_pEffect->Begin( &uPasses, 0 );

            for( UINT iPass = 0; iPass < uPasses; iPass++ )
            {
                m_pEffect->Pass( iPass );

                m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 
                    0, m_pShinyTeapot->m_pLocalMesh->GetNumVertices(),
                    0, m_pShinyTeapot->m_pLocalMesh->GetNumFaces() );

            }

            m_pEffect->End();

            SAFE_RELEASE( pVB );
            SAFE_RELEASE( pIB );
        }


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
    // Load the file objects
    if( FAILED( m_pShinyTeapot->Create( m_pd3dDevice, _T("teapot.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( m_pSkyBox->Create( m_pd3dDevice, _T("lobby_skybox.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("spheremap.bmp"), &m_pSphereMap ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Set mesh properties
    m_pShinyTeapot->SetFVF( m_pd3dDevice, D3DFVF_ENVMAPVERTEX );

    // Restore the device-dependent objects
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Create Effect object
    if( FAILED( D3DXCreateEffect( m_pd3dDevice, g_szEffect, g_cchEffect, &m_pEffect, NULL ) ) )
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
    // InitDeviceObjects for file objects (build textures and vertex buffers)
    m_pShinyTeapot->RestoreDeviceObjects( m_pd3dDevice );
    m_pSkyBox->RestoreDeviceObjects( m_pd3dDevice );
    m_pFont->RestoreDeviceObjects();
    m_pEffect->OnResetDevice();
    m_pEffect->SetTexture( "texSphereMap", m_pSphereMap );

    // Set the transform matrices
    FLOAT fAspect = (FLOAT) m_d3dsdBackBuffer.Width / (FLOAT) m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &m_matProject, D3DX_PI * 0.4f, fAspect, 0.5f, 100.0f );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pShinyTeapot->InvalidateDeviceObjects();
    m_pSkyBox->InvalidateDeviceObjects();
    m_pFont->InvalidateDeviceObjects();

    if(m_pEffect)
        m_pEffect->OnLostDevice();

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
    m_pShinyTeapot->Destroy();
    m_pSkyBox->Destroy();

    SAFE_RELEASE( m_pSphereMap );
    SAFE_RELEASE( m_pEffect );
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
    SAFE_DELETE( m_pShinyTeapot );
    SAFE_DELETE( m_pSkyBox );

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
    if( !(dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING) &&
        !(pCaps->VertexShaderVersion >= D3DVS_VERSION(1, 0)) )
    {
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
    // Capture mouse when clicked
    if( WM_LBUTTONDOWN == uMsg )
    {
        D3DXMATRIX matCursor;
        D3DXQUATERNION qCursor = D3DUtil_GetRotationFromCursor( m_hWnd );
        D3DXMatrixRotationQuaternion( &matCursor, &qCursor );
        D3DXMatrixTranspose( &matCursor, &matCursor );
        D3DXMatrixMultiply( &m_matTrackBall, &m_matTrackBall, &matCursor );

        SetCapture( m_hWnd );
        m_bCapture = TRUE;
        return 0;
    }

    if( WM_LBUTTONUP == uMsg )
    {
        D3DXMATRIX matCursor;
        D3DXQUATERNION qCursor = D3DUtil_GetRotationFromCursor( m_hWnd );
        D3DXMatrixRotationQuaternion( &matCursor, &qCursor );
        D3DXMatrixMultiply( &m_matTrackBall, &m_matTrackBall, &matCursor );

        ReleaseCapture();
        m_bCapture = FALSE;
        return 0;
    }

    // Pass remaining messages to default handler
    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}

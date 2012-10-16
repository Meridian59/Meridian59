//-----------------------------------------------------------------------------
// File: VertexShader.cpp
//
// Desc: Example code showing how to do vertex shaders in D3D.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
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
#include "resource.h"




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
    CD3DFont* m_pFontSmall;

    // Scene
    LPDIRECT3DVERTEXBUFFER8 m_pVB;
    LPDIRECT3DINDEXBUFFER8  m_pIB;
    DWORD        m_dwNumVertices;
    DWORD        m_dwNumIndices;
    DWORD        m_dwShader;
    DWORD        m_dwSize;

    // Transforms
    D3DXMATRIX  m_matPosition;
    D3DXMATRIX  m_matView;
    D3DXMATRIX  m_matProj;

    // Navigation
    BYTE        m_bKey[256];
    FLOAT       m_fSpeed;
    FLOAT       m_fAngularSpeed;
    BOOL        m_bShowHelp;

    D3DXVECTOR3 m_vVelocity;
    D3DXVECTOR3 m_vAngularVelocity;

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT FinalCleanup();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format );
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

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
    m_strWindowTitle    = _T("VertexShader");
    m_bUseDepthBuffer   = TRUE;

    m_pFont            = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pFontSmall       = new CD3DFont( _T("Arial"),  9, D3DFONT_BOLD );
    m_pIB              = NULL;
    m_pVB              = NULL;
    m_dwSize           = 32;
    m_dwNumIndices     = (m_dwSize - 1) * (m_dwSize - 1) * 6;
    m_dwNumVertices    = m_dwSize * m_dwSize;
    m_dwShader         = 0;

    ZeroMemory( m_bKey, sizeof(m_bKey) );
    m_fSpeed           = 5.0f;
    m_fAngularSpeed    = 1.0f;
    m_bShowHelp        = FALSE;

    m_vVelocity        = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vAngularVelocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Setup the view matrix
    D3DXVECTOR3 vEye = D3DXVECTOR3( 2.0f, 3.0f, 3.0f );
    D3DXVECTOR3 vAt  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUp  = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtRH( &m_matView, &vEye, &vAt, &vUp );

    // Set the position matrix
    D3DXMatrixInverse( &m_matPosition, NULL, &m_matView );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    FLOAT fSecsPerFrame = m_fElapsedTime;

    // Process keyboard input
    D3DXVECTOR3 vT( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vR( 0.0f, 0.0f, 0.0f );

    if( m_bKey[VK_LEFT] || m_bKey[VK_NUMPAD1] )                 vT.x -= 1.0f; // Slide Left
    if( m_bKey[VK_RIGHT] || m_bKey[VK_NUMPAD3] )                vT.x += 1.0f; // Slide Right
    if( m_bKey[VK_DOWN] )                                       vT.y -= 1.0f; // Slide Down
    if( m_bKey[VK_UP] )                                         vT.y += 1.0f; // Slide Up
    if( m_bKey['W'] )                                           vT.z -= 2.0f; // Move Forward
    if( m_bKey['S'] )                                           vT.z += 2.0f; // Move Backward
    if( m_bKey['A'] || m_bKey[VK_NUMPAD8] )                     vR.x -= 1.0f; // Pitch Down
    if( m_bKey['Z'] || m_bKey[VK_NUMPAD2] )                     vR.x += 1.0f; // Pitch Up
    if( m_bKey['E'] || m_bKey[VK_NUMPAD6] )                     vR.y -= 1.0f; // Turn Right
    if( m_bKey['Q'] || m_bKey[VK_NUMPAD4] )                     vR.y += 1.0f; // Turn Left
    if( m_bKey[VK_NUMPAD9] )                                    vR.z -= 2.0f; // Roll CW
    if( m_bKey[VK_NUMPAD7] )                                    vR.z += 2.0f; // Roll CCW

    m_vVelocity        = m_vVelocity * 0.9f + vT * 0.1f;
    m_vAngularVelocity = m_vAngularVelocity * 0.9f + vR * 0.1f;

    // Update position and view matricies
    D3DXMATRIX     matT, matR;
    D3DXQUATERNION qR;

    vT = m_vVelocity * fSecsPerFrame * m_fSpeed;
    vR = m_vAngularVelocity * fSecsPerFrame * m_fAngularSpeed;

    D3DXMatrixTranslation( &matT, vT.x, vT.y, vT.z);
    D3DXMatrixMultiply( &m_matPosition, &matT, &m_matPosition );

    D3DXQuaternionRotationYawPitchRoll( &qR, vR.y, vR.x, vR.z );
    D3DXMatrixRotationQuaternion( &matR, &qR );

    D3DXMatrixMultiply( &m_matPosition, &matR, &m_matPosition );
    D3DXMatrixInverse( &m_matView, NULL, &m_matPosition );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_matView );

    // Set up the vertex shader constants
    {
        D3DXMATRIX mat;
        D3DXMatrixMultiply( &mat, &m_matView, &m_matProj );
        D3DXMatrixTranspose( &mat, &mat );

        D3DXVECTOR4 vA( sinf(m_fTime)*15.0f, 0.0f, 0.5f, 1.0f );
        D3DXVECTOR4 vD( D3DX_PI, 1.0f/(2.0f*D3DX_PI), 2.0f*D3DX_PI, 0.05f );

        // Taylor series coefficients for sin and cos
        D3DXVECTOR4 vSin( 1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f );
        D3DXVECTOR4 vCos( 1.0f, -1.0f/2.0f, 1.0f/ 24.0f, -1.0f/ 720.0f );

        m_pd3dDevice->SetVertexShaderConstant(  0, &mat,  4 );
        m_pd3dDevice->SetVertexShaderConstant(  4, &vA,   1 );
        m_pd3dDevice->SetVertexShaderConstant(  7, &vD,   1 );
        m_pd3dDevice->SetVertexShaderConstant( 10, &vSin, 1 );
        m_pd3dDevice->SetVertexShaderConstant( 11, &vCos, 1 );
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
    // Clear the backbuffer
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        m_pd3dDevice->SetVertexShader( m_dwShader );
        m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(D3DXVECTOR2) );
        m_pd3dDevice->SetIndices( m_pIB, 0 );
        m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, m_dwNumVertices,
                                            0, m_dwNumIndices/3 );

        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

        if( m_bShowHelp )
        {
            m_pFontSmall->DrawText(  2, 40, D3DCOLOR_ARGB(255,255,255,255),
                                    _T("Keyboard controls:") );
            m_pFontSmall->DrawText( 20, 60, D3DCOLOR_ARGB(255,255,255,255),
                                    _T("Move\nTurn\nPitch\nSlide\n")
                                    _T("Help\nChange device\nExit") );
            m_pFontSmall->DrawText( 210, 60, D3DCOLOR_ARGB(255,255,255,255),
                                    _T("W,S\nE,Q\nA,Z\nArrow keys\n")
                                    _T("F1\nF2\nEsc") );
        }
        else
        {
            m_pFontSmall->DrawText(  2, 40, D3DCOLOR_ARGB(255,255,255,255), 
                               _T("Press F1 for help") );
        }


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
    m_pFont->InitDeviceObjects( m_pd3dDevice );
    m_pFontSmall->InitDeviceObjects( m_pd3dDevice );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    HRESULT hr;

    m_pFont->RestoreDeviceObjects();
    m_pFontSmall->RestoreDeviceObjects();

    // Setup render states
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Create index buffer
    {
        WORD* pIndices;

        if( FAILED( hr = m_pd3dDevice->CreateIndexBuffer( m_dwNumIndices*sizeof(WORD),
                                                          0, D3DFMT_INDEX16,
                                                          D3DPOOL_DEFAULT, &m_pIB ) ) )
            return hr;

        if( FAILED( hr = m_pIB->Lock( 0, m_dwNumIndices*sizeof(WORD), (BYTE**)&pIndices, 0 ) ) )
            return hr;

        for( DWORD y=1; y<m_dwSize; y++ )
        {
            for( DWORD x=1; x<m_dwSize; x++ )
            {
                *pIndices++ = (WORD)( (y-1)*m_dwSize + (x-1) );
                *pIndices++ = (WORD)( (y-0)*m_dwSize + (x-1) );
                *pIndices++ = (WORD)( (y-1)*m_dwSize + (x-0) );

                *pIndices++ = (WORD)( (y-1)*m_dwSize + (x-0) );
                *pIndices++ = (WORD)( (y-0)*m_dwSize + (x-1) );
                *pIndices++ = (WORD)( (y-0)*m_dwSize + (x-0) );
            }
        }

        if( FAILED( hr = m_pIB->Unlock() ) )
            return hr;
    }


    // Create vertex buffer
    {
        D3DXVECTOR2 *pVertices;

        if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( m_dwNumVertices*sizeof(D3DXVECTOR2),
                                                           0, 0,
                                                           D3DPOOL_DEFAULT, &m_pVB ) ) )
            return hr;

        if( FAILED( hr = m_pVB->Lock( 0, m_dwNumVertices*sizeof(D3DXVECTOR2), (BYTE**)&pVertices, 0 ) ) )
            return hr;

        for( DWORD y=0; y<m_dwSize; y++ )
        {
            for( DWORD x=0; x<m_dwSize; x++ )
            {
                *pVertices++ = D3DXVECTOR2( ((float)x / (float)(m_dwSize-1) - 0.5f) * D3DX_PI,
                                            ((float)y / (float)(m_dwSize-1) - 0.5f) * D3DX_PI );
            }
        }

        if( FAILED( hr = m_pVB->Unlock() ) )
            return hr;
    }

    // Create vertex shader
    {
        TCHAR        strVertexShaderPath[512];
        LPD3DXBUFFER pCode;

        DWORD dwDecl[] =
        {
            D3DVSD_STREAM(0),
            D3DVSD_REG(D3DVSDE_POSITION,  D3DVSDT_FLOAT2),
            D3DVSD_END()
        };

        // Find the vertex shader file
        DXUtil_FindMediaFile( strVertexShaderPath, _T("Ripple.vsh") );

        // Assemble the vertex shader from the file
        if( FAILED( hr = D3DXAssembleShaderFromFile( strVertexShaderPath, 
                                                     0, NULL, &pCode, NULL ) ) )
            return hr;

        // Create the vertex shader
        hr = m_pd3dDevice->CreateVertexShader( dwDecl, (DWORD*)pCode->GetBufferPointer(),
                                               &m_dwShader, 0 );
        pCode->Release();
        if( FAILED(hr) )
            return hr;
    }

    // Set up the projection matrix
    FLOAT fAspectRatio = (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovRH( &m_matProj, D3DXToRadian(60.0f), fAspectRatio, 0.1f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_matProj );

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
    SAFE_RELEASE( m_pIB );
    SAFE_RELEASE( m_pVB );
    m_pd3dDevice->DeleteVertexShader( m_dwShader );

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
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->VertexShaderVersion < D3DVS_VERSION(1,0) )
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
    // Handle key presses
    switch( uMsg )
    {
        case WM_KEYUP:
            m_bKey[wParam] = 0;
            break;

        case WM_KEYDOWN:
            m_bKey[wParam] = 1;
            break;

        case WM_COMMAND:
            {
                switch( LOWORD(wParam) )
                {
                case IDM_TOGGLEHELP:
                    m_bShowHelp = !m_bShowHelp;
                    break;
                }
            }
    }

    // Pass remaining messages to default handler
    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




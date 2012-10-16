//-----------------------------------------------------------------------------
// File: StencilDepth.cpp
//
// Desc: Example code showing how to use stencil buffers to show the depth
//       complexity of a scene.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
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
    CD3DFont* m_pFont;
    BOOL      m_bShowDepthComplexity;
    CD3DFile* m_pFileObject;

    LPDIRECT3DVERTEXBUFFER8 m_pBigSquareVB;

    VOID    SetStatesForRecordingDepthComplexity();
    VOID    ShowDepthComplexity();
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

    D3DXMATRIX  m_matWorldMatrix;

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
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("StencilDepth: Displaying Depth Complexity");
    m_bUseDepthBuffer   = TRUE;
    m_dwMinDepthBits    = 16;
    m_dwMinStencilBits  = 4;

    m_bShowDepthComplexity = TRUE;
    m_pFont                = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pFileObject          = new CD3DFile();
    m_pBigSquareVB         = NULL;
    
    D3DXMatrixIdentity( &m_matWorldMatrix );

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
    // Setup the world spin matrix
    D3DXMatrixRotationAxis( &m_matWorldMatrix, &D3DXVECTOR3(1.0f,1.0f,0.0f), m_fTime/2  );      

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetStatesForRecordingDepthComplexity()
// Desc: Turns on stencil and other states for recording the depth complexity
//       during the rendering of a scene.
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::SetStatesForRecordingDepthComplexity()
{
    // Clear the stencil buffer
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_STENCIL, 0x0, 1.0f, 0L );

    // Turn stenciling
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,     D3DCMP_ALWAYS );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,      0 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,     0x00000000 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK,0xffffffff );

    // Increment the stencil buffer for each pixel drawn
    m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_INCRSAT );
}




//-----------------------------------------------------------------------------
// Name: ShowDepthComplexity()
// Desc: Draws the contents of the stencil buffer in false color. Use alpha
//       blending of one red, one green, and one blue rectangle to do false
//       coloring of bits 1, 2, and 4 in the stencil buffer.
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::ShowDepthComplexity()
{
    // Turn off the buffer, and enable alpha blending
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR );

    // Set up the stencil states
    m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_NOTEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,   0 );

    // Set the background to black
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );

    // Set render states for drawing a rectangle that covers the viewport.
    // The color of the rectangle will be passed in D3DRS_TEXTUREFACTOR
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW );
    m_pd3dDevice->SetStreamSource( 0, m_pBigSquareVB, sizeof(D3DXVECTOR4) );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

    // Draw a red rectangle wherever the 1st stencil bit is set
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK, 0x01 );
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0xffff0000 );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Draw a green rectangle wherever the 2nd stencil bit is set
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK, 0x02 );
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0xff00ff00 );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Draw a blue rectangle wherever the 3rd stencil bit is set
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK, 0x04 );
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0xff0000ff );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Restore states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
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
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff,
                   1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        if( m_bShowDepthComplexity )
            SetStatesForRecordingDepthComplexity();

        // Render the scene
        m_pFileObject->Render( m_pd3dDevice, &m_matWorldMatrix );

        // Show the depth complexity of the scene
        if( m_bShowDepthComplexity )
            ShowDepthComplexity();

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

    // Load a .X file
    if( FAILED( m_pFileObject->Create( m_pd3dDevice, _T("Heli.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Create a big square for rendering the stencilbuffer contents
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(D3DXVECTOR4),
                                         D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW,
                                         D3DPOOL_MANAGED, &m_pBigSquareVB ) ) )
        return E_FAIL;

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

    if( FAILED( hr = m_pFileObject->RestoreDeviceObjects( m_pd3dDevice ) ) )
        return hr;

    // Setup textures (the .X file may have textures)
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX,    TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x00000000 );

    // Set the transform matrices
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f,-15.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
    D3DXMATRIX  matWorld, matView, matProj;

    D3DXMatrixIdentity( &m_matWorldMatrix );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );

    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Setup a material
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, -1.0f, 0.0f );
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Resize the quad used to render the stencilbuffer contents
    D3DXVECTOR4* v;
    FLOAT sx = (FLOAT)m_d3dsdBackBuffer.Width;
    FLOAT sy = (FLOAT)m_d3dsdBackBuffer.Height;
    m_pBigSquareVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0] = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
    v[1] = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
    v[2] = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
    v[3] = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
    m_pBigSquareVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pFileObject->InvalidateDeviceObjects();

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
    m_pFileObject->Destroy();

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
    SAFE_DELETE( m_pFileObject );

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
    // Make sure device supports directional lights
    if( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( 0 == ( pCaps->VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS ) )
            return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    if( WM_COMMAND == uMsg )
    {
        switch( LOWORD(wParam) )
        {
            case IDM_SHOWDEPTHCOMPLEXITY:
                m_bShowDepthComplexity = !m_bShowDepthComplexity;
                CheckMenuItem( GetMenu(hWnd), IDM_SHOWDEPTHCOMPLEXITY,
                          m_bShowDepthComplexity ? MF_CHECKED : MF_UNCHECKED );
                break;
        }
    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




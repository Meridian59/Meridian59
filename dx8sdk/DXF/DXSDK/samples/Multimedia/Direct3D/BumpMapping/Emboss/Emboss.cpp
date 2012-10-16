//-----------------------------------------------------------------------------
// File: Emboss.cpp
//
// Desc: Shows how to do a bumpmapping technique called emobssing, in which a
//       heightmap is subtracted from itself, with slightly offset texture
//       coordinates for the second pass.
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
struct EMBOSSVERTEX 
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu,  tv;
    FLOAT       tu2, tv2;
};

#define D3DFVF_EMBOSSVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2)




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*    m_pFont;                    // Font for drawing text
    CD3DMesh*    m_pObject;                  // Object to render
    D3DLIGHT8    m_Light;                    // The light
    BOOL         m_bShowEmbossMethod;        // Whether to do the embossing

    LPDIRECT3DTEXTURE8 m_pEmbossTexture;     // The emboss texture
    D3DXVECTOR3  m_vBumpLightPos;            // Light position
    D3DXVECTOR3* m_pTangents;                // Array of vertex tangents
    D3DXVECTOR3* m_pBinormals;               // Array of vertex binormals

    // Internal functions
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID    ApplyEnvironmentMap();
    VOID    ComputeTangentsAndBinormals();

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
    m_strWindowTitle    = _T("Emboss: BumpMapping Technique");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pObject           = new CD3DMesh();
    m_pEmbossTexture    = NULL;
    m_bShowEmbossMethod = TRUE;
    m_pTangents         = NULL;
    m_pBinormals        = NULL;
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
// Name: ComputeTangentVector()
// Desc: To find a tangent that heads in the direction of +tv(texcoords), find
//       the components of both vectors on the tangent surface, and add a 
//       linear combination of the two projections that head in the +tv direction
//-----------------------------------------------------------------------------
D3DXVECTOR3 ComputeTangentVector( EMBOSSVERTEX pVtxA, EMBOSSVERTEX pVtxB, 
                                  EMBOSSVERTEX pVtxC )
{
    D3DXVECTOR3 vAB = pVtxB.p - pVtxA.p;
    D3DXVECTOR3 vAC = pVtxC.p - pVtxA.p;
    D3DXVECTOR3 n   = pVtxA.n;

    // Components of vectors to neghboring vertices that are orthogonal to the
    // vertex normal
    D3DXVECTOR3 vProjAB = vAB - ( D3DXVec3Dot( &n, &vAB ) * n );
    D3DXVECTOR3 vProjAC = vAC - ( D3DXVec3Dot( &n, &vAC ) * n );

    // tu and tv texture coordinate differences
    FLOAT duAB = pVtxB.tu - pVtxA.tu;
    FLOAT duAC = pVtxC.tu - pVtxA.tu;
    FLOAT dvAB = pVtxB.tv - pVtxA.tv;
    FLOAT dvAC = pVtxC.tv - pVtxA.tv;

    if( duAC*dvAB > duAB*dvAC )
    {
        duAC = -duAC;
        duAB = -duAB;
    }
    
    D3DXVECTOR3 vTangent = duAC*vProjAB - duAB*vProjAC;
    D3DXVec3Normalize( &vTangent, &vTangent );
    return vTangent;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::ComputeTangentsAndBinormals()
{
    EMBOSSVERTEX* pVertices;
    WORD*         pIndices;
    DWORD         dwNumVertices;
    DWORD         dwNumIndices;

    // Gain access to the object's vertex and index buffers
    LPDIRECT3DVERTEXBUFFER8 pVB;
    m_pObject->GetSysMemMesh()->GetVertexBuffer( &pVB );
    pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );
    dwNumVertices = m_pObject->GetSysMemMesh()->GetNumVertices();

    LPDIRECT3DINDEXBUFFER8 pIB;
    m_pObject->GetSysMemMesh()->GetIndexBuffer( &pIB );
    pIB->Lock( 0, 0, (BYTE**)&pIndices, 0 );
    dwNumIndices  = m_pObject->GetSysMemMesh()->GetNumFaces() * 3;

    // Allocate space for the vertices' tangents and binormals
    m_pTangents  = new D3DXVECTOR3[dwNumVertices];
    m_pBinormals = new D3DXVECTOR3[dwNumVertices];
    ZeroMemory( m_pTangents,  sizeof(D3DXVECTOR3)*dwNumVertices );
    ZeroMemory( m_pBinormals, sizeof(D3DXVECTOR3)*dwNumVertices );

    // Generate the vertices' tangents and binormals
    for( DWORD i=0; i<dwNumIndices; i+=3 )
    {
        WORD a = pIndices[i+0];
        WORD b = pIndices[i+1];
        WORD c = pIndices[i+2];

        // To find a tangent that heads in the direction of +tv(texcoords),
        // find the components of both vectors on the tangent surface ,
        // and add a linear combination of the two projections that head in the +tv direction
        m_pTangents[a] += ComputeTangentVector( pVertices[a], pVertices[b], pVertices[c] );
        m_pTangents[b] += ComputeTangentVector( pVertices[b], pVertices[a], pVertices[c] );
        m_pTangents[c] += ComputeTangentVector( pVertices[c], pVertices[a], pVertices[b] );
    }

    for( i=0; i<dwNumVertices; i++ )
    {
        // Normalize the tangents
        D3DXVec3Normalize( &m_pTangents[i], &m_pTangents[i] );

        // Compute the binormals
        D3DXVec3Cross( &m_pBinormals[i], &pVertices[i].n, &m_pTangents[i] );
    }

    // Unlock and release the vertex and index buffers
    pIB->Unlock();
    pVB->Unlock();
    pIB->Release();
    pVB->Release();
}




//-----------------------------------------------------------------------------
// Name: ApplyEnvironmentMap()
// Desc: Performs a calculation on each of the vertices' normals to determine
//       what the texture coordinates should be for the environment map (in this 
//       case the bump map).
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::ApplyEnvironmentMap()
{
    EMBOSSVERTEX* pv;
    DWORD         dwNumVertices;
    dwNumVertices = m_pObject->GetLocalMesh()->GetNumVertices();

    LPDIRECT3DVERTEXBUFFER8 pVB;
    m_pObject->GetLocalMesh()->GetVertexBuffer( &pVB );
    pVB->Lock( 0, 0, (BYTE**)&pv, 0 );

    // Get the World matrix
    D3DXMATRIX  WV,InvWV;
    m_pd3dDevice->GetTransform( D3DTS_WORLD, &WV );
    D3DXMatrixInverse( &InvWV, NULL, &WV );
    
    // Get the current light position in object space
    D3DXVECTOR4 vTransformed;
    D3DXVec3Transform( &vTransformed, (D3DXVECTOR3*)&m_Light.Position, &InvWV );
    m_vBumpLightPos.x = vTransformed.x;
    m_vBumpLightPos.y = vTransformed.y;
    m_vBumpLightPos.z = vTransformed.z;

    // Dimensions of texture needed for shifting tex coords
    D3DSURFACE_DESC d3dsd;
    m_pEmbossTexture->GetLevelDesc( 0, &d3dsd );
    
    // Loop through the vertices, transforming each one and calculating
    // the correct texture coordinates.
    for( WORD i = 0; i < dwNumVertices; i++ )
    {
        // Find light vector in tangent space
        D3DXVECTOR3 vLightToVertex;
        D3DXVec3Normalize( &vLightToVertex, &(m_vBumpLightPos - pv[i].p) );
        
        // Create rotation matrix (rotate into tangent space)
        FLOAT r = D3DXVec3Dot( &vLightToVertex, &pv[i].n );

        if( r < 0.f ) 
        {
            // Don't shift coordinates when light below surface
            pv[i].tu2 = pv[i].tu;
            pv[i].tv2 = pv[i].tv;
        }
        else
        {
            // Shift coordinates for the emboss effect
            D3DXVECTOR2 vEmbossShift;
            vEmbossShift.x = D3DXVec3Dot( &vLightToVertex, &m_pTangents[i] );
            vEmbossShift.y = D3DXVec3Dot( &vLightToVertex, &m_pBinormals[i] );
            D3DXVec2Normalize( &vEmbossShift, &vEmbossShift );
            pv[i].tu2 = pv[i].tu + vEmbossShift.x/d3dsd.Width;
            pv[i].tv2 = pv[i].tv - vEmbossShift.y/d3dsd.Height;
        }
    }

    pVB->Unlock();
    pVB->Release();
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Rotate the object
    D3DXMATRIX matWorld;
    D3DXMatrixRotationY( &matWorld, m_fTime );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

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
                         0x00000000, 1.0f, 0L );

    // Begin the scene 
    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK;

    // Stage 0 is the base texture, with the height map in the alpha channel
    m_pd3dDevice->SetTexture( 0, m_pEmbossTexture );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    
    if( m_bShowEmbossMethod )
    {
        // Stage 1 passes through the RGB channels (SELECTARG2 = CURRENT), and 
        // does a signed add with the inverted alpha channel. The texture coords
        // associated with Stage 1 are the shifted ones, so the result is:
        //    (height - shifted_height) * tex.RGB * diffuse.RGB
        m_pd3dDevice->SetTexture( 1, m_pEmbossTexture );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE|D3DTA_COMPLEMENT );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

        // Set up the alpha blender to multiply the alpha channel (monochrome emboss)
        // with the src color (lighted texture)
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
        m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
    }

    // Render the object
    m_pObject->Render( m_pd3dDevice );
    
    // Restore render states
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    // Draw some text
    m_pFont->DrawText(   2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText(   2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
    m_pFont->DrawText(   2, 40, 0xffffff00, _T("Move the light with the mouse") );
    m_pFont->DrawText(   2, 60, 0xffffff00, _T("Emboss-mode:") );
    m_pFont->DrawText( 130, 60, 0xffffffff, m_bShowEmbossMethod ? _T("ON") : _T("OFF") );

    // End the scene.
    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
	// Init the font
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Load texture map. Note that this is a special textures, which has a
    // height field stored in the alpha channel
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("emboss1.tga"), 
                                       &m_pEmbossTexture, D3DFMT_A8R8G8B8 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Load geometry
    if( FAILED( m_pObject->Create( m_pd3dDevice, _T("tiger.x") ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Set attributes for the geometry
    m_pObject->SetFVF( m_pd3dDevice, D3DFVF_EMBOSSVERTEX );
    m_pObject->UseMeshMaterials( FALSE );

    // Compute the object's tangents and binormals, whaich are needed for the 
    // emboss-tecnhique's texture-coordinate shifting calculations
    ComputeTangentsAndBinormals();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restore device-memory objects and state after a device is created or
//       resized.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // Restore device objects
    m_pObject->RestoreDeviceObjects( m_pd3dDevice );
    m_pFont->RestoreDeviceObjects();

    // Set up the textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    
    // Set the view and projection matrices
    D3DXMATRIX  matView, matProj;
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 0.0f, 3.5f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 1000.0f );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Setup a material
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set up the light
    D3DUtil_InitLight( m_Light, D3DLIGHT_POINT, 5.0f, 5.0f, -20.0f );
    m_Light.Attenuation0 = 1.0f;
    m_pd3dDevice->SetLight( 0, &m_Light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    
    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,  0x00444444 );

    ApplyEnvironmentMap();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pObject->InvalidateDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pEmbossTexture );

    m_pObject->Destroy();
    m_pFont->DeleteDeviceObjects();

    SAFE_DELETE_ARRAY( m_pTangents );
    SAFE_DELETE_ARRAY( m_pBinormals );

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
    SAFE_DELETE( m_pObject );

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

    // This sample uses the ADDSIGNED texture blending mode
    if( 0 == ( pCaps->TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED ) )
        return E_FAIL;
    if( pCaps->MaxTextureBlendStages < 2 )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message 
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam )
{
    switch( msg )
    {
        case WM_COMMAND:
            if( LOWORD(wParam) == IDM_EMBOSSTOGGLE )
                m_bShowEmbossMethod = !m_bShowEmbossMethod;
            break;

        case WM_MOUSEMOVE:
            if( m_pd3dDevice != NULL )
            {
                FLOAT w = (FLOAT)m_d3dsdBackBuffer.Width;
                FLOAT h = (FLOAT)m_d3dsdBackBuffer.Height;
                m_Light.Position.x = 200.0f * ( 0.5f - LOWORD(lParam) / w );
                m_Light.Position.y = 200.0f * ( 0.5f - HIWORD(lParam) / h );
                m_Light.Position.z = 100.0f;
                m_pd3dDevice->SetLight( 0, &m_Light );
                ApplyEnvironmentMap();
            }
            break;
    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}





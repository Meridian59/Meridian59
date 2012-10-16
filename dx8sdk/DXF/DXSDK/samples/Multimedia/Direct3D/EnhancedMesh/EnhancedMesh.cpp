//-----------------------------------------------------------------------------
// File: EnhancedMesh.cpp
//
// Desc: Sample showing enhanced meshes in D3D
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <d3dx8.h>
#include "D3DApp.h"
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
    TCHAR               m_strMeshFilename[512];
    TCHAR               m_strInitialDir[512];

    LPD3DXMESH          m_pMeshSysMem;      // system memory version of mesh, lives through resize's
    LPD3DXMESH          m_pMeshEnhanced;    // vid mem version of mesh that is enhanced
    UINT                m_dwNumSegs;        // number of segments per edge (tesselation level)
    D3DXMATERIAL*       m_pMaterials;       // pointer to material info in m_pbufMaterials
    LPDIRECT3DTEXTURE8* m_ppTextures;       // array of textures, entries are NULL if no texture specified
    DWORD               m_dwNumMaterials;   // number of materials

    CD3DFont*           m_pFont;
    CD3DArcBall         m_ArcBall;          // mouse rotation utility
    D3DXVECTOR3         m_vObjectCenter;    // Center of bounding sphere of object
    FLOAT               m_fObjectRadius;    // Radius of bounding sphere of object

    LPD3DXBUFFER        m_pbufMaterials;    // contains both the materials data and the filename strings
    LPD3DXBUFFER        m_pbufAdjacency;    // Contains the adjacency info loaded with the mesh

    BOOL                m_bUseHWNPatches; 

    HRESULT GenerateEnhancedMesh(UINT cNewNumSegs);

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT ConfirmDevice (D3DCAPS8* pCaps, DWORD dwBehaviorFlags, D3DFORMAT fmt);
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
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
    m_strWindowTitle    = _T("Enhanced Mesh - N-Patches");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pMeshSysMem     = NULL;
    m_pMeshEnhanced   = NULL;
    m_pMaterials      = NULL;
    m_ppTextures      = NULL;
    m_dwNumMaterials  = NULL;
    m_pbufMaterials   = NULL;
    m_pbufAdjacency   = NULL;
    m_dwNumSegs       = 2;
    m_bUseHWNPatches  = FALSE;
    
    m_pFont           = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );

    _tcscpy( m_strInitialDir, DXUtil_GetDXSDKMediaPath() );
    _tcscpy( m_strMeshFilename, _T("tiger.x") );
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
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
     // Setup world matrix
    D3DXMATRIX matWorld;
    D3DXMatrixTranslation( &matWorld, -m_vObjectCenter.x,
                                      -m_vObjectCenter.y,
                                      -m_vObjectCenter.z );
    D3DXMatrixMultiply( &matWorld, &matWorld, m_ArcBall.GetRotationMatrix() );
    D3DXMatrixMultiply( &matWorld, &matWorld, m_ArcBall.GetTranslationMatrix() );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0, 0, -2*m_fObjectRadius ),
                                  &D3DXVECTOR3( 0, 0, 0 ),
                                  &D3DXVECTOR3( 0, 1, 0 ) );
    m_pd3dDevice->SetTransform( D3DTS_VIEW,  &matView );

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
        if (m_bUseHWNPatches)
        {
            float fNumSegs;

            fNumSegs = (float)m_dwNumSegs;
            m_pd3dDevice->SetRenderState(D3DRS_PATCHSEGMENTS, *((DWORD*)&fNumSegs));
        }

        // set and draw each of the materials in the mesh
        for( UINT i = 0; i < m_dwNumMaterials; i++ )
        {
            m_pd3dDevice->SetMaterial( &m_pMaterials[i].MatD3D );
            m_pd3dDevice->SetTexture( 0, m_ppTextures[i] );

            m_pMeshEnhanced->DrawSubset( i );
        }

        if (m_bUseHWNPatches)
        {
            m_pd3dDevice->SetRenderState(D3DRS_PATCHSEGMENTS, 0);
        }

        // Output stats
        {
            // Output statistics
            m_pFont->DrawText(   2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
            m_pFont->DrawText(   2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

            TCHAR buffer1[80], buffer2[80];
            _stprintf( buffer1, "NumSegs:" ); 
            _stprintf( buffer2, "%d ", m_dwNumSegs );
            m_pFont->DrawText(   2, 40, 0xffffff00, buffer1 );
            m_pFont->DrawText( 150, 40, 0xffffffff, buffer2 );
            _stprintf( buffer1, "NumFaces:" );
            _stprintf( buffer2, "%d ", (m_pMeshEnhanced == NULL) ? 0 : m_pMeshEnhanced->GetNumFaces() );
            m_pFont->DrawText(   2, 60, 0xffffff00, buffer1 );
            m_pFont->DrawText( 150, 60, 0xffffffff, buffer2 );
            _stprintf( buffer1, "NumVertices:" );
            _stprintf( buffer2, "%d ", (m_pMeshEnhanced == NULL) ? 0 : m_pMeshEnhanced->GetNumVertices() );
            m_pFont->DrawText(   2, 80, 0xffffff00, buffer1 );
            m_pFont->DrawText( 150, 80, 0xffffffff, buffer2 );
        }

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: This creates all device-dependent managed objects, such as managed
//       textures and managed vertex buffers.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    LPDIRECT3DVERTEXBUFFER8 pVB = NULL;
    BYTE*      pVertices = NULL;
    LPD3DXMESH pTempMesh;
    TCHAR      strMeshPath[512];
    HRESULT    hr;

    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Load the mesh from the specified file
    DXUtil_FindMediaFile( strMeshPath, m_strMeshFilename );

    hr = D3DXLoadMeshFromX( strMeshPath, D3DXMESH_SYSTEMMEM, m_pd3dDevice, 
                            &m_pbufAdjacency, &m_pbufMaterials, &m_dwNumMaterials, 
                            &m_pMeshSysMem );
    if( FAILED(hr) )
        // hide error so that we can display a blue screen
        return S_OK;

    // Lock the vertex buffer, to generate a simple bounding sphere
    hr = m_pMeshSysMem->GetVertexBuffer( &pVB );
    if( FAILED(hr) )
        return hr;

    hr = pVB->Lock( 0, 0, &pVertices, 0 );
    if( FAILED(hr) )
    {
        SAFE_RELEASE( pVB );
        return hr;
    }

    hr = D3DXComputeBoundingSphere( pVertices, m_pMeshSysMem->GetNumVertices(), 
                                    m_pMeshSysMem->GetFVF(), &m_vObjectCenter, 
                                    &m_fObjectRadius );
    if( FAILED(hr) )
    {
        pVB->Unlock();
        SAFE_RELEASE( pVB );
        return hr;
    }

    if( 0 == m_dwNumMaterials )
    {
        pVB->Unlock();
        SAFE_RELEASE( pVB );
        return E_INVALIDARG;
    }

    // Get the array of materials out of the returned buffer, allocate a
    // texture array, and load the textures
    m_pMaterials = (D3DXMATERIAL*)m_pbufMaterials->GetBufferPointer();
    m_ppTextures = new LPDIRECT3DTEXTURE8[m_dwNumMaterials];

    for( UINT i=0; i<m_dwNumMaterials; i++ )
    {
        TCHAR strTexturePath[512] = _T("");
        DXUtil_FindMediaFile( strTexturePath, m_pMaterials[i].pTextureFilename );
        if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, strTexturePath, 
                                               &m_ppTextures[i] ) ) )
            m_ppTextures[i] = NULL;
    }

    pVB->Unlock();
    SAFE_RELEASE( pVB );

    // Make sure there are normals, which are required for the tesselation
    // enhancement
    if( !(m_pMeshSysMem->GetFVF() & D3DFVF_NORMAL) )
    {
        hr = m_pMeshSysMem->CloneMeshFVF( m_pMeshSysMem->GetOptions(), 
                                          m_pMeshSysMem->GetFVF() | D3DFVF_NORMAL, 
                                          m_pd3dDevice, &pTempMesh );
        if( FAILED(hr) )
            return hr;

        D3DXComputeNormals( pTempMesh, NULL );

        SAFE_RELEASE( m_pMeshSysMem );
        m_pMeshSysMem = pTempMesh;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GenerateEnhancedMesh()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::GenerateEnhancedMesh( UINT dwNewNumSegs )
{
    LPD3DXMESH pMeshEnhancedSysMem = NULL;
    LPD3DXMESH pMeshTemp;
    HRESULT    hr;

    if (m_pMeshSysMem == NULL)
        return S_OK;

    // if using hw, just copy the mesh
    if (m_bUseHWNPatches)
    {
        hr = m_pMeshSysMem->CloneMeshFVF( D3DXMESH_WRITEONLY | D3DXMESH_NPATCHES | 
            (m_pMeshSysMem->GetOptions() & D3DXMESH_32BIT), 
            m_pMeshSysMem->GetFVF(), m_pd3dDevice, &pMeshTemp );
        if( FAILED(hr) )
            return hr;
    }
    else  // tesselate the mesh in sw
    {

        // Create an enhanced version of the mesh, will be in sysmem since source is
        hr = D3DXTessellateNPatches( m_pMeshSysMem, (DWORD*)m_pbufAdjacency->GetBufferPointer(), 
                                     (float)dwNewNumSegs, FALSE, &pMeshEnhancedSysMem, NULL );
        if( FAILED(hr) )
        {
            // If the tessellate failed, there might have been more triangles or vertices 
            // than can fit into a 16bit mesh, so try cloning to 32bit before tessellation

            hr = m_pMeshSysMem->CloneMeshFVF( D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, 
                m_pMeshSysMem->GetFVF(), m_pd3dDevice, &pMeshTemp );
            if (FAILED(hr))
                return hr;

            hr = D3DXTessellateNPatches( pMeshTemp, (DWORD*)m_pbufAdjacency->GetBufferPointer(), 
                                         (float)dwNewNumSegs, FALSE, &pMeshEnhancedSysMem, NULL );
            if( FAILED(hr) )
            {
                pMeshTemp->Release();
                return hr;
            }

            pMeshTemp->Release();
        }

        // Make a vid mem version of the mesh  
        // Only set WRITEONLY if it doesn't use 32bit indices, because those 
        // often need to be emulated, which means that D3DX needs read-access.
        DWORD dwMeshEnhancedFlags = pMeshEnhancedSysMem->GetOptions() & D3DXMESH_32BIT;
        if( (dwMeshEnhancedFlags & D3DXMESH_32BIT) == 0)
            dwMeshEnhancedFlags |= D3DXMESH_WRITEONLY;
        hr = pMeshEnhancedSysMem->CloneMeshFVF( dwMeshEnhancedFlags, m_pMeshSysMem->GetFVF(),
                                                m_pd3dDevice, &pMeshTemp );
        if( FAILED(hr) )
        {
            SAFE_RELEASE( pMeshEnhancedSysMem );
            return hr;
        }

        // Latch in the enhanced mesh
        SAFE_RELEASE( pMeshEnhancedSysMem );
    }

    SAFE_RELEASE( m_pMeshEnhanced );
    m_pMeshEnhanced = pMeshTemp;
    m_dwNumSegs     = dwNewNumSegs;

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

    m_bUseHWNPatches = (m_d3dCaps.DevCaps & D3DDEVCAPS_NPATCHES);

    hr = GenerateEnhancedMesh( m_dwNumSegs );
    if( FAILED(hr) )
        return hr;

    // Setup render state
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,      0x33333333 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    // Setup the light
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f,-1.0f, 1.0f );
    m_pd3dDevice->SetLight(0, &light );
    m_pd3dDevice->LightEnable(0, TRUE );

    // Setup the arcball parameters
    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height, 0.85f );
    m_ArcBall.SetRadius( 1.0f );

    // Setup the projection matrix
    D3DXMATRIX matProj;
    FLOAT      fAspect = (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 
                                m_fObjectRadius/64.0f, m_fObjectRadius*200.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Restore the font
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();

    SAFE_RELEASE( m_pMeshEnhanced );

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

    for( UINT i = 0; i < m_dwNumMaterials; i++ )
        SAFE_RELEASE( m_ppTextures[i] );
    SAFE_DELETE_ARRAY( m_ppTextures );
    SAFE_RELEASE( m_pMeshSysMem );
    SAFE_RELEASE( m_pbufMaterials );
    m_dwNumMaterials = 0L;

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE( m_pFont );

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

    // Trap context menu
    if( WM_CONTEXTMENU == uMsg )
        return 0;

    // Handle key presses
    if( WM_KEYDOWN == uMsg )
    {
        if( VK_UP == (int)wParam )
            GenerateEnhancedMesh( m_dwNumSegs + 1 );
        
        if( VK_DOWN == (int)wParam )
            GenerateEnhancedMesh( max( 1, m_dwNumSegs - 1 ) );

        if( 'W' == (int)wParam )
            m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

        if( 'S' == (int)wParam )
            m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    }
    else if( uMsg == WM_COMMAND )
    {
            // Handle the open file command
        if( LOWORD(wParam) == IDM_OPENFILE )
        {
            TCHAR g_strFilename[512]   = _T("");

            // Display the OpenFileName dialog. Then, try to load the specified file
            OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                                _T(".X Files (.x)\0*.x\0\0"), 
                                NULL, 0, 1, m_strMeshFilename, 512, g_strFilename, 512, 
                                m_strInitialDir, _T("Open Mesh File"), 
                                OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

            if( TRUE == GetOpenFileName( &ofn ) )
            {
                _tcscpy( m_strInitialDir, m_strMeshFilename );
                TCHAR* pLastSlash =  _tcsrchr( m_strInitialDir, _T('\\') );
                if( pLastSlash )
                    *pLastSlash = 0;
                SetCurrentDirectory( m_strInitialDir );

                // Destroy and recreate everything
                InvalidateDeviceObjects();
                DeleteDeviceObjects();
                InitDeviceObjects();
                RestoreDeviceObjects();
            }
        }
    }


    // Pass remaining messages to default handler
    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice (D3DCAPS8* pCaps, DWORD dwBehavior, 
                                          D3DFORMAT fmt)
{
    // On a non-pure device, if it can do rt-Patches, 
    // it can do n-Patches as well.
    if ((dwBehavior & D3DCREATE_PUREDEVICE) && 
        ((pCaps->DevCaps & D3DDEVCAPS_NPATCHES) == 0) && 
        (pCaps->DevCaps & D3DDEVCAPS_RTPATCHES))
        return E_FAIL;
        
    return S_OK;
}





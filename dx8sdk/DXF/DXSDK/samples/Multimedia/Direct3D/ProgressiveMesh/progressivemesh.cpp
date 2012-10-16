//-----------------------------------------------------------------------------
// File: ProgressiveMesh.cpp
//
// Desc: Sample of creating progressive meshes in D3D
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <stdio.h>
#include <windows.h>
#include <commdlg.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Main class to run this application. Most functionality is inherited
//       from the CD3DApplication base class.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    TCHAR               m_strInitialDir[512];
    TCHAR               m_strMeshFilename[512]; // Filename of mesh
    LPD3DXPMESH*        m_pPMeshes;          
    LPD3DXPMESH         m_pPMeshFull;          
    DWORD               m_cPMeshes;
    DWORD               m_iPMeshCur;

    D3DMATERIAL8*       m_mtrlMeshMaterials;
    LPDIRECT3DTEXTURE8* m_pMeshTextures;        // Array of textures, entries are NULL if no texture specified
    DWORD               m_dwNumMaterials;       // Number of materials

    CD3DArcBall         m_ArcBall;              // Mouse rotation utility
    D3DXVECTOR3         m_vObjectCenter;        // Center of bounding sphere of object
    FLOAT               m_fObjectRadius;        // Radius of bounding sphere of object

    CD3DFont*           m_pFont;                // Font for displaying help
    BOOL                m_bDisplayHelp;
    BOOL                m_bShowOptimized;

public:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    void SetNumVertices(DWORD dwNumMeshVertices);

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
    m_strWindowTitle    = _T("ProgressiveMesh: Using Progressive Meshes in D3D");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    // Initialize member variables
    _tcscpy( m_strInitialDir, DXUtil_GetDXSDKMediaPath() );
    _tcscpy( m_strMeshFilename, _T("Tiger.x") );

    m_pPMeshes           = NULL;
    m_cPMeshes           = 0;
    m_pPMeshFull         = NULL;
    m_bShowOptimized     = TRUE;

    m_dwNumMaterials     = 0L;
    m_mtrlMeshMaterials  = NULL;
    m_pMeshTextures      = NULL;

    m_pFont              = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_bDisplayHelp       = FALSE;
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

    // Set up view matrix
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0, 0,-3*m_fObjectRadius ),
                                  &D3DXVECTOR3( 0, 0, 0 ),
                                  &D3DXVECTOR3( 0, 1, 0 ) );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

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
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0x00000000 );

    // Begin scene rendering
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        if( m_pPMeshes )
        {
            // Set and draw each of the materials in the mesh
            for( DWORD i=0; i<m_dwNumMaterials; i++ )
            {
                m_pd3dDevice->SetMaterial( &m_mtrlMeshMaterials[i] );
                m_pd3dDevice->SetTexture( 0, m_pMeshTextures[i] );
                if (m_bShowOptimized)
                    m_pPMeshes[m_iPMeshCur]->DrawSubset( i );
                else
                    m_pPMeshFull->DrawSubset( i );
            }
        }


        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
        TCHAR strBuffer[512];
        if (m_bShowOptimized)
        {
            if (!m_pPMeshes)
            {
                sprintf( strBuffer, _T("Unoptimized") );
                m_pFont->DrawText( 2, 60, 0xffffff00, strBuffer );

                sprintf( strBuffer, _T("Num Vertices = %ld, Optimized %f"),
                         0, 0.0f);
                m_pFont->DrawText( 2, 40, 0xffffff00, strBuffer );
            }
            else
            {
                sprintf( strBuffer, _T("PMesh %d out of %d"), m_iPMeshCur + 1, m_cPMeshes );
                m_pFont->DrawText( 2, 60, 0xffffff00, strBuffer );

                sprintf( strBuffer, _T("Num Vertices = %ld, Min = %ld, Max = %ld%"),
                         m_pPMeshes[m_iPMeshCur]->GetNumVertices(),
                         m_pPMeshes[m_iPMeshCur]->GetMinVertices(),
                         m_pPMeshes[m_iPMeshCur]->GetMaxVertices());
                m_pFont->DrawText( 2, 40, 0xffffff00, strBuffer );
            }
        }
        else
        {
            sprintf( strBuffer, _T("Unoptimized") );
            m_pFont->DrawText( 2, 60, 0xffffff00, strBuffer );

            sprintf( strBuffer, _T("Num Vertices = %ld, Min = %ld, Max = %ld%"),
                     m_pPMeshFull->GetNumVertices(),
                     m_pPMeshFull->GetMinVertices(),
                     m_pPMeshFull->GetMaxVertices());
            m_pFont->DrawText( 2, 40, 0xffffff00, strBuffer );
        }

        // Output text
        if( m_bDisplayHelp )
        {
            m_pFont->DrawText(  2, 80, 0xffffffff, _T("<F1>\n\n")
                                                   _T("<Up>\n")
                                                   _T("<Down>\n\n")
                                                   _T("<PgUp>\n")
                                                   _T("<PgDn>\n\n")
                                                   _T("<Home>\n")
                                                   _T("<End>\n")
                                                   _T("<o>"), 0L );
            m_pFont->DrawText( 70, 80, 0xffffffff, _T("Toggle help\n\n")
                                                   _T("Add one vertex\n")
                                                   _T("Subtract one vertex\n\n")
                                                   _T("Add 100 vertices\n")
                                                   _T("Subtract 100 vertices\n\n")
                                                   _T("Max vertices\n")
                                                   _T("Min vertices\n")
                                                   _T("Show optimized pmeshes") );
        }
        else
        {
            m_pFont->DrawText(  2, 80, 0xffffffff, _T("<F1>\n") );
            m_pFont->DrawText( 70, 80, 0xffffffff, _T("Toggle help\n") );
        }

        // End the scene rendering
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
    DWORD cVerticesPerMesh;

    // Initialize the font
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    CheckMenuItem( GetMenu(m_hWnd), ID_OPTIONS_SHOWOPTIMIZEDPMESHES, m_bShowOptimized ? MF_CHECKED : MF_UNCHECKED );

    // Load mesh
    LPD3DXBUFFER pAdjacencyBuffer = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVertexBuffer = NULL;
    LPD3DXMESH   pMesh = NULL;
    LPD3DXPMESH  pPMesh = NULL;
    LPD3DXMESH   pTempMesh;
    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;
    BYTE*        pVertices;
    TCHAR        strMediaPath[512];
    HRESULT      hr;
    DWORD        dw32BitFlag;
    DWORD        cVerticesMin;
    DWORD        cVerticesMax;
    DWORD        iPMesh;
    D3DXWELDEPSILONS Epsilons;
    DWORD        i;
    D3DXMATERIAL* d3dxMaterials;

    // Find the path to the mesh
    if( FAILED( DXUtil_FindMediaFile( strMediaPath, m_strMeshFilename ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Load the mesh from the specified file
    if( FAILED( hr = D3DXLoadMeshFromX( strMediaPath, D3DXMESH_MANAGED, m_pd3dDevice,
                                        &pAdjacencyBuffer, &pD3DXMtrlBuffer,
                                        &m_dwNumMaterials, &pMesh ) ) )
    {
        // hide error so that device changes will not cause exit, shows blank screen instead
        goto End;
    }

    dw32BitFlag = (pMesh->GetOptions() & D3DXMESH_32BIT);

    // perform simple cleansing operations on mesh
    if( FAILED( hr = D3DXCleanMesh( pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(), &pTempMesh, 
                                           (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL ) ) )
    {
        m_dwNumMaterials = 0;
        goto End;
    }
    SAFE_RELEASE(pMesh);
    pMesh = pTempMesh;

    //  Perform a weld to try and remove excess vertices like the model bigship1.x in the DX8.0 SDK (current model is fixed)
    //    Weld the mesh using all epsilons of 0.0f.  A small epsilon like 1e-6 works well too
    memset(&Epsilons, 0, sizeof(D3DXWELDEPSILONS));
    if( FAILED( hr = D3DXWeldVertices( pMesh, &Epsilons, 
                                                (DWORD*)pAdjacencyBuffer->GetBufferPointer(), 
                                                (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL ) ) )
    {
        m_dwNumMaterials = 0;
        goto End;
    }

    // verify validity of mesh for simplification
    if( FAILED( hr = D3DXValidMesh( pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL ) ) )
    {
        m_dwNumMaterials = 0;
        goto End;
    }

    // Allocate a material/texture arrays
    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    m_mtrlMeshMaterials = new D3DMATERIAL8[m_dwNumMaterials];
    m_pMeshTextures     = new LPDIRECT3DTEXTURE8[m_dwNumMaterials];

    // Copy the materials and load the textures
    for( i=0; i<m_dwNumMaterials; i++ )
    {
        m_mtrlMeshMaterials[i] = d3dxMaterials[i].MatD3D;
        m_mtrlMeshMaterials[i].Ambient = m_mtrlMeshMaterials[i].Diffuse;

        // Find the path to the texture and create that texture
        DXUtil_FindMediaFile( strMediaPath, d3dxMaterials[i].pTextureFilename );
        if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, strMediaPath, 
                                               &m_pMeshTextures[i] ) ) )
            m_pMeshTextures[i] = NULL;
    }
    pD3DXMtrlBuffer->Release();
    pD3DXMtrlBuffer = NULL;


    // Lock the vertex buffer, to generate a simple bounding sphere
    hr = pMesh->GetVertexBuffer( &pVertexBuffer );
    if( FAILED(hr) )
        goto End;

    hr = pVertexBuffer->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
    if( FAILED(hr) )
        goto End;

    hr = D3DXComputeBoundingSphere( pVertices, pMesh->GetNumVertices(),
                                    pMesh->GetFVF(),
                                    &m_vObjectCenter, &m_fObjectRadius );
    pVertexBuffer->Unlock();
    pVertexBuffer->Release();

    if( FAILED(hr) || m_dwNumMaterials == 0 )
        goto End;

    if ( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        hr = pMesh->CloneMeshFVF( dw32BitFlag|D3DXMESH_MANAGED, pMesh->GetFVF() | D3DFVF_NORMAL, 
                                            m_pd3dDevice, &pTempMesh );
        if (FAILED(hr))
            goto End;

        D3DXComputeNormals( pTempMesh, NULL );

        pMesh->Release();
        pMesh = pTempMesh;
    }

    hr = D3DXGeneratePMesh( pMesh, (DWORD*)pAdjacencyBuffer->GetBufferPointer(),
                            NULL, NULL, 1, D3DXMESHSIMP_VERTEX, &pPMesh);
    if( FAILED(hr) )
        goto End;

    cVerticesMin = pPMesh->GetMinVertices();
    cVerticesMax = pPMesh->GetMaxVertices();

    cVerticesPerMesh = (cVerticesMax - cVerticesMin) / 10;

    m_cPMeshes = max(1, (DWORD)ceil((cVerticesMax - cVerticesMin) / (float)cVerticesPerMesh));
    m_pPMeshes = new LPD3DXPMESH[m_cPMeshes];
    if (m_pPMeshes == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto End;
    }
    memset(m_pPMeshes, 0, sizeof(LPD3DXPMESH) * m_cPMeshes);

    // clone full size pmesh
    hr = pPMesh->ClonePMeshFVF( D3DXMESH_MANAGED | D3DXMESH_VB_SHARE, pPMesh->GetFVF(), m_pd3dDevice, &m_pPMeshFull );
    if (FAILED(hr))
        goto End;

    // clone all the separate pmeshes
    for (iPMesh = 0; iPMesh < m_cPMeshes; iPMesh++)
    {
        hr = pPMesh->ClonePMeshFVF( D3DXMESH_MANAGED | D3DXMESH_VB_SHARE, pPMesh->GetFVF(), m_pd3dDevice, &m_pPMeshes[iPMesh] );
        if (FAILED(hr))
            goto End;

        // trim to appropriate space
        hr = m_pPMeshes[iPMesh]->TrimByVertices(cVerticesMin + cVerticesPerMesh * iPMesh, cVerticesMin + cVerticesPerMesh * (iPMesh+1), NULL, NULL);
        if (FAILED(hr))
            goto End;

        hr = m_pPMeshes[iPMesh]->OptimizeBaseLOD(D3DXMESHOPT_VERTEXCACHE, NULL);
        if (FAILED(hr))
            goto End;
    }

    // set current to be maximum number of vertices
    m_iPMeshCur = m_cPMeshes - 1;
    hr = m_pPMeshes[m_iPMeshCur]->SetNumVertices(cVerticesMax);
    if (FAILED(hr))
        goto End;

    hr = m_pPMeshFull->SetNumVertices(cVerticesMax);
    if (FAILED(hr))
        goto End;
End:
    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pD3DXMtrlBuffer );
    SAFE_RELEASE( pMesh );
    SAFE_RELEASE( pPMesh );

    if (FAILED(hr))
    {
        for (iPMesh = 0; iPMesh < m_cPMeshes; iPMesh++)
        {
            SAFE_RELEASE( m_pPMeshes[iPMesh] );
        }

        delete []m_pPMeshes;
        m_cPMeshes = 0;
        m_pPMeshes = NULL;
        SAFE_RELEASE( m_pPMeshFull )
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    // Setup render state
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    // Setup the light
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, -0.5f, 1.0f );
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00333333 );

    // Set the arcball parameters
    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height,
                         0.85f );
    m_ArcBall.SetRadius( m_fObjectRadius );

    // Set the projection matrix
    D3DXMATRIX matProj;
    FLOAT      fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, m_fObjectRadius/64.0f,
                                m_fObjectRadius*200.0f);
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
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
    DWORD iPMesh;

    m_pFont->DeleteDeviceObjects();

    if( m_pMeshTextures != NULL)
    {
        for( UINT i=0; i<m_dwNumMaterials; i++ )
            SAFE_RELEASE( m_pMeshTextures[i] );
    }
    SAFE_DELETE_ARRAY( m_pMeshTextures );

    SAFE_RELEASE( m_pPMeshFull );
    for (iPMesh = 0; iPMesh < m_cPMeshes; iPMesh++)
    {
        SAFE_RELEASE( m_pPMeshes[iPMesh] );
    }
    m_cPMeshes = 0;
    delete []m_pPMeshes;

    SAFE_DELETE_ARRAY( m_mtrlMeshMaterials );
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

    // Trap the context menu
    if( uMsg == WM_CONTEXTMENU )
        return 0;

    if( uMsg == WM_COMMAND )
    {
        // Handle the about command (which displays help)
        if( LOWORD(wParam) == IDM_TOGGLEHELP )
        {
            m_bDisplayHelp = !m_bDisplayHelp;
            return 0;
        }
        else if ( LOWORD(wParam) == ID_OPTIONS_SHOWOPTIMIZEDPMESHES )
        {
            m_bShowOptimized = !m_bShowOptimized;
            CheckMenuItem( GetMenu(hWnd), ID_OPTIONS_SHOWOPTIMIZEDPMESHES, m_bShowOptimized ? MF_CHECKED : MF_UNCHECKED );
        }


        // Handle the open file command
        if( m_bWindowed && LOWORD(wParam) == IDM_OPENFILE )
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
                
                if( FAILED( InitDeviceObjects() ) )
                {
                    MessageBox( m_hWnd, _T("Error loading mesh: mesh may not\n")
                                        _T("be valid. See debug output for\n")
                                        _T("more information.\n\nPlease select ") 
                                        _T("a different .x file."), 
                                        _T("ProgressiveMesh"), MB_ICONERROR|MB_OK );
                }

                RestoreDeviceObjects();
            }
        }
    }


    if( uMsg == WM_KEYDOWN )
    {
        if( m_pPMeshes )
        {
            DWORD dwNumMeshVertices = m_pPMeshes[m_iPMeshCur]->GetNumVertices();

            if( wParam == VK_UP )
            {
                // Sometimes it is necessary to add more than one
                // vertex when increasing the resolution of a 
                // progressive mesh, so keep adding until the 
                // vertex count increases.
                for( int i = 1; i <= 8; i++ )
                {
                    SetNumVertices( dwNumMeshVertices+i );
                    if( m_pPMeshes[m_iPMeshCur]->GetNumVertices() == dwNumMeshVertices+i )
                        break;
                }
            }
            else if( wParam == VK_DOWN )
                SetNumVertices( dwNumMeshVertices-1 );
            else if( wParam == VK_PRIOR )
                SetNumVertices( dwNumMeshVertices+100 );
            else if( wParam == VK_NEXT )
                SetNumVertices( dwNumMeshVertices<=100 ? 1 : dwNumMeshVertices-100 );
            else if( wParam == VK_HOME )
                SetNumVertices( 0xffffffff );
            else if( wParam == VK_END )
                SetNumVertices( 1 );
        }
    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}



//-----------------------------------------------------------------------------
// Name: SetNumVertices()
// Desc: Sets the number of vertices to display on the current progressive mesh
//-----------------------------------------------------------------------------
void
CMyD3DApplication::SetNumVertices(DWORD dwNumVertices)
{
    m_pPMeshFull->SetNumVertices( dwNumVertices );

    // if current pm valid for desired value, then set the number of vertices directly
    if ((dwNumVertices >= m_pPMeshes[m_iPMeshCur]->GetMinVertices()) && (dwNumVertices <= m_pPMeshes[m_iPMeshCur]->GetMaxVertices()))
    {
        m_pPMeshes[m_iPMeshCur]->SetNumVertices( dwNumVertices );
    }
    else  // search for the right one
    {
        m_iPMeshCur = m_cPMeshes - 1;

        // look for the correct "bin" 
        while (m_iPMeshCur > 0)
        {
            // if number of vertices is less than current max then we found one to fit
            if (dwNumVertices >= m_pPMeshes[m_iPMeshCur]->GetMinVertices())
                break;

            m_iPMeshCur -= 1;
        }

        // set the vertices on the newly selected mesh
        m_pPMeshes[m_iPMeshCur]->SetNumVertices( dwNumVertices );
    }

}

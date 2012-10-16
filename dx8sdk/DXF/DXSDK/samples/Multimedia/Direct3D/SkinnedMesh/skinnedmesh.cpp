//-----------------------------------------------------------------------------
// File: SkinnedMesh.cpp
//
// Desc: Example code showing how to use animated models with skinning.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <tchar.h>
#include <d3d8.h>
#include <d3dx8.h>
#include "resource.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "SkinnedMesh.h"




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
    m_strWindowTitle    = _T("Skinned Mesh");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pFont  = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );

    m_pmcSelectedMesh = NULL;
    m_pframeSelected = NULL;
    m_pdeHead = NULL;
    m_pdeSelected = NULL;

    m_dwFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1;

    m_method = D3DNONINDEXED;

    m_pBoneMatrices = NULL;
    m_maxBones = 0;

    m_szPath[0] = '\0';
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    // This sample wants mixed vertex processing rather than hardware
    // vertex processing so it can fallback to sw processing if the 
    // device supports fewer than three matrices when skinning.
    if( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        return E_FAIL;

    return S_OK;
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
    SDrawElement *pdeCur;
    SFrame *pframeCur;

    pdeCur = m_pdeHead;
    while (pdeCur != NULL)
    {
        pdeCur->fCurTime += m_fElapsedTime * 4800;
        if (pdeCur->fCurTime > 1.0e15f)
            pdeCur->fCurTime = 0;

        pframeCur = pdeCur->pframeAnimHead;
        while (pframeCur != NULL)
        {
            pframeCur->SetTime(pdeCur->fCurTime);
            pframeCur = pframeCur->pframeAnimNext;
        }

        pdeCur = pdeCur->pdeNext;
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
    // Set up viewing postion from ArcBall
    SDrawElement *pdeCur;
    D3DXMATRIXA16 mat;
    pdeCur = m_pdeHead;
    while (pdeCur != NULL)
    {
        pdeCur->pframeRoot->matRot = *m_ArcBall.GetRotationMatrix();
        pdeCur->pframeRoot->matTrans = *m_ArcBall.GetTranslationMatrix();
        pdeCur = pdeCur->pdeNext;
    }
    
    // Clear the viewport
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(89,135,179), 1.0f, 0 );

    if (m_pdeHead == NULL)
    {
        return S_OK;
    }

    // Begin the scene 
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        UINT cTriangles = 0;
        HRESULT hr;
        SDrawElement *pdeCur;
        D3DXMATRIXA16 mCur;
        D3DXVECTOR3 vTemp;

        D3DXMatrixTranslation(&m_mView, 0, 0, -m_pdeSelected->fRadius * 2.8f);

        hr = m_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_mView);
        if(FAILED(hr))
            return hr;

        pdeCur = m_pdeHead;
        while (pdeCur != NULL)
        {
            D3DXMatrixIdentity(&mCur);

            hr = UpdateFrames(pdeCur->pframeRoot, mCur);
            if (FAILED(hr))
                return hr;
            hr = DrawFrames(pdeCur->pframeRoot, cTriangles);
            if (FAILED(hr))
                return hr;

            pdeCur = pdeCur->pdeNext;
        }

        // Show frame rate
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
    HRESULT hr = S_OK;

    // Restore the fonts
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // if no filename, use the default
    if (m_szPath[0] == '\0')
    {
        DXUtil_FindMediaFile( m_szPath, _T("tiny.x") );
    }

    LoadMeshHierarchy();

    if( m_pdeHead != NULL)
        m_ArcBall.SetRadius( m_pdeHead->fRadius );
    m_ArcBall.SetRightHanded( TRUE );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // Restore the fonts
    m_pFont->RestoreDeviceObjects();

    HRESULT hr = S_OK;
    D3DLIGHT8 light;

    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height, 2.0f );

    if (m_pdeSelected != NULL)
        SetProjectionMatrix();

    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );

    m_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );

    // Create vertex shader for the indexed skinning
    DWORD dwIndexedVertexDecl1[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 2, D3DVSDT_D3DCOLOR ), // Blend indices
//        D3DVSD_REG( 2, D3DVSDT_UBYTE4 ), // Blend indices
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 4, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    DWORD dwIndexedVertexDecl2[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 1, D3DVSDT_FLOAT1 ), // Blend weights
        D3DVSD_REG( 2, D3DVSDT_D3DCOLOR ), // Blend indices
//        D3DVSD_REG( 2, D3DVSDT_UBYTE4 ), // Blend indices
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 4, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    DWORD dwIndexedVertexDecl3[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 1, D3DVSDT_FLOAT2 ), // Blend weights
        D3DVSD_REG( 2, D3DVSDT_D3DCOLOR ), // Blend indices
//        D3DVSD_REG( 2, D3DVSDT_UBYTE4 ), // Blend indices
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 4, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    DWORD dwIndexedVertexDecl4[] =
    {
        D3DVSD_STREAM( 0 ),
        D3DVSD_REG( 0, D3DVSDT_FLOAT3 ), // Position of first mesh
        D3DVSD_REG( 1, D3DVSDT_FLOAT3 ), // Blend weights
        D3DVSD_REG( 2, D3DVSDT_D3DCOLOR ), // Blend indices
//        D3DVSD_REG( 2, D3DVSDT_UBYTE4 ), // Blend indices
        D3DVSD_REG( 3, D3DVSDT_FLOAT3 ), // Normal
        D3DVSD_REG( 4, D3DVSDT_FLOAT2 ), // Tex coords
        D3DVSD_END()
    };

    DWORD* dwIndexedVertexDecl[] = {dwIndexedVertexDecl1, dwIndexedVertexDecl2, dwIndexedVertexDecl3, dwIndexedVertexDecl4};

    LPD3DXBUFFER pCode;

    DWORD bUseSW = D3DUSAGE_SOFTWAREPROCESSING;
    if (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
    {
        bUseSW = 0;
    }

    for (DWORD i = 0; i < 4; ++i)
    {
        // Assemble the vertex shader file
        if( FAILED( hr = D3DXAssembleShaderFromResource(NULL, MAKEINTRESOURCE(IDD_SHADER1 + i), 0, NULL, &pCode, NULL ) ) )
            return hr;

        // Create the vertex shader
        if( FAILED( hr = m_pd3dDevice->CreateVertexShader( dwIndexedVertexDecl[i], 
                                             (DWORD*)pCode->GetBufferPointer(),
                                             &(m_dwIndexedVertexShader[i]) , bUseSW ) ) )
        {
            return hr;
        }

        pCode->Release();
    }
    
    ZeroMemory( &light, sizeof(light) );
    light.Type = D3DLIGHT_DIRECTIONAL;

    light.Diffuse.r = 1.0;
    light.Diffuse.g = 1.0;
    light.Diffuse.b = 1.0;
    light.Specular.r = 0;
    light.Specular.g = 0;
    light.Specular.b = 0;
    light.Ambient.r = 0.25;
    light.Ambient.g = 0.25;
    light.Ambient.b = 0.25;

    light.Direction = D3DXVECTOR3( 0.0f, 0.0f, -1.0f);

    hr = m_pd3dDevice->SetLight(0, &light );
    if (FAILED(hr))
        return E_FAIL;

    hr = m_pd3dDevice->LightEnable(0, TRUE);
    if (FAILED(hr))
        return E_FAIL;

    // Set Light for vertex shader
    D3DXVECTOR4 vLightDir( 0.0f, 0.0f, 1.0f, 0.0f );
    m_pd3dDevice->SetVertexShaderConstant(1, &vLightDir, 1);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReleaseDeviceDependentMeshes()
// Desc: 
//-----------------------------------------------------------------------------
void ReleaseDeviceDependentMeshes(SFrame* pframe)
{
    if (pframe->pmcMesh != NULL)
    {
        for (SMeshContainer* pmcCurr = pframe->pmcMesh; pmcCurr != NULL; pmcCurr = pmcCurr->pmcNext)
        {
            if (pmcCurr->m_pSkinMesh != NULL)
            {
                GXRELEASE(pmcCurr->pMesh);

                pmcCurr->m_Method = NONE;
            }
        }
    }

    if (pframe->pframeFirstChild != NULL)
        ReleaseDeviceDependentMeshes(pframe->pframeFirstChild);

    if (pframe->pframeSibling != NULL)
        ReleaseDeviceDependentMeshes(pframe->pframeSibling);
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();


    for (SDrawElement* pdeCurr = m_pdeHead; pdeCurr != NULL; pdeCurr = pdeCurr->pdeNext)
    {
        ReleaseDeviceDependentMeshes(pdeCurr->pframeRoot);
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
    
    if( m_pdeSelected == m_pdeHead )
        m_pdeSelected = NULL;

    delete m_pdeHead;
    m_pdeHead = NULL;
    
    delete [] m_pBoneMatrices;

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
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Pass mouse messages to the ArcBall so it can build internal matrices
    m_ArcBall.HandleMouseMessages( hWnd, uMsg, wParam, lParam );

    switch( uMsg )
    {
    case WM_COMMAND:
        if ( 0 == HIWORD(wParam) )
        {
            switch ( LOWORD(wParam) )
            {                
            case ID_FILE_OPENMESHHEIRARCHY:
                {
                    OPENFILENAME ofn;
                    memset( &ofn, 0, sizeof(ofn) );
                    static TCHAR file[256];
                    static TCHAR szFilepath[256];
                    static TCHAR fileTitle[256];
                    static TCHAR filter[] =
                        TEXT("X files (*.x)\0*.x\0")
                        TEXT("All Files (*.*)\0*.*\0");
                    _tcscpy( file, TEXT(""));
                    _tcscpy( fileTitle, TEXT(""));
                    
                    ofn.lStructSize       = sizeof(ofn);
                    ofn.hwndOwner         = m_hWnd;
                    ofn.hInstance         = NULL;//m_hInstance;
                    ofn.lpstrFilter       = filter;
                    ofn.lpstrCustomFilter = NULL;
                    ofn.nMaxCustFilter    = 0L;
                    ofn.nFilterIndex      = 1L;
                    ofn.lpstrFile         = file;
                    ofn.nMaxFile          = sizeof(file);
                    ofn.lpstrFileTitle    = fileTitle;
                    ofn.nMaxFileTitle     = sizeof(fileTitle);
                    ofn.lpstrInitialDir   = NULL;
                    ofn.nFileOffset       = 0;
                    ofn.nFileExtension    = 0;
                    ofn.lpstrDefExt       = TEXT("*.x");
                    ofn.lCustData         = 0;
                    
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                    if ( ! GetOpenFileName( &ofn) )
                    {
                        TCHAR s[40];
                        DWORD dwErr = CommDlgExtendedError();
                        if ( 0 != dwErr )
                        {
                            wsprintf( s, "GetOpenFileName failed with %x", dwErr );
                            MessageBox( m_hWnd, s, "TexWin", MB_OK | MB_SYSTEMMODAL );
                        }
                        return 0;
                    }
                    lstrcpy(m_szPath, ofn.lpstrFile);
                    HRESULT hr = LoadMeshHierarchy();
                    if (FAILED(hr))
                        MessageBox(NULL, "Could not open file or incorrect file type", "Error loading file", MB_OK);

                    return 0;
                }
            case ID_OPTIONS_D3DINDEXED:
                {
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXED, MF_CHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DNONINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_SOFTWARESKINNING, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXEDVS, MF_UNCHECKED);
                    m_method = D3DINDEXED;
                    break;
                }
            case ID_OPTIONS_D3DINDEXEDVS:
                {
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DNONINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_SOFTWARESKINNING, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXEDVS, MF_CHECKED);
                    m_method = D3DINDEXEDVS;
                    break;
                }
            case ID_OPTIONS_D3DNONINDEXED:
                {
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DNONINDEXED, MF_CHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_SOFTWARESKINNING, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXEDVS, MF_UNCHECKED);
                    m_method = D3DNONINDEXED;
                    break;
                }
            case ID_OPTIONS_SOFTWARESKINNING:
                {
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DNONINDEXED, MF_UNCHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_SOFTWARESKINNING, MF_CHECKED);
                    CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_D3DINDEXEDVS, MF_UNCHECKED);
                    m_method = SOFTWARE;
                    break;
                }
            }
        }
        break;
    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: SetProjectionMatrix()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::SetProjectionMatrix()
{
    D3DXMATRIX mat;

    if (m_pdeHead == NULL)
        return S_OK;

    FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovRH(&mat, 0.25f*3.141592654f, fAspect, m_pdeSelected->fRadius / 64, m_pdeSelected->fRadius * 200);
    HRESULT hr = m_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&mat );
    if (FAILED(hr))
        return hr;
    // Set Projection Matrix for vertex shader
    D3DXMatrixTranspose(&mat, &mat);
    return m_pd3dDevice->SetVertexShaderConstant(2, &mat, 4);
}

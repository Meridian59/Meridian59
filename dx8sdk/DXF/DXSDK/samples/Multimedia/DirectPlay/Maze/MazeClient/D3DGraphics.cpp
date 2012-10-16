//----------------------------------------------------------------------------
// File: d3dgraphics.cpp
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS
#include <windows.h>
#include <D3DX8.h>
#include <dplay8.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>
#include <process.h>
#include <dxerr8.h>
#include <tchar.h>
#include <dplay8.h>
#include "SyncObjects.h"
#include "DummyConnector.h"
#include "IMazeGraphics.h"
#include "SmartVB.h"
#include "MazeServer.h"
#include "MazeApp.h"
#include "IMazeGraphics.h"
#include "d3dSaver.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "D3DGraphics.h"
#include "resource.h"
#include "DXUtil.h"

static CD3DGraphics* s_pGraphics = NULL;



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CD3DGraphics::CD3DGraphics()
{
    s_pGraphics             = this;
    m_pMazeApp              = NULL;

    m_pSphere               = NULL;
    m_pWallTexture          = NULL;
    m_pFloorTexture         = NULL;
    m_pCeilingTexture       = NULL;
    m_pNetIconTexture       = NULL;
    m_pLocalIconTexture     = NULL;

    // Tell the d3dsaver framework to run on one monitor only
    m_bOneScreenOnly        = TRUE;

    m_bUseDepthBuffer       = TRUE;
 
    // Create fonts
    m_pFont      = new CD3DFont( _T("Courier"), 12, D3DFONT_BOLD );
    m_pStatsFont = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pPlayerMesh  = new CD3DFile();
    lstrcpy( m_strRegPath, MAZE_REGKEYNAME );
    
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CD3DGraphics::~CD3DGraphics()
{
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
VOID CD3DGraphics::Init( CMazeApp* pMazeApp, CDPlay8Client* pDP8Client, 
                         CMazeClient* pMazeClient )
{
    m_pMazeApp      = pMazeApp;
    m_pMazeClient   = pMazeClient;
    m_pDP8Client    = pDP8Client;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::Create( HINSTANCE hInstance )
{
    return CD3DScreensaver::Create( hInstance );
}



//-----------------------------------------------------------------------------
// Name: PostCreate()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::PostCreate()
{
    HRESULT hr;

    // Create the screen saver window
    if( m_SaverMode == sm_test    || 
        m_SaverMode == sm_full )
    {
        if( FAILED( hr = CreateSaverWindow() ) )
        {
            m_bErrorMode = TRUE;
            m_hrError = hr;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Run()
// Desc:
//-----------------------------------------------------------------------------
INT CD3DGraphics::Run()
{
    HRESULT hr;
    BOOL bIsScreenSaver = TRUE;
    TCHAR* strCmdLine = GetCommandLine();

    // Parse the command line and do the appropriate thing
    m_SaverMode = ParseCommandLine( strCmdLine );

    // Skip along to the first option delimiter "/" or "-"
    while ( *strCmdLine && *strCmdLine != '/' && *strCmdLine != '-' )
        strCmdLine++;
    
    // If there weren't any params, then it must have run from a exe form
    if ( *strCmdLine == 0 )
    {
        m_SaverMode = sm_config;
        bIsScreenSaver = FALSE;
    }

    m_pMazeApp->SetAllowLoopback( TRUE );

    switch ( m_SaverMode )
    {
        case sm_config:
            ScreenSaverDoConfig( bIsScreenSaver );
            if( m_dwStartMode == 0 )
                break;

            // Don't do AutoDisconnnect or Stress Connect in 3D client
            m_pConfig = m_pMazeApp->GetConfig();
            m_pConfig->bAutoDisconnnect = FALSE;
            m_pConfig->dwStressConnect = 0;

            if( m_pConfig->bFullScreen )
                m_SaverMode = sm_full;
            else
                m_SaverMode = sm_test;

            m_pMazeApp->SetAllowConnect( TRUE );

            PostCreate();   

            if( FAILED( hr = DoSaver() ) )
                DisplayErrorMsg( hr, 0 );
            break;

        case sm_preview:
/*
            // This is temp code to allow stepping thru code in the preview case
            WNDCLASS    cls;
            cls.hCursor        = NULL; 
            cls.hIcon          = NULL; 
            cls.lpszMenuName   = NULL;
            cls.lpszClassName  = "Parent"; 
            cls.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
            cls.hInstance      = m_hInstance; 
            cls.style          = CS_VREDRAW|CS_HREDRAW|CS_SAVEBITS|CS_DBLCLKS;
            cls.lpfnWndProc    = DefWindowProc;
            cls.cbWndExtra     = 0; 
            cls.cbClsExtra     = 0; 
            RegisterClass( &cls );

            // Create the window
            RECT    rect;
            HWND hwnd;
            rect.left = rect.top = 40;
            rect.right = rect.left+100;
            rect.bottom = rect.top+100;
            AdjustWindowRect( &rect , WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_POPUP , FALSE );
            hwnd= CreateWindow( "Parent", "FakeShell" ,
                                   WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_POPUP , rect.left , rect.top ,
                                   rect.right-rect.left , rect.bottom-rect.top , NULL ,
                                   NULL , m_hInstance , NULL );
            m_hWndParent = hwnd;
*/
        case sm_test:
        case sm_full:
            if( m_SaverMode == sm_preview )
                m_pMazeApp->SetAllowConnect( FALSE );
            else
                m_pMazeApp->SetAllowConnect( TRUE );
            
            // Don't do AutoDisconnnect or StressConnect in 3D client
            m_pConfig = m_pMazeApp->GetConfig();
            m_pConfig->bAutoDisconnnect = FALSE;
            m_pConfig->dwStressConnect = 0;

            if( m_SaverMode != sm_preview && m_pConfig->bFullScreen )
                m_SaverMode = sm_full;
            else
            {
                m_SaverMode = sm_test;
            }

            if( FAILED( hr = DoSaver() ) )
                DisplayErrorMsg( hr, 0 );
            break;
    
        case sm_passwordchange:
            ChangePassword();
            break;
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::Shutdown()
{
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int CD3DGraphics::ScreenSaverDoConfig( BOOL bIsScreenSaverSettings )
{
    m_bIsScreenSaverSettings = bIsScreenSaverSettings;

    m_pMazeApp->ReadConfig();

    HKEY hKey = NULL;
    RegOpenKeyEx( HKEY_CURRENT_USER, MAZE_REGKEYNAME, 0, KEY_READ, &hKey );
    ReadScreenSettings( hKey );
    RegCloseKey( hKey );

    m_pConfig = m_pMazeApp->GetConfig();

    if( DialogBox( m_hInstance, MAKEINTRESOURCE(IDD_SAVERCONFIG), 
                   NULL, StaticSaverConfigDlgProc ) == IDOK )
    {
        m_pMazeApp->WriteConfig();

        HKEY hKey = NULL;
        RegOpenKeyEx( HKEY_CURRENT_USER, MAZE_REGKEYNAME, 0, KEY_READ, &hKey );
        WriteScreenSettings( hKey );
        RegCloseKey( hKey );
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CD3DGraphics::StaticSaverConfigDlgProc( HWND hDlg, UINT uMsg, 
                                                         WPARAM wParam, LPARAM lParam )
{
    return s_pGraphics->SaverConfigDlgProc( hDlg, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CD3DGraphics::SaverConfigDlgProc( HWND hDlg, UINT uMsg, 
                                                   WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
            PopulateSaverConfigDlg( hDlg );
            return TRUE;

        case WM_CLOSE:
            EndDialog( hDlg, IDCANCEL );
            break;

        case WM_COMMAND:
            switch( HIWORD(wParam) )
            {
                case BN_CLICKED:
                    switch( LOWORD(wParam) )
                    {
                        case IDOK:
                            ExtractSaverConfigDlgSettings( hDlg );
                            m_dwStartMode = 0;
                            EndDialog( hDlg, IDOK );
                            break;

                        case IDC_LAUNCH:
                            ExtractSaverConfigDlgSettings( hDlg );
                            m_dwStartMode = 1;
                            EndDialog( hDlg, IDOK );
                            break;

                        case IDC_SCREENSETTINGS:
                            DoScreenSettingsDialog( hDlg );
                            break;

                        case IDCANCEL:
                            EndDialog( hDlg, IDCANCEL );
                            return TRUE;
                    }
                    break;
            }
            break;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CD3DGraphics::PopulateSaverConfigDlg( HWND hDlg )
{
    if( m_bIsScreenSaverSettings )
        ShowWindow( GetDlgItem( hDlg, IDOK ), SW_SHOW );
    else
        ShowWindow( GetDlgItem( hDlg, IDOK ), SW_HIDE );

    CheckDlgButton( hDlg, IDC_MICROSOFT, m_pConfig->bConnectToMicrosoftSite );
    CheckDlgButton( hDlg, IDC_LOCAL_SERVER, m_pConfig->bConnectToLocalServer );
    CheckDlgButton( hDlg, IDC_REMOTE_SERVER, m_pConfig->bConnectToRemoteServer );
    SetDlgItemInt( hDlg, IDC_RETRY_DELAY, m_pConfig->dwNetworkRetryDelay, FALSE );
    CheckDlgButton( hDlg, IDC_FRAMERATE, m_pConfig->bShowFramerate );
    CheckDlgButton( hDlg, IDC_INDICATORS, m_pConfig->bShowIndicators );
    CheckDlgButton( hDlg, IDC_FULLSCREEN, m_pConfig->bFullScreen );
    CheckDlgButton( hDlg, IDC_MINIMAP, m_pConfig->bDrawMiniMap );
    CheckDlgButton( hDlg, IDC_REFLECTIONS, m_pConfig->bReflections );
    CheckDlgButton( hDlg, IDC_FILELOGGING, m_pConfig->bFileLogging );

    SetDlgItemText( hDlg, IDC_IPADDRESS, m_pConfig->szIPAddress );
    SendDlgItemMessage( hDlg, IDC_IPADDRESS, EM_SETLIMITTEXT, sizeof(m_pConfig->szIPAddress)-1, 0 );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CD3DGraphics::ExtractSaverConfigDlgSettings( HWND hDlg )
{
    m_pConfig->bConnectToMicrosoftSite    = IsDlgButtonChecked( hDlg, IDC_MICROSOFT );
    m_pConfig->bConnectToLocalServer      = IsDlgButtonChecked( hDlg, IDC_LOCAL_SERVER );
    m_pConfig->bConnectToRemoteServer     = IsDlgButtonChecked( hDlg, IDC_REMOTE_SERVER );
    m_pConfig->dwNetworkRetryDelay        = GetDlgItemInt( hDlg, IDC_RETRY_DELAY, NULL, FALSE );
    m_pConfig->bShowFramerate             = IsDlgButtonChecked( hDlg, IDC_FRAMERATE );
    m_pConfig->bShowIndicators            = IsDlgButtonChecked( hDlg, IDC_INDICATORS );
    m_pConfig->bFullScreen                = IsDlgButtonChecked( hDlg, IDC_FULLSCREEN );
    m_pConfig->bDrawMiniMap               = IsDlgButtonChecked( hDlg, IDC_MINIMAP );
    m_pConfig->bReflections               = IsDlgButtonChecked( hDlg, IDC_REFLECTIONS );
    m_pConfig->bFileLogging               = IsDlgButtonChecked( hDlg, IDC_FILELOGGING );

    GetDlgItemText( hDlg, IDC_IPADDRESS, m_pConfig->szIPAddress, sizeof(m_pConfig->szIPAddress) );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CD3DGraphics::HandleOutputMsg( EnumLineType enumLineType, TCHAR* strLine )
{
    // Nothing needs to be done here
    return;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
LRESULT CD3DGraphics::SaverProc( HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam )
{
    switch( message )
    {
        case WM_KEYDOWN:
            if( wParam == 'R' )
                m_pConfig->bReflections = !m_pConfig->bReflections;
            else if( wParam == 'A' )
                m_pMazeClient->EngageAutopilot( !m_pMazeClient->IsAutopilot() );
            break;

        case WM_POWERBROADCAST:
            switch( wParam )
            {
                #ifndef PBT_APMQUERYSUSPEND  // Defines are here for old compilers
                    #define PBT_APMQUERYSUSPEND 0x0000
                #endif
                #ifndef BROADCAST_QUERY_DENY
                    #define BROADCAST_QUERY_DENY         0x424D5144 
                #endif
                case PBT_APMQUERYSUSPEND:
                    // Tell the OS not to suspend, otherwise we
                    // will loose the network connection.
                    return BROADCAST_QUERY_DENY;

                #ifndef PBT_APMRESUMESUSPEND
                    #define PBT_APMRESUMESUSPEND 0x0007
                #endif
                case PBT_APMRESUMESUSPEND:
                    return TRUE;
            }
            break;
    }

    return CD3DScreensaver::SaverProc( hWnd, message, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::ConfirmDevice(D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{ 
    if( dwBehavior & D3DCREATE_PUREDEVICE )
        return E_FAIL; // GetTransform doesn't work on PUREDEVICE

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::OneTimeSceneInit()                         
{ 
    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::CreateTextureFromResource( INT nResource, LPDIRECT3DTEXTURE8* ppTexture )                         
{ 
    if( FAILED( D3DXCreateTextureFromResourceEx( m_pd3dDevice, NULL, MAKEINTRESOURCE( nResource ), 
                        D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, 
                        D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
                        D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0xFF000000, NULL, NULL, ppTexture ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::InitDeviceObjects()
{ 
    HRESULT hr;

    m_dwTesselation = 3;

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    // Restore the fonts
    m_pFont->InitDeviceObjects( m_pd3dDevice );
    m_pStatsFont->InitDeviceObjects( m_pd3dDevice );

    if( FAILED( CreateTextureFromResource( IDB_WALLTEXTURE, &m_pWallTexture ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("IDB_WALLTEXTURE"), D3DAPPERR_MEDIANOTFOUND );
    if( FAILED( CreateTextureFromResource( IDB_FLOORTEXTURE, &m_pFloorTexture ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("IDB_FLOORTEXTURE"), D3DAPPERR_MEDIANOTFOUND );
    if( FAILED( CreateTextureFromResource( IDB_CEILINGTEXTURE, &m_pCeilingTexture ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("IDB_CEILINGTEXTURE"), D3DAPPERR_MEDIANOTFOUND );
    if( FAILED( CreateTextureFromResource( IDB_NETICON, &m_pNetIconTexture ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("IDB_NETICON"), D3DAPPERR_MEDIANOTFOUND );
    if( FAILED( CreateTextureFromResource( IDB_LOCALICON, &m_pLocalIconTexture ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("IDB_LOCALICON"), D3DAPPERR_MEDIANOTFOUND );

    // Create airplane for rendering players
    if( FAILED( m_pPlayerMesh->CreateFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDB_TANK), TEXT("XFILE") ) ) )
        return DXTRACE_ERR( TEXT("tank.x"), D3DAPPERR_MEDIANOTFOUND );

    // Create sphere for rendering clients
    if( FAILED( hr = D3DXCreateSphere( m_pd3dDevice, 0.2f, 8, 8, &m_pSphere, NULL ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("D3DXCreateSphere"), hr );

    if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( sizeof(VERTEX_TL_DIFFUSE) * 4, 
                                                D3DUSAGE_WRITEONLY, FVF_TL_DIFFUSE, 
                                                D3DPOOL_MANAGED, &m_pMiniMapBackgroundVB ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("CreateVertexBuffer"), hr );

    if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( sizeof(VERTEX_TL_TEXTURED) * 4, 
                                                D3DUSAGE_WRITEONLY, FVF_TL_TEXTURED, 
                                                D3DPOOL_MANAGED, &m_pIndicatorVB ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("CreateVertexBuffer"), hr );

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::RestoreDeviceObjects()
{ 
    HRESULT hr;

    // Restore the fonts
    m_pFont->RestoreDeviceObjects();
    m_pStatsFont->RestoreDeviceObjects();

    m_pPlayerMesh->RestoreDeviceObjects( m_pd3dDevice );

    // Initialise the SmartVB for the walls
    if( FAILED( hr = m_SmartVB.Init( m_pD3D, m_pd3dDevice, 2048 ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("m_SmartVB.Init"), hr );

    if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( sizeof(VERTEX_DIFFUSE) * 1024, 
                                                D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, FVF_DIFFUSE, 
                                                D3DPOOL_DEFAULT, &m_pMiniMapVB ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("CreateVertexBuffer"), hr );

    // Set projection matrix
    const float fov = 1.8f;
    const float znear = 0.1f;
    const float zfar = 30.0f;
    const float viewwidth = float(tan(fov*0.5f) ) * znear;
    const float viewheight = viewwidth * float(m_d3dsdBackBuffer.Height)/float(m_d3dsdBackBuffer.Width);
    D3DXMatrixPerspectiveLH( &m_Projection, viewwidth, viewheight, znear, zfar );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_Projection );

    // Set renderstates and lighting
    D3DXMATRIX matIden; D3DXMatrixIdentity( &matIden );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matIden );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x333333 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Init light
    D3DUtil_InitLight( m_Light, D3DLIGHT_POINT, m_vCameraPos.x, m_vCameraPos.y, m_vCameraPos.z );
    m_Light.Direction = D3DXVECTOR3(0,0,0);
    m_Light.Range = 80.0f;
    m_Light.Attenuation0 = 1.0f;
    m_Light.Attenuation1 = 0.0f;
    m_Light.Attenuation2 = 1.0f;
    m_pd3dDevice->SetLight( 0, &m_Light );
    m_pd3dDevice->LightEnable( 0, TRUE );

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::FrameMove()
{ 
    m_pMazeApp->FrameMove( m_fElapsedTime );

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::Render()
{ 
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );

    // Begin the scene 
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        m_vCameraPos = m_pMazeClient->GetCameraPos();
        m_fCameraYaw = AngleToFloat(m_pMazeClient->GetCameraYaw() );

        // Compute and set view matrix
        D3DXMATRIX matTrans,matRot;
        D3DXMatrixTranslation( &matTrans, -m_vCameraPos.x, -m_vCameraPos.y, -m_vCameraPos.z );
        D3DXMatrixRotationY( &matRot, m_fCameraYaw );
        m_Camera = matTrans * matRot;
        m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_Camera );

        // Move light
        m_Light.Position = m_vCameraPos;
        m_pd3dDevice->SetLight( 0, &m_Light );

        // Get visibility set of maze
        D3DXVECTOR2 dir;
        dir.y = float(cos(m_fCameraYaw) );
        dir.x = float(-sin(m_fCameraYaw) );
        m_dwNumVisibleCells = m_pMazeClient->m_Maze.GetVisibleCells( D3DXVECTOR2(m_vCameraPos.x, m_vCameraPos.z) ,
                                                                   dir, 1.8f, m_mcrVisList, MAX_VISLIST );

        // Draw reflection of walls and ceiling in floor
        if( m_pConfig->bReflections )
        {
            D3DXMATRIX reflectcamera = m_Camera;
            reflectcamera.m[1][0] = -reflectcamera.m[1][0];
            reflectcamera.m[1][1] = -reflectcamera.m[1][1];
            reflectcamera.m[1][2] = -reflectcamera.m[1][2];
            reflectcamera.m[1][3] = -reflectcamera.m[1][3];
            m_pd3dDevice->SetTransform( D3DTS_VIEW, &reflectcamera );

            m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

            m_Light.Diffuse = D3DXCOLOR(0.7f,0.7f,0.7f,0);
            m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x222222 );
            m_pd3dDevice->SetLight( 0, &m_Light );

            DrawWalls();
            DrawCeiling();
//            DrawPlayers();

            m_Light.Diffuse = D3DXCOLOR(1,1,1,0);
            m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x333333 );
            m_pd3dDevice->SetLight( 0, &m_Light );

            m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
            m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_Camera );
        }

        // Draw floor
        if( m_pConfig->bReflections )
        {
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
            m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
        }

        DrawFloor();
        if( m_pConfig->bReflections )
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

        // Draw walls and ceiling
        DrawWalls();
        DrawCeiling();

        // Draw Players
        DrawPlayers();

        // Draw mini-map
        if( m_pConfig->bDrawMiniMap )
            DrawMiniMap();

        // Draw indicators
        if( m_pConfig->bShowIndicators )
            DrawIndicators();

        // Show frame rate
        m_pStatsFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pStatsFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawPlayers()
{
    D3DXMATRIX matWorld;
    D3DXMATRIX matScale;
    D3DXMATRIX matTrans;
    D3DXMATRIX matRot;

    D3DXMatrixScaling( &matScale, 0.25f, 0.25f, 0.25f );

    MazeCellRef* pCell = m_mcrVisList;
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTexture( 0, m_pCeilingTexture );

    m_pMazeClient->LockWorld();
    for( DWORD i = 0; i < m_dwNumVisibleCells; i++, pCell++ )
    {
        PlayerObject* pPlayerObject = m_pMazeClient->GetFirstPlayerObjectInCell( pCell->x, pCell->y );
        while( pPlayerObject )
        {
            D3DXMatrixTranslation( &matTrans, pPlayerObject->vPos.x, pPlayerObject->vPos.y - 0.5f, pPlayerObject->vPos.z );
            D3DXMatrixRotationYawPitchRoll( &matRot, -AngleToFloat(pPlayerObject->aCameraYaw), -D3DX_PI/2.0f, 0.0f );
            matWorld = matScale * matRot * matTrans;

            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
            // m_pSphere->DrawSubset( 0 );
            m_pPlayerMesh->Render( m_pd3dDevice );

            pPlayerObject = pPlayerObject->pNext;
        }
    }

    m_pd3dDevice->SetTexture( 0, NULL );

    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    m_pMazeClient->UnlockWorld();
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawFloor()
{
    MazeCellRef*    pCell = m_mcrVisList;
    VERTEX_TEXURED* pVert;
    WORD*           pIndex;
    WORD            offset;
    D3DXVECTOR3     normal(0,1,0);
    DWORD           verts_per_side = (m_dwTesselation+1)*(m_dwTesselation+1);
    DWORD           indicies_per_side = m_dwTesselation*m_dwTesselation*6;

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTexture( 0, m_pFloorTexture );

    m_SmartVB.Begin();
    for( DWORD i = 0; i < m_dwNumVisibleCells; i++, pCell++ )
    {
        D3DXVECTOR3 pos( float(pCell->x), 0, float(pCell->y) );
        m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
        LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(1,0,0), D3DXVECTOR3(0,0,1) ,
                  D3DXVECTOR3(0,1,0) );
    }
    m_SmartVB.End();
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawCeiling()
{
    MazeCellRef*    pCell = m_mcrVisList;
    VERTEX_TEXURED* pVert;
    WORD*           pIndex;
    WORD            offset;
    D3DXVECTOR3     normal(0,1,0);
    DWORD           verts_per_side = (m_dwTesselation+1)*(m_dwTesselation+1);
    DWORD           indicies_per_side = m_dwTesselation*m_dwTesselation*6;

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTexture( 0, m_pCeilingTexture );

    m_SmartVB.Begin();
    for( DWORD i = 0; i < m_dwNumVisibleCells; i++, pCell++ )
    {
        D3DXVECTOR3 pos( float(pCell->x), 1, float(1+pCell->y) );
        m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
        LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(1,0,0), D3DXVECTOR3(0,0,-1) ,
                  D3DXVECTOR3(0,-1,0) );
    }
    m_SmartVB.End();
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawWalls()
{
    MazeCellRef*  pCell = m_mcrVisList;
    VERTEX_TEXURED* pVert;
    WORD*         pIndex;
    WORD          offset;
    DWORD         verts_per_side = (m_dwTesselation+1)*(m_dwTesselation+1);
    DWORD         indicies_per_side = m_dwTesselation*m_dwTesselation*6;

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTexture( 0, m_pWallTexture );

    m_SmartVB.Begin();
    for( DWORD i = 0; i < m_dwNumVisibleCells; i++, pCell++ )
    {
        BYTE cell = m_pMazeClient->m_Maze.GetCell(pCell->x,pCell->y);

        if( cell & MAZE_WALL_NORTH )
        {
            D3DXVECTOR3 pos( float(pCell->x), 1, float(pCell->y) );
            m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
            LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(1,0,0), D3DXVECTOR3(0,-1,0) ,
                      D3DXVECTOR3(0,0,1) );
        }
        if( cell & MAZE_WALL_SOUTH )
        {
            D3DXVECTOR3 pos( float(1+pCell->x), 1, float(1+pCell->y) );
            m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
            LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(-1,0,0), D3DXVECTOR3(0,-1,0) ,
                      D3DXVECTOR3(0,0,-1) );
        }
        if( cell & MAZE_WALL_WEST )
        {
            D3DXVECTOR3 pos( float(pCell->x), 1, float(1+pCell->y) );
            m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
            LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(0,0,-1), D3DXVECTOR3(0,-1,0) ,
                      D3DXVECTOR3(1,0,0) );
        }
        if( cell & MAZE_WALL_EAST )
        {
            D3DXVECTOR3 pos( float(1+pCell->x), 1, float(pCell->y) );
            m_SmartVB.MakeRoom( verts_per_side, indicies_per_side, &pVert, &pIndex, &offset );
            LoadQuad( pVert, pIndex, offset, pos, D3DXVECTOR3(0,0,1), D3DXVECTOR3(0,-1,0) ,
                      D3DXVECTOR3(-1,0,0) );
        }
    }
    m_SmartVB.End();
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::LoadQuad( VERTEX_TEXURED* pVerts, WORD* pIndex, WORD wOffset,
                      const D3DXVECTOR3& vOrigin, const D3DXVECTOR3& vBasis1,
                      const D3DXVECTOR3& vBasis2, const D3DXVECTOR3& vNormal )
{
    // Compute edge steps
    float       fStep = 1.0f / m_dwTesselation;
    D3DXVECTOR3 vStep1 = vBasis1 * fStep;
    D3DXVECTOR3 vStep2 = vBasis2 * fStep;

    // Fill out grid of vertices
    D3DXVECTOR3 rowstart = vOrigin;
    float u = 0;
    for( DWORD i = 0; i <= m_dwTesselation; i++, rowstart += vStep1, u += fStep )
    {
        D3DXVECTOR3 pos = rowstart;
        float     v = 0;
        for( DWORD j = 0; j <= m_dwTesselation; j++, pos += vStep2, v += fStep )
        {
            pVerts->vPos = pos;
            pVerts->vNormal = vNormal;
            pVerts->fU = u;
            pVerts->fV = v;
            pVerts++;
        }
    }

    // Fill out grid of indicies
    for( i = 0; i < m_dwTesselation; i++ )
    {
        for( DWORD j = 0; j < m_dwTesselation; j++ )
        {
            *pIndex++ = wOffset;
            *pIndex++ = wOffset+1;
            *pIndex++ = wOffset+1+(WORD(m_dwTesselation)+1);
            *pIndex++ = wOffset;
            *pIndex++ = wOffset+1+(WORD(m_dwTesselation)+1);
            *pIndex++ = wOffset+(WORD(m_dwTesselation)+1);
            wOffset++;
        }
        wOffset++;
    }
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawMiniMap()
{
    DWORD           y;
    DWORD           x;
    VERTEX_DIFFUSE* verts = NULL;
    D3DXVECTOR3     pos;
    D3DXVECTOR3     origin;
    D3DXMATRIX      matWorld;
    D3DXMATRIX      matTrans;
    D3DXMATRIX      matRot;
    MazeCellRef*    pCell = NULL;

    // Compute size of mini-map based on display size
    DWORD mapsize = m_d3dsdBackBuffer.Width / 4;
    int mapx = m_d3dsdBackBuffer.Width  - mapsize - 16;
    int mapy = m_d3dsdBackBuffer.Height - mapsize - 16;

    if( mapx < 0 || mapy < 0 )
        return;

    // Disable z-buffering
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

    D3DXMatrixIdentity( &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Draw alpha blended background
    VERTEX_TL_DIFFUSE* v;   

    m_pMiniMapBackgroundVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].vPos.x = float(mapx)-0.5f;             v[0].vPos.y = float(mapy)-0.5f;
    v[1].vPos.x = float(mapx+mapsize)-0.5f;     v[1].vPos.y = float(mapy)-0.5f;
    v[2].vPos.x = float(mapx+mapsize)-0.5f;     v[2].vPos.y = float(mapy+mapsize)-0.5f;
    v[3].vPos.x = float(mapx)-0.5f;             v[3].vPos.y = float(mapy+mapsize)-0.5f;
    v[0].vPos.z     = v[1].vPos.z     = v[2].vPos.z     = v[3].vPos.z    = 0.1f;
    v[0].fRHW       = v[1].fRHW       = v[2].fRHW       = v[3].fRHW      = 1.0f;
    v[0].dwDiffuse  = v[1].dwDiffuse  = v[2].dwDiffuse  = v[3].dwDiffuse = 0x80008000;
    m_pMiniMapBackgroundVB->Unlock();

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    m_pd3dDevice->SetVertexShader( FVF_TL_DIFFUSE );
    m_pd3dDevice->SetStreamSource( 0, m_pMiniMapBackgroundVB, sizeof(VERTEX_TL_DIFFUSE) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    DWORD dwLineCount = 0;

    // Now set matrices for orthogonal top-down line drawing into mini-map area
    D3DXMATRIX orthproj;
    D3DXMatrixOrthoLH( &orthproj, 8, 8, 1, 2 );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &orthproj );

    D3DXMATRIX orthcam; D3DXMatrixIdentity( &orthcam );
    orthcam.m[3][0] = -m_vCameraPos.x;
    orthcam.m[3][1] = -m_vCameraPos.z;
    orthcam.m[0][0] = orthcam.m[2][1] = orthcam.m[1][2] = 1;
    orthcam.m[1][1] = orthcam.m[2][2] = 0;
    D3DXMATRIX rot; D3DXMatrixRotationZ( &rot, -m_fCameraYaw );
    orthcam = orthcam * rot;
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &orthcam );

    // Compute range of cells we're interested in
    DWORD centrecellx = DWORD(m_vCameraPos.x);
    DWORD centrecelly = DWORD(m_vCameraPos.z);
    DWORD mazewidth = m_pMazeClient->m_Maze.GetWidth();
    DWORD mazeheight = m_pMazeClient->m_Maze.GetHeight();
    DWORD minx = (centrecellx < 6) ? 0 : centrecellx - 6;
    DWORD miny = (centrecelly < 6) ? 0 : centrecelly - 6;
    DWORD maxx = centrecellx+6 > mazewidth - 1  ? mazewidth - 1 : centrecellx+5;
    DWORD maxy = centrecelly+6 > mazeheight - 1 ? mazeheight - 1 : centrecelly+5;

    // Change viewport to clip to the map region
    D3DVIEWPORT8 oldvp;
    oldvp.X         = 0;
    oldvp.Y         = 0;
    oldvp.Width     = m_d3dsdBackBuffer.Width;
    oldvp.Height    = m_d3dsdBackBuffer.Height;
    oldvp.MinZ      = 0.0f;
    oldvp.MaxZ      = 1.0f;

    D3DVIEWPORT8 mapvp;
    mapvp.X       = mapx; 
    mapvp.Y       = mapy;
    mapvp.Width   = mapsize;
    mapvp.Height  = mapsize;
    mapvp.MinZ    = 0.0f; 
    mapvp.MaxZ    = 1.0f;
    m_pd3dDevice->SetViewport( &mapvp );

    // Lock mini-map VB
    m_pMiniMapVB->Lock( 0, 0, (BYTE**)&verts, D3DLOCK_DISCARD );

    // Loop through cells
    origin = D3DXVECTOR3( float(minx),1,float(miny) );
    for( x = minx; x <= maxx; x++ )
    {
        pos = origin;
        for( DWORD y = miny; y <= maxy; y++ )
        {
            // Grab wall bitmask
            BYTE cell = m_pMazeClient->m_Maze.GetCell(x,y);

            // Check for north wall
            if( cell & MAZE_WALL_NORTH )
            {
                verts->vPos = pos;                  verts->dwDiffuse = 0xffffff; verts++;
                verts->vPos = pos; verts->vPos.x++; verts->dwDiffuse = 0xffffff; verts++;
                dwLineCount++;
            }

            // Check for west wall
            if( cell & MAZE_WALL_WEST )
            {
                verts->vPos = pos;                  verts->dwDiffuse = 0xffffff; verts++;
                verts->vPos = pos; verts->vPos.z++; verts->dwDiffuse = 0xffffff; verts++;
                dwLineCount++;
            }

            if( y==maxy )
            {
                // Check for north wall
                if( cell & MAZE_WALL_SOUTH )
                {
                    verts->vPos = pos; verts->vPos.z++;                  verts->dwDiffuse = 0xffffff; verts++;
                    verts->vPos = pos; verts->vPos.z++; verts->vPos.x++; verts->dwDiffuse = 0xffffff; verts++;
                    dwLineCount++;
                }
            }

            if( x==maxx )
            {
                // Check for west wall
                if( cell & MAZE_WALL_EAST )
                {
                    verts->vPos = pos; verts->vPos.x++;                  verts->dwDiffuse = 0xffffff; verts++;
                    verts->vPos = pos; verts->vPos.x++; verts->vPos.z++; verts->dwDiffuse = 0xffffff; verts++;
                    dwLineCount++;
                }
            }

            pos.z += 1;
        }

        origin.x += 1;
    }

    // Unlock VB and submit for rendering
    m_pMiniMapVB->Unlock();
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    m_pd3dDevice->SetVertexShader( FVF_DIFFUSE );
    m_pd3dDevice->SetStreamSource( 0, m_pMiniMapVB, sizeof(VERTEX_DIFFUSE) );
    m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, dwLineCount );

    // Draw marker for camera
    D3DXMatrixRotationY( &rot, -m_fCameraYaw );
    rot.m[3][0] = m_vCameraPos.x; rot.m[3][2] = m_vCameraPos.z;
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &rot );

    m_pMiniMapVB->Lock( 0, 0, (BYTE**)&verts, D3DLOCK_DISCARD );
    verts[0].vPos.y     = verts[1].vPos.y       = verts[2].vPos.y       = verts[3].vPos.y       = 1.0f;
    verts[0].dwDiffuse  = verts[1].dwDiffuse    = verts[2].dwDiffuse    = verts[3].dwDiffuse    = 0xb01010;
    verts[0].vPos.x =  0.0f; verts[0].vPos.z =  0.4f;
    verts[1].vPos.x =  0.2f; verts[1].vPos.z =  0.0f;
    verts[2].vPos.x =  0.0f; verts[2].vPos.z = -0.2f;
    verts[3].vPos.x = -0.2f; verts[3].vPos.z =  0.0f;
    m_pMiniMapVB->Unlock();

    m_pd3dDevice->SetVertexShader( FVF_DIFFUSE );
    m_pd3dDevice->SetStreamSource( 0, m_pMiniMapVB, sizeof(VERTEX_DIFFUSE) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

    // Draw things in mini-map
    m_pMazeClient->LockWorld();

    m_pMiniMapVB->Lock( 0, 0, (BYTE**)&verts, D3DLOCK_DISCARD );
    verts[0].vPos.y     = verts[1].vPos.y       = verts[2].vPos.y       = verts[3].vPos.y       = 1.0f;
    verts[0].dwDiffuse  = verts[1].dwDiffuse    = verts[2].dwDiffuse    = verts[3].dwDiffuse    = 0x00FFFF00;
    verts[0].vPos.x =  0.0f; verts[0].vPos.z =  0.4f;
    verts[1].vPos.x =  0.2f; verts[1].vPos.z =  0.0f;
    verts[2].vPos.x =  0.0f; verts[2].vPos.z = -0.2f;
    verts[3].vPos.x = -0.2f; verts[3].vPos.z =  0.0f;
    m_pMiniMapVB->Unlock();

    m_pd3dDevice->SetVertexShader( FVF_DIFFUSE );
    m_pd3dDevice->SetStreamSource( 0, m_pMiniMapVB, sizeof(VERTEX_DIFFUSE) );

    for( x = minx; x <= maxx; x++ )
    {
        for( y = miny; y <= maxy; y++ )
        {
            PlayerObject* pPlayerObject = m_pMazeClient->GetFirstPlayerObjectInCell( x, y );
            while( pPlayerObject )
            {
                D3DXMatrixTranslation( &matTrans, pPlayerObject->vPos.x, pPlayerObject->vPos.y, pPlayerObject->vPos.z );
                D3DXMatrixRotationYawPitchRoll( &matRot, -AngleToFloat(pPlayerObject->aCameraYaw), 0.0f, 0.0f );
                matWorld = matRot * matTrans;
                m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

                pPlayerObject = pPlayerObject->pNext;
            }

            pos.z += 1;
        }
    }
    m_pMazeClient->UnlockWorld();

    D3DXMatrixIdentity( &rot );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &rot );

    // Re-enable z-buffering and reset viewport and matrices
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_pd3dDevice->SetViewport( &oldvp );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_Projection );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_Camera );

}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CD3DGraphics::DrawIndicators()
{
    VERTEX_TL_TEXTURED* v;   

    float xscale = float(m_d3dsdBackBuffer.Width)/640.0f;
    float yscale = float(m_d3dsdBackBuffer.Height)/480.0f;

    // Draw network/local icon and player icon (if connected)
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

    m_pIndicatorVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].dwSpecular = v[1].dwSpecular = v[2].dwSpecular = v[3].dwSpecular = 0;
    v[0].vPos.z     = v[1].vPos.z     = v[2].vPos.z     = v[3].vPos.z     = 0.0f;
    v[0].fRHW       = v[1].fRHW       = v[2].fRHW       = v[3].fRHW       = 1.0f;
    v[0].dwDiffuse  = v[1].dwDiffuse  = v[2].dwDiffuse  = v[3].dwDiffuse  = 0xFFFFFFFF;
    v[0].fU = 0; v[0].fV = 0;
    v[1].fU = 1; v[1].fV = 0;
    v[2].fU = 1; v[2].fV = 1;
    v[3].fU = 0; v[3].fV = 1;
    v[0].vPos.x = (xscale*16)-0.5f; v[0].vPos.y = ((480-68)*yscale)-0.5f;
    v[1].vPos.x = (xscale*48)-0.5f; v[1].vPos.y = ((480-68)*yscale)-0.5f;
    v[2].vPos.x = (xscale*48)-0.5f; v[2].vPos.y = ((480-36)*yscale)-0.5f;
    v[3].vPos.x = (xscale*16)-0.5f; v[3].vPos.y = ((480-36)*yscale)-0.5f;
    m_pIndicatorVB->Unlock();

    if( m_pDP8Client->IsSessionLost() )
        m_pd3dDevice->SetTexture( 0, m_pLocalIconTexture );
    else
        m_pd3dDevice->SetTexture( 0, m_pNetIconTexture );

    m_pd3dDevice->SetVertexShader( FVF_TL_TEXTURED );
    m_pd3dDevice->SetStreamSource( 0, m_pIndicatorVB, sizeof(VERTEX_TL_TEXTURED) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

    if( FALSE == m_pDP8Client->IsSessionLost() )
    {
        DWORD dwNumPlayers, dwNumNearbyPlayers;
        m_pMazeClient->GetPlayerStats( &dwNumPlayers, &dwNumNearbyPlayers );

        // Show frame rate
        POINT ptStart  = { (long)(xscale*60.0f), (long)((480-95)*yscale) };
        TCHAR strText[MAX_PATH];
        wsprintf( strText, TEXT("       Players: %4d"), dwNumPlayers );
        m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y, D3DCOLOR_ARGB(255,255,255,0), strText );
        wsprintf( strText, TEXT("Nearby Players: %4d"), dwNumNearbyPlayers );
        m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y+20, D3DCOLOR_ARGB(255,255,255,0), strText );
        wsprintf( strText, TEXT("          Ping: %4d ms"), m_pMazeClient->GetRoundTripLatencyMS() );
        m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y+40, D3DCOLOR_ARGB(255,255,255,0), strText );
        wsprintf( strText, TEXT("     Data Rate: %4d Bps"), m_pMazeClient->GetThroughputBPS() );
        m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y+60, D3DCOLOR_ARGB(255,255,255,0), strText );
    }
    else
    {
        if( m_pMazeApp->IsOutOfDateClient() )
        {
            POINT ptStart  = { (long)(xscale*60.0f), (long)((480-75)*yscale) };
            TCHAR strText[MAX_PATH];
            wsprintf( strText, TEXT("Client is out of date") );
            m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y, D3DCOLOR_ARGB(255,255,255,0), strText );
            wsprintf( strText, TEXT("Please get update at") );
            m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y+20, D3DCOLOR_ARGB(255,255,255,0), strText );
            wsprintf( strText, TEXT("http://msdn.microsoft.com/directx/") );
            m_pFont->DrawText( (float)ptStart.x, (float)ptStart.y+40, D3DCOLOR_ARGB(255,255,255,0), strText );
        }
    }

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::InvalidateDeviceObjects()
{ 
    m_pFont->InvalidateDeviceObjects();
    m_pStatsFont->InvalidateDeviceObjects();

    m_pPlayerMesh->InvalidateDeviceObjects();
    SAFE_RELEASE( m_pMiniMapVB );

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::DeleteDeviceObjects()
{ 
    m_pFont->DeleteDeviceObjects();
    m_pStatsFont->DeleteDeviceObjects();

    SAFE_RELEASE( m_pSphere );
    if( m_pPlayerMesh )
        m_pPlayerMesh->Destroy();
   
    SAFE_RELEASE( m_pMiniMapBackgroundVB );
    SAFE_RELEASE( m_pIndicatorVB );
    m_SmartVB.Uninit();

    SAFE_RELEASE( m_pWallTexture );
    SAFE_RELEASE( m_pFloorTexture );
    SAFE_RELEASE( m_pCeilingTexture );
    SAFE_RELEASE( m_pNetIconTexture );
    SAFE_RELEASE( m_pLocalIconTexture );

    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CD3DGraphics::FinalCleanup()
{ 
    SAFE_DELETE( m_pFont );
    SAFE_DELETE( m_pStatsFont );
    SAFE_DELETE( m_pPlayerMesh );

    return S_OK; 
}




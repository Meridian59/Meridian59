//-----------------------------------------------------------------------------
// File: Donuts.cpp
//
// Desc: DirectInput semantic mapper version of Donuts3D game
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <basetsd.h>
#include <cguid.h>
#include <tchar.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#include <D3DX8.h>
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DIUtil.h"
#include "DMUtil.h"
#include "DXUtil.h"
#include "resource.h"
#include "gamemenu.h"
#include "donuts.h"




//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyApplication*    g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
    CMyApplication app;

    g_hInst = hInstance;

    if( FAILED( app.Create( hInstance ) ) )
        return 0;

    return app.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyApplication()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMyApplication::CMyApplication()
{
    g_pApp                  = this;
    m_strAppName            = _T("Donuts3D");
    m_hWndMain              = NULL;               
    m_dwScreenWidth         = 800;   
    m_dwScreenHeight        = 600;
    m_bFullScreen           = FALSE; 
    m_bIsActive             = FALSE; 
    m_bDisplayReady         = FALSE; 
    m_bMouseVisible         = FALSE;  
    m_hSplashBitmap         = NULL;  
    m_dwAppState            = APPSTATE_LOADSPLASH;              
    m_dwLevel               = 0;     
    m_dwScore               = 0;     
    m_dwViewMode            = 2;     
    m_fViewTransition       = 0.0f;  
    m_bAnimatingViewChange  = FALSE; 
    m_bFirstPersonView      = TRUE;  
    m_fBulletRechargeTime   = 0.0f;  
    m_dwBulletType          = 2;    
    m_pDisplayList          = NULL;          
    m_pShip                 = NULL;          
    m_pd3dDevice            = NULL; 
    m_pConfigSurface        = NULL; 
    m_pViewportVB           = NULL;
    m_pSpriteVB             = NULL;
    m_pShipFileObject       = NULL; 
    m_dwNumShipTypes        = 10L;
    m_dwCurrentShipType     = 1;
    m_pMusicManager         = NULL;  
    m_pBeginLevelSound      = NULL;  
    m_pEngineIdleSound      = NULL;
    m_pEngineRevSound       = NULL;
    m_pShieldBuzzSound      = NULL;
    m_pShipExplodeSound     = NULL;
    m_pFireBulletSound      = NULL;
    m_pShipBounceSound      = NULL;
    m_pDonutExplodeSound    = NULL;
    m_pPyramidExplodeSound  = NULL;
    m_pCubeExplodeSound     = NULL;
    m_pSphereExplodeSound   = NULL;
    m_pGameTexture1         = NULL; 
    m_pGameTexture2         = NULL; 
    m_pTerrain              = NULL;
    m_pGameFont             = NULL;
    m_pMenuFont             = NULL;
    m_pMainMenu             = NULL;    
    m_pQuitMenu             = NULL;
    m_pCurrentMenu          = NULL;
    m_pInputDeviceManager   = NULL;
    m_bPaused               = FALSE;

    ZeroMemory( &m_UserInput, sizeof(m_UserInput) );
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Creates the window
//-----------------------------------------------------------------------------
HRESULT CMyApplication::Create( HINSTANCE hInstance )
{
    // Register the window class
    WNDCLASS wndClass = { CS_DBLCLKS, StaticMsgProc, 0, 0, hInstance,
                          LoadIcon( hInstance, MAKEINTRESOURCE(DONUTS_ICON) ),
                          LoadCursor( NULL, IDC_ARROW ),
                          (HBRUSH)GetStockObject( BLACK_BRUSH ),
                          NULL, TEXT("DonutsClass") };
    RegisterClass( &wndClass );

    // Create our main window
    m_hWndMain = CreateWindowEx( 0, TEXT("DonutsClass"), TEXT("Donuts"),
                                 WS_VISIBLE|WS_POPUP|WS_CAPTION|WS_SYSMENU,
                                 0, 0, 640, 480, NULL, NULL,
                                 hInstance, NULL );
    if( NULL == m_hWndMain )
        return E_FAIL;
    UpdateWindow( m_hWndMain );

        // Create the game objects (display objects, sounds, input devices,
        // menus, etc.)
    if( FAILED( OneTimeSceneInit( m_hWndMain ) ) )
    {
        DestroyWindow( m_hWndMain );
        return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Handles the message loop and calls FrameMove() and Render() when
//       idle.
//-----------------------------------------------------------------------------
INT CMyApplication::Run()
{
    // Load keyboard accelerators
    HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Now we're ready to recieve and process Windows messages.
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        if( m_bIsActive )
            bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

        if( bGotMsg )
        {
            // Translate and dispatch the message
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            // Render a frame during idle time (no messages are waiting)
            if( m_bDisplayReady )
            {
                FrameMove();
                RenderFrame();
            }
        }
    }

    return (int)msg.wParam;
}




//-----------------------------------------------------------------------------
// Name: StaticMsgProc()
// Desc: Static msg handler which passes messages to the application class.
//-----------------------------------------------------------------------------
LRESULT CALLBACK StaticMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return g_pApp->MsgProc( hWnd, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
LRESULT CMyApplication::MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_ACTIVATEAPP:
            m_bIsActive = (BOOL)wParam;
            m_bMouseVisible   = FALSE;

            if( m_bIsActive )
            {
                DXUtil_Timer( TIMER_START );
            }
            else
            {
                DXUtil_Timer( TIMER_STOP );
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 320;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            break;

        case WM_SETCURSOR:
            if( !m_bMouseVisible && m_dwAppState!=APPSTATE_DISPLAYSPLASH )
                SetCursor( NULL );
            else
                SetCursor( LoadCursor( NULL, IDC_ARROW ) );
            return TRUE;

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_KEYMENU:
                case SC_MONITORPOWER:
                        return 1;
            }
            break;

                case WM_SYSKEYDOWN:
            // Handle Alt+Enter to do mode-switching
            if( VK_RETURN == wParam )
            {
                SwitchDisplayModes( !m_bFullScreen, m_dwScreenWidth,
                                    m_dwScreenHeight );
            }
            break;

        case WM_KEYDOWN:
            // Move from splash screen when user presses a key
            if( m_dwAppState == APPSTATE_DISPLAYSPLASH )
            {
                if( wParam==VK_ESCAPE )
                {
                    // Escape keys exits the app
                    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    m_bDisplayReady = FALSE;
                }
                else
                {
                    // Get rid of splash bitmap
                    DeleteObject( m_hSplashBitmap );

                    // Advance to the first level
                    m_dwAppState = APPSTATE_BEGINLEVELSCREEN;
                    DXUtil_Timer( TIMER_START );
                    AdvanceLevel();
                }
            }
            return 0;

        case WM_PAINT:
            if( m_dwAppState == APPSTATE_DISPLAYSPLASH )
            {
                BITMAP bmp;
                RECT rc;
                GetClientRect( m_hWndMain, &rc );

                // Display the splash bitmap in the window
                HDC hDCWindow = GetDC( m_hWndMain );
                HDC hDCImage  = CreateCompatibleDC( NULL );
                SelectObject( hDCImage, m_hSplashBitmap );
                GetObject( m_hSplashBitmap, sizeof(bmp), &bmp );
                StretchBlt( hDCWindow, 0, 0, rc.right, rc.bottom,
                            hDCImage, 0, 0,
                            bmp.bmWidth, bmp.bmHeight, SRCCOPY );
                DeleteDC( hDCImage );
                ReleaseDC( m_hWndMain, hDCWindow );
            }
            else
            {
                if( m_bDisplayReady )
                {
                    DrawDisplayList();
                    ShowFrame();
                }
            }
            break;

        case WM_DESTROY:
            FinalCleanup();
            PostQuitMessage( 0 );
            m_bDisplayReady = FALSE;
            break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::OneTimeSceneInit( HWND hWnd )
{
    HRESULT hr;

    // Initialize the DirectInput stuff
    if( FAILED( hr = CreateInputObjects( hWnd ) ) )
        return hr;

    // Initialize the DirectSound stuff. Note: if this call fails, we can
        // continue with no sound.
    CreateSoundObjects( hWnd );

    // Create the display objects
    if( FAILED( hr = CreateDisplayObjects( hWnd ) ) )
        return hr;

    // Add a ship to the displaylist
    m_pShip = new CShip( D3DXVECTOR3(0.0f,0.0f,0.0f) );
    m_pDisplayList = m_pShip;

    // Construct the game menus
    ConstructMenus();

    // Initial program state is to display the splash screen
    m_dwAppState = APPSTATE_LOADSPLASH;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::FinalCleanup()
{
    DestroyDisplayObjects();
    DestroySoundObjects();
    DestroyInputObjects();
    DestroyMenus();
}




//-----------------------------------------------------------------------------
// Name: AdvanceLevel()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::AdvanceLevel()
{
    // Up the level
    m_dwLevel++;

    srand( timeGetTime() );

    // Clear any stray objects (anything but the ship) out of the display list
    while( m_pShip->pNext )
    {
        DeleteFromList( m_pShip->pNext );
    }

    // Create donuts for the new level
    for( WORD i=0; i<(2*m_dwLevel+3); i++ )
    {
        D3DVECTOR vPosition = 3.0f * D3DXVECTOR3( rnd(), rnd(), 0.0f );
        D3DVECTOR vVelocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

        AddToList( new CDonut( vPosition, vVelocity ) );
    }

    // Delay for 2 seconds before displaying ship
    m_pShip->vPos       = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_pShip->vVel       = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_pShip->bVisible   = FALSE;
    m_pShip->bExploded  = FALSE;
    m_pShip->fShowDelay = 2.0f;

    // Stop engine sounds
    StopSound( m_pEngineIdleSound );
    StopSound( m_pEngineRevSound );
}




//-----------------------------------------------------------------------------
// Name: DisplayObject()
// Desc:
//-----------------------------------------------------------------------------
DisplayObject::DisplayObject( DWORD type, D3DVECTOR p, D3DVECTOR v )
{
    // Set object attributes
    pNext    = NULL;
    pPrev    = NULL;
    bVisible = TRUE;
    dwType   = type;
    vPos     = p;
    vVel     = v;
}




//-----------------------------------------------------------------------------
// Name: C3DSprite()
// Desc:
//-----------------------------------------------------------------------------
C3DSprite::C3DSprite( DWORD type, D3DVECTOR p, D3DVECTOR v )
          :DisplayObject( type, p, v )
{
    dwColor = 0xffffffff;
}




//-----------------------------------------------------------------------------
// Name: CDonut()
// Desc:
//-----------------------------------------------------------------------------
CDonut::CDonut( D3DVECTOR p, D3DVECTOR v )
       :C3DSprite( OBJ_DONUT, p, v )
{
    // Set object attributes
    dwTextureWidth   = DONUT_WIDTH;
    dwTextureHeight  = DONUT_HEIGHT;
    dwTextureOffsetX = 0;
    dwTextureOffsetY = 0;

    fSize           = dwTextureWidth / 256.0f;
    vVel            += 0.5f * D3DXVECTOR3( rnd(), rnd(), 0.0f );

    delay           = rnd( 3.0f, 12.0f );
    dwFramesPerLine = 8;
    frame           = rnd( 0.0f, 30.0f );
    fMaxFrame       = NUM_DONUT_FRAMES;
}




//-----------------------------------------------------------------------------
// Name: CPyramid()
// Desc:
//-----------------------------------------------------------------------------
CPyramid::CPyramid( D3DVECTOR p, D3DVECTOR v )
         :C3DSprite( OBJ_PYRAMID, p, v )
{
    // Set object attributes
    dwTextureWidth   = PYRAMID_WIDTH;
    dwTextureHeight  = PYRAMID_HEIGHT;
    dwTextureOffsetX = 0;
    dwTextureOffsetY = 0;

    fSize           = dwTextureWidth / 256.0f;
    vVel            += 0.5f * D3DXVECTOR3( rnd(), rnd(), 0.0f );

    delay           = rnd( 12.0f, 40.0f );
    dwFramesPerLine = 8;
    frame           = rnd( 0.0f, 30.0f );
    fMaxFrame       = NUM_PYRAMID_FRAMES;

}




//-----------------------------------------------------------------------------
// Name: CSphere()
// Desc:
//-----------------------------------------------------------------------------
CSphere::CSphere( D3DVECTOR p, D3DVECTOR v )
        :C3DSprite( OBJ_SPHERE, p, v )
{
    // Set object attributes
    dwTextureWidth   = SPHERE_WIDTH;
    dwTextureHeight  = SPHERE_HEIGHT;
    dwTextureOffsetX = 0;
    dwTextureOffsetY = 128;

    fSize           = dwTextureWidth / 256.0f;
    vVel            += 0.5f * D3DXVECTOR3( rnd(), rnd(), 0.0f );

    delay           = rnd( 60.0f, 80.0f );
    dwFramesPerLine = 16;
    frame           = rnd( 0.0f, 30.0f );
    fMaxFrame       = NUM_SPHERE_FRAMES;
}





//-----------------------------------------------------------------------------
// Name: CCube()
// Desc:
//-----------------------------------------------------------------------------
CCube::CCube( D3DVECTOR p, D3DVECTOR v )
      :C3DSprite( OBJ_CUBE, p, v )
{
    // Set object attributes
    dwTextureWidth   = CUBE_WIDTH;
    dwTextureHeight  = CUBE_HEIGHT;
    dwTextureOffsetX = 0;
    dwTextureOffsetY = 176;

    fSize           = dwTextureWidth / 256.0f;
    vVel            += 0.5f * D3DXVECTOR3( rnd(), rnd(), 0.0f );

    delay           = rnd( 32.0f, 80.0f );
    dwFramesPerLine = 16;
    frame           = rnd( 0.0f, 30.0f );
    fMaxFrame       = NUM_CUBE_FRAMES;
}




//-----------------------------------------------------------------------------
// Name: CCloud()
// Desc:
//-----------------------------------------------------------------------------
CCloud::CCloud( D3DVECTOR p, D3DVECTOR v )
       :C3DSprite( OBJ_CLOUD, p, v )
{
    // Set object attributes
    dwTextureWidth   = CLOUD_WIDTH;
    dwTextureHeight  = CLOUD_WIDTH;
    dwTextureOffsetX = 224;
    dwTextureOffsetY = 224;

    fSize           = dwTextureWidth / 256.0f;
    delay           = rnd( 1.0f, 3.0f );
    dwFramesPerLine = 1;
    frame           = 0.0f;
    fMaxFrame       = 1;
}




//-----------------------------------------------------------------------------
// Name: CBullet()
// Desc:
//-----------------------------------------------------------------------------
CBullet::CBullet( D3DVECTOR p, D3DVECTOR v, DWORD dwCType )
        :C3DSprite( OBJ_BULLET, p, v )
{
    // Set object attributes
    dwTextureWidth   = CLOUD_WIDTH;
    dwTextureHeight  = CLOUD_HEIGHT;
    dwTextureOffsetX = 224;
    dwTextureOffsetY = 224;

    if( dwCType == 0 )
        dwColor = 0xff2020ff;
    if( dwCType == 1 )
        dwColor = 0xff208020;
    if( dwCType == 2 )
        dwColor = 0xff208080;
    if( dwCType == 3 )
        dwColor = 0xff802020;

    fSize           = 4 / 256.0f;
    fMaxFrame       = NUM_BULLET_FRAMES;

    delay           = 1000.0f;
    dwFramesPerLine = 1;
    frame           = 0.0f;
}




//-----------------------------------------------------------------------------
// Name: CShip()
// Desc:
//-----------------------------------------------------------------------------
CShip::CShip( D3DVECTOR p )
      :DisplayObject( OBJ_SHIP, p, D3DXVECTOR3(0,0,0) )
{
    fSize           = 10.0f / 256.0f;
    bExploded       = FALSE;
    fShowDelay      = 0.0f;

    fRoll           = 0.0f;
    fAngle          = 0.0f;
}




//-----------------------------------------------------------------------------
// Name: AddToList()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::AddToList( DisplayObject* pObject )
{
    pObject->pNext = m_pDisplayList->pNext;
    pObject->pPrev = m_pDisplayList;

    if( m_pDisplayList->pNext )
        m_pDisplayList->pNext->pPrev = pObject;
    m_pDisplayList->pNext = pObject;
}




//-----------------------------------------------------------------------------
// Name: IsDisplayListEmpty()
// Desc:
//-----------------------------------------------------------------------------
BOOL CMyApplication::IsDisplayListEmpty()
{
    DisplayObject* pObject = m_pDisplayList->pNext;

    while( pObject )
    {
        if( pObject->dwType != OBJ_BULLET )
            return FALSE;

        pObject = pObject->pNext;
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: LoadTerrainModel()
// Desc: Loads the 3D geometry for the terrain
//-----------------------------------------------------------------------------
HRESULT CMyApplication::LoadTerrainModel()
{
    LPDIRECT3DVERTEXBUFFER8 pVB;
    DWORD        dwNumVertices;
    MODELVERTEX* pVertices;

    // Delete old object
    SAFE_DELETE( m_pTerrain );

    // Create new object
    m_pTerrain = new CD3DMesh();
    if( FAILED( m_pTerrain->Create( m_pd3dDevice, _T("SeaFloor.x") ) ) )
        return E_FAIL;

    // Set the FVF to a reasonable type
    m_pTerrain->SetFVF( m_pd3dDevice, D3DFVF_MODELVERTEX );

    // Gain access to the model's vertices
    m_pTerrain->GetSysMemMesh()->GetVertexBuffer( &pVB );
    dwNumVertices = m_pTerrain->GetSysMemMesh()->GetNumVertices();
    pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );

    for( DWORD i=0; i<dwNumVertices; i++ )
    {
        pVertices[i].p.x *= 0.1f;
        pVertices[i].p.z *= 0.1f;
        pVertices[i].p.y = HeightField( pVertices[i].p.x, pVertices[i].p.z );
    }

    // Done with the vertex buffer
    pVB->Unlock();
    pVB->Release();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadShipModel()
// Desc: Loads the 3D geometry for the player's ship
//-----------------------------------------------------------------------------
HRESULT CMyApplication::LoadShipModel()
{
    LPDIRECT3DVERTEXBUFFER8 pVB;
    DWORD        dwNumVertices;
    MODELVERTEX* pVertices;
    D3DXVECTOR3  vCenter;
    FLOAT        fRadius;

    // Delete old object
    SAFE_DELETE( m_pShipFileObject );

    // Create new object
    m_pShipFileObject = new CD3DMesh();
    if( FAILED( m_pShipFileObject->Create( m_pd3dDevice,
                                           g_strShipFiles[m_dwCurrentShipType] ) ) )
        return E_FAIL;

    // Set the FVF to a reasonable type
    m_pShipFileObject->SetFVF( m_pd3dDevice, D3DFVF_MODELVERTEX );

    // Gain access to the model's vertices
    m_pShipFileObject->GetSysMemMesh()->GetVertexBuffer( &pVB );
    dwNumVertices = m_pShipFileObject->GetSysMemMesh()->GetNumVertices();
    pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 );

    // Scale the new object to a standard size  
    D3DXComputeBoundingSphere( pVertices, dwNumVertices,
                               D3DFVF_MODELVERTEX, &vCenter, &fRadius );
    for( DWORD i=0; i<dwNumVertices; i++ )
    {
        pVertices[i].p /= 12*fRadius;
    }

    // Done with the vertex buffer
    pVB->Unlock();
    pVB->Release();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SwitchModel()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::SwitchModel()
{
    // Select next model
    m_dwCurrentShipType++;
    if( m_dwCurrentShipType >= m_dwNumShipTypes )
        m_dwCurrentShipType = 0L;

    // Create new object
    if( SUCCEEDED( LoadShipModel() ) )
    {
        // Initialize the new object's device dependent objects
        if( SUCCEEDED( m_pShipFileObject->RestoreDeviceObjects( m_pd3dDevice ) ) )
            return S_OK;
    }

    // Return with a fatal error
    PostMessage( m_hWndMain, WM_CLOSE, 0, 0 );
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::FrameMove()
{
    switch( m_dwAppState )
    {
        case APPSTATE_LOADSPLASH:
            // Set the app state to displaying splash
            m_dwAppState = APPSTATE_DISPLAYSPLASH;

            // Draw the splash bitmap
            m_hSplashBitmap = (HBITMAP)LoadImage( GetModuleHandle( NULL ),
                                                  TEXT("SPLASH"), IMAGE_BITMAP,
                                                  0, 0, LR_CREATEDIBSECTION );
            SendMessage( m_hWndMain, WM_PAINT, 0, 0 );
            break;

        case APPSTATE_ACTIVE:
            UpdateDisplayList();
            CheckForHits();

            if( IsDisplayListEmpty() )
            {
                AdvanceLevel();
                m_dwAppState = APPSTATE_BEGINLEVELSCREEN;
            }
            break;

        case APPSTATE_BEGINLEVELSCREEN:
            PlaySound( m_pBeginLevelSound );
            DXUtil_Timer( TIMER_RESET );
            m_dwAppState = APPSTATE_DISPLAYLEVELSCREEN;
            break;

        case APPSTATE_DISPLAYLEVELSCREEN:
            // Only show the Level intro screen for 3 seconds

            if( DXUtil_Timer( TIMER_GETAPPTIME ) > 3.0f )
            {
                m_dwAppState = APPSTATE_ACTIVE;
            }
            break;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderFrame()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::RenderFrame()
{
    // Test cooperative level
    HRESULT hr;

    // Test the cooperative level to see if it's okay to render
    if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
    {
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return S_OK;

        // Check if the device needs to be resized.
        if( D3DERR_DEVICENOTRESET == hr )
        {
            m_bDisplayReady = FALSE;

            InvalidateDisplayObjects();

            // Resize the device
            if( SUCCEEDED( m_pd3dDevice->Reset( &m_d3dpp ) ) )
            {
                // Initialize the app's device-dependent objects
                if( SUCCEEDED( RestoreDisplayObjects() ) )
                {
                    m_bDisplayReady = TRUE;
                    return S_OK;
                }
            }

            PostMessage( m_hWndMain, WM_CLOSE, 0, 0 );
        }
        return hr;
    }

    // Render the scene based on current state of the app
    switch( m_dwAppState )
    {
        case APPSTATE_LOADSPLASH:
            // Nothing to render while loading the splash screen
            break;

        case APPSTATE_DISPLAYSPLASH:
            // Rendering of the splash screen is handled by WM_PAINT
            break;

        case APPSTATE_BEGINLEVELSCREEN:
            // Nothing to render while starting sound to advance a level
            break;

        case APPSTATE_DISPLAYLEVELSCREEN:
            DisplayLevelIntroScreen( m_dwLevel );
            ShowFrame();
            break;

        case APPSTATE_ACTIVE:
            DrawDisplayList();
            ShowFrame();
            break;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DarkenScene()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DarkenScene( FLOAT fAmount )
{
    if( m_pd3dDevice==NULL )
        return;

    // Setup a dark square to cover the scene
    DWORD dwAlpha = (fAmount<1.0f) ? ((DWORD)(255*fAmount))<<24L : 0xff000000;
    SCREENVERTEX* v;
    m_pViewportVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].color = v[1].color = v[2].color = v[3].color = dwAlpha;
    m_pViewportVB->Unlock();

    // Set renderstates
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,   FALSE );
    m_pd3dDevice->SetTexture( 0, NULL );

    // Draw a big, gray square
    m_pd3dDevice->SetVertexShader( D3DFVF_SCREENVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pViewportVB, sizeof(SCREENVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,0, 2 );

    // Restore states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::RenderFieryText( CD3DFont* pFont, TCHAR* strText )
{
    if( NULL==pFont || NULL==strText )
        return;

    // Render the fiery portion of the text
    for( DWORD i=0; i<20; i++ )
    {
        FLOAT x = -0.5f;
        FLOAT y =  1.8f;

        FLOAT v1 = rnd(0.0f, 1.0f);
        FLOAT red1 = v1*v1*v1;
        FLOAT grn1 = v1*v1;
        FLOAT blu1 = v1;


        FLOAT a1 = rnd(0.0f, 2*D3DX_PI);
        FLOAT r1 = v1 * 0.05f;

        x += r1*sinf(a1);
        y += r1*cosf(a1);

        if( cosf(a1) < 0.0f )
            y -= 2*r1*cosf(a1)*cosf(a1);

        DWORD r = (CHAR)((1.0f-red1)*256.0f);
        DWORD g = (CHAR)((1.0f-grn1)*256.0f);
        DWORD b = (CHAR)((1.0f-blu1)*256.0f);
        DWORD a = (CHAR)255;
        DWORD dwColor = (a<<24) + (r<<16) + (g<<8) + b;

        m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
        m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

        pFont->DrawTextScaled( x, y, 0.9f, 0.25f, 0.25f, dwColor, strText, D3DFONT_FILTERED );
    }

    // Render the plain, black portion of the text
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    FLOAT x = -0.5f;
    FLOAT y =  1.8f;
    pFont->DrawTextScaled( x, y, 0.9f, 0.25f, 0.25f, 0xff000000, strText, D3DFONT_FILTERED );
}




//-----------------------------------------------------------------------------
// Name: DisplayLevelIntroScreen()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DisplayLevelIntroScreen( DWORD dwLevel )
{
    if( m_pd3dDevice==NULL )
        return;

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Erase the screen
        m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0L, 1.0f, 0L );

        TCHAR strLevel[80];
        _stprintf( strLevel, _T("Level %ld"), dwLevel );
        RenderFieryText( m_pGameFont, strLevel );

        DarkenScene( 1.0f - sinf(D3DX_PI*DXUtil_Timer( TIMER_GETAPPTIME )/3.0f) );

        // End the scene
        m_pd3dDevice->EndScene();
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateDisplayList()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::UpdateDisplayList()
{
    DisplayObject* pObject;

    // Get the time lapsed since the last frame
    FLOAT fTimeLapsed = DXUtil_Timer( TIMER_GETELAPSEDTIME );

    // Read input from the joystick/keyboard/etc
    UpdateInput( &m_UserInput );

    // Check for game menu condition
    if( m_pCurrentMenu )
    {
        UpdateMenus();
        return;
    }

    if( m_bPaused )
        return;

    if( m_pShip->fShowDelay > 0.0f )
    {
        m_pShip->fShowDelay -= fTimeLapsed;

        if( m_pShip->fShowDelay <= 0.0f )
        {
            m_pShip->vVel       = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
            m_pShip->fShowDelay = 0.0f;
            m_pShip->bVisible   = TRUE;
            m_pShip->bExploded  = FALSE;
        }
    }

    // Update the ship
    if( m_pShip->bVisible )
    {
        m_pShip->vPos += m_pShip->vVel * fTimeLapsed;
    }

    // Apply banking motion
    m_pShip->fRoll += m_UserInput.fAxisRotateLR * 1.0f * fTimeLapsed;
    if( m_pShip->fRoll > 0.5f )
        m_pShip->fRoll = 0.5f;
    if( m_pShip->fRoll < -0.5f )
        m_pShip->fRoll = -0.5f;

    m_pShip->fAngle += 5 * m_pShip->fRoll * fTimeLapsed;

    if( m_UserInput.fAxisRotateLR < 0.2f && m_UserInput.fAxisRotateLR > -0.2f )
    {
        m_pShip->fRoll *= 0.95f;
    }

    // Slow the ship down
    m_pShip->vVel.x *= 0.97f;
    m_pShip->vVel.y *= 0.97f;

    // Apply thrust
    m_pShip->vVel.x +=  sinf( m_pShip->fAngle ) * m_UserInput.fAxisMoveUD * 5.0f * fTimeLapsed;
    m_pShip->vVel.y += -cosf( m_pShip->fAngle ) * m_UserInput.fAxisMoveUD * 5.0f * fTimeLapsed;

    // Play thrusting sounds
    {
        static bPlayingEngineRevSound = FALSE;

        if( m_UserInput.fAxisMoveUD > 0.5f )
        {
            if( FALSE == bPlayingEngineRevSound )
            {
                bPlayingEngineRevSound = TRUE;
            }
        }
        else
        {
            if( TRUE == bPlayingEngineRevSound )
            {
                StopSound( m_pEngineRevSound );
                bPlayingEngineRevSound = FALSE;
            }
        }
    }

    m_fBulletRechargeTime -= fTimeLapsed;

     // Fire a bullet
    if( m_UserInput.bButtonFireWeapons && m_fBulletRechargeTime <= 0.0f )
    {
        // Ship must be visible and have no shields on to fire
        if( m_pShip->bVisible )
        {
            // Bullets cost one score point
            if( m_dwScore )
                m_dwScore--;

            // Play the "fire" effects
            PlaySound( m_pFireBulletSound );

            // Add a bullet to the display list
            if( m_dwBulletType == 0 )
            {
                D3DXVECTOR3 vDir = D3DXVECTOR3( sinf( m_pShip->fAngle ), -cosf( m_pShip->fAngle ), 0.0f );

                AddToList( new CBullet( m_pShip->vPos, m_pShip->vVel + 2*vDir, 0 ) );
                m_fBulletRechargeTime = 0.05f;
            }
            else if( m_dwBulletType == 1 )
            {
                D3DXVECTOR3 vOffset = 0.02f * D3DXVECTOR3( cosf(m_pShip->fAngle), sinf(m_pShip->fAngle), 0.0f );
                D3DXVECTOR3 vDir = D3DXVECTOR3( sinf( m_pShip->fAngle ), -cosf( m_pShip->fAngle ), 0.0f );

                AddToList( new CBullet( m_pShip->vPos + vOffset, m_pShip->vVel + 2*vDir, 1 ) );
                AddToList( new CBullet( m_pShip->vPos - vOffset, m_pShip->vVel + 2*vDir, 1 ) );
                m_fBulletRechargeTime = 0.10f;
            }
            else if( m_dwBulletType == 2 )
            {
                FLOAT fBulletAngle = m_pShip->fAngle + 0.2f*rnd();
                D3DXVECTOR3 vDir = D3DXVECTOR3( sinf(fBulletAngle), -cosf(fBulletAngle), 0.0f );

                AddToList( new CBullet( m_pShip->vPos, m_pShip->vVel + 2*vDir, 2 ) );
                m_fBulletRechargeTime = 0.01f;
            }
            else
            {
                for( DWORD i=0; i<50; i++ )
                {
                    FLOAT fBulletAngle = m_pShip->fAngle + D3DX_PI*rnd();
                    D3DXVECTOR3 vDir = D3DXVECTOR3( sinf(fBulletAngle), -cosf(fBulletAngle), 0.0f );

                    AddToList( new CBullet( m_pShip->vPos, 2*vDir, 3 ) );
                }

                m_fBulletRechargeTime = 1.0f;
            }
        }
    }

    // Keep ship in bounds
    if( m_pShip->vPos.x < -5.0f || m_pShip->vPos.x > +5.0f ||
        m_pShip->vPos.y < -5.0f || m_pShip->vPos.y > +5.0f )
    {
         D3DXVec3Normalize( &m_pShip->vVel, &m_pShip->vPos );
         m_pShip->vVel.x *= -1.0f;
         m_pShip->vVel.y *= -1.0f;
         m_pShip->vVel.z *= -1.0f;
    }

    // Finally, move all objects on the screen
    for( pObject = m_pDisplayList; pObject; pObject = pObject->pNext )
    {
        // The ship is moved by the code above
        if( pObject->dwType == OBJ_SHIP )
            continue;

        C3DSprite* pSprite = (C3DSprite*)pObject;

        // Update the position and animation frame
        pSprite->vPos  += pSprite->vVel * fTimeLapsed;
        pSprite->frame += pSprite->delay * fTimeLapsed;

        // If this is an "expired" cloud, removed it from list
        if( pObject->dwType == OBJ_CLOUD )
        {
            if( pSprite->frame >= pSprite->fMaxFrame )
            {
                DisplayObject* pVictim = pObject;
                pObject = pObject->pPrev;
                DeleteFromList( pVictim );
            }
        }
        else if( pObject->dwType == OBJ_BULLET )
        {
            // Remove bullets when they leave the scene
            if( pObject->vPos.x < -6.0f || pObject->vPos.x > +6.0f ||
                pObject->vPos.y < -6.0f || pObject->vPos.y > +6.0f )
            {
                DisplayObject* pVictim = pObject;
                pObject = pObject->pPrev;
                DeleteFromList( pVictim );
            }
        }
        else if( pObject->dwType != OBJ_CLOUD )
        {
            // Keep object in bounds in X
            if( pObject->vPos.x < -4.0f || pObject->vPos.x > +4.0f )
            {
                if( pObject->vPos.x < -4.0f ) pObject->vPos.x = -4.0f;
                if( pObject->vPos.x > +4.0f ) pObject->vPos.x = +4.0f;
                pObject->vVel.x = -pObject->vVel.x;
            }

            // Keep object in bounds in Y
            if( pObject->vPos.y < -4.0f || pObject->vPos.y > +4.0f )
            {
                if( pObject->vPos.y < -4.0f ) pObject->vPos.y = -4.0f;
                if( pObject->vPos.y > +4.0f ) pObject->vPos.y = +4.0f;
                pObject->vVel.y = -pObject->vVel.y;
            }

            // Keep animation frame in bounds
            if( pSprite->frame < 0.0f )
                pSprite->frame += pSprite->fMaxFrame;
            if( pSprite->frame >= pSprite->fMaxFrame )
                pSprite->frame -= pSprite->fMaxFrame;
        }
    }

    D3DXVECTOR3 vEyePt[NUMVIEWMODES];
    D3DXVECTOR3 vLookatPt[NUMVIEWMODES];
    D3DXVECTOR3 vUpVec[NUMVIEWMODES];

    // Update the view
    if( m_UserInput.bDoChangeView )
    {
        m_bAnimatingViewChange = TRUE;
        m_UserInput.bDoChangeView = FALSE;
    }

    if( m_bAnimatingViewChange )
    {
        m_fViewTransition += fTimeLapsed;

        if( m_fViewTransition >= 1.0f )
        {
            m_dwViewMode++;
            if( m_dwViewMode >= NUMVIEWMODES )
                m_dwViewMode = 0;

            m_fViewTransition      = 0.0f;
            m_bAnimatingViewChange = FALSE;
        }
    }

    FLOAT fX =  m_pShip->vPos.x;
    FLOAT fZ = -m_pShip->vPos.y;
    FLOAT fY = 0.1f + HeightField( fX, fZ );

    // View mode 0 (third person)
    vEyePt[0]      = D3DXVECTOR3( fX-sinf(m_pShip->fAngle)/2, fY+0.2f, fZ-cosf(m_pShip->fAngle)/2 );
    vLookatPt[0]   = D3DXVECTOR3( fX+sinf(m_pShip->fAngle)/2, fY, fZ+cosf(m_pShip->fAngle)/2 );
    vUpVec[0]      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    // View mode 1 (first person)
    FLOAT fX2 = fX+sinf(m_pShip->fAngle);
    FLOAT fZ2 = fZ+cosf(m_pShip->fAngle);
    FLOAT fY2 = 0.1f + HeightField( fX2, fZ2 );
    vEyePt[1]    = D3DXVECTOR3( fX, fY+0.1f, fZ );
    vLookatPt[1] = D3DXVECTOR3( fX2, fY2+0.1f, fZ2 );
    vUpVec[1]    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    // View mode 2 (top down view)
    vEyePt[2]    = D3DXVECTOR3( fX+1.5f, fY+1.5f, fZ+1.5f );
    vLookatPt[2] = D3DXVECTOR3( fX, fY, fZ );
    vUpVec[2]    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    DWORD start = m_dwViewMode;
    DWORD end   = ( start < (NUMVIEWMODES-1) ) ? m_dwViewMode+1: 0;

    if( start == 1 && m_fViewTransition<0.2f)
        m_bFirstPersonView = TRUE;
    else
        m_bFirstPersonView = FALSE;

    D3DXVECTOR3 vEyePt0    = (1.0f-m_fViewTransition)*vEyePt[start]    + m_fViewTransition*vEyePt[end];
    D3DXVECTOR3 vLookatPt0 = (1.0f-m_fViewTransition)*vLookatPt[start] + m_fViewTransition*vLookatPt[end];
    D3DXVECTOR3 vUpVec0    = (1.0f-m_fViewTransition)*vUpVec[start]    + m_fViewTransition*vUpVec[end];

    // Shake screen if ship exploded
    if( m_pShip->bExploded == TRUE )
        vEyePt0 += D3DXVECTOR3( rnd(), rnd(), rnd() ) * m_pShip->fShowDelay / 50.0f;

    m_Camera.SetViewParams( vEyePt0, vLookatPt0, vUpVec0 );
}




//-----------------------------------------------------------------------------
// Name: CheckForHits()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::CheckForHits()
{
    DisplayObject* pObject;
    DisplayObject* pBullet;

    for( pBullet = m_pDisplayList; pBullet; pBullet = pBullet->pNext )
    {
        BOOL bBulletHit = FALSE;

        // Only bullet objects and the ship (if shieleds are on) can hit
        // other objects. Skip all others.
        if( (pBullet->dwType != OBJ_BULLET) && (pBullet->dwType != OBJ_SHIP) )
            continue;

        for( pObject = m_pDisplayList->pNext; pObject; pObject = pObject->pNext )
        {
            // Only trying to hit explodable targets
            if( ( pObject->dwType != OBJ_DONUT ) &&
                ( pObject->dwType != OBJ_PYRAMID ) &&
                ( pObject->dwType != OBJ_SPHERE ) &&
                ( pObject->dwType != OBJ_CUBE ) )
                continue;

            // Check if bullet is in radius of object
            FLOAT fDistance = D3DXVec3Length( &(pBullet->vPos - pObject->vPos) );

            if( fDistance < (pObject->fSize+pBullet->fSize) )
            {
                // The object was hit
                switch( pObject->dwType )
                {
                    case OBJ_DONUT:
                        PlaySound( m_pDonutExplodeSound );
                        AddToList( new CPyramid( pObject->vPos, pObject->vVel ) );
                        AddToList( new CPyramid( pObject->vPos, pObject->vVel ) );
                        AddToList( new CPyramid( pObject->vPos, pObject->vVel ) );
                        AddToList( new CPyramid( pObject->vPos, pObject->vVel ) );
                        m_dwScore += 10;
                        break;

                    case OBJ_PYRAMID:
                        PlaySound( m_pPyramidExplodeSound );
                        AddToList( new CCube( pObject->vPos, pObject->vVel ) );
                        AddToList( new CCube( pObject->vPos, pObject->vVel ) );
                        AddToList( new CCube( pObject->vPos, pObject->vVel ) );
                        AddToList( new CCube( pObject->vPos, pObject->vVel ) );
                        m_dwScore += 20;
                        break;

                    case OBJ_CUBE:
                        PlaySound( m_pCubeExplodeSound );
                        AddToList( new CSphere( pObject->vPos, pObject->vVel ) );
                        m_dwScore += 40;
                        break;

                    case OBJ_SPHERE:
                        PlaySound( m_pSphereExplodeSound );
                        m_dwScore += 20;
                        break;
                }

                // Add explosion effects to scene
                for( DWORD c=0; c<4; c++ )
                    AddToList( new CCloud( pObject->vPos, 0.05f*D3DXVECTOR3(rnd(),rnd(),0.0f) ) );

                // Remove the victim from the scene
                DisplayObject* pVictim = pObject;
                pObject = pObject->pPrev;
                DeleteFromList( pVictim );

                bBulletHit = TRUE;
            }

            if( bBulletHit )
            {
                if( pBullet->dwType == OBJ_SHIP )
                {
                    bBulletHit = FALSE;

                    if( m_pShip->bVisible )
                    {
                        // Ship has exploded
                        PlaySound( m_pShipExplodeSound );

                        if( m_dwScore < 150 )
                            m_dwScore = 0;
                        else
                            m_dwScore -= 150;

                        // Add explosion debris to scene
                        for( DWORD sphere=0; sphere<4; sphere++ )
                            AddToList( new CSphere( m_pShip->vPos, pObject->vVel ) );

                        for( DWORD bullet=0; bullet<20; bullet++ )
                        {
                            FLOAT     angle     = D3DX_PI * rnd();
                            D3DVECTOR vDir      = D3DXVECTOR3(cosf(angle),sinf(angle),0.0f);
                            AddToList( new CBullet( m_pShip->vPos, 500.0f*vDir, 0 ) );
                        }

                        for( DWORD cloud=0; cloud<100; cloud++ )
                        {
                            FLOAT     magnitude = 1.0f + 0.1f*rnd();
                            FLOAT     angle     = D3DX_PI * rnd();
                            D3DVECTOR vDir      = D3DXVECTOR3(cosf(angle),sinf(angle),0.0f);

                            AddToList( new CCloud( m_pShip->vPos, magnitude*vDir ) );
                        }

                        // Clear out ship params
                        m_pShip->vVel.x = 0.0f;
                        m_pShip->vVel.y = 0.0f;
                        m_UserInput.fAxisMoveUD       = 0.0f;
                        m_UserInput.fAxisRotateLR         = 0.0f;

                        // Delay for 2 seconds before displaying ship
                        m_pShip->fShowDelay = 2.0f;
                        m_pShip->bVisible   = FALSE;
                        m_pShip->bExploded  = TRUE;
                    }
                }

                break;
            }
        }

        if( bBulletHit )
        {
            DisplayObject* pLastBullet = pBullet;
            pBullet = pBullet->pPrev;
            DeleteFromList( pLastBullet );
        }
    }
}




//-----------------------------------------------------------------------------
// Name: DrawDisplayList()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DrawDisplayList()
{
    TCHAR strBuffer[80];

    // Set the world matrix
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Set the app view matrix for normal viewing
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_Camera.GetViewMatrix() );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x33333333 );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Clear the display
        m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0L, 1.0f, 0L );

        // Draw the terrain
        m_pTerrain->Render( m_pd3dDevice );

        // Render the ship
        if( m_pShip->bVisible && m_bFirstPersonView == FALSE )
        {
            // Point of ship, on terrain
            D3DXVECTOR3 vShipPt;
            vShipPt.x =  m_pShip->vPos.x;
            vShipPt.z = -m_pShip->vPos.y;
            vShipPt.y = 0.1f + HeightField( vShipPt.x, vShipPt.z );

            // Point ahead of ship, on terrain
            D3DXVECTOR3 vForwardPt;
            vForwardPt.x = vShipPt.x+sinf(m_pShip->fAngle);
            vForwardPt.z = vShipPt.z+cosf(m_pShip->fAngle);
            vForwardPt.y = 0.1f + HeightField( vForwardPt.x, vForwardPt.z );

            // Point to side of ship, on terrain
            D3DXVECTOR3 vSidePt;
            vSidePt.x = vShipPt.x+sinf(m_pShip->fAngle + D3DX_PI/2.0f);
            vSidePt.z = vShipPt.z+cosf(m_pShip->fAngle + D3DX_PI/2.0f);
            vSidePt.y = 0.1f + HeightField( vSidePt.x, vSidePt.z );

            // Compute vectors of the ship's orientation
            D3DXVECTOR3 vForwardDir = vForwardPt - vShipPt;
            D3DXVECTOR3 vSideDir    = vSidePt - vShipPt;
            D3DXVECTOR3 vNormalDir;
            D3DXVec3Cross( &vNormalDir, &vForwardDir, &vSideDir );

            // Construct matrix to orient ship
            D3DXMATRIX matWorld, matLookAt, matRotateZ;
            D3DXMatrixRotationZ( &matRotateZ, m_pShip->fRoll );
            D3DXMatrixLookAtLH( &matLookAt, &vShipPt, &(vShipPt-vForwardDir), &vNormalDir );
            D3DXMatrixInverse( &matLookAt, NULL, &matLookAt );
            D3DXMatrixIdentity( &matWorld );
            D3DXMatrixMultiply( &matWorld, &matWorld, &matRotateZ );
            D3DXMatrixMultiply( &matWorld, &matWorld, &matLookAt );

            // Set renderstates for rendering the ship
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
            m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,           TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS,   TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
            m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,    FALSE );

            // Render the ship - opaque parts
            m_pShipFileObject->Render( m_pd3dDevice, TRUE, FALSE );

            // Render the ship - transparent parts
            m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
            m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,     D3DBLEND_ONE );
            m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,    D3DBLEND_ONE );
            m_pShipFileObject->Render( m_pd3dDevice, FALSE, TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
        }

        // Remaining objects don't need lighting
        m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,           FALSE );

        // Enable alpha blending and testing
        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

        // Display all visible objects in the display list
        for( DisplayObject* pObject = m_pDisplayList; pObject; pObject = pObject->pNext )
        {
            if( !pObject->bVisible )
                continue;
            if( pObject->dwType == OBJ_SHIP )
                continue;
            if( pObject->dwType == OBJ_BULLET )
                continue;

            // This is really a 3D sprite
            C3DSprite* pSprite = (C3DSprite*)pObject;

            FLOAT fX =  pObject->vPos.x;
            FLOAT fZ = -pObject->vPos.y;
            FLOAT fY =  HeightField( fX, fZ );

            FLOAT x1 = -pObject->fSize;
            FLOAT x2 =  pObject->fSize;
            FLOAT y1 = -pObject->fSize;
            FLOAT y2 =  pObject->fSize;

            FLOAT u1 = (FLOAT)(pSprite->dwTextureOffsetX + pSprite->dwTextureWidth *(((int)pSprite->frame)%pSprite->dwFramesPerLine));
            FLOAT v1 = (FLOAT)(pSprite->dwTextureOffsetY + pSprite->dwTextureHeight*(((int)pSprite->frame)/pSprite->dwFramesPerLine));

            FLOAT tu1 = u1 / (256.0f-1.0f);
            FLOAT tv1 = v1 / (256.0f-1.0f);
            FLOAT tu2 = (u1 + pSprite->dwTextureWidth -1) / (256.0f-1.0f);
            FLOAT tv2 = (v1 + pSprite->dwTextureHeight-1) / (256.0f-1.0f);

            // Set the game texture
            switch( pObject->dwType )
            {
                case OBJ_DONUT:
                case OBJ_CUBE:
                case OBJ_SPHERE:
                    m_pd3dDevice->SetTexture( 0, m_pGameTexture1 );
                    break;
                case OBJ_PYRAMID:
                case OBJ_CLOUD:
                    m_pd3dDevice->SetTexture( 0, m_pGameTexture2 );
                    break;
            }

            // Translate the billboard into place
            D3DXMATRIX mat = m_Camera.GetBillboardMatrix();
            mat._41 = fX;
            mat._42 = fY;
            mat._43 = fZ;
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat );

            DWORD dwColor = pSprite->dwColor;

            m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
            m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
            m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

            if( pObject->dwType == OBJ_CLOUD )
            {
                DWORD red = 255-(int)(pSprite->frame*255.0f);
                DWORD grn = 255-(int)(pSprite->frame*511.0f);
                DWORD blu = 255-(int)(pSprite->frame*1023.0f);
                if( grn > 255 ) grn = 0;
                if( blu > 255 ) blu = 0;
                dwColor = 0xff000000 + (red<<16) + (grn<<8) + blu;

                m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
                m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
                m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
                m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
            }

            FLOAT h = 300.0f*pObject->vPos.z + 0.1f;

            SPRITEVERTEX* v;
            m_pSpriteVB->Lock( 0, 0, (BYTE**)&v, 0 );
            v[0].p = D3DXVECTOR3(x1,y1+h,0); v[0].color=dwColor; v[0].tu=tu1; v[0].tv=tv2;
            v[1].p = D3DXVECTOR3(x1,y2+h,0); v[1].color=dwColor; v[1].tu=tu1; v[1].tv=tv1;
            v[2].p = D3DXVECTOR3(x2,y1+h,0); v[2].color=dwColor; v[2].tu=tu2; v[2].tv=tv2;
            v[3].p = D3DXVECTOR3(x2,y2+h,0); v[3].color=dwColor; v[3].tu=tu2; v[3].tv=tv1;
            m_pSpriteVB->Unlock();

            // Render the billboarded sprite
            m_pd3dDevice->SetVertexShader( D3DFVF_SPRITEVERTEX );
            m_pd3dDevice->SetStreamSource( 0, m_pSpriteVB, sizeof(SPRITEVERTEX) );
            m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
        }

        // Display all bullets
        for( pObject = m_pDisplayList; pObject; pObject = pObject->pNext )
        {
            if( pObject->dwType != OBJ_BULLET )
                continue;

            // This is really a 3D sprite
            C3DSprite* pSprite = (C3DSprite*)pObject;

            FLOAT u1 = (FLOAT)(pSprite->dwTextureOffsetX + pSprite->dwTextureWidth *(((int)pSprite->frame)%pSprite->dwFramesPerLine));
            FLOAT v1 = (FLOAT)(pSprite->dwTextureOffsetY + pSprite->dwTextureHeight*(((int)pSprite->frame)/pSprite->dwFramesPerLine));
            u1 = (FLOAT)(pSprite->dwTextureOffsetX);
            v1 = (FLOAT)(pSprite->dwTextureOffsetY);

            FLOAT tu1 = u1 / (256.0f-1.0f);
            FLOAT tv1 = v1 / (256.0f-1.0f);
            FLOAT tu2 = (u1 + pSprite->dwTextureWidth -1) / (256.0f-1.0f);
            FLOAT tv2 = (v1 + pSprite->dwTextureHeight-1) / (256.0f-1.0f);

            // Set render states
            m_pd3dDevice->SetTexture( 0, m_pGameTexture2 );
            m_pd3dDevice->SetVertexShader( D3DFVF_SPRITEVERTEX );
            m_pd3dDevice->SetStreamSource( 0, m_pSpriteVB, sizeof(SPRITEVERTEX) );
            m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,     D3DBLEND_ONE );
            m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,    D3DBLEND_ONE );
            m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
            m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

            FLOAT x1 = -0.01f;
            FLOAT x2 =  0.01f;
            FLOAT y1 = -0.01f;
            FLOAT y2 =  0.01f;

            DWORD dwColor = pSprite->dwColor;

            for( DWORD a=0; a<6; a++ )
            {
                FLOAT fX =  pObject->vPos.x - a*a*0.0005f*pObject->vVel.x;
                FLOAT fZ = -pObject->vPos.y + a*a*0.0005f*pObject->vVel.y;
                FLOAT fY =  HeightField( fX, fZ );

                // Translate the billboard into place
                D3DXMATRIX mat = m_Camera.GetBillboardMatrix();
                mat._41 = fX;
                mat._42 = fY;
                mat._43 = fZ;
                m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat );

                FLOAT h = 300.0f*pObject->vPos.z + 0.1f;

                SPRITEVERTEX* v;
                m_pSpriteVB->Lock( 0, 0, (BYTE**)&v, 0 );
                v[0].p = D3DXVECTOR3(x1,y1+h,0); v[0].color=dwColor; v[0].tu=tu1; v[0].tv=tv2;
                v[1].p = D3DXVECTOR3(x1,y2+h,0); v[1].color=dwColor; v[1].tu=tu1; v[1].tv=tv1;
                v[2].p = D3DXVECTOR3(x2,y1+h,0); v[2].color=dwColor; v[2].tu=tu2; v[2].tv=tv2;
                v[3].p = D3DXVECTOR3(x2,y2+h,0); v[3].color=dwColor; v[3].tu=tu2; v[3].tv=tv1;
                m_pSpriteVB->Unlock();

                // Render the billboarded sprite
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
            }
        }

        // Restore state
        m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

        // Display help
        _stprintf( strBuffer, _T("F1 for menu") );
        m_pGameFont->DrawTextScaled( -1.0f, 1.1f, 0.9f, 0.05f, 0.05f,
                                     0xffffff00, strBuffer, D3DFONT_FILTERED );

        // Display score
        _stprintf( strBuffer, _T("Score: %08ld"), m_dwScore );
        m_pGameFont->DrawTextScaled( -1.0f, 1.0f, 0.9f, 0.05f, 0.05f,
                                     0xffffff00, strBuffer, D3DFONT_FILTERED );

        // Display ship type
        _stprintf( strBuffer, _T("Ship: %s"), g_strShipNames[m_dwCurrentShipType] );
        m_pGameFont->DrawTextScaled( 0.0f, 1.0f, 0.9f, 0.05f, 0.05f,
                                     0xffffff00, strBuffer, D3DFONT_FILTERED );

        // Display weapon type
        TCHAR* strWeapon;
        if( m_dwBulletType == 0 )      strWeapon = _T("Weapon: Blaster");
        else if( m_dwBulletType == 1 ) strWeapon = _T("Weapon: Double blaster");
        else if( m_dwBulletType == 2 ) strWeapon = _T("Weapon: Spray gun");
        else                           strWeapon = _T("Weapon: Proximity killer");
        m_pGameFont->DrawTextScaled( 0.0f, 1.1f, 0.9f, 0.05f, 0.05f,
                                     0xffffff00, strWeapon, D3DFONT_FILTERED );

        // Render "Paused" text if game is paused
        if( m_bPaused && m_pGameFont )
        {
            DarkenScene( 0.5f );
            RenderFieryText( m_pMenuFont, _T("Paused") );
        }

        if( m_pShip->fShowDelay > 0.0f )
            DarkenScene( m_pShip->fShowDelay/2.0f );

        // Render game menu
        if( m_pCurrentMenu )
        {
            DarkenScene( 0.5f );
            m_pCurrentMenu->Render( m_pd3dDevice, m_pMenuFont );
        }

        m_pd3dDevice->EndScene();
    }
}




//-----------------------------------------------------------------------------
// Name: DeleteFromList()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DeleteFromList( DisplayObject* pObject )
{
    if( pObject->pNext )
        pObject->pNext->pPrev = pObject->pPrev;
    if( pObject->pPrev )
        pObject->pPrev->pNext = pObject->pNext;
    delete( pObject );
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::ConstructMenus()
{
    // Build video sub menu
    CMenuItem* pVideoSubMenu = new CMenuItem( _T("Video Menu"), MENU_VIDEO );
    pVideoSubMenu->Add( new CMenuItem( _T("Windowed"), MENU_WINDOWED ) );
    pVideoSubMenu->Add( new CMenuItem( _T("640x480"),  MENU_640x480 ) );
    pVideoSubMenu->Add( new CMenuItem( _T("800x600"),  MENU_800x600 ) );
    pVideoSubMenu->Add( new CMenuItem( _T("1024x768"), MENU_1024x768 ) );
    pVideoSubMenu->Add( new CMenuItem( _T("Back"),     MENU_BACK ) );

    // Build sound menu
    CMenuItem* pSoundSubMenu = new CMenuItem( _T("Sound Menu"), MENU_SOUND );
    pSoundSubMenu->Add( new CMenuItem( _T("Sound On"),  MENU_SOUNDON ) );
    pSoundSubMenu->Add( new CMenuItem( _T("Sound Off"), MENU_SOUNDOFF ) );
    pSoundSubMenu->Add( new CMenuItem( _T("Back"),      MENU_BACK ) );

    // Build input menu
    CMenuItem* pInputSubMenu = new CMenuItem( _T("Input Menu"), MENU_INPUT );
    pInputSubMenu->Add( new CMenuItem( _T("View Devices"),   MENU_VIEWDEVICES ) );
    pInputSubMenu->Add( new CMenuItem( _T("Config Devices"), MENU_CONFIGDEVICES ) );
    pInputSubMenu->Add( new CMenuItem( _T("Back"),           MENU_BACK ) );

    // Build main menu
    m_pMainMenu = new CMenuItem( _T("Main Menu"),  MENU_MAIN );
    m_pMainMenu->Add( pVideoSubMenu );
    m_pMainMenu->Add( pSoundSubMenu );
    m_pMainMenu->Add( pInputSubMenu );
    m_pMainMenu->Add( new CMenuItem( _T("Back to Game"), MENU_BACK ) );

    // Build "quit game?" menu
    m_pQuitMenu = new CMenuItem( _T("Quit Game?"),  MENU_MAIN );
    m_pQuitMenu->Add( new CMenuItem( _T("Yes"),     MENU_QUIT ) );
    m_pQuitMenu->Add( new CMenuItem( _T("No"),      MENU_BACK ) );

    return;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DestroyMenus()
{
    SAFE_DELETE( m_pQuitMenu );
    SAFE_DELETE( m_pMainMenu );
}




//-----------------------------------------------------------------------------
// Name: UpdateMenus()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::UpdateMenus()
{
    if( m_pCurrentMenu == NULL )
        return;

    // Keep track of current selected menu, to check later for changes
    DWORD dwCurrentSelectedMenu = m_pCurrentMenu->dwSelectedMenu;

    // Check for menu up/down input
    if( m_UserInput.bDoMenuUp )
    {
        m_UserInput.bDoMenuUp = FALSE;
        if( m_pCurrentMenu->dwSelectedMenu > 0 )
            m_pCurrentMenu->dwSelectedMenu--;
    }
    else if( m_UserInput.bDoMenuDown )
    {
        m_UserInput.bDoMenuDown = FALSE;
        if( (m_pCurrentMenu->dwSelectedMenu+1) < m_pCurrentMenu->dwNumChildren )
            m_pCurrentMenu->dwSelectedMenu++;
    }

    // The the current menu changed, play a sound
    if( dwCurrentSelectedMenu != m_pCurrentMenu->dwSelectedMenu )
        PlaySound( m_pSphereExplodeSound );

    if( m_UserInput.bDoMenuSelect )
    {
        m_UserInput.bDoMenuSelect = FALSE;
        PlaySound( m_pSphereExplodeSound );

        DWORD dwID = m_pCurrentMenu->pChild[m_pCurrentMenu->dwSelectedMenu]->dwID;

        switch( dwID )
        {
            case MENU_BACK:
                m_pCurrentMenu = m_pCurrentMenu->pParent;
                break;

            case MENU_VIDEO:
            case MENU_SOUND:
            case MENU_INPUT:
                m_pCurrentMenu = m_pCurrentMenu->pChild[m_pCurrentMenu->dwSelectedMenu];
                break;

            case MENU_WINDOWED:
                SwitchDisplayModes( FALSE, 0L, 0L );
                m_pCurrentMenu = NULL;
                break;

            case MENU_640x480:
                SwitchDisplayModes( TRUE, 640, 480 );
                m_pCurrentMenu = NULL;
                break;

            case MENU_800x600:
                SwitchDisplayModes( TRUE, 800, 600 );
                m_pCurrentMenu = NULL;
                break;

            case MENU_1024x768:
                SwitchDisplayModes( TRUE, 1024, 768 );
                m_pCurrentMenu = NULL;
                break;

            case MENU_SOUNDON:
                if( m_pMusicManager == NULL )
                    CreateSoundObjects( m_hWndMain );
                m_pCurrentMenu = NULL;
                break;

            case MENU_SOUNDOFF:
                if( m_pMusicManager )
                    DestroySoundObjects();
                m_pCurrentMenu = NULL;
                break;

            case MENU_VIEWDEVICES:
            {
                // Put action format to game play actions
                m_pInputDeviceManager->SetActionFormat( m_diafGame, FALSE );

                m_bMouseVisible = TRUE;
                DXUtil_Timer( TIMER_STOP );

                // Configure the devices (with view capability only)
                if( m_bFullScreen )
                    m_pInputDeviceManager->ConfigureDevices( m_hWndMain,
                                                             m_pConfigSurface,
                                                             (VOID*)StaticConfigureInputDevicesCB,
                                                             DICD_DEFAULT, this );
                else
                    m_pInputDeviceManager->ConfigureDevices( m_hWndMain, NULL, NULL,
                                                             DICD_DEFAULT, this );

                m_bMouseVisible = FALSE;
                DXUtil_Timer( TIMER_START );

                m_pCurrentMenu = NULL;
                break;
            }

            case MENU_CONFIGDEVICES:
            {
                // Put action format to game play actions
                m_pInputDeviceManager->SetActionFormat( m_diafGame, FALSE );

                m_bMouseVisible = TRUE;
                DXUtil_Timer( TIMER_STOP );

                // Get access to the list of semantically-mapped input devices
                // to delete all InputDeviceState structs before calling ConfigureDevices()
                CInputDeviceManager::DeviceInfo* pDeviceInfos;
                DWORD dwNumDevices;
                m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

                for( DWORD i=0; i<dwNumDevices; i++ )
                {
                    InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
                    SAFE_DELETE( pInputDeviceState );
                    pDeviceInfos[i].pParam = NULL;
                }

                // Configure the devices (with edit capability)
                if( m_bFullScreen )
                    m_pInputDeviceManager->ConfigureDevices( m_hWndMain,
                                                             m_pConfigSurface,
                                                             (VOID*)StaticConfigureInputDevicesCB,
                                                             DICD_EDIT, this );
                else
                    m_pInputDeviceManager->ConfigureDevices( m_hWndMain, NULL, NULL,
                                                             DICD_EDIT, this );

                DXUtil_Timer( TIMER_START );
                m_bMouseVisible = FALSE;

                m_pCurrentMenu = NULL;
                break;
            }

            case MENU_QUIT:
                PostMessage( m_hWndMain, WM_CLOSE, 0, 0 );
                m_pCurrentMenu = NULL;
                break;
        }
    }

    // Check if the menu system is being exitted
    if( m_UserInput.bDoMenuQuit )
    {
        m_UserInput.bDoMenuQuit = FALSE;
        m_pCurrentMenu = NULL;
    }

    // If the menu is going away, go back to game play actions
    if( m_pCurrentMenu == NULL )
        m_pInputDeviceManager->SetActionFormat( m_diafGame, FALSE );
}




//-----------------------------------------------------------------------------
// Display support code (using Direct3D functionality from D3DUtil.h)
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: CreateDisplayObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::CreateDisplayObjects( HWND hWnd )
{
    HRESULT hr, hr1, hr2;

    // Construct a new display
    LPDIRECT3D8 pD3D = Direct3DCreate8( D3D_SDK_VERSION );
    if( NULL == pD3D )
    {
        CleanupAndDisplayError( DONUTS3DERR_NODIRECT3D );
        return E_FAIL;
    }

    // Get the current desktop format
    pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &m_DesktopMode );

    const D3DFORMAT fmtFullscreenArray[] = 
    {
        D3DFMT_R5G6B5,
        D3DFMT_X1R5G5B5,
        D3DFMT_A1R5G5B5,
        D3DFMT_X8R8G8B8,
        D3DFMT_A8R8G8B8,
    };
    const INT numFullscreenFmts = sizeof(fmtFullscreenArray) / sizeof(fmtFullscreenArray[0]);
    INT iFmt;

    // Find a pixel format that will be good for fullscreen back buffers
    for( iFmt = 0; iFmt < numFullscreenFmts; iFmt++ )
    {
        if( SUCCEEDED( pD3D->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
            fmtFullscreenArray[iFmt], fmtFullscreenArray[iFmt], FALSE ) ) )
        {
            m_d3dfmtFullscreen = fmtFullscreenArray[iFmt];
            break;
        }
    }

    const D3DFORMAT fmtTextureArray[] = 
    {
        D3DFMT_A1R5G5B5,
        D3DFMT_A4R4G4B4,
        D3DFMT_A8R8G8B8,
    };
    const INT numTextureFmts = sizeof(fmtTextureArray) / sizeof(fmtTextureArray[0]);

    // Find a format that is supported as a texture map for the current mode
    for( iFmt = 0; iFmt < numTextureFmts; iFmt++ )
    {
        if( SUCCEEDED( pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
            m_DesktopMode.Format, 0, D3DRTYPE_TEXTURE, fmtTextureArray[iFmt] ) ) )
        {
            m_d3dfmtTexture = fmtTextureArray[iFmt];
            break;
        }
    }

    // Set up presentation parameters for the display
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed         = !m_bFullScreen;
    m_d3dpp.BackBufferCount  = 1;
    m_d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    if( m_bFullScreen )
    {
        m_d3dpp.hDeviceWindow    = hWnd;
        m_d3dpp.BackBufferWidth  = m_dwScreenWidth;
        m_d3dpp.BackBufferHeight = m_dwScreenHeight;
        m_d3dpp.BackBufferFormat = m_d3dfmtFullscreen;
    }
    else
    {
        m_d3dpp.BackBufferFormat = m_DesktopMode.Format;
    }

    // Create the device
    hr = pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                             &m_d3dpp, &m_pd3dDevice );
    pD3D->Release();
    if( FAILED(hr) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NOD3DDEVICE );
        return E_FAIL;
    }

    // Create some game fonts
    m_pGameFont = new CD3DFont( _T("Tahoma"), 30, 0L );
    m_pGameFont->InitDeviceObjects( m_pd3dDevice );

    m_pMenuFont = new CD3DFont( _T("Impact"), 48, 0L );
    m_pMenuFont->InitDeviceObjects( m_pd3dDevice );

    // Find the media files (textures and geometry models) for the game
    TCHAR strGameTexture1[512];
    TCHAR strGameTexture2[512];
    hr1 = DXUtil_FindMediaFile( strGameTexture1, _T("Donuts1.bmp") );
    hr2 = DXUtil_FindMediaFile( strGameTexture2, _T("Donuts2.bmp") );

    if( FAILED(hr1) || FAILED(hr2) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NOTEXTURES );
        return E_FAIL;
    }

    // Load the game textures
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, strGameTexture1,
                                       &m_pGameTexture1, m_d3dfmtTexture ) ) ||
        FAILED( D3DUtil_CreateTexture( m_pd3dDevice, strGameTexture2,
                                       &m_pGameTexture2, m_d3dfmtTexture ) ) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NOTEXTURES );
        return E_FAIL;
    }

    D3DUtil_SetColorKey( m_pGameTexture1, 0x00000000 );
    D3DUtil_SetColorKey( m_pGameTexture2, 0x00000000 );

    // Load the geometry models
    hr1 = LoadShipModel();
    hr2 = LoadTerrainModel();

    if( FAILED(hr1) || FAILED(hr2) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NOGEOMETRY );
        return E_FAIL;
    }

    // Create a viewport covering sqaure
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(SCREENVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_SCREENVERTEX,
                                       D3DPOOL_MANAGED, &m_pViewportVB ) ) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NO3DRESOURCES );
        return E_FAIL;
    }

    // Create a sqaure for rendering the sprites
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(SPRITEVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_SPRITEVERTEX,
                                       D3DPOOL_MANAGED, &m_pSpriteVB ) ) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NO3DRESOURCES );
        return E_FAIL;
    }

    // Now that all the display objects are created, "restore" them (create
    // local mem objects and set state)
    if( FAILED( RestoreDisplayObjects() ) )
                return E_FAIL;

        // The display is now ready
        m_bDisplayReady = TRUE;
        return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDisplayObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::RestoreDisplayObjects()
{
    HRESULT hr;
    HWND hWnd = m_hWndMain;

    if( FALSE == m_bFullScreen )
    {
        // If we are still a WS_POPUP window we should convert to a normal app
        // window so we look like a windows app.
        DWORD dwStyle  = GetWindowStyle( hWnd );
        dwStyle &= ~WS_POPUP;
        dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
        SetWindowLong( hWnd, GWL_STYLE, dwStyle );

        // Aet window size
        RECT rc;
        SetRect( &rc, 0, 0, 640, 480 );

        AdjustWindowRectEx( &rc, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL,
                            GetWindowExStyle(hWnd) );

        SetWindowPos( hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
                      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

        SetWindowPos( hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                      SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );

        //  Make sure our window does not hang outside of the work area
        RECT rcWork;
        SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
        GetWindowRect( hWnd, &rc );
        if( rc.left < rcWork.left ) rc.left = rcWork.left;
        if( rc.top  < rcWork.top )  rc.top  = rcWork.top;
        SetWindowPos( hWnd, NULL, rc.left, rc.top, 0, 0,
                      SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
    }

    // Create the device-dependent objects for the file-based mesh objects
    m_pShipFileObject->RestoreDeviceObjects( m_pd3dDevice );
    m_pTerrain->RestoreDeviceObjects( m_pd3dDevice );
    m_pGameFont->RestoreDeviceObjects();
    m_pMenuFont->RestoreDeviceObjects();

    // Get viewport dimensions
    D3DVIEWPORT8 vp;
    m_pd3dDevice->GetViewport(&vp);
    FLOAT sx = (FLOAT)vp.Width;
    FLOAT sy = (FLOAT)vp.Height;

    // Setup dimensions for the viewport covering sqaure
    SCREENVERTEX* v;
    m_pViewportVB->Lock( 0, 0, (BYTE**)&v, 0 );
    v[0].p = D3DXVECTOR4( 0,sy,0.0f,1.0f);
    v[1].p = D3DXVECTOR4( 0, 0,0.0f,1.0f);
    v[2].p = D3DXVECTOR4(sx,sy,0.0f,1.0f);
    v[3].p = D3DXVECTOR4(sx, 0,0.0f,1.0f);
    m_pViewportVB->Unlock();

    // Create a surface for confguring DInput devices
    hr = m_pd3dDevice->CreateImageSurface( 640, 480, m_d3dfmtFullscreen, &m_pConfigSurface );
    if( FAILED(hr) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NO3DRESOURCES );
        return E_FAIL;
    }

    // Set up the camera
    m_Camera.SetViewParams( D3DXVECTOR3(0.0f,0.0f,0.0f), D3DXVECTOR3(0.0f,0.0f,1.0f),
                            D3DXVECTOR3(0.0f,1.0f,0.0f) );
    m_Camera.SetProjParams( D3DX_PI/4, 1.0f, 0.1f, 100.0f );

    // Set up default matrices (using the CD3DCamera class)
    m_pd3dDevice->SetTransform( D3DTS_VIEW,       &m_Camera.GetViewMatrix() );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &m_Camera.GetProjMatrix() );

    // Setup a material
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set up lighting states
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 1.0f, -1.0f, 1.0f );
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );

    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x33333333 );

    // Set miscellaneous render states
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDisplayObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InvalidateDisplayObjects()
{
    m_pShipFileObject->InvalidateDeviceObjects();
    m_pTerrain->InvalidateDeviceObjects();
    m_pGameFont->InvalidateDeviceObjects();
    m_pMenuFont->InvalidateDeviceObjects();

    SAFE_RELEASE( m_pConfigSurface );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: DestroyDisplayObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::DestroyDisplayObjects()
{
    DisplayObject* pDO;
    while( m_pDisplayList != NULL )
    {
        pDO = m_pDisplayList;
        m_pDisplayList = m_pDisplayList->pNext;
        delete pDO;
        if( m_pDisplayList != NULL)
            m_pDisplayList->pPrev = NULL;
    }
    
    SAFE_RELEASE( m_pGameTexture1 );
    SAFE_RELEASE( m_pGameTexture2 );

    SAFE_DELETE( m_pGameFont );
    SAFE_DELETE( m_pMenuFont );

    SAFE_RELEASE( m_pConfigSurface );

    if( m_pShipFileObject != NULL )
        m_pShipFileObject->Destroy();
    if( m_pTerrain != NULL )
        m_pTerrain->Destroy();

    SAFE_DELETE( m_pTerrain );
    SAFE_DELETE( m_pShipFileObject );

    SAFE_RELEASE( m_pViewportVB );
    SAFE_RELEASE( m_pSpriteVB );
    SAFE_RELEASE( m_pd3dDevice );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SwitchDisplayModes()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::SwitchDisplayModes( BOOL bFullScreen, DWORD dwWidth, DWORD dwHeight )
{
    HRESULT hr;

    if( FALSE==m_bIsActive || FALSE==m_bDisplayReady )
        return S_OK;

    // Check to see if a change was actually requested
    if( bFullScreen )
    {
        if( m_dwScreenWidth==dwWidth && m_dwScreenHeight==dwHeight &&
            m_bFullScreen==bFullScreen )
            return S_OK;
    }
    else
    {
        if( m_bFullScreen == FALSE )
            return S_OK;
    }

    // Invalidate the old display objects
    m_bDisplayReady = FALSE;
    InvalidateDisplayObjects();

    // Set up the new presentation paramters
    if( bFullScreen )
    {
        m_d3dpp.Windowed         = FALSE;
        m_d3dpp.hDeviceWindow    = m_hWndMain;
        m_d3dpp.BackBufferWidth  = m_dwScreenWidth  = dwWidth;
        m_d3dpp.BackBufferHeight = m_dwScreenHeight = dwHeight;
        m_d3dpp.BackBufferFormat = m_d3dfmtFullscreen;
    }
    else
    {
        m_d3dpp.Windowed         = TRUE;
        m_d3dpp.hDeviceWindow    = NULL;
        m_d3dpp.BackBufferWidth  = 0L;
        m_d3dpp.BackBufferHeight = 0L;

        m_d3dpp.BackBufferFormat = m_DesktopMode.Format;
    }

    // Reset the device
    if( SUCCEEDED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
    {
        m_bFullScreen   = bFullScreen;
        if( SUCCEEDED( hr = RestoreDisplayObjects() ) )
        {
            m_bDisplayReady = TRUE;
            SetCursor( NULL );
            return S_OK;
        }
    }

    // If we get here, a fatal error occurred
    PostMessage( m_hWndMain, WM_CLOSE, 0, 0 );
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: ShowFrame()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::ShowFrame()
{
    if( NULL == m_pd3dDevice )
        return;

    // Present the backbuffer contents to the front buffer
    m_pd3dDevice->Present( 0, 0, 0, 0 );
}




//-----------------------------------------------------------------------------
// Sound support code (using DMusic functionality from DMUtil.h)
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: CreateSoundObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::CreateSoundObjects( HWND hWnd )
{
    // Create the music manager class, used to create the sounds
    m_pMusicManager = new CMusicManager();
    if( FAILED( m_pMusicManager->Initialize( hWnd ) ) )
        return E_FAIL;

    // Instruct the music manager where to find the files
    m_pMusicManager->SetSearchDirectory( DXUtil_GetDXSDKMediaPath() );

    // Create the sounds
    m_pMusicManager->CreateSegmentFromResource( &m_pBeginLevelSound,     _T("BEGINLEVEL"), _T("WAV") );
    m_pMusicManager->CreateSegmentFromResource( &m_pEngineIdleSound,     _T("ENGINEIDLE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pEngineRevSound,      _T("ENGINEREV") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pShieldBuzzSound,     _T("SHIELDBUZZ") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pShipExplodeSound,    _T("SHIPEXPLODE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pFireBulletSound,     _T("GUNFIRE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pShipBounceSound,     _T("SHIPBOUNCE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pDonutExplodeSound,   _T("DONUTEXPLODE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pPyramidExplodeSound, _T("PYRAMIDEXPLODE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pCubeExplodeSound,    _T("CUBEEXPLODE") , _T("WAV"));
    m_pMusicManager->CreateSegmentFromResource( &m_pSphereExplodeSound,  _T("SPHEREEXPLODE") , _T("WAV"));

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroySoundObjects()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DestroySoundObjects()
{
    SAFE_DELETE( m_pBeginLevelSound );
    SAFE_DELETE( m_pEngineIdleSound );
    SAFE_DELETE( m_pEngineRevSound );
    SAFE_DELETE( m_pShieldBuzzSound );
    SAFE_DELETE( m_pShipExplodeSound );
    SAFE_DELETE( m_pFireBulletSound );
    SAFE_DELETE( m_pShipBounceSound );
    SAFE_DELETE( m_pDonutExplodeSound );
    SAFE_DELETE( m_pPyramidExplodeSound );
    SAFE_DELETE( m_pCubeExplodeSound );
    SAFE_DELETE( m_pSphereExplodeSound );

    SAFE_DELETE( m_pMusicManager );
}




//-----------------------------------------------------------------------------
// Input support code (using DInput functionality from DIUtil.h)
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Name: CreateInputObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyApplication::CreateInputObjects( HWND hWnd )
{
    HRESULT hr;

    // Setup action format for the acutal gameplay
    ZeroMemory( &m_diafGame, sizeof(DIACTIONFORMAT) );
    m_diafGame.dwSize          = sizeof(DIACTIONFORMAT);
    m_diafGame.dwActionSize    = sizeof(DIACTION);
    m_diafGame.dwDataSize      = NUMBER_OF_GAMEACTIONS * sizeof(DWORD);
    m_diafGame.guidActionMap   = g_guidApp;
    m_diafGame.dwGenre         = DIVIRTUAL_SPACESIM;
    m_diafGame.dwNumActions    = NUMBER_OF_GAMEACTIONS;
    m_diafGame.rgoAction       = g_rgGameAction;
    m_diafGame.lAxisMin        = -10;
    m_diafGame.lAxisMax        = 10;
    m_diafGame.dwBufferSize    = 16;
    _tcscpy( m_diafGame.tszActionMap, _T("Donuts3D New") );

    // Setup action format for the in-game menus
    ZeroMemory( &m_diafBrowser, sizeof(DIACTIONFORMAT) );
    m_diafBrowser.dwSize          = sizeof(DIACTIONFORMAT);
    m_diafBrowser.dwActionSize    = sizeof(DIACTION);
    m_diafBrowser.dwDataSize      = NUMBER_OF_BROWSERACTIONS * sizeof(DWORD);
    m_diafBrowser.guidActionMap   = g_guidApp;
    m_diafBrowser.dwGenre         = DIVIRTUAL_BROWSER_CONTROL;
    m_diafBrowser.dwNumActions    = NUMBER_OF_BROWSERACTIONS;
    m_diafBrowser.rgoAction       = g_rgBrowserAction;
    m_diafBrowser.lAxisMin        = -10;
    m_diafBrowser.lAxisMax        = 10;
    m_diafBrowser.dwBufferSize    = 16;
    _tcscpy( m_diafBrowser.tszActionMap, _T("Donuts New") );

    // Create a new input device manager
    m_pInputDeviceManager = new CInputDeviceManager();

    if( FAILED( hr = m_pInputDeviceManager->Create( hWnd, NULL, m_diafGame,
                                                    StaticInputAddDeviceCB, this ) ) )
    {
        CleanupAndDisplayError( DONUTS3DERR_NOINPUT );
        return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StaticInputAddDeviceCB()
// Desc: Static callback helper to call into CMyApplication class
//-----------------------------------------------------------------------------
HRESULT CALLBACK CMyApplication::StaticInputAddDeviceCB( 
                                         CInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                         const DIDEVICEINSTANCE* pdidi, 
                                         LPVOID pParam )
{
    CMyApplication* pApp = (CMyApplication*) pParam;
    return pApp->InputAddDeviceCB( pDeviceInfo, pdidi );
}




//-----------------------------------------------------------------------------
// Name: InputAddDeviceCB()
// Desc: Called from CInputDeviceManager whenever a device is added. 
//       Set the dead zone, and creates a new InputDeviceState for each device
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                                   const DIDEVICEINSTANCE* pdidi )
{
    // Setup the deadzone 
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = 500;
    pDeviceInfo->pdidDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph );
    
    if( GET_DIDEVICE_TYPE(pdidi->dwDevType) == DI8DEVTYPE_MOUSE )
        pDeviceInfo->pdidDevice->SetCooperativeLevel( m_hWndMain, DISCL_EXCLUSIVE|DISCL_FOREGROUND );

    // Create a new InputDeviceState for each device so the 
    // app can record its state 
    InputDeviceState* pNewInputDeviceState = new InputDeviceState;
    ZeroMemory( pNewInputDeviceState, sizeof(InputDeviceState) );
    pDeviceInfo->pParam = (LPVOID) pNewInputDeviceState;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroyInputObjects()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::DestroyInputObjects()
{
    if( m_pInputDeviceManager )
    {
        // Get access to the list of semantically-mapped input devices
        // to delete all InputDeviceState structs
        CInputDeviceManager::DeviceInfo* pDeviceInfos;
        DWORD dwNumDevices;
        m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );
    
        for( DWORD i=0; i<dwNumDevices; i++ )
        {
            InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
            SAFE_DELETE( pInputDeviceState );
            pDeviceInfos[i].pParam = NULL;
        }
    
        // Delete input device manager
        SAFE_DELETE( m_pInputDeviceManager );
    }
}





//-----------------------------------------------------------------------------
// Name: StaticConfigureInputDevicesCB()
// Desc: Static callback helper to call into CMyD3DApplication class
//-----------------------------------------------------------------------------
BOOL CALLBACK CMyApplication::StaticConfigureInputDevicesCB( IUnknown* pUnknown, VOID* pUserData )
{
    CMyApplication* pApp = (CMyApplication*) pUserData;
    return pApp->ConfigureInputDevicesCB( pUnknown );
}



//-----------------------------------------------------------------------------
// Name: ConfigureInputDevicesCB()
// Desc: Callback function for configuring input devices. This function is
//       called in fullscreen modes, so that the input device configuration
//       window can update the screen.
//-----------------------------------------------------------------------------
BOOL CMyApplication::ConfigureInputDevicesCB( IUnknown* pUnknown )
{
    if( m_dwAppState != APPSTATE_ACTIVE )
        return TRUE;

    // Get access to the surface
    LPDIRECT3DSURFACE8 pConfigSurface;
    if( FAILED( pUnknown->QueryInterface( IID_IDirect3DSurface8,
                                          (VOID**)&pConfigSurface ) ) )
        return TRUE;

    // Draw the scene, with the config surface blitted on top
    DrawDisplayList();

    RECT  rcSrc;
    SetRect( &rcSrc, 0, 0, 640, 480 );

    POINT ptDst;
    ptDst.x = (m_dwScreenWidth-640)/2;
    ptDst.y = (m_dwScreenHeight-480)/2;

    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    m_pd3dDevice->CopyRects( pConfigSurface, &rcSrc, 1, pBackBuffer, &ptDst );
    pBackBuffer->Release();

    ShowFrame();

    // Release the surface
    pConfigSurface->Release();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetInput()
// Desc: Processes data from the input device.  Uses GetDeviceState().
//-----------------------------------------------------------------------------
void CMyApplication::UpdateInput( UserInput* pUserInput )
{
    if( NULL == m_pInputDeviceManager )
        return;

    // Get access to the list of semantically-mapped input devices
    CInputDeviceManager::DeviceInfo* pDeviceInfos;
    DWORD dwNumDevices;
    m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

    // Loop through all devices and check game input
    for( DWORD i=0; i<dwNumDevices; i++ )
    {
        DIDEVICEOBJECTDATA rgdod[10];
        DWORD   dwItems = 10;
        HRESULT hr;
        LPDIRECTINPUTDEVICE8 pdidDevice = pDeviceInfos[i].pdidDevice;
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;

        hr = pdidDevice->Acquire();
        hr = pdidDevice->Poll();
        hr = pdidDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                        rgdod, &dwItems, 0 );
        if( FAILED(hr) )
            continue;

        // Get the sematics codes for the game menu
        for( DWORD j=0; j<dwItems; j++ )
        {
            BOOL  bButtonState = (rgdod[j].dwData==0x80) ? TRUE : FALSE;
            FLOAT fButtonState = (rgdod[j].dwData==0x80) ? 1.0f : 0.0f;
            FLOAT fAxisState   = (FLOAT)((int)rgdod[j].dwData)/10.0f;

            switch( rgdod[j].uAppData )
            {
                // Handle semantics for normal game play

                // Handle relative axis data
                case INPUT_AXIS_LR: 
                    pInputDeviceState->fAxisRotateLR = fAxisState;
                    break;
                case INPUT_AXIS_UD: 
                    pInputDeviceState->fAxisMoveUD   = -fAxisState;
                    break;

                // Handle mouse data                
                case INPUT_MOUSE_LR:
                    if( fAxisState > -0.4f && fAxisState < 0.4f )
                        pInputDeviceState->fAxisRotateLR = 0.0f;
                    else
                        pInputDeviceState->fAxisRotateLR =fAxisState;
                    break;
                case INPUT_MOUSE_UD:
                    if( fAxisState > -0.4f && fAxisState < 0.4f )
                        pInputDeviceState->fAxisMoveUD   = 0.0f;
                    else
                        pInputDeviceState->fAxisMoveUD = -fAxisState;
                    break;

                // Handle buttons separately so the button state data
                // doesn't overwrite the axis state data, and handle
                // each button separately so they don't overwrite each other
                case INPUT_TURNLEFT:      pInputDeviceState->bButtonRotateLeft    = bButtonState; break;
                case INPUT_TURNRIGHT:     pInputDeviceState->bButtonRotateRight   = bButtonState; break;
                case INPUT_FORWARDTHRUST: pInputDeviceState->bButtonForwardThrust = bButtonState; break;
                case INPUT_REVERSETHRUST: pInputDeviceState->bButtonReverseThrust = bButtonState; break;
                case INPUT_FIREWEAPONS:   pInputDeviceState->bButtonFireWeapons   = bButtonState; break;

                // Handle one-shot buttons
                case INPUT_MOUSE_SHIPTYPE:
                    SwitchModel();
                    break;

                case INPUT_CHANGESHIPTYPE: 
                    if( bButtonState ) 
                        SwitchModel();
                    break;

                case INPUT_CHANGEVIEW:    
                    if( bButtonState ) 
                        pUserInput->bDoChangeView = TRUE; 
                    break;

                case INPUT_CHANGEWEAPONS: 
                    if( bButtonState )
                    {
                        if( ++m_dwBulletType > 3 )
                            m_dwBulletType = 0L;
                    }
                    break;

                case INPUT_START:          
                    if( bButtonState ) 
                        m_bPaused = !m_bPaused; 
                    break;

                case INPUT_DISPLAYGAMEMENU:
                    if( bButtonState )
                    {
                        PlaySound( m_pSphereExplodeSound );
                        m_pCurrentMenu = m_pMainMenu;
                        m_pInputDeviceManager->SetActionFormat( m_diafBrowser, FALSE );
                    }
                    break;

                case INPUT_QUITGAME:
                    if( bButtonState )
                    {
                        PlaySound( m_pSphereExplodeSound );
                        m_pCurrentMenu = m_pQuitMenu;
                        m_pInputDeviceManager->SetActionFormat( m_diafBrowser, FALSE );
                    }
                    break;

                // Handle semantics for the game menu
                case INPUT_MENU_UD:     pInputDeviceState->fAxisMenuUD = -fAxisState; break;
                case INPUT_MENU_UP:     if( bButtonState ) pUserInput->bDoMenuUp     = TRUE; break;
                case INPUT_MENU_DOWN:   if( bButtonState ) pUserInput->bDoMenuDown   = TRUE; break;
                case INPUT_MENU_SELECT: if( bButtonState ) pUserInput->bDoMenuSelect = TRUE; break;
                case INPUT_MENU_QUIT:   if( bButtonState ) pUserInput->bDoMenuQuit   = TRUE; break;
                case INPUT_MENU_WHEEL:
                    if( fAxisState > 0.0f )
                        pUserInput->bDoMenuUp = TRUE; 
                    else
                        pUserInput->bDoMenuDown = TRUE; 
                    break;
            }
        }
    }

    pUserInput->bButtonFireWeapons = FALSE; 
    pUserInput->fAxisRotateLR = 0.0f;
    pUserInput->fAxisMoveUD   = 0.0f;

    if( m_pShip->bVisible  )
    {
        // Accumulate thrust inputs

        // Concatinate the data from all the DirectInput devices
        for( i=0; i<dwNumDevices; i++ )
        {
            InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
    
            // Use the axis data that is furthest from zero
            if( fabs(pInputDeviceState->fAxisRotateLR) > fabs(pUserInput->fAxisRotateLR) )
                pUserInput->fAxisRotateLR = pInputDeviceState->fAxisRotateLR;

            if( fabs(pInputDeviceState->fAxisMoveUD) > fabs(pUserInput->fAxisMoveUD) )
                pUserInput->fAxisMoveUD = pInputDeviceState->fAxisMoveUD;

            // Process the button data 
            if( pInputDeviceState->bButtonRotateLeft )
                pUserInput->fAxisRotateLR = -1.0f;
            else if( pInputDeviceState->bButtonRotateRight )
                pUserInput->fAxisRotateLR = 1.0f;

            if( pInputDeviceState->bButtonForwardThrust )
                pUserInput->fAxisMoveUD = 1.0f;
            else if( pInputDeviceState->bButtonReverseThrust )
                pUserInput->fAxisMoveUD = -1.0f;

            if( pInputDeviceState->bButtonFireWeapons )
                pUserInput->bButtonFireWeapons = TRUE;
        }
    }
}




//-----------------------------------------------------------------------------
// Error handling
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: CleanupAndDisplayError()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyApplication::CleanupAndDisplayError( DWORD dwError )
{
    TCHAR* strDbgOut;
    TCHAR* strMsgBox;

    // Cleanup the app
    FinalCleanup();

    // Make the cursor visible
    SetCursor( LoadCursor( NULL, IDC_ARROW ) );
    m_bMouseVisible = TRUE;

    // Get the appropriate error strings
    switch( dwError )
    {
        case DONUTS3DERR_NODIRECT3D:
            strDbgOut = _T("Could not create Direct3D\n");
            strMsgBox = _T("Could not create Direct3D.\n\n")
                        _T("Please make sure you have the latest DirectX\n")
                        _T(".dlls installed on your system.");
            break;
        case DONUTS3DERR_NOD3DDEVICE:
            strDbgOut = _T("Could not create a Direct3D device\n");
            strMsgBox = _T("Could not create a Direct3D device. Your\n")
                        _T("graphics accelerator is not sufficient to\n")
                        _T("run this demo, or your desktop is using\n")
                        _T("a color format that cannot be accelerated by\n")
                        _T("your graphics card (try 16-bit mode).");
            break;
        case DONUTS3DERR_NOTEXTURES:
            strDbgOut = _T("Could not load textures\n");
            strMsgBox = _T("Couldn't load game textures.\n\n")
                        _T("Either your graphics hardware does not have\n")
                        _T("sufficient resources, or the DirectX SDK was\n")
                        _T("not properly installed.");
            break;
        case DONUTS3DERR_NOGEOMETRY:
            strDbgOut = _T("Could not load .x models\n");
            strMsgBox = _T("Couldn't load game geometry.\n\n")
                        _T("Either your graphics hardware does not have\n")
                        _T("sufficient resources, or the DirectX SDK was\n")
                        _T("not properly installed.");
            break;
        case DONUTS3DERR_NO3DRESOURCES:
            strDbgOut = _T("Couldn't load create a d3d object\n");
            strMsgBox = _T("Couldn't create display objects.\n")
                        _T("Yourr graphics hardware does not have\n")
                        _T("sufficient resources to run this app.");
            break;
        case DONUTS3DERR_NOINPUT:
            strDbgOut = _T("Could not create input objects\n");
            strMsgBox = _T("Could not create input objects.");
            break;
    }

    // Output the error strings
    OutputDebugString( strDbgOut );
    MessageBox( m_hWndMain, strMsgBox, _T("Donuts3D"), MB_OK );
}


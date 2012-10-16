//-----------------------------------------------------------------------------
// File: MFCTex.cpp
//
// Desc: Main file for the D3D multitexture app that uses MFC.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <mmsystem.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "MFCTex.h"




//-----------------------------------------------------------------------------
// The MFC macros are all listed here
//-----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE( CAppDoc,      CDocument )
IMPLEMENT_DYNCREATE( CAppFrameWnd, CFrameWnd )
IMPLEMENT_DYNCREATE( CAppForm,     CFormView )


BEGIN_MESSAGE_MAP( CApp, CWinApp )
    //{{AFX_MSG_MAP(CApp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP( CAppForm, CFormView )
    //{{AFX_MSG_MAP(CAppForm)
    ON_COMMAND( IDC_VIEWFULLSCREEN, OnToggleFullScreen )
    ON_COMMAND( IDC_VIEWCODE,       OnViewCode )
    ON_COMMAND( IDM_CHANGEDEVICE,   OnChangeDevice )
    ON_BN_CLICKED( IDM_CHANGEDEVICE,  OnChangeDevice )
    ON_EN_CHANGE(  IDC_TEX0_NAME,     OnChangeTex )
    ON_EN_CHANGE(  IDC_TEX1_NAME,     OnChangeTex )
    ON_EN_CHANGE(  IDC_TEX2_NAME,     OnChangeTex )
    ON_BN_CLICKED( IDC_SELECTTEX0,    OnSelectTexture0Name )
    ON_BN_CLICKED( IDC_SELECTTEX1,    OnSelectTexture1Name )
    ON_BN_CLICKED( IDC_SELECTTEX2,    OnSelectTexture2Name )
    ON_EN_CHANGE(  IDC_BLEND_FACTOR,  OnChangeBlendFactor )
    ON_EN_CHANGE(  IDC_DIFFUSE_COLOR, OnChangeDiffuseColor )
    ON_CBN_SELCHANGE( IDC_PRESET_EFFECTS, OnChangePresetEffects )
    ON_CBN_SELCHANGE( IDC_TEX0_COLORARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX0_COLOROP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX0_COLORARG2, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX0_ALPHAARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX0_ALPHAOP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX0_ALPHAARG2, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_COLORARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_COLOROP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_COLORARG2, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_ALPHAARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_ALPHAOP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX1_ALPHAARG2, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_COLORARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_COLOROP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_COLORARG2, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_ALPHAARG1, OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_ALPHAOP,   OnChangeStageArgs )
    ON_CBN_SELCHANGE( IDC_TEX2_ALPHAARG2, OnChangeStageArgs )
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()




BEGIN_MESSAGE_MAP(CAppDoc, CDocument)
	//{{AFX_MSG_MAP(CAppDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




BEGIN_MESSAGE_MAP(CAppFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CAppFrameWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




//-----------------------------------------------------------------------------
// Global data and objects
//-----------------------------------------------------------------------------
CApp          g_App;
CAppForm*     g_AppFormView = NULL;
TCHAR*        g_strAppTitle = _T("MFCTex: MFC Multitexture Sample");




//-----------------------------------------------------------------------------
// Name: FullScreenWndProc()
// Desc: The WndProc funtion used when the app is in fullscreen mode. This is
//       needed simply to trap the ESC key.
//-----------------------------------------------------------------------------
LRESULT CALLBACK FullScreenWndProc( HWND hWnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam )
{
    if( msg == WM_CLOSE )
    {
        // User wants to exit, so go back to windowed mode and exit for real
        g_AppFormView->OnToggleFullScreen();
        g_App.GetMainWnd()->PostMessage( WM_CLOSE, 0, 0 );
    }

    if( msg == WM_SETCURSOR )
    {
        SetCursor( NULL );
    }

    if( msg == WM_KEYUP && wParam == VK_ESCAPE )
    {
        // User wants to leave fullscreen mode
        g_AppFormView->OnToggleFullScreen();
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: CheckForLostFullscreen()
// Desc: If fullscreen and device was lost (probably due to alt-tab), 
//       automatically switch to windowed mode
//-----------------------------------------------------------------------------
HRESULT CAppForm::CheckForLostFullscreen()
{
    HRESULT hr;

    if( m_bWindowed )
        return S_OK;

    if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
        ForceWindowed();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PreCreateWindow()
// Desc: Change the window style (so it cannot maximize or be sized) before
//       the main frame window is created.
//-----------------------------------------------------------------------------
BOOL CAppFrameWnd::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
    return CFrameWnd::PreCreateWindow( cs );
}




//-----------------------------------------------------------------------------
// Name: InitInstance()
// Desc: This is the main entry point for the application. The MFC window stuff
//       is initialized here. See also the main initialization routine for the
//       CAppForm class, which is called indirectly from here.
//-----------------------------------------------------------------------------
BOOL CApp::InitInstance()
{
    // Asscociate the MFC app with the frame window and doc/view classes
    AddDocTemplate( new CSingleDocTemplate( IDR_MAINFRAME, 
                                            RUNTIME_CLASS(CAppDoc),
                                            RUNTIME_CLASS(CAppFrameWnd),
                                            RUNTIME_CLASS(CAppForm) ) );

    // Dispatch commands specified on the command line (req'd by MFC). This
    // also initializes the the CAppDoc, CAppFrameWnd, and CAppForm classes.
    CCommandLineInfo cmdInfo;
    ParseCommandLine( cmdInfo );
    if( !ProcessShellCommand( cmdInfo ) )
        return FALSE;

    if( !g_AppFormView->IsReady() )
        return FALSE;

    g_AppFormView->GetParentFrame()->RecalcLayout();
    g_AppFormView->ResizeParentToFit( FALSE ); 
    
    m_pMainWnd->SetWindowText( g_strAppTitle );
    m_pMainWnd->UpdateWindow();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: OnIdle()
// Desc: Uses idle time to render the 3D scene.
//-----------------------------------------------------------------------------
BOOL CApp::OnIdle( LONG )
{
    // Do not render if the app is minimized
    if( m_pMainWnd->IsIconic() )
        return FALSE;

    // Update and render a frame
    if( g_AppFormView->IsReady() )
    {
        g_AppFormView->CheckForLostFullscreen();
        g_AppFormView->RenderScene();
    }

    // Keep requesting more idle time
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: CAppForm()
// Desc: Constructor for the dialog resource form
//-----------------------------------------------------------------------------
CAppForm::CAppForm()
         :CFormView( IDD_FORMVIEW )
{
    g_AppFormView       = this;
    m_bActive           = FALSE;
    m_bReady            = FALSE;
    m_bWindowed         = TRUE;
    
    m_dwAdapter         = 0L;
	m_pD3D              = NULL;
    m_pd3dDevice        = NULL;
    m_bUseDepthBuffer   = TRUE;
}




//-----------------------------------------------------------------------------
// Name: ~CAppForm()
// Desc: Destructor for the dialog resource form. Shuts down the app
//-----------------------------------------------------------------------------
CAppForm::~CAppForm()
{
    Cleanup3DEnvironment();
}




//-----------------------------------------------------------------------------
// Name: OnInitialUpdate()
// Desc: When the AppForm object is created, this function is called to
//       initialize it. Here we getting access ptrs to some of the controls,
//       and setting the initial state of some of them as well.
//-----------------------------------------------------------------------------
VOID CAppForm::OnInitialUpdate()
{
	// Update the UI
    CFormView::OnInitialUpdate();
	InitializeUIControls();

    // Save static reference to the render window
    m_hwndRenderWindow = GetDlgItem(IDC_RENDERVIEW)->GetSafeHwnd();

    // Register a class for a fullscreen window
    WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, FullScreenWndProc, 0, 0, NULL,
                          NULL, NULL, (HBRUSH)GetStockObject(WHITE_BRUSH), NULL,
                          _T("Fullscreen Window") };
    RegisterClass( &wndClass );

    // We create the fullscreen window (not visible) at startup, so it can
    // be the focus window.  The focus window can only be set at CreateDevice
    // time, not in a Reset, so ToggleFullscreen wouldn't work unless we have
    // already set up the fullscreen focus window.
    m_hwndRenderFullScreen = CreateWindow( _T("Fullscreen Window"), NULL,
                                           WS_POPUP, CW_USEDEFAULT,
                                           CW_USEDEFAULT, 100, 100,
                                           GetTopLevelParent()->GetSafeHwnd(), 0L, NULL, 0L );

    // Note that for the MFC samples, the device window and focus window
    // are not the same.
    CD3DApplication::m_hWnd = m_hwndRenderWindow;
    CD3DApplication::m_hWndFocus = m_hwndRenderFullScreen;
    CD3DApplication::Create( AfxGetInstanceHandle() );
}




//-----------------------------------------------------------------------------
// Name: OnChangeDevice()
// Desc: Use hit the "Change Device.." button. Display the dialog for the user
//       to select a new device/mode, and call Change3DEnvironment to
//       use the new device/mode.
//-----------------------------------------------------------------------------
VOID CAppForm::OnChangeDevice()
{
    UserSelectNewDevice();
}




//-----------------------------------------------------------------------------
// Name: AdjustWindowForChange()
// Desc: Adjusts the window properties for windowed or fullscreen mode
//-----------------------------------------------------------------------------
HRESULT CAppForm::AdjustWindowForChange()
{
    if( m_bWindowed )
    {
        ::ShowWindow( m_hwndRenderFullScreen, SW_HIDE );
        CD3DApplication::m_hWnd = m_hwndRenderWindow;
    }
    else
    {
        if( ::IsIconic( m_hwndRenderFullScreen ) )
            ::ShowWindow( m_hwndRenderFullScreen, SW_RESTORE );
        ::ShowWindow( m_hwndRenderFullScreen, SW_SHOW );
        CD3DApplication::m_hWnd = m_hwndRenderFullScreen;
    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnToggleFullScreen()
// Desc: Called when user toggles the fullscreen mode
//-----------------------------------------------------------------------------
void CAppForm::OnToggleFullScreen()
{
    ToggleFullscreen();
}





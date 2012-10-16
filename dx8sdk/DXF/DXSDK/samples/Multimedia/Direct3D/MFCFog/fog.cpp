//-----------------------------------------------------------------------------
// File: Fog.cpp
//
// Desc: Example code showing how to do fog in D3D
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include "stdafx.h"
#include <tchar.h>
#include <math.h>
#include <D3DX8.h>
#include "resource.h"
#include "D3DApp.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "fog.h"




//-----------------------------------------------------------------------------
// Structures and Macros
//-----------------------------------------------------------------------------
inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

struct FOGVERTEX
{
        D3DXVECTOR3 p;
        D3DXVECTOR3 n;
        FLOAT       tu, tv;
};

#define D3DFVF_FOGVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

#define FAR_PLANE (150.0f)
#define NEAR_PLANE (1.0f)




//-----------------------------------------------------------------------------
// Application globals
//-----------------------------------------------------------------------------
TCHAR*        g_strAppTitle       = _T( "MFCFog: D3D Fog Sample Using MFC" );
CApp          g_App;
CAppForm*     g_AppFormView = NULL;




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
    ON_COMMAND(    IDC_VIEWFULLSCREEN, OnToggleFullScreen )
    ON_COMMAND(    IDM_CHANGEDEVICE,   OnChangeDevice )
    ON_WM_HSCROLL()
    ON_BN_CLICKED( IDC_FOGCOLOR,       OnFogColor )
    ON_BN_CLICKED( IDC_RANGEBASEDFOG,  OnRangeBasedFog )
    ON_BN_CLICKED( IDC_VERTEXFOG,      OnVertexFog )
    ON_BN_CLICKED( IDC_TABLEFOG,       OnTableFog )
    ON_BN_CLICKED( IDC_LINEARFOGMODE,  OnFogMode )
    ON_BN_CLICKED( IDC_LORESTERRAIN,   OnTerrainResolution)
    ON_BN_CLICKED( IDM_CHANGEDEVICE,   OnChangeDevice )
    ON_BN_CLICKED( IDC_EXPFOGMODE,     OnFogMode )
    ON_BN_CLICKED( IDC_EXP2FOGMODE,    OnFogMode )
    ON_BN_CLICKED( IDC_HIRESTERRAIN,   OnTerrainResolution)
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

    TCHAR strStatsPrev[200];

    lstrcpy(strStatsPrev, g_AppFormView->PstrFrameStats());

    // Update and render a frame
    if( g_AppFormView->IsReady() )
    {
        g_AppFormView->CheckForLostFullscreen();
        g_AppFormView->RenderScene();
        if (lstrcmp(strStatsPrev, g_AppFormView->PstrFrameStats()) != 0)
            g_AppFormView->GetDlgItem(IDC_FPS_TEXT)->SetWindowText(g_AppFormView->PstrFrameStats());
    }

    // Keep requesting more idle time
    return TRUE;
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
// Name: CAppForm()
// Desc: Constructor for the dialog resource form
//-----------------------------------------------------------------------------
CAppForm::CAppForm()
         :CFormView( IDD_FORMVIEW )
{
    g_AppFormView          = this;
    m_bHiResTerrain        = FALSE;
    m_bHiResTerrainOld     = FALSE;
    m_dwFogColor           = 0x00b5b5ff;
    m_dwFogMode            = D3DFOG_LINEAR;
    m_bCanDoTableFog       = FALSE;
    m_bCanDoVertexFog      = FALSE;
    m_bCanDoWFog           = FALSE;
    m_bDeviceUsesWFog      = FALSE;
    m_bRangeBasedFog       = FALSE;
    m_bUsingTableFog       = FALSE;
    m_fFogStartSlider      = 0.0f;
    m_fFogEndSlider        = 1.0f;
    m_fFogStartValue       = 0.0f;
    m_fFogEndValue         = 1.0f;
    m_fFogDensity          = 0.0f;
    m_hwndRenderWindow     = NULL;
    m_hwndRenderFullScreen = NULL;
    m_pFloorTexture        = NULL;
    m_pTerrainVB           = NULL;
    m_dwNumTerrainVertices = 0L;
    m_pColumnVB            = NULL;
    m_dwNumColumnVertices  = 0L;

    // Override some CD3DApplication defaults:
    m_bUseDepthBuffer      = TRUE;
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
// Name: OnToggleFullScreen()
// Desc: Called when user toggles the fullscreen mode
//-----------------------------------------------------------------------------
void CAppForm::OnToggleFullScreen()
{
    ToggleFullscreen();
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

    // Update UI, and device's fog parameters
    UpdateUIForDeviceCapabilites();
    SetFogParameters();
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

    else if( msg == WM_SETCURSOR )
    {
        SetCursor( NULL );
    }

    else if( msg == WM_KEYUP && wParam == VK_ESCAPE )
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
// Name: OnTerrainResolution()
// Desc: Called when the user selects the terrain resolution
//-----------------------------------------------------------------------------
VOID CAppForm::OnTerrainResolution()
{
    m_bHiResTerrain = ((CButton*)GetDlgItem(IDC_HIRESTERRAIN))->GetCheck();

    SetFogParameters();
}




//-----------------------------------------------------------------------------
// Name: OnHScroll()
// Desc: Called when the user moves any scroll bar. Check which scrollbar was
//       moved, and extract the appropiate value to a global variable.
//-----------------------------------------------------------------------------
void CAppForm::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // Get the new fog parameters
    m_fFogStartSlider = ((CSliderCtrl*)GetDlgItem(IDC_FOGSTART_SLIDER))->GetPos()/100.0f;
    m_fFogEndSlider   = ((CSliderCtrl*)GetDlgItem(IDC_FOGEND_SLIDER))->GetPos()/100.0f;
    m_fFogDensity     = ((CSliderCtrl*)GetDlgItem(IDC_FOGDENSITY_SLIDER))->GetPos()/100.0f;

    if( m_fFogEndSlider < m_fFogStartSlider )
    {
        ((CSliderCtrl*)GetDlgItem(IDC_FOGEND_SLIDER))->SetPos((INT)(m_fFogStartSlider*100.0f));
        m_fFogEndSlider = m_fFogStartSlider;
    }
    
    SetFogParameters();

    CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}




//-----------------------------------------------------------------------------
// Name: OnFogColor()
// Desc: Called when the user hits the "fog color..." button. Display a color
//       selection dialog box, and set the global fog color variable.
//-----------------------------------------------------------------------------
void CAppForm::OnFogColor()
{
    CColorDialog dlg;

    if( IDOK == dlg.DoModal() )
    {
        m_dwFogColor = ((((DWORD)dlg.GetColor())&0x000000ff)<<16) +
                       ((((DWORD)dlg.GetColor())&0x0000ff00)) +
                       ((((DWORD)dlg.GetColor())&0x00ff0000)>>16);
        SetFogParameters();
    }
}




//-----------------------------------------------------------------------------
// Name: OnRangeBasedFog()
// Desc: Toggle the boolean variable for whether RangeBasedFog is enabled.
//-----------------------------------------------------------------------------
void CAppForm::OnRangeBasedFog()
{
    m_bRangeBasedFog = ((CButton*)GetDlgItem(IDC_RANGEBASEDFOG))->GetCheck();

    SetFogParameters();
}




//-----------------------------------------------------------------------------
// Name: OnVertexFog()
// Desc: User selected vertex fog. Upadte the global variables as appropiate.
//-----------------------------------------------------------------------------
void CAppForm::OnVertexFog()
{
    // Note: We always assume range fog is available if doing vertex fog
    GetDlgItem(IDC_RANGEBASEDFOG)->EnableWindow(TRUE);

    GetDlgItem(IDC_LINEARFOGMODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXPFOGMODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_EXP2FOGMODE)->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_LINEARFOGMODE))->SetCheck(TRUE);
    ((CButton*)GetDlgItem(IDC_EXPFOGMODE))->SetCheck(FALSE);
    ((CButton*)GetDlgItem(IDC_EXP2FOGMODE))->SetCheck(FALSE);

    GetDlgItem(IDC_FOGSTARTMIN_TEXT)->SetWindowText( _T("near (1.0)") );
    GetDlgItem(IDC_FOGSTARTMAX_TEXT)->SetWindowText( _T("far (150.0)") );
    GetDlgItem(IDC_FOGENDMIN_TEXT)->SetWindowText( _T("near (1.0)") );
    GetDlgItem(IDC_FOGENDMAX_TEXT)->SetWindowText( _T("far (150.0)") );

    m_bUsingTableFog = FALSE;
    OnFogMode();
}




//-----------------------------------------------------------------------------
// Name: OnTableFog()
// Desc: User selected table fog. Upadte the global variables as appropiate.
//-----------------------------------------------------------------------------
void CAppForm::OnTableFog()
{
    // Note: We only assume range fog is available if doing vertex fog
    GetDlgItem(IDC_RANGEBASEDFOG)->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_RANGEBASEDFOG))->SetCheck(FALSE);

    GetDlgItem(IDC_LINEARFOGMODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXPFOGMODE)->EnableWindow(TRUE);
    GetDlgItem(IDC_EXP2FOGMODE)->EnableWindow(TRUE);

    if( m_bCanDoWFog )
    {
        GetDlgItem(IDC_FOGSTARTMIN_TEXT)->SetWindowText( _T("near (1.0)") );
        GetDlgItem(IDC_FOGSTARTMAX_TEXT)->SetWindowText( _T("far (150.0)") );
        GetDlgItem(IDC_FOGENDMIN_TEXT)->SetWindowText( _T("near (1.0)") );
        GetDlgItem(IDC_FOGENDMAX_TEXT)->SetWindowText( _T("far (150.0)") );
    }
    else
    {
        GetDlgItem(IDC_FOGSTARTMIN_TEXT)->SetWindowText( _T("near (0.0)") );
        GetDlgItem(IDC_FOGSTARTMAX_TEXT)->SetWindowText( _T("far (1.0)") );
        GetDlgItem(IDC_FOGENDMIN_TEXT)->SetWindowText( _T("near (0.0)") );
        GetDlgItem(IDC_FOGENDMAX_TEXT)->SetWindowText( _T("far (1.0)") );
    }

    m_bUsingTableFog = TRUE;
    OnFogMode();
}




//-----------------------------------------------------------------------------
// Name: OnFogMode()
// Desc: User changed the fog mode. Update the UI and global variables, as many
//       controls are mutually exclusive.
//-----------------------------------------------------------------------------
void CAppForm::OnFogMode()
{
    if( ((CButton*)GetDlgItem(IDC_LINEARFOGMODE))->GetCheck() )
    {
        m_dwFogMode = D3DFOG_LINEAR;

        GetDlgItem(IDC_FOGSTART_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGSTARTMIN_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGSTARTMAX_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGSTART_SLIDER)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGEND_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGENDMIN_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGENDMAX_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGEND_SLIDER)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGDENSITY_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGDENSITYMIN_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGDENSITYMAX_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGDENSITY_SLIDER)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_FOGSTART_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGSTARTMIN_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGSTARTMAX_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGSTART_SLIDER)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGEND_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGENDMIN_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGENDMAX_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGEND_SLIDER)->EnableWindow(FALSE);
        GetDlgItem(IDC_FOGDENSITY_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGDENSITYMIN_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGDENSITYMAX_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_FOGDENSITY_SLIDER)->EnableWindow(TRUE);

        if( ((CButton*)GetDlgItem(IDC_EXPFOGMODE))->GetCheck() )
            m_dwFogMode = D3DFOG_EXP;
        if( ((CButton*)GetDlgItem(IDC_EXP2FOGMODE))->GetCheck() )
            m_dwFogMode = D3DFOG_EXP2;
    }

    SetFogParameters();
}




//-----------------------------------------------------------------------------
// Name: UpdateUIForDeviceCapabilites()
// Desc: Whenever we get a new device, call this function to enable/disable the
//       appropiate UI controls to match the device's capabilities.
//-----------------------------------------------------------------------------
VOID CAppForm::UpdateUIForDeviceCapabilites()
{
    // Check the capabilities of the device
    DWORD dwCaps = m_d3dCaps.RasterCaps;
    m_bCanDoTableFog  = (dwCaps&D3DPRASTERCAPS_FOGTABLE) &&
                        ((dwCaps&D3DPRASTERCAPS_ZFOG) || (dwCaps&D3DPRASTERCAPS_WFOG))    
                                                          ? TRUE : FALSE;
    m_bCanDoVertexFog = (dwCaps&D3DPRASTERCAPS_FOGVERTEX) ? TRUE : FALSE;
    m_bCanDoWFog      = (dwCaps&D3DPRASTERCAPS_WFOG)      ? TRUE : FALSE;

    // Update the UI checkbox states
    ((CButton*)GetDlgItem(IDC_TABLEFOG))->EnableWindow(m_bCanDoTableFog);
    ((CButton*)GetDlgItem(IDC_VERTEXFOG))->EnableWindow(m_bCanDoVertexFog);

    if( m_bCanDoWFog )
        GetDlgItem(IDC_USINGWFOG)->SetWindowText( _T("Device using W-fog") );
    else
        GetDlgItem(IDC_USINGWFOG)->SetWindowText( _T("Device using Z-fog") );

    if( m_bUsingTableFog && m_bCanDoTableFog )
    {
        ((CButton*)GetDlgItem(IDC_VERTEXFOG))->SetCheck(FALSE);
        ((CButton*)GetDlgItem(IDC_TABLEFOG))->SetCheck(TRUE);
    }
    else if( m_bCanDoVertexFog )
    {
        ((CButton*)GetDlgItem(IDC_VERTEXFOG))->SetCheck(TRUE);
        ((CButton*)GetDlgItem(IDC_TABLEFOG))->SetCheck(FALSE);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_VERTEXFOG))->SetCheck(FALSE);
        ((CButton*)GetDlgItem(IDC_TABLEFOG))->SetCheck(m_bCanDoTableFog);
    }

    // Set up table or vertex mode, as appropiate
    if( ((CButton*)GetDlgItem(IDC_TABLEFOG))->GetCheck() )
        OnTableFog();
    if( ((CButton*)GetDlgItem(IDC_VERTEXFOG))->GetCheck() )
        OnVertexFog();
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
    ((CSliderCtrl*)GetDlgItem( IDC_FOGSTART_SLIDER ))->SetRange(0,100,TRUE);
    ((CSliderCtrl*)GetDlgItem( IDC_FOGSTART_SLIDER ))->SetPos(0);
    ((CSliderCtrl*)GetDlgItem( IDC_FOGEND_SLIDER ))->SetRange(0,100,TRUE);
    ((CSliderCtrl*)GetDlgItem( IDC_FOGEND_SLIDER ))->SetPos(100);
    ((CSliderCtrl*)GetDlgItem( IDC_FOGDENSITY_SLIDER ))->SetRange(0,100,TRUE);
    ((CSliderCtrl*)GetDlgItem( IDC_FOGDENSITY_SLIDER ))->SetPos(0);
    ((CButton*)GetDlgItem( IDC_LORESTERRAIN ))->SetCheck(TRUE);
    ((CButton*)GetDlgItem(IDC_VERTEXFOG))->SetCheck(TRUE);

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

    // Update UI, and device's fog parameters
    OnVertexFog();
    UpdateUIForDeviceCapabilites();
    SetFogParameters();
}




//----------------------------------------------------------------------------
// Name: GenerateTerrainDisk()
// Desc: Generates a trianglestrip for a disk
//----------------------------------------------------------------------------
HRESULT CAppForm::GenerateTerrainDisk( LPDIRECT3DDEVICE8 pd3dDevice, DWORD dwNumSegments,
                                       FLOAT fScale )
{
    HRESULT hr;

    m_dwNumTerrainVertices = 2 * dwNumSegments * (dwNumSegments);

    // Destroy the old vertex buffer, if any
    SAFE_RELEASE( m_pTerrainVB );

    // Create a vertex buffer
    hr = pd3dDevice->CreateVertexBuffer( m_dwNumTerrainVertices*sizeof(FOGVERTEX),
                                         D3DUSAGE_WRITEONLY, D3DFVF_FOGVERTEX,
                                         D3DPOOL_MANAGED, &m_pTerrainVB );
    if( FAILED(hr) )
        return hr;

    FOGVERTEX* pVertices = NULL;
    hr = m_pTerrainVB->Lock( 0, m_dwNumTerrainVertices*sizeof(FOGVERTEX),
                             (BYTE**)&pVertices, 0 );
    if( FAILED(hr) )
        return hr;

    // Generate a spiralized trianglestrip
    for( DWORD ring = 0; ring < dwNumSegments; ring++ )
    {
        for( DWORD seg=0; seg < dwNumSegments; seg++ )
        {
            FLOAT fTheta = (seg*2*D3DX_PI) / dwNumSegments;
            FLOAT r0     = (ring + fTheta/(2*D3DX_PI))*fScale/dwNumSegments;
            FLOAT r1     = r0 + fScale/dwNumSegments;

            FLOAT x   = (FLOAT)sin( fTheta );
            FLOAT z   = (FLOAT)cos( fTheta );

            FLOAT y0  =  (FLOAT)sin(r0*z*z+r0*x*x);
            FLOAT nx0 = -(FLOAT)cos(r0*z*z+r0*x*x)*r0*2*x;
            FLOAT ny0 = 1.0f;
            FLOAT nz0 = -(FLOAT)cos(r0*z*z+r0*x*x)*r0*2*z;

            FLOAT y1  =  (FLOAT)sin(r1*z*z+r1*x*x);
            FLOAT nx1 = -(FLOAT)cos(r1*z*z+r1*x*x)*r1*2*x;
            FLOAT ny1 = 1.0f;
            FLOAT nz1 = -(FLOAT)cos(r1*z*z+r1*x*x)*r1*2*z;

            // Add two vertices to the strip at each step
            pVertices->p.x = r0*x;
            pVertices->p.y = y0;
            pVertices->p.z = r0*z;
            pVertices->n.x = nx0;
            pVertices->n.y = ny0;
            pVertices->n.z = nz0;
            pVertices->tu  = (r0*x)/fScale;
            pVertices->tv  = (r0*z)/fScale;
            pVertices++;

            pVertices->p.x = r1*x;
            pVertices->p.y = y1;
            pVertices->p.z = r1*z;
            pVertices->n.x = nx1;
            pVertices->n.y = ny1;
            pVertices->n.z = nz1;
            pVertices->tu  = (r1*x)/fScale;
            pVertices->tv  = (r1*z)/fScale;
            pVertices++;
        }
    }

    m_pTerrainVB->Unlock();

    return S_OK;
}




//----------------------------------------------------------------------------
// Name: GenerateColumn()
// Desc: Generates a trianglestrip for a column
//----------------------------------------------------------------------------
HRESULT CAppForm::GenerateColumn( LPDIRECT3DDEVICE8 pd3dDevice, DWORD dwNumSegments,
                                  FLOAT fRadius, FLOAT fHeight )
{
    HRESULT hr;

    m_dwNumColumnVertices = 2 * (dwNumSegments+1);

    // Destroy the old vertex buffer, if any
    SAFE_RELEASE( m_pColumnVB );

    // Create a vertex buffer
    hr = pd3dDevice->CreateVertexBuffer( m_dwNumColumnVertices*sizeof(FOGVERTEX),
                                         D3DUSAGE_WRITEONLY, D3DFVF_FOGVERTEX,
                                         D3DPOOL_MANAGED, &m_pColumnVB );
    if( FAILED(hr) )
        return hr;

    FOGVERTEX* pVertices = NULL;
    hr = m_pColumnVB->Lock( 0, m_dwNumColumnVertices*sizeof(FOGVERTEX),
                             (BYTE**)(&pVertices), 0 );
    if( FAILED(hr) )
        return hr;

    // Generate a trianglestrip
    for( DWORD seg=0; seg<=dwNumSegments; seg++ )
    {
        FLOAT fTheta = (2*D3DX_PI*seg)/dwNumSegments;
        FLOAT nx     = (FLOAT)sin(fTheta);
        FLOAT nz     = (FLOAT)cos(fTheta);
        FLOAT r      = fRadius;
        FLOAT u      = (1.0f*seg)/dwNumSegments;

        // Add two vertices to the strip at each step
        pVertices->p.x = r*nx;
        pVertices->p.y = fHeight;
        pVertices->p.z = r*nz;
        pVertices->n.x = nx;
        pVertices->n.y = 0;
        pVertices->n.z = nz;
        pVertices->tu  = u;
        pVertices->tv  = 1;
        pVertices++;

        pVertices->p.x = r*nx;
        pVertices->p.y = -1;
        pVertices->p.z = r*nz;
        pVertices->n.x = nx;
        pVertices->n.y = 0;
        pVertices->n.z = nz;
        pVertices->tu  = u;
        pVertices->tv  = 0;
        pVertices++;
    }

    m_pColumnVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CAppForm::OneTimeSceneInit()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FrameMove()
{
    // Move the camera along an ellipse
    D3DXVECTOR3 from( 50*sinf(m_fTime/2), 5.0f, 60*cosf(m_fTime/2) );
    D3DXVECTOR3 at( 50*sinf(m_fTime/2+1.5f), 4.0f, 60*cosf(m_fTime/2+1.5f) );
    D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &from, &at, &up );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                       m_dwFogColor, 1.0f, 0L );

    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR,  m_dwFogColor );

    m_pd3dDevice->SetRenderState( D3DRS_FOGSTART,   FtoDW(m_fFogStartValue) );
    m_pd3dDevice->SetRenderState( D3DRS_FOGEND,     FtoDW(m_fFogEndValue) );
    m_pd3dDevice->SetRenderState( D3DRS_FOGDENSITY, FtoDW(m_fFogDensity) );

    if( m_bUsingTableFog )
    {
        m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_NONE );
        m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE,   m_dwFogMode );
    }
    else
    {
        m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
        m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE,  m_dwFogMode );
        m_pd3dDevice->SetRenderState( D3DRS_RANGEFOGENABLE, m_bRangeBasedFog );
    }

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Reset the world matrix
        D3DXMATRIX matWorld;
        D3DXMatrixIdentity( &matWorld );
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

        // Draw the terrain
        m_pd3dDevice->SetVertexShader( D3DFVF_FOGVERTEX );
        m_pd3dDevice->SetStreamSource( 0, m_pTerrainVB, sizeof(FOGVERTEX) );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,
                                   0, m_dwNumTerrainVertices-2 );

        // Draw the columns
        for( DWORD i=0; i<20; i++ )
        {
            FLOAT tx = (i%10)*20.0f - 100.0f;
            FLOAT ty =  0.0f;
            FLOAT tz = (i<=10) ? 40.0f : -40.0f;

            D3DXMatrixTranslation( &matWorld, tx, ty, tz );
            m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

            m_pd3dDevice->SetVertexShader( D3DFVF_FOGVERTEX );
            m_pd3dDevice->SetStreamSource( 0, m_pColumnVB, sizeof(FOGVERTEX) );
            m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,
                                         0, m_dwNumColumnVertices-2 );
        }

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InitDeviceObjects()
{
    // Get the device caps
    D3DCAPS8 d3dCaps;
    m_pd3dDevice->GetDeviceCaps( &d3dCaps );

    if( d3dCaps.RasterCaps & D3DPRASTERCAPS_WFOG )
        m_bDeviceUsesWFog = TRUE;
    else
        m_bDeviceUsesWFog = FALSE;

    // Create the floor texture
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("SeaFloor.bmp"),
                                       &m_pFloorTexture, D3DFMT_R5G6B5 ) ) )
        return E_FAIL;

    // Generate some geometry for the app
    if( m_bHiResTerrain )
        GenerateTerrainDisk( m_pd3dDevice, 80, 100.0f );
    else
        GenerateTerrainDisk( m_pd3dDevice, 5, 100.0f );
    GenerateColumn( m_pd3dDevice, 30, 1.0f, 10.0f );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::RestoreDeviceObjects()
{
    // Set up the object material
    D3DMATERIAL8 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    m_pd3dDevice->SetMaterial( &mtrl );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,  0x44444444 );

    // Set up a texture
    m_pd3dDevice->SetTexture( 0, m_pFloorTexture );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Set the transform matrices
    D3DXMATRIX matWorld, matProj;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4.0f, 1.0f, NEAR_PLANE, FAR_PLANE );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,      &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set up the light
    D3DLIGHT8 light;
    D3DUtil_InitLight( light, D3DLIGHT_POINT, 0.0f, 50.0f, 0.0f );
    light.Attenuation0 =  0.1f;
    light.Range        = 200.0f;
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,    TRUE );

    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,            TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,       TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,     FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InvalidateDeviceObjects()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pColumnVB );
    SAFE_RELEASE( m_pTerrainVB );
    SAFE_RELEASE( m_pFloorTexture );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FinalCleanup()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAppForm::ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CAppForm::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, 
                                 D3DFORMAT Format )
{
    if( pCaps->RasterCaps & D3DPRASTERCAPS_FOGVERTEX )
        return S_OK;

    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: SetFogParameters()
// Desc: Sets the apps parameters for rendering the scene
//-----------------------------------------------------------------------------
VOID CAppForm::SetFogParameters()
{
    m_fFogStartValue = ( m_fFogStartSlider*(FAR_PLANE-NEAR_PLANE) ) + NEAR_PLANE;
    m_fFogEndValue   = ( m_fFogEndSlider*(FAR_PLANE-NEAR_PLANE) ) + NEAR_PLANE;

    // Set fog start and end values for table (pixel) fog mode on devices that
    // do not use WFOG. These devices expect fog between 0.0 and 1.0.
    if( (FALSE==m_bDeviceUsesWFog) && (TRUE==m_bUsingTableFog) )
    {
        m_fFogStartValue = m_fFogStartSlider;
        m_fFogEndValue   = m_fFogEndSlider;
    }

    // Adjust terrain if necessary
    if( m_bHiResTerrainOld != m_bHiResTerrain )
    {
        m_bHiResTerrainOld  = m_bHiResTerrain;

        if( m_bHiResTerrain )
            GenerateTerrainDisk( m_pd3dDevice, 80, 100.0f );
        else
            GenerateTerrainDisk( m_pd3dDevice, 5, 100.0f );
    }
}

//-----------------------------------------------------------------------------
// File: PixelShader.cpp
//
// Desc: Example code showing how to use pixel shaders in D3D
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include "stdafx.h"
#include <tchar.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <D3DX8.h>
#include "resource.h"
#include "D3DApp.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "PixelShader.h"




//-----------------------------------------------------------------------------
// Global data and objects
//-----------------------------------------------------------------------------
CApp          g_App;
CAppForm*     g_AppFormView = NULL;
TCHAR*        g_strAppTitle = _T("MFCPixelShader");




//-----------------------------------------------------------------------------
// The MFC macros are all listed here
//-----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE( CAppDoc,      CDocument )
IMPLEMENT_DYNCREATE( CAppFrameWnd, CFrameWnd )
IMPLEMENT_DYNCREATE( CAppForm,     CFormView )


BEGIN_MESSAGE_MAP( CApp, CWinApp )
    //{{AFX_MSG_MAP(CD3DApp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP( CAppForm, CFormView )
    //{{AFX_MSG_MAP(CAppForm)
    ON_WM_HSCROLL()
    ON_COMMAND(    IDC_VIEWFULLSCREEN,       OnToggleFullScreen )
    ON_COMMAND(    IDM_CHANGEDEVICE,         OnChangeDevice )
    ON_BN_CLICKED( IDC_OPEN,                 OnOpenPixelShaderFile )
    ON_BN_CLICKED( IDC_PRESET_0,             OnPresets )
    ON_BN_CLICKED( IDC_PRESET_1,             OnPresets )
    ON_BN_CLICKED( IDC_PRESET_2,             OnPresets )
    ON_BN_CLICKED( IDC_PRESET_3,             OnPresets )
    ON_BN_CLICKED( IDC_PRESET_4,             OnPresets )
    ON_BN_CLICKED( IDC_PRESET_5,             OnPresets )
    ON_EN_CHANGE(  IDC_INSTRUCTIONS,         OnChangeInstructions )
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
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color1, color2;
    FLOAT tu1, tv1;
    FLOAT tu2, tv2;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX2)

static CUSTOMVERTEX g_Vertices[]=
{
    // Color 0 (diffuse) is a color gradient
    // Color 1 (specular) is a greyscale ramp
    //  x      y     z     diffuse     specular    u1    v1    u2    v2
    { -1.0f, -1.0f, 0.0f, 0xff00ffff, 0xffffffff, 1.0f, 1.0f, 1.0f, 1.0f },
    { +1.0f, -1.0f, 0.0f, 0xffffff00, 0xffffffff, 0.0f, 1.0f, 0.0f, 1.0f },
    { +1.0f, +1.0f, 0.0f, 0xffff0000, 0xff000000, 0.0f, 0.0f, 0.0f, 0.0f },
    { -1.0f, +1.0f, 0.0f, 0xff0000ff, 0xff000000, 1.0f, 0.0f, 1.0f, 0.0f },
};

inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }



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
    g_AppFormView     = this;

    m_pTexture0       = NULL;
    m_pTexture1       = NULL;
    m_pQuadVB         = NULL;
    m_hPixelShader    = NULL;
    m_pD3DXBufShader  = NULL;
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
    CFormView::OnInitialUpdate();

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

    // Set up the form's UI
    ((CButton*)GetDlgItem(IDC_PRESET_0))->SetCheck(TRUE);
    OnPresets();
}




//-----------------------------------------------------------------------------
// Name: SetPixelShader()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CAppForm::SetPixelShader( TCHAR* strOpcodes )
{
    HRESULT      hr;
    LPD3DXBUFFER pBuffer = NULL;

    SAFE_RELEASE( m_pD3DXBufShader );

    // Build a DWORD array of opcodes from the text string
    hr = D3DXAssembleShader( strOpcodes, lstrlen(strOpcodes), 0, NULL, 
        &m_pD3DXBufShader, &pBuffer );
    if( FAILED(hr) )
    {
        if( pBuffer != NULL)
        {
            TCHAR* pstr;
            TCHAR strOut[4096];
            TCHAR* pstrOut;
            // Need to replace \n with \r\n so edit box shows newlines properly
            pstr = (TCHAR*)pBuffer->GetBufferPointer();
            strOut[0] = _T('\0');
            pstrOut = strOut;
            for( int i = 0; i < 4096; i++ )
            {
                if( *pstr == _T('\n') )
                    *pstrOut++ = _T('\r');
                *pstrOut = *pstr;
                if( *pstr == _T('\0') )
                    break;
                if( i == 4095 )
                    *pstrOut = _T('\0');
                pstrOut++;
                pstr++;
            }
            // remove any blank lines at the end
            while( strOut[lstrlen(strOut) - 1] == _T('\n') ||
                   strOut[lstrlen(strOut) - 1] == _T('\r') )
            {
                strOut[lstrlen(strOut) - 1] = _T('\0');
            }
            SetDlgItemText( IDC_COMPRESULT, strOut );
            SAFE_RELEASE( pBuffer );
        }
        return hr;
    }
    else
    {
        SAFE_RELEASE( pBuffer );
        // Delete old pixel shader
        if( m_hPixelShader )
            m_pd3dDevice->DeletePixelShader( m_hPixelShader );
        m_hPixelShader = NULL;

        // Create new pixel shader
        hr = m_pd3dDevice->CreatePixelShader( (DWORD*)m_pD3DXBufShader->GetBufferPointer(),
                                              &m_hPixelShader );

        if( FAILED(hr) )
        {
            ((CStatic*)GetDlgItem(IDC_COMPRESULT))->SetWindowText( _T("Failure (D3D)"));
            return hr;
        }
        else
        {
            ((CStatic*)GetDlgItem(IDC_COMPRESULT))->SetWindowText( _T("Success"));
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CAppForm::OneTimeSceneInit()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FrameMove()
{
    // Move the camera along an ellipse
    D3DXVECTOR3 from( 3*sinf(m_fTime/3), 3*cosf(m_fTime/3), 5.0f );
    D3DXVECTOR3 at( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );

    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &from, &at, &up );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 0.5f, 1000.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CAppForm::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // Set device state
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
        m_pd3dDevice->SetRenderState( D3DRS_CLIPPING, FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  FALSE );

        // Set the textures
        m_pd3dDevice->SetTexture( 0, m_pTexture0 );
        m_pd3dDevice->SetTexture( 1, m_pTexture1 );

        // Set the pixel shader constants
        FLOAT fPixelShaderConstants[8][4] =
        { //  Red  Green  Blue  Alpha
            { 1.0f, 0.0f, 0.0f, 1.0f, },  // red
            { 0.0f, 1.0f, 0.0f, 1.0f, },  // green
            { 0.0f, 0.0f, 1.0f, 1.0f, },  // blue
            { 1.0f, 1.0f, 0.0f, 1.0f, },  // yellow
            { 0.0f, 1.0f, 1.0f, 1.0f, },  // cyan
            { 1.0f, 0.0f, 1.0f, 1.0f, },  // purple
            { 1.0f, 1.0f, 1.0f, 1.0f, },  // white
            { 0.0f, 0.0f, 0.0f, 1.0f, },  // black
        };
        m_pd3dDevice->SetPixelShaderConstant( 0, (VOID*)fPixelShaderConstants, 8 );

        // Render the quad with the pixel shader
        m_pd3dDevice->SetStreamSource( 0, m_pQuadVB, sizeof(CUSTOMVERTEX) );
        m_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
        m_pd3dDevice->SetPixelShader( m_hPixelShader );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InitDeviceObjects()
{
    HRESULT hr;

    // Create some textures
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("DX5_Logo.bmp"),
                                       &m_pTexture0, D3DFMT_R5G6B5 ) ) )
    {
        return E_FAIL;
    }

    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Tree01S.tga"),
                                       &m_pTexture1, D3DFMT_R5G6B5 ) ) )
    {
        return E_FAIL;
    }

    // Create quad VB
    hr = m_pd3dDevice->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),
                                           D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
                                           D3DPOOL_MANAGED, &m_pQuadVB );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CAppForm::RestoreDeviceObjects()
{
    HRESULT hr;

    // Fill the quad VB
    CUSTOMVERTEX* pVertices = NULL;
    hr = m_pQuadVB->Lock( 0, 4*sizeof(CUSTOMVERTEX), (BYTE**)&pVertices, 0 );
    if( FAILED(hr) )
        return hr;

    for( DWORD i=0; i<4; i++ )
        pVertices[i] = g_Vertices[i];

    m_pQuadVB->Unlock();

    // Delete old pixel shader (redundant if InvalidateDeviceObjects is called first)
    if( m_hPixelShader )
        m_pd3dDevice->DeletePixelShader( m_hPixelShader );

    m_hPixelShader = NULL;

    // Create new pixel shader
    if( m_pD3DXBufShader ) 
        hr = m_pd3dDevice->CreatePixelShader( 
                                 (DWORD*)m_pD3DXBufShader->GetBufferPointer(),
                                 &m_hPixelShader );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CAppForm::InvalidateDeviceObjects()
{
    if( m_hPixelShader )
        m_pd3dDevice->DeletePixelShader( m_hPixelShader );
    m_hPixelShader = NULL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CAppForm::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pTexture0 );
    SAFE_RELEASE( m_pTexture1 );
    SAFE_RELEASE( m_pQuadVB );
    SAFE_RELEASE( m_pD3DXBufShader );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CAppForm::FinalCleanup()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CAppForm::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
    if( D3DSHADER_VERSION_MAJOR( pCaps->PixelShaderVersion ) < 1 )
        return E_FAIL;

    return S_OK;
}





//-----------------------------------------------------------------------------
// The remaining code handles the UI for the MFC-based app.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: OnChangeDevice()
// Desc: Use hit the "Change Device.." button. Display the dialog for the user
//       to select a new device/mode, and call Change3DEnvironment to
//       use the new device/mode.
//-----------------------------------------------------------------------------
VOID CAppForm::OnChangeDevice()
{
    UserSelectNewDevice();
    GeneratePixelShaderOpcodes();
}




//-----------------------------------------------------------------------------
// Name: OnToggleFullScreen()
// Desc: Called when user toggles the fullscreen mode
//-----------------------------------------------------------------------------
void CAppForm::OnToggleFullScreen()
{
    ToggleFullscreen();
    GeneratePixelShaderOpcodes();
}




//-----------------------------------------------------------------------------
// Name: OnHScroll()
// Desc: Called when the user moves any scroll bar. Check which scrollbar was
//       moved, and extract the appropiate value to a global variable.
//-----------------------------------------------------------------------------
void CAppForm::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}




//-----------------------------------------------------------------------------
// Name: UpdateUIForDeviceCapabilites()
// Desc: Whenever we get a new device, call this function to enable/disable the
//       appropiate UI controls to match the device's capabilities.
//-----------------------------------------------------------------------------
VOID CAppForm::UpdateUIForDeviceCapabilites()
{
    // Check the capabilities of the device

    // Update the UI checkbox states
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CAppForm::OnPresets() 
{
    DWORD dwPreset = 0L;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_0))->GetCheck() ? 0 : dwPreset;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_1))->GetCheck() ? 1 : dwPreset;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_2))->GetCheck() ? 2 : dwPreset;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_3))->GetCheck() ? 3 : dwPreset;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_4))->GetCheck() ? 4 : dwPreset;
    dwPreset = ((CButton*)GetDlgItem(IDC_PRESET_5))->GetCheck() ? 5 : dwPreset;

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_INSTRUCTIONS);
    pEdit->SetWindowText( g_strPixelShaderPresets[dwPreset] );

    // Generate the opcodes and pass them to the app
    if( m_pd3dDevice != NULL )
        GeneratePixelShaderOpcodes();
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CAppForm::GeneratePixelShaderOpcodes() 
{
    TCHAR strOpcodes[2048] = _T("");

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_INSTRUCTIONS);
    pEdit->GetWindowText(strOpcodes, 2048);

    SetPixelShader( strOpcodes );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CAppForm::OnOpenPixelShaderFile() 
{
    ((CButton*)GetDlgItem(IDC_PRESET_0))->SetCheck( FALSE );
    ((CButton*)GetDlgItem(IDC_PRESET_1))->SetCheck( FALSE );
    ((CButton*)GetDlgItem(IDC_PRESET_2))->SetCheck( FALSE );
    ((CButton*)GetDlgItem(IDC_PRESET_3))->SetCheck( FALSE );
    ((CButton*)GetDlgItem(IDC_PRESET_4))->SetCheck( FALSE );
    ((CButton*)GetDlgItem(IDC_PRESET_5))->SetCheck( FALSE );


    static TCHAR g_strFileName[MAX_PATH] = _T("");
    static TCHAR g_strInitialDir[MAX_PATH] = _T("");
    TCHAR strFileName[MAX_PATH] = _T("");
    TCHAR strBuffer[81];

    // Display the OpenFileName dialog. Then, try to load the specified file
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         _T("Pixel Shader Text Files (.txt)\0*.txt\0\0"), 
                         NULL, 0, 1, strFileName, MAX_PATH, g_strFileName, MAX_PATH, 
                         g_strInitialDir, _T("Open Pixel Shader File"), 
                         OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

    if( FALSE == GetOpenFileName( &ofn ) )
        return;
    
    FILE* file = fopen( strFileName, _T("r") );
    if( file == NULL )
        return;

    // Fill the list box with the preset's pixel shader instructions
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_INSTRUCTIONS);
    pEdit->SetSel(0, -1);
    while( fgets( strBuffer, 80, file ) )
    {
        // Remove trailing newline char
        if( strBuffer[max(1,strlen(strBuffer))-1] == '\n' )
            strBuffer[max(1,strlen(strBuffer))-1] = '\0';
        pEdit->ReplaceSel(strBuffer);
        pEdit->ReplaceSel("\r\n");
    }

    fclose(file);

    // Generater the opcodes and pass them to the app
    GeneratePixelShaderOpcodes();
}


void CAppForm::OnChangeInstructions() 
{
    GeneratePixelShaderOpcodes();
}

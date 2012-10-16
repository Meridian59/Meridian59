//-----------------------------------------------------------------------------
// File: Audiopath.cpp
//
// Desc: Uses a 3D Audiopath, and shows off various methods of PlaySegmentEx
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <commctrl.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <cguid.h>
#include <dxerr8.h>
#include <tchar.h>
#include "resource.h"
#include "DMUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );
HRESULT PlaySegment( DWORD dwIndex );
HRESULT SetPosition( float fXPos, float fYPos, float fZPos );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
CMusicManager*          g_pMusicManager          = NULL;
CMusicSegment*          g_pMusicSegments[4]      = { NULL,NULL,NULL,NULL };
IDirectMusicAudioPath*  g_p3DAudiopath           = NULL;
HINSTANCE               g_hInst                  = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    g_hInst = hInst;

    InitCommonControls();

    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );
   
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( hDlg, "Error initializing DirectMusic.  Sample will now exit.", 
                                  "DirectMusic Sample", MB_OK | MB_ICONERROR );
                EndDialog( hDlg, 0 );
                return TRUE;
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, 0 );
                    break;

                case IDC_PLAY1:
                case IDC_PLAY2:
                case IDC_PLAY3:
                case IDC_PLAY4:
                {
                    DWORD dwIndex = LOWORD(wParam) - IDC_PLAY1;
                    if( FAILED( hr = PlaySegment( dwIndex ) ) )
                    {
                        DXTRACE_ERR( TEXT("PlaySegment"), hr );
                        MessageBox( hDlg, "Error playing DirectMusic segment.  Sample will now exit.", 
                                          "DirectMusic Sample", MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, 0 );
                        return TRUE;
                    }                   
                    break;
                }

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_HSCROLL:
        {
            // Set the 3D position
	        int nXPos = (int)SendDlgItemMessage( hDlg, IDC_XPOS, TBM_GETPOS, 0, 0 );
	        int nYPos = (int)SendDlgItemMessage( hDlg, IDC_YPOS, TBM_GETPOS, 0, 0 );
	        int nZPos = (int)SendDlgItemMessage( hDlg, IDC_ZPOS, TBM_GETPOS, 0, 0 );
	        SetDlgItemInt( hDlg, IDC_XDISPLAY, nXPos, TRUE );
	        SetDlgItemInt( hDlg, IDC_YDISPLAY, nYPos, TRUE );
	        SetDlgItemInt( hDlg, IDC_ZDISPLAY, nZPos, TRUE );
            SetPosition( (float) nXPos, (float) nYPos, (float) nZPos );
            break;
        }

        case WM_DESTROY:
        {
            // Cleanup everything
            SAFE_RELEASE( g_p3DAudiopath );

            for( int i=0; i<4; i++ )
                SAFE_DELETE( g_pMusicSegments[i] );

            SAFE_DELETE( g_pMusicManager );
            break; 
        }

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr; 
  
    // Load the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    g_pMusicManager = new CMusicManager();

    if( FAILED( hr = g_pMusicManager->Initialize( hDlg ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

	// Create a 3D Audiopath. This creates a synth port that feeds a 3d buffer.
	// We can then play all segments into this buffer and directly control its
	// 3D parameters.
	IDirectMusicPerformance8* pPerformance = g_pMusicManager->GetPerformance();
	if( FAILED( hr = pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 128, 
                                                            TRUE, &g_p3DAudiopath ) ) )
        return DXTRACE_ERR( TEXT("CreateStandardAudioPath"), hr );

    // Set the default media path (something like C:\MSSDK\SAMPLES\MULTIMEDIA\MEDIA)
    // to be used as the search directory for finding DirectMusic content.
    if( FAILED( hr = g_pMusicManager->SetSearchDirectory( DXUtil_GetDXSDKMediaPath() ) ) )
        return DXTRACE_ERR( TEXT("SetSearchDirectory"), hr );

    TCHAR strFileNames[4][MAX_PATH] = { TEXT("Audiopath1.sgt"),	// Lullaby theme
		                                TEXT("Audiopath2.sgt"),	// Snoring
		                                TEXT("Audiopath3.wav"),	// Muttering in sleep
		                                TEXT("Audiopath4.sgt")  // Rude awakening
	                                  };

    // Create the segments from a file
	for (DWORD dwIndex = 0;dwIndex < 4; dwIndex++)
	{
        if( FAILED( hr = g_pMusicManager->CreateSegmentFromFile( &g_pMusicSegments[dwIndex], 
                                                                strFileNames[dwIndex] ) ) )
            return DXTRACE_ERR( TEXT("CreateSegmentFromFile"), hr );
    }


    // Get the listener from the in the Audiopath.
    IDirectSound3DListener* pDSListener = NULL;
    if( FAILED( hr = g_p3DAudiopath->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0,
                                                      GUID_NULL, 0, IID_IDirectSound3DListener, 
                                                      (LPVOID*) &pDSListener ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Set a new rolloff factor (1.0f is default)
    if( FAILED( hr = pDSListener->SetRolloffFactor( 0.25f, DS3D_IMMEDIATE ) ) )
        return DXTRACE_ERR( TEXT("SetRolloffFactor"), hr );       

    // Release the listener since we are done with it.
    SAFE_RELEASE( pDSListener );

    // Setup the sliders
    HWND hSlider;
    hSlider = GetDlgItem( hDlg, IDC_XPOS );
    SendMessage( hSlider, TBM_SETRANGEMAX, TRUE,   20L );
    SendMessage( hSlider, TBM_SETRANGEMIN, TRUE,  -20L );
    SendMessage( hSlider, TBM_SETPOS,      TRUE,    0L );

    hSlider = GetDlgItem( hDlg, IDC_YPOS );
    SendMessage( hSlider, TBM_SETRANGEMAX, TRUE,   20L );
    SendMessage( hSlider, TBM_SETRANGEMIN, TRUE,  -20L );
    SendMessage( hSlider, TBM_SETPOS,      TRUE,    0L );

    hSlider = GetDlgItem( hDlg, IDC_ZPOS );
    SendMessage( hSlider, TBM_SETRANGEMAX, TRUE,   20L );
    SendMessage( hSlider, TBM_SETRANGEMIN, TRUE,  -20L );
    SendMessage( hSlider, TBM_SETPOS,      TRUE,    0L );

	SetDlgItemInt( hDlg, IDC_XDISPLAY, 0, TRUE );
	SetDlgItemInt( hDlg, IDC_YDISPLAY, 0, TRUE );
	SetDlgItemInt( hDlg, IDC_ZDISPLAY, 0, TRUE );
	SetPosition( 0, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PlaySegment()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT PlaySegment( DWORD dwIndex )
{
    HRESULT hr = S_OK; 

	if( g_pMusicSegments[dwIndex] )
	{
        switch( dwIndex )
        {
        case 0:
			// Lullaby theme. This should play as a primary segment.
            hr = g_pMusicSegments[dwIndex]->Play( DMUS_SEGF_DEFAULT, g_p3DAudiopath );
            break;

        case 1:
        case 2:
			// Sound effects. These play as secondary segments so 
            // they can be triggered multiple times and will layer on top.
            hr = g_pMusicSegments[dwIndex]->Play( DMUS_SEGF_DEFAULT | DMUS_SEGF_SECONDARY, 
                                                  g_p3DAudiopath );
            break;

        case 3:
			// Rude awakening. Notice that this also passes the Audiopath 
            // in pFrom, indicating that all segments currently playing on 
            // the Audiopath should be stopped at the exact time
			// this starts. 
            IDirectMusicSegment8* pSegment = g_pMusicSegments[dwIndex]->GetSegment();
			IDirectMusicPerformance8* pPerformance = g_pMusicManager->GetPerformance();

            hr = pPerformance->PlaySegmentEx( pSegment, 0, NULL, 0, 0, 0, 
                                              g_p3DAudiopath, g_p3DAudiopath );
        }
	}

    if( FAILED(hr) )
        return DXTRACE_ERR( TEXT("PlaySegmentEx"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetPosition()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT SetPosition( float fXPos, float fYPos, float fZPos )
{
    HRESULT hr;

	if( NULL == g_p3DAudiopath )
        return E_INVALIDARG;

	// First, get the 3D interface from the buffer by using GetObjectInPath.
	IDirectSound3DBuffer *pBuffer;
    if( FAILED( hr = g_p3DAudiopath->GetObjectInPath( DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, 
		                                              GUID_NULL, 0, IID_IDirectSound3DBuffer, 
                                                      (void **)&pBuffer ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

	// Then, set the coordinates and release.
	if( FAILED( hr = pBuffer->SetPosition( fXPos, fYPos, fZPos, DS3D_IMMEDIATE ) ) )
        return DXTRACE_ERR( TEXT("SetPosition"), hr );

    SAFE_RELEASE( pBuffer );

    return S_OK;
}



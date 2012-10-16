//-----------------------------------------------------------------------------
// File: 3DAudio.cpp
//
// Desc: Plays a primary segment using DirectMusic 
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
#include <dxerr8.h>
#include <cguid.h>
#include <math.h>
#include <stdio.h>
#include "resource.h"
#include "DMUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
VOID    OnInitDialog( HWND hDlg );
HRESULT InitAudio( HWND hDlg );
VOID    SetSlidersPos( HWND hDlg, FLOAT fDopplerValue, FLOAT fRolloffValue, FLOAT fMinDistValue, FLOAT fMaxDistValue );
VOID    OnOpenAudioFile( HWND hDlg );
HRESULT LoadSegmentFile( HWND hDlg, TCHAR* strFileName );
HRESULT OnPlayAudio( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
VOID    OnMovementTimer( HWND hDlg );
VOID    Set3DParameters( FLOAT fDopplerFactor, FLOAT fRolloffFactor, FLOAT fMinDistance,   FLOAT fMaxDistance );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
VOID    SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity );
VOID    UpdateGrid( HWND hDlg, FLOAT x, FLOAT y );
VOID    OnSliderChanged( HWND hDlg );
VOID    SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity );
FLOAT   ConvertLinearSliderPosToLogScale( LONG lSliderPos );
LONG    ConvertLogScaleToLinearSliderPosTo( FLOAT fValue );





//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define ORBIT_MAX_RADIUS        5.0f
#define IDT_MOVEMENT_TIMER      1

CMusicManager*          g_pMusicManager          = NULL;
CMusicSegment*          g_pMusicSegment          = NULL;
IDirectMusicAudioPath*  g_p3DAudioPath           = NULL;
IDirectSound3DBuffer*   g_pDS3DBuffer            = NULL;   // 3D sound buffer
IDirectSound3DListener* g_pDSListener            = NULL;   // 3D listener object
DS3DBUFFER              g_dsBufferParams;                  // 3D buffer properties
DS3DLISTENER            g_dsListenerParams;                // Listener properties
HINSTANCE               g_hInst                  = NULL;
BOOL                    g_bAllowMovementTimer    = TRUE;
BOOL                    g_bDeferSettings         = FALSE;




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
            OnInitDialog( hDlg );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SOUNDFILE:
                    OnOpenAudioFile( hDlg );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, 0 );
                    break;

                case IDC_PLAY:
                    if( FAILED( hr = OnPlayAudio( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlayAudio"), hr );
                        MessageBox( hDlg, "Error playing DirectMusic segment. "
                                    "Sample will now exit.", "DirectMusic Sample", 
                                    MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, 0 );
                    }
                    break;

                case IDC_STOP:
                    if( g_pMusicSegment )
                    {
                        g_pMusicSegment->Stop( DMUS_SEGF_BEAT ); 
                        EnablePlayUI( hDlg, TRUE );
                    }
                    break;

                case IDC_DEFER:
                    g_bDeferSettings = !g_bDeferSettings;
                    OnSliderChanged( hDlg );                    
                    break;

                case IDC_APPLY:
                    // Call the IDirectSound3DListener::CommitDeferredSettings 
                    // method to execute all of the deferred commands at once.
                    // This is many times more efficent than recomputing everything
                    // for every call.
                    if( g_pDSListener )
                        g_pDSListener->CommitDeferredSettings();
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_TIMER:
            if( wParam == IDT_MOVEMENT_TIMER )
                OnMovementTimer( hDlg );
            break;

        case WM_NOTIFY:
            OnSliderChanged( hDlg );
            break;

        case WM_DESTROY:
            // Cleanup everything
            if( g_pMusicSegment )
            {
                g_pMusicSegment->Unload( g_p3DAudioPath );
                SAFE_DELETE( g_pMusicSegment );
            }

            KillTimer( hDlg, 1 );    
            SAFE_RELEASE( g_pDSListener );
            SAFE_RELEASE( g_pDS3DBuffer );
            SAFE_RELEASE( g_p3DAudioPath );

            SAFE_DELETE( g_pMusicManager );
            break; 

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
VOID OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Load the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    if( FAILED( hr = InitAudio( hDlg ) ) )
    {
        DXTRACE_ERR( TEXT("InitAudio"), hr );
        MessageBox( hDlg, "Error initializing DirectMusic.  Sample will now exit.", 
                          "DirectMusic Sample", MB_OK | MB_ICONERROR );
        EndDialog( hDlg, 0 );
        return;
    }

    // Set the default media path (something like C:\MSSDK\SAMPLES\MULTIMEDIA\MEDIA)
    // to be used as the search directory for finding DirectMusic content.
    g_pMusicManager->SetSearchDirectory( DXUtil_GetDXSDKMediaPath() );

    // Load a default music segment 
    TCHAR strFileName[MAX_PATH];
    strcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
    strcat( strFileName, "sample.sgt" );
    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        // Set the UI controls
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("No file loaded.") );
    }

    // Create a timer to periodically move the 3D object around
    SetTimer( hDlg, IDT_MOVEMENT_TIMER, 5, NULL );

    // Get handles to dialog items
    HWND hDopplerSlider  = GetDlgItem( hDlg, IDC_DOPPLER_SLIDER );
    HWND hRolloffSlider  = GetDlgItem( hDlg, IDC_ROLLOFF_SLIDER );
    HWND hMinDistSlider  = GetDlgItem( hDlg, IDC_MINDISTANCE_SLIDER );
    HWND hMaxDistSlider  = GetDlgItem( hDlg, IDC_MAXDISTANCE_SLIDER );
    HWND hVertSlider     = GetDlgItem( hDlg, IDC_VERTICAL_SLIDER );
    HWND hHorzSlider     = GetDlgItem( hDlg, IDC_HORIZONTAL_SLIDER );

    // Set the range and position of the sliders
    PostMessage( hDopplerSlider, TBM_SETRANGEMAX, TRUE, 40L );
    PostMessage( hDopplerSlider, TBM_SETRANGEMIN, TRUE, 0L );

    PostMessage( hRolloffSlider, TBM_SETRANGEMAX, TRUE, 40L );
    PostMessage( hRolloffSlider, TBM_SETRANGEMIN, TRUE, 0L );

    PostMessage( hMinDistSlider, TBM_SETRANGEMAX, TRUE, 40L );
    PostMessage( hMinDistSlider, TBM_SETRANGEMIN, TRUE, 1L );

    PostMessage( hMaxDistSlider, TBM_SETRANGEMAX, TRUE, 40L );
    PostMessage( hMaxDistSlider, TBM_SETRANGEMIN, TRUE, 1L );

    PostMessage( hVertSlider,    TBM_SETRANGEMAX, TRUE, 100L );
    PostMessage( hVertSlider,    TBM_SETRANGEMIN, TRUE, -100L );
    PostMessage( hVertSlider,    TBM_SETPOS,      TRUE, 10L );

    PostMessage( hHorzSlider,    TBM_SETRANGEMAX, TRUE, 100L );
    PostMessage( hHorzSlider,    TBM_SETRANGEMIN, TRUE, -100L );
    PostMessage( hHorzSlider,    TBM_SETPOS,      TRUE, 50L );

    // Set the position of the sliders
    SetSlidersPos( hDlg, 0.0f, 0.0f, ORBIT_MAX_RADIUS, ORBIT_MAX_RADIUS*2.0f );
}




//-----------------------------------------------------------------------------
// Name: InitAudio()
// Desc: Init both DirectMusic and DirectSound
//-----------------------------------------------------------------------------
HRESULT InitAudio( HWND hDlg )
{
    HRESULT hr;

    // Initialize the performance. This initializes both DirectMusic and DirectSound
    // and optionally sets up the synthesizer and default audio path.
    // However, since this app never uses the default audio path, we don't bother 
    // to do that here.
    g_pMusicManager = new CMusicManager();
    if( FAILED( hr = g_pMusicManager->Initialize( hDlg ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    IDirectMusicPerformance8* pPerformance = g_pMusicManager->GetPerformance();

    // Create a 3D audiopath with a 3d buffer.
    // We can then play all segments into this buffer and directly control its
    // 3D parameters.
    if( FAILED( hr = pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 
                                                            64, TRUE, &g_p3DAudioPath ) ) )
        return DXTRACE_ERR( TEXT("CreateStandardAudioPath"), hr );

    // Get the 3D buffer in the audio path.
    if( FAILED( hr = g_p3DAudioPath->GetObjectInPath( 0, DMUS_PATH_BUFFER, 0,
                                                      GUID_NULL, 0, IID_IDirectSound3DBuffer, 
                                                      (LPVOID*) &g_pDS3DBuffer ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Get the 3D buffer parameters
    g_dsBufferParams.dwSize = sizeof(DS3DBUFFER);
    g_pDS3DBuffer->GetAllParameters( &g_dsBufferParams );

    // Set new 3D buffer parameters
    g_dsBufferParams.dwMode = DS3DMODE_HEADRELATIVE;
    g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_IMMEDIATE );
    
    // Get the listener from the in the audio path.
    if( FAILED( hr = g_p3DAudioPath->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0,
                                                      GUID_NULL, 0, IID_IDirectSound3DListener, 
                                                      (LPVOID*) &g_pDSListener ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Get listener parameters
    g_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
    g_pDSListener->GetAllParameters( &g_dsListenerParams );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetSlidersPos()
// Desc: Sets the slider positions
//-----------------------------------------------------------------------------
VOID SetSlidersPos( HWND hDlg, FLOAT fDopplerValue, FLOAT fRolloffValue,
                    FLOAT fMinDistValue, FLOAT fMaxDistValue )
{
    HWND hDopplerSlider  = GetDlgItem( hDlg, IDC_DOPPLER_SLIDER );
    HWND hRolloffSlider  = GetDlgItem( hDlg, IDC_ROLLOFF_SLIDER );
    HWND hMinDistSlider  = GetDlgItem( hDlg, IDC_MINDISTANCE_SLIDER );
    HWND hMaxDistSlider  = GetDlgItem( hDlg, IDC_MAXDISTANCE_SLIDER );

    LONG lDopplerSlider = ConvertLogScaleToLinearSliderPosTo( fDopplerValue );
    LONG lRolloffSlider = ConvertLogScaleToLinearSliderPosTo( fRolloffValue );
    LONG lMinDistSlider = ConvertLogScaleToLinearSliderPosTo( fMinDistValue );
    LONG lMaxDistSlider = ConvertLogScaleToLinearSliderPosTo( fMaxDistValue );

    SendMessage( hDopplerSlider, TBM_SETPOS, TRUE, lDopplerSlider );
    SendMessage( hRolloffSlider, TBM_SETPOS, TRUE, lRolloffSlider );
    SendMessage( hMinDistSlider, TBM_SETPOS, TRUE, lMinDistSlider );
    SendMessage( hMaxDistSlider, TBM_SETPOS, TRUE, lMaxDistSlider );
}




//-----------------------------------------------------------------------------
// Name: OnOpenAudioFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenAudioFile( HWND hDlg ) 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");

    // Get the default media path (something like C:\MSSDK\SAMPLES\DMUSIC\MEDIA)
    if( '\0' == strPath[0] )
    {
        const TCHAR* szDir = DXUtil_GetDXSDKMediaPath();
        strcpy( strPath, szDir );
    }

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("DirectMusic Content Files\0*.sgt;*.mid;*.rmi\0Wave Files\0*.wav\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Content File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".sgt"), 0, NULL, NULL };

    if( g_pMusicSegment )
        g_pMusicSegment->Stop( 0 );

  // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_STOP ), FALSE );
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Loading file...") );

    // Stop the timer while dialogs are displayed
    g_bAllowMovementTimer = FALSE;

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Load aborted.") );
        g_bAllowMovementTimer = TRUE;
        return;
    }

    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        // Not a critical failure, so just update the status
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create segment from file.") );
    }

    g_bAllowMovementTimer = TRUE;

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}




//-----------------------------------------------------------------------------
// Name: LoadSegmentFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LoadSegmentFile( HWND hDlg, TCHAR* strFileName )
{
    HRESULT hr;
     
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

    // Free any previous segment, and make a new one
    SAFE_DELETE( g_pMusicSegment );

    // Have the loader collect any garbage now that the old 
    // segment has been released
    g_pMusicManager->CollectGarbage();

    // For DirectMusic must know if the file is a standard MIDI file or not
    // in order to load the correct instruments.
    BOOL bMidiFile = FALSE;
    if( strstr( strFileName, ".mid" ) != NULL ||
        strstr( strFileName, ".rmi" ) != NULL ) 
    {
        bMidiFile = TRUE;
    }

    // Load the file into a DirectMusic segment, but don't download 
    // it to the default audio path -- use the 3D audio path instead
    if( FAILED( g_pMusicManager->CreateSegmentFromFile( &g_pMusicSegment, strFileName, 
                                                        FALSE, bMidiFile ) ) )
    {
        // Not a critical failure, so just update the status
        return S_FALSE; 
    }

    // Download the segment on the 3D audio path
    if( FAILED( hr = g_pMusicSegment->Download( g_p3DAudioPath ) ) )
        return DXTRACE_ERR( TEXT("Download"), hr );

    // Update the UI controls to show the segment is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    EnablePlayUI( hDlg, TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnPlayAudio()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnPlayAudio( HWND hDlg )
{
    HRESULT hr;

    if( g_pMusicSegment == NULL )
        return S_FALSE;

    // Set the segment to repeat many times
    if( FAILED( hr = g_pMusicSegment->SetRepeats( DMUS_SEG_REPEAT_INFINITE ) ) )
        return DXTRACE_ERR( TEXT("SetRepeats"), hr );

    // Play the segment and wait. The DMUS_SEGF_BEAT indicates to play on the 
    // next beat if there is a segment currently playing. 
    if( FAILED( hr = g_pMusicSegment->Play( DMUS_SEGF_BEAT, g_p3DAudioPath ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

    EnablePlayUI( hDlg, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnablePlayUI( hDlg,)
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow(   GetDlgItem( hDlg, IDC_STOP ),       FALSE );

        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
        SetFocus(       GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow(  GetDlgItem( hDlg, IDC_STOP ),       TRUE );
        SetFocus(      GetDlgItem( hDlg, IDC_STOP ) );
        EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ),       FALSE );
    }
}




//-----------------------------------------------------------------------------
// Name: OnSliderChanged()  
// Desc: Called when the dialog's slider bars are changed by the user, or need
//       updating
//-----------------------------------------------------------------------------
VOID OnSliderChanged( HWND hDlg )
{
    TCHAR strBuffer[10];
    FLOAT fDopplerFactor;
    FLOAT fRolloffFactor;
    FLOAT fMinDistance; 
    FLOAT fMaxDistance;

    // Get handles to dialog items
    HWND hDopplerSlider  = GetDlgItem( hDlg, IDC_DOPPLER_SLIDER );
    HWND hRolloffSlider  = GetDlgItem( hDlg, IDC_ROLLOFF_SLIDER );
    HWND hMinDistSlider  = GetDlgItem( hDlg, IDC_MINDISTANCE_SLIDER );
    HWND hMaxDistSlider  = GetDlgItem( hDlg, IDC_MAXDISTANCE_SLIDER );

    // Get the position of the sliders
    fDopplerFactor = ConvertLinearSliderPosToLogScale( (long)SendMessage( hDopplerSlider, TBM_GETPOS, 0, 0 ) );
    fRolloffFactor = ConvertLinearSliderPosToLogScale( (long)SendMessage( hRolloffSlider, TBM_GETPOS, 0, 0 ) );
    fMinDistance   = ConvertLinearSliderPosToLogScale( (long)SendMessage( hMinDistSlider, TBM_GETPOS, 0, 0 ) );
    fMaxDistance   = ConvertLinearSliderPosToLogScale( (long)SendMessage( hMaxDistSlider, TBM_GETPOS, 0, 0 ) );

    // Set the static text boxes
    sprintf( strBuffer, TEXT("%.2f"), fDopplerFactor );
    SetWindowText( GetDlgItem( hDlg, IDC_DOPPLERFACTOR ), strBuffer );

    sprintf( strBuffer, TEXT("%.2f"), fRolloffFactor );
    SetWindowText( GetDlgItem( hDlg, IDC_ROLLOFFFACTOR ), strBuffer );

    sprintf( strBuffer, TEXT("%.2f"), fMinDistance );
    SetWindowText( GetDlgItem( hDlg, IDC_MINDISTANCE ), strBuffer );

    sprintf( strBuffer, TEXT("%.2f"), fMaxDistance );
    SetWindowText( GetDlgItem( hDlg, IDC_MAXDISTANCE ), strBuffer );

    // Set the options in the DirectSound buffer
    Set3DParameters( fDopplerFactor, fRolloffFactor, fMinDistance, fMaxDistance );

    EnableWindow( GetDlgItem( hDlg, IDC_APPLY ), g_bDeferSettings );
}




//-----------------------------------------------------------------------------
// Name: Set3DParameters()
// Desc: Set the 3D buffer parameters
//-----------------------------------------------------------------------------
VOID Set3DParameters( FLOAT fDopplerFactor, FLOAT fRolloffFactor,
                      FLOAT fMinDistance,   FLOAT fMaxDistance )
{
    // Every change to 3-D sound buffer and listener settings causes 
    // DirectSound to remix, at the expense of CPU cycles. 
    // To minimize the performance impact of changing 3-D settings, 
    // use the DS3D_DEFERRED flag in the dwApply parameter of any of 
    // the IDirectSound3DListener or IDirectSound3DBuffer methods that 
    // change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
    // method to execute all of the deferred commands at once.
    DWORD dwApplyFlag = ( g_bDeferSettings ) ? DS3D_DEFERRED : DS3D_IMMEDIATE;

    g_dsListenerParams.flDopplerFactor = fDopplerFactor;
    g_dsListenerParams.flRolloffFactor = fRolloffFactor;

    if( g_pDSListener )
        g_pDSListener->SetAllParameters( &g_dsListenerParams, dwApplyFlag );

    g_dsBufferParams.flMinDistance = fMinDistance;
    g_dsBufferParams.flMaxDistance = fMaxDistance;

    if( g_pDS3DBuffer )
        g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, dwApplyFlag );
}




//-----------------------------------------------------------------------------
// Name: OnMovementTimer()
// Desc: Periodically updates the position of the object 
//-----------------------------------------------------------------------------
VOID OnMovementTimer( HWND hDlg ) 
{
    FLOAT fXScale;
    FLOAT fYScale;

    if( !g_bAllowMovementTimer )
        return;

    HWND hHorzSlider = GetDlgItem( hDlg, IDC_HORIZONTAL_SLIDER );
    HWND hVertSlider = GetDlgItem( hDlg, IDC_VERTICAL_SLIDER );

    fXScale = SendMessage( hHorzSlider, TBM_GETPOS, 0, 0 ) / 100.0f;
    fYScale = SendMessage( hVertSlider, TBM_GETPOS, 0, 0 ) / 100.0f;
    FLOAT t = timeGetTime()/1000.0f;

    // Move the sound object around the listener. The maximum radius of the
    // orbit is 27.5 units.
    D3DVECTOR vPosition;
    vPosition.x = ORBIT_MAX_RADIUS * fXScale * (FLOAT)sin(t);
    vPosition.y = 0.0f;
    vPosition.z = ORBIT_MAX_RADIUS * fYScale * (FLOAT)cos(t);

    D3DVECTOR vVelocity;
    vVelocity.x = ORBIT_MAX_RADIUS * fXScale * (FLOAT)sin(t+0.05f);
    vVelocity.y = 0.0f;
    vVelocity.z = ORBIT_MAX_RADIUS * fYScale * (FLOAT)cos(t+0.05f);

    // Show the object's position on the dialog's grid control
    UpdateGrid( hDlg, vPosition.x, vPosition.z );

    // Set the sound buffer velocity and position
    SetObjectProperties( &vPosition, &vVelocity );
}




//-----------------------------------------------------------------------------
// Name: UpdateGrid()
// Desc: Draws a red dot in the dialog's grid bitmap at the x,y coordinate.
//-----------------------------------------------------------------------------
VOID UpdateGrid( HWND hDlg, FLOAT x, FLOAT y )
{
    static LONG s_lPixel[5] = { 0,0,0,0,0 };
    static LONG s_lX = 0;
    static LONG s_lY = 0;

    HWND hWndGrid = GetDlgItem( hDlg, IDC_RENDER_WINDOW );
    HDC  hDC  = GetDC( hWndGrid );
    RECT rc;
    
    // Replace pixels from that were overdrawn last time
    SetPixel( hDC, s_lX-1, s_lY+0, s_lPixel[0] );
    SetPixel( hDC, s_lX+0, s_lY-1, s_lPixel[1] );
    SetPixel( hDC, s_lX+0, s_lY+0, s_lPixel[2] );
    SetPixel( hDC, s_lX+0, s_lY+1, s_lPixel[3] );
    SetPixel( hDC, s_lX+1, s_lY+0, s_lPixel[4] );

    // Convert the world space x,y coordinates to pixel coordinates
    GetClientRect( hWndGrid, &rc );
    s_lX = (LONG)( ( x/ORBIT_MAX_RADIUS + 1 ) * ( rc.left + rc.right ) / 2 );
    s_lY = (LONG)( (-y/ORBIT_MAX_RADIUS + 1 ) * ( rc.top + rc.bottom ) / 2 );

    // Save the pixels before drawing the cross hair
    s_lPixel[0] = GetPixel( hDC, s_lX-1, s_lY+0 );
    s_lPixel[1] = GetPixel( hDC, s_lX+0, s_lY-1 );
    s_lPixel[2] = GetPixel( hDC, s_lX+0, s_lY+0 );
    s_lPixel[3] = GetPixel( hDC, s_lX+0, s_lY+1 );
    s_lPixel[4] = GetPixel( hDC, s_lX+1, s_lY+0 );

    // Draw a crosshair object in red pixels
    SetPixel( hDC, s_lX-1, s_lY+0, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY-1, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY+0, 0x000000ff );
    SetPixel( hDC, s_lX+0, s_lY+1, 0x000000ff );
    SetPixel( hDC, s_lX+1, s_lY+0, 0x000000ff );

    ReleaseDC( hWndGrid, hDC );
}




//-----------------------------------------------------------------------------
// Name: SetObjectProperties()
// Desc: Sets the position and velocity on the 3D buffer
//-----------------------------------------------------------------------------
VOID SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity )
{
    // Every change to 3-D sound buffer and listener settings causes 
    // DirectSound to remix, at the expense of CPU cycles. 
    // To minimize the performance impact of changing 3-D settings, 
    // use the DS3D_DEFERRED flag in the dwApply parameter of any of 
    // the IDirectSound3DListener or IDirectSound3DBuffer methods that 
    // change 3-D settings. Then call the IDirectSound3DListener::CommitDeferredSettings 
    // method to execute all of the deferred commands at once.
    memcpy( &g_dsBufferParams.vPosition, pvPosition, sizeof(D3DVECTOR) );
    memcpy( &g_dsBufferParams.vVelocity, pvVelocity, sizeof(D3DVECTOR) );

    if( g_pDS3DBuffer )
    {
        g_pDS3DBuffer->SetPosition( g_dsBufferParams.vPosition.x,
                                    g_dsBufferParams.vPosition.y,
                                    g_dsBufferParams.vPosition.z, DS3D_IMMEDIATE );

        g_pDS3DBuffer->SetVelocity( g_dsBufferParams.vVelocity.x,
                                    g_dsBufferParams.vVelocity.y,
                                    g_dsBufferParams.vVelocity.z, DS3D_IMMEDIATE );
    }
}




//-----------------------------------------------------------------------------
// Name: ConvertLinearSliderPosToLogScale()
// Desc: Converts a linear slider position to a quasi logrithmic scale
//-----------------------------------------------------------------------------
FLOAT ConvertLinearSliderPosToLogScale( LONG lSliderPos )
{
    if( lSliderPos > 0 && lSliderPos <= 10 )
    {
        return lSliderPos*0.01f;
    }
    else if( lSliderPos > 10 && lSliderPos <= 20 )
    {
        return (lSliderPos-10)*0.1f;
    }
    else if( lSliderPos > 20 && lSliderPos <= 30 )
    {
        return (lSliderPos-20)*1.0f;
    }
    else if( lSliderPos > 30 && lSliderPos <= 40 )
    {
        return (lSliderPos-30)*10.0f;
    }

    return 0.0f;
}




//-----------------------------------------------------------------------------
// Name: ConvertLinearSliderPosToLogScale()
// Desc: Converts a quasi logrithmic scale to a slider position
//-----------------------------------------------------------------------------
LONG ConvertLogScaleToLinearSliderPosTo( FLOAT fValue )
{
    if( fValue > 0.0f && fValue <= 0.1f )
    {
        return (LONG)(fValue/0.01f);
    }
    else if( fValue > 0.1f && fValue <= 1.0f )
    {
        return (LONG)(fValue/0.1f) + 10;
    }
    else if( fValue > 1.0f && fValue <= 10.0f )
    {
        return (LONG)(fValue/1.0f) + 20;
    }
    else if( fValue > 10.0f && fValue <= 100.0f )
    {
        return (LONG)(fValue/10.0f) + 30;
    }

    return 0;
}







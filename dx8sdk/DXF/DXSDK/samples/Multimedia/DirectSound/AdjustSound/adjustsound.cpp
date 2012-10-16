//----------------------------------------------------------------------------
// File: AdjustSound.cpp
//
// Desc: AdjustSound sample sample shows how to load and play a wave file using
//       a DirectSound buffer and adjust its focus, frequency, pan, and volume.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dxerr8.h>
#include <dsound.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dsound.h>
#include "resource.h"
#include "DSUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg,  WPARAM wParam, LPARAM lParam );

VOID    OnInitDialog( HWND hDlg );
VOID    OnTimer( HWND hDlg );

VOID    OnOpenSoundFile( HWND hDlg );
VOID    ValidateWaveFile( HWND hDlg, TCHAR* strFileName );

HRESULT OnPlaySound( HWND hDlg );
HRESULT CreateAndFillBuffer( HWND hDlg, DWORD dwCreationFlags );

VOID    UpdateBehaviorText( HWND hDlg );
VOID    OnSliderChanged( HWND hDlg );
VOID    SetBufferOptions( LONG lFrequency, LONG lPan, LONG lVolume );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
VOID    SetSlidersPos( HWND hDlg, LONG lFreqSlider, LONG lPanSlider, LONG lVolumeSlider );





//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
TCHAR          m_strWaveFileName[MAX_PATH];
CSoundManager* g_pSoundManager = NULL;
CSound*        g_pSound = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
{
    // Init the common control dll 
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
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                case IDC_SOUNDFILE:
                    OnOpenSoundFile( hDlg );
                    break;

                case IDC_PLAY:
                    if( FAILED( hr = OnPlaySound( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlaySound"), hr );
                        MessageBox( hDlg, "Error playing DirectSound buffer."
                                    "Sample will now exit.", "DirectSound Sample", 
                                    MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, IDABORT );
                    }

                    break;

                case IDC_STOP:
                    if( g_pSound )
                    {
                        g_pSound->Stop();
                        g_pSound->Reset();
                    }
                    break;

                case IDC_MIX_DEFAULT:
                case IDC_MIX_HARDWARE:
                case IDC_MIX_SOFTWARE:
                case IDC_FOCUS_GLOBAL:
                case IDC_FOCUS_STICKY:
                case IDC_FOCUS_NORMAL:
                    UpdateBehaviorText( hDlg );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_TIMER:
            OnTimer( hDlg );
            break;

        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;

        case WM_NOTIFY:
            OnSliderChanged( hDlg );
            break;

        case WM_DESTROY:
            // Cleanup everything
            KillTimer( hDlg, 1 );    
            SAFE_DELETE( g_pSound );
            SAFE_DELETE( g_pSoundManager );
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
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Create a static IDirectSound in the CSound class.  
    // Set coop level to DSSCL_PRIORITY, and set primary buffer 
    // format to stereo, 22kHz and 16-bit output.
    g_pSoundManager = new CSoundManager();

    if( FAILED( hr = g_pSoundManager->Initialize( hDlg, DSSCL_PRIORITY, 2, 22050, 16 ) ) )
    {
        DXTRACE_ERR( TEXT("Initialize"), hr );
        MessageBox( hDlg, "Error initializing DirectSound.  Sample will now exit.", 
                            "DirectSound Sample", MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDABORT );
        return;
    }

    // Set the icon for this dialog.
    PostMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    PostMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Create a timer, so we can check for when the soundbuffer is stopped
    SetTimer( hDlg, 0, 250, NULL );

    // Get handles to dialog items
    HWND hFreqSlider    = GetDlgItem( hDlg, IDC_FREQUENCY_SLIDER );
    HWND hPanSlider     = GetDlgItem( hDlg, IDC_PAN_SLIDER );
    HWND hVolumeSlider  = GetDlgItem( hDlg, IDC_VOLUME_SLIDER );

    // Set the focus to normal by default
    CheckRadioButton( hDlg, IDC_FOCUS_NORMAL, IDC_FOCUS_NORMAL, IDC_FOCUS_NORMAL );

    // Set the buffer mixing to default 
    CheckRadioButton( hDlg, IDC_MIX_DEFAULT, IDC_MIX_SOFTWARE, IDC_MIX_DEFAULT );

    // Set the range and position of the freq slider from 
    // DSBFREQUENCY_MIN and DSBFREQUENCY_MAX are DirectSound constants
    PostMessage( hFreqSlider, TBM_SETRANGEMAX, TRUE, DSBFREQUENCY_MAX );
    PostMessage( hFreqSlider, TBM_SETRANGEMIN, TRUE, DSBFREQUENCY_MIN );

    // Set the range and position of the pan slider from 
    PostMessage( hPanSlider, TBM_SETRANGEMAX, TRUE, ( 10000L/500L) );
    PostMessage( hPanSlider, TBM_SETRANGEMIN, TRUE, (-10000L/500L) );

    // Set the range and position of the volume slider 
    PostMessage( hVolumeSlider, TBM_SETRANGEMAX, TRUE, 0L );
    PostMessage( hVolumeSlider, TBM_SETRANGEMIN, TRUE, (-5000L/100L) );

    // Set the position of the sliders
    SetSlidersPos( hDlg, DSBFREQUENCY_MIN, 0, 0 );

    // Set the UI controls
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("No file loaded.") );
    UpdateBehaviorText( hDlg );
}




//-----------------------------------------------------------------------------
// Name: OnOpenSoundFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenSoundFile( HWND hDlg ) 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("Wave Files\0*.wav\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Sound File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".wav"), 0, NULL, NULL };

    // Get the default media path (something like C:\WINDOWS\MEDIA)
    if( '\0' == strPath[0] )
    {
        GetWindowsDirectory( strPath, MAX_PATH );
        if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ) )
            strcat( strPath, TEXT("\\") );
        strcat( strPath, TEXT("MEDIA") );
    }

    // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_STOP ), FALSE );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Loading file...") );

    if( g_pSound )
    {
        g_pSound->Stop();
        g_pSound->Reset();
    }

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        if( g_pSound )
        {
            EnableWindow( GetDlgItem( hDlg, IDC_PLAY ), TRUE );
            EnableWindow( GetDlgItem( hDlg, IDC_STOP ), TRUE );
        }

        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Load aborted.") );
        return;
    }

    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

    // Make sure wave file is a valid wav file
    ValidateWaveFile( hDlg, strFileName );

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}




//-----------------------------------------------------------------------------
// Name: ValidateWaveFile()
// Desc: Open the wave file with the helper 
//       class CWaveFile to make sure it is valid
//-----------------------------------------------------------------------------
VOID ValidateWaveFile( HWND hDlg, TCHAR* strFileName )
{
    HRESULT hr;

    CWaveFile waveFile;

    // Load the wave file
    if( FAILED( hr = waveFile.Open( strFileName, NULL, WAVEFILE_READ ) ) )
    {
        DXTRACE_ERR( TEXT("Open"), hr );
        waveFile.Close();
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Bad wave file.") );
    }
    else // The load call succeeded
    {
        // Update the UI controls to show the sound as the file is loaded
        waveFile.Close();

        EnablePlayUI( hDlg, TRUE );
        SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("File loaded.") );
        strcpy( m_strWaveFileName, strFileName );

        // Get the samples per sec from the wave file
        DWORD dwSamplesPerSec = waveFile.m_pwfx->nSamplesPerSec;

        // Set the slider positions
        SetSlidersPos( hDlg, dwSamplesPerSec, 0, 0 );
    }
}




//-----------------------------------------------------------------------------
// Name: OnPlaySound()
// Desc: User hit the "Play" button
//-----------------------------------------------------------------------------
HRESULT OnPlaySound( HWND hDlg ) 
{
    HRESULT hr;
    DWORD dwCreationFlags;

    BOOL bLooped      = ( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK )      == BST_CHECKED );
    BOOL bFocusSticky = ( IsDlgButtonChecked( hDlg, IDC_FOCUS_STICKY )    == BST_CHECKED );
    BOOL bFocusGlobal = ( IsDlgButtonChecked( hDlg, IDC_FOCUS_GLOBAL )    == BST_CHECKED );
    BOOL bMixHardware = ( IsDlgButtonChecked( hDlg, IDC_MIX_HARDWARE ) == BST_CHECKED );
    BOOL bMixSoftware = ( IsDlgButtonChecked( hDlg, IDC_MIX_SOFTWARE ) == BST_CHECKED );

    // Detrimine the creation flags to use based on the radio buttons
    dwCreationFlags = 0;
    if( bFocusGlobal )
        dwCreationFlags |= DSBCAPS_GLOBALFOCUS;

    if( bFocusSticky )
        dwCreationFlags |= DSBCAPS_STICKYFOCUS;

    if( bMixHardware )
        dwCreationFlags |= DSBCAPS_LOCHARDWARE;

    if( bMixSoftware )
        dwCreationFlags |= DSBCAPS_LOCSOFTWARE;

    // Add extra flags needed for the UI
    dwCreationFlags |= DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;

    // Free any previous sound
    SAFE_DELETE( g_pSound );

    // Since the user can change the focus before the sound is played, 
    // we need to create the sound buffer every time the play button is pressed 

    // Load the wave file into a DirectSound buffer
    if( FAILED( hr = g_pSoundManager->Create( &g_pSound, m_strWaveFileName, dwCreationFlags, GUID_NULL ) ) )
    {
        // Not a critical failure, so just update the status
        DXTRACE_ERR_NOMSGBOX( TEXT("Create"), hr );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Could not create sound buffer.") );
        return S_FALSE; 
    }

    // Set the buffer options to what the sliders are set to
    OnSliderChanged( hDlg );

    // Only if the sound buffer was created perfectly should we update the UI 
    // and play the sound
    // Play the sound
    DWORD dwLooped = bLooped ? DSBPLAY_LOOPING : 0L;

    if( FAILED( hr = g_pSound->Play( 0, dwLooped ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

    // Update the UI controls to show the sound as playing
    EnablePlayUI( hDlg, FALSE );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Sound playing.") );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnTimer()
// Desc: When we think the sound is playing this periodically checks to see if 
//       the sound has stopped.  If it has then updates the dialog.
//-----------------------------------------------------------------------------
VOID OnTimer( HWND hDlg ) 
{
    if( IsWindowEnabled( GetDlgItem( hDlg, IDC_STOP ) ) )
    {
        // We think the sound is playing, so see if it has stopped yet.
        if( !g_pSound->IsSoundPlaying() ) 
        {
            // Update the UI controls to show the sound as stopped
            EnablePlayUI( hDlg, TRUE );
            SetDlgItemText( hDlg, IDC_STATUS, TEXT("Sound stopped.") );
        }
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

    // Get handles to dialog items
    HWND hFreqSlider   = GetDlgItem( hDlg, IDC_FREQUENCY_SLIDER );
    HWND hPanSlider    = GetDlgItem( hDlg, IDC_PAN_SLIDER );
    HWND hVolumeSlider = GetDlgItem( hDlg, IDC_VOLUME_SLIDER );

    // Get the position of the sliders
    LONG lFrequency = (LONG)SendMessage( hFreqSlider,   TBM_GETPOS, 0, 0 ) * 1L;
    LONG lPan       = (LONG)SendMessage( hPanSlider,    TBM_GETPOS, 0, 0 ) * 500L;
    LONG lVolume    = (LONG)SendMessage( hVolumeSlider, TBM_GETPOS, 0, 0 ) * 100L;

    // Set the static text boxes
    wsprintf( strBuffer, TEXT("%ld"), lFrequency );
    SetWindowText( GetDlgItem( hDlg, IDC_FREQUENCY ), strBuffer );

    wsprintf( strBuffer, TEXT("%ld"), lPan );
    SetWindowText( GetDlgItem( hDlg, IDC_PAN       ), strBuffer );

    wsprintf( strBuffer, TEXT("%ld"), lVolume );
    SetWindowText( GetDlgItem( hDlg, IDC_VOLUME    ), strBuffer );

    // Set the options in the DirectSound buffer
    if( g_pSound )
    {
        LPDIRECTSOUNDBUFFER pDSB = g_pSound->GetBuffer( 0 );

        if( pDSB )
        {
            pDSB->SetFrequency( lFrequency );
            pDSB->SetPan( lPan );
            pDSB->SetVolume( lVolume );
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateBehaviorText()
// Desc: Figure out what the expected behavoir is based on the dialog,
//       and display it on the dialog
//-----------------------------------------------------------------------------
VOID UpdateBehaviorText( HWND hDlg )
{
    TCHAR strExcepted[1024];

    BOOL bLooped      = ( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK )   == BST_CHECKED );
    BOOL bFocusSticky = ( IsDlgButtonChecked( hDlg, IDC_FOCUS_STICKY ) == BST_CHECKED );
    BOOL bFocusGlobal = ( IsDlgButtonChecked( hDlg, IDC_FOCUS_GLOBAL ) == BST_CHECKED );
    BOOL bMixHardware = ( IsDlgButtonChecked( hDlg, IDC_MIX_HARDWARE ) == BST_CHECKED );
    BOOL bMixSoftware = ( IsDlgButtonChecked( hDlg, IDC_MIX_SOFTWARE ) == BST_CHECKED );

    // Figure what the user should expect based on the dialog choice
    if( bFocusSticky )
    {
        strcpy( strExcepted, "Buffers with \"sticky\" focus will continue to play "
                             "if the user switches to another application not using "
                             "DirectSound.  However, if the user switches to another "
                             "DirectSound application, all normal-focus and sticky-focus "
                             "buffers in the previous application are muted." ); 

    }
    else if( bFocusGlobal )
    {
        strcpy( strExcepted, "Buffers with global focus will continue to play if the user "
                             "switches focus to another application, even if the new application "
                             "uses DirectSound. The one exception is if you switch focus to a "
                             "DirectSound application that uses the DSSCL_WRITEPRIMARY "
                             "cooperative level. In this case, the global-focus buffers from "
                             "other applications will not be audible." );
    }
    else
    {
        // Normal focus
        strcpy( strExcepted, "Buffers with normal focus will mute if the user switches "
                             "focus to any other application" );
    }


    if( bMixHardware )
    {
        strcat( strExcepted, "\n\nWith the hardware mixing flag, the new buffer will "
                             "be forced to use hardware mixing. If the device does "
                             "not support hardware mixing or if the required "
                             "hardware resources are not available, the call to the "
                             "IDirectSound::CreateSoundBuffer method will fail." ); 
    }
    else if( bMixSoftware )
    {
        strcat( strExcepted, "\n\nWith the software mixing flag, the new buffer will use "
                             "software mixing, even if hardware resources are available." );
    }
    else 
    {
        // Default mixing
        strcat( strExcepted, "\n\nWith default mixing, the new buffer will use hardware "
                             "mixing if available, otherwise software mixing will be used." ); 
    }

    // Tell the user what to expect
    SetDlgItemText( hDlg, IDC_BEHAVIOR, strExcepted );
}




//-----------------------------------------------------------------------------
// Name: EnablePlayUI()
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK      ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY            ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_STOP            ), FALSE );

        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_NORMAL    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_STICKY    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_GLOBAL    ), TRUE );

        EnableWindow( GetDlgItem( hDlg, IDC_MIX_DEFAULT  ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_MIX_HARDWARE ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_MIX_SOFTWARE ), TRUE );

        SetFocus(     GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK      ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY            ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_STOP            ), TRUE );

        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_NORMAL    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_STICKY    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOCUS_GLOBAL    ), FALSE );

        EnableWindow( GetDlgItem( hDlg, IDC_MIX_DEFAULT  ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_MIX_HARDWARE ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_MIX_SOFTWARE ), FALSE );

        SetFocus(     GetDlgItem( hDlg, IDC_STOP ) );
    }
}




//-----------------------------------------------------------------------------
// Name: SetSlidersPos()
// Desc: Sets the slider positions
//-----------------------------------------------------------------------------
VOID SetSlidersPos( HWND hDlg, LONG lFreqSlider, LONG lPanSlider, LONG lVolumeSlider )
{
    HWND hFreqSlider    = GetDlgItem( hDlg, IDC_FREQUENCY_SLIDER );
    HWND hPanSlider     = GetDlgItem( hDlg, IDC_PAN_SLIDER );
    HWND hVolumeSlider  = GetDlgItem( hDlg, IDC_VOLUME_SLIDER );

    PostMessage( hFreqSlider,   TBM_SETPOS, TRUE, lFreqSlider );
    PostMessage( hPanSlider,    TBM_SETPOS, TRUE, lPanSlider );
    PostMessage( hVolumeSlider, TBM_SETPOS, TRUE, lVolumeSlider );
}




//----------------------------------------------------------------------------
// File: AmplitudeModulation.cpp
//
// Desc: AmplitudeModulation sample shows how to create an effect buffer and 
//       adjust amplitude modulation parameters.
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
#include <cguid.h>
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
HRESULT ValidateWaveFile( HWND hDlg, TCHAR* strFileName );
HRESULT OnPlaySound( HWND hDlg );
HRESULT CreateAndFillBuffer( HWND hDlg, DWORD dwCreationFlags );
VOID    OnEffectChanged( HWND hDlg );
VOID    SetBufferOptions( LONG lFrequency, LONG lPan, LONG lVolume );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define DSFX_GARGLE_RATEHZ_MAX 1000
#define DSFX_GARGLE_RATEHZ_MIN 1

TCHAR                       g_strWaveFileName[MAX_PATH];
CSoundManager*              g_pSoundManager     = NULL;
CSound*                     g_pSound            = NULL;
LPDIRECTSOUNDFXGARGLE       g_pIGargle          = NULL;
HINSTANCE                   g_hInst             = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
{
    g_hInst = hInst;

    CoInitialize( NULL );

    // Init the common control dll 
    InitCommonControls();

    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    CoUninitialize();

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
                    SAFE_RELEASE( g_pIGargle );
                    if( g_pSound )
                    {
                        g_pSound->Stop();
                        g_pSound->Reset();
                    }
                    break;

                case IDC_WAVEFORM_TRIANGLE:
                case IDC_WAVEFORM_SQUARE:
                    OnEffectChanged( hDlg );
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
            OnEffectChanged( hDlg );
            break;

        case WM_DESTROY:
            // Cleanup everything
            KillTimer( hDlg, 1 );
            SAFE_RELEASE( g_pIGargle );
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
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

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

    // Set UI defaults 
    CheckDlgButton( hDlg, IDC_LOOP_CHECK, BST_CHECKED );
    CheckRadioButton( hDlg, IDC_WAVEFORM_TRIANGLE, IDC_WAVEFORM_TRIANGLE, IDC_WAVEFORM_TRIANGLE );

    // Set the range and position of the freq slider from 
    // DSBFREQUENCY_MIN and DSBFREQUENCY_MAX are DirectSound constants
    PostMessage( hFreqSlider, TBM_SETRANGEMAX, TRUE, DSFX_GARGLE_RATEHZ_MAX );
    PostMessage( hFreqSlider, TBM_SETRANGEMIN, TRUE, DSFX_GARGLE_RATEHZ_MIN );
    PostMessage( hFreqSlider, TBM_SETPOS, TRUE, DSFX_GARGLE_RATEHZ_MIN );

    // Load default wave file
    TCHAR strFile[MAX_PATH];
    GetWindowsDirectory( strFile, MAX_PATH );
    lstrcat( strFile, "\\media\\ding.wav" );
    
    if( FAILED( hr = ValidateWaveFile( hDlg, strFile ) ) )
    {
        // Set the UI controls
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("No file loaded.") );
    }
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

    SAFE_RELEASE( g_pIGargle );
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
HRESULT ValidateWaveFile( HWND hDlg, TCHAR* strFileName )
{
    HRESULT hr;
    CWaveFile waveFile;

    if( -1 == GetFileAttributes(strFileName) )
        return E_FAIL;
    
    // Load the wave file
    if( FAILED( hr = waveFile.Open( strFileName, NULL, WAVEFILE_READ ) ) )
    {        
        waveFile.Close();
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Bad wave file.") );
        return DXTRACE_ERR( TEXT("Open"), hr );
    }
    else // The load call succeeded
    {
        WAVEFORMATEX* pwfx = waveFile.GetFormat();
        if( pwfx->wFormatTag != WAVE_FORMAT_PCM )
        {
            // Sound must be PCM when using DSBCAPS_CTRLFX            
            SAFE_RELEASE( g_pIGargle );
            SAFE_DELETE( g_pSound );
            SetDlgItemText( hDlg, IDC_STATUS, TEXT("Wave file must be PCM for effects control.") );
            SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
            return S_FALSE;
        }
  
        // Update the UI controls to show the sound as the file is loaded
        waveFile.Close();

        EnablePlayUI( hDlg, TRUE );
        SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("File loaded.") );
        strcpy( g_strWaveFileName, strFileName );

        // Get the samples per sec from the wave file
        DWORD dwSamplesPerSec = waveFile.m_pwfx->nSamplesPerSec;
        return S_OK;
    }
}




//-----------------------------------------------------------------------------
// Name: OnPlaySound()
// Desc: User hit the "Play" button
//-----------------------------------------------------------------------------
HRESULT OnPlaySound( HWND hDlg ) 
{
    HRESULT                 hr;
    DWORD                   dwCreationFlags;
    DWORD                   dwResults;

    LPDIRECTSOUNDBUFFER  pDSB = NULL;
    LPDIRECTSOUNDBUFFER8 pDSB8 = NULL;

    BOOL bLooped        = ( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK )     == BST_CHECKED );

    // We would only use CTRLFX control on dsound buffer
    dwCreationFlags = DSBCAPS_CTRLFX;

    // Free any previous sound and FXs
    SAFE_RELEASE( g_pIGargle );
    SAFE_DELETE( g_pSound );

    // Since the user can change the focus before the sound is played, 
    // we need to create the sound buffer every time the play button is pressed 

    // Load the wave file into a DirectSound buffer
    if( FAILED( hr = g_pSoundManager->Create( &g_pSound, g_strWaveFileName, dwCreationFlags, GUID_NULL ) ) )
    {
        // Not a critical failure, so just update the status
        DXTRACE_ERR_NOMSGBOX( TEXT("Create"), hr );
        if( hr == DSERR_BUFFERTOOSMALL )
        {
            // DSERR_BUFFERTOOSMALL will be returned if the buffer is
            // less than DSBSIZE_FX_MIN (100ms) and the buffer is created
            // with DSBCAPS_CTRLFX.                           
            SetDlgItemText( hDlg, IDC_STATUS, TEXT("Wave file is too short (less than 100ms) for effect processing.") );
        }
        else
        {
            SetDlgItemText( hDlg, IDC_STATUS, TEXT("Could not create sound buffer.") );
        }
        
        return S_FALSE; 
    }

    // Query IDirectSoundBuffer8 interface
    pDSB = g_pSound->GetBuffer( 0 );
    if( FAILED( hr = pDSB->QueryInterface( IID_IDirectSoundBuffer8, (LPVOID*) &pDSB8 ) ) )
        return DXTRACE_ERR( TEXT("QueryInterface"), hr );

    // Set gargle effect on the IDirectSoundBuffer8
    DSEFFECTDESC dsed;
    ZeroMemory( &dsed, sizeof(DSEFFECTDESC) );
    dsed.dwSize       = sizeof(DSEFFECTDESC);
    dsed.dwFlags      = 0;
    dsed.guidDSFXClass = GUID_DSFX_STANDARD_GARGLE;

    if( FAILED( hr = pDSB8->SetFX( 1, &dsed, &dwResults ) ) )
    {
        // Not a critical failure, so just update the status
        DXTRACE_ERR( TEXT("SetFX"), hr );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Could not set gargle effect.") );
        return S_FALSE;
    }

    // Get gargle effect friendly interface
    if( FAILED( hr = pDSB8->GetObjectInPath( GUID_DSFX_STANDARD_GARGLE, 0, 
                                             IID_IDirectSoundFXGargle, 
                                             (LPVOID*) &g_pIGargle ) ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Cleanup
    SAFE_RELEASE( pDSB8 );

    // Set the buffer options to what the sliders are set to
    OnEffectChanged( hDlg );

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
// Name: OnEffectChanged()  
// Desc: Called when the UI prompted an effect change
//-----------------------------------------------------------------------------
VOID OnEffectChanged( HWND hDlg )
{
    DSFXGargle dsfxGargle;

    // Get handles to dialog items
    HWND hFreqSlider = GetDlgItem( hDlg, IDC_FREQUENCY_SLIDER );

    // Get the position of the sliders
    dsfxGargle.dwRateHz = (DWORD)SendMessage( hFreqSlider, TBM_GETPOS, 0, 0 );

    // Update UI
    TCHAR strBuffer[10];
    wsprintf( strBuffer, TEXT("%ld"), dsfxGargle.dwRateHz );
    SetDlgItemText( hDlg, IDC_FREQUENCY, strBuffer );

    // Get wave form
    if( IsDlgButtonChecked( hDlg, IDC_WAVEFORM_SQUARE ) == BST_CHECKED )
        dsfxGargle.dwWaveShape = DSFXGARGLE_WAVE_SQUARE;
    else 
        dsfxGargle.dwWaveShape = DSFXGARGLE_WAVE_TRIANGLE;

    // Set the options in the DirectSound buffer
    if( g_pSound && g_pIGargle )
    {
        g_pIGargle->SetAllParameters( &dsfxGargle );
    }
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
        SetFocus(     GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK      ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY            ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_STOP            ), TRUE );
        SetFocus(     GetDlgItem( hDlg, IDC_STOP ) );
    }
}







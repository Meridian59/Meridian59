//----------------------------------------------------------------------------
// File: Play3DSound.cpp
//
// Desc: Main application file for the Play3DSound sample. This sample shows how
//       to load a wave file and play it using a 3D DirectSound buffer.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commctrl.h>
#include <commdlg.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <dxerr8.h>
#include <dsound.h>
#include <math.h>
#include <stdio.h>
#include "resource.h"
#include "DSUtil.h"
#include "DXUtil.h"



//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
VOID    OnInitDialog( HWND hDlg );
HRESULT InitDirectSound( HWND hDlg );
HRESULT FreeDirectSound();
VOID    SetSlidersPos( HWND hDlg, FLOAT fDopplerValue, FLOAT fRolloffValue, FLOAT fMinDistValue, FLOAT fMaxDistValue );
VOID    OnOpenSoundFile( HWND hDlg );
VOID    LoadWaveFileIntoBuffer( HWND hDlg, TCHAR* strFileName );
INT_PTR CALLBACK AlgorithmDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnPlaySound( HWND hDlg );
VOID    OnSliderChanged( HWND hDlg );
VOID    Set3DParameters( FLOAT fDopplerFactor, FLOAT fRolloffFactor, FLOAT fMinDistance,   FLOAT fMaxDistance );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
VOID    OnMovementTimer( HWND hDlg );
VOID    UpdateGrid( HWND hDlg, FLOAT x, FLOAT y );
VOID    SetObjectProperties( D3DVECTOR* pvPosition, D3DVECTOR* pvVelocity );
FLOAT   ConvertLinearSliderPosToLogScale( LONG lSliderPos );
LONG    ConvertLogScaleToLinearSliderPosTo( FLOAT fValue );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define ORBIT_MAX_RADIUS        5.0f
#define IDT_MOVEMENT_TIMER      1

CSoundManager*          g_pSoundManager       = NULL;
CSound*                 g_pSound              = NULL;
LPDIRECTSOUND3DBUFFER   g_pDS3DBuffer         = NULL;   // 3D sound buffer
LPDIRECTSOUND3DLISTENER g_pDSListener         = NULL;   // 3D listener object
DS3DBUFFER              g_dsBufferParams;               // 3D buffer properties
DS3DLISTENER            g_dsListenerParams;             // Listener properties
BOOL                    g_bDeferSettings      = FALSE;
BOOL                    g_bAllowMovementTimer = TRUE;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
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
        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SOUNDFILE:
                    OnOpenSoundFile( hDlg );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
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

                    // Update the UI controls to show the sound as stopped
                    EnablePlayUI( hDlg, TRUE );
                    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Sound stopped.") );
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
            KillTimer( hDlg, 1 );    
            SAFE_RELEASE( g_pDSListener );
            SAFE_RELEASE( g_pDS3DBuffer );

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

    hr = g_pSoundManager->Initialize( hDlg, DSSCL_PRIORITY, 2, 22050, 16 );

    // Get the 3D listener, so we can control its params
    hr |= g_pSoundManager->Get3DListenerInterface( &g_pDSListener );

    if( FAILED(hr) )
    {
        DXTRACE_ERR( TEXT("Get3DListenerInterface"), hr );
        MessageBox( hDlg, "Error initializing DirectSound.  Sample will now exit.", 
                            "DirectSound Sample", MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDABORT );
        return;
    }

    // Get listener parameters
    g_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
    g_pDSListener->GetAllParameters( &g_dsListenerParams );

    // Set the icon for this dialog.
    PostMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    PostMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Create a timer to periodically move the 3D object around
    SetTimer( hDlg, IDT_MOVEMENT_TIMER, 0, NULL );

    // Set the UI controls
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("No file loaded.") );

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
    PostMessage( hVertSlider,    TBM_SETPOS,      TRUE, 100L );

    PostMessage( hHorzSlider,    TBM_SETRANGEMAX, TRUE, 100L );
    PostMessage( hHorzSlider,    TBM_SETRANGEMIN, TRUE, -100L );
    PostMessage( hHorzSlider,    TBM_SETPOS,      TRUE, 100L );

    // Set the position of the sliders
    SetSlidersPos( hDlg, 0.0f, 0.0f, ORBIT_MAX_RADIUS, ORBIT_MAX_RADIUS*2.0f );
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

    PostMessage( hDopplerSlider, TBM_SETPOS, TRUE, lDopplerSlider );
    PostMessage( hRolloffSlider, TBM_SETPOS, TRUE, lRolloffSlider );
    PostMessage( hMinDistSlider, TBM_SETPOS, TRUE, lMinDistSlider );
    PostMessage( hMaxDistSlider, TBM_SETPOS, TRUE, lMaxDistSlider );
}




//-----------------------------------------------------------------------------
// Name: OnOpenSoundFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenSoundFile( HWND hDlg ) 
{
    GUID    guid3DAlgorithm = GUID_NULL;
    int     nResult;
    HRESULT hr; 

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

    if( g_pSound )
    {
        g_pSound->Stop();
        g_pSound->Reset();
    }

    // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY ), FALSE);
    EnableWindow( GetDlgItem( hDlg, IDC_STOP ), FALSE);
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Loading file...") );

    // Stop the timer while dialogs are displayed
    g_bAllowMovementTimer = FALSE;

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Load aborted.") );
        g_bAllowMovementTimer = TRUE;
        return;
    }

    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

    // Free any previous sound, and make a new one
    SAFE_DELETE( g_pSound );

    CWaveFile waveFile;
    waveFile.Open( strFileName, NULL, WAVEFILE_READ );
    WAVEFORMATEX* pwfx = waveFile.GetFormat();
    if( pwfx == NULL )
    {
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Invalid wave file format.") );
        return;
    }

    if( pwfx->nChannels > 1 )
    {
        // Too many channels in wave.  Sound must be mono when using DSBCAPS_CTRL3D
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Wave file must be mono for 3D control.") );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        return;
    }

    if( pwfx->wFormatTag != WAVE_FORMAT_PCM )
    {
        // Sound must be PCM when using DSBCAPS_CTRL3D
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Wave file must be PCM for 3D control.") );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        return;
    }

    // Get the software DirectSound3D emulation algorithm to use
    // Ask the user for this sample, so display the algorithm dialog box.
    nResult = (int)DialogBox( NULL, MAKEINTRESOURCE(IDD_3D_ALGORITHM), 
                              NULL, AlgorithmDlgProc );
    switch( nResult )
    {
    case -1: // User canceled dialog box
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Load aborted.") );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        return;

    case 0: // User selected DS3DALG_NO_VIRTUALIZATION  
        guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
        break;

    case 1: // User selected DS3DALG_HRTF_FULL  
        guid3DAlgorithm = DS3DALG_HRTF_FULL;
        break;

    case 2: // User selected DS3DALG_HRTF_LIGHT
        guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
        break;
    }

    // Load the wave file into a DirectSound buffer
    hr = g_pSoundManager->Create( &g_pSound, strFileName, DSBCAPS_CTRL3D, guid3DAlgorithm );  
    if( FAILED( hr ) || hr == DS_NO_VIRTUALIZATION )
    {
        DXTRACE_ERR_NOMSGBOX( TEXT("Create"), hr );
        if( DS_NO_VIRTUALIZATION == hr )
        {
            MessageBox( hDlg, "The 3D virtualization algorithm requested is not supported under this "
                        "operating system.  It is available only on Windows 2000, Windows ME, and Windows 98 with WDM "
                        "drivers and beyond.  Creating buffer with no virtualization.", 
                        "DirectSound Sample", MB_OK );
        }

        // Unknown error, but not a critical failure, so just update the status
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create sound buffer.") );
        return; 
    }

    // Get the 3D buffer from the secondary buffer
    if( FAILED( hr = g_pSound->Get3DBufferInterface( 0, &g_pDS3DBuffer ) ) )
    {
        DXTRACE_ERR( TEXT("Get3DBufferInterface"), hr );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Could not get 3D buffer.") );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        return;
    }

    // Get the 3D buffer parameters
    g_dsBufferParams.dwSize = sizeof(DS3DBUFFER);
    g_pDS3DBuffer->GetAllParameters( &g_dsBufferParams );

    // Set new 3D buffer parameters
    g_dsBufferParams.dwMode = DS3DMODE_HEADRELATIVE;
    g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_IMMEDIATE );

    DSBCAPS dsbcaps;
    ZeroMemory( &dsbcaps, sizeof(DSBCAPS) );
    dsbcaps.dwSize = sizeof(DSBCAPS);

    LPDIRECTSOUNDBUFFER pDSB = g_pSound->GetBuffer( 0 );
    pDSB->GetCaps( &dsbcaps );
    if( ( dsbcaps.dwFlags & DSBCAPS_LOCHARDWARE ) != 0 )
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("File loaded using hardware mixing.") );
    else
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("File loaded using software mixing.") );

    // Update the UI controls to show the sound as the file is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    EnablePlayUI( hDlg, TRUE );

    g_bAllowMovementTimer = TRUE;

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';

    // Set the slider positions
    SetSlidersPos( hDlg, 0.0f, 0.0f, ORBIT_MAX_RADIUS, ORBIT_MAX_RADIUS*2.0f );
    OnSliderChanged( hDlg );
}




//-----------------------------------------------------------------------------
// Name: AlgorithmDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK AlgorithmDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    // Default is DS3DALG_NO_VIRTUALIZATION for fastest performance
    static int nDefaultRadio = IDC_NO_VIRT_RADIO;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            // Default is DS3DALG_NO_VIRTUALIZATION for fastest performance
            CheckRadioButton( hDlg, IDC_NO_VIRT_RADIO, IDC_LIGHT_VIRT_RADIO, nDefaultRadio );
            return TRUE; // Message handled 

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, -1 );
                    return TRUE; // Message handled 

                case IDOK:
                    if( IsDlgButtonChecked( hDlg, IDC_NO_VIRT_RADIO )    == BST_CHECKED )
                    {
                        nDefaultRadio = IDC_NO_VIRT_RADIO;
                        EndDialog( hDlg, 0 );
                    }

                    if( IsDlgButtonChecked( hDlg, IDC_HIGH_VIRT_RADIO )  == BST_CHECKED )
                    {               
                        nDefaultRadio = IDC_HIGH_VIRT_RADIO;
                        EndDialog( hDlg, 1 );
                    }

                    if( IsDlgButtonChecked( hDlg, IDC_LIGHT_VIRT_RADIO ) == BST_CHECKED )
                    {               
                        nDefaultRadio = IDC_LIGHT_VIRT_RADIO;
                        EndDialog( hDlg, 2 );
                    }
                        
                    return TRUE; // Message handled 
            }
            break;
    }

    return FALSE; // Message not handled 
}




//-----------------------------------------------------------------------------
// Name: OnPlaySound()
// Desc: User hit the "Play" button
//-----------------------------------------------------------------------------
HRESULT OnPlaySound( HWND hDlg ) 
{
    HRESULT hr;

    if( NULL == g_pSound )
        return E_FAIL;

    // Play buffer always in looped mode just for this sample
    if( FAILED( hr = g_pSound->Play( 0, DSBPLAY_LOOPING ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

    // Update the UI controls to show the sound as playing
    EnablePlayUI( hDlg, FALSE );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Sound playing.") );

    return S_OK;
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
// Name: EnablePlayUI()
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
        EnableWindow(   GetDlgItem( hDlg, IDC_STOP ),       FALSE );
        SetFocus(       GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ),       FALSE );
        EnableWindow(  GetDlgItem( hDlg, IDC_STOP ),       TRUE );
        SetFocus(      GetDlgItem( hDlg, IDC_STOP ) );
    }
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
    static LONG s_lPixel[5] = { CLR_INVALID,CLR_INVALID,CLR_INVALID,CLR_INVALID,CLR_INVALID };
    static LONG s_lX = 0;
    static LONG s_lY = 0;

    HWND hWndGrid = GetDlgItem( hDlg, IDC_RENDER_WINDOW );
    HDC  hDC      = GetDC( hWndGrid );
    RECT rc;

    // Don't update the grid if a WM_PAINT will be called soon
    BOOL bUpdateInProgress = GetUpdateRect(hDlg,NULL,FALSE);
    if( bUpdateInProgress )
        return;

    if( s_lPixel[0] != CLR_INVALID ) 
    {
        // Replace pixels from that were overdrawn last time
        SetPixel( hDC, s_lX-1, s_lY+0, s_lPixel[0] );
        SetPixel( hDC, s_lX+0, s_lY-1, s_lPixel[1] );
        SetPixel( hDC, s_lX+0, s_lY+0, s_lPixel[2] );
        SetPixel( hDC, s_lX+0, s_lY+1, s_lPixel[3] );
        SetPixel( hDC, s_lX+1, s_lY+0, s_lPixel[4] );   
    }

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
        g_pDS3DBuffer->SetAllParameters( &g_dsBufferParams, DS3D_IMMEDIATE );
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



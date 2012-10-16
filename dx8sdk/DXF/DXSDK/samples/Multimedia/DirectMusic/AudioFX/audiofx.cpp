//-----------------------------------------------------------------------------
// File: AudioFX.cpp
//
// Desc: Sample to demonstrate soundFX and parameters with DirectMusic
//
//
// Copyright ( c ) 1998-2001 Microsoft Corporation. All rights reserved.
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
#include <stdio.h>
#include <tchar.h>

#include "DMUtil.h"
#include "DSUtil.h"
#include "DXUtil.h"
#include "resource.h"





//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg,  WPARAM wParam, LPARAM lParam );
VOID    OnInitDialog( HWND hDlg );
VOID    OnOpenSoundFile( HWND hDlg );
HRESULT OnPlaySound( HWND hDlg );
VOID    OnEffectChanged( HWND hDlg );
HRESULT ValidateFile( HWND hDlg, TCHAR* strFileName );
HRESULT CreateAndFillBuffer( HWND hDlg, DWORD dwCreationFlags );
VOID    SetBufferOptions( LONG lFrequency, LONG lPakn, LONG lVolume );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
HRESULT ProcessDirectMusicMessages( HWND hDlg );
VOID    LoadParameterUI ( HWND hDlg, DWORD dwFXType );
VOID    ResetParameterUI ( HWND hDlg );




//-----------------------------------------------------------------------------
// Name: enum ESFXType
// Desc: each is a unique identifier mapped to a DirectSoundFX
//-----------------------------------------------------------------------------
enum ESFXType
{
    eSFX_chorus = 0,
    eSFX_compressor,
    eSFX_distortion,
    eSFX_echo,
    eSFX_flanger,
    eSFX_gargle,
    eSFX_parameq,
    eSFX_reverb,

    // number of enumerated effects
    eNUM_SFX
};




//-----------------------------------------------------------------------------
// Name: class CSoundFXManager
// Desc: Takes care of effects for one DirectSoundBuffer
//-----------------------------------------------------------------------------
class CSoundFXManager
{
public:
    CSoundFXManager();
    ~CSoundFXManager();

public: // interface
    HRESULT Initialize ( IDirectMusicAudioPath8 * pAudioPath8, BOOL bLoadDefaultParamValues );
    HRESULT UnInitialize ();

    HRESULT SetFXEnable( DWORD esfxType );
    HRESULT ActivateFX();
    HRESULT DisableAllFX();
    HRESULT LoadCurrentFXParameters();

public: // members
    LPDIRECTSOUNDFXCHORUS8      m_lpChorus;
    LPDIRECTSOUNDFXCOMPRESSOR8  m_lpCompressor;
    LPDIRECTSOUNDFXDISTORTION8  m_lpDistortion;
    LPDIRECTSOUNDFXECHO8        m_lpEcho;
    LPDIRECTSOUNDFXFLANGER8     m_lpFlanger;
    LPDIRECTSOUNDFXGARGLE8      m_lpGargle;
    LPDIRECTSOUNDFXPARAMEQ8     m_lpParamEq;
    LPDIRECTSOUNDFXWAVESREVERB8 m_lpReverb;

    DSFXChorus                  m_paramsChorus;
    DSFXCompressor              m_paramsCompressor;
    DSFXDistortion              m_paramsDistortion;
    DSFXEcho                    m_paramsEcho;
    DSFXFlanger                 m_paramsFlanger;
    DSFXGargle                  m_paramsGargle;
    DSFXParamEq                 m_paramsParamEq;
    DSFXWavesReverb             m_paramsReverb;

    LPDIRECTSOUNDBUFFER8        m_lpDSB8;
    IDirectMusicAudioPath8 *    m_pAudioPath;

protected:
    DSEFFECTDESC                m_rgFxDesc[eNUM_SFX];
    const GUID *                m_rgRefGuids[eNUM_SFX];
    LPVOID *                    m_rgPtrs[eNUM_SFX];
    BOOL                        m_rgLoaded[eNUM_SFX];
    DWORD                       m_dwNumFX;

    HRESULT EnableGenericFX( GUID guidSFXClass, REFGUID rguidInterface, LPVOID * ppObj );
    HRESULT LoadDefaultParamValues();
};




//-----------------------------------------------------------------------------
// defines and global variables
//-----------------------------------------------------------------------------
#define             DEFAULT_SLIDER_MIN          1
#define             DEFAULT_SLIDER_MAX          0x7FFFFF
#define             DEFAULT_SLIDER_INC          DEFAULT_SLIDER_MAX >> 11

CMusicManager *     g_lpMusicManager        = NULL;
CMusicSegment *     g_pSegment              = NULL;
CSoundFXManager *   g_lpFXManager           = NULL;
HINSTANCE           g_hInst                 = NULL;
TCHAR               g_strFileName[MAX_PATH];
DWORD               g_dwCurrentFXType       = eSFX_chorus;
HANDLE              g_hDMusicMessageEvent   = NULL;

const TCHAR *       g_strFXNames[] = { "Chorus", "Compressor", "Distortion", "Echo", 
                                       "Flanger", "Gargle", "Param Eq", "Reverb" };




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
{
    HWND    hDlg = NULL;
    BOOL    bDone = FALSE;
    int     nExitCode;
    HRESULT hr; 
    DWORD   dwResult;
    MSG     msg;

    g_hInst = hInst;

    CoInitialize( NULL );

    // Init the common control dll 
    InitCommonControls();

    // Display the main dialog box.
    hDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );
    ShowWindow( hDlg, nCmdShow );
    UpdateWindow( hDlg );

    while( !bDone ) 
    { 
        dwResult = MsgWaitForMultipleObjects( 1, &g_hDMusicMessageEvent, 
                                              FALSE, INFINITE, QS_ALLEVENTS );
        switch( dwResult )
        {
            case WAIT_OBJECT_0 + 0:
                // g_hDPMessageEvent is signaled, so there are
                // DirectPlay messages available
                if( FAILED( hr = ProcessDirectMusicMessages( hDlg ) ) ) 
                {
                    DXTRACE_ERR( TEXT("ProcessDirectMusicMessages"), hr );
                    return FALSE;
                }
                break;

            case WAIT_OBJECT_0 + 1:
                // Windows messages are available
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                { 
                    if( !IsDialogMessage( hDlg, &msg ) )  
                    {
                        TranslateMessage( &msg ); 
                        DispatchMessage( &msg ); 
                    }

                    if( msg.message == WM_QUIT )
                    {
                        nExitCode = (int)msg.wParam;
                        bDone     = TRUE;
                        DestroyWindow( hDlg );
                    }
                }
                break;
        }
    }

    CoUninitialize();

    return nExitCode;
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
                case IDOK:
                case IDCANCEL:
                    PostQuitMessage( 0 );
                    break;

                case IDC_BUTTON_OPEN:
                    OnOpenSoundFile( hDlg );
                    break;

                case IDC_BUTTON_PLAY:
                    if( FAILED( hr = OnPlaySound( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlaySound"), hr );
                        MessageBox( hDlg, "Error playing DirectSound buffer."
                                    "Sample will now exit.", "DirectSound Sample", 
                                    MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, IDABORT );
                    }
                    break;

                case IDC_BUTTON_STOP:
                    if( g_pSegment )
                        g_pSegment->Stop();
                    EnablePlayUI( hDlg, TRUE );
                    SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("Sound stopped.") );
                    break;

                case IDC_RADIO_TRIANGLE:
                case IDC_RADIO_SQUARE:
                case IDC_RADIO_SINE:
                case IDC_RADIO_NEG_180:
                case IDC_RADIO_NEG_90:
                case IDC_RADIO_ZERO:
                case IDC_RADIO_90:
                case IDC_RADIO_180:
                    OnEffectChanged( hDlg );
                    break;

                default:
                    if( LOWORD( wParam ) >= IDC_RADIO_CHORUS &&
                        LOWORD( wParam ) <= IDC_RADIO_REVERB )
                    {
                        g_dwCurrentFXType = LOWORD( wParam ) - IDC_RADIO_CHORUS;
                        LoadParameterUI( hDlg, g_dwCurrentFXType );
                    }
                    else
                        return FALSE; // Didn't handle message
            }
            break;

        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;

        case WM_NOTIFY:
        {
            //LPNMHDR pnmh = ( LPNMHDR ) lParam;
            //if( pnmh->code >= IDC_SLIDER1 && pnmh->code <= IDC_SLIDER6 )
            //{
                OnEffectChanged( hDlg );
            //}
            break;
        }
        case WM_DESTROY:
            // Cleanup everything
            CloseHandle( g_hDMusicMessageEvent );
            SAFE_DELETE( g_lpFXManager );
            SAFE_DELETE( g_pSegment );
            SAFE_DELETE( g_lpMusicManager );
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

    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_ICON ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG, ( LPARAM ) hIcon );
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, ( LPARAM ) hIcon );
    
    // create a DirectMusicManager
    g_lpMusicManager = new CMusicManager();
    g_lpFXManager = new CSoundFXManager();

    // create a music manager with 128 performance channels, stereo (not shared)
    if( FAILED( hr = g_lpMusicManager->Initialize( hDlg, 128, DMUS_APATH_DYNAMIC_STEREO ) ) )
    {
        DXTRACE_ERR( TEXT("Initialize"), hr );
        MessageBox( hDlg, "Error initializing DirectSound.  Sample will now exit.", 
                    "DirectSound Sample", MB_OK | MB_ICONERROR );
        PostQuitMessage( 0 );
        return;
    }

    // Register segment notification, to know when segment stopped
    g_hDMusicMessageEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    GUID guid = GUID_NOTIFICATION_SEGMENT;

    IDirectMusicPerformance * pPerf = g_lpMusicManager->GetPerformance();
    pPerf->AddNotificationType( guid );
    pPerf->SetNotificationHandle( g_hDMusicMessageEvent, 0 );  

    // Load default file
    TCHAR strFile[MAX_PATH];
    GetWindowsDirectory( strFile, MAX_PATH );
    lstrcat( strFile, "\\media\\ding.wav" );
    
    if( FAILED( hr = ValidateFile( hDlg, strFile ) ) )
    {
        // Set the UI controls
        SetDlgItemText( hDlg, IDC_TEXT_FILENAME, TEXT("") );
        SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("No file loaded.") );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );
        return;
    }

    if( SUCCEEDED( hr = g_lpMusicManager->CreateSegmentFromFile( &g_pSegment, g_strFileName, TRUE, FALSE ) ) )
    {
        g_lpFXManager->Initialize( g_lpMusicManager->GetDefaultAudioPath(), TRUE );

        // reset the manager so nothing is enabled - the parameters retained, however
        g_lpFXManager->UnInitialize();

        SAFE_DELETE( g_pSegment );
    }

    // set UI defaults
    CheckDlgButton( hDlg, IDC_CHECK_LOOP, BST_CHECKED );
    CheckRadioButton( hDlg, IDC_RADIO_CHORUS, IDC_RADIO_REVERB, IDC_RADIO_CHORUS );
    LoadParameterUI( hDlg, g_dwCurrentFXType );
    EnablePlayUI( hDlg, TRUE );
}




//-----------------------------------------------------------------------------
// Name: OnPlaySound()
// Desc: User hit the "Play" button
//-----------------------------------------------------------------------------
HRESULT OnPlaySound( HWND hDlg )
{
    HRESULT hr;
    DWORD   i;

    BOOL bLooped = ( IsDlgButtonChecked( hDlg, IDC_CHECK_LOOP ) == BST_CHECKED );

    // Free any previous sound and FXs
    g_lpFXManager->DisableAllFX();

    SAFE_DELETE( g_pSegment );
    g_lpMusicManager->CollectGarbage();

    // For DirectMusic must know if the file is a standard MIDI file or not
    // in order to load the correct instruments.
    BOOL bMidiFile = FALSE;
    if( strstr( g_strFileName, ".mid" ) != NULL ||
        strstr( g_strFileName, ".rmi" ) != NULL ) 
    {
        bMidiFile = TRUE;
    }

    // Since the user can change the focus before the sound is played, 
    // we need to create the sound buffer every time the play button is pressed 
    // Load the segment file into a DirectMusicSegment buffer
    if( FAILED( hr = g_lpMusicManager->CreateSegmentFromFile( &g_pSegment, g_strFileName, 
                                                              TRUE, bMidiFile ) ) )
    {
        // Not a critical failure, so just update the status
        DXTRACE_ERR_NOMSGBOX( TEXT("Create"), hr );
        SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("Could not load the segment file.") );
        return S_FALSE; 
    }

    g_lpFXManager->Initialize( g_lpMusicManager->GetDefaultAudioPath(), FALSE );
    for( i = IDC_CHECK_CHORUS; i <= IDC_CHECK_REVERB; i++ )
    {
        if( IsDlgButtonChecked( hDlg, i ) == BST_CHECKED )
            g_lpFXManager->SetFXEnable( i - IDC_CHECK_CHORUS );
    }
    g_lpFXManager->ActivateFX();
    g_lpFXManager->LoadCurrentFXParameters();

    // Play the sound
    if( FAILED( hr = g_pSegment->SetRepeats( ( bLooped ?  DMUS_SEG_REPEAT_INFINITE : 0 ) ) ) )
        return DXTRACE_ERR( TEXT("SetRepeats"), hr );
    if( FAILED( hr = g_pSegment->Play( 0 ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

    // Update the UI controls to show the sound as playing
    EnablePlayUI( hDlg, FALSE );
    SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("Sound playing.") );

    return S_OK;
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
                         TEXT("Audio Files\0*.sgt;*.mid;*.rmi;*.wav\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Audio File"),
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
    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );
    SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("Loading file...") );

    if( g_pSegment )
        g_pSegment->Stop();

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        if( g_pSegment )
        {
            EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), TRUE );
            EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), TRUE );
        }

        SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("Load aborted.") );
        return;
    }

    SetDlgItemText( hDlg, IDC_TEXT_FILENAME, TEXT("") );

    // Make sure file is a valid file
    ValidateFile( hDlg, strFileName );

    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY ), TRUE );
    EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP ), FALSE );

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}




//-----------------------------------------------------------------------------
// Name: EnablePlayUI()
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_CHECK_LOOP      ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY     ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP     ), FALSE );
        SetFocus(     GetDlgItem( hDlg, IDC_BUTTON_PLAY )   );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_CHECK_LOOP      ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_PLAY     ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUTTON_STOP     ), TRUE );
        SetFocus(     GetDlgItem( hDlg, IDC_BUTTON_STOP )   );
    }

    for( DWORD i = IDC_CHECK_CHORUS; i <= IDC_CHECK_REVERB; i++ )
        EnableWindow( GetDlgItem( hDlg, i ), bEnable );
}




//-----------------------------------------------------------------------------
// Name: ResetParameterUI()
// Desc: Blanks the paramter UI
//-----------------------------------------------------------------------------
VOID ResetParameterUI( HWND hwndDlg )
{
    HWND hwndItem;
    DWORD i;

    for ( i = IDC_PARAM_NAME1; i <= IDC_PARAM_MAX6; i++ )
    {
        hwndItem = GetDlgItem( hwndDlg, i );
        SendMessage( hwndItem, WM_SETTEXT, 0, ( LPARAM ) TEXT( "- - -" ) );
        EnableWindow( hwndItem, FALSE );
    }

    for ( i = IDC_SLIDER1; i <= IDC_SLIDER6; i += 5 )
    {
        PostMessage( GetDlgItem( hwndDlg, i ), TBM_SETRANGEMIN, FALSE, DEFAULT_SLIDER_MIN );
        PostMessage( GetDlgItem( hwndDlg, i ), TBM_SETRANGEMAX, FALSE, DEFAULT_SLIDER_MAX );
        PostMessage( GetDlgItem( hwndDlg, i ), TBM_SETLINESIZE, FALSE, DEFAULT_SLIDER_INC );
        PostMessage( GetDlgItem( hwndDlg, i ), TBM_SETPOS, TRUE, 0 );
    }

    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_TRIANGLE ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_SQUARE ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_SINE ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_WAVEFORM ), FALSE );

    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_NEG_180 ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_NEG_90 ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_ZERO ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_90 ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_180 ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_PHASE ), FALSE );
}




//-----------------------------------------------------------------------------
// Name: EnableSingleParameter
// Desc: 
//-----------------------------------------------------------------------------
VOID EnableSingleParameter( HWND hwndDlg, DWORD resID )
{
    for ( DWORD i = resID; i < resID + 5; i++ )
        EnableWindow( GetDlgItem( hwndDlg, i ), TRUE );      
}




//-----------------------------------------------------------------------------
// Name: PosSlider
// Desc: 
//-----------------------------------------------------------------------------
VOID PosSlider( HWND hwndDlg, DWORD dwSlider, FLOAT val, FLOAT min, FLOAT max, 
                FLOAT fSliderInc )
{
    HWND hwndSlider = GetDlgItem( hwndDlg, dwSlider );

    LONG lSliderInc = (LONG) ( (fSliderInc / (max - min)) * DEFAULT_SLIDER_MAX );
    PostMessage( hwndSlider, TBM_SETLINESIZE, FALSE, lSliderInc );

    FLOAT res = ( val - min ) / ( max - min );
    if( res < 0.0f )
        res = 0.0f;
    else if( res > 1.0f )
        res = 1.0f;

    LONG pos = (LONG) ( res * DEFAULT_SLIDER_MAX );
    PostMessage( hwndSlider, TBM_SETPOS, TRUE, pos );
}




//-----------------------------------------------------------------------------
// Name: LoadSingleParameter
// Desc: 
//-----------------------------------------------------------------------------
VOID LoadSingleParameter( HWND hwndDlg, DWORD id, const TCHAR * strName, FLOAT val, 
                          FLOAT min, FLOAT max, FLOAT fSliderInc = 0.1f,
                          DWORD dwPrecision = 1 )
{
    TCHAR strTemp[MAX_PATH];

    // wet dry mix
    EnableSingleParameter( hwndDlg, id );
    SendMessage( GetDlgItem( hwndDlg, id ), WM_SETTEXT, 0, ( LPARAM ) strName );

    switch( dwPrecision )
    {
        case 0:
            sprintf( strTemp, "%.0f", val );
            SendMessage( GetDlgItem( hwndDlg, id + 1 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.0f", min );
            SendMessage( GetDlgItem( hwndDlg, id + 3 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.0f", max );
            SendMessage( GetDlgItem( hwndDlg, id + 4 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            break;
        case 1:
            sprintf( strTemp, "%.1f", val );
            SendMessage( GetDlgItem( hwndDlg, id + 1 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.1f", min );
            SendMessage( GetDlgItem( hwndDlg, id + 3 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.1f", max );
            SendMessage( GetDlgItem( hwndDlg, id + 4 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            break;
        default:
        case 2:
            sprintf( strTemp, "%.2f", val );
            SendMessage( GetDlgItem( hwndDlg, id + 1 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.2f", min );
            SendMessage( GetDlgItem( hwndDlg, id + 3 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            sprintf( strTemp, "%.2f", max );
            SendMessage( GetDlgItem( hwndDlg, id + 4 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
            break;
    }

    PosSlider( hwndDlg, id + 2, val, min, max, fSliderInc );
}




//-----------------------------------------------------------------------------
// Name: SaveSingleParameter
// Desc: 
//-----------------------------------------------------------------------------
VOID SaveSingleParameter( HWND hwndDlg, DWORD id, FLOAT * fVal, FLOAT fMin, FLOAT fMax, DWORD dwPrecision = 1 )
{
    TCHAR strTemp[MAX_PATH];

    DWORD dwPos = ( DWORD ) SendMessage( GetDlgItem( hwndDlg, id + 2), TBM_GETPOS, 0, 0 );

    FLOAT fPercent = (FLOAT) ( dwPos - DEFAULT_SLIDER_MIN ) / 
                     (FLOAT) ( DEFAULT_SLIDER_MAX - DEFAULT_SLIDER_MIN );
    *fVal = fPercent * ( fMax - fMin ) + fMin;

    switch( dwPrecision )
    {
        case 0:
            sprintf( strTemp, "%.0f", *fVal );
            break;
        case 1:
            sprintf( strTemp, "%.1f", *fVal );
            break;
        case 2:
        default:
            sprintf( strTemp, "%.2f", *fVal );
            break;
    }

    SendMessage( GetDlgItem( hwndDlg, id + 1 ), WM_SETTEXT, 0, ( LPARAM ) strTemp );
}




//-----------------------------------------------------------------------------
// Name: LoadWaveformRadio
// Desc: 
//-----------------------------------------------------------------------------
VOID LoadWaveformRadio( HWND hwndDlg, LONG waveform, LONG triangle, LONG square, LONG sine )
{
    if( waveform == triangle )
        CheckRadioButton( hwndDlg, IDC_RADIO_TRIANGLE, IDC_RADIO_SINE, IDC_RADIO_TRIANGLE );
    else if( waveform == square )
        CheckRadioButton( hwndDlg, IDC_RADIO_TRIANGLE, IDC_RADIO_SINE, IDC_RADIO_SQUARE );
    else if( waveform == sine )
        CheckRadioButton( hwndDlg, IDC_RADIO_TRIANGLE, IDC_RADIO_SINE, IDC_RADIO_SINE );

    if( triangle >= 0 )
        EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_TRIANGLE ), TRUE );
    if( square >= 0 )
        EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_SQUARE ), TRUE );
    if( sine >= 0 )
        EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO_SINE ), TRUE );

    EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_WAVEFORM ), ( triangle + square + sine != -3 ) );
}




//-----------------------------------------------------------------------------
// Name: LoadPhaseRadio
// Desc: 
//-----------------------------------------------------------------------------
VOID LoadPhaseRadio( HWND hwndDlg, LONG phase, LONG neg180, LONG neg90, LONG zero, LONG pos90, LONG pos180 )
{
    for( int i = IDC_RADIO_NEG_180; i <= IDC_RADIO_180; i++ )
        EnableWindow( GetDlgItem( hwndDlg, i ), TRUE );

    EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_PHASE), TRUE );

    if( phase == neg180 )
        CheckRadioButton( hwndDlg, IDC_RADIO_NEG_180, IDC_RADIO_90, IDC_RADIO_NEG_180 );
    else if( phase == neg90 )
        CheckRadioButton( hwndDlg, IDC_RADIO_NEG_180, IDC_RADIO_90, IDC_RADIO_NEG_90 );
    else if( phase == zero )
        CheckRadioButton( hwndDlg, IDC_RADIO_NEG_180, IDC_RADIO_90, IDC_RADIO_ZERO );
    else if( phase == pos90 )
        CheckRadioButton( hwndDlg, IDC_RADIO_NEG_180, IDC_RADIO_90, IDC_RADIO_90 );
    else if( phase == pos180 )
        CheckRadioButton( hwndDlg, IDC_RADIO_NEG_180, IDC_RADIO_90, IDC_RADIO_180 );
}




//-----------------------------------------------------------------------------
// Name: LoadParameterUI()
// Desc: loads the paramter ui for particular effect
//-----------------------------------------------------------------------------
VOID LoadParameterUI( HWND hwndDlg, DWORD dwFXType )
{
    TCHAR strTemp[MAX_PATH];

    // reset the parameter ui
    ResetParameterUI( hwndDlg );

    sprintf( strTemp, "Parameters for [ %s ]", g_strFXNames[dwFXType] );
    SetDlgItemText( hwndDlg, IDC_FRAME, strTemp );

    switch( dwFXType)
    {
        case eSFX_chorus:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Wet/Dry Mix (%)" ), g_lpFXManager->m_paramsChorus.fWetDryMix, DSFXCHORUS_WETDRYMIX_MIN, DSFXCHORUS_WETDRYMIX_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Depth (%)" ), g_lpFXManager->m_paramsChorus.fDepth, DSFXCHORUS_DEPTH_MIN, DSFXCHORUS_DEPTH_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Feedback (%)" ), g_lpFXManager->m_paramsChorus.fFeedback, DSFXCHORUS_FEEDBACK_MIN, DSFXCHORUS_FEEDBACK_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "Frequency (Hz)" ), g_lpFXManager->m_paramsChorus.fFrequency, DSFXCHORUS_FREQUENCY_MIN, DSFXCHORUS_FREQUENCY_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME5, TEXT( "Delay (ms)" ), g_lpFXManager->m_paramsChorus.fDelay, DSFXCHORUS_DELAY_MIN, DSFXCHORUS_DELAY_MAX );
            LoadWaveformRadio( hwndDlg, g_lpFXManager->m_paramsChorus.lWaveform, DSFXCHORUS_WAVE_TRIANGLE, -1, DSFXCHORUS_WAVE_SIN );
            LoadPhaseRadio( hwndDlg, g_lpFXManager->m_paramsChorus.lPhase, DSFXCHORUS_PHASE_NEG_180, DSFXCHORUS_PHASE_NEG_90, DSFXCHORUS_PHASE_ZERO, DSFXCHORUS_PHASE_90, DSFXCHORUS_PHASE_180 );
            break;
        }

        case eSFX_compressor:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Gain (dB)" ), g_lpFXManager->m_paramsCompressor.fGain, DSFXCOMPRESSOR_GAIN_MIN, DSFXCOMPRESSOR_GAIN_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Attack (ms)" ), g_lpFXManager->m_paramsCompressor.fAttack, DSFXCOMPRESSOR_ATTACK_MIN, DSFXCOMPRESSOR_ATTACK_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Release (ms)" ), g_lpFXManager->m_paramsCompressor.fRelease, DSFXCOMPRESSOR_RELEASE_MIN, DSFXCOMPRESSOR_RELEASE_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "Threshold (dB)" ), g_lpFXManager->m_paramsCompressor.fThreshold, DSFXCOMPRESSOR_THRESHOLD_MIN, DSFXCOMPRESSOR_THRESHOLD_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME5, TEXT( "Ratio (x:1)" ), g_lpFXManager->m_paramsCompressor.fRatio, DSFXCOMPRESSOR_RATIO_MIN, DSFXCOMPRESSOR_RATIO_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME6, TEXT( "Predelay (ms)" ), g_lpFXManager->m_paramsCompressor.fPredelay, DSFXCOMPRESSOR_PREDELAY_MIN, DSFXCOMPRESSOR_PREDELAY_MAX, 0.05f, 2 );
            break;
        }

        case eSFX_distortion:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Gain (dB)" ), g_lpFXManager->m_paramsDistortion.fGain, DSFXDISTORTION_GAIN_MIN, DSFXDISTORTION_GAIN_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Edge (%)" ), g_lpFXManager->m_paramsDistortion.fEdge, DSFXDISTORTION_EDGE_MIN, DSFXDISTORTION_EDGE_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "PostEQ Center Freq (Hz)" ), g_lpFXManager->m_paramsDistortion.fPostEQCenterFrequency, DSFXDISTORTION_POSTEQCENTERFREQUENCY_MIN, DSFXDISTORTION_POSTEQCENTERFREQUENCY_MAX, 1.0f, 0 );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "PostEQ Bandwidth (Hz)" ), g_lpFXManager->m_paramsDistortion.fPostEQBandwidth, DSFXDISTORTION_POSTEQBANDWIDTH_MIN, DSFXDISTORTION_POSTEQBANDWIDTH_MAX, 1.0f, 0 );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME5, TEXT( "PreLowpass Cutoff (Hz)" ), g_lpFXManager->m_paramsDistortion.fPreLowpassCutoff, DSFXDISTORTION_PRELOWPASSCUTOFF_MIN, DSFXDISTORTION_PRELOWPASSCUTOFF_MAX, 1.0f, 0 );
            break;
        }

        case eSFX_echo:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Wet/Dry Mix (%)" ), g_lpFXManager->m_paramsEcho.fWetDryMix, DSFXECHO_WETDRYMIX_MIN, DSFXECHO_WETDRYMIX_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Feedback (%)" ), g_lpFXManager->m_paramsEcho.fFeedback, DSFXECHO_FEEDBACK_MIN, DSFXECHO_FEEDBACK_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Left Delay (ms)" ), g_lpFXManager->m_paramsEcho.fLeftDelay, DSFXECHO_LEFTDELAY_MIN, DSFXECHO_LEFTDELAY_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "Right Delay (ms)" ), g_lpFXManager->m_paramsEcho.fRightDelay, DSFXECHO_RIGHTDELAY_MIN, DSFXECHO_RIGHTDELAY_MAX );
            break;
        }

        case eSFX_flanger:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Wet/Dry Mix (%)" ), g_lpFXManager->m_paramsFlanger.fWetDryMix, DSFXFLANGER_WETDRYMIX_MIN, DSFXFLANGER_WETDRYMIX_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Depth (%)" ), g_lpFXManager->m_paramsFlanger.fDepth, DSFXFLANGER_DEPTH_MIN, DSFXFLANGER_DEPTH_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Feedback (%)" ), g_lpFXManager->m_paramsFlanger.fFeedback, DSFXFLANGER_FEEDBACK_MIN, DSFXFLANGER_FEEDBACK_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "Frequency (Hz)" ), g_lpFXManager->m_paramsFlanger.fFrequency, DSFXFLANGER_FREQUENCY_MIN, DSFXFLANGER_FREQUENCY_MAX, 0.01f, 2 );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME5, TEXT( "Delay (ms)" ), g_lpFXManager->m_paramsFlanger.fDelay, DSFXFLANGER_DELAY_MIN, DSFXFLANGER_DELAY_MAX, 0.01f, 2 );
            LoadWaveformRadio( hwndDlg, g_lpFXManager->m_paramsFlanger.lWaveform, DSFXFLANGER_WAVE_TRIANGLE, -1, DSFXFLANGER_WAVE_SIN );
            LoadPhaseRadio( hwndDlg, g_lpFXManager->m_paramsFlanger.lPhase, DSFXFLANGER_PHASE_NEG_180, DSFXFLANGER_PHASE_NEG_90, DSFXFLANGER_PHASE_ZERO, DSFXFLANGER_PHASE_90, DSFXFLANGER_PHASE_180 );
            break;
        }

        case eSFX_gargle:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Rate (Hz)" ), ( FLOAT ) g_lpFXManager->m_paramsGargle.dwRateHz, ( FLOAT ) DSFXGARGLE_RATEHZ_MIN, ( FLOAT ) DSFXGARGLE_RATEHZ_MAX );
            LoadWaveformRadio( hwndDlg, g_lpFXManager->m_paramsGargle.dwWaveShape, DSFXGARGLE_WAVE_TRIANGLE, DSFXGARGLE_WAVE_SQUARE, -1 );
            break;
        }

        case eSFX_parameq:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "Center Freq (Hz)" ), g_lpFXManager->m_paramsParamEq.fCenter, DSFXPARAMEQ_CENTER_MIN, DSFXPARAMEQ_CENTER_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Bandwidth (Hz)" ), g_lpFXManager->m_paramsParamEq.fBandwidth, DSFXPARAMEQ_BANDWIDTH_MIN, DSFXPARAMEQ_BANDWIDTH_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Gain (dB)" ), g_lpFXManager->m_paramsParamEq.fGain, DSFXPARAMEQ_GAIN_MIN, DSFXPARAMEQ_GAIN_MAX );
            break;
        }

        case eSFX_reverb:
        {            
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME1, TEXT( "In Gain (dB)" ), g_lpFXManager->m_paramsReverb.fInGain, DSFX_WAVESREVERB_INGAIN_MIN, DSFX_WAVESREVERB_INGAIN_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME2, TEXT( "Reverb Mix (dB)" ), g_lpFXManager->m_paramsReverb.fReverbMix, DSFX_WAVESREVERB_REVERBMIX_MIN, DSFX_WAVESREVERB_REVERBMIX_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME3, TEXT( "Reverb Time (ms)" ), g_lpFXManager->m_paramsReverb.fReverbTime, DSFX_WAVESREVERB_REVERBTIME_MIN, DSFX_WAVESREVERB_REVERBTIME_MAX );
            LoadSingleParameter( hwndDlg, IDC_PARAM_NAME4, TEXT( "HighFreq RT Ratio (x:1)" ), g_lpFXManager->m_paramsReverb.fHighFreqRTRatio, DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN, DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX, 0.01f, 2 );
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: OnEffectChanged()  
// Desc: Called when the UI prompted an effect change
//-----------------------------------------------------------------------------
VOID OnEffectChanged( HWND hwndDlg )
{
    switch( g_dwCurrentFXType )
    {
        case eSFX_chorus:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsChorus.fWetDryMix, DSFXCHORUS_WETDRYMIX_MIN, DSFXCHORUS_WETDRYMIX_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsChorus.fDepth, DSFXCHORUS_DEPTH_MIN, DSFXCHORUS_DEPTH_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsChorus.fFeedback, DSFXCHORUS_FEEDBACK_MIN, DSFXCHORUS_FEEDBACK_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsChorus.fFrequency, DSFXCHORUS_FREQUENCY_MIN, DSFXCHORUS_FREQUENCY_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME5, &g_lpFXManager->m_paramsChorus.fDelay, DSFXCHORUS_DELAY_MIN, DSFXCHORUS_DELAY_MAX );

            if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_TRIANGLE ) == BST_CHECKED )
                g_lpFXManager->m_paramsChorus.lWaveform = DSFXCHORUS_WAVE_TRIANGLE;
            else
                g_lpFXManager->m_paramsChorus.lWaveform = DSFXCHORUS_WAVE_SIN;

            if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_NEG_180 ) == BST_CHECKED )
                g_lpFXManager->m_paramsChorus.lPhase = DSFXCHORUS_PHASE_NEG_180;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_NEG_90 ) == BST_CHECKED )
                g_lpFXManager->m_paramsChorus.lPhase = DSFXCHORUS_PHASE_NEG_90;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_ZERO ) == BST_CHECKED )
                g_lpFXManager->m_paramsChorus.lPhase = DSFXCHORUS_PHASE_ZERO;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_90 ) == BST_CHECKED )
                g_lpFXManager->m_paramsChorus.lPhase = DSFXCHORUS_PHASE_90;
            else
                g_lpFXManager->m_paramsChorus.lPhase = DSFXCHORUS_PHASE_180;

            if( g_lpFXManager->m_lpChorus )
                g_lpFXManager->m_lpChorus->SetAllParameters( &g_lpFXManager->m_paramsChorus );

            break;
        }

        case eSFX_compressor:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsCompressor.fGain, DSFXCOMPRESSOR_GAIN_MIN, DSFXCOMPRESSOR_GAIN_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsCompressor.fAttack, DSFXCOMPRESSOR_ATTACK_MIN, DSFXCOMPRESSOR_ATTACK_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsCompressor.fRelease, DSFXCOMPRESSOR_RELEASE_MIN, DSFXCOMPRESSOR_RELEASE_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsCompressor.fThreshold, DSFXCOMPRESSOR_THRESHOLD_MIN, DSFXCOMPRESSOR_THRESHOLD_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME5, &g_lpFXManager->m_paramsCompressor.fRatio, DSFXCOMPRESSOR_RATIO_MIN, DSFXCOMPRESSOR_RATIO_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME6, &g_lpFXManager->m_paramsCompressor.fPredelay, DSFXCOMPRESSOR_PREDELAY_MIN, DSFXCOMPRESSOR_PREDELAY_MAX, 2 );

            if( g_lpFXManager->m_lpCompressor )
                g_lpFXManager->m_lpCompressor->SetAllParameters( &g_lpFXManager->m_paramsCompressor );
            break;
        }

        case eSFX_distortion:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsDistortion.fGain, DSFXDISTORTION_GAIN_MIN, DSFXDISTORTION_GAIN_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsDistortion.fEdge, DSFXDISTORTION_EDGE_MIN, DSFXDISTORTION_EDGE_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsDistortion.fPostEQCenterFrequency, DSFXDISTORTION_POSTEQCENTERFREQUENCY_MIN, DSFXDISTORTION_POSTEQCENTERFREQUENCY_MAX, 0 );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsDistortion.fPostEQBandwidth, DSFXDISTORTION_POSTEQBANDWIDTH_MIN, DSFXDISTORTION_POSTEQBANDWIDTH_MAX, 0 );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME5, &g_lpFXManager->m_paramsDistortion.fPreLowpassCutoff, DSFXDISTORTION_PRELOWPASSCUTOFF_MIN, DSFXDISTORTION_PRELOWPASSCUTOFF_MAX, 0 );

            if( g_lpFXManager->m_lpDistortion )
                g_lpFXManager->m_lpDistortion->SetAllParameters( &g_lpFXManager->m_paramsDistortion );
            break;
        }

        case eSFX_echo:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsEcho.fWetDryMix, DSFXECHO_WETDRYMIX_MIN, DSFXECHO_WETDRYMIX_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsEcho.fFeedback, DSFXECHO_FEEDBACK_MIN, DSFXECHO_FEEDBACK_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsEcho.fLeftDelay, DSFXECHO_LEFTDELAY_MIN, DSFXECHO_LEFTDELAY_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsEcho.fRightDelay, DSFXECHO_RIGHTDELAY_MIN, DSFXECHO_RIGHTDELAY_MAX );

            if( g_lpFXManager->m_lpEcho )
                g_lpFXManager->m_lpEcho->SetAllParameters( &g_lpFXManager->m_paramsEcho );
            break;
        }

        case eSFX_flanger:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsFlanger.fWetDryMix, DSFXFLANGER_WETDRYMIX_MIN, DSFXFLANGER_WETDRYMIX_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsFlanger.fDepth, DSFXFLANGER_DEPTH_MIN, DSFXFLANGER_DEPTH_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsFlanger.fFeedback, DSFXFLANGER_FEEDBACK_MIN, DSFXFLANGER_FEEDBACK_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsFlanger.fFrequency, DSFXFLANGER_FREQUENCY_MIN, DSFXFLANGER_FREQUENCY_MAX, 2 );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME5, &g_lpFXManager->m_paramsFlanger.fDelay, DSFXFLANGER_DELAY_MIN, DSFXFLANGER_DELAY_MAX, 2 );

            if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_TRIANGLE ) == BST_CHECKED )
                g_lpFXManager->m_paramsFlanger.lWaveform = DSFXFLANGER_WAVE_TRIANGLE;
            else
                g_lpFXManager->m_paramsFlanger.lWaveform = DSFXFLANGER_WAVE_SIN;

            if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_NEG_180 ) == BST_CHECKED )
                g_lpFXManager->m_paramsFlanger.lPhase = DSFXFLANGER_PHASE_NEG_180;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_NEG_90 ) == BST_CHECKED )
                g_lpFXManager->m_paramsFlanger.lPhase = DSFXFLANGER_PHASE_NEG_90;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_ZERO ) == BST_CHECKED )
                g_lpFXManager->m_paramsFlanger.lPhase = DSFXFLANGER_PHASE_ZERO;
            else if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_90 ) == BST_CHECKED )
                g_lpFXManager->m_paramsFlanger.lPhase = DSFXFLANGER_PHASE_90;
            else
                g_lpFXManager->m_paramsFlanger.lPhase = DSFXFLANGER_PHASE_180;

            if( g_lpFXManager->m_lpFlanger )
                g_lpFXManager->m_lpFlanger->SetAllParameters( &g_lpFXManager->m_paramsFlanger );
            break;
        }

        case eSFX_gargle:
        {
            FLOAT fRateHz;
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &fRateHz, DSFXGARGLE_RATEHZ_MIN, DSFXGARGLE_RATEHZ_MAX );
            g_lpFXManager->m_paramsGargle.dwRateHz = (DWORD) fRateHz;

            if( IsDlgButtonChecked( hwndDlg, IDC_RADIO_TRIANGLE ) == BST_CHECKED )
                g_lpFXManager->m_paramsGargle.dwWaveShape = DSFXGARGLE_WAVE_TRIANGLE;
            else
                g_lpFXManager->m_paramsGargle.dwWaveShape = DSFXGARGLE_WAVE_SQUARE;

            if( g_lpFXManager->m_lpGargle )
                g_lpFXManager->m_lpGargle->SetAllParameters( &g_lpFXManager->m_paramsGargle );
            break;
        }

        case eSFX_parameq:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsParamEq.fCenter, DSFXPARAMEQ_CENTER_MIN, DSFXPARAMEQ_CENTER_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsParamEq.fBandwidth, DSFXPARAMEQ_BANDWIDTH_MIN, DSFXPARAMEQ_BANDWIDTH_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsParamEq.fGain, DSFXPARAMEQ_GAIN_MIN, DSFXPARAMEQ_GAIN_MAX );

            if( g_lpFXManager->m_lpParamEq )
                g_lpFXManager->m_lpParamEq->SetAllParameters( &g_lpFXManager->m_paramsParamEq );
            break;
        }

        case eSFX_reverb:
        {
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME1, &g_lpFXManager->m_paramsReverb.fInGain, DSFX_WAVESREVERB_INGAIN_MIN, DSFX_WAVESREVERB_INGAIN_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME2, &g_lpFXManager->m_paramsReverb.fReverbMix, DSFX_WAVESREVERB_REVERBMIX_MIN, DSFX_WAVESREVERB_REVERBMIX_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME3, &g_lpFXManager->m_paramsReverb.fReverbTime, DSFX_WAVESREVERB_REVERBTIME_MIN, DSFX_WAVESREVERB_REVERBTIME_MAX );
            SaveSingleParameter( hwndDlg, IDC_PARAM_NAME4, &g_lpFXManager->m_paramsReverb.fHighFreqRTRatio, DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN, DSFX_WAVESREVERB_HIGHFREQRTRATIO_MAX, 2 );

            if( g_lpFXManager->m_lpReverb )
                g_lpFXManager->m_lpReverb->SetAllParameters( &g_lpFXManager->m_paramsReverb );
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: ValidateFile()
// Desc: Checks to see if the file exists
//-----------------------------------------------------------------------------
HRESULT ValidateFile( HWND hDlg, TCHAR* strFileName )
{
    if( -1 == GetFileAttributes(strFileName) )
        return E_FAIL;
        
    SetDlgItemText( hDlg, IDC_TEXT_FILENAME, strFileName );
    SetDlgItemText( hDlg, IDC_TEXT_STATUS, TEXT("File loaded.") );
    strcpy( g_strFileName, strFileName );
    
    return S_OK;        
}




//-----------------------------------------------------------------------------
// Name: CSoundFXManager()
// Desc: constructor
//-----------------------------------------------------------------------------
CSoundFXManager::CSoundFXManager()
{
    m_lpChorus = NULL;
    m_lpCompressor = NULL;
    m_lpDistortion = NULL;
    m_lpEcho = NULL;
    m_lpFlanger = NULL;
    m_lpGargle = NULL;
    m_lpParamEq = NULL;
    m_lpReverb = NULL;

    ZeroMemory( &m_paramsChorus, sizeof( DSFXChorus ) );
    ZeroMemory( &m_paramsCompressor, sizeof( DSFXCompressor ) );
    ZeroMemory( &m_paramsDistortion, sizeof( DSFXDistortion ) );
    ZeroMemory( &m_paramsFlanger, sizeof( DSFXFlanger ) );
    ZeroMemory( &m_paramsEcho, sizeof( DSFXEcho ) );
    ZeroMemory( &m_paramsGargle, sizeof( DSFXGargle ) );
    ZeroMemory( &m_paramsParamEq, sizeof( DSFXParamEq ) );
    ZeroMemory( &m_paramsReverb, sizeof( DSFXWavesReverb ) );

    m_dwNumFX = 0;
    ZeroMemory( m_rgFxDesc, sizeof( DSEFFECTDESC ) * eNUM_SFX );
    ZeroMemory( m_rgRefGuids, sizeof( GUID * ) * eNUM_SFX );
    ZeroMemory( m_rgPtrs, sizeof(LPVOID*) * eNUM_SFX );
    ZeroMemory( m_rgLoaded, sizeof( BOOL ) * eNUM_SFX );

    m_lpDSB8 = NULL;
    m_pAudioPath = NULL;
}




//-----------------------------------------------------------------------------
// Name: ~CSoundFXManager()
// Desc: destructor
//-----------------------------------------------------------------------------
CSoundFXManager::~CSoundFXManager()
{
    // free any effects
    DisableAllFX();
    SAFE_RELEASE( m_lpDSB8 );
    SAFE_RELEASE( m_pAudioPath );
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: associates a DirectSoundBuffer with the manager, any effects
//       enabled in the old DirectSoundBuffer will be disabled, 
//       and the effect objects released
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::Initialize( IDirectMusicAudioPath8* pAudioPath, 
                                     BOOL bLoadDefaultParamValues )
{
    HRESULT hr;

    if( m_lpDSB8 )
    {
        // release the effect for the previously associated sound buffers
        DisableAllFX();
        SAFE_RELEASE( m_lpDSB8 );
        SAFE_RELEASE( m_pAudioPath );
    }

    if( NULL == pAudioPath )
        return S_OK;
    
    if( FAILED( hr = pAudioPath->GetObjectInPath( DMUS_PCHANNEL_ALL,
                DMUS_PATH_BUFFER, 0, GUID_NULL, 0, IID_IDirectSoundBuffer8, 
                (LPVOID*) &m_lpDSB8 )))
        return hr;

    m_pAudioPath = pAudioPath;
 
    if( bLoadDefaultParamValues )
        LoadDefaultParamValues();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UnInitialize()
// Desc: the manager goes back to default state, the effects params, however
//       will not be reset
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::UnInitialize()
{
    Initialize( NULL, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadDefaultParamValues()
// Desc: loads the default param value for each effect
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::LoadDefaultParamValues()
{
    DWORD i;

    if( NULL == m_lpDSB8 )
        return E_FAIL;

    for( i = eSFX_chorus; i < eNUM_SFX; i++ )
        SetFXEnable( i );
    
    ActivateFX();

    if( m_lpChorus )
        m_lpChorus->GetAllParameters( &m_paramsChorus );

    if( m_lpCompressor )
        m_lpCompressor->GetAllParameters( &m_paramsCompressor );
    
    if( m_lpDistortion )
        m_lpDistortion->GetAllParameters( &m_paramsDistortion );

    if( m_lpEcho )
        m_lpEcho->GetAllParameters( &m_paramsEcho );

    if( m_lpFlanger )
        m_lpFlanger->GetAllParameters( &m_paramsFlanger );

    if( m_lpGargle )
        m_lpGargle->GetAllParameters( &m_paramsGargle );

    if( m_lpParamEq )
        m_lpParamEq->GetAllParameters( &m_paramsParamEq );

    if( m_lpReverb )
        m_lpReverb->GetAllParameters( &m_paramsReverb );

    DisableAllFX();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadCurrentFXParameters
// Desc: loads the default param value for each effect
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::LoadCurrentFXParameters()
{
    if( m_lpChorus )
        m_lpChorus->SetAllParameters( &m_paramsChorus );

    if( m_lpCompressor )
        m_lpCompressor->SetAllParameters( &m_paramsCompressor );
    
    if( m_lpDistortion )
        m_lpDistortion->SetAllParameters( &m_paramsDistortion );

    if( m_lpEcho )
        m_lpEcho->SetAllParameters( &m_paramsEcho );

    if( m_lpFlanger )
        m_lpFlanger->SetAllParameters( &m_paramsFlanger );

    if( m_lpGargle )
        m_lpGargle->SetAllParameters( &m_paramsGargle );

    if( m_lpParamEq )
        m_lpParamEq->SetAllParameters( &m_paramsParamEq );

    if( m_lpReverb )
        m_lpReverb->SetAllParameters( &m_paramsReverb );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetFXEnable()
// Desc: enables a sound effect for the sound buffer associated with this
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::SetFXEnable( DWORD esfxType )
{
    HRESULT hr;

    if( esfxType >= eNUM_SFX )
        return E_FAIL;

    if( m_rgLoaded[esfxType] )
        return S_FALSE;
    else
        m_rgLoaded[esfxType] = TRUE;

    switch ( esfxType )
    {
    case eSFX_chorus:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_CHORUS,     IID_IDirectSoundFXChorus8,      
                              (LPVOID*) &m_lpChorus );
        break;
    case eSFX_compressor:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_COMPRESSOR, IID_IDirectSoundFXCompressor8,  
                              (LPVOID*) &m_lpCompressor );
        break;
    case eSFX_distortion:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_DISTORTION, IID_IDirectSoundFXDistortion8,  
                              (LPVOID*) &m_lpDistortion );
        break;
    case eSFX_echo:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_ECHO,       IID_IDirectSoundFXEcho8,        
                              (LPVOID*) &m_lpEcho );
        break;
    case eSFX_flanger:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_FLANGER,    IID_IDirectSoundFXFlanger8,     
                              (LPVOID*) &m_lpFlanger );
        break;
    case eSFX_gargle:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_GARGLE,     IID_IDirectSoundFXGargle8,      
                              (LPVOID*) &m_lpGargle );
        break;
    case eSFX_parameq:
        hr = EnableGenericFX( GUID_DSFX_STANDARD_PARAMEQ,    IID_IDirectSoundFXParamEq8,     
                              (LPVOID*) &m_lpParamEq );
        break;
    case eSFX_reverb:
        hr = EnableGenericFX( GUID_DSFX_WAVES_REVERB,        IID_IDirectSoundFXWavesReverb8, 
                              (LPVOID*) &m_lpReverb );
        break;
    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: DisableAllFX()
// Desc: disables all effect in the DirectSoundBuffer, and releases all effect
//       object.
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::DisableAllFX()
{
    HRESULT hr;

    // release all effect interfaces created with this manager so far
    SAFE_RELEASE( m_lpChorus );
    SAFE_RELEASE( m_lpCompressor );
    SAFE_RELEASE( m_lpDistortion );
    SAFE_RELEASE( m_lpEcho );
    SAFE_RELEASE( m_lpFlanger );
    SAFE_RELEASE( m_lpGargle );
    SAFE_RELEASE( m_lpParamEq );
    SAFE_RELEASE( m_lpReverb );
    
    m_dwNumFX = 0;
    ZeroMemory( m_rgFxDesc, sizeof( DSEFFECTDESC ) * eNUM_SFX );
    ZeroMemory( m_rgRefGuids, sizeof( GUID * ) * eNUM_SFX );
    ZeroMemory( m_rgPtrs, sizeof(LPVOID*) * eNUM_SFX );
    ZeroMemory( m_rgLoaded, sizeof( BOOL ) * eNUM_SFX );

    if( NULL == m_lpDSB8 )
        return E_FAIL;

    if( m_pAudioPath )
        m_pAudioPath->Activate( FALSE );
    
    // this removes all fx from the buffer
    if( FAILED( hr = m_lpDSB8->SetFX( 0, NULL, NULL ) ) )
        return DXTRACE_ERR( TEXT("SetFX"), hr );

    if( m_pAudioPath )
        m_pAudioPath->Activate( TRUE );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ActivateFX()
// Desc: activate the effects enabled from EnableFX()
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::ActivateFX()
{
    DWORD dwResults[eNUM_SFX];
    HRESULT hr;
    DWORD i;

    if( NULL == m_lpDSB8 )
        return E_FAIL;

    if( m_dwNumFX == 0 )
        return S_FALSE;

    if( m_pAudioPath )
        m_pAudioPath->Activate( FALSE );

    if( FAILED( hr = m_lpDSB8->SetFX( m_dwNumFX, m_rgFxDesc, dwResults ) ) )
        return DXTRACE_ERR( TEXT("SetFX"), hr );

    // get reference to the effect object
    for( i = 0; i < m_dwNumFX; i++ )
        if( FAILED( hr = m_lpDSB8->GetObjectInPath( m_rgFxDesc[i].guidDSFXClass, 0, *m_rgRefGuids[i], m_rgPtrs[i] ) ) )
            return hr;

    if( m_pAudioPath )
        m_pAudioPath->Activate( TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnableGenericFX()
// Desc: given information, tries to enabled an effect in a DirectSoundBuffer8,
//       and tries obtain reference to effect interface
//-----------------------------------------------------------------------------
HRESULT CSoundFXManager::EnableGenericFX( GUID guidSFXClass, REFGUID rguidInterface, LPVOID * ppObj )
{
    // if an effect already allocated
    if( *ppObj )
        return S_FALSE;

    if( m_dwNumFX >= eNUM_SFX )
        return E_FAIL;

    // set the effect to be enabled
    ZeroMemory( &m_rgFxDesc[m_dwNumFX], sizeof(DSEFFECTDESC) );
    m_rgFxDesc[m_dwNumFX].dwSize         = sizeof(DSEFFECTDESC);
    m_rgFxDesc[m_dwNumFX].dwFlags        = 0;
    CopyMemory( &m_rgFxDesc[m_dwNumFX].guidDSFXClass, &guidSFXClass, sizeof(GUID) );

    m_rgRefGuids[m_dwNumFX] = &rguidInterface;
    m_rgPtrs[m_dwNumFX] = ppObj;

    m_dwNumFX++;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ProcessDirectMusicMessages()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT ProcessDirectMusicMessages( HWND hDlg )
{
    HRESULT hr;
    IDirectMusicPerformance8* pPerf = NULL;
    DMUS_NOTIFICATION_PMSG* pPMsg;
        
    if( NULL == g_lpMusicManager )
        return S_OK;

    pPerf = g_lpMusicManager->GetPerformance();

    // Get waiting notification message from the performance
    while( S_OK == pPerf->GetNotificationPMsg( &pPMsg ) )
    {
        switch( pPMsg->dwNotificationOption )
        {
        case DMUS_NOTIFICATION_SEGEND:
            if( pPMsg->punkUser )
            {
                IDirectMusicSegmentState8* pSegmentState   = NULL;
                IDirectMusicSegment*       pNotifySegment   = NULL;
                IDirectMusicSegment8*      pNotifySegment8  = NULL;
                IDirectMusicSegment8*      pPrimarySegment8 = NULL;

                // The pPMsg->punkUser contains a IDirectMusicSegmentState8, 
                // which we can query for the segment that the SegmentState refers to.
                if( FAILED( hr = pPMsg->punkUser->QueryInterface( IID_IDirectMusicSegmentState8,
                                                                  (VOID**) &pSegmentState ) ) )
                    return DXTRACE_ERR( TEXT("QueryInterface"), hr );

                if( FAILED( hr = pSegmentState->GetSegment( &pNotifySegment ) ) )
                {
                    // Sometimes the segend arrives after the segment is gone
                    // This can happen when you load another segment as 
                    // a motif or the segment is ending
                    if( hr == DMUS_E_NOT_FOUND )
                    {
                        SAFE_RELEASE( pSegmentState );
                        return S_OK;
                    }

                    return DXTRACE_ERR( TEXT("GetSegment"), hr );
                }

                if( FAILED( hr = pNotifySegment->QueryInterface( IID_IDirectMusicSegment8,
                                                                 (VOID**) &pNotifySegment8 ) ) )
                    return DXTRACE_ERR( TEXT("QueryInterface"), hr );

                // Get the IDirectMusicSegment for the primary segment
                pPrimarySegment8 = g_pSegment->GetSegment();

                // Figure out which segment this is
                if( pNotifySegment8 == pPrimarySegment8 )
                {
                    // Update the UI controls to show the sound as stopped
                    EnablePlayUI( hDlg, TRUE );
                }

                // Cleanup
                SAFE_RELEASE( pSegmentState );
                SAFE_RELEASE( pNotifySegment );
                SAFE_RELEASE( pNotifySegment8 );
            }
            break;
        }

        pPerf->FreePMsg( (DMUS_PMSG*)pPMsg ); 
    }

    return S_OK;
}

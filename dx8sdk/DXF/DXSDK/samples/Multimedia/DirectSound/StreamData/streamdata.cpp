//----------------------------------------------------------------------------
// File: StreamData.cpp
//
// Desc: The StreamData sample shows how to streaming wave data into 
//       a DirectSound buffer.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <mmreg.h>
#include <dxerr8.h>
#include <dsound.h>
#include "resource.h"
#include "DSUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
DWORD WINAPI NotificationProc( LPVOID lpParameter );
VOID    OnInitDialog( HWND hDlg );
HRESULT InitDirectSound( HWND hDlg );
HRESULT FreeDirectSound();
VOID    OnOpenSoundFile( HWND hDlg );
VOID    LoadWaveAndCreateBuffer( HWND hDlg, TCHAR* strFileName );
HRESULT InitDSoundNotification();
HRESULT PlayBuffer( BOOL bLooped );
HRESULT HandleNotification( BOOL bLooped );
VOID    OnTimer( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define NUM_PLAY_NOTIFICATIONS  16

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

CSoundManager*      g_pSoundManager         = NULL;
CStreamingSound*    g_pStreamingSound       = NULL;
HANDLE              g_hNotificationEvent    = NULL;
DWORD               g_dwNotifyThreadID      = 0;
HANDLE              g_hNotifyThread         = NULL;
HINSTANCE           g_hInst                 = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    g_hInst = hInst;
    g_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    CloseHandle( g_hNotificationEvent );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: NotificationProc()
// Desc: Handles dsound notifcation events
//-----------------------------------------------------------------------------
DWORD WINAPI NotificationProc( LPVOID lpParameter )
{
    HRESULT hr;
    HWND    hDlg = (HWND) lpParameter;
    MSG     msg;
    DWORD   dwResult;
    BOOL    bDone = FALSE;
    BOOL    bLooped;

    while( !bDone ) 
    { 
        dwResult = MsgWaitForMultipleObjects( 1, &g_hNotificationEvent, 
                                              FALSE, INFINITE, QS_ALLEVENTS );
        switch( dwResult )
        {
            case WAIT_OBJECT_0 + 0:
                // g_hNotificationEvent is signaled

                // This means that DirectSound just finished playing 
                // a piece of the buffer, so we need to fill the circular 
                // buffer with new sound from the wav file
                bLooped = ( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK ) == BST_CHECKED );
                if( FAILED( hr = g_pStreamingSound->HandleWaveStreamNotification( bLooped ) ) )
                {
                    DXTRACE_ERR( TEXT("HandleWaveStreamNotification"), hr );
                    MessageBox( hDlg, "Error handling DirectSound notifications."
                               "Sample will now exit.", "DirectSound Sample", 
                               MB_OK | MB_ICONERROR );
                    bDone = TRUE;
                }

                break;

            case WAIT_OBJECT_0 + 1:
                // Messages are available
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                { 
                    if( msg.message == WM_QUIT )
                        bDone = TRUE;
                }
                break;
        }
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch (msg) 
    {
    case WM_INITDIALOG:
        OnInitDialog( hDlg );
        break;

    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
            case IDC_SOUNDFILE:
                OnOpenSoundFile( hDlg );
                break;

            case IDCANCEL:
                PostQuitMessage( 0 );
                EndDialog( hDlg, IDCANCEL );
                break;

            case IDC_PLAY:
                {
                    BOOL bLooped = ( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK ) == BST_CHECKED );

                    if( FAILED( hr = PlayBuffer( bLooped ) ) )
                    {
                        DXTRACE_ERR( TEXT("PlayBuffer"), hr );
                        MessageBox( hDlg, "Error playing DirectSound buffer."
                                    "Sample will now exit.", "DirectSound Sample", 
                                    MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, IDABORT );
                    }

                    // Update the UI controls to show the sound as playing
                    EnablePlayUI( hDlg, FALSE );
                }
                break;

            case IDC_STOP:
                if( g_pStreamingSound )
                {
                    g_pStreamingSound->Stop();
                    g_pStreamingSound->Reset();
                }

                EnablePlayUI( hDlg, TRUE );
                break;

            default:
                return FALSE; // Didn't handle message
        }
        break;

    case WM_TIMER:
        OnTimer( hDlg );
        break;

    case WM_DESTROY:
        // Cleanup everything
        KillTimer( hDlg, 1 );    
        SAFE_DELETE( g_pStreamingSound );
        SAFE_DELETE( g_pSoundManager );

        // Close down notification thread
        PostThreadMessage( g_dwNotifyThreadID, WM_QUIT, 0, 0 );
        WaitForSingleObject( g_hNotifyThread, INFINITE );
        CloseHandle( g_hNotifyThread );
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

    // Create a thread to handle DSound notifications
    g_hNotifyThread = CreateThread( NULL, 0, NotificationProc, 
                                    hDlg, 0, &g_dwNotifyThreadID );

    // Create a timer, so we can check for when the soundbuffer is stopped
    SetTimer( hDlg, 0, 250, NULL );

    // Set the UI controls
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("No file loaded.") );
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
        if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ))
            strcat( strPath, TEXT("\\") );
        strcat( strPath, TEXT("MEDIA") );
    }

    if( g_pStreamingSound )
    {
        g_pStreamingSound->Stop();
        g_pStreamingSound->Reset();
    }

    // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY ),       FALSE );
    EnableWindow( GetDlgItem( hDlg, IDC_STOP ),       FALSE );
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Loading file...") );

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Load aborted.") );
        return;
    }

    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

    // Load the wave file and create a DirectSound buffer
    LoadWaveAndCreateBuffer( hDlg, strFileName );

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}





//-----------------------------------------------------------------------------
// Name: LoadWaveAndCreateBuffer()
// Desc: Loads the wave file, and create a DirectSound buffer.  Since we are
//       streaming data into the buffer, the buffer will be filled with data 
//       when the sound is played, and as notification events are signaled
//-----------------------------------------------------------------------------
VOID LoadWaveAndCreateBuffer( HWND hDlg, TCHAR* strFileName )
{
    HRESULT   hr;
    CWaveFile waveFile;
    DWORD     dwNotifySize;

    // Load the wave file
    if( FAILED( hr = waveFile.Open( strFileName, NULL, WAVEFILE_READ ) ) )
    {
        DXTRACE_ERR( TEXT("Open"), hr );
        waveFile.Close();
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Bad wave file.") );
        return;
    }

    if( waveFile.GetSize() == 0 )
    {
        waveFile.Close();
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Wave file blank.") );
        return;
    }

    // The wave file is valid, and waveFile.m_pwfx is the wave's format
    // so we are done with the reader.
    waveFile.Close();

    // This samples works by dividing a 3 second streaming buffer into 
    // NUM_PLAY_NOTIFICATIONS (16) pieces.  It creates a notification for each
    // piece and when a notification arrives then it fills the circular streaming 
    // buffer with new wav data over the sound data which was just played

    // Determine the g_dwNotifySize.  It should be an integer multiple of nBlockAlign
    DWORD nBlockAlign = (DWORD)waveFile.m_pwfx->nBlockAlign;
    INT nSamplesPerSec = waveFile.m_pwfx->nSamplesPerSec;
    dwNotifySize = nSamplesPerSec * 3 * nBlockAlign / NUM_PLAY_NOTIFICATIONS;
    dwNotifySize -= dwNotifySize % nBlockAlign;   

    // Create a new sound
    SAFE_DELETE( g_pStreamingSound );

    // Set up the direct sound buffer.  Request the NOTIFY flag, so
    // that we are notified as the sound buffer plays.  Note, that using this flag
    // may limit the amount of hardware acceleration that can occur. 
    if( FAILED( hr = g_pSoundManager->CreateStreaming( &g_pStreamingSound, strFileName, 
                                                  DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2, 
                                                  GUID_NULL, NUM_PLAY_NOTIFICATIONS, 
                                                  dwNotifySize, g_hNotificationEvent ) ) )
    {
        if( hr != DSERR_BADFORMAT && hr != E_INVALIDARG )
            DXTRACE_ERR( TEXT("CreateStreaming"), hr );

        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create sound buffer.") );
        return;
    }

    // Update the UI controls to show the sound as the file is loaded
    EnablePlayUI( hDlg, TRUE );
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
}




//-----------------------------------------------------------------------------
// Name: PlayBuffer()
// Desc: Reset the buffer, fill it with sound, and starting it playing
//-----------------------------------------------------------------------------
HRESULT PlayBuffer( BOOL bLooped )
{
    HRESULT hr;
    
    if( NULL == g_pStreamingSound )
        return E_FAIL; // Sanity check

    if( FAILED( hr = g_pStreamingSound->Reset() ) )
        return DXTRACE_ERR( TEXT("Reset"), hr );

    // Fill the entire buffer with wave data, and if the wav file is small then
    // repeat the wav file if the user wants to loop the file, otherwise fill in
    // silence 
    LPDIRECTSOUNDBUFFER pDSB = g_pStreamingSound->GetBuffer( 0 );
    if( FAILED( hr = g_pStreamingSound->FillBufferWithSound( pDSB, bLooped ) ) )
        return DXTRACE_ERR( TEXT("FillBufferWithSound"), hr );

    // Always play with the LOOPING flag since the streaming buffer
    // wraps around before the entire WAV is played
    if( FAILED( hr = g_pStreamingSound->Play( 0, DSBPLAY_LOOPING ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

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
        if( !g_pStreamingSound->IsSoundPlaying() ) 
        {
            // Update the UI controls to show the sound as stopped
            EnablePlayUI( hDlg, TRUE );
        }
    }
}




//-----------------------------------------------------------------------------
// Name: EnablePlayUI( hDlg,)
// Desc: Enables or disables the Play UI controls 
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_STOP ),       FALSE );
        SetFocus(     GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_LOOP_CHECK ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY ),       FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_STOP ),       TRUE );
        SetFocus(     GetDlgItem( hDlg, IDC_STOP ) );
    }
}





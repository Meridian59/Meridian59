//----------------------------------------------------------------------------
// File: FullDuplexFilter.cpp
//
// Desc: The FullDuplexFilter sample shows how to use DirectSound to implement 
//       full duplex audio.
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
#include "resource.h"
#include "dsutil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT InitDirectSound( HWND hDlg );
HRESULT FreeDirectSound();

INT_PTR CALLBACK FormatsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitFormatsDialog( HWND hDlg );
HRESULT ScanAvailableOutputFormats();
VOID    GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx );
HRESULT FillFormatListBox( HWND hListBox, BOOL* aFormatSupported );
VOID    ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName );
HRESULT OnOutputFormatBoxSelected( HWND hDlg );
HRESULT ScanAvailableInputFormats();
HRESULT OnInputFormatBoxSelected( HWND hDlg );
HRESULT OnFormatsOK( HWND hDlg );
HRESULT SetBufferFormats( WAVEFORMATEX* pwfxInput, WAVEFORMATEX* pwfxOutput );
HRESULT CreateOutputBuffer();

INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT SetMainDialogText( HWND hDlg );
HRESULT OnInitMainDialog( HWND hDlg );
HRESULT StartBuffers();
HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSBuffer, BOOL* pbRestored );

HRESULT HandleNotification();
HRESULT TransformData( BYTE* pbOut, BYTE* pbIn, DWORD dwLength );



//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define NUM_PLAY_NOTIFICATIONS  16
#define NUM_BUFFERS     (16)
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTSOUND              g_pDS            = NULL;
LPDIRECTSOUNDCAPTURE       g_pDSCapture     = NULL;
LPDIRECTSOUNDBUFFER        g_pDSBPrimary    = NULL;
LPDIRECTSOUNDBUFFER        g_pDSBOutput     = NULL;
LPDIRECTSOUNDCAPTUREBUFFER g_pDSBCapture    = NULL;
LPDIRECTSOUNDNOTIFY        g_pDSNotify      = NULL;

DSBPOSITIONNOTIFY    g_aPosNotify[ NUM_PLAY_NOTIFICATIONS ];  
HANDLE               g_hNotificationEvent; 
BOOL                 g_abOutputFormatSupported[16];
BOOL                 g_abInputFormatSupported[16];
DWORD                g_dwOutputBufferSize;
DWORD                g_dwCaptureBufferSize;
DWORD                g_dwNextOutputOffset;
DWORD                g_dwNextCaptureOffset;
DWORD                g_dwNotifySize;
WAVEFORMATEX         g_wfxCaptureWaveFormat;
BOOL                 g_bRecording;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    DWORD   dwResult;
    MSG     msg;
    BOOL    bDone;
    HRESULT hr;
    HWND    hDlg;
    
    g_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Display the main dialog box.
    hDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    // Init DirectSound
    if( FAILED( hr = InitDirectSound( hDlg ) ) )
    {
        DXTRACE_ERR( TEXT("InitDirectSound"), hr );
        if( hr == DSERR_ALLOCATED )
        {
            MessageBox( hDlg, "Full duplex audio failed. "
                        "Make sure the sound card supports this operation. "
                        "The sample will now exit.", "DirectSound Sample", 
                        MB_OK | MB_ICONERROR );
            EndDialog( hDlg, IDABORT );
        }
        else
        {
            MessageBox( hDlg, "Error initializing DirectSound. "
                        "The sample will now exit.", "DirectSound Sample", 
                        MB_OK | MB_ICONERROR );
            EndDialog( hDlg, IDABORT );
        }
        return 1;
    }

    // Display the main dialog box.
    dwResult = (DWORD)DialogBox( hInst, MAKEINTRESOURCE(IDD_FORMATS), NULL, FormatsDlgProc );

    if( dwResult != IDOK )
    {
        // The user canceled, so stop message pump, 
        // and fall through to the cleanup code
        PostQuitMessage( 0 );
    }
    else
    {
        if( FAILED( hr = CreateOutputBuffer() ) )
        {
            DXTRACE_ERR( TEXT("CreateOutputBuffer"), hr );
            MessageBox( hDlg, "Error creating output buffer. "
                       "The sample will now exit.", "DirectSound Sample", 
                       MB_OK | MB_ICONERROR );
            EndDialog( hDlg, IDABORT );
        }

        SetMainDialogText( hDlg );

        g_bRecording = FALSE;

        ShowWindow( hDlg, SW_SHOW ); 
    }

    bDone = FALSE;
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
                if( FAILED( hr = HandleNotification() ) )
                {
                    DXTRACE_ERR( TEXT("HandleNotification"), hr );
                    MessageBox( hDlg, "Error handling DirectSound notifications. "
                               "The sample will now exit.", "DirectSound Sample", 
                               MB_OK | MB_ICONERROR );
                    bDone = TRUE;
                }

                break;

            case WAIT_OBJECT_0 + 1:
                // Messages are available
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                { 
                    if( !IsDialogMessage( hDlg, &msg ) )  
                    {
                        TranslateMessage( &msg ); 
                        DispatchMessage( &msg ); 
                    }

                    if( msg.message == WM_QUIT )
                        bDone = TRUE;
                }
                break;
        }
    }

    // Clean up everything
    FreeDirectSound();

    CloseHandle( g_hNotificationEvent );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT InitDirectSound( HWND hDlg )
{
    HRESULT             hr;
    DSBUFFERDESC        dsbdesc;

    ZeroMemory( &g_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
                               (NUM_PLAY_NOTIFICATIONS) );
    g_dwOutputBufferSize  = 0;
    g_dwCaptureBufferSize = 0;
    g_dwNotifySize        = 0;
    g_dwNextOutputOffset  = 0;

    // Initialize COM
    if( FAILED( hr = CoInitialize(NULL) ) )
        return DXTRACE_ERR( TEXT("CoInitialize"), hr );

    // Create IDirectSound using the preferred sound device
    if( FAILED( hr = DirectSoundCreate( NULL, &g_pDS, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCreate"), hr );

    // Set coop level to DSSCL_PRIORITY
    if( FAILED( hr = g_pDS->SetCooperativeLevel( hDlg, DSSCL_PRIORITY ) ) )
        return DXTRACE_ERR( TEXT("SetCooperativeLevel"), hr );

    // Obtain primary buffer 
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize  = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if( FAILED( hr = g_pDS->CreateSoundBuffer( &dsbdesc, &g_pDSBPrimary, NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );

    // Create IDirectSoundCapture using the preferred capture device
    if( FAILED( hr = DirectSoundCaptureCreate( NULL, &g_pDSCapture, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCaptureCreate"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT FreeDirectSound()
{
    // Release DirectSound interfaces
    SAFE_RELEASE( g_pDSNotify );

    SAFE_RELEASE( g_pDSBPrimary );
    SAFE_RELEASE( g_pDSBOutput );
    SAFE_RELEASE( g_pDSBCapture );

    SAFE_RELEASE( g_pDSCapture ); 
    SAFE_RELEASE( g_pDS ); 

    // Release COM
    CoUninitialize();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FormatsDlgProc()
// Desc: Handles dialog messages for formats dlg box
//-----------------------------------------------------------------------------
INT_PTR CALLBACK FormatsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            if( FAILED( hr = OnInitFormatsDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitFormatsDialog"), hr );
                MessageBox( hDlg, "Error scanning DirectSound formats. "
                            "The sample will now exit.", "DirectSound Sample", 
                            MB_OK | MB_ICONERROR );
                EndDialog( hDlg, IDABORT );
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                case IDOK:
                    if( FAILED( hr = OnFormatsOK( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnFormatsOK"), hr );
                        if( hr == DSERR_ALLOCATED )
                        {
                            MessageBox( hDlg, "Full duplex audio failed. "
                                        "Make sure the sound card supports this operation. "
                                        "The sample will now exit.", "DirectSound Sample", 
                                        MB_OK | MB_ICONERROR );
                        }
                        else
                        {
                            MessageBox( hDlg, "Error accepting DirectSound formats. "
                                        "The sample will now exit.", "DirectSound Sample", 
                                        MB_OK | MB_ICONERROR );
                        }
                        EndDialog( hDlg, IDABORT );
                    }
                    break;

                case IDC_FORMATS_INPUT_LISTBOX:
                    OnInputFormatBoxSelected( hDlg );
                    break;
            
                case IDC_FORMATS_OUTPUT_LISTBOX:
                    if( HIWORD(wParam) == LBN_SELCHANGE )
                    {
                        OnOutputFormatBoxSelected( hDlg );
                    }
    
                    break;

                default:
                    return FALSE; // Didn't handle message
            }   
            break;

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitFormatsDialog()
// Desc: Initializes the formats dialog
//-----------------------------------------------------------------------------
HRESULT OnInitFormatsDialog( HWND hDlg )
{
    HRESULT hr;

    if( FAILED( hr = ScanAvailableOutputFormats() ) )
        return DXTRACE_ERR( TEXT("ScanAvailableOutputFormats"), hr );

    HWND hOutputList = GetDlgItem( hDlg, IDC_FORMATS_OUTPUT_LISTBOX );
    FillFormatListBox( hOutputList, g_abOutputFormatSupported );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ScanAvailableOutputFormats()
// Desc: Tests to see if 16 different standard wave formats are supported by
//       the playback device 
//-----------------------------------------------------------------------------
HRESULT ScanAvailableOutputFormats()
{
    WAVEFORMATEX wfx;
    WAVEFORMATEX wfxSet;
    HRESULT      hr;
    HCURSOR      hCursor;
    
    // This might take a second or two, so throw up the hourglass
    hCursor = GetCursor();
    SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    
    ZeroMemory( &wfxSet, sizeof(wfxSet) );
    wfxSet.wFormatTag = WAVE_FORMAT_PCM;

    ZeroMemory( &wfx, sizeof(wfx) );
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    
    // Try 16 different standard format to see if they are supported
    for( INT iIndex = 0; iIndex < 16; iIndex++ )
    {
        GetWaveFormatFromIndex( iIndex, &wfx );

        // To test if a playback format is supported, try to set the format 
        // using a specific format.  If it works then the format is 
        // supported, otherwise not.
        if( FAILED( hr = g_pDSBPrimary->SetFormat( &wfx ) ) )
        {
            g_abOutputFormatSupported[ iIndex ] = FALSE;
        }
        else
        {
            // Get the format that was just set, and see if it 
            // is actually supported since SetFormat() sometimes returns DS_OK 
            // even if the format was not supported
            if( FAILED( hr = g_pDSBPrimary->GetFormat( &wfxSet, sizeof(wfxSet), 
                                                       NULL ) ) )
                return DXTRACE_ERR( TEXT("GetFormat"), hr );

            if( memcmp( &wfx, &wfxSet, sizeof(wfx) ) == 0 )
                g_abOutputFormatSupported[ iIndex ] = TRUE;
            else
                g_abOutputFormatSupported[ iIndex ] = FALSE;    
        }
    }

    SetCursor( hCursor );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetWaveFormatFromIndex()
// Desc: Returns 16 different wave formats based on nIndex
//-----------------------------------------------------------------------------
VOID GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx )
{
    INT iSampleRate = nIndex / 4;
    INT iType = nIndex % 4;

    switch( iSampleRate )
    {
        case 0: pwfx->nSamplesPerSec =  8000; break;
        case 1: pwfx->nSamplesPerSec = 11025; break;
        case 2: pwfx->nSamplesPerSec = 22050; break;
        case 3: pwfx->nSamplesPerSec = 44100; break;
    }

    switch( iType )
    {
        case 0: pwfx->wBitsPerSample =  8; pwfx->nChannels = 1; break;
        case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
        case 2: pwfx->wBitsPerSample =  8; pwfx->nChannels = 2; break;
        case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
    }

    pwfx->nBlockAlign = pwfx->nChannels * ( pwfx->wBitsPerSample / 8 );
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
}




//-----------------------------------------------------------------------------
// Name: FillFormatListBox()
// Desc: Initializes the main dialog
//-----------------------------------------------------------------------------
HRESULT FillFormatListBox( HWND hListBox, BOOL* aFormatSupported )
{
    TCHAR        strFormatName[255];
    WAVEFORMATEX wfx;
    DWORD        dwStringIndex;

    SendMessage( hListBox, LB_RESETCONTENT, 0, 0 );

    for( INT iIndex = 0; iIndex < 16; iIndex++ )
    {
        if( aFormatSupported[ iIndex ] )
        {
            GetWaveFormatFromIndex( iIndex, &wfx );
            ConvertWaveFormatToString( &wfx, strFormatName );

            dwStringIndex = (DWORD)SendMessage( hListBox, LB_ADDSTRING, 0, 
                                                (LPARAM) (LPCTSTR) strFormatName );

            SendMessage( hListBox, LB_SETITEMDATA, dwStringIndex, iIndex );
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConvertWaveFormatToString()
// Desc: Converts a format to a text string
//-----------------------------------------------------------------------------
VOID ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName )
{
    wsprintf( strFormatName, 
              TEXT("%u Hz, %u-bit %s"), 
              pwfx->nSamplesPerSec, 
              pwfx->wBitsPerSample, 
              ( pwfx->nChannels == 1 ) ? TEXT("Mono") : TEXT("Stereo") );
}




//-----------------------------------------------------------------------------
// Name: OnOutputFormatBoxSelected()
// Desc: When the output format is selected, then we can enumerate the
//       the availible capture formats that are supported
//-----------------------------------------------------------------------------
HRESULT OnOutputFormatBoxSelected( HWND hDlg )
{
    HRESULT      hr;
    WAVEFORMATEX wfx;
    HWND         hOutputList;
    DWORD        dwOutputSelect;
    DWORD        dwOutputWavIndex;
    HWND         hOK;

    hOK         = GetDlgItem( hDlg, IDOK );
    hOutputList = GetDlgItem( hDlg, IDC_FORMATS_OUTPUT_LISTBOX );

    EnableWindow( hOK, FALSE );

    dwOutputSelect = (DWORD)SendMessage( hOutputList, LB_GETCURSEL, 0, 0 );
    dwOutputWavIndex = (DWORD)SendMessage( hOutputList, LB_GETITEMDATA, dwOutputSelect, 0 );

    ZeroMemory( &wfx, sizeof(wfx) );
    wfx.wFormatTag = WAVE_FORMAT_PCM;

    GetWaveFormatFromIndex( dwOutputWavIndex, &wfx );

    if( FAILED( hr = g_pDSBPrimary->SetFormat( &wfx ) ) )
        return DXTRACE_ERR( TEXT("SetFormat"), hr );

    if( FAILED( hr = ScanAvailableInputFormats() ) )
        return DXTRACE_ERR( TEXT("ScanAvailableInputFormats"), hr );

    HWND hInputList = GetDlgItem( hDlg, IDC_FORMATS_INPUT_LISTBOX );
    FillFormatListBox( hInputList, g_abInputFormatSupported );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ScanAvailableInputFormats()
// Desc: Tests to see if 16 different standard wave formats are supported by
//       the capture device 
//-----------------------------------------------------------------------------
HRESULT ScanAvailableInputFormats()
{
    HRESULT       hr;
    WAVEFORMATEX  wfx;
    HCURSOR       hCursor;
    DSCBUFFERDESC dscbd;
    LPDIRECTSOUNDCAPTUREBUFFER pDSCaptureBuffer = NULL;
    
    // This might take a second or two, so throw up the hourglass
    hCursor = GetCursor();
    SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    
    ZeroMemory( &wfx, sizeof(wfx) );
    wfx.wFormatTag = WAVE_FORMAT_PCM;

    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize = sizeof(dscbd);

    // Try 16 different standard format to see if they are supported
    for( INT iIndex = 0; iIndex < 16; iIndex++ )
    {
        GetWaveFormatFromIndex( iIndex, &wfx );

        // To test if a capture format is supported, try to create a 
        // new capture buffer using a specific format.  If it works
        // then the format is supported, otherwise not.
        dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
        dscbd.lpwfxFormat = &wfx;
        
        if( FAILED( hr = g_pDSCapture->CreateCaptureBuffer( &dscbd, 
                                                            &pDSCaptureBuffer, 
                                                            NULL ) ) )
            g_abInputFormatSupported[ iIndex ] = FALSE;
        else
            g_abInputFormatSupported[ iIndex ] = TRUE;

        SAFE_RELEASE( pDSCaptureBuffer );
    }

    SetCursor( hCursor );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnInputFormatBoxSelected()
// Desc: When both format boxes have a selection then enable the OK button
//-----------------------------------------------------------------------------
HRESULT OnInputFormatBoxSelected( HWND hDlg )
{
    HWND hInputList  = GetDlgItem( hDlg, IDC_FORMATS_INPUT_LISTBOX );
    HWND hOutputList = GetDlgItem( hDlg, IDC_FORMATS_OUTPUT_LISTBOX );
    HWND hOK         = GetDlgItem( hDlg, IDOK );

    if( SendMessage( hInputList,  LB_GETCURSEL, 0, 0 ) != -1 &&
        SendMessage( hOutputList, LB_GETCURSEL, 0, 0 ) != -1 )
    {
        EnableWindow( hOK, TRUE );
    }
    else
    {
        EnableWindow( hOK, FALSE );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnFormatsOK()
// Desc: Sets the formats of the capture and sound buffer based on the UI
//-----------------------------------------------------------------------------
HRESULT OnFormatsOK( HWND hDlg )
{
    HRESULT       hr;
    DWORD         dwOutputSelect;
    DWORD         dwInputSelect;
    DWORD         dwOutputWavIndex;
    DWORD         dwInputWavIndex;
    WAVEFORMATEX  wfxInput;
    WAVEFORMATEX  wfxOutput;

    ZeroMemory( &wfxInput, sizeof(wfxInput) );
    wfxInput.wFormatTag = WAVE_FORMAT_PCM;

    ZeroMemory( &wfxOutput, sizeof(wfxOutput) );
    wfxOutput.wFormatTag = WAVE_FORMAT_PCM;

    HWND hInputList  = GetDlgItem( hDlg, IDC_FORMATS_INPUT_LISTBOX );
    HWND hOutputList = GetDlgItem( hDlg, IDC_FORMATS_OUTPUT_LISTBOX );

    dwOutputSelect   = (DWORD)SendMessage( hOutputList, LB_GETCURSEL, 0, 0 );
    dwOutputWavIndex = (DWORD)SendMessage( hOutputList, LB_GETITEMDATA, dwOutputSelect, 0 );

    dwInputSelect   = (DWORD)SendMessage( hInputList, LB_GETCURSEL, 0, 0 );
    dwInputWavIndex = (DWORD)SendMessage( hInputList, LB_GETITEMDATA, dwInputSelect, 0 );

    GetWaveFormatFromIndex( dwOutputWavIndex, &wfxOutput );
    GetWaveFormatFromIndex( dwInputWavIndex, &wfxInput );

    if( FAILED( hr = SetBufferFormats( &wfxInput, &wfxOutput ) ) )
        return DXTRACE_ERR( TEXT("SetBufferFormats"), hr );

    EndDialog( hDlg, IDOK );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetBufferFormats()
// Desc: Sets the buffer formats for the primary buffer, and the capture buffer
//-----------------------------------------------------------------------------
HRESULT SetBufferFormats( WAVEFORMATEX* pwfxInput, 
                          WAVEFORMATEX* pwfxOutput )
{
    HRESULT hr;

    // Set the format of the primary buffer 
    // to the format of the output buffer
    if( FAILED( hr = g_pDSBPrimary->SetFormat( pwfxOutput ) ) )
        return DXTRACE_ERR( TEXT("SetFormat"), hr );

    // Set the notification size
    g_dwNotifySize = MAX( 4096, pwfxInput->nAvgBytesPerSec / 8 );
    g_dwNotifySize -= g_dwNotifySize % pwfxInput->nBlockAlign;   

    // Set the buffer sizes 
    g_dwOutputBufferSize  = NUM_BUFFERS * g_dwNotifySize / 2;
    g_dwCaptureBufferSize = g_dwNotifySize * NUM_BUFFERS;

    SAFE_RELEASE( g_pDSBCapture );

    // Create the capture buffer
    DSCBUFFERDESC dscbd;
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = g_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pwfxInput; // Set the format during creatation

    if( FAILED( hr = g_pDSCapture->CreateCaptureBuffer( &dscbd, 
                                                        &g_pDSBCapture, 
                                                        NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateCaptureBuffer"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetMainDialogText()
// Desc: Sets the static text of the main dialog based on the formats
//-----------------------------------------------------------------------------
HRESULT SetMainDialogText( HWND hDlg )
{
    WAVEFORMATEX  wfxInput;
    WAVEFORMATEX  wfxOutputPri;
    WAVEFORMATEX  wfxOutputSec;

    ZeroMemory( &wfxInput,     sizeof(wfxInput) );
    ZeroMemory( &wfxOutputPri, sizeof(wfxOutputPri) );
    ZeroMemory( &wfxOutputSec, sizeof(wfxOutputSec) );

    g_pDSBCapture->GetFormat( &wfxInput,     sizeof(wfxInput), NULL );
    g_pDSBPrimary->GetFormat( &wfxOutputPri, sizeof(wfxOutputPri), NULL );
    g_pDSBOutput->GetFormat(  &wfxOutputSec, sizeof(wfxOutputSec), NULL );

    TCHAR strInputFormat[255];
    TCHAR strOutputSecFormat[255];
    TCHAR strOutputPriFormat[255];

    ConvertWaveFormatToString( &wfxInput, strInputFormat );
    ConvertWaveFormatToString( &wfxOutputPri, strOutputPriFormat );
    ConvertWaveFormatToString( &wfxOutputSec, strOutputSecFormat );
    
    HWND hInputFormatText     = GetDlgItem( hDlg, IDC_MAIN_INPUTFORMAT_TEXT );
    HWND hPrimaryOutputText   = GetDlgItem( hDlg, IDC_MAIN_PRIMARYFORMAT_TEXT );
    HWND hSecondaryOutputText = GetDlgItem( hDlg, IDC_MAIN_SECONDARYFORMAT_TEXT );    

    SetWindowText( hInputFormatText, strInputFormat );
    SetWindowText( hPrimaryOutputText, strOutputPriFormat );
    SetWindowText( hSecondaryOutputText, strOutputSecFormat );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateOutputBuffer()
// Desc: Creates the ouptut buffer and sets up the notification positions
//       on the capture buffer
//-----------------------------------------------------------------------------
HRESULT CreateOutputBuffer()
{
    HRESULT hr; 
    WAVEFORMATEX wfxInput;

    // This sample works by creating notification events which 
    // are signaled when the capture buffer reachs specific offsets 
    // WinMain() waits for the associated event to be signaled, and
    // when it is, it calls HandleNotifications() which copy the 
    // data from the capture buffer into the output buffer

    ZeroMemory( &wfxInput, sizeof(wfxInput) );
    g_pDSBCapture->GetFormat( &wfxInput, sizeof(wfxInput), NULL );

    // Create the direct sound buffer using the same format as the 
    // capture buffer. 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = DSBCAPS_GLOBALFOCUS;
    dsbd.dwBufferBytes   = g_dwOutputBufferSize;
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = &wfxInput;

    // Create the DirectSound buffer 
    if( FAILED( hr = g_pDS->CreateSoundBuffer( &dsbd, &g_pDSBOutput, NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );

    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = g_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
                                                    (VOID**)&g_pDSNotify ) ) )
        return DXTRACE_ERR( TEXT("QueryInterface"), hr );

    // Setup the notification positions
    for( INT i = 0; i < NUM_PLAY_NOTIFICATIONS; i++ )
    {
        g_aPosNotify[i].dwOffset = (g_dwNotifySize * i) + g_dwNotifySize - 1;
        g_aPosNotify[i].hEventNotify = g_hNotificationEvent;             
    }
    
    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    if( FAILED( hr = g_pDSNotify->SetNotificationPositions( NUM_PLAY_NOTIFICATIONS, 
                                                            g_aPosNotify ) ) )
        return DXTRACE_ERR( TEXT("SetNotificationPositions"), hr );

    return S_OK;
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
            OnInitMainDialog( hDlg );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    PostQuitMessage( 0 );
                    EndDialog( hDlg, IDCANCEL );
                    break; 

                case IDC_RECORD:
                    g_bRecording = !g_bRecording;

                    if( g_bRecording )
                    {
                        if( FAILED( hr = StartBuffers() ) )
                        {
                            DXTRACE_ERR( TEXT("StartBuffers"), hr );
                            MessageBox( hDlg, "Error starting DirectSound buffers. "                            
                                        "The sample will now exit.", "DirectSound Sample", 
                                        MB_OK | MB_ICONERROR );
                            PostQuitMessage( 0 );
                            EndDialog( hDlg, IDABORT );
                        }
                    }
                    else
                    {
                        if( g_pDSBCapture && g_pDSBOutput )
                        {
                            g_pDSBCapture->Stop();
                            g_pDSBOutput->Stop();
                        }
                    }
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_ACTIVATE:
            if( LOWORD(wParam) == WA_INACTIVE )
            {
                if( g_pDSBCapture && g_pDSBOutput )
                {
                    g_pDSBCapture->Stop();
                    g_pDSBOutput->Stop();
                }
            }
            else
            {
                if( g_bRecording )
                {
                    if( FAILED( hr = StartBuffers() ) )
                    {
                        DXTRACE_ERR( TEXT("StartBuffers"), hr );
                        MessageBox( hDlg, "Error starting DirectSound buffers. "                            
                                    "The sample will now exit.", "DirectSound Sample", 
                                    MB_OK | MB_ICONERROR );
                        PostQuitMessage( 0 );
                        EndDialog( hDlg, IDABORT );
                    }
                }
            }
            break;

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitMainDialog()
// Desc: Initializes the main dialog
//-----------------------------------------------------------------------------
HRESULT OnInitMainDialog( HWND hDlg )
{
    // Store HWND in global
    hDlg = hDlg;

    // Load the icon
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StartBuffers()
// Desc: Start the capture buffer, and the start playing the output buffer
//-----------------------------------------------------------------------------
HRESULT StartBuffers()
{
    WAVEFORMATEX wfxOutput;
    VOID*        pDSLockedBuffer = NULL;
    DWORD        dwDSLockedBufferSize;
    HRESULT hr;

    // Restore lost buffers
    if( FAILED( hr = RestoreBuffer( g_pDSBOutput, NULL ) ) )
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    // Find out where the capture buffer is right now, then write data 
    // some extra amount forward to make sure we're ahead of the write cursor
    g_pDSBCapture->GetCurrentPosition( &g_dwNextCaptureOffset, NULL );
    g_dwNextCaptureOffset -= g_dwNextCaptureOffset % g_dwNotifySize;

    g_dwNextOutputOffset = g_dwNextCaptureOffset + ( 2 * g_dwNotifySize );
    g_dwNextOutputOffset %= g_dwOutputBufferSize;  // Circular buffer

    // Tell the capture buffer to start recording
    g_pDSBCapture->Start( DSCBSTART_LOOPING );
    
    // Rewind the output buffer, fill it with silence, and play it
    g_pDSBOutput->SetCurrentPosition( g_dwNextCaptureOffset );

    // Save the format of the capture buffer in g_pCaptureWaveFormat
    ZeroMemory( &g_wfxCaptureWaveFormat, sizeof(WAVEFORMATEX) );
    g_pDSBCapture->GetFormat( &g_wfxCaptureWaveFormat, sizeof(WAVEFORMATEX), NULL );

    // Get the format of the output buffer
    ZeroMemory( &wfxOutput, sizeof(wfxOutput) );
    g_pDSBOutput->GetFormat( &wfxOutput, sizeof(wfxOutput), NULL );

    // Fill the output buffer with silence at first
    // As capture data arrives, HandleNotifications() will fill
    // the output buffer with wave data.
    if( FAILED( hr = g_pDSBOutput->Lock( 0, g_dwOutputBufferSize, 
                                         &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                         NULL, NULL, 0 ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );
    FillMemory( (BYTE*) pDSLockedBuffer, dwDSLockedBufferSize, 
                (BYTE)( wfxOutput.wBitsPerSample == 8 ? 128 : 0 ) );
    g_pDSBOutput->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, NULL ); 

    // Play the output buffer 
    g_pDSBOutput->Play( 0, 0, DSBPLAY_LOOPING );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreBuffer()
// Desc: Restores a lost buffer. *pbWasRestored returns TRUE if the buffer was 
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSBuffer, BOOL* pbRestored )
{
    HRESULT hr;

    if( pbRestored != NULL )
        *pbRestored = FALSE;

    if( NULL == pDSBuffer )
        return S_FALSE;

    DWORD dwStatus;
    if( FAILED( hr = pDSBuffer->GetStatus( &dwStatus ) ) )
        return DXTRACE_ERR( TEXT("GetStatus"), hr );

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr = pDSBuffer->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( hr = pDSBuffer->Restore() );

        if( pbRestored != NULL )
            *pbRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: HandleNotification()
// Desc: Handle the notification that tells us to copy data from the 
//       capture buffer to the output buffer 
//-----------------------------------------------------------------------------
HRESULT HandleNotification() 
{
    HRESULT hr;
    VOID* pDSCaptureLockedBuffer    = NULL;
    VOID* pDSOutputLockedBuffer     = NULL;
    DWORD dwDSCaptureLockedBufferSize;
    DWORD dwDSOutputLockedBufferSize;
   
    DWORD dwStatus;

    // Make sure buffers were not lost, if the were we need 
    // to start the capture again
    if( FAILED( hr = g_pDSBOutput->GetStatus( &dwStatus ) ) )
        return DXTRACE_ERR( TEXT("GetStatus"), hr );

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        if( FAILED( hr = StartBuffers() ) )
            return DXTRACE_ERR( TEXT("StartBuffers"), hr );

        return S_OK;
    }

    // Lock the capture buffer down
    if( FAILED( hr = g_pDSBCapture->Lock( g_dwNextCaptureOffset, g_dwNotifySize, 
                                          &pDSCaptureLockedBuffer, 
                                          &dwDSCaptureLockedBufferSize, 
                                          NULL, NULL, 0L ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );

    // Lock the output buffer down
    if( FAILED( hr = g_pDSBOutput->Lock( g_dwNextOutputOffset, g_dwNotifySize, 
                                         &pDSOutputLockedBuffer, 
                                         &dwDSOutputLockedBufferSize, 
                                         NULL, NULL, 0L ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );

    // These should be equal
    if( dwDSOutputLockedBufferSize != dwDSCaptureLockedBufferSize )
        return E_FAIL;  // Sanity check unhandled case 

    // Just copy the memory from the 
    // capture buffer to the playback buffer 
    CopyMemory( pDSOutputLockedBuffer, 
                pDSCaptureLockedBuffer, 
                dwDSOutputLockedBufferSize );

    // Unlock the play buffer
    g_pDSBOutput->Unlock( pDSOutputLockedBuffer, dwDSOutputLockedBufferSize, 
                          NULL, 0 );

    // Unlock the capture buffer
    g_pDSBCapture->Unlock( pDSCaptureLockedBuffer, dwDSCaptureLockedBufferSize, 
                           NULL, 0 );

    // Move the capture offset along
    g_dwNextCaptureOffset += dwDSCaptureLockedBufferSize; 
    g_dwNextCaptureOffset %= g_dwCaptureBufferSize; // Circular buffer

    // Move the playback offset along
    g_dwNextOutputOffset += dwDSOutputLockedBufferSize; 
    g_dwNextOutputOffset %= g_dwOutputBufferSize; // Circular buffer

    return S_OK;
}





//----------------------------------------------------------------------------
// File: CaptureSound.cpp
//
// Desc: The CaptureSound sample shows how to use DirectSoundCapture to capture 
//       sound into a wave file 
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
HRESULT InitDirectSound( HWND hDlg, GUID* pDeviceGuid );
HRESULT FreeDirectSound();

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
                                     VOID* pContext );
INT_PTR CALLBACK DevicesDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK FormatsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDevicesDialog( HWND hDlg );
HRESULT OnInitFormatsDialog( HWND hDlg );
HRESULT ScanAvailableInputFormats();
VOID    GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx );
HRESULT FillFormatListBox( HWND hListBox, BOOL* aFormatSupported );
VOID    ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName );
HRESULT OnInputFormatBoxSelected( HWND hDlg );
HRESULT OnFormatsOK( HWND hDlg );

INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitMainDialog( HWND hDlg );
VOID    OnSaveSoundFile( HWND hDlg );
HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput );
HRESULT InitNotifications();
HRESULT StartOrStopRecord( BOOL bStartRecording );
HRESULT RecordCapturedData();




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define NUM_REC_NOTIFICATIONS  16
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTSOUNDCAPTURE       g_pDSCapture         = NULL;
LPDIRECTSOUNDCAPTUREBUFFER g_pDSBCapture        = NULL;
LPDIRECTSOUNDNOTIFY        g_pDSNotify          = NULL;
HINSTANCE                  g_hInst              = NULL;
GUID                       g_guidCaptureDevice  = GUID_NULL;
BOOL                       g_bRecording;
WAVEFORMATEX               g_wfxInput;
DSBPOSITIONNOTIFY          g_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];  
HANDLE                     g_hNotificationEvent; 
BOOL                       g_abInputFormatSupported[16];
DWORD                      g_dwCaptureBufferSize;
DWORD                      g_dwNextCaptureOffset;
DWORD                      g_dwNotifySize;
CWaveFile*                  g_pWaveFile;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    HRESULT hr;
    DWORD dwResult;
    MSG   msg;
    BOOL  bDone;
    HWND  hDlg;

    g_hInst = hInst;
    g_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Create the main dialog box, but keep it hidden for now
    hDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );


    // Display the formats dialog box, and show it
    dwResult = (DWORD)DialogBox( hInst, MAKEINTRESOURCE(IDD_DEVICES), 
                                 NULL, DevicesDlgProc );
    if( dwResult != IDOK )
    {
        // The user canceled, so stop message pump, 
        // and fall through to the cleanup code
        PostQuitMessage( 0 );
    }
    else
    {
        // Init DirectSound
        if( FAILED( hr = InitDirectSound( hDlg, &g_guidCaptureDevice ) ) )
        {
            DXTRACE_ERR( TEXT("InitDirectSound"), hr );
            MessageBox( hDlg, "Error initializing DirectSound.  Sample will now exit.", 
                        "DirectSound Sample", MB_OK | MB_ICONERROR );
            PostQuitMessage( 0 );
        }
    }


    // Display the formats dialog box, and show it
    if( dwResult == IDOK )
    {
        dwResult = (DWORD)DialogBox( hInst, MAKEINTRESOURCE(IDD_FORMATS), 
                                     NULL, FormatsDlgProc );
    }

    if( dwResult != IDOK )
    {
        // The user canceled, so stop message pump, 
        // and fall through to the cleanup code
        PostQuitMessage( 0 );
    }
    else
    {
        WAVEFORMATEX wfxInput;
        TCHAR        strInputFormat[255];
        HWND         hInputFormatText;

        hInputFormatText = GetDlgItem( hDlg, IDC_MAIN_INPUTFORMAT_TEXT );

        ZeroMemory( &wfxInput, sizeof(wfxInput));
        g_pDSBCapture->GetFormat( &wfxInput, sizeof(wfxInput), NULL );
        ConvertWaveFormatToString( &wfxInput, strInputFormat );   

        SetWindowText( hInputFormatText, strInputFormat );

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
                // g_hNotificationEvents[0] is signaled

                // This means that DirectSound just finished playing 
                // a piece of the buffer, so we need to fill the circular 
                // buffer with new sound from the wav file

                if( FAILED( hr = RecordCapturedData() ) )
                {
                    DXTRACE_ERR( TEXT("RecordCapturedData"), hr );
                    MessageBox( hDlg, "Error handling DirectSound notifications. "
                               "Sample will now exit.", "DirectSound Sample", 
                               MB_OK | MB_ICONERROR );
                    bDone = TRUE;
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
                        bDone = TRUE;
                }
                break;
        }
    }

    // Stop the capture and read any data that was not caught by a notification
    StartOrStopRecord( FALSE );

    // Clean up everything
    FreeDirectSound();

    CloseHandle( g_hNotificationEvent );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT InitDirectSound( HWND hDlg, GUID* pDeviceGuid )
{
    HRESULT hr;

    ZeroMemory( &g_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
                               (NUM_REC_NOTIFICATIONS + 1) );
    g_dwCaptureBufferSize = 0;
    g_dwNotifySize        = 0;
    g_pWaveFile           = NULL;

    // Initialize COM
    if( FAILED( hr = CoInitialize(NULL) ) )
        return DXTRACE_ERR( TEXT("CoInitialize"), hr );


    // Create IDirectSoundCapture using the preferred capture device
    if( FAILED( hr = DirectSoundCaptureCreate( pDeviceGuid, &g_pDSCapture, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCaptureCreate"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT FreeDirectSound()
{
    SAFE_DELETE( g_pWaveFile );

    // Release DirectSound interfaces
    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );
    SAFE_RELEASE( g_pDSCapture ); 

    // Release COM
    CoUninitialize();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DevicesDlgProc()
// Desc: Handles dialog messages for devices dlg box
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DevicesDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
    case WM_INITDIALOG:
        if( FAILED( hr = OnInitDevicesDialog( hDlg ) ) )
        {
            DXTRACE_ERR( TEXT("OnInitDevicesDialog"), hr );
            MessageBox( hDlg, "Error scanning DirectSoundCapture devices. "
                        "Sample will now exit.", "DirectSound Sample", 
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
            {
                HWND hCaptureDeviceCombo = GetDlgItem( hDlg, IDC_CAPTURE_DEVICE_COMBO );

                // Get the index of the currently selected devices
                INT nCaptureIndex = (INT)SendMessage( hCaptureDeviceCombo, CB_GETCURSEL, 0, 0 ); 

                // Get the GUID attached to the combo box item
                GUID* pCaptureGUID = (GUID*) SendMessage( hCaptureDeviceCombo, CB_GETITEMDATA, 
                                                          nCaptureIndex, 0 );

                // Remember that guid
                if( pCaptureGUID ) 
                    g_guidCaptureDevice = *pCaptureGUID;

                EndDialog( hDlg, IDOK );
                break;
            }

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
// Name: OnInitDevicesDialog()
// Desc: Initializes the devices dialog
//-----------------------------------------------------------------------------
HRESULT OnInitDevicesDialog( HWND hDlg )
{
    // Enumerate the capture devices and place them in the combo box
    HWND hCaptureDeviceCombo = GetDlgItem( hDlg, IDC_CAPTURE_DEVICE_COMBO );
    DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                 (VOID*)hCaptureDeviceCombo );

    // Select the first device in the combo box
    SendMessage( hCaptureDeviceCombo, CB_SETCURSEL, 0, 0 );

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
                        "Sample will now exit.", "DirectSound Sample", 
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
                    MessageBox( hDlg, "Error accepting DirectSound formats. "
                                "Sample will now exit.", "DirectSound Sample", 
                                MB_OK | MB_ICONERROR );
                    EndDialog( hDlg, IDABORT );
                }

                break;

            case IDC_FORMATS_INPUT_LISTBOX:
                OnInputFormatBoxSelected( hDlg );
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
    
    ZeroMemory( &wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;

    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize = sizeof(dscbd);

    // Try 16 different standard formats to see if they are supported
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
// Desc: Fills the format list box based on the availible formats
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
            // Turn the index into a WAVEFORMATEX then turn that into a
            // string and put the string in the listbox
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
// Desc: Converts a wave format to a text string
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
// Name: OnInputFormatBoxSelected()
// Desc: Enables the OK button when there is a selection
//-----------------------------------------------------------------------------
HRESULT OnInputFormatBoxSelected( HWND hDlg )
{
    HWND hInputList  = GetDlgItem( hDlg, IDC_FORMATS_INPUT_LISTBOX );
    HWND hOK         = GetDlgItem( hDlg, IDOK );

    if( SendMessage( hInputList,  LB_GETCURSEL, 0, 0 ) != -1 )        
        EnableWindow( hOK, TRUE );
    else
        EnableWindow( hOK, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnFormatsOK()
// Desc: Creates a capture buffer format based on what was selected
//-----------------------------------------------------------------------------
HRESULT OnFormatsOK( HWND hDlg )
{
    HRESULT       hr;
    DWORD         dwInputSelect;
    DWORD         dwInputWavIndex;
    HWND          hInputList;

    hInputList = GetDlgItem( hDlg, IDC_FORMATS_INPUT_LISTBOX );

    dwInputSelect   = (DWORD)SendMessage( hInputList, LB_GETCURSEL, 0, 0 );
    dwInputWavIndex = (DWORD)SendMessage( hInputList, LB_GETITEMDATA, dwInputSelect, 0 );

    ZeroMemory( &g_wfxInput, sizeof(g_wfxInput));
    g_wfxInput.wFormatTag = WAVE_FORMAT_PCM;

    GetWaveFormatFromIndex( dwInputWavIndex, &g_wfxInput );

    if( FAILED( hr = CreateCaptureBuffer( &g_wfxInput ) ) )
        return DXTRACE_ERR( TEXT("CreateCaptureBuffer"), hr );

    EndDialog( hDlg, IDOK );

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

            case IDC_SOUNDFILE:
                OnSaveSoundFile( hDlg );
                break;

            case IDC_RECORD:
                g_bRecording = !g_bRecording;
                if( FAILED( hr = StartOrStopRecord( g_bRecording ) ) )
                {
                    DXTRACE_ERR( TEXT("StartOrStopRecord"), hr );
                    MessageBox( hDlg, "Error with DirectSoundCapture buffer."                            
                                "Sample will now exit.", "DirectSound Sample", 
                                MB_OK | MB_ICONERROR );
                    PostQuitMessage( 0 );
                    EndDialog( hDlg, IDABORT );
                }

                if( !g_bRecording )
                    EnableWindow( GetDlgItem( hDlg, IDC_RECORD ), FALSE );

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
// Name: OnInitMainDialog()
// Desc: Initializes the main dialog
//-----------------------------------------------------------------------------
HRESULT OnInitMainDialog( HWND hDlg )
{
    // Load the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    EnableWindow( GetDlgItem( hDlg, IDC_RECORD ), FALSE);
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("No file loaded.") );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
                                  VOID* pContext )
{
    // Set aside static storage space for 20 audio drivers
    static GUID  AudioDriverGUIDs[20];
    static DWORD dwAudioDriverIndex = 0;

    GUID* pTemp  = NULL;

    if( pGUID )
    {
        if( dwAudioDriverIndex >= 20 )
            return TRUE;

        pTemp = &AudioDriverGUIDs[dwAudioDriverIndex++];
        memcpy( pTemp, pGUID, sizeof(GUID) );
    }

    HWND hSoundDeviceCombo = (HWND)pContext;

    // Add the string to the combo box
    SendMessage( hSoundDeviceCombo, CB_ADDSTRING, 
                 0, (LPARAM) (LPCTSTR) strDesc );

    // Get the index of the string in the combo box
    INT nIndex = (INT)SendMessage( hSoundDeviceCombo, CB_FINDSTRING, 
                                   0, (LPARAM) (LPCTSTR) strDesc );

    // Set the item data to a pointer to the static guid stored in AudioDriverGUIDs
    SendMessage( hSoundDeviceCombo, CB_SETITEMDATA, 
                 nIndex, (LPARAM) pTemp );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: OnSaveSoundFile()
// Desc: Called when the user requests to save to a sound file
//-----------------------------------------------------------------------------
VOID OnSaveSoundFile( HWND hDlg ) 
{
    HRESULT hr;

    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("Wave Files\0*.wav\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Save Sound File"),
                         OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | 
                         OFN_HIDEREADONLY    | OFN_NOREADONLYRETURN, 
                         0, 0, TEXT(".wav"), 0, NULL, NULL };

    // Get the default media path (something like C:\WINDOWS\MEDIA)
    if( '\0' == strPath[0] )
    {
        GetWindowsDirectory( strPath, MAX_PATH );
        if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ) )
            strcat( strPath, TEXT("\\") );
        strcat( strPath, TEXT("MEDIA") );
    }

    if( g_bRecording )
    {
        // Stop the capture and read any data that 
        // was not caught by a notification
        StartOrStopRecord( FALSE );
        g_bRecording = FALSE;
    }

    // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_RECORD ), FALSE );
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Saving file...") );

    // Display the SaveFileName dialog. Then, try to load the specified file
    if( TRUE != GetSaveFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Save aborted.") );
        return;
    }

    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

    SAFE_DELETE( g_pWaveFile );
    g_pWaveFile = new CWaveFile;

    // Get the format of the capture buffer in g_wfxCaptureWaveFormat
    WAVEFORMATEX wfxCaptureWaveFormat;
    ZeroMemory( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX) );
    g_pDSBCapture->GetFormat( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX), NULL );

    // Load the wave file
    if( FAILED( hr = g_pWaveFile->Open( strFileName, &wfxCaptureWaveFormat, WAVEFILE_WRITE ) ) )
    {
        DXTRACE_ERR( TEXT("Open"), hr );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Can not create wave file.") );
        return;
    }

    // Update the UI controls to show the sound as the file is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    EnableWindow( GetDlgItem( hDlg, IDC_RECORD ), TRUE );

    // Remember the path for next time
    strcpy( strPath, strFileName );
    char* strLastSlash = strrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}




//-----------------------------------------------------------------------------
// Name: CreateCaptureBuffer()
// Desc: Creates a capture buffer and sets the format 
//-----------------------------------------------------------------------------
HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput )
{
    HRESULT hr;
    DSCBUFFERDESC dscbd;

    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );

    // Set the notification size
    g_dwNotifySize = MAX( 1024, pwfxInput->nAvgBytesPerSec / 8 );
    g_dwNotifySize -= g_dwNotifySize % pwfxInput->nBlockAlign;   

    // Set the buffer sizes 
    g_dwCaptureBufferSize = g_dwNotifySize * NUM_REC_NOTIFICATIONS;

    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );

    // Create the capture buffer
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = g_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pwfxInput; // Set the format during creatation

    if( FAILED( hr = g_pDSCapture->CreateCaptureBuffer( &dscbd, 
                                                        &g_pDSBCapture, 
                                                        NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateCaptureBuffer"), hr );

    g_dwNextCaptureOffset = 0;

    if( FAILED( hr = InitNotifications() ) )
        return DXTRACE_ERR( TEXT("InitNotifications"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitNotifications()
// Desc: Inits the notifications on the capture buffer which are handled
//       in WinMain()
//-----------------------------------------------------------------------------
HRESULT InitNotifications()
{
    HRESULT hr; 

    if( NULL == g_pDSBCapture )
        return E_FAIL;

    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = g_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
                                                    (VOID**)&g_pDSNotify ) ) )
        return DXTRACE_ERR( TEXT("QueryInterface"), hr );

    // Setup the notification positions
    for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
    {
        g_aPosNotify[i].dwOffset = (g_dwNotifySize * i) + g_dwNotifySize - 1;
        g_aPosNotify[i].hEventNotify = g_hNotificationEvent;             
    }
    
    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    if( FAILED( hr = g_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS, 
                                                            g_aPosNotify ) ) )
        return DXTRACE_ERR( TEXT("SetNotificationPositions"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StartOrStopRecord()
// Desc: Starts or stops the capture buffer from recording
//-----------------------------------------------------------------------------
HRESULT StartOrStopRecord( BOOL bStartRecording )
{
    HRESULT hr;

    if( bStartRecording )
    {
        // Create a capture buffer, and tell the capture 
        // buffer to start recording   
        if( FAILED( hr = CreateCaptureBuffer( &g_wfxInput ) ) )
            return DXTRACE_ERR( TEXT("CreateCaptureBuffer"), hr );

        if( FAILED( hr = g_pDSBCapture->Start( DSCBSTART_LOOPING ) ) )
            return DXTRACE_ERR( TEXT("Start"), hr );
    }
    else
    {
        // Stop the capture and read any data that 
        // was not caught by a notification
        if( NULL == g_pDSBCapture )
            return S_OK;

        // Stop the buffer, and read any data that was not 
        // caught by a notification
        if( FAILED( hr = g_pDSBCapture->Stop() ) )
            return DXTRACE_ERR( TEXT("Stop"), hr );

        if( FAILED( hr = RecordCapturedData() ) )
            return DXTRACE_ERR( TEXT("RecordCapturedData"), hr );

        // Close the wav file
        SAFE_DELETE( g_pWaveFile );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RecordCapturedData()
// Desc: Copies data from the capture buffer to the output buffer 
//-----------------------------------------------------------------------------
HRESULT RecordCapturedData() 
{
    HRESULT hr;
    VOID*   pbCaptureData    = NULL;
    DWORD   dwCaptureLength;
    VOID*   pbCaptureData2   = NULL;
    DWORD   dwCaptureLength2;
    VOID*   pbPlayData       = NULL;
    UINT    dwDataWrote;
    DWORD   dwReadPos;
    DWORD   dwCapturePos;
    LONG lLockSize;

    if( NULL == g_pDSBCapture )
        return S_FALSE;
    if( NULL == g_pWaveFile )
        return S_FALSE;

    if( FAILED( hr = g_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
        return DXTRACE_ERR( TEXT("GetCurrentPosition"), hr );

    lLockSize = dwReadPos - g_dwNextCaptureOffset;
    if( lLockSize < 0 )
        lLockSize += g_dwCaptureBufferSize;

    // Block align lock size so that we are always write on a boundary
    lLockSize -= (lLockSize % g_dwNotifySize);

    if( lLockSize == 0 )
        return S_FALSE;

    // Lock the capture buffer down
    if( FAILED( hr = g_pDSBCapture->Lock( g_dwNextCaptureOffset, lLockSize, 
                                          &pbCaptureData, &dwCaptureLength, 
                                          &pbCaptureData2, &dwCaptureLength2, 0L ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );

    // Write the data into the wav file
    if( FAILED( hr = g_pWaveFile->Write( dwCaptureLength, 
                                              (BYTE*)pbCaptureData, 
                                              &dwDataWrote ) ) )
        return DXTRACE_ERR( TEXT("Write"), hr );

    // Move the capture offset along
    g_dwNextCaptureOffset += dwCaptureLength; 
    g_dwNextCaptureOffset %= g_dwCaptureBufferSize; // Circular buffer

    if( pbCaptureData2 != NULL )
    {
        // Write the data into the wav file
        if( FAILED( hr = g_pWaveFile->Write( dwCaptureLength2, 
                                                  (BYTE*)pbCaptureData2, 
                                                  &dwDataWrote ) ) )
            return DXTRACE_ERR( TEXT("Write"), hr );

        // Move the capture offset along
        g_dwNextCaptureOffset += dwCaptureLength2; 
        g_dwNextCaptureOffset %= g_dwCaptureBufferSize; // Circular buffer
    }

    // Unlock the capture buffer
    g_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
                           pbCaptureData2, dwCaptureLength2 );


    return S_OK;
}




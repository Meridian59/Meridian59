//-----------------------------------------------------------------------------
// File: MusicTool.cpp
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
#include <tchar.h>
#include "resource.h"
#include "DMUtil.h"
#include "DXUtil.h"
#include "EchoTool.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );
HRESULT ProcessDirectMusicMessages( HWND hDlg );
VOID    OnOpenSoundFile( HWND hDlg );
HRESULT LoadSegmentFile( HWND hDlg, TCHAR* strFileName );
HRESULT OnPlaySegment( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
HRESULT OnChangeTool( HWND hDlg );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
HINSTANCE               g_hInst                 = NULL;
CMusicManager*          g_pMusicManager         = NULL;
CMusicSegment*          g_pMusicSegment         = NULL;
CEchoTool*              g_pEchoTool             = NULL;
IDirectMusicTool*       g_pCurrentTool          = NULL;
IDirectMusicGraph*      g_pGraph                = NULL;
HANDLE                  g_hDMusicMessageEvent   = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    HWND    hDlg = NULL;
    BOOL    bDone = FALSE;
    int     nExitCode;
    HRESULT hr; 
    DWORD   dwResult;
    MSG     msg;

    g_hInst = hInst;

    // Display the main dialog box.
    hDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_MAIN), 
                         NULL, MainDlgProc );

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
        case WM_INITDIALOG:
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( hDlg, "Error initializing DirectMusic.  Sample will now exit.", 
                                  "DirectMusic Sample", MB_OK | MB_ICONERROR );
                PostQuitMessage( IDABORT );
                return FALSE;
            }

            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SOUNDFILE:
                    OnOpenSoundFile( hDlg );
                    break;

                case IDCANCEL:
                    PostQuitMessage( IDCANCEL );
                    break;

                case IDC_PLAY:
                    if( FAILED( hr = OnPlaySegment( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlaySegment"), hr );
                        MessageBox( hDlg, "Error playing DirectMusic segment. "
                                    "Sample will now exit.", "DirectMusic Sample", 
                                    MB_OK | MB_ICONERROR );
                        PostQuitMessage( IDABORT );
                    }
                    break;

                case IDC_STOP:
                    g_pMusicSegment->Stop( DMUS_SEGF_BEAT ); 
                    EnablePlayUI( hDlg, TRUE );
                    break;

                case IDC_TOOL_COMBO:
                    OnChangeTool( hDlg );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_DESTROY:
            // Cleanup everything
            SAFE_DELETE( g_pMusicSegment );
            SAFE_DELETE( g_pMusicManager );
            SAFE_DELETE( g_pEchoTool );
            CloseHandle( g_hDMusicMessageEvent );
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
HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr;
    LONG lIndex;

    // Set the icon for this dialog.
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    g_hDMusicMessageEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    g_pMusicManager = new CMusicManager();

    // Init DirectMusic with a default audio path
    hr = g_pMusicManager->Initialize( hDlg );
    IDirectMusicPerformance8* pPerformance    = g_pMusicManager->GetPerformance();
    IDirectMusicAudioPath8* pDefaultAudioPath = g_pMusicManager->GetDefaultAudioPath();

    // Create a DirectMusicGraph, and tell the preformance about it
    hr = pDefaultAudioPath->GetObjectInPath( 0, DMUS_PATH_PERFORMANCE_GRAPH, 0,
                                             GUID_NULL, 0, IID_IDirectMusicGraph, 
                                             (LPVOID*) &g_pGraph );
    if( FAILED( hr ) )
        return DXTRACE_ERR( TEXT("GetObjectInPath"), hr );

    // Register segment notification
    GUID guid = GUID_NOTIFICATION_SEGMENT;
    if( FAILED( hr = pPerformance->AddNotificationType( guid ) ) )
        return DXTRACE_ERR( TEXT("AddNotificationType"), hr );

    if( FAILED( hr = pPerformance->SetNotificationHandle( g_hDMusicMessageEvent, 0 ) ) )
        return DXTRACE_ERR( TEXT("SetNotificationHandle"), hr );

    g_pEchoTool = new CEchoTool();

    // Init the UI
    HWND hToolCombo = GetDlgItem( hDlg, IDC_TOOL_COMBO );
    lIndex = (LONG)SendMessage( hToolCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("None") );
    SendMessage( hToolCombo, CB_SETITEMDATA, lIndex, (LPARAM) NULL );
    lIndex = (LONG)SendMessage( hToolCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("Echo Tool") );
    SendMessage( hToolCombo, CB_SETITEMDATA, lIndex, (LPARAM) (IDirectMusicTool*) g_pEchoTool );
    SendMessage( hToolCombo, CB_SETCURSEL, 0, 0 );

    // Load a default music segment 
    TCHAR strFileName[MAX_PATH];
    strcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
    strcat( strFileName, "sample.sgt" );
    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        // Set the UI controls
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("No file loaded.") );
    }

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

    // Get the default media path (something like C:\MSSDK\SAMPLES\DMUSIC\MEDIA)
    if( '\0' == strPath[0] )
    {
        const TCHAR* szDir = DXUtil_GetDXSDKMediaPath();
        strcpy( strPath, szDir );
    }

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("DirectMusic Content Files\0*.sgt;*.mid;*.rmi\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Content File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".sgt"), 0, NULL, NULL };

    if( g_pMusicSegment )
        g_pMusicSegment->Stop( 0 );

    // Update the UI controls to show the sound as loading a file
    EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ), FALSE);
    EnableWindow(  GetDlgItem( hDlg, IDC_STOP ), FALSE);
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Loading file...") );

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Load aborted.") );
        return;
    }

    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        // Not a critical failure, so just update the status
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create segment from file.") );
    }

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
    // script has been released
    g_pMusicManager->CollectGarbage();

    // Set the media path based on the file name (something like C:\MEDIA)
    // to be used as the search directory for finding DirectMusic content
    // related to this file.
    TCHAR strMediaPath[MAX_PATH];
    _tcscpy( strMediaPath, strFileName );
    TCHAR* strLastSlash = _tcsrchr(strMediaPath, TEXT('\\'));
    *strLastSlash = 0;
    if( FAILED( hr = g_pMusicManager->SetSearchDirectory( strMediaPath ) ) )
        return DXTRACE_ERR( TEXT("SetSearchDirectory"), hr );

    // For DirectMusic must know if the file is a standard MIDI file or not
    // in order to load the correct instruments.
    BOOL bMidiFile = FALSE;
    if( strstr( strFileName, ".mid" ) != NULL ||
        strstr( strFileName, ".rmi" ) != NULL ) 
    {
        bMidiFile = TRUE;
    }

    // Load the file into a DirectMusic segment 
    if( FAILED( g_pMusicManager->CreateSegmentFromFile( &g_pMusicSegment, strFileName, 
                                                        TRUE, bMidiFile ) ) )
    {
        // Not a critical failure, so just update the status
        return S_FALSE; 
    }

    // Update the UI controls to show the segment is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    EnablePlayUI( hDlg, TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ProcessDirectMusicMessages()
// Desc: Handle DirectMusic notification messages
//-----------------------------------------------------------------------------
HRESULT ProcessDirectMusicMessages( HWND hDlg )
{
    HRESULT hr;
    IDirectMusicPerformance8* pPerf = NULL;
    DMUS_NOTIFICATION_PMSG* pPMsg;
        
    if( NULL == g_pMusicManager )
        return S_OK;

    pPerf = g_pMusicManager->GetPerformance();

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

                if( SUCCEEDED( hr = pSegmentState->GetSegment( &pNotifySegment ) ) )
                {
                    if( FAILED( hr = pNotifySegment->QueryInterface( IID_IDirectMusicSegment8,
                                                                     (VOID**) &pNotifySegment8 ) ) )
                        return DXTRACE_ERR( TEXT("QueryInterface"), hr );

                    // Get the IDirectMusicSegment for the primary segment
                    pPrimarySegment8 = g_pMusicSegment->GetSegment();

                    // Figure out which segment this is
                    if( pNotifySegment8 == pPrimarySegment8 )
                    {
                        // Update the UI controls to show the sound as stopped
                        EnablePlayUI( hDlg, TRUE );
                    }
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



//-----------------------------------------------------------------------------
// Name: OnPlaySegment()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnPlaySegment( HWND hDlg )
{
    HRESULT hr;

    HWND hLoopButton = GetDlgItem( hDlg, IDC_LOOP_CHECK );
    BOOL bLooped = ( SendMessage( hLoopButton, BM_GETSTATE, 0, 0 ) == BST_CHECKED );

    if( bLooped )
    {
        // Set the segment to repeat many times
        if( FAILED( hr = g_pMusicSegment->SetRepeats( DMUS_SEG_REPEAT_INFINITE ) ) )
            return DXTRACE_ERR( TEXT("SetRepeats"), hr );
    }
    else
    {
        // Set the segment to not repeat
        if( FAILED( hr = g_pMusicSegment->SetRepeats( 0 ) ) )
            return DXTRACE_ERR( TEXT("SetRepeats"), hr );
    }

    // Play the segment and wait. The DMUS_SEGF_BEAT indicates to play on the 
    // next beat if there is a segment currently playing. 
    if( FAILED( hr = g_pMusicSegment->Play( DMUS_SEGF_BEAT ) ) )
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
        EnableWindow(   GetDlgItem( hDlg, IDC_LOOP_CHECK ), TRUE );
        EnableWindow(   GetDlgItem( hDlg, IDC_STOP ),       FALSE );

        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
        SetFocus(       GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow(  GetDlgItem( hDlg, IDC_LOOP_CHECK ), FALSE );
        EnableWindow(  GetDlgItem( hDlg, IDC_STOP ),       TRUE );
        SetFocus(      GetDlgItem( hDlg, IDC_STOP ) );
        EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ),       FALSE );
    }
}




//-----------------------------------------------------------------------------
// Name: OnChangeTool()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnChangeTool( HWND hDlg )
{
    HRESULT hr;
    IDirectMusicTool* pSelectedTool;
    LONG lCurSelection;

    lCurSelection = (LONG)SendDlgItemMessage( hDlg, IDC_TOOL_COMBO, CB_GETCURSEL, 0, 0 );
    pSelectedTool = (IDirectMusicTool*) SendDlgItemMessage( hDlg, IDC_TOOL_COMBO, 
                                                            CB_GETITEMDATA, lCurSelection, 0 );
    if( pSelectedTool != g_pCurrentTool )
    {
        // Remove the current tool from the graph
        if( g_pCurrentTool != NULL )
        {
            if( FAILED( hr = g_pGraph->RemoveTool( g_pCurrentTool ) ) )
                return DXTRACE_ERR( TEXT("RemoveTool"), hr );
        }

        // Add the tool to the graph on all PChannels 
        // and at the beginning of the graph. 
        if( pSelectedTool != NULL )
        {
            if( FAILED( hr = g_pGraph->InsertTool( pSelectedTool, NULL, 0, 0 ) ) )
                return DXTRACE_ERR( TEXT("InsertTool"), hr );
        }

        g_pCurrentTool = pSelectedTool;
    }

    return S_OK;
}





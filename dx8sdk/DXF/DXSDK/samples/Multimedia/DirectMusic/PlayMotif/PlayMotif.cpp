//-----------------------------------------------------------------------------
// File: PlayMotif.cpp
//
// Desc: Lets the user play a primary segment using DirectMusic as well as 
//       any of motifs contained inside the segment.  
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <commctrl.h>
#include <objbase.h>
#include <conio.h>
#include <direct.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <dxerr8.h>
#include <tchar.h>
#include <commctrl.h>
#include "resource.h"
#include "DMUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );
HRESULT ProcessDirectMusicMessages( HWND hDlg );
VOID    OnOpenSegmentFile( HWND hDlg );
HRESULT LoadSegmentFile( HWND hDlg, TCHAR* strFileName );
HRESULT OnPlaySegment( HWND hDlg );
HRESULT OnPlayMotif( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );

struct MOTIF_NODE
{
    IDirectMusicSegment* pMotif;
    DWORD dwPlayCount;
};




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
CMusicManager*     g_pMusicManager          = NULL;
CMusicSegment*     g_pMusicSegment          = NULL;
HINSTANCE          g_hInst                  = NULL;
HANDLE             g_hDMusicMessageEvent    = NULL;




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
                EndDialog( hDlg, 0 );
                return TRUE;
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SOUNDFILE:
                    OnOpenSegmentFile( hDlg );
                    break;

                case IDCANCEL:
                    PostQuitMessage( IDCANCEL );
                    break;

                case IDC_MOTIF_LIST:
                    if (HIWORD(wParam) == LBN_DBLCLK )
                    {
                    if( FAILED( hr = OnPlayMotif( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlayMotif"), hr );
                        MessageBox( hDlg, "Error playing DirectMusic motif. "
                                    "Sample will now exit.", "DirectMusic Sample", 
                                    MB_OK | MB_ICONERROR );
                        PostQuitMessage( IDABORT );
                    }
                    break;
                    }

                    if( g_pMusicSegment )
                    {
                        if( g_pMusicSegment->IsPlaying() )
                        {
                            HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
                            int nIndex = (int)SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

                            if( nIndex == LB_ERR )
                                EnableWindow( GetDlgItem( hDlg, IDC_PLAY_MOTIF ), FALSE );
                            else
                                EnableWindow( GetDlgItem( hDlg, IDC_PLAY_MOTIF ), TRUE );
                        }
                    }
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
                    SetDlgItemText( hDlg, IDC_STATUS, "Primary segment stopped." );
                    EnablePlayUI( hDlg, TRUE );
                    break;

                case IDC_PLAY_MOTIF:
                    if( FAILED( hr = OnPlayMotif( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("OnPlayMotif"), hr );
                        MessageBox( hDlg, "Error playing DirectMusic motif. "
                                    "Sample will now exit.", "DirectMusic Sample", 
                                    MB_OK | MB_ICONERROR );
                        PostQuitMessage( IDABORT );
                    }
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_DESTROY:
        {            
            // Cleanup everything
            HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
            DWORD dwCount = (DWORD)SendMessage( hListBox, LB_GETCOUNT, 0, 0 );                 
            for( DWORD i=0; i<dwCount; i++ )
            {
                MOTIF_NODE* pMotifNode = (MOTIF_NODE*) SendMessage( hListBox, LB_GETITEMDATA, i, 0 );
                if( pMotifNode )
                {
                    SAFE_RELEASE( pMotifNode->pMotif );
                    SAFE_DELETE( pMotifNode );
                }
            }

            CloseHandle( g_hDMusicMessageEvent );
            SAFE_DELETE( g_pMusicSegment );
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

    g_hDMusicMessageEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    g_pMusicManager = new CMusicManager();

    if( FAILED( hr = g_pMusicManager->Initialize( hDlg ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    // Register segment notification
    IDirectMusicPerformance* pPerf = g_pMusicManager->GetPerformance();
    GUID guid = GUID_NOTIFICATION_SEGMENT;
    if( FAILED( hr = pPerf->AddNotificationType( guid ) ) )
        return DXTRACE_ERR( TEXT("AddNotificationType"), hr );

    if( FAILED( hr = pPerf->SetNotificationHandle( g_hDMusicMessageEvent, 0 ) ) )
        return DXTRACE_ERR( TEXT("SetNotificationHandle"), hr );

    EnableWindow( GetDlgItem( hDlg, IDC_PLAY_MOTIF ), FALSE );
    CheckRadioButton( hDlg, IDC_RADIO_DEFAULT, IDC_RADIO_MEASURE, IDC_RADIO_MEASURE );

    // Load a default music segment 
    TCHAR strFileName[MAX_PATH];
    strcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
    strcat( strFileName, "sample.sgt" );
    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        // Set the UI controls
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("No file loaded.") );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnOpenSegmentFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenSegmentFile( HWND hDlg ) 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");
    HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );

    // Get the default media path (something like C:\MSSDK\SAMPLES\MULTIMEDIA\MEDIA)
    if( '\0' == strPath[0] )
    {
        const TCHAR* szDir = DXUtil_GetDXSDKMediaPath();
        strcpy( strPath, szDir );
    }

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("DirectMusic Content Files\0*.sgt;*.mid;*.rmi\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Segment File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".sgt"), 0, NULL, NULL };

    if( g_pMusicSegment )
        g_pMusicSegment->Stop( 0 );

    // Update the UI controls to show the sound as loading a file
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY ), FALSE);
    EnableWindow( GetDlgItem( hDlg, IDC_STOP ), FALSE);
    EnableWindow( GetDlgItem( hDlg, IDC_PLAY_MOTIF ), FALSE );
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Loading file...") );

    // Cleanup motif listbox
    DWORD dwCount = (DWORD)SendMessage( hListBox, LB_GETCOUNT, 0, 0 );                 
    for( DWORD i=0; i<dwCount; i++ )
    {
        MOTIF_NODE* pMotifNode = (MOTIF_NODE*) SendMessage( hListBox, LB_GETITEMDATA, i, 0 );
        if( pMotifNode )
        {
            SAFE_RELEASE( pMotifNode->pMotif );
            SAFE_DELETE( pMotifNode );
        }
    }
    SendMessage( hListBox, LB_RESETCONTENT, 0, 0 );

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Load aborted.") );
        return;
    }

    if( S_FALSE == LoadSegmentFile( hDlg, strFileName ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Could not create segment from file.") );
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

    HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("") );

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

    IDirectMusicStyle8* pStyle = NULL;
    DWORD dwStyleIndex = 0;

    while( TRUE )
    {
        // Get the style from the segment
        // Segments may have any number of styles.
        hr = g_pMusicSegment->GetStyle( &pStyle, dwStyleIndex );
        if( FAILED(hr) )
            break;

        // Get the names of the motifs from the style. 
        // Styles may have any number of motifs.
        DWORD dwIndex = 0;
        while( TRUE )
        {
            WCHAR wstrMotifName[MAX_PATH];
            CHAR  strMotifName[MAX_PATH];
            if( S_FALSE == pStyle->EnumMotif( dwIndex, wstrMotifName ) )
                break;

            wcstombs( strMotifName, wstrMotifName, wcslen( wstrMotifName ) + 1 );
            int nIndex = (int)SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM) strMotifName );

            MOTIF_NODE* pMotifNode = new MOTIF_NODE;
            if( FAILED( hr = pStyle->GetMotif( wstrMotifName, &pMotifNode->pMotif ) ) )
                return DXTRACE_ERR( TEXT("GetMotif"), hr );
            pMotifNode->dwPlayCount = 0;
            SendMessage( hListBox, LB_SETITEMDATA, nIndex, (LPARAM) pMotifNode );

            dwIndex++;
        }

        SAFE_RELEASE( pStyle );
        dwStyleIndex++;
    }

    SendMessage( hListBox, LB_SETCURSEL, 0, 0 );

    // Update the UI controls to show the segment is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("File loaded.") );
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
    IDirectMusicPerformance* pPerf = NULL;
    DMUS_NOTIFICATION_PMSG* pPMsg;
        
    if( NULL == g_pMusicManager )
        return S_OK;

    pPerf = g_pMusicManager->GetPerformance();

    // Get waiting notification message from the performance
    while( S_OK == pPerf->GetNotificationPMsg( &pPMsg ) )
    {
        switch( pPMsg->dwNotificationOption )
        {
        case DMUS_NOTIFICATION_SEGSTART:
            if( pPMsg->punkUser )
            {
                IDirectMusicSegment*       pPrimarySegment  = NULL;
                IDirectMusicSegmentState8* pSegmentState    = NULL;
                IDirectMusicSegment*       pNotifySegment   = NULL;
                IDirectMusicSegment8*      pPrimarySegment8 = NULL;

                // The pPMsg->punkUser contains a IDirectMusicSegmentState8, 
                // which we can query for the segment that segment it refers to.
                if( FAILED( hr = pPMsg->punkUser->QueryInterface( IID_IDirectMusicSegmentState8,
                                                                  (VOID**) &pSegmentState ) ) )
                    return DXTRACE_ERR( TEXT("QueryInterface"), hr );

                if( FAILED( hr = pSegmentState->GetSegment( &pNotifySegment ) ) )
                    return DXTRACE_ERR( TEXT("GetSegment"), hr );

                // Get the IDirectMusicSegment for the primary segment
                pPrimarySegment8 = g_pMusicSegment->GetSegment();
                if( FAILED( hr = pPrimarySegment8->QueryInterface( IID_IDirectMusicSegment,
                                                                   (VOID**) &pPrimarySegment ) ) )
                    return DXTRACE_ERR( TEXT("QueryInterface"), hr );
                // Figure out which segment this is
                if( pNotifySegment == pPrimarySegment )
                {
                    SetDlgItemText( hDlg, IDC_STATUS, "Primary segment playing." );
                }
                else
                {
                    // Look through the motfs and see if they are playing 
                    HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
                    DWORD dwCount = (DWORD)SendMessage( hListBox, LB_GETCOUNT, 0, 0 );                 
                    for( DWORD i=0; i<dwCount; i++ )
                    {
                        MOTIF_NODE* pMotifNode = (MOTIF_NODE*) SendMessage( hListBox, LB_GETITEMDATA, i, 0 );
                        if( pNotifySegment == pMotifNode->pMotif )
                        {
                            // If its motif segment update the UI
                            TCHAR strMotifName[MAX_PATH];
                            SendMessage( hListBox, LB_GETTEXT, i, (LPARAM) strMotifName );

                            TCHAR strStatus[MAX_PATH];
                            if( pMotifNode->dwPlayCount > 0 )
                                strMotifName[ strlen(strMotifName) - strlen(" (Playing)") ] = 0;

                            wsprintf( strStatus, "%s motif started playing.", strMotifName );
                            SetDlgItemText( hDlg, IDC_STATUS, strStatus );

                            pMotifNode->dwPlayCount++;
                            if( pMotifNode->dwPlayCount == 1 )
                            {
                                int nCurSel = (int)SendMessage( hListBox, LB_GETCURSEL, 0, 0 );
                                SendMessage( hListBox, LB_DELETESTRING, i, 0 );
                                strcat( strMotifName, " (Playing)" );
                                SendMessage( hListBox, LB_INSERTSTRING, i, (LPARAM) strMotifName );
                                SendMessage( hListBox, LB_SETITEMDATA,  i, (LPARAM) pMotifNode );
                                SendMessage( hListBox, LB_SETCURSEL, nCurSel, 0 );
                            }
                        }
                    }
                }

                // Cleanup
                SAFE_RELEASE( pSegmentState );
                SAFE_RELEASE( pNotifySegment );
                SAFE_RELEASE( pPrimarySegment );
            }
            break;

        case DMUS_NOTIFICATION_SEGEND:
            if( pPMsg->punkUser )
            {
                IDirectMusicSegment*       pPrimarySegment  = NULL;
                IDirectMusicSegmentState8* pSegmentState    = NULL;
                IDirectMusicSegment*       pNotifySegment   = NULL;
                IDirectMusicSegment8*      pPrimarySegment8 = NULL;

                // The pPMsg->punkUser contains a IDirectMusicSegmentState8, 
                // which we can query for the segment that segment it refers to.
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

                // Get the IDirectMusicSegment for the primary segment
                pPrimarySegment8 = g_pMusicSegment->GetSegment();
                if( FAILED( hr = pPrimarySegment8->QueryInterface( IID_IDirectMusicSegment,
                                                                   (VOID**) &pPrimarySegment ) ) )
                    return DXTRACE_ERR( TEXT("QueryInterface"), hr );

                // Figure out which segment this is
                if( pNotifySegment == pPrimarySegment )
                {
                    // Update the UI controls to show the sound as stopped
                    SetDlgItemText( hDlg, IDC_STATUS, "Primary segment stopped." );
                    EnablePlayUI( hDlg, TRUE );
                }
                else
                {
                    HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
                    DWORD dwCount = (DWORD)SendMessage( hListBox, LB_GETCOUNT, 0, 0 );                 
                    for( DWORD i=0; i<dwCount; i++ )
                    {
                        MOTIF_NODE* pMotifNode = (MOTIF_NODE*) SendMessage( hListBox, LB_GETITEMDATA, i, 0 );
                        if( pNotifySegment == pMotifNode->pMotif )
                        {
                            // If its motif segment update the UI
                            TCHAR strMotifName[MAX_PATH];
                            SendMessage( hListBox, LB_GETTEXT, i, (LPARAM) strMotifName );
                            strMotifName[ strlen(strMotifName) - strlen(" (Playing)") ] = 0;

                            pMotifNode->dwPlayCount--;
                            if( pMotifNode->dwPlayCount == 0 )
                            {
                                int nCurSel = (int)SendMessage( hListBox, LB_GETCURSEL, 0, 0 );
                                SendMessage( hListBox, LB_DELETESTRING, i, 0 );
                                SendMessage( hListBox, LB_INSERTSTRING, i, (LPARAM) strMotifName );
                                SendMessage( hListBox, LB_SETITEMDATA,  i, (LPARAM) pMotifNode );
                                SendMessage( hListBox, LB_SETCURSEL, nCurSel, 0 );
                            }

                            TCHAR strStatus[MAX_PATH];
                            wsprintf( strStatus, "%s motif stopped playing.", strMotifName );
                            SetDlgItemText( hDlg, IDC_STATUS, strStatus );
                        }
                    }
                }

                // Cleanup
                SAFE_RELEASE( pSegmentState );
                SAFE_RELEASE( pNotifySegment );
                SAFE_RELEASE( pPrimarySegment );
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
    // next beat if there is a segment currently playing. The first 0 indicates 
    // to play (on the next beat from) now.  
    if( FAILED( hr = g_pMusicSegment->Play( DMUS_SEGF_BEAT ) ) )
        return DXTRACE_ERR( TEXT("Play"), hr );

    EnablePlayUI( hDlg, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnPlayMotif()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnPlayMotif( HWND hDlg )
{
    HRESULT hr;
    DWORD dwSegFlags = DMUS_SEGF_SECONDARY;

    HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
    int nIndex = (int)SendMessage( hListBox, LB_GETCURSEL, 0, 0 );
    if( nIndex == LB_ERR )
        return S_FALSE;

    if( IsDlgButtonChecked( hDlg, IDC_RADIO_DEFAULT ) == BST_CHECKED )
        dwSegFlags |= DMUS_SEGF_DEFAULT;
    else if( IsDlgButtonChecked( hDlg, IDC_RADIO_IMMEDIATE ) == BST_CHECKED )
        dwSegFlags |= 0;
    else if( IsDlgButtonChecked( hDlg, IDC_RADIO_GRID ) == BST_CHECKED )
        dwSegFlags |= DMUS_SEGF_GRID;
    else if( IsDlgButtonChecked( hDlg, IDC_RADIO_BEAT ) == BST_CHECKED )
        dwSegFlags |= DMUS_SEGF_BEAT;
    else if( IsDlgButtonChecked( hDlg, IDC_RADIO_MEASURE ) == BST_CHECKED )
        dwSegFlags |= DMUS_SEGF_MEASURE;

    TCHAR strMotifName[ MAX_PATH ];
    SendMessage( hListBox, LB_GETTEXT, nIndex, (LPARAM) strMotifName );

    WCHAR wstrMotifName[ MAX_PATH ];
    mbstowcs( wstrMotifName, strMotifName, MAX_PATH );

    IDirectMusicSegment* pMotif = NULL;
    MOTIF_NODE* pMotifNode = (MOTIF_NODE*) SendMessage( hListBox, LB_GETITEMDATA, nIndex, 0 );
    IDirectMusicPerformance* pPerformance = g_pMusicManager->GetPerformance();

    if( FAILED( hr = pPerformance->PlaySegment( pMotifNode->pMotif, dwSegFlags, 
                                                0, NULL ) ) )
        return DXTRACE_ERR( TEXT("PlaySegment"), hr );

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

        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY_MOTIF ), FALSE );
        EnableWindow(   GetDlgItem( hDlg, IDC_PLAY ),       TRUE );
        SetFocus(       GetDlgItem( hDlg, IDC_PLAY ) );
    }
    else
    {
        EnableWindow(  GetDlgItem( hDlg, IDC_LOOP_CHECK ), FALSE );
        EnableWindow(  GetDlgItem( hDlg, IDC_STOP ),       TRUE );
        SetFocus(      GetDlgItem( hDlg, IDC_STOP ) );
        EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ),       FALSE );

        HWND hListBox = GetDlgItem( hDlg, IDC_MOTIF_LIST );
        int nIndex = (int)SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

        if( nIndex == LB_ERR )
            EnableWindow(   GetDlgItem( hDlg, IDC_PLAY_MOTIF ), FALSE );
        else
            EnableWindow(   GetDlgItem( hDlg, IDC_PLAY_MOTIF ), TRUE );
    }
}




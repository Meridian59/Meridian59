//----------------------------------------------------------------------------
// File: consolegraphics.cpp
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS
#include <windows.h>
#include <d3dx.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>
#include <process.h>
#include <dxerr8.h>
#include <tchar.h>
#include <dplay8.h>
#include "SyncObjects.h"
#include "DummyConnector.h"
#include "MazeApp.h"
#include "IMazeGraphics.h"
#include "ConsoleGraphics.h"

static CConsoleGraphics* s_pGraphics = NULL;



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CConsoleGraphics::CConsoleGraphics()
{
    s_pGraphics         = this;

    m_bQuitThread       = FALSE;
    m_pMazeApp          = NULL;
    m_dwNextOutput      = 0;
    m_hStdOut           = NULL;
    m_dwNumCmdLines     = 11;
    m_dwSeperatorLine   = 0;
    m_dwNumLogLines     = 0;
    m_dwWindowSizeY     = 0;
    m_bLocalLoopback    = TRUE;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CConsoleGraphics::~CConsoleGraphics()
{
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
VOID CConsoleGraphics::Init( CMazeApp* pMazeApp, CDPlay8Client* pDP8Client, 
                             CMazeClient* pMazeClient )
{
    m_pMazeApp      = pMazeApp;
    m_pMazeClient   = pMazeClient;
    m_pDP8Client    = pDP8Client;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CConsoleGraphics::Create( HINSTANCE hInstance )
{
    // Set a console control handler so we can clean 
    // up gracefully if we're forcibly shut down
    SetConsoleCtrlHandler( CtrlHandler, TRUE );
    m_hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );

    m_pConfig = m_pMazeApp->GetConfig();

    if( FALSE == ParseCommandLine() )
        return E_FAIL; // stop the app now

    if( m_pConfig->bAutoConnnect )
    {
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Type 'AUTOCONNECT OFF' to stop automatically connecting upon startup.") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Type 'SETUP' to change current connections settings.") );
    }
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Type 'HELP' for a list of commands.") );
    
    // Set up the console
    SetConsoleMode( GetStdHandle(STD_INPUT_HANDLE), 
                    ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_INPUT );

    m_dwLogBufferNext = 0;
    m_dwCmdBufferNext = 0;
    m_dwLastPrompt    = 0;
    m_dwCaretPos      = 9;
    m_dwNumProcessed  = 0;

    ZeroMemory( m_szCmdBuffer, sizeof(TCHAR)*MAX_CMD_LINES*256 );
    ZeroMemory( m_szLogBuffer, sizeof(TCHAR)*MAX_LOG_LINES*256 );

    GetConsoleScreenBufferInfo( m_hStdOut, &m_SavedConsoleInfo );

    m_dwWindowSizeY   = 30;
    m_dwSeperatorLine = m_dwWindowSizeY - m_dwNumCmdLines - 2;
    m_dwNumLogLines   = m_dwWindowSizeY - m_dwNumCmdLines - 3;
        
    SetupConsole( m_dwWindowSizeY );
    ClearScreen();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
INT CConsoleGraphics::Run()
{
    BOOL bHaveConnectSettings = m_pConfig->bAutoConnnect;

    // Allow immediate connect only if we already have the
    // connection settings
    m_pMazeApp->SetAllowConnect( bHaveConnectSettings );

    // Spin up a thread to record/display the output
    UINT dwPromptThreadID;
    m_hPromptThread = (HANDLE)_beginthreadex( NULL, 0, StaticPromptThread, 
                                              NULL, 0, &dwPromptThreadID );

    FLOAT fElapsedTime;
    while( !m_bQuitThread )
    {
        fElapsedTime = DXUtil_Timer( TIMER_GETELAPSEDTIME );
        m_pMazeApp->FrameMove( fElapsedTime );

        // Sleep for a little bit to avoid maxing out CPU
        Sleep( 30 );
    };

    // Wait for threads to shutdown
    WaitForSingleObject( m_hPromptThread, INFINITE );
    CloseHandle( m_hPromptThread );

    return 0;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CConsoleGraphics::Shutdown()
{
    RestoreOldConsoleSettings();
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
WCHAR** CConsoleGraphics::ConvertCommandLineToArgvW( WCHAR* strCommandLine, int* argc )
{
    WCHAR** argv = new WCHAR*[100];
    WCHAR* wstrTemp;
    WCHAR* wstrLast;
    int i = 0;
    
    wstrLast = strCommandLine;
    while( i < 100 )
    {
        argv[i] = new WCHAR[MAX_PATH];
        wcscpy( argv[i], wstrLast );
        wstrTemp = wcschr( argv[i], L' ' );
        if( wstrTemp )
        {
            wstrLast = wstrTemp + 1;
            *wstrTemp = 0;
            i++;
        }
        else
        {
            break;
        }
    }

    *argc = i + 1;
    return argv;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CConsoleGraphics::ParseCommandLine()
{
    int argc = 0;
    int i;
    WCHAR** argv;

    argv = ConvertCommandLineToArgvW( GetCommandLineW(), &argc );

    BOOL bParamFound = FALSE;
    BOOL bContinue   = TRUE;

    for ( i = 1 ; i < argc ; i++ )
    {
        const WCHAR* arg = argv[i];

        if( arg[0] == L'/' || arg[0] == L'-' )
        {
            if( _wcsicmp( arg+1, L"m" ) == 0 )
            {
                m_pConfig->bConnectToMicrosoftSite = TRUE;
                m_pConfig->bConnectToLocalServer   = FALSE;
                m_pConfig->bConnectToRemoteServer  = FALSE;
                m_pConfig->dwStressConnect = FALSE;

                bParamFound = TRUE;
            }
            else if( _wcsicmp( arg+1, L"l" ) == 0 )
            {
                m_pConfig->bConnectToMicrosoftSite = FALSE;
                m_pConfig->bConnectToLocalServer   = TRUE;
                m_pConfig->bConnectToRemoteServer  = FALSE;
                m_pConfig->dwStressConnect = FALSE;

                bParamFound = TRUE;
            }
            else if( arg[1] == L'r' || arg[1] == L'R' )
            {
                m_pConfig->bConnectToMicrosoftSite = FALSE;
                m_pConfig->bConnectToLocalServer   = FALSE;
                m_pConfig->bConnectToRemoteServer  = TRUE;
                m_pConfig->dwStressConnect = FALSE;
                    
                bParamFound = TRUE;

                if( arg[2] == L':' )
                {
                    DXUtil_ConvertWideStringToGeneric( m_pConfig->szIPAddress, arg+3 );
                }
            }   
            else if( _wcsnicmp( arg+1, L"SPThreads:", 8 ) == 0 )
            {
                // Points to the number of SP threads the user wants to use.
                m_pConfig->dwSPThreads = _wtoi( arg+11 ); 
                bParamFound = TRUE;

                if(m_pConfig->dwSPThreads > MAX_SP_THREADS || m_pConfig->dwSPThreads < MIN_SP_THREADS)
                {
                    printf( "Invalid SPThreads Value \n" );
                    printf( "  SPThreads Value must be between %i and %i.\n",MIN_SP_THREADS, MAX_SP_THREADS);
                    printf( "  Format: MazeConsoleClient.exe [/SPThreads:n] : Sets number of SP Threads to n \n\n" );

                    bContinue = FALSE;
                }
            }
            else if( _wcsnicmp( arg+1, L"SPBuffer:", 8 ) == 0 )
            {
                // Points to the size of the SP Buffer user wants to use.
                DWORD spBufferSize = _wtoi( arg+10 );
                bParamFound = TRUE;

                if( spBufferSize > MAX_SP_BUFFER )
                {
                    printf( "Invalid SPBuffer Value \n" );
                    printf( "  SPBuffer Value must be between 0 and %i.\n", MAX_SP_BUFFER);
                    printf( "  Format: MazeConsoleClient.exe [/SPBuffer:n] : Sets SP Buffer Size to n \n\n" );

                    bContinue = FALSE;
                }

                // Wait to assign since our buffer size is initialized to f's
                m_pConfig->dwSPBufferSize = spBufferSize;
                
            }
            else if( _wcsnicmp( arg+1, L"StressConnect:", 7 ) == 0 )
            {
                m_pConfig->bConnectToMicrosoftSite = FALSE;
                m_pConfig->bConnectToLocalServer   = FALSE;
                m_pConfig->bConnectToRemoteServer  = FALSE;

                // Points to the first connection attempt.
                m_pConfig->dwStressConnect = 1;
                bParamFound = TRUE;

                if( arg[14] == L':' )
                {
                    DXUtil_ConvertWideStringToGeneric( m_pConfig->szIPAddress, arg+15 );
                }
            }   
            else if( _wcsicmp( arg+1, L"d" ) == 0 || 
                _wcsicmp( arg+1, L"d+" ) == 0 )
            {
                m_pConfig->bAutoDisconnnect = TRUE;
                bParamFound = TRUE;
            }   
            else if( _wcsicmp( arg+1, L"d-" ) == 0 )
            {
                m_pConfig->bAutoDisconnnect = FALSE;
                bParamFound = TRUE;
            }   
            if( _wcsicmp( arg+1, L"f" ) == 0 || 
                _wcsicmp( arg+1, L"f+" ) == 0 )
            {
                m_pConfig->bFileLogging = TRUE;
                bParamFound = TRUE;
            }   
            else if( _wcsicmp( arg+1, L"f-" ) == 0 )
            {
                m_pConfig->bFileLogging = FALSE;
                bParamFound = TRUE;
            }
            else if( _wcsnicmp( arg+1, L"log:", 2 ) == 0 )
            {
                m_pConfig->dwLogLevel = _wtoi( arg+5 );
                bParamFound = TRUE;
            }
            else if( _wcsicmp( arg+1, L"?" ) == 0 )
            {
                printf( "MazeConsoleClient\n" );
                printf( "  Format: MazeConsoleClient.exe [/M] [/L] [/R[:ipaddress]] [/D] [/F] [/LOG]\n" );
                printf( "\n" );
                printf( "  Options:\n" );
                printf( "           /M  : connects to Microsoft server\n" );
                printf( "           /L  : connects to local server\n" );
                printf( "           /R[:ipaddress] : connects to remote server at [ipaddress]\n" );
                printf( "           /D+ : turns on auto-disconnecting\n" );
                printf( "           /D- : turns off auto-disconnecting\n" );
                printf( "           /F+ : turns on file logging\n" );
                printf( "           /F- : turns off file logging\n" );
                printf( "           /Log:n : sets the log level to n\n" );
                printf( "           /SPThreads:n : SP threads to n. Range %d - %d\n", MIN_SP_THREADS, MAX_SP_THREADS );
                printf( "           /SPBuffer:n : SP buffer size to n. Range 0 - %d\n", MAX_SP_BUFFER );
                printf( "           /StressConnect[:ipaddress] : tries [ipaddress], then local server.\n" );
                printf( "\n" );
                printf( "  Examples: \n" );
                printf( "       MazeConsoleClient /F /LOG:1\n" );
                printf( "       MazeConsoleClient /M /D+\n" );
                printf( "       MazeConsoleClient /R:myserver.myip.com /F-\n" );

                bContinue = FALSE;
            }   
        }
    }

    if( bParamFound )
    {
        m_pConfig->bAutoConnnect = TRUE;
        m_pMazeApp->SetSaveSettings( FALSE );
    }
    
    for ( i = 1 ; i < argc ; i++ )
    {
        SAFE_DELETE_ARRAY( argv[i] );    
    }
    SAFE_DELETE_ARRAY( argv );
    
    return bContinue;
}






//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL WINAPI CConsoleGraphics::CtrlHandler( DWORD type )
{
    switch ( type )
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            // Signal thread to quit
            FreeConsole();
            s_pGraphics->m_bQuitThread = TRUE;
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CConsoleGraphics::HandleOutputMsg( EnumLineType enumLineType, TCHAR* strLine )
{
    DWORD i;
    DWORD dwCoordY;

    switch( enumLineType )
    {
        case LINE_LOG:
        {
            _tcscpy( m_szLogBuffer[m_dwLogBufferNext], strLine );

            m_dwLogBufferNext++;
            m_dwLogBufferNext %= m_dwNumLogLines;
            dwCoordY = 0;

            for( i=m_dwLogBufferNext; i<m_dwNumLogLines; i++ )
            {
                dwCoordY++;
                WriteLine( dwCoordY, m_szLogBuffer[i] );
            }

            for( i=0; i<m_dwLogBufferNext; i++ )
            {
                dwCoordY++;
                WriteLine( dwCoordY, m_szLogBuffer[i] );
            }
            break;
        }

        case LINE_PROMPT:
        case LINE_CMD:
        {
            // Add m_szOutputBuffer[m_dwNextOutput] to szCmdBuffer array,
            // and redisplay the array on the top half of the screen
            _tcscpy( m_szCmdBuffer[m_dwCmdBufferNext], strLine );

#ifdef _DEBUG
            if( enumLineType != LINE_PROMPT )
            {
                OutputDebugString( m_szCmdBuffer[m_dwCmdBufferNext] );
                OutputDebugString( TEXT("\n") );
            }
#endif

            if( enumLineType == LINE_PROMPT )
            {
                m_dwLastPrompt = m_dwCmdBufferNext;
                m_dwCaretPos = _tcslen( m_szCmdBuffer[m_dwCmdBufferNext] );
            }

            m_dwCmdBufferNext++;
            m_dwCmdBufferNext %= m_dwNumCmdLines;
            dwCoordY = m_dwSeperatorLine;

            for( i=m_dwCmdBufferNext; i<m_dwNumCmdLines; i++ )
            {
                dwCoordY++;
                WriteLine( dwCoordY, m_szCmdBuffer[i] );
            }

            for( i=0; i<m_dwCmdBufferNext; i++ )
            {
                dwCoordY++;
                WriteLine( dwCoordY, m_szCmdBuffer[i] );
            }
            break;
        }

        case LINE_INPUT:
        {
            // Update the last prompt line in the szCmdBuffer array with this
            // string of input, so what was typed in is displayed as it scrolls
            _tcscpy( &m_szCmdBuffer[m_dwLastPrompt][m_dwCaretPos], strLine );
            break;
        }
    }

    if( enumLineType == LINE_PROMPT )
    {                 
        // Reset the cursor position if this is a cmd prompt line
        COORD coord = { (WORD)m_dwCaretPos, (WORD)m_dwWindowSizeY-2 };
        SetConsoleCursorPosition( m_hStdOut, coord );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CConsoleGraphics::SetupConsole( DWORD dwWindowSizeY )
{
    static TCHAR strEmpty[255] = TEXT("                                                                                                                                                                                                                                                              ");
    DWORD dwWritten;
    COORD coord = { 0, 0 };
    SMALL_RECT rcWindow = { 0, 0, 79, (WORD)dwWindowSizeY-1 };

    SetConsoleWindowInfo( m_hStdOut, TRUE, &rcWindow );

    COORD crdBufferSize;
    crdBufferSize.X = 80;
    crdBufferSize.Y = (WORD)dwWindowSizeY;
    SetConsoleScreenBufferSize( m_hStdOut, crdBufferSize );

    // Write a blank string first
    for( int i=rcWindow.Top; i<rcWindow.Bottom; i++ )
    {
        coord.Y = (WORD)i;
        SetConsoleCursorPosition( m_hStdOut, coord );
        WriteConsole( m_hStdOut, strEmpty, rcWindow.Right + 1, &dwWritten, NULL );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CConsoleGraphics::RestoreOldConsoleSettings()
{
    static TCHAR strEmpty[255] = TEXT("                                                                                                                                                                                                                                                              ");
    DWORD dwWritten;
    COORD coord = { 0, 0 };

    SetConsoleScreenBufferSize( m_hStdOut, m_SavedConsoleInfo.dwSize );
    SetConsoleWindowInfo( m_hStdOut, TRUE, &m_SavedConsoleInfo.srWindow );

    // Write a blank string first
    for( int i=m_SavedConsoleInfo.srWindow.Top; 
         i<m_SavedConsoleInfo.srWindow.Bottom;
         i++ )
    {
        coord.Y = (WORD)i;
        SetConsoleCursorPosition( m_hStdOut, coord );
        WriteConsole( m_hStdOut, strEmpty, m_SavedConsoleInfo.srWindow.Right + 1, &dwWritten, NULL );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CConsoleGraphics::WriteLine( DWORD nCoordY, TCHAR* strBuffer )
{
    // Write blanks to make all strings 80 TCHARs long so that
    // the old text is erased as this one is displayed
    for( DWORD dwIndex = _tcslen(strBuffer); dwIndex<80; dwIndex++ )
        strBuffer[dwIndex] = ' ';
    strBuffer[dwIndex] = 0;

    // Write strBuffer at (0,nCoordY)
    DWORD dwWritten;
    COORD coord = { 0, (WORD) nCoordY };
    WriteConsoleOutputCharacter( m_hStdOut, strBuffer, 80, coord, &dwWritten ); 
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CConsoleGraphics::PrintHelp()
{
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Commands:") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("    SETUP, EXIT, LOGLEVEL, CONNECTIONINFO") ); 
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("    FILELOG, CONNECT, DISCONNECT") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("    AUTOCONNECT, AUTODISCONNECT") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("    SETSTATRATE, SETRETRYDELAY") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("    SPTHREADS, SPBUFFER") );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CConsoleGraphics::ClearScreen()
{
    static TCHAR strEmpty[255] = TEXT("                                                                                                                                                                                                                                                              ");
    DWORD dwWritten;
    DWORD i;
    COORD coord = { 0, 0 };
    DWORD dwCoordY;

    // Write a blank string first
    for( i=0; i<m_dwWindowSizeY; i++ )
    {
        coord.Y = (WORD)i;
        SetConsoleCursorPosition( m_hStdOut, coord );
        WriteConsole( m_hStdOut, strEmpty, m_SavedConsoleInfo.srWindow.Right + 1, &dwWritten, NULL );
    }

    // Display a seperator between the two areas of the console window
    TCHAR strBuffer[200];
    _tcscpy( strBuffer, TEXT("-------------------------------------------------------------------------------") );
    WriteLine( m_dwSeperatorLine, strBuffer );

    dwCoordY = 0; 
    for( i=m_dwLogBufferNext; i<m_dwNumLogLines; i++ )
    {
        dwCoordY++;
        WriteLine( dwCoordY, m_szLogBuffer[i] );
    }

    for( i=0; i<m_dwLogBufferNext; i++ )
    {
        dwCoordY++;
        WriteLine( dwCoordY, m_szLogBuffer[i] );
    }

    dwCoordY = m_dwSeperatorLine;

    for( i=m_dwCmdBufferNext; i<m_dwNumCmdLines; i++ )
    {
        dwCoordY++;
        WriteLine( dwCoordY, m_szCmdBuffer[i] );
    }

    for( i=0; i<m_dwCmdBufferNext; i++ )
    {
        dwCoordY++;
        WriteLine( dwCoordY, m_szCmdBuffer[i] );
    }

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CConsoleGraphics::DoPrompt( TCHAR* strPromptString, TCHAR* strBuffer )
{
    m_pMazeApp->ConsolePrintf( LINE_PROMPT, strPromptString );
    DWORD dwRead;
    BOOL bSuccess;
    bSuccess = ReadConsole( GetStdHandle(STD_INPUT_HANDLE), strBuffer, 128, &dwRead, NULL );

    if( !bSuccess || dwRead < 2 )
    {
        _tcscpy( strBuffer, TEXT("") );
        return;
    }
        
    strBuffer[dwRead-2]=0;
    m_pMazeApp->ConsolePrintf( LINE_INPUT, strBuffer );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CConsoleGraphics::RunSetupWizard()
{
    BOOL bWrongToken;
    TCHAR buffer[512];

    m_pConfig->bConnectToMicrosoftSite = FALSE;
    m_pConfig->bConnectToLocalServer   = FALSE;
    m_pConfig->bConnectToRemoteServer  = FALSE;

    bWrongToken = TRUE;
    while( bWrongToken ) 
    {
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Do you want to connect to DirectPlayMaze.rte.microsoft.com? (YES/NO)") );
        DoPrompt( TEXT("(Default:YES) > "), buffer );

        TCHAR* token = _tcstok( buffer, TEXT(" \t") );
        if( token != NULL )
        {
            _tcsupr( token );
            if( !_tcscmp( token, TEXT("Y") ) || !_tcscmp( token, TEXT("YES") ) )
            {
                m_pConfig->bConnectToMicrosoftSite = TRUE;
                bWrongToken = FALSE;
            }
            else if( !_tcscmp( token, TEXT("N") ) || !_tcscmp( token, TEXT("NO") ) )
            {
                bWrongToken = FALSE;
            }
        }
        else
        {
            m_pConfig->bConnectToMicrosoftSite = TRUE;
            bWrongToken = FALSE;
        }
    }

    if( m_pConfig->bConnectToMicrosoftSite == FALSE )
    {
        bWrongToken = TRUE;
        while( bWrongToken ) 
        {
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Do you want to connect to a local server (searches local subnet)? (YES/NO)") );
            DoPrompt( TEXT("(Default:YES) > "), buffer );

            TCHAR* token = _tcstok( buffer, TEXT(" \t") );
            if( token != NULL )
            {
                _tcsupr( token );
                if( !_tcscmp( token, TEXT("Y") ) || !_tcscmp( token, TEXT("YES") ) )
                {
                    m_pConfig->bConnectToLocalServer   = TRUE;
                    bWrongToken = FALSE;
                }
                else if( !_tcscmp( token, TEXT("N") ) || !_tcscmp( token, TEXT("NO") ) )
                {
                    bWrongToken = FALSE;
                }
            }
            else
            {
                m_pConfig->bConnectToLocalServer   = TRUE;
                bWrongToken = FALSE;
            }
        }

        if( m_pConfig->bConnectToLocalServer == FALSE )
        {
            bWrongToken = TRUE;
            while( bWrongToken ) 
            {
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("What IP address do you want to connect to? ") );
                DoPrompt( TEXT("> "), buffer );

                TCHAR* token = _tcstok( buffer, TEXT(" \t") );
                if( token != NULL )
                {
                    _tcscpy( m_pConfig->szIPAddress, token );
                    m_pConfig->bConnectToRemoteServer = TRUE;
                    bWrongToken = FALSE;
                }
            }
        }
    }

    bWrongToken = TRUE;
    while( bWrongToken ) 
    {
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Turn file logging on? (YES/NO)") );
        DoPrompt( TEXT("(Default:YES) > "), buffer );

        TCHAR* token = _tcstok( buffer, TEXT(" \t") );
        if( token != NULL )
        {
            _tcsupr( token );
            if( !_tcscmp( token, TEXT("Y") ) || !_tcscmp( token, TEXT("YES") ) )
            {
                m_pConfig->bFileLogging = TRUE;
                bWrongToken = FALSE;
            }
            else if( !_tcscmp( token, TEXT("N") ) || !_tcscmp( token, TEXT("NO") ) )
            {
                m_pConfig->bFileLogging = FALSE;
                bWrongToken = FALSE;
            }
        }
        else
        {
            m_pConfig->bFileLogging = TRUE;
            bWrongToken = FALSE;
        }
    }

    bWrongToken = TRUE;
    while( bWrongToken ) 
    {
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Use these settings when disconnected and when the app starts? (YES/NO)") );
        DoPrompt( TEXT("(Default:YES) > "), buffer );

        TCHAR* token = _tcstok( buffer, TEXT(" \t") );
        if( token != NULL )
        {
            _tcsupr( token );
            if( !_tcscmp( token, TEXT("Y") ) || !_tcscmp( token, TEXT("YES") ) )
            {
                m_pConfig->bAutoConnnect = TRUE;
                bWrongToken = FALSE;
            }
            else if( !_tcscmp( token, TEXT("N") ) || !_tcscmp( token, TEXT("NO") ) )
            {
                m_pConfig->bAutoConnnect = FALSE;
                bWrongToken = FALSE;
            }
        }
        else
        {
            m_pConfig->bAutoConnnect = TRUE;
            bWrongToken = FALSE;
        }
    }

    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Type 'SETUP' anytime to change these connections settings.") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Type 'HELP' for a list of commands.") );
    m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("") );

    m_pMazeApp->SetSaveSettings( TRUE );
    m_pMazeApp->WriteConfig();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CConsoleGraphics::ParseInput( TCHAR* buffer )
{
    // Strip first token from the buffer and pull to upper case
    TCHAR* token = _tcstok( buffer, TEXT(" \t") );
    if( token == NULL )
        return;

    _tcsupr( token );

    // See what it is and act accordingly
    if( !_tcscmp( token, TEXT("STOP") ) || 
        !_tcscmp( token, TEXT("QUIT") ) || 
        !_tcscmp( token, TEXT("EXIT") ) )
    {
        m_bQuitThread = TRUE;
    } 
    else if( !_tcscmp( token, TEXT("CONNECT") ) )
    {
        m_pMazeApp->SetConnectNow( TRUE );
    } 
    else if( !_tcscmp( token, TEXT("DISCONNECT") ) )
    {
        m_pMazeApp->SetDisconnectNow( TRUE );
    } 
    else if( !_tcscmp( token, TEXT("AUTOCONNECT") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            _tcsupr( token );
            if( !_tcscmp( token, TEXT("ON") ) )
                m_pConfig->bAutoConnnect = TRUE;
            else if( !_tcscmp( token, TEXT("OFF") ) )
                m_pConfig->bAutoConnnect = FALSE;
            m_pMazeApp->WriteConfig();
        }

        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Auto-Connect set to %s"), (m_pConfig->bAutoConnnect) ? TEXT("ON") : TEXT("OFF") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("  If ON, MazeConsoleClient connects automatically reconnects") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("         when disconnected, and connects when loading app.") );
    } 
    else if( !_tcscmp( token, TEXT("SETRETRYDELAY") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            DWORD dwDelay = _ttol( token );
            if( m_pConfig->dwNetworkRetryDelay < 1 || m_pConfig->dwNetworkRetryDelay > 300 )
            {
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Must enter a value between 1 and 300") );
            }
            else
            {           
                m_pConfig->dwNetworkRetryDelay = dwDelay;
            }
        }

        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Retry Delay set to %d"), m_pConfig->dwNetworkRetryDelay );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("  Sets how long the app waits between attempts to connect to the server.") );
    }     
    else if( !_tcscmp( token, TEXT("LOGLEVEL") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            DWORD dwLevel = _ttol( token );
            if( dwLevel > 3 )
            {
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Must enter a value between 0 and 3") );
            }
            else
            {
                m_pConfig->dwLogLevel = dwLevel;
            }
        }

        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Log level set to %d"), m_pConfig->dwLogLevel );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("     Level 1: No client position ") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("     Level 2: Client position every 10 seconds") );
    }
    else if( !_tcscmp( token, TEXT("SETSTATRATE") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            DWORD dwLevel = _ttol( token );
            m_pConfig->dwAutoPrintStats = dwLevel;
        }

        if( m_pConfig->dwAutoPrintStats != 0 )
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Auto-displaying stats every %d mins"), m_pConfig->dwAutoPrintStats );
        else
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Not auto-displaying stats") );

        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("     Controls how often the connection stats are auto-displayed") );
    }
    else if( !_tcscmp( token, TEXT("SETUP") ) )
    {
        m_pMazeApp->SetAllowConnect( FALSE );
        m_pMazeApp->SetDisconnectNow( TRUE );
        RunSetupWizard();

        if( m_pConfig->bFileLogging )
            m_pMazeApp->CreateTempLogFile();
        else
            m_pMazeApp->CloseTempLogFile();

        m_pMazeApp->SetConnectNow( TRUE );
        m_pMazeApp->SetAllowConnect( TRUE );
    }
    else if( !_tcscmp( token, TEXT("FILELOG") ) )
    {
        m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Log Directory: '%s'"), m_pMazeApp->GetLogDir() );
        m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Logging to temp file: '%s'"), m_pMazeApp->GetLogFile() );
    }
    else if( !_tcscmp( token, TEXT("AUTODISCONNECT") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            _tcsupr( token );
            if( !_tcscmp( token, TEXT("ON") ) )
                m_pConfig->bAutoDisconnnect = TRUE;
            else if( !_tcscmp( token, TEXT("OFF") ) )
                m_pConfig->bAutoDisconnnect = FALSE;
            m_pMazeApp->WriteConfig();
        }

        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Auto-Disconnect set to %s"), (m_pConfig->bAutoDisconnnect) ? TEXT("ON") : TEXT("OFF") );
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("  If ON, app disconnects randomly") );
    } 
    else if( !_tcscmp( token, TEXT("CONNECTIONINFO") ) || !_tcscmp( token, TEXT("CI") ) )
    {
        TCHAR strInfo[5000];
        TCHAR* strEndOfLine;
        TCHAR* strStartOfLine;

        // Query the IOutboudNet for info about the connection to this user
        m_pDP8Client->GetConnectionInfo( strInfo );

        m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("Displaying connection info for 0x%0.8x"), m_pMazeClient->GetLocalClientID() );
        m_pMazeApp->ConsolePrintf( LINE_LOG, TEXT("(Key: G=Guaranteed NG=Non-Guaranteed B=Bytes P=Packets)") );

        // Display each line seperately
        strStartOfLine = strInfo;
        while( TRUE )
        {
            strEndOfLine = _tcschr( strStartOfLine, '\n' );
            if( strEndOfLine == NULL )
                break;

            *strEndOfLine = 0;
            m_pMazeApp->ConsolePrintf( LINE_LOG, strStartOfLine );
            strStartOfLine = strEndOfLine + 1;
        }
    }
    else if( !_tcscmp( token, TEXT("SPTHREADS") ) || !_tcscmp( token, TEXT("SPT") ) )
    {
        DWORD currentthreads = m_pDP8Client->GetNumSPThreads();

        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            DWORD threads = _ttol( token );
            if( (threads > MAX_SP_THREADS) || (threads <= currentthreads) )
            {
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Must enter a value between %d and %d"), 
                                                            currentthreads, MAX_SP_THREADS );
            }
            else
            {
                m_pDP8Client->SetNumSPThreads( threads );
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("SP Threads Set to %d"), threads );
            }
        }
        else
        {
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("SP Threads set to %d"), 
                                                       m_pDP8Client->GetNumSPThreads() );
        }
    }
    else if( !_tcscmp( token, TEXT("SPBUFFER") ) || !_tcscmp( token, TEXT("SPB") ) )
    {
        token = _tcstok( NULL, TEXT(" \t") );
        if( token )
        {
            DWORD buffer = _ttol( token );
            if( (buffer > MAX_SP_BUFFER) )
            {
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Must enter a value between 0 and %d"), 
                                                                                    MAX_SP_BUFFER );
            }
            else
            {
                m_pDP8Client->SetSPBuffer( buffer );
                m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("SP Buffer Size Set to %d"),
                                                                    m_pDP8Client->GetSPBuffer() );
            }
        }
        else
        {
            m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("SP Buffer Size set to %d"), 
                                                                    m_pDP8Client->GetSPBuffer() );
        }
    }
    else if( !_tcscmp( token, TEXT("HELP") ) || !_tcscmp( token, TEXT("?") ) )
    {
        PrintHelp();
    }
    else if( !_tcscmp( token, TEXT("CLS") ) )
    {
        ClearScreen();
    }
    else
    {
        m_pMazeApp->ConsolePrintf( LINE_CMD, TEXT("Unknown command. Type HELP for list of commands") );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
UINT WINAPI CConsoleGraphics::StaticPromptThread( LPVOID pParam )
{
    return s_pGraphics->PromptThread( pParam );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
UINT WINAPI CConsoleGraphics::PromptThread( LPVOID pParam )
{
    BOOL bHaveConnectSettings = m_pConfig->bAutoConnnect;

    // Loop around getting and dealing with keyboard input
    TCHAR buffer[512];
    while( !m_bQuitThread )
    {
        if( !bHaveConnectSettings )
        {
            // If we don't have the connection settings yet, 
            // then prompt for where to connect to
            RunSetupWizard();

            if( m_pConfig->bFileLogging )
                m_pMazeApp->CreateTempLogFile();
            else
                m_pMazeApp->CloseTempLogFile();

            m_pMazeApp->SetAllowConnect( TRUE );
            bHaveConnectSettings = TRUE;
        }
        else
        {
            DoPrompt( TEXT("Command> "), buffer );
            ParseInput( buffer );
        }
    };

    _tprintf( TEXT("Stopping...") );

    return 0;
}

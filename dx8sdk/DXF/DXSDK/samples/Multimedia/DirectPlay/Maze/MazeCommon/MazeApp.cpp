//----------------------------------------------------------------------------
// File: mazeapp.cpp
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
#include <tchar.h>
#include <math.h>
#include <process.h>
#include <dxerr8.h>
#include <dplay8.h>
#include "DXUtil.h"
#include "SyncObjects.h"
#include "DummyConnector.h"
#include "DPlay8Client.h"
#include "MazeClient.h"
#include "IMazeGraphics.h"
#include "MazeApp.h"


static CMazeApp* s_pMazeApp = NULL;


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CMazeApp::CMazeApp()
{
    s_pMazeApp                  = this;
    m_hOutputMsgThread          = NULL;
    m_bQuitThread               = NULL;
    m_hOutputMsgEvent           = NULL;
    m_dwNextOutputMsg           = 0;
    m_bLocalLoopback            = TRUE;
    m_bAllowConnect             = FALSE;
    m_bConnectNow               = TRUE;
    m_bSaveSettings             = TRUE;
    m_lQueueSize                = 0;
    m_dwNextFreeOutputMsg       = 0;
    m_hLogFile                  = NULL;

    m_bLocalLoopbackInitDone    = FALSE;
    m_bInitDone                 = FALSE;

    // Create an event object to flag pending output messages
    m_hOutputMsgEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    //Initialize our Config structure to 0.
    ZeroMemory(&m_Config, sizeof(m_Config));

    //Must init buffer size to -1, since 0 is valid.
    m_Config.dwSPBufferSize = 0xffffffff;


}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CMazeApp::~CMazeApp()
{
    CloseHandle( m_hOutputMsgEvent );
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMazeApp::Create( IMazeGraphics* pMazeGraphics )
{
    m_pMazeGraphics = pMazeGraphics;
    if( m_pMazeGraphics == NULL )
        return E_FAIL;

    m_pMazeGraphics->Init( this, &m_DP8Client, &m_MazeClient );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
int CMazeApp::Run( HINSTANCE hInst )
{
    if( NULL == m_pMazeGraphics )
        return 0;

    // Start the timer and init the random seed
    DXUtil_Timer( TIMER_START );
    DWORD dwSRand = (DWORD) (DXUtil_Timer( TIMER_GETABSOLUTETIME ) * UINT_MAX * (DWORD)GetCurrentThreadId() );
    srand( dwSRand );

    // Tell OS's that have power management to not 
    // sleep, since this app will be using the 
    // network connection and need very little user input
    SuspendPowerManagement();

    // Initialize COM
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    // Extract configuration settings from the registry
    ReadConfig();

    if( m_Config.bFileLogging )
        CreateTempLogFile();

    if( SUCCEEDED( m_pMazeGraphics->Create( hInst ) ) )
    {
        ConsolePrintf( LINE_LOG, TEXT("DirectPlayMaze client started.") );

        // Spin up a thread to record/display the output
        UINT dwOutputMsgThreadID;
        m_hOutputMsgThread = (HANDLE)_beginthreadex( NULL, 0, StaticOutputMsgThread, 
                                                  NULL, 0, &dwOutputMsgThreadID );

        // Initialize maze client object - basically just build the maze
        m_MazeClient.Init( this, m_pMazeGraphics );

        m_pMazeGraphics->Run();

        // Wait for threads to shutdown
        DXUtil_Trace( TEXT("Quiting\n") );
        m_bQuitThread = TRUE;
        SetEvent( m_hOutputMsgEvent );
        WaitForSingleObject( m_hOutputMsgThread, INFINITE );
        CloseHandle( m_hOutputMsgThread );

        // Write configuration settings to registry
        WriteConfig();

        m_pMazeGraphics->Shutdown();
    }

    // Clean up
    DXUtil_Trace( TEXT("Shutting down client\n") );
    m_MazeClient.Shutdown();
    DXUtil_Trace( TEXT("Shutting down dp8\n") );
    m_DP8Client.Shutdown();
    CoUninitialize();

    return 0;
}
    



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::SuspendPowerManagement()
{
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstKernel32 = NULL;
    typedef EXECUTION_STATE (WINAPI* LPSETTHREADEXECUTIONSTATE)( EXECUTION_STATE esFlags );
    LPSETTHREADEXECUTIONSTATE pSetThreadExecutionState = NULL;

    GetSystemDirectory(szPath, MAX_PATH);

    // SetThreadExecutionState() isn't availible on some old OS's, 
    // so do a LoadLibrary to get to it.
    lstrcat(szPath, TEXT("\\kernel32.dll"));
    hInstKernel32 = LoadLibrary(szPath);

    if (hInstKernel32 != NULL)
    {
        pSetThreadExecutionState = (LPSETTHREADEXECUTIONSTATE)GetProcAddress(hInstKernel32, "SetThreadExecutionState");
        if( pSetThreadExecutionState != NULL )
        {
            // Tell OS's that have power management to not 
            // sleep, since this app will be using the 
            // network connection and need very little user input
            pSetThreadExecutionState( ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
        }

        FreeLibrary(hInstKernel32);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeApp::FrameMove( FLOAT fElapsedTime )
{
    HRESULT hr;
    FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );

    static FLOAT s_fLastConnect     = INT_MIN;
    static FLOAT s_fStartEnumTime   = INT_MIN;
    static FLOAT s_fStopEnumTime    = INT_MIN;
    static FLOAT s_fEnumStarted     = FALSE;

    if( m_DP8Client.IsSessionLost() ) 
    {

        if( FALSE == m_bLocalLoopbackInitDone )
        {
            if( m_bInitDone )
            {
                ConsolePrintf( LINE_LOG, TEXT("Disconnected from server") );

                if( m_DP8Client.GetSessionLostReason() == DISCONNNECT_REASON_CLIENT_OUT_OF_DATE )
                {
                    ConsolePrintf( LINE_LOG, TEXT("Disconnected because MazeClient is out of date.") );
                    ConsolePrintf( LINE_LOG, TEXT("Please get updated version") );
                    ConsolePrintf( LINE_LOG, TEXT("from http://msdn.microsoft.com/directx/") );
                    m_bOutOfDateClient = TRUE;
                }

                // Disconnected, so retry in 10 seconds
                s_fStopEnumTime = fCurTime - m_Config.dwNetworkRetryDelay * 60.0f + 10.0f;     
            }
            else
            {
                // If just starting up, then retry immediately
                m_bInitDone = TRUE;
            }

            m_MazeClient.LockWorld();
            m_MazeClient.Reset();

            // Now that the session is lost we need to 
            // restart DirectPlay by calling Close() 
            // and Init() on m_pDPlay
            m_DP8Client.Shutdown();

            //Pass in the our structure in order to get out configuration data.
            m_DP8Client.Init(GetConfig());

            if( m_bAllowLoopback )
                InitServerForLoopback();

            m_MazeClient.UnlockWorld();
            m_bLocalLoopbackInitDone = TRUE;
        }

        if( ( !s_fEnumStarted && fCurTime - s_fStopEnumTime > m_Config.dwNetworkRetryDelay * 60.0f || m_bConnectNow )
            && m_bAllowConnect && !m_bOutOfDateClient )
        {
            m_bConnectNow = FALSE;

            if( SUCCEEDED( hr = StartSessionEnum() ) )
            {
                // DirectPlay host enumeration started
                ConsolePrintf( LINE_LOG, TEXT("Starting DirectPlay host enumeration") );
                s_fStartEnumTime = fCurTime;
                s_fEnumStarted = TRUE;
            }
            else
            {
                //If we are in the stress connect, try the next connection. 
                //When we get back to 1, then we have gone through the list, so let's wait.
                if(m_Config.dwStressConnect > 1) 
                {
                    //We are going through the list of machines to attempt connection. So lets try the next right away.
                    ConsolePrintf( LINE_LOG, TEXT("Host not found. Trying local subnet.") );
                    m_bConnectNow = TRUE;  //Try the enum right away with the next item in the list.
                }
                else
                {
                    ConsolePrintf( LINE_LOG, TEXT("DirectPlay host enumeration failed to start.") );
                    ConsolePrintf( LINE_LOG, TEXT("Will try again in %d minutes."), m_Config.dwNetworkRetryDelay );
                }
                
                // DirectPlay host enumeration failed to start
                // Will try again in m_Config.dwNetworkRetryDelay minutes
                s_fStopEnumTime = fCurTime;
                s_fEnumStarted = FALSE;
            }
        }

        if( s_fEnumStarted && fCurTime - s_fStartEnumTime > 5.0f * 60.0f )
        {
            //If we are in the stress connect, try the next connection. 
            //When we get back to 1, then we have gone through the list, so let's start over.
            if(m_Config.dwStressConnect > 1) 
            {
                //We are going through the list of machines to attempt connection. So lets try the next right away.
                ConsolePrintf( LINE_LOG, TEXT("Host not found. Trying local subnet.") );
                m_bConnectNow = TRUE;  //Try the enum right away with the next item in the list.
            }
            else
            {
                ConsolePrintf( LINE_LOG, TEXT("No host found. Stopping DirectPlay host enumeration") );
                ConsolePrintf( LINE_LOG, TEXT("Will try again in %d minutes."), m_Config.dwNetworkRetryDelay );
            }
            // Stop enumeration
            m_DP8Client.StopSessionEnum();
            s_fStopEnumTime = fCurTime;
            s_fEnumStarted = FALSE;
        }

        if( s_fEnumStarted && fCurTime - s_fLastConnect > 0.5f )
        {
            if( TRUE == TryToConnect() )
            {
                // Connect successful 
                ConsolePrintf( LINE_LOG, TEXT("Connected to server.  Host enumeration stopped.") );
                m_bLocalLoopback    = FALSE;
                s_fEnumStarted      = FALSE;
                m_bLocalLoopbackInitDone = FALSE;
                
            }

            s_fLastConnect = fCurTime;
        }

        m_bDisconnectNow = FALSE;
    }
    else
    {
        m_bLocalLoopbackInitDone = FALSE;

        if( m_Config.dwLogLevel > 1 )
        {
            // Display position every so often
            static float fLastPosUpdate = fCurTime;
            if( fCurTime - fLastPosUpdate > 10.0f )
            {
                D3DXVECTOR3 vPos = m_MazeClient.GetCameraPos(); 
                DWORD dwNumPlayers, dwNumNearbyPlayers;
                m_MazeClient.GetPlayerStats( &dwNumPlayers, &dwNumNearbyPlayers );
                ConsolePrintf( LINE_LOG, TEXT("Position: (%5.1f,%5.1f), Players: %d, Nearby Players: %d"), 
                              vPos.x, vPos.z, dwNumPlayers, dwNumNearbyPlayers );
                fLastPosUpdate = fCurTime;
            }
        }

        // Display connection info every so often
        static float fLastLogUpdate = fCurTime;
        if( m_Config.dwAutoPrintStats > 0 && 
            fCurTime - fLastLogUpdate > m_Config.dwAutoPrintStats * 60.0f )
        {
            D3DXVECTOR3 vPos = m_MazeClient.GetCameraPos(); 
            DWORD dwNumPlayers, dwNumNearbyPlayers;
            m_MazeClient.GetPlayerStats( &dwNumPlayers, &dwNumNearbyPlayers );
            ConsolePrintf( LINE_LOG, TEXT("Position: (%5.1f,%5.1f), Players: %d, Nearby Players: %d"), 
                          vPos.x, vPos.z, dwNumPlayers, dwNumNearbyPlayers );

            TCHAR strInfo[5000];
            TCHAR* strEndOfLine;
            TCHAR* strStartOfLine;

            // Query the IOutboudNet for info about the connection to this user
            m_DP8Client.GetConnectionInfo( strInfo );

            ConsolePrintf( LINE_LOG, TEXT("Displaying connection info for 0x%0.8x"), m_MazeClient.GetLocalClientID() );
            ConsolePrintf( LINE_LOG, TEXT("(Key: G=Guaranteed NG=Non-Guaranteed B=Bytes P=Packets)") );

            // Display each line seperately
            strStartOfLine = strInfo;
            while( TRUE )
            {
                strEndOfLine = _tcschr( strStartOfLine, '\n' );
                if( strEndOfLine == NULL )
                    break;

                *strEndOfLine = 0;
                ConsolePrintf( LINE_LOG, strStartOfLine );
                strStartOfLine = strEndOfLine + 1;
            }

            fLastLogUpdate = fCurTime;
        }

        // If we are testing connect/disconnect, break after so many iterations.
        if( m_Config.bAutoDisconnnect )  
        {
            // Disconnect between 5-25seconds 
            static float fDisconnectCountdown = 10.0f;

            fDisconnectCountdown -= fElapsedTime;
            if( fDisconnectCountdown < 0.0f )  
            {
                fDisconnectCountdown = (float)(rand() % 20000 + 5000 ) / 1000.0f;
                ConsolePrintf( LINE_LOG, TEXT("Intentional disconnect.  Connecting again for %0.0f seconds..."), fDisconnectCountdown );
                m_DP8Client.Shutdown();
                m_MazeClient.Shutdown();
            }
        }

        if( m_bDisconnectNow )
        {
            m_bDisconnectNow = FALSE;
            ConsolePrintf( LINE_LOG, TEXT("Intentional disconnect.") );
            m_DP8Client.Shutdown();
            m_MazeClient.Shutdown();

            if( m_bAllowLoopback )
                InitServerForLoopback();
        }
    }

    // Update state of client
    m_MazeClient.Update( fElapsedTime );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeApp::StartSessionEnum()
{
    HRESULT hr;

    // If we're not the preview, then enum sessions
    if( m_pMazeGraphics->IsPreview() )
        return S_OK;

    // Start enumerating available sessions at specified IP address.

    if( m_Config.bConnectToMicrosoftSite )
    {
        ConsolePrintf( LINE_LOG, TEXT("Connecting to DirectPlayMaze.rte.microsoft.com") );
        hr = m_DP8Client.StartSessionEnum( MICROSOFT_SERVER );
    }
    else if( m_Config.bConnectToLocalServer )
    {
        ConsolePrintf( LINE_LOG, TEXT("Connecting to local server (searches the local subnet)") );
        hr = m_DP8Client.StartSessionEnum( TEXT("") );
    }
    else if( m_Config.bConnectToRemoteServer )
    {
        ConsolePrintf( LINE_LOG, TEXT("Connecting to remote server at '%s'"), m_Config.szIPAddress );
        hr = m_DP8Client.StartSessionEnum( m_Config.szIPAddress );
    }
    // If users wants Stress Connect, go through the sequence they have requested.
    else if( m_Config.dwStressConnect )
    {
        if( m_Config.dwStressConnect == 1 )
        {
            // Point to local subnet for next enum.
            m_Config.dwStressConnect = 2;

            ConsolePrintf( LINE_LOG, TEXT("Connecting to remote server at '%s'"), m_Config.szIPAddress );
            hr = m_DP8Client.StartSessionEnum( m_Config.szIPAddress );
            
        }
        else
        {
            // Point back at remote server for next enum.
            m_Config.dwStressConnect = 1;

            // Must equal 2 or something higher. Try the local subnet.
            ConsolePrintf( LINE_LOG, TEXT("Connecting to local server (searches the local subnet)") );
            hr = m_DP8Client.StartSessionEnum( TEXT("") );
            
        }
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL CMazeApp::TryToConnect()
{
    if( m_DP8Client.GetNumSessions() > 0 )
    {
        m_MazeClient.Reset();
        m_MazeClient.SetOutboundClient( m_DP8Client.GetOutboundClient() );
        m_DP8Client.SetClient( &m_MazeClient );

        // Loop through the available sessions and attempt to connect
        for( DWORD i = 0; i < m_DP8Client.GetNumSessions(); i++ )
        {
            if( SUCCEEDED(m_DP8Client.JoinSession( i ) ) )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMazeApp::InitServerForLoopback()
{
    HRESULT hr;

    m_bLocalLoopback = TRUE;

    #define LOOPBACK_MAZE_WIDTH  16
    #define LOOPBACK_MAZE_HEIGHT 16

    // Initalize maze and server objects for loopback mode
    if( FAILED( hr = m_MazeClient.m_Maze.Init( LOOPBACK_MAZE_WIDTH, 
                                               LOOPBACK_MAZE_HEIGHT, 
                                               DEFAULT_SEED ) ) )
    {
        return DXTRACE_ERR( TEXT("m_Maze.Init"), hr );
    }

    // Initialize maze server object - hook up to the maze object in the client
    m_MazeServer.Init( m_bLocalLoopback, &m_MazeClient.m_Maze );

    m_DummyClientConnection.SetTarget( &m_MazeServer );
    m_MazeClient.SetOutboundClient( &m_DummyClientConnection );
    m_DummyServerConnection.SetTarget( &m_MazeClient );
    m_MazeServer.SetOutboundServer( &m_DummyServerConnection );

    m_DummyClientConnection.Connect( 2 );
    m_MazeClient.EngageAutopilot( TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
UINT WINAPI CMazeApp::StaticOutputMsgThread( LPVOID pParam )
{
    return s_pMazeApp->OutputMsgThread( pParam );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
UINT WINAPI CMazeApp::OutputMsgThread( LPVOID pParam )
{
#define MAX_LOG_LINES 100

    TCHAR szLogBuffer[MAX_PATH];
    DWORD dwNumProcessed = 0;

    while( 1 )
    {
        // Wait for output to be added to the queue or the quit flag to be set
        WaitForSingleObject( m_hOutputMsgEvent, INFINITE );
        if( m_bQuitThread )
            break;

        // Update the time stamp
        UpdateTimeStamp();

        // Lock output queue
        m_OutputMsgQueueLock.Enter();

        dwNumProcessed = 0;

        // While we have there are messages to print and we
        // have display'ed less than 5 messages 
        while ( m_lQueueSize > 0 && dwNumProcessed < 5 )
        {
            switch( m_EnumLineType[m_dwNextOutputMsg] )
            {
                case LINE_LOG:
                {
                    // Add m_szOutputMsgBuffer[m_dwNextOutputMsg] to szLogBuffer array,
                    // and redisplay the array on the top half of the screen
                    _stprintf( szLogBuffer, TEXT("%s %s"), 
                               m_strTimeStamp, m_szOutputMsgBuffer[m_dwNextOutputMsg] );

#ifdef _DEBUG
                    OutputDebugString( szLogBuffer );
                    OutputDebugString( TEXT("\n") );
#endif
                    if( m_hLogFile )
                    {
                        DWORD dwWritten;
                        WriteFile( m_hLogFile, szLogBuffer, 
                                   lstrlen( szLogBuffer ), &dwWritten, NULL );
                        TCHAR strEOL = TEXT('\r');
                        WriteFile( m_hLogFile, &strEOL, 
                                   sizeof(TCHAR), &dwWritten, NULL );
                        strEOL = TEXT('\n');
                        WriteFile( m_hLogFile, &strEOL, 
                                   sizeof(TCHAR), &dwWritten, NULL );

                        static float s_fLastFlushTime = DXUtil_Timer( TIMER_GETAPPTIME );
                        float fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
                        if( fCurTime - s_fLastFlushTime > 0.2f )
                        {
                            FlushFileBuffers( m_hLogFile );
                            s_fLastFlushTime = fCurTime;
                        }
                    }
                    break;
                }

                default:
                    _tcscpy( szLogBuffer, m_szOutputMsgBuffer[m_dwNextOutputMsg] );
                    break;
            }

            m_pMazeGraphics->HandleOutputMsg( m_EnumLineType[m_dwNextOutputMsg], szLogBuffer );

            m_dwNextOutputMsg++;
            if( m_dwNextOutputMsg == MAX_OUTPUT_QUEUE )
                m_dwNextOutputMsg = 0;

            m_lQueueSize--;
            dwNumProcessed++;
        }

        // Unlock output queue
        m_OutputMsgQueueLock.Leave();

        if( m_hLogFile )
            FlushFileBuffers( m_hLogFile );

        // Yield time to other threads
        Sleep( 10 );

        // If there are still messages left, then signal the event
        if( m_lQueueSize > 0 )
            SetEvent( m_hOutputMsgEvent );
    }

    if( m_hLogFile )
    {
        CloseHandle( m_hLogFile );
        m_hLogFile = NULL;
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::ConsolePrintf( EnumLineType enumLineType, const TCHAR* fmt, ... )
{
    // Format the message into a buffer
    TCHAR buffer[512];
    _vstprintf( buffer, fmt, (CHAR*) ((&fmt)+1) );

    // Lock the output queue
    m_OutputMsgQueueLock.Enter();

    // Find free spot
    if( m_lQueueSize != MAX_OUTPUT_QUEUE )
    {
        // Format message into the buffer
        _vstprintf( m_szOutputMsgBuffer[m_dwNextFreeOutputMsg], fmt, (CHAR*)((&fmt)+1) );
        m_EnumLineType[m_dwNextFreeOutputMsg] = enumLineType;

        // Increment output pointer and wrap around
        m_dwNextFreeOutputMsg++;
        if( m_dwNextFreeOutputMsg == MAX_OUTPUT_QUEUE )
            m_dwNextFreeOutputMsg = 0;

        // Increment message count
        m_lQueueSize++;
    }

    // Unlock output queue
    m_OutputMsgQueueLock.Leave();

    // Signal event so the output thread empties the queue
    SetEvent( m_hOutputMsgEvent );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::UpdateTimeStamp()
{
    static float s_fTimeStampUpdateCountdown = -10.0f;
    float fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );

    if( fCurTime - s_fTimeStampUpdateCountdown > 1.0f )
    {
        SYSTEMTIME sysTime;
        GetLocalTime( &sysTime );
        _stprintf( m_strTimeStamp, TEXT("[%02d-%02d-%02d %02d:%02d:%02d]"),
                   sysTime.wMonth, sysTime.wDay, sysTime.wYear % 100, 
                   sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

        // Compute how many milliseconds until the next second change
        s_fTimeStampUpdateCountdown = fCurTime;
    }
}
    



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::CreateTempLogFile()
{
    BOOL bSuccess;
    TCHAR strTempFileName[MAX_PATH];
    TCHAR strTime[MAX_PATH];
    DWORD dwCount;
    
    GetTempPath( MAX_PATH, m_strLogDir );
    lstrcat( m_strLogDir, TEXT("DirectPlayMaze\\") );

    // Create the directory if it doesn't exist
    if( GetFileAttributes( m_strLogDir ) == -1 )
    {
        bSuccess = CreateDirectory( m_strLogDir, NULL );
        if( !bSuccess )
        {
            ConsolePrintf( LINE_LOG, TEXT("Could not create create temp directory '%s'"), m_strLogDir );
            goto LFail;
        }
    }

    ConsolePrintf( LINE_LOG, TEXT("Log Directory: '%s'"), m_strLogDir );

    SYSTEMTIME sysTime;
    GetLocalTime( &sysTime );
    _stprintf( strTime, TEXT("client-%04d-%02d-%02d-"),
               sysTime.wYear, sysTime.wMonth, sysTime.wDay );

    dwCount = 0;

    while(TRUE)
    {
        wsprintf( m_strLogFile, TEXT("%s%05d.log"), strTime, dwCount );
        lstrcpy( strTempFileName, m_strLogDir );
        lstrcat( strTempFileName, m_strLogFile );
        DWORD dwResult = GetFileAttributes( strTempFileName );
        if( dwResult == -1 )
            break;

        dwCount++;
    }

    if( m_hLogFile )
    {
        CloseHandle( m_hLogFile );
        m_hLogFile = NULL;
    }

    m_hLogFile = CreateFile( strTempFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                             CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
    if( m_hLogFile == INVALID_HANDLE_VALUE )
    {
        ConsolePrintf( LINE_LOG, TEXT("Could not create create temp file '%s'"), strTempFileName );
        goto LFail;
    }

    ConsolePrintf( LINE_LOG, TEXT("Logging to temp file: '%s'"), m_strLogFile );
    return;

LFail:
    ConsolePrintf( LINE_LOG, TEXT("File logging disabled") );
    m_Config.bFileLogging = FALSE;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::CloseTempLogFile()
{
    CloseHandle( m_hLogFile );
    m_hLogFile = NULL;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::ReadConfig()
{
    HKEY hKey = NULL;
    RegOpenKeyEx( HKEY_CURRENT_USER, MAZE_REGKEYNAME, 0, KEY_READ, &hKey );

    DXUtil_ReadBoolRegKey( hKey, TEXT("ConnectToMicrosoftSite"), &m_Config.bConnectToMicrosoftSite, TRUE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("ConnectToLocalServer"), &m_Config.bConnectToLocalServer, FALSE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("ConnectToRemoteServer"), &m_Config.bConnectToRemoteServer, FALSE );
    DXUtil_ReadIntRegKey(  hKey, TEXT("NetworkRetryDelay"), &m_Config.dwNetworkRetryDelay, 30 );
    DXUtil_ReadBoolRegKey( hKey, TEXT("FileLogging"), &m_Config.bFileLogging, TRUE );
    DXUtil_ReadStringRegKey( hKey, TEXT("IPAddress"), m_Config.szIPAddress, sizeof(m_Config.szIPAddress), TEXT("\0") );

    DXUtil_ReadBoolRegKey( hKey, TEXT("ShowFramerate"), &m_Config.bShowFramerate, TRUE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("ShowIndicators"), &m_Config.bShowIndicators, TRUE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("DrawMiniMap"), &m_Config.bDrawMiniMap, TRUE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("FullScreen"), &m_Config.bFullScreen, TRUE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("Reflections"), &m_Config.bReflections, FALSE );

    DXUtil_ReadBoolRegKey( hKey, TEXT("AutoDisconnnect"), &m_Config.bAutoDisconnnect, FALSE );
    DXUtil_ReadBoolRegKey( hKey, TEXT("AutoConnnect"), &m_Config.bAutoConnnect, FALSE );
    DXUtil_ReadIntRegKey( hKey, TEXT("LogLevel"), &m_Config.dwLogLevel, 2 );
    DXUtil_ReadIntRegKey( hKey, TEXT("AutoPrintStats"), &m_Config.dwAutoPrintStats, 10 );

    RegCloseKey( hKey );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CMazeApp::WriteConfig()
{
    HKEY    hKey;
    DWORD   dwDisposition;
    
    if( !m_bSaveSettings )
        return;

    RegCreateKeyEx( HKEY_CURRENT_USER, MAZE_REGKEYNAME, 0, NULL, 
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey, &dwDisposition );

    DXUtil_WriteBoolRegKey( hKey, TEXT("ConnectToMicrosoftSite"), m_Config.bConnectToMicrosoftSite );
    DXUtil_WriteBoolRegKey( hKey, TEXT("ConnectToLocalServer"), m_Config.bConnectToLocalServer );
    DXUtil_WriteBoolRegKey( hKey, TEXT("ConnectToRemoteServer"), m_Config.bConnectToRemoteServer );
    DXUtil_WriteIntRegKey(  hKey, TEXT("NetworkRetryDelay"), m_Config.dwNetworkRetryDelay );
    DXUtil_WriteBoolRegKey( hKey, TEXT("FileLogging"), m_Config.bFileLogging );
    DXUtil_WriteStringRegKey( hKey, TEXT("IPAddress"), m_Config.szIPAddress );

    DXUtil_WriteBoolRegKey( hKey, TEXT("ShowFramerate"), m_Config.bShowFramerate );
    DXUtil_WriteBoolRegKey( hKey, TEXT("ShowIndicators"), m_Config.bShowIndicators );
    DXUtil_WriteBoolRegKey( hKey, TEXT("DrawMiniMap"), m_Config.bDrawMiniMap );
    DXUtil_WriteBoolRegKey( hKey, TEXT("FullScreen"), m_Config.bFullScreen );
    DXUtil_WriteBoolRegKey( hKey, TEXT("Reflections"), m_Config.bReflections );

    DXUtil_WriteBoolRegKey( hKey, TEXT("AutoDisconnnect"), m_Config.bAutoDisconnnect );
    DXUtil_WriteBoolRegKey( hKey, TEXT("AutoConnnect"), m_Config.bAutoConnnect );
    DXUtil_WriteIntRegKey( hKey, TEXT("LogLevel"), m_Config.dwLogLevel );
    DXUtil_WriteIntRegKey( hKey, TEXT("AutoPrintStats"), m_Config.dwAutoPrintStats );

    RegCloseKey( hKey );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void ConsolePrintf( ServerBufferType enumLineType, const TCHAR* fmt , ... )
{
    // Format the message into a buffer
    TCHAR buffer[512];
    _vstprintf( buffer, fmt, (CHAR*) ((&fmt)+1) );

    s_pMazeApp->ConsolePrintf( LINE_LOG, buffer );
}

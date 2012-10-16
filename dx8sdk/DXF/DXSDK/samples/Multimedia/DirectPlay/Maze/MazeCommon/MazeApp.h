//----------------------------------------------------------------------------
// File: mazeapp.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MAZE_APP_H
#define _MAZE_APP_H


#include "DPlay8Client.h"
#include "MazeClient.h"
#include "IMazeGraphics.h"

class   CDPlay8Client;

//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define MAX_OUTPUT_QUEUE    256
#define MICROSOFT_SERVER    TEXT("DirectPlayMaze.rte.microsoft.com")
#define MAZE_REGKEYNAME     TEXT("Software\\Microsoft\\DirectPlayMaze\\MazeClient")

//-----------------------------------------------------------------------------
// Dplay Defines, and constants
//-----------------------------------------------------------------------------
#define MIN_SP_THREADS      1
#define MAX_SP_THREADS      128
#define MAX_SP_BUFFER       1024000

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct MazeConfig
{
    BOOL    bConnectToMicrosoftSite;
    BOOL    bConnectToLocalServer;
    BOOL    bConnectToRemoteServer;
    DWORD   dwNetworkRetryDelay;
    BOOL    bShowFramerate;
    BOOL    bShowIndicators;
    BOOL    bDrawMiniMap;
    BOOL    bFullScreen;
    BOOL    bReflections;
    BOOL    bFileLogging;

    DWORD   dwSPThreads;
    DWORD   dwSPBufferSize;

    DWORD   dwStressConnect;

    TCHAR   szIPAddress[64];

    DWORD   dwLogLevel;
    DWORD   dwAutoPrintStats;
    BOOL    bAutoDisconnnect;
    BOOL    bAutoConnnect;
};


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CMazeApp
{

public:
    CMazeApp();
    ~CMazeApp();

    HRESULT Create( IMazeGraphics* pMazeGraphics );
    int     Run( HINSTANCE hInst );
    HRESULT FrameMove( FLOAT fElapsedTime );
    void    ConsolePrintf( EnumLineType EnumLineType, const TCHAR* fmt, ... );
    void    ReadConfig();
    void    WriteConfig();
    void    CreateTempLogFile();
    void    CloseTempLogFile();

    MazeConfig* GetConfig() {return &m_Config; };

    TCHAR*  GetLogDir() { return m_strLogDir; };
    TCHAR*  GetLogFile() { return m_strLogFile; };
    BOOL    IsOutOfDateClient() { return m_bOutOfDateClient; };

    VOID    SetOutOfDateClient( BOOL bOutOfDateClient ) { m_bOutOfDateClient = bOutOfDateClient; };
    VOID    SetAllowConnect(    BOOL bAllowConnect )    { m_bAllowConnect = bAllowConnect; };
    VOID    SetSaveSettings(    BOOL bSaveSettings )    { m_bSaveSettings = bSaveSettings; };
    VOID    SetConnectNow(      BOOL bConnectNow )      { m_bConnectNow = bConnectNow; };
    VOID    SetDisconnectNow(   BOOL bDisconnectNow )   { m_bDisconnectNow = bDisconnectNow; };
    VOID    SetAllowLoopback(   BOOL bAllowLoopback )   { m_bAllowLoopback = bAllowLoopback; };

protected:
    static UINT WINAPI StaticOutputMsgThread( LPVOID pParam );
    static UINT WINAPI StaticClientThread( LPVOID pParam );
    UINT WINAPI     OutputMsgThread( LPVOID pParam );
    UINT WINAPI     ClientThread( LPVOID pParam );

    void            SuspendPowerManagement();
    BOOL            ParseCommandLine();

    HRESULT         StartSessionEnum();
    HRESULT         InitServerForLoopback();
    BOOL            TryToConnect();
    void            UpdateTimeStamp();

    IMazeGraphics*      m_pMazeGraphics;
    CDPlay8Client       m_DP8Client;
    CMazeClient         m_MazeClient;
    CMazeServer         m_MazeServer;
    CDummyConnectorServer m_DummyServerConnection;
    CDummyConnectorClient m_DummyClientConnection;

    MazeConfig          m_Config;

    HANDLE              m_hOutputMsgThread;
    BOOL                m_bQuitThread;
    HANDLE              m_hClientThread;
    DWORD               m_dwNextOutputMsg;

    BOOL                m_bLocalLoopbackInitDone;
    BOOL                m_bInitDone;

    BOOL                m_bAllowLoopback;
    BOOL                m_bLocalLoopback;
    BOOL                m_bAllowConnect;
    BOOL                m_bConnectNow;
    BOOL                m_bDisconnectNow;
    BOOL                m_bOutOfDateClient;

    TCHAR               m_strTimeStamp[50];

    BOOL                m_bSaveSettings;

    CCriticalSection    m_OutputMsgQueueLock;
    LONG                m_lQueueSize;
    TCHAR               m_szOutputMsgBuffer[MAX_OUTPUT_QUEUE][256];
    EnumLineType        m_EnumLineType[MAX_OUTPUT_QUEUE];
    DWORD               m_dwNextFreeOutputMsg;
    HANDLE              m_hOutputMsgEvent;
    HANDLE              m_hLogFile;
    TCHAR               m_strLogFile[MAX_PATH];
    TCHAR               m_strLogDir[MAX_PATH];
};


#endif

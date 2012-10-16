//----------------------------------------------------------------------------
// File: consolegraphics.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _CONSOLE_GRAPHICS_H
#define _CONSOLE_GRAPHICS_H


#define MAX_CMD_LINES 100
#define MAX_LOG_LINES 100
class CMazeApp;


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CConsoleGraphics : public IMazeGraphics
{
public:
    CConsoleGraphics();
    ~CConsoleGraphics();

    // From IMazeGraphics
    virtual VOID        Init( CMazeApp* pMazeApp, CDPlay8Client* pDP8Client, CMazeClient* pMazeClient ); 
    virtual HRESULT     Create( HINSTANCE hInstance );
    virtual INT         Run();
    virtual VOID        HandleOutputMsg( EnumLineType enumLineType, TCHAR* strLine );
    virtual BOOL        IsPreview() { return FALSE; };
    virtual void        Shutdown();

    CMazeApp*           m_pMazeApp;
    CDPlay8Client*      m_pDP8Client;
    CMazeClient*        m_pMazeClient;

protected:
    static BOOL WINAPI  CtrlHandler( DWORD type );
    WCHAR** ConvertCommandLineToArgvW( WCHAR* strCommandLine, int* argc );
    void ParseInput( TCHAR* pBuffer );
    VOID SetupConsole( DWORD dwWindowSizeY );
    VOID RestoreOldConsoleSettings();
    VOID WriteLine( DWORD nCoordY, TCHAR* strBuffer );
    void DoPrompt( TCHAR* strPromptString, TCHAR* strBuffer );
    void PrintHelp();
    void ClearScreen();
    BOOL TryToConnect();
    void RunSetupWizard();
    BOOL ParseCommandLine();

    CONSOLE_SCREEN_BUFFER_INFO m_SavedConsoleInfo;
    
	MazeConfig*          m_pConfig;
    
	BOOL   m_bQuitThread;
    DWORD  m_dwNextOutput;
    HANDLE m_hStdOut;
    DWORD  m_dwNumCmdLines;
    DWORD  m_dwSeperatorLine;
    DWORD  m_dwNumLogLines;
    DWORD  m_dwWindowSizeY;
    BOOL   m_bLocalLoopback;

    static UINT WINAPI StaticPromptThread( LPVOID pParam );
    UINT WINAPI PromptThread( LPVOID pParam );

    HANDLE m_hPromptThread;

    TCHAR m_szLogBuffer[MAX_LOG_LINES][256];
    TCHAR m_szCmdBuffer[MAX_CMD_LINES][256];
    DWORD m_dwLogBufferNext;
    DWORD m_dwCmdBufferNext;
    DWORD m_dwLastPrompt;
    int   m_dwCaretPos;
    DWORD m_dwNumProcessed;
};


#endif

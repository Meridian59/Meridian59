//-----------------------------------------------------------------------------
// File: MultiMapper.cpp
//
// Desc: This is a simple sample to demonstrate how to code using the DInput
//       action mapper feature.
//
//-----------------------------------------------------------------------------
#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <DXErr8.h>
#include <tchar.h>
#include <dinput.h>
#include "MultiDI.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define SAMPLE_KEY        TEXT("Software\\Microsoft\\DirectX SDK\\MultiMapper")

// This GUID must be unique for every game, and the same for 
// every instance of this app.  // {67131584-2938-4857-8A2E-D99DC2C82068}
// The GUID allows DirectInput to remember input settings
GUID g_guidApp = { 0x67131584, 0x2938, 0x4857, { 0x8a, 0x2e, 0xd9, 0x9d, 0xc2, 0xc8, 0x20, 0x68 } };


// DirectInput action mapper reports events only when buttons/axis change
// so we need to remember the present state of relevant axis/buttons for 
// each DirectInput device.  The CMultiplayerInputDeviceManager will store a 
// pointer for each device that points to this struct
struct InputDeviceState
{
    FLOAT fAxisMoveUD;
    BOOL  bButtonForwardThrust;
    BOOL  bButtonReverseThrust;

    FLOAT fAxisRotateLR;
    BOOL  bButtonRotateLeft;
    BOOL  bButtonRotateRight;

    BOOL  bButtonFireWeapons;
    BOOL  bButtonEnableShield;
};


// Struct to store the current input state
struct UserInput
{
    FLOAT fAxisMoveUD;
    FLOAT fAxisRotateLR;
    BOOL  bButtonFireWeapons;
    BOOL  bButtonEnableShield;

    BOOL  bDoConfigureInput;
    BOOL  bDoQuitGame;
};


// Input semantics used by this app
enum INPUT_SEMANTICS
{
    // Gameplay semantics
    INPUT_ROTATE_AXIS_LR=1,  INPUT_MOVE_AXIS_UD,       
    INPUT_FIREWEAPONS,       INPUT_ENABLESHIELD,    
    INPUT_TURNLEFT,          INPUT_TURNRIGHT, 
    INPUT_FORWARDTHRUST,     INPUT_REVERSETHRUST,
    INPUT_DISPLAYGAMEMENU,   INPUT_QUITGAME,
};

// Actions used by this app
DIACTION g_rgGameAction[] =
{
    // If DirectInput has already been given an action map for this GUID, it
    // will have created a user map for this application 
    // (C:\Program Files\DirectX\DirectInput\User Maps\*.ini). If a user map
    // exists, DirectInput will use the action map defined in the stored user 
    // map instead of the map defined in your program. This allows the user to
    // customize controls without losing changes when the game restarts. If you 
    // wish to make changes to the default action map without changing the 
    // GUID, you will need to delete the stored user map from your hard drive
    // for the system to detect your changes and recreate a stored user map.
    
    // Device input (joystick, etc.) that is pre-defined by DInput, according
    // to genre type. The genre for this app is space simulators.
    { INPUT_ROTATE_AXIS_LR,  DIAXIS_SPACESIM_LATERAL,      0, TEXT("Turn left/right"), },
    { INPUT_MOVE_AXIS_UD,    DIAXIS_SPACESIM_MOVE,         0, TEXT("Move"), },
    { INPUT_FIREWEAPONS,     DIBUTTON_SPACESIM_FIRE,       0, TEXT("Shoot"), },
    { INPUT_ENABLESHIELD,    DIBUTTON_SPACESIM_GEAR,       0, TEXT("Shield"), },
    { INPUT_DISPLAYGAMEMENU, DIBUTTON_SPACESIM_DISPLAY,    0, TEXT("Configure"), },

    // Keyboard input mappings
    { INPUT_TURNLEFT,        DIKEYBOARD_LEFT,    0, TEXT("Turn left"), },
    { INPUT_TURNRIGHT,       DIKEYBOARD_RIGHT,   0, TEXT("Turn right"), },
    { INPUT_FORWARDTHRUST,   DIKEYBOARD_UP,      0, TEXT("Move forward"), },
    { INPUT_REVERSETHRUST,   DIKEYBOARD_DOWN,    0, TEXT("Move backward"), },
    { INPUT_FIREWEAPONS,     DIKEYBOARD_F,       0, TEXT("Fire weapons"), },
    { INPUT_ENABLESHIELD,    DIKEYBOARD_S,       0, TEXT("Enable shields"), },
    { INPUT_DISPLAYGAMEMENU, DIKEYBOARD_D,       DIA_APPFIXED, TEXT("Configure"), },
    { INPUT_QUITGAME,        DIKEYBOARD_ESCAPE,  DIA_APPFIXED, TEXT("Quit game"), },

    // Mouse input mappings
    { INPUT_ROTATE_AXIS_LR,  DIMOUSE_XAXIS,      0, _T("Turn left/right"), },
    { INPUT_MOVE_AXIS_UD,    DIMOUSE_YAXIS,      0, _T("Move"), },
    { INPUT_FIREWEAPONS,     DIMOUSE_BUTTON0,    0, _T("Shoot"), },
    { INPUT_ENABLESHIELD,    DIMOUSE_BUTTON1,    0, _T("Enable shields"), },
};

#define NUMBER_OF_GAMEACTIONS    (sizeof(g_rgGameAction)/sizeof(DIACTION))




//-----------------------------------------------------------------------------
// Function prototypes 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );




//-----------------------------------------------------------------------------
// Name: class CMyApplication 
// Desc: Application class.
//-----------------------------------------------------------------------------
class CMyApplication 
{
    TCHAR*                  m_strWindowTitle;       // Title for the app's window
    HWND                    m_hWnd;                 // The main app window
    FLOAT                   m_fTime;                // Current time in seconds
    FLOAT                   m_fElapsedTime;         // Time elapsed since last frame

    CMultiplayerInputDeviceManager* m_pInputDeviceManager;  // DirectInput multiplayer device manager
    DIACTIONFORMAT          m_diafGame;             // Action format for game play
    UserInput               m_UserInput[4];         // Struct for storing user input 

    DWORD                   m_dwNumPlayers;         // Number of players in the game
    FLOAT                   m_fWorldRotX;           // World rotation state X-axis
    FLOAT                   m_fWorldRotY;           // World rotation state Y-axis

protected:
    HRESULT OneTimeSceneInit();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();

    HRESULT EnableUI();
    HRESULT RenderText();
    
    HRESULT InitInput();
    HRESULT ChangeNumPlayers( DWORD dwNumPlayers, BOOL bResetOwnership, BOOL bResetMappings );
    VOID    ConfigInput();
    void    UpdateInput();
    void    CleanupDirectInput();
    void    CleanupDeviceStateStructs();
        
public:
    HRESULT Create( HINSTANCE hInstance );
    INT     Run();
    INT_PTR MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    CMyApplication();

    HRESULT InputAddDeviceCB( CMultiplayerInputDeviceManager::PlayerInfo* pPlayerInfo, CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi );
    static HRESULT CALLBACK StaticInputAddDeviceCB( CMultiplayerInputDeviceManager::PlayerInfo* pPlayerInfo, CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi, LPVOID pParam );   
};




//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyApplication*    g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
    CMyApplication app;

    g_hInst = hInstance;

    if( FAILED( app.Create( hInstance ) ) )
        return 0;

    return app.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyApplication()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMyApplication::CMyApplication()
{
    g_pApp                      = this;
    m_hWnd                      = NULL;
    m_strWindowTitle            = TEXT( "DirectInput MultiMapper Sample" );
    m_pInputDeviceManager       = NULL;

    ZeroMemory( m_UserInput, sizeof(UserInput)*4 );
    m_fWorldRotX                = 0.0f;
    m_fWorldRotY                = 0.0f;
    m_dwNumPlayers              = 1;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Creates the window
//-----------------------------------------------------------------------------
HRESULT CMyApplication::Create( HINSTANCE hInstance )
{
    // Display the main dialog box.
    CreateDialog( hInstance, MAKEINTRESOURCE(IDD_MAIN), 
                  NULL, StaticMsgProc );
    if( NULL == m_hWnd )
        return E_FAIL;

    // Initialize the application timer
    DXUtil_Timer( TIMER_START );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Handles the message loop and calls FrameMove() and Render() when
//       idle.
//-----------------------------------------------------------------------------
INT CMyApplication::Run()
{
    MSG msg;

    // Message loop to run the app
    while( TRUE )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            // Skip WM_KEYDOWN so they aren't handled by the dialog
            if( msg.message == WM_KEYDOWN )
                continue;

            if( !IsDialogMessage( m_hWnd, &msg ) )  
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }

            if( msg.message == WM_QUIT )
            {
                DestroyWindow( m_hWnd );
                break;
            }
        }
        else
        {
            // Update the time variables
            m_fTime        = DXUtil_Timer( TIMER_GETAPPTIME );
            m_fElapsedTime = DXUtil_Timer( TIMER_GETELAPSEDTIME );

            // This app uses idle time processing for the game loop
            if( FAILED( FrameMove() ) )
                SendMessage( m_hWnd, WM_DESTROY, 0, 0 );
            if( FAILED( Render() ) ) 
                SendMessage( m_hWnd, WM_DESTROY, 0, 0 );

            Sleep( 20 );
        }
    }

    FinalCleanup();

    return (INT)msg.wParam;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyApplication::OneTimeSceneInit()
{
    HRESULT hr;
    
    // Load the icon
    HICON hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_MAIN ) );
    
    // Set the icon for this dialog.
    SendMessage( m_hWnd, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon
    
    // Initialize DirectInput
    if( FAILED( hr = InitInput() ) )
        return DXTRACE_ERR( TEXT("InitInput"), hr );
    
    HWND hNumPlayers = GetDlgItem( m_hWnd, IDC_NUM_PLAYERS_COMBO );
    SendMessage( hNumPlayers, CB_ADDSTRING, 0, (LPARAM) TEXT("1") );
    SendMessage( hNumPlayers, CB_ADDSTRING, 0, (LPARAM) TEXT("2") );
    SendMessage( hNumPlayers, CB_ADDSTRING, 0, (LPARAM) TEXT("3") );
    SendMessage( hNumPlayers, CB_ADDSTRING, 0, (LPARAM) TEXT("4") );
    SendMessage( hNumPlayers, CB_SETCURSEL, m_dwNumPlayers-1, 0 );

    EnableUI();
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitInput()
// Desc: Initialize DirectInput objects
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InitInput()
{
    HRESULT hr;

    // Setup action format for the actual gameplay
    ZeroMemory( &m_diafGame, sizeof(DIACTIONFORMAT) );
    m_diafGame.dwSize          = sizeof(DIACTIONFORMAT);
    m_diafGame.dwActionSize    = sizeof(DIACTION);
    m_diafGame.dwDataSize      = NUMBER_OF_GAMEACTIONS * sizeof(DWORD);
    m_diafGame.guidActionMap   = g_guidApp;
    m_diafGame.dwGenre         = DIVIRTUAL_SPACESIM; 
    m_diafGame.dwNumActions    = NUMBER_OF_GAMEACTIONS;
    m_diafGame.rgoAction       = g_rgGameAction;
    m_diafGame.lAxisMin        = -100;
    m_diafGame.lAxisMax        = 100;
    m_diafGame.dwBufferSize    = 16;
    _tcscpy( m_diafGame.tszActionMap, _T("MultiMapper Sample") );
    
    // Create a new input device manager
    m_pInputDeviceManager = new CMultiplayerInputDeviceManager( SAMPLE_KEY );

    if( FAILED( hr = ChangeNumPlayers( 1, FALSE, FALSE ) ) )
        return DXTRACE_ERR( TEXT("ChangeNumPlayers"), hr );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StaticInputAddDeviceCB()
// Desc: Static callback helper to call into CMyApplication class
//-----------------------------------------------------------------------------
HRESULT CALLBACK CMyApplication::StaticInputAddDeviceCB( 
                                         CMultiplayerInputDeviceManager::PlayerInfo* pPlayerInfo, 
                                         CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                         const DIDEVICEINSTANCE* pdidi, 
                                         LPVOID pParam )
{
    CMyApplication* pApp = (CMyApplication*) pParam;
    return pApp->InputAddDeviceCB( pPlayerInfo, pDeviceInfo, pdidi );
}




//-----------------------------------------------------------------------------
// Name: InputAddDeviceCB()
// Desc: Called from CMultiplayerInputDeviceManager whenever a device is added. 
//       Set the dead zone, and creates a new InputDeviceState for each device
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InputAddDeviceCB( CMultiplayerInputDeviceManager::PlayerInfo* pPlayerInfo, 
                                         CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                         const DIDEVICEINSTANCE* pdidi )
{
    if( (GET_DIDEVICE_TYPE(pdidi->dwDevType) != DI8DEVTYPE_KEYBOARD) &&
        (GET_DIDEVICE_TYPE(pdidi->dwDevType) != DI8DEVTYPE_MOUSE) )
    {
        // Setup the deadzone 
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = 500;
        pDeviceInfo->pdidDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph );
    }
    
    // Create a new InputDeviceState for each device so the 
    // app can record its state 
    InputDeviceState* pNewInputDeviceState = new InputDeviceState;
    ZeroMemory( pNewInputDeviceState, sizeof(InputDeviceState) );
    pDeviceInfo->pParam = (LPVOID) pNewInputDeviceState;
            
    HWND hPlayerDevice;
    switch( pPlayerInfo->dwPlayerIndex )
    {
    case 0:
        hPlayerDevice = GetDlgItem( m_hWnd, IDC_DEVICE_ASSIGNED_P1 );
        break;
    case 1:
        hPlayerDevice = GetDlgItem( m_hWnd, IDC_DEVICE_ASSIGNED_P2 );
        break;
    case 2:
        hPlayerDevice = GetDlgItem( m_hWnd, IDC_DEVICE_ASSIGNED_P3 );
        break;
    case 3:
        hPlayerDevice = GetDlgItem( m_hWnd, IDC_DEVICE_ASSIGNED_P4 );
        break;
    }

    SendMessage( hPlayerDevice, LB_ADDSTRING, 0, (LPARAM) pdidi->tszProductName );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ChangeNumPlayers()
// Desc: Signals a change in the number of players. It is also called to reset
//       ownership and mapping data.
//-----------------------------------------------------------------------------
HRESULT CMyApplication::ChangeNumPlayers( DWORD dwNumPlayers, BOOL bResetOwnership, 
                                          BOOL bResetMappings )
{
    HRESULT hr;
    
    m_dwNumPlayers = dwNumPlayers;

    // Just pass in stock names.  Real games may want to ask the user for a name, etc.
    TCHAR* strUserNames[4] = { TEXT("Player 1"), TEXT("Player 2"), 
                               TEXT("Player 3"), TEXT("Player 4") };

    BOOL bSuccess = FALSE;
    while( !bSuccess )
    {
        // Update UI
        EnableUI();    
        SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P1), LB_RESETCONTENT, 0, 0 );
        SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P2), LB_RESETCONTENT, 0, 0 );
        SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P3), LB_RESETCONTENT, 0, 0 );
        SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P4), LB_RESETCONTENT, 0, 0 );
        
        hr = m_pInputDeviceManager->Create( m_hWnd, strUserNames, m_dwNumPlayers, &m_diafGame, 
                                            StaticInputAddDeviceCB, this, 
                                            bResetOwnership, bResetMappings );

        if( FAILED(hr) )
        {
            switch( hr )
            {
                case E_DIUTILERR_DEVICESTAKEN:
                {
                    // It's possible that a single user could "own" too many devices for the other
                    // players to get into the game. If so, we reinit the manager class to provide 
                    // each user with a device that has a default configuration.
                    bResetOwnership = TRUE;
                    
                    MessageBox( m_hWnd, TEXT("You have entered more users than there are suitable ")       \
                                        TEXT("devices, or some users are claiming too many devices.\n") \
                                        TEXT("Press OK to give each user a default device"), 
                                        TEXT("Devices Are Taken"), MB_OK | MB_ICONEXCLAMATION );
                    break;
                }
                    
                case E_DIUTILERR_TOOMANYUSERS:
                {
                    // Another common error is if more users are attempting to play than there are devices
                    // attached to the machine. In this case, the number of players is automatically 
                    // lowered to make playing the game possible. 
                    DWORD dwNumDevices = m_pInputDeviceManager->GetNumDevices();
                    m_dwNumPlayers = dwNumDevices;                    
                    SendMessage( GetDlgItem( m_hWnd, IDC_NUM_PLAYERS_COMBO ), 
                                 CB_SETCURSEL, m_dwNumPlayers-1, 0 );
                                
                    TCHAR sz[256];
                    wsprintf( sz, TEXT("There are not enough devices attached to the ")          \
                                  TEXT("system for the number of users you entered.\nThe ")      \
                                  TEXT("number of users has been automatically changed to ")     \
                                  TEXT("%i (the number of devices available on the system)."),
                                  m_dwNumPlayers );
                    MessageBox( m_hWnd, sz, _T("Too Many Users"), MB_OK | MB_ICONEXCLAMATION );                    
                    break;
                }

                default:
                    return DXTRACE_ERR( TEXT("m_pInputDeviceManager->Create"), hr );
                    break;
            }
            
            m_pInputDeviceManager->Cleanup();                                
        }
        else
        {
            bSuccess = TRUE;
        }
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyApplication::FrameMove()
{
    // Update user input state
    UpdateInput();
    
    for( DWORD iPlayer=0; iPlayer<m_dwNumPlayers; iPlayer++ )
    {       
        // Respond to input
        if( m_UserInput[iPlayer].bDoConfigureInput )
        {
            // One-shot per keypress
            m_UserInput[iPlayer].bDoConfigureInput = FALSE;

            ConfigInput();
        }
    
        // Update the world state according to user input
        if( m_UserInput[iPlayer].fAxisMoveUD )
            m_fWorldRotY += m_fElapsedTime * m_UserInput[iPlayer].fAxisMoveUD;
    
        if( m_UserInput[iPlayer].fAxisRotateLR )
            m_fWorldRotX += m_fElapsedTime * m_UserInput[iPlayer].fAxisRotateLR;
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: Update the user input.  Called once per frame 
//-----------------------------------------------------------------------------
void CMyApplication::UpdateInput()
{
    if( NULL == m_pInputDeviceManager )
        return;
    
    CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfos;
    DWORD dwNumDevices;

    // Get access to the list of semantically-mapped input devices
    m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

    // Loop through all devices and check game input
    for( DWORD i=0; i<dwNumDevices; i++ )
    {
        // skip past any devices that aren't assigned, 
        // since we don't care about them
        if( pDeviceInfos[i].pPlayerInfo == NULL )
            continue;

        DIDEVICEOBJECTDATA rgdod[10];
        DWORD   dwItems = 10;
        HRESULT hr;
        LPDIRECTINPUTDEVICE8 pdidDevice = pDeviceInfos[i].pdidDevice;
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
        FLOAT fScale = 1.0f;

        if( pDeviceInfos[i].bRelativeAxis )
        {
            // For relative axis data, the action mapper only informs us when
            // the delta data is non-zero, so we need to zero its state
            // out each frame
            pInputDeviceState->fAxisMoveUD   = 0.0f;
            pInputDeviceState->fAxisRotateLR = 0.0f;      

            // Scale the relative axis data to make it more equal to 
            // absolute joystick data
            fScale = 5.0f;
        }

        hr = pdidDevice->Acquire();
        hr = pdidDevice->Poll();
        hr = pdidDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                        rgdod, &dwItems, 0 );
        if( FAILED(hr) )
            continue;

        // Get the sematics codes for the game menu
        for( DWORD j=0; j<dwItems; j++ )
        {
            BOOL  bButtonState = (rgdod[j].dwData==0x80) ? TRUE : FALSE;
            FLOAT fButtonState = (rgdod[j].dwData==0x80) ? 1.0f : 0.0f;
            FLOAT fAxisState   = (FLOAT)((int)rgdod[j].dwData)/100.0f * fScale;

            switch( rgdod[j].uAppData )
            {
                // Handle relative axis data
                case INPUT_ROTATE_AXIS_LR: 
                    pInputDeviceState->fAxisRotateLR = fAxisState;
                    break;
                case INPUT_MOVE_AXIS_UD:
                    pInputDeviceState->fAxisMoveUD = -fAxisState;
                    break;

                // Handle buttons separately so the button state data
                // doesn't overwrite the axis state data, and handle
                // each button separately so they don't overwrite each other
                case INPUT_TURNLEFT:        pInputDeviceState->bButtonRotateLeft    = bButtonState; break;
                case INPUT_TURNRIGHT:       pInputDeviceState->bButtonRotateRight   = bButtonState; break;
                case INPUT_FORWARDTHRUST:   pInputDeviceState->bButtonForwardThrust = bButtonState; break;
                case INPUT_REVERSETHRUST:   pInputDeviceState->bButtonReverseThrust = bButtonState; break;
                case INPUT_FIREWEAPONS:     pInputDeviceState->bButtonFireWeapons   = bButtonState; break;
                case INPUT_ENABLESHIELD:    pInputDeviceState->bButtonEnableShield  = bButtonState; break;

                // Handle one-shot buttons
                case INPUT_DISPLAYGAMEMENU: if( bButtonState ) m_UserInput[0].bDoConfigureInput = TRUE; break;
                case INPUT_QUITGAME:        if( bButtonState ) m_UserInput[0].bDoQuitGame       = TRUE; break;
            }
        }
    }

    for( DWORD iPlayer=0; iPlayer<m_dwNumPlayers; iPlayer++ )
    {       
        // Process user input and store result into pUserInput struct
        m_UserInput[iPlayer].fAxisRotateLR = 0.0f;
        m_UserInput[iPlayer].fAxisMoveUD   = 0.0f;
        m_UserInput[iPlayer].bButtonFireWeapons  = FALSE;
        m_UserInput[iPlayer].bButtonEnableShield = FALSE;

        // Concatinate the data from all the DirectInput devices
        for( i=0; i<dwNumDevices; i++ )
        {
            // Only look at devices that are assigned to this player 
            if( pDeviceInfos[i].pPlayerInfo == NULL || 
                pDeviceInfos[i].pPlayerInfo->dwPlayerIndex != iPlayer )
                continue;

            InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;

            // Use the axis data that is furthest from zero
            if( fabs(pInputDeviceState->fAxisRotateLR) > fabs(m_UserInput[iPlayer].fAxisRotateLR) )
                m_UserInput[iPlayer].fAxisRotateLR = pInputDeviceState->fAxisRotateLR;

            if( fabs(pInputDeviceState->fAxisMoveUD) > fabs(m_UserInput[iPlayer].fAxisMoveUD) )
                m_UserInput[iPlayer].fAxisMoveUD = pInputDeviceState->fAxisMoveUD;

            // Process the button data 
            if( pInputDeviceState->bButtonRotateLeft )
                m_UserInput[iPlayer].fAxisRotateLR = -1.0f;
            else if( pInputDeviceState->bButtonRotateRight )
                m_UserInput[iPlayer].fAxisRotateLR = 1.0f;

            if( pInputDeviceState->bButtonForwardThrust )
                m_UserInput[iPlayer].fAxisMoveUD = 1.0f;
            else if( pInputDeviceState->bButtonReverseThrust )
                m_UserInput[iPlayer].fAxisMoveUD = -1.0f;

            if( pInputDeviceState->bButtonFireWeapons )
                m_UserInput[iPlayer].bButtonFireWeapons = TRUE;
            if( pInputDeviceState->bButtonEnableShield )
                m_UserInput[iPlayer].bButtonEnableShield = TRUE;
        } 
    }
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for rendering the 
//       world.
//-----------------------------------------------------------------------------
HRESULT CMyApplication::Render()
{
    TCHAR szMsg[MAX_PATH];
    TCHAR szMsgCurrent[MAX_PATH];

    // Update Player 1
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[0].fAxisMoveUD );
    GetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P1), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P1), szMsg );
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[0].fAxisRotateLR );
    GetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P1), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P1), szMsg );
    if( !m_UserInput[0].bButtonEnableShield && !m_UserInput[0].bButtonFireWeapons )
        _stprintf( szMsg, TEXT("None") );
    else
        _stprintf( szMsg, TEXT("%s%s"), m_UserInput[0].bButtonEnableShield ? TEXT("Shield ") : TEXT(""), 
                                      m_UserInput[0].bButtonFireWeapons ? TEXT("Fire ") : TEXT("") );
    GetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P1), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P1), szMsg );

    // Update Player 2
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[1].fAxisMoveUD );
    GetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P2), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P2), szMsg );
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[1].fAxisRotateLR );
    GetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P2), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P2), szMsg );
    if( !m_UserInput[1].bButtonEnableShield && !m_UserInput[1].bButtonFireWeapons )
        _stprintf( szMsg, TEXT("None") );
    else
        _stprintf( szMsg, TEXT("%s%s"), m_UserInput[1].bButtonEnableShield ? TEXT("Shield ") : TEXT(""), 
        m_UserInput[1].bButtonFireWeapons ? TEXT("Fire ") : TEXT("") );
    GetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P2), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P2), szMsg );
    
    // Update Player 3
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[2].fAxisMoveUD );
    GetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P3), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P3), szMsg );
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[2].fAxisRotateLR );
    GetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P3), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P3), szMsg );
    if( !m_UserInput[2].bButtonEnableShield && !m_UserInput[2].bButtonFireWeapons )
        _stprintf( szMsg, TEXT("None") );
    else
        _stprintf( szMsg, TEXT("%s%s"), m_UserInput[2].bButtonEnableShield ? TEXT("Shield ") : TEXT(""), 
        m_UserInput[2].bButtonFireWeapons ? TEXT("Fire ") : TEXT("") );
    GetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P3), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P3), szMsg );
    
    // Update Player 4
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[3].fAxisMoveUD );
    GetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P4), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P4), szMsg );
    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput[3].fAxisRotateLR );
    GetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P4), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P4), szMsg );
    if( !m_UserInput[3].bButtonEnableShield && !m_UserInput[3].bButtonFireWeapons )
        _stprintf( szMsg, TEXT("None") );
    else
        _stprintf( szMsg, TEXT("%s%s"), m_UserInput[3].bButtonEnableShield ? TEXT("Shield ") : TEXT(""), 
        m_UserInput[3].bButtonFireWeapons ? TEXT("Fire ") : TEXT("") );
    GetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P4), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P4), szMsg );      
    
    // Update world state
    _stprintf( szMsg, TEXT("%0.3f, %0.3f"), m_fWorldRotX, m_fWorldRotY );
    GetWindowText( GetDlgItem(m_hWnd,IDC_WORLD_STATE), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_WORLD_STATE), szMsg );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StaticMsgProc()
// Desc: Static msg handler which passes messages to the application class.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK StaticMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return g_pApp->MsgProc( hWnd, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
INT_PTR CMyApplication::MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg )
    {
        case WM_INITDIALOG:
            m_hWnd = hWnd;

            // Initialize the app's custom scene stuff
            if( FAILED( hr = OneTimeSceneInit() ) )
            {
                DXTRACE_ERR( TEXT("OneTimeSceneInit"), hr );
                MessageBox( hWnd, TEXT("Error initializing DirectInput.  Sample will now exit."), 
                            TEXT("DirectInput Sample"), MB_OK | MB_ICONERROR );
                PostQuitMessage( IDCANCEL );
                return TRUE;
            }
            break;
            
        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
            case IDC_NUM_PLAYERS_COMBO:
                {
                    if( HIWORD(wParam) == CBN_SELCHANGE )
                    {
                        TCHAR strItem[MAX_PATH];
                        HWND hNumPlayers = GetDlgItem( m_hWnd, IDC_NUM_PLAYERS_COMBO );
                    
                        int nCurSel = (int) SendMessage( hNumPlayers, CB_GETCURSEL, 0, 0 );
                        SendMessage( hNumPlayers, CB_GETLBTEXT, nCurSel, (LPARAM) strItem );
                        DWORD dwNumPlayers = _ttoi( strItem );

                        if( FAILED( hr = ChangeNumPlayers( dwNumPlayers, FALSE, FALSE ) ) )
                        {
                            PostQuitMessage( IDCANCEL );                            
                        }
                    }
                    break;
                }

                case IDC_RESET_OWNERSHIP:
                    ChangeNumPlayers( m_dwNumPlayers, TRUE, FALSE );
                    break;
                
                case IDC_RESET_MAPPINGS:
                    ChangeNumPlayers( m_dwNumPlayers, FALSE, TRUE );
                    break;
                
                case IDCANCEL:
                    PostQuitMessage( IDCANCEL );
                    break;

                case IDM_CONFIGINPUT:
                    m_UserInput[0].bDoConfigureInput = TRUE;
                    break;
            }
            break;
        }
    }
    
    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyApplication::FinalCleanup()
{
    // Cleanup DirectInput
    CleanupDirectInput();
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CleanupDirectInput()
// Desc: Cleanup DirectInput 
//-----------------------------------------------------------------------------
VOID CMyApplication::CleanupDirectInput()
{
    if( NULL == m_pInputDeviceManager )
        return;
    
    CleanupDeviceStateStructs();
    
    // Cleanup DirectX input objects
    SAFE_DELETE( m_pInputDeviceManager );
}




//-----------------------------------------------------------------------------
// Name: CleanupDeviceStateStructs()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMyApplication::CleanupDeviceStateStructs()
{
    // Get access to the list of semantically-mapped input devices
    // to delete all InputDeviceState structs before calling ConfigureDevices()
    CMultiplayerInputDeviceManager::DeviceInfo* pDeviceInfos;
    DWORD dwNumDevices;
    
    m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

    for( DWORD i=0; i<dwNumDevices; i++ )
    {
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
        SAFE_DELETE( pInputDeviceState );
        pDeviceInfos[i].pParam = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: ConfigInput()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMyApplication::ConfigInput()
{
    HRESULT hr;
    
    EnableWindow( m_hWnd, FALSE );

    CleanupDeviceStateStructs();
    
    // Update UI
    EnableUI();    
    SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P1), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P2), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P3), LB_RESETCONTENT, 0, 0 );
    SendMessage( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P4), LB_RESETCONTENT, 0, 0 );
    
    // Configure the devices (with edit capability)
    hr = m_pInputDeviceManager->ConfigureDevices( m_hWnd, NULL, NULL, DICD_EDIT, NULL );    
    if( FAILED(hr) )
    {
        if( hr == E_DIUTILERR_PLAYERWITHOUTDEVICE )
        {
            // There's a player that hasn't been assigned a device.  Some games may
            // want to handle this by reducing the number of players, or auto-assigning
            // a device, or warning the user, etc.
            MessageBox( m_hWnd, TEXT("There is at least one player that wasn't assigned ") \
                                TEXT("a device\n") \
                                TEXT("Press OK to auto-assign a device to these users"), 
                                TEXT("Player Without Device"), MB_OK | MB_ICONEXCLAMATION );
        }

        // Auto-reassign every player a device.
        ChangeNumPlayers( m_dwNumPlayers, FALSE, FALSE );
    }

    EnableWindow( m_hWnd, TRUE );
    SetForegroundWindow( m_hWnd );
}




//-----------------------------------------------------------------------------
// Name: EnableUI()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyApplication::EnableUI()
{
    BOOL bEnablePlayer4 = ( m_dwNumPlayers > 3 );
    BOOL bEnablePlayer3 = ( m_dwNumPlayers > 2 );
    BOOL bEnablePlayer2 = ( m_dwNumPlayers > 1 );

    // Enable/disable the player 2 ui
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT1_P2), bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT2_P2), bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT3_P2), bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_PLAYER2_GROUP),    bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P2), bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P2), bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P2),  bEnablePlayer2 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P2),  bEnablePlayer2 );
    if( !bEnablePlayer2 )
    {
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P2), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P2), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P2),  TEXT("n/a") );
    }
    
    // Enable/disable the player 3 ui
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT1_P3), bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT2_P3), bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT3_P3), bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_PLAYER3_GROUP),    bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P3), bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P3), bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P3),  bEnablePlayer3 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P3),  bEnablePlayer3 );
    if( !bEnablePlayer4 )
    {
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P3), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P3), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P3),  TEXT("n/a") );
    }
    
    // Enable/disable the player 4 ui
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT1_P4), bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT2_P4), bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_TEXT3_P4), bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_PLAYER4_GROUP),    bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P4), bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P4), bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P4),  bEnablePlayer4 );
    EnableWindow( GetDlgItem(m_hWnd,IDC_DEVICE_ASSIGNED_P4), bEnablePlayer4 );
    if( !bEnablePlayer4 )
    {
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE_P4), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE_P4), TEXT("n/a") );
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE_P4),  TEXT("n/a") );
    }
    
    return S_OK;
}






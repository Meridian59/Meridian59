//-----------------------------------------------------------------------------
// File: ActionMapper.cpp
//
// Desc: This is a simple sample to demonstrate how to code using the DInput
//       action mapper feature.
//
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <DXErr8.h>
#include <tchar.h>
#include <dinput.h>
#include "DIUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
// This GUID must be unique for every game, and the same for 
// every instance of this app.  // {67131584-2938-4857-8A2E-D99DC2C82068}
// The GUID allows DirectInput to remember input settings
GUID g_guidApp = { 0x67131584, 0x2938, 0x4857, { 0x8a, 0x2e, 0xd9, 0x9d, 0xc2, 0xc8, 0x20, 0x68 } };


// DirectInput action mapper reports events only when buttons/axis change
// so we need to remember the present state of relevant axis/buttons for 
// each DirectInput device.  The CInputDeviceManager will store a 
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
    // (C:\Program Files\DirectX\DirectInput\User Maps\*.ini)
    // after changing this, otherwise settings won't reset and will be read 
    // from the out of date ini files 

    // Device input (joystick, etc.) that is pre-defined by DInput, according
    // to genre type. The genre for this app is space simulators.
    { INPUT_ROTATE_AXIS_LR,  DIAXIS_SPACESIM_LATERAL,      0, TEXT("Rotate left/right"), },
    { INPUT_MOVE_AXIS_UD,    DIAXIS_SPACESIM_MOVE,         0, TEXT("Move"), },
    { INPUT_FIREWEAPONS,     DIBUTTON_SPACESIM_FIRE,       0, TEXT("Fire weapons"), },
    { INPUT_ENABLESHIELD,    DIBUTTON_SPACESIM_GEAR,       0, TEXT("Enable shield"), },
    { INPUT_DISPLAYGAMEMENU, DIBUTTON_SPACESIM_DISPLAY,    0, TEXT("Configure"), },

    // Keyboard input mappings
    { INPUT_TURNLEFT,        DIKEYBOARD_LEFT,    0, TEXT("Turn left"), },
    { INPUT_TURNRIGHT,       DIKEYBOARD_RIGHT,   0, TEXT("Turn right"), },
    { INPUT_FORWARDTHRUST,   DIKEYBOARD_UP,      0, TEXT("Forward thrust"), },
    { INPUT_REVERSETHRUST,   DIKEYBOARD_DOWN,    0, TEXT("Reverse thrust"), },
    { INPUT_FIREWEAPONS,     DIKEYBOARD_F,       0, TEXT("Fire weapons"), },
    { INPUT_ENABLESHIELD,    DIKEYBOARD_S,       0, TEXT("Enable shield"), },
    { INPUT_DISPLAYGAMEMENU, DIKEYBOARD_D,       DIA_APPFIXED, TEXT("Configure"), },
    { INPUT_QUITGAME,        DIKEYBOARD_ESCAPE,  DIA_APPFIXED, TEXT("Quit game"), },
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

    CInputDeviceManager*    m_pInputDeviceManager;  // DirectInput device manager
    DIACTIONFORMAT          m_diafGame;             // Action format for game play
    UserInput               m_UserInput;            // Struct for storing user input 

    FLOAT                   m_fWorldRotX;           // World rotation state X-axis
    FLOAT                   m_fWorldRotY;           // World rotation state Y-axis

protected:
    HRESULT OneTimeSceneInit();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    HRESULT InitInput( HWND hWnd );
    void    UpdateInput( UserInput* pUserInput );
    void    CleanupDirectInput();

public:
    HRESULT Create( HINSTANCE hInstance );
    INT     Run();
    INT_PTR MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    CMyApplication();

    HRESULT InputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi );
    static HRESULT CALLBACK StaticInputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi, LPVOID pParam );   
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
    m_strWindowTitle            = TEXT( "DirectInput ActionMapper Sample" );
    m_pInputDeviceManager       = NULL;

    ZeroMemory( &m_UserInput, sizeof(m_UserInput) );
    m_fWorldRotX                = 0.0f;
    m_fWorldRotY                = 0.0f;
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
    // Initialize DirectInput
    InitInput( m_hWnd );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StaticInputAddDeviceCB()
// Desc: Static callback helper to call into CMyApplication class
//-----------------------------------------------------------------------------
HRESULT CALLBACK CMyApplication::StaticInputAddDeviceCB( 
                                         CInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                         const DIDEVICEINSTANCE* pdidi, 
                                         LPVOID pParam )
{
    CMyApplication* pApp = (CMyApplication*) pParam;
    return pApp->InputAddDeviceCB( pDeviceInfo, pdidi );
}




//-----------------------------------------------------------------------------
// Name: InputAddDeviceCB()
// Desc: Called from CInputDeviceManager whenever a device is added. 
//       Set the dead zone, and creates a new InputDeviceState for each device
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, 
                                                   const DIDEVICEINSTANCE* pdidi )
{
    // Setup the deadzone 
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = 500;
    pDeviceInfo->pdidDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

    // Create a new InputDeviceState for each device so the 
    // app can record its state 
    InputDeviceState* pNewInputDeviceState = new InputDeviceState;
    if (!pNewInputDeviceState)
        return E_FAIL;
    
    ZeroMemory( pNewInputDeviceState, sizeof(InputDeviceState) );
    pDeviceInfo->pParam = (LPVOID) pNewInputDeviceState;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitInput()
// Desc: Initialize DirectInput objects
//-----------------------------------------------------------------------------
HRESULT CMyApplication::InitInput( HWND hWnd )
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
    _tcscpy( m_diafGame.tszActionMap, _T("ActionMapper Sample") );

    // Create a new input device manager
    m_pInputDeviceManager = new CInputDeviceManager();

    if( FAILED( hr = m_pInputDeviceManager->Create( hWnd, NULL, m_diafGame, 
                                                    StaticInputAddDeviceCB, this ) ) )
        return DXTRACE_ERR_NOMSGBOX( TEXT("m_pInputDeviceManager->Create"), hr );

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
    UpdateInput( &m_UserInput );

    // Respond to input
    if( m_UserInput.bDoConfigureInput )
    {
        // One-shot per keypress
        m_UserInput.bDoConfigureInput = FALSE;

        // Get access to the list of semantically-mapped input devices
        // to delete all InputDeviceState structs before calling ConfigureDevices()
        CInputDeviceManager::DeviceInfo* pDeviceInfos;
        DWORD dwNumDevices;
        m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

        for( DWORD i=0; i<dwNumDevices; i++ )
        {
            InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
            SAFE_DELETE( pInputDeviceState );
            pDeviceInfos[i].pParam = NULL;
        }

        // Configure the devices (with edit capability)
        m_pInputDeviceManager->ConfigureDevices( m_hWnd, NULL, NULL, DICD_EDIT, NULL );
    }

    // Update the world state according to user input
    if( m_UserInput.fAxisRotateLR )
        m_fWorldRotY += m_fElapsedTime * m_UserInput.fAxisRotateLR;

    if( m_UserInput.fAxisMoveUD )
        m_fWorldRotX += m_fElapsedTime * m_UserInput.fAxisMoveUD;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: Update the user input.  Called once per frame 
//-----------------------------------------------------------------------------
void CMyApplication::UpdateInput( UserInput* pUserInput )
{
    if( NULL == m_pInputDeviceManager )
        return;

    // Get access to the list of semantically-mapped input devices
    CInputDeviceManager::DeviceInfo* pDeviceInfos;
    DWORD dwNumDevices;
    m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

    // Loop through all devices and check game input
    for( DWORD i=0; i<dwNumDevices; i++ )
    {
        DIDEVICEOBJECTDATA rgdod[10];
        DWORD   dwItems = 10;
        HRESULT hr;
        LPDIRECTINPUTDEVICE8 pdidDevice = pDeviceInfos[i].pdidDevice;
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;

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
            FLOAT fAxisState   = (FLOAT)((int)rgdod[j].dwData)/100.0f;

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
                case INPUT_DISPLAYGAMEMENU: if( bButtonState ) pUserInput->bDoConfigureInput = TRUE; break;
                case INPUT_QUITGAME:        if( bButtonState ) pUserInput->bDoQuitGame       = TRUE; break;
            }
        }
    }

    // Process user input and store result into pUserInput struct
    pUserInput->fAxisRotateLR = 0.0f;
    pUserInput->fAxisMoveUD   = 0.0f;
    pUserInput->bButtonFireWeapons  = FALSE;
    pUserInput->bButtonEnableShield = FALSE;

    // Concatinate the data from all the DirectInput devices
    for( i=0; i<dwNumDevices; i++ )
    {
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;

        // Use the axis data that is furthest from zero
        if( fabs(pInputDeviceState->fAxisRotateLR) > fabs(pUserInput->fAxisRotateLR) )
            pUserInput->fAxisRotateLR = pInputDeviceState->fAxisRotateLR;

        if( fabs(pInputDeviceState->fAxisMoveUD) > fabs(pUserInput->fAxisMoveUD) )
            pUserInput->fAxisMoveUD = pInputDeviceState->fAxisMoveUD;

        // Process the button data 
        if( pInputDeviceState->bButtonRotateLeft )
            pUserInput->fAxisRotateLR = -1.0f;
        else if( pInputDeviceState->bButtonRotateRight )
            pUserInput->fAxisRotateLR = 1.0f;

        if( pInputDeviceState->bButtonForwardThrust )
            pUserInput->fAxisMoveUD = 1.0f;
        else if( pInputDeviceState->bButtonReverseThrust )
            pUserInput->fAxisMoveUD = -1.0f;

        if( pInputDeviceState->bButtonFireWeapons )
            pUserInput->bButtonFireWeapons = TRUE;
        if( pInputDeviceState->bButtonEnableShield )
            pUserInput->bButtonEnableShield = TRUE;
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

    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput.fAxisMoveUD );
    GetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_UD_AXIS_STATE), szMsg );

    _stprintf( szMsg, TEXT("%0.2f"), m_UserInput.fAxisRotateLR );
    GetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_LR_AXIS_STATE), szMsg );

    if( !m_UserInput.bButtonEnableShield && !m_UserInput.bButtonFireWeapons )
    {
        _stprintf( szMsg, TEXT("None") );
    }
    else
    {
        _stprintf( szMsg, TEXT("%s%s"), m_UserInput.bButtonEnableShield ? TEXT("Shield ") : TEXT(""), 
                                      m_UserInput.bButtonFireWeapons ? TEXT("Fire ") : TEXT("") );
    }

    GetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE), szMsgCurrent, MAX_PATH );
    if( lstrcmp( szMsgCurrent, szMsg ) != 0 )
        SetWindowText( GetDlgItem(m_hWnd,IDC_BUTTON_STATE), szMsg );

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
                case IDCANCEL:
                    PostQuitMessage( IDCANCEL );
                    break;

                case IDM_CONFIGINPUT:
                    m_UserInput.bDoConfigureInput = TRUE;
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

    // Get access to the list of semantically-mapped input devices
    // to delete all InputDeviceState structs
    CInputDeviceManager::DeviceInfo* pDeviceInfos;
    DWORD dwNumDevices;
    m_pInputDeviceManager->GetDevices( &pDeviceInfos, &dwNumDevices );

    for( DWORD i=0; i<dwNumDevices; i++ )
    {
        InputDeviceState* pInputDeviceState = (InputDeviceState*) pDeviceInfos[i].pParam;
        SAFE_DELETE( pInputDeviceState );
        pDeviceInfos[i].pParam = NULL;
    }

    // Cleanup DirectX input objects
    SAFE_DELETE( m_pInputDeviceManager );
}





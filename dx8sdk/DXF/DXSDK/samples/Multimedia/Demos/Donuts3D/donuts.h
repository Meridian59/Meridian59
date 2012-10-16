//-----------------------------------------------------------------------------
// File: Donuts.h
//
// Desc: Header for Donuts3D game
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef DONUTS_H
#define DONUTS_H




//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
// This GUID must be unique for every game, and the same for 
// every instance of this app.  // {2014BAAC-F1FD-458e-9091-0F85C8CF6AFE}
// The GUID allows DirectInput to remember input settings
GUID g_guidApp = { 0x2014baac, 0xf1fd, 0x458e, { 0x90, 0x91, 0xf, 0x85, 0xc8, 0xcf, 0x6a, 0xfe } };

// Error codes
#define DONUTS3DERR_NODIRECT3D       0x00000001
#define DONUTS3DERR_NOD3DDEVICE      0x00000002
#define DONUTS3DERR_NOTEXTURES       0x00000003
#define DONUTS3DERR_NOGEOMETRY       0x00000004
#define DONUTS3DERR_NO3DRESOURCES    0x00000005
#define DONUTS3DERR_NOINPUT          0x00000006

// States the app can be in
enum{ APPSTATE_LOADSPLASH, APPSTATE_DISPLAYSPLASH, APPSTATE_ACTIVE, 
      APPSTATE_BEGINLEVELSCREEN, APPSTATE_DISPLAYLEVELSCREEN };

// Game object types
enum{ OBJ_DONUT=0, OBJ_PYRAMID, OBJ_CUBE, OBJ_SPHERE, OBJ_CLOUD, OBJ_SHIP,
      OBJ_BULLET };

// Object dimensions and fixed properties
#define DONUT_WIDTH        32
#define DONUT_HEIGHT       32
#define PYRAMID_WIDTH      32
#define PYRAMID_HEIGHT     32
#define SPHERE_WIDTH       16
#define SPHERE_HEIGHT      16
#define CUBE_WIDTH         16
#define CUBE_HEIGHT        16
#define CLOUD_WIDTH        32
#define CLOUD_HEIGHT       32
#define BULLET_WIDTH        3
#define BULLET_HEIGHT       3

#define NUM_DONUT_FRAMES   30
#define NUM_PYRAMID_FRAMES 40
#define NUM_SPHERE_FRAMES  40
#define NUM_CUBE_FRAMES    40
#define NUM_BULLET_FRAMES 400

#define BULLET_XOFFSET    304
#define BULLET_YOFFSET      0

// Defines for the in-game menu
#define MENU_MAIN           1
#define MENU_SOUND          2
#define MENU_VIDEO          3
#define MENU_INPUT          4
#define MENU_VIEWDEVICES    5
#define MENU_CONFIGDEVICES  6
#define MENU_WINDOWED       7
#define MENU_640x480        8
#define MENU_800x600        9
#define MENU_1024x768      10
#define MENU_BACK          11
#define MENU_SOUNDON       12
#define MENU_SOUNDOFF      13
#define MENU_QUIT          14


TCHAR* g_strShipFiles[] = { _T("Concept Plane 3.x"), _T("Spaceship 2.x"), _T("Shusui.x"),
                             _T("Space Station 7.x"), _T("Spaceship 8.x"), _T("Orbiter.x"),
                             _T("Spaceship 13.x"),    _T("Spaceship 5.x"), _T("Star Sail.x"), 
                             _T("Heli.x"), };
TCHAR* g_strShipNames[] = { _T("Concept Plane"), _T("Green Machine"),  _T("Purple Prowler"),
                             _T("Drone Clone"),   _T("Canyon Fighter"), _T("Roundabout"),
                             _T("Tie-X7"),        _T("Gunner"),         _T("Star Sail"), 
                             _T("Helicopter"), };



//-----------------------------------------------------------------------------
// Name: struct DisplayObject
// Desc: A game object that goes in the display list
//-----------------------------------------------------------------------------
struct DisplayObject
{
    DisplayObject* pNext;          // Link to next object
    DisplayObject* pPrev;          // Link to previous object
    
    DWORD          dwType;            // Type of object
    BOOL           bVisible;          // Whether the object is visible
    D3DXVECTOR3    vPos;              // Position
    D3DXVECTOR3    vVel;              // Velocity
    FLOAT          fSize;
    
    // Constructor
    DisplayObject( DWORD type, D3DVECTOR p, D3DVECTOR v );
};


//-----------------------------------------------------------------------------
// Derived classes for displayable game objects
//-----------------------------------------------------------------------------
struct C3DSprite : public DisplayObject
{
    DWORD dwFramesPerLine;   // How anim frames are packed in bitmap
    FLOAT frame;             // Current animation frame
    FLOAT fMaxFrame;         // Max animation frame value
    FLOAT delay;             // Frame/second
    
    DWORD dwColor;

    DWORD dwTextureOffsetX; // Pixel offsets into the game texture
    DWORD dwTextureOffsetY;
    DWORD dwTextureWidth;   // Width and height in pixels
    DWORD dwTextureHeight; 
    
    C3DSprite( DWORD type, D3DVECTOR p, D3DVECTOR v );
};


class CDonut : public C3DSprite
{
public:
    CDonut( D3DVECTOR p, D3DVECTOR v );
};


class CPyramid : public C3DSprite
{
public:
    CPyramid( D3DVECTOR p, D3DVECTOR v );
};


class CSphere : public C3DSprite
{
public:
    CSphere( D3DVECTOR p, D3DVECTOR v );
};


class CCube : public C3DSprite
{
public:
    CCube( D3DVECTOR p, D3DVECTOR v );
};


class CCloud : public C3DSprite
{
public:
    CCloud( D3DVECTOR p, D3DVECTOR v );
};


class CBullet : public C3DSprite
{
public:
    CBullet( D3DVECTOR p, D3DVECTOR v, DWORD dwType );
};


class CShip : public DisplayObject
{
public:
    FLOAT fRoll;

    FLOAT fAngle;

    BOOL  bExploded;
    FLOAT fShowDelay;

public:
    CShip( D3DVECTOR p );
};



//-----------------------------------------------------------------------------
// Custom Direct3D vertex types
//-----------------------------------------------------------------------------
struct SCREENVERTEX
{
    D3DXVECTOR4 p;
    DWORD       color;
};

struct SPRITEVERTEX
{
    D3DXVECTOR3 p;
    DWORD       color;
    FLOAT       tu, tv;
};

struct MODELVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;
};

#define D3DFVF_SCREENVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_SPRITEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_MODELVERTEX  (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


//-----------------------------------------------------------------------------
// App defined structures
//-----------------------------------------------------------------------------

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

    // Menu input variables
    FLOAT fAxisMenuUD;
};

// Struct to store the current input state
struct UserInput
{
    FLOAT fAxisMoveUD;
    FLOAT fAxisRotateLR;
    BOOL  bButtonFireWeapons;
    BOOL  bButtonEnableShield;

    // One-shot variables
    BOOL  bDoChangeView;    

    // Menu input variables
    BOOL  bDoMenuUp;
    BOOL  bDoMenuDown;
    BOOL  bDoMenuSelect;
    BOOL  bDoMenuQuit;

};


//-----------------------------------------------------------------------------
// Game actions (using DInput semantic mapper). The definitions here are kind
// of the whole point of this sample. The game uses these actions to map
// physical input like, "the user pressed the 'W' key", to a more useable
// constant for the game, like "if( dwInput == INPUT_CHANGEWEAPONS )...".
//-----------------------------------------------------------------------------

// Input semantics used by this game
enum INPUT_SEMANTICS
{
    // Gameplay semantics
    INPUT_AXIS_LR=1,     INPUT_AXIS_UD,       
    INPUT_MOUSE_LR,      INPUT_MOUSE_UD,      INPUT_MOUSE_SHIPTYPE,
    INPUT_TURNLEFT,      INPUT_TURNRIGHT,     INPUT_FORWARDTHRUST,
    INPUT_REVERSETHRUST, INPUT_FIREWEAPONS,   INPUT_CHANGESHIPTYPE,
    INPUT_CHANGEVIEW,    INPUT_CHANGEWEAPONS, INPUT_DISPLAYGAMEMENU,
    INPUT_QUITGAME,      INPUT_START,

    // Menu semantics
    INPUT_MENU_UD,       INPUT_MENU_WHEEL,
    INPUT_MENU_UP,       INPUT_MENU_DOWN,     
    INPUT_MENU_SELECT,   INPUT_MENU_QUIT,
};

// Game actions used by this game.
DIACTION g_rgGameAction[] =
{
    // (C:\Program Files\DirectX\DirectInput\User Maps\*.ini)
    // after changing this, otherwise settings won't reset and will be read 
    // from the out of date ini files 

    // Device input (joystick, etc.) that is pre-defined by DInput, according
    // to genre type. The genre for this app is space simulators.
    { INPUT_AXIS_LR,         DIAXIS_SPACESIM_LATERAL,   0, TEXT("Turn"), },
    { INPUT_AXIS_UD,         DIAXIS_SPACESIM_MOVE,      0, TEXT("Move"), },
    { INPUT_FIREWEAPONS,     DIBUTTON_SPACESIM_FIRE,    0, TEXT("Fire weapons"), },
    { INPUT_CHANGEVIEW,      DIBUTTON_SPACESIM_VIEW,    0, TEXT("Change view"), },
    { INPUT_CHANGEWEAPONS,   DIBUTTON_SPACESIM_WEAPONS, 0, TEXT("Change weapons"), },
    { INPUT_CHANGESHIPTYPE,  DIBUTTON_SPACESIM_LOWER,    0, TEXT("Change ship type"), },
    { INPUT_DISPLAYGAMEMENU, DIBUTTON_SPACESIM_DEVICE,    0, TEXT("Display game menu"), },
    { INPUT_START,           DIBUTTON_SPACESIM_MENU,  0, TEXT("Start/pause"), },

    // Keyboard input mappings
    { INPUT_TURNLEFT,        DIKEYBOARD_LEFT,    0, TEXT("Turn left"), },
    { INPUT_TURNRIGHT,       DIKEYBOARD_RIGHT,   0, TEXT("Turn right"), },
    { INPUT_FORWARDTHRUST,   DIKEYBOARD_UP,      0, TEXT("Forward thrust"), },
    { INPUT_REVERSETHRUST,   DIKEYBOARD_DOWN,    0, TEXT("Reverse thrust"), },
    { INPUT_FIREWEAPONS,     DIKEYBOARD_SPACE,   0, TEXT("Fire weapons"), },
    { INPUT_CHANGESHIPTYPE,  DIKEYBOARD_A,       0, TEXT("Change ship type"), },
    { INPUT_CHANGEVIEW,      DIKEYBOARD_V,       0, TEXT("Change view"), },
    { INPUT_CHANGEWEAPONS,   DIKEYBOARD_W,       0, TEXT("Change weapons"), },
    { INPUT_DISPLAYGAMEMENU, DIKEYBOARD_F1,      DIA_APPFIXED, TEXT("Display game menu"), },
    { INPUT_START,           DIKEYBOARD_PAUSE,   0, TEXT("Start/pause"), },
    { INPUT_QUITGAME,        DIKEYBOARD_ESCAPE,  DIA_APPFIXED, TEXT("Quit game"), },

    // Mouse input mappings
    { INPUT_MOUSE_LR,        DIMOUSE_XAXIS,      0, TEXT("Turn"), },
    { INPUT_MOUSE_UD,        DIMOUSE_YAXIS,      0, TEXT("Move"), },
    { INPUT_MOUSE_SHIPTYPE,  DIMOUSE_WHEEL,      0, TEXT("Change ship type"), },
    { INPUT_FIREWEAPONS,     DIMOUSE_BUTTON0,    0, TEXT("Fire weapons"), },
    { INPUT_CHANGEWEAPONS,   DIMOUSE_BUTTON1,    0, TEXT("Change weapons"), },
};

// Game actions used by this game.
DIACTION g_rgBrowserAction[] =
{
    // (C:\Program Files\DirectX\DirectInput\User Maps\*.ini)
    // after changing this, otherwise settings won't reset and will be read 
    // from the out of date ini files 

    // Device input (joystick, etc.) that is pre-defined by DInput, according
    // to genre type. The genre for this app is space simulators.
    { INPUT_MENU_UD,         DIAXIS_BROWSER_MOVE,       0, TEXT("Up/down"), },
    { INPUT_MENU_UP,         DIBUTTON_BROWSER_PREVIOUS, 0, TEXT("Up"), },
    { INPUT_MENU_DOWN,       DIBUTTON_BROWSER_NEXT,     0, TEXT("Down"), },
    { INPUT_MENU_SELECT,     DIBUTTON_BROWSER_SELECT,   0, TEXT("Select"), },
    { INPUT_MENU_QUIT,       DIBUTTON_BROWSER_DEVICE,   0, TEXT("Quit menu"), },

    // Keyboard input mappings
    { INPUT_MENU_UP,         DIKEYBOARD_UP,          0, TEXT("Up"), },
    { INPUT_MENU_DOWN,       DIKEYBOARD_DOWN,        0, TEXT("Down"), },
    { INPUT_MENU_SELECT,     DIKEYBOARD_SPACE,       0, TEXT("Select"), },
    { INPUT_MENU_SELECT,     DIKEYBOARD_RETURN,      0, TEXT("Select"), },
    { INPUT_MENU_SELECT,     DIKEYBOARD_NUMPADENTER, 0, TEXT("Select"), },
    { INPUT_MENU_QUIT,       DIKEYBOARD_ESCAPE,      0, TEXT("Quit menu"), },

    // Mouse input mappings
    { INPUT_MENU_WHEEL,      DIMOUSE_WHEEL,      0, TEXT("Up/down"), },
    { INPUT_MENU_SELECT,     DIMOUSE_BUTTON0,    0, TEXT("Select"), },
};

// Number of actions
#define NUMBER_OF_GAMEACTIONS    (sizeof(g_rgGameAction)/sizeof(DIACTION))
#define NUMBER_OF_BROWSERACTIONS (sizeof(g_rgBrowserAction)/sizeof(DIACTION))


//-----------------------------------------------------------------------------
// Inline helper functions
//-----------------------------------------------------------------------------

// Simple function to define "hilliness" for terrain
inline FLOAT HeightField( FLOAT x, FLOAT z )
{
    return (cosf(x/2.0f+0.2f)*cosf(z/1.5f-0.2f)+1.0f) - 2.0f;
}

// Simple function for generating random numbers
inline FLOAT rnd( FLOAT low, FLOAT high )
{
    return low + ( high - low ) * ( (FLOAT)rand() ) / RAND_MAX;
}

FLOAT rnd( FLOAT low=-1.0f, FLOAT high=1.0f );

// Convenient macros for playing sounds
inline VOID PlaySound( CMusicSegment* pSound )
{
        if( pSound ) pSound->Play( DMUS_SEGF_SECONDARY );
}

inline VOID StopSound( CMusicSegment* pSound )
{
        if( pSound ) pSound->Stop();
}


//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );



//-----------------------------------------------------------------------------
// Name: class CMyApplication 
// Desc: Application class.
//-----------------------------------------------------------------------------
class CMyApplication 
{
public:
    HRESULT Create( HINSTANCE hInstance );
    INT     Run();
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    CMyApplication();

    HRESULT InputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi );
    static HRESULT CALLBACK StaticInputAddDeviceCB( CInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi, LPVOID pParam );   
    BOOL    ConfigureInputDevicesCB( IUnknown* pUnknown );
    static BOOL CALLBACK StaticConfigureInputDevicesCB( IUnknown* pUnknown, VOID* pUserData );

protected:
    HRESULT          OneTimeSceneInit( HWND hWnd );
    HRESULT          FrameMove();
    HRESULT          RenderFrame();
    VOID             FinalCleanup();

    // Sound functions
    HRESULT          CreateSoundObjects( HWND hWnd );
    VOID             DestroySoundObjects();

    // Input functions
    HRESULT          CreateInputObjects( HWND hWnd );
    VOID             DestroyInputObjects();
    void             UpdateInput( UserInput* pUserInput );

    // Display functions
    HRESULT          CreateDisplayObjects( HWND hWnd );
    HRESULT          RestoreDisplayObjects();
    HRESULT          InvalidateDisplayObjects();
    HRESULT          DestroyDisplayObjects();
    HRESULT          SwitchDisplayModes( BOOL bFullScreen, DWORD dwWidth, DWORD dwHeight );

    // Menu functions
    VOID             ConstructMenus();
    VOID             DestroyMenus();
    VOID             UpdateMenus();

    // Rendering functions
    VOID             UpdateDisplayList();
    VOID             DrawDisplayList();
    VOID             ShowFrame();
    VOID             DarkenScene( FLOAT fAmount );
    VOID             RenderFieryText( CD3DFont* pFont, TCHAR* strText );

    // Misc game functions
    VOID             DisplayLevelIntroScreen( DWORD dwLevel );
    VOID             AdvanceLevel();
    BOOL             IsDisplayListEmpty();
    VOID             AddToList( DisplayObject* pObject );
    VOID             DeleteFromList( DisplayObject* pObject );
    VOID             CheckForHits();

    HRESULT          LoadTerrainModel();
    HRESULT          LoadShipModel();
    HRESULT          SwitchModel();

    // Error handling
    VOID             CleanupAndDisplayError( DWORD dwError );

protected:
    TCHAR*               m_strAppName;
    HWND                 m_hWndMain;                // Main window
    DWORD                m_dwScreenWidth;           // Dimensions for fullscreen modes
    DWORD                m_dwScreenHeight;
    D3DDISPLAYMODE       m_DesktopMode;
    D3DFORMAT            m_d3dfmtFullscreen;        // Pixel format for fullscreen modes
    D3DFORMAT            m_d3dfmtTexture;           // Pixel format for textures
    BOOL                 m_bFullScreen;             // Whether app is fullscreen (or windowed)
    BOOL                 m_bIsActive;               // Whether app is active
    BOOL                 m_bDisplayReady;           // Whether display class is initialized
    BOOL                 m_bMouseVisible;           // Whether mouse is visible
    HBITMAP              m_hSplashBitmap;           // Bitmap for splash screen

    DWORD                m_dwAppState;              // Current state the app is in
    DWORD                m_dwLevel;                 // Current game level
    DWORD                m_dwScore;                 // Current game score

    // Player view mode
    #define NUMVIEWMODES 3
    CD3DCamera           m_Camera;                  // Camera used for 3D scene
    DWORD                m_dwViewMode;              // Which view mode is being used
    FLOAT                m_fViewTransition;         // Amount used to transittion views
    BOOL                 m_bAnimatingViewChange;    // Whether view is transitioning
    BOOL                 m_bFirstPersonView;        // Whether view is first-person

    // Bullet mode
    FLOAT                m_fBulletRechargeTime;     // Recharge time for firing bullets
    DWORD                m_dwBulletType;            // Current bullet type

    // Display list and player ship
    DisplayObject*       m_pDisplayList;            // Global display list
    CShip*               m_pShip;                   // Player's display object

    // DirectDraw/Direct3D objects
    LPDIRECT3DDEVICE8       m_pd3dDevice;           // Class to handle D3D device
    D3DPRESENT_PARAMETERS   m_d3dpp;
    LPDIRECT3DSURFACE8      m_pConfigSurface;       // Surface for config'ing DInput devices
    LPDIRECT3DVERTEXBUFFER8 m_pViewportVB;
    LPDIRECT3DVERTEXBUFFER8 m_pSpriteVB;

    // Support for the ship model
    CD3DMesh*            m_pShipFileObject;         // Geometry model of player's ship
    DWORD                m_dwNumShipTypes;
    DWORD                m_dwCurrentShipType;

    // DirectMusic objects
    CMusicManager*       m_pMusicManager;           // Class to manage DMusic objects
    CMusicSegment*       m_pBeginLevelSound;        // Sounds for the app
    CMusicSegment*       m_pEngineIdleSound;
    CMusicSegment*       m_pEngineRevSound;
    CMusicSegment*       m_pShieldBuzzSound;
    CMusicSegment*       m_pShipExplodeSound;
    CMusicSegment*       m_pFireBulletSound;
    CMusicSegment*       m_pShipBounceSound;
    CMusicSegment*       m_pDonutExplodeSound;
    CMusicSegment*       m_pPyramidExplodeSound;
    CMusicSegment*       m_pCubeExplodeSound;
    CMusicSegment*       m_pSphereExplodeSound;

    // Game objects
    LPDIRECT3DTEXTURE8   m_pGameTexture1;           // Texture with game object animations
    LPDIRECT3DTEXTURE8   m_pGameTexture2;           // Texture with game object animations
    CD3DMesh*            m_pTerrain;                // Geometry model of terrain
    CD3DFont*            m_pGameFont;               // Font for displaying score, etc.
    CD3DFont*            m_pMenuFont;               // Font for displaying in-game menus


    // Menu objects
    CMenuItem*           m_pMainMenu;               // Menu class for in-game menus
    CMenuItem*           m_pQuitMenu;
    CMenuItem*           m_pCurrentMenu;

    // DirectInput objects
    CInputDeviceManager* m_pInputDeviceManager;     // Class for managing DInput devices
    UserInput            m_UserInput;               // Struct for storing user input 
    DIACTIONFORMAT       m_diafGame;                // Action format for game play
    DIACTIONFORMAT       m_diafBrowser;             // Action format for menu navigation

    BOOL                 m_bPaused;    
};


#endif


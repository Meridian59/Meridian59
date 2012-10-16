//------------------------------------------------------------------------------
// File: DDrawXcl.h
//
// Desc: DirectShow sample code - DDraw Exclusive Mode Video Playback 
//       test/sample application header file.
//
// Copyright (c) 1993-2001 Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------


//
// App menu ids
//
#define IDM_SELECTDVD        101
#define IDM_SELECTFILE       102
#define IDM_ABOUT            103
#define IDM_EXIT             104

#define IDM_STARTPLAY        111

#define IDC_STATIC           -1

//
// Version info related constant ids
//
#define DLG_VERFIRST        400
#define IDC_COMPANY         DLG_VERFIRST
#define IDC_FILEDESC        DLG_VERFIRST+1
#define IDC_PRODVER         DLG_VERFIRST+2
#define IDC_COPYRIGHT       DLG_VERFIRST+3
#define IDC_OSVERSION       DLG_VERFIRST+4
#define IDC_TRADEMARK       DLG_VERFIRST+5
#define DLG_VERLAST         DLG_VERFIRST+5

#define IDC_LABEL           DLG_VERLAST+1

//
// Message displayed on failure to start playback
//
#define STR_EXCLUSIVE_MODE_FAILURE                                                          \
    TEXT("Exclusive mode playback failed to start.")                                        \
    TEXT("\nPlease verify that the selected file can be played with your video card.")      \
    TEXT("\n\nMore information:")                                                           \
    TEXT("\n----------------------")                                                        \
    TEXT("\nSome display cards do not support RGB overlays in hardware, but the decoder\n") \
    TEXT("used by the selected media file may require them.  If the filter upstream\n")     \
    TEXT("of the overlay mixer proposes only RGB formats, and the video card does not\n")   \
    TEXT("support RGB overlays in hardware, then the filter graph manager will not\n")      \
    TEXT("be able to connect the filters to complete the graph.")


//
// App string resource ids
//
#define IDS_APP_TITLE       500
#define IDS_APP_NAME        501
#define IDS_WINDOW_TITLE    502
#define IDS_VER_INFO_LANG   503
#define IDS_VERSION_ERROR   504


//
// Some constants
//
#define TIMER_ID        1
#define TIMER_RATE      100

//
// Some enumerated types
//
typedef enum {
    Unspecified = 0, DVD , File
} VIDEO_TYPE ;


//
// Function signatures for Windows
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                     LPSTR lpCmdLine, int nCmdShow) ;
BOOL    InitApplication(void) ;
BOOL    InitInstance(int nCmdShow) ;
LRESULT CALLBACK MenuProc(HWND hWnd, WPARAM wParam, LPARAM lParam) ;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT CALLBACK KeyProc(HWND hWnd, WPARAM wParam, LPARAM lParam) ;
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT StartPlay(HWND hWndApp) ;
LRESULT OnPlaybackEvent(HWND hWnd, WPARAM wParam, LPARAM lParam) ;
void    OnEndOfPlayback(HWND hWnd) ;
void    SetVideoPosition(BOOL bSetBallPosition) ;
BOOL    FileSelect(HWND hWnd, VIDEO_TYPE eType) ;
BOOL    CreatePlayer(VIDEO_TYPE eType) ;
BOOL    IsVideoTypeKnown(void) ;
VIDEO_TYPE  GetVideoType(void) ;


//
//  Global variables used by the (Windows) app
//
HWND                ghWndApp ;
TCHAR               gszAppName[10] ;
TCHAR               gszAppTitle[100] ;
TCHAR               gszDirection[100] ;
HINSTANCE           ghInstance ;

CBaseVideoPlayer   *gpPlayer ;
CDDrawObject       *gpDDrawObj ;
UINT                guTimerID ;
VIDEO_TYPE          geVideoType ;
BOOL                gbAppActive ;

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* client.c
*
*/

#include <assert.h>
#include <crtdbg.h>

#include "client.h"

HWND hMain = NULL;             /* Main window */
HINSTANCE hInst = NULL;           /* Program's instance */

HWND hCurrentDlg = NULL;       /* Current modeless dialog, if any */

static HANDLE hStdout;         /* Debugging for NT */
static FILE *debug_file = NULL;

char *szAppName;

/************************************************************************/
void _cdecl dprintf(char *fmt,...)
{
	char s[1024];
	va_list marker;
	DWORD written;

	va_start(marker,fmt);
	vsprintf(s,fmt,marker);
	va_end(marker);

	assert(strlen(s)<1024);	/* overflowed local stack buffer, increase sizeof s buffer */

	_RPT1(_CRT_WARN,"dprintf() says : %s",s);


	if (!config.debug)
		return;

	WriteFile(hStdout, s, strlen(s), &written, NULL);
  if (debug_file != NULL)
    fputs(s, debug_file);
}

unsigned short gCRC16=0;
extern WORD GetCRC16(char *buf, int length);

static unsigned short crc16( char *name)
{
	FILE*fp;
	char*buffer;
	unsigned long size;

	unsigned short retval;

	fp=fopen(name,"rb");
	if( fp == NULL) {
		return 98;
	}

	fseek(fp,0,SEEK_END);

	size = ftell( fp );

	fseek(fp,0,SEEK_SET);

	if( size == 0 ) {
		fclose( fp );
		return 99;
	}

	buffer =(char*) malloc(size);
	if(buffer==NULL) {
		fclose( fp );
		return 97;
	}

	fread(buffer,size,1,fp);

	fclose( fp );

	retval = GetCRC16(buffer,size);

	free( buffer );

	return retval ;
}

static void GenerateCRC16( void )
{
	gCRC16 = crc16( __argv[0] );
}

/************************************************************************/
long CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* See if module wants to handle message */
	if (ModuleEvent(EVENT_WINDOWMSG, hwnd, message, wParam, lParam) == False)
		return 0;

	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, MainInit);
		HANDLE_MSG(hwnd, WM_PAINT, MainExpose);
		HANDLE_MSG(hwnd, WM_DESTROY, MainQuit);

		HANDLE_MSG(hwnd, WM_KEYDOWN, MainKey);
		HANDLE_MSG(hwnd, WM_KEYUP, MainKey);


	case WM_SYSKEYDOWN:
		if (HANDLE_WM_SYSKEYDOWN_BLAK(hwnd, wParam, lParam, MainKey) == True)
			return 0;
		break;  // Pass message on to Windows for default menu handling

	case WM_SYSKEYUP:
		if (HANDLE_WM_SYSKEYDOWN_BLAK(hwnd, wParam, lParam, MainKey) == True)
			return 0;
		break;  // Pass message on to Windows for default menu handling

		HANDLE_MSG(hwnd, WM_CHAR, MainChar);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, MainMouseLButtonDown);
		HANDLE_MSG(hwnd, WM_RBUTTONDOWN, MainMouseRButtonDown);
		HANDLE_MSG(hwnd, WM_MBUTTONDOWN, MainMouseMButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, MainMouseLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, MainMouseLButtonUp);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, MainMouseMove);
		HANDLE_MSG(hwnd, WM_VSCROLL,MainVScroll);

		HANDLE_MSG(hwnd, WM_TIMER, MainTimer);
		HANDLE_MSG(hwnd, WM_COMMAND, MainCommand);

		HANDLE_MSG(hwnd, WM_SIZE, MainResize);
		HANDLE_MSG(hwnd, WM_GETMINMAXINFO, MainMinMaxInfo);
		HANDLE_MSG(hwnd, WM_MOVE, MainMove);
		HANDLE_MSG(hwnd, WM_SETFOCUS, MainSetFocus);
		HANDLE_MSG(hwnd, WM_KILLFOCUS, MainKillFocus);
		HANDLE_MSG(hwnd, WM_MENUSELECT, MainMenuSelect);
		HANDLE_MSG(hwnd, WM_ENTERIDLE, MainEnterIdle);
		HANDLE_MSG(hwnd, WM_NOTIFY, MainNotify);
		HANDLE_MSG(hwnd, WM_ACTIVATEAPP, MainActivateApp);
		HANDLE_MSG(hwnd, WM_PALETTECHANGED, MainPaletteChanged);
		HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, MainQueryNewPalette);
		HANDLE_MSG(hwnd, WM_INITMENUPOPUP, InitMenuPopupHandler);

	case WM_MEASUREITEM:
		ItemListMeasureItem(hwnd, (MEASUREITEMSTRUCT *) lParam);
		return 0;
	case WM_DRAWITEM:     // windowsx.h macro always returns FALSE
		return MainDrawItem(hwnd, (const DRAWITEMSTRUCT *)(lParam));

	case WM_SETCURSOR:
		if (HANDLE_WM_SETCURSOR(hwnd, wParam, lParam, MainSetCursor))
			return TRUE; // avoid DefWindowProc if we handled it
		break;

		HANDLE_MSG(hwnd, WM_CTLCOLOREDIT, MainCtlColor);
		HANDLE_MSG(hwnd, WM_CTLCOLORLISTBOX, MainCtlColor);
		HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, MainCtlColor);
		HANDLE_MSG(hwnd, WM_CTLCOLORSCROLLBAR, MainCtlColor);

	case BK_SOCKETEVENT:
		MainReadSocket(hwnd, WSAGETSELECTEVENT(lParam), (SOCKET) wParam, WSAGETSELECTERROR(lParam));
		return 0;

	case BK_NORESOURCE:
		MissingResource();
		break;

	case BK_MODULEUNLOAD:
		ModuleUnloadById(lParam);
		break;
	}

	return DefWindowProc (hwnd, message, wParam, lParam);
}
/************************************************************************/
void CreateDebugWindow(void)
{
	/* Make console for debugging info */
  AllocConsole();
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  debug_file = fopen("debug.txt", "w");
}
/************************************************************************/
void RegisterWindowClasses(void)
{
	WNDCLASS wndclass;
	WNDCLASSEX wndclassex;

	// Load app name
	szAppName = strdup(GetString(hInst, IDS_APPNAME));

	/* Main window class */
	wndclassex.cbSize        = sizeof(WNDCLASSEX);
	wndclassex.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wndclassex.lpfnWndProc   = WndProc;
	wndclassex.cbClsExtra    = 0;
	wndclassex.cbWndExtra    = 0;
	wndclassex.hInstance     = hInst;
	wndclassex.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wndclassex.hIconSm       = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
	wndclassex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclassex.hbrBackground = NULL;
	wndclassex.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wndclassex.lpszClassName = szAppName;
	RegisterClassEx(&wndclassex);

	/* Class for "offer" modeless dialogs */
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc   = DefDlgProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = DLGWINDOWEXTRA;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OFFER));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "Offer";
	RegisterClass (&wndclass);

	GraphCtlRegister(hInst);

	HookInit(hInst);
}
/************************************************************************/
void UnregisterWindowClasses(void)
{
	GraphCtlUnregister(hInst);

	SafeFree(szAppName);
}
/***********************************************************************/
/*
* ClearMessageQueue:  Process outstanding Windows messages.  Use when
*   a time-consuming operation may be blocking messages.
*   This procedure should be called as infrequently as possible; it is
*   preferable to have all messages go through the main window loop.
*/
void ClearMessageQueue(void)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
/************************************************************************/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam,
				   int nCmdShow)
{
	MSG msg;
	WINDOWPLACEMENT w;
	BOOL bQuit = FALSE;

	InitCommonControls();

	hInst = hInstance;

	// Find location of configuration file
	ConfigInit();

	/* Register our custom classes */
	RegisterWindowClasses();

	hMain = CreateWindow(szAppName,       /* window class name */
		szAppName,               /* window caption */
		WS_OVERLAPPEDWINDOW,     /* window style */
		0, 0, 0, 0,              /* initially zero size; changed below */
		NULL,                    /* parent window handle */
		NULL,                    /* window menu handle */
		hInstance,               /* program instance handle */
		NULL);	   	         /* creation parameters */

	if (!hMain)
	{
		char buf[256];
		DWORD err = GetLastError();
		sprintf(buf, "Error - Couldn't Create Client Window : %d", err);
		MessageBox(NULL, buf, "ERROR!", MB_OK);
		MainQuit(hMain);
		exit(1);
	}

	if (config.debug)
		CreateDebugWindow();

	if (lpszCmdParam && strlen(lpszCmdParam) > 0)
		ConfigOverride(lpszCmdParam);

	w.length = sizeof(WINDOWPLACEMENT);
	WindowSettingsLoad(&w);
	SetWindowPlacement(hMain, &w);

	D3DRenderInit(hMain);

	AudioInit();

	ModulesInit();   // Set up data structures for modules


/* attempt make a crc16 on the meridian.exe */
	GenerateCRC16();

	MainInitState(STATE_OFFLINE);

	UpdateWindow(hMain);

	while (!bQuit)
	{
		MainIdle();

		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
			{
				bQuit = TRUE;
				break;
			}

			// Forward appropriate messages for tooltips
			if (state == STATE_GAME)
				TooltipForwardMessage(&msg);

			/* Handle modeless dialog messages separately */
			if ((hCurrentDlg == NULL || !IsDialogMessage(hCurrentDlg, &msg)))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	/* Unregister our custom classes--not good to leave them around */
	UnregisterWindowClasses();

	AudioShutdown();

	return msg.wParam;  // Return value of PostQuitMessage
}
/******************************************************************************/
/*
* GetGamePath:  Get pathname to game directory (including trailing \\ ) from registry and return it -- AJM.
*				 Defaults to current directory if not found.  Does not currently set registry.
*/
void GetGamePath( char *szGamePath )
{
  if (!GetWorkingDirectory(szGamePath, MAX_PATH))
    debug(("Unable to get current directory!\n"));
}

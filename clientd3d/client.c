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

#include "client.h"
#include <dwmapi.h>

/* DWMWA_BORDER_COLOR (34) is available on Windows 11 22H2+.
   Older SDKs may not define it. */
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_COLOR_DEFAULT
#define DWMWA_COLOR_DEFAULT 0xFFFFFFFF
#endif

#ifdef M59_RETAIL
  // Minidump reporting
  #include "bugsplat.h"
#endif


HWND hMain = NULL;             /* Main window */
HINSTANCE hInst = NULL;           /* Program's instance */

HWND hCurrentDlg = NULL;       /* Current modeless dialog, if any */

static HANDLE hStdout;         /* Debugging for NT */
static FILE *debug_file = NULL;

char *szAppName;

/************************************************************************/
void _cdecl dprintf(const char *fmt, ...)
{
	const int bufferSize = 256;
	const char s[bufferSize]{ 0 };
	va_list marker;
	DWORD written;

	va_start(marker,fmt);
	vsnprintf((char*)s, bufferSize, fmt, marker);
	va_end(marker);

	_RPT1(_CRT_WARN,"dprintf() says : %s",s);

	if (!config.debug)
		return;

	WriteFile(hStdout, s, (int) strlen(s), &written, NULL);
	if (debug_file != NULL)
		fputs(s, debug_file);
}

unsigned short gCRC16=0;

static unsigned short crc16(const char *name)
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
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* See if module wants to handle message */
	if (ModuleEvent(EVENT_WINDOWMSG, hwnd, message, wParam, lParam) == false)
		return 0;

	switch (message)
	{
		case WM_MOUSEWHEEL:
		{
				if (state == STATE_GAME)
				{
						int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
						int direction = (zDelta > 0) ? 1 : -1; // Scroll up is positive, scroll down is negative
						MapZoom(direction);
						return 0;
				}
				break;
		}

		HANDLE_MSG(hwnd, WM_CREATE, MainInit);
		HANDLE_MSG(hwnd, WM_PAINT, MainExpose);
		HANDLE_MSG(hwnd, WM_DESTROY, MainQuit);

		HANDLE_MSG(hwnd, WM_KEYDOWN, MainKey);
		HANDLE_MSG(hwnd, WM_KEYUP, MainKey);


	case WM_SYSKEYDOWN:
		if (HANDLE_WM_SYSKEYDOWN_BLAK(hwnd, wParam, lParam, MainKey) == true)
			return 0;
		break;  // Pass message on to Windows for default menu handling

	case WM_SYSKEYUP:
		if (HANDLE_WM_SYSKEYDOWN_BLAK(hwnd, wParam, lParam, MainKey) == true)
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

	case WM_NCPAINT:
	{
		LRESULT res = DefWindowProc(hwnd, message, wParam, lParam);

		/* In dark mode, Windows draws bright 1px separator lines at
		   the top and bottom edges of the menu bar. Paint over them
		   with the dark background color. */
		if (config.theme != THEME_DEFAULT && GetMenu(hwnd))
		{
			RECT rcWindow, rcClient;
			GetWindowRect(hwnd, &rcWindow);
			GetClientRect(hwnd, &rcClient);

			POINT ptClient = { 0, 0 };
			ClientToScreen(hwnd, &ptClient);

			int lineLeft = ptClient.x - rcWindow.left;
			int lineRight = lineLeft + (rcClient.right - rcClient.left);

			HDC hdc = GetWindowDC(hwnd);
			if (hdc)
			{
				HBRUSH hBr = CreateSolidBrush(GetColor(COLOR_BGD));

				/* Bottom separator (between menu bar and client area) */
				int bottomY = ptClient.y - rcWindow.top - 1;
				RECT rcBottom = { lineLeft, bottomY, lineRight, bottomY + 1 };
				FillRect(hdc, &rcBottom, hBr);

				/* Top separator (between title bar and menu bar) */
				MENUBARINFO mbi = { sizeof(mbi) };
				if (GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi))
				{
					int topY = mbi.rcBar.top - rcWindow.top;
					RECT rcTop = { lineLeft, topY - 1, lineRight, topY };
					FillRect(hdc, &rcTop, hBr);
				}

				DeleteObject(hBr);
				ReleaseDC(hwnd, hdc);
			}
		}
		return res;
	}

	case WM_NCACTIVATE:
	{
		LRESULT res = DefWindowProc(hwnd, message, wParam, lParam);
		if (config.theme != THEME_DEFAULT)
			SendMessage(hwnd, WM_NCPAINT, (WPARAM)1, 0);
		return res;
	}

	case WM_MEASUREITEM:
		if (DarkMenuBar_MeasureItem((MEASUREITEMSTRUCT *)lParam))
			return TRUE;
		ItemListMeasureItem(hwnd, (MEASUREITEMSTRUCT *) lParam);
		return 0;
	case WM_DRAWITEM:     // windowsx.h macro always returns FALSE
		if (DarkMenuBar_DrawItem((DRAWITEMSTRUCT *)lParam))
			return TRUE;
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

	case BK_NEWSOUND:
		NewMusic(wParam, lParam);
		break;

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
void SetUpCrashReporting() {
#ifdef M59_RETAIL
  static MiniDmpSender *pSender;
  static const int VERSION_SIZE = 20;
  wchar_t version[VERSION_SIZE];
  _snwprintf(version, VERSION_SIZE, L"%d", VERSION_NUMBER(MAJOR_REV, MINOR_REV));
  pSender = new MiniDmpSender(L"Meridian59", L"Meridian 59", version,
                              NULL, MDSF_PREVENTHIJACKING | MDSF_LOGFILE | MDSF_LOG_VERBOSE);
	SetGlobalCRTExceptionBehavior();
	SetPerThreadCRTExceptionBehavior();    
#endif  
}
/************************************************************************/
/*
 * ThemeApply:  Apply the current theme at runtime. Swaps colors, toggles
 *   dark title bar and menu chrome, updates scrollbar themes, reloads
 *   background bitmaps, and forces a full UI redraw.
 */
void ThemeApply(void)
{
   bool dark = (config.theme == THEME_DARK);

   /* Toggle dark title bar and window border color */
   BOOL useDarkMode = dark ? TRUE : FALSE;
   DwmSetWindowAttribute(hMain, DWMWA_USE_IMMERSIVE_DARK_MODE,
      &useDarkMode, sizeof(useDarkMode));

   COLORREF borderColor = dark ? GetColor(COLOR_BGD) : DWMWA_COLOR_DEFAULT;
   DwmSetWindowAttribute(hMain, DWMWA_BORDER_COLOR,
      &borderColor, sizeof(borderColor));

   /* Toggle dark scrollbar on the main text area */
   HWND hwndText = EditBoxWindow();
   if (hwndText)
      SetWindowTheme(hwndText, dark ? L"DarkMode_Explorer" : NULL, NULL);

   /* Remove owner-drawn menu state before destroying color brushes */
   DarkMenuBar_Destroy();

   /* Swap color palette */
   ColorsDestroy();
   ColorsCreate(false);

   /* Reload background bitmap */
   CreateWindowBackground();

   /* Reapply owner-drawn menu bar for non-classic themes */
   DarkMenuBar_Apply(GetMenu(hMain));
   DrawMenuBar(hMain);

   /* Invalidate the DLL-side dark mode cache BEFORE triggering
      any resize or repaint.  The module DLL caches the theme value
      from the INI file; writing the new value and then immediately
      firing EVENT_COLORCHANGED ensures that every subsequent call
      to IsNonClassicTheme()/IsDarkMode() in the resize chain
      returns the correct result. */
   char theme_str[12];
   snprintf(theme_str, sizeof(theme_str), "%d", config.theme);
   WritePrivateProfileString("Interface", "Theme", theme_str, ini_file);
   ModuleEvent(EVENT_COLORCHANGED, -1, 0);

   /* Force full UI redraw.  MainChangeColor triggers InterfaceResize
      which recalculates all layout dimensions (stat bar widths, icon
      sizes, etc.) using the now-correct theme cache. */
   MainChangeColor();
   EditBoxRetheme();
   TextInputRetheme();
}
/************************************************************************/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam,
				   int nCmdShow)
{
	MSG msg;
	WINDOWPLACEMENT w;
	BOOL bQuit = FALSE;

	InitCommonControls();
  SetUpCrashReporting();

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
		snprintf(buf, sizeof(buf), "Error - Couldn't Create Client Window : %d", err);
		MessageBox(NULL, buf, "ERROR!", MB_OK);
		MainQuit(hMain);
		exit(1);
	}

	if (config.theme == THEME_DARK)
	{
		BOOL useDarkMode = TRUE;
		DwmSetWindowAttribute(hMain, DWMWA_USE_IMMERSIVE_DARK_MODE,
			&useDarkMode, sizeof(useDarkMode));

		COLORREF borderColor = GetColor(COLOR_BGD);
		DwmSetWindowAttribute(hMain, DWMWA_BORDER_COLOR,
			&borderColor, sizeof(borderColor));
	}

	DarkMenuBar_Apply(GetMenu(hMain));

	if (config.debug)
		CreateDebugWindow();

  ConfigOverride(lpszCmdParam);

	w.length = sizeof(WINDOWPLACEMENT);
	WindowSettingsLoad(&w);
	int showCmd = w.showCmd;

	if (showCmd == SW_SHOWMAXIMIZED || showCmd == SW_MAXIMIZE)
	{
		// Maximize now so splash screen is positioned correctly.
		// WM_SETREDRAW prevents visual flash during resize.
		SendMessage(hMain, WM_SETREDRAW, FALSE, 0);
		ShowWindow(hMain, SW_SHOWMAXIMIZED);
		ShowWindow(hMain, SW_HIDE);
		SendMessage(hMain, WM_SETREDRAW, TRUE, 0);
	}
	else
	{
		w.showCmd = SW_HIDE;
		SetWindowPlacement(hMain, &w);
	}

	D3DRenderInit(hMain);

	ModulesInit();   // Set up data structures for modules


/* attempt make a crc16 on the meridian.exe */
	GenerateCRC16();

	MainInitState(STATE_OFFLINE);

	ShowWindow(hMain, showCmd);
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

			/* WM_MOUSEWHEEL is sent to the focused window, but we want
			   scroll-under-cursor behavior.  Redirect the message to the
			   window under the cursor and let DefWindowProc bubbling
			   propagate it up the parent chain naturally. */
			if (msg.message == WM_MOUSEWHEEL && state == STATE_GAME)
			{
				POINT pt = { GET_X_LPARAM(msg.lParam),
				             GET_Y_LPARAM(msg.lParam) };
				HWND hwndUnder = WindowFromPoint(pt);

				if (hwndUnder && hwndUnder != hMain
					&& IsChild(hMain, hwndUnder))
				{
					SendMessage(hwndUnder, msg.message,
						msg.wParam, msg.lParam);
					continue;
				}
			}

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

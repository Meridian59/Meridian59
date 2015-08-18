// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
* winmsg.c
*
*/

#include "client.h"

static HMODULE hRichEditLib; 

Bool is_foreground;   // True when program is in the foreground charlie: i want to access this elsewhere.

extern HPALETTE hPal;
extern BOOL		gbMouselook;

int connection;  /* Current connection to server: CON_NONE, CON_SERIAL, ... */
int state;       /* Current client mode: terminal, game, blank, login, ... */

extern RECT		gD3DRect;

/****************************************************************************/
void StartHelp(void)
{
	WebLaunchBrowser(GetString(hInst, IDS_HELPFILE));
}
/****************************************************************************/
void MainInitState(int new_state)
{
	state = new_state;
	debug(("New main state = %d\n", state));
	
	switch (new_state)
	{
	case STATE_OFFLINE:
		DisableGameMenus();
		MenuConnect();
		OfflineInit();
		break;
		
	case STATE_CONNECTING:
		ConnectingInit();
		break;
	case STATE_LOGIN:
		LoginInit();
		break;
	case STATE_STARTUP:
		StartupInit(STATE_LOGIN);
		break;
	case STATE_TERM:
		TermInit();
		break;
	case STATE_GAME:
		GameInit();
		break;
	case STATE_DOWNLOAD:
		DownloadInit();
		break;
	}
}
/****************************************************************************/
void MainExitState(void)
{
	debug(("Old main state = %d\n", state));
	switch (state)
	{
	case STATE_OFFLINE:
		MenuDisconnect();
		OfflineExit();
		break;
		
	case STATE_CONNECTING:
		ConnectingExit();
		break;
	case STATE_LOGIN:
		// Free any modules that are sitting around (need to be unloaded so that
		// they can be overwritten during download).						   
		ModulesClose();
		LoginExit();
		break;
	case STATE_STARTUP:
		AbortStartupTimer();
		break;
	case STATE_TERM:
		TermExit();
		break;
	case STATE_GAME:
		GameExit();
		break;
	case STATE_DOWNLOAD:
		DownloadExit();
		break;
	}
}

void MainSetState(int new_state)
{
	if (new_state == state)
		return;
	
	MainExitState();
	MainInitState(new_state);
}

/****************************************************************************/
/*
* MainInit:  Called when main window is created.
*/
BOOL MainInit(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	connection = CON_NONE;
	state = STATE_INIT;
	
	/* Verify that user's hardware is sufficient to run game */
	if (!StartupCheck())
		exit(1); 
	
	/* Make standard palette */
	hPal = InitializePalette();
	InitStandardXlats(hPal);
	
	LoadSettings();
	MenuDisplaySettings(hwnd);
	
	// Load rich edit control DLL and common controls
	hRichEditLib = LoadLibrary("riched32.dll");
	InitCommonControls();
	
	MusicInitialize();
	
	SetMainCursor(LoadCursor(NULL, IDC_ARROW));
	
	// Find default Web browser, if not set manually
	if (config.default_browser)
		WebFindDefaultBrowser();
	
	return TRUE;
}
/****************************************************************************/
void MainQuit(HWND hwnd)
{
	CloseConnection();
	
	if (config.save_settings)
		SaveSettings();
	FreeProfaneTerms();
	
	MainExitState();
	
	FontsDestroy();
	ColorsDestroy();
	MusicClose();
	PaletteDeactivate();
	
	DeleteObject(hPal);

	HookClose();
	
	FreeLibrary(hRichEditLib);

	D3DRenderShutDown();
	
	PostQuitMessage(0);
}
/****************************************************************************/
void MainExpose(HWND hwnd)
{                   
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (state)
	{
	case STATE_OFFLINE:
		OfflineExpose(hwnd);
		break;
		
	case STATE_GAME:
		GameExpose(hwnd);
		break;
		
	default:
		/* Redraw background */
		hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, GetBrush(COLOR_BGD));
		EndPaint(hwnd, &ps);
		break;
	}
}
/****************************************************************************/
/* Return True iff message should NOT be passed to Windows for default processing */
Bool MainKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	/* See if a module wants to handle key */
	if (ModuleEvent(EVENT_KEY, hwnd, vk, fDown, cRepeat, flags) == False)
		return True;
	
	switch (state)
	{
	case STATE_GAME:
		return GameKey(hwnd, vk, fDown, cRepeat, flags);
	}
	return False;
}

/****************************************************************************/
void MainChar(HWND hwnd,char ch,int cRepeat)
{
}
/****************************************************************************/
void MainMouseLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	// Send it to HandleKeys first in case user set a keybind to one
	// of the mouse buttons.
	HandleKeys(0);

	/* See if a module wants to handle mouse click */
	if (ModuleEvent(EVENT_MOUSECLICK, hwnd, fDoubleClick, x, y, keyFlags) == False)
		return;

	switch (state)
	{
	case STATE_GAME:
		GameMouseButtonDown(hwnd, fDoubleClick, x, y, keyFlags);
		break;
	}
}
/****************************************************************************/
void MainMouseMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	// Send it to HandleKeys first in case user set a keybind to one
	// of the mouse buttons.
	HandleKeys(0);

	/* See if a module wants to handle mouse click */
	if (ModuleEvent(EVENT_MOUSECLICK, hwnd, fDoubleClick, x, y, keyFlags) == False)
		return;

	switch (state)
	{
	case STATE_GAME:
		GameMouseButtonDown(hwnd, fDoubleClick, x, y, keyFlags);
		break;
	}
}
/****************************************************************************/
void MainMouseRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	// Send it to HandleKeys first in case user set a keybind to one
	// of the mouse buttons.
	HandleKeys(0);

	/* See if a module wants to handle mouse click */
	if (ModuleEvent(EVENT_MOUSECLICK, hwnd, fDoubleClick, x, y, keyFlags) == False)
		return;

	switch (state)
	{
	case STATE_GAME:
		GameMouseButtonDown(hwnd, fDoubleClick, x, y, keyFlags);
		break;
	}
}
/****************************************************************************/
void MainMouseLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	switch (state)
	{
	case STATE_GAME:
		GameLButtonUp(hwnd, x, y, keyFlags);
		break;
	}
}
/****************************************************************************/
void MainMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	switch (state)
	{
	case STATE_GAME:
		if (gbMouselook)
		{
			RECT		rect;
//			WINDOWINFO	windowInfo;
			POINT	pt, center;
//			int		x, y;

//			GetWindowInfo(hMain, &windowInfo);
			GetClientRect(hMain, &rect);
			pt.x = rect.left;
			pt.y = rect.top;
			ClientToScreen(hMain, &pt);

			center.x = (gD3DRect.right - gD3DRect.left) / 2;
			center.y = (gD3DRect.bottom - gD3DRect.top) / 2;

			center.x += gD3DRect.left + pt.x;
			center.y += gD3DRect.top + pt.y;

			GetCursorPos(&pt);

/*			if (pt.x != (rect.right / 2))
				UserTurnPlayerMouse((pt.x - (rect.right / 2)) * gMouselookXScale);
			if (pt.y != (rect.bottom / 2))
				PlayerChangeHeightMouse((pt.y - (rect.bottom / 2)) * gMouselookYScale);

			if ((pt.x != (rect.right / 2)) || (pt.y != (rect.bottom / 2)))
				SetCursorPos(rect.right / 2, rect.bottom / 2);*/
			if (pt.x != center.x)
				UserTurnPlayerMouse((pt.x - center.x) * config.mouselookXScale / 3);
			if (pt.y != center.y)
				PlayerChangeHeightMouse((pt.y - center.y) * config.mouselookYScale / 3);

			if ((pt.x != center.x) || (pt.y != center.y))
				SetCursorPos(center.x, center.y);
		}

		GameMouseMove(hwnd, x, y, keyFlags);
		break;
	}
}
/****************************************************************************/
void MainVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	switch (state)
	{
	case STATE_GAME:
		GameVScroll(hwnd, hwndCtl, code, pos);
	}
}
/****************************************************************************/
void MainTimer(HWND hwnd, UINT id)
{
  GameTimer(hwnd, id);
}
/****************************************************************************/
void MainMove(HWND hwnd, int x, int y)
{
	switch (state)
	{
	case STATE_GAME:
		GameMove(hwnd,x,y);
		break;
	case STATE_CONNECTING:
		GuestMove(hwnd, x, y);
		break;
	}
}
/****************************************************************************/
void MainResize(HWND hwnd, UINT resize_flag, int xsize, int ysize)
{
	switch (state)
	{
	case STATE_OFFLINE:
		OfflineResize(xsize, ysize);
		break;
	case STATE_TERM:
		TermResize(hwnd, resize_flag, xsize, ysize);
		break;
	case STATE_GAME:
		GameResize(hwnd, resize_flag, xsize, ysize);
		break;
	case STATE_CONNECTING:
		GuestResize(hwnd, resize_flag, xsize, ysize);
		break;
	}
}
/****************************************************************************/
void MainMinMaxInfo(HWND hwnd, MINMAXINFO *lpmmi)
{
	SIZE s;
	
	/* Set minimum size of window */
	lpmmi->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
	lpmmi->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
	
	InterfaceGetMaxSize(&s);
	lpmmi->ptMaxTrackSize.x = s.cx;
	lpmmi->ptMaxTrackSize.y = s.cy;
}
/****************************************************************************/
void MainSetFocus(HWND hwnd, HWND hwndOldFocus)
{
	switch (state)
	{
	case STATE_OFFLINE:
		OfflineSetFocus(hwnd, hwndOldFocus);
		break;
	case STATE_TERM:
		TermSetFocus(hwnd, hwndOldFocus);
		break;
	case STATE_GAME:
		GameSetFocus(hwnd, hwndOldFocus);
		break;
	}
}
/****************************************************************************/
void MainKillFocus(HWND hwnd, HWND hwndNewFocus)
{
	switch (state)
	{
	case STATE_GAME:
		if (gbMouselook)
			UserMouselookToggle();
		GameKillFocus(hwnd, hwndNewFocus);
		break;
	}
}
/****************************************************************************/
void MainMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
	switch (state)
	{
	case STATE_GAME:
		GameMenuSelect(hwnd, hmenu, item, hmenuPopup, flags);
		break;
	}
}
/****************************************************************************/
Bool MainDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
	switch (state)
	{
	case STATE_GAME:
		return GameDrawItem(hwnd, lpdis);
	}
	return False;
}
/****************************************************************************/
void MainPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
	if (hwndPaletteChange != hMain)
		PaletteActivate(is_foreground);
}
/****************************************************************************/
Bool MainQueryNewPalette(HWND hwnd)
{
	PaletteActivate(is_foreground);
	return True;
}

/****************************************************************************/
void MainProcessBuffer(void)
{
	switch (state)
	{
	case STATE_OFFLINE:
		debug(("can't get here, mainprocessbuffer offline\n"));
		break;
	default:
		ProcessMsgBuffer();
		break;
	}
}
/****************************************************************************/
void MainChangeFont(void)
{
	switch (state)
	{
	case STATE_TERM:
		TermChangeFont();
		break;
	case STATE_GAME:
		GameChangeFont();
		break;
	}
}
/****************************************************************************/
void MainChangeColor(void)
{
	switch (state)
	{
	case STATE_OFFLINE:
	case STATE_TERM:
		InvalidateRect(hMain, NULL, TRUE);
		break;
	case STATE_GAME:
		GameChangeColor();
		break;
	}
}
/****************************************************************************/
void MainEnterIdle(HWND hwnd, UINT source, HWND hwndSource)
{
	switch(state)
	{
	case STATE_GAME:
		GameEnterIdle(hwnd, source, hwndSource);
		break;
	}
}
/****************************************************************************/
/* Called when event queue is empty */
void MainIdle(void)
{
	switch(state)
	{
	case STATE_GAME:
		GameIdle();
		break;
	}
}
/****************************************************************************/
void MainCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	/* Handle stuff other than menus & accelerators */
	if (hwndCtl != 0)
		switch (state)
	{
      case STATE_GAME:
		  GameCommand(hwnd, id, hwndCtl, codeNotify);
		  break;
	}
	
	MenuCommand(hwnd, id, hwndCtl, codeNotify);
}
/****************************************************************************/
int MainNotify(HWND hwnd, int idCtrl, NMHDR *pnmh)
{
	switch(state)
	{
	case STATE_GAME:
		return GameNotify(hwnd, idCtrl, pnmh);
		break;
	}  
	return 0;
}

static BOOL CALLBACK InvalidateTheWindow(HWND hwnd,LPARAM lParam)
{
	InvalidateRect(hwnd,NULL,FALSE);
	return TRUE;
}

/****************************************************************************/
void MainActivateApp(HWND hwnd, BOOL fActivate, DWORD dwThreadId)
{
	is_foreground = fActivate;
	if (fActivate)
	{
		EnumChildWindows(hwnd,InvalidateTheWindow,0);
	}
}

/********************************************************************/
void MainReadSocket(HWND hwnd, int SelectType, SOCKET s, int error)
{
	switch (SelectType)
	{
	case FD_CONNECT:
		if (state != STATE_CONNECTING)
		{
			debug(("MainReadSocket got connect in wrong state (%d)\n", state));
			return;
		}
		ConnectingDone(error);
		break;
		
	case FD_CLOSE:
		// When guest server denies us during login, don't abort
		if (config.guest && 
			(state == STATE_CONNECTING || state == STATE_LOGIN))
			return;
		
		MainSetState(STATE_OFFLINE);  /* Kill off dialogs, etc. */
		connection = CON_NONE;
		ClientError(hInst, hMain, IDS_LOSTSERVER);
		return;
		
	case FD_READ:        /* Read stuff from server */
		ReadServer();
		MainProcessBuffer();
		break;
		
	default:
		ClientError(hInst, hMain, IDS_SOCKETMESSAGE);
		break;
	}
}

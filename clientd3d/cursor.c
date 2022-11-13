// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* cursor.c:  Deal with user moving the cursor around with the keyboard, 
*   changing the shape of the cursor, and other cursor-related functions.
*/

#include "client.h"

// Standard game cursors.
static HCURSOR hCursorArrow = NULL;
static HCURSOR hCursorTarget = NULL;
static HCURSOR hCursorInside = NULL;
static HCURSOR hCursorCross = NULL;
static HCURSOR hCursorGet = NULL;
static HCURSOR hCursorWait = NULL;

// Resize window cursors.
static HCURSOR hCursorResizeWE = NULL;
static HCURSOR hCursorResizeNS = NULL;
static HCURSOR hCursorResizeNESW = NULL;
static HCURSOR hCursorResizeNWSE = NULL;

void UserMoveEsc(void)
{
	SetPlayerRemoteView(0,0,0,0);
}

/************************************************************************/
/*
* UserMoveCursor:  Move cursor by dx pixels horizontally and dy pixels
*   vertically.  Also ensure that cursor is in grid area and on screen.
*/
void UserMoveCursor(int command, int dy, int dx)
{
	POINT c;
	POINT upleft, downright;
	AREA view;
	
	CopyCurrentView(&view);
	
	GetCursorPos(&c);
	
	/* Convert view to screen coordinates */
	upleft.x = view.x;
	upleft.y = view.y;
	ClientToScreen(hMain, &upleft);
	
	downright.x = view.x + view.cx - 1;
	downright.y = view.y + view.cy - 1;
	ClientToScreen(hMain, &downright);
	
	c.y = min(max(upleft.y, c.y + dy), downright.y - 1);
	c.x = min(max(upleft.x, c.x + dx), downright.x - 1);
	
	SetCursorPos(c.x, c.y);
}

/************************************************************************/


/************************************************************************/
/* Returns TRUE iff cursor was set                                          */
BOOL MainSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	switch (codeHitTest) {
		case HTLEFT:
		case HTRIGHT:
			SetMainCursor(hCursorResizeWE);
			return true;
		break;
		case HTTOP:
		case HTBOTTOM:
			SetMainCursor(hCursorResizeNS);
			return true;
		break;
		case HTBOTTOMLEFT:
		case HTTOPRIGHT:
			SetMainCursor(hCursorResizeNESW);
			return true;
		break;
		case HTBOTTOMRIGHT:
		case HTTOPLEFT:
			SetMainCursor(hCursorResizeNWSE);
			return true;
		break;
		default:
		break;
	}
	return GameWindowSetCursor();
}
/************************************************************************/
void SetMainCursor(HCURSOR cursor)
{
	SetCursor(cursor);
}
/************************************************************************/
/*
* GameWindowSetCursor: Set mouse pointer to appropriate shape, depending on
*   what is underneath it.
*/
BOOL GameWindowSetCursor(void)
{
	int x, y;
	HCURSOR cursor = NULL;
	room_contents_node *r;
	POINT pt;
	
	if (hCursorArrow == NULL)
	{
		hCursorArrow = LoadCursor(NULL, IDC_ARROW);
		hCursorTarget = LoadCursor(hInst, MAKEINTRESOURCE(IDC_TARGETCURSOR));
		hCursorInside = LoadCursor(hInst, MAKEINTRESOURCE(IDC_INSIDECURSOR));
		hCursorCross = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CROSSCURSOR));
		hCursorGet = LoadCursor(hInst, MAKEINTRESOURCE(IDC_GETCURSOR));
		hCursorWait = LoadCursor(NULL, IDC_WAIT);
		hCursorResizeWE = LoadCursor(NULL, IDC_SIZEWE);
		hCursorResizeNS = LoadCursor(NULL, IDC_SIZENS);
		hCursorResizeNESW = LoadCursor(NULL, IDC_SIZENESW);
		hCursorResizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
	}
	
	if (state != STATE_GAME)
	{
		SetMainCursor(hCursorArrow);
		return TRUE;
	}
	
	GetCursorPos(&pt);
	
	if (GameGetState() == GAME_SELECT)
	{
		HWND hwndTop = WindowFromPoint(pt);
		
		while (GetParent(hwndTop) && (WS_CHILD & GetWindowLong(hwndTop,GWL_STYLE))) 
		{
			hwndTop = GetParent(hwndTop);
		}
		if (hMain == hwndTop)
		{
			RECT rc;
			POINT ptClient;
			ptClient = pt;
			GetClientRect(hwndTop,&rc);
			ScreenToClient(hwndTop,&ptClient);
			if (PtInRect(&rc,ptClient))
			{
				int x,y;
				if (!TranslateToRoom(ptClient.x,ptClient.y,&x,&y)) // if outside graphic view
				{ // then set to target, otherwise let code below handle the decision
					SetMainCursor(hCursorTarget);
					return TRUE;
				}
			}
			else
				return FALSE;
		}
		else
		{
			return FALSE;
		}
	}

	if (hMain != WindowFromPoint(pt))
      return FALSE;
		
		if (MouseToRoom(&x, &y) && !GraphicsMouseCaptured())
		{
			r = GetObjectByPosition(x, y, 0, 0, 0);
			if (r != NULL)
			{
				if ((r->obj.flags & OF_CONTAINER) && GetKeyState(VK_SHIFT) < 0)
					cursor = hCursorInside;
				else
					cursor = hCursorCross;
			}
			else if (GameGetState() == GAME_SELECT)
				cursor = hCursorTarget;
			else
				cursor = hCursorArrow;
		}
		
		if (!ModuleEvent(EVENT_SETCURSOR, cursor))
			return FALSE;
		
		switch (GameGetState())
		{
		case GAME_SELECT:
			if (cursor != NULL)
				break;
			cursor = hCursorTarget;
			break;
			
		case GAME_WAIT:
		case GAME_INIT:
		case GAME_INVALID:
		case GAME_RESYNC:
			cursor = hCursorWait;
			break;
			
		case GAME_PLAY:
		default:
			if (cursor == hCursorWait)
				cursor = hCursorArrow;
			
			if (GraphicsMouseCaptured())
				cursor = hCursorGet;
			
			if (NULL == cursor)
				cursor = hCursorArrow;
			
			break;
		}
		
		if (cursor)
			SetMainCursor(cursor);
		
		return (NULL != cursor);
}


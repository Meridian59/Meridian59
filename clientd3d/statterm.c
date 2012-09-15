// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * statterm.c:  Handle terminal mode (for administration)
 */

#include "client.h"

static HWND hwndHistory;  /* Scrollback history window */
static HWND hwndInput;    /* Input edit box */

static WNDPROC lpfnDefHistProc;  /* Default edit window procedure */
static WNDPROC lpfnDefInputProc; /* Default edit window procedure */

static char    *buf;             /* Buffer to store stuff coming from server */
static int     pos = 0;          /* Current position in buf */

extern Bool admin_mode;            // True when user wants to go into admin mode

/* local function prototypes */

long CALLBACK InputProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
long CALLBACK HistoryProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
/************************************************************************/
void TermInit(void)
{
   RECT r;

   admin_mode = False;     // Don't re-enter admin mode after leaving 

   /* clear the window for our use */
   InvalidateRect(hMain,NULL,TRUE); 

   hwndHistory = CreateWindow("edit", NULL, 
			      WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
			      ES_MULTILINE | ES_READONLY,
			      0, 0, 100, 100,
			      hMain, (HMENU) IDC_ADMIN1, hInst, NULL);

   hwndInput = CreateWindow("edit", NULL, 
			    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			    0, 0, 100, 100,
			    hMain, (HMENU) IDC_ADMIN2, hInst, NULL);
   Edit_LimitText(hwndInput, MAX_ADMIN);

   lpfnDefHistProc = SubclassWindow(hwndHistory, HistoryProc);
   lpfnDefInputProc = SubclassWindow(hwndInput, InputProc);

   /* Send resize message so that subwindows can size themselves */
   GetClientRect(hMain,&r);
   TermResize(hMain, 0, r.right,r.bottom);

   buf = (char *) SafeMalloc(MAX_HISTORY + 1);

   SetFocus(hwndInput);
}
/************************************************************************/
void TermResize(HWND hwnd, UINT resize_flag, int xsize, int ysize)
{
   MoveWindow(hwndHistory, 0, 0, xsize, ysize - 50, TRUE);
   MoveWindow(hwndInput, 0, ysize - 50, xsize, 50, TRUE);
   TermChangeFont();  /* Recalculate edit box's size */
}
/************************************************************************/
void TermExit(void)
{
   SafeFree(buf);

   DestroyWindow(hwndHistory);
   DestroyWindow(hwndInput);
}
/************************************************************************/
void TermSetFocus(HWND hwnd, HWND hwndOldFocus)
{
   /* Force focus from main window proper to input box */
   SetFocus(hwndInput);
}
/************************************************************************/
/*
 * TermChangeFont:  Fix up edit boxes when user changes font.
 */
void TermChangeFont(void)
{
   HFONT hFont = GetFont(FONT_ADMIN);   

   SetWindowFont(hwndHistory, hFont, TRUE);
   SetWindowFont(hwndInput, GetFont(FONT_INPUT), TRUE);

   /* Rescroll, since bigger font means fewer lines fit. */
   WindowBeginUpdate(hwndHistory);
   EditBoxScroll(hwndHistory, False);
   WindowEndUpdate(hwndHistory);
}
/************************************************************************/
/*
 * HistoryProc:  Subclassed window procedure for history edit box.
 */
long CALLBACK HistoryProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_KEYDOWN:
      /* Allow TAB key to switch between main windows */
      if (wParam == VK_TAB)
	 SetFocus(hwndInput);
      break;
   }
   return CallWindowProc(lpfnDefHistProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * InputProc:  Subclassed window procedure for input edit box.
 */
long CALLBACK InputProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   int len;
   char buf[MAX_ADMIN + 1];

   switch (message)
   {
   case WM_CHAR:
      /* Allow TAB key to switch between main windows */
      if (wParam == '\t')
      {
	 SetFocus(hwndHistory);
	 return 0;
      }

      /* Enter key sends text to server */
      if (wParam == '\r')
      {
	 len = Edit_GetTextLength(hwndInput);
	 Edit_GetText(hwndInput, buf, MAX_ADMIN);
	 WriteServer(buf, len);
	 WriteServer("\r", 1);
	 Edit_SetSel(hwndInput, 0, -1);
	 return 0;
      }

      if (wParam == '\n')
      {
	 return 0;
      }
      break;
   }
   return CallWindowProc(lpfnDefInputProc, hwnd, message, wParam, lParam);
}
/************************************************************************/


/* things called not from winmsg, but from characters from port */

/************************************************************************/
/*
 * AddCharTerm:  We got some characters from the server.  Add to buffer
 *   of characters to add to history window, and check if we should leave
 *   admin mode.
 */
void AddCharTerm(char *message, int num_chars)
{
   int i;
   static char prev_ch = 0;

   /* Check for server telling us to start game */
   for (i=0; i < num_chars; i++)
   {
      if (prev_ch == 27 && message[i] == 'G')
      {
	 pos = 0;
	 debug(("got leave main menu code\n"));
	 MainSetState(STATE_STARTUP); 
	 return;
      }
      prev_ch = message[i];
      buf[pos++] = message[i];
      if (pos > MAX_HISTORY)
      {
	 memmove(buf, buf + 1, MAX_HISTORY);
	 pos = MAX_HISTORY;
      }
   }
}
/************************************************************************/
/*
 * DoneCharsTerm:  Done reading characters from server; update edit box
 *   with characters that were read.
 */
void DoneCharsTerm(void)
{
   UINT txtlen;

   txtlen = Edit_GetTextLength(hwndHistory);

   WindowBeginUpdate(hwndHistory);

   /* If box is full remove some text at the start */
   if (txtlen + pos >= MAX_HISTORY)
   {
      Edit_SetSel(hwndHistory, 0, txtlen + pos - MAX_HISTORY);
      Edit_ReplaceSel(hwndHistory, "");
   }

   // Add new text to end of box
   buf[pos] = 0;
   txtlen = Edit_GetTextLength(hwndHistory);
   Edit_SetSel(hwndHistory, txtlen, txtlen);
   Edit_ReplaceSel(hwndHistory, buf);

   /* Scroll new string into view */
   EditBoxScroll(hwndHistory, False);

   WindowEndUpdate(hwndHistory);
   pos = 0;
}
/************************************************************************/


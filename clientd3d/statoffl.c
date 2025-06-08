// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statoffl.c:  Handle offline state, where user isn't connected to server.
 */

#include "client.h"

#define BUTTON_XSIZE 300   // Default button width; will be resized to size of splash bitmap 
#define BUTTON_YSIZE 30 

static HWND hwndDialButton = NULL;

/* Offscreen bitmap & DC for drawing title screen */
static HDC      hTitleDC;
static HBITMAP  hOldTitleBitmap;

static int bm_width, bm_height;  /* Size of intro bitmap */
static int button_width, button_height;

static POINT bm_origin;          /* Upper left corner of bitmap */
static POINT button_origin;      /* Upper left corner of button */

static char *splash_filename = "resource\\splash.bgf";  // Splash screen bitmap
static char *splash_music    = "resource\\main.mid";    // Music file to play

static Bool showing_splash;      // True when displaying splash screen

static WNDPROC lpfnDefButtonProc; /* Default button window procedure */

#define MUSIC_DELAY 3000         // # of milliseconds to wait before starting music

static int timer_id;             /* timer to start music */

static void IntroInit(void);
static void IntroShowSplash(void);
static LRESULT CALLBACK MainButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void CALLBACK PlayMusicProc(HWND hwnd, UINT msg, UINT_PTR timer, DWORD dwTime);

extern HPALETTE hPal;
extern int connection;  /* Current connection to server: CON_NONE, CON_SERIAL, ... */

/****************************************************************************/
void OfflineInit(void)
{
   connection = CON_NONE;
   ResetSecurityToken();
   showing_splash = False;

   if (config.quickstart)
   {
      OfflineConnect();
   }
   else
   {
     IntroShowSplash();
   }
}
/****************************************************************************/
void OfflineExit(void)
{
   MusicAbort();

   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }

   if (hTitleDC != NULL)
   {
      HBITMAP hCurrentBitmap = (HBITMAP) SelectObject(hTitleDC, hOldTitleBitmap);
      DeleteObject(hCurrentBitmap);
      DeleteDC(hTitleDC);
   }

   if (hwndDialButton != NULL)
     DestroyWindow(hwndDialButton);
   hwndDialButton = NULL;
   InvalidateRect(hMain, NULL, TRUE);
}
/****************************************************************************/
void OfflineSetFocus(HWND hwnd, HWND hwndOldFocus)
{
   if (hwndDialButton != NULL)
     SetFocus(hwndDialButton);
}
/****************************************************************************/
void OfflineExpose(HWND hwnd)
{
   PAINTSTRUCT ps;
   HDC hdc;
   int height;
   RECT r;

   /* Redraw background */
   hdc = BeginPaint(hMain, &ps);
   FillRect(hdc, &ps.rcPaint, GetBrush(COLOR_BGD));

   SelectPalette(hdc, hPal, FALSE);

   /* Draw bitmap */   
   GetClientRect(hMain, &r);
   height = min(bm_height, r.bottom - button_height);
   
   if (hTitleDC != NULL)
      BitBlt(hdc, bm_origin.x, bm_origin.y, bm_width, height, hTitleDC, 0, 0, SRCCOPY);
   EndPaint(hMain, &ps);
}
/****************************************************************************/
void OfflineResize(int xsize, int ysize)
{
   bm_origin.x = max(0, (xsize - bm_width) / 2);
   bm_origin.y = max(0, (ysize - button_height - bm_height) / 2);
   
   button_width = min(bm_width, xsize);

   button_origin.x = max(0, (xsize - button_width) / 2);
   button_origin.y = max(0, min(bm_origin.y + bm_height, ysize - button_height));

   if (hwndDialButton != NULL)
     MoveWindow(hwndDialButton, button_origin.x, button_origin.y, 
		button_width, button_height, TRUE);
   InvalidateRect(hMain, NULL, TRUE);  /* redraw all */
}
/****************************************************************************/
/*
 * OfflineConnect:  Attempt to connect to server using appropriate method.
 */
void OfflineConnect(void)
{
   if (!GetLogin())
      return;

   connection = CON_SOCKET;
   
   if (!OpenSocketConnection(config.comm.hostname, config.comm.sockport))
   {
      connection = CON_NONE;
      ClientError(hInst, hMain, IDS_SOCKETOPEN, config.comm.sockport, config.comm.hostname);
      return;
   }
   MainSetState(STATE_CONNECTING);
}
/****************************************************************************/
/*
 * IntroShowSplash:  Display splash screen.
 */
void IntroShowSplash(void)
{
   RECT rect;
   BYTE *gTitleBits;
   int i;
   Bitmaps b;
   PDIB pdib;

   if (config.quickstart)
   {
      OfflineConnect();
      return;
   }
   if (showing_splash)
     return;
   showing_splash = True;

   hwndDialButton = CreateWindow("button", NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hMain, (HMENU) IDC_DIALBUTTON,
		hInst, NULL);
   SetWindowText(hwndDialButton, GetString(hInst, IDS_INTRO));

   /* Subclass button */
   lpfnDefButtonProc = SubclassWindow(hwndDialButton, MainButtonProc);

   /* Get bits of bitmap from bgf file */   
   if (DibOpenFile(splash_filename, &b))
   {
      pdib = BitmapsGetPdibByIndex(b, 0);

      /* Get bitmap's size */
      bm_width  = DibWidth(pdib);
      bm_height = DibHeight(pdib);

      /* Create bitmap */
      hTitleDC = CreateMemBitmap(bm_width, bm_height, &hOldTitleBitmap, &gTitleBits);
      if (hTitleDC == NULL)
      {
	 debug(("IntroShowSplash couldn't create bitmap!\n"));
	 BitmapsFree(&b);
	 return;
      }
      
      /* Copy bits into bitmap */
      for (i=0; i < bm_height; i++)
	 memcpy(gTitleBits + i * DIBWIDTH(bm_width), DibPtr(pdib) + i * bm_width, bm_width);

      BitmapsFree(&b);

   }
   else 
   {
      hTitleDC = NULL;
      bm_width = BUTTON_XSIZE;
   }

   button_width = bm_width;
   button_height = BUTTON_YSIZE;

   /* Simulate resize to get positions right */
   GetClientRect(hMain, &rect);
   OfflineResize(rect.right, rect.bottom);

   SetFocus(hwndDialButton);

   timer_id = SetTimer(NULL, 0, MUSIC_DELAY, PlayMusicProc);
}
/************************************************************************/
void CALLBACK PlayMusicProc(HWND hwnd, UINT msg, UINT_PTR timer, DWORD dwTime)
{
   KillTimer(NULL, timer_id);
   timer_id = 0;

   // Play music
   if (config.play_music)
      PlayMusicFile(hMain, splash_music);
}
/****************************************************************************/
/*
 * MainButtonProc:  Subclassed window procedure for main button.
 */
LRESULT CALLBACK MainButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_KEYDOWN:
      if (wParam == VK_RETURN || wParam == VK_SPACE)
      {
	 OfflineConnect();
	 return 0;
      }
      break;

   case WM_LBUTTONUP:
      OfflineConnect();
      return 0;
   }
   return CallWindowProc(lpfnDefButtonProc, hwnd, message, wParam, lParam);
}

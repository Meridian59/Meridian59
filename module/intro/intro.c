// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intro.c:  Main source file for intro module, which handles splash screen.
 */

#include "client.h"
#include "introrc.h"

HINSTANCE hInst;              // Handle of this DLL

ClientInfo *cinfo;         // Holds data passed from main client

#define BUTTON_XSIZE 300   // Default button width; will be resized to size of splash bitmap 
#define BUTTON_YSIZE 30 

static HWND hwndDialButton = NULL;

/* Offscreen bitmap & DC for drawing title screen */
static HDC      hTitleDC;
static HBITMAP  hOldTitleBitmap;
static BYTE *gTitleBits;        // Pointer to actual bits of offscreen bitmap

static RawBitmap logo;          // Logo bitmap

static int bm_width, bm_height;  /* Size of intro bitmap */
static int button_width, button_height;

static POINT bm_origin;          /* Upper left corner of bitmap */
static POINT button_origin;      /* Upper left corner of button */

static char *splash_filename = "resource\\splash.bgf";  // Splash screen bitmap
static char *logo_filename   = "resource\\logo.bgf";    // Logo bgf file
static char *splash_music    = "main.mp3";              // Music file to play

static BYTE light_level;         // Light level for logo fade

static Bool showing_splash;      // True when displaying splash screen

static WNDPROC lpfnDefButtonProc; /* Default button window procedure */

#define LOGO_FADE_DELAY  10      // # of milliseconds between fade frames
#define LOGO_DELAY  3000         // # of milliseconds to display logo
#define MUSIC_DELAY 3000         // # of milliseconds to wait before starting music

static int timer_id;             /* timer to start music */

static void IntroInit(void);
static void IntroShowSplash(void);
static void IntroDrawLogo(void);
static void IntroFreeLogo(void);
static long CALLBACK MainButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
static void CALLBACK PlayMusicProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime);
static void CALLBACK LightTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime);
/****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
      hInst = hModule;
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   info->event_mask = EVENT_REDRAW | EVENT_RESIZE | EVENT_KEY | EVENT_MOUSECLICK | EVENT_SETFOCUS;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_OFFLINE_ID;
   cinfo = client_info;    // Save client info for our use later

   showing_splash = False;

   if (cinfo->config->quickstart)
   {
      OfflineConnect();
   }
   else
   {
      // Only show logo the first time in (at startup)
      if (*(cinfo->first_load))
        IntroInit();
      else IntroShowSplash();
   }
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   HBITMAP hCurrentBitmap;

   MusicStop();

   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }

   if (hTitleDC != NULL)
   {
      hCurrentBitmap = (HBITMAP) SelectObject(hTitleDC, hOldTitleBitmap);
      DeleteObject(hCurrentBitmap);
      DeleteDC(hTitleDC);
   }

   if (hwndDialButton != NULL)
     DestroyWindow(hwndDialButton);
   hwndDialButton = NULL;
   InvalidateRect(cinfo->hMain, NULL, TRUE);

   PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_OFFLINE_ID);
}
/****************************************************************************/
/*
 * EVENT_REDRAW
 */
/****************************************************************************/
Bool WINAPI EventRedraw(HDC main_hdc)
{
   PAINTSTRUCT ps;
   HDC hdc;
   int height;
   RECT r;

   /* Redraw background */
   hdc = BeginPaint(cinfo->hMain, &ps);
   FillRect(hdc, &ps.rcPaint, GetBrush(COLOR_BGD));

   SelectPalette(hdc, cinfo->hPal, FALSE);

   /* Draw bitmap */   
   GetClientRect(cinfo->hMain, &r);
   height = min(bm_height, r.bottom - button_height);
   
   if (hTitleDC != NULL)
      BitBlt(hdc, bm_origin.x, bm_origin.y, bm_width, height, hTitleDC, 0, 0, SRCCOPY);
   EndPaint(cinfo->hMain, &ps);
   return True;
}
/****************************************************************************/
/*
 * EVENT_RESIZE
 */
/****************************************************************************/
Bool WINAPI EventResize(int xsize, int ysize, AREA *view)
{
   bm_origin.x = max(0, (xsize - bm_width) / 2);
   bm_origin.y = max(0, (ysize - button_height - bm_height) / 2);
   
   button_width = min(bm_width, xsize);

   button_origin.x = max(0, (xsize - button_width) / 2);
   button_origin.y = max(0, min(bm_origin.y + bm_height, ysize - button_height));

   if (hwndDialButton != NULL)
     MoveWindow(hwndDialButton, button_origin.x, button_origin.y, 
		button_width, button_height, TRUE);
   InvalidateRect(cinfo->hMain, NULL, TRUE);  /* redraw all */
   return True;
}
/****************************************************************************/
/*
 * EVENT_SETFOCUS
 */
/****************************************************************************/
Bool WINAPI EventSetFocus(void)
{
   if (hwndDialButton != NULL)
     SetFocus(hwndDialButton);
   return True;
}
/****************************************************************************/
/*
 * EVENT_KEY
 */
/****************************************************************************/
Bool WINAPI EventKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
  KillTimer(NULL, timer_id);
  IntroShowSplash();
  return True;
}
/****************************************************************************/
/*
 * EVENT_MOUSECLICK
 */
/****************************************************************************/
Bool WINAPI EventMouseClick(HWND hwnd, Bool fDoubleClick, int x, int y, UINT flags)
{
  KillTimer(NULL, timer_id);
  IntroShowSplash();
  return True;
}
/****************************************************************************/
/*
 * IntroInit:  Display logo.
 */
void IntroInit(void)
{
   RECT rect;

   // Load logo
   if (!GetBitmapResourceInfo(hInst, IDB_LOGO, &logo))
   {
     IntroShowSplash();
     return;
   }

   /* Create bitmap for copying to screen */
   hTitleDC = CreateMemBitmap(logo.width, logo.height, &hOldTitleBitmap, &gTitleBits);
   if (hTitleDC == NULL)
   {
     debug(("IntroInit couldn't create bitmap!\n"));
     IntroShowSplash();
     return;
   }

   bm_width = logo.width;
   bm_height = logo.height;
   
   /* Simulate resize to get positions right */
   GetClientRect(cinfo->hMain, &rect);
   EventResize(rect.right, rect.bottom, NULL);

   // Fade light level in
   light_level = 0;

   // Display logo for a certain amount of time
   timer_id = SetTimer(NULL, 0, LOGO_FADE_DELAY, LightTimerProc);
}
/************************************************************************/
/*
 * LightTimerProc:  Adjust light level for logo fade.
 */
void CALLBACK LightTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
  light_level++;

  if (light_level >= NUM_PALETTES)
  {
    // Move on to splash screen
    KillTimer(NULL, timer_id);
    IntroShowSplash();
  }    
  else if (light_level == NUM_PALETTES - 1)
  {
    KillTimer(NULL, timer_id);
    // Show logo for a few seconds
    timer_id = SetTimer(NULL, 0, LOGO_DELAY, LightTimerProc);
  }
  else IntroDrawLogo();
}
/************************************************************************/
/*
 * IntroDrawLogo:  Draw logo at current light level.
 */
void IntroDrawLogo(void)
{
  int i, j, width;
  HDC hdc;
  BYTE *palette;

  /* Copy bits into bitmap */
  palette = (*(cinfo->light_palettes))[light_level];
  width = DIBWIDTH(logo.width);
  for (i=0; i < logo.height; i++)
    for (j=0; j < logo.width; j++)
      *(gTitleBits + i * width + j) = palette[*(logo.bits + (logo.height - i - 1) * width + j)];
  
  // Copy to screen
  hdc = GetDC(cinfo->hMain);
  SelectPalette(hdc, cinfo->hPal, FALSE);
  BitBlt(hdc, bm_origin.x, bm_origin.y, logo.width, logo.height, hTitleDC, 0, 0, SRCCOPY);
  ReleaseDC(cinfo->hMain, hdc);
}
/************************************************************************/
void IntroFreeLogo(void)
{
   HBITMAP hCurrentBitmap;

   if (hTitleDC != NULL)
   {
      hCurrentBitmap = (HBITMAP) SelectObject(hTitleDC, hOldTitleBitmap);
      DeleteObject(hCurrentBitmap);
      DeleteDC(hTitleDC);
   }
   hTitleDC = NULL;
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

   if (cinfo->config->quickstart)
   {
      OfflineConnect();
      return;
   }
   if (showing_splash)
     return;
   showing_splash = True;

   IntroFreeLogo();
   hwndDialButton = CreateWindow("button", NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, cinfo->hMain, (HMENU) IDC_DIALBUTTON,
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
   GetClientRect(cinfo->hMain, &rect);
   EventResize(rect.right, rect.bottom, NULL);

   SetFocus(hwndDialButton);

   timer_id = SetTimer(NULL, 0, MUSIC_DELAY, PlayMusicProc);
}
/************************************************************************/
void CALLBACK PlayMusicProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
   KillTimer(NULL, timer_id);
   timer_id = 0;

   // Play music
   if (cinfo->config->play_music)
      MusicPlayFile(splash_music);
}

/****************************************************************************/
/*
 * MainButtonProc:  Subclassed window procedure for main button.
 */
long CALLBACK MainButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
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

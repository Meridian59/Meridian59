// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * about.c:  Handle the "About" dialog.
 */

#include "client.h"

#define TIMER_ABOUT 1
#define ABOUT_INTERVAL 80 // # of milliseconds between animations
#define NUM_DUDES 2

static int dude_x;         // x position of dude area on window
#define DUDE_X1  40       // x position of center of dude #1
#define DUDE_X2  110       // x position of center of dude #2
#define DUDE_AREA_WIDTH  150
static int dude_y;
#define DUDE_VELOCITY 20   // Walking speed in pixels per second

#define DUDE_MAX_HEIGHT 150

static HWND hAboutDlg;
static char *credits_filename = "resource\\about.bgf";
static Bitmaps credits_b;          // Info on scrolling bitmap
static int credits_page = 0;  // which graphics page to show;
static PDIB credits_pdib;  // Actual pdib for scrolling bitmap

static HDC      gDC;       // Offscreen area for drawing into
static BYTE    *gBits;     // Pointer to pixels of offscreen bitmap
static HBITMAP  gOldBitmap;
static int gbits_width, gbits_height;  // Size of offscreen bitmap

static int timer_id;
static int scroll_width, scroll_height;  // Size of scrolling window
static int scroll_y;       // Current y position in scrolling bitmap

static char *sounds[] = { "swrdmtl1.wav", 
			  "swrdmtl2.wav", 
			  "swrdmtl3.wav",
			  "patk.wav",
			  "orc_awr.wav",
			  "fquake.wav",                          
};
#define num_sounds 3   // # of sounds for default dude

typedef struct {
  object_node *obj;
  int x;               // Place on window to draw guy
  int angle;
} Dude;

static Dude dudes[NUM_DUDES];

typedef struct {
   int min_y, max_y;   // Vertical extent of name
   int index;          // Dude # to change
   int rsc;            // Rsc of bgf to make dude
} Name;

static Name names[] = {
   {118,  136, 1,  ABOUT_RSC3},
   {136,  157, 0,  ABOUT_RSC1},
   {303,  323, 1,  ABOUT_RSC2},
};
#define num_names (sizeof(names) / sizeof(Name))

extern HPALETTE hPal;

static BOOL AboutInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
static void AboutTimer(HWND hwnd, UINT id);
static void AboutLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
/*****************************************************************************/
/*
 * AboutDialogProc:  Display info about client.
 */
BOOL CALLBACK AboutDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int i;

   HBITMAP gBitmap;

   switch (message)
   {
     HANDLE_MSG(hDlg, WM_INITDIALOG, AboutInitDialog);
     HANDLE_MSG(hDlg, WM_TIMER, AboutTimer);
     HANDLE_MSG(hDlg, WM_LBUTTONDOWN, AboutLButtonDown);
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
      case IDCANCEL:
	 EndDialog(hDlg, IDOK);
	 if (timer_id != 0)
	   KillTimer(hDlg, TIMER_ABOUT);

	 gBitmap = (HBITMAP) SelectObject(gDC, gOldBitmap);
	 DeleteObject(gBitmap);
	 DeleteDC(gDC);

	 for (i=0; i < NUM_DUDES; i++)
	   if (dudes[i].obj != NULL)
	     dudes[i].obj = ObjectDestroyAndFree(dudes[i].obj);

	 BitmapsFree(&credits_b);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hAboutDlg = NULL;
      return TRUE;
   
   }

   return FALSE;
}

void CenterDlgItem(HWND hDlg, UINT idcItem)
{
   HWND hwnd = GetDlgItem(hDlg, idcItem);
   RECT rcItem,rcDlg;
   POINT pt;
   int widthDlg,widthItem;
   GetWindowRect(hDlg,&rcDlg);
   GetWindowRect(hwnd,&rcItem);
   widthDlg = rcDlg.right - rcDlg.left;
   widthItem = rcItem.right - rcItem.left;
   pt.x = rcItem.left;
   pt.y = rcItem.top;
   ScreenToClient(hDlg,&pt);
   SetWindowPos(hwnd,NULL,(widthDlg - widthItem)/2,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}

/*****************************************************************************/
/*
 * AboutInitDialog:  Handle WM_INITDIALOG messages.
 */ 
BOOL AboutInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam) 
{
   int i,sideBuffer,xEdge,yEdge;
   POINT pt;
   HWND hwndScroll = GetDlgItem(hDlg, IDC_SCROLL);
   RECT r, rcScrollClient, rcScroll, rcDlg, rcDlgClient;
   char buffer[80];
   char format[80];

   /* Load credits from bgf file */
   if (DibOpenFile(credits_filename, &credits_b))
      credits_pdib = BitmapsGetPdibByIndex(credits_b, credits_page);
   else debug(("AboutInitDialog unable to load file %s\n", credits_filename));

   GetWindowRect(hwndScroll, &r);
   OffsetRect(&r, -r.left, -r.top);

   GetWindowRect(hDlg, &rcDlg);
   GetWindowRect(hwndScroll, &rcScroll);
   GetClientRect(hwndScroll, &rcScrollClient);
   sideBuffer = rcScroll.left - rcDlg.left;
   xEdge = (rcScroll.right-rcScroll.left) - rcScrollClient.right;
   yEdge = (rcScroll.bottom-rcScroll.top) - rcScrollClient.bottom;

   scroll_width  = DibWidth(credits_pdib);
   scroll_height = rcScrollClient.bottom;
   SetWindowPos(hwndScroll,NULL,0,0,xEdge + scroll_width,rcScroll.bottom - rcScroll.top,SWP_NOZORDER|SWP_NOMOVE);
   SetWindowPos(hDlg,NULL,0,0,xEdge + sideBuffer*2 + scroll_width,rcDlg.bottom - rcDlg.top,SWP_NOZORDER|SWP_NOMOVE);

   GetWindowRect(hDlg, &rcDlg);
   GetClientRect(hDlg, &rcDlgClient);

   // Center windows horizontally
   CenterDlgItem(hDlg, IDC_NUMBER);
   CenterDlgItem(hDlg, IDC_SPECIAL1);
   CenterDlgItem(hDlg, IDC_SPECIAL2);
   CenterDlgItem(hDlg, IDC_SCROLL);
   CenterDlgItem(hDlg, IDOK);
   
   hAboutDlg = hDlg;
   CenterWindow(hDlg, GetParent(hDlg));

   GetDlgItemText(hDlg, IDC_NUMBER, format, 80);
   sprintf(buffer, format, MAJOR_REV*100+MINOR_REV);
   SetDlgItemText(hDlg, IDC_NUMBER, buffer);

   if (config.technical)
   {
	sprintf(buffer, "Client Software Version %d:%d", MAJOR_REV, MINOR_REV);
	SetDlgItemText(hDlg, IDC_SPECIAL1, buffer);

	sprintf(buffer, "Server Host Address %s:%d",
		(LPCTSTR)config.comm.hostname, config.comm.sockport);
	SetDlgItemText(hDlg, IDC_SPECIAL2, buffer);
   }

   GetWindowRect(GetDlgItem(hDlg,IDC_SPECIAL2),&r);
   pt.x = r.left + (r.right-r.left)/2 - DUDE_AREA_WIDTH/2;
   pt.y = r.bottom + 10;
   ScreenToClient(hDlg,&pt);

   scroll_y = 0;
   dude_x = pt.x;
   dude_y = pt.y;

   gbits_width  = max(scroll_width, DUDE_AREA_WIDTH);
   gbits_height = max(scroll_height, DUDE_MAX_HEIGHT);

   gDC = CreateMemBitmap(gbits_width, gbits_height, &gOldBitmap, &gBits);
   if (gDC == 0)
      debug(("AboutInitDialog couldn't create DC!\n"));

   timer_id = SetTimer(hDlg, TIMER_ABOUT, ABOUT_INTERVAL, NULL);

   if (state == STATE_GAME)
   {
      // Set up animated characters
      for (i=0; i < NUM_DUDES; i++)
      {
	 object_node *obj;
	 
	 obj = dudes[i].obj = ObjectGetBlank();	
	 obj->icon_res = ABOUT_RSC;
	 obj->animate->animation = ANIMATE_NONE;
	 
	 if (i == 0)
	 {
	    dudes[i].angle = 3 * NUMDEGREES / 4;
	    dudes[i].x = DUDE_X1;
	 }
	 else 
	 {
	    dudes[i].angle = NUMDEGREES / 4;
	    dudes[i].x = DUDE_X2;
	 }
      }
   }
   
   CenterWindow(hDlg, GetParent(hDlg));
   return TRUE;
}

/****************************************************************************/
void AboutTimer(HWND hwnd, UINT id)
{
   HWND hwndBitmap;
   HDC hdc;
   int i, j, k, x, y;
   RECT r;
   PDIB pdib;
   BYTE *bits, index;
   object_node *obj;

   // Copy bits to offscreen area
   for (i=0; i < scroll_height; i++)
   {
      int yBitmap = (scroll_y + i) % DibHeight(credits_pdib);
      BYTE *pSource = DibPtr(credits_pdib) + (yBitmap * DibWidth(credits_pdib));
      BYTE *pDest = gBits + i*DIBWIDTH(gbits_width);
      memcpy(pDest,pSource,scroll_width);
   }
   scroll_y++;
   if (scroll_y >= DibHeight(credits_pdib))
      scroll_y = 0;

   hwndBitmap = GetDlgItem(hwnd, IDC_SCROLL);
   hdc = GetDC(hwndBitmap);
   SelectPalette(hdc, hPal, FALSE);
   BitBlt(hdc, 0, 0, scroll_width, scroll_height, gDC, 0, 0, SRCCOPY);
   ReleaseDC(hwndBitmap, hdc);

   // Draw animated characters
   r.left = r.top = 0;
   r.right = gbits_width;
   r.bottom = gbits_height;
   FillRect(gDC, &r, GetSysColorBrush(COLOR_3DFACE));
   GdiFlush();

   hdc = GetDC(hwnd);
   for (i=0; i < NUM_DUDES; i++)
   {
      if (dudes[i].obj != NULL)
      {
	 obj = dudes[i].obj;
	 if (rand() % 30 == 0)
	 {
	    obj->animate->animation = ANIMATE_ONCE;
	    obj->animate->group = obj->animate->group_low = 3;
	    obj->animate->group_high = 4;
	    obj->animate->group_final = 0;
	    obj->animate->period = obj->animate->tick = 400;

	    if (config.play_sound)
	    {
	       switch (dudes[i].obj->icon_res)
	       {
	       case ABOUT_RSC1: index = 3; break;
	       case ABOUT_RSC2: index = 4; break;
	       case ABOUT_RSC3: index = 5; break;
	       default: index = rand() % num_sounds; break;
	       }
		   SoundPlayFile(sounds[index], SF_RANDOM_PITCH);
	    }
	 }
		
	 AnimateObject(dudes[i].obj, ABOUT_INTERVAL);
	
	 pdib = GetObjectPdib(dudes[i].obj->icon_res, dudes[i].angle, dudes[i].obj->animate->group);
	 if (pdib == NULL)
	    continue;
	
	 bits = DibPtr(pdib);
	 x = dudes[i].x - DibWidth(pdib) / 2;
	 y = DUDE_MAX_HEIGHT - DibHeight(pdib);
	 for (j=0; j < DibHeight(pdib); j++)
	 {
	    for (k=0; k < DibWidth(pdib); k++)
	    {
	       index = *(bits + j * DibWidth(pdib) + k);
	       if (index != TRANSPARENT_INDEX)
		  *(gBits + (j + y) * DIBWIDTH(gbits_width) + x + k) = index;
	    }
	 }
      }
   }
   SelectPalette(hdc, hPal, FALSE);
   BitBlt(hdc, dude_x, dude_y, DUDE_AREA_WIDTH, DUDE_MAX_HEIGHT, gDC, 0, 0, SRCCOPY);
   ReleaseDC(hwnd, hdc);
}

/****************************************************************************/
void AboutLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
#if 0
   int i, index;
   RECT rect;
   POINT p;

   GetWindowRect(GetDlgItem(hwnd, IDC_SCROLL), &rect);
   p.x = rect.left;
   p.y = rect.top;
   ScreenToClient(hwnd, &p);

   y = y - p.y + scroll_y;
   for (i=0; i < num_names; i++)
      if (y >= names[i].min_y && y <= names[i].max_y)
      {
	 index = names[i].index;
	 if (dudes[index].obj != NULL)
	    dudes[index].obj->icon_res = names[i].rsc;
      }
#else
   RECT rect;
   POINT p;

   GetWindowRect(GetDlgItem(hwnd, IDC_SCROLL), &rect);
   p.x = x;
   p.y = y;
   ClientToScreen(hwnd, &p);
   if (PtInRect(&rect,p))
   {
      credits_page++;
      if (credits_page >= credits_b.num_bitmaps)
	 credits_page = 0;
      credits_pdib = BitmapsGetPdibByIndex(credits_b, credits_page);
      scroll_y = 0;
   }
#endif
}
/****************************************************************************/
void AbortAboutDialog(void)
{
   if (hAboutDlg != NULL)
      SendMessage(hAboutDlg, WM_COMMAND, IDCANCEL, 0);
}

// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * util.c: Random utility procedures.  Many of these are exported to modules.
 */

#include "client.h"

int mem_allocated = 0;   // Count of bytes allocated - bytes freed

/************************************************************************/
/*
 * SafeMalloc:  Allocate and return given amount of memory
 *   Need to use this procedure name since util library calls it.
 */
void *SafeMalloc(int bytes)
{
   void *temp = (void *) malloc(bytes);
   if (temp == NULL)
   {
      MessageBox(hMain, "Out of memory!\nTry increasing your Windows swap file size.",
		 szAppName, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
      MainQuit(hMain);
      exit(1);
   }

   mem_allocated += bytes;

   return temp;
}

void *ZeroSafeMalloc(int bytes)
{
   BYTE *mem = (BYTE *) SafeMalloc(bytes);
   if (mem)
   {
      ZeroMemory(mem,bytes);
   }
   return (void*)mem;
}

/************************************************************************/
/*
 * SafeFree:  Free a pointer allocated by SafeMalloc.
 */
void SafeFree(void *ptr)
{
   if (ptr == NULL)
   {
      debug(("Freeing a NULL pointer!\n"));
      return;
   }

   //BUGBUG:  we don't know the size, so we can't mem_allocated -= size.

   free(ptr);
}
/************************************************************************/
/*
 * SafeRealloc:  Reallocate a SafeMalloc block and return given amount of memory
 *   Need to use this procedure name since util library calls it.
 */
void *SafeRealloc(void* pMemory, int bytes)
{
   void* temp;

   if (pMemory == NULL)
   {
      debug(("Reallocating a NULL pointer!\n"));
      return NULL;
   }

   temp = (void *)realloc(pMemory, bytes);
   if (temp == NULL)
   {
      MessageBox(hMain, "Out of memory!\nTry increasing your Windows swap file size.",
		 szAppName, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
      MainQuit(hMain);
      exit(1);
   }

   //BUGBUG:  we don't know the old size, so we can't mem_allocated -= old size.

   mem_allocated += bytes;

   return temp;
}
/************************************************************************/
char *strtolower(char *s)
{
   //REVIEW:  see strlwr() in C Runtime Library

   char *p = s;
   while (*p)
   {
      *p = tolower(*p);
      p++;
   }
   return s;
}
/********************************************************************/
void wait(long seconds)
{
   long t = time(NULL);
   while(time(NULL) < t + seconds)
      ;
}
/************************************************************************/
/*
 * set_extension: Set newfile to filename with its extension set to the
 *    given string.
 */
void set_extension(char *newfile, char *filename, char *extension)
{
   char *ptr;

   strcpy(newfile, filename);

   ptr = strrchr(newfile, '\\');  /* Find last component of path */
   if (ptr == NULL)
      ptr = newfile;

   ptr = strchr(ptr, '.');
   if (ptr != NULL)
      *ptr = '\0';
   strcat(newfile, extension);
}
/************************************************************************/
/*
 * string_hash: return a number i s.t. 0 <= i < max based on given
 *      string.  This function was stolen from Bison.
 *      Max should be odd for best results.
 */
DWORD string_hash(char *name, DWORD max_val)
{
  register char *cp;
  register DWORD k;

  cp = name;
  k = 0;
  while (*cp)
    k = ((k << 1) ^ (*cp++)) & 0x3fff;

   return k % max_val;
}
/************************************************************************/
/*
 * MakeDirectory:  Create directory of given name, if it doesn't already exist.
 *   Returns True if directory exists or was successfully created.
 */
Bool MakeDirectory(char *name)
{
   struct stat s;

   // If file already exists, OK only if it's a directory
   if (stat(name, &s) == 0)
      return (s.st_mode & S_IFDIR) != 0;

   // If file doesn't exist, OK only if it can be created
   return mkdir(name) == 0;
}
/************************************************************************/
char *GetLastErrorStr(void)
{
   char *error_str;

   error_str = GetString(hInst, IDS_UNKNOWN); /* in case the call  fails */

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,GetLastError(),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                 (LPTSTR) &error_str,0,NULL);
   return error_str;
}
/*****************************************************************************/
/*
 * CenterWindow: Center one window over another.  Also ensure that the
 *   centered window (hwnd) is completely on the screen.
 *   Call when processing the WM_INITDIALOG message of dialogs, or
 *   WM_CREATE in WndProcs.
 */
void CenterWindow(HWND hwnd, HWND hwndParent)
{
   RECT rcDlg, rcParent;
   int x, y;
   //int screen_width, screen_height;

   /* If dialog has no parent, then its parent is really the desktop */
   if (hwndParent == NULL)
      hwndParent = GetDesktopWindow();

   GetWindowRect(hwndParent, &rcParent);
   GetWindowRect(hwnd, &rcDlg);

   /* Move dialog rectangle to upper left (0, 0) for ease of calculation */
   OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);

   x = rcParent.left + (rcParent.right - rcParent.left)/2 - rcDlg.right/2;
   y = rcParent.top + (rcParent.bottom - rcParent.top)/2 - rcDlg.bottom/2;

   // Disabled checking for compatibility with multi-monitor setups, since
   // the values for x and y can be negative (non-primary monitor on left).
   // Make sure that child window is completely on the screen
   //screen_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   //screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
   //x = max(0, min(x, screen_width  - rcDlg.right));
   //y = max(0, min(y, screen_height - rcDlg.bottom));

   SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}
/************************************************************************/
/*
 * EditBoxScroll:  Scroll contents of an edit box so that its last line is
 *   scrolled into view at the bottom of the box.  This procedure makes
 *   the contents of the edit box flash, so surround calls to it with
 *   WindowBeginUpdate/WindowEndUpdate.
 *
 *   When scrollback is True, the edit box is scrolled back from the bottom
 *   of the edit box; when it is False, the edit box is scrolled to the bottom
 *   of the edit box.  For some unknown reason, under Windows 95, scrolling an edit
 *   box to the bottom does NOT put the last line of text at the top of the window,
 *   though the documentation says it does.
 * NOTE:  Assumes that the edit box has a constant font height.
 */
void EditBoxScroll(HWND hEdit, Bool scrollback)
{
   RECT r;
   int screen_lines, new_lines, current_line;
   HFONT hFont;

   Edit_GetRect(hEdit, &r);

   // Get font of edit box
   hFont = GetWindowFont(hEdit);
   if (hFont == NULL)
     hFont = GetFont(FONT_EDIT);

   /* Calculate # of visible lines in box, by dividing edit's height by font height */
   screen_lines = (r.bottom - r.top) / GetFontHeight(hFont);

   new_lines = Edit_GetLineCount(hEdit);

   if (new_lines > screen_lines)
   {
      /* First scroll to the end, then go to correct position */
      Edit_Scroll(hEdit, new_lines, 0);
      if (scrollback)
      {
         current_line = Edit_GetFirstVisibleLine(hEdit);
         Edit_Scroll(hEdit, (new_lines-screen_lines) - current_line, 0);
      }
   }
}
/************************************************************************/
/*
 * EditBoxLastVisible:  Return True iff the last line of text in the given edit
 *   box is visible.
 * NOTE:  Assumes that the edit box has a constant font height.
 */
Bool EditBoxLastVisible(HWND hEdit)
{
   RECT r;
   int screen_lines, num_lines, first_line;
   HFONT hFont;

   Edit_GetRect(hEdit, &r);

   // Get font of edit box
   hFont = GetWindowFont(hEdit);
   if (hFont == NULL)
     hFont = GetFont(FONT_EDIT);

   /* Calculate # of visible lines in box, by dividing edit's height by font height */
   screen_lines = (r.bottom - r.top) / GetFontHeight(hFont);

   first_line = Edit_GetFirstVisibleLine(hEdit);

   num_lines = Edit_GetLineCount(hEdit);

   if (first_line + screen_lines >= num_lines - 1)
      return True;
   return False;
}
/*****************************************************************************/
/*
 * HasExtension:  Return True iff given filename (with path) has given extension.
 *   extension should not contain a period.
 */
Bool HasExtension(char *filename, char *extension)
{
   char *ptr, *ext;

   if (filename == NULL || extension == NULL)
      return False;

   /* Find last component of path */
   ptr = strrchr(filename, '\\');
   if (ptr == NULL)
      ext = strchr(filename, '.');
   else ext = strchr(ptr + 1, '.');

   if (ext == NULL)
      return False;
   return !strcmp(ext + 1, extension);
}
/*****************************************************************************/
/*
 * GetWorkingDirectory:  Fill given buffer with current working directory, which
 *   will end with a backslash.  If directory name doesn't fit in buffer, set
 *   buffer to empty string and return False, else return True.
 */
Bool GetWorkingDirectory(char *buf, int buflen)
{
   int len;

   if (getcwd(buf, buflen) == NULL)
   {
      buf[0] = 0;
      return False;
   }

   // Add backslash to end of dir if not already there
   len = strlen(buf);
   if (buf[len - 1] != '\\')
   {
      buf[len] = '\\';
      buf[len + 1] = 0;
   }
   return True;
}
/*****************************************************************************/
/*
 * AreaToRect:  Convert an AREA structure to a RECT structure.
 */
void AreaToRect(AREA *a, RECT *r)
{
   r->left   = a->x;
   r->top    = a->y;
   r->right  = a->x + a->cx;
   r->bottom = a->y + a->cy;
}
/*****************************************************************************/
/*
 * RectToArea:  Convert a RECT structure to an AREA structure.
 */
void RectToArea(RECT *r, AREA *a)
{
   a->x  = r->left;
   a->y  = r->top;
   a->cx = r->right - r->left;
   a->cy = r->bottom - r->top;
}
/*****************************************************************************/
/*
 * UnionArea:  Computes the union of a1 and a2, and puts it in a.
 */
void UnionArea(AREA *a, AREA *a1, AREA *a2)
{
   RECT r1, r2, r;

   AreaToRect(a1, &r1);
   AreaToRect(a2, &r2);
   UnionRect(&r, &r1, &r2);
   RectToArea(&r, a);
}
/*****************************************************************************/
/*
 * IsInArea:  Return True iff point is in area.
 */
Bool IsInArea(AREA *area, int x, int y)
{
   return (x >= area->x && x < area->x + area->cx &&
	   y >= area->y && y < area->y + area->cy);
}
/************************************************************************/
/*
 * GetPartnerCode:  Grab a number from a text file if it's present.
 *                  Different companies that distribute clients have
 *                  different codes.
 */
int GetPartnerCode(void)
{
   FILE* pFile;
   char achBuffer[20];

   pFile = fopen("partner.txt", "r");
   if (!pFile)
      return 0;

   if (!fgets(achBuffer, sizeof(achBuffer), pFile))
   {
      fclose(pFile);
      return 0;
   }

   fclose(pFile);

   achBuffer[sizeof(achBuffer)-1] = '\0';

   return (atoi(achBuffer) & 0xFF);
}
/************************************************************************/
/*
 * GetSystemStats:  Fill in given structure with info about the computer.
 */
#define DISP_640x480	0x01
#define DISP_800x600	0x02
#define DISP_1024x768	0x04
#define DISP_1152x882	0x08
#define DISP_1280x1024	0x10
#define DISP_1600x1200	0x20
#define DISP_HIGHER	0x80
#define DISP_MASK	0xff
#define DISP_8BIT_SHIFT	   0
#define DISP_16BIT_SHIFT   8
#define DISP_24BIT_SHIFT   16
#define DISP_32BIT_SHIFT   32
#define DISP_8BIT	1
#define DISP_15BIT	2
#define DISP_16BIT	3
#define DISP_24BIT	4
#define DISP_32BIT	5

/* BoundsChecker Claims this overruns in EnumDisplaySettings */
/* so since its a windows call thats supposed to be doing it i`ll add */
/* so dummy memory after it, ie the second devmode is not used ! */

static DEVMODE devMode[2];

extern unsigned short gCRC16;

void GetSystemStats(SystemInfo *s)
{
   OSVERSIONINFO version;
   MEMORYSTATUS mem;
   SYSTEM_INFO sys;
   int iModeNum;
   DWORD display = 0;
   unsigned long crc32;
   HDC dc;

   version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&version);

   mem.dwLength = sizeof(MEMORYSTATUS);
   GlobalMemoryStatus(&mem);

   GetSystemInfo(&sys);

   crc32 = gCRC16;

   s->platform = version.dwPlatformId;
   s->platform_minor = version.dwMinorVersion;
   s->platform_major = version.dwMajorVersion;
   s->memory = mem.dwTotalPhys;
   s->chip = (crc32<<16)|sys.dwProcessorType;
   iModeNum = 0;
   while (EnumDisplaySettings(NULL,iModeNum,&devMode[0]))
   {
      int shift = 0;
      int disp = 0;
      switch (devMode[0].dmBitsPerPel) {
      case 8: shift = DISP_8BIT_SHIFT; break;
      case 15: case 16: shift = DISP_16BIT_SHIFT; break;
      case 24: shift = DISP_24BIT_SHIFT; break;
      case 32: shift = DISP_32BIT_SHIFT; break;
      default: shift = DISP_8BIT_SHIFT; break;
      }
      switch (devMode[0].dmPelsWidth) {
      case 640: disp = DISP_640x480; break;
      case 800: disp = DISP_800x600; break;
      case 1024: disp = DISP_1024x768; break;
      case 1152: disp = DISP_1152x882; break;
      case 1280: disp = DISP_1280x1024; break;
      case 1600: disp = DISP_1600x1200; break;
      default:
	 if (devMode[0].dmPelsWidth > 1600)
	    disp = DISP_HIGHER;
	 break;
      }
      display |= disp << shift;
      iModeNum++;
   }
   s->color_depth = display;
   s->bandwidth = SIBW_UNKNOWN;
   dc = GetDC(GetDesktopWindow());
   s->screen_width  = GetSystemMetrics(SM_CXSCREEN);
   s->screen_height = GetSystemMetrics(SM_CYSCREEN);

   s->reserved = GetDeviceCaps(dc,BITSPIXEL); //devMode[0].dmBitsPerPel;
   s->reserved &= 0xFF;

   s->reserved |= (GetPartnerCode() << 8);
   s->reserved &= 0xFFFF;

   ReleaseDC(GetDesktopWindow(),dc);
}
/*****************************************************************************/
/*
 * ResizeDialogItem:  Resize an item in a dialog box when the dialog box itself
 *   is resized.
 *
 *   hDlg is the handle of the dialog.
 *   hItem is the handle of the dialog item.
 *   old_rect gives the screen rectangle of the dialog before it was resized.
 *   flags describes how the dialog item is to be resized (values are ORed together):
 *     RDI_LEFT:    Left side of dialog item resizes to fit dialog
 *     RDI_RIGHT:   Right side of dialog item resizes to fit dialog
 *     RDI_TOP:     Top side of dialog item resizes to fit dialog
 *     RDI_BOTTOM:  Bottom side of dialog item resizes to fit dialog
 *     RDI_ALL:     All 4 sides resize to fit dialog
 *     RDI_HCENTER: Center dialog item horizontally
 *     RDI_VCENTER: Center dialog item vertically
 *     RDI_HPIN:    Keep horiz. center of dialog item same distance from horiz. center of dialog
 *     RDI_VPIN:    Keep vert. center of dialog item same distance from vert. center of dialog
 *   redraw tells if dialog item should be redrawn
 */
void ResizeDialogItem(HWND hDlg, HWND hItem, RECT *old_rect, int flags, Bool redraw)
{
   RECT dlg_rect, item_rect;
   POINT p;
   RECT new_rect;   // New screen rectangle of dialog item
   int x_increase, y_increase, xsize, ysize, dist;

   GetWindowRect(hDlg, &dlg_rect);
   GetWindowRect(hItem, &item_rect);

   x_increase = (dlg_rect.right - dlg_rect.left) - (old_rect->right - old_rect->left);
   y_increase = (dlg_rect.bottom - dlg_rect.top) - (old_rect->bottom - old_rect->top);

   /* Try to keep dialog item from getting too big after window obscures it */
   if (y_increase > 0)
      y_increase = max(0, min(y_increase, dlg_rect.bottom - item_rect.bottom));

   if (x_increase > 0)
      x_increase = max(0, min(x_increase, dlg_rect.right - item_rect.left));

   if (flags & RDI_LEFT)
      new_rect.left = item_rect.left;
   else new_rect.left = item_rect.left + x_increase;

   if (flags & RDI_RIGHT)
      new_rect.right = item_rect.right + x_increase;
   else new_rect.right = item_rect.right;

   if (flags & RDI_TOP)
      new_rect.top = item_rect.top;
   else new_rect.top = item_rect.top + y_increase;

   if (flags & RDI_BOTTOM)
      new_rect.bottom = item_rect.bottom + y_increase;
   else new_rect.bottom = item_rect.bottom;

   if (flags & RDI_HCENTER)
   {
      xsize = new_rect.right - new_rect.left;
      new_rect.left = (dlg_rect.left + dlg_rect.right - xsize) / 2;
      new_rect.right = new_rect.left + xsize;
   }

   if (flags & RDI_VCENTER)
   {
      ysize = new_rect.bottom - new_rect.top;
      new_rect.top = (dlg_rect.top + dlg_rect.bottom - ysize) / 2;
      new_rect.bottom = new_rect.top + ysize;
   }

   if (flags & RDI_HPIN)
   {
      xsize = new_rect.right - new_rect.left;
      dist = (old_rect->left + old_rect->right) / 2 - (item_rect.left + item_rect.right) / 2;
      new_rect.left = (dlg_rect.left + dlg_rect.right) / 2 - dist - xsize / 2;
      new_rect.right = new_rect.left + xsize;
   }

   if (flags & RDI_VPIN)
   {
      ysize = new_rect.bottom - new_rect.top;
      dist = (old_rect->top + old_rect->bottom) / 2 - (item_rect.top + item_rect.bottom) / 2;
      new_rect.top = (dlg_rect.top + dlg_rect.bottom) / 2 - dist - ysize / 2;
      new_rect.bottom = new_rect.top + ysize;
   }

   /* Find origin of dialog item in client coordinates */
   p.x = new_rect.left;
   p.y = new_rect.top;
   ScreenToClient(hDlg, &p);
   MoveWindow(hItem, p.x, p.y,
	      new_rect.right - new_rect.left, new_rect.bottom - new_rect.top, redraw);
}
/*****************************************************************************/
/*
 * ResizeDialog:  Given dialog has been resized; move its child controls around.
 *   dlg_rect gives old rectangle of dialog.
 *   children is a table telling how to move each child control.
 *     (table must end with an entry whose id field is 0).
 *
 *   Modifies dlg_rect to contain dialog's new rectangle.
 */
void ResizeDialog(HWND hDlg, RECT *dlg_rect, ChildPlacement *children)
{
   int i;
   HWND hCtrl;

   // Move child windows around
   for (i = 0; children[i].id != 0; i++)
   {
      hCtrl = GetDlgItem(hDlg, children[i].id);
      if (GetWindowLong(hCtrl, GWL_STYLE) & WS_VISIBLE)
	 ResizeDialogItem(hDlg, GetDlgItem(hDlg, children[i].id), dlg_rect,
			  children[i].placement, False);
   }
   InvalidateRect(hDlg, NULL, TRUE);
   GetWindowRect(hDlg, dlg_rect);
}
/********************************************************************/
/*
 * MenuFindItemByName:  Utility function to find menu item based on string name.
 *   Returns (0-based) index of item in given menu, or -1 if not found.
 */
int MenuFindItemByName(HMENU hMenu, char *name)
{
   int i, num, len;
   char item_name[MAXRSCSTRING + 1];

   // Find menu item by name
   num = GetMenuItemCount(hMenu);
   for (i = 0; i < num; i++)
   {
      len = GetMenuString(hMenu, i, item_name, MAXRSCSTRING, MF_BYPOSITION);
      if (len == 0)
	 continue;
      if (!strcmp(item_name, name))
	 return i;
   }
   return -1;
}
/********************************************************************/
/*
 * GetCRC16:  Return 16 bit CRC of given buffer with given length.
 */
WORD GetCRC16(char *buf, int length)
{
   DWORD crc = GetCRC32(buf, length);
   return (WORD) (crc & 0xffff);
}
/********************************************************************/
/*
 * GetCRC32:  Return 32 bit CRC of given buffer with given length.
 */
DWORD GetCRC32(char *buf, int length)
{
   return CRC32(buf, length);
}
/********************************************************************/
/*
 * GetBitmapResource:  Return pointer to bitmap with given Windows resource id in given
 *   module, or NULL if none.
 */
BITMAPINFOHEADER *GetBitmapResource(HMODULE hModule, int bitmap_id)
{
   HRSRC hrsrc;
   HGLOBAL hglobal;
   BITMAPINFOHEADER *ptr;

   // Get pointer to bits of bitmap
   hrsrc = FindResource(hModule, MAKEINTRESOURCE(bitmap_id), RT_BITMAP);
   if (hrsrc == NULL)
   {
      debug(("GetBitmapResource failed to find bitmap %d\n", bitmap_id));
      return NULL;
   }

   hglobal = LoadResource(hModule, hrsrc);
   if (hglobal == NULL)
   {
      debug(("GetBitmapResource failed to load bitmap %d\n", bitmap_id));
      return NULL;
   }

   ptr = (BITMAPINFOHEADER *) LockResource(hglobal);
   if (ptr == NULL)
   {
      debug(("GetBitmapResource failed to lock bitmap %d\n", bitmap_id));
      return NULL;
   }

   return ptr;
}
/********************************************************************/
/*
 * GetBitmapResourceInfo:  Load bitmap from resources and fill in RawBitmap structure.
 *   Returns True on success.
 */
Bool GetBitmapResourceInfo(HMODULE hModule, int bitmap_id, RawBitmap *b)
{
  BITMAPINFOHEADER *ptr;

  ptr = GetBitmapResource(hModule, bitmap_id);
  if (ptr == NULL)
    return False;

  b->bits   = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
  b->width  = ptr->biWidth;
  b->height = ptr->biHeight;
  return True;
}
/********************************************************************/
/*
 * InitMenuPopupHandler:  Disable the move and size items on the given menu.
 *   We need to do this because the mouse capture during these
 *   commands interrupts all our timers, causing the server to time us out.
 */
void InitMenuPopupHandler(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
   if (fSystemMenu)
   {
      EnableMenuItem(hMenu, SC_MOVE, MF_GRAYED);
      EnableMenuItem(hMenu, SC_SIZE, MF_GRAYED);
   }
}

/********************************************************************/
/*
 * GetHBitmapFromResource:  Creates a DIB from resources. Returns handle to new DIB.
 *   Added by ajw.
 *   xxx Not sure if this works or is useful for anything. Ended up not using it.
 */
HBITMAP GetHBitmapFromResource( HMODULE hModule, int bitmap_id )
{
	HBITMAP hbmpReturn;
	BITMAPINFOHEADER* pbmiHeader = GetBitmapResource( hModule, bitmap_id );
	HDC hDC = GetDC( hMain );
	BITMAPINFO* pbmInfo = (BITMAPINFO*)SafeMalloc( sizeof(BITMAPINFO) + NUM_COLORS * sizeof(RGBQUAD) );
	memcpy( &pbmInfo->bmiHeader, pbmiHeader, sizeof(BITMAPINFOHEADER) );
	SetBMIColors( pbmInfo );
	hbmpReturn = CreateDIBitmap( hDC, pbmiHeader, CBM_INIT,
									((BYTE*)pbmiHeader) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD),
									pbmInfo, DIB_PAL_COLORS );
	ReleaseDC( hMain, hDC );
	return hbmpReturn;
}

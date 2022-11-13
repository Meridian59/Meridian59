// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* util.h
 * Header file for util.c
 */

#ifndef _UTIL_H
#define _UTIL_H

/* Flags for ResizeDialogItem */
#define RDI_LEFT    0x00000001
#define RDI_RIGHT   0x00000002
#define RDI_TOP     0x00000004
#define RDI_BOTTOM  0x00000008
#define RDI_HCENTER 0x00000010
#define RDI_VCENTER 0x00000020
#define RDI_HPIN    0x00000040
#define RDI_VPIN    0x00000080
#define RDI_ALL    (RDI_LEFT | RDI_RIGHT | RDI_TOP | RDI_BOTTOM)

typedef struct {
  BYTE *bits;
  int   width;
  int   height;
} RawBitmap;

typedef struct {
   int x, y;  
   int cx, cy;      
} AREA;

// How to move child windows around when dialogs resize
typedef struct {
   int id;          // Id of child control
   int placement;   // ResizeDialogItem() placement flags
} ChildPlacement;

#define SICD_UNKNOWN	(0)
#define SICD_256		(8)
#define SICD_HICOLOR	(16)
#define SICD_TRUECOLOR	(24)

#define SIBW_UNKNOWN	(0)
#define SIBW_144KBPS	(14)
#define SIBW_288KBPS	(28)
#define SIBW_336KBPS	(33)
#define SIBW_560KBPS	(56)
#define SIBW_10MBPS		(1000)

// Info on computer running game
typedef struct {
   int  platform;           // Operating system identifier
   int  platform_major;     // Major version of OS
   int  platform_minor;     // Minor version of OS
   int  memory;             // Amount of physical memory, in bytes
   int chip;                // Chip type
   WORD screen_width;
   WORD screen_height;
   int color_depth;
   int bandwidth;
   int reserved;
} SystemInfo;

char *strtolower(char *);
void  wait(long seconds);
DWORD string_hash(char *name, DWORD max_val);
void  set_extension(char *newfile, char *filename, char *extension);
M59EXPORT Bool  MakeDirectory(char *name);
Bool  GetWorkingDirectory(char *buf, int buflen);
char *GetLastErrorStr(void);
M59EXPORT void  CenterWindow(HWND hDlg, HWND hwndParent);
M59EXPORT void  EditBoxScroll(HWND hEdit, Bool scrollback);
Bool  EditBoxLastVisible(HWND hEdit);
Bool  HasExtension(char *filename, char *extension);
M59EXPORT void  AreaToRect(AREA *a, RECT *r);
M59EXPORT void  RectToArea(RECT *r, AREA *a);
M59EXPORT void  UnionArea(AREA *a, AREA *a1, AREA *a2);
M59EXPORT Bool  IsInArea(AREA *a, int x, int y);
void  GetSystemStats(SystemInfo *s);
M59EXPORT void  ResizeDialogItem(HWND hDlg, HWND hItem, RECT *old_rect, int flags, Bool redraw);
M59EXPORT void  ResizeDialog(HWND hDlg, RECT *dlg_rect, ChildPlacement *children);
M59EXPORT int   MenuFindItemByName(HMENU hMenu, char *name);
WORD  GetCRC16(char *buf, int length);
DWORD  GetCRC32(char *buf, int length);
M59EXPORT BITMAPINFOHEADER *GetBitmapResource(HMODULE hModule, int bitmap_id);
M59EXPORT Bool GetBitmapResourceInfo(HMODULE hModule, int bitmap_id, RawBitmap *b);
M59EXPORT void InitMenuPopupHandler(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu);

M59EXPORT void *SafeMalloc(int bytes);
M59EXPORT void *ZeroSafeMalloc(int bytes);
M59EXPORT void *SafeRealloc(void* pMemory, int bytes);
M59EXPORT void SafeFree(void *pMemory);

#endif /* #ifndef _UTIL_H */

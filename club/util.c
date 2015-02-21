// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * util.c:  Utility functions for club.
 */

#include "club.h"

#define MAXSTRINGLEN 512

void CenterWindow(HWND hwnd, HWND hwndParent)
{
   RECT rcDlg, rcParent;
   int screen_width, screen_height, x, y;
   
   /* If dialog has no parent, then its parent is really the desktop */
   if (hwndParent == NULL)
      hwndParent = GetDesktopWindow();
   
   GetWindowRect(hwndParent, &rcParent);
   GetWindowRect(hwnd, &rcDlg);
   
   /* Move dialog rectangle to upper left (0, 0) for ease of calculation */
   OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);

   x = rcParent.left + (rcParent.right - rcParent.left)/2 - rcDlg.right/2;
   y = rcParent.top + (rcParent.bottom - rcParent.top)/2 - rcDlg.bottom/2;

   
   /* Make sure that child window is completely on the screen */
   screen_width  = GetSystemMetrics(SM_CXSCREEN);
   screen_height = GetSystemMetrics(SM_CYSCREEN);
   
   x = max(0, min(x, screen_width  - rcDlg.right));
   y = max(0, min(y, screen_height - rcDlg.bottom));

   SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
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

/******************************************************************************/
/*
 * GetString:  Load and return resource string with given resource identifier.
 *             String is loaded from given module's resources.
 * NOTE:  Only use 4 strings from this function at a time; it uses a circular
 *        buffer of static strings!!
 */
char *GetString(HMODULE hModule, int idnum)
{
   static int index = 0;
   static char szLoadedString[4][MAXSTRINGLEN];

   index = (index + 1) % 4;
   
   szLoadedString[index][0] = 0; 
   LoadString (hModule, idnum, szLoadedString[index], MAXSTRINGLEN);
   return (LPSTR)szLoadedString[index];
}


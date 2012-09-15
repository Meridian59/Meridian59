// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * hook.c:  Set keyboard hook to receive all messages from anywhere in the program.
 *   This is used to intercept the F1 key for help.
 */

#include "client.h"

static HHOOK hHook;   // Next hook in calling chain

static LRESULT CALLBACK MainKeyHook(int code, WPARAM wParam, LPARAM lParam);
/************************************************************************/
/*
 * HookInit:  Set hook to capture keyboard events.
 */
void HookInit(HINSTANCE hInst)
{
   HANDLE hTask;

   hTask = (HANDLE) GetCurrentThreadId();

   if (hTask == NULL)
      return;

   hHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC) MainKeyHook, NULL, (DWORD) hTask);
   if (hHook == NULL)
      MessageBeep(1);
}
/************************************************************************/
/*
 * HookClose:  Remove hook to capture keyboard events.
 */
void HookClose(void)
{
   UnhookWindowsHookEx(hHook);   
}
/************************************************************************/
LRESULT CALLBACK MainKeyHook(int code, WPARAM wParam, LPARAM lParam)
{
   if (code == HC_ACTION)
   {
      UINT vk     = wParam;
      int cRepeat = (int)(short)LOWORD(lParam);
      UINT flags  = (UINT)HIWORD(lParam);
      int down    = !(flags & 0xc000);
      
/*
      if (vk == VK_F1 && down)
	 StartHelp();
*/
   }
   return CallNextHookEx(hHook, code, wParam, lParam);
}

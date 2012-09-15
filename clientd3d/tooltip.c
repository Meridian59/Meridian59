// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * tooltip.c:  Handle tooltips.  Callers can set up a tooltip for a rectangular region
 *   on the main window.
 */

#include "client.h"

static HWND    hToolTips;     // Handle of tooltips window (invisible)

/****************************************************************************/
/*
 * TooltipCreate:  Create tooltip control.
 */
void TooltipCreate(void)
{
   hToolTips = CreateWindow(TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP, 
			    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
			    hMain, (HMENU) NULL, hInst, NULL); 
}
/****************************************************************************/
/*
 * TooltipDestroy:  Destroy tooltip control.
 */
void TooltipDestroy(void)
{
   DestroyWindow(hToolTips);
   hToolTips = NULL;
}
/****************************************************************************/
/*
 * TooltipForwardMessage:  Forward appropriate messages to tooltips control.
 */
void TooltipForwardMessage(MSG *msg)
{
   if (config.tooltips && hToolTips != NULL)
     SendMessage(hToolTips, TTM_RELAYEVENT, 0, (LPARAM) msg);
}
/****************************************************************************/
/*
 * TooltipAddRectangle:  Create a tooltip for the given rectangle on hwnd.
 */
void TooltipAddRectangle(HWND hwnd, RECT *rect, HINSTANCE hModule, int name)
{
   TOOLINFO ti;

   ti.cbSize = sizeof(TOOLINFO); 
   ti.uFlags = 0; 
   ti.hwnd = hwnd; 
   ti.hinst = hModule; 
   memcpy(&ti.rect, rect, sizeof(RECT));
   ti.lpszText = (LPTSTR) name; 
   
   if (!SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti))
      debug(("Creation of tooltip failed\n"));
}
/****************************************************************************/
/*
 * TooltipAddWindow:  Create a tooltip for the given window.
 */
void TooltipAddWindow(HWND hwnd, HINSTANCE hModule, int name)
{
   TOOLINFO ti;

   ti.cbSize = sizeof(TOOLINFO); 
   ti.uFlags = TTF_IDISHWND; 
   ti.hwnd = hwnd; 
   ti.uId = (UINT) hwnd; 
   ti.hinst = hModule; 
   ti.lpszText = MAKEINTRESOURCE(name); 
   
   if (!SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti))
      debug(("Creation of tooltip failed\n"));
}
/****************************************************************************/
/*
 * TooltipAddWindowCallback:  Create a tooltip for the given window.
 */
void TooltipAddWindowCallback(HWND hwnd, HINSTANCE hModule)
{
   TOOLINFO ti;

   ti.cbSize = sizeof(TOOLINFO); 
   ti.uFlags = TTF_IDISHWND; 
   ti.hwnd = hwnd; 
   ti.uId = (UINT) hwnd; 
   ti.hinst = hModule; 
   ti.lpszText = LPSTR_TEXTCALLBACK;
   
   if (!SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti))
      debug(("Creation of tooltip failed\n"));
}
/****************************************************************************/
/*
 * TooltipGetControl:  Returns tooltip control.
 */
HWND TooltipGetControl(void)
{
   return hToolTips;
}
/****************************************************************************/
/*
 * TooltipReset:  When a modal dialog box is displayed, window messages from the
 *   dialog's parent (e.g. the main window) don't get relayed to the tooltip, since
 *   the dialog handles them.  This causes the tooltip to get a mouse down event, but
 *   no mouse up event, which makes the tooltips break.  Here we simulate the mouse
 *   up events to fake out the tooltip after a dialog is dismissed.
 */
void TooltipReset(void)
{
  MSG msg;
  
  msg.hwnd = hMain;
  msg.message = WM_LBUTTONUP;
  SendMessage(hToolTips, TTM_RELAYEVENT, TRUE, (LPARAM) &msg);
  msg.message = WM_RBUTTONUP;
  SendMessage(hToolTips, TTM_RELAYEVENT, TRUE, (LPARAM) &msg);
}

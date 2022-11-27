// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * statgame.c:  Dispatch window messages while in game mode.
 *
 */

#include "client.h"

/* Current state of game--selecting object, picking character, etc. */
static int game_state = GAME_NONE;

extern int main_viewport_height;
extern int main_viewport_width;

bool text_area_resize_zone = false;
bool text_area_resize_inprogress = false;

int text_area_resize_start_x = 0;
int text_area_resize_start_y = 0;

/****************************************************************************/
void GameInit(void)
{
   LoadProfaneTerms();

   // Set period of multimedia timer
   timeBeginPeriod(1);

   /* clear the window for our use */
   InvalidateRect(hMain,NULL,TRUE); 
   UpdateWindow(hMain);

   InitializeGame();

   EnableGameMenus();

   KeyInit();
   DrawInitialize();
   SoundInitialize();
   MapFileInitialize();

   GameSetState(GAME_INVALID);

   LogoffTimerStart();
   PingTimerStart();

   /* Redraw background so we get texture */
   InvalidateRect(hMain, NULL, TRUE);
}
/****************************************************************************/
void GameExit(void)
{
   timeEndPeriod(1);

   ModulesExitGame();
   
   GameSetState(GAME_NONE);

   AnimationTimerAbort();
   LogoffTimerAbort();
   PingTimerAbort();

   // Can't close modules here, because WM_DESTROY messages sent to dialogs causes crash.
   // Modules are freed elsewhere.

   SoundClose();
   MusicAbort();

   KeyClose();
   InterfaceClose();
   CloseGame();
   DrawClose();
   DisableGameMenus();
   MapFileClose();

   AbortStartupTimer();

   SetMainCursor(LoadCursor(NULL, IDC_ARROW));

   /* Redraw background so we remove texture */
   InvalidateRect(hMain, NULL, TRUE);
}

/************************************************************************/
void GameSetState(int new_state)
{
   int old_state = game_state;

   /* First exit old state */
   switch (old_state)
   {
   case GAME_PLAY:
      break;

   case GAME_WAIT:
   case GAME_INVALID:
   case GAME_INIT:
   case GAME_SELECT:
   case GAME_RESYNC:
      break;
   }

   /* Now enter new state */
   debug(("old game state = %d\n", game_state));
   game_state = new_state;
   debug(("new game state = %d\n", game_state));

   GameWindowSetCursor();
   
   switch (new_state)
   {
   case GAME_INIT:
      InterfaceInitialize(hMain);
      ModulesEnterGame();   // Must call this after initializing game structures
      
      ResetUserData();
      GameSetState(GAME_INVALID);
      break;

   case GAME_WAIT:
   case GAME_INVALID:
      break;

   case GAME_PLAY:
      break;

   case GAME_SELECT:
      break;

   case GAME_RESYNC:
      StartupInit(STATE_GAME);
      break;      
   }

   // Tell modules about state change
   ModuleEvent(EVENT_STATECHANGED, old_state, new_state);
}
/************************************************************************/
int GameGetState(void)
{
   return game_state;
}
/****************************************************************************/
void GameExpose(HWND hwnd)
{
   HDC hdc;
   PAINTSTRUCT ps;
   
   hdc = BeginPaint(hwnd, &ps);

   DrawWindowBackground(hdc, &ps.rcPaint, ps.rcPaint.left, ps.rcPaint.top);
   InterfaceRedraw(hdc);

   EndPaint(hwnd, &ps);
}
/****************************************************************************/
/* Return True iff message should NOT be passed to Windows for default processing */
Bool GameKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   Bool held_down = (flags & 0x4000) ? True : False;  /* Is key being held down? */

   UserDidSomething();   /* User is alive! */

   if (!fDown)
   {
      KeyUpHack(vk);   // Work around Windows bug
      return False;
   }

   /* Since mouse clicks are handled in separate procedure, throw them out here */
   if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON)
      return False;

   if (cRepeat > 1 || held_down)
      // Stop Windows from going to menu when F10 pressed
      if (vk == VK_F10)
	 return True;
      else return False;

   HandleKeys();    // Handle key right away; don't wait for idle processing to get it

   /* 
    *  Special case for ALT key:  since the key is used for movement, it would
    *  often bring up menus when we don't want them.  So we only pass it on to 
    *  Windows when no other key is down.
    */

   if (vk == VK_MENU && KeysDown() > 1)
      return True;

   // Stop Windows from going to menu when F10 pressed
   if (vk == VK_F10)
      return True;

   return False;
}
/****************************************************************************/
void GameMouseButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   UserDidSomething();   /* User is alive! */

   SetFocus(hMain);

   if (text_area_resize_zone) {
       // Start text area resize.
       text_area_resize_start_x = x;
       text_area_resize_start_y = y;
       text_area_resize_inprogress = true;
   }

}
/****************************************************************************/
void GameLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (text_area_resize_inprogress) {
        // End text area resize.
        text_area_resize_zone = false;
        text_area_resize_inprogress = false;
    }

    UserEndDrag();
}
/****************************************************************************/
void GameMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    RECT rcToolbar;
    ToolbarGetUnionRect(&rcToolbar);

    /* Resize the text area is possible from the left hand edge of the client window,
    in-between the viewport and textarea. The vertical resize cursor will be displayed
    at this time. */
    int offset = 5;
    if (text_area_resize_inprogress ||
        (y >= main_viewport_height + rcToolbar.bottom + EDGETREAT_HEIGHT- offset
        && y <= main_viewport_height + EDGETREAT_HEIGHT + rcToolbar.bottom + offset
        && x <= EDGETREAT_HEIGHT + offset))
    {
        MainSetCursor(hwnd, hwnd, HTTOP, 0);
        text_area_resize_zone = true;
    }
    else
    {
        text_area_resize_zone = false;
        text_area_resize_inprogress = false;
    }

    if (text_area_resize_inprogress)
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Calculate new text area size.
        int delta_x = text_area_resize_start_x - x;
        int delta_y = text_area_resize_start_y - y;
        int new_text_area_height = (((float)config.text_area_size / 100.0) * height) + delta_y;

        // Define limits of the text area size as a perecentage of the client size.
        int textarea_min_height = height * TEXT_AREA_HEIGHT_MIN;
        int textarea_max_height = height * TEXT_AREA_HEIGHT_MAX;

        new_text_area_height = max(textarea_min_height, new_text_area_height);
        new_text_area_height = min(textarea_max_height, new_text_area_height);

        // Save updated text area size and apply changes.
        int text_area_height_percentage = ((float)new_text_area_height / (float)height) * 100.0;
        if (config.text_area_size != text_area_height_percentage) {
            config.text_area_size = text_area_height_percentage;
            debug(("Resizing text area to %d\n", config.text_area_size));
            InterfaceResize(width, height);
            RedrawAll();
        }
    }
}
/****************************************************************************/
void GameMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
   UserDidSomething();
   // If menu just disappeared, stop animation timer
   AnimationTimerAbort();    // In case animation timer is going in background
}
/****************************************************************************/
void GameVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
}
/****************************************************************************/
void GameMove(HWND hwnd, int x, int y)
{
}
/****************************************************************************/
void GameResize(HWND hwnd, UINT resize_flag, int xsize, int ysize)
{
   InterfaceResize(xsize, ysize);
}
/****************************************************************************/
void GameSetFocus(HWND hwnd, HWND hwndOldFocus)
{
   AnimationTimerAbort();
   DrawGridBorder();
   RedrawAll();
   // After a dialog closes, block keys for a short time to avoid interpreting
   // the key that closed the dialog as a new command.
   KeySetLastNorepeatTime();
}
/****************************************************************************/
void GameKillFocus(HWND hwnd, HWND hwndNewFocus)
{
   DrawGridBorder();
   GraphicsReleaseCapture();
   RedrawAll();
}
/****************************************************************************/
Bool GameDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   if (!ModuleEvent(EVENT_DRAWITEM, hwnd, lpdis))
      return True;

   switch (lpdis->CtlID)
   {
   case IDS_LATENCY0:
      return Lagbox_OnDrawItem(hwnd, lpdis);
   case IDC_TOOLBUTTON:
      return ToolbarDrawButton(hwnd, lpdis);
   }
   return False;
}
/****************************************************************************/
void GameCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   switch (id)
   {
   case IDS_LATENCY0:
      Lagbox_Command(hwnd, id, hwndCtl, codeNotify);
      break;
   case IDC_TOOLBUTTON:
      ToolbarCommand(hwnd, id, hwndCtl, codeNotify);
      break;
   }
}
/****************************************************************************/
void GameTimer(HWND hwnd, UINT id)
{
   switch (id)
   {
   case TIMER_ANIMATE:
      AnimationTimerProc(hwnd, id);
      break;
   }
}
/****************************************************************************/
void GameIdle(void)
{
   MoveUpdateServer();       // Update our position on server, if necessary

   AnimationTimerProc(hMain, 0);
   HandleKeys();

   // Are we the active foreground application?
   AnimationTimerAbort();

   // If animation is off, or we're in the background, sleep to lower CPU load
   if (!config.animate ||
       GetWindowLong(GetActiveWindow(), GWL_HINSTANCE) != (LONG)hInst)
   {
      AnimationSleep();
   }
}
/****************************************************************************/
void GameEnterIdle(HWND hwnd, UINT source, HWND hwndSource)
{
   // When menu or dialog goes up, start animation timer in the background
   AnimationTimerStart();
}
/****************************************************************************/
int GameNotify(HWND hwnd, int idCtrl, NMHDR *pnmh)
{
  if (pnmh->hwndFrom == TooltipGetControl() && pnmh->code == TTN_NEEDTEXT)
    MapAnnotationGetText((TOOLTIPTEXT *) pnmh);
  return 0;
}

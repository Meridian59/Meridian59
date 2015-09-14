// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * winmenu.c:  Handle main client window menu.
 */

#include "client.h"

static HMENU menu;          // Main menu

extern int connection;
extern Bool gLargeArea;

/* local function prototypes */
/****************************************************************************/
/*
 * EnableGameMenus:  Enable appropriate menu items when the game begins.
 */
void EnableGameMenus(void)
{
   EnableMenuItem(menu, ID_GAME_PASSWORD, MF_ENABLED);
}
/****************************************************************************/
/*
 * DisableGameMenus:  Disable appropriate menu items when the game ends, 
 *    or at startup.
 */
void DisableGameMenus(void)
{
   EnableMenuItem(menu, ID_GAME_PASSWORD, MF_GRAYED);
}
/****************************************************************************/
/*
 * MenuDisplaySettings:  Set checks in main menu to reflect configuration settings.
 */
void MenuDisplaySettings(HWND hwnd)
{
   menu = GetMenu(hwnd);

   CheckMenuItem(menu, ID_OPTIONS_SAVEEXIT, config.save_settings ? MF_CHECKED : MF_UNCHECKED);
   CheckMenuItem(menu, ID_OPTIONS_MUSIC, config.play_music ? MF_CHECKED : MF_UNCHECKED);
   CheckMenuItem(menu, ID_OPTIONS_SOUND, config.play_sound ? MF_CHECKED : MF_UNCHECKED);
   CheckMenuItem(menu, ID_OPTIONS_AREA, config.large_area ? MF_CHECKED : MF_UNCHECKED);
}
/****************************************************************************/
/*
 * MenuConnect:  Change menu when we've connected to server.
 */
void MenuConnect(void)
{
   // This is the state when we're newly DISCONNECTED.
   // We show the user it's okay to ID_GAME_CONNECT now, and turn off others.

   EnableMenuItem(menu, ID_GAME_CONNECT, MF_ENABLED);
   EnableMenuItem(menu, ID_GAME_DISCONNECT, MF_GRAYED);

   EnableMenuItem(menu, ID_GAME_PRINTMAP, MF_GRAYED);
   EnableMenuItem(menu, ID_HELP_REQUEST_CS, MF_GRAYED);
}
/****************************************************************************/
/*
 * MenuDisconnect:  Change menu when we've disconnected from server.
 */
void MenuDisconnect(void)
{
   // This is the state when we're newly CONNECTED.
   // We show the user it's okay to ID_GAME_DISCONNECT now, and turn on others.

   EnableMenuItem(menu, ID_GAME_CONNECT, MF_GRAYED);
   EnableMenuItem(menu, ID_GAME_DISCONNECT, MF_ENABLED);

   EnableMenuItem(menu, ID_GAME_PRINTMAP, MF_ENABLED);
}
/********************************************************************/
/*
 * MenuCommand:  A menu item has been selected.
 */
void MenuCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   // Set last action time to now, to prevent keypress from being interpreted as a game action.
   KeySetLastNorepeatTime();

   // See if module wants to handle menu selection
   if (ModuleEvent(EVENT_MENUITEM, id) == False)
      return;
   
   /* Handle menu selections */
   switch (id)
   {
   case ID_GAME_EXIT:
      if (connection != CON_NONE && !AreYouSure(hInst, hMain, NO_BUTTON, IDS_LOGOFF))
	 break;
      MainQuit(hwnd);
      break;

   case ID_GAME_PRINTMAP:
      if (state == STATE_GAME)
	 PrintMap(FALSE);
      break;

   case ID_GAME_CONNECT:
      OfflineConnect();	 
      break;
      
   case ID_GAME_DISCONNECT:
      if (!AreYouSure(hInst, hMain, NO_BUTTON, IDS_LOGOFF))
	 break;
      Logoff();
      MainSetState(STATE_OFFLINE);
      break;

   case ID_GAME_SETTINGS:
      if (DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hMain, PreferencesDialogProc) == IDOK)
      {
         ModuleEvent(EVENT_CONFIGCHANGED);
         MenuDisplaySettings(hMain);
      }
      break;

  case ID_CONFIGMENU:
	  ConfigMenuLaunch();
      break;

   case ID_GAME_PASSWORD:
      PerformAction(A_CHANGEPASSWORD, NULL);
      break;

   case ID_OPTIONS_TIMEOUT:
      UserSetTimeout();
      break;
   case ID_OPTIONS_MUSIC:
      config.play_music = !config.play_music;
      CheckMenuItem(menu, ID_OPTIONS_MUSIC, config.play_music ? MF_CHECKED : MF_UNCHECKED);
      UserToggleMusic(config.play_music);
      break;
   case ID_OPTIONS_SOUND:
      config.play_sound = !config.play_sound;
      CheckMenuItem(menu, ID_OPTIONS_SOUND, config.play_sound ? MF_CHECKED : MF_UNCHECKED);
      if (!config.play_sound)
	 SoundStopAll();
      break;
   case ID_OPTIONS_SAVENOW:
      SaveSettings();
      break;
   case ID_OPTIONS_SAVEEXIT:
      config.save_settings = !config.save_settings;
      CheckMenuItem(menu, ID_OPTIONS_SAVEEXIT, config.save_settings ? MF_CHECKED : MF_UNCHECKED);
      break;
   case ID_OPTIONS_AREA:
	   // due to issues with certain D3D drivers, this no longer immediately updates the config
	   // it now sets a temporary variable that will update the config on shutdown
	   // this means a shutdown and restart are necessary for window size changes
	   MessageBox(hMain, "You must shutdown and restart Meridian 59 for these changes to take effect",
		   "Direct3D", MB_OK);
		   
//      config.large_area = !config.large_area;
	   gLargeArea = !gLargeArea;
      CheckMenuItem(menu, ID_OPTIONS_AREA, gLargeArea ? MF_CHECKED : MF_UNCHECKED);
/*      if (state == STATE_GAME)
	 // Send ourselves a resize message 
	 ResizeAll();
      RedrawAll();*/
      break;

   case ID_OPTIONS_FONT_MAP_TITLE: UserSelectFont(FONT_MAP_TITLE); break;
   case ID_OPTIONS_FONT_MAP_LABEL: UserSelectFont(FONT_MAP_LABEL); break;
   case ID_OPTIONS_FONT_MAP_TEXT: UserSelectFont(FONT_MAP_TEXT); break;

   case ID_FONT_GAMETEXT:
      UserSelectFont(FONT_EDIT);
      break;
   case ID_FONT_LIST:
      UserSelectFont(FONT_LIST);
      break;
   case ID_FONT_MAIL:
      UserSelectFont(FONT_MAIL);
      break;
   case ID_FONT_TITLES:
      UserSelectFont(FONT_TITLES);
      break;
   case ID_FONT_STATISTICS:
      UserSelectFont(FONT_STATS);
      break;
   case ID_FONT_INPUT:
      UserSelectFont(FONT_INPUT);
      break;
   case ID_FONT_LABELS:
      UserSelectFont(FONT_LABELS);
      break;
   case ID_FONT_DEFAULTS:
      FontsRestoreDefaults();
      break;

   case ID_COLOR_MAIN:
      UserSelectColors(COLOR_FGD, COLOR_BGD);
      break;
   case ID_COLOR_LIST:
      UserSelectColors(COLOR_LISTFGD, COLOR_LISTBGD);
      break;
   case ID_COLOR_LISTSEL:
      UserSelectColors(COLOR_LISTSELFGD, COLOR_LISTSELBGD);
      break;
   case ID_COLOR_MAGIC:
      UserSelectColor(COLOR_ITEM_MAGIC_FG);
      break;
   case ID_COLOR_HIGHLIGHT:
      UserSelectColor(COLOR_HIGHLITE);
      break;
   case ID_COLOR_MAIL:
      UserSelectColors(COLOR_MAILFGD, COLOR_MAILBGD);
      break;
   case ID_COLOR_TEXT:
      UserSelectColors(COLOR_MAINEDITFGD, COLOR_MAINEDITBGD);
      break;
   case ID_COLOR_EDIT:
      UserSelectColors(COLOR_EDITFGD, COLOR_EDITBGD);
      break;
   case ID_COLOR_SYSMSG:
      UserSelectColor(COLOR_SYSMSGFGD);
   case ID_COLOR_QUESTHEADER:
      UserSelectColor(COLOR_QUEST_HEADER);
      break;
      break;
   case ID_COLOR_STATS:
      UserSelectColors(COLOR_STATSFGD, COLOR_STATSBGD);
      break;
   case ID_COLOR_BAR1:
      UserSelectColor(COLOR_BAR1);
      break;
   case ID_COLOR_BAR2:
      UserSelectColor(COLOR_BAR2);
      break;
   case ID_COLOR_BAR3:
      UserSelectColor(COLOR_BAR3);
      break;
   case ID_COLOR_BAR4:
      UserSelectColor(COLOR_BAR4);
      break;
   case ID_COLOR_INVNUM:
      UserSelectColors(COLOR_INVNUMFGD, COLOR_INVNUMBGD);
      break;
   case ID_COLOR_DEFAULTS:
      ColorsRestoreDefaults();
      break;

   case ID_HELP_CONTENTS:
      StartHelp();
      break;
   case ID_HOMEPAGE:
      WebLaunchBrowser(GetString(hInst, IDS_HOMEPAGEURL));
      break;
   case ID_FORUM:
      WebLaunchBrowser(GetString(hInst, IDS_FORUMURL));
      break;
   case ID_WIKI:
      WebLaunchBrowser(GetString(hInst, IDS_WIKIURL));
      break;
   case ID_HELP_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hMain, AboutDialogProc);
      break;

   }
}

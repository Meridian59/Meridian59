// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * winmenu.c:  Handle main client window menu, including owner-drawn
 *   menu bar rendering for dark and light themes.
 */
#include "client.h"

static HMENU menu;          // Main menu
static HBRUSH hMenuBarBrush = NULL;
static HBRUSH hMenuBarHotBrush = NULL;

extern int connection;

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
   if (ModuleEvent(EVENT_MENUITEM, id) == false)
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
      {
         ShowPreferencesDialog(hMain);
         ModuleEvent(EVENT_CONFIGCHANGED);
      }
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
      break;
   case ID_OPTIONS_FONT_MAP_TITLE: UserSelectFont(FONT_MAP_TITLE); break;
   case ID_OPTIONS_FONT_MAP_LABEL: UserSelectFont(FONT_MAP_LABEL); break;
   case ID_OPTIONS_FONT_MAP_TEXT: UserSelectFont(FONT_MAP_TEXT); break;
   case ID_FONT_ANNOTATIONS:
      UserSelectFont(FONT_MAP_ANNOTATIONS);
      break;
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
   case ID_HELP_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hMain, AboutDialogProc);
      break;

   }
}
/************************************************************************/
/*
 * DarkMenuBar_Apply:  Convert all top-level menu bar items to
 *   owner-drawn so they render with dark theme colors. Sets the menu
 *   bar background brush via MENUINFO. Safe to call multiple times;
 *   items already converted are skipped.
 */
void DarkMenuBar_Apply(HMENU hMenu)
{
   if (!hMenu || config.theme == THEME_DEFAULT)
      return;

   if (!hMenuBarBrush)
   {
      hMenuBarBrush = CreateSolidBrush(GetColor(COLOR_BGD));
      hMenuBarHotBrush = CreateSolidBrush(GetColor(COLOR_EDITBGD));
   }

   MENUINFO mi;
   memset(&mi, 0, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.fMask = MIM_BACKGROUND;
   mi.hbrBack = hMenuBarBrush;
   SetMenuInfo(hMenu, &mi);

   int count = GetMenuItemCount(hMenu);
   for (int i = 0; i < count; i++)
   {
      MENUITEMINFO mii;
      memset(&mii, 0, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_DATA;

      mii.dwTypeData = NULL;
      mii.cch = 0;
      GetMenuItemInfo(hMenu, i, TRUE, &mii);

      if (mii.fType & MFT_OWNERDRAW)
         continue;
      if (mii.fType & MFT_SEPARATOR)
         continue;

      UINT textLen = mii.cch + 1;
      char *text = (char *)SafeMalloc(textLen);
      mii.dwTypeData = text;
      mii.cch = textLen;
      GetMenuItemInfo(hMenu, i, TRUE, &mii);

      mii.fMask = MIIM_FTYPE | MIIM_DATA;
      mii.fType |= MFT_OWNERDRAW;
      mii.dwItemData = (ULONG_PTR)text;
      SetMenuItemInfo(hMenu, i, TRUE, &mii);
   }
}
/************************************************************************/
/*
 * DarkMenuBar_Remove:  Restore all owner-drawn menu bar items to
 *   standard string items and clear the background brush.
 */
void DarkMenuBar_Remove(HMENU hMenu)
{
   if (!hMenu)
      return;

   int count = GetMenuItemCount(hMenu);
   for (int i = 0; i < count; i++)
   {
      MENUITEMINFO mii;
      memset(&mii, 0, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_FTYPE | MIIM_DATA;
      GetMenuItemInfo(hMenu, i, TRUE, &mii);

      if (!(mii.fType & MFT_OWNERDRAW))
         continue;

      char *text = (char *)mii.dwItemData;

      mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_DATA;
      mii.fType &= ~MFT_OWNERDRAW;
      mii.dwTypeData = text ? text : (char *)"";
      mii.cch = text ? (UINT)strlen(text) : 0;
      mii.dwItemData = 0;
      SetMenuItemInfo(hMenu, i, TRUE, &mii);

      if (text)
         SafeFree(text);
   }

   MENUINFO mi;
   memset(&mi, 0, sizeof(mi));
   mi.cbSize = sizeof(mi);
   mi.fMask = MIM_BACKGROUND;
   mi.hbrBack = NULL;
   SetMenuInfo(hMenu, &mi);
}
/************************************************************************/
/*
 * DarkMenuBar_Destroy:  Remove owner-drawn state from the current menu
 *   and free the dark menu bar brushes.
 */
void DarkMenuBar_Destroy(void)
{
   HMENU hMenu = GetMenu(hMain);
   if (hMenu)
      DarkMenuBar_Remove(hMenu);

   if (hMenuBarBrush)
   {
      DeleteObject(hMenuBarBrush);
      hMenuBarBrush = NULL;
   }
   if (hMenuBarHotBrush)
   {
      DeleteObject(hMenuBarHotBrush);
      hMenuBarHotBrush = NULL;
   }
}
/************************************************************************/
/*
 * DarkMenuBar_MeasureItem:  Returns true and fills the MEASUREITEMSTRUCT
 *   if the item is an owner-drawn menu bar item in a non-classic theme.
 */
bool DarkMenuBar_MeasureItem(MEASUREITEMSTRUCT *mis)
{
   if (mis->CtlType != ODT_MENU)
      return false;
   if (config.theme == THEME_DEFAULT)
      return false;

   const char *text = (const char *)mis->itemData;
   if (!text)
      return false;

   HDC hdc = GetDC(hMain);
   NONCLIENTMETRICS ncm;
   memset(&ncm, 0, sizeof(ncm));
   ncm.cbSize = sizeof(ncm);
   SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   HFONT hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);
   HFONT hOldFont = (HFONT)SelectObject(hdc, hMenuFont);

   SIZE size;
   GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);

   mis->itemWidth = size.cx;
   mis->itemHeight = size.cy + 8;

   SelectObject(hdc, hOldFont);
   DeleteObject(hMenuFont);
   ReleaseDC(hMain, hdc);

   return true;
}
/************************************************************************/
/*
 * DarkMenuBar_DrawItem:  Returns true and paints the item if it is an
 *   owner-drawn menu bar item in a non-classic theme.
 */
bool DarkMenuBar_DrawItem(DRAWITEMSTRUCT *dis)
{
   if (dis->CtlType != ODT_MENU)
      return false;
   if (config.theme == THEME_DEFAULT)
      return false;

   const char *text = (const char *)dis->itemData;
   if (!text)
      return false;

   HDC hdc = dis->hDC;
   RECT rc = dis->rcItem;

   HBRUSH bgBrush;
   COLORREF textColor;

   if (dis->itemState & (ODS_SELECTED | ODS_HOTLIGHT))
   {
      bgBrush = hMenuBarHotBrush;
      textColor = RGB(255, 255, 255);
   }
   else
   {
      bgBrush = hMenuBarBrush;
      textColor = GetColor(COLOR_FGD);
   }

   FillRect(hdc, &rc, bgBrush);

   SetTextColor(hdc, textColor);
   SetBkMode(hdc, TRANSPARENT);

   NONCLIENTMETRICS ncm;
   memset(&ncm, 0, sizeof(ncm));
   ncm.cbSize = sizeof(ncm);
   SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   HFONT hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);
   HFONT hOldFont = (HFONT)SelectObject(hdc, hMenuFont);

   DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

   SelectObject(hdc, hOldFont);
   DeleteObject(hMenuFont);

   return true;
}

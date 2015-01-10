// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charpick.c:  Let player select a character to use in the game.
 */

#include "client.h"
#include "char.h"
#include <algorithm>

// Parameters to pick char dialog box
typedef struct {
   Character     *characters;
   WORD           num_characters;
   char          *motd;
   BYTE           num_ads;
   Advertisement *ads;
} PickCharStruct;

ID    char_to_use;   /* ID # of character player wants to use in game */
char  name_to_use[MAXNAME];   /* name resource of character player wants to use in game */

char *ad_directory = "ads";   // Subdirectory with advertisement files

static HWND hPickCharDialog = NULL;
extern HWND hMakeCharDialog;

// Animation controls
static int animation_controls[] = { IDC_ANIMATE3, IDC_ANIMATE4, };
static PickCharStruct *info;

static BOOL CALLBACK PickCharDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static void CharPickLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
/********************************************************************/
/* character is id of character to use */
void GoToGame(ID character)
{
   RequestUseCharacter(character);
   GameSetState(GAME_INIT);  // Start up game interface
}
/********************************************************************/
void AbortCharDialogs(void)
{
   Bool has_dialog = False;

   if (hPickCharDialog != NULL)
   {
      has_dialog = True;
      EndDialog(hPickCharDialog, IDCANCEL);
   }
   if (hMakeCharDialog != NULL)
   {
      has_dialog = True;
      PropSheet_PressButton(hMakeCharDialog, PSBTN_CANCEL);
      // Hide in case another dialog pops up; for some reason the property sheet lingers awhile
      ShowWindow(hMakeCharDialog, SW_HIDE);
   }
   exiting = True;

   if (!has_dialog)
      PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
}
/********************************************************************/
/*
 * ChooseCharacter:  Ask user to choose a character from the given characters.
 */
void ChooseCharacter(Character *characters, WORD num_characters, char *motd,
		     BYTE num_ads, Advertisement *ads, BOOL quickstart)
{
   int retval;
   PickCharStruct s;

   if ((num_characters == 1) && quickstart && (characters->flags != 1))
   {
      char_to_use = characters->id;
      GoToGame(char_to_use);
      return;
   }

   GameSetState(GAME_PICKCHAR);

   s.characters     = characters;
   s.num_characters = num_characters;
   s.motd           = motd;
   s.num_ads        = num_ads;
   s.ads            = ads;

   retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHARPICK), cinfo->hMain,PickCharDialogProc, (LPARAM) &s);

   CharactersDestroy(characters, num_characters);

   switch (retval)
   {
   case IDCANCEL:
      return;

   case IDOK:
      GoToGame(char_to_use);
      return;

   case IDC_RESET:
      /* Player wants to start a new character */
      RequestCharInfo(); 
      break;
   }
}
/********************************************************************/
BOOL CALLBACK PickCharDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static HWND hList;
   int index, i;
   Character *c;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      hList = GetDlgItem(hDlg, IDC_CHARLIST);
      info = (PickCharStruct *) lParam;

      // Insertion sort alphabetically, with new characters at the end
      for (i = 1; i < info->num_characters; ++i) {
         int j = i;
         
         while (j > 0 &&
                ((info->characters[j-1].flags == 1 && info->characters[j].flags != 1) ||
                 strcmpi(info->characters[j-1].name, info->characters[j].name) > 0)) {
            std::swap(info->characters[j], info->characters[j - 1]);
            --j;
         }
      }
      
      /* Display characters in list */
      for (i=0; i < info->num_characters; i++)
      {
         // For new characters, show a special string
         if (info->characters[i].flags == 1)
            index = ListBox_AddString(hList, GetString(hInst, IDS_NEWCHARACTER));
         else index = ListBox_AddString(hList, info->characters[i].name);
         ListBox_SetItemData(hList, index, i);
      }
      
      /* Select first char */
      ListBox_SetCurSel(hList, 0);
      
      // Show message of the day
      Edit_SetText(GetDlgItem(hDlg, IDC_MOTD), info->motd);  
      
      // Display advertisements
      for (i=0; i < info->num_ads; i++)
      {
         char filename[MAX_PATH + FILENAME_MAX];
         sprintf(filename, "%s\\%s", ad_directory, info->ads[i].filename);
         Animate_Open(GetDlgItem(hDlg, animation_controls[i]), filename);
      }
      
      hPickCharDialog = hDlg;
      return TRUE;
      
   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM:
      return ItemListDrawItem(hDlg, (const DRAWITEMSTRUCT *) lParam);

      HANDLE_MSG(hDlg, WM_LBUTTONDOWN, CharPickLButtonDown);
	 
      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_RESET:
	 /* Get currently chosen character's name */
	 if ((index = ListBox_GetCurSel(hList)) == LB_ERR)
	    return TRUE;

	 index = ListBox_GetItemData(hList, index);

	 c = &info->characters[index];
	 
	 if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_DELETECHAR, c->name))
	    return TRUE;

	 char_to_use = c->id;
	 strcpy(name_to_use, c->name);
	 EndDialog(hDlg, IDC_RESET);
	 return TRUE;
	 
      case IDOK:
	 /* Tell server that we want to use selected character */
	 if ((index = ListBox_GetCurSel(hList)) == LB_ERR)
	    return TRUE;

	 index = ListBox_GetItemData(hList, index);

	 c = &info->characters[index];
	 
	 char_to_use = c->id;
	 strcpy(name_to_use, c->name);

	 // If this character hasn't been used before, go to char creation
	 if (c->flags == 1)
	    EndDialog(hDlg, IDC_RESET);
	 else EndDialog(hDlg, IDOK);
	 return TRUE;
	 
      case IDCANCEL:
	 /* If user verifies, ask to leave game */	 
	 if (!AreYouSure(cinfo->hInst, hDlg, NO_BUTTON, IDS_LOGOFF))
	    break;
	 Logoff();

	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hPickCharDialog = NULL;
      if (exiting)
	 PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      break;
   }
   return FALSE;
}
/********************************************************************/
/*
 * CharactersDestroy:  Free memory for given characters array.
 */
void CharactersDestroy(Character *c, WORD num_characters)
{
   int i;

   for (i=0; i < num_characters; i++)
      SafeFree(c[i].name);
   SafeFree(c);
}
/****************************************************************************/
/*
 * CharPickLButtonDown:  Mouse button pressed on dialog; see if inside an ad
 *   control and bring up browser on URL if so.
 */ 
void CharPickLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   int i;
   RECT rect;
   POINT p;

   p.x = x;
   p.y = y;

   ClientToScreen(hwnd, &p);
   for (i=0; i < info->num_ads; i++)
   {
      GetWindowRect(GetDlgItem(hPickCharDialog, animation_controls[i]), &rect);
      if (PtInRect(&rect, p))
      {
	 WebLaunchBrowser(info->ads[i].url);

	 // Tell server that user selected ad
	 SendAdSelected(i + 1);
	 return;
      }
   }
}

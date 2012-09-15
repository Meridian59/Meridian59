// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildshi.c:  Handle guild shield dialog.
 */

#include "client.h"
#include "merintr.h"

#define legalShield(a,b,c) ((a) != 9 || (b) != 9)

static HWND hwndShieldPage = NULL;
static HWND hwndShield = NULL;
static list_type listShieldIDs = NULL;
static int iNumPatterns;
static int iChosenPattern;
static int iChosenColor1;
static int iChosenColor2;
static int iNumColors;
static void GuildShieldDestroy();
static ID idMyGuild = 0;
static char szClaimFormat[MAX_GUILD_NAME + 100];
static char szClaimBuffer[MAX_GUILD_NAME + 100];

void GuildShieldControl(HWND hwndGraphic)
{
	hwndShield = hwndGraphic;
}

/*****************************************************************************/
/*
 * GuildShieldDialogProc:  Dialog procedure for guild shield dialog.
 */
BOOL CALLBACK GuildShieldDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:

		hwndShieldPage = hDlg;
		GuildShieldControl(GetDlgItem(hDlg, IDC_SHIELD));

		RequestGuildShieldSamples();
		RequestGuildShield();

		// claimant guild control holds format ("claimed by %s")
		SetWindowFont(GetDlgItem(hDlg, IDC_STATIC_GUILD), GetFont(FONT_LIST), FALSE);
		GetDlgItemText(hDlg, IDC_STATIC_GUILD, szClaimFormat, sizeof(szClaimFormat));

		// claimant guild control starts with "can't use those colors" since it's grey+grey
		GetDlgItemText(hDlg, IDC_RENOUNCE, szClaimBuffer, sizeof(szClaimBuffer));
		SetDlgItemText(hDlg, IDC_STATIC_GUILD, szClaimBuffer);

		CenterWindow(hDlg, GetParent(hDlg));
		return TRUE;

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam))
		{

		// in range [1,iNumPatterns]
		case IDC_PATTERN_PREV:
			iChosenPattern--;
			if(iChosenPattern < 1)
				iChosenPattern = iNumPatterns;
			break;
		case IDC_PATTERN_NEXT:
			iChosenPattern++;
			if(iChosenPattern > iNumPatterns)
				iChosenPattern = 1;
			break;

		// in range [0,iNumColors-1]
		case IDC_COLOR1_PREV:
			iChosenColor1--;
			if(iChosenColor1 < 0)
				iChosenColor1 = iNumColors - 1;
			break;
		case IDC_COLOR1_NEXT:
			iChosenColor1++;
			if(iChosenColor1 >= iNumColors)
				iChosenColor1 = 0;
			break;
		case IDC_COLOR2_PREV:
			iChosenColor2--;
			if(iChosenColor2 < 0)
				iChosenColor2 = iNumColors - 1;
			break;
		case IDC_COLOR2_NEXT:
			iChosenColor2++;
			if(iChosenColor2 >= iNumColors)
				iChosenColor2 = 0;
			break;

		case IDC_ACCEPT:
			if (legalShield(iChosenColor1,iChosenColor2,iChosenPattern))
			{
				RequestClaimGuildShield(iChosenColor1, iChosenColor2, iChosenPattern);
			}
			break;
		}
		RequestFindGuildShield(iChosenColor1, iChosenColor2, iChosenPattern);
		GuildShieldDraw();
		return FALSE;

	case WM_SHOWWINDOW:
		if (!(BOOL)wParam)
			RequestGuildShield();
		break;

	case WM_DRAWITEM:
		GuildShieldControl(GetDlgItem(hDlg, IDC_SHIELD));
		GuildShieldDraw();
		return TRUE;

	case WM_DESTROY:
		hwndShieldPage = NULL;
		return FALSE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------
/*
 * GuildShieldInit:
 */
void GuildShieldInit()
{
	GuildShieldDestroy();

	// We start with zero guild shield patterns.
	// We will eventually receive a list of guild shield pattern RSC numbers for use.
	// See GuildGotShields().

	iNumColors = (int)sqrt((float) XLAT_GUILDCOLOR_END - XLAT_GUILDCOLOR_BASE + 1);
	iChosenColor1 = rand() % iNumColors;
	iChosenColor2 = rand() % iNumColors;
	iChosenPattern = 1;
}

void GuildGotShields(list_type shields)
{
	iNumPatterns = list_length(shields);

	listShieldIDs = shields;
	iChosenPattern = (iNumPatterns > 0)? (rand() % iNumPatterns)+1 : 1;
	GuildShieldDraw();
}

void GuildGotShield(ID idGuild, char* pszGuildName, BYTE color1, BYTE color2, BYTE pattern)
{
   if (hwndShieldPage)
   {
      if (idMyGuild && idGuild == idMyGuild && legalShield(color1,color2,pattern))
      {
	 // if i'm getting my claimed guild shield colors, i can't claim other colors
	 EnableWindow(GetDlgItem(hwndShieldPage, IDC_ACCEPT), FALSE);
      }
      if (!legalShield(color1,color2,pattern))
      {
	 // claimant guild gets "can't use those colors"
	 GetDlgItemText(hwndShieldPage, IDC_RENOUNCE, szClaimBuffer, sizeof(szClaimBuffer));
	 SetDlgItemText(hwndShieldPage, IDC_STATIC_GUILD, szClaimBuffer);
      }
      else if (!idGuild)
      {
	 // claimant guild is blank
	 SetDlgItemText(hwndShieldPage, IDC_STATIC_GUILD, "");
      }
      else
      {
	 // claimant guild's name appears
	 sprintf(szClaimBuffer, szClaimFormat, pszGuildName);
	 SetDlgItemText(hwndShieldPage, IDC_STATIC_GUILD, szClaimBuffer);
      }
#if 0
      {
      char buffer[256];
      wsprintf(buffer,"Color1=%d, Color2=%d, Pattern=%d",(int)color1,(int)color2,(int)pattern);
      SetDlgItemText(hwndShieldPage, IDC_STATIC_GUILD, buffer);
      }
#endif
   }

   iChosenColor1 = (int)color1;
   iChosenColor2 = (int)color2;
   iChosenPattern = (int)pattern;
   GuildShieldDraw();

   if (!idMyGuild && idGuild)
      idMyGuild = idGuild;
}

//-------------------------------------------------------------------------------------------------------------------------
/*
 * GuildShieldDestroy:
 */
void GuildShieldDestroy()
{
	if (listShieldIDs)
		list_destroy(listShieldIDs);
	listShieldIDs = NULL;
}

//-------------------------------------------------------------------------------------------------------------------------
/*
 * GuildShieldDraw:
 */
void GuildShieldDraw(void)
{
   HDC hDC;
   AREA areaWndShield;
   RECT rcWndShield;
   object_node obj;
   list_type listBlank = NULL;
   void* pIcon = NULL;

   if (!hwndShield)
      return;

   GetClientRect(hwndShield, &rcWndShield);
   RectToArea(&rcWndShield, &areaWndShield);
   memset(&obj, 0, sizeof(object_node));

   pIcon = list_nth_item(listShieldIDs, iChosenPattern-1);
   if (pIcon)
   {
      obj.icon_res = (pIcon? (*(ID*)pIcon) : 0);
      obj.translation = XLAT_GUILDCOLOR_BASE + iNumColors*iChosenColor1 + iChosenColor2;
      obj.overlays = &listBlank;
      hDC = GetDC(hwndShield);
      DrawStretchedObjectDefault(hDC, &obj, &areaWndShield, GetSysColorBrush(COLOR_3DFACE));
      ReleaseDC(hwndShield, hDC);
   }
}

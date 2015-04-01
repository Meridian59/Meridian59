// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* color.c:  Deal with saving, retrieving, and restoring fonts.
*/

#include "client.h"

typedef struct {
	WORD fg;     /* Foreground color of sample text */
	WORD bg;     /* Background color of sample text */
} ColorDialogStruct;

static COLORREF colors[MAXCOLORS];
static COLORREF DefaultColor;

/* To save GDI space, brushes are only created if they're needed in GetBrush */
static HBRUSH brushes[MAXCOLORS];
static HBRUSH hDefaultBrush;

static COLORREF CustColors[16];  /* User's custom colors for Choose Color dialog */

/* Default color information structures */
// default colors changed for grey background in new client - mistery
static char colorinfo[][15] = {
	{ "0,0,0"},         /* COLOR_BGD */
	{ "255,255,255"},   /* COLOR_FGD */
	{ "255,255,255"},   /* COLOR_LISTSELBGD */
	{ "0,0,0"},         /* COLOR_LISTSELFGD */
	{ "255,255,255"},   /* COLOR_MAILBGD */
	{ "0,0,0"},         /* COLOR_MAILFGD */
//	{ "252,128,0"},     /* COLOR_HIGHLITE */
	{ "240,208,25"},     /* COLOR_HIGHLITE */
	{ "0,0,0"},         /* COLOR_EDITFGD */
	{ "255,255,255"},   /* COLOR_EDITBGD */
	{ "128,0,128"},     /* COLOR_SYSMSGFGD */
//	{ "0,0,0"},         /* COLOR_MAINEDITFGD */
	{ "255,255,255"},         /* COLOR_MAINEDITFGD */
//	{ "188,152,108"},   /* COLOR_MAINEDITBGD */
	{ "120,120,120"},   /* COLOR_MAINEDITBGD */
	{ "255,255,255"},   /* COLOR_LISTFGD */
	{ "0,0,0"},         /* COLOR_LISTBGD */
	{ "128,0,0"},       /* COLOR_RMMSGFGD */
	{ "0,0,0"},         /* COLOR_RMMSGBGD */
//	{ "248,208,96"},    /* COLOR_STATSGFGD */
	{ "231,231,231"},    /* COLOR_STATSGFGD */
	{ "48,0,0"},        /* COLOR_STATSGBGD */
	{ "0,128,0"},       /* COLOR_BAR1 */
	{ "128,0,0"},       /* COLOR_BAR2 */
	{ "48,0,0"},        /* COLOR_BAR3 */
	{ "255,255,255"},   /* COLOR_BAR4 */
	{ "192,192,192"},   /* COLOR_INVNUMFGD */
	{ "0,0,0"},         /* COLOR_INVNUMBGD */
	{ "255,80,0"}       /* COLOR_ITEM_MAGIC_FG */
};

static char color_section[] = "Colors";  /* Section for colors in INI file */

/* Colors for drawing player names. These are no longer
   needed since we send RGB hex values. Keeping these
   here for now.
#define NAME_COLOR_NORMAL_FG   PALETTERGB(255, 255, 255)
#define NAME_COLOR_OUTLAW_FG   PALETTERGB(252, 128, 0)
#define NAME_COLOR_KILLER_FG   PALETTERGB(255, 0, 0)
#define NAME_COLOR_EVENT_FG    PALETTERGB(255, 0, 255)
#define NAME_COLOR_CREATOR_FG  PALETTEINDEX(251) // yellow
#define NAME_COLOR_SUPER_FG    PALETTEINDEX(250) // green
#define NAME_COLOR_DM_FG       PALETTEINDEX(254) // cyan
#define NAME_COLOR_MOD_FG      PALETTERGB(0, 120, 255)
#define NAME_COLOR_BLACK_FG    PALETTERGB(0, 0, 0)
#define NAME_COLOR_DAENKS_FG   PALETTERGB(179,0,179)*/

extern HPALETTE hPal;

BOOL CALLBACK ColorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
Bool SetColor(WORD color, COLORREF cr);
/************************************************************************/
/*
* ColorsCreate:  Create all colors for use in the game.
*  If use_defaults is False, try to load colors from INI file.
*  Otherwise use colors fonts.
*/
void ColorsCreate(Bool use_defaults)
{
	WORD i;
	int temp;
	BYTE red, green, blue;
	char str[100], name[10], *ptr;
	char *separators = ",";
	Bool success;

	DefaultColor = PALETTERGB(255, 255, 255);
	hDefaultBrush = (HBRUSH) GetStockObject(BLACK_BRUSH);

	/* Restore default custom colors */
	if (use_defaults)
		for (i=0; i < 16; i++)
			CustColors[i] = PALETTERGB(0, 0, 0);

		/* Try to load colors from private .INI file; if not there, use defaults */
		for (i=0; i < MAXCOLORS; i++)
		{
			if (use_defaults)
				strcpy(str, colorinfo[i]);
			else
			{
				sprintf(name, "Color%d", i);
				GetPrivateProfileString(color_section, name, colorinfo[i], str, 100, ini_file);
			}

			success = True;
			if ((ptr = strtok(str, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = False;
			else red = temp;
			if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = False;
			else green = temp;
			if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = False;
			else blue = temp;

			if (success)
				SetColor(i, PALETTERGB(red, green, blue));
			else SetColor(i, DefaultColor);

			brushes[i] = NULL;
		}
}
/************************************************************************/
void ColorsDestroy(void)
{
	WORD i;

	for (i=0; i < MAXCOLORS; i++)
		if (brushes[i] != hDefaultBrush && brushes[i] != NULL)
			DeleteObject(brushes[i]);
}
/************************************************************************/
COLORREF GetColor(WORD color)
{
	if (color > MAXCOLORS)
	{
		debug(("Illegal color #%u in GetColor\n", color));
		return DefaultColor;
	}
	// Return palette-relative color
	return MAKEPALETTERGB(colors[color]);
}
/************************************************************************/
HBRUSH GetBrush(WORD color)
{
	if (color > MAXCOLORS)
	{
		debug(("Illegal color #%u in GetBrush\n", color));
		return hDefaultBrush;
	}

	/* Create brush if necessary */
	if (brushes[color] == NULL)
		brushes[color] = CreateSolidBrush(GetColor(color));
	if (brushes[color] == NULL)
		brushes[color] = hDefaultBrush;

	return brushes[color];
}
/************************************************************************/
/*
* SetColor:  Set given user color # using given COLORREF.
*   Returns True on success; returns False and uses default color on failure.
*/
Bool SetColor(WORD color, COLORREF cr)
{
	if (color > MAXCOLORS)
	{
		debug(("Illegal color #%u\n", color));
		return False;
	}

	/* Round color to nearest match in our palette; this lets transparency work */
	colors[color] = GetNearestPaletteColor(cr);

	if (brushes[color] != NULL)
	{
		DeleteObject(brushes[color]);
		brushes[color] = NULL;
	}
	return True;
}
/************************************************************************/
/*
* ColorsSave:  Save current color selections to our INI file.
*/
void ColorsSave(void)
{
	int i;
	char str[100], name[10];

	for (i=0; i < MAXCOLORS; i++)
	{
		sprintf(str, "%d,%d,%d",
			GetRValue(colors[i]), GetGValue(colors[i]), GetBValue(colors[i]));
		sprintf(name, "Color%d", i);

		WritePrivateProfileString(color_section, name, str, ini_file);
	}
}
/************************************************************************/
void ColorsRestoreDefaults(void)
{
	ColorsDestroy();
	ColorsCreate(True);
	MainChangeColor();

	ModuleEvent(EVENT_COLORCHANGED, -1, 0);
}



/************************************************************************/
/*
* UserSelectColor:  Allow user to select a single color.
*/
void UserSelectColor(WORD color)
{
	CHOOSECOLOR cc;

	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hMain;
	cc.rgbResult = GetColor(color);
	cc.lpCustColors = CustColors;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	if (ChooseColor(&cc) == 0)
		return;

	/* Set new color */
	SetColor(color, cc.rgbResult);

	/* See if a module wants to intercept color change */
	if (ModuleEvent(EVENT_COLORCHANGED, color, cc.rgbResult) == False)
		return;

	MainChangeColor();
}
/************************************************************************/
/*
* UserSelectColors:  Allow user to select a foreground/background pair.
*/
void UserSelectColors(WORD fg, WORD bg)
{
	ColorDialogStruct info;

	info.fg = fg;
	info.bg = bg;

	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COLOR), hMain,
		ColorDialogProc, (LPARAM) &info);
}
/************************************************************************/
/*
* ColorDialogProc:  Allow user to select foreground & background colors.
*/
BOOL CALLBACK ColorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ColorDialogStruct *info;
	static HWND hSample;
	static COLORREF temp_fg, temp_bg;
	int ctrl;
	CHOOSECOLOR cc;
	HDC hdc;

	switch (message)
	{
	case WM_INITDIALOG:
		info = (ColorDialogStruct *) lParam;

		temp_fg = GetColor(info->fg);
		temp_bg = GetColor(info->bg);

		hSample = GetDlgItem(hDlg, IDC_SAMPLETEXT);
		CenterWindow(hDlg, hMain);

		return TRUE;

	case WM_PAINT:
		hdc = GetDC(hSample);

		SelectPalette(hdc, hPal, FALSE);

		SetTextColor(hdc, temp_fg);
		SetBkColor(hdc, temp_bg);

		InvalidateRect(hSample, NULL, TRUE);
		UpdateWindow(hSample);

		SelectObject(hdc, GetFont(FONT_TITLES));
		TextOut(hdc, 0, 0, szAppName, strlen(szAppName));

		ReleaseDC(hSample, hdc);
		break;

	case WM_COMMAND:
		switch(ctrl = GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDC_BACKGROUND:
		case IDC_FOREGROUND:
			memset(&cc, 0, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hDlg;
			cc.rgbResult = (ctrl == IDC_FOREGROUND) ? temp_fg : temp_bg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_RGBINIT | CC_FULLOPEN;
			if (ChooseColor(&cc) == 0)
				return TRUE;

			/* Set new color */
			if (ctrl == IDC_FOREGROUND)
				temp_fg = MAKEPALETTERGB(cc.rgbResult);
			else temp_bg = MAKEPALETTERGB(cc.rgbResult);

			/* Redraw to see new colors */
			InvalidateRect(hDlg, NULL, TRUE);
			return TRUE;

		case IDOK:
			/* Save user's choices */
			SetColor(info->fg, temp_fg);
			SetColor(info->bg, temp_bg);

			MainChangeColor();

			EndDialog(hDlg, IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
	}

	return FALSE;
}
/****************************************************************************/
/*
* MainCtlColor:  Handle CTLCOLOR messages for main window.
*/
HBRUSH MainCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	switch (type)
	{
	case CTLCOLOR_EDIT:
		SelectPalette(hdc, hPal, FALSE);
		SetTextColor(hdc, GetColor(COLOR_MAINEDITFGD));
		SetBkColor(hdc, GetColor(COLOR_MAINEDITBGD));
		return GetBrush(COLOR_MAINEDITBGD);

	case CTLCOLOR_SCROLLBAR:  /* Don't color scrollbars */
		//return (HBRUSH) FALSE;
		return (HBRUSH) GetStockObject( BLACK_BRUSH );		//	xxx

	default:
		SelectPalette(hdc, hPal, FALSE);
		SetTextColor(hdc, GetColor(COLOR_FGD));
		SetBkColor(hdc, GetColor(COLOR_BGD));
		return GetBrush(COLOR_BGD);
	}
}
/****************************************************************************/
/*
* DialogCtlColor:  Handle CTLCOLOR messages for dialog boxes.
*/
HBRUSH DialogCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	switch (type)
	{
	case CTLCOLOR_EDIT:
		SelectPalette(hdc, hPal, FALSE);
		SetTextColor(hdc, GetColor(COLOR_EDITFGD));
		SetBkColor(hdc, GetColor(COLOR_EDITBGD));
		return GetBrush(COLOR_EDITBGD);

	case CTLCOLOR_LISTBOX:
		SelectPalette(hdc, hPal, FALSE);
		SetTextColor(hdc, GetColor(COLOR_LISTFGD));
		SetBkColor(hdc, GetColor(COLOR_LISTBGD));
		return GetBrush(COLOR_LISTBGD);

	case CTLCOLOR_SCROLLBAR:
		return (HBRUSH) FALSE;

	default:
		SelectPalette(hdc, hPal, FALSE);
		/* All others--dialog, static, buttons, etc. */
		// Removed for CTL3D
		//      SetTextColor(hdc, GetColor(COLOR_DIALOGFGD));
		//      SetBkColor(hdc, GetColor(COLOR_DIALOGBGD));
		//      return GetBrush(COLOR_DIALOGBGD);
		return (HBRUSH) FALSE;
	}
}
/****************************************************************************/
/*
* GetItemListColor:  Get given color id # for given owner-drawn list box.
*    (Inventory has different colors than popup dialog lists)
*    Doesn't return color itself so that caller can use id to call GetBrush.
*    Now colors magic items; any future item colors should be added here.
*/
WORD GetItemListColor(HWND hwnd, int type, int flags)
{
	if ((flags != NULL) && (GetItemFlags(flags) == (OF_ITEM_MAGIC | OF_GETTABLE)))
		return COLOR_ITEM_MAGIC_FG;
	else
   {
		switch(type)
		{
		case UNSEL_FGD:
			return COLOR_LISTFGD;
		case UNSEL_BGD:
			return COLOR_LISTBGD;
		case SEL_FGD:
			return COLOR_LISTSELFGD;
		case SEL_BGD:
			return COLOR_LISTSELBGD;
		}
	}

	return 0;
}

/****************************************************************************/
/*
* GetPlayerNameColor:  Return color that player's name should be drawn in,
*   depending on player's object flags
*/
COLORREF GetPlayerNameColor(object_node* obj, char *name)
{
   int r, g, b;

   if (obj->drawingtype == DRAWFX_BLACK)
      return PALETTERGB(0,0,0);

     r = (obj->namecolor & 0xFF0000) >> 16;
     g = (obj->namecolor & 0x00FF00) >> 8;
     b = (obj->namecolor & 0x0000FF);

     return PALETTERGB(r, g, b);
}

/****************************************************************************/
/*
* GetPlayerWhoNameColor:  Return color that player's name should be drawn on
*   the who list, depending on player's object flags
*/
COLORREF GetPlayerWhoNameColor(object_node* obj, char *name)
{
     int r, g, b;

     r = (obj->namecolor & 0xFF0000) >> 16;
     g = (obj->namecolor & 0x00FF00) >> 8;
     b = (obj->namecolor & 0x0000FF);

      return PALETTERGB(r, g, b);
}

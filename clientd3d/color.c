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

// Theme color tables.  See docs/themes.md for details.
// Each theme has a parallel table of MAXCOLORS "R,G,B" strings indexed
// by the COLOR_* enum from color.h.
static constexpr size_t COLOR_STR_LEN = 15;

// Default theme.
static char colorinfo_default[][COLOR_STR_LEN] = {
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
	{ "141,242,242"},	/* COLOR_ITEM_TEXT_UNCOMMON     - cyan   */
	{ "0,255,0"},	    /* COLOR_ITEM_TEXT_RARE         - lime   */
	{ "255,0,255"},     /* COLOR_ITEM_TEXT_LEGENDARY    - purple */
	{ "252,128,0"},	    /* COLOR_ITEM_TEXT_UNIDENTIFIED - orange */
	{ "255,0,0"},	    /* COLOR_ITEM_TEXT_CURSED       - red    */
};

// Dark theme.
static char colorinfo_dark[][COLOR_STR_LEN] = {
	{ "0,0,0"},         /* COLOR_BGD */
	{ "212,212,212"},   /* COLOR_FGD */
	{ "255,255,255"},   /* COLOR_LISTSELBGD - matches default for dialog list highlights */
	{ "0,0,0"},         /* COLOR_LISTSELFGD - matches default for dialog list highlights */
	{ "50,50,53"},      /* COLOR_MAILBGD */
	{ "212,212,212"},   /* COLOR_MAILFGD */
	{ "240,208,25"},    /* COLOR_HIGHLITE */
	{ "212,212,212"},   /* COLOR_EDITFGD */
	{ "60,60,64"},      /* COLOR_EDITBGD */
	{ "200,160,255"},   /* COLOR_SYSMSGFGD */
	{ "212,212,212"},   /* COLOR_MAINEDITFGD */
	{ "50,50,53"},      /* COLOR_MAINEDITBGD */
	{ "255,255,255"},   /* COLOR_LISTFGD - matches default for dialog lists */
	{ "0,0,0"},         /* COLOR_LISTBGD - matches default for dialog lists */
	{ "230,100,100"},   /* COLOR_RMMSGFGD */
	{ "50,50,53"},      /* COLOR_RMMSGBGD */
	{ "212,212,212"},   /* COLOR_STATSFGD */
	{ "50,50,53"},      /* COLOR_STATSBGD */
	{ "0,128,0"},       /* COLOR_BAR1 - bar fill (health/mana/vigor) */
	{ "128,0,0"},       /* COLOR_BAR2 - limit/damage bar */
	{ "48,0,0"},        /* COLOR_BAR3 - bar background */
	{ "255,255,255"},   /* COLOR_BAR4 - bar numbers */
	{ "180,180,180"},   /* COLOR_INVNUMFGD */
	{ "50,50,53"},      /* COLOR_INVNUMBGD */
	{ "141,242,242"},   /* COLOR_ITEM_TEXT_UNCOMMON */
	{ "0,255,0"},       /* COLOR_ITEM_TEXT_RARE */
	{ "255,0,255"},     /* COLOR_ITEM_TEXT_LEGENDARY */
	{ "252,128,0"},     /* COLOR_ITEM_TEXT_UNIDENTIFIED */
	{ "255,0,0"},       /* COLOR_ITEM_TEXT_CURSED */
};

// INI section names; one section per theme.
static char color_section_default[] = "Colors";
static char color_section_dark[]    = "ColorsDark";
static char INIColorVersion[]       = "ColorVersion";

// Bump when default values for any color change.
static const int THEME_COLOR_VERSION = 7;

// Returns the INI section name for the active theme.
static char *ColorSectionForTheme(Theme theme)
{
	return (theme == Theme::Default) ? color_section_default : color_section_dark;
}

// Returns the default color table for the active theme.
static char (*ColorDefaultsForTheme(Theme theme))[COLOR_STR_LEN]
{
	return (theme == Theme::Default) ? colorinfo_default : colorinfo_dark;
}

// Colors for drawing player names
#define NAME_COLOR_NORMAL_FG   PALETTERGB(255, 255, 255)
#define NAME_COLOR_OUTLAW_FG   PALETTERGB(252, 128, 0)
#define NAME_COLOR_KILLER_FG   PALETTERGB(255, 0, 0)
#define NAME_COLOR_EVENT_FG    PALETTERGB(255, 0, 255)
#define NAME_COLOR_CREATOR_FG  PALETTEINDEX(251) // yellow
#define NAME_COLOR_SUPER_FG    PALETTEINDEX(250) // green
#define NAME_COLOR_DM_FG       PALETTEINDEX(254) // cyan
#define NAME_COLOR_BLACK_FG    PALETTERGB(0, 0, 0)

extern HPALETTE hPal;

INT_PTR CALLBACK ColorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
bool SetColor(WORD color, COLORREF cr);
/************************************************************************/
/*
* ColorsCreate:  Create all colors for use in the game.
*  If use_defaults is false, try to load colors from INI file.
*  Otherwise use colors fonts.
*/
void ColorsCreate(bool use_defaults)
{
	WORD i;
	int temp;
	BYTE red, green, blue;
	char str[100], name[10], *ptr;
	const char *separators = ",";
	bool success;

	const char *section = ColorSectionForTheme(config.theme);
	char (*defaults)[COLOR_STR_LEN] = ColorDefaultsForTheme(config.theme);

	// Discard saved colors when the version doesn't match.
	if (!use_defaults)
	{
		int saved_version = GetPrivateProfileInt(section, INIColorVersion,
			0, ini_file);
		if (saved_version != THEME_COLOR_VERSION)
			use_defaults = true;
	}

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
				strcpy(str, defaults[i]);
			else
			{
				snprintf(name, sizeof(name), "Color%d", i);
				GetPrivateProfileString(section, name, defaults[i], str, 100, ini_file);
			}

			success = true;
			if ((ptr = strtok(str, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = false;
			else red = temp;
			if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = false;
			else green = temp;
			if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
				success = false;
			else blue = temp;

			if (success)
				SetColor(i, RGB(red, green, blue));
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
	// Default theme snaps to the indexed game palette; other themes pass
	// raw 24-bit RGB.
	if (config.theme == Theme::Default)
		return MAKEPALETTERGB(colors[color]);
	return colors[color] & 0x00FFFFFF;
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
*   Returns true on success; returns false and uses default color on failure.
*/
bool SetColor(WORD color, COLORREF cr)
{
	if (color > MAXCOLORS)
	{
		debug(("Illegal color #%u\n", color));
		return false;
	}

	// See GetColor.
	if (config.theme == Theme::Default)
		colors[color] = GetNearestPaletteColor(cr);
	else
		colors[color] = cr & 0x00FFFFFF;

	if (brushes[color] != NULL)
	{
		DeleteObject(brushes[color]);
		brushes[color] = NULL;
	}
	return true;
}
/************************************************************************/
/*
* ColorsSave:  Save current color selections to our INI file.
*/
void ColorsSave(void)
{
	int i;
	char str[100], name[10];
	char *section = ColorSectionForTheme(config.theme);

	for (i=0; i < MAXCOLORS; i++)
	{
		snprintf(str, sizeof(str), "%d,%d,%d",
			GetRValue(colors[i]), GetGValue(colors[i]), GetBValue(colors[i]));
		snprintf(name, sizeof(name), "Color%d", i);

		WritePrivateProfileString(section, name, str, ini_file);
	}

	// Stamp the version so future loads can detect stale saved colors.
	WriteConfigInt(section, INIColorVersion, THEME_COLOR_VERSION, ini_file);
}
/************************************************************************/
/*
 * ThemeCurrent:  Returns the active theme.
 */
Theme ThemeCurrent(void)
{
	return config.theme;
}
/************************************************************************/
/*
 * MainThemeResourceId:  Returns the variant of a main-client bitmap
 *   resource ID for the active theme.  Returns the input ID unchanged
 *   when no variant exists for the active theme.
 *
 *   See docs/themes.md.
 *
 *   TODO: add cases here as more main-client bitmaps get dark variants.
 */
int MainThemeResourceId(int id)
{
	switch (ThemeCurrent())
	{
	case Theme::Dark:
		switch (id)
		{
		case IDB_BACKGROUND: return IDB_BACKGROUND_DARK;
		default:             return id;
		}
	default:
		return id;
	}
}
/************************************************************************/
void ColorsRestoreDefaults(void)
{
	ColorsDestroy();
	ColorsCreate(true);
	MainChangeColor();

	ModuleEvent(EVENT_COLORCHANGED, COLOR_ID_ALL, 0);
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
	if (ModuleEvent(EVENT_COLORCHANGED, color, cc.rgbResult) == false)
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

	SafeDialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COLOR), hMain,
		ColorDialogProc, (LPARAM) &info);
}
/************************************************************************/
/*
* ColorDialogProc:  Allow user to select foreground & background colors.
*/
INT_PTR CALLBACK ColorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		TextOut(hdc, 0, 0, szAppName, (int) strlen(szAppName));

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

	case CTLCOLOR_SCROLLBAR:
		// Default paints scrollbars black; other themes use the system look.
		if (config.theme == Theme::Default)
			return (HBRUSH) GetStockObject( BLACK_BRUSH );
		return (HBRUSH) FALSE;

	default:
		SelectPalette(hdc, hPal, FALSE);
		SetTextColor(hdc, GetColor(COLOR_FGD));
		SetBkColor(hdc, GetColor(COLOR_BGD));
		return GetBrush(COLOR_BGD);
	}
}
/****************************************************************************/
/*
* DialogCtlColor:  Returns a brush and sets DC colors for dialog controls.
*   Only the default theme reads from GetColor().  Other themes use stock
*   white/black brushes so theme palettes stay out of dialogs.  See
*   docs/themes.md.
*/
HBRUSH DialogCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	switch (type)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_LISTBOX:
		SelectPalette(hdc, hPal, FALSE);
		if (config.theme == Theme::Default)
		{
			if (type == CTLCOLOR_EDIT)
			{
				SetTextColor(hdc, GetColor(COLOR_EDITFGD));
				SetBkColor(hdc, GetColor(COLOR_EDITBGD));
				return GetBrush(COLOR_EDITBGD);
			}
			SetTextColor(hdc, GetColor(COLOR_LISTFGD));
			SetBkColor(hdc, GetColor(COLOR_LISTBGD));
			return GetBrush(COLOR_LISTBGD);
		}
		// Other themes use stock dialog colors.
		if (type == CTLCOLOR_EDIT)
		{
			SetTextColor(hdc, PALETTERGB(0, 0, 0));
			SetBkColor(hdc, PALETTERGB(255, 255, 255));
			return (HBRUSH)GetStockObject(WHITE_BRUSH);
		}
		SetTextColor(hdc, PALETTERGB(255, 255, 255));
		SetBkColor(hdc, PALETTERGB(0, 0, 0));
		return (HBRUSH)GetStockObject(BLACK_BRUSH);

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
*    Also colors item text in lists based on an object's rarity.
*/
WORD GetItemListColor(HWND hwnd, int type, item_rarity_grade text_color_value)
{
	if (text_color_value != ITEM_RARITY_GRADE_NORMAL)
	{
		switch (text_color_value)
		{
			case ITEM_RARITY_GRADE_UNCOMMON:
				return COLOR_ITEM_TEXT_UNCOMMON;
			case ITEM_RARITY_GRADE_RARE:
				return COLOR_ITEM_TEXT_RARE;
			case ITEM_RARITY_GRADE_LEGENDARY:
				return COLOR_ITEM_TEXT_LEGENDARY;
			case ITEM_RARITY_GRADE_UNIDENTIFIED:
				return COLOR_ITEM_TEXT_UNIDENTIFIED;
			case ITEM_RARITY_GRADE_CURSED:
				return COLOR_ITEM_TEXT_CURSED;
		}
	}
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
	return 0;
}

/****************************************************************************/
/*
* GetPlayerNameColor:  Return color that player's name should be drawn in,
*   depending on player's object flags
*/
COLORREF GetPlayerNameColor(int flags,const char*name)
{
	if (GetDrawingEffect(flags) == OF_BLACK)
		return NAME_COLOR_BLACK_FG;

	switch (GetPlayerFlags(flags))
	{
		case PF_DM:
			return NAME_COLOR_DM_FG;
		case PF_KILLER:
			return NAME_COLOR_KILLER_FG;
		case PF_OUTLAW:
			return NAME_COLOR_OUTLAW_FG;
		case PF_CREATOR:
			return NAME_COLOR_CREATOR_FG;
		case PF_SUPER:
			return NAME_COLOR_SUPER_FG;
		case PF_EVENTCHAR:
			return NAME_COLOR_EVENT_FG;
            
    default:
			return NAME_COLOR_NORMAL_FG;
	}
}

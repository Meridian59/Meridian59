// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * font.c: Deal with creating, destroying, and retrieving fonts.
 */

#include "client.h"

#define MAX_FONTNAME 100         // Max length of a font name string

static HFONT fonts[MAXFONTS];
static LOGFONT logfonts[MAXFONTS];
static HFONT hDefaultFont;

/* Default font information structures */
static char fontinfo[][MAX_FONTNAME] = {
{ "-12,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman" },   /* FONT_LIST */
{ "-30,0,0,0,0,0,0,0,0,0,0,0,0,Heidelberg-Normal"},    /* FONT_TITLES */
{ "-12,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman" },   /* FONT_EDIT */
{ "-12,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman" },   /* FONT_MAIL */
{ "-16,0,0,0,400,0,0,0,0,3,2,1,2,Heidelberg-Normal" }, /* FONT_STATS */
{ "-14,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman"},    /* FONT_INPUT */
{ "-14,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman"},    /* FONT_SYSMSG */
//{ "-14,0,0,0,0,0,0,0,0,0,0,0,0,Times New Roman"},    /* FONT_LABELS */
{ "-19,0,0,0,0,0,0,0,0,0,0,0,0,Arial"},    /* FONT_LABELS */
{ "8,0,0,0,400,0,0,0,255,1,2,1,49,Terminal"},        /* FONT_ADMIN */
{ "-9,0,0,0,700,0,0,0,0,3,2,1,34,Arial" },           /* FONT_STATNUM */
{ "24,0,0,0,700,0,0,0,0,0,0,0,0,Arial" },          /* FONT_MAP_TITLE */
{ "12,0,0,0,400,0,0,0,0,0,0,0,0,Arial" },           /* FONT_MAP_LABEL */
{ "12,0,0,0,400,0,0,0,0,0,0,0,0,Arial" },           /* FONT_MAP_TEXT */
};

static char font_section[] = "Fonts";  /* Section for fonts in INI file */

/* local function prototypes */
UINT CALLBACK ChooseFontHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
Bool SetFont(WORD font, LOGFONT *lf);
void DestroyFont(WORD font);
HFONT GetFont(WORD font);

LOGFONT *GetLogfont(int fontNum)
{
   return &logfonts[fontNum];
}

Bool GetLogFont(WORD fontnum, LOGFONT *pLogFont)
{
   char str[MAX_FONTNAME], name[10], *ptr;
   char *separators = ",";
   int temp;
   LOGFONT lf;
   Bool success;

   sprintf(name, "Font%d", fontnum);
   GetPrivateProfileString(font_section, name, fontinfo[fontnum], str, MAX_FONTNAME, ini_file);

   success = True;
   if ((ptr = strtok(str, separators)) == NULL || sscanf(ptr, "%d", &lf.lfHeight) != 1)
      success = False;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfWidth) != 1)
      success = False;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfEscapement) != 1)
      success = False;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfOrientation) != 1)
      success = False;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfWeight) != 1)
      success = False;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfItalic = temp; /* 1 byte value */
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfUnderline = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfStrikeOut = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfCharSet = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfOutPrecision = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfClipPrecision = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfQuality = temp;
   if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
      success = False;
   else lf.lfPitchAndFamily = temp;
   if ((ptr = strtok(NULL, separators)) == NULL)	 
      success = False; 
   else strcpy(lf.lfFaceName, ptr);
   
   if (success)
      memcpy(pLogFont,&lf,sizeof(LOGFONT));
   return success;
}

/************************************************************************/
/*
 * FontsCreate:  Create all fonts for use in the game.
 *  If use_defaults is False, try to load fonts from INI file.
 *  Otherwise use default fonts.
 */
void FontsCreate(Bool use_defaults)
{
   WORD i;
   int temp;
   LOGFONT lf;
   char str[MAX_FONTNAME], name[10], *ptr;
   char *separators = ",";
   Bool success;

   hDefaultFont = (HFONT) GetStockObject(SYSTEM_FONT);

   /* Try to load fonts from private .INI file; if not there, use defaults */
   for (i=0; i < MAXFONTS; i++)
   {
      if (use_defaults)
	 strcpy(str, fontinfo[i]);
      else
      {
	 sprintf(name, "Font%d", i);
	 GetPrivateProfileString(font_section, name, fontinfo[i], str, MAX_FONTNAME, ini_file);
      }

      success = True;
      if ((ptr = strtok(str, separators)) == NULL || sscanf(ptr, "%d", &lf.lfHeight) != 1)
	 success = False;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfWidth) != 1)
	 success = False;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfEscapement) != 1)
	 success = False;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfOrientation) != 1)
	 success = False;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &lf.lfWeight) != 1)
	 success = False;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfItalic = temp; /* 1 byte value */
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfUnderline = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfStrikeOut = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfCharSet = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfOutPrecision = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfClipPrecision = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfQuality = temp;
      if ((ptr = strtok(NULL, separators)) == NULL || sscanf(ptr, "%d", &temp) != 1)
	 success = False;
      else lf.lfPitchAndFamily = temp;
      if ((ptr = strtok(NULL, separators)) == NULL)	 
	 success = False; 
      else strcpy(lf.lfFaceName, ptr);

      memcpy(&logfonts[i],&lf,sizeof(LOGFONT));
      if (success)
	 SetFont(i, &lf);
      else fonts[i] = hDefaultFont;
   }
}
/************************************************************************/
void DestroyFont(WORD font)
{
   if (font > MAXFONTS)
   {
      debug(("Illegal font #%u", font));
      return;
   }
   if (fonts[font] != hDefaultFont)
      DeleteObject(fonts[font]);
}
/************************************************************************/
void FontsDestroy(void)
{
   WORD i;

   /* Don't destroy system's stock font */
   for (i=0; i < MAXFONTS; i++)
      DestroyFont(i);
}
/************************************************************************/
HFONT GetFont(WORD font)
{
   if (font > MAXFONTS)
   {
      debug(("Illegal font #%u\n", font));
      return hDefaultFont;
   }
   return fonts[font];
}
/************************************************************************/
/*
 * SetFont:  Set given user font # using given font info.  
 *   Returns True on success; returns False and uses default font on failure.
 */
Bool SetFont(WORD font, LOGFONT *lf)
{
   if (font > MAXFONTS)
   {
      debug(("Illegal font #%u\n", font));
      return False;
   }

   fonts[font] = CreateFontIndirect(lf);
   if (fonts[font] == NULL)
   {
      fonts[font] = hDefaultFont;
      return False;
   }
   return True;
}
/************************************************************************/
/* 
 * FontsSave:  Save current font selections to our INI file.
 */
void FontsSave(void)
{
   int i;
   LOGFONT lf;
   char str[MAX_FONTNAME], name[10];

   for (i=0; i < MAXFONTS; i++)
   {
      /* Get LOGFONT structure for current font; use default on failure */
      if (GetObject(fonts[i], sizeof(LOGFONT), &lf) == 0)
	 GetObject(hDefaultFont, sizeof(LOGFONT), &lf);
      
      sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s", 
	      lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
	      lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut,
	      lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision,
	      lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
      sprintf(name, "Font%d", i);

      WritePrivateProfileString(font_section, name, str, ini_file);
   }
}
/************************************************************************/
void FontsRestoreDefaults(void)
{
   FontsDestroy();
   FontsCreate(True);
   MainChangeFont();

   ModuleEvent(EVENT_FONTCHANGED, -1, NULL);
}


/************************************************************************/
/*
 * UserSelectFont:  Bring up font chooser dialog to allow user to change
 *   given font.
 */
void UserSelectFont(WORD font)
{
   CHOOSEFONT cf;
   LOGFONT newfont;

   if (font > MAXFONTS)
   {
      debug(("Illegal font #%u", font));
      return;
   }

   /* Get LOGFONT structure for current font; use default on failure */
   if (GetObject(fonts[font], sizeof(LOGFONT), &newfont) == 0)
      GetObject(hDefaultFont, sizeof(LOGFONT), &newfont);

   memset(&cf, 0, sizeof(CHOOSEFONT));
   cf.lStructSize = sizeof(CHOOSEFONT);
   cf.hwndOwner   = hMain;
   cf.lpfnHook    = ChooseFontHookProc;
   cf.lpLogFont   = &newfont;
   cf.Flags       = CF_SCREENFONTS | CF_ENABLEHOOK | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
   if (ChooseFont(&cf) == 0)
      return;

   /* Make new font */
   DestroyFont(font);

   cf.lpLogFont->lfQuality = ANTIALIASED_QUALITY;
   SetFont(font, cf.lpLogFont);

   /* See if a module wants to intercept font change */
   if (ModuleEvent(EVENT_FONTCHANGED, font, cf.lpLogFont) == False)
      return;

   /* Update fonts on screen */
   MainChangeFont();
}
/************************************************************************/
/*
 * ChooseFontHookProc:  Intercept window messages of Choose Font common
 *   dialog.
 */
UINT CALLBACK ChooseFontHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, hMain);

      /* Hide Color combo box */
      ShowWindow(GetDlgItem(hDlg, stc4), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, cmb4), SW_HIDE);
      return TRUE;
   }
   return FALSE;
}
/************************************************************************/
/*
 * GetFontHeight:  Utility procedure to retrieve the height of the given font.
 */
int GetFontHeight(HFONT hFont)
{
   TEXTMETRIC tm;
   HFONT hOldFont;
   HDC hdc = GetDC(hMain);

   hOldFont = (HFONT) SelectObject(hdc, hFont);
   GetTextMetrics(hdc, &tm);
   SelectObject(hdc, hOldFont);
   ReleaseDC(hMain, hdc);
   
   return max(tm.tmHeight, 1);  // In case of divide by zero
}

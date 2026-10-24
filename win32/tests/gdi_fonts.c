/*
 * gdi_fonts.c:  Fonts realize as on Windows 11 at 96 dpi: the face GetTextFace
 *   gives, a missing face's substitute, and the text metrics, for what the
 *   client asks for, the stock and system fonts, and Windows' faces over sizes.
 *   The values are Windows' own, measured there (win32/tools/fontprobe.c).
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

static int failures;

struct Expected
{
   int height, weight, charset, precision, pitch;
   const char *face, *realized;
   int tm_height, ascent, descent, internal, average;
};

static const struct Expected expected[] = {
   // The client's (font.c, its meridian.ini files, its dialogs).
   {-12, 0, 0, 0, 0x00, "Times New Roman", "Times New Roman", 15, 12, 3, 3, 5},
   {-30, 0, 0, 0, 0x00, "Heidelbe-Normal", "MS Sans Serif", 36, 29, 7, 6, 15},
   {-16, 400, 0, 3, 0x02, "Heidelbe-Normal", "Arial", 18, 15, 3, 2, 7},
   {-14, 0, 0, 0, 0x00, "Times New Roman", "Times New Roman", 16, 13, 3, 2, 6},
   {-19, 0, 0, 0, 0x00, "Arial", "Arial", 22, 18, 4, 3, 8},
   {8, 400, 255, 1, 0x31, "Terminal", "Terminal", 8, 7, 1, 0, 6},
   {-9, 700, 0, 3, 0x22, "Arial", "Arial", 11, 9, 2, 2, 4},
   {24, 700, 0, 0, 0x00, "Arial", "Arial", 24, 19, 5, 4, 10},
   {12, 400, 0, 0, 0x00, "Arial", "Arial", 12, 9, 3, 3, 4},
   {-13, 700, 0, 3, 0x00, "Arial", "Arial", 16, 13, 3, 3, 6},
   {-12, 400, 0, 3, 0x22, "Arial", "Arial", 15, 12, 3, 3, 5},
   {-11, 400, 0, 3, 0x22, "Arial", "Arial", 14, 11, 3, 3, 5},
   {-15, 400, 0, 3, 0x22, "Arial", "Arial", 17, 14, 3, 2, 7},
   {-21, 500, 0, 3, 0x02, "Heidelberg-Normal", "Arial", 24, 19, 5, 3, 9},
   {-16, 500, 0, 3, 0x02, "Heidelberg-Normal", "MS Sans Serif", 20, 16, 4, 4, 8},
   {-11, 0, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 13, 11, 2, 2, 5},
   {-11, 0, 1, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 13, 11, 2, 2, 5},
   {-11, 0, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 13, 11, 2, 2, 5},
   {-11, 0, 0, 0, 0x00, "MS Shell Dlg 2", "MS Shell Dlg 2", 13, 11, 2, 2, 5},
   // A face that isn't installed: by weight, family and precision (but fixed
   // pitch's TrueType only at small sizes, Consolas there).
   {-12, 0, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 15, 12, 3, 3, 5},
   {-16, 0, 0, 0, 0x00, "Heidelbe-Normal", "MS Sans Serif", 20, 16, 4, 4, 8},
   {-30, 0, 0, 0, 0x00, "Heidelbe-Normal", "MS Sans Serif", 36, 29, 7, 6, 15},
   {-12, 0, 0, 7, 0x00, "Heidelbe-Normal", "Arial", 15, 12, 3, 3, 5},
   {-16, 0, 0, 7, 0x00, "Heidelbe-Normal", "Arial", 18, 15, 3, 2, 7},
   {-30, 0, 0, 7, 0x00, "Heidelbe-Normal", "Arial", 35, 28, 7, 5, 13},
   {-12, 0, 0, 0, 0x01, "Heidelbe-Normal", "Fixedsys", 15, 12, 3, 3, 8},
   {-16, 0, 0, 0, 0x01, "Heidelbe-Normal", "Courier", 16, 13, 3, 0, 9},
   {-30, 0, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 33, 25, 8, 3, 18},
   {-30, 0, 0, 7, 0x01, "Heidelbe-Normal", "Courier New", 33, 25, 8, 3, 18},
   {-12, 0, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 15, 12, 3, 3, 5},
   {-16, 0, 0, 0, 0x12, "Heidelbe-Normal", "MS Serif", 19, 15, 4, 3, 8},
   {-30, 0, 0, 0, 0x12, "Heidelbe-Normal", "MS Serif", 33, 26, 7, 3, 14},
   {-12, 0, 0, 7, 0x12, "Heidelbe-Normal", "Times New Roman", 15, 12, 3, 3, 5},
   {-16, 0, 0, 7, 0x12, "Heidelbe-Normal", "Times New Roman", 19, 15, 4, 3, 6},
   {-30, 0, 0, 7, 0x12, "Heidelbe-Normal", "Times New Roman", 34, 27, 7, 4, 12},
   {-30, 400, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 35, 28, 7, 5, 13},
   {-30, 700, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 35, 28, 7, 5, 14},
   {-16, 400, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 18, 15, 3, 2, 7},
   {-16, 700, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 19, 15, 4, 3, 8},
   {-12, 400, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 15, 12, 3, 3, 5},
   {-12, 700, 0, 0, 0x00, "Heidelbe-Normal", "Arial", 15, 12, 3, 3, 6},
   {-30, 400, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 33, 25, 8, 3, 18},
   {-30, 700, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 34, 25, 9, 4, 18},
   {-16, 400, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 18, 14, 4, 2, 10},
   {-16, 700, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 18, 13, 5, 2, 10},
   {-12, 400, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 15, 12, 3, 3, 7},
   {-12, 700, 0, 0, 0x01, "Heidelbe-Normal", "Courier New", 16, 12, 4, 4, 7},
   {-30, 400, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 34, 27, 7, 4, 12},
   {-30, 700, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 35, 28, 7, 5, 13},
   {-16, 400, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 19, 15, 4, 3, 6},
   {-16, 700, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 19, 15, 4, 3, 7},
   {-12, 400, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 15, 12, 3, 3, 5},
   {-12, 700, 0, 0, 0x12, "Heidelbe-Normal", "Times New Roman", 15, 12, 3, 3, 5},
   // Each face over sizes.
   {-30, 400, 0, 0, 0x00, "Arial", "Arial", 35, 28, 7, 5, 13},
   {-16, 400, 0, 0, 0x00, "Arial", "Arial", 18, 15, 3, 2, 7},
   {-11, 400, 0, 0, 0x00, "Arial", "Arial", 14, 11, 3, 3, 5},
   {0, 400, 0, 0, 0x00, "Arial", "Arial", 18, 15, 3, 2, 7},
   {24, 400, 0, 0, 0x00, "Arial", "Arial", 24, 19, 5, 3, 9},
   {-16, 700, 0, 0, 0x00, "Arial", "Arial", 19, 15, 4, 3, 8},
   {-30, 400, 0, 0, 0x00, "Courier", "Courier", 26, 22, 4, 0, 16},
   {-16, 400, 0, 0, 0x00, "Courier", "Courier", 16, 13, 3, 0, 9},
   {-11, 400, 0, 0, 0x00, "Courier", "Courier", 13, 11, 2, 0, 8},
   {0, 400, 0, 0, 0x00, "Courier", "Courier", 16, 13, 3, 0, 9},
   {24, 400, 0, 0, 0x00, "Courier", "Courier", 20, 16, 4, 0, 12},
   {-16, 700, 0, 0, 0x00, "Courier", "Courier", 16, 13, 3, 0, 10},
   {-30, 400, 0, 0, 0x00, "Courier New", "Courier New", 33, 25, 8, 3, 18},
   {-16, 400, 0, 0, 0x00, "Courier New", "Courier New", 18, 14, 4, 2, 10},
   {-11, 400, 0, 0, 0x00, "Courier New", "Courier New", 14, 11, 3, 3, 7},
   {0, 400, 0, 0, 0x00, "Courier New", "Courier New", 18, 14, 4, 2, 10},
   {24, 400, 0, 0, 0x00, "Courier New", "Courier New", 24, 18, 6, 2, 13},
   {-16, 700, 0, 0, 0x00, "Courier New", "Courier New", 18, 13, 5, 2, 10},
   {-30, 400, 0, 0, 0x00, "Fixedsys", "Fixedsys", 30, 24, 6, 6, 16},
   {-16, 400, 0, 0, 0x00, "Fixedsys", "Fixedsys", 15, 12, 3, 3, 8},
   {-11, 400, 0, 0, 0x00, "Fixedsys", "Fixedsys", 15, 12, 3, 3, 8},
   {0, 400, 0, 0, 0x00, "Fixedsys", "Fixedsys", 15, 12, 3, 3, 8},
   {24, 400, 0, 0, 0x00, "Fixedsys", "Fixedsys", 15, 12, 3, 3, 8},
   {-16, 700, 0, 0, 0x00, "Fixedsys", "Fixedsys", 15, 12, 3, 3, 9},
   {-30, 400, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 36, 29, 7, 6, 15},
   {-16, 400, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 20, 16, 4, 4, 8},
   {-11, 400, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 13, 11, 2, 2, 5},
   {0, 400, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 16, 13, 3, 3, 7},
   {24, 400, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 24, 19, 5, 6, 9},
   {-16, 700, 0, 0, 0x00, "MS Sans Serif", "MS Sans Serif", 20, 16, 4, 4, 9},
   {-30, 400, 0, 0, 0x00, "MS Serif", "MS Serif", 33, 26, 7, 3, 14},
   {-16, 400, 0, 0, 0x00, "MS Serif", "MS Serif", 19, 15, 4, 3, 8},
   {-11, 400, 0, 0, 0x00, "MS Serif", "MS Serif", 13, 11, 2, 2, 5},
   {0, 400, 0, 0, 0x00, "MS Serif", "MS Serif", 16, 13, 3, 3, 6},
   {24, 400, 0, 0, 0x00, "MS Serif", "MS Serif", 23, 18, 5, 3, 10},
   {-16, 700, 0, 0, 0x00, "MS Serif", "MS Serif", 19, 15, 4, 3, 9},
   {-30, 400, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 36, 29, 7, 6, 13},
   {-16, 400, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 20, 16, 4, 4, 7},
   {-11, 400, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 13, 11, 2, 2, 5},
   {0, 400, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 20, 16, 4, 4, 7},
   {24, 400, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 24, 19, 5, 5, 8},
   {-16, 700, 0, 0, 0x00, "MS Shell Dlg", "MS Shell Dlg", 20, 16, 4, 4, 8},
   {-30, 400, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 36, 29, 7, 6, 13},
   {-16, 400, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 20, 16, 4, 4, 7},
   {-11, 400, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 13, 11, 2, 2, 5},
   {0, 400, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 20, 16, 4, 4, 7},
   {24, 400, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 24, 19, 5, 5, 8},
   {-16, 700, 0, 0, 0x00, "Microsoft Sans Serif", "Microsoft Sans Serif", 20, 16, 4, 4, 8},
   {-30, 400, 0, 0, 0x00, "Segoe UI", "Segoe UI", 41, 33, 8, 11, 16},
   {-16, 400, 0, 0, 0x00, "Segoe UI", "Segoe UI", 21, 17, 4, 5, 9},
   {-11, 400, 0, 0, 0x00, "Segoe UI", "Segoe UI", 13, 11, 2, 2, 6},
   {0, 400, 0, 0, 0x00, "Segoe UI", "Segoe UI", 21, 17, 4, 5, 9},
   {24, 400, 0, 0, 0x00, "Segoe UI", "Segoe UI", 23, 18, 5, 6, 9},
   {-16, 700, 0, 0, 0x00, "Segoe UI", "Segoe UI", 21, 17, 4, 5, 9},
   {-30, 400, 0, 0, 0x00, "Small Fonts", "Small Fonts", 36, 30, 6, 6, 15},
   {-16, 400, 0, 0, 0x00, "Small Fonts", "Small Fonts", 20, 16, 4, 4, 8},
   {-11, 400, 0, 0, 0x00, "Small Fonts", "Small Fonts", 13, 11, 2, 2, 6},
   {0, 400, 0, 0, 0x00, "Small Fonts", "Small Fonts", 16, 14, 2, 2, 8},
   {24, 400, 0, 0, 0x00, "Small Fonts", "Small Fonts", 24, 20, 4, 4, 10},
   {-16, 700, 0, 0, 0x00, "Small Fonts", "Small Fonts", 20, 16, 4, 4, 10},
   {-30, 400, 0, 0, 0x00, "System", "System", 32, 26, 6, 6, 14},
   {-16, 400, 0, 0, 0x00, "System", "System", 16, 13, 3, 3, 7},
   {-11, 400, 0, 0, 0x00, "System", "System", 16, 13, 3, 3, 7},
   {0, 400, 0, 0, 0x00, "System", "System", 16, 13, 3, 3, 7},
   {24, 400, 0, 0, 0x00, "System", "System", 16, 13, 3, 3, 7},
   {-16, 700, 0, 0, 0x00, "System", "System", 16, 13, 3, 3, 7},
   {-30, 400, 0, 0, 0x00, "Tahoma", "Tahoma", 36, 30, 6, 6, 13},
   {-16, 400, 0, 0, 0x00, "Tahoma", "Tahoma", 19, 16, 3, 3, 7},
   {-11, 400, 0, 0, 0x00, "Tahoma", "Tahoma", 13, 11, 2, 2, 5},
   {0, 400, 0, 0, 0x00, "Tahoma", "Tahoma", 19, 16, 3, 3, 7},
   {24, 400, 0, 0, 0x00, "Tahoma", "Tahoma", 24, 20, 4, 4, 9},
   {-16, 700, 0, 0, 0x00, "Tahoma", "Tahoma", 19, 16, 3, 3, 8},
   {-30, 400, 255, 0, 0x00, "Terminal", "Terminal", 24, 20, 4, 0, 10},
   {-16, 400, 255, 0, 0x00, "Terminal", "Terminal", 16, 14, 2, 0, 12},
   {-11, 400, 255, 0, 0x00, "Terminal", "Terminal", 8, 7, 1, 0, 6},
   {0, 400, 255, 0, 0x00, "Terminal", "Terminal", 16, 14, 2, 0, 12},
   {24, 400, 255, 0, 0x00, "Terminal", "Terminal", 24, 20, 4, 0, 10},
   {-16, 700, 255, 0, 0x00, "Terminal", "Terminal", 16, 14, 2, 0, 13},
   {-30, 400, 0, 0, 0x00, "Times New Roman", "Times New Roman", 34, 27, 7, 4, 12},
   {-16, 400, 0, 0, 0x00, "Times New Roman", "Times New Roman", 19, 15, 4, 3, 6},
   {-11, 400, 0, 0, 0x00, "Times New Roman", "Times New Roman", 14, 11, 3, 3, 4},
   {0, 400, 0, 0, 0x00, "Times New Roman", "Times New Roman", 19, 15, 4, 3, 6},
   {24, 400, 0, 0, 0x00, "Times New Roman", "Times New Roman", 23, 18, 5, 2, 8},
   {-16, 700, 0, 0, 0x00, "Times New Roman", "Times New Roman", 19, 15, 4, 3, 7},
   {-30, 400, 0, 0, 0x00, "Verdana", "Verdana", 36, 30, 6, 6, 15},
   {-16, 400, 0, 0, 0x00, "Verdana", "Verdana", 18, 15, 3, 2, 8},
   {-11, 400, 0, 0, 0x00, "Verdana", "Verdana", 13, 11, 2, 2, 6},
   {0, 400, 0, 0, 0x00, "Verdana", "Verdana", 18, 15, 3, 2, 8},
   {24, 400, 0, 0, 0x00, "Verdana", "Verdana", 23, 19, 4, 4, 10},
   {-16, 700, 0, 0, 0x00, "Verdana", "Verdana", 18, 15, 3, 2, 9},
};

static void Check(const char *what, HDC dc, const char *realized, int height, int ascent, int descent, int internal,
                  int average)
{
   char face[LF_FACESIZE] = "";
   TEXTMETRICA tm;
   GetTextFaceA(dc, sizeof(face), face);
   GetTextMetricsA(dc, &tm);
   int ok = strcmp(face, realized) == 0 && tm.tmHeight == height && tm.tmAscent == ascent && tm.tmDescent == descent &&
            tm.tmInternalLeading == internal && tm.tmAveCharWidth == average;
   if (!ok)
   {
      printf("FAIL %s: %s %ld/%ld/%ld/%ld/%ld, Windows %s %d/%d/%d/%d/%d\n", what, face, (long) tm.tmHeight,
             (long) tm.tmAscent, (long) tm.tmDescent, (long) tm.tmInternalLeading, (long) tm.tmAveCharWidth, realized,
             height, ascent, descent, internal, average);
      failures++;
   }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   HDC dc = GetDC(NULL);
   for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
   {
      const struct Expected *e = &expected[i];
      HFONT f = CreateFontA(e->height, 0, 0, 0, e->weight, 0, 0, 0, e->charset, e->precision, 0, 0, e->pitch, e->face);
      HGDIOBJ old = SelectObject(dc, f);
      char what[96];
      snprintf(what, sizeof(what), "%s %d weight %d precision %d pitch %#x", e->face, e->height, e->weight,
               e->precision, e->pitch);
      Check(what, dc, e->realized, e->tm_height, e->ascent, e->descent, e->internal, e->average);
      SelectObject(dc, old);
      DeleteObject(f);
   }

   static const struct
   {
      int id;
      const char *name, *realized;
      int height, ascent, descent, internal, average;
   } stock[] = {
      {SYSTEM_FONT, "SYSTEM_FONT", "Arial", 16, 13, 3, 3, 6},
      {DEFAULT_GUI_FONT, "DEFAULT_GUI_FONT", "MS Shell Dlg", 13, 11, 2, 2, 5},
      {ANSI_VAR_FONT, "ANSI_VAR_FONT", "MS Sans Serif", 13, 11, 2, 2, 5},
      {ANSI_FIXED_FONT, "ANSI_FIXED_FONT", "Courier", 13, 11, 2, 0, 8},
      {OEM_FIXED_FONT, "OEM_FIXED_FONT", "Terminal", 12, 10, 2, 0, 8},
      {SYSTEM_FIXED_FONT, "SYSTEM_FIXED_FONT", "Fixedsys", 15, 12, 3, 3, 8},
   };
   for (size_t i = 0; i < sizeof(stock) / sizeof(stock[0]); i++)
   {
      HGDIOBJ old = SelectObject(dc, GetStockObject(stock[i].id));
      Check(stock[i].name, dc, stock[i].realized, stock[i].height, stock[i].ascent, stock[i].descent,
            stock[i].internal, stock[i].average);
      SelectObject(dc, old);
   }

   // Windows 11's system fonts: Segoe UI 9 points.
   NONCLIENTMETRICSA ncm = {sizeof(ncm)};
   SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   HFONT menu = CreateFontIndirectA(&ncm.lfMenuFont);
   HGDIOBJ old = SelectObject(dc, menu);
   Check("menu font", dc, "Segoe UI", 15, 12, 3, 3, 6);
   SelectObject(dc, old);
   DeleteObject(menu);

   ReleaseDC(NULL, dc);
   printf(failures ? "gdi_fonts: %d failed\n" : "gdi_fonts: all passed\n", failures);
   return failures != 0;
}

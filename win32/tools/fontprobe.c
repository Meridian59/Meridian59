/*
 * fontprobe.c:  Measures, on Windows, what GDI realizes for the faces the Win32
 *   layer substitutes; win32/tools/fonttables.py makes core/fonttables.inc from
 *   its output.  Build with mingw-w64 (x86_64-w64-mingw32-gcc fontprobe.c -lgdi32),
 *   run at 96 dpi:  fontprobe > probe.csv
 *
 *   size,face,charset,height,ascent,descent,internal,external,average,max,weight
 *   req,height,weight,charset,precision,pitch,face,realized,height,ascent,descent,
 *       internal,external,average,max,weight,overhang,pitch,charset
 *   os2,face,weight,upem,win ascent,win descent,average
 */

#include <windows.h>
#include <stdio.h>

static HDC dc;

static void Request(int h, int weight, int charset, const char *face)
{
   HFONT f = CreateFontA(h, 0, 0, 0, weight, 0, 0, 0, charset, 0, 0, 0, 0, face);
   HGDIOBJ old = SelectObject(dc, f);
   char realized[LF_FACESIZE] = "";
   TEXTMETRICA tm;
   GetTextFaceA(dc, sizeof(realized), realized);
   GetTextMetricsA(dc, &tm);
   printf("req,%d,%d,%d,0,0,%s,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%d,%d\n", h, weight, charset, face, realized,
          (long) tm.tmHeight, (long) tm.tmAscent, (long) tm.tmDescent, (long) tm.tmInternalLeading,
          (long) tm.tmExternalLeading, (long) tm.tmAveCharWidth, (long) tm.tmMaxCharWidth, (long) tm.tmWeight,
          (long) tm.tmOverhang, tm.tmPitchAndFamily, tm.tmCharSet);
   if (h == -100 && charset == 0)
   {
      unsigned char os2[78] = {0}, head[20] = {0};
      if (GetFontData(dc, 0x322f534f, 0, os2, sizeof(os2)) != GDI_ERROR &&
          GetFontData(dc, 0x64616568, 0, head, sizeof(head)) != GDI_ERROR)
         printf("os2,%s,%d,%u,%u,%u,%d\n", face, weight, (head[18] << 8) | head[19], (os2[74] << 8) | os2[75],
                (os2[76] << 8) | os2[77], (short) ((os2[2] << 8) | os2[3]));
   }
   SelectObject(dc, old);
   DeleteObject(f);
}

static int CALLBACK Size(const LOGFONTA *lf, const TEXTMETRICA *tm, DWORD type, LPARAM l)
{
   printf("size,%s,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", lf->lfFaceName, lf->lfCharSet, (long) tm->tmHeight,
          (long) tm->tmAscent, (long) tm->tmDescent, (long) tm->tmInternalLeading, (long) tm->tmExternalLeading,
          (long) tm->tmAveCharWidth, (long) tm->tmMaxCharWidth, (long) tm->tmWeight);
   return 1;
}

int main(void)
{
   static const char *raster[] = {"MS Sans Serif", "MS Serif", "Small Fonts", "Courier", "Fixedsys", "System",
                                  "Terminal"};
   static const char *truetype[] = {"Arial", "Times New Roman", "Courier New", "Tahoma", "Microsoft Sans Serif",
                                    "MS Shell Dlg", "Verdana", "Segoe UI"};
   dc = GetDC(NULL);
   if (GetDeviceCaps(dc, LOGPIXELSY) != 96)
   {
      fprintf(stderr, "fontprobe: run at 96 dpi\n");
      return 1;
   }
   for (int i = 0; i < 7; i++)
   {
      LOGFONTA lf = {0};
      lf.lfCharSet = DEFAULT_CHARSET;
      strcpy(lf.lfFaceName, raster[i]);
      EnumFontFamiliesExA(dc, &lf, (FONTENUMPROCA) Size, 0, 0);
   }
   for (int weight = 400; weight <= 700; weight += 300)
      for (int h = -100; h <= 100; h++)
      {
         for (int i = 0; i < 7; i++)
            Request(h, weight, i == 6 ? OEM_CHARSET : ANSI_CHARSET, raster[i]);
         for (int i = 0; i < 8; i++)
            Request(h, weight, ANSI_CHARSET, truetype[i]);
      }
   ReleaseDC(NULL, dc);
   return 0;
}

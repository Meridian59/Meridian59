/*
 * ctl_text.c:  The Unicode string and text functions the controls use: lstr*W,
 *   Char*W and IsChar*W on Latin-1, text extents (W against A, partial extents
 *   and fits), tabbed text, the usp10 subset (caret positions match drawing),
 *   window and viewport origins, DrawFrameControl, DrawState, ScrollWindowEx and
 *   ScrollDC, and the clipboard's synthesized formats.
 */

#include "tests/ctl_common.h"
#include <usp10.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   // Strings.
   WCHAR buf[64];
   const WCHAR hello[] = u"Héllo";
   CHECK_INT(lstrlenW(hello), 5);
   lstrcpyW(buf, hello);
   CHECK(lstrcmpW(buf, hello) == 0);
   lstrcpynW(buf, hello, 3);
   CHECK(lstrlenW(buf) == 2 && buf[1] == 0xE9);
   CHECK(lstrcmpiW(u"Été", u"éTÉ") == 0);
   CHECK(lstrcmpW(u"a", u"b") < 0);
   lstrcpyW(buf, u"straße été ÿ");
   CharUpperW(buf);
   CHECK(buf[0] == 'S' && buf[4] == 0xDF && buf[7] == 0xC9 && buf[11] == 0x178);
   CharLowerBuffW(buf, 3);
   CHECK(buf[0] == 's' && buf[3] == 'A');
   CHECK((WCHAR) (UINT_PTR) CharUpperW((LPWSTR) (UINT_PTR) 0xFC) == 0xDC);
   CHECK(IsCharAlphaW(0xE9) && IsCharAlphaW('z') && !IsCharAlphaW('3') && !IsCharAlphaW(0xD7));
   CHECK(IsCharUpperW(0xC4) && !IsCharUpperW(0xE4) && IsCharLowerW(0xE4));
   CHECK(CharNextW(hello) == hello + 1);
   CHECK(CharPrevW(hello, hello) == hello);
   char a[16] = "\xE4\xF6\xFC";
   CharUpperBuffA(a, 3);
   CHECK_STR(a, "\xC4\xD6\xDC");

   // Text extents: W as A, partial extents increasing, the fit.
   HDC hdc = CreateCompatibleDC(NULL);
   SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
   SIZE sa, sw;
   GetTextExtentPoint32A(hdc, "Gr\xFC\xDF" "e", 5, &sa);
   GetTextExtentPoint32W(hdc, u"Grüße", 5, &sw);
   CHECK(sa.cx == sw.cx && sa.cy == sw.cy && sa.cx > 20);
   INT dx[5], fit = -1;
   SIZE se;
   CHECK(GetTextExtentExPointW(hdc, u"Grüße", 5, sw.cx / 2, &fit, dx, &se));
   CHECK_INT(se.cx, sw.cx);
   CHECK_INT(dx[4], sw.cx);
   CHECK(dx[0] > 0 && dx[0] < dx[1] && dx[1] < dx[2] && dx[3] < dx[4]);
   CHECK(fit >= 1 && fit <= 4 && dx[fit - 1] <= sw.cx / 2 && dx[fit] > sw.cx / 2);
   INT w32[3];
   CHECK(GetCharWidth32W(hdc, 'a', 'c', w32));
   CHECK(w32[0] > 0 && w32[1] > 0);
   ABC abc;
   CHECK(GetCharABCWidthsW(hdc, 'W', 'W', &abc));
   CHECK((int) abc.abcB > 0);
   LONG height = 0;
   LONG avg = GdiGetCharDimensions(hdc, NULL, &height);
   CHECK(avg > 2 && avg < 12 && height == sa.cy);

   // Tabbed text: a tab to the default stop (8 average widths).
   TEXTMETRICW tm;
   GetTextMetricsW(hdc, &tm);
   DWORD e = GetTabbedTextExtentW(hdc, u"a\tb", 3, 0, NULL);
   SIZE sb;
   GetTextExtentPoint32W(hdc, u"b", 1, &sb);
   CHECK_INT(LOWORD(e), 8 * tm.tmAveCharWidth + sb.cx);
   INT stop = 100;
   e = GetTabbedTextExtentA(hdc, "a\tb", 3, 1, &stop);
   CHECK_INT(LOWORD(e), 100 + sb.cx);

   // usp10: caret positions are the partial extents; X to CP rounds by halves.
   SCRIPT_STRING_ANALYSIS ssa = NULL;
   CHECK(ScriptStringAnalyse(hdc, u"Grüße", 5, 16, -1, SSA_GLYPHS, -1, NULL, NULL, NULL, NULL, NULL, &ssa) == S_OK);
   int x;
   ScriptStringCPtoX(ssa, 2, FALSE, &x);
   CHECK_INT(x, dx[1]);
   ScriptStringCPtoX(ssa, 2, TRUE, &x);
   CHECK_INT(x, dx[2]);
   ScriptStringCPtoX(ssa, 5, FALSE, &x);
   CHECK_INT(x, sw.cx);
   int cp, trailing;
   ScriptStringXtoCP(ssa, dx[1] + 1, &cp, &trailing);
   CHECK(cp == 2 && trailing == 0);
   ScriptStringXtoCP(ssa, dx[2] - 1, &cp, &trailing);
   CHECK(cp == 2 && trailing == 1);
   ScriptStringXtoCP(ssa, sw.cx + 10, &cp, &trailing);
   CHECK(cp == 5 && trailing == 0);
   CHECK_INT(ScriptString_pSize(ssa)->cx, sw.cx);
   CHECK_INT(*ScriptString_pcOutChars(ssa), 5);
   ScriptStringFree(&ssa);
   CHECK(ssa == NULL);
   CHECK(ScriptStringAnalyse(hdc, u"", 0, 16, -1, SSA_GLYPHS, -1, NULL, NULL, NULL, NULL, NULL, &ssa) == E_INVALIDARG);
   const WCHAR star = '*';
   CHECK(ScriptStringAnalyse(hdc, &star, 4, 16, -1, SSA_PASSWORD, -1, NULL, NULL, NULL, NULL, NULL, &ssa) == S_OK);
   SIZE stars;
   GetTextExtentPoint32W(hdc, u"****", 4, &stars);
   CHECK_INT(ScriptString_pSize(ssa)->cx, stars.cx);
   ScriptStringFree(&ssa);
   SCRIPT_LOGATTR la[7];
   SCRIPT_ANALYSIS sa0 = {0};
   CHECK(ScriptBreak(u"ab cd e", 7, &sa0, la) == S_OK);
   CHECK(la[3].fSoftBreak && !la[1].fSoftBreak && la[2].fWhiteSpace && la[6].fSoftBreak);

   // Drawing into a DIB.
   BITMAPINFO bi = {0};
   bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   bi.bmiHeader.biWidth = 64;
   bi.bmiHeader.biHeight = -64;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   DWORD *bits;
   HBITMAP dib = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void **) &bits, NULL, 0);
   HGDIOBJ old = SelectObject(hdc, dib);
   RECT all = {0, 0, 64, 64};
   FillRect(hdc, &all, (HBRUSH) GetStockObject(WHITE_BRUSH));
   // Origins: the window origin moves logical (0, 0) left/up, the viewport right/down.
   SetWindowOrgEx(hdc, 10, 0, NULL);
   SetViewportOrgEx(hdc, 0, 20, NULL);
   SetPixel(hdc, 10, 0, RGB(255, 0, 0));
   CHECK((bits[20 * 64 + 0] & 0xFFFFFF) == 0xFF0000);
   POINT p = {0, 20};
   DPtoLP(hdc, &p, 1);
   CHECK(p.x == 10 && p.y == 0);
   LPtoDP(hdc, &p, 1);
   CHECK(p.x == 0 && p.y == 20);
   SetWindowOrgEx(hdc, 0, 0, NULL);
   SetViewportOrgEx(hdc, 0, 0, NULL);
   // Clipping regions round trip in device coordinates.
   HRGN r = CreateRectRgn(5, 5, 20, 20), got = CreateRectRgn(0, 0, 0, 0);
   CHECK_INT(GetClipRgn(hdc, got), 0);
   SelectClipRgn(hdc, r);
   CHECK_INT(GetClipRgn(hdc, got), 1);
   RECT box;
   GetRgnBox(got, &box);
   CHECK(box.left == 5 && box.right == 20);
   HRGN r2 = CreateRectRgn(10, 0, 64, 64);
   ExtSelectClipRgn(hdc, r2, RGN_AND);
   GetClipBox(hdc, &box);
   CHECK(box.left == 10 && box.right == 20 && box.top == 5);
   SelectClipRgn(hdc, NULL);
   DeleteObject(r);
   DeleteObject(r2);
   DeleteObject(got);
   // DrawFrameControl: a checked check box has dark pixels.
   FillRect(hdc, &all, (HBRUSH) GetStockObject(WHITE_BRUSH));
   RECT cb = {0, 0, 13, 13};
   DrawFrameControl(hdc, &cb, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_CHECKED);
   int dark = 0;
   for (int y = 3; y < 10; y++)
      for (int xx = 3; xx < 10; xx++)
         dark += (bits[y * 64 + xx] & 0xFFFFFF) == 0;
   CHECK(dark >= 8);
   // DrawState with DSS_DISABLED: the text in shadow and highlight, no black.
   FillRect(hdc, &all, GetSysColorBrush(COLOR_BTNFACE));
   SetBkMode(hdc, TRANSPARENT);
   DrawStateW(hdc, NULL, NULL, (LPARAM) u"Ab", 2, 2, 30, 0, 0, DST_TEXT | DSS_DISABLED);
   int shadow = 0, black = 0;
   for (int i = 0; i < 64 * 64; i++)
   {
      shadow += (bits[i] & 0xFFFFFF) == (DWORD) (GetRValue(GetSysColor(COLOR_BTNSHADOW)) * 0x10101);
      black += (bits[i] & 0xFFFFFF) == 0;
   }
   CHECK(shadow > 10);
   CHECK_INT(black, 0);
   // ScrollDC: a red pixel moves down 5; the uncovered strip comes back.
   FillRect(hdc, &all, (HBRUSH) GetStockObject(WHITE_BRUSH));
   SetPixel(hdc, 3, 3, RGB(255, 0, 0));
   RECT upd;
   HRGN ur = CreateRectRgn(0, 0, 0, 0);
   CHECK(ScrollDC(hdc, 0, 5, &all, &all, ur, &upd));
   CHECK((bits[8 * 64 + 3] & 0xFFFFFF) == 0xFF0000);
   CHECK(upd.top == 0 && upd.bottom == 5 && upd.right == 64);
   DeleteObject(ur);
   SelectObject(hdc, old);
   DeleteObject(dib);
   DeleteDC(hdc);

   // ScrollWindowEx: pixels move on the surface; the uncovered strip is invalid.
   HWND parent = MakeParent(100, 100);
   UpdateWindow(parent);
   Pump();
   HDC wdc = GetDC(parent);
   RECT pr = {0, 0, 100, 100};
   FillRect(wdc, &pr, (HBRUSH) GetStockObject(WHITE_BRUSH));
   SetPixel(wdc, 10, 10, RGB(0, 0, 255));
   ReleaseDC(parent, wdc);
   RECT invalid;
   CHECK_INT(ScrollWindowEx(parent, 0, -4, NULL, NULL, NULL, &invalid, SW_INVALIDATE), SIMPLEREGION);
   CHECK(invalid.top == 96 && invalid.bottom == 100);
   wdc = GetDC(parent);
   CHECK(GetPixel(wdc, 10, 6) == RGB(0, 0, 255));
   ReleaseDC(parent, wdc);
   RECT ur2;
   CHECK(GetUpdateRect(parent, &ur2, FALSE));
   CHECK(ur2.top == 96);
   DestroyWindow(parent);

   // The clipboard: CF_UNICODETEXT in, CF_TEXT and CF_OEMTEXT out.
   CHECK(OpenClipboard(NULL));
   EmptyClipboard();
   HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, 4 * sizeof(WCHAR));
   WCHAR *wp = (WCHAR *) GlobalLock(g);
   wp[0] = 0xE9, wp[1] = 'a', wp[2] = 0x20AC, wp[3] = 0;
   GlobalUnlock(g);
   SetClipboardData(CF_UNICODETEXT, g);
   CHECK(IsClipboardFormatAvailable(CF_TEXT));
   CHECK_STR((const char *) GlobalLock(GetClipboardData(CF_TEXT)), "\xE9" "a\x80");   // the euro is 0x80 in 1252
   CHECK(GetClipboardData(CF_OEMTEXT) != NULL);
   CHECK(CountClipboardFormats() == 3);
   CHECK(EnumClipboardFormats(0) == CF_UNICODETEXT);
   CloseClipboard();
   CHECK(!EmptyClipboard());   // not open
   UINT f = RegisterClipboardFormatA("M59 Thing");
   CHECK(f >= 0xC000 && f == RegisterClipboardFormatA("m59 thing"));

   return Report("ctl_text");
}

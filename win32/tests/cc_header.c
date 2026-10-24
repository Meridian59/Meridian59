/*
 * cc_header.c:  The header control (Wine's SysHeader32) on its own, with an
 *   ANSI parent: items (HDITEMA, Latin-1 text), formats (HDF_SORTUP/DOWN,
 *   HDF_BITMAP_ON_RIGHT), hit tests, clicks (HDN_ITEMCLICKA, not W), dragging a
 *   divider (HDN_BEGINTRACKA / TRACKA / ENDTRACKA, HDN_ITEMCHANGEDA), layout;
 *   renders by WM_PRINTCLIENT and checks the sort arrow and text are drawn.
 */

#include "cc_util.h"

static HWND header;
static int clicks_a, clicks_w, tracks, end_tracks, changed;
static int last_item = -1;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_NOTIFY && ((NMHDR *) lparam)->hwndFrom == header)
   {
      NMHEADERA *nm = (NMHEADERA *) lparam;
      switch (nm->hdr.code)
      {
      case HDN_ITEMCLICKA: clicks_a++; last_item = nm->iItem; break;
      case HDN_ITEMCLICKW: clicks_w++; break;
      case HDN_TRACKA: tracks++; break;
      case HDN_ENDTRACKA: end_tracks++; break;
      case HDN_ITEMCHANGEDA: changed++; break;
      }
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int Insert(int i, const char *text, int width, int fmt)
{
   HDITEMA item = {};
   item.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
   item.pszText = (LPSTR) text;
   item.cxy = width;
   item.fmt = fmt;
   return Header_InsertItem(header, i, &item);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 400, 100);
   header = CreateWindowExA(0, WC_HEADERA, "", WS_CHILD | WS_VISIBLE | HDS_BUTTONS | HDS_HORZ, 0, 0, 360, 24, parent,
                            (HMENU) 3, instance, NULL);
   CHECK(header != NULL);
   if (header == NULL)
      return CheckReport("cc_header");

   CHECK_INT(Insert(0, "Nome", 100, HDF_LEFT | HDF_STRING), 0);
   CHECK_INT(Insert(1, "Pre\xE7o", 80, HDF_RIGHT | HDF_STRING | HDF_SORTUP), 1);   // Preço
   CHECK_INT(Insert(2, "Gr\xF6\xDF" "e", 120, HDF_CENTER | HDF_STRING | HDF_SORTDOWN), 2);
   CHECK_INT(Header_GetItemCount(header), 3);

   char text[32];
   HDITEMA item = {};
   item.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
   item.pszText = text;
   item.cchTextMax = sizeof(text);
   CHECK(Header_GetItem(header, 1, &item));
   CHECK_STR(text, "Pre\xE7o");
   CHECK_INT(item.cxy, 80);
   CHECK(item.fmt & HDF_SORTUP);

   RECT r;
   CHECK(Header_GetItemRect(header, 2, &r));
   CHECK_INT(r.left, 180);
   CHECK_INT(r.right, 300);

   HDHITTESTINFO ht = {};
   ht.pt.x = 120;
   ht.pt.y = 10;
   CHECK_INT(SendMessageA(header, HDM_HITTEST, 0, (LPARAM) &ht), 1);
   CHECK(ht.flags & HHT_ONHEADER);
   ht.pt.x = 99;   // on the divider after item 0
   SendMessageA(header, HDM_HITTEST, 0, (LPARAM) &ht);
   CHECK(ht.flags & HHT_ONDIVIDER);

   // HDM_LAYOUT: where the header goes in a rectangle.
   RECT area = {0, 0, 360, 200};
   WINDOWPOS wp = {};
   HDLAYOUT layout = {&area, &wp};
   CHECK(Header_Layout(header, &layout));
   CHECK(wp.cy > 10 && wp.cy < 40);
   CHECK_INT(area.top, wp.cy);

   // A click on item 2: HDN_ITEMCLICKA to the ANSI parent.
   SendMessageA(header, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(240, 10));
   SendMessageA(header, WM_LBUTTONUP, 0, MAKELPARAM(240, 10));
   CHECK_INT(clicks_a, 1);
   CHECK_INT(clicks_w, 0);
   CHECK_INT(last_item, 2);

   // Dragging the divider after item 0 by 30 pixels widens it.
   SendMessageA(header, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(99, 10));
   SendMessageA(header, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(115, 10));
   SendMessageA(header, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(129, 10));
   SendMessageA(header, WM_LBUTTONUP, 0, MAKELPARAM(129, 10));
   CHECK(tracks > 0);
   CHECK_INT(end_tracks, 1);
   CHECK(changed > 0);
   item.mask = HDI_WIDTH;
   Header_GetItem(header, 0, &item);
   CHECK_INT(item.cxy, 130);

   // Rendering: button faces, the sort arrows (shadow-colored triangles), text.
   Canvas32 c = MakeCanvas(360, 24, RGB(255, 0, 255));
   SendMessageA(header, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   RECT all = {0, 0, 360, 24};
   CHECK(CountColor(&c, all, GetSysColor(COLOR_BTNFACE), 0) > 360 * 24 / 2);
   Header_GetItemRect(header, 1, &r);
   CHECK(CountDark(&c, r, 80) > 10);   // "Preço"
   // Wine's header draws sort arrows on its window DC, whatever DC it paints:
   // count gray-text pixels there, items 1 and 2 (arrows) against item 0 (none).
   HDC wdc = GetDC(header);
   int gray[3] = {0, 0, 0};
   for (int i = 0; i < 3; i++)
   {
      Header_GetItemRect(header, i, &r);
      for (int y = r.top + 2; y < r.bottom - 2; y++)
         for (int x = r.left + 2; x < r.right - 2; x++)
            if (GetPixel(wdc, x, y) == GetSysColor(COLOR_GRAYTEXT))
            {
               gray[i]++;
               SetPixel(c.dc, x, y, GetSysColor(COLOR_GRAYTEXT));
            }
   }
   ReleaseDC(header, wdc);
   printf("arrow pixels: %d %d %d\n", gray[0], gray[1], gray[2]);
   CHECK_INT(gray[0], 0);
   CHECK(gray[1] >= 10);
   CHECK(gray[2] >= 10);
   DumpBmp(&c, "cc_header.bmp");
   FreeCanvas(&c);

   CHECK(Header_DeleteItem(header, 0));
   CHECK_INT(Header_GetItemCount(header), 2);
   DestroyWindow(parent);
   return CheckReport("cc_header");
}

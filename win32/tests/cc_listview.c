/*
 * cc_listview.c:  The list view (Wine's, SysListView32) as the client uses it:
 *   report view with columns, items and subitems with Latin-1 text, sorting with
 *   callbacks (SortItems by lParam, SortItemsEx by index reading item text back
 *   as the client's mail reader does), selection state, hit testing, FindItem,
 *   extended styles, colors, the header's sort bitmaps, and notifications
 *   arriving in ANSI form at an ANSI parent (LVN_GETDISPINFOA, LVN_ITEMCHANGED,
 *   LVN_COLUMNCLICK, LVN_KEYDOWN, NM_CLICK).  Renders it with WM_PRINTCLIENT,
 *   checks pixels and writes cc_listview.bmp.
 */

#include "cc_util.h"

static HWND list;
static int notify_format_queries;
static int got_itemchanged, got_columnclick, got_click, got_keydown, got_dispinfo_a, got_dispinfo_w;
static int last_changed_item = -1;
static UINT last_new_state;
static int last_column = -1, last_click_item = -1;
static WORD last_vkey;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_NOTIFYFORMAT:
      if (lparam == NF_QUERY)
         notify_format_queries++;
      break;
   case WM_NOTIFY:
   {
      NMHDR *hdr = (NMHDR *) lparam;
      if (hdr->hwndFrom != list)
         break;
      switch (hdr->code)
      {
      case LVN_ITEMCHANGED:
      {
         NM_LISTVIEW *nm = (NM_LISTVIEW *) lparam;
         if (nm->uChanged & LVIF_STATE)
         {
            got_itemchanged++;
            last_changed_item = nm->iItem;
            last_new_state = nm->uNewState;
         }
         return 0;
      }
      case LVN_COLUMNCLICK:
         got_columnclick++;
         last_column = ((NM_LISTVIEW *) lparam)->iSubItem;
         return 0;
      case NM_CLICK:
         got_click++;
         last_click_item = ((NMITEMACTIVATE *) lparam)->iItem;
         return 0;
      case LVN_KEYDOWN:
         got_keydown++;
         last_vkey = ((NMLVKEYDOWN *) lparam)->wVKey;
         return 0;
      case LVN_GETDISPINFOA:
      {
         got_dispinfo_a++;
         NMLVDISPINFOA *di = (NMLVDISPINFOA *) lparam;
         if (di->item.mask & LVIF_TEXT)
            lstrcpynA(di->item.pszText, "Gr\xF6\xDF" "e", di->item.cchTextMax);   // "Größe"
         return 0;
      }
      case LVN_GETDISPINFOW:
         got_dispinfo_w++;
         return 0;
      }
      break;
   }
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int CALLBACK CompareParamDescending(LPARAM a, LPARAM b, LPARAM sort)
{
   CHECK_INT(sort, 1234);
   return (int) (b - a);
}

// As the client's CompareMailListItems: indices, text read back during the sort.
static int CALLBACK CompareTextByIndex(LPARAM a, LPARAM b, LPARAM sort)
{
   char ta[64], tb[64];
   ListView_GetItemText(list, (int) a, (int) sort, ta, sizeof(ta));
   ListView_GetItemText(list, (int) b, (int) sort, tb, sizeof(tb));
   return lstrcmpiA(ta, tb);
}

static int ItemParam(int i)
{
   LVITEMA item = {};
   item.mask = LVIF_PARAM;
   item.iItem = i;
   ListView_GetItem(list, &item);
   return (int) item.lParam;
}

static void ItemText(int i, int sub, char *out, int size)
{
   out[0] = 0;
   ListView_GetItemText(list, i, sub, out, size);
}

static const char *names[] = {"Caf\xE9", "M\xFCller", "Alpha", "Zeta", "Beta"};   // Café, Müller

static void Populate(void)
{
   LVCOLUMNA col = {};
   col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   const char *titles[] = {"Name", "Number", "Note"};
   int widths[] = {90, 60, 120};
   for (int i = 0; i < 3; i++)
   {
      col.pszText = (LPSTR) titles[i];
      col.cx = widths[i];
      col.iSubItem = i;
      CHECK_INT(ListView_InsertColumn(list, i, &col), i);
   }
   for (int i = 0; i < 5; i++)
   {
      LVITEMA item = {};
      item.mask = LVIF_TEXT | LVIF_PARAM;
      item.iItem = i;
      item.pszText = (LPSTR) names[i];
      item.lParam = 100 + i;
      CHECK_INT(ListView_InsertItem(list, &item), i);
      char number[16];
      sprintf(number, "%d", 10 * (i + 1));
      ListView_SetItemText(list, i, 1, number);
      LVITEMA sub = {};
      sub.mask = LVIF_TEXT;
      sub.iItem = i;
      sub.iSubItem = 2;
      sub.pszText = (LPSTR) (i == 1 ? "\xFC" "ber" : "note");   // über
      CHECK(ListView_SetItem(list, &sub));
   }
}

static void TestItems(void)
{
   char text[64];
   CHECK_INT(ListView_GetItemCount(list), 5);
   ItemText(0, 0, text, sizeof(text));
   CHECK_STR(text, "Caf\xE9");
   ItemText(1, 0, text, sizeof(text));
   CHECK_STR(text, "M\xFCller");
   ItemText(1, 2, text, sizeof(text));
   CHECK_STR(text, "\xFC" "ber");
   ItemText(3, 1, text, sizeof(text));
   CHECK_STR(text, "40");
   CHECK_INT(ItemParam(4), 104);

   // LVM_GETITEMA with a buffer: text and its length.
   LVITEMA item = {};
   item.mask = LVIF_TEXT;
   item.iItem = 0;
   item.pszText = text;
   item.cchTextMax = sizeof(text);
   CHECK(ListView_GetItem(list, &item));
   CHECK_STR(text, "Caf\xE9");

   // Header items follow the columns.
   HWND header = ListView_GetHeader(list);
   CHECK(header != NULL);
   CHECK_INT(Header_GetItemCount(header), 3);
   HDITEMA hdi = {};
   char htext[32];
   hdi.mask = HDI_TEXT | HDI_WIDTH;
   hdi.pszText = htext;
   hdi.cchTextMax = sizeof(htext);
   CHECK(Header_GetItem(header, 1, &hdi));
   CHECK_STR(htext, "Number");
   CHECK_INT(hdi.cxy, 60);

   // A callback item asks the ANSI parent in ANSI.
   LVITEMA cb = {};
   cb.mask = LVIF_TEXT | LVIF_PARAM;
   cb.iItem = 5;
   cb.pszText = LPSTR_TEXTCALLBACKA;
   cb.lParam = 105;
   CHECK_INT(ListView_InsertItem(list, &cb), 5);
   ItemText(5, 0, text, sizeof(text));
   CHECK_STR(text, "Gr\xF6\xDF" "e");
   CHECK(got_dispinfo_a > 0);
   CHECK_INT(got_dispinfo_w, 0);
   CHECK(ListView_DeleteItem(list, 5));
   CHECK_INT(ListView_GetItemCount(list), 5);
   CHECK(notify_format_queries > 0);
}

static void TestSort(void)
{
   CHECK(ListView_SortItems(list, CompareParamDescending, 1234));
   for (int i = 0; i < 5; i++)
      CHECK_INT(ItemParam(i), 104 - i);
   char text[64];
   ItemText(0, 0, text, sizeof(text));
   CHECK_STR(text, "Beta");   // param 104

   // By text of column 0, through indices: Alpha Beta Café Müller Zeta.
   CHECK(ListView_SortItemsEx(list, CompareTextByIndex, 0));
   const char *sorted[] = {"Alpha", "Beta", "Caf\xE9", "M\xFCller", "Zeta"};
   for (int i = 0; i < 5; i++)
   {
      ItemText(i, 0, text, sizeof(text));
      CHECK_STR(text, sorted[i]);
   }
   // Subitems moved with their items.
   ItemText(3, 2, text, sizeof(text));
   CHECK_STR(text, "\xFC" "ber");
   ItemText(0, 1, text, sizeof(text));
   CHECK_STR(text, "30");
}

static void TestSelection(void)
{
   got_itemchanged = 0;
   ListView_SetItemState(list, 2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
   CHECK(got_itemchanged > 0);
   CHECK_INT(last_changed_item, 2);
   CHECK(last_new_state & LVIS_SELECTED);
   CHECK_INT(ListView_GetNextItem(list, -1, LVNI_SELECTED), 2);
   CHECK_INT(ListView_GetSelectedCount(list), 1);
   CHECK_INT(ListView_GetItemState(list, 2, LVIS_SELECTED | LVIS_FOCUSED), LVIS_SELECTED | LVIS_FOCUSED);
   // LVS_SINGLESEL: selecting another deselects.
   ListView_SetItemState(list, 4, LVIS_SELECTED, LVIS_SELECTED);
   CHECK_INT(ListView_GetSelectedCount(list), 1);
   CHECK_INT(ListView_GetNextItem(list, -1, LVNI_SELECTED), 4);
   // Clear (as the client's ListView_ClearSelection: state 0 for all).
   ListView_SetItemState(list, -1, 0, LVIS_SELECTED);
   CHECK_INT(ListView_GetSelectedCount(list), 0);
}

static void TestFindAndHit(void)
{
   LVFINDINFOA fi = {};
   fi.flags = LVFI_STRING;
   fi.psz = "M\xFCller";
   CHECK_INT(ListView_FindItem(list, -1, &fi), 3);
   fi.flags = LVFI_PARTIAL | LVFI_STRING | LVFI_WRAP;
   fi.psz = "ca";
   CHECK_INT(ListView_FindItem(list, -1, &fi), 2);
   fi.flags = LVFI_PARAM;
   fi.lParam = 103;
   CHECK_INT(ListView_FindItem(list, -1, &fi), 4);   // Zeta

   RECT r;
   CHECK(ListView_GetItemRect(list, 1, &r, LVIR_BOUNDS));
   CHECK(r.bottom > r.top && r.right > r.left);
   LVHITTESTINFO ht = {};
   ht.pt.x = 10;
   ht.pt.y = (r.top + r.bottom) / 2;
   CHECK_INT(ListView_HitTest(list, &ht), 1);
   CHECK(ht.flags & LVHT_ONITEM);
   ht.pt.y = 1000;
   CHECK_INT(ListView_HitTest(list, &ht), -1);
   CHECK(ListView_EnsureVisible(list, 4, FALSE));
}

static void TestStyles(void)
{
   ListView_SetExtendedListViewStyleEx(list, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
   CHECK(ListView_GetExtendedListViewStyle(list) & LVS_EX_FULLROWSELECT);
   CHECK(ListView_SetBkColor(list, RGB(250, 240, 200)));
   CHECK_INT(ListView_GetBkColor(list), RGB(250, 240, 200));
   CHECK(ListView_SetTextBkColor(list, RGB(250, 240, 200)));
   CHECK(ListView_SetTextColor(list, RGB(0, 0, 128)));
   CHECK_INT(ListView_GetTextColor(list), RGB(0, 0, 128));

   // Sort arrow on column 1's header, as the client's ListView_SetHeaderSortImage.
   HWND header = ListView_GetHeader(list);
   HBITMAP arrow = CreateBitmap(8, 8, 1, 32, NULL);
   {
      HDC dc = CreateCompatibleDC(NULL);
      HGDIOBJ old = SelectObject(dc, arrow);
      RECT a = {0, 0, 8, 8};
      HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
      FillRect(dc, &a, red);
      DeleteObject(red);
      SelectObject(dc, old);
      DeleteDC(dc);
   }
   for (int i = 0; i < Header_GetItemCount(header); i++)
   {
      HDITEMA hdi = {};
      hdi.mask = HDI_FORMAT | HDI_BITMAP;
      CHECK(Header_GetItem(header, i, &hdi));
      if (i == 1)
      {
         hdi.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
         hdi.hbm = arrow;
      }
      else
      {
         hdi.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
         hdi.hbm = NULL;
      }
      CHECK(Header_SetItem(header, i, &hdi));
   }
   HDITEMA check = {};
   check.mask = HDI_FORMAT | HDI_BITMAP;
   Header_GetItem(header, 1, &check);
   CHECK(check.fmt & HDF_BITMAP);
   CHECK(check.hbm == arrow);
}

static void Click(HWND hwnd, int x, int y)
{
   // The button goes up while the control tracks the press (its own loop).
   PostMessageA(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
   SendMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
   Pump(20);
}

static void TestInput(void)
{
   // A click on item 1: selected, NM_CLICK with the item.
   RECT r;
   ListView_GetItemRect(list, 1, &r, LVIR_BOUNDS);
   got_click = 0;
   Click(list, 30, (r.top + r.bottom) / 2);
   CHECK(got_click > 0);
   CHECK_INT(last_click_item, 1);
   CHECK_INT(ListView_GetNextItem(list, -1, LVNI_SELECTED), 1);

   // Down arrow: LVN_KEYDOWN, then the selection moves.
   got_keydown = 0;
   SendMessageA(list, WM_KEYDOWN, VK_DOWN, 0);
   CHECK(got_keydown > 0);
   CHECK_INT(last_vkey, VK_DOWN);
   CHECK_INT(ListView_GetNextItem(list, -1, LVNI_SELECTED), 2);

   // A click on the second column's header: LVN_COLUMNCLICK with iSubItem 1.
   HWND header = ListView_GetHeader(list);
   RECT hr;
   CHECK(Header_GetItemRect(header, 1, &hr));
   got_columnclick = 0;
   SendMessageA(header, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM((hr.left + hr.right) / 2, (hr.top + hr.bottom) / 2));
   SendMessageA(header, WM_LBUTTONUP, 0, MAKELPARAM((hr.left + hr.right) / 2, (hr.top + hr.bottom) / 2));
   Pump(20);
   CHECK_INT(got_columnclick, 1);
   CHECK_INT(last_column, 1);
}

static void TestRender(HWND parent)
{
   RECT cr;
   GetClientRect(list, &cr);
   Canvas32 c = MakeCanvas(cr.right, cr.bottom, RGB(255, 0, 255));
   // Wine's list view prints its items only (WM_PRINTCLIENT is a partial stub there).
   SendMessageA(list, WM_ERASEBKGND, (WPARAM) c.dc, 0);
   SendMessageA(list, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT | PRF_ERASEBKGND);
   HWND header = ListView_GetHeader(list);
   RECT hr;
   GetWindowRect(header, &hr);
   MapWindowPoints(NULL, list, (POINT *) &hr, 2);
   POINT old;
   SetViewportOrgEx(c.dc, hr.left, hr.top, &old);
   SendMessageA(header, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT | PRF_ERASEBKGND);
   SetViewportOrgEx(c.dc, old.x, old.y, NULL);

   // The header: button face, with the red sort bitmap in column 1.
   RECT hdr = {0, hr.top, cr.right, hr.bottom};
   CHECK(hr.bottom - hr.top > 10);
   CHECK(CountColor(&c, hdr, GetSysColor(COLOR_BTNFACE), 0) > 100);
   // Wine's header draws item bitmaps on its window DC, whatever DC it paints.
   RECT h1;
   Header_GetItemRect(header, 1, &h1);
   HDC wdc = GetDC(header);
   int red = 0;
   for (int y = h1.top; y < h1.bottom; y++)
      for (int x = h1.left; x < h1.right; x++)
         if (GetPixel(wdc, x, y) == RGB(255, 0, 0))
         {
            red++;
            SetPixel(c.dc, x + hr.left, y + hr.top, RGB(255, 0, 0));
         }
   ReleaseDC(header, wdc);
   CHECK_INT(red, 64);
   DumpBmp(&c, "cc_listview.bmp");
   // The header's text is dark.
   CHECK(CountDark(&c, hdr, 80) > 20);

   // Rows: the background color; the selected row (2) highlighted across all
   // columns (full row select), with highlight-text-colored text.
   RECT row0, row2;
   ListView_GetItemRect(list, 0, &row0, LVIR_BOUNDS);
   ListView_GetItemRect(list, 2, &row2, LVIR_BOUNDS);
   CHECK(CountColor(&c, row0, RGB(250, 240, 200), 0) > (row0.right - row0.left) * (row0.bottom - row0.top) / 2);
   CHECK(CountColor(&c, row0, RGB(0, 0, 128), 60) > 10);   // text in the text color
   int hl = CountColor(&c, row2, GetSysColor(COLOR_HIGHLIGHT), 0);
   printf("highlight pixels in row 2: %d of %d (focus %d)\n", hl,
          (int) ((row2.right - row2.left) * (row2.bottom - row2.top)), GetFocus() == list);
   // Highlighted when focused; LVS_SHOWSELALWAYS: button face when not.
   COLORREF sel = GetFocus() == list ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_BTNFACE);
   RECT sub2 = row2;
   sub2.left = 90 + 60 + 5;   // in the Note column
   sub2.right = sub2.left + 20;
   CHECK(CountColor(&c, sub2, sel, 0) > 0);
   // Below the items: background only.
   RECT below = {0, row2.bottom + 3 * (row2.bottom - row2.top) + 2, cr.right, cr.bottom};
   if (below.top < below.bottom)
      CHECK_INT(CountColor(&c, below, RGB(250, 240, 200), 0), (below.right - below.left) * (below.bottom - below.top));
   FreeCanvas(&c);
   (void) parent;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 400, 300);
   CHECK(parent != NULL);
   list = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
                          WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
                          10, 10, 300, 200, parent, (HMENU) 100, instance, NULL);
   CHECK(list != NULL);
   if (list == NULL)
      return CheckReport("cc_listview");
   CHECK(!IsWindowUnicode(parent));
   Populate();
   TestItems();
   TestSort();
   TestSelection();
   TestFindAndHit();
   TestStyles();
   SetFocus(list);
   TestInput();
   TestRender(parent);
   DestroyWindow(parent);
   return CheckReport("cc_listview");
}

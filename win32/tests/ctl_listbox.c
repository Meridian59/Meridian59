/*
 * ctl_listbox.c:  Wine's ListBox on the layer, through windowsx.h as the client
 *   uses it: strings (sorted, Latin-1), 64-bit item data, selection by message,
 *   mouse and keyboard with LBN_SELCHANGE/LBN_DBLCLK, multiple selection, top and
 *   caret index, owner-drawn fixed and variable lists with and without strings
 *   (WM_MEASUREITEM/WM_DRAWITEM data), the vertical scroll bar, an ANSI
 *   subclass, and rendering.
 */

#include "tests/ctl_common.h"

static WNDPROC old_list_proc;
static int subclass_keys;

static LRESULT CALLBACK ListSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_KEYDOWN)
      subclass_keys++;
   return CallWindowProcA(old_list_proc, hwnd, msg, wparam, lparam);
}

struct Thing
{
   int value;
   char name[16];
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(400, 400);
   HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
   char text[128];

   // Strings, sorted.
   HWND list = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
                               WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | LBS_NOINTEGRALHEIGHT, 10,
                               10, 150, 100, parent, (HMENU) 301, instance, NULL);
   CHECK(list != NULL);
   SendMessageA(list, WM_SETFONT, (WPARAM) font, FALSE);
   CHECK_INT(ListBox_AddString(list, "pear"), 0);
   CHECK_INT(ListBox_AddString(list, "apple"), 0);
   CHECK_INT(ListBox_AddString(list, "\xFC" "ber"), 2);
   ListBox_AddString(list, "banana");
   CHECK_INT(ListBox_GetCount(list), 4);
   ListBox_GetText(list, 0, text);
   CHECK_STR(text, "apple");
   ListBox_GetText(list, 1, text);
   CHECK_STR(text, "banana");
   int ae = ListBox_FindString(list, -1, "\xFC" "b");
   CHECK(ae >= 0);
   ListBox_GetText(list, ae, text);
   CHECK_STR(text, "\xFC" "ber");
   CHECK_INT(ae, 3);
   CHECK_INT(ListBox_GetTextLen(list, ae), 4);
   CHECK_INT(ListBox_FindString(list, -1, "ban"), 1);
   CHECK_INT(ListBox_FindStringExact(list, -1, "BANANA"), 1);
   CHECK_INT(ListBox_FindString(list, -1, "zzz"), LB_ERR);

   // LB_SETSEL is for multiple selection: a single-selection list keeps its
   // selection (the client "deselects" its offer amounts' list so).
   CHECK_INT(ListBox_SetSel(list, FALSE, 1), LB_ERR);
   CHECK_INT(ListBox_GetCurSel(list), LB_ERR);
   ListBox_SetCurSel(list, 0);
   CHECK_INT(ListBox_SetSel(list, TRUE, 2), LB_ERR);
   CHECK_INT(ListBox_GetCurSel(list), 0);
   ListBox_SetCurSel(list, -1);

   // 64-bit item data.
   struct Thing things[3] = {{1, "one"}, {2, "two"}, {3, "three"}};
   ListBox_SetItemData(list, 1, &things[1]);
   CHECK(ListBox_GetItemData(list, 1) == (LRESULT) &things[1]);
   ListBox_SetItemData(list, 2, (LPARAM) 0x123456789ABCDEF0ull);
   CHECK(ListBox_GetItemData(list, 2) == (LRESULT) 0x123456789ABCDEF0ull);

   // Selection by message; LBN_SELCHANGE comes only from the user.
   memset(&rcv, 0, sizeof(rcv));
   CHECK_INT(ListBox_GetCurSel(list), LB_ERR);
   ListBox_SetCurSel(list, 2);
   CHECK_INT(ListBox_GetCurSel(list), 2);
   CHECK_INT(rcv.commands, 0);
   // By mouse: a click on the second item.
   int h = ListBox_GetItemHeight(list, 0);
   CHECK(h >= 13 && h <= 20);
   Click(list, 10, h + h / 2);
   CHECK_INT(ListBox_GetCurSel(list), 1);
   CHECK_INT(rcv.last_code, LBN_SELCHANGE);
   CHECK_INT(rcv.last_id, 301);
   // A double click.
   memset(&rcv, 0, sizeof(rcv));
   SendMessageA(list, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(10, h / 2));
   SendMessageA(list, WM_LBUTTONUP, 0, MAKELPARAM(10, h / 2));
   CHECK(rcv.code_count >= 1 && rcv.codes[rcv.code_count - 1] == LBN_DBLCLK);
   // By keyboard.
   SetFocus(list);
   memset(&rcv, 0, sizeof(rcv));
   Key(list, VK_DOWN);   // a double click doesn't select (its button down would)
   CHECK_INT(ListBox_GetCurSel(list), 2);
   CHECK_INT(rcv.last_code, LBN_SELCHANGE);
   Key(list, VK_END);
   CHECK_INT(ListBox_GetCurSel(list), 3);
   CHECK_INT(ListBox_GetCaretIndex(list), 3);
   // Typing a letter selects the next item starting with it.
   SendMessageA(list, WM_CHAR, 'b', 1);
   CHECK_INT(ListBox_GetCurSel(list), 1);

   // Insert, delete, reset.
   CHECK_INT(ListBox_InsertString(list, 0, "zz first"), 0);   // insert ignores sorting
   CHECK_INT(ListBox_GetCount(list), 5);
   CHECK_INT(ListBox_DeleteString(list, 0), 4);
   ListBox_GetText(list, 0, text);
   CHECK_STR(text, "apple");

   // Many items: scrolling with the vertical bar.
   for (int i = 0; i < 30; i++)
   {
      char item[16];
      sprintf(item, "item %02d", i);
      ListBox_AddString(list, item);
   }
   CHECK_INT(ListBox_GetTopIndex(list), 0);
   ListBox_SetTopIndex(list, 10);
   CHECK_INT(ListBox_GetTopIndex(list), 10);
   SCROLLINFO si = {sizeof(si), SIF_ALL};
   CHECK(GetScrollInfo(list, SB_VERT, &si));
   CHECK_INT(si.nPos, 10);
   CHECK_INT(si.nMax, 33);
   CHECK(si.nPage >= 5 && si.nPage <= 8);
   CHECK(GetWindowLongA(list, GWL_STYLE) & WS_VSCROLL);
   SendMessageA(list, WM_VSCROLL, SB_LINEDOWN, 0);
   CHECK_INT(ListBox_GetTopIndex(list), 11);
   SendMessageA(list, WM_VSCROLL, SB_PAGEDOWN, 0);
   int page = ListBox_GetTopIndex(list) - 11;
   CHECK(page >= 5 && page <= 8);
   SendMessageA(list, WM_VSCROLL, SB_BOTTOM, 0);
   CHECK_INT(ListBox_GetTopIndex(list), 34 - (int) si.nPage);
   // The wheel.
   SendMessageA(list, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), 0);
   CHECK(ListBox_GetTopIndex(list) < 34 - (int) si.nPage);
   // Dragging the scroll bar's thumb: WM_NCLBUTTONDOWN on the bar tracks it until
   // the button goes up.
   ListBox_SetTopIndex(list, 0);
   {
      RECT wr;
      GetWindowRect(list, &wr);
      int bx = wr.right - 2 - GetSystemMetrics(SM_CXVSCROLL) / 2;
      LRESULT hit = SendMessageA(list, WM_NCHITTEST, 0, MAKELPARAM(bx, wr.top + 40));
      CHECK_INT(hit, HTVSCROLL);
      CHECK_INT(SendMessageA(list, WM_NCHITTEST, 0, MAKELPARAM(wr.left + 20, wr.top + 20)), HTCLIENT);
      // A press on the lower page area: one page down, then the release ends it.
      PostMessageA(list, WM_LBUTTONUP, 0, 0);
      SendMessageA(list, WM_NCLBUTTONDOWN, HTVSCROLL, MAKELPARAM(bx, wr.bottom - 25));
      CHECK(ListBox_GetTopIndex(list) >= 5);
      CHECK(GetCapture() == NULL);
   }
   ListBox_ResetContent(list);
   CHECK_INT(ListBox_GetCount(list), 0);

   // Multiple selection.
   HWND multi = CreateWindowExA(0, "ListBox", "", WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_EXTENDEDSEL, 170, 10, 100,
                                100, parent, (HMENU) 302, instance, NULL);
   for (int i = 0; i < 5; i++)
      ListBox_AddString(multi, things[i % 3].name);
   ListBox_SetSel(multi, TRUE, 1);
   ListBox_SetSel(multi, TRUE, 3);
   CHECK(ListBox_GetSel(multi, 1) > 0);
   CHECK_INT(ListBox_GetSel(multi, 2), 0);
   CHECK_INT(ListBox_GetSelCount(multi), 2);
   int sel[8];
   CHECK_INT(ListBox_GetSelItems(multi, 8, sel), 2);
   CHECK(sel[0] == 1 && sel[1] == 3);
   ListBox_SetSel(multi, FALSE, -1);
   CHECK_INT(ListBox_GetSelCount(multi), 0);

   // LBS_MULTIPLESEL toggles by space and click; LB_GETCURSEL is the item it
   // happened on, the focus item, by key and by mouse alike (the client's drop
   // and offer dialogs ask so on LBN_SELCHANGE).
   HWND toggle = CreateWindowExA(0, "ListBox", "", WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_MULTIPLESEL, 280, 10,
                                 100, 100, parent, (HMENU) 306, instance, NULL);
   for (int i = 0; i < 3; i++)
      ListBox_AddString(toggle, things[i].name);
   SetFocus(toggle);
   Key(toggle, VK_DOWN);
   memset(&rcv, 0, sizeof(rcv));
   Key(toggle, VK_SPACE);
   CHECK(ListBox_GetSel(toggle, 1) > 0);
   CHECK_INT(rcv.code_count, 1);
   CHECK_INT(rcv.last_code, LBN_SELCHANGE);
   CHECK_INT(ListBox_GetCurSel(toggle), 1);
   int item_height = ListBox_GetItemHeight(toggle, 0);
   Click(toggle, 10, 2 * item_height + item_height / 2);
   CHECK(ListBox_GetSel(toggle, 2) > 0);
   CHECK_INT(ListBox_GetCurSel(toggle), 2);
   Click(toggle, 10, item_height / 2);
   CHECK_INT(ListBox_GetCurSel(toggle), 0);
   DestroyWindow(toggle);

   // Owner-drawn, fixed, without strings: the item data is the item (a pointer).
   memset(&rcv, 0, sizeof(rcv));
   HWND od = CreateWindowExA(0, "ListBox", "",
                             WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 10,
                             120, 150, 100, parent, (HMENU) 303, instance, NULL);
   CHECK_INT(rcv.measures, 1);
   CHECK_INT(rcv.last_measure.CtlType, ODT_LISTBOX);
   CHECK_INT(rcv.last_measure.CtlID, 303);
   CHECK_INT(ListBox_GetItemHeight(od, 0), 20);   // what WM_MEASUREITEM said
   for (int i = 0; i < 3; i++)
      CHECK_INT(ListBox_AddItemData(od, &things[i]), i);
   CHECK(ListBox_GetItemData(od, 2) == (LRESULT) &things[2]);
   ListBox_SetCurSel(od, 1);
   memset(&rcv, 0, sizeof(rcv));
   struct Image im = Render(od);
   CHECK(rcv.draws >= 3);
   CHECK_INT(rcv.last_draw.CtlType, ODT_LISTBOX);
   CHECK_INT(rcv.last_draw.CtlID, 303);
   CHECK(rcv.last_draw.hwndItem == od);
   CHECK(rcv.last_draw.itemData == (ULONG_PTR) &things[rcv.last_draw.itemID]);
   CHECK_INT(rcv.last_draw.rcItem.bottom - rcv.last_draw.rcItem.top, 20);
   // Item 1 selected (red), 0 and 2 not (blue).
   CHECK(Pixel(&im, 20, 10) == RGB(0, 0, 255));
   CHECK(Pixel(&im, 20, 30) == RGB(255, 0, 0));
   CHECK(Pixel(&im, 20, 50) == RGB(0, 0, 255));
   FreeImage(&im);
   // LB_GETTEXT without LBS_HASSTRINGS gives the item data.
   ULONG_PTR data = 0;
   CHECK_INT(SendMessageA(od, LB_GETTEXT, 2, (LPARAM) &data), sizeof(ULONG_PTR));
   CHECK(data == (ULONG_PTR) &things[2]);
   // A click selects with ODA_SELECT drawing.
   memset(&rcv, 0, sizeof(rcv));
   Click(od, 10, 50);
   CHECK_INT(ListBox_GetCurSel(od), 2);
   CHECK_INT(rcv.last_code, LBN_SELCHANGE);

   // Owner-drawn, variable, with strings: WM_MEASUREITEM per item, with its data.
   memset(&rcv, 0, sizeof(rcv));
   HWND var = CreateWindowExA(0, "ListBox", "",
                              WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT, 170,
                              120, 150, 100, parent, (HMENU) 304, instance, NULL);
   int before = rcv.measures;
   ListBox_AddString(var, "first \xE9");
   CHECK_INT(rcv.measures, before + 1);
   CHECK_INT(rcv.last_measure.CtlID, 304);
   CHECK_INT(rcv.last_measure.itemID, 0);
   ListBox_SetItemData(var, 0, (LPARAM) 77);
   ListBox_AddString(var, "second");
   CHECK_INT(rcv.last_measure.itemID, 1);
   ListBox_GetText(var, 0, text);
   CHECK_STR(text, "first \xE9");
   CHECK_INT(ListBox_GetItemHeight(var, 1), 20);
   SendMessageA(var, LB_SETITEMHEIGHT, 1, 30);
   CHECK_INT(ListBox_GetItemHeight(var, 1), 30);

   // An item replaced with redrawing off, then redrawn (the client's offer
   // amounts): its new text is painted as soon as the queue runs.
   HWND quan = CreateWindowExA(0, "ListBox", "", WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
                               330, 120, 40, 100, parent, (HMENU) 305, instance, NULL);
   ListBox_AddString(quan, " ");
   ListBox_AddString(quan, " ");
   Pump();
   for (int round = 0; round < 2; round++)
   {
      SetWindowRedraw(quan, FALSE);
      ListBox_DeleteString(quan, 1);
      ListBox_InsertString(quan, 1, round ? "1" : "2500");
      SetWindowRedraw(quan, TRUE);
      InvalidateRect(quan, NULL, TRUE);
      memset(&rcv, 0, sizeof(rcv));
      Pump();
      CHECK(rcv.draws >= 2);
      CHECK(rcv.last_draw.hwndItem == quan);
   }
   DestroyWindow(quan);

   // An ANSI subclass sees the keys, the control still works.
   old_list_proc = SubclassWindow(od, ListSubclass);
   SetFocus(od);
   Key(od, VK_UP);
   CHECK_INT(subclass_keys, 1);
   CHECK_INT(ListBox_GetCurSel(od), 1);
   SubclassWindow(od, old_list_proc);

   // Rendering: a string list with a selection, its scroll bar.
   for (int i = 0; i < 12; i++)
   {
      char item[16];
      sprintf(item, "Zeile %d \xFC", i);
      ListBox_AddString(list, item);
   }
   ListBox_SetCurSel(list, 2);
   SetFocus(list);
   memset(&rcv, 0, sizeof(rcv));
   im = Render(list);
   CHECK(rcv.ctlcolor[WM_CTLCOLORLISTBOX - WM_CTLCOLORMSGBOX] >= 1);
   SaveBmp(&im, "ctl_listbox.bmp");
   RECT selected = {4, 2 + 2 * h, 60, 2 + 3 * h};
   CHECK(CountColor(&im, selected, GetSysColor(COLOR_HIGHLIGHT)) > 50);
   RECT normal = {4, 4, 60, 2 + h};
   CHECK(CountDark(&im, normal) > 10);
   CHECK(CountColor(&im, normal, RGB(255, 255, 255)) > 50);
   FreeImage(&im);

   DestroyWindow(parent);
   return Report("ctl_listbox");
}

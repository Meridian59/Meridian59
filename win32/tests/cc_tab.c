/*
 * cc_tab.c:  The tab control (Wine's SysTabControl32) with an ANSI parent:
 *   items (TCITEMA, Latin-1 text), TabCtrl_GetCurSel / SetCurSel, a click and the
 *   arrow keys changing the selection with TCN_SELCHANGING / TCN_SELCHANGE,
 *   vetoing a change, TabCtrl_AdjustRect, hit tests; renders by WM_PRINTCLIENT,
 *   checks the selected tab stands out, writes cc_tab.bmp.
 */

#include "cc_util.h"

static HWND tab;
static int changing, changed;
static BOOL veto;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_NOTIFY && ((NMHDR *) lparam)->hwndFrom == tab)
   {
      switch (((NMHDR *) lparam)->code)
      {
      case TCN_SELCHANGING:
         changing++;
         return veto;
      case TCN_SELCHANGE:
         changed++;
         return 0;
      }
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 400, 200);
   tab = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 10, 10, 300, 150, parent,
                         (HMENU) 4, instance, NULL);
   CHECK(tab != NULL);
   if (tab == NULL)
      return CheckReport("cc_tab");
   SendMessageA(tab, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), FALSE);

   const char *names[] = {"Geral", "Configura\xE7\xF5" "es", "\xC1udio"};   // Configurações, Áudio
   for (int i = 0; i < 3; i++)
   {
      TCITEMA item = {};
      item.mask = TCIF_TEXT | TCIF_PARAM;
      item.pszText = (LPSTR) names[i];
      item.lParam = 10 + i;
      CHECK_INT(TabCtrl_InsertItem(tab, i, &item), i);
   }
   CHECK_INT(TabCtrl_GetItemCount(tab), 3);
   CHECK_INT(TabCtrl_GetCurSel(tab), 0);

   char text[32];
   TCITEMA item = {};
   item.mask = TCIF_TEXT | TCIF_PARAM;
   item.pszText = text;
   item.cchTextMax = sizeof(text);
   CHECK(TabCtrl_GetItem(tab, 1, &item));
   CHECK_STR(text, names[1]);
   CHECK_INT(item.lParam, 11);

   // SetCurSel: no notifications.
   CHECK_INT(TabCtrl_SetCurSel(tab, 2), 0);
   CHECK_INT(TabCtrl_GetCurSel(tab), 2);
   CHECK_INT(changing + changed, 0);

   // A click on tab 1.
   RECT r;
   CHECK(TabCtrl_GetItemRect(tab, 1, &r));
   CHECK(r.right > r.left && r.bottom > r.top);
   TCHITTESTINFO ht = {{(r.left + r.right) / 2, (r.top + r.bottom) / 2}, 0};
   CHECK_INT(TabCtrl_HitTest(tab, &ht), 1);
   SendMessageA(tab, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ht.pt.x, ht.pt.y));
   SendMessageA(tab, WM_LBUTTONUP, 0, MAKELPARAM(ht.pt.x, ht.pt.y));
   CHECK_INT(changing, 1);
   CHECK_INT(changed, 1);
   CHECK_INT(TabCtrl_GetCurSel(tab), 1);

   // Keys: right selects the next; a vetoed change keeps the selection.
   SetFocus(tab);
   SendMessageA(tab, WM_KEYDOWN, VK_RIGHT, 0);
   CHECK_INT(TabCtrl_GetCurSel(tab), 2);
   CHECK_INT(changed, 2);
   // A vetoed change from the keyboard still moves (as Windows' does), without
   // TCN_SELCHANGE.
   veto = TRUE;
   SendMessageA(tab, WM_KEYDOWN, VK_LEFT, 0);
   CHECK_INT(TabCtrl_GetCurSel(tab), 1);
   CHECK_INT(changed, 2);
   veto = FALSE;
   SendMessageA(tab, WM_KEYDOWN, VK_RIGHT, 0);
   CHECK_INT(TabCtrl_GetCurSel(tab), 2);
   CHECK_INT(changed, 3);
   // A vetoed click doesn't.
   veto = TRUE;
   SendMessageA(tab, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ht.pt.x, ht.pt.y));
   SendMessageA(tab, WM_LBUTTONUP, 0, MAKELPARAM(ht.pt.x, ht.pt.y));
   CHECK_INT(TabCtrl_GetCurSel(tab), 2);
   veto = FALSE;
   TabCtrl_SetCurSel(tab, 1);

   // The display area below the tabs.
   RECT display = {0, 0, 300, 150};
   TabCtrl_AdjustRect(tab, FALSE, &display);
   CHECK(display.top >= r.bottom - 2 && display.top < 40);
   CHECK(display.left > 0 && display.right < 300 && display.bottom < 150);
   RECT window = display;
   TabCtrl_AdjustRect(tab, TRUE, &window);
   CHECK_INT(window.left, 0);
   CHECK_INT(window.right, 300);

   // Rendering: the selected tab (1) is taller than the others.
   Canvas32 c = MakeCanvas(300, 150, RGB(255, 0, 255));
   SendMessageA(tab, WM_ERASEBKGND, (WPARAM) c.dc, 0);
   SendMessageA(tab, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   DumpBmp(&c, "cc_tab.bmp");
   RECT r0, r1;
   TabCtrl_GetItemRect(tab, 0, &r0);
   TabCtrl_GetItemRect(tab, 1, &r1);
   // The highlight line along each tab's top: the selected one's is higher.
   int top0 = -1, top1 = -1;
   for (int y = 0; y < 30 && (top0 < 0 || top1 < 0); y++)
   {
      if (top0 < 0 && CanvasPixel(&c, (r0.left + r0.right) / 2, y) == GetSysColor(COLOR_BTNHIGHLIGHT))
         top0 = y;
      if (top1 < 0 && CanvasPixel(&c, (r1.left + r1.right) / 2, y) == GetSysColor(COLOR_BTNHIGHLIGHT))
         top1 = y;
   }
   printf("tab tops: %d (unselected) %d (selected)\n", top0, top1);
   CHECK(top0 >= 0 && top1 >= 0 && top1 < top0);
   CHECK(CountDark(&c, r1, 80) > 20);   // its text
   FreeCanvas(&c);

   CHECK(TabCtrl_DeleteItem(tab, 0));
   CHECK_INT(TabCtrl_GetItemCount(tab), 2);
   DestroyWindow(parent);
   return CheckReport("cc_tab");
}

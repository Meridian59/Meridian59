/*
 * cc_tooltip.c:  Tooltips (Wine's tooltips_class32) as the client's tooltip.c
 *   uses them: created with TTS_ALWAYSTIP and the main window as owner, tools
 *   added with ANSI TTTOOLINFOA (rectangles with string-resource text, windows
 *   with TTF_IDISHWND and LPSTR_TEXTCALLBACK), TTM_RELAYEVENT of mouse moves,
 *   TTN_NEEDTEXTA to the ANSI parent, TTM_SETTOOLINFOA, TTM_UPDATETIPTEXTA,
 *   TTM_POP, TTM_DELTOOLA.  Shows the tip (a popup surface on the owner: the
 *   parent is shown briefly), renders it and writes cc_tooltip.bmp.
 */

#include "cc_util.h"

static HWND tips, parent, button;
static int needtext_a, needtext_w;
static UINT_PTR needtext_id;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_NOTIFY)
   {
      NMHDR *hdr = (NMHDR *) lparam;
      if (hdr->hwndFrom == tips && hdr->code == TTN_NEEDTEXTA)
      {
         needtext_a++;
         NMTTDISPINFOA *di = (NMTTDISPINFOA *) lparam;
         needtext_id = hdr->idFrom;
         strcpy(di->szText, "Grr\xFC\xDF" "e");   // Grrüße
         return 0;
      }
      if (hdr->hwndFrom == tips && hdr->code == TTN_NEEDTEXTW)
         needtext_w++;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void InitTool(TTTOOLINFOA *ti)
{
   memset(ti, 0, sizeof(*ti));
   ti->cbSize = sizeof(*ti);
}

static int ToolCount(void)
{
   return (int) SendMessageA(tips, TTM_GETTOOLCOUNT, 0, 0);
}

static void Relay(HWND hwnd, int x, int y)
{
   // Where the pointer is, as the tooltip checks it (screen coordinates).
   POINT p = {x, y};
   ClientToScreen(hwnd, &p);
   SetCursorPos(p.x, p.y);
   MSG msg = {};
   msg.hwnd = hwnd;
   msg.message = WM_MOUSEMOVE;
   msg.lParam = MAKELPARAM(x, y);
   msg.time = GetTickCount();
   msg.pt = p;
   SendMessageA(tips, TTM_RELAYEVENT, 0, (LPARAM) &msg);
}

// Hovers (x, y) of hwnd until the tip shows, up to 5 seconds.
static BOOL Hover(HWND hwnd, int x, int y)
{
   // The pointer rests: the tip shows after the initial delay.
   Relay(hwnd, x, y);
   for (int i = 0; i < 500; i++)
   {
      Pump(10);
      if (IsWindowVisible(tips))
         return TRUE;
   }
   return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   parent = MakeParent(ParentProc, 320, 200);
   WNDCLASSA wc = {};
   wc.lpfnWndProc = DefWindowProcA;
   wc.hInstance = instance;
   wc.lpszClassName = "CcChild";
   RegisterClassA(&wc);
   button = CreateWindowExA(0, "CcChild", "x", WS_CHILD | WS_VISIBLE, 200, 20, 60, 40, parent, (HMENU) 5, instance, NULL);
   tips = CreateWindowA(TOOLTIPS_CLASSA, NULL, TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, parent, NULL, instance, NULL);
   CHECK(tips != NULL);
   if (tips == NULL)
      return CheckReport("cc_tooltip");
   CHECK(GetWindowLongA(tips, GWL_STYLE) & WS_POPUP);
   CHECK(GetWindowLongA(tips, GWL_EXSTYLE) & WS_EX_TOOLWINDOW);
   CHECK(GetParent(tips) == NULL || GetWindow(tips, GW_OWNER) == parent);

   // A rectangle tool whose text is a string resource: comctl32's own "Close".
   HMODULE comctl = GetModuleHandleA("comctl32.dll");
   CHECK(comctl != NULL);
   CHECK(LoadLibraryA("comctl32.dll") == comctl);
   char close[32] = "";
   CHECK(LoadStringA(comctl, 4160 /* IDS_CLOSE */, close, sizeof(close)) > 0);
   CHECK_STR(close, "Close");
   TTTOOLINFOA ti;
   InitTool(&ti);
   ti.uFlags = 0;
   ti.hwnd = parent;
   ti.uId = 1;
   ti.hinst = comctl;
   SetRect(&ti.rect, 10, 10, 110, 60);
   ti.lpszText = MAKEINTRESOURCEA(4160);
   CHECK(SendMessageA(tips, TTM_ADDTOOLA, 0, (LPARAM) &ti));
   // A window tool with callback text (TooltipAddWindowCallback).
   InitTool(&ti);
   ti.uFlags = TTF_IDISHWND;
   ti.hwnd = button;
   ti.uId = (UINT_PTR) button;
   ti.hinst = instance;
   ti.lpszText = LPSTR_TEXTCALLBACKA;
   CHECK(SendMessageA(tips, TTM_ADDTOOLA, 0, (LPARAM) &ti));
   // A plain text tool.
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 3;
   SetRect(&ti.rect, 10, 100, 110, 150);
   ti.lpszText = (LPSTR) "Caf\xE9";
   CHECK(SendMessageA(tips, TTM_ADDTOOLA, 0, (LPARAM) &ti));
   CHECK_INT(ToolCount(), 3);

   // Texts read back in ANSI.
   char text[80];
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 3;
   ti.lpszText = text;
   SendMessageA(tips, TTM_GETTEXTA, sizeof(text), (LPARAM) &ti);
   CHECK_STR(text, "Caf\xE9");
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 1;
   ti.lpszText = text;
   SendMessageA(tips, TTM_GETTEXTA, sizeof(text), (LPARAM) &ti);
   CHECK_STR(text, "Close");

   // TTM_UPDATETIPTEXTA and TTM_SETTOOLINFOA change a tool.
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 3;
   ti.lpszText = (LPSTR) "M\xFCller";
   SendMessageA(tips, TTM_UPDATETIPTEXTA, 0, (LPARAM) &ti);
   ti.lpszText = text;
   SendMessageA(tips, TTM_GETTEXTA, sizeof(text), (LPARAM) &ti);
   CHECK_STR(text, "M\xFCller");
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 3;
   SetRect(&ti.rect, 10, 100, 150, 180);
   ti.lpszText = (LPSTR) "Zeta";
   SendMessageA(tips, TTM_SETTOOLINFOA, 0, (LPARAM) &ti);
   memset(&ti.rect, 0, sizeof(ti.rect));
   ti.lpszText = text;
   CHECK(SendMessageA(tips, TTM_GETTOOLINFOA, 0, (LPARAM) &ti));
   CHECK_INT(ti.rect.right, 150);
   CHECK_STR(text, "Zeta");

   // Showing: the parent is shown, the pointer relayed over the callback tool:
   // TTN_NEEDTEXTA (not W) reaches the ANSI parent; the tip pops up.
   SetWindowPos(parent, NULL, 0, 0, 320, 200, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
   Pump(200);
   CHECK(Hover(button, 20, 20));
   CHECK(needtext_a > 0);
   CHECK_INT(needtext_w, 0);
   CHECK(needtext_id == (UINT_PTR) button);
   RECT tr;
   GetWindowRect(tips, &tr);
   printf("tip at %ld,%ld %ldx%ld\n", tr.left, tr.top, tr.right - tr.left, tr.bottom - tr.top);
   CHECK(tr.right - tr.left > 20 && tr.bottom - tr.top > 8);
   // It shows near the pointer, under it.
   POINT cursor;
   GetCursorPos(&cursor);
   CHECK(abs(tr.left - cursor.x) < 40);
   CHECK(tr.top >= cursor.y);

   // Its rendering: the info background, dark text.
   RECT cr;
   GetClientRect(tips, &cr);
   Canvas32 c = MakeCanvas(cr.right, cr.bottom, RGB(255, 0, 255));
   SendMessageA(tips, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   CHECK(CountColor(&c, cr, GetSysColor(COLOR_INFOBK), 0) > cr.right * cr.bottom / 2);
   CHECK(CountDark(&c, cr, 100) > 15);
   DumpBmp(&c, "cc_tooltip.bmp");
   FreeCanvas(&c);

   // TTM_POP hides it; moving to the rectangle tool shows its resource text.
   SendMessageA(tips, TTM_POP, 0, 0);
   CHECK(!IsWindowVisible(tips));
   Relay(parent, 300, 190);   // away from the tools
   Pump(50);
   CHECK(Hover(parent, 50, 30));
   InitTool(&ti);
   ti.lpszText = text;
   text[0] = 0;
   CHECK(SendMessageA(tips, TTM_GETCURRENTTOOLA, 0, (LPARAM) &ti));
   CHECK_INT(ti.uId, 1);
   SendMessageA(tips, TTM_POP, 0, 0);

   // TTM_DELTOOLA.
   InitTool(&ti);
   ti.hwnd = parent;
   ti.uId = 3;
   SendMessageA(tips, TTM_DELTOOLA, 0, (LPARAM) &ti);
   CHECK_INT(ToolCount(), 2);

   DestroyWindow(tips);
   DestroyWindow(parent);
   return CheckReport("cc_tooltip");
}

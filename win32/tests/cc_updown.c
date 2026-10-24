/*
 * cc_updown.c:  The up-down control (Wine's msctls_updown32) as the client's
 *   amount dialog uses it (UDM_SETRANGE with min above max, UDS_ARROWKEYS), and
 *   the progress bar (msctls_progress32, the guest dialog's): positions, ranges,
 *   clicks on the arrows and keys with WM_VSCROLL / UDN_DELTAPOS to the parent;
 *   PBM_SETRANGE / SETPOS / DELTAPOS / STEPIT.  Renders both by WM_PRINTCLIENT
 *   and writes cc_updown.bmp.
 */

#include "cc_util.h"

static HWND updown, progress;
static int vscrolls, deltas;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_VSCROLL && (HWND) lparam == updown)
   {
      vscrolls++;
      return 0;
   }
   if (msg == WM_NOTIFY && ((NMHDR *) lparam)->hwndFrom == updown && ((NMHDR *) lparam)->code == UDN_DELTAPOS)
   {
      deltas++;
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int Pos(void)
{
   // Without a buddy the high word reports an error, as on Windows.
   LRESULT r = SendMessageA(updown, UDM_GETPOS, 0, 0);
   return (short) LOWORD(r);
}

static void Click(int x, int y)
{
   SendMessageA(updown, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
   SendMessageA(updown, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
   Pump(20);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 300, 100);
   updown = CreateWindowExA(0, UPDOWN_CLASSA, "", WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_NOTHOUSANDS, 10, 10, 16,
                            24, parent, (HMENU) 6, instance, NULL);
   progress = CreateWindowExA(0, PROGRESS_CLASSA, "", WS_CHILD | WS_VISIBLE | WS_BORDER, 40, 10, 200, 16, parent,
                              (HMENU) 8, instance, NULL);
   CHECK(updown != NULL && progress != NULL);
   if (updown == NULL || progress == NULL)
      return CheckReport("cc_updown");

   // As the amount dialog: SendMessage(hwndSpin, UDM_SETRANGE, 0, MAKELONG(max, min)).
   SendMessageA(updown, UDM_SETRANGE, 0, MAKELONG(50, 1));
   LRESULT range = SendMessageA(updown, UDM_GETRANGE, 0, 0);
   CHECK_INT(LOWORD(range), 50);   // max in the low word
   CHECK_INT(HIWORD(range), 1);
   SendMessageA(updown, UDM_SETPOS, 0, MAKELONG(10, 0));
   CHECK_INT(Pos(), 10);

   // The upper arrow increases.
   Click(8, 4);
   CHECK_INT(Pos(), 11);
   CHECK(vscrolls >= 1);
   CHECK(deltas >= 1);
   Click(8, 20);
   CHECK_INT(Pos(), 10);
   SetFocus(updown);
   SendMessageA(updown, WM_KEYDOWN, VK_UP, 0);
   SendMessageA(updown, WM_KEYUP, VK_UP, 0);
   CHECK_INT(Pos(), 11);
   // Clamped at the range's ends.
   SendMessageA(updown, UDM_SETPOS, 0, MAKELONG(50, 0));
   Click(8, 4);
   CHECK_INT(Pos(), 50);

   // The progress bar.
   SendMessageA(progress, PBM_SETRANGE, 0, MAKELPARAM(0, 200));
   CHECK_INT(SendMessageA(progress, PBM_SETPOS, 50, 0), 0);   // returns the old position
   CHECK_INT(SendMessageA(progress, PBM_GETPOS, 0, 0), 50);
   SendMessageA(progress, PBM_DELTAPOS, 25, 0);
   CHECK_INT(SendMessageA(progress, PBM_GETPOS, 0, 0), 75);
   SendMessageA(progress, PBM_SETSTEP, 25, 0);
   SendMessageA(progress, PBM_STEPIT, 0, 0);
   CHECK_INT(SendMessageA(progress, PBM_GETPOS, 0, 0), 100);   // half

   // Rendering: the arrows' faces; half the bar filled with the highlight color.
   Canvas32 c = MakeCanvas(240, 30, RGB(255, 0, 255));
   SendMessageA(updown, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   RECT ur = {0, 0, 16, 24};
   CHECK(CountColor(&c, ur, GetSysColor(COLOR_BTNFACE), 0) > 100);
   CHECK(CountColor(&c, ur, GetSysColor(COLOR_BTNTEXT), 0) >= 6);   // the arrow glyphs
   POINT old;
   SetViewportOrgEx(c.dc, 30, 0, &old);
   SendMessageA(progress, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   SetViewportOrgEx(c.dc, old.x, old.y, NULL);
   DumpBmp(&c, "cc_updown.bmp");
   RECT pc;
   GetClientRect(progress, &pc);
   RECT first = {30, 0, 30 + pc.right / 2 - 4, pc.bottom}, second = {30 + pc.right / 2 + 4, 0, 30 + pc.right, pc.bottom};
   int lit1 = CountColor(&c, first, GetSysColor(COLOR_HIGHLIGHT), 0);
   int lit2 = CountColor(&c, second, GetSysColor(COLOR_HIGHLIGHT), 0);
   printf("progress: %d lit in the first half, %d in the second\n", lit1, lit2);
   CHECK(lit1 > (first.right - first.left) * pc.bottom / 3);
   CHECK_INT(lit2, 0);
   FreeCanvas(&c);
   DestroyWindow(parent);
   return CheckReport("cc_updown");
}

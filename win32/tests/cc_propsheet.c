/*
 * cc_propsheet.c:  Property sheets (Wine's propsheet.c on the layer's dialog
 *   manager) as the client's preferences and character maker use them:
 *   PropertySheetA with PROPSHEETPAGEA pages from dialog resources (ANSI page
 *   procedures, Latin-1 titles), PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW |
 *   PSH_USEICONID | PSH_USECALLBACK with PSCB_INITIALIZED, PSN_SETACTIVE /
 *   KILLACTIVE / APPLY / RESET to the pages, PropSheet_GetTabControl,
 *   IndexToHwnd, SetCurSel, RemovePage, PressButton (OK and Cancel).  The sheet is
 *   modal and shown while the test runs it from a timer; a rendering of it is
 *   written to cc_propsheet.bmp.  Button and Static are stand-ins here (the real
 *   ones live elsewhere).
 */

#include "cc_util.h"

static HWND sheet;
static HWND pages[3];
static LPARAM page_params[3];
static int setactive[3], killactive[3], apply[3], reset[3];
static BOOL script_ran, cancel_run;

/************************************************************************/
// Stand-in controls

static LRESULT CALLBACK StandInProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_GETDLGCODE:
      return DLGC_BUTTON;
   case BM_SETCHECK:
      SetWindowLongPtrA(hwnd, GWLP_USERDATA, wparam);
      return 0;
   case BM_GETCHECK:
      return GetWindowLongPtrA(hwnd, GWLP_USERDATA);
   case BM_SETSTYLE:
      SetWindowLongA(hwnd, GWL_STYLE, (GetWindowLongA(hwnd, GWL_STYLE) & ~0xFFFF) | (WORD) wparam);
      return 0;
   case BM_CLICK:
      SendMessageA(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM) hwnd);
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void RegisterStandIns(void)
{
   WNDCLASSA wc = {};
   wc.lpfnWndProc = StandInProc;
   wc.hInstance = GetModuleHandleA(NULL);
   wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
   wc.lpszClassName = "Button";
   RegisterClassA(&wc);
   wc.lpszClassName = "Static";
   RegisterClassA(&wc);
}

/************************************************************************/

static int PageIndex(HWND hwnd)
{
   for (int i = 0; i < 3; i++)
      if (pages[i] == hwnd)
         return i;
   return -1;
}

static INT_PTR CALLBACK PageProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_INITDIALOG:
   {
      PROPSHEETPAGEA *psp = (PROPSHEETPAGEA *) lparam;
      int i = (int) psp->lParam;
      CHECK(i >= 0 && i < 3);
      if (i < 0 || i >= 3)
         return TRUE;
      pages[i] = hwnd;
      page_params[i] = psp->lParam;
      CHECK(!IsWindowUnicode(hwnd));
      if (HWND bar = GetDlgItem(hwnd, 1001))
      {
         SendMessageA(bar, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
         SendMessageA(bar, TBM_SETPOS, TRUE, 40);
      }
      if (HWND bar = GetDlgItem(hwnd, 1004))
         SendMessageA(bar, PBM_SETPOS, 60, 0);
      return TRUE;
   }
   case WM_NOTIFY:
   {
      NMHDR *hdr = (NMHDR *) lparam;
      int i = PageIndex(hwnd);
      if (i < 0)
         break;
      if ((int) hdr->code <= (int) PSN_FIRST && (int) hdr->code >= (int) PSN_LAST)
         CHECK(hdr->hwndFrom == GetParent(hwnd));   // the sheet
      switch (hdr->code)
      {
      case PSN_SETACTIVE: setactive[i]++; break;
      case PSN_KILLACTIVE: killactive[i]++; break;
      case PSN_APPLY:
         apply[i]++;
         SetWindowLongPtrA(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
         return TRUE;
      case PSN_RESET: reset[i]++; break;
      }
      SetWindowLongPtrA(hwnd, DWLP_MSGRESULT, 0);
      return TRUE;
   }
   }
   return FALSE;
}

static void Capture(void)
{
   RedrawWindow(sheet, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
   RECT cr;
   GetClientRect(sheet, &cr);
   Canvas32 c = MakeCanvas(cr.right, cr.bottom, RGB(255, 0, 255));
   HDC dc = GetDC(sheet);
   BitBlt(c.dc, 0, 0, cr.right, cr.bottom, dc, 0, 0, SRCCOPY);
   ReleaseDC(sheet, dc);
   DumpBmp(&c, "cc_propsheet.bmp");
   // The dialog background, the tab control's highlight edges, the trackbar's
   // channel on page 0 and the progress bar's lit part are drawn.
   CHECK(CountColor(&c, cr, GetSysColor(COLOR_BTNFACE), 0) > cr.right * cr.bottom / 2);
   CHECK(CountColor(&c, cr, GetSysColor(COLOR_BTNHIGHLIGHT), 0) > 200);
   CHECK_INT(CountColor(&c, cr, RGB(255, 0, 255), 0), 0);
   FreeCanvas(&c);
}

static void CALLBACK Script(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
   KillTimer(hwnd, id);
   script_ran = TRUE;
   if (cancel_run)
   {
      // As the character maker's "cancel": PropSheet_PressButton(PSBTN_CANCEL).
      PropSheet_SetCurSel(sheet, 0, 1);
      PropSheet_PressButton(sheet, PSBTN_CANCEL);
      return;
   }

   char text[64];
   GetWindowTextA(sheet, text, sizeof(text));
   CHECK_STR(text, "Prefer\xEAncias");   // Preferências
   CHECK(!(GetWindowLongA(sheet, GWL_EXSTYLE) & WS_EX_CONTEXTHELP));

   HWND tab = PropSheet_GetTabControl(sheet);
   CHECK(tab != NULL);
   CHECK_INT(TabCtrl_GetItemCount(tab), 3);
   CHECK_INT(TabCtrl_GetCurSel(tab), 0);
   TCITEMA item = {};
   item.mask = TCIF_TEXT;
   item.pszText = text;
   item.cchTextMax = sizeof(text);
   TabCtrl_GetItem(tab, 1, &item);
   CHECK_STR(text, "Configura\xE7\xF5" "es");

   CHECK(pages[0] != NULL);
   CHECK(PropSheet_IndexToHwnd(sheet, 0) == pages[0]);
   CHECK(IsWindowVisible(pages[0]));
   CHECK_INT(setactive[0], 1);
   CHECK(pages[1] == NULL);   // made when first shown

   // Page 0's trackbar works inside the sheet.
   HWND bar = GetDlgItem(pages[0], 1001);
   CHECK(bar != NULL);
   CHECK_INT(SendMessageA(bar, TBM_GETPOS, 0, 0), 40);
   Capture();

   // PropSheet_SetCurSel: page 0 is left, page 1 made and entered.
   CHECK(PropSheet_SetCurSel(sheet, 0, 1));
   CHECK_INT(killactive[0], 1);
   CHECK(pages[1] != NULL);
   CHECK_INT(setactive[1], 1);
   CHECK_INT(TabCtrl_GetCurSel(tab), 1);
   CHECK(PropSheet_IndexToHwnd(sheet, 1) == pages[1]);
   CHECK(IsWindowVisible(pages[1]) && !IsWindowVisible(pages[0]));

   // A click on the third tab.
   RECT r;
   TabCtrl_GetItemRect(tab, 2, &r);
   SendMessageA(tab, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2));
   SendMessageA(tab, WM_LBUTTONUP, 0, MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2));
   CHECK_INT(TabCtrl_GetCurSel(tab), 2);
   CHECK_INT(setactive[2], 1);
   CHECK_INT(killactive[1], 1);

   // PropSheet_RemovePage: the third goes; the selection moves to a remaining one.
   PropSheet_RemovePage(sheet, 2, NULL);
   CHECK_INT(TabCtrl_GetItemCount(tab), 2);
   CHECK(TabCtrl_GetCurSel(tab) == 0 || TabCtrl_GetCurSel(tab) == 1);

   // OK: PSN_APPLY to the pages made, then the sheet ends.
   PropSheet_PressButton(sheet, PSBTN_OK);
}

static int CALLBACK SheetCallback(HWND hwnd, UINT msg, LPARAM)
{
   if (msg == PSCB_INITIALIZED && hwnd)
   {
      // As the preferences: no help button; then run the test from a timer.
      sheet = hwnd;
      LONG ex = GetWindowLongA(hwnd, GWL_EXSTYLE);
      SetWindowLongA(hwnd, GWL_EXSTYLE, ex & ~WS_EX_CONTEXTHELP);
      SetTimer(hwnd, 77, 200, Script);
   }
   return 0;
}

static INT_PTR RunSheet(HWND owner)
{
   memset(pages, 0, sizeof(pages));
   memset(setactive, 0, sizeof(setactive));
   memset(killactive, 0, sizeof(killactive));
   memset(apply, 0, sizeof(apply));
   memset(reset, 0, sizeof(reset));
   script_ran = FALSE;
   const char *titles[3] = {"Geral", "Configura\xE7\xF5" "es", "\xC1udio"};
   PROPSHEETPAGEA psp[3] = {};
   for (int i = 0; i < 3; i++)
   {
      psp[i].dwSize = sizeof(PROPSHEETPAGEA);
      psp[i].dwFlags = PSP_USETITLE;
      psp[i].hInstance = GetModuleHandleA(NULL);
      psp[i].pszTemplate = MAKEINTRESOURCEA(101 + i);
      psp[i].pfnDlgProc = PageProc;
      psp[i].pszTitle = titles[i];
      psp[i].lParam = i;
   }
   PROPSHEETHEADERA psh = {};
   psh.dwSize = sizeof(PROPSHEETHEADERA);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USEICONID | PSH_USECALLBACK;
   psh.pfnCallback = SheetCallback;
   psh.hwndParent = owner;
   psh.hInstance = GetModuleHandleA(NULL);
   psh.pszCaption = "Prefer\xEAncias";
   psh.nPages = 3;
   psh.nStartPage = 0;
   psh.ppsp = psp;
   psh.pszIcon = MAKEINTRESOURCEA(1);
   return PropertySheetA(&psh);
}

static LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) instance, (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   RegisterStandIns();
   HWND owner = MakeParent(OwnerProc, 100, 100);

   INT_PTR result = RunSheet(owner);
   CHECK(script_ran);
   CHECK(result > 0);   // OK
   CHECK_INT(apply[0], 1);
   CHECK_INT(apply[1], 1);
   CHECK_INT(apply[2], 0);   // removed
   CHECK(!IsWindow(sheet));
   CHECK(IsWindowEnabled(owner));   // the modal sheet re-enabled its owner

   cancel_run = TRUE;
   result = RunSheet(owner);
   CHECK(script_ran);
   CHECK_INT(result, 0);   // cancelled
   CHECK_INT(apply[0] + apply[1], 0);
   CHECK(reset[0] + reset[1] > 0);
   CHECK(!IsWindow(sheet));

   DestroyWindow(owner);
   return CheckReport("cc_propsheet");
}

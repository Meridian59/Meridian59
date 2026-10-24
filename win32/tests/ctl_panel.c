/*
 * ctl_panel.c:  A panel with one of each control (checked check box, radio
 *   buttons, push and default buttons, an edit with a selection, a list box with
 *   a selection and its scroll bar, a drop-down list, static text, an etched
 *   line, a scroll bar control, a button over a group box's edge), drawn two ways: WM_PRINT into a DIB
 *   (ctl_panel.bmp) and the windows' own painting on the top-level surface, read
 *   back with BitBlt (ctl_panel_painted.bmp).  Both must agree, and a sibling
 *   lower in the Z order paints over a higher one, as in the client's login
 *   dialog.
 */

#include "tests/ctl_common.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(360, 250);
   HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
#define MAKE(ex, cls, text, style, x, y, w, h, id) \
   CreateWindowExA(ex, cls, text, WS_CHILD | WS_VISIBLE | (style), x, y, w, h, parent, (HMENU) (id), instance, NULL)
   HWND controls[16];
   int n = 0;
   controls[n++] = MAKE(0, "Button", "Ch\xE9" "ck box", BS_AUTOCHECKBOX, 10, 10, 110, 18, 1);
   controls[n++] = MAKE(0, "Button", "Radio \xFC" "ne", BS_AUTORADIOBUTTON | WS_GROUP, 10, 30, 110, 18, 2);
   controls[n++] = MAKE(0, "Button", "Radio two", BS_AUTORADIOBUTTON, 10, 50, 110, 18, 3);
   controls[n++] = MAKE(0, "Button", "Push", BS_PUSHBUTTON, 10, 75, 70, 24, 4);
   controls[n++] = MAKE(0, "Button", "Default", BS_DEFPUSHBUTTON, 90, 75, 70, 24, 5);
   controls[n++] = MAKE(WS_EX_CLIENTEDGE, "Edit", "Stra\xDF" "e und Weg", ES_AUTOHSCROLL, 10, 108, 150, 22, 6);
   controls[n++] = MAKE(0, "Static", "Static text: \xC4\xD6\xDC", SS_LEFT, 10, 138, 150, 16, 7);
   controls[n++] = MAKE(0, "Static", "", SS_ETCHEDHORZ, 10, 158, 150, 2, 8);
   controls[n++] = MAKE(0, "ComboBox", "", CBS_DROPDOWNLIST | WS_VSCROLL, 10, 166, 150, 120, 9);
   controls[n++] = MAKE(WS_EX_CLIENTEDGE, "ListBox", "", LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 175, 10, 140,
                        120, 10);
   controls[n++] = MAKE(0, "ScrollBar", "", SBS_VERT, 325, 10, 16, 120, 11);
   controls[n++] = MAKE(0, "Button", "Group", BS_GROUPBOX, 175, 140, 165, 60, 12);
   controls[n++] = MAKE(0, "Button", "Disabled", BS_PUSHBUTTON | WS_DISABLED, 185, 160, 80, 24, 13);
   controls[n++] = MAKE(0, "Button", "Over", BS_PUSHBUTTON, 270, 190, 65, 24, 14);
   for (int i = 0; i < n; i++)
   {
      CHECK(controls[i] != NULL);
      SendMessageA(controls[i], WM_SETFONT, (WPARAM) font, FALSE);
   }
   Button_SetCheck(controls[0], BST_CHECKED);
   Button_SetCheck(controls[2], BST_CHECKED);
   HWND list = controls[9], combo = controls[8], edit = controls[5];
   for (int i = 0; i < 12; i++)
   {
      char item[32];
      sprintf(item, "List item %d \xE9", i);
      ListBox_AddString(list, item);
      ComboBox_AddString(combo, item);
   }
   ListBox_SetCurSel(list, 1);
   ComboBox_SetCurSel(combo, 3);
   SCROLLINFO si = {sizeof(si), SIF_ALL, 0, 100, 20, 30};
   SetScrollInfo(controls[10], SB_CTL, &si, TRUE);
   SetFocus(edit);
   Edit_SetSel(edit, 0, 6);

   // WM_PRINT of the whole panel.
   struct Image printed = Render(parent);
   SaveBmp(&printed, "ctl_panel.bmp");
   // What painting put on the (never shown) surface.
   InvalidateRect(parent, NULL, TRUE);
   UpdateWindow(parent);
   Pump();
   struct Image painted = printed;
   {
      BITMAPINFO bi = {0};
      bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
      bi.bmiHeader.biWidth = printed.width;
      bi.bmiHeader.biHeight = -printed.height;
      bi.bmiHeader.biPlanes = 1;
      bi.bmiHeader.biBitCount = 32;
      painted.dc = CreateCompatibleDC(NULL);
      painted.bitmap = CreateDIBSection(painted.dc, &bi, DIB_RGB_COLORS, (void **) &painted.bits, NULL, 0);
      SelectObject(painted.dc, painted.bitmap);
      HDC wdc = GetDC(parent);
      HideCaret(edit);   // not part of the comparison
      BitBlt(painted.dc, 0, 0, printed.width, printed.height, wdc, 0, 0, SRCCOPY);
      ShowCaret(edit);
      ReleaseDC(parent, wdc);
   }
   SaveBmp(&painted, "ctl_panel_painted.bmp");
   int diff = 0;
   for (int i = 0; i < printed.width * printed.height; i++)
      diff += (printed.bits[i] & 0xFFFFFF) != (painted.bits[i] & 0xFFFFFF);
   CHECK_INT(diff, 0);
   if (diff)
      printf("ctl_panel: %d pixels differ between WM_PRINT and painting\n", diff);

   // The group box's bottom edge (y 198-199) doesn't cross the button under it.
   CHECK(Pixel(&painted, 300, 198) == GetSysColor(COLOR_BTNFACE));
   CHECK(Pixel(&painted, 300, 199) == GetSysColor(COLOR_BTNFACE));

   // Everything was drawn: no pixel left in the fill color.
   RECT all = {0, 0, printed.width, printed.height};
   CHECK_INT(CountColor(&printed, all, RGB(255, 0, 255)), 0);
   FreeImage(&printed);
   FreeImage(&painted);

   DestroyWindow(parent);
   return Report("ctl_panel");
}

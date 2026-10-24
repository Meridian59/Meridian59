/*
 * ctl_combo.c:  Wine's ComboBox on the layer: drop-down lists (CBS_DROPDOWNLIST)
 *   and drop-down combos with an edit field (CBS_DROPDOWN), strings with
 *   Latin-1, item data, selection by message and keyboard (CBN_SELCHANGE), the
 *   drop-down list as a popup top-level window under the combo (CBN_DROPDOWN,
 *   CBN_CLOSEUP, choosing with the mouse), ComboBox_* macros, rendering.
 */

#include "tests/ctl_common.h"

static int Has(int code)
{
   for (int i = 0; i < rcv.code_count; i++)
      if (rcv.codes[i] == code)
         return 1;
   return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(400, 300);
   HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
   char text[128];

   HWND combo = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST, 20, 20, 150,
                                200, parent, (HMENU) 401, instance, NULL);
   CHECK(combo != NULL);
   SendMessageA(combo, WM_SETFONT, (WPARAM) font, FALSE);
   CHECK_INT(ComboBox_AddString(combo, "Norden"), 0);
   CHECK_INT(ComboBox_AddString(combo, "S\xFC" "den"), 1);
   CHECK_INT(ComboBox_AddString(combo, "Osten"), 2);
   CHECK_INT(ComboBox_GetCount(combo), 3);
   ComboBox_GetLBText(combo, 1, text);
   CHECK_STR(text, "S\xFC" "den");
   CHECK_INT(ComboBox_GetLBTextLen(combo, 1), 5);
   ComboBox_SetItemData(combo, 2, (LPARAM) 0x1122334455667788ull);
   CHECK(ComboBox_GetItemData(combo, 2) == (LRESULT) 0x1122334455667788ull);

   // The closed combo is one line high; its list is a separate window.
   RECT r;
   GetWindowRect(combo, &r);
   CHECK(r.bottom - r.top < 40);
   COMBOBOXINFO ci = {sizeof(ci)};
   CHECK(GetComboBoxInfo(combo, &ci));
   HWND lbox = ci.hwndList;
   CHECK(lbox != NULL);
   char cls[32];
   GetClassNameA(lbox, cls, sizeof(cls));
   CHECK_STR(cls, "ComboLBox");
   // The list is a top-level window (a child of the desktop), not the combo's child.
   CHECK(!IsChild(combo, lbox));
   CHECK(GetAncestor(lbox, GA_ROOT) == lbox || GetParent(lbox) == GetDesktopWindow());

   // Selection by message: no notification; by keyboard: CBN_SELCHANGE.
   memset(&rcv, 0, sizeof(rcv));
   ComboBox_SetCurSel(combo, 0);
   CHECK_INT(ComboBox_GetCurSel(combo), 0);
   CHECK_INT(rcv.commands, 0);
   GetWindowTextA(combo, text, sizeof(text));
   CHECK_STR(text, "Norden");
   SetFocus(combo);
   memset(&rcv, 0, sizeof(rcv));
   Key(combo, VK_DOWN);
   CHECK_INT(ComboBox_GetCurSel(combo), 1);
   CHECK(Has(CBN_SELCHANGE));
   CHECK_INT(rcv.last_id, 401);
   GetWindowTextA(combo, text, sizeof(text));
   CHECK_STR(text, "S\xFC" "den");
   ComboBox_GetText(combo, text, sizeof(text));
   CHECK_STR(text, "S\xFC" "den");

   // Dropping down: the list shows under the combo, in screen coordinates.
   memset(&rcv, 0, sizeof(rcv));
   ComboBox_ShowDropdown(combo, TRUE);
   CHECK(Has(CBN_DROPDOWN));
   CHECK(SendMessageA(combo, CB_GETDROPPEDSTATE, 0, 0));
   CHECK(GetWindowLongA(lbox, GWL_STYLE) & WS_VISIBLE);
   RECT lr, cr;
   GetWindowRect(lbox, &lr);
   GetWindowRect(combo, &cr);
   CHECK_INT(lr.top, cr.bottom);
   CHECK_INT(lr.left, cr.left);
   CHECK(lr.bottom - lr.top >= 3 * 13);
   CHECK(GetCapture() == lbox);
   // A click on its third item chooses it and closes the list.
   memset(&rcv, 0, sizeof(rcv));
   int h = (int) SendMessageA(combo, CB_GETITEMHEIGHT, 0, 0);
   CHECK(h >= 13 && h <= 20);
   POINT p = {10, 2 * h + h / 2};
   SendMessageA(lbox, WM_MOUSEMOVE, 0, MAKELPARAM(p.x, p.y));
   SendMessageA(lbox, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p.x, p.y));
   SendMessageA(lbox, WM_LBUTTONUP, 0, MAKELPARAM(p.x, p.y));
   Pump();
   CHECK_INT(ComboBox_GetCurSel(combo), 2);
   CHECK(Has(CBN_SELCHANGE));
   CHECK(Has(CBN_CLOSEUP));
   CHECK(!SendMessageA(combo, CB_GETDROPPEDSTATE, 0, 0));
   CHECK(!(GetWindowLongA(lbox, GWL_STYLE) & WS_VISIBLE));
   CHECK(GetCapture() == NULL);
   // A click on the combo drops it down; one outside closes it.
   SendMessageA(combo, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 5));
   SendMessageA(combo, WM_LBUTTONUP, 0, MAKELPARAM(10, 5));
   CHECK(SendMessageA(combo, CB_GETDROPPEDSTATE, 0, 0));
   SendMessageA(lbox, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(-50, -50));
   Pump();
   CHECK(!SendMessageA(combo, CB_GETDROPPEDSTATE, 0, 0));
   ComboBox_ShowDropdown(combo, FALSE);

   // Find, delete, reset.
   CHECK_INT(ComboBox_FindString(combo, -1, "Os"), 2);
   CHECK_INT(ComboBox_FindStringExact(combo, -1, "s\xFC" "den"), 1);
   CHECK_INT(ComboBox_DeleteString(combo, 0), 2);
   CHECK_INT(ComboBox_InsertString(combo, 0, "Westen"), 0);
   CHECK_INT(ComboBox_GetCount(combo), 3);

   // A drop-down combo with an edit field (textin.c's): text, limit, edit selection.
   HWND dd = CreateWindowExA(0, "ComboBox", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, 20,
                             60, 150, 200, parent, (HMENU) 402, instance, NULL);
   SendMessageA(dd, WM_SETFONT, (WPARAM) font, FALSE);
   ComboBox_AddString(dd, "say hello");
   ComboBox_AddString(dd, "who");
   ComboBox_SetText(dd, "tell \xE9mile");
   GetWindowTextA(dd, text, sizeof(text));
   CHECK_STR(text, "tell \xE9mile");
   CHECK_INT(ComboBox_GetTextLength(dd), 10);
   ComboBox_LimitText(dd, 12);
   ComboBox_SetEditSel(dd, 0, 4);
   DWORD sel = (DWORD) SendMessageA(dd, CB_GETEDITSEL, 0, 0);
   CHECK_INT(LOWORD(sel), 0);
   CHECK_INT(HIWORD(sel), 4);
   COMBOBOXINFO di = {sizeof(di)};
   GetComboBoxInfo(dd, &di);
   CHECK(di.hwndItem != NULL);
   GetClassNameA(di.hwndItem, cls, sizeof(cls));
   CHECK_STR(cls, "Edit");
   CHECK(GetWindow(dd, GW_CHILD) == di.hwndItem);   // textin.c finds the edit field so
   // Typing into its edit: CBN_EDITUPDATE, CBN_EDITCHANGE.
   memset(&rcv, 0, sizeof(rcv));
   SendMessageA(di.hwndItem, EM_SETSEL, 10, 10);
   Type(di.hwndItem, "!!!!");
   GetWindowTextA(dd, text, sizeof(text));
   CHECK_STR(text, "tell \xE9mile!!");   // limited to 12
   CHECK(Has(CBN_EDITCHANGE));
   CHECK(Has(CBN_EDITUPDATE));
   // Selecting a list item puts its text in the edit field.
   ComboBox_SetCurSel(dd, 1);
   GetWindowTextA(dd, text, sizeof(text));
   CHECK_STR(text, "who");
   ComboBox_ResetContent(dd);
   CHECK_INT(ComboBox_GetCount(dd), 0);

   // Rendering: the drop-down list combo shows its text and the arrow button.
   ComboBox_SetCurSel(combo, 1);
   memset(&rcv, 0, sizeof(rcv));
   struct Image im = Render(combo);
   SaveBmp(&im, "ctl_combo.bmp");
   RECT txt = {4, 4, 60, im.height - 4};
   CHECK(CountColor(&im, txt, GetSysColor(COLOR_HIGHLIGHT)) > 50);   // focused: the text highlighted
   CHECK(CountLight(&im, txt) > 10);   // its text, light on the highlight
   int bx = im.width - 2 - GetSystemMetrics(SM_CXVSCROLL) / 2;
   RECT arrow = {bx - 4, im.height / 2 - 3, bx + 4, im.height / 2 + 3};
   CHECK(CountDark(&im, arrow) >= 4);
   FreeImage(&im);

   DestroyWindow(parent);
   CHECK(!IsWindow(lbox));
   return Report("ctl_combo");
}

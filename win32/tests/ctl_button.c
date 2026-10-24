/*
 * ctl_button.c:  Wine's Button on the layer: class lookup in any case, push and
 *   default buttons, auto check boxes and radio groups by mouse and keyboard,
 *   BN_CLICKED to an ANSI parent, Button_SetState/GetState, owner draw, BM_SETIMAGE,
 *   Latin-1 text round trips, an ANSI subclass over the Unicode control, and the
 *   classic rendering (pixel checks).
 */

#include "tests/ctl_common.h"

static WNDPROC old_button_proc;
static int subclass_clicks;
static char subclass_text[64];

static LRESULT CALLBACK ButtonSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_LBUTTONUP)
      subclass_clicks++;
   if (msg == WM_SETTEXT && lparam)
      lstrcpynA(subclass_text, (const char *) lparam, sizeof(subclass_text));
   return CallWindowProcA(old_button_proc, hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(300, 200);
   CHECK(parent != NULL);

   // The class, in each spelling the client uses.
   const char *names[] = {"Button", "button", "BUTTON"};
   for (int i = 0; i < 3; i++)
   {
      HWND b = CreateWindowExA(0, names[i], "x", WS_CHILD, 0, 0, 10, 10, parent, (HMENU) 1, instance, NULL);
      CHECK(b != NULL);
      char cls[32];
      GetClassNameA(b, cls, sizeof(cls));
      CHECK(lstrcmpiA(cls, "Button") == 0);
      CHECK(IsWindowUnicode(b));
      DestroyWindow(b);
   }

   // Push button: BN_CLICKED on a click inside, not when released outside.
   HWND push = CreateWindowExA(0, "Button", "Push", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 80, 24, parent,
                               (HMENU) 101, instance, NULL);
   memset(&rcv, 0, sizeof(rcv));
   Click(push, 5, 5);
   CHECK_INT(rcv.commands, 1);
   CHECK_INT(rcv.last_id, 101);
   CHECK_INT(rcv.last_code, BN_CLICKED);
   CHECK(rcv.last_control == push);
   memset(&rcv, 0, sizeof(rcv));
   SendMessageA(push, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(5, 5));
   CHECK(Button_GetState(push) & BST_PUSHED);
   SendMessageA(push, WM_LBUTTONUP, 0, MAKELPARAM(200, 5));
   CHECK_INT(rcv.commands, 0);
   CHECK(!(Button_GetState(push) & BST_PUSHED));
   Button_SetState(push, TRUE);
   CHECK(Button_GetState(push) & BST_PUSHED);
   Button_SetState(push, FALSE);
   CHECK(!(Button_GetState(push) & BST_PUSHED));
   // Space clicks it too; BM_CLICK as well.
   memset(&rcv, 0, sizeof(rcv));
   SendMessageA(push, WM_KEYDOWN, VK_SPACE, 0);
   SendMessageA(push, WM_KEYUP, VK_SPACE, (LPARAM) 0xC0000001);
   CHECK_INT(rcv.commands, 1);
   SendMessageA(push, BM_CLICK, 0, 0);
   CHECK_INT(rcv.commands, 2);
   CHECK_INT(SendMessageA(push, WM_GETDLGCODE, 0, 0) & (DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON),
             DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON);

   // Default push button.
   HWND def = CreateWindowExA(0, "Button", "Default", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 100, 10, 80, 24,
                              parent, (HMENU) 102, instance, NULL);
   CHECK(SendMessageA(def, WM_GETDLGCODE, 0, 0) & DLGC_DEFPUSHBUTTON);

   // Auto check box.
   HWND check = CreateWindowExA(0, "Button", "Check", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 40, 100, 20,
                                parent, (HMENU) 103, instance, NULL);
   CHECK_INT(Button_GetCheck(check), BST_UNCHECKED);
   memset(&rcv, 0, sizeof(rcv));
   Click(check, 5, 5);
   CHECK_INT(Button_GetCheck(check), BST_CHECKED);
   CHECK_INT(rcv.last_code, BN_CLICKED);
   Click(check, 5, 5);
   CHECK_INT(Button_GetCheck(check), BST_UNCHECKED);
   Button_SetCheck(check, BST_CHECKED);
   CHECK_INT(SendMessageA(check, BM_GETCHECK, 0, 0), BST_CHECKED);

   // Auto radio buttons: a group; clicking one unchecks the others.
   HWND r1 = CreateWindowExA(0, "Button", "One", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 10, 70, 80,
                             20, parent, (HMENU) 104, instance, NULL);
   HWND r2 = CreateWindowExA(0, "Button", "Two", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 90, 80, 20, parent,
                             (HMENU) 105, instance, NULL);
   HWND r3 = CreateWindowExA(0, "Button", "Three", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 110, 80, 20,
                             parent, (HMENU) 106, instance, NULL);
   HWND after = CreateWindowExA(0, "Button", "Other", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 10,
                                130, 80, 20, parent, (HMENU) 107, instance, NULL);
   Button_SetCheck(after, BST_CHECKED);
   Click(r1, 5, 5);
   CHECK_INT(Button_GetCheck(r1), BST_CHECKED);
   Click(r2, 5, 5);
   CHECK_INT(Button_GetCheck(r1), BST_UNCHECKED);
   CHECK_INT(Button_GetCheck(r2), BST_CHECKED);
   CHECK_INT(Button_GetCheck(r3), BST_UNCHECKED);
   CHECK_INT(Button_GetCheck(after), BST_CHECKED);   // another group

   // Text with Latin-1 characters through the ANSI functions.
   SetWindowTextA(check, "Gr\xFC\xDF \xE9t\xE9");
   char text[64];
   GetWindowTextA(check, text, sizeof(text));
   CHECK_STR(text, "Gr\xFC\xDF \xE9t\xE9");
   CHECK_INT(GetWindowTextLengthA(check), 8);
   WCHAR wtext[64];
   GetWindowTextW(check, wtext, 64);
   CHECK(wtext[2] == 0xFC && wtext[3] == 0xDF && wtext[5] == 0xE9);

   // An ANSI subclass over the Unicode control (SubclassWindow).
   old_button_proc = SubclassWindow(push, ButtonSubclass);
   CHECK(old_button_proc != NULL);
   memset(&rcv, 0, sizeof(rcv));
   Click(push, 5, 5);
   CHECK_INT(subclass_clicks, 1);
   CHECK_INT(rcv.commands, 1);
   SetWindowTextA(push, "\xC4pfel");
   CHECK_STR(subclass_text, "\xC4pfel");
   GetWindowTextA(push, text, sizeof(text));
   CHECK_STR(text, "\xC4pfel");
   SubclassWindow(push, old_button_proc);

   // Owner draw: WM_DRAWITEM with the button's id, rectangle and state.
   HWND owner = CreateWindowExA(0, "Button", "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 200, 40, 40, 30, parent,
                                (HMENU) 108, instance, NULL);
   memset(&rcv, 0, sizeof(rcv));
   struct Image im = Render(owner);
   CHECK(rcv.draws >= 1);
   CHECK_INT(rcv.last_draw.CtlType, ODT_BUTTON);
   CHECK_INT(rcv.last_draw.CtlID, 108);
   CHECK(rcv.last_draw.hwndItem == owner);
   CHECK_INT(rcv.last_draw.rcItem.right, 40);
   CHECK_INT(rcv.last_draw.rcItem.bottom, 30);
   CHECK(Pixel(&im, 20, 15) == RGB(0, 0, 255));
   FreeImage(&im);
   memset(&rcv, 0, sizeof(rcv));
   Click(owner, 5, 5);
   CHECK_INT(rcv.last_code, BN_CLICKED);
   SendMessageA(owner, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(5, 5));
   im = Render(owner);
   CHECK(rcv.last_draw.itemState & ODS_SELECTED);
   CHECK(Pixel(&im, 20, 15) == RGB(255, 0, 0));
   FreeImage(&im);
   SendMessageA(owner, WM_LBUTTONUP, 0, MAKELPARAM(5, 5));

   // BS_BITMAP with BM_SETIMAGE: the bitmap is drawn in the middle.
   HWND bmpbtn = CreateWindowExA(0, "Button", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 200, 80, 40,
                                 30, parent, (HMENU) 109, instance, NULL);
   HDC screen = GetDC(NULL);
   HBITMAP green = CreateCompatibleBitmap(screen, 12, 12);
   ReleaseDC(NULL, screen);
   {
      HDC mem = CreateCompatibleDC(NULL);
      HGDIOBJ old = SelectObject(mem, green);
      RECT r = {0, 0, 12, 12};
      HBRUSH b = CreateSolidBrush(RGB(0, 255, 0));
      FillRect(mem, &r, b);
      DeleteObject(b);
      SelectObject(mem, old);
      DeleteDC(mem);
   }
   CHECK(SendMessageA(bmpbtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) green) == 0);
   CHECK((HBITMAP) SendMessageA(bmpbtn, BM_GETIMAGE, IMAGE_BITMAP, 0) == green);
   im = Render(bmpbtn);
   SaveBmp(&im, "ctl_button_bitmap.bmp");
   CHECK(Pixel(&im, 20, 15) == RGB(0, 255, 0));
   FreeImage(&im);

   // Rendering: a checked check box has a dark check mark in a white box; an
   // unchecked one none.
   Button_SetCheck(check, BST_CHECKED);
   im = Render(check);
   RECT box = {0, 4, 13, 17};
   CHECK(CountColor(&im, box, RGB(255, 255, 255)) > 20);
   CHECK(CountDark(&im, box) > 20);
   int dark_checked = CountDark(&im, box);
   FreeImage(&im);
   Button_SetCheck(check, BST_UNCHECKED);
   im = Render(check);
   CHECK(CountDark(&im, box) < dark_checked - 10);
   // The label is drawn, right of the box.
   RECT label = {16, 2, 100, 18};
   CHECK(CountDark(&im, label) > 10);
   FreeImage(&im);

   // A push button: 3D frame (white top-left, dark bottom-right), face color.
   im = Render(push);
   CHECK(Pixel(&im, 1, 1) == GetSysColor(COLOR_BTNHIGHLIGHT) || Pixel(&im, 0, 0) == GetSysColor(COLOR_BTNHIGHLIGHT));
   CHECK(Pixel(&im, 79, 23) == GetSysColor(COLOR_3DDKSHADOW));
   CHECK(Pixel(&im, 5, 12) == GetSysColor(COLOR_BTNFACE));
   FreeImage(&im);
   // The default button has an outer frame in the window frame color.
   im = Render(def);
   CHECK(Pixel(&im, 0, 0) == GetSysColor(COLOR_WINDOWFRAME));
   FreeImage(&im);

   // Radio: the checked one has a dark dot in its middle.
   im = Render(r2);
   RECT dot = {4, 8, 9, 13};
   CHECK(CountDark(&im, dot) > 3);
   FreeImage(&im);

   // WM_CTLCOLORBTN / WM_CTLCOLORSTATIC go to the parent.
   memset(&rcv, 0, sizeof(rcv));
   im = Render(check);
   CHECK(rcv.ctlcolor[WM_CTLCOLORSTATIC - WM_CTLCOLORMSGBOX] + rcv.ctlcolor[WM_CTLCOLORBTN - WM_CTLCOLORMSGBOX] >= 1);
   FreeImage(&im);

   // Disabled: grayed text.
   EnableWindow(push, FALSE);
   im = Render(push);
   SaveBmp(&im, "ctl_button_disabled.bmp");
   RECT inner = {10, 5, 70, 19};
   CHECK(CountColor(&im, inner, GetSysColor(COLOR_BTNSHADOW)) > 5);
   FreeImage(&im);

   DestroyWindow(parent);
   return Report("ctl_button");
}

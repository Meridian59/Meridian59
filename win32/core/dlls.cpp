/*
 * dlls.cpp:  The Windows DLLs the layer provides (Wine's, from its source
 *   release, and its own), attached before WinMain as the loader would.
 */

#include "core/module.h"

extern "C" BOOL WINAPI COMCTL32_DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);

// user32's controls, which comctl32 v6 registers itself (RegisterClassNameW, on a
// class lookup that fails); Windows has them from the start, as the client expects.
extern "C" void BUTTON_Register(void);
extern "C" void COMBO_Register(void);
extern "C" void COMBOLBOX_Register(void);
extern "C" void EDIT_Register(void);
extern "C" void LISTBOX_Register(void);
extern "C" void STATIC_Register(void);

namespace w32 {

namespace {

WNDPROC wine_listbox_proc;

// Where Wine's ListBox and Windows' differ, and the client's offer and drop
// dialogs (its item list and amounts' list) rely on Windows':
// - LB_SETSEL is for multiple selection: Windows' does nothing in a single-
//   selection list box (LB_ERR), Wine's selects the item whichever way asked;
// - LB_GETCURSEL of a multiple-selection list box is its focus item (caret) in
//   Windows', in Wine's the item last selected by key, even after a click;
// - space toggling an LBS_MULTIPLESEL item notifies (LBN_SELCHANGE) in
//   Windows', not in Wine's.
LRESULT CALLBACK ListBoxProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   LONG style = GetWindowLongW(hwnd, GWL_STYLE);
   bool multiple = (style & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0;
   if (msg == WM_KEYDOWN && wparam == VK_SPACE && (style & LBS_MULTIPLESEL) && (style & LBS_NOTIFY))
   {
      int focus = (int) CallWindowProcW(wine_listbox_proc, hwnd, LB_GETCARETINDEX, 0, 0);
      LRESULT was = CallWindowProcW(wine_listbox_proc, hwnd, LB_GETSEL, focus, 0);
      LRESULT r = CallWindowProcW(wine_listbox_proc, hwnd, msg, wparam, lparam);
      if (focus >= 0 && IsWindow(hwnd) && CallWindowProcW(wine_listbox_proc, hwnd, LB_GETSEL, focus, 0) != was)
         SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_SELCHANGE), (LPARAM) hwnd);
      return r;
   }
   if (msg == LB_SETSEL && !multiple)
      return LB_ERR;
   if (msg == LB_GETCURSEL && multiple)
   {
      if (CallWindowProcW(wine_listbox_proc, hwnd, LB_GETCOUNT, 0, 0) <= 0)
         return LB_ERR;
      return CallWindowProcW(wine_listbox_proc, hwnd, LB_GETCARETINDEX, 0, 0);
   }
   return CallWindowProcW(wine_listbox_proc, hwnd, msg, wparam, lparam);
}

void WrapListBox()
{
   WNDCLASSW wc;
   if (!GetClassInfoW(nullptr, u"ListBox", &wc))
      return;
   wine_listbox_proc = wc.lpfnWndProc;
   UnregisterClassW(u"ListBox", nullptr);
   wc.lpfnWndProc = ListBoxProc;
   RegisterClassW(&wc);
}

}  // namespace

void RegisterScrollBarClass();
void RegisterRichEdit();

void AttachDlls()
{
   // comctl32 loads its resources (property sheet templates, bitmaps,
   // cursors) through its instance.
   COMCTL32_DllMain(BuiltinDll("comctl32.dll"), DLL_PROCESS_ATTACH, nullptr);
   BUTTON_Register();
   COMBO_Register();
   COMBOLBOX_Register();
   EDIT_Register();
   LISTBOX_Register();
   WrapListBox();
   STATIC_Register();
   RegisterScrollBarClass();
   // riched32.dll, the layer's own.
   RegisterRichEdit();
}

}  // namespace w32

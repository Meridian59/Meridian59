/*
 * core_dialog.c:  The dialog manager.  Parses every dialog of the client's
 *   compiled resources (the .res files in the build directory, or M59_RES_DIR),
 *   then checks templates, base units, keyboard navigation, DefDlgProc, the item
 *   helpers, modal loops, MessageBox and dialogs from other threads with
 *   stand-in controls registered here (the real ones live elsewhere).  Ends by
 *   itself; exit status 0 on success.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_check.h"

/************************************************************************/
// Stand-in controls

#define MAX_LOG 64

// What each control saw at creation, by control ID (low byte).
static struct
{
   BOOL created;
   BYTE name[4];           // first bytes of lpszName
   const WORD *data;       // lpCreateParams
   int keydowns;
   int chars;
} created_log[256];

static LRESULT CALLBACK StandInProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   char cls[64];
   GetClassNameA(hwnd, cls, sizeof(cls));
   int id = GetDlgCtrlID(hwnd) & 0xFF;
   switch (msg)
   {
   case WM_CREATE:
   {
      CREATESTRUCTA *cs = (CREATESTRUCTA *) lparam;
      created_log[id].created = TRUE;
      memset(created_log[id].name, 0, 4);
      if (cs->lpszName && !IS_INTRESOURCE(cs->lpszName))
         for (int i = 0; i < 3 && cs->lpszName[i]; i++)
            created_log[id].name[i] = (BYTE) cs->lpszName[i];
      created_log[id].data = (const WORD *) cs->lpCreateParams;
      return 0;
   }
   case WM_GETDLGCODE:
      if (!lstrcmpiA(cls, "Button"))
      {
         switch (GetWindowLongA(hwnd, GWL_STYLE) & BS_TYPEMASK)
         {
         case BS_PUSHBUTTON: return DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON;
         case BS_DEFPUSHBUTTON: return DLGC_BUTTON | DLGC_DEFPUSHBUTTON;
         case BS_RADIOBUTTON:
         case BS_AUTORADIOBUTTON: return DLGC_BUTTON | DLGC_RADIOBUTTON;
         case BS_GROUPBOX: return DLGC_STATIC;
         default: return DLGC_BUTTON;
         }
      }
      if (!lstrcmpiA(cls, "Edit"))
         return DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS;
      if (!lstrcmpiA(cls, "Static"))
         return DLGC_STATIC;
      if (!lstrcmpiA(cls, "WantAll"))
         return DLGC_WANTALLKEYS;
      return 0;
   case BM_SETCHECK:
      SetWindowLongPtrA(hwnd, GWLP_USERDATA, wparam);
      return 0;
   case BM_GETCHECK:
      return GetWindowLongPtrA(hwnd, GWLP_USERDATA);
   case BM_SETSTYLE:
      SetWindowLongA(hwnd, GWL_STYLE, (GetWindowLongA(hwnd, GWL_STYLE) & ~0xFFFF) | (WORD) wparam);
      return 0;
   case BM_CLICK:
      if ((GetWindowLongA(hwnd, GWL_STYLE) & BS_TYPEMASK) == BS_AUTORADIOBUTTON)
         SetWindowLongPtrA(hwnd, GWLP_USERDATA, BST_CHECKED);
      SendMessageA(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM) hwnd);
      return 0;
   case WM_SETFONT:
      SetPropA(hwnd, "font", (HANDLE) wparam);
      return 0;
   case WM_GETFONT:
      return (LRESULT) GetPropA(hwnd, "font");
   case EM_SETSEL:
      SetPropA(hwnd, "sel", (HANDLE) 1);
      return 0;
   case STM_SETICON:
      SetPropA(hwnd, "icon", (HANDLE) wparam);
      return 0;
   case WM_KEYDOWN:
      created_log[id].keydowns++;
      return 0;
   case WM_CHAR:
      created_log[id].chars++;
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

// Stand-ins for the predefined controls, the client's custom and common controls.
static const char *stand_in_classes[] = {
   "Button", "Edit", "Static", "ListBox", "ComboBox", "ScrollBar", "WantAll", "BlakGraph",
   "msctls_progress32", "msctls_trackbar32", "msctls_updown32", "SysAnimate32", "SysListView32",
   "SysTabControl32", "SysTreeView32", "RichEdit", "RichEdit20A", "RICHEDIT50W", "ComboBoxEx32",
   "SysDateTimePick32", "SysIPAddress32", "msctls_hotkey32", "SysLink", "SysHeader32",
};

static void RegisterStandIns(HINSTANCE instance)
{
   for (size_t i = 0; i < sizeof(stand_in_classes) / sizeof(stand_in_classes[0]); i++)
   {
      WNDCLASSA wc = {0};
      wc.lpfnWndProc = StandInProc;
      wc.hInstance = instance;
      wc.lpszClassName = stand_in_classes[i];
      RegisterClassA(&wc);
   }
   // The client's dialog classes ("Offer", "Mail"): DefDlgProc and DLGWINDOWEXTRA.
   const char *dialogs[] = {"Offer", "Mail"};
   for (int i = 0; i < 2; i++)
   {
      WNDCLASSA wc = {0};
      wc.lpfnWndProc = DefDlgProcA;
      wc.cbWndExtra = DLGWINDOWEXTRA;
      wc.hInstance = instance;
      wc.lpszClassName = dialogs[i];
      RegisterClassA(&wc);
   }
}

/************************************************************************/
// Templates

typedef struct
{
   union
   {
      WORD w[4096];
      DWORD align;
   };
   int n;
} Tmpl;

static void TW(Tmpl *t, WORD v) { t->w[t->n++] = v; }
static void TD(Tmpl *t, DWORD v) { TW(t, LOWORD(v)); TW(t, HIWORD(v)); }
static void TS(Tmpl *t, const char *s)
{
   while (*s)
      TW(t, (BYTE) *s++);
   TW(t, 0);
}
static void TAlign(Tmpl *t)
{
   if (t->n & 1)
      TW(t, 0);
}

static void Header(Tmpl *t, BOOL ex, DWORD style, DWORD exstyle, WORD count, short x, short y, short cx, short cy,
                   const char *cls, const char *title, WORD points, WORD weight, BYTE italic, const char *face)
{
   t->n = 0;
   if (ex)
   {
      TW(t, 1);
      TW(t, 0xFFFF);
      TD(t, 0);   // help ID
      TD(t, exstyle);
      TD(t, style);
   }
   else
   {
      TD(t, style);
      TD(t, exstyle);
   }
   TW(t, count);
   TW(t, x), TW(t, y), TW(t, cx), TW(t, cy);
   TW(t, 0);   // menu
   if (cls)
      TS(t, cls);
   else
      TW(t, 0);
   TS(t, title);
   if (style & DS_SETFONT)
   {
      TW(t, points);
      if (ex)
      {
         TW(t, weight);
         TW(t, italic | (DEFAULT_CHARSET << 8));
      }
      TS(t, face);
   }
}

// cls: a name, or NULL with atom; title_ordinal >= 0: an ordinal title.
static void Item(Tmpl *t, BOOL ex, DWORD style, DWORD exstyle, short x, short y, short cx, short cy, DWORD id,
                 const char *cls, WORD atom, const char *title, int title_ordinal, const BYTE *data, WORD size)
{
   TAlign(t);
   if (ex)
   {
      TD(t, 0);
      TD(t, exstyle);
      TD(t, style);
   }
   else
   {
      TD(t, style);
      TD(t, exstyle);
   }
   TW(t, x), TW(t, y), TW(t, cx), TW(t, cy);
   if (ex)
      TD(t, id);
   else
      TW(t, (WORD) id);
   if (cls)
      TS(t, cls);
   else
      TW(t, 0xFFFF), TW(t, atom);
   if (title_ordinal >= 0)
      TW(t, 0xFFFF), TW(t, (WORD) title_ordinal);
   else
      TS(t, title);
   TW(t, size);
   for (int i = 0; i < size; i += 2)
      TW(t, (WORD) (data[i] | ((i + 1 < size ? data[i + 1] : 0) << 8)));
}

/************************************************************************/
// Every dialog of the client's resources

static INT_PTR CALLBACK QuietProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   (void) hwnd, (void) wparam, (void) lparam;
   return FALSE;   // WM_INITDIALOG: no focus (the dialogs stay hidden)
}

static BYTE *ReadFile_(const char *path, long *size)
{
   FILE *f = fopen(path, "rb");
   if (f == NULL)
      return NULL;
   fseek(f, 0, SEEK_END);
   *size = ftell(f);
   fseek(f, 0, SEEK_SET);
   BYTE *data = (BYTE *) malloc(*size + 16);
   if (fread(data, 1, *size, f) != (size_t) *size)
      *size = 0;
   fclose(f);
   return data;
}

static int total_dialogs = 0;
static HWND res_parent;

static void ParseResFile(const char *path)
{
   long size = 0;
   BYTE *data = ReadFile_(path, &size);
   if (data == NULL)
   {
      printf("FAIL: can't read %s (build the .res files first, or set M59_RES_DIR)\n", path);
      check_failures++;
      return;
   }
   int dialogs = 0, items = 0, ex = 0;
   int class_counts[sizeof(stand_in_classes) / sizeof(stand_in_classes[0])] = {0};
   int dialog_classes = 0;
   const BYTE *p = data, *end = data + size;
   while (p + 8 <= end)
   {
      DWORD data_size, header_size;
      memcpy(&data_size, p, 4);
      memcpy(&header_size, p + 4, 4);
      if (header_size < 16 || p + header_size + data_size > end)
         break;
      const WORD *type = (const WORD *) (p + 8);
      if (type[0] == 0xFFFF && type[1] == 5 /* RT_DIALOG */)
      {
         // A copy, hidden.
         BYTE *t = (BYTE *) malloc(data_size + 16);
         memcpy(t, p + header_size, data_size);
         WORD version = ((WORD *) t)[0], signature = ((WORD *) t)[1];
         BOOL is_ex = version == 1 && signature == 0xFFFF;
         DWORD *style = (DWORD *) (t + (is_ex ? 12 : 0));
         *style &= ~WS_VISIBLE;
         WORD count = *(WORD *) (t + (is_ex ? 16 : 8));
         // Child dialogs (property pages) need a parent.
         HWND dlg = CreateDialogIndirectParamA(GetModuleHandleA(NULL), (LPCDLGTEMPLATEA) t, res_parent, QuietProc, 0);
         CHECK(dlg != NULL);
         if (dlg == NULL)
         {
            printf("  dialog failed: error %lu, ex %d, style %08lx, %u items, bytes:", (unsigned long) GetLastError(),
                   is_ex, (unsigned long) *style, count);
            for (int i = 0; i < 48 && i < (int) data_size; i++)
               printf(" %02x", t[i]);
            printf("\n");
         }
         if (dlg)
         {
            int children = 0;
            for (HWND c = GetWindow(dlg, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
            {
               children++;
               char cls[64];
               GetClassNameA(c, cls, sizeof(cls));
               for (size_t i = 0; i < sizeof(stand_in_classes) / sizeof(stand_in_classes[0]); i++)
                  if (!lstrcmpiA(cls, stand_in_classes[i]))
                     class_counts[i]++;
            }
            char cls[64];
            GetClassNameA(dlg, cls, sizeof(cls));
            if (strcmp(cls, "#32770") != 0)
               dialog_classes++;
            CHECK_INT(children, count);
            CHECK(!IsWindowVisible(dlg));
            DestroyWindow(dlg);
            CHECK(!IsWindow(dlg));
            items += children;
         }
         dialogs++;
         ex += is_ex;
         free(t);
      }
      p += (header_size + data_size + 3) & ~3u;
   }
   const char *name = strrchr(path, '/');
   printf("%s: %d dialogs (%d DIALOGEX, %d of own classes), %d controls:", name ? name + 1 : path, dialogs, ex,
          dialog_classes, items);
   for (size_t i = 0; i < sizeof(stand_in_classes) / sizeof(stand_in_classes[0]); i++)
      if (class_counts[i])
         printf(" %s %d", stand_in_classes[i], class_counts[i]);
   printf("\n");
   total_dialogs += dialogs;
   free(data);
}

static void TestClientDialogs(void)
{
   const char *dir = getenv("M59_RES_DIR");
   if (dir == NULL)
      dir = "../../res";   // from build-w32-*/run/tests
   res_parent = CreateWindowA("Static", "", WS_POPUP, 0, 0, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);
   // res/<target>/<name>.res
   const char *files[][2] = {{"meridian", "client"}, {"admin", "admin"}, {"char", "char"}, {"chess", "chess"},
                             {"dm", "dm"}, {"mailnews", "mailnews"}, {"merintr", "merintr"}};
   for (int i = 0; i < 7; i++)
   {
      char path[512];
      snprintf(path, sizeof(path), "%s/%s/%s.res", dir, files[i][0], files[i][1]);
      ParseResFile(path);
   }
   CHECK(total_dialogs > 100);
   DestroyWindow(res_parent);
}

/************************************************************************/
// A modeless dialog: creation, DefDlgProc, navigation, items

enum { ID_LABEL = 100, ID_EDIT, ID_APPLY, ID_DISABLED, ID_HIDDEN, ID_R1, ID_R2, ID_R3, ID_WANTALL = 110, ID_ICON };

static HWND init_focus;
static LPARAM init_param;
static int last_command;
static HFONT dialog_setfont;
static HBRUSH dlg_brush;

static INT_PTR CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_INITDIALOG:
      init_focus = (HWND) wparam;
      init_param = lparam;
      return TRUE;
   case WM_SETFONT:
      dialog_setfont = (HFONT) wparam;
      return TRUE;
   case WM_COMMAND:
      last_command = LOWORD(wparam);
      return TRUE;
   case WM_USER + 100:
      SetWindowLongPtrA(hwnd, DWLP_MSGRESULT, 1234);
      return TRUE;
   case WM_CTLCOLORDLG:
      return (INT_PTR) dlg_brush;
   }
   return FALSE;
}

static void Key(HWND hwnd, UINT msg, WPARAM key)
{
   MSG m = {0};
   m.hwnd = hwnd ? hwnd : GetFocus();
   m.message = msg;
   m.wParam = key;
   m.lParam = 1;
   IsDialogMessageA(GetParent(m.hwnd) ? GetParent(m.hwnd) : m.hwnd, &m);
}

static void Pump(void)
{
   MSG m;
   while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE))
   {
      TranslateMessage(&m);
      DispatchMessageA(&m);
   }
}

static void ExpectedUnits(HFONT font, int *cx, int *cy)
{
   HDC dc = CreateCompatibleDC(NULL);
   HGDIOBJ old = SelectObject(dc, font);
   SIZE s;
   TEXTMETRICA tm;
   GetTextExtentPoint32A(dc, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &s);
   GetTextMetricsA(dc, &tm);
   *cx = (s.cx / 26 + 1) / 2;
   *cy = tm.tmHeight;
   SelectObject(dc, old);
   DeleteDC(dc);
}

static WORD Btype(HWND hwnd)
{
   return (WORD) (GetWindowLongA(hwnd, GWL_STYLE) & BS_TYPEMASK);
}

static void TestModeless(HINSTANCE instance)
{
   static Tmpl t;
   static const BYTE data[] = {0xAB, 0xCD, 0xEF, 0x01};
   DWORD vis = WS_CHILD | WS_VISIBLE;
   Header(&t, FALSE, WS_POPUP | WS_CAPTION | DS_SETFONT | DS_MODALFRAME, 0, 12, 10, 10, 200, 120, NULL, "Modeless", 8,
          0, 0, "MS Shell Dlg");
   Item(&t, FALSE, vis | SS_LEFT, 0, 5, 10, 40, 10, ID_LABEL, NULL, 0x82, "&Name:", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP | WS_BORDER, 0, 50, 10, 100, 12, ID_EDIT, NULL, 0x81, "", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP | BS_PUSHBUTTON, 0, 5, 30, 50, 14, ID_APPLY, NULL, 0x80, "&Apply", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP | WS_DISABLED, 0, 60, 30, 50, 14, ID_DISABLED, NULL, 0x80, "Off", -1, NULL, 0);
   Item(&t, FALSE, WS_CHILD | WS_TABSTOP, 0, 115, 30, 50, 14, ID_HIDDEN, NULL, 0x80, "Hidden", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP | WS_GROUP | BS_AUTORADIOBUTTON, 0, 5, 50, 50, 10, ID_R1, NULL, 0x80, "One", -1,
        NULL, 0);
   Item(&t, FALSE, vis | BS_AUTORADIOBUTTON, 0, 5, 60, 50, 10, ID_R2, NULL, 0x80, "Two", -1, NULL, 0);
   Item(&t, FALSE, vis | BS_AUTORADIOBUTTON, 0, 5, 70, 50, 10, ID_R3, NULL, 0x80, "Three", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP | WS_GROUP | BS_DEFPUSHBUTTON, 0, 90, 100, 50, 14, IDOK, "button", 0, "OK", -1,
        NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP, 0, 145, 100, 50, 14, IDCANCEL, NULL, 0x80, "Cancel", -1, NULL, 0);
   Item(&t, FALSE, vis | WS_TABSTOP, 0, 90, 50, 50, 14, ID_WANTALL, "WantAll", 0, "", -1, data, 4);
   Item(&t, FALSE, vis | SS_ICON, 0, 150, 50, 20, 20, ID_ICON, NULL, 0x82, NULL, 0x1234, NULL, 0);

   dlg_brush = (HBRUSH) GetStockObject(GRAY_BRUSH);
   HWND dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, MainProc, 0x5150);
   CHECK(dlg != NULL);
   if (dlg == NULL)
      return;
   HWND edit = GetDlgItem(dlg, ID_EDIT), apply = GetDlgItem(dlg, ID_APPLY), ok = GetDlgItem(dlg, IDOK),
        cancel = GetDlgItem(dlg, IDCANCEL), r1 = GetDlgItem(dlg, ID_R1), r2 = GetDlgItem(dlg, ID_R2),
        r3 = GetDlgItem(dlg, ID_R3), wantall = GetDlgItem(dlg, ID_WANTALL), label = GetDlgItem(dlg, ID_LABEL);

   // Creation.
   char cls[32];
   GetClassNameA(dlg, cls, sizeof(cls));
   CHECK_STR(cls, "#32770");
   CHECK(!IsWindowVisible(dlg));
   CHECK(init_focus == edit);
   CHECK(init_param == 0x5150);
   CHECK(GetFocus() == edit);
   CHECK(GetPropA(edit, "sel") != NULL);
   CHECK(GetWindow(dlg, GW_CHILD) == label);
   CHECK(GetWindow(label, GW_HWNDNEXT) == edit);
   HFONT font = (HFONT) SendMessageA(dlg, WM_GETFONT, 0, 0);
   CHECK(font != NULL);
   CHECK(dialog_setfont == font);
   CHECK((HFONT) SendMessageA(edit, WM_GETFONT, 0, 0) == font);
   CHECK(!(GetWindowLongA(edit, GWL_STYLE) & WS_BORDER));
   CHECK(GetWindowLongA(edit, GWL_EXSTYLE) & WS_EX_CLIENTEDGE);
   CHECK(GetWindowLongA(edit, GWL_EXSTYLE) & WS_EX_NOPARENTNOTIFY);
   CHECK(created_log[ID_EDIT].data == NULL);
   CHECK(created_log[ID_WANTALL].data != NULL);
   if (created_log[ID_WANTALL].data)
   {
      CHECK_INT(created_log[ID_WANTALL].data[0], 4);
      CHECK(memcmp(created_log[ID_WANTALL].data + 1, data, 4) == 0);
   }
   CHECK_INT(created_log[ID_ICON].name[0], 0xFF);
   CHECK_INT(created_log[ID_ICON].name[1], 0x34);
   CHECK_INT(created_log[ID_ICON].name[2], 0x12);
   char text[64];
   GetWindowTextA(dlg, text, sizeof(text));
   CHECK_STR(text, "Modeless");

   // Base units.
   LOGFONTA lf;
   GetObjectA(font, sizeof(lf), &lf);
   CHECK_INT(lf.lfHeight, -11);
   CHECK_STR(lf.lfFaceName, "MS Shell Dlg");
   int bx, by;
   ExpectedUnits(font, &bx, &by);
   RECT r = {0, 0, 4, 8};
   CHECK(MapDialogRect(dlg, &r));
   CHECK_INT(r.right, bx);
   CHECK_INT(r.bottom, by);
   GetClientRect(dlg, &r);
   CHECK_INT(r.right, MulDiv(200, bx, 4));
   CHECK_INT(r.bottom, MulDiv(120, by, 8));
   GetWindowRect(edit, &r);
   MapWindowPoints(NULL, dlg, (POINT *) &r, 2);
   CHECK_INT(r.left, MulDiv(50, bx, 4));
   CHECK_INT(r.top, MulDiv(10, by, 8));
   CHECK_INT(r.right - r.left, MulDiv(100, bx, 4));
   CHECK_INT(r.bottom - r.top, MulDiv(12, by, 8));
   printf("dialog base units %dx%d (MS Shell Dlg 8)\n", bx, by);

   // Window longs, DWLP_MSGRESULT, procedure return values.
   CHECK(GetWindowLongPtrA(dlg, DWLP_DLGPROC) == (LONG_PTR) MainProc);
   SetWindowLongPtrA(dlg, DWLP_USER, 0x7777);
   CHECK(GetWindowLongPtrA(dlg, DWLP_USER) == 0x7777);
   CHECK_INT(SendMessageA(dlg, WM_USER + 100, 0, 0), 1234);
   CHECK_INT(SendMessageA(dlg, WM_USER + 101, 0, 0), 0);
   CHECK(SendMessageA(dlg, WM_CTLCOLORDLG, 0, (LPARAM) dlg) == (LRESULT) dlg_brush);
   CHECK_INT(SendMessageA(dlg, DM_GETDEFID, 0, 0), MAKELONG(IDOK, DC_HASDEFID));

   // Tab order: skips disabled, hidden and non-tabstop controls; wraps.
   Key(NULL, WM_KEYDOWN, VK_TAB);
   CHECK(GetFocus() == apply);
   CHECK_INT(Btype(apply), BS_DEFPUSHBUTTON);   // a focused push button is the default
   CHECK_INT(Btype(ok), BS_PUSHBUTTON);
   Key(NULL, WM_KEYDOWN, VK_RETURN);
   CHECK_INT(last_command, ID_APPLY);
   Key(NULL, WM_KEYDOWN, VK_TAB);
   CHECK(GetFocus() == r1);
   CHECK_INT(Btype(ok), BS_DEFPUSHBUTTON);       // back to the default ID's
   CHECK_INT(Btype(apply), BS_PUSHBUTTON);
   Key(NULL, WM_KEYDOWN, VK_TAB);
   CHECK(GetFocus() == ok);
   Key(NULL, WM_KEYDOWN, VK_TAB);
   CHECK(GetFocus() == cancel);
   Key(NULL, WM_KEYDOWN, VK_TAB);
   CHECK(GetFocus() == wantall);
   Key(NULL, WM_KEYDOWN, VK_TAB);                 // DLGC_WANTALLKEYS: the control's
   CHECK(GetFocus() == wantall);
   CHECK_INT(created_log[ID_WANTALL].keydowns, 1);
   SetFocus(edit);
   CHECK(GetNextDlgTabItem(dlg, edit, TRUE) == wantall);
   CHECK(GetNextDlgTabItem(dlg, NULL, FALSE) == edit);
   CHECK(GetNextDlgTabItem(dlg, wantall, FALSE) == edit);
   SendMessageA(dlg, WM_NEXTDLGCTL, 1, 0);
   CHECK(GetFocus() == wantall);
   SendMessageA(dlg, WM_NEXTDLGCTL, (WPARAM) ok, 1);
   CHECK(GetFocus() == ok);

   // Groups and arrows.
   CHECK(GetNextDlgGroupItem(dlg, r1, FALSE) == r2);
   CHECK(GetNextDlgGroupItem(dlg, r3, FALSE) == r1);
   CHECK(GetNextDlgGroupItem(dlg, r1, TRUE) == r3);
   SetFocus(r1);
   Key(NULL, WM_KEYDOWN, VK_DOWN);
   CHECK(GetFocus() == r2);
   CHECK_INT(IsDlgButtonChecked(dlg, ID_R2), BST_CHECKED);
   Key(NULL, WM_KEYDOWN, VK_DOWN);
   Key(NULL, WM_KEYDOWN, VK_DOWN);
   CHECK(GetFocus() == r1);
   Key(NULL, WM_KEYDOWN, VK_UP);
   CHECK(GetFocus() == r3);
   SetFocus(edit);
   Key(NULL, WM_KEYDOWN, VK_DOWN);                // DLGC_WANTARROWS
   CHECK(GetFocus() == edit);

   // Enter, Esc, WM_CLOSE.
   last_command = 0;
   Key(NULL, WM_KEYDOWN, VK_RETURN);
   CHECK_INT(last_command, IDOK);
   SendMessageA(dlg, DM_SETDEFID, IDCANCEL, 0);
   CHECK_INT(SendMessageA(dlg, DM_GETDEFID, 0, 0), MAKELONG(IDCANCEL, DC_HASDEFID));
   CHECK_INT(Btype(cancel), BS_DEFPUSHBUTTON);
   CHECK_INT(Btype(ok), BS_PUSHBUTTON);
   Key(NULL, WM_KEYDOWN, VK_RETURN);
   CHECK_INT(last_command, IDCANCEL);
   SendMessageA(dlg, DM_SETDEFID, IDOK, 0);
   last_command = 0;
   Key(NULL, WM_KEYDOWN, VK_ESCAPE);
   CHECK_INT(last_command, IDCANCEL);
   last_command = 0;
   SendMessageA(dlg, WM_CLOSE, 0, 0);
   CHECK_INT(last_command, 0);   // posted
   Pump();
   CHECK_INT(last_command, IDCANCEL);

   // Mnemonics: a static's moves to the next control, a button's clicks it.
   SetFocus(cancel);
   Key(NULL, WM_SYSCHAR, 'n');
   CHECK(GetFocus() == edit);
   last_command = 0;
   Key(NULL, WM_SYSCHAR, 'A');
   CHECK_INT(last_command, ID_APPLY);
   last_command = 0;
   int chars = created_log[ID_EDIT].chars;
   SetFocus(edit);
   Key(NULL, WM_CHAR, 'a');                       // the edit wants characters
   CHECK_INT(last_command, 0);
   CHECK_INT(created_log[ID_EDIT].chars, chars + 1);
   SetFocus(ok);
   Key(NULL, WM_CHAR, 'a');                       // a plain mnemonic elsewhere
   CHECK_INT(last_command, ID_APPLY);

   // Items.
   CHECK(SetDlgItemTextA(dlg, ID_EDIT, "hello"));
   CHECK_INT(GetDlgItemTextA(dlg, ID_EDIT, text, sizeof(text)), 5);
   CHECK_STR(text, "hello");
   CHECK_INT(GetDlgItemTextA(dlg, ID_EDIT, text, 3), 2);
   CHECK_STR(text, "he");
   WCHAR wtext[16];
   CHECK_INT(GetDlgItemTextW(dlg, ID_EDIT, wtext, 16), 5);
   CHECK(wtext[0] == 'h' && wtext[4] == 'o' && wtext[5] == 0);
   wchar_t wct[16];
   CHECK_INT(GetDlgItemTextW(dlg, ID_EDIT, wct, 16), 5);
   CHECK(wct[1] == L'e' && wct[5] == 0);
   BOOL translated;
   SetDlgItemInt(dlg, ID_EDIT, (UINT) -42, TRUE);
   GetDlgItemTextA(dlg, ID_EDIT, text, sizeof(text));
   CHECK_STR(text, "-42");
   CHECK_INT((INT) GetDlgItemInt(dlg, ID_EDIT, &translated, TRUE), -42);
   CHECK(translated);
   CHECK_INT(GetDlgItemInt(dlg, ID_EDIT, &translated, FALSE), 0);
   CHECK(!translated);
   SetDlgItemInt(dlg, ID_EDIT, 4000000000u, FALSE);
   CHECK(GetDlgItemInt(dlg, ID_EDIT, &translated, FALSE) == 4000000000u && translated);
   CHECK_INT(GetDlgItemInt(dlg, ID_EDIT, &translated, TRUE), 0);
   CHECK(!translated);
   SetDlgItemTextA(dlg, ID_EDIT, " 17 ");
   CHECK_INT(GetDlgItemInt(dlg, ID_EDIT, &translated, TRUE), 17);
   CHECK(translated);
   SetDlgItemTextA(dlg, ID_EDIT, "17x");
   CHECK_INT(GetDlgItemInt(dlg, ID_EDIT, &translated, TRUE), 0);
   CHECK(!translated);
   CHECK_INT(GetDlgItemInt(dlg, 999, &translated, TRUE), 0);
   CHECK(!translated);
   CheckDlgButton(dlg, ID_R1, BST_CHECKED);
   CHECK_INT(IsDlgButtonChecked(dlg, ID_R1), BST_CHECKED);
   CheckRadioButton(dlg, ID_R1, ID_R3, ID_R3);
   CHECK_INT(IsDlgButtonChecked(dlg, ID_R1), BST_UNCHECKED);
   CHECK_INT(IsDlgButtonChecked(dlg, ID_R2), BST_UNCHECKED);
   CHECK_INT(IsDlgButtonChecked(dlg, ID_R3), BST_CHECKED);
   CHECK_INT(SendDlgItemMessageA(dlg, ID_R3, BM_GETCHECK, 0, 0), BST_CHECKED);

   // EndDialog on a modeless dialog hides it.
   ShowWindow(dlg, SW_SHOWNOACTIVATE);
   CHECK(IsWindowVisible(dlg));
   CHECK(EndDialog(dlg, 5));
   CHECK(IsWindow(dlg));
   CHECK(!IsWindowVisible(dlg));
   CHECK(!EndDialog(edit, 5));
   DestroyWindow(dlg);
   CHECK(!IsWindow(dlg));
   CHECK(!MapDialogRect(dlg, &r));
}

/************************************************************************/
// Template variants

static void TestTemplates(HINSTANCE instance)
{
   static Tmpl t;
   // A control that can't be made fails the dialog, unless DS_NOFAILCREATE.
   for (int nofail = 0; nofail < 2; nofail++)
   {
      Header(&t, FALSE, WS_POPUP | (nofail ? DS_NOFAILCREATE : 0), 0, 2, 0, 0, 100, 50, NULL, "", 0, 0, 0, "");
      Item(&t, FALSE, WS_CHILD, 0, 0, 0, 10, 10, 1, "NoSuchClass", 0, "", -1, NULL, 0);
      Item(&t, FALSE, WS_CHILD, 0, 0, 0, 10, 10, 2, NULL, 0x80, "x", -1, NULL, 0);
      HWND dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, QuietProc, 0);
      if (!nofail)
         CHECK(dlg == NULL);
      else
      {
         CHECK(dlg != NULL);
         CHECK(GetDlgItem(dlg, 1) == NULL);
         CHECK(GetDlgItem(dlg, 2) != NULL);
         DestroyWindow(dlg);
      }
   }

   // DIALOGEX: font weight and italic, styles, DWORD IDs.
   Header(&t, TRUE, WS_POPUP | DS_SETFONT, WS_EX_TOOLWINDOW, 1, 0, 0, 100, 50, NULL, "Ex", 9, FW_BOLD, TRUE, "Tahoma");
   Item(&t, TRUE, WS_CHILD | WS_VISIBLE, WS_EX_STATICEDGE, 4, 4, 40, 10, 0x77, NULL, 0x82, "Static", -1, NULL, 0);
   HWND dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, MainProc, 0);
   CHECK(dlg != NULL);
   if (dlg)
   {
      LOGFONTA lf;
      GetObjectA((HFONT) SendMessageA(dlg, WM_GETFONT, 0, 0), sizeof(lf), &lf);
      CHECK_INT(lf.lfHeight, -12);
      CHECK_INT(lf.lfWeight, FW_BOLD);
      CHECK_INT(lf.lfItalic, TRUE);
      CHECK_STR(lf.lfFaceName, "Tahoma");
      CHECK(GetWindowLongA(dlg, GWL_EXSTYLE) & WS_EX_TOOLWINDOW);
      HWND s = GetDlgItem(dlg, 0x77);
      CHECK(s != NULL);
      CHECK(GetWindowLongA(s, GWL_EXSTYLE) & WS_EX_STATICEDGE);
      GetWindowTextA(s, (char *) t.w, 32);
      CHECK_STR((char *) t.w, "Static");
      DestroyWindow(dlg);
   }

   // DS_SHELLFONT: "MS Shell Dlg" is the shell's "MS Shell Dlg 2".
   Header(&t, TRUE, WS_POPUP | DS_SHELLFONT, 0, 0, 0, 0, 100, 50, NULL, "", 8, FW_NORMAL, FALSE, "MS Shell Dlg");
   dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, MainProc, 0);
   CHECK(dlg != NULL);
   if (dlg)
   {
      LOGFONTA lf;
      GetObjectA((HFONT) SendMessageA(dlg, WM_GETFONT, 0, 0), sizeof(lf), &lf);
      CHECK_STR(lf.lfFaceName, "MS Shell Dlg 2");
      DestroyWindow(dlg);
   }

   // No font: the system font's base units (GetDialogBaseUnits).
   Header(&t, FALSE, WS_POPUP, 0, 0, 0, 0, 100, 50, NULL, "", 0, 0, 0, "");
   dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, MainProc, 0);
   CHECK(dlg != NULL);
   if (dlg)
   {
      int bx, by;
      ExpectedUnits((HFONT) GetStockObject(SYSTEM_FONT), &bx, &by);
      DWORD units = GetDialogBaseUnits();
      CHECK_INT(LOWORD(units), bx);
      CHECK_INT(HIWORD(units), by);
      RECT r = {0, 0, 4, 8};
      MapDialogRect(dlg, &r);
      CHECK_INT(r.right, bx);
      CHECK_INT(r.bottom, by);
      CHECK(SendMessageA(dlg, WM_GETFONT, 0, 0) == 0);
      printf("system base units %dx%d\n", bx, by);
      DestroyWindow(dlg);
   }

   // A class named in the template; DS_CENTER.
   Header(&t, FALSE, WS_POPUP | DS_CENTER, 0, 0, 0, 0, 100, 50, "Offer", "", 0, 0, 0, "");
   dlg = CreateDialogIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, NULL, MainProc, 0);
   CHECK(dlg != NULL);
   if (dlg)
   {
      char cls[32];
      GetClassNameA(dlg, cls, sizeof(cls));
      CHECK_STR(cls, "Offer");
      RECT r, work;
      GetWindowRect(dlg, &r);
      SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0);
      CHECK(abs((r.left + r.right) / 2 - (work.left + work.right) / 2) <= 1);
      CHECK(abs((r.top + r.bottom) / 2 - (work.top + work.bottom) / 2) <= 1);
      CHECK(GetWindowLongPtrA(dlg, DWLP_DLGPROC) == (LONG_PTR) MainProc);
      CHECK_INT(SendMessageA(dlg, WM_USER + 100, 0, 0), 1234);
      DestroyWindow(dlg);
   }
}

/************************************************************************/
// Modal dialogs

static HWND owner;
static int enter_idle;
static HWND modal_dialog;
static int modal_mode;
static BOOL shown_during_init, owner_enabled_during_init, shown_at_timer;

static int owner_ticks, dialog_ticks;

static LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_ENTERIDLE && wparam == MSGF_DIALOGBOX && (HWND) lparam != NULL)
      enter_idle++;
   if (msg == WM_TIMER && wparam == 2)
   {
      // Slower to handle than its period, as the client's frames can be: due
      // again by every GetMessage.
      Sleep(30);
      if (++owner_ticks > 100)
         EndDialog(modal_dialog, -2);
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

// The news reader's: re-armed from WM_TIMER while the owner's is always due.
static INT_PTR CALLBACK FairProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   (void) wparam, (void) lparam;
   switch (msg)
   {
   case WM_INITDIALOG:
      modal_dialog = hwnd;
      SetTimer(owner, 2, 10, NULL);
      SetTimer(hwnd, 1, 20, NULL);
      return TRUE;
   case WM_TIMER:
      if (++dialog_ticks == 5)
         EndDialog(hwnd, 5);
      else
         SetTimer(hwnd, 1, 20, NULL);
      return TRUE;
   }
   return FALSE;
}

static INT_PTR CALLBACK ModalProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   (void) lparam;
   switch (msg)
   {
   case WM_INITDIALOG:
      modal_dialog = hwnd;
      shown_during_init = IsWindowVisible(hwnd);
      owner_enabled_during_init = IsWindowEnabled(owner);
      if (modal_mode == 2)
         EndDialog(hwnd, 7);
      else
         SetTimer(hwnd, 1, 150, NULL);
      return TRUE;
   case WM_TIMER:
      KillTimer(hwnd, 1);
      shown_at_timer = IsWindowVisible(hwnd);
      if (modal_mode == 0)
         EndDialog(hwnd, 42);
      else
         // Through the modal loop, to a control: not GetFocus(), which is null
         // while the desktop hasn't given the dialog the keyboard.
         PostMessageA(GetDlgItem(hwnd, IDOK), WM_KEYDOWN, VK_ESCAPE, 1);
      return TRUE;
   case WM_COMMAND:
      if (LOWORD(wparam) == IDCANCEL)
         EndDialog(hwnd, IDCANCEL);
      return TRUE;
   }
   return FALSE;
}

static void TestModal(HINSTANCE instance)
{
   WNDCLASSA wc = {0};
   wc.lpfnWndProc = OwnerProc;
   wc.hInstance = instance;
   wc.lpszClassName = "TestOwner";
   RegisterClassA(&wc);
   owner = CreateWindowA("TestOwner", "owner", WS_POPUP, 0, 0, 100, 100, NULL, NULL, instance, NULL);

   static Tmpl t;
   Header(&t, FALSE, WS_POPUP | WS_CAPTION | DS_MODALFRAME | DS_SETFONT | DS_CENTER, 0, 2, 0, 0, 120, 60, NULL,
          "Modal", 8, 0, 0, "MS Shell Dlg");
   Item(&t, FALSE, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 0, 10, 40, 40, 14, IDOK, NULL, 0x80, "OK",
        -1, NULL, 0);
   Item(&t, FALSE, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 60, 40, 40, 14, IDCANCEL, NULL, 0x80, "Cancel", -1, NULL, 0);

   for (modal_mode = 0; modal_mode < 3; modal_mode++)
   {
      enter_idle = 0;
      shown_at_timer = FALSE;
      INT_PTR r = DialogBoxIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, owner, ModalProc, 0);
      CHECK_INT(r, modal_mode == 0 ? 42 : modal_mode == 1 ? IDCANCEL : 7);
      CHECK(!shown_during_init);
      CHECK(!owner_enabled_during_init);
      CHECK(IsWindowEnabled(owner));
      CHECK(!IsWindow(modal_dialog));
      if (modal_mode < 2)
      {
         CHECK(shown_at_timer);
         CHECK(enter_idle > 0);
      }
   }

   // Every due timer has its turn, not just the first window's.
   CHECK_INT(DialogBoxIndirectParamA(instance, (LPCDLGTEMPLATEA) t.w, owner, FairProc, 0), 5);
   KillTimer(owner, 2);
   CHECK(owner_ticks > 0);

   CHECK_INT(DialogBoxParamA(instance, MAKEINTRESOURCEA(999), owner, ModalProc, 0), -1);
   CHECK_INT(DialogBoxParamA(instance, MAKEINTRESOURCEA(999), (HWND) (UINT_PTR) 0x1234, ModalProc, 0), 0);
   CHECK(CreateDialogParamA(instance, MAKEINTRESOURCEA(999), NULL, ModalProc, 0) == NULL);
}

/************************************************************************/
// MessageBox

static int box_step, box_case, box_ticks;
static char box_text[256];

static VOID CALLBACK BoxTimer(HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
{
   (void) hwnd, (void) msg, (void) time;
   HWND focus = GetFocus();
   HWND box = focus ? GetParent(focus) : NULL;
   if (++box_ticks > 100)
   {
      printf("FAIL: message box %d not found\n", box_case);
      exit(1);
   }
   if (box == NULL || !IsWindowVisible(box))
      return;
   char cls[32];
   GetClassNameA(box, cls, sizeof(cls));
   CHECK_STR(cls, "#32770");
   GetDlgItemTextA(box, 0xFFFF, box_text, sizeof(box_text));
   switch (box_case)
   {
   case 0:   // MB_OK | MB_ICONINFORMATION, no owner: Esc answers OK
   {
      CHECK(GetDlgItem(box, IDOK) == focus);
      CHECK(GetDlgItem(box, IDCANCEL) == NULL);
      CHECK(GetDlgItem(box, 0x14) != NULL);
      CHECK(SendMessageA(GetDlgItem(box, 0x14), STM_GETICON, 0, 0) != 0);
      char caption[64];
      GetWindowTextA(box, caption, sizeof(caption));
      CHECK_STR(caption, "Caption");
      // Layout: the button inside the client area, below the text.
      RECT client, button, text;
      GetClientRect(box, &client);
      GetWindowRect(GetDlgItem(box, IDOK), &button);
      GetWindowRect(GetDlgItem(box, 0xFFFF), &text);
      MapWindowPoints(NULL, box, (POINT *) &button, 2);
      MapWindowPoints(NULL, box, (POINT *) &text, 2);
      CHECK(button.left >= 0 && button.right <= client.right && button.bottom <= client.bottom);
      CHECK(text.right > text.left && text.bottom > text.top && text.bottom <= button.top);
      CHECK(abs((button.left + button.right) / 2 - client.right / 2) <= 1);
      PostMessageA(focus, WM_KEYDOWN, VK_ESCAPE, 1);
      break;
   }
   case 1:   // MB_YESNO | MB_DEFBUTTON2 with an owner: Enter answers No
      CHECK(GetDlgItem(box, IDNO) == focus);
      CHECK(!IsWindowEnabled(owner));
      {
         RECT yes, no;
         GetWindowRect(GetDlgItem(box, IDYES), &yes);
         GetWindowRect(GetDlgItem(box, IDNO), &no);
         CHECK(yes.right < no.left);
         CHECK(yes.top == no.top && yes.right - yes.left == no.right - no.left);
      }
      PostMessageA(focus, WM_KEYDOWN, VK_RETURN, 1);
      break;
   case 2:   // MB_YESNO: Esc does nothing; then Yes
      if (box_step++ == 0)
         PostMessageA(focus, WM_KEYDOWN, VK_ESCAPE, 1);
      else
         SendMessageA(GetDlgItem(box, IDYES), BM_CLICK, 0, 0);
      return;
   case 3:   // MB_YESNOCANCEL: Esc is Cancel; the default caption
   {
      char caption[64];
      GetWindowTextA(box, caption, sizeof(caption));
      CHECK_STR(caption, "Error");
      CHECK(GetDlgItem(box, IDYES) == focus);
      PostMessageA(focus, WM_KEYDOWN, VK_ESCAPE, 1);
      break;
   }
   }
   KillTimer(NULL, id);
}

// A system icon (LoadIcon(NULL, IDI_*)) is a 32x32 image with something drawn.
static void CheckSystemIcon(LPCSTR id)
{
   HICON icon = LoadIconA(NULL, id);
   ICONINFO info;
   CHECK(icon != NULL && GetIconInfo(icon, &info));
   if (icon == NULL || !GetIconInfo(icon, &info))
      return;
   BITMAP bm;
   GetObjectA(info.hbmColor, sizeof(bm), &bm);
   CHECK(bm.bmWidth == 32 && bm.bmHeight == 32);
   BITMAPINFO bi = {{sizeof(BITMAPINFOHEADER), 32, -32, 1, 32, BI_RGB}};
   DWORD pixels[32 * 32];
   HDC dc = CreateCompatibleDC(NULL);
   GetDIBits(dc, info.hbmColor, 0, 32, pixels, &bi, DIB_RGB_COLORS);
   DeleteDC(dc);
   int opaque = 0;
   for (int i = 0; i < 32 * 32; i++)
      opaque += (pixels[i] >> 24) != 0;
   CHECK(opaque > 100);
   DeleteObject(info.hbmColor);
   DeleteObject(info.hbmMask);
}

static void TestMessageBox(void)
{
   CheckSystemIcon(IDI_HAND);
   CheckSystemIcon(IDI_QUESTION);
   CheckSystemIcon(IDI_EXCLAMATION);
   CheckSystemIcon(IDI_ASTERISK);

   static const struct
   {
      HWND *owner;
      const char *caption;
      UINT type;
      int expect;
   } cases[] = {
      {NULL, "Caption", MB_OK | MB_ICONINFORMATION, IDOK},
      {&owner, "Caption", MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION, IDNO},
      {NULL, "Caption", MB_YESNO, IDYES},
      {NULL, NULL, MB_YESNOCANCEL, IDCANCEL},
   };
   for (box_case = 0; box_case < 4; box_case++)
   {
      box_step = box_ticks = 0;
      SetTimer(NULL, 0, 100, BoxTimer);
      const char *text = "A message long enough that it has to wrap onto more than one line in the message box.";
      int r = MessageBoxA(cases[box_case].owner ? *cases[box_case].owner : NULL, text, cases[box_case].caption,
                          cases[box_case].type);
      CHECK_INT(r, cases[box_case].expect);
      CHECK_STR(box_text, text);
   }
   CHECK(IsWindowEnabled(owner));
}

static VOID CALLBACK Watchdog(HWND, UINT, UINT_PTR, DWORD)
{
   printf("FAIL: timed out\n");
   exit(1);
}

/************************************************************************/
// From another thread: the dialog and the window are made, and the dialog's
// procedure runs, on the GUI thread (as AppKit requires), while it pumps.

static DWORD gui_thread_id, proc_thread_id;
static Tmpl thread_tmpl;
static INT_PTR thread_result;
static HWND thread_window;

static INT_PTR CALLBACK ThreadDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   (void) wparam, (void) lparam;
   if (msg != WM_INITDIALOG)
      return FALSE;
   proc_thread_id = GetCurrentThreadId();
   EndDialog(hwnd, 77);
   return TRUE;
}

static DWORD WINAPI DialogThread(LPVOID instance)
{
   thread_result = DialogBoxIndirectParamA((HINSTANCE) instance, (LPCDLGTEMPLATEA) thread_tmpl.w, NULL,
                                           ThreadDialogProc, 0);
   thread_window = CreateWindowA("Static", "from a thread", WS_POPUP, 0, 0, 10, 10, NULL, NULL,
                                 (HINSTANCE) instance, NULL);
   return 0;
}

static void TestFromThread(HINSTANCE instance)
{
   Header(&thread_tmpl, FALSE, WS_POPUP | WS_CAPTION | DS_SETFONT, 0, 0, 0, 0, 100, 40, NULL, "Thread", 8, 0, 0,
          "MS Shell Dlg");
   gui_thread_id = GetCurrentThreadId();
   HANDLE thread = CreateThread(NULL, 0, DialogThread, instance, 0, NULL);
   CHECK(thread != NULL);
   while (thread && WaitForSingleObject(thread, 10) == WAIT_TIMEOUT)
   {
      MSG m;
      while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE))
         DispatchMessageA(&m);
   }
   if (thread)
      CloseHandle(thread);
   CHECK_INT(thread_result, 77);
   CHECK(proc_thread_id == gui_thread_id);
   CHECK(IsWindow(thread_window));
   DestroyWindow(thread_window);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   SetTimer(NULL, 0, 60000, Watchdog);
   RegisterStandIns(instance);
   TestClientDialogs();
   TestTemplates(instance);
   TestModeless(instance);
   TestModal(instance);
   TestMessageBox();
   TestFromThread(instance);
   DestroyWindow(owner);
   return CheckReport("core_dialog");
}

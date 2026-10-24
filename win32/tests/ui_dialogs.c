/*
 * ui_dialogs.c:  Every dialog of the client's compiled resources (the .res files
 *   in the build directory, or M59_RES_DIR), made with the real controls and
 *   painted: what painting puts on the surface must be what WM_PRINT draws (a
 *   control that skips its frame, or paints in the wrong order, differs).  Also
 *   reports controls outside their dialog and text that doesn't fit its control,
 *   and saves each dialog as painted in ui_dialogs/ for review.
 */

#include "tests/ctl_common.h"

#include <stdlib.h>
#include <sys/stat.h>

static int dialogs, painted_diffs, out_of_bounds, clipped;

// The client's own control and dialog classes: plain boxes here.
static LRESULT CALLBACK StandInProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_PAINT || msg == WM_PRINTCLIENT)
   {
      PAINTSTRUCT ps;
      HDC hdc = msg == WM_PAINT ? BeginPaint(hwnd, &ps) : (HDC) wparam;
      RECT r;
      GetClientRect(hwnd, &r);
      FillRect(hdc, &r, (HBRUSH) GetStockObject(GRAY_BRUSH));
      if (msg == WM_PAINT)
         EndPaint(hwnd, &ps);
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void RegisterStandIns(void)
{
   const char *controls[] = {"WantAll", "BlakGraph"};
   for (int i = 0; i < 2; i++)
   {
      WNDCLASSA wc = {0};
      wc.lpfnWndProc = StandInProc;
      wc.hInstance = GetModuleHandleA(NULL);
      wc.lpszClassName = controls[i];
      RegisterClassA(&wc);
   }
   const char *dlgs[] = {"Offer", "Mail"};
   for (int i = 0; i < 2; i++)
   {
      WNDCLASSA wc = {0};
      wc.lpfnWndProc = DefDlgProcA;
      wc.cbWndExtra = DLGWINDOWEXTRA;
      wc.hInstance = GetModuleHandleA(NULL);
      wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
      wc.lpszClassName = dlgs[i];
      RegisterClassA(&wc);
   }
}

static INT_PTR CALLBACK QuietProc(HWND, UINT, WPARAM, LPARAM)
{
   return FALSE;   // no focus, so no caret in the pictures
}

static struct Image NewImage(int width, int height)
{
   struct Image im;
   im.width = width;
   im.height = height;
   BITMAPINFO bi = {0};
   bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   bi.bmiHeader.biWidth = width;
   bi.bmiHeader.biHeight = -height;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   im.dc = CreateCompatibleDC(NULL);
   im.bitmap = CreateDIBSection(im.dc, &bi, DIB_RGB_COLORS, (void **) &im.bits, NULL, 0);
   SelectObject(im.dc, im.bitmap);
   for (int i = 0; i < width * height; i++)
      im.bits[i] = 0xFF00FF;
   return im;
}

// Text that doesn't fit: push buttons and statics on one line, check boxes and
// radio buttons after their box, word-wrapped statics in their height.
static void CheckText(HWND dlg, HWND c, const char *name)
{
   char cls[32], text[512];
   GetClassNameA(c, cls, sizeof(cls));
   if (GetWindowTextA(c, text, sizeof(text)) <= 0)
      return;
   DWORD style = GetWindowLongA(c, GWL_STYLE);
   RECT r;
   GetClientRect(c, &r);
   HDC hdc = GetDC(c);
   HFONT old = (HFONT) SelectObject(hdc, (HFONT) SendMessageA(c, WM_GETFONT, 0, 0));
   RECT need = {0, 0, r.right, 0};
   int room = r.right, height = r.bottom;
   UINT flags = DT_CALCRECT | DT_SINGLELINE;
   if (!lstrcmpiA(cls, "Button"))
   {
      DWORD type = style & BS_TYPEMASK;
      if (type == BS_GROUPBOX || type == BS_OWNERDRAW)
         goto done;
      if (type == BS_CHECKBOX || type == BS_AUTOCHECKBOX || type == BS_RADIOBUTTON || type == BS_AUTORADIOBUTTON ||
          type == BS_3STATE || type == BS_AUTO3STATE)
         room -= 16;   // the box and its gap
      else
         room -= 6;    // the push button's edge
      if (style & BS_MULTILINE)
         flags = DT_CALCRECT | DT_WORDBREAK;
   }
   else if (!lstrcmpiA(cls, "Static"))
   {
      DWORD type = style & SS_TYPEMASK;
      if (type == SS_LEFT || type == SS_CENTER || type == SS_RIGHT)
         flags = DT_CALCRECT | DT_WORDBREAK;
      else if (type != SS_SIMPLE && type != SS_LEFTNOWORDWRAP)
         goto done;
   }
   else
      goto done;
   need.right = room;
   DrawTextA(hdc, text, -1, &need, flags | ((style & SS_NOPREFIX) && !lstrcmpiA(cls, "Static") ? DT_NOPREFIX : 0));
   if (need.right > room + 1 || need.bottom > height + 1)
   {
      RECT w;
      GetWindowRect(c, &w);
      MapWindowPoints(NULL, dlg, (POINT *) &w, 2);
      printf("  %s: text clipped in %s %d at (%ld,%ld %ldx%ld), needs %ldx%ld: \"%s\"\n", name, cls,
             GetDlgCtrlID(c), w.left, w.top, (long) r.right, (long) r.bottom, need.right, need.bottom, text);
      clipped++;
   }
done:
   SelectObject(hdc, old);
   ReleaseDC(c, hdc);
}

static void CheckDialog(HWND parent, const BYTE *tmpl, DWORD size, const char *name)
{
   BYTE *t = (BYTE *) malloc(size + 16);
   memcpy(t, tmpl, size);
   WORD version = ((WORD *) t)[0], signature = ((WORD *) t)[1];
   BOOL ex = version == 1 && signature == 0xFFFF;
   DWORD *style = (DWORD *) (t + (ex ? 12 : 0));
   *style &= ~WS_VISIBLE;   // never on screen
   BOOL child = (*style & WS_CHILD) != 0;
   HWND dlg = CreateDialogIndirectParamA(GetModuleHandleA(NULL), (LPCDLGTEMPLATEA) t, child ? parent : NULL, QuietProc, 0);
   free(t);
   CHECK(dlg != NULL);
   if (dlg == NULL)
   {
      printf("  %s: not created (error %lu)\n", name, (unsigned long) GetLastError());
      return;
   }
   dialogs++;
   if (child)
      SetWindowPos(dlg, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
   SetWindowLongA(dlg, GWL_STYLE, GetWindowLongA(dlg, GWL_STYLE) | WS_VISIBLE);
   RECT client;
   GetClientRect(dlg, &client);

   for (HWND c = GetWindow(dlg, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
   {
      if (!(GetWindowLongA(c, GWL_STYLE) & WS_VISIBLE))
         continue;
      RECT r;
      GetWindowRect(c, &r);
      MapWindowPoints(NULL, dlg, (POINT *) &r, 2);
      char cls[32];
      GetClassNameA(c, cls, sizeof(cls));
      // A drop-down combo box's rect includes its list.
      if (lstrcmpiA(cls, "ComboBox") != 0 &&
          (r.left < 0 || r.top < 0 || r.right > client.right || r.bottom > client.bottom))
      {
         printf("  %s: %s %d at (%ld,%ld)-(%ld,%ld) outside the %ldx%ld dialog\n", name, cls, GetDlgCtrlID(c), r.left,
                r.top, r.right, r.bottom, client.right, client.bottom);
         out_of_bounds++;
      }
      CheckText(dlg, c, name);
   }

   if (client.right > 0 && client.bottom > 0)
   {
      InvalidateRect(dlg, NULL, TRUE);
      UpdateWindow(dlg);
      Pump();
      struct Image painted = NewImage(client.right, client.bottom);
      HDC wdc = GetDC(dlg);
      BitBlt(painted.dc, 0, 0, client.right, client.bottom, wdc, 0, 0, SRCCOPY);
      ReleaseDC(dlg, wdc);
      // WM_PRINT's DC is the whole window's: the client area at its offset in it.
      RECT win;
      GetWindowRect(dlg, &win);
      POINT origin = {0, 0};
      ClientToScreen(dlg, &origin);
      int ox = origin.x - win.left, oy = origin.y - win.top;
      struct Image whole = NewImage(win.right - win.left, win.bottom - win.top);
      SendMessageA(dlg, WM_PRINT, (WPARAM) whole.dc, PRF_CLIENT | PRF_ERASEBKGND | PRF_CHILDREN);
      struct Image printed = NewImage(client.right, client.bottom);
      BitBlt(printed.dc, 0, 0, client.right, client.bottom, whole.dc, ox, oy, SRCCOPY);
      FreeImage(&whole);
      int diff = 0;
      for (int i = 0; i < client.right * client.bottom; i++)
         diff += (painted.bits[i] & 0xFFFFFF) != (printed.bits[i] & 0xFFFFFF);
      char path[256];
      snprintf(path, sizeof(path), "ui_dialogs/%s.bmp", name);
      SaveBmp(&painted, path);
      if (diff)
      {
         printf("FAIL %s: %d pixels differ between painting and WM_PRINT\n", name, diff);
         snprintf(path, sizeof(path), "ui_dialogs/%s.printed.bmp", name);
         SaveBmp(&printed, path);
         painted_diffs++;
      }
      checks++;
      failures += diff != 0;
      FreeImage(&painted);
      FreeImage(&printed);
   }
   DestroyWindow(dlg);
}

static void CheckResFile(HWND parent, const char *dir, const char *target, const char *file)
{
   char path[512];
   snprintf(path, sizeof(path), "%s/%s/%s.res", dir, target, file);
   FILE *f = fopen(path, "rb");
   CHECK(f != NULL);
   if (f == NULL)
   {
      printf("  can't read %s (build the .res files first, or set M59_RES_DIR)\n", path);
      return;
   }
   fseek(f, 0, SEEK_END);
   long size = ftell(f);
   fseek(f, 0, SEEK_SET);
   BYTE *data = (BYTE *) malloc(size);
   if (fread(data, 1, size, f) != (size_t) size)
      size = 0;
   fclose(f);
   // RES entries: sizes, type, name, then (DWORD-aligned) version, flags, language.
   for (const BYTE *p = data; p + 32 <= data + size;)
   {
      DWORD data_size, header_size;
      memcpy(&data_size, p, 4);
      memcpy(&header_size, p + 4, 4);
      if (header_size < 16 || p + header_size + data_size > data + size)
         break;
      const WORD *type = (const WORD *) (p + 8);
      if (type[0] == 0xFFFF && type[1] == 5 /* RT_DIALOG */)
      {
         const WORD *id = type + 2;
         WORD language;
         memcpy(&language, p + header_size - 10, 2);
         char name[96];
         if (id[0] == 0xFFFF)
            snprintf(name, sizeof(name), "%s_%u_%04x", file, id[1], language);
         else
         {
            int n = snprintf(name, sizeof(name), "%s_", file);
            for (int i = 0; id[i] && n < (int) sizeof(name) - 8; i++)
               name[n++] = (char) id[i];
            snprintf(name + n, sizeof(name) - n, "_%04x", language);
         }
         CheckDialog(parent, p + header_size, data_size, name);
      }
      p += (header_size + data_size + 3) & ~3u;
   }
   free(data);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   InitCommonControls();
   RegisterStandIns();
   mkdir("ui_dialogs", 0755);
   const char *dir = getenv("M59_RES_DIR");
   if (dir == NULL)
      dir = "../../res";   // from build-w32-*/run/tests
   HWND parent = MakeParent(800, 600);   // for the child dialogs (property pages)
   const char *files[][2] = {{"meridian", "client"}, {"admin", "admin"}, {"char", "char"}, {"chess", "chess"},
                             {"dm", "dm"}, {"mailnews", "mailnews"}, {"merintr", "merintr"}};
   for (int i = 0; i < 7; i++)
      CheckResFile(parent, dir, files[i][0], files[i][1]);
   CHECK(dialogs > 100);
   printf("ui_dialogs: %d dialogs, %d painted differently, %d controls outside, %d texts clipped\n", dialogs,
          painted_diffs, out_of_bounds, clipped);
   DestroyWindow(parent);
   return Report("ui_dialogs");
}

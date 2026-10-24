/*
 * ctl_static.c:  Wine's Static on the layer: text (left, centered, right) with
 *   Latin-1, WM_CTLCOLORSTATIC, SS_NOTIFY (STN_CLICKED; without it the mouse goes
 *   through: HTTRANSPARENT), SS_ICON with STM_SETIMAGE/Static_SetIcon, SS_BITMAP,
 *   SS_ETCHEDHORZ/SS_ETCHEDFRAME, rendering.
 */

#include "tests/ctl_common.h"

// The dark pixels' horizontal extent in a rectangle.
static void DarkSpan(const struct Image *im, RECT r, int *left, int *right)
{
   *left = 10000, *right = -1;
   for (int y = r.top; y < r.bottom; y++)
      for (int x = r.left; x < r.right; x++)
         if (IsDark(Pixel(im, x, y)))
         {
            if (x < *left)
               *left = x;
            if (x > *right)
               *right = x;
         }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(400, 300);
   HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
   char text[64];

   HWND left = CreateWindowExA(0, "STATIC", "Gr\xFC\xDF" "e", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 10, 200, 16, parent,
                               (HMENU) 501, instance, NULL);
   HWND center = CreateWindowExA(0, "Static", "mid", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 30, 200, 16, parent,
                                 (HMENU) 502, instance, NULL);
   HWND right = CreateWindowExA(0, "static", "end", WS_CHILD | WS_VISIBLE | SS_RIGHT, 10, 50, 200, 16, parent,
                                (HMENU) 503, instance, NULL);
   CHECK(left && center && right);
   SendMessageA(left, WM_SETFONT, (WPARAM) font, FALSE);
   SendMessageA(center, WM_SETFONT, (WPARAM) font, FALSE);
   SendMessageA(right, WM_SETFONT, (WPARAM) font, FALSE);
   GetWindowTextA(left, text, sizeof(text));
   CHECK_STR(text, "Gr\xFC\xDF" "e");
   Static_SetText(left, "\xC0 la carte");
   Static_GetText(left, text, sizeof(text));
   CHECK_STR(text, "\xC0 la carte");

   // Alignment.
   RECT all = {0, 0, 200, 16};
   int l, r;
   struct Image im = Render(left);
   DarkSpan(&im, all, &l, &r);
   CHECK(l < 5);
   FreeImage(&im);
   im = Render(center);
   DarkSpan(&im, all, &l, &r);
   CHECK(l > 80 && r < 120);
   FreeImage(&im);
   im = Render(right);
   DarkSpan(&im, all, &l, &r);
   CHECK(r > 190);
   FreeImage(&im);

   // WM_CTLCOLORSTATIC: the parent's brush and text color.
   memset(&rcv, 0, sizeof(rcv));
   rcv.ctl_brush = CreateSolidBrush(RGB(10, 200, 10));
   im = Render(left);
   CHECK(rcv.ctlcolor[WM_CTLCOLORSTATIC - WM_CTLCOLORMSGBOX] >= 1);
   CHECK(Pixel(&im, 190, 8) == RGB(10, 200, 10));
   FreeImage(&im);
   DeleteObject(rcv.ctl_brush);
   rcv.ctl_brush = NULL;

   // Without SS_NOTIFY a static is transparent to the mouse; with it, STN_CLICKED.
   RECT wr;
   GetWindowRect(left, &wr);
   CHECK_INT(SendMessageA(left, WM_NCHITTEST, 0, MAKELPARAM(wr.left + 5, wr.top + 5)), HTTRANSPARENT);
   HWND notify = CreateWindowExA(0, "Static", "click", WS_CHILD | WS_VISIBLE | SS_NOTIFY, 10, 70, 100, 16, parent,
                                 (HMENU) 504, instance, NULL);
   GetWindowRect(notify, &wr);
   CHECK_INT(SendMessageA(notify, WM_NCHITTEST, 0, MAKELPARAM(wr.left + 5, wr.top + 5)), HTCLIENT);
   memset(&rcv, 0, sizeof(rcv));
   Click(notify, 5, 5);
   CHECK_INT(rcv.last_code, STN_CLICKED);
   CHECK_INT(rcv.last_id, 504);

   // An icon: SS_ICON sizes itself to it; STM_SETIMAGE and Static_SetIcon.
   BITMAPINFO bi = {0};
   bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   bi.bmiHeader.biWidth = 16;
   bi.bmiHeader.biHeight = -16;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   DWORD *bits;
   HBITMAP color = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void **) &bits, NULL, 0);
   for (int i = 0; i < 256; i++)
      bits[i] = (i % 16 < 8) ? 0x00FF0000 : 0x000000FF;   // red | blue, no alpha: the mask decides
   HBITMAP mask = CreateBitmap(16, 16, 1, 1, NULL);   // all 0: opaque
   ICONINFO ii = {TRUE, 0, 0, mask, color};
   HICON icon = CreateIconIndirect(&ii);
   CHECK(icon != NULL);
   HWND ico = CreateWindowExA(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_ICON, 220, 10, 0, 0, parent, (HMENU) 505,
                              instance, NULL);
   CHECK(Static_SetIcon(ico, icon) == NULL);
   CHECK(Static_GetIcon(ico, 0) == icon);
   CHECK((HICON) SendMessageA(ico, STM_GETIMAGE, IMAGE_ICON, 0) == icon);
   GetWindowRect(ico, &wr);
   CHECK_INT(wr.right - wr.left, 16);
   CHECK_INT(wr.bottom - wr.top, 16);
   im = Render(ico);
   CHECK(Pixel(&im, 2, 8) == RGB(255, 0, 0));
   CHECK(Pixel(&im, 12, 8) == RGB(0, 0, 255));
   FreeImage(&im);

   // A bitmap.
   HWND bmp = CreateWindowExA(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_BITMAP, 250, 10, 0, 0, parent,
                              (HMENU) 506, instance, NULL);
   SendMessageA(bmp, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) color);
   CHECK((HBITMAP) SendMessageA(bmp, STM_GETIMAGE, IMAGE_BITMAP, 0) == color);
   im = Render(bmp);
   CHECK_INT(im.width, 16);
   CHECK(Pixel(&im, 2, 2) == RGB(255, 0, 0));
   FreeImage(&im);

   // Etched lines and frames: shadow then highlight.
   HWND etched = CreateWindowExA(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 10, 100, 100, 2, parent,
                                 (HMENU) 507, instance, NULL);
   im = Render(etched);
   GetWindowRect(etched, &wr);
   CHECK_INT(wr.bottom - wr.top, 2);   // sized to its edge
   CHECK(Pixel(&im, 50, 0) == GetSysColor(COLOR_BTNSHADOW));
   CHECK(Pixel(&im, 50, 1) == GetSysColor(COLOR_BTNHIGHLIGHT));
   FreeImage(&im);
   HWND frame = CreateWindowExA(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME, 10, 110, 50, 30, parent,
                                (HMENU) 508, instance, NULL);
   im = Render(frame);
   CHECK(Pixel(&im, 0, 15) == GetSysColor(COLOR_BTNSHADOW));
   CHECK(Pixel(&im, 1, 15) == GetSysColor(COLOR_BTNHIGHLIGHT));
   FreeImage(&im);

   // SS_GRAYRECT with SS_SUNKEN | WS_BORDER (client.rc's IDC_SCROLL).
   HWND gray = CreateWindowExA(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_GRAYRECT | SS_SUNKEN | WS_BORDER, 70, 110, 50,
                               30, parent, (HMENU) 509, instance, NULL);
   im = Render(gray);
   CHECK(Pixel(&im, 0, 0) == GetSysColor(COLOR_WINDOWFRAME));   // the border
   FreeImage(&im);

   DestroyWindow(parent);
   return Report("ctl_static");
}

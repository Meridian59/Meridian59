/*
 * cc_animate.c:  The animation control (Wine's SysAnimate32) as the client's
 *   download dialog uses it: ACS_CENTER | ACS_TRANSPARENT | ACS_AUTOPLAY,
 *   Animate_Open of the client's AVI resource (download.avi, RLE8 through the
 *   layer's msvfw32 codec, read with mmio), ACN_START / ACN_STOP to the parent,
 *   seeking to single frames, playing by timer.  Renders frames by
 *   WM_PRINTCLIENT, checks they differ and are transparent where the first pixel's
 *   color is, and writes cc_animate.bmp (frames 0, 7, 14, 21 stacked).
 */

#include "cc_util.h"

static int started, stopped;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_COMMAND && LOWORD(wparam) == 9)
   {
      if (HIWORD(wparam) == ACN_START)
         started++;
      if (HIWORD(wparam) == ACN_STOP)
         stopped++;
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static void Render(HWND anim, Canvas32 *c, int y)
{
   POINT old;
   SetViewportOrgEx(c->dc, 0, y, &old);
   SendMessageA(anim, WM_PRINTCLIENT, (WPARAM) c->dc, PRF_CLIENT);
   SetViewportOrgEx(c->dc, old.x, old.y, NULL);
}

static int Differences(const Canvas32 *c, int y0, int y1, int height)
{
   int n = 0;
   for (int y = 0; y < height; y++)
      for (int x = 0; x < c->width; x++)
         if (CanvasPixel(c, x, y0 + y) != CanvasPixel(c, x, y1 + y))
            n++;
   return n;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 320, 100);
   const int width = 300, height = 60;
   HWND anim = CreateWindowExA(0, ANIMATE_CLASSA, "", WS_CHILD | WS_VISIBLE | ACS_CENTER | ACS_TRANSPARENT | ACS_AUTOPLAY,
                               10, 10, width, height, parent, (HMENU) 9, instance, NULL);
   CHECK(anim != NULL);
   if (anim == NULL)
      return CheckReport("cc_animate");

   // The AVI: 260x48, 22 frames, 'mrle' (RLE8).
   HRSRC res = FindResourceA(instance, MAKEINTRESOURCEA(170), "AVI");
   CHECK(res != NULL);
   CHECK(Animate_Open(anim, MAKEINTRESOURCEA(170)));
   PumpUntil(&started, 5000);
   CHECK_INT(started, 1);   // ACS_AUTOPLAY (notifications are posted)
   CHECK(Animate_Stop(anim));
   PumpUntil(&stopped, 5000);
   CHECK_INT(stopped, 1);

   // Frames by seeking (play from n to n: drawn at once, no notification).
   Canvas32 c = MakeCanvas(width, height * 4, RGB(255, 0, 255));
   const int frames[4] = {0, 7, 14, 21};
   for (int i = 0; i < 4; i++)
   {
      CHECK(Animate_Seek(anim, frames[i]));
      Render(anim, &c, height * i);
   }
   DumpBmp(&c, "cc_animate.bmp");
   Pump(20);
   CHECK_INT(started, 1);

   // Centered: 20 pixels each side of the 260 wide frame, 6 above and below
   // the 48 high one, untouched (still the canvas's magenta).
   RECT left = {0, 0, 20, height}, right = {280, 0, 300, height}, top = {20, 0, 280, 6};
   CHECK_INT(CountColor(&c, left, RGB(255, 0, 255), 0), 20 * height);
   CHECK_INT(CountColor(&c, right, RGB(255, 0, 255), 0), 20 * height);
   CHECK_INT(CountColor(&c, top, RGB(255, 0, 255), 0), 260 * 6);
   RECT frame = {20, 6, 280, 54};
   CHECK_INT(CountColor(&c, frame, RGB(255, 0, 255), 0), 0);
   // ACS_TRANSPARENT: the first pixel's color shows the parent's background
   // (WM_CTLCOLORSTATIC: button face) instead; the picture is the rest.
   int face = CountColor(&c, frame, GetSysColor(COLOR_BTNFACE), 0);
   printf("frame 0: %d of %d pixels background\n", face, 260 * 48);
   CHECK(face > 260 * 48 / 4 && face < 260 * 48);
   // The frames differ.
   for (int i = 1; i < 4; i++)
   {
      int d = Differences(&c, 0, height * i, height);
      printf("frame %d differs from frame 0 in %d pixels\n", frames[i], d);
      CHECK(d > 50);
   }

   // Playing: by timer, frames change on the window.
   started = stopped = 0;
   CHECK(Animate_Play(anim, 0, -1, 1));   // once through
   PumpUntil(&stopped, 22 * 67 + 10000);
   CHECK_INT(started, 1);
   CHECK_INT(stopped, 1);   // one loop, then it stops by itself
   FreeCanvas(&c);
   DestroyWindow(parent);
   return CheckReport("cc_animate");
}

/*
 * gfx_window.c:  Direct3D 9 presenting into part of a window that also has a
 *   menu bar and GDI drawing: an animated pattern of Clear rectangles, the frame
 *   count in the title.  Quits after M59_TEST_SECONDS (default 20).
 */

#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>

static IDirect3DDevice9 *device;
static int frames;
static const RECT view = {20, 40, 340, 280};   // client coordinates

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      SetBkMode(hdc, TRANSPARENT);
      TextOut(hdc, 20, 10, "GDI above the view", 18);
      TextOut(hdc, 20, 290, "GDI below the view", 18);
      EndPaint(hwnd, &ps);
      return 0;
   }
   case WM_TIMER:
   case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
   }
   return DefWindowProc(hwnd, msg, wparam, lparam);
}

static void Frame(HWND hwnd)
{
   int t = frames++;
   device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(20, 20, 60), 1.0f, 0);
   D3DRECT r[3];
   for (int i = 0; i < 3; i++)
   {
      int x = (t * (2 + i) + i * 90) % 280;
      r[i].x1 = x;
      r[i].y1 = 30 + i * 70;
      r[i].x2 = x + 40;
      r[i].y2 = 70 + i * 70;
   }
   device->Clear(1, &r[0], D3DCLEAR_TARGET, D3DCOLOR_XRGB(220, 40, 40), 1.0f, 0);
   device->Clear(1, &r[1], D3DCLEAR_TARGET, D3DCOLOR_XRGB(40, 220, 40), 1.0f, 0);
   device->Clear(1, &r[2], D3DCLEAR_TARGET, D3DCOLOR_XRGB(40, 40, 220), 1.0f, 0);
   RECT src = {0, 0, view.right - view.left, view.bottom - view.top};
   device->Present(&src, &view, NULL, NULL);
   if (t % 60 == 0)
   {
      char title[64];
      sprintf(title, "gfx_window: frame %d", t);
      SetWindowText(hwnd, title);
   }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   WNDCLASS wc = {0};
   wc.lpfnWndProc = WndProc;
   wc.hInstance = instance;
   wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
   wc.lpszClassName = "GfxWindow";
   RegisterClass(&wc);
   HMENU bar = CreateMenu();
   AppendMenu(bar, MF_STRING, 1, "&Menu");
   HWND hwnd = CreateWindow("GfxWindow", "gfx_window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 360, 340,
                            NULL, bar, instance, NULL);
   ShowWindow(hwnd, show);
   UpdateWindow(hwnd);

   IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
   D3DPRESENT_PARAMETERS pp = {0};
   pp.Windowed = TRUE;
   pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
   pp.BackBufferFormat = D3DFMT_X8R8G8B8;
   pp.BackBufferWidth = view.right - view.left;
   pp.BackBufferHeight = view.bottom - view.top;
   pp.hDeviceWindow = hwnd;
   HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp,
                                  &device);
   printf("CreateDevice: 0x%lx\n", (unsigned long) hr);
   fflush(stdout);
   if (FAILED(hr))
      return 1;

   const char *seconds = getenv("M59_TEST_SECONDS");
   SetTimer(hwnd, 1, (seconds ? atoi(seconds) : 20) * 1000, NULL);
   MSG msg;
   for (;;)
   {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
            goto done;
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      Frame(hwnd);
      Sleep(16);
   }
done:
   printf("frames: %d\n", frames);
   device->Release();
   d3d->Release();
   return 0;
}

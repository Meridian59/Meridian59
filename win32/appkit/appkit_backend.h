/*
 * appkit_backend.h:  What the AppKit backend's Objective-C++ files share (no
 *   Windows headers here: bridge.h).  Our coordinates have the primary screen's
 *   top-left at (0, 0), y down; AppKit's have its bottom-left, y up.
 */

#ifndef W32_APPKIT_BACKEND_H
#define W32_APPKIT_BACKEND_H

#include "appkit/bridge.h"
#include "appkit/vk.h"

#import <AppKit/AppKit.h>

@class M59View;

namespace w32::mac {

class GLView;

}  // namespace w32::mac

namespace w32::native {

class Top
{
public:
   void *hwnd = nullptr;           // the core's HWND
   NSWindow *window = nil;
   M59View *view = nil;            // the content view: shows pixels, takes input
   id delegate = nil;
   std::vector<uint8_t> pixels;    // the surface: BGRA, row 0 at the top
   int width = 0, height = 0;
   int x = 0, y = 0;               // surface position, our screen coordinates
   NSCursor *cursor = nil;
   bool cursor_hidden = false;
   bool resizable = false;
   mac::GLView *gl = nullptr;      // Direct3D frames over the surface (appkit/glsurface.mm)
};

}  // namespace w32::native

namespace w32::mac {

// AppKit screen coordinates <-> ours.
NSRect ToAppKit(int x, int y, int width, int height);
Point FromAppKit(NSPoint p);

// A VK_* code for a macOS virtual key code, or 0.
int KeyCodeToVk(unsigned short keycode);

// The GL view follows the surface's size; DestroyTop frees it.
void UpdateGLView(Top *top);
void DestroyGLView(Top *top);
// GDI drew r (surface coordinates): what it covers of the GL view shows GDI's,
// as on Windows, where the last to draw a pixel wins; the view hides until the
// next frame (a game drawing its map with GDI between Direct3D frames).
void GLViewDrawnOver(Top *top, const Rect &r);

}  // namespace w32::mac

#endif

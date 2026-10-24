/*
 * gfx.h:  Hooks into Direct3D 9 (gfx/) for the backend and the test programs.
 *   Self-contained: tests include it as "../gfx/gfx.h".
 */

#ifndef W32_GFX_GFX_H
#define W32_GFX_GFX_H

#include <windows.h>

namespace w32 {
class GLSurface;
}

namespace w32::gfx {

// Replaces w32::GetGLSurface for CreateDevice and Present, e.g. with an
// offscreen surface in a test; nullptr restores it.
__attribute__((visibility("default"))) void SetSurfaceProvider(GLSurface *(*provider)(HWND hwnd));

// Devices keep their GL state from their first call after a Present to the
// next Present.  The backend calls this if, in between, it makes another
// context current or changes state in a device's context, so each device makes
// its context current and rebinds everything on its next call.
__attribute__((visibility("default"))) void InvalidateGLState();

}  // namespace w32::gfx

#endif

/*
 * glsurface.h:  OpenGL for a window: what Direct3D 9 (gfx/) renders with.  The
 *   backend supplies the context and puts frames on screen over the window's
 *   client area.
 */

#ifndef W32_CORE_GLSURFACE_H
#define W32_CORE_GLSURFACE_H

#include "core/win32.h"

namespace w32 {

class GLSurface
{
public:
   virtual ~GLSurface() = default;

   // Makes the surface's desktop OpenGL 3.3 core context current on this thread;
   // the device renders into its own framebuffer objects with it.
   virtual bool MakeCurrent() = 0;

   // Shows texture (GL_TEXTURE_2D, width x height, row 0 at the top) in dest, in
   // client coordinates of the window; the rest of the client area keeps what
   // GDI drew there.  Returns once the frame is queued, not shown.
   virtual void Present(unsigned texture, int width, int height, const RECT &dest) = 0;

   // Address of an OpenGL function, for the loader.
   virtual void *GetProcAddress(const char *name) = 0;
};

// The window's surface, made on first use; nullptr if the window has none (not
// shown yet, destroyed) or OpenGL 3.3 core is unavailable.
GLSurface *GetGLSurface(HWND hwnd);

}  // namespace w32

#endif

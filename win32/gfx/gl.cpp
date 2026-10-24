/*
 * gl.cpp:  Loads the OpenGL functions of gl.h from a GLSurface.
 */

#include "core/win32.h"
#include "core/glsurface.h"
#include "gfx/gl.h"

#include <cstdio>

namespace w32::gfx {

bool GL::Load(GLSurface *surface)
{
#define W32_GL_LOAD(ret, name, params) \
   name = reinterpret_cast<ret (*) params>(surface->GetProcAddress("gl" #name)); \
   if (name == nullptr) \
   { \
      fprintf(stderr, "d3d9: OpenGL function gl%s unavailable\n", #name); \
      return false; \
   }
   W32_GL_FUNCTIONS(W32_GL_LOAD)
#undef W32_GL_LOAD
   return true;
}

}  // namespace w32::gfx

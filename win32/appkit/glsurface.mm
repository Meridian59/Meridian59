/*
 * glsurface.mm:  OpenGL for Direct3D on AppKit.  A top-level window that renders
 *   gets an NSOpenGLContext (4.1 core, which runs the device's 3.3 shaders) on a
 *   subview over its surface, placed where frames are presented and transparent
 *   to input.  Devices render into their own framebuffer objects with it; a
 *   Present draws the frame into the subview and swaps at once.
 */

#define GL_SILENCE_DEPRECATION
#include "appkit/appkit_backend.h"

#import <OpenGL/gl3.h>

#include <cstdio>
#include <dlfcn.h>

// Input goes to the window's view below.
@interface M59GLView : NSView
@end

@implementation M59GLView
- (NSView *)hitTest:(NSPoint)point
{
   return nil;
}
- (BOOL)isOpaque
{
   return YES;
}
@end

namespace w32::mac {

class GLView
{
public:
   M59GLView *view = nil;
   NSOpenGLContext *context = nil;
   bool failed = false;
   GLuint program = 0, vao = 0;
   Rect placed;
};

namespace {

const char *const vertex_shader = R"(#version 330 core
out vec2 uv;
void main()
{
   vec2 p = vec2(gl_VertexID & 1, gl_VertexID >> 1);
   uv = vec2(p.x, 1.0 - p.y);
   gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char *const fragment_shader = R"(#version 330 core
in vec2 uv;
out vec4 color;
uniform sampler2D frame;
void main()
{
   color = vec4(texture(frame, uv).rgb, 1.0);
}
)";

GLuint Compile(GLenum type, const char *source)
{
   GLuint s = glCreateShader(type);
   glShaderSource(s, 1, &source, nullptr);
   glCompileShader(s);
   GLint ok = 0;
   glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
   if (!ok)
   {
      char log[512];
      glGetShaderInfoLog(s, sizeof(log), nullptr, log);
      fprintf(stderr, "m59win32: GL shader: %s\n", log);
   }
   return s;
}

// The view, made on first use; nullptr if OpenGL 4.1 core is unavailable.
GLView *View(Top *top)
{
   if (top->gl == nullptr)
   {
      auto *v = new GLView;
      top->gl = v;
      NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
                                              NSOpenGLPFAColorSize,     24,
                                              NSOpenGLPFAAlphaSize,     8,
                                              NSOpenGLPFADoubleBuffer,  NSOpenGLPFAAllowOfflineRenderers,
                                              0};
      NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
      if (format == nil)
      {
         fprintf(stderr, "m59win32: no OpenGL 4.1 core pixel format\n");
         v->failed = true;
         return nullptr;
      }
      v->context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
      [format release];
      GLint interval = 0;
      [v->context setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
      v->view = [[M59GLView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
      v->view.wantsBestResolutionOpenGLSurface = NO;
      v->view.hidden = YES;
      [(NSView *) top->view addSubview:v->view];
      v->context.view = v->view;
   }
   return top->gl->failed ? nullptr : top->gl;
}

}  // namespace

bool GLMakeCurrent(Top *top)
{
   GLView *v = View(top);
   if (v == nullptr)
      return false;
   [v->context makeCurrentContext];
   return true;
}

void GLViewDrawnOver(Top *top, const Rect &r)
{
   GLView *v = top->gl;
   if (v == nullptr || v->view == nil || v->view.hidden)
      return;
   const Rect &p = v->placed;
   if (r.left >= p.right || r.top >= p.bottom || r.right <= p.left || r.bottom <= p.top)
      return;
   v->view.hidden = YES;
}

void GLPresent(Top *top, unsigned texture, const Rect &r)
{
   GLView *v = View(top);
   if (v == nullptr)
      return;
   if (r.left != v->placed.left || r.top != v->placed.top || r.right != v->placed.right ||
       r.bottom != v->placed.bottom)
   {
      // The content view is flipped: the surface's numbers.
      v->placed = r;
      v->view.frame = NSMakeRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
      [v->context update];
   }
   v->view.hidden = NO;
   [v->context makeCurrentContext];
   if (v->program == 0)
   {
      v->program = glCreateProgram();
      glAttachShader(v->program, Compile(GL_VERTEX_SHADER, vertex_shader));
      glAttachShader(v->program, Compile(GL_FRAGMENT_SHADER, fragment_shader));
      glLinkProgram(v->program);
      glGenVertexArrays(1, &v->vao);
   }
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   NSRect pixels = [v->view convertRectToBacking:v->view.bounds];
   glViewport(0, 0, (GLsizei) pixels.size.width, (GLsizei) pixels.size.height);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_BLEND);
   glDisable(GL_SCISSOR_TEST);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glUseProgram(v->program);
   glBindVertexArray(v->vao);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture);
   glUniform1i(glGetUniformLocation(v->program, "frame"), 0);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   [v->context flushBuffer];
   // The device's state in this context is no longer what it set.
   OnGLStateChanged();
}

void *GLProcAddress(const char *name)
{
   static void *gl = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_NOW | RTLD_LOCAL);
   return gl ? dlsym(gl, name) : nullptr;
}

void UpdateGLView(Top *top)
{
   if (top->gl && top->gl->context)
      [top->gl->context update];
}

void DestroyGLView(Top *top)
{
   if (GLView *v = top->gl)
   {
      if (v->context)
      {
         [v->context clearDrawable];
         [v->context release];
      }
      [v->view removeFromSuperview];
      [v->view release];
      delete v;
      top->gl = nullptr;
   }
}

}  // namespace w32::mac

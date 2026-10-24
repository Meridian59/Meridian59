/*
 * glsurface.cpp:  OpenGL for Direct3D on GTK 4.  Each top-level window that
 *   renders gets a GtkGLArea over its GDI surface, placed where frames are
 *   presented and transparent to input.  Devices render into their own
 *   framebuffer objects with the area's context; the area draws the latest
 *   presented texture when GTK paints it.
 */

#include "gtk/gtk_backend.h"

#include "core/glsurface.h"
#include "core/window.h"
#include "gfx/gfx.h"

#include <epoxy/gl.h>

#include <dlfcn.h>
#include <map>

namespace w32::native {

class GLView
{
public:
   Top *top;
   GtkWidget *area = nullptr;
   bool failed = false;
   GLuint texture = 0;                 // the latest presented frame, in the area's context
   int texture_width = 0, texture_height = 0;
   GLuint program = 0, vao = 0;
   gulong after_paint = 0;
};

}  // namespace w32::native

namespace w32 {

namespace {

using native::GLView;
using native::Top;

void *(*egl_get_proc_address)(const char *) = nullptr;

const char *const vertex_shader = R"(#version 330 core
out vec2 uv;
void main()
{
   // A strip over the viewport; row 0 of the texture at the top.
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

GdkGLContext *OnCreateContext(GtkGLArea *area, gpointer)
{
   GdkSurface *surface = gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(area)));
   GError *error = nullptr;
   GdkGLContext *context = gdk_surface_create_gl_context(surface, &error);
   if (context == nullptr)
   {
      fprintf(stderr, "m59win32: no GL context: %s\n", error ? error->message : "?");
      g_clear_error(&error);
      return nullptr;
   }
   // Desktop OpenGL 3.3 core, what the Direct3D device is written for.
   gdk_gl_context_set_allowed_apis(context, GDK_GL_API_GL);
   gdk_gl_context_set_required_version(context, 3, 3);
   gdk_gl_context_set_forward_compatible(context, TRUE);
   if (!gdk_gl_context_realize(context, &error))
   {
      fprintf(stderr, "m59win32: GL 3.3 core unavailable: %s\n", error ? error->message : "?");
      g_clear_error(&error);
      g_object_unref(context);
      return nullptr;
   }
   return context;
}

gboolean OnRender(GtkGLArea *area, GdkGLContext *, gpointer data)
{
   auto *v = static_cast<GLView *>(data);
   int scale = gtk_widget_get_scale_factor(GTK_WIDGET(area));
   glViewport(0, 0, gtk_widget_get_width(GTK_WIDGET(area)) * scale, gtk_widget_get_height(GTK_WIDGET(area)) * scale);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_BLEND);
   glDisable(GL_SCISSOR_TEST);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   if (v->texture == 0)
   {
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);
   }
   else
   {
      if (v->program == 0)
      {
         v->program = glCreateProgram();
         glAttachShader(v->program, Compile(GL_VERTEX_SHADER, vertex_shader));
         glAttachShader(v->program, Compile(GL_FRAGMENT_SHADER, fragment_shader));
         glLinkProgram(v->program);
         glGenVertexArrays(1, &v->vao);
      }
      glUseProgram(v->program);
      glBindVertexArray(v->vao);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, v->texture);
      glUniform1i(glGetUniformLocation(v->program, "frame"), 0);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   }
   // The device's state in this context is no longer what it set.
   gfx::InvalidateGLState();
   return TRUE;
}

// GTK's renderer makes its own context current when it paints a frame.
void OnAfterPaint(GdkFrameClock *, gpointer)
{
   gfx::InvalidateGLState();
}

class AreaSurface : public GLSurface
{
public:
   explicit AreaSurface(HWND hwnd) : hwnd_(hwnd) {}

   bool MakeCurrent() override
   {
      GLView *v = View();
      if (v == nullptr)
         return false;
      gtk_gl_area_make_current(GTK_GL_AREA(v->area));
      if (gtk_gl_area_get_error(GTK_GL_AREA(v->area)) != nullptr)
      {
         v->failed = true;
         return false;
      }
      return true;
   }

   void Present(unsigned texture, int width, int height, const RECT &dest) override
   {
      GLView *v = View();
      if (v == nullptr)
         return;
      // dest is in hwnd's client coordinates; the area sits on the surface.
      RECT c = ClientRectInTop(hwnd_);
      int x = c.left + dest.left, y = c.top + dest.top;
      int w = dest.right - dest.left, h = dest.bottom - dest.top;
      if (gtk_widget_get_margin_start(v->area) != x || gtk_widget_get_margin_top(v->area) != y)
      {
         gtk_widget_set_margin_start(v->area, x);
         gtk_widget_set_margin_top(v->area, y);
      }
      int rw, rh;
      gtk_widget_get_size_request(v->area, &rw, &rh);
      if (rw != w || rh != h)
         gtk_widget_set_size_request(v->area, w, h);
      v->texture = texture;
      v->texture_width = width;
      v->texture_height = height;
      gtk_widget_set_visible(v->area, TRUE);
      gtk_gl_area_queue_render(GTK_GL_AREA(v->area));
   }

   void *GetProcAddress(const char *name) override
   {
      // GDK's contexts are EGL's (Wayland, and X11 since GTK 4.14), with every
      // function available through eglGetProcAddress; epoxy loaded libEGL locally.
      if (egl_get_proc_address == nullptr)
      {
         void *egl = dlopen("libEGL.so.1", RTLD_NOW | RTLD_NOLOAD);
         if (egl == nullptr)
            egl = dlopen("libEGL.so.1", RTLD_NOW);
         if (egl)
            egl_get_proc_address = (void *(*)(const char *)) dlsym(egl, "eglGetProcAddress");
      }
      void *p = egl_get_proc_address ? egl_get_proc_address(name) : nullptr;
      if (p == nullptr)
         p = dlsym(RTLD_DEFAULT, name);
      return p;
   }

   HWND hwnd() const { return hwnd_; }

private:
   // The view of the window's top-level, made (and realized, for its context) on
   // first use; nullptr if it has none or GL failed.
   GLView *View()
   {
      Window *w = GetWindowObject(TopLevelOf(hwnd_));
      if (w == nullptr || w->top == nullptr)
         return nullptr;
      Top *top = w->top;
      if (top->gl == nullptr)
      {
         auto *v = new GLView{top};
         v->area = gtk_gl_area_new();
         gtk_gl_area_set_auto_render(GTK_GL_AREA(v->area), FALSE);
         gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(v->area), FALSE);
         gtk_widget_set_can_target(v->area, FALSE);
         gtk_widget_set_focusable(v->area, FALSE);
         gtk_widget_set_halign(v->area, GTK_ALIGN_START);
         gtk_widget_set_valign(v->area, GTK_ALIGN_START);
         gtk_widget_set_visible(v->area, FALSE);
         g_signal_connect(v->area, "create-context", G_CALLBACK(OnCreateContext), v);
         g_signal_connect(v->area, "render", G_CALLBACK(OnRender), v);
         gtk_overlay_add_overlay(GTK_OVERLAY(top->overlay), v->area);
         top->gl = v;
      }
      GLView *v = top->gl;
      if (v->failed)
         return nullptr;
      if (!gtk_widget_get_realized(v->area))
      {
         // The context comes with realizing, which needs the window's surface.
         gtk_widget_realize(top->window);
         gtk_widget_realize(v->area);
         if (GdkFrameClock *clock = gtk_widget_get_frame_clock(top->window); clock && v->after_paint == 0)
            v->after_paint = g_signal_connect(clock, "after-paint", G_CALLBACK(OnAfterPaint), nullptr);
      }
      if (gtk_gl_area_get_context(GTK_GL_AREA(v->area)) == nullptr)
      {
         v->failed = true;
         return nullptr;
      }
      return v;
   }

   HWND hwnd_;
};

std::map<HWND, AreaSurface *> &Surfaces()
{
   static std::map<HWND, AreaSurface *> surfaces;
   return surfaces;
}

}  // namespace

GLSurface *GetGLSurface(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   Window *t = GetWindowObject(TopLevelOf(hwnd));
   if (t == nullptr || t->top == nullptr)
      return nullptr;
   auto &surfaces = Surfaces();
   auto it = surfaces.find(hwnd);
   if (it != surfaces.end())
      return it->second;
   auto *s = new AreaSurface(hwnd);
   surfaces[hwnd] = s;
   return s;
}

}  // namespace w32

namespace w32::native {

void GLViewDrawnOver(Top *top, const cairo_rectangle_int_t &r)
{
   GLView *v = top->gl;
   if (v == nullptr || !gtk_widget_get_visible(v->area))
      return;
   int x = gtk_widget_get_margin_start(v->area), y = gtk_widget_get_margin_top(v->area), w, h;
   gtk_widget_get_size_request(v->area, &w, &h);
   if (r.x >= x + w || r.y >= y + h || r.x + r.width <= x || r.y + r.height <= y)
      return;
   gtk_widget_set_visible(v->area, FALSE);
}

void DestroyGLView(Top *top)
{
   // The window's surfaces go: devices on them are lost.
   auto &surfaces = Surfaces();
   for (auto it = surfaces.begin(); it != surfaces.end();)
   {
      if (TopLevelOf(it->second->hwnd()) == top->hwnd || !IsWindow(it->second->hwnd()))
      {
         delete it->second;
         it = surfaces.erase(it);
      }
      else
         ++it;
   }
   if (top->gl)
   {
      if (top->gl->after_paint)
         if (GdkFrameClock *clock = gtk_widget_get_frame_clock(top->window))
            g_signal_handler_disconnect(clock, top->gl->after_paint);
      delete top->gl;
      top->gl = nullptr;
   }
}

}  // namespace w32::native

/*
 * gfx_d3d9.c:  Direct3D 9 on OpenGL (win32/gfx/), rendered offscreen and read
 *   back.  The window's GLSurface is a test one on EGL (desktop GL 3.3 core,
 *   surfaceless or a pbuffer), handed to the device through
 *   w32::gfx::SetSurfaceProvider; Present records the texture it is given and
 *   the test reads its pixels.  Linux only (EGL); elsewhere it reports a skip.
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

#include "core/glsurface.h"
#include "gfx/gfx.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifndef __APPLE__
#include <dlfcn.h>
#endif


namespace {

int gFailures = 0;

#define CHECK(cond)                                                           \
   do                                                                         \
   {                                                                          \
      if (!(cond))                                                            \
      {                                                                       \
         fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);      \
         gFailures++;                                                         \
      }                                                                       \
   } while (0)

/************************************************************************/
// EGL, loaded at run time

#ifndef __APPLE__

typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLSurface;
typedef void *EGLDeviceEXT;
typedef int32_t EGLint;
typedef unsigned EGLBoolean;
typedef unsigned EGLenum;

constexpr EGLint EGL_NONE = 0x3038;
constexpr EGLint EGL_ALPHA_SIZE = 0x3021;
constexpr EGLint EGL_BLUE_SIZE = 0x3022;
constexpr EGLint EGL_GREEN_SIZE = 0x3023;
constexpr EGLint EGL_RED_SIZE = 0x3024;
constexpr EGLint EGL_SURFACE_TYPE = 0x3033;
constexpr EGLint EGL_RENDERABLE_TYPE = 0x3040;
constexpr EGLint EGL_HEIGHT = 0x3056;
constexpr EGLint EGL_WIDTH = 0x3057;
constexpr EGLint EGL_PBUFFER_BIT = 0x0001;
constexpr EGLint EGL_OPENGL_BIT = 0x0008;
constexpr EGLenum EGL_OPENGL_API = 0x30A2;
constexpr EGLint EGL_CONTEXT_MAJOR_VERSION = 0x3098;
constexpr EGLint EGL_CONTEXT_MINOR_VERSION = 0x30FB;
constexpr EGLint EGL_CONTEXT_OPENGL_PROFILE_MASK = 0x30FD;
constexpr EGLint EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT = 0x0001;
constexpr EGLenum EGL_PLATFORM_DEVICE_EXT = 0x313F;

struct Egl
{
   void *(*GetProcAddress)(const char *);
   EGLDisplay (*GetDisplay)(void *);
   EGLBoolean (*Initialize)(EGLDisplay, EGLint *, EGLint *);
   EGLBoolean (*Terminate)(EGLDisplay);
   EGLBoolean (*BindAPI)(EGLenum);
   EGLBoolean (*ChooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
   EGLContext (*CreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
   EGLBoolean (*DestroyContext)(EGLDisplay, EGLContext);
   EGLSurface (*CreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint *);
   EGLBoolean (*DestroySurface)(EGLDisplay, EGLSurface);
   EGLBoolean (*MakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
   EGLBoolean (*QueryDevicesEXT)(EGLint, EGLDeviceEXT *, EGLint *);
   EGLDisplay (*GetPlatformDisplayEXT)(EGLenum, void *, const EGLint *);
} egl;

// The few GL calls the test makes itself, to read presented frames back.
constexpr unsigned GL_TEXTURE_2D = 0x0DE1;
constexpr unsigned GL_BGRA = 0x80E1;
constexpr unsigned GL_UNSIGNED_BYTE = 0x1401;
constexpr unsigned GL_PACK_ALIGNMENT = 0x0D05;
constexpr unsigned GL_RENDERER = 0x1F01;
constexpr unsigned GL_VERSION = 0x1F02;

struct
{
   void (*BindTexture)(unsigned, unsigned);
   void (*GetTexImage)(unsigned, int, unsigned, unsigned, void *);
   void (*PixelStorei)(unsigned, int);
   const unsigned char *(*GetString)(unsigned);
   unsigned (*GetError)(void);
} gl;

template <class F>
bool Sym(F &f, void *lib, const char *name)
{
   f = reinterpret_cast<F>(dlsym(lib, name));
   return f != nullptr;
}

class EglSurface : public w32::GLSurface
{
public:
   bool MakeCurrent() override { return egl.MakeCurrent(display, surface, surface, context) != 0; }
   void Present(unsigned t, int w, int h, const RECT &d) override
   {
      texture = t;
      width = w;
      height = h;
      dest = d;
      presents++;
   }
   void *GetProcAddress(const char *name) override { return egl.GetProcAddress(name); }

   bool Create();

   EGLDisplay display = nullptr;
   EGLContext context = nullptr;
   EGLSurface surface = nullptr;   // a pbuffer, or none if surfaceless works

   unsigned texture = 0;
   int      width = 0, height = 0;
   RECT     dest{};
   int      presents = 0;
};

bool TryDisplay(EglSurface *s, EGLDisplay display)
{
   if (display == nullptr || !egl.Initialize(display, nullptr, nullptr))
      return false;
   const EGLint configAttribs[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                                    EGL_NONE };
   EGLConfig config = nullptr;
   EGLint n = 0;
   if (!egl.ChooseConfig(display, configAttribs, &config, 1, &n) || n < 1 || !egl.BindAPI(EGL_OPENGL_API))
      return false;
   const EGLint contextAttribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
                                     EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                     EGL_NONE };
   EGLContext context = egl.CreateContext(display, config, nullptr, contextAttribs);
   if (context == nullptr)
      return false;
   EGLSurface surface = nullptr;
   if (!egl.MakeCurrent(display, nullptr, nullptr, context))
   {
      const EGLint pbufferAttribs[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
      surface = egl.CreatePbufferSurface(display, config, pbufferAttribs);
      if (surface == nullptr || !egl.MakeCurrent(display, surface, surface, context))
      {
         egl.DestroyContext(display, context);
         return false;
      }
   }
   s->display = display;
   s->context = context;
   s->surface = surface;
   return true;
}

bool EglSurface::Create()
{
   void *lib = dlopen("libEGL.so.1", RTLD_NOW);
   if (lib == nullptr)
      return false;
   if (!Sym(egl.GetProcAddress, lib, "eglGetProcAddress") || !Sym(egl.GetDisplay, lib, "eglGetDisplay") ||
       !Sym(egl.Initialize, lib, "eglInitialize") || !Sym(egl.Terminate, lib, "eglTerminate") ||
       !Sym(egl.BindAPI, lib, "eglBindAPI") || !Sym(egl.ChooseConfig, lib, "eglChooseConfig") ||
       !Sym(egl.CreateContext, lib, "eglCreateContext") || !Sym(egl.DestroyContext, lib, "eglDestroyContext") ||
       !Sym(egl.CreatePbufferSurface, lib, "eglCreatePbufferSurface") ||
       !Sym(egl.DestroySurface, lib, "eglDestroySurface") || !Sym(egl.MakeCurrent, lib, "eglMakeCurrent"))
      return false;
   egl.QueryDevicesEXT = reinterpret_cast<decltype(egl.QueryDevicesEXT)>(egl.GetProcAddress("eglQueryDevicesEXT"));
   egl.GetPlatformDisplayEXT =
      reinterpret_cast<decltype(egl.GetPlatformDisplayEXT)>(egl.GetProcAddress("eglGetPlatformDisplayEXT"));

   // A GPU without a window system first; the default display otherwise.
   bool ok = false;
   if (egl.QueryDevicesEXT && egl.GetPlatformDisplayEXT)
   {
      EGLDeviceEXT devices[8];
      EGLint count = 0;
      if (egl.QueryDevicesEXT(8, devices, &count))
         for (EGLint i = 0; i < count && !ok; i++)
            ok = TryDisplay(this, egl.GetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[i], nullptr));
   }
   if (!ok)
      ok = TryDisplay(this, egl.GetDisplay(nullptr));
   if (!ok)
      return false;

   gl.BindTexture = reinterpret_cast<decltype(gl.BindTexture)>(egl.GetProcAddress("glBindTexture"));
   gl.GetTexImage = reinterpret_cast<decltype(gl.GetTexImage)>(egl.GetProcAddress("glGetTexImage"));
   gl.PixelStorei = reinterpret_cast<decltype(gl.PixelStorei)>(egl.GetProcAddress("glPixelStorei"));
   gl.GetString = reinterpret_cast<decltype(gl.GetString)>(egl.GetProcAddress("glGetString"));
   gl.GetError = reinterpret_cast<decltype(gl.GetError)>(egl.GetProcAddress("glGetError"));
   return gl.BindTexture && gl.GetTexImage && gl.PixelStorei && gl.GetString && gl.GetError;
}

EglSurface gSurface;
w32::GLSurface *gProvided = &gSurface;

w32::GLSurface *Provide(HWND)
{
   return gProvided;
}

/************************************************************************/
// Frames

const HWND WINDOW = reinterpret_cast<HWND>(0x1234);
constexpr int SIZE = 64;

// The last presented frame, row 0 at the top, as 0xAARRGGBB.
struct Frame
{
   int width = 0, height = 0;
   std::vector<uint32_t> pixels;

   uint32_t At(int x, int y) const { return pixels[(size_t) y * width + x]; }
};

Frame ReadPresented()
{
   Frame f;
   f.width = gSurface.width;
   f.height = gSurface.height;
   f.pixels.resize((size_t) f.width * f.height);
   gSurface.MakeCurrent();
   gl.BindTexture(GL_TEXTURE_2D, gSurface.texture);
   gl.PixelStorei(GL_PACK_ALIGNMENT, 4);
   gl.GetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, f.pixels.data());
   CHECK(gl.GetError() == 0);
   return f;
}

bool Near(uint32_t a, uint32_t b, int tolerance)
{
   for (int shift = 0; shift < 24; shift += 8)   // RGB
      if (std::abs((int) ((a >> shift) & 0xff) - (int) ((b >> shift) & 0xff)) > tolerance)
         return false;
   return true;
}

void ExpectPixel(const Frame &f, int x, int y, uint32_t want, int tolerance, int line)
{
   uint32_t got = f.At(x, y);
   if (!Near(got, want, tolerance))
   {
      fprintf(stderr, "FAIL line %d: pixel (%d, %d) is %08x, want %08x\n", line, x, y, got, want);
      gFailures++;
   }
}

#define EXPECT_PIXEL(f, x, y, want) ExpectPixel(f, x, y, want, 2, __LINE__)

/************************************************************************/
// Geometry, laid out as the client's d3dcache: one stream per element.

constexpr UINT CAPACITY = 50000;   // the client's TEMP_CACHE_MAX

struct XYZ { float x, y, z; };
struct ST { float s, t; };

struct Quad
{
   float x0, y0, x1, y1, z;   // clip-space corners: (x0, y0) top left
   DWORD color;
   ST    uv0 = { 0, 0 }, uv1 = { 1, 1 };
   UINT  vbase = 0;           // first vertex
   UINT  ibase = 0;           // first index
};

struct Scene
{
   IDirect3DDevice9 *dev = nullptr;
   IDirect3DVertexBuffer9 *xyz = nullptr, *bgra = nullptr, *st[2] = {};
   IDirect3DIndexBuffer9 *ib = nullptr;
   IDirect3DVertexDeclaration9 *decl1 = nullptr, *decl2 = nullptr;

   bool Create(IDirect3D9 *d3d, D3DMULTISAMPLE_TYPE samples);
   void Release();
   // Fills every quad in one D3DLOCK_DISCARD lock of the whole buffers, as the
   // cache does, then draws each.
   void Draw(const std::vector<Quad> &quads, bool twoStages = false);
};

bool Scene::Create(IDirect3D9 *d3d, D3DMULTISAMPLE_TYPE samples)
{
   D3DPRESENT_PARAMETERS pp = {};
   pp.Windowed = TRUE;
   pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
   pp.BackBufferWidth = SIZE;
   pp.BackBufferHeight = SIZE;
   pp.BackBufferFormat = D3DFMT_A8R8G8B8;
   pp.BackBufferCount = 1;
   pp.EnableAutoDepthStencil = TRUE;
   pp.AutoDepthStencilFormat = D3DFMT_D24S8;
   pp.MultiSampleType = samples;
   if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, WINDOW, D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                &pp, &dev)))
      return false;

   CHECK(dev->CreateVertexBuffer(CAPACITY * sizeof(XYZ), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ,
                                 D3DPOOL_DEFAULT, &xyz, nullptr) == D3D_OK);
   CHECK(dev->CreateVertexBuffer(CAPACITY * sizeof(DWORD), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_DIFFUSE,
                                 D3DPOOL_DEFAULT, &bgra, nullptr) == D3D_OK);
   for (auto &b : st)
      CHECK(dev->CreateVertexBuffer(CAPACITY * sizeof(ST), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_TEX1,
                                    D3DPOOL_DEFAULT, &b, nullptr) == D3D_OK);
   CHECK(dev->CreateIndexBuffer(CAPACITY * sizeof(WORD), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                D3DPOOL_DEFAULT, &ib, nullptr) == D3D_OK);

   static const D3DVERTEXELEMENT9 tex1[] = {
      { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
      { 2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      D3DDECL_END() };
   static const D3DVERTEXELEMENT9 tex2[] = {
      { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
      { 2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      { 3, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
      D3DDECL_END() };
   CHECK(dev->CreateVertexDeclaration(tex1, &decl1) == D3D_OK);
   CHECK(dev->CreateVertexDeclaration(tex2, &decl2) == D3D_OK);

   dev->SetRenderState(D3DRS_LIGHTING, FALSE);
   dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
   dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
   dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
   dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
   dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
   dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
   dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
   dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
   dev->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
   dev->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
   return true;
}

void Scene::Release()
{
   for (IUnknown *u : std::initializer_list<IUnknown *>{ xyz, bgra, st[0], st[1], ib, decl1, decl2 })
      if (u)
         u->Release();
}

void Scene::Draw(const std::vector<Quad> &quads, bool twoStages)
{
   XYZ *p;
   DWORD *c;
   ST *t0, *t1;
   WORD *idx;
   CHECK(xyz->Lock(0, CAPACITY * sizeof(XYZ), (void **) &p, D3DLOCK_DISCARD) == D3D_OK);
   CHECK(bgra->Lock(0, CAPACITY * sizeof(DWORD), (void **) &c, D3DLOCK_DISCARD) == D3D_OK);
   CHECK(st[0]->Lock(0, CAPACITY * sizeof(ST), (void **) &t0, D3DLOCK_DISCARD) == D3D_OK);
   CHECK(st[1]->Lock(0, CAPACITY * sizeof(ST), (void **) &t1, D3DLOCK_DISCARD) == D3D_OK);
   CHECK(ib->Lock(0, CAPACITY * sizeof(WORD), (void **) &idx, D3DLOCK_DISCARD) == D3D_OK);
   for (const Quad &q : quads)
   {
      // Strip: top left, bottom left, top right, bottom right.
      XYZ corners[4] = { { q.x0, q.y0, q.z }, { q.x0, q.y1, q.z }, { q.x1, q.y0, q.z }, { q.x1, q.y1, q.z } };
      ST uv[4] = { q.uv0, { q.uv0.s, q.uv1.t }, { q.uv1.s, q.uv0.t }, q.uv1 };
      for (int i = 0; i < 4; i++)
      {
         p[q.vbase + i] = corners[i];
         c[q.vbase + i] = q.color;
         t0[q.vbase + i] = uv[i];
         t1[q.vbase + i] = uv[i];
         idx[q.ibase + i] = (WORD) i;   // relative to the base vertex
      }
   }
   xyz->Unlock();
   bgra->Unlock();
   st[0]->Unlock();
   st[1]->Unlock();
   ib->Unlock();

   dev->SetVertexShader(nullptr);
   dev->SetVertexDeclaration(twoStages ? decl2 : decl1);
   dev->SetStreamSource(0, xyz, 0, sizeof(XYZ));
   dev->SetStreamSource(1, bgra, 0, sizeof(DWORD));
   dev->SetStreamSource(2, st[0], 0, sizeof(ST));
   dev->SetStreamSource(3, st[1], 0, sizeof(ST));
   dev->SetIndices(ib);
   for (const Quad &q : quads)
      CHECK(dev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, q.vbase, q.ibase, 4, q.ibase, 2) == D3D_OK);
}

IDirect3DTexture9 *MakeTexture(IDirect3DDevice9 *dev, int w, int h, const std::vector<DWORD> &texels,
                               D3DPOOL pool = D3DPOOL_MANAGED)
{
   IDirect3DTexture9 *t = nullptr;
   CHECK(dev->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, pool, &t, nullptr) == D3D_OK);
   D3DLOCKED_RECT r;
   CHECK(t->LockRect(0, &r, nullptr, 0) == D3D_OK);
   for (int y = 0; y < h; y++)
      memcpy((BYTE *) r.pBits + y * r.Pitch, &texels[y * w], w * 4);
   CHECK(t->UnlockRect(0) == D3D_OK);
   return t;
}

void Present(IDirect3DDevice9 *dev, const RECT *src = nullptr, const RECT *dst = nullptr)
{
   int before = gSurface.presents;
   CHECK(dev->Present(src, dst, nullptr, nullptr) == D3D_OK);
   CHECK(gSurface.presents == before + 1);
}

/************************************************************************/
// Tests

constexpr DWORD RED = 0xffff0000, GREEN = 0xff00ff00, BLUE = 0xff0000ff, WHITE = 0xffffffff;
constexpr DWORD GRAY = 0xff404040, MAGENTA = 0xffff00ff, YELLOW = 0xffffff00, CYAN = 0xff00ffff;

const Quad FULL = { -1, 1, 1, -1, 0.5f, WHITE };

// A textured quad with alpha test: orientation (row 0 at the top, v = 0 at the
// top of the texture), point sampling, discard.
void TestTexturedAlphaTest(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   IDirect3DTexture9 *tex = MakeTexture(dev, 2, 2, { RED, 0x0000ff00, GREEN, BLUE });
   dev->SetTexture(0, tex);
   dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
   dev->SetRenderState(D3DRS_ALPHAREF, 1);
   dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

   CHECK(dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0) == D3D_OK);
   CHECK(dev->BeginScene() == D3D_OK);
   s.Draw({ FULL });
   CHECK(dev->EndScene() == D3D_OK);
   Present(dev);

   CHECK(gSurface.width == SIZE && gSurface.height == SIZE);
   CHECK(gSurface.dest.left == 0 && gSurface.dest.top == 0 && gSurface.dest.right == SIZE &&
         gSurface.dest.bottom == SIZE);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 16, 16, RED);
   EXPECT_PIXEL(f, 48, 16, GRAY);   // alpha 0: discarded
   EXPECT_PIXEL(f, 16, 48, GREEN);
   EXPECT_PIXEL(f, 48, 48, BLUE);
   EXPECT_PIXEL(f, 0, 0, RED);
   EXPECT_PIXEL(f, 63, 63, BLUE);

   // Linear W fog: a projection whose w is the view depth, the quad at w = 2,
   // fog from 0 to 4: halfway to white.
   D3DMATRIX proj = {};
   proj._11 = proj._22 = 1.0f;
   proj._33 = 0.5f;
   proj._34 = 1.0f;
   dev->SetTransform(D3DTS_PROJECTION, &proj);
   float start = 0.0f, end = 4.0f;
   dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
   dev->SetRenderState(D3DRS_FOGCOLOR, WHITE);
   dev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
   dev->SetRenderState(D3DRS_FOGSTART, *(DWORD *) &start);
   dev->SetRenderState(D3DRS_FOGEND, *(DWORD *) &end);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ { -2, 2, 2, -2, 2.0f, WHITE } });
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 16, 16, 0xffff8080);
   EXPECT_PIXEL(f, 16, 48, 0xff80ff80);
   EXPECT_PIXEL(f, 48, 16, GRAY);   // discarded fragments get no fog

   D3DMATRIX identity = {};
   identity._11 = identity._22 = identity._33 = identity._44 = 1.0f;
   dev->SetTransform(D3DTS_PROJECTION, &identity);
   dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
   dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
   dev->SetTexture(0, nullptr);
   CHECK(tex->Release() == 0);
}

// Depth test, depth bias and cull winding, untextured (an empty stage is white).
void TestDepthAndCulling(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   dev->SetTexture(0, nullptr);
   dev->SetRenderState(D3DRS_ZENABLE, TRUE);
   dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ { -1, 1, 1, -1, 0.5f, RED } });
   s.Draw({ { -1, 1, 1, -1, 0.7f, GREEN } });   // behind: hidden
   s.Draw({ { -1, 1, 0, -1, 0.3f, BLUE } });    // in front, left half
   // Behind the red at 0.6, but a bias of -0.2 brings it to 0.4: right half.
   float bias = -0.2f;
   dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD *) &bias);
   s.Draw({ { 0, 1, 1, 0, 0.6f, YELLOW } });
   bias = 0.0f;
   dev->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD *) &bias);
   dev->EndScene();
   Present(dev);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, BLUE);
   EXPECT_PIXEL(f, 48, 48, RED);
   EXPECT_PIXEL(f, 48, 16, YELLOW);

   // The quads' first triangle runs top left, bottom left, top right:
   // counterclockwise on screen.  D3DCULL_CCW culls it, D3DCULL_CW keeps it.
   dev->SetRenderState(D3DRS_ZENABLE, FALSE);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
   s.Draw({ { -1, 1, 0, -1, 0.5f, RED } });
   dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
   s.Draw({ { 0, 1, 1, -1, 0.5f, GREEN } });
   dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, GRAY);
   EXPECT_PIXEL(f, 48, 32, GREEN);
   dev->SetRenderState(D3DRS_ZENABLE, TRUE);
}

// Draws read only what they need from buffers locked whole with DISCARD: quads
// spread over a 50000-vertex buffer, drawn one by one (uploads grow), base
// vertices, a second discard, and a lock without discard changing one quad.
void TestBufferUploads(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   dev->SetTexture(0, nullptr);
   std::vector<Quad> quads;
   for (int k = 0; k < 32; k++)
   {
      float x0 = -1 + k * (2.0f / 32), x1 = x0 + 2.0f / 32;
      DWORD color = 0xff000000 | (DWORD) (k * 8) << 16 | 0x40;
      quads.push_back({ x0, 1, x1, -1, 0.5f, color, { 0, 0 }, { 1, 1 }, (UINT) (k * 1500), (UINT) (k * 1500) });
   }
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw(quads);
   dev->EndScene();
   Present(dev);
   Frame f = ReadPresented();
   for (int k = 0; k < 32; k++)
      EXPECT_PIXEL(f, 2 * k + 1, 32, 0xff000040 | (DWORD) (k * 8) << 16);

   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ { -1, 1, 1, -1, 0.5f, YELLOW, { 0, 0 }, { 1, 1 }, 40000, 30000 } });
   // A new discard: the same vertex range now holds other data.
   s.Draw({ { -1, 1, 0, -1, 0.4f, CYAN, { 0, 0 }, { 1, 1 }, 40000, 30000 } });
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, CYAN);
   EXPECT_PIXEL(f, 48, 32, YELLOW);

   // Without discard: rewrite the colors of the quad last drawn, draw again.
   DWORD *c;
   CHECK(s.bgra->Lock(40000 * sizeof(DWORD), 4 * sizeof(DWORD), (void **) &c, 0) == D3D_OK);
   for (int i = 0; i < 4; i++)
      c[i] = MAGENTA;
   s.bgra->Unlock();
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   CHECK(dev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 40000, 30000, 4, 30000, 2) == D3D_OK);
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, MAGENTA);
   EXPECT_PIXEL(f, 48, 32, GRAY);
}

// Two texture stages: stage 1 modulates with a second texture.
void TestTwoStages(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   IDirect3DTexture9 *t0 = MakeTexture(dev, 1, 1, { 0xffff8040 });
   IDirect3DTexture9 *t1 = MakeTexture(dev, 1, 1, { 0xff8080ff });
   dev->SetTexture(0, t0);
   dev->SetTexture(1, t1);
   dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
   dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
   dev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ FULL }, true);
   dev->EndScene();
   Present(dev);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 32, 32, 0xff804040);

   dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
   dev->BeginScene();
   s.Draw({ FULL }, true);
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 32, 32, 0xffff8080);

   dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
   dev->SetTexture(0, nullptr);
   dev->SetTexture(1, nullptr);
   t0->Release();
   t1->Release();
}

// Render to a texture, sample it, StretchRect, and Present a part of the frame
// to a destination rectangle.
void TestRenderTargets(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   IDirect3DTexture9 *rt = nullptr, *copy = nullptr;
   CHECK(dev->CreateTexture(32, 32, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &rt, nullptr) ==
         D3D_OK);
   CHECK(dev->CreateTexture(64, 64, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &copy, nullptr) ==
         D3D_OK);
   D3DLOCKED_RECT locked;
   CHECK(rt->LockRect(0, &locked, nullptr, 0) == D3DERR_INVALIDCALL);

   IDirect3DSurface9 *back = nullptr, *depth = nullptr, *rtSurface = nullptr, *copySurface = nullptr;
   CHECK(dev->GetRenderTarget(0, &back) == D3D_OK);
   CHECK(dev->GetDepthStencilSurface(&depth) == D3D_OK);
   CHECK(rt->GetSurfaceLevel(0, &rtSurface) == D3D_OK);
   CHECK(copy->GetSurfaceLevel(0, &copySurface) == D3D_OK);
   D3DSURFACE_DESC desc;
   CHECK(back->GetDesc(&desc) == D3D_OK && desc.Width == SIZE && desc.Height == SIZE);

   dev->SetTexture(0, nullptr);
   dev->BeginScene();
   CHECK(dev->SetRenderTarget(0, rtSurface) == D3D_OK);
   D3DVIEWPORT9 vp;
   CHECK(dev->GetViewport(&vp) == D3D_OK && vp.Width == 32 && vp.Height == 32);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET, MAGENTA, 1.0f, 0);
   s.Draw({ { -1, 1, 1, 0, 0.5f, RED } });   // the top half
   CHECK(dev->SetRenderTarget(0, back) == D3D_OK);
   CHECK(dev->SetDepthStencilSurface(depth) == D3D_OK);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->SetTexture(0, rt);
   s.Draw({ FULL });
   dev->EndScene();
   Present(dev);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 32, 8, RED);
   EXPECT_PIXEL(f, 32, 56, MAGENTA);

   // Part of the frame, the top rows red, to a rectangle of the window.
   RECT src = { 0, 24, 64, 40 }, dst = { 10, 20, 74, 36 };
   Present(dev, &src, &dst);
   CHECK(gSurface.width == 64 && gSurface.height == 16);
   CHECK(gSurface.dest.left == 10 && gSurface.dest.top == 20 && gSurface.dest.right == 74 &&
         gSurface.dest.bottom == 36);
   f = ReadPresented();
   EXPECT_PIXEL(f, 5, 2, RED);
   EXPECT_PIXEL(f, 5, 13, MAGENTA);

   // The back buffer's top left quarter into a texture, drawn back flipped
   // nowhere: the lower half of the screen shows the quarter stretched.
   RECT quarter = { 0, 0, 32, 32 };
   CHECK(dev->StretchRect(back, &quarter, copySurface, &quarter, D3DTEXF_NONE) == D3D_OK);
   dev->SetTexture(0, copy);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ { -1, 1, 1, -1, 0.5f, WHITE, { 0, 0 }, { 0.5f, 0.5f } } });
   dev->EndScene();
   Present(dev);
   f = ReadPresented();
   EXPECT_PIXEL(f, 32, 32, RED);   // all of the quarter is red

   dev->SetTexture(0, nullptr);
   back->Release();
   depth->Release();
   rtSurface->Release();
   copySurface->Release();
   CHECK(rt->Release() == 0);
   CHECK(copy->Release() == 0);
}

// System memory texture (A1R5G5B5) copied to a default one with UpdateTexture;
// a managed texture locked twice keeps what was written.
void TestTextureUploads(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   IDirect3DTexture9 *sys = nullptr, *final = nullptr;
   CHECK(dev->CreateTexture(2, 1, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &sys, nullptr) == D3D_OK);
   D3DLOCKED_RECT r;
   CHECK(sys->LockRect(0, &r, nullptr, 0) == D3D_OK);
   CHECK(r.Pitch == 4);
   ((WORD *) r.pBits)[0] = 0xfc00;   // opaque red
   ((WORD *) r.pBits)[1] = 0x03e0;   // transparent green
   CHECK(sys->UnlockRect(0) == D3D_OK);
   CHECK(dev->CreateTexture(2, 1, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &final, nullptr) == D3D_OK);
   CHECK(final->AddDirtyRect(nullptr) == D3D_OK);
   CHECK(dev->UpdateTexture(sys, final) == D3D_OK);
   CHECK(sys->Release() == 0);

   dev->SetTexture(0, final);
   dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
   dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   dev->BeginScene();
   s.Draw({ FULL });
   dev->EndScene();
   Present(dev);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, RED);
   EXPECT_PIXEL(f, 48, 32, GRAY);
   dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
   dev->SetTexture(0, nullptr);
   final->Release();

   IDirect3DTexture9 *managed = MakeTexture(dev, 4, 4, std::vector<DWORD>(16, CYAN));
   CHECK(managed->LockRect(0, &r, nullptr, D3DLOCK_READONLY) == D3D_OK);
   CHECK(((DWORD *) r.pBits)[5] == CYAN);
   managed->UnlockRect(0);
   RECT part = { 1, 1, 3, 3 };
   CHECK(managed->LockRect(0, &r, &part, 0) == D3D_OK);
   ((DWORD *) r.pBits)[0] = YELLOW;   // texel (1, 1)
   managed->UnlockRect(0);
   CHECK(managed->LockRect(0, &r, nullptr, D3DLOCK_READONLY) == D3D_OK);
   CHECK(((DWORD *) r.pBits)[5] == YELLOW && ((DWORD *) r.pBits)[6] == CYAN);
   managed->UnlockRect(0);
   D3DSURFACE_DESC desc;
   CHECK(managed->GetLevelDesc(0, &desc) == D3D_OK && desc.Width == 4 && desc.Format == D3DFMT_A8R8G8B8 &&
         desc.Pool == D3DPOOL_MANAGED);
   managed->Release();
}

// d3d9.h only declares its IIDs.
const GUID kIID_IUnknown = { 0, 0, 0, { 0xc0, 0, 0, 0, 0, 0, 0, 0x46 } };
const GUID kIID_IDirect3DTexture9 = { 0x85c31227, 0x3de5, 0x4f00, { 0x9b, 0x3a, 0xf1, 0x1a, 0xc3, 0x8c, 0x18, 0xb5 } };
const GUID kIID_IDirect3DDevice9 = { 0xd0223b96, 0xbf7a, 0x43fd, { 0x92, 0xbd, 0xa4, 0x3b, 0x0d, 0x82, 0xb9, 0xeb } };

// COM behaviour: reference counts, QueryInterface, the unimplemented rest.
void TestCom(Scene &s)
{
   IDirect3DDevice9 *dev = s.dev;
   IDirect3DTexture9 *tex = MakeTexture(dev, 1, 1, { WHITE });
   IDirect3DSurface9 *level = nullptr;
   CHECK(tex->GetSurfaceLevel(0, &level) == D3D_OK);
   CHECK(tex->AddRef() == 3);   // its own, the surface's, this one
   CHECK(tex->Release() == 2);
   IDirect3DTexture9 *container = nullptr;
   CHECK(level->GetContainer(kIID_IDirect3DTexture9, (void **) &container) == D3D_OK && container == tex);
   container->Release();
   IUnknown *unknown = nullptr;
   CHECK(tex->QueryInterface(kIID_IUnknown, (void **) &unknown) == S_OK && unknown != nullptr);
   unknown->Release();
   CHECK(tex->QueryInterface(kIID_IDirect3DDevice9, (void **) &unknown) == E_NOINTERFACE);
   IDirect3DDevice9 *owner = nullptr;
   CHECK(tex->GetDevice(&owner) == D3D_OK && owner == dev);
   owner->Release();
   CHECK(level->Release() == 1);
   CHECK(tex->Release() == 0);

   IDirect3DCubeTexture9 *cube = nullptr;
   CHECK(dev->CreateCubeTexture(16, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &cube, nullptr) == D3DERR_INVALIDCALL);
   CHECK(dev->SetFVF(D3DFVF_XYZ) == D3DERR_INVALIDCALL);
   CHECK(dev->Reset(nullptr) == D3DERR_INVALIDCALL);
   CHECK(dev->TestCooperativeLevel() == D3D_OK);
   D3DCAPS9 caps;
   CHECK(dev->GetDeviceCaps(&caps) == D3D_OK);
   CHECK(caps.MaxTextureBlendStages >= 2 && caps.MaxSimultaneousTextures >= 2 && caps.MaxStreams > 0);
   CHECK((caps.RasterCaps & (D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_ZFOG)) ==
         (D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_ZFOG));
   CHECK(caps.MaxTextureWidth >= 2048);
   DWORD value = 0;
   CHECK(dev->GetRenderState(D3DRS_CULLMODE, &value) == D3D_OK && value == D3DCULL_NONE);
}

// A device created with 4 samples renders and resolves on Present.
void TestMultisample(IDirect3D9 *d3d)
{
   Scene s;
   CHECK(s.Create(d3d, D3DMULTISAMPLE_4_SAMPLES));
   if (s.dev == nullptr)
      return;
   s.dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GRAY, 1.0f, 0);
   s.dev->BeginScene();
   s.Draw({ { -1, 1, 0, -1, 0.5f, GREEN } });
   s.dev->EndScene();
   Present(s.dev);
   Frame f = ReadPresented();
   EXPECT_PIXEL(f, 16, 32, GREEN);
   EXPECT_PIXEL(f, 48, 32, GRAY);
   s.Release();
   CHECK(s.dev->Release() == 0);
}

// The window's surface replaced: Present reports the device lost, and it
// asks for a reset (the client then creates a new device).
void TestLost(IDirect3D9 *d3d)
{
   Scene s;
   CHECK(s.Create(d3d, D3DMULTISAMPLE_NONE));
   if (s.dev == nullptr)
      return;
   EglSurface other;
   other.display = gSurface.display;
   other.context = gSurface.context;
   other.surface = gSurface.surface;
   gProvided = &other;
   CHECK(s.dev->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST);
   CHECK(s.dev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET);
   CHECK(s.dev->Clear(0, nullptr, D3DCLEAR_TARGET, GRAY, 1.0f, 0) == D3D_OK);
   gProvided = &gSurface;
   s.Release();
   CHECK(s.dev->Release() == 0);
}

#endif  // !__APPLE__

}  // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#ifdef __APPLE__
   printf("gfx_d3d9: skipped (the test surface needs EGL)\n");
   return 0;
#else
   if (!gSurface.Create())
   {
      fprintf(stderr, "gfx_d3d9: no EGL desktop OpenGL 3.3 core context\n");
      return 1;
   }
   printf("gfx_d3d9: %s, %s\n", gl.GetString(GL_RENDERER), gl.GetString(GL_VERSION));
   w32::gfx::SetSurfaceProvider(Provide);

   IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
   CHECK(d3d != nullptr);
   if (d3d == nullptr)
      return 1;
   CHECK(d3d->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, TRUE) == D3D_OK);
   DWORD quality = 0;
   CHECK(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, TRUE,
                                         D3DMULTISAMPLE_16_SAMPLES, &quality) == D3D_OK && quality >= 1);

   Scene s;
   CHECK(s.Create(d3d, D3DMULTISAMPLE_NONE));
   if (s.dev == nullptr)
      return 1;
   D3DADAPTER_IDENTIFIER9 id;
   CHECK(d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id) == D3D_OK && id.Description[0] != 0);

   TestTexturedAlphaTest(s);
   TestDepthAndCulling(s);
   TestBufferUploads(s);
   TestTwoStages(s);
   TestRenderTargets(s);
   TestTextureUploads(s);
   TestCom(s);

   // A resource outliving its device keeps working as an object.
   IDirect3DTexture9 *survivor = MakeTexture(s.dev, 1, 1, { WHITE });
   s.Release();
   CHECK(s.dev->Release() == 0);
   IDirect3DDevice9 *gone = nullptr;
   CHECK(survivor->GetDevice(&gone) == D3DERR_INVALIDCALL);
   CHECK(survivor->Release() == 0);

   TestMultisample(d3d);
   TestLost(d3d);
   CHECK(d3d->Release() == 0);

   if (gFailures)
   {
      printf("gfx_d3d9: %d failures\n", gFailures);
      return 1;
   }
   printf("gfx_d3d9: all passed\n");
   return 0;
#endif
}

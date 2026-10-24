/*
 * device.cpp:  IDirect3DDevice9 on OpenGL 3.3 core.
 *
 * The device renders with the window's GLSurface context into a back buffer
 * FBO.  One shader program emulates the fixed-function pipeline the client
 * uses: three transforms, two texture stages, alpha test, linear W fog and
 * depth bias.  Every target is drawn with clip-space y negated, which leaves
 * row 0 of each target at the top as in Direct3D (culling winding is inverted to
 * match); Present copies the presented part of the frame into a texture and
 * hands it to the surface.
 */

#include "core/win32.h"
#include "gfx/d3d9_impl.h"
#include "gfx/gfx.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace w32::gfx {

namespace {

Device *gDevices = nullptr;

// Uploads and readbacks bind here, so they never disturb the stages' bindings.
constexpr GLuint UPLOAD_UNIT = NUM_STAGES;

enum AttribLocation { ATTRIB_POSITION = 0, ATTRIB_COLOR = 1, ATTRIB_TEX0 = 2, ATTRIB_TEX1 = 3 };

const char *VERTEX_SHADER = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTex0;
layout(location = 3) in vec2 aTex1;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uDepthBias;
// Elements the vertex declaration lacks read as opaque white and (0, 0).
uniform bool uHasColor;
uniform bool uHasTexCoord0;
uniform bool uHasTexCoord1;

out vec4 vColor;
out vec2 vTex0;
out vec2 vTex1;
out float vFogDepth;

void main()
{
   // Direct3D row vectors: v * World * View * Proj, with the matrices uploaded as stored.
   vec4 clip = uProj * (uView * (uWorld * vec4(aPosition, 1.0)));
   vFogDepth = clip.w;
   clip.z = 2.0 * (clip.z + uDepthBias * clip.w) - clip.w;   // 0..w depth to -w..w
   clip.y = -clip.y;                                          // row 0 at the top
   gl_Position = clip;
   vColor = uHasColor ? aColor : vec4(1.0);
   vTex0 = uHasTexCoord0 ? aTex0 : vec2(0.0);
   vTex1 = uHasTexCoord1 ? aTex1 : vec2(0.0);
}
)";

const char *FRAGMENT_SHADER = R"(#version 330 core
in vec4 vColor;
in vec2 vTex0;
in vec2 vTex1;
in float vFogDepth;

uniform sampler2D uTex0;
uniform sampler2D uTex1;
uniform bool uHasTex0;
uniform bool uHasTex1;
// Per stage: op, arg1, arg2, for color and for alpha
uniform ivec3 uColorStage0;
uniform ivec3 uAlphaStage0;
uniform ivec3 uColorStage1;
uniform ivec3 uAlphaStage1;

uniform bool uAlphaTest;
uniform int uAlphaFunc;
uniform float uAlphaRef;

uniform bool uFog;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;

out vec4 fragColor;

const int OP_DISABLE = 1, OP_SELECTARG1 = 2, OP_SELECTARG2 = 3, OP_MODULATE = 4,
          OP_MODULATE2X = 5, OP_MODULATE4X = 6, OP_ADD = 7;
const int ARG_DIFFUSE = 0, ARG_CURRENT = 1, ARG_TEXTURE = 2;

vec4 arg(int a, vec4 diffuse, vec4 current, vec4 tex)
{
   if (a == ARG_TEXTURE) return tex;
   if (a == ARG_CURRENT) return current;
   return diffuse;
}

vec4 combine(int op, vec4 a1, vec4 a2)
{
   if (op == OP_SELECTARG1) return a1;
   if (op == OP_SELECTARG2) return a2;
   if (op == OP_MODULATE) return a1 * a2;
   if (op == OP_MODULATE2X) return clamp(a1 * a2 * 2.0, 0.0, 1.0);
   if (op == OP_MODULATE4X) return clamp(a1 * a2 * 4.0, 0.0, 1.0);
   if (op == OP_ADD) return clamp(a1 + a2, 0.0, 1.0);
   return a1;
}

bool compare(int func, float a, float ref)
{
   if (func == 1) return false;
   if (func == 2) return a < ref;
   if (func == 3) return a == ref;
   if (func == 4) return a <= ref;
   if (func == 5) return a > ref;
   if (func == 6) return a != ref;
   if (func == 7) return a >= ref;
   return true;
}

void main()
{
   // An empty stage samples opaque white.
   vec4 tex0 = uHasTex0 ? texture(uTex0, vTex0) : vec4(1.0);
   vec4 tex1 = uHasTex1 ? texture(uTex1, vTex1) : vec4(1.0);
   vec4 current = vColor;

   if (uColorStage0.x != OP_DISABLE)
   {
      vec3 c = combine(uColorStage0.x, arg(uColorStage0.y, vColor, current, tex0),
                                       arg(uColorStage0.z, vColor, current, tex0)).rgb;
      float a = current.a;
      if (uAlphaStage0.x != OP_DISABLE)
         a = combine(uAlphaStage0.x, arg(uAlphaStage0.y, vColor, current, tex0),
                                     arg(uAlphaStage0.z, vColor, current, tex0)).a;
      current = vec4(c, a);

      if (uColorStage1.x != OP_DISABLE)
      {
         vec3 c1 = combine(uColorStage1.x, arg(uColorStage1.y, vColor, current, tex1),
                                           arg(uColorStage1.z, vColor, current, tex1)).rgb;
         float a1 = current.a;
         if (uAlphaStage1.x != OP_DISABLE)
            a1 = combine(uAlphaStage1.x, arg(uAlphaStage1.y, vColor, current, tex1),
                                         arg(uAlphaStage1.z, vColor, current, tex1)).a;
         current = vec4(c1, a1);
      }
   }

   if (uAlphaTest && !compare(uAlphaFunc, floor(current.a * 255.0 + 0.5), uAlphaRef))
      discard;

   if (uFog)
   {
      float f = clamp((uFogEnd - vFogDepth) / (uFogEnd - uFogStart), 0.0, 1.0);
      current.rgb = mix(uFogColor, current.rgb, f);
   }

   fragColor = current;
}
)";

GLenum CompareFunc(DWORD f)
{
   switch (f)
   {
   case D3DCMP_NEVER:        return GL_NEVER;
   case D3DCMP_LESS:         return GL_LESS;
   case D3DCMP_EQUAL:        return GL_EQUAL;
   case D3DCMP_LESSEQUAL:    return GL_LEQUAL;
   case D3DCMP_GREATER:      return GL_GREATER;
   case D3DCMP_NOTEQUAL:     return GL_NOTEQUAL;
   case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
   default:                  return GL_ALWAYS;
   }
}

GLenum BlendFactor(DWORD b)
{
   switch (b)
   {
   case D3DBLEND_ZERO:         return GL_ZERO;
   case D3DBLEND_ONE:          return GL_ONE;
   case D3DBLEND_SRCCOLOR:     return GL_SRC_COLOR;
   case D3DBLEND_INVSRCCOLOR:  return GL_ONE_MINUS_SRC_COLOR;
   case D3DBLEND_SRCALPHA:     return GL_SRC_ALPHA;
   case D3DBLEND_INVSRCALPHA:  return GL_ONE_MINUS_SRC_ALPHA;
   case D3DBLEND_DESTALPHA:    return GL_DST_ALPHA;
   case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
   case D3DBLEND_DESTCOLOR:    return GL_DST_COLOR;
   case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
   case D3DBLEND_SRCALPHASAT:  return GL_SRC_ALPHA_SATURATE;
   default:                    return GL_ONE;
   }
}

GLenum BlendEquation(DWORD op)
{
   switch (op)
   {
   case D3DBLENDOP_SUBTRACT:    return GL_FUNC_SUBTRACT;
   case D3DBLENDOP_REVSUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
   case D3DBLENDOP_MIN:         return GL_MIN;
   case D3DBLENDOP_MAX:         return GL_MAX;
   default:                     return GL_FUNC_ADD;
   }
}

GLenum StencilOp(DWORD op)
{
   switch (op)
   {
   case D3DSTENCILOP_ZERO:    return GL_ZERO;
   case D3DSTENCILOP_REPLACE: return GL_REPLACE;
   case D3DSTENCILOP_INCRSAT: return GL_INCR;
   case D3DSTENCILOP_DECRSAT: return GL_DECR;
   case D3DSTENCILOP_INVERT:  return GL_INVERT;
   case D3DSTENCILOP_INCR:    return GL_INCR_WRAP;
   case D3DSTENCILOP_DECR:    return GL_DECR_WRAP;
   default:                   return GL_KEEP;
   }
}

GLenum AddressMode(DWORD a)
{
   switch (a)
   {
   case D3DTADDRESS_MIRROR: return GL_MIRRORED_REPEAT;
   case D3DTADDRESS_CLAMP:  return GL_CLAMP_TO_EDGE;
   case D3DTADDRESS_BORDER: return GL_CLAMP_TO_BORDER;
   default:                 return GL_REPEAT;
   }
}

float AsFloat(DWORD d)
{
   float f;
   memcpy(&f, &d, sizeof(f));
   return f;
}

DWORD AsDword(float f)
{
   DWORD d;
   memcpy(&d, &f, sizeof(d));
   return d;
}

void Identity(D3DMATRIX *m)
{
   memset(m, 0, sizeof(*m));
   m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
}

// GL attribute layout of a declaration type: components, type, normalized,
// bytes; false if the shader cannot read it.
bool AttribFormat(BYTE type, GLint *size, GLenum *glType, GLboolean *normalized, UINT *bytes)
{
   *normalized = GL_FALSE;
   switch (type)
   {
   case D3DDECLTYPE_FLOAT1:   *size = 1; *glType = GL_FLOAT; *bytes = 4; return true;
   case D3DDECLTYPE_FLOAT2:   *size = 2; *glType = GL_FLOAT; *bytes = 8; return true;
   case D3DDECLTYPE_FLOAT3:   *size = 3; *glType = GL_FLOAT; *bytes = 12; return true;
   case D3DDECLTYPE_FLOAT4:   *size = 4; *glType = GL_FLOAT; *bytes = 16; return true;
   case D3DDECLTYPE_D3DCOLOR:
      // Stored B, G, R, A in memory.
      *size = GL_BGRA; *glType = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; *bytes = 4; return true;
   case D3DDECLTYPE_UBYTE4N:
      *size = 4; *glType = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; *bytes = 4; return true;
   default:
      return false;
   }
}

// GL primitive and index count for a Direct3D primitive count.
bool Primitive(D3DPRIMITIVETYPE type, UINT primCount, GLenum *mode, UINT *count)
{
   switch (type)
   {
   case D3DPT_POINTLIST:     *mode = GL_POINTS;         *count = primCount; return true;
   case D3DPT_LINELIST:      *mode = GL_LINES;          *count = primCount * 2; return true;
   case D3DPT_LINESTRIP:     *mode = GL_LINE_STRIP;     *count = primCount + 1; return true;
   case D3DPT_TRIANGLELIST:  *mode = GL_TRIANGLES;      *count = primCount * 3; return true;
   case D3DPT_TRIANGLESTRIP: *mode = GL_TRIANGLE_STRIP; *count = primCount + 2; return true;
   case D3DPT_TRIANGLEFAN:   *mode = GL_TRIANGLE_FAN;   *count = primCount + 2; return true;
   default:                  return false;
   }
}

GLuint CompileShader(const GL &gl, GLenum kind, const char *source)
{
   GLuint shader = gl.CreateShader(kind);
   gl.ShaderSource(shader, 1, &source, nullptr);
   gl.CompileShader(shader);
   GLint ok = 0;
   gl.GetShaderiv(shader, GL_COMPILE_STATUS, &ok);
   if (!ok)
   {
      char log[2048];
      gl.GetShaderInfoLog(shader, sizeof(log), nullptr, log);
      fprintf(stderr, "d3d9: shader compile failed: %s\n", log);
   }
   return shader;
}

}  // namespace

void InvalidateGLState()
{
   for (Device *d = gDevices; d != nullptr; d = d->nextDevice)
      d->glValid = false;
}

/************************************************************************/
// Creation and teardown

Device *Device::Create(Direct3D9 *d3d, HWND window, HWND focus, DWORD flags,
                       const D3DPRESENT_PARAMETERS &params)
{
   auto *device = new Device;
   if (!device->Init(d3d, window, focus, flags, params))
   {
      delete device;
      return nullptr;
   }
   return device;
}

bool Device::Init(Direct3D9 *d3dIn, HWND windowIn, HWND focus, DWORD flags,
                  const D3DPRESENT_PARAMETERS &params)
{
   d3d = d3dIn;
   d3d->AddRef();
   window = windowIn;
   focusWindow = focus;
   behaviorFlags = flags;
   nextDevice = gDevices;
   gDevices = this;

   width = params.BackBufferWidth;
   height = params.BackBufferHeight;
   if (width <= 0 || height <= 0)
   {
      fprintf(stderr, "d3d9: a back buffer size is required\n");
      return false;
   }

   surface = SurfaceFor(window);
   if (surface == nullptr)
   {
      fprintf(stderr, "d3d9: the window has no OpenGL surface\n");
      return false;
   }
   if (!surface->MakeCurrent())
   {
      fprintf(stderr, "d3d9: could not make the OpenGL context current\n");
      surface = nullptr;
      return false;
   }
   if (!gl.Load(surface))
   {
      surface = nullptr;
      return false;
   }

   GLint major = 0, minor = 0;
   gl.GetIntegerv(GL_MAJOR_VERSION, &major);
   gl.GetIntegerv(GL_MINOR_VERSION, &minor);
   if (major * 10 + minor < 33)
   {
      fprintf(stderr, "d3d9: OpenGL %d.%d; 3.3 core is required\n", major, minor);
      surface = nullptr;
      return false;
   }

   gl.GetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
   GLint numExtensions = 0;
   gl.GetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
   for (GLint i = 0; i < numExtensions; i++)
   {
      auto *name = reinterpret_cast<const char *>(gl.GetStringi(GL_EXTENSIONS, i));
      if (name && (!strcmp(name, "GL_EXT_texture_filter_anisotropic") ||
                   !strcmp(name, "GL_ARB_texture_filter_anisotropic")))
         gl.GetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
   }
   if (auto *s = reinterpret_cast<const char *>(gl.GetString(GL_RENDERER)))
      snprintf(d3d->renderer, sizeof(d3d->renderer), "%s", s);
   if (auto *s = reinterpret_cast<const char *>(gl.GetString(GL_VENDOR)))
      snprintf(d3d->vendor, sizeof(d3d->vendor), "%s", s);

   GLint maxSamples = 0;
   gl.GetIntegerv(GL_MAX_SAMPLES, &maxSamples);
   samples = params.MultiSampleType >= D3DMULTISAMPLE_2_SAMPLES ? (int) params.MultiSampleType : 0;
   samples = std::min(samples, (int) maxSamples);
   if (samples < 2)
      samples = 0;

   if (!CreateProgram())
   {
      surface = nullptr;
      return false;
   }
   gl.GenVertexArrays(1, &vao);
   gl.GenSamplers(NUM_STAGES, samplers);
   // Texture 0 is incomplete, and macOS's GL warns of every sampler left on it.
   const GLubyte white[4] = {255, 255, 255, 255};
   gl.GenTextures(1, &whiteTexture);
   BindForUpload(whiteTexture);
   gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

   // Direct3D 9 defaults.
   DWORD *rs = renderState;
   rs[D3DRS_ZENABLE] = params.EnableAutoDepthStencil ? D3DZB_TRUE : D3DZB_FALSE;
   rs[D3DRS_FILLMODE] = D3DFILL_SOLID;
   rs[D3DRS_SHADEMODE] = D3DSHADE_GOURAUD;
   rs[D3DRS_ZWRITEENABLE] = TRUE;
   rs[D3DRS_LASTPIXEL] = TRUE;
   rs[D3DRS_SRCBLEND] = D3DBLEND_ONE;
   rs[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
   rs[D3DRS_CULLMODE] = D3DCULL_CCW;
   rs[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
   rs[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
   rs[D3DRS_FOGEND] = AsDword(1.0f);
   rs[D3DRS_FOGDENSITY] = AsDword(1.0f);
   rs[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
   rs[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
   rs[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
   rs[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
   rs[D3DRS_STENCILMASK] = 0xffffffff;
   rs[D3DRS_STENCILWRITEMASK] = 0xffffffff;
   rs[D3DRS_CLIPPING] = TRUE;
   rs[D3DRS_LIGHTING] = TRUE;
   rs[D3DRS_COLORVERTEX] = TRUE;
   rs[D3DRS_MULTISAMPLEANTIALIAS] = TRUE;
   rs[D3DRS_MULTISAMPLEMASK] = 0xffffffff;
   rs[D3DRS_COLORWRITEENABLE] = 0xf;
   rs[D3DRS_BLENDOP] = D3DBLENDOP_ADD;
   for (int i = 0; i < 8; i++)
   {
      DWORD *v = stage[i].value;
      v[D3DTSS_COLOROP] = i == 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE;
      v[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
      v[D3DTSS_COLORARG2] = D3DTA_CURRENT;
      v[D3DTSS_ALPHAOP] = i == 0 ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE;
      v[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
      v[D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
      v[D3DTSS_TEXCOORDINDEX] = i;
   }
   Identity(&world);
   Identity(&view);
   Identity(&proj);
   depthEnabled = params.EnableAutoDepthStencil != FALSE;

   CreateBackBuffer();
   currentTarget = &backSurface;
   currentFramebuffer = backFramebuffer;
   targetWidth = width;
   targetHeight = height;
   viewport = { 0, 0, (DWORD) width, (DWORD) height, 0.0f, 1.0f };
   backSurface.owner = depthSurface.owner = this;

   glValid = false;
   return Begin();
}

bool Device::CreateProgram()
{
   GLuint vs = CompileShader(gl, GL_VERTEX_SHADER, VERTEX_SHADER);
   GLuint fs = CompileShader(gl, GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
   program = gl.CreateProgram();
   gl.AttachShader(program, vs);
   gl.AttachShader(program, fs);
   gl.LinkProgram(program);
   gl.DeleteShader(vs);
   gl.DeleteShader(fs);
   GLint linked = 0;
   gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
   if (!linked)
   {
      char log[2048];
      gl.GetProgramInfoLog(program, sizeof(log), nullptr, log);
      fprintf(stderr, "d3d9: program link failed: %s\n", log);
      return false;
   }

   auto loc = [&](const char *name) { return gl.GetUniformLocation(program, name); };
   u.world = loc("uWorld");
   u.view = loc("uView");
   u.proj = loc("uProj");
   u.depthBias = loc("uDepthBias");
   u.tex0 = loc("uTex0");
   u.tex1 = loc("uTex1");
   u.hasTex0 = loc("uHasTex0");
   u.hasTex1 = loc("uHasTex1");
   u.colorStage0 = loc("uColorStage0");
   u.alphaStage0 = loc("uAlphaStage0");
   u.colorStage1 = loc("uColorStage1");
   u.alphaStage1 = loc("uAlphaStage1");
   u.alphaTest = loc("uAlphaTest");
   u.alphaFunc = loc("uAlphaFunc");
   u.alphaRef = loc("uAlphaRef");
   u.fog = loc("uFog");
   u.fogColor = loc("uFogColor");
   u.fogStart = loc("uFogStart");
   u.fogEnd = loc("uFogEnd");
   u.hasColor = loc("uHasColor");
   u.hasTexCoord0 = loc("uHasTexCoord0");
   u.hasTexCoord1 = loc("uHasTexCoord1");
   gl.UseProgram(program);
   gl.Uniform1i(u.tex0, 0);
   gl.Uniform1i(u.tex1, 1);
   return true;
}

void Device::CreateBackBuffer()
{
   gl.GenFramebuffers(1, &backFramebuffer);
   gl.BindFramebuffer(GL_FRAMEBUFFER, backFramebuffer);

   gl.GenRenderbuffers(1, &backColor);
   gl.BindRenderbuffer(GL_RENDERBUFFER, backColor);
   if (samples)
      gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
   else
      gl.RenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
   gl.FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, backColor);

   gl.GenRenderbuffers(1, &backDepth);
   gl.BindRenderbuffer(GL_RENDERBUFFER, backDepth);
   if (samples)
      gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
   else
      gl.RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
   gl.FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, backDepth);

   if (gl.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      fprintf(stderr, "d3d9: back buffer incomplete\n");

   gl.GenFramebuffers(1, &presentFramebuffer);
}

Device::~Device()
{
   bool current = surface != nullptr && !lost && surface->MakeCurrent();
   while (resources != nullptr)
   {
      Tracked *t = resources;
      Untrack(t);
      if (current)
         t->Orphan();
      else
         t->device = nullptr;
   }
   if (current)
   {
      gl.DeleteFramebuffers(1, &backFramebuffer);
      gl.DeleteFramebuffers(1, &presentFramebuffer);
      gl.DeleteRenderbuffers(1, &backColor);
      gl.DeleteRenderbuffers(1, &backDepth);
      gl.DeleteTextures(2, presentTextures);
      gl.DeleteSamplers(NUM_STAGES, samplers);
      gl.DeleteTextures(1, &whiteTexture);
      gl.DeleteVertexArrays(1, &vao);
      gl.DeleteProgram(program);
   }
   for (Device **p = &gDevices; *p != nullptr; p = &(*p)->nextDevice)
      if (*p == this)
      {
         *p = nextDevice;
         break;
      }
   if (d3d != nullptr)
      d3d->Release();
}

bool Device::HasInterface(REFIID riid)
{
   return SameIID(riid, kIID_IDirect3DDevice9);
}

void Device::Destroy()
{
   delete this;
}

void Device::Track(Tracked *t)
{
   t->device = this;
   t->prev = nullptr;
   t->next = resources;
   if (resources != nullptr)
      resources->prev = t;
   resources = t;
}

void Device::Untrack(Tracked *t)
{
   if (t->prev != nullptr)
      t->prev->next = t->next;
   else if (resources == t)
      resources = t->next;
   if (t->next != nullptr)
      t->next->prev = t->prev;
   t->prev = t->next = nullptr;
}

void Device::Unbind(Texture *t)
{
   for (auto &bound : texture)
      if (bound == t)
      {
         bound = nullptr;
         texturesDirty = true;
      }
   if (currentTarget == &t->level0)
      SetRenderTarget(0, &backSurface);
}

void Device::Unbind(VertexBuffer *b)
{
   for (Stream &s : streams)
      if (s.buffer == b)
      {
         s.buffer = nullptr;
         attributesDirty = true;
      }
}

void Device::Unbind(IndexBuffer *b)
{
   if (indices == b)
   {
      indices = nullptr;
      attributesDirty = true;
   }
}

void Device::Unbind(VertexDeclaration *d)
{
   if (declaration == d)
   {
      declaration = nullptr;
      attributesDirty = true;
   }
}

/************************************************************************/
// GL state

bool Device::Begin()
{
   if (lost || surface == nullptr)
      return false;
   if (glValid)
      return true;
   if (!surface->MakeCurrent())
      return false;
   Rebind();
   glValid = true;
   return true;
}

void Device::Rebind()
{
   gl.BindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
   ApplyViewport();
   gl.UseProgram(program);
   gl.BindVertexArray(vao);
   for (int i = 0; i < NUM_STAGES; i++)
   {
      gl.BindSampler(i, samplers[i]);
      sampler[i].dirty = true;
   }
   gl.Disable(GL_SCISSOR_TEST);
   gl.Disable(GL_FRAMEBUFFER_SRGB);
   gl.Disable(GL_DITHER);
   gl.PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   gl.PixelStorei(GL_UNPACK_SKIP_ROWS, 0);
   gl.PixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
   gl.PixelStorei(GL_PACK_ROW_LENGTH, 0);
   stateDirty = transformsDirty = texturesDirty = attributesDirty = true;
}

void Device::BindTarget(GLuint framebuffer, int w, int h)
{
   currentFramebuffer = framebuffer;
   targetWidth = w;
   targetHeight = h;
   // Setting a render target resets the viewport to the whole target.
   viewport = { 0, 0, (DWORD) w, (DWORD) h, 0.0f, 1.0f };
   if (Begin())
   {
      gl.BindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      ApplyViewport();
   }
}

void Device::ApplyViewport()
{
   // Targets are stored top row first, so the Direct3D origin maps directly.
   gl.Viewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
   gl.DepthRange(viewport.MinZ, viewport.MaxZ);
}

void Device::BindForUpload(GLuint tex)
{
   gl.ActiveTexture(GL_TEXTURE0 + UPLOAD_UNIT);
   gl.BindTexture(GL_TEXTURE_2D, tex);
}

void Device::ApplySampler(int i)
{
   SamplerState &s = sampler[i];
   if (!s.dirty)
      return;
   s.dirty = false;

   GLuint id = samplers[i];
   gl.SamplerParameteri(id, GL_TEXTURE_WRAP_S, AddressMode(s.addressU));
   gl.SamplerParameteri(id, GL_TEXTURE_WRAP_T, AddressMode(s.addressV));
   float border[4] = { ((s.borderColor >> 16) & 0xff) / 255.0f, ((s.borderColor >> 8) & 0xff) / 255.0f,
                       (s.borderColor & 0xff) / 255.0f, ((s.borderColor >> 24) & 0xff) / 255.0f };
   gl.SamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, border);

   bool linearMag = s.magFilter == D3DTEXF_LINEAR || s.magFilter == D3DTEXF_ANISOTROPIC;
   bool linearMin = s.minFilter == D3DTEXF_LINEAR || s.minFilter == D3DTEXF_ANISOTROPIC;
   // Textures have a single level, so the mip filter never selects another level.
   gl.SamplerParameteri(id, GL_TEXTURE_MAG_FILTER, linearMag ? GL_LINEAR : GL_NEAREST);
   gl.SamplerParameteri(id, GL_TEXTURE_MIN_FILTER, linearMin ? GL_LINEAR : GL_NEAREST);
   if (maxAnisotropy > 1.0f)
   {
      float aniso = 1.0f;
      if (s.minFilter == D3DTEXF_ANISOTROPIC || s.magFilter == D3DTEXF_ANISOTROPIC)
         aniso = std::clamp((float) s.maxAnisotropy, 1.0f, maxAnisotropy);
      gl.SamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, aniso);
   }
}

void Device::ApplyTextures()
{
   if (!texturesDirty)
      return;
   texturesDirty = false;
   bool has[NUM_STAGES];
   for (int i = 0; i < NUM_STAGES; i++)
   {
      auto *t = static_cast<Texture *>(texture[i]);
      GLuint name = t ? t->glTexture : 0;   // system memory textures have none
      has[i] = name != 0;
      gl.ActiveTexture(GL_TEXTURE0 + i);
      gl.BindTexture(GL_TEXTURE_2D, name ? name : whiteTexture);
   }
   gl.Uniform1i(u.hasTex0, has[0]);
   gl.Uniform1i(u.hasTex1, has[1]);
}

void Device::ApplyState()
{
   for (int i = 0; i < NUM_STAGES; i++)
      ApplySampler(i);

   if (transformsDirty)
   {
      gl.UniformMatrix4fv(u.world, 1, GL_FALSE, &world._11);
      gl.UniformMatrix4fv(u.view, 1, GL_FALSE, &view._11);
      gl.UniformMatrix4fv(u.proj, 1, GL_FALSE, &proj._11);
      transformsDirty = false;
   }

   if (!stateDirty)
      return;
   stateDirty = false;

   const DWORD *rs = renderState;
   const DWORD *s0 = stage[0].value, *s1 = stage[1].value;
   gl.Uniform3i(u.colorStage0, s0[D3DTSS_COLOROP], s0[D3DTSS_COLORARG1], s0[D3DTSS_COLORARG2]);
   gl.Uniform3i(u.alphaStage0, s0[D3DTSS_ALPHAOP], s0[D3DTSS_ALPHAARG1], s0[D3DTSS_ALPHAARG2]);
   gl.Uniform3i(u.colorStage1, s1[D3DTSS_COLOROP], s1[D3DTSS_COLORARG1], s1[D3DTSS_COLORARG2]);
   gl.Uniform3i(u.alphaStage1, s1[D3DTSS_ALPHAOP], s1[D3DTSS_ALPHAARG1], s1[D3DTSS_ALPHAARG2]);

   gl.Uniform1i(u.alphaTest, rs[D3DRS_ALPHATESTENABLE] != 0);
   gl.Uniform1i(u.alphaFunc, rs[D3DRS_ALPHAFUNC]);
   gl.Uniform1f(u.alphaRef, (float) (rs[D3DRS_ALPHAREF] & 0xff));

   bool fog = rs[D3DRS_FOGENABLE] && rs[D3DRS_FOGTABLEMODE] == D3DFOG_LINEAR;
   gl.Uniform1i(u.fog, fog);
   DWORD fc = rs[D3DRS_FOGCOLOR];
   gl.Uniform3f(u.fogColor, ((fc >> 16) & 0xff) / 255.0f, ((fc >> 8) & 0xff) / 255.0f, (fc & 0xff) / 255.0f);
   gl.Uniform1f(u.fogStart, AsFloat(rs[D3DRS_FOGSTART]));
   gl.Uniform1f(u.fogEnd, AsFloat(rs[D3DRS_FOGEND]));
   gl.Uniform1f(u.depthBias, AsFloat(rs[D3DRS_DEPTHBIAS]));

   float slope = AsFloat(rs[D3DRS_SLOPESCALEDEPTHBIAS]);
   if (slope != 0.0f)
   {
      gl.Enable(GL_POLYGON_OFFSET_FILL);
      gl.PolygonOffset(slope, 0.0f);
   }
   else
      gl.Disable(GL_POLYGON_OFFSET_FILL);

   // Texture targets have no depth buffer; there GL passes every depth test.
   if (rs[D3DRS_ZENABLE] && depthEnabled)
   {
      gl.Enable(GL_DEPTH_TEST);
      gl.DepthFunc(CompareFunc(rs[D3DRS_ZFUNC]));
   }
   else
      gl.Disable(GL_DEPTH_TEST);
   gl.DepthMask(rs[D3DRS_ZWRITEENABLE] && depthEnabled ? GL_TRUE : GL_FALSE);

   if (rs[D3DRS_ALPHABLENDENABLE])
   {
      gl.Enable(GL_BLEND);
      gl.BlendFunc(BlendFactor(rs[D3DRS_SRCBLEND]), BlendFactor(rs[D3DRS_DESTBLEND]));
      gl.BlendEquation(BlendEquation(rs[D3DRS_BLENDOP]));
   }
   else
      gl.Disable(GL_BLEND);

   DWORD mask = rs[D3DRS_COLORWRITEENABLE];
   gl.ColorMask((mask & D3DCOLORWRITEENABLE_RED) != 0, (mask & D3DCOLORWRITEENABLE_GREEN) != 0,
                (mask & D3DCOLORWRITEENABLE_BLUE) != 0, (mask & D3DCOLORWRITEENABLE_ALPHA) != 0);

   if (rs[D3DRS_STENCILENABLE])
   {
      gl.Enable(GL_STENCIL_TEST);
      gl.StencilFunc(CompareFunc(rs[D3DRS_STENCILFUNC]), rs[D3DRS_STENCILREF], rs[D3DRS_STENCILMASK]);
      gl.StencilOp(StencilOp(rs[D3DRS_STENCILFAIL]), StencilOp(rs[D3DRS_STENCILZFAIL]),
                   StencilOp(rs[D3DRS_STENCILPASS]));
      gl.StencilMask(rs[D3DRS_STENCILWRITEMASK]);
   }
   else
      gl.Disable(GL_STENCIL_TEST);

   // Every target is drawn upside down (see the vertex shader), which reverses
   // the winding Direct3D saw on screen: its clockwise is GL's counterclockwise.
   switch (rs[D3DRS_CULLMODE])
   {
   case D3DCULL_CW:
      gl.Enable(GL_CULL_FACE);
      gl.FrontFace(GL_CW);
      gl.CullFace(GL_BACK);
      break;
   case D3DCULL_CCW:
      gl.Enable(GL_CULL_FACE);
      gl.FrontFace(GL_CCW);
      gl.CullFace(GL_BACK);
      break;
   default:
      gl.Disable(GL_CULL_FACE);
      break;
   }

   gl.PolygonMode(GL_FRONT_AND_BACK, rs[D3DRS_FILLMODE] == D3DFILL_WIREFRAME ? GL_LINE
                                     : rs[D3DRS_FILLMODE] == D3DFILL_POINT  ? GL_POINT : GL_FILL);
}

void Device::ApplyAttributes()
{
   if (!attributesDirty)
      return;
   attributesDirty = false;

   bool enabled[4] = {};
   if (declaration != nullptr)
   {
      for (int i = 0; i < declaration->numElements; i++)
      {
         const D3DVERTEXELEMENT9 &e = declaration->elements[i];
         if (e.Stream >= NUM_STREAMS)
            continue;
         const Stream &s = streams[e.Stream];
         if (s.buffer == nullptr || s.buffer->store.glBuffer == 0)
            continue;

         int location;
         if (e.Usage == D3DDECLUSAGE_POSITION && e.UsageIndex == 0)
            location = ATTRIB_POSITION;
         else if (e.Usage == D3DDECLUSAGE_COLOR && e.UsageIndex == 0)
            location = ATTRIB_COLOR;
         else if (e.Usage == D3DDECLUSAGE_TEXCOORD && e.UsageIndex < 2)
            location = ATTRIB_TEX0 + e.UsageIndex;
         else
            continue;

         GLint size;
         GLenum type;
         GLboolean normalized;
         UINT bytes;
         if (!AttribFormat(e.Type, &size, &type, &normalized, &bytes))
            continue;
         gl.BindBuffer(GL_ARRAY_BUFFER, s.buffer->store.glBuffer);
         gl.VertexAttribPointer(location, size, type, normalized, s.stride,
                                reinterpret_cast<const void *>((uintptr_t) (s.offset + e.Offset)));
         gl.EnableVertexAttribArray(location);
         enabled[location] = true;
         attribStream[location] = e.Stream;
         attribEnd[location] = e.Offset + bytes;
      }
   }

   for (int loc = ATTRIB_POSITION; loc <= ATTRIB_TEX1; loc++)
   {
      attribEnabled[loc] = enabled[loc];
      if (!enabled[loc])
         gl.DisableVertexAttribArray(loc);
   }
   gl.Uniform1i(u.hasColor, enabled[ATTRIB_COLOR]);
   gl.Uniform1i(u.hasTexCoord0, enabled[ATTRIB_TEX0]);
   gl.Uniform1i(u.hasTexCoord1, enabled[ATTRIB_TEX1]);

   gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices ? indices->store.glBuffer : 0);
}

void Device::UploadVertices(UINT first, UINT last)
{
   for (int loc = ATTRIB_POSITION; loc <= ATTRIB_TEX1; loc++)
   {
      if (!attribEnabled[loc])
         continue;
      const Stream &s = streams[attribStream[loc]];
      BufferStore &store = s.buffer->store;
      uint64_t low = s.offset + (uint64_t) first * s.stride;
      uint64_t high = s.offset + (uint64_t) last * s.stride + attribEnd[loc];
      store.Ensure(gl, (UINT) std::min<uint64_t>(low, store.length),
                   (UINT) std::min<uint64_t>(high, store.length));
   }
}

/************************************************************************/
// IDirect3DDevice9: device, swap chain, resources

HRESULT Device::TestCooperativeLevel()
{
   // A lost device (its window's surface went away or was replaced) is reset
   // by creating a new one; the client does that on D3DERR_DEVICENOTRESET.
   return lost ? D3DERR_DEVICENOTRESET : D3D_OK;
}

UINT Device::GetAvailableTextureMem()
{
   // GL does not report it portably; budget the caches as for a 1 GB card.
   return 1024u * 1024u * 1024u;
}

HRESULT Device::GetDirect3D(IDirect3D9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   d3d->AddRef();
   *out = d3d;
   return D3D_OK;
}

HRESULT Device::GetDeviceCaps(D3DCAPS9 *caps)
{
   if (caps == nullptr)
      return D3DERR_INVALIDCALL;
   FillCaps(caps, maxTextureSize, maxAnisotropy);
   return D3D_OK;
}

HRESULT Device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *params)
{
   if (params == nullptr)
      return D3DERR_INVALIDCALL;
   params->AdapterOrdinal = 0;
   params->DeviceType = D3DDEVTYPE_HAL;
   params->hFocusWindow = focusWindow;
   params->BehaviorFlags = behaviorFlags;
   return D3D_OK;
}

HRESULT Device::Present(const RECT *src, const RECT *dst, HWND override, const RGNDATA *)
{
   if (lost)
      return D3DERR_DEVICELOST;
   if (SurfaceFor(window) != surface)
   {
      lost = true;
      return D3DERR_DEVICELOST;
   }
   GLSurface *out = override && override != window ? SurfaceFor(override) : surface;
   if (out == nullptr || !Begin())
      return D3D_OK;

   RECT s = src ? *src : RECT{ 0, 0, width, height };
   s.left = std::clamp<LONG>(s.left, 0, width);
   s.right = std::clamp<LONG>(s.right, 0, width);
   s.top = std::clamp<LONG>(s.top, 0, height);
   s.bottom = std::clamp<LONG>(s.bottom, 0, height);
   int w = s.right - s.left, h = s.bottom - s.top;
   if (w <= 0 || h <= 0)
      return D3D_OK;
   // Without a destination, the frame keeps its size at the client area's origin.
   RECT d = dst ? *dst : RECT{ 0, 0, w, h };

   int i = presentIndex;
   presentIndex ^= 1;
   GLuint &tex = presentTextures[i];
   if (tex == 0)
      gl.GenTextures(1, &tex);
   if (presentSize[i][0] != w || presentSize[i][1] != h)
   {
      BindForUpload(tex);
      gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      presentSize[i][0] = w;
      presentSize[i][1] = h;
   }

   // Equal sizes, so this also resolves a multisampled back buffer.  Rows count
   // from the top in both, so the texture has row 0 at the top.
   gl.BindFramebuffer(GL_READ_FRAMEBUFFER, backFramebuffer);
   gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, presentFramebuffer);
   gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
   gl.BlitFramebuffer(s.left, s.top, s.right, s.bottom, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
   gl.BindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
   gl.Flush();

   out->Present(tex, w, h, d);
   // The surface may have used the context; rebind everything on the next call.
   glValid = false;
   return D3D_OK;
}

HRESULT Device::GetBackBuffer(UINT swapChain, UINT index, D3DBACKBUFFER_TYPE type, IDirect3DSurface9 **out)
{
   if (out == nullptr || swapChain != 0 || index != 0 || type != D3DBACKBUFFER_TYPE_MONO)
      return D3DERR_INVALIDCALL;
   backSurface.AddRef();
   *out = &backSurface;
   return D3D_OK;
}

HRESULT Device::CreateTexture(UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool,
                              IDirect3DTexture9 **out, HANDLE *shared)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   PixelFormat f;
   if (w == 0 || h == 0 || shared != nullptr || levels > 1 || !LookupFormat(format, &f) ||
       w > (UINT) maxTextureSize || h > (UINT) maxTextureSize)
      return D3DERR_INVALIDCALL;
   bool target = (usage & D3DUSAGE_RENDERTARGET) != 0;
   if (target && (pool != D3DPOOL_DEFAULT || (format != D3DFMT_A8R8G8B8 && format != D3DFMT_X8R8G8B8)))
      return D3DERR_INVALIDCALL;
   if (pool != D3DPOOL_SYSTEMMEM && !Begin())
      return D3DERR_DEVICELOST;

   auto *t = new Texture;
   t->width = w;
   t->height = h;
   t->format = format;
   t->usage = usage;
   t->pool = pool;
   Track(t);

   if (pool != D3DPOOL_SYSTEMMEM)
   {
      gl.GenTextures(1, &t->glTexture);
      BindForUpload(t->glTexture);
      gl.TexImage2D(GL_TEXTURE_2D, 0, f.internal, w, h, 0, f.format, f.type, nullptr);
      gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      if (f.opaque)
         gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
      if (target)
      {
         gl.GenFramebuffers(1, &t->glFramebuffer);
         gl.BindFramebuffer(GL_FRAMEBUFFER, t->glFramebuffer);
         gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->glTexture, 0);
         gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
         gl.ClearColor(0, 0, 0, 0);
         gl.Clear(GL_COLOR_BUFFER_BIT);
         gl.BindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
         stateDirty = true;   // the color mask
         t->hasContent = true;
      }
   }
   *out = t;
   return D3D_OK;
}

HRESULT Device::CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool,
                                   IDirect3DVertexBuffer9 **out, HANDLE *shared)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   if (length == 0 || shared != nullptr)
      return D3DERR_INVALIDCALL;
   if (!Begin())
      return D3DERR_DEVICELOST;
   auto *b = new VertexBuffer;
   b->usage = usage;
   b->fvf = fvf;
   b->pool = pool;
   b->store.length = length;
   b->store.memory.reset(new BYTE[length]());
   gl.GenBuffers(1, &b->store.glBuffer);
   gl.BindBuffer(GL_COPY_WRITE_BUFFER, b->store.glBuffer);
   gl.BufferData(GL_COPY_WRITE_BUFFER, length, nullptr, GL_STREAM_DRAW);
   Track(b);
   *out = b;
   return D3D_OK;
}

HRESULT Device::CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool,
                                  IDirect3DIndexBuffer9 **out, HANDLE *shared)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   if (length == 0 || shared != nullptr || (format != D3DFMT_INDEX16 && format != D3DFMT_INDEX32))
      return D3DERR_INVALIDCALL;
   if (!Begin())
      return D3DERR_DEVICELOST;
   auto *b = new IndexBuffer;
   b->usage = usage;
   b->format = format;
   b->pool = pool;
   b->store.length = length;
   b->store.memory.reset(new BYTE[length]());
   gl.GenBuffers(1, &b->store.glBuffer);
   // The element array binding belongs to the VAO; upload through another target.
   gl.BindBuffer(GL_COPY_WRITE_BUFFER, b->store.glBuffer);
   gl.BufferData(GL_COPY_WRITE_BUFFER, length, nullptr, GL_STREAM_DRAW);
   Track(b);
   *out = b;
   return D3D_OK;
}

HRESULT Device::UpdateTexture(IDirect3DBaseTexture9 *source, IDirect3DBaseTexture9 *dest)
{
   if (source == nullptr || dest == nullptr || source->GetType() != D3DRTYPE_TEXTURE ||
       dest->GetType() != D3DRTYPE_TEXTURE)
      return D3DERR_INVALIDCALL;
   auto *src = static_cast<Texture *>(source);
   auto *dst = static_cast<Texture *>(dest);
   PixelFormat f;
   if (dst->glTexture == 0 || src->format != dst->format || src->width != dst->width ||
       src->height != dst->height || !LookupFormat(src->format, &f))
      return D3DERR_INVALIDCALL;
   if (!Begin())
      return D3D_OK;

   if (src->glTexture == 0)
   {
      if (src->staging == nullptr)
         return D3D_OK;   // never written
      BindForUpload(dst->glTexture);
      gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
      gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dst->width, dst->height, f.format, f.type, src->staging.get());
   }
   else
   {
      GLuint fbo[2];
      gl.GenFramebuffers(2, fbo);
      gl.BindFramebuffer(GL_READ_FRAMEBUFFER, fbo[0]);
      gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src->glTexture, 0);
      gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
      gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst->glTexture, 0);
      gl.BlitFramebuffer(0, 0, src->width, src->height, 0, 0, dst->width, dst->height,
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);
      gl.DeleteFramebuffers(2, fbo);
      gl.BindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
   }
   dst->hasContent = true;
   return D3D_OK;
}

HRESULT Device::StretchRect(IDirect3DSurface9 *source, const RECT *sourceRect, IDirect3DSurface9 *dest,
                            const RECT *destRect, D3DTEXTUREFILTERTYPE filter)
{
   auto *src = static_cast<Surface *>(source);
   auto *dst = static_cast<Surface *>(dest);
   if (src == nullptr || dst == nullptr || src == dst || src->kind == Surface::DEPTH_STENCIL ||
       dst->kind == Surface::DEPTH_STENCIL)
      return D3DERR_INVALIDCALL;
   Texture *st = src->texture, *dt = dst->texture;
   if ((st && !st->glFramebuffer) || (dt && !dt->glFramebuffer))
      return D3DERR_INVALIDCALL;   // only render targets
   if (!Begin())
      return D3D_OK;

   int sw = st ? st->width : width, sh = st ? st->height : height;
   int dw = dt ? dt->width : width, dh = dt ? dt->height : height;
   RECT s = sourceRect ? *sourceRect : RECT{ 0, 0, sw, sh };
   RECT d = destRect ? *destRect : RECT{ 0, 0, dw, dh };
   bool same = s.right - s.left == d.right - d.left && s.bottom - s.top == d.bottom - d.top;

   gl.BindFramebuffer(GL_READ_FRAMEBUFFER, st ? st->glFramebuffer : backFramebuffer);
   gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, dt ? dt->glFramebuffer : backFramebuffer);
   // Rows count from the top in both, as in Direct3D.
   gl.BlitFramebuffer(s.left, s.top, s.right, s.bottom, d.left, d.top, d.right, d.bottom, GL_COLOR_BUFFER_BIT,
                      !same && filter == D3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
   gl.BindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
   if (dt)
      dt->hasContent = true;
   return D3D_OK;
}

HRESULT Device::SetRenderTarget(DWORD index, IDirect3DSurface9 *target)
{
   auto *s = static_cast<Surface *>(target);
   if (index != 0 || s == nullptr || s->kind == Surface::DEPTH_STENCIL)
      return D3DERR_INVALIDCALL;
   if (s->kind == Surface::BACK_BUFFER)
   {
      currentTarget = &backSurface;
      BindTarget(backFramebuffer, width, height);
      return D3D_OK;
   }
   Texture *t = s->texture;
   if (t->glFramebuffer == 0)
      return D3DERR_INVALIDCALL;
   currentTarget = s;
   t->hasContent = true;
   BindTarget(t->glFramebuffer, t->width, t->height);
   return D3D_OK;
}

HRESULT Device::GetRenderTarget(DWORD index, IDirect3DSurface9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   if (index != 0)
      return D3DERR_NOTFOUND;
   currentTarget->AddRef();
   *out = currentTarget;
   return D3D_OK;
}

HRESULT Device::SetDepthStencilSurface(IDirect3DSurface9 *s)
{
   if (s != nullptr && s != &depthSurface)
      return D3DERR_INVALIDCALL;
   // The back buffer keeps its own depth buffer; texture targets draw without one.
   depthEnabled = s != nullptr;
   stateDirty = true;
   return D3D_OK;
}

HRESULT Device::GetDepthStencilSurface(IDirect3DSurface9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   if (!depthEnabled)
      return D3DERR_NOTFOUND;
   depthSurface.AddRef();
   *out = &depthSurface;
   return D3D_OK;
}

/************************************************************************/
// IDirect3DDevice9: drawing

HRESULT Device::BeginScene()
{
   Begin();
   return D3D_OK;
}

HRESULT Device::EndScene()
{
   return D3D_OK;
}

HRESULT Device::Clear(DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
   if (!Begin())
      return D3D_OK;
   GLbitfield mask = 0;
   if (flags & D3DCLEAR_TARGET)
   {
      gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      gl.ClearColor(((color >> 16) & 0xff) / 255.0f, ((color >> 8) & 0xff) / 255.0f, (color & 0xff) / 255.0f,
                    ((color >> 24) & 0xff) / 255.0f);
      mask |= GL_COLOR_BUFFER_BIT;
   }
   if ((flags & D3DCLEAR_ZBUFFER) && depthEnabled)
   {
      gl.DepthMask(GL_TRUE);
      gl.ClearDepth(z);
      mask |= GL_DEPTH_BUFFER_BIT;
   }
   if ((flags & D3DCLEAR_STENCIL) && depthEnabled)
   {
      gl.StencilMask(0xff);
      gl.ClearStencil(stencil);
      mask |= GL_STENCIL_BUFFER_BIT;
   }
   stateDirty = true;   // the masks
   if (mask == 0)
      return D3D_OK;

   // Direct3D clears the viewport, or the given rectangles within it.
   RECT vp = { (LONG) viewport.X, (LONG) viewport.Y, (LONG) (viewport.X + viewport.Width),
               (LONG) (viewport.Y + viewport.Height) };
   auto clearRect = [&](const RECT &r) {
      if (r.left >= r.right || r.top >= r.bottom)
         return;
      bool whole = r.left <= 0 && r.top <= 0 && r.right >= targetWidth && r.bottom >= targetHeight;
      if (whole)
         gl.Disable(GL_SCISSOR_TEST);
      else
      {
         gl.Enable(GL_SCISSOR_TEST);
         gl.Scissor(r.left, r.top, r.right - r.left, r.bottom - r.top);
      }
      gl.Clear(mask);
   };
   if (count == 0 || rects == nullptr)
      clearRect(vp);
   else
      for (DWORD i = 0; i < count; i++)
         clearRect(RECT{ std::max(rects[i].x1, vp.left), std::max(rects[i].y1, vp.top),
                         std::min(rects[i].x2, vp.right), std::min(rects[i].y2, vp.bottom) });
   gl.Disable(GL_SCISSOR_TEST);
   return D3D_OK;
}

HRESULT Device::DrawPrimitive(D3DPRIMITIVETYPE type, UINT start, UINT primCount)
{
   GLenum mode;
   UINT count;
   if (!Primitive(type, primCount, &mode, &count) || declaration == nullptr)
      return D3DERR_INVALIDCALL;
   if (primCount == 0 || !Begin())
      return D3D_OK;
   ApplyState();
   ApplyTextures();
   ApplyAttributes();
   UploadVertices(start, start + count - 1);
   gl.DrawArrays(mode, start, count);
   return D3D_OK;
}

HRESULT Device::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertex, UINT, UINT, UINT startIndex,
                                     UINT primCount)
{
   GLenum mode;
   UINT count;
   if (!Primitive(type, primCount, &mode, &count) || indices == nullptr || declaration == nullptr)
      return D3DERR_INVALIDCALL;
   bool wide = indices->format == D3DFMT_INDEX32;
   UINT indexSize = wide ? 4 : 2;
   uint64_t firstByte = (uint64_t) startIndex * indexSize;
   uint64_t endByte = firstByte + (uint64_t) count * indexSize;
   if (endByte > indices->store.length)
      return D3DERR_INVALIDCALL;
   if (primCount == 0 || !Begin())
      return D3D_OK;

   ApplyState();
   ApplyTextures();
   ApplyAttributes();

   // The vertices the indices name, from the CPU copy; MinVertexIndex and
   // NumVertices are only hints (the client's are not exact).
   UINT lo = UINT32_MAX, hi = 0;
   const BYTE *p = indices->store.memory.get() + firstByte;
   if (wide)
      for (UINT i = 0; i < count; i++)
      {
         uint32_t v;
         memcpy(&v, p + 4 * i, 4);
         lo = std::min(lo, v);
         hi = std::max(hi, v);
      }
   else
      for (UINT i = 0; i < count; i++)
      {
         uint16_t v;
         memcpy(&v, p + 2 * i, 2);
         lo = std::min<UINT>(lo, v);
         hi = std::max<UINT>(hi, v);
      }
   int64_t first = std::max<int64_t>(0, (int64_t) baseVertex + lo);
   int64_t last = std::max<int64_t>(0, (int64_t) baseVertex + hi);
   UploadVertices((UINT) first, (UINT) last);
   indices->store.Ensure(gl, (UINT) firstByte, (UINT) endByte);

   gl.DrawElementsBaseVertex(mode, count, wide ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
                             reinterpret_cast<const void *>((uintptr_t) firstByte), baseVertex);
   return D3D_OK;
}

/************************************************************************/
// IDirect3DDevice9: state

HRESULT Device::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix)
{
   if (matrix == nullptr)
      return D3DERR_INVALIDCALL;
   switch ((DWORD) state)   // D3DTS_WORLD is outside the enum
   {
   case D3DTS_WORLD:      world = *matrix; break;
   case D3DTS_VIEW:       view = *matrix; break;
   case D3DTS_PROJECTION: proj = *matrix; break;
   default:               return D3D_OK;   // texture and other world matrices: unused
   }
   transformsDirty = true;
   return D3D_OK;
}

HRESULT Device::GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix)
{
   if (matrix == nullptr)
      return D3DERR_INVALIDCALL;
   switch ((DWORD) state)   // D3DTS_WORLD is outside the enum
   {
   case D3DTS_WORLD:      *matrix = world; break;
   case D3DTS_VIEW:       *matrix = view; break;
   case D3DTS_PROJECTION: *matrix = proj; break;
   default:               Identity(matrix); break;
   }
   return D3D_OK;
}

HRESULT Device::SetViewport(const D3DVIEWPORT9 *v)
{
   if (v == nullptr || v->X + v->Width > (DWORD) targetWidth || v->Y + v->Height > (DWORD) targetHeight)
      return D3DERR_INVALIDCALL;
   viewport = *v;
   if (Begin())
      ApplyViewport();
   return D3D_OK;
}

HRESULT Device::GetViewport(D3DVIEWPORT9 *v)
{
   if (v == nullptr)
      return D3DERR_INVALIDCALL;
   *v = viewport;
   return D3D_OK;
}

HRESULT Device::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
   if ((unsigned) state >= NUM_RENDER_STATES)
      return D3DERR_INVALIDCALL;
   if (renderState[state] != value)
   {
      renderState[state] = value;
      stateDirty = true;
   }
   return D3D_OK;
}

HRESULT Device::GetRenderState(D3DRENDERSTATETYPE state, DWORD *value)
{
   if ((unsigned) state >= NUM_RENDER_STATES || value == nullptr)
      return D3DERR_INVALIDCALL;
   *value = renderState[state];
   return D3D_OK;
}

HRESULT Device::GetTexture(DWORD i, IDirect3DBaseTexture9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = i < NUM_STAGES ? texture[i] : nullptr;
   if (*out != nullptr)
      (*out)->AddRef();
   return D3D_OK;
}

HRESULT Device::SetTexture(DWORD i, IDirect3DBaseTexture9 *t)
{
   if (t != nullptr && t->GetType() != D3DRTYPE_TEXTURE)
      return D3DERR_INVALIDCALL;   // only 2D textures exist here
   if (i >= NUM_STAGES)
      return D3D_OK;   // stages the shader does not combine
   if (texture[i] != t)
   {
      texture[i] = t;
      texturesDirty = true;
   }
   return D3D_OK;
}

HRESULT Device::GetTextureStageState(DWORD i, D3DTEXTURESTAGESTATETYPE type, DWORD *value)
{
   if (i >= 8 || type < 1 || type > 32 || value == nullptr)
      return D3DERR_INVALIDCALL;
   *value = stage[i].value[type];
   return D3D_OK;
}

HRESULT Device::SetTextureStageState(DWORD i, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
   if (i >= 8 || type < 1 || type > 32)
      return D3DERR_INVALIDCALL;
   if (stage[i].value[type] != value)
   {
      stage[i].value[type] = value;
      if (i < NUM_STAGES)
         stateDirty = true;
   }
   return D3D_OK;
}

namespace {

DWORD *SamplerField(SamplerState &s, D3DSAMPLERSTATETYPE type)
{
   switch (type)
   {
   case D3DSAMP_ADDRESSU:      return &s.addressU;
   case D3DSAMP_ADDRESSV:      return &s.addressV;
   case D3DSAMP_ADDRESSW:      return &s.addressW;
   case D3DSAMP_BORDERCOLOR:   return &s.borderColor;
   case D3DSAMP_MAGFILTER:     return &s.magFilter;
   case D3DSAMP_MINFILTER:     return &s.minFilter;
   case D3DSAMP_MIPFILTER:     return &s.mipFilter;
   case D3DSAMP_MIPMAPLODBIAS: return &s.mipLodBias;
   case D3DSAMP_MAXMIPLEVEL:   return &s.maxMipLevel;
   case D3DSAMP_MAXANISOTROPY: return &s.maxAnisotropy;
   case D3DSAMP_SRGBTEXTURE:   return &s.srgbTexture;
   default:                    return nullptr;
   }
}

}  // namespace

HRESULT Device::GetSamplerState(DWORD i, D3DSAMPLERSTATETYPE type, DWORD *value)
{
   DWORD *field = i < NUM_SAMPLERS ? SamplerField(sampler[i], type) : nullptr;
   if (value == nullptr)
      return D3DERR_INVALIDCALL;
   *value = field ? *field : 0;
   return D3D_OK;
}

HRESULT Device::SetSamplerState(DWORD i, D3DSAMPLERSTATETYPE type, DWORD value)
{
   DWORD *field = i < NUM_SAMPLERS ? SamplerField(sampler[i], type) : nullptr;
   if (field != nullptr && *field != value)
   {
      *field = value;
      sampler[i].dirty = true;
   }
   return D3D_OK;
}

HRESULT Device::ValidateDevice(DWORD *passes)
{
   if (passes == nullptr)
      return D3DERR_INVALIDCALL;
   *passes = 1;
   return D3D_OK;
}

HRESULT Device::CreateVertexDeclaration(const D3DVERTEXELEMENT9 *elements, IDirect3DVertexDeclaration9 **out)
{
   if (out == nullptr || elements == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   int n = 0;
   while (elements[n].Stream != 0xFF)
      if (++n > VertexDeclaration::MAX_ELEMENTS)
         return D3DERR_INVALIDCALL;
   auto *d = new VertexDeclaration;
   std::copy(elements, elements + n, d->elements);
   d->numElements = n;
   Track(d);
   *out = d;
   return D3D_OK;
}

HRESULT Device::SetVertexDeclaration(IDirect3DVertexDeclaration9 *decl)
{
   auto *d = static_cast<VertexDeclaration *>(decl);
   if (declaration != d)
   {
      declaration = d;
      attributesDirty = true;
   }
   return D3D_OK;
}

HRESULT Device::GetVertexDeclaration(IDirect3DVertexDeclaration9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = declaration;
   if (declaration != nullptr)
      declaration->AddRef();
   return D3D_OK;
}

HRESULT Device::SetVertexShader(IDirect3DVertexShader9 *shader)
{
   // Fixed function only; no shader objects are ever created.
   return shader == nullptr ? D3D_OK : D3DERR_INVALIDCALL;
}

HRESULT Device::GetVertexShader(IDirect3DVertexShader9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   return D3D_OK;
}

HRESULT Device::SetPixelShader(IDirect3DPixelShader9 *shader)
{
   return shader == nullptr ? D3D_OK : D3DERR_INVALIDCALL;
}

HRESULT Device::GetPixelShader(IDirect3DPixelShader9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   return D3D_OK;
}

HRESULT Device::SetStreamSource(UINT n, IDirect3DVertexBuffer9 *buffer, UINT offset, UINT stride)
{
   if (n >= NUM_STREAMS)
      return D3DERR_INVALIDCALL;
   Stream s = { static_cast<VertexBuffer *>(buffer), offset, stride };
   Stream &cur = streams[n];
   if (cur.buffer != s.buffer || cur.offset != s.offset || cur.stride != s.stride)
   {
      cur = s;
      attributesDirty = true;
   }
   return D3D_OK;
}

HRESULT Device::GetStreamSource(UINT n, IDirect3DVertexBuffer9 **buffer, UINT *offset, UINT *stride)
{
   if (n >= NUM_STREAMS || buffer == nullptr || offset == nullptr || stride == nullptr)
      return D3DERR_INVALIDCALL;
   *buffer = streams[n].buffer;
   if (*buffer != nullptr)
      (*buffer)->AddRef();
   *offset = streams[n].offset;
   *stride = streams[n].stride;
   return D3D_OK;
}

HRESULT Device::SetIndices(IDirect3DIndexBuffer9 *buffer)
{
   auto *b = static_cast<IndexBuffer *>(buffer);
   if (indices != b)
   {
      indices = b;
      attributesDirty = true;
   }
   return D3D_OK;
}

HRESULT Device::GetIndices(IDirect3DIndexBuffer9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = indices;
   if (indices != nullptr)
      indices->AddRef();
   return D3D_OK;
}

}  // namespace w32::gfx

/*
 * gl.h:  The OpenGL 3.3 core subset Direct3D 9 renders with, loaded through
 *   GLSurface::GetProcAddress.  Declared here rather than taken from the
 *   platform's headers, which differ (Mesa's glcorearb.h, macOS's gl3.h).
 */

#ifndef W32_GFX_GL_H
#define W32_GFX_GL_H

#include <cstddef>

namespace w32 {
class GLSurface;
}

namespace w32::gfx {

typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef int            GLint;
typedef unsigned int   GLuint;
typedef int            GLsizei;
typedef float          GLfloat;
typedef double         GLdouble;
typedef char           GLchar;
typedef unsigned char  GLubyte;
typedef std::ptrdiff_t GLsizeiptr;
typedef std::ptrdiff_t GLintptr;

constexpr GLenum GL_NO_ERROR = 0;
constexpr GLboolean GL_FALSE = 0;
constexpr GLboolean GL_TRUE = 1;

constexpr GLenum GL_POINTS = 0x0000;
constexpr GLenum GL_LINES = 0x0001;
constexpr GLenum GL_LINE_STRIP = 0x0003;
constexpr GLenum GL_TRIANGLES = 0x0004;
constexpr GLenum GL_TRIANGLE_STRIP = 0x0005;
constexpr GLenum GL_TRIANGLE_FAN = 0x0006;

constexpr GLenum GL_NEVER = 0x0200;
constexpr GLenum GL_LESS = 0x0201;
constexpr GLenum GL_EQUAL = 0x0202;
constexpr GLenum GL_LEQUAL = 0x0203;
constexpr GLenum GL_GREATER = 0x0204;
constexpr GLenum GL_NOTEQUAL = 0x0205;
constexpr GLenum GL_GEQUAL = 0x0206;
constexpr GLenum GL_ALWAYS = 0x0207;

constexpr GLenum GL_ZERO = 0;
constexpr GLenum GL_ONE = 1;
constexpr GLenum GL_SRC_COLOR = 0x0300;
constexpr GLenum GL_ONE_MINUS_SRC_COLOR = 0x0301;
constexpr GLenum GL_SRC_ALPHA = 0x0302;
constexpr GLenum GL_ONE_MINUS_SRC_ALPHA = 0x0303;
constexpr GLenum GL_DST_ALPHA = 0x0304;
constexpr GLenum GL_ONE_MINUS_DST_ALPHA = 0x0305;
constexpr GLenum GL_DST_COLOR = 0x0306;
constexpr GLenum GL_ONE_MINUS_DST_COLOR = 0x0307;
constexpr GLenum GL_SRC_ALPHA_SATURATE = 0x0308;
constexpr GLenum GL_FUNC_ADD = 0x8006;
constexpr GLenum GL_MIN = 0x8007;
constexpr GLenum GL_MAX = 0x8008;
constexpr GLenum GL_FUNC_SUBTRACT = 0x800A;
constexpr GLenum GL_FUNC_REVERSE_SUBTRACT = 0x800B;

constexpr GLenum GL_KEEP = 0x1E00;
constexpr GLenum GL_REPLACE = 0x1E01;
constexpr GLenum GL_INCR = 0x1E02;
constexpr GLenum GL_DECR = 0x1E03;
constexpr GLenum GL_INVERT = 0x150A;
constexpr GLenum GL_INCR_WRAP = 0x8507;
constexpr GLenum GL_DECR_WRAP = 0x8508;

constexpr GLenum GL_FRONT = 0x0404;
constexpr GLenum GL_BACK = 0x0405;
constexpr GLenum GL_FRONT_AND_BACK = 0x0408;
constexpr GLenum GL_CW = 0x0900;
constexpr GLenum GL_CCW = 0x0901;
constexpr GLenum GL_POINT = 0x1B00;
constexpr GLenum GL_LINE = 0x1B01;
constexpr GLenum GL_FILL = 0x1B02;

constexpr GLenum GL_CULL_FACE = 0x0B44;
constexpr GLenum GL_DEPTH_TEST = 0x0B71;
constexpr GLenum GL_STENCIL_TEST = 0x0B90;
constexpr GLenum GL_DITHER = 0x0BD0;
constexpr GLenum GL_BLEND = 0x0BE2;
constexpr GLenum GL_SCISSOR_TEST = 0x0C11;
constexpr GLenum GL_POLYGON_OFFSET_FILL = 0x8037;
constexpr GLenum GL_MULTISAMPLE = 0x809D;
constexpr GLenum GL_FRAMEBUFFER_SRGB = 0x8DB9;

constexpr GLbitfield GL_DEPTH_BUFFER_BIT = 0x0100;
constexpr GLbitfield GL_STENCIL_BUFFER_BIT = 0x0400;
constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x4000;

constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
constexpr GLenum GL_UNSIGNED_SHORT = 0x1403;
constexpr GLenum GL_UNSIGNED_INT = 0x1405;
constexpr GLenum GL_FLOAT = 0x1406;
constexpr GLenum GL_UNSIGNED_SHORT_5_6_5 = 0x8363;
constexpr GLenum GL_UNSIGNED_SHORT_4_4_4_4_REV = 0x8365;
constexpr GLenum GL_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366;
constexpr GLenum GL_UNSIGNED_INT_8_8_8_8_REV = 0x8367;

constexpr GLenum GL_RGB = 0x1907;
constexpr GLenum GL_RGBA = 0x1908;
constexpr GLenum GL_BGR = 0x80E0;
constexpr GLenum GL_BGRA = 0x80E1;
constexpr GLenum GL_RGBA4 = 0x8056;
constexpr GLenum GL_RGB5_A1 = 0x8057;
constexpr GLenum GL_RGB8 = 0x8051;
constexpr GLenum GL_RGBA8 = 0x8058;
constexpr GLenum GL_RGB565 = 0x8D62;
constexpr GLenum GL_DEPTH24_STENCIL8 = 0x88F0;

constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
constexpr GLenum GL_TEXTURE0 = 0x84C0;
constexpr GLenum GL_TEXTURE_BORDER_COLOR = 0x1004;
constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
constexpr GLenum GL_TEXTURE_WRAP_S = 0x2802;
constexpr GLenum GL_TEXTURE_WRAP_T = 0x2803;
constexpr GLenum GL_TEXTURE_BASE_LEVEL = 0x813C;
constexpr GLenum GL_TEXTURE_MAX_LEVEL = 0x813D;
constexpr GLenum GL_TEXTURE_SWIZZLE_A = 0x8E45;
constexpr GLenum GL_TEXTURE_MAX_ANISOTROPY = 0x84FE;
constexpr GLenum GL_MAX_TEXTURE_MAX_ANISOTROPY = 0x84FF;
constexpr GLenum GL_NEAREST = 0x2600;
constexpr GLenum GL_LINEAR = 0x2601;
constexpr GLenum GL_REPEAT = 0x2901;
constexpr GLenum GL_CLAMP_TO_BORDER = 0x812D;
constexpr GLenum GL_CLAMP_TO_EDGE = 0x812F;
constexpr GLenum GL_MIRRORED_REPEAT = 0x8370;

constexpr GLenum GL_UNPACK_ROW_LENGTH = 0x0CF2;
constexpr GLenum GL_UNPACK_SKIP_ROWS = 0x0CF3;
constexpr GLenum GL_UNPACK_SKIP_PIXELS = 0x0CF4;
constexpr GLenum GL_UNPACK_ALIGNMENT = 0x0CF5;
constexpr GLenum GL_PACK_ROW_LENGTH = 0x0D02;
constexpr GLenum GL_PACK_ALIGNMENT = 0x0D05;

constexpr GLenum GL_MAX_TEXTURE_SIZE = 0x0D33;
constexpr GLenum GL_VENDOR = 0x1F00;
constexpr GLenum GL_RENDERER = 0x1F01;
constexpr GLenum GL_VERSION = 0x1F02;
constexpr GLenum GL_EXTENSIONS = 0x1F03;
constexpr GLenum GL_MAJOR_VERSION = 0x821B;
constexpr GLenum GL_MINOR_VERSION = 0x821C;
constexpr GLenum GL_NUM_EXTENSIONS = 0x821D;
constexpr GLenum GL_MAX_SAMPLES = 0x8D57;

constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
constexpr GLenum GL_ELEMENT_ARRAY_BUFFER = 0x8893;
constexpr GLenum GL_COPY_WRITE_BUFFER = 0x8F37;
constexpr GLenum GL_STREAM_DRAW = 0x88E0;

constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
constexpr GLenum GL_LINK_STATUS = 0x8B82;

constexpr GLenum GL_FRAMEBUFFER = 0x8D40;
constexpr GLenum GL_READ_FRAMEBUFFER = 0x8CA8;
constexpr GLenum GL_DRAW_FRAMEBUFFER = 0x8CA9;
constexpr GLenum GL_RENDERBUFFER = 0x8D41;
constexpr GLenum GL_COLOR_ATTACHMENT0 = 0x8CE0;
constexpr GLenum GL_DEPTH_STENCIL_ATTACHMENT = 0x821A;
constexpr GLenum GL_FRAMEBUFFER_COMPLETE = 0x8CD5;

// X(return type, name without the gl prefix, parameters)
#define W32_GL_FUNCTIONS(X) \
   X(GLenum, GetError, (void)) \
   X(void, GetIntegerv, (GLenum pname, GLint *data)) \
   X(void, GetFloatv, (GLenum pname, GLfloat *data)) \
   X(const GLubyte *, GetString, (GLenum name)) \
   X(const GLubyte *, GetStringi, (GLenum name, GLuint index)) \
   X(void, Enable, (GLenum cap)) \
   X(void, Disable, (GLenum cap)) \
   X(void, DepthFunc, (GLenum func)) \
   X(void, DepthMask, (GLboolean flag)) \
   X(void, DepthRange, (GLdouble n, GLdouble f)) \
   X(void, BlendFunc, (GLenum sfactor, GLenum dfactor)) \
   X(void, BlendEquation, (GLenum mode)) \
   X(void, ColorMask, (GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
   X(void, StencilFunc, (GLenum func, GLint ref, GLuint mask)) \
   X(void, StencilOp, (GLenum fail, GLenum zfail, GLenum zpass)) \
   X(void, StencilMask, (GLuint mask)) \
   X(void, CullFace, (GLenum mode)) \
   X(void, FrontFace, (GLenum mode)) \
   X(void, PolygonMode, (GLenum face, GLenum mode)) \
   X(void, PolygonOffset, (GLfloat factor, GLfloat units)) \
   X(void, Viewport, (GLint x, GLint y, GLsizei width, GLsizei height)) \
   X(void, Scissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
   X(void, ClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a)) \
   X(void, ClearDepth, (GLdouble depth)) \
   X(void, ClearStencil, (GLint s)) \
   X(void, Clear, (GLbitfield mask)) \
   X(void, Flush, (void)) \
   X(GLuint, CreateShader, (GLenum type)) \
   X(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length)) \
   X(void, CompileShader, (GLuint shader)) \
   X(void, GetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
   X(void, GetShaderInfoLog, (GLuint shader, GLsizei size, GLsizei *length, GLchar *log)) \
   X(void, DeleteShader, (GLuint shader)) \
   X(GLuint, CreateProgram, (void)) \
   X(void, AttachShader, (GLuint program, GLuint shader)) \
   X(void, LinkProgram, (GLuint program)) \
   X(void, GetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
   X(void, GetProgramInfoLog, (GLuint program, GLsizei size, GLsizei *length, GLchar *log)) \
   X(void, DeleteProgram, (GLuint program)) \
   X(void, UseProgram, (GLuint program)) \
   X(GLint, GetUniformLocation, (GLuint program, const GLchar *name)) \
   X(void, Uniform1i, (GLint location, GLint v0)) \
   X(void, Uniform1f, (GLint location, GLfloat v0)) \
   X(void, Uniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
   X(void, Uniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
   X(void, UniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
   X(void, GenVertexArrays, (GLsizei n, GLuint *arrays)) \
   X(void, DeleteVertexArrays, (GLsizei n, const GLuint *arrays)) \
   X(void, BindVertexArray, (GLuint array)) \
   X(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)) \
   X(void, EnableVertexAttribArray, (GLuint index)) \
   X(void, DisableVertexAttribArray, (GLuint index)) \
   X(void, GenBuffers, (GLsizei n, GLuint *buffers)) \
   X(void, DeleteBuffers, (GLsizei n, const GLuint *buffers)) \
   X(void, BindBuffer, (GLenum target, GLuint buffer)) \
   X(void, BufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage)) \
   X(void, BufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data)) \
   X(void, GenTextures, (GLsizei n, GLuint *textures)) \
   X(void, DeleteTextures, (GLsizei n, const GLuint *textures)) \
   X(void, BindTexture, (GLenum target, GLuint texture)) \
   X(void, ActiveTexture, (GLenum texture)) \
   X(void, TexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
   X(void, TexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
   X(void, TexParameteri, (GLenum target, GLenum pname, GLint param)) \
   X(void, GetTexImage, (GLenum target, GLint level, GLenum format, GLenum type, void *pixels)) \
   X(void, PixelStorei, (GLenum pname, GLint param)) \
   X(void, GenSamplers, (GLsizei count, GLuint *samplers)) \
   X(void, DeleteSamplers, (GLsizei count, const GLuint *samplers)) \
   X(void, BindSampler, (GLuint unit, GLuint sampler)) \
   X(void, SamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
   X(void, SamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
   X(void, SamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat *param)) \
   X(void, GenFramebuffers, (GLsizei n, GLuint *framebuffers)) \
   X(void, DeleteFramebuffers, (GLsizei n, const GLuint *framebuffers)) \
   X(void, BindFramebuffer, (GLenum target, GLuint framebuffer)) \
   X(void, FramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
   X(void, FramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
   X(GLenum, CheckFramebufferStatus, (GLenum target)) \
   X(void, BlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
   X(void, GenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
   X(void, DeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers)) \
   X(void, BindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
   X(void, RenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
   X(void, RenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
   X(void, DrawArrays, (GLenum mode, GLint first, GLsizei count)) \
   X(void, DrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex))

struct GL
{
#define W32_GL_MEMBER(ret, name, params) ret (*name) params = nullptr;
   W32_GL_FUNCTIONS(W32_GL_MEMBER)
#undef W32_GL_MEMBER

   // Fills in every function from the surface; false (and logs the first
   // missing one) if any is unavailable.
   bool Load(GLSurface *surface);
};

}  // namespace w32::gfx

#endif

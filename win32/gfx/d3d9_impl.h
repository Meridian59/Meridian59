/*
 * d3d9_impl.h:  The Direct3D 9 objects on OpenGL 3.3 core: classes deriving from
 *   Wine's d3d9.h interfaces, so the Windows sources call them through the usual
 *   vtables.  Only the fixed-function subset the client renders with does
 *   anything; the rest fails with D3DERR_INVALIDCALL.
 *
 *   Conventions carried over from Direct3D 9:
 *   - Matrices are row-major with row vectors (v' = v * World * View * Proj).
 *   - Clip-space depth runs 0..1; DEPTHBIAS is added to that depth.
 *   - Every render target stores its top row first (GL row 0 is Direct3D row 0),
 *     so render-to-texture output samples with the texture coordinates uploaded
 *     bitmaps use, and Present hands the backend a texture with row 0 at the top.
 */

#ifndef W32_GFX_D3D9_IMPL_H
#define W32_GFX_D3D9_IMPL_H

#include "core/win32.h"
#include "core/glsurface.h"
#include "gfx/gl.h"

#include <d3d9.h>

#include <cstring>
#include <memory>

// Wine marks the interface methods nothrow (__attribute__((nothrow))), which
// clang reads as noexcept and wants repeated on every override; they do not
// throw.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-exception-spec"
#endif

namespace w32::gfx {

class Device;

// GUIDs from d3d9.h; the headers only declare them (no dxguid here).
inline constexpr GUID kIID_IUnknown = { 0x00000000, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
inline constexpr GUID kIID_IDirect3D9 = { 0x81bdcbca, 0x64d4, 0x426d, { 0xae, 0x8d, 0xad, 0x01, 0x47, 0xf4, 0x27, 0x5c } };
inline constexpr GUID kIID_IDirect3DDevice9 = { 0xd0223b96, 0xbf7a, 0x43fd, { 0x92, 0xbd, 0xa4, 0x3b, 0x0d, 0x82, 0xb9, 0xeb } };
inline constexpr GUID kIID_IDirect3DResource9 = { 0x05eec05d, 0x8f7d, 0x4362, { 0xb9, 0x99, 0xd1, 0xba, 0xf3, 0x57, 0xc7, 0x04 } };
inline constexpr GUID kIID_IDirect3DBaseTexture9 = { 0x580ca87e, 0x1d3c, 0x4d54, { 0x99, 0x1d, 0xb7, 0xd3, 0xe3, 0xc2, 0x98, 0xce } };
inline constexpr GUID kIID_IDirect3DTexture9 = { 0x85c31227, 0x3de5, 0x4f00, { 0x9b, 0x3a, 0xf1, 0x1a, 0xc3, 0x8c, 0x18, 0xb5 } };
inline constexpr GUID kIID_IDirect3DSurface9 = { 0x0cfbaf3a, 0x9ff6, 0x429a, { 0x99, 0xb3, 0xa2, 0x79, 0x6a, 0xf8, 0xb8, 0x9b } };
inline constexpr GUID kIID_IDirect3DVertexBuffer9 = { 0xb64bb1b5, 0xfd70, 0x4df6, { 0xbf, 0x91, 0x19, 0xd0, 0xa1, 0x24, 0x55, 0xe3 } };
inline constexpr GUID kIID_IDirect3DIndexBuffer9 = { 0x7c9dd65e, 0xd3f7, 0x4529, { 0xac, 0xee, 0x78, 0x58, 0x30, 0xac, 0xde, 0x35 } };
inline constexpr GUID kIID_IDirect3DVertexDeclaration9 = { 0xdd13c59c, 0x36fa, 0x4098, { 0xa8, 0xfb, 0xc7, 0xed, 0x39, 0xdc, 0x85, 0x46 } };

inline bool SameIID(REFIID a, const GUID &b)
{
   return memcmp(&a, &b, sizeof(GUID)) == 0;
}

// The surface CreateDevice and Present use for a window: the backend's, or the
// test hook's (gfx.h).
GLSurface *SurfaceFor(HWND hwnd);

// How a texture format is stored in GL; false if unsupported.
struct PixelFormat
{
   GLint  internal;
   GLenum format, type;
   int    bytes;
   bool   opaque;   // X formats: alpha reads as 1
};
bool LookupFormat(D3DFORMAT format, PixelFormat *out);

// The caps a device reports; the IDirect3D9 reports them with the limits a GL
// 3.3 implementation guarantees, since it has no context yet.
void FillCaps(D3DCAPS9 *caps, int maxTextureSize, float maxAnisotropy);

/************************************************************************/
// COM plumbing

// AddRef/Release/QueryInterface for an object implementing Interface.
template <class Interface>
class RefCounted : public Interface
{
public:
   HRESULT QueryInterface(REFIID riid, void **out) override
   {
      if (out == nullptr)
         return E_POINTER;
      if (SameIID(riid, kIID_IUnknown) || HasInterface(riid))
      {
         this->AddRef();
         *out = this;
         return S_OK;
      }
      *out = nullptr;
      return E_NOINTERFACE;
   }
   ULONG AddRef() override { return ++refs; }
   ULONG Release() override
   {
      ULONG r = --refs;
      if (r == 0)
         Destroy();
      return r;
   }

protected:
   virtual bool HasInterface(REFIID riid) = 0;
   virtual void Destroy() = 0;

   ULONG refs = 1;
};

// Objects whose GL names the device frees if the device goes first; they then
// fail every call that needs it.
class Tracked
{
public:
   virtual ~Tracked() = default;
   // Frees GL objects and drops the device; the device's context is current.
   virtual void Orphan() = 0;

   Device  *device = nullptr;
   Tracked *prev = nullptr, *next = nullptr;
};

// IUnknown + IDirect3DResource9 for the resource classes.
template <class Interface, D3DRESOURCETYPE kType>
class Resource : public RefCounted<Interface>, public Tracked
{
public:
   HRESULT GetDevice(IDirect3DDevice9 **out) override;
   HRESULT SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
   HRESULT FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
   DWORD SetPriority(DWORD p) override
   {
      DWORD old = priority;
      priority = p;
      return old;
   }
   DWORD GetPriority() override { return priority; }
   void PreLoad() override {}
   D3DRESOURCETYPE GetType() override { return kType; }

protected:
   DWORD priority = 0;
};

/************************************************************************/
// Resources

class Texture;

// A level of a texture, or the device's back buffer or depth buffer.  Its
// reference count is its container's, as in Direct3D.
class Surface final : public IDirect3DSurface9
{
public:
   enum Kind { BACK_BUFFER, DEPTH_STENCIL, TEXTURE_LEVEL };

   Surface(Kind k, IUnknown *c) : kind(k), container(c) {}

   HRESULT QueryInterface(REFIID riid, void **out) override;
   ULONG AddRef() override { return container->AddRef(); }
   ULONG Release() override { return container->Release(); }
   HRESULT GetDevice(IDirect3DDevice9 **out) override;
   HRESULT SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
   HRESULT FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
   DWORD SetPriority(DWORD) override { return 0; }
   DWORD GetPriority() override { return 0; }
   void PreLoad() override {}
   D3DRESOURCETYPE GetType() override { return D3DRTYPE_SURFACE; }
   HRESULT GetContainer(REFIID riid, void **out) override;
   HRESULT GetDesc(D3DSURFACE_DESC *desc) override;
   HRESULT LockRect(D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags) override;
   HRESULT UnlockRect() override;
   HRESULT GetDC(HDC *) override { return D3DERR_INVALIDCALL; }
   HRESULT ReleaseDC(HDC) override { return D3DERR_INVALIDCALL; }

   const Kind kind;
   IUnknown  *container;            // Texture or Device
   Texture   *texture = nullptr;    // TEXTURE_LEVEL
   Device    *owner = nullptr;      // BACK_BUFFER, DEPTH_STENCIL
};

class Texture final : public Resource<IDirect3DTexture9, D3DRTYPE_TEXTURE>
{
public:
   Texture() : level0(Surface::TEXTURE_LEVEL, this) { level0.texture = this; }

   // IDirect3DBaseTexture9
   DWORD SetLOD(DWORD) override { return 0; }
   DWORD GetLOD() override { return 0; }
   DWORD GetLevelCount() override { return 1; }
   HRESULT SetAutoGenFilterType(D3DTEXTUREFILTERTYPE) override { return D3DERR_INVALIDCALL; }
   D3DTEXTUREFILTERTYPE GetAutoGenFilterType() override { return D3DTEXF_NONE; }
   void GenerateMipSubLevels() override {}
   // IDirect3DTexture9
   HRESULT GetLevelDesc(UINT level, D3DSURFACE_DESC *desc) override;
   HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface9 **surface) override;
   HRESULT LockRect(UINT level, D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags) override;
   HRESULT UnlockRect(UINT level) override;
   HRESULT AddDirtyRect(const RECT *) override { return D3D_OK; }

   void Orphan() override;

   UINT      width = 0, height = 0;
   D3DFORMAT format = D3DFMT_UNKNOWN;
   DWORD     usage = 0;
   D3DPOOL   pool = D3DPOOL_MANAGED;
   GLuint    glTexture = 0;        // none for D3DPOOL_SYSTEMMEM
   GLuint    glFramebuffer = 0;    // render targets
   bool      hasContent = false;   // glTexture holds data a lock must read back
   // The CPU copy: while locked, and always for D3DPOOL_SYSTEMMEM.
   std::unique_ptr<BYTE[]> staging;
   bool      locked = false;
   DWORD     lockFlags = 0;
   RECT      lockRect{};
   Surface   level0;

protected:
   bool HasInterface(REFIID riid) override;
   void Destroy() override;
};

// The CPU copy of a vertex or index buffer, uploaded when a draw first needs
// what was written.  Direct3D uploads (or maps) on Unlock; the client locks its
// 50000-entry cache buffers whole with D3DLOCK_DISCARD and fills a fraction of
// them, so uploading the locked range on Unlock would move megabytes per frame
// that no draw reads.  Draws say which vertices and indices they read, and only
// those (rounded up, see Ensure) go to the GL buffer.
struct BufferStore
{
   UINT     length = 0;
   std::unique_ptr<BYTE[]> memory;
   GLuint   glBuffer = 0;
   // Bytes written since they were last uploaded: one interval.
   UINT     pendingLow = 0, pendingHigh = 0;
   // Start of the run uploaded since the last discard; uploads grow with it.
   UINT     runStart = 0;
   bool     orphan = false;   // D3DLOCK_DISCARD since the last upload
   bool     locked = false;

   HRESULT Lock(UINT offset, UINT size, void **data, DWORD flags);
   HRESULT Unlock();
   // Makes bytes [low, high) of the GL buffer current.
   void Ensure(const GL &gl, UINT low, UINT high);
};

class VertexBuffer final : public Resource<IDirect3DVertexBuffer9, D3DRTYPE_VERTEXBUFFER>
{
public:
   HRESULT Lock(UINT offset, UINT size, void **data, DWORD flags) override { return store.Lock(offset, size, data, flags); }
   HRESULT Unlock() override { return store.Unlock(); }
   HRESULT GetDesc(D3DVERTEXBUFFER_DESC *desc) override;

   void Orphan() override;

   BufferStore store;
   DWORD   usage = 0, fvf = 0;
   D3DPOOL pool = D3DPOOL_DEFAULT;

protected:
   bool HasInterface(REFIID riid) override;
   void Destroy() override;
};

class IndexBuffer final : public Resource<IDirect3DIndexBuffer9, D3DRTYPE_INDEXBUFFER>
{
public:
   HRESULT Lock(UINT offset, UINT size, void **data, DWORD flags) override { return store.Lock(offset, size, data, flags); }
   HRESULT Unlock() override { return store.Unlock(); }
   HRESULT GetDesc(D3DINDEXBUFFER_DESC *desc) override;

   void Orphan() override;

   BufferStore store;
   DWORD     usage = 0;
   D3DFORMAT format = D3DFMT_INDEX16;
   D3DPOOL   pool = D3DPOOL_DEFAULT;

protected:
   bool HasInterface(REFIID riid) override;
   void Destroy() override;
};

class VertexDeclaration final : public RefCounted<IDirect3DVertexDeclaration9>, public Tracked
{
public:
   static constexpr int MAX_ELEMENTS = 16;

   HRESULT GetDevice(IDirect3DDevice9 **out) override;
   HRESULT GetDeclaration(D3DVERTEXELEMENT9 *elements, UINT *count) override;

   void Orphan() override;

   D3DVERTEXELEMENT9 elements[MAX_ELEMENTS];
   int               numElements = 0;

protected:
   bool HasInterface(REFIID riid) override;
   void Destroy() override;
};

/************************************************************************/
// Device

constexpr int NUM_STAGES = 2;     // texture stages the shader combines
constexpr int NUM_SAMPLERS = 16;  // sampler state kept (Direct3D's count)
constexpr int NUM_STREAMS = 16;
constexpr int NUM_RENDER_STATES = 256;

struct SamplerState
{
   DWORD addressU = D3DTADDRESS_WRAP;
   DWORD addressV = D3DTADDRESS_WRAP;
   DWORD addressW = D3DTADDRESS_WRAP;
   DWORD borderColor = 0;
   DWORD magFilter = D3DTEXF_POINT;
   DWORD minFilter = D3DTEXF_POINT;
   DWORD mipFilter = D3DTEXF_NONE;
   DWORD mipLodBias = 0;
   DWORD maxMipLevel = 0;
   DWORD maxAnisotropy = 1;
   DWORD srgbTexture = 0;
   bool  dirty = true;
};

struct StageState
{
   DWORD value[33];   // by D3DTEXTURESTAGESTATETYPE (1..32)
};

struct Stream
{
   VertexBuffer *buffer = nullptr;
   UINT offset = 0;
   UINT stride = 0;
};

struct Uniforms
{
   GLint world, view, proj, depthBias;
   GLint tex0, tex1, hasTex0, hasTex1;
   GLint colorStage0, alphaStage0, colorStage1, alphaStage1;
   GLint alphaTest, alphaFunc, alphaRef;
   GLint fog, fogColor, fogStart, fogEnd;
   GLint hasColor, hasTexCoord0, hasTexCoord1;
};

class Direct3D9;

class Device final : public RefCounted<IDirect3DDevice9>
{
public:
   // Creates the device on window's surface; nullptr if it has none or it lacks
   // OpenGL 3.3 core.
   static Device *Create(Direct3D9 *d3d, HWND window, HWND focus, DWORD flags,
                         const D3DPRESENT_PARAMETERS &params);

   /*** IDirect3DDevice9 ***/
   HRESULT TestCooperativeLevel() override;
   UINT GetAvailableTextureMem() override;
   HRESULT EvictManagedResources() override { return D3D_OK; }
   HRESULT GetDirect3D(IDirect3D9 **out) override;
   HRESULT GetDeviceCaps(D3DCAPS9 *caps) override;
   HRESULT GetDisplayMode(UINT, D3DDISPLAYMODE *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *params) override;
   HRESULT SetCursorProperties(UINT, UINT, IDirect3DSurface9 *) override { return D3DERR_INVALIDCALL; }
   void SetCursorPosition(int, int, DWORD) override {}
   BOOL ShowCursor(BOOL) override { return FALSE; }
   HRESULT CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *, IDirect3DSwapChain9 **) override { return D3DERR_INVALIDCALL; }
   HRESULT GetSwapChain(UINT, IDirect3DSwapChain9 **) override { return D3DERR_INVALIDCALL; }
   UINT GetNumberOfSwapChains() override { return 1; }
   HRESULT Reset(D3DPRESENT_PARAMETERS *) override { return D3DERR_INVALIDCALL; }
   HRESULT Present(const RECT *src, const RECT *dst, HWND window, const RGNDATA *dirty) override;
   HRESULT GetBackBuffer(UINT swapChain, UINT index, D3DBACKBUFFER_TYPE type, IDirect3DSurface9 **out) override;
   HRESULT GetRasterStatus(UINT, D3DRASTER_STATUS *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetDialogBoxMode(BOOL) override { return D3D_OK; }
   void SetGammaRamp(UINT, DWORD, const D3DGAMMARAMP *) override {}
   void GetGammaRamp(UINT, D3DGAMMARAMP *) override {}
   HRESULT CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format,
                         D3DPOOL pool, IDirect3DTexture9 **out, HANDLE *shared) override;
   HRESULT CreateVolumeTexture(UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DVolumeTexture9 **, HANDLE *) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture9 **, HANDLE *) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool,
                              IDirect3DVertexBuffer9 **out, HANDLE *shared) override;
   HRESULT CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool,
                             IDirect3DIndexBuffer9 **out, HANDLE *shared) override;
   HRESULT CreateRenderTarget(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9 **, HANDLE *) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateDepthStencilSurface(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9 **, HANDLE *) override { return D3DERR_INVALIDCALL; }
   HRESULT UpdateSurface(IDirect3DSurface9 *, const RECT *, IDirect3DSurface9 *, const POINT *) override { return D3DERR_INVALIDCALL; }
   HRESULT UpdateTexture(IDirect3DBaseTexture9 *src, IDirect3DBaseTexture9 *dst) override;
   HRESULT GetRenderTargetData(IDirect3DSurface9 *, IDirect3DSurface9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetFrontBufferData(UINT, IDirect3DSurface9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT StretchRect(IDirect3DSurface9 *src, const RECT *srcRect, IDirect3DSurface9 *dst,
                       const RECT *dstRect, D3DTEXTUREFILTERTYPE filter) override;
   HRESULT ColorFill(IDirect3DSurface9 *, const RECT *, D3DCOLOR) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateOffscreenPlainSurface(UINT, UINT, D3DFORMAT, D3DPOOL, IDirect3DSurface9 **, HANDLE *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetRenderTarget(DWORD index, IDirect3DSurface9 *surface) override;
   HRESULT GetRenderTarget(DWORD index, IDirect3DSurface9 **out) override;
   HRESULT SetDepthStencilSurface(IDirect3DSurface9 *surface) override;
   HRESULT GetDepthStencilSurface(IDirect3DSurface9 **out) override;
   HRESULT BeginScene() override;
   HRESULT EndScene() override;
   HRESULT Clear(DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil) override;
   HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix) override;
   HRESULT GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix) override;
   HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetViewport(const D3DVIEWPORT9 *viewport) override;
   HRESULT GetViewport(D3DVIEWPORT9 *viewport) override;
   HRESULT SetMaterial(const D3DMATERIAL9 *) override { return D3D_OK; }
   HRESULT GetMaterial(D3DMATERIAL9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetLight(DWORD, const D3DLIGHT9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetLight(DWORD, D3DLIGHT9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT LightEnable(DWORD, BOOL) override { return D3DERR_INVALIDCALL; }
   HRESULT GetLightEnable(DWORD, BOOL *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetClipPlane(DWORD, const float *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetClipPlane(DWORD, float *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override;
   HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD *value) override;
   HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE, IDirect3DStateBlock9 **) override { return D3DERR_INVALIDCALL; }
   HRESULT BeginStateBlock() override { return D3DERR_INVALIDCALL; }
   HRESULT EndStateBlock(IDirect3DStateBlock9 **) override { return D3DERR_INVALIDCALL; }
   HRESULT SetClipStatus(const D3DCLIPSTATUS9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetClipStatus(D3DCLIPSTATUS9 *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetTexture(DWORD stage, IDirect3DBaseTexture9 **out) override;
   HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture9 *texture) override;
   HRESULT GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD *value) override;
   HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override;
   HRESULT GetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD *value) override;
   HRESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value) override;
   HRESULT ValidateDevice(DWORD *passes) override;
   HRESULT SetPaletteEntries(UINT, const PALETTEENTRY *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPaletteEntries(UINT, PALETTEENTRY *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetCurrentTexturePalette(UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetCurrentTexturePalette(UINT *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetScissorRect(const RECT *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetScissorRect(RECT *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetSoftwareVertexProcessing(BOOL) override { return D3D_OK; }
   BOOL GetSoftwareVertexProcessing() override { return FALSE; }
   HRESULT SetNPatchMode(float) override { return D3DERR_INVALIDCALL; }
   float GetNPatchMode() override { return 0.0f; }
   HRESULT DrawPrimitive(D3DPRIMITIVETYPE type, UINT start, UINT count) override;
   HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertex, UINT minIndex, UINT numVertices,
                                UINT startIndex, UINT primCount) override;
   HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE, UINT, UINT, UINT, const void *, D3DFORMAT, const void *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT ProcessVertices(UINT, UINT, UINT, IDirect3DVertexBuffer9 *, IDirect3DVertexDeclaration9 *, DWORD) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateVertexDeclaration(const D3DVERTEXELEMENT9 *elements, IDirect3DVertexDeclaration9 **out) override;
   HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9 *decl) override;
   HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9 **out) override;
   HRESULT SetFVF(DWORD) override { return D3DERR_INVALIDCALL; }
   HRESULT GetFVF(DWORD *) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateVertexShader(const DWORD *, IDirect3DVertexShader9 **) override { return D3DERR_INVALIDCALL; }
   HRESULT SetVertexShader(IDirect3DVertexShader9 *shader) override;
   HRESULT GetVertexShader(IDirect3DVertexShader9 **out) override;
   HRESULT SetVertexShaderConstantF(UINT, const float *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetVertexShaderConstantF(UINT, float *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT SetVertexShaderConstantI(UINT, const int *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetVertexShaderConstantI(UINT, int *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT SetVertexShaderConstantB(UINT, const BOOL *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetVertexShaderConstantB(UINT, BOOL *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT SetStreamSource(UINT stream, IDirect3DVertexBuffer9 *buffer, UINT offset, UINT stride) override;
   HRESULT GetStreamSource(UINT stream, IDirect3DVertexBuffer9 **buffer, UINT *offset, UINT *stride) override;
   HRESULT SetStreamSourceFreq(UINT, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetStreamSourceFreq(UINT, UINT *) override { return D3DERR_INVALIDCALL; }
   HRESULT SetIndices(IDirect3DIndexBuffer9 *indices) override;
   HRESULT GetIndices(IDirect3DIndexBuffer9 **out) override;
   HRESULT CreatePixelShader(const DWORD *, IDirect3DPixelShader9 **) override { return D3DERR_INVALIDCALL; }
   HRESULT SetPixelShader(IDirect3DPixelShader9 *shader) override;
   HRESULT GetPixelShader(IDirect3DPixelShader9 **out) override;
   HRESULT SetPixelShaderConstantF(UINT, const float *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPixelShaderConstantF(UINT, float *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT SetPixelShaderConstantI(UINT, const int *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPixelShaderConstantI(UINT, int *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT SetPixelShaderConstantB(UINT, const BOOL *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT GetPixelShaderConstantB(UINT, BOOL *, UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT DrawRectPatch(UINT, const float *, const D3DRECTPATCH_INFO *) override { return D3DERR_INVALIDCALL; }
   HRESULT DrawTriPatch(UINT, const float *, const D3DTRIPATCH_INFO *) override { return D3DERR_INVALIDCALL; }
   HRESULT DeletePatch(UINT) override { return D3DERR_INVALIDCALL; }
   HRESULT CreateQuery(D3DQUERYTYPE, IDirect3DQuery9 **) override { return D3DERR_NOTAVAILABLE; }

   /*** For the resources ***/
   // Makes the context current and the device's GL state the context's, the
   // first time after a Present (or InvalidateGLState); false if the device is
   // lost.  Every call that touches GL starts with it.
   bool Begin();
   void Track(Tracked *t);
   void Untrack(Tracked *t);
   // Clears whatever binding refers to the resource going away.
   void Unbind(Texture *t);
   void Unbind(VertexBuffer *b);
   void Unbind(IndexBuffer *b);
   void Unbind(VertexDeclaration *d);
   // Binds texture on a unit draws never sample, for uploads and readbacks.
   void BindForUpload(GLuint texture);
   int Width() const { return width; }
   int Height() const { return height; }
   int Samples() const { return samples; }

   GL         gl;
   GLuint     currentFramebuffer = 0;
   bool       glValid = false;
   Device    *nextDevice = nullptr;   // all devices, for InvalidateGLState

protected:
   bool HasInterface(REFIID riid) override;
   void Destroy() override;

private:
   Device() : backSurface(Surface::BACK_BUFFER, this), depthSurface(Surface::DEPTH_STENCIL, this) {}
   ~Device();

   bool Init(Direct3D9 *d3d, HWND window, HWND focus, DWORD flags, const D3DPRESENT_PARAMETERS &params);
   bool CreateProgram();
   void CreateBackBuffer();
   void Rebind();
   void BindTarget(GLuint framebuffer, int w, int h);
   void ApplyViewport();
   void ApplyState();
   void ApplySampler(int stage);
   void ApplyTextures();
   // Points the attributes at the streams and uploads vertices [first, last]
   // of each and, for indexed draws, indices [startIndex, startIndex + count).
   void ApplyAttributes();
   void UploadVertices(UINT first, UINT last);

   Direct3D9 *d3d = nullptr;
   GLSurface *surface = nullptr;
   HWND       window = nullptr, focusWindow = nullptr;
   DWORD      behaviorFlags = 0;
   bool       lost = false;
   Tracked   *resources = nullptr;

   int        width = 0, height = 0, samples = 0;
   GLint      maxTextureSize = 2048;
   float      maxAnisotropy = 1.0f;
   GLuint     program = 0;
   Uniforms   u{};
   GLuint     vao = 0;
   GLuint     samplers[NUM_STAGES]{};
   GLuint     whiteTexture = 0;    // bound to stages without one (the shader skips it)

   // Back buffer (possibly multisampled); Present copies the presented part to
   // one of two textures in turn, so the backend may still be reading the last.
   GLuint     backFramebuffer = 0, backColor = 0, backDepth = 0;
   GLuint     presentFramebuffer = 0, presentTextures[2]{};
   int        presentSize[2][2]{};
   int        presentIndex = 0;
   int        targetWidth = 0, targetHeight = 0;
   Surface    backSurface, depthSurface;
   Surface   *currentTarget = nullptr;
   bool       depthEnabled = true;   // a depth surface is set

   D3DVIEWPORT9 viewport{};
   DWORD        renderState[NUM_RENDER_STATES]{};
   SamplerState sampler[NUM_SAMPLERS];
   StageState   stage[8]{};
   D3DMATRIX    world{}, view{}, proj{};
   bool         transformsDirty = true;
   bool         stateDirty = true;
   bool         texturesDirty = true;
   bool         attributesDirty = true;
   IDirect3DBaseTexture9 *texture[NUM_STAGES]{};
   VertexDeclaration *declaration = nullptr;
   Stream       streams[NUM_STREAMS];
   IndexBuffer *indices = nullptr;
   // Per attribute location: whether the declaration feeds it, from which
   // stream, and how many bytes past the vertex's start it ends.
   bool         attribEnabled[4]{};
   int          attribStream[4]{};
   UINT         attribEnd[4]{};
};

class Direct3D9 final : public RefCounted<IDirect3D9>
{
public:
   HRESULT RegisterSoftwareDevice(void *) override { return D3DERR_INVALIDCALL; }
   UINT GetAdapterCount() override { return 1; }
   HRESULT GetAdapterIdentifier(UINT adapter, DWORD flags, D3DADAPTER_IDENTIFIER9 *id) override;
   UINT GetAdapterModeCount(UINT, D3DFORMAT) override { return 0; }
   HRESULT EnumAdapterModes(UINT, D3DFORMAT, UINT, D3DDISPLAYMODE *) override { return D3DERR_INVALIDCALL; }
   HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *) override { return D3DERR_INVALIDCALL; }
   HRESULT CheckDeviceType(UINT adapter, D3DDEVTYPE type, D3DFORMAT display, D3DFORMAT back, BOOL windowed) override;
   HRESULT CheckDeviceFormat(UINT adapter, D3DDEVTYPE type, D3DFORMAT adapterFormat, DWORD usage,
                             D3DRESOURCETYPE rtype, D3DFORMAT format) override;
   HRESULT CheckDeviceMultiSampleType(UINT adapter, D3DDEVTYPE type, D3DFORMAT format, BOOL windowed,
                                      D3DMULTISAMPLE_TYPE samples, DWORD *quality) override;
   HRESULT CheckDepthStencilMatch(UINT adapter, D3DDEVTYPE type, D3DFORMAT adapterFormat,
                                  D3DFORMAT target, D3DFORMAT depth) override;
   HRESULT CheckDeviceFormatConversion(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT) override { return D3DERR_NOTAVAILABLE; }
   HRESULT GetDeviceCaps(UINT adapter, D3DDEVTYPE type, D3DCAPS9 *caps) override;
   HMONITOR GetAdapterMonitor(UINT) override { return nullptr; }
   HRESULT CreateDevice(UINT adapter, D3DDEVTYPE type, HWND focus, DWORD flags,
                        D3DPRESENT_PARAMETERS *params, IDirect3DDevice9 **out) override;

   // The renderer and vendor of the last device created, for the identifier.
   char renderer[MAX_DEVICE_IDENTIFIER_STRING] = "OpenGL";
   char vendor[MAX_DEVICE_IDENTIFIER_STRING] = "OpenGL";

protected:
   bool HasInterface(REFIID riid) override { return SameIID(riid, kIID_IDirect3D9); }
   void Destroy() override { delete this; }
};

template <class Interface, D3DRESOURCETYPE kType>
HRESULT Resource<Interface, kType>::GetDevice(IDirect3DDevice9 **out)
{
   if (out == nullptr || device == nullptr)
      return D3DERR_INVALIDCALL;
   device->AddRef();
   *out = static_cast<IDirect3DDevice9 *>(device);
   return D3D_OK;
}

}  // namespace w32::gfx

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

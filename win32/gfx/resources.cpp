/*
 * resources.cpp:  Direct3D 9 textures, surfaces, vertex and index buffers and
 *   vertex declarations on OpenGL.
 */

#include "core/win32.h"
#include "gfx/d3d9_impl.h"

#include <algorithm>
#include <cstdint>

namespace w32::gfx {

namespace {

// Frees the resource's GL objects (if its device still has a context) and
// forgets it, before it is deleted.
template <class T>
void Detach(T *resource)
{
   Device *device = resource->device;
   if (device == nullptr)
      return;
   if (device->Begin())
      resource->Orphan();
   else
   {
      device->Unbind(resource);
      resource->device = nullptr;
   }
   device->Untrack(resource);
}

}  // namespace

bool LookupFormat(D3DFORMAT format, PixelFormat *out)
{
   switch (format)
   {
   case D3DFMT_A8R8G8B8: *out = { GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, false }; return true;
   case D3DFMT_X8R8G8B8: *out = { GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, true }; return true;
   case D3DFMT_A1R5G5B5: *out = { GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, false }; return true;
   case D3DFMT_X1R5G5B5: *out = { GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, true }; return true;
   case D3DFMT_A4R4G4B4: *out = { GL_RGBA4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, false }; return true;
   case D3DFMT_R5G6B5:   *out = { GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, true }; return true;
   default:              return false;
   }
}

/************************************************************************/
// Surfaces

HRESULT Surface::QueryInterface(REFIID riid, void **out)
{
   if (out == nullptr)
      return E_POINTER;
   if (SameIID(riid, kIID_IUnknown) || SameIID(riid, kIID_IDirect3DResource9) ||
       SameIID(riid, kIID_IDirect3DSurface9))
   {
      AddRef();
      *out = static_cast<IDirect3DSurface9 *>(this);
      return S_OK;
   }
   *out = nullptr;
   return E_NOINTERFACE;
}

HRESULT Surface::GetDevice(IDirect3DDevice9 **out)
{
   if (texture != nullptr)
      return texture->GetDevice(out);
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   owner->AddRef();
   *out = owner;
   return D3D_OK;
}

HRESULT Surface::GetContainer(REFIID riid, void **out)
{
   return container->QueryInterface(riid, out);
}

HRESULT Surface::GetDesc(D3DSURFACE_DESC *desc)
{
   if (desc == nullptr)
      return D3DERR_INVALIDCALL;
   if (texture != nullptr)
      return texture->GetLevelDesc(0, desc);
   desc->Format = kind == BACK_BUFFER ? D3DFMT_A8R8G8B8 : D3DFMT_D24S8;
   desc->Type = D3DRTYPE_SURFACE;
   desc->Usage = kind == BACK_BUFFER ? D3DUSAGE_RENDERTARGET : D3DUSAGE_DEPTHSTENCIL;
   desc->Pool = D3DPOOL_DEFAULT;
   desc->MultiSampleType = (D3DMULTISAMPLE_TYPE) owner->Samples();
   desc->MultiSampleQuality = 0;
   desc->Width = owner->Width();
   desc->Height = owner->Height();
   return D3D_OK;
}

HRESULT Surface::LockRect(D3DLOCKED_RECT *locked, const RECT *rect, DWORD flags)
{
   if (texture == nullptr)
      return D3DERR_INVALIDCALL;   // the back buffer is not lockable
   return texture->LockRect(0, locked, rect, flags);
}

HRESULT Surface::UnlockRect()
{
   if (texture == nullptr)
      return D3DERR_INVALIDCALL;
   return texture->UnlockRect(0);
}

/************************************************************************/
// Textures

bool Texture::HasInterface(REFIID riid)
{
   return SameIID(riid, kIID_IDirect3DResource9) || SameIID(riid, kIID_IDirect3DBaseTexture9) ||
          SameIID(riid, kIID_IDirect3DTexture9);
}

void Texture::Orphan()
{
   device->Unbind(this);
   if (glFramebuffer)
      device->gl.DeleteFramebuffers(1, &glFramebuffer);
   if (glTexture)
      device->gl.DeleteTextures(1, &glTexture);
   glFramebuffer = glTexture = 0;
   device = nullptr;
}

void Texture::Destroy()
{
   Detach(this);
   delete this;
}

HRESULT Texture::GetLevelDesc(UINT level, D3DSURFACE_DESC *desc)
{
   if (level != 0 || desc == nullptr)
      return D3DERR_INVALIDCALL;
   desc->Format = format;
   desc->Type = D3DRTYPE_SURFACE;
   desc->Usage = usage;
   desc->Pool = pool;
   desc->MultiSampleType = D3DMULTISAMPLE_NONE;
   desc->MultiSampleQuality = 0;
   desc->Width = width;
   desc->Height = height;
   return D3D_OK;
}

HRESULT Texture::GetSurfaceLevel(UINT level, IDirect3DSurface9 **surface)
{
   if (level != 0 || surface == nullptr)
      return D3DERR_INVALIDCALL;
   level0.AddRef();
   *surface = &level0;
   return D3D_OK;
}

HRESULT Texture::LockRect(UINT level, D3DLOCKED_RECT *out, const RECT *rect, DWORD flags)
{
   PixelFormat f;
   if (level != 0 || out == nullptr || locked || (usage & D3DUSAGE_RENDERTARGET) ||
       !LookupFormat(format, &f))
      return D3DERR_INVALIDCALL;
   RECT r = rect ? *rect : RECT{ 0, 0, (LONG) width, (LONG) height };
   if (r.left < 0 || r.top < 0 || r.left >= r.right || r.top >= r.bottom ||
       r.right > (LONG) width || r.bottom > (LONG) height)
      return D3DERR_INVALIDCALL;

   if (staging == nullptr)
   {
      staging.reset(new BYTE[(size_t) width * height * f.bytes]());
      // Managed textures keep no CPU copy once uploaded; a later lock reads
      // the texture back so it sees what Direct3D's copy would hold.
      if (hasContent && !(flags & D3DLOCK_DISCARD) && device != nullptr && device->Begin())
      {
         device->BindForUpload(glTexture);
         device->gl.PixelStorei(GL_PACK_ALIGNMENT, 1);
         device->gl.GetTexImage(GL_TEXTURE_2D, 0, f.format, f.type, staging.get());
      }
   }

   out->Pitch = (INT) (width * f.bytes);
   out->pBits = staging.get() + ((size_t) r.top * width + r.left) * f.bytes;
   locked = true;
   lockFlags = flags;
   lockRect = r;
   return D3D_OK;
}

HRESULT Texture::UnlockRect(UINT level)
{
   if (level != 0 || !locked)
      return D3DERR_INVALIDCALL;
   locked = false;
   if (pool == D3DPOOL_SYSTEMMEM)
      return D3D_OK;   // UpdateTexture uploads it

   PixelFormat f;
   LookupFormat(format, &f);
   if (!(lockFlags & D3DLOCK_READONLY) && device != nullptr && device->Begin())
   {
      const GL &gl = device->gl;
      const RECT &r = lockRect;
      device->BindForUpload(glTexture);
      gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
      gl.PixelStorei(GL_UNPACK_ROW_LENGTH, width);
      gl.TexSubImage2D(GL_TEXTURE_2D, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, f.format, f.type,
                       staging.get() + ((size_t) r.top * width + r.left) * f.bytes);
      gl.PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      hasContent = true;
   }
   staging.reset();
   return D3D_OK;
}

/************************************************************************/
// Buffers

HRESULT BufferStore::Lock(UINT offset, UINT size, void **data, DWORD flags)
{
   if (data == nullptr || offset > length)
      return D3DERR_INVALIDCALL;
   UINT end = size == 0 ? length : std::min(length, offset + size);
   if (flags & D3DLOCK_DISCARD)
   {
      // The whole buffer is discarded; only what this lock writes matters.
      orphan = true;
      pendingLow = runStart = offset;
      pendingHigh = end;
   }
   else if (!(flags & D3DLOCK_READONLY) && offset < end)
   {
      if (pendingLow == pendingHigh)
      {
         pendingLow = runStart = offset;
         pendingHigh = end;
      }
      else
      {
         pendingLow = std::min(pendingLow, offset);
         pendingHigh = std::max(pendingHigh, end);
         runStart = std::min(runStart, pendingLow);
      }
   }
   locked = true;
   *data = memory.get() + offset;
   return D3D_OK;
}

HRESULT BufferStore::Unlock()
{
   locked = false;
   return D3D_OK;
}

void BufferStore::Ensure(const GL &gl, UINT low, UINT high)
{
   low = std::max(low, pendingLow);
   high = std::min(high, pendingHigh);
   if (low >= high)
      return;

   // Upload from the start of what is pending (so it stays one interval), and
   // at least as much again as this run has uploaded so far: a cache filled
   // once and drawn chunk by chunk takes a few uploads, not one per draw, and
   // never more than twice what its draws read.
   UINT start = pendingLow;
   uint64_t grow = std::max<uint64_t>(64 * 1024, start - runStart);
   UINT end = (UINT) std::min<uint64_t>(pendingHigh, std::max<uint64_t>(high, start + grow));

   gl.BindBuffer(GL_COPY_WRITE_BUFFER, glBuffer);
   if (orphan)
   {
      // New storage, so the driver need not wait for draws reading the old.
      gl.BufferData(GL_COPY_WRITE_BUFFER, length, nullptr, GL_STREAM_DRAW);
      orphan = false;
   }
   gl.BufferSubData(GL_COPY_WRITE_BUFFER, start, end - start, memory.get() + start);
   pendingLow = end;
   if (pendingLow >= pendingHigh)
      pendingLow = pendingHigh = 0;
}

bool VertexBuffer::HasInterface(REFIID riid)
{
   return SameIID(riid, kIID_IDirect3DResource9) || SameIID(riid, kIID_IDirect3DVertexBuffer9);
}

void VertexBuffer::Orphan()
{
   device->Unbind(this);
   device->gl.DeleteBuffers(1, &store.glBuffer);
   store.glBuffer = 0;
   device = nullptr;
}

void VertexBuffer::Destroy()
{
   Detach(this);
   delete this;
}

HRESULT VertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC *desc)
{
   if (desc == nullptr)
      return D3DERR_INVALIDCALL;
   desc->Format = D3DFMT_VERTEXDATA;
   desc->Type = D3DRTYPE_VERTEXBUFFER;
   desc->Usage = usage;
   desc->Pool = pool;
   desc->Size = store.length;
   desc->FVF = fvf;
   return D3D_OK;
}

bool IndexBuffer::HasInterface(REFIID riid)
{
   return SameIID(riid, kIID_IDirect3DResource9) || SameIID(riid, kIID_IDirect3DIndexBuffer9);
}

void IndexBuffer::Orphan()
{
   device->Unbind(this);
   device->gl.DeleteBuffers(1, &store.glBuffer);
   store.glBuffer = 0;
   device = nullptr;
}

void IndexBuffer::Destroy()
{
   Detach(this);
   delete this;
}

HRESULT IndexBuffer::GetDesc(D3DINDEXBUFFER_DESC *desc)
{
   if (desc == nullptr)
      return D3DERR_INVALIDCALL;
   desc->Format = format;
   desc->Type = D3DRTYPE_INDEXBUFFER;
   desc->Usage = usage;
   desc->Pool = pool;
   desc->Size = store.length;
   return D3D_OK;
}

/************************************************************************/
// Vertex declarations

bool VertexDeclaration::HasInterface(REFIID riid)
{
   return SameIID(riid, kIID_IDirect3DVertexDeclaration9);
}

void VertexDeclaration::Orphan()
{
   device->Unbind(this);
   device = nullptr;
}

void VertexDeclaration::Destroy()
{
   if (device != nullptr)
   {
      Device *d = device;
      Orphan();
      d->Untrack(this);
   }
   delete this;
}

HRESULT VertexDeclaration::GetDevice(IDirect3DDevice9 **out)
{
   if (out == nullptr || device == nullptr)
      return D3DERR_INVALIDCALL;
   device->AddRef();
   *out = device;
   return D3D_OK;
}

HRESULT VertexDeclaration::GetDeclaration(D3DVERTEXELEMENT9 *out, UINT *count)
{
   if (count == nullptr)
      return D3DERR_INVALIDCALL;
   *count = numElements + 1;   // with D3DDECL_END
   if (out != nullptr)
   {
      static constexpr D3DVERTEXELEMENT9 end = D3DDECL_END();
      std::copy(elements, elements + numElements, out);
      out[numElements] = end;
   }
   return D3D_OK;
}

}  // namespace w32::gfx

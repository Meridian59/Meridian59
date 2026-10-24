/*
 * d3d9.cpp:  Direct3DCreate9 and IDirect3D9: one adapter, the window's OpenGL
 *   3.3 core context.
 */

#include "core/win32.h"
#include "gfx/d3d9_impl.h"
#include "gfx/gfx.h"

#include <cstdio>
#include <cstring>

namespace w32 {

// The backend defines it; this stands in until then, and for programs without
// a backend surface (the tests set a provider).
__attribute__((weak)) GLSurface *GetGLSurface(HWND)
{
   return nullptr;
}

}  // namespace w32

namespace w32::gfx {

namespace {

GLSurface *(*gProvider)(HWND) = nullptr;

bool SupportedTargetFormat(D3DFORMAT f)
{
   return f == D3DFMT_A8R8G8B8 || f == D3DFMT_X8R8G8B8;
}

}  // namespace

void SetSurfaceProvider(GLSurface *(*provider)(HWND))
{
   gProvider = provider;
}

GLSurface *SurfaceFor(HWND hwnd)
{
   return gProvider ? gProvider(hwnd) : GetGLSurface(hwnd);
}

void FillCaps(D3DCAPS9 *caps, int maxTextureSize, float maxAnisotropy)
{
   memset(caps, 0, sizeof(*caps));
   caps->DeviceType = D3DDEVTYPE_HAL;
   caps->AdapterOrdinal = 0;
   caps->Caps2 = D3DCAPS2_DYNAMICTEXTURES;
   caps->PresentationIntervals = D3DPRESENT_INTERVAL_IMMEDIATE | D3DPRESENT_INTERVAL_ONE;
   caps->DevCaps = D3DDEVCAPS_EXECUTESYSTEMMEMORY | D3DDEVCAPS_EXECUTEVIDEOMEMORY |
                   D3DDEVCAPS_TLVERTEXSYSTEMMEMORY | D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
                   D3DDEVCAPS_TEXTURESYSTEMMEMORY | D3DDEVCAPS_TEXTUREVIDEOMEMORY |
                   D3DDEVCAPS_DRAWPRIMTLVERTEX | D3DDEVCAPS_CANRENDERAFTERFLIP |
                   D3DDEVCAPS_DRAWPRIMITIVES2 | D3DDEVCAPS_DRAWPRIMITIVES2EX |
                   D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION;
   caps->PrimitiveMiscCaps = D3DPMISCCAPS_MASKZ | D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW |
                             D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_COLORWRITEENABLE |
                             D3DPMISCCAPS_CLIPTLVERTS | D3DPMISCCAPS_BLENDOP;
   caps->RasterCaps = D3DPRASTERCAPS_DITHER | D3DPRASTERCAPS_ZTEST | D3DPRASTERCAPS_FOGTABLE |
                      D3DPRASTERCAPS_ZFOG | D3DPRASTERCAPS_WFOG | D3DPRASTERCAPS_DEPTHBIAS |
                      D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS;
   if (maxAnisotropy > 1.0f)
      caps->RasterCaps |= D3DPRASTERCAPS_ANISOTROPY;
   DWORD cmp = D3DPCMPCAPS_NEVER | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_LESSEQUAL |
               D3DPCMPCAPS_GREATER | D3DPCMPCAPS_NOTEQUAL | D3DPCMPCAPS_GREATEREQUAL | D3DPCMPCAPS_ALWAYS;
   caps->ZCmpCaps = cmp;
   caps->AlphaCmpCaps = cmp;
   DWORD blend = D3DPBLENDCAPS_ZERO | D3DPBLENDCAPS_ONE | D3DPBLENDCAPS_SRCCOLOR |
                 D3DPBLENDCAPS_INVSRCCOLOR | D3DPBLENDCAPS_SRCALPHA | D3DPBLENDCAPS_INVSRCALPHA |
                 D3DPBLENDCAPS_DESTALPHA | D3DPBLENDCAPS_INVDESTALPHA | D3DPBLENDCAPS_DESTCOLOR |
                 D3DPBLENDCAPS_INVDESTCOLOR;
   caps->SrcBlendCaps = blend;
   caps->DestBlendCaps = blend;
   caps->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_ALPHAGOURAUDBLEND;
   caps->TextureCaps = D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_ALPHA;
   caps->TextureFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR |
                             D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
   if (maxAnisotropy > 1.0f)
      caps->TextureFilterCaps |= D3DPTFILTERCAPS_MINFANISOTROPIC | D3DPTFILTERCAPS_MAGFANISOTROPIC;
   caps->TextureAddressCaps = D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | D3DPTADDRESSCAPS_CLAMP |
                              D3DPTADDRESSCAPS_BORDER | D3DPTADDRESSCAPS_INDEPENDENTUV;
   caps->LineCaps = D3DLINECAPS_TEXTURE | D3DLINECAPS_ZTEST | D3DLINECAPS_BLEND |
                    D3DLINECAPS_ALPHACMP | D3DLINECAPS_FOG;
   caps->MaxTextureWidth = caps->MaxTextureHeight = maxTextureSize;
   caps->MaxTextureRepeat = 8192;
   caps->MaxTextureAspectRatio = maxTextureSize;
   caps->MaxAnisotropy = maxAnisotropy > 1.0f ? (DWORD) maxAnisotropy : 1;
   caps->MaxVertexW = 1e10f;
   caps->GuardBandLeft = caps->GuardBandTop = -32768.0f;
   caps->GuardBandRight = caps->GuardBandBottom = 32768.0f;
   caps->StencilCaps = D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE |
                       D3DSTENCILCAPS_INCRSAT | D3DSTENCILCAPS_DECRSAT | D3DSTENCILCAPS_INVERT |
                       D3DSTENCILCAPS_INCR | D3DSTENCILCAPS_DECR;
   caps->FVFCaps = NUM_STAGES;   // texture coordinate sets
   caps->TextureOpCaps = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
                         D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X |
                         D3DTEXOPCAPS_ADD;
   caps->MaxTextureBlendStages = NUM_STAGES;
   caps->MaxSimultaneousTextures = NUM_STAGES;
   caps->MaxPointSize = 1.0f;
   caps->MaxPrimitiveCount = 0xfffff;
   caps->MaxVertexIndex = 0xffffff;
   caps->MaxStreams = NUM_STREAMS;
   caps->MaxStreamStride = 255;
   caps->MasterAdapterOrdinal = 0;
   caps->AdapterOrdinalInGroup = 0;
   caps->NumberOfAdaptersInGroup = 1;
   caps->NumSimultaneousRTs = 1;
   caps->StretchRectFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR |
                                 D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
}

/************************************************************************/

HRESULT Direct3D9::GetAdapterIdentifier(UINT adapter, DWORD, D3DADAPTER_IDENTIFIER9 *id)
{
   if (adapter != 0 || id == nullptr)
      return D3DERR_INVALIDCALL;
   memset(id, 0, sizeof(*id));
   snprintf(id->Driver, sizeof(id->Driver), "%s", vendor);
   snprintf(id->Description, sizeof(id->Description), "%s", renderer);
   snprintf(id->DeviceName, sizeof(id->DeviceName), "\\\\.\\DISPLAY1");
   id->WHQLLevel = 1;
   return D3D_OK;
}

HRESULT Direct3D9::CheckDeviceType(UINT adapter, D3DDEVTYPE type, D3DFORMAT display, D3DFORMAT back, BOOL)
{
   if (adapter != 0)
      return D3DERR_INVALIDCALL;
   if (type != D3DDEVTYPE_HAL || display != D3DFMT_X8R8G8B8 ||
       (back != D3DFMT_UNKNOWN && !SupportedTargetFormat(back)))
      return D3DERR_NOTAVAILABLE;
   return D3D_OK;
}

HRESULT Direct3D9::CheckDeviceFormat(UINT adapter, D3DDEVTYPE type, D3DFORMAT, DWORD usage,
                                     D3DRESOURCETYPE rtype, D3DFORMAT format)
{
   if (adapter != 0)
      return D3DERR_INVALIDCALL;
   if (type != D3DDEVTYPE_HAL)
      return D3DERR_NOTAVAILABLE;
   if (usage & D3DUSAGE_DEPTHSTENCIL)
      return format == D3DFMT_D24S8 && rtype == D3DRTYPE_SURFACE ? D3D_OK : D3DERR_NOTAVAILABLE;
   if (rtype != D3DRTYPE_TEXTURE && rtype != D3DRTYPE_SURFACE)
      return D3DERR_NOTAVAILABLE;
   if (usage & D3DUSAGE_RENDERTARGET)
      return SupportedTargetFormat(format) ? D3D_OK : D3DERR_NOTAVAILABLE;
   PixelFormat f;
   return LookupFormat(format, &f) ? D3D_OK : D3DERR_NOTAVAILABLE;
}

HRESULT Direct3D9::CheckDeviceMultiSampleType(UINT adapter, D3DDEVTYPE type, D3DFORMAT format, BOOL,
                                              D3DMULTISAMPLE_TYPE samples, DWORD *quality)
{
   if (adapter != 0)
      return D3DERR_INVALIDCALL;
   // GL 3.3 guarantees 4 samples; CreateDevice takes fewer if the context
   // has fewer than asked for.
   if (type != D3DDEVTYPE_HAL || !(SupportedTargetFormat(format) || format == D3DFMT_D24S8) ||
       samples == D3DMULTISAMPLE_NONMASKABLE || samples > D3DMULTISAMPLE_16_SAMPLES)
      return D3DERR_NOTAVAILABLE;
   if (quality != nullptr)
      *quality = 1;
   return D3D_OK;
}

HRESULT Direct3D9::CheckDepthStencilMatch(UINT adapter, D3DDEVTYPE type, D3DFORMAT, D3DFORMAT target,
                                          D3DFORMAT depth)
{
   if (adapter != 0)
      return D3DERR_INVALIDCALL;
   return type == D3DDEVTYPE_HAL && SupportedTargetFormat(target) && depth == D3DFMT_D24S8
             ? D3D_OK : D3DERR_NOTAVAILABLE;
}

HRESULT Direct3D9::GetDeviceCaps(UINT adapter, D3DDEVTYPE type, D3DCAPS9 *caps)
{
   if (adapter != 0 || caps == nullptr)
      return D3DERR_INVALIDCALL;
   if (type != D3DDEVTYPE_HAL)
      return D3DERR_NOTAVAILABLE;
   FillCaps(caps, 4096, 16.0f);
   return D3D_OK;
}

HRESULT Direct3D9::CreateDevice(UINT adapter, D3DDEVTYPE type, HWND focus, DWORD flags,
                                D3DPRESENT_PARAMETERS *params, IDirect3DDevice9 **out)
{
   if (out == nullptr)
      return D3DERR_INVALIDCALL;
   *out = nullptr;
   if (adapter != 0 || params == nullptr)
      return D3DERR_INVALIDCALL;
   if (type != D3DDEVTYPE_HAL)
      return D3DERR_NOTAVAILABLE;
   if (!params->Windowed)
      return D3DERR_NOTAVAILABLE;   // windowed only
   HWND window = params->hDeviceWindow ? params->hDeviceWindow : focus;
   if (window == nullptr)
      return D3DERR_INVALIDCALL;

   Device *device = Device::Create(this, window, focus, flags, *params);
   if (device == nullptr)
      return D3DERR_NOTAVAILABLE;
   *out = device;
   return D3D_OK;
}

}  // namespace w32::gfx

extern "C" W32_EXPORT IDirect3D9 *WINAPI Direct3DCreate9(UINT sdk_version)
{
   if (sdk_version != D3D_SDK_VERSION && sdk_version != (D3D_SDK_VERSION | 0x80000000))
      return nullptr;
   return new w32::gfx::Direct3D9;
}

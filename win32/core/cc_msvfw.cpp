/*
 * cc_msvfw.cpp:  msvfw32's video codec interface (ICOpen, ICSendMessage,
 *   ICDecompress) with the one codec the client's animations use: Microsoft
 *   RLE ('mrle', 'RLE '), 8- and 4-bit run-length DIBs decoded to plain DIBs of
 *   the same depth and color table.  The animation control finds these by
 *   GetProcAddress on LoadLibrary("msvfw32.dll"), a built-in module.
 */

#include "core/handles.h"
#include "core/rle.h"

#include <mmsystem.h>
#include <vfw.h>

#include <cstring>

using namespace w32;

namespace {

struct Codec
{
   DWORD handler;
   bool begun = false;
};

HandleTable<Codec, HIC> &Codecs()
{
   static HandleTable<Codec, HIC> codecs(kMiscHandleBase + 0x5000000);
   return codecs;
}

DWORD Lower(DWORD fourcc)
{
   DWORD out = 0;
   for (int i = 0; i < 4; i++)
      out |= (DWORD) tolower((int) ((fourcc >> (8 * i)) & 0xFF)) << (8 * i);
   return out;
}

bool IsRle(DWORD handler)
{
   DWORD h = Lower(handler);
   return h == mmioFOURCC('m', 'r', 'l', 'e') || h == mmioFOURCC('r', 'l', 'e', ' ');
}

// Inputs the codec takes: RLE8 or RLE4, bottom-up.
bool Supported(const BITMAPINFOHEADER *in)
{
   return in && in->biHeight > 0 && in->biWidth > 0 &&
          ((in->biCompression == BI_RLE8 && in->biBitCount == 8) ||
           (in->biCompression == BI_RLE4 && in->biBitCount == 4));
}

DWORD ColorCount(const BITMAPINFOHEADER *bih)
{
   if (bih->biBitCount > 8)
      return 0;
   return bih->biClrUsed ? bih->biClrUsed : 1u << bih->biBitCount;
}

DWORD Stride(const BITMAPINFOHEADER *bih)
{
   return ((bih->biWidth * bih->biBitCount + 31) / 32) * 4;
}

}  // namespace

namespace w32 {

namespace {

void PutPixel(BYTE *row, int x, int bpp, BYTE value)
{
   if (bpp == 8)
      row[x] = value;
   else if (x & 1)
      row[x / 2] = (BYTE) ((row[x / 2] & 0xF0) | (value & 0x0F));
   else
      row[x / 2] = (BYTE) ((row[x / 2] & 0x0F) | (value << 4));
}

}  // namespace

void DecodeRle(const BITMAPINFOHEADER *in, const BYTE *data, DWORD size, BYTE *out, DWORD stride)
{
   int bpp = in->biBitCount, width = in->biWidth, height = in->biHeight;
   const BYTE *p = data, *end = data + size;
   int x = 0, y = 0;
   auto put = [&](BYTE v) {
      if (x < width && y < height)
         PutPixel(out + (size_t) y * stride, x, bpp, v);
      x++;
   };
   while (p + 1 < end && y < height)
   {
      BYTE count = p[0], value = p[1];
      p += 2;
      if (count > 0)
      {
         // A run: count pixels; RLE4 alternates the two nibbles.
         for (int i = 0; i < count; i++)
            put(bpp == 8 ? value : (BYTE) ((i & 1) ? (value & 0x0F) : (value >> 4)));
         continue;
      }
      switch (value)
      {
      case 0:   // end of line
         x = 0;
         y++;
         break;
      case 1:   // end of bitmap
         return;
      case 2:   // delta
         if (p + 1 >= end)
            return;
         x += p[0];
         y += p[1];
         p += 2;
         break;
      default:
      {
         // Absolute: value literal pixels, padded to a WORD.
         int n = value;
         int bytes = bpp == 8 ? n : (n + 1) / 2;
         if (p + bytes > end)
            return;
         for (int i = 0; i < n; i++)
            put(bpp == 8 ? p[i] : (BYTE) ((i & 1) ? (p[i / 2] & 0x0F) : (p[i / 2] >> 4)));
         p += (bytes + 1) & ~1;
         break;
      }
      }
   }
}

bool DecodeRleDib(const BITMAPINFO *bi, const void *bits, std::vector<BYTE> *info, std::vector<BYTE> *pixels)
{
   const BITMAPINFOHEADER &h = bi->bmiHeader;
   if (!Supported(&h) || bits == nullptr)
      return false;
   DWORD colors = ColorCount(&h);
   info->assign((const BYTE *) bi, (const BYTE *) bi + h.biSize + colors * sizeof(RGBQUAD));
   auto *out = (BITMAPINFOHEADER *) info->data();
   out->biCompression = BI_RGB;
   out->biSizeImage = Stride(&h) * h.biHeight;
   pixels->assign(out->biSizeImage, 0);
   // Without a size, the data ends at its end-of-bitmap code.
   DecodeRle(&h, (const BYTE *) bits, h.biSizeImage ? h.biSizeImage : 0x7FFFFFFF, pixels->data(), Stride(&h));
   return true;
}

}  // namespace w32

extern "C" {

W32_EXPORT HIC VFWAPI ICOpen(DWORD type, DWORD handler, UINT mode)
{
   if ((type != ICTYPE_VIDEO && type != 0) || mode != ICMODE_DECOMPRESS || !IsRle(handler))
      return nullptr;
   return Codecs().Add(new Codec{handler});
}

W32_EXPORT LRESULT VFWAPI ICClose(HIC hic)
{
   Codec *c = Codecs().Remove(hic);
   if (c == nullptr)
      return ICERR_BADHANDLE;
   delete c;
   return ICERR_OK;
}

W32_EXPORT LRESULT VFWAPI ICSendMessage(HIC hic, UINT msg, DWORD_PTR param1, DWORD_PTR param2)
{
   Codec *c = Codecs().Get(hic);
   if (c == nullptr)
      return ICERR_BADHANDLE;
   auto *in = (const BITMAPINFOHEADER *) param1;
   auto *out = (BITMAPINFOHEADER *) param2;
   switch (msg)
   {
   case ICM_DECOMPRESS_GET_FORMAT:
   {
      if (!Supported(in))
         return ICERR_BADFORMAT;
      DWORD colors = ColorCount(in);
      LRESULT need = sizeof(BITMAPINFOHEADER) + colors * sizeof(RGBQUAD);
      if (out == nullptr)
         return need;
      memcpy(out, in, need < (LRESULT) (in->biSize + colors * sizeof(RGBQUAD)) ? need : in->biSize);
      out->biSize = sizeof(BITMAPINFOHEADER);
      out->biCompression = BI_RGB;
      out->biSizeImage = Stride(in) * in->biHeight;
      out->biClrUsed = colors;
      memcpy((BYTE *) out + sizeof(BITMAPINFOHEADER), (const BYTE *) in + in->biSize, colors * sizeof(RGBQUAD));
      return ICERR_OK;
   }
   case ICM_DECOMPRESS_QUERY:
      return Supported(in) ? ICERR_OK : ICERR_BADFORMAT;
   case ICM_DECOMPRESS_BEGIN:
      if (!Supported(in) || out == nullptr || out->biBitCount != in->biBitCount || out->biCompression != BI_RGB)
         return ICERR_BADFORMAT;
      c->begun = true;
      return ICERR_OK;
   case ICM_DECOMPRESS_END:
      c->begun = false;
      return ICERR_OK;
   case ICM_DECOMPRESS:
   {
      auto *d = (ICDECOMPRESS *) param1;
      if (d == nullptr || !Supported(d->lpbiInput) || d->lpOutput == nullptr || d->lpbiOutput == nullptr)
         return ICERR_BADFORMAT;
      DecodeRle(d->lpbiInput, (const BYTE *) d->lpInput, d->lpbiInput->biSizeImage, (BYTE *) d->lpOutput,
                Stride(d->lpbiOutput));
      return ICERR_OK;
   }
   }
   return ICERR_UNSUPPORTED;
}

W32_EXPORT DWORD VFWAPIV ICDecompress(HIC hic, DWORD flags, LPBITMAPINFOHEADER in, LPVOID data,
                                      LPBITMAPINFOHEADER out, LPVOID bits)
{
   ICDECOMPRESS d = {};
   d.dwFlags = flags;
   d.lpbiInput = in;
   d.lpInput = data;
   d.lpbiOutput = out;
   d.lpOutput = bits;
   return (DWORD) ICSendMessage(hic, ICM_DECOMPRESS, (DWORD_PTR) &d, sizeof(d));
}

}  // extern "C"

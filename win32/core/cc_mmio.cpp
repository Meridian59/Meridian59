/*
 * cc_mmio.cpp:  winmm's buffered file I/O (mmio) for reading RIFF files, from
 *   memory (FOURCC_MEM, as the animation control opens AVI resources) or a file.
 *   Read-only: nothing in the client writes RIFF files.
 */

#include "core/handles.h"
#include "core/paths.h"
#include "core/winproc.h"

#include <mmsystem.h>

#include <cstdio>
#include <cstring>
#include <vector>

using namespace w32;

namespace {

struct Stream
{
   std::vector<BYTE> file;      // a file's contents (read whole)
   const BYTE *data = nullptr;
   LONG size = 0;
   LONG pos = 0;
};

HandleTable<Stream, HMMIO> &Streams()
{
   static HandleTable<Stream, HMMIO> streams(kMiscHandleBase + 0x4000000);
   return streams;
}

HMMIO Open(const char *name, MMIOINFO *info, DWORD flags)
{
   if (flags & (MMIO_WRITE | MMIO_READWRITE | MMIO_CREATE | MMIO_DELETE | MMIO_PARSE | MMIO_EXIST | MMIO_GETTEMP))
   {
      if (info)
         info->wErrorRet = MMIOERR_CANNOTWRITE;
      return nullptr;
   }
   auto *s = new Stream;
   if (info && info->fccIOProc == FOURCC_MEM)
   {
      if (info->pchBuffer == nullptr)
      {
         delete s;
         info->wErrorRet = MMIOERR_OUTOFMEMORY;
         return nullptr;
      }
      s->data = (const BYTE *) info->pchBuffer;
      s->size = info->cchBuffer;
   }
   else
   {
      FILE *f = name ? fopen(HostPath(name).c_str(), "rb") : nullptr;
      if (f == nullptr)
      {
         delete s;
         if (info)
            info->wErrorRet = MMIOERR_CANNOTOPEN;
         return nullptr;
      }
      BYTE buf[65536];
      size_t n;
      while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
         s->file.insert(s->file.end(), buf, buf + n);
      fclose(f);
      s->data = s->file.data();
      s->size = (LONG) s->file.size();
   }
   if (info)
      info->wErrorRet = MMSYSERR_NOERROR;
   return Streams().Add(s);
}

bool ReadDword(Stream *s, DWORD *out)
{
   if (s->pos + 4 > s->size)
      return false;
   memcpy(out, s->data + s->pos, 4);
   s->pos += 4;
   return true;
}

}  // namespace

extern "C" {

HMMIO WINAPI mmioOpenA(LPSTR name, MMIOINFO *info, DWORD flags)
{
   return Open(name, info, flags);
}

HMMIO WINAPI mmioOpenW(LPWSTR name, MMIOINFO *info, DWORD flags)
{
   std::string a = name ? WideToAnsi(name) : std::string();
   return Open(name ? a.c_str() : nullptr, info, flags);
}

MMRESULT WINAPI mmioClose(HMMIO h, UINT)
{
   Stream *s = Streams().Remove(h);
   if (s == nullptr)
      return MMSYSERR_INVALHANDLE;
   delete s;
   return MMSYSERR_NOERROR;
}

LONG WINAPI mmioRead(HMMIO h, HPSTR buffer, LONG count)
{
   Stream *s = Streams().Get(h);
   if (s == nullptr || buffer == nullptr || count < 0)
      return -1;
   LONG n = std::max<LONG>(0, std::min(count, s->size - s->pos));
   memcpy(buffer, s->data + s->pos, n);
   s->pos += n;
   return n;
}

LONG WINAPI mmioSeek(HMMIO h, LONG offset, INT origin)
{
   Stream *s = Streams().Get(h);
   if (s == nullptr)
      return -1;
   LONG base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? s->pos : origin == SEEK_END ? s->size : -1;
   if (base < 0 || base + offset < 0)
      return -1;
   s->pos = base + offset;   // past the end is allowed; reads then return 0
   return s->pos;
}

MMRESULT WINAPI mmioDescend(HMMIO h, MMCKINFO *ck, const MMCKINFO *parent, UINT flags)
{
   Stream *s = Streams().Get(h);
   if (s == nullptr)
      return MMSYSERR_INVALHANDLE;
   if (ck == nullptr)
      return MMSYSERR_INVALPARAM;
   LONG end = parent ? (LONG) (parent->dwDataOffset + parent->cksize) : s->size;
   FOURCC want_id = ck->ckid, want_type = ck->fccType;
   if (flags & MMIO_FINDRIFF)
      want_id = FOURCC_RIFF;
   else if (flags & MMIO_FINDLIST)
      want_id = FOURCC_LIST;
   bool find = (flags & (MMIO_FINDCHUNK | MMIO_FINDRIFF | MMIO_FINDLIST)) != 0;
   for (;;)
   {
      LONG start = s->pos;
      DWORD id, size;
      if (start + 8 > end || !ReadDword(s, &id) || !ReadDword(s, &size))
      {
         s->pos = start;
         return MMIOERR_CHUNKNOTFOUND;
      }
      ck->ckid = id;
      ck->cksize = size;
      ck->dwDataOffset = start + 8;
      ck->dwFlags = 0;
      ck->fccType = 0;
      bool form = id == FOURCC_RIFF || id == FOURCC_LIST;
      if (form)
      {
         DWORD type;
         if (!ReadDword(s, &type))
         {
            s->pos = start;
            return MMIOERR_CHUNKNOTFOUND;
         }
         ck->fccType = type;
      }
      bool match = !find || (id == want_id && (!(flags & (MMIO_FINDRIFF | MMIO_FINDLIST)) || ck->fccType == want_type));
      if (match)
      {
         // At the data, after a form's type.
         s->pos = ck->dwDataOffset + (form ? 4 : 0);
         return MMSYSERR_NOERROR;
      }
      s->pos = ck->dwDataOffset + size + (size & 1);
   }
}

MMRESULT WINAPI mmioAscend(HMMIO h, MMCKINFO *ck, UINT)
{
   Stream *s = Streams().Get(h);
   if (s == nullptr)
      return MMSYSERR_INVALHANDLE;
   if (ck == nullptr)
      return MMSYSERR_INVALPARAM;
   s->pos = ck->dwDataOffset + ck->cksize + (ck->cksize & 1);
   return MMSYSERR_NOERROR;
}

FOURCC WINAPI mmioStringToFOURCCA(LPCSTR s, UINT flags)
{
   char c[4] = {' ', ' ', ' ', ' '};
   for (int i = 0; i < 4 && s && s[i]; i++)
      c[i] = (flags & MMIO_TOUPPER) ? (char) toupper((unsigned char) s[i]) : s[i];
   return mmioFOURCC(c[0], c[1], c[2], c[3]);
}

}  // extern "C"

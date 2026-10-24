/*
 * gdi_wide.cpp:  Unicode versions of GDI and resource functions, through the
 *   ANSI ones (text in the code page) except where resources are UTF-16 already.
 */

#include "core/gdi.h"
#include "core/module.h"
#include "core/winproc.h"

#include <cstring>

using namespace w32;

INT WINAPI DrawTextW(HDC hdc, LPCWSTR text, INT count, RECT *rect, UINT flags)
{
   if (text == nullptr)
      return 0;
   std::string a = WideToAnsi(text, count);
   return DrawTextA(hdc, a.c_str(), (INT) a.size(), rect, flags & ~DT_MODIFYSTRING);
}

BOOL WINAPI GetTextMetricsW(HDC hdc, TEXTMETRICW *tm)
{
   TEXTMETRICA a;
   if (tm == nullptr || !GetTextMetricsA(hdc, &a))
      return FALSE;
   tm->tmHeight = a.tmHeight;
   tm->tmAscent = a.tmAscent;
   tm->tmDescent = a.tmDescent;
   tm->tmInternalLeading = a.tmInternalLeading;
   tm->tmExternalLeading = a.tmExternalLeading;
   tm->tmAveCharWidth = a.tmAveCharWidth;
   tm->tmMaxCharWidth = a.tmMaxCharWidth;
   tm->tmWeight = a.tmWeight;
   tm->tmOverhang = a.tmOverhang;
   tm->tmDigitizedAspectX = a.tmDigitizedAspectX;
   tm->tmDigitizedAspectY = a.tmDigitizedAspectY;
   tm->tmFirstChar = 0x20;
   tm->tmLastChar = 0xFFFC;
   tm->tmDefaultChar = 0x1F;
   tm->tmBreakChar = 0x20;
   tm->tmItalic = a.tmItalic;
   tm->tmUnderlined = a.tmUnderlined;
   tm->tmStruckOut = a.tmStruckOut;
   tm->tmPitchAndFamily = a.tmPitchAndFamily;
   tm->tmCharSet = a.tmCharSet;
   return TRUE;
}

INT WINAPI GetObjectW(HGDIOBJ h, INT size, LPVOID out)
{
   if (GetObjectType(h) != OBJ_FONT)
      return GetObjectA(h, size, out);
   LOGFONTA a;
   if (!GetObjectA(h, sizeof(a), &a))
      return 0;
   if (out == nullptr)
      return sizeof(LOGFONTW);
   LOGFONTW w;
   memcpy(&w, &a, offsetof(LOGFONTA, lfFaceName));
   std::basic_string<WCHAR> face = AnsiToWide(a.lfFaceName);
   memset(w.lfFaceName, 0, sizeof(w.lfFaceName));
   memcpy(w.lfFaceName, face.data(), std::min(face.size(), (size_t) LF_FACESIZE - 1) * sizeof(WCHAR));
   size = std::min<INT>(size, sizeof(w));
   memcpy(out, &w, size);
   return size;
}

BOOL WINAPI PolyPolyline(HDC hdc, const POINT *points, const DWORD *counts, DWORD polylines)
{
   for (DWORD i = 0; i < polylines; i++)
   {
      if (!Polyline(hdc, points, (INT) counts[i]))
         return FALSE;
      points += counts[i];
   }
   return TRUE;
}

HRSRC WINAPI FindResourceW(HMODULE module, LPCWSTR name, LPCWSTR type)
{
   return FindResourceA(module, AnsiArg(name), AnsiArg(type));
}

INT WINAPI LoadStringW(HINSTANCE instance, UINT id, LPWSTR buffer, INT size)
{
   // String tables: block (id / 16) + 1 holds 16 counted UTF-16 strings.  With
   // size 0, the buffer receives a pointer to the string in the resource.
   const BYTE *data;
   DWORD length;
   if (buffer == nullptr)
      return 0;
   if (!FindResourceData(instance, MAKEINTRESOURCEA((id >> 4) + 1), (LPCSTR) RT_STRING, &data, &length))
   {
      if (size > 0)
         buffer[0] = 0;
      return 0;
   }
   const WORD *p = (const WORD *) data, *end = (const WORD *) (data + length);
   for (UINT i = 0; i < (id & 15) && p < end; i++)
      p += 1 + *p;
   if (p >= end)
   {
      if (size > 0)
         buffer[0] = 0;
      return 0;
   }
   int n = *p;
   if (size == 0)
   {
      *(const WCHAR **) buffer = (const WCHAR *) (p + 1);
      return n;
   }
   n = std::min(n, size - 1);
   memcpy(buffer, p + 1, n * sizeof(WCHAR));
   buffer[n] = 0;
   return n;
}

HANDLE WINAPI LoadImageW(HINSTANCE instance, LPCWSTR name, UINT type, INT cx, INT cy, UINT flags)
{
   return LoadImageA(instance, AnsiArg(name), type, cx, cy, flags);
}

/*
 * ctl_usp10.cpp:  The part of Uniscribe (usp10.dll) Wine's edit control uses:
 *   ScriptString* analyses of a line of text in the DC's font, left to right,
 *   with tab stops and password characters, measured with the GDI text
 *   extents (core/native.h CharEdges) and drawn with ExtTextOutW, so caret
 *   positions are where the characters are drawn.  ScriptBreak finds word
 *   starts after white space.
 */

#include "core/win32.h"

#include <usp10.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

namespace {

struct Analysis
{
   HDC hdc = nullptr;
   std::basic_string<WCHAR> text;
   std::vector<int> edges;           // trailing edge of each character
   std::vector<SCRIPT_LOGATTR> attrs;
   SIZE size = {0, 0};
   int count = 0;
   struct Run
   {
      int start, length, x;
   };
   std::vector<Run> runs;            // tab-free stretches, at their x
};

Analysis *Get(SCRIPT_STRING_ANALYSIS ssa)
{
   return static_cast<Analysis *>(ssa);
}

int Leading(const Analysis *a, int i)
{
   return i <= 0 ? 0 : a->edges[std::min(i, a->count) - 1];
}

// The next tab stop after x (pixels).
int NextTabStop(int x, const SCRIPT_TABDEF *tabs, int average)
{
   int scale = (tabs && tabs->iScale) ? tabs->iScale : average * 4;
   int origin = tabs ? tabs->iTabOrigin : 0;
   if (tabs && tabs->cTabStops > 1 && tabs->pTabStops)
      for (int i = 0; i < tabs->cTabStops; i++)
      {
         int stop = origin + abs(tabs->pTabStops[i]) * scale / 4;
         if (stop > x)
            return stop;
      }
   int interval = (tabs && tabs->cTabStops == 1 && tabs->pTabStops) ? tabs->pTabStops[0] * scale / 4 : 8 * scale / 4;
   if (interval <= 0)
      interval = 8 * std::max(average, 1);
   return origin + ((x - origin) / interval + 1) * interval;
}

void Break(const WCHAR *s, int n, SCRIPT_LOGATTR *attrs)
{
   for (int i = 0; i < n; i++)
   {
      bool space = iswspace(s[i]) != 0;
      bool prev_space = i > 0 && iswspace(s[i - 1]);
      attrs[i] = {};
      attrs[i].fWhiteSpace = space;
      attrs[i].fCharStop = 1;
      attrs[i].fSoftBreak = i > 0 && prev_space && !space;
      attrs[i].fWordStop = (i == 0 && !space) || (prev_space && !space);
   }
}

}  // namespace

W32_EXPORT HRESULT WINAPI ScriptStringAnalyse(HDC hdc, const void *string, int length, int, int charset, DWORD flags, int,
                                   SCRIPT_CONTROL *, SCRIPT_STATE *, const int *, SCRIPT_TABDEF *tabs, const BYTE *,
                                   SCRIPT_STRING_ANALYSIS *ssa)
{
   if (string == nullptr || length < 1 || ssa == nullptr)
      return E_INVALIDARG;
   auto *a = new Analysis;
   a->hdc = hdc;
   a->count = length;
   if (charset != -1)
   {
      // Narrow text in the code page.
      int n = MultiByteToWideChar(CP_ACP, 0, (const char *) string, length, nullptr, 0);
      a->text.resize(n);
      MultiByteToWideChar(CP_ACP, 0, (const char *) string, length, a->text.data(), n);
   }
   else if (flags & SSA_PASSWORD)
      a->text.assign(length, *(const WCHAR *) string);
   else
      a->text.assign((const WCHAR *) string, length);
   a->count = (int) a->text.size();

   TEXTMETRICW tm;
   GetTextMetricsW(hdc, &tm);
   a->edges.resize(a->count);
   int x = 0;
   for (int i = 0; i < a->count;)
   {
      if (a->text[i] == '\t')
      {
         x = NextTabStop(x, (flags & SSA_TAB) ? tabs : nullptr, tm.tmAveCharWidth);
         a->edges[i++] = x;
         continue;
      }
      int j = i;
      while (j < a->count && a->text[j] != '\t')
         j++;
      std::vector<INT> dx(j - i);
      SIZE size;
      GetTextExtentExPointW(hdc, a->text.c_str() + i, j - i, 0, nullptr, dx.data(), &size);
      for (int k = i; k < j; k++)
         a->edges[k] = x + dx[k - i];
      a->runs.push_back({i, j - i, x});
      x += j > i ? dx[j - i - 1] : 0;
      i = j;
   }
   a->size = {x, tm.tmHeight};
   a->attrs.resize(a->count);
   Break(a->text.c_str(), a->count, a->attrs.data());
   *ssa = a;
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringFree(SCRIPT_STRING_ANALYSIS *ssa)
{
   if (ssa == nullptr)
      return E_INVALIDARG;
   delete Get(*ssa);
   *ssa = nullptr;
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringValidate(SCRIPT_STRING_ANALYSIS ssa)
{
   return ssa ? S_OK : E_INVALIDARG;
}

W32_EXPORT const SIZE *WINAPI ScriptString_pSize(SCRIPT_STRING_ANALYSIS ssa)
{
   return ssa ? &Get(ssa)->size : nullptr;
}

W32_EXPORT const int *WINAPI ScriptString_pcOutChars(SCRIPT_STRING_ANALYSIS ssa)
{
   return ssa ? &Get(ssa)->count : nullptr;
}

W32_EXPORT const SCRIPT_LOGATTR *WINAPI ScriptString_pLogAttr(SCRIPT_STRING_ANALYSIS ssa)
{
   return ssa ? Get(ssa)->attrs.data() : nullptr;
}

W32_EXPORT HRESULT WINAPI ScriptStringGetLogicalWidths(SCRIPT_STRING_ANALYSIS ssa, int *dx)
{
   Analysis *a = Get(ssa);
   if (a == nullptr || dx == nullptr)
      return E_INVALIDARG;
   for (int i = 0; i < a->count; i++)
      dx[i] = a->edges[i] - Leading(a, i);
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringGetOrder(SCRIPT_STRING_ANALYSIS ssa, UINT *order)
{
   Analysis *a = Get(ssa);
   if (a == nullptr || order == nullptr)
      return E_INVALIDARG;
   for (int i = 0; i < a->count; i++)
      order[i] = i;   // left to right
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringCPtoX(SCRIPT_STRING_ANALYSIS ssa, int cp, BOOL trailing, int *x)
{
   Analysis *a = Get(ssa);
   if (a == nullptr || x == nullptr)
      return E_INVALIDARG;
   if (cp < 0)
      *x = 0;
   else if (cp >= a->count)
      *x = a->size.cx;
   else
      *x = trailing ? a->edges[cp] : Leading(a, cp);
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringXtoCP(SCRIPT_STRING_ANALYSIS ssa, int x, int *ch, int *trailing)
{
   Analysis *a = Get(ssa);
   if (a == nullptr || ch == nullptr || trailing == nullptr)
      return E_INVALIDARG;
   if (x < 0)
   {
      *ch = -1;
      *trailing = TRUE;
      return S_OK;
   }
   for (int i = 0; i < a->count; i++)
      if (x < a->edges[i])
      {
         int lead = Leading(a, i);
         *ch = i;
         *trailing = x >= lead + (a->edges[i] - lead) / 2 + ((a->edges[i] - lead) & 1);
         return S_OK;
      }
   *ch = a->count;
   *trailing = FALSE;
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptStringOut(SCRIPT_STRING_ANALYSIS ssa, int x, int y, UINT options, const RECT *clip, int min_sel,
                               int max_sel, BOOL disabled)
{
   Analysis *a = Get(ssa);
   if (a == nullptr)
      return E_INVALIDARG;
   HDC hdc = a->hdc;
   UINT clip_flag = (clip && (options & ETO_CLIPPED)) ? ETO_CLIPPED : 0;
   if (clip && (options & ETO_OPAQUE))
      ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, clip, nullptr, 0, nullptr);
   for (const Analysis::Run &r : a->runs)
      ExtTextOutW(hdc, x + r.x, y, clip_flag, clip, a->text.c_str() + r.start, r.length, nullptr);

   min_sel = std::max(min_sel, 0);
   max_sel = std::min(max_sel, a->count);
   if (min_sel >= max_sel)
      return S_OK;
   // The selection: its cell in the highlight colors, the text over it again.
   RECT sel = {x + Leading(a, min_sel), y, x + a->edges[max_sel - 1], y + a->size.cy};
   if (clip)
      IntersectRect(&sel, &sel, clip);
   if (IsRectEmpty(&sel))
      return S_OK;
   COLORREF text = SetTextColor(hdc, GetSysColor(disabled ? COLOR_GRAYTEXT : COLOR_HIGHLIGHTTEXT));
   COLORREF back = SetBkColor(hdc, GetSysColor(disabled ? COLOR_BTNFACE : COLOR_HIGHLIGHT));
   int mode = SetBkMode(hdc, TRANSPARENT);
   ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &sel, nullptr, 0, nullptr);
   for (const Analysis::Run &r : a->runs)
      ExtTextOutW(hdc, x + r.x, y, ETO_CLIPPED, &sel, a->text.c_str() + r.start, r.length, nullptr);
   SetBkMode(hdc, mode);
   SetBkColor(hdc, back);
   SetTextColor(hdc, text);
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptBreak(const WCHAR *chars, int count, const SCRIPT_ANALYSIS *, SCRIPT_LOGATTR *attrs)
{
   if (chars == nullptr || count <= 0 || attrs == nullptr)
      return E_INVALIDARG;
   Break(chars, count, attrs);
   return S_OK;
}

W32_EXPORT HRESULT WINAPI ScriptIsComplex(const WCHAR *, int, DWORD)
{
   return S_FALSE;   // left-to-right text needs no shaping here
}

W32_EXPORT HRESULT WINAPI ScriptFreeCache(SCRIPT_CACHE *cache)
{
   if (cache)
      *cache = nullptr;
   return S_OK;
}

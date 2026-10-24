/*
 * richedit.cpp:  The "RichEdit" class (riched32.dll, Rich Edit 1.0) as the
 *   client uses it: read-only text with per-character color, bold, italic and
 *   underline, word-wrapped to the window, scrolled by lines with a vertical
 *   scroll bar, selectable with the mouse and copied with Ctrl+C.  Text keeps its
 *   "\r\n" line breaks, two characters each, as Rich Edit 1.0 counts them.
 */

#include "core/gdi.h"
#include "core/window.h"

#include <richedit.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace w32 {

namespace {

struct Format
{
   COLORREF color = 0;
   bool auto_color = true;
   bool bold = false, italic = false, underline = false;
   bool operator==(const Format &o) const
   {
      return color == o.color && auto_color == o.auto_color && bold == o.bold && italic == o.italic &&
             underline == o.underline;
   }
};

struct Line
{
   int start, end;                     // character range, without the line break
};

struct RichEdit
{
   std::string text;
   std::vector<uint16_t> attr;         // per character, index into formats
   std::vector<Format> formats = {Format()};
   uint16_t insert_format = 0;         // what typed/inserted text gets at an empty selection
   int sel_start = 0, sel_end = 0;
   int anchor = 0;
   bool selecting = false;
   int limit = 32767;
   bool read_only = false;
   COLORREF background = 0;
   bool system_background = true;
   HFONT font = nullptr;
   HFONT variants[8] = {};             // bold | italic << 1 | underline << 2
   int line_height = 16;
   std::vector<Line> lines;
   bool layout_valid = false;
   int first_line = 0;
};

RichEdit *Get(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return w ? static_cast<RichEdit *>(w->control_state) : nullptr;
}

uint16_t AddFormat(RichEdit *e, const Format &f)
{
   for (size_t i = 0; i < e->formats.size(); i++)
      if (e->formats[i] == f)
         return (uint16_t) i;
   e->formats.push_back(f);
   return (uint16_t) (e->formats.size() - 1);
}

void DropFonts(RichEdit *e)
{
   for (HFONT &f : e->variants)
      if (f)
      {
         DeleteObject(f);
         f = nullptr;
      }
}

HFONT FontFor(RichEdit *e, const Format &f)
{
   int index = (f.bold ? 1 : 0) | (f.italic ? 2 : 0) | (f.underline ? 4 : 0);
   if (e->variants[index] == nullptr)
   {
      LOGFONTA lf;
      HFONT base = e->font ? e->font : (HFONT) GetStockObject(DEFAULT_GUI_FONT);
      GetObjectA(base, sizeof(lf), &lf);
      if (f.bold)
         lf.lfWeight = FW_BOLD;
      if (f.italic)
         lf.lfItalic = TRUE;
      if (f.underline)
         lf.lfUnderline = TRUE;
      e->variants[index] = CreateFontIndirectA(&lf);
   }
   return e->variants[index];
}

RECT TextArea(HWND hwnd)
{
   RECT r;
   GetClientRect(hwnd, &r);
   r.left += 1;   // Rich Edit's left margin
   return r;
}

int CharWidth(HDC hdc, RichEdit *e, int i)
{
   SelectObject(hdc, FontFor(e, e->formats[e->attr[i]]));
   SIZE s;
   GetTextExtentPoint32A(hdc, &e->text[i], 1, &s);
   return s.cx;
}

// Word-wraps the text into lines of the text area's width.
void Layout(HWND hwnd, RichEdit *e)
{
   if (e->layout_valid)
      return;
   e->layout_valid = true;
   e->lines.clear();
   RECT area = TextArea(hwnd);
   int width = std::max(1, (int) (area.right - area.left));
   HDC hdc = GetDC(nullptr);
   HGDIOBJ old = SelectObject(hdc, FontFor(e, Format()));
   TEXTMETRICA tm;
   GetTextMetricsA(hdc, &tm);
   e->line_height = std::max(1, (int) tm.tmHeight);

   int n = (int) e->text.size();
   int start = 0;
   while (start <= n)
   {
      // A paragraph: up to "\r\n" (or a lone "\r" / "\n").
      int end = start;
      while (end < n && e->text[end] != '\r' && e->text[end] != '\n')
         end++;
      int pos = start;
      do
      {
         int x = 0, last_break = -1, i = pos;
         for (; i < end; i++)
         {
            int cw = CharWidth(hdc, e, i);
            if (x + cw > width && i > pos)
               break;
            x += cw;
            if (e->text[i] == ' ')
               last_break = i + 1;
         }
         int line_end = i;
         if (i < end && last_break > pos)
            line_end = last_break;
         e->lines.push_back({pos, line_end});
         pos = line_end;
      } while (pos < end);
      if (end >= n)
         break;
      start = end + ((e->text[end] == '\r' && end + 1 < n && e->text[end + 1] == '\n') ? 2 : 1);
      if (start >= n)
      {
         e->lines.push_back({n, n});   // the empty line after a final break
         break;
      }
   }
   if (e->lines.empty())
      e->lines.push_back({0, 0});
   SelectObject(hdc, old);
   DeleteDC(hdc);
}

void UpdateScrollBar(HWND hwnd, RichEdit *e)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || !(w->style & WS_VSCROLL))
      return;
   RECT area = TextArea(hwnd);
   SCROLLINFO si = {sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL};
   si.nMin = 0;
   si.nMax = std::max(0, (int) e->lines.size() - 1);
   si.nPage = std::max(1, (int) (area.bottom - area.top) / e->line_height);
   si.nPos = e->first_line;
   SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void Changed(HWND hwnd, RichEdit *e)
{
   e->layout_valid = false;
   Layout(hwnd, e);
   e->first_line = std::clamp(e->first_line, 0, std::max(0, (int) e->lines.size() - 1));
   UpdateScrollBar(hwnd, e);
   InvalidateRect(hwnd, nullptr, TRUE);
}

void ScrollTo(HWND hwnd, RichEdit *e, int line)
{
   Layout(hwnd, e);
   line = std::clamp(line, 0, std::max(0, (int) e->lines.size() - 1));
   if (line == e->first_line)
      return;
   e->first_line = line;
   UpdateScrollBar(hwnd, e);
   InvalidateRect(hwnd, nullptr, TRUE);
}

int LineOfChar(RichEdit *e, int c)
{
   for (size_t i = 0; i < e->lines.size(); i++)
      if (c <= e->lines[i].end && (i + 1 == e->lines.size() || c < e->lines[i + 1].start))
         return (int) i;
   return (int) e->lines.size() - 1;
}

void Replace(HWND hwnd, RichEdit *e, const char *with)
{
   int a = std::min(e->sel_start, e->sel_end), b = std::max(e->sel_start, e->sel_end);
   std::string s = with ? with : "";
   if ((int) (e->text.size() - (b - a) + s.size()) > e->limit)
      s.resize(std::max(0, e->limit - (int) (e->text.size() - (b - a))));
   // Inserted text takes the format of the selection's start, or the insertion
   // point's.
   uint16_t f = a == b ? e->insert_format : e->attr[a];
   e->text.replace(a, b - a, s);
   e->attr.erase(e->attr.begin() + a, e->attr.begin() + b);
   e->attr.insert(e->attr.begin() + a, s.size(), f);
   e->sel_start = e->sel_end = e->anchor = a + (int) s.size();
   e->insert_format = f;
   Changed(hwnd, e);
}

Format Apply(Format f, const CHARFORMATA &cf)
{
   if (cf.dwMask & CFM_COLOR)
   {
      f.auto_color = (cf.dwEffects & CFE_AUTOCOLOR) != 0;
      f.color = cf.crTextColor;
   }
   if (cf.dwMask & CFM_BOLD)
      f.bold = (cf.dwEffects & CFE_BOLD) != 0;
   if (cf.dwMask & CFM_ITALIC)
      f.italic = (cf.dwEffects & CFE_ITALIC) != 0;
   if (cf.dwMask & CFM_UNDERLINE)
      f.underline = (cf.dwEffects & CFE_UNDERLINE) != 0;
   return f;
}

void SetFormat(HWND hwnd, RichEdit *e, WPARAM which, const CHARFORMATA *cf)
{
   if (cf == nullptr || cf->cbSize < sizeof(CHARFORMATA))
      return;
   if (which & (SCF_SELECTION | SCF_WORD))
   {
      int a = std::min(e->sel_start, e->sel_end), b = std::max(e->sel_start, e->sel_end);
      if (a == b)
         e->insert_format = AddFormat(e, Apply(e->formats[e->insert_format], *cf));
      for (int i = a; i < b; i++)
         e->attr[i] = AddFormat(e, Apply(e->formats[e->attr[i]], *cf));
   }
   else
   {
      // SCF_ALL / SCF_DEFAULT: everything, and new text.
      for (auto &a : e->attr)
         a = AddFormat(e, Apply(e->formats[a], *cf));
      e->insert_format = AddFormat(e, Apply(e->formats[e->insert_format], *cf));
   }
   Changed(hwnd, e);
}

int HitChar(HWND hwnd, RichEdit *e, int x, int y)
{
   Layout(hwnd, e);
   RECT area = TextArea(hwnd);
   int line = e->first_line + (y - area.top) / e->line_height;
   if (y < area.top)
      line = e->first_line - 1;
   line = std::clamp(line, 0, (int) e->lines.size() - 1);
   const Line &l = e->lines[line];
   HDC hdc = GetDC(nullptr);
   int px = area.left;
   int c = l.start;
   for (; c < l.end; c++)
   {
      int cw = CharWidth(hdc, e, c);
      if (x < px + cw / 2)
         break;
      px += cw;
   }
   DeleteDC(hdc);
   return c;
}

void Copy(RichEdit *e)
{
   int a = std::min(e->sel_start, e->sel_end), b = std::max(e->sel_start, e->sel_end);
   if (a == b || !OpenClipboard(nullptr))
      return;
   EmptyClipboard();
   HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, b - a + 1);
   char *p = (char *) GlobalLock(mem);
   memcpy(p, e->text.data() + a, b - a);
   p[b - a] = 0;
   GlobalUnlock(mem);
   SetClipboardData(CF_TEXT, mem);
   CloseClipboard();
}

void Paint(HWND hwnd, RichEdit *e, HDC hdc)
{
   Layout(hwnd, e);
   RECT client;
   GetClientRect(hwnd, &client);
   RECT area = TextArea(hwnd);
   COLORREF back = e->system_background ? GetSysColor(COLOR_WINDOW) : e->background;
   HBRUSH bg = CreateSolidBrush(back);
   FillRect(hdc, &client, bg);
   DeleteObject(bg);
   SetBkMode(hdc, TRANSPARENT);
   int a = std::min(e->sel_start, e->sel_end), b = std::max(e->sel_start, e->sel_end);
   HGDIOBJ old_font = SelectObject(hdc, FontFor(e, Format()));
   int y = area.top;
   for (int li = e->first_line; li < (int) e->lines.size() && y < area.bottom; li++, y += e->line_height)
   {
      const Line &l = e->lines[li];
      int x = area.left;
      int i = l.start;
      while (i < l.end)
      {
         // A run: same format and same selection state.
         bool selected = i >= a && i < b;
         int j = i + 1;
         while (j < l.end && e->attr[j] == e->attr[i] && (j >= a && j < b) == selected)
            j++;
         const Format &f = e->formats[e->attr[i]];
         SelectObject(hdc, FontFor(e, f));
         SIZE s;
         GetTextExtentPoint32A(hdc, &e->text[i], j - i, &s);
         if (selected)
         {
            RECT r = {x, y, x + s.cx, y + e->line_height};
            FillRect(hdc, &r, GetSysColorBrush(COLOR_HIGHLIGHT));
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
         }
         else
            SetTextColor(hdc, f.auto_color ? GetSysColor(COLOR_WINDOWTEXT) : f.color);
         TextOutA(hdc, x, y, &e->text[i], j - i);
         x += s.cx;
         i = j;
      }
   }
   SelectObject(hdc, old_font);
}

LRESULT CALLBACK RichEditProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   RichEdit *e = Get(hwnd);
   if (e == nullptr && msg != WM_NCCREATE)
      return DefWindowProcA(hwnd, msg, wparam, lparam);
   switch (msg)
   {
   case WM_NCCREATE:
   {
      Window *w = GetWindowObject(hwnd);
      e = new RichEdit;
      e->read_only = (w->style & ES_READONLY) != 0;
      w->control_state = e;
      return DefWindowProcA(hwnd, msg, wparam, lparam);
   }
   case WM_NCDESTROY:
   {
      DropFonts(e);
      delete e;
      GetWindowObject(hwnd)->control_state = nullptr;
      return DefWindowProcA(hwnd, msg, wparam, lparam);
   }
   case WM_SETFONT:
      e->font = (HFONT) wparam;
      DropFonts(e);
      e->layout_valid = false;
      if (LOWORD(lparam))
         Changed(hwnd, e);
      return 0;
   case WM_GETFONT:
      return (LRESULT) e->font;
   case WM_SIZE:
      Changed(hwnd, e);
      return 0;
   case WM_ERASEBKGND:
      return 1;   // WM_PAINT fills it
   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      Paint(hwnd, e, hdc);
      EndPaint(hwnd, &ps);
      return 0;
   }
   case WM_SETTEXT:
      e->sel_start = 0;
      e->sel_end = (int) e->text.size();
      Replace(hwnd, e, (const char *) lparam);
      e->sel_start = e->sel_end = e->anchor = 0;
      e->first_line = 0;
      Changed(hwnd, e);
      return TRUE;
   case WM_GETTEXT:
      if (wparam == 0 || lparam == 0)
         return 0;
      lstrcpynA((LPSTR) lparam, e->text.c_str(), (INT) wparam);
      return (LRESULT) strlen((LPSTR) lparam);
   case WM_GETTEXTLENGTH:
      return (LRESULT) e->text.size();
   case EM_SETSEL:
   {
      int n = (int) e->text.size();
      int s = (int) (INT) wparam, f = (int) (INT) lparam;
      if (s == -1)
         s = f = std::max(e->sel_start, e->sel_end);   // remove the selection
      else if (f == -1 || f > n)
         f = n;
      e->sel_start = std::clamp(s, 0, n);
      e->sel_end = std::clamp(f, 0, n);
      e->anchor = e->sel_start;
      InvalidateRect(hwnd, nullptr, FALSE);
      return 0;
   }
   case EM_GETSEL:
      if (wparam)
         *(DWORD *) wparam = std::min(e->sel_start, e->sel_end);
      if (lparam)
         *(DWORD *) lparam = std::max(e->sel_start, e->sel_end);
      return MAKELRESULT(std::min(e->sel_start, e->sel_end), std::max(e->sel_start, e->sel_end));
   case EM_EXSETSEL:
   {
      auto *r = (CHARRANGE *) lparam;
      if (r)
         SendMessageA(hwnd, EM_SETSEL, r->cpMin, r->cpMax);
      return std::max(e->sel_start, e->sel_end);
   }
   case EM_EXGETSEL:
      if (auto *r = (CHARRANGE *) lparam)
      {
         r->cpMin = std::min(e->sel_start, e->sel_end);
         r->cpMax = std::max(e->sel_start, e->sel_end);
      }
      return 0;
   case EM_REPLACESEL:
      Replace(hwnd, e, (const char *) lparam);
      return 0;
   case EM_SETCHARFORMAT:
      SetFormat(hwnd, e, wparam, (const CHARFORMATA *) lparam);
      return 1;
   case EM_GETCHARFORMAT:
   {
      auto *cf = (CHARFORMATA *) lparam;
      if (cf == nullptr || cf->cbSize < sizeof(CHARFORMATA))
         return 0;
      const Format &f = e->formats[(wparam & SCF_SELECTION) && e->sel_start != e->sel_end
                                      ? e->attr[std::min(e->sel_start, e->sel_end)]
                                      : e->insert_format];
      cf->dwMask = CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
      cf->dwEffects = (f.auto_color ? CFE_AUTOCOLOR : 0) | (f.bold ? CFE_BOLD : 0) | (f.italic ? CFE_ITALIC : 0) |
                      (f.underline ? CFE_UNDERLINE : 0);
      cf->crTextColor = f.color;
      return cf->dwMask;
   }
   case EM_SETBKGNDCOLOR:
   {
      COLORREF old = e->system_background ? GetSysColor(COLOR_WINDOW) : e->background;
      e->system_background = wparam != 0;
      e->background = (COLORREF) lparam;
      InvalidateRect(hwnd, nullptr, TRUE);
      return old;
   }
   case EM_LIMITTEXT:
      e->limit = wparam ? (int) wparam : 65536;
      return 0;
   case EM_EXLIMITTEXT:
      e->limit = lparam ? (int) lparam : 65536;
      return 0;
   case EM_GETLIMITTEXT:
      return e->limit;
   case EM_SETREADONLY:
      e->read_only = wparam != 0;
      return TRUE;
   case EM_GETLINECOUNT:
      Layout(hwnd, e);
      return (LRESULT) e->lines.size();
   case EM_GETFIRSTVISIBLELINE:
      Layout(hwnd, e);
      return e->first_line;
   case EM_LINESCROLL:
      ScrollTo(hwnd, e, e->first_line + (int) (INT) lparam);
      return TRUE;
   case EM_SCROLL:
   {
      RECT area = TextArea(hwnd);
      int page = std::max(1, (int) (area.bottom - area.top) / e->line_height);
      int by = wparam == SB_LINEUP ? -1 : wparam == SB_LINEDOWN ? 1 : wparam == SB_PAGEUP ? -page
                                                                     : wparam == SB_PAGEDOWN ? page : 0;
      int before = e->first_line;
      ScrollTo(hwnd, e, e->first_line + by);
      return MAKELRESULT(e->first_line - before, TRUE);
   }
   case EM_SCROLLCARET:
   {
      Layout(hwnd, e);
      RECT area = TextArea(hwnd);
      int page = std::max(1, (int) (area.bottom - area.top) / e->line_height);
      int line = LineOfChar(e, e->sel_end);
      if (line < e->first_line)
         ScrollTo(hwnd, e, line);
      else if (line >= e->first_line + page)
         ScrollTo(hwnd, e, line - page + 1);
      return 0;
   }
   case EM_GETRECT:
      if (lparam)
         *(RECT *) lparam = TextArea(hwnd);
      return 0;
   case EM_LINEINDEX:
   {
      Layout(hwnd, e);
      int line = (INT) wparam == -1 ? LineOfChar(e, e->sel_end) : (int) wparam;
      return line >= 0 && line < (int) e->lines.size() ? e->lines[line].start : -1;
   }
   case EM_LINEFROMCHAR:
      Layout(hwnd, e);
      return LineOfChar(e, (INT) wparam == -1 ? e->sel_end : (int) wparam);
   case EM_EXLINEFROMCHAR:
      Layout(hwnd, e);
      return LineOfChar(e, (int) lparam);
   case EM_LINELENGTH:
   {
      Layout(hwnd, e);
      int line = LineOfChar(e, (INT) wparam == -1 ? e->sel_end : (int) wparam);
      return e->lines[line].end - e->lines[line].start;
   }
   case EM_CANUNDO:
   case EM_UNDO:
      return 0;
   case EM_SETEVENTMASK:
   case EM_GETEVENTMASK:
   case EM_SETOPTIONS:
      return 0;
   case WM_VSCROLL:
   {
      RECT area = TextArea(hwnd);
      int page = std::max(1, (int) (area.bottom - area.top) / e->line_height);
      switch (LOWORD(wparam))
      {
      case SB_LINEUP: ScrollTo(hwnd, e, e->first_line - 1); break;
      case SB_LINEDOWN: ScrollTo(hwnd, e, e->first_line + 1); break;
      case SB_PAGEUP: ScrollTo(hwnd, e, e->first_line - page); break;
      case SB_PAGEDOWN: ScrollTo(hwnd, e, e->first_line + page); break;
      case SB_TOP: ScrollTo(hwnd, e, 0); break;
      case SB_BOTTOM: ScrollTo(hwnd, e, (int) e->lines.size()); break;
      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
      {
         SCROLLINFO si = {sizeof(si), SIF_TRACKPOS};
         GetScrollInfo(hwnd, SB_VERT, &si);
         ScrollTo(hwnd, e, si.nTrackPos);
         break;
      }
      }
      return 0;
   }
   case WM_MOUSEWHEEL:
      ScrollTo(hwnd, e, e->first_line - 3 * (SHORT) HIWORD(wparam) / WHEEL_DELTA);
      return 0;
   case WM_LBUTTONDOWN:
      SetFocus(hwnd);
      e->anchor = e->sel_start = e->sel_end = HitChar(hwnd, e, (SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam));
      e->selecting = true;
      SetCapture(hwnd);
      InvalidateRect(hwnd, nullptr, FALSE);
      return 0;
   case WM_MOUSEMOVE:
      if (e->selecting)
      {
         int c = HitChar(hwnd, e, (SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam));
         if (c != e->sel_end)
         {
            e->sel_start = e->anchor;
            e->sel_end = c;
            InvalidateRect(hwnd, nullptr, FALSE);
         }
         // Dragging past the edges scrolls.
         RECT area = TextArea(hwnd);
         SHORT y = (SHORT) HIWORD(lparam);
         if (y < area.top)
            ScrollTo(hwnd, e, e->first_line - 1);
         else if (y >= area.bottom)
            ScrollTo(hwnd, e, e->first_line + 1);
      }
      return 0;
   case WM_LBUTTONUP:
      if (e->selecting)
      {
         e->selecting = false;
         ReleaseCapture();
      }
      return 0;
   case WM_KEYDOWN:
   {
      bool ctrl = GetKeyState(VK_CONTROL) < 0;
      RECT area = TextArea(hwnd);
      int page = std::max(1, (int) (area.bottom - area.top) / e->line_height);
      switch (wparam)
      {
      case 'C':
      case VK_INSERT:
         if (ctrl)
            Copy(e);
         break;
      case 'A':
         if (ctrl)
         {
            e->sel_start = 0;
            e->sel_end = (int) e->text.size();
            InvalidateRect(hwnd, nullptr, FALSE);
         }
         break;
      case VK_UP: ScrollTo(hwnd, e, e->first_line - 1); break;
      case VK_DOWN: ScrollTo(hwnd, e, e->first_line + 1); break;
      case VK_PRIOR: ScrollTo(hwnd, e, e->first_line - page); break;
      case VK_NEXT: ScrollTo(hwnd, e, e->first_line + page); break;
      case VK_HOME: if (ctrl) ScrollTo(hwnd, e, 0); break;
      case VK_END: if (ctrl) ScrollTo(hwnd, e, (int) e->lines.size()); break;
      }
      return 0;
   }
   case WM_COPY:
      Copy(e);
      return 0;
   case WM_GETDLGCODE:
      return DLGC_WANTARROWS | DLGC_HASSETSEL;
   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

}  // namespace

void RegisterRichEdit()
{
   RegisterSystemClass("RichEdit", RichEditProc, CS_DBLCLKS | CS_GLOBALCLASS, 0, LoadCursorA(nullptr, IDC_IBEAM),
                       nullptr);
}

}  // namespace w32

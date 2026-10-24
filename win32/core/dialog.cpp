/*
 * dialog.cpp:  The dialog manager: DLGTEMPLATE(EX) parsing, creating dialogs and
 *   their controls, modal loops, DefDlgProc, keyboard navigation
 *   (IsDialogMessage), the item helpers and MessageBox.
 *
 *   Controls are used only through their class names and messages (WM_GETDLGCODE,
 *   BM_*, WM_SETFONT ...), whatever implements them.  Behaviour follows Windows
 *   as Wine's user32 reproduces it.
 */

#include "core/window.h"

#include "core/eventloop.h"
#include "core/message.h"
#include "core/module.h"
#include "core/native.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <map>
#include <strings.h>
#include <string>
#include <vector>

namespace w32 {

namespace {

/************************************************************************/
// Templates

struct ItemTemplate
{
   DWORD help = 0, style = 0, exstyle = 0;
   short x = 0, y = 0, cx = 0, cy = 0;
   DWORD id = 0;
   std::string cls;                  // name, or "" with cls_atom
   WORD cls_atom = 0;
   std::string title;
   bool title_is_ordinal = false;
   WORD title_ordinal = 0;
   const void *data = nullptr;       // creation data, at its size WORD; nullptr if none
};

struct DialogTemplate
{
   bool ex = false;
   DWORD help = 0, style = 0, exstyle = 0;
   short x = 0, y = 0, cx = 0, cy = 0;
   std::string menu;                 // name, or "" with menu_ordinal
   WORD menu_ordinal = 0;
   std::string cls;
   WORD cls_atom = 0;
   std::string title;
   WORD point_size = 0, weight = FW_DONTCARE;
   BYTE italic = FALSE, charset = DEFAULT_CHARSET;
   std::string face;
   std::vector<ItemTemplate> items;
};

// Reads a template; end is nullptr when its size isn't known (the *Indirect
// functions).  Fails rather than reading past a known end.
class Reader
{
public:
   Reader(const BYTE *p, const BYTE *end) : p_(p), end_(end) {}

   bool Ok() const { return ok_; }
   const BYTE *Pos() const { return p_; }

   bool Need(size_t n)
   {
      if (end_ && (size_t) (end_ - p_) < n)
         ok_ = false;
      return ok_;
   }
   WORD Word()
   {
      WORD v = 0;
      if (Need(2))
         memcpy(&v, p_, 2), p_ += 2;
      return v;
   }
   DWORD Dword()
   {
      DWORD v = 0;
      if (Need(4))
         memcpy(&v, p_, 4), p_ += 4;
      return v;
   }
   BYTE Byte()
   {
      BYTE v = 0;
      if (Need(1))
         v = *p_++;
      return v;
   }
   void Skip(size_t n)
   {
      if (Need(n))
         p_ += n;
   }
   // Items are DWORD-aligned in memory, as Windows reads them.
   void Align()
   {
      const BYTE *a = (const BYTE *) (((uintptr_t) p_ + 3) & ~(uintptr_t) 3);
      Skip(a - p_);
   }
   // A NUL-terminated UTF-16 string, in the ANSI code page.
   std::string String()
   {
      std::u16string s;
      for (;;)
      {
         WORD c = Word();
         if (!ok_ || c == 0)
            break;
         s += (char16_t) c;
      }
      if (s.empty())
         return std::string();
      int n = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) s.data(), (int) s.size(), nullptr, 0, nullptr, nullptr);
      std::string out(n, '\0');
      WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) s.data(), (int) s.size(), out.data(), n, nullptr, nullptr);
      return out;
   }
   // sz_Or_Ord: 0x0000 none, 0xFFFF then an ordinal, else a string.
   void StringOrOrdinal(std::string *s, WORD *ordinal, bool *is_ordinal = nullptr)
   {
      s->clear();
      *ordinal = 0;
      if (is_ordinal)
         *is_ordinal = false;
      if (!Need(2))
         return;
      WORD first;
      memcpy(&first, p_, 2);
      if (first == 0)
         p_ += 2;
      else if (first == 0xFFFF)
      {
         p_ += 2;
         *ordinal = Word();
         if (is_ordinal)
            *is_ordinal = true;
      }
      else
         *s = String();
   }

private:
   const BYTE *p_, *end_;
   bool ok_ = true;
};

const char *PredefinedClass(WORD atom)
{
   switch (atom)
   {
   case 0x0080: return "Button";
   case 0x0081: return "Edit";
   case 0x0082: return "Static";
   case 0x0083: return "ListBox";
   case 0x0084: return "ScrollBar";
   case 0x0085: return "ComboBox";
   }
   return nullptr;
}

bool ParseTemplate(const void *data, const BYTE *end, DialogTemplate *t)
{
   if (data == nullptr)
      return false;
   Reader r((const BYTE *) data, end);
   WORD version = 0, signature = 0;
   if (r.Need(4))
   {
      memcpy(&version, data, 2);
      memcpy(&signature, (const BYTE *) data + 2, 2);
   }
   t->ex = version == 1 && signature == 0xFFFF;
   if (t->ex)
   {
      r.Skip(4);
      t->help = r.Dword();
      t->exstyle = r.Dword();
      t->style = r.Dword();
   }
   else
   {
      t->style = r.Dword();
      t->exstyle = r.Dword();
   }
   WORD count = r.Word();
   t->x = (short) r.Word();
   t->y = (short) r.Word();
   t->cx = (short) r.Word();
   t->cy = (short) r.Word();
   r.StringOrOrdinal(&t->menu, &t->menu_ordinal);
   r.StringOrOrdinal(&t->cls, &t->cls_atom);
   t->title = r.String();
   if (t->style & DS_SETFONT)
   {
      t->point_size = r.Word();
      if (t->ex)
      {
         t->weight = r.Word();
         t->italic = r.Byte();
         t->charset = r.Byte();
      }
      t->face = r.String();
   }
   if (!r.Ok())
      return false;

   t->items.resize(count);
   for (ItemTemplate &it : t->items)
   {
      r.Align();
      if (t->ex)
      {
         it.help = r.Dword();
         it.exstyle = r.Dword();
         it.style = r.Dword();
      }
      else
      {
         it.style = r.Dword();
         it.exstyle = r.Dword();
      }
      it.x = (short) r.Word();
      it.y = (short) r.Word();
      it.cx = (short) r.Word();
      it.cy = (short) r.Word();
      it.id = t->ex ? r.Dword() : r.Word();
      r.StringOrOrdinal(&it.cls, &it.cls_atom);
      if (it.cls.empty())
         if (const char *name = PredefinedClass(it.cls_atom))
            it.cls = name, it.cls_atom = 0;
      r.StringOrOrdinal(&it.title, &it.title_ordinal, &it.title_is_ordinal);
      const BYTE *size_word = r.Pos();
      WORD extra = r.Word();
      if (extra)
      {
         it.data = size_word;
         r.Skip(extra);
      }
      if (!r.Ok())
         return false;
   }
   return true;
}

/************************************************************************/
// Dialog state

struct DialogInfo
{
   HWND focus = nullptr;             // the control with the focus when deactivated
   HFONT font = nullptr;             // the template's font (owned)
   HMENU menu = nullptr;
   int xbase = 0, ybase = 0;         // base units: pixels per 4 x 8 dialog units
   WORD def_id = IDOK;               // DM_SETDEFID
   INT_PTR result = 0;               // EndDialog
   bool ended = false;
   bool modal = false;
   HWND disabled_owner = nullptr;    // modal: the owner we disabled
   HWND modal_owner = nullptr;       // modal: the top-level owner (WM_ENTERIDLE)
};

// Handle values aren't reused, so the map can't confuse a new window with a
// dead one.
std::map<HWND, DialogInfo *> &Infos()
{
   static std::map<HWND, DialogInfo *> infos;
   return infos;
}

LONG BaseUnitsOfSystemFont();

DialogInfo *GetInfo(HWND hwnd, bool create)
{
   auto &infos = Infos();
   auto it = infos.find(hwnd);
   if (it != infos.end())
      return it->second;
   if (!create || !IsWindow(hwnd))
      return nullptr;
   // A window of a class whose procedure is DefDlgProc, made by CreateWindow.
   auto *info = new DialogInfo;
   LONG units = BaseUnitsOfSystemFont();
   info->xbase = LOWORD(units);
   info->ybase = HIWORD(units);
   infos[hwnd] = info;
   return info;
}

void FreeInfo(HWND hwnd)
{
   auto &infos = Infos();
   auto it = infos.find(hwnd);
   if (it == infos.end())
      return;
   DialogInfo *info = it->second;
   infos.erase(it);
   if (info->font)
      DeleteObject(info->font);
   // The menu is the window's; DestroyWindow destroys it.
   if (info->disabled_owner && !info->ended)
      EnableWindow(info->disabled_owner, TRUE);
   delete info;
}

// Base units of a font, as Windows computes them (GdiGetCharDimensions): the
// alphabet's average width, rounded, and the font's height.
bool CharDimensions(HFONT font, int *cx, int *cy)
{
   HDC dc = CreateCompatibleDC(nullptr);
   if (dc == nullptr)
      return false;
   HGDIOBJ old = SelectObject(dc, font);
   static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
   TEXTMETRICA tm;
   SIZE size;
   bool ok = GetTextMetricsA(dc, &tm) && GetTextExtentPoint32A(dc, alphabet, 52, &size);
   if (ok)
   {
      *cx = (size.cx / 26 + 1) / 2;
      *cy = tm.tmHeight;
   }
   SelectObject(dc, old);
   DeleteDC(dc);
   return ok && *cx > 0;
}

LONG BaseUnitsOfSystemFont()
{
   static LONG units = 0;
   if (units == 0)
   {
      int cx = 8, cy = 16;
      CharDimensions((HFONT) GetStockObject(SYSTEM_FONT), &cx, &cy);
      units = MAKELONG(cx, cy);
   }
   return units;
}

bool IsDialog(HWND hwnd)
{
   return Infos().count(hwnd) != 0;
}

DWORD Style(HWND hwnd)
{
   return (DWORD) GetWindowLongA(hwnd, GWL_STYLE);
}

LRESULT DlgCode(HWND hwnd, const MSG *msg = nullptr)
{
   return SendMessageA(hwnd, WM_GETDLGCODE, msg ? msg->wParam : 0, (LPARAM) msg);
}

void EnsureDialogClass()
{
   static bool registered = false;
   if (registered)
      return;
   registered = true;
   RegisterSystemClass("#32770", DefDlgProcA, CS_SAVEBITS | CS_DBLCLKS, DLGWINDOWEXTRA,
                       LoadCursorA(nullptr, (LPCSTR) IDC_ARROW), nullptr);
}

HWND FindDefButton(HWND dialog)
{
   for (HWND c = GetWindow(dialog, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
   {
      if (DlgCode(c) & DLGC_DEFPUSHBUTTON)
         return c;
      if (GetWindowLongA(c, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
         if ((Style(c) & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE)
            if (HWND found = FindDefButton(c))
               return found;
   }
   return nullptr;
}

void SetDefId(HWND dialog, DialogInfo *info, WPARAM id)
{
   HWND now = GetDlgItem(dialog, (INT) id);
   WORD old_id = info->def_id;
   info->def_id = (WORD) id;
   LRESULT code = 0;
   if (now && !((code = DlgCode(now)) & (DLGC_UNDEFPUSHBUTTON | DLGC_BUTTON)))
      return;   // not a push button
   HWND old = GetDlgItem(dialog, old_id);
   if (old == nullptr || !(DlgCode(old) & DLGC_DEFPUSHBUTTON))
      old = FindDefButton(dialog);
   if (old && old != now)
      SendMessageA(old, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
   if (now && (code & DLGC_UNDEFPUSHBUTTON))
      SendMessageA(now, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
}

// Focus moved to now: a push button there becomes the default; elsewhere the
// default ID's button is.
void SetDefButton(HWND dialog, DialogInfo *info, HWND now)
{
   HWND old = FindDefButton(dialog);
   LRESULT code = now ? DlgCode(now) : 0;
   if (now == nullptr || !(code & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON)))
   {
      now = GetDlgItem(dialog, info->def_id);
      if (now && !(DlgCode(now) & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON)))
         now = nullptr;
      if (now == nullptr)
         now = old;
      code = now ? DlgCode(now) : 0;
   }
   if (old && old != now)
      SendMessageA(old, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
   if (now && (code & DLGC_UNDEFPUSHBUTTON))
      SendMessageA(now, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
}

void SetDialogFocus(HWND control)
{
   if (DlgCode(control) & DLGC_HASSETSEL)
      SendMessageA(control, EM_SETSEL, 0, -1);
   SetFocus(control);
}

void SaveFocus(HWND dialog)
{
   HWND f = GetFocus();
   if (f == nullptr || !IsChild(dialog, f))
      return;
   if (DialogInfo *info = GetInfo(dialog, false))
      info->focus = f;
}

void RestoreFocus(HWND dialog, bool just_activated)
{
   DialogInfo *info = GetInfo(dialog, false);
   if (info == nullptr || IsIconic(dialog) || info->ended)
      return;
   if (!IsWindow(info->focus) || info->focus == dialog)
   {
      if (just_activated)
         return;
      info->focus = GetNextDlgTabItem(dialog, nullptr, FALSE);
      if (info->focus == nullptr)
         info->focus = GetNextDlgGroupItem(dialog, nullptr, FALSE);
      if (!IsWindow(info->focus))
         return;
   }
   if (just_activated)
      SetFocus(info->focus);
   else
      SetDialogFocus(info->focus);
   info->focus = nullptr;
}

// A nested DS_CONTROL dialog's keys are its parent dialog's.
HWND MessageDestination(HWND dialog)
{
   while (Style(dialog) & DS_CONTROL)
   {
      HWND parent = GetParent(dialog);
      if (parent == nullptr || !IsDialog(parent))
         break;
      dialog = parent;
   }
   return dialog;
}

void FitToWorkArea(POINT *pos, SIZE size)
{
   MONITORINFO mi = {sizeof(mi)};
   GetMonitorInfoA(MonitorFromPoint(*pos, MONITOR_DEFAULTTOPRIMARY), &mi);
   int dx = pos->x + size.cx + GetSystemMetrics(SM_CXDLGFRAME) - mi.rcWork.right;
   int dy = pos->y + size.cy + GetSystemMetrics(SM_CYDLGFRAME) - mi.rcWork.bottom;
   if (dx > 0)
      pos->x -= dx;
   if (dy > 0)
      pos->y -= dy;
   pos->x = std::max(pos->x, mi.rcWork.left);
   pos->y = std::max(pos->y, mi.rcWork.top);
}

bool CreateControls(HWND dialog, const DialogTemplate &t, HINSTANCE instance, DialogInfo *info)
{
   HWND def_button = nullptr;
   for (const ItemTemplate &it : t.items)
   {
      DWORD style = (it.style | WS_CHILD) & ~WS_POPUP;
      DWORD exstyle = it.exstyle | WS_EX_NOPARENTNOTIFY;
      if (style & WS_BORDER)
      {
         // Dialogs have the 3D look: borders are sunken edges.
         style &= ~WS_BORDER;
         exstyle |= WS_EX_CLIENTEDGE;
      }
      // An ordinal title (an icon's or bitmap's resource ID), as Windows passes
      // it to an ANSI window: 0xFF and the ID's two bytes.
      char ordinal[4] = {(char) 0xFF, (char) LOBYTE(it.title_ordinal), (char) HIBYTE(it.title_ordinal), 0};
      LPCSTR title = it.title_is_ordinal ? ordinal : it.title.c_str();
      LPCSTR cls = it.cls.empty() ? (LPCSTR) MAKEINTATOM(it.cls_atom) : it.cls.c_str();
      HWND c = CreateWindowExA(exstyle, cls, title, style, MulDiv(it.x, info->xbase, 4), MulDiv(it.y, info->ybase, 8),
                               MulDiv(it.cx, info->xbase, 4), MulDiv(it.cy, info->ybase, 8), dialog,
                               (HMENU) (UINT_PTR) it.id, instance, (LPVOID) it.data);
      if (c == nullptr)
      {
         fprintf(stderr, "win32: dialog control %u of class %s not created%s\n", (unsigned) it.id,
                 it.cls.empty() ? ("#" + std::to_string(it.cls_atom)).c_str() : it.cls.c_str(),
                 (t.style & DS_NOFAILCREATE) ? "" : "; dialog fails");
         if (t.style & DS_NOFAILCREATE)
            continue;
         return false;
      }
      if (info->font)
         SendMessageA(c, WM_SETFONT, (WPARAM) info->font, 0);
      if (DlgCode(c) & DLGC_DEFPUSHBUTTON)
      {
         // One default button: the last one in the template.
         if (def_button)
            SendMessageA(def_button, BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
         def_button = c;
         info->def_id = (WORD) GetWindowLongPtrA(c, GWLP_ID);
      }
   }
   return true;
}

// Creates a dialog from a template.  modal: owner is disabled, and *modal_owner
// is the top-level window that was.
HWND CreateFromTemplate(HINSTANCE instance, const void *data, const BYTE *end, HWND owner, DLGPROC proc,
                        LPARAM param, bool modal)
{
   if (!loop::IsGuiThread())
      return (HWND) OnGuiThread(
         [&] { return (LRESULT) CreateFromTemplate(instance, data, end, owner, proc, param, modal); });
   DialogTemplate t;
   if (!ParseTemplate(data, end, &t))
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   EnsureDialogClass();

   HMENU menu = nullptr;
   if (!t.menu.empty() || t.menu_ordinal)
      menu = LoadMenuA(instance, t.menu.empty() ? MAKEINTRESOURCEA(t.menu_ordinal) : t.menu.c_str());

   // The font and the base units.
   LONG units = BaseUnitsOfSystemFont();
   int xbase = LOWORD(units), ybase = HIWORD(units);
   HFONT font = nullptr;
   if (t.style & DS_SETFONT)
   {
      LOGFONTA lf = {};
      if (t.point_size == 0x7FFF)
      {
         NONCLIENTMETRICSA ncm = {sizeof(ncm)};
         SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
         lf = ncm.lfMessageFont;
      }
      else
      {
         HDC dc = CreateCompatibleDC(nullptr);
         lf.lfHeight = -MulDiv(t.point_size, GetDeviceCaps(dc, LOGPIXELSY), 72);
         DeleteDC(dc);
         lf.lfWeight = t.weight;
         lf.lfItalic = t.italic;
         lf.lfCharSet = t.charset;
         lf.lfQuality = PROOF_QUALITY;
         std::string face = t.face;
         // DS_SHELLFONT asks for the shell's dialog font, "MS Shell Dlg 2".
         if (t.ex && (t.style & DS_SHELLFONT) == DS_SHELLFONT && strcasecmp(face.c_str(), "MS Shell Dlg") == 0)
            face = "MS Shell Dlg 2";
         lstrcpynA(lf.lfFaceName, face.c_str(), LF_FACESIZE);
      }
      font = CreateFontIndirectA(&lf);
      int cx, cy;
      if (font && CharDimensions(font, &cx, &cy))
         xbase = cx, ybase = cy;
   }

   DWORD style = t.style, exstyle = t.exstyle;
   if (style & DS_CONTROL)
      style &= ~(WS_CAPTION | WS_SYSMENU);
   style |= DS_3DLOOK;
   if (style & DS_MODALFRAME)
      exstyle |= WS_EX_DLGMODALFRAME;
   if ((style & DS_CONTROL) || !(style & WS_CHILD))
      exstyle |= WS_EX_CONTROLPARENT;
   if (style & DS_SYSMODAL)
      exstyle |= WS_EX_TOPMOST;

   RECT rect = {0, 0, MulDiv(t.cx, xbase, 4), MulDiv(t.cy, ybase, 8)};
   AdjustWindowRectEx(&rect, style, menu != nullptr, exstyle);
   POINT pos = {rect.left, rect.top};
   SIZE size = {rect.right - rect.left, rect.bottom - rect.top};

   if (t.x == (short) 0x8000)   // CW_USEDEFAULT16
      pos.x = pos.y = CW_USEDEFAULT;
   else
   {
      bool fit = !(style & WS_CHILD);
      if (style & DS_CENTER)
      {
         MONITORINFO mi = {sizeof(mi)};
         GetMonitorInfoA(MonitorFromWindow(owner ? owner : GetActiveWindow(), MONITOR_DEFAULTTOPRIMARY), &mi);
         pos.x = (mi.rcWork.left + mi.rcWork.right - size.cx) / 2;
         pos.y = (mi.rcWork.top + mi.rcWork.bottom - size.cy) / 2;
      }
      else if (style & DS_CENTERMOUSE)
      {
         GetCursorPos(&pos);
         pos.x -= size.cx / 2;
         pos.y -= size.cy / 2;
      }
      else
      {
         pos.x += MulDiv(t.x, xbase, 4);
         pos.y += MulDiv(t.y, ybase, 8);
         if (!(style & (WS_CHILD | DS_ABSALIGN)) && owner)
            ClientToScreen(owner, &pos);
      }
      if (fit)
         FitToWorkArea(&pos, size);
   }

   // A modal dialog's owner is a top-level window; it is disabled before the
   // dialog exists.
   HWND disabled_owner = nullptr;
   if (modal && owner)
   {
      while ((Style(owner) & (WS_POPUP | WS_CHILD)) == WS_CHILD)
      {
         HWND parent = GetParent(owner);
         if (parent == nullptr || parent == GetDesktopWindow())
            break;
         owner = parent;
      }
      if (IsWindowEnabled(owner))
      {
         disabled_owner = owner;
         EnableWindow(owner, FALSE);
      }
   }

   LPCSTR cls = !t.cls.empty() ? t.cls.c_str() : t.cls_atom ? (LPCSTR) MAKEINTATOM(t.cls_atom) : (LPCSTR) WC_DIALOG;
   HWND hwnd = CreateWindowExA(exstyle, cls, t.title.c_str(), style & ~WS_VISIBLE, pos.x, pos.y, size.cx, size.cy,
                               owner, menu, instance, nullptr);
   if (hwnd == nullptr)
   {
      if (font)
         DeleteObject(font);
      if (menu)
         DestroyMenu(menu);
      if (disabled_owner)
         EnableWindow(disabled_owner, TRUE);
      return nullptr;
   }

   DialogInfo *info = GetInfo(hwnd, true);
   info->focus = nullptr;
   info->font = font;
   info->menu = menu;
   info->xbase = xbase;
   info->ybase = ybase;
   info->modal = modal;
   info->disabled_owner = disabled_owner;
   info->modal_owner = modal ? owner : nullptr;
   if (modal && owner)
      if (Window *w = GetWindowObject(hwnd); w && w->top)
         native::SetModal(w->top, true);

   SetWindowLongPtrA(hwnd, DWLP_DLGPROC, (LONG_PTR) proc);
   if (proc && font)
      SendMessageA(hwnd, WM_SETFONT, (WPARAM) font, 0);

   if (!CreateControls(hwnd, t, instance, info))
   {
      if (IsWindow(hwnd))
         DestroyWindow(hwnd);   // re-enables the owner (FreeInfo)
      return nullptr;
   }

   if (proc && IsWindow(hwnd))
   {
      HWND focus = GetNextDlgTabItem(hwnd, nullptr, FALSE);
      if (focus == nullptr)
         focus = GetNextDlgGroupItem(hwnd, nullptr, FALSE);
      if (SendMessageA(hwnd, WM_INITDIALOG, (WPARAM) focus, param) && IsWindow(hwnd) &&
          (!(t.style & DS_CONTROL) || (t.style & WS_VISIBLE)))
      {
         // TRUE: the default focus.  WM_INITDIALOG may have changed the tab order.
         focus = GetNextDlgTabItem(hwnd, nullptr, FALSE);
         if (focus == nullptr)
            focus = GetNextDlgGroupItem(hwnd, nullptr, FALSE);
         if (focus)
            SetDialogFocus(focus);
         else if (!(t.style & WS_CHILD))
            SetFocus(hwnd);
      }
   }
   if (!IsWindow(hwnd))
      return nullptr;

   if ((t.style & WS_VISIBLE) && !(Style(hwnd) & WS_VISIBLE))
   {
      ShowWindow(hwnd, SW_SHOWNORMAL);
      UpdateWindow(hwnd);
      if (t.style & DS_SETFOREGROUND)
         SetForegroundWindow(hwnd);
   }
   return hwnd;
}

// The modal loop: until EndDialog, showing the dialog once the queue first runs
// dry, the owner told of idle times (WM_ENTERIDLE).
INT_PTR RunModal(HWND hwnd, HWND owner)
{
   DialogInfo *info = GetInfo(hwnd, false);
   if (info == nullptr)
      return -1;
   bool shown = (Style(hwnd) & WS_VISIBLE) != 0;
   DWORD template_style = Style(hwnd);
   auto show = [&] {
      ShowWindow(hwnd, SW_SHOWNORMAL);
      if (template_style & DS_SETFOREGROUND)
         SetForegroundWindow(hwnd);
      shown = true;
   };
   while (!info->ended)
   {
      MSG msg;
      if (!PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
      {
         if (!shown)
            show();
         if (owner && !(Style(hwnd) & DS_NOIDLEMSG))
            SendMessageA(owner, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM) hwnd);
         if (!IsWindow(hwnd))
            return 0;
         if (info->ended)
            break;
         GetMessageA(&msg, nullptr, 0, 0);
      }
      if (msg.message == WM_QUIT)
      {
         PostQuitMessage((INT) msg.wParam);
         if (!IsWindow(hwnd))
            return 0;
         break;
      }
      if (!IsWindow(hwnd))
         return 0;
      if (!IsDialogMessageA(hwnd, &msg))
      {
         TranslateMessage(&msg);
         DispatchMessageA(&msg);
      }
      if (!IsWindow(hwnd))
         return 0;
      if (!shown && msg.message == WM_TIMER)
         show();
   }
   INT_PTR result = info->result;
   DestroyWindow(hwnd);
   return result;
}

INT_PTR DialogBoxCommon(HINSTANCE instance, const void *data, const BYTE *end, HWND owner, DLGPROC proc, LPARAM param)
{
   // Another thread's dialog runs its modal loop on the GUI thread (OnGuiThread).
   if (!loop::IsGuiThread())
      return OnGuiThread([&] { return (LRESULT) DialogBoxCommon(instance, data, end, owner, proc, param); });
   if (owner && !IsWindow(owner))
      return 0;
   HWND hwnd = CreateFromTemplate(instance, data, end, owner, proc, param, true);
   if (hwnd == nullptr)
      return -1;
   DialogInfo *info = GetInfo(hwnd, false);
   HWND modal_owner = info ? info->modal_owner : nullptr;
   return RunModal(hwnd, modal_owner);
}

bool FindDialogResource(HINSTANCE instance, LPCSTR name, const BYTE **data, const BYTE **end)
{
   DWORD size;
   if (!FindResourceData(instance, name, (LPCSTR) RT_DIALOG, data, &size))
   {
      SetLastError(ERROR_RESOURCE_NAME_NOT_FOUND);
      return false;
   }
   *end = *data + size;
   return true;
}

std::string NameFromWide(LPCWSTR name)
{
   if (name == nullptr)
      return std::string();
   int n = WideCharToMultiByte(CP_ACP, 0, name, -1, nullptr, 0, nullptr, nullptr);
   std::string out(n > 0 ? n - 1 : 0, '\0');
   if (n > 1)
      WideCharToMultiByte(CP_ACP, 0, name, -1, out.data(), n, nullptr, nullptr);
   return out;
}

/************************************************************************/
// Keyboard navigation

HWND NextTabItem(HWND main, HWND dialog, HWND control, bool previous)
{
   UINT search = previous ? GW_HWNDPREV : GW_HWNDNEXT;
   HWND first = nullptr;
   if (control == nullptr)
   {
      first = GetWindow(dialog, GW_CHILD);
      if (previous)
         first = GetWindow(first, GW_HWNDLAST);
   }
   else if (IsChild(main, control))
   {
      first = GetWindow(control, search);
      if (first == nullptr)
      {
         if (GetParent(control) != main)
            first = GetWindow(GetParent(control), search);
         else
            first = GetWindow(control, previous ? GW_HWNDLAST : GW_HWNDFIRST);
      }
   }

   for (; first; first = GetWindow(first, search))
   {
      DWORD style = Style(first);
      DWORD exstyle = (DWORD) GetWindowLongA(first, GWL_EXSTYLE);
      if ((exstyle & WS_EX_CONTROLPARENT) && (style & WS_VISIBLE) && !(style & WS_DISABLED))
      {
         if (HWND found = NextTabItem(main, first, nullptr, previous))
            return found;
      }
      else if ((style & WS_TABSTOP) && (style & WS_VISIBLE) && !(style & WS_DISABLED))
         return first;
   }
   HWND found = nullptr;
   if (control)
   {
      for (HWND parent = GetParent(control); parent && parent != main; parent = GetParent(parent))
         if ((found = NextTabItem(main, GetParent(parent), parent, previous)) != nullptr)
            break;
      if (found == nullptr)
         found = NextTabItem(main, main, nullptr, previous);
   }
   return found ? found : control;
}

// Mnemonics: a visible, enabled button or static whose text's &-prefixed
// character is ch.  A button is clicked; a static or group box moves the focus
// to the control after it.
bool IsAccelerator(HWND hwnd, HWND dialog, WPARAM ch)
{
   HWND control = hwnd;
   do
   {
      HWND next = nullptr;
      DWORD style = Style(control);
      if ((style & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE)
      {
         LRESULT code = DlgCode(control);
         char text[256];
         if ((code & (DLGC_BUTTON | DLGC_STATIC)) && GetWindowTextA(control, text, sizeof(text)))
         {
            const char *p = text;
            while ((p = strchr(p, '&')) != nullptr && p[1] == '&')
               p += 2;
            if (p && p[1] && toupper((unsigned char) p[1]) == toupper((int) (unsigned char) ch))
            {
               if ((code & DLGC_STATIC) || (style & BS_TYPEMASK) == BS_GROUPBOX)
               {
                  SendMessageA(dialog, WM_NEXTDLGCTL, (WPARAM) control, 1);
                  SendMessageA(dialog, WM_NEXTDLGCTL, 0, 0);
               }
               else if (code & DLGC_BUTTON)
                  SendMessageA(control, BM_CLICK, 0, 0);
               return true;
            }
         }
         next = GetWindow(control, GW_CHILD);
      }
      if (next == nullptr)
         next = GetWindow(control, GW_HWNDNEXT);
      while (next == nullptr && control)
      {
         control = GetParent(control);
         if (control == dialog)
         {
            if (hwnd == dialog)
            {
               next = hwnd;
               break;
            }
            next = GetWindow(dialog, GW_CHILD);
         }
         else
            next = GetWindow(control, GW_HWNDNEXT);
      }
      control = next;
   } while (control && control != hwnd);
   return false;
}

/************************************************************************/
// MessageBox

constexpr WORD kIconId = 0x0014, kTextId = 0xFFFF;

struct MessageBoxParams
{
   std::string text;
   UINT type;
   HWND owner;
   bool has_cancel;
};

// An in-memory DLGTEMPLATE, WORD by WORD.
class TemplateBuilder
{
public:
   void Dword(DWORD v) { Word(LOWORD(v)), Word(HIWORD(v)); }
   void Word(WORD v) { words_.push_back(v); }
   void String(const std::string &s)
   {
      int n = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int) s.size(), nullptr, 0);
      std::u16string w(n, u'\0');
      MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int) s.size(), (LPWSTR) w.data(), n);
      for (char16_t c : w)
         Word((WORD) c);
      Word(0);
   }
   void Align()
   {
      if (words_.size() & 1)
         Word(0);
   }
   // The vector's storage is new[]'d: DWORD-aligned, as templates must be.
   const std::vector<WORD> &Words() const { return words_; }

   void Header(DWORD style, DWORD exstyle, WORD count, const std::string &title, WORD points, const std::string &face)
   {
      Dword(style);
      Dword(exstyle);
      Word(count);
      Word(0), Word(0), Word(0), Word(0);
      Word(0);   // menu
      Word(0);   // class: the dialog class
      String(title);
      Word(points);
      String(face);
   }
   void Item(DWORD style, WORD id, WORD atom, const std::string &title)
   {
      Align();
      Dword(style);
      Dword(0);
      Word(0), Word(0), Word(0), Word(0);
      Word(id);
      Word(0xFFFF), Word(atom);
      String(title);
      Word(0);   // no creation data
   }

private:
   std::vector<WORD> words_;
};

INT_PTR CALLBACK MessageBoxProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_INITDIALOG:
   {
      auto *mb = (MessageBoxParams *) lparam;
      SetWindowLongPtrA(hwnd, DWLP_USER, (LONG_PTR) mb);
      UINT icon_type = mb->type & MB_ICONMASK;
      HWND icon = GetDlgItem(hwnd, kIconId);
      if (icon)
      {
         LPCSTR id = icon_type == MB_ICONHAND ? IDI_HAND : icon_type == MB_ICONQUESTION ? IDI_QUESTION
                     : icon_type == MB_ICONEXCLAMATION ? IDI_EXCLAMATION : IDI_ASTERISK;
         SendMessageA(icon, STM_SETICON, (WPARAM) LoadIconA(nullptr, id), 0);
      }

      // Layout, in pixels (as Windows does it, after the template is made).
      HFONT font = (HFONT) SendMessageA(hwnd, WM_GETFONT, 0, 0);
      DialogInfo *info = GetInfo(hwnd, false);
      int xbase = info ? info->xbase : 6;
      RECT wr, cr;
      GetWindowRect(hwnd, &wr);
      GetClientRect(hwnd, &cr);
      int border_w = (wr.right - wr.left) - cr.right, border_h = (wr.bottom - wr.top) - cr.bottom;
      int ileft = MulDiv(8, xbase, 4);
      int iwidth = icon ? GetSystemMetrics(SM_CXICON) : 0, iheight = icon ? GetSystemMetrics(SM_CYICON) : 0;

      HDC dc = CreateCompatibleDC(nullptr);
      HGDIOBJ old_font = font ? SelectObject(dc, font) : nullptr;
      std::vector<HWND> buttons;
      int bw = 1, bh = 1;
      for (HWND c = GetWindow(hwnd, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
      {
         int id = GetDlgCtrlID(c);
         if (id == kIconId || id == kTextId)
            continue;
         buttons.push_back(c);
         char text[64];
         if (GetWindowTextA(c, text, sizeof(text)))
         {
            RECT r = {0, 0, 0, 0};
            DrawTextA(dc, text, -1, &r, DT_LEFT | DT_EXPANDTABS | DT_CALCRECT);
            bw = std::max(bw, (int) (r.right - r.left));
            bh = std::max(bh, (int) (r.bottom - r.top));
         }
      }
      bw = std::max(bw, bh * 2);
      bh *= 2;
      bw *= 2;
      int bspace = bw / 3;
      int n = (int) buttons.size();

      RECT tr = {0, 0, MulDiv(176, xbase, 4), 0};
      DrawTextA(dc, mb->text.c_str(), -1, &tr, DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK | DT_CALCRECT);
      if (old_font)
         SelectObject(dc, old_font);
      DeleteDC(dc);
      int tleft = ileft + (iwidth ? ileft + iwidth : 0);
      int twidth = std::max((bw + bspace) * n + bspace - tleft, (int) tr.right);
      int theight = tr.bottom;

      int tiheight = 16 + std::max(iheight, theight);
      int wwidth = tleft + twidth + ileft + border_w;
      int wheight = 8 + tiheight + bh + border_h;
      MONITORINFO mi = {sizeof(mi)};
      GetMonitorInfoA(MonitorFromWindow(mb->owner ? mb->owner : GetActiveWindow(), MONITOR_DEFAULTTOPRIMARY), &mi);
      SetWindowPos(hwnd, nullptr, (mi.rcWork.left + mi.rcWork.right - wwidth) / 2,
                   (mi.rcWork.top + mi.rcWork.bottom - wheight) / 2, wwidth, wheight,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
      if (icon)
         SetWindowPos(icon, nullptr, ileft, (tiheight - iheight) / 2, iwidth, iheight,
                      SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
      SetWindowPos(GetDlgItem(hwnd, kTextId), nullptr, tleft, (tiheight - theight) / 2, twidth, theight,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
      int bx = (wwidth - border_w - (bw + bspace) * n + bspace) / 2;
      int def = (int) ((mb->type & MB_DEFMASK) >> 8);
      if (def >= n)
         def = 0;
      for (int i = 0; i < n; i++)
      {
         SetWindowPos(buttons[i], nullptr, bx, tiheight, bw, bh, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
         bx += bw + bspace;
      }
      if (n)
      {
         SendMessageA(hwnd, DM_SETDEFID, GetDlgCtrlID(buttons[def]), 0);
         SetFocus(buttons[def]);
      }
      if (icon_type)
         MessageBeep(icon_type);
      return FALSE;   // the focus is set
   }

   case WM_COMMAND:
   {
      auto *mb = (MessageBoxParams *) GetWindowLongPtrA(hwnd, DWLP_USER);
      int id = LOWORD(wparam);
      if (id == IDCANCEL && mb && !mb->has_cancel)
      {
         // Esc and the close box answer OK in a box with only OK; nothing otherwise.
         if ((mb->type & MB_TYPEMASK) == MB_OK)
            EndDialog(hwnd, IDOK);
         return TRUE;
      }
      if ((id >= IDOK && id <= IDNO) || id == IDTRYAGAIN || id == IDCONTINUE)
         EndDialog(hwnd, id);
      return TRUE;
   }
   }
   return FALSE;
}

}  // namespace

}  // namespace w32

using namespace w32;

/************************************************************************/
// Creating dialogs

HWND WINAPI CreateDialogIndirectParamA(HINSTANCE instance, LPCDLGTEMPLATEA tmpl, HWND owner, DLGPROC proc, LPARAM param)
{
   return CreateFromTemplate(instance, tmpl, nullptr, owner, proc, param, false);
}

HWND WINAPI CreateDialogIndirectParamW(HINSTANCE instance, LPCDLGTEMPLATEW tmpl, HWND owner, DLGPROC proc, LPARAM param)
{
   return CreateFromTemplate(instance, tmpl, nullptr, owner, proc, param, false);
}

HWND WINAPI CreateDialogParamA(HINSTANCE instance, LPCSTR name, HWND owner, DLGPROC proc, LPARAM param)
{
   const BYTE *data, *end;
   if (!FindDialogResource(instance, name, &data, &end))
      return nullptr;
   return CreateFromTemplate(instance, data, end, owner, proc, param, false);
}

HWND WINAPI CreateDialogParamW(HINSTANCE instance, LPCWSTR name, HWND owner, DLGPROC proc, LPARAM param)
{
   if (IS_INTRESOURCE(name))
      return CreateDialogParamA(instance, (LPCSTR) name, owner, proc, param);
   return CreateDialogParamA(instance, NameFromWide(name).c_str(), owner, proc, param);
}

INT_PTR WINAPI DialogBoxIndirectParamA(HINSTANCE instance, LPCDLGTEMPLATEA tmpl, HWND owner, DLGPROC proc, LPARAM param)
{
   return DialogBoxCommon(instance, tmpl, nullptr, owner, proc, param);
}

INT_PTR WINAPI DialogBoxIndirectParamW(HINSTANCE instance, LPCDLGTEMPLATEW tmpl, HWND owner, DLGPROC proc, LPARAM param)
{
   return DialogBoxCommon(instance, tmpl, nullptr, owner, proc, param);
}

INT_PTR WINAPI DialogBoxParamA(HINSTANCE instance, LPCSTR name, HWND owner, DLGPROC proc, LPARAM param)
{
   if (owner && !IsWindow(owner))
      return 0;
   const BYTE *data, *end;
   if (!FindDialogResource(instance, name, &data, &end))
      return -1;
   return DialogBoxCommon(instance, data, end, owner, proc, param);
}

INT_PTR WINAPI DialogBoxParamW(HINSTANCE instance, LPCWSTR name, HWND owner, DLGPROC proc, LPARAM param)
{
   if (IS_INTRESOURCE(name))
      return DialogBoxParamA(instance, (LPCSTR) name, owner, proc, param);
   return DialogBoxParamA(instance, NameFromWide(name).c_str(), owner, proc, param);
}

BOOL WINAPI EndDialog(HWND hwnd, INT_PTR result)
{
   DialogInfo *info = GetInfo(hwnd, false);
   if (info == nullptr)
      return FALSE;
   bool was_active = hwnd == GetActiveWindow();
   info->result = result;
   info->ended = true;

   HWND owner = GetWindow(hwnd, GW_OWNER);
   if (info->disabled_owner)
      EnableWindow(info->disabled_owner, TRUE);
   if (info->modal)
      if (Window *w = GetWindowObject(hwnd); w && w->top)
         native::SetModal(w->top, false);
   if (was_active && IsChild(hwnd, GetFocus()))
      SetFocus(hwnd);
   SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
   if (was_active && owner)
      SetActiveWindow(owner);
   // Wakes the modal loop.
   PostMessageA(hwnd, WM_NULL, 0, 0);
   return TRUE;
}

/************************************************************************/
// DefDlgProc

static LRESULT DefDlgDefault(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, DialogInfo *info)
{
   switch (msg)
   {
   case WM_ERASEBKGND:
   {
      HBRUSH brush = (HBRUSH) SendMessageA(hwnd, WM_CTLCOLORDLG, wparam, (LPARAM) hwnd);
      if (brush == nullptr)
         brush = (HBRUSH) DefWindowProcA(hwnd, WM_CTLCOLORDLG, wparam, (LPARAM) hwnd);
      if (brush)
      {
         RECT r;
         GetClientRect(hwnd, &r);
         FillRect((HDC) wparam, &r, brush);
      }
      return 1;
   }
   case WM_SHOWWINDOW:
      if (!wparam)
         SaveFocus(hwnd);
      return DefWindowProcA(hwnd, msg, wparam, lparam);
   case WM_ACTIVATE:
      if (LOWORD(wparam) != WA_INACTIVE)
         RestoreFocus(hwnd, true);
      else
         SaveFocus(hwnd);
      return 0;
   case WM_SETFOCUS:
      RestoreFocus(hwnd, false);
      return 0;
   case DM_SETDEFID:
      if (!info->ended)
         SetDefId(hwnd, info, wparam);
      return 1;
   case DM_GETDEFID:
      if (!info->ended)
      {
         if (info->def_id)
            return MAKELONG(info->def_id, DC_HASDEFID);
         if (HWND def = FindDefButton(hwnd))
            return MAKELONG(GetDlgCtrlID(def), DC_HASDEFID);
      }
      return 0;
   case DM_REPOSITION:
      if (!(Style(hwnd) & WS_CHILD))
      {
         RECT r;
         GetWindowRect(hwnd, &r);
         POINT pos = {r.left, r.top};
         FitToWorkArea(&pos, {r.right - r.left, r.bottom - r.top});
         SetWindowPos(hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
      return 0;
   case WM_NEXTDLGCTL:
   {
      HWND dest = (HWND) wparam;
      if (!lparam)
         dest = GetNextDlgTabItem(hwnd, GetFocus(), (BOOL) wparam);
      if (dest)
         SetDialogFocus(dest);
      SetDefButton(hwnd, info, dest);
      return 0;
   }
   case WM_ENTERMENULOOP:
   case WM_LBUTTONDOWN:
   case WM_NCLBUTTONDOWN:
      // A combo box's drop-down closes.
      if (HWND f = GetFocus())
         if (!SendMessageA(f, CB_SHOWDROPDOWN, FALSE, 0))
            SendMessageA(GetParent(f), CB_SHOWDROPDOWN, FALSE, 0);
      return DefWindowProcA(hwnd, msg, wparam, lparam);
   case WM_GETFONT:
      return (LRESULT) info->font;
   case WM_CLOSE:
      PostMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), (LPARAM) GetDlgItem(hwnd, IDCANCEL));
      return 0;
   }
   return 0;
}

LRESULT WINAPI DefDlgProcA(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   DialogInfo *info = GetInfo(hwnd, true);
   if (info == nullptr)
      return 0;
   SetWindowLongPtrA(hwnd, DWLP_MSGRESULT, 0);
   INT_PTR result = 0;
   if (DLGPROC proc = (DLGPROC) GetWindowLongPtrA(hwnd, DWLP_DLGPROC))
      result = proc(hwnd, msg, wparam, lparam);

   if (msg == WM_NCDESTROY)
   {
      FreeInfo(hwnd);
      return result ? (LRESULT) result : DefWindowProcA(hwnd, msg, wparam, lparam);
   }

   if (!result && IsWindow(hwnd) && (info = GetInfo(hwnd, false)) != nullptr)
   {
      switch (msg)
      {
      case WM_ERASEBKGND:
      case WM_SHOWWINDOW:
      case WM_ACTIVATE:
      case WM_SETFOCUS:
      case DM_SETDEFID:
      case DM_GETDEFID:
      case DM_REPOSITION:
      case WM_NEXTDLGCTL:
      case WM_GETFONT:
      case WM_CLOSE:
      case WM_ENTERMENULOOP:
      case WM_LBUTTONDOWN:
      case WM_NCLBUTTONDOWN:
         return DefDlgDefault(hwnd, msg, wparam, lparam, info);
      case WM_INITDIALOG:
      case WM_VKEYTOITEM:
      case WM_COMPAREITEM:
      case WM_CHARTOITEM:
         break;
      default:
         return DefWindowProcA(hwnd, msg, wparam, lparam);
      }
   }
   // These return the procedure's value; the rest DWLP_MSGRESULT.
   if ((msg >= WM_CTLCOLORMSGBOX && msg <= WM_CTLCOLORSTATIC) || msg == WM_COMPAREITEM || msg == WM_VKEYTOITEM ||
       msg == WM_CHARTOITEM || msg == WM_QUERYDRAGICON || msg == WM_INITDIALOG)
      return (LRESULT) result;
   return IsWindow(hwnd) ? GetWindowLongPtrA(hwnd, DWLP_MSGRESULT) : 0;
}

/************************************************************************/
// Keyboard navigation

HWND WINAPI GetNextDlgTabItem(HWND dialog, HWND control, BOOL previous)
{
   if (dialog == control)
      control = nullptr;
   if (dialog == nullptr)
      return nullptr;
   return NextTabItem(dialog, dialog, control, previous != FALSE);
}

HWND WINAPI GetNextDlgGroupItem(HWND dialog, HWND control, BOOL previous)
{
   if (dialog == control)
      control = nullptr;
   if (control == nullptr && previous)
      return nullptr;
   if (control)
   {
      if (!IsChild(dialog, control))
         return nullptr;
   }
   else
   {
      // No control: the first one, if it is visible and enabled.
      if ((control = GetWindow(dialog, GW_CHILD)) == nullptr)
         return nullptr;
      if ((Style(control) & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE)
         return control;
   }

   // Around the list (once), noting the group's start; the previous item is the
   // last eligible one before control, the next the first after it.
   HWND result = control, hwnd = control, last_group = nullptr;
   bool looped = false, skipping = false;
   for (;;)
   {
      HWND next = GetWindow(hwnd, GW_HWNDNEXT);
      while (next == nullptr)
      {
         if (dialog == GetParent(hwnd))
         {
            if (looped)
               return result;
            looped = true;
            next = GetWindow(dialog, GW_CHILD);
         }
         else
         {
            hwnd = GetParent(hwnd);
            next = GetWindow(hwnd, GW_HWNDNEXT);
         }
      }
      hwnd = next;
      while ((GetWindowLongA(hwnd, GWL_EXSTYLE) & WS_EX_CONTROLPARENT) &&
             (Style(hwnd) & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE && (next = GetWindow(hwnd, GW_CHILD)) != nullptr)
         hwnd = next;

      if (Style(hwnd) & WS_GROUP)
      {
         last_group = hwnd;
         skipping = true;
      }
      if (hwnd == control)
      {
         if (!skipping || last_group == hwnd)
            break;
         hwnd = last_group;
         skipping = false;
         looped = false;
      }
      if (!skipping && (Style(hwnd) & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE)
      {
         result = hwnd;
         if (!previous)
            break;
      }
   }
   return result;
}

BOOL WINAPI IsDialogMessageA(HWND dialog, MSG *msg)
{
   if (msg == nullptr || !IsWindow(dialog) || dialog == GetDesktopWindow())
      return FALSE;
   if (dialog != msg->hwnd && !IsChild(dialog, msg->hwnd))
      return FALSE;
   dialog = MessageDestination(dialog);

   switch (msg->message)
   {
   case WM_KEYDOWN:
   {
      LRESULT code = DlgCode(msg->hwnd, msg);
      if (code & DLGC_WANTMESSAGE)
         break;
      switch (msg->wParam)
      {
      case VK_TAB:
         if (!(code & DLGC_WANTTAB))
         {
            bool previous = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (IsDialog(dialog))
               SendMessageA(dialog, WM_NEXTDLGCTL, previous, 0);
            else
            {
               HWND f = GetFocus();
               HWND next = GetNextDlgTabItem(dialog, f == dialog ? nullptr : f, previous);
               if (next == nullptr)
                  return FALSE;
               SetDialogFocus(next);
            }
            return TRUE;
         }
         break;

      case VK_RIGHT:
      case VK_DOWN:
      case VK_LEFT:
      case VK_UP:
         if (!(code & DLGC_WANTARROWS))
         {
            bool previous = msg->wParam == VK_LEFT || msg->wParam == VK_UP;
            HWND next = GetNextDlgGroupItem(dialog, msg->hwnd, previous);
            if (next && DlgCode(next, msg) == (DLGC_BUTTON | DLGC_RADIOBUTTON))
            {
               SetFocus(next);
               if ((Style(next) & BS_TYPEMASK) == BS_AUTORADIOBUTTON &&
                   SendMessageA(next, BM_GETCHECK, 0, 0) != BST_CHECKED)
                  SendMessageA(next, BM_CLICK, 1, 0);
            }
            else
               SendMessageA(dialog, WM_NEXTDLGCTL, (WPARAM) next, 1);
            return TRUE;
         }
         break;

      case VK_CANCEL:
      case VK_ESCAPE:
         SendMessageA(dialog, WM_COMMAND, IDCANCEL, (LPARAM) GetDlgItem(dialog, IDCANCEL));
         return TRUE;

      case VK_EXECUTE:
      case VK_RETURN:
      {
         HWND f = GetFocus();
         DWORD def;
         if (IsChild(dialog, f) && (DlgCode(f) & DLGC_DEFPUSHBUTTON))
            SendMessageA(dialog, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(f), BN_CLICKED), (LPARAM) f);
         else if (HIWORD(def = (DWORD) SendMessageA(dialog, DM_GETDEFID, 0, 0)) == DC_HASDEFID)
         {
            HWND button = GetDlgItem(dialog, LOWORD(def));
            if (button == nullptr || IsWindowEnabled(button))
               SendMessageA(dialog, WM_COMMAND, MAKEWPARAM(LOWORD(def), BN_CLICKED), (LPARAM) button);
         }
         else
            SendMessageA(dialog, WM_COMMAND, IDOK, (LPARAM) GetDlgItem(dialog, IDOK));
         return TRUE;
      }
      }
      break;
   }

   case WM_CHAR:
   {
      LRESULT code = DlgCode(msg->hwnd, msg);
      if (code & (DLGC_WANTCHARS | DLGC_WANTMESSAGE))
         break;
      if (msg->wParam == '\t' && (code & DLGC_WANTTAB))
         break;
      if (IsAccelerator(msg->hwnd, dialog, msg->wParam))
         return TRUE;
      break;
   }

   case WM_SYSCHAR:
      if (IsAccelerator(msg->hwnd, dialog, msg->wParam))
         return TRUE;
      break;
   }

   TranslateMessage(msg);
   DispatchMessageA(msg);
   return TRUE;
}

/************************************************************************/
// Units

BOOL WINAPI MapDialogRect(HWND hwnd, RECT *rect)
{
   DialogInfo *info = GetInfo(hwnd, false);
   if (info == nullptr || rect == nullptr)
      return FALSE;
   rect->left = MulDiv(rect->left, info->xbase, 4);
   rect->right = MulDiv(rect->right, info->xbase, 4);
   rect->top = MulDiv(rect->top, info->ybase, 8);
   rect->bottom = MulDiv(rect->bottom, info->ybase, 8);
   return TRUE;
}

DWORD WINAPI GetDialogBaseUnits(void)
{
   return (DWORD) BaseUnitsOfSystemFont();
}

/************************************************************************/
// Items

UINT WINAPI GetDlgItemTextA(HWND dialog, INT id, LPSTR text, INT size)
{
   if (text && size > 0)
      text[0] = 0;
   return (UINT) SendDlgItemMessageA(dialog, id, WM_GETTEXT, size, (LPARAM) text);
}

UINT WINAPI GetDlgItemTextW(HWND dialog, INT id, LPWSTR text, INT size)
{
   if (text == nullptr || size <= 0)
      return 0;
   text[0] = 0;
   HWND item = GetDlgItem(dialog, id);
   if (item == nullptr)
      return 0;
   int length = (int) SendMessageA(item, WM_GETTEXTLENGTH, 0, 0);
   std::string a(std::max(length, 0) + 1, '\0');
   int got = (int) SendMessageA(item, WM_GETTEXT, a.size(), (LPARAM) a.data());
   int n = MultiByteToWideChar(CP_ACP, 0, a.c_str(), std::max(got, 0), text, size - 1);
   text[n] = 0;
   return (UINT) n;
}

// The wchar_t (UTF-32 here) form the client's sources call (include/native/winuser.h).
W32_EXPORT UINT WINAPI GetDlgItemTextW(HWND dialog, INT id, wchar_t *text, INT size)
{
   if (text == nullptr || size <= 0)
      return 0;
   std::vector<WCHAR> w(size);
   UINT n = GetDlgItemTextW(dialog, id, w.data(), size);
   for (UINT i = 0; i < n; i++)
      text[i] = (wchar_t) w[i];
   text[n] = 0;
   return n;
}

BOOL WINAPI SetDlgItemTextA(HWND dialog, INT id, LPCSTR text)
{
   return (BOOL) SendDlgItemMessageA(dialog, id, WM_SETTEXT, 0, (LPARAM) text);
}

UINT WINAPI GetDlgItemInt(HWND dialog, INT id, BOOL *translated, BOOL is_signed)
{
   if (translated)
      *translated = FALSE;
   char text[64];
   if (!SendDlgItemMessageA(dialog, id, WM_GETTEXT, sizeof(text), (LPARAM) text))
      return 0;
   // Optional blanks, a sign (signed only), digits, optional blanks.
   const char *p = text;
   while (*p == ' ' || *p == '\t')
      p++;
   bool negative = false;
   if (is_signed && (*p == '-' || *p == '+'))
      negative = *p++ == '-';
   if (*p < '0' || *p > '9')
      return 0;
   unsigned long long v = 0;
   for (; *p >= '0' && *p <= '9'; p++)
   {
      v = v * 10 + (unsigned) (*p - '0');
      if (v > 0xFFFFFFFFull)
         return 0;
   }
   while (*p == ' ' || *p == '\t')
      p++;
   if (*p)
      return 0;
   long long value = negative ? -(long long) v : (long long) v;
   if (is_signed ? (value < INT_MIN || value > INT_MAX) : v > UINT_MAX)
      return 0;
   if (translated)
      *translated = TRUE;
   return (UINT) value;
}

BOOL WINAPI SetDlgItemInt(HWND dialog, INT id, UINT value, BOOL is_signed)
{
   char text[16];
   if (is_signed)
      snprintf(text, sizeof(text), "%d", (INT) value);
   else
      snprintf(text, sizeof(text), "%u", value);
   return SetDlgItemTextA(dialog, id, text);
}

BOOL WINAPI CheckDlgButton(HWND dialog, INT id, UINT check)
{
   SendDlgItemMessageA(dialog, id, BM_SETCHECK, check, 0);
   return TRUE;
}

UINT WINAPI IsDlgButtonChecked(HWND dialog, int id)
{
   return (UINT) SendDlgItemMessageA(dialog, id, BM_GETCHECK, 0, 0);
}

namespace {

struct RadioGroup
{
   int first, last, check;
};

BOOL CALLBACK CheckRadio(HWND hwnd, LPARAM lparam)
{
   auto *g = (RadioGroup *) lparam;
   int id = (int) GetWindowLongPtrA(hwnd, GWLP_ID);
   if (id >= g->first && id <= g->last)
      SendMessageA(hwnd, BM_SETCHECK, id == g->check ? BST_CHECKED : BST_UNCHECKED, 0);
   return TRUE;
}

}  // namespace

BOOL WINAPI CheckRadioButton(HWND dialog, int first, int last, int check)
{
   RadioGroup group = {first, last, check};
   EnumChildWindows(dialog, CheckRadio, (LPARAM) &group);
   return TRUE;
}

/************************************************************************/
// MessageBox

INT WINAPI MessageBoxA(HWND owner, LPCSTR text, LPCSTR caption, UINT type)
{
   if (!loop::IsGuiThread())
      return (INT) OnGuiThread([&] { return (LRESULT) MessageBoxA(owner, text, caption, type); });
   if (owner && !IsWindow(owner))
      owner = nullptr;
   MessageBoxParams mb;
   mb.text = text ? text : "";
   mb.type = type;
   mb.owner = owner;

   // The buttons, in Windows' order.
   static const struct { WORD id; const char *text; } kButtons[] = {
      {IDYES, "&Yes"}, {IDNO, "&No"}, {IDOK, "OK"}, {IDABORT, "&Abort"}, {IDRETRY, "&Retry"},
      {IDCANCEL, "Cancel"}, {IDIGNORE, "&Ignore"}, {IDTRYAGAIN, "&Try Again"}, {IDCONTINUE, "&Continue"},
   };
   std::vector<WORD> ids;
   switch (type & MB_TYPEMASK)
   {
   case MB_OKCANCEL: ids = {IDOK, IDCANCEL}; break;
   case MB_ABORTRETRYIGNORE: ids = {IDABORT, IDRETRY, IDIGNORE}; break;
   case MB_YESNOCANCEL: ids = {IDYES, IDNO, IDCANCEL}; break;
   case MB_YESNO: ids = {IDYES, IDNO}; break;
   case MB_RETRYCANCEL: ids = {IDRETRY, IDCANCEL}; break;
   case MB_CANCELTRYCONTINUE: ids = {IDCANCEL, IDTRYAGAIN, IDCONTINUE}; break;
   default: ids = {IDOK}; break;
   }
   mb.has_cancel = std::find(ids.begin(), ids.end(), (WORD) IDCANCEL) != ids.end();
   UINT def = (type & MB_DEFMASK) >> 8;

   NONCLIENTMETRICSA ncm = {sizeof(ncm)};
   SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
   WORD points = (WORD) MulDiv(abs(ncm.lfMessageFont.lfHeight), 72, 96);

   DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_SETFONT | DS_CENTER;
   if (type & MB_SYSTEMMODAL)
      style |= DS_SYSMODAL;
   if (type & MB_SETFOREGROUND)
      style |= DS_SETFOREGROUND;
   DWORD exstyle = (type & MB_TOPMOST) ? WS_EX_TOPMOST : 0;
   bool icon = (type & MB_ICONMASK) != 0;

   TemplateBuilder b;
   b.Header(style, exstyle, (WORD) (ids.size() + 1 + (icon ? 1 : 0)), caption ? caption : "Error", points,
            ncm.lfMessageFont.lfFaceName);
   if (icon)
      b.Item(WS_CHILD | WS_VISIBLE | SS_ICON, kIconId, 0x0082, "");
   b.Item(WS_CHILD | WS_VISIBLE | WS_GROUP | SS_LEFT | SS_NOPREFIX, kTextId, 0x0082, mb.text);
   for (size_t i = 0; i < ids.size(); i++)
      for (const auto &button : kButtons)
         if (button.id == ids[i])
            b.Item(WS_CHILD | WS_VISIBLE | WS_TABSTOP | (i == 0 ? WS_GROUP : 0) |
                       (i == (def < ids.size() ? def : 0) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
                   button.id, 0x0080, button.text);

   // Task-modal without an owner: all the thread's top-level windows are disabled.
   std::vector<HWND> disabled;
   if (owner == nullptr && (type & MB_MODEMASK) == MB_TASKMODAL)
      ForEachTopLevel([](HWND h, void *data) {
         if (IsWindowEnabled(h) && IsWindowVisible(h))
         {
            EnableWindow(h, FALSE);
            static_cast<std::vector<HWND> *>(data)->push_back(h);
         }
      }, &disabled);

   std::vector<WORD> words = b.Words();
   INT_PTR r = DialogBoxIndirectParamA(nullptr, (LPCDLGTEMPLATEA) words.data(), owner, MessageBoxProc, (LPARAM) &mb);
   for (HWND h : disabled)
      EnableWindow(h, TRUE);
   return r < 0 ? 0 : (INT) r;
}

INT WINAPI MessageBoxW(HWND owner, LPCWSTR text, LPCWSTR caption, UINT type)
{
   std::string t = NameFromWide(text), c = NameFromWide(caption);
   return MessageBoxA(owner, text ? t.c_str() : nullptr, caption ? c.c_str() : nullptr, type);
}

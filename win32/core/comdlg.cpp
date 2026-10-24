/*
 * comdlg.cpp:  Common dialogs: ChooseColor and ChooseFont as the toolkit's
 *   choosers, run modally as Windows runs its dialogs (the owner disabled,
 *   messages dispatched until they close); printing reports no printer.  And
 *   DwmSetWindowAttribute's dark mode.
 */

#include "core/eventloop.h"
#include "core/native.h"
#include "core/window.h"

#include <commdlg.h>
#include <dwmapi.h>

#include <cstring>

using namespace w32;

namespace {

DWORD last_error = 0;

struct Pending
{
   bool done = false;
   bool ok = false;
   COLORREF color = 0;
   LOGFONTA font = {};
};

native::Top *OwnerTop(HWND owner)
{
   Window *w = GetWindowObject(TopLevelOf(owner));
   return w ? w->top : nullptr;
}

// Dispatches messages until p is done, with owner disabled meanwhile.
void RunModal(HWND owner, Pending *p)
{
   bool was_enabled = owner && IsWindowEnabled(owner);
   if (was_enabled)
      EnableWindow(owner, FALSE);
   MSG msg;
   while (!p->done)
   {
      if (!GetMessageA(&msg, nullptr, 0, 0))
      {
         // WM_QUIT: leave it for the caller's loop, as Windows' dialogs do.
         PostQuitMessage((int) msg.wParam);
         break;
      }
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
   }
   if (was_enabled)
      EnableWindow(owner, TRUE);
}

}  // namespace

BOOL WINAPI ChooseColorA(CHOOSECOLORA *cc)
{
   if (cc == nullptr || cc->lStructSize < sizeof(CHOOSECOLORA))
   {
      last_error = CDERR_STRUCTSIZE;
      return FALSE;
   }
   last_error = 0;
   Pending p;
   native::ChooseColor(OwnerTop(cc->hwndOwner), (cc->Flags & CC_RGBINIT) ? cc->rgbResult : RGB(0, 0, 0),
                       [](bool ok, COLORREF color, void *data) {
                          auto *p = (Pending *) data;
                          p->ok = ok;
                          p->color = color;
                          p->done = true;
                          loop::Wake();
                       },
                       &p);
   RunModal(cc->hwndOwner, &p);
   if (!p.ok)
      return FALSE;
   cc->rgbResult = p.color;
   return TRUE;
}

BOOL WINAPI ChooseFontA(CHOOSEFONTA *cf)
{
   if (cf == nullptr || cf->lStructSize < sizeof(CHOOSEFONTA) || cf->lpLogFont == nullptr)
   {
      last_error = cf ? CDERR_INITIALIZATION : CDERR_STRUCTSIZE;
      return FALSE;
   }
   last_error = 0;
   // The hook (CF_ENABLEHOOK) customizes Windows' own dialog; the toolkit's has no
   // dialog window for it.
   Pending p;
   LOGFONTA initial = {};
   if (cf->Flags & CF_INITTOLOGFONTSTRUCT)
      initial = *cf->lpLogFont;
   else
   {
      initial.lfHeight = -16;
      strcpy(initial.lfFaceName, "Arial");
   }
   native::ChooseFont(OwnerTop(cf->hwndOwner), initial,
                      [](bool ok, const LOGFONTA &font, void *data) {
                         auto *p = (Pending *) data;
                         p->ok = ok;
                         p->font = font;
                         p->done = true;
                         loop::Wake();
                      },
                      &p);
   RunModal(cf->hwndOwner, &p);
   if (!p.ok)
      return FALSE;
   LOGFONTA chosen = p.font;
   // Effects the toolkit's chooser doesn't offer stay as they were.
   chosen.lfUnderline = initial.lfUnderline;
   chosen.lfStrikeOut = initial.lfStrikeOut;
   chosen.lfCharSet = initial.lfCharSet;
   chosen.lfQuality = initial.lfQuality;
   *cf->lpLogFont = chosen;
   cf->iPointSize = MulDiv(abs(chosen.lfHeight), 720, 96);   // tenths of a point at 96 DPI
   cf->nFontType = SCREEN_FONTTYPE | ((chosen.lfWeight >= FW_BOLD) ? BOLD_FONTTYPE : REGULAR_FONTTYPE) |
                   (chosen.lfItalic ? ITALIC_FONTTYPE : 0);
   return TRUE;
}

// No printing yet: the print dialog closes as if cancelled.
BOOL WINAPI PrintDlgA(PRINTDLGA *pd)
{
   if (pd == nullptr || pd->lStructSize < sizeof(PRINTDLGA))
   {
      last_error = CDERR_STRUCTSIZE;
      return FALSE;
   }
   last_error = 0;
   return FALSE;
}

DWORD WINAPI CommDlgExtendedError(void)
{
   return last_error;
}

INT WINAPI StartDocA(HDC, const DOCINFOA *)
{
   SetLastError(ERROR_NOT_SUPPORTED);
   return SP_ERROR;
}

INT WINAPI StartPage(HDC)
{
   return SP_ERROR;
}

INT WINAPI EndPage(HDC)
{
   return SP_ERROR;
}

INT WINAPI EndDoc(HDC)
{
   return SP_ERROR;
}

INT WINAPI AbortDoc(HDC)
{
   return SP_ERROR;
}

// dwmapi.h declares it without DECLSPEC_IMPORT.
W32_EXPORT HRESULT WINAPI DwmSetWindowAttribute(HWND hwnd, DWORD attribute, LPCVOID value, DWORD size)
{
   if (GetWindowObject(hwnd) == nullptr)
      return E_HANDLE;
   if (attribute == DWMWA_USE_IMMERSIVE_DARK_MODE && value && size >= sizeof(BOOL))
   {
      // GTK themes are per application: the main window's choice is everyone's.
      native::SetDarkTheme(*(const BOOL *) value != FALSE);
      return S_OK;
   }
   return E_INVALIDARG;
}

/*
 * winproc.h:  Window procedures in two character sets.  An ANSI procedure gets
 *   messages with char text, a Unicode one (registered or installed through a W
 *   function) WCHAR text; calls across sets go through the translations Windows
 *   makes (Wine's winproc.c: text, CREATESTRUCT, characters).  Asking for a
 *   procedure in the other set gives a handle that CallWindowProc translates
 *   through, as on Windows.
 */

#ifndef W32_CORE_WINPROC_H
#define W32_CORE_WINPROC_H

#include "core/win32.h"

#include <string>

namespace w32 {

struct ProcRef
{
   WNDPROC fn = nullptr;
   bool unicode = false;
};

// What GetWindowLongPtrA (unicode false) or W returns for p.
WNDPROC ProcValue(ProcRef p, bool unicode);
// What SetWindowLongPtrA/W(value) or CallWindowProcA/W(value) means.
ProcRef ProcFromValue(WNDPROC value, bool unicode);

// Calls p with a message in ANSI (unicode false) or Unicode form.
LRESULT CallProcRef(ProcRef p, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, bool unicode);

// Converts a message character (WM_CHAR's wParam ...) between the sets.
WPARAM CharAtoW(WPARAM wparam);
WPARAM CharWtoA(WPARAM wparam);

std::string WideToAnsi(const WCHAR *s, int length = -1);

// Case mapping of Latin-1 and Latin Extended-A, whatever the C locale
// (core/ctl_strings.cpp).
WCHAR WideLower(WCHAR c);
WCHAR WideUpper(WCHAR c);
std::basic_string<WCHAR> AnsiToWide(const char *s, int length = -1);

// A string argument that may be an atom or resource id (IS_INTRESOURCE): its ANSI
// form, kept alive for the call.
class AnsiArg
{
public:
   explicit AnsiArg(LPCWSTR s);
   LPCSTR get() const { return atom_ ? (LPCSTR) atom_ : text_.c_str(); }
   operator LPCSTR() const { return get(); }

private:
   ULONG_PTR atom_ = 0;
   std::string text_;
};

}  // namespace w32

#endif

/*
 * winuser.h:  Wine's winuser.h, plus wchar_t forms of the wide functions the
 *   Windows sources call with wchar_t buffers.  On Windows wchar_t is WCHAR
 *   (UTF-16); here WCHAR is char16_t and wchar_t is UTF-32, so these convert.
 */

#include_next <winuser.h>

#if defined(__cplusplus) && !defined(M59_WINUSER_WCHAR)
#define M59_WINUSER_WCHAR
UINT WINAPI GetDlgItemTextW(HWND dialog, INT id, wchar_t *text, INT size);
#endif

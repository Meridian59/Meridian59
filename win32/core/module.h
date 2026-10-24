/*
 * module.h:  Loaded binaries (HMODULE/HINSTANCE) and their resources.
 */

#ifndef W32_CORE_MODULE_H
#define W32_CORE_MODULE_H

#include "core/win32.h"

#include <string>

namespace w32 {

// A resource's bytes, or false.  type and name as FindResource takes them
// (MAKEINTRESOURCE ordinals or names, "#123" for ordinal 123).
bool FindResourceData(HMODULE module, LPCSTR name, LPCSTR type, const BYTE **data, DWORD *size);

// The user's language (LANGID, from LANG): resources' and the locale's.
WORD UserLanguage();

// The executable's module (GetModuleHandle(NULL)).
HMODULE MainModule();

// The module of a DLL the layer provides ("comctl32.dll"), with the resources
// the layer embeds for it; nullptr for other names.
HMODULE BuiltinDll(const char *name);

}  // namespace w32

#endif

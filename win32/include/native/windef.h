/*
 * windef.h:  Wine's windef.h, with one calling convention.  On x86_64 Wine marks
 *   WINAPI, CALLBACK and __cdecl as ms_abi, for calls between Windows and Unix
 *   code; everything here is native code, so they are the platform's own, and the
 *   C library can call the client's __cdecl callbacks (atexit, qsort, threads).
 */

#include_next <windef.h>

#undef __stdcall
#undef __cdecl
#undef __fastcall
#undef __thiscall
#define __stdcall
#define __cdecl
#define __fastcall
#define __thiscall
#undef __ms_va_list
#define __ms_va_list va_list

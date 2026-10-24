/*
 * winnt.h:  Wine's winnt.h.  Without MSVC it defines the Interlocked and BitScan
 *   intrinsics as inline functions under their MSVC names; Clang with
 *   -fms-extensions (the Windows sources' flag) has those names as builtins, so
 *   the definitions take other names, which the names then stand for.
 */

#if defined(__clang__) && !defined(_MSC_VER) && !defined(M59_WINNT_RENAMED)
#if __has_builtin(_InterlockedIncrement)
#define M59_WINNT_RENAMED
#define _BitScanForward m59_BitScanForward
#define _BitScanReverse m59_BitScanReverse
#define _InterlockedAnd m59_InterlockedAnd
#define _InterlockedAnd64 m59_InterlockedAnd64
#define _InterlockedCompareExchange m59_InterlockedCompareExchange
#define _InterlockedCompareExchange64 m59_InterlockedCompareExchange64
#define _InterlockedCompareExchange128 m59_InterlockedCompareExchange128
#define _InterlockedCompareExchangePointer m59_InterlockedCompareExchangePointer
#define _InterlockedDecrement m59_InterlockedDecrement
#define _InterlockedDecrement16 m59_InterlockedDecrement16
#define _InterlockedDecrement64 m59_InterlockedDecrement64
#define _InterlockedExchange m59_InterlockedExchange
#define _InterlockedExchangeAdd m59_InterlockedExchangeAdd
#define _InterlockedExchangeAdd16 m59_InterlockedExchangeAdd16
#define _InterlockedExchangeAdd64 m59_InterlockedExchangeAdd64
#define _InterlockedExchangePointer m59_InterlockedExchangePointer
#define _InterlockedIncrement m59_InterlockedIncrement
#define _InterlockedIncrement16 m59_InterlockedIncrement16
#define _InterlockedIncrement64 m59_InterlockedIncrement64
#define _InterlockedOr m59_InterlockedOr
#define _InterlockedOr64 m59_InterlockedOr64
#define _InterlockedXor m59_InterlockedXor
#define _InterlockedXor64 m59_InterlockedXor64
#define __fastfail m59_fastfail
#endif
#endif

#include_next <winnt.h>

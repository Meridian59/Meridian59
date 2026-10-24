/* crtdbg.h:  MSVC debug-heap hooks; none here. */
#ifndef M59_CRTDBG_H
#define M59_CRTDBG_H
#define _CRT_WARN   0
#define _CRT_ERROR  1
#define _CRT_ASSERT 2
#define _RPT0(type, msg)
#define _RPT1(type, fmt, a1)
#define _RPT2(type, fmt, a1, a2)
#define _ASSERT(expr)
#define _ASSERTE(expr)
#define _CrtSetDbgFlag(flag) 0
#define _CrtDumpMemoryLeaks() 0
#define _CRTDBG_ALLOC_MEM_DF   0
#define _CRTDBG_LEAK_CHECK_DF  0
#endif

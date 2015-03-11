//----------------------------------------------------------------------------
// ObjectWindows, OWL NExt
// Created by Kenneth Haley ( khaley@bigfoot.com ) 
//
/// \file
/// GNU support 
//----------------------------------------------------------------------------
#define _EXCPT_H
#define _EXCPT_H_

#if !defined(MAINWIN) && !defined(WINELIB)

#undef _OWLPCH
#include <owl/window.h>
#include <owl/registry.h>

using namespace owl;

#ifdef UNIX
#include <pshpack8.h>
#else
/* winemaker: #pragma pack(push, 8) */
#include <pshpack8.h>
#endif

extern "C" {

/// \cond
typedef struct _SCOPETABLE {
  unsigned long previousTryLevel;
  int __stdcall (*lpfnFilter)();     // address of filter function for this 
                                    // _try/_except 0 for finally
  void __stdcall (*lpfnHandler)();   // address of _except _except_handler3 
                                    // returns here if EXCEPTION_EXECUTE_HANDLER 
                                    // in filter function
} SCOPETABLE;


typedef struct _VC_EXCEPTION_REGISTRATION {
  unsigned long stan_stack;
  unsigned long xcept_ptrs;
  unsigned long prev;
  unsigned long handler;
  SCOPETABLE *tbl_ptr;
  unsigned long trylevel;
  unsigned long _ebp;
} VC_EXCEPTION_REGISTRATION;
/// \endcond

}

#define EH_NONCONTINUABLE  0x01
#define EH_UNWINDING      0x02
#define EH_EXIT_UNWIND    0x04
#define EH_STACK_INVALID  0x08
#define EH_NESTED_CALL    0x10

#define EXCEPTION_EXECUTE_HANDLER  1
//#define EXCEPTION_CONTINUE_EXECUTION   -1
#define EXCEPTION_CONTINUE_SEARCH       0

typedef enum {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;

// vc++ generated functions

#define GetExceptionInformation() \
__vcer.xcept_ptrs

#define GetExceptionCode()\
(**((int**)__vcer.xcept_ptrs))

#define RpcExceptionCode() GetExceptionCode()

//the above macros are only usable inside _try/_except block
//gcc will not generate a warning if these are used
//outside the _except block, but you will get garbage results

#define _SXV(args...) \
__asm__ __volatile__("/BEGIN_SXV\n"); \
VC_EXCEPTION_REGISTRATION __vcer; \
int __volatile__ __xcpt_filter_result __attribute__((__unused__)); \
int __volatile__ __xcpt_happened __attribute__((__unused__)); \
__asm__ __volatile__(".equ __except_list, 0\n"); \
__vcer.trylevel=(unsigned long)-1; \
__vcer.handler=(unsigned long)&mexcept_handler3; \
__asm__ __volatile__("/END_SXV\n");

#undef _try
#define _try \
__asm__ __volatile__(\
"/ BEGIN_TRY\n\t"\
"movl %%esp, %0;\n\t"\
"movl %%ebp, %1;\n"\
: "=g" (__vcer.stan_stack), "=g" (__vcer._ebp) : );\
__asm__ __volatile__(\
  "movl %%fs:__except_list,%0\n\t"\
  "movl %2,%%fs:__except_list\n\t"\
  "/ ScopeTable Pointer\n\t"\
  "movl $5f,%1\n"\
  : "=g" (__vcer.prev), "=m" (__vcer.tbl_ptr): "r" (&__vcer.prev) : "memory");\
  ++__vcer.trylevel;\
__asm__ __volatile__("/ END_TRY");\
   if(1)
//JJH
#if defined(__GNUC__)
#undef _except //(args...)
#else
#undef _except(args...)
#endif //__GNUC__
#define _except(args...) \
__asm__ __volatile__("/BEGIN_EXCEPT\n");\
__xcpt_happened = 0;\
__asm__ __volatile__(\
".section .rdata\n"\
".align 16, 0x0\n\t"\
  "5:.int -1, 1f, 2f\n"\
".text\n\t"\
  "jmp 2f\n"\
  "1:\n");\
__xcpt_filter_result = (## args);\
__asm__ __volatile__(\
        "cmpl $-1,%1\n\t"\
        "je 4f\n\t"\
        "incl %0\n\t"\
  : "=m" (__xcpt_happened)\
  : "a" (__xcpt_filter_result)\
   );\
__asm__ __volatile__(\
        "4:ret\n\t"\
        "2:\n"\
        "/ reset __except_list to previous exception function\n\t"\
        "movl %0,%%fs:__except_list\n" : : "r" (__vcer.prev) : "memory");\
--__vcer.trylevel;\
__asm__ __volatile__("/END_EXCEPT\n");\
if(__xcpt_happened && (## args))

#define __try _try
#define __try__ _try
#define __except _except
#define __except__ _except

#ifdef UNIX
#include <poppack.h>
#else
/* winemaker: #pragma pack(pop) */
#include <poppack.h>
#endif

extern "C" {
  int mexcept_handler3(
    struct _EXCEPTION_RECORD*  pExceptionRecord,
    VC_EXCEPTION_REGISTRATION*  pRegistrationFrame,
    struct _CONTEXT*    pContextRecord,
    void*
  );
};
#define TRYLEVEL_NONE -1
extern "C" {
  void _global_unwind2(void*);
  void _local_unwind2(void*,int);
  int mexcept_handler3(
    struct _EXCEPTION_RECORD*  pExceptionRecord,
    VC_EXCEPTION_REGISTRATION*  pRegistrationFrame,
    struct _CONTEXT*    pContextRecord,
    void*
  );
};

int __cdecl mexcept_handler3(
  struct _EXCEPTION_RECORD*  pExceptionRecord,
  VC_EXCEPTION_REGISTRATION*  pRegistrationFrame,
  struct _CONTEXT*    pContextRecord,
  void *
  )
{
  VC_EXCEPTION_REGISTRATION*  __vcer=(VC_EXCEPTION_REGISTRATION*)((long*)pRegistrationFrame-2);
  LONG  filterFuncRet;
  LONG trylevel;
  EXCEPTION_POINTERS exceptPtrs;
  SCOPETABLE* pScopeTable;


  __asm__ __volatile__("cld"); // Clear the direction flag (make no assumptions!)

  // if neither the EXCEPTION_UNWINDING nor EXCEPTION_EXIT_UNWIND bit
  // is set... This is true the first time through the handler (the
  // non-unwinding case)
  if ( ! (pExceptionRecord->ExceptionFlags
     & (EH_UNWINDING | EH_EXIT_UNWIND)
               ) )
  {
    // Build the EXCEPTION_POINTERS structure on the stack
    exceptPtrs.ExceptionRecord = pExceptionRecord;
    exceptPtrs.ContextRecord = pContextRecord;

    // Put the pointer to the EXCEPTION_POINTERS 4 bytes below the
    // establisher frame. See ASM code for GetExceptionInformation
    __vcer->xcept_ptrs = (unsigned long)&exceptPtrs;

    // Get initial "trylevel" value
    trylevel = __vcer->trylevel ;

    // Get a pointer to the scopetable array
    pScopeTable = __vcer->tbl_ptr;

search_for_handler: 
    if ( trylevel != TRYLEVEL_NONE ){
      if ( pScopeTable[trylevel].lpfnFilter ){


      // !!!Very Important!!! Switch to original EBP. This is
      // what allows all locals in the frame to have the same
      // value as before the exception occurred.


        // Call the filter function
        __asm__ __volatile__("pushl %%ebp\n\t"\
            "movl %2,%%ebp\n\t"\
            "call *%1\n\t"\
            "popl %%ebp" : "=a" (filterFuncRet) \
            : "r" (pScopeTable[trylevel].lpfnFilter), "m" (__vcer->_ebp) );


        if ( filterFuncRet != EXCEPTION_CONTINUE_SEARCH ){
          if ( filterFuncRet < 0 ) // EXCEPTION_CONTINUE_EXECUTION
            return ExceptionContinueExecution;

          // If we get here, EXCEPTION_EXECUTE_HANDLER was specified
          //scopetable == pRegistrationFrame->scopetable

          // Does the actual OS cleanup of registration frames
          // Causes this function to recurse
          _global_unwind2( pRegistrationFrame );


          // Once we get here, everything is all cleaned up, except
          // for the last frame, where we'll continue execution
          //PUSH EBP // Save EBP
          //EBP = pRegistrationFrame->_ebp // Set EBP for __local_unwind2
          //why???

          _local_unwind2( pRegistrationFrame, trylevel );

          // POP EBP

          // Set the current trylevel to whatever SCOPETABLE entry
          // was being used when a handler was found   (why?)

          __vcer->trylevel = pScopeTable->previousTryLevel;

          // Call the _except {} block. Never returns.
          //__vcer->tbl_ptr[trylevel].lpfnHandler();
          // effectively a longjump

          __asm__ __volatile__ ("pushfl\n\tpopl %%esi\n\t"\
              "movl %0, %%esp\n\t"\
              "movl %1, %%ebp\n\t"\
              "pushl %2\n\t"\
              "pushl %%esi\n\tpopfl\n\t"\
              "ret": : "r" (__vcer->stan_stack), "r" (__vcer->_ebp), "r" (pScopeTable->lpfnHandler) );

        }///continue search
      }//filter
      __vcer=(VC_EXCEPTION_REGISTRATION*)(((long*)__vcer->prev)-2);
      trylevel = pScopeTable->previousTryLevel;
      pScopeTable = __vcer->tbl_ptr;

      goto search_for_handler;
    }  
    else{ // trylevel == TRYLEVEL_NONE
      //retvalue == DISPOSITION_CONTINUE_SEARCH;
      return ExceptionContinueSearch;
    }
  }  
  else { // EXCEPTION_UNWINDING or EXCEPTION_EXIT_UNWIND flags are set
//    PUSH EBP // Save EBP
//    EBP = pRegistrationFrame->_ebp // Set EBP for __local_unwind2
//why ???
//    __asm__ __volatile__("pushl %%ebp\n\tmovl %0,%%ebp": :"m" (__vcer->_ebp));

    _local_unwind2( pRegistrationFrame, TRYLEVEL_NONE );

//    __asm__ __volatile__("pop %ebp"); // Restore EBP

//    retvalue == DISPOSITION_CONTINUE_SEARCH;
    return ExceptionContinueSearch;
  }
}

namespace owl {

TDrawItem* lTCB(uint32 data)
{
  _SXV(;);
  // use SEH (structured exception handling) to catch even GPFs
  //  that result from partially valid objects.
  TDrawItem* item;
  __try{
    item = TYPESAFE_DOWNCAST((TDrawItemBase*)data, TDrawItem);
  }
  __except(EXCEPTION_EXECUTE_HANDLER){
    return 0;
  }
  return item;
}

TRegKey::TRegKey(THandle aliasKey, bool shouldClose, LPCTSTR keyName)
:
  Key(aliasKey),
  Name(strnewdup(keyName))
{
  _SXV(;);
  ShouldClose = shouldClose;

  __try {
    long err = QueryInfo(0, 0, &SubkeyCount, 0, 0, &ValueCount, 0, 0, 0, 0);
    if (err != S_OK && err != ERROR_INSUFFICIENT_BUFFER)
      Key = 0;
  }
  // Some key handles are unsupported & sometimes the OS crashes, & doesn't
  // just return an error. Catch it here & zero out this key.
  //
  __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
  {
    Key = 0;
  }
}
} // OWL namespace

#endif //ifndef MAINWIN && !defined(WINELIB)
/*===============================================================*/

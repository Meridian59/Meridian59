//----------------------------------------------------------------------------
// ObjectWindows
// (C) Copyright 1993, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_DUMPSTACK_H)
#define OWL_PRIVATE_DUMPSTACK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

namespace owl {

#define OWL_STACK_DUMP_TARGET_TRACE     0x0001
#define OWL_STACK_DUMP_TARGET_CLIPBOARD 0x0002
#define OWL_STACK_DUMP_TARGET_BOTH      0x0003
#define OWL_STACK_DUMP_TARGET_ODS       0x0004
#ifdef _DEBUG
#define OWL_STACK_DUMP_TARGET_DEFAULT   OWL_STACK_DUMP_TARGET_TRACE
#else
#define OWL_STACK_DUMP_TARGET_DEFAULT   OWL_STACK_DUMP_TARGET_CLIPBOARD
#endif

_OWLFUNC(void) OwlDumpStack(uint32 target = OWL_STACK_DUMP_TARGET_DEFAULT);


} // OWL namespace

#endif  // OWL_PRIVATE_DUMPSTACK_H

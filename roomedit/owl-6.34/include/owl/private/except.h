//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// Reliable include for standard C++ exception classes
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_EXCEPT_H)
# define OWL_PRIVATE_EXCEPT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

//
// In a Borland C++ environment, use the file provided
//
# if defined(BI_COMP_BORLANDC)

#  include <except.h>

//
// In a non-Borland C++ environment, use the C++ Exception Handling support
// and add missing components of that support.
//
# else
#    if defined(BI_COMP_MSC)
#    include <eh.h>
#    endif

#    include <stdlib.h>
#    include <new>

#  endif  // if/else BI_COMP_BORLANDC

#endif  // OWL_PRIVATE_EXCEPT_H

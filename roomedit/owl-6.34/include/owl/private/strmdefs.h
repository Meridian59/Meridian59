//----------------------------------------------------------------------------
// Object Windows Library
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
// I/O stream defines
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_STRMDEFS_H)
#define OWL_PRIVATE_STRMDEFS_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <iomanip>
#include <iosfwd>
#include <sstream>

namespace owl {

#if defined(UNICODE)

typedef std::wostringstream tostringstream;
typedef std::wistringstream tistringstream;
typedef std::wstringstream tstringstream;
typedef std::wistream tistream;
typedef std::wostream tostream;
typedef std::wstreambuf tstreambuf;
typedef std::wios tios;
typedef std::wifstream tifstream;
typedef std::wofstream tofstream;

#else

typedef std::ostringstream tostringstream;
typedef std::istringstream tistringstream;
typedef std::stringstream tstringstream;
typedef std::istream tistream;
typedef std::ostream tostream;
typedef std::streambuf tstreambuf;
typedef std::ios tios;
typedef std::ifstream tifstream;
typedef std::ofstream tofstream;

#endif

} // OWL namespace

#endif //OWL_PRIVATE_STRMDEFS_H

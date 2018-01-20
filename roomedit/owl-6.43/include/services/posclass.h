//----------------------------------------------------------------------------
// Borland Services Library Compatibility header
//
//----------------------------------------------------------------------------

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#pragma nopackwarning

#pragma message ("Warning!!! Included obsolete file: <services/posclass.h> use <owl/posclass.h instead>")

#if ! (defined(lint) || defined(_lint))
#  if ( _MSC_VER >= 800 ) || (defined(__BORLANDC__))
#    pragma warning(disable:4103)
#    pragma pack()
#  endif
#endif // ! (defined(lint) || defined(_lint))


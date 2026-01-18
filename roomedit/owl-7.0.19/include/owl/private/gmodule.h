#if !defined(OWL_GMODULE_H)
#define OWL_GMODULE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

namespace owl {
class _OWLCLASS TModule;
extern TModule* Module;   // Global Module ptr in each DLL/EXE
} // OWL namespace


#endif  // OWL_GMODULE_H

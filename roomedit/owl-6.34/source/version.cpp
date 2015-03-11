//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include <owl/version.h>

namespace owl {

OWL_DIAGINFO;

/// \cond


struct TVersionHeader {
  char          Signature[6];
  unsigned long InternalVersionNumber;
  /*
  char          ComponentIdString[];
  */
};

// NOTE: The following variables must be contiguous in memory
//
//#if defined(WINELIB)
//static
//#endif
TVersionHeader VersionHeader = {
  {'$', '*', '$', '*', '$', '*'},
  OWLInternalVersion
};


/// \endcond

} // OWL namespace
/* ========================================================================== */


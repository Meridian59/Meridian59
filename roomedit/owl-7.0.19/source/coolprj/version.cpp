//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <coolprj/pch.h>
#pragma hdrstop

#include <coolprj/version.h>

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
  COOLPRJ_VERSION_32BIT
};


/// \endcond

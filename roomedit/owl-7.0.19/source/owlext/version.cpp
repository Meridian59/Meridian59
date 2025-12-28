//----------------------------------------------------------------------------
// OWLExt
//
//----------------------------------------------------------------------------
#include <owlext/pch.h>
#pragma hdrstop

#include <owlext/version.h>

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
  OWLEXT_VERSION_32BIT
};


/// \endcond

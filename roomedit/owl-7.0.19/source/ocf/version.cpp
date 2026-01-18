//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#pragma hdrstop

#include <ocf/version.h>

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
  OCF_VERSION_32BIT
};


/// \endcond

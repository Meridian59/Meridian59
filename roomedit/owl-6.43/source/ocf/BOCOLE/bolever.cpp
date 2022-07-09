//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//   Global version structure for vprint
//----------------------------------------------------------------------------
#include "bolever.h"

#define WhoIsUs "BOCOLE" //SL: check if we can safely rename to "BOCOF"/"BOCOFW"

struct TVersionHeader {
  char          Signature[6];
  unsigned long InternalVersionNumber;
  char          ComponentIdString[sizeof(WhoIsUs)+1];
};

TVersionHeader VersionHeader = {
  {'$', '*', '$', '*', '$', '*'},
  {BOLE_FILEVER_LS},
  WhoIsUs,
};

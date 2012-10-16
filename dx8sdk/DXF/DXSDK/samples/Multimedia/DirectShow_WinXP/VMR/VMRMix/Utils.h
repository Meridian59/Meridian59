//------------------------------------------------------------------------------
// File: Utils.h
//
// Desc: DirectShow sample code
//       Helper function prototypes
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(UTILS_H)
#define UTILS_H

#include "stdafx.h"

// helper function prototypes

DWORD MyMessage(char *sQuestion, char *sTitle);
DWORD ShellAbort(CVMRCore *core);

const char * hresultNameLookup(HRESULT hres);

bool MySleep(DWORD  dwTime = 2500);
void GetMessageName(LPSTR Name, UINT msg);

#endif
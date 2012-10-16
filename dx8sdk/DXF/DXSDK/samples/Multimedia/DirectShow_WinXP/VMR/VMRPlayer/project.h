//------------------------------------------------------------------------------
// File: project.h
//
// Desc: DirectShow sample code
//       - Master header file that includes all other header files used
//         by the project.  This enables precompiled headers during build.
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#include "app.h"
#include "vcdplyer.h"
#include "resource.h"


void InitStreamParams(int i);
